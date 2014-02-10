#pragma once

#define SCBIFSI_LOCK_CHANGES_EVENTS	1
#define SCBIF_ID			1
#define SCBIF_GROUP_ID		2
#define SCBIF_FLAG			4
#define SCBIF_SERVER_FLAG	8
#define SCBIF_STATUS		16
#define SCBIF_EMAIL			32
#define SCBIF_NICK			64
#define SCBIF_PHONES		128

#define MAIN_MENU_ITEMS_COUNT		13
#define CONTACT_MENU_ITEMS_COUNT	10
#define ADV_ICON_MAX             5

struct MRA_FILES_QUEUE_ITEM;

struct MraGroupItem
{
	MraGroupItem(DWORD id, DWORD flags, const CMStringW &name) :
		m_id(id), m_flags(flags), m_name(name)
		{}

	~MraGroupItem()
	{}

	DWORD     m_id, m_flags;
	CMStringW m_name;
};

struct CMraProto : public PROTO<CMraProto>
{
				CMraProto(const char*, const TCHAR*);
				~CMraProto();

	// ====================================================================================
	// PROTO_INTERFACE
	// ====================================================================================

	virtual	HCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	HCONTACT  __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int       __cdecl Authorize(HANDLE hDBEvent);
	virtual	int       __cdecl AuthDeny(HANDLE hDBEvent, const TCHAR* szReason);
	virtual	int       __cdecl AuthRecv(HCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl AuthRequest(HCONTACT hContact, const TCHAR* szMessage);

	virtual	HANDLE    __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE    __cdecl FileAllow(HCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath);
	virtual	int       __cdecl FileCancel(HCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(HCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, HCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(HCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const TCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(HCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(HCONTACT hContact, PROTORECVFILET*);
	virtual	int       __cdecl RecvMsg(HCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(HCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(HCONTACT hContact, int flags, int nContacts, HCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(HCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles);
	virtual	int       __cdecl SendMsg(HCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(HCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(HCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(HCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(HCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const TCHAR* msg);

	virtual	int       __cdecl UserIsTyping(HCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	void   ShowFormattedErrorMessage(LPWSTR lpwszErrText, DWORD dwErrorCode);
	void   MraPopupShowW(HCONTACT hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszTitle, LPCWSTR lpszMessage);
	void   MraPopupShowFromContactW(HCONTACT hContact, DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage);
	__forceinline void MraPopupShowFromAgentW(DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage) {
		MraPopupShowFromContactW(NULL, dwType, dwFlags, lpszMessage); }

	__forceinline bool mraGetStaticStringA(HCONTACT Contact, const char *ValueName, char *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringA(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }
	__forceinline bool mraGetStaticStringW(HCONTACT Contact, const char *ValueName, WCHAR *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringW(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }

	__forceinline bool mraGetStringA(HCONTACT Contact, const char *ValueName, CMStringA &Ret) {
		return DB_GetStringA(Contact, m_szModuleName, ValueName, Ret); }
	__forceinline bool mraGetStringW(HCONTACT Contact, const char *ValueName, CMStringW &Ret) {
		return DB_GetStringW(Contact, m_szModuleName, ValueName, Ret); }

	__forceinline bool mraSetStringA(HCONTACT Contact, const char *valueName, const CMStringA &parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue); }
	__forceinline bool mraSetStringW(HCONTACT Contact, const char *valueName, const CMStringW &parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue); }

	__forceinline bool mraSetStringExA(HCONTACT Contact, const char *valueName, const CMStringA &parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue); }
	__forceinline bool mraSetStringExW(HCONTACT Contact, const char *valueName, const CMStringW &parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue); }

	__forceinline bool mraWriteContactSettingBlob(HCONTACT hContact, const char *lpszValueName, LPVOID lpbValue, size_t dwValueSize) {
		return db_set_blob(hContact, m_szModuleName, lpszValueName, lpbValue, dwValueSize) != 0; }
	__forceinline bool mraGetContactSettingBlob(HCONTACT hContact, const char *lpszValueName, LPVOID lpbRet, size_t dwRetBuffSize, size_t *pdwRetBuffSize) {
		return DB_GetContactSettingBlob(hContact, m_szModuleName, lpszValueName, lpbRet, dwRetBuffSize, pdwRetBuffSize); }

	// ====| Services  |====================================================================
	INT_PTR __cdecl MraSetXStatusEx(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetXStatusEx(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetXStatusIcon(WPARAM, LPARAM);
	INT_PTR __cdecl MraXStatusMenu(WPARAM, LPARAM, LPARAM param);

	INT_PTR __cdecl MraSetListeningTo(WPARAM, LPARAM);

	INT_PTR __cdecl MraSendNudge(WPARAM, LPARAM);

	INT_PTR __cdecl MraGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetMyAvatar(WPARAM, LPARAM);

	INT_PTR __cdecl MraSendSMS(WPARAM, LPARAM);

	INT_PTR __cdecl MraGotoInbox(WPARAM, LPARAM);
	INT_PTR __cdecl MraShowInboxStatus(WPARAM, LPARAM);
	INT_PTR __cdecl MraEditProfile(WPARAM, LPARAM);
	INT_PTR __cdecl MraWebSearch(WPARAM, LPARAM);
	INT_PTR __cdecl MraUpdateAllUsersInfo(WPARAM, LPARAM);
	INT_PTR __cdecl MraCheckUpdatesUsersAvt(WPARAM, LPARAM);
	INT_PTR __cdecl MraRequestAuthForAll(WPARAM, LPARAM);

	INT_PTR __cdecl MraRequestAuthorization(WPARAM, LPARAM);
	INT_PTR __cdecl MraGrantAuthorization(WPARAM, LPARAM);
	INT_PTR __cdecl MraSendPostcard(WPARAM, LPARAM);
	INT_PTR __cdecl MraViewAlbum(WPARAM, LPARAM);
	INT_PTR __cdecl MraReadBlog(WPARAM, LPARAM);
	INT_PTR __cdecl MraReplyBlogStatus(WPARAM, LPARAM);
	INT_PTR __cdecl MraViewVideo(WPARAM, LPARAM);
	INT_PTR __cdecl MraAnswers(WPARAM, LPARAM);
	INT_PTR __cdecl MraWorld(WPARAM, LPARAM);
	INT_PTR __cdecl MraCreateAccMgrUI(WPARAM, LPARAM);

	// ====| Events  |======================================================================
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnPopupOptInit(WPARAM, LPARAM);
	int __cdecl OnGroupChanged(WPARAM, LPARAM);

	int __cdecl MraChatGcEventHook(WPARAM, LPARAM);

	int __cdecl MraExtraIconsApply(WPARAM, LPARAM);

	int __cdecl MraContactDeleted(WPARAM, LPARAM);
	int __cdecl MraDbSettingChanged(WPARAM, LPARAM);
	int __cdecl MraRebuildContactMenu(WPARAM, LPARAM);
	int __cdecl MraRebuildStatusMenu(WPARAM, LPARAM);
	int __cdecl MraMusicChanged(WPARAM, LPARAM);

	// ====| Data  |========================================================================
	bool     m_bLoggedIn;

	HANDLE   hSendQueueHandle, hFilesQueueHandle, hMPopSessionQueue;

	HANDLE   m_heNudgeReceived;
	HANDLE   m_hConnection;
	DWORD    m_dwThreadWorkerLastPingTime, m_dwNextPingSendTickTime, m_dwPingPeriod;
	DWORD    m_dwThreadWorkerRunning;
	DWORD    dwCMDNum;

	OBJLIST<MraGroupItem> m_groups;

	HANDLE   hAvatarsQueueHandle;

	HGENMENU hMainMenuItems[MAIN_MENU_ITEMS_COUNT];
	HGENMENU hContactMenuItems[CONTACT_MENU_ITEMS_COUNT];

	HWND     hWndEMailPopupStatus;
	DWORD    dwEmailMessagesTotal, m_dwEmailMessagesUnread;

	bool     bHideXStatusUI;
	HGENMENU hXStatusMenuItems[MRA_XSTATUS_COUNT+4], hContactMenuRoot;
	HANDLE   hExtraXstatusIcon;
	HANDLE   hExtraInfo;

	char     szNewMailSound[MAX_PATH];

	CRITICAL_SECTION csCriticalSectionSend;

	HCONTACT AddToListByEmail(LPCTSTR plpsEMail, LPCTSTR plpsNick, LPCTSTR plpsFirstName, LPCTSTR plpsLastName, DWORD dwFlags);

	DWORD  MraMessage(BOOL bAddToQueue, HCONTACT hContact, DWORD dwAckType, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &wszMessage, LPBYTE lpbMultiChatData, size_t dwMultiChatDataSize);
	DWORD  MraMessageAsk(DWORD dwMsgID, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &wszMessage, const CMStringW &wszMessageRTF);
	DWORD  MraMessageRecv(const CMStringA &szFrom, DWORD dwMsgID);
	DWORD  MraAddContact(HCONTACT hContact, DWORD dwContactFlag, DWORD dwGroupID, const CMStringA &szEmail, const CMStringW &wszCustomName, const CMStringA *szPhones = 0, const CMStringW *wszAuthMessage = 0);
	DWORD  MraModifyContact(HCONTACT hContact, DWORD *pdwID = 0, DWORD *pdwContactFlag = 0, DWORD *pdwGroupID = 0, const CMStringA *pszEmail = 0, const CMStringW *pwszCustomName = 0, const CMStringA *pszPhones = 0);
	DWORD  MraOfflineMessageDel(DWORDLONG dwMsgUIDL);
	DWORD  MraMoveContactToGroup(HCONTACT hContact, DWORD dwGroupID, LPCTSTR ptszGroup);
	DWORD  MraAuthorize(const CMStringA &szEmail);
	DWORD  MraChangeStatus(DWORD dwStatus, const CMStringA &szStatusUri, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc, DWORD dwFutureFlags);
	DWORD  MraFileTransfer(const CMStringA &szEmail, DWORD dwIdRequest, DWORD dwFilesTotalSize, const CMStringW &lpwszFiles, const CMStringA &szAddresses);
	DWORD  MraFileTransferAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIdRequest, LPBYTE lpbDescription, size_t dwDescriptionSize);
	HANDLE MraWPRequestW(HCONTACT hContact, DWORD dwAckType, DWORD dwRequestFlags, const CMStringA &szUser, const CMStringA &szDomain, const CMStringW &wszNickName, const CMStringW &wszFirstName, const CMStringW &wszLastName, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline);
	HANDLE MraWPRequestByEMail(HCONTACT hContact, DWORD dwAckType, CMStringA &szEmail);
	DWORD  MraGame(const CMStringA &szEmail, DWORD dwGameSessionID, DWORD dwGameMsg, DWORD dwGameMsgID, const CMStringA &lpszData);
	DWORD  MraLogin2W(CMStringA &szLogin, CMStringA &szPassword, DWORD dwStatus, CMStringA &szStatusUri, CMStringW &wszStatusTitle, CMStringW &wszStatusDesc, DWORD dwFutureFlags, CMStringA &szUserAgentFormatted, CMStringA &szUserAgent);
	DWORD  MraSMSW(HCONTACT hContact, const CMStringA &lpszPhone, const CMStringW &lpwszMessage);
	DWORD  MraProxy(const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID);
	DWORD  MraProxyAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID);
	DWORD  MraChangeUserBlogStatus(DWORD dwFlags, const CMStringW &wszText, DWORDLONG dwBlogStatusID);

	DWORD  MraSendPacket(HANDLE m_hConnection, DWORD dwCMDNum, DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendCMD(DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendQueueCMD(HANDLE hSendQueueHandle, DWORD dwFlags, HCONTACT hContact, DWORD dwAckType, LPBYTE lpbDataQueue, size_t dwDataQueueSize, DWORD dwType, LPVOID lpData, size_t dwDataSize);

	DWORD  MraSendNewStatus(DWORD dwStatusMir, DWORD dwXStatusMir, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc);

	void   MraAddrListStoreToContact(HCONTACT hContact, MRA_ADDR_LIST *pmalAddrList);

	DWORD  GetContactFlags(HCONTACT hContact);
	DWORD  SetContactFlags(HCONTACT hContact, DWORD dwContactFlag);
	DWORD  GetContactBasicInfoW(HCONTACT hContact, DWORD *pdwID, DWORD *pdwGroupID, DWORD *pdwContactFlag, DWORD *pdwContactSeverFlags, DWORD *pdwStatus, CMStringA *szEmail, CMStringW *wszNick, CMStringA *szPhones);
	DWORD  SetContactBasicInfoW(HCONTACT hContact, DWORD dwSetInfoFlags, DWORD dwFlags, DWORD dwID, DWORD dwGroupID, DWORD dwContactFlag, DWORD dwContactSeverFlags, DWORD dwStatus, const CMStringA *szEmail, const CMStringW *wszNick, const CMStringA *szPhones);

	DWORD  GetContactEMailCount(HCONTACT hContact, BOOL bMRAOnly);
	bool   GetContactFirstEMail(HCONTACT hContact, BOOL bMRAOnly, CMStringA &szRetBuff);

	bool   IsContactMra(HCONTACT hContact);
	bool   IsEMailMy(const CMStringA &szEmail);
	bool   IsEMailChatAgent(const CMStringA &szEmail);
	bool   IsContactChatAgent(HCONTACT hContact);

	HCONTACT MraHContactFromEmail(const CMStringA &szEmail, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded);
	bool   MraUpdateContactInfo(HCONTACT hContact);
	DWORD  MraSetXStatusInternal(DWORD dwXStatus);
	DWORD  MraGetContactStatus(HCONTACT hContact);
	DWORD  MraSetContactStatus(HCONTACT hContact, DWORD dwNewStatus);
	DWORD  MraContactCapabilitiesGet(HCONTACT hContact);
	void   MraContactCapabilitiesSet(HCONTACT hContact, DWORD dwFutureFlags);
	void   MraUpdateEmailStatus(const CMStringA &szFrom, const CMStringA &szSubject, DWORD dwDate, DWORD dwUIDL);
	DWORD  MraConvertToRTFW(const CMStringW &wszMessage, CMStringA &szMessageRTF);

	DWORD  StartConnect();
	void   __cdecl MraThreadProc(LPVOID lpParameter);
	DWORD  MraGetNLBData(CMStringA &szHost, WORD *pwPort);
	DWORD  MraNetworkDispatcher();

	bool   MraCommandDispatcher(struct mrim_packet_header_t *pmaHeader);
	bool   CmdHelloAck(BinBuffer &buf);
	bool   CmdLoginAck();
	bool   CmdLoginRejected(BinBuffer &buf);
	bool   CmdMessageAck(BinBuffer &buf);
	bool   CmdMessageStatus(ULONG seq, BinBuffer &buf);
	bool   CmdOfflineMessageAck(BinBuffer &buf);
	bool   CmdAuthAck(BinBuffer &buf);
	bool   CmdPopSession(BinBuffer &buf);
	bool   CmdFileTransfer(BinBuffer &buf);
	bool   CmdFileTransferAck(BinBuffer &buf);
	bool   CmdUserInfo(BinBuffer &buf);
	bool   CmdUserStatus(BinBuffer &buf);
	bool   CmdContactAck(int cmd, int seq, BinBuffer &buf);
	bool   CmdAnketaInfo(int seq, BinBuffer &buf);
	bool   CmdGame(BinBuffer &buf);
	bool   CmdClist2(BinBuffer &buf);
	bool   CmdProxy(BinBuffer &buf);
	bool   CmdProxyAck(BinBuffer &buf);
	bool   CmdNewMail(BinBuffer &buf);
	bool   CmdBlogStatus(BinBuffer &buf);

	DWORD  MraRecvCommand_Message(DWORD dwTime, DWORD dwFlags, CMStringA &plpsFrom, CMStringA &plpsText, CMStringA &plpsRFTText, CMStringA &plpsMultiChatData);

	void   SetExtraIcons(HCONTACT hContact);

	void   InitMenus(void);
	HGENMENU CListCreateMenu(LONG lPosition, LONG lPopupPosition, BOOL bIsStatus, const IconItem *pgdiItems, size_t dwCount, HGENMENU *hResult);

	DWORD  ProtoBroadcastAckAsync(HCONTACT hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam);
	CMStringA CreateBlobFromContact(HCONTACT hContact, const CMStringW &wszRequestReason);

	bool   GetPassDB(CMStringA& pass);

	bool   MraRequestXStatusDetails(DWORD dwXStatus);
	bool   MraSendReplyBlogStatus(HCONTACT hContact);
	DWORD  MraSelectEMailDlgShow(HCONTACT hContact, DWORD dwType);

	DWORD	 MraMrimProxyConnect(HANDLE hMraMrimProxyData, HANDLE *phConnection);

	DWORD  MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, const CMStringA &szUrl);
	DWORD  MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, const CMStringA &, CMStringA &szEmail);
	DWORD  MraMPopSessionQueueStart(HANDLE hMPopSessionQueue);
	void   MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue);

	size_t MraFilesQueueGetLocalAddressesList(LPSTR lpszBuff, size_t dwBuffSize, DWORD dwPort);
	DWORD	 MraFilesQueueAddReceive(HANDLE hFilesQueueHandle, DWORD dwFlags, HCONTACT hContact, DWORD dwIDRequest, const CMStringW &wszFiles, const CMStringA &szAddresses);
	DWORD  MraFilesQueueAddSend(HANDLE hFilesQueueHandle, DWORD dwFlags, HCONTACT hContact, LPWSTR *plpwszFiles, size_t dwFilesCount, DWORD *pdwIDRequest);
	DWORD  MraFilesQueueCancel(HANDLE hFilesQueueHandle, DWORD dwIDRequest, BOOL bSendDecline);
	DWORD  MraFilesQueueStartMrimProxy(HANDLE hFilesQueueHandle, DWORD dwIDRequest);
	DWORD  MraFilesQueueSendMirror(HANDLE hFilesQueueHandle, DWORD dwIDRequest, const CMStringA &szAddresses);
	bool   MraFilesQueueHandCheck(HANDLE m_hConnection, MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	HANDLE MraFilesQueueConnectOut(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	HANDLE MraFilesQueueConnectIn(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	DWORD  MraFilesQueueAccept(HANDLE hFilesQueueHandle, DWORD dwIDRequest, LPCWSTR lpwszPath, size_t dwPathSize);

	void   __cdecl MraFilesQueueRecvThreadProc(LPVOID lpParameter);
	void   __cdecl MraFilesQueueSendThreadProc(LPVOID lpParameter);

	bool    bChatExists;
	void    MraChatDllError();
	bool    MraChatRegister();
	INT_PTR MraChatSessionNew(HCONTACT hContactChatSession);
	void    MraChatSessionDestroy(HCONTACT hContactChatSession);
	void    MraChatSendPrivateMessage(LPSTR lpwszEMail);

	INT_PTR MraChatSessionEventSendByHandle(HCONTACT hContactChatSession, DWORD dwType, DWORD dwFlags, const CMStringA &szUID, LPCWSTR lpwszStatus, LPCWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime);
	INT_PTR MraChatSessionInvite(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionMembersAdd(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionJoinUser(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionLeftUser(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionSetIviter(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat);
	INT_PTR MraChatSessionSetOwner(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat);
	INT_PTR MraChatSessionMessageAdd(HCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, const CMStringW &wszMessage, DWORD dwTime);

	DWORD   MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle);
	void    MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle);
	void    MraAvatarsQueueSuspend(HANDLE hAvatarsQueueHandle);
	void    MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle);
	DWORD   MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HCONTACT hContact, DWORD *pdwAvatarsQueueID);
	bool    MraAvatarsGetContactTime	(HCONTACT hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	void    MraAvatarsSetContactTime	(HCONTACT hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	DWORD   MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle, HCONTACT hContact, DWORD dwFormat, CMStringW &res);
	DWORD   MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HCONTACT hContact, DWORD *pdwAvatarsQueueID, DWORD *pdwFormat, LPTSTR lpszPath);
   DWORD   MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HCONTACT hContact, DWORD dwSourceID);
   DWORD   MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle, HCONTACT hContact);

	void    __cdecl MraAvatarsThreadProc(LPVOID lpParameter);
};
