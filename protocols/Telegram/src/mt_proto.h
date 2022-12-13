#pragma once

class CMTProto;
typedef void (CMTProto::*TG_QUERY_HANDLER)(td::ClientManager::Response &response);

struct TG_REQUEST
{
	TG_REQUEST(td::ClientManager::RequestId _1, TG_QUERY_HANDLER _2) :
		requestId(_1),
		pHandler(_2)
	{}

	td::ClientManager::RequestId requestId;
	TG_QUERY_HANDLER pHandler;
};

class CMTProto : public PROTO<CMTProto>
{
	std::unique_ptr<td::ClientManager> m_pClientMmanager;
	td::td_api::object_ptr<td::td_api::AuthorizationState> pAuthState;

	bool m_bAuthorized, m_bRunning = false, m_bTerminated;
	int32_t m_iClientId;
	uint64_t m_iQueryId;

	OBJLIST<TG_REQUEST> m_arRequests;

	static INT_PTR CALLBACK EnterPhoneCode(void *param);
	static INT_PTR CALLBACK EnterPassword(void *param);

	CMStringW GetProtoFolder() const
	{	return CMStringW(VARSW(L"%miranda_userdata%")) + L"\\" + _A2T(m_szModuleName);
	}

	void OnUpdateAuth(td::ClientManager::Response &response);

	void LogOut(void);
	void OnLoggedIn(void);
	void ProcessAuth(td::td_api::updateAuthorizationState *pObj);
	void ProcessGroups(td::td_api::updateChatFilters *pObj);
	void ProcessResponse(td::ClientManager::Response);
	void SendQuery(td::td_api::Function *pFunc, TG_QUERY_HANDLER pHandler = nullptr);

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
	
	int SetStatus(int iNewStatus) override;

	void OnErase() override;

	// Services //////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	
	// Events ////////////////////////////////////////////////////////////////////////////
	
	int __cdecl OnOptionsInit(WPARAM, LPARAM);

	// Options ///////////////////////////////////////////////////////////////////////////
	
	CMOption<wchar_t*> m_szOwnPhone;       // our own phone number
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation
	CMOption<bool>     m_bUsePopups;

	// Processing Threads ////////////////////////////////////////////////////////////////

	void __cdecl ServerThread(void *);
};
