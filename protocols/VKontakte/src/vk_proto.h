/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

#define PS_CREATECHAT "/CreateNewChat"
#define PS_LOADVKNEWS "/LoadVKNews"
#define PS_GETSERVERHISTORY "/SyncHistory"
#define PS_GETSERVERHISTORYLAST1DAY "/GetServerHystoryLast1Day"
#define PS_GETSERVERHISTORYLAST3DAY "/GetServerHystoryLast3Day"
#define PS_GETSERVERHISTORYLAST7DAY "/GetServerHystoryLast7Day"
#define PS_GETSERVERHISTORYLAST30DAY "/GetServerHystoryLast30Day"
#define PS_GETSERVERHISTORYLAST90DAY "/GetServerHystoryLast90Day"
#define PS_GETALLSERVERHISTORY "/GetAllServerHystory"
#define PS_VISITPROFILE "/VisitProfile"
#define PS_ADDASFRIEND "/AddAsFriend"
#define PS_DELETEFRIEND "/DeleteFriend"
#define PS_BANUSER "/BanUser"
#define PS_REPORTABUSE "/ReportAbuse"
#define PS_DESTROYKICKCHAT "/DestroyKickChat"
#define PS_OPENBROADCAST "/OpenBroadcast"
#define PS_SETSTATUSMSG "/SetStatusMsg"
#define PS_WALLPOST "/WallPost"
#define MAXHISTORYMIDSPERONE 200
#define MAX_RETRIES 10

struct CVkProto;
typedef void (CVkProto::*VK_REQUEST_HANDLER)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);

struct AsyncHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	enum RequestPriority { rpLow, rpMedium, rpHigh };

	AsyncHttpRequest();
	AsyncHttpRequest(CVkProto*, int iRequestType, LPCSTR szUrl, bool bSecure, VK_REQUEST_HANDLER pFunc, RequestPriority rpPriority = rpMedium);
	~AsyncHttpRequest();

	void AddHeader(LPCSTR, LPCSTR);
	void Redirect(NETLIBHTTPREQUEST*);

	CMStringA m_szUrl;
	CMStringA m_szParam;
	VK_REQUEST_HANDLER m_pFunc;
	void *pUserInfo;
	int m_iRetry;
	RequestPriority m_priority;
	static ULONG m_reqCount;
	ULONG m_reqNum;
	bool m_bApiReq;
	bool bExpUrlEncode;
	bool bNeedsRestart, bIsMainConn;
};

struct PARAM
{
	LPCSTR szName;
	__forceinline PARAM(LPCSTR _name) : szName(_name) 
	{}
};

struct INT_PARAM : public PARAM
{
	int iValue;
	__forceinline INT_PARAM(LPCSTR _name, int _value) :
		PARAM(_name), iValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT_PARAM&);

struct CHAR_PARAM : public PARAM
{
	LPCSTR szValue;
	__forceinline CHAR_PARAM(LPCSTR _name, LPCSTR _value) :
		PARAM(_name), szValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const CHAR_PARAM&);

struct TCHAR_PARAM : public PARAM
{
	LPCTSTR tszValue;
	__forceinline TCHAR_PARAM(LPCSTR _name, LPCTSTR _value) :
		PARAM(_name), tszValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const TCHAR_PARAM&);

struct CVkSendMsgParam
{
	CVkSendMsgParam(MCONTACT _p1, int _p2 = 0, int _p3 = 0) :
		hContact(_p1),
		iMsgID(_p2),
		iCount(_p3)
	{}

	MCONTACT hContact;
	int iMsgID;
	int iCount;
};

struct CVkChatMessage : public MZeroedObject
{
	CVkChatMessage(int _id) : 
		m_mid(_id),
		m_uid(0),
		m_date(0),
		m_bHistory(false)
	{}

	int m_mid, m_uid, m_date;
	bool m_bHistory;
	ptrT m_tszBody;
};

struct CVkChatUser : public MZeroedObject
{
	CVkChatUser(int _id) : 
		m_uid(_id),
		m_bDel(false), 
		m_bUnknown(false)
	{}

	int m_uid;
	bool m_bDel, m_bUnknown;
	ptrT m_tszNick;
};

struct CVkChatInfo : public MZeroedObject
{
	CVkChatInfo(int _id) :
		m_users(10, NumericKeySortT),
		m_msgs(10, NumericKeySortT),
		m_chatid(_id), 
		m_admin_id(0),
		m_bHistoryRead(0),
		m_hContact(INVALID_CONTACT_ID)
	{}

	int m_chatid, m_admin_id;
	bool m_bHistoryRead;
	ptrT m_tszTopic, m_tszId;
	MCONTACT m_hContact;
	OBJLIST<CVkChatUser> m_users;
	OBJLIST<CVkChatMessage> m_msgs;

	CVkChatUser* GetUserById(LPCTSTR);
};

struct CVkFileUploadParam {
	enum VKFileType {typeInvalid, typeImg, typeAudio, typeDoc, typeNotSupported};
	TCHAR* FileName;
	TCHAR* Desc;
	char* atr;
	char* fname;
	MCONTACT hContact;
	VKFileType filetype;
	int iErrorCode;
	
	CVkFileUploadParam(MCONTACT _hContact, const PROTOCHAR* _desc, PROTOCHAR** _files);
	~CVkFileUploadParam();
	VKFileType GetType();
	__forceinline bool IsAccess() { return ::_taccess(FileName, 0) == 0; }
	__forceinline char* atrName() { return atr; }
	__forceinline char* fileName() { return fname; }
};

struct CVkUserInfo : public MZeroedObject {
	CVkUserInfo(LONG _UserId) : 
		m_UserId(_UserId),
		m_bIsGroup(false)
	{}

	CVkUserInfo(LONG _UserId, bool _bIsGroup, CMString& _tszUserNick, CMString& _tszLink, MCONTACT _hContact = NULL) :
		m_UserId(_UserId),
		m_bIsGroup(_bIsGroup),
		m_tszUserNick(_tszUserNick),
		m_tszLink(_tszLink),
		m_hContact(_hContact)
	{}

	LONG m_UserId;
	MCONTACT m_hContact;
	CMString m_tszUserNick;
	CMString m_tszLink;
	bool m_bIsGroup;
};

enum VKObjType { vkNull, vkPost, vkPhoto, vkVideo, vkComment, vkTopic, vkUsers, vkCopy, vkInvite };

struct CVKNotification {
	TCHAR *ptszType;
	VKObjType vkParent, vkFeedback;
	TCHAR *ptszTranslate;
};

struct CVKNewsItem : public MZeroedObject {
	CVKNewsItem() : 
		tDate(NULL),
		vkUser(NULL),
		bIsGroup(false),
		bIsRepost(false),
		vkFeedbackType(vkNull), 
		vkParentType(vkNull)
	{}
	
	CMString tszId;
	time_t tDate;
	CVkUserInfo *vkUser;
	CMString tszText;
	CMString tszLink;
	CMString tszType;
	VKObjType vkFeedbackType, vkParentType;
	bool bIsGroup;
	bool bIsRepost;
};

enum VKBBCType { vkbbcB, vkbbcI, vkbbcS, vkbbcU, vkbbcUrl, vkbbcSize, vkbbcColor };
enum BBCSupport { bbcNo, bbcBasic, bbcAdvanced };

struct CVKBBCItem {
	VKBBCType vkBBCType;
	BBCSupport vkBBCSettings;
	TCHAR *ptszTempate;
};

struct TFakeAckParams
{
	__inline TFakeAckParams(MCONTACT _hContact, int _msgid) :
		hContact(_hContact), msgid(_msgid)
	{}

	MCONTACT hContact;
	int msgid;
};

struct CVkProto : public PROTO<CVkProto>
{
	CVkProto(const char*, const TCHAR*);
	~CVkProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	int __cdecl Authorize(MEVENT hDbEvent);
	virtual	int __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR *szReason);
	virtual	int __cdecl AuthRequest(MCONTACT hContact, const TCHAR *szMessage);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE __cdecl SearchBasic(const TCHAR *id);
	virtual	HANDLE __cdecl SearchByEmail(const TCHAR *email);
	virtual	HANDLE __cdecl SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName);

	virtual	int __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int __cdecl SendMsg(MCONTACT hContact, int flags, const char *msg);

	virtual	HANDLE __cdecl SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles);

	virtual	int __cdecl SetStatus(int iNewStatus);

	virtual	int __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//==== Events ========================================================================

	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);

	void OnOAuthAuthorize(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveAvatar(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	
	//==== Services ======================================================================

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetListeningTo(WPARAM, LPARAM);

	//==== Menus ==========================================================================

	INT_PTR __cdecl SvcVisitProfile(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcAddAsFriend(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcDeleteFriend(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcBanUser(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcReportAbuse(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcDestroyKickChat(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcOpenBroadcast(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcWallPost(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcSetStatusMsg(WPARAM, LPARAM);

	//==== History Menus ==================================================================

	INT_PTR __cdecl SvcGetServerHistoryLast1Day(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcGetServerHistoryLast3Day(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcGetServerHistoryLast7Day(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcGetServerHistoryLast30Day(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcGetServerHistoryLast90Day(WPARAM hContact, LPARAM);
	INT_PTR __cdecl SvcGetAllServerHistory(WPARAM hContact, LPARAM);
	void InitMenus();
	void UnInitMenus();
	int __cdecl OnPreBuildContactMenu(WPARAM hContact, LPARAM);

	//==== PopUps ========================================================================

	void InitPopups(void);
	void MsgPopup(MCONTACT hContact, const TCHAR *szMsg, const TCHAR *szTitle, bool err = false);

	//==== Hooks =========================================================================

	int __cdecl OnProcessSrmmEvent(WPARAM, LPARAM);
	int __cdecl OnDbEventRead(WPARAM, LPARAM);
	int __cdecl OnDbSettingChanged(WPARAM, LPARAM);

	//==== Search ========================================================================

	void __cdecl SearchBasicThread(void* id);
	void __cdecl SearchByMailThread(void* email);
	void __cdecl SearchThread(void* p);
	void OnSearch(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnSearchByMail(NETLIBHTTPREQUEST *, AsyncHttpRequest *);

	//==== Files Upload ==================================================================

	void __cdecl SendFileThread(void *p);
	void SendFileFiled(CVkFileUploadParam *fup, TCHAR* reason = NULL);
	void OnReciveUploadServer(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReciveUpload(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReciveUploadFile(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	
	//==== Feed ==========================================================================

	void AddFeedSpecialUser();
	void AddFeedEvent(CMString& tszBody, time_t tTime);
	
	CVkUserInfo* GetVkUserInfo(LONG iUserId, OBJLIST<CVkUserInfo> &vkUsers);
	void CreateVkUserInfoList(OBJLIST<CVkUserInfo> &vkUsers, const JSONNode &jnResponse);	
		
	CVKNewsItem* GetVkNewsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers, bool isRepost = false);

	CVKNewsItem* GetVkGroupInvates(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers);
	CVKNewsItem* GetVkNotificationsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers);
	CMString GetVkFeedback(const JSONNode &jnFeedback, VKObjType vkFeedbackType, OBJLIST<CVkUserInfo> &vkUsers, CVkUserInfo *vkUser);
	CVKNewsItem* GetVkParent(const JSONNode &jnParent, VKObjType vkParentType, TCHAR *ptszReplyText = NULL, TCHAR *ptszReplyLink = NULL);
	
	void RetrieveUnreadNews(time_t tLastNewsTime);
	void OnReceiveUnreadNews(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	
	void RetrieveUnreadNotifications(time_t tLastNotificationsTime);
	bool FilterNotification(CVKNewsItem* vkNotificationItem, bool& isCommented);
	void NotificationMarkAsViewed();
	void OnReceiveUnreadNotifications(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void RetrieveUnreadEvents();
	void NewsClearHistory();

	INT_PTR __cdecl SvcLoadVKNews(WPARAM, LPARAM);

	//==== Misc ==========================================================================

	TCHAR* GetUserStoredPassword(void);
	void SetAllContactStatuses(int status);
	
	MCONTACT FindUser(LONG userid, bool bCreate = false);
	MCONTACT FindChat(LONG dwUserid);

	bool CheckMid(LIST<void> &lList, int guid);
		
	JSONNode& CheckJsonResponse(AsyncHttpRequest *pReq, NETLIBHTTPREQUEST *reply, JSONNode &root);	
	bool CheckJsonResult(AsyncHttpRequest *pReq, JSONNode &Node);

	void OnReceiveSmth(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	bool AutoFillForm(char*, CMStringA&, CMStringA&);
	CMString RunConfirmationCode();

	void GrabCookies(NETLIBHTTPREQUEST *nhr);
	void ApplyCookies(AsyncHttpRequest*);

	void __cdecl DBAddAuthRequestThread(void *p);
	void DBAddAuthRequest(const MCONTACT hContact);
	MCONTACT MContactFromDbEvent(MEVENT hDbEvent);

	void SetMirVer(MCONTACT hContact, int platform);

	void __cdecl ContactTypingThread(void *p);
	void SetSrmmReadStatus(MCONTACT hContact);

	char* GetStickerId(const char* Msg, int& stickerid);

	CMString SpanVKNotificationType(CMString& tszType, VKObjType& vkFeedback, VKObjType& vkParent);
	CMString GetVkPhotoItem(const JSONNode &jnPhoto, BBCSupport iBBC);
	CMString SetBBCString(TCHAR *tszString, BBCSupport iBBC, VKBBCType bbcType, TCHAR *tszAddString = NULL);
	CMString& ClearFormatNick(CMString& tszText);

	CMString GetAttachmentDescr(const JSONNode jnAttachments, BBCSupport iBBC = bbcNo);
	CMString GetFwdMessages(const JSONNode &jnMessages, BBCSupport iBBC = bbcNo);

	void SetInvisible(MCONTACT hContact);

	//====================================================================================

	void RetrieveStatusMsg(const CMString &StatusMsg);
	void RetrieveStatusMusic(const CMString &StatusMsg);
	void OnReceiveStatus(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveStatusMsg(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	MCONTACT SetContactInfo(const JSONNode &jnItem, bool flag = false, bool self = false);
	void RetrieveMyInfo(void);
	void OnReceiveMyInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void RetrieveUserInfo(LONG userId);
	void RetrieveUsersInfo(bool flag = false);
	void OnReceiveUserInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void RetrieveFriends();
	void OnReceiveFriends(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void MarkMessagesRead(const CMStringA &mids);
	void MarkMessagesRead(const MCONTACT hContact);

	void RetrieveMessagesByIds(const CMStringA &mids);
	void RetrieveUnreadMessages();
	void OnReceiveMessages(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveDlgs(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnSendMessage(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void WallPost(MCONTACT hContact, TCHAR *ptszMsg, TCHAR *ptszUrl, bool bFriendsOnly);

	void GetServerHistoryLastNDay(MCONTACT hContact, int NDay);
	void GetServerHistory(MCONTACT hContact, int iOffset, int iCount, int iTime, int iLastMsgId, bool once = false);
	void OnReceiveHistoryMessages(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq);
	void GetHistoryDlg(MCONTACT hContact, int iLastMsg);

	void RetrievePollingInfo();
	void OnReceivePollingInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void __cdecl PollingThread(void*);
	int PollServer();
	void PollUpdates(const JSONNode&);
	void OnReceivePolling(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void OnReceiveAuthRequest(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnReceiveDeleteFriend(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void SetServerStatus(int);

	void CreateNewChat(LPCSTR uids, LPCTSTR ptrszTitle);
	void OnCreateNewChat(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	__forceinline bool IsOnline() const { return m_bOnline; }

	__forceinline LPCTSTR getGroup() const { return m_defaultGroup; }
	__forceinline void setGroup(LPCTSTR grp) { m_defaultGroup = mir_tstrdup(grp); }

	static UINT_PTR m_timer;

private:
	friend struct AsyncHttpRequest;

	LIST<AsyncHttpRequest> m_arRequestsQueue;
	mir_cs m_csRequestsQueue;
	HANDLE m_evRequestsQueue;
	HANDLE m_hWorkerThread;
	bool m_bTerminated, m_bServerDelivery;
	CMStringA m_prevUrl;

	enum CLMenuIndexes {
		CMI_VISITPROFILE,
		CMI_WALLPOST,
		CMI_ADDASFRIEND,
		CMI_DELETEFRIEND,
		CMI_BANUSER,
		CMI_REPORTABUSE,
		CMI_DESTROYKICKCHAT,
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
		CHMI_GETALLSERVERHISTORY,
		CHMI_COUNT
	};
	enum ProtoMenuIndexes {
		PMI_CREATECHAT,
		PMI_SETSTATUSMSG,
		PMI_WALLPOST,
		PMI_LOADVKNEWS,
		PMI_VISITPROFILE,
		PMI_COUNT
	};

	HGENMENU g_hContactMenuItems[CMI_COUNT];
	HGENMENU g_hContactHistoryMenuItems[CHMI_COUNT];
	HGENMENU g_hProtoMenuItems[PMI_COUNT];

	struct Cookie
	{
		Cookie(const CMStringA& name, const CMStringA& value, const CMStringA& domain) :
		m_name(name),
		m_value(value),
		m_domain(domain)
		{}

		CMStringA m_name, m_value, m_domain;
	};

	OBJLIST<Cookie> m_cookies;
	
	void InitQueue();
	void UninitQueue();
	void ExecuteRequest(AsyncHttpRequest*);
	void __cdecl WorkerThread(void*);

	AsyncHttpRequest* Push(AsyncHttpRequest*, int iTimeout = 10000);

	bool RunCaptchaForm(LPCSTR szUrl, CMStringA&);
	bool ApplyCaptcha(AsyncHttpRequest *pReq, const JSONNode&);

	void ConnectionFailed(int iReason);
	void OnLoggedIn();
	void OnLoggedOut();
	void ShutdownSession();

	void SetAvatarUrl(MCONTACT hContact, CMString &tszUrl);
	void GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen);
	void ReloadAvatarInfo(MCONTACT hContact);

	void __cdecl SendMsgAck(void *param);

	//============== Options =============================================================
	

	bool	m_prevError,
		m_bOnline,
		m_bNeedSendOnline,
		m_bHideChats,
		m_bMesAsUnread,
		m_bUseLocalTime,
		m_bReportAbuse,
		m_bClearServerHistory,
		m_bRemoveFromFrendlist,
		m_bRemoveFromClist,
		m_bPopUpSyncHistory,
		m_bStikersAsSmyles,
		m_bUserForceOnlineOnActivity,
		m_bNewsEnabled,
		m_bNotificationsEnabled,
		m_bNotificationsMarkAsViewed,
		m_bSpecialContactAlwaysEnabled,
		m_bNewsAutoClearHistory,
		m_bNewsFilterPosts,
		m_bNewsFilterPhotos,
		m_bNewsFilterTags,
		m_bNewsFilterWallPhotos,
		m_bNewsSourceFriends,
		m_bNewsSourceGroups,
		m_bNewsSourcePages,
		m_bNewsSourceFollowing,
		m_bNewsSourceIncludeBanned,
		m_bNewsSourceNoReposts,
		m_bNotificationFilterComments,
		m_bNotificationFilterLikes,
		m_bNotificationFilterReposts,
		m_bNotificationFilterMentions,
		m_bNotificationFilterInvites,
		m_bUseBBCOnAttacmentsAsNews,
		m_bUseNonStandardUrlEncode,
		m_bSetBroadcast;

	int m_iNewsInterval, 
		m_iNotificationsInterval, 
		m_iNewsAutoClearHistoryInterval,
		m_iMaxLoadNewsPhoto,
		m_iInvisibleInterval;

	enum MarkMsgReadOn { markOnRead, markOnReceive, markOnReply, markOnTyping };
	MarkMsgReadOn m_iMarkMessageReadOn;
	
	enum SyncHistoryMetod { syncOff, syncAuto, sync1Days, sync3Days };
	SyncHistoryMetod m_iSyncHistoryMetod;
	
	enum MusicSendMetod { sendNone, sendStatusOnly, sendBroadcastOnly, sendBroadcastAndStatus };
	MusicSendMetod	m_iMusicSendMetod;

	enum IMGBBCSypport { imgNo, imgFullSize, imgPreview130, imgPreview604 };
	IMGBBCSypport m_iIMGBBCSupport;

	BBCSupport m_iBBCForNews, 
		m_iBBCForAttachments;

	LONG	m_myUserId;
	ptrT	m_defaultGroup;

	ptrA
		m_pollingServer,
		m_pollingKey,
		m_pollingTs,
		m_szAccessToken;

	HANDLE 
		m_pollingConn, 
		m_hPollingThread, 
		m_hPopupClassError, 
		m_hPopupClassNotification;
	
	ULONG m_msgId;
	
	LIST<void> m_sendIds;
	LIST<void> m_incIds;

	static INT_PTR CALLBACK OptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK OptionsAdvProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK OptionsFeedsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK OptionsViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	OBJLIST<CVkChatInfo> m_chats;
	CVkChatInfo* AppendChat(int id, const JSONNode &jnNode);
	void AppendChatMessage(int id, const JSONNode &jnMsg, bool bIsHistory);
	void AppendChatMessage(CVkChatInfo *cc, int uid, int msgTime, LPCTSTR ptszBody, bool bIsHistory);
	void RetrieveChatInfo(CVkChatInfo*);
	void OnReceiveChatInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnSendChatMsg(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void OnChatLeave(NETLIBHTTPREQUEST*, AsyncHttpRequest *);
	void OnChatDestroy(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	int __cdecl OnChatEvent(WPARAM, LPARAM);
	int __cdecl OnGcMenuHook(WPARAM, LPARAM);
	void KickFromChat(int chat_id, int user_id, const JSONNode &jnMsg);
	void LeaveChat(int chat_id, bool close_window = true, bool delete_chat = false);
	INT_PTR __cdecl OnLeaveChat(WPARAM, LPARAM);
	INT_PTR __cdecl OnJoinChat(WPARAM, LPARAM);
	void LogMenuHook(CVkChatInfo*, GCHOOK*);
	void NickMenuHook(CVkChatInfo*, GCHOOK*);
	LPTSTR ChangeChatTopic(CVkChatInfo*);
	void SetChatStatus(MCONTACT hContact, int iStatus);
	CVkChatInfo* GetChatById(LPCTSTR ptszId);
	INT_PTR __cdecl SvcCreateChat(WPARAM, LPARAM);
};