/*
Copyright © 2016-22 Miranda NG team

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

void CDiscordProto::Push(AsyncHttpRequest *pReq, int iTimeout)
{
	pReq->timeout = iTimeout;
	{
		mir_cslock lck(m_csHttpQueue);
		m_arHttpQueue.insert(pReq);
	}
	SetEvent(m_evRequestsQueue);
}

void CDiscordProto::SaveToken(const JSONNode &data)
{
	CMStringA szToken = data["token"].as_mstring();
	if (!szToken.IsEmpty())
		m_szTempToken = szToken.Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////

static LONG g_reqNum = 0;

AsyncHttpRequest::AsyncHttpRequest(CDiscordProto *ppro, int iRequestType, LPCSTR _url, MTHttpRequestHandler pFunc, JSONNode *pRoot)
{
	if (*_url == '/') {	// relative url leads to a site
		m_szUrl = "https://discord.com/api/v8";
		m_szUrl += _url;
		m_bMainSite = true;
	}
	else {
		m_szUrl = _url;
		m_bMainSite = false;
	}

	flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_SSL;
	if (ppro->m_szAccessToken != nullptr) {
		AddHeader("Authorization", ppro->m_szAccessToken);
		flags |= NLHRF_DUMPASTEXT | NLHRF_NODUMPHEADERS;
	}
	else flags |= NLHRF_NODUMPSEND;

	if (pRoot != nullptr) {
		ptrW text(json_write(pRoot));
		pData = mir_utf8encodeW(text);
		dataLength = (int)mir_strlen(pData);

		AddHeader("Content-Type", "application/json");
	}

	m_pFunc = pFunc;
	requestType = iRequestType;
	m_iErrorCode = 0;
	m_iReqNum = ::InterlockedIncrement(&g_reqNum);
}

JsonReply::JsonReply(NETLIBHTTPREQUEST *pReply)
{
	if (pReply == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = pReply->resultCode;

	m_root = json_parse(pReply->pData);
	if (m_root == nullptr)
		m_errorCode = 500;
}

JsonReply::~JsonReply()
{
	json_delete(m_root);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ServerThread(void*)
{
	m_szAccessToken = getStringA("AccessToken");
	m_hAPIConnection = nullptr;
	m_bTerminated = false;

	debugLogA("CDiscordProto::WorkerThread: %s", "entering");

	if (m_szAccessToken != nullptr)
		RetrieveMyInfo(); // try to receive a response from server
	else {
		if (mir_wstrlen(m_wszEmail) == 0) {
			ConnectionFailed(LOGINERR_BADUSERID);
			return;
		}

		ptrW wszPassword(getWStringA(DB_KEY_PASSWORD));
		if (wszPassword == nullptr) {
			ConnectionFailed(LOGINERR_WRONGPASSWORD);
			return;
		}

		JSONNode root; root << WCHAR_PARAM("email", m_wszEmail) << WCHAR_PARAM("password", wszPassword);
		Push(new AsyncHttpRequest(this, REQUEST_POST, "/auth/login", &CDiscordProto::OnReceiveToken, &root));
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
				debugLogA("CDiscordProto::WorkerThread: %s", "need to sleep");
			}
		}
	}

	m_hWorkerThread = nullptr;
	if (m_hAPIConnection) {
		Netlib_CloseHandle(m_hAPIConnection);
		m_hAPIConnection = nullptr;
	}

	debugLogA("CDiscordProto::WorkerThread: %s", "leaving");
}
