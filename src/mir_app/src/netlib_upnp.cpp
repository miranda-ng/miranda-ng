/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

static uint16_t *portList;
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

	if (presearch != nullptr) {
		cp1 = strstr(szData, presearch);
		if (cp1 == nullptr) return false;
	}
	else
		cp1 = szData;

	cp = strstr(cp1, start);
	if (cp == nullptr) return false;
	cp += mir_strlen(start);
	while (*cp == ' ') ++cp;

	cp1 = strstr(cp, finish);
	if (cp1 == nullptr) return false;
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
	if (phost == nullptr) phost = szUrl;
	else phost += 3;

	ppath = strchr(phost, '/');
	if (ppath == nullptr) ppath = phost + mir_strlen(phost);

	pport = strchr(phost, ':');
	if (pport == nullptr) pport = ppath;

	if (szHost != nullptr) {
		sz = pport - phost + 1;
		if (sz > 256) sz = 256;
		strncpy(szHost, phost, sz);
		szHost[sz - 1] = 0;
	}

	if (sPort != nullptr) {
		if (pport < ppath) {
			long prt = atol(pport + 1);
			*sPort = prt != 0 ? (unsigned short)prt : 80;
		}
		else
			*sPort = 80;
	}

	if (szPath != nullptr) {
		strncpy(szPath, ppath, 256);
		szPath[255] = 0;
	}
}

static void LongLog(char* szData)
{
	Netlib_Logf(nullptr, szData);
}

static void closeRouterConnection(void)
{
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

static void validateSocket(void)
{
	static const TIMEVAL tv = { 0, 0 };
	fd_set rfd;
	char buf[4];

	if (sock == INVALID_SOCKET)
		return;

	FD_ZERO(&rfd);
	FD_SET(sock, &rfd);

	bool opened = false;
	switch (select(1, &rfd, nullptr, nullptr, &tv)) {
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
	bool needClose = false;

	const char* szPostHdr = soap_post_hdr;
	char* szData = (char*)mir_alloc(4096);
	char* szReq = nullptr;

	parseURL(szUrl, szHost, &sPort, szPath);

	if (sPort != sConnPort || _stricmp(szHost, szConnHost))
		closeRouterConnection();
	else
		validateSocket();

	while (true) {
		retryCount = 0;
		switch (reqtype) {
		case DeviceGetReq:
			sz = mir_snprintf(szData, 4096, xml_get_hdr, szPath, szHost, sPort);
			break;

		case ControlAction:
		{
			char szData1[1024];

			szReq = mir_strdup(szResult);
			sz = mir_snprintf(szData1, soap_action, szActionName, szDev, szReq, szActionName);
			sz = mir_snprintf(szData, 4096, szPostHdr, szPath, szHost, sPort, sz, szDev, szActionName, szData1);
		}
		break;

		case ControlQuery:
		{
			char szData1[1024];
			sz = mir_snprintf(szData1, soap_query, szActionName);
			sz = mir_snprintf(szData, 4096, szPostHdr, szPath, szHost, sPort, sz, "urn:schemas-upnp-org:control-1-0", "QueryStateVariable", szData1);
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
			if (sock == INVALID_SOCKET) {
				sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				enetaddr.sin_family = AF_INET;
				enetaddr.sin_port = htons(sPort);
				enetaddr.sin_addr.s_addr = inet_addr(szHost);

				// Resolve host name if needed
				if (enetaddr.sin_addr.s_addr == INADDR_NONE) {
					PHOSTENT he = gethostbyname(szHost);
					if (he)
						enetaddr.sin_addr.s_addr = *(unsigned*)he->h_addr_list[0];
				}

				Netlib_Logf(nullptr, "UPnP HTTP connection Host: %s Port: %u", szHost, sPort);

				FD_ZERO(&rfd); FD_ZERO(&wfd); FD_ZERO(&efd);
				FD_SET(sock, &rfd); FD_SET(sock, &wfd); FD_SET(sock, &efd);

				// Limit the scope of the connection (does not work for
				setsockopt(sock, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(unsigned));

				// Put socket into non-blocking mode for timeout on connect
				ioctlsocket(sock, FIONBIO, &mode);

				// Connect to the remote host
				if (connect(sock, (SOCKADDR*)&enetaddr, sizeof(enetaddr)) == SOCKET_ERROR) {
					int err = WSAGetLastError();

					// Socket connection failed
					if (err != WSAEWOULDBLOCK) {
						closeRouterConnection();
						Netlib_Logf(nullptr, "UPnP connect failed %d", err);
						break;
					}
					// Wait for socket to connect
					else if (select(1, &rfd, &wfd, &efd, &tv) != 1) {
						closeRouterConnection();
						Netlib_Logf(nullptr, "UPnP connect timeout");
						break;
					}
					else if (!FD_ISSET(sock, &wfd)) {
						closeRouterConnection();
						Netlib_Logf(nullptr, "UPnP connect failed");
						break;
					}
				}
				strncpy_s(szConnHost, szHost, _TRUNCATE);
				sConnPort = sPort;
			}

			if (send(sock, szData, sz, 0) != SOCKET_ERROR) {
				char *hdrend = nullptr;
				int acksz = 0, pktsz = 0;

				if (szActionName == nullptr) {
					int len = sizeof(locIP);
					getsockname(sock, (SOCKADDR*)&locIP, &len);
					if (locIP.sin_addr.S_un.S_addr == 0x0100007f) {
						struct hostent *he;

						gethostname(szPath, sizeof(szPath));
						he = gethostbyname(szPath);
						if (he != nullptr)
							locIP.sin_addr.S_un.S_addr = *(PDWORD)he->h_addr_list[0];
					}
				}

				LongLog(szData);
				sz = 0;
				while (true) {
					int bytesRecv;

					FD_ZERO(&rfd);
					FD_SET(sock, &rfd);

					// Wait for the next packet
					if (select(1, &rfd, nullptr, nullptr, &tv) != 1) {
						closeRouterConnection();
						Netlib_Logf(nullptr, "UPnP recieve timeout");
						break;
					}

					//
					bytesRecv = recv(sock, &szResult[sz], resSize - sz, 0);

					// Connection closed or aborted, all data received
					if (bytesRecv == 0 || bytesRecv == SOCKET_ERROR) {
						closeRouterConnection();
						if ((bytesRecv == SOCKET_ERROR || sz == 0) && retryCount < 2) {
							++retryCount;
							goto retrycon;
						}
						break;
					}

					sz += bytesRecv;

					// Insert null terminator to use string functions
					if (sz >= (resSize - 1)) {
						szResult[resSize - 1] = 0;
						break;
					}
					else
						szResult[sz] = 0;

					// HTTP header found?
					if (hdrend == nullptr) {
						// Find HTTP header end
						hdrend = strstr(szResult, "\r\n\r\n");
						if (hdrend == nullptr) {
							hdrend = strstr(szResult, "\n\n");
							if (hdrend) hdrend += 2;
						}

						else
							hdrend += 4;

						if (hdrend != nullptr) {
							// Get packet size if provided
							if (txtParseParam(szResult, nullptr, "Content-Length:", "\n", szRes, sizeof(szRes)) ||
								txtParseParam(szResult, nullptr, "CONTENT-LENGTH:", "\n", szRes, sizeof(szRes))) {
								// Add size of HTTP header to the packet size to compute full transmission size
								pktsz = atol(ltrimp(szRes)) + (hdrend - szResult);
							}
							// Get encoding type if provided
							else if (txtParseParam(szResult, nullptr, "Transfer-Encoding:", "\n", szRes, sizeof(szRes))) {
								if (_stricmp(lrtrimp(szRes), "Chunked") == 0)
									acksz = hdrend - szResult;
							}
							if (txtParseParam(szResult, nullptr, "Connection:", "\n", szRes, sizeof(szRes))) {
								needClose = (_stricmp(lrtrimp(szRes), "close") == 0);
							}
						}
					}

					// Content-Length bytes reached, all data received
					if (sz >= pktsz && pktsz != 0) {
						szResult[pktsz] = 0;
						break;
					}

					// Chunked encoding processing
					if (sz > acksz && acksz != 0) {
retry:
						// Parse out chunk size
						char* data = szResult + acksz;
						char* peol1 = data == hdrend ? data - 1 : strchr(data, '\n');
						if (peol1 != nullptr) {
							char *peol2 = strchr(++peol1, '\n');
							if (peol2 != nullptr) {
								// Get chunk size
								int chunkBytes = strtol(peol1, nullptr, 16);
								acksz += chunkBytes;
								peol2++;

								memmove(data, peol2, mir_strlen(peol2) + 1);
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
			else {
				if (retryCount < 2) {
					closeRouterConnection();
					++retryCount;
					goto retrycon;
				}
				else
					Netlib_Logf(nullptr, "UPnP send failed %d", WSAGetLastError());
			}
		}
		txtParseParam(szResult, "HTTP", " ", " ", szRes, sizeof(szRes));
		res = atol(szRes);
		if (szActionName != nullptr && res == 405 && szPostHdr == soap_post_hdr)
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

	gatewayFound = httpTransact(szUrl, szData, 8192, nullptr, DeviceGetReq) == 200;
	if (gatewayFound) {
		char szTemp[256], *rpth;
		size_t ctlLen;

		txtParseParam(szData, nullptr, "<URLBase>", "</URLBase>", szTemp, sizeof(szTemp));
		strncpy(szCtlUrl, szTemp[0] ? szTemp : szUrl, sizeof(szCtlUrl));
		szCtlUrl[sizeof(szCtlUrl) - 1] = 0;

		mir_snprintf(szTemp, search_device, szDev);
		txtParseParam(szData, szTemp, "<controlURL>", "</controlURL>", szUrl, sizeUrl);

		// URL combining per RFC 2396
		if (szUrl[0] != 0) {
			if (strstr(szUrl, "://") != nullptr)                     // absolute URI
				rpth = szCtlUrl;
			else if (strncmp(szUrl, "//", 2) == 0)                // relative URI net_path
			{
				rpth = strstr(szCtlUrl, "//");
				if (rpth == nullptr) rpth = szCtlUrl;
			}
			else if (szUrl[0] == '/')                             // relative URI abs_path
			{
				rpth = strstr(szCtlUrl, "//");
				rpth = rpth ? rpth + 2 : szCtlUrl;

				rpth = strchr(rpth, '/');
				if (rpth == nullptr) rpth = szCtlUrl + mir_strlen(szCtlUrl);
			}
			else {                                                      // relative URI rel_path
				size_t ctlCLen = mir_strlen(szCtlUrl);
				rpth = szCtlUrl + ctlCLen;
				if (ctlCLen != 0 && *(rpth - 1) != '/')
					strncpy(rpth++, "/", sizeof(szCtlUrl) - ctlCLen);
			}

			ctlLen = sizeof(szCtlUrl) - (rpth - szCtlUrl);
			strncpy(rpth, szUrl, ctlLen);
			szCtlUrl[sizeof(szCtlUrl) - 1] = 0;
		}
		else {
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
	unsigned* ips = nullptr;

	static const unsigned any = INADDR_ANY;
	static const TIMEVAL tv = { 1, 600000 };

	char szUrl[256] = "";
	char hostname[256];
	PHOSTENT he;
	fd_set readfd;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	SOCKADDR_IN enetaddr;
	enetaddr.sin_family = AF_INET;
	enetaddr.sin_port = htons(1900);
	enetaddr.sin_addr.s_addr = inet_addr("239.255.255.250");

	gethostname(hostname, sizeof(hostname));
	he = gethostbyname(hostname);

	if (he) {
		while (he->h_addr_list[nip]) ++nip;

		ips = (unsigned*)mir_alloc(nip * sizeof(unsigned));

		for (j = 0; j < nip; j++)
			ips[j] = *(unsigned*)he->h_addr_list[j];
	}

	buf = (char*)mir_alloc(1500);

	for (i = 3; --i && szUrl[0] == 0;) {
		for (j = 0; j < nip; j++) {
			if (ips)
				setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&ips[j], sizeof(unsigned));

			buflen = mir_snprintf(buf, 1500, search_request_msg, "WANIPConnection:1");
			sendto(s, buf, buflen, 0, (SOCKADDR*)&enetaddr, sizeof(enetaddr));
			LongLog(buf);

			buflen = mir_snprintf(buf, 1500, search_request_msg, "WANPPPConnection:1");
			sendto(s, buf, buflen, 0, (SOCKADDR*)&enetaddr, sizeof(enetaddr));
			LongLog(buf);
		}

		if (Miranda_IsTerminated()) break;

		FD_ZERO(&readfd);
		FD_SET(s, &readfd);

		while (select(1, &readfd, nullptr, nullptr, &tv) >= 1) {
			buflen = recv(s, buf, 1500, 0);
			if (buflen != SOCKET_ERROR) {
				buf[buflen] = 0;
				LongLog(buf);

				if (txtParseParam(buf, nullptr, "LOCATION:", "\n", szUrl, sizeof(szUrl)) ||
					txtParseParam(buf, nullptr, "Location:", "\n", szUrl, sizeof(szUrl))) {
					char age[30];
					char szHostNew[256], szHostExist[256];

					lrtrim(szUrl);

					parseURL(szUrl, szHostNew, nullptr, nullptr);
					parseURL(szCtlUrl, szHostExist, nullptr, nullptr);
					if (mir_strcmp(szHostNew, szHostExist) == 0) {
						gatewayFound = true;
						break;
					}

					txtParseParam(buf, nullptr, "ST:", "\n", szDev, sizeof(szDev));
					txtParseParam(buf, "max-age", " = ", "\n", age, sizeof(age));
					expireTime = atoi(lrtrimp(age));
					lrtrim(szDev);

					if (getUPnPURLs(szUrl, sizeof(szUrl))) {
						gatewayFound = getExtIP() != 0;
						if (gatewayFound) break;
					}
				}
			}
			FD_ZERO(&readfd);
			FD_SET(s, &readfd);
		}
	}

	mir_free(buf);
	mir_free(ips);
	setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&any, sizeof(unsigned));
	closesocket(s);
}

static bool findUPnPGateway(void)
{
	if ((time(0) - lastDiscTime) >= expireTime) {
		WaitForSingleObject(portListMutex, INFINITE);

		time_t curTime = time(0);

		if ((curTime - lastDiscTime) >= expireTime) {
			gatewayFound = false;

			discoverUPnP();
			lastDiscTime = curTime;

			Netlib_Logf(nullptr, "UPnP Gateway detected %d, Control URL: %s", gatewayFound, szCtlUrl);
		}

		ReleaseMutex(portListMutex);
	}

	return gatewayFound;
}

bool NetlibUPnPAddPortMapping(uint16_t intport, char *proto, uint16_t *extport, uint32_t *extip, bool search)
{
	int res = 0, i = 5;

	if (findUPnPGateway()) {
		char* szData = (char*)mir_alloc(4096);
		char szExtIP[30];

		*extport = intport - 1;
		*extip = ntohl(locIP.sin_addr.S_un.S_addr);

		WaitForSingleObject(portListMutex, INFINITE);

		do {
			++*extport;
			mir_snprintf(szData, 4096, add_port_mapping,
				*extport, proto, intport, inet_ntoa(locIP.sin_addr));
			res = httpTransact(szCtlUrl, szData, 4096, "AddPortMapping", ControlAction);
			txtParseParam(szData, nullptr, "<errorCode>", "</errorCode>", szExtIP, sizeof(szExtIP));

		} while (search && res == 500 && atol(szExtIP) == 718 && --i);

		mir_free(szData);

		if (res == 200) {
			unsigned ip = getExtIP();
			if (ip) *extip = ip;

			if (numports >= numportsAlloc)
				mir_realloc(portList, sizeof(uint16_t)*(numportsAlloc += 10));
			portList[numports++] = *extport;
		}

		ReleaseMutex(portListMutex);
	}

	return res == 200;
}

void NetlibUPnPDeletePortMapping(uint16_t extport, char* proto)
{
	if (extport == 0)
		return;

	//	findUPnPGateway();

	if (gatewayFound) {
		unsigned i;
		char* szData = (char*)mir_alloc(4096);

		WaitForSingleObject(portListMutex, INFINITE);
		mir_snprintf(szData, 4096, delete_port_mapping, extport, proto);
		httpTransact(szCtlUrl, szData, 4096, "DeletePortMapping", ControlAction);

		for (i = 0; i < numports; i++)
			if (portList[i] == extport && --numports > 0)
				memmove(&portList[i], &portList[i + 1], (numports - i) * sizeof(uint16_t));

		mir_free(szData);
		ReleaseMutex(portListMutex);
	}
}

void NetlibUPnPCleanup(void*)
{
	// upnp is disabled globally, no need for a cleanup
	if (db_get_b(0, "Netlib", "NLEnableUPnP", 1) == 0)
		return;

	{
		int incoming = 0;
		mir_cslock lck(csNetlibUser);
		for (auto &p : netlibUser)
			if (p->user.flags & NUF_INCOMING) {
				incoming = 1;
				break;
			}

		if (!incoming)
			return;
	}

	if (findUPnPGateway()) {
		char *szData = (char*)alloca(4096);
		char buf[50], lip[50];
		unsigned j = 0, k, num = 100;

		strncpy_s(lip, inet_ntoa(locIP.sin_addr), _TRUNCATE);

		WaitForSingleObject(portListMutex, INFINITE);

		if (httpTransact(szCtlUrl, szData, 4096, "PortMappingNumberOfEntries", ControlQuery) == 200 &&
			txtParseParam(szData, "QueryStateVariableResponse", "<return>", "<", buf, sizeof(buf)))
			num = atol(buf);

		uint16_t ports[30];
		for (unsigned i = 0; i < num && !Miranda_IsTerminated(); i++) {
			mir_snprintf(szData, 4096, get_port_mapping, i);

			ReleaseMutex(portListMutex);
			WaitForSingleObject(portListMutex, INFINITE);

			if (httpTransact(szCtlUrl, szData, 4096, "GetGenericPortMappingEntry", ControlAction) != 200)
				break;

			if (!txtParseParam(szData, "<NewPortMappingDescription", ">", "<", buf, sizeof(buf)) || mir_strcmp(buf, "Miranda") != 0)
				continue;

			if (!txtParseParam(szData, "<NewInternalClient", ">", "<", buf, sizeof(buf)) || mir_strcmp(buf, lip) != 0)
				continue;

			if (txtParseParam(szData, "<NewExternalPort", ">", "<", buf, sizeof(buf))) {
				uint16_t mport = (uint16_t)atol(buf);

				if (j >= _countof(ports))
					break;

				for (k = 0; k < numports; ++k)
					if (portList[k] == mport)
						break;

				if (k >= numports)
					ports[j++] = mport;
			}
		}

		ReleaseMutex(portListMutex);

		for (unsigned i = 0; i < j && !Miranda_IsTerminated(); i++)
			NetlibUPnPDeletePortMapping(ports[i], "TCP");
	}
}

void NetlibUPnPInit(void)
{
	numports = 0;
	numportsAlloc = 10;
	portList = (uint16_t*)mir_alloc(sizeof(uint16_t)*numportsAlloc);

	portListMutex = CreateMutex(nullptr, FALSE, nullptr);
}

void NetlibUPnPDestroy(void)
{
	mir_free(portList);
	CloseHandle(portListMutex);
}
