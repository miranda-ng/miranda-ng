#include "stdafx.h"

/*
void CSteamProto::OnGotCaptcha(const HttpResponse &response, void *arg)
{
	ptrA captchaId((char *)arg);

	if (!response.IsSuccess()) {
		debugLogA(__FUNCTION__ ": failed to get captcha");
		return;
	}

	CSteamCaptchaDialog captchaDialog(this, (const uint8_t *)response.data(), response.length());
	if (!captchaDialog.DoModal()) {
		DeleteAuthSettings();
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	setString("CaptchaId", captchaId);
	setString("CaptchaText", captchaDialog.GetCaptchaText());

	ptrA username(getUStringA("Username"));
	// SendRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
}

void CSteamProto::OnAuthorizationError(const JSONNode &root)
{
	CMStringW message = root["message"].as_mstring();
	if (message == L"Incorrect login.") {
		// We can't continue with incorrect login/password
		debugLogA(__FUNCTION__ ": incorrect login");
		DeleteAuthSettings();
		SetStatus(ID_STATUS_OFFLINE);
		ShowNotification(message);
		return;
	}

	if (root["clear_password_field"].as_bool()) {
		// describes if the password field was cleared. This can also be true if the twofactor code was wrong
		debugLogA(__FUNCTION__ ": clear password field");
		delSetting("Passowrd"); // experiment
		delSetting("TwoFactorCode");
		SetStatus(ID_STATUS_OFFLINE);
		ShowNotification(message);
	}

	T2Utf username(getWStringA("Username"));

	if (root["requires_twofactor"].as_bool()) {
		debugLogA(__FUNCTION__ ": requires twofactor");
		delSetting("TwoFactorCode");

		CSteamTwoFactorDialog twoFactorDialog(this);
		if (!twoFactorDialog.DoModal()) {
			DeleteAuthSettings();
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		setString("TwoFactorCode", twoFactorDialog.GetTwoFactorCode());

		// SendRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
	}

	if (root["emailauth_needed"].as_bool()) {
		debugLogA(__FUNCTION__ ": emailauth needed");

		std::string guardId = root["emailsteamid"].as_string();
		ptrA oldGuardId(getStringA("GuardId"));
		if (mir_strcmp(guardId.c_str(), oldGuardId) == 0) {
			delSetting("GuardId");
			delSetting("GuardCode");
		}

		json_string domain = root["emaildomain"].as_string();

		// Make absolute link
		if (domain.find("://") == std::string::npos)
			domain = "http://" + domain;

		if (m_hwndGuard != nullptr)
			return;

		CSteamGuardDialog guardDialog(this, domain.c_str());
		if (!guardDialog.DoModal()) {
			DeleteAuthSettings();
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}

		setString("GuardId", guardId.c_str());
		setString("GuardCode", guardDialog.GetGuardCode());

		// SendRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
		return;
	}

	if (root["captcha_needed"].as_bool()) {
		debugLogA(__FUNCTION__ ": captcha needed");
		delSetting("CaptchaId");
		delSetting("CaptchaText");
		json_string captchaId = root["captcha_gid"].as_string();
		SendRequest(new GetCaptchaRequest(captchaId.c_str()), &CSteamProto::OnGotCaptcha, mir_strdup(captchaId.c_str()));
		return;
	}

	// unhadled error
	DeleteAuthSettings();
	SetStatus(ID_STATUS_OFFLINE);
	ShowNotification(message);
}
*/

void CSteamProto::DeleteAuthSettings()
{
	delSetting("TwoFactorCode");
	delSetting("GuardId");
	delSetting("GuardCode");
	delSetting("CaptchaId");
	delSetting("CaptchaText");
}

bool CSteamProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_hServerConn != nullptr;
}

bool CSteamProto::IsMe(const char *steamId)
{
	ptrA mySteamId(getStringA(DBKEY_STEAM_ID));
	return mir_strcmp(steamId, mySteamId) == 0;
}

void CSteamProto::Login()
{
	CMsgClientHello hello;
	hello.protocol_version = STEAM_PROTOCOL_VERSION; hello.has_protocol_version = true;
	WSSend(EMsg::ClientHello, hello);

	ptrA username(getUStringA("Username"));
	if (username == NULL)
		LoginFailed();
	else {
		CAuthenticationGetPasswordRSAPublicKeyRequest request;
		request.account_name = username.get();
		WSSendService("Authentication.GetPasswordRSAPublicKey#1", request, &CSteamProto::OnGotRsaKey);
	}
}

void CSteamProto::LoginFailed()
{
	m_bTerminated = true;

	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iStatus);
}

void CSteamProto::Logout()
{
	m_bTerminated = true;

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iStatus);
}

void CSteamProto::OnGotHosts(const JSONNode &root, void*)
{
	db_delete_module(0, STEAM_MODULE);

	int i = 0;
	CMStringA szSetting;
	for (auto &it : root["response"]["serverlist_websockets"]) {
		szSetting.Format("Host%d", i++);
		db_set_ws(0, STEAM_MODULE, szSetting, it.as_mstring());
	}

	db_set_dw(0, STEAM_MODULE, DBKEY_HOSTS_COUNT, i);
	db_set_dw(0, STEAM_MODULE, DBKEY_HOSTS_DATE, time(0));
}

void CSteamProto::OnGotRsaKey(const uint8_t *buf, size_t cbLen)
{
	proto::AuthenticationGetPasswordRSAPublicKeyResponse reply(buf, cbLen);
	if (reply == nullptr || !reply->publickey_exp || !reply->publickey_mod) {
		LoginFailed();
		return;
	}

	// encrypt password
	ptrA szPassword(getStringA("Password"));

	MBinBuffer encPassword(RsaEncrypt(reply->publickey_mod, reply->publickey_exp, szPassword));
	ptrA base64RsaEncryptedPassword(mir_base64_encode(encPassword.data(), encPassword.length()));

	// run authorization request
	ptrA userName(getUStringA("Username"));
	T2Utf deviceName(m_wszDeviceName);
	
	CAuthenticationDeviceDetails details;
	details.device_friendly_name = deviceName.get();
	details.os_type = 16; details.has_os_type = true;
	details.platform_type = EAUTH_TOKEN_PLATFORM_TYPE__k_EAuthTokenPlatformType_SteamClient; details.has_platform_type = true;

	CAuthenticationBeginAuthSessionViaCredentialsRequest request;
	request.account_name = userName.get();
	request.website_id = "Client";
	request.encrypted_password = base64RsaEncryptedPassword;
	request.encryption_timestamp = reply->timestamp; request.has_encryption_timestamp = true;
	request.persistence = ESESSION_PERSISTENCE__k_ESessionPersistence_Persistent; request.has_persistence = true;
	request.remember_login = 0; request.has_remember_login = true;
	request.language = 1; request.has_language = true;
	request.qos_level = 2; request.has_qos_level = true;

	request.device_details = &details;
	request.device_friendly_name = details.device_friendly_name;
	request.platform_type = details.platform_type; request.has_platform_type = true;

	WSSendService("Authentication.BeginAuthSessionViaCredentials#1", request, &CSteamProto::OnAuthorization);
}

void CSteamProto::OnAuthorization(const uint8_t *buf, size_t cbLen)
{
	proto::AuthenticationBeginAuthSessionViaCredentialsResponse reply(buf, cbLen);
	if (reply == nullptr) {
		LoginFailed();
		return;
	}

	// Success
	if (reply->has_client_id && reply->has_steamid) {
		DeleteAuthSettings();
		SetId(DBKEY_STEAM_ID, reply->steamid);
		SetId(DBKEY_CLIENT_ID, reply->client_id);

		CAuthenticationPollAuthSessionStatusRequest request;
		request.client_id = reply->client_id; request.has_client_id = true;
		request.request_id = reply->request_id; request.has_request_id = true;
		WSSendService("Authentication.PollAuthSessionStatus#1", request, &CSteamProto::OnPollSession);
	}
	else {
		debugLogA("Something went wrong: %s", reply->extended_error_message);
		LoginFailed();
	}
}

void CSteamProto::OnPollSession(const uint8_t *buf, size_t cbLen)
{
	proto::AuthenticationPollAuthSessionStatusResponse reply(buf, cbLen);
	if (reply == nullptr || !reply->access_token || !reply->refresh_token) {
		LoginFailed();
		return;
	}

	m_szAccessToken = reply->access_token;
	m_szRefreshToken = reply->refresh_token;

	ptrA szAccountName(getUStringA(DBKEY_ACCOUNT_NAME));
	
	MBinBuffer machineId(getBlob(DBKEY_MACHINE_ID));
	if (!machineId.length()) {
		uint8_t random[100], hashOut[20];
		Utils_GetRandom(random, sizeof(random));
		mir_sha1_hash(random, sizeof(random), hashOut);

		db_set_blob(0, m_szModuleName, DBKEY_MACHINE_ID, hashOut, sizeof(hashOut));
		machineId.append(hashOut, sizeof(hashOut));
	}

	CMsgIPAddress privateIp;
	privateIp.ip_case = CMSG_IPADDRESS__IP_V4;
	privateIp.v4 = 0;

	CMsgClientLogon request;
	request.access_token = reply->access_token;
	request.account_name = szAccountName.get();
	request.client_language = "english";
	request.client_os_type = 16; request.has_client_os_type = true;
	request.should_remember_password = false; request.has_should_remember_password = true;
	request.obfuscated_private_ip = &privateIp;
	request.protocol_version = STEAM_PROTOCOL_VERSION; request.has_protocol_version = true;
	request.supports_rate_limit_response = request.has_supports_rate_limit_response = true;
	request.machine_name = "";
	request.steamguard_dont_remember_computer = false; request.has_steamguard_dont_remember_computer = true;
	request.chat_mode = 2; request.has_chat_mode = true;
	request.cell_id = 7; request.has_cell_id = true;
	request.machine_id.data = machineId.data();
	request.machine_id.len = machineId.length();
	WSSend(EMsg::ClientLogon, request);
}

void CSteamProto::OnLoggedOn(const uint8_t *buf, size_t cbLen)
{
	proto::MsgClientLogonResponse reply(buf, cbLen);
	if (reply == nullptr || !reply->has_eresult) {
		LoginFailed();
		return;
	}

	if (reply->eresult != 1) {
		debugLogA("Login failed with error %d", reply->eresult);
		LoginFailed();
		return;
	}

	// go to online now
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);

	// load contact list
	// SendRequest(new GetFriendListRequest(token, steamId, "friend,ignoredfriend,requestrecipient"), &CSteamProto::OnGotFriendList);
}
