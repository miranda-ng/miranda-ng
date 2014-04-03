#include "common.h"

int CSteamProto::PollStatus()
{
	Sleep(2000);

	return 0;
}

void CSteamProto::PollingThread(void*)
{
	debugLogA("CSteamProto::PollingThread: entering");

	while (!m_bTerminated)
		if (PollStatus() == -1)
			break;

	m_hPollingThread = NULL;
	debugLogA("CSteamProto::PollingThread: leaving");
}