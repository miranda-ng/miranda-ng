/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

CStdCrypt::CStdCrypt()
{
}

void CStdCrypt::destroy()
{
	delete this;
}

size_t CStdCrypt::getKeyLength()
{
	return sizeof(ExternalKey);
}

void CStdCrypt::key2ext(const char *pszPassword, ExternalKey &key)
{
	if (mir_strlen(pszPassword) == 0)
		pszPassword = "Miranda";

	ExternalKey tmp = {};
	memcpy(&tmp.m_key, m_key, KEY_LENGTH);
	tmp.m_crc32 = crc32(0xAbbaDead, (LPCBYTE)pszPassword, (int)strlen(pszPassword));
	getRandomBytes(tmp.slack, sizeof(tmp.slack));

	uint8_t tmpHash[32];
	slow_hash(pszPassword, strlen(pszPassword), tmpHash);

	CRijndael tmpAes;
	tmpAes.MakeKey(tmpHash, tmpAes.sm_chain0, KEY_LENGTH, BLOCK_SIZE);
	tmpAes.Encrypt(&tmp, &key, sizeof(ExternalKey));
}

bool CStdCrypt::getKey(uint8_t *pKey, size_t cbKeyLen)
{
	if (!m_valid || cbKeyLen < sizeof(m_extKey))
		return false;

	memcpy(pKey, &m_extKey, sizeof(m_extKey));
	return true;
}

bool CStdCrypt::checkKey(const char *pszPassword, const ExternalKey *pPublic, ExternalKey &tmp)
{
	if (mir_strlen(pszPassword) == 0)
		pszPassword = "Miranda";

	uint8_t tmpHash[32];
	slow_hash(pszPassword, strlen(pszPassword), tmpHash);

	CRijndael tmpAes;
	tmpAes.MakeKey(tmpHash, tmpAes.sm_chain0, KEY_LENGTH, BLOCK_SIZE);

	tmpAes.Decrypt(pPublic, &tmp, sizeof(tmp));
	return (tmp.m_crc32 == crc32(0xAbbaDead, (LPCBYTE)pszPassword, (int)strlen(pszPassword)));
}

bool CStdCrypt::setKey(const char *pszPassword, const uint8_t *pPublic, size_t cbKeyLen)
{
	// full external key. decode & check password
	if (cbKeyLen != sizeof(m_extKey))
		return false;

	ExternalKey tmp = {};
	if (!checkKey(pszPassword, (const ExternalKey*)pPublic, tmp))
		return false;

	memcpy(&m_extKey, pPublic, sizeof(m_extKey));
	memcpy(m_key, &tmp.m_key, KEY_LENGTH);
	m_aes.MakeKey(m_key, "Miranda", KEY_LENGTH, BLOCK_SIZE);
	return m_valid = true;
}

bool CStdCrypt::generateKey(void)
{
	uint8_t tmp[KEY_LENGTH];
	if (!getRandomBytes(tmp, sizeof(tmp)))
		return false;

	memcpy(m_key, tmp, KEY_LENGTH);
	m_aes.MakeKey(m_key, "Miranda", KEY_LENGTH, BLOCK_SIZE);
	key2ext(nullptr, m_extKey);
	return m_valid = true;
}

void CStdCrypt::purgeKey(void)
{
	memset(m_key, 0, sizeof(m_key));
	m_valid = false;
}

// checks the master password (in utf-8)
bool CStdCrypt::checkPassword(const char *pszPassword)
{
	ExternalKey tmp;
	return checkKey(pszPassword, &m_extKey, tmp);
}

void CStdCrypt::setPassword(const char *pszPassword)
{
	key2ext(pszPassword, m_extKey);
}

// result must be freed using mir_free or assigned to mir_ptr<uint8_t>
uint8_t* CStdCrypt::encodeString(const char *src, size_t *cbResultLen)
{
	if (!m_valid || src == nullptr) {
		if (cbResultLen)
			*cbResultLen = 0;
		return nullptr;
	}

	return encodeBuffer(src, mir_strlen(src)+1, cbResultLen);
}

uint8_t* CStdCrypt::encodeBuffer(const void *src, size_t cbLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || src == nullptr || cbLen >= 0xFFFE)
		return nullptr;

	uint8_t *tmpBuf = (uint8_t*)_alloca(cbLen + 2);
	*(PWORD)tmpBuf = (uint16_t)cbLen;
	memcpy(tmpBuf + 2, src, cbLen);
	cbLen += 2;
	size_t rest = cbLen % BLOCK_SIZE;
	if (rest)
		cbLen += BLOCK_SIZE - rest;

	uint8_t *result = (uint8_t*)mir_alloc(cbLen);
	m_aes.ResetChain();
	if (m_aes.Encrypt(tmpBuf, LPSTR(result), cbLen)) {
		mir_free(result);
		return nullptr;
	}

	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

char* CStdCrypt::decodeString(const uint8_t *pBuf, size_t bufLen, size_t *cbResultLen)
{
	size_t resLen;
	char *result = (char*)decodeBuffer(pBuf, bufLen, &resLen);
	if (result) {
		if (result[resLen-1] != 0) { // smth went wrong
			mir_free(result);
			return nullptr;
		}
	}

	if (cbResultLen)
		*cbResultLen = resLen;
	return result;
}

void* CStdCrypt::decodeBuffer(const uint8_t *pBuf, size_t bufLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || pBuf == nullptr || (bufLen % BLOCK_SIZE) != 0)
		return nullptr;

	char *result = (char*)mir_alloc(bufLen + 1);
	m_aes.ResetChain();
	if (m_aes.Decrypt(LPCSTR(pBuf), result, bufLen)) {
		mir_free(result);
		return nullptr;
	}

	result[bufLen] = 0;
	uint16_t cbLen = *(PWORD)result;
	if (cbLen > bufLen) {
		mir_free(result);
		return nullptr;
	}

	memmove(result, result + 2, cbLen);
	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

static MICryptoEngine* __cdecl builder()
{
	return new CStdCrypt();
}

int LoadEncryptionModule(void)
{
	CRYPTO_PROVIDER cp = { sizeof(cp) };
	cp.pszName = "AES (Rjindale)";
	cp.szDescr.a = LPGEN("Standard crypto provider");
	cp.pFactory = builder;
	cp.pPlugin = &g_plugin;
	Crypto_RegisterEngine(&cp);
	return 0;
}
