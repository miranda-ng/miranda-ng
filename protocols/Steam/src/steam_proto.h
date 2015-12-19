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

typedef void(CSteamProto::*SteamResponseCallback)(const HttpResponse *response);
typedef void(CSteamProto::*SteamResponseWithArgCallback)(const HttpResponse *response, void *arg);

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
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR *szReason);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR * szMessage);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR *id);

	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

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
	TCHAR *password;
	bool isLoginAgain;
	time_t m_idleTS;
	HANDLE m_evRequestsQueue, m_hQueueThread;
	HANDLE m_pollingConnection, m_hPollingThread;
	ULONG  hAuthProcess;
	ULONG  hMessageProcess;
	mir_cs contact_search_lock;
	mir_cs requests_queue_lock;
	mir_cs set_status_lock;

	// instances
	static LIST<CSteamProto> InstanceList;
	static int CompareProtos(const CSteamProto *p1, const CSteamProto *p2);

	// requests
	RequestQueue *requestQueue;

	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, SteamResponseCallback response);
	void PushRequest(HttpRequest *request, SteamResponseWithArgCallback response, void *arg, HttpFinallyCallback last = NULL);

	void SendRequest(HttpRequest *request);
	void SendRequest(HttpRequest *request, SteamResponseCallback response);
	void SendRequest(HttpRequest *request, SteamResponseWithArgCallback response, void *arg, HttpFinallyCallback last = NULL);

	static void MirFreeArg(void *arg) { mir_free(arg); }

	// pooling thread
	void ParsePollData(JSONNode *data);
	void __cdecl PollingThread(void*);

	// account
	bool IsOnline();
	bool IsMe(const char *steamId);
	
	bool Relogin();

	void OnGotRsaKey(const HttpResponse *response);
	
	void OnAuthorization(const HttpResponse *response);
	void OnAuthorizationError(const JSONNode &node);
	void OnAuthorizationSuccess(const JSONNode &node);
	void OnGotSession(const HttpResponse *response);

	void OnLoggedOn(const HttpResponse *response);

	void HandleTokenExpired();

	// contacts
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void UpdateContact(MCONTACT hContact, JSONNode *data);
	void ProcessContact(std::map<std::string, JSONNode*>::iterator *it, MCONTACT hContact);
	
	void ContactIsRemoved(MCONTACT hContact);
	void ContactIsFriend(MCONTACT hContact);
	void ContactIsIgnored(MCONTACT hContact);

	MCONTACT FindContact(const char *steamId);
	MCONTACT AddContact(const char *steamId, bool isTemporary = false);

	void OnGotFriendList(const HttpResponse *response);
	void OnGotBlockList(const HttpResponse *response);
	void OnGotUserSummaries(const HttpResponse *response);
	void OnGotAvatar(const HttpResponse *response, void *arg);

	void OnFriendAdded(const HttpResponse *response, void *arg);
	void OnFriendBlocked(const HttpResponse *response, void *arg);
	void OnFriendRemoved(const HttpResponse *response, void *arg);

	void OnAuthRequested(const HttpResponse *response, void *arg);

	void OnPendingApproved(const HttpResponse *response, void *arg);
	void OnPendingIgnoreded(const HttpResponse *response, void *arg);

	void OnSearchByIdEnded(const HttpResponse *response, void *arg);

	void OnSearchByNameStarted(const HttpResponse *response, void *arg);
	void OnSearchByNameFinished(const HttpResponse *response, void *arg);

	// messages
	int OnSendMessage(MCONTACT hContact, const char* message);
	void OnMessageSent(const HttpResponse *response, void *arg);

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
	TCHAR* GetAvatarFilePath(MCONTACT hContact);
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
	int OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnIdleChanged(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	// utils
	static WORD SteamToMirandaStatus(int state);
	static int MirandaToSteamState(int status);

	static int RsaEncrypt(const char *pszModulus, const char *data, BYTE *encrypted, DWORD &encryptedSize);

	static void CSteamProto::ShowNotification(const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	static void CSteamProto::ShowNotification(const TCHAR *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

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
};

int OnReloadIcons(WPARAM wParam, LPARAM lParam);
void SetContactExtraIcon(MCONTACT hContact, int status);

#endif //_STEAM_PROTO_H_