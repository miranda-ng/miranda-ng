#include "common.h"

int CSkypeProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
	{
		return 0;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;
	switch (iNewStatus)
	{
	case ID_STATUS_OFFLINE:
		isTerminated = true;
		PushRequest(new LogoutRequest());
		requestQueue->Stop();
		if (!Miranda_Terminated())
		{
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		break;

	case ID_STATUS_INVISIBLE:
	case ID_STATUS_AWAY:
	case ID_STATUS_DND:
	case ID_STATUS_IDLE:
	{
		GetEndpointRequest *request = new GetEndpointRequest(getStringA("registrationToken"), getStringA("endpointId"));
			request->Send(m_hNetlibUser);
			delete request;
		PushRequest(new SetStatusRequest(ptrA(getStringA("registrationToken")), iNewStatus), &CSkypeProto::OnSetStatus);
		break;
	}
	default:
		if (old_status == ID_STATUS_CONNECTING)
			return 0;

		if (m_iStatus == ID_STATUS_INVISIBLE || m_iStatus == ID_STATUS_AWAY || m_iStatus == ID_STATUS_DND || m_iStatus == ID_STATUS_IDLE)
		{
				   GetEndpointRequest *request = new GetEndpointRequest(getStringA("registrationToken"), getStringA("endpointId"));
						request->Send(m_hNetlibUser);
						delete request;
				   PushRequest(new SetStatusRequest(ptrA(getStringA("registrationToken")), ID_STATUS_ONLINE), &CSkypeProto::OnSetStatus);
		}
		else if (old_status == ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE)
		{
			// login
			isTerminated = false;
			m_iStatus = ID_STATUS_CONNECTING;

			requestQueue->Start();
			PushRequest(new LoginRequest(), &CSkypeProto::OnLoginFirst);
		}
		else
		{
			// set status
			m_iStatus = iNewStatus;
		}
		break;

	}
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

void CSkypeProto::SetServerStatus(int iNewStatus){ return; }