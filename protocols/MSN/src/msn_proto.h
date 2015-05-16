/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2009-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MSN_PROTO_H_
#define _MSN_PROTO_H_

#include <m_protoint.h>

struct CMsnProto : public PROTO<CMsnProto>
{
	CMsnProto(const char*, const TCHAR*);
	~CMsnProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);

	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR* szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const TCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//====| Services |====================================================================
	INT_PTR  __cdecl SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl GetAvatarInfo(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetMyAwayMsg(WPARAM wParam,LPARAM lParam);

	INT_PTR  __cdecl GetAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SetAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetAvatarCaps(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl GetCurrentMedia(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SetCurrentMedia(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl SetNickName(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SendNudge(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl GetUnreadEmailCount(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl ManageAccount(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl OnLeaveChat(WPARAM wParam, LPARAM lParam);

	//====| Events |======================================================================
	int  __cdecl OnContactDeleted(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnIdleChanged(WPARAM wParam, LPARAM lParam);
	int  __cdecl OnGroupChange(WPARAM wParam, LPARAM lParam);
	int  __cdecl OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	int  __cdecl OnOptionsInit(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnPrebuildContactMenu(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnPreShutdown(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnContactDoubleClicked(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnDbSettingChanged(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnUserInfoInit(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnWindowEvent(WPARAM wParam, LPARAM lParam);
	int  __cdecl OnWindowPopup(WPARAM wParam, LPARAM lParam);

	//====| Data |========================================================================

	// Security Tokens
	char *pAuthToken, *tAuthToken;
	char *oimSendToken;
	char *authStrToken, *authSecretToken;
	char *authContactToken;
	char *authStorageToken;
	char *hotSecretToken, *hotAuthToken;
	char *authUser, *authUIC, *authCookies, *authSSLToken, *authAccessToken;
	int  authMethod;
	time_t authTokenExpiretime;
	bool bSentBND;

	char *abCacheKey, *sharingCacheKey, *storageCacheKey;

	mir_cs m_csLists;
	OBJLIST<MsnContact> m_arContacts;

	LIST<ServerGroupItem> m_arGroups;

	mir_cs m_csThreads;
	OBJLIST<ThreadData> m_arThreads;
	LIST<GCThreadData> m_arGCThreads;

	mir_cs m_csSessions;
	OBJLIST<filetransfer> m_arSessions;
	OBJLIST<directconnection> m_arDirect;

	mir_cs csMsgQueue;
	int msgQueueSeq;
	OBJLIST<MsgQueueEntry> lsMessageQueue;

	mir_cs csAvatarQueue;
	LIST<AvatarQueueEntry> lsAvatarQueue;
	HANDLE hevAvatarQueue;

	LONG m_chatID;

	int msnPingTimeout;
	HANDLE hKeepAliveThreadEvt;

	char* msnModeMsgs[MSN_NUM_MODES];

	LISTENINGTOINFO     msnCurrentMedia;
	MYOPTIONS			MyOptions;
	MyConnectionType	MyConnection;

	ThreadData*	msnNsThread;
	bool        msnLoggedIn;
	bool        usingGateway;

	char*       msnExternalIP;
	char*		msnRegistration;
	char*       msnPreviousUUX;
	char*       msnLastStatusMsg;

	char*       mailsoundname;
	char*       alertsoundname;

	unsigned    langpref;
	unsigned    emailEnabled;
	unsigned    abchMigrated;
	unsigned    myFlags;

	unsigned    msnOtherContactsBlocked;
	int			mUnreadMessages;
	int			mUnreadJunkEmails;
	clock_t		mHttpsTS;
	clock_t		mStatusMsgTS;

	HANDLE		msnSearchId;
	HANDLE		hNetlibUserHttps;
	HANDLE		hHttpsConnection;
	HANDLE		hMSNNudge;
	HANDLE      hPopupError, hPopupHotmail, hPopupNotify;

	HANDLE		hCustomSmileyFolder;
	bool        InitCstFldRan;
	bool        isConnectSuccess;
	bool        isIdle;

	void        InitCustomFolders(void);

	char*       getSslResult(char** parUrl, const char* parAuthInfo, const char* hdrs, unsigned& status);
	bool        getMyAvatarFile(char *url, TCHAR *fname);

	void        MSN_GoOffline(void);
	void        MSN_GetCustomSmileyFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen, const char* SmileyName, int Type);

	const char*	MirandaStatusToMSN(int status);
	WORD        MSNStatusToMiranda(const char *status);
	char**		GetStatusMsgLoc(int status);

	void        MSN_SendStatusMessage(const char* msg);
	void        MSN_SetServerStatus(int newStatus);
	void		MSN_FetchRecentMessages(time_t since = 0);
	void        MSN_StartStopTyping(GCThreadData* info, bool start);
	void        MSN_SendTyping(ThreadData* info, const char* email, int netId, bool bTyping );

	void        MSN_InitSB(ThreadData* info, const char* szEmail);
	void        MSN_ReceiveMessage(ThreadData* info, char* cmdString, char* params);
	int			MSN_HandleCommands(ThreadData* info, char* cmdString);
	int			MSN_HandleErrors(ThreadData* info, char* cmdString);
	void        MSN_ProcessNotificationMessage(char* buf, unsigned len);
	void		MSN_ProcessStatusMessage(ezxml_t xmli, const char* wlid);
	void		MSN_ProcessNLN(const char *userStatus, const char *wlid, char *userNick, const char *objid, char *cmdstring);
	void        MSN_ProcessPage(char* buf, unsigned len);
	void        MSN_ProcessRemove(char* buf, size_t len);
	void        MSN_ProcessAdd(char* buf, size_t len);
	void        MSN_ProcessYFind(char* buf, size_t len);
	void        MSN_CustomSmiley(const char* msgBody, char* email, char* nick, int iSmileyType);
	void        MSN_InviteMessage(ThreadData* info, char* msgBody, char* email, char* nick);
	void        MSN_SetMirVer(MCONTACT hContact, DWORD dwValue, bool always);

	void        LoadOptions(void);

	void        InitPopups(void);
	void        MSN_ShowPopup(const TCHAR* nickname, const TCHAR* msg, int flags, const char* url);
	void        MSN_ShowPopup(const MCONTACT hContact, const TCHAR* msg, int flags);
	void        MSN_ShowError(const char* msgtext, ...);

	void        MSN_SetNicknameUtf(const char* nickname);
	void        MSN_SendNicknameUtf(const char* nickname);

	typedef struct { TCHAR *szName; const char *szMimeType; unsigned char *data; size_t dataSize; } StoreAvatarData;
	void __cdecl msn_storeAvatarThread(void* arg);

	void __cdecl msn_storeProfileThread(void*);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN Connection properties detection

	void		DecryptEchoPacket(UDPProbePkt& pkt);
	void		MSNatDetect(void);

	void __cdecl MSNConnDetectThread(void*);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN menus

	HGENMENU mainMenuRoot;
	HGENMENU menuItemsMain[4];

	void MsnInitMainMenu(void);
	void MsnRemoveMainMenus(void);
	void MSN_EnableMenuItems(bool parEnable);
	void MsnInvokeMyURL(bool ismail, const char* url);

	INT_PTR __cdecl MsnBlockCommand(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MsnGotoInbox(WPARAM, LPARAM);
	INT_PTR __cdecl MsnSendHotmail(WPARAM wParam, LPARAM);
	INT_PTR __cdecl MsnEditProfile(WPARAM, LPARAM);
	INT_PTR __cdecl MsnInviteCommand(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MsnSendNetMeeting(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SetNicknameUI(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MsnViewProfile(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MsnSetupAlerts(WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN thread functions

	void __cdecl msn_keepAliveThread(void* arg);
	void __cdecl msn_loginThread(void* arg);
	void __cdecl MSNServerThread(void* arg);

	void __cdecl MsnFileAckThread(void* arg);
	void __cdecl MsnSearchAckThread(void* arg);
	void __cdecl sttFakeAvatarAck(void* arg);
	void __cdecl MsnFakeAck(void* arg);

	void __cdecl MsnGetAwayMsgThread(void* arg);

	void __cdecl p2p_sendFeedThread(void* arg );
	void __cdecl p2p_fileActiveThread(void* arg );
	void __cdecl p2p_filePassiveThread(void* arg);

	void __cdecl MsgQueue_AllClearThread(void* arg);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN thread support

	void         Threads_Uninit(void);
	void         MSN_CloseConnections(void);
	int          MSN_GetChatThreads(ThreadData** parResult);
	int          MSN_GetActiveThreads(ThreadData**);
	ThreadData*  MSN_GetThreadByConnection(HANDLE hConn);
	ThreadData*  MSN_GetThreadByContact(const char* wlid, TInfoType type = SERVER_SWITCHBOARD);
	GCThreadData*MSN_GetThreadByChatId(const TCHAR* chatId);
	ThreadData*  MSN_GetP2PThreadByContact(const char *wlid);
	void         MSN_StartP2PTransferByContact(const char* wlid);
	ThreadData*  MSN_GetThreadByPort(WORD wPort);
	ThreadData*  MSN_GetUnconnectedThread(const char* wlid, TInfoType type = SERVER_SWITCHBOARD);
	ThreadData*  MSN_GetOtherContactThread(ThreadData* thread);
	
	ThreadData*  MSN_StartSB(const char* uid, bool& isOffline);
	void __cdecl ThreadStub(void* arg);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN message queue support

	int         MsgQueue_Add(const char* wlid, int msgType, const char* msg, int msglen, filetransfer* ft = NULL, int flags = 0, STRLIST *cnt = NULL);
	const char* MsgQueue_CheckContact(const char* wlid, time_t tsc = 0);
	const char* MsgQueue_GetNextRecipient(void);
	bool        MsgQueue_GetNext(const char* wlid, MsgQueueEntry& retVal);
	int         MsgQueue_NumMsg(const char* wlid);
	void        MsgQueue_Clear(const char* wlid = NULL, bool msg = false);

	void MsgQueue_Init(void);
	void MsgQueue_Uninit(void);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN message reassembly support

	OBJLIST<chunkedmsg> msgCache;

	int   addCachedMsg(const char* id, const char* msg, const size_t offset,
						const size_t portion, const size_t totsz, const bool bychunk);
	size_t getCachedMsgSize(const char* id);
	bool  getCachedMsg(const int idx, char*& msg, size_t& size);
	bool  getCachedMsg(const char* id, char*& msg, size_t& size);
	void  clearCachedMsg(int idx = -1);
	void  CachedMsg_Uninit(void);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN P2P session support

	void  p2p_clearDormantSessions(void);
	void  p2p_cancelAllSessions(void);
	void  p2p_redirectSessions(const char* wlid);
	void  p2p_startSessions(const char* wlid);
	void  p2p_clearThreadSessions(MCONTACT hContact, TInfoType mType);

	void  p2p_invite(unsigned iAppID, filetransfer* ft, const char *wlid);
	void  p2p_inviteDc(filetransfer* ft, const char *wlid);
	void  p2p_processMsg(ThreadData* info, char* msgbody, const char* wlid);
	void  p2p_processMsgV2(ThreadData* info, char* msgbody, const char* wlid);
	void  p2p_processSIP(ThreadData* info, char* msgbody, P2PB_Header* hdr, const char* wlid);

	void  p2p_AcceptTransfer(MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid);
	void  p2p_InitDirectTransfer(MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid);
	void  p2p_InitDirectTransfer2(MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid);
	void  p2p_InitFileTransfer(ThreadData* info, MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid);
	void  p2p_pictureTransferFailed(filetransfer* ft);
	void  p2p_savePicture2disk(filetransfer* ft);

	bool  p2p_createListener(filetransfer* ft, directconnection *dc, MimeHeaders& chdrs);
	void  p2p_startConnect(const char* wlid, const char* szCallID, const char* addr, const char* port, bool ipv6);

	void  p2p_sendAbortSession(filetransfer* ft);
	void  p2p_sendAck(const char *wlid, P2PB_Header* hdrdata);
	void  p2p_sendAvatarInit(filetransfer* ft);
	void  p2p_sendBye(filetransfer* ft);
	void  p2p_sendCancel(filetransfer* ft);
	void  p2p_sendMsg(const char *wlid, unsigned appId, P2PB_Header& hdrdata, char* msgbody, size_t msgsz);
	void  p2p_sendMsg(ThreadData* info, const char *wlid, unsigned appId, P2PB_Header& hdrdata, char* msgbody, size_t msgsz);
	void  p2p_sendNoCall(filetransfer* ft);
	void  p2p_sendSlp(int iKind, filetransfer *ft, MimeHeaders &pHeaders, MimeHeaders &pContent, const char *wlid = NULL);
	void  p2p_sendRedirect(filetransfer* ft);
	void  p2p_sendStatus(filetransfer* ft, long lStatus);

	void  p2p_sendFeedStart(filetransfer* ft);
	LONG  p2p_sendPortion(filetransfer* ft, ThreadData* T, bool isV2);
	void  p2p_sendRecvFileDirectly(ThreadData* info);
	bool  p2p_connectTo(ThreadData* info, directconnection *dc);
	bool  p2p_listen(ThreadData* info, directconnection *dc);

	void  p2p_registerSession(filetransfer* ft);
	void  p2p_unregisterSession(filetransfer* ft);
	void  p2p_sessionComplete(filetransfer* ft);

	void P2pSessions_Uninit(void);

	filetransfer*  p2p_getAvatarSession(MCONTACT hContact);
	filetransfer*  p2p_getThreadSession(MCONTACT hContact, TInfoType mType);
	filetransfer*  p2p_getSessionByID(unsigned id);
	filetransfer*  p2p_getSessionByUniqueID(unsigned id);
	filetransfer*  p2p_getSessionByCallID(const char* CallID, const char* wlid);

	bool     p2p_sessionRegistered(filetransfer* ft);
	bool     p2p_isAvatarOnly(MCONTACT hContact);
	unsigned p2p_getMsgId(const char* wlid, int inc);
	unsigned p2p_getPktNum(const char* wlid);

	void  p2p_registerDC(directconnection* ft);
	void  p2p_unregisterDC(directconnection* dc);
	directconnection*  p2p_getDCByCallID(const char* CallID, const char* wlid);

	/////////////////////////////////////////////////////////////////////////////////////////
	// MSN MSNFTP file transfer

	void msnftp_invite(filetransfer *ft);
	void msnftp_sendAcceptReject(filetransfer *ft, bool acc);
	void msnftp_startFileSend(ThreadData* info, const char* Invcommand, const char* Invcookie);

	int  MSN_HandleMSNFTP(ThreadData *info, char *cmdString);

	void __cdecl msnftp_sendFileThread(void* arg);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN Chat support

	int  MSN_ChatInit(GCThreadData *info, const char *pszID, const char *pszTopic);
	void MSN_ChatStart(ezxml_t xmli);
	void MSN_KillChatSession(const TCHAR* id);
	void MSN_Kickuser(GCHOOK *gch);
	void MSN_Promoteuser(GCHOOK *gch, const char *pszRole);
	const TCHAR *MSN_GCGetRole(GCThreadData* thread, const char *pszWLID);
	void MSN_GCProcessThreadActivity(ezxml_t xmli, const TCHAR *mChatID);
	void MSN_GCAddMessage(TCHAR *mChatID, MCONTACT hContact, char *email, time_t ts, bool sentMsg, char *msgBody);
	void MSN_GCRefreshThreadsInfo(void);

	MCONTACT MSN_GetChatInernalHandle(MCONTACT hContact);

	int __cdecl MSN_GCEventHook(WPARAM wParam, LPARAM lParam);
	int __cdecl MSN_GCMenuHook(WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN contact list

	int      Lists_Add(int list, int netId, const char* email, MCONTACT hContact = NULL, const char* nick = NULL, const char* invite = NULL);
	bool     Lists_IsInList(int list, const char* email);
	int      Lists_GetMask(const char* email);
	int      Lists_GetNetId(const char* email);
	void     Lists_Remove(int list, const char* email);
	void     Lists_Populate(void);
	void     Lists_Wipe(void);

	MsnContact* Lists_Get(const char* email);
	MsnContact* Lists_Get(MCONTACT hContact);
	MsnContact* Lists_GetNext(int& i);

	MsnPlace* Lists_GetPlace(const char* wlid);
	MsnPlace* Lists_AddPlace(const char* email, const char* id, unsigned cap1, unsigned cap2);

	void     Lists_Uninit(void);

	void     AddDelUserContList(const char* email, const int list, const int netId, const bool del);

	void     MSN_CreateContList(void);
	void     MSN_CleanupLists(void);
	void     MSN_FindYahooUser(const char* email);
	bool     MSN_RefreshContactList(void);

	bool     MSN_IsMyContact(MCONTACT hContact);
	bool     MSN_IsMeByContact(MCONTACT hContact, char* szEmail  = NULL);
	bool     MSN_AddUser(MCONTACT hContact, const char* email, int netId, int flags, const char *msg = NULL);
	void     MSN_AddAuthRequest(const char *email, const char *nick, const char *reason);
	void     MSN_SetContactDb(MCONTACT hContact, const char *szEmail);
	MCONTACT MSN_HContactFromEmail(const char* msnEmail, const char* msnNick = NULL, bool addIfNeeded = false, bool temporary = false);
	MCONTACT MSN_HContactFromChatID(const char* wlid);
	MCONTACT AddToListByEmail(const char *email, const char *nick, DWORD flags);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN server groups

	void     MSN_AddGroup(const char* pName, const char* pId, bool init);
	void     MSN_DeleteGroup(const char* pId);
	void     MSN_FreeGroups(void);
	LPCSTR   MSN_GetGroupById(const char* pId);
	LPCSTR   MSN_GetGroupByName(const char* pName);
	void     MSN_SetGroupName(const char* pId, const char* pName);

	void     MSN_MoveContactToGroup(MCONTACT hContact, const char* grpName);
	void     MSN_RenameServerGroup(LPCSTR szId, const char* newName);
	void     MSN_DeleteServerGroup(LPCSTR szId);
	void     MSN_RemoveEmptyGroups(void);
	void     MSN_SyncContactToServerGroup(MCONTACT hContact, const char* szContId, ezxml_t cgrp);
	void     MSN_UploadServerGroups(char* group);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN Authentication

	int       MSN_GetPassportAuth(void);
	int       MSN_SkypeAuth(const char *pszNonce, char *pszUIC);
	int       MSN_DoOAuth(void);
	char*     GenerateLoginBlob(char* challenge);
	void      LoadAuthTokensDB(void);
	void      SaveAuthTokensDB(void);
	int       LoginSkypeOAuth(const char *pRefreshToken);
	bool      RefreshOAuth(const char *pszRefreshToken, const char *pszService, char *pszAccessToken, char *pszOutRefreshToken=NULL, time_t *ptExpires=NULL);
	int       MSN_AuthOAuth(void);
	CMStringA HotmailLogin(const char* url);
	void	    FreeAuthTokens(void);
	int       GetMyNetID(void);
	LPCSTR    GetMyUsername(int netId);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN avatars support

	void   AvatarQueue_Init(void);
	void   AvatarQueue_Uninit(void);

	void   MSN_GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen, const TCHAR *ext);
	int    MSN_SetMyAvatar(const TCHAR* szFname, void* pData, size_t cbLen);

	void   __cdecl MSN_AvatarsThread(void*);

	void   pushAvatarRequest(MCONTACT hContact, LPCSTR pszUrl);
	bool   loadHttpAvatar(AvatarQueueEntry *p);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN Mail & Offline messaging support

	bool nickChg;

	void getMetaData(void);
	void getOIMs(ezxml_t xmli);
	ezxml_t oimRecvHdr(const char* service, ezxml_t& tbdy, char*& httphdr);

	void processMailData(char* mailData);
	void sttNotificationMessage(char* msgBody, bool isInitial);
	void displayEmailCount(MCONTACT hContact);

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN SOAP Address Book

	bool MSN_SharingFindMembership(bool deltas = false, bool allowRecurse = true);
	bool MSN_SharingAddDelMember(const char* szEmail, const int listId, const int netId, const char* szMethod, bool allowRecurse = true);
	bool MSN_SharingMyProfile(bool allowRecurse = true);
	bool MSN_ABAdd(bool allowRecurse = true);
	bool MSN_ABFind(const char* szMethod, const char* szGuid, bool deltas = false, bool allowRecurse = true);
	bool MSN_ABAddDelContactGroup(const char* szCntId, const char* szGrpId, const char* szMethod, bool allowRecurse = true);
	void MSN_ABAddGroup(const char* szGrpName, bool allowRecurse = true);
	void MSN_ABRenameGroup(const char* szGrpName, const char* szGrpId, bool allowRecurse = true);
	void MSN_ABUpdateNick(const char* szNick, const char* szCntId);
	void MSN_ABUpdateAttr(const char* szCntId, const char* szAttr, const char* szValue, bool allowRecurse = true);
	bool MSN_ABUpdateProperty(const char* szCntId, const char* propName, const char* propValue, bool allowRecurse = true);
	bool MSN_ABAddRemoveContact(const char* szCntId, int netId, bool add, bool allowRecurse = true);
	unsigned MSN_ABContactAdd(const char* szEmail, const char* szNick, int netId, const char* szInvite, bool search, bool retry = false, bool allowRecurse = true);
	void MSN_ABUpdateDynamicItem(bool allowRecurse = true);
	bool MSN_ABRefreshClist(void);

	ezxml_t abSoapHdr(const char* service, const char* scenario, ezxml_t& tbdy, char*& httphdr);
	char* GetABHost(const char* service, bool isSharing);
	void SetAbParam(MCONTACT hContact, const char *name, const char *par);
	void UpdateABHost(const char* service, const char* url);
	void UpdateABCacheKey(ezxml_t bdy,  bool isSharing);

	ezxml_t getSoapResponse(ezxml_t bdy, const char* service);
	ezxml_t getSoapFault(ezxml_t bdy, bool err);

	char mycid[32];
	char mypuid[32];

	/////////////////////////////////////////////////////////////////////////////////////////
	//	MSN SOAP Roaming Storage

	bool MSN_StoreGetProfile(bool allowRecurse = true);
	bool MSN_StoreUpdateProfile(const char* szNick, const char* szStatus, bool lock, bool allowRecurse = true);
	bool MSN_StoreCreateProfile(bool allowRecurse = true);
	bool MSN_StoreShareItem(const char* id, bool allowRecurse = true);
	bool MSN_StoreCreateRelationships(bool allowRecurse = true);
	bool MSN_StoreDeleteRelationships(bool tile, bool allowRecurse = true);
	bool MSN_StoreCreateDocument(const TCHAR *sztName, const char *szMimeType, const char *szPicData, bool allowRecurse = true);
	bool MSN_StoreUpdateDocument(const TCHAR *sztName, const char *szMimeType, const char *szPicData, bool allowRecurse = true);
	bool MSN_StoreFindDocuments(bool allowRecurse = true);

	ezxml_t storeSoapHdr(const char* service, const char* scenario, ezxml_t& tbdy, char*& httphdr);
	char* GetStoreHost(const char* service);
	void UpdateStoreHost(const char* service, const char* url);
	void UpdateStoreCacheKey(ezxml_t bdy);

	char proresid[64];
	char expresid[64];
	char photoid[64];

	//////////////////////////////////////////////////////////////////////////////////////

	TCHAR *m_DisplayNameCache;
	TCHAR* GetContactNameT(MCONTACT hContact);

	int    getStringUtf(MCONTACT hContact, const char* name, DBVARIANT* result);
	int    getStringUtf(const char* name, DBVARIANT* result);
	void   setStringUtf(MCONTACT hContact, const char* name, const char* value);
};

extern OBJLIST<CMsnProto> g_Instances;

#endif
