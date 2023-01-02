#pragma once

#define DBKEY_ID "id"

class CMTProto;
typedef void (CMTProto:: *TG_QUERY_HANDLER)(td::ClientManager::Response &response);
typedef void (CMTProto:: *TG_QUERY_HANDLER_FULL)(td::ClientManager::Response &response, void *pUserInfo);

struct TG_REQUEST_BASE
{
	TG_REQUEST_BASE(td::ClientManager::RequestId _1) :
		requestId(_1)
	{}

	virtual ~TG_REQUEST_BASE()
	{}

	td::ClientManager::RequestId requestId;

	virtual void Execute(CMTProto *ppro, td::ClientManager::Response &response) = 0;
};

struct TG_REQUEST : public TG_REQUEST_BASE
{
	TG_REQUEST(td::ClientManager::RequestId _1, TG_QUERY_HANDLER _2) :
		TG_REQUEST_BASE(_1),
		pHandler(_2)
	{}

	TG_QUERY_HANDLER pHandler;

	void Execute(CMTProto *ppro, td::ClientManager::Response &response) override
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

	void Execute(CMTProto *ppro, td::ClientManager::Response &response) override
	{
		(ppro->*pHandler)(response, pUserInfo);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct TG_USER
{
	TG_USER(uint64_t _1, MCONTACT _2, bool _3 = false) :
		id(_1),
		hContact(_2),
		isGroupChat(_3)
	{}

	uint64_t id;
	MCONTACT hContact;
	bool isGroupChat;
	time_t m_timer1 = 0, m_timer2 = 0;
};

class CMTProto : public PROTO<CMTProto>
{
	class CProtoImpl
	{
		friend class CMTProto;
		CMTProto &m_proto;

		CTimer m_keepAlive;
		void OnKeepAlive(CTimer *)
		{
			m_proto.SendKeepAlive();
		}

		CProtoImpl(CMTProto &pro) :
			m_proto(pro),
			m_keepAlive(Miranda_GetSystemWindow(), UINT_PTR(this))
		{
			m_keepAlive.OnEvent = Callback(this, &CProtoImpl::OnKeepAlive);
		}
	} m_impl;

	bool __forceinline isRunning() const 
	{	return m_pClientMmanager != nullptr; 
	}

	std::unique_ptr<td::ClientManager> m_pClientMmanager;
	TD::object_ptr<TD::AuthorizationState> pAuthState;

	bool m_bAuthorized, m_bTerminated, m_bUnregister = false;
	int32_t m_iClientId, m_iMsgId;
	uint64_t m_iQueryId;

	OBJLIST<TG_REQUEST_BASE> m_arRequests;

	static INT_PTR CALLBACK EnterPhoneCode(void *param);
	static INT_PTR CALLBACK EnterPassword(void *param);

	CMStringW GetProtoFolder() const
	{	return CMStringW(VARSW(L"%miranda_userdata%")) + L"\\" + _A2T(m_szModuleName);
	}

	void OnUpdateAuth(td::ClientManager::Response &response);

	void LogOut(void);
	void OnEndSession(td::ClientManager::Response &response);
	void OnLoggedIn(void);
	void ProcessResponse(td::ClientManager::Response);
	void SendKeepAlive(void);
	void SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler = nullptr);
	void SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo);

	void ProcessAuth(TD::updateAuthorizationState *pObj);
	void ProcessChat(TD::updateNewChat *pObj);
	void ProcessChatPosition(TD::updateChatPosition *pObj);
	void ProcessGroups(TD::updateChatFilters *pObj);
	void ProcessMessage(TD::updateNewMessage *pObj);
	void ProcessStatus(TD::updateUserStatus *pObj);
	void ProcessUser(TD::updateUser *pObj);

	void OnSendMessage(td::ClientManager::Response &response, void *pUserInfo);
	int  SendTextMessage(uint64_t chatId, const char *pszMessage);

	void UpdateString(MCONTACT hContact, const char *pszSetting, const std::string &str);

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

	CMTProto(const char *protoName, const wchar_t *userName);
	~CMTProto();

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

	// Options ///////////////////////////////////////////////////////////////////////////
	
	CMOption<wchar_t*> m_szOwnPhone;       // our own phone number
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<wchar_t*> m_wszDeviceName;    // how do you see this session in Device List
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation
	CMOption<bool>     m_bUsePopups;

	// Processing Threads ////////////////////////////////////////////////////////////////

	void __cdecl ServerThread(void *);
};
