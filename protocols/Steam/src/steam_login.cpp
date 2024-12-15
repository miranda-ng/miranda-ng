#include "stdafx.h"

void CSteamProto::DeleteAuthSettings()
{
	m_requestId = MBinBuffer();
	delSetting("TwoFactorCode");
	delSetting("GuardId");
	delSetting("GuardCode");
	delSetting("CaptchaId");
	delSetting("CaptchaText");
}

bool CSteamProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_ws != nullptr;
}

bool CSteamProto::IsMe(const char *steamId)
{
	return m_iSteamId == _atoi64(steamId);
}

void CSteamProto::Logout()
{
	m_bTerminated = true;

	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
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

/////////////////////////////////////////////////////////////////////////////////////////
// Restore the saved session or establish a new one

void CSteamProto::Login()
{
	m_iSessionId = 0;

	CMsgClientHello hello;
	hello.protocol_version = STEAM_PROTOCOL_VERSION; hello.has_protocol_version = true;
	WSSend(EMsg::ClientHello, hello);

	ptrA username(getUStringA("Username"));
	if (username == NULL) {
		Logout();
		return;
	}

	CAuthenticationGetPasswordRSAPublicKeyRequest request;
	request.account_name = username.get();
	WSSendService(GetPasswordRSAPublicKey, request, true);
}

void CSteamProto::OnGotRsaKey(const CAuthenticationGetPasswordRSAPublicKeyResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed()) {
		Logout();
		return;
	}

	// encrypt password
	ptrA szPassword(getStringA("Password"));

	MBinBuffer encPassword(RsaEncrypt(reply.publickey_mod, reply.publickey_exp, szPassword));
	ptrA base64RsaEncryptedPassword(mir_base64_encode(encPassword.data(), encPassword.length()));

	// run authorization request
	ptrA userName(getUStringA("Username"));
	ptrA machineId(getUStringA("MachineId"));
	T2Utf deviceName(m_wszDeviceName);
	
	CAuthenticationDeviceDetails details;
	details.device_friendly_name = deviceName.get();
	details.os_type = 16; details.has_os_type = true;
	details.platform_type = EAUTH_TOKEN_PLATFORM_TYPE__k_EAuthTokenPlatformType_SteamClient; details.has_platform_type = true;

	CAuthenticationBeginAuthSessionViaCredentialsRequest request;
	request.account_name = userName.get();
	request.website_id = "Client";
	request.encrypted_password = base64RsaEncryptedPassword;
	request.encryption_timestamp = reply.timestamp; request.has_encryption_timestamp = true;
	request.persistence = ESESSION_PERSISTENCE__k_ESessionPersistence_Persistent; request.has_persistence = true;
	request.remember_login = request.has_remember_login = true;
	request.language = 1; request.has_language = true;
	request.qos_level = 2; request.has_qos_level = true;

	request.device_details = &details;
	request.device_friendly_name = details.device_friendly_name;
	request.platform_type = details.platform_type; request.has_platform_type = true;
	request.guard_data = machineId;

	WSSendService(BeginAuthSessionViaCredentials, request, true);
}

void CSteamProto::OnBeginSession(const CAuthenticationBeginAuthSessionViaCredentialsResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed()) {
		Logout();
		return;
	}

	if (reply.has_client_id && reply.has_steamid) {
		DeleteAuthSettings();
		SetId(DBKEY_STEAM_ID, m_iSteamId = reply.steamid);
		SetId(DBKEY_CLIENT_ID, m_iClientId = reply.client_id);

		if (reply.has_request_id)
			m_requestId.append(reply.request_id.data, reply.request_id.len);

		for (int i = 0; i < reply.n_allowed_confirmations; i++) {
			auto &conf = reply.allowed_confirmations[i];
			debugLogA("Confirmation required %d (%s)", conf->confirmation_type, conf->associated_message);
			switch (conf->confirmation_type) {
			case EAUTH_SESSION_GUARD_TYPE__k_EAuthSessionGuardType_None: // nothing to do
				SendPollRequest();
				return;

			case EAUTH_SESSION_GUARD_TYPE__k_EAuthSessionGuardType_EmailCode: // email confirmation
				CallFunctionSync(EnterEmailCode, this);
				return;

			case EAUTH_SESSION_GUARD_TYPE__k_EAuthSessionGuardType_DeviceCode: // totp confirmation
				CallFunctionSync(EnterTotpCode, this);
				return;
			}

			debugLogA("Unsupported confirmation code: %i", conf->confirmation_type);
			Logout();
		}

		// no confirmation needed - we've done
		SendPollRequest();
	}
	else {
		debugLogA("Something went wrong: %s", reply.extended_error_message);
		Logout();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CSteamProto::EnterEmailCode(void *param)
{
	auto *ppro = (CSteamProto *)param;

	ENTER_STRING es = {};
	es.szModuleName = ppro->m_szModuleName;
	es.caption = TranslateT("Enter email confimation code");
	if (EnterString(&es)) {
		ppro->SendConfirmationCode(true, T2Utf(es.ptszResult));
		mir_free(es.ptszResult);
	}
	else ppro->Logout();
	return 0;
}

INT_PTR CALLBACK CSteamProto::EnterTotpCode(void *param)
{
	auto *ppro = (CSteamProto *)param;

	ENTER_STRING es = {};
	es.szModuleName = ppro->m_szModuleName;
	es.caption = TranslateT("Enter the code from your authentication device");
	if (EnterString(&es)) {
		ppro->SendConfirmationCode(false, T2Utf(es.ptszResult));
		mir_free(es.ptszResult);
	}
	else ppro->Logout();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendConfirmationCode(bool isEmail, const char *pszCode)
{
	CAuthenticationUpdateAuthSessionWithSteamGuardCodeRequest request;
	request.steamid = GetId(DBKEY_STEAM_ID); request.has_steamid = true;
	request.client_id = GetId(DBKEY_CLIENT_ID); request.has_client_id = true;
	if (isEmail)
		request.code_type = EAUTH_SESSION_GUARD_TYPE__k_EAuthSessionGuardType_EmailCode;
	else
		request.code_type = EAUTH_SESSION_GUARD_TYPE__k_EAuthSessionGuardType_DeviceCode;
	request.has_code_type = true;
	request.code = (char*)pszCode;
	WSSendService(UpdateAuthSessionWithSteamGuardCode, request, true);
}

void CSteamProto::OnGotConfirmationCode(const CAuthenticationUpdateAuthSessionWithSteamGuardCodeResponse&, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		Logout();
	else
		SendPollRequest();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendPollRequest()
{
	CAuthenticationPollAuthSessionStatusRequest request;
	request.client_id = GetId(DBKEY_CLIENT_ID); request.has_client_id = true;
	request.request_id.data = m_requestId.data(); request.request_id.len = m_requestId.length(); request.has_request_id = true;
	WSSendService(PollAuthSessionStatus, request, true);
}

void CSteamProto::OnPollSession(const CAuthenticationPollAuthSessionStatusResponse &reply, const CMsgProtoBufHeader &)
{
	if (reply.has_new_client_id)
		m_iClientId = reply.new_client_id;

	if (reply.new_guard_data)
		setString("MachineId", reply.new_guard_data);

	m_szAccessToken = reply.access_token;
	m_szRefreshToken = reply.refresh_token;

	// sending logon packet
	ptrA szAccountName(getUStringA(DBKEY_ACCOUNT_NAME)), szPassword(getUStringA("Password"));
	T2Utf szMachineName(m_wszDeviceName);
	MBinBuffer machineId(createMachineID(szAccountName));

	CMsgIPAddress privateIp;
	privateIp.ip_case = CMSG_IPADDRESS__IP_V4;
	privateIp.v4 = 0;

	CMsgClientLogon request;
	request.access_token = reply.refresh_token;
	request.machine_name = szMachineName;
	request.client_language = "english";
	request.client_os_type = 16; request.has_client_os_type = true;
	request.should_remember_password = request.has_should_remember_password = true;
	request.obfuscated_private_ip = &privateIp;
	request.protocol_version = STEAM_PROTOCOL_VERSION; request.has_protocol_version = true;
	request.supports_rate_limit_response = request.has_supports_rate_limit_response = true;
	request.steamguard_dont_remember_computer = false; request.has_steamguard_dont_remember_computer = true;
	request.chat_mode = 2; request.has_chat_mode = true;
	request.cell_id = 7; request.has_cell_id = true;
	request.machine_id.data = machineId.data(); request.machine_id.len = machineId.length();
	WSSend(EMsg::ClientLogon, request);
}

void CSteamProto::OnClientLogon(const CMsgClientLogonResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed()) {
		Logout();
		return;
	}

	if (reply.has_heartbeat_seconds)
		m_impl.m_heartBeat.Start(reply.heartbeat_seconds * 1000);

	// go to online now
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);
}

void CSteamProto::OnClientLogoff(const CMsgClientLoggedOff &reply, const CMsgProtoBufHeader&)
{
	debugLogA("received logout request with error code %d", reply.has_eresult ? reply.eresult : 0);
	Logout();
}
