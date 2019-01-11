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

CIcqProto::CIcqProto(const char* aProtoName, const wchar_t* aUserName) :
	PROTO<CIcqProto>(aProtoName, aUserName),
	m_arHttpQueue(10),
	m_arOwnIds(1),
	m_arCache(20, NumericKeySortT),
	arMarkReadQueue(10, NumericKeySortT),
	m_evRequestsQueue(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_dwUin(this, DB_KEY_UIN, 0),
	m_szPassword(this, "Password"),
	m_bUseFriendly(this, "UseFriendly", 1),
	m_bHideGroupchats(this, "HideChats", 1)
{
	// services
	CreateProtoService(PS_CREATEACCMGRUI, &CIcqProto::CreateAccMgrUI);
	CreateProtoService(PS_GETAVATARINFO, &CIcqProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CIcqProto::GetAvatar);
	CreateProtoService(PS_GETAVATARCAPS, &CIcqProto::GetAvatarCaps);
	CreateProtoService(PS_SETMYAVATAR, &CIcqProto::SetAvatar);

	// events
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

	m_hWorkerThread = ForkThreadEx(&CIcqProto::ServerThread, nullptr, nullptr);

	InitContactCache();
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
}

void CIcqProto::OnShutdown()
{
	m_bTerminated = true;
}

void CIcqProto::OnContactDeleted(MCONTACT hContact)
{
	DWORD dwUin = getDword(hContact, DB_KEY_UIN);
	m_arCache.remove(FindContactByUIN(dwUin));

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/removeBuddy");
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << INT_PARAM("buddy", dwUin)
		<< CHAR_PARAM("r", pReq->m_reqId) << INT_PARAM("allGroups", 1);
	pReq->flags |= NLHRF_NODUMPSEND;
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::MarkReadTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD)
{
	CIcqProto *ppro = (CIcqProto*)id;

	mir_cslock lck(ppro->csMarkReadQueue);
	while (ppro->arMarkReadQueue.getCount()) {
		IcqCacheItem *pUser = ppro->arMarkReadQueue[0];

		char buf[100];
		itoa(ppro->getDword(pUser->m_hContact, DB_KEY_UIN), buf, 10);

		auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER);
		JSONNode request, params; params.set_name("params");
		params << CHAR_PARAM("sn", buf) << INT64_PARAM("lastRead", ppro->getId(pUser->m_hContact, DB_KEY_LASTMSGID));
		request << CHAR_PARAM("method", "setDlgStateWim") << CHAR_PARAM("reqId", pReq->m_reqId) 
			<< CHAR_PARAM("authToken", ppro->m_szRToken) << INT_PARAM("clientId", ppro->m_iRClientId) << params;
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
		
		IcqCacheItem *pCache = FindContactByUIN(getDword(hContact, DB_KEY_UIN));
		if (pCache) {
			mir_cslock lck(csMarkReadQueue);
			if (arMarkReadQueue.indexOf(pCache) == -1)
				arMarkReadQueue.insert(pCache);
		}
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_AddToList - adds a contact to the contact list

MCONTACT CIcqProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	if (mir_wstrlen(psr->id.w) == 0)
		return 0;

	DWORD dwUin = _wtol(psr->id.w);

	MCONTACT hContact = CreateContact(dwUin, true);
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
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("r", pReq->m_reqId) << WCHAR_PARAM("authorizationMsg", szMessage)
		<< INT_PARAM("buddy", getDword(hContact, DB_KEY_UIN)) << CHAR_PARAM("group", "General") << INT_PARAM("preAuthorized", 1);
	pReq->flags |= NLHRF_NODUMPSEND;
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileAllow - starts a file transfer

HANDLE CIcqProto::FileAllow(MCONTACT, HANDLE, const wchar_t*)
{
	return nullptr; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileCancel - cancels a file transfer

int CIcqProto::FileCancel(MCONTACT, HANDLE)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileDeny - denies a file transfer

int CIcqProto::FileDeny(MCONTACT, HANDLE, const wchar_t*)
{
	return 1; // Invalid contact
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_FileResume - processes file renaming etc

int CIcqProto::FileResume(HANDLE, int*, const wchar_t**)
{
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

INT_PTR CIcqProto::GetCaps(int type, MCONTACT)
{
	INT_PTR nReturn = 0;

	switch (type) {
	case PFLAGNUM_1:
		nReturn = PF1_IM | PF1_URL | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES | /*PF1_SEARCHBYNAME | TODO */
			PF1_VISLIST | PF1_INVISLIST | PF1_MODEMSG | PF1_FILE | PF1_CONTACT | PF1_SERVERCLIST;
		break;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE | PF2_ONTHEPHONE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE;

	case PFLAGNUM_4:
		nReturn = PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_IMSENDOFFLINE | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SERVERMSGID;
		break;

	case PFLAGNUM_5:
		return PF2_FREECHAT | PF2_ONTHEPHONE;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("ICQ number or phone");
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
	request << CHAR_PARAM("method", "search") << CHAR_PARAM("reqId", pReq->m_reqId) << CHAR_PARAM("authToken", m_szRToken)
		<< INT_PARAM("clientId", m_iRClientId) << params;
	pReq->m_szParam = ptrW(json_write(&request));
	Push(pReq);
	return pReq;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE CIcqProto::SendFile(MCONTACT, const wchar_t*, wchar_t**)
{
	return nullptr; // Failure
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
	m_arOwnIds.insert(pOwn);

	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("f", "json") << CHAR_PARAM("k", ICQ_APP_ID)
		<< CHAR_PARAM("mentions", "") << CHAR_PARAM("message", pszSrc) << CHAR_PARAM("offlineIM", "true") << CHAR_PARAM("r", pReq->m_reqId)
		<< CHAR_PARAM("t", szUserid) << INT_PARAM("ts", time(0));
	Push(pReq);
	return id;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int CIcqProto::SendUrl(MCONTACT, int, const char*)
{
	return 1; // Failure
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
		if (m_bOnline) {
			SetServerStatus(ID_STATUS_OFFLINE);
			ShutdownSession();
		}
		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE, false);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (!IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);

		if (m_dwUin == 0) {
			debugLogA("Thread ended, UIN/password are not configured");
			ConnectionFailed(LOGINERR_BADUSERID);
			return 0;
		}

		if (!getByte("PhoneReg") && mir_wstrlen(m_szPassword) == 0) {
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
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/im/setTyping");
	pReq->flags |= NLHRF_NODUMPSEND;
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("f", "json")
		<< INT_PARAM("t", getDword(hContact, DB_KEY_UIN)) << CHAR_PARAM("r", pReq->m_reqId)
		<< CHAR_PARAM("typingStatus", (type == PROTOTYPE_SELFTYPING_ON) ? "typing" : "typed");
	Push(pReq);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetApparentMode - sets the visibility status

int CIcqProto::SetApparentMode(MCONTACT, int)
{
	return 1; // Failure
}
