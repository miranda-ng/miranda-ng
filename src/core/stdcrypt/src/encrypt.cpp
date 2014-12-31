/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
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

#include "commonheaders.h"

#include "..\..\..\plugins\zlib\src\zlib.h"

struct ExternalKey
{
	BYTE  m_key[KEY_LENGTH];
	DWORD m_crc32;
	BYTE  slack[BLOCK_SIZE - sizeof(DWORD)];
};

CStdCrypt::CStdCrypt() :
	m_password("Miranda")
{}

void CStdCrypt::destroy()
{
	delete this;
}

size_t CStdCrypt::getKeyLength()
{
	return sizeof(ExternalKey);
}

bool CStdCrypt::getKey(BYTE *pKey, size_t cbKeyLen)
{
	if (!m_valid || cbKeyLen < sizeof(ExternalKey))
		return false;

	ExternalKey tmp = { 0 };
	memcpy(&tmp.m_key, m_key, KEY_LENGTH);
	tmp.m_crc32 = crc32(0xAbbaDead, (LPCBYTE)m_password.GetString(), m_password.GetLength());
	getRandomBytes(tmp.slack, sizeof(tmp.slack));

	BYTE tmpHash[32];
	slow_hash(m_password, m_password.GetLength(), tmpHash);

	CRijndael tmpAes;
	tmpAes.MakeKey(tmpHash, tmpAes.sm_chain0, KEY_LENGTH, BLOCK_SIZE);
	tmpAes.Encrypt(&tmp, pKey, sizeof(tmp));
	return true;
}

bool CStdCrypt::setKey(const BYTE *pKey, size_t cbKeyLen)
{
	// full external key. decode & check password
	if (cbKeyLen != sizeof(ExternalKey))
		return false;
		
	BYTE tmpHash[32];
	slow_hash(m_password, m_password.GetLength(), tmpHash);

	CRijndael tmpAes;
	tmpAes.MakeKey(tmpHash, tmpAes.sm_chain0, KEY_LENGTH, BLOCK_SIZE);

	ExternalKey tmp = { 0 };
	tmpAes.Decrypt(pKey, &tmp, sizeof(tmp));
	if (tmp.m_crc32 != crc32(0xAbbaDead, (LPCBYTE)m_password.GetString(), m_password.GetLength()))
		return false;

	memcpy(m_key, &tmp.m_key, KEY_LENGTH);
	m_aes.MakeKey(m_key, "Miranda", KEY_LENGTH, BLOCK_SIZE);
	return m_valid = true;
}

bool CStdCrypt::generateKey(void)
{
	BYTE tmp[KEY_LENGTH];
	if (!getRandomBytes(tmp, sizeof(tmp)))
		return false;

	memcpy(m_key, tmp, KEY_LENGTH);
	m_aes.MakeKey(m_key, "Miranda", KEY_LENGTH, BLOCK_SIZE);
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
	return m_password == pszPassword;
}

// sets the master password (in utf-8)
void CStdCrypt::setPassword(const char *pszPassword)
{
	m_password = (pszPassword == NULL) ? "Miranda" : pszPassword;
}

// result must be freed using mir_free or assigned to mir_ptr<BYTE>
BYTE* CStdCrypt::encodeString(const char *src, size_t *cbResultLen)
{
	if (!m_valid || src == NULL) {
		if (cbResultLen)
			*cbResultLen = 0;
		return NULL;
	}

	return encodeBuffer(src, strlen(src)+1, cbResultLen);
}

BYTE* CStdCrypt::encodeBuffer(const void *src, size_t cbLen, size_t *cbResultLen)
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
	m_aes.ResetChain();
	if (m_aes.Encrypt(tmpBuf, LPSTR(result), cbLen)) {
		mir_free(result);
		return NULL;
	}

	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

char* CStdCrypt::decodeString(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
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

void* CStdCrypt::decodeBuffer(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || pBuf == NULL || (bufLen % BLOCK_SIZE) != 0)
		return NULL;

	char *result = (char*)mir_alloc(bufLen + 1);
	m_aes.ResetChain();
	if (m_aes.Decrypt(LPCSTR(pBuf), result, bufLen)) {
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
	return new CStdCrypt();
}

int LoadEncryptionModule(void)
{
	CRYPTO_PROVIDER cp = { sizeof(cp) };
	cp.pszName = "AES (Rjindale)";
	cp.pszDescr = LPGEN("Standard crypto provider");
	cp.pFactory = builder;
	Crypto_RegisterEngine(&cp);
	return 0;
}
