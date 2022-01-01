/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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
	m_evRequestsQueue = CreateEvent(nullptr, false, false, nullptr);
}

void CVkProto::UninitQueue()
{
	debugLogA("CVkProto::UninitQueue");
	m_arRequestsQueue.destroy();
	CloseHandle(m_evRequestsQueue);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	CMStringA str;
	do {
		pReq->bNeedsRestart = false;
		pReq->m_iErrorCode = 0;
		pReq->szUrl = pReq->m_szUrl.GetBuffer();
		if (!pReq->m_szParam.IsEmpty()) {
			if (pReq->requestType == REQUEST_GET) {
				str.Format("%s?%s", pReq->m_szUrl.c_str(), pReq->m_szParam.c_str());
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

		if (m_bTerminated)
			break;

		time_t tLocalWorkThreadTimer = 0;
		{
			mir_cslock lck(m_csWorkThreadTimer);
			tLocalWorkThreadTimer = m_tWorkThreadTimer = time(0);
		}

		debugLogA("CVkProto::ExecuteRequest \n====\n%s\n====\n", pReq->szUrl);
		NLHR_PTR reply(Netlib_HttpTransaction(m_hNetlibUser, pReq));
		{
			mir_cslock lck(m_csWorkThreadTimer);
			if (tLocalWorkThreadTimer != m_tWorkThreadTimer) {
				debugLogA("CVkProto::WorkerThread is living Dead => return");
				delete pReq;
				return false;
			}
		}

		if (reply != nullptr) {
			if (pReq->m_pFunc != nullptr)
				(this->*(pReq->m_pFunc))(reply, pReq); // may be set pReq->bNeedsRestart

			if (pReq->m_bApiReq)
				m_hAPIConnection = reply->nlc;
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
			CloseAPIConnection();

	} while (pReq->bNeedsRestart && !m_bTerminated);
	delete pReq;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest* CVkProto::Push(MHttpRequest *p, int iTimeout)
{
	AsyncHttpRequest *pReq = (AsyncHttpRequest*)p;

	debugLogA("CVkProto::Push");
	pReq->timeout = iTimeout;
	if (pReq->m_bApiReq) {
		pReq << VER_API;
		if (!IsEmpty(m_vkOptions.pwszVKLang))
			pReq << WCHAR_PARAM("lang", m_vkOptions.pwszVKLang);
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

	extern char szScore[];

	CMStringA szAccessScore(ptrA(getStringA("AccessScore")));
	if (szAccessScore != szScore) {
		setString("AccessScore", szScore);
		delSetting("AccessToken");
		m_szAccessToken = nullptr;
	}

	if (m_szAccessToken != nullptr)
		// try to receive a response from server
		RetrieveMyInfo();
	else {
		// Initialize new OAuth session
		extern char szBlankUrl[];
		extern char szVKUserAgent[];
		extern char szVKUserAgentCH[];

		AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "https://oauth.vk.com/authorize", false, &CVkProto::OnOAuthAuthorize);
		pReq
			<< INT_PARAM("client_id", VK_APP_ID)
			<< CHAR_PARAM("scope", szScore)
			<< CHAR_PARAM("redirect_uri", szBlankUrl)
			<< CHAR_PARAM("display", "mobile")
			<< CHAR_PARAM("response_type", "token")
			<< VER_API;

		// Headers
		pReq->AddHeader("User-agent", szVKUserAgent);
		pReq->AddHeader("dht", "1");
		pReq->AddHeader("origin", "https://oauth.vk.com");
		pReq->AddHeader("referer", "https://oauth.vk.com/");
		pReq->AddHeader("sec-ch-ua", szVKUserAgentCH);
		pReq->AddHeader("sec-ch-ua-mobile", "?0");
		pReq->AddHeader("sec-ch-ua-platform", "Windows");
		pReq->AddHeader("sec-fetch-dest", "document");
		pReq->AddHeader("sec-fetch-mode", "navigate");
		pReq->AddHeader("sec-fetch-site", "same-site");
		pReq->AddHeader("sec-fetch-user", "?1");
		pReq->AddHeader("upgrade-insecure-requests", "1");
		//Headers

		pReq->m_bApiReq = false;
		pReq->bIsMainConn = true;
		Push(pReq);
	}

	CloseAPIConnection();

	while (true) {
		WaitForSingleObject(m_evRequestsQueue, 1000);
		if (m_bTerminated)
			break;

		AsyncHttpRequest *pReq;
		ULONG uTime[3] = { 0, 0, 0 };
		long lWaitingTime = 0;

		while (true) {
			{
				mir_cslock lck(m_csRequestsQueue);
				if (m_arRequestsQueue.getCount() == 0)
					break;

				pReq = m_arRequestsQueue[0];
				m_arRequestsQueue.remove(0);

				ULONG utime = GetTickCount();
				lWaitingTime = (utime - uTime[0]) > 1500 ? 0 : 1500 - (utime - uTime[0]);

				if (!(pReq->m_bApiReq))
					lWaitingTime = 0;
			}

			if (m_bTerminated)
				break;

			if (lWaitingTime) {
				debugLogA("CVkProto::WorkerThread: need sleep %d msec", lWaitingTime);
				Sleep(lWaitingTime);
			}

			if (pReq->m_bApiReq) {
				uTime[0] = uTime[1];
				uTime[1] = uTime[2];
				uTime[2] = GetTickCount();
				// There can be maximum 3 requests to API methods per second from a client
				// see https://vk.com/dev/api_requests
			}

			if (!ExecuteRequest(pReq))
				return;
		}
	}

	CloseAPIConnection();

	debugLogA("CVkProto::WorkerThread: leaving m_bTerminated = %d", m_bTerminated ? 1 : 0);

	if (m_hWorkerThread) {
		CloseHandle(m_hWorkerThread);
		m_hWorkerThread = nullptr;
	}
}
