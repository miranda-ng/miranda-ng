/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////

static CMStringA sttJsonIdStr(const JSONNode &n)
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

static bool sttUidInContactsArray(const JSONNode &contacts, const char *uid)
{
	if (uid == nullptr || uid[0] == 0 || contacts.type() != JSON_ARRAY)
		return false;
	for (unsigned i = 0; i < contacts.size(); i++) {
		if (sttJsonIdStr(contacts[i]["id"]) == uid)
			return true;
	}
	return false;
}

static void sttPushUniqueUid(std::vector<CMStringA> &out, const CMStringA &uid)
{
	if (uid.IsEmpty())
		return;
	for (const auto &x : out)
		if (x == uid)
			return;
	out.push_back(uid);
}

// Contact payload: server marks deleted address-book entries.
static bool sttContactPayloadSaysRemoved(const JSONNode &c)
{
	const JSONNode &st = c["status"];
	if (st.type() != JSON_STRING)
		return false;
	const char *s = st.as_string().c_str();
	if (!mir_strcmpi(s, "REMOVED"))
		return true;
	if (!mir_strcmpi(s, "DELETED"))
		return true;
	return false;
}

// Chat payload: dialog no longer belongs on the client roster (deleted/hidden server-side).
static bool sttChatExcludedFromRoster(const JSONNode &chat)
{
	const JSONNode &st = chat["status"];
	if (st.type() != JSON_STRING)
		return false;
	const char *s = st.as_string().c_str();
	if (!mir_strcmpi(s, "REMOVED"))
		return true;
	if (!mir_strcmpi(s, "DELETED"))
		return true;
	if (!mir_strcmpi(s, "ARCHIVED"))
		return true;
	if (!mir_strcmpi(s, "LEFT"))
		return true;
	if (!mir_strcmpi(s, "CLOSED"))
		return true;
	if (!mir_strcmpi(s, "INACTIVE"))
		return true;
	return false;
}

static void sttMergeOneDialogChat(CMaxProto *p, const JSONNode &chat, const CMStringA &myUid, const JSONNode *pAllContacts,
	OBJLIST<CMStringA> *pNeedFetch, CMStringA *pOutPeerUid, std::vector<CMStringA> *pAllowedUids);

static bool sttTypeIsDialog(const CMStringA &t)
{
	return t.IsEmpty() || !mir_strcmpi(t.c_str(), "DIALOG");
}

// Some payloads use a tiny "id" (ordinal) while cid/chatId holds the real dialog id used by opcode 48.
static CMStringA sttResolveDialogChatId(const JSONNode &chat)
{
	CMStringA id = sttJsonIdStr(chat["id"]);
	CMStringA cid = sttJsonIdStr(chat["cid"]);
	CMStringA ch = sttJsonIdStr(chat["chatId"]);
	CMStringA lcid = sttJsonIdStr(chat["lastMessage"]["cid"]);

	uint64_t idv = id.IsEmpty() ? 0 : _strtoui64(id.c_str(), nullptr, 10);
	uint64_t cidv = cid.IsEmpty() ? 0 : _strtoui64(cid.c_str(), nullptr, 10);
	uint64_t chv = ch.IsEmpty() ? 0 : _strtoui64(ch.c_str(), nullptr, 10);

	if (!id.IsEmpty() && idv == 0)
		return id; // Favorites/self dialog uses chatId==0 in server APIs

	if (!cid.IsEmpty() && idv != 0 && idv < 10000 && cidv >= 10000)
		return cid;
	if (!ch.IsEmpty() && idv != 0 && idv < 10000 && chv >= 10000)
		return ch;

	// Some payloads (notably self dialog / Favorites) have id==0 while cid/lastMessage.cid carries the real dialog id.
	if (idv != 0)
		return id;
	if (!cid.IsEmpty())
		return cid;
	if (!ch.IsEmpty())
		return ch;
	return lcid;
}

// Do not use "first participant that is not me" — array order and object key order are unreliable.
static CMStringA sttPickDialogPeerUid(CMaxProto *p, const JSONNode &chat, CMStringA myUid)
{
	if (myUid.IsEmpty() && p != nullptr) {
		ptrA db(p->getStringA(DB_KEY_MY_MAX_ID));
		if (db != nullptr && db[0])
			myUid = db.get();
	}

	const JSONNode &pt = chat["participants"];
	OBJLIST<CMStringA> ids(8);

	if (pt.type() == JSON_NODE) {
		for (auto it = pt.begin(); it != pt.end(); ++it) {
			CMStringA k((*it).name());
			if (!k.IsEmpty())
				ids.insert(new CMStringA(k));
		}
	}
	else if (pt.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < pt.size(); i++) {
			const JSONNode &mem = pt[i];
			CMStringA cand = sttJsonIdStr(mem["id"]);
			if (cand.IsEmpty())
				cand = sttJsonIdStr(mem["userId"]);
			if (!cand.IsEmpty())
				ids.insert(new CMStringA(cand));
		}
	}

	CMStringA sender;
	const JSONNode &lm = chat["lastMessage"];
	if (lm.type() == JSON_NODE)
		sender = sttJsonIdStr(lm["sender"]);

	auto idInList = [&](const CMStringA &x) -> bool {
		if (x.IsEmpty())
			return false;
		for (int i = 0; i < ids.getCount(); i++)
			if (ids[i] == x)
				return true;
		return false;
	};

	for (int i = ids.getCount() - 1; i >= 0; i--)
		if (!myUid.IsEmpty() && ids[i] == myUid)
			ids.remove(i);

	if (ids.getCount() == 1)
		return ids[0];

	if (ids.getCount() == 0) {
		if (!sender.IsEmpty() && (myUid.IsEmpty() || sender != myUid))
			return sender;
		return CMStringA();
	}

	if (!sender.IsEmpty() && (myUid.IsEmpty() || sender != myUid) && idInList(sender))
		return sender;

	uint64_t myv = myUid.IsEmpty() ? 0 : _strtoui64(myUid.c_str(), nullptr, 10);
	bool allSmall = true;
	for (int j = 0; j < ids.getCount(); j++) {
		if (_strtoui64(ids[j].c_str(), nullptr, 10) >= 1000000ull) {
			allSmall = false;
			break;
		}
	}
	uint64_t sndv = sender.IsEmpty() ? 0 : _strtoui64(sender.c_str(), nullptr, 10);
	if (allSmall && myv >= 1000000ull && sndv >= 1000000ull && (myUid.IsEmpty() || sender != myUid))
		return sender;

	uint64_t bestV = 0;
	CMStringA best;
	for (int j = 0; j < ids.getCount(); j++) {
		uint64_t v = _strtoui64(ids[j].c_str(), nullptr, 10);
		if (v >= bestV) {
			bestV = v;
			best = ids[j];
		}
	}
	return best;
}

// Official / bot DIALOGs (e.g. MAX notifications) often appear in chats[] but not in contacts[]; keep them visible with contact-book filter.
static void sttAugmentContactBookFromBotDialogs(CMaxProto *p, const JSONNode &chats, const CMStringA &myUid,
	std::vector<CMStringA> &contactBookFromSync, std::vector<CMStringA> &allowedUids)
{
	if (chats.type() != JSON_ARRAY || p == nullptr)
		return;

	for (unsigned i = 0; i < chats.size(); i++) {
		const JSONNode &chat = chats[i];
		const JSONNode &hb = chat["hasBots"];
		bool hasBots = false;
		if (hb.type() == JSON_BOOL)
			hasBots = hb.as_bool();
		else if (hb.type() == JSON_NUMBER)
			hasBots = (hb.as_int() != 0);
		if (!hasBots)
			continue;

		CMStringA chatId = sttResolveDialogChatId(chat);
		if (chatId.IsEmpty())
			continue;

		const JSONNode &ptOrig = chat["participants"];
		unsigned partCount = 0;
		if (ptOrig.type() == JSON_NODE) {
			for (auto it = ptOrig.begin(); it != ptOrig.end(); ++it)
				partCount++;
		}
		else if (ptOrig.type() == JSON_ARRAY)
			partCount = (unsigned)ptOrig.size();

		CMStringA typ(chat["type"].type() != JSON_NULL ? chat["type"].as_string().c_str() : "");
		bool treatAsDialog = sttTypeIsDialog(typ);
		if (typ.IsEmpty() && partCount > 2)
			treatAsDialog = false;
		if (!treatAsDialog)
			continue;

		CMStringA peerUid = sttPickDialogPeerUid(p, chat, myUid);
		if (peerUid.IsEmpty())
			continue;

		CMStringA myEff = myUid;
		if (myEff.IsEmpty()) {
			ptrA db(p->getStringA(DB_KEY_MY_MAX_ID));
			if (db != nullptr && db[0])
				myEff = db.get();
		}
		if (!myEff.IsEmpty() && peerUid == myEff)
			continue;

		sttPushUniqueUid(contactBookFromSync, peerUid);
		sttPushUniqueUid(allowedUids, peerUid);
	}
}

static const JSONNode &sttResolveSyncPayload(const JSONNode &payload)
{
	if (payload["chats"].type() == JSON_ARRAY || payload["contacts"].type() == JSON_ARRAY
	    || payload["profile"].type() != JSON_NULL)
		return payload;

	static const char *inners[] = { "sync", "snapshot", "state", "data", "result", "fullSync", "payload", nullptr };
	for (int i = 0; inners[i]; i++) {
		const JSONNode &n = payload[inners[i]];
		if (n.type() == JSON_NODE
		    && (n["chats"].type() == JSON_ARRAY || n["contacts"].type() == JSON_ARRAY || n["profile"].type() != JSON_NULL))
			return n;
	}
	return payload;
}

static const JSONNode &sttResolveContactsFetchPayload(const JSONNode &payload)
{
	if (payload["contacts"].type() == JSON_ARRAY || payload["contact"].type() == JSON_NODE)
		return payload;
	if (payload["contacts"].type() == JSON_NODE && payload["contacts"].size() != 0)
		return payload;

	static const char *inners[] = { "result", "data", "response", "payload", nullptr };
	for (int i = 0; inners[i]; i++) {
		const JSONNode &n = payload[inners[i]];
		if (n.type() == JSON_NODE
		    && (n["contacts"].type() == JSON_ARRAY || n["contact"].type() == JSON_NODE
		        || (n["contacts"].type() == JSON_NODE && n["contacts"].size() != 0)))
			return n;
	}
	return payload;
}

// Fill FirstName / LastName from one names[] element (prefers firstName/lastName, then splits "name", then nick keys).
static void sttNamePartsFromRecord(const JSONNode &rec, CMStringW &outFn, CMStringW &outLn)
{
	outFn.Empty();
	outLn.Empty();
	if (rec.type() != JSON_NODE)
		return;

	if (rec["firstName"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(rec["firstName"].as_string().c_str()));
		if (w && w[0])
			outFn = w;
	}
	if (rec["lastName"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(rec["lastName"].as_string().c_str()));
		if (w && w[0])
			outLn = w;
	}
	if (!outFn.IsEmpty() || !outLn.IsEmpty())
		return;

	if (rec["name"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(rec["name"].as_string().c_str()));
		if (w && w[0])
			outFn = w;
	}
}

// Strict name extraction: only firstName/lastName/name from names[] or direct contact object.
static bool sttFillNamePartsDepth(const JSONNode &contact, int, CMStringW &outFn, CMStringW &outLn)
{
	outFn.Empty();
	outLn.Empty();

	const JSONNode &nm = contact["names"];
	if (nm.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < nm.size(); i++) {
			sttNamePartsFromRecord(nm[i], outFn, outLn);
			if (!outFn.IsEmpty() || !outLn.IsEmpty())
				return true;
		}
	}
	else if (nm.type() == JSON_NODE) {
		sttNamePartsFromRecord(nm, outFn, outLn);
		if (!outFn.IsEmpty() || !outLn.IsEmpty())
			return true;
	}
	sttNamePartsFromRecord(contact, outFn, outLn);
	if (!outFn.IsEmpty() || !outLn.IsEmpty())
		return true;
	return false;
}

static void sttFillNamePartsFromContact(const JSONNode &contact, CMStringW &outFn, CMStringW &outLn)
{
	sttFillNamePartsDepth(contact, 0, outFn, outLn);
}

void CMaxProto::FillNameFromMaxContactJson(const JSONNode &c, CMStringW &outFn, CMStringW &outLn)
{
	sttFillNamePartsFromContact(c, outFn, outLn);
}

static bool sttIsUserStubDisplay(const CMStringW &s)
{
	return !s.IsEmpty() && s.GetLength() >= 5 && !_wcsnicmp(s.c_str(), L"User ", 5);
}

// True if dialog title / stub should still be applied (no real name yet).
static bool sttContactNeedsPlaceholderName(CMaxProto *p, MCONTACT h)
{
	if (!h)
		return true;
	CMStringW fn = p->getMStringW(h, "FirstName");
	CMStringW ln = p->getMStringW(h, "LastName");
	if (sttIsUserStubDisplay(fn))
		return true;
	return fn.IsEmpty() && ln.IsEmpty();
}

// True if JSON carries a user-defined / address-book style name (not only profile ONEME).
static bool sttJsonHasLocalOrAliasContactName(const JSONNode &contact)
{
	(void)contact;
	return false;
}

static CMStringW sttGroupTitle(const JSONNode &chat)
{
	if (chat["title"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(chat["title"].as_string().c_str()));
		if (w && w[0])
			return CMStringW(w);
	}
	CMStringA id = sttResolveDialogChatId(chat);
	CMStringW r;
	r.Format(L"Chat %S", id.c_str());
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW CMaxProto::GetDefaultGroupW()
{
	ptrW w(getWStringA(DB_KEY_DEFAULT_GROUP));
	if (w != nullptr && w[0])
		return CMStringW(w);
	return L"Max";
}

void CMaxProto::RegisterChatModule()
{
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_DATABASE;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);
}

MCONTACT CMaxProto::FindContactByMaxUid(const char *szUid)
{
	if (szUid == nullptr || szUid[0] == 0)
		return 0;

	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact))
			continue;
		ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
		if (uid != nullptr && !mir_strcmp(uid, szUid))
			return hContact;
	}
	return 0;
}

void CMaxProto::RemoveMaxUserContact(const char *szUid)
{
	if (szUid == nullptr || szUid[0] == 0)
		return;
	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my != nullptr && my[0] && !mir_strcmp(szUid, my))
		return;

	{
		mir_cslock lck(m_csContactBook);
		for (auto it = m_contactBookUids.begin(); it != m_contactBookUids.end(); ) {
			if (*it == szUid)
				it = m_contactBookUids.erase(it);
			else
				++it;
		}
	}

	auto findOne = [&]() -> MCONTACT {
		MCONTACT h = FindContactByMaxUid(szUid);
		if (h)
			return h;
		if (my != nullptr && my[0]) {
			uint64_t a = _strtoui64(my, nullptr, 10);
			uint64_t b = _strtoui64(szUid, nullptr, 10);
			if (a != 0 && b != 0) {
				CMStringA chatId;
				chatId.Format("%llu", (unsigned long long)(a ^ b));
				MCONTACT h2 = FindContactByDialogChatId(chatId.c_str());
				if (h2) {
					ptrA u(getStringA(h2, DB_KEY_MAX_UID));
					if (u == nullptr || u[0] == 0 || !mir_strcmp(u, szUid))
						return h2;
				}
			}
		}
		return 0;
	};

	bool removed = false;
	while (MCONTACT h = findOne()) {
		debugLogA("Max: removing local contact uid=%s h=%u (no longer on server roster for this account)", szUid, (unsigned)h);
		db_delete_contact(h, 0);
		removed = true;
	}
	if (!removed)
		debugLogA("Max: RemoveMaxUserContact uid=%s — no local hContact (book updated)", szUid);
}

void CMaxProto::RemoveLocalPeerIfChatOnly(MCONTACT hContact)
{
	if (hContact == 0 || isChatRoom(hContact))
		return;
	if (!m_bContactBookSnapshotApplied)
		return;
	ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
	if (uid == nullptr || uid[0] == 0)
		return;
	// Never prune self contact (Favorites uses uid==myUid).
	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my != nullptr && my[0] && !mir_strcmp(uid, my))
		return;
	if (IsMaxUidInServerContactBook(uid))
		return;
	debugLogA("Max: drop chat-only contact uid=%s (not in server address book)", uid.get());
	RemoveMaxUserContact(uid);
}

void CMaxProto::ClearMaxDialogLocalHistory(MCONTACT hContact)
{
	if (hContact == 0)
		return;

	// Do not call MS_HISTORY_EMPTY: with PS_CAN_EMPTY_HISTORY implemented it would also run
	// PS_EMPTY_SRV_HISTORY (server delete) on forced empty. Mirror core empty-history DB cleanup only.
	DB::ECPTR pCursor(DB::Events(hContact));
	while (pCursor.FetchNext())
		pCursor.DeleteEvent();

	if (isChatRoom(hContact)) {
		if (SESSION_INFO *si = Chat_Find(hContact, m_szModuleName))
			Chat_EmptyHistory(si);
		if (const char *szProto = Proto_GetBaseAccountName(hContact))
			db_unset(hContact, szProto, "ApparentMode");
	}

	delSetting(hContact, DB_KEY_MAX_CHATID);
}

void CMaxProto::ResetServerContactBookCache()
{
	mir_cslock lck(m_csContactBook);
	m_contactBookUids.clear();
	m_bContactBookSnapshotApplied = false;
}

void CMaxProto::ApplyServerContactBookSnapshot(const std::vector<CMStringA> &uids)
{
	mir_cslock lck(m_csContactBook);
	m_contactBookUids = uids;
	m_bContactBookSnapshotApplied = true;
}

bool CMaxProto::IsMaxUidInServerContactBook(const char *szUid)
{
	if (szUid == nullptr || szUid[0] == 0)
		return false;
	mir_cslock lck(m_csContactBook);
	for (const auto &u : m_contactBookUids)
		if (u == szUid)
			return true;
	return false;
}

MCONTACT CMaxProto::EnsureUserContact(const char *szUid, const wchar_t *wszFirst, const wchar_t *wszLast, const char *szDialogChatId)
{
	if (szUid == nullptr || szUid[0] == 0)
		return 0;

	MCONTACT hContact = FindContactByMaxUid(szUid);
	if (!hContact) {
		hContact = db_add_contact();
		Proto_AddToContact(hContact, m_szModuleName);
		setString(hContact, DB_KEY_MAX_UID, szUid);
		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 1);
		Contact::PutOnList(hContact);
		setByte(hContact, DB_KEY_MAX_PEER_ORIGIN, MAX_PEER_ORIGIN_CHATONLY);
	}

	if (wszFirst != nullptr) {
		if (wszFirst[0])
			setWString(hContact, "FirstName", wszFirst);
		else
			delSetting(hContact, "FirstName");
	}
	if (wszLast != nullptr) {
		if (wszLast[0])
			setWString(hContact, "LastName", wszLast);
		else
			delSetting(hContact, "LastName");
	}

	if (szDialogChatId != nullptr && szDialogChatId[0])
		setString(hContact, DB_KEY_MAX_CHATID, szDialogChatId);

	// Favorites/self contact must mirror protocol state.
	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	if (myUid != nullptr && myUid[0] != 0 && !mir_strcmp(myUid, szUid)) {
		setWord(hContact, "Status", (GetStatus() == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
		delSetting(hContact, "StatusMsg");
	}

	Clist_SetGroup(hContact, GetDefaultGroupW());
	return hContact;
}

void CMaxProto::MergeContactJson(const JSONNode &c, const char *szRequestedUid, bool bMarkAsContactsRoster)
{
	CMStringA uid;
	if (szRequestedUid != nullptr && szRequestedUid[0])
		uid = szRequestedUid;
	else {
		uid = sttJsonIdStr(c["id"]);
		if (uid.IsEmpty())
			uid = sttJsonIdStr(c["contactId"]);
		if (uid.IsEmpty())
			uid = sttJsonIdStr(c["userId"]);
	}
	if (uid.IsEmpty())
		return;

	if (sttContactPayloadSaysRemoved(c)) {
		{
			mir_cslock lck(m_csContactBook);
			for (auto it = m_contactBookUids.begin(); it != m_contactBookUids.end(); ) {
				if (*it == uid)
					it = m_contactBookUids.erase(it);
				else
					++it;
			}
		}
		MCONTACT hRm = FindContactByMaxUid(uid.c_str());
		bool hasServerDialog = false;
		if (hRm) {
			ptrA cid(getStringA(hRm, DB_KEY_MAX_CHATID));
			// Only persisted chat id counts: XOR is always derivable from MaxUid and would block (5) after (4) cleared MaxChatId.
			hasServerDialog = (cid != nullptr && cid[0]);
		}
		if (hasServerDialog)
			return;
		RemoveMaxUserContact(uid.c_str());
		return;
	}

	MCONTACT hPrev = FindContactByMaxUid(uid);

	CMStringW fromFn, fromLn;
	sttFillNamePartsFromContact(c, fromFn, fromLn);
	bool fromApiEmpty = fromFn.IsEmpty() && fromLn.IsEmpty();

	CMStringW oldFn, oldLn;
	if (hPrev) {
		oldFn = getMStringW(hPrev, "FirstName");
		oldLn = getMStringW(hPrev, "LastName");
	}
	bool oldIsUserStub = sttIsUserStubDisplay(oldFn);
	bool apiHasLocal = sttJsonHasLocalOrAliasContactName(c);
	bool oldHas = !oldFn.IsEmpty() || !oldLn.IsEmpty();

	CMStringW outFn, outLn;
	if (!fromApiEmpty) {
		if (hPrev && oldHas && !oldIsUserStub && !apiHasLocal) {
			outFn = oldFn;
			outLn = oldLn;
		}
		else {
			outFn = fromFn;
			outLn = fromLn;
		}
	}
	else if (oldHas) {
		outFn = oldFn;
		outLn = oldLn;
	}
	else {
		debugLogA("Max: contact name fallback to stub uid=%s (no first/last/name in payload)", uid.c_str());
		outFn.Format(L"User %S", uid.c_str());
		outLn.Empty();
	}

	EnsureUserContact(uid, outFn.IsEmpty() ? L"" : outFn.c_str(), outLn.IsEmpty() ? L"" : outLn.c_str(), nullptr);

	if (MCONTACT hAv = FindContactByMaxUid(uid)) {
		if (bMarkAsContactsRoster)
			setByte(hAv, DB_KEY_MAX_PEER_ORIGIN, MAX_PEER_ORIGIN_CONTACTS);
		const JSONNode &about = c["description"];
		if (about.type() == JSON_STRING && !about.as_string().empty()) {
			ptrW w(mir_utf8decodeW(about.as_string().c_str()));
			if (w != nullptr && w[0])
				setWString(hAv, "About", w);
			else
				delSetting(hAv, "About");
		}
		else
			delSetting(hAv, "About");

		ApplyPresenceToContact(hAv, c);
		SyncContactAvatarFromJson(hAv, c);
	}
}

static bool sttChatPayloadStatusEqualsCi(const JSONNode &chat, const char *ascii)
{
	const JSONNode &st = chat["status"];
	if (st.type() != JSON_STRING || ascii == nullptr || !ascii[0])
		return false;
	return !mir_strcmpi(st.as_string().c_str(), ascii);
}

void CMaxProto::OnMaxPushChatRemoved(const JSONNode &payload)
{
	const JSONNode &chat = payload["chat"];
	if (chat.type() != JSON_NODE)
		return;
	// Only "chat deleted" in official client — not ARCHIVED etc.
	if (!sttChatPayloadStatusEqualsCi(chat, "REMOVED"))
		return;

	CMStringA typ(chat["type"].type() != JSON_NULL ? chat["type"].as_string().c_str() : "");
	if (!typ.IsEmpty() && mir_strcmpi(typ.c_str(), "DIALOG"))
		return;

	CMStringA myUid;
	{
		ptrA my(getStringA(DB_KEY_MY_MAX_ID));
		if (my != nullptr && my[0])
			myUid = my.get();
	}

	CMStringA chatId = sttResolveDialogChatId(chat);
	CMStringA peerUid = sttPickDialogPeerUid(this, chat, myUid);

	MCONTACT h = 0;
	if (!chatId.IsEmpty())
		h = FindContactByDialogChatId(chatId.c_str());
	if (!h && !peerUid.IsEmpty())
		h = FindContactByMaxUid(peerUid.c_str());
	if (h == 0) {
		debugLogA("Max: push chat REMOVED but no local contact chat=%s peer=%s", chatId.c_str(), peerUid.c_str());
		return;
	}

	CMStringA uidPeer(peerUid);
	if (uidPeer.IsEmpty())
		uidPeer = getMStringA(h, DB_KEY_MAX_UID);

	// Only MaxPeerOrigin=CONTACTS means "in address book" for this rule. Server contacts[] often
	// includes chat-only peers that the official app does not show as contacts — do not use m_contactBookUids here.
	const int peerOrigin = getByte(h, DB_KEY_MAX_PEER_ORIGIN);
	const bool treatAsAddressBook = (peerOrigin == MAX_PEER_ORIGIN_CONTACTS);

	if (!uidPeer.IsEmpty() && treatAsAddressBook) {
		ClearMaxDialogLocalHistory(h);
		debugLogA("Max: server removed chat; kept contact (contacts merge / Add) chat=%s peer=%s", chatId.c_str(), uidPeer.c_str());
	}
	else if (!uidPeer.IsEmpty())
		RemoveMaxUserContact(uidPeer.c_str());
	else {
		ClearMaxDialogLocalHistory(h);
		debugLogA("Max: server removed chat; cleared local dialog (no peer uid) chat=%s", chatId.c_str());
	}
}

void CMaxProto::EnsureGroupChatSession(const CMStringA &szChatId, const wchar_t *wszTitle)
{
	if (szChatId.IsEmpty() || wszTitle == nullptr)
		return;

	ptrW wId(mir_a2u(szChatId.c_str()));
	if (!wId || !wId[0])
		return;

	if (Chat_Find(wId, m_szModuleName))
		return;

	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wId, wszTitle, nullptr);
	if (!si)
		return;

	Chat_AddGroup(si, TranslateT("Members"));
	Chat_Control(si, SESSION_INITDONE);
	Chat_Control(si, SESSION_ONLINE);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMaxProto::ApiFetchContactsBatch(WebSocket<CMaxProto> *ws, const CMStringA *pUids, size_t nUids, bool bMarkAsContactsRoster)
{
	if (!ws || !pUids || nUids == 0)
		return true;

	JSONNode ids(JSON_ARRAY);
	for (size_t i = 0; i < nUids; i++) {
		int64_t v = _strtoi64(pUids[i].c_str(), nullptr, 10);
		ids << INT64_PARAM("", v);
	}

	JSONNode payload(JSON_NODE);
	payload << JSON_PARAM("contactIds", ids);
	if (!SendJsonAndWait(ws, 32, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return false;

	const JSONNode &pl = sttResolveContactsFetchPayload(resp["payload"]);
	const JSONNode &ar = pl["contacts"];
	if (ar.type() == JSON_ARRAY) {
		for (unsigned j = 0; j < ar.size(); j++) {
			const char *req = (j < nUids) ? pUids[j].c_str() : nullptr;
			MergeContactJson(ar[j], req, bMarkAsContactsRoster);
		}
		return true;
	}

	if (ar.type() == JSON_NODE) {
		for (auto it = ar.begin(); it != ar.end(); ++it) {
			const char *key = (*it).name();
			MergeContactJson(*it, (key && key[0]) ? key : nullptr, bMarkAsContactsRoster);
		}
		return true;
	}

	const JSONNode &one = pl["contact"];
	if (one.type() == JSON_NODE) {
		const char *req = (nUids >= 1) ? pUids[0].c_str() : nullptr;
		MergeContactJson(one, req, bMarkAsContactsRoster);
	}
	return true;
}

bool CMaxProto::ApiFetchChatsByIds(WebSocket<CMaxProto> *ws, const CMStringA *pChatIds, size_t nIds)
{
	if (!ws || !pChatIds || nIds == 0)
		return true;

	JSONNode ids(JSON_ARRAY);
	for (size_t i = 0; i < nIds; i++) {
		int64_t v = _strtoi64(pChatIds[i].c_str(), nullptr, 10);
		ids << INT64_PARAM("", v);
	}

	JSONNode payload(JSON_NODE);
	payload << JSON_PARAM("chatIds", ids);
	if (!SendJsonAndWait(ws, 48, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return false;

	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	CMStringA myUid;
	if (my != nullptr && my[0])
		myUid = my.get();

	const JSONNode &pl = resp["payload"];
	const JSONNode &chats = pl["chats"];
	if (chats.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < chats.size(); i++) {
			const JSONNode &chat = chats[i];
			CMStringA chatId = sttResolveDialogChatId(chat);

			const JSONNode &ptOrig = chat["participants"];
			unsigned partCount = 0;
			if (ptOrig.type() == JSON_NODE) {
				for (auto it = ptOrig.begin(); it != ptOrig.end(); ++it)
					partCount++;
			}
			else if (ptOrig.type() == JSON_ARRAY)
				partCount = (unsigned)ptOrig.size();

			CMStringA typ(chat["type"].type() != JSON_NULL ? chat["type"].as_string().c_str() : "");
			bool treatAsDialog = sttTypeIsDialog(typ);
			if (typ.IsEmpty() && partCount > 2)
				treatAsDialog = false;

			sttMergeOneDialogChat(this, chat, myUid, nullptr, nullptr, nullptr, nullptr);
			if (treatAsDialog && !chatId.IsEmpty()) {
				MCONTACT hPeer = FindContactByDialogChatId(chatId.c_str());
				uint64_t lastMs = GetLastLocalMessageTimeMs(hPeer);
				if (hPeer && lastMs > 0)
					ApiFetchChatMessages(ws, chatId.c_str(), (int64_t)lastMs, 50, 0);
			}
		}
	}
	return true;
}

void CMaxProto::TryMergeContactsFromPayload(const JSONNode &payload)
{
	const JSONNode &p = sttResolveContactsFetchPayload(payload);
	const JSONNode &ca = p["contacts"];
	if (ca.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < ca.size(); i++)
			MergeContactJson(ca[i]);
	}
	else if (ca.type() == JSON_NODE) {
		for (auto it = ca.begin(); it != ca.end(); ++it) {
			const char *key = (*it).name();
			MergeContactJson(*it, (key && key[0]) ? key : nullptr);
		}
	}
	// Always merge top-level contact (e.g. opcode-131 REMOVED) even if contacts[] is empty or present.
	const JSONNode &one = p["contact"];
	if (one.type() == JSON_NODE)
		MergeContactJson(one);
}

void CMaxProto::TryApplySyncPayloadFromPush(const JSONNode &payload)
{
	const JSONNode &p = sttResolveSyncPayload(payload);
	if (p["chats"].type() != JSON_ARRAY && p["contacts"].type() != JSON_ARRAY && p["contacts"].type() != JSON_NODE
	    && p["profile"].type() == JSON_NULL)
		return;
	ApplySyncPayload(payload, nullptr);
}

// Shared by opcode-19 sync and opcode-48 chat refresh.
static void sttMergeOneDialogChat(CMaxProto *p, const JSONNode &chat, const CMStringA &myUid, const JSONNode *pAllContacts,
	OBJLIST<CMStringA> *pNeedFetch, CMStringA *pOutPeerUid, std::vector<CMStringA> *pAllowedUids)
{
	CMStringA chatId = sttResolveDialogChatId(chat);
	if (chatId.IsEmpty())
		return;

	const JSONNode &ptOrig = chat["participants"];
	unsigned partCount = 0;
	bool onlySelfParticipant = false;
	if (ptOrig.type() == JSON_NODE) {
		for (auto it = ptOrig.begin(); it != ptOrig.end(); ++it)
			partCount++;
		if (partCount == 1 && !myUid.IsEmpty()) {
			for (auto it = ptOrig.begin(); it != ptOrig.end(); ++it) {
				if (!mir_strcmp((*it).name(), myUid.c_str())) {
					onlySelfParticipant = true;
					break;
				}
			}
		}
	}
	else if (ptOrig.type() == JSON_ARRAY) {
		partCount = (unsigned)ptOrig.size();
		if (partCount == 1 && !myUid.IsEmpty()) {
			CMStringA pid = sttJsonIdStr(ptOrig[(json_index_t)0]["id"]);
			if (pid.IsEmpty())
				pid = sttJsonIdStr(ptOrig[(json_index_t)0]["userId"]);
			onlySelfParticipant = !pid.IsEmpty() && !mir_strcmp(pid.c_str(), myUid.c_str());
		}
	}

	CMStringA typ(chat["type"].type() != JSON_NULL ? chat["type"].as_string().c_str() : "");
	bool treatAsDialog = sttTypeIsDialog(typ);
	if (typ.IsEmpty() && partCount > 2)
		treatAsDialog = false;

	if (!treatAsDialog)
		return;

	if (sttChatExcludedFromRoster(chat)) {
		CMStringA peerRm = sttPickDialogPeerUid(p, chat, myUid);
		if (!peerRm.IsEmpty()) {
			CMStringA myEffRm = myUid;
			if (myEffRm.IsEmpty()) {
				ptrA db(p->getStringA(DB_KEY_MY_MAX_ID));
				if (db != nullptr && db[0])
					myEffRm = db.get();
			}
			if (!myEffRm.IsEmpty() && peerRm == myEffRm)
				return;
			MCONTACT hEx = p->FindContactByDialogChatId(chatId.c_str());
			if (!hEx)
				hEx = p->FindContactByMaxUid(peerRm.c_str());
			const int origin = hEx ? p->getByte(hEx, DB_KEY_MAX_PEER_ORIGIN) : MAX_PEER_ORIGIN_UNKNOWN;
			const bool treatAsAddressBook = (origin == MAX_PEER_ORIGIN_CONTACTS);
			if (treatAsAddressBook) {
				if (hEx)
					p->ClearMaxDialogLocalHistory(hEx);
				if (pAllowedUids != nullptr)
					sttPushUniqueUid(*pAllowedUids, peerRm);
			}
			else
				p->RemoveMaxUserContact(peerRm.c_str());
		}
		return;
	}

	CMStringA peerUid = sttPickDialogPeerUid(p, chat, myUid);
	if (peerUid.IsEmpty()) {
		// Self-only dialog ("Favorites"): no peer uid to resolve, keep it as own contact.
		if (onlySelfParticipant && !myUid.IsEmpty()) {
			CMStringW fn;
			if (chat["title"].type() != JSON_NULL) {
				ptrW w(mir_utf8decodeW(chat["title"].as_string().c_str()));
				if (w && w[0])
					fn = w;
			}
			if (fn.IsEmpty())
				fn = TranslateT("Favorites");
		p->EnsureUserContact(myUid.c_str(), fn.c_str(), L"", "0");
		p->debugLogA("Max: dlg self-only mapped as favorites chat=%s", "0");
		}
		return;
	}

	CMStringA myEff = myUid;
	if (myEff.IsEmpty()) {
		ptrA db(p->getStringA(DB_KEY_MY_MAX_ID));
		if (db != nullptr && db[0])
			myEff = db.get();
	}
	if (!myEff.IsEmpty() && peerUid == myEff) {
		// Self dialog ("Favorites"): keep it as a visible contact, same as official client.
		CMStringW fn;
		if (chat["title"].type() != JSON_NULL) {
			ptrW w(mir_utf8decodeW(chat["title"].as_string().c_str()));
			if (w && w[0])
				fn = w;
		}
		if (fn.IsEmpty())
			fn = TranslateT("Favorites");

		p->EnsureUserContact(myEff.c_str(), fn.c_str(), L"", chatId.c_str());
		p->debugLogA("Max: dlg self-peer mapped as favorites chat=%s", chatId.c_str());
		return;
	}

	p->debugLogA("Max: dlg peer=%s chat=%s my=%s", peerUid.c_str(), chatId.c_str(), myEff.c_str());

	if (pOutPeerUid != nullptr)
		*pOutPeerUid = peerUid;

	MCONTACT hPeer = p->FindContactByMaxUid(peerUid);
	if (sttContactNeedsPlaceholderName(p, hPeer)) {
		CMStringW fn, ln;
		ln.Empty();
		if (chat["title"].type() != JSON_NULL) {
			ptrW w(mir_utf8decodeW(chat["title"].as_string().c_str()));
			if (w && w[0])
				fn = w;
		}
		if (fn.IsEmpty())
			fn.Format(L"User %S", peerUid.c_str());
		p->EnsureUserContact(peerUid, fn.c_str(), L"", chatId);
	}
	else
		p->EnsureUserContact(peerUid, nullptr, nullptr, chatId);

	if (pAllowedUids != nullptr)
		sttPushUniqueUid(*pAllowedUids, peerUid);

	// Do not ingest chat["lastMessage"] here: new accounts should stay empty until live pushes (opcode 128);
	// gap fill uses opcode 49 only when GetLastLocalMessageTimeMs > 0.

	if (pNeedFetch != nullptr) {
		bool need = true;
		if (pAllContacts != nullptr && pAllContacts->type() == JSON_ARRAY && sttUidInContactsArray(*pAllContacts, peerUid))
			need = false;
		if (need) {
			bool found = false;
			for (int ni = 0; ni < pNeedFetch->getCount(); ni++)
				if ((*pNeedFetch)[ni] == peerUid) {
					found = true;
					break;
				}
			if (!found)
				pNeedFetch->insert(new CMStringA(peerUid));
		}
	}
}

void CMaxProto::SyncLiveDialogFromPushPayload(const JSONNode &payload)
{
	const JSONNode &chat = payload["chat"];
	if (chat.type() != JSON_NODE)
		return;

	CMStringA myUid;
	{
		ptrA my(getStringA(DB_KEY_MY_MAX_ID));
		if (my != nullptr && my[0])
			myUid = my.get();
	}

	sttMergeOneDialogChat(this, chat, myUid, nullptr, nullptr, nullptr, nullptr);
}

void CMaxProto::ApplySyncPayload(const JSONNode &payload, WebSocket<CMaxProto> *ws)
{
	const JSONNode &p = sttResolveSyncPayload(payload);
	const JSONNode &contacts = p["contacts"];
	const JSONNode &chats = p["chats"];
	const bool hadContactsInPayload = (contacts.type() == JSON_ARRAY || contacts.type() == JSON_NODE);
	size_t nc = 0;
	if (contacts.type() == JSON_ARRAY)
		nc = contacts.size();
	else if (contacts.type() == JSON_NODE) {
		for (auto it = contacts.begin(); it != contacts.end(); ++it)
			nc++;
	}
	size_t nch = (chats.type() == JSON_ARRAY) ? chats.size() : 0;
	debugLogA("Max: ApplySync contacts=%zu chats=%zu", nc, nch);

	std::vector<CMStringA> allowedUids;
	std::vector<CMStringA> contactBookFromSync;

	if (ws != nullptr)
		ResetServerContactBookCache();

	CMStringA myUid;
	const JSONNode &prof = p["profile"];
	if (prof.type() != JSON_NULL) {
		const JSONNode &ct = prof["contact"];
		if (ct.type() != JSON_NULL) {
			myUid = sttJsonIdStr(ct["id"]);
			if (!myUid.IsEmpty())
				setString(DB_KEY_MY_MAX_ID, myUid.c_str());

			// Keep own profile name in account module settings (Max_1) so UI can show sender name.
			CMStringW myFn, myLn;
			sttFillNamePartsFromContact(ct, myFn, myLn);
			if (!myFn.IsEmpty() || !myLn.IsEmpty()) {
				setWString("FirstName", myFn.c_str());
				setWString("LastName", myLn.c_str());
			}
			else {
				delSetting("FirstName");
				delSetting("LastName");
			}
			const JSONNode &myAbout = ct["description"];
			if (myAbout.type() == JSON_STRING && !myAbout.as_string().empty()) {
				ptrW w(mir_utf8decodeW(myAbout.as_string().c_str()));
				if (w != nullptr && w[0])
					setWString("About", w);
				else
					delSetting("About");
			}
			else
				delSetting("About");

			SyncContactAvatarFromJson(0, ct);
		}
	}

	if (contacts.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < contacts.size(); i++) {
			const JSONNode &c = contacts[i];
			CMStringA cuid = sttJsonIdStr(c["id"]);
			if (cuid.IsEmpty())
				cuid = sttJsonIdStr(c["contactId"]);
			if (cuid.IsEmpty())
				cuid = sttJsonIdStr(c["userId"]);
			if (cuid.IsEmpty())
				continue;
			if (sttContactPayloadSaysRemoved(c)) {
				RemoveMaxUserContact(cuid.c_str());
				continue;
			}
			sttPushUniqueUid(allowedUids, cuid);
			sttPushUniqueUid(contactBookFromSync, cuid);
			MergeContactJson(contacts[i]);
		}
	}
	else if (contacts.type() == JSON_NODE) {
		for (auto it = contacts.begin(); it != contacts.end(); ++it) {
			const JSONNode &c = *it;
			const char *key = (*it).name();
			CMStringA cuid;
			if (key != nullptr && key[0])
				cuid = key;
			if (cuid.IsEmpty())
				cuid = sttJsonIdStr(c["id"]);
			if (cuid.IsEmpty())
				cuid = sttJsonIdStr(c["contactId"]);
			if (cuid.IsEmpty())
				cuid = sttJsonIdStr(c["userId"]);
			if (cuid.IsEmpty())
				continue;
			if (sttContactPayloadSaysRemoved(c)) {
				RemoveMaxUserContact(cuid.c_str());
				continue;
			}
			sttPushUniqueUid(allowedUids, cuid);
			sttPushUniqueUid(contactBookFromSync, cuid);
			MergeContactJson(c, (key && key[0]) ? key : nullptr);
		}
	}

	if (chats.type() == JSON_ARRAY)
		sttAugmentContactBookFromBotDialogs(this, chats, myUid, contactBookFromSync, allowedUids);

	if (hadContactsInPayload || !contactBookFromSync.empty())
		ApplyServerContactBookSnapshot(contactBookFromSync);

	OBJLIST<CMStringA> needFetch(1);
	OBJLIST<CMStringA> needChatTitles(1);

	if (chats.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < chats.size(); i++) {
			const JSONNode &chat = chats[i];
			CMStringA chatId = sttResolveDialogChatId(chat);
			if (chatId.IsEmpty())
				continue;

			const JSONNode &ptOrig = chat["participants"];
			unsigned partCount = 0;
			if (ptOrig.type() == JSON_NODE) {
				for (auto it = ptOrig.begin(); it != ptOrig.end(); ++it)
					partCount++;
			}
			else if (ptOrig.type() == JSON_ARRAY)
				partCount = (unsigned)ptOrig.size();

			CMStringA typ(chat["type"].type() != JSON_NULL ? chat["type"].as_string().c_str() : "");
			bool treatAsDialog = sttTypeIsDialog(typ);
			if (typ.IsEmpty() && partCount > 2)
				treatAsDialog = false;

			if (treatAsDialog) {
				sttMergeOneDialogChat(this, chat, myUid, &contacts, &needFetch, nullptr,
					ws != nullptr ? &allowedUids : nullptr);
				if (ws != nullptr) {
					MCONTACT hPeer = FindContactByDialogChatId(chatId.c_str());
					uint64_t lastMs = GetLastLocalMessageTimeMs(hPeer);
					if (hPeer && lastMs > 0)
						ApiFetchChatMessages(ws, chatId.c_str(), (int64_t)lastMs, 50, 0);
				}
			}
			else
				EnsureGroupChatSession(chatId, sttGroupTitle(chat));
		}
	}

	while (needFetch.getCount() != 0 && ws != nullptr) {
		// Server may send one contact per WebSocket frame; batched opcode 32 would lose follow-up frames on SendJsonAndWait.
		size_t batchSz = 1;
		std::vector<CMStringA> batch;
		batch.reserve(batchSz);
		for (size_t k = 0; k < batchSz; k++) {
			batch.push_back(needFetch[0]);
			needFetch.remove(0);
		}
		if (!ApiFetchContactsBatch(ws, batch.data(), batch.size()))
			break;
	}

	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact))
			continue;
		ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
		if (uid == nullptr || uid[0] == 0)
			continue;
		CMStringW fn = getMStringW(hContact, "FirstName");
		CMStringW ln = getMStringW(hContact, "LastName");
		bool needTitle = sttIsUserStubDisplay(fn);
		if (!needTitle && fn.IsEmpty() && ln.IsEmpty())
			needTitle = true;
		if (!needTitle)
			continue;
		ptrA cid(getStringA(hContact, DB_KEY_MAX_CHATID));
		if (cid == nullptr || cid[0] == 0)
			continue;
		bool dup = false;
		for (int ti = 0; ti < needChatTitles.getCount(); ti++)
			if (needChatTitles[ti] == cid) {
				dup = true;
				break;
			}
		if (!dup)
			needChatTitles.insert(new CMStringA(cid));
	}

	while (needChatTitles.getCount() != 0 && ws != nullptr) {
		CMStringA cid = needChatTitles[0];
		needChatTitles.remove(0);
		CMStringA one[1] = { cid };
		if (!ApiFetchChatsByIds(ws, one, 1))
			break;
	}

	// Full connect sync only: drop local 1:1 contacts the server no longer lists in contacts + active dialogs.
	if (ws != nullptr && !allowedUids.empty()) {
		ptrA myAcc(getStringA(DB_KEY_MY_MAX_ID));
		std::vector<MCONTACT> toDelete;
		for (auto &hContact : AccContacts()) {
			if (isChatRoom(hContact))
				continue;
			ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
			if (uid == nullptr || uid[0] == 0)
				continue;
			if (myAcc != nullptr && myAcc[0] && !mir_strcmp(uid, myAcc))
				continue;
			bool keep = false;
			for (const auto &a : allowedUids)
				if (a == uid) {
					keep = true;
					break;
				}
			if (!keep)
				toDelete.push_back(hContact);
		}
		for (MCONTACT hDel : toDelete) {
			ptrA uid(getStringA(hDel, DB_KEY_MAX_UID));
			debugLogA("Max: prune contact uid=%s (not in sync contacts + dialog peer set)", uid.get());
			db_delete_contact(hDel, 0);
		}
	}

	if (nc == 0 && nch == 0 && prof.type() == JSON_NULL && payload.type() == JSON_NODE) {
		CMStringA keys;
		for (auto it = payload.begin(); it != payload.end(); ++it) {
			const char *nm = (*it).name();
			if (nm != nullptr && nm[0]) {
				if (!keys.IsEmpty())
					keys += ",";
				keys += nm;
			}
		}
		debugLogA("Max: ApplySync still empty; raw payload top-level keys: %s", keys.c_str());
	}

	debugLogA("Max: roster sync applied (group chats + contacts)");
}
