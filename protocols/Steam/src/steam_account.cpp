#include "common.h"

void CSteamProto::LogInThread(void* param)
{
	CMStringA token(getStringA("TokenSecret"));
	if (token.IsEmpty())
	{
		ptrW username(getWStringA("Username"));
		ptrA password(getStringA("Password"));

		SteamWebApi::AuthorizationApi::Authorization authResult;
		SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, password, &authResult);
	
		if (authResult.IsEmailAuthNeeded()/* || authResult.captcha_needed*/)
		{
			do
			{
				if (authResult.IsEmailAuthNeeded())
				{
					GuardParam guard;

					//lstrcpy(guard.emailDomain, authResult.emaildomain);

					if (DialogBoxParam(
						g_hInstance,
						MAKEINTRESOURCE(IDD_GUARD),
						NULL,
						CSteamProto::GuardProc,
						(LPARAM)&guard) != 1)
						break;

					authResult.SetAuthCode(guard.code);
				}

				//if (result->captcha_needed)
				//	;// proto->fail

				SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, password, &authResult);

			} while (authResult.IsEmailAuthNeeded()/* || auth->captcha_needed*/);
		}

		if (!authResult.IsSuccess())
		{
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
			return;
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
		}

		token = authResult.GetToken();
		setString("TokenSecret", token);
		setString("SteamID", authResult.GetSteamid());
	}

	SteamWebApi::LogInApi::LogIn login;
	SteamWebApi::LogInApi::LogOn(m_hNetlibUser, token, &login);

	if (!login.IsSuccess())
	{
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}

	setString("SessionID", login.GetSessionId());
	setDword("MessageID", login.GetMessageId());

	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iDesiredStatus);

	if (m_hPollingThread == NULL && !m_bTerminated)
	{
		m_bTerminated = false;
		m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, NULL, NULL);
	}

	SteamWebApi::FriendListApi::FriendList friendList;
	SteamWebApi::FriendListApi::Load(m_hNetlibUser, token, login.GetSteamId(), &friendList);
	
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