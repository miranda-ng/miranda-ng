// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-22 Miranda NG team
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

void CIcqProto::DropQueue()
{
	mir_cslock lck(m_csHttpQueue);

	while (m_arHttpQueue.getCount()) {
		auto *pReq = m_arHttpQueue[0];
		m_arHttpQueue.remove(0);
		delete pReq;
	}
}

bool CIcqProto::IsQueueEmpty()
{
	mir_cslock lck(m_csHttpQueue);
	return m_arHttpQueue.getCount() == 0;
}

void __cdecl CIcqProto::ServerThread(void*)
{
	memset(&m_ConnPool, 0, sizeof(m_ConnPool));
	m_bTerminated = false;

	debugLogA("CIcqProto::WorkerThread: %s", "entering");

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
			
			ExecuteRequest(pReq);
			if (bNeedSleep)
				Sleep(200);
		}

		int ts = time(0);
		for (auto &it : m_ConnPool) {
			int idx = int(&it - m_ConnPool);
			if (idx == CONN_FETCH)
				continue;

			if (it.s && it.lastTs + it.timeout < ts) {
				debugLogA("Socket #1 (%p) expired", idx, it.s);
				Netlib_CloseHandle(it.s);
				it.s = nullptr;
				it.lastTs = 0;
			}
		}
	}

	m_hWorkerThread = nullptr;
	for (auto &it : m_ConnPool) {
		if (it.s)
			Netlib_CloseHandle(it.s);
		it.s = nullptr;
		it.lastTs = it.timeout = 0;
	}

	debugLogA("CIcqProto::WorkerThread: %s", "leaving");
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncRapiRequest::AsyncRapiRequest(CIcqProto *ppro, const char *pszMethod, MTHttpRequestHandler pFunc) :
	AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, pFunc)
{
	params.set_name("params");

	if (ppro->getByte(DB_KEY_PHONEREG)) {
		m_szUrl.AppendChar('/');
		m_szUrl.Append(pszMethod);
		
		AddHeader("Content-Type", "application/json");
		request << CHAR_PARAM("aimsid", ppro->m_aimsid);
	}
	else request << CHAR_PARAM("method", pszMethod);
}

void AsyncRapiRequest::OnPush()
{
	request << CHAR_PARAM("reqId", m_reqId) << params;

	m_szParam = ptrW(json_write(&request));
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest::AsyncHttpRequest(IcqConnection conn, int iType, const char *szUrl, MTHttpRequestHandler pFunc) :
	m_conn(conn)
{
	flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_DUMPASTEXT;
	requestType = iType;
	m_szUrl = szUrl;
	m_pFunc = pFunc;
	timeout = 10000;

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

void AsyncHttpRequest::ReplaceJsonParam(const JSONNode &n)
{
	auto *szNodeName = n.name();

	JSONNode root = JSONNode::parse(m_szParam);
	JSONNode& old = root.at(szNodeName);
	if (old)
		old = n;
	else
		root.push_back(n);
	m_szParam = root.write().c_str();

	replaceStr(pData, nullptr);
	dataLength = 0;
}

bool CIcqProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	CMStringA str;

	pReq->szUrl = pReq->m_szUrl.GetBuffer();
	if (!pReq->m_szParam.IsEmpty()) {
		if (pReq->requestType == REQUEST_GET) {
			str.Format("%s?%s", pReq->m_szUrl.c_str(), pReq->m_szParam.c_str());
			pReq->szUrl = str.GetBuffer();
		}
		else {
			pReq->dataLength = pReq->m_szParam.GetLength();
			pReq->pData = mir_strdup(pReq->m_szParam);
		}
	}

	// replace credentials inside JSON body for pure RAPI requests
	if (pReq->m_conn == CONN_RAPI && !mir_strcmp(pReq->szUrl, ICQ_ROBUST_SERVER) && !getByte(DB_KEY_PHONEREG)) {
		CMStringA szAgent(FORMAT, "%S Mail.ru Windows ICQ (version 10.0.1999)", (wchar_t*)m_szOwnId);
		pReq->AddHeader("User-Agent", szAgent);
		pReq->AddHeader("Content-Type", "application/json");

		if (m_szRToken.IsEmpty()) {
			if (!RefreshRobustToken(pReq)) {
				delete pReq;
				return false;
			}
		}

		if (m_iRClientId)
			pReq->ReplaceJsonParam(JSONNode("clientId", m_iRClientId));
		pReq->ReplaceJsonParam(JSONNode("authToken", m_szRToken));
		pReq->dataLength = pReq->m_szParam.GetLength();
		pReq->pData = mir_strdup(pReq->m_szParam);
	}

	debugLogA("Executing request %s:\n%s", pReq->m_reqId, pReq->szUrl);

	if (pReq->m_conn != CONN_NONE) {
		pReq->flags |= NLHRF_PERSISTENT;
		pReq->nlc = m_ConnPool[pReq->m_conn].s;
		m_ConnPool[pReq->m_conn].lastTs = time(0);
	}

	bool bRet;
	NLHR_PTR reply(Netlib_HttpTransaction(m_hNetlibUser, pReq));
	if (reply != nullptr) {
		if (pReq->m_conn != CONN_NONE) {
			auto &conn = m_ConnPool[pReq->m_conn];
			conn.s = reply->nlc;
			conn.timeout = 0;
			if (auto *pszHdr = Netlib_GetHeader(reply, "Keep-Alive")) {
				int timeout;
				if (1 == sscanf(pszHdr, "timeout=%d", &timeout))
					conn.timeout = timeout;
			}
		}

		if (pReq->m_conn == CONN_RAPI && reply->pData && strstr(reply->pData, "\"code\": 40201")) {
			RobustReply r(reply);
			if (r.error() == 40201) { // robust token expired
				m_szRToken.Empty();
				
				// if token refresh succeeded, replace it in the query and push request back
				if (!RefreshRobustToken(pReq)) {
					delete pReq;
					return false;
				}

				Push(pReq);
				return true;
			}
		}

		if (pReq->m_pFunc != nullptr)
			(this->*(pReq->m_pFunc))(reply, pReq);

		bRet = true;
	}
	else {
		debugLogA("Request %s failed", pReq->m_reqId);

		if (IsStatusConnecting(m_iStatus))
			ConnectionFailed(LOGINERR_NONETWORK);

		if (pReq->m_conn != CONN_NONE)
			m_ConnPool[pReq->m_conn].s = nullptr;

		bRet = false;
	}

	delete pReq;
	return bRet;
}

void CIcqProto::Push(MHttpRequest *p)
{
	AsyncHttpRequest *pReq = (AsyncHttpRequest*)p;

	pReq->OnPush();
	{
		mir_cslock lck(m_csHttpQueue);
		m_arHttpQueue.insert(pReq);
	}
	
	SetEvent(m_evRequestsQueue);
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest* operator<<(AsyncHttpRequest *pReq, const AIMSID &param)
{
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", param.m_ppro->m_aimsid) << CHAR_PARAM("r", pReq->m_reqId);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	return pReq;
}

/////////////////////////////////////////////////////////////////////////////////////////

MHttpRequest* operator<<(MHttpRequest *pReq, const GROUP_PARAM &param)
{
	if (param.wszValue) {
		CMStringW tmp(param.wszValue);
		tmp.Replace(L"\\", L">");
		tmp.Replace(L"/", L">");
		pReq << WCHAR_PARAM(param.szName, tmp);
	}
	return pReq;
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
	m_requestId = response["requestId"].as_mstring();
	m_detailCode = response["statusDetailCode"].as_int();
	m_data = &response["data"];
}

JsonReply::~JsonReply()
{
	json_delete(m_root);
}

/////////////////////////////////////////////////////////////////////////////////////////

FileReply::FileReply(NETLIBHTTPREQUEST *pReply)
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

	m_errorCode = (*m_root)["status"].as_int();
	m_data = &(*m_root)["data"];
}

FileReply::~FileReply()
{
	json_delete(m_root);
}

/////////////////////////////////////////////////////////////////////////////////////////

RobustReply::RobustReply(NETLIBHTTPREQUEST *pReply)
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

	m_errorCode = (*m_root)["status"]["code"].as_int();
	m_result = &(*m_root)["result"];
	m_results = &(*m_root)["results"];
}

RobustReply::~RobustReply()
{
	json_delete(m_root);
}
