#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#define STEAM_SEARCH_BYID 1001
#define STEAM_SEARCH_BYNAME 1002

struct GuardParam
{
	char code[10];
	char domain[32];
};

struct CaptchaParam
{
	BYTE *data;
	size_t size;
	char text[10];
};

struct SendMessageParam
{
	MCONTACT hContact;
	HANDLE hMessage;
	const char *text;
};

struct STEAM_SEARCH_RESULT
{
	PROTOSEARCHRESULT hdr;
	const SteamWebApi::FriendApi::Summary *contact;
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

	// instances
	static CSteamProto* InitProtoInstance(const char* protoName, const wchar_t* userName);
	static int UninitProtoInstance(CSteamProto* ppro);

	static CSteamProto* GetContactProtoInstance(MCONTACT hContact);
	static void UninitProtoInstances();

protected:
	bool m_bTerminated;
	HANDLE m_hPollingThread;
	ULONG  hMessageProcess;
	CRITICAL_SECTION contact_search_lock;

	// instances
	static LIST<CSteamProto> InstanceList;
	static int CompareProtos(const CSteamProto *p1, const CSteamProto *p2);

	// pooling thread
	void PollServer(const char *sessionId, const char *steamId, UINT32 messageId, SteamWebApi::PollApi::PollResult *pollResult);
	void __cdecl PollingThread(void*);

	// account
	bool IsOnline();
	bool IsMe(const char *steamId);
	void SetServerStatus(WORD status);
	void Authorize(SteamWebApi::AuthorizationApi::AuthResult *authResult);
	void __cdecl LogInThread(void*);
	void __cdecl LogOutThread(void*);
	void __cdecl SetServerStatusThread(void*);

	// contacts
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	void UpdateContact(MCONTACT hContact, const SteamWebApi::FriendApi::Summary *contact);
	void __cdecl UpdateContactsThread(void*);

	MCONTACT FindContact(const char *steamId);
	MCONTACT AddContact(const SteamWebApi::FriendApi::Summary *contact);

	void __cdecl LoadContactListThread(void*);
	
	void __cdecl SearchByIdThread(void*);
	void __cdecl SearchByNameThread(void*);

	// messages
	void __cdecl SendMessageThread(void*);
	void __cdecl SendTypingThread(void*);

	// avatars
	wchar_t * GetAvatarFilePath(MCONTACT hContact);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	//events
	int OnModulesLoaded(WPARAM, LPARAM);
	int OnPreShutdown(WPARAM, LPARAM);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);
	static int __cdecl OnOptionsInit(void *obj, WPARAM wParam, LPARAM lParam);

	// utils
	static WORD SteamToMirandaStatus(int state);
	static int MirandaToSteamState(int status);

	static int RsaEncrypt(const SteamWebApi::RsaKeyApi::RsaKey &rsaKey, const char *data, DWORD dataSize, BYTE *encrypted, DWORD &encryptedSize);

	// options
	static INT_PTR CALLBACK GuardProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK CaptchaProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK MainOptionsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif //_STEAM_PROTO_H_