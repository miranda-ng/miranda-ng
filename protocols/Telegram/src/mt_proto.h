#pragma once

class CMTProto;
typedef void (CMTProto::*TG_QUERY_HANDLER)(td::ClientManager::Response &response);

struct TG_REQUEST
{
	TG_REQUEST(int32_t _1, TG_QUERY_HANDLER _2) :
		queryId(_1),
		pHandler(_2)
	{}

	int32_t queryId;
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
	void ProcessResponse(td::ClientManager::Response);
	void SendQuery(td::td_api::Function *pFunc, TG_QUERY_HANDLER pHandler = nullptr);

public:
	//////////////////////////////////////////////////////////////////////////////////////
	// Ctors

	CMTProto(const char *protoName, const wchar_t *userName);
	~CMTProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	INT_PTR GetCaps(int type, MCONTACT hContact = NULL) override;
	
	int SetStatus(int iNewStatus) override;

	// Processing Threads //////////////////////////////////////////////////////////////////

	void __cdecl ServerThread(void *);
};
