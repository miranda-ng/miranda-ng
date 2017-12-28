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
	PROTOSEARCHRESULT hdr;
	JSONNode *data;
};

enum
{
	CMI_AUTH_REQUEST,
	//CMI_AUTH_GRANT,
	//CMI_AUTH_REVOKE,
	CMI_BLOCK,
	CMI_JOIN_GAME,
	SMI_BLOCKED_LIST,
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
};

class CSteamProto : public PROTO<CSteamProto>
{
	friend CSteamPasswordEditor;
	friend CSteamOptionsMain;
	friend CSteamOptionsBlockList;

public:
	// PROTO_INTERFACE
	CSteamProto(const char *protoName, const wchar_t *userName);
	~CSteamProto();

	// PROTO_INTERFACE
	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT *psr);

	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const wchar_t *szReason);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const wchar_t * szMessage);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	HANDLE    __cdecl SearchBasic(const wchar_t *id);
	virtual HANDLE    __cdecl SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName);

	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	// instances
	static CSteamProto* InitProtoInstance(const char* protoName, const wchar_t* userName);
	static int UninitProtoInstance(CSteamProto* ppro);

	static CSteamProto* GetContactProtoInstance(MCONTACT hContact);
	static void UninitProtoInstances();

	// menus
	static void InitMenus();
	static void UninitMenus();

protected:
	wchar_t *password;
	bool isLoginAgain;
	time_t m_idleTS;
	bool isTerminated, isConnected;
	mir_cs requestQueueLock;
	HANDLE hRequestsQueueEvent;
	HANDLE hRequestQueueThread;
	LIST<RequestQueueItem> requestQueue;
	HANDLE m_pollingConnection, m_hPollingThread;
	ULONG  hAuthProcess;
	ULONG  hMessageProcess;
	mir_cs contact_search_lock;
	mir_cs requests_queue_lock;
	mir_cs set_status_lock;
	std::map<HANDLE, time_t> m_mpOutMessages;
	std::map<std::string, time_t> m_typingTimestamps;

	/**
	 * Used only to compare in steam_history.cpp, others should write such value directly to db profile, because PollingThread
	 * may start sooner than steam_history requests so it could possibly break getting history messages from server
	 */
	time_t m_lastMessageTS;

	// instances
	static LIST<CSteamProto> InstanceList;
	static int CompareProtos(const CSteamProto *p1, const CSteamProto *p2);

	// requests
	HttpResponse* SendRequest(HttpRequest *request);
	void SendRequest(HttpRequest *request, HttpCallback callback, void *param = NULL);
	void SendRequest(HttpRequest *request, JsonCallback callback, void *param = NULL);
	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, HttpCallback callback, void *param = NULL);
	void PushRequest(HttpRequest *request, JsonCallback callback, void *param = NULL);
	void __cdecl RequestQueueThread(void*);

	// pooling thread
	void ParsePollData(const JSONNode &data);
	void OnGotPoll(const HttpResponse &response, void *arg);
	void __cdecl PollingThread(void*);

	// login
	bool IsOnline();
	bool IsMe(const char *steamId);
	
	void Login();
	bool Relogin();
	void LogOut();

	void OnGotRsaKey(const JSONNode &root, void*);

	void OnGotCaptcha(const HttpResponse &response, void *arg);
	
	void OnAuthorization(const HttpResponse &response, void*);
	void OnAuthorizationError(const JSONNode &root);
	void OnAuthorizationSuccess(const JSONNode &root);
	void OnGotSession(const HttpResponse &response, void*);

	void OnLoggedOn(const HttpResponse &response, void*);

	void HandleTokenExpired();
	void DeleteAuthSettings();

	// contacts
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void UpdateContactDetails(MCONTACT hContact, const JSONNode &data);
	void UpdateContactRelationship(MCONTACT hContact, const JSONNode &data);
	void OnGotAppInfo(const JSONNode &root, void *arg);

	void ContactIsRemoved(MCONTACT hContact);
	void ContactIsFriend(MCONTACT hContact);
	void ContactIsIgnored(MCONTACT hContact);
	void ContactIsAskingAuth(MCONTACT hContact);

	MCONTACT FindContact(const char *steamId);
	MCONTACT AddContact(const char *steamId, bool isTemporary = false);

	void OnGotFriendList(const JSONNode &root, void*);
	void OnGotBlockList(const JSONNode &root, void*);
	void OnGotUserSummaries(const JSONNode &root, void*);
	void OnGotAvatar(const HttpResponse &response, void *arg);

	void OnFriendAdded(const HttpResponse &response, void *arg);
	void OnFriendBlocked(const HttpResponse &response, void *arg);
	void OnFriendRemoved(const HttpResponse &response, void *arg);

	void OnAuthRequested(const JSONNode &root, void *arg);

	void OnPendingApproved(const JSONNode &root, void *arg);
	void OnPendingIgnoreded(const JSONNode &root, void *arg);

	void OnSearchResults(const HttpResponse &response, void *arg);
	void OnSearchByNameStarted(const HttpResponse &response, void *arg);

	// messages
	int OnSendMessage(MCONTACT hContact, const char* message);
	void OnMessageSent(const HttpResponse &response, void *arg);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	// history
	void OnGotConversations(const JSONNode &root, void *arg);
	void OnGotHistoryMessages(const JSONNode &root, void*);

	// menus
	static int hChooserMenu;
	static HGENMENU contactMenuItems[CMI_MAX];

	int __cdecl AuthRequestCommand(WPARAM, LPARAM);
	int __cdecl BlockCommand(WPARAM, LPARAM);
	int __cdecl JoinToGameCommand(WPARAM, LPARAM);

	INT_PTR __cdecl OpenBlockListCommand(WPARAM, LPARAM);

	static INT_PTR MenuChooseService(WPARAM wParam, LPARAM lParam);

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

	HANDLE __cdecl GetAwayMsg(MCONTACT hContact);
	void __cdecl GetAwayMsgThread(void *arg);

	// events
	int OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnIdleChanged(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	// utils
	static WORD SteamToMirandaStatus(int state);
	static int MirandaToSteamState(int status);

	static int RsaEncrypt(const char *pszModulus, DWORD &exponent, const char *data, BYTE *encrypted, DWORD &encryptedSize);

	static void CSteamProto::ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	static void CSteamProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	INT_PTR __cdecl OnGetEventTextChatStates(WPARAM wParam, LPARAM lParam);

	// helpers
	inline int IdleSeconds() {
		// Based on idle time we report Steam server will mark us as online/away/snooze
		switch (this->m_iStatus) {
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
};

int OnReloadIcons(WPARAM wParam, LPARAM lParam);
void SetContactExtraIcon(MCONTACT hContact, int status);

#endif //_STEAM_PROTO_H_