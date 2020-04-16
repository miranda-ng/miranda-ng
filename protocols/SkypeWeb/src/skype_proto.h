/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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
	friend CSkypeOptionsMain;
	friend CSkypeGCCreateDlg;

	class CSkypeProtoImpl
	{
		friend struct CSkypeProto;
		CSkypeProto &m_proto;

		CTimer m_heartBeat;
		void OnHeartBeat(CTimer *)
		{
			m_proto.ProcessTimer();
		}

		CSkypeProtoImpl(CSkypeProto &pro) :
			m_proto(pro),
			m_heartBeat(Miranda_GetSystemWindow(), UINT_PTR(this) + 1)
		{
			m_heartBeat.OnEvent = Callback(this, &CSkypeProtoImpl::OnHeartBeat);
		}
	} m_impl;

public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CSkypeProto(const char *protoName, const wchar_t *userName);
	~CSkypeProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;
	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t* szMessage) override;
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
	int      AuthRecv(MCONTACT hContact, PROTORECVEVENT*) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;
	HANDLE   SearchBasic(const wchar_t* id) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;
	int      RecvContacts(MCONTACT hContact, PROTORECVEVENT*) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int m_iStatus, const wchar_t *msg) override;

	void     OnBuildProtoMenu(void) override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	// icons
	static void InitIcons();

	// menus
	static void InitMenus();

	//popups
	void InitPopups();
	void UninitPopups();

	// languages
	static void InitLanguages();

	// events
	static int	OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnDbEventRead(WPARAM, LPARAM);

	// search
	void __cdecl SearchBasicThread(void* id);

	////////////////////////////////////////////
	static INT_PTR EventGetIcon(WPARAM wParam, LPARAM lParam);
	static INT_PTR GetEventText(WPARAM, LPARAM lParam);

	CSkypeOptions m_opts;

	int m_iPollingId;
	ptrA m_szApiToken, m_szToken, m_szId, m_szServer;
	CMStringA m_szSkypename, m_szMyname;

	__forceinline CMStringA getId(MCONTACT hContact) {
		return getMStringA(hContact, SKYPE_SETTINGS_ID);
	}

private:
	class RequestQueue *requestQueue;

	bool m_bHistorySynced;

	std::map<std::string, std::string> cookies;
	static std::map<std::wstring, std::wstring> languages;

	HNETLIBCONN m_TrouterConnection;
	HANDLE m_hPollingThread, m_hTrouterThread;

	TRInfo TRouter;

	LIST<void> m_PopupClasses;
	LIST<void> m_OutMessages;

	// dialogs
	LIST<CSkypeInviteDlg> m_InviteDialogs;
	LIST<CSkypeGCCreateDlg> m_GCCreateDialogs;

	// locks
	mir_cs m_lckOutMessagesList;
	mir_cs m_InviteDialogsLock;
	mir_cs m_GCCreateDialogsLock;
	mir_cs messageSyncLock;
	mir_cs m_StatusLock;
	mir_cs m_AppendMessageLock;

	bool m_bThreadsTerminated;

	EventHandle m_hPollingEvent;
	EventHandle m_hTrouterEvent;

	EventHandle m_hTrouterHealthEvent;

	INT_PTR __cdecl OnAccountManagerInit(WPARAM, LPARAM);

	std::wstring m_tszAvatarFolder;

	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

	// requests

	void InitNetwork();
	void UnInitNetwork();
	void ShutdownConnections();

	void PushRequest(HttpRequest *request);
	void PushRequest(HttpRequest *request, SkypeResponseCallback response);
	void PushRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg);

	template<typename F>
	void PushRequest(HttpRequest *request, F callback)
	{
		SkypeResponseDelegateBase *delegate = new SkypeResponseDelegateLambda<F>(this, callback);
		requestQueue->Push(request, SkypeHttpResponse, delegate);
	}

	void SendRequest(HttpRequest *request);
	void SendRequest(HttpRequest *request, SkypeResponseCallback response);
	void SendRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg);

	template<typename F>
	void SendRequest(HttpRequest *request, F callback)
	{
		SkypeResponseDelegateBase *delegate = new SkypeResponseDelegateLambda<F>(this, response);
		requestQueue->Send(request, SkypeHttpResponse, delegate);
	}

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// oauth
	void OnOAuthStart(const NETLIBHTTPREQUEST *response);
	void OnOAuthAuthorize(const NETLIBHTTPREQUEST *response);
	void OnOAuthEnd(const NETLIBHTTPREQUEST *response);

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
	void OnReceiveStatus(const NETLIBHTTPREQUEST *response);

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
	void UpdateProfileXStatusMessage(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact = NULL);

	void LoadProfile(const NETLIBHTTPREQUEST *response, void *arg);


	void __cdecl CSkypeProto::SendFileThread(void *p);
	void OnASMObjectCreated(const NETLIBHTTPREQUEST *response, void *arg);
	void OnASMObjectUploaded(const NETLIBHTTPREQUEST *response, void *arg);

	// contacts
	WORD GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, WORD status);

	void SetAvatarUrl(MCONTACT hContact, CMStringW &tszUrl);
	void ReloadAvatarInfo(MCONTACT hContact);
	void GetAvatarFileName(MCONTACT hContact, wchar_t* pszDest, size_t cbLen);

	void OnReceiveAvatar(const NETLIBHTTPREQUEST *response, void *arg);
	void OnSentAvatar(const NETLIBHTTPREQUEST *response);
	void OnSearch(const NETLIBHTTPREQUEST *response);

	MCONTACT FindContact(const char *skypename);
	MCONTACT AddContact(const char *skypename, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	void LoadContactsAuth(const NETLIBHTTPREQUEST *response);
	void LoadContactList(const NETLIBHTTPREQUEST *response);

	void OnBlockContact(const NETLIBHTTPREQUEST *response, void *p);
	void OnUnblockContact(const NETLIBHTTPREQUEST *response, void *p);

	// messages
	std::map<ULONGLONG, HANDLE> m_mpOutMessagesIds;

	MEVENT GetMessageFromDb(const char *messageId);
	MEVENT AddDbEvent(WORD type, MCONTACT hContact, DWORD timestamp, DWORD flags, const char *content, const char *uid);
	void EditEvent(MCONTACT hContact, MEVENT hEvent, const char *szContent, time_t edit_time);

	int OnSendMessage(MCONTACT hContact, int flags, const char *message);
	void OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void MarkMessagesRead(MCONTACT hContact, MEVENT hDbEvent);

	void OnPrivateMessageEvent(const JSONNode &node);

	void ProcessContactRecv(MCONTACT hContact, time_t timestamp, const char *szContent, const char *szMessageId);

	// sync
	void OnGetServerHistory(const NETLIBHTTPREQUEST *response);
	void OnSyncHistory(const NETLIBHTTPREQUEST *response);

	// chats
	void InitGroupChatModule();

	MCONTACT FindChatRoom(const char *chatname);

	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);
	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);

	void StartChatRoom(const wchar_t *tid, const wchar_t *tname);
	void OnLoadChats(const NETLIBHTTPREQUEST *response);
	void OnGetChatInfo(const NETLIBHTTPREQUEST *response, void *p);

	void OnChatEvent(const JSONNode &node);
	void OnSendChatMessage(const char *chat_id, const wchar_t *tszMessage);
	void AddMessageToChat(const char *chat_id, const char *from, const char *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading = false);
	void AddChatContact(const char *chat_id, const char *id, const char *name, const char *role, bool isChange = false);
	void RemoveChatContact(const char *chat_id, const char *id, const char *name, bool isKick = false, const char *initiator = "");
	char* GetChatContactNick(const char *chat_id, const char *id, const char *name);

	void RenameChat(const char *chat_id, const char *name);
	void ChangeChatTopic(const char * chat_id, const char *topic, const char *initiator);

	void SetChatStatus(MCONTACT hContact, int iStatus);

	// polling
	void __cdecl PollingThread(void*);

	void ParsePollData(const char*);

	void ProcessNewMessage(const JSONNode &node);
	void ProcessUserPresence(const JSONNode &node);
	void ProcessThreadUpdate(const JSONNode &node);
	void ProcessEndpointPresence(const JSONNode &node);
	void ProcessConversationUpdate(const JSONNode &node);

	void RefreshStatuses(void);

	// utils
	template <typename T>
	__inline static void FreeList(const LIST<T> &lst)
	{
		for (auto &it : lst)
			mir_free(it);
	}

	__forceinline bool IsOnline() const
	{	return (m_iStatus > ID_STATUS_OFFLINE);
	}

	__forceinline bool IsMe(const char *str)
	{	return (!mir_strcmpi(str, m_szMyname) || !mir_strcmp(str, ptrA(getStringA("SelfEndpointName"))));
	}

	MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob);
	static time_t IsoToUnixTime(const char *stamp);
	static char *RemoveHtml(const char *text);
	static CMStringA GetStringChunk(const char *haystack, const char *start, const char *end);

	static int SkypeToMirandaStatus(const char *status);
	static const char *MirandaToSkypeStatus(int status);

	void ShowNotification(const wchar_t *message, MCONTACT hContact = NULL);
	void ShowNotification(const wchar_t *caption, const wchar_t *message, MCONTACT hContact = NULL, int type = 0);
	static bool IsFileExists(std::wstring path);

	static LRESULT CALLBACK PopupDlgProcCall(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static CMStringA ParseUrl(const char *url, const char *token);

	static CMStringA UrlToSkypename(const char *url);
	static CMStringA GetServerFromUrl(const char *url);

	void ProcessTimer();

	CMStringW RunConfirmationCode();
	CMStringW ChangeTopicForm();
	void CloseDialogs();
	//events
	void InitDBEvents();

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
		CSkypeProto *proto = CMPlugin::getInstance((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

struct CMPlugin : public ACCPROTOPLUGIN<CSkypeProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif //_SKYPE_PROTO_H_