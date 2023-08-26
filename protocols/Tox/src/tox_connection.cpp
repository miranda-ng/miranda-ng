#include "stdafx.h"

bool CToxProto::IsOnline()
{
	return m_tox && m_iStatus >= ID_STATUS_ONLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CToxProto::OnLoggedIn()
{
	debugLogA(__FUNCTION__": successfuly connected to DHT");

	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
	tox_self_set_status(m_tox, MirandaToToxStatus(m_iStatus));

	debugLogA(__FUNCTION__": changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);

	UpdateStatusMenu(NULL, NULL);

	LoadFriendList(m_tox);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tox polling callback

static void sttScheduledDisconnect(void *param)
{
	auto *ppro = (CToxProto *)param;
	ppro->SetStatus(ID_STATUS_OFFLINE);
	ppro->ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
	ppro->debugLogA(__FUNCTION__": failed to connect to DHT");
}

void CToxProto::OnConnectionStatus(Tox*, Tox_Connection iNewStatus, void *pUserData)
{
	auto *ppro = (CToxProto *)pUserData;
	if (ppro->m_tox == nullptr)
		return;

	if (iNewStatus == TOX_CONNECTION_NONE) {
		// we cannot destroy Tox object inside its hook... #3649
		Miranda_WaitOnHandleEx(sttScheduledDisconnect, ppro);
	}
	else ppro->OnLoggedIn();
}

void CToxProto::OnToxPoll()
{
	if (m_tox)
		tox_iterate(m_tox, this);
}
