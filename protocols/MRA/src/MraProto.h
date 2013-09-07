#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SCBIFSI_LOCK_CHANGES_EVENTS	1
#define SCBIF_ID			1
#define SCBIF_GROUP_ID		2
#define SCBIF_FLAG			4
#define SCBIF_SERVER_FLAG	8
#define SCBIF_STATUS		16
#define SCBIF_EMAIL			32
#define SCBIF_NICK			64
#define SCBIF_PHONES		128

#define MAIN_MENU_ITEMS_COUNT		15
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

	virtual	HANDLE __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	HANDLE __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int    __cdecl Authorize(HANDLE hDBEvent);
	virtual	int    __cdecl AuthDeny(HANDLE hContact, const TCHAR* szReason);
	virtual	int    __cdecl AuthRecv(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl AuthRequest(HANDLE hContact, const TCHAR* szMessage);

	virtual	HANDLE __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE __cdecl FileAllow(HANDLE hContact, HANDLE hTransfer, const TCHAR* szPath);
	virtual	int    __cdecl FileCancel(HANDLE hContact, HANDLE hTransfer);
	virtual	int    __cdecl FileDeny(HANDLE hContact, HANDLE hTransfer, const TCHAR* szReason);
	virtual	int    __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, HANDLE hContact = NULL);
	virtual	int    __cdecl GetInfo(HANDLE hContact, int infoType);

	virtual	HANDLE __cdecl SearchBasic(const TCHAR* id);
	virtual	HANDLE __cdecl SearchByEmail(const TCHAR* email);
	virtual	HANDLE __cdecl SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName);
	virtual	HWND   __cdecl SearchAdvanced(HWND owner);
	virtual	HWND   __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int    __cdecl RecvContacts(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl RecvFile(HANDLE hContact, PROTORECVFILET*);
	virtual	int    __cdecl RecvMsg(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl RecvUrl(HANDLE hContact, PROTORECVEVENT*);

	virtual	int    __cdecl SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList);
	virtual	HANDLE __cdecl SendFile(HANDLE hContact, const TCHAR* szDescription, TCHAR** ppszFiles);
	virtual	int    __cdecl SendMsg(HANDLE hContact, int flags, const char* msg);
	virtual	int    __cdecl SendUrl(HANDLE hContact, int flags, const char* url);

	virtual	int    __cdecl SetApparentMode(HANDLE hContact, int mode);
	virtual	int    __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE __cdecl GetAwayMsg(HANDLE hContact);
	virtual	int    __cdecl RecvAwayMsg(HANDLE hContact, int mode, PROTORECVEVENT* evt);
	virtual	int    __cdecl SendAwayMsg(HANDLE hContact, HANDLE hProcess, const char* msg);
	virtual	int    __cdecl SetAwayMsg(int m_iStatus, const TCHAR* msg);

	virtual	int    __cdecl UserIsTyping(HANDLE hContact, int type);

	virtual	int    __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	void   ShowFormattedErrorMessage(LPWSTR lpwszErrText, DWORD dwErrorCode);
	void   MraPopupShowW(HANDLE hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszTitle, LPCWSTR lpszMessage);
	void   MraPopupShowFromContactW(HANDLE hContact, DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage);
	__forceinline void MraPopupShowFromAgentW(DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage) {
		MraPopupShowFromContactW(NULL, dwType, dwFlags, lpszMessage); }

	__forceinline BOOL mraGetStaticStringA(HANDLE Contact, const char *ValueName, char *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringA(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }
	__forceinline BOOL mraGetStaticStringW(HANDLE Contact, const char *ValueName, WCHAR *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringW(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }

	__forceinline BOOL mraGetStringA(HANDLE Contact, const char *ValueName, CMStringA &Ret) {
		return DB_GetStringA(Contact, m_szModuleName, ValueName, Ret); }
	__forceinline BOOL mraGetStringW(HANDLE Contact, const char *ValueName, CMStringW &Ret) {
		return DB_GetStringW(Contact, m_szModuleName, ValueName, Ret); }

	__forceinline BOOL mraSetStringA(HANDLE Contact, const char *valueName, const CMStringA &parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue); }
	__forceinline BOOL mraSetStringW(HANDLE Contact, const char *valueName, const CMStringW &parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue); }

	__forceinline BOOL mraSetStringExA(HANDLE Contact, const char *valueName, const CMStringA &parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue); }
	__forceinline BOOL mraSetStringExW(HANDLE Contact, const char *valueName, const CMStringW &parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue); }

	__forceinline BOOL mraWriteContactSettingBlob(HANDLE hContact, const char *lpszValueName, LPVOID lpbValue, size_t dwValueSize) {
		return db_set_blob(hContact, m_szModuleName, lpszValueName, lpbValue, dwValueSize); }
	__forceinline BOOL mraGetContactSettingBlob(HANDLE hContact, const char *lpszValueName, LPVOID lpbRet, size_t dwRetBuffSize, size_t *pdwRetBuffSize) {
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
	INT_PTR __cdecl MraZhuki(WPARAM, LPARAM);
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

	HANDLE   m_hNetlibUser, m_heNudgeReceived;
	HANDLE   m_hThreadWorker;
	HANDLE   m_hConnection;
	DWORD    m_dwThreadWorkerLastPingTime, m_dwNextPingSendTickTime, m_dwPingPeriod;
	DWORD    m_dwThreadWorkerRunning;
	DWORD    dwCMDNum;

	OBJLIST<MraGroupItem> m_groups;

	HANDLE   hAvatarsQueueHandle;

	HGENMENU hMainMenuItems[MAIN_MENU_ITEMS_COUNT+4];
	HGENMENU hContactMenuItems[CONTACT_MENU_ITEMS_COUNT+4];

	HWND     hWndEMailPopupStatus;
	DWORD    dwEmailMessagesTotal, m_dwEmailMessagesUnread;

	bool     bHideXStatusUI;
	HGENMENU hXStatusMenuItems[MRA_XSTATUS_COUNT+4];
	HANDLE   hExtraXstatusIcon;
	HANDLE   hExtraInfo;

	char    szNewMailSound[MAX_PATH];

	CRITICAL_SECTION csCriticalSectionSend;

	HANDLE  AddToListByEmail(LPCTSTR plpsEMail, LPCTSTR plpsNick, LPCTSTR plpsFirstName, LPCTSTR plpsLastName, DWORD dwFlags);

	DWORD  MraMessage(BOOL bAddToQueue, HANDLE hContact, DWORD dwAckType, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &wszMessage, LPBYTE lpbMultiChatData, size_t dwMultiChatDataSize);
	DWORD  MraMessageAsk(DWORD dwMsgID, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &wszMessage, const CMStringW &wszMessageRTF);
	DWORD  MraMessageRecv(const CMStringA &szFrom, DWORD dwMsgID);
	DWORD  MraAddContact(HANDLE hContact, DWORD dwContactFlag, DWORD dwGroupID, const CMStringA &szEmail, const CMStringW &wszCustomName, const CMStringA &szPhones, const CMStringW &wszAuthMessage, DWORD dwActions);
	DWORD  MraModifyContact(HANDLE hContact, DWORD *pdwID = 0, DWORD *pdwContactFlag = 0, DWORD *pdwGroupID = 0, const CMStringA *pszEmail = 0, const CMStringW *pwszCustomName = 0, const CMStringA *pszPhones = 0);
	DWORD  MraOfflineMessageDel(DWORDLONG dwMsgUIDL);
	DWORD  MraMoveContactToGroup(HANDLE hContact, DWORD dwGroupID, LPCTSTR ptszGroup);
	DWORD  MraAuthorize(const CMStringA &szEmail);
	DWORD  MraChangeStatus(DWORD dwStatus, const CMStringA &szStatusUri, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc, DWORD dwFutureFlags);
	DWORD  MraFileTransfer(const CMStringA &szEmail, DWORD dwIdRequest, DWORD dwFilesTotalSize, const CMStringW &lpwszFiles, const CMStringA &szAddresses);
	DWORD  MraFileTransferAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIdRequest, LPBYTE lpbDescription, size_t dwDescriptionSize);
	HANDLE MraWPRequestW(HANDLE hContact, DWORD dwAckType, DWORD dwRequestFlags, const CMStringA &szUser, const CMStringA &szDomain, const CMStringW &wszNickName, const CMStringW &wszFirstName, const CMStringW &wszLastName, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline);
	HANDLE MraWPRequestByEMail(HANDLE hContact, DWORD dwAckType, CMStringA &szEmail);
	DWORD  MraGame(const CMStringA &szEmail, DWORD dwGameSessionID, DWORD dwGameMsg, DWORD dwGameMsgID, const CMStringA &lpszData);
	DWORD  MraLogin2W(CMStringA &szLogin, CMStringA &szPassword, DWORD dwStatus, CMStringA &szStatusUri, CMStringW &wszStatusTitle, CMStringW &wszStatusDesc, DWORD dwFutureFlags, CMStringA &szUserAgentFormatted, CMStringA &szUserAgent);
	DWORD  MraSMSW(HANDLE hContact, const CMStringA &lpszPhone, const CMStringW &lpwszMessage);
	DWORD  MraProxy(const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID);
	DWORD  MraProxyAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID);
	DWORD  MraChangeUserBlogStatus(DWORD dwFlags, const CMStringW &wszText, DWORDLONG dwBlogStatusID);

	DWORD  MraSendPacket(HANDLE m_hConnection, DWORD dwCMDNum, DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendCMD(DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendQueueCMD(HANDLE hSendQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwAckType, LPBYTE lpbDataQueue, size_t dwDataQueueSize, DWORD dwType, LPVOID lpData, size_t dwDataSize);

	DWORD  MraSendNewStatus(DWORD dwStatusMir, DWORD dwXStatusMir, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc);

	void   MraAddrListStoreToContact(HANDLE hContact, MRA_ADDR_LIST *pmalAddrList);

	DWORD  GetContactFlags(HANDLE hContact);
	DWORD  SetContactFlags(HANDLE hContact, DWORD dwContactFlag);
	DWORD  GetContactBasicInfoW(HANDLE hContact, DWORD *pdwID, DWORD *pdwGroupID, DWORD *pdwContactFlag, DWORD *pdwContactSeverFlags, DWORD *pdwStatus, CMStringA *szEmail, CMStringW *wszNick, CMStringA *szPhones);
	DWORD  SetContactBasicInfoW(HANDLE hContact, DWORD dwSetInfoFlags, DWORD dwFlags, DWORD dwID, DWORD dwGroupID, DWORD dwContactFlag, DWORD dwContactSeverFlags, DWORD dwStatus, const CMStringA *szEmail, const CMStringW *wszNick, const CMStringA *szPhones);

	DWORD  GetContactEMailCount(HANDLE hContact, BOOL bMRAOnly);
	bool   GetContactFirstEMail(HANDLE hContact, BOOL bMRAOnly, CMStringA &szRetBuff);

	bool   IsContactMra(HANDLE hContact);
	bool   IsEMailMy(const CMStringA &szEmail);
	bool   IsEMailChatAgent(const CMStringA &szEmail);
	bool   IsContactChatAgent(HANDLE hContact);

	HANDLE MraHContactFromEmail(const CMStringA &szEmail, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded);
	bool   MraUpdateContactInfo(HANDLE hContact);
	DWORD  MraSetXStatusInternal(DWORD dwXStatus);
	DWORD  MraGetContactStatus(HANDLE hContact);
	DWORD  MraSetContactStatus(HANDLE hContact, DWORD dwNewStatus);
	DWORD  MraContactCapabilitiesGet(HANDLE hContact);
	void   MraContactCapabilitiesSet(HANDLE hContact, DWORD dwFutureFlags);
	void   MraUpdateEmailStatus(const CMStringA &szFrom, const CMStringA &szSubject, DWORD dwDate, DWORD dwUIDL);
	DWORD  MraConvertToRTFW(const CMStringW &wszMessage, CMStringA &szMessageRTF);

	void   MraThreadClean();
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

	void   SetExtraIcons(HANDLE hContact);

	#define	MESSAGE_NOT_SPAM	1
	#define	MESSAGE_SPAM		2

	DWORD  MraAntiSpamReceivedMessageW(const CMStringA &szEmail, DWORD dwMessageFlags, const CMStringW &wszMessage);
	bool   MraAntiSpamHasMessageBadWordsW(const CMStringW &wszMessage);

	void   InitMainMenu();
	void   InitContactMenu();
	void   CListCreateMenu(LONG lPosition, LONG lPopupPosition, HICON hMainIcon, LPSTR pszContactOwner, BOOL bIsStatus, const struct GUI_DISPLAY_ITEM *pgdiItems, size_t dwCount, HGENMENU *hResult);

	DWORD  ProtoBroadcastAckAsync(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam);
	CMStringA CreateBlobFromContact(HANDLE hContact, const CMStringW &wszRequestReason);

	bool   SetPassDB(const CMStringA& pass);
	bool   GetPassDB(CMStringA& pass);
	bool   GetPassDB_v1(CMStringA &res);

	bool   MraRequestXStatusDetails(DWORD dwXStatus);
	bool   MraSendReplyBlogStatus(HANDLE hContact);
	DWORD  MraSelectEMailDlgShow(HANDLE hContact, DWORD dwType);

	DWORD	 MraMrimProxyConnect(HANDLE hMraMrimProxyData, HANDLE *phConnection);

	DWORD  MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, const CMStringA &szUrl);
	DWORD  MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, const CMStringA &, CMStringA &szEmail);
	DWORD  MraMPopSessionQueueStart(HANDLE hMPopSessionQueue);
	void   MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue);

	size_t MraFilesQueueGetLocalAddressesList(LPSTR lpszBuff, size_t dwBuffSize, DWORD dwPort);
	DWORD	 MraFilesQueueAddReceive(HANDLE hFilesQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwIDRequest, const CMStringW &wszFiles, const CMStringA &szAddresses);
	DWORD  MraFilesQueueAddSend(HANDLE hFilesQueueHandle, DWORD dwFlags, HANDLE hContact, LPWSTR *plpwszFiles, size_t dwFilesCount, DWORD *pdwIDRequest);
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
	INT_PTR MraChatSessionNew(HANDLE hContactChatSession);
	void    MraChatSessionDestroy(HANDLE hContactChatSession);
	void    MraChatSendPrivateMessage(LPSTR lpwszEMail);

	INT_PTR MraChatSessionEventSendByHandle(HANDLE hContactChatSession, DWORD dwType, DWORD dwFlags, const CMStringA &szUID, LPCWSTR lpwszStatus, LPCWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime);
	INT_PTR MraChatSessionInvite(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionMembersAdd(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionJoinUser(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionLeftUser(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionSetIviter(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat);
	INT_PTR MraChatSessionSetOwner(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat);
	INT_PTR MraChatSessionMessageAdd(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, const CMStringW &wszMessage, DWORD dwTime);

	DWORD   MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle);
	void    MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle);
	void    MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle);
	DWORD   MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD *pdwAvatarsQueueID);
	bool    MraAvatarsGetContactTime	(HANDLE hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	void    MraAvatarsSetContactTime	(HANDLE hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	DWORD   MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle, HANDLE hContact, DWORD dwFormat, CMStringW &res);
	DWORD   MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD *pdwAvatarsQueueID, DWORD *pdwFormat, LPTSTR lpszPath);
   DWORD   MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwSourceID);
   DWORD   MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle, HANDLE hContact);

	void    __cdecl MraAvatarsThreadProc(LPVOID lpParameter);

};
