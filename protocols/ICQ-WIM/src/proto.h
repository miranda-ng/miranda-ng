// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2019 Miranda NG team
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

#define ICQ_APP_ID "ic1nmMjqg7Yu-0hL"
#define ICQ_API_SERVER "https://u.icq.net/wim"
#define ICQ_FAKE_EVENT_ID 0xBABAEB
#define ICQ_ROBUST_SERVER "https://u.icq.net/rapi"

#define PS_DUMMY "/DoNothing"
#define PS_GOTO_INBOX "/GotoInbox"

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

struct IcqCacheItem : public MZeroedObject
{
	IcqCacheItem(const CMStringW &wszId, MCONTACT _contact) :
		m_aimid(wszId),
		m_hContact(_contact)
	{}

	CMStringW m_aimid;
	MCONTACT m_hContact;
	bool m_bInList;
	int m_iApparentMode;
	time_t m_timer1, m_timer2;
};

struct IcqOwnMessage
{
	IcqOwnMessage(MCONTACT _hContact, int _msgid, const char *guid)
		: m_hContact(_hContact), m_msgid(_msgid)
	{
		strncpy_s(m_guid, guid, _TRUNCATE);
	}

	MCONTACT m_hContact;
	int m_msgid;
	char m_guid[50];
};

struct IcqConn
{
	HNETLIBCONN s;
	int lastTs, timeout;
};

struct IcqFileTransfer : public MZeroedObject
{
	IcqFileTransfer(MCONTACT hContact, const wchar_t *pwszFileName) :
		m_wszFileName(pwszFileName)
	{
		pfts.flags = PFTS_UNICODE | PFTS_SENDING;
		pfts.hContact = hContact;
		pfts.szCurrentFile.w = m_wszFileName.GetBuffer();

		const wchar_t *p = wcsrchr(pfts.szCurrentFile.w, '\\');
		if (pwszFileName != nullptr)
			p++;
		else
			p = pfts.szCurrentFile.w;
		m_wszShortName = p;
	}

	~IcqFileTransfer()
	{
		if (m_fileId >= 0)
			_close(m_fileId);
	}

	int m_fileId = -1;
	CMStringA m_szHost;
	CMStringW m_wszFileName, m_wszDescr;
	const wchar_t *m_wszShortName;
	PROTOFILETRANSFERSTATUS pfts;

	void FillHeaders(AsyncHttpRequest *pReq)
	{
		pReq->AddHeader("Content-Type", "application/octet-stream");
		pReq->AddHeader("Content-Disposition", CMStringA(FORMAT, "attachment; filename=\"%s\"", T2Utf(m_wszShortName).get()));

		DWORD dwPortion = pfts.currentFileSize - pfts.currentFileProgress;
		if (dwPortion > 1000000)
			dwPortion = 1000000;

		pReq->AddHeader("Content-Range", CMStringA(FORMAT, "bytes %lld-%lld/%lld", pfts.currentFileProgress, pfts.currentFileProgress + dwPortion - 1, pfts.currentFileSize));
		pReq->AddHeader("Content-Length", CMStringA(FORMAT, "%d", dwPortion));

		pReq->dataLength = dwPortion;
		pReq->pData = (char*)mir_alloc(dwPortion);
		_lseek(m_fileId, pfts.currentFileProgress, SEEK_SET);
		_read(m_fileId, pReq->pData, dwPortion);

		pfts.currentFileProgress += dwPortion;
		pfts.totalProgress += dwPortion;
	}
};

class CIcqProto : public PROTO<CIcqProto>
{
	friend struct CIcqRegistrationDlg;
	friend class CGroupchatInviteDlg;
	friend class CEditIgnoreListDlg;
	friend class CIcqEnterLoginDlg;
	friend class CIcqOptionsDlg;
	friend class CGroupEditDlg;

	friend AsyncHttpRequest* operator <<(AsyncHttpRequest*, const AIMSID&);

	bool      m_bOnline, m_bTerminated, m_bFirstBos;
	int       m_iTimeShift;

	MCONTACT  CheckOwnMessage(const CMStringA &reqId, const CMStringA &msgId, bool bRemove);
	void      CheckPassword(void);
	void      ConnectionFailed(int iReason, int iErrorCode = 0);
	void      EmailNotification(const wchar_t *pwszText);
	void      GetPermitDeny();
	wchar_t*  GetUIN(MCONTACT hContact);
	void      MoveContactToGroup(MCONTACT hContact, const wchar_t *pwszGroup, const wchar_t *pwszNewGroup);
	bool      RetrievePassword();
	void      RetrieveUserHistory(MCONTACT, __int64 startMsgId, __int64 endMsgId = -1);
	void      RetrieveUserInfo(MCONTACT = INVALID_CONTACT_ID);
	void      SetServerStatus(int iNewStatus);
	void      ShutdownSession(void);
	void      StartSession(void);

	void      CheckAvatarChange(MCONTACT hContact, const JSONNode&);
	void      CheckLastId(MCONTACT hContact, const JSONNode&);
	void      Json2int(MCONTACT, const JSONNode&, const char *szJson, const char *szSetting);
	void      Json2string(MCONTACT, const JSONNode&, const char *szJson, const char *szSetting);
	MCONTACT  ParseBuddyInfo(const JSONNode &buddy, MCONTACT hContact = -1);
	void      ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &msg, bool bFromHistory);

	void      OnLoggedIn(void);
	void      OnLoggedOut(void);

	mir_cs    m_csMarkReadQueue;
	LIST<IcqCacheItem> m_arMarkReadQueue;
	static    void CALLBACK MarkReadTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD);

	AsyncHttpRequest* UserInfoRequest(MCONTACT);

	__int64   getId(MCONTACT hContact, const char *szSetting);
	void      setId(MCONTACT hContact, const char *szSetting, __int64 iValue);

	void      OnAddBuddy(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnAddClient(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnCheckPassword(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnCheckPhone(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnFetchEvents(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnGetChatInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnGetPermitDeny(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnGetUserHistory(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnGetUserInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnFileContinue(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnFileInit(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnLoginViaPhone(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnNormalizePhone(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnReceiveAvatar(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnSearchResults(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnSendMessage(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnStartSession(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void      OnValidateSms(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void      ProcessBuddyList(const JSONNode&);
	void      ProcessDiff(const JSONNode&);
	void      ProcessEvent(const JSONNode&);
	void      ProcessGroupChat(const JSONNode&);
	void      ProcessHistData(const JSONNode&);
	void      ProcessImState(const JSONNode&);
	void      ProcessMyInfo(const JSONNode&);
	void      ProcessNotification(const JSONNode&);
	void      ProcessPermissions(const JSONNode&);
	void      ProcessPresence(const JSONNode&);
	void      ProcessSessionEnd(const JSONNode&);
	void      ProcessTyping(const JSONNode&);

	IcqConn   m_ConnPool[CONN_LAST];
	CMStringA m_szPassword;
	CMStringA m_szSessionKey;
	CMStringA m_szAToken;
	CMStringA m_szRToken;
	CMStringA m_fetchBaseURL;
	CMStringA m_aimsid;
	LONG      m_msgId = 1;
	int       m_iRClientId;
	HGENMENU  m_hUploadGroups;

	mir_cs    m_csOwnIds;
	OBJLIST<IcqOwnMessage> m_arOwnIds;

	OBJLIST<IcqGroup> m_arGroups;

	CIcqDlgBase *m_pdlgEditIgnore;	

	int       m_unreadEmails = -1;
	CMStringA m_szMailBox;

	bool      m_bIgnoreListEmpty = true;
	bool      m_bRememberPwd; // store password in a database
	bool      m_bDlgActive;

	////////////////////////////////////////////////////////////////////////////////////////
	// group chats

	int       __cdecl GroupchatEventHook(WPARAM, LPARAM);
	int       __cdecl GroupchatMenuHook(WPARAM, LPARAM);

	void      Chat_ProcessLogMenu(SESSION_INFO *si, int);
	void      Chat_SendPrivateMessage(GCHOOK *gch);

	void      InviteUserToChat(SESSION_INFO *si);
	void      LeaveDestroyChat(SESSION_INFO *si);
	void      LoadChatInfo(SESSION_INFO *si);

	////////////////////////////////////////////////////////////////////////////////////////
	// http queue

	mir_cs    m_csHttpQueue;
	HANDLE    m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;

	void      CalcHash(AsyncHttpRequest*);
	bool      ExecuteRequest(AsyncHttpRequest*);
	bool      IsQueueEmpty();
	void      Push(MHttpRequest*);
	bool      RefreshRobustToken();

	////////////////////////////////////////////////////////////////////////////////////////
	// cache

	mir_cs    m_csCache;
	OBJLIST<IcqCacheItem> m_arCache;

	void      InitContactCache(void);
	IcqCacheItem* FindContactByUIN(const CMStringW &pwszId);
	MCONTACT  CreateContact(const CMStringW &pwszId, bool bTemporary);

	void      GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen);

	////////////////////////////////////////////////////////////////////////////////////////
	// threads

	HANDLE    m_hWorkerThread;
	void      __cdecl ServerThread(void*);

	HANDLE    m_hPollThread;
	void      __cdecl PollThread(void*);

	////////////////////////////////////////////////////////////////////////////////////////
	// services

	INT_PTR   __cdecl GetAvatar(WPARAM, LPARAM);
	INT_PTR   __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR   __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR   __cdecl SetAvatar(WPARAM, LPARAM);
	
	INT_PTR   __cdecl CreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR   __cdecl EditGroups(WPARAM, LPARAM);
	INT_PTR   __cdecl GetEmailCount(WPARAM, LPARAM);
	INT_PTR   __cdecl GotoInbox(WPARAM, LPARAM);
	INT_PTR   __cdecl UploadGroups(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// events

	int       __cdecl OnGroupChange(WPARAM, LPARAM);
	int       __cdecl OnDbEventRead(WPARAM, LPARAM);
	int       __cdecl OnOptionsInit(WPARAM, LPARAM);
	int       __cdecl OnUserInfoInit(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT  AddToList( int flags, PROTOSEARCHRESULT *psr) override;
			    
	int       AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	INT_PTR   GetCaps(int type, MCONTACT hContact = NULL) override;
	int       GetInfo(MCONTACT hContact, int infoType) override;
			    
	HANDLE    SearchBasic(const wchar_t *id) override;
			    
	HANDLE    SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int       SendMsg(MCONTACT hContact, int flags, const char *msg) override;
			    
	int       SetApparentMode(MCONTACT hContact, int mode) override;
	int       SetStatus(int iNewStatus) override;
			    
	HANDLE    GetAwayMsg(MCONTACT hContact) override;
	int       RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt) override;
	int       SetAwayMsg(int m_iStatus, const wchar_t *msg) override;
			    
	int       UserIsTyping(MCONTACT hContact, int type) override;
			    
	void      OnBuildProtoMenu(void) override;
	void      OnContactDeleted(MCONTACT) override;
	void      OnModulesLoaded() override;
	void      OnShutdown() override;

public:
	CIcqProto(const char*, const wchar_t*);
	~CIcqProto();

	CMOption<wchar_t*> m_szOwnId;      // our own aim id
	CMOption<BYTE>  m_bHideGroupchats; // don't pop up group chat windows on startup
	CMOption<BYTE>  m_bUseTrayIcon;    // use tray icon notifications
	CMOption<BYTE>  m_bErrorPopups;    // display popups with errors
	CMOption<BYTE>  m_bLaunchMailbox;  // launch browser to view email
	CMOption<DWORD> m_iTimeDiff1;		  // set this status to m_iStatus1 after this interval of secs
	CMOption<DWORD> m_iStatus1;
	CMOption<DWORD> m_iTimeDiff2;		  // set this status to m_iStatus2 after this interval of secs
	CMOption<DWORD> m_iStatus2;

	void CheckStatus(void);
	CMStringW GetUserId(MCONTACT);

	__forceinline int TS() const
	{	return time(0) - m_iTimeShift;
	}

	void SetPermitDeny(const CMStringW &userId, bool bAllow);
};

struct CMPlugin : public ACCPROTOPLUGIN<CIcqProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
