#ifndef _TOX_PROTO_H_
#define _TOX_PROTO_H_

struct CToxProto : public PROTO<CToxProto>
{
public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CToxProto(const char *protoName, const wchar_t *userName);
	~CToxProto();

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
	static CToxProto* InitAccount(const char *protoName, const TCHAR *userName);
	static int        UninitAccount(CToxProto *proto);

	// icons
	static void InitIcons();
	static void UninitIcons();

	// menus
	static void InitMenus();
	static void UninitMenus();

private:
	Tox *tox;
	char *password;
	mir_cs toxLock;
	TCHAR *accountName;
	HANDLE hNetlib, hPollingThread;
	bool isTerminated, isConnected;
	CTransferList transfers;

	// tox profile
	std::tstring GetToxProfilePath();
	static std::tstring CToxProto::GetToxProfilePath(const TCHAR *accountName);

	bool LoadToxProfile();
	void SaveToxProfile();

	INT_PTR __cdecl OnCopyToxID(WPARAM, LPARAM);

	static INT_PTR CALLBACK ToxProfileImportProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ToxProfilePasswordProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// tox core
	bool IsToxCoreInited();

	bool InitToxCore();
	void UninitToxCore();

	// tox network
	bool IsOnline();

	int BootstrapNodesFromDb(bool isIPv6);
	int BootstrapNodesFromIni(bool isIPv6);
	void BootstrapNodes();
	void TryConnect();
	void CheckConnection(int &retriesCount);
	void DoTox();

	void __cdecl PollingThread(void*);

	// accounts
	static LIST<CToxProto> Accounts;
	static int CompareAccounts(const CToxProto *p1, const CToxProto *p2);

	static CToxProto* GetContactAccount(MCONTACT hContact);

	int __cdecl OnAccountLoaded(WPARAM, LPARAM);
	int __cdecl OnAccountRenamed(WPARAM, LPARAM);

	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);

	// netlib
	void InitNetlib();
	void UninitNetlib();

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
	static INT_PTR CALLBACK NodesOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// events
	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	// userinfo
	static INT_PTR CALLBACK UserInfoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int __cdecl OnUserInfoInit(WPARAM wParam, LPARAM lParam);

	// contacts
	WORD GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	MCONTACT GetContact(const int friendNumber);
	MCONTACT GetContact(const char *pubKey);

	MCONTACT AddContact(const char *address, const std::tstring &dnsId, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	int32_t GetToxFriendNumber(MCONTACT hContact);

	void __cdecl LoadFriendList(void*);

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);

	static void OnFriendRequest(Tox *tox, const uint8_t *pubKey, const uint8_t *message, const uint16_t messageSize, void *arg);
	static void OnFriendNameChange(Tox *tox, const int friendNumber, const uint8_t *name, const uint16_t nameSize, void *arg);
	static void OnStatusMessageChanged(Tox *tox, const int friendNumber, const uint8_t* message, const uint16_t messageSize, void *arg);
	static void OnUserStatusChanged(Tox *tox, int32_t friendNumber, uint8_t usertatus, void *arg);
	static void OnConnectionStatusChanged(Tox *tox, const int friendNumber, const uint8_t status, void *arg);

	// contacts search
	void __cdecl SearchByNameAsync(void* arg);
	void __cdecl SearchFailedAsync(void* arg);

	static INT_PTR CALLBACK SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND __cdecl OnSearchAdvanced(HWND owner);
	HWND __cdecl OnCreateExtendedSearchUI(HWND owner);

	// chat rooms
	//MCONTACT GetChatRoom(const char *pubKey);
	MCONTACT GetChatRoom(int groupNumber);

	//MCONTACT GetChatRoom(const char *pubKey);
	MCONTACT AddChatRoom(int groupNumber);

	void __cdecl LoadChatRoomList(void*);

	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);

	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnCreateChatRoom(WPARAM, LPARAM);

	void InitGroupChatModule();
	void CloseAllChatChatSessions();

	static void OnGroupChatInvite(Tox *tox, int32_t friendNumber, uint8_t type, const uint8_t *data, const uint16_t length, void *arg);

	void ChatValidateContact(HWND hwndList, const std::vector<MCONTACT> &contacts, MCONTACT hContact = NULL);
	void ChatPrepare(HWND hwndList, const std::vector<MCONTACT> &contacts, MCONTACT hContact = NULL);
	static std::vector<MCONTACT> GetInvitedContacts(HWND hwndList, MCONTACT hContact = NULL);
	static INT_PTR CALLBACK ChatRoomInviteProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// messages
	int OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre);
	int OnSendMessage(MCONTACT hContact, int flags, const char *message);

	static void OnFriendMessage(Tox *tox, const int friendNumber, const uint8_t *message, const uint16_t messageSize, void *arg);
	static void OnFriendAction(Tox *tox, const int friendNumber, const uint8_t *action, const uint16_t actionSize, void *arg);
	static void OnTypingChanged(Tox *tox, const int friendNumber, uint8_t isTyping, void *arg);
	static void OnReadReceipt(Tox *tox, int32_t friendNumber, uint32_t receipt, void *arg);

	int __cdecl OnPreCreateMessage(WPARAM wParam, LPARAM lParam);

	// transfer
	void __cdecl SendFileAsync(void* arg);

	//static void OnFileControlCallback(Tox *tox, int32_t number, uint8_t hFile, uint64_t fileSize, uint8_t *name, uint16_t nameSize, void *arg);
	static void OnFileRequest(Tox *tox, int32_t friendNumber, uint8_t receive_send, uint8_t fileNumber, uint8_t type, const uint8_t *data, uint16_t length, void *arg);
	static void OnFriendFile(Tox *tox, int32_t friendNumber, uint8_t fileNumber, uint64_t fileSize, const uint8_t *fileName, uint16_t length, void *arg);
	static void OnFileData(Tox *tox, int32_t friendNumber, uint8_t fileNumber, const uint8_t *data, uint16_t length, void *arg);

	// avatars
	std::tstring GetAvatarFilePath(MCONTACT hContact = NULL);
	void SetToxAvatar(std::tstring path, bool checkHash = false);

	INT_PTR __cdecl GetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM lParam);
	INT_PTR __cdecl GetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SetMyAvatar(WPARAM wParam, LPARAM lParam);

	INT_PTR __cdecl CToxProto::SetMyNickname(WPARAM wParam, LPARAM lParam);

	static void OnGotFriendAvatarInfo(Tox *tox, int32_t number, uint8_t format, uint8_t *hash, void *arg);
	static void OnGotFriendAvatarData(Tox *tox, int32_t number, uint8_t format, uint8_t *hash, uint8_t *data, uint32_t length, void *arg);

	// utils
	TOX_USERSTATUS MirandaToToxStatus(int status);
	int ToxToMirandaStatus(TOX_USERSTATUS userstatus);

	static void ShowNotification(const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const TCHAR *caption, const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);

	static bool IsFileExists(std::tstring path);

	template<INT_PTR(__cdecl CToxProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CToxProto *proto = CToxProto::GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

#endif //_TOX_PROTO_H_