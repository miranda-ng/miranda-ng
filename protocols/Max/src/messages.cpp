/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"

static CMStringA sttMsgJsonIdStr(const JSONNode &n)
{
	if (n.type() == JSON_NULL)
		return "";
	if (n.type() == JSON_NUMBER) {
		CMStringA s;
		s.Format("%.0f", n.as_float());
		return s;
	}
	return CMStringA(n.as_string().c_str());
}

static CMStringA sttMessageBodyUtf8(const JSONNode &msg)
{
	const JSONNode &t = msg["text"];
	if (t.type() == JSON_STRING && !t.as_string().empty())
		return CMStringA(t.as_string().c_str());
	return CMStringA();
}

/// 1:1 dialog id from Max: chatId = myMaxUid XOR peerMaxUid (decimal string ids).
static CMStringA sttXorPeerUidFromDialog(const char *szMyUid, const char *szChatId)
{
	if (szMyUid == nullptr || szMyUid[0] == 0 || szChatId == nullptr || szChatId[0] == 0)
		return CMStringA();
	uint64_t a = _strtoui64(szMyUid, nullptr, 10);
	uint64_t c = _strtoui64(szChatId, nullptr, 10);
	if (c == 0)
		return CMStringA();
	uint64_t b = a ^ c;
	CMStringA out;
	out.Format("%llu", (unsigned long long)b);
	return out;
}

MCONTACT CMaxProto::ResolveContactForDialogMessage(const char *szChatId, const char *senderUid)
{
	if (szChatId == nullptr || szChatId[0] == 0)
		return 0;

	MCONTACT hContact = FindContactByDialogChatId(szChatId);
	if (hContact)
		return hContact;

	for (auto &hc : AccContacts()) {
		if (isChatRoom(hc))
			continue;
		CMStringA rid = GetOrResolveDialogChatId(hc, false);
		if (!rid.IsEmpty() && !mir_strcmp(rid, szChatId))
			return hc;
	}

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	const bool fromSelf = (senderUid != nullptr && senderUid[0] != 0 && myUid != nullptr && myUid[0] != 0
		&& !mir_strcmp(senderUid, myUid));

	if (!fromSelf) {
		if (senderUid != nullptr && senderUid[0] != 0) {
			hContact = FindContactByMaxUid(senderUid);
			if (hContact)
				return hContact;
			return EnsureUserContact(senderUid, nullptr, nullptr, szChatId);
		}
		return 0;
	}

	CMStringA peerStr = sttXorPeerUidFromDialog(myUid, szChatId);
	if (peerStr.IsEmpty() || !mir_strcmp(peerStr, myUid))
		return 0;

	hContact = FindContactByMaxUid(peerStr.c_str());
	if (hContact)
		return hContact;
	return EnsureUserContact(peerStr.c_str(), nullptr, nullptr, szChatId);
}

MCONTACT CMaxProto::FindContactByDialogChatId(const char *szChatId)
{
	if (szChatId == nullptr || szChatId[0] == 0)
		return 0;

	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact))
			continue;
		ptrA cid(getStringA(hContact, DB_KEY_MAX_CHATID));
		if (cid != nullptr && !mir_strcmp(cid, szChatId))
			return hContact;
	}
	return 0;
}

CMStringA CMaxProto::GetOrResolveDialogChatId(MCONTACT hContact, bool bPersistIfDerived)
{
	if (hContact == 0)
		return CMStringA();

	CMStringA chatId(getMStringA(hContact, DB_KEY_MAX_CHATID));
	if (!chatId.IsEmpty())
		return chatId;

	ptrA peer(getStringA(hContact, DB_KEY_MAX_UID));
	if (peer == nullptr || peer[0] == 0)
		return CMStringA();

	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my == nullptr || my[0] == 0)
		return CMStringA();

	uint64_t a = _strtoui64(my, nullptr, 10);
	uint64_t b = _strtoui64(peer, nullptr, 10);
	if (b == 0)
		return CMStringA();

	CMStringA resolved;
	resolved.Format("%llu", (unsigned long long)(a ^ b));

	if (bPersistIfDerived && !resolved.IsEmpty())
		setString(hContact, DB_KEY_MAX_CHATID, resolved.c_str());

	return resolved;
}

void CMaxProto::IngestMaxMessageJson(const JSONNode &msg, const char *szChatId, bool bMarkRead)
{
	if (msg.type() != JSON_NODE || szChatId == nullptr || szChatId[0] == 0)
		return;

	const JSONNode &typeNd = msg["type"];
	if (typeNd.type() == JSON_STRING && mir_strcmpi(typeNd.as_string().c_str(), "USER"))
		return;

	CMStringA msgId = sttMsgJsonIdStr(msg["id"]);
	if (msgId.IsEmpty())
		return;

	MEVENT hExisting = db_event_getById(m_szModuleName, msgId.c_str());
	bool isEdited = false;
	const JSONNode &st = msg["status"];
	if (st.type() == JSON_STRING && !mir_strcmpi(st.as_string().c_str(), "EDITED"))
		isEdited = true;
	else if (msg["updateTime"].type() != JSON_NULL)
		isEdited = true;

	// Existing server message id is normally a duplicate push.
	// Keep only explicit edits to update text in place.
	if (hExisting != 0 && !isEdited)
		return;

	CMStringA sender = sttMsgJsonIdStr(msg["sender"]);
	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	const bool fromSelf = (myUid != nullptr && sender == myUid);

	CMStringA text = sttMessageBodyUtf8(msg);
	if (text.IsEmpty())
		return;

	MCONTACT hContact = ResolveContactForDialogMessage(szChatId, sender.IsEmpty() ? nullptr : sender.c_str());
	if (!hContact)
		return;

	const JSONNode &t = msg["time"];
	uint64_t tMs = 0;
	if (t.type() == JSON_NUMBER)
		tMs = (uint64_t)(t.as_float() + 0.5);
	else if (t.type() == JSON_STRING)
		tMs = _strtoui64(t.as_string().c_str(), nullptr, 10);

	DB::EventInfo dbei;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.bUtf = true;
	dbei.szId = msgId.c_str();
	if (isEdited)
		dbei.bEdited = true;
	if (tMs != 0) {
		dbei.bMsec = true;
		dbei.iTimestamp = tMs;
	}
	else
		dbei.iTimestamp = (uint64_t)time(nullptr);

	dbei.cbBlob = (int)mir_strlen(text.c_str());
	dbei.pBlob = mir_strdup(text.c_str());

	if (fromSelf) {
		dbei.bSent = true;
		dbei.bRead = true;
	}
	else if (bMarkRead)
		dbei.bRead = true;

	ProtoChainRecvMsg(hContact, dbei);
	debugLogA("Max: ingested msg id=%s chat=%s from=%s", msgId.c_str(), szChatId, sender.c_str());
}

uint64_t CMaxProto::GetLastLocalMessageTimeMs(MCONTACT hContact)
{
	if (hContact == 0)
		return 0;

	for (MEVENT hEv = db_event_last(hContact); hEv; hEv = db_event_prev(hContact, hEv)) {
		DB::EventInfo ei(hEv, false);
		if (!ei)
			continue;
		if (mir_strcmp(ei.szModule, m_szModuleName))
			continue;
		if (ei.eventType != EVENTTYPE_MESSAGE)
			continue;
		if (ei.bMsec)
			return (uint64_t)ei.iTimestamp;
		return (uint64_t)ei.iTimestamp * 1000;
	}
	return 0;
}

void CMaxProto::TryIngestNotifMessagePayload(const JSONNode &payload)
{
	const JSONNode &msg = payload["message"];
	if (msg.type() != JSON_NODE)
		return;

	CMStringA chatId = sttMsgJsonIdStr(payload["chatId"]);
	if (chatId.IsEmpty())
		return;

	QueueLiveNotifIngest(payload);
}

void CMaxProto::IngestChatHistoryPayload(const JSONNode &payload, const char *szChatId, bool bMarkRead)
{
	if (szChatId == nullptr || szChatId[0] == 0)
		return;

	const JSONNode *msgs = nullptr;
	if (payload["messages"].type() == JSON_ARRAY)
		msgs = &payload["messages"];
	else {
		const JSONNode &ch = payload["chat"];
		if (ch.type() == JSON_NODE && ch["messages"].type() == JSON_ARRAY)
			msgs = &ch["messages"];
	}

	if (msgs == nullptr || msgs->type() != JSON_ARRAY)
		return;

	unsigned n = (unsigned)msgs->size();
	debugLogA("Max: history chat=%s messages=%u", szChatId, n);
	for (unsigned i = 0; i < n; i++) {
		const JSONNode &msg = (*msgs)[i];
		CMStringA msgId = sttMsgJsonIdStr(msg["id"]);
		if (msgId.IsEmpty() || db_event_getById(m_szModuleName, msgId.c_str()) != 0)
			continue;

		IngestMaxMessageJson(msg, szChatId, bMarkRead);
	}
}
