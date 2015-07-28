/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_PROTO_H_
#define _SKYPE_PROTO_H_

typedef void(CSkypeProto::*SkypeResponseCallback)(const NETLIBHTTPREQUEST *response);
typedef void(CSkypeProto::*SkypeResponseWithArgCallback)(const NETLIBHTTPREQUEST *response, void *arg);

struct CSkypeProto : public PROTO < CSkypeProto >
{
	friend CSkypeGCCreateDlg;

public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CSkypeProto(const char *protoName, const wchar_t *userName);
	~CSkypeProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);
	virtual  int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage);
	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR* szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);
	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SetStatus(int iNewStatus);
	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);
	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// accounts
	static CSkypeProto* InitAccount(const char *protoName, const TCHAR *userName);
	static int          UninitAccount(CSkypeProto *proto);

	// icons
	static void InitIcons();
	static void UninitIcons();

	// menus
	static void InitMenus();
	static void UninitMenus();

	//popups
	void InitPopups();

	// languages
	static void InitLanguages();

	// events
	static int	OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnDbEventRead(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	//search
	void __cdecl SearchBasicThread(void* id);

	////////////////////////////////////////////
	static INT_PTR EventGetIcon(WPARAM wParam, LPARAM lParam);
	static INT_PTR GetEventText(WPARAM, LPARAM lParam);

private:

	static UINT_PTR m_timer, m_trouterTimer;

	//---Accounts
	static LIST<CSkypeProto> CSkypeProto::Accounts; 
	static int CompareAccounts(const CSkypeProto *p1, const CSkypeProto *p2);
	//---/

	RequestQueue *requestQueue;

	bool isTerminated,
		 HistorySynced;
	std::map<std::string, std::string> cookies;
	static std::map<std::tstring, std::tstring> languages;

	HANDLE m_pollingConnection,
		   m_hPollingThread,
		   m_hTrouterThread,
		   m_TrouterConnection,
		   m_hTrouterEvent,
		   m_hCallHook;

	TRInfo TRouter;

	LIST<void> m_PopupClasses;
	LIST<void> m_OutMessages;
	//dialogs
	LIST<CSkypeInviteDlg> m_InviteDialogs;
	LIST<CSkypeGCCreateDlg> m_GCCreateDialogs;

	//locks
	mir_cs m_InviteDialogsLock;
	mir_cs m_GCCreateDialogsLock;
	mir_cs messageSyncLock;
	static mir_cs accountsLock;
	static mir_cs timerLock;
	

	ptrA m_szServer,
		 m_szRegToken,
		 m_szTokenSecret,
		 m_szEndpointId,
		 m_szSelfSkypeName;

	static CSkypeProto* GetContactAccount(MCONTACT hContact);
	int __cdecl OnAccountLoaded(WPARAM, LPARAM);

	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);

	std::tstring m_tszAvatarFolder;

	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

	int InternalSetAvatar(MCONTACT hContact, const char *szJid, const TCHAR *ptszFileName);

	// requests

	void InitNetwork();
	void UnInitNetwork();

	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, SkypeResponseCallback response);
	void PushRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg);

	void SendRequest(HttpRequest *request);
	void SendRequest(HttpRequest *request, SkypeResponseCallback response);
	void SendRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg);

	// icons
	static IconInfo Icons[];
	static HANDLE GetIconHandle(const char *name);
	static HANDLE Skin_GetIconHandle(const char *name);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	int OnInitStatusMenu();

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// login
	void Login();
	void OnMSLoginFirst(const NETLIBHTTPREQUEST *response);
	void OnMSLoginSecond(const NETLIBHTTPREQUEST *response);
	void OnMSLoginThird(const NETLIBHTTPREQUEST *response);
	void OnMSLoginEnd(const NETLIBHTTPREQUEST *response);
	void OnLoginOAuth(const NETLIBHTTPREQUEST *response);
	void OnLoginSuccess();
	void OnEndpointCreated(const NETLIBHTTPREQUEST *response);
	void SendPresence(bool isLogin = false);
	void OnSubscriptionsCreated(const NETLIBHTTPREQUEST *response);
	void OnCapabilitiesSended(const NETLIBHTTPREQUEST *response);
	void OnStatusChanged(const NETLIBHTTPREQUEST *response);

	//TRouter

	void OnCreateTrouter(const NETLIBHTTPREQUEST *response);
	void OnTrouterPoliciesCreated(const NETLIBHTTPREQUEST *response);
	void OnGetTrouter(const NETLIBHTTPREQUEST *response);
	void OnHealth(const NETLIBHTTPREQUEST *response);
	void OnTrouterEvent(const JSONNode &body, const JSONNode &headers);
	void __cdecl TRouterThread(void*);

	// profile
	void UpdateProfileFirstName(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileLastName(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileDisplayName(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileGender(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileBirthday(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileCountry(const JSONNode &node, MCONTACT hContact = NULL);
	void UpdateProfileState(const JSONNode &node, MCONTACT hContact = NULL);
	void UpdateProfileCity(const JSONNode &node, MCONTACT hContact = NULL);
	void UpdateProfileLanguage(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileHomepage(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileAbout(const JSONNode &node, MCONTACT hContact = NULL);
	void UpdateProfileEmails(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneMobile(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneHome(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneOffice(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileStatusMessage(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileXStatusMessage(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact = NULL);

	void LoadProfile(const NETLIBHTTPREQUEST *response);

	// contacts
	WORD GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, WORD status);
	void SetAllContactsStatus(WORD status);

	void SetAvatarUrl(MCONTACT hContact, CMString &tszUrl);
	void ReloadAvatarInfo(MCONTACT hContact);
	void GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen);

	void OnReceiveAvatar(const NETLIBHTTPREQUEST *response, void *arg);
	void OnSentAvatar(const NETLIBHTTPREQUEST *response);
	void OnSearch(const NETLIBHTTPREQUEST *response);

	MCONTACT FindContact(const char *skypename);
	MCONTACT AddContact(const char *skypename, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void LoadContactsAuth(const NETLIBHTTPREQUEST *response);
	void LoadContactsInfo(const NETLIBHTTPREQUEST *response);
	void LoadContactList(const NETLIBHTTPREQUEST *response);

	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	void OnBlockContact(const NETLIBHTTPREQUEST *response, void *p);
	void OnUnblockContact(const NETLIBHTTPREQUEST *response, void *p);

	// messages

	MEVENT GetMessageFromDb(MCONTACT hContact, const char *messageId, LONGLONG timestamp = 0);
	MEVENT AddDbEvent(WORD type, MCONTACT hContact, DWORD timestamp, DWORD flags, const char *content, const char *uid);
	MEVENT AppendDBEvent(MCONTACT hContact, MEVENT hEvent, const char *szContent, const char *szUid, time_t edit_time);
	int OnReceiveMessage(MCONTACT hContact, const char *szContent, const char *szMessageId, time_t timestamp,  int emoteOffset = 0, bool isRead = false);

	int OnSendMessage(MCONTACT hContact, int flags, const char *message);
	void OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void MarkMessagesRead(MCONTACT hContact, MEVENT hDbEvent);

	void OnPrivateMessageEvent(const JSONNode &node);

	// sync
	void OnGetServerHistory(const NETLIBHTTPREQUEST *response);
	void OnSyncHistory(const NETLIBHTTPREQUEST *response);
	void SyncHistory();

	//chats
	void InitGroupChatModule();
	void CloseAllChatChatSessions();

	MCONTACT FindChatRoom(const char *chatname);
	MCONTACT AddChatRoom(const char *chatname);

	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);

	void StartChatRoom(const TCHAR *tid, const TCHAR *tname);

	void OnLoadChats(const NETLIBHTTPREQUEST *response);

	void OnGetChatInfo(const NETLIBHTTPREQUEST *response, void *p);

	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);
	void OnChatEvent(const JSONNode &node);
	void OnSendChatMessage(const TCHAR *chat_id, const TCHAR * tszMessage);
	char *GetChatUsers(const TCHAR *chat_id);
	bool IsChatContact(const TCHAR *chat_id, const char *id);
	void AddMessageToChat(const TCHAR *chat_id, const TCHAR *from, const char *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading = false);
	void AddChatContact(const TCHAR *tchat_id, const char *id, const char *name, const TCHAR *role, bool isChange = false);
	void RemoveChatContact(const TCHAR *tchat_id, const char *id, const char *name, bool isKick = false, const char *initiator = "");

	void RenameChat(const char *chat_id, const char *name);
	void ChangeChatTopic(const char * chat_id, const char *topic, const char *initiator);

	void SetChatStatus(MCONTACT hContact, int iStatus);

	//polling
	void __cdecl PollingThread(void*);
	void ParsePollData(const JSONNode &data);
	void ProcessEndpointPresenceRes(const JSONNode &node);
	void ProcessUserPresenceRes(const JSONNode &node);
	void ProcessNewMessageRes(const JSONNode &node);
	void ProcessConversationUpdateRes(const JSONNode &node);
	void ProcessThreadUpdateRes(const JSONNode &node);

	// utils
	bool IsOnline();
	bool IsMe(const char *skypeName);

	MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob);
	time_t IsoToUnixTime(const char *stamp);
	char *RemoveHtml(const char *text);
	CMStringA GetStringChunk(const char *haystack, const char *start, const char *end);

	int SkypeToMirandaStatus(const char *status);
	char *MirandaToSkypeStatus(int status);

	void ShowNotification(const TCHAR *message, MCONTACT hContact = NULL);
	void ShowNotification(const TCHAR *caption, const TCHAR *message, MCONTACT hContact = NULL, int type = 0);
	static bool IsFileExists(std::tstring path);

	static LRESULT CALLBACK PopupDlgProcCall(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam);

	CMStringA ParseUrl(const char *url, const char *token);

	void SetSrmmReadStatus(MCONTACT hContact);

	CMStringA UrlToSkypename(const char *url);
	CMStringA GetServerFromUrl(const char *url);

	//---Timers
	void CALLBACK SkypeUnsetTimer(void*);
	void CALLBACK SkypeSetTimer(void*);
	void ProcessTimer();
	static void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
	//---/

	time_t GetLastMessageTime(MCONTACT hContact);
	CMString RunConfirmationCode();
	CMString ChangeTopicForm();
	void CloseDialogs();
	//events
	void InitDBEvents();
	int __cdecl ProcessSrmmEvent(WPARAM, LPARAM);

	//services
	INT_PTR __cdecl OnIncomingCallCLE(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnIncomingCallPP(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl BlockContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl UnblockContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);
	INT_PTR __cdecl GetContactHistory(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl SvcCreateChat(WPARAM, LPARAM);
	INT_PTR __cdecl ParseSkypeUriService(WPARAM, LPARAM lParam);
	static INT_PTR __cdecl GlobalParseSkypeUriService(WPARAM, LPARAM lParam);

	template<INT_PTR(__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CSkypeProto *proto = CSkypeProto::GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

#endif //_SKYPE_PROTO_H_