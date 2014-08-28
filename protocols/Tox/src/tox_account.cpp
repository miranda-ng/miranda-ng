#include "common.h"
#include "tox_bootstrap.h"

void CToxProto::InitToxCore()
{
	Tox_Options options = { 0 };
	options.udp_disabled = getByte("DisableUDP", 0);
	options.ipv6enabled = !getByte("DisableIPv6", 0);

	if (hNetlibUser)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)hNetlibUser, (LPARAM)&nlus);

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
	tox_callback_file_control(tox, OnFileRequest, this);

	LoadToxData();

	int size = tox_get_self_name_size(tox);
	std::vector<uint8_t> username(size);
	tox_get_self_name(tox, &username[0]);
	std::string nick(username.begin(), username.end());
	setWString("Nick", ptrW(Utf8DecodeW(nick.c_str())));

	std::vector<uint8_t> address(TOX_FRIEND_ADDRESS_SIZE);
	tox_get_address(tox, &address[0]);
	std::string toxId = DataToHexString(address);
	setString(TOX_SETTINGS_ID, toxId.c_str());
}

void CToxProto::UninitToxCore()
{
	SaveToxData();
	tox_kill(tox);
}

bool CToxProto::IsOnline()
{
	return isConnected && m_iStatus > ID_STATUS_OFFLINE;
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
	uint32_t interval = 50;
	{
		mir_cs(tox_lock);

		tox_do(tox);
		interval = tox_do_interval(tox);
	}
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

				LoadContactList();

				debugLogA("CToxProto::PollingThread: changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
				m_iStatus = m_iDesiredStatus;
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
			}
			else
			{
				DoBootstrap();
			}
		}
	}

	isConnected = false;

	debugLogA("CToxProto::PollingThread: leaving");
}