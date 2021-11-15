/*
Copyright © 2012-21 Miranda NG team
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

#pragma once

#include "..\..\..\..\miranda-private-keys\Twitter\oauth.dev.h"

typedef __int64 twitter_id;

struct twitter_status
{
	std::string text;
	twitter_id id;
	time_t time;
};

struct twitter_user
{
	std::string username;
	std::string real_name;
	std::string profile_image_url;
	twitter_status status;
};

time_t parse_time(const CMStringA &str);

struct AsyncHttpRequest : public MHttpRequest
{
	AsyncHttpRequest(int type, const char *szUrl);
};

struct CChatMark
{
	CChatMark(MEVENT _p1, const CMStringA &_p2) :
		hEvent(_p1),
		szId(_p2)
	{
	}

	MEVENT hEvent;
	CMStringA szId;
};

class CTwitterProto : public PROTO<CTwitterProto>
{
	ptrA m_szChatId;

	http::response request_token();
	http::response request_access_tokens();

	bool get_info(const CMStringA &name, twitter_user *);
	bool get_info_by_email(const CMStringA &email, twitter_user *);

	bool add_friend(const CMStringA &name, twitter_user &u);
	void remove_friend(const CMStringA &name);

	void mark_read(MCONTACT hContact, const CMStringA &msgId);

	void set_status(const CMStringA &text);
	void send_direct(const CMStringA &name, const CMStringA &text);

	http::response Execute(AsyncHttpRequest *req);

	CMStringA m_szUserName;
	CMStringA m_szMyId;
	CMStringA m_szPassword;
	CMStringA m_szConsumerKey;
	CMStringA m_szConsumerSecret;
	CMStringA m_szAccessToken;
	CMStringA m_szAccessTokenSecret;
	CMStringA m_szPin;

	CMStringW GetAvatarFolder();

	mir_cs signon_lock_;
	mir_cs avatar_lock_;
	mir_cs twitter_lock_;

	OBJLIST<CChatMark> m_arChatMarks;

	HNETLIBUSER hAvatarNetlib_;
	HANDLE hMsgLoop_;

	twitter_id since_id_;
	twitter_id dm_since_id_;

	bool in_chat_;

	int disconnectionCount;

	// OAuthWebRequest used for all OAuth related queries
	//
	// consumerKey and consumerSecret - must be provided for every call, they identify the application
	// oauthToken and oauthTokenSecret - need to be provided for every call, except for the first token request before authorizing
	// pin - only used during authorization, when the user enters the PIN they received from the CTwitterProto website
	
	CMStringA OAuthWebRequestSubmit(const CMStringA &url, const char *httpMethod, const char *postData);

	CMStringA UrlGetQuery(const CMStringA &url);

	CMStringA BuildSignedOAuthParameters(const CMStringA &requestParameters, const CMStringA &url, const char *httpMethod, const char *postData);

	CMStringA OAuthCreateNonce();
	CMStringA OAuthCreateSignature(const CMStringA &signatureBase, const CMStringA &consumerSecret, const CMStringA &requestTokenSecret);

	HNETLIBCONN m_hConnHttp;
	void Disconnect(void) { if (m_hConnHttp) Netlib_CloseHandle(m_hConnHttp); m_hConnHttp = nullptr; }

	bool NegotiateConnection();

	void UpdateStatuses(bool pre_read, bool popups, bool tweetToMsg);
	void UpdateMessages(bool pre_read);
	void UpdateFriends();
	void UpdateAvatar(MCONTACT, const CMStringA &, bool force = false);

	int  ShowPinDialog();
	void ShowPopup(const wchar_t *, int Error = 0);
	void ShowPopup(const char *, int Error = 0);
	void ShowContactPopup(MCONTACT, const CMStringA &, const CMStringA *);

	bool IsMyContact(MCONTACT, bool include_chat = false);
	MCONTACT UsernameToHContact(const char *);
	MCONTACT AddToClientList(const char *, const char *);
	MCONTACT FindContactById(const char *);

	static void CALLBACK APC_callback(ULONG_PTR p);

	void UpdateChat(const twitter_user &update);
	void AddChatContact(const char *name, const char *nick = nullptr);
	void DeleteChatContact(const char *name);
	void SetChatStatus(int);

	void resetOAuthKeys();

public:
	CTwitterProto(const char*,const wchar_t*);
	~CTwitterProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT AddToList(int,PROTOSEARCHRESULT *) override;

	INT_PTR  GetCaps(int, MCONTACT = 0) override;
	int      GetInfo(MCONTACT, int) override;

	HANDLE   SearchBasic(const wchar_t *) override;
	HANDLE   SearchByEmail(const wchar_t *) override;

	int      SendMsg(MCONTACT, int, const char *) override;

	int      SetStatus(int) override;

	HANDLE   GetAwayMsg(MCONTACT) override;

	void     OnModulesLoaded() override;

	void UpdateSettings();

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM,LPARAM);
	INT_PTR __cdecl ReplyToTweet(WPARAM,LPARAM);
	INT_PTR __cdecl VisitHomepage(WPARAM,LPARAM);
	INT_PTR __cdecl GetAvatar(WPARAM,LPARAM);
	INT_PTR __cdecl SetAvatar(WPARAM,LPARAM);

	INT_PTR __cdecl OnJoinChat(WPARAM,LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM,LPARAM);

	INT_PTR __cdecl OnTweet(WPARAM,LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// Events

	int  __cdecl OnBuildStatusMenu(WPARAM, LPARAM);
	int  __cdecl OnChatOutgoing(WPARAM, LPARAM);
	int  __cdecl OnContactDeleted(WPARAM,LPARAM);
	int  __cdecl OnMarkedRead(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM,LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM,LPARAM);

	void __cdecl SendTweetWorker(void *);

	//////////////////////////////////////////////////////////////////////////////////////
	// Threads

	void __cdecl AddToListWorker(void *p);
	void __cdecl DoSearch(void *);
	void __cdecl SignOn(void *);
	void __cdecl MessageLoop(void *);
	void __cdecl GetAwayMsgWorker(void *);
	void __cdecl UpdateAvatarWorker(void *);
	void __cdecl UpdateInfoWorker(void *);
};

struct CMPlugin : public ACCPROTOPLUGIN<CTwitterProto>
{
	CMPlugin();

	int Load() override;
};
