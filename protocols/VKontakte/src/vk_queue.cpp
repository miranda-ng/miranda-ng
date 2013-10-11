/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

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

#include "stdafx.h"

void CVkProto::InitQueue()
{
	::InitializeCriticalSection(&m_csRequestsQueue);
	m_evRequestsQueue = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CVkProto::UninitQueue()
{
	m_arRequestsQueue.destroy();
	CloseHandle(m_evRequestsQueue);
	::DeleteCriticalSection(&m_csRequestsQueue);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CVkProto::SetupConnection()
{
	if (m_hNetlibConn != NULL)
		return TRUE;

	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.flags = NLOCF_SSL | NLOCF_HTTP | NLOCF_V2;
	nloc.szHost = VK_API_URL;
	nloc.wPort = 443;
	nloc.timeout = 5000;
	m_hNetlibConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
	return m_hNetlibConn != NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)pReq);
	if (reply != NULL) {
		if (pReq->m_pFunc != NULL)
			(this->*(pReq->m_pFunc))(reply, pReq->pUserInfo);
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
	}
	delete pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::PushAsyncHttpRequest(int iRequestType, LPCSTR szUrl, bool bSecure, VK_REQUEST_HANDLER pFunc, int nParams, HttpParam *pParams, int iTimeout)
{
	if ( !SetupConnection())
		return false;

	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->flags = NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (bSecure)
		pReq->flags |= NLHRF_SSL;

	CMStringA url;
	if (*szUrl == '/') {
		url = VK_API_URL;
		url += szUrl;
		pReq->nlc = m_hNetlibConn;
	}
	else {
		url = szUrl;
		pReq->flags |= NLHRF_REMOVEHOST | NLHRF_SMARTREMOVEHOST;
	}
	
	for (int i=0; i < nParams; i++) {
		url.AppendChar((i == 0) ? '?' : '&');
		url += pParams[i].szName;
		url.AppendChar('=');
		url += ptrA( mir_urlEncode(pParams[i].szValue));
	}

	pReq->requestType = iRequestType;
	pReq->szUrl = mir_strdup(url);
	pReq->m_pFunc = pFunc;
	return PushAsyncHttpRequest(pReq, iTimeout);
}

bool CVkProto::PushAsyncHttpRequest(AsyncHttpRequest *pReq, int iTimeout)
{
	pReq->timeout = iTimeout;
	{
		mir_cslock lck(m_csRequestsQueue);
		m_arRequestsQueue.insert(pReq);
	}
	SetEvent(m_evRequestsQueue);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::WorkerThread(void*)
{
	m_bTerminated = false;
	m_szAccessToken = getStringA("AccessToken");
	if (m_szAccessToken != NULL)
		OnLoggedIn();
	else { // Initialize new OAuth session
		HttpParam params[] = {
			{ "client_id", VK_APP_ID },
			{ "scope", "friends,photos,audio,video,wall,messages,offline" },
			{ "redirect_uri", VK_REDIRECT_URL },
			{ "display", "wap" },
			{ "response_type", "token" }
		};
		PushAsyncHttpRequest(REQUEST_GET, "/oauth/authorize", false, &CVkProto::OnOAuthAuthorize, SIZEOF(params), params);
	}

	while(true) {
		WaitForSingleObject(m_evRequestsQueue, 1000);
		if (m_bTerminated)
			break;

		AsyncHttpRequest *pReq;
		while(true) {
			{
				mir_cslock lck(m_csRequestsQueue);
				if (m_arRequestsQueue.getCount() == 0)
					break;

				pReq = m_arRequestsQueue[0];
				m_arRequestsQueue.remove(0);
			}
			ExecuteRequest(pReq);
		}
	}

	OnLoggedOut();
}
