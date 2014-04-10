#include "common.h"

bool CSteamProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_hPollingThread;
}

bool CSteamProto::IsMe(const char *steamId)
{
	ptrA mySteamId(getStringA("SteamID"));
	if (!lstrcmpA(steamId, mySteamId))
		return true;

	return false;
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
	else
		m_iStatus = oldStatus;
}

void CSteamProto::Authorize(SteamWebApi::AuthorizationApi::AuthResult *authResult)
{
	ptrW username(getWStringA("Username"));
	ptrA base64RsaEncryptedPassword;

	const wchar_t *nickname = getWStringA("Nick");
	if (lstrlen(nickname) == 0 && username)
		setWString("Nick", username);

	// get rsa public key
	SteamWebApi::RsaKeyApi::RsaKey rsaKey;
	SteamWebApi::RsaKeyApi::GetRsaKey(m_hNetlibUser, username, &rsaKey);
	if (!rsaKey.IsSuccess())
		return;

	ptrA password(getStringA("Password"));

	DWORD error = 0;
	DWORD encryptedSize = 0;
	DWORD passwordSize = (DWORD)strlen(password);
	if ((error = RsaEncrypt(rsaKey, password, passwordSize, NULL, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::Rsa: encryption error (%lu)", error);
		return;
	}

	BYTE *encryptedPassword = (BYTE*)mir_calloc(encryptedSize);
	if ((error = RsaEncrypt(rsaKey, password, passwordSize, encryptedPassword, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::Rsa: encryption error (%lu)", error);
		return;
	}

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);

	// try to authorize
	SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, base64RsaEncryptedPassword, rsaKey.GetTimestamp(), authResult);
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
			SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, base64RsaEncryptedPassword, rsaKey.GetTimestamp(), authResult);
		} while (authResult->IsEmailAuthNeeded() || authResult->IsCaptchaNeeded());
	}
}

void CSteamProto::LogInThread(void* param)
{
	if (this->IsOnline())
		return;

	ptrA token(getStringA("TokenSecret"));
	if (!token || lstrlenA(token) == 0)
	{
		SteamWebApi::AuthorizationApi::AuthResult authResult;
		Authorize(&authResult);
		// if some error
		if (!authResult.IsSuccess())
		{
			// todo: dosplay error message from authResult.GetMessage()
			//ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			debugLogA("CSteamProto::Authorize: Error (%s)", authResult.GetMessage());

			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
			return;
		}

		token = mir_strdup(authResult.GetToken());

		setString("TokenSecret", token);
		//setString("Cookie", authResult.GetCookie());
		setString("SteamID", authResult.GetSteamid());
	}

	SteamWebApi::LoginApi::LoginResult loginResult;
	SteamWebApi::LoginApi::Logon(m_hNetlibUser, token, &loginResult);
	
	// if some error
	if (!loginResult.IsSuccess())
	{
		debugLogA("CSteamProto::Login: Error (%d)", loginResult.GetStatus());

		// token has expired
		if (loginResult.GetStatus() == HTTP_STATUS_UNAUTHORIZED)
		{
			delSetting("TokenSecret");
			//delSetting("Cookie");
		}

		// set status to offline
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}

	setString("SessionID", loginResult.GetSessionId());
	setDword("MessageID", loginResult.GetMessageId());

	// set selected status
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iDesiredStatus);

	// load contact list
	LoadContactList();

	// start pooling thread
	if (m_hPollingThread == NULL && !m_bTerminated)
	{
		m_bTerminated = false;
		m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, NULL, NULL);
	}
}

void CSteamProto::LogOutThread(void*)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));

	SteamWebApi::LoginApi::Logoff(m_hNetlibUser, token, sessionId);

	delSetting("SessionID");
}