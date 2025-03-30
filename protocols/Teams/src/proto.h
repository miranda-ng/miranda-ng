#define TEAMS_CLIENT_ID  "8ec6bc83-69c8-4392-8f08-b3c986009232"

enum HostType {
	HOST_OTHER = 0,
	HOST_LOGIN = 1,
	HOST_TEAMS = 2,
};

struct AsyncHttpRequest : public MTHttpRequest<CTeamsProto>
{
	AsyncHttpRequest(int iRequestType, HostType host, const char *pszUrl, MTHttpRequestHandler pFunc = 0);

	HostType m_host;
};

class CTeamsProto : public PROTO<CTeamsProto>
{
	friend class COptionsMainDlg;
	friend class CDeviceCodeDlg;

	class CTeamsProtoImpl
	{
		friend class CTeamsProto;
		CTeamsProto &m_proto;

		CTimer m_loginPoll;
		void OnLoginPoll(CTimer *)
		{
			m_proto.LoginPoll();
		}

		CTeamsProtoImpl(CTeamsProto &pro) :
			m_proto(pro),
			m_loginPoll(Miranda_GetSystemWindow(), UINT_PTR(this) + 1)
		{
			m_loginPoll.OnEvent = Callback(this, &CTeamsProtoImpl::OnLoginPoll);
		}
	} m_impl;

	// http queue
	bool m_isTerminated = true;
	mir_cs m_requestQueueLock;
	LIST<AsyncHttpRequest> m_requests;
	MEventHandle m_hRequestQueueEvent;
	HANDLE m_hRequestQueueThread;
	CMStringA m_szAccessToken, m_szSubstrateToken, m_szSkypeToken;

	void __cdecl WorkerThread(void *);

	void StartQueue();
	void StopQueue();

	MHttpResponse* DoSend(AsyncHttpRequest *request);

	void Execute(AsyncHttpRequest *request);
	void PushRequest(AsyncHttpRequest *request);

	// login
	CMStringW m_wszUserCode;
	CMStringA m_szDeviceCode, m_szDeviceCookie, m_szVerificationUrl;
	time_t m_iLoginExpires;

	void Login();
	void LoggedIn();
	void LoginPoll();
	void LoginError();
	
	void OauthRefreshServices();
	void RefreshToken(const char *pszScope, AsyncHttpRequest::MTHttpRequestHandler pFunc);

	void OnReceiveSkypeToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnReceiveDevicePoll(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnReceiveDeviceToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnRefreshAccessToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnRefreshSkypeToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
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
