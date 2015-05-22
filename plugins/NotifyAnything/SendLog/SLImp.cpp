#define _WINSOCKAPI_ // don't include winsock 1.x
#include <windows.h>
#include <winioctl.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>

void implementation(int argc, char **argv, bool console)
{
	int port = 12001;
	bool tcp = false;
	const char *target = "127.0.0.1";

	if (argc >= 2 && !mir_strcmp("-T", argv[1])) {
		tcp = true;
		argc -= 1;
		argv += 1;
	}

	if (argc >= 3 && !mir_strcmp("-H", argv[1])) {
		target = argv[2];
		argc -= 2;
		argv += 2;
	}

	if (argc >= 3 && !mir_strcmp("-P", argv[1])) {
		port = atoi(argv[2]);
		argc -= 2;
		argv += 2;
	}

	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err)
		throw (DWORD) err;

	char portbuf[10];
	sprintf(portbuf, "%i", port);

	ADDRINFO hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = PF_INET;
	ADDRINFO *res;
	err = getaddrinfo(target, portbuf, &hints, &res);

	if (err || !res)
		throw (DWORD) err;

	SOCKET sock;
	SOCKADDR_IN to = *(SOCKADDR_IN*)res->ai_addr;

	sock = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, tcp ? IPPROTO_TCP : IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
		throw (DWORD) WSAGetLastError();

	SOCKADDR_IN self;
	ZeroMemory(&self, sizeof self);
	self.sin_family = AF_INET;

	if (bind(sock, (const SOCKADDR *) &self, sizeof self))
		throw (DWORD) WSAGetLastError();

	if (connect(sock, (const SOCKADDR *) &to, sizeof to))
		throw (DWORD) WSAGetLastError();

	if (console && (argc == 1 || argc == 3 && !mir_strcmp("-p", argv[1]))) {
		std::string prefix;
		if (argc == 3)
			prefix = argv[2];
		std::string line;
		while (std::getline(std::cin, line)) {
			std::string data = prefix + line;
			send(sock, data.data(), data.size(), 0);
		}
	} else {
		std::string message = argv[1];
		for (int i=2; i<argc; ++i) {
			message += ' ';
			message += argv[i];
		}
		send(sock, message.data(), message.size(), 0);
	}

	shutdown(sock, SD_BOTH);
	closesocket(sock);

	WSACleanup();
}
