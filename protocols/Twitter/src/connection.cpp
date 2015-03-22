/*
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
#include "proto.h"
#include "twitter.h"

void CALLBACK TwitterProto::APC_callback(ULONG_PTR p)
{
	reinterpret_cast<TwitterProto*>(p)->debugLogA("***** Executing APC");
}

template<typename T>
inline static T db_pod_get(MCONTACT hContact,const char *module,const char *setting,T errorValue)
{
	DBVARIANT dbv;
	if(db_get(hContact, module, setting, &dbv))
		return errorValue;

	// TODO: remove this, it's just a temporary workaround
	if(dbv.type == DBVT_DWORD)
		return dbv.dVal;

	if(dbv.cpbVal != sizeof(T))
		return errorValue;
	return *reinterpret_cast<T*>(dbv.pbVal);
}

template<typename T>
inline static INT_PTR db_pod_set(MCONTACT hContact,const char *module,const char *setting,T val)
{
	return db_set_blob(hContact, module, setting, &val, sizeof(T));
}

void TwitterProto::SignOn(void*)
{
	debugLogA("***** Beginning SignOn process");
	WaitForSingleObject(&signon_lock_,INFINITE);

	// Kill the old thread if it's still around
	// this doesn't seem to work.. should we wait infinitely?
	if(hMsgLoop_)
	{
		debugLogA("***** Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback,hMsgLoop_,(ULONG_PTR)this);
		debugLogA("***** Waiting for old MessageLoop to exit");
		//WaitForSingleObject(hMsgLoop_,INFINITE);
		WaitForSingleObject(hMsgLoop_,180000);
		CloseHandle(hMsgLoop_);
	}
	if(NegotiateConnection()) // Could this be? The legendary Go Time??
	{
		if(!in_chat_ && db_get_b(0,m_szModuleName,TWITTER_KEY_CHATFEED,0))
			OnJoinChat(0,true);
		
		SetAllContactStatuses(ID_STATUS_ONLINE);
		hMsgLoop_ = ForkThreadEx(&TwitterProto::MessageLoop, NULL, 0);
	}

	ReleaseMutex(signon_lock_);
	debugLogA("***** SignOn complete");
}

bool TwitterProto::NegotiateConnection()
{
	debugLogA("***** Negotiating connection with Twitter");
	disconnectionCount = 0;

	// saving the current status to a temp var
	int old_status = m_iStatus;
	DBVARIANT dbv;

	wstring oauthToken;
	wstring oauthTokenSecret;	
	wstring oauthAccessToken;
	wstring oauthAccessTokenSecret;
	string screenName;

	INT_PTR dbTOK = db_get_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK,&dbv);
	if (!dbTOK) {
		oauthToken = dbv.pwszVal;
		db_free(&dbv);
		//debugLogW("**NegotiateConnection - we have an oauthToken already in the db - %s", oauthToken);
	}
 
	INT_PTR dbTOKSec = db_get_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK_SECRET,&dbv);
	if (!dbTOKSec) {
		oauthTokenSecret = dbv.pwszVal;
		db_free(&dbv);
		//debugLogW("**NegotiateConnection - we have an oauthTokenSecret already in the db - %s", oauthTokenSecret);
	}

	INT_PTR dbName = db_get_s(0,m_szModuleName,TWITTER_KEY_NICK,&dbv);
	if (!dbName) {
		screenName = dbv.pszVal;
		db_free(&dbv);
		//debugLogW("**NegotiateConnection - we have a username already in the db - %s", screenName);
	}
	else {
		dbName = db_get_s(0,m_szModuleName,TWITTER_KEY_UN,&dbv);
		if (!dbName) {
			screenName = dbv.pszVal;
			db_set_s(0,m_szModuleName,TWITTER_KEY_NICK,dbv.pszVal);
			db_free(&dbv);
			//debugLogW("**NegotiateConnection - we have a username already in the db - %s", screenName);
		}
	}

 	// twitter changed the base URL in v1.1 of the API, I don't think users will need to modify it, so
	// i'll be forcing it to the new API URL here. After a while I can get rid of this as users will
	// have either had this run at least once, or have reset their miranda profile. 14/10/2012
	if(db_get_b(0,m_szModuleName,"UpgradeBaseURL",1)) {
		db_set_s(0,m_szModuleName,TWITTER_KEY_BASEURL,"https://api.twitter.com/");
		db_set_b(0,m_szModuleName,"UpgradeBaseURL",0);
	}

	if((oauthToken.size() <= 1) || (oauthTokenSecret.size() <= 1)) {
		// first, reset all the keys so we can start fresh
		resetOAuthKeys();
		debugLogA("**NegotiateConnection - Reset OAuth Keys");

		//twit_.set_credentials(ConsumerKey, ConsumerSecret, oauthAccessToken, oauthAccessTokenSecret, L"", false);
		// i think i was doin the wrong thing here.. i was setting the credentials as oauthAccessToken instead of oauthToken
		// have to test..
		debugLogA("**NegotiateConnection - Setting Consumer Keys...");
		/*debugLogW("**NegotiateConnection - sending set_cred: consumerKey is %s", ConsumerKey);
		debugLogW("**NegotiateConnection - sending set_cred: consumerSecret is %s", ConsumerSecret);
		debugLogW("**NegotiateConnection - sending set_cred: oauthToken is %s", oauthToken);
		debugLogW("**NegotiateConnection - sending set_cred: oauthTokenSecret is %s", oauthTokenSecret);
		debugLogA("**NegotiateConnection - sending set_cred: no pin");*/
		twit_.set_credentials("", ConsumerKey, ConsumerSecret, oauthToken, oauthTokenSecret, L"", false);
		debugLogA("**NegotiateConnection - Requesting oauthTokens");
		http::response resp = twit_.request_token();

		//wstring rdata_WSTR(resp.data.length(),L' ');
		wstring rdata_WSTR = UTF8ToWide(resp.data);

		//debugLogW("**NegotiateConnection - REQUEST TOKEN IS %s", rdata_WSTR);
		OAuthParameters response = twit_.ParseQueryString(rdata_WSTR);
		oauthToken = response[L"oauth_token"];
		oauthTokenSecret = response[L"oauth_token_secret"];
		//debugLogW("**NegotiateConnection - oauthToken is %s", oauthToken);
		//debugLogW("**NegotiateConnection - oauthTokenSecret is %s", oauthTokenSecret);

		if (oauthToken.length() < 1) {
			ShowPopup("OAuth Tokens not received, check your internet connection?", 1);
			debugLogA("**NegotiateConnection - OAuth tokens not received, stopping before we open the web browser..");
			return false;
		}

		//write those bitches to the db foe latta
		db_set_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK,oauthToken.c_str());
		db_set_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK_SECRET,oauthTokenSecret.c_str());
		
		// this looks like bad code.. can someone clean this up please?  or confirm that it's ok
		wchar_t buf[1024] = {};
		mir_snwprintf(buf, SIZEOF(buf), AuthorizeUrl.c_str(), oauthToken.c_str());

		debugLogW( _T("**NegotiateConnection - Launching %s"), buf);
		ShellExecute(NULL, L"open", buf, NULL, NULL, SW_SHOWNORMAL);
		
		ShowPinDialog();
	}

	if (!db_get_ts(NULL,m_szModuleName,TWITTER_KEY_GROUP,&dbv)) {
		CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)dbv.ptszVal );
		db_free(&dbv);	
	}

	bool realAccessTok = false;
	bool realAccessTokSecret = false;

	// remember, dbTOK is 0 (false) if the db setting has returned something
	dbTOK = db_get_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK,&dbv);
	if (!dbTOK) { 
		oauthAccessToken = dbv.pwszVal;
		db_free(&dbv);
		// this bit is saying "if we have found the db key, but it contains no data, then set dbTOK to 1"
		if (oauthAccessToken.size() > 1) { 
			realAccessTok = true; 
			//debugLogW("**NegotiateConnection - we have an oauthAccessToken already in the db - %s", oauthAccessToken); 
		}
		else { debugLogA("**NegotiateConnection - oauthAccesToken too small? this is.. weird."); }
	}

	dbTOKSec = db_get_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET,&dbv);
	if (!dbTOKSec) { 
		oauthAccessTokenSecret = dbv.pwszVal;
		db_free(&dbv);
		if (oauthAccessTokenSecret.size() > 1) { 
			realAccessTokSecret = true; 
			//debugLogW("**NegotiateConnection - we have an oauthAccessTokenSecret already in the db - %s", oauthAccessTokenSecret); 
		}
		else { debugLogA("**NegotiateConnection - oauthAccessTokenSecret too small? weird"); }
	}

	if (!realAccessTok || !realAccessTokSecret) {  // if we don't have one of these beasties then lets go get 'em!
		wstring pin;
		debugLogA("**NegotiateConnection - either the accessToken or accessTokenSecret was not there..");
		if (!db_get_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_PIN,&dbv)) {
			pin = dbv.pwszVal;
			//debugLogW("**NegotiateConnection - we have an pin already in the db - %s", pin);
			db_free(&dbv);
		}
		else {
			ShowPopup(TranslateT("OAuth variables are out of sequence, they have been reset. Please reconnect and reauthorize Miranda to Twitter.com (do the PIN stuff again)"));
			debugLogA("**NegotiateConnection - We don't have a PIN?  this doesn't make sense.  Resetting OAuth keys and setting offline.");
			resetOAuthKeys();

			ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_FAILED,(HANDLE)old_status,m_iStatus);

			// Set to offline
			old_status = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_iStatus);

			return false;
		}

		debugLogA("**NegotiateConnection - Setting Consumer Keys and PIN...");
		/*debugLogW("**NegotiateConnection - sending set_cred: consumerKey is %s", ConsumerKey);
		debugLogW("**NegotiateConnection - sending set_cred: consumerSecret is %s", ConsumerSecret);
		debugLogW("**NegotiateConnection - sending set_cred: oauthToken is %s", oauthToken);
		debugLogW("**NegotiateConnection - sending set_cred: oauthTokenSecret is %s", oauthTokenSecret);
		debugLogW("**NegotiateConnection - sending set_cred: pin is %s", pin);*/

		twit_.set_credentials("", ConsumerKey, ConsumerSecret, oauthToken, oauthTokenSecret, pin, false);

		debugLogA("**NegotiateConnection - requesting access tokens...");
		http::response accessResp = twit_.request_access_tokens();
		if (accessResp.code != 200) {
			debugLogA("**NegotiateConnection - Failed to get Access Tokens, HTTP response code is: %d", accessResp.code);
			ShowPopup(TranslateT("Failed to get Twitter Access Tokens, please go offline and try again. If this keeps happening, check your internet connection."));

			resetOAuthKeys();

			ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_FAILED,(HANDLE)old_status,m_iStatus);

			// Set to offline
			old_status = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_iStatus);

			return false;
		}
		else {
			debugLogA("**NegotiateConnection - Successfully retrieved Access Tokens");

			wstring rdata_WSTR2 = UTF8ToWide(accessResp.data);
			//debugLogW("**NegotiateConnection - accessToken STring is %s", rdata_WSTR2);
		
			OAuthParameters accessTokenParameters = twit_.ParseQueryString(rdata_WSTR2);

			oauthAccessToken = accessTokenParameters[L"oauth_token"];
			//debugLogW("**NegotiateConnection - oauthAccessToken is %s", oauthAccessToken);

			oauthAccessTokenSecret = accessTokenParameters[L"oauth_token_secret"];
			//debugLogW("**NegotiateConnection - oauthAccessTokenSecret is %s", oauthAccessTokenSecret);

			screenName = WideToUTF8(accessTokenParameters[L"screen_name"]);
			debugLogA("**NegotiateConnection - screen name is %s", screenName.c_str());
	
			//save em
			db_set_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK,oauthAccessToken.c_str());
			db_set_ws(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET,oauthAccessTokenSecret.c_str());
			db_set_s(0,m_szModuleName,TWITTER_KEY_NICK,screenName.c_str());
			db_set_s(0,m_szModuleName,TWITTER_KEY_UN,screenName.c_str());
		}
	}

/*	if( !db_get_s(0,m_szModuleName,TWITTER_KEY_PASS,&dbv)) {
		CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
			reinterpret_cast<LPARAM>(dbv.pszVal));
		pass = dbv.pszVal;
		db_free(&dbv);
	}
	else {
		ShowPopup(TranslateT("Please enter a password."));
		return false;
	}*/

	if( !db_get_s(0,m_szModuleName,TWITTER_KEY_BASEURL,&dbv))
	{
		ScopedLock s(twitter_lock_);
		twit_.set_base_url(dbv.pszVal);
		db_free(&dbv);
	}

	debugLogA("**NegotiateConnection - Setting Consumer Keys and verifying creds...");

	if (screenName.empty()) {
		ShowPopup(TranslateT("You're missing the Nick key in the database. This isn't really a big deal, but you'll notice some minor quirks (self contact in list, no group chat outgoing message highlighting, etc). To fix it either add it manually or reset your Twitter account in the Miranda account options"));
		debugLogA("**NegotiateConnection - Missing the Nick key in the database.  Everything will still work, but it's nice to have");
	}

	bool success;
	{		
		ScopedLock s(twitter_lock_);

		success = twit_.set_credentials(screenName, ConsumerKey, ConsumerSecret, oauthAccessToken, oauthAccessTokenSecret, L"", true);
	}

	if(!success) {
		//ShowPopup(TranslateT("Something went wrong with authorization, OAuth keys have been reset.  Please try to reconnect.  If problems persist, please se your doctor"));
		debugLogA("**NegotiateConnection - Verifying credentials failed!  No internet maybe?");

		//resetOAuthKeys();
		ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_FAILED,(HANDLE)old_status,m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_iStatus);

		return false;
	}
	else {
		m_iStatus = m_iDesiredStatus;

		ProtoBroadcastAck(0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_iStatus);
		return true;
	}
}


void TwitterProto::MessageLoop(void*)
{
	debugLogA("***** Entering Twitter::MessageLoop");

	since_id_    = db_pod_get<twitter_id>(0,m_szModuleName,TWITTER_KEY_SINCEID,0);
	dm_since_id_ = db_pod_get<twitter_id>(0,m_szModuleName,TWITTER_KEY_DMSINCEID,0);

	bool new_account = db_get_b(0,m_szModuleName,TWITTER_KEY_NEW,1) != 0;
	bool popups = db_get_b(0,m_szModuleName,TWITTER_KEY_POPUP_SIGNON,1) != 0;

	// if this isn't set, it will automatically not turn a tweet into a msg. probably should make the default that it does turn a tweet into a message
	bool tweetToMsg  = db_get_b(0,m_szModuleName,TWITTER_KEY_TWEET_TO_MSG,0) != 0;

	int poll_rate = db_get_dw(0,m_szModuleName,TWITTER_KEY_POLLRATE,80);

	for(unsigned int i=0;;i++)
	{

		if(m_iStatus != ID_STATUS_ONLINE)
			break;
//		if(i%10 == 0)
//			UpdateFriends();

		if(m_iStatus != ID_STATUS_ONLINE)
			break;
		UpdateStatuses(new_account,popups, tweetToMsg);

		if(m_iStatus != ID_STATUS_ONLINE)
			break;
		
		if(i%10 == 0)
			UpdateMessages(new_account);

		if(new_account) // Not anymore!
		{
			new_account = false;
			db_set_b(0,m_szModuleName,TWITTER_KEY_NEW,0);
		}

		if(m_iStatus != ID_STATUS_ONLINE)
			break;
		debugLogA("***** TwitterProto::MessageLoop going to sleep...");
		if(SleepEx(poll_rate*1000,true) == WAIT_IO_COMPLETION)
			break;
		debugLogA("***** TwitterProto::MessageLoop waking up...");

		popups = true;
	}

	{
		ScopedLock s(twitter_lock_);
		twit_.set_credentials("",L"",L"",L"",L"",L"", false);
	}
	debugLogA("***** Exiting TwitterProto::MessageLoop");
}

struct update_avatar
{
	update_avatar(MCONTACT hContact,const std::string &url) : hContact(hContact),url(url) {}
	MCONTACT hContact;
	std::string url;
};

/* void *p should always be a struct of type update_avatar */
void TwitterProto::UpdateAvatarWorker(void *p)
{
	if(p == 0)
		return;
	std::auto_ptr<update_avatar> data( static_cast<update_avatar*>(p));
	DBVARIANT dbv = {0};

	// db_get_s returns 0 when it suceeds, so if this suceeds it will return 0, or false.
	// therefore if it returns 1, or true, we want to return as there is no such user.
	// as a side effect, dbv now has the username in it i think
	if(db_get_ts(data->hContact,m_szModuleName,TWITTER_KEY_UN,&dbv))
		return;

	std::string ext = data->url.substr(data->url.rfind('.')); // finds the filetype of the avatar
	std::tstring filename = GetAvatarFolder() + _T('\\') + dbv.ptszVal + (TCHAR*)_A2T(ext.c_str()); // local filename and path
	db_free(&dbv);

	PROTO_AVATAR_INFORMATIONT ai = {sizeof(ai)};
	ai.hContact = data->hContact;
	ai.format = ProtoGetAvatarFormat(filename.c_str());

	if (ai.format == PA_FORMAT_UNKNOWN) {
		debugLogA("***** Update avatar: Terminated for this contact, extension format unknown for %s", data->url.c_str());
		return; // lets just ignore unknown formats... if not it crashes miranda. should probably speak to borkra about this.
	}
	
	_tcsncpy(ai.filename,filename.c_str(),MAX_PATH); // puts the local file name in the avatar struct, to a max of 260 chars (as of now)

	debugLogA("***** Updating avatar: %s", data->url.c_str());
	WaitForSingleObjectEx(avatar_lock_,INFINITE,true);
	if (CallService(MS_SYSTEM_TERMINATED, 0, 0)) // if miranda is shutting down...
	{
		debugLogA("***** Terminating avatar update early: %s", data->url.c_str());
		return;
	}

	if (save_url(hAvatarNetlib_, data->url, filename)) {
		db_set_s(data->hContact, m_szModuleName, TWITTER_KEY_AV_URL, data->url.c_str());
		ProtoBroadcastAck(data->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai, 0);
	}
	else ProtoBroadcastAck(data->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai, 0);
	
	ReleaseMutex(avatar_lock_);
	debugLogA("***** Done avatar: %s", data->url.c_str());
}

void TwitterProto::UpdateAvatar(MCONTACT hContact,const std::string &url,bool force)
{
	DBVARIANT dbv = {0};

	if( !force && (!db_get_s(hContact,m_szModuleName,TWITTER_KEY_AV_URL,&dbv) && url == dbv.pszVal))
	{
		debugLogA("***** Avatar already up-to-date: %s", url.c_str());
	}
	else
	{
		// TODO: more defaults (configurable?)
		if(url == "http://static.twitter.com/images/default_profile_normal.png")
		{
			PROTO_AVATAR_INFORMATIONT ai = {sizeof(ai),hContact};
			
			db_set_s(hContact,m_szModuleName,TWITTER_KEY_AV_URL,url.c_str());
			ProtoBroadcastAck(hContact,ACKTYPE_AVATAR,ACKRESULT_SUCCESS,&ai,0);
		}
		else
		{
			ForkThread(&TwitterProto::UpdateAvatarWorker, new update_avatar(hContact,url));
		}
	}

	db_free(&dbv);
}

void TwitterProto::UpdateFriends()
{
	try
	{
		ScopedLock s(twitter_lock_);
		std::vector<twitter_user> friends = twit_.get_friends();
		s.Unlock();
		for(std::vector<twitter_user>::iterator i=friends.begin(); i!=friends.end(); ++i)
		{
			if(i->username == twit_.get_username())
				continue;

			MCONTACT hContact = AddToClientList(i->username.c_str(),i->status.text.c_str());
			UpdateAvatar(hContact,i->profile_image_url);
		}
		disconnectionCount = 0; 
		debugLogA("***** Friends list updated");
	}
	catch(const bad_response &)
	{
		++disconnectionCount;
		debugLogA("***** UpdateFriends - Bad response from server, this has happened %d time(s)", disconnectionCount);
		if (disconnectionCount > 2) {
			debugLogA("***** UpdateFriends - Too many bad responses from the server, signing off");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating friends list, an error occurred: ")+e.what()).c_str());
		debugLogA("***** Error updating friends list: %s", e.what());
	}

}


LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		// Get the plugin data (we need the Popup service to do it)
		std::string *url = (std::string *)PUGetPluginData(hwnd);
		if (url != NULL) {
			//std::string url = profile_base_url("https://twitter.com/") + http::url_encode(dbv.pszVal);
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, reinterpret_cast<LPARAM>(url->c_str()));
		}
		// Intentionally no break here
	}
	case WM_CONTEXTMENU:
	{
		// After a click, destroy popup
		PUDeletePopup(hwnd);
		return TRUE;
	}
	case UM_FREEPLUGINDATA:
	{
		// After close, free
		std::string *url = (std::string *)PUGetPluginData(hwnd);
		delete url;
		return FALSE;
	}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
};

void TwitterProto::ShowContactPopup(MCONTACT hContact,const std::string &text,const std::string *url)
{
	if(!ServiceExists(MS_POPUP_ADDPOPUPT) || db_get_b(0,m_szModuleName,TWITTER_KEY_POPUP_SHOW,0) == 0)
	{
		return;
	}

	POPUPDATAT popup = {};
	popup.lchContact = hContact;
	popup.iSeconds = db_get_dw(0,m_szModuleName,TWITTER_KEY_POPUP_TIMEOUT,0);
	
	popup.colorBack = db_get_dw(0,m_szModuleName,TWITTER_KEY_POPUP_COLBACK,0);
	if(popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOW);
	popup.colorText = db_get_dw(0,m_szModuleName,TWITTER_KEY_POPUP_COLTEXT,0);
	if(popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOWTEXT);

	DBVARIANT dbv;
	if( !db_get_ts(hContact,"CList","MyHandle",&dbv) || !db_get_ts(hContact,m_szModuleName,TWITTER_KEY_UN,&dbv))
	{
		_tcsncpy(popup.lptzContactName,dbv.ptszVal,MAX_CONTACTNAME);
		db_free(&dbv);
	}

	if (url != NULL) {
		popup.PluginWindowProc = PopupWindowProc;
		popup.PluginData = (void *)url;
	}

	mbcs_to_tcs(CP_UTF8,text.c_str(),popup.lptzText,MAX_SECONDLINE);
	PUAddPopupT(&popup);
}

void TwitterProto::UpdateStatuses(bool pre_read, bool popups, bool tweetToMsg)
{
	try
	{
		ScopedLock s(twitter_lock_);
		twitter::status_list updates = twit_.get_statuses(200,since_id_);
		s.Unlock();
		if(!updates.empty()) {
			since_id_ = std::max(since_id_, updates[0].status.id);
		}

		for(twitter::status_list::reverse_iterator i=updates.rbegin(); i!=updates.rend(); ++i)
		{

			if(!pre_read && in_chat_)
				UpdateChat(*i);

			if(i->username == twit_.get_username())
				continue;

			MCONTACT hContact = AddToClientList(i->username.c_str(),"");
			UpdateAvatar(hContact,i->profile_image_url); // as UpdateFriends() doesn't work at the moment, i'm going to update the avatars here

			// i think we maybe should just do that DBEF_READ line instead of stopping ALL this code.  have to test.
			if (tweetToMsg) {
				DBEVENTINFO dbei = {sizeof(dbei)};
				dbei.pBlob = (BYTE*)(i->status.text.c_str());
				dbei.cbBlob = (int)i->status.text.size()+1;
				dbei.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
				dbei.flags = DBEF_UTF | DBEF_READ;
				dbei.timestamp = static_cast<DWORD>(i->status.time);
				dbei.szModule = m_szModuleName;
				db_event_add(hContact, &dbei);
			}

			db_set_utf(hContact,"CList","StatusMsg",i->status.text.c_str());

			if (!pre_read && popups) {
				std::stringstream url;
				url << std::string("https://twitter.com/") << i->username << std::string("/status/") << i->status.id;
				ShowContactPopup(hContact, i->status.text, new std::string(url.str()));
			}
		}

		db_pod_set(0,m_szModuleName,TWITTER_KEY_SINCEID,since_id_);
		disconnectionCount = 0;
		debugLogA("***** Status messages updated");
	}
	catch(const bad_response &)
	{
		++disconnectionCount;
		debugLogA("***** UpdateStatuses - Bad response from server, this has happened %d time(s)", disconnectionCount);
		if (disconnectionCount > 2) {
			debugLogA("***** UpdateStatuses - Too many bad responses from the server, signing off");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating status messages, an error occurred: ")
			+e.what()).c_str());
		debugLogA("***** Error updating status messages: %s", e.what());
	}
}

void TwitterProto::UpdateMessages(bool pre_read)
{
	try
	{
		ScopedLock s(twitter_lock_);
		twitter::status_list messages = twit_.get_direct(dm_since_id_);
		s.Unlock();

		if(messages.size())
			dm_since_id_ = std::max(dm_since_id_, messages[0].status.id);

		for(twitter::status_list::reverse_iterator i=messages.rbegin(); i!=messages.rend(); ++i)
		{
			MCONTACT hContact = AddToClientList(i->username.c_str(),"");

			PROTORECVEVENT recv = { 0 };
			recv.flags = PREF_UTF;
			if(pre_read)
				recv.flags |= PREF_CREATEREAD;
			recv.szMessage = const_cast<char*>(i->status.text.c_str());
			recv.timestamp = static_cast<DWORD>(i->status.time);
			ProtoChainRecvMsg(hContact, &recv);
		}

		db_pod_set(0,m_szModuleName,TWITTER_KEY_DMSINCEID,dm_since_id_);
		disconnectionCount = 0;
		debugLogA("***** Direct messages updated");
	}
	catch(const bad_response &)
	{
		++disconnectionCount;
		debugLogA("***** UpdateMessages - Bad response from server, this has happened %d time(s)", disconnectionCount);
		if (disconnectionCount > 2) {
			debugLogA("***** UpdateMessages - Too many bad responses from the server, signing off");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating direct messages, an error occurred: ")+e.what()).c_str());
		debugLogA("***** Error updating direct messages: %s", e.what());
	}
}

void TwitterProto::resetOAuthKeys() {
	db_unset(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK);
	db_unset(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET);
	db_unset(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK);
	db_unset(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK_SECRET);
	db_unset(0,m_szModuleName,TWITTER_KEY_OAUTH_PIN);
}
