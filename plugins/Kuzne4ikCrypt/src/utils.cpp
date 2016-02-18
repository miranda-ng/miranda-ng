/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-16 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "stdafx.h"

bool getRandomBytes(BYTE *buf, size_t bufLen)
{
	// try to use Intel hardware randomizer first
	HCRYPTPROV hProvider = NULL;
	if (::CryptAcquireContext(&hProvider, NULL, _T("Intel Hardware Cryptographic Service Provider"), PROV_INTEL_SEC, 0) ||
		 ::CryptAcquireContext(&hProvider, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) ||
		 ::CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		::CryptGenRandom(hProvider, DWORD(bufLen), buf);
		::CryptReleaseContext(hProvider, 0);
	}
	// no luck? try to use Windows NT RTL
	else {
		typedef BOOL(WINAPI *pfnGetRandom)(PVOID RandomBuffer, ULONG RandomBufferLength);
		pfnGetRandom fnGetRandom = (pfnGetRandom)GetProcAddress(GetModuleHandleA("advapi32.dll"), "SystemFunction036");
		if (fnGetRandom == NULL)
			return false;

		fnGetRandom(buf, DWORD(bufLen));
	}
	return true;
}

void slow_hash(const void *buf, size_t bufLen, BYTE* tmpHash)
{
	mir_sha256_hash(buf, bufLen, tmpHash);

	for (int i = 0; i < 50000; i++)
		mir_sha256_hash(tmpHash, 32, tmpHash);
}
