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

#include "common.h"

CSkypeProto::CSkypeProto(const char* protoName, const TCHAR* userName) :
PROTO<CSkypeProto>(protoName, userName), password(NULL)
{
	m_hProtoIcon = Icons[0].Handle;
	SetAllContactsStatus(ID_STATUS_OFFLINE);

	wchar_t name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);
	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	requestQueue = new RequestQueue(m_hNetlibUser);

	CreateProtoService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	
	CreateProtoService(PS_GETAVATARINFOT, &CSkypeProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSkypeProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATART, &CSkypeProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATART, &CSkypeProto::SvcSetMyAvatar);

	m_tszAvatarFolder = std::tstring(VARST(_T("%miranda_avatarcache%"))) + _T("\\") + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(m_tszAvatarFolder.c_str());

	// custom event
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;
	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming Call");
	dbEventType.eventIcon = GetIconHandle("inc_call");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	SkinAddNewSoundEx("skype_inc_call", "SkypeWeb", LPGEN("Incoming call sound"));
}

CSkypeProto::~CSkypeProto()
{
	delete requestQueue;
	Netlib_CloseHandle(m_hNetlibUser);
	m_hNetlibUser = NULL;
}

DWORD_PTR CSkypeProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_IMSENDUTF;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Skypename";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)SKYPE_SETTINGS_ID;
	}

	return 0;
}

MCONTACT CSkypeProto::AddToList(int, PROTOSEARCHRESULT *psr) 
{ 
	debugLogA("CSkypeProto::AddToList");


	ptrA skypeName(mir_t2a(ptrT(psr->id)));
	if (skypeName == NULL)
		return NULL;

	MCONTACT hContact = AddContact(skypeName);
	return hContact;
}

MCONTACT CSkypeProto::AddToListByEvent(int, int, MEVENT) { return 0; }

int CSkypeProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;
	
	ptrA token(getStringA("TokenSecret"));
	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AuthAcceptRequest(token, skypename));
	return 0;
}

int CSkypeProto::AuthDeny(MEVENT hDbEvent, const PROTOCHAR*)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA token(getStringA("TokenSecret"));
	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AuthDeclineRequest(token, skypename));
	return 0;
}

int CSkypeProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int CSkypeProto::AuthRequest(MCONTACT hContact, const PROTOCHAR *szMessage)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA token(getStringA("TokenSecret"));
	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AddContactRequest(token, skypename, ptrA(mir_t2a(szMessage))));
	return 0; 
}

int CSkypeProto::GetInfo(MCONTACT hContact, int) 
{
	PushRequest(
		new GetProfileRequest(ptrA(getStringA("TokenSecret")), ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID))),
		&CSkypeProto::LoadProfile);
	return 0;
}

int CSkypeProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return SaveMessageToDb(hContact, pre);
}

int CSkypeProto::SendMsg(MCONTACT hContact, int flags, const char *msg) 
{
	return OnSendMessage(hContact, flags, msg);
}

int CSkypeProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		// logout
		isTerminated = true;
		if (m_pollingConnection)
			CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_pollingConnection, 0);
		if (m_TrouterConnection)
			CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_TrouterConnection, 0);

		if (m_iStatus > ID_STATUS_CONNECTING + 1)
		{
			SendRequest(new DeleteEndpointRequest(RegToken, EndpointId, Server));
			delSetting("registrationRoken");
			delSetting("endpointId");
			delSetting("expires");

		}
		requestQueue->Stop();

		if (!Miranda_Terminated())
			SetAllContactsStatus(ID_STATUS_OFFLINE);

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	}
	else
	{
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		if (old_status == ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE)
		{
			// login
			m_iStatus = ID_STATUS_CONNECTING;
			requestQueue->Start();
			int tokenExpires(getDword("TokenExpiresIn", 0));
			if ((tokenExpires - 1800) > time(NULL))
				OnLoginSuccess();
			else 
				SendRequest(new LoginRequest(), &CSkypeProto::OnLoginFirst);
		}
		else
		{
			SendRequest(new SetStatusRequest(RegToken, MirandaToSkypeStatus(m_iDesiredStatus)), &CSkypeProto::OnStatusChanged);
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

int CSkypeProto::UserIsTyping(MCONTACT hContact, int type)
{
	SendRequest(new SendTypingRequest(RegToken, ptrA(getStringA(hContact, SKYPE_SETTINGS_ID)), type, Server));
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

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	debugLogA(__FUNCTION__);
		
	isTerminated = true;
	if (m_pollingConnection)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_pollingConnection, 0);
	if (m_TrouterConnection)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_TrouterConnection, 0);

	return 0;
}