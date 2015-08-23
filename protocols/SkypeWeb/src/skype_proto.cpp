/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

CSkypeProto::CSkypeProto(const char* protoName, const TCHAR* userName) :
	PROTO<CSkypeProto>(protoName, userName),
		m_PopupClasses(1), 
		m_InviteDialogs(1),
		m_GCCreateDialogs(1),
		m_OutMessages(3, PtrKeySortT)
{
	m_hProtoIcon = Icons[0].Handle;

	InitNetwork();

	requestQueue = new RequestQueue(m_hNetlibUser);

	CreateProtoService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	CreateProtoService(PS_GETAVATARINFO,  &CSkypeProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS,  &CSkypeProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR,    &CSkypeProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR,    &CSkypeProto::SvcSetMyAvatar);

	CreateProtoService("/IncomingCallCLE", &CSkypeProto::OnIncomingCallCLE);
	CreateProtoService("/IncomingCallPP", &CSkypeProto::OnIncomingCallPP);

	m_tszAvatarFolder = std::tstring(VARST(_T("%miranda_avatarcache%"))) + _T("\\") + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(m_tszAvatarFolder.c_str());

	//sounds
	SkinAddNewSoundEx("skype_inc_call", "SkypeWeb", LPGEN("Incoming call sound"));
	SkinAddNewSoundEx("skype_call_canceled", "SkypeWeb", LPGEN("Incoming call canceled sound"));

	m_hTrouterEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hPollingEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hTrouterHealthEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	SkypeSetTimer();

	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, NULL, NULL);
	m_hTrouterThread = ForkThreadEx(&CSkypeProto::TRouterThread, NULL, NULL);
}

CSkypeProto::~CSkypeProto()
{
	requestQueue->Stop();
	delete requestQueue;
	
	UnInitNetwork();
	UninitPopups();

	if (m_hPollingThread)
	{
		TerminateThread(m_hPollingThread, NULL);
		m_hPollingThread = NULL;
	}
	CloseHandle(m_hPollingEvent); m_hPollingEvent = NULL;

	if (m_hTrouterThread)
	{
		TerminateThread(m_hTrouterThread, NULL);
		m_hTrouterThread = NULL;
	}
	CloseHandle(m_hTrouterEvent); m_hTrouterEvent = NULL;
	
	CloseHandle(m_hTrouterHealthEvent);

	SkypeUnsetTimer();
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	debugLogA(__FUNCTION__);

	requestQueue->Stop();

	m_bThreadsTerminated = true;

	ShutdownConnections();

	SetEvent(m_hPollingEvent);
	
	return 0;
}

DWORD_PTR CSkypeProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH | PF1_MODEMSG;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Skypename";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)SKYPE_SETTINGS_ID;
	}
	return 0;
}

MCONTACT CSkypeProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	debugLogA(__FUNCTION__);

	if (psr->id.a == NULL)
		return NULL;
	MCONTACT hContact;
	
	if (psr->flags & PSR_UNICODE)
		hContact = AddContact(mir_utf8encodeT(psr->id.t));
	else 
		hContact = AddContact(psr->id.a);
		
	return hContact;
}

MCONTACT CSkypeProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);
	DBEVENTINFO dbei = { sizeof(dbei) };
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
	
	MCONTACT hContact = AddContact(ptrA(blob.get_id()));
	return hContact;
}

int CSkypeProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA token(getStringA("TokenSecret"));
	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AuthAcceptRequest(li, skypename));
	return 0;
}

int CSkypeProto::AuthDeny(MEVENT hDbEvent, const TCHAR*)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA token(getStringA("TokenSecret"));
	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AuthDeclineRequest(li, skypename));
	return 0;
}

int CSkypeProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int CSkypeProto::AuthRequest(MCONTACT hContact, const TCHAR *szMessage)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA token(getStringA("TokenSecret"));
	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AddContactRequest(li, skypename, T2Utf(szMessage)));
	return 0;
}

int CSkypeProto::GetInfo(MCONTACT hContact, int)
{
	if (!isChatRoom(hContact))
		PushRequest(
			new GetProfileRequest(li, ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID))),
			&CSkypeProto::LoadProfile);
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

	switch (iNewStatus)
	{
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

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		if (m_iStatus > ID_STATUS_CONNECTING + 1)
		{
			SendRequest(new DeleteEndpointRequest(li));
			delSetting("registrationRoken");
			delSetting("endpointId");
			delSetting("expires");
		}
		// logout
		requestQueue->Stop();
		ShutdownConnections();
		
		CloseDialogs();
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

		if (!Miranda_Terminated())
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		return 0;
	}
	else
	{
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		if (old_status == ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE)
		{
			Login();
		}
		else
		{
			SendRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus), li), &CSkypeProto::OnStatusChanged);
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

int CSkypeProto::UserIsTyping(MCONTACT hContact, int type)
{
	SendRequest(new SendTypingRequest(ptrA(getStringA(hContact, SKYPE_SETTINGS_ID)), type, li));
	return 0;
}

int CSkypeProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	switch (iEventType)
	{
	case EV_PROTO_ONLOAD:
		return OnAccountLoaded(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_ONMENU:
		return OnInitStatusMenu();

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam, lParam);
	}

	return 1;
}