#include "stdafx.h"

Tox_Options* CToxProto::GetToxOptions()
{
	TOX_ERR_OPTIONS_NEW error;
	Tox_Options *options = tox_options_new(&error);
	if (error != TOX_ERR_OPTIONS_NEW_OK) {
		debugLogA(__FUNCTION__": failed to initialize tox options (%d)", error);
		return nullptr;
	}

	options->udp_enabled = getBool("EnableUDP", 1);
	if (options->udp_enabled && getBool("EnableUDPHolePunching", 1))
		options->hole_punching_enabled = true;
	options->ipv6_enabled = getBool("EnableIPv6", 0);
	options->local_discovery_enabled = getBool("EnableLocalDiscovery", 0);

	if (m_hNetlibUser != nullptr) {
		NETLIBUSERSETTINGS nlus = { sizeof(nlus) };
		Netlib_GetUserSettings(m_hNetlibUser, &nlus);

		if (nlus.useProxy) {
			if (nlus.proxyType == PROXYTYPE_HTTP || nlus.proxyType == PROXYTYPE_HTTPS) {
				debugLogA(__FUNCTION__": setting http user proxy config");
				options->proxy_type = TOX_PROXY_TYPE_HTTP;
				options->proxy_host = nlus.szProxyServer;
				options->proxy_port = nlus.wProxyPort;
			}

			if (nlus.proxyType == PROXYTYPE_SOCKS4 || nlus.proxyType == PROXYTYPE_SOCKS5) {
				debugLogA(__FUNCTION__": setting socks user proxy config");
				options->proxy_type = TOX_PROXY_TYPE_SOCKS5;
				options->proxy_host = nlus.szProxyServer;
				options->proxy_port = nlus.wProxyPort;
			}
		}
	}

	options->log_callback = CToxProto::OnToxLog;
	options->log_user_data = this;

	if (LoadToxProfile(options))
		return options;

	tox_options_free(options);
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CToxProto::InitThread(void *)
{
	Tox_Options *options = GetToxOptions();

	TOX_ERR_NEW error;
	m_tox = tox_new(options, &error);
	tox_options_free(options);
	if (error != TOX_ERR_NEW_OK) {
		debugLogA(__FUNCTION__": failed to initialize tox core (%d)", error);
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr);
		ShowNotification(TranslateT("Unable to initialize Tox core"), ToxErrorToString(error), MB_ICONERROR);
		return;
	}

	InitToxCore(m_tox);
	BootstrapNodes(m_tox);

	m_impl.timerPoll.StartSafe(TOX_DEFAULT_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CToxProto::InitToxCore(Tox *tox)
{
	debugLogA(__FUNCTION__": initializing tox core");

	if (tox == nullptr)
		return;

	tox_callback_self_connection_status(tox, OnConnectionStatus);

	tox_callback_friend_request(tox, OnFriendRequest);
	tox_callback_friend_message(tox, OnFriendMessage);
	tox_callback_friend_read_receipt(tox, OnReadReceipt);
	tox_callback_friend_typing(tox, OnTypingChanged);
	
	tox_callback_friend_name(tox, OnFriendNameChange);
	tox_callback_friend_status_message(tox, OnStatusMessageChanged);
	tox_callback_friend_status(tox, OnUserStatusChanged);
	tox_callback_friend_connection_status(tox, OnConnectionStatusChanged);

	// transfers
	tox_callback_file_recv_control(tox, OnFileRequest);
	tox_callback_file_recv(tox, OnFriendFile);
	tox_callback_file_recv_chunk(tox, OnDataReceiving);
	tox_callback_file_chunk_request(tox, OnFileSendData);

	uint8_t data[TOX_ADDRESS_SIZE];
	tox_self_get_address(tox, data);
	ToxHexAddress address(data);
	setString(TOX_SETTINGS_ID, address);

	TOX_ERR_SET_INFO setInfoError;

	/*uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
	tox_self_get_name(toxThread->Tox(), nick);
	setWString("Nick", ptrW(mir_utf8decodeW((char*)nick)));*/

	ptrA nick(mir_utf8encodeW(ptrW(getWStringA("Nick"))));
	tox_self_set_name(tox, (uint8_t*)(char*)nick, mir_strlen(nick), &setInfoError);
	if (setInfoError != TOX_ERR_SET_INFO_OK)
		debugLogA(__FUNCTION__": failed to set self name (%d)", setInfoError);

	/*uint8_t statusMessage[TOX_MAX_STATUS_MESSAGE_LENGTH] = { 0 };
	tox_self_get_status_message(toxThread->Tox(), statusMessage);
	setWString("StatusMsg", ptrW(mir_utf8decodeW((char*)statusMessage)));*/

	ptrA statusMessage(mir_utf8encodeW(ptrW(getWStringA("StatusMsg"))));
	tox_self_set_status_message(tox, (uint8_t*)(char*)statusMessage, mir_strlen(statusMessage), &setInfoError);
	if (setInfoError != TOX_ERR_SET_INFO_OK)
		debugLogA(__FUNCTION__": failed to set self status message (%d)", setInfoError);
}

void CToxProto::UninitToxCore(Tox *tox)
{
	CancelAllTransfers(tox);
	SaveToxProfile(tox);
}

void CToxProto::OnToxLog(Tox*, TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func, const char *message, void *user_data)
{
	CToxProto *proto = (CToxProto*)user_data;

	if (level == TOX_LOG_LEVEL_ERROR)
		proto->debugLogA("TOXCORE: %s at %s(...) in %s:%u", message, func, file, line);
}
