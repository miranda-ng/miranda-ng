#include "stdafx.h"

bool CSteamProto::SendRequest(HttpRequest *request)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, request->Get());
	HttpResponse response(pResp);
	delete request;
	return response.IsSuccess();
}

bool CSteamProto::SendRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, request->Get());
	HttpResponse response(pResp);
	if (callback)
		(this->*callback)(response, param);
	delete request;
	return response.IsSuccess();
}

bool CSteamProto::SendRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, request->Get());
	HttpResponse response(pResp);
	if (callback) {
		JSONNode root = JSONNode::parse(response.data());
		(this->*callback)(root, param);
	}
	delete request;
	return response.IsSuccess();
}

/////////////////////////////////////////////////////////////////////////////////////////
// HttpRequest

HttpRequest::HttpRequest(int iRequestType, const char *szUrl)
{
	flags = NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_DUMPASTEXT;
	requestType = iRequestType;
	timeout = 30000;
	if (szUrl)
		m_szUrl = szUrl;
}

NETLIBHTTPREQUEST* HttpRequest::Get()
{
	if (m_szUrl[0]== '/') {
		m_szUrl.Insert(0, STEAM_API_URL);
		AddHeader("User-Agent", STEAM_USER_AGENT);
	}
	szUrl = m_szUrl.GetBuffer();
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// HttpResponse

LIST<NETLIBHTTPHEADER> HttpResponse::Headers() const
{
	LIST<NETLIBHTTPHEADER> ret(10);

	if (m_response)
		for (int i = 0; i < m_response->headersCount; i++)
			ret.insert(&m_response->headers[i]);
	
	return ret;
}
