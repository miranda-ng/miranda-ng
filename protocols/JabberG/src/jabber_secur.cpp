/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2012-13  Miranda NG Project

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

#include "jabber.h"
#include "jabber_secur.h"

typedef BYTE (WINAPI *GetUserNameExType)(int NameFormat, LPTSTR lpNameBuffer, PULONG nSize);


/////////////////////////////////////////////////////////////////////////////////////////
// ntlm auth - LanServer based authorization

TNtlmAuth::TNtlmAuth(ThreadData* info, const char* mechanism, const TCHAR *hostname) :
	TJabberAuth(info)
{
	szName = mechanism;
	szHostName = hostname;

	const TCHAR *szProvider;
	if ( !strcmp(mechanism, "GSS-SPNEGO"))
		szProvider = _T("Negotiate");
	else if ( !strcmp(mechanism, "GSSAPI"))
		szProvider = _T("GSSAPI");
	else if ( !strcmp(mechanism, "NTLM"))
		szProvider = _T("NTLM");
	else {
		bIsValid = false;
		return;
	}

	TCHAR szSpn[ 1024 ] = _T("");
	if (strcmp(mechanism, "NTLM")) {
		if ( !getSpn(szSpn, SIZEOF(szSpn)) && !strcmp(mechanism, "GSSAPI")) {
			bIsValid = false;
			return;
	}	}

	if ((hProvider = Netlib_InitSecurityProvider2(szProvider, szSpn)) == NULL)
		bIsValid = false;
}

TNtlmAuth::~TNtlmAuth()
{
	if (hProvider != NULL)
		Netlib_DestroySecurityProvider(NULL, hProvider);
}

bool TNtlmAuth::getSpn(TCHAR* szSpn, size_t dwSpnLen)
{
	HMODULE hDll = GetModuleHandle(_T("secur32.dll"));
	if ( !hDll)
		hDll = LoadLibrary(_T("secur32.dll"));
	if ( !hDll)
		return false;

	GetUserNameExType myGetUserNameEx =
		(GetUserNameExType)GetProcAddress(hDll, "GetUserNameExW");

	if ( !myGetUserNameEx) return false;

	TCHAR szFullUserName[128] = _T("");
	ULONG szFullUserNameLen = SIZEOF(szFullUserName);
	if ( !myGetUserNameEx(12, szFullUserName, &szFullUserNameLen)) {
		szFullUserName[ 0 ] = 0;
		szFullUserNameLen = SIZEOF(szFullUserName);
		myGetUserNameEx(2, szFullUserName, &szFullUserNameLen);
	}

	TCHAR *name = _tcsrchr(szFullUserName, '\\');
	if (name) *name = 0;
	else return false;

	if (szHostName && szHostName[0]) {
		TCHAR *szFullUserNameU = _tcsupr(mir_tstrdup(szFullUserName));
		mir_sntprintf(szSpn, dwSpnLen, _T("xmpp/%s/%s@%s"), szHostName, szFullUserName, szFullUserNameU);
		mir_free(szFullUserNameU);
	} else {
		const char* connectHost = info->manualHost[0] ? info->manualHost : info->server;

		 unsigned long ip = inet_addr(connectHost);
		// Convert host name to FQDN
//		PHOSTENT host = (ip == INADDR_NONE) ? gethostbyname(szHost) : gethostbyaddr((char*)&ip, 4, AF_INET);
		PHOSTENT host = (ip == INADDR_NONE) ? NULL : gethostbyaddr((char*)&ip, 4, AF_INET);
		if (host && host->h_name)
			connectHost = host->h_name;

		TCHAR *connectHostT = mir_a2t(connectHost);
		mir_sntprintf(szSpn, dwSpnLen, _T("xmpp/%s@%s"), connectHostT, _tcsupr(szFullUserName));
		mir_free(connectHostT);
	}

	Netlib_Logf(NULL, "SPN: %S", szSpn);


	return true;
}

char* TNtlmAuth::getInitialRequest()
{
	if ( !hProvider)
		return NULL;

	// This generates login method advertisement packet
	char* result;
	if (info->password[0] != 0)
		result = Netlib_NtlmCreateResponse2(hProvider, "", info->username, info->password, &complete);
	else
		result = Netlib_NtlmCreateResponse2(hProvider, "", NULL, NULL, &complete);

	return result;
}

char* TNtlmAuth::getChallenge(const TCHAR *challenge)
{
	if ( !hProvider)
		return NULL;

	char *text = (!lstrcmp(challenge, _T("="))) ? mir_strdup("") : mir_t2a(challenge), *result;
	if (info->password[0] != 0)
		result = Netlib_NtlmCreateResponse2(hProvider, text, info->username, info->password, &complete);
	else
		result = Netlib_NtlmCreateResponse2(hProvider, text, NULL, NULL, &complete);

	mir_free(text);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// md5 auth - digest-based authorization

TMD5Auth::TMD5Auth(ThreadData* info) :
	TJabberAuth(info),
	iCallCount(0)
{
	szName = "DIGEST-MD5";
}

TMD5Auth::~TMD5Auth()
{
}

char* TMD5Auth::getChallenge(const TCHAR *challenge)
{
	if (iCallCount > 0)
		return NULL;

	iCallCount++;

	unsigned resultLen;
	ptrA text((char*)mir_base64_decode( _T2A(challenge), &resultLen));

	TStringPairs pairs(text);
	const char *realm = pairs["realm"], *nonce = pairs["nonce"];

	char cnonce[40], tmpBuf[40];
	DWORD digest[4], hash1[4], hash2[4];
	mir_md5_state_t ctx;

	CallService(MS_UTILS_GETRANDOM, sizeof(digest), (LPARAM)digest);
	mir_snprintf(cnonce, SIZEOF(cnonce), "%08x%08x%08x%08x", htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	ptrA uname( mir_utf8encodeT(info->username)),
	     passw( mir_utf8encodeT(info->password)),
	     serv( mir_utf8encode(info->server));

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)(char*)uname, (int)strlen(uname));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)realm, (int)strlen(realm));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)(char*)passw,  (int)strlen(passw));
	mir_md5_finish(&ctx, (BYTE*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)hash1, 16);
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)nonce,  (int)strlen(nonce));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)cnonce, (int)strlen(cnonce));
	mir_md5_finish(&ctx, (BYTE*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)"AUTHENTICATE:xmpp/", 18);
	mir_md5_append(&ctx, (BYTE*)(char*)serv, (int)strlen(serv));
	mir_md5_finish(&ctx, (BYTE*)hash2);

	mir_md5_init(&ctx);
	mir_snprintf(tmpBuf, SIZEOF(tmpBuf), "%08x%08x%08x%08x", htonl(hash1[0]), htonl(hash1[1]), htonl(hash1[2]), htonl(hash1[3]));
	mir_md5_append(&ctx, (BYTE*)tmpBuf, (int)strlen(tmpBuf));
	mir_md5_append(&ctx, (BYTE*)":",    1);
	mir_md5_append(&ctx, (BYTE*)nonce,  (int)strlen(nonce));
	mir_snprintf(tmpBuf, SIZEOF(tmpBuf), ":%08d:", iCallCount);
	mir_md5_append(&ctx, (BYTE*)tmpBuf, (int)strlen(tmpBuf));
	mir_md5_append(&ctx, (BYTE*)cnonce, (int)strlen(cnonce));
	mir_md5_append(&ctx, (BYTE*)":auth:", 6);
	mir_snprintf(tmpBuf, SIZEOF(tmpBuf), "%08x%08x%08x%08x", htonl(hash2[0]), htonl(hash2[1]), htonl(hash2[2]), htonl(hash2[3]));
	mir_md5_append(&ctx, (BYTE*)tmpBuf, (int)strlen(tmpBuf));
	mir_md5_finish(&ctx, (BYTE*)digest);

	char *buf = (char*)alloca(8000);
	int cbLen = mir_snprintf(buf, 8000,
		"username=\"%s\",realm=\"%s\",nonce=\"%s\",cnonce=\"%s\",nc=%08d,"
		"qop=auth,digest-uri=\"xmpp/%s\",charset=utf-8,response=%08x%08x%08x%08x",
		uname, realm, nonce, cnonce, iCallCount, serv,
		htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

   return mir_base64_encode((PBYTE)buf, cbLen);
}


/////////////////////////////////////////////////////////////////////////////////////////
// SCRAM-SHA-1 authorization

void hmac_sha1(mir_sha1_byte_t *md, mir_sha1_byte_t *key, size_t keylen, mir_sha1_byte_t *text, size_t textlen)
{
	const unsigned SHA_BLOCKSIZE = 64;

	unsigned char mdkey[MIR_SHA1_HASH_SIZE];
	unsigned char k_ipad[SHA_BLOCKSIZE], k_opad[SHA_BLOCKSIZE];
	mir_sha1_ctx ctx;

	if (keylen > SHA_BLOCKSIZE)
	{
		mir_sha1_init(&ctx);
		mir_sha1_append(&ctx, key, (int)keylen);
		mir_sha1_finish(&ctx, mdkey);
		keylen = 20;
		key = mdkey;
	}

	memcpy(k_ipad, key, keylen);
	memcpy(k_opad, key, keylen);
	memset(k_ipad+keylen, 0x36, SHA_BLOCKSIZE - keylen);
	memset(k_opad+keylen, 0x5c, SHA_BLOCKSIZE - keylen);

	for (unsigned i = 0; i < keylen; i++)
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, k_ipad, SHA_BLOCKSIZE);
	mir_sha1_append(&ctx, text, (int)textlen);
	mir_sha1_finish(&ctx, md);

	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, k_opad, SHA_BLOCKSIZE);
	mir_sha1_append(&ctx, md, MIR_SHA1_HASH_SIZE);
	mir_sha1_finish(&ctx, md);
}

TScramAuth::TScramAuth(ThreadData* info) :
	TJabberAuth(info)
{
	szName = "SCRAM-SHA-1";
	cnonce = msg1 = serverSignature = NULL;
}

TScramAuth::~TScramAuth()
{
	mir_free(cnonce);
	mir_free(msg1);
	mir_free(serverSignature);
}

void TScramAuth::Hi(mir_sha1_byte_t* res , char* passw, size_t passwLen, char* salt, size_t saltLen, int ind)
{
	mir_sha1_byte_t u[ MIR_SHA1_HASH_SIZE ];
	memcpy(u, salt,  saltLen); *(unsigned*)(u + saltLen) = htonl(1); saltLen += 4;
	memset(res, 0, MIR_SHA1_HASH_SIZE);

	for (int i = 0; i < ind; i++)
	{
		hmac_sha1(u, (mir_sha1_byte_t*)passw, passwLen, u, saltLen);
		saltLen = sizeof(u);

		for (unsigned j = 0; j < sizeof(u); j++)
			res[j] ^= u[j];
	}
}

char* TScramAuth::getChallenge(const TCHAR *challenge)
{
	unsigned chlLen;
	ptrA chl((char*)mir_base64_decode( _T2A(challenge), &chlLen));

	char *r = strstr(chl, "r=");
	if ( !r)
		return NULL;

	char *e = strchr(r, ','); if (e) *e = 0;
	ptrA snonce( mir_strdup(r + 2));
	if (e) *e = ',';

	size_t cnlen = strlen(cnonce);
	if (strncmp(cnonce, snonce, cnlen))
		return NULL;

	char *s = strstr(chl, "s=");
	if (!s)
		return NULL;
	e = strchr(s, ','); if (e) *e = 0;

	unsigned saltLen;
	ptrA salt((char*)mir_base64_decode(s + 2, &saltLen));
	if (e) *e = ',';
	if (saltLen > 16)
		return NULL;

	char *in = strstr(chl, "i=");
	if (!in)
		return NULL;

	e = strchr(in, ','); if (e) *e = 0;
	int ind = atoi(in + 2);
	if (e) *e = ',';

	ptrA passw( mir_utf8encodeT(info->password));
	size_t passwLen = strlen(passw);

	mir_sha1_byte_t saltedPassw[ MIR_SHA1_HASH_SIZE ];
	Hi(saltedPassw, passw, passwLen, salt,  saltLen, ind);

	mir_sha1_byte_t clientKey[ MIR_SHA1_HASH_SIZE ];
	hmac_sha1(clientKey, saltedPassw, sizeof(saltedPassw), (mir_sha1_byte_t*)"Client Key", 10);

	mir_sha1_byte_t storedKey[ MIR_SHA1_HASH_SIZE ];

	mir_sha1_ctx ctx;
	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, clientKey, MIR_SHA1_HASH_SIZE);
	mir_sha1_finish(&ctx, storedKey);

	char authmsg[4096];
	int authmsgLen = mir_snprintf(authmsg, sizeof(authmsg), "%s,%s,c=biws,r=%s", msg1, chl, snonce);

	mir_sha1_byte_t clientSig[ MIR_SHA1_HASH_SIZE ];
	hmac_sha1(clientSig, storedKey, sizeof(storedKey), (mir_sha1_byte_t*)authmsg, authmsgLen);

	mir_sha1_byte_t clientProof[ MIR_SHA1_HASH_SIZE ];
	for (unsigned j = 0; j < sizeof(clientKey); j++)
		clientProof[j] = clientKey[j] ^ clientSig[j];

	/* Calculate the server signature */
	mir_sha1_byte_t serverKey[ MIR_SHA1_HASH_SIZE ];
	hmac_sha1(serverKey, saltedPassw, sizeof(saltedPassw), (mir_sha1_byte_t*)"Server Key", 10);

	mir_sha1_byte_t srvSig[ MIR_SHA1_HASH_SIZE ];
	hmac_sha1(srvSig, serverKey, sizeof(serverKey), (mir_sha1_byte_t*)authmsg, authmsgLen);
	serverSignature = mir_base64_encode((PBYTE)srvSig, sizeof(srvSig));

	char buf[4096];
	ptrA encproof( mir_base64_encode((PBYTE)clientProof, sizeof(clientProof)));
	int cbLen = mir_snprintf(buf, sizeof(buf), "c=biws,r=%s,p=%s", snonce, encproof);
	return mir_base64_encode((PBYTE)buf, cbLen);
}

char* TScramAuth::getInitialRequest()
{
	ptrA uname( mir_utf8encodeT(info->username)), serv( mir_utf8encode(info->server));

	unsigned char nonce[24];
	CallService(MS_UTILS_GETRANDOM, sizeof(nonce), (LPARAM)nonce);
	cnonce = mir_base64_encode((PBYTE)nonce, sizeof(nonce));

	char buf[4096];
	int cbLen = mir_snprintf(buf, sizeof(buf), "n,,n=%s@%s,r=%s", uname, serv, cnonce);
	msg1 = mir_strdup(buf + 3);
	return mir_base64_encode((PBYTE)buf, cbLen);
}

bool TScramAuth::validateLogin(const TCHAR *challenge)
{
	unsigned chlLen;
	ptrA chl((char*)mir_base64_decode( _T2A(challenge), &chlLen));
	return chl && strncmp((char*)chl + 2, serverSignature, chlLen - 2) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plain auth - the most simple one

TPlainAuth::TPlainAuth(ThreadData* info, bool old) :
	TJabberAuth(info)
{
	szName = "PLAIN";
	bOld = old;
}

TPlainAuth::~TPlainAuth()
{
}

char* TPlainAuth::getInitialRequest()
{
	ptrA uname( mir_utf8encodeT(info->username)), passw( mir_utf8encodeT(info->password));

	size_t size = 2 * strlen(uname) + strlen(passw) + strlen(info->server) + 4;
	char *toEncode = (char*)alloca(size);
	if (bOld)
		size = mir_snprintf(toEncode, size, "%s@%s%c%s%c%s", uname, info->server, 0, uname, 0, passw);
	else
		size = mir_snprintf(toEncode, size, "%c%s%c%s", 0, uname, 0, passw);

	return mir_base64_encode((PBYTE)toEncode, (int)size);
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic type

TJabberAuth::TJabberAuth(ThreadData* pInfo) :
	bIsValid(true),
	szName(NULL),
	info(pInfo)
{
}

TJabberAuth::~TJabberAuth()
{
}

char* TJabberAuth::getInitialRequest()
{
	return NULL;
}

char* TJabberAuth::getChallenge(const TCHAR*)
{
	return NULL;
}

bool TJabberAuth::validateLogin(const TCHAR*)
{
	return true;
}
