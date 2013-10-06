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
	int bytesSent = CallService(MS_NETLIB_SENDHTTPREQUEST, (WPARAM)m_hNetlibConn, (LPARAM)pReq);
	if (bytesSent > 0) {
		NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_RECVHTTPHEADERS, (WPARAM)m_hNetlibConn, 0);
		if (reply != NULL) {
			(this->*(pReq->m_pFunc))(reply);
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
		}
	}
	delete pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

static NETLIBHTTPHEADER hdrs[3] =
{
	{ "Connection", "keep-alive" },
	{ "User-Agent", "Mozilla/4.0 (compatible; MSIE 5.5)" },
	{ "Host", VK_API_URL }
};

bool CVkProto::PushAsyncHttpRequest(int iRequestType, LPCSTR szUrl, bool bSecure, VK_REQUEST_HANDLER pFunc, int iTimeout)
{
	if ( !SetupConnection())
		return false;

	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->cbSize = sizeof(NETLIBHTTPREQUEST);
	pReq->requestType = iRequestType;
	pReq->headers = hdrs;
	pReq->headersCount = SIZEOF(hdrs);
	pReq->szUrl = mir_strdup(szUrl);
	pReq->nlc = m_hNetlibConn;
	pReq->flags = NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_NODUMP;
	if (bSecure)
		pReq->flags |= NLHRF_SSL;
	pReq->m_expireTime = time(0) + iTimeout;
	pReq->m_pFunc = pFunc;
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
	m_szAccessToken = getStringA("AccessToken");
	if (m_szAccessToken != NULL)
		RequestMyInfo();
	else { // Initialize new OAuth session
		CMStringA szUrl;
		szUrl.Format("/oauth/authorize?client_id=%d&scope=%s&redirect_uri=%s&display=wap&response_type=token",
			VK_APP_ID, "friends,photos,audio,video,wall,messages,offline", VK_REDIRECT_URL);
		PushAsyncHttpRequest(REQUEST_GET, szUrl, false, &CVkProto::OnOAuthAuthorize);
	}

	while(true) {
		DWORD dwRet = WaitForSingleObject(m_evRequestsQueue, 1000);
		if (dwRet == WAIT_TIMEOUT) {
			// check expiration;
			continue;
		}

		if (dwRet != WAIT_OBJECT_0)
			continue;

		if (m_bTerminated)
			break;

		AsyncHttpRequest *pReq;
		{	mir_cslock lck(m_csRequestsQueue);
			if (m_arRequestsQueue.getCount() == 0)
				continue;

			pReq = m_arRequestsQueue[0];
			m_arRequestsQueue.remove(0);
		}
		ExecuteRequest(pReq);
	}

	OnLoggedOut();
}
