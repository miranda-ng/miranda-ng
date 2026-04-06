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

void CMaxProto::TryIngestNotifMessagePayload(const JSONNode &payload)
{
	const JSONNode &msg = payload["message"];
	if (msg.type() != JSON_NODE)
		return;

	const JSONNode &typeNd = msg["type"];
	if (typeNd.type() == JSON_STRING && mir_strcmp(typeNd.as_string().c_str(), "USER"))
		return;

	CMStringA msgId = sttMsgJsonIdStr(msg["id"]);
	if (msgId.IsEmpty())
		return;

	if (db_event_getById(m_szModuleName, msgId.c_str()) != 0)
		return;

	CMStringA chatId = sttMsgJsonIdStr(payload["chatId"]);
	if (chatId.IsEmpty())
		return;

	CMStringA sender = sttMsgJsonIdStr(msg["sender"]);
	CMStringA text = sttMessageBodyUtf8(msg);
	if (text.IsEmpty())
		return;

	MCONTACT hContact = FindContactByDialogChatId(chatId.c_str());
	if (!hContact)
		hContact = FindContactByMaxUid(sender.c_str());
	if (!hContact)
		hContact = EnsureUserContact(sender.c_str(), nullptr, nullptr, chatId.c_str());
	if (!hContact)
		return;

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	const bool fromSelf = (myUid != nullptr && sender == myUid);

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

	ProtoChainRecvMsg(hContact, dbei);
	debugLogA("Max: ingested msg id=%s chat=%s from=%s", msgId.c_str(), chatId.c_str(), sender.c_str());
}
