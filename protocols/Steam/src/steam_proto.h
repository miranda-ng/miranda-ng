#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#define STEAM_SEARCH_BYID 1001
#define STEAM_SEARCH_BYNAME 1002
#define STEAM_TYPING_TIME 10

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

typedef void(CSteamProto::*HttpCallback)(const HttpResponse&, void*);
typedef void(CSteamProto::*JsonCallback)(const JSONNode&, void*);

struct RequestQueueItem
{
	HttpRequest *request;
	HttpCallback httpCallback;
	JsonCallback jsonCallback;
	void *param;

	RequestQueueItem(HttpRequest *request)
		: request(request), httpCallback(nullptr), jsonCallback(nullptr), param(nullptr)
	{
	}
	RequestQueueItem(HttpRequest *request, HttpCallback callback, void *param)
		: request(request), httpCallback(callback), jsonCallback(nullptr), param(param)
	{
	}
	RequestQueueItem(HttpRequest *request, JsonCallback callback, void *param)
		: request(request), httpCallback(nullptr), jsonCallback(callback), param(param)
	{
	}
};

class CSteamProto : public PROTO<CSteamProto>
{
	friend class CSteamGuardDialog;
	friend class CSteamPasswordEditor;
	friend class CSteamOptionsMain;
	friend class CSteamOptionsBlockList;
	friend class PollRequest;

	ptrW m_password;
	ptrW m_defaultGroup;
	bool isLoginAgain;
	time_t m_idleTS;
	HWND m_hwndGuard;

	// requests
	bool m_isTerminated;
	mir_cs m_requestQueueLock;
	HANDLE m_hRequestsQueueEvent;
	HANDLE m_hRequestQueueThread;
	LIST<RequestQueueItem> m_requestQueue;

	// polling
	HANDLE m_hPollingThread;
	ULONG hAuthProcess;
	ULONG hMessageProcess;
	mir_cs m_addContactLock;
	mir_cs m_setStatusLock;
	std::map<HANDLE, time_t> m_mpOutMessages;
	std::map<std::string, time_t> m_typingTimestamps;

public:
	// PROTO_INTERFACE
	CSteamProto(const char *protoName, const wchar_t *userName);
	~CSteamProto();

	// PROTO_INTERFACE
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthRecv(MCONTACT, PROTORECVEVENT*) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	HANDLE   GetAwayMsg(MCONTACT hContact) override;

	HANDLE   SearchBasic(const wchar_t *id) override;
	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;

	int      SendMsg(MCONTACT hContact, int flags, const char *msg) override;

	int      SetStatus(int iNewStatus) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;

	// menus
	static void InitMenus();

protected:
	// requests
	void SendRequest(HttpRequest *request);
	void SendRequest(HttpRequest *request, HttpCallback callback, void *param = nullptr);
	void SendRequest(HttpRequest *request, JsonCallback callback, void *param = nullptr);
	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, HttpCallback callback, void *param = nullptr);
	void PushRequest(HttpRequest *request, JsonCallback callback, void *param = nullptr);
	void PushToRequestQueue(RequestQueueItem *item);
	RequestQueueItem *PopFromRequestQueue();
	void ProcessRequestQueue();
	void __cdecl RequestQueueThread(void*);

	// pooling
	void ParsePollData(const JSONNode &data);
	void OnGotPoll(const HttpResponse &response, void *arg);
	void __cdecl PollingThread(void*);

	// login
	bool IsOnline();
	bool IsMe(const char *steamId);

	void Login();
	void Logout();

	void OnGotRsaKey(const JSONNode &root, void*);

	void OnGotCaptcha(const HttpResponse &response, void *arg);
	
	void OnAuthorization(const HttpResponse &response, void*);
	void OnAuthorizationError(const JSONNode &root);
	void OnAuthorizationSuccess(const JSONNode &root);
	void OnGotSession(const HttpResponse &response, void*);

	void OnLoggedOn(const HttpResponse &response, void*);
	void OnReLogin(const JSONNode &root, void*);

	void HandleTokenExpired();
	void DeleteAuthSettings();

	// contacts
	void SetAllContactStatuses(int status);
	void SetContactStatus(MCONTACT hContact, uint16_t status);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void UpdateContactDetails(MCONTACT hContact, const JSONNode &data);
	void UpdateContactRelationship(MCONTACT hContact, const JSONNode &data);
	void OnGotAppInfo(const JSONNode &root, void *arg);

	void ContactIsRemoved(MCONTACT hContact);
	void ContactIsFriend(MCONTACT hContact);
	void ContactIsBlocked(MCONTACT hContact);
	void ContactIsUnblocked(MCONTACT hContact);
	void ContactIsAskingAuth(MCONTACT hContact);

	MCONTACT GetContact(const char *steamId);
	MCONTACT AddContact(const char *steamId, const wchar_t *nick = nullptr, bool isTemporary = false);

	void OnGotFriendList(const JSONNode &root, void*);
	void OnGotBlockList(const JSONNode &root, void*);
	void OnGotUserSummaries(const JSONNode &root, void*);
	void OnGotAvatar(const HttpResponse &response, void *arg);

	void OnFriendAdded(const HttpResponse &response, void *arg);
	void OnFriendBlocked(const HttpResponse &response, void *arg);
	void OnFriendUnblocked(const HttpResponse &response, void *arg);
	void OnFriendRemoved(const HttpResponse &response, void *arg);

	void OnAuthRequested(const JSONNode &root, void *arg);

	void OnPendingApproved(const JSONNode &root, void *arg);
	void OnPendingIgnoreded(const JSONNode &root, void *arg);

	void OnSearchResults(const HttpResponse &response, void *arg);
	void OnSearchByNameStarted(const HttpResponse &response, void *arg);

	// messages
	int OnSendMessage(MCONTACT hContact, const char *message);
	void OnMessageSent(const HttpResponse &response, void *arg);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	// history
	void OnGotConversations(const JSONNode &root, void *arg);
	void OnGotHistoryMessages(const JSONNode &root, void*);

	// menus
	static int hChooserMenu;
	static HGENMENU contactMenuItems[CMI_MAX];

	INT_PTR __cdecl AuthRequestCommand(WPARAM, LPARAM);
	INT_PTR __cdecl AuthRevokeCommand(WPARAM, LPARAM);

	int __cdecl BlockCommand(WPARAM, LPARAM);
	int __cdecl UnblockCommand(WPARAM, LPARAM);
	int __cdecl JoinToGameCommand(WPARAM, LPARAM);

	INT_PTR __cdecl OpenBlockListCommand(WPARAM, LPARAM);

	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM);

	void OnInitStatusMenu();

	// avatars
	wchar_t* GetAvatarFilePath(MCONTACT hContact);
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATION &pai);
	void CheckAvatarChange(MCONTACT hContact, std::string avatarUrl);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);

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
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	// utils
	static uint16_t SteamToMirandaStatus(PersonaState state);
	static PersonaState MirandaToSteamState(int status);

	static int RsaEncrypt(const char *pszModulus, DWORD &exponent, const char *data, uint8_t *encrypted, DWORD &encryptedSize);

	static void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	INT_PTR __cdecl OnGetEventTextChatStates(WPARAM wParam, LPARAM lParam);

	// helpers
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

	inline const char *AccountIdToSteamId(long long accountId)
	{
		static char steamId[20];
		mir_snprintf(steamId, "%llu", accountId + 76561197960265728ll);
		return steamId;
	}

	inline const char *SteamIdToAccountId(long long steamId)
	{
		static char accountId[10];
		mir_snprintf(accountId, "%llu", steamId - 76561197960265728ll);
		return accountId;
	}
};

struct CMPlugin : public ACCPROTOPLUGIN<CSteamProto>
{
	CMPlugin();

	int Load() override;
};

int OnReloadIcons(WPARAM wParam, LPARAM lParam);
void SetContactExtraIcon(MCONTACT hContact, int status);

#endif //_STEAM_PROTO_H_
