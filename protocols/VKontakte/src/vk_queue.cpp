/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

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
	debugLogA("CVkProto::InitQueue");
	m_evRequestsQueue = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CVkProto::UninitQueue()
{
	debugLogA("CVkProto::UninitQueue");
	m_arRequestsQueue.destroy();
	CloseHandle(m_evRequestsQueue);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	CMStringA str;
	do {
		pReq->bNeedsRestart = false;
		pReq->szUrl = pReq->m_szUrl.GetBuffer();
		if (!pReq->m_szParam.IsEmpty()) {
			if (pReq->requestType == REQUEST_GET) {
				str.Format("%s?%s", pReq->m_szUrl, pReq->m_szParam);
				pReq->szUrl = str.GetBuffer();
			}
			else {
				pReq->pData = mir_strdup(pReq->m_szParam);
				pReq->dataLength = pReq->m_szParam.GetLength();
			}
		}

		if (pReq->m_bApiReq) {
			pReq->flags |= NLHRF_PERSISTENT;
			pReq->nlc = m_hAPIConnection;
		}

		debugLogA("CVkProto::ExecuteRequest \n====\n%s\n====\n", pReq->szUrl);
		NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)pReq);
		if (reply != NULL) {
			if (pReq->m_pFunc != NULL)
				(this->*(pReq->m_pFunc))(reply, pReq); // may be set pReq->bNeedsRestart 

			if (pReq->m_bApiReq)
				m_hAPIConnection = reply->nlc;

			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
		}
		else if (pReq->bIsMainConn) {
			if (IsStatusConnecting(m_iStatus))
				ConnectionFailed(LOGINERR_NONETWORK);
			else if (pReq->m_iRetry && !m_bTerminated) {
				pReq->bNeedsRestart = true;
				Sleep(1000); //Pause for fix err 
				pReq->m_iRetry--;
				debugLogA("CVkProto::ExecuteRequest restarting (retry = %d)", MAX_RETRIES - pReq->m_iRetry);
			}
			else {
				debugLogA("CVkProto::ExecuteRequest ShutdownSession");
				ShutdownSession();
			}
		}
		debugLogA("CVkProto::ExecuteRequest pReq->bNeedsRestart = %d", (int)pReq->bNeedsRestart);

		if (!reply && pReq->m_bApiReq)
			m_hAPIConnection = NULL;

	} while (pReq->bNeedsRestart && !m_bTerminated);
	delete pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest* CVkProto::Push(AsyncHttpRequest *pReq, int iTimeout)
{
	debugLogA("CVkProto::Push");
	pReq->timeout = iTimeout;
	if (pReq->m_bApiReq) {
		pReq << VER_API;
		if (!IsEmpty(m_VKLang)) 
			pReq << TCHAR_PARAM("lang", m_VKLang);
	}

	{
		mir_cslock lck(m_csRequestsQueue);
		m_arRequestsQueue.insert(pReq);
	}
	SetEvent(m_evRequestsQueue);
	return pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::WorkerThread(void*)
{
	debugLogA("CVkProto::WorkerThread: entering");
	m_bTerminated = m_prevError = false;
	m_szAccessToken = getStringA("AccessToken");

	char Score[] = "friends,photos,audio,docs,video,wall,messages,offline,status,notifications,groups";

	CMStringA szAccessScore(ptrA(getStringA("AccessScore")));
	if (szAccessScore != Score) {
		setString("AccessScore", Score);
		delSetting("AccessToken");
		m_szAccessToken = NULL;
	}

	if (m_szAccessToken != NULL)
		// try to receive a response from server
		RetrieveMyInfo();
	else {
		// Initialize new OAuth session
		extern char szBlankUrl[];
		AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "https://oauth.vk.com/authorize", false, &CVkProto::OnOAuthAuthorize)
			<< INT_PARAM("client_id", VK_APP_ID)
			<< CHAR_PARAM("scope", Score)
			<< CHAR_PARAM("redirect_uri", szBlankUrl)
			<< CHAR_PARAM("display", "mobile")
			<< CHAR_PARAM("response_type", "token")
			<< VER_API;
		pReq->m_bApiReq = false;
		pReq->bIsMainConn = true;
		Push(pReq);
	}

	m_hAPIConnection = NULL;

	while (true) {
		WaitForSingleObject(m_evRequestsQueue, 1000);
		if (m_bTerminated)
			break;

		AsyncHttpRequest *pReq;
		bool need_sleep = false;
		while (true) {
			{
				mir_cslock lck(m_csRequestsQueue);
				if (m_arRequestsQueue.getCount() == 0)
					break;

				pReq = m_arRequestsQueue[0];
				m_arRequestsQueue.remove(0);
				need_sleep = (m_arRequestsQueue.getCount() > 1) && (pReq->m_bApiReq); // more than two to not gather
			}
			if (m_bTerminated)
				break;
			ExecuteRequest(pReq);
			if (need_sleep) { // There can be maximum 3 requests to API methods per second from a client
				Sleep(330);	// (c) https://vk.com/dev/api_requests
				debugLogA("CVkProto::WorkerThread: need sleep");
			}
		}
	}

	if (m_hAPIConnection) 
		Netlib_CloseHandle(m_hAPIConnection);

	m_hAPIConnection = NULL;
	m_hWorkerThread = 0;
	debugLogA("CVkProto::WorkerThread: leaving");
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
	s.AppendFormat("%s=%s", param.szName, ptrA(pReq->bExpUrlEncode ? ExpUrlEncode(param.szValue) : mir_urlEncode(param.szValue)));
	return pReq;
}

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const TCHAR_PARAM &param)
{
	T2Utf szValue(param.tszValue);
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, ptrA(pReq->bExpUrlEncode ? ExpUrlEncode(szValue) : mir_urlEncode(szValue)));
	return pReq;
}