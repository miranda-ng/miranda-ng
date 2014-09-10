/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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
	CMStringA str;

LBL_Restart:
	if (pReq->requestType == REQUEST_GET) {
		str.Format("%s?%s", pReq->m_szUrl, pReq->m_szParam);
		pReq->szUrl = str.GetBuffer();
	}
	else {
		pReq->szUrl = pReq->m_szUrl.GetBuffer();
		if (!pReq->m_szParam.IsEmpty()) {
			pReq->pData = mir_strdup(pReq->m_szParam.GetBuffer());
			pReq->dataLength = pReq->m_szParam.GetLength();
		}
	}

	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)pReq);
	if (reply != NULL) {
		if (pReq->m_pFunc != NULL)
			(this->*(pReq->m_pFunc))(reply, pReq);

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
		if (pReq->bNeedsRestart)
			goto LBL_Restart;
	}
	else if (pReq->bIsMainConn) {
		if (m_iStatus >= ID_STATUS_CONNECTING && m_iStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
			ConnectionFailed(LOGINERR_NONETWORK);
		else
			ShutdownSession();
	}
	delete pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest* CVkProto::Push(AsyncHttpRequest *pReq, int iTimeout)
{
	pReq->timeout = iTimeout;
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
	if (m_szAccessToken != NULL)
		// try to receive a response from server
		RetrieveMyInfo();
	else {
		// Initialize new OAuth session
		extern char szBlankUrl[];
		//need rework
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/oauth/authorize", false, &CVkProto::OnOAuthAuthorize)
			<< INT_PARAM("client_id", VK_APP_ID) << CHAR_PARAM("scope", "friends,photos,audio,video,wall,messages,offline")
			<< CHAR_PARAM("redirect_uri", szBlankUrl) << CHAR_PARAM("display", "wap") << CHAR_PARAM("response_type", "token"));
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

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const TCHAR_PARAM &param)
{
	ptrA szValue(mir_utf8encodeT(param.tszValue));
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, ptrA(mir_urlEncode(szValue)));
	return pReq;
}
