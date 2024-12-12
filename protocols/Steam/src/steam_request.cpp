#include "stdafx.h"

bool CSteamProto::SendRequest(HttpRequest *request)
{
	if (auto *pResp = Netlib_HttpTransaction(m_hNetlibUser, request->Get())) {
		bool bSuccess = pResp->resultCode == 200;
		delete pResp;
		return bSuccess;
	}
	return false;
}

bool CSteamProto::SendRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	if (auto *pResp = Netlib_HttpTransaction(m_hNetlibUser, request->Get())) {
		bool bSuccess = pResp->resultCode == 200;
		if (callback)
			(this->*callback)(*pResp, param);
		delete request;
		delete pResp;
		return bSuccess;
	}
	return false;
}

bool CSteamProto::SendRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	if (auto *pResp = Netlib_HttpTransaction(m_hNetlibUser, request->Get())) {
		bool bSuccess = pResp->resultCode == 200;
		if (callback) {
			JSONNode root = JSONNode::parse(pResp->body);
			(this->*callback)(root, param);
		}
		delete request;
		delete pResp;
		return bSuccess;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// HttpRequest

#define STEAM_USER_AGENT "Valve/Steam HTTP Client 1.0"

HttpRequest::HttpRequest(int iRequestType, const char *szUrl)
{
	flags = NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_DUMPASTEXT;
	requestType = iRequestType;
	timeout = 30000;
	if (szUrl)
		m_szUrl = szUrl;
}

MHttpRequest* HttpRequest::Get()
{
	if (m_szUrl[0]== '/') {
		m_szUrl.Insert(0, STEAM_API_URL);
		AddHeader("User-Agent", STEAM_USER_AGENT);
	}
	return this;
}
