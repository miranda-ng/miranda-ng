#ifndef _TOX_PROTO_H_
#define _TOX_PROTO_H_

struct CToxProto : public PROTO<CToxProto>
{
	friend CToxPasswordEditor;
	friend CToxOptionsMain;
	friend CToxOptionsNodeList;
	friend CToxAudioCall;

public:
	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CToxProto(const char *protoName, const wchar_t *userName);
	~CToxProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	int      __cdecl Authorize(MEVENT hDbEvent);
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszPath);
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** tszFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR*, PROTOCHAR **ppszFiles);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
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

	// events
	static void InitCustomDbEvents();

	static int OnModulesLoaded(WPARAM, LPARAM);

private:
	Tox *tox;
	ToxAv *toxAv;
	char *password;
	mir_cs toxLock;
	TCHAR *accountName;
	HANDLE hNetlib, hPollingThread;
	bool isTerminated, isConnected;
	CTransferList transfers;

	static HANDLE hProfileFolderPath;

	// tox profile
	std::tstring GetToxProfilePath();
	static std::tstring CToxProto::GetToxProfilePath(const TCHAR *accountName);

	bool LoadToxProfile(Tox_Options *options);
	void SaveToxProfile();

	INT_PTR __cdecl OnCopyToxID(WPARAM, LPARAM);

	// tox core
	bool InitToxCore();
	void UninitToxCore();

	// tox network
	bool IsOnline();

	void BootstrapNode(const char *address, int port, const uint8_t *pubKey);
	void BootstrapNodesFromDb(bool isIPv6);
	void BootstrapNodesFromIni(bool isIPv6);
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

	//services
	INT_PTR __cdecl CToxProto::SetMyNickname(WPARAM wParam, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// events
	static int __cdecl OnDbEventAdded(WPARAM hContact, LPARAM hEvent);

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

	uint32_t GetToxFriendNumber(MCONTACT hContact);

	void __cdecl LoadFriendList(void*);

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);

	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	static void OnFriendRequest(Tox *tox, const uint8_t *pubKey, const uint8_t *message, size_t length, void *arg);
	static void OnFriendNameChange(Tox *tox, uint32_t friendNumber, const uint8_t *name, size_t length, void *arg);
	static void OnStatusMessageChanged(Tox *tox, uint32_t friendNumber, const uint8_t *message, size_t length, void *arg);
	static void OnUserStatusChanged(Tox *tox, uint32_t friendNumber, TOX_USER_STATUS status, void *arg);
	static void OnConnectionStatusChanged(Tox *tox, uint32_t friendNumber, TOX_CONNECTION status, void *arg);

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

	static void OnFriendMessage(Tox *tox, uint32_t friendNumber, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *arg);
	static void OnReadReceipt(Tox *tox, uint32_t friendNumber, uint32_t messageId, void *arg);

	int OnUserIsTyping(MCONTACT hContact, int type);
	static void OnTypingChanged(Tox *tox, uint32_t friendNumber, bool isTyping, void *arg);

	int __cdecl OnPreCreateMessage(WPARAM wParam, LPARAM lParam);

	// transfer
	HANDLE OnFileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR *tszPath);
	int OnFileResume(HANDLE hTransfer, int *action, const PROTOCHAR **szFilename);
	int OnFileCancel(MCONTACT hContact, HANDLE hTransfer);
	HANDLE OnSendFile(MCONTACT hContact, const PROTOCHAR*, PROTOCHAR **ppszFiles);

	static void OnFileRequest(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, TOX_FILE_CONTROL control, void *arg);
	static void OnFriendFile(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint32_t kind, uint64_t fileSize, const uint8_t *fileName, size_t filenameLength, void *arg);
	static void OnFileReceiveData(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, const uint8_t *data, size_t length, void *arg);

	static void OnFileSendData(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, size_t length, void *arg);

	// avatars
	std::tstring GetAvatarFilePath(MCONTACT hContact = NULL);
	void SetToxAvatar(std::tstring path, bool checkHash = false);

	INT_PTR __cdecl GetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM lParam);
	INT_PTR __cdecl GetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SetMyAvatar(WPARAM wParam, LPARAM lParam);

	void OnGotFriendAvatarInfo(FileTransferParam *transfer, const uint8_t *hash);

	// multimedia
	HANDLE hAudioDialogs;
	std::map<MCONTACT, int> calls;
	
	static void OnFriendAudio(void *agent, int32_t callId, const int16_t *PCM, uint16_t size, void *arg);
	INT_PTR __cdecl OnRecvAudioCall(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnAudioRing(WPARAM hContact, LPARAM lParam);

	static void OnAvInvite(void*, int32_t callId, void *arg);
	static void OnAvRinging(void*, int32_t callId, void *arg);
	static void OnAvStart(void*, int32_t callId, void *arg);
	static void OnAvEnd(void*, int32_t callId, void *arg);
	static void OnAvReject(void*, int32_t callId, void *arg);
	static void OnAvCancel(void*, int32_t callId, void *arg);
	static void OnAvCsChange(void*, int32_t callId, void *arg);
	static void OnAvRequestTimeout(void*, int32_t callId, void *arg);
	static void OnAvPeerTimeout(void*, int32_t callId, void *arg);

	// utils
	TOX_USER_STATUS MirandaToToxStatus(int status);
	int ToxToMirandaStatus(TOX_USER_STATUS userstatus);

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