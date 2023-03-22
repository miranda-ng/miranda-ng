#pragma once

#define STATUS_SWITCH_TIMEOUT 600

#define DBKEY_ID "id"

#define DBKEY_AVATAR_HASH "AvatarHash"
#define DBKEY_AVATAR_TYPE "AvatarType"

class CTelegramProto;
typedef void (CTelegramProto:: *TG_QUERY_HANDLER)(td::ClientManager::Response &response);
typedef void (CTelegramProto:: *TG_QUERY_HANDLER_FULL)(td::ClientManager::Response &response, void *pUserInfo);

struct TG_REQUEST_BASE
{
	TG_REQUEST_BASE(td::ClientManager::RequestId _1) :
		requestId(_1)
	{}

	virtual ~TG_REQUEST_BASE()
	{}

	td::ClientManager::RequestId requestId;

	virtual void Execute(CTelegramProto *ppro, td::ClientManager::Response &response) = 0;
};

struct TG_REQUEST : public TG_REQUEST_BASE
{
	TG_REQUEST(td::ClientManager::RequestId _1, TG_QUERY_HANDLER _2) :
		TG_REQUEST_BASE(_1),
		pHandler(_2)
	{}

	TG_QUERY_HANDLER pHandler;

	void Execute(CTelegramProto *ppro, td::ClientManager::Response &response) override
	{
		(ppro->*pHandler)(response);
	}
};

struct TG_REQUEST_FULL : public TG_REQUEST_BASE
{
	TG_REQUEST_FULL(td::ClientManager::RequestId _1, TG_QUERY_HANDLER_FULL _2, void *_3) :
		TG_REQUEST_BASE(_1),
		pHandler(_2),
		pUserInfo(_3)
	{}

	TG_QUERY_HANDLER_FULL pHandler;
	void *pUserInfo;

	void Execute(CTelegramProto *ppro, td::ClientManager::Response &response) override
	{
		(ppro->*pHandler)(response, pUserInfo);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct TG_FILE_REQUEST : public MZeroedObject
{
	enum Type { AVATAR = 1, FILE = 2 };

	TG_FILE_REQUEST(Type _1, TD::int53 _2, const char *_3) :
		m_type(_1),
		m_fileId(_2),
		m_uniqueId(_3)
	{}

	Type m_type;
	TD::int53 m_fileId;
	CMStringA m_uniqueId;
	CMStringW m_destPath, m_fileName;
	PROTOFILETRANSFERSTATUS pfts;
};

struct TG_USER
{
	TG_USER(int64_t _1, MCONTACT _2, bool _3 = false) :
		id(_1),
		hContact(_2),
		isGroupChat(_3)
	{
		chatId = (isGroupChat) ? -1 :id;
	}

	int64_t   id, chatId;
	MCONTACT  hContact;
	bool      isGroupChat;
	CMStringA szAvatarHash;
	CMStringW wszNick, wszFirstName, wszLastName;
	time_t    m_timer1 = 0, m_timer2 = 0;
	SESSION_INFO *m_si = nullptr;
	TD::chatNotificationSettings notificationSettings;

	CMStringW getDisplayName() const;
};

struct TG_SUPER_GROUP
{
	TG_SUPER_GROUP(int64_t _1, TD::object_ptr<TD::supergroup> _2) :
		id(_1),
		group(std::move(_2))
	{}

	int64_t id;
	TD::object_ptr<TD::supergroup> group;
};

struct TG_BASIC_GROUP
{
	TG_BASIC_GROUP(int64_t _1, TD::object_ptr<TD::basicGroup> _2) :
		id(_1),
		group(std::move(_2))
	{}

	int64_t id;
	TD::object_ptr<TD::basicGroup> group;
};

class CTelegramProto : public PROTO<CTelegramProto>
{
	friend class CAddPhoneContactDlg;

	class CProtoImpl
	{
		friend class CTelegramProto;
		CTelegramProto &m_proto;

		CTimer m_keepAlive, m_markRead, m_deleteMsg;
		void OnKeepAlive(CTimer *)
		{	m_proto.SendKeepAlive();
		}

		void OnDeleteMsg(CTimer *)
		{	m_proto.SendDeleteMsg();
		}

		void OnMarkRead(CTimer *)
		{	m_proto.SendMarkRead();
		}

		CProtoImpl(CTelegramProto &pro) :
			m_proto(pro),
			m_markRead(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_keepAlive(Miranda_GetSystemWindow(), UINT_PTR(this)+1),
			m_deleteMsg(Miranda_GetSystemWindow(), UINT_PTR(this)+2)
		{
			m_markRead.OnEvent = Callback(this, &CProtoImpl::OnMarkRead);
			m_deleteMsg.OnEvent = Callback(this, &CProtoImpl::OnDeleteMsg);
			m_keepAlive.OnEvent = Callback(this, &CProtoImpl::OnKeepAlive);
		}
	} m_impl;

	bool __forceinline isRunning() const 
	{	return m_pClientManager != nullptr; 
	}

	std::unique_ptr<td::ClientManager> m_pClientManager;
	TD::object_ptr<TD::AuthorizationState> pAuthState;

	mir_cs m_csMarkRead;
	MCONTACT m_markContact = 0;
	TD::array<TD::int53> m_markIds;

	mir_cs m_csDeleteMsg;
	MCONTACT m_deleteMsgContact = 0;
	TD::array<TD::int53> m_deleteIds;

	bool m_bAuthorized, m_bTerminated, m_bUnregister = false, m_bSmileyAdd = false;
	int32_t m_iClientId, m_iMsgId;
	int64_t m_iQueryId;
	CMStringA m_szFullPhone;

	OBJLIST<TG_REQUEST_BASE> m_arRequests;
	OBJLIST<TG_FILE_REQUEST> m_arFiles;

	static INT_PTR CALLBACK EnterEmail(void *param);
	static INT_PTR CALLBACK EnterEmailCode(void *param);
	static INT_PTR CALLBACK EnterPassword(void *param);
	static INT_PTR CALLBACK EnterPhoneCode(void *param);

	CMStringW GetProtoFolder() const
	{	return CMStringW(VARSW(L"%miranda_userdata%")) + L"\\" + _A2T(m_szModuleName);
	}

	void OnEndSession(td::ClientManager::Response &response);
	void OnSearchResults(td::ClientManager::Response &response);
	void OnSendMessage(td::ClientManager::Response &response, void *pUserInfo);
	void OnUpdateAuth(td::ClientManager::Response &response);

	void LogOut(void);
	void OnLoggedIn(void);
	void ProcessResponse(td::ClientManager::Response);

	void SendKeepAlive(void);
	void SendDeleteMsg(void);
	void SendMarkRead(void);
	void SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler = nullptr);
	void SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo);
	int  SendTextMessage(int64_t chatId, const char *pszMessage);

	void ProcessAuth(TD::updateAuthorizationState *pObj);
	void ProcessBasicGroup(TD::updateBasicGroup *pObj);
	void ProcessChat(TD::updateNewChat *pObj);
	void ProcessChatLastMessage(TD::updateChatLastMessage *pObj);
	void ProcessChatNotification(TD::updateChatNotificationSettings *pObj);
	void ProcessChatPosition(TD::updateChatPosition *pObj);
	void ProcessDeleteMessage(TD::updateDeleteMessages *pObj);
	void ProcessFile(TD::updateFile *pObj);
	void ProcessGroups(TD::updateChatFilters *pObj);
	void ProcessMarkRead(TD::updateChatReadInbox *pObj);
	void ProcessMessage(TD::updateNewMessage *pObj);
	void ProcessStatus(TD::updateUserStatus *pObj);
	void ProcessSuperGroup(TD::updateSupergroup *pObj);
	void ProcessUser(TD::updateUser *pObj);

	CMStringA GetMessageText(TG_USER *pUser, TD::MessageContent *pBody);

	void UpdateString(MCONTACT hContact, const char *pszSetting, const std::string &str);

	// Avatars
	CMStringW GetAvatarFilename(MCONTACT hContact);

	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

	// Group chats
	OBJLIST<TG_BASIC_GROUP>  m_arBasicGroups;
	OBJLIST<TG_SUPER_GROUP>  m_arSuperGroups;

	void InitGroupChat(TG_USER *pUser, const TD::chat *pChat, bool bUpdateMembers);
	void StartGroupChat(td::ClientManager::Response &response, void *pUserData);
	
	void Chat_SendPrivateMessage(GCHOOK *gch);
	void Chat_LogMenu(GCHOOK *gch);

	// Search
	TD::array<TD::int53> m_searchIds;

	bool CheckSearchUser(TG_USER *pUser);
	void ReportSearchUser(TG_USER *pUser);

	// Users
	int64_t m_iOwnId;
	MGROUP m_iBaseGroup;
	LIST<TG_USER> m_arChats;
	OBJLIST<TG_USER> m_arUsers;

	TG_USER* FindChat(int64_t id);
	TG_USER* FindUser(int64_t id);
	TG_USER* AddUser(int64_t id, bool bIsChat);
	TG_USER* AddFakeUser(int64_t id, bool bIsChat);
	TG_USER* GetSender(const TD::MessageSender *pSender);
	void     SetId(MCONTACT, int64_t id);

	// Popups
	HANDLE m_hPopupClass;

	void InitPopups(void);
	void Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle);

public:
	//////////////////////////////////////////////////////////////////////////////////////
	// Ctors

	CTelegramProto(const char *protoName, const wchar_t *userName);
	~CTelegramProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr);
		
	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileResume(HANDLE hTransfer, int action, const wchar_t *szFilename) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;

	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;
	int      SendMsg(MCONTACT hContact, int flags, const char *pszMessage) override;
	int      SetStatus(int iNewStatus) override;
		
	void     OnBuildProtoMenu() override;
	void     OnContactDeleted(MCONTACT hContact) override;
	MWindow  OnCreateAccMgrUI(MWindow hwndParent) override;
	void     OnEventDeleted(MCONTACT, MEVENT) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;
	void     OnErase() override;

	// Events ////////////////////////////////////////////////////////////////////////////
	
	int __cdecl OnEmptyHistory(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);

	int __cdecl GcMenuHook(WPARAM, LPARAM);
	int __cdecl GcMuteHook(WPARAM, LPARAM);
	int __cdecl GcEventHook(WPARAM, LPARAM);

	// Services //////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl AddByPhone(WPARAM, LPARAM);

	// Options ///////////////////////////////////////////////////////////////////////////
	
	CMOption<uint32_t> m_iCountry;		   // set this status to m_iStatus1 after this interval of secs
	CMOption<wchar_t*> m_szOwnPhone;       // our own phone number
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<wchar_t*> m_wszDeviceName;    // how do you see this session in Device List
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation
	CMOption<bool>     m_bUsePopups;
	CMOption<uint32_t> m_iTimeDiff1;		   // set this status to m_iStatus1 after this interval of secs
	CMOption<uint32_t> m_iStatus1;
	CMOption<uint32_t> m_iTimeDiff2;		   // set this status to m_iStatus2 after this interval of secs
	CMOption<uint32_t> m_iStatus2;

	// Processing Threads ////////////////////////////////////////////////////////////////

	void __cdecl ServerThread(void *);
};

typedef CProtoDlgBase<CTelegramProto> CTelegramDlgBase;
