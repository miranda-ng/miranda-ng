#include "stdafx.h"

void FreeSettingBlob(WORD pSize, void *pbBlob)
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	dbv.cpbVal = pSize;
	dbv.pbVal = (BYTE*)pbBlob;
	db_free(&dbv);
}

void ReadSettingBlob(MCONTACT hContact, char *ModuleName, char *SettingName, WORD *pSize, void **pbBlob)
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_BLOB;
	if (db_get(hContact, ModuleName, SettingName, &dbv)) {
		*pSize = 0;
		*pbBlob = nullptr;
	}
	else {
		*pSize = LOWORD(dbv.cpbVal);
		*pbBlob = (int*)dbv.pbVal;
	}
}

/////////////////////////////////////////////////////////////////////
// Email/SMS and WinSock functions

BOOL WS_Init()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) return FALSE;
	return TRUE;
}

void WS_CleanUp()
{
	WSACleanup();
}

int WS_Send(SOCKET s, char *data, int datalen)
{
	int rlen;
	if ((rlen = send(s, data, datalen, 0)) == SOCKET_ERROR) return FALSE;
	return TRUE;
}

unsigned long WS_ResolveName(char *name, WORD *port, int defaultPort)
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
	DWORD ret = inet_addr(nameCopy);
	free(nameCopy);
	return ret;
}

void Send(char *user, char *host, char *Msg, char *server)
{
	SOCKADDR_IN sockaddr;
	WORD port;
	char *ch = nullptr;
	SOCKET S = socket(AF_INET, SOCK_STREAM, 0);
	if (!server) server = host;
	if ((sockaddr.sin_addr.S_un.S_addr = WS_ResolveName(server,
		&port, 25)) == SOCKET_ERROR) return;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_family = AF_INET;
	if (connect(S, (SOCKADDR*)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) return;
	ch = (char*)malloc(mir_strlen(user) + mir_strlen(host) + 16);
	ch = (char*)realloc(ch, sprintf(ch, "rcpt to:%s@%s\r\n", user, host)); //!!!!!!!!!!
	WS_Send(S, "mail from: \r\n", 13);
	WS_Send(S, ch, (int)mir_strlen(ch));
	WS_Send(S, "data\r\n", 6);
	WS_Send(S, "From:<REM>\r\n\r\n", 14);
	WS_Send(S, Msg, (int)mir_strlen(Msg));
	WS_Send(S, "\r\n.\r\n", 5);
	WS_Send(S, "quit\r\n", 6);
	SAFE_FREE((void**)&ch);
	closesocket(S);
}
