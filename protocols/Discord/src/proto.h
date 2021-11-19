#pragma once

#define EVENT_INCOMING_CALL 10001
#define EVENT_CALL_FINISHED 10002

typedef __int64 SnowFlake;

__forceinline int compareInt64(const SnowFlake i1, const SnowFlake i2)
{
	return (i1 == i2) ? 0 : (i1 < i2) ? -1 : 1;
}

class CDiscordProto;
typedef void (CDiscordProto::*GatewayHandlerFunc)(const JSONNode&);

struct AsyncHttpRequest : public MTHttpRequest<CDiscordProto>
{
	AsyncHttpRequest(CDiscordProto*, int iRequestType, LPCSTR szUrl, MTHttpRequestHandler pFunc, JSONNode *pNode = nullptr);

	int m_iErrorCode, m_iReqNum;
	bool m_bMainSite;
	MCONTACT hContact;
};

class JsonReply
{
	JSONNode *m_root = nullptr;
	int m_errorCode = 0;

public:
	JsonReply(NETLIBHTTPREQUEST *);
	~JsonReply();

	__forceinline int error() const { return m_errorCode; }
	__forceinline JSONNode& data() const { return *m_root; }
	__forceinline operator bool() const { return m_errorCode == 200; }
};

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

	~CDiscordUser();

	SnowFlake id;
	MCONTACT  hContact;

	SnowFlake channelId;
	SnowFlake lastReadId, lastMsgId;
	SnowFlake parentId;
	bool      bIsPrivate;
	bool      bIsGroup;
	bool      bSynced;

	struct CDiscordGuild *pGuild;

	CMStringW wszUsername, wszChannelName, wszTopic;
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
	CMStringW wszDiscordId, wszNick, wszRole;
	int iStatus;
};

struct CDiscordGuild : public MZeroedObject
{
	CDiscordGuild(SnowFlake _id);
	~CDiscordGuild();

	__forceinline CDiscordGuildMember* FindUser(SnowFlake userId)
	{
		return arChatUsers.find((CDiscordGuildMember *)&userId);
	}

	__inline CMStringW GetCacheFile() const
	{
		return CMStringW(FORMAT, L"%s\\DiscordCache\\%lld.json", VARSW(L"%miranda_userdata%").get(), id);
	}

	SnowFlake id, ownerId;
	CMStringW wszName;
	MCONTACT hContact;
	MGROUP groupId;
	bool bSynced = false;
	LIST<CDiscordUser> arChannels;

	SESSION_INFO *pParentSi;
	OBJLIST<CDiscordGuildMember> arChatUsers;
	OBJLIST<CDiscordRole> arRoles; // guild roles

	void LoadFromFile();
	void SaveToFile();
};

struct CDiscordVoiceCall
{
	CMStringA szId;
	SnowFlake channelId;
	time_t    startTime;
};

/////////////////////////////////////////////////////////////////////////////////////////

#define OPCODE_DISPATCH              0
#define OPCODE_HEARTBEAT             1
#define OPCODE_IDENTIFY              2
#define OPCODE_STATUS_UPDATE         3
#define OPCODE_VOICE_UPDATE          4
#define OPCODE_VOICE_PING            5
#define OPCODE_RESUME                6
#define OPCODE_RECONNECT             7
#define OPCODE_REQUEST_MEMBERS       8
#define OPCODE_INVALID_SESSION       9
#define OPCODE_HELLO                10
#define OPCODE_HEARTBEAT_ACK        11
#define OPCODE_REQUEST_SYNC         12
#define OPCODE_REQUEST_SYNC_GROUP   13
#define OPCODE_REQUEST_SYNC_CHANNEL 14

class CDiscordProto : public PROTO<CDiscordProto>
{
	friend struct AsyncHttpRequest;
	friend class CDiscardAccountOptions;

	class CDiscordProtoImpl
	{
		friend class CDiscordProto;
		CDiscordProto &m_proto;

		CTimer m_heartBeat, m_markRead;
		void OnHeartBeat(CTimer *) {
			m_proto.GatewaySendHeartbeat();
		}
		
		void OnMarkRead(CTimer *pTimer) {
			m_proto.SendMarkRead();
			pTimer->Stop();
		}

		CDiscordProtoImpl(CDiscordProto &pro) :
			m_proto(pro),
			m_markRead(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_heartBeat(Miranda_GetSystemWindow(), UINT_PTR(this) + 1)
		{
			m_markRead.OnEvent = Callback(this, &CDiscordProtoImpl::OnMarkRead);
			m_heartBeat.OnEvent = Callback(this, &CDiscordProtoImpl::OnHeartBeat);
		}
	} m_impl;

	//////////////////////////////////////////////////////////////////////////////////////
	// threads

	void __cdecl SendFileThread(void*);
	void __cdecl ServerThread(void*);
	void __cdecl SearchThread(void *param);
	void __cdecl BatchChatCreate(void* param);
	void __cdecl GetAwayMsgThread(void *param);

	//////////////////////////////////////////////////////////////////////////////////////
	// session control

	void ConnectionFailed(int iReason);
	void ShutdownSession(void);

	wchar_t *m_wszStatusMsg[MAX_STATUS_COUNT];

	ptrA m_szAccessToken, m_szTempToken;

	mir_cs m_csHttpQueue;
	HANDLE m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;
	
	void ExecuteRequest(AsyncHttpRequest *pReq);
	void Push(AsyncHttpRequest *pReq, int iTimeout = 10000);
	void SaveToken(const JSONNode &data);

	HANDLE m_hWorkerThread;       // worker thread handle
	HNETLIBCONN m_hAPIConnection; // working connection

	bool 
		m_bOnline,         // protocol is online
		m_bTerminated;     // Miranda's going down

	//////////////////////////////////////////////////////////////////////////////////////
	// gateway

	CMStringA
		m_szGateway,           // gateway url
		m_szGatewaySessionId,  // current session id
		m_szCookie,            // cookie used for all http queries
		m_szWSCookie;          // cookie used for establishing websocket connection
	
	HNETLIBUSER m_hGatewayNetlibUser; // the separate netlib user handle for gateways
	HNETLIBCONN m_hGatewayConnection;      // gateway connection
	
	void __cdecl GatewayThread(void*);
	bool  GatewayThreadWorker(void);
	
	bool  GatewaySend(const JSONNode &pNode);
	bool  GatewayProcess(const JSONNode &pNode);

	void  GatewaySendGuildInfo(CDiscordGuild *pGuild);
	void  GatewaySendHeartbeat(void);
	void  GatewaySendIdentify(void);
	void  GatewaySendResume(void);
	bool  GatewaySendStatus(int iStatus, const wchar_t *pwszStatusText);

	GatewayHandlerFunc GetHandler(const wchar_t*);

	int   m_iHartbeatInterval;	// in milliseconds
	int   m_iGatewaySeq;       // gateway sequence number

	//////////////////////////////////////////////////////////////////////////////////////
	// options

	CMOption<wchar_t*> m_wszEmail;        // my own email
	CMOption<wchar_t*> m_wszDefaultGroup; // clist group to store contacts
	CMOption<BYTE>     m_bUseGroupchats;  // Shall we connect Guilds at all?
	CMOption<BYTE>     m_bHideGroupchats; // Do not open chat windows on creation
	CMOption<BYTE>     m_bUseGuildGroups; // use special subgroups for guilds
	CMOption<BYTE>     m_bSyncDeleteMsgs; // delete messages from Miranda if they are deleted at the server

	//////////////////////////////////////////////////////////////////////////////////////
	// common data

	SnowFlake m_ownId;

	mir_cs csMarkReadQueue;
	LIST<CDiscordUser> arMarkReadQueue;

	OBJLIST<CDiscordUser> arUsers;
	OBJLIST<COwnMessage> arOwnMessages;
	OBJLIST<CDiscordVoiceCall> arVoiceCalls;

	CDiscordUser* FindUser(SnowFlake id);
	CDiscordUser* FindUser(const wchar_t *pwszUsername, int iDiscriminator);
	CDiscordUser* FindUserByChannel(SnowFlake channelId);

	void          PreparePrivateChannel(const JSONNode &);
	CDiscordUser* PrepareUser(const JSONNode &);

	//////////////////////////////////////////////////////////////////////////////////////
	// menu items

	void InitMenus(void);

	int __cdecl OnMenuPrebuild(WPARAM, LPARAM);

	INT_PTR __cdecl OnMenuCopyId(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuCreateChannel(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuJoinGuild(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuLeaveGuild(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuLoadHistory(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuToggleSync(WPARAM, LPARAM);

	HGENMENU m_hMenuLeaveGuild, m_hMenuCreateChannel, m_hMenuToggleSync;

	//////////////////////////////////////////////////////////////////////////////////////
	// guilds

	OBJLIST<CDiscordGuild> arGuilds;

	__forceinline CDiscordGuild* FindGuild(SnowFlake id) const
	{
		return arGuilds.find((CDiscordGuild*)&id);
	}

	void AddGuildUser(CDiscordGuild *guild, const CDiscordGuildMember &pUser);
	void ProcessGuild(const JSONNode &json);
	void ProcessPresence(const JSONNode &json);
	void ProcessRole(CDiscordGuild *guild, const JSONNode &json);
	void ProcessType(CDiscordUser *pUser, const JSONNode &json);

	CDiscordUser* ProcessGuildChannel(CDiscordGuild *guild, const JSONNode &json);
	CDiscordGuildMember* ProcessGuildUser(CDiscordGuild *pGuild, const JSONNode &json, bool *bNew = nullptr);

	//////////////////////////////////////////////////////////////////////////////////////
	// group chats

	int  __cdecl GroupchatEventHook(WPARAM, LPARAM);
	int  __cdecl GroupchatMenuHook(WPARAM, LPARAM);

	void Chat_SendPrivateMessage(GCHOOK *gch);
	void Chat_ProcessLogMenu(GCHOOK *gch);
	void Chat_ProcessNickMenu(GCHOOK* gch);

	void CreateChat(CDiscordGuild *pGuild, CDiscordUser *pUser);
	void ProcessChatUser(CDiscordUser *pChat, const CMStringW &wszUserId, const JSONNode &pRoot);
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

	INT_PTR  GetCaps(int, MCONTACT = 0) override;

	HWND     CreateExtendedSearchUI(HWND owner) override;
	HWND     SearchAdvanced(HWND owner) override;

	HANDLE   SearchBasic(const wchar_t *id) override;
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent(int flags, int, MEVENT hDbEvent) override;
	
	int      AuthRecv(MCONTACT, PROTORECVEVENT *pre) override;
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t*) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int iStatus, const wchar_t *msg) override;

	int      SendMsg(MCONTACT hContact, int flags, const char *pszSrc) override;

	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	int      SetStatus(int iNewStatus) override;

	void     OnBuildProtoMenu() override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

	INT_PTR __cdecl RequestFriendship(WPARAM, LPARAM);
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	INT_PTR __cdecl VoiceCaps(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// Events

	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnAccountChanged(WPARAM, LPARAM);
	int  __cdecl OnDbEventRead(WPARAM, LPARAM);
	
	int  __cdecl OnVoiceState(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// dispatch commands

	void OnCommandCallCreated(const JSONNode &json);
	void OnCommandCallDeleted(const JSONNode &json);
	void OnCommandCallUpdated(const JSONNode &json);
	void OnCommandChannelCreated(const JSONNode &json);
	void OnCommandChannelDeleted(const JSONNode &json);
	void OnCommandChannelUpdated(const JSONNode &json);
	void OnCommandGuildCreated(const JSONNode &json);
	void OnCommandGuildDeleted(const JSONNode &json);
	void OnCommandGuildMemberAdded(const JSONNode &json);
	void OnCommandGuildMemberListUpdate(const JSONNode &json);
	void OnCommandGuildMemberRemoved(const JSONNode &json);
	void OnCommandGuildMemberUpdated(const JSONNode &json);
	void OnCommandFriendAdded(const JSONNode &json);
	void OnCommandFriendRemoved(const JSONNode &json);
	void OnCommandMessage(const JSONNode&, bool);
	void OnCommandMessageCreate(const JSONNode &json);
	void OnCommandMessageDelete(const JSONNode &json);
	void OnCommandMessageUpdate(const JSONNode &json);
	void OnCommandMessageAck(const JSONNode &json);
	void OnCommandPresence(const JSONNode &json);
	void OnCommandReady(const JSONNode &json);
	void OnCommandRoleCreated(const JSONNode &json);
	void OnCommandRoleDeleted(const JSONNode &json);
	void OnCommandTyping(const JSONNode &json);
	void OnCommandUserUpdate(const JSONNode &json);
	void OnCommandUserSettingsUpdate(const JSONNode &json);

	void OnLoggedIn();
	void OnLoggedOut();
	
	void OnReceiveCreateChannel(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveFile(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveGateway(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveMarkRead(NETLIBHTTPREQUEST *, AsyncHttpRequest *);
	void OnReceiveMessageAck(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveToken(NETLIBHTTPREQUEST *, AsyncHttpRequest *);
	void OnReceiveUserinfo(NETLIBHTTPREQUEST *, AsyncHttpRequest *);

	void RetrieveMyInfo();
	void OnReceiveMyInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void RetrieveHistory(CDiscordUser *pUser, CDiscordHistoryOp iOp = MSG_NOFILTER, SnowFlake msgid = 0, int iLimit = 50);
	void OnReceiveHistory(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	bool RetrieveAvatar(MCONTACT hContact);
	void OnReceiveAvatar(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void OnSendMsg(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	//////////////////////////////////////////////////////////////////////////////////////
	// Misc

	void SendMarkRead(void);
	void SetServerStatus(int iStatus);
	void RemoveFriend(SnowFlake id);

	CMStringW GetAvatarFilename(MCONTACT hContact);
	void CheckAvatarChange(MCONTACT hContact, const CMStringW &wszNewHash);
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDiscordProto>
{
	CMPlugin();

	bool bVoiceService = false;

	int Load() override;
};
