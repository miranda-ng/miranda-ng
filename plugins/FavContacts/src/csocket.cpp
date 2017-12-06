#include "stdafx.h"

#include "csocket.h"

int CSocket::Recv(char *buf, int count)
{
	return recv(m_socket, buf, count, 0);
}

int CSocket::Send(char *buf, int count)
{
	if (count < 0)
		count = (int)mir_strlen(buf);
	return send(m_socket, buf, count, 0);
}

void CSocket::Close()
{
	shutdown(m_socket, SD_BOTH);
	closesocket(m_socket);
}
