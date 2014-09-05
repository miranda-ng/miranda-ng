#pragma once

struct _tag_iconList
{
	wchar_t*	Description;
	char*		Name;
	int			IconId;
	HANDLE		Handle;
};

struct ReadMessageParam
{
	SEBinary& guid;
	CMessage::TYPE msgType;
};

struct ContactParam
{
	ContactRef contact;
	CSkypeProto *ppro;
	
	ContactParam(const ContactRef &contact, CSkypeProto *ppro) : contact(contact), ppro(ppro) 
	{
	}
};

struct BlockParam
{
	MCONTACT hContact;
	CSkypeProto *ppro;
	bool remove;
	bool abuse;

	BlockParam(MCONTACT hContact, CSkypeProto *ppro) : ppro(ppro) 
	{
		this->hContact = hContact;
	}
};

struct ChatRoomParam
{
	wchar_t		*id;
	StringList	invitedContacts;
	CSkypeProto *ppro;

	wchar_t		topic[256];
	wchar_t		guidline[256];

	bool		enableJoining;
	int			joinRank;

	bool		passwordProtection;
	wchar_t		password[32];
	wchar_t		confirmation[32];
	wchar_t		hint[32];

	ChatRoomParam(CSkypeProto *ppro)
		: ppro(ppro) 
	{
		this->id = NULL;
		this->topic[0] = 0;
		this->guidline[0] = 0;
		this->password[0] = 0;
		this->confirmation[0] = 0;
		this->hint[0] = 0;
		this->enableJoining = true;
		this->joinRank = Participant::WRITER;
		this->passwordProtection = false;
	}

	ChatRoomParam(const wchar_t *id, const StringList &contacts, CSkypeProto *ppro)
		: id(::mir_wstrdup(id)), invitedContacts(contacts), ppro(ppro) 
	{
		this->topic[0] = 0;
		this->guidline[0] = 0;
		this->password[0] = 0;
		this->confirmation[0] = 0;
		this->hint[0] = 0;
		this->enableJoining = true;
		this->joinRank = Participant::WRITER;
		this->passwordProtection = false;
	}

	~ChatRoomParam()
	{ ::mir_free(id); }
};

struct PasswordRequestBoxParam
{
	wchar_t		*login;
	char		*password;
	bool		rememberPassword;
	bool		showRememberPasswordBox;

	PasswordRequestBoxParam(const wchar_t *login, bool showRememberPasswordBox = true, bool rememberPassword = false) :
		login(::mir_wstrdup(login)),
		password(NULL),
		rememberPassword(rememberPassword),
		showRememberPasswordBox(showRememberPasswordBox) { }

	~PasswordRequestBoxParam()
	{
		::mir_free(login);
		::mir_free(password);
	}
};

struct PasswordChangeBoxParam
{
	char		*password;
	char		*password2;

	PasswordChangeBoxParam() { }

	~PasswordChangeBoxParam()
	{
		::mir_free(password);
		::mir_free(password2);
	}
};

struct FileParam
{
	bool isCanceled;
	bool isCompleted;
	
	unsigned __int64 size;
	unsigned __int64 transfered;

	FileParam() { }
	FileParam(unsigned __int64 size) 
	{
		this->size = size;
		this->transfered = 0;
		this->isCanceled = this->isCompleted = false;
	}
};

struct FileTransferParam
{
	//CTransfer::Refs transfers;
	PROTOFILETRANSFERSTATUS pfts;
	std::map<int, FileParam> files;

	FileTransferParam() 
	{
		this->pfts.cbSize = sizeof(this->pfts);
		this->pfts.flags = 0;
		this->pfts.currentFileNumber = 0;
		this->pfts.currentFileProgress = 0;
		this->pfts.currentFileSize = 0;
		this->pfts.currentFileTime = 0;
		this->pfts.totalBytes = 0;
		this->pfts.totalFiles = 0;
		this->pfts.totalProgress = 0;
		this->pfts.tszWorkingDir = NULL;
		this->pfts.wszCurrentFile = NULL;

		//Sid::fetch(this->transfers);
	}
};

class ChatMember;
class ChatRoom;

struct CSkypeProto : public PROTO<CSkypeProto>, private Skype
{
	friend class ChatRoom;
	friend class CAccount;
	friend class CContact;
	friend class CConversation;
	friend class CContactGroup;
	friend class CContactSearch;
	friend class CTransfer;

public:
	// PROTO_INTERFACE
	CSkypeProto(const char *protoName, const wchar_t *userName);
	~CSkypeProto();

	// PROTO_INTERFACE
	virtual	MCONTACT  __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
	virtual	MCONTACT  __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent );

	virtual	int       __cdecl Authorize( HANDLE hDbEvent );
	virtual	int       __cdecl AuthDeny( HANDLE hDbEvent, const TCHAR* szReason );
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT* );
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage );

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath );
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer );
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason );
	virtual	int       __cdecl FileResume( HANDLE hTransfer, int* action, const TCHAR** szFilename );

	virtual	DWORD_PTR __cdecl GetCaps( int type, MCONTACT hContact = NULL );
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType );

	virtual	HANDLE    __cdecl SearchBasic( const TCHAR* id );
	virtual	HANDLE    __cdecl SearchByEmail( const TCHAR* email );
	virtual	HANDLE    __cdecl SearchByName( const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName );
	virtual	HWND      __cdecl SearchAdvanced( HWND owner );
	virtual	HWND      __cdecl CreateExtendedSearchUI( HWND owner );

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT* );
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET* );
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT* );
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT* );

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR* szDescription, TCHAR** ppszFiles );
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg );
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url );

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode );
	virtual	int       __cdecl SetStatus( int iNewStatus );

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact );
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt );
	virtual	int       __cdecl SetAwayMsg( int m_iStatus, const TCHAR* msg );

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type );

	virtual	int       __cdecl OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam );

	// instances
	static CSkypeProto* InitSkypeProto(const char* protoName, const wchar_t* userName);
	static int UninitSkypeProto(CSkypeProto* ppro);

	static CSkypeProto* GetContactInstance(MCONTACT hContact);
	static void UninitInstances();

	// icons
	static void InitIcons();
	static void UninitIcons();

	// menus
	void OnInitStatusMenu();
	static void InitMenus();
	static void UninitMenus();

	// services
	static void InitServiceList();

	// hooks
	static void InitHookList();

	// languages
	static void InitLanguages();

	static INT_PTR __cdecl ParseSkypeUri(WPARAM wParam, LPARAM lParam);

private:
	// Skype
	CAccount		*newAccount(int oid);
	CContactGroup	*newContactGroup(int oid);
	CConversation	*newConversation(int oid);
	CContactSearch	*newContactSearch(int oid);
	CParticipant	*newParticipant(int oid);
	CContact		*newContact(int oid);	
	CMessage		*newMessage(int oid);
	CTransfer		*newTransfer(int oid);

	void OnMessage(
		const MessageRef & message,
		const bool & changesInboxTimestamp,
		const MessageRef & supersedesHistoryMessage,
		const ConversationRef & conversation);

	void OnConversationListChange(
		const ConversationRef& conversation,
		const Conversation::LIST_TYPE& type,
		const bool& added);

	int skypeKitPort;
	PROCESS_INFORMATION skypeKitProcessInfo;

protected:
	CAccount::Ref account;
	CContact::Refs contactList;
	CTransfer::Refs transferList;
	
	CContactGroup::Ref commonList;
	CContactGroup::Ref authWaitList;

	CRITICAL_SECTION contact_search_lock;

	bool	IsOnline();

	BYTE NeedUpdate;

	// account
	static wchar_t *LogoutReasons[];
	static wchar_t *PasswordChangeReasons[];

	wchar_t	*login;
	char	*password;
	bool	rememberPassword;	

	bool	RequestPassword(PasswordRequestBoxParam &param);
	bool	ChangePassword(PasswordChangeBoxParam &param);
	
	bool	PrepareLogin();
	bool	PreparePassword();

	void	InitProxy();
	void	SetAccountSettings();

	void	SetServerStatus(int iStatus);

	bool	LogIn();
	void	LogOut();
	
	void	OnLoggedIn();
	void	OnCblUpdated();
	void	OnLoggedOut(CAccount::LOGOUTREASON reason);

	void	OnAccountChanged(int prop);

	INT_PTR __cdecl SetMyNickName(WPARAM, LPARAM);

	// avatars
	bool IsAvatarChanged(const SEBinary &avatar, MCONTACT hContact = NULL);

	wchar_t* GetContactAvatarFilePath(MCONTACT hContact);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	SEBinary GetAvatarBinary(wchar_t *path);

	// messages
	void	OnMessageEvent(const ConversationRef &conversation, const MessageRef &message);
	void	OnMessageSent(const ConversationRef &conversation, const MessageRef &message);
	void	OnMessageReceived(const ConversationRef &conversation, const MessageRef &message);

	void	SyncMessageHystory(const ConversationRef &conversation, const time_t timestamp);
	void	SyncHistoryCommand(MCONTACT hContact, time_t timestamp);
	int __cdecl SyncLastDayHistoryCommand(WPARAM wParam, LPARAM lParam);
	int __cdecl SyncLastWeekHistoryCommand(WPARAM wParam, LPARAM lParam);
	int __cdecl SyncLastMonthHistoryCommand(WPARAM wParam, LPARAM lParam);
	int __cdecl SyncLast3MonthHistoryCommand(WPARAM wParam, LPARAM lParam);
	int __cdecl SyncLastYearHistoryCommand(WPARAM wParam, LPARAM lParam);
	int __cdecl SyncAllTimeHistoryCommand(WPARAM wParam, LPARAM lParam);

	// contacts
	void	OnContactsEvent(const ConversationRef &conversation, const MessageRef &message);
	void	OnContactsSent(const ConversationRef &conversation, const MessageRef &message);
	void	OnContactsReceived(const ConversationRef &conversation, const MessageRef &message);

	// transfer
	static wchar_t *TransferFailureReasons[];

	std::map<int, FileTransferParam> transferts;

	void	OnFileEvent(const ConversationRef &conversation, const MessageRef &message);
	void	OnTransferChanged(const TransferRef &transfer, int prop);

	// chat
	void InitChatModule();
	INT_PTR __cdecl SkypeGCGetToolTipText(WPARAM wParam, LPARAM lParam);

	void UpdateChatUserStatus(const ContactRef &contact);
	void UpdateChatUserNick(const ContactRef &contact);

	void ChatValidateContact(MCONTACT hItem, HWND hwndList, const StringList &contacts);
	void ChatPrepare(MCONTACT hItem, HWND hwndList, const StringList &contacts);

	void GetInvitedContacts(MCONTACT hItem, HWND hwndList, StringList &invitedContacts);

	void ChatRoomParseUriComands(const wchar_t *commands);

	void ChatRoomInvite(MCONTACT hContact);

	void CloseAllChatSessions();

	ChatRoom *FindChatRoom(const wchar_t *cid);

	INT_PTR __cdecl CreateChatRoomCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnJoinChat(WPARAM wParam, LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM wParam, LPARAM);

	int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);

	void	OnChatEvent(const ConversationRef &conversation, const MessageRef &message);

	OBJLIST<ChatRoom> chatRooms;

	// contacts
	void	UpdateContactAuthState(MCONTACT hContact, const ContactRef &contact);
	void	UpdateContactStatus(MCONTACT hContact, const ContactRef &contact);
	void	UpdateContactClient(MCONTACT hContact, const ContactRef &contact);
	void	UpdateContactOnlineSinceTime(MCONTACT hContact, const ContactRef &contact);
	void	UpdateContactLastEventDate(MCONTACT hContact, const ContactRef &contact);

	void	OnSearchCompleted(HANDLE hSearch);
	void	OnContactFinded(CContact::Ref contact, HANDLE hSearch);

	void	OnContactChanged(const ContactRef &contact, int prop);
	void	OnContactListChanged(const ContactRef &contact);

	bool	IsProtoContact(MCONTACT hContact);
	MCONTACT GetContactBySid(const wchar_t* sid);
	MCONTACT GetContactFromAuthEvent(HANDLE hEvent);
	MCONTACT AddContact(CContact::Ref contact, bool isTemporary = false);

	bool	IsContactOnline(MCONTACT hContact);
	void	SetAllContactStatus(int status);

	void __cdecl LoadContactList(void*);
	void __cdecl LoadChatList(void*);
	void __cdecl LoadAuthWaitList(void*);

	void __cdecl SearchBySidAsync(void*);
	void __cdecl SearchByNamesAsync(void*);
	void __cdecl SearchByEmailAsync(void*);

	// profile
	void	UpdateProfileAvatar(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileAboutText(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileBirthday(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileCity(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileCountry(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileEmails(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileFullName(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileGender(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileHomepage(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileLanguages(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileMobilePhone(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileNick(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfilePhone(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileOfficePhone(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileState(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileStatusMessage(SEObject *obj, MCONTACT hContact = NULL);
	void	UpdateProfileTimezone(SEObject *obj, MCONTACT hContact = NULL);

	void	UpdateProfile(SEObject *obj, MCONTACT hContact = NULL);

	void __cdecl LoadOwnInfo(void*);

	void SaveOwnInfoToServer(HWND hwndPage, int iPage);

	// utils
	static wchar_t* ValidationReasons[];

	static int SkypeToMirandaLoginError(CAccount::LOGOUTREASON logoutReason);

	static char *RemoveHtml(const char *data);

	static int SkypeToMirandaStatus(CContact::AVAILABILITY availability);
	static CContact::AVAILABILITY MirandaToSkypeStatus(int status);

	static bool FileExists(wchar_t *path);

	static void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

	static void CopyToClipboard(const wchar_t *text);

	static void ReplaceSpecialChars(wchar_t *text, wchar_t replaceWith = L'_');	

	// languages
	static std::map<std::wstring, std::wstring> languages;

	// instances
	static LIST<CSkypeProto> instanceList;
	static int CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2);

	//
	int SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam);
	int SendBroadcast(MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam);

	HANDLE	CreateEvent(const char* szService);
	
	// netlib
	void	InitNetLib();
	void	UninitNetLib();

	// services
	void InitInstanceServiceList();

	// hooks
	void InitInstanceHookList();

	// icons
	static _tag_iconList IconList[];
	static HANDLE GetIconHandle(const char *name);
	static HANDLE GetSkinIconHandle(const char *name);

	// menus
	HGENMENU m_hMenuRoot;
	static HANDLE hChooserMenu;
	static HGENMENU contactMenuItems[CMI_MAX];

	virtual	int __cdecl RequestAuth(WPARAM, LPARAM);
	virtual	int __cdecl GrantAuth(WPARAM, LPARAM);
	virtual	int __cdecl RevokeAuth(WPARAM, LPARAM);
	
	static INT_PTR MenuChooseService(WPARAM wParam, LPARAM lParam);

	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM);

	// blocked list
	int __cdecl BlockCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OpenBlockedListCommand(WPARAM, LPARAM);

	static INT_PTR CALLBACK SkypeBlockProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SkypeBlockedOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK SkypeBlockedOptionsSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// database
	bool IsMessageInDB(MCONTACT hContact, DWORD timestamp, SEBinary &guid, int flag = 0);

	HANDLE AddDBEvent(MCONTACT hContact, WORD type, DWORD time, DWORD flags = 0, DWORD cbBlob = 0, PBYTE pBlob = 0);
	void RaiseMessageSentEvent(
		MCONTACT hContact,
		DWORD timestamp,
		SEBinary &guid,
		const char *message,
		bool isUnread = true);
	void RaiseAuthRequestEvent(
		DWORD timestamp,
		CContact::Ref contact);

	// dialog procs
	static INT_PTR CALLBACK SkypeMainOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static INT_PTR CALLBACK SkypePasswordRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SkypePasswordChangeProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChatRoomProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SkypePrivacyOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	static INT_PTR CALLBACK SkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK PersonalSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ContactSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK HomeSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK AccountSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// skype runtime
	char *LoadKeyPair();
	int StartSkypeRuntime(const wchar_t *profileName);
	BOOL SafeTerminateProcess(HANDLE hProcess, UINT uExitCode);
	void StopSkypeRuntime();

	// events
	static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);

	int __cdecl OnProtoModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnContactDeleted(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnSrmmWindowOpen(WPARAM, LPARAM);
	int __cdecl OnUserInfoInit(WPARAM, LPARAM);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	int __cdecl OnMessagePreCreate(WPARAM, LPARAM);
	int __cdecl OnTabSRMMButtonPressed(WPARAM, LPARAM);
};
