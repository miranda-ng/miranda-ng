#ifndef _TOX_PROTO_H_
#define _TOX_PROTO_H_

struct CToxProto : public PROTO<CToxProto>
{
	friend CToxEnterPasswordDlg;
	friend CToxCreatePasswordDlg;
	friend CToxChangePasswordDlg;
	friend CToxOptionsMain;
	friend CToxOptionsNodeList;

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
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const wchar_t* szMessage);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* tszPath);
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* tszReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const wchar_t** tszFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const wchar_t*, wchar_t **ppszFiles);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const wchar_t* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// accounts
	static CToxProto* InitAccount(const char *protoName, const wchar_t *userName);
	static int        UninitAccount(CToxProto *proto);

	// icons
	static void InitIcons();

	// menus
	static void InitContactMenu();

	// utils
	static void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	static INT_PTR ParseToxUri(WPARAM, LPARAM lParam);

private:
	CToxThread *m_toxThread;
	mir_cs m_profileLock;
	ptrW m_accountName;
	ptrW m_defaultGroup;
	
	CTransferList transfers;
	ULONG hMessageProcess;

	bool isTerminated;
	HANDLE hConnectingThread;
	HANDLE hCheckingThread;
	HANDLE hPollingThread;
	HANDLE hTerminateEvent;

	static HANDLE hProfileFolderPath;

	// tox profile
	wchar_t* GetToxProfilePath();
	static wchar_t* GetToxProfilePath(const wchar_t *accountName);

	bool LoadToxProfile(Tox_Options *options);
	void SaveToxProfile(Tox *tox);
	int OnDeleteToxProfile();

	INT_PTR __cdecl OnCopyToxID(WPARAM, LPARAM);
	INT_PTR __cdecl OnCreatePassword(WPARAM, LPARAM);
	INT_PTR __cdecl OnChangePassword(WPARAM, LPARAM);
	INT_PTR __cdecl OnRemovePassword(WPARAM, LPARAM);

	// tox core
	Tox_Options* GetToxOptions();
	void InitToxCore(Tox *tox);
	void UninitToxCore(Tox *tox);

	// tox bootstrap
	void BootstrapUdpNode(Tox *tox, const char *address, int port, const char *pubKey);
	void BootstrapTcpRelay(Tox *tox, const char *address, int port, const char *pubKey);

	void BootstrapNodesFromDb(Tox *tox, bool isIPv6);
	void BootstrapNodesFromJson(Tox *tox, bool isIPv6);
	void BootstrapNodes(Tox *tox);

	void UpdateNodes();

	// tox connection
	bool IsOnline();

	void TryConnect(Tox *tox);
	void CheckConnection(Tox *tox, int &retriesCount);

	void __cdecl CheckingThread(void*);
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
	static IconItemT Icons[];
	static HANDLE GetIconHandle(int iconId);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	HGENMENU StatusMenuItems[SMI_MAX];
	int OnInitStatusMenu();
	int __cdecl UpdateStatusMenu(WPARAM, LPARAM);

	//services
	INT_PTR __cdecl SetMyNickname(WPARAM wParam, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// userinfo
	static INT_PTR CALLBACK UserInfoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int __cdecl OnUserInfoInit(WPARAM wParam, LPARAM lParam);

	// contacts
	WORD GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, WORD status);

	MCONTACT GetContact(const Tox *tox, const int friendNumber);
	MCONTACT GetContact(const char *pubKey);

	ToxHexAddress GetContactPublicKey(const Tox *tox, const int friendNumber);

	MCONTACT AddContact(const char *address, const char *nick = NULL, const char *dnsId = NULL, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	uint32_t GetToxFriendNumber(MCONTACT hContact);

	void LoadFriendList(Tox *tox);

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);

	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	static void OnFriendRequest(Tox *tox, const uint8_t *pubKey, const uint8_t *message, size_t length, void *arg);
	static void OnFriendNameChange(Tox *tox, uint32_t friendNumber, const uint8_t *name, size_t length, void *arg);
	static void OnStatusMessageChanged(Tox *tox, uint32_t friendNumber, const uint8_t *message, size_t length, void *arg);
	static void OnUserStatusChanged(Tox *tox, uint32_t friendNumber, TOX_USER_STATUS status, void *arg);
	static void OnConnectionStatusChanged(Tox *tox, uint32_t friendNumber, TOX_CONNECTION status, void *arg);

	// contacts search
	void __cdecl SearchByNameAsync(void *arg);
	void __cdecl SearchFailedAsync(void *arg);

	static INT_PTR CALLBACK SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND __cdecl OnSearchAdvanced(HWND owner);
	HWND __cdecl OnCreateExtendedSearchUI(HWND owner);

	// messages
	std::map<uint64_t, UINT> messages;

	void InitCustomDbEvents();

	void __cdecl SendMessageAsync(void *arg);
	int OnSendMessage(MCONTACT hContact, const char *message);

	static void OnFriendMessage(Tox *tox, uint32_t friendNumber, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *arg);
	static void OnReadReceipt(Tox *tox, uint32_t friendNumber, uint32_t messageNumber, void *arg);

	void __cdecl GetStatusMessageAsync(void* arg);

	int OnUserIsTyping(MCONTACT hContact, int type);
	static void OnTypingChanged(Tox *tox, uint32_t friendNumber, bool isTyping, void *arg);

	int __cdecl OnPreCreateMessage(WPARAM wParam, LPARAM lParam);

	// transfer
	HANDLE OnFileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *tszPath);
	int OnFileResume(HANDLE hTransfer, int *action, const wchar_t **szFilename);
	HANDLE OnSendFile(MCONTACT hContact, const wchar_t*, wchar_t **ppszFiles);

	int CancelTransfer(MCONTACT hContact, HANDLE hTransfer);

	static void OnFileRequest(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, TOX_FILE_CONTROL control, void *arg);
	static void OnFriendFile(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint32_t kind, uint64_t fileSize, const uint8_t *fileName, size_t filenameLength, void *arg);
	static void OnDataReceiving(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, const uint8_t *data, size_t length, void *arg);

	static void OnFileSendData(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, size_t length, void *arg);

	void OnTransferCompleted(FileTransferParam *transfer);

	void PauseOutgoingTransfers(uint32_t friendNumber);
	void ResumeIncomingTransfers(uint32_t friendNumber);
	void CancelAllTransfers(Tox *tox);

	// avatars
	wchar_t* GetAvatarFilePath(MCONTACT hContact = NULL);
	void SetToxAvatar(const wchar_t* path);

	INT_PTR __cdecl GetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM lParam);
	INT_PTR __cdecl GetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SetMyAvatar(WPARAM wParam, LPARAM lParam);

	void OnGotFriendAvatarInfo(AvatarTransferParam *transfer);
	void OnGotFriendAvatarData(AvatarTransferParam *transfer);

	// utils
	static int MapStatus(int status);
	static TOX_USER_STATUS MirandaToToxStatus(int status);
	static int ToxToMirandaStatus(TOX_USER_STATUS userstatus);

	static wchar_t* ToxErrorToString(TOX_ERR_NEW error);
	static wchar_t* ToxErrorToString(TOX_ERR_FRIEND_SEND_MESSAGE error);

	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	static bool IsFileExists(const wchar_t* path);

	MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, PBYTE pBlob, size_t cbBlob);

	template<INT_PTR(__cdecl CToxProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CToxProto *proto = CToxProto::GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

#endif //_TOX_PROTO_H_