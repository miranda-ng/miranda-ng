#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#define STEAM_SEARCH_BYID 1001
#define STEAM_SEARCH_BYNAME 1002

#define STEAM_PROTOCOL_VERSION 65580
#define STEAM_PROTOCOL_MASK 0x80000000

// Global settings for all accounts: hosts' list
#define DBKEY_HOSTS_COUNT  "HostsCount"
#define DBKEY_HOSTS_DATE   "HostsDate"
#define DBKEY_LASTMSG      "LastMessageTS"

#define DBKEY_CLIENT_ID     "ClientID"
#define DBKEY_STEAM_ID      "SteamID"
#define DBKEY_ACCOUNT_NAME  "Username"

// Steam services
#define PollAuthSessionStatus               "Authentication.PollAuthSessionStatus#1"
#define GetPasswordRSAPublicKey             "Authentication.GetPasswordRSAPublicKey#1"
#define BeginAuthSessionViaCredentials      "Authentication.BeginAuthSessionViaCredentials#1"
#define UpdateAuthSessionWithSteamGuardCode "Authentication.UpdateAuthSessionWithSteamGuardCode#1"

#define GetOwnAuthorizedDevices             "DeviceAuth.GetOwnAuthorizedDevices#1"

#define FriendAckMessage                    "FriendMessages.AckMessage#1"
#define FriendSendMessage                   "FriendMessages.SendMessage#1"
#define FriendGetActiveSessions             "FriendMessages.GetActiveMessageSessions#1"
#define FriendGetRecentMessages             "FriendMessages.GetRecentMessages#1"
#define FriendGetIncomingMessage            "FriendMessagesClient.IncomingMessage#1"
#define FriendNotifyAckMessage              "FriendMessagesClient.NotifyAckMessageEcho#1"

#define GetMyChatRoomGroups                 "ChatRoom.GetMyChatRoomGroups#1"
#define GetChatHistory                      "ChatRoom.GetMessageHistory#1"
#define SendChatMessage                     "ChatRoom.SendChatMessage#1"
#define LeaveChatGroup                      "ChatRoom.LeaveChatRoomGroup#1"
#define AckChatMessage                      "ChatRoom.AckChatMessage#1"
#define DeleteChatMessage                   "ChatRoom.DeleteChatMessages#1"


#define NotifyIncomingChatMessage           "ChatRoomClient.NotifyIncomingChatMessage#1"
#define NotifyModifiedChatMessage           "ChatRoomClient.NotifyChatMessageModified#1"
#define NotifyChatGroupUserStateChanged     "ChatRoomClient.NotifyChatGroupUserStateChanged#1"

#define NotificationReceived                "SteamNotificationClient.NotificationsReceived#1"

struct SendAuthParam
{
	MCONTACT hContact;
	HANDLE hAuth;
};

struct STEAM_SEARCH_RESULT
{
	PROTOSEARCHRESULT psr;
	const JSONNode *data;
};

enum
{
	CMI_BLOCK,
	CMI_UNBLOCK,
	CMI_JOIN_GAME,
	CMI_MAX   // this item shall be the last one
};

typedef void (CSteamProto::*MsgCallback)(const uint8_t *pBuf, size_t cbLen);
typedef void (CSteamProto::*HttpCallback)(const MHttpResponse &, void *);
typedef void (CSteamProto::*JsonCallback)(const JSONNode &, void *);

struct HttpRequest : public MTHttpRequest<CSteamProto>
{
	HttpRequest(int iRequestType, const char *pszUrl);

	MHttpRequest* Get();
};

struct ProtoRequest
{
   ProtoRequest(int64_t _1, MsgCallback _2) :
      id(_1),
      pCallback(_2)
   {}

   int64_t id;
   MsgCallback pCallback;
};

struct COwnMessage
{
	COwnMessage(MCONTACT _1, int _2) :
		hContact(_1),
		iMessageId(_2)
	{}

	int iMessageId, timestamp = 0;
	MCONTACT hContact;
};

class CSteamProto : public PROTO<CSteamProto>
{
	friend struct CMPlugin;
	friend class CSteamGuardDialog;
	friend class CSteamPasswordEditor;
	friend class CSteamOptionsMain;
	friend class CSteamOptionsBlockList;
	friend class PollRequest;
	friend class WebSocket<CSteamProto>;

	class CProtoImpl
	{
		friend class CSteamProto;
		CSteamProto &m_proto;

		CTimer m_heartBeat, m_deleteMsg;

		void OnHeartBeat(CTimer *)
		{
			m_proto.SendHeartBeat();
		}

		void OnDeleteMsg(CTimer *)
		{
			m_proto.SendDeleteMessageRequest();
		}

		CProtoImpl(CSteamProto &pro) :
			m_proto(pro),
			m_heartBeat(Miranda_GetSystemWindow(), UINT_PTR(this) + 1),
			m_deleteMsg(Miranda_GetSystemWindow(), UINT_PTR(this) + 2)
		{
			m_heartBeat.OnEvent = Callback(this, &CProtoImpl::OnHeartBeat);
			m_deleteMsg.OnEvent = Callback(this, &CProtoImpl::OnDeleteMsg);
		}
	}
		m_impl;

	ptrW m_password;
	bool m_bTerminated;
	time_t m_idleTS;
	uint64_t m_iSteamId, m_iClientId, m_iSessionId;
	MBinBuffer m_requestId;

	int64_t  GetId(const char *pszSetting);
	void     SetId(const char *pszSetting, int64_t id);

   int64_t  GetId(MCONTACT, const char *pszSetting);
   void     SetId(MCONTACT, const char *pszSetting, int64_t id);

	// polling
	CMStringA m_szRefreshToken, m_szAccessToken;
	ULONG hAuthProcess = 1;
	ULONG hMessageProcess = 1;
	mir_cs m_addContactLock;
	mir_cs m_setStatusLock;

	// connection
	WebSocket<CSteamProto> *m_ws;
	
	mir_cs m_csRequestLock;
	std::map<uint64_t, void *> m_requestInfo;

	void SetRequestInfo(uint64_t, void *);
	void* GetRequestInfo(uint64_t);

	void __cdecl ServerThread(void *);

   void ProcessMulti(const uint8_t *buf, size_t cbLen);
   void ProcessMessage(const uint8_t *buf, size_t cbLen);
	void ProcessServiceResponse(const uint8_t *buf, size_t cbLen, const CMsgProtoBufHeader &hdr);

	void WSSend(EMsg msgType, const ProtobufCppMessage &msg);
	void WSSendRaw(EMsg msgType, const MBinBuffer &buf);
	void WSSendAnon(const char *pszServiceName, const ProtobufCppMessage &msg);
	void WSSendHeader(EMsg msgType, const CMsgProtoBufHeader &hdr, const ProtobufCppMessage &msg);
	void WSSendService(const char *pszServiceName, const ProtobufCppMessage &msg, void *pInfo = nullptr);

	// requests
	bool SendRequest(HttpRequest *request);
	bool SendRequest(HttpRequest *request, HttpCallback callback, void *param = nullptr);
	bool SendRequest(HttpRequest *request, JsonCallback callback, void *param = nullptr);

	void SendAppInfoRequest(uint32_t appId);
	void SendDeviceListRequest();
	void SendHeartBeat();
	void SendLogout();
	void SendPersonaStatus(int iStatus);
	void SendPollRequest();

	// login
	bool IsOnline();

	void Login();
	void Logout();

	static INT_PTR CALLBACK EnterTotpCode(void *param);
	static INT_PTR CALLBACK EnterEmailCode(void *param);

	void OnBeginSession(const CAuthenticationBeginAuthSessionViaCredentialsResponse &pResponse, const CMsgProtoBufHeader &hdr);
	void OnClientLogon(const CMsgClientLogonResponse &pResponse, const CMsgProtoBufHeader &hdr);
	void OnClientLogoff(const CMsgClientLoggedOff &pResponse, const CMsgProtoBufHeader &hdr);
	void OnGotAppInfo(const CMsgClientPICSProductInfoResponse &pResponse, const CMsgProtoBufHeader &hdr);
	void OnGotDeviceList(const CDeviceAuthGetOwnAuthorizedDevicesResponse &pResponse, const CMsgProtoBufHeader &hdr);
	void OnGotRsaKey(const CAuthenticationGetPasswordRSAPublicKeyResponse &pResponse, const CMsgProtoBufHeader &hdr);
	void OnGotConfirmationCode(const CAuthenticationUpdateAuthSessionWithSteamGuardCodeResponse &pResponse, const CMsgProtoBufHeader &hdr);
	void OnPollSession(const CAuthenticationPollAuthSessionStatusResponse &pResponse, const CMsgProtoBufHeader &hdr);

	void OnGotHosts(const JSONNode &root, void *);

	void DeleteAuthSettings();
	void SendConfirmationCode(bool, const char *pszCode);

	// avatars
	wchar_t *GetAvatarFilePath(MCONTACT hContact);
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATION &pai);
	void CheckAvatarChange(MCONTACT hContact, const char *avatarHash);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);

	// chats
	mir_cs m_csChats;
	std::map<uint64_t, SESSION_INFO*> m_chatContactInfo;
	
	MCONTACT m_deletedContact = INVALID_CONTACT_ID;
	std::vector<uint64_t> m_deletedMessages;

	void SendDeleteMessageRequest();

	void SendGetChatsRequest();
	void OnGetMyChats(const CChatRoomGetMyChatRoomGroupsResponse &pResponse, const CMsgProtoBufHeader &hdr);

	void SendGetChatHistory(MCONTACT hContact, uint32_t iLastMsgId);
	void OnGetChatHistory(const CChatRoomGetMessageHistoryResponse &reply, const CMsgProtoBufHeader &hdr);

	void OnGetChatMessage(const CChatRoomIncomingChatMessageNotification &reply, const CMsgProtoBufHeader &hdr);
	void OnLeftChat(const CChatRoomLeaveChatRoomGroupResponse &reply, const CMsgProtoBufHeader &hdr);

	INT_PTR __cdecl SvcLeaveChat(WPARAM, LPARAM);

	int __cdecl GcMenuHook(WPARAM, LPARAM);
	int __cdecl GcEventHook(WPARAM, LPARAM);

	void Chat_LogMenu(GCHOOK *gch);
	void Chat_SendPrivateMessage(GCHOOK *gch);

	// contacts
	void SetAllContactStatuses(int status);
	void SetContactStatus(MCONTACT hContact, uint16_t status);

	void SendUserInfoRequest(uint64_t id);
	void SendUserInfoRequest(const std::vector<uint64_t> &ids);
	
	void SendUserAddRequest(uint64_t id);
	void SendUserRemoveRequest(MCONTACT hContact);
	void SendUserIgnoreRequest(MCONTACT hContact, bool bIgnore);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void UpdateContactRelationship(MCONTACT hContact, uint32_t);

	void ContactIsRemoved(MCONTACT hContact);
	void ContactIsFriend(MCONTACT hContact);
	void ContactIsBlocked(MCONTACT hContact);
	void ContactIsUnblocked(MCONTACT hContact);
	void ContactIsAskingAuth(MCONTACT hContact);

	void OnGotFriendList(const CMsgClientFriendsList &reply, const CMsgProtoBufHeader &hdr);
	void OnGotFriendInfo(const CMsgClientPersonaState &reply, const CMsgProtoBufHeader &hdr);

	MCONTACT GetContact(int64_t steamId);
	MCONTACT AddContact(int64_t steamId, const wchar_t *nick = nullptr, bool isTemporary = false);

	void OnGotBlockList(const JSONNode &root, void *);
	void OnGotAvatar(const MHttpResponse &response, void *arg);

	void OnPendingApproved(const JSONNode &root, void *arg);
	void OnPendingIgnoreded(const JSONNode &root, void *arg);

	void OnSearchResults(const MHttpResponse &response, void *arg);
	void OnSearchByNameStarted(const MHttpResponse &response, void *arg);

	// history
	INT_PTR __cdecl SvcLoadServerHistory(WPARAM, LPARAM);
	INT_PTR __cdecl SvcCanEmptyHistory(WPARAM, LPARAM);
	INT_PTR __cdecl SvcEmptyHistory(WPARAM, LPARAM);

	void SendHistoryRequest(uint64_t accountId, uint32_t startTime);
	void OnGotRecentMessages(const CFriendMessagesGetRecentMessagesResponse &reply, const CMsgProtoBufHeader &hdr);

	void OnGotConversations(const CFriendsMessagesGetActiveMessageSessionsResponse &reply, const CMsgProtoBufHeader &hdr);
	void OnGotHistoryMessages(const CMsgClientChatGetFriendMessageHistoryResponse &reply, const CMsgProtoBufHeader &hdr);

	// menus
	static int hChooserMenu;
	static HGENMENU contactMenuItems[CMI_MAX];

	INT_PTR __cdecl AuthRequestCommand(WPARAM, LPARAM);
	INT_PTR __cdecl AuthRevokeCommand(WPARAM, LPARAM);

	INT_PTR __cdecl BlockCommand(WPARAM, LPARAM);
	INT_PTR __cdecl UnblockCommand(WPARAM, LPARAM);
	INT_PTR __cdecl JoinToGameCommand(WPARAM, LPARAM);

	INT_PTR __cdecl OpenBlockListCommand(WPARAM, LPARAM);

	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM);

	void OnInitStatusMenu();

	// notifications
	void OnGotNotification(const CSteamNotificationNotificationsReceivedNotification &reply, const CMsgProtoBufHeader &hdr);

	// messages
	mir_cs m_csOwnMessages;
	OBJLIST<COwnMessage> m_arOwnMessages;

	void SendFriendMessage(EChatEntryType, int64_t steamId, const char *pszMessage, void *pInfo = nullptr);
	void OnGotIncomingMessage(const CFriendMessagesIncomingMessageNotification &reply, const CMsgProtoBufHeader &hdr);
	void OnGotMarkRead(const CFriendMessagesAckMessageNotification &reply, const CMsgProtoBufHeader &hdr);
	void OnMessageSent(const CFriendMessagesSendMessageResponse &reply, const CMsgProtoBufHeader &hdr);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void SendFriendActiveSessions();

	// xstatuses
	INT_PTR  __cdecl OnGetXStatusEx(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl OnGetXStatusIcon(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl OnRequestAdvStatusIconIdx(WPARAM wParam, LPARAM lParam);
	HICON GetXStatusIcon(int status, UINT flags);
	int GetContactXStatus(MCONTACT hContact);

	void __cdecl GetAwayMsgThread(void *arg);

	// events
	int __cdecl OnIdleChanged(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	INT_PTR __cdecl OnGetEventTextChatStates(WPARAM wParam, LPARAM lParam);

	// helpers
	void OnDoNothing(const CMsgProtoBufHeader &, const CMsgProtoBufHeader &) {}

	void DecodeBbcodes(SESSION_INFO *si, CMStringA &szText);

	inline int IdleSeconds()
	{
		// Based on idle time we report Steam server will mark us as online/away/snooze
		switch (m_iStatus) {
		case ID_STATUS_AWAY:
			return STEAM_API_IDLEOUT_AWAY;
		case ID_STATUS_NA:
			return STEAM_API_IDLEOUT_SNOOZE;
		default:
			return 0;
		}

		// ... or we can report real idle info
		// return m_idleTS ? time(0) - m_idleTS : 0;
	}

public:
	// constructor
	CSteamProto(const char *protoName, const wchar_t *userName);
	~CSteamProto();

	// options
	CMOption<wchar_t*> m_wszGroupName;   // default group for this account's contacts
	CMOption<wchar_t*> m_wszDeviceName;  // how do you see this account in the Device List

	// PROTO_INTERFACE
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthRecv(MCONTACT, DB::EventInfo &dbei) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	HANDLE   GetAwayMsg(MCONTACT hContact) override;

	HANDLE   SearchBasic(const wchar_t *id) override;
	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;

	int      SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;

	int      SetStatus(int iNewStatus) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags) override;
	void     OnMarkRead(MCONTACT hContact, MEVENT hDbEvent) override;
	void     OnModulesLoaded() override;

	// menus
	static void InitMenus();
};

struct CMPlugin : public ACCPROTOPLUGIN<CSteamProto>
{
	CMPlugin();

	void InitSteamServices();

	std::map<EMsg, const ProtobufCMessageDescriptor *> messages;
	std::map<std::string, const ProtobufCServiceDescriptor *> services;

	typedef void (CSteamProto:: *ServiceResponseHandler)(const ProtobufCMessage &msg, const CMsgProtoBufHeader &hdr);
	std::map<EMsg, ServiceResponseHandler> messageHandlers;
	std::map<std::string, ServiceResponseHandler> serviceHandlers;

	int Load() override;
};

int OnReloadIcons(WPARAM wParam, LPARAM lParam);
void SetContactExtraIcon(MCONTACT hContact, int status);

#endif //_STEAM_PROTO_H_
