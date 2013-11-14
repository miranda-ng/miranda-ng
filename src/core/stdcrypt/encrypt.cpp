/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-13 Miranda NG project,
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

CStdCrypt::CStdCrypt() :
	m_password("Miranda")
{}

void CStdCrypt::destroy()
{
	delete this;
}

size_t CStdCrypt::getKeyLength()
{
	return KEY_LENGTH;
}

bool CStdCrypt::getKey(BYTE *pKey, size_t cbKeyLen)
{
	if (!m_valid || cbKeyLen < KEY_LENGTH)
		return false;

	memcpy(pKey, m_key, sizeof(m_key));
	if (cbKeyLen > KEY_LENGTH)
		memset(pKey + KEY_LENGTH, 0, cbKeyLen - KEY_LENGTH);
	return true;
}

int CStdCrypt::setKey(const BYTE *pKey, size_t cbKeyLen)
{
	if (cbKeyLen > KEY_LENGTH)
		return false;

	memcpy(m_key, pKey, cbKeyLen);
	if (cbKeyLen < KEY_LENGTH)
		memset(m_key + cbKeyLen, 0, KEY_LENGTH - cbKeyLen);

	m_aes.MakeKey(m_key, m_password, KEY_LENGTH, BLOCK_SIZE);
	m_valid = true;
	return 0;
}

void CStdCrypt::generateKey(void)
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	srand(counter.LowPart);

	for (int i = 0; i < sizeof(m_key); i++) {
		m_key[i] = (BYTE)rand();
		Sleep(0);
	}

	m_aes.MakeKey(m_key, m_password, KEY_LENGTH, BLOCK_SIZE);
	m_valid = true;
}

void CStdCrypt::purgeKey(void)
{
	memset(m_key, 0, sizeof(m_key));
	m_valid = false;
}

// sets the master password (in utf-8)
void CStdCrypt::setPassword(const char *pszPassword)
{
	m_password = pszPassword;
}

// result must be freed using mir_free or assigned to mir_ptr<BYTE>
BYTE* CStdCrypt::encodeString(const char *src, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || src == NULL || *src == 0)
		return NULL;

	size_t cbLen = strlen(src);
	cbLen += BLOCK_SIZE - (cbLen % BLOCK_SIZE);
	BYTE *result = (BYTE*)mir_alloc(cbLen);
	if (m_aes.Encrypt(src, LPSTR(result), cbLen)) {
		mir_free(result);
		return NULL;
	}

	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

BYTE* CStdCrypt::encodeStringW(const WCHAR *src, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || src == NULL || *src == 0)
		return NULL;

	size_t cbLen = wcslen(src) * sizeof(WCHAR);
	cbLen += BLOCK_SIZE - (cbLen % BLOCK_SIZE);
	BYTE *result = (BYTE*)mir_alloc(cbLen);
	if (m_aes.Encrypt(LPCSTR(src), LPSTR(result), cbLen)) {
		mir_free(result);
		return NULL;
	}

	if (cbResultLen)
		*cbResultLen = cbLen;
	return result;
}

char* CStdCrypt::decodeString(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || pBuf == NULL || (bufLen % BLOCK_SIZE) != 0)
		return NULL;

	char *result = (char*)mir_alloc(bufLen+1);
	if (m_aes.Decrypt(LPCSTR(pBuf), result, bufLen)) {
		mir_free(result);
		return NULL;
	}

	result[bufLen] = 0;
	for (int i = (int)bufLen-1; i >= 0; i--)
		if (result[i] == 0)
			bufLen--;

	if (cbResultLen)
		*cbResultLen = bufLen;
	return result;
}

WCHAR* CStdCrypt::decodeStringW(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	if (cbResultLen)
		*cbResultLen = 0;

	if (!m_valid || pBuf == NULL || (bufLen % BLOCK_SIZE) != 0)
		return NULL;

	WCHAR *result = (WCHAR*)mir_alloc(bufLen + sizeof(WCHAR));
	if (m_aes.Decrypt(LPCSTR(pBuf), LPSTR(result), bufLen)) {
		mir_free(result);
		return NULL;
	}

	bufLen /= sizeof(WCHAR);
	result[bufLen] = 0;
	for (int i = (int)bufLen - 1; i >= 0; i--)
		if (result[i] == 0)
			bufLen--;

	if (cbResultLen)
		*cbResultLen = bufLen;
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
