#pragma once

#include "skype.h"
#include "string_list.h"

#include "skypekit\common.h"
#include "skypekit\group.h"
#include "skypekit\search.h"
#include "skypekit\account.h"
#include "skypekit\contact.h"
#include "skypekit\message.h"
#include "skypekit\transfer.h"
#include "skypekit\participant.h"
#include "skypekit\conversation.h"

#include <map>
#include <string>

typedef void    (__cdecl CSkypeProto::* SkypeThreadFunc) (void*);
typedef int     (__cdecl CSkypeProto::* SkypeEventFunc)(WPARAM, LPARAM);
typedef INT_PTR (__cdecl CSkypeProto::* SkypeServiceFunc)(WPARAM, LPARAM);
typedef INT_PTR (__cdecl CSkypeProto::* SkypeServiceFuncParam)(WPARAM, LPARAM, LPARAM);

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

struct InviteChatParam
{
	wchar_t		*id;
	StringList	invitedContacts;
	CSkypeProto *ppro;

	InviteChatParam(const wchar_t *id, const StringList &contacts, CSkypeProto *ppro)
		: id(::mir_wstrdup(id)), invitedContacts(contacts), ppro(ppro) { /*this->invitedContacts = contacts;*/ }

	~InviteChatParam()
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

struct CSkypeProto : public PROTO_INTERFACE, private Skype
{
public:
	// PROTO_INTERFACE
	CSkypeProto(const char *protoName, const wchar_t *userName);
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

	// instances
	static CSkypeProto* InitSkypeProto(const char* protoName, const wchar_t* userName);
	static int UninitSkypeProto(CSkypeProto* ppro);

	static CSkypeProto* GetInstanceByHContact(HANDLE hContact);

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

	bool CreateConferenceWithConsumers(ConversationRef &conference, const SEStringList &identities);

	void OnMessage(
		const MessageRef & message,
		const bool & changesInboxTimestamp,
		const MessageRef & supersedesHistoryMessage,
		const ConversationRef & conversation);

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

	// avatars
	bool IsAvatarChanged(const SEBinary &avatar, HANDLE hContact = NULL);

	static int DetectAvatarFormatBuffer(const char *pBuffer);
	static int DetectAvatarFormat(const wchar_t *path);

	wchar_t* GetContactAvatarFilePath(HANDLE hContact);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	SEBinary GetAvatarBinary(wchar_t *path);

	// messages
	void	OnMessageEvent(const ConversationRef &conversation, const MessageRef &message);
	void	OnMessageSent(const ConversationRef &conversation, const MessageRef &message);
	void	OnMessageReceived(const ConversationRef &conversation, const MessageRef &message);

	// contacts
	void	OnContactsEvent(const ConversationRef &conversation, const MessageRef &message);
	void	OnContactsSent(const ConversationRef &conversation, const MessageRef &message);
	void	OnContactsReceived(const ConversationRef &conversation, const MessageRef &message);

	// transfer
	void	OnFileEvent(const ConversationRef &conversation, const MessageRef &message);
	void	OnTransferChanged(CTransfer::Ref transfer, int prop);

	// chat
	static wchar_t* Roles[];

	bool IsChatRoom(HANDLE hContact);
	HANDLE GetChatRoomByCid(const wchar_t *cid);
	HANDLE AddChatRoom(CConversation::Ref conversation);

	wchar_t *CSkypeProto::GetChatUsers(const wchar_t *cid);
	void CSkypeProto::UpdateChatUserStatus(CContact::Ref contact);

	void ChatValidateContact(HANDLE hItem, HWND hwndList, const StringList &contacts);
	void ChatPrepare(HANDLE hItem, HWND hwndList, const StringList &contacts);

	void GetInvitedContacts(HANDLE hItem, HWND hwndList, StringList &invitedContacts);

	void InitChat();
	CConversation::Ref StartChat();
	CConversation::Ref StartChat(StringList &invitedContacts);
	void LeaveChat(const wchar_t *cid);

	void CreateChatWindow(CConversation::Ref conversation, bool showWindow = true);

	void JoinToChat(CConversation::Ref conversation, bool showWindow = true);
	void InviteConactsToChat(CConversation::Ref conversation, const StringList &invitedContacts);
	void AddConactsToChat(CConversation::Ref conversation, const StringList &invitedContacts);

	void RaiseChatEvent(const wchar_t *cid, const wchar_t *sid, int evt, DWORD flags = 0x0001, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL, DWORD timestamp = time(NULL));
	void SendChatMessage(const wchar_t *cid, const wchar_t *sid, const wchar_t *message);
	void AddChatContact(const wchar_t *cid, const wchar_t *sid, const wchar_t *group, const WORD status = ID_STATUS_ONLINE);
	void KickChatContact(const wchar_t *cid, const wchar_t *sid);
	void RemoveChatContact(const wchar_t *cid, const wchar_t *sid);	

	INT_PTR __cdecl OnJoinChat(WPARAM wParam, LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM wParam, LPARAM);

	int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);
	int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);
	
	void	OnChatMessageSent(const ConversationRef &conversation, const MessageRef &message, uint messageType);
	void	OnChatMessageReceived(const ConversationRef &conversation, const MessageRef &message, uint messageType);
	void	OnChatEvent(const ConversationRef &conversation, const MessageRef &message);

	// contacts
	void	UpdateContactAuthState(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactAvatar(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactStatus(HANDLE hContact, CContact::Ref contact);
	void	UpdateContactNickName(SEObject *obj, HANDLE hContact);
	void	UpdateContactOnlineSinceTime(SEObject *obj, HANDLE hContact);
	void	UpdateContactLastEventDate(SEObject *obj, HANDLE hContact);

	void	OnSearchCompleted(HANDLE hSearch);
	void	OnContactFinded(CContact::Ref contact, HANDLE hSearch);

	void	OnContactChanged(CContact::Ref contact, int prop);
	void	OnContactListChanged(CContact::Ref contact);

	bool	IsProtoContact(HANDLE hContact);
	HANDLE	GetContactBySid(const wchar_t* sid);
	HANDLE	GetContactFromAuthEvent(HANDLE hEvent);
	HANDLE	AddContact(CContact::Ref contact);

	bool	IsContactOnline(HANDLE hContact);
	void	SetAllContactStatus(int status);

	void __cdecl LoadContactList(void*);
	void __cdecl LoadChatList(void*);
	void __cdecl LoadAuthWaitList(void*);

	void __cdecl SearchBySidAsync(void*);
	void __cdecl SearchByNamesAsync(void*);
	void __cdecl SearchByEmailAsync(void*);

	// profile
	//static std::map<std::wstring, std::wstring> FillLanguages();

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
	void	UpdateProfileNickName(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfilePhone(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileOfficePhone(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileState(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileStatusMessage(SEObject *obj, HANDLE hContact = NULL);
	void	UpdateProfileTimezone(SEObject *obj, HANDLE hContact = NULL);

	void	UpdateProfile(SEObject *obj, HANDLE hContact = NULL);

	void __cdecl LoadOwnInfo(void*);

	void SaveOwnInfoToServer(HWND hwndPage, int iPage);

	// utils
	static wchar_t* ValidationReasons[];

	static void FakeAsync(void*);
	void InitCustomFolders();

	HANDLE m_hAvatarsFolder;
	bool   m_bInitDone;

	static int SkypeToMirandaLoginError(CAccount::LOGOUTREASON logoutReason);

	static char *RemoveHtml(const char *data);

	static int SkypeToMirandaStatus(CContact::AVAILABILITY availability);
	static CContact::AVAILABILITY MirandaToSkypeStatus(int status);

	static bool FileExists(wchar_t *path);

	static void ShowNotification(const wchar_t *message, int flags = 0, HANDLE hContact = NULL);
	static void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, HANDLE hContact = NULL);

	// languages
	static std::map<std::wstring, std::wstring> languages;

	// instances
	static LIST<CSkypeProto> instanceList;
	static int CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2);

	//
	int SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam);
	int SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam);

	void	CreateServiceObj(const char* szService, SkypeServiceFunc serviceProc);
	void	CreateServiceObjParam(const char* szService, SkypeServiceFunc serviceProc, LPARAM lParam);

	HANDLE	CreateEvent(const char* szService);
	
	DWORD	SendBroadcastAsync(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam, size_t paramSize = 0);

	void	ForkThread(SkypeThreadFunc, void*);
	HANDLE	ForkThreadEx(SkypeThreadFunc, void*, UINT* threadID = NULL);

	// netlib
	HANDLE	hNetLibUser;
	
	void	InitNetLib();
	void	UninitNetLib();

	void	Log(const wchar_t *fmt, ...);

	// services
	void InitInstanceServiceList();

	// hooks
	HANDLE HookEvent(const char*, SkypeEventFunc);

	void InitInstanceHookList();

	// icons
	static _tag_iconList IconList[];
	static HANDLE GetIconHandle(const char *name);

	// menus
	HGENMENU m_hMenuRoot;
	static HANDLE hChooserMenu;
	static HANDLE contactMenuItems[CMI_TEMS_COUNT];
	static HANDLE contactMenuServices[CMI_TEMS_COUNT];

	virtual	int __cdecl RequestAuth(WPARAM, LPARAM);
	virtual	int __cdecl GrantAuth(WPARAM, LPARAM);
	virtual	int __cdecl RevokeAuth(WPARAM, LPARAM);
	
	static void EnableMenuItem(HANDLE hMenuItem, BOOL bEnable);

	static INT_PTR MenuChooseService(WPARAM wParam, LPARAM lParam);

	INT_PTR __cdecl InviteCommand(WPARAM, LPARAM);

	static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int OnPrebuildContactMenu(WPARAM wParam, LPARAM);

	// database
	bool IsMessageInDB(HANDLE hContact, DWORD timestamp, SEBinary &guid, int flag = 0);

	HANDLE AddDBEvent(HANDLE hContact, WORD type, DWORD time, DWORD flags = 0, DWORD cbBlob = 0, PBYTE pBlob = 0);
	void RaiseMessageSentEvent(
		HANDLE hContact,
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
	static INT_PTR CALLBACK InviteToChatProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static INT_PTR CALLBACK SkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK PersonalSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ContactSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK HomeSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	// skype runtime
	char *LoadKeyPair();
	int StartSkypeRuntime(const wchar_t *profileName);
	void CSkypeProto::StopSkypeRuntime();

	// events
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnContactDeleted(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnSrmmWindowOpen(WPARAM, LPARAM);
	int __cdecl OnUserInfoInit(WPARAM, LPARAM);
	INT_PTR __cdecl OnAccountManagerInit(WPARAM wParam, LPARAM lParam);

	int __cdecl OnMessagePreCreate(WPARAM, LPARAM);
	int __cdecl OnTabSRMMButtonPressed(WPARAM, LPARAM);
};
