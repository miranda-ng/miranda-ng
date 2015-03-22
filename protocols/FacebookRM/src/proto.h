/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-15 Robert P�sel

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
public:
	FacebookProto(const char *proto_name, const TCHAR *username);
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
		//return (m_iStatus == ID_STATUS_INVISIBLE);
	}

	inline int IdleSeconds()
	{
		return m_idleTS ? time(0) - m_idleTS : 0;
	}

	bool m_invisible;
	bool m_enableChat;
	bool m_signingOut;
	time_t m_idleTS;
	time_t m_pingTS;
	std::string m_locale;

	// DB utils missing in proto_interface

	__forceinline INT_PTR getStringUtf(const char *name, DBVARIANT *result) {
		return db_get_utf(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getStringUtf(MCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_utf(hContact, m_szModuleName, name, result); }

	__forceinline void setStringUtf(const char *name, const char* value) { db_set_utf(NULL, m_szModuleName, name, value); }
	__forceinline void setStringUtf(MCONTACT hContact, const char *name, const char* value) { db_set_utf(hContact, m_szModuleName, name, value); }

	//PROTO_INTERFACE

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	int      __cdecl Authorize(MEVENT hDbEvent);
	virtual	int      __cdecl AuthDeny(MEVENT hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	////////////////////////

	// Services
	INT_PTR __cdecl GetMyAwayMsg(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAwayMsg(WPARAM, LPARAM);
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl VisitProfile(WPARAM, LPARAM);
	INT_PTR __cdecl VisitFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl VisitConversation(WPARAM, LPARAM);
	INT_PTR __cdecl VisitNotifications(WPARAM, LPARAM);
	INT_PTR __cdecl Poke(WPARAM, LPARAM);
	INT_PTR __cdecl CancelFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl RequestFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl ApproveFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl DenyFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl OnCancelFriendshipRequest(WPARAM, LPARAM);
	INT_PTR __cdecl CheckNewsfeeds(WPARAM, LPARAM);
	INT_PTR __cdecl CheckFriendRequests(WPARAM, LPARAM);
	INT_PTR __cdecl RefreshBuddyList(WPARAM, LPARAM);
	INT_PTR __cdecl GetNotificationsCount(WPARAM, LPARAM);

	INT_PTR __cdecl OnJoinChat(WPARAM,LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM,LPARAM);

	INT_PTR __cdecl OnMind(WPARAM,LPARAM);

	// Initialiation
	void InitPopups();
	void InitHotkeys();
	void InitSounds();

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnToolbarInit(WPARAM, LPARAM);
	int  __cdecl OnBuildStatusMenu(WPARAM,LPARAM);
	int  __cdecl OnContactDeleted(WPARAM,LPARAM);
	int  __cdecl OnPreShutdown(WPARAM,LPARAM);
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
	void __cdecl ProcessBuddyList(void*);
	void __cdecl ProcessFriendList(void*);
	void __cdecl ProcessMessages(void*);
	void __cdecl ProcessUnreadMessages(void*);
	void __cdecl ProcessUnreadMessage(void*);
	void __cdecl ProcessFeeds(void*);
	void __cdecl ProcessNotifications(void*);
	void __cdecl ProcessFriendRequests(void*);
	void __cdecl SearchAckThread(void*);
	void __cdecl SearchIdAckThread(void*);
	void __cdecl ProcessPages(void*);
	void __cdecl LoadLastMessages(void*);
	void __cdecl SyncThreads(void*);

	// Worker threads
	void __cdecl SignOn(void*);
	void __cdecl ChangeStatus(void*);
	void __cdecl SignOff(void*);
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

	// Contacts handling
	bool		IsMyContact(MCONTACT, bool include_chat = false);
	MCONTACT	ContactIDToHContact(const std::string&);
	MCONTACT	ChatIDToHContact(const std::tstring&);
	std::string	ThreadIDToContactID(const std::string&);
	void		LoadContactInfo(facebook_user* fbu);
	MCONTACT	AddToContactList(facebook_user*, ContactType type, bool force_add = false, bool add_temporarily = false);
	void		SetAllContactStatuses(int status);
	MCONTACT	HContactFromAuthEvent(MEVENT hEvent);
	void		StartTyping(MCONTACT hContact);
	void		StopTyping(MCONTACT hContact);

	// Chats handling
 	void AddChat(const TCHAR *id, const TCHAR *name);
	void UpdateChat(const TCHAR *chat_id, const char *id, const char *name, const char *message, DWORD timestamp = 0, bool is_old = false);
	void RenameChat(const char *chat_id, const char *name);
	bool IsChatContact(const TCHAR *chat_id, const char *id);
	void AddChatContact(const TCHAR *chat_id, const char *id, const char *name);
	void RemoveChatContact(const TCHAR *chat_id, const char *id, const char *name);
	char *GetChatUsers(const TCHAR *chat_id);
	void ReceiveMessages(std::vector<facebook_message*> messages, bool check_duplicates = false);
	void LoadChatInfo(facebook_chatroom* fbc);
	void LoadParticipantsNames(facebook_chatroom *fbc);
	
	bool IsSpecialChatRoom(MCONTACT hContact);
	void PrepareNotificationsChatRoom();
	void UpdateNotificationsChatRoom(facebook_notification *notification);

	// Connection client
	facebook_client facy; // TODO: Refactor to "client" and make dynamic

	// Helpers
	std::tstring GetAvatarFolder();
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATIONT &ai, std::string *url);
	void CheckAvatarChange(MCONTACT hContact, const std::string &image_url);
	void ToggleStatusMenuItems(BOOL bEnable);
	void StickerAsSmiley(std::string stickerId, const std::string &url, MCONTACT hContact);
	void SaveName(MCONTACT hContact, const facebook_user *fbu);	
	std::string PrepareUrl(std::string url);
	void OpenUrl(std::string url);
	void __cdecl OpenUrlThread(void*);
	void MessageRead(MCONTACT hContact);
	bool RunCaptchaForm(std::string imageUrl, std::string &result);

	// Menu items
	HGENMENU m_hMenuRoot;
	HGENMENU m_hMenuServicesRoot;
	HGENMENU m_hStatusMind;

	// Locks
	HANDLE signon_lock_;
	HANDLE avatar_lock_;
	HANDLE log_lock_;
	HANDLE update_loop_lock_;

	ptrT m_tszDefaultGroup;

	std::vector<HANDLE> popupClasses;

	std::string last_status_msg_;
	std::vector<MCONTACT> avatar_queue;

	static void CALLBACK APC_callback(ULONG_PTR p);

	// Information providing
	HWND NotifyEvent(TCHAR* title, TCHAR* info, MCONTACT contact, DWORD flags, std::string *url = NULL, std::string *notification_id = NULL);
	void ShowNotifications();
};
