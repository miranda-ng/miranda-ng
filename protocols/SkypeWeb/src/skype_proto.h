/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

struct CSkypeProto : public PROTO <CSkypeProto>
{
	friend class CSkypeOptionsMain;
	friend class CSkypeGCCreateDlg;
	friend class CSkypeInviteDlg;

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

	//////////////////////////////////////////////////////////////////////////////////////
	// services

	static INT_PTR EventGetIcon(WPARAM wParam, LPARAM lParam);
	static INT_PTR GetEventText(WPARAM, LPARAM lParam);

	//////////////////////////////////////////////////////////////////////////////////////
	// settings

	CMOption<bool> bAutoHistorySync;
	CMOption<bool> bMarkAllAsUnread;
	CMOption<bool> bUseBBCodes;
	CMOption<bool> bUseServerTime; // hidden setting!

	CMOption<bool> bUseHostnameAsPlace;
	CMOption<wchar_t*> wstrPlace;

	CMOption<wchar_t*> wstrCListGroup;

	//////////////////////////////////////////////////////////////////////////////////////
	// other data

	int m_iPollingId;
	ptrA m_szApiToken, m_szToken, m_szId;
	CMStringA m_szSkypename, m_szMyname;

	__forceinline CMStringA getId(MCONTACT hContact) {
		return getMStringA(hContact, SKYPE_SETTINGS_ID);
	}

	void OnReceiveAvatar(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnSentAvatar(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnSearch(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	// login
	void OnLoginOAuth(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnSubscriptionsCreated(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnCapabilitiesSended(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnReceiveStatus(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnStatusChanged(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void OnEndpointCreated(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnEndpointDeleted(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	// oauth
	void OnOAuthStart(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnOAuthConfirm(NETLIBHTTPREQUEST* response, AsyncHttpRequest* pRequest);
	void OnOAuthAuthorize(NETLIBHTTPREQUEST* response, AsyncHttpRequest* pRequest);
	void OnOAuthEnd(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void OnASMObjectCreated(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnASMObjectUploaded(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void LoadContactsAuth(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void LoadContactList(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void OnBlockContact(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnUnblockContact(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void OnMessageSent(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void OnGetServerHistory(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnSyncHistory(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	void OnLoadChats(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnGetChatInfo(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);
	void OnReceiveAwayMsg(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

	bool CheckOauth(const char *szResponse);
	void LoadProfile(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest);

private:
	bool m_bHistorySynced;

	std::map<std::string, std::string> cookies;
	static std::map<std::wstring, std::wstring> languages;

	HANDLE m_hPollingThread;

	LIST<void> m_PopupClasses;
	LIST<void> m_OutMessages;

	// locks
	mir_cs m_lckOutMessagesList;
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
	bool m_isTerminated = true;
	mir_cs m_requestQueueLock;
	LIST<AsyncHttpRequest> m_requests;
	EventHandle m_hRequestQueueEvent;
	HANDLE m_hRequestQueueThread;

	void __cdecl WorkerThread(void *);

	void StartQueue();
	void StopQueue();

	NETLIBHTTPREQUEST* DoSend(AsyncHttpRequest *request);

	void Execute(AsyncHttpRequest *request);
	void PushRequest(AsyncHttpRequest *request);

	// menus
	static HGENMENU ContactMenuItems[CMI_MAX];
	int OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam);

	// options
	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM lParam);

	// login
	void Login();
	void OnLoginSuccess();
	void SendPresence();
	
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

	void __cdecl CSkypeProto::SendFileThread(void *p);

	// contacts
	uint16_t GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, uint16_t status);

	void SetAvatarUrl(MCONTACT hContact, CMStringW &tszUrl);
	void ReloadAvatarInfo(MCONTACT hContact);
	void GetAvatarFileName(MCONTACT hContact, wchar_t* pszDest, size_t cbLen);

	MCONTACT FindContact(const char *skypeId);
	MCONTACT FindContact(const wchar_t *skypeId);

	MCONTACT AddContact(const char *skypename, const char *nick, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	// messages
	std::map<ULONGLONG, HANDLE> m_mpOutMessagesIds;

	MEVENT GetMessageFromDb(const char *messageId);
	MEVENT AddDbEvent(uint16_t type, MCONTACT hContact, uint32_t timestamp, uint32_t flags, const CMStringW &content, const CMStringA &msgId);
	void EditEvent(MCONTACT hContact, MEVENT hEvent, const CMStringW &content, time_t edit_time);

	int OnSendMessage(MCONTACT hContact, int flags, const char *message);
	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void MarkMessagesRead(MCONTACT hContact, MEVENT hDbEvent);

	void OnPrivateMessageEvent(const JSONNode &node);

	void ProcessContactRecv(MCONTACT hContact, time_t timestamp, const char *szContent, const char *szMessageId);

	// chats
	void InitGroupChatModule();

	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);
	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);

	SESSION_INFO* StartChatRoom(const wchar_t *tid, const wchar_t *tname);

	void OnChatEvent(const JSONNode &node);
	wchar_t* GetChatContactNick(MCONTACT hContact, const wchar_t *id, const wchar_t *name = nullptr);

	void AddMessageToChat(SESSION_INFO *si, const wchar_t *from, const wchar_t *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading = false);
	void AddChatContact(SESSION_INFO *si, const wchar_t *id, const wchar_t *role, bool isChange = false);
	void RemoveChatContact(SESSION_INFO *si, const wchar_t *id, bool isKick = false, const wchar_t *initiator = L"");
	void SendChatMessage(SESSION_INFO *si, const wchar_t *tszMessage);

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
	void ReadHistoryRest(const char *url);

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

	__forceinline bool IsMe(const wchar_t *str)
	{	return (!mir_wstrcmpi(str, Utf2T(m_szMyname)) || !mir_wstrcmp(str, getMStringW("SelfEndpointName")));
	}

	__forceinline bool IsMe(const char *str)
	{	return (!mir_strcmpi(str, m_szMyname) || !mir_strcmp(str, ptrA(getUStringA("SelfEndpointName"))));
	}

	static time_t IsoToUnixTime(const std::string &stamp);

	static int SkypeToMirandaStatus(const char *status);
	static const char *MirandaToSkypeStatus(int status);

	void ShowNotification(const wchar_t *message, MCONTACT hContact = NULL);
	void ShowNotification(const wchar_t *caption, const wchar_t *message, MCONTACT hContact = NULL, int type = 0);
	static bool IsFileExists(std::wstring path);

	static LRESULT CALLBACK PopupDlgProcCall(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void ProcessTimer();

	CMStringW RunConfirmationCode();
	CMStringW ChangeTopicForm();

	//events
	void InitDBEvents();

	//services
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

typedef CProtoDlgBase<CSkypeProto> CSkypeDlgBase;

#endif //_SKYPE_PROTO_H_