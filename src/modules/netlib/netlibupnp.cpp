/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

static const char search_request_msg[] =
	"M-SEARCH * HTTP/1.1\r\n"
	"HOST: 239.255.255.250:1900\r\n"
	"MAN: \"ssdp:discover\"\r\n"
	"MX: 1\r\n"
	"ST: urn:schemas-upnp-org:service:%s\r\n"
	"\r\n";

static const char xml_get_hdr[] =
	"GET %s HTTP/1.1\r\n"
	"HOST: %s:%u\r\n"
	"ACCEPT-LANGUAGE: *\r\n\r\n";

static const char soap_post_hdr[] =
	"POST %s HTTP/1.1\r\n"
	"HOST: %s:%u\r\n"
	"CONTENT-LENGTH: %u\r\n"
	"CONTENT-TYPE: text/xml; charset = \"utf-8\"\r\n"
	"SOAPACTION: \"%s#%s\"\r\n\r\n"
	"%s";

static const char soap_post_hdr_m[] =
	"M-POST %s URL HTTP/1.1\r\n"
	"HOST: %s:%u\r\n"
	"CONTENT-LENGTH: %u\r\n"
	"CONTENT-TYPE: text/xml; charset = \"utf-8\"\r\n"
	"MAN: \"http://schemas.xmlsoap.org/soap/envelope/\"; ns = 01\r\n"
	"01-SOAPACTION: \"%s#%s\"\r\n\r\n"
	"%s";

static const char search_device[] =
	"<serviceType>%s</serviceType>";

static const char soap_action[] =
	"<?xml version = \"1.0\"?>\r\n"
	"<s:Envelope\r\n"
	"    xmlns:s = \"http://schemas.xmlsoap.org/soap/envelope/\"\r\n"
	"    s:encodingStyle = \"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
	"  <s:Body>\r\n"
	"    <u:%s xmlns:u = \"%s\">\r\n"
	"%s"
	"    </u:%s>\r\n"
	"  </s:Body>\r\n"
	"</s:Envelope>\r\n";

static const char soap_query[] =
	"<s:Envelope\r\n"
	"    xmlns:s = \"http://schemas.xmlsoap.org/soap/envelope/\"\r\n"
	"    s:encodingStyle = \"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
	"  <s:Body>\r\n"
	"    <u:QueryStateVariable xmlns:u = \"urn:schemas-upnp-org:control-1-0\">\r\n"
	"      <u:varName>%s</u:varName>\r\n"
	"    </u:QueryStateVariable>\r\n"
	"  </s:Body>\r\n"
	"</s:Envelope>\r\n";

static const char add_port_mapping[] =
	"      <NewRemoteHost></NewRemoteHost>\r\n"
	"      <NewExternalPort>%i</NewExternalPort>\r\n"
	"      <NewProtocol>%s</NewProtocol>\r\n"
	"      <NewInternalPort>%i</NewInternalPort>\r\n"
	"      <NewInternalClient>%s</NewInternalClient>\r\n"
	"      <NewEnabled>1</NewEnabled>\r\n"
	"      <NewPortMappingDescription>Miranda</NewPortMappingDescription>\r\n"
	"      <NewLeaseDuration>0</NewLeaseDuration>\r\n";

static const char delete_port_mapping[] =
	"     <NewRemoteHost></NewRemoteHost>\r\n"
	"     <NewExternalPort>%i</NewExternalPort>\r\n"
	"     <NewProtocol>%s</NewProtocol>\r\n";

static const char get_port_mapping[] =
	"     <NewPortMappingIndex>%i</NewPortMappingIndex>\r\n";

static bool gatewayFound;
static SOCKADDR_IN locIP;
static time_t lastDiscTime;
static int expireTime = 120;

static int retryCount;
static SOCKET sock = INVALID_SOCKET;
static char szConnHost[256];
static unsigned short sConnPort;

static WORD *portList;
static unsigned numports, numportsAlloc;
static HANDLE portListMutex;

static char szCtlUrl[256], szDev[256];

typedef enum
{
	DeviceGetReq,
	ControlAction,
	ControlQuery
} ReqType;

static bool txtParseParam(char* szData, char* presearch,
	char* start, char* finish, char* param, size_t size)
{
	char *cp, *cp1;
	size_t len;

	*param = 0;

	if (presearch != NULL) {
		cp1 = strstr(szData, presearch);
		if (cp1 == NULL) return false;
	}
	else
		cp1 = szData;

	cp = strstr(cp1, start);
	if (cp == NULL) return false;
	cp += strlen(start);
	while (*cp == ' ') ++cp;

	cp1 = strstr(cp, finish);
	if (cp1 == NULL) return false;
	while (*(cp1-1) == ' ' && cp1 > cp) --cp1;

	len = min((size_t)(cp1 - cp), size-1);
	strncpy(param, cp, len);
	param[len] = 0;

	return true;
}

void parseURL(char* szUrl, char* szHost, unsigned short* sPort, char* szPath)
{
	char *ppath, *phost, *pport;
	int sz;

	phost = strstr(szUrl, "://");
	if (phost == NULL) phost = szUrl;
	else phost += 3;

	ppath = strchr(phost, '/');
	if (ppath == NULL) ppath = phost + strlen(phost);

	pport = strchr(phost, ':');
	if (pport == NULL) pport = ppath;

	if (szHost != NULL)
	{
		sz = pport - phost + 1;
		if (sz>256) sz = 256;
		strncpy(szHost, phost, sz);
		szHost[sz-1] = 0;
	}

	if (sPort != NULL)
	{
		if (pport < ppath)
		{
			long prt = atol(pport+1);
			*sPort = prt != 0 ? (unsigned short)prt : 80;
		}
		else
			*sPort = 80;
	}

	if (szPath != NULL)
	{
		strncpy(szPath, ppath, 256);
		szPath[255] = 0;
	}
}

static void LongLog(char* szData)
{
	CallService(MS_NETLIB_LOG, 0, (LPARAM)szData);
}

static void closeRouterConnection(void)
{
	if (sock != INVALID_SOCKET)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

static void validateSocket(void)
{
	static const TIMEVAL tv = { 0, 0 };
	fd_set rfd;
	char buf[4];
	bool opened;

	if (sock == INVALID_SOCKET)
		return;

	FD_ZERO(&rfd);
	FD_SET(sock, &rfd);

	switch (select(1, &rfd, NULL, NULL, &tv))
	{
	case SOCKET_ERROR:
		opened = false;
		break;

	case 0:
		opened = true;
		break;

	case 1:
		opened = recv(sock, buf, 1, MSG_PEEK) > 0;
		break;
	}

	if (!opened)
		closeRouterConnection();
}

static int httpTransact(char* szUrl, char* szResult, int resSize, char* szActionName, ReqType reqtype)
{
	// Parse URL
	char szHost[256], szPath[256], szRes[16];
	int sz = 0, res = 0;
	unsigned short sPort;
	bool needClose;

	const char* szPostHdr = soap_post_hdr;
	char* szData = (char*)mir_alloc(4096);
	char* szReq = NULL;

	parseURL(szUrl, szHost, &sPort, szPath);

	if (sPort != sConnPort || _stricmp(szHost, szConnHost))
		closeRouterConnection();
	else
		validateSocket();

	while(true)
	{
		retryCount = 0;
		switch(reqtype)
		{
		case DeviceGetReq:
			sz = mir_snprintf (szData, 4096, xml_get_hdr, szPath, szHost, sPort);
			break;

		case ControlAction:
			{
				char szData1[1024];

				szReq = mir_strdup(szResult);
				sz = mir_snprintf (szData1, SIZEOF(szData1),
					soap_action, szActionName, szDev, szReq, szActionName);

				sz = mir_snprintf (szData, 4096,
					szPostHdr, szPath, szHost, sPort,
					sz, szDev, szActionName, szData1);
			}
			break;

		case ControlQuery:
			{
				char szData1[1024];

				sz = mir_snprintf (szData1, SIZEOF(szData1),
					soap_query, szActionName);

				sz = mir_snprintf (szData, 4096,
					szPostHdr, szPath, szHost, sPort,
					sz, "urn:schemas-upnp-org:control-1-0", "QueryStateVariable", szData1);
			}
			break;
		}
		szResult[0] = 0;
		{
			static const TIMEVAL tv = { 6, 0 };
			static unsigned ttl = 4;
			static u_long mode = 1;
			fd_set rfd, wfd, efd;
			SOCKADDR_IN enetaddr;

retrycon:
			if (sock == INVALID_SOCKET)
			{
				sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				enetaddr.sin_family = AF_INET;
				enetaddr.sin_port = htons(sPort);
				enetaddr.sin_addr.s_addr = inet_addr(szHost);

				// Resolve host name if needed
				if (enetaddr.sin_addr.s_addr == INADDR_NONE)
				{
					PHOSTENT he = gethostbyname(szHost);
					if (he)
						enetaddr.sin_addr.s_addr = *(unsigned*)he->h_addr_list[0];
				}

				NetlibLogf(NULL, "UPnP HTTP connection Host: %s Port: %u", szHost, sPort);

				FD_ZERO(&rfd); FD_ZERO(&wfd); FD_ZERO(&efd);
				FD_SET(sock, &rfd); FD_SET(sock, &wfd); FD_SET(sock, &efd);

				// Limit the scope of the connection (does not work for
				setsockopt(sock, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(unsigned));

				// Put socket into non-blocking mode for timeout on connect
				ioctlsocket(sock, FIONBIO, &mode);

				// Connect to the remote host
				if (connect(sock, (SOCKADDR*)&enetaddr, sizeof(enetaddr)) == SOCKET_ERROR)
				{
					int err = WSAGetLastError();

					// Socket connection failed
					if (err != WSAEWOULDBLOCK)
					{
						closeRouterConnection();
						NetlibLogf(NULL, "UPnP connect failed %d", err);
						break;
					}
					// Wait for socket to connect
					else if (select(1, &rfd, &wfd, &efd, &tv) != 1)
					{
						closeRouterConnection();
						NetlibLogf(NULL, "UPnP connect timeout");
						break;
					}
					else if (!FD_ISSET(sock, &wfd))
					{
						closeRouterConnection();
						NetlibLogf(NULL, "UPnP connect failed");
						break;
					}
				}
				strcpy(szConnHost, szHost); sConnPort = sPort;
			}

			if (send(sock, szData, sz, 0) != SOCKET_ERROR)
			{
				char *hdrend = NULL;
				int acksz = 0, pktsz = 0;

				if (szActionName == NULL)
				{
					int len = sizeof(locIP);
					getsockname(sock, (SOCKADDR*)&locIP, &len);
					if (locIP.sin_addr.S_un.S_addr == 0x0100007f)
					{
						struct hostent *he;

						gethostname(szPath, sizeof(szPath));
						he = gethostbyname(szPath);
						if (he != NULL)
							locIP.sin_addr.S_un.S_addr = *(PDWORD)he->h_addr_list[0];
					}
				}

				LongLog(szData);
				sz = 0;
				while(true)
				{
					int bytesRecv;

					FD_ZERO(&rfd);
					FD_SET(sock, &rfd);

					// Wait for the next packet
					if (select(1, &rfd, NULL, NULL, &tv) != 1)
					{
						closeRouterConnection();
						NetlibLogf(NULL, "UPnP recieve timeout");
						break;
					}

					//
					bytesRecv = recv(sock, &szResult[sz], resSize-sz, 0);

					// Connection closed or aborted, all data received
					if (bytesRecv == 0 || bytesRecv == SOCKET_ERROR)
					{
						closeRouterConnection();
						if ((bytesRecv == SOCKET_ERROR || sz == 0) && retryCount < 2)
						{
							++retryCount;
							goto retrycon;
						}
						break;
					}

					sz += bytesRecv;

					// Insert null terminator to use string functions
					if (sz >= (resSize-1))
					{
						szResult[resSize-1] = 0;
						break;
					}
					else
						szResult[sz] = 0;

					// HTTP header found?
					if (hdrend == NULL)
					{
						// Find HTTP header end
						hdrend = strstr(szResult, "\r\n\r\n");
						if (hdrend == NULL)
						{
							hdrend = strstr(szResult, "\n\n");
							if (hdrend) hdrend += 2;
						}

						else
							hdrend += 4;

						if (hdrend != NULL)
						{
							// Get packet size if provided
							if (txtParseParam(szResult, NULL, "Content-Length:", "\n", szRes, sizeof(szRes))  ||
								txtParseParam(szResult, NULL, "CONTENT-LENGTH:", "\n", szRes, sizeof(szRes)))
							{
								// Add size of HTTP header to the packet size to compute full transmission size
								pktsz = atol(ltrimp(szRes)) + (hdrend - szResult);
							}
							// Get encoding type if provided
							else if (txtParseParam(szResult, NULL, "Transfer-Encoding:", "\n", szRes, sizeof(szRes)))
							{
								if (_stricmp(lrtrimp(szRes), "Chunked") == 0)
									acksz = hdrend - szResult;
							}
							if (txtParseParam(szResult, NULL, "Connection:", "\n", szRes, sizeof(szRes)))
							{
								needClose = (_stricmp(lrtrimp(szRes), "close") == 0);
							}
						}
					}

					// Content-Length bytes reached, all data received
					if (sz >= pktsz && pktsz != 0)
					{
						szResult[pktsz] = 0;
						break;
					}

					// Chunked encoding processing
					if (sz > acksz && acksz != 0)
					{
retry:
						// Parse out chunk size
						char* data = szResult + acksz;
						char* peol1 = data == hdrend ? data - 1 : strchr(data, '\n');
						if (peol1 != NULL)
						{
							char *peol2 = strchr(++peol1, '\n');
							if (peol2 != NULL)
							{
								// Get chunk size
								int chunkBytes = strtol(peol1, NULL, 16);
								acksz += chunkBytes;
								peol2++;

								memmove(data, peol2, strlen(peol2) + 1);
								sz -= peol2 - data;

								// Last chunk, all data received
								if (chunkBytes == 0) break;
								if (sz > acksz) goto retry;
							}
						}
					}
				}
				LongLog(szResult);
			}
			else
			{
				if (retryCount < 2)
				{
					closeRouterConnection();
					++retryCount;
					goto retrycon;
				}
				else
					NetlibLogf(NULL, "UPnP send failed %d", WSAGetLastError());
			}
		}
		txtParseParam(szResult, "HTTP", " ", " ", szRes, sizeof(szRes));
		res = atol(szRes);
		if (szActionName != NULL && res == 405 && szPostHdr == soap_post_hdr)
			szPostHdr = soap_post_hdr_m;
		else
			break;
	}

	if (needClose)
		closeRouterConnection();

	mir_free(szData);
	mir_free(szReq);
	return res;
}

static unsigned getExtIP(void)
{
	char szExtIP[30];
	char* szData = (char*)mir_alloc(4096); szData[0] = 0;

	unsigned extip = 0;
	int res = httpTransact(szCtlUrl, szData, 4096, "GetExternalIPAddress", ControlAction);
	if (res == 200 && txtParseParam(szData, "<NewExternalIPAddress", ">", "<", szExtIP, sizeof(szExtIP)))
		extip = ntohl(inet_addr(szExtIP));

	mir_free(szData);
	return extip;
}

static bool getUPnPURLs(char* szUrl, size_t sizeUrl)
{
	char* szData = (char*)mir_alloc(8192);

	gatewayFound = httpTransact(szUrl, szData, 8192, NULL, DeviceGetReq) == 200;
	if (gatewayFound)
	{
		char szTemp[256], *rpth;
		size_t ctlLen;

		txtParseParam(szData, NULL, "<URLBase>", "</URLBase>", szTemp, sizeof(szTemp));
		strncpy(szCtlUrl, szTemp[0] ? szTemp : szUrl, sizeof(szCtlUrl));
		szCtlUrl[sizeof(szCtlUrl)-1] = 0;

		mir_snprintf(szTemp, SIZEOF(szTemp), search_device, szDev);
		txtParseParam(szData, szTemp, "<controlURL>", "</controlURL>", szUrl, sizeUrl);

		// URL combining per RFC 2396
		if (szUrl[0] != 0)
		{
			if (strstr(szUrl, "://") != NULL)                     // absolute URI
				rpth = szCtlUrl;
			else if (strncmp(szUrl, "//", 2) == 0)                // relative URI net_path
			{
				rpth = strstr(szCtlUrl, "//");
				if (rpth == NULL) rpth = szCtlUrl;
			}
			else if (szUrl[0] == '/')                             // relative URI abs_path
			{
				rpth = strstr(szCtlUrl, "//");
				rpth = rpth ? rpth + 2 : szCtlUrl;

				rpth = strchr(rpth, '/');
				if (rpth == NULL) rpth = szCtlUrl + strlen(szCtlUrl);
			}
			else
			{                                                      // relative URI rel_path
				size_t ctlCLen = strlen(szCtlUrl);
				rpth = szCtlUrl + ctlCLen;
				if (ctlCLen != 0 && *(rpth-1) != '/')
					strncpy(rpth++, "/", sizeof(szCtlUrl) - ctlCLen);
			}

			ctlLen = sizeof(szCtlUrl) - (rpth - szCtlUrl);
			strncpy(rpth, szUrl, ctlLen);
			szCtlUrl[sizeof(szCtlUrl)-1] = 0;
		}
		else
		{
			szCtlUrl[0] = 0;
			gatewayFound = false;
		}
	}
	mir_free(szData);

	return gatewayFound;
}

static void discoverUPnP(void)
{
	char* buf;
	int buflen;
	unsigned i, j, nip = 0;
	unsigned* ips = NULL;

	static const unsigned any = INADDR_ANY;
	static const TIMEVAL tv = { 1, 600000 };

	char szUrl[256] = "";
	char hostname[256];
	PHOSTENT he;
	fd_set readfd;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	SOCKADDR_IN enetaddr;
	enetaddr.sin_family = AF_INET;
	enetaddr.sin_port = htons(1900);
	enetaddr.sin_addr.s_addr = inet_addr("239.255.255.250");

	gethostname(hostname, sizeof(hostname));
	he = gethostbyname(hostname);

	if (he)
	{
		while (he->h_addr_list[nip]) ++nip;

		ips = (unsigned*)mir_alloc(nip * sizeof(unsigned));

		for (j = 0; j < nip; j++)
			ips[j] = *(unsigned*)he->h_addr_list[j];
	}

	buf = (char*)mir_alloc(1500);

	for (i = 3;  --i && szUrl[0] == 0;)
	{
		for (j = 0; j < nip; j++)
		{
			if (ips)
				setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&ips[j], sizeof(unsigned));

			buflen = mir_snprintf(buf, 1500, search_request_msg, "WANIPConnection:1");
			sendto(sock, buf, buflen, 0, (SOCKADDR*)&enetaddr, sizeof(enetaddr));
			LongLog(buf);

			buflen = mir_snprintf(buf, 1500, search_request_msg, "WANPPPConnection:1");
			sendto(sock, buf, buflen, 0, (SOCKADDR*)&enetaddr, sizeof(enetaddr));
			LongLog(buf);
		}

		if (Miranda_Terminated()) break;

		FD_ZERO(&readfd);
		FD_SET(sock, &readfd);

		while (select(1, &readfd, NULL, NULL, &tv) >= 1)
		{
			buflen = recv(sock, buf, 1500, 0);
			if (buflen != SOCKET_ERROR)
			{
				buf[buflen] = 0;
				LongLog(buf);

				if (txtParseParam(buf, NULL, "LOCATION:", "\n", szUrl, sizeof(szUrl))  ||
					txtParseParam(buf, NULL, "Location:", "\n", szUrl, sizeof(szUrl)))
				{
					char age[30];
					char szHostNew[256], szHostExist[256];

					lrtrim(szUrl);

					parseURL(szUrl, szHostNew, NULL, NULL);
					parseURL(szCtlUrl, szHostExist, NULL, NULL);
					if (strcmp(szHostNew, szHostExist) == 0)
					{
						gatewayFound = true;
						break;
					}

					txtParseParam(buf, NULL, "ST:", "\n", szDev, sizeof(szDev));
					txtParseParam(buf, "max-age", " = ", "\n", age, sizeof(age));
					expireTime = atoi(lrtrimp(age));
					lrtrim(szDev);

					if (getUPnPURLs(szUrl, sizeof(szUrl)))
					{
						gatewayFound = getExtIP() != 0;
						if (gatewayFound) break;
					}
				}
			}
			FD_ZERO(&readfd);
			FD_SET(sock, &readfd);
		}
	}

	mir_free(buf);
	mir_free(ips);
	setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&any, sizeof(unsigned));
	closesocket(sock);
}

static bool findUPnPGateway(void)
{
	if ((time(NULL) - lastDiscTime) >= expireTime)
	{
		WaitForSingleObject(portListMutex, INFINITE);

		time_t curTime = time(NULL);

		if ((curTime - lastDiscTime) >= expireTime)
		{
			gatewayFound = false;

			discoverUPnP();
			lastDiscTime = curTime;

			NetlibLogf(NULL, "UPnP Gateway detected %d, Control URL: %s", gatewayFound, szCtlUrl);
		}

		ReleaseMutex(portListMutex);
	}

	return gatewayFound;
}

bool NetlibUPnPAddPortMapping(WORD intport, char *proto, WORD *extport, DWORD *extip, bool search)
{
	int res = 0, i = 5;

	if (findUPnPGateway())
	{
		char* szData = (char*)mir_alloc(4096);
		char szExtIP[30];

		*extport = intport - 1;
		*extip = ntohl(locIP.sin_addr.S_un.S_addr);

		WaitForSingleObject(portListMutex, INFINITE);

		do
		{
			++*extport;
			mir_snprintf(szData, 4096, add_port_mapping,
				*extport, proto, intport, inet_ntoa(locIP.sin_addr));
			res = httpTransact(szCtlUrl, szData, 4096, "AddPortMapping", ControlAction);
			txtParseParam(szData, NULL, "<errorCode>", "</errorCode>", szExtIP, sizeof(szExtIP));

		}
		while (search && res == 500 && atol(szExtIP) == 718 && --i);

		mir_free(szData);

		if (res == 200)
		{
			unsigned ip = getExtIP();
			if (ip) *extip = ip;

			if (numports >= numportsAlloc)
				mir_realloc(portList, sizeof(WORD)*(numportsAlloc += 10));
			portList[numports++] = *extport;
		}

		ReleaseMutex(portListMutex);
	}

	return res == 200;
}

void NetlibUPnPDeletePortMapping(WORD extport, char* proto)
{
	if (extport == 0)
		return;

	//	findUPnPGateway();

	if (gatewayFound)
	{
		unsigned i;
		char* szData = (char*)mir_alloc(4096);

		WaitForSingleObject(portListMutex, INFINITE);
		mir_snprintf(szData, 4096, delete_port_mapping, extport, proto);
		httpTransact(szCtlUrl, szData, 4096, "DeletePortMapping", ControlAction);

		for (i=0; i < numports; i++)
			if (portList[i] == extport && --numports > 0)
				memmove(&portList[i], &portList[i+1], (numports - i) * sizeof(WORD));

		mir_free(szData);
		ReleaseMutex(portListMutex);
	}
}

void NetlibUPnPCleanup(void*)
{
	if (db_get_b(NULL, "Netlib", "NLEnableUPnP", 1) == 0)
		// upnp is disabled globally, no need for a cleanup
		return;

	{
		int incoming = 0;
		mir_cslock lck(csNetlibUser);
		for (int i=0; i < netlibUser.getCount(); i++)
			if (netlibUser[i]->user.flags & NUF_INCOMING) {
				incoming = 1;
				break;
			}

		if (!incoming)
			return;
	}

	if (findUPnPGateway())
	{
		char* szData = (char*)alloca(4096);
		char buf[50], lip[50];
		unsigned i, j = 0, k, num = 100;

		WORD ports[30];

		strcpy(lip, inet_ntoa(locIP.sin_addr));

		WaitForSingleObject(portListMutex, INFINITE);

		if (httpTransact(szCtlUrl, szData, 4096, "PortMappingNumberOfEntries", ControlQuery) == 200 &&
			txtParseParam(szData, "QueryStateVariableResponse", "<return>", "<", buf, sizeof(buf)))
			num = atol(buf);

		for (i=0; i<num && !Miranda_Terminated(); i++)
		{
			mir_snprintf(szData, 4096, get_port_mapping, i);

			ReleaseMutex(portListMutex);
			WaitForSingleObject(portListMutex, INFINITE);

			if (httpTransact(szCtlUrl, szData, 4096, "GetGenericPortMappingEntry", ControlAction) != 200)
				break;

			if (!txtParseParam(szData, "<NewPortMappingDescription", ">", "<", buf, sizeof(buf)) || strcmp(buf, "Miranda") != 0)
				continue;

			if (!txtParseParam(szData, "<NewInternalClient", ">", "<", buf, sizeof(buf)) || strcmp(buf, lip) != 0)
				continue;

			if (txtParseParam(szData, "<NewExternalPort", ">", "<", buf, sizeof(buf)))
			{
				WORD mport = (WORD)atol(buf);

				if (j >= SIZEOF(ports))
					break;

				for (k = 0; k<numports; ++k)
					if (portList[k] == mport)
						break;

				if (k >= numports)
					ports[j++] = mport;
			}
		}

		ReleaseMutex(portListMutex);

		for (i=0; i<j && !Miranda_Terminated(); i++)
			NetlibUPnPDeletePortMapping(ports[i], "TCP");
	}
}

void NetlibUPnPInit(void)
{
	numports = 0;
	numportsAlloc = 10;
	portList = (WORD*)mir_alloc(sizeof(WORD)*numportsAlloc);

	portListMutex = CreateMutex(NULL, FALSE, NULL);
}

void NetlibUPnPDestroy(void)
{
	mir_free(portList);
	CloseHandle(portListMutex);
}
