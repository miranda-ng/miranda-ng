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

void FacebookProto::OnLoggedIn()
{
	m_bOnline = true;
	m_mid = 0;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	// MqttPublish("/foreground_state", "{\"foreground\":\"true\", \"keepalive_timeout\":\"60\"}");

	MqttSubscribe("/inbox", "/mercury", "/messaging_events", "/orca_presence", "/orca_typing_notifications", "/pp", "/t_ms", "/t_p", "/t_rtc", "/webrtc", "/webrtc_response", 0);
	//	MqttUnsubscribe("/orca_message_notifications", 0);
}

void FacebookProto::OnLoggedOut()
{
	OnShutdown();

	m_bOnline = false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::ServerThread(void *)
{
	m_szAuthToken = getMStringA(DBKEY_TOKEN);

	if (m_szAuthToken.IsEmpty()) {
		auto *pReq = CreateRequest("authenticate", "auth.login");
		pReq->m_szUrl = FB_API_URL_AUTH;

		pReq << CHAR_PARAM("email", getMStringA(DBKEY_LOGIN));
		pReq << CHAR_PARAM("password", getMStringA(DBKEY_PASS));

		pReq->CalcSig();

		JsonReply reply(ExecuteRequest(pReq));

		if (reply.error()) {
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)m_iStatus, m_iDesiredStatus);

			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
			return;
		}

		m_szAuthToken = reply.data()["access_token"].as_mstring();
		setString(DBKEY_TOKEN, m_szAuthToken);

		m_uid = reply.data()["uid"].as_int();
		CMStringA m_szUid = reply.data()["uid"].as_mstring();
		setString(DBKEY_ID, m_szUid);
	}

	auto *pReq = CreateRequestGQL(FB_API_QUERY_CONTACTS);
	pReq << CHAR_PARAM("query_params", "{\"0\":[\"user\"],\"1\":\"" FB_API_CONTACTS_COUNT "\"}");
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (reply.error()) {
FAIL:
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)m_iStatus, m_iDesiredStatus);

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
		return;
	}

	// connect to MQTT server
	if (!MqttConnect())
		goto FAIL;

	// send initial packet
	MqttOpen();

	__int64 startTime = GetTickCount64();

	while (!Miranda_IsTerminated()) {
		NETLIBSELECT nls = {};
		nls.hReadConns[0] = m_mqttConn;
		nls.dwTimeout = 1000;
		int ret = Netlib_Select(&nls);
		if (ret == SOCKET_ERROR) {
			debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
			break;
		}

		__int64 currTime = GetTickCount64();
		if (currTime - startTime > 60000) {
			startTime = currTime;
			MqttPing();
		}

		// no data, continue waiting
		if (ret == 0)
			continue;

		MqttMessage msg;
		if (!MqttRead(msg)) {
			debugLogA("MqttRead() failed");
			break;
		}

		if (!MqttParse(msg)) {
			debugLogA("MqttParse() failed");
			break;
		}
	}

	debugLogA("exiting ServerThread");

	Netlib_CloseHandle(m_mqttConn);
	m_mqttConn = nullptr;

	int oldStatus = m_iStatus;
	m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}
