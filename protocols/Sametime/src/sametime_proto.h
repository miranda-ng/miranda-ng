#ifndef _SAMETIME_PROTO_H
#define _SAMETIME_PROTO_H

struct CSametimeProto : public PROTO<CSametimeProto>
{

	CSametimeProto(const char*, const wchar_t* );
	~CSametimeProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;

	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szReason) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;

	HANDLE   SearchBasic(const wchar_t* id) override;
	HWND     SearchAdvanced(HWND owner) override;
	HWND     CreateExtendedSearchUI(HWND owner) override;

	int      RecvFile(MCONTACT hContact, PROTORECVFILE*) override;
	MEVENT   RecvMsg(MCONTACT hContact, PROTORECVEVENT*) override;

	HANDLE   SendFile(MCONTACT hContact, const wchar_t* szDescription, wchar_t** ppszFiles) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;

	int      SetStatus(int iNewStatus) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt) override;
	int      SetAwayMsg(int iStatus, const wchar_t* msg) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnShutdown() override;

	// sametime.cpp
	int __cdecl OnSametimeContactDeleted(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetName(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SametimeLoadIcon(WPARAM wParam, LPARAM lParam);
	int __cdecl OnWindowEvent(WPARAM wParam, LPARAM lParam);
	int __cdecl OnIdleChanged(WPARAM wParam, LPARAM lParam);
	void SetAllOffline();
	void BroadcastNewStatus(int iNewStatus);

	// messaging.cpp
	void InitMessaging();
	void DeinitMessaging();
	void SendTyping(MCONTACT hContact, bool typing);
	HANDLE SendMessageToUser(MCONTACT hContact, const char *szMsg);
	void CloseIm(MCONTACT hContact);

	// userlist.cpp
	int SearchForUser(const char* name, BOOLEAN advanced);
	MCONTACT AddSearchedUser(MYPROTOSEARCHRESULT* mpsr, bool temporary);
	MCONTACT AddContact(mwSametimeUser* user, bool temporary);
	bool GetAwareIdFromContact(MCONTACT hContact, mwAwareIdBlock* id_block);
	MCONTACT FindContactByUserId(const char* id);
	void ImportContactsFromList(mwSametimeList* user_list, bool temporary);
	void ImportContactsFromFile(wchar_t* filename);
	void ExportContactsToList(mwSametimeList* user_list);
	void ExportContactsToServer();
	void UserListAddStored();
	void InitUserList();
	void DeinitUserList();
	void SetContactGroup(MCONTACT hContact, const char* name);
	void AddGroup(const char* name, bool expanded);
	void UserListCreate();
	void UserListDestroy();
	int GetMoreDetails(const char* name);
	void UserRecvAwayMessage(MCONTACT hContact);
	int ContactDeleted(MCONTACT hContact);

	// files.cpp
	void InitFiles();
	void DeinitFiles();
	HANDLE AcceptFileTransfer(MCONTACT hContact, HANDLE hFt, char* save_path);
	void RejectFileTransfer(HANDLE hFt);
	void CancelFileTransfer(HANDLE hFt);
	HANDLE SendFilesToUser(MCONTACT hContact, wchar_t** files, const wchar_t* pszDesc);

	// conference.cpp
	void InitConference();
	void InitConferenceMenu();
	void DeinitConference();
	void ClearInviteQueue();
	void TerminateConference(char* name);
	int __cdecl GcEventHook(WPARAM wParam, LPARAM lParam);
	int __cdecl PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int ChatDeleted(MCONTACT hContact);
	INT_PTR __cdecl onMenuLeaveChat(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl onMenuCreateChat(WPARAM wParam, LPARAM lParam);

	// sametime_session.cpp
	void SessionStarted();
	void SessionStopping();
	void InitSessionMenu();
	int LogIn(int status);
	int LogOut();
	int OnLogInRedirect(char* newHost);
	int SetSessionStatus(int status);
	void UpdateSelfStatus();
	int SetIdle(bool idle);
	void SetSessionAwayMessage(int status, const wchar_t* msg);
	uint16_t GetClientVersion();
	uint16_t GetServerVersion();
	INT_PTR __cdecl SessionAnnounce(WPARAM wParam, LPARAM lParam);
	void InitAwayMsg();
	void DeinitAwayMsg();
	void InitMeanwhileServices();
	void DeinitMeanwhileServices();
	
	void __cdecl KeepAliveThread(void*);
	void __cdecl SessionThread(void*);

	// options.cpp
	int __cdecl OptInit(WPARAM wParam, LPARAM lParam);
	void LoadOptions();
	void SaveOptions();

	// utils.cpp
	void showPopup(const wchar_t* msg, SametimePopupEnum flag);
	void showPopup(guint32 code);
	void RegisterPopups();
	void UnregisterPopups();
	void RegisterGLibLogger();
	void UnRegisterGLibLogger();

	// places.cpp
	void InitPlaces(mwSession* session);
	void DeinitPlaces(mwSession* session);

	/* properties */

	char szProtoGroups[128];
	HANDLE mainThread;
	mir_cs session_cs;
	mwSession* session;
	int previous_status;
	bool is_idle;

	// userlist.cpp
	mwServiceStorage* service_storage;
	mwServiceAware* service_aware;
	mwServiceResolve* service_resolve;
	mwAwareList* aware_list;

	// files.cpp
	mwServiceFileTransfer* service_files;

	// messaging.cpp
	mir_cs q_cs;
	mwServiceIm* service_im;
	ContactMessageQueue contact_message_queue;

	// sametime_session.cpp
	bool first_online; // set our status after the first online status comes from the server
	int login_status;
	bool idle_status;
	int idle_timerid;
	HNETLIBCONN server_connection;

	// conference.cpp
	InviteQueue invite_queue;
	mwServiceConference* service_conference;
	mwLoginInfo* my_login_info;
	mwConference* my_conference;
	HGENMENU hLeaveChatMenuItem, hCreateChatMenuItem;

	// options.cpp
	SametimeOptions options;

	// places.cpp
	mwServicePlace* service_places;

	// utils.cpp
	HANDLE hPopupError;
	HANDLE hPopupNotify;
	guint gLogHandler;

};

struct CMPlugin : public ACCPROTOPLUGIN<CSametimeProto>
{
	CMPlugin();
};

struct SendAnnouncementFunc_arg
{
	CSametimeProto* proto;
	wchar_t msg[MAX_MESSAGE_SIZE];
	GList* recipients;
};

typedef void (*SendAnnouncementFunc)(SendAnnouncementFunc_arg* ad);

struct SessionAnnounceDialogProc_arg
{
	CSametimeProto* proto;
	SendAnnouncementFunc sendAnnouncementFunc;
};


struct PopupData
{
	SametimePopupEnum flag;
	wchar_t* title;
	wchar_t* text;
	CSametimeProto* proto;
};

#endif //#ifndef _SAMETIME_PROTO_H
