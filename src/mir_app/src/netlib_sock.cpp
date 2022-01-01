/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "netlib.h"

extern HANDLE hConnectionHeaderMutex, hSendEvent, hRecvEvent;

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_Send(HNETLIBCONN nlc, const char *buf, int len, int flags)
{
	if (!NetlibEnterNestedCS(nlc, NLNCS_SEND))
		return SOCKET_ERROR;

	int result;
	Netlib_Dump(nlc, (uint8_t*)buf, len, true, flags);
	if (nlc->hSsl)
		result = Netlib_SslWrite(nlc->hSsl, buf, len);
	else
		result = send(nlc->s, buf, len, flags & 0xFFFF);

	NetlibLeaveNestedCS(&nlc->ncsSend);

	NETLIBNOTIFY nln = { buf, len, flags, result };
	NotifyFastHook(hSendEvent, (WPARAM)&nln, (LPARAM)&nlc->nlu->user);

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_Recv(HNETLIBCONN nlc, char *buf, int len, int flags)
{
	if (!NetlibEnterNestedCS(nlc, NLNCS_RECV))
		return SOCKET_ERROR;

	int recvResult;
	if (!nlc->foreBuf.isEmpty()) {
		recvResult = min(len, (int)nlc->foreBuf.length());
		memcpy(buf, nlc->foreBuf.data(), recvResult);
		nlc->foreBuf.remove(recvResult);
	}
	else if (nlc->hSsl)
		recvResult = Netlib_SslRead(nlc->hSsl, buf, len, (flags & MSG_PEEK) != 0);
	else
		recvResult = recv(nlc->s, buf, len, flags & 0xFFFF);

	NetlibLeaveNestedCS(&nlc->ncsRecv);
	if (recvResult <= 0)
		return recvResult;

	Netlib_Dump(nlc, (uint8_t*)buf, recvResult, false, flags);

	if ((flags & MSG_PEEK) == 0) {
		NETLIBNOTIFY nln = { buf, len, flags, recvResult };
		NotifyFastHook(hRecvEvent, (WPARAM)&nln, (LPARAM)&nlc->nlu->user);
	}
	return recvResult;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ConnectionListToSocketList(const HNETLIBCONN *hConns, fd_set *fd, int& pending)
{
	FD_ZERO(fd);
	for (int i = 0; hConns[i] && hConns[i] != INVALID_HANDLE_VALUE && i < FD_SETSIZE; i++) {
		NetlibConnection *nlcCheck = hConns[i];
		if (nlcCheck->handleType != NLH_CONNECTION && nlcCheck->handleType != NLH_BOUNDPORT) {
			SetLastError(ERROR_INVALID_DATA);
			return 0;
		}
		FD_SET(nlcCheck->s, fd);
		if (!nlcCheck->foreBuf.isEmpty() || Netlib_SslPending(nlcCheck->hSsl))
			pending++;
	}
	return 1;
}

MIR_APP_DLL(int) Netlib_Select(NETLIBSELECT *nls)
{
	if (nls == nullptr) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	int pending = 0;
	fd_set readfd, writefd, exceptfd;
	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);
	if (!ConnectionListToSocketList(nls->hReadConns, &readfd, pending)
		 || !ConnectionListToSocketList(nls->hWriteConns, &writefd, pending)
		 || !ConnectionListToSocketList(nls->hExceptConns, &exceptfd, pending))
	{
		ReleaseMutex(hConnectionHeaderMutex);
		return SOCKET_ERROR;
	}
	ReleaseMutex(hConnectionHeaderMutex);
	if (pending)
		return 1;

	TIMEVAL tv;
	tv.tv_sec = nls->dwTimeout / 1000;
	tv.tv_usec = (nls->dwTimeout % 1000) * 1000;
	return select(0, &readfd, &writefd, &exceptfd, nls->dwTimeout == INFINITE ? nullptr : &tv);
}

MIR_APP_DLL(int) Netlib_SelectEx(NETLIBSELECTEX *nls)
{
	if (nls == nullptr) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	TIMEVAL tv;
	tv.tv_sec = nls->dwTimeout / 1000;
	tv.tv_usec = (nls->dwTimeout % 1000) * 1000;
	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);

	int pending = 0;
	fd_set readfd, writefd, exceptfd;
	if (!ConnectionListToSocketList(nls->hReadConns, &readfd, pending)
		|| !ConnectionListToSocketList(nls->hWriteConns, &writefd, pending)
		|| !ConnectionListToSocketList(nls->hExceptConns, &exceptfd, pending))
	{
		ReleaseMutex(hConnectionHeaderMutex);
		return SOCKET_ERROR;
	}
	ReleaseMutex(hConnectionHeaderMutex);

	int rc = (pending) ? pending : select(0, &readfd, &writefd, &exceptfd, nls->dwTimeout == INFINITE ? nullptr : &tv);

	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);
	/* go thru each passed HCONN array and grab its socket handle, then give it to FD_ISSET()
	to see if an event happened for that socket, if it has it will be returned as TRUE (otherwise not)
	This happens for read/write/except */
	NetlibConnection *conn = nullptr;
	int j;
	for (j = 0; j < FD_SETSIZE; j++) {
		conn = (NetlibConnection*)nls->hReadConns[j];
		if (conn == nullptr || conn == INVALID_HANDLE_VALUE) break;

		if (Netlib_SslPending(conn->hSsl))
			nls->hReadStatus[j] = TRUE;
		nls->hReadStatus[j] = FD_ISSET(conn->s, &readfd);
	}

	for (j = 0; j < FD_SETSIZE; j++) {
		conn = (NetlibConnection*)nls->hWriteConns[j];
		if (conn == nullptr || conn == INVALID_HANDLE_VALUE) break;
		nls->hWriteStatus[j] = FD_ISSET(conn->s, &writefd);
	}

	for (j = 0; j < FD_SETSIZE; j++) {
		conn = (NetlibConnection*)nls->hExceptConns[j];
		if (conn == nullptr || conn == INVALID_HANDLE_VALUE) break;
		nls->hExceptStatus[j] = FD_ISSET(conn->s, &exceptfd);
	}
	ReleaseMutex(hConnectionHeaderMutex);
	return rc;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Netlib_StringToAddress(const char *str, SOCKADDR_INET_M *addr)
{
	if (!str) return false;

	int len = sizeof(SOCKADDR_INET_M);
	return !WSAStringToAddressA((char*)str, AF_INET6, nullptr, (PSOCKADDR)addr, &len);
}

MIR_APP_DLL(char*) Netlib_AddressToString(sockaddr_in *addr)
{
	char saddr[128];
	DWORD len = sizeof(saddr);
	if (!WSAAddressToStringA((PSOCKADDR)addr, sizeof(*addr), nullptr, saddr, &len))
		return mir_strdup(saddr);

	if (addr->sin_family == AF_INET) {
		char *szIp = inet_ntoa(addr->sin_addr);
		if (addr->sin_port != 0) {
			mir_snprintf(saddr, "%s:%d", szIp, addr->sin_port);
			return mir_strdup(saddr);
		}
		return mir_strdup(szIp);
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_GetConnectionInfo(HNETLIBCONN nlc, NETLIBCONNINFO *connInfo)
{
	if (!nlc || !connInfo)
		return 1;

	sockaddr_in sin = { 0 };
	int len = sizeof(sin);
	if (!getsockname(nlc->s, (PSOCKADDR)&sin, &len)) {
		connInfo->wPort = ntohs(sin.sin_port);
		connInfo->dwIpv4 = sin.sin_family == AF_INET ? htonl(sin.sin_addr.s_addr) : 0;
		strncpy_s(connInfo->szIpPort, ptrA(Netlib_AddressToString(&sin)), _TRUNCATE);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline bool IsAddrGlobal(const IN6_ADDR *a)
{
	unsigned char High = a->s6_bytes[0] & 0xf0;
	return High != 0 && High != 0xf0;
}

MIR_APP_DLL(NETLIBIPLIST*) Netlib_GetMyIp(bool bGlobalOnly)
{
	addrinfo *air = nullptr, *ai, hints = { 0 };
	const char *szMyHost = "";

	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;

	if (GetAddrInfoA(szMyHost, nullptr, &hints, &air))
		return nullptr;

	unsigned n = 0;
	for (ai = air; ai; ai = ai->ai_next) {
		SOCKADDR_INET_M *iaddr = (SOCKADDR_INET_M*)ai->ai_addr;
		if (ai->ai_family == AF_INET || (ai->ai_family == AF_INET6 && (!bGlobalOnly || IsAddrGlobal(&iaddr->Ipv6.sin6_addr))))
			++n;
	}

	NETLIBIPLIST *addr = (NETLIBIPLIST*)mir_calloc(n * 64 + 4);
	addr->cbNum = n;

	unsigned i = 0;
	for (ai = air; ai; ai = ai->ai_next) {
		sockaddr_in6 *iaddr = (sockaddr_in6*)ai->ai_addr;
		if (ai->ai_family == AF_INET || (ai->ai_family == AF_INET6 && (!bGlobalOnly || IsAddrGlobal(&iaddr->sin6_addr)))) {
			char *szIp = Netlib_AddressToString((sockaddr_in*)iaddr);
			if (szIp)
				strncpy_s(addr->szIp[i++], szIp, _TRUNCATE);
			mir_free(szIp);
		}
	}
	FreeAddrInfoA(air);
	return addr;
}

static NETLIBIPLIST* GetMyIpv4(void)
{
	char hostname[256] = "";

	gethostname(hostname, sizeof(hostname));
	PHOSTENT he = gethostbyname(hostname);

	unsigned n;
	for (n = 0; he->h_addr_list[n]; ++n)
		;

	NETLIBIPLIST *addr = (NETLIBIPLIST*)mir_calloc(n * 64 + 4);
	addr->cbNum = n;

	for (unsigned i = 0; i < n; i++)
		strncpy_s(addr->szIp[i], inet_ntoa(*(PIN_ADDR)he->h_addr_list[i]), _TRUNCATE);

	return addr;
}
