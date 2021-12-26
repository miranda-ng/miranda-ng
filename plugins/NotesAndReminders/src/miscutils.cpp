#include "stdafx.h"

/////////////////////////////////////////////////////////////////////
// Email/SMS and WinSock functions

int WS_Send(SOCKET s, const char *data, int datalen)
{
	int rlen;
	if ((rlen = send(s, data, datalen, 0)) == SOCKET_ERROR) return FALSE;
	return TRUE;
}

unsigned long WS_ResolveName(char *name, uint16_t *port, int defaultPort)
{
	char *nameCopy = _strdup(name);
	if (port != nullptr)
		*port = defaultPort;
	char *pcolon = strchr(nameCopy, ':');
	if (pcolon != nullptr) {
		if (port != nullptr) *port = atoi(pcolon + 1);
		*pcolon = 0;
	}
	if (inet_addr(nameCopy) == INADDR_NONE) {
		HOSTENT *lk = gethostbyname(nameCopy);
		if (lk == nullptr)
			return SOCKET_ERROR;

		free(nameCopy);
		return *(u_long*)lk->h_addr_list[0];
	}
	uint32_t ret = inet_addr(nameCopy);
	free(nameCopy);
	return ret;
}

void Send(char *user, char *host, const char *Msg, char *server)
{
	SOCKET S = socket(AF_INET, SOCK_STREAM, 0);
	if (!server)
		server = host;

	uint16_t port;
	SOCKADDR_IN sockaddr;
	if ((sockaddr.sin_addr.S_un.S_addr = WS_ResolveName(server, &port, 25)) == SOCKET_ERROR)
		return;

	sockaddr.sin_port = htons(port);
	sockaddr.sin_family = AF_INET;
	if (connect(S, (SOCKADDR*)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR)
		return;

	CMStringA rcpt(FORMAT, "rcpt to:%s@%s\r\n", user, host);
	WS_Send(S, "mail from: \r\n", 13);
	WS_Send(S, rcpt, rcpt.GetLength());
	WS_Send(S, "data\r\n", 6);
	WS_Send(S, "From:<REM>\r\n\r\n", 14);
	WS_Send(S, Msg, (int)mir_strlen(Msg));
	WS_Send(S, "\r\n.\r\n", 5);
	WS_Send(S, "quit\r\n", 6);
	closesocket(S);
}
