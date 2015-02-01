#include "common.h"
#include "tox_bootstrap.h"

bool CToxProto::IsOnline()
{
	return isConnected && m_iStatus > ID_STATUS_OFFLINE;
}

int CToxProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CToxProto::OnOptionsInit);
	HookProtoEvent(ME_USERINFO_INITIALISE, &CToxProto::OnUserInfoInit);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CToxProto::OnPreCreateMessage);

	return 0;
}

int CToxProto::OnAccountRenamed(WPARAM, LPARAM lParam)
{
	PROTOACCOUNT *account = (PROTOACCOUNT*)lParam;

	std::tstring newPath = GetToxProfilePath();
	TCHAR oldPath[MAX_PATH];
	mir_sntprintf(oldPath, MAX_PATH, _T("%s\\%s.tox"), VARST(_T("%miranda_userdata%")), accountName);
	_trename(oldPath, newPath.c_str());
	mir_free(accountName);
	accountName = mir_tstrdup(m_tszUserName);

	return 0;
}

bool CToxProto::InitToxCore()
{
	debugLogA("CToxProto::InitToxCore: initializing tox profile");

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

	tox = tox_new(&options);
	password = mir_utf8encodeW(ptrT(getTStringA("Password")));
	bool isProfileLoaded = LoadToxProfile();
	if (isProfileLoaded)
	{
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

		uint8_t data[TOX_FRIEND_ADDRESS_SIZE];
		tox_get_address(tox, data);
		ToxHexAddress address(data, TOX_FRIEND_ADDRESS_SIZE);
		setString(NULL, TOX_SETTINGS_ID, address);

		int size = tox_get_self_name_size(tox);
		std::string nick(size, 0);
		tox_get_self_name(tox, (uint8_t*)nick.data());
		setWString("Nick", ptrW(Utf8DecodeW(nick.c_str())));

		std::tstring avatarPath = GetAvatarFilePath();
		if (IsFileExists(avatarPath))
		{
			SetToxAvatar(avatarPath);
		}
	}
	else
	{
		if (password != NULL)
		{
			mir_free(password);
			password = NULL;
		}
		tox_kill(tox);
	}

	return isProfileLoaded;
}

void CToxProto::UninitToxCore()
{
	ptrA nickname(mir_utf8encodeW(ptrT(getTStringA("Nick"))));
	tox_set_name(tox, (uint8_t*)(char*)nickname, mir_strlen(nickname));

	SaveToxProfile();
	if (password != NULL)
	{
		mir_free(password);
		password = NULL;
	}
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
	tox_do(tox);
	uint32_t interval = tox_do_interval(tox);
	Sleep(interval);
}

void CToxProto::PollingThread(void*)
{
	debugLogA("CToxProto::PollingThread: entering");

	isConnected = false;
	if (InitToxCore())
	{
		DoBootstrap();
	}
	else
	{
		isTerminated = true;
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, LOGINERR_WRONGPASSWORD);
		debugLogA("CToxProto::PollingThread: leaving");
		return;
	}

	while (!isTerminated)
	{
		DoTox();

		if (!isConnected)
		{
			if (tox_isconnected(tox))
			{
				isConnected = true;
				debugLogA("CToxProto::PollingThread: successfuly connected to DHT");

				ForkThread(&CToxProto::LoadFriendList, NULL);

				debugLogA("CToxProto::PollingThread: changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
				m_iStatus = m_iDesiredStatus;
				tox_set_user_status(tox, MirandaToToxStatus(m_iStatus));
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

	UninitToxCore();
	isConnected = false;

	debugLogA("CToxProto::PollingThread: leaving");
}