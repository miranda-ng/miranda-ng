//////////////////////////////////////////////////////////////////////////
// Lan protocol

#include "stdafx.h"

CLan::CLan()
{
	memset(&m_curAddr, 0, sizeof(m_curAddr));
	memset(&m_hostAddr, 0, sizeof(m_hostAddr));

	Startup();
}

CLan::~CLan()
{
	Shutdown();
}

bool CLan::ResetInterfaces()
{
	char hostname[256];
	if (gethostname(hostname, 256) != 0) {
		m_status = LS_CANT_GET_HOSTADDR;
		return false;
	}

	int hostAddrCount = 0;
	in_addr hostAddr[MAX_INTERNAL_IP];

	hostent *host = gethostbyname(hostname);
	char **pAddr = host->h_addr_list;

	while (*pAddr && hostAddrCount < MAX_INTERNAL_IP) {
		in_addr addr;
		addr.S_un.S_addr = *((u_long *)(*pAddr));
		hostAddr[hostAddrCount++] = addr;
		pAddr++;
	}

	// nothing changed? return false
	if (m_hostAddrCount == hostAddrCount && !memcmp(m_hostAddr, hostAddr, sizeof(hostAddr)))
		return false;

	m_hostAddrCount = hostAddrCount;
	memcpy(m_hostAddr, hostAddr, sizeof(hostAddr));
	return true;
}

void CLan::Startup()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
		m_status = LS_OK;
		m_mode = LM_ON;

		ResetInterfaces();
	}
	else {
		m_status = LS_OK;
		m_mode = LM_OFF;
	}
}

void CLan::Shutdown()
{
	if (m_mode == LM_OFF)
		return;

	m_hostAddrCount = 0;
	StopListen();
	WSACleanup();
	m_status = LS_OK;
	m_mode = LM_OFF;
}

void CLan::StopListen()
{
	if (m_mode == LM_OFF)
		return;

	if (m_hListenThread) {
		shutdown(m_income, SD_BOTH);
		while (m_hListenThread)
			SleepEx(100, TRUE);
	}
	if (m_hAcceptTCPThread) {
		TerminateThread(m_hAcceptTCPThread, 0);
		m_hAcceptTCPThread = nullptr;
	}
	if (m_income != INVALID_SOCKET) {
		closesocket(m_income);
		m_income = INVALID_SOCKET;
	}
	if (m_filesoc != INVALID_SOCKET) {
		closesocket(m_filesoc);
		m_filesoc = INVALID_SOCKET;
	}
	m_mode = LM_ON;
}

void CLan::StartListen()
{
	if (m_mode != LM_ON)
		return;

	if (m_curAddr.S_un.S_addr == 0)
		return;

	m_income = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	m_filesoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_income == INVALID_SOCKET || m_filesoc == INVALID_SOCKET) {
		m_status = LS_CANT_CREATE_SOCKET;
		StopListen();
	}
	else {
		int enable = 1;
		if (setsockopt(m_income, SOL_SOCKET, SO_BROADCAST, (const char*)&enable, sizeof(enable)) != 0) {
			m_mode = LM_ON;
			m_status = LS_CANT_TURN_ON_BROADCAST;
			StopListen();
			return;
		}
		m_mode = LM_LISTEN;
		sockaddr_in addr;
		addr.sin_addr = m_curAddr;
		addr.sin_family = AF_INET;
		addr.sin_port = PORT_NUMBER;
		if (bind(m_income, (sockaddr*)&addr, sizeof(addr)) != 0) {
			m_mode = LM_ON;
			m_status = LS_CANT_BIND_SOCKET;
			StopListen();
			return;
		}

		if (bind(m_filesoc, (sockaddr*)&addr, sizeof(addr)) != 0) {
			m_mode = LM_ON;
			m_status = LS_CANT_BIND_SOCKET;
			StopListen();
			return;
		}

		if (listen(m_filesoc, SOMAXCONN) != 0) {
			m_mode = LM_ON;
			m_status = LS_CANT_START_LISTEN;
			StopListen();
			return;
		}

		m_hListenThread = mir_forkthread(ListenProc, this);
		m_hAcceptTCPThread = mir_forkthread(AcceptTCPProc, this);
		if (m_hListenThread == nullptr || m_hAcceptTCPThread == nullptr) {
			m_mode = LM_ON;
			m_status = LS_CANT_CREATE_THREADS;
			StopListen();
			return;
		}
	}
}

void CLan::SetCurHostAddress(in_addr addr)
{
	if (m_mode != LM_OFF) {
		int oldMode = m_mode;
		StopListen();
		m_curAddr = addr;
		if (oldMode == LM_LISTEN)
			StartListen();
	}
}

void __cdecl CLan::ListenProc(void *lpParameter)
{
	CLan* lan = (CLan*)lpParameter;
	lan->Listen();
}

void CLan::Listen()
{
	if (m_mode != LM_LISTEN)
		return;

	MThreadLock threadLock(m_hListenThread);
	mir_ptr<char> buf((char*)mir_alloc(65536));

	while (true) {
		Sleep(20);

		sockaddr_in addr;
		int addrLen = sizeof(addr);
		int recLen = recvfrom(m_income, buf, 65536, 0, (sockaddr*)&addr, &addrLen);
		if (recLen == SOCKET_ERROR)
			break;

		OnRecvPacket((u_char*)buf.get(), recLen, addr.sin_addr);
	}
}

void CLan::SendPacketBroadcast(const u_char* mes, int len)
{
	in_addr addr;
	addr.S_un.S_addr = INADDR_BROADCAST;
	SendPacket(addr, mes, len);
}

void CLan::SendPacket(in_addr addr, const u_char* mes, int len)
{
	if (m_mode == LM_LISTEN) {
		sockaddr_in addrTo;
		addrTo.sin_addr = addr;
		addrTo.sin_family = AF_INET;
		addrTo.sin_port = PORT_NUMBER;
		sendto(m_income, (const char*)mes, len, 0, (sockaddr*)&addrTo, sizeof(addrTo));
	}
}

//////////////////////////////////////////////////////////////////////////

void __cdecl CLan::AcceptTCPProc(void *lpParameter)
{
	CLan* lan = (CLan*)lpParameter;
	lan->AcceptTCP();
}

void CLan::AcceptTCP()
{
	while (true) {
		sockaddr_in addrFrom;
		int addrLen = sizeof(addrFrom);
		SOCKET in_socket = accept(m_filesoc, (sockaddr*)&addrFrom, &addrLen);
		if (in_socket == SOCKET_ERROR)
			break;
		
		TTCPConnect* tcp_conn = new TTCPConnect;
		tcp_conn->m_addr = addrFrom.sin_addr.S_un.S_addr;
		tcp_conn->m_lan = this;
		tcp_conn->m_socket = in_socket;
		mir_forkthread(OnInTCPConnectionProc, tcp_conn);
		Sleep(100);
	}
	m_hAcceptTCPThread = nullptr;
}

void __cdecl CLan::OnInTCPConnectionProc(void *lpParameter)
{
	TTCPConnect* tcp_conn = (TTCPConnect*)lpParameter;
	tcp_conn->m_lan->OnInTCPConnection(tcp_conn->m_addr, tcp_conn->m_socket);
	shutdown(tcp_conn->m_socket, SD_BOTH);
	closesocket(tcp_conn->m_socket);
	delete tcp_conn;
}

SOCKET CLan::CreateTCPConnection(u_long addr, LPVOID lpParameter)
{
	SOCKET out_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (out_socket == INVALID_SOCKET)
		return INVALID_SOCKET;

	sockaddr_in addrTo;
	addrTo.sin_addr.S_un.S_addr = addr;
	addrTo.sin_family = AF_INET;
	addrTo.sin_port = PORT_NUMBER;
	if (connect(out_socket, (sockaddr*)&addrTo, sizeof(addrTo)) != 0) {
		closesocket(out_socket);
		out_socket = INVALID_SOCKET;
	}

	//OnOutTCPConnectionProc is called anyway
	TTCPConnect* tcp_conn = new TTCPConnect;
	tcp_conn->m_socket = out_socket;
	tcp_conn->m_lan = this;
	tcp_conn->m_addr = addr;
	tcp_conn->m_lpParameter = lpParameter;
	mir_forkthread(OnOutTCPConnectionProc, tcp_conn);

	return out_socket;
}

void __cdecl CLan::OnOutTCPConnectionProc(void *lpParameter)
{
	TTCPConnect* tcp_conn = (TTCPConnect*)lpParameter;
	tcp_conn->m_lan->OnOutTCPConnection(tcp_conn->m_addr, tcp_conn->m_socket, tcp_conn->m_lpParameter);
	shutdown(tcp_conn->m_socket, SD_BOTH);
	closesocket(tcp_conn->m_socket);
	delete tcp_conn;
}
