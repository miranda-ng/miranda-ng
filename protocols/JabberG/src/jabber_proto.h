/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-19 Miranda NG team

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
#include "jabber_omemo.h"
#include "jabber_strm_mgmt.h"

struct CJabberProto;
class CJabberMucJidListDlg;
class CRosterEditorDlg;

class CJabberFormDlg;
typedef void (CJabberProto::*JABBER_FORM_SUBMIT_FUNC)(CJabberFormDlg *pDlg, void *userdata);

enum TJabberGcLogInfoType { INFO_BAN, INFO_STATUS, INFO_CONFIG, INFO_AFFILIATION, INFO_ROLE };

#define JABBER_DEFAULT_RECENT_COUNT 10

struct TFilterInfo
{
	enum Type { T_JID, T_XMLNS, T_ANY, T_OFF };

	volatile BOOL msg, presence, iq;
	volatile Type type;

	mir_cs csPatternLock;
	wchar_t pattern[256];
};

struct CJabberProto : public PROTO<CJabberProto>, public IJabberInterface
{
				CJabberProto(const char*, const wchar_t*);
				~CJabberProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;
	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;

	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szReason) override;
	int      FileResume(HANDLE hTransfer, int* action, const wchar_t** szFilename) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = 0) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;

	HANDLE   SearchBasic(const wchar_t *id) override;
	HANDLE   SearchByEmail(const wchar_t *email) override;
	HANDLE   SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName) override;
	HWND     SearchAdvanced(HWND owner) override;
	HWND     CreateExtendedSearchUI(HWND owner) override;

	int      SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int      SendMsg(MCONTACT hContact, int flags, const char *msg) override;

	int      SetApparentMode(MCONTACT hContact, int mode) override;
	int      SetStatus(int iNewStatus) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int m_iStatus, const wchar_t *msg) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnBuildProtoMenu(void) override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	//====| Services |====================================================================
	INT_PTR  __cdecl SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetMyAwayMsg(WPARAM wParam, LPARAM lParam);

	//====| Events |======================================================================
	void __cdecl OnAddContactForever(MCONTACT hContact);
	int  __cdecl OnDbSettingChanged(WPARAM, LPARAM);
	int  __cdecl OnIdleChanged(WPARAM, LPARAM);
	int  __cdecl OnLangChanged(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM, LPARAM);
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

	CMOption<BYTE> m_bAllowVersionRequests;
	CMOption<BYTE> m_bAcceptHttpAuth;
	CMOption<BYTE> m_bAddRoster2Bookmarks;
	CMOption<BYTE> m_bAutoAcceptAuthorization;
	CMOption<BYTE> m_bAutoAcceptMUC;
	CMOption<BYTE> m_bAutoAdd;
	CMOption<BYTE> m_bAutoJoinBookmarks;
	CMOption<BYTE> m_bAutoJoinConferences;
	CMOption<BYTE> m_bAutoJoinHidden;
	CMOption<BYTE> m_bAvatarType;
	CMOption<BYTE> m_bBsDirect;
	CMOption<BYTE> m_bBsDirectManual;
	CMOption<BYTE> m_bBsOnlyIBB;
	CMOption<BYTE> m_bBsProxyManual;
	CMOption<BYTE> m_bDisable3920auth;
	CMOption<BYTE> m_bDisableFrame;
	CMOption<BYTE> m_bEnableAvatars;
	CMOption<BYTE> m_bEnableRemoteControl;
	CMOption<BYTE> m_bEnableUserActivity;
	CMOption<BYTE> m_bEnableUserMood;
	CMOption<BYTE> m_bEnableUserTune;
	CMOption<BYTE> m_bEnableZlib;
	CMOption<BYTE> m_bExtendedSearch;
	CMOption<BYTE> m_bFixIncorrectTimestamps;
	CMOption<BYTE> m_bGcLogAffiliations;
	CMOption<BYTE> m_bGcLogBans;
	CMOption<BYTE> m_bGcLogConfig;
	CMOption<BYTE> m_bGcLogRoles;
	CMOption<BYTE> m_bGcLogStatuses;
	CMOption<BYTE> m_bGcLogChatHistory;
	CMOption<BYTE> m_bHostNameAsResource;
	CMOption<BYTE> m_bIgnoreMUCInvites;
	CMOption<BYTE> m_bKeepAlive;
	CMOption<BYTE> m_bLogChatstates;
	CMOption<BYTE> m_bLogPresence;
	CMOption<BYTE> m_bLogPresenceErrors;
	CMOption<BYTE> m_bManualConnect;
	CMOption<BYTE> m_bMsgAck;
	CMOption<BYTE> m_bRosterSync;
	CMOption<BYTE> m_bSavePassword;
	CMOption<BYTE> m_bUseDomainLogin;
	CMOption<BYTE> m_bShowForeignResourceInMirVer;
	CMOption<BYTE> m_bShowOSVersion;
	CMOption<BYTE> m_bShowTransport;
	CMOption<BYTE> m_bUseSSL;
	CMOption<BYTE> m_bUseTLS;
	CMOption<BYTE> m_bAcceptNotes;
	CMOption<BYTE> m_bAutosaveNotes;
	CMOption<BYTE> m_bEnableMsgArchive;
	CMOption<BYTE> m_bRcMarkMessagesAsRead;
	CMOption<BYTE> m_bProcessXMPPLinks;
	CMOption<BYTE> m_bIgnoreRosterGroups;
	CMOption<BYTE> m_bEnableCarbons;
	CMOption<BYTE> m_bUseOMEMO;
	CMOption<BYTE> m_bEnableStreamMgmt;

	CMOption<DWORD> m_iConnectionKeepAliveInterval;
	CMOption<DWORD> m_iConnectionKeepAliveTimeout;

	PVOID  m_sslCtx;

	HANDLE m_hThreadHandle;

	char *m_szJabberJID;
	int      m_nJabberSearchID;
	time_t   m_tmJabberLoggedInTime;
	time_t   m_tmJabberIdleStartTime;
	UINT     m_nJabberCodePage;
	char *m_tszSelectedLang;

	mir_cs   m_csModeMsgMutex;
	JABBER_MODEMSGS m_modeMsgs;

	bool   m_bCisAvailable;
	bool   m_bJabberOnline; // XMPP connection initialized and we can send XMPP packets
	bool   m_bShutdown;
	bool   m_bSendKeepAlive;
	bool   m_bPepSupported;
	bool   m_bStreamSent;

	HWND   m_hwndAgentRegInput;
	HWND   m_hwndRegProgress;
	HWND   m_hwndJabberChangePassword;
	HWND   m_hwndPrivacyRule;
	HWND   m_hwndJabberAddBookmark;

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
	LIST<char> m_lstTransports;

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

	//xep-0198 related vars
	strm_mgmt m_StrmMgmt;

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
	BOOL   HandleAdhocCommandRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   IsRcRequestAllowedByACL(CJabberIqInfo *pInfo);

	int    AdhocSetStatusHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);
	int    AdhocOptionsHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);
	int    AdhocForwardHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);
	int    AdhocLockWSHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);
	int    AdhocQuitMirandaHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);
	int    AdhocLeaveGroupchatsHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);

	void   OnIqResult_ListOfCommands(const TiXmlElement *iqNode, CJabberIqInfo*);
	void   OnIqResult_CommandExecution(const TiXmlElement *iqNode, CJabberIqInfo*);
	void   AdHoc_RequestListOfCommands(char *szResponder, HWND hwndDlg);
	int    AdHoc_ExecuteCommand(HWND hwndDlg, char *jid, struct JabberAdHocData *dat);
	int    AdHoc_SubmitCommandForm(HWND hwndDlg, JabberAdHocData *dat, char *action);
	int    AdHoc_AddCommandRadio(HWND hFrame, const char *labelStr, int id, int ypos, int value);
	int    AdHoc_OnJAHMCommandListResult(HWND hwndDlg, TiXmlElement *iqNode, JabberAdHocData *dat);
	int    AdHoc_OnJAHMProcessResult(HWND hwndDlg, TiXmlElement *workNode, JabberAdHocData *dat);

	void   ContactMenuAdhocCommands(struct CJabberAdhocStartupParams *param);

	//---- jabber_archive.c --------------------------------------------------------------

	void   EnableArchive(bool bEnable);
	void   RetrieveMessageArchive(MCONTACT hContact, JABBER_LIST_ITEM *pItem);

	void   OnIqResultGetCollection(const TiXmlElement *iqNode, CJabberIqInfo*);
	void   OnIqResultGetCollectionList(const TiXmlElement *iqNode, CJabberIqInfo*);

	//---- jabber_bookmarks.c ------------------------------------------------------------

	INT_PTR __cdecl OnMenuHandleBookmarks(WPARAM wParam, LPARAM lParam);

	int    AddEditBookmark(JABBER_LIST_ITEM *item);

	//---- jabber_notes.c -----------------------------------------------------------------

	void ProcessIncomingNote(CNoteItem *pNote, bool ok);
	void ProcessOutgoingNote(CNoteItem *pNote, bool ok);

	bool OnIncomingNote(const char *szFrom, const TiXmlElement *hXml);

	INT_PTR __cdecl OnMenuSendNote(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuHandleNotes(WPARAM, LPARAM);
	INT_PTR __cdecl OnIncomingNoteEvent(WPARAM, LPARAM);

	//---- jabber_byte.c -----------------------------------------------------------------

	void  __cdecl ByteSendThread(JABBER_BYTE_TRANSFER *jbt);
	void  __cdecl ByteReceiveThread(JABBER_BYTE_TRANSFER *jbt);

	void  IqResultProxyDiscovery(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void  ByteInitiateResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void  ByteSendViaProxy(JABBER_BYTE_TRANSFER *jbt);
	int   ByteSendParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen);
	void  IqResultStreamActivate(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	int   ByteReceiveParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen);
	int   ByteSendProxyParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen);

	//---- jabber_caps.cpp ---------------------------------------------------------------

	void  AddDefaultCaps();
	void  RequestOldCapsInfo(pResourceStatus &r, const char *fullJid);
	void  GetCachedCaps(const char *szNode, const char *szVer, class pResourceStatus &r);

	JabberCapsBits GetTotalJidCapabilities(const char *jid);
	JabberCapsBits GetResourceCapabilities(const char *jid);
	JabberCapsBits GetResourceCapabilities(const char *jid, pResourceStatus &r);

	//---- jabber_captcha.cpp ------------------------------------------------------------

	void   sendCaptchaResult(char* buf, ThreadData *info, const char *from, const char *challenge, const char *fromjid, const char *sid);
	void   sendCaptchaError(ThreadData *info, const char *from, const char *to, const char *challenge);

	//---- jabber_chat.cpp ---------------------------------------------------------------

	int    GcInit(JABBER_LIST_ITEM *item);
	void   GcLogUpdateMemberStatus(JABBER_LIST_ITEM *item, const char *resource, const char *nick, const char *jid, int action, const TiXmlElement *reason, int nStatusCode = -1);
	void   GcLogShowInformation(JABBER_LIST_ITEM *item, pResourceStatus &user, TJabberGcLogInfoType type);
	void   GcQuit(JABBER_LIST_ITEM* jid, int code, const TiXmlElement *reason);

	void   AdminSet(const char *to, const char *ns, const char *szItem, const char *itemVal, const char *var, const char *varVal);
	void   AdminGet(const char *to, const char *ns, const char *var, const char *varVal, JABBER_IQ_HANDLER foo);
	void   AdminSetReason(const char *to, const char *ns, const char *szItem, const char *itemVal, const char *var, const char *varVal, const char *rsn);
	void   AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const char *str);
	void   AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const char *str, const char *reason);
	void   DeleteMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const char* jid);

	//---- jabber_omemo.cpp --------------------------------------------------------------
	
	bool OmemoHandleMessage(const TiXmlElement *node, const char *jid, time_t msgTime);
	void OmemoPutMessageToOutgoingQueue(MCONTACT hContact, int, const char* pszSrc);
	void OmemoPutMessageToIncommingQueue(const TiXmlElement *node, const char *jid, time_t msgTime);
	void OmemoHandleMessageQueue();
	void OmemoHandleDeviceList(const TiXmlElement *node);
	void OmemoInitDevice();
	void OmemoAnnounceDevice();
	void OmemoSendBundle();
	void OmemoPublishNodes();
	bool OmemoCheckSession(MCONTACT hContact);
	unsigned int OmemoEncryptMessage(XmlNode &msg, const char *msg_text, MCONTACT hContact);
	bool OmemoIsEnabled(MCONTACT hContact);
	void OmemoOnIqResultGetBundle(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);

	omemo::omemo_impl m_omemo;

	//---- jabber_console.cpp ------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleConsole(WPARAM wParam, LPARAM lParam);
	void   __cdecl ConsoleThread(void*);

	void   ConsoleInit(void);
	void   ConsoleUninit(void);

	bool   FilterXml(const TiXmlElement *node, DWORD flags);
	bool   RecursiveCheckFilter(const TiXmlElement *node, DWORD flags);

	//---- jabber_disco.cpp --------------------------------------------------------------

	void   LaunchServiceDiscovery(char *jid);
	INT_PTR __cdecl OnMenuHandleServiceDiscovery(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnMenuHandleServiceDiscoveryMyTransports(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnMenuHandleServiceDiscoveryTransports(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl OnMenuHandleServiceDiscoveryConferences(WPARAM wParam, LPARAM lParam);

	void   OnIqResultServiceDiscoveryInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryItems(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryRootInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultServiceDiscoveryRootItems(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   SendInfoRequest(CJabberSDNode *pNode, TiXmlNode *parent);
	BOOL   SendBothRequests(CJabberSDNode *pNode, TiXmlNode *parent = nullptr);
	void   PerformBrowse(HWND hwndDlg);
	BOOL   IsNodeRegistered(CJabberSDNode *pNode);
	void   ApplyNodeIcon(HTREELISTITEM hItem, CJabberSDNode *pNode);
	BOOL   SyncTree(HTREELISTITEM hIndex, CJabberSDNode *pNode);
	void   ServiceDiscoveryShowMenu(CJabberSDNode *node, HTREELISTITEM hItem, POINT pt);

	void   OnIqResultCapsDiscoInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);

	void   RegisterAgent(HWND hwndDlg, char* jid);

	//---- jabber_file.cpp ---------------------------------------------------------------

	int    FileReceiveParse(filetransfer *ft, char* buffer, int datalen);
	int    FileSendParse(HNETLIBCONN s, filetransfer *ft, char* buffer, int datalen);

	void   GroupchatJoinRoomByJid(HWND hwndParent, char *jid);

	void   RenameParticipantNick(JABBER_LIST_ITEM *item, const char *oldNick, const TiXmlElement *itemNode);
	void   AcceptGroupchatInvite(const char *roomJid, const char *reason, const char *password);

	//---- jabber_ft.c -------------------------------------------------------------------

	void   __cdecl FileReceiveThread(filetransfer *ft);
	void   __cdecl FileServerThread(filetransfer *ft);

	void   FtCancel(filetransfer *ft);
	void   FtInitiate(char* jid, filetransfer *ft);
	void   FtHandleSiRequest(const TiXmlElement *iqNode);
	void   FtAcceptSiRequest(filetransfer *ft);
	void   FtAcceptIbbRequest(filetransfer *ft);
	BOOL   FtHandleBytestreamRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   FtHandleIbbRequest(const TiXmlElement *iqNode, BOOL bOpen);

	//---- jabber_groupchat.c ------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleJoinGroupchat(WPARAM wParam, LPARAM lParam);

	INT_PTR    __cdecl OnJoinChat(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnLeaveChat(WPARAM wParam, LPARAM lParam);

	void   GroupchatJoinRoom(const char *server, const char *room, const char *nick, const char *password, bool autojoin = false);
	void   GroupchatProcessPresence(const TiXmlElement *node);
	void   GroupchatProcessMessage(const TiXmlElement *node);
	void   GroupchatProcessInvite(const char *roomJid, const char *from, const char *reason, const char *password);
	void   OnIqResultDiscovery(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);

	//---- jabber_icolib.cpp -------------------------------------------------------------

	int*   m_transportProtoTableStartIndex;

	void   IconsInit(void);
	int    LoadAdvancedIcons(int iID);
	int    GetTransportProtoID(char* TransportDomain);
	int    GetTransportStatusIconIndex(int iID, int Status);
	BOOL   DBCheckIsTransportedContact(const char *jid, MCONTACT hContact);
	void   CheckAllContactsAreTransported(void);
	INT_PTR __cdecl JGetAdvancedStatusIcon(WPARAM wParam, LPARAM lParam);

	//---- jabber_iq.c -------------------------------------------------------------------

	__forceinline CJabberIqInfo* AddIQ(JABBER_IQ_HANDLER pHandler, int nIqType = JABBER_IQ_TYPE_GET, const char *szReceiver = nullptr, void *pUserData = nullptr, int iPriority = JH_PRIORITY_DEFAULT)
	{
		return m_iqManager.AddHandler(pHandler, nIqType, szReceiver, pUserData, iPriority);
	}

	void   __cdecl ExpirerThread(void*);

	void   OnIqResultBind(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultDiscoBookmarks(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultEntityTime(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultExtSearch(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetAuth(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetVCardAvatar(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetClientAvatar(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetServerAvatar(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGotAvatar(MCONTACT hContact, const char *pszText, const char *mimeType);
	void   OnIqResultGetMuc(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetRegister(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetRoster(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetVcard(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultLastActivity(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetAdminList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetBanList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetMemberList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetModeratorList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetOwnerList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultMucGetVoiceList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultNestedRosterGroups(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultNotes(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSession(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetAuth(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetBookmarks(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetPassword(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetRegister(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetSearch(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultSetVcard(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnProcessLoginRq(ThreadData *info, DWORD rq);
	void   OnLoggedIn(void);

	//---- jabber_iq_handlers.cpp -------------------------------------------------------

	BOOL   OnIqRequestVersion(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestLastActivity(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestPing(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestTime(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqProcessIqOldTime(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestAvatar(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnSiRequest(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnRosterPushRequest(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqRequestOOB(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   OnIqHttpAuth(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   AddClistHttpAuthEvent(CJabberHttpAuthParams *pParams);

	void   __cdecl IbbSendThread(JABBER_IBB_TRANSFER *jibb);
	void   __cdecl IbbReceiveThread(JABBER_IBB_TRANSFER *jibb);

	void   OnIbbInitiateResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIbbCloseResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   OnFtHandleIbbIq(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   OnIbbRecvdData(const char *data, const char *sid, const char *seq);

	void   OnFtSiResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   FtIbbSend(int blocksize, filetransfer *ft);
	BOOL   FtSend(HNETLIBCONN hConn, filetransfer *ft);
	void   FtSendFinal(BOOL success, filetransfer *ft);
	int    FtReceive(HNETLIBCONN hConn, filetransfer *ft, char* buffer, int datalen);
	void   FtReceiveFinal(BOOL success, filetransfer *ft);

	//---- jabber_iqid.cpp ---------------------------------------------------------------

	void   GroupchatJoinByHContact(MCONTACT hContact, bool autojoin=false);

	//---- jabber_iqid_muc.cpp -----------------------------------------------------------

	CJabberMucJidListDlg *m_pDlgMucVoiceList, *m_pDlgMucMemberList, *m_pDlgMucModeratorList;
	CJabberMucJidListDlg *m_pDlgMucBanList, *m_pDlgMucAdminList, *m_pDlgMucOwnerList;
	CJabberMucJidListDlg *& GetMucDlg(JABBER_MUC_JIDLIST_TYPE);

	void   SetMucConfig(CJabberFormDlg *pDlg, void *from);
	void   MucShutdown(void);
	void   OnIqResultMucGetJidList(const TiXmlElement *iqNode, JABBER_MUC_JIDLIST_TYPE listType);

	//---- jabber_message_handlers.cpp ---------------------------------------------------

	BOOL   OnMessageError(const TiXmlElement *node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   OnMessageIbb(const TiXmlElement *node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   OnMessagePubsubEvent(const TiXmlElement *node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   OnMessageGroupchat(const TiXmlElement *node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);

	//---- jabber_list.cpp ---------------------------------------------------------------

	JABBER_LIST_ITEM* ListAdd(JABBER_LIST list, const char *jid, MCONTACT hContact = 0);
	JABBER_LIST_ITEM* ListGetItemPtr(JABBER_LIST list, const char *jid);
	JABBER_LIST_ITEM* ListGetItemPtrFromIndex(int index);

	void   ListInit(void);
	void   ListWipe(void);

	void   ListRemove(JABBER_LIST list, const char *jid);
	void   ListRemoveList(JABBER_LIST list);
	void   ListRemoveByIndex(int index);
	int    ListFindNext(JABBER_LIST list, int fromOffset);

	pResourceStatus ListFindResource(JABBER_LIST list, const char *jid);

	bool   ListAddResource(JABBER_LIST list, const char *jid, int status, const char *statusMessage, int priority = 0, const char *nick = nullptr);
	void   ListRemoveResource(JABBER_LIST list, const char *jid);
	char*  ListGetBestClientResourceNamePtr(const char *jid);

	void   OnIqResultServerDiscoInfo(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetVcardPhoto(const TiXmlElement *n, MCONTACT hContact, bool &hasPhoto);
	void   SetBookmarkRequest(XmlNodeIq &iqId);

	//---- jabber_menu.cpp ---------------------------------------------------------------

	INT_PTR    __cdecl OnMenuConvertChatContact(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuRosterAdd(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuOptions(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuTransportLogin(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuTransportResolve(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuBookmarkAdd(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleRevokeAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnMenuHandleResource(WPARAM wParam, LPARAM lParam, LPARAM res);
	INT_PTR    __cdecl OnMenuHandleDirectPresence(WPARAM wParam, LPARAM lParam, LPARAM res);
	INT_PTR    __cdecl OnMenuSetPriority(WPARAM wParam, LPARAM lParam, LPARAM dwDelta);

	void   GlobalMenuInit(void);
	void   GlobalMenuUninit(void);

	void   MenuUpdateSrmmIcon(JABBER_LIST_ITEM *item);

	void   BuildPriorityMenu(void);
	void   UpdatePriorityMenu(int priority);

	HGENMENU m_hMenuPriorityRoot;
	int    m_priorityMenuVal;
	bool   m_priorityMenuValSet;

	//---- jabber_misc.c -----------------------------------------------------------------

	INT_PTR    __cdecl OnGetEventTextChatStates(WPARAM wParam, LPARAM lParam);
	INT_PTR    __cdecl OnGetEventTextPresence(WPARAM wParam, LPARAM lParam);

	void   AddContactToRoster(const char *jid, const char *nick, const char *grpName);
	void   DBAddAuthRequest(const char *jid, const char *nick);
	BOOL   AddDbPresenceEvent(MCONTACT hContact, BYTE btEventType);
	MCONTACT DBCreateContact(const char *jid, const char *nick, bool temporary, bool stripResource);
	void   GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen);
	void   ResolveTransportNicks(const char *jid);
	void   SetServerStatus(int iNewStatus);
	void   FormatMirVer(const pResourceStatus &resource, CMStringA &res);
	void   UpdateMirVer(JABBER_LIST_ITEM *item);
	void   UpdateMirVer(MCONTACT hContact, const pResourceStatus&);
	void   UpdateSubscriptionInfo(MCONTACT hContact, JABBER_LIST_ITEM *item);
	void   SetContactOfflineStatus(MCONTACT hContact);
	void   InitPopups(void);
	void   MsgPopup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle);
	CMStringA ExtractImage(const TiXmlElement *node);
	const char* GetSoftName(const char *wszName);

	//---- jabber_password.cpp --------------------------------------------------------------

	INT_PTR    __cdecl OnMenuHandleChangePassword(WPARAM wParam, LPARAM lParam);

	//---- jabber_privacy.cpp ------------------------------------------------------------

	INT_PTR __cdecl menuSetPrivacyList(WPARAM wParam, LPARAM lParam, LPARAM iList);
	INT_PTR __cdecl OnMenuHandlePrivacyLists(WPARAM wParam, LPARAM lParam);

	void   BuildPrivacyMenu(void);
	void   BuildPrivacyListsMenu(bool bDeleteOld);

	void   QueryPrivacyLists(ThreadData *pThreadInfo = nullptr);

	BOOL   OnIqRequestPrivacyLists(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyList(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyLists(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyListActive(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyListDefault(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultPrivacyListModify(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);

	//---- jabber_proto.cpp --------------------------------------------------------------

	void   __cdecl BasicSearchThread(struct JABBER_SEARCH_BASIC *jsb);
	void   __cdecl GetAwayMsgThread(void* hContact);
	void   __cdecl SendMessageAckThread(void* hContact);

	MCONTACT AddToListByJID(const char *newJid, DWORD flags);

	void   InfoFrame_OnSetup(CJabberInfoFrame_Event *evt);
	void   InfoFrame_OnTransport(CJabberInfoFrame_Event *evt);

	//---- jabber_rc.cpp -----------------------------------------------------------------

	int    RcGetUnreadEventsCount(void);

	//---- jabber_rostereditor.cpp -------------------------------------------------------

	CRosterEditorDlg *m_hwndRosterEditor;

	INT_PTR  __cdecl OnMenuHandleRosterControl(WPARAM wParam, LPARAM lParam);
	void   _RosterHandleGetRequest(const TiXmlElement *node, CJabberIqInfo*);

	//---- jabber_search.cpp -------------------------------------------------------------

	void   OnIqResultAdvancedSearch(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	void   OnIqResultGetSearchFields(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	int    SearchRenewFields(HWND hwndDlg, JabberSearchData * dat);
	void   SearchDeleteFromRecent(const char *szAddr, bool deleteLastFromDB);
	void   SearchAddToRecent(const char *szAddr, HWND hwndDialog = nullptr);

	//---- jabber_std.cpp ----------------------------------------------
	void   JLoginFailed(int errorCode);

	//---- jabber_svc.c ------------------------------------------------------------------

	void   CheckMenuItems();
	void   EnableMenuItems(bool bEnable);

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

	void   ExternalTempIqHandler(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   ExternalIqHandler(const TiXmlElement *node, CJabberIqInfo *pInfo);
	BOOL   ExternalMessageHandler(const TiXmlElement *node, ThreadData *pThreadData, CJabberMessageInfo* pInfo);
	BOOL   ExternalPresenceHandler(const TiXmlElement *node, ThreadData *pThreadData, CJabberPresenceInfo* pInfo);
	BOOL   ExternalSendHandler(const TiXmlElement *node, ThreadData *pThreadData, CJabberSendInfo* pInfo);

	BOOL SendHttpAuthReply(CJabberHttpAuthParams *pParams, BOOL bAuthorized);

	//---- jabber_thread.c ----------------------------------------------

	ptrW   m_savedPassword;

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
		char *m_gssapiHostName;
	} AUTHMECHS;

	AUTHMECHS m_AuthMechs;

	void   __cdecl ServerThread(JABBER_CONN_DATA *info);

	void   OnProcessFailure(const TiXmlElement *node, ThreadData *info);
	void   OnProcessFailed(const TiXmlElement *node, ThreadData *info);
	void   OnProcessEnabled(const TiXmlElement *node, ThreadData *info);
	void   OnProcessError(const TiXmlElement *node, ThreadData *info);
	void   OnProcessSuccess(const TiXmlElement *node, ThreadData *info);
	void   OnProcessChallenge(const TiXmlElement *node, ThreadData *info);
	void   OnProcessProceed(const TiXmlElement *node, ThreadData *info);
	void   OnProcessCompressed(const TiXmlElement *node, ThreadData *info);
	void   OnProcessMessage(const TiXmlElement *node, ThreadData *info);
	void   OnProcessPresence(const TiXmlElement *node, ThreadData *info);
	void   OnProcessPresenceCapabilites(const TiXmlElement *node, pResourceStatus &resource);
	void   OnProcessPubsubEvent(const TiXmlElement *node);
	void   OnProcessStreamOpening(const TiXmlElement *node, ThreadData *info);
	void   OnProcessProtocol(const TiXmlElement *node, ThreadData *info);

	void   UpdateJidDbSettings(const char *jid);
	MCONTACT CreateTemporaryContact(const char *szJid, JABBER_LIST_ITEM* chatItem);

	void   PerformRegistration(ThreadData *info);
	void   PerformIqAuth(ThreadData *info);
	void   PerformAuthentication(ThreadData *info);
	void   OnProcessFeatures(const TiXmlElement *node, ThreadData *info);

	void   xmlStreamInitialize(char *which);
	void   xmlStreamInitializeNow(ThreadData *info);

	BOOL   OnProcessJingle(const TiXmlElement *node);
	void   OnProcessIq(const TiXmlElement *node);
	void   SetRegConfig(CJabberFormDlg *pDlg, void *from);
	void   CancelRegConfig(CJabberFormDlg *pDlg, void *from);
	void   OnProcessRegIq(const TiXmlElement *node, ThreadData *info);
	void   OnPingReply(const TiXmlElement *node, CJabberIqInfo *pInfo);

	bool   ProcessCaptcha(const TiXmlElement *node, const TiXmlElement *parentNode, ThreadData *info);

	void   EnableCarbons(bool bEnable);

	//---- jabber_util.c -----------------------------------------------------------------
	pResourceStatus ResourceInfoFromJID(const char *jid);

	MCONTACT HContactFromJID(const char *jid, bool bStripResource = true);
	MCONTACT ChatRoomHContactFromJID(const char *jid);
	void   SendVisibleInvisiblePresence(bool invisible);
	void   SendPresenceTo(int status, const char* to, const TiXmlElement *extra = nullptr, const char *msg = nullptr);
	void   SendPresence(int m_iStatus, bool bSendToAll);

	void   RebuildInfoFrame(void);
	void   InitInfoFrame(void);

	// returns buf or nullptr on error
	char* GetClientJID(MCONTACT hContact, char *dest, size_t destLen);
	char* GetClientJID(const char *jid, char *dest, size_t destLen);

	void   ComboLoadRecentStrings(HWND hwndDlg, UINT idcCombo, char *param, int recentCount = JABBER_DEFAULT_RECENT_COUNT);
	void   ComboAddRecentString(HWND hwndDlg, UINT idcCombo, char *param, const wchar_t *string, int recentCount = JABBER_DEFAULT_RECENT_COUNT);
	BOOL   EnterString(CMStringW &result, const wchar_t *caption, int type, char *windowName=nullptr, int recentCount = JABBER_DEFAULT_RECENT_COUNT, int timeout = 0);
	bool   IsMyOwnJID(const char *szJID);

	void __cdecl LoadHttpAvatars(void* param);

	//---- jabber_vcard.c -----------------------------------------------

	int    m_vCardUpdates;
	bool   m_bPhotoChanged;
	wchar_t  m_szPhotoFileName[MAX_PATH];
	void   OnUserInfoInit_VCard(WPARAM, LPARAM);

	int    SendGetVcard(const char *jid);
	void   AppendVcardFromDB(TiXmlElement *n, char* tag, char* key);
	void   SetServerVcard(BOOL bPhotoChanged, wchar_t* szPhotoFileName);
	void   SaveVcardToDB(HWND hwndPage, int iPage);

	//---- jabber_ws.c -------------------------------------------------

	HNETLIBCONN WsConnect(char* host, WORD port);

	BOOL   WsInit(void);
	int    WsSend(HNETLIBCONN s, char* data, int datalen, int flags);
	int    WsRecv(HNETLIBCONN s, char* data, long datalen, int flags);

	//---- jabber_xml.c ------------------------------------------------------------------

	void   OnConsoleProcessXml(const TiXmlElement *node, DWORD flags);

	//---- jabber_xmlns.c ----------------------------------------------------------------

	BOOL   OnHandleDiscoInfoRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
	BOOL   OnHandleDiscoItemsRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);

	//---- jabber_xstatus.c --------------------------------------------------------------

	INT_PTR __cdecl OnSetListeningTo(WPARAM wParam, LPARAM lParams);
	INT_PTR __cdecl OnGetXStatusIcon(WPARAM wParam, LPARAM lParams);
	INT_PTR __cdecl OnGetXStatusEx(WPARAM wParam, LPARAM lParams);
	INT_PTR __cdecl OnSetXStatusEx(WPARAM wParam, LPARAM lParams);

	HICON  GetXStatusIcon(int bStatus, UINT flags);

	void   RegisterAdvStatusSlot(const char *pszSlot);
	void   ResetAdvStatus(MCONTACT hContact, const char *pszSlot);
	void   WriteAdvStatus(MCONTACT hContact, const char *pszSlot, const wchar_t *pszMode, const char *pszIcon, const wchar_t *pszTitle, const wchar_t *pszText);
	char*  ReadAdvStatusA(MCONTACT hContact, const char *pszSlot, const char *pszValue);
	wchar_t *ReadAdvStatusT(MCONTACT hContact, const char *pszSlot, const char *pszValue);

	BOOL   SendPepTune(wchar_t *szArtist, wchar_t *szLength, wchar_t *szSource, wchar_t *szTitle, wchar_t *szTrack, wchar_t *szUri);

	void   XStatusInit(void);

	void   SetContactTune(MCONTACT hContact,  const wchar_t *szArtist, const wchar_t *szLength, const wchar_t *szSource, const wchar_t *szTitle, const wchar_t *szTrack);

	void   InfoFrame_OnUserMood(CJabberInfoFrame_Event *evt);
	void   InfoFrame_OnUserActivity(CJabberInfoFrame_Event *evt);

	CPepServiceList m_pepServices;

private:
	char  *m_szXmlStreamToBeInitialized;

	DWORD  m_lastTicks;

	HANDLE m_hPopupClass;

	LONG   m_nSerial;

	HGENMENU   m_hPrivacyMenuRoot;
	bool       m_menuItemsStatus;
	LIST<void> m_hPrivacyMenuItems;

	int        m_nMenuResourceItems;
	HGENMENU  *m_phMenuResourceItems;

	JabberFeatCapPairDynamic* FindFeature(const char *szFeature);

public:
	DWORD     STDMETHODCALLTYPE GetFlags() const override;                      // Set of JIF_* flags.
	int       STDMETHODCALLTYPE GetVersion() const override;                    // Returns version of IJabberInterface.
	DWORD     STDMETHODCALLTYPE GetJabberVersion() const override;              // Returns Jabber plugin version.

	MCONTACT  STDMETHODCALLTYPE ContactFromJID(const char *jid) override;       // Returns contact handle for given JID.
	char*     STDMETHODCALLTYPE ContactToJID(MCONTACT hContact) override;       // Returns JID of hContact. You must free the result using mir_free().
	char*     STDMETHODCALLTYPE GetBestResourceName(const char *jid) override;  // Returns best resource name for given JID. You must free the result using mir_free().
	char*     STDMETHODCALLTYPE GetResourceList(const char *jid) override;      // Returns all resource names for a given JID in format "resource1\0resource2\0resource3\0\0" (all resources are separated by \0 character and the whole string is terminated with two \0 characters). You must free the string using mir_free().
	char*     STDMETHODCALLTYPE GetModuleName() const override;                 // Returns Jabber module name.

	int       STDMETHODCALLTYPE SerialNext() override;           // Returns id that can be used for next message sent through SendXmlNode().

	HJHANDLER STDMETHODCALLTYPE AddPresenceHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority) override;
	HJHANDLER STDMETHODCALLTYPE AddMessageHandler(JABBER_HANDLER_FUNC Func, int iMsgTypes, const char *szXmlns, const char *szTag, void *pUserData, int iPriority) override;
	HJHANDLER STDMETHODCALLTYPE AddIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, const char *szXmlns, const char *szTag, void *pUserData, int iPriority) override;
	HJHANDLER STDMETHODCALLTYPE AddSendHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority) override;
	int       STDMETHODCALLTYPE RemoveHandler(HJHANDLER hHandler) override;

	int       STDMETHODCALLTYPE RegisterFeature(const char *szFeature, const char *szDescription) override;
	int       STDMETHODCALLTYPE AddFeatures(const char *szFeatures) override;    // Adds features to the list of features returned by the client.
	int       STDMETHODCALLTYPE RemoveFeatures(const char *szFeatures) override; // Removes features from the list of features returned by the client.
	char*     STDMETHODCALLTYPE GetResourceFeatures(const char *jid) override;   // Returns all features supported by JID in format "feature1\0feature2\0...\0featureN\0\0". You must free returned string using mir_free().
	
	HNETLIBUSER STDMETHODCALLTYPE GetHandle() override;  // Returns connection handle
};

#endif
