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

#include "proto.h"
//#include "tc2.h"
#include "twitter.h"

void CALLBACK TwitterProto::APC_callback(ULONG_PTR p)
{
	reinterpret_cast<TwitterProto*>(p)->LOG("***** Executing APC");
}

template<typename T>
inline static T db_pod_get(HANDLE hContact,const char *module,const char *setting,
	T errorValue)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;

	cgs.szModule  = module;
	cgs.szSetting = setting;
	cgs.pValue    = &dbv;
	if(CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs))
		return errorValue;

	// TODO: remove this, it's just a temporary workaround
	if(dbv.type == DBVT_DWORD)
		return dbv.dVal;

	if(dbv.cpbVal != sizeof(T))
		return errorValue;
	return *reinterpret_cast<T*>(dbv.pbVal);
}

template<typename T>
inline static INT_PTR db_pod_set(HANDLE hContact,const char *module,const char *setting,
	T val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule     = module;
	cws.szSetting    = setting;
	cws.value.type   = DBVT_BLOB;
	cws.value.cpbVal = sizeof(T);
	cws.value.pbVal  = reinterpret_cast<BYTE*>(&val);
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

void TwitterProto::SignOn(void*)
{
	LOG("***** Beginning SignOn process");
	WaitForSingleObject(&signon_lock_,INFINITE);

	// Kill the old thread if it's still around
	// this doesn't seem to work.. should we wait infinitely?
	if(hMsgLoop_)
	{
		LOG("***** Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback,hMsgLoop_,(ULONG_PTR)this);
		LOG("***** Waiting for old MessageLoop to exit");
		//WaitForSingleObject(hMsgLoop_,INFINITE);
		WaitForSingleObject(hMsgLoop_,180000);
		CloseHandle(hMsgLoop_);
	}
	if(NegotiateConnection()) // Could this be? The legendary Go Time??
	{
		if(!in_chat_ && db_byte_get(0,m_szModuleName,TWITTER_KEY_CHATFEED,0))
			OnJoinChat(0,true);
		
		SetAllContactStatuses(ID_STATUS_ONLINE);
		hMsgLoop_ = ForkThreadEx(&TwitterProto::MessageLoop,this);
	}

	ReleaseMutex(signon_lock_);
	LOG("***** SignOn complete");
}

bool TwitterProto::NegotiateConnection()
{
	LOG("***** Negotiating connection with Twitter");
	disconnectionCount = 0;

	// saving the current status to a temp var
	int old_status = m_iStatus;
	DBVARIANT dbv;

	wstring oauthToken;
	wstring oauthTokenSecret;	
	wstring oauthAccessToken;
	wstring oauthAccessTokenSecret;
	string screenName;

	int dbTOK = DBGetContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK,&dbv);
	if (!dbTOK) {
		oauthToken = dbv.pwszVal;
		DBFreeVariant(&dbv);
		//WLOG("**NegotiateConnection - we have an oauthToken already in the db - %s", oauthToken);
	}
 
	int dbTOKSec = DBGetContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK_SECRET,&dbv);
	if (!dbTOKSec) {
		oauthTokenSecret = dbv.pwszVal;
		DBFreeVariant(&dbv);
		//WLOG("**NegotiateConnection - we have an oauthTokenSecret already in the db - %s", oauthTokenSecret);
	}

	int dbName = DBGetContactSettingString(0,m_szModuleName,TWITTER_KEY_NICK,&dbv);
	if (!dbName) {
		screenName = dbv.pszVal;
		DBFreeVariant(&dbv);
		//WLOG("**NegotiateConnection - we have a username already in the db - %s", screenName);
	}
	else {
		dbName = DBGetContactSettingString(0,m_szModuleName,TWITTER_KEY_UN,&dbv);
		if (!dbName) {
			screenName = dbv.pszVal;
			DBWriteContactSettingString(0,m_szModuleName,TWITTER_KEY_NICK,dbv.pszVal);
			//WLOG("**NegotiateConnection - we have a username already in the db - %s", screenName);
		}
		DBFreeVariant(&dbv);
	}


	if((oauthToken.size() <= 1) || (oauthTokenSecret.size() <= 1) ) {
		// first, reset all the keys so we can start fresh
		resetOAuthKeys();
		LOG("**NegotiateConnection - Reset OAuth Keys");

		//twit_.set_credentials(ConsumerKey, ConsumerSecret, oauthAccessToken, oauthAccessTokenSecret, L"", false);
		// i think i was doin the wrong thing here.. i was setting the credentials as oauthAccessToken instead of oauthToken
		// have to test..
		LOG("**NegotiateConnection - Setting Consumer Keys...");
		/*WLOG("**NegotiateConnection - sending set_cred: consumerKey is %s", ConsumerKey);
		WLOG("**NegotiateConnection - sending set_cred: consumerSecret is %s", ConsumerSecret);
		WLOG("**NegotiateConnection - sending set_cred: oauthToken is %s", oauthToken);
		WLOG("**NegotiateConnection - sending set_cred: oauthTokenSecret is %s", oauthTokenSecret);
		LOG("**NegotiateConnection - sending set_cred: no pin");*/
		twit_.set_credentials("", ConsumerKey, ConsumerSecret, oauthToken, oauthTokenSecret, L"", false);
		LOG("**NegotiateConnection - Requesting oauthTokens");
		http::response resp = twit_.request_token();

		//wstring rdata_WSTR(resp.data.length(),L' ');
		//std::copy(resp.data.begin(), resp.data.end(), rdata_WSTR.begin());
		wstring rdata_WSTR = UTF8ToWide(resp.data);

		//WLOG("**NegotiateConnection - REQUEST TOKEN IS %s", rdata_WSTR);
		OAuthParameters response = twit_.ParseQueryString(rdata_WSTR);
		oauthToken = response[L"oauth_token"];
		oauthTokenSecret = response[L"oauth_token_secret"];
		//WLOG("**NegotiateConnection - oauthToken is %s", oauthToken);
		//WLOG("**NegotiateConnection - oauthTokenSecret is %s", oauthTokenSecret);

		if (oauthToken.length() < 1) {
			ShowPopup("OAuth Tokens not received, check your internet connection?", 1);
			LOG("**NegotiateConnection - OAuth tokens not received, stopping before we open the web browser..");
			return false;
		}

		//write those bitches to the db foe latta
		DBWriteContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK,oauthToken.c_str());
		DBWriteContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK_SECRET,oauthTokenSecret.c_str());
		
		// this looks like bad code.. can someone clean this up please?  or confirm that it's ok
		wchar_t buf[1024] = {};
		swprintf_s(buf, SIZEOF(buf), AuthorizeUrl.c_str(), oauthToken.c_str());

		WLOG("**NegotiateConnection - Launching %s", buf);
		ShellExecute(NULL, L"open", buf, NULL, NULL, SW_SHOWNORMAL);
		
		ShowPinDialog();
	}

	if (!DBGetContactSettingTString(NULL,m_szModuleName,TWITTER_KEY_GROUP,&dbv)) {
		CallService( MS_CLIST_GROUPCREATE, 0, (LPARAM)dbv.ptszVal );
		DBFreeVariant(&dbv);	
	}

	bool realAccessTok = false;
	bool realAccessTokSecret = false;

	// remember, dbTOK is 0 (false) if the db setting has returned something
	dbTOK = DBGetContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK,&dbv);
	if (!dbTOK) { 
		oauthAccessToken = dbv.pwszVal;
		DBFreeVariant(&dbv);
		// this bit is saying "if we have found the db key, but it contains no data, then set dbTOK to 1"
		if (oauthAccessToken.size() > 1) { 
			realAccessTok = true; 
			//WLOG("**NegotiateConnection - we have an oauthAccessToken already in the db - %s", oauthAccessToken); 
		}
		else { LOG("**NegotiateConnection - oauthAccesToken too small? this is.. weird."); }
	}

	dbTOKSec = DBGetContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET,&dbv);
	if (!dbTOKSec) { 
		oauthAccessTokenSecret = dbv.pwszVal;
		DBFreeVariant(&dbv);
		if (oauthAccessTokenSecret.size() > 1) { 
			realAccessTokSecret = true; 
			//WLOG("**NegotiateConnection - we have an oauthAccessTokenSecret already in the db - %s", oauthAccessTokenSecret); 
		}
		else { LOG("**NegotiateConnection - oauthAccessTokenSecret too small? weird"); }
	}

	if (!realAccessTok || !realAccessTokSecret) {  // if we don't have one of these beasties then lets go get 'em!
		wstring pin;
		LOG("**NegotiateConnection - either the accessToken or accessTokenSecret was not there..");
		if (!DBGetContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_PIN,&dbv)) {
			pin = dbv.pwszVal;
			//WLOG("**NegotiateConnection - we have an pin already in the db - %s", pin);
			DBFreeVariant(&dbv);
		}
		else {
			ShowPopup(TranslateT("OAuth variables are out of sequence, they have been reset.  Please reconnect and reauthorise Miranda to Twitter.com (do the PIN stuff again)"));
			LOG("**NegotiateConnection - We don't have a PIN?  this doesn't make sense.  Resetting OAuth keys and setting offline.");
			resetOAuthKeys();

			ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_FAILED,
			(HANDLE)old_status,m_iStatus);

			// Set to offline
			old_status = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
				(HANDLE)old_status,m_iStatus);

			return false;
		}

		LOG("**NegotiateConnection - Setting Consumer Keys and PIN...");
		/*WLOG("**NegotiateConnection - sending set_cred: consumerKey is %s", ConsumerKey);
		WLOG("**NegotiateConnection - sending set_cred: consumerSecret is %s", ConsumerSecret);
		WLOG("**NegotiateConnection - sending set_cred: oauthToken is %s", oauthToken);
		WLOG("**NegotiateConnection - sending set_cred: oauthTokenSecret is %s", oauthTokenSecret);
		WLOG("**NegotiateConnection - sending set_cred: pin is %s", pin);*/

		twit_.set_credentials("", ConsumerKey, ConsumerSecret, oauthToken, oauthTokenSecret, pin, false);

		LOG("**NegotiateConnection - requesting access tokens...");
		http::response accessResp = twit_.request_access_tokens();
		if (accessResp.code != 200) {
			LOG("**NegotiateConnection - Failed to get Access Tokens, HTTP response code is: %d", accessResp.code);
			ShowPopup(TranslateT("Failed to get Twitter Access Tokens, please go offline and try again.  If this keeps happening, check your internet connection."));

			resetOAuthKeys();

			ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_FAILED,
			(HANDLE)old_status,m_iStatus);

			// Set to offline
			old_status = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
				(HANDLE)old_status,m_iStatus);

			return false;
		}
		else {
			LOG("**NegotiateConnection - Successfully retrieved Access Tokens");

			wstring rdata_WSTR2 = UTF8ToWide(accessResp.data);
			//WLOG("**NegotiateConnection - accessToken STring is %s", rdata_WSTR2);
		
			OAuthParameters accessTokenParameters = twit_.ParseQueryString(rdata_WSTR2);

			oauthAccessToken = accessTokenParameters[L"oauth_token"];
			//WLOG("**NegotiateConnection - oauthAccessToken is %s", oauthAccessToken);

			oauthAccessTokenSecret = accessTokenParameters[L"oauth_token_secret"];
			//WLOG("**NegotiateConnection - oauthAccessTokenSecret is %s", oauthAccessTokenSecret);

			screenName = WideToUTF8(accessTokenParameters[L"screen_name"]);
			LOG("**NegotiateConnection - screen name is %s", screenName.c_str());
	
			//save em
			DBWriteContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK,oauthAccessToken.c_str());
			DBWriteContactSettingWString(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET,oauthAccessTokenSecret.c_str());
			DBWriteContactSettingString(0,m_szModuleName,TWITTER_KEY_NICK,screenName.c_str());
			DBWriteContactSettingString(0,m_szModuleName,TWITTER_KEY_UN,screenName.c_str());
		}
	}

/*	if( !DBGetContactSettingString(0,m_szModuleName,TWITTER_KEY_PASS,&dbv) ) {
		CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
			reinterpret_cast<LPARAM>(dbv.pszVal));
		pass = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else {
		ShowPopup(TranslateT("Please enter a password."));
		return false;
	}*/

	if( !DBGetContactSettingString(0,m_szModuleName,TWITTER_KEY_BASEURL,&dbv) )
	{
		ScopedLock s(twitter_lock_);
		twit_.set_base_url(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	LOG("**NegotiateConnection - Setting Consumer Keys and verifying creds...");
	/*WLOG("**NegotiateConnection - sending set_cred: consumerKey is %s", ConsumerKey);
	WLOG("**NegotiateConnection - sending set_cred: consumerSecret is %s", ConsumerSecret);
	WLOG("**NegotiateConnection - sending set_cred: oauthAccessToken is %s", oauthAccessToken);
	WLOG("**NegotiateConnection - sending set_cred: oauthAccessTokenSecret is %s", oauthAccessTokenSecret);
	LOG("**NegotiateConnection - sending set_cred: no pin");*/

	if (screenName.empty()) {
		ShowPopup(TranslateT("You're missing the Nick key in the database.  This isn't really a big deal, but you'll notice some minor quirks (self contact in list, no group chat outgoing message highlighting, etc).  To fix it either add it manually or reset your twitter account in the miranda account options"));
		LOG("**NegotiateConnection - Missing the Nick key in the database.  Everything will still work, but it's nice to have");
	}

	bool success;
	{		
		ScopedLock s(twitter_lock_);

		success = twit_.set_credentials(screenName, ConsumerKey, ConsumerSecret, oauthAccessToken, oauthAccessTokenSecret, L"", true);
	}

	if(!success) {
		//ShowPopup(TranslateT("Something went wrong with authorisation, OAuth keys have been reset.  Please try to reconnect.  If problems persist, please se your doctor"));
		LOG("**NegotiateConnection - Verifying credentials failed!  No internet maybe?");

		//resetOAuthKeys();
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_FAILED,
			(HANDLE)old_status,m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		return false;
	}
	else {
		m_iStatus = m_iDesiredStatus;

		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);
		return true;
	}
}


void TwitterProto::MessageLoop(void*)
{
	LOG("***** Entering Twitter::MessageLoop");

	since_id_    = db_pod_get<twitter_id>(0,m_szModuleName,TWITTER_KEY_SINCEID,0);
	dm_since_id_ = db_pod_get<twitter_id>(0,m_szModuleName,TWITTER_KEY_DMSINCEID,0);

	bool new_account = db_byte_get(0,m_szModuleName,TWITTER_KEY_NEW,1) != 0;
	bool popups      = db_byte_get(0,m_szModuleName,TWITTER_KEY_POPUP_SIGNON,1) != 0;

	// if this isn't set, it will automatically not turn a tweet into a msg. probably should make the default that it does turn a tweet into a message
	bool tweetToMsg  = db_byte_get(0,m_szModuleName,TWITTER_KEY_TWEET_TO_MSG,0) != 0;

	int poll_rate = db_dword_get(0,m_szModuleName,TWITTER_KEY_POLLRATE,80);

	for(unsigned int i=0;;i++)
	{

		if(m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		if(i%4 == 0)
			UpdateFriends();

		if(m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		UpdateStatuses(new_account,popups, tweetToMsg);

		if(m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		UpdateMessages(new_account);

		if(new_account) // Not anymore!
		{
			new_account = false;
			DBWriteContactSettingByte(0,m_szModuleName,TWITTER_KEY_NEW,0);
		}

		if(m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		LOG("***** TwitterProto::MessageLoop going to sleep...");
		if(SleepEx(poll_rate*1000,true) == WAIT_IO_COMPLETION)
			goto exit;
		LOG("***** TwitterProto::MessageLoop waking up...");

		popups = true;
	}

exit:
	{
		ScopedLock s(twitter_lock_);
		twit_.set_credentials("",L"",L"",L"",L"",L"", false);
	}
	LOG("***** Exiting TwitterProto::MessageLoop");
}

struct update_avatar
{
	update_avatar(HANDLE hContact,const std::string &url) : hContact(hContact),url(url) {}
	HANDLE hContact;
	std::string url;
};

void TwitterProto::UpdateAvatarWorker(void *p)
{
	if(p == 0)
		return;
	std::auto_ptr<update_avatar> data( static_cast<update_avatar*>(p) );
	DBVARIANT dbv;

	if(DBGetContactSettingString(data->hContact,m_szModuleName,TWITTER_KEY_UN,&dbv))
		return;

	std::string ext = data->url.substr(data->url.rfind('.'));
	std::string filename = GetAvatarFolder() + '\\' + dbv.pszVal + ext;
	DBFreeVariant(&dbv);

	PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
	ai.hContact = data->hContact;
	ai.format = ext_to_format(ext);

	if (ai.format == PA_FORMAT_UNKNOWN) {
		LOG("***** Update avatar: Terminated for this contact, extension format unknown for %s", data->url.c_str());
		return; // lets just ignore unknown formats... if not it crashes miranda. should probably speak to borkra about this.
	}
	
	strncpy(ai.filename,filename.c_str(),MAX_PATH);

	LOG("***** Updating avatar: %s",data->url.c_str());
	WaitForSingleObjectEx(avatar_lock_,INFINITE,true);
	if(CallService(MS_SYSTEM_TERMINATED,0,0))
	{
		LOG("***** Terminating avatar update early: %s",data->url.c_str());
		return;
	}

	if(save_url(hAvatarNetlib_,data->url,filename))
	{
		DBWriteContactSettingString(data->hContact,m_szModuleName,TWITTER_KEY_AV_URL,
			data->url.c_str());
		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_AVATAR,
			ACKRESULT_SUCCESS,&ai,0);
	}
	else
		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_AVATAR,
			ACKRESULT_FAILED, &ai,0);
	ReleaseMutex(avatar_lock_);
	LOG("***** Done avatar: %s",data->url.c_str());
}

void TwitterProto::UpdateAvatar(HANDLE hContact,const std::string &url,bool force)
{
	DBVARIANT dbv;

	if( !force &&
	  ( !DBGetContactSettingString(hContact,m_szModuleName,TWITTER_KEY_AV_URL,&dbv) &&
	    url == dbv.pszVal) )
	{
		LOG("***** Avatar already up-to-date: %s",url.c_str());
	}
	else
	{
		// TODO: more defaults (configurable?)
		if(url == "http://static.twitter.com/images/default_profile_normal.png")
		{
			PROTO_AVATAR_INFORMATION ai = {sizeof(ai),hContact};
			
			db_string_set(hContact,m_szModuleName,TWITTER_KEY_AV_URL,url.c_str());
			ProtoBroadcastAck(m_szModuleName,hContact,ACKTYPE_AVATAR,
				ACKRESULT_SUCCESS,&ai,0);
		}
		else
		{
			ForkThread(&TwitterProto::UpdateAvatarWorker, this,
				new update_avatar(hContact,url));
		}
	}

	DBFreeVariant(&dbv);
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

			HANDLE hContact = AddToClientList(i->username.c_str(),i->status.text.c_str());
			UpdateAvatar(hContact,i->profile_image_url);
		}
		disconnectionCount = 0; 
		LOG("***** Friends list updated");
	}
	catch(const bad_response &)
	{
		++disconnectionCount;
		LOG("***** UpdateFriends - Bad response from server, this has happened %d time(s)", disconnectionCount);
		if (disconnectionCount > 2) {
			LOG("***** UpdateFriends - Too many bad responses from the server, signing off");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating friends list, an error occurred: ")
			+e.what()).c_str() );
		LOG("***** Error updating friends list: %s",e.what());
	}

}

void TwitterProto::ShowContactPopup(HANDLE hContact,const std::string &text)
{
	if(!ServiceExists(MS_POPUP_ADDPOPUPT) || DBGetContactSettingByte(0,
		m_szModuleName,TWITTER_KEY_POPUP_SHOW,0) == 0)
	{
		return;
	}

	POPUPDATAT popup = {};
	popup.lchContact = hContact;
	popup.iSeconds = db_dword_get(0,m_szModuleName,TWITTER_KEY_POPUP_TIMEOUT,0);
	
	popup.colorBack = db_dword_get(0,m_szModuleName,TWITTER_KEY_POPUP_COLBACK,0);
	if(popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOW);
	popup.colorText = db_dword_get(0,m_szModuleName,TWITTER_KEY_POPUP_COLTEXT,0);
	if(popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOWTEXT);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(hContact,"CList","MyHandle",&dbv) ||
		!DBGetContactSettingString(hContact,m_szModuleName,TWITTER_KEY_UN,&dbv) )
	{
		mbcs_to_tcs(CP_UTF8,dbv.pszVal,popup.lptzContactName,MAX_CONTACTNAME);
		DBFreeVariant(&dbv);
	}

	mbcs_to_tcs(CP_UTF8,text.c_str(),popup.lptzText,MAX_SECONDLINE);
	CallService(MS_POPUP_ADDPOPUPT,reinterpret_cast<WPARAM>(&popup),0);
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

			HANDLE hContact = AddToClientList(i->username.c_str(),"");

			// i think we maybe should just do that DBEF_READ line instead of stopping ALL this code.  have to test.
			if (tweetToMsg) {
				DBEVENTINFO dbei = {sizeof(dbei)};
			
				dbei.pBlob = (BYTE*)(i->status.text.c_str());
				dbei.cbBlob = i->status.text.size()+1;
				dbei.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
				dbei.flags = DBEF_UTF;
				dbei.flags = DBEF_READ; // i had commented this line out.. can't remember why :(  might need to do it again, uncommented for mrQQ for testing
				dbei.timestamp = static_cast<DWORD>(i->status.time);
				dbei.szModule = m_szModuleName;
				CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
			}

			DBWriteContactSettingUTF8String(hContact,"CList","StatusMsg",
				i->status.text.c_str());

			if(!pre_read && popups)
				ShowContactPopup(hContact,i->status.text);
		}

		db_pod_set(0,m_szModuleName,TWITTER_KEY_SINCEID,since_id_);
		disconnectionCount = 0;
		LOG("***** Status messages updated");
	}
	catch(const bad_response &)
	{
		++disconnectionCount;
		LOG("***** UpdateStatuses - Bad response from server, this has happened %d time(s)", disconnectionCount);
		if (disconnectionCount > 2) {
			LOG("***** UpdateStatuses - Too many bad responses from the server, signing off");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating status messages, an error occurred: ")
			+e.what()).c_str() );
		LOG("***** Error updating status messages: %s",e.what());
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
			HANDLE hContact = AddToClientList(i->username.c_str(),"");

			PROTORECVEVENT recv = {};
			CCSDATA ccs = {};

			recv.flags = PREF_UTF;
			if(pre_read)
				recv.flags |= PREF_CREATEREAD;
			recv.szMessage = const_cast<char*>(i->status.text.c_str());
			recv.timestamp = static_cast<DWORD>(i->status.time);

			ccs.hContact = hContact;
			ccs.szProtoService = PSR_MESSAGE;
			ccs.wParam = ID_STATUS_ONLINE;
			ccs.lParam = reinterpret_cast<LPARAM>(&recv);
			CallService(MS_PROTO_CHAINRECV,0,reinterpret_cast<LPARAM>(&ccs));
		}

		db_pod_set(0,m_szModuleName,TWITTER_KEY_DMSINCEID,dm_since_id_);
		disconnectionCount = 0;
		LOG("***** Direct messages updated");
	}
	catch(const bad_response &)
	{
		++disconnectionCount;
		LOG("***** UpdateMessages - Bad response from server, this has happened %d time(s)", disconnectionCount);
		if (disconnectionCount > 2) {
			LOG("***** UpdateMessages - Too many bad responses from the server, signing off");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating direct messages, an error occurred: ")
			+e.what()).c_str() );
		LOG("***** Error updating direct messages: %s",e.what());
	}
}

void TwitterProto::resetOAuthKeys() {
	DBDeleteContactSetting(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK);
	DBDeleteContactSetting(0,m_szModuleName,TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET);
	DBDeleteContactSetting(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK);
	DBDeleteContactSetting(0,m_szModuleName,TWITTER_KEY_OAUTH_TOK_SECRET);
	DBDeleteContactSetting(0,m_szModuleName,TWITTER_KEY_OAUTH_PIN);
}