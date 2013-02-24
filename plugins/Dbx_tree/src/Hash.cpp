/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Interface.h"
#include "Hash.h"


/// lookup3, by Bob Jenkins, May 2006, Public Domain.
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define HASHmix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}
#define HASHfinal(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c, 4); \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}


uint32_t Hash(const void * Data, uint32_t Length)
{
	register uint32_t a,b,c; // internal state
  union { const void *ptr; uint32_t i; } u; // needed for Mac Powerbook G4

  // Set up the internal state
  a = b = c = 0xdeadbeef + Length; // + initval = 0

  u.ptr = Data;
  if ((u.i & 0x3) == 0) 
	{
    const uint32_t *k = (const uint32_t *)Data; // read 32-bit chunks

    // all but last block: aligned reads and affect 32 bits of (a,b,c)
    while (Length > 12)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      HASHmix(a,b,c);
      Length -= 12;
      k += 3;
    }

    switch(Length)
    {
			case 12: c += k[2];            b += k[1]; a += k[0]; break;
			case 11: c += k[2] & 0xffffff; b += k[1]; a += k[0]; break;
			case 10: c += k[2] & 0xffff;   b += k[1]; a += k[0]; break;
			case 9 : c += k[2] & 0xff;     b += k[1]; a += k[0]; break;
			case 8 : b += k[1];            a += k[0]; break;
			case 7 : b += k[1] & 0xffffff; a += k[0]; break;
			case 6 : b += k[1] & 0xffff;   a += k[0]; break;
			case 5 : b += k[1] & 0xff;     a += k[0]; break;
			case 4 : a += k[0];            break;
			case 3 : a += k[0] & 0xffffff; break;
			case 2 : a += k[0] & 0xffff;   break;
			case 1 : a += k[0] & 0xff;     break;
			case 0 : return c;  // zero length strings require no mixing
    }

  } else if ((u.i & 0x1) == 0) {
    const uint16_t *k = (const uint16_t *)Data;         /* read 16-bit chunks */
		const uint8_t  *k8;

    // all but last block: aligned reads and different mixing
    while (Length > 12)
    {
      a += k[0] + (((uint32_t)k[1]) << 16);
      b += k[2] + (((uint32_t)k[3]) << 16);
      c += k[4] + (((uint32_t)k[5]) << 16);
      HASHmix(a,b,c);
      Length -= 12;
      k += 6;
    }

    // handle the last (probably partial) block
    k8 = (const uint8_t *)k;
    switch(Length)
    {
			case 12: c += k[4] + (((uint32_t)k[5]) << 16);
							 b += k[2] + (((uint32_t)k[3]) << 16);
							 a += k[0] + (((uint32_t)k[1]) << 16);
							 break;
			case 11: c += ((uint32_t)k8[10]) << 16; // fall through 
			case 10: c += k[4];
							 b += k[2] + (((uint32_t)k[3]) << 16);
							 a += k[0] + (((uint32_t)k[1]) << 16);
							 break;
			case 9 : c += k8[8];                        // fall through
			case 8 : b += k[2] + (((uint32_t)k[3]) << 16);
							 a += k[0] + (((uint32_t)k[1]) << 16);
							 break;
			case 7 : b += ((uint32_t)k8[6]) << 16;  // fall through
			case 6 : b += k[2];
							 a += k[0] + (((uint32_t)k[1]) << 16);
							 break;
			case 5 : b += k8[4];                        // fall through
			case 4 : a += k[0] + (((uint32_t)k[1]) << 16);
							 break;
			case 3 : a += ((uint32_t)k8[2]) << 16;  // fall through
			case 2 : a += k[0];
							 break;
			case 1 : a += k8[0];
							 break;
			case 0 : return c; // zero length requires no mixing
    }

  } else { // need to read the key one byte at a time
    const uint8_t *k = (const uint8_t *)Data;

    // all but the last block: affect some 32 bits of (a,b,c)
    while (Length > 12)
    {
      a += k[0];
      a += ((uint32_t)k[1] ) <<  8;
      a += ((uint32_t)k[2] ) << 16;
      a += ((uint32_t)k[3] ) << 24;
      b += k[4];
      b += ((uint32_t)k[5] ) <<  8;
      b += ((uint32_t)k[6] ) << 16;
      b += ((uint32_t)k[7] ) << 24;
      c += k[8];
      c += ((uint32_t)k[9] ) << 8;
      c += ((uint32_t)k[10]) << 16;
      c += ((uint32_t)k[11]) << 24;
      HASHmix(a,b,c);
      Length -= 12;
      k += 12;
    }

    // last block: affect all 32 bits of (c)
    switch(Length) // all the case statements fall through
    {
			case 12: c += ((uint32_t)k[11]) << 24;
			case 11: c += ((uint32_t)k[10]) << 16;
			case 10: c += ((uint32_t)k[9] ) <<  8;
			case 9 : c += k[8];
			case 8 : b += ((uint32_t)k[7] ) << 24;
			case 7 : b += ((uint32_t)k[6] ) << 16;
			case 6 : b += ((uint32_t)k[5] ) <<  8;
			case 5 : b += k[4];
			case 4 : a += ((uint32_t)k[3] ) << 24;
			case 3 : a += ((uint32_t)k[2] ) << 16;
			case 2 : a += ((uint32_t)k[1] ) <<  8;
			case 1 : a += k[0];
							 break;
			case 0 : return c;
    }
  }

  HASHfinal(a,b,c);
  return c;
}
