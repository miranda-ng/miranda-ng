#include "common.h"

bool CToxProto::InitToxCore()
{
	debugLogA(__FUNCTION__": initializing tox core");

	TOX_ERR_OPTIONS_NEW error;
	Tox_Options *options = tox_options_new(&error);
	if (error != TOX_ERR_OPTIONS_NEW::TOX_ERR_OPTIONS_NEW_OK)
	{
		debugLogA(__FUNCTION__": failed to initialize tox options (%d)", error);
		return false;
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
				debugLogA("CToxProto::InitToxCore: setting http user proxy config");
				options->proxy_type = TOX_PROXY_TYPE_HTTP;
				mir_strcpy((char*)&options->proxy_host[0], nlus.szProxyServer);
				options->proxy_port = nlus.wProxyPort;
			}

			if (nlus.proxyType == PROXYTYPE_SOCKS4 || nlus.proxyType == PROXYTYPE_SOCKS5)
			{
				debugLogA("CToxProto::InitToxCore: setting socks user proxy config");
				options->proxy_type = TOX_PROXY_TYPE_SOCKS5;
				mir_strcpy((char*)&options->proxy_host[0], nlus.szProxyServer);
				options->proxy_port = nlus.wProxyPort;
			}
		}
	}

	debugLogA("CToxProto::InitToxCore: loading tox profile");

	if (LoadToxProfile(options))
	{
		tox_callback_friend_request(tox, OnFriendRequest, this);
		tox_callback_friend_message(tox, OnFriendMessage, this);
		tox_callback_friend_read_receipt(tox, OnReadReceipt, this);
		tox_callback_friend_typing(tox, OnTypingChanged, this);
		tox_callback_friend_name(tox, OnFriendNameChange, this);
		tox_callback_friend_status_message(tox, OnStatusMessageChanged, this);
		tox_callback_friend_status(tox, OnUserStatusChanged, this);
		tox_callback_friend_connection_status(tox, OnConnectionStatusChanged, this);
		// file transfers
		tox_callback_file_recv_control(tox, OnFileRequest, this);
		tox_callback_file_recv(tox, OnFriendFile, this);
		tox_callback_file_recv_chunk(tox, OnFileReceiveData, this);
		tox_callback_file_chunk_request(tox, OnFileSendData, this);
		// group chats
		//tox_callback_group_invite(tox, OnGroupChatInvite, this);

		uint8_t data[TOX_ADDRESS_SIZE];
		tox_self_get_address(tox, data);
		ToxHexAddress address(data, TOX_ADDRESS_SIZE);
		setString(TOX_SETTINGS_ID, address);

		uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
		tox_self_get_name(tox, nick);
		setWString("Nick", ptrW(Utf8DecodeW((char*)nick)));

		uint8_t statusMessage[TOX_MAX_STATUS_MESSAGE_LENGTH] = { 0 };
		tox_self_get_status_message(tox, statusMessage);
		setWString("StatusMsg", ptrW(Utf8DecodeW((char*)statusMessage)));

		return true;
	}

	tox_options_free(options);

	return false;
}

void CToxProto::UninitToxCore()
{
	if (tox) {
		for (size_t i = 0; i < transfers.Count(); i++)
		{
			FileTransferParam *transfer = transfers.GetAt(i);
			transfer->status = CANCELED;
			tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
			ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
			transfers.Remove(transfer);
		}

		//if (IsToxCoreInited())
		//{
		//	ptrA nickname(mir_utf8encodeW(ptrT(getTStringA("Nick"))));
		//	tox_set_name(tox, (uint8_t*)(char*)nickname, mir_strlen(nickname));

		//	//temporary
		//	ptrA statusmes(mir_utf8encodeW(ptrT(getTStringA("StatusMsg"))));
		//	tox_set_status_message(tox, (uint8_t*)(char*)statusmes, mir_strlen(statusmes));
		//}

		SaveToxProfile();
		if (password != NULL)
		{
			mir_free(password);
			password = NULL;
		}
		tox_kill(tox);
		tox = NULL;
	}
}