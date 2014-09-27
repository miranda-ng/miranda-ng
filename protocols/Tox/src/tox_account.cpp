#include "common.h"
#include "tox_bootstrap.h"

bool CToxProto::IsOnline()
{
	return isConnected && m_iStatus > ID_STATUS_OFFLINE;
}

int CToxProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CToxProto::OnOptionsInit);
	HookProtoEvent(ME_PROTO_ACCLISTCHANGED, &CToxProto::OnAccountListChanged);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CToxProto::OnSettingsChanged);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CToxProto::OnPreCreateMessage);

	InitNetlib();

	return 0;
}

void CToxProto::InitToxCore()
{
	Tox_Options options = { 0 };
	options.udp_disabled = getByte("DisableUDP", 0);
	options.ipv6enabled = !getByte("DisableIPv6", 0);

	if (hNetlib)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)hNetlib, (LPARAM)&nlus);

		if (nlus.useProxy)
		{
			if (nlus.proxyType == PROXYTYPE_SOCKS4 || nlus.proxyType == PROXYTYPE_SOCKS5)
			{
				debugLogA("CToxProto::InitToxCore: Setting socks user proxy config");
				options.proxy_enabled = 1;
				strcpy(&options.proxy_address[0], nlus.szProxyServer);
				options.proxy_port = nlus.wProxyPort;
			}
		}
	}

	tox = tox_new(&options);
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

	LoadToxData();

	int size = tox_get_self_name_size(tox);
	std::vector<uint8_t> username(size);
	tox_get_self_name(tox, &username[0]);
	std::string nick(username.begin(), username.end());
	setWString("Nick", ptrW(Utf8DecodeW(nick.c_str())));

	std::vector<uint8_t> pubKey(TOX_FRIEND_ADDRESS_SIZE);
	tox_get_address(tox, &pubKey[0]);
	std::string address = DataToHexString(pubKey);
	setString(NULL, TOX_SETTINGS_ID, address.c_str());

	std::tstring avatarPath = GetAvatarFilePath();
	if (IsFileExists(avatarPath))
	{
		SetToxAvatar(avatarPath);
	}
}

void CToxProto::UninitToxCore()
{
	SaveToxData();
	tox_kill(tox);
}

void CToxProto::DoBootstrap()
{
	static int j = 0;
	int i = 0;
	while (i < 2)
	{
		struct bootstrap_node *d = &bootstrap_nodes[j % SIZEOF(bootstrap_nodes)];
		tox_bootstrap_from_address(tox, d->address, d->port, d->key);
		i++; j++;
	}
}

void CToxProto::DoTox()
{
	mir_cslock lck(toxLock);

	tox_do(tox);
	uint32_t interval = tox_do_interval(tox);
	Sleep(interval);
}

void CToxProto::PollingThread(void*)
{
	debugLogA("CToxProto::PollingThread: entering");

	isConnected = false;
	DoBootstrap();

	while (!isTerminated)
	{
		DoTox();

		if (!isConnected)
		{
			if (tox_isconnected(tox))
			{
				isConnected = true;
				debugLogA("CToxProto::PollingThread: successfuly connected to DHT");

				LoadFriendList();

				debugLogA("CToxProto::PollingThread: changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
				m_iStatus = m_iDesiredStatus;
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
			}
			else
			{
				DoBootstrap();
			}
		}
		/*else
		{
			if (!tox_isconnected(tox))
			{
				debugLogA("CToxProto::PollingThread: disconnected from DHT");
				SetStatus(ID_STATUS_OFFLINE);
			}
		}*/
	}

	isConnected = false;

	debugLogA("CToxProto::PollingThread: leaving");
}