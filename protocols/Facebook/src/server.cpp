/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

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

void FacebookProto::OnLoggedOut()
{
	m_bOnline = false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::ServerThread(void *)
{
	m_szAuthToken = getMStringA(DBKEY_TOKEN);
	if (m_szAuthToken.IsEmpty()) {
		auto *pReq = new AsyncHttpRequest();
		pReq->requestType = REQUEST_GET;
		pReq->flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
		pReq->m_szUrl = "https://www.facebook.com/v3.3/dialog/oauth?client_id=478386432928815&redirect_uri=https://oauth.miranda-ng.org/facebook.php&state=qq";

		if (JsonReply(ExecuteRequest(pReq)).error()) {
FAIL:
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)m_iStatus, m_iDesiredStatus);

			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
			return;
		}
	}

	auto *pReq = CreateRequest("FetchContactsFullQuery", "get");
	pReq->m_szUrl = "https://graph.facebook.com/me/friends";
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (reply.error())
		goto FAIL;

	if (!MqttConnect())
		goto FAIL;

	MqttOpen();

	int bufSize = 2048;
	char *buf = (char*)mir_alloc(bufSize);

	while (!Miranda_IsTerminated()) {
		int ret = Netlib_Recv(m_mqttConn, buf, bufSize);
		if (ret == SOCKET_ERROR) {
			debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
			break;
		}
		if (ret == 0) {
			debugLogA("Connection closed gracefully");
			break;
		}
	}

	debugLogA("exiting ServerThread");
	int oldStatus = m_iStatus;
	m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}
