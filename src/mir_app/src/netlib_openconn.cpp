/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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

extern mir_cs csNetlibUser;
extern uint32_t g_LastConnectionTick;
extern int connectionTimeout;
static int iUPnPCleanup = 0;

#define RECV_DEFAULT_TIMEOUT	60000

//returns in network byte order
uint32_t DnsLookup(NetlibUser *nlu, const char *szHost)
{
	HOSTENT* host;
	uint32_t ip = inet_addr(szHost);
	if (ip != INADDR_NONE)
		return ip;

	__try {
		host = gethostbyname(szHost);
		if (host)
			return *(u_long*)host->h_addr_list[0];

		Netlib_Logf(nlu, "%s %d: %s() for host %s failed (%u)", __FILE__, __LINE__, "gethostbyname", szHost, WSAGetLastError());
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

	return 0;
}

int WaitUntilReadable(SOCKET s, uint32_t dwTimeout, bool check)
{
	fd_set readfd;
	TIMEVAL tv;

	if (s == INVALID_SOCKET) return SOCKET_ERROR;

	tv.tv_sec = dwTimeout / 1000;
	tv.tv_usec = (dwTimeout % 1000) * 1000;

	FD_ZERO(&readfd);
	FD_SET(s, &readfd);

	int result = select(0, &readfd, nullptr, nullptr, &tv);
	if (result == 0 && !check) SetLastError(ERROR_TIMEOUT);
	return result;
}

int WaitUntilWritable(SOCKET s, uint32_t dwTimeout)
{
	fd_set writefd;
	TIMEVAL tv;

	tv.tv_sec = dwTimeout / 1000;
	tv.tv_usec = (dwTimeout % 1000) * 1000;

	FD_ZERO(&writefd);
	FD_SET(s, &writefd);

	switch (select(0, nullptr, &writefd, nullptr, &tv)) {
	case 0:
		SetLastError(ERROR_TIMEOUT);
	case SOCKET_ERROR:
		return 0;
	}
	return 1;
}

bool RecvUntilTimeout(NetlibConnection *nlc, char *buf, int len, int flags, uint32_t dwTimeout)
{
	int nReceived = 0;
	uint32_t dwTimeNow, dwCompleteTime = GetTickCount() + dwTimeout;

	while ((dwTimeNow = GetTickCount()) < dwCompleteTime) {
		if (WaitUntilReadable(nlc->s, dwCompleteTime - dwTimeNow) <= 0) return false;
		nReceived = Netlib_Recv(nlc, buf, len, flags);
		if (nReceived <= 0) return false;

		buf += nReceived;
		len -= nReceived;
		if (len <= 0) return true;
	}
	SetLastError(ERROR_TIMEOUT);
	return false;
}

static int NetlibInitSocks4Connection(NetlibConnection *nlc)
{
	//	http://www.socks.nec.com/protocol/socks4.protocol and http://www.socks.nec.com/protocol/socks4a.protocol
	NetlibUrl &url = nlc->url;
	if (url.szHost.IsEmpty())
		return 0;

	NetlibUser *nlu = nlc->nlu;
	size_t nHostLen = mir_strlen(url.szHost) + 1;
	size_t nUserLen = nlu->settings.szProxyAuthUser ? mir_strlen(nlu->settings.szProxyAuthUser) + 1 : 1;
	size_t len = 8 + nUserLen;

	char* pInit = (char*)alloca(len + nHostLen);
	pInit[0] = 4;	// SOCKS4
	pInit[1] = 1;	//connect
	*(PWORD)&pInit[2] = htons(url.port);

	if (nUserLen <= 1) pInit[8] = 0;
	else memcpy(&pInit[8], nlu->settings.szProxyAuthUser, nUserLen);

	//if cannot resolve host, try resolving through proxy (requires SOCKS4a)
	uint32_t ip = DnsLookup(nlu, url.szHost);
	*(PDWORD)&pInit[4] = ip ? ip : 0x01000000;
	if (!ip) {
		memcpy(&pInit[len], url.szHost, nHostLen);
		len += nHostLen;
	}

	if (Netlib_Send(nlc, pInit, (int)len, MSG_DUMPPROXY) == SOCKET_ERROR) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "Netlib_Send", GetLastError());
		return 0;
	}

	char reply[8];
	if (!RecvUntilTimeout(nlc, reply, sizeof(reply), MSG_DUMPPROXY, RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "RecvUntilTimeout", GetLastError());
		return 0;
	}

	switch ((uint8_t)reply[1]) {
		case 90: return 1;
		case 91: SetLastError(ERROR_ACCESS_DENIED); break;
		case 92: SetLastError(ERROR_CONNECTION_UNAVAIL); break;
		case 93: SetLastError(ERROR_INVALID_ACCESS); break;
		default: SetLastError(ERROR_INVALID_DATA); break;
	}
	Netlib_Logf(nlu, "%s %d: Proxy connection failed (%x %u)", __FILE__, __LINE__, (uint8_t)reply[1], GetLastError());
	return 0;
}

static int NetlibInitSocks5Connection(NetlibConnection *nlc)
{
	//rfc1928
	uint8_t buf[258];
	NetlibUser *nlu = nlc->nlu;

	buf[0] = 5;  //yep, socks5
	buf[1] = 1;  //one auth method
	buf[2] = nlu->settings.useProxyAuth ? 2 : 0;
	if (Netlib_Send(nlc, (char*)buf, 3, MSG_DUMPPROXY) == SOCKET_ERROR) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "Netlib_Send", GetLastError());
		return 0;
	}

	//confirmation of auth method
	if (!RecvUntilTimeout(nlc, (char*)buf, 2, MSG_DUMPPROXY, RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "RecvUntilTimeout", GetLastError());
		return 0;
	}
	if ((buf[1] != 0 && buf[1] != 2)) {
		SetLastError(ERROR_INVALID_ID_AUTHORITY);
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "Netlib_Recv", GetLastError());
		return 0;
	}

	if (buf[1] == 2) {		//rfc1929
		size_t nUserLen = mir_strlen(nlu->settings.szProxyAuthUser);
		size_t nPassLen = mir_strlen(nlu->settings.szProxyAuthPassword);
		uint8_t *pAuthBuf = (uint8_t*)mir_alloc(3 + nUserLen + nPassLen);
		pAuthBuf[0] = 1;		//auth version
		pAuthBuf[1] = (uint8_t)nUserLen;
		memcpy(pAuthBuf + 2, nlu->settings.szProxyAuthUser, nUserLen);
		pAuthBuf[2 + nUserLen] = (uint8_t)nPassLen;
		memcpy(pAuthBuf + 3 + nUserLen, nlu->settings.szProxyAuthPassword, nPassLen);
		if (Netlib_Send(nlc, (char*)pAuthBuf, int(3 + nUserLen + nPassLen), MSG_DUMPPROXY) == SOCKET_ERROR) {
			Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "Netlib_Send", GetLastError());
			mir_free(pAuthBuf);
			return 0;
		}
		mir_free(pAuthBuf);

		if (!RecvUntilTimeout(nlc, (char*)buf, 2, MSG_DUMPPROXY, RECV_DEFAULT_TIMEOUT)) {
			Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "RecvUntilTimeout", GetLastError());
			return 0;
		}
		if (buf[1]) {
			SetLastError(ERROR_ACCESS_DENIED);
			Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "RecvUntilTimeout", GetLastError());
			return 0;
		}
	}

	size_t nHostLen;
	uint32_t hostIP;

	NetlibUrl &url = nlc->url;
	if (nlc->dnsThroughProxy) {
		hostIP = inet_addr(url.szHost);
		nHostLen = (hostIP == INADDR_NONE) ? mir_strlen(url.szHost) + 1 : 4;
	}
	else {
		hostIP = DnsLookup(nlu, url.szHost);
		if (hostIP == 0)
			return 0;
		nHostLen = 4;
	}
	uint8_t *pInit = (uint8_t*)mir_alloc(6 + nHostLen);
	pInit[0] = 5;   //SOCKS5
	pInit[1] = url.flags & NLOCF_UDP ? 3 : 1; //connect or UDP
	pInit[2] = 0;   //reserved
	if (hostIP == INADDR_NONE) {		 //DNS lookup through proxy
		pInit[3] = 3;
		pInit[4] = uint8_t(nHostLen - 1);
		memcpy(pInit + 5, url.szHost, nHostLen - 1);
	}
	else {
		pInit[3] = 1;
		*(PDWORD)(pInit + 4) = hostIP;
	}
	*(PWORD)(pInit + 4 + nHostLen) = htons(url.port);
	if (Netlib_Send(nlc, (char*)pInit, int(6 + nHostLen), MSG_DUMPPROXY) == SOCKET_ERROR) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "Netlib_Send", GetLastError());
		mir_free(pInit);
		return 0;
	}
	mir_free(pInit);

	if (!RecvUntilTimeout(nlc, (char*)buf, 5, MSG_DUMPPROXY, RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "RecvUntilTimeout", GetLastError());
		return 0;
	}

	if (buf[0] != 5 || buf[1]) {
		const char* err = "Unknown response";
		if (buf[0] != 5)
			SetLastError(ERROR_BAD_FORMAT);
		else {
			switch (buf[1]) {
			case 1: SetLastError(ERROR_GEN_FAILURE); err = "General failure"; break;
			case 2: SetLastError(ERROR_ACCESS_DENIED); err = "Connection not allowed by ruleset";  break;
			case 3: SetLastError(WSAENETUNREACH); err = "Network unreachable"; break;
			case 4: SetLastError(WSAEHOSTUNREACH); err = "Host unreachable"; break;
			case 5: SetLastError(WSAECONNREFUSED); err = "Connection refused by destination host"; break;
			case 6: SetLastError(WSAETIMEDOUT); err = "TTL expired"; break;
			case 7: SetLastError(ERROR_CALL_NOT_IMPLEMENTED); err = "Command not supported / protocol error"; break;
			case 8: SetLastError(ERROR_INVALID_ADDRESS); err = "Address type not supported"; break;
			default: SetLastError(ERROR_INVALID_DATA); break;
			}
		}
		Netlib_Logf(nlu, "%s %d: Proxy conection failed. %s.", __FILE__, __LINE__, err);
		return 0;
	}

	int nRecvSize = 0;
	switch (buf[3]) {
	case 1:// ipv4 addr
		nRecvSize = 5;
		break;
	case 3:// dns name addr
		nRecvSize = buf[4] + 2;
		break;
	case 4:// ipv6 addr
		nRecvSize = 17;
		break;
	default:
		Netlib_Logf(nlu, "%s %d: %s() unknown address type (%u)", __FILE__, __LINE__, "NetlibInitSocks5Connection", (int)buf[3]);
		return 0;
	}
	if (!RecvUntilTimeout(nlc, (char*)buf, nRecvSize, MSG_DUMPPROXY, RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "RecvUntilTimeout", GetLastError());
		return 0;
	}

	//connected
	return 1;
}

static bool NetlibInitHttpsConnection(NetlibConnection *nlc)
{
	// rfc2817
	NetlibUrl &url = nlc->url;
	CMStringA szUrl;
	if (nlc->dnsThroughProxy)
		szUrl.Format("%s:%u", url.szHost.c_str(), url.port);
	else {
		uint32_t ip = DnsLookup(nlc->nlu, url.szHost);
		if (ip == 0) return false;
		szUrl.Format("%s:%u", inet_ntoa(*(PIN_ADDR)&ip), url.port);
	}

	MMemoryChunkStorage storage;
	MHttpRequest nlhrSend(REQUEST_CONNECT);
	nlhrSend.flags = NLHRF_DUMPPROXY | NLHRF_HTTP11 | NLHRF_NOPROXY | NLHRF_REDIRECT;
	nlhrSend.m_szUrl = szUrl;

	if (Netlib_SendHttpRequest(nlc, &nlhrSend, storage) == SOCKET_ERROR)
		return false;

	NLHR_PTR nlhrReply(NetlibHttpRecv(nlc, MSG_DUMPPROXY | MSG_RAW, MSG_DUMPPROXY | MSG_RAW, storage, true));
	if (nlhrReply == nullptr)
		return false;

	if (nlhrReply->resultCode < 200 || nlhrReply->resultCode >= 300) {
		if (nlhrReply->resultCode == 403 && nlc->dnsThroughProxy) {
			nlc->dnsThroughProxy = 0;
			return NetlibInitHttpsConnection(nlc);
		}

		NetlibHttpSetLastErrorUsingHttpResult(nlhrReply->resultCode);
		Netlib_Logf(nlc->nlu, "%s %d: %s request failed (%u %s)", __FILE__, __LINE__, 
			nlc->nlu->settings.proxyType == PROXYTYPE_HTTP ? "HTTP" : "HTTPS", nlhrReply->resultCode, nlhrReply->szResultDescr);
		return false;
	}

	return true; // connected
}

static void FreePartiallyInitedConnection(NetlibConnection *nlc)
{
	uint32_t dwOriginalLastError = GetLastError();

	if (GetNetlibHandleType(nlc) == NLH_CONNECTION)
		delete nlc;

	SetLastError(dwOriginalLastError);
}

static bool my_connectIP(NetlibConnection *nlc)
{
	NetlibUser *nlu = nlc->nlu;
	int rc = SOCKET_ERROR, retrycnt = 0;
	u_long notblocking = 1;
	uint32_t lasterr = 0;
	static const TIMEVAL tv = { 1, 0 };
	
	// if timeout is zero then its an old style connection or new with a 0 timeout, select() will error quicker anyway
	int timeout = (nlc->timeout <= 0) ? 30 : nlc->timeout;

	// this is for XP SP2 where there is a default connection attempt limit of 10/second
	if (connectionTimeout) {
		WaitForSingleObject(hConnectionOpenMutex, 10000);
		int waitdiff = GetTickCount() - g_LastConnectionTick;
		if (waitdiff < connectionTimeout) SleepEx(connectionTimeout, TRUE);
		g_LastConnectionTick = GetTickCount();
		ReleaseMutex(hConnectionOpenMutex);

		// might have died in between the wait
		if (Miranda_IsTerminated())
			return false;
	}

	char szPort[6];
	addrinfo *air = nullptr, *ai, hints = { 0 };

	hints.ai_family = AF_UNSPEC;

	NetlibUrl &url = nlc->url;
	if (url.flags & NLOCF_UDP) {
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
	}
	else {
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}

	if (nlc->proxyType) {
		if (!nlc->szProxyServer)
			return false;

		Netlib_Logf(nlu, "(%p) Connecting to proxy %s:%d for %s:%d ....", nlc, nlc->szProxyServer, nlc->wProxyPort, url.szHost.c_str(), url.port);

		_itoa(nlc->wProxyPort, szPort, 10);
		if (GetAddrInfoA(nlc->szProxyServer, szPort, &hints, &air)) {
			Netlib_Logf(nlu, "%s %d: %s() for host %s failed (%u)", __FILE__, __LINE__, "getaddrinfo", nlc->szProxyServer, WSAGetLastError());
			return false;
		}
	}
	else {
		if (url.szHost.IsEmpty())
			return false;

		Netlib_Logf(nlu, "(%p) Connecting to server %s:%d....", nlc, url.szHost.c_str(), url.port);

		_itoa(url.port, szPort, 10);

		if (GetAddrInfoA(url.szHost, szPort, &hints, &air)) {
			Netlib_Logf(nlu, "%s %d: %s() for host %s failed (%u)", __FILE__, __LINE__, "getaddrinfo", url.szHost.c_str(), WSAGetLastError());
			return false;
		}
	}

	for (ai = air; ai && !Miranda_IsTerminated(); ai = ai->ai_next) {
		Netlib_Logf(nlu, "(%p) Connecting to ip %s ....", nlc, ptrA(Netlib_AddressToString((sockaddr_in*)ai->ai_addr)).get());
retry:
		nlc->s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (nlc->s == INVALID_SOCKET) {
			FreeAddrInfoA(air);
			return false;
		}

		// return the socket to non blocking
		if (ioctlsocket(nlc->s, FIONBIO, &notblocking) != 0) {
			FreeAddrInfoA(air);
			return false;
		}

		if (nlu->settings.specifyOutgoingPorts && nlu->settings.szOutgoingPorts  && nlu->settings.szOutgoingPorts[0]) {
			SOCKET s = ai->ai_family == AF_INET ? nlc->s : INVALID_SOCKET;
			SOCKET s6 = ai->ai_family == AF_INET6 ? nlc->s : INVALID_SOCKET;
			if (!BindSocketToPort(nlu->settings.szOutgoingPorts, s, s6, &nlu->inportnum))
				Netlib_Logf(nlu, "Netlib connect: Not enough ports for outgoing connections specified");
		}

		// try a connect
		if (connect(nlc->s, ai->ai_addr, (int)ai->ai_addrlen) == 0) {
			rc = 0;
			break;
		}

		// didn't work, was it cos of nonblocking?
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			rc = SOCKET_ERROR;
			closesocket(nlc->s);
			nlc->s = INVALID_SOCKET;
			continue;
		}

		while (true) { // timeout loop
			fd_set r, w, e;
			FD_ZERO(&r); FD_ZERO(&w); FD_ZERO(&e);
			FD_SET(nlc->s, &r);
			FD_SET(nlc->s, &w);
			FD_SET(nlc->s, &e);
			if ((rc = select(0, &r, &w, &e, &tv)) == SOCKET_ERROR)
				break;

			if (rc > 0) {
				if (FD_ISSET(nlc->s, &w)) {
					// connection was successful
					rc = 0;
					lasterr = 0;
				}
				if (FD_ISSET(nlc->s, &r)) {
					// connection was closed
					rc = SOCKET_ERROR;
					lasterr = WSAECONNRESET;
				}
				if (FD_ISSET(nlc->s, &e)) {
					// connection failed.
					int len = sizeof(lasterr);
					rc = SOCKET_ERROR;
					getsockopt(nlc->s, SOL_SOCKET, SO_ERROR, (char*)&lasterr, &len);
					if (lasterr == WSAEADDRINUSE && ++retrycnt <= 2) {
						closesocket(nlc->s);
						nlc->s = INVALID_SOCKET;
						goto retry;
					}
				}
				break;
			}
			else if (Miranda_IsTerminated()) {
				rc = SOCKET_ERROR;
				lasterr = ERROR_TIMEOUT;
				break;
			}

			if (--timeout == 0) {
				rc = SOCKET_ERROR;
				lasterr = ERROR_TIMEOUT;
				break;
			}
		}

		if (rc == 0) break;

		closesocket(nlc->s);
		nlc->s = INVALID_SOCKET;
	}

	FreeAddrInfoA(air);

	notblocking = 0;
	if (nlc->s != INVALID_SOCKET) ioctlsocket(nlc->s, FIONBIO, &notblocking);
	if (rc && lasterr) SetLastError(lasterr);
	return rc == 0;
}

static int NetlibHttpFallbackToDirect(NetlibConnection *nlc)
{
	NetlibDoCloseSocket(nlc, true);

	Netlib_Logf(nlc->nlu, "Fallback to direct connection");

	nlc->proxyAuthNeeded = false;
	nlc->proxyType = 0;
	replaceStr(nlc->szProxyServer, nullptr);
	if (!my_connectIP(nlc)) {
		Netlib_Logf(nlc->nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "connect", WSAGetLastError());
		return false;
	}
	return true;
}

bool NetlibDoConnect(NetlibConnection *nlc)
{
	NetlibUser *nlu = nlc->nlu;

	replaceStr(nlc->szProxyServer, nullptr);

	bool usingProxy = false, forceHttps = false;
	if (nlu->settings.useProxy) {
		if (nlu->settings.proxyType == PROXYTYPE_IE)
			usingProxy = NetlibGetIeProxyConn(nlc, false);
		else {
			if (nlu->settings.szProxyServer && nlu->settings.szProxyServer[0]) {
				nlc->szProxyServer = mir_strdup(nlu->settings.szProxyServer);
				nlc->wProxyPort = nlu->settings.wProxyPort;
				nlc->proxyType = nlu->settings.proxyType;
				usingProxy = true;
			}
		}
	}

	while (!my_connectIP(nlc)) {
		// if connection failed, the state of nlc might be unpredictable
		if (GetNetlibHandleType(nlc) == NLH_CONNECTION) {
			// Fallback to direct only when using HTTP proxy, as this is what used by companies
			// If other type of proxy used it's an indication of security nutcase, leave him alone
			if (usingProxy && (nlc->proxyType == PROXYTYPE_HTTPS || nlc->proxyType == PROXYTYPE_HTTP)) {
				usingProxy = false;
				nlc->proxyType = 0;
				Netlib_Logf(nlu, "Fallback to direct connection");
				continue;
			}
			if (nlu->settings.useProxy && !usingProxy && nlu->settings.proxyType == PROXYTYPE_IE && !forceHttps) {
				forceHttps = true;
				usingProxy = NetlibGetIeProxyConn(nlc, true);
				if (usingProxy)
					continue;
			}
		}
		Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "connect", WSAGetLastError());
		return false;
	}

	if (usingProxy && !((nlc->url.flags & (NLOCF_HTTP | NLOCF_SSL)) == NLOCF_HTTP && (nlc->proxyType == PROXYTYPE_HTTP || nlc->proxyType == PROXYTYPE_HTTPS))) {
		if (!WaitUntilWritable(nlc->s, 30000))
			return false;

		switch (nlc->proxyType) {
		case PROXYTYPE_SOCKS4:
			if (!NetlibInitSocks4Connection(nlc))
				return false;
			break;

		case PROXYTYPE_SOCKS5:
			if (!NetlibInitSocks5Connection(nlc))
				return false;
			break;

		case PROXYTYPE_HTTPS:
		case PROXYTYPE_HTTP:
			nlc->proxyAuthNeeded = true;
			if (!NetlibInitHttpsConnection(nlc)) {
				usingProxy = false;
				if (!NetlibHttpFallbackToDirect(nlc))
					return false;
			}
			break;

		default:
			SetLastError(ERROR_INVALID_PARAMETER);
			FreePartiallyInitedConnection(nlc);
			return false;
		}
	}

	Netlib_Logf(nlu, "(%d) Connected to %s:%d", nlc->s, nlc->url.szHost.c_str(), nlc->url.port);

	if (GetSubscribersCount((THook*)hEventConnected)) {
		NETLIBCONNECTIONEVENTINFO ncei = {};
		ncei.connected = 1;
		ncei.szSettingsModule = nlu->user.szSettingsModule;
		int size = sizeof(SOCKADDR_IN);
		getsockname(nlc->s, (SOCKADDR *)&ncei.local, &size);
		if (nlu->settings.useProxy) {
			size = sizeof(SOCKADDR_IN);
			getpeername(nlc->s, (SOCKADDR *)&ncei.proxy, &size);
			ncei.remote.sin_family = AF_INET;
			ncei.remote.sin_port = htons(nlc->url.port);
			ncei.remote.sin_addr.S_un.S_addr = DnsLookup(nlu, nlc->url.szHost);
		}
		else {
			size = sizeof(SOCKADDR_IN);
			getpeername(nlc->s, (SOCKADDR *)&ncei.remote, &size);
		}
		NotifyFastHook(hEventConnected, (WPARAM)&ncei, 0);
	}

	if (NLOCF_SSL & nlc->url.flags)
		return Netlib_StartSsl(nlc, nullptr) != 0;

	return true;
}

bool NetlibReconnect(NetlibConnection *nlc)
{
	// a connection might be freed already
	if (GetNetlibHandleType(nlc) != NLH_CONNECTION)
		return false;

	char buf[4];
	bool opened = nlc->s != INVALID_SOCKET;
	if (opened) {
		switch (WaitUntilReadable(nlc->s, 0, true)) {
		case SOCKET_ERROR:
			opened = false;
			break;

		case 0:
			opened = true;
			break;

		case 1:
			opened = recv(nlc->s, buf, 1, MSG_PEEK) > 0;
			break;
		}

		if (!opened)
			NetlibDoCloseSocket(nlc, true);
	}

	if (!opened) {
		if (Miranda_IsTerminated())
			return false;

		return NetlibDoConnect(nlc);
	}
	return true;
}

MIR_APP_DLL(HNETLIBCONN) Netlib_OpenConnection(NetlibUser *nlu, const char *szHost, int port, int timeout, int flags)
{
	if (szHost == nullptr || port == 0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}

	if (GetNetlibHandleType(nlu) != NLH_USER || !(nlu->user.flags & NUF_OUTGOING))
		return nullptr;

	Netlib_Logf(nlu, "Connection request to %s:%d (Flags %x)....", szHost, port, flags);

	NetlibConnection *nlc = new NetlibConnection();
	nlc->nlu = nlu;
	nlc->timeout = timeout;
	nlc->url.szHost = szHost;
	nlc->url.port = port;
	nlc->url.flags = flags;
	nlc->dnsThroughProxy = nlu->settings.dnsThroughProxy != 0;

	if (!NetlibDoConnect(nlc)) {
		FreePartiallyInitedConnection(nlc);
		return nullptr;
	}

	if (iUPnPCleanup == 0) {
		mir_cslock lck(csNetlibUser);
		iUPnPCleanup = 1;
		mir_forkthread(NetlibUPnPCleanup);
	}

	return nlc;
}

NetlibConnection::NetlibConnection()
{
	handleType = NLH_CONNECTION;
	s = s2 = INVALID_SOCKET;
	hOkToCloseEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	NetlibInitializeNestedCS(&ncsSend);
	NetlibInitializeNestedCS(&ncsRecv);
}

NetlibConnection::~NetlibConnection()
{
	handleType = 0;

	if (s != INVALID_SOCKET)
		closesocket(s);

	mir_free(szNewUrl);
	mir_free(szProxyServer);

	mir_free(nlhpi.szHttpPostUrl);
	mir_free(nlhpi.szHttpGetUrl);

	NetlibDeleteNestedCS(&ncsSend);
	NetlibDeleteNestedCS(&ncsRecv);

	CloseHandle(hOkToCloseEvent);
}
