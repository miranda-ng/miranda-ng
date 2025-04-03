#define TEAMS_CLIENT_ID  "8ec6bc83-69c8-4392-8f08-b3c986009232"
#define TEAMS_CLIENTINFO_NAME "skypeteams"
#define TEAMS_CLIENTINFO_VERSION "49/24062722442"

#define TEAMS_USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36 Edg/126.0.0.0 Teams/24165.1410.2974.6689/49"

#define DBKEY_ID       "id"
#define DBKEY_GROUP    "DefaultGroup"

struct COwnMessage
{
	COwnMessage(int _1, int64_t _2) :
		hMessage(_1),
		hClientMessageId(_2)
	{}

	int hMessage;
	int64_t hClientMessageId, iTimestamp = -1;
};

struct CSkypeTransfer
{
	CMStringA docId, fileName, fileType, url;
	int iFileSize = 0, iWidth = -1, iHeight = -1;
};

class CTeamsProto : public PROTO<CTeamsProto>
{
	friend class COptionsMainDlg;
	friend class CDeviceCodeDlg;

	friend class CSkypeOptionsMain;
	friend class CSkypeGCCreateDlg;
	friend class CSkypeInviteDlg;
	friend class CMoodDialog;
	friend class CDeviceCodeDlg;

	class CTeamsProtoImpl
	{
		friend class CTeamsProto;
		CTeamsProto &m_proto;

		CTimer m_heartBeat, m_loginPoll;
		void OnHeartBeat(CTimer *)
		{
			m_proto.TRouterSendJson("ping");
		}
		void OnLoginPoll(CTimer *)
		{
			m_proto.LoginPoll();
		}

		CTeamsProtoImpl(CTeamsProto &pro) :
			m_proto(pro),
			m_heartBeat(Miranda_GetSystemWindow(), UINT_PTR(this) + 1),
			m_loginPoll(Miranda_GetSystemWindow(), UINT_PTR(this) + 2)
		{
			m_heartBeat.OnEvent = Callback(this, &CTeamsProtoImpl::OnHeartBeat);
			m_loginPoll.OnEvent = Callback(this, &CTeamsProtoImpl::OnLoginPoll);
		}
	} m_impl;

	// http queue
	bool m_isTerminated = true;
	mir_cs m_requestQueueLock;
	LIST<AsyncHttpRequest> m_requests;
	MEventHandle m_hRequestQueueEvent;
	HANDLE m_hRequestQueueThread;
	CMStringA m_szAccessToken, m_szSubstrateToken;

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

	void SendCreateEndpoint();
	void SendPresence();
	
	void OauthRefreshServices();
	void RefreshToken(const char *pszScope, AsyncHttpRequest::MTHttpRequestHandler pFunc);

	void OnReceiveSkypeToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnReceiveDevicePoll(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnReceiveDeviceToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnRefreshAccessToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnRefreshSkypeToken(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnRefreshSubstrate(MHttpResponse *response, AsyncHttpRequest *pRequest);

public:
	// constructor
	CTeamsProto(const char *protoName, const wchar_t *userName);
	~CTeamsProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;
	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t* szMessage) override;
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
	int      AuthRecv(MCONTACT hContact, DB::EventInfo &dbei) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;
	HANDLE   SearchBasic(const wchar_t* id) override;
	int      SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;
	int      RecvContacts(MCONTACT hContact, DB::EventInfo &dbei) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;

	void     OnBuildProtoMenu(void) override;
	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnEventEdited(MCONTACT hContact, MEVENT hDbEvent, const DBEVENTINFO &dbei) override;
	void     OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;
	void     OnReceiveOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob) override;
	void     OnShutdown() override;

	// menus
	static void InitMenus();

	// popups
	void InitPopups();
	void UninitPopups();

	// search
	void __cdecl SearchBasicThread(void *param);

	//////////////////////////////////////////////////////////////////////////////////////
	// settings

	CMOption<bool> m_bAutoHistorySync;
	CMOption<bool> m_bUseBBCodes;

	CMOption<bool> m_bUseHostnameAsPlace;
	CMOption<wchar_t*> m_wstrPlace;

	CMOption<wchar_t*> m_wstrCListGroup;

	CMOption<uint8_t> m_iMood;
	CMOption<wchar_t*> m_wstrMoodMessage, m_wstrMoodEmoji;

	//////////////////////////////////////////////////////////////////////////////////////
	// other data

	int m_iPollingId, m_iMessageId = 1;
	ptrA m_szToken, m_szEndpoint, m_szOwnSkypeId;
	CMStringA m_szSkypename, m_szMyname, m_szSkypeToken;
	MCONTACT m_hMyContact;

	__forceinline CMStringA getId(MCONTACT hContact) {
		return getMStringA(hContact, DBKEY_ID);
	}

	void OnSearch(MHttpResponse *response, AsyncHttpRequest *pRequest);

	// login
	void OnSubscriptionsCreated(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnCapabilitiesSended(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnStatusChanged(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnEndpointCreated(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnEndpointDeleted(MHttpResponse *response, AsyncHttpRequest *pRequest);

	// oauth
	void OnOAuthStart(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnOAuthConfirm(MHttpResponse* response, AsyncHttpRequest* pRequest);
	void OnOAuthAuthorize(MHttpResponse* response, AsyncHttpRequest* pRequest);
	void OnOAuthEnd(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnASMObjectCreated(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnASMObjectUploaded(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void LoadContactsAuth(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void LoadContactList(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnBlockContact(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnUnblockContact(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnReceiveAvatar(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnSentAvatar(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnMessageSent(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnGetServerHistory(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnSyncConversations(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnGetChatInfo(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnGetChatMembers(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void LoadProfile(MHttpResponse *response, AsyncHttpRequest *pRequest);

	static INT_PTR __cdecl GlobalParseSkypeUriService(WPARAM, LPARAM lParam);

private:
	bool m_bHistorySynced;

	static std::map<std::wstring, std::wstring> languages;

	LIST<void> m_PopupClasses;
	OBJLIST<COwnMessage> m_OutMessages;

	// locks
	mir_cs m_lckOutMessagesList;
	mir_cs messageSyncLock;
	mir_cs m_StatusLock;

	// avatars
	void SetAvatarUrl(MCONTACT hContact, const CMStringW &tszUrl);
	bool ReceiveAvatar(MCONTACT hContact);
	void ReloadAvatarInfo(MCONTACT hContact);
	void GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen);

	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// profile
	void UpdateProfileDisplayName(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileGender(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileBirthday(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileCountry(const JSONNode &node, MCONTACT hContact = NULL);
	void UpdateProfileEmails(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact = NULL);

	// contacts
	uint16_t GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, uint16_t status);

	MCONTACT FindContact(const char *skypeId);
	MCONTACT FindContact(const wchar_t *skypeId);

	MCONTACT AddContact(const char *skypename, const char *nick, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	// files
	void SendFile(CFileUploadParam *fup);

	void __cdecl ReceiveFileThread(void *param);

	INT_PTR __cdecl SvcOfflineFile(WPARAM, LPARAM);

	// messages
	std::map<ULONGLONG, HANDLE> m_mpOutMessagesIds;

	int SendServerMsg(MCONTACT hContact, const char *szMessage, int64_t iMessageId = 0);

	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void ProcessContactRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei);
	void ProcessFileRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei);

	// chats
	void InitGroupChatModule();

	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);
	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);

	SESSION_INFO* StartChatRoom(const wchar_t *tid, const wchar_t *tname, const char *pszVersion = nullptr);

	bool OnChatEvent(const JSONNode &node);
	wchar_t* GetChatContactNick(SESSION_INFO *si, const wchar_t *id, const wchar_t *name = nullptr, bool *isQualified = nullptr);

	bool AddChatContact(SESSION_INFO *si, const wchar_t *id, const wchar_t *role, bool isChange = false);
	void RemoveChatContact(SESSION_INFO *si, const wchar_t *id, const wchar_t *initiator = L"");
	void SendChatMessage(SESSION_INFO *si, const wchar_t *tszMessage);

	void KickChatUser(const char *chatId, const char *userId);

	void SetChatStatus(MCONTACT hContact, int iStatus);

	bool ParseMessage(const JSONNode &node, DB::EventInfo &dbei);
	void ParsePollData(const char*);
	// utils
	template <typename T>
	__inline static void FreeList(const LIST<T> &lst)
	{
		for (auto &it : lst)
			mir_free(it);
	}

	__forceinline bool IsOnline() const
	{	return (m_iStatus > ID_STATUS_OFFLINE);
	}

	bool IsMe(const wchar_t *str);
	bool IsMe(const char *str);

	int64_t getLastTime(MCONTACT);
	void setLastTime(MCONTACT, int64_t);

	CMStringW RemoveHtml(const CMStringW &src, bool bCheckSS = false);

	static time_t IsoToUnixTime(const std::string &stamp);

	static int SkypeToMirandaStatus(const char *status);
	static const char *MirandaToSkypeStatus(int status);

	void ShowNotification(const wchar_t *message, MCONTACT hContact = NULL);
	void ShowNotification(const wchar_t *caption, const wchar_t *message, MCONTACT hContact = NULL, int type = 0);
	static bool IsFileExists(std::wstring path);

	static LRESULT CALLBACK PopupDlgProcCall(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetString(MCONTACT hContact, const char *pszSetting, const JSONNode &node);

	CMStringW ChangeTopicForm();

	// services
	INT_PTR __cdecl BlockContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl UnblockContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcLoadHistory(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcEmptyHistory(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcCreateChat(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMood(WPARAM, LPARAM);
	INT_PTR __cdecl ParseSkypeUriService(WPARAM, LPARAM lParam);

	template<INT_PTR(__cdecl CTeamsProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		auto *proto = CMPlugin::getInstance((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}

	// trouter
public:
	void TRouterProcess(const char *str);

private:
	CMStringA m_szTrouterUrl, m_szTrouterSurl;
	WebSocket<CTeamsProto> *m_ws;
	MHttpHeaders m_connectParams;
	int iCommandId;

	void __cdecl GatewayThread(void *);

	void TRouterSendJson(const char *szName, const JSONNode *node = 0);
	void TRouterSendJson(const JSONNode &node);

	void TRouterSendAuthentication();
	void TRouterSendActive(bool);
	void TRouterRegister();
	void TRouterRegister(const char *pszAppId, const char *pszKey, const char *pszPath);

	void StartTrouter();
	void StopTrouter();

	void OnTrouterInfo(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnTrouterSession(MHttpResponse *response, AsyncHttpRequest *pRequest);
};

typedef CProtoDlgBase<CTeamsProto> CTeamsDlgBase;

struct CMPlugin : public ACCPROTOPLUGIN<CTeamsProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
