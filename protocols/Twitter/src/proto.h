/*
Copyright © 2012-15 Miranda NG team
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

#include "utility.h"

class TwitterProto : public PROTO<TwitterProto>
{
public:
	TwitterProto(const char*,const TCHAR*);
	~TwitterProto();

	inline const char * ModuleName() const
	{
		return m_szModuleName;
	}

	//PROTO_INTERFACE

	virtual	MCONTACT  __cdecl AddToList(int,PROTOSEARCHRESULT *);

	virtual	DWORD_PTR __cdecl GetCaps(int, MCONTACT = 0);
	virtual	int       __cdecl GetInfo(MCONTACT, int);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR *);
	virtual	HANDLE    __cdecl SearchByEmail(const TCHAR *);

	virtual	int       __cdecl RecvMsg(MCONTACT, PROTORECVEVENT *);
	virtual	int       __cdecl SendMsg(MCONTACT, int, const char *);

	virtual	int       __cdecl SetStatus(int);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE,WPARAM,LPARAM);

	void UpdateSettings();

	// Services
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM,LPARAM);
	INT_PTR __cdecl GetName(WPARAM,LPARAM);
	INT_PTR __cdecl GetStatus(WPARAM,LPARAM);
	INT_PTR __cdecl ReplyToTweet(WPARAM,LPARAM);
	INT_PTR __cdecl VisitHomepage(WPARAM,LPARAM);
	INT_PTR __cdecl GetAvatar(WPARAM,LPARAM);
	INT_PTR __cdecl SetAvatar(WPARAM,LPARAM);

	INT_PTR __cdecl OnJoinChat(WPARAM,LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM,LPARAM);

	INT_PTR __cdecl OnTweet(WPARAM,LPARAM);

	// Events
	int  __cdecl OnContactDeleted(WPARAM,LPARAM);
	int  __cdecl OnBuildStatusMenu(WPARAM,LPARAM);
	int  __cdecl OnOptionsInit(WPARAM,LPARAM);
	int  __cdecl OnModulesLoaded(WPARAM,LPARAM);
	int  __cdecl OnPreShutdown(WPARAM,LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM,LPARAM);
	int  __cdecl OnChatOutgoing(WPARAM,LPARAM);

	void __cdecl SendTweetWorker(void *);
private:
	// Worker threads
	void __cdecl AddToListWorker(void *p);
	void __cdecl SendSuccess(void *);
	void __cdecl DoSearch(void *);
	void __cdecl SignOn(void *);
	void __cdecl MessageLoop(void *);
	void __cdecl GetAwayMsgWorker(void *);
	void __cdecl UpdateAvatarWorker(void *);
	void __cdecl UpdateInfoWorker(void *);

	bool NegotiateConnection();

	void UpdateStatuses(bool pre_read,bool popups, bool tweetToMsg);
	void UpdateMessages(bool pre_read);
	void UpdateFriends();
	void UpdateAvatar(MCONTACT, const std::string &, bool force = false);

	int ShowPinDialog();
	void ShowPopup(const wchar_t *, int Error = 0);
	void ShowPopup(const char *, int Error = 0);
	void ShowContactPopup(MCONTACT, const std::string &, const std::string *);

	bool IsMyContact(MCONTACT, bool include_chat = false);
	MCONTACT UsernameToHContact(const char *);
	MCONTACT AddToClientList(const char *, const char *);
	void SetAllContactStatuses(int);

	static void CALLBACK APC_callback(ULONG_PTR p);

	void UpdateChat(const twitter_user &update);
	void AddChatContact(const char *name,const char *nick=0);
	void DeleteChatContact(const char *name);
	void SetChatStatus(int);

	void TwitterProto::resetOAuthKeys();

	std::tstring GetAvatarFolder();

	mir_cs signon_lock_;
	mir_cs avatar_lock_;
	mir_cs twitter_lock_;

	HANDLE hAvatarNetlib_;
	HANDLE hMsgLoop_;
	mir_twitter twit_;

	twitter_id since_id_;
	twitter_id dm_since_id_;

	bool in_chat_;

	int disconnectionCount;

	//mirandas keys
	wstring ConsumerKey;
	wstring ConsumerSecret;

	// various twitter api URLs
	wstring AuthorizeUrl;
};

// TODO: remove this
inline std::string profile_base_url(const std::string &url)
{
	size_t x = url.find("://");
	if(x == std::string::npos)
		return url.substr(0,url.find('/')+1);
	else
		return url.substr(0,url.find('/',x+3)+1);
}