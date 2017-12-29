#include "stdafx.h"

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

void CSteamProto::Login()
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	if (mir_strlen(token) > 0 && mir_strlen(sessionId) > 0)
	{
		PushRequest(
			new LogonRequest(token),
			&CSteamProto::OnLoggedOn);
	}
	else
	{
		T2Utf username(getWStringA("Username"));
		if (username == NULL)
		{
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		PushRequest(
			new GetRsaKeyRequest(username),
			&CSteamProto::OnGotRsaKey);
	}
}

bool CSteamProto::Relogin()
{
	ptrA token(getStringA("TokenSecret"));
	if (mir_strlen(token) <= 0)
		return false;

	HttpRequest *request = new LogonRequest(token);
	HttpResponse *response = SendRequest(request);

	bool success = false;
	if (response)
	{
		JSONNode root = JSONNode::parse(response->Content);
		if (root.isnull()) {
			json_string error = root["error"].as_string();
			if (!mir_strcmpi(error.c_str(), "OK"))
			{
				json_string umqId = root["umqid"].as_string();
				setString("UMQID", umqId.c_str());

				long messageId = root["message"].as_int();
				setDword("MessageID", messageId);

				success = true;
			}
		}
	}

	delete response;

	return success;
}

void CSteamProto::LogOut()
{
	isTerminated = true;
	if (m_hRequestQueueThread)
		SetEvent(m_hRequestsQueueEvent);

	ptrA token(getStringA("TokenSecret"));
	if (mir_strlen(token) > 0)
	{
		ptrA umqid(getStringA("UMQID"));
		SendRequest(new LogoffRequest(token, umqid));
	}
}

void CSteamProto::OnGotRsaKey(const JSONNode &root, void*)
{
	if (root.isnull())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!root["success"].as_bool())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	// load rsa key parts
	json_string modulus = root["publickey_mod"].as_string();
	json_string exp = root["publickey_exp"].as_string();
	DWORD exponent = strtoul(exp.c_str(), nullptr, 16); // default "010001" = 0x10001

	json_string timestamp = root["timestamp"].as_string();

	// encrcrypt password
	ptrA base64RsaEncryptedPassword;
	ptrA szPassword(getStringA("Password"));

	DWORD error = 0;
	DWORD encryptedSize = 0;
	if ((error = RsaEncrypt(modulus.c_str(), exponent, szPassword, nullptr, encryptedSize)) != 0)
	{
		debugLogA(__FUNCTION__ ": encryption error (%lu)", error);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	BYTE *encryptedPassword = (BYTE*)mir_calloc(encryptedSize);
	if ((error = RsaEncrypt(modulus.c_str(), exponent, szPassword, encryptedPassword, encryptedSize)) != 0)
	{
		debugLogA(__FUNCTION__ ": encryption error (%lu)", error);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);

	// run authorization request
	T2Utf username(getWStringA("Username"));

	ptrA twoFactorCode(getStringA("TwoFactorCode"));
	if (!twoFactorCode) twoFactorCode = mir_strdup("");

	ptrA guardId(getStringA("GuardId"));
	if (!guardId) guardId = mir_strdup("");
	ptrA guardCode(getStringA("GuardCode"));
	if (!guardCode) guardCode = mir_strdup("");

	ptrA captchaId(getStringA("CaptchaId"));
	if (!captchaId) captchaId = mir_strdup("-1");
	ptrA captchaText(getStringA("CaptchaText"));
	if (!captchaText) captchaText = mir_strdup("");

	PushRequest(
		new AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp.c_str(), twoFactorCode, guardCode, guardId, captchaId, captchaText),
		&CSteamProto::OnAuthorization);
}

void CSteamProto::OnGotCaptcha(const HttpResponse &response, void *arg)
{
	ptrA captchaId((char*)arg);

	if (!response.IsSuccess())
	{
		debugLogA(__FUNCTION__ ": failed to get captcha");
		return;
	}

	CSteamCaptchaDialog captchaDialog(this, (BYTE*)response.Content.GetData(), response.Content.GetSize());
	if (captchaDialog.DoModal() != DIALOG_RESULT_OK)
	{
		DeleteAuthSettings();
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	setString("CaptchaId", captchaId);
	setString("CaptchaText", captchaDialog.GetCaptchaText());

	T2Utf username(getWStringA("Username"));
	PushRequest(
		new GetRsaKeyRequest(username),
		&CSteamProto::OnGotRsaKey);
}

void CSteamProto::OnAuthorization(const HttpResponse &response, void*)
{
	if (!response)
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!root["success"].as_bool())
	{
		OnAuthorizationError(root);
		return;
	}

	OnAuthorizationSuccess(root);
}

void CSteamProto::DeleteAuthSettings()
{
	delSetting("TwoFactorCode");
	delSetting("GuardId");
	delSetting("GuardCode");
	delSetting("CaptchaId");
	delSetting("CaptchaText");
}

void CSteamProto::OnAuthorizationError(const JSONNode &root)
{
	CMStringW message = root["message"].as_mstring();
	debugLogA(__FUNCTION__ ": %s", _T2A(message.GetBuffer()));
	if (!mir_wstrcmpi(message, L"Incorrect login."))
	{
		// We can't continue with incorrect login/password
		DeleteAuthSettings();
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	T2Utf username(getWStringA("Username"));

	if (root["requires_twofactor"].as_bool())
	{
		debugLogA(__FUNCTION__ ": requires twofactor");

		CSteamTwoFactorDialog twoFactorDialog(this);
		if (twoFactorDialog.DoModal() != DIALOG_RESULT_OK)
		{
			DeleteAuthSettings();
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		setString("TwoFactorCode", twoFactorDialog.GetTwoFactorCode());

		PushRequest(
			new GetRsaKeyRequest(username),
			&CSteamProto::OnGotRsaKey);
		return;
	}

	if (root["clear_password_field"].as_bool())
	{
		debugLogA(__FUNCTION__ ": clear password field");
		// describes if the password field was cleared. This can also be true if the twofactor code was wrong
		DeleteAuthSettings();
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (root["emailauth_needed"].as_bool())
	{
		debugLogA(__FUNCTION__ ": emailauth needed");

		std::string guardId = root["emailsteamid"].as_string();
		ptrA oldGuardId(getStringA("GuardId"));
		if (mir_strcmp(guardId.c_str(), oldGuardId) == 0)
		{
			delSetting("GuardId");
			delSetting("GuardCode");
			PushRequest(
				new GetRsaKeyRequest(username),
				&CSteamProto::OnGotRsaKey);
			return;
		}

		std::string domain = root["emaildomain"].as_string();

		// Make absolute link
		if (domain.find("://") == std::string::npos)
			domain = "http://" + domain;

		CSteamGuardDialog guardDialog(this, domain.c_str());
		if (guardDialog.DoModal() != DIALOG_RESULT_OK)
		{
			DeleteAuthSettings();
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		setString("GuardId", guardId.c_str());
		setString("GuardCode", guardDialog.GetGuardCode());

		PushRequest(
			new GetRsaKeyRequest(username),
			&CSteamProto::OnGotRsaKey);
		return;
	}

	if (root["captcha_needed"].as_bool())
	{
		debugLogA(__FUNCTION__ ": captcha needed");
		std::string captchaId = root["captcha_gid"].as_string();
		PushRequest(
			new GetCaptchaRequest(captchaId.c_str()),
			&CSteamProto::OnGotCaptcha,
			mir_strdup(captchaId.c_str()));
		return;
	}

	DeleteAuthSettings();
	SetStatus(ID_STATUS_OFFLINE);
	ShowNotification(L"Steam", message);
}

void CSteamProto::OnAuthorizationSuccess(const JSONNode &root)
{
	DeleteAuthSettings();

	if (!root["login_complete"].as_bool())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::string oauth = root["oauth"].as_string();
	JSONNode node = JSONNode::parse(oauth.c_str());
	if (node.isnull())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::string steamId = node["steamid"].as_string();
	setString("SteamID", steamId.c_str());

	std::string token = node["oauth_token"].as_string();
	setString("TokenSecret", token.c_str());

	std::string cookie = node["webcookie"].as_string();

	SendRequest(
		new GetSessionRequest(token.c_str(), steamId.c_str(), cookie.c_str()),
		&CSteamProto::OnGotSession);

	// We need to load homepage to get sessionid cookie
	SendRequest(
		new GetSessionRequest2(),
		&CSteamProto::OnGotSession);

	PushRequest(
		new LogonRequest(token.c_str()),
		&CSteamProto::OnLoggedOn);
}

void CSteamProto::OnGotSession(const HttpResponse &response, void*)
{
	if (!response)
		return;

	for (size_t i = 0; i < response.Headers.GetSize(); i++)
	{
		if (lstrcmpiA(response.Headers[i]->szName, "Set-Cookie"))
			continue;

		std::string cookies = response.Headers[i]->szValue;
		size_t start = cookies.find("sessionid=") + 10;
		size_t end = cookies.substr(start).find(';');
		std::string sessionId = cookies.substr(start, end - start + 10);
		setString("SessionID", sessionId.c_str());
		break;
	}
}

void CSteamProto::HandleTokenExpired()
{
	// Delete expired token
	delSetting("TokenSecret");

	// Try to relogin automatically (but only once)
	if (isLoginAgain) {
		// Notify error to user
		debugLogA(__FUNCTION__ ": cannot obtain connection token");
		ShowNotification(L"Steam", TranslateT("Cannot obtain connection token."));
		// Just go offline; it also resets the isLoginAgain to false
		SetStatus(ID_STATUS_OFFLINE);
	}
	else
	{
		// Remember we are trying to relogin
		isLoginAgain = true;

		// Remember status user wanted
		int desiredStatus = m_iDesiredStatus;

		// Set status to offline
		SetStatus(ID_STATUS_OFFLINE);

		// Try to login again automatically
		SetStatus(desiredStatus);
	}
}

void CSteamProto::OnLoggedOn(const HttpResponse &response, void*)
{
	if (!response.IsSuccess())
	{
		// Probably timeout or no connection, we can do nothing here
		debugLogA(__FUNCTION__ ": unknown login error");
		ShowNotification(L"Steam", TranslateT("Unknown login error."));
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (response.GetStatusCode() == HTTP_CODE_UNAUTHORIZED)
	{
		// Probably expired TokenSecret
		HandleTokenExpired();
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	CMStringW error = root["error"].as_mstring();
	if (mir_wstrcmpi(error, L"OK"))
	{
		// Probably expired TokenSecret
		HandleTokenExpired();
		return;
	}

	json_string umqId = root["umqid"].as_string();
	setString("UMQID", umqId.c_str());

	long messageId = root["umqid"].as_int();
	setDword("MessageID", messageId);
	
	if (m_lastMessageTS <= 0)
	{
		time_t timestamp = _wtoi64(root["utc_timestamp"].as_mstring());
		setDword("LastMessageTS", timestamp);
	}

	// load contact list
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));

	// send this request immediately, so we can start polling thread with already loaded all contacts
	SendRequest(
		new GetFriendListRequest(token, steamId),
		&CSteamProto::OnGotFriendList);

	// go to online now
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);

	// start polling thread
	m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, nullptr, nullptr);
}
