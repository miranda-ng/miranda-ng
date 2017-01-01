/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-17 Miranda NG project,
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

#include "../../../libs/zlib/src/zlib.h"

const BYTE iv0[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct ExternalKey
{
	BYTE  m_key[KEY_LENGTH];
	BYTE  m_iv[KEY_LENGTH];
	DWORD m_crc32;
	BYTE  slack[BLOCK_SIZE - sizeof(DWORD)];
};

CCrypt::CCrypt() :
	m_password("Miranda")
{}

CCrypt::~CCrypt()
{
	if (m_valid)
	{
		free_ecb(m_ctx);
		getRandomBytes(m_key, KEY_LENGTH);
	}
}

void CCrypt::destroy()
{
	delete this;
}

size_t CCrypt::getKeyLength()
{
	return sizeof(ExternalKey);
}

bool CCrypt::getKey(BYTE *pKey, size_t cbKeyLen)
{
	if (!m_valid || cbKeyLen < sizeof(ExternalKey))
		return false;

	ExternalKey tmp = { 0 };
	memcpy(&tmp.m_key, m_key, KEY_LENGTH);
	memcpy(&tmp.m_iv, m_iv, KEY_LENGTH);
	tmp.m_crc32 = crc32(0xAbbaDead, (LPCBYTE)m_password.GetString(), m_password.GetLength());
	getRandomBytes(tmp.slack, sizeof(tmp.slack));

	BYTE tmpHash[32];
	slow_hash(m_password, m_password.GetLength(), tmpHash);

	BYTE ctx[kCbc14ContextLen];
	init_cbc_14(tmpHash, ctx, iv0, _countof(iv0));
	bool val = !encrypt_cbc(ctx, (BYTE*)&tmp, pKey, cbKeyLen);
	free_cbc(ctx);

	return val;
}

bool CCrypt::setKey(const BYTE *pKey, size_t cbKeyLen)
{
	// full external key. decode & check password
	if (cbKeyLen != sizeof(ExternalKey))
		return false;
		
	BYTE tmpHash[32];
	slow_hash(m_password, m_password.GetLength(), tmpHash);


	BYTE ctx[kCbc14ContextLen];
	init_cbc_14(tmpHash, ctx, iv0, _countof(iv0));

	ExternalKey tmp = { 0 };

	decrypt_cbc(ctx, (BYTE*)pKey, (BYTE*)&tmp, sizeof(tmp));

	free_cbc(ctx);

	if (tmp.m_crc32 != crc32(0xAbbaDead, (LPCBYTE)m_password.GetString(), m_password.GetLength()))
		return false;

	memcpy(m_key, &tmp.m_key, KEY_LENGTH);
	memcpy(m_iv, &tmp.m_iv, KEY_LENGTH);
	init_cbc_14(m_key, m_ctx, m_iv, KEY_LENGTH);

	return m_valid = true;
}

bool CCrypt::generateKey(void)
{
	BYTE tmp[KEY_LENGTH], tmpiv[KEY_LENGTH];
	if (!getRandomBytes(tmp, sizeof(tmp)) || !getRandomBytes(tmpiv, sizeof(tmpiv)))
		return false;

	memcpy(m_key, tmp, KEY_LENGTH);
	memcpy(m_iv, tmpiv, KEY_LENGTH);
	init_cbc_14(m_key, m_ctx, m_iv, _countof(m_iv));

	SecureZeroMemory(tmp, _countof(tmp)); SecureZeroMemory(tmpiv, _countof(tmpiv));

	return m_valid = true;
}

void CCrypt::purgeKey(void)
{
	memset(m_key, 0, sizeof(m_key));
	m_valid = false;
}

// checks the master password (in utf-8)
bool CCrypt::checkPassword(const char *pszPassword)
{
	return m_password == pszPassword;
}

// sets the master password (in utf-8)
void CCrypt::setPassword(const char *pszPassword)
{
	m_password = (pszPassword == NULL) ? "Miranda" : pszPassword;
}

// result must be freed using mir_free or assigned to mir_ptr<BYTE>
BYTE* CCrypt::encodeString(const char *src, size_t *cbResultLen)
{
	if (!m_valid || src == NULL) {
		if (cbResultLen)
			*cbResultLen = 0;
		return NULL;
	}

	return encodeBuffer(src, mir_strlen(src)+1, cbResultLen);
}

BYTE* CCrypt::encodeBuffer(const void *src, size_t cbLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || src == NULL || cbLen >= 0xFFFE)
		return NULL;

	BYTE *tmpBuf = (BYTE*)_alloca(cbLen + 2);
	*(PWORD)tmpBuf = (WORD)cbLen;
	memcpy(tmpBuf + 2, src, cbLen);
	cbLen += 2;
	size_t rest = cbLen % BLOCK_SIZE;
	if (rest)
		cbLen += BLOCK_SIZE - rest;

	BYTE *result = (BYTE*)mir_alloc(cbLen);

	if (encrypt_cbc(m_ctx, tmpBuf, result, cbLen)) {
		mir_free(result);
		return NULL;
	}

	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

char* CCrypt::decodeString(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	size_t resLen;
	char *result = (char*)decodeBuffer(pBuf, bufLen, &resLen);
	if (result) {
		if (result[resLen-1] != 0) { // smth went wrong
			mir_free(result);
			return NULL;
		}
	}

	if (cbResultLen)
		*cbResultLen = resLen;
	return result;
}

void* CCrypt::decodeBuffer(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || pBuf == NULL || (bufLen % BLOCK_SIZE) != 0)
		return NULL;

	char *result = (char*)mir_alloc(bufLen + 1);

	if (decrypt_cbc(m_ctx, LPBYTE(pBuf), (BYTE*)result, bufLen)) {
		mir_free(result);
		return NULL;
	}

	result[bufLen] = 0;
	WORD cbLen = *(PWORD)result;
	if (cbLen > bufLen) {
		mir_free(result);
		return NULL;
	}

	memmove(result, result + 2, cbLen);
	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

static MICryptoEngine* __cdecl builder()
{
	return new CCrypt();
}

int LoadEncryptionModule(void)
{
	CRYPTO_PROVIDER cp = { sizeof(cp) };
	cp.pszName = "Kuznechik";
	cp.pszDescr = LPGEN("Kuznechik (GOST P 34.12-2015) crypto provider");
	cp.pFactory = builder;
	Crypto_RegisterEngine(&cp);
	return 0;
}
