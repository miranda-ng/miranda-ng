#include "common.h"
#include "tox_bootstrap.h"

bool CToxProto::IsOnline()
{
	return isConnected && m_iStatus > ID_STATUS_OFFLINE;
}

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
				debugLogA("Setting socks user proxy config");
				options.proxy_enabled = 1;
				strcpy(&options.proxy_address[0], nlus.szProxyServer);
				options.proxy_port = nlus.wProxyPort;
			}
		}
	}

	tox = tox_new(&options);
	tox_callback_friend_request(tox, OnFriendRequest, this);
	tox_callback_friend_message(tox, OnFriendMessage, this);
	tox_callback_friend_action(tox, OnAction, this);
	tox_callback_name_change(tox, OnFriendNameChange, this);
	tox_callback_status_message(tox, OnStatusMessageChanged, this);
	tox_callback_user_status(tox, OnUserStatusChanged, this);
	tox_callback_read_receipt(tox, OnReadReceipt, this);
	tox_callback_connection_status(tox, OnConnectionStatusChanged, this);

	LoadToxData();

	std::vector<uint8_t> username(TOX_MAX_NAME_LENGTH);
	tox_get_self_name(tox, &username[0]);
	std::string nick(username.begin(), username.end());
	setString("Nick", nick.c_str());
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
	uint32_t interval = 1000;
	{
		//mir_cslock lock(tox_lock);

		tox_do(tox);
		interval = tox_do_interval(tox);
	}
	Sleep(interval);
}

void CToxProto::PollingThread(void*)
{
	debugLogA("CToxProto::PollingThread: entering");

	while (!isTerminated)
	{
		DoTox();
	}

	debugLogA("CToxProto::PollingThread: leaving");
}

void CToxProto::ConnectionThread(void*)
{
	debugLogA("CToxProto::ConnectionThread: entering");

	while (!isTerminated && !isConnected)
	{
		DoBootstrap();

		if (tox_isconnected(tox))
		{
			isConnected = true;

			LoadContactList();

			m_iStatus = m_iDesiredStatus = ID_STATUS_ONLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);

			break;
		}

		DoTox();
	}

	debugLogA("CToxProto::ConnectionThread: leaving");

	if (!isTerminated && isConnected)
	{
		poolingThread = ForkThreadEx(&CToxProto::PollingThread, 0, NULL);
	}
}