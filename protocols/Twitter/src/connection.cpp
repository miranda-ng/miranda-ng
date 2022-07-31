/*
Copyright © 2012-22 Miranda NG team
Copyright © 2009 Jim Porter

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

AsyncHttpRequest::AsyncHttpRequest(int type, const char *szUrl)
{
	requestType = type;
	m_szUrl = szUrl;
}

void CALLBACK CTwitterProto::APC_callback(ULONG_PTR p)
{
	reinterpret_cast<CTwitterProto*>(p)->debugLogA("***** Executing APC");
}

template<typename T>
inline static T db_pod_get(MCONTACT hContact, const char *module, const char *setting, T errorValue)
{
	DBVARIANT dbv;
	if (db_get(hContact, module, setting, &dbv))
		return errorValue;

	T ret = *(T*)dbv.pbVal;
	db_free(&dbv);
	return ret;
}

template<typename T>
inline static INT_PTR db_pod_set(MCONTACT hContact, const char *module, const char *setting, T val)
{
	return db_set_blob(hContact, module, setting, &val, sizeof(T));
}

void CTwitterProto::SignOn(void*)
{
	debugLogA("***** Beginning SignOn process");
	mir_cslock lck(signon_lock_);

	// Kill the old thread if it's still around
	// this doesn't seem to work.. should we wait infinitely?
	if (hMsgLoop_) {
		debugLogA("***** Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback, hMsgLoop_, (ULONG_PTR)this);
		debugLogA("***** Waiting for old MessageLoop to exit");
		//WaitForSingleObject(hMsgLoop_,INFINITE);
		WaitForSingleObject(hMsgLoop_, 180000);
		CloseHandle(hMsgLoop_);
	}
	if (NegotiateConnection()) // Could this be? The legendary Go Time??
	{
		if (!in_chat_ && getByte(TWITTER_KEY_CHATFEED))
			OnJoinChat(0, true);

		setAllContactStatuses(ID_STATUS_ONLINE);
		SetChatStatus(ID_STATUS_ONLINE);
		hMsgLoop_ = ForkThreadEx(&CTwitterProto::MessageLoop, nullptr, nullptr);
	}

	debugLogA("***** SignOn complete");
}

bool CTwitterProto::NegotiateConnection()
{
	debugLogA("***** Negotiating connection with Twitter");
	disconnectionCount = 0;

	// saving the current status to a temp var
	int old_status = m_iStatus;

	CMStringA szOauthToken = getMStringA(TWITTER_KEY_OAUTH_TOK);
	CMStringA szOauthTokenSecret = getMStringA(TWITTER_KEY_OAUTH_TOK_SEC);
	m_szUserName = getMStringA(TWITTER_KEY_NICK);
	if (m_szUserName.IsEmpty())
		m_szUserName = getMStringA(TWITTER_KEY_UN);

	if (szOauthToken.IsEmpty() || szOauthTokenSecret.IsEmpty()) {
		// first, reset all the keys so we can start fresh
		debugLogA("**NegotiateConnection - Reset OAuth Keys");
		resetOAuthKeys();

		m_szUserName.Empty();
		debugLogA("**NegotiateConnection - Requesting oauthTokens");
		http::response resp = request_token();

		StringPairs response = ParseQueryString(resp.data);
		szOauthToken = response[L"oauth_token"];
		szOauthTokenSecret = response[L"oauth_token_secret"];

		if (szOauthToken.IsEmpty()) {
			ShowPopup("OAuth token not received, check your internet connection?", 1);
			debugLogA("**NegotiateConnection - OAuth tokens not received, stopping before we open the web browser..");
			return false;
		}

		// write those bitches to the db foe latta
		setString(TWITTER_KEY_OAUTH_TOK, m_szAccessToken = szOauthToken);
		setString(TWITTER_KEY_OAUTH_TOK_SEC, m_szAccessTokenSecret = szOauthTokenSecret);

		// this looks like bad code.. can someone clean this up please?  or confirm that it's ok
		char buf[1024];
		mir_snprintf(buf, "https://api.twitter.com/oauth/authorize?oauth_token=%s", szOauthToken.c_str());
		debugLogA("**NegotiateConnection - Launching %s", buf);
		Utils_OpenUrl(buf);

		ShowPinDialog();
	}

	ptrW wszGroup(getWStringA(TWITTER_KEY_GROUP));
	if (wszGroup)
		Clist_GroupCreate(0, wszGroup);

	// remember, dbTOK is 0 (false) if the db setting has returned something
	szOauthToken = getMStringA(TWITTER_KEY_OAUTH_ACCESS_TOK);
	szOauthTokenSecret = getMStringA(TWITTER_KEY_OAUTH_ACCESS_SEC);

	if (szOauthToken.IsEmpty() || szOauthTokenSecret.IsEmpty()) {  // if we don't have one of these beasties then lets go get 'em!
		debugLogA("**NegotiateConnection - either the accessToken or accessTokenSecret was not there..");
		m_szPin = getMStringA(TWITTER_KEY_OAUTH_PIN);
		if (m_szPin.IsEmpty()) {
			ShowPopup(TranslateT("OAuth variables are out of sequence, they have been reset. Please reconnect and reauthorize Miranda to Twitter.com (do the PIN stuff again)"));
			debugLogA("**NegotiateConnection - We don't have a PIN?  this doesn't make sense.  Resetting OAuth keys and setting offline.");
			resetOAuthKeys();

			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)old_status, m_iStatus);

			// Set to offline
			old_status = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
			return false;
		}

		debugLogA("**NegotiateConnection - requesting access tokens...");
		http::response accessResp = request_access_tokens();
		m_szPin.Empty();
		if (accessResp.code != 200) {
			debugLogA("**NegotiateConnection - Failed to get Access Tokens, HTTP response code is: %d", accessResp.code);
			ShowPopup(TranslateT("Failed to get Twitter Access Tokens, please go offline and try again. If this keeps happening, check your internet connection."));

			resetOAuthKeys();

			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)old_status, m_iStatus);

			// Set to offline
			old_status = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

			return false;
		}

		debugLogA("**NegotiateConnection - Successfully retrieved Access Tokens");

		StringPairs accessTokenParameters = ParseQueryString(accessResp.data);
		m_szAccessToken = accessTokenParameters[L"oauth_token"];
		m_szAccessTokenSecret = accessTokenParameters[L"oauth_token_secret"];
		m_szUserName = accessTokenParameters[L"screen_name"];
		debugLogA("**NegotiateConnection - screen name is %s", m_szUserName.c_str());

		// save em
		setUString(TWITTER_KEY_OAUTH_ACCESS_TOK, m_szAccessToken);
		setUString(TWITTER_KEY_OAUTH_ACCESS_SEC, m_szAccessTokenSecret);
		setUString(TWITTER_KEY_NICK, m_szUserName);
		setUString(TWITTER_KEY_UN, m_szUserName);
	}
	else {
		m_szAccessToken = szOauthToken;
		m_szAccessTokenSecret = szOauthTokenSecret;
	}

	debugLogA("**NegotiateConnection - Setting Consumer Keys and verifying creds...");

	if (m_szUserName.IsEmpty()) {
		ShowPopup(TranslateT("You're missing the Nick key in the database. This isn't really a big deal, but you'll notice some minor quirks (self contact in list, no group chat outgoing message highlighting, etc). To fix it either add it manually or recreate your Miranda Twitter account"));
		debugLogA("**NegotiateConnection - Missing the Nick key in the database.  Everything will still work, but it's nice to have");
	}

	auto *req = new AsyncHttpRequest(REQUEST_GET, "/account/verify_credentials.json");
	auto resp(Execute(req));
	if (resp.code != 200) {
		debugLogA("**NegotiateConnection - Verifying credentials failed!  No internet maybe?");

LBL_Error:
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)old_status, m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		return false;
	}

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root) {
		debugLogA("unable to parse response");
		goto LBL_Error;
	}

	m_szMyId = root["id_str"].as_mstring();
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return true;
}

void CTwitterProto::MessageLoop(void*)
{
	debugLogA("***** Entering Twitter::MessageLoop");

	since_id_ = db_pod_get<twitter_id>(0, m_szModuleName, TWITTER_KEY_SINCEID, 0);
	dm_since_id_ = db_pod_get<twitter_id>(0, m_szModuleName, TWITTER_KEY_DMSINCEID, 0);

	bool new_account = getByte(TWITTER_KEY_NEW, 1) != 0;
	bool popups = getByte(TWITTER_KEY_POPUP_SIGNON, 1) != 0;

	// if this isn't set, it will automatically not turn a tweet into a msg. probably should make the default that it does turn a tweet into a message
	bool tweetToMsg = getByte(TWITTER_KEY_TWEET_TO_MSG) != 0;

	int poll_rate = getDword(TWITTER_KEY_POLLRATE, 80);

	for (unsigned int i = 0;; i++) {

		if (m_iStatus != ID_STATUS_ONLINE)
			break;

		if (i % 10 == 0)
			UpdateFriends();

		if (m_iStatus != ID_STATUS_ONLINE)
			break;

		UpdateStatuses(new_account, popups, tweetToMsg);
		if (m_iStatus != ID_STATUS_ONLINE)
			break;

		UpdateMessages(new_account);

		if (new_account) { // Not anymore!
			new_account = false;
			setByte(TWITTER_KEY_NEW, 0);
		}

		if (m_iStatus != ID_STATUS_ONLINE)
			break;
		debugLogA("***** CTwitterProto::MessageLoop going to sleep...");
		if (SleepEx(poll_rate * 1000, true) == WAIT_IO_COMPLETION)
			break;
		debugLogA("***** CTwitterProto::MessageLoop waking up...");

		popups = true;
	}

	debugLogA("***** Exiting CTwitterProto::MessageLoop");
}

struct update_avatar
{
	update_avatar(MCONTACT hContact, const CMStringA &url) : hContact(hContact), url(url) {}
	MCONTACT hContact;
	CMStringA url;
};

/* void *p should always be a struct of type update_avatar */
void CTwitterProto::UpdateAvatarWorker(void *p)
{
	if (p == nullptr)
		return;
	
	std::unique_ptr<update_avatar> data((update_avatar*)p);

	// db_get_s returns 0 when it suceeds, so if this suceeds it will return 0, or false.
	// therefore if it returns 1, or true, we want to return as there is no such user.
	// as a side effect, dbv now has the username in it i think
	CMStringA username(getMStringA(data->hContact, TWITTER_KEY_UN));
	if (username.IsEmpty())
		return;

	CMStringA ext = data->url.Right(data->url.ReverseFind('.')); // finds the filetype of the avatar
	CMStringW filename(FORMAT, L"%s\\%S%S", GetAvatarFolder().c_str(), username.c_str(), ext.c_str()); // local filename and path

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = data->hContact;
	ai.format = ProtoGetAvatarFormat(filename.c_str());

	if (ai.format == PA_FORMAT_UNKNOWN) {
		debugLogA("***** Update avatar: Terminated for this contact, extension format unknown for %s", data->url.c_str());
		return; // lets just ignore unknown formats... if not it crashes miranda. should probably speak to borkra about this.
	}

	wcsncpy(ai.filename, filename.c_str(), MAX_PATH); // puts the local file name in the avatar struct, to a max of 260 chars (as of now)

	debugLogA("***** Updating avatar: %s", data->url.c_str());
	mir_cslock lck(avatar_lock_);

	if (Miranda_IsTerminated()) { // if miranda is shutting down...
		debugLogA("***** Terminating avatar update early: %s", data->url.c_str());
		return;
	}

	if (save_url(hAvatarNetlib_, data->url, filename)) {
		setString(data->hContact, TWITTER_KEY_AV_URL, data->url.c_str());
		ProtoBroadcastAck(data->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
	}
	else ProtoBroadcastAck(data->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);

	debugLogA("***** Done avatar: %s", data->url.c_str());
}

void CTwitterProto::UpdateAvatar(MCONTACT hContact, const CMStringA &url, bool force)
{
	DBVARIANT dbv = { 0 };

	if (!force && (!getString(hContact, TWITTER_KEY_AV_URL, &dbv) && url == dbv.pszVal)) {
		debugLogA("***** Avatar already up-to-date: %s", url.c_str());
	}
	else {
		// TODO: more defaults (configurable?)
		if (url == "http://static.twitter.com/images/default_profile_normal.png") {
			PROTO_AVATAR_INFORMATION ai = { 0 };
			ai.hContact = hContact;

			setString(hContact, TWITTER_KEY_AV_URL, url.c_str());
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai, 0);
		}
		else {
			ForkThread(&CTwitterProto::UpdateAvatarWorker, new update_avatar(hContact, url));
		}
	}

	db_free(&dbv);
}

void CTwitterProto::UpdateFriends()
{
	auto *req = new AsyncHttpRequest(REQUEST_GET, "/friends/list.json");
	http::response resp = Execute(req);
	if (resp.code != 200) {
		debugLogA("Friend list reading failed");
		return;
	}

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root) {
		debugLogA("unable to parse response");
		return;
	}

	for (auto &one : root["users"]) {
		std::string username = one["screen_name"].as_string();
		if (m_szUserName == username.c_str())
			continue;

		std::string id = one["id_str"].as_string();
		std::string real_name = one["name"].as_string();
		std::string profile_image_url = one["profile_image_url"].as_string();
		std::string status_text = one["status"]["text"].as_string();

		MCONTACT hContact = AddToClientList(username.c_str(), status_text.c_str());
		setString(hContact, TWITTER_KEY_ID, id.c_str());
		setUString(hContact, "Nick", real_name.c_str());
		UpdateAvatar(hContact, profile_image_url.c_str());
	}
	disconnectionCount = 0;
	debugLogA("***** Friends list updated");
}

LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CMStringA *url;

	switch (message) {
	case WM_COMMAND:
		// Get the plugin data (we need the Popup service to do it)
		url = (CMStringA *)PUGetPluginData(hwnd);
		if (url != nullptr)
			Utils_OpenUrl(url->c_str());

		// Intentionally no break here

	case WM_CONTEXTMENU:
		// After a click, destroy popup
		PUDeletePopup(hwnd);
		return TRUE;

	case UM_FREEPLUGINDATA:
		// After close, free
		url = (CMStringA *)PUGetPluginData(hwnd);
		delete url;
		return FALSE;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
};

void CTwitterProto::ShowContactPopup(MCONTACT hContact, const CMStringA &text, const CMStringA *url)
{
	if (!getByte(TWITTER_KEY_POPUP_SHOW))
		return;

	POPUPDATAW popup = {};
	popup.lchContact = hContact;
	popup.iSeconds = getDword(TWITTER_KEY_POPUP_TIMEOUT);

	popup.colorBack = getDword(TWITTER_KEY_POPUP_COLBACK);
	if (popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOW);
	popup.colorText = getDword(TWITTER_KEY_POPUP_COLTEXT);
	if (popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOWTEXT);

	DBVARIANT dbv;
	if (!db_get_ws(hContact, "CList", "MyHandle", &dbv) || !getWString(hContact, TWITTER_KEY_UN, &dbv)) {
		wcsncpy(popup.lpwzContactName, dbv.pwszVal, MAX_CONTACTNAME);
		db_free(&dbv);
	}

	if (url != nullptr) {
		popup.PluginWindowProc = PopupWindowProc;
		popup.PluginData = (void *)url;
	}

	wcsncpy_s(popup.lpwzText, Utf2T(text), MAX_SECONDLINE);
	PUAddPopupW(&popup);
}

void CTwitterProto::UpdateStatuses(bool pre_read, bool popups, bool tweetToMsg)
{
	auto *req = new AsyncHttpRequest(REQUEST_GET, "/statuses/home_timeline.json");
	req << INT_PARAM("count", 200);
	if (since_id_ != 0)
		req << INT64_PARAM("since_id", since_id_);

	http::response resp = Execute(req);
	if (resp.code != 200) {
		debugLogA("Status update failed with error %d", resp.code);
		return;
	}

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root) {
		debugLogA("Status update failed: unable to parse response");
		return;
	}

	OBJLIST<twitter_user> messages(10);

	for (auto &one : root) {
		const JSONNode &pUser = one["user"];

		auto *u = new twitter_user();
		u->username = pUser["screen_name"].as_string();
		u->real_name = pUser["name"].as_string();
		u->profile_image_url = pUser["profile_image_url"].as_string();

		CMStringA retweeteesName, retweetText;

		// the tweet will be truncated unless we take action.  i hate you CTwitterProto API
		const JSONNode &pStatus = one["retweeted_status"];
		if (pStatus) {
			// here we grab the "retweeted_status" um.. section?  it's in here that all the info we need is
			// at this point the user will get no tweets and an error popup if the tweet happens to be exactly 140 chars, start with
			// "RT @", end in " ...", and notactually be a real retweet.  it's possible but unlikely, wish i knew how to get
			// the retweet_count variable to work :(
			const JSONNode &pRetweet = one["retweeted_status"],
				&pUser2 = pRetweet["user"];

			retweeteesName = pUser2["screen_name"].as_string().c_str(); // the user that is being retweeted
			retweetText = pRetweet["text"].as_string().c_str(); // their tweet in all it's untruncated glory

			// fix html entities in the text
			htmlEntitiesDecode(retweetText);

			u->status.text = "RT @" + retweeteesName + " " + retweetText; // mash it together in some format people will understand
		}
		else {
			// if it's not truncated, then the CTwitterProto API returns the native RT correctly anyway,
			CMStringA rawText = one["text"].as_string().c_str();

			// fix html entities in the text
			htmlEntitiesDecode(rawText);

			u->status.text = rawText;
		}

		u->status.id = _atoi64(one["id"].as_string().c_str());
		if (u->status.id > since_id_)
			since_id_ = u->status.id;

		std::string timestr = one["created_at"].as_string();
		u->status.time = parse_time(timestr.c_str());
		messages.insert(u);
	}

	for (auto &u : messages.rev_iter()) {
		if (!pre_read && in_chat_)
			UpdateChat(*u);

		if (u->username == m_szUserName.c_str())
			continue;

		MCONTACT hContact = AddToClientList(u->username.c_str(), "");
		UpdateAvatar(hContact, u->profile_image_url.c_str());

		// if we send twits as messages, add an unread event
		if (tweetToMsg) {
			DBEVENTINFO dbei = {};
			dbei.pBlob = (uint8_t*)(u->status.text.c_str());
			dbei.cbBlob = (int)u->status.text.length() + 1;
			dbei.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
			dbei.flags = DBEF_UTF;
			dbei.timestamp = static_cast<uint32_t>(u->status.time);
			dbei.szModule = m_szModuleName;
			db_event_add(hContact, &dbei);
		}
		else db_set_utf(hContact, "CList", "StatusMsg", u->status.text.c_str());

		if (!pre_read && popups) {
			Skin_PlaySound("TwitterNew");
			ShowContactPopup(hContact, u->status.text.c_str(), new CMStringA(FORMAT, "https://twitter.com/%s/status/%lld", u->username.c_str(), u->status.id));
		}
	}

	db_pod_set(0, m_szModuleName, TWITTER_KEY_SINCEID, since_id_);
	disconnectionCount = 0;
	debugLogA("***** Status messages updated");
}

void CTwitterProto::UpdateMessages(bool pre_read)
{
	auto *req = new AsyncHttpRequest(REQUEST_GET, "/direct_messages/events/list.json");
	req << INT_PARAM("count", 50);
	if (dm_since_id_ != 0)
		req << INT64_PARAM("since_id", dm_since_id_);

	http::response resp = Execute(req);
	if (resp.code != 200) {
		debugLogA("Message request failed, error %d", resp.code);
		return;
	}

	JSONNode root = JSONNode::parse(resp.data.c_str());
	if (!root) {
		debugLogA("unable to parse response");
		return;
	}

	for (auto &one : root["events"]) {
		std::string type = one["type"].as_string();
		if (type != "message_create")
			continue;

		bool bIsMe = false;
		auto &msgCreate = one["message_create"];
		std::string sender = msgCreate["sender_id"].as_string();
		if (m_szMyId == sender.c_str()) {
			bIsMe = true;
			sender = msgCreate["target"]["recipient_id"].as_string();
		}

		MCONTACT hContact = FindContactById(sender.c_str());
		if (hContact == INVALID_CONTACT_ID) {
			hContact = AddToClientList(sender.c_str(), "");
			Contact::RemoveFromList(hContact);
		}

		std::string text = msgCreate["message_data"]["text"].as_string();
		__time64_t time = _atoi64(one["created_timestamp"].as_string().c_str()) / 1000;

		std::string msgid = one["id"].as_string();
		if (db_event_getById(m_szModuleName, msgid.c_str()))
			continue;

		twitter_id id = _atoi64(msgid.c_str());
		if (id > dm_since_id_)
			dm_since_id_ = id;

		PROTORECVEVENT recv = { 0 };
		if (pre_read)
			recv.flags |= PREF_CREATEREAD;
		if (bIsMe)
			recv.flags |= PREF_SENT;
		recv.szMessage = const_cast<char*>(text.c_str());
		recv.timestamp = static_cast<uint32_t>(time);
		recv.szMsgId = msgid.c_str();
		
		MEVENT hDbEVent = (MEVENT)ProtoChainRecvMsg(hContact, &recv);
		if (!msgid.empty())
			m_arChatMarks.insert(new CChatMark(hDbEVent, msgid.c_str()));
	}

	db_pod_set(0, m_szModuleName, TWITTER_KEY_DMSINCEID, dm_since_id_);
	disconnectionCount = 0;
	debugLogA("***** Direct messages updated");
}

void CTwitterProto::resetOAuthKeys()
{
	delSetting(TWITTER_KEY_OAUTH_ACCESS_TOK);
	delSetting(TWITTER_KEY_OAUTH_ACCESS_SEC);
	delSetting(TWITTER_KEY_OAUTH_TOK);
	delSetting(TWITTER_KEY_OAUTH_TOK_SEC);
	delSetting(TWITTER_KEY_OAUTH_PIN);
}
