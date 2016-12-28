/*
Copyright © 2016 Miranda NG team

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

AsyncHttpRequest* CDiscordProto::Push(AsyncHttpRequest *pReq, int iTimeout)
{
	pReq->timeout = iTimeout;
	{
		mir_cslock lck(m_csHttpQueue);
		m_arHttpQueue.insert(pReq);
	}
	SetEvent(m_evRequestsQueue);
	return pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LONG g_reqNum = 0;

AsyncHttpRequest::AsyncHttpRequest()
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	AddHeader("Connection", "keep-alive");
	pUserInfo = NULL;
	m_iErrorCode = 0;
	m_pCallback = NULL;
	m_iReqNum = ::InterlockedIncrement(&g_reqNum);
}

AsyncHttpRequest::AsyncHttpRequest(CDiscordProto *ppro, int iRequestType, LPCSTR _url, HttpCallback pFunc)
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	AddHeader("Connection", "keep-alive");
		
	if (*_url == '/') {	// relative url leads to a site
		m_szUrl = "https://discardapp.com";
		m_szUrl += _url;
	}
	else m_szUrl = _url;

	flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_SSL;
	if (ppro->m_szAccessToken != NULL)
		this << CHAR_PARAM("access_token", ppro->m_szAccessToken);

	requestType = iRequestType;
	m_pCallback = pFunc;
	pUserInfo = NULL;
	m_iErrorCode = 0;
	m_iReqNum = ::InterlockedIncrement(&g_reqNum);
}

AsyncHttpRequest::~AsyncHttpRequest()
{
	for (int i = 0; i < headersCount; i++) {
		mir_free(headers[i].szName);
		mir_free(headers[i].szValue);
	}
	mir_free(headers);
	mir_free(pData);
}

void AsyncHttpRequest::AddHeader(LPCSTR szName, LPCSTR szValue)
{
	headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
	headers[headersCount].szName = mir_strdup(szName);
	headers[headersCount].szValue = mir_strdup(szValue);
	headersCount++;
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const INT_PARAM &param)
{
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%i", param.szName, param.iValue);
	return pReq;
}

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const CHAR_PARAM &param)
{
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, ptrA(mir_urlEncode(param.szValue)));
	return pReq;
}

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const WCHAR_PARAM &param)
{
	T2Utf szValue(param.wszValue);
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, ptrA(mir_urlEncode(szValue)));
	return pReq;
}
