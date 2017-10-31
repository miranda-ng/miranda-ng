/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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
	m_InviteDialogs(1),
	m_GCCreateDialogs(1),
	m_OutMessages(3, PtrKeySortT),
	m_bThreadsTerminated(false),
	m_TrouterConnection(nullptr),
	m_pollingConnection(nullptr),
	m_opts(this),
	Contacts(this)
{
	InitNetwork();

	requestQueue = new RequestQueue(m_hNetlibUser);

	CreateProtoService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	CreateProtoService(PS_GETAVATARINFO, &CSkypeProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSkypeProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CSkypeProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CSkypeProto::SvcSetMyAvatar);

	CreateProtoService("/IncomingCallCLE", &CSkypeProto::OnIncomingCallCLE);
	CreateProtoService("/IncomingCallPP", &CSkypeProto::OnIncomingCallPP);

	m_tszAvatarFolder = std::wstring(VARSW(L"%miranda_avatarcache%")) + L"\\" + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(m_tszAvatarFolder.c_str());

	//sounds
	Skin_AddSound("skype_inc_call", L"SkypeWeb", LPGENW("Incoming call sound"));
	Skin_AddSound("skype_call_canceled", L"SkypeWeb", LPGENW("Incoming call canceled sound"));

	SkypeSetTimer();

	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, NULL, NULL);
	m_hTrouterThread = ForkThreadEx(&CSkypeProto::TRouterThread, NULL, NULL);
}

CSkypeProto::~CSkypeProto()
{
	requestQueue->Stop();
	delete requestQueue; requestQueue = nullptr;

	UnInitNetwork();
	UninitPopups();

	if (m_hPollingThread) {
		WaitForSingleObject(m_hPollingThread, INFINITE);
		m_hPollingThread = NULL;
	}

	if (m_hTrouterThread) {
		WaitForSingleObject(m_hTrouterThread, INFINITE);
		m_hTrouterThread = NULL;
	}

	SkypeUnsetTimer();
}

int CSkypeProto::OnExit()
{
	debugLogA(__FUNCTION__);

	requestQueue->Stop();

	m_bThreadsTerminated = true;

	m_hPollingEvent.Set();
	m_hTrouterEvent.Set();

	return 0;
}

DWORD_PTR CSkypeProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH | PF1_MODEMSG | PF1_FILE;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("Skypename");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)SKYPE_SETTINGS_ID;
	}
	return 0;
}

int CSkypeProto::SetAwayMsg(int, const wchar_t *msg)
{
	PushRequest(new SetStatusMsgRequest(msg ? T2Utf(msg) : "", li));
	return 0;
}

HANDLE CSkypeProto::GetAwayMsg(MCONTACT hContact)
{
	PushRequest(new GetProfileRequest(li, Contacts[hContact]), [this, hContact](const NETLIBHTTPREQUEST *response) {
		if (!response || !response->pData)
			return;

		JSONNode root = JSONNode::parse(response->pData);

		if (JSONNode &mood = root["mood"]) {
			CMStringW str = mood.as_mstring();
			this->ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)str.c_str());
		}
		else {
			this->ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		}
	});
	return (HANDLE)1;
}

MCONTACT CSkypeProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	debugLogA(__FUNCTION__);

	if (psr->id.a == NULL)
		return NULL;
	MCONTACT hContact;

	if (psr->flags & PSR_UNICODE)
		hContact = AddContact(T2Utf(psr->id.w));
	else
		hContact = AddContact(psr->id.a);

	return hContact;
}

MCONTACT CSkypeProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);
	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return NULL;
	if ((dbei.pBlob = (PBYTE)alloca(dbei.cbBlob)) == NULL)
		return NULL;
	if (db_event_get(hDbEvent, &dbei))
		return NULL;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return NULL;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return NULL;

	DB_AUTH_BLOB blob(dbei.pBlob);

	MCONTACT hContact = AddContact(blob.get_email());
	return hContact;
}

int CSkypeProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthAcceptRequest(li, Contacts[hContact]));
	return 0;
}

int CSkypeProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthDeclineRequest(li, Contacts[hContact]));
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

	PushRequest(new AddContactRequest(li, Contacts[hContact], T2Utf(szMessage)));
	return 0;
}

int CSkypeProto::GetInfo(MCONTACT hContact, int)
{
	if (isChatRoom(hContact))
		return 1;

	PushRequest(new GetProfileRequest(li, Contacts[hContact]), &CSkypeProto::LoadProfile, (void*)hContact);
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
	case ID_STATUS_FREECHAT:
	case ID_STATUS_ONTHEPHONE:
		iNewStatus = ID_STATUS_ONLINE;
		break;

	case ID_STATUS_NA:
	case ID_STATUS_OUTTOLUNCH:
		iNewStatus = ID_STATUS_AWAY;
		break;

	case ID_STATUS_OCCUPIED:
		iNewStatus = ID_STATUS_DND;
		break;
	}

	//mir_cslock lck(m_StatusLock);

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_iStatus > ID_STATUS_CONNECTING + 1) {
			SendRequest(new DeleteEndpointRequest(li));
		}
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		// logout
		requestQueue->Stop();

		CloseDialogs();
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, ID_STATUS_OFFLINE);

		if (!Miranda_IsTerminated())
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		return 0;
	}
	else {
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		if (old_status == ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE) {
			Login();
		}
		else {
			SendRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus), li), &CSkypeProto::OnStatusChanged);
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

int CSkypeProto::UserIsTyping(MCONTACT hContact, int type)
{
	SendRequest(new SendTypingRequest(Contacts[hContact], type, li));
	return 0;
}

int CSkypeProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	switch (iEventType) {
	case EV_PROTO_ONLOAD:
		return OnAccountLoaded(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_ONMENU:
		return OnInitStatusMenu();

	case EV_PROTO_ONEXIT:
		return OnExit();
	}

	return 1;
}

int CSkypeProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre)
{
	PROTOSEARCHRESULT **isrList = (PROTOSEARCHRESULT**)pre->szMessage;
	DWORD cbBlob = 0;
	BYTE *pBlob;
	BYTE *pCurBlob;
	int i;

	int nCount = *((LPARAM*)pre->lParam);
	char* szMessageId = ((char*)pre->lParam + sizeof(LPARAM));

	//if (GetMessageFromDb(hContact, szMessageId, pre->timestamp)) return 0;

	for (i = 0; i < nCount; i++)
		cbBlob += int(/*mir_wstrlen(isrList[i]->nick.w)*/0 + 2 + mir_wstrlen(isrList[i]->id.w) + mir_strlen(szMessageId));

	pBlob = (PBYTE)mir_calloc(cbBlob);

	for (i = 0, pCurBlob = pBlob; i < nCount; i++) {
		//mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->nick.w));
		pCurBlob += mir_strlen((PCHAR)pCurBlob) + 1;

		mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->id.w));
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	}

	//memcpy(pCurBlob + 1, szMessageId, mir_strlen(szMessageId));

	AddEventToDb(hContact, EVENTTYPE_CONTACTS, pre->timestamp, (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0, cbBlob, pBlob);

	mir_free(pBlob);

	return 0;
}
