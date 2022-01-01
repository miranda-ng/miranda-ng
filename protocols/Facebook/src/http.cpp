/*

Facebook plugin for Miranda NG
Copyright © 2019-22 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareParams(const AsyncHttpRequest::Param *p1, const AsyncHttpRequest::Param *p2)
{
	return strcmp(p1->key, p2->key);
}

AsyncHttpRequest::AsyncHttpRequest() :
	params(5, CompareParams)
{
}

void AsyncHttpRequest::CalcSig()
{
	CMStringA buf;
	for (auto &it : params)
		buf.AppendFormat("%s=%s", it->key.c_str(), it->val.c_str());

	buf.Append(FB_API_SECRET);

	char szHash[33];
	uint8_t digest[16];
	mir_md5_hash((uint8_t*)buf.c_str(), buf.GetLength(), digest);
	bin2hex(digest, sizeof(digest), szHash);
	this << CHAR_PARAM("sig", szHash);

	for (auto &it : params) {
		if (!m_szParam.IsEmpty())
			m_szParam.AppendChar('&');
		m_szParam.AppendFormat("%s=%s", it->key.c_str(), mir_urlEncode(it->val.c_str()).c_str());
	}
}

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const CHAR_PARAM &param)
{
	pReq->params.insert(new AsyncHttpRequest::Param(param.szName, param.szValue));
	return pReq;
}

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const INT_PARAM &param)
{
	char value[40];
	itoa(param.iValue, value, 10);
	pReq->params.insert(new AsyncHttpRequest::Param(param.szName, value));
	return pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

JsonReply::JsonReply(NETLIBHTTPREQUEST *pReply)
{
	if (pReply == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = pReply->resultCode;
	if (m_errorCode != 200)
		return;

	m_root = json_parse(pReply->pData);
	if (m_root == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = (*m_root)["error_code"].as_int();
}

JsonReply::~JsonReply()
{
	json_delete(m_root);
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest* FacebookProto::CreateRequest(const char *szUrl, const char *szName, const char *szMethod)
{
	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->m_szUrl = szUrl;
	pReq->requestType = REQUEST_POST;
	pReq << CHAR_PARAM("api_key", FB_API_KEY) 
		<< CHAR_PARAM("device_id", m_szDeviceID) 
		<< CHAR_PARAM("fb_api_req_friendly_name", szName)
		<< CHAR_PARAM("format", "json") 
		<< CHAR_PARAM("method", szMethod);

	CMStringA szLocale = getMStringA(DBKEY_LOCALE);
	if (szLocale.IsEmpty())
		szLocale = "en";
	pReq << CHAR_PARAM("locale", szLocale);

	if (!m_szAuthToken.IsEmpty()) {
		pReq->flags |= NLHRF_NODUMPHEADERS;
		pReq->AddHeader("Authorization", "OAuth " + m_szAuthToken);
	}

	pReq->AddHeader("User-Agent", FB_API_AGENT);
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
	return pReq;
}

AsyncHttpRequest* FacebookProto::CreateRequestGQL(int64_t query_id) {
	const char* szName;

	switch (query_id) {
		case FB_API_QUERY_CONTACT:
			szName = "UsersQuery";
			break;
		case FB_API_QUERY_CONTACTS:
			szName = "FetchContactsFullQuery";
			break;
		case FB_API_QUERY_CONTACTS_AFTER:
			szName = "FetchContactsFullWithAfterQuery";
			break;
		case FB_API_QUERY_CONTACTS_DELTA:
			szName = "FetchContactsDeltaQuery";
			break;
		case FB_API_QUERY_STICKER:
			szName = "FetchStickersWithPreviewsQuery";
			break;
		case FB_API_QUERY_THREAD:
			szName = "ThreadQuery";
			break;
		case FB_API_QUERY_SEQ_ID:
		case FB_API_QUERY_THREADS:
			szName = "ThreadListQuery";
			break;
		case FB_API_QUERY_XMA:
			szName = "XMAQuery";
			break;
		default:
			return nullptr;
	}

	AsyncHttpRequest* pReq = CreateRequest(FB_API_URL_GQL, szName, "get");
	pReq << INT64_PARAM("query_id", query_id);
	return pReq;
}

NETLIBHTTPREQUEST* FacebookProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	CMStringA str;

	pReq->flags |= NLHRF_HTTP11;
	pReq->szUrl = pReq->m_szUrl.GetBuffer();
	if (!pReq->m_szParam.IsEmpty()) {
		if (pReq->requestType == REQUEST_GET) {
			str.Format("%s?%s", pReq->m_szUrl.c_str(), pReq->m_szParam.c_str());
			pReq->szUrl = str.GetBuffer();
		}
		else {
			pReq->dataLength = pReq->m_szParam.GetLength();
			pReq->pData = mir_strdup(pReq->m_szParam);
		}
	}

	debugLogA("Executing request:\n%s", pReq->szUrl);

	NETLIBHTTPREQUEST *reply = Netlib_HttpTransaction(m_hNetlibUser, pReq);
	delete pReq;
	return reply;
}
