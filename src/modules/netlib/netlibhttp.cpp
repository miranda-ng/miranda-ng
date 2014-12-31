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
#include "../plugins/zlib/src/zlib.h"
#include "netlib.h"

#define HTTPRECVHEADERSTIMEOUT   30000  //in ms
#define HTTPRECVDATATIMEOUT      20000

struct ResizableCharBuffer
{
	char *sz;
	int iEnd, cbAlloced;
};

struct ProxyAuth
{
	char *szServer;
	char *szMethod;
//	char *szUserName;
//	char *szPassword;

	ProxyAuth(const char *pszServer, const char *pszMethod)
	{
		szServer = mir_strdup(pszServer);
		szMethod = mir_strdup(pszMethod);
	}
	~ProxyAuth()
	{
		mir_free(szServer);
		mir_free(szMethod);
	}
	static int Compare(const ProxyAuth* p1, const ProxyAuth* p2)
	{ return mir_strcmpi(p1->szServer, p2->szServer); }
};

struct ProxyAuthList : OBJLIST<ProxyAuth>
{
	ProxyAuthList() :  OBJLIST<ProxyAuth>(2, ProxyAuth::Compare) {}

	void add(const char *szServer, const char *szMethod)
	{
		if (szServer == NULL) return;
		int i = getIndex((ProxyAuth*)&szServer);
		if (i >= 0) {
			ProxyAuth &rec = (*this)[i];
			if (szMethod == NULL)
				remove(i);
			else if (_stricmp(rec.szMethod, szMethod)) {
				mir_free(rec.szMethod);
				rec.szMethod = mir_strdup(szMethod);
			}
		}
		else insert(new ProxyAuth(szServer, szMethod));
	}

	const char* find(const char *szServer)
	{
		ProxyAuth * rec = szServer ? OBJLIST<ProxyAuth>::find((ProxyAuth*)&szServer) : NULL;
		return rec ? rec->szMethod : NULL;
	}
};

ProxyAuthList proxyAuthList;

static void AppendToCharBuffer(struct ResizableCharBuffer *rcb, const char *fmt, ...)
{
	va_list va;
	int charsDone;

	if (rcb->cbAlloced == 0) {
		rcb->cbAlloced = 512;
		rcb->sz = (char*)mir_alloc(rcb->cbAlloced);
	}
	va_start(va, fmt);
	while (true) {
		charsDone = mir_vsnprintf(rcb->sz + rcb->iEnd, rcb->cbAlloced-rcb->iEnd, fmt, va);
		if (charsDone >= 0) break;
		rcb->cbAlloced += 512;
		rcb->sz = (char*)mir_realloc(rcb->sz, rcb->cbAlloced);
	}
	va_end(va);
	rcb->iEnd += charsDone;
}

static int RecvWithTimeoutTime(NetlibConnection *nlc, unsigned dwTimeoutTime, char *buf, int len, int flags)
{
	DWORD dwTimeNow;

	if (!si.pending(nlc->hSsl)) {
		while ((dwTimeNow = GetTickCount()) < dwTimeoutTime) {
			unsigned dwDeltaTime = min(dwTimeoutTime - dwTimeNow, 1000);
			int res = WaitUntilReadable(nlc->s, dwDeltaTime);

			switch (res) {
			case SOCKET_ERROR:
				return SOCKET_ERROR;

			case 1:
				return NLRecv(nlc, buf, len, flags);
			}

			if (nlc->termRequested || Miranda_Terminated())
				return 0;
		}
		SetLastError(ERROR_TIMEOUT);
		return SOCKET_ERROR;
	}
	return NLRecv(nlc, buf, len, flags);
}

static char* NetlibHttpFindHeader(NETLIBHTTPREQUEST *nlhrReply, const char *hdr)
{
	for (int i=0; i < nlhrReply->headersCount; i++) {
		NETLIBHTTPHEADER &p = nlhrReply->headers[i];
		if (_stricmp(p.szName, hdr) == 0)
			return p.szValue;
	}

	return NULL;
}

static char* NetlibHttpFindAuthHeader(NETLIBHTTPREQUEST *nlhrReply, const char *hdr, const char *szProvider)
{
	char *szBasicHdr = NULL;
	char *szNegoHdr = NULL;
	char *szNtlmHdr = NULL;

	for (int i=0; i < nlhrReply->headersCount; i++) {
		NETLIBHTTPHEADER &p = nlhrReply->headers[i];
		if (_stricmp(p.szName, hdr) == 0) {
			if (_strnicmp(p.szValue, "Negotiate", 9) == 0)
				szNegoHdr = p.szValue;
			else if (_strnicmp(p.szValue, "NTLM", 4) == 0)
				szNtlmHdr = p.szValue;
			else if (_strnicmp(p.szValue, "Basic", 5) == 0)
				szBasicHdr = p.szValue;
		}
	}

	if (szNegoHdr && (!szProvider || !_stricmp(szProvider, "Negotiate"))) return szNegoHdr;
	if (szNtlmHdr && (!szProvider || !_stricmp(szProvider, "NTLM"))) return szNtlmHdr;
	if (!szProvider || !_stricmp(szProvider, "Basic")) return szBasicHdr;
	return NULL;
}

void NetlibConnFromUrl(const char* szUrl, bool secur, NETLIBOPENCONNECTION &nloc)
{
	secur = secur || _strnicmp(szUrl, "https", 5) == 0;
	const char* phost = strstr(szUrl, "://");

	char* szHost = mir_strdup(phost ? phost + 3 : szUrl);

	char* ppath = strchr(szHost, '/');
	if (ppath) *ppath = '\0';

	memset(&nloc, 0, sizeof(nloc));
	nloc.cbSize = sizeof(nloc);
	nloc.szHost = szHost;

	char* pcolon = strrchr(szHost, ':');
	if (pcolon) {
		*pcolon = '\0';
		nloc.wPort = (WORD)strtol(pcolon+1, NULL, 10);
	}
	else nloc.wPort = secur ? 443 : 80;
	nloc.flags = (secur ? NLOCF_SSL : 0);
}

static NetlibConnection* NetlibHttpProcessUrl(NETLIBHTTPREQUEST *nlhr, NetlibUser *nlu, NetlibConnection* nlc, const char* szUrl = NULL)
{
	NETLIBOPENCONNECTION nloc;

	if (szUrl == NULL)
		NetlibConnFromUrl(nlhr->szUrl, (nlhr->flags & NLHRF_SSL) != 0, nloc);
	else
		NetlibConnFromUrl(szUrl, false, nloc);

	nloc.flags |= NLOCF_HTTP;
	if (nloc.flags & NLOCF_SSL) nlhr->flags |= NLHRF_SSL; else nlhr->flags &= ~NLHRF_SSL;

	if (nlc != NULL) {
		bool httpProxy = !(nloc.flags & NLOCF_SSL) && nlc->proxyType == PROXYTYPE_HTTP;
		bool sameHost = mir_strcmp(nlc->nloc.szHost, nloc.szHost) == 0 && nlc->nloc.wPort == nloc.wPort;

		if (!httpProxy && !sameHost) {
			NetlibDoClose(nlc);

			mir_free((char*)nlc->nloc.szHost);
			nlc->nloc = nloc;
			return NetlibDoConnect(nlc) ? nlc : NULL;
		}
	}
	else nlc = (NetlibConnection*)NetlibOpenConnection((WPARAM)nlu, (LPARAM)&nloc);

	mir_free((char*)nloc.szHost);

	return nlc;
}

struct HttpSecurityContext
{
	HANDLE m_hNtlmSecurity;
	char *m_szHost;
	char *m_szProvider;

	HttpSecurityContext()
	{ m_hNtlmSecurity = NULL;  m_szHost = NULL; m_szProvider = NULL; }

	~HttpSecurityContext() { Destroy(); }

	void Destroy(void)
	{
		if (!m_hNtlmSecurity) return;

		NetlibDestroySecurityProvider(m_hNtlmSecurity);
		m_hNtlmSecurity = NULL;
		mir_free(m_szHost); m_szHost = NULL;
		mir_free(m_szProvider); m_szProvider = NULL;
	}

	bool TryBasic(void)
	{
		return m_hNtlmSecurity && m_szProvider && _stricmp(m_szProvider, "Basic");
	}

	char* Execute(NetlibConnection *nlc, char* szHost, const char* szProvider,
		const char* szChallenge, unsigned& complete)
	{
		char* szAuthHdr = NULL;
		bool justCreated = false;

		if (m_hNtlmSecurity) {
			bool newAuth = !m_szProvider || !szProvider || _stricmp(m_szProvider, szProvider);
			newAuth = newAuth || (m_szHost != szHost && (!m_szHost || !szHost || _stricmp(m_szHost, szHost)));
			if (newAuth)
				Destroy();
		}

		if (m_hNtlmSecurity == NULL) {
			char szSpnStr[256] = "";
			if (szHost && _stricmp(szProvider, "Basic")) {
				unsigned long ip = inet_addr(szHost);
				PHOSTENT host = (ip == INADDR_NONE) ? gethostbyname(szHost) : gethostbyaddr((char*)&ip, 4, AF_INET);
				mir_snprintf(szSpnStr, SIZEOF(szSpnStr), "HTTP/%s", host && host->h_name ? host->h_name : szHost);
				_strlwr(szSpnStr + 5);
				NetlibLogf(nlc->nlu, "Host SPN: %s", szSpnStr);
			}
			m_hNtlmSecurity = NetlibInitSecurityProvider(szProvider, szSpnStr[0] ? szSpnStr : NULL);
			if (m_hNtlmSecurity) {
				m_szProvider = mir_strdup(szProvider);
				m_szHost = mir_strdup(szHost);
				justCreated = true;
			}
		}

		if (m_hNtlmSecurity) {
			TCHAR *szLogin = NULL, *szPassw = NULL;

			if (nlc->nlu->settings.useProxyAuth) {
				mir_cslock lck(csNetlibUser);
				szLogin = mir_a2t(nlc->nlu->settings.szProxyAuthUser);
				szPassw = mir_a2t(nlc->nlu->settings.szProxyAuthPassword);
			}

			szAuthHdr = NtlmCreateResponseFromChallenge(m_hNtlmSecurity,
				szChallenge, szLogin, szPassw, true, complete);

			if (!szAuthHdr) {
				NetlibLogf(NULL, "Security login %s failed, user: %S pssw: %S",
					szProvider, szLogin ? szLogin : _T("(no user)"), szPassw ? _T("(exist)") : _T("(no psw)"));
			}
			else if (justCreated)
				proxyAuthList.add(m_szHost, m_szProvider);

			mir_free(szLogin);
			mir_free(szPassw);
		}
		else complete = 1;

		return szAuthHdr;
	}
};

static int HttpPeekFirstResponseLine(NetlibConnection *nlc, DWORD dwTimeoutTime, DWORD recvFlags, int *resultCode, char **ppszResultDescr, int *length)
{
	int bytesPeeked;
	char buffer[2048];
	char *peol;

	while(true) {
		bytesPeeked = RecvWithTimeoutTime(nlc, dwTimeoutTime, buffer, SIZEOF(buffer) - 1, MSG_PEEK | recvFlags);

		if (bytesPeeked == 0) {
			SetLastError(ERROR_HANDLE_EOF);
			return 0;
		}
		if (bytesPeeked == SOCKET_ERROR)
			return 0;

		buffer[bytesPeeked] = '\0';
		if ((peol = strchr(buffer, '\n')) != NULL)
			break;

		if ((int)strlen(buffer) < bytesPeeked) {
			SetLastError(ERROR_BAD_FORMAT);
			return 0;
		}
		if (bytesPeeked == SIZEOF(buffer) - 1) {
			SetLastError(ERROR_BUFFER_OVERFLOW);
			return 0;
		}
		if (Miranda_Terminated()) return 0;
		Sleep(10);
	}

	if (peol == buffer) {
		SetLastError(ERROR_BAD_FORMAT);
		return 0;
	}

	*peol = '\0';

	if (_strnicmp(buffer, "HTTP/", 5)) {
		SetLastError(ERROR_BAD_FORMAT);
		return 0;
	}

	size_t off = strcspn(buffer, " \t");
	if (off >= (unsigned)bytesPeeked)
		return 0;

	char* pResultCode = buffer + off;
	*(pResultCode++) = 0;

	char* pResultDescr;
	*resultCode = strtol(pResultCode, &pResultDescr, 10);

	if (ppszResultDescr)
		*ppszResultDescr = mir_strdup(lrtrimp(pResultDescr));

	if (length) *length = peol - buffer + 1;
	return 1;
}

static int SendHttpRequestAndData(NetlibConnection *nlc, struct ResizableCharBuffer *httpRequest, NETLIBHTTPREQUEST *nlhr, int sendContentLengthHeader)
{
	bool sendData = (nlhr->requestType == REQUEST_POST || nlhr->requestType == REQUEST_PUT);

	if (sendContentLengthHeader && sendData)
		AppendToCharBuffer(httpRequest, "Content-Length: %d\r\n\r\n", nlhr->dataLength);
	else
		AppendToCharBuffer(httpRequest, "\r\n");

	DWORD hflags = (nlhr->flags & NLHRF_DUMPASTEXT ? MSG_DUMPASTEXT : 0) |
		(nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPSEND | NLHRF_NODUMPHEADERS) ?
			MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
		(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

	int bytesSent = NLSend(nlc, httpRequest->sz, httpRequest->iEnd, hflags);
	if (bytesSent != SOCKET_ERROR && sendData && nlhr->dataLength) {
		DWORD sflags = (nlhr->flags & NLHRF_DUMPASTEXT ? MSG_DUMPASTEXT : 0) |
			(nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPSEND) ?
				MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
			(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

		int sendResult = NLSend(nlc, nlhr->pData, nlhr->dataLength, sflags);

		bytesSent = sendResult != SOCKET_ERROR ? bytesSent + sendResult : SOCKET_ERROR;
	}
	mir_free(httpRequest->sz);
	memset(httpRequest, 0, sizeof(*httpRequest));

	return bytesSent;
}

INT_PTR NetlibHttpSendRequest(WPARAM wParam, LPARAM lParam)
{
	NetlibConnection *nlc = (struct NetlibConnection*)wParam;
	NETLIBHTTPREQUEST *nlhr = (NETLIBHTTPREQUEST*)lParam;
	NETLIBHTTPREQUEST *nlhrReply = NULL;
	HttpSecurityContext httpSecurity;

	struct ResizableCharBuffer httpRequest = {0};
	char *szHost = NULL, *szNewUrl = NULL;
	char *pszProxyAuthHdr = NULL, *pszAuthHdr = NULL;
	int i, doneHostHeader, doneContentLengthHeader, doneProxyAuthHeader, doneAuthHeader;
	int bytesSent;
	bool lastFirstLineFail = false;

	if (nlhr == NULL || nlhr->cbSize < NETLIBHTTPREQUEST_V1_SIZE || nlhr->szUrl == NULL || nlhr->szUrl[0] == '\0') {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	int hdrTimeout = nlhr->cbSize > NETLIBHTTPREQUEST_V1_SIZE && nlhr->timeout ? nlhr->timeout : HTTPRECVHEADERSTIMEOUT;

	const char *pszRequest;
	switch(nlhr->requestType) {
	case REQUEST_GET:     pszRequest = "GET";     break;
	case REQUEST_POST:    pszRequest = "POST";    break;
	case REQUEST_CONNECT: pszRequest = "CONNECT"; break;
	case REQUEST_HEAD:    pszRequest = "HEAD";    break;
	case REQUEST_PUT:     pszRequest = "PUT";     break;
	case REQUEST_DELETE:  pszRequest = "DELETE";  break;
	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	if (!nlc->usingHttpGateway)
		if (!NetlibEnterNestedCS(nlc, NLNCS_SEND))
			return SOCKET_ERROR;

	const char *pszFullUrl = nlhr->szUrl;
	const char *pszUrl = NULL;

	unsigned complete = false;
	int count = 11;
	while (--count) {
		if (!NetlibReconnect(nlc)) {
			bytesSent = SOCKET_ERROR;
			break;
		}

		if (!pszUrl) {
			pszUrl = pszFullUrl;
			if (nlhr->flags & (NLHRF_SMARTREMOVEHOST | NLHRF_REMOVEHOST | NLHRF_GENERATEHOST)) {
				bool usingProxy = nlc->proxyType == PROXYTYPE_HTTP && !(nlhr->flags & NLHRF_SSL);

				mir_free(szHost);
				szHost = NULL;

				const char *ppath, *phost;
				phost = strstr(pszUrl, "://");
				if (phost == NULL) phost = pszUrl;
				else phost += 3;
				ppath = strchr(phost, '/');
				if (ppath == phost) phost = NULL;

				if (nlhr->flags & NLHRF_GENERATEHOST) {
					szHost = mir_strdup(phost);
					if (ppath && phost) szHost[ppath - phost] = 0;
				}

				if (nlhr->flags & NLHRF_REMOVEHOST || (nlhr->flags & NLHRF_SMARTREMOVEHOST && !usingProxy))
					pszUrl = ppath ? ppath : "/";

				if (usingProxy && phost && !nlc->dnsThroughProxy) {
					char* tszHost = mir_strdup(phost);
					if (ppath && phost) tszHost[ppath - phost] = 0;
					char* cln = strchr(tszHost, ':'); if (cln) *cln = 0;

					if (inet_addr(tszHost) == INADDR_NONE) {
						DWORD ip = DnsLookup(nlc->nlu, tszHost);
						if (ip && szHost) {
							mir_free(szHost);
							szHost = (char*)mir_alloc(64);
							if (cln) *cln = ':';
							mir_snprintf(szHost, 64, "%s%s", inet_ntoa(*(PIN_ADDR)&ip), cln ? cln : "");
						}
					}
					mir_free(tszHost);
				}
			}
		}

		if (nlc->proxyAuthNeeded && proxyAuthList.getCount()) {
			if (httpSecurity.m_szProvider == NULL && nlc->szProxyServer) {
				const char* szAuthMethodNlu = proxyAuthList.find(nlc->szProxyServer);
				if (szAuthMethodNlu) {
					mir_free(pszProxyAuthHdr);
					pszProxyAuthHdr = httpSecurity.Execute(nlc, nlc->szProxyServer, szAuthMethodNlu, "", complete);
				}
			}
		}
		nlc->proxyAuthNeeded = false;

		AppendToCharBuffer(&httpRequest, "%s %s HTTP/1.%d\r\n", pszRequest, pszUrl, (nlhr->flags & NLHRF_HTTP11) != 0);

		//HTTP headers
		doneHostHeader = doneContentLengthHeader = doneProxyAuthHeader = doneAuthHeader = 0;
		for (i=0; i < nlhr->headersCount; i++) {
			NETLIBHTTPHEADER &p = nlhr->headers[i];
			if (!mir_strcmpi(p.szName, "Host")) doneHostHeader = 1;
			else if (!mir_strcmpi(p.szName, "Content-Length")) doneContentLengthHeader = 1;
			else if (!mir_strcmpi(p.szName, "Proxy-Authorization")) doneProxyAuthHeader = 1;
			else if (!mir_strcmpi(p.szName, "Authorization")) doneAuthHeader = 1;
			else if (!mir_strcmpi(p.szName, "Connection")) continue;
			if (p.szValue == NULL) continue;
			AppendToCharBuffer(&httpRequest, "%s: %s\r\n", p.szName, p.szValue);
		}
		if (szHost && !doneHostHeader)
			AppendToCharBuffer(&httpRequest, "%s: %s\r\n", "Host", szHost);
		if (pszProxyAuthHdr && !doneProxyAuthHeader)
			AppendToCharBuffer(&httpRequest, "%s: %s\r\n", "Proxy-Authorization", pszProxyAuthHdr);
		if (pszAuthHdr && !doneAuthHeader)
			AppendToCharBuffer(&httpRequest, "%s: %s\r\n", "Authorization", pszAuthHdr);
		AppendToCharBuffer(&httpRequest, "%s: %s\r\n", "Connection", "Keep-Alive");
		AppendToCharBuffer(&httpRequest, "%s: %s\r\n", "Proxy-Connection", "Keep-Alive");

		// Add Sticky Headers
		if (nlc->nlu->szStickyHeaders != NULL)
			AppendToCharBuffer(&httpRequest, "%s\r\n", nlc->nlu->szStickyHeaders);

		//send it
		bytesSent = SendHttpRequestAndData(nlc, &httpRequest, nlhr, !doneContentLengthHeader);
		if (bytesSent == SOCKET_ERROR)
			break;

		//ntlm reply
		if (doneContentLengthHeader && nlhr->requestType != REQUEST_HEAD)
			break;

		DWORD fflags = MSG_PEEK | MSG_NODUMP | ((nlhr->flags & NLHRF_NOPROXY) ? MSG_RAW : 0);
		DWORD dwTimeOutTime = hdrTimeout < 0 ? -1 : GetTickCount() + hdrTimeout;
		if (!HttpPeekFirstResponseLine(nlc, dwTimeOutTime, fflags, &nlhr->resultCode, NULL, NULL)) {
			NetlibLogf(nlc->nlu, "%s %d: %s Failed (%u %u)", __FILE__, __LINE__, "HttpPeekFirstResponseLine", GetLastError(), count);
			DWORD err = GetLastError();
			if (err == ERROR_TIMEOUT || err == ERROR_BAD_FORMAT || err == ERROR_BUFFER_OVERFLOW || lastFirstLineFail || nlc->termRequested || nlhr->requestType == REQUEST_CONNECT) {
				bytesSent = SOCKET_ERROR;
				break;
			}

			lastFirstLineFail = true;
			continue;
		}

		int resultCode = nlhr->resultCode;
		lastFirstLineFail = false;

		DWORD hflags = (nlhr->flags & (NLHRF_NODUMP|NLHRF_NODUMPHEADERS|NLHRF_NODUMPSEND) ?
					MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
					(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

		DWORD dflags = (nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPSEND) ? MSG_NODUMP : MSG_DUMPASTEXT | MSG_DUMPPROXY) |
				 (nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0) | MSG_NODUMP;

		if (resultCode == 100)
			nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, hflags);

		else if (resultCode == 307 || ((resultCode == 301 || resultCode == 302) && (nlhr->flags & NLHRF_REDIRECT))) { // redirect
			pszUrl = NULL;

			if (nlhr->requestType == REQUEST_HEAD)
				nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, hflags);
			else
				nlhrReply = NetlibHttpRecv(nlc, hflags, dflags);

			if (nlhrReply) {
				char* tmpUrl = NetlibHttpFindHeader(nlhrReply, "Location");
				if (tmpUrl) {
					size_t rlen = 0;
					if (tmpUrl[0] == '/') {
						const char *ppath, *phost;
						phost = strstr(pszFullUrl, "://");
						phost = phost ? phost + 3 : pszFullUrl;
						ppath = strchr(phost, '/');
						rlen = ppath ? ppath - pszFullUrl : strlen(pszFullUrl);
					}

					nlc->szNewUrl = (char*)mir_realloc(nlc->szNewUrl, rlen + strlen(tmpUrl) * 3 + 1);

					strncpy(nlc->szNewUrl, pszFullUrl, rlen);
					strcpy(nlc->szNewUrl + rlen, tmpUrl);
					pszFullUrl = nlc->szNewUrl;
					pszUrl = NULL;

					if (NetlibHttpProcessUrl(nlhr, nlc->nlu, nlc, pszFullUrl) == NULL) {
						bytesSent = SOCKET_ERROR;
						break;
					}
				}
				else {
					NetlibHttpSetLastErrorUsingHttpResult(resultCode);
					bytesSent = SOCKET_ERROR;
					break;
				}
			}
			else {
				NetlibHttpSetLastErrorUsingHttpResult(resultCode);
				bytesSent = SOCKET_ERROR;
				break;
			}
		}
		else if (resultCode == 401 && !doneAuthHeader) { //auth required
			if (nlhr->requestType == REQUEST_HEAD)
				nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, hflags);
			else
				nlhrReply = NetlibHttpRecv(nlc, hflags, dflags);

			mir_free(pszAuthHdr); pszAuthHdr = NULL;
			if (nlhrReply) {
				char *szAuthStr = NULL;
				if (!complete) {
					szAuthStr = NetlibHttpFindAuthHeader(nlhrReply, "WWW-Authenticate", httpSecurity.m_szProvider);
					if (szAuthStr) {
						char *szChallenge = strchr(szAuthStr, ' ');
						if (!szChallenge || !*lrtrimp(szChallenge))
							complete = true;
					}
				}
				if (complete && httpSecurity.m_hNtlmSecurity)
					szAuthStr = httpSecurity.TryBasic() ? NetlibHttpFindAuthHeader(nlhrReply, "WWW-Authenticate", "Basic") : NULL;

				if (szAuthStr) {
					char *szChallenge = strchr(szAuthStr, ' ');
					if (szChallenge) { *szChallenge = 0; szChallenge = lrtrimp(szChallenge + 1); }

					pszAuthHdr = httpSecurity.Execute(nlc, szHost, szAuthStr, szChallenge, complete);
				}
			}
			if (pszAuthHdr == NULL) {
				proxyAuthList.add(szHost, NULL);
				NetlibHttpSetLastErrorUsingHttpResult(resultCode);
				bytesSent = SOCKET_ERROR;
				break;
			}
		}
		else if (resultCode == 407 && !doneProxyAuthHeader) { //proxy auth required
			if (nlhr->requestType == REQUEST_HEAD)
				nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, hflags);
			else
				nlhrReply = NetlibHttpRecv(nlc, hflags, dflags);

			mir_free(pszProxyAuthHdr); pszProxyAuthHdr = NULL;
			if (nlhrReply) {
				char *szAuthStr = NULL;
				if (!complete) {
					szAuthStr = NetlibHttpFindAuthHeader(nlhrReply, "Proxy-Authenticate", httpSecurity.m_szProvider);
					if (szAuthStr) {
						char *szChallenge = strchr(szAuthStr, ' ');
						if (!szChallenge || !*lrtrimp(szChallenge + 1))
							complete = true;
					}
				}
				if (complete && httpSecurity.m_hNtlmSecurity)
					szAuthStr = httpSecurity.TryBasic() ? NetlibHttpFindAuthHeader(nlhrReply, "Proxy-Authenticate", "Basic") : NULL;

				if (szAuthStr) {
					char *szChallenge = strchr(szAuthStr, ' ');
					if (szChallenge) { *szChallenge = 0; szChallenge = lrtrimp(szChallenge + 1); }

					pszProxyAuthHdr = httpSecurity.Execute(nlc, nlc->szProxyServer, szAuthStr, szChallenge, complete);
				}
			}
			if (pszProxyAuthHdr == NULL) {
				proxyAuthList.add(nlc->szProxyServer, NULL);
				NetlibHttpSetLastErrorUsingHttpResult(resultCode);
				bytesSent = SOCKET_ERROR;
				break;
			}
		}
		else break;

		if (pszProxyAuthHdr && resultCode != 407 && !doneProxyAuthHeader)
			replaceStr(pszProxyAuthHdr, NULL);

		if (pszAuthHdr && resultCode != 401 && !doneAuthHeader)
			replaceStr(pszAuthHdr, NULL);

		if (nlhrReply) {
			NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
			nlhrReply = NULL;
		}
	}

	if (count == 0) bytesSent = SOCKET_ERROR;
	if (nlhrReply) NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);

	//clean up
	mir_free(pszProxyAuthHdr);
	mir_free(pszAuthHdr);
	mir_free(szHost);
	mir_free(szNewUrl);

	if (!nlc->usingHttpGateway)
		NetlibLeaveNestedCS(&nlc->ncsSend);

	return bytesSent;
}

INT_PTR NetlibHttpFreeRequestStruct(WPARAM, LPARAM lParam)
{
	NETLIBHTTPREQUEST *nlhr = (NETLIBHTTPREQUEST*)lParam;

	if (nlhr == NULL || nlhr->cbSize != sizeof(NETLIBHTTPREQUEST) || nlhr->requestType != REQUEST_RESPONSE) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if (nlhr->headers) {
		for (int i=0; i < nlhr->headersCount; i++) {
			NETLIBHTTPHEADER &p = nlhr->headers[i];
			mir_free(p.szName);
			mir_free(p.szValue);
		}
		mir_free(nlhr->headers);
	}
	mir_free(nlhr->pData);
	mir_free(nlhr->szResultDescr);
	mir_free(nlhr->szUrl);
	mir_free(nlhr);
	return 1;
}

INT_PTR NetlibHttpRecvHeaders(WPARAM wParam, LPARAM lParam)
{
	NetlibConnection *nlc = (struct NetlibConnection*)wParam;
	if (!NetlibEnterNestedCS(nlc, NLNCS_RECV))
		return 0;

	char *peol, *pbuffer;
	int headersCount = 0, bufferSize = 8192;

	DWORD dwRequestTimeoutTime = GetTickCount() + HTTPRECVDATATIMEOUT;
	NETLIBHTTPREQUEST *nlhr = (NETLIBHTTPREQUEST*)mir_calloc(sizeof(NETLIBHTTPREQUEST));
	nlhr->cbSize = sizeof(NETLIBHTTPREQUEST);
	nlhr->nlc = nlc;  // Needed to id connection in the protocol HTTP gateway wrapper functions
	nlhr->requestType = REQUEST_RESPONSE;

	int firstLineLength = 0;
	if (!HttpPeekFirstResponseLine(nlc, dwRequestTimeoutTime, lParam | MSG_PEEK, &nlhr->resultCode, &nlhr->szResultDescr, &firstLineLength)) {
		NetlibLeaveNestedCS(&nlc->ncsRecv);
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhr);
		return 0;
	}

	char *buffer = (char*)mir_alloc(bufferSize + 1);
	int bytesPeeked = NLRecv(nlc, buffer, min(firstLineLength, bufferSize), lParam | MSG_DUMPASTEXT);
	if (bytesPeeked != firstLineLength) {
		NetlibLeaveNestedCS(&nlc->ncsRecv);
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhr);
		if (bytesPeeked != SOCKET_ERROR) SetLastError(ERROR_HANDLE_EOF);
		mir_free(buffer);
		return 0;
	}

	// Make sure all headers arrived
	bytesPeeked = 0;
	for (bool headersCompleted = false; !headersCompleted; ) {
		if (bytesPeeked >= bufferSize) {
			bufferSize += 8192;
			mir_free(buffer);
			if (bufferSize > 32 * 1024) {
				bytesPeeked = 0;
				break;
			}
			buffer = (char*)mir_alloc(bufferSize + 1);
		}

		bytesPeeked = RecvWithTimeoutTime(nlc, dwRequestTimeoutTime, buffer, bufferSize, MSG_PEEK | MSG_NODUMP | lParam);
		if (bytesPeeked == 0)
			break;

		if (bytesPeeked == SOCKET_ERROR) {
			bytesPeeked = 0;
			break;
		}
		buffer[bytesPeeked] = 0;

		for (pbuffer = buffer, headersCount = 0;; pbuffer = peol + 1, ++headersCount) {
			peol = strchr(pbuffer, '\n');
			if (peol == NULL) break;
			if (peol == pbuffer || (peol == (pbuffer + 1) && *pbuffer == '\r')) {
				bytesPeeked = peol - buffer + 1;
				headersCompleted = true;
				break;
			}
		}
	}

	// Receive headers
	if (bytesPeeked > 0)
		bytesPeeked = NLRecv(nlc, buffer, bytesPeeked, lParam | MSG_DUMPASTEXT);
	if (bytesPeeked <= 0) {
		NetlibLeaveNestedCS(&nlc->ncsRecv);
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhr);
		mir_free(buffer);
		return 0;
	}
	buffer[bytesPeeked] = 0;

	nlhr->headersCount = headersCount;
	nlhr->headers = (NETLIBHTTPHEADER*)mir_calloc(sizeof(NETLIBHTTPHEADER) * headersCount);

	for (pbuffer = buffer, headersCount = 0;; pbuffer = peol + 1, ++headersCount) {
		peol = strchr(pbuffer, '\n');
		if (peol == NULL || peol == pbuffer || (peol == (pbuffer + 1) && *pbuffer == '\r')) break;
		*peol = 0;

		char *pColon = strchr(pbuffer, ':');
		if (pColon == NULL) {
			NetlibHttpFreeRequestStruct(0, (LPARAM)nlhr); nlhr = NULL;
			SetLastError(ERROR_INVALID_DATA);
			break;
		}

		*(pColon++) = 0;
		nlhr->headers[headersCount].szName = mir_strdup(rtrim(pbuffer));
		nlhr->headers[headersCount].szValue = mir_strdup(lrtrimp(pColon));
	}

	NetlibLeaveNestedCS(&nlc->ncsRecv);
	mir_free(buffer);
	return (INT_PTR)nlhr;
}

INT_PTR NetlibHttpTransaction(WPARAM wParam, LPARAM lParam)
{
	NetlibUser *nlu = (NetlibUser*)wParam;
	NETLIBHTTPREQUEST *nlhr = (NETLIBHTTPREQUEST*)lParam, *nlhrReply;
	DWORD dflags, hflags;

	if (GetNetlibHandleType(nlu) != NLH_USER || !(nlu->user.flags & NUF_OUTGOING) ||
		nlhr == NULL || nlhr->cbSize < NETLIBHTTPREQUEST_V1_SIZE ||
		nlhr->szUrl == NULL || nlhr->szUrl[0] == 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	if (nlhr->nlc != NULL && GetNetlibHandleType(nlhr->nlc) != NLH_CONNECTION)
		nlhr->nlc = NULL;

	NetlibConnection* nlc = NetlibHttpProcessUrl(nlhr, nlu, (NetlibConnection*)nlhr->nlc);
	if (nlc == NULL)
		return 0;

	NETLIBHTTPREQUEST nlhrSend;
	char szUserAgent[64];

	nlhrSend = *nlhr;
	nlhrSend.flags &= ~NLHRF_REMOVEHOST;
	nlhrSend.flags |= NLHRF_GENERATEHOST | NLHRF_SMARTREMOVEHOST | NLHRF_SMARTAUTHHEADER;

	bool doneUserAgentHeader = NetlibHttpFindHeader(nlhr, "User-Agent") != NULL;
	bool doneAcceptEncoding = NetlibHttpFindHeader(nlhr, "Accept-Encoding") != NULL;

	if (!doneUserAgentHeader || !doneAcceptEncoding) {
		nlhrSend.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER) * (nlhrSend.headersCount + 2));
		memcpy(nlhrSend.headers, nlhr->headers, sizeof(NETLIBHTTPHEADER) * nlhr->headersCount);
	}

	if (!doneUserAgentHeader) {
		nlhrSend.headers[nlhrSend.headersCount].szName = "User-Agent";
		nlhrSend.headers[nlhrSend.headersCount].szValue = szUserAgent;
		++nlhrSend.headersCount;

		char szMirandaVer[64];
		CallService(MS_SYSTEM_GETVERSIONTEXT, SIZEOF(szMirandaVer), (LPARAM)szMirandaVer);
		char *pspace = strchr(szMirandaVer, ' ');
		if (pspace) {
			*pspace++='\0';
			mir_snprintf(szUserAgent, SIZEOF(szUserAgent), "Miranda/%s (%s)", szMirandaVer, pspace);
		}
		else mir_snprintf(szUserAgent, SIZEOF(szUserAgent), "Miranda/%s", szMirandaVer);
	}
	if (!doneAcceptEncoding) {
		nlhrSend.headers[nlhrSend.headersCount].szName = "Accept-Encoding";
		nlhrSend.headers[nlhrSend.headersCount].szValue = "deflate, gzip";
		++nlhrSend.headersCount;
	}
	if (NetlibHttpSendRequest((WPARAM)nlc, (LPARAM)&nlhrSend) == SOCKET_ERROR) {
		if (!doneUserAgentHeader || !doneAcceptEncoding) mir_free(nlhrSend.headers);
		nlhr->resultCode = nlhrSend.resultCode;
		NetlibCloseHandle((WPARAM)nlc, 0);
		return 0;
	}
	if (!doneUserAgentHeader || !doneAcceptEncoding)
		mir_free(nlhrSend.headers);

	dflags = (nlhr->flags & NLHRF_DUMPASTEXT ? MSG_DUMPASTEXT:0) |
		(nlhr->flags & NLHRF_NODUMP ? MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
		(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

	hflags =
		(nlhr->flags & NLHRF_NODUMP ? MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
		(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

	if (nlhr->requestType == REQUEST_HEAD)
		nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, 0);
	else
		nlhrReply = NetlibHttpRecv(nlc, hflags, dflags);

	if (nlhrReply) {
		nlhrReply->szUrl = nlc->szNewUrl;
		nlc->szNewUrl = NULL;
	}

	if ((nlhr->flags & NLHRF_PERSISTENT) == 0 || nlhrReply == NULL) {
		NetlibCloseHandle((WPARAM)nlc, 0);
		if (nlhrReply)
			nlhrReply->nlc = NULL;
	}
	else nlhrReply->nlc = nlc;

	return (INT_PTR)nlhrReply;
}

void NetlibHttpSetLastErrorUsingHttpResult(int result)
{
	if (result >= 200 && result < 300) {
		SetLastError(ERROR_SUCCESS);
		return;
	}
	switch(result) {
		case 400: SetLastError(ERROR_BAD_FORMAT); break;
		case 401:
		case 402:
		case 403:
		case 407: SetLastError(ERROR_ACCESS_DENIED); break;
		case 404: SetLastError(ERROR_FILE_NOT_FOUND); break;
		case 405:
		case 406: SetLastError(ERROR_INVALID_FUNCTION); break;
		case 408: SetLastError(ERROR_TIMEOUT); break;
		default: SetLastError(ERROR_GEN_FAILURE); break;
	}
}

char* gzip_decode(char *gzip_data, int *len_ptr, int window)
{
	if (*len_ptr == 0) return NULL;

	int gzip_len = *len_ptr * 5;
	char* output_data = NULL;

	int gzip_err;
	z_stream zstr;

	do {
		output_data = (char*)mir_realloc(output_data, gzip_len+1);

		zstr.next_in = (Bytef*)gzip_data;
		zstr.avail_in = *len_ptr;
		zstr.zalloc = Z_NULL;
		zstr.zfree = Z_NULL;
		zstr.opaque = Z_NULL;
		inflateInit2_(&zstr, window, ZLIB_VERSION, sizeof(z_stream));

		zstr.next_out = (Bytef*)output_data;
		zstr.avail_out = gzip_len;

		gzip_err = inflate(&zstr, Z_FINISH);

		inflateEnd(&zstr);
		gzip_len *= 2;
	}
	while (gzip_err == Z_BUF_ERROR);

	gzip_len = gzip_err == Z_STREAM_END ? zstr.total_out : -1;

	if (gzip_len <= 0) {
		mir_free(output_data);
		output_data = NULL;
	}
	else output_data[gzip_len] = 0;

	*len_ptr = gzip_len;
	return output_data;
}

static int NetlibHttpRecvChunkHeader(NetlibConnection* nlc, bool first, DWORD flags)
{
	char data[64], *peol1;

	while(true) {
		int recvResult = NLRecv(nlc, data, 31, MSG_RAW | MSG_PEEK);
		if (recvResult <= 0)
			return SOCKET_ERROR;

		data[recvResult] = 0;

		peol1 = strchr(data, '\n');
		if (peol1 != NULL) {
			char *peol2 = first ? peol1 : strchr(peol1 + 1, '\n');
			if (peol2 != NULL) {
				int sz = peol2 - data + 1;
				int r = strtol(first ? data : peol1 + 1, NULL, 16);
				if (r == 0) {
					char *peol3 = strchr(peol2 + 1, '\n');
					if (peol3 == NULL) continue;
					sz = peol3 - data + 1;
				}
				NLRecv(nlc, data, sz, MSG_RAW | flags);
				return r;
			}
			else if (recvResult >= 31)
				return SOCKET_ERROR;
		}
	}
}

NETLIBHTTPREQUEST* NetlibHttpRecv(NetlibConnection* nlc, DWORD hflags, DWORD dflags, bool isConnect)
{
	int dataLen = -1, i, chunkhdr = 0;
	bool chunked = false;
	int cenc = 0, cenctype = 0, close = 0;

next:
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, hflags);
	if (nlhrReply == NULL)
		return NULL;

	if (nlhrReply->resultCode == 100) {
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
		goto next;
	}

	for (i=0; i<nlhrReply->headersCount; i++) {
		NETLIBHTTPHEADER &p = nlhrReply->headers[i];
		if (!mir_strcmpi(p.szName, "Content-Length"))
			dataLen = atoi(p.szValue);

		if (!mir_strcmpi(p.szName, "Content-Encoding")) {
			cenc = i;
			if (strstr(p.szValue, "gzip"))
				cenctype = 1;
			else if (strstr(p.szValue, "deflate"))
				cenctype = 2;
		}

		if (!mir_strcmpi(p.szName, "Connection"))
			close = !mir_strcmpi(p.szValue, "close");

		if (!mir_strcmpi(p.szName, "Transfer-Encoding") &&
			!mir_strcmpi(p.szValue, "chunked"))
		{
			chunked = true;
			chunkhdr = i;
			dataLen = -1;
		}
	}

	if (nlhrReply->resultCode >= 200 && (dataLen > 0 || (!isConnect && dataLen < 0))) {
		int recvResult, chunksz = -1;
		int dataBufferAlloced;

		if (chunked) {
			chunksz = NetlibHttpRecvChunkHeader(nlc, true, dflags);
			if (chunksz == SOCKET_ERROR) {
				NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
				return NULL;
			}
			dataLen = chunksz;
		}
		dataBufferAlloced = dataLen < 0 ? 2048 : dataLen + 1;
		nlhrReply->pData = (char*)mir_realloc(nlhrReply->pData, dataBufferAlloced);

		while (chunksz != 0) {
			while(true) {
				recvResult = RecvWithTimeoutTime(nlc, GetTickCount() + HTTPRECVDATATIMEOUT,
					nlhrReply->pData + nlhrReply->dataLength,
					dataBufferAlloced - nlhrReply->dataLength - 1,
					dflags | (cenctype ? MSG_NODUMP : 0));

				if (recvResult == 0) break;
				if (recvResult == SOCKET_ERROR) {
					NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
					return NULL;
				}
				nlhrReply->dataLength += recvResult;

				if (dataLen >= 0)
				{
					if (nlhrReply->dataLength >= dataLen)
						break;
				}
				else if ((dataBufferAlloced - nlhrReply->dataLength) < 256) {
					dataBufferAlloced += 2048;
					nlhrReply->pData = (char*)mir_realloc(nlhrReply->pData, dataBufferAlloced);
					if (nlhrReply->pData == NULL) {
						SetLastError(ERROR_OUTOFMEMORY);
						NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
						return NULL;
					}
				}
				Sleep(10);
			}

			if (!chunked)
				break;

			chunksz = NetlibHttpRecvChunkHeader(nlc, false, dflags);
			if (chunksz == SOCKET_ERROR) {
				NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
				return NULL;
			}
			dataLen += chunksz;
			dataBufferAlloced += chunksz;

			nlhrReply->pData = (char*)mir_realloc(nlhrReply->pData, dataBufferAlloced);
		}

		nlhrReply->pData[nlhrReply->dataLength] = '\0';
	}

	if (chunked) {
		nlhrReply->headers[chunkhdr].szName = (char*)mir_realloc(nlhrReply->headers[chunkhdr].szName, 16);
		mir_strcpy(nlhrReply->headers[chunkhdr].szName, "Content-Length");

		nlhrReply->headers[chunkhdr].szValue = (char*)mir_realloc(nlhrReply->headers[chunkhdr].szValue, 16);
		mir_snprintf(nlhrReply->headers[chunkhdr].szValue, 16, "%u", nlhrReply->dataLength);
	}

	if (cenctype) {
		int bufsz = nlhrReply->dataLength;
		char* szData = NULL;

		switch (cenctype) {
		case 1:
			szData = gzip_decode(nlhrReply->pData, &bufsz, 0x10 | MAX_WBITS);
			break;

		case 2:
			szData = gzip_decode(nlhrReply->pData, &bufsz, -MAX_WBITS);
			if (bufsz < 0)
			{
				bufsz = nlhrReply->dataLength;
				szData = gzip_decode(nlhrReply->pData, &bufsz, MAX_WBITS);
			}
			break;
		}

		if (bufsz > 0) {
			NetlibDumpData(nlc, (PBYTE)szData, bufsz, 0, dflags);
			mir_free(nlhrReply->pData);
			nlhrReply->pData = szData;
			nlhrReply->dataLength = bufsz;

			mir_free(nlhrReply->headers[cenc].szName);
			mir_free(nlhrReply->headers[cenc].szValue);
			memmove(&nlhrReply->headers[cenc], &nlhrReply->headers[cenc+1], (--nlhrReply->headersCount-cenc)*sizeof(nlhrReply->headers[0]));
		}
		else if (bufsz == 0) {
			mir_free(nlhrReply->pData);
			nlhrReply->pData = NULL;
			nlhrReply->dataLength = 0;
		}
	}

	if (close &&
		(nlc->proxyType != PROXYTYPE_HTTP || nlc->nloc.flags & NLOCF_SSL) &&
		(!isConnect || nlhrReply->resultCode != 200))
		NetlibDoClose(nlc);

	return nlhrReply;
}
