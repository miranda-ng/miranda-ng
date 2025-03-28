enum HostType {
	HOST_OTHER = 0,
	HOST_LOGIN = 1,
};

struct AsyncHttpRequest : public MTHttpRequest<CTeamsProto>
{
	AsyncHttpRequest(int iRequestType, HostType host, const char *pszUrl, MTHttpRequestHandler pFunc = 0);

	HostType m_host;
};

class CTeamsProto : public PROTO<CTeamsProto>
{
	friend class COptionsMainDlg;

	CMStringA GetTenant();

	// http queue
	bool m_isTerminated = true;
	mir_cs m_requestQueueLock;
	LIST<AsyncHttpRequest> m_requests;
	MEventHandle m_hRequestQueueEvent;
	HANDLE m_hRequestQueueThread;

	void __cdecl WorkerThread(void *);

	void StartQueue();
	void StopQueue();

	MHttpResponse* DoSend(AsyncHttpRequest *request);

	void Execute(AsyncHttpRequest *request);
	void PushRequest(AsyncHttpRequest *request);

	// login
	void Login();
	void RefreshToken(const char *pszScope, AsyncHttpRequest::MTHttpRequestHandler pFunc);

	void OnRefreshAccessToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnRefreshSubstrate(MHttpResponse *response, AsyncHttpRequest *pRequest);

	// options
	int __cdecl OnOptionsInit(WPARAM, LPARAM);

	// settings
	CMOption<wchar_t *> m_wstrCListGroup;

public:
	// constructor
	CTeamsProto(const char *protoName, const wchar_t *userName);
	~CTeamsProto();

	MWindow  OnCreateAccMgrUI(MWindow) override;

	INT_PTR  GetCaps(int type, MCONTACT) override;
	int      SetStatus(int iNewStatus) override;
};

typedef CProtoDlgBase<CTeamsProto> CTeamsDlgBase;

struct CMPlugin : public ACCPROTOPLUGIN<CTeamsProto>
{
	CMPlugin();

	int Load() override;
};
