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

void CVkProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
LBL_Restart:
	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)pReq);
	if (reply != NULL) {
		if (pReq->m_pFunc != NULL)
			(this->*(pReq->m_pFunc))(reply, pReq);
		
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
		if (pReq->bNeedsRestart)
			goto LBL_Restart;
	}
	delete pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::PushAsyncHttpRequest(int iRequestType, LPCSTR szUrl, bool bSecure, VK_REQUEST_HANDLER pFunc, int nParams, HttpParam *pParams, int iTimeout)
{
	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->flags = NLHRF_NODUMPHEADERS | NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (bSecure)
		pReq->flags |= NLHRF_SSL;

	CMStringA url;
	if (*szUrl == '/') {
		url = VK_API_URL;
		url += szUrl;
	}
	else url = szUrl;
	
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
	debugLogA("CVkProto::WorkerThread: entering");
	m_bTerminated = m_prevError = false;
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

	m_hWorkerThread = 0;
	debugLogA("CVkProto::WorkerThread: leaving");
}
