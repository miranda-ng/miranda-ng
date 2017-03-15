#ifndef _SLACK_PROTO_H_
#define _SLACK_PROTO_H_

struct CSlackProto : public PROTO<CSlackProto>
{
	friend class CSlackOptionsMain;
	friend class CSlackOAuth;

	typedef void(CSlackProto::*HttpCallback)(HttpResponse&, void*);
	typedef void(CSlackProto::*JsonCallback)(JSONNode&, void*);
	struct RequestQueueItem
	{
		HttpRequest *request;
		HttpCallback httpCallback;
		JsonCallback jsonCallback;
		void *param;
	};

	struct SendMessageParam
	{
		MCONTACT hContact;
		UINT hMessage;
		char *message;
	};

public:
	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CSlackProto(const char *protoName, const wchar_t *userName);
	~CSlackProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const wchar_t* szMessage);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// accounts
	static CSlackProto*       InitAccount(const char *protoName, const TCHAR *userName);
	static int                UninitAccount(CSlackProto *proto);

	// icons
	static void InitIcons();

	// menus
	static void InitMenus();

	static int OnModulesLoaded(WPARAM, LPARAM);

private:
	HANDLE hNetlib;
	bool isTerminated, isConnected;
	mir_cs requestQueueLock;
	HANDLE hRequestsQueueEvent;
	HANDLE hRequestQueueThread;
	LIST<RequestQueueItem> requestQueue;
	ULONG hMessageProcess;

	// requests
	void SendRequest(HttpRequest *request);
	void SendRequest(HttpRequest *request, HttpCallback callback, void *param = NULL);
	void SendRequest(HttpRequest *request, JsonCallback callback, void *param = NULL);
	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, HttpCallback callback, void *param = NULL);
	void PushRequest(HttpRequest *request, JsonCallback callback, void *param = NULL);
	void __cdecl RequestQueueThread(void*);
	
	// network
	bool IsOnline();

	// accounts
	static LIST<CSlackProto> Accounts;
	static int CompareAccounts(const CSlackProto *p1, const CSlackProto *p2);

	static CSlackProto* GetContactAccount(MCONTACT hContact);

	int __cdecl OnAccountLoaded(WPARAM, LPARAM);
	int __cdecl OnAccountDeleted(WPARAM, LPARAM);

	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);

	// netlib
	void InitNetlib();
	void UninitNetlib();

	// login
	void Login();
	void LogOut();
	void OnAuthorize(JSONNode &root, void*);

	// icons
	static IconItemT Icons[];
	static HANDLE GetIconHandle(int iconId);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// contacts
	WORD GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	MCONTACT GetContact(const char *userId);
	MCONTACT AddContact(const char *userId, const char *nick, bool isTemporary = false);

	void OnGotUserProfile(JSONNode &root);
	void OnGotUserProfile(MCONTACT hContact, JSONNode &root);
	void OnGotUserList(JSONNode &root, void*);

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);

	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	// messages
	int OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre);

	void OnMessageSent(JSONNode &root, void *arg);
	void OnImChannelOppened(JSONNode &root, void *arg);
	void __cdecl SendMessageThread(void*);
	void __cdecl SendMessageAckThread(void*);

	int OnUserIsTyping(MCONTACT hContact, int type);

	int __cdecl OnPreCreateMessage(WPARAM wParam, LPARAM lParam);

	// utils
	static int SlackToMirandaStatus(const char *presence);

	static void ShowNotification(const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const TCHAR *caption, const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);

	MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, PBYTE pBlob, DWORD cbBlob);

	template<INT_PTR(__cdecl CSlackProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CSlackProto *proto = CSlackProto::GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

#endif //_SLACK_PROTO_H_
