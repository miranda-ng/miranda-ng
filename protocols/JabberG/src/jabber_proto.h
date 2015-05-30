/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _JABBER_PROTO_H_
#define _JABBER_PROTO_H_

#include "jabber_disco.h"
#include "jabber_rc.h"
#include "jabber_privacy.h"
#include "jabber_search.h"
#include "jabber_iq.h"
#include "jabber_icolib.h"
#include "jabber_xstatus.h"
#include "jabber_notes.h"
#include "jabber_message_manager.h"
#include "jabber_presence_manager.h"
#include "jabber_send_manager.h"

struct CJabberProto;

enum TJabberGcLogInfoType { INFO_BAN, INFO_STATUS, INFO_CONFIG, INFO_AFFILIATION, INFO_ROLE };

typedef UNIQUE_MAP<TCHAR,TCharKeyCmp> U_TCHAR_MAP;

#define JABBER_DEFAULT_RECENT_COUNT 10

struct JABBER_GROUPCHAT_INVITE_INFO
{
	TCHAR *roomJid;
	TCHAR *from;
	TCHAR *reason;
	TCHAR *password;
};

struct ROSTERREQUSERDATA
{
	HWND hwndDlg;
	BYTE bRRAction;
	BOOL bReadyToDownload;
	BOOL bReadyToUpload;
};

struct TFilterInfo
{
	enum Type { T_JID, T_XMLNS, T_ANY, T_OFF };

	volatile BOOL msg, presence, iq;
	volatile Type type;

	mir_cs csPatternLock;
	TCHAR  pattern[256];
};

struct CJabberProto : public PROTO<CJabberProto>, public IJabberInterface
{
				CJabberProto(const char*, const TCHAR*);
				~CJabberProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);

	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR *szReason);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR *id);
	virtual	HANDLE    __cdecl SearchByEmail(const TCHAR *email);
	virtual	HANDLE    __cdecl SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char *msg);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const TCHAR *msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//====| Services |====================================================================
	INT_PTR  __cdecl SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetMyAwayMsg(WPARAM wParam, LPARAM lParam);

	//====| Events |======================================================================
	void __cdecl OnAddContactForever(DBCONTACTWRITESETTING* cws, MCONTACT hContact);
	int  __cdecl OnContactDeleted(WPARAM, LPARAM);
	int  __cdecl OnDbSettingChanged(WPARAM, LPARAM);
	int  __cdecl OnIdleChanged(WPARAM, LPARAM);
	int  __cdecl OnModernOptInit(WPARAM, LPARAM);
	int  __cdecl OnModulesLoadedEx(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnPreShutdown(WPARAM, LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM, LPARAM);
	int  __cdecl OnMsgUserTyping(WPARAM, LPARAM);
	int  __cdecl OnProcessSrmmIconClick(WPARAM, LPARAM);
	int  __cdecl OnProcessSrmmEvent(WPARAM, LPARAM);
	int  __cdecl OnReloadIcons(WPARAM, LPARAM);
	void __cdecl OnRenameContact(DBCONTACTWRITESETTING* cws, MCONTACT hContact);
	void __cdecl OnRenameGroup(DBCONTACTWRITESETTING* cws, MCONTACT hContact);
	int  __cdecl OnUserInfoInit(WPARAM, LPARAM);

	int  __cdecl JabberGcEventHook(WPARAM, LPARAM);
	int  __cdecl JabberGcMenuHook(WPARAM, LPARAM);

	//====| Data |========================================================================

	ThreadData* m_ThreadInfo;
	CJabberOptions m_options;

	PVOID  m_sslCtx;

	HANDLE m_hThreadHandle;

	TCHAR *m_szJabberJID;
	int    m_nJabberSearchID;
	time_t m_tmJabberLoggedInTime;
	time_t m_tmJabberIdleStartTime;
	UINT   m_nJabberCodePage;
	TCHAR *m_tszSelectedLang;

	mir_cs m_csModeMsgMutex;
	JABBER_MODEMSGS m_modeMsgs;

	bool   m_bJabberOnline; // XMPP connection initialized and we can send XMPP packets
	bool   m_bShutdown;
	bool   m_bSendKeepAlive;
	bool   m_bPepSupported;

	HWND   m_hwndAgentRegInput;
	HWND   m_hwndRegProgress;
	HWND   m_hwndJabberChangePassword;
	HWND   m_hwndMucVoiceList;
	HWND   m_hwndMucMemberList;
	HWND   m_hwndMucModeratorList;
	HWND   m_hwndMucBanList;
	HWND   m_hwndMucAdminList;
	HWND   m_hwndMucOwnerList;
	HWND   m_hwndJabberAddBookmark;
	HWND   m_hwndPrivacyRule;

	CJabberDlgBase *m_pDlgPrivacyLists;
	CJabberDlgBase *m_pDlgBookmarks;
	CJabberDlgBase *m_pDlgServiceDiscovery;
	CJabberDlgBase *m_pDlgJabberJoinGroupchat;
	CJabberDlgBase *m_pDlgNotes;

	// Service and event handles
	HANDLE m_hEventNudge;
	HANDLE m_hEventXStatusIconChanged;
	HANDLE m_hEventXStatusChanged;

	// Transports list
	LIST<TCHAR> m_lstTransports;

	CJabberIqManager m_iqManager;
	CJabberMessageManager m_messageManager;
	CJabberPresenceManager m_presenceManager; // manager of <presence> stanzas and their handlers
	CJabberSendManager m_sendManager; // manager of outgoing stanza handlers
	CJabberAdhocManager m_adhocManager;
	CJabberClientCapsManager m_clientCapsManager;
	CPrivacyListManager m_privacyListManager;
	CJabberSDManager m_SDManager;

	//HWND m_hwndConsole;
	CJabberDlgBase *m_pDlgConsole;
	HANDLE m_hThreadConsole;
	UINT m_dwConsoleThreadId;

	// proto frame
	CJabberInfoFrame *m_pInfoFrame;

	LIST<JABBER_LIST_ITEM> m_lstRoster;
	mir_cs m_csLists;
	BOOL   m_bListInitialised;

	LIST<JabberFeatCapPairDynamic> m_lstJabberFeatCapPairsDynamic; // list of features registered through IJabberNetInterface::RegisterFeature()
	JabberCapsBits m_uEnabledFeatCapsDynamic;

	HGENMENU m_hMenuRoot;
	HGENMENU m_hMenuChangePassword;
	HGENMENU m_hMenuGroupchat;
	HGENMENU m_hMenuBookmarks;
	HGENMENU m_hMenuNotes;

	HGENMENU m_hMenuPrivacyLists;
	HGENMENU m_hMenuRosterControl;
	HGENMENU m_hMenuServiceDiscovery;
	HGENMENU m_hMenuSDMyTransports;
	HGENMENU m_hMenuSDTransports;
	HGENMENU m_hMenuSDConferences;

	HWND m_hwndCommandWindow;

	int m_nIqIdRegGetReg;
	int m_nIqIdRegSetReg;

	int m_nSDBrowseMode;
	DWORD m_dwSDLastRefresh;
	DWORD m_dwSDLastAutoDisco;

	HGENMENU m_hChooseMenuItem;
	int m_privacyMenuServiceAllocated;

	TFilterInfo m_filterInfo;

	CNoteList m_notes;

	/*******************************************************************
	* Function declarations
	*******************************************************************/

	void   JabberUpdateDialogs(BOOL bEnable);

	//---- jabber_adhoc.cpp --------------------------------------------------------------

	int    __cdecl ContactMenuRunCommands(WPARAM wParam, LPARAM lParam);

	HWND   GetWindowFromIq(CJabberIqInfo *pInfo);
	BOOL   HandleAdhocCommandRequest(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   IsRcRequestAllowedByACL(CJabberIqInfo *pInfo);

	int    AdhocSetStatusHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);
	int    AdhocOptionsHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);
	int    AdhocForwardHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);
	int    AdhocLockWSHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);
	int    AdhocQuitMirandaHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);
	int    AdhocLeaveGroupchatsHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);

	void   OnIqResult_ListOfCommands(HXML iqNode, CJabberIqInfo*);
	void   OnIqResult_CommandExecution(HXML iqNode, CJabberIqInfo*);
	void   AdHoc_RequestListOfCommands(TCHAR * szResponder, HWND hwndDlg);
	int    AdHoc_ExecuteCommand(HWND hwndDlg, TCHAR * jid, struct JabberAdHocData* dat);
	int    AdHoc_SubmitCommandForm(HWND hwndDlg, JabberAdHocData * dat, TCHAR* action);
	int    AdHoc_AddCommandRadio(HWND hFrame, TCHAR * labelStr, int id, int ypos, int value);
	int    AdHoc_OnJAHMCommandListResult(HWND hwndDlg, HXML  iqNode, JabberAdHocData* dat);
	int    AdHoc_OnJAHMProcessResult(HWND hwndDlg, HXML workNode, JabberAdHocData* dat);

	void   ContactMenuAdhocCommands(struct CJabberAdhocStartupParams* param);

	//---- jabber_archive.c --------------------------------------------------------------

	void   EnableArchive(bool bEnable);
	void   RetrieveMessageArchive(MCONTACT hContact, JABBER_LIST_ITEM *pItem);

	void   OnIqResultGetCollection(HXML iqNode, CJabberIqInfo*);
	void   OnIqResultGetCollectionList(HXML iqNode, CJabberIqInfo*);

	//---- jabber_bookmarks.c ------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleBookmarks(WPARAM wParam, LPARAM lParam);

	int    AddEditBookmark(JABBER_LIST_ITEM *item);

	//---- jabber_notes.c -----------------------------------------------------------------

	void CJabberProto::ProcessIncomingNote(CNoteItem *pNote, bool ok);
	void CJabberProto::ProcessOutgoingNote(CNoteItem *pNote, bool ok);

	bool CJabberProto::OnIncomingNote(const TCHAR *szFrom, HXML hXml);

	INT_PTR    __cdecl CJabberProto::OnMenuSendNote(WPARAM, LPARAM);
	INT_PTR    __cdecl CJabberProto::OnMenuHandleNotes(WPARAM, LPARAM);
	INT_PTR    __cdecl CJabberProto::OnIncomingNoteEvent(WPARAM, LPARAM);

	//---- jabber_byte.c -----------------------------------------------------------------

	void   __cdecl ByteSendThread(JABBER_BYTE_TRANSFER *jbt);
	void   __cdecl ByteReceiveThread(JABBER_BYTE_TRANSFER *jbt);

	void   IqResultProxyDiscovery(HXML iqNode, CJabberIqInfo *pInfo);
	void   ByteInitiateResult(HXML iqNode, CJabberIqInfo *pInfo);
	void   ByteSendViaProxy(JABBER_BYTE_TRANSFER *jbt);
	int    ByteSendParse(HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen);
	void   IqResultStreamActivate(HXML iqNode, CJabberIqInfo *pInfo);
	int    ByteReceiveParse(HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen);
	int    ByteSendProxyParse(HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen);

	//---- jabber_caps.cpp ---------------------------------------------------------------

	JabberCapsBits GetTotalJidCapabilites(const TCHAR *jid);
	JabberCapsBits GetResourceCapabilites(const TCHAR *jid, BOOL appendBestResource);

	//---- jabber_captcha.cpp ------------------------------------------------------------

	void   sendCaptchaResult(TCHAR* buf, ThreadData *info, LPCTSTR from, LPCTSTR challenge, LPCTSTR fromjid, LPCTSTR sid);
	void   sendCaptchaError(ThreadData *info, LPCTSTR from, LPCTSTR to, LPCTSTR challenge);

	//---- jabber_chat.cpp ---------------------------------------------------------------

	int    GcInit(JABBER_LIST_ITEM *item);
	void   GcLogCreate(JABBER_LIST_ITEM *item);
	void   GcLogUpdateMemberStatus(JABBER_LIST_ITEM *item, const TCHAR *resource, const TCHAR *nick, const TCHAR *jid, int action, HXML reason, int nStatusCode = -1);
	void   GcLogShowInformation(JABBER_LIST_ITEM *item, pResourceStatus &user, TJabberGcLogInfoType type);
	void   GcQuit(JABBER_LIST_ITEM* jid, int code, HXML reason);

	void   FilterList(HWND hwndList);
	void   ResetListOptions(HWND hwndList);
	void   InviteUser(TCHAR *room, TCHAR *pUser, TCHAR *text);

	void   AdminSet(const TCHAR *to, const TCHAR *ns, const TCHAR *szItem, const TCHAR *itemVal, const TCHAR *var, const TCHAR *varVal);
	void   AdminGet(const TCHAR *to, const TCHAR *ns, const TCHAR *var, const TCHAR *varVal, JABBER_IQ_HANDLER foo);
	void   AdminSetReason(const TCHAR *to, const TCHAR *ns, const TCHAR *szItem, const TCHAR *itemVal, const TCHAR *var, const TCHAR *varVal, const TCHAR *rsn);
	void   AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const TCHAR* str);
	void   AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const TCHAR* str, const TCHAR* rsn);
	void   DeleteMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const TCHAR* jid);

	//---- jabber_console.cpp ------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleConsole(WPARAM wParam, LPARAM lParam);
	void   __cdecl ConsoleThread(void*);

	void   ConsoleInit(void);
	void   ConsoleUninit(void);

	bool   FilterXml(HXML node, DWORD flags);
	bool   RecursiveCheckFilter(HXML node, DWORD flags);

	//---- jabber_disco.cpp --------------------------------------------------------------

	void   LaunchServiceDiscovery(TCHAR *jid);
	INT_PTR    __cdecl OnMenuHandleServiceDiscovery(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleServiceDiscoveryMyTransports(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleServiceDiscoveryTransports(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleServiceDiscoveryConferences(WPARAM wParam, LPARAM lParam);

	void   OnIqResultServiceDiscoveryInfo(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryItems(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryRootInfo(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryRoot(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryRootItems(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   SendInfoRequest(CJabberSDNode *pNode, HXML parent);
	BOOL   SendBothRequests(CJabberSDNode *pNode, HXML parent);
	void   PerformBrowse(HWND hwndDlg);
	BOOL   IsNodeRegistered(CJabberSDNode *pNode);
	void   ApplyNodeIcon(HTREELISTITEM hItem, CJabberSDNode *pNode);
	BOOL   SyncTree(HTREELISTITEM hIndex, CJabberSDNode *pNode);
	void   ServiceDiscoveryShowMenu(CJabberSDNode *node, HTREELISTITEM hItem, POINT pt);

	int    SetupServiceDiscoveryDlg(TCHAR* jid);

	void   OnIqResultCapsDiscoInfo(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultCapsDiscoInfoSI(HXML iqNode, CJabberIqInfo *pInfo);

	void   RegisterAgent(HWND hwndDlg, TCHAR* jid);

	//---- jabber_file.cpp ---------------------------------------------------------------

	int    FileReceiveParse(filetransfer *ft, char* buffer, int datalen);
	int    FileSendParse(JABBER_SOCKET s, filetransfer *ft, char* buffer, int datalen);

	void   UpdateChatUserStatus(wchar_t* chat_jid, wchar_t* jid, wchar_t* nick, int role, int affil, int status, BOOL update_nick);

	void   GroupchatJoinRoomByJid(HWND hwndParent, TCHAR *jid);

	void   RenameParticipantNick(JABBER_LIST_ITEM *item, const TCHAR *oldNick, HXML itemNode);
	void   AcceptGroupchatInvite(const TCHAR *roomJid, const TCHAR *reason, const TCHAR *password);

	//---- jabber_form.c -----------------------------------------------------------------

	void   FormCreateDialog(HXML xNode, TCHAR* defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata);

	//---- jabber_ft.c -------------------------------------------------------------------

	void   __cdecl FileReceiveThread(filetransfer *ft);
	void   __cdecl FileServerThread(filetransfer *ft);

	void   FtCancel(filetransfer *ft);
	void   FtInitiate(TCHAR* jid, filetransfer *ft);
	void   FtHandleSiRequest(HXML iqNode);
	void   FtAcceptSiRequest(filetransfer *ft);
	void   FtAcceptIbbRequest(filetransfer *ft);
	BOOL   FtHandleBytestreamRequest(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   FtHandleIbbRequest(HXML iqNode, BOOL bOpen);

	//---- jabber_groupchat.c ------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleJoinGroupchat(WPARAM wParam, LPARAM lParam);
	void       __cdecl GroupchatInviteAcceptThread(JABBER_GROUPCHAT_INVITE_INFO *inviteInfo);

	INT_PTR    __cdecl OnJoinChat(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnLeaveChat(WPARAM wParam, LPARAM lParam);

	void   GroupchatJoinRoom(LPCTSTR server, LPCTSTR room, LPCTSTR nick, LPCTSTR password, bool autojoin = false);
	void   GroupchatProcessPresence(HXML node);
	void   GroupchatProcessMessage(HXML node);
	void   GroupchatProcessInvite(LPCTSTR roomJid, LPCTSTR from, LPCTSTR reason, LPCTSTR password);
	void   GroupchatJoinDlg(TCHAR* roomJid);
	void   OnIqResultDiscovery(HXML iqNode, CJabberIqInfo *pInfo);

	//---- jabber_icolib.cpp -------------------------------------------------------------

	int*   m_transportProtoTableStartIndex;

	void   IconsInit(void);
	HANDLE GetIconHandle(int iconId);
	HICON  LoadIconEx(const char* name, bool big = false);
	int    LoadAdvancedIcons(int iID);
	int    GetTransportProtoID(TCHAR* TransportDomain);
	int    GetTransportStatusIconIndex(int iID, int Status);
	BOOL   DBCheckIsTransportedContact(const TCHAR *jid, MCONTACT hContact);
	void   CheckAllContactsAreTransported(void);
	INT_PTR __cdecl JGetAdvancedStatusIcon(WPARAM wParam, LPARAM lParam);

	//---- jabber_iq.c -------------------------------------------------------------------

	__forceinline CJabberIqInfo* AddIQ(JABBER_IQ_HANDLER pHandler, int nIqType = JABBER_IQ_TYPE_GET, const TCHAR *szReceiver = NULL, DWORD dwParamsToParse = 0, int nIqId = -1, void *pUserData = NULL, int iPriority = JH_PRIORITY_DEFAULT)
	{
		return m_iqManager.AddHandler(pHandler, nIqType, szReceiver, dwParamsToParse, nIqId, pUserData, iPriority);
	}

	void   __cdecl ExpirerThread(void*);

	void   OnIqResultBind(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultDiscoBookmarks(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultEntityTime(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultExtSearch(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetAuth(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetVCardAvatar(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetClientAvatar(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetServerAvatar(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGotAvatar(MCONTACT hContact, HXML n, const TCHAR *mimeType);
	void   OnIqResultGetMuc(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetRegister(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetRoster(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetVcard(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultLastActivity(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetAdminList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetBanList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetMemberList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetModeratorList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetOwnerList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetVoiceList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultNestedRosterGroups(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultNotes(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSession(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetAuth(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetBookmarks(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetPassword(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetRegister(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetSearch(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetVcard(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultVersion(HXML node, CJabberIqInfo *pInfo);
	void   OnProcessLoginRq(ThreadData *info, DWORD rq);
	void   OnLoggedIn(void);

	//---- jabber_iq_handlers.cpp -------------------------------------------------------

	BOOL   OnIqRequestVersion(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestLastActivity(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestPing(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestTime(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqProcessIqOldTime(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestAvatar(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnSiRequest(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnRosterPushRequest(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestOOB(HXML node, CJabberIqInfo *pInfo);
	BOOL   OnIqHttpAuth(HXML node, CJabberIqInfo *pInfo);
	BOOL   AddClistHttpAuthEvent(CJabberHttpAuthParams *pParams);

	void   __cdecl IbbSendThread(JABBER_IBB_TRANSFER *jibb);
	void   __cdecl IbbReceiveThread(JABBER_IBB_TRANSFER *jibb);

	void   OnIbbInitiateResult(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIbbCloseResult(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   OnFtHandleIbbIq(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   OnIbbRecvdData(const TCHAR *data, const TCHAR *sid, const TCHAR *seq);

	void   OnFtSiResult(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   FtIbbSend(int blocksize, filetransfer *ft);
	BOOL   FtSend(HANDLE hConn, filetransfer *ft);
	void   FtSendFinal(BOOL success, filetransfer *ft);
	int    FtReceive(HANDLE hConn, filetransfer *ft, char* buffer, int datalen);
	void   FtReceiveFinal(BOOL success, filetransfer *ft);

	//---- jabber_iqid.cpp ---------------------------------------------------------------

	void   GroupchatJoinByHContact(MCONTACT hContact, bool autojoin=false);

	//---- jabber_message_handlers.cpp ---------------------------------------------------

	BOOL   OnMessageError(HXML node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   OnMessageIbb(HXML node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   OnMessagePubsubEvent(HXML node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   OnMessageGroupchat(HXML node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);

	//---- jabber_list.cpp ---------------------------------------------------------------

	JABBER_LIST_ITEM *ListAdd(JABBER_LIST list, const TCHAR *jid);
	JABBER_LIST_ITEM *ListGetItemPtr(JABBER_LIST list, const TCHAR *jid);
	JABBER_LIST_ITEM *ListGetItemPtrFromIndex(int index);

	void   ListWipe(void);

	void   ListRemove(JABBER_LIST list, const TCHAR *jid);
	void   ListRemoveList(JABBER_LIST list);
	void   ListRemoveByIndex(int index);
	int    ListFindNext(JABBER_LIST list, int fromOffset);

	pResourceStatus ListFindResource(JABBER_LIST list, const TCHAR *jid);

	bool   ListAddResource(JABBER_LIST list, const TCHAR *jid, int status, const TCHAR *statusMessage, char priority = 0, const TCHAR *nick = NULL);
	void   ListRemoveResource(JABBER_LIST list, const TCHAR *jid);
	TCHAR* ListGetBestClientResourceNamePtr(const TCHAR *jid);

	void   SetMucConfig(HXML node, void *from);
	void   OnIqResultMucGetJidList(HXML iqNode, JABBER_MUC_JIDLIST_TYPE listType);

	void   OnIqResultServerDiscoInfo(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetVcardPhoto(HXML n, MCONTACT hContact, bool &hasPhoto);
	void   SetBookmarkRequest (XmlNodeIq& iqId);

	//---- jabber_menu.cpp ---------------------------------------------------------------

	INT_PTR    __cdecl OnMenuConvertChatContact(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuRosterAdd(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuOptions(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuTransportLogin(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuTransportResolve(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuBookmarkAdd(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuRevokeAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleResource(WPARAM wParam, LPARAM lParam, LPARAM res);
	INT_PTR    __cdecl OnMenuHandleDirectPresence(WPARAM wParam, LPARAM lParam, LPARAM res);
	INT_PTR    __cdecl OnMenuSetPriority(WPARAM wParam, LPARAM lParam, LPARAM dwDelta);

	void   GlobalMenuInit(void);
	void   GlobalMenuUninit(void);

	void   MenuInit(void);
	void   MenuUpdateSrmmIcon(JABBER_LIST_ITEM *item);

	void   AuthWorker(MCONTACT hContact, char* authReqType);

	void   UpdatePriorityMenu(short priority);

	HGENMENU m_hMenuPriorityRoot;
	short  m_priorityMenuVal;
	bool   m_priorityMenuValSet;

	//---- jabber_misc.c -----------------------------------------------------------------

	INT_PTR    __cdecl OnGetEventTextChatStates(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnGetEventTextPresence(WPARAM wParam, LPARAM lParam);

	void   AddContactToRoster(const TCHAR *jid, const TCHAR *nick, const TCHAR *grpName);
	void   DBAddAuthRequest(const TCHAR *jid, const TCHAR *nick);
	BOOL   AddDbPresenceEvent(MCONTACT hContact, BYTE btEventType);
	MCONTACT DBCreateContact(const TCHAR *jid, const TCHAR *nick, BOOL temporary, BOOL stripResource);
	void   GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen);
	void   ResolveTransportNicks(const TCHAR *jid);
	void   SetServerStatus(int iNewStatus);
	void   FormatMirVer(pResourceStatus &resource, CMString&);
	void   UpdateMirVer(JABBER_LIST_ITEM *item);
	void   UpdateMirVer(MCONTACT hContact, pResourceStatus &resource);
	void   UpdateSubscriptionInfo(MCONTACT hContact, JABBER_LIST_ITEM *item);
	void   SetContactOfflineStatus(MCONTACT hContact);
	void   InitPopups(void);
	void   MsgPopup(MCONTACT hContact, const TCHAR *szMsg, const TCHAR *szTitle);
	CMString ExtractImage(HXML node);

	//---- jabber_opt.cpp ----------------------------------------------------------------
	INT_PTR  __cdecl OnMenuHandleRosterControl(WPARAM wParam, LPARAM lParam);

	void   _RosterExportToFile(HWND hwndDlg);
	void   _RosterImportFromFile(HWND hwndDlg);
	void   _RosterSendRequest(HWND hwndDlg, BYTE rrAction);
	void   _RosterHandleGetRequest(HXML node, CJabberIqInfo*);

	//---- jabber_password.cpp --------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleChangePassword(WPARAM wParam, LPARAM lParam);

	//---- jabber_privacy.cpp ------------------------------------------------------------
	ROSTERREQUSERDATA rrud;

	INT_PTR __cdecl menuSetPrivacyList(WPARAM wParam, LPARAM lParam, LPARAM iList);
	INT_PTR __cdecl OnMenuHandlePrivacyLists(WPARAM wParam, LPARAM lParam);

	void   BuildPrivacyMenu(void);
	void   BuildPrivacyListsMenu(bool bDeleteOld);

	void   QueryPrivacyLists(ThreadData *pThreadInfo = NULL);

	BOOL   OnIqRequestPrivacyLists(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyList(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyLists(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyListActive(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyListDefault(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyListModify(HXML iqNode, CJabberIqInfo *pInfo);

	//---- jabber_proto.cpp --------------------------------------------------------------

	void   __cdecl BasicSearchThread(struct JABBER_SEARCH_BASIC *jsb);
	void   __cdecl GetAwayMsgThread(void* hContact);
	void   __cdecl SendMessageAckThread(void* hContact);

	MCONTACT AddToListByJID(const TCHAR *newJid, DWORD flags);

	void   InfoFrame_OnSetup(CJabberInfoFrame_Event *evt);
	void   InfoFrame_OnTransport(CJabberInfoFrame_Event *evt);

	//---- jabber_rc.cpp -----------------------------------------------------------------

	int    RcGetUnreadEventsCount(void);

	//---- jabber_search.cpp -------------------------------------------------------------

	void   SearchReturnResults(HANDLE id, void* pvUsersInfo, U_TCHAR_MAP* pmAllFields);
	void   OnIqResultAdvancedSearch(HXML iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetSearchFields(HXML iqNode, CJabberIqInfo *pInfo);
	int    SearchRenewFields(HWND hwndDlg, JabberSearchData * dat);
	void   SearchDeleteFromRecent(const TCHAR *szAddr, BOOL deleteLastFromDB = TRUE);
	void   SearchAddToRecent(const TCHAR *szAddr, HWND hwndDialog = NULL);

	//---- jabber_std.cpp ----------------------------------------------
	void   ConvertPasswords();
	void   JLoginFailed(int errorCode);

	//---- jabber_svc.c ------------------------------------------------------------------

	void   CheckMenuItems();
	void   EnableMenuItems(BOOL bEnable);

	INT_PTR    __cdecl JabberGetAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberGetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberGetAvatarInfo(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl ServiceSendXML(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberSetAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberSetNickname(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberSendNudge(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberGCGetToolTipText(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberServiceParseXmppURI(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnHttpAuthRequest(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl JabberGetApi(WPARAM wParam, LPARAM lParam);

	void   ExternalTempIqHandler(HXML node, CJabberIqInfo *pInfo);
	BOOL   ExternalIqHandler(HXML node, CJabberIqInfo *pInfo);
	BOOL   ExternalMessageHandler(HXML node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   ExternalPresenceHandler(HXML node, ThreadData *pThreadData, CJabberPresenceInfo* pInfo);
	BOOL   ExternalSendHandler(HXML node, ThreadData *pThreadData, CJabberSendInfo* pInfo);

	BOOL SendHttpAuthReply(CJabberHttpAuthParams *pParams, BOOL bAuthorized);

	//---- jabber_thread.c ----------------------------------------------

	ptrT   m_savedPassword;

	typedef struct {
		bool isPlainAvailable;
		bool isPlainOldAvailable;
		bool isMd5Available;
		bool isScramAvailable;
		bool isNtlmAvailable;
		bool isSpnegoAvailable;
		bool isKerberosAvailable;
		bool isAuthAvailable;
		bool isSessionAvailable;
		TCHAR *m_gssapiHostName;
	} AUTHMECHS;

	AUTHMECHS m_AuthMechs;

	void   __cdecl ServerThread(JABBER_CONN_DATA *info);

	void   OnProcessFailure(HXML node, ThreadData *info);
	void   OnProcessError(HXML node, ThreadData *info);
	void   OnProcessSuccess(HXML node, ThreadData *info);
	void   OnProcessChallenge(HXML node, ThreadData *info);
	void   OnProcessProceed(HXML node, ThreadData *info);
	void   OnProcessCompressed(HXML node, ThreadData *info);
	void   OnProcessMessage(HXML node, ThreadData *info);
	void   OnProcessPresence(HXML node, ThreadData *info);
	void   OnProcessPresenceCapabilites(HXML node);
	void   OnProcessPubsubEvent(HXML node);

	void   OnProcessStreamOpening(HXML node, ThreadData *info);
	void   OnProcessProtocol(HXML node, ThreadData *info);

	void   UpdateJidDbSettings(const TCHAR *jid);
	MCONTACT CreateTemporaryContact(const TCHAR *szJid, JABBER_LIST_ITEM* chatItem);

	void   PerformRegistration(ThreadData *info);
	void   PerformIqAuth(ThreadData *info);
	void   PerformAuthentication(ThreadData *info);
	void   OnProcessFeatures(HXML node, ThreadData *info);

	void   xmlStreamInitialize(char *which);
	void   xmlStreamInitializeNow(ThreadData *info);

	BOOL   OnProcessJingle(HXML node);
	void   OnProcessIq(HXML node);
	void   OnProcessRegIq(HXML node, ThreadData *info);
	void   OnPingReply(HXML node, CJabberIqInfo *pInfo);

	bool   ProcessCaptcha(HXML node, HXML parentNode, ThreadData *info);

	//---- jabber_util.c -----------------------------------------------------------------
	pResourceStatus ResourceInfoFromJID(const TCHAR *jid);

	MCONTACT HContactFromJID(const TCHAR *jid, BOOL bStripResource = 3);
	MCONTACT ChatRoomHContactFromJID(const TCHAR *jid);
	void   SendVisibleInvisiblePresence(BOOL invisible);
	void   SendPresenceTo(int status, const TCHAR* to, HXML extra, const TCHAR *msg = NULL);
	void   SendPresence(int m_iStatus, bool bSendToAll);
	void   StringAppend(char* *str, int *sizeAlloced, const char* fmt, ...);
	void   RebuildInfoFrame(void);

	// returns buf or NULL on error
	TCHAR* GetClientJID(MCONTACT hContact, TCHAR *dest, size_t destLen);
	TCHAR* GetClientJID(const TCHAR *jid, TCHAR *dest, size_t destLen);

	void   ComboLoadRecentStrings(HWND hwndDlg, UINT idcCombo, char *param, int recentCount=JABBER_DEFAULT_RECENT_COUNT);
	void   ComboAddRecentString(HWND hwndDlg, UINT idcCombo, char *param, const TCHAR *string, int recentCount=JABBER_DEFAULT_RECENT_COUNT);
	BOOL   EnterString(CMString &result, LPCTSTR caption, int type, char *windowName=NULL, int recentCount=JABBER_DEFAULT_RECENT_COUNT, int timeout=0);
	bool   IsMyOwnJID(LPCTSTR szJID);

	void __cdecl LoadHttpAvatars(void* param);

	//---- jabber_vcard.c -----------------------------------------------

	int    m_vCardUpdates;
	bool   m_bPhotoChanged;
	TCHAR  m_szPhotoFileName[MAX_PATH];
	void   OnUserInfoInit_VCard(WPARAM, LPARAM);

	int    SendGetVcard(const TCHAR *jid);
	void   AppendVcardFromDB(HXML n, char* tag, char* key);
	void   SetServerVcard(BOOL bPhotoChanged, TCHAR* szPhotoFileName);
	void   SaveVcardToDB(HWND hwndPage, int iPage);

	//---- jabber_ws.c -------------------------------------------------

	JABBER_SOCKET WsConnect(char* host, WORD port);

	BOOL   WsInit(void);
	void   WsUninit(void);
	int    WsSend(JABBER_SOCKET s, char* data, int datalen, int flags);
	int    WsRecv(JABBER_SOCKET s, char* data, long datalen, int flags);

	//---- jabber_xml.c ------------------------------------------------------------------

	int    OnXmlParse(char* buffer);
	void   OnConsoleProcessXml(HXML node, DWORD flags);

	//---- jabber_xmlns.c ----------------------------------------------------------------

	BOOL   OnHandleDiscoInfoRequest(HXML iqNode, CJabberIqInfo *pInfo);
	BOOL   OnHandleDiscoItemsRequest(HXML iqNode, CJabberIqInfo *pInfo);

	//---- jabber_xstatus.c --------------------------------------------------------------

	INT_PTR __cdecl OnSetListeningTo(WPARAM wParam, LPARAM lParams);
	INT_PTR __cdecl OnGetXStatusIcon(WPARAM wParam, LPARAM lParams);
	INT_PTR __cdecl OnGetXStatusEx(WPARAM wParam, LPARAM lParams);
	INT_PTR __cdecl OnSetXStatusEx(WPARAM wParam, LPARAM lParams);

	HICON  GetXStatusIcon(int bStatus, UINT flags);

	void   RegisterAdvStatusSlot(const char *pszSlot);
	void   ResetAdvStatus(MCONTACT hContact, const char *pszSlot);
	void   WriteAdvStatus(MCONTACT hContact, const char *pszSlot, const TCHAR *pszMode, const char *pszIcon, const TCHAR *pszTitle, const TCHAR *pszText);
	char*  ReadAdvStatusA(MCONTACT hContact, const char *pszSlot, const char *pszValue);
	TCHAR* ReadAdvStatusT(MCONTACT hContact, const char *pszSlot, const char *pszValue);

	BOOL   SendPepTune(TCHAR* szArtist, TCHAR* szLength, TCHAR* szSource, TCHAR* szTitle, TCHAR* szTrack, TCHAR* szUri);

	void   XStatusInit(void);

	void   SetContactTune(MCONTACT hContact,  LPCTSTR szArtist, LPCTSTR szLength, LPCTSTR szSource, LPCTSTR szTitle, LPCTSTR szTrack);

	void   InfoFrame_OnUserMood(CJabberInfoFrame_Event *evt);
	void   InfoFrame_OnUserActivity(CJabberInfoFrame_Event *evt);

	CPepServiceList m_pepServices;

private:
	char  *m_szXmlStreamToBeInitialized;

	DWORD  m_lastTicks;

	HANDLE m_hPopupClass;

	LONG   m_nSerial;

	HGENMENU   m_hPrivacyMenuRoot;
	BOOL       m_menuItemsStatus;
	LIST<void> m_hPrivacyMenuItems;

	int        m_nMenuResourceItems;
	HGENMENU  *m_phMenuResourceItems;

public:
	DWORD     STDMETHODCALLTYPE GetFlags() const;                    // Set of JIF_* flags.
	int       STDMETHODCALLTYPE GetVersion() const;                  // Returns version of IJabberInterface.
	DWORD     STDMETHODCALLTYPE GetJabberVersion() const;            // Returns Jabber plugin version.

	int       STDMETHODCALLTYPE CompareJIDs(LPCTSTR jid1, LPCTSTR jid2); // Strips resource names from given JIDs and returns result of comparison for these JIDs.
	MCONTACT  STDMETHODCALLTYPE ContactFromJID(LPCTSTR jid);             // Returns contact handle for given JID.
	LPTSTR    STDMETHODCALLTYPE ContactToJID(MCONTACT hContact);           // Returns JID of hContact. You must free the result using mir_free().
	LPTSTR    STDMETHODCALLTYPE GetBestResourceName(LPCTSTR jid);        // Returns best resource name for given JID. You must free the result using mir_free().
	LPTSTR    STDMETHODCALLTYPE GetResourceList(LPCTSTR jid);            // Returns all resource names for a given JID in format "resource1\0resource2\0resource3\0\0" (all resources are separated by \0 character and the whole string is terminated with two \0 characters). You must free the string using mir_free().
	char*     STDMETHODCALLTYPE GetModuleName() const;                   // Returns Jabber module name.

	int       STDMETHODCALLTYPE SerialNext();           // Returns id that can be used for next message sent through SendXmlNode().
	int       STDMETHODCALLTYPE SendXmlNode(HXML node); // Sends XML node.

	HJHANDLER STDMETHODCALLTYPE AddPresenceHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority);
	HJHANDLER STDMETHODCALLTYPE AddMessageHandler(JABBER_HANDLER_FUNC Func, int iMsgTypes, LPCTSTR szXmlns, LPCTSTR szTag, void *pUserData, int iPriority);
	HJHANDLER STDMETHODCALLTYPE AddIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, LPCTSTR szXmlns, LPCTSTR szTag, void *pUserData, int iPriority);
	HJHANDLER STDMETHODCALLTYPE AddTemporaryIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, int iIqId, void *pUserData, DWORD dwTimeout, int iPriority);
	HJHANDLER STDMETHODCALLTYPE AddSendHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority);
	int       STDMETHODCALLTYPE RemoveHandler(HJHANDLER hHandler);

	int       STDMETHODCALLTYPE RegisterFeature(LPCTSTR szFeature, LPCTSTR szDescription);
	int       STDMETHODCALLTYPE AddFeatures(LPCTSTR szFeatures);    // Adds features to the list of features returned by the client.
	int       STDMETHODCALLTYPE RemoveFeatures(LPCTSTR szFeatures); // Removes features from the list of features returned by the client.
	LPTSTR    STDMETHODCALLTYPE GetResourceFeatures(LPCTSTR jid);   // Returns all features supported by JID in format "feature1\0feature2\0...\0featureN\0\0". You must free returned string using mir_free().
	HANDLE    STDMETHODCALLTYPE GetHandle();                        // Returns connection handle

private:
	JabberFeatCapPairDynamic *FindFeature(LPCTSTR szFeature);
};

extern LIST<CJabberProto> g_Instances;

#endif
