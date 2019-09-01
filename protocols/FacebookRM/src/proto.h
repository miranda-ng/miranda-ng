/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

#include "constants.h"

#pragma once

class FacebookProto : public PROTO<FacebookProto>
{
	friend class facebook_client;

	bool IgnoreDuplicates(const std::string &mid);
	bool ProcessSpecialMessage(std::vector<facebook_message> &messages, const JSONNode &meta_, MessageType messageType, const std::string &messageData = "");

	void ParseAttachments(std::string &message_text, const JSONNode &delta_, std::string other_user_fbid, bool legacy);
	void ParseMessageType(facebook_message &message, const JSONNode &log_type_, const JSONNode &log_body_, const JSONNode &log_data_);
	bool ParseMessageMetadata(facebook_message &message, const JSONNode &meta_);

	int ParseChatInfo(std::string* data, facebook_chatroom* fbc);
	int ParseChatParticipants(std::string *data, std::map<std::string, chatroom_participant>* participants);
	int ParseFriends(std::string*, std::map< std::string, facebook_user* >*, bool);
	int ParseHistory(std::string* data, std::vector<facebook_message> &messages, std::string *firstTimestamp);
	int ParseMessages(std::string &data, std::vector< facebook_message >&);
	int ParseMessagesCount(std::string *data, int *messagesCount, int *unreadCount);
	int ParseNotifications(std::string*, std::map< std::string, facebook_notification* >*);
	int ParseThreadInfo(std::string* data, std::string* user_id);
	int ParseThreadMessages(std::string*, std::vector< facebook_message >*, bool unreadOnly);
	int ParseUnreadThreads(std::string*, std::vector< std::string >*);
	int ParseUserInfo(std::string* data, facebook_user* fbu);
	int ParseBuddylistUpdate(std::string* data);

	const char* ParseIcon(const std::string &url);

public:
	FacebookProto(const char *proto_name, const wchar_t *username);
	~FacebookProto();

	inline const char* ModuleName() const
	{
		return m_szModuleName;
	}

	inline bool isOnline()
	{
		return (m_iStatus != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_CONNECTING);
	}

	inline bool isOffline()
	{
		return (m_iStatus == ID_STATUS_OFFLINE);
	}

	inline bool isInvisible()
	{
		return m_invisible;
	}

	inline int IdleSeconds()
	{
		if ((m_iStatus == ID_STATUS_AWAY || m_iStatus == ID_STATUS_INVISIBLE) && m_awayTS)
			return time(0) - m_awayTS;

		return m_idleTS ? time(0) - m_idleTS : 0;
	}

	bool m_invisible;
	bool m_enableChat;
	bool m_signingOut;
	time_t m_idleTS;
	time_t m_awayTS;
	time_t m_pingTS;
	std::string m_locale;
	std::string m_pagePrefix;

	// DB utils missing in proto_interface

	__forceinline INT_PTR getStringUtf(const char *name, DBVARIANT *result) {
		return db_get_utf(0, m_szModuleName, name, result); }
	__forceinline INT_PTR getStringUtf(MCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_utf(hContact, m_szModuleName, name, result); }

	__forceinline void setStringUtf(const char *name, const char* value) { db_set_utf(0, m_szModuleName, name, value); }
	__forceinline void setStringUtf(MCONTACT hContact, const char *name, const char* value) { db_set_utf(hContact, m_szModuleName, name, value); }

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;
				 
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t* szMessage) override;
				 
	INT_PTR  GetCaps(int type, MCONTACT hContact = 0) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;
				 
	HANDLE   SearchBasic(const wchar_t* id) override;
	HANDLE   SearchByEmail(const wchar_t* email) override;
	HANDLE   SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName) override;
				 
	MEVENT   RecvMsg(MCONTACT hContact, PROTORECVEVENT*) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
				 
	int      SetStatus(int iNewStatus) override;
				 
	int      SetAwayMsg(int iStatus, const wchar_t* msg) override;
				 
	int      UserIsTyping(MCONTACT hContact, int type) override;
				 
	void     OnBuildProtoMenu(void) override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	////////////////////////

	// Services
	INT_PTR __cdecl GetMyAwayMsg(WPARAM, LPARAM);
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl VisitProfile(WPARAM, LPARAM);
	INT_PTR __cdecl VisitFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl VisitConversation(WPARAM, LPARAM);
	INT_PTR __cdecl VisitNotifications(WPARAM, LPARAM);
	INT_PTR __cdecl Poke(WPARAM, LPARAM);
	INT_PTR __cdecl LoadHistory(WPARAM, LPARAM);
	INT_PTR __cdecl CancelFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl RequestFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl ApproveFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl DenyFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl OnCancelFriendshipRequest(WPARAM, LPARAM);
	INT_PTR __cdecl CheckNewsfeeds(WPARAM, LPARAM);
	INT_PTR __cdecl CheckFriendRequests(WPARAM, LPARAM);
	INT_PTR __cdecl CheckNotifications(WPARAM, LPARAM);
	INT_PTR __cdecl CheckMemories(WPARAM, LPARAM);
	INT_PTR __cdecl GetNotificationsCount(WPARAM, LPARAM);

	INT_PTR __cdecl OnJoinChat(WPARAM,LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM,LPARAM);

	INT_PTR __cdecl OnMind(WPARAM,LPARAM);

	// Initialization
	void InitPopups();
	void InitHotkeys();
	void InitSounds();

	// Events
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnToolbarInit(WPARAM, LPARAM);
	int  __cdecl OnBuildStatusMenu(WPARAM,LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM,LPARAM);
	int  __cdecl OnIdleChanged(WPARAM,LPARAM);
	int  __cdecl OnGCEvent(WPARAM,LPARAM);
	int  __cdecl OnGCMenuHook(WPARAM,LPARAM);
	int  __cdecl OnDbEventRead(WPARAM, LPARAM);
	int  __cdecl OnProcessSrmmEvent(WPARAM, LPARAM);
	int  __cdecl OnPreCreateEvent(WPARAM, LPARAM);

	// Loops
	bool    NegotiateConnection();
	BYTE    GetPollRate();
	void __cdecl MessageLoop(void*);
	void __cdecl UpdateLoop(void*);

	// Processing threads
	void __cdecl ProcessFriendList(void*);
	void __cdecl ProcessBuddylistUpdate(void*);
	void __cdecl ProcessUnreadMessages(void*);
	void __cdecl ProcessUnreadMessage(void*);
	void __cdecl ProcessFeeds(void*);
	void __cdecl ProcessNotifications(void*);
	void __cdecl ProcessFriendRequests(void*);
	void __cdecl SearchAckThread(void*);
	void __cdecl SearchIdAckThread(void*);
	void __cdecl ProcessPages(void*);
	void __cdecl LoadLastMessages(void*);
	void __cdecl LoadHistory(void*);
	void __cdecl ProcessMemories(void*);

	// Worker threads
	void __cdecl ChangeStatus(void*);
	void __cdecl SetAwayMsgWorker(void*);
	void __cdecl UpdateAvatarWorker(void*);
	void __cdecl SendMsgWorker(void*);
	void __cdecl SendChatMsgWorker(void*);
	void __cdecl SendTypingWorker(void*);
	void __cdecl ReadMessageWorker(void*);
	void __cdecl ReadNotificationWorker(void*);
	void __cdecl DeleteContactFromServer(void*);
	void __cdecl AddContactToServer(void*);
	void __cdecl ApproveContactToServer(void*);
	void __cdecl CancelFriendsRequest(void*);
	void __cdecl SendPokeWorker(void*);
	void __cdecl IgnoreFriendshipRequest(void*);
	void __cdecl RefreshUserInfo(void*);

	// Contacts handling
	bool		IsMyContact(MCONTACT, bool include_chat = false);
	MCONTACT	ContactIDToHContact(const std::string&);
	MCONTACT	ChatIDToHContact(const std::string&);
	std::string	ThreadIDToContactID(const std::string&);
	void		LoadContactInfo(facebook_user* fbu);
	MCONTACT	AddToContactList(facebook_user*, bool force_add = false, bool add_temporarily = false);
	MCONTACT	HContactFromAuthEvent(MEVENT hEvent);
	void		StartTyping(MCONTACT hContact);
	void		StopTyping(MCONTACT hContact);

	// Chats handling
	void AddChat(const char *chat_id, const wchar_t *name);
	void UpdateChat(const char *chat_id, const char *id, const char *name, const char *message, DWORD timestamp = 0, bool is_old = false);
	void RenameChat(const char *chat_id, const char *name);
	bool IsChatContact(const char *chat_id, const char *id);
	void AddChatContact(const char *chat_id, const chatroom_participant &user, bool addToLog);
	void RemoveChatContact(const char *chat_id, const char *id, const char *name);
	char *GetChatUsers(const char *chat_id);
	void ReceiveMessages(std::vector<facebook_message> &messages, bool check_duplicates = false);
	void LoadChatInfo(facebook_chatroom* fbc);
	void LoadParticipantsNames(facebook_chatroom *fbc);
	void JoinChatrooms();

	bool IsSpecialChatRoom(MCONTACT hContact);
	void PrepareNotificationsChatRoom();
	void UpdateNotificationsChatRoom(facebook_notification *notification);
	std::string FacebookProto::GenerateChatName(facebook_chatroom *fbc);

	// Connection client
	facebook_client facy; // TODO: Refactor to "client" and make dynamic

	// Helpers
	std::wstring GetAvatarFolder();
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATION &ai, std::string *url);
	void CheckAvatarChange(MCONTACT hContact, const std::string &image_url);
	void ToggleStatusMenuItems(bool bEnable);
	void StickerAsSmiley(std::string stickerId, const std::string &url, MCONTACT hContact);
	void SaveName(MCONTACT hContact, const facebook_user *fbu);	
	std::string PrepareUrl(std::string url);
	void OpenUrl(std::string url);
	void __cdecl OpenUrlThread(void*);
	void MessageRead(MCONTACT hContact);
	bool RunCaptchaForm(std::string imageUrl, std::string &result);

	// Menu items
	HGENMENU m_hMenuServicesRoot;
	HGENMENU m_hStatusMind;

	// Locks
	mir_cs signon_lock_;
	mir_cs avatar_lock_;
	mir_cs log_lock_;
	HANDLE update_loop_event;

	ptrW m_tszDefaultGroup;

	std::vector<HANDLE> popupClasses;

	std::string last_status_msg_;
	std::vector<MCONTACT> avatar_queue;

	mir_cs csReactions;
	std::map<std::string, std::string> reactions;

	// Information providing
	HWND NotifyEvent(const wchar_t* title, const wchar_t* text, MCONTACT contact, EventType type, std::string *url = nullptr, std::string *notification_id = nullptr, const char *icon = nullptr);
	void ShowNotifications();
};

struct CMPlugin : public ACCPROTOPLUGIN<FacebookProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
