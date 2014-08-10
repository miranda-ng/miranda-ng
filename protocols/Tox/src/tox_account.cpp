#include "common.h"
#include "tox_bootstrap.h"

void CToxProto::DoBootstrap()
{
	static int j = 0;
	int i = 0;
	while (i < 2)
	{
		struct bootstrap_node *d = &bootstrap_nodes[j % SIZEOF(bootstrap_nodes)];
		tox_bootstrap_from_address(tox, d->address, TOX_ENABLE_IPV6_DEFAULT, d->port, d->key);
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

		uint8_t name[TOX_MAX_NAME_LENGTH + 1];
		uint16_t namelen = tox_get_self_name(tox, name);
		name[namelen] = 0;

		if (tox_isconnected(tox))
		{
			isConnected = true;

			char dataPath[MAX_PATH];
			mir_snprintf(dataPath, MAX_PATH, "%s\\%s.tox", VARS("%miranda_profile%\\%miranda_profilename%"), _T2A(m_tszUserName));

			SaveToxData(dataPath);

			char idstring3[200] = { 0 };
			get_id(tox, idstring3);

			break;
		}

		DoTox();
	}

	if (!isTerminated && isConnected)
	{
		poolingThread = ForkThreadEx(&CToxProto::PollingThread, 0, NULL);
	}

	debugLogA("CToxProto::ConnectionThread: leaving");
}