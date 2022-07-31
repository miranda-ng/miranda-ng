/*

Facebook plugin for Miranda NG
Copyright © 2019-22 Miranda NG team

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

void FacebookProto::ConnectionFailed(int iErrorCode)
{
	if (iErrorCode) {
		POPUPDATAW popup;
		popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR), true);
		wcscpy_s(popup.lpwzContactName, m_tszUserName);
		mir_snwprintf(popup.lpwzText, TranslateT("Connection failed with error code %d"), iErrorCode);
		PUAddPopupW(&popup);
	}

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)m_iStatus, m_iDesiredStatus);

	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);

	OnShutdown();
}

bool FacebookProto::ExtractOwnMessage(__int64 msgId, COwnMessage &res)
{
	mir_cslock lck(m_csOwnMessages);
	for (auto &it : arOwnMessages)
		if (it->msgId == msgId) {
			res = *it;
			arOwnMessages.removeItem(&it);
			return true;
		}

	return false;
}

void FacebookProto::OnLoggedIn()
{
	m_mid = 0;

	JSONNode root; root << BOOL_PARAM("foreground", true) << INT_PARAM("keepalive_timeout", 60);
	MqttPublish("/foreground_state", root);

	MqttSubscribe("/inbox", "/mercury", "/messaging_events", "/orca_presence", "/orca_typing_notifications", "/pp", "/t_ms", "/t_p", "/t_rtc", "/webrtc", "/webrtc_response", 0);
	MqttUnsubscribe("/orca_message_notifications", 0);

	// if sequence is not initialized, request SID from the server
	if (m_sid == 0) {
		if (!RefreshSid()) {
			ConnectionFailed();
			return;
		}
	}

	// point of no return;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;
	m_bOnline = true;
	m_impl.m_heartBeat.Start(60000);

	// connect message queue
	MqttQueueConnect();

	// request message threads if needed
	if (m_bUseGroupchats)
		RefreshThreads();
}

void FacebookProto::OnLoggedOut()
{
	m_impl.m_heartBeat.Stop();
	m_bOnline = false;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

FacebookUser* FacebookProto::AddContact(const CMStringW &wszId, bool bTemp)
{
	MCONTACT hContact = db_add_contact();
	setWString(hContact, DBKEY_ID, wszId);
	Proto_AddToContact(hContact, m_szModuleName);
	Clist_SetGroup(hContact, m_wszDefaultGroup);
	if (bTemp)
		Contact::RemoveFromList(hContact);

	return FindUser(_wtoi64(wszId));
}

FacebookUser* FacebookProto::FindUser(__int64 id)
{
	mir_cslock lck(m_csUsers);
	return m_users.find((FacebookUser *)&id);
}

FacebookUser* FacebookProto::UserFromJson(const JSONNode &root, CMStringW &wszUserId, bool &bIsChat)
{
	bIsChat = false;
	wszUserId = root["threadKey"]["otherUserFbId"].as_mstring();
	if (wszUserId.IsEmpty()) {
		// if only thread id is present, it must be a group chat
		wszUserId = root["threadKey"]["threadFbId"].as_mstring();
		bIsChat = true;
	}

	auto *pUser = FindUser(_wtoi64(wszUserId));
	if (pUser == nullptr) {
		debugLogA("Message from unknown contact %s, ignored", wszUserId.c_str());
		return nullptr;
	}

	if (pUser->bIsChat != bIsChat) {
		debugLogA("Wrong chat user: %d vs %d for user %lld, ignored", pUser->bIsChat, bIsChat, pUser->id);
		return nullptr;
	}

	return pUser;
}

int FacebookProto::RefreshContacts()
{
	CMStringA szCursor;
	bool bNeedUpdate = false;

	while (true) {
		JSONNode root; root << CHAR_PARAM("0", "user");

		AsyncHttpRequest *pReq;
		if (szCursor.IsEmpty()) {
			pReq = CreateRequestGQL(FB_API_QUERY_CONTACTS);
			root << INT_PARAM("1", FB_API_CONTACTS_COUNT);
		}
		else {
			pReq = CreateRequestGQL(FB_API_QUERY_CONTACTS_AFTER);
			root << CHAR_PARAM("1", szCursor) << INT_PARAM("2", FB_API_CONTACTS_COUNT);
		}
		pReq << CHAR_PARAM("query_params", root.write().c_str());
		pReq->flags |= NLHRF_NODUMPSEND;
		pReq->CalcSig();

		JsonReply reply(ExecuteRequest(pReq));
		if (int iErrorCode = reply.error())
			return iErrorCode;  // unknown error

		bool bLoadAll = m_bLoadAll;
		auto &data = reply.data()["viewer"]["messenger_contacts"];

		for (auto &it : data["nodes"]) {
			auto &n = it["represented_profile"];
			CMStringW wszId(n["id"].as_mstring());
			__int64 id = _wtoi64(wszId);

			MCONTACT hContact;
			if (id != m_uid) {
				bool bIsFriend = bLoadAll || n["friendship_status"].as_mstring() == L"ARE_FRIENDS";

				auto *pUser = FindUser(id);
				if (pUser == nullptr) {
					if (!bIsFriend)
						continue;
					pUser = AddContact(wszId, false);
				}
				else if (!bIsFriend)
					Contact::RemoveFromList(pUser->hContact); // adios!

				hContact = pUser->hContact;
			}
			else hContact = 0;

			if (auto &nName = it["structured_name"]) {
				CMStringW wszName(nName["text"].as_mstring());
				setWString(hContact, DBKEY_NICK, wszName);
				for (auto &nn : nName["parts"]) {
					CMStringW wszPart(nn["part"].as_mstring());
					int offset = nn["offset"].as_int(), length = nn["length"].as_int();
					if (wszPart == L"first")
						setWString(hContact, "FirstName", wszName.Mid(offset, length));
					else if (wszPart == L"last")
						setWString(hContact, "LastName", wszName.Mid(offset, length));
				}
			}

			if (auto &nBirth = n["birthdate"]) {
				setDword(hContact, "BirthDay", nBirth["day"].as_int());
				setDword(hContact, "BirthMonth", nBirth["month"].as_int());
			}

			if (auto &nCity = n["current_city"])
				setWString(hContact, "City", nCity["name"].as_mstring());

			if (auto &nAva = it[(m_bUseBigAvatars) ? "hugePictureUrl" : "bigPictureUrl"]) {
				CMStringW wszOldUrl(getMStringW(hContact, DBKEY_AVATAR)), wszNewUrl(nAva["uri"].as_mstring());
				if (wszOldUrl != wszNewUrl) {
					bNeedUpdate = true;
					setByte(hContact, "UpdateNeeded", 1);
					setWString(hContact, DBKEY_AVATAR, wszNewUrl);
				}
			}
		}

		if (!data["page_info"]["has_next_page"].as_bool()) {
			debugLogA("Got no next page, exiting", szCursor.c_str());
			break;
		}
		
		szCursor = data["page_info"]["end_cursor"].as_mstring();
		debugLogA("Got cursor: %s", szCursor.c_str());
	}

	if (bNeedUpdate)
		ForkThread(&FacebookProto::AvatarsUpdate);
	return 0;
}

bool FacebookProto::RefreshSid()
{
	auto *pReq = CreateRequestGQL(FB_API_QUERY_SEQ_ID);
	JSONNode root; root << CHAR_PARAM("1", "0");
	pReq << CHAR_PARAM("query_params", root.write().c_str());
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (reply.error())
		return false;

	auto &n = reply.data()["viewer"]["message_threads"];
	CMStringW wszSid(n["sync_sequence_id"].as_mstring());
	setWString(DBKEY_SID, wszSid);
	m_sid = _wtoi64(wszSid);
	m_iUnread = n["unread_count"].as_int();
	return true;
}

FacebookUser* FacebookProto::RefreshThread(JSONNode &n)
{
	if (!n["is_group_thread"].as_bool())
		return nullptr;

	CMStringW chatId(n["thread_key"]["thread_fbid"].as_mstring());
	CMStringW name(n["name"].as_mstring());
	if (name.IsEmpty()) {
		for (auto &u : n["all_participants"]["nodes"]) {
			auto &ur = u["messaging_actor"];
			CMStringW userId(ur["id"].as_mstring());
			if (_wtoi64(userId) == m_uid)
				continue;

			if (!name.IsEmpty())
				name.Append(L", ");
			name += ur["name"].as_mstring();
		}

		if (name.GetLength() > 128) {
			name.Truncate(125);
			name.Append(L"...");
		}
	}

	auto *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, chatId, name);
	if (si == nullptr)
		return nullptr;

	setWString(si->hContact, DBKEY_ID, chatId);
	Chat_AddGroup(si, TranslateT("Participant"));

	for (auto &u : n["all_participants"]["nodes"]) {
		auto &ur = u["messaging_actor"];
		CMStringW userId(ur["id"].as_mstring());
		CMStringW userName(ur["name"].as_mstring());

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
		gce.pszID.w = chatId;
		gce.pszUID.w = userId;
		gce.pszNick.w = userName;
		gce.bIsMe = _wtoi64(userId) == m_uid;
		gce.time = time(0);
		Chat_Event(&gce);
	}

	Chat_Control(m_szModuleName, chatId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, chatId, SESSION_ONLINE);

	__int64 userId = _wtoi64(chatId);
	auto *pUser = FindUser(userId);

	if (pUser == nullptr) {
		mir_cslock lck(m_csUsers);
		pUser = new FacebookUser(userId, si->hContact, true, true);
		m_users.insert(pUser);
	}
	else {
		pUser->hContact = si->hContact;
		pUser->bIsChatInitialized = true;
	}

	return pUser;
}

FacebookUser* FacebookProto::RefreshThread(CMStringW &wszId)
{
	auto *pReq = CreateRequestGQL(FB_API_QUERY_THREAD);
	pReq << WCHAR_PARAM("query_params", CMStringW(FORMAT, L"{\"0\":[\"%s\"], \"12\":0, \"13\":\"false\"}", wszId.c_str()));
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (!reply.error()) {
		auto &root = reply.data();
		for (auto &n : root)
			return RefreshThread(n);
	}

	return nullptr;
}

void FacebookProto::RefreshThreads()
{
	int threadsLimit = 40;

	auto * pReq = CreateRequestGQL(FB_API_QUERY_THREADS);
	JSONNode json; json << INT_PARAM("1", threadsLimit) << CHAR_PARAM("2", "true") << CHAR_PARAM("12", "false") << CHAR_PARAM("13", "false");
	pReq << CHAR_PARAM("query_params", json.write().c_str());
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (!reply.error()) {
		auto &root = reply.data()["viewer"]["message_threads"];

		for (auto &n : root["nodes"]) {
			if (n["is_group_thread"].as_bool() && n["is_viewer_subscribed"].as_bool() && !n["has_viewer_archived"].as_bool())
				RefreshThread(n);
		}

		// TODO: save timestamp of last message/action/... into DB
		// TODO: lower threadsLimit to 10, load next pages if timestamp of last message is higher than timestamp in DB
	}
}

int FacebookProto::RefreshToken()
{
	auto *pReq = CreateRequest(FB_API_URL_AUTH, "authenticate", "auth.login");
	pReq->flags |= NLHRF_NODUMP;
	pReq << CHAR_PARAM("email", getMStringA(DBKEY_LOGIN));
	pReq << CHAR_PARAM("password", getMStringA(DBKEY_PASS));
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	if (reply.error())
		return reply.error();

	m_szAuthToken = reply.data()["access_token"].as_mstring();
	setString(DBKEY_TOKEN, m_szAuthToken);

	CMStringA m_szUid = reply.data()["uid"].as_mstring();
	setString(DBKEY_ID, m_szUid);
	m_uid = _atoi64(m_szUid);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::ServerThread(void *)
{
	m_QueueCreated = false;

LBL_Begin:
	m_szAuthToken = getMStringA(DBKEY_TOKEN);
	if (m_szAuthToken.IsEmpty()) {
		if (int iErrorCode = RefreshToken()) {
			ConnectionFailed(iErrorCode);
			return;
		}
	}

	int iErrorCode = RefreshContacts();
	if (iErrorCode != 0) {
		if (iErrorCode == 401) {
			delSetting(DBKEY_TOKEN);
			goto LBL_Begin;
		}

		ConnectionFailed(iErrorCode);
		return;
	}

	// connect to MQTT server
	m_mqttConn = Netlib_OpenConnection(m_hNetlibUser, "mqtt.facebook.com", 443, 0, NLOCF_SSL);
	if (m_mqttConn == nullptr) {
		debugLogA("connection failed, exiting");
		ConnectionFailed();
		return;
	}

	// send initial packet
	MqttLogin();

	while (!Miranda_IsTerminated()) {
		NETLIBSELECT nls = {};
		nls.hReadConns[0] = m_mqttConn;
		nls.dwTimeout = 1000;
		int ret = Netlib_Select(&nls);
		if (ret == SOCKET_ERROR) {
			debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
			break;
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

	OnLoggedOut();
}

/////////////////////////////////////////////////////////////////////////////////////////

int FacebookProto::OnMarkedRead(WPARAM, LPARAM hDbEvent)
{
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	if (m_bKeepUnread)
		return 0;

	JSONNode root; root << BOOL_PARAM("state", true) << INT_PARAM("syncSeqId", m_sid) << CHAR_PARAM("mark", "read");
	if (isChatRoom(hContact))
		root << CHAR_PARAM("threadFbId", getMStringA(hContact, DBKEY_ID));
	else
		root << CHAR_PARAM("otherUserFbId", getMStringA(hContact, DBKEY_ID));
	MqttPublish("/mark_thread", root);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::OnPublish(const char *topic, const uint8_t *p, size_t cbLen)
{
	FbThriftReader rdr;

	// that might be a zipped buffer
	if (cbLen >= 2) {
		size_t dataSize;
		void *pData = doUnzip(cbLen, p, dataSize);
		if (pData != nullptr) {
			debugLogA("UNZIP: %d bytes unzipped ok", dataSize);
			Netlib_Dump(m_mqttConn, pData, dataSize, false, 0);
			rdr.reset(dataSize, pData);
			mir_free(pData);
		}
	}

	if (rdr.size() == 0)
		rdr.reset(cbLen, (void*)p);

	if (!strcmp(topic, "/t_p"))
		OnPublishPresence(rdr);
	else if (!strcmp(topic, "/t_ms"))
		OnPublishMessage(rdr);
	else if (!strcmp(topic, "/orca_typing_notifications"))
		OnPublishUtn(rdr);
}

void FacebookProto::OnPublishPresence(FbThriftReader &rdr)
{
	char *str = nullptr;
	rdr.readStr(str);
	mir_free(str);

	bool bVal;
	uint8_t fieldType;
	uint16_t fieldId;
	rdr.readField(fieldType, fieldId);
	assert(fieldType == FB_THRIFT_TYPE_BOOL);
	assert(fieldId == 1);
	rdr.readBool(bVal);

	rdr.readField(fieldType, fieldId);
	assert(fieldType == FB_THRIFT_TYPE_LIST);
	assert(fieldId == 1);

	uint32_t size;
	rdr.readList(fieldType, size);
	assert(fieldType == FB_THRIFT_TYPE_STRUCT);

	debugLogA("Received list of presences: %d records", size);
	for (uint32_t i = 0; i < size; i++) {
		uint64_t userId, timestamp, voipBits;
		rdr.readField(fieldType, fieldId);
		assert(fieldType == FB_THRIFT_TYPE_I64);
		assert(fieldId == 1);
		rdr.readInt64(userId);

		uint32_t u32;
		rdr.readField(fieldType, fieldId);
		assert(fieldType == FB_THRIFT_TYPE_I32);
		assert(fieldId == 1);
		rdr.readInt32(u32);

		auto *pUser = FindUser(userId);
		if (pUser == nullptr)
			debugLogA("Skipping presence from unknown user %lld", userId);
		else {
			debugLogA("Presence from user %lld => %d", userId, u32);
			setWord(pUser->hContact, "Status", (u32 != 0) ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
		}

		rdr.readField(fieldType, fieldId);
		assert(fieldType == FB_THRIFT_TYPE_I64);
		assert(fieldId == 1 || fieldId == 3 || fieldId == 4);
		rdr.readInt64(timestamp);

		while (!rdr.isStop()) {
			rdr.readField(fieldType, fieldId);
			assert(fieldType == FB_THRIFT_TYPE_I64 || fieldType == FB_THRIFT_TYPE_I16 || fieldType == FB_THRIFT_TYPE_I32);																									
			rdr.readIntV(voipBits);
		}

		rdr.readByte(fieldType);
		assert(fieldType == FB_THRIFT_TYPE_STOP);
	}

	rdr.readByte(fieldType);
	assert(fieldType == FB_THRIFT_TYPE_STOP);
}

void FacebookProto::OnPublishUtn(FbThriftReader &rdr)
{
	JSONNode root = JSONNode::parse(rdr.rest());
	auto *pUser = FindUser(_wtoi64(root["sender_fbid"].as_mstring()));
	if (pUser != nullptr) {
		int length = (root["state"].as_int() == 0) ? PROTOTYPE_CONTACTTYPING_OFF : 60;
		CallService(MS_PROTO_CONTACTISTYPING, pUser->hContact, length);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	const char *messageType;
	void (FacebookProto:: *pFunc)(const JSONNode &);
}
static MsgHandlers[] =
{
	{ "deltaNewMessage",                     &FacebookProto::OnPublishPrivateMessage },
	{ "deltaThreadName",                     &FacebookProto::OnPublishThreadName },
	{ "deltaSentMessage",                    &FacebookProto::OnPublishSentMessage },
	{ "deltaReadReceipt",                    &FacebookProto::OnPublishReadReceipt },
	{ "deltaParticipantsAddedToGroupThread", &FacebookProto::OnPublishChatJoin },
	{ "deltaParticipantLeftGroupThread",     &FacebookProto::OnPublishChatLeave },
};

void FacebookProto::OnPublishMessage(FbThriftReader &rdr)
{
	uint8_t stop;
	if (rdr.isStop())
		rdr.readByte(stop);
	else {
		uint8_t type;
		uint16_t id;
		rdr.readField(type, id);
		_ASSERT(type == FB_THRIFT_TYPE_STRING);
		_ASSERT(id == 1 || id == 2);
		
		char *szShit = nullptr;
		rdr.readStr(szShit);
		mir_free(szShit);

		rdr.readByte(stop);
	}

	CMStringA szJson(rdr.rest());
	debugLogA("MS: <%s>", szJson.c_str());
	JSONNode root = JSONNode::parse(szJson);

	CMStringA errorCode = root["errorCode"].as_mstring();
	if (!errorCode.IsEmpty()) {
		if (!m_QueueCreated && (errorCode == "ERROR_QUEUE_OVERFLOW" || errorCode == "ERROR_QUEUE_NOT_FOUND" || errorCode == "ERROR_QUEUE_LOST" || errorCode == "ERROR_QUEUE_EXCEEDS_MAX_DELTAS")) {
			m_QueueCreated = true; // prevent queue creation request from being sent twice
			delSetting(DBKEY_SYNC_TOKEN); m_szSyncToken.Empty();
			delSetting(DBKEY_SID);        m_sid = 0;
			if (!RefreshSid()) {
				ConnectionFailed();
				return;
			}

			MqttQueueConnect();
		}
	}

	CMStringW str = root["lastIssuedSeqId"].as_mstring();
	if (!str.IsEmpty()) {
		setWString(DBKEY_SID, str);
		m_sid = _wtoi64(str);
	}

	str = root["syncToken"].as_mstring();
	if (!str.IsEmpty()) {
		m_szSyncToken = str;
		setString(DBKEY_SYNC_TOKEN, m_szSyncToken);
		return;
	}

	for (auto &it : root["deltas"]) {
		for (auto &handler : MsgHandlers) {
			auto &json = it[handler.messageType];
			if (json) {
				(this->*(handler.pFunc))(json);
				break;
			}
		}
	}
}

// new message arrived
struct
{
	const char *szTag, *szClientVersion;
}
static facebookClients[] =
{
	{ "source:titan:web",       "Facebook (website)" },
	{ "app_id:256002347743983", "Facebook (Facebook Messenger)" }
};

void FacebookProto::FetchAttach(const CMStringA &mid, __int64 fbid, CMStringA &szBody)
{
	for (int iAttempt = 0; iAttempt < 5; iAttempt++) {
		auto *pReq = CreateRequest(FB_API_URL_ATTACH, "getAttachment", "messaging.getAttachment");
		pReq << CHAR_PARAM("mid", mid) << INT64_PARAM("aid", fbid);
		pReq->CalcSig();

		JsonReply reply(ExecuteRequest(pReq));
		switch (reply.error()) {
		case 0:
			{
				std::string uri = reply.data()["redirect_uri"].as_string();
				std::string type = reply.data()["content_type"].as_string();
				if (!uri.empty())
					szBody.AppendFormat("\r\n%s: %s", TranslateU(type.find("image/") != -1 ? "Picture attachment" : "File attachment"), uri.c_str());
			}
			return;

		case 509: // attachment isn't ready, wait a bit and retry
			::Sleep(100);
			continue;

		default: // shit happened, exiting
			return;
		}
	}
}

void FacebookProto::OnPublishPrivateMessage(const JSONNode &root)
{
	auto &metadata = root["messageMetadata"];
	__int64 offlineId = _wtoi64(metadata["offlineThreadingId"].as_mstring());
	if (!offlineId) {
		debugLogA("We care about messages only, event skipped");
		return;
	}

	bool bIsChat;
	CMStringW wszUserId;
	auto *pUser = UserFromJson(metadata, wszUserId, bIsChat);

	if (!bIsChat && pUser == nullptr)
		pUser = AddContact(wszUserId, true);
	else if (bIsChat && (pUser == nullptr || !pUser->bIsChatInitialized)) // chat room does not exists or is not initialized
		pUser = RefreshThread(wszUserId);
	
	if (pUser == nullptr) {
		debugLogA("User not found and adding failed, event skipped");
		return;
	}

	for (auto &it : metadata["tags"]) {
		auto *szTagName = it.name();
		for (auto &cli : facebookClients) {
			if (!mir_strcmp(szTagName, cli.szTag)) {
				setString(pUser->hContact, "MirVer", cli.szClientVersion);
				break;
			}
		}
	}

	CMStringA szId(metadata["messageId"].as_mstring());
	if (CheckOwnMessage(pUser, offlineId, szId)) {
		debugLogA("own message <%s> skipped", szId.c_str());
		return;
	}

	if (db_event_getById(m_szModuleName, szId)) {
		debugLogA("this message <%s> was already stored, exiting", szId.c_str());
		return;
	}

	// parse message body
	CMStringA szBody(root["body"].as_string().c_str());
	if (szBody.IsEmpty())
		szBody = metadata["snippet"].as_string().c_str();

	// parse stickers
	CMStringA stickerId = root["stickerId"].as_mstring();
	if (!stickerId.IsEmpty()) {
		if (ServiceExists(MS_SMILEYADD_LOADCONTACTSMILEYS)) {
			CMStringW wszPath(FORMAT, L"%s\\%S\\Stickers", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
			CreateDirectoryTreeW(wszPath);

			bool bSuccess = false;
			CMStringW wszFileName(FORMAT, L"%s\\STK{%S}.png", wszPath.c_str(), stickerId.c_str());
			uint32_t dwAttrib = GetFileAttributesW(wszFileName);
			if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
				wszFileName.Format(L"%s\\STK{%S}.webp", wszPath.c_str(), stickerId.c_str());
				dwAttrib = GetFileAttributesW(wszFileName);
			}

			// new sticker
			if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
				auto *pReq = CreateRequestGQL(FB_API_QUERY_STICKER);
				pReq << CHAR_PARAM("query_params", CMStringA(FORMAT, "{\"0\":[\"%s\"]}", stickerId.c_str()));
				pReq->CalcSig();

				JsonReply reply(ExecuteRequest(pReq));
				if (!reply.error()) {
					for (auto &sticker : reply.data()) {
						// std::string szUrl = sticker["animated_image"]["uri"].as_string();
						// if (szUrl.empty())
						// 	szUrl = sticker["thread_image"]["uri"].as_string();
						// else
						// 	wszFileName.Format(L"%s\\STK{%S}.webp", wszPath.c_str(), stickerId.c_str());
						std::string szUrl = sticker["thread_image"]["uri"].as_string();

						NETLIBHTTPREQUEST req = {};
						req.cbSize = sizeof(req);
						req.flags = NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
						req.requestType = REQUEST_GET;
						req.szUrl = (char*)szUrl.c_str();

						NETLIBHTTPREQUEST *pReply = Netlib_HttpTransaction(m_hNetlibUser, &req);
						if (pReply != nullptr && pReply->resultCode == 200 && pReply->pData && pReply->dataLength) {
							bSuccess = true;
							FILE *out = _wfopen(wszFileName, L"wb");
							fwrite(pReply->pData, 1, pReply->dataLength, out);
							fclose(out);
						}
					}
				}
			}
			else bSuccess = true;

			if (bSuccess) {
				if (!szBody.IsEmpty())
					szBody += "\r\n";
				szBody += "STK{" + stickerId + "}";

				SMADD_CONT cont = { 1, m_szModuleName, wszFileName };
				CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));
			}
			else szBody += TranslateU("Sticker received");
		}
		else szBody += TranslateU("SmileyAdd plugin required to support stickers");
	}

	// parse attachments (links, files, ...)
	for (auto &it : root["attachments"]) {
		// madness... json inside json
		CMStringA szJson(it["xmaGraphQL"].as_mstring());
		if (szJson.IsEmpty()) {
			__int64 fbid = _wtoi64(it["fbid"].as_mstring());
			if (fbid == 0) {
				debugLogA("Neither a GQL nor an inline attachment, nothing to do");
				continue;
			}

			// inline attachment, request its description
			FetchAttach(szId, fbid, szBody);
			continue;
		}

		JSONROOT nBody(szJson);
		if (!nBody)
			continue;

		const JSONNode &attach = (*nBody).at((json_index_t)0)["story_attachment"];
		szBody += "\r\n-----------------------------------";

		CMStringA str = attach["url"].as_mstring();
		if (!str.IsEmpty()) {
			if (str.Left(8) == "fbrpc://") {
				int iStart = str.Find("target_url=");
				if (iStart != 0) {
					CMStringA tmp;

					iStart += 11;
					int iEnd = str.Find("&", iStart);
					if (iEnd != -1)
						tmp = str.Mid(iStart, iEnd - iStart);
					else
						tmp = str.Right(iStart);
					
					mir_urlDecode(tmp.GetBuffer());
					szBody.AppendFormat("\r\n\t%s: %s", TranslateU("URL"), tmp.c_str());
				}
			}
			else szBody.AppendFormat("\r\n\t%s: %s", TranslateU("URL"), str.c_str());
		}

		str = attach["title"].as_string().c_str();
		if (!str.IsEmpty())
			szBody.AppendFormat("\r\n\t%s: %s", TranslateU("Title"), str.c_str());

		str = attach["source"]["text"].as_string().c_str();
		if (!str.IsEmpty())
			szBody.AppendFormat("\r\n\t%s: %s", TranslateU("Source"), str.c_str());

		str = attach["description"]["text"].as_string().c_str();
		if (!str.IsEmpty())
			szBody.AppendFormat("\r\n\t%s: %s", TranslateU("Description"), str.c_str());

		str = attach["media"]["playable_url"].as_string().c_str();
		if (!str.IsEmpty())
			szBody.AppendFormat("\r\n\t%s: %s", TranslateU("Playable media"), str.c_str());
	}

	// if that's a group chat, send it to the room
	CMStringW wszActorFbId(metadata["actorFbId"].as_mstring());
	__int64 actorFbId = _wtoi64(wszActorFbId);

	if (pUser->bIsChat) {
		szBody.Replace("%", "%%");
		ptrW wszText(mir_utf8decodeW(szBody));

		// TODO: GC_EVENT_JOIN for chat participants which are missing (for example added later during group chat)

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
		gce.pszID.w = wszUserId;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.pszUID.w = wszActorFbId;
		gce.pszText.w = wszText;
		gce.time = time(0);
		gce.bIsMe = actorFbId == m_uid;
		Chat_Event(&gce);

		debugLogA("New channel %lld message from %S: %s", pUser->id, gce.pszUID.w, gce.pszText.w);
	}
	else { // otherwise store a private message
		PROTORECVEVENT pre = {};
		pre.timestamp = uint32_t(_wtoi64(metadata["timestamp"].as_mstring()) / 1000);
		pre.szMessage = (char *)szBody.c_str();
		pre.szMsgId = (char *)szId.c_str();

		if (m_uid == actorFbId)
			pre.flags |= PREF_SENT;

		ProtoChainRecvMsg(pUser->hContact, &pre);
	}
}

// changing thread name
void FacebookProto::OnPublishThreadName(const JSONNode &root)
{
	auto &metadata = root["messageMetadata"];
	__int64 offlineId = _wtoi64(metadata["offlineThreadingId"].as_mstring());
	if (!offlineId) {
		debugLogA("We care about messages only, event skipped");
		return;
	}

	bool bIsChat;
	CMStringW wszUserId;
	auto *pUser = UserFromJson(metadata, wszUserId, bIsChat);
	if (!bIsChat || pUser == nullptr)
		return;

	CMStringW wszTitle = root["name"].as_mstring();
	if (!wszTitle.IsEmpty())
		setWString(pUser->hContact, DBKEY_NICK, wszTitle);
	else
		delSetting(pUser->hContact, DBKEY_NICK);
}

// user joined chat 
void FacebookProto::OnPublishChatJoin(const JSONNode &root)
{
	auto &metadata = root["messageMetadata"];
	__int64 offlineId = _wtoi64(metadata["offlineThreadingId"].as_mstring());
	if (!offlineId) {
		debugLogA("We care about messages only, event skipped");
		return;
	}

	bool bIsChat;
	CMStringW wszUserId;
	auto *pUser = UserFromJson(metadata, wszUserId, bIsChat);
	if (!bIsChat || pUser == nullptr)
		return;

	CMStringW wszText(metadata["adminText"].as_mstring());
	for (auto &it : root["addedParticipants"]) {
		CMStringW wszNick(it["fullName"].as_mstring()), wszId(it["userFbId"].as_mstring());
		
		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
		gce.pszID.w = wszUserId;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.pszNick.w = wszNick;
		gce.pszUID.w = wszId;
		gce.pszText.w = wszText;
		gce.time = time(0);
		gce.bIsMe = _wtoi64(wszId) == m_uid;
		Chat_Event(&gce);
	}
}

// user left chat 
void FacebookProto::OnPublishChatLeave(const JSONNode &root)
{
	auto &metadata = root["messageMetadata"];
	__int64 offlineId = _wtoi64(metadata["offlineThreadingId"].as_mstring());
	if (!offlineId) {
		debugLogA("We care about messages only, event skipped");
		return;
	}

	bool bIsChat;
	CMStringW wszUserId;
	auto *pUser = UserFromJson(metadata, wszUserId, bIsChat);
	if (!bIsChat || pUser == nullptr)
		return;

	CMStringW wszText(metadata["adminText"].as_mstring()), wszId(root["leftParticipantFbId"].as_mstring());
	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_PART };
	gce.pszID.w = wszUserId;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszUID.w = wszId;
	gce.pszText.w = wszText;
	gce.time = time(0);
	gce.bIsMe = _wtoi64(wszId) == m_uid;
	Chat_Event(&gce);
}

// read notification
void FacebookProto::OnPublishReadReceipt(const JSONNode &root)
{
	CMStringW wszUserId;
	bool bIsChat;
	auto *pUser = UserFromJson(root, wszUserId, bIsChat);
	if (pUser == nullptr) {
		debugLogA("Message from unknown contact %S, ignored", wszUserId.c_str());
		return;
	}

	uint32_t timestamp = _wtoi64(root["watermarkTimestampMs"].as_mstring());
	for (MEVENT ev = db_event_firstUnread(pUser->hContact); ev != 0; ev = db_event_next(pUser->hContact, ev)) {
		DBEVENTINFO dbei = {};
		if (db_event_get(ev, &dbei))
			continue;

		if (dbei.timestamp > timestamp)
			break;

		if (!dbei.markedRead())
			db_event_markRead(pUser->hContact, ev);
	}
}

// my own message was sent
bool FacebookProto::CheckOwnMessage(FacebookUser *pUser, __int64 offlineId, const char *pszMsgId)
{
	COwnMessage tmp;
	if (!ExtractOwnMessage(offlineId, tmp))
		return false;

	if (pUser->bIsChat) {
		CMStringW wszId(FORMAT, L"%lld", m_uid);
		tmp.wszText.Replace(L"%", L"%%");

		wchar_t userId[100];
		_i64tow_s(pUser->id, userId, _countof(userId), 10);

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
		gce.pszID.w = userId;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.pszUID.w = wszId;
		gce.pszText.w = tmp.wszText;
		gce.time = time(0);
		gce.bIsMe = true;
		Chat_Event(&gce);
	}
	else ProtoBroadcastAck(pUser->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tmp.reqId, (LPARAM)pszMsgId);

	return true;
}

void FacebookProto::OnPublishSentMessage(const JSONNode &root)
{
	auto &metadata = root["messageMetadata"];

	__int64 offlineId = _wtoi64(metadata["offlineThreadingId"].as_mstring());

	CMStringW wszUserId;
	bool bIsChat;
	auto *pUser = UserFromJson(metadata, wszUserId, bIsChat);
	if (pUser == nullptr) {
		debugLogA("Message from unknown contact %s, ignored", wszUserId.c_str());
		return;
	}

	std::string szMsgId(metadata["messageId"].as_string());
	CheckOwnMessage(pUser, offlineId, szMsgId.c_str());
}
