/*
Copyright (C) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// The legacy OK REST API (api.ok.ru) the way OKLumessenger calls it, which in turn copies the
// official Windows Phone client v4.9: the method name is the path, parameters go in the query
// (or a form body once the query reaches 1000 characters), and every call is signed.
//
// Signature (Ok.Api.UrlUtilities.QueryBuilder.CalculateSignature): every parameter except "url"
// takes part - the session key too - as "key=value" strings sorted ordinally as whole strings,
// concatenated, followed by the secret, MD5 in lowercase hex. The secret is the application
// secret for login and the session secret for every call after it.

#include "stdafx.h"

#define OK_API_BASE "https://api.ok.ru/"

static CMStringA Md5Hex(const CMStringA &str)
{
	uint8_t digest[16];
	mir_md5_hash((const uint8_t *)str.c_str(), str.GetLength(), digest);

	char hex[33];
	bin2hex(digest, sizeof(digest), hex);
	return CMStringA(hex).MakeLower();
}

static CMStringA Signature(const std::map<CMStringA, CMStringA> &params, const char *secret)
{
	std::vector<CMStringA> pairs;
	for (auto &it : params)
		if (it.first != "url")
			pairs.push_back(it.first + "=" + it.second);
	std::sort(pairs.begin(), pairs.end(), [](const CMStringA &a, const CMStringA &b) { return strcmp(a, b) < 0; });

	CMStringA str;
	for (auto &it : pairs)
		str += it;
	str += secret;
	return Md5Hex(str);
}

OkReply COdnoklassnikiProto::Call(const char *szMethod, const std::map<CMStringA, CMStringA> &extra, bool bLogin)
{
	std::map<CMStringA, CMStringA> params(extra);
	params["application_key"] = OK_APP_KEY;
	params["format"] = "json";

	CMStringA szSecret, szServer;
	if (bLogin) {
		szSecret = OK_APP_SECRET;
		szServer = OK_API_BASE;
	}
	else {
		mir_cslock lck(m_csSession);
		params["session_key"] = m_szSessionKey;
		szSecret = m_szSessionSecret;
		szServer = m_szApiServer.IsEmpty() ? OK_API_BASE : m_szApiServer;
	}
	params["sig"] = Signature(params, szSecret);

	CMStringA szQuery;
	for (auto &it : params) {
		if (!szQuery.IsEmpty())
			szQuery.AppendChar('&');
		szQuery += mir_urlEncode(it.first) + "=" + mir_urlEncode(it.second);
	}

	// the official client switches to POST once the query gets long; the body is the same.
	// Login carries the password or the auth token: it is always a POST and never logged
	MHttpRequest req((bLogin || szQuery.GetLength() >= 1000) ? REQUEST_POST : REQUEST_GET);
	req.flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NOUSERAGENT | (bLogin ? NLHRF_NODUMP : NLHRF_NODUMPSEND);
	req.timeout = 40000;
	req.m_szUrl = szServer + szMethod;
	req.AddHeader("User-Agent", OK_USER_AGENT);
	if (req.requestType == REQUEST_POST) {
		req.AddHeader("Content-Type", "application/x-www-form-urlencoded");
		req.m_szParam = szQuery;
	}
	else req.m_szUrl += "?" + szQuery;

	OkReply ret;
	NLHR_PTR reply(Netlib_HttpTransaction(m_hNetlibUser, &req));
	if (reply == nullptr) {
		ret.code = -1;
		ret.error = "no connection";
		return ret;
	}

	ret.root = JSONNode::parse(reply->body);
	if (!ret.root) {
		ret.code = -1;
		ret.error.Format("HTTP %d, unparseable reply", reply->resultCode);
		return ret;
	}

	// a top-level array is wrapped, so that callers always get an object
	if (ret.root.type() == JSON_ARRAY) {
		JSONNode wrap;
		JSONNode items(ret.root);
		items.set_name("items");
		wrap.push_back(items);
		ret.root = wrap;
	}

	auto &err = ret.root["error_code"];
	if (!err.isnull()) {
		ret.code = err.as_int();
		ret.error = ret.root["error_msg"].as_string().c_str();
		debugLogA("%s failed: %d %s", szMethod, ret.code, ret.error.c_str());
	}
	return ret;
}

// api/batch/execute with a JSON array of {"method":..., "params":{...}}; each result comes back
// under "<method with the dot replaced by _>_response"
OkReply COdnoklassnikiProto::Batch(const CMStringA &szMethodsJson)
{
	std::map<CMStringA, CMStringA> params;
	params["methods"] = szMethodsJson;
	return Call("api/batch/execute", params);
}

/////////////////////////////////////////////////////////////////////////////////////////
// photo upload: photosV2/getUploadUrl gives a one-shot url; the image goes there as a
// multipart/form-data file part (a raw octet body is rejected), and the reply
// {"photos":{"<photoId>":{"token":"..."}}} carries the token messagesV2/send takes

bool COdnoklassnikiProto::UploadPhoto(const wchar_t *pwszFile, CMStringA &szToken)
{
	FILE *in = _wfopen(pwszFile, L"rb");
	if (in == nullptr) {
		debugLogW(L"cannot open %s", pwszFile);
		return false;
	}

	size_t cbFile = _filelength(_fileno(in));
	std::vector<char> data(cbFile);
	size_t cbRead = fread(data.data(), 1, cbFile, in);
	fclose(in);
	if (cbRead != cbFile || cbFile == 0)
		return false;

	auto urlReply = Call("api/photosV2/getUploadUrl", {});
	CMStringA szUploadUrl = urlReply ? urlReply.root["upload_url"].as_string().c_str() : "";
	if (szUploadUrl.IsEmpty()) {
		debugLogA("getUploadUrl returned no upload_url");
		return false;
	}

	char szRandom[16], szRandomText[33];
	Utils_GetRandom(szRandom, sizeof(szRandom));
	bin2hex(szRandom, sizeof(szRandom), szRandomText);
	CMStringA szBoundary(FORMAT, "----MirandaOk%s", szRandomText);

	const wchar_t *pwszName = wcsrchr(pwszFile, '\\');
	pwszName = pwszName ? pwszName + 1 : pwszFile;

	CMStringA szHead;
	szHead.AppendFormat("--%s\r\n", szBoundary.c_str());
	szHead.AppendFormat("Content-Disposition: form-data; name=\"pic1\"; filename=\"%s\"\r\n", T2Utf(pwszName).get());
	szHead.AppendFormat("Content-Type: %s\r\n\r\n", ProtoGetAvatarMimeType(ProtoGetAvatarFileFormat(pwszFile)));
	CMStringA szTail(FORMAT, "\r\n--%s--\r\n", szBoundary.c_str());

	MHttpRequest req(REQUEST_POST);
	req.flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NOUSERAGENT | NLHRF_NODUMPSEND;
	req.timeout = 120000;
	req.m_szUrl = szUploadUrl;
	req.AddHeader("User-Agent", OK_USER_AGENT);
	req.AddHeader("Content-Type", "multipart/form-data; boundary=" + szBoundary);

	size_t cbTotal = szHead.GetLength() + cbFile + szTail.GetLength();
	char *p = req.m_szParam.GetBuffer(int(cbTotal));
	memcpy(p, szHead.c_str(), szHead.GetLength());
	memcpy(p + szHead.GetLength(), data.data(), cbFile);
	memcpy(p + szHead.GetLength() + cbFile, szTail.c_str(), szTail.GetLength());
	req.m_szParam.ReleaseBuffer(int(cbTotal));

	NLHR_PTR reply(Netlib_HttpTransaction(m_hNetlibUser, &req));
	if (reply == nullptr || reply->resultCode != 200) {
		debugLogA("photo upload failed: HTTP %d", reply ? reply->resultCode : 0);
		return false;
	}

	JSONNode root = JSONNode::parse(reply->body);
	for (auto &it : root["photos"]) {
		szToken = it["token"].as_string().c_str();
		if (!szToken.IsEmpty())
			return true;
	}

	debugLogA("photo upload returned no token: %s", reply->body.Left(200).c_str());
	return false;
}
