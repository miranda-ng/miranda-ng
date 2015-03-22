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
		return PF1_AUTHREQ;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_NOAUTHDENYREASON;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Skypename";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)SKYPE_SETTINGS_ID;
	}

	return 0;
}

MCONTACT CSkypeProto::AddToList(int flags, PROTOSEARCHRESULT *psr) { return 0; }

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

int CSkypeProto::AuthRequest(MCONTACT hContact, const PROTOCHAR *szMessage) { return 0; }

HANDLE CSkypeProto::ChangeInfo(int, void*) { return 0; }

HANDLE CSkypeProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszPath) { return 0; }

int CSkypeProto::FileCancel(MCONTACT hContact, HANDLE hTransfer) { return 0; }

int CSkypeProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszReason) { return 0; }

int CSkypeProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** tszFilename) { return 0; }

int CSkypeProto::GetInfo(MCONTACT, int) { return 0; }

HANDLE CSkypeProto::SearchBasic(const PROTOCHAR*) { return 0; }

HANDLE CSkypeProto::SearchByEmail(const PROTOCHAR*) { return 0; }

HANDLE CSkypeProto::SearchByName(const PROTOCHAR*, const PROTOCHAR*, const PROTOCHAR*) { return 0; }

HWND CSkypeProto::SearchAdvanced(HWND owner) { return 0; }

HWND CSkypeProto::CreateExtendedSearchUI(HWND owner) { return 0; }

int CSkypeProto::RecvContacts(MCONTACT, PROTORECVEVENT*) { return 0; }

int CSkypeProto::RecvFile(MCONTACT hContact, PROTOFILEEVENT *pre) { return 0; }

int CSkypeProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre) { return 0; }

int CSkypeProto::RecvUrl(MCONTACT, PROTORECVEVENT*) { return 0; }

int CSkypeProto::SendContacts(MCONTACT, int, int, MCONTACT*) { return 0; }

HANDLE CSkypeProto::SendFile(MCONTACT hContact, const PROTOCHAR *szDescription, PROTOCHAR **ppszFiles) { return 0; }

int CSkypeProto::SendMsg(MCONTACT hContact, int flags, const char *msg) { return 0; }

int CSkypeProto::SendUrl(MCONTACT, int, const char*) { return 0; }

int CSkypeProto::SetApparentMode(MCONTACT, int) { return 0; }

int CSkypeProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
	{
		return 0;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		// logout
		PushRequest(new LogoutRequest());
		requestQueue->Stop();

		if (!Miranda_Terminated())
		{
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	}
	else if (iNewStatus == ID_STATUS_INVISIBLE)
	{
		PushRequest(new GetEndpointRequest(ptrA(getStringA("RegistrationToken")), ptrA(getStringA("Endpoint"))));
		PushRequest(new SetStatusRequest(ptrA(getStringA("RegistrationToken")), false));
	}
	else
	{
		if (old_status == ID_STATUS_CONNECTING)
		{
			return 0;
		}

		if (old_status == ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE)
		{
			// login
			m_iStatus = ID_STATUS_CONNECTING;

			requestQueue->Start();
			PushRequest(new LoginRequest(), &CSkypeProto::OnLoginFirst);
		}
		else
		{
			// set status
			m_iStatus = iNewStatus;
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

HANDLE CSkypeProto::GetAwayMsg(MCONTACT) { return 0; }

int CSkypeProto::RecvAwayMsg(MCONTACT, int, PROTORECVEVENT*) { return 0; }

int CSkypeProto::SetAwayMsg(int, const PROTOCHAR *msg) { return 0; }

int CSkypeProto::UserIsTyping(MCONTACT hContact, int type) { return 0; }

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
	}

	return 1;
}