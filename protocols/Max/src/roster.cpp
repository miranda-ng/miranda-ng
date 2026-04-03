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

static void sttMergeOneDialogChat(CMaxProto *p, const JSONNode &chat, const CMStringA &myUid, const JSONNode *pAllContacts,
	OBJLIST<CMStringA> *pNeedFetch, CMStringA *pOutPeerUid);

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

	uint64_t idv = id.IsEmpty() ? 0 : _strtoui64(id.c_str(), nullptr, 10);
	uint64_t cidv = cid.IsEmpty() ? 0 : _strtoui64(cid.c_str(), nullptr, 10);
	uint64_t chv = ch.IsEmpty() ? 0 : _strtoui64(ch.c_str(), nullptr, 10);

	if (!cid.IsEmpty() && idv != 0 && idv < 10000 && cidv >= 10000)
		return cid;
	if (!ch.IsEmpty() && idv != 0 && idv < 10000 && chv >= 10000)
		return ch;

	if (!id.IsEmpty())
		return id;
	if (!cid.IsEmpty())
		return cid;
	return ch;
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

// One element of contact.names — either a JSON object or the legacy single "names" object.
static CMStringW sttFormatOneNameRecord(const JSONNode &rec)
{
	if (rec.type() != JSON_NODE)
		return L"";

	if (rec["name"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(rec["name"].as_string().c_str()));
		if (w && w[0])
			return CMStringW(w);
	}

	CMStringW fn, ln;
	if (rec["firstName"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(rec["firstName"].as_string().c_str()));
		if (w && w[0])
			fn = w;
	}
	if (rec["lastName"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(rec["lastName"].as_string().c_str()));
		if (w && w[0])
			ln = w;
	}
	if (!fn.IsEmpty() || !ln.IsEmpty()) {
		CMStringW r = fn;
		if (!ln.IsEmpty()) {
			if (!r.IsEmpty())
				r.AppendChar(L' ');
			r += ln;
		}
		return r;
	}

	static const char *nickKeys[] = { "nick", "nickname", "nickName", "displayName", "login", nullptr };
	for (int k = 0; nickKeys[k]; k++) {
		const JSONNode &nn = rec[nickKeys[k]];
		if (nn.type() != JSON_NULL) {
			ptrW w(mir_utf8decodeW(nn.as_string().c_str()));
			if (w && w[0])
				return CMStringW(w);
		}
	}
	return L"";
}

static CMStringW sttFormatNamesDepth(const JSONNode &contact, int depth)
{
	if (depth > 4)
		return L"";

	// User-set alias in the official client (address book) beats profile/ONEME name.
	static const char *aliasKeys[] = { "alias", "localName", "localAlias", "addressBookName", "phoneBookName", "bookName", "customName", "remark", "remarkName", nullptr };
	for (int k = 0; aliasKeys[k]; k++) {
		const JSONNode &n = contact[aliasKeys[k]];
		if (n.type() != JSON_NULL) {
			ptrW w(mir_utf8decodeW(n.as_string().c_str()));
			if (w && w[0])
				return CMStringW(w);
		}
	}

	const JSONNode &nm = contact["names"];
	if (nm.type() == JSON_ARRAY) {
		auto pickNameByTypes = [&](const char **typesList) -> CMStringW {
			for (unsigned i = 0; i < nm.size(); i++) {
				const JSONNode &el = nm[i];
				if (el.type() != JSON_NODE)
					continue;
				CMStringA typ(el["type"].type() != JSON_NULL ? el["type"].as_string().c_str() : "");
				bool match = false;
				for (const char **w = typesList; *w && !match; w++)
					if (!mir_strcmpi(typ.c_str(), *w))
						match = true;
				if (!match)
					continue;
				CMStringW r = sttFormatOneNameRecord(el);
				if (!r.IsEmpty())
					return r;
			}
			return L"";
		};

		static const char *localTypes[] = { "LOCAL", "TT_LOCAL", "CUSTOM", "ALIAS", "CONTACT", "ADDRESSBOOK", "BOOK", "NICKNAME", "PHONEBOOK", nullptr };
		static const char *profileTypes[] = { "ONEME", "TT", "MAX", "DEFAULT", "PROFILE", "MAIN", nullptr };

		CMStringW r = pickNameByTypes(localTypes);
		if (!r.IsEmpty())
			return r;
		r = pickNameByTypes(profileTypes);
		if (!r.IsEmpty())
			return r;
		for (unsigned i = 0; i < nm.size(); i++) {
			CMStringW r2 = sttFormatOneNameRecord(nm[i]);
			if (!r2.IsEmpty())
				return r2;
		}
	}
	else if (nm.type() == JSON_NODE) {
		CMStringW r = sttFormatOneNameRecord(nm);
		if (!r.IsEmpty())
			return r;
	}

	const JSONNode &bc = contact["baseContact"];
	if (bc.type() == JSON_NODE) {
		CMStringW r = sttFormatNamesDepth(bc, depth + 1);
		if (!r.IsEmpty())
			return r;
	}

	static const char *nestKeys[] = { "user", "profile", "contact", "person", nullptr };
	for (int k = 0; nestKeys[k]; k++) {
		const JSONNode &nn = contact[nestKeys[k]];
		if (nn.type() == JSON_NODE) {
			CMStringW r = sttFormatNamesDepth(nn, depth + 1);
			if (!r.IsEmpty())
				return r;
		}
	}

	static const char *topKeys[] = { "displayName", "name", "nick", "nickname", "login", nullptr };
	for (int k = 0; topKeys[k]; k++) {
		const JSONNode &n = contact[topKeys[k]];
		if (n.type() != JSON_NULL) {
			ptrW w(mir_utf8decodeW(n.as_string().c_str()));
			if (w && w[0])
				return CMStringW(w);
		}
	}
	return L"";
}

static CMStringW sttFormatNames(const JSONNode &contact)
{
	return sttFormatNamesDepth(contact, 0);
}

// True if JSON carries a user-defined / address-book style name (not only profile ONEME).
static bool sttJsonHasLocalOrAliasContactName(const JSONNode &contact)
{
	static const char *aliasKeys[] = { "alias", "localName", "localAlias", "addressBookName", "phoneBookName", "bookName", "customName", "remark", "remarkName", nullptr };
	for (int k = 0; aliasKeys[k]; k++) {
		const JSONNode &n = contact[aliasKeys[k]];
		if (n.type() != JSON_NULL && !n.as_string().empty())
			return true;
	}

	const JSONNode &nm = contact["names"];
	if (nm.type() != JSON_ARRAY)
		return false;

	static const char *localTypes[] = { "LOCAL", "TT_LOCAL", "CUSTOM", "ALIAS", "CONTACT", "ADDRESSBOOK", "BOOK", "NICKNAME", "PHONEBOOK", nullptr };
	for (unsigned i = 0; i < nm.size(); i++) {
		const JSONNode &el = nm[i];
		if (el.type() != JSON_NODE)
			continue;
		CMStringA typ(el["type"].type() != JSON_NULL ? el["type"].as_string().c_str() : "");
		for (const char **w = localTypes; *w; w++) {
			if (!mir_strcmpi(typ.c_str(), *w)) {
				if (!sttFormatOneNameRecord(el).IsEmpty())
					return true;
				break;
			}
		}
	}
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

MCONTACT CMaxProto::EnsureUserContact(const char *szUid, const wchar_t *wszNick, const char *szDialogChatId)
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
	}

	if (wszNick != nullptr && wszNick[0])
		setWString(hContact, "Nick", wszNick);

	if (szDialogChatId != nullptr && szDialogChatId[0])
		setString(hContact, DB_KEY_MAX_CHATID, szDialogChatId);

	Clist_SetGroup(hContact, GetDefaultGroupW());
	return hContact;
}

void CMaxProto::MergeContactJson(const JSONNode &c, const char *szRequestedUid)
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

	CMStringW fromApi = sttFormatNames(c);

	MCONTACT hPrev = FindContactByMaxUid(uid);
	CMStringW oldNick;
	if (hPrev)
		oldNick = getMStringW(hPrev, "Nick");

	bool oldIsUserStub = (!oldNick.IsEmpty() && oldNick.GetLength() >= 5 && !_wcsnicmp(oldNick.c_str(), L"User ", 5));
	bool apiHasLocal = sttJsonHasLocalOrAliasContactName(c);

	CMStringW nick;
	if (!fromApi.IsEmpty()) {
		if (hPrev && !oldNick.IsEmpty() && !oldIsUserStub && !apiHasLocal)
			nick = oldNick;
		else
			nick = fromApi;
	}
	else if (hPrev && !oldNick.IsEmpty())
		nick = oldNick;
	else
		nick.Format(L"User %S", uid.c_str());

	EnsureUserContact(uid, nick, nullptr);
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

bool CMaxProto::ApiFetchContactsBatch(WebSocket<CMaxProto> *ws, const CMStringA *pUids, size_t nUids)
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
			MergeContactJson(ar[j], req);
		}
		return true;
	}

	if (ar.type() == JSON_NODE) {
		for (auto it = ar.begin(); it != ar.end(); ++it) {
			const char *key = (*it).name();
			MergeContactJson(*it, (key && key[0]) ? key : nullptr);
		}
		return true;
	}

	const JSONNode &one = pl["contact"];
	if (one.type() == JSON_NODE) {
		const char *req = (nUids >= 1) ? pUids[0].c_str() : nullptr;
		MergeContactJson(one, req);
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
		for (unsigned i = 0; i < chats.size(); i++)
			sttMergeOneDialogChat(this, chats[i], myUid, nullptr, nullptr, nullptr);
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
		return;
	}
	if (ca.type() == JSON_NODE) {
		for (auto it = ca.begin(); it != ca.end(); ++it) {
			const char *key = (*it).name();
			MergeContactJson(*it, (key && key[0]) ? key : nullptr);
		}
		return;
	}
	const JSONNode &one = p["contact"];
	if (one.type() == JSON_NODE)
		MergeContactJson(one);
}

void CMaxProto::TryApplySyncPayloadFromPush(const JSONNode &payload)
{
	const JSONNode &p = sttResolveSyncPayload(payload);
	if (p["chats"].type() != JSON_ARRAY && p["contacts"].type() != JSON_ARRAY && p["profile"].type() == JSON_NULL)
		return;
	ApplySyncPayload(payload, nullptr);
}

static bool sttNickIsUserStub(const CMStringW &nick)
{
	return !nick.IsEmpty() && nick.GetLength() >= 5 && !_wcsnicmp(nick.c_str(), L"User ", 5);
}

// Shared by opcode-19 sync and opcode-48 chat refresh.
static void sttMergeOneDialogChat(CMaxProto *p, const JSONNode &chat, const CMStringA &myUid, const JSONNode *pAllContacts,
	OBJLIST<CMStringA> *pNeedFetch, CMStringA *pOutPeerUid = nullptr)
{
	CMStringA chatId = sttResolveDialogChatId(chat);
	if (chatId.IsEmpty())
		return;

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
		return;

	CMStringA peerUid = sttPickDialogPeerUid(p, chat, myUid);
	if (peerUid.IsEmpty())
		return;

	CMStringA myEff = myUid;
	if (myEff.IsEmpty()) {
		ptrA db(p->getStringA(DB_KEY_MY_MAX_ID));
		if (db != nullptr && db[0])
			myEff = db.get();
	}
	if (!myEff.IsEmpty() && peerUid == myEff) {
		p->debugLogA("Max: dlg skip self-peer chat=%s", chatId.c_str());
		return;
	}

	p->debugLogA("Max: dlg peer=%s chat=%s my=%s", peerUid.c_str(), chatId.c_str(), myEff.c_str());

	if (pOutPeerUid != nullptr)
		*pOutPeerUid = peerUid;

	CMStringW nick;
	if (MCONTACT hPeer = p->FindContactByMaxUid(peerUid))
		nick = p->getMStringW(hPeer, "Nick");

	if (nick.IsEmpty() || sttNickIsUserStub(nick)) {
		if (chat["title"].type() != JSON_NULL) {
			ptrW w(mir_utf8decodeW(chat["title"].as_string().c_str()));
			if (w && w[0])
				nick = w;
		}
	}
	if (nick.IsEmpty())
		nick.Format(L"User %S", peerUid.c_str());

	p->EnsureUserContact(peerUid, nick, chatId);

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

void CMaxProto::ApplySyncPayload(const JSONNode &payload, WebSocket<CMaxProto> *ws)
{
	const JSONNode &p = sttResolveSyncPayload(payload);
	const JSONNode &contacts = p["contacts"];
	const JSONNode &chats = p["chats"];
	size_t nc = (contacts.type() == JSON_ARRAY) ? contacts.size() : 0;
	size_t nch = (chats.type() == JSON_ARRAY) ? chats.size() : 0;
	debugLogA("Max: ApplySync contacts=%zu chats=%zu", nc, nch);

	CMStringA myUid;
	const JSONNode &prof = p["profile"];
	if (prof.type() != JSON_NULL) {
		const JSONNode &ct = prof["contact"];
		if (ct.type() != JSON_NULL) {
			myUid = sttJsonIdStr(ct["id"]);
			if (!myUid.IsEmpty()) {
				setString(DB_KEY_MY_MAX_ID, myUid.c_str());
				setString(DB_KEY_MAX_UID, myUid.c_str());
			}
		}
	}

	if (contacts.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < contacts.size(); i++)
			MergeContactJson(contacts[i]);
	}

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

			if (treatAsDialog)
				sttMergeOneDialogChat(this, chat, myUid, &contacts, &needFetch, nullptr);
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
		// Old builds stored bogus low "MaxUid" / "MaxChatId" pairs — do not opcode-48 those.
		if (_strtoui64(uid, nullptr, 10) < 100000ull)
			continue;
		CMStringW nn = getMStringW(hContact, "Nick");
		if (!sttNickIsUserStub(nn))
			continue;
		ptrA cid(getStringA(hContact, DB_KEY_MAX_CHATID));
		if (cid == nullptr || cid[0] == 0)
			continue;
		if (_strtoui64(cid, nullptr, 10) < 100000ull)
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
