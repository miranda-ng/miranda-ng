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
		return (m_iStatus == ID_STATUS_INVISIBLE);
	}

	// DB utils missing in proto_interface

	__forceinline INT_PTR getStringUtf(const char *name, DBVARIANT *result) {
		return db_get_utf(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getStringUtf(HANDLE hContact, const char *name, DBVARIANT *result) {
		return db_get_utf(hContact, m_szModuleName, name, result); }

	__forceinline void setStringUtf(const char *name, const char* value) { db_set_utf(NULL, m_szModuleName, name, value); }
	__forceinline void setStringUtf(HANDLE hContact, const char *name, const char* value) { db_set_utf(hContact, m_szModuleName, name, value); }

	//PROTO_INTERFACE

	virtual	HANDLE   __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	HANDLE   __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int      __cdecl Authorize(HANDLE hDbEvent);
	virtual	int      __cdecl AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRecv(HANDLE hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(HANDLE hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE   __cdecl FileAllow(HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
	virtual	int      __cdecl FileCancel(HANDLE hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, HANDLE hContact = NULL);
	virtual	int       __cdecl GetInfo(HANDLE hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(HANDLE hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(HANDLE hContact, PROTOFILEEVENT*);
	virtual	int       __cdecl RecvMsg(HANDLE hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(HANDLE hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList);
	virtual	HANDLE    __cdecl SendFile(HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(HANDLE hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(HANDLE hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(HANDLE hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(HANDLE hContact);
	virtual	int       __cdecl RecvAwayMsg(HANDLE hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg);

	virtual	int       __cdecl UserIsTyping(HANDLE hContact, int type);

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
	INT_PTR __cdecl Poke(WPARAM, LPARAM);
	INT_PTR __cdecl CancelFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl RequestFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl ApproveFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl OnCancelFriendshipRequest(WPARAM, LPARAM);
	INT_PTR __cdecl CheckNewsfeeds(WPARAM, LPARAM);
	INT_PTR __cdecl CheckFriendRequests(WPARAM, LPARAM);
	INT_PTR __cdecl RefreshBuddyList(WPARAM, LPARAM);

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

	// Contacts handling
	bool    IsMyContact(HANDLE, bool include_chat = false);
	HANDLE  ContactIDToHContact(std::string);
	HANDLE  ChatIDToHContact(std::string);
	HANDLE  AddToContactList(facebook_user*, ContactType type, bool dont_check = false);
	void    SetAllContactStatuses(int status, bool reset_client = false);
	HANDLE  HContactFromAuthEvent(HANDLE hEvent);

	// Chats handling
 	void AddChat(const char *id, const char *name);
	void UpdateChat(const char *chat_id, const char *id, const char *name, const char *message, DWORD timestamp = 0);
	bool IsChatContact(const char *chat_id, const char *id);
	void AddChatContact(const char *chat_id, const char *id, const char *name);
	void RemoveChatContact(const char *chat_id, const char *id);
	void SetChatStatus(const char *chat_id, int status);
	char *GetChatUsers(const char *chat_id);

	// Connection client
	facebook_client facy; // TODO: Refactor to "client" and make dynamic

	// Helpers
	std::tstring GetAvatarFolder();
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATIONT &ai, std::string *url);
	void CheckAvatarChange(HANDLE hContact, std::string image_url);
	void ToggleStatusMenuItems(BOOL bEnable);
	void ParseSmileys(std::string message, HANDLE hContact);
	void OpenUrl(std::string url);
	void SaveName(HANDLE hContact, const facebook_user *fbu);

	// Handles, Locks
	HGENMENU m_hMenuRoot, m_hMenuServicesRoot, m_hStatusMind;

	HANDLE  signon_lock_;
	HANDLE  avatar_lock_;
	HANDLE  log_lock_;
	HANDLE  update_loop_lock_;

	HANDLE  m_hNetlibUser;

	std::vector<HANDLE> popupClasses;

	std::string last_status_msg_;
	HANDLE  hAvatarFolder_;
	HANDLE	hSmileysFolder_;
	std::vector<HANDLE> avatar_queue;

	static void CALLBACK APC_callback(ULONG_PTR p);

	// Information providing
	int Log(const char *fmt,...);
	void NotifyEvent(TCHAR* title, TCHAR* info, HANDLE contact, DWORD flags, std::string *url = NULL, std::string *notification_id = NULL);
};
