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

void CSteamProto::OnGotRsaKey(const NETLIBHTTPREQUEST *response, void *arg)
{
	// load rsa key parts
	JSONNODE *root = json_parse(response->pData), *node;
	if (!root) return;

	node = json_get(root, "success");
	if (!json_as_bool(node)) return;

	node = json_get(root, "publickey_mod");
	ptrA modulus(mir_u2a(json_as_string(node)));

	// exponent "010001" is used as constant in CSteamProto::RsaEncrypt
	/*node = json_get(root, "publickey_exp");
	ptrA exponent(mir_u2a(json_as_string(node)));*/

	node = json_get(root, "timestamp");
	ptrA timestamp(mir_u2a(json_as_string(node)));
	setString("Timestamp", timestamp);

	// encrcrypt password
	ptrA base64RsaEncryptedPassword;
	ptrA password(getStringA("Password"));

	DWORD error = 0;
	DWORD encryptedSize = 0;
	DWORD passwordSize = (DWORD)strlen(password);
	if ((error = RsaEncrypt(modulus, password, NULL, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::OnGotRsaKey: encryption error (%lu)", error);
		return;
	}

	BYTE *encryptedPassword = (BYTE*)mir_calloc(encryptedSize);
	if ((error = RsaEncrypt(modulus, password, encryptedPassword, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::OnGotRsaKey: encryption error (%lu)", error);
		return;
	}

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);

	setString("EncryptedPassword", base64RsaEncryptedPassword);
	
	// run authorization request
	ptrA username(mir_urlEncode(ptrA(mir_utf8encodeW(getWStringA("Username")))));

	PushRequest(
		new SteamWebApi::AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp),
		&CSteamProto::OnAuthorization);
}

void CSteamProto::OnAuthorization(const NETLIBHTTPREQUEST *response, void *arg)
{
	JSONNODE *root = json_parse(response->pData), *node;

	node = json_get(root, "success");
	if (json_as_bool(node) == 0)
	{
		node = json_get(root, "emailauth_needed");
		if (json_as_bool(node) > 0)
		{
			node = json_get(root, "emailsteamid");
			ptrA guardId(mir_u2a(json_as_string(node)));

			node = json_get(root, "emaildomain");
			ptrA emailDomain(mir_utf8encodeW(json_as_string(node)));

			GuardParam guard;
			lstrcpyA(guard.domain, emailDomain);

			if (DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_GUARD),
				NULL,
				CSteamProto::GuardProc,
				(LPARAM)&guard) != 1)
				return;

			ptrA username(mir_urlEncode(ptrA(mir_utf8encodeW(getWStringA("Username")))));
			ptrA base64RsaEncryptedPassword(getStringA("EncryptedPassword"));
			ptrA timestamp(getStringA("Timestamp"));

			PushRequest(
				new SteamWebApi::AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp, guardId, guard.code),
				&CSteamProto::OnAuthorization);
		}

		// todo: show captcha dialog
		/*node = json_get(root, "captcha_needed");
		if (json_as_bool(node) > 0)
		{
			node = json_get(root, "captcha_gid");
			authResult->captchagid = ptrA(mir_u2a(json_as_string(node)));
		}

		if (!authResult->emailauth_needed && !authResult->captcha_needed)
		{
			node = json_get(root, "message");
			authResult->message = json_as_string(node);
		}*/

		return;
	}
	
	node = json_get(root, "login_complete");
	if (!json_as_bool(node))
		return;

	node = json_get(root, "oauth");
	root = json_parse(ptrA(mir_u2a(json_as_string(node))));

	node = json_get(root, "steamid");
	ptrA steamId(mir_u2a(json_as_string(node)));
	setString("SteamID", steamId);

	node = json_get(root, "oauth_token");
	ptrA token(mir_u2a(json_as_string(node)));
	setString("TokenSecret", token);

	node = json_get(root, "webcookie");
	ptrA cookie(mir_u2a(json_as_string(node)));

	delSetting("Timestamp");
	delSetting("EncryptedPassword");

	PushRequest(
		new SteamWebApi::GetSessionRequest(token, steamId, cookie),
		&CSteamProto::OnGotSession);

	PushRequest(
		new SteamWebApi::LogonRequest(token),
		&CSteamProto::OnLoggedOn);
}

void CSteamProto::OnGotSession(const NETLIBHTTPREQUEST *response, void *arg)
{
	for (int i = 0; i < response->headersCount; i++)
	{
		if (lstrcmpiA(response->headers[i].szName, "Set-Cookie"))
			continue;

		std::string cookies = response->headers[i].szValue;
		size_t start = cookies.find("sessionid=") + 10;
		size_t end = cookies.substr(start).find(';');
		std::string sessionId = cookies.substr(start, end - start + 10);
		setString("SessionID", sessionId.c_str());
		break;
	}
}

void CSteamProto::OnLoggedOn(const NETLIBHTTPREQUEST *response, void *arg)
{
	JSONNODE *root = json_parse(response->pData), *node;

	node = json_get(root, "error");
	ptrW error(json_as_string(node));
	if (lstrcmpi(error, L"OK")/* || response->resultCode == HTTP_STATUS_UNAUTHORIZED*/)
	{
		//delSetting("TokenSecret");
		//delSetting("Cookie");

		// set status to offline
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}

	node = json_get(root, "umqid");
	setString("UMQID", ptrA(mir_u2a(json_as_string(node))));
	
	node = json_get(root, "message");
	setDword("MessageID", json_as_int(node));

	// load contact list
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));

	PushRequest(
		new SteamWebApi::GetFriendListRequest(token, steamId),
		&CSteamProto::OnGotFriendList);

	// start polling thread
	m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, 0, NULL);

	// go to online now
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);
}

void CSteamProto::SetServerStatusThread(void *arg)
{
	WORD status = *((WORD*)&arg);

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));

	int state = CSteamProto::MirandaToSteamState(status);

	// change status
	WORD oldStatus = m_iStatus;
	m_iDesiredStatus = status;

	SteamWebApi::MessageApi::SendResult sendResult;
	debugLogA("CSteamProto::SetServerStatusThread: call SteamWebApi::MessageApi::SendStatus");
	SteamWebApi::MessageApi::SendStatus(m_hNetlibUser, token, umqId, state, &sendResult);

	if (sendResult.IsSuccess())
	{
		m_iStatus = status;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else
		m_iDesiredStatus = m_iStatus;
}

void CSteamProto::SetServerStatus(WORD status)
{
	if (m_iStatus == status)
		return;

	ForkThread(&CSteamProto::SetServerStatusThread, (void*)status);
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
	debugLogA("CSteamProto::Authorize: call SteamWebApi::RsaKeyApi::GetRsaKey");
	SteamWebApi::RsaKeyApi::GetRsaKey(m_hNetlibUser, username, &rsaKey);
	if (!rsaKey.IsSuccess())
		return;

	ptrA password(getStringA("Password"));

	/*DWORD error = 0;
	DWORD encryptedSize = 0;
	DWORD passwordSize = (DWORD)strlen(password);
	if ((error = RsaEncrypt(rsaKey, password, passwordSize, NULL, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::Authorize: encryption error (%lu)", error);
		return;
	}

	BYTE *encryptedPassword = (BYTE*)mir_calloc(encryptedSize);
	if ((error = RsaEncrypt(rsaKey, password, passwordSize, encryptedPassword, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::Authorize: encryption error (%lu)", error);
		return;
	}

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);*/

	// try to authorize
	debugLogA("CSteamProto::Authorize: call SteamWebApi::AuthorizationApi::Authorize");
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
			debugLogA("CSteamProto::Authorize: call SteamWebApi::AuthorizationApi::Authorize");
			SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, base64RsaEncryptedPassword, rsaKey.GetTimestamp(), authResult);
		} while (authResult->IsEmailAuthNeeded() || authResult->IsCaptchaNeeded());
	}
}

void CSteamProto::LogInThread(void* param)
{
	while (isTerminated || m_hPollingThread != NULL)
		Sleep(500);

	ptrA token(getStringA("TokenSecret"));
	if (!token || lstrlenA(token) == 0)
	{
		SteamWebApi::AuthorizationApi::AuthResult authResult;
		Authorize(&authResult);
		// if some error
		if (!authResult.IsSuccess())
		{
			// todo: display error message from authResult.GetMessage()
			//ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			debugLogA("CSteamProto::LogInThread: Authorization error (%s)", authResult.GetMessage());

			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
			return;
		}

		token = mir_strdup(authResult.GetToken());

		setString("TokenSecret", token);
		//setString("Cookie", authResult.GetCookie());
		setString("SteamID", authResult.GetSteamid());
		setString("SessionID", authResult.GetSessionId());
	}

	SteamWebApi::LoginApi::LoginResult loginResult;
	debugLogA("CSteamProto::LogInThread: call SteamWebApi::LoginApi::Logon");
	SteamWebApi::LoginApi::Logon(m_hNetlibUser, token, &loginResult);
	
	// if some error
	if (!loginResult.IsSuccess())
	{
		debugLogA("CSteamProto::LogInThread: Login error (%d)", loginResult.GetStatus());

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

	setString("UMQID", loginResult.GetUmqId());
	setDword("MessageID", loginResult.GetMessageId());

	// set selected status
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);

	/*ptrA sessionId(getStringA("SessionID"));
	if (!sessionId || lstrlenA(sessionId) == 0)
	{
		SteamWebApi::SessionApi::SessionId result;
		debugLogA("CSteamProto::LogInThread: call SteamWebApi::SessionApi::GetSessionId");
		SteamWebApi::SessionApi::GetSessionId(m_hNetlibUser, token, loginResult.GetSteamId(), &result);
		if (result.IsSuccess())
			setString("SessionID", result.GetSessionId());
	}*/

	// load contact list
	LoadContactListThread(NULL);

	// start pooling thread
	if (m_hPollingThread == NULL)
	{
		isTerminated = false;
		m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, NULL, NULL);
	}
}

void CSteamProto::LogOutThread(void*)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));

	while (!Miranda_Terminated() && isTerminated && m_hPollingThread != NULL)
		Sleep(200);

	debugLogA("CSteamProto::LogOutThread: call SteamWebApi::LoginApi::Logoff");
	SteamWebApi::LoginApi::Logoff(m_hNetlibUser, token, umqId);

	delSetting("UMQID");
	isTerminated = false;
}