
typedef __int64 SnowFlake;

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

JSONNode& operator<<(JSONNode &json, const INT_PARAM &param);
JSONNode& operator<<(JSONNode &json, const CHAR_PARAM &param);
JSONNode& operator<<(JSONNode &json, const WCHAR_PARAM &param);

/////////////////////////////////////////////////////////////////////////////////////////

enum CDiscordHitoryOp
{
	MSG_NOFILTER, MSG_AFTER, MSG_BEFORE
};

struct CDiscordUser : public MZeroedObject
{
	CDiscordUser(SnowFlake _id) :
		id(_id)
		{}

	SnowFlake id;
	MCONTACT  hContact;

	SnowFlake channelId;
	SnowFlake lastMessageId;
	bool      bIsPrivate;

	CMStringW wszUsername;
	int       iDiscriminator;
};

class CDiscordProto : public PROTO<CDiscordProto>
{
	friend struct AsyncHttpRequest;
	friend class CDiscardAccountOptions;

	//////////////////////////////////////////////////////////////////////////////////////
	// threads

	void __cdecl ServerThread(void*);
	void __cdecl SearchThread(void *param);

	//////////////////////////////////////////////////////////////////////////////////////
	// session control

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

	//////////////////////////////////////////////////////////////////////////////////////
	// gateway

	CMStringA m_szGateway;
	HANDLE 
		m_hGatewayNetlibUser,  // the separate netlib user handle for gateways
		m_hGatewayConnection;  // gateway connection
	void __cdecl GatewayThread(void*);
	void GatewaySend(int opCode, const char*);

	void OnReceiveGateway(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	//////////////////////////////////////////////////////////////////////////////////////
	// options

	CMOption<wchar_t*> m_wszEmail;        // my own email
	CMOption<wchar_t*> m_wszDefaultGroup; // clist group to store contacts 

	//////////////////////////////////////////////////////////////////////////////////////
	// common data

	SnowFlake m_ownId;

	OBJLIST<CDiscordUser> arUsers;
	CDiscordUser* FindUser(SnowFlake id);
	CDiscordUser* FindUser(const wchar_t *pwszUsername, int iDiscriminator);
	CDiscordUser* PrepareUser(const JSONNode&);

	//////////////////////////////////////////////////////////////////////////////////////
	// misc methods

	SnowFlake getId(const char *szName);
	SnowFlake getId(MCONTACT hContact, const char *szName);

	void setId(const char *szName, SnowFlake iValue);
	void setId(MCONTACT hContact, const char *szName, SnowFlake iValue);

public:
	CDiscordProto(const char*,const wchar_t*);
	~CDiscordProto();

	// PROTO_INTERFACE
	virtual DWORD_PTR __cdecl GetCaps(int, MCONTACT = 0) override;

	virtual HWND __cdecl CreateExtendedSearchUI(HWND owner) override;
	virtual HWND __cdecl SearchAdvanced(HWND owner) override;

	virtual HANDLE __cdecl SearchBasic(const wchar_t* id) override;
	virtual MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr) override;
	
	virtual int __cdecl AuthRequest(MCONTACT hContact, const wchar_t*) override;

	virtual int __cdecl SetStatus(int iNewStatus) override;
	virtual int __cdecl OnEvent(PROTOEVENTTYPE, WPARAM, LPARAM) override;

	// Services
	INT_PTR __cdecl GetStatus(WPARAM, LPARAM);

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnPreShutdown(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnSrmmEvent(WPARAM, LPARAM);

	void OnLoggedIn();
	void OnLoggedOut();

	void OnReceiveAuth(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveToken(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveGuilds(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveChannels(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveFriends(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveUserInfo(MCONTACT hContact);
	void OnReceiveUserInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveHistory(MCONTACT hContact, CDiscordHitoryOp iOp = MSG_NOFILTER, SnowFlake msgid = 0, int iLimit = 50);
	void OnReceiveHistory(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	// Misc
	void SetServerStatus(int iStatus);
};
