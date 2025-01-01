/*
Copyright © 2012-25 Miranda NG team
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

class CTwitterProto;

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

struct AsyncHttpRequest : public MTHttpRequest<CTwitterProto>
{
	AsyncHttpRequest(int type, const char *szUrl, MTHttpRequestHandler pHandler = nullptr);
};

class CTwitterProto : public PROTO<CTwitterProto>
{
	SESSION_INFO *m_si;

	// http server thread routines
	bool m_bTerminated;
	mir_cs m_csHttpQueue;
	HANDLE m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;

	void Push(AsyncHttpRequest *req);
	void Execute(AsyncHttpRequest *req);

	void BeginConnection();
	void OnLoggedIn();
	void OnLoggedFail();

	int  m_hWorkerThreadId;
	void __cdecl ServerThread(void *);

	HNETLIBCONN m_hConnHttp;

	// internal data
	CMStringA m_szUserName;
	CMStringA m_szMyId;
	CMStringA m_szPassword;
	CMStringA m_szConsumerKey;
	CMStringA m_szConsumerSecret;
	CMStringA m_szAccessToken;
	CMStringA m_szAccessTokenSecret;
	CMStringA m_szPin;

	CMStringW GetAvatarFolder();

	mir_cs avatar_lock_;
	mir_cs twitter_lock_;

	OBJLIST<CChatMark> m_arChatMarks;

	HNETLIBUSER hAvatarNetlib_;
	HANDLE hMsgLoop_;

	twitter_id since_id_;
	twitter_id dm_since_id_;

	////////////////////////////////////////////////////////////////////////////////////////
	// utils

	twitter_id getId(const char *szSetting);
	void setId(const char *szSetting, twitter_id id);

	////////////////////////////////////////////////////////////////////////////////////////
	// oauth

	uint8_t code_verifier[32];
	CMStringA code_challenge;

	void RequestOauthAuth();
	void RequestOauthToken(const char *szPin = nullptr);
	void ResetOauthKeys();

	void Oauth2RequestAuth(MHttpResponse *, AsyncHttpRequest *);
	void Oauth2RequestToken(MHttpResponse *, AsyncHttpRequest *);

	CMStringA OAuthWebRequestSubmit(const CMStringA &url, const char *httpMethod, const char *postData);

	CMStringA UrlGetQuery(const CMStringA &url);

	CMStringA BuildSignedOAuthParameters(const CMStringA &requestParameters, const CMStringA &url, const char *httpMethod, const char *postData);

	CMStringA OAuthCreateNonce();
	CMStringA OAuthCreateSignature(const CMStringA &signatureBase, const CMStringA &consumerSecret, const CMStringA &requestTokenSecret);

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

	void UpdateChat(const twitter_user &update);
	void AddChatContact(const char *name, const char *nick = nullptr);
	void DeleteChatContact(const char *name);
	void SetChatStatus(int);

	bool get_info(const CMStringA &name, twitter_user *);
	bool get_info_by_email(const CMStringA &email, twitter_user *);

	bool add_friend(const CMStringA &name, twitter_user &u);
	void remove_friend(const CMStringA &name);

	void mark_read(MCONTACT hContact, const CMStringA &msgId);

	void set_status(const CMStringA &text);
	void send_direct(const CMStringA &name, const CMStringA &text);

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

	int      SendMsg(MCONTACT, MEVENT, const char *) override;

	int      SetStatus(int) override;

	HANDLE   GetAwayMsg(MCONTACT) override;

	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;

	void UpdateSettings();

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

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
	int  __cdecl OnOptionsInit(WPARAM,LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM,LPARAM);

	void __cdecl SendTweetWorker(void *);

	//////////////////////////////////////////////////////////////////////////////////////
	// Threads

	void __cdecl AddToListWorker(void *p);
	void __cdecl DoSearch(void *);
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
