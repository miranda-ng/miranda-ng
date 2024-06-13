#pragma once

enum Permission : uint64_t
{
	CREATE_INVITE      = (1ll << 0),  // Allows creation of instant invites
	KICK_MEMBERS       = (1ll << 1),  // Allows kicking members
	BAN_MEMBERS        = (1ll << 2),  // Allows banning members
	ADMIN              = (1ll << 3),  // Allows all permissions and bypasses channel permission overwrites
	MANAGE_CHANNELS    = (1ll << 4),  // Allows management and editing of channels
	MANAGE_GUILD       = (1ll << 5),  // Allows management and editing of the guild	
	ADD_REACTIONS      = (1ll << 6),  // Allows for the addition of reactions to messages
	VIEW_AUDIT_LOG     = (1ll << 7),  // Allows for viewing of audit logs
	PRIORITY_SPEAKER   = (1ll << 8),  // Allows for using priority speaker in a voice channel
	STREAM             = (1ll << 9),  // Allows the user to go live
	VIEW_CHANNEL       = (1ll << 10), // Allows guild members to view a channel, which includes reading messages in text channels and joining voice channels
	SEND_MESSAGES      = (1ll << 11), // Allows for sending messages in a channel and creating threads in a forum (does not allow sending messages in threads)
	SEND_TTS_MESSAGES  = (1ll << 12), // Allows for sending of /tts messages	
	MANAGE_MESSAGES    = (1ll << 13), // Allows for deletion of other users messages	
	EMBED_LINKS        = (1ll << 14), // Links sent by users with this permission will be auto-embedded
	ATTACH_FILES       = (1ll << 15), // Allows for uploading images and files
	READ_HISTORY       = (1ll << 16), // Allows for reading of message history
	MENTION_EVERYONE   = (1ll << 17), // Allows for using the @everyone tag to notify all users in a channel, and the @here tag to notify all online users in a channel
	USE_EXT_EMOJI      = (1ll << 18), // Allows the usage of custom emojis from other servers
	VIEW_INSIGHTS      = (1ll << 19), // Allows for viewing guild insights	
	VOICE_CONNECT      = (1ll << 20), // Allows for joining of a voice channel	
	VOICE_SPEAK 		 = (1ll << 21), // Allows for speaking in a voice channel	
	VOICE_MUTE 			 = (1ll << 22), // Allows for muting members in a voice channel	
	VOICE_DEAFEN		 = (1ll << 23), // Allows for deafening of members in a voice channel	
	VOICE_MOVE 			 = (1ll << 24), // Allows for moving of members between voice channels	
	USE_VAD            = (1ll << 25), // Allows for using voice-activity-detection in a voice channel
	CHANGE_NICKNAME    = (1ll << 26), // Allows for modification of own nickname	
	MANAGE_NICKS 		 = (1ll << 27), // Allows for modification of other users nicknames	
	MANAGE_ROLES 		 = (1ll << 28), // Allows management and editing of roles	
	MANAGE_WEBHOOKS    = (1ll << 29), // 
	MANAGE_EMOJI 		 = (1ll << 30), // Allows for editing and deleting emojis, stickers, and soundboard sounds created by all users
	USE_APP_COMMANDS   = (1ll << 31), // Allows members to use application commands, including slash commands and context menu commands.
	REQUEST_TO_SPEAK   = (1ll << 32), // Allows for requesting to speak in stage channels.
	MANAGE_EVENTS		 = (1ll << 33), // Allows for editing and deleting scheduled events created by all users
	MANAGE_THREADS		 = (1ll << 34), // Allows for deleting and archiving threads, and viewing all private threads
	PUBLIC_THREADS		 = (1ll << 35), // Allows for creating public and announcement threads
	PRIVATE_THREADS	 = (1ll << 36), // Allows for creating private threads	
	USE_EXT_STICKERS	 = (1ll << 37), // Allows the usage of custom stickers from other servers
	SEND_THREADS		 = (1ll << 38), // Allows for sending messages in threads	
	EMBED_ACTIVITY		 = (1ll << 39), // Allows for using Activities (applications with the EMBEDDED flag) in a voice channel
	MODERATE_MEMBERS	 = (1ll << 40), // Allows for timing out users to prevent them from sending or reacting to messages in chat and threads, and from speaking in voice and stage channels	
	VIEW_MONETIZATION  = (1ll << 41), // Allows for viewing role subscription insights	
	USE_SOUNDBOARD		 = (1ll << 42), // Allows for using soundboard in a voice channel	
	CREATE_EXPRESSIONS = (1ll << 43), // Allows for creating emojis, stickers, and soundboard sounds, and editing and deleting those created by the current user.
	CREATE_EVEMTS 		 = (1ll << 44), // Allows for creating scheduled events, and editing and deleting those created by the current user.
	USE_EXT_SOUNDS		 = (1ll << 45), // Allows the usage of custom soundboard sounds from other servers	
	SEND_VOICE			 = (1ll << 46), // Allows sending voice messages	
	ALL                = -1ll
};							

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
	JsonReply(MHttpResponse *);
	~JsonReply();

	__forceinline int error() const { return m_errorCode; }
	__forceinline JSONNode& data() const { return *m_root; }
	__forceinline operator bool() const { return m_errorCode == 200; }
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CDiscordRole : public MZeroedObject
{
	SnowFlake id;
	uint64_t permissions;
	COLORREF color;
	int position;
	bool bIsMe;
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
	bool      bIsVoice;
	bool      bSynced;

	struct CDiscordGuild *pGuild;
	SESSION_INFO *si;

	CMStringW wszUsername, wszChannelName, wszTopic;
	int       iDiscriminator;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CDiscordVoiceState : public MZeroedObject
{
	explicit CDiscordVoiceState(const JSONNode &node);

	SnowFlake m_userId, m_channelId;
	CMStringA m_sessionId;

	union {
		int m_flags;
		struct {
			bool m_bDeaf : 1;      // deafened by the server
			bool m_bMute : 1;      // muted by the server
			bool m_bSelfMute : 1;  // locally muted
			bool m_bSelfDeaf : 1;  // locally deafened
			bool m_nSelfVideo : 1; // camera enabled
			bool m_bSuppress : 1;  // user can't speak
		};
	};
};

/////////////////////////////////////////////////////////////////////////////////////////

class CDiscordVoiceCall : public MZeroedObject
{
	friend class CDiscordProto;

	CDiscordProto *ppro;

	JsonWebSocket<CDiscordVoiceCall> *m_ws;

	CTimer m_timer;
	HNETLIBBIND m_hBind;
	mir_cs m_cs;
	bool m_bTerminated;
	time_t startTime;

	CMStringA m_szIp;
	int m_iPort, m_iSsrc;
	OBJLIST<char> m_arModes;

	OpusEncoder *m_encoder;
	OpusRepacketizer *m_repacketizer;

	void onTimer(CTimer *)
	{
		JSONNode d; d << INT_PARAM("", rand());
		write(3, d);
	}

	static void GetConnection(HNETLIBCONN /*hNewConnection*/, uint32_t /*dwRemoteIP*/, void *pExtra);

public:
	CDiscordVoiceCall(CDiscordProto *pOwner);
	~CDiscordVoiceCall();

	// config
	SnowFlake channelId, guildId;
	CMStringA szSessionId, szToken, szEndpoint;

	__forceinline operator bool() const {
		return !m_bTerminated;
	}

	void write(int op, JSONNode &d);

	void process(const JSONNode &node);
	void processHello(const JSONNode &d);
	void processStreams(const JSONNode &d);
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
	uint64_t  permissions;
	CMStringW wszDiscordId, wszNick, wszRole;
	int iStatus;
};

struct CDiscordGuild : public MZeroedObject
{
	CDiscordGuild(SnowFlake _id);
	~CDiscordGuild();

	__forceinline CDiscordGuildMember* FindUser(SnowFlake userId) {
		return arChatUsers.find((CDiscordGuildMember *)&userId);
	}

	__forceinline CDiscordRole* FindRole(SnowFlake id) {
		return arRoles.find((CDiscordRole *)&id);
	}

	__forceinline CMStringW GetCacheFile() const {
		return CMStringW(FORMAT, L"%s\\DiscordCache\\%lld.json", VARSW(L"%miranda_userdata%").get(), m_id);
	}

	SnowFlake m_id, m_ownerId;
	uint64_t  m_permissions = 0; // my effective permissions
	CMStringW m_wszName;
	MCONTACT  m_hContact;
	MGROUP    m_groupId;
	bool      m_bSynced = false;
	bool      m_bEnableHistory = true;

	SESSION_INFO *pParentSi;
	LIST<CDiscordUser> arChannels;
	OBJLIST<CDiscordGuildMember> arChatUsers;
	OBJLIST<CDiscordRole> arRoles; // guild roles
	OBJLIST<CDiscordVoiceState> arVoiceStates;
	CDiscordVoiceCall *pVoiceCall;

	uint64_t CalcPermissionOverride(SnowFlake myUserId, const JSONNode &json);
	void ProcessRole(const JSONNode &json);

	void LoadFromFile();
	void SaveToFile();
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
	friend class CMfaDialog;
	friend class CGroupchatInviteDlg;
	friend class CDiscordVoiceCall;
	friend class JsonWebSocket<CDiscordProto>;

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

	CDlgBase *pMfaDialog;

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
	JsonWebSocket<CDiscordProto> *m_ws;

	void __cdecl GatewayThread(void*);
	bool  GatewayThreadWorker(void);
	
	bool  GatewaySend(const JSONNode &pNode);

	void  GatewaySendGuildInfo(CDiscordGuild *pGuild);
	void  GatewaySendHeartbeat(void);
	void  GatewaySendIdentify(void);
	void  GatewaySendResume(void);
	bool  GatewaySendStatus(int iStatus, const wchar_t *pwszStatusText);
	bool  GatewaySendVoice(JSONNode &node);

	GatewayHandlerFunc GetHandler(const wchar_t*);

	int   m_iHartbeatInterval;	// in milliseconds
	int   m_iGatewaySeq;       // gateway sequence number

	//////////////////////////////////////////////////////////////////////////////////////
	// options

	CMOption<wchar_t*> m_wszEmail;         // my own email
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<bool>     m_bUseGroupchats;   // Shall we connect Guilds at all?
	CMOption<bool>     m_bHideGroupchats;  // Do not open chat windows on creation
	CMOption<bool>     m_bUseGuildGroups;  // use special subgroups for guilds
	CMOption<bool>     m_bSyncDeleteMsgs;  // delete messages from Miranda if they are deleted at the server
	CMOption<bool>     m_bSyncDeleteUsers; // delete contacts from Miranda if they are deleted at the server
	CMOption<bool>     m_bSyncMarkRead;    // hidden option: send "mark read" packet to server when Miranda displays a message

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

	CMStringW     PrepareMessageText(const JSONNode &pRoot, CDiscordUser *pUser);
	void          PreparePrivateChannel(const JSONNode &);
	CDiscordUser* PrepareUser(const JSONNode &);

	//////////////////////////////////////////////////////////////////////////////////////
	// menu items

	void InitMenus(void);

	int __cdecl OnMenuPrebuild(WPARAM, LPARAM);

	INT_PTR __cdecl OnMenuCopyId(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuCreateChannel(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuDatabaseHistory(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuJoinGuild(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuLeaveGuild(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuLoadHistory(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuToggleSync(WPARAM, LPARAM);

	HGENMENU m_hMenuLeaveGuild, m_hMenuCreateChannel, m_hMenuToggleSync, m_hMenuDatabaseHistory;

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
	void KickChatUser(CDiscordUser *pChat, const wchar_t *pszUID);
	void LeaveChat(CDiscordUser *pChat);
	void MakeChatOwner(CDiscordUser *pChat, const wchar_t *pszUID);
	void ProcessChatUser(CDiscordUser *pChat, SnowFlake userId, const JSONNode &pRoot);
	void ParseSpecialChars(SESSION_INFO *si, CMStringW &str);

	//////////////////////////////////////////////////////////////////////////////////////
	// two-factor auth

	void ShowMfaDialog(const JSONNode &pRoot);
	
	void OnSendTotp(MHttpResponse *, struct AsyncHttpRequest *);

	//////////////////////////////////////////////////////////////////////////////////////
	// voice

	mir_cs m_csVoice;
	OBJLIST<CDiscordVoiceCall> arVoiceCalls;

	void InitVoip(bool bEnable);
	void TryVoiceStart(CDiscordGuild *pGuild);
	void VoiceChannelConnect(MCONTACT hContact);

	CDiscordVoiceCall* FindCall(SnowFlake channelId);

	INT_PTR __cdecl VoiceCaps(WPARAM, LPARAM);
	INT_PTR __cdecl VoiceCanCall(WPARAM, LPARAM);
	INT_PTR __cdecl VoiceCallCreate(WPARAM, LPARAM);
	INT_PTR __cdecl VoiceCallAnswer(WPARAM, LPARAM);
	INT_PTR __cdecl VoiceCallCancel(WPARAM, LPARAM);

	int  __cdecl OnVoiceState(WPARAM, LPARAM);
	
	void __cdecl VoiceClientThread(void *);

	//////////////////////////////////////////////////////////////////////////////////////
	// misc methods

	SnowFlake getId(const char *szName);
	SnowFlake getId(MCONTACT hContact, const char *szName);

	void setId(const char *szName, SnowFlake iValue);
	void setId(MCONTACT hContact, const char *szName, SnowFlake iValue);

	bool surelyGetBool(MCONTACT hContact, const char *szSetting);

public:
	CDiscordProto(const char*,const wchar_t*);
	~CDiscordProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR  GetCaps(int, MCONTACT = 0) override;

	HWND     CreateExtendedSearchUI(HWND owner) override;
	HANDLE   SearchAdvanced(HWND owner) override;

	HANDLE   SearchBasic(const wchar_t *id) override;
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent(int flags, int, MEVENT hDbEvent) override;
	
	int      AuthRecv(MCONTACT, DB::EventInfo &dbei) override;
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t*) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int iStatus, const wchar_t *msg) override;

	int      SendMsg(MCONTACT hContact, MEVENT, const char *pszSrc) override;

	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	int      SetStatus(int iNewStatus) override;

	void     OnBuildProtoMenu() override;
	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	void     OnEventDeleted(MCONTACT, MEVENT, int flags) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

	INT_PTR __cdecl RequestFriendship(WPARAM, LPARAM);

	INT_PTR __cdecl SvcLeaveChat(WPARAM, LPARAM);
	INT_PTR __cdecl SvcEmptyServerHistory(WPARAM, LPARAM);

	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// Events

	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnAccountChanged(WPARAM, LPARAM);
	
	//////////////////////////////////////////////////////////////////////////////////////
	// dispatch commands

	void OnCommandCallCreated(const JSONNode &json);
	void OnCommandCallDeleted(const JSONNode &json);
	void OnCommandCallUpdated(const JSONNode &json);
	void OnCommandChannelCreated(const JSONNode &json);
	void OnCommandChannelDeleted(const JSONNode &json);
	void OnCommandChannelUnreadUpdate(const JSONNode &json);
	void OnCommandChannelUpdated(const JSONNode &json);
	void OnCommandChannelUserAdded(const JSONNode &json);
	void OnCommandChannelUserLeft(const JSONNode &json);
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
	void OnCommandMessageAddReaction(const JSONNode &json);
	void OnCommandMessageRemoveReaction(const JSONNode &json);
	void OnCommandMessageAck(const JSONNode &json);
	void OnCommandPresence(const JSONNode &json);
	void OnCommandReady(const JSONNode &json);
	void OnCommandRoleCreated(const JSONNode &json);
	void OnCommandRoleDeleted(const JSONNode &json);
	void OnCommandTyping(const JSONNode &json);
	void OnCommandUserUpdate(const JSONNode &json);
	void OnCommandUserSettingsUpdate(const JSONNode &json);
	void OnCommandVoiceServerUpdate(const JSONNode &json);
	void OnCommandVoiceStateUpdate(const JSONNode &json);

	void OnLoggedIn();
	void OnLoggedOut();
	
	void OnReceiveCreateChannel(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveFile(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveGateway(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveLogout(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveMarkRead(MHttpResponse*, AsyncHttpRequest *);
	void OnReceiveMessageAck(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveToken(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveUserinfo(MHttpResponse*, AsyncHttpRequest*);

	void RetrieveMyInfo();
	void OnReceiveMyInfo(MHttpResponse*, AsyncHttpRequest*);

	void RetrieveHistory(CDiscordUser *pUser, CDiscordHistoryOp iOp = MSG_NOFILTER, SnowFlake msgid = 0, int iLimit = 50);
	void OnReceiveHistory(MHttpResponse*, AsyncHttpRequest*);

	bool RetrieveAvatar(MCONTACT hContact);
	bool RetrieveChannelAvatar(MCONTACT hContact);
	void OnReceiveAvatar(MHttpResponse*, AsyncHttpRequest*);

	void OnSendMsg(MHttpResponse*, AsyncHttpRequest*);

	//////////////////////////////////////////////////////////////////////////////////////
	// Misc

	void SendMarkRead(void);
	void SetServerStatus(int iStatus);

	void AddFriend(SnowFlake id);
	void RemoveFriend(SnowFlake id);

	CMStringW GetAvatarFilename(MCONTACT hContact);
	void CheckAvatarChange(MCONTACT hContact, const CMStringW &wszNewHash);
};

typedef CProtoDlgBase<CDiscordProto> CDiscordDlgBase;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDiscordProto>
{
	CMPlugin();

	bool bVoiceService = false, bVoiceEnabled = true;

	int Load() override;
};
