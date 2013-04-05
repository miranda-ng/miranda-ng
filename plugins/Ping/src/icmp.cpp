#include "common.h"

char data[] = "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHH";
ICMP *ICMP::instance = 0;

#define BUFFER_SIZE			(16 * (sizeof(ICMP_ECHO_REPLY) + sizeof(data)))

ICMP::ICMP():
	timeout(2000),
	functions_loaded(false)
{
	hDLL = LoadLibrary(_T("IPHLPAPI.DLL"));
	if(hDLL) {
		pIcmpCreateFile = (pfnHV)GetProcAddress(hDLL, "IcmpCreateFile");
		pIcmpCloseHandle = (pfnBH)GetProcAddress(hDLL, "IcmpCloseHandle");
		pIcmpSendEcho2 = (pfnDHDPWPipPDD)GetProcAddress(hDLL, "IcmpSendEcho2");
	}
	if (hDLL == 0 || pIcmpCreateFile == 0 || pIcmpCloseHandle == 0 || pIcmpSendEcho2 == 0) {
		hDLL = LoadLibrary(_T("ICMP.DLL"));
		if(hDLL) {
			pIcmpCreateFile = (pfnHV)GetProcAddress(hDLL, "IcmpCreateFile");
			pIcmpCloseHandle = (pfnBH)GetProcAddress(hDLL, "IcmpCloseHandle");
			pIcmpSendEcho2 = (pfnDHDPWPipPDD)GetProcAddress(hDLL, "IcmpSendEcho2");
		}
		if (hDLL == 0 || pIcmpCreateFile == 0 || pIcmpCloseHandle == 0 || pIcmpSendEcho2 == 0)
			return;
		else
			db_set_s(0, PLUG, "PingLib", "ICMP.DLL"); // for debugging
	} else
		db_set_s(0, PLUG, "PingLib", "IPHLPAPI.DLL"); // for debugging

	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		WSACleanup();
		FreeLibrary((HMODULE)hDLL);
		return;
    }

	/*
	hIP = pIcmpCreateFile();
	if (hIP == INVALID_HANDLE_VALUE) {
		pIcmpCloseHandle(hIP);
		return;
	}
	*/

	buff = new char[BUFFER_SIZE];
	functions_loaded = true;
}

void ICMP::stop() {
	//if(hIP) {
	//	pIcmpCloseHandle(hIP);
	//	hIP = 0;
	//}
}

ICMP::~ICMP() {
	if(hIP) stop();
	WSACleanup();
	if(hDLL)
		FreeLibrary(hDLL);
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

	hIP = pIcmpCreateFile();
	if (hIP == INVALID_HANDLE_VALUE)
		return false;

	//pIcmpSendEcho2(hIP, 0, 0, 0, address, data, sizeof(data), &ipoi, buff, sizeof(ICMP_ECHO_REPLY) + sizeof(data), timeout);
	DWORD rep_cnt = pIcmpSendEcho2(hIP, 0, 0, 0, address, data, sizeof(data), 0, buff, BUFFER_SIZE, timeout);
	if (rep_cnt == 0)
	{
		DWORD code = GetLastError();
		if (code != 11010)
		{
			char winmsg[512], msg[1024];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, winmsg, 512, 0);
			mir_snprintf(msg, 1024, "Ping error (%d): %s", code, winmsg);
			PUShowMessage(msg, SM_NOTIFY);
			return false;
		}
	}
	memcpy(&reply, buff, sizeof(ICMP_ECHO_REPLY));

	pIcmpCloseHandle(hIP);

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