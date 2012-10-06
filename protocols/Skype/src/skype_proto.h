#pragma once

#include "skype.h"

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

	// utils
	static int countriesCount;
	static CountryListEntry* countryList;

	// events
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	// instances
	static CSkypeProto* InitSkypeProto(const char* protoName, const wchar_t* userName);
	static int UninitSkypeProto(CSkypeProto* ppro);

	// services
	static void InitServiceList();
	static void UninitServiceList();

	// icons
	static void InitIcons();
	static void UninitIcons();

	// menus
	static void InitMenus();
	static void UninitMenus();

	static CSkypeProto* GetInstanceByHContact(HANDLE hContact);
	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);

protected:
	CAccount::Ref account;
	CContactGroup::Ref contactGroup;

	TCHAR*	login;
	TCHAR*	password;

	HANDLE	signin_lock;
	void __cdecl SignIn(void*);

	void	LoadContactInfo(HANDLE hContact, CContact::Ref contact);
	void __cdecl LoadContactList(void*);

	// contacts
	void  OnContactChanged(CContact* contact, int prop);
	
	bool	IsProtoContact(HANDLE hContact);
	HANDLE	AddContactBySkypeName(wchar_t* skypeName, wchar_t* displayName, DWORD flags);
	HANDLE	GetContactBySkypeName(wchar_t* skypeName);
	int		SkypeToMirandaStatus(CContact::AVAILABILITY availability);
	CContact::AVAILABILITY MirandaToSkypeStatus(int status);
	void	SetAllContactStatus(int status);

	// utils
	static char* GetCountryNameById(int countryId);
	static int GetCountryIdByName(const char* countryName);

	// instances
	static LIST<CSkypeProto> instanceList;
	static int CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2);

	void	CreateService(const char* szService, SkypeServiceFunc serviceProc);
	void	CreateServiceParam(const char* szService, SkypeServiceFunc serviceProc, LPARAM lParam);
	
	HANDLE	CreateEvent(const char* szService);
	void	HookEvent(const char*, SkypeEventFunc);

	int		SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam);
	int		SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam);

	void	ForkThread(SkypeThreadFunc, void*);
	HANDLE	ForkThreadEx(SkypeThreadFunc, void*, UINT* threadID = NULL);

	// netlib
	HANDLE	hNetlibUser;
	void	InitNetLib();
	void	UninitNetLib();
	void	Log(const char* fmt, ...);

	// services
	static LIST<HANDLE> serviceList;

	// icons
	static _tag_iconList iconList[];

	// menu
	static HANDLE hPrebuildMenuHook;

	int		OnPrebuildContactMenu(WPARAM wParam, LPARAM);

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

	bool	SetSettingByte(const char *setting, BYTE value);
	bool	SetSettingByte(HANDLE hContact, const char *setting, BYTE value);
	bool	SetSettingWord(const char *setting, WORD value);
	bool	SetSettingWord(HANDLE hContact, const char *setting, WORD value);
	bool	SetSettingDword(const char *setting, DWORD value);
	bool	SetSettingDword(HANDLE hContact, const char *setting, DWORD value);
	bool	SetSettingString(const char *setting, wchar_t* value);
	bool	SetSettingString(HANDLE hContact, const char *setting, wchar_t* value);
	bool	SetDecodeSettingString(const char *setting, wchar_t* value);
	bool	SetDecodeSettingString(HANDLE hContact, const char *setting, wchar_t* value);

	// dialog procs
	static INT_PTR CALLBACK SkypeAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	static INT_PTR CALLBACK SkypeOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};