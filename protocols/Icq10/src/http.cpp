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
	memset(&m_ConnPool, 0, sizeof(m_ConnPool));
	m_bTerminated = false;

	if (m_dwUin == 0 || mir_wstrlen(m_szPassword) == 0) {
		debugLogA("Thread ended, UIN/password are not configured");
		ConnectionFailed(LOGINERR_BADUSERID);
		return;
	}

	debugLogA("CIcqProto::WorkerThread: %s", "entering");
	{
		char mirVer[100];
		Miranda_GetVersionText(mirVer, _countof(mirVer));

		m_szAToken = getMStringA("AToken");
		m_szSessionKey = getMStringA("SessionKey");
		if (m_szAToken.IsEmpty() || m_szSessionKey.IsEmpty()) {
			auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "https://api.login.icq.net/auth/clientLogin", &CIcqProto::OnCheckPassword);
			pReq << CHAR_PARAM("clientName", "Miranda NG") << CHAR_PARAM("clientVersion", mirVer) << CHAR_PARAM("devId", ICQ_APP_ID)
				<< CHAR_PARAM("f", "json") << CHAR_PARAM("tokenType", "longTerm") << INT_PARAM("s", m_dwUin) << WCHAR_PARAM("pwd", m_szPassword);
			pReq->flags |= NLHRF_NODUMPSEND;
			Push(pReq);
		}
		else StartSession();
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
	for (auto &it : m_ConnPool)
		if (it) {
			Netlib_CloseHandle(it);
			it = nullptr;
		}

	debugLogA("CIcqProto::WorkerThread: %s", "leaving");
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest::AsyncHttpRequest(IcqConnection conn, int iType, const char *szUrl, MTHttpRequestHandler pFunc) :
	m_conn(conn)
{
	flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_DUMPASTEXT;
	requestType = iType;
	m_szUrl = szUrl;
	m_pFunc = pFunc;

	GUID packetId;
	UuidCreate(&packetId);

	RPC_CSTR szId;
	UuidToStringA(&packetId, &szId);
	strncpy_s(m_reqId, (char*)szId, _TRUNCATE);
	RpcStringFreeA(&szId);

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

	if (pReq->m_conn != CONN_NONE) {
		pReq->flags |= NLHRF_PERSISTENT;
		pReq->nlc = m_ConnPool[pReq->m_conn];
	}

	debugLogA("Executing request %s:\n%s", pReq->m_reqId, pReq->szUrl);
	
	NETLIBHTTPREQUEST *reply = Netlib_HttpTransaction(m_hNetlibUser, pReq);
	if (reply != nullptr) {
		if (pReq->m_pFunc != nullptr)
			(this->*(pReq->m_pFunc))(reply, pReq);

		if (pReq->m_conn != CONN_NONE)
			m_ConnPool[pReq->m_conn] = pReq->nlc;

		Netlib_FreeHttpRequest(reply);
	}
	else {
		debugLogA("Request %s failed", pReq->m_reqId);

		if (pReq->m_conn != CONN_NONE) {
			if (IsStatusConnecting(m_iStatus))
				ConnectionFailed(LOGINERR_NONETWORK);
			m_ConnPool[pReq->m_conn] = nullptr;
		}
	}

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

/////////////////////////////////////////////////////////////////////////////////////////

JsonReply::JsonReply(NETLIBHTTPREQUEST *pReply)
{
	if (pReply == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = pReply->resultCode;
	if (m_errorCode != 200)
		return;

	m_root = json_parse(pReply->pData);
	if (m_root == nullptr) {
		m_errorCode = 500;
		return;
	}

	JSONNode &response = (*m_root)["response"];
	m_errorCode = response["statusCode"].as_int();
	m_data = &response["data"];
}

JsonReply::~JsonReply()
{
	json_delete(m_root);
}
