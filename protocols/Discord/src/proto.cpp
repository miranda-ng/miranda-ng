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

static int compareRequests(const AsyncHttpRequest *p1, const AsyncHttpRequest *p2)
{
	return p1->m_iReqNum - p2->m_iReqNum;
}

static int compareUsers(const CDiscordUser *p1, const CDiscordUser *p2)
{
	return p1->id - p2->id;
}

CDiscordProto::CDiscordProto(const char *proto_name, const wchar_t *username) :
	PROTO<CDiscordProto>(proto_name, username),
	m_arHttpQueue(10, compareRequests),
	m_evRequestsQueue(CreateEvent(NULL, FALSE, FALSE, NULL)),
	m_wszDefaultGroup(this, DB_KEY_GROUP, DB_KEYVAL_GROUP),
	m_wszEmail(this, DB_KEY_EMAIL, L""),
	arUsers(50, compareUsers)
{
	// Services
	CreateProtoService(PS_GETSTATUS, &CDiscordProto::GetStatus);

	// Events
	HookProtoEvent(ME_OPT_INITIALISE, &CDiscordProto::OnOptionsInit);
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CDiscordProto::OnSrmmEvent);

	// database
	db_set_resident(m_szModuleName, "XStatusMsg");

	// Clist
	Clist_GroupCreate(NULL, m_wszDefaultGroup);

	// Fill users list
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		CDiscordUser *pNew = new CDiscordUser(getId(hContact, DB_KEY_ID));
		pNew->hContact = hContact;
		pNew->channelId = getId(hContact, DB_KEY_CHANNELID);
		pNew->lastMessageId = getId(hContact, DB_KEY_LASTMSGID);
		pNew->wszUsername = ptrW(getWStringA(hContact, DB_KEY_NICK));
		pNew->iDiscriminator = getDword(hContact, DB_KEY_DISCR);
		arUsers.insert(pNew);
	}

	// Network initialization
	CMStringW descr;
	NETLIBUSER nlu = { sizeof(nlu) };

	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	descr.Format(TranslateT("%s server connection"), m_tszUserName);
	nlu.ptszDescriptiveName = descr.GetBuffer();
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	CMStringA module(FORMAT, "%s.Gateway", m_szModuleName);
	nlu.szSettingsModule = module.GetBuffer();
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_UNICODE;
	descr.Format(TranslateT("%s gateway connection"), m_tszUserName);
	nlu.ptszDescriptiveName = descr.GetBuffer();
	m_hGatewayNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

CDiscordProto::~CDiscordProto()
{
	debugLogA("CDiscordProto::~CDiscordProto");
	Netlib_CloseHandle(m_hNetlibUser);
	m_hNetlibUser = NULL;

	m_arHttpQueue.destroy();
	::CloseHandle(m_evRequestsQueue);
}

DWORD_PTR CDiscordProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_SERVERCLIST | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_ADDSEARCHRES;
	case PFLAGNUM_2:
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_LONGAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_SUPPORTIDLE | PF4_AVATARS | PF4_IMSENDOFFLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("User ID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)DB_KEY_EMAIL;
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
	else if (m_hWorkerThread == NULL && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		m_hWorkerThread = ForkThreadEx(&CDiscordProto::ServerThread, NULL, NULL);
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

	return NULL;
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
	psr.id.w = (wchar_t*)param;
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)param, (LPARAM)&psr);

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)param, 0);
	mir_free(param);
}

HWND CDiscordProto::SearchAdvanced(HWND hwndDlg)
{
	if (!m_bOnline || !IsWindow(hwndDlg))
		return NULL;

	wchar_t wszNick[200];
	GetDlgItemTextW(hwndDlg, IDC_NICK, wszNick, _countof(wszNick));
	if (wszNick[0] == 0) // empty string? reject
		return NULL;

	wchar_t *p = wcschr(wszNick, '#');
	if (p == NULL) // wrong user id
		return NULL;

	p = mir_wstrdup(wszNick);
	ForkThread(&CDiscordProto::SearchThread, p);
	return (HWND)p;
}

HANDLE CDiscordProto::SearchBasic(const wchar_t *wszId)
{
	if (!m_bOnline)
		return NULL;

	CMStringA szUrl = "/users/";
	szUrl.AppendFormat(ptrA(mir_utf8encodeW(wszId)));
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveUserInfo);
	pReq->pUserInfo = (void*)-1;
	Push(pReq);
	return (HANDLE)1; // Success
}

int CDiscordProto::AuthRequest(MCONTACT hContact, const wchar_t*)
{
	ptrW wszUsername(getWStringA(hContact, DB_KEY_NICK));
	int iDiscriminator(getDword(hContact, DB_KEY_DISCR, -1));
	if (wszUsername == NULL || iDiscriminator == -1)
		return 1; // error

	JSONNode root; root << WCHAR_PARAM("username", wszUsername) << INT_PARAM("discriminator", iDiscriminator);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/users/@me/relationships", &CDiscordProto::OnReceiveAuth, &root);
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
	return 0;
}

MCONTACT CDiscordProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr->id.w == NULL)
		return 0;

	wchar_t *p = wcschr(psr->id.w, '#');
	if (p == NULL)
		return 0;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);
	if (flags & PALF_TEMPORARY)
		db_set_b(hContact, "CList", "NotOnList", 1);
	
	*p = 0;
	CDiscordUser *pUser = new CDiscordUser(0);
	pUser->hContact = hContact;
	pUser->wszUsername = psr->id.w;
	pUser->iDiscriminator = _wtoi(p + 1);
	*p = '#';

	setWString(hContact, DB_KEY_NICK, pUser->wszUsername);
	setDword(hContact, DB_KEY_DISCR, pUser->iDiscriminator);

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
	ProtoBroadcastAck((MCONTACT)param, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate("Protocol is offline or no JID"));
}

int CDiscordProto::SendMsg(MCONTACT hContact, int /*flags*/, const char *pszSrc)
{
	if (!m_bOnline) {
		ForkThread(&CDiscordProto::SendMessageAckThread, (void*)hContact);
		return 1;
	}

	ptrW wszText(mir_utf8decodeW(pszSrc));
	if (wszText == NULL)
		return 0;

	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == NULL || pUser->channelId == NULL)
		return 0;

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", pUser->channelId);
	JSONNode body; body << WCHAR_PARAM("content", wszText);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveMessageAck, &body);
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
	return pReq->m_iReqNum;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnModulesLoaded(WPARAM, LPARAM)
{
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

int CDiscordProto::OnSrmmEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;

	if (MWeventdata->uType == MSG_WINDOW_EVT_OPENING && MWeventdata->hContact) {
		SnowFlake oldid = getId(MWeventdata->hContact, DB_KEY_LASTMSGID);
		if (oldid > 0)
			RetrieveHistory(MWeventdata->hContact, MSG_AFTER, oldid, 99);
		else
			RetrieveHistory(MWeventdata->hContact, MSG_NOFILTER, 0, 99);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event) {
		case EV_PROTO_ONLOAD:    return OnModulesLoaded(wParam, lParam);
		case EV_PROTO_ONEXIT:    return OnPreShutdown(wParam, lParam);
		case EV_PROTO_ONOPTIONS: return OnOptionsInit(wParam, lParam);
	}

	return 1;
}
