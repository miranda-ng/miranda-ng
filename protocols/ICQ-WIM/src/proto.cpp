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
//  Protocol Interface Implementation
// -----------------------------------------------------------------------------

#include "stdafx.h"

#include "m_icolib.h"

#pragma warning(disable:4355)

static int CompareCache(const IcqCacheItem *p1, const IcqCacheItem *p2)
{
	return mir_wstrcmp(p1->m_aimid, p2->m_aimid);
}

CIcqProto::CIcqProto(const char* aProtoName, const wchar_t* aUserName) :
	PROTO<CIcqProto>(aProtoName, aUserName),
	m_arHttpQueue(10),
	m_arOwnIds(1, PtrKeySortT),
	m_arCache(20, &CompareCache),
	arMarkReadQueue(10, NumericKeySortT),
	m_evRequestsQueue(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_szOwnId(this, DB_KEY_ID),
	m_iStatus1(this, "Status1", ID_STATUS_AWAY),
	m_iStatus2(this, "Status2", ID_STATUS_NA),
	m_iTimeDiff1(this, "TimeDiff1", 0),
	m_iTimeDiff2(this, "TimeDiff2", 0),
	m_szPassword(this, "Password"),
	m_bHideGroupchats(this, "HideChats", true),
	m_bUseTrayIcon(this, "UseTrayIcon", false),
	m_bErrorPopups(this, "ShowErrorPopups", true),
	m_bLaunchMailbox(this, "LaunchMailbox", true)
{
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "OnlineTS");

	// services
	CreateProtoService(PS_CREATEACCMGRUI, &CIcqProto::CreateAccMgrUI);
	CreateProtoService(PS_GETAVATARCAPS, &CIcqProto::GetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFO, &CIcqProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CIcqProto::GetAvatar);
	CreateProtoService(PS_GETUNREADEMAILCOUNT, &CIcqProto::GetEmailCount);
	CreateProtoService(PS_GOTO_INBOX, &CIcqProto::GotoInbox);
	CreateProtoService(PS_SETMYAVATAR, &CIcqProto::SetAvatar);

	// events
	HookProtoEvent(ME_CLIST_GROUPCHANGE, &CIcqProto::OnGroupChange);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CIcqProto::OnDbEventRead);
	HookProtoEvent(ME_GC_EVENT, &CIcqProto::GroupchatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CIcqProto::GroupchatMenuHook);
	HookProtoEvent(ME_OPT_INITIALISE, &CIcqProto::OnOptionsInit);

	// netlib handle
	CMStringW descr(FORMAT, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// this was previously an old ICQ account
	ptrW wszUin(GetUIN(0));
	if (wszUin != nullptr) {
		delSetting("UIN");

		m_szOwnId = wszUin;

		for (auto &it : AccContacts())
			delSetting(it, "e-mail");
	}
	// this was previously an old MRA account
	else {
		CMStringW wszEmail(getMStringW("e-mail"));
		if (!wszEmail.IsEmpty()) {
			m_szOwnId = wszEmail;
			delSetting("e-mail");
		}
	}

	InitContactCache();

	m_hWorkerThread = ForkThreadEx(&CIcqProto::ServerThread, nullptr, nullptr);
}

CIcqProto::~CIcqProto()
{
	::CloseHandle(m_evRequestsQueue);
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

void CIcqProto::OnModulesLoaded()
{
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_USERINFO_INITIALISE, &CIcqProto::OnUserInfoInit);
}

void CIcqProto::OnShutdown()
{
	UI_SAFE_CLOSE(m_pdlgEditIgnore);

	m_bTerminated = true;
}

void CIcqProto::OnContactDeleted(MCONTACT hContact)
{
	CMStringW szId(GetUserId(hContact));
	if (!isChatRoom(hContact))
		m_arCache.remove(FindContactByUIN(szId));

	Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/removeBuddy")
		<< AIMSID(this) << WCHAR_PARAM("buddy", szId) << INT_PARAM("allGroups", 1));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	// "Bookmarks..."
	mi.pszService = "/UploadGroups";
	CreateProtoService(mi.pszService, &CIcqProto::UploadGroups);
	mi.name.a = LPGEN("Synchronize server groups");
	mi.position = 200001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUP);
	m_hUploadGroups = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	Menu_ShowItem(m_hUploadGroups, false);
}

INT_PTR CIcqProto::UploadGroups(WPARAM, LPARAM)
{
	for (auto &it : AccContacts()) {
		if (isChatRoom(it))
			continue;

		CMStringW wszIcqGroup(getMStringW(it, "IcqGroup")), wszMirGroup(db_get_wsm(it, "CList", "Group"));
		if (wszMirGroup.IsEmpty())
			wszMirGroup = L"General";
		if (wszIcqGroup != wszMirGroup)
			MoveContactToGroup(it, wszIcqGroup, wszMirGroup);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CIcqProto::GetEmailCount(WPARAM, LPARAM)
{
	if (!m_bOnline)
		return 0;
	return m_unreadEmails;
}

INT_PTR CIcqProto::GotoInbox(WPARAM, LPARAM)
{
	Utils_OpenUrl("https://e.mail.ru/messages/inbox");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::MarkReadTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD)
{
	CIcqProto *ppro = (CIcqProto*)id;

	mir_cslock lck(ppro->csMarkReadQueue);
	while (ppro->arMarkReadQueue.getCount()) {
		IcqCacheItem *pUser = ppro->arMarkReadQueue[0];

		auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER);
		JSONNode request, params; params.set_name("params");
		params << WCHAR_PARAM("sn", ppro->GetUserId(pUser->m_hContact)) << INT64_PARAM("lastRead", ppro->getId(pUser->m_hContact, DB_KEY_LASTMSGID));
		request << CHAR_PARAM("method", "setDlgStateWim") << CHAR_PARAM("reqId", pReq->m_reqId) << params;
		pReq->m_szParam = ptrW(json_write(&request));
		ppro->Push(pReq);

		ppro->arMarkReadQueue.remove(0);
	}
	KillTimer(hwnd, id);
}

int CIcqProto::OnDbEventRead(WPARAM, LPARAM hDbEvent)
{
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = GetContactProto(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	if (m_bOnline) {
		SetTimer(g_hwndHeartbeat, UINT_PTR(this), 200, &CIcqProto::MarkReadTimerProc);
		
		IcqCacheItem *pCache = FindContactByUIN(GetUserId(hContact));
		if (pCache) {
			mir_cslock lck(csMarkReadQueue);
			if (arMarkReadQueue.indexOf(pCache) == -1)
				arMarkReadQueue.insert(pCache);
		}
	}
	return 0;
}

int CIcqProto::OnGroupChange(WPARAM hContact, LPARAM lParam)
{
	if (!m_bOnline)
		return 0;

	CLISTGROUPCHANGE *pParam = (CLISTGROUPCHANGE*)lParam;
	if (hContact == 0) { // whole group is changed
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/") << AIMSID(this);
		if (pParam->pszOldName == nullptr) {
			pReq->m_szUrl += "addGroup";
			pReq << GROUP_PARAM("group", pParam->pszNewName);
		}
		else if (pParam->pszNewName == nullptr) {
			pReq->m_szUrl += "removeGroup";
			pReq << GROUP_PARAM("group", pParam->pszOldName);
		}
		else {
			pReq->m_szUrl += "renameGroup";
			pReq << GROUP_PARAM("oldGroup", pParam->pszOldName) << GROUP_PARAM("newGroup", pParam->pszNewName);
		}
		Push(pReq);
	}
	else MoveContactToGroup(hContact, getMStringW(hContact, "IcqGroup"), pParam->pszNewName);
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_AddToList - adds a contact to the contact list

MCONTACT CIcqProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	if (mir_wstrlen(psr->id.w) == 0)
		return 0;

	MCONTACT hContact = CreateContact(psr->id.w, true);
	if (psr->nick.w)
		setWString(hContact, "Nick", psr->nick.w);
	if (psr->firstName.w)
		setWString(hContact, "FirstName", psr->firstName.w);
	if (psr->lastName.w)
		setWString(hContact, "LastName", psr->lastName.w);

	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int CIcqProto::AuthRequest(MCONTACT hContact, const wchar_t* szMessage)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/buddylist/addBuddy", &CIcqProto::OnAddBuddy);
	pReq << AIMSID(this) << WCHAR_PARAM("authorizationMsg", szMessage) << WCHAR_PARAM("buddy", GetUserId(hContact)) << CHAR_PARAM("group", "General") << INT_PARAM("preAuthorized", 1);
	pReq->hContact = hContact;
	Push(pReq);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

INT_PTR CIcqProto::GetCaps(int type, MCONTACT)
{
	INT_PTR nReturn = 0;

	switch (type) {
	case PFLAGNUM_1:
		nReturn = PF1_IM | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES | /*PF1_SEARCHBYNAME | TODO */
			PF1_VISLIST | PF1_MODEMSG | PF1_FILE | PF1_CONTACT | PF1_SERVERCLIST;
		break;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_INVISIBLE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_INVISIBLE;

	case PFLAGNUM_4:
		nReturn = PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_OFFLINEFILES | PF4_IMSENDOFFLINE | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SERVERMSGID | PF4_READNOTIFY;
		break;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("UIN/e-mail/phone");
	}

	return nReturn;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int CIcqProto::GetInfo(MCONTACT hContact, int)
{
	RetrieveUserInfo(hContact);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by UID

HANDLE CIcqProto::SearchBasic(const wchar_t *pszSearch)
{
	if (!m_bOnline)
		return nullptr;

	auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, &CIcqProto::OnSearchResults);

	JSONNode request, params; params.set_name("params");
	params << WCHAR_PARAM("keyword", pszSearch);
	request << CHAR_PARAM("method", "search") << CHAR_PARAM("reqId", pReq->m_reqId) << params;
	pReq->m_szParam = ptrW(json_write(&request));
	Push(pReq);
	return pReq;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE CIcqProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	// we can't send more than one file at a time
	if (ppszFiles[1] != 0)
		return nullptr;

	struct _stat statbuf;
	if (_wstat(ppszFiles[0], &statbuf)) {
		debugLogW(L"'%s' is an invalid filename", ppszFiles[0]);
		return nullptr;
	}

	int iFileId = _wopen(ppszFiles[0], _O_RDONLY | _O_BINARY, _S_IREAD);
	if (iFileId < 0)
		return nullptr;

	auto *pTransfer = new IcqFileTransfer(hContact, ppszFiles[0]);
	pTransfer->pfts.totalFiles = 1;
	pTransfer->pfts.currentFileSize = pTransfer->pfts.totalBytes = statbuf.st_size;
	pTransfer->m_fileId = iFileId;
	if (mir_wstrlen(szDescription))
		pTransfer->m_wszDescr = szDescription;

	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_GET, "https://files.icq.com/files/init", &CIcqProto::OnFileInit);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("f", "json") << CHAR_PARAM("filename", mir_urlEncode(T2Utf(pTransfer->m_wszShortName))) 
		<< CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("size", statbuf.st_size) << INT_PARAM("ts", TS());
	CalcHash(pReq);
	pReq->pUserInfo = pTransfer;
	Push(pReq);

	return pTransfer; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SendMessage - sends a message

int CIcqProto::SendMsg(MCONTACT hContact, int, const char *pszSrc)
{
	CMStringA szUserid(GetUserId(hContact));
	if (szUserid.IsEmpty())
		return 0;

	int id = InterlockedIncrement(&m_msgId);
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/im/sendIM", &CIcqProto::OnSendMessage);

	auto *pOwn = new IcqOwnMessage(hContact, id, pReq->m_reqId);
	pReq->pUserInfo = pOwn;
	{
		mir_cslock lck(m_csOwnIds);
		m_arOwnIds.insert(pOwn);
	}

	pReq << AIMSID(this) << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", ICQ_APP_ID) << CHAR_PARAM("mentions", "") 
		<< CHAR_PARAM("message", pszSrc) << CHAR_PARAM("offlineIM", "true") << CHAR_PARAM("t", szUserid) << INT_PARAM("ts", TS());
	Push(pReq);
	return id;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetStatus - sets the protocol status

int CIcqProto::SetStatus(int iNewStatus)
{
	debugLogA("CIcqProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %p", iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread);

	if (iNewStatus == m_iStatus)
		return 0;

	m_iDesiredStatus = iNewStatus;
	int iOldStatus = m_iStatus;

	// go offline
	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_bOnline)
			SetServerStatus(ID_STATUS_OFFLINE);

		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE, false);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (!m_bOnline && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);

		if (mir_wstrlen(m_szOwnId) == 0) {
			debugLogA("Thread ended, UIN/password are not configured");
			ConnectionFailed(LOGINERR_BADUSERID);
			return 0;
		}

		if (!getByte(DB_KEY_PHONEREG) && mir_wstrlen(m_szPassword) == 0) {
			debugLogA("Thread ended, password is not configured");
			ConnectionFailed(LOGINERR_BADUSERID);
			return 0;
		}

		CheckPassword();
	}
	else if (m_bOnline) {
		debugLogA("setting server online status to %d", iNewStatus);
		SetServerStatus(iNewStatus);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_GetAwayMsg - returns a contact's away message

HANDLE CIcqProto::GetAwayMsg(MCONTACT)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG - processes received status mode message

int CIcqProto::RecvAwayMsg(MCONTACT, int, PROTORECVEVENT*)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetAwayMsg - sets the away status message

int CIcqProto::SetAwayMsg(int, const wchar_t*)
{
	return 0; // Success
}

/////////////////////////////////////////////////////////////////////////////////////////
// PS_UserIsTyping - sends a UTN notification

int CIcqProto::UserIsTyping(MCONTACT hContact, int type)
{
	Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/im/setTyping")
		<< AIMSID(this) << WCHAR_PARAM("t", GetUserId(hContact)) << CHAR_PARAM("typingStatus", (type == PROTOTYPE_SELFTYPING_ON) ? "typing" : "typed"));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetApparentMode - sets the visibility status

int CIcqProto::SetApparentMode(MCONTACT hContact, int iMode)
{
	int oldMode = getWord(hContact, "ApparentMode");
	if (oldMode != iMode) {
		setWord(hContact, "ApparentMode", iMode);
		SetPermitDeny(GetUserId(hContact), iMode != ID_STATUS_OFFLINE);
	}
	return 0;
}
