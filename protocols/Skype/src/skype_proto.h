#pragma once

#include "skype.h"
#include <time.h>


struct CSkypeProto;

typedef void    (__cdecl CSkypeProto::* SkypeThreadFunc) (void*);
typedef INT_PTR (__cdecl CSkypeProto::* SkypeServiceFunc)(WPARAM, LPARAM);
typedef int     (__cdecl CSkypeProto::* SkypeEventFunc)(WPARAM, LPARAM);
typedef INT_PTR (__cdecl CSkypeProto::* SkypeServiceFuncParam)(WPARAM, LPARAM, LPARAM);

struct StringList : public LIST<char>
{
	static int compare(const char* p1, const char* p2)
	{ return _stricmp(p1, p2); }

	StringList() : LIST<char>(2, compare) {}
	StringList(const char* string, const char *delimeters) : LIST<char>(2, compare)
	{
		char *data = ::mir_strdup(string);
		if (data)
		{
			char *p = ::strtok(data, delimeters);
			if (p)
			{
				this->insert(::mir_strdup(p));
				while (p = strtok(NULL, delimeters))
				{
					this->insert(::mir_strdup(p));
				}
			}
			::mir_free(data);
		}
	}
	~StringList() { destroy(); }

	void destroy( void )
	{
		for (int i=0; i < count; i++)
			mir_free(items[i]);

		List_Destroy((SortedList*)this);
	}

	int insertn(const char* p) { return insert(mir_strdup(p)); }

	int remove(int idx)
	{
		mir_free(items[idx]);
		return List_Remove((SortedList*)this, idx);
	}

	int remove(const char* p)
	{
		int idx;
		return  List_GetIndex((SortedList*)this, (char*)p, &idx) == 1 ? remove(idx) : -1;
	}

	bool contains(char* p)
	{
		int idx;
		return List_GetIndex((SortedList*)this, (char*)p, &idx) == 1;
	}
};

struct _tag_iconList
{
	wchar_t*	Description;
	char*		Name;
	int			IconId;
	HANDLE		Handle;
};

struct LanguagesListEntry {
	const char *szName;
	char ISOcode[3];
};

#define LI_STRING        0
#define LI_LIST          1
#define LI_NUMBER        2

struct SettingItem
{
  const TCHAR *szDescription;
  const char *szDbSetting;
  int dbType;              //DBVT_ constant
  unsigned displayType;    //LI_ constant
};

const SettingItem setting[]={
  {LPGENT("Full name"),		"Nick",			DBVT_WCHAR,	LI_STRING},
  {LPGENT("Mood"),			"XStatusMsg",	DBVT_WCHAR,	LI_STRING},

  {LPGENT("Mobile phone"),	"Cellular",		DBVT_WCHAR,	LI_NUMBER},
  {LPGENT("Home phone"),	"Phone",		DBVT_WCHAR,	LI_NUMBER},
  {LPGENT("Office phone"),	"CompanyPhone",	DBVT_WCHAR,	LI_NUMBER},
  {LPGENT("E-mail 1"),		"e-mail0",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("E-mail 2"),		"e-mail1",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("E-mail 3"),		"e-mail2",		DBVT_WCHAR,	LI_STRING},

  {LPGENT("Country"),		"Country",		DBVT_WCHAR,	LI_LIST},
  {LPGENT("State"),			"State",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("City"),			"City",			DBVT_WCHAR,	LI_STRING},
  {LPGENT("Time zone"),		"Timezone",		DBVT_BYTE,	LI_LIST},
  {LPGENT("Homepage"),		"Homepage",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("Gender"),		"Gender",		DBVT_BYTE,	LI_LIST},
  {LPGENT("Birth day"),		"BirthDay",		DBVT_BYTE,	LI_NUMBER},
  {LPGENT("Birth month"),	"BirthMonth",	DBVT_BYTE,	LI_NUMBER},
  {LPGENT("Birth year"),	"BirthYear",	DBVT_WORD,	LI_NUMBER},
  {LPGENT("Language"),		"Language1",	DBVT_WCHAR,	LI_LIST},

  {LPGENT("About"),			"About",		DBVT_WCHAR,	LI_STRING}
};


struct HtmlEntity
{
	const char *entity;
	char symbol;
};

const HtmlEntity htmlEntities[]={
	{"nbsp",	' '},
	{"amp",		'&'},
	{"quot",	'"'},
	{"lt",		'<'},
	{"gt",		'>'},
	{"apos",	'\''},
	{"copy",	'©'},
	// TODO: add more
};

struct InviteChatParam
{
	char		*id;
	HANDLE		hContact;
	CSkypeProto *ppro;

	InviteChatParam(const char *id, HANDLE hContact, CSkypeProto *ppro)
		: id(::mir_strdup(id)), hContact(hContact), ppro(ppro) {}

	~InviteChatParam()
	{ ::mir_free(id); }
};

struct PasswordRequestBoxParam
{
	char		*login;
	char		*password;
	bool		rememberPassword;
	bool		showRememberPasswordBox;

	PasswordRequestBoxParam(const char *login, bool showRememberPasswordBox = true, bool rememberPassword = false) :
		login(::mir_strdup(login)),
		password(NULL),
		rememberPassword(rememberPassword),
		showRememberPasswordBox(showRememberPasswordBox) { }

	~PasswordRequestBoxParam()
	{
		if (login) ::mir_free(login);
		if (password) ::mir_free(password);
	}
};

struct PasswordChangeBoxParam
{
	char		*password;
	char		*password2;

	PasswordChangeBoxParam() { }

	~PasswordChangeBoxParam()
	{
		if (password) ::mir_free(password);
		if (password2) ::mir_free(password2);
	}
};

struct FileTransfer
{
	CSkypeProto* ppro;
	SEBinary guid;
	CTransfer::Refs transfers;
	//char *who;
	//char *msg;
	//char *ftoken;
	//char *relay;
	//HANDLE hContact;
	//int  cancel;
	//char *url;
	//HANDLE hWaitEvent;
	//DWORD action;
	//int y7;
	////YList *files;
	//PROTOFILETRANSFERSTATUS pfts;

	FileTransfer(CSkypeProto* ppro) { this->ppro = ppro; }
};

struct CSkypeProto : public PROTO_INTERFACE
{
public:
	CSkypeProto(const char *, const wchar_t *);
	~CSkypeProto();

	// PROTO_INTERFACE
	virtual	HANDLE __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
	virtual	HANDLE __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent );

	virtual	int    __cdecl Authorize( HANDLE hDbEvent );
	virtual	int    __cdecl AuthDeny( HANDLE hDbEvent, const TCHAR* szReason );
	virtual	int    __cdecl AuthRecv( HANDLE hContact, PROTORECVEVENT* );
	virtual	int    __cdecl AuthRequest( HANDLE hContact, const TCHAR* szMessage );

	virtual	HANDLE __cdecl ChangeInfo( int iInfoType, void* pInfoData );

	virtual	HANDLE __cdecl FileAllow( HANDLE hContact, HANDLE hTransfer, const TCHAR* szPath );
	virtual	int    __cdecl FileCancel( HANDLE hContact, HANDLE hTransfer );
	virtual	int    __cdecl FileDeny( HANDLE hContact, HANDLE hTransfer, const TCHAR* szReason );
	virtual	int    __cdecl FileResume( HANDLE hTransfer, int* action, const TCHAR** szFilename );

	virtual	DWORD_PTR __cdecl GetCaps( int type, HANDLE hContact = NULL );
	virtual	int    __cdecl GetInfo( HANDLE hContact, int infoType );

	virtual	HANDLE __cdecl SearchBasic( const TCHAR* id );
	virtual	HANDLE __cdecl SearchByEmail( const TCHAR* email );
	virtual	HANDLE __cdecl SearchByName( const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName );
	virtual	HWND   __cdecl SearchAdvanced( HWND owner );
	virtual	HWND   __cdecl CreateExtendedSearchUI( HWND owner );

	virtual	int    __cdecl RecvContacts( HANDLE hContact, PROTORECVEVENT* );
	virtual	int    __cdecl RecvFile( HANDLE hContact, PROTORECVFILET* );
	virtual	int    __cdecl RecvMsg( HANDLE hContact, PROTORECVEVENT* );
	virtual	int    __cdecl RecvUrl( HANDLE hContact, PROTORECVEVENT* );

	virtual	int    __cdecl SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList );
	virtual	HANDLE __cdecl SendFile( HANDLE hContact, const TCHAR* szDescription, TCHAR** ppszFiles );
	virtual	int    __cdecl SendMsg( HANDLE hContact, int flags, const char* msg );
	virtual	int    __cdecl SendUrl( HANDLE hContact, int flags, const char* url );

	virtual	int    __cdecl SetApparentMode( HANDLE hContact, int mode );
	virtual	int    __cdecl SetStatus( int iNewStatus );

	virtual	HANDLE __cdecl GetAwayMsg( HANDLE hContact );
	virtual	int    __cdecl RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt );
	virtual	int    __cdecl SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg );
	virtual	int    __cdecl SetAwayMsg( int m_iStatus, const TCHAR* msg );

	virtual	int    __cdecl UserIsTyping( HANDLE hContact, int type );

	virtual	int    __cdecl OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam );

	virtual	int __cdecl RequestAuth(WPARAM, LPARAM);
	virtual	int __cdecl GrantAuth(WPARAM, LPARAM);
	virtual	int __cdecl RevokeAuth(WPARAM, LPARAM);

	// events
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnContactDeleted(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnUserInfoInit(WPARAM, LPARAM);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	// instances
	static CSkypeProto* InitSkypeProto(const char* protoName, const wchar_t* userName);
	static int UninitSkypeProto(CSkypeProto* ppro);

	// services
	static void InitServiceList();

	// icons
	static void InitIcons();
	static void UninitIcons();
	static HANDLE GetIconHandle(const char *name);

	// menus
	void OnInitStatusMenu();
	static void InitMenus();
	static void UninitMenus();

	INT_PTR __cdecl InviteCommand(WPARAM, LPARAM);

	static CSkypeProto* GetInstanceByHContact(HANDLE hContact);
	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);

	bool	IsOnline();

protected:
	DWORD   dwCMDNum;
	CSkype *skype;
	CAccount::Ref account;
	CContact::Refs contactList;
//	CTransfer::Refs transferList;
	CContactGroup::Ref commonList;
	CContactGroup::Ref authWaitList;	

	// account
	void	OnAccountChanged(int prop);

	char	*login;
	char	*password;
	bool	rememberPassword;
	bool	RequestPassword(PasswordRequestBoxParam &param);
	bool	ChangePassword(PasswordChangeBoxParam &param);

	HANDLE	signin_lock;
	bool	SignIn(int status);
	void __cdecl SignInAsync(void*);

	static wchar_t* LogoutReasons[];
	static wchar_t* ValidationReasons[];
	static wchar_t* PasswordChangeReasons[];
	static LanguagesListEntry languages[223];

	// messages
	void	OnMessage(CConversation::Ref conversation, CMessage::Ref message);
	void	OnMessageSended(CConversation::Ref conversation, CMessage::Ref message);
	void	OnMessageReceived(CConversation::Ref conversation, CMessage::Ref message);

	// file transfer
	LIST<FileTransfer> fileTransferList;
	FileTransfer *FindTransfer(SEBinary guid);
	FileTransfer *FindFileTransfer(CTransfer::Ref transfer);
	
	void	OnFileReceived(CConversation::Ref conversation, CMessage::Ref message);
	void	OnTransferChanged(int prop, CTransfer::Ref transfer);

	// chat
	static char* Groups[];

	bool IsChatRoom(HANDLE hContact);
	HANDLE GetChatRoomByID(const char *cid);
	HANDLE	AddChatRoomByID(const char* cid, const char* name, DWORD flags = 0);

	char *CSkypeProto::GetChatUsers(const char *cid);

	void ChatValidateContact(HANDLE hItem, HWND hwndList, const char *contacts);
	void ChatPrepare(HANDLE hItem, HWND hwndList, const char *contacts);

	void GetInviteContacts(HANDLE hItem, HWND hwndList, SEStringList &invitedContacts);

	void InitChat();
	char *StartChat(const char *cid, const SEStringList &invitedContacts);
	void JoinToChat(const char *cid, bool showWindow = true);
	void LeaveChat(const char *cid);

	void RaiseChatEvent(const char *cid, const char *sid, int evt, const DWORD itemData = 0, const char *status = NULL, const char *message = NULL);
	void SendChatMessage(const char *cid, const char *sid, const char *message);
	void AddChatContact(const char *cid, const char *sid, const char *group, const WORD status = ID_STATUS_ONLINE);
	void KickChatContact(const char *cid, const char *sid);
	void RemoveChatContact(const char *cid, const char *sid);

	INT_PTR __cdecl OnJoinChat(WPARAM wParam, LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM wParam, LPARAM);

	int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);
	int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);

	// contacts
	void	UpdateContactAuthState(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactAvatar(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactStatus(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactOnlineSinceTime(SEObject *obj, HANDLE hContact);
	void	UpdateContactLastEventDate(SEObject *obj, HANDLE hContact);

	void	OnSearchCompleted(HANDLE hSearch);
	void	OnContactFinded(HANDLE hSearch, CContact::Ref contact);

	void	OnContactChanged(CContact::Ref contact, int prop);
	void	OnContactListChanged(const ContactRef& contact);

	bool	IsProtoContact(HANDLE hContact);
	HANDLE	GetContactBySid(const char* sid);
	HANDLE	GetContactFromAuthEvent(HANDLE hEvent);
	HANDLE	AddContactBySid(const char* sid, const char* nick, DWORD flags = 0);

	void	SetAllContactStatus(int status);

	void __cdecl LoadContactList(void*);
	void __cdecl SearchBySidAsync(void*);
	void __cdecl SearchByNamesAsync(void*);
	void __cdecl SearchByEmailAsync(void*);

	// profile
	void	UpdateProfileAvatar(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileAboutText(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileBirthday(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileCity(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileCountry(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileEmails(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileFullName(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileGender(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileHomepage(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileLanguages(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileMobilePhone(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfilePhone(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileOfficePhone(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileState(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileStatusMessage(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileTimezone(SEObject *obj, HANDLE hContact = NULL);

	void	UpdateProfile(SEObject *obj, HANDLE hContact = NULL);

	void __cdecl LoadOwnInfo(void*);

	// utils
	static void FakeAsync(void*);
	void InitCustomFolders();

	HANDLE m_hAvatarsFolder;
	bool   m_bInitDone;

	static int DetectAvatarFormatBuffer(const char *pBuffer);
	static int DetectAvatarFormat(const wchar_t *path);

	wchar_t* GetContactAvatarFilePath(HANDLE hContact);

	int SkypeToMirandaLoginError(CAccount::LOGOUTREASON logoutReason);

	static void ShowNotification(const wchar_t *message, int flags = 0, HANDLE hContact = NULL);
	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, HANDLE hContact = NULL);

	static char *RemoveHtml(char *data);

	int		SkypeToMirandaStatus(CContact::AVAILABILITY availability);
	CContact::AVAILABILITY MirandaToSkypeStatus(int status);

	// runtime
	void InitSkype();
	void UninitSkype();

	// instances
	static LIST<CSkypeProto> instanceList;
	static int CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2);

	void	CreateService(const char* szService, SkypeServiceFunc serviceProc);
	void	CreateServiceParam(const char* szService, SkypeServiceFunc serviceProc, LPARAM lParam);

	HANDLE	CreateEvent(const char* szService);
	void	HookEvent(const char*, SkypeEventFunc);

	int		SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam);
	int		SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam);
	DWORD	SendBroadcastAsync(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam, size_t paramSize = 0);

	void	ForkThread(SkypeThreadFunc, void*);
	HANDLE	ForkThreadEx(SkypeThreadFunc, void*, UINT* threadID = NULL);

	// netlib
	HANDLE	hNetLibUser;
	void	InitNetLib();
	void	UninitNetLib();
	void	InitProxy();
	void	Log(const wchar_t *fmt, ...);

	// services
	static LIST<void> serviceList;

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	// icons
	static _tag_iconList IconList[];

	// menu
	HGENMENU m_hMenuRoot;
	static HANDLE hChooserMenu;
	static HANDLE hPrebuildMenuHook;
	static HANDLE g_hContactMenuItems[CMITEMS_COUNT];
	static HANDLE g_hContactMenuSvc[CMITEMS_COUNT];
	static INT_PTR MenuChooseService(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM);


	// database
	HANDLE AddDataBaseEvent(HANDLE hContact, WORD type, DWORD time, DWORD flags, DWORD cbBlob, PBYTE pBlob);
	void RaiseMessageReceivedEvent(
		DWORD timestamp,
		const char* sid,
		const char* nick,
		const char* message = "");
	void RaiseMessageSendedEvent(
		DWORD timestamp,
		const char* sid,
		const char* nick,
		const char* message = "");
	void RaiseFileReceivedEvent(
		DWORD timestamp,
		const char* sid,
		const char* nick,
		const char* message = "");
	void RaiseAuthRequestEvent(
		DWORD timestamp,
		const char* sid,
		const char* nick,
		const char* firstName = "",
		const char* lastName = "",
		const char* reason = "");

	// database settings
	BYTE	GetSettingByte(const char *setting, BYTE errorValue = 0);
	BYTE	GetSettingByte(HANDLE hContact, const char *setting, BYTE errorValue = 0);
	WORD	GetSettingWord(const char *setting, WORD errorValue = 0);
	WORD	GetSettingWord(HANDLE hContact, const char *setting, WORD errorValue = 0);
	DWORD	GetSettingDword(const char *setting, DWORD defVal = 0);
	DWORD	GetSettingDword(HANDLE hContact, const char *setting, DWORD errorValue = 0);
	wchar_t *GetSettingString(const char *setting, wchar_t* errorValue = NULL);
	wchar_t *GetSettingString(HANDLE hContact, const char *setting, wchar_t* errorValue = NULL);
	char	*GetDecodeSettingString(HANDLE hContact, const char *setting, char* errorValue = NULL);
	//
	bool	SetSettingByte(const char *setting, BYTE value);
	bool	SetSettingByte(HANDLE hContact, const char *setting, BYTE value);
	bool	SetSettingWord(const char *setting, WORD value);
	bool	SetSettingWord(HANDLE hContact, const char *setting, WORD value);
	bool	SetSettingDword(const char *setting, DWORD value);
	bool	SetSettingDword(HANDLE hContact, const char *setting, DWORD value);
	bool	SetSettingString(const char *setting, const wchar_t* value);
	bool	SetSettingString(HANDLE hContact, const char *setting, const wchar_t* value);
	bool	SetDecodeSettingString(HANDLE hContact, const char *setting, const char* value);
	//
	void	DeleteSetting(const char *setting);
	void	DeleteSetting(HANDLE hContact, const char *setting);

	// dialog procs
	static INT_PTR CALLBACK SkypeMainOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static INT_PTR CALLBACK SkypePasswordRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SkypePasswordChangeProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK OwnSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK InviteToChatProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};