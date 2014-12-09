#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#define STEAM_SEARCH_BYID 1001
#define STEAM_SEARCH_BYNAME 1002

struct GuardParam
{
	char code[10];
	char domain[32];
};

struct CaptchaParam
{
	BYTE *data;
	size_t size;
	char text[10];
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

typedef void (CSteamProto::*RESPONSE)(const NETLIBHTTPREQUEST *response, void *arg);

struct QueueItem
{
	SteamWebApi::HttpRequest *request;
	void *arg;
	RESPONSE responseCallback;
	RESPONSE responseFailedCallback;

	QueueItem(SteamWebApi::HttpRequest *request) : 
		request(request), arg(NULL), responseCallback(NULL), responseFailedCallback(NULL) { }
	
	QueueItem(SteamWebApi::HttpRequest *request, RESPONSE response) : 
		request(request), arg(NULL), responseCallback(response), responseFailedCallback(NULL) { }
	
	QueueItem(SteamWebApi::HttpRequest *request, RESPONSE response, RESPONSE responseFailedCallback) : 
		request(request), arg(NULL), responseCallback(response), responseFailedCallback(responseFailedCallback) { }

	~QueueItem() { delete request; responseCallback = NULL; }
};

class CSteamProto : public PROTO<CSteamProto>
{
public:
	// PROTO_INTERFACE
	CSteamProto(const char *protoName, const wchar_t *userName);
	~CSteamProto();

	// PROTO_INTERFACE
	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT *psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int       __cdecl Authorize(HANDLE hDbEvent);
	virtual	int       __cdecl AuthDeny(HANDLE hDbEvent, const TCHAR *szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT *);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR * szMessage);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int *action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR *id);
	virtual	HANDLE    __cdecl SearchByEmail(const TCHAR *email);
	virtual	HANDLE    __cdecl SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR* szDescription, TCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const TCHAR* msg);

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
	void PushRequest(SteamWebApi::HttpRequest *request, RESPONSE response, void *arg);

	void ExecuteRequest(QueueItem *requestItem);

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

	MCONTACT GetContactFromAuthEvent(HANDLE hEvent);

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
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);
	static int __cdecl OnOptionsInit(void *obj, WPARAM wParam, LPARAM lParam);

	// utils
	static WORD SteamToMirandaStatus(int state);
	static int MirandaToSteamState(int status);

	static int RsaEncrypt(const char *pszModulus, const char *data, BYTE *encrypted, DWORD &encryptedSize);

	HANDLE AddDBEvent(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob);

	static void CSteamProto::ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	static void CSteamProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	// dialog procs
	static INT_PTR CALLBACK GuardProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK CaptchaProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK MainOptionsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK BlockListOptionsSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK BlockListOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// helpers
	inline int IdleSeconds() { return m_idleTS ? time(0) - m_idleTS : 0; }
};

int OnReloadIcons(WPARAM wParam, LPARAM lParam);

#endif //_STEAM_PROTO_H_