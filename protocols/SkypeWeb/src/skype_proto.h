#ifndef _SKYPE_PROTO_H_
#define _SKYPE_PROTO_H_

typedef void(CSkypeProto::*SkypeResponseCallback)(const NETLIBHTTPREQUEST *response);

struct CSkypeProto : public PROTO < CSkypeProto >
{
public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CSkypeProto(const char *protoName, const wchar_t *userName);
	~CSkypeProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);

	virtual	int      __cdecl Authorize(MEVENT hDbEvent);
	virtual	int      __cdecl AuthDeny(MEVENT hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszPath);
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** tszFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTOFILEEVENT*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR *szDescription, PROTOCHAR **ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// accounts
	static CSkypeProto* InitAccount(const char *protoName, const TCHAR *userName);
	static int          UninitAccount(CSkypeProto *proto);

	// icons
	static void InitIcons();
	static void UninitIcons();

	// menus
	static void InitMenus();
	static void UninitMenus();

	// languages
	static void InitLanguages();

	// events
	static int OnModulesLoaded(WPARAM, LPARAM);

private:
	char *password;
	RequestQueue *requestQueue;
	bool isTerminated;
	std::map<std::string, std::string> cookies;
	std::map<std::string, std::string> RegInfo;
	HANDLE m_pollingConnection, m_hPollingThread;
	static std::map<std::tstring, std::tstring> languages;

	static INT_PTR CALLBACK PasswordEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// accounts
	static LIST<CSkypeProto> Accounts;
	static int CompareAccounts(const CSkypeProto *p1, const CSkypeProto *p2);

	static CSkypeProto* GetContactAccount(MCONTACT hContact);

	int __cdecl OnAccountLoaded(WPARAM, LPARAM);

	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);

	// requests
	void PushRequest(HttpRequest *request, SkypeResponseCallback response = NULL);

	// icons
	static IconInfo Icons[];
	static HANDLE GetIconHandle(const char *name);
	static HANDLE GetSkinIconHandle(const char *name);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	int OnInitStatusMenu();

	// options
	static INT_PTR CALLBACK MainOptionsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// events
	void OnLoginFirst(const NETLIBHTTPREQUEST *response);
	void OnLoginSecond(const NETLIBHTTPREQUEST *response);
	void OnGetRegInfo(const NETLIBHTTPREQUEST *response);
	void OnSetStatus(const NETLIBHTTPREQUEST *response);
	void OnGetEndpoint(const NETLIBHTTPREQUEST *response);
	// profile
	void UpdateProfileFirstName(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileLastName(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileDisplayName(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileGender(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileBirthday(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileCountry(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileState(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileCity(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileLanguage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileHomepage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileAbout(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileEmails(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneMobile(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneHome(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneOffice(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileStatusMessage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileXStatusMessage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileAvatar(JSONNODE *root, MCONTACT hContact = NULL);

	void LoadProfile(const NETLIBHTTPREQUEST *response);

	// contacts
	WORD GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	MCONTACT GetContact(const char *skypename);

	MCONTACT AddContact(const char *skypename, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void LoadContactsAuth(const NETLIBHTTPREQUEST *response);
	void LoadContactsInfo(const NETLIBHTTPREQUEST *response);
	void LoadContactList(const NETLIBHTTPREQUEST *response);

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);

	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	// messages
	int OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre);
	int OnSendMessage(MCONTACT hContact, int flags, const char *message);
	//polling
	void __cdecl ParsePollData(JSONNODE *data);
	void __cdecl PollingThread(void*);
	void CSkypeProto::ProcessEndpointPresenceRes(JSONNODE *node);
	void CSkypeProto::ProcessUserPresenceRes(JSONNODE *node);
	void CSkypeProto::ProcessNewMessageRes(JSONNODE *node);
	// utils
	time_t __stdcall IsoToUnixTime(const TCHAR *stamp);
	char *GetStringChunk(const char *haystack, size_t len, const char *start, const char *end);
	bool IsMe(const char *skypeName);
	int SkypeToMirandaStatus(const char *status);
	char *MirandaToSkypeStatus(int status);
	static void ShowNotification(const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const TCHAR *caption, const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	void SetServerStatus(int iNewStatus);
	static bool IsFileExists(std::tstring path);
	char *ContactUrlToName(const char *url);
	char *SelfUrlToName(const char *url);
	char *GetServerFromUrl(const char *url);
	template<INT_PTR(__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CSkypeProto *proto = CSkypeProto::GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

#endif //_SKYPE_PROTO_H_