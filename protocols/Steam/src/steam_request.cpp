#include "stdafx.h"

bool CSteamProto::SendRequest(HttpRequest *request)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, request);
	HttpResponse response(request, pResp);
	delete request;
	return response.IsSuccess();
}

bool CSteamProto::SendRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	if (callback)
		(this->*callback)(response, param);
	delete request;
	return response.IsSuccess();
}

bool CSteamProto::SendRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	if (callback) {
		JSONNode root = JSONNode::parse(response.Content);
		(this->*callback)(root, param);
	}
	delete request;
	return response.IsSuccess();
}
