#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#include "http_request.h"

struct CaptchaParam
{
	BYTE *data;
	size_t size;
	char text[10];
};

struct GuardParam
{
	char domain[32];
	char code[10];
};

class CSteamProto : public PROTO<CSteamProto>
{
public:
	// PROTO_INTERFACE
	CSteamProto(const char *protoName, const wchar_t *userName);
	~CSteamProto();

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

	// accounts
	static CSteamProto* InitAccount(const char* protoName, const wchar_t* userName);
	static int UninitAccount(CSteamProto* ppro);

	static CSteamProto* GetContactAccount(MCONTACT hContact);
	static void UninitAccounts();

protected:

	NETLIBHTTPREQUEST *LoginRequest(const char *username, const char *password, const char *timestamp, const char *captchagid, const char *captcha_text, const char *emailauth, const char *emailsteamid);

	bool Login();
	bool Logout();

	// accounts
	static LIST<CSteamProto> InstanceList;
	static int CompareProtos(const CSteamProto *p1, const CSteamProto *p2);

	//events
	int OnModulesLoaded(WPARAM, LPARAM);
	int OnPreShutdown(WPARAM, LPARAM);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	//options
	static INT_PTR CALLBACK GuardProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK CaptchaProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK MainOptionsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif //_STEAM_PROTO_H_