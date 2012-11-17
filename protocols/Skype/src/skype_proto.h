#pragma once

#include "skype.h"
#include <time.h>
#include <stdlib.h>

struct CSkypeProto;

typedef void    (__cdecl CSkypeProto::* SkypeThreadFunc) (void*);
typedef INT_PTR (__cdecl CSkypeProto::* SkypeServiceFunc)(WPARAM, LPARAM);
typedef int     (__cdecl CSkypeProto::* SkypeEventFunc)(WPARAM, LPARAM);
typedef INT_PTR (__cdecl CSkypeProto::* SkypeServiceFuncParam)(WPARAM, LPARAM, LPARAM);

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

struct CSkypeProto : public PROTO_INTERFACE, public MZeroedObject
{
public:
	CSkypeProto(const char*, const TCHAR*);
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
	virtual	HICON  __cdecl GetIcon( int iconIndex );
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
	int __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

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
	static void InitMenus();
	static void UninitMenus();

	static CSkypeProto* GetInstanceByHContact(HANDLE hContact);
	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);

protected:
	DWORD   dwCMDNum;
	CAccount::Ref account;
	CContact::Refs contactList;
	CContactGroup::Ref commonList;
	CContactGroup::Ref authWaitList;

	wchar_t	*login;
	wchar_t *password;
	bool	rememberPassword;
	void	RequestPassword();

	HANDLE	signin_lock;
	bool	SignIn(bool isReadPassword = true);
	void __cdecl SignInAsync(void*);

	bool	IsOnline();

	static LanguagesListEntry languages[223];

	// messages
	void	OnOnMessageReceived(const char *sid, const char *text);
	void	OnConversationAdded(CConversation::Ref conversation);

	// contacts
	void	UpdateContactAboutText(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactAuthState(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactAvatar(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactBirthday(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactCity(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactCountry(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactEmails(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactGender(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactHomepage(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactLanguages(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactMobilePhone(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactPhone(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactOfficePhone(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactState(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactStatus(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactStatusMessage(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactTimezone(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactProfile(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactOnlineSinceTime(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactLastEventDate(HANDLE hContact, CContact::Ref contact);
	void	UpdateFullName(HANDLE hContact, CContact::Ref contact);

	void	OnSearchCompleted(HANDLE hSearch);
	void	OnContactFinded(HANDLE hSearch, CContact::Ref contact);

	void	OnContactChanged(CContact::Ref contact, int prop);
	void	OnContactListChanged(const ContactRef& contact);
	
	bool	IsProtoContact(HANDLE hContact);
	HANDLE	GetContactBySid(const wchar_t* sid);
	HANDLE	GetContactFromAuthEvent(HANDLE hEvent);
	HANDLE	AddContactBySid(const wchar_t* skypeName, const wchar_t* nick, DWORD flags = 0);

	int		SkypeToMirandaStatus(CContact::AVAILABILITY availability);
	CContact::AVAILABILITY MirandaToSkypeStatus(int status);
	void	SetAllContactStatus(int status);

	void __cdecl LoadContactList(void*);
	void __cdecl SearchBySidAsync(void*);
	void __cdecl SearchByNamesAsync(void*);
	void __cdecl SearchByEmailAsync(void*);

	// profile
	void	UpdateOwnAvatar();
	void	UpdateOwnBirthday();
	void	UpdateOwnCity();
	void	UpdateOwnCountry();
	void	UpdateOwnEmails();
	void	UpdateOwnGender();
	void	UpdateOwnHomepage();
	void	UpdateOwnLanguages();
	void	UpdateOwnMobilePhone();
	void	UpdateOwnNickName();
	void	UpdateOwnPhone();
	void	UpdateOwnOfficePhone();
	void	UpdateOwnState();
	void	UpdateOwnStatusMessage();
	void	UpdateOwnTimezone();
	void	UpdateOwnProfile();	

	void	OnAccountChanged(int prop);

	void __cdecl LoadOwnInfo(void*);

	// utils
	static void FakeAsync(void*);

	static int DetectAvatarFormatBuffer(const char *pBuffer);
	static int DetectAvatarFormat(const wchar_t *path);

	wchar_t* GetContactAvatarFilePath(HANDLE hContact = 0);

	int SkypeToMirandaLoginError(CAccount::LOGOUTREASON logoutReason);

	void ShowNotification(const wchar_t *sid, const wchar_t *message, int flags = 0);

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
	HANDLE	hNetlibUser;
	void	InitNetLib();
	void	UninitNetLib();
	void	Log(const char* fmt, ...);

	// services
	static LIST<void> serviceList;

	int __cdecl GetAvatarInfo(WPARAM, LPARAM);
	int __cdecl GetAvatarCaps(WPARAM, LPARAM);
	int __cdecl GetMyAvatar(WPARAM, LPARAM);
	int __cdecl SetMyAvatar(WPARAM, LPARAM);

	// icons
	static _tag_iconList iconList[];

	// menu
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
	wchar_t*	GetSettingString(const char *setting, wchar_t* errorValue = NULL);
	wchar_t*	GetSettingString(HANDLE hContact, const char *setting, wchar_t* errorValue = NULL);
	wchar_t*	GetDecodeSettingString(const char *setting, wchar_t* errorValue = NULL);
	wchar_t*	GetDecodeSettingString(HANDLE hContact, const char *setting, wchar_t* errorValue = NULL);
	//
	bool	SetSettingByte(const char *setting, BYTE value);
	bool	SetSettingByte(HANDLE hContact, const char *setting, BYTE value);
	bool	SetSettingWord(const char *setting, WORD value);
	bool	SetSettingWord(HANDLE hContact, const char *setting, WORD value);
	bool	SetSettingDword(const char *setting, DWORD value);
	bool	SetSettingDword(HANDLE hContact, const char *setting, DWORD value);
	bool	SetSettingString(const char *setting, const wchar_t* value);
	bool	SetSettingString(HANDLE hContact, const char *setting, const wchar_t* value);
	bool	SetDecodeSettingString(const char *setting, const wchar_t* value);
	bool	SetDecodeSettingString(HANDLE hContact, const char *setting, const wchar_t* value);
	//
	void	DeleteSetting(const char *setting);
	void	DeleteSetting(HANDLE hContact, const char *setting);

	// dialog procs
	static INT_PTR CALLBACK SkypeAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static INT_PTR CALLBACK SkypeOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static INT_PTR CALLBACK SkypePasswordProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};