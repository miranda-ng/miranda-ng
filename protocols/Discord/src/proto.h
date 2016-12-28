
class CDiscordProto;
typedef void (CDiscordProto::*HttpCallback)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);

struct AsyncHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	AsyncHttpRequest();
	AsyncHttpRequest(CDiscordProto*, int iRequestType, LPCSTR szUrl, HttpCallback pFunc);
	~AsyncHttpRequest();

	void AddHeader(LPCSTR, LPCSTR);

	CMStringA m_szUrl;
	CMStringA m_szParam;
	HttpCallback m_pCallback;
	int m_iErrorCode, m_iReqNum;
	void *pUserInfo;
};

struct PARAM
{
	LPCSTR szName;
	__forceinline PARAM(LPCSTR _name) : szName(_name)
	{}
};

struct INT_PARAM : public PARAM
{
	int iValue;
	__forceinline INT_PARAM(LPCSTR _name, int _value) :
		PARAM(_name), iValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT_PARAM&);

struct CHAR_PARAM : public PARAM
{
	LPCSTR szValue;
	__forceinline CHAR_PARAM(LPCSTR _name, LPCSTR _value) :
		PARAM(_name), szValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const CHAR_PARAM&);

struct WCHAR_PARAM : public PARAM
{
	LPCWSTR wszValue;
	__forceinline WCHAR_PARAM(LPCSTR _name, LPCWSTR _value) :
		PARAM(_name), wszValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const WCHAR_PARAM&);

/////////////////////////////////////////////////////////////////////////////////////////

class CDiscordProto : public PROTO<CDiscordProto>
{
	friend struct AsyncHttpRequest;

	void __cdecl ServerThread(void*);

	void SetAllContactStatuses(int iStatus);
	bool TryToConnect(void);
	void ConnectionFailed(int iReason);
	void ShutdownSession(void);

	ptrA m_szAccessToken;

	mir_cs m_csHttpQueue;
	HANDLE m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;
	
	void ExecuteRequest(AsyncHttpRequest *pReq);
	AsyncHttpRequest* Push(AsyncHttpRequest *pReq, int iTimeout = 10000);

	HANDLE
		m_hAPIConnection,  // working connection
		m_hWorkerThread;   // worker thread handle

	bool 
		m_bOnline,         // protocol is online
		m_bTerminated;     // Miranda's going down

public:
	CDiscordProto(const char*,const wchar_t*);
	~CDiscordProto();

	// PROTO_INTERFACE
	virtual DWORD_PTR __cdecl GetCaps(int, MCONTACT = 0) override;

	virtual int __cdecl SetStatus(int iNewStatus) override;

	virtual int __cdecl OnEvent(PROTOEVENTTYPE, WPARAM, LPARAM) override;

	// Services
	INT_PTR __cdecl GetName(WPARAM, LPARAM);
	INT_PTR __cdecl GetStatus(WPARAM, LPARAM);

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnPreShutdown(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);

	void OnLoggedIn();
	void OnLoggedOut();
	
	void OnReceiveToken(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	// Misc
	void RetrieveMyInfo(void);
	void SetServerStatus(int iStatus);
};
