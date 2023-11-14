#pragma once

#define DBKEY_ID "id"
#define DBKEY_COMPAT "Compatibility"
#define DBKEY_AUTHORIZED "Authorized"

#define DBKEY_AVATAR_HASH "AvatarHash"
#define DBKEY_AVATAR_TYPE "AvatarType"

class CTelegramProto;
typedef void (CTelegramProto:: *TG_QUERY_HANDLER)(td::ClientManager::Response &response);
typedef void (CTelegramProto:: *TG_QUERY_HANDLER_FULL)(td::ClientManager::Response &response, void *pUserInfo);

struct TG_REQUEST_BASE
{
	TG_REQUEST_BASE(td::ClientManager::RequestId _1) :
		requestId(_1)
	{}

	virtual ~TG_REQUEST_BASE()
	{}

	td::ClientManager::RequestId requestId;

	virtual void Execute(CTelegramProto *ppro, td::ClientManager::Response &response) = 0;
};

struct TG_REQUEST : public TG_REQUEST_BASE
{
	TG_REQUEST(td::ClientManager::RequestId _1, TG_QUERY_HANDLER _2) :
		TG_REQUEST_BASE(_1),
		pHandler(_2)
	{}

	TG_QUERY_HANDLER pHandler;

	void Execute(CTelegramProto *ppro, td::ClientManager::Response &response) override
	{
		(ppro->*pHandler)(response);
	}
};

struct TG_REQUEST_FULL : public TG_REQUEST_BASE
{
	TG_REQUEST_FULL(td::ClientManager::RequestId _1, TG_QUERY_HANDLER_FULL _2, void *_3) :
		TG_REQUEST_BASE(_1),
		pHandler(_2),
		pUserInfo(_3)
	{}

	TG_QUERY_HANDLER_FULL pHandler;
	void *pUserInfo;

	void Execute(CTelegramProto *ppro, td::ClientManager::Response &response) override
	{
		(ppro->*pHandler)(response, pUserInfo);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct TG_FILE_REQUEST : public MZeroedObject
{
	enum Type { AVATAR = 1, FILE = 2, PICTURE = 3, VIDEO = 4, VOICE = 5 };

	TG_FILE_REQUEST(Type _1, TD::int53 _2, const char *_3) :
		m_type(_1),
		m_fileId(_2),
		m_uniqueId(_3)
	{}

	~TG_FILE_REQUEST()
	{	delete ofd;
	}

	void AutoDetectType();

	Type m_type;
	MCONTACT m_hContact = 0;
	TD::int53 m_fileId, m_fileSize = 0;
	CMStringA m_uniqueId, m_szUserId;
	CMStringW m_destPath, m_fileName, m_wszDescr;
	OFDTHREAD *ofd = 0;
};

struct TG_USER : public MZeroedObject
{
	TG_USER(int64_t _1, MCONTACT _2, bool _3 = false) :
		id(_1),
		hContact(_2),
		isGroupChat(_3)
	{
	}

	~TG_USER()
	{
		delete pReactions;
	}

	int64_t   id, chatId = -1;
	MCONTACT  hContact;
	bool      isGroupChat, bLoadMembers;
	CMStringA szAvatarHash;
	CMStringW wszNick, wszFirstName, wszLastName;
	time_t    m_timer1 = 0, m_timer2 = 0;
	SESSION_INFO *m_si = nullptr;
	TD::chatNotificationSettings notificationSettings;
	OBJLIST<char> *pReactions = nullptr;

	CMStringW getDisplayName() const;
};

struct TG_SUPER_GROUP
{
	TG_SUPER_GROUP(int64_t _1, TD::object_ptr<TD::supergroup> _2) :
		id(_1),
		group(std::move(_2))
	{}

	int64_t id;
	TD::object_ptr<TD::supergroup> group;
};

struct TG_BASIC_GROUP
{
	TG_BASIC_GROUP(int64_t _1, TD::object_ptr<TD::basicGroup> _2) :
		id(_1),
		group(std::move(_2))
	{}

	int64_t id;
	TD::object_ptr<TD::basicGroup> group;
};

struct TG_OWN_MESSAGE
{
	TG_OWN_MESSAGE(MCONTACT _1, HANDLE _2, int64_t _3) :
		hContact(_1),
		hAck(_2),
		tmpMsgId(_3)
	{}

	int64_t  tmpMsgId, tmpFileId = -1;
	HANDLE   hAck;
	MCONTACT hContact;
};

class CTelegramProto : public PROTO<CTelegramProto>
{
	friend class CForwardDlg;
	friend class CReactionsDlg;
	friend class CAddPhoneContactDlg;

	class CProtoImpl
	{
		friend class CTelegramProto;
		CTelegramProto &m_proto;

		CTimer m_keepAlive, m_markRead, m_deleteMsg;
		void OnKeepAlive(CTimer *)
		{	m_proto.SendKeepAlive();
		}

		void OnDeleteMsg(CTimer *)
		{	m_proto.SendDeleteMsg();
		}

		void OnMarkRead(CTimer *)
		{	m_proto.SendMarkRead();
		}

		CProtoImpl(CTelegramProto &pro) :
			m_proto(pro),
			m_markRead(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_keepAlive(Miranda_GetSystemWindow(), UINT_PTR(this)+1),
			m_deleteMsg(Miranda_GetSystemWindow(), UINT_PTR(this)+2)
		{
			m_markRead.OnEvent = Callback(this, &CProtoImpl::OnMarkRead);
			m_deleteMsg.OnEvent = Callback(this, &CProtoImpl::OnDeleteMsg);
			m_keepAlive.OnEvent = Callback(this, &CProtoImpl::OnKeepAlive);
		}
	} m_impl;

	bool __forceinline isRunning() const 
	{	return m_pClientManager != nullptr; 
	}

	std::unique_ptr<td::ClientManager> m_pClientManager;
	TD::object_ptr<TD::AuthorizationState> pAuthState;
	TD::object_ptr<TD::ConnectionState> pConnState;

	mir_cs m_csMarkRead;
	TD::int53 m_markChatId = 0;
	TD::array<TD::int53> m_markIds;

	mir_cs m_csDeleteMsg;
	TD::int53 m_deleteChatId = 0;
	TD::array<TD::int53> m_deleteIds;

	bool m_bAuthorized, m_bTerminated, m_bUnregister = false, m_bSmileyAdd = false;
	int32_t m_iClientId, m_iQueryId;
	CMStringA m_defaultEmoji;

	OBJLIST<TG_OWN_MESSAGE> m_arOwnMsg;
	OBJLIST<TG_REQUEST_BASE> m_arRequests;

	mir_cs m_csFiles;
	LIST<TG_FILE_REQUEST> m_arFiles;
	TG_FILE_REQUEST* PopFile(const char *pszUniqueId);

	static INT_PTR CALLBACK EnterEmail(void *param);
	static INT_PTR CALLBACK EnterEmailCode(void *param);
	static INT_PTR CALLBACK EnterPassword(void *param);
	static INT_PTR CALLBACK EnterPhoneCode(void *param);

	CMStringW GetProtoFolder() const
	{	return CMStringW(VARSW(L"%miranda_userdata%")) + L"\\" + _A2T(m_szModuleName);
	}

	void OnEndSession(td::ClientManager::Response &response);
	void OnGetFileInfo(td::ClientManager::Response &response, void *pUserInfo);
	void OnGetHistory(td::ClientManager::Response &response, void *pUserInfo);
	void OnSendFile(td::ClientManager::Response &response, void *pUserInfo);
	void OnSendMessage(td::ClientManager::Response &response);
	void OnUpdateAuth(td::ClientManager::Response &response);
	void OnGetChats(td::ClientManager::Response &response);

	void LogOut(void);
	void OnLoggedIn(void);
	void ProcessResponse(td::ClientManager::Response);

	void SendKeepAlive(void);
	void SendDeleteMsg(void);
	void SendMarkRead(void);
	int  SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER pHandler = nullptr);
	int  SendQuery(TD::Function *pFunc, TG_QUERY_HANDLER_FULL pHandler, void *pUserInfo);
	int  SendTextMessage(int64_t chatId, const char *pszMessage);

	void ProcessAuth(TD::updateAuthorizationState *pObj);
	void ProcessBasicGroup(TD::updateBasicGroup *pObj);
	void ProcessChat(TD::updateNewChat *pObj);
	void ProcessChatLastMessage(TD::updateChatLastMessage *pObj);
	void ProcessChatNotification(TD::updateChatNotificationSettings *pObj);
	void ProcessChatPosition(TD::updateChatPosition *pObj);
	void ProcessChatReactions(TD::updateChatAvailableReactions *);
	void ProcessConnectionState(TD::updateConnectionState *pObj);
	void ProcessActiveEmoji(TD::updateActiveEmojiReactions *pObj);
	void ProcessDeleteMessage(TD::updateDeleteMessages *pObj);
	void ProcessFile(TD::updateFile *pObj);
	void ProcessGroups(TD::updateChatFolders *pObj);
	void ProcessMarkRead(TD::updateChatReadInbox *pObj);
	void ProcessMessage(const TD::message *pMsg);
	void ProcessMessageContent(TD::updateMessageContent *pObj);
	void ProcessOption(TD::updateOption *pObj);
	void ProcessStatus(TD::updateUserStatus *pObj);
	void ProcessSuperGroup(TD::updateSupergroup *pObj);
	void ProcessUser(TD::updateUser *pObj);

	bool GetMessageFile(TG_FILE_REQUEST::Type, TG_USER *pUser, const TD::file *pFile, const char *pszFileName, const std::string &caption, const char *szId, const char *szUser, const TD::message *pMsg);
	CMStringA GetMessageSticker(const TD::file *pFile, const char *pwszExtension);
	CMStringA GetMessageText(TG_USER *pUser, const TD::message *pMsg);

	void UpdateString(MCONTACT hContact, const char *pszSetting, const std::string &str);

	// Avatars
	CMStringW GetAvatarFilename(MCONTACT hContact);

	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SvcSetMyAvatar(WPARAM, LPARAM);

	// Group chats
	OBJLIST<TG_BASIC_GROUP>  m_arBasicGroups;
	OBJLIST<TG_SUPER_GROUP>  m_arSuperGroups;

	void InitGroupChat(TG_USER *pUser, const TD::chat *pChat);
	void StartGroupChat(td::ClientManager::Response &response, void *pUserData);
	
	void Chat_SendPrivateMessage(GCHOOK *gch);
	void Chat_LogMenu(GCHOOK *gch);

	bool GetGcUserId(TG_USER *pUser, const TD::message *pMsg, char *dest);

	// Search
	volatile unsigned m_iSearchCount;
	TD::array<TD::int53> m_searchIds;

	void OnSearchResults(td::ClientManager::Response &response);

	bool CheckSearchUser(TG_USER *pUser);
	void ReportSearchUser(TG_USER *pUser);

	// Users
	int64_t m_iOwnId;
	MGROUP m_iBaseGroup;
	MCONTACT m_iSavedMessages;
	LIST<TG_USER> m_arChats;
	OBJLIST<TG_USER> m_arUsers;

	TG_USER* FindChat(int64_t id);
	TG_USER* FindUser(int64_t id);
	TG_USER* AddUser(int64_t id, bool bIsChat);
	TG_USER* AddFakeUser(int64_t id, bool bIsChat);
	TG_USER* GetSender(const TD::MessageSender *pSender);
	
	int64_t  GetId(MCONTACT);
	void     SetId(MCONTACT, int64_t id);

	MCONTACT GetRealContact(const TG_USER *pUser);

	// Menus
	HGENMENU hmiForward, hmiReaction;

	void InitMenus();

	INT_PTR __cdecl SvcExecMenu(WPARAM, LPARAM);
	int     __cdecl OnPrebuildMenu(WPARAM, LPARAM);

	// Popups
	HANDLE m_hPopupClass;

	void InitPopups(void);
	void Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle);

public:
	//////////////////////////////////////////////////////////////////////////////////////
	// Ctors

	CTelegramProto(const char *protoName, const wchar_t *userName);
	~CTelegramProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;

	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	MEVENT   RecvFile(MCONTACT hContact, PROTORECVFILE *pre) override;

	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;
	int      SendMsg(MCONTACT hContact, const char *pszMessage) override;
	int      SetStatus(int iNewStatus) override;
		
	void     OnBuildProtoMenu() override;
	void     OnContactDeleted(MCONTACT hContact) override;
	MWindow  OnCreateAccMgrUI(MWindow hwndParent) override;
	void     OnErase() override;
	void     OnEventDeleted(MCONTACT, MEVENT) override;
	void     OnEventEdited(MCONTACT, MEVENT, const DBEVENTINFO &dbei) override;
	void     OnMarkRead(MCONTACT, MEVENT) override;
	void     OnModulesLoaded() override;
	void     OnReceiveOfflineFile(DB::FILE_BLOB &blob, void *ft) override;
	void     OnSendOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob, void *hTransfer) override;
	void     OnShutdown() override;

	// Events ////////////////////////////////////////////////////////////////////////////
	
	int __cdecl OnEmptyHistory(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnWindowEvent(WPARAM, LPARAM);

	int __cdecl GcMenuHook(WPARAM, LPARAM);
	int __cdecl GcMuteHook(WPARAM, LPARAM);
	int __cdecl GcEventHook(WPARAM, LPARAM);

	// Services //////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcAddByPhone(WPARAM, LPARAM);
	INT_PTR __cdecl SvcOfflineFile(WPARAM, LPARAM);
	INT_PTR __cdecl SvcLoadServerHistory(WPARAM, LPARAM);

	// Options ///////////////////////////////////////////////////////////////////////////
	
	CMOption<uint32_t> m_iCountry;		   // set this status to m_iStatus1 after this interval of secs
	CMOption<wchar_t*> m_szOwnPhone;       // our own phone number
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<wchar_t*> m_wszDeviceName;    // how do you see this session in Device List
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation
	CMOption<bool>     m_bUsePopups;
	CMOption<bool>     m_bCompressFiles;   // embed pictures & videos into a message on send
	CMOption<uint32_t> m_iTimeDiff1;		   // set this status to m_iStatus1 after this interval of secs
	CMOption<uint32_t> m_iStatus1;
	CMOption<uint32_t> m_iTimeDiff2;		   // set this status to m_iStatus2 after this interval of secs
	CMOption<uint32_t> m_iStatus2;

	// Processing Threads ////////////////////////////////////////////////////////////////

	void __cdecl OfflineFileThread(void *);
	void __cdecl ServerThread(void *);
};

typedef CProtoDlgBase<CTelegramProto> CTelegramDlgBase;
