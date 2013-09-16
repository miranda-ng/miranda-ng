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
#include "SHA256.h"
#include <stdlib.h>
#include <string.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#define NO_ASM
#endif
#define SHA_LOOPUNROLL

#define rotr(x,n) _lrotr(x,n)


// table of round constants
// (first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311):
static const uint32_t cKey[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// initialisation vector
// (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
static const SHA256::THash cHashInit = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};



SHA256::SHA256()
{
	SHAInit();
}
SHA256::~SHA256()
{

}
void SHA256::SHAInit()
{
	memcpy(m_Hash, cHashInit, sizeof(m_Hash));
	m_Length = 0;
}
void SHA256::SHAUpdate(void * Data, uint32_t Length)
{
	uint8_t * dat = (uint8_t *)Data;
	uint32_t len = Length;

	if (m_Length & 63)
	{
		uint32_t p = (m_Length & 63);
		uint32_t pl = 64 - p;
		if (pl > len)
			pl = len;

		memcpy(&(m_Block[p]), dat, pl);
		len -= pl;
		dat += pl;

		if (p + pl == 64)
			SHABlock();
	}

	while (len >= 64)
	{
		memcpy(m_Block, dat, sizeof(m_Block));
		SHABlock();
		len -= 64;
		dat += 64;
	}

	if (len > 0)
	{
		memcpy(m_Block, dat, len);
	}

	m_Length += Length;
}

void SHA256::SHAFinal(SHA256::THash & Hash)
{
	uint8_t pad[128] = {
			0x80, 0,0,0,0,0,0,0,
			    0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0
			};

	uint32_t padlen = 0;
	if ((m_Length & 63) < 55) // 64 - 9 -> short padding
	{
		padlen = 64 - (m_Length & 63);
	} else {
		padlen = 128 - (m_Length & 63);
	}

	uint64_t l = m_Length << 3;
	{
		uint8_t * p = (uint8_t *) &l;
#ifdef NO_ASM
		pad[padlen - 1] = (uint8_t)(p[0]);
		pad[padlen - 2] = (uint8_t)(p[1]);
		pad[padlen - 3] = (uint8_t)(p[2]);
		pad[padlen - 4] = (uint8_t)(p[3]);
		pad[padlen - 5] = (uint8_t)(p[4]);
		pad[padlen - 6] = (uint8_t)(p[5]);
		pad[padlen - 7] = (uint8_t)(p[6]);
		pad[padlen - 8] = (uint8_t)(p[7]);
#else
		uint8_t * p2 = (uint8_t *) &(pad[padlen - 8]);
		__asm {
			MOV ebx, p
			MOV eax, [ebx]
			BSWAP eax
			MOV edx, [ebx + 4]
			MOV ebx, p2
			BSWAP edx
			MOV [ebx + 4], eax
			MOV [ebx], edx
		}
#endif
	}

	SHAUpdate((uint32_t *)pad, padlen);

	{
		uint8_t * h = (uint8_t *)Hash;
		uint8_t * m = (uint8_t *)m_Hash;
#ifdef NO_ASM
		for (int i = 0; i < 32; i += 4)
		{
			h[i] = m[i + 3];
			h[i + 1] = m[i + 2];
			h[i + 2] = m[i + 1];
			h[i + 3] = m[i];
		}
#else
		__asm {
			MOV esi, m
			MOV edi, h
			MOV ecx, 8
			loop_label:
				LODSD
				BSWAP eax
				STOSD
				dec ecx
			jnz loop_label
		}
#endif
	}

	SHAInit();
}

#define SHA256_ROUND(a,b,c,d,e,f,g,h, i) { \
t1  = (h) + (rotr((e), 6) ^ rotr((e), 11) ^ rotr((e), 25)) + \
		  (((e) & (f)) ^ ((~(e)) & (g))) + cKey[i] + w[i];       \
t2  = (rotr((a), 2) ^ rotr((a), 13) ^ rotr((a), 22)) +       \
      (((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)));             \
d += t1;     \
h = t1 + t2; \
}

void SHA256::SHABlock()
{
	uint32_t w[64];

	// make Big Endian
	{
		uint8_t * d = (uint8_t *)w;
		uint8_t * s = (uint8_t *)m_Block;
#ifdef NO_ASM
		for (int i = 0; i < 64; i += 4)
		{
			d[i] = s[i + 3];
			d[i + 1] = s[i + 2];
			d[i + 2] = s[i + 1];
			d[i + 3] = s[i];
		}
#else
		__asm {
			MOV esi, s
			MOV edi, d
			MOV ecx, 16
			loop_label:
				LODSD
				BSWAP eax
				STOSD
				dec ecx
			jnz loop_label
		}
#endif
	}

	uint32_t t1, t2, a,b,c,d,e,f,g,h;
	for (uint32_t i = 16; i < 64; ++i)
	{
		t1 = w[i-15];
		t2 = w[i-2];
		w[i] = w[i-16] + (rotr(t1, 7) ^ rotr(t1, 18) ^ (t1 >> 3)) + w[i-7] + (rotr(t2, 17) ^ rotr(t2, 19) ^ (t2 >> 10));
	}

	a = m_Hash[0];
	b = m_Hash[1];
	c = m_Hash[2];
	d = m_Hash[3];
	e = m_Hash[4];
	f = m_Hash[5];
	g = m_Hash[6];
	h = m_Hash[7];

#ifdef SHA_LOOPUNROLL
	for (uint32_t i = 0; i < 64; ++i)
	{
		SHA256_ROUND(a,b,c,d,e,f,g,h,i); ++i;
		SHA256_ROUND(h,a,b,c,d,e,f,g,i); ++i;
		SHA256_ROUND(g,h,a,b,c,d,e,f,i); ++i;
		SHA256_ROUND(f,g,h,a,b,c,d,e,i); ++i;
		SHA256_ROUND(e,f,g,h,a,b,c,d,i); ++i;
		SHA256_ROUND(d,e,f,g,h,a,b,c,i); ++i;
		SHA256_ROUND(c,d,e,f,g,h,a,b,i); ++i;
		SHA256_ROUND(b,c,d,e,f,g,h,a,i);
	}
#else
	for (uint32_t i = 0; i < 64; ++i)
	{
		t1  = h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + //s1
			        ((e & f) ^ ((~e) & g)) + cKey[i] + w[i]; //ch

		t2  = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) +  //s0
		      ((a & b) ^ (a & c) ^ (b & c)); //maj

		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}
#endif

	m_Hash[0] += a;
	m_Hash[1] += b;
	m_Hash[2] += c;
	m_Hash[3] += d;
	m_Hash[4] += e;
	m_Hash[5] += f;
	m_Hash[6] += g;
	m_Hash[7] += h;
}
