/*

Standard encryption plugin for Miranda NG
Copyright (C) 2012-22 George Hazan

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

#pragma once

#include "Rijndael.h"

// we use 256-bit keys & 128-bit blocks
#define KEY_LENGTH 32
#define BLOCK_SIZE 16

struct ExternalKey
{
	uint8_t  m_key[KEY_LENGTH];
	uint32_t m_crc32;
	uint8_t  slack[BLOCK_SIZE - sizeof(uint32_t)];
};

struct CStdCrypt : public MICryptoEngine, public MZeroedObject
{
	CStdCrypt();

	bool      m_valid = false;
	uint8_t   m_key[KEY_LENGTH];
	CRijndael m_aes;
	ExternalKey m_extKey;

	bool checkKey(const char *pszPassword, const ExternalKey *pPublic, ExternalKey &key);
	void key2ext(const char *pszPassword, ExternalKey &key);

	STDMETHODIMP_(void) destroy();

	// get/set the instance key
	STDMETHODIMP_(size_t) getKeyLength(void) override;
	STDMETHODIMP_(bool) getKey(uint8_t *pKey, size_t cbKeyLen) override;
	STDMETHODIMP_(bool) setKey(const char *pszPassword, const uint8_t *pKey, size_t cbKeyLen) override;

	STDMETHODIMP_(bool) generateKey(void) override; // creates a new key inside
	STDMETHODIMP_(void) purgeKey(void) override;    // purges a key from memory

	// sets the master password (in utf-8)
	STDMETHODIMP_(void) setPassword(const char *pszPassword);
	STDMETHODIMP_(bool) checkPassword(const char *pszPassword) override;

	// result must be freed using mir_free or assigned to mir_ptr<uint8_t>
	STDMETHODIMP_(uint8_t*) encodeString(const char *src, size_t *cbResultLen) override;
	STDMETHODIMP_(uint8_t*) encodeBuffer(const void *src, size_t cbLen, size_t *cbResultLen) override;

	// result must be freed using mir_free or assigned to ptrA/ptrW
	STDMETHODIMP_(char*) decodeString(const uint8_t *pBuf, size_t bufLen, size_t *cbResultLen) override;
	STDMETHODIMP_(void*) decodeBuffer(const uint8_t *pBuf, size_t bufLen, size_t *cbResultLen) override;
};
