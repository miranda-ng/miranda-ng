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

static int paramsortFunc(OAUTHPARAMETER *p1, OAUTHPARAMETER *p2)
{
	int res = strcmp(p1->name, p2->name);
	return res != 0 ? res : strcmp(p1->value, p2->value);
}

// HMAC-SHA1 (see RFC 2104 for details)
void hmacsha1_hash(mir_sha1_byte_t *text, int text_len, mir_sha1_byte_t *key, int key_len,
				   mir_sha1_byte_t digest[MIR_SHA1_HASH_SIZE])
{
	mir_sha1_ctx context;
	mir_sha1_byte_t k_ipad[64];
	mir_sha1_byte_t k_opad[64];
	int i;

	if (key_len > 64) {
		mir_sha1_ctx tctx;
		mir_sha1_byte_t tk[MIR_SHA1_HASH_SIZE];

		mir_sha1_init(&tctx);
		mir_sha1_append(&tctx, key, key_len);
		mir_sha1_finish(&tctx, tk);

		key = tk;
		key_len = MIR_SHA1_HASH_SIZE;
	}

	memset(k_ipad, 0x36, 64);
	memset(k_opad, 0x5c, 64);

	for (i = 0; i < key_len; i++) {
		k_ipad[i] ^= key[i];
		k_opad[i] ^= key[i];
	}

	mir_sha1_init(&context);
	mir_sha1_append(&context, k_ipad, 64);
	mir_sha1_append(&context, text, text_len);
	mir_sha1_finish(&context, digest);

	mir_sha1_init(&context);
	mir_sha1_append(&context, k_opad, 64);
	mir_sha1_append(&context, digest, MIR_SHA1_HASH_SIZE);
	mir_sha1_finish(&context, digest);
}

// see RFC 3986 for details
#define isunreserved(c) ( isalnum((unsigned char)c) || c == '-' || c == '.' || c == '_' || c == '~')
char *oauth_uri_escape(const char *str)
{
	char *res;
	int size, ix = 0;

	if (str == NULL) return mir_strdup("");

	size = (int)strlen(str) + 1;
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
char *oauth_generate_signature(SortedList *params, const char *httpmethod, const char *url)
{
	char *res, *urlenc, *urlnorm;
	OAUTHPARAMETER *p;
	int i, ix = 0, size;

	if (httpmethod == NULL || url == NULL || !params->realCount) return mir_strdup("");

	urlnorm = (char *)mir_alloc(strlen(url) + 1);
	while (*url) {
		if (*url == '?' || *url == '#')	break; // see RFC 3986 section 3
		urlnorm[ix++] = tolower(*url);
		url++;
	}
	urlnorm[ix] = 0;
	if ((res = strstr(urlnorm, ":80")) != NULL)
		memmove(res, res + 3, strlen(res) - 2);
	else if ((res = strstr(urlnorm, ":443")) != NULL)
		memmove(res, res + 4, strlen(res) - 3);

	urlenc = oauth_uri_escape(urlnorm);
	mir_free(urlnorm);
	size = (int)strlen(httpmethod) + (int)strlen(urlenc) + 1 + 2;

	for (i = 0; i < params->realCount; i++) {
		p = params->items[i];
		if (!strcmp(p->name, "oauth_signature")) continue;
		if (i > 0) size += 3;
		size += (int)strlen(p->name) + (int)strlen(p->value) + 3;
	}

	res = (char *)mir_alloc(size);
	strcpy(res, httpmethod);
	strcat(res, "&");
	strcat(res, urlenc);
	mir_free(urlenc);
	strcat(res, "&");

	for (i = 0; i < params->realCount; i++) {
		p = params->items[i];
		if (!strcmp(p->name, "oauth_signature")) continue;
		if (i > 0) strcat(res, "%26");
		strcat(res, p->name);
		strcat(res, "%3D");
		strcat(res, p->value);
	}

	return res;
}

char *oauth_getparam(SortedList *params, const char *name)
{
	OAUTHPARAMETER *p;
	int i;

	if (name == NULL) return NULL;

	for (i = 0; i < params->realCount; i++) {
		p = params->items[i];
		if (!strcmp(p->name, name))
			return p->value;
	}

	return NULL;
}

void oauth_setparam(SortedList *params, const char *name, const char *value)
{
	OAUTHPARAMETER *p;
	int i;

	if (name == NULL) return;

	for (i = 0; i < params->realCount; i++) {
		p = params->items[i];
		if (!strcmp(p->name, name)) {
			mir_free(p->value);
			p->value = oauth_uri_escape(value);
			return;
		}
	}

	p = mir_alloc(sizeof(OAUTHPARAMETER));
	p->name = oauth_uri_escape(name);
	p->value = oauth_uri_escape(value);
	li.List_InsertPtr(params, p);
}

void oauth_freeparams(SortedList *params)
{
	OAUTHPARAMETER *p;
	int i;

	for (i = 0; i < params->realCount; i++) {
		p = params->items[i];
		mir_free(p->name);
		mir_free(p->value);
	}
}

int oauth_sign_request(SortedList *params, const char *httpmethod, const char *url,
					   const char *consumer_secret, const char *token_secret)
{
	char *sign = NULL, *signmethod;

	if (!params->realCount) return -1;

	signmethod = oauth_getparam(params, "oauth_signature_method");
	if (signmethod == NULL) return -1;

	if (!strcmp(signmethod, "HMAC-SHA1")) {
		char *text = oauth_generate_signature(params, httpmethod, url);
		char *key;
		char *csenc = oauth_uri_escape(consumer_secret);
		char *tsenc = oauth_uri_escape(token_secret);
		mir_sha1_byte_t digest[MIR_SHA1_HASH_SIZE];
		NETLIBBASE64 nlb64 = {0};
		int signlen;

		key = (char *)mir_alloc(strlen(csenc) + strlen(tsenc) + 2);
		strcpy(key, csenc);
		strcat(key, "&");
		strcat(key, tsenc);
		mir_free(csenc);
		mir_free(tsenc);

		hmacsha1_hash(text, (int)strlen(text), key, (int)strlen(key), digest);

		signlen = Netlib_GetBase64EncodedBufferSize(MIR_SHA1_HASH_SIZE);
		sign = (char *)mir_alloc(signlen);
		nlb64.pszEncoded = sign;
		nlb64.cchEncoded = signlen;
		nlb64.pbDecoded = digest;
		nlb64.cbDecoded = MIR_SHA1_HASH_SIZE;
		CallService(MS_NETLIB_BASE64ENCODE, 0, (LPARAM)&nlb64);

		mir_free(text);
		mir_free(key);
	}
//	else if (!strcmp(signmethod, "RSA-SHA1")) { // unimplemented
//	}
	else { // PLAINTEXT
		char *csenc = oauth_uri_escape(consumer_secret);
		char *tsenc = oauth_uri_escape(token_secret);

		sign = (char *)mir_alloc(strlen(csenc) + strlen(tsenc) + 2);
		strcpy(sign, csenc);
		strcat(sign, "&");
		strcat(sign, tsenc);
		mir_free(csenc);
		mir_free(tsenc);
	}

	oauth_setparam(params, "oauth_signature", sign);
	mir_free(sign);

	return 0;
}

char *oauth_generate_nonce()
{
	mir_md5_byte_t digest[16];
	char *result, *str, timestamp[22], randnum[16];
	int i;

	mir_snprintf(timestamp, sizeof(timestamp), "%ld", time(NULL)); 
	CallService(MS_UTILS_GETRANDOM, (WPARAM)sizeof(randnum), (LPARAM)randnum);

	str = (char *)mir_alloc(strlen(timestamp) + strlen(randnum) + 1);
	strcpy(str, timestamp);
	strcat(str, randnum);
	mir_md5_hash(str, (int)strlen(str), digest);
	mir_free(str);

	result = (char *)mir_alloc(32 + 1);
	for (i = 0; i < 16; i++)
		sprintf(result + (i<<1), "%02x", digest[i]);

	return result;
}

char *oauth_auth_header(const char *httpmethod, const char *url, OAUTHSIGNMETHOD signmethod,
						const char *consumer_key, const char *consumer_secret,
						const char *token, const char *token_secret)
{
	OAUTHPARAMETER *p;
	int i, size;
	char *res, timestamp[22], *nonce;
	SortedList oauth_parameters = {0};

	if (httpmethod == NULL || url == NULL) return NULL;

	oauth_parameters.sortFunc = paramsortFunc;
	oauth_parameters.increment = 1;

	oauth_setparam(&oauth_parameters, "oauth_consumer_key", consumer_key);
	oauth_setparam(&oauth_parameters, "oauth_version", "1.0");
	switch (signmethod) {
		case HMACSHA1: oauth_setparam(&oauth_parameters, "oauth_signature_method", "HMAC-SHA1"); break;
		case RSASHA1: oauth_setparam(&oauth_parameters, "oauth_signature_method", "RSA-SHA1"); break;
		default: oauth_setparam(&oauth_parameters, "oauth_signature_method", "PLAINTEXT"); break;
	};
	mir_snprintf(timestamp, sizeof(timestamp), "%ld", time(NULL)); 
	oauth_setparam(&oauth_parameters, "oauth_timestamp", timestamp);
	nonce = oauth_generate_nonce();
	oauth_setparam(&oauth_parameters, "oauth_nonce", nonce);
	mir_free(nonce);
	if (token != NULL && *token)
		oauth_setparam(&oauth_parameters, "oauth_token", token);

	if (oauth_sign_request(&oauth_parameters, httpmethod, url, consumer_secret, token_secret)) {
		oauth_freeparams(&oauth_parameters);
		li.List_Destroy(&oauth_parameters);
		return NULL;
	}

	size = 7;
	for (i = 0; i < oauth_parameters.realCount; i++) {
		p = oauth_parameters.items[i];
		if (i > 0) size++;
		size += (int)strlen(p->name) + (int)strlen(p->value) + 3;
	}

	res = (char *)mir_alloc(size);
	strcpy(res, "OAuth ");

	for (i = 0; i < oauth_parameters.realCount; i++) {
		p = oauth_parameters.items[i];
		if (i > 0) strcat(res, ",");
		strcat(res, p->name);
		strcat(res, "=\"");
		strcat(res, p->value);
		strcat(res, "\"");
	}

	oauth_freeparams(&oauth_parameters);
	li.List_Destroy(&oauth_parameters);

	return res;
}

char *gg_oauth_header(GGPROTO *gg, const char *httpmethod, const char *url)
{
	char *res, uin[32], *password = NULL, *token = NULL, *token_secret = NULL;
	DBVARIANT dbv;

	UIN2ID(DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0), uin);
	if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv)) {
		CallService(MS_DB_CRYPT_DECODESTRING, (WPARAM)(int)strlen(dbv.pszVal) + 1, (LPARAM)dbv.pszVal);
		password = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_TOKEN, &dbv)) {
		token = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_TOKENSECRET, &dbv)) {
		CallService(MS_DB_CRYPT_DECODESTRING, (WPARAM)(int)strlen(dbv.pszVal) + 1, (LPARAM)dbv.pszVal);
		token_secret = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	res = oauth_auth_header(httpmethod, url, HMACSHA1, uin, password, token, token_secret);
	mir_free(password);
	mir_free(token);
	mir_free(token_secret);

	return res;
}

int gg_oauth_receivetoken(GGPROTO *gg)
{
	NETLIBHTTPHEADER httpHeaders[3];
	NETLIBHTTPREQUEST req = {0};
	NETLIBHTTPREQUEST *resp;
	char szUrl[256], uin[32], *password = NULL, *str, *token = NULL, *token_secret = NULL;
	DBVARIANT dbv;
	int res = 0;

	UIN2ID(DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0), uin);
	if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv)) {
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM)dbv.pszVal);
		password = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	// 1. Obtaining an Unauthorized Request Token
	gg_netlog(gg, "gg_oauth_receivetoken(): Obtaining an Unauthorized Request Token...");
	strcpy(szUrl, "http://api.gadu-gadu.pl/request_token");
	str = oauth_auth_header("POST", szUrl, HMACSHA1, uin, password, NULL, NULL);

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_POST;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
	req.headersCount = 3;
	req.headers = httpHeaders;
	httpHeaders[0].szName   = "User-Agent";
	httpHeaders[0].szValue = GG8_VERSION;
	httpHeaders[1].szName  = "Authorization";
	httpHeaders[1].szValue = str;
	httpHeaders[2].szName  = "Accept";
	httpHeaders[2].szValue = "*/*";

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			HXML hXml;
			TCHAR *xmlAction;
			TCHAR *tag;

			xmlAction = gg_a2t(resp->pData);
			tag = gg_a2t("result");
			hXml = xi.parseString(xmlAction, 0, tag);
			if (hXml != NULL) {
				HXML node;

				mir_free(tag); tag = gg_a2t("oauth_token");
				node = xi.getChildByPath(hXml, tag, 0);
				token = node != NULL ? gg_t2a(xi.getText(node)) : NULL;

				mir_free(tag); tag = gg_a2t("oauth_token_secret");
				node = xi.getChildByPath(hXml, tag, 0);
				token_secret = node != NULL ? gg_t2a(xi.getText(node)) : NULL;

				xi.destroyNode(hXml);
			}
			mir_free(tag);
			mir_free(xmlAction);
		}
		else gg_netlog(gg, "gg_oauth_receivetoken(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else gg_netlog(gg, "gg_oauth_receivetoken(): No response from HTTP request");

	// 2. Obtaining User Authorization
	gg_netlog(gg, "gg_oauth_receivetoken(): Obtaining User Authorization...");
	mir_free(str);
	str = oauth_uri_escape("http://www.mojageneracja.pl");

	mir_snprintf(szUrl, 256, "callback_url=%s&request_token=%s&uin=%s&password=%s",
			str, token, uin, password); 
	mir_free(str);
	str = mir_strdup(szUrl);

	ZeroMemory(&req, sizeof(req));
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
	req.dataLength = (int)strlen(str);

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
	if (resp) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	else gg_netlog(gg, "gg_oauth_receivetoken(): No response from HTTP request");

	// 3. Obtaining an Access Token
	gg_netlog(gg, "gg_oauth_receivetoken(): Obtaining an Access Token...");
	strcpy(szUrl, "http://api.gadu-gadu.pl/access_token");
	mir_free(str);
	str = oauth_auth_header("POST", szUrl, HMACSHA1, uin, password, token, token_secret);
	mir_free(token);
	mir_free(token_secret);
	token = NULL;
	token_secret = NULL;

	ZeroMemory(&req, sizeof(req));
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_POST;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
	req.headersCount = 3;
	req.headers = httpHeaders;
	httpHeaders[1].szName  = "Authorization";
	httpHeaders[1].szValue = str;
	req.pData = NULL;
	req.dataLength = 0;

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			HXML hXml;
			TCHAR *xmlAction;
			TCHAR *tag;

			xmlAction = gg_a2t(resp->pData);
			tag = gg_a2t("result");
			hXml = xi.parseString(xmlAction, 0, tag);
			if (hXml != NULL) {
				HXML node;

				mir_free(tag); tag = gg_a2t("oauth_token");
				node = xi.getChildByPath(hXml, tag, 0);
				token = node != NULL ? gg_t2a(xi.getText(node)) : NULL;

				mir_free(tag); tag = gg_a2t("oauth_token_secret");
				node = xi.getChildByPath(hXml, tag, 0);
				token_secret = node != NULL ? gg_t2a(xi.getText(node)) : NULL;

				xi.destroyNode(hXml);
			}
			mir_free(tag);
			mir_free(xmlAction);
		}
		else gg_netlog(gg, "gg_oauth_receivetoken(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else gg_netlog(gg, "gg_oauth_receivetoken(): No response from HTTP request");

	mir_free(password);
	mir_free(str);

	if (token != NULL && token_secret != NULL) {
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_TOKEN, token);
		CallService(MS_DB_CRYPT_ENCODESTRING, (WPARAM)(int)strlen(token_secret) + 1, (LPARAM) token_secret);
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_TOKENSECRET, token_secret);
		gg_netlog(gg, "gg_oauth_receivetoken(): Access Token obtained successfully.");
		res = 1;
	}
	else {
		DBDeleteContactSetting(NULL, GG_PROTO, GG_KEY_TOKEN);
		DBDeleteContactSetting(NULL, GG_PROTO, GG_KEY_TOKENSECRET);
		gg_netlog(gg, "gg_oauth_receivetoken(): Failed to obtain Access Token.");
	}
	mir_free(token);
	mir_free(token_secret);

	return res;
}

int gg_oauth_checktoken(GGPROTO *gg, int force)
{
	if (!force) {
		char *token = NULL, *token_secret = NULL;
		DBVARIANT dbv;
		int res = 1;

		if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_TOKEN, &dbv)) {
			token = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_TOKENSECRET, &dbv)) {
			CallService(MS_DB_CRYPT_DECODESTRING, (WPARAM)(int)strlen(dbv.pszVal) + 1, (LPARAM)dbv.pszVal);
			token_secret = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		if (token == NULL || token_secret == NULL) {
			res = gg_oauth_receivetoken(gg);
		}

		mir_free(token);
		mir_free(token_secret);

		return res;
	}

	return gg_oauth_receivetoken(gg);
}
