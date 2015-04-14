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
	friend CSkypePasswordEditor;

public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CSkypeProto(const char *protoName, const wchar_t *userName);
	~CSkypeProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);
	virtual int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage); 
	virtual	int      __cdecl Authorize(MEVENT hDbEvent);
	virtual	int      __cdecl AuthDeny(MEVENT hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre);
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

	// languages
	static void InitLanguages();

	// events
	static int OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnDbEventRead(WPARAM, LPARAM);
	//search
	void __cdecl SearchBasicThread(void* id);

private:
	char *password;
	RequestQueue *requestQueue;
	bool isTerminated;
	std::map<std::string, std::string> cookies;
	std::map<std::string, std::string> RegInfo;
	HANDLE m_pollingConnection, m_hPollingThread;
	static std::map<std::tstring, std::tstring> languages;

	static INT_PTR CALLBACK PasswordEditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// accounts
	static LIST<CSkypeProto> Accounts;
	static int CompareAccounts(const CSkypeProto *p1, const CSkypeProto *p2);

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
	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, SkypeResponseCallback response);
	void PushRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg);

	// icons
	static IconInfo Icons[];
	static HANDLE GetIconHandle(const char *name);
	static HANDLE GetSkinIconHandle(const char *name);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	int OnInitStatusMenu();

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// login
	void OnLoginFirst(const NETLIBHTTPREQUEST *response);
	void OnLoginSecond(const NETLIBHTTPREQUEST *response);
	void OnEndpointCreated(const NETLIBHTTPREQUEST *response);
	void OnSubscriptionsCreated(const NETLIBHTTPREQUEST *response);
	void OnStatusChanged(const NETLIBHTTPREQUEST *response);

	// profile
	void UpdateProfileFirstName(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileLastName(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileDisplayName(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileGender(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileBirthday(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileCountry(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileState(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileCity(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileLanguage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileHomepage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileAbout(JSONNODE *node, MCONTACT hContact = NULL);
	void UpdateProfileEmails(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneMobile(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneHome(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfilePhoneOffice(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileStatusMessage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileXStatusMessage(JSONNODE *root, MCONTACT hContact = NULL);
	void UpdateProfileAvatar(JSONNODE *root, MCONTACT hContact = NULL);

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

	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);
	INT_PTR __cdecl GetContactHistory(WPARAM hContact, LPARAM lParam);

	int __cdecl OnContactDeleted(MCONTACT, LPARAM);

	// messages
	mir_cs messageSyncLock;

	MEVENT GetMessageFromDb(MCONTACT hContact, const char *messageId, LONGLONG timestamp = 0);
	MEVENT AddMessageToDb(MCONTACT hContact, DWORD timestamp, DWORD flags, const char *messageId, char *content, int emoteOffset = 0);

	int OnReceiveMessage(const char *messageId, const char *url, time_t timestamp, char *content, int emoteOffset = 0, bool isRead = false);
	int SaveMessageToDb(MCONTACT hContact, PROTORECVEVENT *pre);

	int OnSendMessage(MCONTACT hContact, int flags, const char *message);
	void OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void MarkMessagesRead(MEVENT hDbEvent);

	void OnPrivateMessageEvent(JSONNODE *node);


	// sync
	void OnGetServerHistory(const NETLIBHTTPREQUEST *response);
	void SyncHistory();
	void OnSyncHistory(const NETLIBHTTPREQUEST *response);

	//chats
	void InitGroupChatModule();
	void CloseAllChatChatSessions();

	MCONTACT FindChatRoom(const char *chatname);
	MCONTACT AddChatRoom(const char *chatname);
	
	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);

	void StartChatRoom(MCONTACT hChatRoom, bool showWindow = false);

	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);

	void OnChatEvent(JSONNODE *node);

	//polling
	void __cdecl PollingThread(void*);
	void ParsePollData(JSONNODE *data);
	void ProcessEndpointPresenceRes(JSONNODE *node);
	void ProcessUserPresenceRes(JSONNODE *node);
	void ProcessNewMessageRes(JSONNODE *node);
	void ProcessConversationUpdateRes(JSONNODE *node);
	void ProcessThreadUpdateRes(JSONNODE *node);

	// utils
	bool IsOnline();
	bool IsMe(const char *skypeName);

	MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob);
	time_t IsoToUnixTime(const TCHAR *stamp);
	char *RemoveHtml(const char *text);
	char *GetStringChunk(const char *haystack, size_t len, const char *start, const char *end);

	int SkypeToMirandaStatus(const char *status);
	char *MirandaToSkypeStatus(int status);

	static void ShowNotification(const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	static void ShowNotification(const TCHAR *caption, const TCHAR *message, int flags = 0, MCONTACT hContact = NULL);
	static bool IsFileExists(std::tstring path);

	char *ChatUrlToName(const char *url);
	char *ContactUrlToName(const char *url);
	char *SelfUrlToName(const char *url);
	char *GetServerFromUrl(const char *url);

	template<INT_PTR(__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CSkypeProto *proto = CSkypeProto::GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

#endif //_SKYPE_PROTO_H_