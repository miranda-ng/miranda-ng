/*
Copyright © 2012-22 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static CMStringA OAuthNormalizeUrl(const CMStringA &url)
{
	CMStringA normalUrl = url;

	int idx = normalUrl.Find('#');
	if (idx != -1)
		normalUrl.Truncate(idx);

	idx = normalUrl.Find('?');
	if (idx != -1)
		normalUrl.Truncate(idx);

	return normalUrl;
}

static int CompareCMString(const CMStringA *p1, const CMStringA *p2)
{
	return mir_strcmp(p1->c_str(), p2->c_str());
}

static CMStringA OAuthNormalizeRequestParameters(const CMStringA &requestParameters, bool bShort)
{
	OBJLIST<CMStringA> params(10);
	Split(requestParameters, params, '&');

	LIST<CMStringA> sorted(params.getCount(), CompareCMString);
	for (auto &it : params)
		sorted.insert(it);

	CMStringA res;
	for (auto &it : sorted) {
		if (!res.IsEmpty())
			res.AppendChar(bShort ? '&' : ',');

		if (!bShort) {
			it->Replace("=", "=\"");
			it->AppendChar('\"');
		}
		res += *it;
	}

	return res;
}

CMStringA CTwitterProto::BuildSignedOAuthParameters(const CMStringA &request, const CMStringA &url, const char *httpMethod, const char *postData)
{
	CMStringA timestamp(FORMAT, "%lld", _time64(0));
	CMStringA nonce = OAuthCreateNonce();

	// create oauth requestParameters
	auto *req = new AsyncHttpRequest(!mir_strcmp(httpMethod, "GET") ? REQUEST_GET : REQUEST_POST, url);
	req << CHAR_PARAM("oauth_timestamp", timestamp) << CHAR_PARAM("oauth_nonce", nonce) << CHAR_PARAM("oauth_version", "1.0")
		<< CHAR_PARAM("oauth_signature_method", "HMAC-SHA1") << CHAR_PARAM("oauth_consumer_key", OAUTH_CONSUMER_KEY) << CHAR_PARAM("oauth_callback", "oob");

	// add the request token if found
	if (!m_szAccessToken.IsEmpty())
		req << CHAR_PARAM("oauth_token", m_szAccessToken);

	// add the authorization pin if found
	if (!m_szPin.IsEmpty())
		req << CHAR_PARAM("oauth_verifier", m_szPin);

	// create a parameter list containing both oauth and original parameters
	// this will be used to create the parameter signature
	if (!request.IsEmpty()) {
		req->m_szParam.AppendChar('&');
		req->m_szParam.Append(request);
	}

	if (!mir_strlen(postData)) {
		req->m_szParam.AppendChar('&');
		req->m_szParam.Append(postData);
	}

	// prepare a signature base, a carefully formatted string containing 
	// all of the necessary information needed to generate a valid signature
	CMStringA normalUrl = OAuthNormalizeUrl(url);
	CMStringA normalizedParameters = OAuthNormalizeRequestParameters(req->m_szParam, true);
	CMStringA signatureBase = CMStringA(httpMethod) + "&" + mir_urlEncode(normalUrl) + "&" + mir_urlEncode(normalizedParameters);

	// obtain a signature and add it to header requestParameters
	CMStringA signature = OAuthCreateSignature(signatureBase, OAUTH_CONSUMER_SECRET, m_szAccessTokenSecret);
	req << CHAR_PARAM("oauth_signature", signature);

	CMStringA ret = OAuthNormalizeRequestParameters(req->m_szParam, false);
	delete req;
	return ret;
}

CMStringA CTwitterProto::UrlGetQuery(const CMStringA &url)
{
	CMStringA query = url;

	int idx = query.Find('?');
	if (idx == -1)
		return "";

	query.Delete(0, idx+1);

	idx = query.Find('#');
	if (idx != -1)
		query.Truncate(idx);

	return query;
}

// OAuthWebRequest used for all OAuth related queries
//
// consumerKey and consumerSecret - must be provided for every call, they identify the application
// oauthToken and oauthTokenSecret - need to be provided for every call, except for the first token request before authorizing
// pin - only used during authorization, when the user enters the PIN they received from the CTwitterProto website
CMStringA CTwitterProto::OAuthWebRequestSubmit(const CMStringA &url, const char *httpMethod, const char *postData)
{
	CMStringA query = UrlGetQuery(url);

	CMStringA oauthSignedParameters = BuildSignedOAuthParameters(query, url, httpMethod, postData);
	oauthSignedParameters.Insert(0, "OAuth ");
	return oauthSignedParameters;
}

static char ALPHANUMERIC[62+1] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

CMStringA CTwitterProto::OAuthCreateNonce()
{
	CMStringA nonce;

	for (int i = 0; i <= 16; ++i)
		nonce.AppendChar(ALPHANUMERIC[rand() % 62]); 

	return nonce;
}

CMStringA CTwitterProto::OAuthCreateSignature(const CMStringA &signatureBase, const CMStringA &consumerSecret, const CMStringA &requestTokenSecret)
{
	// URL encode key elements
	CMStringA key = mir_urlEncode(consumerSecret) + "&" + mir_urlEncode(requestTokenSecret);

	uint8_t digest[MIR_SHA1_HASH_SIZE];
	unsigned int len;
	HMAC(EVP_sha1(), key.c_str(), (int)key.GetLength(), (uint8_t*)signatureBase.c_str(), signatureBase.GetLength(), digest, &len);
	return CMStringA(ptrA(mir_base64_encode(digest, sizeof(digest))));
}
