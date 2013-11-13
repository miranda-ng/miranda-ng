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
	return KEYLENGTH;
}

bool CStdCrypt::getKey(BYTE *pKey, size_t cbKeyLen)
{
	if (cbKeyLen < KEYLENGTH)
		return false;

	memcpy(pKey, m_key, sizeof(m_key));
	if (cbKeyLen > KEYLENGTH)
		memset(pKey + KEYLENGTH, 0, cbKeyLen - KEYLENGTH);
	return true;
}

int CStdCrypt::setKey(const BYTE *pKey, size_t cbKeyLen)
{
	if (cbKeyLen > KEYLENGTH)
		return false;

	memcpy(m_key, pKey, cbKeyLen);
	if (cbKeyLen < KEYLENGTH)
		memset(m_key + cbKeyLen, 0, KEYLENGTH - cbKeyLen);
	return 0;
}

void CStdCrypt::generateKey(void)
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	srand((UINT)counter.QuadPart);
	for (int i = 0; i < sizeof(m_key); i++)
		m_key[i] = (BYTE)rand();
}

void CStdCrypt::purgeKey(void)
{
	memset(m_key, 0, sizeof(m_key));
}

// sets the master password (in utf-8)
void CStdCrypt::setPassword(const char *pszPassword)
{
	m_password = pszPassword;
}

// result must be freed using mir_free or assigned to mir_ptr<BYTE>
BYTE* CStdCrypt::encodeString(const char *src, size_t *cbResultLen)
{
	return 0;
}

BYTE* CStdCrypt::encodeStringW(const WCHAR* src, size_t *cbResultLen)
{
	return 0;
}

char* CStdCrypt::decodeString(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	return 0;
}

WCHAR* CStdCrypt::decodeStringW(const BYTE *pBuf, size_t bufLen, size_t *cbResultLen)
{
	return 0;
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
