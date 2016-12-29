
class CDiscordProto;
typedef void (CDiscordProto::*HttpCallback)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);

struct AsyncHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	AsyncHttpRequest();
	AsyncHttpRequest(CDiscordProto*, int iRequestType, LPCSTR szUrl, HttpCallback pFunc, JSONNode *pNode = NULL);
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

__forceinline JSONNode& operator<<(JSONNode &json, const INT_PARAM &param)
{
	json.push_back(JSONNode(param.szName, param.iValue));
	return json;
}

__forceinline JSONNode& operator<<(JSONNode &json, const CHAR_PARAM &param)
{
	json.push_back(JSONNode(param.szName, param.szValue));
	return json;
}

__forceinline JSONNode& operator<<(JSONNode &json, const WCHAR_PARAM &param)
{
	json.push_back(JSONNode(param.szName, ptrA(mir_utf8encodeW(param.wszValue)).get()));
	return json;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CDiscordProto : public PROTO<CDiscordProto>
{
	friend struct AsyncHttpRequest;
	friend class CDiscardAccountOptions;

	void __cdecl ServerThread(void*);

	void SetAllContactStatuses(int iStatus);
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

	CMOption<wchar_t*> m_wszEmail;        // my own email
	CMOption<wchar_t*> m_wszDefaultGroup; // clist group to store contacts 

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
	void OnReceiveMyInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveGuilds(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveChannels(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveMyInfo();

	// Misc
	void SetServerStatus(int iStatus);
};
