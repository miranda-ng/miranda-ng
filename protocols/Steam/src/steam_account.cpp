#include "common.h"

WORD CSteamProto::SteamToMirandaStatus(int state)
{
	switch (state)
	{
	case 0: //Offline
		return ID_STATUS_OFFLINE;
	case 2: //Busy
		return ID_STATUS_DND;
	case 3: //Away
		return ID_STATUS_AWAY;
		/*case 4: //Snoozing
		prim = PURPLE_STATUS_EXTENDED_AWAY;
		break;
		case 5: //Looking to trade
		return "trade";
		case 6: //Looking to play
		return "play";*/
		//case 1: //Online
	default:
		return ID_STATUS_ONLINE;
	}
}

int CSteamProto::MirandaToSteamState(int status)
{
	switch (status)
	{
	case ID_STATUS_ONLINE:
		return 1; //Online
	case ID_STATUS_DND:
		return 2; //Busy
	case ID_STATUS_AWAY:
		return 3; //Away
		/*case 4: //Snoozing
		prim = PURPLE_STATUS_EXTENDED_AWAY;
		break;
		case 5: //Looking to trade
		return "trade";
		case 6: //Looking to play
		return "play";*/
		//case 1: //Online
	default:
		return ID_STATUS_OFFLINE;
	}
}

bool CSteamProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE;
}

void CSteamProto::SetServerStatusThread(void *arg)
{
	WORD status = *((WORD*)arg);

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));

	int state = CSteamProto::MirandaToSteamState(status);

	// change status
	if (m_iStatus == state)
		return;

	int oldStatus = m_iStatus;
	m_iStatus = state;

	SteamWebApi::MessageApi::SendResult sendResult;
	SteamWebApi::MessageApi::SendStatus(m_hNetlibUser, token, sessionId, state, &sendResult);

	if (sendResult.IsSuccess())
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}

void CSteamProto::Authorize(SteamWebApi::AuthorizationApi::AuthResult *authResult)
{
	ptrW username(getWStringA("Username"));
	ptrA password(getStringA("Password"));

	// try to authorize
	SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, password, authResult);
	if (authResult->IsEmailAuthNeeded() || authResult->IsCaptchaNeeded())
	{
		do
		{
			if (authResult->IsEmailAuthNeeded())
			{
				GuardParam guard;

				lstrcpyA(guard.domain, authResult->GetEmailDomain());

				if (DialogBoxParam(
					g_hInstance,
					MAKEINTRESOURCE(IDD_GUARD),
					NULL,
					CSteamProto::GuardProc,
					(LPARAM)&guard) != 1)
					break;

				authResult->SetAuthCode(guard.code);
			}
				
			if (authResult->IsCaptchaNeeded())
			{
				// todo: show captcha dialog
			}

			// try to authorize with emailauthcode or captcha taxt
			SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, password, authResult);
		} while (authResult->IsEmailAuthNeeded() || authResult->IsCaptchaNeeded());
	}
}

void CSteamProto::LogInThread(void* param)
{
	CMStringA token(getStringA("TokenSecret"));
	if (token.IsEmpty()/* && !this->IsOnline()*/)
	{
		SteamWebApi::AuthorizationApi::AuthResult authResult;
		Authorize(&authResult);
		// if some error
		if (!authResult.IsSuccess())
		{
			// todo: dosplay error message from authResult.GetMessage()
			//ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
			return;
		}

		token = authResult.GetToken();
		setString("TokenSecret", token);
		setString("SteamID", authResult.GetSteamid());
	}

	SteamWebApi::LoginApi::LoginResult loginResult;
	SteamWebApi::LoginApi::Logon(m_hNetlibUser, token, &loginResult);
	// if some error
	if (!loginResult.IsSuccess())
	{
		// set status to offline
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}
	else
	{
		setString("SessionID", loginResult.GetSessionId());
		setDword("MessageID", loginResult.GetMessageId());
		// set selected status
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iDesiredStatus);
	}

	// start pooling thread
	if (m_hPollingThread == NULL && !m_bTerminated)
	{
		m_bTerminated = false;
		m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, NULL, NULL);
	}

	// get contact list
	SteamWebApi::FriendListApi::FriendList friendList;
	SteamWebApi::FriendListApi::Load(m_hNetlibUser, token, loginResult.GetSteamId(), &friendList);
	
	if (friendList.IsSuccess())
	{
		for (int i = 0; i < friendList.GetCount(); i++)
		{
			if (!FindContact(friendList[i]))
			{
				SteamWebApi::FriendApi::Friend rFriend;
				SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, friendList[i], &rFriend);
				if (!rFriend.IsSuccess()) continue;
				AddContact(rFriend);
			}
		}
	}
}

void CSteamProto::LogOutThread(void*)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));

	SteamWebApi::LoginApi::Logoff(m_hNetlibUser, token, sessionId);

	delSetting("SessionID");
}