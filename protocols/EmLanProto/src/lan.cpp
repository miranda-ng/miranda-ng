//////////////////////////////////////////////////////////////////////////
// Lan protocol

#include "stdafx.h"

CLan::CLan()
{
	m_income = INVALID_SOCKET;
	m_filesoc = INVALID_SOCKET;
	m_status = LS_OK;
	m_mode = LM_OFF;
	m_hListenThread = NULL;
	m_hAcceptTCPThread = NULL;
	Startup();
}

CLan::~CLan()
{
	Shutdown();
}

void CLan::Startup()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2), &wsa)==0)
	{
		m_status = LS_OK;
		m_mode = LM_ON;

		char hostname[256];
		if (gethostname(hostname, 256)==0)
		{
			hostent* host = gethostbyname(hostname);
			char** pAddr = host->h_addr_list;
			m_hostAddrCount = 0;
			while (*pAddr && m_hostAddrCount<MAX_INTERNAL_IP)
			{
				in_addr addr;
				addr.S_un.S_addr = *((u_long*)(*pAddr));
				m_hostAddr[m_hostAddrCount++] = addr;
				pAddr++;
			}
			m_curAddr = m_hostAddr[0];
		}
		else
		{
			m_status = LS_CANT_GET_HOSTADDR;
		}
	}
	else
	{
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
	if (m_mode==LM_OFF)
		return;

	if (m_hListenThread)
	{
		TerminateThread(m_hListenThread, 0);
		m_hListenThread = NULL;
	}
	if (m_hAcceptTCPThread)
	{
		TerminateThread(m_hAcceptTCPThread, 0);
		m_hAcceptTCPThread = NULL;
	}
	if (m_income != INVALID_SOCKET)
	{
		closesocket(m_income);
		m_income = INVALID_SOCKET;
	}
	if (m_filesoc != INVALID_SOCKET)
	{
		closesocket(m_filesoc);
		m_filesoc = INVALID_SOCKET;
	}
	m_mode = LM_ON;
}

void CLan::StartListen()
{
	if (m_mode!=LM_ON)
		return;

	m_income = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	m_filesoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_income == INVALID_SOCKET || m_filesoc == INVALID_SOCKET)
	{
		m_status = LS_CANT_CREATE_SOCKET;
		StopListen();
	}
	else
	{
		int enable = 1;
		if (setsockopt(m_income, SOL_SOCKET, SO_BROADCAST, (const char*)&enable, sizeof(enable))!=0)
		{
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
		if (bind(m_income, (sockaddr*)&addr, sizeof(addr))!=0)
		{
			m_mode = LM_ON;
			m_status = LS_CANT_BIND_SOCKET;
			StopListen();
			return;
		}

		if (bind(m_filesoc, (sockaddr*)&addr, sizeof(addr))!=0)
		{
			m_mode = LM_ON;
			m_status = LS_CANT_BIND_SOCKET;
			StopListen();
			return;
		}

		if (listen(m_filesoc, SOMAXCONN)!=0)
		{
			m_mode = LM_ON;
			m_status = LS_CANT_START_LISTEN;
			StopListen();
			return;
		}

		DWORD threadId;
		m_hListenThread =  CreateThread(NULL, 0, ListenProc, (LPVOID)this, 0, &threadId);
		m_hAcceptTCPThread = CreateThread(NULL, 0, AcceptTCPProc, (LPVOID)this, 0, &threadId);
		if (m_hListenThread==NULL || m_hAcceptTCPThread==NULL)
		{
			m_mode = LM_ON;
			m_status = LS_CANT_CREATE_THREADS;
			StopListen();
			return;
		}
	}
}

void CLan::SetCurHostAddress(in_addr addr)
{
	if (m_mode!=LM_OFF)
	{
		int oldMode = m_mode;
		StopListen();
		m_curAddr = addr;
		if (oldMode==LM_LISTEN)
			StartListen();
	}
}

DWORD WINAPI CLan::ListenProc(LPVOID lpParameter)
{
	CLan* lan = (CLan*)lpParameter;
	lan->Listen();
	return 0;
}

void CLan::Listen()
{
	if (m_mode==LM_LISTEN)
	{
		char buf[65536];
		while(1)
		{
			sockaddr_in addr;
			int addrLen = sizeof(addr);
			Sleep(20);
			int recLen = recvfrom(m_income, buf, 65536, 0, (sockaddr*)&addr, &addrLen);
			if (recLen!=SOCKET_ERROR)
				OnRecvPacket((u_char*)buf, recLen, addr.sin_addr);
		}
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
	if (m_mode==LM_LISTEN)
	{
		sockaddr_in addrTo;
		addrTo.sin_addr = addr;
		addrTo.sin_family = AF_INET;
		addrTo.sin_port = PORT_NUMBER;
		int res = sendto(m_income, (const char*)mes, len, 0, (sockaddr*)&addrTo, sizeof(addrTo));
	}
}

//////////////////////////////////////////////////////////////////////////

DWORD WINAPI CLan::AcceptTCPProc(LPVOID lpParameter)
{
	CLan* lan = (CLan*)lpParameter;
	lan->AcceptTCP();
	return 0;
}

void CLan::AcceptTCP()
{
	while (1)
	{
		SOCKET in_socket;
		sockaddr_in addrFrom;
		int addrLen = sizeof(addrFrom);
		in_socket = accept(m_filesoc, (sockaddr*)&addrFrom, &addrLen);
		if (in_socket != INVALID_SOCKET)
		{
			TTCPConnect* tcp_conn = new TTCPConnect;
			tcp_conn->m_addr = addrFrom.sin_addr.S_un.S_addr;
			tcp_conn->m_lan = this;
			tcp_conn->m_socket = in_socket;
			mir_forkthread(OnInTCPConnectionProc, (void*)tcp_conn);
		}
		Sleep(100);
	}
}

void __cdecl CLan::OnInTCPConnectionProc(void *lpParameter)
{
	TTCPConnect* tcp_conn = (TTCPConnect*)lpParameter;
	tcp_conn->m_lan->OnInTCPConnection(tcp_conn->m_addr, tcp_conn->m_socket);
	shutdown(tcp_conn->m_socket, SD_BOTH);
	closesocket(tcp_conn->m_socket);
	delete tcp_conn;
	return;
}

SOCKET CLan::CreateTCPConnection(u_long addr, LPVOID lpParameter)
{
	SOCKET out_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (out_socket==INVALID_SOCKET)
		return INVALID_SOCKET;

	sockaddr_in addrTo;
	addrTo.sin_addr.S_un.S_addr = addr;
	addrTo.sin_family = AF_INET;
	addrTo.sin_port = PORT_NUMBER;
	if (connect(out_socket, (sockaddr*)&addrTo, sizeof(addrTo))!=0)
	{
		closesocket(out_socket);
		out_socket = INVALID_SOCKET;
	}

	//OnOutTCPConnectionProc is called anyway
	TTCPConnect* tcp_conn = new TTCPConnect;
	tcp_conn->m_socket = out_socket;
	tcp_conn->m_lan = this;
	tcp_conn->m_addr = addr;
	tcp_conn->m_lpParameter = lpParameter;

	DWORD threadId;
	CreateThread(NULL, 0, OnOutTCPConnectionProc, (LPVOID)tcp_conn, 0, &threadId);

	return out_socket;
}

DWORD WINAPI CLan::OnOutTCPConnectionProc(LPVOID lpParameter)
{
	TTCPConnect* tcp_conn = (TTCPConnect*)lpParameter;
	tcp_conn->m_lan->OnOutTCPConnection(tcp_conn->m_addr, tcp_conn->m_socket, tcp_conn->m_lpParameter);
	shutdown(tcp_conn->m_socket, SD_BOTH);
	closesocket(tcp_conn->m_socket);
	delete tcp_conn;
	return 0;
}
