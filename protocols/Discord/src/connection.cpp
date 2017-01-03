/*
Copyright © 2016-17 Miranda NG team

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

void CDiscordProto::ExecuteRequest(AsyncHttpRequest *pReq)
{
	CMStringA str;

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

	pReq->flags |= NLHRF_PERSISTENT;
	pReq->nlc = m_hAPIConnection;

	debugLogA("Executing request #%d:\n%s", pReq->m_iReqNum, pReq->szUrl);
	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)pReq);
	if (reply != NULL) {
		if (pReq->m_pCallback != NULL)
			(this->*(pReq->m_pCallback))(reply, pReq);

		m_hAPIConnection = reply->nlc;

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
	}
	else {
		debugLogA("Request %d failed", pReq->m_iReqNum);

		if (IsStatusConnecting(m_iStatus))
			ConnectionFailed(LOGINERR_NONETWORK);
		else
			ShutdownSession();
		m_hAPIConnection = NULL;
	}
	delete pReq;
}

void CDiscordProto::OnLoggedIn()
{
	debugLogA("CDiscordProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/users/@me/guilds", &CDiscordProto::OnReceiveGuilds));
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/users/@me/channels", &CDiscordProto::OnReceiveChannels));
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/users/@me/relationships", &CDiscordProto::OnReceiveFriends));

	if (m_szGateway.IsEmpty())
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/gateway", &CDiscordProto::OnReceiveGateway));
	else
		ForkThread(&CDiscordProto::GatewayThread, NULL);
}

void CDiscordProto::OnLoggedOut()
{
	debugLogA("CDiscordProto::OnLoggedOut");
	m_bOnline = false;
	m_hWorkerThread = NULL;

	if (m_hAPIConnection)
		Netlib_CloseHandle(m_hAPIConnection);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
}

void CDiscordProto::ShutdownSession()
{
	debugLogA("CDiscordProto::ShutdownSession");
	m_bTerminated = true;
	if (m_hWorkerThread)
		SetEvent(m_evRequestsQueue);
	OnLoggedOut();
}

void CDiscordProto::ConnectionFailed(int iReason)
{
	debugLogA("CDiscordProto::ConnectionFailed -> reason %d", iReason);
	delSetting("AccessToken");

	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, iReason);
	ShutdownSession();
}

void CDiscordProto::ServerThread(void*)
{
	m_szAccessToken = getStringA("AccessToken");
	m_hAPIConnection = NULL;
	m_bTerminated = false;

	debugLogA("CDiscordProto::WorkerThread: %s", "entering");

	if (m_szAccessToken != NULL)
		// try to receive a response from server
		RetrieveUserInfo(NULL);
	else {
		if (mir_wstrlen(m_wszEmail) == NULL) {
			ConnectionFailed(LOGINERR_BADUSERID);
			return;
		}

		ptrW wszPassword(getWStringA(DB_KEY_PASSWORD));
		if (wszPassword == NULL) {
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

	m_hWorkerThread = NULL;
	if (m_hAPIConnection) {
		Netlib_CloseHandle(m_hAPIConnection);
		m_hAPIConnection = NULL;
	}

	debugLogA("CDiscordProto::WorkerThread: %s", "leaving");
}
