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

static uint64_t sttPresenceTimeMs(const JSONNode &n)
{
	if (n.type() == JSON_NUMBER) {
		double v = n.as_float();
		if (v <= 0)
			return 0;
		// Server can return seconds or milliseconds.
		if (v < 20000000000.0)
			return (uint64_t)(v * 1000.0 + 0.5);
		return (uint64_t)(v + 0.5);
	}
	if (n.type() == JSON_STRING) {
		const char *s = n.as_string().c_str();
		if (s == nullptr || s[0] == 0)
			return 0;
		uint64_t v = _strtoui64(s, nullptr, 10);
		if (v == 0)
			return 0;
		if (v < 20000000000ull)
			return v * 1000ull;
		return v;
	}
	return 0;
}

void CMaxProto::ApplyPresenceToContact(MCONTACT hContact, const JSONNode &src)
{
	if (hContact == 0 || src.type() != JSON_NODE || isChatRoom(hContact))
		return;

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	ptrA contactUid(getStringA(hContact, DB_KEY_MAX_UID));
	const bool isSelfContact = (myUid != nullptr && myUid[0] != 0 && contactUid != nullptr && contactUid[0] != 0
		&& !mir_strcmp(myUid, contactUid));

	// Favorites/self dialog should follow protocol state, not server presence snapshots.
	if (isSelfContact) {
		setWord(hContact, "Status", (GetStatus() == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
		return;
	}

	bool hasOnline = false, isOnline = false;

	auto takeBool = [](const JSONNode &n, bool &has, bool &val) {
		if (n.type() == JSON_BOOL) {
			has = true;
			val = n.as_bool();
		}
	};
	takeBool(src["online"], hasOnline, isOnline);
	if (!hasOnline)
		takeBool(src["isOnline"], hasOnline, isOnline);
	if (!hasOnline && src["status"].type() == JSON_NUMBER) {
		hasOnline = true;
		isOnline = ((int)src["status"].as_float() == 1);
	}
	if (!hasOnline && src["status"].type() == JSON_STRING) {
		CMStringA s(src["status"].as_string().c_str());
		s.MakeLower();
		if (s == "online") {
			hasOnline = true;
			isOnline = true;
		}
		else if (s == "offline") {
			hasOnline = true;
			isOnline = false;
		}
	}

	const JSONNode &st = src["status"];
	if (st.type() == JSON_NODE) {
		takeBool(st["online"], hasOnline, isOnline);
		if (!hasOnline)
			takeBool(st["isOnline"], hasOnline, isOnline);
		if (!hasOnline && st["status"].type() == JSON_NUMBER) {
			hasOnline = true;
			isOnline = ((int)st["status"].as_float() == 1);
		}
	}

	uint64_t lastMs = 0;
	lastMs = sttPresenceTimeMs(src["lastSeen"]);
	if (lastMs == 0)
		lastMs = sttPresenceTimeMs(src["lastSeenAt"]);
	if (lastMs == 0)
		lastMs = sttPresenceTimeMs(src["seen"]);
	if (lastMs == 0)
		lastMs = sttPresenceTimeMs(src["lastActivity"]);
	if (lastMs == 0)
		lastMs = sttPresenceTimeMs(src["lastActivityAt"]);
	if (lastMs == 0 && st.type() == JSON_NODE) {
		lastMs = sttPresenceTimeMs(st["lastSeen"]);
		if (lastMs == 0)
			lastMs = sttPresenceTimeMs(st["lastSeenAt"]);
		if (lastMs == 0)
			lastMs = sttPresenceTimeMs(st["seen"]);
		if (lastMs == 0)
			lastMs = sttPresenceTimeMs(st["lastActivity"]);
		if (lastMs == 0)
			lastMs = sttPresenceTimeMs(st["lastActivityAt"]);
	}

	// Some presence updates indicate offline by sending only "seen" timestamp (without explicit status).
	if (!hasOnline && lastMs != 0) {
		hasOnline = true;
		isOnline = false;
	}

	if (hasOnline)
		setWord(hContact, "Status", isOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);

	if (lastMs != 0) {
		uint32_t sec = (uint32_t)(lastMs / 1000ull);
		setDword(hContact, "LastSeen", sec);
	}
}

static CMStringA sttMessageBodyUtf8(const JSONNode &msg)
{
	const JSONNode &t = msg["text"];
	if (t.type() == JSON_STRING && !t.as_string().empty())
		return CMStringA(t.as_string().c_str());
	return CMStringA();
}

struct CMaxIncomingFile
{
	CMStringA fileId;
	CMStringW name;
	int64_t size = -1;
	CMStringA directUrl;
	bool isPhoto = false;
};

static bool sttTakeHttpUrl(const JSONNode &a, const char *key, CMStringA &out)
{
	const JSONNode &n = a[key];
	if (n.type() != JSON_STRING || n.as_string().empty())
		return false;
	CMStringA v(n.as_string().c_str());
	if (_strnicmp(v.c_str(), "https://", 8) && _strnicmp(v.c_str(), "http://", 7))
		return false;
	out = v;
	return true;
}

static CMStringA sttSelectPhotoBestUrl(const JSONNode &a)
{
	CMStringA url;
	// Prefer "raw/original" variants when present.
	if (sttTakeHttpUrl(a, "baseRawUrl", url)) return url;
	if (sttTakeHttpUrl(a, "rawUrl", url)) return url;
	if (sttTakeHttpUrl(a, "originalUrl", url)) return url;
	if (sttTakeHttpUrl(a, "downloadUrl", url)) return url;
	if (sttTakeHttpUrl(a, "url", url)) return url;
	if (sttTakeHttpUrl(a, "baseUrl", url)) return url;
	return CMStringA();
}

static CMStringW sttPhotoFileNameFromAttach(const JSONNode &a, const CMStringA &fileId)
{
	// Max PHOTO push usually has no original filename; derive a stable one from payload mime + photoId.
	CMStringW ext = L"jpg";
	if (a["previewData"].type() == JSON_STRING) {
		CMStringA pd(a["previewData"].as_string().c_str());
		const char *pfx = "data:image/";
		if (!_strnicmp(pd.c_str(), pfx, mir_strlen(pfx))) {
			const char *p = pd.c_str() + mir_strlen(pfx);
			const char *q = strchr(p, ';');
			if (q != nullptr && q > p) {
				CMStringA mimeSub;
				for (const char *t = p; t < q; ++t) {
					char c = (char)tolower((unsigned char)*t);
					if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
						mimeSub.AppendChar(c);
				}
				if (!mimeSub.IsEmpty()) {
					if (!mir_strcmp(mimeSub.c_str(), "jpeg"))
						ext = L"jpg";
					else
						ext = mir_utf8decodeW(mimeSub.c_str());
				}
			}
		}
	}

	CMStringW out;
	out.Format(L"photo_%S.%s", fileId.c_str(), ext.c_str());
	return out;
}

static void sttCollectIncomingFiles(const JSONNode &msg, std::vector<CMaxIncomingFile> &out)
{
	const JSONNode &att = msg["attaches"];
	if (att.type() != JSON_ARRAY)
		return;

	for (unsigned i = 0; i < att.size(); ++i) {
		const JSONNode &a = att[(json_index_t)i];
		if (a.type() != JSON_NODE)
			continue;
		CMaxIncomingFile f;
		CMStringA at;
		if (a["_type"].type() == JSON_STRING)
			at = a["_type"].as_string().c_str();
		at.MakeUpper();

		if (!mir_strcmp(at.c_str(), "FILE")) {
			f.fileId = sttMsgJsonIdStr(a["fileId"]);
			if (f.fileId.IsEmpty())
				continue;
			if (a["name"].type() == JSON_STRING && !a["name"].as_string().empty())
				f.name = mir_utf8decodeW(a["name"].as_string().c_str());
			if (f.name.IsEmpty())
				f.name = L"file";
			if (a["size"].type() == JSON_NUMBER)
				f.size = (int64_t)(a["size"].as_float() + 0.5);
			else if (a["size"].type() == JSON_STRING)
				f.size = _strtoi64(a["size"].as_string().c_str(), nullptr, 10);
			out.push_back(f);
			continue;
		}

		if (!mir_strcmp(at.c_str(), "PHOTO")) {
			f.fileId = sttMsgJsonIdStr(a["photoId"]);
			if (f.fileId.IsEmpty())
				f.fileId = "photo";
			if (a["name"].type() == JSON_STRING && !a["name"].as_string().empty())
				f.name = mir_utf8decodeW(a["name"].as_string().c_str());
			if (a["size"].type() == JSON_NUMBER)
				f.size = (int64_t)(a["size"].as_float() + 0.5);
			else if (a["size"].type() == JSON_STRING)
				f.size = _strtoi64(a["size"].as_string().c_str(), nullptr, 10);
			f.directUrl = sttSelectPhotoBestUrl(a);
			if (f.directUrl.IsEmpty())
				continue;
			if (f.name.IsEmpty())
				f.name = sttPhotoFileNameFromAttach(a, f.fileId);
			f.isPhoto = true;
			out.push_back(f);
			continue;
		}
	}
}

static bool sttJsonNodeContainsTypingOn(const JSONNode &n)
{
	CMStringA s;
	if (n["event"].type() == JSON_STRING)
		s = n["event"].as_string().c_str();
	else if (n["type"].type() == JSON_STRING)
		s = n["type"].as_string().c_str();
	else if (n["action"].type() == JSON_STRING)
		s = n["action"].as_string().c_str();
	if (s.IsEmpty())
		return false;
	s.MakeLower();
	return strstr(s, "typing") != nullptr && strstr(s, "clear") == nullptr && strstr(s, "stop") == nullptr && strstr(s, "off") == nullptr;
}

static bool sttJsonNodeContainsTypingOff(const JSONNode &n)
{
	CMStringA s;
	if (n["event"].type() == JSON_STRING)
		s = n["event"].as_string().c_str();
	else if (n["type"].type() == JSON_STRING)
		s = n["type"].as_string().c_str();
	else if (n["action"].type() == JSON_STRING)
		s = n["action"].as_string().c_str();
	if (s.IsEmpty())
		return false;
	s.MakeLower();
	return strstr(s, "clear") != nullptr || strstr(s, "stop") != nullptr || strstr(s, "off") != nullptr;
}

static int sttDetectTypingStateFromMessage(const JSONNode &msg)
{
	// -1 unknown, 0 off, 1 on
	if (msg["typing"].type() == JSON_BOOL)
		return msg["typing"].as_bool() ? 1 : 0;
	if (msg["isTyping"].type() == JSON_BOOL)
		return msg["isTyping"].as_bool() ? 1 : 0;

	const JSONNode &att = msg["attaches"];
	if (att.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < att.size(); ++i) {
			const JSONNode &a = att[i];
			if (a.type() != JSON_NODE)
				continue;
			if (a["_type"].type() == JSON_STRING) {
				CMStringA t(a["_type"].as_string().c_str());
				t.MakeLower();
				if (strstr(t, "control") == nullptr)
					continue;
			}
			if (sttJsonNodeContainsTypingOff(a))
				return 0;
			if (sttJsonNodeContainsTypingOn(a))
				return 1;
		}
	}

	const JSONNode &elems = msg["elements"];
	if (elems.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < elems.size(); ++i) {
			const JSONNode &e = elems[i];
			if (e.type() != JSON_NODE)
				continue;
			if (sttJsonNodeContainsTypingOff(e))
				return 0;
			if (sttJsonNodeContainsTypingOn(e))
				return 1;
		}
	}

	if (sttJsonNodeContainsTypingOff(msg))
		return 0;
	if (sttJsonNodeContainsTypingOn(msg))
		return 1;
	return -1;
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

	const JSONNode &st = msg["status"];
	if (st.type() == JSON_STRING && !mir_strcmpi(st.as_string().c_str(), "REMOVED")) {
		MEVENT hEv = db_event_getById(m_szModuleName, msgId.c_str());
		if (hEv != 0) {
			db_event_delete(hEv, CDF_FROM_SERVER);
			debugLogA("Max: server removed msg id=%s chat=%s", msgId.c_str(), szChatId);
		}
		return;
	}

	MEVENT hExisting = db_event_getById(m_szModuleName, msgId.c_str());
	bool isEdited = false;
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
	std::vector<CMaxIncomingFile> files;
	sttCollectIncomingFiles(msg, files);
	if (text.IsEmpty() && files.empty())
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

	if (!text.IsEmpty()) {
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

	if (!files.empty()) {
		debugLogA("Max: files detected chat=%s msg=%s count=%u", szChatId, msgId.c_str(), (unsigned)files.size());
		for (auto &f : files) {
			CMStringA eventId;
			eventId.Format("%s:file:%s", msgId.c_str(), f.fileId.c_str());
			if (db_event_getById(m_szModuleName, eventId.c_str()) != 0)
				continue;

			CMStringA url;
			if (!f.directUrl.IsEmpty()) {
				url = f.directUrl;
				// For PHOTO, try to resolve server file URL (can provide original format in some accounts),
				// then fall back to directUrl from attach.
				if (f.isPhoto && WaitForGatewayReady() && m_pGateway != nullptr) {
					CMStringA altUrl;
					if (ApiGetFileDownloadUrl(m_pGateway, szChatId, msgId.c_str(), _strtoi64(f.fileId.c_str(), nullptr, 10), altUrl) && !altUrl.IsEmpty()) {
						debugLogA("Max: photo url replaced by opcode 88 chat=%s msg=%s file=%s", szChatId, msgId.c_str(), f.fileId.c_str());
						url = altUrl;
					}
				}
			}
			else {
				if (!WaitForGatewayReady() || m_pGateway == nullptr) {
					debugLogA("Max: file url unresolved (gateway offline) chat=%s msg=%s file=%s", szChatId, msgId.c_str(), f.fileId.c_str());
					continue;
				}
				if (!ApiGetFileDownloadUrl(m_pGateway, szChatId, msgId.c_str(), _strtoi64(f.fileId.c_str(), nullptr, 10), url) || url.IsEmpty()) {
					debugLogA("Max: file url unresolved chat=%s msg=%s file=%s", szChatId, msgId.c_str(), f.fileId.c_str());
					continue;
				}
			}

			DB::EventInfo fdbei;
			fdbei.eventType = EVENTTYPE_FILE;
			fdbei.bUtf = true;
			// Offline cloud file: avoid SRFile live-transfer dialog ("cannot start transfer") for incoming files.
			// For self-sent files keep it non-temporary to prevent auto-download into Received Files path.
			fdbei.bTemporary = !fromSelf;
			fdbei.szId = eventId.c_str();
			if (tMs != 0) {
				fdbei.bMsec = true;
				fdbei.iTimestamp = tMs;
			}
			else fdbei.iTimestamp = (uint64_t)time(nullptr);
			if (fromSelf) {
				fdbei.bSent = true;
				fdbei.bRead = true;
			}
			else if (bMarkRead)
				fdbei.bRead = true;

			ptrW wszName(mir_wstrdup(f.name.c_str()));
			DB::FILE_BLOB blob(wszName, nullptr);
			blob.setUrl(url.c_str());
			if (f.size > 0)
				blob.setSize(f.size);
			if (!isChatRoom(hContact))
				setString(hContact, DB_KEY_MAX_CHATID, szChatId);
			ProtoChainRecvFile(hContact, blob, fdbei);
			debugLogA("Max: ingested file chat=%s h=%u msg=%s file=%s", szChatId, (unsigned)hContact, msgId.c_str(), f.fileId.c_str());
		}
	}
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
		chatId = sttMsgJsonIdStr(payload["cid"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(msg["cid"]);
	if (chatId.IsEmpty())
		return;

	int typingState = sttDetectTypingStateFromMessage(msg);
	if (typingState != -1) {
		CMStringA sender = sttMsgJsonIdStr(msg["sender"]);
		MCONTACT hContact = ResolveContactForDialogMessage(chatId.c_str(), sender.IsEmpty() ? nullptr : sender.c_str());
		if (hContact != 0) {
			ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
			if (sender.IsEmpty() || myUid == nullptr || myUid[0] == 0 || mir_strcmp(sender.c_str(), myUid))
				CallService(MS_PROTO_CONTACTISTYPING, hContact, typingState ? 10 : PROTOTYPE_CONTACTTYPING_OFF);
		}
	}

	QueueLiveNotifIngest(payload);
}

void CMaxProto::TryIngestTypingPayload(const JSONNode &payload)
{
	if (payload.type() != JSON_NODE)
		return;

	CMStringA chatId = sttMsgJsonIdStr(payload["chatId"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["cid"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["message"]["cid"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["chat"]["id"]);
	if (chatId.IsEmpty()) {
		debugLogA("Max: typing skip (no chatId)");
		return;
	}

	CMStringA sender = sttMsgJsonIdStr(payload["userId"]);
	if (sender.IsEmpty())
		sender = sttMsgJsonIdStr(payload["sender"]);
	if (sender.IsEmpty())
		sender = sttMsgJsonIdStr(payload["uid"]);
	if (sender.IsEmpty() && payload["typingUsers"].type() == JSON_ARRAY && payload["typingUsers"].size() > 0)
		sender = sttMsgJsonIdStr(payload["typingUsers"][(json_index_t)0]);
	if (sender.IsEmpty() && payload["userIds"].type() == JSON_ARRAY && payload["userIds"].size() > 0)
		sender = sttMsgJsonIdStr(payload["userIds"][(json_index_t)0]);

	int typingState = -1;
	if (payload["typing"].type() == JSON_BOOL)
		typingState = payload["typing"].as_bool() ? 1 : 0;
	else if (payload["isTyping"].type() == JSON_BOOL)
		typingState = payload["isTyping"].as_bool() ? 1 : 0;
	else if (payload["typingUsers"].type() == JSON_ARRAY)
		typingState = payload["typingUsers"].size() > 0 ? 1 : 0;
	else if (payload["userIds"].type() == JSON_ARRAY)
		typingState = payload["userIds"].size() > 0 ? 1 : 0;
	else if (payload["message"].type() == JSON_NODE)
		typingState = sttDetectTypingStateFromMessage(payload["message"]);
	else {
		const JSONNode &ev = payload["event"];
		if (ev.type() == JSON_STRING && !ev.as_string().empty()) {
			CMStringA s(ev.as_string().c_str());
			s.MakeLower();
			typingState = (strstr(s, "stop") != nullptr || strstr(s, "clear") != nullptr || strstr(s, "off") != nullptr) ? 0 : 1;
		}
	}
	// Max NOTIF_TYPING (opcode 129) may carry only {chatId,userId} without explicit typing flag.
	// But payloads like {"type":"PHOTO"} are media notifications and must not trigger typing.
	if (typingState == -1 && !sender.IsEmpty()) {
		bool bLooksTyping = true;
		if (payload["type"].type() == JSON_STRING) {
			CMStringA typeStr(payload["type"].as_string().c_str());
			typeStr.MakeLower();
			bLooksTyping = (strstr(typeStr, "typing") != nullptr);
		}
		if (bLooksTyping)
			typingState = 1;
	}
	if (typingState == -1) {
		debugLogA("Max: typing skip chat=%s (unknown state format)", chatId.c_str());
		return;
	}

	MCONTACT hContact = ResolveContactForDialogMessage(chatId.c_str(), sender.IsEmpty() ? nullptr : sender.c_str());
	if (hContact == 0) {
		debugLogA("Max: typing skip chat=%s from=%s (contact not resolved)", chatId.c_str(), sender.IsEmpty() ? "(unknown)" : sender.c_str());
		return;
	}

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	if (!sender.IsEmpty() && myUid != nullptr && myUid[0] != 0 && !mir_strcmp(sender.c_str(), myUid)) {
		debugLogA("Max: typing skip chat=%s (self sender)", chatId.c_str());
		return;
	}

	CallService(MS_PROTO_CONTACTISTYPING, hContact, typingState ? 10 : PROTOTYPE_CONTACTTYPING_OFF);
	debugLogA("Max: typing chat=%s from=%s state=%s", chatId.c_str(), sender.IsEmpty() ? "(unknown)" : sender.c_str(), typingState ? "on" : "off");
}

void CMaxProto::TryIngestPresencePayload(const JSONNode &payload, int opcode)
{
	if (payload.type() != JSON_NODE)
		return;
	if (opcode != 132 && opcode != 35 && opcode != 159 && payload["presence"].type() == JSON_NULL
		&& payload["status"].type() == JSON_NULL && payload["online"].type() == JSON_NULL
		&& payload["lastSeen"].type() == JSON_NULL && payload["lastActivity"].type() == JSON_NULL
		&& payload["contact"].type() == JSON_NULL && payload["contacts"].type() == JSON_NULL)
		return;

	auto applyOne = [&](const JSONNode &n, const char *fallbackUid) {
		if (n.type() != JSON_NODE)
			return;
		CMStringA uid = sttMsgJsonIdStr(n["userId"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["id"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["uid"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["contactId"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["peerId"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["participantId"]);
		if (uid.IsEmpty() && n["user"].type() == JSON_NODE) {
			uid = sttMsgJsonIdStr(n["user"]["id"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["user"]["userId"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["user"]["uid"]);
		}
		if (uid.IsEmpty() && n["contact"].type() == JSON_NODE) {
			uid = sttMsgJsonIdStr(n["contact"]["id"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["contact"]["contactId"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["contact"]["userId"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["contact"]["uid"]);
		}
		if (uid.IsEmpty() && fallbackUid != nullptr && fallbackUid[0] != 0)
			uid = fallbackUid;
		if (uid.IsEmpty())
			return;
		MCONTACT h = FindContactByMaxUid(uid.c_str());
		if (!h) {
			debugLogA("Max: presence uid=%s not matched to local contact", uid.c_str());
			return;
		}
		ApplyPresenceToContact(h, n);
		debugLogA("Max: presence applied uid=%s hContact=%u", uid.c_str(), (unsigned)h);
	};

	CMStringA payloadUid = sttMsgJsonIdStr(payload["userId"]);
	if (payloadUid.IsEmpty())
		payloadUid = sttMsgJsonIdStr(payload["id"]);
	if (payloadUid.IsEmpty())
		payloadUid = sttMsgJsonIdStr(payload["uid"]);
	if (payloadUid.IsEmpty())
		payloadUid = sttMsgJsonIdStr(payload["contactId"]);

	if (payload["presence"].type() == JSON_NODE)
		applyOne(payload["presence"], payloadUid.IsEmpty() ? nullptr : payloadUid.c_str());
	if (payload["presence"].type() == JSON_ARRAY)
		for (unsigned i = 0; i < payload["presence"].size(); ++i)
			applyOne(payload["presence"][i], nullptr);
	if (payload["contacts"].type() == JSON_ARRAY)
		for (unsigned i = 0; i < payload["contacts"].size(); ++i)
			applyOne(payload["contacts"][i], nullptr);
	if (payload["contacts"].type() == JSON_NODE)
		for (auto it = payload["contacts"].begin(); it != payload["contacts"].end(); ++it)
			applyOne(*it, nullptr);
	if (payload["contact"].type() == JSON_NODE)
		applyOne(payload["contact"], nullptr);
	if (payload["user"].type() == JSON_NODE)
		applyOne(payload["user"], nullptr);

	const bool hasNestedPresence = (payload["presence"].type() != JSON_NULL
		|| payload["contacts"].type() != JSON_NULL
		|| payload["contact"].type() != JSON_NULL
		|| payload["user"].type() != JSON_NULL);
	if (!hasNestedPresence)
		applyOne(payload, nullptr);
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
		if (msgId.IsEmpty())
			continue;

		const JSONNode &st = msg["status"];
		if (st.type() == JSON_STRING && !mir_strcmpi(st.as_string().c_str(), "REMOVED")) {
			if (MEVENT hEv = db_event_getById(m_szModuleName, msgId.c_str()))
				db_event_delete(hEv, CDF_FROM_SERVER);
			continue;
		}

		if (db_event_getById(m_szModuleName, msgId.c_str()) != 0)
			continue;

		IngestMaxMessageJson(msg, szChatId, bMarkRead);
	}
}
