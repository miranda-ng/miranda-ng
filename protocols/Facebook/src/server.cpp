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

bool FacebookProto::RefreshContacts()
{
	auto *pReq = CreateRequestGQL(FB_API_QUERY_CONTACTS);
	pReq << CHAR_PARAM("query_params", "{\"0\":[\"user\"],\"1\":\"" FB_API_CONTACTS_COUNT "\"}");
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (reply.error())
		return false;

	for (auto &it : reply.data()["viewer"]["messenger_contacts"]["nodes"]) {
		auto &n = it["represented_profile"];
		CMStringW wszId(n["id"].as_mstring());
		__int64 id = _wtoi64(wszId);

		MCONTACT hContact;
		if (id != m_uid) {
			auto *pUser = FindUser(id);
			if (pUser == nullptr) {
				hContact = db_add_contact();
				Proto_AddToContact(hContact, m_szModuleName);
				setWString(hContact, DBKEY_ID, wszId);
				
				m_users.insert(new FacebookUser(id, hContact));
			}
			else hContact = pUser->hContact;
		}
		else hContact = 0;

		if (auto &nName = n["structured_name"]) {
			CMStringW wszName(nName["text"].as_mstring());
			setWString(hContact, DBKEY_NICK, wszName);
			for (auto &nn : nName["parts"]) {
				CMStringW wszPart(nn["part"].as_mstring());
				int offset = nn["offset"].as_int(), length = nn["length"].as_int();
				if (wszPart == L"first")
					setWString(hContact, DBKEY_FIRST_NAME, wszName.Mid(offset, length));
				else if (wszPart == L"last")
					setWString(hContact, DBKEY_LAST_NAME, wszName.Mid(offset, length));
			}
		}

		if (auto &nBirth = n["birthdate"]) {
			setDword(hContact, "BirthDay", nBirth["day"].as_int());
			setDword(hContact, "BirthMonth", nBirth["month"].as_int());
		}

		if (auto &nCity = n["current_city"])
			setDword(hContact, "City", nCity["name"].as_int());

		if (auto &nAva = n["smallPictureUrl"])
			setWString(hContact, "Avatar", nAva["uri"].as_mstring());
	}
	return true;
}

bool FacebookProto::RefreshToken()
{
	auto *pReq = CreateRequest("authenticate", "auth.login");
	pReq->m_szUrl = FB_API_URL_AUTH;
	pReq << CHAR_PARAM("email", getMStringA(DBKEY_LOGIN));
	pReq << CHAR_PARAM("password", getMStringA(DBKEY_PASS));
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (reply.error())
		return false;

	m_szAuthToken = reply.data()["access_token"].as_mstring();
	setString(DBKEY_TOKEN, m_szAuthToken);

	m_uid = reply.data()["uid"].as_int();
	CMStringA m_szUid = reply.data()["uid"].as_mstring();
	setString(DBKEY_ID, m_szUid);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::ServerThread(void *)
{
	m_szAuthToken = getMStringA(DBKEY_TOKEN);

	if (m_szAuthToken.IsEmpty()) {
		if (!RefreshToken()) {
FAIL:
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)m_iStatus, m_iDesiredStatus);

			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
			return;
		}
	}

	if (!RefreshContacts())
		goto FAIL;

	// connect to MQTT server
	NETLIBOPENCONNECTION nloc = {};
	nloc.szHost = "mqtt.facebook.com";
	nloc.wPort = 443;
	nloc.flags = NLOCF_SSL | NLOCF_V2;
	m_mqttConn = Netlib_OpenConnection(m_hNetlibUser, &nloc);
	if (m_mqttConn == nullptr) {
		debugLogA("connection failed, exiting");
		goto FAIL;
	}

	// send initial packet
	MqttLogin();

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

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::OnPublish(const char *topic, const uint8_t *p, size_t cbLen)
{
	FbThriftReader rdr;

	// that might be a zipped buffer
	if (cbLen >= 2) {
		if ((((p[0] << 8) | p[1]) % 31) == 0 && (p[0] & 0x0F) == 8) { // zip header ok
			size_t dataSize;
			void *pData = doUnzip(cbLen, p, dataSize);
			if (pData != nullptr) {
				rdr.reset(dataSize, pData);
				mir_free(pData);
			}
		}
	}

	if (rdr.size() == 0)
		rdr.reset(cbLen, (void*)p);

	if (!strcmp(topic, "/t_p"))
		OnPublishP(rdr);

}

void FacebookProto::OnPublishP(FbThriftReader &rdr)
{
	char *str;
	assert(rdr.readStr(str));
	mir_free(str);

	bool bVal;
	uint8_t fieldType;
	uint16_t fieldId;
	assert(rdr.readField(fieldType, fieldId));
	assert(fieldType == FB_THRIFT_TYPE_BOOL);
	assert(fieldId == 1);
	assert(rdr.readBool(bVal));

	assert(rdr.readField(fieldType, fieldId));
	assert(fieldType == FB_THRIFT_TYPE_LIST);
	assert(fieldId == 1);

	uint32_t size;
	assert(rdr.readList(fieldType, size));
	assert(fieldType == FB_THRIFT_TYPE_STRUCT);

	for (uint32_t i = 0; i < size; i++) {
		uint64_t userId, timestamp, voipBits;
		assert(rdr.readField(fieldType, fieldId));
		assert(fieldType == FB_THRIFT_TYPE_I64);
		assert(fieldId == 1);
		assert(rdr.readInt64(userId));

		uint32_t u32;
		assert(rdr.readField(fieldType, fieldId));
		assert(fieldType == FB_THRIFT_TYPE_I32);
		assert(fieldId == 1);
		assert(rdr.readInt32(u32));

		auto *pUser = FindUser(userId);
		if (pUser == nullptr)
			debugLogA("Skipping presence from unknown user %lld", userId);
		else {
			debugLogA("Presence from user %lld => %d", userId, u32);
			setWord(pUser->hContact, "Status", (u32 != 0) ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
		}

		assert(rdr.readField(fieldType, fieldId));
		assert(fieldType == FB_THRIFT_TYPE_I64);
		assert(fieldId == 1);
		assert(rdr.readInt64(timestamp));

		while (!rdr.isStop()) {
			assert(rdr.readField(fieldType, fieldId));
			assert(fieldType == FB_THRIFT_TYPE_I64 || fieldType == FB_THRIFT_TYPE_I16 || fieldType == FB_THRIFT_TYPE_I32);
			assert(rdr.readIntV(voipBits));
		}

		assert(rdr.readByte(fieldType));
		assert(fieldType == FB_THRIFT_TYPE_STOP);
	}

	assert(rdr.readByte(fieldType));
	assert(fieldType == FB_THRIFT_TYPE_STOP);
}
