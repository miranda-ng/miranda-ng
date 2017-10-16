/*
Copyright © 2016-17 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static int compareMessages(const SnowFlake *p1, const SnowFlake *p2)
{
	return compareInt64(*p1, *p2);
}

static int compareRequests(const AsyncHttpRequest *p1, const AsyncHttpRequest *p2)
{
	return p1->m_iReqNum - p2->m_iReqNum;
}

int compareUsers(const CDiscordUser *p1, const CDiscordUser *p2)
{
	return compareInt64(p1->id, p2->id);
}

static int compareGuilds(const CDiscordGuild *p1, const CDiscordGuild *p2)
{
	return compareInt64(p1->id, p2->id);
}

CDiscordProto::CDiscordProto(const char *proto_name, const wchar_t *username) :
	PROTO<CDiscordProto>(proto_name, username),
	m_arHttpQueue(10, compareRequests),
	m_evRequestsQueue(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_wszDefaultGroup(this, DB_KEY_GROUP, DB_KEYVAL_GROUP),
	m_wszEmail(this, DB_KEY_EMAIL, L""),
	m_bHideGroupchats(this, DB_KEY_HIDECHATS, true),
	arGuilds(1, compareGuilds),
	arMarkReadQueue(1, compareUsers),
	arOwnMessages(1, compareMessages),
	arUsers(10, compareUsers)
{
	// Services
	CreateProtoService(PS_GETSTATUS, &CDiscordProto::GetStatus);
	CreateProtoService(PS_CREATEACCMGRUI, &CDiscordProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &CDiscordProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CDiscordProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CDiscordProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CDiscordProto::SetMyAvatar);

	// Events
	HookProtoEvent(ME_OPT_INITIALISE, &CDiscordProto::OnOptionsInit);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CDiscordProto::OnDbEventRead);

	// database
	db_set_resident(m_szModuleName, "XStatusMsg");

	// Clist
	Clist_GroupCreate(0, m_wszDefaultGroup);

	// Fill users list
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		CDiscordUser *pNew = new CDiscordUser(getId(hContact, DB_KEY_ID));
		pNew->hContact = hContact;
		pNew->channelId = getId(hContact, DB_KEY_CHANNELID);
		pNew->lastMsg.id = getId(hContact, DB_KEY_LASTMSGID);
		pNew->wszUsername = ptrW(getWStringA(hContact, DB_KEY_NICK));
		pNew->iDiscriminator = getDword(hContact, DB_KEY_DISCR);
		arUsers.insert(pNew);
	}

	// Network initialization
	CMStringW descr;
	NETLIBUSER nlu = {};

	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	descr.Format(TranslateT("%s server connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CMStringA module(FORMAT, "%s.Gateway", m_szModuleName);
	nlu.szSettingsModule = module.GetBuffer();
	nlu.flags = NUF_OUTGOING | NUF_UNICODE;
	descr.Format(TranslateT("%s gateway connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hGatewayNetlibUser = Netlib_RegisterUser(&nlu);
}

CDiscordProto::~CDiscordProto()
{
	debugLogA("CDiscordProto::~CDiscordProto");
	Netlib_CloseHandle(m_hNetlibUser);
	m_hNetlibUser = nullptr;

	m_arHttpQueue.destroy();
	::CloseHandle(m_evRequestsQueue);
}

DWORD_PTR CDiscordProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_SERVERCLIST | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_ADDSEARCHRES | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_LONGAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_SUPPORTIDLE | PF4_AVATARS | PF4_IMSENDOFFLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("User ID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)DB_KEY_ID;
	}
	return 0;
}

INT_PTR CDiscordProto::GetStatus(WPARAM, LPARAM)
{
	return m_iStatus;
}

int CDiscordProto::SetStatus(int iNewStatus)
{
	debugLogA("CDiscordProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %p", iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread);

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
		SetAllContactStatuses(ID_STATUS_OFFLINE);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (m_hWorkerThread == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		m_hWorkerThread = ForkThreadEx(&CDiscordProto::ServerThread, nullptr, nullptr);
	}
	else if (m_bOnline) {
		debugLogA("setting server online status to %d", iNewStatus);
		SetServerStatus(iNewStatus);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK AdvancedSearchDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetFocus(GetDlgItem(hwndDlg, IDC_NICK));
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_SETFOCUS)
			PostMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), (LPARAM)hwndDlg);
	}
	return FALSE;
}

HWND CDiscordProto::CreateExtendedSearchUI(HWND hwndParent)
{
	if (hwndParent)
		return CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_EXTSEARCH), hwndParent, AdvancedSearchDlgProc, 0);

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::SearchThread(void *param)
{
	Sleep(100);

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.nick.w = (wchar_t*)param;
	psr.firstName.w = L"";
	psr.lastName.w = L"";
	psr.id.w = L"";
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)param, (LPARAM)&psr);

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)param, 0);
	mir_free(param);
}

HWND CDiscordProto::SearchAdvanced(HWND hwndDlg)
{
	if (!m_bOnline || !IsWindow(hwndDlg))
		return nullptr;

	wchar_t wszNick[200];
	GetDlgItemTextW(hwndDlg, IDC_NICK, wszNick, _countof(wszNick));
	if (wszNick[0] == 0) // empty string? reject
		return nullptr;

	wchar_t *p = wcschr(wszNick, '#');
	if (p == nullptr) // wrong user id
		return nullptr;

	p = mir_wstrdup(wszNick);
	ForkThread(&CDiscordProto::SearchThread, p);
	return (HWND)p;
}

HANDLE CDiscordProto::SearchBasic(const wchar_t *wszId)
{
	if (!m_bOnline)
		return nullptr;

	CMStringA szUrl = "/users/";
	szUrl.AppendFormat(ptrA(mir_utf8encodeW(wszId)));
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveMyInfo);
	pReq->pUserInfo = (void*)-1;
	Push(pReq);
	return (HANDLE)1; // Success
}

////////////////////////////////////////////////////////////////////////////////////////
// Authorization

int CDiscordProto::AuthRequest(MCONTACT hContact, const wchar_t*)
{
	ptrW wszUsername(getWStringA(hContact, DB_KEY_NICK));
	int iDiscriminator(getDword(hContact, DB_KEY_DISCR, -1));
	if (wszUsername == nullptr || iDiscriminator == -1)
		return 1; // error

	JSONNode root; root << WCHAR_PARAM("username", wszUsername) << INT_PARAM("discriminator", iDiscriminator);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/users/@me/relationships", nullptr, &root);
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
	return 0;
}

int CDiscordProto::AuthRecv(MCONTACT, PROTORECVEVENT *pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int CDiscordProto::Authorize(MEVENT hDbEvent)
{
	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1)) return 1;
	if ((dbei.pBlob = (PBYTE)alloca(dbei.cbBlob)) == nullptr) return 1;
	if (db_event_get(hDbEvent, &dbei)) return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return 1;
	if (mir_strcmp(dbei.szModule, m_szModuleName)) return 1;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	CMStringA szUrl(FORMAT, "/users/@me/relationships/%lld", getId(hContact, DB_KEY_ID));
	Push(new AsyncHttpRequest(this, REQUEST_PUT, szUrl, nullptr));
	return 0;
}

int CDiscordProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1)) return 1;
	if ((dbei.pBlob = (PBYTE)alloca(dbei.cbBlob)) == nullptr) return 1;
	if (db_event_get(hDbEvent, &dbei)) return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return 1;
	if (mir_strcmp(dbei.szModule, m_szModuleName)) return 1;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	RemoveFriend(getId(hContact, DB_KEY_ID));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CDiscordProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (mir_wstrlen(psr->nick.w) == 0)
		return 0;

	wchar_t *p = wcschr(psr->nick.w, '#');
	if (p == nullptr)
		return 0;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);
	if (flags & PALF_TEMPORARY)
		db_set_b(hContact, "CList", "NotOnList", 1);
	
	*p = 0;
	CDiscordUser *pUser = new CDiscordUser(0);
	pUser->hContact = hContact;
	pUser->wszUsername = psr->nick.w;
	pUser->iDiscriminator = _wtoi(p + 1);
	*p = '#';

	if (mir_wstrlen(psr->id.w)) {
		pUser->id = _wtoi64(psr->id.w);
		setId(hContact, DB_KEY_ID, pUser->id);
	}

	db_set_ws(hContact, "CList", "Group", m_wszDefaultGroup);
	setWString(hContact, DB_KEY_NICK, pUser->wszUsername);
	setDword(hContact, DB_KEY_DISCR, pUser->iDiscriminator);
	arUsers.insert(pUser);

	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

int CDiscordProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *evt)
{
	T2Utf szResUtf((const wchar_t*)evt->lParam);
	evt->pCustomData = (char*)szResUtf;
	evt->cbCustomDataSize = (DWORD)mir_strlen(szResUtf);
	Proto_RecvMessage(hContact, evt);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendMsg

void __cdecl CDiscordProto::SendMessageAckThread(void *param)
{
	Sleep(100);
	ProtoBroadcastAck((UINT_PTR)param, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate("Protocol is offline or user isn't authorized yet"));
}

int CDiscordProto::SendMsg(MCONTACT hContact, int /*flags*/, const char *pszSrc)
{
	if (!m_bOnline) {
		ForkThread(&CDiscordProto::SendMessageAckThread, (void*)hContact);
		return 1;
	}

	ptrW wszText(mir_utf8decodeW(pszSrc));
	if (wszText == nullptr)
		return 0;

	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == nullptr || pUser->id == 0)
		return 0;

	// no channel - we need to create one
	if (pUser->channelId == 0) {
		JSONNode list(JSON_ARRAY); list.set_name("recipients"); list << INT64_PARAM("", pUser->id);
		JSONNode body; body << list;
		CMStringA szUrl(FORMAT, "/users/%lld/channels", m_ownId);

		// theoretically we get the same data from the gateway thread, but there could be a delay
		// so we bind data analysis to the http packet reply
		mir_cslock lck(m_csHttpQueue);
		ExecuteRequest(new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveCreateChannel, &body));
		if (pUser->channelId == 0)
			return 0;
	}

	// we generate a random 64-bit integer and pass it to the server
	// to distinguish our own messages from these generated by another clients
	SnowFlake nonce; Utils_GetRandom(&nonce, sizeof(nonce));
	JSONNode body; body << WCHAR_PARAM("content", wszText) << INT64_PARAM("nonce", nonce);
	arOwnMessages.insert(new SnowFlake(nonce));

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", pUser->channelId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveMessage, &body);
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
	return pReq->m_iReqNum;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (type == PROTOTYPE_SELFTYPING_ON) {
		CMStringA szUrl(FORMAT, "/channels/%lld/typing", getId(hContact, DB_KEY_CHANNELID));
		Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, nullptr));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::MarkReadTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD)
{
	CDiscordProto *ppro = (CDiscordProto*)(id - 1);

	mir_cslock lck(ppro->csMarkReadQueue);
	while (ppro->arMarkReadQueue.getCount()) {
		CDiscordUser *pUser = ppro->arMarkReadQueue[0];
		CMStringA szUrl(FORMAT, "/channels/%lld/messages/%lld/ack", pUser->channelId, pUser->lastMsg.id);
		ppro->Push(new AsyncHttpRequest(ppro, REQUEST_POST, szUrl, nullptr));
		ppro->arMarkReadQueue.remove(0);
	}
	KillTimer(hwnd, id);
}

int CDiscordProto::OnDbEventRead(WPARAM, LPARAM hDbEvent)
{
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = GetContactProto(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	if (m_bOnline) {
		SetTimer(g_hwndHeartbeat, UINT_PTR(this) + 1, 200, &CDiscordProto::MarkReadTimerProc);

		CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
		if (pUser != nullptr) {
			mir_cslock lck(csMarkReadQueue);
			if (arMarkReadQueue.indexOf(pUser) == -1)
				arMarkReadQueue.insert(pUser);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnDeleteContact(MCONTACT hContact)
{
	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == nullptr || !m_bOnline)
		return 0;

	if (pUser->channelId)
		Push(new AsyncHttpRequest(this, REQUEST_DELETE, CMStringA(FORMAT, "/channels/%lld", pUser->channelId), nullptr));

	if (pUser->id)
		RemoveFriend(pUser->id);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct SendFileThreadParam
{
	MCONTACT hContact;
	CMStringW wszDescr, wszFileName;

	SendFileThreadParam(MCONTACT _p1, LPCWSTR _p2, LPCWSTR _p3) :
		hContact(_p1),
		wszFileName(_p2),
		wszDescr(_p3)
	{}
};

void CDiscordProto::SendFileThread(void *param)
{
	SendFileThreadParam *p = (SendFileThreadParam*)param;

	FILE *in = _wfopen(p->wszFileName, L"rb");
	if (in == nullptr) {
		debugLogA("cannot open file %S for reading", p->wszFileName.c_str());
	LBL_Error:
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, param);
		delete p;
		return;
	}

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, param);

	char szRandom[16], szRandomText[33];
	Utils_GetRandom(szRandom, _countof(szRandom));
	bin2hex(szRandom, _countof(szRandom), szRandomText);
	CMStringA szBoundary(FORMAT, "----Boundary%s", szRandomText);

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", getId(p->hContact, DB_KEY_CHANNELID));
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveFile);
	pReq->AddHeader("Content-Type", CMStringA("multipart/form-data; boundary=" + szBoundary));
	pReq->AddHeader("Accept", "*/*");

	szBoundary.Insert(0, "--");

	CMStringA szBody;
	szBody.Append(szBoundary + "\r\n");
	szBody.Append("Content-Disposition: form-data; name=\"content\"\r\n\r\n");
	szBody.Append(ptrA(mir_utf8encodeW(p->wszDescr)));
	szBody.Append("\r\n");

	szBody.Append(szBoundary + "\r\n");
	szBody.Append("Content-Disposition: form-data; name=\"tts\"\r\n\r\nfalse\r\n");

	wchar_t *pFileName = wcsrchr(p->wszFileName.GetBuffer(), '\\');
	if (pFileName != nullptr)
		pFileName++;
	else
		pFileName = p->wszFileName.GetBuffer();

	szBody.Append(szBoundary + "\r\n");
	szBody.AppendFormat("Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n", ptrA(mir_utf8encodeW(pFileName)));
	szBody.AppendFormat("Content-Type: %S\r\n", ProtoGetAvatarMimeType(ProtoGetAvatarFileFormat(p->wszFileName)));
	szBody.Append("\r\n");

	size_t cbBytes = filelength(fileno(in));

	szBoundary.Insert(0, "\r\n");
	szBoundary.Append("--\r\n");
	pReq->dataLength = int(szBody.GetLength() + szBoundary.GetLength() + cbBytes);
	pReq->pData = (char*)mir_alloc(pReq->dataLength+1);
	memcpy(pReq->pData, szBody.c_str(), szBody.GetLength());
	size_t cbRead = fread(pReq->pData + szBody.GetLength(), 1, cbBytes, in);
	fclose(in);
	if (cbBytes != cbRead) {
		debugLogA("cannot read file %S: %d bytes read instead of %d", p->wszFileName.c_str(), cbRead, cbBytes);
		delete pReq;
		goto LBL_Error;
	}
	
	memcpy(pReq->pData + szBody.GetLength() + cbBytes, szBoundary, szBoundary.GetLength());
	pReq->pUserInfo = p;
	Push(pReq);

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, param);
}

void CDiscordProto::OnReceiveFile(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	SendFileThreadParam *p = (SendFileThreadParam*)pReq->pUserInfo;
	if (pReply->resultCode != 200) {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, p);
		debugLogA("CDiscordProto::SendFile failed: %d", pReply->resultCode);
	}
	else {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, p);
		debugLogA("CDiscordProto::SendFile succeeded");
	}

	delete p;
}

HANDLE CDiscordProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	SnowFlake id = getId(hContact, DB_KEY_CHANNELID);
	if (id == 0)
		return nullptr;

	// we don't wanna block the main thread, right?
	SendFileThreadParam *param = new SendFileThreadParam(hContact, ppszFiles[0], szDescription);
	ForkThread(&CDiscordProto::SendFileThread, param);
	return param;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnModulesLoaded(WPARAM, LPARAM)
{
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &CDiscordProto::GroupchatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CDiscordProto::GroupchatMenuHook);

	InitMenus();
	return 0;
}

int CDiscordProto::OnPreShutdown(WPARAM, LPARAM)
{
	debugLogA("CDiscordProto::OnPreShutdown");

	m_bTerminated = true;
	SetEvent(m_evRequestsQueue);

	if (m_hGatewayConnection)
		Netlib_Shutdown(m_hGatewayConnection);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event) {
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam, lParam);

	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnDeleteContact((MCONTACT)wParam);
	}

	return 1;
}
