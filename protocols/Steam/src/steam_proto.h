#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#define STEAM_SEARCH_BYID 1001
#define STEAM_SEARCH_BYNAME 1002
#define STEAM_TYPING_TIME 10

struct PasswordParam
{
	char password[513];
	char timestamp[16];
};

struct SendAuthParam
{
	MCONTACT hContact;
	HANDLE hAuth;
};

struct SendMessageParam
{
	MCONTACT hContact;
	HANDLE hMessage;
	const char *msg;
	int flags;
};

struct STEAM_SEARCH_RESULT
{
	PROTOSEARCHRESULT hdr;
	JSONNODE *data;
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

enum HTTP_STATUS
{
	HTTP_STATUS_NONE = 0,
	HTTP_STATUS_OK = 200,
	HTTP_STATUS_FOUND = 302,
	HTTP_STATUS_BAD_REQUEST = 400,
	HTTP_STATUS_UNAUTHORIZED = 401,
	HTTP_STATUS_FORBIDDEN = 403,
	HTTP_STATUS_NOT_FOUND = 404,
	HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
	HTTP_STATUS_TOO_MANY_REQUESTS = 429,
	HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
	HTTP_STATUS_INSUFICIENTE_STORAGE = 507
};

enum ARG_FREE_TYPE
{
	ARG_NO_FREE,
	ARG_MIR_FREE
};

typedef void (CSteamProto::*RESPONSE)(const NETLIBHTTPREQUEST *response, void *arg);

struct QueueItem
{
	SteamWebApi::HttpRequest *request;
	void *arg;
	ARG_FREE_TYPE arg_free_type;
	RESPONSE responseCallback;
	//RESPONSE responseFailedCallback;

	QueueItem(SteamWebApi::HttpRequest *request) : 
		request(request), arg(NULL), responseCallback(NULL)/*, responseFailedCallback(NULL)*/ { }
	
	QueueItem(SteamWebApi::HttpRequest *request, RESPONSE response) : 
		request(request), arg(NULL), responseCallback(response)/*, responseFailedCallback(NULL)*/ { }
	
	//QueueItem(SteamWebApi::HttpRequest *request, RESPONSE response, RESPONSE responseFailedCallback) : 
	//	request(request), arg(NULL), responseCallback(response), responseFailedCallback(responseFailedCallback) { }

	~QueueItem() { 
		// Free request
		delete request;

		// Free argument
		switch (arg_free_type)
		{
		case ARG_NO_FREE:
			break;
		case ARG_MIR_FREE:
			mir_free(arg);
		default:
			break;
		}

		responseCallback = NULL;
		//responseFailedCallback = NULL;
	}
};

class CSteamProto : public PROTO<CSteamProto>
{
	friend CSteamPasswordEditor;
	friend CSteamOptionsMain;

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

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

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
	bool isTerminated;
	time_t m_idleTS;
	HANDLE m_evRequestsQueue, m_hQueueThread;
	HANDLE m_pollingConnection, m_hPollingThread;
	ULONG  hAuthProcess;
	ULONG  hMessageProcess;
	CRITICAL_SECTION contact_search_lock;
	CRITICAL_SECTION requests_queue_lock;
	CRITICAL_SECTION set_status_lock;
	LIST<QueueItem> requestsQueue;

	// instances
	static LIST<CSteamProto> InstanceList;
	static int CompareProtos(const CSteamProto *p1, const CSteamProto *p2);

	// queue
	void InitQueue();
	void UninitQueue();
	
	void StartQueue();
	void StopQueue();

	void PushRequest(SteamWebApi::HttpRequest *request);
	void PushRequest(SteamWebApi::HttpRequest *request, RESPONSE response);
	void PushRequest(SteamWebApi::HttpRequest *request, RESPONSE response, void *arg, ARG_FREE_TYPE arg_free_type);

	void ExecuteRequest(QueueItem *requestItem);

	void __cdecl SendMsgThread(void*);
	void __cdecl QueueThread(void*);

	// pooling thread
	void ParsePollData(JSONNODE *data);
	void __cdecl PollingThread(void*);

	// account
	bool IsOnline();
	bool IsMe(const char *steamId);

	void OnGotRsaKey(const NETLIBHTTPREQUEST *response, void *arg);
	
	void OnAuthorization(const NETLIBHTTPREQUEST *response, void *arg);
	void OnGotSession(const NETLIBHTTPREQUEST *response, void *arg);

	void OnLoggedOn(const NETLIBHTTPREQUEST *response, void *arg);

	// contacts
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void UpdateContact(MCONTACT hContact, JSONNODE *data);
	void ProcessContact(std::map<std::string, JSONNODE*>::iterator *it, MCONTACT hContact);
	
	void ContactIsRemoved(MCONTACT hContact);
	void ContactIsFriend(MCONTACT hContact);
	void ContactIsIgnored(MCONTACT hContact);

	MCONTACT FindContact(const char *steamId);
	MCONTACT AddContact(const char *steamId, bool isTemporary = false);

	void OnGotFriendList(const NETLIBHTTPREQUEST *response, void *arg);
	void OnGotBlockList(const NETLIBHTTPREQUEST *response, void *arg);
	void OnGotUserSummaries(const NETLIBHTTPREQUEST *response, void *arg);
	void OnGotAvatar(const NETLIBHTTPREQUEST *response, void *arg);

	void OnFriendAdded(const NETLIBHTTPREQUEST *response, void *arg);
	void OnFriendBlocked(const NETLIBHTTPREQUEST *response, void *arg);
	void OnFriendRemoved(const NETLIBHTTPREQUEST *response, void *arg);

	void OnAuthRequested(const NETLIBHTTPREQUEST *response, void *arg);

	void OnPendingApproved(const NETLIBHTTPREQUEST *response, void *arg);
	void OnPendingIgnoreded(const NETLIBHTTPREQUEST *response, void *arg);

	void OnSearchByIdEnded(const NETLIBHTTPREQUEST *response, void *arg);

	void OnSearchByNameStarted(const NETLIBHTTPREQUEST *response, void *arg);
	void OnSearchByNameFinished(const NETLIBHTTPREQUEST *response, void *arg);

	// messages
	void OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg);

	// menus
	HGENMENU m_hMenuRoot;
	static HANDLE hChooserMenu;
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
	bool GetDbAvatarInfo(PROTO_AVATAR_INFORMATIONT &pai);
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

	MEVENT AddDBEvent(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob);

	static void CSteamProto::ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	static void CSteamProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

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