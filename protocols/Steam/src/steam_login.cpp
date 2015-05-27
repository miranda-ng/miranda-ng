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

void CSteamProto::OnGotRsaKey(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	// load rsa key parts
	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	if (!root["success"].as_bool())
		return;

	std::string modulus = root["publickey_mod"].as_string();
	// exponent "010001" is used as constant in CSteamProto::RsaEncrypt
	//std::string exponent = root["publickey_exp"].as_string();

	std::string timestamp = root["timestamp"].as_string();

	// encrcrypt password
	ptrA base64RsaEncryptedPassword;
	ptrA password(getStringA("Password"));

	DWORD error = 0;
	DWORD encryptedSize = 0;
	if ((error = RsaEncrypt(modulus.c_str(), password, NULL, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::OnGotRsaKey: encryption error (%lu)", error);
		return;
	}

	BYTE *encryptedPassword = (BYTE*)mir_calloc(encryptedSize);
	if ((error = RsaEncrypt(modulus.c_str(), password, encryptedPassword, encryptedSize)) != 0)
	{
		debugLogA("CSteamProto::OnGotRsaKey: encryption error (%lu)", error);
		return;
	}

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);

	// run authorization request
	T2Utf username(getTStringA("Username"));

	ptrA guardId(getStringA("GuardId"));
	if (!guardId) guardId = mir_strdup("");
	ptrA guardCode(getStringA("GuardCode"));
	if (!guardCode) guardCode = mir_strdup("");

	ptrA captchaId(getStringA("CaptchaId"));
	if (!captchaId) captchaId = mir_strdup("-1");
	ptrA captchaText(getStringA("CaptchaText"));
	if (!captchaText) captchaText = mir_strdup("");

	PushRequest(
		new AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp.c_str(), "", guardCode, guardId, captchaId, captchaText),
		&CSteamProto::OnAuthorization);
}

void CSteamProto::OnAuthorization(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
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

void CSteamProto::OnAuthorizationError(const JSONNode &node)
{
	std::string message = node["message"].as_string();
	ptrT messageT(mir_utf8decodeT(message.c_str()));
	if (!mir_tstrcmpi(messageT, _T("Incorrect login.")))
	{
		// We can't continue with incorrect login/password
		delSetting("GuardId");
		delSetting("GuardCode");
		delSetting("CaptchaId");
		delSetting("CaptchaText");
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	T2Utf username(getTStringA("Username"));

	if (node["requires_twofactor"].as_bool())
	{
		return;
	}

	if (node["emailauth_needed"].as_bool())
	{
		std::string guardId = node["emailsteamid"].as_string();
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

		std::string domain = node["emaildomain"].as_string();

		CSteamGuardDialog guardDialog(this, domain.c_str());
		if (!guardDialog.DoModal())
		{
			delSetting("GuardId");
			delSetting("GuardCode");
			delSetting("CaptchaId");
			delSetting("CaptchaText");
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

	if (node["captcha_needed"].as_bool())
	{
		std::string captchaId = node["captcha_gid"].as_string();

		GetCaptchaRequest *request = new GetCaptchaRequest(captchaId.c_str());
		NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);
		delete request;

		CSteamCaptchaDialog captchaDialog(this, (BYTE*)response->pData, response->dataLength);
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
		if (!captchaDialog.DoModal())
		{
			delSetting("CaptchaId");
			delSetting("CaptchaText");
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		setString("CaptchaId", captchaId.c_str());
		setString("CaptchaText", captchaDialog.GetCaptchaText());

		PushRequest(
			new GetRsaKeyRequest(username),
			&CSteamProto::OnGotRsaKey);
		return;
	}

	delSetting("GuardId");
	delSetting("GuardCode");
	delSetting("CaptchaId");
	delSetting("CaptchaText");
	SetStatus(ID_STATUS_OFFLINE);
}

void CSteamProto::OnAuthorizationSuccess(const JSONNode &node)
{
	delSetting("GuardId");
	delSetting("GuardCode");
	delSetting("CaptchaId");
	delSetting("CaptchaText");

	if (!node["login_complete"].as_bool())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::string oauth = node["oauth"].as_string();
	JSONNode root = JSONNode::parse(oauth.c_str());
	if (!root)
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::string steamId = root["steamid"].as_string();
	setString("SteamID", steamId.c_str());

	std::string token = root["oauth_token"].as_string();
	setString("TokenSecret", token.c_str());

	std::string cookie = root["webcookie"].as_string();

	PushRequest(
		new GetSessionRequest(token.c_str(), steamId.c_str(), cookie.c_str()),
		&CSteamProto::OnGotSession);

	PushRequest(
		new LogonRequest(token.c_str()),
		&CSteamProto::OnLoggedOn);
}

void CSteamProto::OnGotSession(const NETLIBHTTPREQUEST *response)
{
	if(response == NULL)
		return;

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

void CSteamProto::HandleTokenExpired()
{
	// Notify error to user
	ShowNotification(_T("Steam"), TranslateT("Connection token expired. Please login again."));

	// Delete expired token
	delSetting("TokenSecret");

	// Set status to offline
	SetStatus(ID_STATUS_OFFLINE);
}

void CSteamProto::OnLoggedOn(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
	{
		// Probably timeout or no connection, we can do nothing here
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONROOT root(response->pData);

	JSONNODE *node = json_get(root, "error");
	ptrT error(json_as_string(node));
	if (mir_tstrcmpi(error, _T("OK")) || response->resultCode == HTTP_STATUS_UNAUTHORIZED)
	{
		// Probably expired TokenSecret
		HandleTokenExpired();
		return;
	}

	node = json_get(root, "umqid");
	setString("UMQID", ptrA(mir_u2a(ptrT(json_as_string(node)))));

	node = json_get(root, "message");
	setDword("MessageID", json_as_int(node));

	// load contact list
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));

	PushRequest(
		new GetFriendListRequest(token, steamId),
		&CSteamProto::OnGotFriendList);

	// start polling thread
	m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, 0, NULL);

	// go to online now
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);
}