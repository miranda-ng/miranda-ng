#include "stdafx.h"

#include "csocket.h"
#include "cserver.h"

void CServer::Start(int port, IConnectionProcessorFactory *connectionProcessorFactory, bool background)
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET) return;

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((WORD)port);
	if (bind(m_socket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return;
	}

	listen(m_socket, SOMAXCONN);

	m_connectionProcessorFactory = connectionProcessorFactory;

	if (background)
		mir_forkthread(GlobalConnectionAcceptThread, this);
	else
		ConnectionAcceptThread();
}

void CServer::Stop()
{
	shutdown(m_socket, SD_BOTH);
	closesocket(m_socket);
}

DWORD CServer::ConnectionAcceptThread()
{
	while (1) {
		SOCKET s = accept(m_socket, NULL, NULL);
		if (s == INVALID_SOCKET) break;

		mir_forkthread(GlobalConnectionProcessThread, new GlobalConnectionProcessThreadArgs(this, s));
	}
	return 0;
}

DWORD CServer::ConnectionProcessThread(SOCKET s)
{
	CSocket sock(s);
	IConnectionProcessor *processor = m_connectionProcessorFactory->Create(&sock);
	processor->ProcessConnection();
	delete processor;
	return 0;
}

void CServer::GlobalConnectionAcceptThread(void *arg)
{
	CServer *server = (CServer *)arg;
	server->ConnectionAcceptThread();
}

void CServer::GlobalConnectionProcessThread(void *arg)
{
	GlobalConnectionProcessThreadArgs *args = (GlobalConnectionProcessThreadArgs *)arg;
	args->m_server->ConnectionProcessThread(args->m_socket);
	delete args;
}
