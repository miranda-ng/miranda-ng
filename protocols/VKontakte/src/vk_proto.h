/*
Copyright (c) 2013-24 Miranda NG team (https://miranda-ng.org)

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

#pragma once

#define PS_CREATECHAT "/CreateNewChat"
#define PS_LOADVKNEWS "/LoadVKNews"
#define PS_WIPENONFRIENDS "/WipeNonFriendContacts"
#define PS_GETSERVERHISTORY "/SyncHistory"
#define PS_GETSERVERHISTORYLAST1DAY "/GetServerHystoryLast1Day"
#define PS_GETSERVERHISTORYLAST3DAY "/GetServerHystoryLast3Day"
#define PS_GETSERVERHISTORYLAST7DAY "/GetServerHystoryLast7Day"
#define PS_GETSERVERHISTORYLAST30DAY "/GetServerHystoryLast30Day"
#define PS_GETSERVERHISTORYLAST90DAY "/GetServerHystoryLast90Day"
#define PS_GETALLSERVERHISTORYFORCONTACT "/GetAllServerHystoryForContact"
#define PS_GETALLSERVERHISTORY "/GetAllServerHystory"
#define PS_VISITPROFILE "/VisitProfile"
#define PS_ADDASFRIEND "/AddAsFriend"
#define PS_DELETEFRIEND "/DeleteFriend"
#define PS_BANUSER "/BanUser"
#define PS_REPORTABUSE "/ReportAbuse"
#define PS_OPENBROADCAST "/OpenBroadcast"
#define PS_SETSTATUSMSG "/SetStatusMsg"
#define PS_WALLPOST "/WallPost"
#define PS_MARKMESSAGESASREAD "/MarkMessagesAsRead"
#define PS_CHATCHANGETOPIC "/ChatChangeTopic"
#define PS_CHATINVITEUSER "/ChatInviteUser"
#define PS_CHATDESTROY "/ChatDestroy"
#define PS_NSEXECMENU "/NSExecMenu"



#define MAXHISTORYMIDSPERONE 100
#define MAX_RETRIES 10
#define MAX_CONTACTS_PER_REQUEST 530

struct CVkProto : public PROTO<CVkProto>
{
	CVkProto(const char*, const wchar_t*);
	~CVkProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = 0) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;
	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	HANDLE   SearchBasic(const wchar_t *id) override;
	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;
	int      SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnBuildProtoMenu() override;
	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	//==== Events ========================================================================

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	void OnOAuthAuthorize(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveAvatar(MHttpResponse*, AsyncHttpRequest*);

	//==== Services ======================================================================

	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetListeningTo(WPARAM, LPARAM);

	//==== Menus ==========================================================================

	INT_PTR __cdecl SvcVisitProfile(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcAddAsFriend(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcWipeNonFriendContacts(WPARAM, LPARAM);
	INT_PTR __cdecl SvcDeleteFriend(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcBanUser(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcReportAbuse(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcOpenBroadcast(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcWallPost(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcMarkMessagesAsRead(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcSetStatusMsg(WPARAM, LPARAM);
	INT_PTR __cdecl SvcChatChangeTopic(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcChatInviteUser(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcChatDestroy(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcNSExecMenu(WPARAM iCommand, LPARAM pHandle);

	//==== History Menus ==================================================================

	template <unsigned short Days>
	INT_PTR __cdecl SvcGetServerHistoryLastNDay(WPARAM hContact, LPARAM)
	{
		debugLogA("CVkProto::SvcGetServerHistoryLast%dDay", Days);
		if (!IsOnline())
			return 0;
		GetServerHistoryLastNDay(hContact, Days);
		return 1;
	}

	INT_PTR __cdecl SvcGetAllServerHistoryForContact(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcGetAllServerHistory(WPARAM, LPARAM);
	INT_PTR __cdecl SvcEmptyServerHistory(WPARAM, LPARAM);

	//=====================================================================================

	void CreateNewChat(LPCSTR uids, LPCWSTR pwszTitle);
	__forceinline bool IsOnline() const { return m_bOnline; }
	bool CheckHealthThreads();
	void OnTimerTic();
	void ClearAccessToken();
	wchar_t* GetUserStoredPassword(void);
	void ShowCaptchaInBrowser(HBITMAP hBitmap);

	static mir_cs m_csTimer;
	static UINT_PTR m_Timer;
	CVKOptions m_vkOptions;

	VKUserID_t ReadVKUserIDFromString(MCONTACT hContact);
	VKUserID_t ReadVKUserID(MCONTACT hContact);
	void WriteVKUserID(MCONTACT hContact, VKUserID_t iUserId);

	int ForwardMsg(MCONTACT hContact, std::vector<MEVENT> &vForvardEvents, const char* szMsg);
	uint8_t GetContactType(MCONTACT hContact);

private:

	VKPeerType GetVKPeerType(VKUserID_t iPeerId);

	friend struct AsyncHttpRequest;
	friend class CVkUserItem;

	//==== Enums =========================================================================

	enum CLMenuIndexes {
		CMI_CHATDESTROY,	
		CMI_CHATCHANGETOPIC,
		CMI_CHATINVITEUSER,
		CMI_VISITPROFILE,
		CMI_MARKMESSAGESASREAD,
		CMI_WALLPOST,
		CMI_ADDASFRIEND,
		CMI_DELETEFRIEND,
		CMI_BANUSER,
		CMI_REPORTABUSE,
		CMI_OPENBROADCAST,
		CMI_LOADVKNEWS,
		CMI_GETSERVERHISTORY,
		CMI_COUNT
	};

	enum CLMenuHistoruIndexes {
		CHMI_GETSERVERHISTORY,
		CHMI_GETSERVERHISTORYLAST1DAY,
		CHMI_GETSERVERHISTORYLAST3DAY,
		CHMI_GETSERVERHISTORYLAST7DAY,
		CHMI_GETSERVERHISTORYLAST30DAY,
		CHMI_GETSERVERHISTORYLAST90DAY,
		CHMI_GETALLSERVERHISTORYFORCONTACT,
		CHMI_COUNT
	};

	enum ProtoMenuIndexes {
		PMI_CREATECHAT,
		PMI_SETSTATUSMSG,
		PMI_WALLPOST,
		PMI_LOADVKNEWS,
		PMI_GETALLSERVERHISTORY,
		PMI_WIPENONFRIENDS,
		PMI_VISITPROFILE,
		PMI_COUNT
	};

	enum NewStoryMenuIndexes {
		NSMI_FORWARD,
		NSMI_RELOADMESSAGE,
		NSMI_COUNT
	};

	//====================================================================================

	bool
		m_bPrevError,
		m_bOnline,
		m_bNeedSendOnline,
		m_bSetBroadcast,
		m_bNotifyForEndLoadingHistory,
		m_bNotifyForEndLoadingHistoryAllContact,
		m_bTerminated,
		m_bErr404Return;

	VKUserID_t m_iMyUserId;

	ptrA
		m_szPollingServer,
		m_szPollingKey,
		m_szPollingTs,
		m_szAccessToken;

	HNETLIBCONN
		m_hPollingConn,
		m_hAPIConnection;

	HANDLE
		m_hPollingThread,
		m_hPopupClassError,
		m_hPopupClassNotification,
		m_hEvRequestsQueue,
		m_hWorkerThread;

	long m_iMsgId;

	mir_cs
		m_csChatTyping,
		m_csLoadHistoryTask,
		m_csRequestsQueue,
		m_csSetStatus,
		m_csWorkThreadTimer,
		m_csPoolThreadTimer,
		m_csAPIConnection,
		m_csPoolingConnection;

	time_t
		m_tWorkThreadTimer,
		m_tPoolThreadTimer;

	int m_iLoadHistoryTask;

	OBJLIST<CVkChatInfo> m_chats;
	OBJLIST<CVKChatContactTypingParam> m_ChatsTyping;
	OBJLIST<CVkCookie> m_cookies;
	LIST<AsyncHttpRequest> m_arRequestsQueue;

	CMStringA m_szPrevUrl;

	HGENMENU
		m_hContactMenuItems[CMI_COUNT],
		m_hContactHistoryMenuItems[CHMI_COUNT],
		m_hProtoMenuItems[PMI_COUNT],
		m_hNewStoryMenuItems[NSMI_COUNT];


	void InitSmileys();

	//==== Menus =========================================================================

	void InitMenus();
	void UnInitMenus();
	int __cdecl OnPreBuildContactMenu(WPARAM hContact, LPARAM);
	int __cdecl OnPrebuildNSMenu(WPARAM hContact, LPARAM);

	//==== PopUps ========================================================================

	void InitPopups(void);
	void MsgPopup(MCONTACT hContact, const wchar_t *wszMsg, const wchar_t *wszTitle, bool err = false);
	void MsgPopup(const wchar_t *wszMsg, const wchar_t *wszTitle, bool err = false);

	void InitDBCustomEvents();

	//==== Hooks =========================================================================

	int __cdecl OnProcessSrmmEvent(WPARAM, LPARAM);
	int __cdecl OnDbSettingChanged(WPARAM, LPARAM);

	//==== Search ========================================================================

	void FreeProtoShearchStruct(PROTOSEARCHBYNAME *pParam);
	void OnSearch(MHttpResponse*, AsyncHttpRequest*);
	void OnSearchByMail(MHttpResponse*, AsyncHttpRequest*);

	//==== Files Download ================================================================

	CMStringW CVkProto::GetVkFileItem(CMStringW& wszUrl, MCONTACT hContact, VKMessageID_t iMessageId);
	void OnGetVkFileItem(MHttpResponse* reply, AsyncHttpRequest* pReq);

	//==== Files Upload ==================================================================

	void SendFileFailed(CVkFileUploadParam *fup, int ErrorCode);
	void OnReciveUploadServer(MHttpResponse*, AsyncHttpRequest*);
	void OnReciveUpload(MHttpResponse*, AsyncHttpRequest*);
	void OnReciveUploadFile(MHttpResponse*, AsyncHttpRequest*);

	//==== Feed ==========================================================================

	void AddFeedSpecialUser();
	void AddFeedEvent(CVKNewsItem& vkNewsItem);
	void AddCListEvent(bool bNews);
	CVkUserInfo* GetVkUserInfo(VKUserID_t iUserId, OBJLIST<CVkUserInfo> &vkUsers);
	void CreateVkUserInfoList(OBJLIST<CVkUserInfo> &vkUsers, const JSONNode &jnResponse);
	CVKNewsItem* GetVkNewsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers, bool bIsRepost = false);
	CVKNewsItem* GetVkGroupInvates(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers);
	CVKNewsItem* GetVkNotificationsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers);
	void OnFriendAccepted(const JSONNode &jnFeedback);
	CMStringW GetVkFeedback(const JSONNode &jnFeedback, VKObjType vkFeedbackType, OBJLIST<CVkUserInfo> &vkUsers, CVkUserInfo *vkUser);
	CVKNewsItem* GetVkParent(const JSONNode &jnParent, VKObjType vkParentType, LPCWSTR pwszReplyText = nullptr, LPCWSTR pwszReplyLink = nullptr);
	void RetrieveUnreadNews(time_t tLastNewsTime);
	void OnReceiveUnreadNews(MHttpResponse*, AsyncHttpRequest*);
	void RetrieveUnreadNotifications(time_t tLastNotificationsTime);
	bool FilterNotification(CVKNewsItem *vkNotificationItem, bool& bIsCommented);
	void NotificationMarkAsViewed();
	void OnReceiveUnreadNotifications(MHttpResponse*, AsyncHttpRequest*);
	void RetrieveUnreadEvents();
	void NewsClearHistory();
	INT_PTR __cdecl SvcLoadVKNews(WPARAM, LPARAM);

	//====================================================================================

	void SetServerStatus(int);
	void RetrieveUsersInfo(bool bFlag = false, bool bRepeat = false);
	void RetrieveStatusMsg(const CMStringW &StatusMsg);
	void RetrieveStatusMusic(const CMStringW &StatusMsg);
	void OnReceiveStatus(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveStatusMsg(MHttpResponse*, AsyncHttpRequest*);
	MCONTACT SetContactInfo(const JSONNode &jnItem, bool bFlag = false, VKContactType vkContactType = VKContactType::vkContactNormal);
	void TrackVisitor();
	void RetrieveMyInfo(void);
	void OnReceiveMyInfo(MHttpResponse*, AsyncHttpRequest*);
	void RetrieveUserInfo(VKUserID_t iUserId);
	void RetrieveGroupInfo(VKUserID_t iGroupID);
	void RetrieveGroupInfo(CMStringA & groupIDs);
	void OnReceiveUserInfo(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveGroupInfo(MHttpResponse * reply, AsyncHttpRequest * pReq);
	void RetrieveFriends(bool bCleanNonFriendContacts = false);
	void OnReceiveFriends(MHttpResponse*, AsyncHttpRequest*);
	void MarkMessagesRead(const MCONTACT hContact);
	void RetrieveMessagesByIds(const CMStringA &szMids);
	void RetrieveUnreadMessages();
	void OnReceiveMessages(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveDlgs(MHttpResponse*, AsyncHttpRequest*);
	void OnSendMessage(MHttpResponse*, AsyncHttpRequest*);
	void WallPost(MCONTACT hContact, wchar_t *pwszMsg, wchar_t *pwszUrl, bool bFriendsOnly);
	void GetServerHistoryLastNDay(MCONTACT hContact, int iNDay);
	void GetServerHistory(MCONTACT hContact, int iOffset, int iCount, time_t tTime, VKMessageID_t iLastMsgId, bool bOnce = false);
	void OnReceiveHistoryMessages(MHttpResponse *reply, AsyncHttpRequest *pReq);
	void GetHistoryDlg(MCONTACT hContact, VKMessageID_t iLastMsg);
	void RetrievePollingInfo();
	void OnReceivePollingInfo(MHttpResponse*, AsyncHttpRequest*);
	void __cdecl PollingThread(void*);
	int PollServer();
	void PollUpdates(const JSONNode&);
	void OnReceiveAuthRequest(MHttpResponse*, AsyncHttpRequest*);
	void OnReceiveDeleteFriend(MHttpResponse*, AsyncHttpRequest*);

	//==== Misc ==========================================================================

	void SetAllContactStatuses(int iStatus);
	MCONTACT FindTempUser(VKUserID_t iUserid, int iWait = 0);
	MCONTACT FindUser(VKUserID_t iUserId, bool bCreate = false);
	MCONTACT FindChat(VKUserID_t iUserId);
	JSONNode& CheckJsonResponse(AsyncHttpRequest *pReq, MHttpResponse *reply, JSONNode &root);
	bool CheckJsonResult(AsyncHttpRequest *pReq, const JSONNode &Node);
	void OnReceiveSmth(MHttpResponse*, AsyncHttpRequest*);
	bool AutoFillForm(char*, CMStringA&, CMStringA&);
	CMStringW RunConfirmationCode(LPCWSTR pwszTitle);
	CMStringW RunRenameNick(LPCWSTR pwszOldName);
	void GrabCookies(MHttpResponse *nhr, CMStringA szDefDomain = "");
	void ApplyCookies(AsyncHttpRequest*);
	bool IsAuthContactLater(MCONTACT hContact);
	bool AddAuthContactLater(MCONTACT hContact);
	void __cdecl DBAddAuthRequestThread(void *p);
	void DBAddAuthRequest(const MCONTACT hContact, bool bAdded = false);
	MCONTACT MContactFromDbEvent(MEVENT hDbEvent);
	void SetMirVer(MCONTACT hContact, int iPlatform);
	void __cdecl ContactTypingThread(void *p);
	int IsHystoryMessageExist(MCONTACT hContact);
	void SetSrmmReadStatus(MCONTACT hContact);
	void MarkRemoteRead(MCONTACT hContact, VKMessageID_t iMessageId = 0);
	void MarkDialogAsRead(MCONTACT hContact);
	void CheckUpdate();
	bool IsGroupUser(MCONTACT hContact);
	char* GetStickerId(const char *szMsg, int& iStickerid);
	CMStringA GetAttachmentsFromMessage(const char * szMsg);
	CMStringW SpanVKNotificationType(CMStringW& wszType, VKObjType& vkFeedback, VKObjType& vkParent);
	CMStringW GetVkPhotoItem(const JSONNode &jnPhoto, BBCSupport iBBC, MCONTACT hContact, VKMessageID_t iMessageId);
	CMStringW GetVkPhotoForVideoItem(const JSONNode& jnPhoto, MCONTACT hContact, VKMessageID_t iMessageId);
	CMStringW SetBBCString(LPCWSTR wszString, BBCSupport iBBC, VKBBCType bbcType, LPCWSTR wszAddString = nullptr);
	CMStringW& ClearFormatNick(CMStringW& wszText);
	CMStringW GetAttachmentDescr(const JSONNode &jnAttachments, BBCSupport iBBC, MCONTACT hContact, VKMessageID_t iMessageId);
	CMStringW GetFwdMessage(const JSONNode& jnMsg, const JSONNode& jnFUsers, OBJLIST<CVkUserInfo>& vkUsers, BBCSupport iBBC);
	CMStringW GetFwdMessages(const JSONNode &jnMessages, const JSONNode &jnFUsers, BBCSupport iBBC);
	void SetInvisible(MCONTACT hContact);
	CMStringW RemoveBBC(CMStringW& wszSrc);
	void AddVkDeactivateEvent(MCONTACT hContact, CMStringW & wszType);
	MEVENT GetMessageFromDb(VKMessageID_t iMessageId, time_t& tTimeStamp, CMStringW& wszMsg);
	MEVENT GetMessageFromDb(const char * szMessageId, time_t& tTimeStamp, CMStringW& wszMsg);
	int DeleteContact(MCONTACT hContact);
	bool IsMessageExist(VKMessageID_t iMessageId, VKMesType vkType = vkALL);
	CMStringW UserProfileUrl(VKUserID_t iUserId);
	void InitQueue();
	void UninitQueue();
	bool ExecuteRequest(AsyncHttpRequest*);
	void __cdecl WorkerThread(void*);
	AsyncHttpRequest* Push(MHttpRequest *pReq, int iTimeout = 10000);
	bool RunCaptchaForm(LPCSTR szUrl, CMStringA&);
	bool ApplyCaptcha(AsyncHttpRequest *pReq, const JSONNode&);
	void ConnectionFailed(int iReason);
	void OnLoggedIn();
	void ClosePollingConnection(bool bShutdown = false);
	void CloseAPIConnection(bool bShutdown = false);
	void OnLoggedOut();
	void ShutdownSession();
	void SetAvatarUrl(MCONTACT hContact, CMStringW &wszUrl);
	void GetAvatarFileName(MCONTACT hContact, wchar_t *pwszDest, size_t cbLen);
	void ReloadAvatarInfo(MCONTACT hContact);
	void __cdecl ChatContactTypingThread(void *p);

	void StopChatContactTyping(VKUserID_t iChatId, VKUserID_t iUserId);

	void OnCreateNewChat(MHttpResponse*, AsyncHttpRequest*);
	CVkChatInfo* AppendConversationChat(VKUserID_t iChatId, const JSONNode& jnItem);
	void SetChatTitle(CVkChatInfo *cc, LPCWSTR wszTopic);
	void AppendChatConversationMessage(VKUserID_t iChatId, const JSONNode& jnMsg, const JSONNode& jnFUsers, bool bIsHistory);
	void AppendChatMessage(CVkChatInfo* vkChatInfo, VKMessageID_t iMessageId, VKMessageID_t iReplyMsgId, VKUserID_t iUserId, time_t tMsgTime, LPCWSTR pwszBody, bool bIsHistory, bool bIsRead, bool bIsAction = false);
	void RetrieveChatInfo(CVkChatInfo*);
	void OnReceiveChatInfo(MHttpResponse*, AsyncHttpRequest*);	
	void OnChatLeave(MHttpResponse*, AsyncHttpRequest *);
	void OnChatDestroy(MHttpResponse*, AsyncHttpRequest*);
	int __cdecl OnChatEvent(WPARAM, LPARAM);
	int __cdecl OnGcMenuHook(WPARAM, LPARAM);
	void KickFromChat(VKUserID_t iChatId, VKUserID_t iUserId, const JSONNode &jnMsg, const JSONNode &jnFUsers);
	void LeaveChat(VKUserID_t iChatId, bool bCloseWindow = true, bool bDeleteChat = false);
	INT_PTR __cdecl OnLeaveChat(WPARAM, LPARAM);
	INT_PTR __cdecl OnJoinChat(WPARAM, LPARAM);
	void LogMenuHook(CVkChatInfo*, GCHOOK*);
	void NickMenuHook(CVkChatInfo*, GCHOOK*);
	LPTSTR ChangeChatTopic(CVkChatInfo*);
	void SetChatStatus(MCONTACT hContact, int iStatus);
	CVkChatInfo* GetChatById(LPCWSTR pwszId);
	CVkChatInfo* GetChatByContact(MCONTACT hContact);
	INT_PTR __cdecl SvcCreateChat(WPARAM, LPARAM);
	void __cdecl GetAwayMsgThread(void* p);
	
	void CVkProto::WriteQSWord(MCONTACT hContact, const char* szParam, uint64_t uValue);
	uint64_t CVkProto::ReadQSWord(MCONTACT hContact, const char* szParam, uint64_t uDefaultValue = 0);

	bool bIint64IDCompatibility;
};

struct CMPlugin : public ACCPROTOPLUGIN<CVkProto>
{
	CMPlugin();

	bool hasMessageState = false, hasNewStory = false;

	int Load() override;
};
