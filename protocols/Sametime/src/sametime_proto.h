
#ifndef _SAMETIME_PROTO_H
#define _SAMETIME_PROTO_H



struct CSametimeProto : public PROTO<CSametimeProto>
{

	CSametimeProto(const char*, const TCHAR* );
	~CSametimeProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================


	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR* szDescription, TCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const TCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);



	// sametime.cpp
	int __cdecl OnSametimeContactDeleted(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetName(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetStatus(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SametimeLoadIcon(WPARAM wParam, LPARAM lParam);
	int __cdecl OnWindowEvent(WPARAM wParam, LPARAM lParam);
	int __cdecl OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	int __cdecl OnPreShutdown(WPARAM wParam, LPARAM lParam);
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
	void ImportContactsFromFile(TCHAR* filename);
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
	HANDLE SendFilesToUser(MCONTACT hContact, TCHAR** files, const TCHAR* pszDesc);

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
	int LogIn(int status, HANDLE hNetlibUser);
	int LogOut();
	int SetSessionStatus(int status);
	void UpdateSelfStatus();
	int SetIdle(bool idle);
	void SetSessionAwayMessage(int status, const TCHAR* msg);
	WORD GetClientVersion();
	WORD GetServerVersion();
	INT_PTR __cdecl SessionAnnounce(WPARAM wParam, LPARAM lParam);
	void InitAwayMsg();
	void DeinitAwayMsg();
	void InitMeanwhileServices();
	void DeinitMeanwhileServices();

	// options.cpp
	int __cdecl OptInit(WPARAM wParam, LPARAM lParam);
	void LoadOptions();
	void SaveOptions();

	// utils.cpp
	void showPopup(const TCHAR* msg, SametimePopupEnum flag);
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
	HANDLE server_connection;

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



typedef struct tag_TFakeAckParams {
	CSametimeProto* proto;
	MCONTACT hContact;
	LPARAM lParam;
} TFakeAckParams;


struct SendAnnouncementFunc_arg {
	CSametimeProto* proto;
	TCHAR msg[MAX_MESSAGE_SIZE];
	GList* recipients;
};

typedef void (*SendAnnouncementFunc)(SendAnnouncementFunc_arg* ad);

struct SessionAnnounceDialogProc_arg {
	CSametimeProto* proto;
	SendAnnouncementFunc sendAnnouncementFunc;
};


struct PopupData {
	SametimePopupEnum flag;
	TCHAR* title;
	TCHAR* text;
	CSametimeProto* proto;
};


#endif //#ifndef _SAMETIME_PROTO_H

