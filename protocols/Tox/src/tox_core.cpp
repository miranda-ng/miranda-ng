#include "common.h"

bool CToxProto::IsToxCoreInited()
{
	return tox != NULL;
}

bool CToxProto::InitToxCore()
{
	debugLogA("CToxProto::InitToxCore: initializing tox core");

	Tox_Options options = { 0 };
	options.udp_disabled = getBool("DisableUDP", 0);
	options.ipv6enabled = !getBool("DisableIPv6", 0);

	if (hNetlib != NULL)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)hNetlib, (LPARAM)&nlus);

		if (nlus.useProxy)
		{
			if (nlus.proxyType == PROXYTYPE_HTTP || nlus.proxyType == PROXYTYPE_HTTPS)
			{
				debugLogA("CToxProto::InitToxCore: setting http user proxy config");
				options.proxy_type = TOX_PROXY_HTTP;
				strcpy(&options.proxy_address[0], nlus.szProxyServer);
				options.proxy_port = nlus.wProxyPort;
			}

			if (nlus.proxyType == PROXYTYPE_SOCKS4 || nlus.proxyType == PROXYTYPE_SOCKS5)
			{
				debugLogA("CToxProto::InitToxCore: setting socks user proxy config");
				options.proxy_type = TOX_PROXY_SOCKS5;
				strcpy(&options.proxy_address[0], nlus.szProxyServer);
				options.proxy_port = nlus.wProxyPort;
			}
		}
	}

	debugLogA("CToxProto::InitToxCore: loading tox profile");

	tox = tox_new(&options);
	password = mir_utf8encodeW(ptrT(getTStringA("Password")));
	bool isProfileLoaded = LoadToxProfile();
	if (isProfileLoaded)
	{
		debugLogA("CToxProto::InitToxCore: tox profile load successfully");

		tox_callback_friend_request(tox, OnFriendRequest, this);
		tox_callback_friend_message(tox, OnFriendMessage, this);
		tox_callback_friend_action(tox, OnFriendAction, this);
		tox_callback_typing_change(tox, OnTypingChanged, this);
		tox_callback_name_change(tox, OnFriendNameChange, this);
		tox_callback_status_message(tox, OnStatusMessageChanged, this);
		tox_callback_user_status(tox, OnUserStatusChanged, this);
		tox_callback_read_receipt(tox, OnReadReceipt, this);
		tox_callback_connection_status(tox, OnConnectionStatusChanged, this);
		// file transfers
		tox_callback_file_control(tox, OnFileRequest, this);
		tox_callback_file_send_request(tox, OnFriendFile, this);
		tox_callback_file_data(tox, OnFileData, this);
		// avatars
		tox_callback_avatar_info(tox, OnGotFriendAvatarInfo, this);
		tox_callback_avatar_data(tox, OnGotFriendAvatarData, this);
		// group chats
		tox_callback_group_invite(tox, OnGroupChatInvite, this);

		uint8_t data[TOX_FRIEND_ADDRESS_SIZE];
		tox_get_address(tox, data);
		ToxHexAddress address(data, TOX_FRIEND_ADDRESS_SIZE);
		setString(TOX_SETTINGS_ID, address);

		int size = tox_get_self_name_size(tox);
		std::string nick(size, 0);
		tox_get_self_name(tox, (uint8_t*)nick.data());
		setWString("Nick", ptrW(Utf8DecodeW(nick.c_str())));

		//temporary
		size = tox_get_self_status_message_size(tox);
		std::string statusmes(size, 0);
		tox_get_self_status_message(tox, (uint8_t*)statusmes.data(), size);
		setWString("StatusMsg", ptrW(Utf8DecodeW(statusmes.c_str())));

		std::tstring avatarPath = GetAvatarFilePath();
		if (IsFileExists(avatarPath))
		{
			SetToxAvatar(avatarPath);
		}
	}
	else
	{
		debugLogA("CToxProto::InitToxCore: failed to load tox profile");

		if (password != NULL)
		{
			mir_free(password);
			password = NULL;
		}
		tox_kill(tox);
		tox = NULL;
	}

	return isProfileLoaded;
}

void CToxProto::UninitToxCore()
{
	for (size_t i = 0; i < transfers.Count(); i++)
	{
		FileTransferParam *transfer = transfers.GetAt(i);
		transfer->status = CANCELED;
		tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
		transfers.Remove(transfer);
	}

	ptrA nickname(mir_utf8encodeW(ptrT(getTStringA("Nick"))));
	tox_set_name(tox, (uint8_t*)(char*)nickname, mir_strlen(nickname));

	//temporary
	ptrA statusmes(mir_utf8encodeW(ptrT(getTStringA("StatusMsg"))));
	tox_set_status_message(tox, (uint8_t*)(char*)statusmes, mir_strlen(statusmes));

	SaveToxProfile();
	if (password != NULL)
	{
		mir_free(password);
		password = NULL;
	}
	tox_kill(tox);
	tox = NULL;
}