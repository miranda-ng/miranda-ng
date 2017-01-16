
typedef __int64 SnowFlake;

class CDiscordProto;
typedef void (CDiscordProto::*HttpCallback)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);
typedef void (CDiscordProto::*GatewayHandlerFunc)(const JSONNode&);

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
	bool m_bMainSite;
	void *pUserInfo;
};

struct PARAM
{
	LPCSTR szName;
	__forceinline PARAM(LPCSTR _name) : szName(_name)
	{}
};

struct BOOL_PARAM : public PARAM
{
	bool bValue;
	__forceinline BOOL_PARAM(LPCSTR _name, bool _value) :
		PARAM(_name), bValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const BOOL_PARAM&);

struct INT_PARAM : public PARAM
{
	int iValue;
	__forceinline INT_PARAM(LPCSTR _name, int _value) :
		PARAM(_name), iValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT_PARAM&);

struct INT64_PARAM : public PARAM
{
	SnowFlake iValue;
	__forceinline INT64_PARAM(LPCSTR _name, SnowFlake _value) :
		PARAM(_name), iValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT64_PARAM&);

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
JSONNode& operator<<(JSONNode &json, const BOOL_PARAM &param);
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
	void __cdecl SendMessageAckThread(void* param);

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

	HANDLE m_hWorkerThread;       // worker thread handle
	HNETLIBCONN m_hAPIConnection; // working connection

	bool 
		m_bOnline,         // protocol is online
		m_bTerminated;     // Miranda's going down

	//////////////////////////////////////////////////////////////////////////////////////
	// gateway

	CMStringA
		m_szGateway,           // gateway url
		m_szGatewaySessionId;  // current session id
	
	HNETLIBUSER m_hGatewayNetlibUser; // the separate netlib user handle for gateways
	HNETLIBCONN m_hGatewayConnection;      // gateway connection
	
	void __cdecl GatewayThread(void*);
	void CDiscordProto::GatewayThreadWorker(void);
	
	void  GatewaySend(const JSONNode&, int opCode = 1);
	void  GatewayProcess(const JSONNode&);

	void  GatewaySendHeartbeat(void);
	void  GatewaySendIdentify(void);

	GatewayHandlerFunc GetHandler(const wchar_t*);

	int   m_iHartbeatInterval;	// in milliseconds
	int   m_iGatewaySeq;       // gateway sequence number

	//////////////////////////////////////////////////////////////////////////////////////
	// options

	CMOption<wchar_t*> m_wszEmail;        // my own email
	CMOption<wchar_t*> m_wszDefaultGroup; // clist group to store contacts 

	//////////////////////////////////////////////////////////////////////////////////////
	// common data

	SnowFlake m_ownId;

	mir_cs csMarkReadQueue;
	LIST<CDiscordUser> arMarkReadQueue;

	OBJLIST<CDiscordUser> arUsers;
	CDiscordUser* FindUser(SnowFlake id);
	CDiscordUser* FindUser(const wchar_t *pwszUsername, int iDiscriminator);
	CDiscordUser* FindUserByChannel(SnowFlake channelId);
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

	virtual int __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT *evt) override;
	virtual int __cdecl SendMsg(MCONTACT hContact, int flags, const char* pszSrc) override;

	virtual int __cdecl SetStatus(int iNewStatus) override;
	virtual int __cdecl OnEvent(PROTOEVENTTYPE, WPARAM, LPARAM) override;

	// Services
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl GetStatus(WPARAM, LPARAM);

	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnPreShutdown(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnDbEventRead(WPARAM, LPARAM);
	
	// dispatch commands
	void OnCommandFriendRemoved(const JSONNode&);
	void OnCommandMessage(const JSONNode&);
	void OnCommandPresence(const JSONNode&);
	void OnCommandReady(const JSONNode&);
	void OnCommandTyping(const JSONNode&);
	void OnCommandUserUpdate(const JSONNode&);

	void OnLoggedIn();
	void OnLoggedOut();
	
	int  OnDeleteContact(MCONTACT hContact);

	void OnReceiveAuth(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveChannels(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveFriends(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveGateway(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveGuilds(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveMessage(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveMessageAck(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveToken(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveUserInfo(MCONTACT hContact);
	void OnReceiveUserInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveHistory(MCONTACT hContact, CDiscordHitoryOp iOp = MSG_NOFILTER, SnowFlake msgid = 0, int iLimit = 50);
	void OnReceiveHistory(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	bool RetrieveAvatar(MCONTACT hContact);
	void OnReceiveAvatar(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	// Misc
	void SetServerStatus(int iStatus);

	CMStringW GetAvatarFilename(MCONTACT hContact);

	__forceinline int getHeartbeatInterval() const { return m_iHartbeatInterval; }

	static void CALLBACK HeartbeatTimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD);
	static void CALLBACK MarkReadTimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD);
};
