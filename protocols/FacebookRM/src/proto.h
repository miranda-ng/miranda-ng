/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

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

	bool m_invisible;
	bool m_enableChat;

	// DB utils missing in proto_interface

	__forceinline INT_PTR getStringUtf(const char *name, DBVARIANT *result) {
		return db_get_utf(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getStringUtf(MCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_utf(hContact, m_szModuleName, name, result); }

	__forceinline void setStringUtf(const char *name, const char* value) { db_set_utf(NULL, m_szModuleName, name, value); }
	__forceinline void setStringUtf(MCONTACT hContact, const char *name, const char* value) { db_set_utf(hContact, m_szModuleName, name, value); }

	//PROTO_INTERFACE

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int      __cdecl Authorize(HANDLE hDbEvent);
	virtual	int      __cdecl AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTOFILEEVENT*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
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
	MCONTACT	ContactIDToHContact(std::string);
	MCONTACT	ChatIDToHContact(std::tstring);
	std::string	ThreadIDToContactID(std::string thread_id);
	void		LoadContactInfo(facebook_user* fbu);
	MCONTACT	AddToContactList(facebook_user*, ContactType type, bool force_add = false);
	void		SetAllContactStatuses(int status);
	MCONTACT	HContactFromAuthEvent(HANDLE hEvent);
	void		StartTyping(MCONTACT hContact);
	void		StopTyping(MCONTACT hContact);

	// Chats handling
 	void AddChat(const TCHAR *id, const TCHAR *name);
	void UpdateChat(const TCHAR *chat_id, const char *id, const char *name, const char *message, DWORD timestamp = 0);
	void RenameChat(const char *chat_id, const char *name);
	bool IsChatContact(const TCHAR *chat_id, const char *id);
	void AddChatContact(const TCHAR *chat_id, const char *id, const char *name);
	void RemoveChatContact(const TCHAR *chat_id, const char *id, const char *name);
	void SetChatStatus(const char *chat_id, int status);
	char *GetChatUsers(const TCHAR *chat_id);
	void ReceiveMessages(std::vector<facebook_message*> messages, bool local_timestamp, bool check_duplicates = false);
	void LoadChatInfo(facebook_chatroom* fbc);
	void LoadParticipantsNames(facebook_chatroom *fbc);

	// Connection client
	facebook_client facy; // TODO: Refactor to "client" and make dynamic

	// Helpers
	std::tstring GetAvatarFolder();
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATIONT &ai, std::string *url);
	void CheckAvatarChange(MCONTACT hContact, std::string image_url);
	void ToggleStatusMenuItems(BOOL bEnable);
	void StickerAsSmiley(std::string stickerId, std::string url, MCONTACT hContact);
	void SaveName(MCONTACT hContact, const facebook_user *fbu);	
	void OpenUrl(std::string url);
	void __cdecl OpenUrlThread(void*);
	void MessageRead(MCONTACT hContact);

	// Handles, Locks
	HGENMENU m_hMenuRoot, m_hMenuServicesRoot, m_hStatusMind;

	HANDLE  signon_lock_;
	HANDLE  avatar_lock_;
	HANDLE  log_lock_;
	HANDLE  update_loop_lock_;

	std::vector<HANDLE> popupClasses;

	std::string last_status_msg_;
	HANDLE	hSmileysFolder_;
	std::vector<MCONTACT> avatar_queue;

	static void CALLBACK APC_callback(ULONG_PTR p);

	// Information providing
	HWND NotifyEvent(TCHAR* title, TCHAR* info, MCONTACT contact, DWORD flags, std::string *url = NULL, std::string *notification_id = NULL);
	void ShowNotifications();
};
