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

void CDiscordProto::ExecuteRequest(AsyncHttpRequest *pReq)
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
		pReq->AddHeader("Cookie", m_szCookie);
	}

	bool bRetryable = pReq->nlc != nullptr;
	debugLogA("Executing request #%d:\n%s", pReq->m_iReqNum, pReq->szUrl);

LBL_Retry:
	NLHR_PTR reply(Netlib_HttpTransaction(m_hNetlibUser, pReq));
	if (reply == nullptr) {
		debugLogA("Request %d failed", pReq->m_iReqNum);

		if (pReq->m_bMainSite) {
			if (IsStatusConnecting(m_iStatus))
				ConnectionFailed(LOGINERR_NONETWORK);
			m_hAPIConnection = nullptr;
		}

		if (bRetryable) {
			debugLogA("Attempt to retry request #%d", pReq->m_iReqNum);
			pReq->nlc = nullptr;
			bRetryable = false;
			goto LBL_Retry;
		}
	}
	else {
		if (pReq->m_pFunc != nullptr)
			(this->*(pReq->m_pFunc))(reply, pReq);

		if (pReq->m_bMainSite)
			m_hAPIConnection = reply->nlc;
	}
	delete pReq;
}

void CDiscordProto::OnLoggedIn()
{
	debugLogA("CDiscordProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);
}

void CDiscordProto::OnLoggedOut()
{
	debugLogA("CDiscordProto::OnLoggedOut");
	m_bOnline = false;
	m_bTerminated = true;
	m_iGatewaySeq = 0;
	m_szTempToken = nullptr;
	m_szCookie.Empty();
	m_szWSCookie.Empty();

	m_impl.m_heartBeat.StopSafe();

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void CDiscordProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("CDiscordProto::ShutdownSession");

	// shutdown all resources
	if (m_hWorkerThread)
		SetEvent(m_evRequestsQueue);
	if (m_hGatewayConnection)
		Netlib_Shutdown(m_hGatewayConnection);
	if (m_hAPIConnection)
		Netlib_Shutdown(m_hAPIConnection);

	OnLoggedOut();
}

void CDiscordProto::ConnectionFailed(int iReason)
{
	debugLogA("CDiscordProto::ConnectionFailed -> reason %d", iReason);
	delSetting("AccessToken");

	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, iReason);
	ShutdownSession();
}
