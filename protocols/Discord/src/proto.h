#pragma once

typedef __int64 SnowFlake;

__forceinline int compareInt64(const SnowFlake i1, const SnowFlake i2)
{
	return (i1 == i2) ? 0 : (i1 < i2) ? -1 : 1;
}

class CDiscordProto;
typedef void (CDiscordProto::*HttpCallback)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);
typedef void (CDiscordProto::*GatewayHandlerFunc)(const JSONNode&);

struct AsyncHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	AsyncHttpRequest();
	AsyncHttpRequest(CDiscordProto*, int iRequestType, LPCSTR szUrl, HttpCallback pFunc, JSONNode *pNode = nullptr);
	~AsyncHttpRequest();

	void AddHeader(LPCSTR, LPCSTR);

	CMStringA m_szUrl;
	CMStringA m_szParam;
	HttpCallback m_pCallback;
	int m_iErrorCode, m_iReqNum;
	bool m_bMainSite;
	void *pUserInfo;
};

AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT_PARAM&);
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT64_PARAM&);
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const CHAR_PARAM&);
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const WCHAR_PARAM&);

/////////////////////////////////////////////////////////////////////////////////////////

struct CDiscordRole : public MZeroedObject
{
	SnowFlake id;
	COLORREF color;
	DWORD permissions;
	int position;
	CMStringW wszName;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CDiscordMessage
{
	SnowFlake id, authorId;

	CDiscordMessage(SnowFlake _id = 0, SnowFlake _authorId = 0) :
		id(_id),
		authorId(_authorId)
	{}
};

struct COwnMessage
{
	SnowFlake nonce;
	int reqId;

	COwnMessage(SnowFlake _id, int _reqId) :
		nonce(_id),
		reqId(_reqId)
	{}
};

/////////////////////////////////////////////////////////////////////////////////////////

enum CDiscordHistoryOp
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

	SnowFlake guildId;
	SnowFlake channelId;
	SnowFlake lastReadId;
	bool      bIsPrivate;

	CDiscordMessage lastMsg;

	CMStringW wszUsername;
	int       iDiscriminator;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CDiscordGuildMember : public MZeroedObject
{
	CDiscordGuildMember(SnowFlake id) :
		userId(id)
	{}

	~CDiscordGuildMember()
	{}

	SnowFlake userId;
	CMStringW wszNick, wszRole;
	int iStatus;
};

struct CDiscordGuild : public MZeroedObject
{
	CDiscordGuild(SnowFlake _id);
	~CDiscordGuild();

	__forceinline CDiscordGuildMember* FindUser(SnowFlake userId)
	{
		return arChatUsers.find((CDiscordGuildMember*)&userId);
	}

	SnowFlake id, ownerId;
	CMStringW wszName;
	MCONTACT hContact;

	GCSessionInfoBase *pParentSi;
	OBJLIST<CDiscordGuildMember> arChatUsers;
	OBJLIST<CDiscordRole> arRoles; // guild roles
};

/////////////////////////////////////////////////////////////////////////////////////////

class CDiscordProto : public PROTO<CDiscordProto>
{
	friend struct AsyncHttpRequest;
	friend class CDiscardAccountOptions;

	//////////////////////////////////////////////////////////////////////////////////////
	// threads

	void __cdecl SendFileThread(void*);
	void __cdecl ServerThread(void*);
	void __cdecl SearchThread(void *param);
	void __cdecl SendMessageAckThread(void* param);

	//////////////////////////////////////////////////////////////////////////////////////
	// session control

	void ConnectionFailed(int iReason);
	void ShutdownSession(void);

	ptrA m_szAccessToken, m_szAccessCookie;

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
	void  GatewayThreadWorker(void);
	
	void  GatewaySend(const JSONNode&, int opCode = 1);
	void  GatewayProcess(const JSONNode&);

	void  GatewaySendHeartbeat(void);
	void  GatewaySendIdentify(void);
	void  GatewaySendGuildInfo(SnowFlake id);
	void  GatewaySendResume(void);

	GatewayHandlerFunc GetHandler(const wchar_t*);

	int   m_iHartbeatInterval;	// in milliseconds
	int   m_iGatewaySeq;       // gateway sequence number

	//////////////////////////////////////////////////////////////////////////////////////
	// options

	CMOption<wchar_t*> m_wszEmail;        // my own email
	CMOption<wchar_t*> m_wszDefaultGroup; // clist group to store contacts
	CMOption<BYTE>     m_bHideGroupchats; // hide guild chats on startup

	//////////////////////////////////////////////////////////////////////////////////////
	// common data

	SnowFlake m_ownId;

	mir_cs csMarkReadQueue;
	LIST<CDiscordUser> arMarkReadQueue;

	OBJLIST<CDiscordUser> arUsers;
	OBJLIST<COwnMessage> arOwnMessages;

	CDiscordUser* FindUser(SnowFlake id);
	CDiscordUser* FindUser(const wchar_t *pwszUsername, int iDiscriminator);
	CDiscordUser* FindUserByChannel(SnowFlake channelId);
	CDiscordUser* PrepareUser(const JSONNode&);

	//////////////////////////////////////////////////////////////////////////////////////
	// menu items

	void InitMenus(void);

	int __cdecl OnMenuPrebuild(WPARAM, LPARAM);

	INT_PTR __cdecl OnMenuCopyId(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuCreateChannel(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuJoinGuild(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuLeaveGuild(WPARAM, LPARAM);

	HGENMENU m_hMenuLeaveGuild, m_hMenuCreateChannel;

	//////////////////////////////////////////////////////////////////////////////////////
	// guilds

	OBJLIST<CDiscordGuild> arGuilds;

	__forceinline CDiscordGuild* FindGuild(SnowFlake id) const
	{
		return arGuilds.find((CDiscordGuild*)&id);
	}

	void ProcessGuild(const JSONNode&);
	void AddGuildUser(CDiscordGuild *guild, const CDiscordGuildMember &pUser);
	void ParseGuildContents(CDiscordGuild *guild, const JSONNode &);
	CDiscordUser* ProcessGuildChannel(CDiscordGuild *guild, const JSONNode&);
	void ProcessRole(CDiscordGuild *guild, const JSONNode&);
	void ProcessType(CDiscordUser *pUser, const JSONNode&);

	//////////////////////////////////////////////////////////////////////////////////////
	// group chats

	int  __cdecl GroupchatEventHook(WPARAM, LPARAM);
	int  __cdecl GroupchatMenuHook(WPARAM, LPARAM);

	void Chat_SendPrivateMessage(GCHOOK *gch);
	void Chat_ProcessLogMenu(GCHOOK *gch);

	void BuildStatusList(const CDiscordGuild *pGuild, const CMStringW &wszChannelId);
	void ParseSpecialChars(SESSION_INFO *si, CMStringW &str);

	//////////////////////////////////////////////////////////////////////////////////////
	// misc methods

	SnowFlake getId(const char *szName);
	SnowFlake getId(MCONTACT hContact, const char *szName);

	void setId(const char *szName, SnowFlake iValue);
	void setId(MCONTACT hContact, const char *szName, SnowFlake iValue);

public:
	CDiscordProto(const char*,const wchar_t*);
	~CDiscordProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR GetCaps(int, MCONTACT = 0) override;

	HWND CreateExtendedSearchUI(HWND owner) override;
	HWND SearchAdvanced(HWND owner) override;

	HANDLE SearchBasic(const wchar_t *id) override;
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	
	int AuthRecv(MCONTACT, PROTORECVEVENT *pre) override;
	int Authorize(MEVENT hDbEvent) override;
	int AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
	int AuthRequest(MCONTACT hContact, const wchar_t*) override;

	int SendMsg(MCONTACT hContact, int flags, const char *pszSrc) override;

	HANDLE SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;

	int UserIsTyping(MCONTACT hContact, int type) override;

	int SetStatus(int iNewStatus) override;

	void OnContactDeleted(MCONTACT) override;
	void OnModulesLoaded() override;
	void OnShutdown() override;

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// Events

	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnDbEventRead(WPARAM, LPARAM);
	
	//////////////////////////////////////////////////////////////////////////////////////
	// dispatch commands

	void OnCommandChannelCreated(const JSONNode&);
	void OnCommandChannelDeleted(const JSONNode&);
	void OnCommandChannelUpdated(const JSONNode&);
	void OnCommandGuildCreated(const JSONNode&);
	void OnCommandGuildDeleted(const JSONNode&);
	void OnCommandGuildMemberAdded(const JSONNode&);
	void OnCommandGuildMemberRemoved(const JSONNode&);
	void OnCommandGuildMemberUpdated(const JSONNode&);
	void OnCommandGuildSync(const JSONNode&);
	void OnCommandFriendAdded(const JSONNode&);
	void OnCommandFriendRemoved(const JSONNode&);
	void OnCommandMessage(const JSONNode&);
	void OnCommandMessageAck(const JSONNode&);
	void OnCommandPresence(const JSONNode&);
	void OnCommandReady(const JSONNode&);
	void OnCommandRoleCreated(const JSONNode&);
	void OnCommandRoleDeleted(const JSONNode&);
	void OnCommandTyping(const JSONNode&);
	void OnCommandUserUpdate(const JSONNode&);
	void OnCommandUserSettingsUpdate(const JSONNode&);

	void OnLoggedIn();
	void OnLoggedOut();
	
	void OnReceiveCreateChannel(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveFile(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveGateway(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveMessage(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveMessageAck(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveToken(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveMyInfo();
	void OnReceiveMyInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveHistory(MCONTACT hContact, CDiscordHistoryOp iOp = MSG_NOFILTER, SnowFlake msgid = 0, int iLimit = 50);
	void OnReceiveHistory(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	bool RetrieveAvatar(MCONTACT hContact);
	void OnReceiveAvatar(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	//////////////////////////////////////////////////////////////////////////////////////
	// Misc

	void SetServerStatus(int iStatus);
	void RemoveFriend(SnowFlake id);

	CMStringW GetAvatarFilename(MCONTACT hContact);
	void CheckAvatarChange(MCONTACT hContact, const CMStringW &wszNewHash);

	__forceinline int getHeartbeatInterval() const { return m_iHartbeatInterval; }

	static void CALLBACK HeartbeatTimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD);
	static void CALLBACK MarkReadTimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD);
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDiscordProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
