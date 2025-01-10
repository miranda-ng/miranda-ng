/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

struct CSkypeTransfer
{
	CMStringA docId, fileName, fileType, url;
	int iFileSize = 0, iWidth = -1, iHeight = -1;
};

struct CSkypeProto : public PROTO <CSkypeProto>
{
	friend class CSkypeOptionsMain;
	friend class CSkypeGCCreateDlg;
	friend class CSkypeInviteDlg;
	friend class CMoodDialog;

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
	int      AuthRecv(MCONTACT hContact, DB::EventInfo &dbei) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;
	HANDLE   SearchBasic(const wchar_t* id) override;
	int      SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;
	int      RecvContacts(MCONTACT hContact, DB::EventInfo &dbei) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;

	void     OnBuildProtoMenu(void) override;
	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;
	void     OnReceiveOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob) override;
	void     OnShutdown() override;

	// icons
	static void InitIcons();

	// menus
	static void InitMenus();

	// popups
	void InitPopups();
	void UninitPopups();

	// search
	void __cdecl SearchBasicThread(void *param);

	//////////////////////////////////////////////////////////////////////////////////////
	// services

	static INT_PTR __cdecl SvcEventGetIcon(WPARAM, LPARAM);
	static INT_PTR __cdecl SvcGetEventText(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// settings

	CMOption<bool> m_bAutoHistorySync;
	CMOption<bool> m_bUseBBCodes;
	CMOption<bool> m_bUseServerTime; // hidden setting!

	CMOption<bool> m_bUseHostnameAsPlace;
	CMOption<wchar_t*> m_wstrPlace;

	CMOption<wchar_t*> m_wstrCListGroup;

	CMOption<uint8_t> m_iMood;
	CMOption<wchar_t*> m_wstrMoodMessage, m_wstrMoodEmoji;

	//////////////////////////////////////////////////////////////////////////////////////
	// other data

	int m_iPollingId;
	ptrA m_szApiToken, m_szToken, m_szId, m_szOwnSkypeId;
	CMStringA m_szSkypename, m_szMyname;
	MCONTACT m_hMyContact;

	__forceinline CMStringA getId(MCONTACT hContact) {
		return getMStringA(hContact, SKYPE_SETTINGS_ID);
	}

	void OnSearch(MHttpResponse *response, AsyncHttpRequest *pRequest);

	// login
	void OnLoginOAuth(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnSubscriptionsCreated(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnCapabilitiesSended(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnStatusChanged(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnEndpointCreated(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnEndpointDeleted(MHttpResponse *response, AsyncHttpRequest *pRequest);

	// oauth
	void OnOAuthStart(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnOAuthConfirm(MHttpResponse* response, AsyncHttpRequest* pRequest);
	void OnOAuthAuthorize(MHttpResponse* response, AsyncHttpRequest* pRequest);
	void OnOAuthEnd(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnASMObjectCreated(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnASMObjectUploaded(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void LoadContactsAuth(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void LoadContactList(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnBlockContact(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnUnblockContact(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnReceiveAvatar(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnSentAvatar(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnMessageSent(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnGetServerHistory(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnSyncConversations(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void OnGetChatInfo(MHttpResponse *response, AsyncHttpRequest *pRequest);
	void OnGetChatMembers(MHttpResponse *response, AsyncHttpRequest *pRequest);

	void CheckConvert(void);
	bool CheckOauth(const char *szResponse);
	
	void LoadProfile(MHttpResponse *response, AsyncHttpRequest *pRequest);

	static INT_PTR __cdecl GlobalParseSkypeUriService(WPARAM, LPARAM lParam);

private:
	bool m_bHistorySynced;

	static std::map<std::wstring, std::wstring> languages;

	LIST<void> m_PopupClasses;
	LIST<void> m_OutMessages;

	// locks
	mir_cs m_lckOutMessagesList;
	mir_cs messageSyncLock;
	mir_cs m_StatusLock;

	HANDLE m_hPollingThread;
	HNETLIBCONN m_hPollingConn;

	// requests
	bool m_isTerminated = true;
	mir_cs m_requestQueueLock;
	LIST<AsyncHttpRequest> m_requests;
	EventHandle m_hRequestQueueEvent;
	HANDLE m_hRequestQueueThread;

	void __cdecl WorkerThread(void *);

	void StartQueue();
	void StopQueue();

	MHttpResponse* DoSend(AsyncHttpRequest *request);

	void Execute(AsyncHttpRequest *request);
	void PushRequest(AsyncHttpRequest *request);

	// avatars
	void SetAvatarUrl(MCONTACT hContact, const CMStringW &tszUrl);
	bool ReceiveAvatar(MCONTACT hContact);
	void ReloadAvatarInfo(MCONTACT hContact);
	void GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen);

	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

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
	void UpdateProfileDisplayName(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileGender(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileBirthday(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileCountry(const JSONNode &node, MCONTACT hContact = NULL);
	void UpdateProfileEmails(const JSONNode &root, MCONTACT hContact = NULL);
	void UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact = NULL);

	// contacts
	uint16_t GetContactStatus(MCONTACT hContact);
	void SetContactStatus(MCONTACT hContact, uint16_t status);

	MCONTACT FindContact(const char *skypeId);
	MCONTACT FindContact(const wchar_t *skypeId);

	MCONTACT AddContact(const char *skypename, const char *nick, bool isTemporary = false);

	MCONTACT GetContactFromAuthEvent(MEVENT hEvent);

	// files
	void SendFile(CFileUploadParam *fup);

	void __cdecl ReceiveFileThread(void *param);

	INT_PTR __cdecl SvcOfflineFile(WPARAM, LPARAM);

	// messages
	std::map<ULONGLONG, HANDLE> m_mpOutMessagesIds;

	int __cdecl OnPreCreateMessage(WPARAM, LPARAM lParam);

	void ProcessContactRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei);
	void ProcessFileRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei);

	// chats
	void InitGroupChatModule();

	int __cdecl OnGroupChatEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGroupChatMenuHook(WPARAM, LPARAM lParam);
	INT_PTR __cdecl OnJoinChatRoom(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnLeaveChatRoom(WPARAM hContact, LPARAM);

	SESSION_INFO* StartChatRoom(const wchar_t *tid, const wchar_t *tname, const char *pszVersion = nullptr);

	bool OnChatEvent(const JSONNode &node);
	wchar_t* GetChatContactNick(MCONTACT hContact, const wchar_t *id, const wchar_t *name = nullptr, bool *isQualified = nullptr);

	bool AddChatContact(SESSION_INFO *si, const wchar_t *id, const wchar_t *role, bool isChange = false);
	void RemoveChatContact(SESSION_INFO *si, const wchar_t *id, bool isKick = false, const wchar_t *initiator = L"");
	void SendChatMessage(SESSION_INFO *si, const wchar_t *tszMessage);

	void KickChatUser(const char *chatId, const char *userId);

	void SetChatStatus(MCONTACT hContact, int iStatus);

	// polling
	void __cdecl PollingThread(void*);

	bool ParseMessage(const JSONNode &node, DB::EventInfo &dbei);
	void ParsePollData(const char*);

	void ProcessNewMessage(const JSONNode &node);
	void ProcessUserPresence(const JSONNode &node);
	void ProcessThreadUpdate(const JSONNode &node);
	void ProcessEndpointPresence(const JSONNode &node);
	void ProcessConversationUpdate(const JSONNode &node);

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

	bool IsMe(const wchar_t *str);
	bool IsMe(const char *str);

	int64_t getLastTime(MCONTACT);
	void setLastTime(MCONTACT, int64_t);

	CMStringW RemoveHtml(const CMStringW &src, bool bCheckSS = false);

	static time_t IsoToUnixTime(const std::string &stamp);

	static int SkypeToMirandaStatus(const char *status);
	static const char *MirandaToSkypeStatus(int status);

	void ShowNotification(const wchar_t *message, MCONTACT hContact = NULL);
	void ShowNotification(const wchar_t *caption, const wchar_t *message, MCONTACT hContact = NULL, int type = 0);
	static bool IsFileExists(std::wstring path);

	static LRESULT CALLBACK PopupDlgProcCall(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void ProcessTimer();

	void SetString(MCONTACT hContact, const char *pszSetting, const JSONNode &node);

	CMStringW ChangeTopicForm();

	// events
	void InitDBEvents();

	// services
	INT_PTR __cdecl BlockContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl UnblockContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnRequestAuth(WPARAM hContact, LPARAM);
	INT_PTR __cdecl OnGrantAuth(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcLoadHistory(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcEmptyHistory(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcCreateChat(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMood(WPARAM, LPARAM);
	INT_PTR __cdecl ParseSkypeUriService(WPARAM, LPARAM lParam);

	template<INT_PTR(__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CSkypeProto *proto = CMPlugin::getInstance((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}
};

typedef CProtoDlgBase<CSkypeProto> CSkypeDlgBase;

#endif //_SKYPE_PROTO_H_