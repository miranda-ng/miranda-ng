/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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

#ifndef M_CRYPTO_H__
#define M_CRYPTO_H__ 1

#include <m_core.h>

typedef struct tagCRYPTOENGINE
{
	DWORD	dwVersion;

	// get/set the instance key
	void (__cdecl *pfnGetKey)(const BYTE *pKey, size_t cbKeyLen);
	BOOL (__cdecl *pfnSetKey)(const BYTE **pKey, size_t *cbKeyLen);

	void(__cdecl *pfnGenerateKey)(void); // creates a new key inside
	void(__cdecl *pfnPurgeKey)(void);    // purges a key from memory

	// sets the master password (in utf-8)
	void(__cdecl *pfnSetPassword)(const char *pszPassword);

	BYTE* (__cdecl *pfnEncodeString)( const char *src, size_t *cbResultLen);
	BYTE* (__cdecl *pfnEncodeStringW)(const WCHAR* src, size_t *cbResultLen);

	char*  (__cdecl *pfnDecodeString)(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen);
	WCHAR* (__cdecl *pfnDecodeStringW)(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen);
}
CRYPTO_ENGINE;

//registers a crypto provider v0.94+
//wParam = (int)hLangpack
//lParam = (CRYPTO_PROVIDER*)
//returns HANDLE on success or NULL on failure

typedef CRYPTO_ENGINE* (__cdecl *pfnCryptoProviderFactory)(void);

#define CPF_UNICODE 1

#if defined(_UNICODE)
	#define CPF_TCHAR CPF_UNICODE 
#else
	#define CPF_TCHAR 0
#endif

typedef struct tagCRYPTOPROVIDER
{
	DWORD	dwSize;
	DWORD	dwFlags; // one of CPF_* constants

	char *pszName; // unique id
	union {
		char *pszDescr;   // description
		TCHAR *ptszDescr;	// auto translated by core
		WCHAR *pwszDescr;
	};

	pfnCryptoProviderFactory pFactory;
}
CRYPTO_PROVIDER;

#define MS_CRYPTO_REGISTER_ENGINE "SRCrypto/RegisterEngine"

__forceinline HANDLE Crypto_RegisterEngine(CRYPTO_PROVIDER *pProvider)
{
	extern int hLangpack;
	return (HANDLE)CallService(MS_CRYPTO_REGISTER_ENGINE, hLangpack, (LPARAM)pProvider);
}

#endif // M_CRYPTO_H__
