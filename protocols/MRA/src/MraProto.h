#pragma once

#define SCBIFSI_LOCK_CHANGES_EVENTS	1
#define SCBIF_ID			1
#define SCBIF_GROUP_ID			2
#define SCBIF_FLAG			4
#define SCBIF_SERVER_FLAG		8
#define SCBIF_STATUS			16
#define SCBIF_EMAIL			32
#define SCBIF_NICK			64
#define SCBIF_PHONES			128

#define MAIN_MENU_ITEMS_COUNT		12
#define CONTACT_MENU_ITEMS_COUNT	10
#define ADV_ICON_MAX			5

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

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);

	virtual	int       __cdecl Authorize(MEVENT hDBEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDBEvent, const TCHAR* szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const TCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const TCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	void   ShowFormattedErrorMessage(LPWSTR lpwszErrText, DWORD dwErrorCode);
	void   MraPopupShowW(MCONTACT hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszTitle, LPCWSTR lpszMessage);
	void   MraPopupShowFromContactW(MCONTACT hContact, DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage);
	__forceinline void MraPopupShowFromAgentW(DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage) {
		MraPopupShowFromContactW(NULL, dwType, dwFlags, lpszMessage); }

	__forceinline bool mraGetStaticStringA(MCONTACT Contact, const char *ValueName, char *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringA(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }
	__forceinline bool mraGetStaticStringW(MCONTACT Contact, const char *ValueName, WCHAR *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringW(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }

	__forceinline bool mraGetStringA(MCONTACT Contact, const char *ValueName, CMStringA &Ret) {
		return DB_GetStringA(Contact, m_szModuleName, ValueName, Ret); }
	__forceinline bool mraGetStringW(MCONTACT Contact, const char *ValueName, CMStringW &Ret) {
		return DB_GetStringW(Contact, m_szModuleName, ValueName, Ret); }

	__forceinline bool mraSetStringA(MCONTACT Contact, const char *valueName, const CMStringA &parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue); }
	__forceinline bool mraSetStringW(MCONTACT Contact, const char *valueName, const CMStringW &parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue); }

	__forceinline bool mraSetStringExA(MCONTACT Contact, const char *valueName, const CMStringA &parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue); }
	__forceinline bool mraSetStringExW(MCONTACT Contact, const char *valueName, const CMStringW &parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue); }

	__forceinline bool mraWriteContactSettingBlob(MCONTACT hContact, const char *lpszValueName, LPVOID lpbValue, size_t dwValueSize) {
		return db_set_blob(hContact, m_szModuleName, lpszValueName, lpbValue, dwValueSize) != 0; }
	__forceinline bool mraGetContactSettingBlob(MCONTACT hContact, const char *lpszValueName, LPVOID lpbRet, size_t dwRetBuffSize, size_t *pdwRetBuffSize) {
		return DB_GetContactSettingBlob(hContact, m_szModuleName, lpszValueName, lpbRet, dwRetBuffSize, pdwRetBuffSize); }

	// ====| Services  |====================================================================
	INT_PTR __cdecl MraSetXStatusEx(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetXStatusEx(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetXStatusIcon(WPARAM, LPARAM);
	INT_PTR __cdecl MraXStatusMenu(WPARAM, LPARAM, LPARAM param);

	INT_PTR __cdecl MraSetListeningTo(WPARAM, LPARAM);

	INT_PTR __cdecl MraSendSMS(WPARAM, LPARAM);

	INT_PTR __cdecl MraSendNudge(WPARAM, LPARAM);

	INT_PTR __cdecl GetUnreadEmailCount(WPARAM, LPARAM);

	INT_PTR __cdecl MraGetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl MraGetMyAvatar(WPARAM, LPARAM);

	INT_PTR __cdecl MraGotoInbox(WPARAM, LPARAM);
	INT_PTR __cdecl MraShowInboxStatus(WPARAM, LPARAM);
	INT_PTR __cdecl MraEditProfile(WPARAM, LPARAM);
	INT_PTR __cdecl MraWebSearch(WPARAM, LPARAM);
	INT_PTR __cdecl MraUpdateAllUsersInfo(WPARAM, LPARAM);
	INT_PTR __cdecl MraCheckUpdatesUsersAvt(WPARAM, LPARAM);
	INT_PTR __cdecl MraRequestAuthForAll(WPARAM, LPARAM);

	INT_PTR __cdecl MraRequestAuthorization(WPARAM, LPARAM);
	INT_PTR __cdecl MraGrantAuthorization(WPARAM, LPARAM);
	INT_PTR __cdecl MraSendEmail(WPARAM, LPARAM);
	INT_PTR __cdecl MraSendPostcard(WPARAM, LPARAM);
	INT_PTR __cdecl MraViewAlbum(WPARAM, LPARAM);
	INT_PTR __cdecl MraReplyBlogStatus(WPARAM, LPARAM);
	INT_PTR __cdecl MraViewVideo(WPARAM, LPARAM);
	INT_PTR __cdecl MraAnswers(WPARAM, LPARAM);
	INT_PTR __cdecl MraWorld(WPARAM, LPARAM);
	INT_PTR __cdecl MraCreateAccMgrUI(WPARAM, LPARAM);

	// ====| Events  |======================================================================
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnShutdown(WPARAM, LPARAM);
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
	DWORD	   m_dwNextPingSendTickTime;
	DWORD    m_dwPingPeriod;
	volatile DWORD	m_dwThreadWorkerLastPingTime;
	volatile DWORD	m_dwThreadWorkerRunning;
	volatile DWORD	dwCMDNum;

	OBJLIST<MraGroupItem> m_groups;

	HANDLE   hAvatarsQueueHandle;

	HGENMENU hMainMenuItems[MAIN_MENU_ITEMS_COUNT];
	HGENMENU hContactMenuItems[CONTACT_MENU_ITEMS_COUNT];

	HWND     hWndEMailPopupStatus;
	DWORD    dwEmailMessagesTotal, m_dwEmailMessagesUnread;

	bool     m_bShutdown;
	bool     m_bHideXStatusUI;
	HGENMENU hXStatusMenuItems[MRA_XSTATUS_COUNT+4], hContactMenuRoot;
	HANDLE   hExtraXstatusIcon;
	HANDLE   hExtraInfo;

	char     szNewMailSound[MAX_PATH];

	CRITICAL_SECTION csCriticalSectionSend;

	MCONTACT AddToListByEmail(LPCTSTR plpsEMail, LPCTSTR plpsNick, LPCTSTR plpsFirstName, LPCTSTR plpsLastName, DWORD dwFlags);

	DWORD  MraMessage(BOOL bAddToQueue, MCONTACT hContact, DWORD dwAckType, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &wszMessage, LPBYTE lpbMultiChatData, size_t dwMultiChatDataSize);
	DWORD  MraMessageAsk(DWORD dwMsgID, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &wszMessage, const CMStringW &wszMessageRTF);
	DWORD  MraMessageRecv(const CMStringA &szFrom, DWORD dwMsgID);
	DWORD  MraAddContact(MCONTACT hContact, DWORD dwContactFlag, DWORD dwGroupID, const CMStringA &szEmail, const CMStringW &wszCustomName, const CMStringA *szPhones = 0, const CMStringW *wszAuthMessage = 0);
	DWORD  MraModifyContact(MCONTACT hContact, DWORD *pdwID = 0, DWORD *pdwContactFlag = 0, DWORD *pdwGroupID = 0, const CMStringA *pszEmail = 0, const CMStringW *pwszCustomName = 0, const CMStringA *pszPhones = 0);
	DWORD  MraOfflineMessageDel(DWORDLONG dwMsgUIDL);
	DWORD  MraMoveContactToGroup(MCONTACT hContact, DWORD dwGroupID, LPCTSTR ptszGroup);
	DWORD  MraAuthorize(const CMStringA &szEmail);
	DWORD  MraChangeStatus(DWORD dwStatus, const CMStringA &szStatusUri, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc, DWORD dwFutureFlags);
	DWORD  MraFileTransfer(const CMStringA &szEmail, DWORD dwIdRequest, DWORD dwFilesTotalSize, const CMStringW &lpwszFiles, const CMStringA &szAddresses);
	DWORD  MraFileTransferAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIdRequest, const CMStringA &szDescription);
	HANDLE MraWPRequestW(MCONTACT hContact, DWORD dwAckType, DWORD dwRequestFlags, const CMStringA &szUser, const CMStringA &szDomain, const CMStringW &wszNickName, const CMStringW &wszFirstName, const CMStringW &wszLastName, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline);
	HANDLE MraWPRequestByEMail(MCONTACT hContact, DWORD dwAckType, CMStringA &szEmail);
	DWORD  MraGame(const CMStringA &szEmail, DWORD dwGameSessionID, DWORD dwGameMsg, DWORD dwGameMsgID, const CMStringA &lpszData);
	DWORD  MraLogin2W(const CMStringA &szLogin, const CMStringA &szPassword, DWORD dwStatus, const CMStringA &szStatusUri, CMStringW &wszStatusTitle, CMStringW &wszStatusDesc, DWORD dwFutureFlags, CMStringA &szUserAgentFormatted, CMStringA &szUserAgent);
	DWORD  MraSMSW(MCONTACT hContact, const CMStringA &lpszPhone, const CMStringW &lpwszMessage);
	DWORD  MraProxy(const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID);
	DWORD  MraProxyAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID);
	DWORD  MraChangeUserBlogStatus(DWORD dwFlags, const CMStringW &wszText, DWORDLONG dwBlogStatusID);

	DWORD  MraSendPacket(HANDLE hConnection, DWORD dwCMDNum, DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendCMD(DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendQueueCMD(HANDLE hSendQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD dwAckType, LPBYTE lpbDataQueue, size_t dwDataQueueSize, DWORD dwType, LPVOID lpData, size_t dwDataSize);

	DWORD  MraSendNewStatus(DWORD dwStatusMir, DWORD dwXStatusMir, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc);

	void   MraAddrListStoreToContact(MCONTACT hContact, MRA_ADDR_LIST *pmalAddrList);

	DWORD  GetContactFlags(MCONTACT hContact);
	DWORD  SetContactFlags(MCONTACT hContact, DWORD dwContactFlag);
	DWORD  GetContactBasicInfoW(MCONTACT hContact, DWORD *pdwID, DWORD *pdwGroupID, DWORD *pdwContactFlag, DWORD *pdwContactSeverFlags, DWORD *pdwStatus, CMStringA *szEmail, CMStringW *wszNick, CMStringA *szPhones);
	DWORD  SetContactBasicInfoW(MCONTACT hContact, DWORD dwSetInfoFlags, DWORD dwFlags, DWORD dwID, DWORD dwGroupID, DWORD dwContactFlag, DWORD dwContactSeverFlags, DWORD dwStatus, const CMStringA *szEmail, const CMStringW *wszNick, const CMStringA *szPhones);

	DWORD  GetContactEMailCount(MCONTACT hContact, BOOL bMRAOnly);
	bool   GetContactFirstEMail(MCONTACT hContact, BOOL bMRAOnly, CMStringA &szRetBuff);

	bool   IsContactMra(MCONTACT hContact);
	bool   IsEMailMy(const CMStringA &szEmail);
	bool   IsEMailChatAgent(const CMStringA &szEmail);
	bool   IsContactChatAgent(MCONTACT hContact);

	MCONTACT MraHContactFromEmail(const CMStringA &szEmail, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded);
	bool   MraUpdateContactInfo(MCONTACT hContact);
	DWORD  MraSetXStatusInternal(DWORD dwXStatus);
	DWORD  MraGetContactStatus(MCONTACT hContact);
	DWORD  MraSetContactStatus(MCONTACT hContact, DWORD dwNewStatus);
	DWORD  MraContactCapabilitiesGet(MCONTACT hContact);
	void   MraContactCapabilitiesSet(MCONTACT hContact, DWORD dwFutureFlags);
	void   MraUpdateEmailStatus(const CMStringA &szFrom, const CMStringA &szSubject, bool force_display);
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

	void   SetExtraIcons(MCONTACT hContact);

	void   InitMenus(void);
	HGENMENU CListCreateMenu(LONG lPosition, LONG lPopupPosition, BOOL bIsStatus, const IconItem *pgdiItems, size_t dwCount, HGENMENU *hResult);

	DWORD  ProtoBroadcastAckAsync(MCONTACT hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam);
	CMStringA CreateBlobFromContact(MCONTACT hContact, const CMStringW &wszRequestReason);

	bool   GetPassDB(CMStringA& pass);

	bool   MraRequestXStatusDetails(DWORD dwXStatus);
	bool   MraSendReplyBlogStatus(MCONTACT hContact);
	DWORD  MraSelectEMailDlgShow(MCONTACT hContact, DWORD dwType);

	DWORD	 MraMrimProxyConnect(HANDLE hMraMrimProxyData, HANDLE *phConnection);

	DWORD  MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, const CMStringA &szUrl);
	DWORD  MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, const CMStringA &, CMStringA &szEmail);
	void   MraMPopSessionQueueStart(HANDLE hMPopSessionQueue);
	void   MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue);

	size_t MraFilesQueueGetLocalAddressesList(LPSTR lpszBuff, size_t dwBuffSize, DWORD dwPort);
	DWORD	 MraFilesQueueAddReceive(HANDLE hFilesQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD dwIDRequest, const CMStringW &wszFiles, const CMStringA &szAddresses);
	DWORD  MraFilesQueueAddSend(HANDLE hFilesQueueHandle, DWORD dwFlags, MCONTACT hContact, LPWSTR *plpwszFiles, size_t dwFilesCount, DWORD *pdwIDRequest);
	DWORD  MraFilesQueueCancel(HANDLE hFilesQueueHandle, DWORD dwIDRequest, BOOL bSendDecline);
	DWORD  MraFilesQueueStartMrimProxy(HANDLE hFilesQueueHandle, DWORD dwIDRequest);
	DWORD  MraFilesQueueSendMirror(HANDLE hFilesQueueHandle, DWORD dwIDRequest, const CMStringA &szAddresses);
	bool   MraFilesQueueHandCheck(HANDLE hConnection, MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	HANDLE MraFilesQueueConnectOut(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	HANDLE MraFilesQueueConnectIn(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	DWORD  MraFilesQueueAccept(HANDLE hFilesQueueHandle, DWORD dwIDRequest, LPCWSTR lpwszPath, size_t dwPathSize);

	void   __cdecl MraFilesQueueRecvThreadProc(LPVOID lpParameter);
	void   __cdecl MraFilesQueueSendThreadProc(LPVOID lpParameter);

	bool    bChatExists;
	void    MraChatDllError();
	bool    MraChatRegister();
	INT_PTR MraChatSessionNew(MCONTACT hContactChatSession);
	void    MraChatSessionDestroy(MCONTACT hContactChatSession);
	void    MraChatSendPrivateMessage(LPSTR lpwszEMail);

	INT_PTR MraChatSessionEventSendByHandle(MCONTACT hContactChatSession, DWORD dwType, DWORD dwFlags, const CMStringA &szUID, LPCWSTR lpwszStatus, LPCWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime);
	INT_PTR MraChatSessionInvite(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionMembersAdd(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionJoinUser(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionLeftUser(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime);
	INT_PTR MraChatSessionSetIviter(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat);
	INT_PTR MraChatSessionSetOwner(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat);
	INT_PTR MraChatSessionMessageAdd(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, const CMStringW &wszMessage, DWORD dwTime);

	DWORD   MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle);
	void    MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle);
	void    MraAvatarsQueueSuspend(HANDLE hAvatarsQueueHandle);
	void    MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle);
	DWORD   MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD *pdwAvatarsQueueID);
	bool    MraAvatarsGetContactTime	(MCONTACT hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	void    MraAvatarsSetContactTime	(MCONTACT hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	DWORD   MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle, MCONTACT hContact, DWORD dwFormat, CMStringW &res);
	DWORD   MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD *pdwAvatarsQueueID, DWORD *pdwFormat, LPTSTR lpszPath);
	DWORD   MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle, DWORD dwFlags, MCONTACT hContact);
	DWORD   MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle, MCONTACT hContact);

	void    __cdecl MraAvatarsThreadProc(LPVOID lpParameter);
};
