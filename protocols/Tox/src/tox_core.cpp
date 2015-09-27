#include "stdafx.h"

Tox_Options* CToxProto::GetToxOptions()
{
	TOX_ERR_OPTIONS_NEW error;
	Tox_Options *options = tox_options_new(&error);
	if (error != TOX_ERR_OPTIONS_NEW_OK)
	{
		logger->Log(__FUNCTION__": failed to initialize tox options (%d)", error);
		return NULL;
	}

	options->udp_enabled = getBool("EnableUDP", 1);
	options->ipv6_enabled = getBool("EnableIPv6", 0);

	if (hNetlib != NULL)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)hNetlib, (LPARAM)&nlus);

		if (nlus.useProxy)
		{
			if (nlus.proxyType == PROXYTYPE_HTTP || nlus.proxyType == PROXYTYPE_HTTPS)
			{
				logger->Log("CToxProto::InitToxCore: setting http user proxy config");
				options->proxy_type = TOX_PROXY_TYPE_HTTP;
				mir_strcpy((char*)&options->proxy_host[0], nlus.szProxyServer);
				options->proxy_port = nlus.wProxyPort;
			}

			if (nlus.proxyType == PROXYTYPE_SOCKS4 || nlus.proxyType == PROXYTYPE_SOCKS5)
			{
				logger->Log(__FUNCTION__": setting socks user proxy config");
				options->proxy_type = TOX_PROXY_TYPE_SOCKS5;
				mir_strcpy((char*)&options->proxy_host[0], nlus.szProxyServer);
				options->proxy_port = nlus.wProxyPort;
			}
		}
	}
	
	return options;
}

bool CToxProto::InitToxCore()
{
	logger->Log(__FUNCTION__": initializing tox core");

	Tox_Options *options = GetToxOptions();
	if (options == NULL)
		return false;

	if (LoadToxProfile(options))
	{
		if (toxThread == NULL) {
			tox_options_free(options);
			return false;
		}

		TOX_ERR_NEW initError;
		toxThread->tox = tox_new(options, &initError);
		if (initError != TOX_ERR_NEW_OK)
		{
			logger->Log(__FUNCTION__": failed to initialize tox core (%d)", initError);
			ShowNotification(ToxErrorToString(initError), TranslateT("Unable to initialize Tox core"), MB_ICONERROR);
			tox_options_free(options);
			return false;
		}

		tox_callback_friend_request(toxThread->tox, OnFriendRequest, this);
		tox_callback_friend_message(toxThread->tox, OnFriendMessage, this);
		tox_callback_friend_read_receipt(toxThread->tox, OnReadReceipt, this);
		tox_callback_friend_typing(toxThread->tox, OnTypingChanged, this);
		//
		tox_callback_friend_name(toxThread->tox, OnFriendNameChange, this);
		tox_callback_friend_status_message(toxThread->tox, OnStatusMessageChanged, this);
		tox_callback_friend_status(toxThread->tox, OnUserStatusChanged, this);
		tox_callback_friend_connection_status(toxThread->tox, OnConnectionStatusChanged, this);
		// transfers
		tox_callback_file_recv_control(toxThread->tox, OnFileRequest, this);
		tox_callback_file_recv(toxThread->tox, OnFriendFile, this);
		tox_callback_file_recv_chunk(toxThread->tox, OnDataReceiving, this);
		tox_callback_file_chunk_request(toxThread->tox, OnFileSendData, this);
		// group chats
		//tox_callback_group_invite(tox, OnGroupChatInvite, this);
		// a/v
		if (IsWinVerVistaPlus())
		{
			/*toxAv = toxav_new(tox, TOX_MAX_CALLS);
			toxav_register_audio_callback(toxThread->toxAv, OnFriendAudio, this);
			toxav_register_callstate_callbacktox(Thread->toxAv, OnAvInvite, av_OnInvite, this);
			toxav_register_callstate_callbacktox(Thread->toxAv, OnAvStart, av_OnStart, this);
			toxav_register_callstate_callbacktox(Thread->toxAv, OnAvCancel, av_OnCancel, this);
			toxav_register_callstate_callbacktox(Thread->toxAv, OnAvReject, av_OnReject, this);
			toxav_register_callstate_callback(toxThread->toxAv, OnAvEnd, av_OnEnd, this);
			toxav_register_callstate_callback(toxThread->toxAv, OnAvCallTimeout, av_OnRequestTimeout, this);
			toxav_register_callstate_callback(toxThread->toxAv, OnAvPeerTimeout, av_OnPeerTimeout, this);*/
		}

		uint8_t data[TOX_ADDRESS_SIZE];
		tox_self_get_address(toxThread->tox, data);
		ToxHexAddress address(data);
		setString(TOX_SETTINGS_ID, address);

		uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
		tox_self_get_name(toxThread->tox, nick);
		setTString("Nick", ptrT(Utf8DecodeT((char*)nick)));

		uint8_t statusMessage[TOX_MAX_STATUS_MESSAGE_LENGTH] = { 0 };
		tox_self_get_status_message(toxThread->tox, statusMessage);
		setTString("StatusMsg", ptrT(Utf8DecodeT((char*)statusMessage)));

		return true;
	}

	tox_options_free(options);

	return false;
}

void CToxProto::UninitToxCore()
{
	if (toxThread) {
		if (toxThread->toxAv)
			toxav_kill(toxThread->toxAv);

		if (toxThread->tox)
		{
			CancelAllTransfers();

			SaveToxProfile();

			tox_kill(toxThread->tox);
		}
		toxThread = NULL;
	}
}