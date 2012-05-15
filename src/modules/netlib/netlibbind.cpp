/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "commonheaders.h"
#include "netlib.h"

bool BindSocketToPort(const char *szPorts, SOCKET s, int* portn)
{
    SOCKADDR_IN sin = {0};
   	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	EnterCriticalSection(&csNetlibUser);

    if (--*portn < 0 && s != INVALID_SOCKET)
    {
        BindSocketToPort(szPorts, INVALID_SOCKET, portn);
        if (*portn == 0)
        {
            LeaveCriticalSection(&csNetlibUser);
            return false;
        }
        WORD num;
        CallService(MS_UTILS_GETRANDOM, sizeof(WORD), (LPARAM)&num);
        *portn = num % *portn;
    }

    bool before=false;
    for (;;)
    {
	    const char *psz;
	    char *pszEnd;
	    int portMin, portMax, port, portnum = 0;

        for(psz=szPorts;*psz;) 
		{
	        while (*psz == ' ' || *psz == ',') psz++;
	        portMin = strtol(psz, &pszEnd, 0);
	        if (pszEnd == psz) break;
	        while (*pszEnd == ' ') pszEnd++;
	        if(*pszEnd == '-') 
			{
		        psz = pszEnd + 1;
		        portMax = strtol(psz, &pszEnd, 0);
		        if (pszEnd == psz) portMax = 65535;
		        if (portMin > portMax) 
				{
			        port = portMin;
			        portMin = portMax;
			        portMax = port;
		        }
	        }
	        else portMax = portMin;
	        if (portMax >= 1) 
            {
		        if (portMin <= 0) portMin = 1;
		        for (port = portMin; port <= portMax; port++) 
                {
			        if (port > 65535) break;

                    ++portnum;

                    if (s == INVALID_SOCKET) continue;
                    if (!before && portnum <= *portn) continue;
                    if (before  && portnum >= *portn) 
                    {
	                    LeaveCriticalSection(&csNetlibUser);
                        return false;
                    }

                    sin.sin_port = htons((WORD)port);
   		            if (bind(s, (SOCKADDR*)&sin, sizeof(sin)) == 0) 
                    {
	                    LeaveCriticalSection(&csNetlibUser);
                        *portn = portnum + 1;
                        return true;
                    }
		        }
	        }
	        psz = pszEnd;
        }
        if (*portn < 0) 
        {
           *portn = portnum;
	        LeaveCriticalSection(&csNetlibUser);
            return true;
        }
        else if (*portn >= portnum)
            *portn = 0;
        else
            before = true;
   }
}


int NetlibFreeBoundPort(struct NetlibBoundPort *nlbp)
{
	closesocket(nlbp->s);
	WaitForSingleObject(nlbp->hThread,INFINITE);
	CloseHandle(nlbp->hThread);
	NetlibLogf(nlbp->nlu, "(%u) Port %u closed for incoming connections", nlbp->s, nlbp->wPort);
	mir_free(nlbp);
	return 1;
}

static unsigned __stdcall NetlibBindAcceptThread(void* param)
{
	SOCKET s;
	SOCKADDR_IN sin;
	int sinLen;
	struct NetlibConnection *nlc;
	struct NetlibBoundPort *nlbp = (NetlibBoundPort*)param;

	NetlibLogf(nlbp->nlu, "(%u) Port %u opened for incoming connections", nlbp->s, nlbp->wPort);
	for(;;) 
	{
		sinLen = sizeof(sin);
		s = accept(nlbp->s, (struct sockaddr*)&sin, &sinLen);
		if (s == INVALID_SOCKET) break;
		NetlibLogf(nlbp->nlu, "New incoming connection on port %u from %s (%d)", nlbp->wPort, inet_ntoa(sin.sin_addr), s);
		nlc = (NetlibConnection*)mir_calloc(sizeof(NetlibConnection));
		nlc->handleType = NLH_CONNECTION;
		nlc->nlu = nlbp->nlu;
		nlc->s = s;
		nlc->s2 = INVALID_SOCKET;
		InitializeCriticalSection(&nlc->csHttpSequenceNums);
		nlc->hOkToCloseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		nlc->dontCloseNow = 0;
		NetlibInitializeNestedCS(&nlc->ncsSend);
		NetlibInitializeNestedCS(&nlc->ncsRecv);
		nlbp->pfnNewConnectionV2((HANDLE)nlc,ntohl(sin.sin_addr.S_un.S_addr), nlbp->pExtra);
	}
	NetlibUPnPDeletePortMapping(nlbp->wExPort, "TCP");
	return 0;
}

INT_PTR NetlibBindPort(WPARAM wParam,LPARAM lParam)
{
	NETLIBBIND *nlb = (NETLIBBIND*)lParam;
	struct NetlibUser *nlu = (struct NetlibUser*)wParam;
	struct NetlibBoundPort *nlbp;
	SOCKADDR_IN sin;
	int foundPort = 0;
	UINT dwThreadId;

	if (GetNetlibHandleType(nlu) != NLH_USER || !(nlu->user.flags & NUF_INCOMING) ||
		nlb == NULL || nlb->pfnNewConnection == NULL) 
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if (nlb->cbSize != sizeof(NETLIBBIND)   &&
		nlb->cbSize != NETLIBBIND_SIZEOF_V2 &&
		nlb->cbSize != NETLIBBIND_SIZEOF_V1)
	{
		return 0;
	}
	nlbp = (NetlibBoundPort*)mir_calloc(sizeof(NetlibBoundPort));
	nlbp->handleType = NLH_BOUNDPORT;
	nlbp->nlu = nlu;
	nlbp->pfnNewConnectionV2 = nlb->pfnNewConnectionV2;
	nlbp->s = socket(AF_INET, SOCK_STREAM, 0);
	nlbp->pExtra = (nlb->cbSize != NETLIBBIND_SIZEOF_V1) ? nlb->pExtra : NULL;
	if (nlbp->s == INVALID_SOCKET) 
	{
		NetlibLogf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"socket",WSAGetLastError());
		mir_free(nlbp);
		return 0;
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = 0;

	/* if the netlib user wanted a free port given in the range, then
	they better have given wPort==0, let's hope so */
	if (nlu->settings.specifyIncomingPorts && nlu->settings.szIncomingPorts && nlb->wPort == 0) 
	{
		if (!BindSocketToPort(nlu->settings.szIncomingPorts, nlbp->s, &nlu->outportnum))
		{
			NetlibLogf(nlu, "Netlib bind: Not enough ports for incoming connections specified");
			SetLastError(WSAEADDRINUSE);
		}
		else
			foundPort = 1;
	}
	else 
	{
		/* if ->wPort==0 then they'll get any free port, otherwise they'll
		be asking for whatever was in nlb->wPort*/
		if (nlb->wPort != 0) 
		{
			NetlibLogf(nlu,"%s %d: trying to bind port %d, this 'feature' can be abused, please be sure you want to allow it.",__FILE__,__LINE__,nlb->wPort);
			sin.sin_port = htons(nlb->wPort);
		}
		if (bind(nlbp->s, (PSOCKADDR)&sin, sizeof(sin)) == 0) 
			foundPort = 1;
	}
	if (!foundPort) 
	{
		NetlibLogf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"bind",WSAGetLastError());
		closesocket(nlbp->s);
		mir_free(nlbp);
		return 0;
	}

	if (listen(nlbp->s, 5)) 
	{
		NetlibLogf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"listen",WSAGetLastError());
		closesocket(nlbp->s);
		mir_free(nlbp);
		return 0;
	}

	{	int len;
		DWORD extIP;

		ZeroMemory(&sin,sizeof(sin));
		len = sizeof(sin);
		if (getsockname(nlbp->s,(SOCKADDR *)&sin,&len))
		{
			NetlibLogf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"getsockname",WSAGetLastError());
			closesocket(nlbp->s);
			mir_free(nlbp);
			return 0;
		}
		nlb->wPort = ntohs(sin.sin_port);
		nlbp->wPort = nlb->wPort;
		nlb->dwInternalIP = ntohl(sin.sin_addr.S_un.S_addr);

		if (nlb->dwInternalIP == 0)
		{
			char hostname[64];
			struct hostent *he;

			gethostname(hostname, SIZEOF(hostname));
			he = gethostbyname(hostname);
			if (he && he->h_addr_list[0])
				nlb->dwInternalIP = ntohl(*(PDWORD)he->h_addr_list[0]);
		}
		if (nlu->settings.enableUPnP && 
			NetlibUPnPAddPortMapping(nlb->wPort, "TCP", &nlbp->wExPort, &extIP, nlb->cbSize > NETLIBBIND_SIZEOF_V2))
		{
			NetlibLogf(NULL, "UPnP port mapping succeeded. Internal Port: %u External Port: %u\n", 
				nlb->wPort, nlbp->wExPort); 
			if (nlb->cbSize > NETLIBBIND_SIZEOF_V2)
			{
				nlb->wExPort = nlbp->wExPort;
				nlb->dwExternalIP = extIP;
			}
		}
		else
		{
			if (nlu->settings.enableUPnP)
				NetlibLogf(NULL, "UPnP port mapping failed. Internal Port: %u\n", nlb->wPort); 
			else
				NetlibLogf(NULL, "UPnP disabled. Internal Port: %u\n", nlb->wPort); 

			nlbp->wExPort = 0;
			if (nlb->cbSize > NETLIBBIND_SIZEOF_V2)
			{
				nlb->wExPort = nlb->wPort;
				nlb->dwExternalIP = nlb->dwInternalIP;
			}
		}
	}
	nlbp->hThread = (HANDLE)forkthreadex(NULL, 0, NetlibBindAcceptThread, 0, nlbp, &dwThreadId);
	return (INT_PTR)nlbp;
}
