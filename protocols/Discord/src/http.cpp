/*
Copyright © 2016-18 Miranda NG team

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
	m_iReqNum = ::InterlockedIncrement(&g_reqNum);
}

AsyncHttpRequest::AsyncHttpRequest(CDiscordProto *ppro, int iRequestType, LPCSTR _url, MTHttpRequestHandler pFunc, JSONNode *pRoot)
{
	cbSize = sizeof(NETLIBHTTPREQUEST);

	if (*_url == '/') {	// relative url leads to a site
		m_szUrl = "https://discordapp.com/api/v6";
		m_szUrl += _url;
		m_bMainSite = true;
	}
	else {
		m_szUrl = _url;
		m_bMainSite = false;
	}

	flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_SSL;
	if (ppro->m_szAccessToken != nullptr) {
		AddHeader("Authorization", ppro->m_szAccessToken);
		flags |= NLHRF_DUMPASTEXT;
	}
	else flags |= NLHRF_NODUMPSEND;

	if (pRoot != nullptr) {
		ptrW text(json_write(pRoot));
		pData = mir_utf8encodeW(text);
		dataLength = (int)mir_strlen(pData);
	}
	AddHeader("Content-Type", "application/json");

	m_pFunc = pFunc;
	requestType = iRequestType;
	m_iErrorCode = 0;
	m_iReqNum = ::InterlockedIncrement(&g_reqNum);
}
