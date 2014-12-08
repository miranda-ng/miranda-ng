#include "common.h"

char data[] = "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHH";
ICMP *ICMP::instance = 0;

#define BUFFER_SIZE			(16 * (sizeof(ICMP_ECHO_REPLY) + sizeof(data)))

ICMP::ICMP():
	timeout(2000),
	functions_loaded(false)
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		WSACleanup();
		return;
    }

	buff = new char[BUFFER_SIZE];
	functions_loaded = true;
}

void ICMP::stop()
{
}

ICMP::~ICMP() {
	if(hIP) stop();
	WSACleanup();
	delete[] buff;
}


bool ICMP::ping(char *host, ICMP_ECHO_REPLY &reply)
{
	if (!functions_loaded)
		return false;

	HOSTENT *rec;
	IP_OPTION_INFORMATION ipoi;

	unsigned long address = inet_addr(host);
	if (address == INADDR_NONE)
	{
		rec = gethostbyname(host);
		if (rec != NULL)
			address = *(unsigned long *)(*rec->h_addr_list);
		else
			return false;
	}

	ipoi.Ttl = 255;
	ipoi.Tos = 0;
	ipoi.Flags = 0;
	ipoi.OptionsSize = 0;
	ipoi.OptionsData = 0;
	
	reply.Status = 0;

	hIP = IcmpCreateFile();
	if (hIP == INVALID_HANDLE_VALUE)
		return false;

	DWORD rep_cnt = IcmpSendEcho2(hIP, 0, 0, 0, address, data, sizeof(data), 0, buff, BUFFER_SIZE, timeout);
	if (rep_cnt == 0)
	{
		DWORD code = GetLastError();
		if (code != 11010)
		{
			char winmsg[512], msg[1024];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, winmsg, 512, 0);
			mir_snprintf(msg, SIZEOF(msg), "Ping error (%d): %s", code, winmsg);
			PUShowMessage(msg, SM_NOTIFY);
			return false;
		}
	}
	memcpy(&reply, buff, sizeof(ICMP_ECHO_REPLY));

	IcmpCloseHandle(hIP);

	return (reply.Status == 0);
}

ICMP *ICMP::get_instance() {
	if(!instance)
		instance = new ICMP();
	return instance;
}

void ICMP::cleanup() {
	if(instance) delete instance;
}