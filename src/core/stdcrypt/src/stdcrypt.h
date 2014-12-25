/*

Standard encryption plugin for Myranda NG
Copyright (C) 2012-13 George Hazan

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "Rijndael.h"

// we use 256-bit keys & 128-bit blocks
#define KEY_LENGTH 32
#define BLOCK_SIZE 16

struct CStdCrypt : public MICryptoEngine, public MZeroedObject
{
	CStdCrypt();

	BOOL      m_valid;
	CMStringA m_password;

	BYTE      m_key[KEY_LENGTH];
	CRijndael m_aes;

	STDMETHODIMP_(void) destroy();

	// get/set the instance key
	STDMETHODIMP_(size_t) getKeyLength(void);
	STDMETHODIMP_(bool) getKey(BYTE *pKey, size_t cbKeyLen);
	STDMETHODIMP_(bool) setKey(const BYTE *pKey, size_t cbKeyLen);

	STDMETHODIMP_(bool) generateKey(void); // creates a new key inside
	STDMETHODIMP_(void) purgeKey(void);    // purges a key from memory

	// sets the master password (in utf-8)
	STDMETHODIMP_(bool) checkPassword(const char *pszPassword);
	STDMETHODIMP_(void) setPassword(const char *pszPassword);

	// result must be freed using mir_free or assigned to mir_ptr<BYTE>
	STDMETHODIMP_(BYTE*) encodeString(const char *src, size_t *cbResultLen);
	STDMETHODIMP_(BYTE*) encodeBuffer(const void *src, size_t cbLen, size_t *cbResultLen);

	// result must be freed using mir_free or assigned to ptrA/ptrT
	STDMETHODIMP_(char*) decodeString(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen);
	STDMETHODIMP_(void*) decodeBuffer(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen);
};
