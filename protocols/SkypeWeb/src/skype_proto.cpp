/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CSkypeProto::CSkypeProto(const char* protoName, const wchar_t* userName) :
	PROTO<CSkypeProto>(protoName, userName),
	m_PopupClasses(1),
	m_OutMessages(3, PtrKeySortT),
	m_bThreadsTerminated(false),
	m_impl(*this),
	m_requests(1),
	bAutoHistorySync(this, "AutoSync", true),
	bMarkAllAsUnread(this, "MarkMesUnread", true),
	bUseHostnameAsPlace(this, "UseHostName", true),
	bUseBBCodes(this, "UseBBCodes", true),
	bUseServerTime(this, "UseServerTime", false),
	wstrCListGroup(this, SKYPE_SETTINGS_GROUP, L"Skype"),
	wstrPlace(this, "Place", L"")
{
	NETLIBUSER nlu = {};
	CMStringW name(FORMAT, TranslateT("%s connection"), m_tszUserName);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = name.GetBuffer();
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateProtoService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	CreateProtoService(PS_GETAVATARINFO, &CSkypeProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSkypeProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CSkypeProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CSkypeProto::SvcSetMyAvatar);

	CreateProtoService(PS_MENU_REQAUTH, &CSkypeProto::OnRequestAuth);
	CreateProtoService(PS_MENU_GRANTAUTH, &CSkypeProto::OnGrantAuth);
	CreateProtoService(PS_MENU_LOADHISTORY, &CSkypeProto::GetContactHistory);

	HookProtoEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CSkypeProto::OnDbEventRead);

	m_tszAvatarFolder = std::wstring(VARSW(L"%miranda_avatarcache%")) + L"\\" + m_tszUserName;
	CreateDirectoryTreeW(m_tszAvatarFolder.c_str());

	//sounds
	g_plugin.addSound("skype_inc_call", L"SkypeWeb", LPGENW("Incoming call"));
	g_plugin.addSound("skype_call_canceled", L"SkypeWeb", LPGENW("Incoming call canceled"));

	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, NULL, NULL);

	m_szSkypename = getMStringA(SKYPE_SETTINGS_ID);
	if (m_szSkypename.IsEmpty()) {
		m_szSkypename = getMStringA(SKYPE_SETTINGS_LOGIN);
		if (!m_szSkypename.IsEmpty()) { // old settings format, need to update all settings
			m_szSkypename.Insert(0, "8:");
			setString(SKYPE_SETTINGS_ID, m_szSkypename);

			for (auto &hContact : AccContacts()) {
				CMStringA id(ptrA(getUStringA(hContact, "Skypename")));
				if (!id.IsEmpty())
					setString(hContact, SKYPE_SETTINGS_ID, (isChatRoom(hContact)) ? "19:"+id : "8:"+id);

				ptrW wszNick(getWStringA(hContact, "Nick"));
				if (wszNick == nullptr)
					setUString(hContact, "Nick", id);

				delSetting(hContact, "Skypename");
			}
		}
	}

	InitGroupChatModule();
}

CSkypeProto::~CSkypeProto()
{
	StopQueue();
	if (m_hRequestQueueThread) {
		WaitForSingleObject(m_hRequestQueueThread, INFINITE);
		m_hRequestQueueThread = nullptr;
	}

	UninitPopups();

	if (m_hPollingThread) {
		WaitForSingleObject(m_hPollingThread, INFINITE);
		m_hPollingThread = nullptr;
	}
}

void CSkypeProto::OnModulesLoaded()
{
	setAllContactStatuses(ID_STATUS_OFFLINE, false);

	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CSkypeProto::OnPreCreateMessage);

	InitDBEvents();
	InitPopups();
}

void CSkypeProto::OnShutdown()
{
	debugLogA(__FUNCTION__);

	StopQueue();

	m_bThreadsTerminated = true;

	m_hPollingEvent.Set();
	m_hTrouterEvent.Set();
}

INT_PTR CSkypeProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH | PF1_MODEMSG | PF1_FILE;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_4:
		return PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SERVERMSGID;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Skypename");
	}
	return 0;
}

int CSkypeProto::SetAwayMsg(int, const wchar_t *msg)
{
	if (IsOnline())
		PushRequest(new SetStatusMsgRequest(msg ? T2Utf(msg) : ""));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSkypeProto::OnReceiveAwayMsg(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	MCONTACT hContact = DWORD_PTR(pRequest->pUserInfo);
	auto &root = reply.data();
	if (JSONNode &mood = root["mood"]) {
		CMStringW str = mood.as_mstring();
		ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)str.c_str());
	}
	else {
		ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	}
}

HANDLE CSkypeProto::GetAwayMsg(MCONTACT hContact)
{
	auto *pReq = new GetProfileRequest(this, hContact);
	pReq->m_pFunc = &CSkypeProto::OnReceiveAwayMsg;
	return (HANDLE)1;
}

MCONTACT CSkypeProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	debugLogA(__FUNCTION__);

	if (psr->id.a == nullptr)
		return NULL;

	MCONTACT hContact;
	if (psr->flags & PSR_UNICODE)
		hContact = AddContact(T2Utf(psr->id.w), T2Utf(psr->nick.w));
	else
		hContact = AddContact(psr->id.a, psr->nick.a);

	return hContact;
}

MCONTACT CSkypeProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);
	
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei))
		return NULL;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return NULL;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return NULL;

	DB::AUTH_BLOB blob(dbei.pBlob);
	return AddContact(blob.get_email(), blob.get_nick());
}

int CSkypeProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthAcceptRequest(getId(hContact)));
	return 0;
}

int CSkypeProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthDeclineRequest(getId(hContact)));
	return 0;
}

int CSkypeProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int CSkypeProto::AuthRequest(MCONTACT hContact, const wchar_t *szMessage)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AddContactRequest(getId(hContact), T2Utf(szMessage)));
	return 0;
}

int CSkypeProto::GetInfo(MCONTACT hContact, int)
{
	if (isChatRoom(hContact))
		return 1;

	PushRequest(new GetProfileRequest(this, hContact));
	return 0;
}

int CSkypeProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	return OnSendMessage(hContact, flags, msg);
}

int CSkypeProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	switch (iNewStatus) {
	case ID_STATUS_FREECHAT: iNewStatus = ID_STATUS_ONLINE; break;
	case ID_STATUS_NA:       iNewStatus = ID_STATUS_AWAY;   break;
	case ID_STATUS_OCCUPIED: iNewStatus = ID_STATUS_DND;    break;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_iStatus > ID_STATUS_CONNECTING + 1 && m_szId)
			PushRequest(new DeleteEndpointRequest(this));

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		// logout
		StopQueue();

		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, ID_STATUS_OFFLINE);

		m_impl.m_heartBeat.StopSafe();

		if (!Miranda_IsTerminated())
			setAllContactStatuses(ID_STATUS_OFFLINE, false);
		return 0;
	}
	else {
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		if (old_status == ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE)
			Login();
		else
			PushRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus)));
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

int CSkypeProto::UserIsTyping(MCONTACT hContact, int type)
{
	PushRequest(new SendTypingRequest(getId(hContact), type));
	return 0;
}

int CSkypeProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre)
{
	PROTOSEARCHRESULT **isrList = (PROTOSEARCHRESULT**)pre->szMessage;

	int nCount = *((LPARAM*)pre->lParam);
	char* szMessageId = ((char*)pre->lParam + sizeof(LPARAM));

	//if (GetMessageFromDb(hContact, szMessageId, pre->timestamp)) return 0;

	uint32_t cbBlob = 0;
	for (int i = 0; i < nCount; i++)
		cbBlob += int(/*mir_wstrlen(isrList[i]->nick.w)*/0 + 2 + mir_wstrlen(isrList[i]->id.w) + mir_strlen(szMessageId));

	uint8_t *pBlob = (uint8_t*)mir_calloc(cbBlob);
	uint8_t *pCurBlob = pBlob;

	for (int i = 0; i < nCount; i++) {
		//mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->nick.w));
		pCurBlob += mir_strlen((PCHAR)pCurBlob) + 1;

		mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->id.w));
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	}

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_CONTACTS;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	db_event_add(hContact, &dbei);

	mir_free(pBlob);
	return 0;
}
