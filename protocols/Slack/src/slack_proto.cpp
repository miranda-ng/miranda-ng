#include "stdafx.h"

CSlackProto::CSlackProto(const char* protoName, const TCHAR* userName) :
	PROTO<CSlackProto>(protoName, userName), requestQueue(1),
	hMessageProcess(1)
{
	InitNetlib();

	//CreateProtoService(PS_CREATEACCMGRUI, &CSlackProto::OnAccountManagerInit);

	SetAllContactsStatus(ID_STATUS_OFFLINE);
}

CSlackProto::~CSlackProto()
{
	UninitNetlib();
}

DWORD_PTR CSlackProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IMSEND;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"User Id";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"UserId";
	}

	return 0;
}

MCONTACT CSlackProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	return NULL;
}

int CSlackProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int CSlackProto::AuthRequest(MCONTACT hContact, const wchar_t *szMessage)
{
	ptrA reason(mir_utf8encodeW(szMessage));
	return OnRequestAuth(hContact, (LPARAM)reason);
}

int CSlackProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return OnReceiveMessage(hContact, pre);
}

int CSlackProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	if (!IsOnline())
	{
		ForkThread(&CSlackProto::SendMessageAckThread, (void*)hContact);
		return 1;
	}

	UINT hMessage = InterlockedIncrement(&hMessageProcess);

	SendMessageParam *param = (SendMessageParam*)mir_calloc(sizeof(SendMessageParam));
	param->hContact = hContact;
	param->hMessage = hMessage;
	param->message = mir_strdup(msg);
	ForkThread(&CSlackProto::SendMessageThread, param);

	return hMessage;
}

int CSlackProto::SetStatus(int iNewStatus)
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

	case ID_STATUS_DND:
	case ID_STATUS_INVISIBLE:
		iNewStatus = ID_STATUS_OCCUPIED;
		break;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		// logout
		LogOut();

		if (!Miranda_IsTerminated())
		{
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	}
	else
	{
		if (old_status == ID_STATUS_CONNECTING)
		{
			return 0;
		}

		if (old_status == ID_STATUS_OFFLINE && !IsOnline())
		{
			// login
			m_iStatus = ID_STATUS_CONNECTING;

			isTerminated = false;

			hRequestQueueThread = ForkThreadEx(&CSlackProto::RequestQueueThread, NULL, NULL);
		}
		else
		{
			// set tox status
			m_iStatus = iNewStatus;
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

int CSlackProto::UserIsTyping(MCONTACT hContact, int type)
{
	return OnUserIsTyping(hContact, type);
}

int CSlackProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	switch (iEventType)
	{
	case EV_PROTO_ONLOAD:
		return OnAccountLoaded(wParam, lParam);

	case EV_PROTO_ONERASE:
		return OnAccountDeleted(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);
	}

	return 1;
}
