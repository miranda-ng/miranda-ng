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

struct CMraProto;
typedef void    (__cdecl CMraProto::*ThreadFunc)(void*);
typedef int     (__cdecl CMraProto::*EventFunc)(WPARAM, LPARAM);
typedef INT_PTR (__cdecl CMraProto::*ServiceFunc)(WPARAM, LPARAM);
typedef INT_PTR (__cdecl CMraProto::*ServiceFuncParam)(WPARAM, LPARAM, LPARAM);

BOOL	DB_GetStaticStringA(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize);
BOOL	DB_GetStaticStringW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPWSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize);

BOOL	DB_SetStringExA(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPCSTR lpszValue, size_t dwValueSize);
BOOL	DB_SetStringExW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPCWSTR lpwszValue, size_t dwValueSize);

int   DB_WriteContactSettingBlob(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPVOID lpValue, size_t dwValueSize);
BOOL  DB_GetContactSettingBlob(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPVOID lpRet, size_t dwRetBuffSize, size_t *pdwRetBuffSize);

struct CMraProto : public PROTO_INTERFACE
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

	void   CreateObjectSvc(const char* szService, ServiceFunc serviceProc);
	void   CreateObjectSvcParam(const char* szService, ServiceFuncParam serviceProc, LPARAM lParam);
	HANDLE CreateHookableEvent(const char* szService);
	void   ForkThread(ThreadFunc, void*);
	HANDLE ForkThreadEx(ThreadFunc, void*, UINT* threadID = NULL);
	HANDLE HookEvent(const char*, EventFunc);

	void   ShowFormattedErrorMessage(LPWSTR lpwszErrText, DWORD dwErrorCode);
	void   MraPopupShowW(HANDLE hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszTitle, LPWSTR lpszMessage);
	void   MraPopupShowFromContactW(HANDLE hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszMessage);
	__forceinline void MraPopupShowFromAgentW(DWORD dwType, DWORD dwFlags, LPWSTR lpszMessage) {
		MraPopupShowFromContactW(NULL, dwType, dwFlags, lpszMessage); }

	__forceinline void mraDelValue(HANDLE Contact, const char *valueName) {
		db_unset(Contact, m_szModuleName, valueName); }

	__forceinline DWORD mraGetDword(HANDLE Contact, const char *valueName, DWORD parDefltValue) {
		return db_get_dw(Contact, m_szModuleName, valueName, parDefltValue); }
	__forceinline void mraSetDword(HANDLE Contact, const char *valueName, DWORD parValue) {
		db_set_dw(Contact, m_szModuleName, valueName, parValue); }

	__forceinline WORD mraGetWord(HANDLE Contact, const char *valueName, WORD parDefltValue) {
		return db_get_w(Contact, m_szModuleName, valueName, parDefltValue); }
	__forceinline void mraSetWord(HANDLE Contact, const char *valueName, WORD parValue) {
		db_set_w(Contact, m_szModuleName, valueName, parValue); }

	__forceinline BYTE mraGetByte(HANDLE Contact, const char *valueName, BYTE parDefltValue) {
		return db_get_b(Contact, m_szModuleName, valueName, parDefltValue); }
	__forceinline void mraSetByte(HANDLE Contact, const char *valueName, BYTE parValue) {
		db_set_b(Contact, m_szModuleName, valueName, parValue); }

	__forceinline BOOL mraGetStaticStringA(HANDLE Contact, const char *ValueName, char *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringA(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }
	__forceinline BOOL mraGetStaticStringW(HANDLE Contact, const char *ValueName, WCHAR *Ret, size_t RetBuffSize, size_t *pRetBuffSize) {
		return DB_GetStaticStringW(Contact, m_szModuleName, ValueName, Ret, RetBuffSize, pRetBuffSize); }

	__forceinline BOOL mraSetStringA(HANDLE Contact, const char *valueName, const char *parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue, lstrlenA(parValue)); }
	__forceinline BOOL mraSetStringW(HANDLE Contact, const char *valueName, const WCHAR *parValue) {
		return db_set_ws(Contact, m_szModuleName, valueName, parValue); }

	__forceinline BOOL mraSetLPSStringA(HANDLE Contact, const char *valueName, MRA_LPS *parValue) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue->lpszData, parValue->dwSize); }
	__forceinline BOOL mraSetLPSStringW(HANDLE Contact, const char *valueName, MRA_LPS *parValue) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue->lpwszData, parValue->dwSize/sizeof(WCHAR)); }

	__forceinline BOOL mraSetStringExA(HANDLE Contact, const char *valueName, const char *parValue, size_t parValueSize) {
		return DB_SetStringExA(Contact, m_szModuleName, valueName, parValue, parValueSize); }
	__forceinline BOOL mraSetStringExW(HANDLE Contact, const char *valueName, const WCHAR *parValue, size_t parValueSize) {
		return DB_SetStringExW(Contact, m_szModuleName, valueName, parValue, parValueSize); }

	__forceinline BOOL mraWriteContactSettingBlob(HANDLE hContact, const char *lpszValueName, LPVOID lpbValue, size_t dwValueSize) {
		return DB_WriteContactSettingBlob(hContact, m_szModuleName, lpszValueName, lpbValue, dwValueSize); }
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

	int __cdecl MraChatGcEventHook(WPARAM, LPARAM);

	int __cdecl MraExtraIconsApply(WPARAM, LPARAM);

	int __cdecl MraContactDeleted(WPARAM, LPARAM);
	int __cdecl MraDbSettingChanged(WPARAM, LPARAM);
	int __cdecl MraRebuildContactMenu(WPARAM, LPARAM);
	int __cdecl MraRebuildStatusMenu(WPARAM, LPARAM);
	int __cdecl MraMusicChanged(WPARAM, LPARAM);

	// ====| Data  |========================================================================
	bool   m_bLoggedIn;

	HANDLE  hSendQueueHandle, hFilesQueueHandle, hMPopSessionQueue;

	HANDLE  hNetlibUser, heNudgeReceived, hHookExtraIconsApply;
	HANDLE  hThreadWorker;
	HANDLE  hConnection;
	DWORD   dwThreadWorkerLastPingTime;
	DWORD   dwThreadWorkerRunning;
	DWORD   dwCMDNum;

	HANDLE  hAvatarsQueueHandle;

	HGENMENU hMainMenuItems[MAIN_MENU_ITEMS_COUNT+4];
	HGENMENU hContactMenuItems[CONTACT_MENU_ITEMS_COUNT+4];

	HWND     hWndEMailPopupStatus;
	DWORD    dwEmailMessagesTotal, dwEmailMessagesUnread;

	BOOL     bHideXStatusUI;
	HGENMENU hXStatusMenuItems[MRA_XSTATUS_COUNT+4];
	HANDLE   hExtraXstatusIcon;
	HANDLE   hExtraInfo;

	char    szNewMailSound[MAX_PATH];

	CRITICAL_SECTION csCriticalSectionSend;

	HANDLE  AddToListByEmail(LPCTSTR plpsEMail, LPCTSTR plpsNick, LPCTSTR plpsFirstName, LPCTSTR plpsLastName, DWORD dwFlags);

	DWORD  MraMessageW(BOOL bAddToQueue, HANDLE hContact, DWORD dwAckType, DWORD dwFlags, LPSTR lpszEMail, size_t dwEMailSize, LPCWSTR lpwszMessage, size_t dwMessageSize, LPBYTE lpbMultiChatData, size_t dwMultiChatDataSize);
	DWORD  MraMessageAskW(DWORD dwMsgID, DWORD dwFlags, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszMessage, size_t dwMessageSize, LPSTR lpwszMessageRTF, size_t dwMessageRTFSize);
	DWORD  MraMessageRecv(LPSTR lpszFrom, size_t dwFromSize, DWORD dwMsgID);
	DWORD  MraAddContactW(HANDLE hContact, DWORD dwContactFlag, DWORD dwGroupID, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszCustomName, size_t dwCustomNameSize, LPSTR lpszPhones, size_t dwPhonesSize, LPWSTR lpwszAuthMessage, size_t dwAuthMessageSize, DWORD dwActions);
	DWORD  MraModifyContactW(HANDLE hContact, DWORD dwID, DWORD dwContactFlag, DWORD dwGroupID, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszCustomName, size_t dwCustomNameSize, LPSTR lpszPhones, size_t dwPhonesSize);
	DWORD  MraOfflineMessageDel(DWORDLONG dwMsgUIDL);
	DWORD  MraAuthorize(LPSTR lpszEMail, size_t dwEMailSize);
	DWORD  MraChangeStatusW(DWORD dwStatus, LPSTR lpszStatusUri, size_t dwStatusUriSize, LPCWSTR lpwszStatusTitle, size_t dwStatusTitleSize, LPCWSTR lpwszStatusDesc, size_t dwStatusDescSize, DWORD dwFutureFlags);
	DWORD  MraFileTransfer(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIdRequest, DWORD dwFilesTotalSize, LPWSTR lpwszFiles, size_t dwFilesSize, LPSTR lpszAddreses, size_t dwAddresesSize);
	DWORD  MraFileTransferAck(DWORD dwStatus, LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIdRequest, LPBYTE lpbDescription, size_t dwDescriptionSize);
	HANDLE MraWPRequestW(HANDLE hContact, DWORD dwAckType, DWORD dwRequestFlags, LPSTR lpszUser, size_t dwUserSize, LPSTR lpszDomain, size_t dwDomainSize, LPCWSTR lpwszNickName, size_t dwNickNameSize, LPCWSTR lpwszFirstName, size_t dwFirstNameSize, LPCWSTR lpwszLastName, size_t dwLastNameSize, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline);
	HANDLE MraWPRequestA(HANDLE hContact, DWORD dwAckType, DWORD dwRequestFlags, LPSTR lpszUser, size_t dwUserSize, LPSTR lpszDomain, size_t dwDomainSize, LPSTR lpszNickName, size_t dwNickNameSize, LPSTR lpszFirstName, size_t dwFirstNameSize, LPSTR lpszLastName, size_t dwLastNameSize, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline);
	HANDLE MraWPRequestByEMail(HANDLE hContact, DWORD dwAckType, LPCSTR lpszEMail, size_t dwEMailSize);
	DWORD  MraGame(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwGameSessionID, DWORD dwGameMsg, DWORD dwGameMsgID, LPSTR lpszData, size_t dwDataSize);
	DWORD  MraLogin2W(LPSTR lpszLogin, size_t dwLoginSize, LPSTR lpszPassword, size_t dwPasswordSize, DWORD dwStatus, LPSTR lpszStatusUri, size_t dwStatusUriSize, LPWSTR lpwszStatusTitle, size_t dwStatusTitleSize, LPWSTR lpwszStatusDesc, size_t dwStatusDescSize, DWORD dwFutureFlags, LPSTR dwUserAgentFormatted, size_t dwUserAgentFormattedSize, LPSTR lpszUserAgent, size_t dwUserAgentSize);
	DWORD  MraSMSW(HANDLE hContact, LPSTR lpszPhone, size_t dwPhoneSize, LPWSTR lpwszMessage, size_t dwMessageSize);
	DWORD  MraProxy(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, DWORD dwDataType, LPSTR lpszData, size_t dwDataSize, LPSTR lpszAddreses, size_t dwAddresesSize, MRA_GUID mguidSessionID);
	DWORD  MraProxyAck(DWORD dwStatus, LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, DWORD dwDataType, LPSTR lpszData, size_t dwDataSize, LPSTR lpszAddreses, size_t dwAddresesSize, MRA_GUID mguidSessionID);
	DWORD  MraChangeUserBlogStatus(DWORD dwFlags, LPWSTR lpwszText, size_t dwTextSize, DWORDLONG dwBlogStatusID);

	DWORD  MraSendPacket(HANDLE hConnection, DWORD dwCMDNum, DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendCMD(DWORD dwType, LPVOID lpData, size_t dwDataSize);
	DWORD  MraSendQueueCMD(HANDLE hSendQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwAckType, LPBYTE lpbDataQueue, size_t dwDataQueueSize, DWORD dwType, LPVOID lpData, size_t dwDataSize);

	DWORD  MraSendNewStatus(DWORD dwStatusMir, DWORD dwXStatusMir, LPCWSTR lpwszStatusTitle, size_t dwStatusTitleSize, LPCWSTR lpwszStatusDesc, size_t dwStatusDescSize);

	void   MraAddrListStoreToContact(HANDLE hContact, MRA_ADDR_LIST *pmalAddrList);

	DWORD  GetContactFlags(HANDLE hContact);
	DWORD  SetContactFlags(HANDLE hContact, DWORD dwContactFlag);
	DWORD  GetContactBasicInfoW(HANDLE hContact, DWORD *pdwID, DWORD *pdwGroupID, DWORD *pdwContactFlag, DWORD *pdwContactSeverFlags, DWORD *pdwStatus, LPSTR lpszEMail, size_t dwEMailSize, size_t *pdwEMailSize, LPWSTR lpwszNick, size_t dwNickSize, size_t *pdwNickSize, LPSTR lpszPhones, size_t dwPhonesSize, size_t *pdwPhonesSize);
	DWORD  SetContactBasicInfoW(HANDLE hContact, DWORD dwSetInfoFlags, DWORD dwFlags, DWORD dwID, DWORD dwGroupID, DWORD dwContactFlag, DWORD dwContactSeverFlags, DWORD dwStatus, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszNick, size_t dwNickSize, LPSTR lpszPhones, size_t dwPhonesSize);

	DWORD  GetContactEMailCount(HANDLE hContact, BOOL bMRAOnly);
	BOOL   GetContactFirstEMail(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize);

	BOOL   IsContactMra(HANDLE hContact);
	BOOL   IsEMailMy(LPSTR lpszEMail, size_t dwEMailSize);
	BOOL   IsEMailChatAgent(LPSTR lpszEMail, size_t dwEMailSize);
	BOOL   IsContactChatAgent(HANDLE hContact);

	HANDLE MraHContactFromEmail(LPSTR lpszEMail, size_t dwEMailSize, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded);
	BOOL   MraUpdateContactInfo(HANDLE hContact);
	DWORD  MraSetXStatusInternal(DWORD dwXStatus);
	DWORD  MraGetContactStatus(HANDLE hContact);
	DWORD  MraSetContactStatus(HANDLE hContact, DWORD dwNewStatus);
	DWORD  MraContactCapabilitiesGet(HANDLE hContact);
	void   MraContactCapabilitiesSet(HANDLE hContact, DWORD dwFutureFlags);
	void   MraUpdateEmailStatus(LPSTR lpszFrom, size_t dwFromSize, LPSTR lpszSubject, size_t dwSubjectSize, DWORD dwDate, DWORD dwUIDL);
	DWORD  MraConvertToRTFW(LPCWSTR lpwszMessage, size_t dwMessageSize, LPSTR lpszMessageRTF, size_t dwMessageRTFBuffSize, size_t *pdwMessageRTFSize);

	void   MraThreadClean();
	DWORD  StartConnect();
	void   __cdecl MraThreadProc(LPVOID lpParameter);
	DWORD  MraGetNLBData(LPSTR lpszHost, size_t dwHostBuffSize, WORD *pwPort);
	DWORD  MraNetworkDispatcher();
	DWORD  MraCommandDispatcher(struct mrim_packet_header_t *pmaHeader, DWORD *pdwPingPeriod, DWORD *pdwNextPingSendTickTime, BOOL *pbContinue);

	DWORD  MraRecvCommand_Message(DWORD dwTime, DWORD dwFlags, MRA_LPS *plpsFrom, MRA_LPS *plpsText, MRA_LPS *plpsRFTText, MRA_LPS *plpsMultiChatData);

	void   SetExtraIcons(HANDLE hContact);

	#define	MESSAGE_NOT_SPAM	1
	#define	MESSAGE_SPAM		2

	DWORD  MraAntiSpamReceivedMessageW(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwMessageFlags, LPWSTR lpwszMessage, size_t dwMessageSize);
	BOOL   MraAntiSpamHasMessageBadWordsW(LPWSTR lpwszMessage, size_t dwMessageSize);

	void   InitMainMenu();
	void   InitContactMenu();
	void   CListCreateMenu(LONG lPosition, LONG lPopupPosition, HICON hMainIcon, LPSTR pszContactOwner, BOOL bIsStatus, const struct GUI_DISPLAY_ITEM *pgdiItems, size_t dwCount, HGENMENU *hResult);

	DWORD  ProtoBroadcastAckAsync(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam);
	DWORD  CreateBlobFromContact(HANDLE hContact, LPWSTR lpwszRequestReason, size_t dwRequestReasonSize, LPBYTE lpbBuff, size_t dwBuffSize, size_t *pdwBuffSizeRet);

	BOOL   SetPassDB(LPSTR lpszBuff, size_t dwBuffSize);
	BOOL   GetPassDB(LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSize);
	BOOL   GetPassDB_v1(LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSize);
	BOOL   GetPassDB_v2(LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSize);

	BOOL   MraRequestXStatusDetails(DWORD dwXStatus);
	BOOL   MraSendReplyBlogStatus(HANDLE hContact);
	DWORD  MraSelectEMailDlgShow(HANDLE hContact, DWORD dwType);

	DWORD	 MraMrimProxyConnect(HANDLE hMraMrimProxyData, HANDLE *phConnection);

	DWORD  MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue, LPSTR lpszUrl, size_t dwUrlSize);
	DWORD  MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue, LPSTR lpszUrl, size_t dwUrlSize, LPSTR lpszEMail, size_t dwEMailSize);
	DWORD  MraMPopSessionQueueStart(HANDLE hMPopSessionQueue);
	void   MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue);

	size_t MraFilesQueueGetLocalAddressesList(LPSTR lpszBuff, size_t dwBuffSize, DWORD dwPort);
	DWORD	 MraFilesQueueAddReceive(HANDLE hFilesQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwIDRequest, LPWSTR lpwszFiles, size_t dwFilesSize, LPSTR lpszAddreses, size_t dwAddresesSize);
	DWORD  MraFilesQueueAddSend(HANDLE hFilesQueueHandle, DWORD dwFlags, HANDLE hContact, LPWSTR *plpwszFiles, size_t dwFilesCount, DWORD *pdwIDRequest);
	DWORD  MraFilesQueueCancel(HANDLE hFilesQueueHandle, DWORD dwIDRequest, BOOL bSendDecline);
	DWORD  MraFilesQueueStartMrimProxy(HANDLE hFilesQueueHandle, DWORD dwIDRequest);
	DWORD  MraFilesQueueSendMirror(HANDLE hFilesQueueHandle, DWORD dwIDRequest, LPSTR lpszAddreses, size_t dwAddresesSize);
	BOOL   MraFilesQueueHandCheck(HANDLE hConnection, MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	HANDLE MraFilesQueueConnectOut(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	HANDLE MraFilesQueueConnectIn(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
	DWORD  MraFilesQueueAccept(HANDLE hFilesQueueHandle, DWORD dwIDRequest, LPCWSTR lpwszPath, size_t dwPathSize);

	void   __cdecl MraFilesQueueRecvThreadProc(LPVOID lpParameter);
	void   __cdecl MraFilesQueueSendThreadProc(LPVOID lpParameter);

	BOOL    bChatExists;
	void    MraChatDllError();
	BOOL    MraChatRegister();
	INT_PTR MraChatSessionNew(HANDLE hContactChatSession);
	void    MraChatSessionDestroy(HANDLE hContactChatSession);
	void    MraChatSendPrivateMessage(LPWSTR lpwszEMail);

	INT_PTR MraChatSessionEventSendByHandle(HANDLE hContactChatSession, DWORD dwType, DWORD dwFlags, LPSTR lpszUID, size_t dwUIDSize, LPWSTR lpwszStatus, LPWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime);
	INT_PTR MraChatSessionInvite(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize, DWORD dwTime);
	INT_PTR MraChatSessionMembersAdd(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize, DWORD dwTime);
	INT_PTR MraChatSessionJoinUser(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize, DWORD dwTime);
	INT_PTR MraChatSessionLeftUser(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize, DWORD dwTime);
	INT_PTR MraChatSessionSetIviter(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize);
	INT_PTR MraChatSessionSetOwner(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize);
	INT_PTR MraChatSessionMessageAdd(HANDLE hContactChatSession, LPSTR lpszEMailInMultiChat, size_t dwEMailInMultiChatSize, LPWSTR lpwszMessage, size_t dwMessageSize, DWORD dwTime);

	DWORD   MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle);
	void    MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle);
	void    MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle);
	DWORD   MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD *pdwAvatarsQueueID);
	BOOL    MraAvatarsGetContactTime	(HANDLE hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	void    MraAvatarsSetContactTime	(HANDLE hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime);
	DWORD   MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle, HANDLE hContact, DWORD dwFormat, LPTSTR lpszPath, size_t dwPathSize, size_t *pdwPathSizeRet);
	DWORD   MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD *pdwAvatarsQueueID, DWORD *pdwFormat, LPTSTR lpszPath);
   DWORD   MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwSourceID);
   DWORD   MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle, HANDLE hContact);

	void    __cdecl MraAvatarsThreadProc(LPVOID lpParameter);

};
