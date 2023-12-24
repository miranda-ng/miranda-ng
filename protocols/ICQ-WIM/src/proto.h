// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2023 Miranda NG team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Protocol Interface declarations
// -----------------------------------------------------------------------------

#ifndef _ICQ_PROTO_H_
#define _ICQ_PROTO_H_

#include "m_system.h"
#include "m_protoint.h"

#define MRA_APP_ID "ic1pzYNtEU6dDnEQ"
#define ICQ_APP_ID "ic1nmMjqg7Yu-0hL"
#define ICQ_FILE_SERVER "https://u.icq.net/files/api/v1.1"
#define ICQ_FAKE_EVENT_ID 0xBABAEB
#define ICQ_ROBUST_SERVER "https://u.icq.net/rapi"

#define PS_DUMMY "/DoNothing"
#define PS_GOTO_INBOX "/SvcGotoInbox"

#define WIM_CAP_VOIP_VOICE         "094613504c7f11d18222444553540000"
#define WIM_CAP_VOIP_VIDEO         "094613514c7f11d18222444553540000"
#define WIM_CAP_FILETRANSFER       "094613434c7f11d18222444553540000"
#define WIM_CAP_UNIQ_REQ_ID        "094613534c7f11d18222444553540000"
#define WIM_CAP_EMOJI              "094613544c7f11d18222444553540000"
#define WIM_CAP_MENTIONS           "0946135b4c7f11d18222444553540000"
#define WIM_CAP_MAIL_NOTIFICATIONS "094613594c7f11d18222444553540000"
#define WIM_CAP_INTRO_DLG_STATE    "0946135a4c7f11d18222444553540000"

#define NG_CAP_SECUREIM            "4d69724e47536563757265494d000000"

typedef CProtoDlgBase<CIcqProto> CIcqDlgBase;

struct AIMSID
{
	AIMSID(CIcqProto *_ppro) :
		m_ppro(_ppro)
	{}

	CIcqProto *m_ppro;
};

enum ChatMenuItems
{
	IDM_INVITE = 10, IDM_LEAVE
};

struct IcqFileInfo
{
	IcqFileInfo(const std::string &pszUrl, const CMStringW &pwszDescr, uint32_t dwSize) :
		szUrl(pszUrl.c_str()),
		wszDescr(pwszDescr),
		dwFileSize(dwSize)
	{}

	CMStringA szUrl, szOrigUrl;
	CMStringW wszDescr;
	uint32_t dwFileSize;
	bool bIsSticker = false;
};

struct IcqGroup
{
	IcqGroup(int _p1, const CMStringW &_p2) :
		id(_p1),
		wszSrvName(_p2)
	{
		SetName(_p2);
	}

	int id;
	int level;
	CMStringW wszName, wszSrvName;

	void SetName(const CMStringW &str)
	{
		wszName = str;
		level = wszName.SpanIncluding(L">").GetLength();
		if (level != 0)
			wszName.Delete(0, level);
		wszName.Replace(L">", L"\\");
	}
};

struct IcqUser : public MZeroedObject
{
	IcqUser(const CMStringW &wszId, MCONTACT _contact) :
		m_aimid(wszId),
		m_hContact(_contact)
	{}

	CMStringW m_aimid;
	MCONTACT  m_hContact;
	bool      m_bInList, m_bGotCaps, m_bWasOnline;
	__int64   m_iProcessedMsgId;
	int       m_iApparentMode;
	time_t    m_timer1, m_timer2;
};

struct IcqConn
{
	HNETLIBCONN s;
	int lastTs, timeout;
};

struct IcqFileTransfer : public MZeroedObject
{
	bool m_bCanceled = false, m_bStarted = false;
	int m_fileId = -1;
	CMStringA m_szHost, m_szMsgId;
	CMStringW m_wszFileName, m_wszDescr;
	const wchar_t *m_wszShortName;
	PROTOFILETRANSFERSTATUS pfts;

	// create an object for sending
	IcqFileTransfer(MCONTACT hContact, const wchar_t *pwszFileName);

	~IcqFileTransfer();

	void FillHeaders(AsyncHttpRequest *pReq);
};

struct IcqOwnMessage
{
	IcqOwnMessage(MCONTACT _hContact, int _msgid, const char *pszText) :
		m_msgid(_msgid),
		m_hContact(_hContact),
		m_szText(mir_strdup(pszText))
	{
	}

	void setGuid(const char *pszGuid)
	{
		strncpy_s(m_guid, pszGuid, _TRUNCATE);
	}

	MCONTACT m_hContact;
	int m_msgid;
	char m_guid[50];
	ptrA m_szText;
	IcqFileTransfer *pTransfer = nullptr;
};

class CIcqProto : public PROTO<CIcqProto>
{
	friend class CForwardDlg;
	friend struct AsyncRapiRequest;

	class CIcqProtoImpl
	{
		friend class CIcqProto;

		CIcqProto &m_proto;
		CTimer m_heartBeat, m_markRead;
		
		void OnHeartBeat(CTimer *) {
			m_proto.CheckStatus();
		}

		void OnMarkRead(CTimer *pTimer) {
			m_proto.SendMarkRead();
			pTimer->Stop();
		}

		CIcqProtoImpl(CIcqProto &pro) :
			m_proto(pro),
			m_markRead(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_heartBeat(Miranda_GetSystemWindow(), UINT_PTR(this) + 1)
		{
			m_markRead.OnEvent = Callback(this, &CIcqProtoImpl::OnMarkRead);
			m_heartBeat.OnEvent = Callback(this, &CIcqProtoImpl::OnHeartBeat);
		}
	} m_impl;

	friend struct CIcqRegistrationDlg;
	friend class CGroupchatInviteDlg;
	friend class CEditIgnoreListDlg;
	friend class COptionsDlg;
	friend class CIcqEnterLoginDlg;
	friend class CGroupEditDlg;

	friend AsyncHttpRequest* operator <<(AsyncHttpRequest*, const AIMSID&);

	bool          m_bOnline, m_bTerminated, m_bFirstBos, m_isMra, m_bError462;
	int           m_iTimeShift;
				     
	MCONTACT      CheckOwnMessage(const CMStringA &reqId, const CMStringA &msgId, bool bRemove);
	void          CheckPassword(void);
	void          ConnectionFailed(int iReason, int iErrorCode = 0);
	void          EmailNotification(const wchar_t *pwszText);
	void          ForwardMessage(MEVENT hEVent, MCONTACT hContact);
	void          GetPermitDeny();
	wchar_t*      GetUIN(MCONTACT hContact);
	void          MoveContactToGroup(MCONTACT hContact, const wchar_t *pwszGroup, const wchar_t *pwszNewGroup);
	void          RetrieveHistoryChunk(MCONTACT hContact, __int64 patchVer, __int64 startMsgId, unsigned iCount);
	bool          RetrievePassword();
	void          RetrievePresence(MCONTACT hContact);
	void          RetrieveUserCaps(IcqUser *pUser);
	void          RetrieveUserHistory(MCONTACT, __int64 startMsgId, bool bCreateRead);
	void          RetrieveUserInfo(MCONTACT hContact);
	void          SendMrimLogin(NETLIBHTTPREQUEST *pReply);
	void          SendMessageParts(MCONTACT hContact, const JSONNode &parts, IcqOwnMessage *pOwn = nullptr);
	void          SetOwnId(const CMStringW &wszId);
	void          SetServerStatus(int iNewStatus);
	void          ShutdownSession(void);
	void          StartSession(void);
				     
	void          CheckAvatarChange(MCONTACT hContact, const JSONNode&);
	bool          CheckFile(MCONTACT hContact, CMStringW &wszFileName, IcqFileInfo* &pFileInfo);
	void          CheckLastId(MCONTACT hContact, const JSONNode&);
	void          Json2int(MCONTACT, const JSONNode&, const char *szJson, const char *szSetting, bool bIsPartial);
	void          Json2string(MCONTACT, const JSONNode&, const char *szJson, const char *szSetting, bool bIsPartial);
	MCONTACT      ParseBuddyInfo(const JSONNode &buddy, MCONTACT hContact = INVALID_CONTACT_ID, bool bIsPartial = false);
	void          ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &msg, bool bCreateRead, bool bLocalTime);
	IcqFileInfo*  RetrieveFileInfo(MCONTACT hContact, const CMStringW &wszUrl);
	int           StatusFromPresence(const JSONNode &presence, MCONTACT hContact);
	void          ProcessPatchVersion(MCONTACT hContact, __int64 currPatch);
	void          ProcessStatus(IcqUser *pUser, int iStatus);
				     
	void          OnLoggedIn(void);
	void          OnLoggedOut(void);

	mir_cs        m_csMarkReadQueue;
	LIST<IcqUser> m_arMarkReadQueue;
	void          SendMarkRead();

	__int64       getId(MCONTACT hContact, const char *szSetting);
	void          setId(MCONTACT hContact, const char *szSetting, __int64 iValue);
				     
	void          OnAddBuddy(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnAddClient(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnCheckMraAuth(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnCheckMraAuthFinal(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnCheckMrimLogin(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnCheckPassword(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnCheckPhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnFetchEvents(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnFileContinue(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnFileInit(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnFileInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnFileRecv(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGenToken(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetChatInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetPatches(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetPermitDeny(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGePresence(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetSticker(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetUserCaps(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetUserHistory(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnGetUserInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnLeaveChat(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnLoginViaPhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnNormalizePhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnReceiveAvatar(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnSearchResults(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnSendMessage(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnSessionEnd(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void          OnValidateSms(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
				     
	void          ProcessBuddyList(const JSONNode &pRoot);
	void          ProcessDiff(const JSONNode &pRoot);
	void          ProcessEvent(const JSONNode &pRoot);
	void          ProcessGroupChat(const JSONNode &pRoot);
	void          ProcessHistData(const JSONNode &pRoot);
	void          ProcessImState(const JSONNode &pRoot);
	void          ProcessMyInfo(const JSONNode &pRoot);
	void          ProcessNotification(const JSONNode &pRoot);
	void          ProcessOnline(const JSONNode &presence, MCONTACT hContact);
	void          ProcessPermissions(const JSONNode &pRoot);
	void          ProcessPresence(const JSONNode &pRoot);
	void          ProcessSessionEnd(const JSONNode &pRoot);
	void          ProcessTyping(const JSONNode &pRoot);

	IcqConn       m_ConnPool[CONN_LAST];
	CMStringA     m_szPassword;
	CMStringA     m_szSessionKey;
	CMStringA     m_szAToken;
	CMStringA     m_szRToken;
	CMStringA     m_fetchBaseURL;
	CMStringA     m_aimsid;
	CMStringA     m_szMraCookie;
	LONG          m_msgId = 1;
	int           m_iRClientId;
	HGENMENU      m_hUploadGroups;
	MCONTACT      m_hFavContact = INVALID_CONTACT_ID;

	mir_cs        m_csOwnIds;
	OBJLIST<IcqOwnMessage> m_arOwnIds;

	OBJLIST<IcqGroup> m_arGroups;

	int           m_unreadEmails = -1;
	CMStringA     m_szMailBox;

	bool          m_bIgnoreListEmpty = true;
	bool          m_bRememberPwd; // store password in a database
	bool          m_bDlgActive;

	////////////////////////////////////////////////////////////////////////////////////////
	// group chats

	int       __cdecl GcEventHook(WPARAM, LPARAM);
	int       __cdecl GcMenuHook(WPARAM, LPARAM);

	void      GcProcessLogMenu(SESSION_INFO *si, int);
	void      GcSendPrivateMessage(GCHOOK *gch);

	SESSION_INFO* GcCreate(const wchar_t *pwszId, const wchar_t *pwszNick);

	void      RetrieveChatInfo(MCONTACT hContact);

	void      InviteUserToChat(SESSION_INFO *si);
	void      LeaveDestroyChat(SESSION_INFO *si);

	////////////////////////////////////////////////////////////////////////////////////////
	// http queue

	mir_cs    m_csHttpQueue;
	HANDLE    m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;

	void      CalcHash(AsyncHttpRequest*);
	void      DropQueue();
	bool      ExecuteRequest(AsyncHttpRequest*);
	bool      IsQueueEmpty();
	void      Push(MHttpRequest*);
	bool      RefreshRobustToken(AsyncHttpRequest *pReq);

	////////////////////////////////////////////////////////////////////////////////////////
	// cache

	mir_cs    m_csCache;
	OBJLIST<IcqUser> m_arCache;

	void      InitContactCache(void);
	IcqUser*  FindUser(const CMStringW &pwszId);
	MCONTACT  CreateContact(const CMStringW &pwszId, bool bTemporary);
	
	void      GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen);

	////////////////////////////////////////////////////////////////////////////////////////
	// Menus

	HGENMENU  hmiForward, hmiConvert;

	void      InitMenus();

	INT_PTR   __cdecl SvcExecMenu(WPARAM, LPARAM);
	int       __cdecl OnPrebuildMenu(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// threads

	HANDLE    m_hWorkerThread;
	void      __cdecl ServerThread(void*);
	void      __cdecl PollThread(void*);
	void      __cdecl OfflineFileThread(void*);

	////////////////////////////////////////////////////////////////////////////////////////
	// services

	INT_PTR   __cdecl GetAvatar(WPARAM, LPARAM);
	INT_PTR   __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR   __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR   __cdecl SetAvatar(WPARAM, LPARAM);
	
	INT_PTR   __cdecl SvcLeaveChat(WPARAM, LPARAM);
	INT_PTR   __cdecl SvcOfflineFile(WPARAM, LPARAM);

	INT_PTR   __cdecl EditGroups(WPARAM, LPARAM);
	INT_PTR   __cdecl EditProfile(WPARAM, LPARAM);
	INT_PTR   __cdecl SvcGetEmailCount(WPARAM, LPARAM);
	INT_PTR   __cdecl SvcGotoInbox(WPARAM, LPARAM);
	INT_PTR   __cdecl UploadGroups(WPARAM, LPARAM);

	INT_PTR   __cdecl SvcLoadHistory(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// events

	int       __cdecl OnGroupChange(WPARAM, LPARAM);
	int       __cdecl OnOptionsInit(WPARAM, LPARAM);
	int       __cdecl OnUserInfoInit(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT  AddToList( int flags, PROTOSEARCHRESULT *psr) override;
			    
	int       AuthRecv(MCONTACT, PROTORECVEVENT *pre) override;
	int       AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	INT_PTR   GetCaps(int type, MCONTACT hContact = NULL) override;
	int       GetInfo(MCONTACT hContact, int infoType) override;
			    
	HANDLE    SearchBasic(const wchar_t *id) override;

	HANDLE    SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int       SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;
			    
	int       SetApparentMode(MCONTACT hContact, int mode) override;
	int       SetStatus(int iNewStatus) override;
			    
	int       UserIsTyping(MCONTACT hContact, int type) override;
			    
	void      OnBuildProtoMenu(void) override;
	void      OnContactAdded(MCONTACT) override;
	bool      OnContactDeleted(MCONTACT) override;
	MWindow   OnCreateAccMgrUI(MWindow) override;
	void      OnEventEdited(MCONTACT, MEVENT, const DBEVENTINFO &dbei) override;
	void      OnMarkRead(MCONTACT, MEVENT) override;
	void      OnModulesLoaded() override;
	void      OnReceiveOfflineFile(DB::FILE_BLOB &blob, void *ft) override;
	void      OnSendOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob, void *ft) override;
	void      OnShutdown() override;

public:
	CIcqProto(const char*, const wchar_t*);
	~CIcqProto();

	CMOption<wchar_t*> m_szOwnId;         // our own aim id

	CMOption<bool>     m_bHideGroupchats; // don't pop up group chat windows on startup
	CMOption<bool>     m_bUseTrayIcon;    // use tray icon notifications
	CMOption<bool>     m_bErrorPopups;    // display popups with errors
	CMOption<bool>     m_bLaunchMailbox;  // launch browser to view email

	CMOption<uint32_t> m_iTimeDiff1;		  // set this status to m_iStatus1 after this interval of secs
	CMOption<uint32_t> m_iStatus1;
	CMOption<uint32_t> m_iTimeDiff2;		  // set this status to m_iStatus2 after this interval of secs
	CMOption<uint32_t> m_iStatus2;

	void CheckStatus(void);
	MCONTACT GetRealContact(IcqUser *pUser);
	CMStringW GetUserId(MCONTACT);

	__forceinline int TS() const
	{	return time(0) - m_iTimeShift;
	}

	__forceinline const char *appId() const
	{	return (m_isMra) ? MRA_APP_ID : ICQ_APP_ID;
	}

	void SetPermitDeny(const CMStringW &userId, bool bAllow);
};

struct CMPlugin : public ACCPROTOPLUGIN<CIcqProto>
{
	CMPlugin();

	int Load() override;
};

#endif
