#ifndef _TOX_PROTO_H_
#define _TOX_PROTO_H_

struct CToxProto : public PROTO<CToxProto>
{
	friend class CToxEnterPasswordDlg;
	friend class CToxCreatePasswordDlg;
	friend class CToxChangePasswordDlg;
	friend class CToxOptionsMain;
	friend class CToxOptionsNodeList;

public:
	//////////////////////////////////////////////////////////////////////////////////////
	// Ctors

	CToxProto(const char *protoName, const wchar_t *userName);
	~CToxProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthRecv(MCONTACT hContact, PROTORECVEVENT *) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *tszPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t *tszReason) override;
	int      FileResume(HANDLE hTransfer, int action, const wchar_t *tszFilename) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;

	HWND     SearchAdvanced(HWND owner) override;
	HWND     CreateExtendedSearchUI(HWND owner) override;

	int      SendMsg(MCONTACT hContact, int flags, const char *msg) override;

	HANDLE   SendFile(MCONTACT hContact, const wchar_t *, wchar_t **ppszFiles) override;

	int      SetStatus(int iNewStatus) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int iStatus, const wchar_t *msg) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnBuildProtoMenu(void) override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnErase() override;
	void     OnModulesLoaded() override;

	// icons
	static void InitIcons();

	// utils
	static void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	static INT_PTR ParseToxUri(WPARAM, LPARAM lParam);

private:
	Tox *m_tox;
	mir_cs m_profileLock;
	ptrW m_accountName;
	ptrW m_defaultGroup;
	
	CTransferList transfers;
	ULONG hMessageProcess;

	int m_retriesCount;
	HANDLE m_hTimerQueue;
	HANDLE m_hPollingTimer;
	HANDLE m_hCheckingTimer;

	static HANDLE hProfileFolderPath;

	// tox profile
	wchar_t* GetToxProfilePath();
	static wchar_t* GetToxProfilePath(const wchar_t *accountName);

	bool LoadToxProfile(Tox_Options *options);
	void SaveToxProfile(Tox *tox);

	INT_PTR __cdecl OnCopyToxID(WPARAM, LPARAM);
	INT_PTR __cdecl OnCreatePassword(WPARAM, LPARAM);
	INT_PTR __cdecl OnChangePassword(WPARAM, LPARAM);
	INT_PTR __cdecl OnRemovePassword(WPARAM, LPARAM);

	// tox core
	Tox_Options* GetToxOptions();
	void InitToxCore(Tox *tox);
	void UninitToxCore(Tox *tox);

	static void OnToxLog(Tox *tox, TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func, const char *message, void *user_data);

	// tox bootstrap
	void BootstrapUdpNode(Tox *tox, const char *address, int port, const char *pubKey);
	void BootstrapTcpRelay(Tox *tox, const char *address, int port, const char *pubKey);

	void BootstrapNodesFromDb(Tox *tox, bool isIPv6);
	void BootstrapNodesFromJson(Tox *tox, bool isIPv6);
	void BootstrapNodes(Tox *tox);

	void UpdateNodes();
	JSONNode ParseNodes();

	// tox connection
	bool IsOnline();

	void TryConnect();
	void CheckConnection();

	static void __stdcall OnToxCheck(void*, uint8_t);
	static void __stdcall OnToxPoll(void*, uint8_t);

	// accounts
	int __cdecl OnAccountRenamed(WPARAM, LPARAM);

	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);

	// netlib
	void InitNetlib();

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int __cdecl OnPrebuildContactMenu(WPARAM hContact, LPARAM);

	HGENMENU StatusMenuItems[SMI_MAX];
	int __cdecl UpdateStatusMenu(WPARAM, LPARAM);

	//services
	INT_PTR __cdecl SetMyNickname(WPARAM wParam, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// userinfo
	static INT_PTR CALLBACK UserInfoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int __cdecl OnUserInfoInit(WPARAM wParam, LPARAM lParam);

	// contacts
	uint16_t GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, uint16_t status);

	MCONTACT GetContact(const Tox *tox, const int friendNumber);
	MCONTACT GetContact(const char *pubKey);

	ToxHexAddress GetContactPublicKey(const Tox *tox, const int friendNumber);

	MCONTACT AddContact(const char *address, const wchar_t *nick = nullptr, const wchar_t *dnsId = nullptr, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	uint32_t GetToxFriendNumber(MCONTACT hContact);

	void LoadFriendList(Tox *tox);

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);

	static void OnFriendRequest(Tox *tox, const uint8_t *pubKey, const uint8_t *message, size_t length, void *arg);
	static void OnFriendNameChange(Tox *tox, uint32_t friendNumber, const uint8_t *name, size_t length, void *arg);
	static void OnStatusMessageChanged(Tox *tox, uint32_t friendNumber, const uint8_t *message, size_t length, void *arg);
	static void OnUserStatusChanged(Tox *tox, uint32_t friendNumber, TOX_USER_STATUS status, void *arg);
	static void OnConnectionStatusChanged(Tox *tox, uint32_t friendNumber, TOX_CONNECTION status, void *arg);

	// contacts search
	void __cdecl SearchByNameAsync(void *arg);

	static INT_PTR CALLBACK SearchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND __cdecl OnSearchAdvanced(HWND owner);
	HWND __cdecl OnCreateExtendedSearchUI(HWND owner);

	// messages
	std::map<uint64_t, UINT> messages;

	void InitCustomDbEvents();

	static INT_PTR EventGetIcon(WPARAM wParam, LPARAM lParam);

	void __cdecl SendMessageAsync(void *arg);
	int OnSendMessage(MCONTACT hContact, const char *message);

	static void OnFriendMessage(Tox *tox, uint32_t friendNumber, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *arg);
	static void OnReadReceipt(Tox *tox, uint32_t friendNumber, uint32_t messageNumber, void *arg);

	void __cdecl GetStatusMessageAsync(void* arg);

	int OnUserIsTyping(MCONTACT hContact, int type);
	static void OnTypingChanged(Tox *tox, uint32_t friendNumber, bool isTyping, void *arg);

	int __cdecl OnPreCreateMessage(WPARAM wParam, LPARAM lParam);

	// transfer
	HANDLE OnFileAllow(Tox *tox, MCONTACT hContact, HANDLE hTransfer, const wchar_t *tszPath);
	int OnFileResume(Tox *tox, HANDLE hTransfer, int action, const wchar_t *szFilename);
	HANDLE OnSendFile(Tox *tox, MCONTACT hContact, const wchar_t*, wchar_t **ppszFiles);

	int CancelTransfer(MCONTACT hContact, HANDLE hTransfer);

	static void OnFriendFile(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint32_t kind, uint64_t fileSize, const uint8_t *fileName, size_t filenameLength, void *arg);
	static void OnDataReceiving(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, const uint8_t *data, size_t length, void *arg);
	static void OnFileRequest(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, TOX_FILE_CONTROL control, void *arg);

	static void OnFileSendData(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, size_t length, void *arg);

	void OnTransferCompleted(Tox *tox, FileTransferParam *transfer);

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

	void OnGotFriendAvatarInfo(Tox *tox, AvatarTransferParam *transfer);
	void OnGotFriendAvatarData(AvatarTransferParam *transfer);

	// utils
	static int MapStatus(int status);
	static TOX_USER_STATUS MirandaToToxStatus(int status);
	static int ToxToMirandaStatus(TOX_USER_STATUS userstatus);

	static wchar_t* ToxErrorToString(TOX_ERR_NEW error);
	static wchar_t* ToxErrorToString(TOX_ERR_FRIEND_SEND_MESSAGE error);

	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	static bool IsFileExists(const wchar_t* path);

	MEVENT AddEventToDb(MCONTACT hContact, uint16_t type, uint32_t timestamp, uint32_t flags, uint8_t *pBlob, size_t cbBlob);
};

struct CMPlugin : public ACCPROTOPLUGIN<CToxProto>
{
	CMPlugin();

	int Load() override;
};

#endif //_TOX_PROTO_H_