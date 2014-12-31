/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

struct MICryptoEngine
{
	DWORD	dwVersion;

	STDMETHOD_(void, destroy)(void) PURE;

	// get/set the instance key
	STDMETHOD_(size_t, getKeyLength)(void) PURE;
	STDMETHOD_(bool, getKey)(BYTE *pKey, size_t cbKeyLen) PURE;
	STDMETHOD_(bool, setKey)(const BYTE *pKey, size_t cbKeyLen) PURE;

	STDMETHOD_(bool, generateKey)(void)PURE; // creates a new key inside
	STDMETHOD_(void, purgeKey)(void)PURE;    // purges a key from memory

	// sets the master password (in utf-8)
	STDMETHOD_(bool, checkPassword)(const char *pszPassword) PURE;
	STDMETHOD_(void, setPassword)(const char *pszPassword) PURE;

	// result must be freed using mir_free or assigned to mir_ptr<BYTE>
	STDMETHOD_(BYTE*, encodeString)(const char *src, size_t *cbResultLen) PURE;
	STDMETHOD_(BYTE*, encodeBuffer)(const void *src, size_t cbLen, size_t *cbResultLen) PURE;

	// result must be freed using mir_free or assigned to ptrA/ptrT
	STDMETHOD_(char*, decodeString)(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen) PURE;
	STDMETHOD_(void*, decodeBuffer)(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen) PURE;
};

/////////////////////////////////////////////////////////////////////////////////////////
// registers a crypto provider v0.94+
// wParam = (int)hLangpack
// lParam = (CRYPTO_PROVIDER*)
// returns HANDLE on success or NULL on failure

typedef MICryptoEngine* (__cdecl *pfnCryptoProviderFactory)(void);

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

#define MS_CRYPTO_REGISTER_ENGINE "Crypto/RegisterEngine"

__forceinline HANDLE Crypto_RegisterEngine(CRYPTO_PROVIDER *pProvider)
{
	extern int hLangpack;
	return (HANDLE)CallService(MS_CRYPTO_REGISTER_ENGINE, hLangpack, (LPARAM)pProvider);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// retrieves list of all available crypto providers
// wParam = (WPARAM)(int*)piNumProviders
// lParam = (CRYPTO_PROVIDER***)pointer to an array of CRYPTO_PROVIDER*
// always returns 0

#define MS_CRYPTO_ENUM_PROVIDERS "Crypto/EnumProviders"

__forceinline void Crypto_EnumProviders(int *numProvs, CRYPTO_PROVIDER ***pResult)
{
	CallService(MS_CRYPTO_ENUM_PROVIDERS, WPARAM(numProvs), LPARAM(pResult));
}

///////////////////////////////////////////////////////////////////////////////////////////////
// retrieves a crypto provider by name
// wParam = 0 (unused)
// lParam = (const char*)pszProviderName = CRYPTO_PROVIDER::pszName
// returns 0 on fail or CRYPTO_PROVIDER* on success

#define MS_CRYPTO_GET_PROVIDER "Crypto/GetProvider"

__forceinline CRYPTO_PROVIDER* Crypto_GetProvider(const char *pszName)
{
	return (CRYPTO_PROVIDER*)CallService(MS_CRYPTO_GET_PROVIDER, 0, LPARAM(pszName));
}

#endif // M_CRYPTO_H__
