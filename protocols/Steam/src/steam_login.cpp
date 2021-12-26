#include "stdafx.h"

bool CSteamProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_hPollingThread;
}

bool CSteamProto::IsMe(const char *steamId)
{
	ptrA mySteamId(getStringA("SteamID"));
	return mir_strcmp(steamId, mySteamId) == 0;
}

void CSteamProto::Login()
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	if (mir_strlen(token) > 0 && mir_strlen(sessionId) > 0) {
		PushRequest(new LogonRequest(token), &CSteamProto::OnLoggedOn);
		return;
	}

	T2Utf username(getWStringA("Username"));
	if (username == NULL)
		SetStatus(ID_STATUS_OFFLINE);
	else
		PushRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
}

void CSteamProto::Logout()
{
	m_isTerminated = true;
	if (m_hRequestQueueThread)
		SetEvent(m_hRequestsQueueEvent);

	ptrA token(getStringA("TokenSecret"));
	if (mir_strlen(token) > 0) {
		ptrA umqid(getStringA("UMQID"));
		SendRequest(new LogoffRequest(token, umqid));
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iStatus);
}

void CSteamProto::OnGotRsaKey(const JSONNode &root, void *)
{
	if (root.isnull()) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!root["success"].as_bool()) {
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
	if ((error = RsaEncrypt(modulus.c_str(), exponent, szPassword, nullptr, encryptedSize)) != 0) {
		debugLogA(__FUNCTION__ ": encryption error (%lu)", error);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	uint8_t *encryptedPassword = (uint8_t *)mir_calloc(encryptedSize);
	if ((error = RsaEncrypt(modulus.c_str(), exponent, szPassword, encryptedPassword, encryptedSize)) != 0) {
		debugLogA(__FUNCTION__ ": encryption error (%lu)", error);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);

	// run authorization request
	T2Utf username(getWStringA("Username"));

	ptrA twoFactorCode(getStringA("TwoFactorCode"));
	if (!twoFactorCode)
		twoFactorCode = mir_strdup("");

	ptrA guardId(getStringA("GuardId"));
	if (!guardId)
		guardId = mir_strdup("");
	ptrA guardCode(getStringA("GuardCode"));
	if (!guardCode)
		guardCode = mir_strdup("");

	ptrA captchaId(getStringA("CaptchaId"));
	if (!captchaId)
		captchaId = mir_strdup("-1");
	ptrA captchaText(getStringA("CaptchaText"));
	if (!captchaText)
		captchaText = mir_strdup("");

	PushRequest(
		new AuthorizationRequest(username, base64RsaEncryptedPassword, timestamp.c_str(), twoFactorCode, guardCode, guardId, captchaId, captchaText),
		&CSteamProto::OnAuthorization);
}

void CSteamProto::OnGotCaptcha(const HttpResponse &response, void *arg)
{
	ptrA captchaId((char *)arg);

	if (!response.IsSuccess()) {
		debugLogA(__FUNCTION__ ": failed to get captcha");
		return;
	}

	CSteamCaptchaDialog captchaDialog(this, response.Content, response.Content.size());
	if (!captchaDialog.DoModal()) {
		DeleteAuthSettings();
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	setString("CaptchaId", captchaId);
	setString("CaptchaText", captchaDialog.GetCaptchaText());

	T2Utf username(getWStringA("Username"));
	PushRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
}

void CSteamProto::OnAuthorization(const HttpResponse &response, void *)
{
	if (!response) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull()) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!root["success"].as_bool()) {
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
		return;
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

		PushRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
		return;
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

		PushRequest(new GetRsaKeyRequest(username), &CSteamProto::OnGotRsaKey);
		return;
	}

	if (root["captcha_needed"].as_bool()) {
		debugLogA(__FUNCTION__ ": captcha needed");
		delSetting("CaptchaId");
		delSetting("CaptchaText");
		json_string captchaId = root["captcha_gid"].as_string();
		PushRequest(new GetCaptchaRequest(captchaId.c_str()), &CSteamProto::OnGotCaptcha, mir_strdup(captchaId.c_str()));
		return;
	}

	// unhadled error
	DeleteAuthSettings();
	SetStatus(ID_STATUS_OFFLINE);
	ShowNotification(message);
}

void CSteamProto::OnAuthorizationSuccess(const JSONNode &root)
{
	DeleteAuthSettings();

	if (!root["login_complete"].as_bool()) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	json_string oauth = root["oauth"].as_string();
	JSONNode node = JSONNode::parse(oauth.c_str());
	if (node.isnull()) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	json_string steamId = node["steamid"].as_string();
	setString("SteamID", steamId.c_str());

	json_string token = node["oauth_token"].as_string();
	setString("TokenSecret", token.c_str());

	SendRequest(new GetSessionRequest2(token.c_str(), steamId.c_str()), &CSteamProto::OnGotSession);

	PushRequest(new LogonRequest(token.c_str()), &CSteamProto::OnLoggedOn);
}

void CSteamProto::OnGotSession(const HttpResponse &response, void *)
{
	if (!response) {
		debugLogA(__FUNCTION__ ": failed to get session id");
		return;
	}

	for (auto &header : response.Headers) {
		if (mir_strcmpi(header.szName, "Set-Cookie"))
			continue;

		std::string cookies = header.szValue;
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
		ShowNotification(TranslateT("Cannot obtain connection token."));
		// Just go offline; it also resets the isLoginAgain to false
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	// Remember we are trying to relogin
	isLoginAgain = true;

	Login();
}

void CSteamProto::OnLoggedOn(const HttpResponse &response, void *)
{
	if (response.GetStatusCode() == HTTP_CODE_UNAUTHORIZED) {
		// Probably expired TokenSecret
		SetStatus(ID_STATUS_OFFLINE);
		HandleTokenExpired();
		return;
	}

	if (!response.IsSuccess()) {
		// Probably timeout or no connection, we can do nothing here
		debugLogA(__FUNCTION__ ": unknown login error");
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	json_string error = root["error"].as_string();
	if (error != "OK") {
		// Probably expired TokenSecret
		HandleTokenExpired();
		return;
	}

	json_string umqId = root["umqid"].as_string();
	setString("UMQID", umqId.c_str());

	long messageId = root["umqid"].as_int();
	setDword("MessageID", messageId);

	// load contact list
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));

	SendRequest(new GetSessionRequest2(token, steamId), &CSteamProto::OnGotSession);

	// send this request immediately, so we can start polling thread with already loaded all contacts
	SendRequest(new GetFriendListRequest(token, steamId, "friend,ignoredfriend,requestrecipient"), &CSteamProto::OnGotFriendList);

	// go to online now
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);

	// start polling thread
	m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, nullptr, nullptr);
}

void CSteamProto::OnReLogin(const JSONNode &root, void*)
{
	if (root.isnull()) {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	json_string error = root["error"].as_string();
	if (error != "OK") {
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	json_string umqId = root["umqid"].as_string();
	setString("UMQID", umqId.c_str());

	long messageId = root["message"].as_int();
	setDword("MessageID", messageId);
}
