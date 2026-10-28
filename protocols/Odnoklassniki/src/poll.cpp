/*
Copyright (C) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// OK's REST API has no push and no long-poll, so the session is a loop: the conversation list
// (messagesV2/getList) is read every few seconds, and a conversation whose last message is
// newer than what the database holds gets its latest page read (getMessages, batched with
// getAttachedResources for the picture urls of photos).

#include "stdafx.h"

#define OK_ERROR_SESSION_EXPIRED 102
#define MAX_NETWORK_FAILURES 4
#define FRIENDS_REFRESH_POLLS 20

static int64_t GetInt64(const JSONNode &node)
{
	return _atoi64(node.as_string().c_str());
}

static CMStringA JsonEscape(const char *str)
{
	CMStringA ret;
	for (const char *p = str; *p; p++) {
		switch (*p) {
		case '"': ret += "\\\""; break;
		case '\\': ret += "\\\\"; break;
		default: ret.AppendChar(*p);
		}
	}
	return ret;
}

static int64_t GetLastTime(PROTO_INTERFACE *ppro, MCONTACT hContact)
{
	return _atoi64(ppro->getMStringA(hContact, DB_KEY_LASTTIME));
}

static void SetLastTime(PROTO_INTERFACE *ppro, MCONTACT hContact, int64_t value)
{
	char buf[30];
	_i64toa_s(value, buf, sizeof(buf), 10);
	ppro->setString(hContact, DB_KEY_LASTTIME, buf);
}

/////////////////////////////////////////////////////////////////////////////////////////
// login: by the saved auth token when there is one (the password is then not sent again),
// else by password with gen_token=true so that the next login can use the token

void COdnoklassnikiProto::ShowVerification(const CMStringA &szUrl)
{
	debugLogA("login needs a verification: %s", szUrl.c_str());
	CMStringW wszText(FORMAT, TranslateT("Odnoklassniki asks to confirm this login (captcha or a code) on its web page.\n\nOpen the page in the browser now? Go online again once it is confirmed.\n\n%S"), szUrl.c_str());
	if (IDYES == MessageBoxW(nullptr, wszText, m_tszUserName, MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST))
		Utils_OpenUrl(szUrl);
}

bool COdnoklassnikiProto::Login()
{
	OkReply reply;

	CMStringA szToken(getMStringA(DB_KEY_TOKEN));
	if (!szToken.IsEmpty()) {
		std::map<CMStringA, CMStringA> params;
		params["token"] = szToken;
		params["verification_supported_v"] = "1";
		reply = Call("api/auth/loginByToken", params, true);
		if (!reply && reply.code != -1) {
			debugLogA("token login refused, trying the password");
			delSetting(DB_KEY_TOKEN);
		}
	}

	if (szToken.IsEmpty() || (!reply && reply.code != -1)) {
		CMStringA szPassword(getMStringA("Password"));
		if (szPassword.IsEmpty()) {
			ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_WRONGPASSWORD);
			return false;
		}

		std::map<CMStringA, CMStringA> params;
		params["user_name"] = T2Utf(ptrW(m_wszLogin)).get();
		params["password"] = szPassword;
		params["gen_token"] = "true";
		params["verification_supported_v"] = "1";
		reply = Call("api/auth/login", params, true);
	}

	if (!reply) {
		ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, reply.code == -1 ? LOGINERR_NONETWORK : LOGINERR_WRONGPASSWORD);
		return false;
	}

	CMStringA szVerify(reply.root["verification_url"].as_string().c_str());
	if (!szVerify.IsEmpty()) {
		ShowVerification(szVerify);
		ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_WRONGPASSWORD);
		return false;
	}

	CMStringA szNewToken(reply.root["auth_token"].as_string().c_str());
	if (!szNewToken.IsEmpty())
		setString(DB_KEY_TOKEN, szNewToken);

	mir_cslock lck(m_csSession);
	m_szSessionKey = reply.root["session_key"].as_string().c_str();
	m_szSessionSecret = reply.root["session_secret_key"].as_string().c_str();
	CMStringA szServer(reply.root["api_server"].as_string().c_str());
	if (!szServer.IsEmpty())
		m_szApiServer = szServer;
	CMStringA szUid(reply.root["uid"].as_string().c_str());
	if (!szUid.IsEmpty())
		m_szMyUid = szUid;

	if (m_szSessionKey.IsEmpty() || m_szSessionSecret.IsEmpty()) {
		debugLogA("login returned no session");
		ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_WRONGPROTOCOL);
		return false;
	}

	debugLogA("logged in as %s, api server %s", m_szMyUid.c_str(), m_szApiServer.c_str());
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// the session thread

void COdnoklassnikiProto::ServerThread(void *)
{
	if (!Login()) {
		int iOldStatus = m_iStatus;
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		return;
	}

	m_onlineChats.clear();
	m_bOnline = true;
	int iOldStatus = m_iStatus;
	m_iStatus = ID_STATUS_ONLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);

	LoadFriends();
	PollConversations();

	int nPolls = 0;
	while (!m_bTerminated) {
		int interval = m_iPollInterval;
		WaitForSingleObject(m_hPollEvent, max(interval, 5) * 1000);
		if (m_bTerminated)
			break;

		if (++nPolls % FRIENDS_REFRESH_POLLS == 0)
			LoadFriends();
		PollConversations();
	}

	m_bOnline = false;
	Chat_Control(m_szModuleName, SESSION_OFFLINE);
	setAllContactStatuses(ID_STATUS_OFFLINE, false);

	iOldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	debugLogA("session finished");
}

/////////////////////////////////////////////////////////////////////////////////////////
// users: names (and presence) come from users/getInfo, a hundred at a time

void COdnoklassnikiProto::ResolveUsers(const std::vector<CMStringA> &uids)
{
	for (size_t i = 0; i < uids.size(); i += 100) {
		CMStringA szList;
		for (size_t k = i; k < uids.size() && k < i + 100; k++) {
			if (!szList.IsEmpty())
				szList.AppendChar(',');
			szList += uids[k];
		}

		std::map<CMStringA, CMStringA> params;
		params["uids"] = szList;
		params["fields"] = "uid,first_name,last_name,online";
		params["emptyPictures"] = "false";
		auto reply = Call("api/users/getInfo", params);
		if (!reply)
			continue;

		for (auto &it : reply.root["items"]) {
			CMStringA uid(it["uid"].as_string().c_str());
			if (uid.IsEmpty())
				continue;

			CMStringW wszFirst(it["first_name"].as_mstring()), wszLast(it["last_name"].as_mstring());
			CMStringW wszName(wszFirst);
			if (!wszLast.IsEmpty())
				wszName += L" " + wszLast;
			{
				mir_cslock lck(m_csNames);
				m_names[uid] = wszName;
			}

			MCONTACT hContact = FindUser(uid);
			if (hContact == 0)
				continue;

			setWString(hContact, "Nick", wszName);
			setWString(hContact, "FirstName", wszFirst);
			setWString(hContact, "LastName", wszLast);

			// "online" is absent or OFFLINE when away, else the kind of device (WEB, MOBILE...)
			CMStringA szOnline(it["online"].as_string().c_str());
			bool bOnline = !szOnline.IsEmpty() && szOnline != "OFFLINE" && szOnline != "false";
			setWord(hContact, "Status", bOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
		}
	}
}

// the friends become the contact list: that is where a new chat is started from
void COdnoklassnikiProto::LoadFriends()
{
	auto reply = Call("api/friends/get", {});
	if (!reply) {
		debugLogA("friends/get failed: %d %s", reply.code, reply.error.c_str());
		return;
	}

	std::vector<CMStringA> uids;
	for (auto &it : reply.root["items"]) {
		CMStringA uid(it.as_string().c_str());
		if (uid.IsEmpty() || uid == m_szMyUid)
			continue;

		uids.push_back(uid);
		AddUser(uid, false);
	}
	debugLogA("%d friends", (int)uids.size());
	ResolveUsers(uids);
}

/////////////////////////////////////////////////////////////////////////////////////////
// conversations

void COdnoklassnikiProto::PollConversations()
{
	static int nFailures = 0;

	std::map<CMStringA, CMStringA> params;
	params["fields"] = "conversation.*";
	params["count"] = "50";
	auto reply = Call("api/messagesV2/getList", params);

	if (!reply) {
		if (reply.code == OK_ERROR_SESSION_EXPIRED) {
			debugLogA("session expired, logging in again");
			if (!Login())
				m_bTerminated = true;
		}
		else if (reply.code == -1 && ++nFailures >= MAX_NETWORK_FAILURES) {
			debugLogA("the server does not answer, going offline");
			m_bTerminated = true;
		}
		return;
	}
	nFailures = 0;

	std::vector<OkConversation> list;
	std::vector<CMStringA> unknown;
	auto addUnknown = [&](const CMStringA &uid)
	{
		if (!uid.IsEmpty() && uid != m_szMyUid && GetUserName(uid).IsEmpty())
			if (std::find(unknown.begin(), unknown.end(), uid) == unknown.end())
				unknown.push_back(uid);
	};

	for (auto &it : reply.root["conversation"]) {
		OkConversation cnv;
		cnv.id = it["id"].as_string().c_str();
		if (cnv.id.IsEmpty())
			continue;

		cnv.bChat = !mir_strcmpi(it["type"].as_string().c_str(), "CHAT");
		cnv.topic = it["topic"].as_mstring();
		cnv.lastTime = GetInt64(it["last_msg_time_ms"]);
		cnv.newCount = it["new_msgs_count"].as_int();

		for (auto &p : it["participant"]) {
			CMStringA uid(p["id"].as_string().c_str());
			if (uid.IsEmpty())
				continue;
			cnv.participants.push_back(uid);
			if (!cnv.bChat && uid != m_szMyUid)
				cnv.peerUid = uid;
		}

		// fallback: a one-to-one id is base64("PRIVATE:<uid>")
		if (!cnv.bChat && cnv.peerUid.IsEmpty()) {
			size_t cbLen = 0;
			ptrA raw((char *)mir_base64_decode(cnv.id, &cbLen));
			if (raw) {
				CMStringA szRaw(raw.get(), (int)cbLen);
				int idx = szRaw.Find(':');
				if (idx != -1 && szRaw.Mid(idx + 1) != m_szMyUid)
					cnv.peerUid = szRaw.Mid(idx + 1);
			}
		}

		if (cnv.bChat)
			for (auto &uid : cnv.participants)
				addUnknown(uid);
		else
			addUnknown(cnv.peerUid);
		list.push_back(cnv);
	}

	ResolveUsers(unknown);

	for (auto &cnv : list) {
		if (m_bTerminated)
			break;
		ProcessConversation(cnv);
	}
}

void COdnoklassnikiProto::ProcessConversation(const OkConversation &cnv)
{
	MCONTACT hContact;
	if (cnv.bChat) {
		SESSION_INFO *si = StartChat(cnv);
		if (si == nullptr)
			return;
		hContact = si->hContact;
	}
	else {
		// the chat with yourself cannot be read through this API ("messageType is null")
		if (cnv.peerUid.IsEmpty() || cnv.peerUid == m_szMyUid)
			return;

		hContact = FindUser(cnv.peerUid);
		if (hContact == 0) {
			if (cnv.newCount == 0)
				return;   // a stranger with nothing new: nothing to show
			hContact = AddUser(cnv.peerUid, true);
		}
	}

	int64_t since = GetLastTime(this, hContact);
	if (since == 0) {
		// a conversation seen for the first time: its old history is not imported, only what is unread
		if (cnv.newCount > 0)
			FetchMessages(hContact, cnv, 0, cnv.newCount);
		else
			SetLastTime(this, hContact, cnv.lastTime);
		return;
	}

	if (cnv.lastTime > since)
		FetchMessages(hContact, cnv, since, -1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// messages

struct OkMessage
{
	CMStringA id, author, type;
	CMStringW text;
	int64_t date;
	std::vector<CMStringA> photos;   // attachment ids of the photos
	CMStringW others;                // what is shown for attachments we do not support
};

void COdnoklassnikiProto::FetchMessages(MCONTACT hContact, const OkConversation &cnv, int64_t since, int iMaxUnread)
{
	CMStringA szCnv(JsonEscape(cnv.id));
	CMStringA szMethods;
	szMethods.Format("[{\"method\":\"messagesV2.getMessages\",\"params\":{\"cnv_id\":\"%s\",\"direction\":\"Forward\",\"anchor\":\"U\","
		"\"frmt\":\"PLAIN_EXT_SMILES\",\"count\":\"30\",\"mark_as_read\":\"true\",\"fields\":\"%s\"}},"
		"{\"method\":\"messagesV2.getAttachedResources\",\"params\":{\"attach_ids\":{\"supplier\":\"messagesV2.getMessages.attachment_ids\"},"
		"\"fields\":\"attachment_photo.pic640x480,attachment_photo.pic1024max\"}}]",
		szCnv.c_str(), OK_MESSAGE_FIELDS);

	auto reply = Batch(szMethods);
	if (!reply)
		return;

	// picture urls of the photos, by attachment id
	std::map<CMStringA, CMStringA> photoUrls;
	for (auto &it : reply.root["messagesV2_getAttachedResources_response"]["attachments"]) {
		CMStringA id(it["id"].as_string().c_str());
		CMStringA url(it["pic1024max"].as_string().c_str());
		if (url.IsEmpty())
			url = it["pic640x480"].as_string().c_str();
		if (!id.IsEmpty() && !url.IsEmpty()) {
			if (!strncmp(url, "http://", 7))
				url = "https://" + url.Mid(7);
			photoUrls[id] = url;
		}
	}

	std::vector<OkMessage> messages;
	for (auto &it : reply.root["messagesV2_getMessages_response"]["message"]) {
		OkMessage msg;
		msg.id = it["id"].as_string().c_str();
		msg.author = it["author_id"].as_string().c_str();
		msg.type = it["type"].as_string().c_str();
		msg.text = it["text"].as_mstring();
		msg.text.Trim();
		msg.date = GetInt64(it["date_ms"]);
		if (msg.id.IsEmpty() || !mir_strcmpi(msg.type, "SYSTEM"))
			continue;

		for (auto &a : it["attachments"]) {
			CMStringA type(a["type"].as_string().c_str());
			if (!mir_strcmpi(type, "PHOTO"))
				msg.photos.push_back(a["id"].as_string().c_str());
			else
				msg.others.AppendFormat(L"%s[%S]", msg.others.IsEmpty() ? L"" : L" ", type.c_str());
		}
		messages.push_back(msg);
	}

	std::sort(messages.begin(), messages.end(), [](const OkMessage &a, const OkMessage &b) { return a.date < b.date; });

	// the first look at a conversation brings in only its unread incoming messages
	size_t iFirst = 0;
	if (iMaxUnread > 0) {
		int nIncoming = 0;
		iFirst = messages.size();
		while (iFirst > 0 && nIncoming < iMaxUnread) {
			iFirst--;
			if (messages[iFirst].author != m_szMyUid)
				nIncoming++;
		}
	}

	int64_t newest = max(since, cnv.lastTime);
	for (size_t i = iFirst; i < messages.size(); i++) {
		auto &msg = messages[i];
		if (iMaxUnread <= 0 && msg.date <= since)
			continue;
		if (msg.date > newest)
			newest = msg.date;

		bool bOut = (msg.author == m_szMyUid);
		if (bOut && db_event_getById(m_szModuleName, msg.id))
			continue;   // sent from here, already in the database
		if (bOut) {
			mir_cslock lck(m_csSentPhotos);
			if (m_sentPhotos.erase(msg.id))
				continue;   // a photo sent from here: it is in the database as a file transfer
		}

		CMStringA szUserId(msg.author);
		uint32_t timestamp = uint32_t(msg.date / 1000);

		// photos become files that are downloaded on demand; the text goes with the first one
		int iPhoto = 0;
		for (auto &photoId : msg.photos) {
			auto url = photoUrls.find(photoId);
			if (url == photoUrls.end())
				continue;

			CMStringA szId(msg.id);
			if (iPhoto > 0)
				szId.AppendFormat("_%d", iPhoto);
			if (db_event_getById(m_szModuleName, szId))
				continue;

			CMStringA szName(FORMAT, "photo_%s.jpg", photoId.c_str());
			T2Utf szDescr(iPhoto == 0 ? msg.text : CMStringW());

			DB::EventInfo dbei;
			dbei.iTimestamp = timestamp;
			dbei.szId = szId;
			if (cnv.bChat)
				dbei.szUserId = szUserId;
			if (bOut)
				dbei.flags |= DBEF_SENT | DBEF_READ;
			ProtoChainRecvFile(hContact, DB::FILE_BLOB(new OkPhoto{ url->second }, szName, szDescr), dbei);
			iPhoto++;
		}

		CMStringW wszText(iPhoto == 0 ? msg.text : CMStringW());
		if (!msg.others.IsEmpty())
			wszText += (wszText.IsEmpty() ? L"" : L"\n") + msg.others;
		if (wszText.IsEmpty())
			continue;

		T2Utf szText(wszText);
		DB::EventInfo dbei;
		dbei.iTimestamp = timestamp;
		dbei.szId = msg.id;
		dbei.pBlob = szText.get();
		if (cnv.bChat)
			dbei.szUserId = szUserId;
		if (bOut)
			dbei.flags |= DBEF_SENT | DBEF_READ;
		ProtoChainRecvMsg(hContact, dbei);
	}

	SetLastTime(this, hContact, newest);
}
