/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-15 Miranda NG project

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

/////////////////////////////////////////////////////////////////////////////////////////
// ntlm auth - LanServer based authorization

TNtlmAuth::TNtlmAuth(ThreadData *info, const char* mechanism, const TCHAR *hostname) :
	TJabberAuth(info)
{
	szName = mechanism;
	szHostName = hostname;

	const TCHAR *szProvider;
	if (!strcmp(mechanism, "GSS-SPNEGO"))
		szProvider = _T("Negotiate");
	else if (!strcmp(mechanism, "GSSAPI"))
		szProvider = _T("GSSAPI");
	else if (!strcmp(mechanism, "NTLM"))
		szProvider = _T("NTLM");
	else {
LBL_Invalid:
		bIsValid = false;
		hProvider = NULL;
		return;
	}

	TCHAR szSpn[1024] = _T("");
	if (strcmp(mechanism, "NTLM"))
		if (!getSpn(szSpn, SIZEOF(szSpn)) && !strcmp(mechanism, "GSSAPI"))
			goto LBL_Invalid;

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
	TCHAR szFullUserName[128] = _T("");
	ULONG szFullUserNameLen = SIZEOF(szFullUserName);
	if (!GetUserNameEx(NameDnsDomain, szFullUserName, &szFullUserNameLen)) {
		szFullUserName[0] = 0;
		szFullUserNameLen = SIZEOF(szFullUserName);
		GetUserNameEx(NameSamCompatible, szFullUserName, &szFullUserNameLen);
	}

	TCHAR *name = _tcsrchr(szFullUserName, '\\');
	if (name) *name = 0;
	else return false;

	if (szHostName && szHostName[0]) {
		TCHAR *szFullUserNameU = _tcsupr(mir_tstrdup(szFullUserName));
		mir_sntprintf(szSpn, dwSpnLen, _T("xmpp/%s/%s@%s"), szHostName, szFullUserName, szFullUserNameU);
		mir_free(szFullUserNameU);
	}
	else {
		const char* connectHost = info->conn.manualHost[0] ? info->conn.manualHost : info->conn.server;

		unsigned long ip = inet_addr(connectHost);
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
	if (!hProvider)
		return NULL;

	// This generates login method advertisement packet
	if (info->conn.password[0] != 0)
		return Netlib_NtlmCreateResponse2(hProvider, "", info->conn.username, info->conn.password, &complete);

	return Netlib_NtlmCreateResponse2(hProvider, "", NULL, NULL, &complete);
}

char* TNtlmAuth::getChallenge(const TCHAR *challenge)
{
	if (!hProvider)
		return NULL;

	ptrA text((!mir_tstrcmp(challenge, _T("="))) ? mir_strdup("") : mir_t2a(challenge));
	if (info->conn.password[0] != 0)
		return Netlib_NtlmCreateResponse2(hProvider, text, info->conn.username, info->conn.password, &complete);
	
	return Netlib_NtlmCreateResponse2(hProvider, text, NULL, NULL, &complete);
}

/////////////////////////////////////////////////////////////////////////////////////////
// md5 auth - digest-based authorization

TMD5Auth::TMD5Auth(ThreadData *info) :
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

	ptrA uname(mir_utf8encodeT(info->conn.username)),
		passw(mir_utf8encodeT(info->conn.password)),
		serv(mir_utf8encode(info->conn.server));

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)(char*)uname, (int)strlen(uname));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)realm, (int)strlen(realm));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)(char*)passw, (int)strlen(passw));
	mir_md5_finish(&ctx, (BYTE*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)hash1, 16);
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)nonce, (int)strlen(nonce));
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
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)nonce, (int)strlen(nonce));
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

TScramAuth::TScramAuth(ThreadData *info) :
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

void TScramAuth::Hi(BYTE* res, char* passw, size_t passwLen, char* salt, size_t saltLen, int ind)
{
	size_t bufLen = saltLen + sizeof(UINT32);
	BYTE *u = (BYTE*)_alloca(max(bufLen, MIR_SHA1_HASH_SIZE));
	memcpy(u, salt, saltLen); *(UINT32*)(u + saltLen) = htonl(1);
	
	memset(res, 0, MIR_SHA1_HASH_SIZE);

	for (int i = 0; i < ind; i++) {
		mir_hmac_sha1(u, (BYTE*)passw, passwLen, u, bufLen);
		bufLen = MIR_SHA1_HASH_SIZE;

		for (unsigned j = 0; j < MIR_SHA1_HASH_SIZE; j++)
			res[j] ^= u[j];
	}
}

char* TScramAuth::getChallenge(const TCHAR *challenge)
{
	unsigned chlLen, saltLen = 0;
	ptrA snonce, salt;
	int ind = -1;

	ptrA chl((char*)mir_base64_decode(_T2A(challenge), &chlLen));

	for (char *p = strtok(NEWSTR_ALLOCA(chl), ","); p != NULL; p = strtok(NULL, ",")) {
		if (*p == 'r' && p[1] == '=') { // snonce
			if (strncmp(cnonce, p + 2, strlen(cnonce)))
				return NULL;
			snonce = mir_strdup(p + 2);
		}
		else if (*p == 's' && p[1] == '=') // salt
			salt = (char*)mir_base64_decode(p + 2, &saltLen);
		else if (*p == 'i' && p[1] == '=')
			ind = atoi(p + 2);
	}

	if (snonce == NULL || salt == NULL || ind == -1)
		return NULL;

	ptrA passw(mir_utf8encodeT(info->conn.password));
	size_t passwLen = strlen(passw);

	BYTE saltedPassw[MIR_SHA1_HASH_SIZE];
	Hi(saltedPassw, passw, passwLen, salt, saltLen, ind);

	BYTE clientKey[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(clientKey, saltedPassw, sizeof(saltedPassw), (BYTE*)"Client Key", 10);

	BYTE storedKey[MIR_SHA1_HASH_SIZE];

	mir_sha1_ctx ctx;
	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, clientKey, MIR_SHA1_HASH_SIZE);
	mir_sha1_finish(&ctx, storedKey);

	char authmsg[4096];
	int authmsgLen = mir_snprintf(authmsg, SIZEOF(authmsg), "%s,%s,c=biws,r=%s", msg1, chl, snonce);

	BYTE clientSig[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(clientSig, storedKey, sizeof(storedKey), (BYTE*)authmsg, authmsgLen);

	BYTE clientProof[MIR_SHA1_HASH_SIZE];
	for (unsigned j = 0; j < sizeof(clientKey); j++)
		clientProof[j] = clientKey[j] ^ clientSig[j];

	/* Calculate the server signature */
	BYTE serverKey[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(serverKey, saltedPassw, sizeof(saltedPassw), (BYTE*)"Server Key", 10);

	BYTE srvSig[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(srvSig, serverKey, sizeof(serverKey), (BYTE*)authmsg, authmsgLen);
	serverSignature = mir_base64_encode((PBYTE)srvSig, sizeof(srvSig));

	char buf[4096];
	ptrA encproof(mir_base64_encode((PBYTE)clientProof, sizeof(clientProof)));
	int cbLen = mir_snprintf(buf, SIZEOF(buf), "c=biws,r=%s,p=%s", snonce, encproof);
	return mir_base64_encode((PBYTE)buf, cbLen);
}

char* TScramAuth::getInitialRequest()
{
	ptrA uname(mir_utf8encodeT(info->conn.username));

	unsigned char nonce[24];
	CallService(MS_UTILS_GETRANDOM, sizeof(nonce), (LPARAM)nonce);
	cnonce = mir_base64_encode((PBYTE)nonce, sizeof(nonce));

	char buf[4096];
	int cbLen = mir_snprintf(buf, SIZEOF(buf), "n,,n=%s,r=%s", uname, cnonce);
	msg1 = mir_strdup(buf + 3);
	return mir_base64_encode((PBYTE)buf, cbLen);
}

bool TScramAuth::validateLogin(const TCHAR *challenge)
{
	unsigned chlLen;
	ptrA chl((char*)mir_base64_decode(_T2A(challenge), &chlLen));
	return chl && strncmp((char*)chl + 2, serverSignature, chlLen - 2) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plain auth - the most simple one

TPlainAuth::TPlainAuth(ThreadData *info, bool old) :
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
	ptrA uname(mir_utf8encodeT(info->conn.username)), passw(mir_utf8encodeT(info->conn.password));

	size_t size = 2 * strlen(uname) + strlen(passw) + strlen(info->conn.server) + 4;
	char *toEncode = (char*)alloca(size);
	if (bOld)
		size = mir_snprintf(toEncode, size, "%s@%s%c%s%c%s", uname, info->conn.server, 0, uname, 0, passw);
	else
		size = mir_snprintf(toEncode, size, "%c%s%c%s", 0, uname, 0, passw);

	return mir_base64_encode((PBYTE)toEncode, (int)size);
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic type

TJabberAuth::TJabberAuth(ThreadData* pInfo) :
	bIsValid(true),
	complete(0),
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
