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