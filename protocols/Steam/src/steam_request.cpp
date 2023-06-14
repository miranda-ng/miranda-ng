#include "stdafx.h"

bool CSteamProto::SendRequest(HttpRequest *request)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, request);
	HttpResponse response(pResp);
	delete request;
	return response.IsSuccess();
}

bool CSteamProto::SendRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(pResp);
	if (callback)
		(this->*callback)(response, param);
	delete request;
	return response.IsSuccess();
}

bool CSteamProto::SendRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
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
	requestType = iRequestType;
	timeout = 30000;
	if (szUrl)
		m_szUrl = szUrl;
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
