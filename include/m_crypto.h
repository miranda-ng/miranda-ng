/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

#ifndef M_CORE_H__
#include <m_core.h>
#endif

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
// wParam = 0
// lParam = (CRYPTO_PROVIDER*)
// returns HANDLE on success or NULL on failure

typedef MICryptoEngine* (__cdecl *pfnCryptoProviderFactory)(void);

#define CPF_UNICODE 1

struct CRYPTO_PROVIDER
{
	DWORD	dwSize;
	DWORD	dwFlags; // one of CPF_* constants
	HPLUGIN pPlugin;

	char *pszName; // unique id
	MAllStrings szDescr;  // description

	pfnCryptoProviderFactory pFactory;
};

MIR_APP_DLL(void) Crypto_RegisterEngine(const CRYPTO_PROVIDER *pProvider);

///////////////////////////////////////////////////////////////////////////////////////////////
// retrieves list of all available crypto providers

MIR_APP_DLL(void) Crypto_ListProviders(int *pCount, CRYPTO_PROVIDER ***pList);

///////////////////////////////////////////////////////////////////////////////////////////////
// retrieves a crypto provider by name

MIR_APP_DLL(CRYPTO_PROVIDER*) Crypto_GetProvider(const char *pszName);

#endif // M_CRYPTO_H__
