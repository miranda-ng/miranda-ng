/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

// global:
HINSTANCE		ghInst		= NULL;
TIME_API		tmi;					//timezone interface
FI_INTERFACE	*FIP		= NULL;		//freeimage interface
CLIST_INTERFACE *pcli		= NULL;

MGLOBAL			myGlobals;
pfnDwmIsCompositionEnabled	dwmIsCompositionEnabled;

/**
 * Calculates an unique DWORD number from a string.
 * It's the same as used in langpack.
 *
 * @param		szStr	- string to calculate the hash value for
 * @return	the unique id for the szStr
 **/

#if __GNUC__
#define NOINLINEASM
#endif

DWORD hashSetting(LPCSTR szStr)
{
#if defined _M_IX86 && !defined _NUMEGA_BC_FINALCHECK && !defined NOINLINEASM
	__asm
	{
		xor		edx,edx
		xor		eax,eax
		mov		esi,szStr
		mov		al,[esi]
		dec		esi
		xor		cl,cl
		lph_top:			//only 4 of 9 instructions in here don't use AL, so optimal pipe use is impossible
		xor		edx,eax
		inc		esi
		and		cl,31
		movzx	eax,byte ptr [esi]
		add		cl,5
		test	al,al
		rol		eax,cl		//rol is u-pipe only, but pairable
							//rol doesn't touch z-flag
		jnz		lph_top		//5 clock tick loop. not bad.

		xor		eax,edx
	}
#else
	DWORD hash = 0;
	int i;
	int shift = 0;
	for (i = 0; szStr[i]; i++)
	{
		hash ^= szStr[i] << shift;
		if (shift > 24)
		{
			hash ^= (szStr[i] >> (32 - shift)) & 0x7F;
		}
		shift = (shift + 5) & 0x1F;
	}
	return hash;
#endif
}

// MurmurHash2
#ifdef _DEBUG
#pragma optimize( "gt", on )
#endif
unsigned int __fastcall hash_M2(const void * key, unsigned int len)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value
	unsigned int h = len;

	// Mix 4 bytes at a time into the hash
	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array
	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
			h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

unsigned int hashSettingW_M2(const char * key)
{
	if (key == NULL) return 0;
	const unsigned int len = (unsigned int)mir_wstrlen((const wchar_t*)key);
	char* buf = (char*)alloca(len + 1);
	for (unsigned i = 0; i <= len ; ++i)
		buf[i] = key[i << 1];
	return hash_M2(buf, len);
}

unsigned int hashSetting_M2(const char * key)
{
	if (key == NULL) return 0;
	const unsigned int len = (unsigned int)mir_strlen((const char*)key);
	return hash_M2(key, len);
}

unsigned int hashSetting_M2(const wchar_t * key)
{
	if (key == NULL) return 0;
	const unsigned int len = (unsigned int)mir_wstrlen((const wchar_t*)key);
	return hash_M2(key, len * sizeof(wchar_t));
}

#ifdef _DEBUG
#pragma optimize( "", on )
#endif

INT_PTR myDestroyServiceFunction(const char * key) {
	//DestroyServiceFunction always return 0 therfore we must call ServiceExists to enshure it is delete
	if (!ServiceExists(key)) return 0;
	DestroyServiceFunction((HANDLE)(INT_PTR)hashSetting(key));		//old hash
	if (!ServiceExists(key)) return 0;
	DestroyServiceFunction((HANDLE)(INT_PTR)hashSetting_M2(key));	//new MurmurHash2
	if (!ServiceExists(key)) return 0;
	return 1;
}
