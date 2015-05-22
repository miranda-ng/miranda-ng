////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2010 Bartosz Bia³ek
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"
#include <io.h>
#include <fcntl.h>
#include "protocol.h"

//////////////////////////////////////////////////////////
// OAuth 1.0 implementation

// Service Provider must accept the HTTP Authorization header

// RSA-SHA1 signature method (see RFC 3447 section 8.2
// and RSASSA-PKCS1-v1_5 algorithm) is unimplemented

typedef struct
{
	char *name;
	char *value;
} OAUTHPARAMETER;

typedef enum
{
	HMACSHA1,
	RSASHA1,
	PLAINTEXT
} OAUTHSIGNMETHOD;

static int paramsortFunc(const OAUTHPARAMETER *p1, const OAUTHPARAMETER *p2)
{
	int res = strcmp(p1->name, p2->name);
	return res != 0 ? res : strcmp(p1->value, p2->value);
}

// see RFC 3986 for details
#define isunreserved(c) ( isalnum((unsigned char)c) || c == '-' || c == '.' || c == '_' || c == '~')
char *oauth_uri_escape(const char *str)
{
	char *res;
	int size, ix = 0;

	if (str == NULL) return mir_strdup("");

	size = (int)mir_strlen(str) + 1;
	res = (char *)mir_alloc(size);

	while (*str) {
		if (!isunreserved(*str)) {
			size += 2;
			res = (char *)mir_realloc(res, size);
			mir_snprintf(&res[ix], 4, "%%%X%X", (*str >> 4) & 15, *str & 15);
			ix += 3;
		}
		else
			res[ix++] = *str;
		str++;
	}
	res[ix] = 0;

	return res;
}

// generates Signature Base String

char *oauth_generate_signature(LIST<OAUTHPARAMETER> &params, const char *httpmethod, const char *url)
{
	char *res, *urlenc, *urlnorm;
	OAUTHPARAMETER *p;
	int i, ix = 0, size;

	if (httpmethod == NULL || url == NULL || !params.getCount()) return mir_strdup("");

	urlnorm = (char *)mir_alloc(mir_strlen(url) + 1);
	while (*url) {
		if (*url == '?' || *url == '#')	break; // see RFC 3986 section 3
		urlnorm[ix++] = tolower(*url);
		url++;
	}
	urlnorm[ix] = 0;
	if ((res = strstr(urlnorm, ":80")) != NULL)
		memmove(res, res + 3, mir_strlen(res) - 2);
	else if ((res = strstr(urlnorm, ":443")) != NULL)
		memmove(res, res + 4, mir_strlen(res) - 3);

	urlenc = oauth_uri_escape(urlnorm);
	mir_free(urlnorm);
	size = (int)mir_strlen(httpmethod) + (int)mir_strlen(urlenc) + 1 + 2;

	for (i = 0; i < params.getCount(); i++) {
		p = params[i];
		if (!strcmp(p->name, "oauth_signature")) continue;
		if (i > 0) size += 3;
		size += (int)mir_strlen(p->name) + (int)mir_strlen(p->value) + 3;
	}

	res = (char *)mir_alloc(size);
	strcpy(res, httpmethod);
	strcat(res, "&");
	strcat(res, urlenc);
	mir_free(urlenc);
	strcat(res, "&");

	for (i = 0; i < params.getCount(); i++) {
		p = params[i];
		if (!strcmp(p->name, "oauth_signature")) continue;
		if (i > 0) strcat(res, "%26");
		strcat(res, p->name);
		strcat(res, "%3D");
		strcat(res, p->value);
	}

	return res;
}

char *oauth_getparam(LIST<OAUTHPARAMETER> &params, const char *name)
{
	OAUTHPARAMETER *p;
	int i;

	if (name == NULL) return NULL;

	for (i = 0; i < params.getCount(); i++) {
		p = params[i];
		if (!strcmp(p->name, name))
			return p->value;
	}

	return NULL;
}

void oauth_setparam(LIST<OAUTHPARAMETER> &params, const char *name, const char *value)
{
	OAUTHPARAMETER *p;
	int i;

	if (name == NULL) return;

	for (i = 0; i < params.getCount(); i++) {
		p = params[i];
		if (!strcmp(p->name, name)) {
			mir_free(p->value);
			p->value = oauth_uri_escape(value);
			return;
		}
	}

	p = (OAUTHPARAMETER*)mir_alloc(sizeof(OAUTHPARAMETER));
	p->name = oauth_uri_escape(name);
	p->value = oauth_uri_escape(value);
	params.insert(p);
}

void oauth_freeparams(LIST<OAUTHPARAMETER> &params)
{
	OAUTHPARAMETER *p;
	int i;

	for (i = 0; i < params.getCount(); i++) {
		p = params[i];
		mir_free(p->name);
		mir_free(p->value);
	}
}

int oauth_sign_request(LIST<OAUTHPARAMETER> &params, const char *httpmethod, const char *url,
					   const char *consumer_secret, const char *token_secret)
{
	char *sign = NULL, *signmethod;

	if (!params.getCount()) return -1;

	signmethod = oauth_getparam(params, "oauth_signature_method");
	if (signmethod == NULL) return -1;

	if (!strcmp(signmethod, "HMAC-SHA1")) {
		ptrA text( oauth_generate_signature(params, httpmethod, url));
		ptrA csenc( oauth_uri_escape(consumer_secret));
		ptrA tsenc( oauth_uri_escape(token_secret));
		ptrA key((char *)mir_alloc(mir_strlen(csenc) + mir_strlen(tsenc) + 2));
		strcpy(key, csenc);
		strcat(key, "&");
		strcat(key, tsenc);

		BYTE digest[MIR_SHA1_HASH_SIZE];
		mir_hmac_sha1(digest, (BYTE*)(char*)key, mir_strlen(key), (BYTE*)(char*)text, mir_strlen(text));
		sign = mir_base64_encode(digest, MIR_SHA1_HASH_SIZE);
	}
	else { // PLAINTEXT
		ptrA csenc( oauth_uri_escape(consumer_secret));
		ptrA tsenc( oauth_uri_escape(token_secret));

		sign = (char *)mir_alloc(mir_strlen(csenc) + mir_strlen(tsenc) + 2);
		strcpy(sign, csenc);
		strcat(sign, "&");
		strcat(sign, tsenc);
	}

	oauth_setparam(params, "oauth_signature", sign);
	mir_free(sign);

	return 0;
}

char *oauth_generate_nonce()
{
	char timestamp[22], randnum[16];
	mir_snprintf(timestamp, SIZEOF(timestamp), "%ld", time(NULL)); 
	CallService(MS_UTILS_GETRANDOM, (WPARAM)sizeof(randnum), (LPARAM)randnum);

	int strSizeB = int(mir_strlen(timestamp) + sizeof(randnum));
	ptrA str((char *)mir_calloc(strSizeB + 1));
	strcpy(str, timestamp);
	strncat(str, randnum, sizeof(randnum));

	BYTE digest[16];
	mir_md5_hash((BYTE*)(char*)str, strSizeB, digest);
	return bin2hex(digest, sizeof(digest), (char *)mir_alloc(32 + 1));
}

char *oauth_auth_header(const char *httpmethod, const char *url, OAUTHSIGNMETHOD signmethod,
						const char *consumer_key, const char *consumer_secret,
						const char *token, const char *token_secret)
{
	int i, size;
	char *res, timestamp[22], *nonce;

	if (httpmethod == NULL || url == NULL) return NULL;

	LIST<OAUTHPARAMETER> oauth_parameters(1, paramsortFunc);
	oauth_setparam(oauth_parameters, "oauth_consumer_key", consumer_key);
	oauth_setparam(oauth_parameters, "oauth_version", "1.0");
	switch (signmethod) {
		case HMACSHA1: oauth_setparam(oauth_parameters, "oauth_signature_method", "HMAC-SHA1"); break;
		case RSASHA1: oauth_setparam(oauth_parameters, "oauth_signature_method", "RSA-SHA1"); break;
		default: oauth_setparam(oauth_parameters, "oauth_signature_method", "PLAINTEXT"); break;
	};
	mir_snprintf(timestamp, SIZEOF(timestamp), "%ld", time(NULL)); 
	oauth_setparam(oauth_parameters, "oauth_timestamp", timestamp);
	nonce = oauth_generate_nonce();
	oauth_setparam(oauth_parameters, "oauth_nonce", nonce);
	mir_free(nonce);
	if (token != NULL && *token)
		oauth_setparam(oauth_parameters, "oauth_token", token);

	if (oauth_sign_request(oauth_parameters, httpmethod, url, consumer_secret, token_secret)) {
		oauth_freeparams(oauth_parameters);
		return NULL;
	}

	size = 7;
	for (i = 0; i < oauth_parameters.getCount(); i++) {
		OAUTHPARAMETER *p = oauth_parameters[i];
		if (i > 0) size++;
		size += (int)mir_strlen(p->name) + (int)mir_strlen(p->value) + 3;
	}

	res = (char *)mir_alloc(size);
	strcpy(res, "OAuth ");

	for (i = 0; i < oauth_parameters.getCount(); i++) {
		OAUTHPARAMETER *p = oauth_parameters[i];
		if (i > 0) strcat(res, ",");
		strcat(res, p->name);
		strcat(res, "=\"");
		strcat(res, p->value);
		strcat(res, "\"");
	}

	oauth_freeparams(oauth_parameters);
	return res;
}

int GGPROTO::oauth_receivetoken()
{
	char szUrl[256], uin[32], *str, *token = NULL, *token_secret = NULL;
	int res = 0;
	HANDLE nlc = NULL;

	UIN2IDA( getDword(GG_KEY_UIN, 0), uin);
	char *password = getStringA(GG_KEY_PASSWORD);

	// 1. Obtaining an Unauthorized Request Token
	debugLogA("oauth_receivetoken(): Obtaining an Unauthorized Request Token...");
	strcpy(szUrl, "http://api.gadu-gadu.pl/request_token");
	str = oauth_auth_header("POST", szUrl, HMACSHA1, uin, password, NULL, NULL);

	NETLIBHTTPHEADER httpHeaders[3];
	httpHeaders[0].szName   = "User-Agent";
	httpHeaders[0].szValue = GG8_VERSION;
	httpHeaders[1].szName  = "Authorization";
	httpHeaders[1].szValue = str;
	httpHeaders[2].szName  = "Accept";
	httpHeaders[2].szValue = "*/*";

	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_POST;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_PERSISTENT;
	req.headersCount = 3;
	req.headers = httpHeaders;

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (resp) {
		nlc = resp->nlc; 
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			TCHAR *xmlAction = mir_a2t(resp->pData);
			HXML hXml = xi.parseString(xmlAction, 0, _T("result"));
			if (hXml != NULL) {
				HXML node = xi.getChildByPath(hXml, _T("oauth_token"), 0);
				token = node != NULL ? mir_t2a(xi.getText(node)) : NULL;

				node = xi.getChildByPath(hXml, _T("oauth_token_secret"), 0);
				token_secret = node != NULL ? mir_t2a(xi.getText(node)) : NULL;

				xi.destroyNode(hXml);
			}
			mir_free(xmlAction);
		}
		else debugLogA("oauth_receivetoken(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else debugLogA("oauth_receivetoken(): No response from HTTP request");

	// 2. Obtaining User Authorization
	debugLogA("oauth_receivetoken(): Obtaining User Authorization...");
	mir_free(str);
	str = oauth_uri_escape("http://www.mojageneracja.pl");

	mir_snprintf(szUrl, SIZEOF(szUrl), "callback_url=%s&request_token=%s&uin=%s&password=%s",
			str, token, uin, password); 
	mir_free(str);
	str = mir_strdup(szUrl);

	memset(&req, 0, sizeof(req));
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_POST;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
	req.headersCount = 3;
	req.headers = httpHeaders;
	strcpy(szUrl, "https://login.gadu-gadu.pl/authorize");
	httpHeaders[1].szName  = "Content-Type";
	httpHeaders[1].szValue = "application/x-www-form-urlencoded";
	req.pData = str;
	req.dataLength = (int)mir_strlen(str);

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (resp) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	else debugLogA("oauth_receivetoken(): No response from HTTP request");

	// 3. Obtaining an Access Token
	debugLogA("oauth_receivetoken(): Obtaining an Access Token...");
	strcpy(szUrl, "http://api.gadu-gadu.pl/access_token");
	mir_free(str);
	str = oauth_auth_header("POST", szUrl, HMACSHA1, uin, password, token, token_secret);
	mir_free(token);
	mir_free(token_secret);
	token = NULL;
	token_secret = NULL;

	memset(&req, 0, sizeof(req));
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_POST;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_PERSISTENT;
	req.nlc = nlc;
	req.headersCount = 3;
	req.headers = httpHeaders;
	httpHeaders[1].szName  = "Authorization";
	httpHeaders[1].szValue = str;

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			TCHAR *xmlAction = mir_a2t(resp->pData);
			HXML hXml = xi.parseString(xmlAction, 0, _T("result"));
			if (hXml != NULL) {
				HXML node = xi.getChildByPath(hXml, _T("oauth_token"), 0);
				token = mir_t2a(xi.getText(node));

				node = xi.getChildByPath(hXml, _T("oauth_token_secret"), 0);
				token_secret = mir_t2a(xi.getText(node));

				xi.destroyNode(hXml);
			}
			mir_free(xmlAction);
		}
		else debugLogA("oauth_receivetoken(): Invalid response code from HTTP request");
		Netlib_CloseHandle(resp->nlc);
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else debugLogA("oauth_receivetoken(): No response from HTTP request");

	mir_free(password);
	mir_free(str);

	if (token != NULL && token_secret != NULL) {
		setString(GG_KEY_TOKEN, token);
		setString(GG_KEY_TOKENSECRET, token_secret);
		debugLogA("oauth_receivetoken(): Access Token obtained successfully.");
		res = 1;
	}
	else {
		delSetting(GG_KEY_TOKEN);
		delSetting(GG_KEY_TOKENSECRET);
		debugLogA("oauth_receivetoken(): Failed to obtain Access Token.");
	}
	mir_free(token);
	mir_free(token_secret);

	return res;
}

int GGPROTO::oauth_checktoken(int force)
{
	if (force)
		return oauth_receivetoken();

	ptrA token( getStringA(GG_KEY_TOKEN));
	ptrA token_secret( getStringA(GG_KEY_TOKENSECRET));
	if (token == NULL || token_secret == NULL)
		return oauth_receivetoken();

	return 1;
}
