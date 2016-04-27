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

	if (LoadToxProfile(options))
	{
		return options;
	}
	
	tox_options_free(options);
	
	return NULL;
}

bool CToxProto::InitToxCore(CToxThread *toxThread)
{
	logger->Log(__FUNCTION__": initializing tox core");

	if (toxThread == NULL)
		return false;

	/*Tox_Options *options = GetToxOptions();
	if (options == NULL)
		return false;

	if (LoadToxProfile(options))
	{
		TOX_ERR_NEW initError;
		toxThread->Tox() = tox_new(options, &initError);
		if (initError != TOX_ERR_NEW_OK)
		{
			logger->Log(__FUNCTION__": failed to initialize tox core (%d)", initError);
			ShowNotification(ToxErrorToString(initError), TranslateT("Unable to initialize Tox core"), MB_ICONERROR);
			tox_options_free(options);
			return false;
		}*/

		tox_callback_friend_request(toxThread->Tox(), OnFriendRequest, this);
		tox_callback_friend_message(toxThread->Tox(), OnFriendMessage, this);
		tox_callback_friend_read_receipt(toxThread->Tox(), OnReadReceipt, this);
		tox_callback_friend_typing(toxThread->Tox(), OnTypingChanged, this);
		//
		tox_callback_friend_name(toxThread->Tox(), OnFriendNameChange, this);
		tox_callback_friend_status_message(toxThread->Tox(), OnStatusMessageChanged, this);
		tox_callback_friend_status(toxThread->Tox(), OnUserStatusChanged, this);
		tox_callback_friend_connection_status(toxThread->Tox(), OnConnectionStatusChanged, this);
		// transfers
		tox_callback_file_recv_control(toxThread->Tox(), OnFileRequest, this);
		tox_callback_file_recv(toxThread->Tox(), OnFriendFile, this);
		tox_callback_file_recv_chunk(toxThread->Tox(), OnDataReceiving, this);
		tox_callback_file_chunk_request(toxThread->Tox(), OnFileSendData, this);
		// group chats
		//tox_callback_group_invite(tox, OnGroupChatInvite, this);
		// a/v
		//if (IsWinVerVistaPlus())
		//{
		//	TOXAV_ERR_NEW avInitError;
		//	toxThread->Tox()AV = toxav_new(toxThread->Tox(), &avInitError);
		//	if (initError != TOX_ERR_NEW_OK)
		//	{
		//		toxav_callback_call(toxThread->Tox()AV, OnFriendCall, this);
		//		toxav_callback_call_state(toxThread->Tox()AV, OnFriendCallState, this);
		//		toxav_callback_bit_rate_status(toxThread->Tox()AV, OnBitrateChanged, this);
		//		toxav_callback_audio_receive_frame(toxThread->Tox()AV, OnFriendAudioFrame, this);
		//		//toxav_callback_video_receive_frame(toxThread->Tox()AV, , this);
		//	}
		//}

		uint8_t data[TOX_ADDRESS_SIZE];
		tox_self_get_address(toxThread->Tox(), data);
		ToxHexAddress address(data);
		setString(TOX_SETTINGS_ID, address);

		uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
		tox_self_get_name(toxThread->Tox(), nick);
		setTString("Nick", ptrT(Utf8DecodeT((char*)nick)));

		uint8_t statusMessage[TOX_MAX_STATUS_MESSAGE_LENGTH] = { 0 };
		tox_self_get_status_message(toxThread->Tox(), statusMessage);
		setTString("StatusMsg", ptrT(Utf8DecodeT((char*)statusMessage)));

		return true;
	/*}

	tox_options_free(options);

	return false;*/
}

void CToxProto::UninitToxCore(CToxThread *toxThread)
{
	if (toxThread == NULL)
		return;

	/*if (toxThread->ToxAV())
		toxav_kill(toxThread->ToxAV());

	if (toxThread->Tox() == NULL)
		return;*/
	
	CancelAllTransfers();
	SaveToxProfile(toxThread);

	//tox_kill(toxThread->Tox());
	//toxThread->Tox() = NULL;
}