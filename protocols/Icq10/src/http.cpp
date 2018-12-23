// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

#pragma comment(lib, "Rpcrt4.lib")

void __cdecl CIcqProto::ServerThread(void*)
{
	m_hAPIConnection = nullptr;
	m_bTerminated = false;

	int uin = getDword("UIN");
	ptrA szPassword(getStringA("Password"));
	if (uin == 0 || szPassword == nullptr) {
		debugLogA("Thread ended, UIN/password are not configured");
		ConnectionFailed(LOGINERR_BADUSERID);
		return;
	}

	debugLogA("CIcqProto::WorkerThread: %s", "entering");
	{
		char mirVer[100];
		Miranda_GetVersionText(mirVer, _countof(mirVer));

		auto *pReq = new AsyncHttpRequest(REQUEST_POST, "https://api.login.icq.net/auth/clientLogin", &CIcqProto::OnCheckPassword);
		pReq << CHAR_PARAM("clientName", "Miranda NG") << CHAR_PARAM("clientVersion", mirVer) << CHAR_PARAM("devId", "ic1nmMjqg7Yu-0hL")
			<< CHAR_PARAM("f", "json") << CHAR_PARAM("tokenType", "longTerm") << INT_PARAM("s", uin) << CHAR_PARAM("pwd", szPassword);
		pReq->flags |= NLHRF_NODUMPSEND;
		Push(pReq);
	}

	while (true) {
		WaitForSingleObject(m_evRequestsQueue, 1000);
		if (m_bTerminated)
			break;

		AsyncHttpRequest *pReq;
		bool need_sleep = false;
		while (true) {
			{
				mir_cslock lck(m_csHttpQueue);
				if (m_arHttpQueue.getCount() == 0)
					break;

				pReq = m_arHttpQueue[0];
				m_arHttpQueue.remove(0);
				need_sleep = (m_arHttpQueue.getCount() > 1);
			}
			if (m_bTerminated)
				break;
			ExecuteRequest(pReq);
			if (need_sleep) {
				Sleep(330);
				debugLogA("CIcqProto::WorkerThread: %s", "need to sleep");
			}
		}
	}

	m_hWorkerThread = nullptr;
	if (m_hAPIConnection) {
		Netlib_CloseHandle(m_hAPIConnection);
		m_hAPIConnection = nullptr;
	}

	debugLogA("CIcqProto::WorkerThread: %s", "leaving");
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest::AsyncHttpRequest(int iType, const char *szUrl, MTHttpRequestHandler pFunc)
{
	flags = NLHRF_HTTP11 | NLHRF_SSL;
	requestType = iType;
	m_szUrl = szUrl;
	m_pFunc = pFunc;
	UuidCreate(&m_reqId);

	if (iType == REQUEST_POST) {
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
		
		dataLength = m_szParam.GetLength();
		pData = m_szParam.Detach();
	}
}

void CIcqProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	CMStringA str;

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

	if (pReq->m_bMainSite) {
		pReq->flags |= NLHRF_PERSISTENT;
		pReq->nlc = m_hAPIConnection;
	}

	RPC_CSTR szId;
	UuidToStringA(&pReq->m_reqId, &szId);
	debugLogA("Executing request %s:\n%s", (char*)szId, pReq->szUrl);
	
	NETLIBHTTPREQUEST *reply = Netlib_HttpTransaction(m_hNetlibUser, pReq);
	if (reply != nullptr) {
		if (pReq->m_pFunc != nullptr)
			(this->*(pReq->m_pFunc))(reply, pReq);

		if (pReq->m_bMainSite)
			m_hAPIConnection = reply->nlc;

		Netlib_FreeHttpRequest(reply);
	}
	else {
		debugLogA("Request %s failed", (char*)szId);

		if (pReq->m_bMainSite) {
			if (IsStatusConnecting(m_iStatus))
				ConnectionFailed(LOGINERR_NONETWORK);
			m_hAPIConnection = nullptr;
		}
	}

	RpcStringFreeA(&szId);
	delete pReq;
}

void CIcqProto::Push(MHttpRequest *p)
{
	AsyncHttpRequest *pReq = (AsyncHttpRequest*)p;

	pReq->timeout = 10000;
	{
		mir_cslock lck(m_csHttpQueue);
		m_arHttpQueue.insert(pReq);
	}
	
	SetEvent(m_evRequestsQueue);
}
