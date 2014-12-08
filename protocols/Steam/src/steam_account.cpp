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
	if (response == NULL)
		return;

	// load rsa key parts
	JSONROOT root(response->pData);
	if (!root)
		return;

	JSONNODE *node = json_get(root, "success");
	if (!json_as_bool(node)) {
		return;
	}

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
	if (response == NULL) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONROOT root(response->pData);

	JSONNODE *node = json_get(root, "success");
	if (json_as_bool(node) == 0)
	{
		node = json_get(root, "message");
		const wchar_t *message = json_as_string(node);
		if (!lstrcmpi(json_as_string(node), L"Incorrect login"))
		{
			ShowNotification(TranslateTS(message));
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		node = json_get(root, "emailauth_needed");
		if (json_as_bool(node) > 0)
		{
			node = json_get(root, "emailsteamid");
			ptrA guardId(mir_u2a(json_as_string(node)));

			node = json_get(root, "emaildomain");
			ptrA emailDomain(mir_utf8encodeW(json_as_string(node)));

			GuardParam guard;
			lstrcpyA(guard.domain, emailDomain);

			if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_GUARD), NULL, CSteamProto::GuardProc, (LPARAM)&guard) != 1) {
				return;
			}

			ptrA username(mir_urlEncode(ptrA(mir_utf8encodeW(getWStringA("Username")))));
			ptrA base64RsaEncryptedPassword(getStringA("EncryptedPassword"));
			ptrA timestamp(getStringA("Timestamp"));

			PushRequest(
				new SteamWebApi::AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp, guardId, guard.code),
				&CSteamProto::OnAuthorization);
		}

		node = json_get(root, "captcha_needed");
		if (json_as_bool(node) > 0)
		{
			node = json_get(root, "captcha_gid");
			ptrA captchaId(mir_u2a(json_as_string(node)));

			char url[MAX_PATH];
			mir_snprintf(url, SIZEOF(url), STEAM_COM_URL "/public/captcha.php?gid=%s", captchaId);

			SteamWebApi::GetCaptchaRequest *request = new SteamWebApi::GetCaptchaRequest(url);
			request->szUrl = (char*)request->url.c_str();
			NETLIBHTTPREQUEST *response = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)request);
			delete request;

			CaptchaParam captcha = { 0 };
			captcha.size = response->dataLength;
			captcha.data = (BYTE*)mir_alloc(captcha.size);
			memcpy(captcha.data, response->pData, captcha.size);

			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);

			int res = DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_CAPTCHA),
				NULL,
				CSteamProto::CaptchaProc,
				(LPARAM)&captcha);

			mir_free(captcha.data);

			if (res != 1)
			{
				SetStatus(ID_STATUS_OFFLINE);
				return;
			}

			ptrA username(mir_urlEncode(ptrA(mir_utf8encodeW(getWStringA("Username")))));
			ptrA base64RsaEncryptedPassword(getStringA("EncryptedPassword"));
			ptrA timestamp(getStringA("Timestamp"));

			PushRequest(
				new SteamWebApi::AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp, "-1", "", captchaId, captcha.text),
				&CSteamProto::OnAuthorization);
		}

		return;
	}

	node = json_get(root, "login_complete");
	if (!json_as_bool(node))
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	node = json_get(root, "oauth");
	JSONROOT nroot(ptrA(mir_u2a(json_as_string(node))));

	node = json_get(nroot, "steamid");
	ptrA steamId(mir_u2a(json_as_string(node)));
	setString("SteamID", steamId);

	node = json_get(nroot, "oauth_token");
	ptrA token(mir_u2a(json_as_string(node)));
	setString("TokenSecret", token);

	node = json_get(nroot, "webcookie");
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
	if (response == NULL)
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

void CSteamProto::OnLoggedOn(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
	{
		// set status to offline
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}

	JSONROOT root(response->pData);

	JSONNODE *node = json_get(root, "error");
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