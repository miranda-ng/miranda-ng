/*
Copyright © 2012-23 Miranda NG team
Copyright © 2009 Jim Porter

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

AsyncHttpRequest::AsyncHttpRequest(int type, const char *szUrl, MTHttpRequestHandler pHandler)
{
	m_pFunc = pHandler;
	requestType = type;
	m_szUrl = szUrl;
}

void CTwitterProto::Execute(AsyncHttpRequest *pReq)
{
	if (pReq->m_szUrl[0] == '/')
		pReq->m_szUrl.Insert(0, "https://api.twitter.com");

	bool bIsJson = false;
	if (!pReq->m_szParam.IsEmpty()) {
		if (pReq->requestType == REQUEST_POST) {
			if (pReq->m_szParam[0] == '{') {
				bIsJson = true;
				pReq->AddHeader("Content-Type", "application/json");
			}
			else pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
			pReq->AddHeader("Cache-Control", "no-cache");
		}
		else if (pReq->requestType == REQUEST_PATCH)
			pReq->requestType = REQUEST_POST;
	}

	// CMStringA auth(FORMAT, "%s:%s", OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET);
	// auth.Format("Basic: %s", ptrA(mir_base64_encode(auth, auth.GetLength())).get());
	//	if (pReq->requestType == REQUEST_GET)
	//	auth = OAuthWebRequestSubmit(pReq->m_szUrl, "GET", "");
	//else
	//	auth = OAuthWebRequestSubmit(pReq->m_szUrl, "POST", (bIsJson) ? "" : pReq->pData);
	// pReq->AddHeader("Authorization", auth);

	pReq->flags = NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	pReq->nlc = m_hConnHttp;

	NLHR_PTR resp(Netlib_HttpTransaction(m_hNetlibUser, pReq));
	if (resp) {
		m_hConnHttp = resp->nlc;

		if (pReq->m_pFunc != nullptr)
			(this->*(pReq->m_pFunc))(resp, pReq);
	}
	else m_hConnHttp = nullptr;

	delete pReq;
}

void CTwitterProto::Push(AsyncHttpRequest *pReq)
{
	{
		mir_cslock lck(m_csHttpQueue);
		m_arHttpQueue.insert(pReq);
	}

	SetEvent(m_evRequestsQueue);
}

void __cdecl CTwitterProto::ServerThread(void *)
{
	m_bTerminated = false;
	m_hWorkerThreadId = GetCurrentThreadId();
	debugLogA("CTwitterProto::ServerThread: %s", "entering");

	while (true) {
		WaitForSingleObject(m_evRequestsQueue, 1000);
		if (m_bTerminated)
			break;

		while (true) {
			bool bNeedSleep = false;
			AsyncHttpRequest *pReq;
			{
				mir_cslock lck(m_csHttpQueue);
				if (m_arHttpQueue.getCount() == 0)
					break;

				pReq = m_arHttpQueue[0];
				m_arHttpQueue.remove(0);
				bNeedSleep = (m_arHttpQueue.getCount() > 1);
			}
			
			if (m_bTerminated)
				break;

			Execute(pReq);
			if (bNeedSleep)
				Sleep(200);
		}
	}

	m_hWorkerThreadId = 0;
	debugLogA("CTwitterProto::ServerThread: %s", "leaving");
}
