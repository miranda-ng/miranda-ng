// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2022 Miranda NG team
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
#define ICQ_API_SERVER "https://u.icq.net/api/v17/wim"
#define ICQ_FILE_SERVER "https://u.icq.net/files/api/v1.1"
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

struct IcqFileInfo
{
	IcqFileInfo(const std::string &pszUrl, const CMStringW &pwszDescr, uint32_t dwSize) :
		szUrl(pszUrl.c_str()),
		wszDescr(pwszDescr),
		dwFileSize(dwSize)
	{}

	CMStringA szUrl;
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

struct IcqCacheItem : public MZeroedObject
{
	IcqCacheItem(const CMStringW &wszId, MCONTACT _contact) :
		m_aimid(wszId),
		m_hContact(_contact)
	{}

	CMStringW m_aimid;
	MCONTACT  m_hContact;
	bool      m_bInList;
	__int64   m_iProcessedMsgId;
	int       m_iApparentMode;
	time_t    m_timer1, m_timer2;
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
	// create an object for receiving
	IcqFileTransfer(MCONTACT hContact, const char *pszUrl) :
		m_szHost(pszUrl)
	{
		pfts.hContact = hContact;
		pfts.totalFiles = 1;
		pfts.flags = PFTS_UNICODE | PFTS_RECEIVING;

		ptrW pwszFileName(mir_utf8decodeW(pszUrl));
		if (pwszFileName == nullptr)
			pwszFileName = mir_a2u(pszUrl);

		const wchar_t *p = wcsrchr(pwszFileName, '/');
		m_wszFileName = (p == nullptr) ? pwszFileName : p + 1;
		m_wszShortName = m_wszFileName;
	}

	// create an object for sending
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

	bool m_bCanceled = false, m_bStarted = false;
	int m_fileId = -1;
	CMStringA m_szHost;
	CMStringW m_wszFileName, m_wszDescr;
	const wchar_t *m_wszShortName;
	PROTOFILETRANSFERSTATUS pfts;
	HANDLE hWaitEvent;

	void FillHeaders(AsyncHttpRequest *pReq)
	{
		pReq->AddHeader("Content-Type", "application/octet-stream");
		pReq->AddHeader("Content-Disposition", CMStringA(FORMAT, "attachment; filename=\"%s\"", T2Utf(m_wszShortName).get()));

		uint32_t dwPortion = pfts.currentFileSize - pfts.currentFileProgress;
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
	friend class CIcqEnterLoginDlg;
	friend class CIcqOptionsDlg;
	friend class CGroupEditDlg;

	friend AsyncHttpRequest* operator <<(AsyncHttpRequest*, const AIMSID&);

	bool         m_bOnline, m_bTerminated, m_bFirstBos, m_isMra, m_bError462;
	int          m_iTimeShift;
				    
	MCONTACT     CheckOwnMessage(const CMStringA &reqId, const CMStringA &msgId, bool bRemove);
	void         CheckPassword(void);
	void         ConnectionFailed(int iReason, int iErrorCode = 0);
	void         EmailNotification(const wchar_t *pwszText);
	void         GetPermitDeny();
	wchar_t*     GetUIN(MCONTACT hContact);
	void         MarkAsRead(MCONTACT hContact);
	void         MoveContactToGroup(MCONTACT hContact, const wchar_t *pwszGroup, const wchar_t *pwszNewGroup);
	bool         RetrievePassword();
	void         RetrieveUserHistory(MCONTACT, __int64 startMsgId, bool bCreateRead);
	void         RetrieveUserInfo(MCONTACT = INVALID_CONTACT_ID);
	void         SendMrimLogin(NETLIBHTTPREQUEST *pReply);
	void         SetServerStatus(int iNewStatus);
	void         ShutdownSession(void);
	void         StartSession(void);
				    
	void         CheckAvatarChange(MCONTACT hContact, const JSONNode&);
	IcqFileInfo* CheckFile(MCONTACT hContact, CMStringW &wszFileName, bool &bIsFile);
	void         CheckLastId(MCONTACT hContact, const JSONNode&);
	void         Json2int(MCONTACT, const JSONNode&, const char *szJson, const char *szSetting, bool bIsPartial);
	void         Json2string(MCONTACT, const JSONNode&, const char *szJson, const char *szSetting, bool bIsPartial);
	MCONTACT     ParseBuddyInfo(const JSONNode &buddy, MCONTACT hContact = INVALID_CONTACT_ID, bool bIsPartial = false);
	void         ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &msg, bool bCreateRead, bool bLocalTime);
	int          StatusFromPresence(const JSONNode &presence, MCONTACT hContact);
				    
	void         OnLoggedIn(void);
	void         OnLoggedOut(void);

	mir_cs       m_csMarkReadQueue;
	LIST<IcqCacheItem> m_arMarkReadQueue;
	void         SendMarkRead();

	AsyncHttpRequest* UserInfoRequest(MCONTACT);

	__int64   getId(MCONTACT hContact, const char *szSetting);
	void      setId(MCONTACT hContact, const char *szSetting, __int64 iValue);

	void      OnAddBuddy(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnAddClient(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnCheckMraAuth(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnCheckMraAuthFinal(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnCheckMrimLogin(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnCheckPassword(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnCheckPhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnFetchEvents(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnFileContinue(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnFileInit(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnFileInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnFileRecv(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnGenToken(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnGetChatInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnGetPermitDeny(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnGetSticker(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnGetUserHistory(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnGetUserInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnLoginViaPhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnNormalizePhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnReceiveAvatar(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnSearchResults(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnSendMessage(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnSessionEnd(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);
	void      OnValidateSms(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq);

	void      ProcessBuddyList(const JSONNode &pRoot);
	void      ProcessDiff(const JSONNode &pRoot);
	void      ProcessEvent(const JSONNode &pRoot);
	void      ProcessGroupChat(const JSONNode &pRoot);
	void      ProcessHistData(const JSONNode &pRoot);
	void      ProcessImState(const JSONNode &pRoot);
	void      ProcessMyInfo(const JSONNode &pRoot);
	void      ProcessNotification(const JSONNode &pRoot);
	void      ProcessPermissions(const JSONNode &pRoot);
	void      ProcessPresence(const JSONNode &pRoot);
	void      ProcessSessionEnd(const JSONNode &pRoot);
	void      ProcessTyping(const JSONNode &pRoot);

	IcqConn   m_ConnPool[CONN_LAST];
	CMStringA m_szPassword;
	CMStringA m_szSessionKey;
	CMStringA m_szAToken;
	CMStringA m_szRToken;
	CMStringA m_fetchBaseURL;
	CMStringA m_aimsid;
	CMStringA m_szMraCookie;
	LONG      m_msgId = 1;
	int       m_iRClientId;
	HGENMENU  m_hUploadGroups;

	mir_cs    m_csOwnIds;
	OBJLIST<IcqOwnMessage> m_arOwnIds;

	OBJLIST<IcqGroup> m_arGroups;

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
	void      DropQueue();
	bool      ExecuteRequest(AsyncHttpRequest*);
	bool      IsQueueEmpty();
	void      Push(MHttpRequest*);
	bool      RefreshRobustToken(AsyncHttpRequest *pReq);

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
	void      __cdecl PollThread(void*);

	////////////////////////////////////////////////////////////////////////////////////////
	// services

	INT_PTR   __cdecl GetAvatar(WPARAM, LPARAM);
	INT_PTR   __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR   __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR   __cdecl SetAvatar(WPARAM, LPARAM);
	
	INT_PTR   __cdecl CreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR   __cdecl EditGroups(WPARAM, LPARAM);
	INT_PTR   __cdecl EditProfile(WPARAM, LPARAM);
	INT_PTR   __cdecl GetEmailCount(WPARAM, LPARAM);
	INT_PTR   __cdecl GotoInbox(WPARAM, LPARAM);
	INT_PTR   __cdecl UploadGroups(WPARAM, LPARAM);

	INT_PTR   __cdecl OnMenuLoadHistory(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// events

	int       __cdecl OnGroupChange(WPARAM, LPARAM);
	int       __cdecl OnDbEventRead(WPARAM, LPARAM);
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

	HANDLE    FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath) override;
	int       FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int       FileResume(HANDLE hTransfer, int action, const wchar_t *szFilename) override;

	HANDLE    SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int       SendMsg(MCONTACT hContact, int flags, const char *msg) override;
			    
	int       SetApparentMode(MCONTACT hContact, int mode) override;
	int       SetStatus(int iNewStatus) override;
			    
	int       UserIsTyping(MCONTACT hContact, int type) override;
			    
	void      OnBuildProtoMenu(void) override;
	void      OnContactAdded(MCONTACT) override;
	void      OnContactDeleted(MCONTACT) override;
	void      OnEventEdited(MCONTACT, MEVENT) override;
	void      OnModulesLoaded() override;
	void      OnShutdown() override;

public:
	CIcqProto(const char*, const wchar_t*);
	~CIcqProto();

	CMOption<wchar_t*> m_szOwnId;      // our own aim id
	CMOption<uint8_t>  m_bHideGroupchats; // don't pop up group chat windows on startup
	CMOption<uint8_t>  m_bUseTrayIcon;    // use tray icon notifications
	CMOption<uint8_t>  m_bErrorPopups;    // display popups with errors
	CMOption<uint8_t>  m_bLaunchMailbox;  // launch browser to view email
	CMOption<uint32_t> m_iTimeDiff1;		  // set this status to m_iStatus1 after this interval of secs
	CMOption<uint32_t> m_iStatus1;
	CMOption<uint32_t> m_iTimeDiff2;		  // set this status to m_iStatus2 after this interval of secs
	CMOption<uint32_t> m_iStatus2;

	void CheckStatus(void);
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
