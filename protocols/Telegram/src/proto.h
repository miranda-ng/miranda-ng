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

struct TG_FILE_REQUEST
{
	enum Type { AVATAR = 1 };

	TG_FILE_REQUEST(Type _1, const char *_2, const wchar_t *_3) :
		m_type(_1),
		m_uniqueId(_2),
		m_destPath(_3)
	{}

	Type m_type;
	CMStringA m_uniqueId;
	CMStringW m_destPath;
};

struct TG_USER
{
	TG_USER(uint64_t _1, MCONTACT _2, bool _3 = false) :
		id(_1),
		hContact(_2),
		isGroupChat(_3)
	{}

	uint64_t  id;
	MCONTACT  hContact;
	bool      isGroupChat;
	CMStringA szAvatarHash;
	time_t    m_timer1 = 0, m_timer2 = 0;
};

class CTelegramProto : public PROTO<CTelegramProto>
{
	class CProtoImpl
	{
		friend class CTelegramProto;
		CTelegramProto &m_proto;

		CTimer m_keepAlive, m_markRead;
		void OnKeepAlive(CTimer *)
		{	m_proto.SendKeepAlive();
		}

		void OnMarkRead(CTimer *)
		{	m_proto.SendMarkRead();
		}

		CProtoImpl(CTelegramProto &pro) :
			m_proto(pro),
			m_markRead(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_keepAlive(Miranda_GetSystemWindow(), UINT_PTR(this)+1)
		{
			m_markRead.OnEvent = Callback(this, &CProtoImpl::OnMarkRead);
			m_keepAlive.OnEvent = Callback(this, &CProtoImpl::OnKeepAlive);
		}
	} m_impl;

	bool __forceinline isRunning() const 
	{	return m_pClientMmanager != nullptr; 
	}

	std::unique_ptr<td::ClientManager> m_pClientMmanager;
	TD::object_ptr<TD::AuthorizationState> pAuthState;

	mir_cs m_csMarkRead;
	MCONTACT m_markContact = 0;
	TD::array<TD::int53> m_markIds;

	bool m_bAuthorized, m_bTerminated, m_bUnregister = false, m_bSmileyAdd = false;
	int32_t m_iClientId, m_iMsgId;
	uint64_t m_iQueryId;

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
	void OnSendMessage(td::ClientManager::Response &response, void *pUserInfo);
	void OnUpdateAuth(td::ClientManager::Response &response);

	void LogOut(void);
	void OnLoggedIn(void);
	void ProcessResponse(td::ClientManager::Response);

	void SendKeepAlive(void);
	void SendMarkRead(void);
	void SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler = nullptr);
	void SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo);
	int  SendTextMessage(uint64_t chatId, const char *pszMessage);

	void ProcessAuth(TD::updateAuthorizationState *pObj);
	void ProcessChat(TD::updateNewChat *pObj);
	void ProcessChatPosition(TD::updateChatPosition *pObj);
	void ProcessFile(TD::updateFile *pObj);
	void ProcessGroups(TD::updateChatFilters *pObj);
	void ProcessMarkRead(TD::updateChatReadInbox *pObj);
	void ProcessMessage(TD::updateNewMessage *pObj);
	void ProcessStatus(TD::updateUserStatus *pObj);
	void ProcessUser(TD::updateUser *pObj);

	CMStringA GetMessageText(TD::MessageContent *pBody);

	void UpdateString(MCONTACT hContact, const char *pszSetting, const std::string &str);

	// Avatars
	CMStringW GetAvatarFilename(MCONTACT hContact);

	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

	// Users
	int64_t m_iOwnId;
	MGROUP m_iBaseGroup;
	OBJLIST<TG_USER> m_arUsers;

	TG_USER* FindUser(uint64_t id);
	TG_USER* AddUser(uint64_t id, bool bIsChat);

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

	INT_PTR GetCaps(int type, MCONTACT hContact = NULL) override;
	
	int SendMsg(MCONTACT hContact, int flags, const char *pszMessage) override;
	int SetStatus(int iNewStatus) override;

	void OnModulesLoaded() override;
	void OnShutdown() override;
	void OnErase() override;

	// Services //////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	
	// Events ////////////////////////////////////////////////////////////////////////////
	
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnDbMarkedRead(WPARAM, LPARAM);

	// Options ///////////////////////////////////////////////////////////////////////////
	
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
