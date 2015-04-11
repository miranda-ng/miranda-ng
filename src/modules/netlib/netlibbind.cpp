/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "netlib.h"

bool BindSocketToPort(const char *szPorts, SOCKET s, SOCKET s6, int* portn)
{
	SOCKADDR_IN sin = {0};
	sin.sin_family = AF_INET;

	SOCKADDR_IN6 sin6 = {0};
	sin6.sin6_family = AF_INET6;

	mir_cslock lck(csNetlibUser);

	if (--*portn < 0 && (s != INVALID_SOCKET || s6 != INVALID_SOCKET)) {
		BindSocketToPort(szPorts, INVALID_SOCKET, INVALID_SOCKET, portn);
		if (*portn == 0)
			return false;

		WORD num;
		CallService(MS_UTILS_GETRANDOM, sizeof(WORD), (LPARAM)&num);
		*portn = num % *portn;
	}

	bool before = false;
	while (true) {
		const char *psz;
		char *pszEnd;
		int portMin, portMax, port, portnum = 0;

		for (psz = szPorts;*psz;) {
			while (*psz == ' ' || *psz == ',') psz++;
			portMin = strtol(psz, &pszEnd, 0);
			if (pszEnd == psz)
				break;
			while (*pszEnd == ' ')
				pszEnd++;
			if (*pszEnd == '-') {
				psz = pszEnd + 1;
				portMax = strtol(psz, &pszEnd, 0);
				if (pszEnd == psz) portMax = 65535;
				if (portMin > portMax) {
					port = portMin;
					portMin = portMax;
					portMax = port;
				}
			}
			else portMax = portMin;
			if (portMax >= 1) {
				if (portMin <= 0) portMin = 1;
				for (port = portMin; port <= portMax; port++) {
					if (port > 65535)
						break;

					++portnum;

					if (s == INVALID_SOCKET) continue;
					if (!before && portnum <= *portn) continue;
					if (before  && portnum >= *portn)
						return false;

					sin.sin_port = htons((WORD)port);
					bool bV4Mapped = s == INVALID_SOCKET || bind(s, (SOCKADDR*)&sin, sizeof(sin)) == 0;

					sin6.sin6_port = htons((WORD)port);
					bool bV6Mapped = s6 == INVALID_SOCKET || bind(s6, (PSOCKADDR)&sin6, sizeof(sin6)) == 0;

					if (bV4Mapped && bV6Mapped) {
						*portn = portnum + 1;
						return true;
					}
				}
			}
			psz = pszEnd;
		}

		if (*portn < 0) {
			*portn = portnum;
			return true;
		}

		if (*portn >= portnum)
			*portn = 0;
		else
			before = true;
	}
}

int NetlibFreeBoundPort(struct NetlibBoundPort *nlbp)
{
	closesocket(nlbp->s);
	closesocket(nlbp->s6);
	if (nlbp->hThread)
		WaitForSingleObject(nlbp->hThread, INFINITE);
	NetlibLogf(nlbp->nlu, "(%u) Port %u closed for incoming connections", nlbp->s, nlbp->wPort);
	mir_free(nlbp);
	return 1;
}

static void NetlibBindAcceptThread(void* param)
{
	NetlibBoundPort *nlbp = (NetlibBoundPort*)param;
	NetlibLogf(nlbp->nlu, "(%u) Port %u opened for incoming connections", nlbp->s, nlbp->wPort);

	while (true) {
		fd_set r;
		FD_ZERO(&r);
		if (nlbp->s != INVALID_SOCKET)
			FD_SET(nlbp->s, &r);
		if (nlbp->s6 != INVALID_SOCKET)
			FD_SET(nlbp->s6, &r);
		if (select(0, &r, NULL, NULL, NULL) == SOCKET_ERROR) {
			NetlibLogf(nlbp->nlu, "NetlibBindAcceptThread (%p): select failed (%d)", nlbp->s, GetLastError());
			break;
		}

		SOCKADDR_INET_M sin;
		int sinLen = sizeof(sin);
		memset(&sin, 0, sizeof(sin));

		SOCKET s;
		if (FD_ISSET(nlbp->s, &r)) {
			s = accept(nlbp->s, (sockaddr*)&sin, &sinLen);
			if (s == INVALID_SOCKET) {
				NetlibLogf(nlbp->nlu, "NetlibBindAcceptThread (%p): accept V4 failed (%d)", nlbp->s, GetLastError());
				break;
			}
		}
		else if (FD_ISSET(nlbp->s6, &r)) {
			s = accept(nlbp->s6, (sockaddr*)&sin, &sinLen);
			if (s == INVALID_SOCKET) {
				NetlibLogf(nlbp->nlu, "NetlibBindAcceptThread (%p): accept V6 failed (%d)", nlbp->s, GetLastError());
				break;
			}
		}

		NetlibLogf(nlbp->nlu, "New incoming connection on port %u from %s (%p)", nlbp->wPort, ptrA(NetlibAddressToString(&sin)), s);
		
		NetlibConnection *nlc = (NetlibConnection*)mir_calloc(sizeof(NetlibConnection));
		nlc->handleType = NLH_CONNECTION;
		nlc->nlu = nlbp->nlu;
		nlc->s = s;
		nlc->s2 = INVALID_SOCKET;
		InitializeCriticalSection(&nlc->csHttpSequenceNums);
		nlc->hOkToCloseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		NetlibInitializeNestedCS(&nlc->ncsSend);
		NetlibInitializeNestedCS(&nlc->ncsRecv);

		if (nlbp->pfnNewConnectionV2)
			nlbp->pfnNewConnectionV2(nlc, ntohl(sin.Ipv4.sin_addr.S_un.S_addr), nlbp->pExtra);
	}

	NetlibUPnPDeletePortMapping(nlbp->wExPort, "TCP");
	nlbp->hThread = 0;

	NetlibLogf(nlbp->nlu, "NetlibBindAcceptThread: (%p) thread for port %u closed", nlbp->s, nlbp->wPort);
}

INT_PTR NetlibBindPort(WPARAM wParam, LPARAM lParam)
{
	NETLIBBIND *nlb = (NETLIBBIND*)lParam;
	NetlibUser *nlu = (NetlibUser*)wParam;
	struct NetlibBoundPort *nlbp;
	SOCKADDR_IN sin = { 0 };
	SOCKADDR_IN6 sin6 = { 0 };
	int foundPort = 0;

	if (GetNetlibHandleType(nlu) != NLH_USER || !(nlu->user.flags & NUF_INCOMING) ||
		nlb == NULL || nlb->pfnNewConnection == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if (nlb->cbSize != sizeof(NETLIBBIND))
		return 0;

	nlbp = (NetlibBoundPort*)mir_calloc(sizeof(NetlibBoundPort));
	nlbp->handleType = NLH_BOUNDPORT;
	nlbp->nlu = nlu;
	nlbp->pfnNewConnectionV2 = nlb->pfnNewConnectionV2;

	nlbp->s = socket(PF_INET, SOCK_STREAM, 0);
	nlbp->s6 = socket(PF_INET6, SOCK_STREAM, 0);
	nlbp->pExtra = nlb->pExtra;
	if (nlbp->s == INVALID_SOCKET && nlbp->s6 == INVALID_SOCKET) {
		NetlibLogf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "socket", WSAGetLastError());
		mir_free(nlbp);
		return 0;
	}
	sin.sin_family = AF_INET;
	sin6.sin6_family = AF_INET6;

	/* if the netlib user wanted a free port given in the range, then
	they better have given wPort == 0, let's hope so */
	if (nlu->settings.specifyIncomingPorts && nlu->settings.szIncomingPorts && nlb->wPort == 0) {
		if (!BindSocketToPort(nlu->settings.szIncomingPorts, nlbp->s, nlbp->s6, &nlu->outportnum)) {
			NetlibLogf(nlu, "Netlib bind: Not enough ports for incoming connections specified");
			SetLastError(WSAEADDRINUSE);
		}
		else foundPort = 1;
	}
	else {
		/* if ->wPort == 0 then they'll get any free port, otherwise they'll
		be asking for whatever was in nlb->wPort*/
		if (nlb->wPort != 0) {
			NetlibLogf(nlu, "%s %d: trying to bind port %d, this 'feature' can be abused, please be sure you want to allow it.", __FILE__, __LINE__, nlb->wPort);
			sin.sin_port = htons(nlb->wPort);
			sin6.sin6_port = htons(nlb->wPort);
		}

		if (nlbp->s != INVALID_SOCKET)
		if (bind(nlbp->s, (PSOCKADDR)&sin, sizeof(sin)) == 0) {
			SOCKADDR_IN sin = { 0 };
			int len = sizeof(sin);
			if (!getsockname(nlbp->s, (PSOCKADDR)&sin, &len))
				sin6.sin6_port = sin.sin_port;
			foundPort = 1;
		}

		if (nlbp->s6 != INVALID_SOCKET)
			if (bind(nlbp->s6, (PSOCKADDR)&sin6, sizeof(sin6)) == 0)
				foundPort = 1;
	}
	if (!foundPort) {
		NetlibLogf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "bind", WSAGetLastError());
LBL_Error:
		closesocket(nlbp->s);
		closesocket(nlbp->s6);
		mir_free(nlbp);
		return 0;
	}

	if (nlbp->s != INVALID_SOCKET && listen(nlbp->s, 5)) {
		NetlibLogf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "listen", WSAGetLastError());
		goto LBL_Error;
	}

	if (nlbp->s6 != INVALID_SOCKET && listen(nlbp->s6, 5)) {
		NetlibLogf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "listen", WSAGetLastError());
		goto LBL_Error;
	}

	SOCKADDR_INET_M sinm = { 0 };
	int len = sizeof(sinm);
	if (!getsockname(nlbp->s, (PSOCKADDR)&sinm, &len)) {
		nlb->wPort = ntohs(sinm.Ipv4.sin_port);
		nlb->dwInternalIP = ntohl(sinm.Ipv4.sin_addr.S_un.S_addr);
	}
	else if (!getsockname(nlbp->s6, (PSOCKADDR)&sinm, &len))
		nlb->wPort = ntohs(sinm.Ipv6.sin6_port);
	else {
		NetlibLogf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "getsockname", WSAGetLastError());
		goto LBL_Error;
	}
	nlbp->wPort = nlb->wPort;

	if (nlb->dwInternalIP == 0) {
		char hostname[64] = "";
		gethostname(hostname, SIZEOF(hostname));

		PHOSTENT he = gethostbyname(hostname);
		if (he && he->h_addr)
			nlb->dwInternalIP = ntohl(*(PDWORD)he->h_addr);
	}

	DWORD extIP;
	if (nlu->settings.enableUPnP && NetlibUPnPAddPortMapping(nlb->wPort, "TCP", &nlbp->wExPort, &extIP, true)) {
		NetlibLogf(NULL, "UPnP port mapping succeeded. Internal Port: %u External Port: %u\n", nlb->wPort, nlbp->wExPort);
		nlb->wExPort = nlbp->wExPort;
		nlb->dwExternalIP = extIP;
	}
	else {
		if (nlu->settings.enableUPnP)
			NetlibLogf(NULL, "UPnP port mapping failed. Internal Port: %u\n", nlb->wPort);
		else
			NetlibLogf(NULL, "UPnP disabled. Internal Port: %u\n", nlb->wPort);

		nlbp->wExPort = 0;
		nlb->wExPort = nlb->wPort;
		nlb->dwExternalIP = nlb->dwInternalIP;
	}

	nlbp->hThread = mir_forkthread(NetlibBindAcceptThread, nlbp);
	return (INT_PTR)nlbp;
}
