/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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
#include "jabber_secur.h"

/////////////////////////////////////////////////////////////////////////////////////////
// ntlm auth - LanServer based authorization

TNtlmAuth::TNtlmAuth(ThreadData *info, const char *mechanism) :
	TJabberAuth(info, mechanism)
{
	bIsValid = false;

	const wchar_t *szProvider;
	if (!mir_strcmp(mechanism, "GSS-SPNEGO"))
		szProvider = L"Negotiate", priority = 703;
	else if (!mir_strcmp(mechanism, "GSSAPI"))
		szProvider = L"Kerberos", priority = 702;
	else if (!mir_strcmp(mechanism, "NTLM"))
		szProvider = L"NTLM", priority = 701;
	else
		return;

	wchar_t szSpn[1024]; szSpn[0] = 0;
	if (!mir_strcmp(mechanism, "GSSAPI"))
		if (!getSpn(szSpn, _countof(szSpn)))
			return;

	if ((hProvider = Netlib_InitSecurityProvider(szProvider, szSpn)) == nullptr)
		return;
	
	// This generates login method advertisement packet
	if (info->conn.password[0] != 0)
		szInitRequest = Netlib_NtlmCreateResponse(hProvider, "", Utf2T(info->conn.username), Utf2T(info->conn.password), complete);
	else
		szInitRequest = Netlib_NtlmCreateResponse(hProvider, "", nullptr, nullptr, complete);
	if (szInitRequest == nullptr)
		return;

	bIsValid = true;
}

TNtlmAuth::~TNtlmAuth()
{
	if (hProvider != nullptr)
		Netlib_DestroySecurityProvider(hProvider);
}

bool TNtlmAuth::getSpn(wchar_t* szSpn, size_t dwSpnLen)
{
	wchar_t szFullUserName[128] = L"";
	ULONG szFullUserNameLen = _countof(szFullUserName);
	if (!GetUserNameEx(NameDnsDomain, szFullUserName, &szFullUserNameLen)) {
		szFullUserName[0] = 0;
		szFullUserNameLen = _countof(szFullUserName);
		GetUserNameEx(NameSamCompatible, szFullUserName, &szFullUserNameLen);
	}

	wchar_t *name = wcsrchr(szFullUserName, '\\');
	if (name) *name = 0;
	else return false;

	if (info->gssapiHostName && info->gssapiHostName[0]) {
		wchar_t *szFullUserNameU = wcsupr(mir_wstrdup(szFullUserName));
		mir_snwprintf(szSpn, dwSpnLen, L"xmpp/%s/%s@%s", info->gssapiHostName, szFullUserName, szFullUserNameU);
		mir_free(szFullUserNameU);
	}
	else {
		const char* connectHost = info->conn.manualHost[0] ? info->conn.manualHost : info->conn.server;

		unsigned long ip = inet_addr(connectHost);
		PHOSTENT host = (ip == INADDR_NONE) ? nullptr : gethostbyaddr((char*)&ip, 4, AF_INET);
		if (host && host->h_name)
			connectHost = host->h_name;

		wchar_t *connectHostT = mir_a2u(connectHost);
		mir_snwprintf(szSpn, dwSpnLen, L"xmpp/%s@%s", connectHostT, wcsupr(szFullUserName));
		mir_free(connectHostT);
	}

	Netlib_Logf(nullptr, "SPN: %S", szSpn);
	return true;
}

char* TNtlmAuth::getInitialRequest()
{
	return szInitRequest.detach();
}

char* TNtlmAuth::getChallenge(const char *challenge)
{
	if (!hProvider)
		return nullptr;

	const char *text((!mir_strcmp(challenge, "=")) ? "" : challenge);
	if (info->conn.password[0] != 0)
		return Netlib_NtlmCreateResponse(hProvider, text, Utf2T(info->conn.username), Utf2T(info->conn.password), complete);
	
	return Netlib_NtlmCreateResponse(hProvider, text, nullptr, nullptr, complete);
}

/////////////////////////////////////////////////////////////////////////////////////////
// md5 auth - digest-based authorization

TMD5Auth::TMD5Auth(ThreadData *info) :
	TJabberAuth(info, "DIGEST-MD5"),
	iCallCount(0)
{
	priority = 301;
}

TMD5Auth::~TMD5Auth()
{
}

char* TMD5Auth::getChallenge(const char *challenge)
{
	if (iCallCount > 0)
		return nullptr;

	iCallCount++;

	size_t resultLen;
	ptrA text((char*)mir_base64_decode(challenge, &resultLen));

	TStringPairs pairs(text);
	const char *realm = pairs["realm"], *nonce = pairs["nonce"];

	char cnonce[40], tmpBuf[40];
	uint32_t digest[4], hash1[4], hash2[4];
	mir_md5_state_t ctx;

	Utils_GetRandom(digest, sizeof(digest));
	mir_snprintf(cnonce, "%08x%08x%08x%08x", htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	ptrA serv(mir_utf8encode(info->conn.server));

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (uint8_t*)info->conn.username, (int)mir_strlen(info->conn.username));
	mir_md5_append(&ctx, (uint8_t*)":", 1);
	mir_md5_append(&ctx, (uint8_t*)realm, (int)mir_strlen(realm));
	mir_md5_append(&ctx, (uint8_t*)":", 1);
	mir_md5_append(&ctx, (uint8_t*)info->conn.password, (int)mir_strlen(info->conn.password));
	mir_md5_finish(&ctx, (uint8_t*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (uint8_t*)hash1, 16);
	mir_md5_append(&ctx, (uint8_t*)":", 1);
	mir_md5_append(&ctx, (uint8_t*)nonce, (int)mir_strlen(nonce));
	mir_md5_append(&ctx, (uint8_t*)":", 1);
	mir_md5_append(&ctx, (uint8_t*)cnonce, (int)mir_strlen(cnonce));
	mir_md5_finish(&ctx, (uint8_t*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (uint8_t*)"AUTHENTICATE:xmpp/", 18);
	mir_md5_append(&ctx, (uint8_t*)(char*)serv, (int)mir_strlen(serv));
	mir_md5_finish(&ctx, (uint8_t*)hash2);

	mir_md5_init(&ctx);
	mir_snprintf(tmpBuf, "%08x%08x%08x%08x", htonl(hash1[0]), htonl(hash1[1]), htonl(hash1[2]), htonl(hash1[3]));
	mir_md5_append(&ctx, (uint8_t*)tmpBuf, (int)mir_strlen(tmpBuf));
	mir_md5_append(&ctx, (uint8_t*)":", 1);
	mir_md5_append(&ctx, (uint8_t*)nonce, (int)mir_strlen(nonce));
	mir_snprintf(tmpBuf, ":%08d:", iCallCount);
	mir_md5_append(&ctx, (uint8_t*)tmpBuf, (int)mir_strlen(tmpBuf));
	mir_md5_append(&ctx, (uint8_t*)cnonce, (int)mir_strlen(cnonce));
	mir_md5_append(&ctx, (uint8_t*)":auth:", 6);
	mir_snprintf(tmpBuf, "%08x%08x%08x%08x", htonl(hash2[0]), htonl(hash2[1]), htonl(hash2[2]), htonl(hash2[3]));
	mir_md5_append(&ctx, (uint8_t*)tmpBuf, (int)mir_strlen(tmpBuf));
	mir_md5_finish(&ctx, (uint8_t*)digest);

	char *buf = (char*)alloca(8000);
	int cbLen = mir_snprintf(buf, 8000,
		"username=\"%s\",realm=\"%s\",nonce=\"%s\",cnonce=\"%s\",nc=%08d,"
		"qop=auth,digest-uri=\"xmpp/%s\",charset=utf-8,response=%08x%08x%08x%08x",
		info->conn.username, realm, nonce, cnonce, iCallCount, serv.get(),
		htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	return mir_base64_encode(buf, cbLen);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SCRAM-SHA-1 authorization

TScramAuth::TScramAuth(ThreadData *info, const char *pszMech, const EVP_MD *pMethod, int iPriority) :
	TJabberAuth(info, pszMech),
	hashMethod(pMethod)
{
	priority = iPriority;

	if ((iPriority % 10) == 1) {
		int cbLen, tlsVer;
		void *pData = Netlib_GetTlsUnique(info->s, cbLen, tlsVer);
		if (pData == nullptr)
			bIsValid = false;
		else {
			bindFlag = (tlsVer == 13) ? "p=tls-exporter,," : "p=tls-unique,,";
			bindData.append(pData, cbLen);
		}
	}
	else bindFlag = "n,,";
}

TScramAuth::~TScramAuth()
{
	mir_free(cnonce);
	mir_free(msg1);
	mir_free(serverSignature);
}

void TScramAuth::Hi(uint8_t *res, char *passw, size_t passwLen, char *salt, size_t saltLen, int ind)
{
	size_t bufLen = saltLen + sizeof(UINT32);
	uint8_t *u = (uint8_t*)_alloca(max(bufLen, EVP_MAX_MD_SIZE));
	memcpy(u, salt, saltLen); *(UINT32*)(u + saltLen) = htonl(1);
	
	memset(res, 0, EVP_MAX_MD_SIZE);

	for (int i = 0; i < ind; i++) {
		unsigned int len;
		HMAC(hashMethod, (uint8_t*)passw, (unsigned)passwLen, u, (unsigned)bufLen, u, &len);
		bufLen = EVP_MD_size(hashMethod);

		for (size_t j = 0; j < bufLen; j++)
			res[j] ^= u[j];
	}
}

char* TScramAuth::getInitialRequest()
{
	unsigned char nonce[24];
	Utils_GetRandom(nonce, sizeof(nonce));
	cnonce = mir_base64_encode(nonce, sizeof(nonce));

	CMStringA buf(FORMAT, "n=%s,r=%s", info->conn.username, cnonce);
	msg1 = mir_strdup(buf);

	buf.Insert(0, bindFlag);
	return mir_base64_encode(buf, buf.GetLength());
}

char* TScramAuth::getChallenge(const char *challenge)
{
	size_t chlLen, saltLen = 0;
	ptrA snonce, salt;
	int ind = -1;

	ptrA chl((char *)mir_base64_decode(challenge, &chlLen)), cbd;
	if (bindData.isEmpty())
		cbd = mir_base64_encode(bindFlag, mir_strlen(bindFlag));
	else {
		bindData.appendBefore((void*)bindFlag, mir_strlen(bindFlag));
		cbd = mir_base64_encode(bindData.data(), bindData.length());
	}

	for (char *p = strtok(NEWSTR_ALLOCA(chl), ","); p != nullptr; p = strtok(nullptr, ",")) {
		if (*p == 'r' && p[1] == '=') { // snonce
			if (strncmp(cnonce, p + 2, mir_strlen(cnonce)))
				return nullptr;
			snonce = mir_strdup(p + 2);
		}
		else if (*p == 's' && p[1] == '=') // salt
			salt = (char*)mir_base64_decode(p + 2, &saltLen);
		else if (*p == 'i' && p[1] == '=')
			ind = atoi(p + 2);
	}

	if (snonce == nullptr || salt == nullptr || ind == -1)
		return nullptr;

	int hashSize = EVP_MD_size(hashMethod);

	uint8_t saltedPassw[EVP_MAX_MD_SIZE];
	Hi(saltedPassw, info->conn.password, mir_strlen(info->conn.password), salt, saltLen, ind);

	uint8_t clientKey[EVP_MAX_MD_SIZE];
	unsigned int len;
	HMAC(hashMethod, saltedPassw, hashSize, (uint8_t*)"Client Key", 10, clientKey, &len);

	uint8_t storedKey[EVP_MAX_MD_SIZE];
	{
		EVP_MD_CTX *pctx = EVP_MD_CTX_new();
		EVP_DigestInit(pctx, hashMethod);
		EVP_DigestUpdate(pctx, clientKey, hashSize);
		EVP_DigestFinal(pctx, storedKey, &len);
		EVP_MD_CTX_free(pctx);
	}

	uint8_t clientSig[EVP_MAX_MD_SIZE];
	CMStringA authmsg(FORMAT, "%s,%s,c=%s,r=%s", msg1, chl.get(), cbd.get(), snonce.get());
	HMAC(hashMethod, storedKey, hashSize, (uint8_t*)authmsg.c_str(), authmsg.GetLength(), clientSig, &len);

	uint8_t clientProof[EVP_MAX_MD_SIZE];
	for (int j = 0; j < hashSize; j++)
		clientProof[j] = clientKey[j] ^ clientSig[j];

	/* Calculate the server signature */
	uint8_t serverKey[EVP_MAX_MD_SIZE];
	HMAC(hashMethod, saltedPassw, hashSize, (uint8_t*)"Server Key", 10, serverKey, &len);

	uint8_t srvSig[EVP_MAX_MD_SIZE];
	HMAC(hashMethod, serverKey, hashSize, (uint8_t*)authmsg.c_str(), authmsg.GetLength(), srvSig, &len);
	serverSignature = mir_base64_encode(srvSig, hashSize);

	ptrA encproof(mir_base64_encode(clientProof, hashSize));
	CMStringA buf(FORMAT, "c=%s,r=%s,p=%s", cbd.get(), snonce.get(), encproof.get());
	return mir_base64_encode(buf, buf.GetLength());
}

bool TScramAuth::validateLogin(const char *challenge)
{
	size_t chlLen;
	ptrA chl((char*)mir_base64_decode(challenge, &chlLen));
	return chl && strncmp((char*)chl + 2, serverSignature, chlLen - 2) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plain auth - the most simple one

TPlainAuth::TPlainAuth(ThreadData *info, bool old) :
	TJabberAuth(info, "PLAIN"),
	bOld(old)
{
	priority = (old) ? 100 : 101;
}

char* TPlainAuth::getInitialRequest()
{
	CMStringA buf;
	if (bOld)
		buf.Format("%s@%s%c%s%c%s", info->conn.username, info->conn.server, 0, info->conn.username, 0, info->conn.password);
	else
		buf.Format("%c%s%c%s", 0, info->conn.username, 0, info->conn.password);

	return mir_base64_encode(buf, buf.GetLength());
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic type

TJabberAuth::TJabberAuth(ThreadData *pInfo, const char *pszMech) :
	info(pInfo),
	szName(mir_strdup(pszMech))
{
}

TJabberAuth::~TJabberAuth()
{
}

char* TJabberAuth::getInitialRequest()
{
	return nullptr;
}

char* TJabberAuth::getChallenge(const char*)
{
	return nullptr;
}

bool TJabberAuth::validateLogin(const char*)
{
	return true;
}
