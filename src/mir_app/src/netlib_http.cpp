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
#include "../libs/zlib/src/zlib.h"
#include "netlib.h"

#define HTTPRECVHEADERSTIMEOUT   30000  //in ms
#define HTTPRECVDATATIMEOUT      20000

struct ProxyAuth
{
	char *szServer;
	char *szMethod;

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
	static int Compare(const ProxyAuth *p1, const ProxyAuth *p2)
	{
		return mir_strcmpi(p1->szServer, p2->szServer);
	}
};

struct ProxyAuthList : OBJLIST<ProxyAuth>
{
	ProxyAuthList() : OBJLIST<ProxyAuth>(2, ProxyAuth::Compare) {}

	void add(const char *szServer, const char *szMethod)
	{
		if (szServer == nullptr) return;
		int i = getIndex((ProxyAuth*)&szServer);
		if (i >= 0) {
			ProxyAuth &rec = (*this)[i];
			if (szMethod == nullptr)
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
		ProxyAuth *rec = szServer ? OBJLIST<ProxyAuth>::find((ProxyAuth*)&szServer) : nullptr;
		return rec ? rec->szMethod : nullptr;
	}
};

ProxyAuthList proxyAuthList;

/////////////////////////////////////////////////////////////////////////////////////////
// Module exports

EXTERN_C MIR_APP_DLL(bool) Netlib_FreeHttpRequest(MHttpResponse *nlhr)
{
	if (nlhr == nullptr) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	delete nlhr;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int RecvWithTimeoutTime(NetlibConnection *nlc, int dwTimeoutTime, char *buf, int len, int flags)
{
	if (!nlc->foreBuf.isEmpty() || Netlib_SslPending(nlc->hSsl)) 
		return Netlib_Recv(nlc, buf, len, flags);

	int dwTimeNow;
	while ((dwTimeNow = GetTickCount()) < dwTimeoutTime) {
		int dwDeltaTime = min(dwTimeoutTime - dwTimeNow, 1000);
		int res = WaitUntilReadable(nlc->s, dwDeltaTime);

		switch (res) {
		case SOCKET_ERROR:
			return SOCKET_ERROR;

		case 1:
			return Netlib_Recv(nlc, buf, len, flags);
		}

		if (nlc->termRequested || Miranda_IsTerminated())
			return 0;
	}
	SetLastError(ERROR_TIMEOUT);
	return SOCKET_ERROR;
}

static char* NetlibHttpFindAuthHeader(MHttpResponse *nlhrReply, const char *hdr, const char *szProvider)
{
	char *szBasicHdr = nullptr;
	char *szNegoHdr = nullptr;
	char *szNtlmHdr = nullptr;

	for (auto &it : *nlhrReply) {
		if (_stricmp(it->szName, hdr))
			continue;

		if (!_strnicmp(it->szValue, "Negotiate", 9))
			szNegoHdr = it->szValue;
		else if (!_strnicmp(it->szValue, "NTLM", 4))
			szNtlmHdr = it->szValue;
		else if (!_strnicmp(it->szValue, "Basic", 5))
			szBasicHdr = it->szValue;
	}

	if (szNegoHdr && (!szProvider || !_stricmp(szProvider, "Negotiate"))) return szNegoHdr;
	if (szNtlmHdr && (!szProvider || !_stricmp(szProvider, "NTLM"))) return szNtlmHdr;
	if (!szProvider || !_stricmp(szProvider, "Basic")) return szBasicHdr;
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void NetlibConnFromUrl(const char *szUrl, bool secur, NetlibUrl &url)
{
	secur = secur || _strnicmp(szUrl, "https", 5) == 0;
	
	const char* phost = strstr(szUrl, "://");
	url.szHost = phost ? phost + 3 : szUrl;

	int idx = url.szHost.Find('/');
	if (idx != -1)
		url.szHost.Truncate(idx);

	if ((idx = url.szHost.Find(':')) != -1) {
		url.port = strtol(url.szHost.c_str() + idx + 1, nullptr, 10);
		url.szHost.Truncate(idx);
	}
	else url.port = secur ? 443 : 80;
	url.flags = (secur ? NLOCF_SSL : 0);
}

static NetlibConnection* NetlibHttpProcessUrl(MHttpRequest *nlhr, NetlibUser *nlu, NetlibConnection *nlc, const char *szUrl = nullptr)
{
	NetlibUrl url;

	if (szUrl == nullptr)
		NetlibConnFromUrl(nlhr->m_szUrl, (nlhr->flags & NLHRF_SSL) != 0, url);
	else
		NetlibConnFromUrl(szUrl, false, url);

	url.flags |= NLOCF_HTTP;
	if (url.flags & NLOCF_SSL)
		nlhr->flags |= NLHRF_SSL;
	else
		nlhr->flags &= ~NLHRF_SSL;

	if (nlc != nullptr) {
		bool httpProxy = !(url.flags & NLOCF_SSL) && nlc->proxyType == PROXYTYPE_HTTP;
		bool sameHost = mir_strcmp(nlc->url.szHost, url.szHost) == 0 && nlc->url.port == url.port;

		if (!httpProxy && !sameHost) {
			NetlibDoCloseSocket(nlc);

			nlc->url = url;
			return NetlibDoConnect(nlc) ? nlc : nullptr;
		}
	}
	else nlc = (NetlibConnection*)Netlib_OpenConnection(nlu, url.szHost, url.port, 0, url.flags);

	return nlc;
}

struct HttpSecurityContext
{
	HANDLE m_hNtlmSecurity;
	char *m_szHost;
	char *m_szProvider;

	HttpSecurityContext()
	{
		m_hNtlmSecurity = nullptr;  m_szHost = nullptr; m_szProvider = nullptr;
	}

	~HttpSecurityContext() { Destroy(); }

	void Destroy(void)
	{
		if (!m_hNtlmSecurity) return;

		Netlib_DestroySecurityProvider(m_hNtlmSecurity);
		m_hNtlmSecurity = nullptr;
		mir_free(m_szHost); m_szHost = nullptr;
		mir_free(m_szProvider); m_szProvider = nullptr;
	}

	bool TryBasic(void)
	{
		return m_hNtlmSecurity && m_szProvider && _stricmp(m_szProvider, "Basic");
	}

	char* Execute(NetlibConnection *nlc, char *szHost, const char *szProvider, const char *szChallenge, unsigned &complete)
	{
		char *szAuthHdr = nullptr;
		bool justCreated = false;
		NetlibUser *nlu = nlc->nlu;

		if (m_hNtlmSecurity) {
			bool newAuth = !m_szProvider || !szProvider || _stricmp(m_szProvider, szProvider);
			newAuth = newAuth || (m_szHost != szHost && (!m_szHost || !szHost || _stricmp(m_szHost, szHost)));
			if (newAuth)
				Destroy();
		}

		if (m_hNtlmSecurity == nullptr) {
			CMStringA szSpnStr;
			if (szHost && _stricmp(szProvider, "Basic")) {
				unsigned long ip = inet_addr(szHost);
				PHOSTENT host = (ip == INADDR_NONE) ? gethostbyname(szHost) : gethostbyaddr((char*)&ip, 4, AF_INET);
				szSpnStr.Format("HTTP/%s", host && host->h_name ? host->h_name : szHost);
				_strlwr(szSpnStr.GetBuffer() + 5);
				Netlib_Logf(nlu, "Host SPN: %s", szSpnStr.c_str());
			}
			m_hNtlmSecurity = Netlib_InitSecurityProvider(_A2T(szProvider), szSpnStr.IsEmpty() ? nullptr : _A2T(szSpnStr.c_str()));
			if (m_hNtlmSecurity) {
				m_szProvider = mir_strdup(szProvider);
				m_szHost = mir_strdup(szHost);
				justCreated = true;
			}
		}

		if (m_hNtlmSecurity) {
			ptrW szLogin, szPassw;

			if (nlu->settings.useProxyAuth) {
				mir_cslock lck(csNetlibUser);
				szLogin = mir_a2u(nlu->settings.szProxyAuthUser);
				szPassw = mir_a2u(nlu->settings.szProxyAuthPassword);
			}

			szAuthHdr = NtlmCreateResponseFromChallenge(m_hNtlmSecurity, szChallenge, szLogin, szPassw, true, complete);
			if (!szAuthHdr)
				Netlib_Logf(nullptr, "Security login %s failed, user: %S pssw: %S", szProvider, szLogin ? szLogin.get() : L"(no user)", szPassw ? L"(exist)" : L"(no psw)");
			else if (justCreated)
				proxyAuthList.add(m_szHost, m_szProvider);
		}
		else complete = 1;

		return szAuthHdr;
	}
};

static int HttpPeekFirstResponseLine(NetlibConnection *nlc, uint32_t dwTimeoutTime, uint32_t recvFlags, int *resultCode, char **ppszResultDescr, int *length)
{
	int bytesPeeked;
	char buffer[2048], *peol;

	while (true) {
		bytesPeeked = RecvWithTimeoutTime(nlc, dwTimeoutTime, buffer, _countof(buffer) - 1, MSG_PEEK | recvFlags);
		if (bytesPeeked == 0) {
			SetLastError(ERROR_HANDLE_EOF);
			return 0;
		}
		if (bytesPeeked == SOCKET_ERROR)
			return 0;

		buffer[bytesPeeked] = '\0';
		if ((peol = strchr(buffer, '\n')) != nullptr)
			break;

		int iLen = (int)mir_strlen(buffer);
		if (iLen < bytesPeeked) {
			Netlib_Logf(nlc->nlu, "%s %d: %s Failed (%u %u)", __FILE__, __LINE__, "HttpPeekFirstResponseLine", iLen, bytesPeeked);
			SetLastError(ERROR_BAD_FORMAT);
			return 0;
		}
		if (bytesPeeked == _countof(buffer) - 1) {
			SetLastError(ERROR_BUFFER_OVERFLOW);
			return 0;
		}
		if (Miranda_IsTerminated())
			return 0;
		Sleep(10);
	}

	if (peol == buffer) {
		Netlib_Logf(nlc->nlu, "%s %d: %s Failed", __FILE__, __LINE__, "HttpPeekFirstResponseLine");
		SetLastError(ERROR_BAD_FORMAT);
		return 0;
	}

	*peol = '\0';

	if (_strnicmp(buffer, "HTTP/", 5)) {
		Netlib_Logf(nlc->nlu, "%s %d: %s Failed", __FILE__, __LINE__, "HttpPeekFirstResponseLine");
		SetLastError(ERROR_BAD_FORMAT);
		return 0;
	}

	size_t off = strcspn(buffer, " \t");
	if (off >= (unsigned)bytesPeeked)
		return 0;

	char *pResultCode = buffer + off;
	*(pResultCode++) = 0;

	char *pResultDescr;
	*resultCode = strtol(pResultCode, &pResultDescr, 10);

	if (ppszResultDescr)
		*ppszResultDescr = mir_strdup(lrtrimp(pResultDescr));

	if (length)
		*length = peol - buffer + 1;
	return 1;
}

static int SendHttpRequestAndData(NetlibConnection *nlc, CMStringA &httpRequest, MHttpRequest *nlhr, int sendContentLengthHeader)
{
	bool sendData = (nlhr->requestType == REQUEST_POST || nlhr->requestType == REQUEST_PUT || nlhr->requestType == REQUEST_PATCH);

	if (sendContentLengthHeader && sendData)
		httpRequest.AppendFormat("Content-Length: %d\r\n\r\n", nlhr->m_szParam.GetLength());
	else
		httpRequest.AppendFormat("\r\n");

	uint32_t hflags = (nlhr->flags & NLHRF_DUMPASTEXT ? MSG_DUMPASTEXT : 0) |
		(nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPSEND | NLHRF_NODUMPHEADERS) ?
	MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
					 (nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

	int bytesSent = Netlib_Send(nlc, httpRequest, httpRequest.GetLength(), hflags);
	if (bytesSent != SOCKET_ERROR && sendData && nlhr->m_szParam.GetLength()) {
		uint32_t sflags = MSG_NOTITLE | (nlhr->flags & NLHRF_DUMPASTEXT ? MSG_DUMPASTEXT : 0) |
			(nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPSEND) ?
		MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
						 (nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

		int sendResult = Netlib_Send(nlc, nlhr->m_szParam, nlhr->m_szParam.GetLength(), sflags);

		bytesSent = sendResult != SOCKET_ERROR ? bytesSent + sendResult : SOCKET_ERROR;
	}

	return bytesSent;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Receives HTTP headers
//
// Returns a pointer to a MHttpRequest structure on success, NULL on failure.
// Call Netlib_FreeHttpRequest() to free this.
// hConnection must have been returned by MS_NETLIB_OPENCONNECTION
// nlhr->pData = NULL and nlhr->dataLength = 0 always. The requested data should
// be retrieved using MS_NETLIB_RECV once the header has been parsed.
// If the headers haven't finished within 60 seconds the function returns NULL
// and ERROR_TIMEOUT.
// Errors: ERROR_INVALID_PARAMETER, any from MS_NETLIB_RECV or select()
//    ERROR_HANDLE_EOF (connection closed before headers complete)
//    ERROR_TIMEOUT (headers still not complete after 60 seconds)
//    ERROR_BAD_FORMAT (invalid character or line ending in headers, or first line is blank)
//    ERROR_BUFFER_OVERFLOW (each header line must be less than 4096 chars long)
//    ERROR_INVALID_DATA (first header line is malformed ("http/[01].[0-9] [0-9]+ .*", or no colon in subsequent line)

#define NHRV_BUF_SIZE 8192

static MHttpResponse* Netlib_RecvHttpHeaders(NetlibConnection *nlc, int flags)
{
	if (!NetlibEnterNestedCS(nlc, NLNCS_RECV))
		return nullptr;

	uint32_t dwRequestTimeoutTime = GetTickCount() + HTTPRECVDATATIMEOUT;
	std::unique_ptr<MHttpResponse> nlhr(new MHttpResponse());
	nlhr->nlc = nlc;  // Needed to id connection in the protocol HTTP gateway wrapper functions

	int firstLineLength = 0;
	if (!HttpPeekFirstResponseLine(nlc, dwRequestTimeoutTime, flags | MSG_PEEK, &nlhr->resultCode, &nlhr->szResultDescr, &firstLineLength)) {
		NetlibLeaveNestedCS(&nlc->ncsRecv);
		return nullptr;
	}

	char *buffer = (char *)_alloca(NHRV_BUF_SIZE + 1);
	int bytesPeeked = Netlib_Recv(nlc, buffer, min(firstLineLength, NHRV_BUF_SIZE), flags | MSG_DUMPASTEXT);
	if (bytesPeeked != firstLineLength) {
		NetlibLeaveNestedCS(&nlc->ncsRecv);
		if (bytesPeeked != SOCKET_ERROR)
			SetLastError(ERROR_HANDLE_EOF);
		return nullptr;
	}

	// Make sure all headers arrived
	MBinBuffer buf;
	int headersCount = 0;
	bytesPeeked = 0;
	for (bool headersCompleted = false; !headersCompleted;) {
		bytesPeeked = RecvWithTimeoutTime(nlc, dwRequestTimeoutTime, buffer, NHRV_BUF_SIZE, flags | MSG_DUMPASTEXT | MSG_NOTITLE);
		if (bytesPeeked == 0)
			break;

		if (bytesPeeked == SOCKET_ERROR) {
			bytesPeeked = 0;
			break;
		}

		buf.append(buffer, bytesPeeked);

		headersCount = 0;
		for (char *pbuffer = (char *)buf.data();; headersCount++) {
			char *peol = strchr(pbuffer, '\n');
			if (peol == nullptr) break;
			if (peol == pbuffer || (peol == (pbuffer + 1) && *pbuffer == '\r')) {
				bytesPeeked = peol - (char *)buf.data() + 1;
				headersCompleted = true;
				break;
			}
			pbuffer = peol + 1;
		}
	}

	if (bytesPeeked <= 0) {
		NetlibLeaveNestedCS(&nlc->ncsRecv);
		return nullptr;
	}

	// Receive headers
	headersCount = 0;
	for (char *pbuffer = (char *)buf.data();; headersCount++) {
		char *peol = strchr(pbuffer, '\n');
		if (peol == nullptr || peol == pbuffer || (peol == (pbuffer + 1) && *pbuffer == '\r'))
			break;
		*peol = 0;

		char *pColon = strchr(pbuffer, ':');
		if (pColon == nullptr) {
			SetLastError(ERROR_INVALID_DATA);
			return nullptr;
		}

		*pColon = 0;
		nlhr->insert(new MHttpHeader(rtrim(pbuffer), lrtrimp(pColon + 1)));
		pbuffer = peol + 1;
	}

	// remove processed data
	buf.remove(bytesPeeked);
	nlc->foreBuf.appendBefore(buf.data(), buf.length());

	NetlibLeaveNestedCS(&nlc->ncsRecv);
	return nlhr.release();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a HTTP request over a connection
//
// Returns number of bytes sent on success, SOCKET_ERROR on failure
// hConnection must have been returned by MS_NETLIB_OPENCONNECTION
// Note that if you use NLHRF_SMARTAUTHHEADER and NTLM authentication is in use
// then the full NTLM authentication transaction occurs, comprising sending the
// domain, receiving the challenge, then sending the response.
// nlhr.resultCode and nlhr.szResultDescr are ignored by this function.
// Errors: ERROR_INVALID_PARAMETER, anything returned by MS_NETLIB_SEND

int Netlib_SendHttpRequest(HNETLIBCONN nlc, MHttpRequest *nlhr, MChunkHandler &pHandler)
{
	MHttpResponse *nlhrReply = nullptr;
	HttpSecurityContext httpSecurity;

	char *szHost = nullptr, *szNewUrl = nullptr;
	char *pszProxyAuthHdr = nullptr, *pszAuthHdr = nullptr;
	int bytesSent = 0;
	bool lastFirstLineFail = false;

	if (nlhr == nullptr || nlhr->m_szUrl.IsEmpty()) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	NetlibUser *nlu = nlc->nlu;
	if (GetNetlibHandleType(nlu) != NLH_USER) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	int hdrTimeout = (nlhr->timeout) ? nlhr->timeout : HTTPRECVHEADERSTIMEOUT;

	const char *pszRequest;
	switch (nlhr->requestType) {
	case REQUEST_GET:     pszRequest = "GET";     break;
	case REQUEST_POST:    pszRequest = "POST";    break;
	case REQUEST_CONNECT: pszRequest = "CONNECT"; break;
	case REQUEST_HEAD:    pszRequest = "HEAD";    break;
	case REQUEST_PUT:     pszRequest = "PUT";     break;
	case REQUEST_DELETE:  pszRequest = "DELETE";  break;
	case REQUEST_PATCH:   pszRequest = "PATCH";   break;
	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	if (!NetlibEnterNestedCS(nlc, NLNCS_SEND))
		return SOCKET_ERROR;

	const char *pszFullUrl = nlhr->m_szUrl;
	const char *pszUrl = nullptr;

	unsigned complete = false;
	int count = 11;
	while (--count) {
		if (GetNetlibHandleType(nlc) != NLH_CONNECTION) {
			nlc = nullptr;
			bytesSent = SOCKET_ERROR;
			break;
		}

		if (!NetlibReconnect(nlc)) {
			bytesSent = SOCKET_ERROR;
			break;
		}

		if (!pszUrl) {
			pszUrl = pszFullUrl;
			if (!(nlhr->flags & NLHRF_MANUALHOST)) {
				bool usingProxy = nlc->proxyType == PROXYTYPE_HTTP && !(nlhr->flags & NLHRF_SSL);

				const char *ppath, *phost;
				phost = strstr(pszUrl, "://");
				if (phost == nullptr) phost = pszUrl;
				else phost += 3;
				ppath = strchr(phost, '/');
				if (ppath == phost)
					phost = nullptr;

				replaceStr(szHost, phost);
				if (ppath && phost)
					szHost[ppath - phost] = 0;

				if ((nlhr->flags & NLHRF_SMARTREMOVEHOST) && !usingProxy)
					pszUrl = ppath ? ppath : "/";

				if (usingProxy && phost && !nlc->dnsThroughProxy) {
					char *tszHost = mir_strdup(phost);
					if (ppath)
						tszHost[ppath - phost] = 0;
					char *cln = strchr(tszHost, ':'); if (cln) *cln = 0;

					if (inet_addr(tszHost) == INADDR_NONE) {
						in_addr ip;
						if (ip.S_un.S_addr = DnsLookup(nlu, tszHost)) {
							mir_free(szHost);
							if (cln) *cln = ':';
							szHost = CMStringA(FORMAT, "%s%s", inet_ntoa(ip), cln ? cln : "").Detach();
						}
					}
					mir_free(tszHost);
				}
			}
		}

		if (nlc->proxyAuthNeeded && proxyAuthList.getCount()) {
			if (httpSecurity.m_szProvider == nullptr && nlc->szProxyServer) {
				const char *szAuthMethodNlu = proxyAuthList.find(nlc->szProxyServer);
				if (szAuthMethodNlu) {
					mir_free(pszProxyAuthHdr);
					pszProxyAuthHdr = httpSecurity.Execute(nlc, nlc->szProxyServer, szAuthMethodNlu, "", complete);
				}
			}
		}
		nlc->proxyAuthNeeded = false;

		CMStringA httpRequest(FORMAT, "%s %s HTTP/1.%d\r\n", pszRequest, pszUrl, (nlhr->flags & NLHRF_HTTP11) != 0);

		// HTTP headers
		bool doneHostHeader = false, doneContentLengthHeader = false, doneProxyAuthHeader = false, doneAuthHeader = false, doneConnection = false;
		for (auto &it : *nlhr) {
			if (!mir_strcmpi(it->szName, "Host")) doneHostHeader = true;
			else if (!mir_strcmpi(it->szName, "Content-Length")) doneContentLengthHeader = true;
			else if (!mir_strcmpi(it->szName, "Proxy-Authorization")) doneProxyAuthHeader = true;
			else if (!mir_strcmpi(it->szName, "Authorization")) doneAuthHeader = true;
			else if (!mir_strcmpi(it->szName, "Connection")) doneConnection = true;
			if (it->szValue == nullptr) continue;
			httpRequest.AppendFormat("%s: %s\r\n", it->szName.get(), it->szValue.get());
		}
		if (szHost && !doneHostHeader)
			httpRequest.AppendFormat("%s: %s\r\n", "Host", szHost);
		if (pszProxyAuthHdr && !doneProxyAuthHeader)
			httpRequest.AppendFormat("%s: %s\r\n", "Proxy-Authorization", pszProxyAuthHdr);
		if (pszAuthHdr && !doneAuthHeader)
			httpRequest.AppendFormat("%s: %s\r\n", "Authorization", pszAuthHdr);
		if (!doneConnection)
			httpRequest.AppendFormat("%s: %s\r\n", "Connection", "Keep-Alive");
		httpRequest.AppendFormat("%s: %s\r\n", "Proxy-Connection", "Keep-Alive");

		// Add Sticky Headers
		if (nlu->szStickyHeaders != nullptr)
			httpRequest.AppendFormat("%s\r\n", nlu->szStickyHeaders);

		// send it
		bytesSent = SendHttpRequestAndData(nlc, httpRequest, nlhr, !doneContentLengthHeader);
		if (bytesSent == SOCKET_ERROR)
			break;

		// ntlm reply
		if (doneContentLengthHeader && nlhr->requestType != REQUEST_HEAD)
			break;

		int resultCode;
		uint32_t fflags = MSG_PEEK | MSG_NODUMP | ((nlhr->flags & NLHRF_NOPROXY) ? MSG_RAW : 0);
		uint32_t dwTimeOutTime = hdrTimeout < 0 ? -1 : GetTickCount() + hdrTimeout;
		if (!HttpPeekFirstResponseLine(nlc, dwTimeOutTime, fflags, &resultCode, nullptr, nullptr)) {
			uint32_t err = GetLastError();
			Netlib_Logf(nlu, "%s %d: %s Failed (%u %u)", __FILE__, __LINE__, "HttpPeekFirstResponseLine", err, count);

			// connection died while we were waiting
			if (GetNetlibHandleType(nlc) != NLH_CONNECTION) {
				nlc = nullptr;
				break;
			}

			if (err == ERROR_TIMEOUT || err == ERROR_BAD_FORMAT || err == ERROR_BUFFER_OVERFLOW || lastFirstLineFail || nlc->termRequested || nlhr->requestType == REQUEST_CONNECT) {
				bytesSent = SOCKET_ERROR;
				break;
			}

			lastFirstLineFail = true;
			continue;
		}

		lastFirstLineFail = false;

		uint32_t hflags = (nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPHEADERS | NLHRF_NODUMPSEND) ?
		MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
						 (nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

		uint32_t dflags = (nlhr->flags & (NLHRF_NODUMP | NLHRF_NODUMPSEND) ? MSG_NODUMP : MSG_DUMPASTEXT | MSG_DUMPPROXY) |
			(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0) | MSG_NODUMP;

		if (resultCode == 100)
			nlhrReply = Netlib_RecvHttpHeaders(nlc, hflags);

		else if (resultCode == 307 || ((resultCode == 301 || resultCode == 302) && (nlhr->flags & NLHRF_REDIRECT))) { // redirect
			pszUrl = nullptr;

			if (nlhr->requestType == REQUEST_HEAD)
				nlhrReply = Netlib_RecvHttpHeaders(nlc, hflags);
			else
				nlhrReply = NetlibHttpRecv(nlc, hflags, dflags, pHandler);

			if (nlhrReply) {
				auto *tmpUrl = nlhrReply->FindHeader("Location");
				if (tmpUrl) {
					size_t rlen = 0;
					if (tmpUrl[0] == '/') {
						const char *ppath, *phost;
						phost = strstr(pszFullUrl, "://");
						phost = phost ? phost + 3 : pszFullUrl;
						ppath = strchr(phost, '/');
						rlen = ppath ? ppath - pszFullUrl : mir_strlen(pszFullUrl);
					}

					nlc->szNewUrl = (char*)mir_realloc(nlc->szNewUrl, rlen + mir_strlen(tmpUrl) * 3 + 1);

					strncpy(nlc->szNewUrl, pszFullUrl, rlen);
					mir_strcpy(nlc->szNewUrl + rlen, tmpUrl);
					pszFullUrl = nlc->szNewUrl;
					pszUrl = nullptr;

					if (NetlibHttpProcessUrl(nlhr, nlu, nlc, pszFullUrl) == nullptr) {
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
				nlhrReply = Netlib_RecvHttpHeaders(nlc, hflags);
			else
				nlhrReply = NetlibHttpRecv(nlc, hflags, dflags, pHandler);

			replaceStr(pszAuthHdr, nullptr);
			if (nlhrReply) {
				char *szAuthStr = nullptr;
				if (!complete) {
					szAuthStr = NetlibHttpFindAuthHeader(nlhrReply, "WWW-Authenticate", httpSecurity.m_szProvider);
					if (szAuthStr) {
						char *szChallenge = strchr(szAuthStr, ' ');
						if (!szChallenge || !*lrtrimp(szChallenge))
							complete = true;
					}
				}
				if (complete && httpSecurity.m_hNtlmSecurity)
					szAuthStr = httpSecurity.TryBasic() ? NetlibHttpFindAuthHeader(nlhrReply, "WWW-Authenticate", "Basic") : nullptr;

				if (szAuthStr) {
					char *szChallenge = strchr(szAuthStr, ' ');
					if (szChallenge) { *szChallenge = 0; szChallenge = lrtrimp(szChallenge + 1); }

					pszAuthHdr = httpSecurity.Execute(nlc, szHost, szAuthStr, szChallenge, complete);
				}
			}
			if (pszAuthHdr == nullptr) {
				proxyAuthList.add(szHost, nullptr);
				NetlibHttpSetLastErrorUsingHttpResult(resultCode);
				bytesSent = SOCKET_ERROR;
				break;
			}
		}
		else if (resultCode == 407 && !doneProxyAuthHeader) { //proxy auth required
			if (nlhr->requestType == REQUEST_HEAD)
				nlhrReply = Netlib_RecvHttpHeaders(nlc, hflags);
			else
				nlhrReply = NetlibHttpRecv(nlc, hflags, dflags, pHandler);

			mir_free(pszProxyAuthHdr); pszProxyAuthHdr = nullptr;
			if (nlhrReply) {
				char *szAuthStr = nullptr;
				if (!complete) {
					szAuthStr = NetlibHttpFindAuthHeader(nlhrReply, "Proxy-Authenticate", httpSecurity.m_szProvider);
					if (szAuthStr) {
						char *szChallenge = strchr(szAuthStr, ' ');
						if (!szChallenge || !*lrtrimp(szChallenge + 1))
							complete = true;
					}
				}
				if (complete && httpSecurity.m_hNtlmSecurity)
					szAuthStr = httpSecurity.TryBasic() ? NetlibHttpFindAuthHeader(nlhrReply, "Proxy-Authenticate", "Basic") : nullptr;

				if (szAuthStr) {
					char *szChallenge = strchr(szAuthStr, ' ');
					if (szChallenge) { *szChallenge = 0; szChallenge = lrtrimp(szChallenge + 1); }

					pszProxyAuthHdr = httpSecurity.Execute(nlc, nlc->szProxyServer, szAuthStr, szChallenge, complete);
				}
			}
			if (pszProxyAuthHdr == nullptr) {
				proxyAuthList.add(nlc->szProxyServer, nullptr);
				NetlibHttpSetLastErrorUsingHttpResult(resultCode);
				bytesSent = SOCKET_ERROR;
				break;
			}
		}
		else break;

		if (pszProxyAuthHdr && resultCode != 407 && !doneProxyAuthHeader)
			replaceStr(pszProxyAuthHdr, nullptr);

		if (pszAuthHdr && resultCode != 401 && !doneAuthHeader)
			replaceStr(pszAuthHdr, nullptr);

		if (nlhrReply) {
			delete nlhrReply;
			nlhrReply = nullptr;
		}
	}

	if (count == 0) bytesSent = SOCKET_ERROR;

	delete nlhrReply;

	//clean up
	mir_free(pszProxyAuthHdr);
	mir_free(pszAuthHdr);
	mir_free(szHost);
	mir_free(szNewUrl);

	if (nlc)
		NetlibLeaveNestedCS(&nlc->ncsSend);

	return bytesSent;
}

void NetlibHttpSetLastErrorUsingHttpResult(int result)
{
	if (result >= 200 && result < 300) {
		SetLastError(ERROR_SUCCESS);
		return;
	}
	switch (result) {
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

static char* gzip_decode(char *gzip_data, int &len_ptr, int window)
{
	int gzip_len = len_ptr * 5;
	char* output_data = nullptr;

	int gzip_err;
	z_stream zstr;

	do {
		output_data = (char*)mir_realloc(output_data, gzip_len+1);
		if (output_data == nullptr)
			break;

		zstr.next_in = (Bytef*)gzip_data;
		zstr.avail_in = len_ptr;
		zstr.zalloc = Z_NULL;
		zstr.zfree = Z_NULL;
		zstr.opaque = Z_NULL;
		inflateInit2_(&zstr, window, ZLIB_VERSION, sizeof(z_stream));

		zstr.next_out = (Bytef*)output_data;
		zstr.avail_out = gzip_len;

		gzip_err = inflate(&zstr, Z_FINISH);

		inflateEnd(&zstr);
		gzip_len *= 2;
		if (gzip_len > 10000000)
			break;
	} while (gzip_err == Z_BUF_ERROR);

	gzip_len = gzip_err == Z_STREAM_END ? zstr.total_out : -1;

	if (gzip_len <= 0) {
		mir_free(output_data);
		output_data = nullptr;
	}
	else output_data[gzip_len] = 0;

	len_ptr = gzip_len;
	return output_data;
}

static int NetlibHttpRecvChunkHeader(NetlibConnection *nlc, bool first, uint32_t flags)
{
	MBinBuffer buf;

	while (true) {
		char data[1000];
		int recvResult = Netlib_Recv(nlc, data, _countof(data) - 1, MSG_RAW | flags);
		if (recvResult <= 0 || recvResult >= _countof(data))
			return SOCKET_ERROR;

		buf.append(data, recvResult); // add chunk

		auto *peol1 = (const char*)memchr(buf.data(), '\n', buf.length());
		if (peol1 == nullptr)
			continue;

		auto *pStart = (const char *)buf.data();
		int cbRest = int(peol1 - pStart) + 1;
		const char *peol2 = first ? peol1 : (const char*)memchr(peol1 + 1, '\n', buf.length() - cbRest);
		if (peol2 == nullptr)
			continue;

		int sz = peol2 - pStart + 1;
		int r = strtol(first ? pStart : peol1 + 1, nullptr, 16);
		if (r == 0) {
			const char *peol3 = strchr(peol2 + 1, '\n');
			if (peol3 == nullptr)
				continue;
			sz = peol3 - pStart + 1;
		}
		buf.remove(sz); // remove all our data from buffer
		nlc->foreBuf.appendBefore(buf.data(), buf.length());
		return r;
	}
}

MHttpResponse* NetlibHttpRecv(NetlibConnection *nlc, uint32_t hflags, uint32_t dflags, MChunkHandler &pHandler, bool isConnect)
{
	int dataLen = -1;
	bool chunked = false;
	int cenc = 0, cenctype = 0, close = 0;

next:
	std::unique_ptr<MHttpResponse> nlhrReply(Netlib_RecvHttpHeaders(nlc, hflags));
	if (nlhrReply == nullptr)
		return nullptr;

	if (nlhrReply->resultCode == 100)
		goto next;

	if (nlhrReply->resultCode == 204)
		dataLen = 0;

	for (auto &it : *nlhrReply) {
		if (!mir_strcmpi(it->szName, "Content-Length"))
			dataLen = atoi(it->szValue);

		if (!mir_strcmpi(it->szName, "Content-Encoding")) {
			cenc = nlhrReply->indexOf(&it);
			if (strstr(it->szValue, "gzip"))
				cenctype = 1;
			else if (strstr(it->szValue, "deflate"))
				cenctype = 2;
		}

		if (!mir_strcmpi(it->szName, "Connection"))
			close = mir_strcmpi(it->szValue, "close") == 0;

		if (!mir_strcmpi(it->szName, "Transfer-Encoding") && !mir_strcmpi(it->szValue, "chunked")) {
			chunked = true;
			dataLen = -1;
		}
	}

	if (nlhrReply->resultCode >= 200 && (dataLen > 0 || (!isConnect && dataLen < 0))) {
		if (chunked)
			dataLen = NetlibHttpRecvChunkHeader(nlc, true, dflags | (cenctype ? MSG_NODUMP : 0));
		if (dataLen == SOCKET_ERROR)
			return nullptr;

		ptrA tmpBuf((char *)mir_alloc(65536));
		while (dataLen != 0) {
			// fetching one chunk
			while (true) {
				int recvResult = RecvWithTimeoutTime(nlc, GetTickCount() + HTTPRECVDATATIMEOUT,
					tmpBuf, 65536, dflags | (cenctype ? MSG_NODUMP : 0));

				if (recvResult == 0) break;
				if (recvResult == SOCKET_ERROR)
					return nullptr;

				if (recvResult <= dataLen) {
					if (!pHandler.updateChunk(tmpBuf, recvResult))
						return nullptr;

					dataLen -= recvResult;
					if (!dataLen)
						break;
				}
				else {
					if (!pHandler.updateChunk(tmpBuf, dataLen))
						return nullptr;

					nlc->foreBuf.appendBefore(tmpBuf.get() + dataLen, recvResult - dataLen);
					break;
				}
				
				Sleep(10);
			}

			if (!chunked)
				break;

			dataLen = NetlibHttpRecvChunkHeader(nlc, false, dflags | MSG_NODUMP);
			if (dataLen == SOCKET_ERROR)
				return nullptr;
		}
	}

	pHandler.apply(nlhrReply.get());

	if (chunked)
		nlhrReply->AddHeader("Content-Length", CMStringA(FORMAT, "%u", dataLen));

	if (cenctype && !nlhrReply->body.IsEmpty()) {
		int bufsz = nlhrReply->body.GetLength();
		char* szData = nullptr;

		switch (cenctype) {
		case 1:
			szData = gzip_decode(nlhrReply->body.GetBuffer(), bufsz, 0x10 | MAX_WBITS);
			break;

		case 2:
			szData = gzip_decode(nlhrReply->body.GetBuffer(), bufsz, -MAX_WBITS);
			if (bufsz < 0) {
				bufsz = nlhrReply->body.GetLength();
				szData = gzip_decode(nlhrReply->body.GetBuffer(), bufsz, MAX_WBITS);
			}
			break;
		}

		if (bufsz > 0) {
			Netlib_Dump(nlc, (uint8_t*)szData, bufsz, false, dflags | MSG_NOTITLE);
			nlhrReply->body.Truncate(bufsz + 1);
			memcpy(nlhrReply->body.GetBuffer(), szData, bufsz);
			nlhrReply->body.SetAt(bufsz, 0);

			nlhrReply->remove(cenc);
		}
		else if (bufsz == 0)
			nlhrReply->body.Empty();
	}

	if (close &&
		(nlc->proxyType != PROXYTYPE_HTTP || nlc->url.flags & NLOCF_SSL) &&
		(!isConnect || nlhrReply->resultCode != 200))
		NetlibDoCloseSocket(nlc);

	return nlhrReply.release();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

static MHttpResponse* HttpTransactionWorker(HNETLIBUSER nlu, MHttpRequest *nlhr, MChunkHandler &pHandler)
{
	if (GetNetlibHandleType(nlu) != NLH_USER || !(nlu->user.flags & NUF_OUTGOING) || !nlhr || nlhr->m_szUrl.IsEmpty()) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}

	if (nlhr->nlc != nullptr && GetNetlibHandleType(nlhr->nlc) != NLH_CONNECTION)
		nlhr->nlc = nullptr;

	NetlibConnection *nlc = NetlibHttpProcessUrl(nlhr, nlu, nlhr->nlc);
	if (nlc == nullptr)
		return nullptr;

	nlhr->flags |= NLHRF_SMARTREMOVEHOST;

	if (!nlhr->m_szParam.IsEmpty() && nlhr->requestType == REQUEST_GET) {
		nlhr->m_szUrl.AppendChar('?');
		nlhr->m_szUrl += nlhr->m_szParam;
		nlhr->m_szParam.Empty();
	}

	if (!nlhr->FindHeader("User-Agent")) {
		char szUserAgent[64], szMirandaVer[64];
		strncpy_s(szMirandaVer, MIRANDA_VERSION_STRING, _TRUNCATE);
		#if defined(_WIN64)
		strncat_s(szMirandaVer, " x64", _TRUNCATE);
		#endif

		char *pspace = strchr(szMirandaVer, ' ');
		if (pspace) {
			*pspace++ = '\0';
			mir_snprintf(szUserAgent, "Miranda/%s (%s)", szMirandaVer, pspace);
		}
		else mir_snprintf(szUserAgent, "Miranda/%s", szMirandaVer);

		nlhr->AddHeader("User-Agent", szUserAgent);
	}

	if (!nlhr->FindHeader("Accept-Encoding"))
		nlhr->AddHeader("Accept-Encoding", "deflate, gzip");

	if (Netlib_SendHttpRequest(nlc, nlhr, pHandler) == SOCKET_ERROR) {
		Netlib_CloseHandle(nlc);
		return nullptr;
	}

	uint32_t dflags = (nlhr->flags & NLHRF_DUMPASTEXT ? MSG_DUMPASTEXT : 0) |
		(nlhr->flags & NLHRF_NODUMP ? MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
		(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

	uint32_t hflags =
		(nlhr->flags & NLHRF_NODUMP ? MSG_NODUMP : (nlhr->flags & NLHRF_DUMPPROXY ? MSG_DUMPPROXY : 0)) |
		(nlhr->flags & NLHRF_NOPROXY ? MSG_RAW : 0);

	MHttpResponse *nlhrReply;
	if (nlhr->requestType == REQUEST_HEAD)
		nlhrReply = Netlib_RecvHttpHeaders(nlc, 0);
	else
		nlhrReply = NetlibHttpRecv(nlc, hflags, dflags, pHandler);

	if (nlhrReply) {
		nlhrReply->szUrl = nlc->szNewUrl;
		nlc->szNewUrl = nullptr;
	}

	if ((nlhr->flags & NLHRF_PERSISTENT) == 0 || nlhrReply == nullptr) {
		Netlib_CloseHandle(nlc);
		if (nlhrReply)
			nlhrReply->nlc = nullptr;
	}
	else nlhrReply->nlc = nlc;

	return nlhrReply;
}

/////////////////////////////////////////////////////////////////////////////////////////

void MMemoryChunkStorage::apply(MHttpResponse *nlhr)
{
	unsigned dataLen = (unsigned)buf.length();
	nlhr->body.Truncate(dataLen);
	memcpy(nlhr->body.GetBuffer(), buf.data(), dataLen);
}

bool MMemoryChunkStorage::updateChunk(const void *pData, size_t cbLen)
{
	buf.append(pData, cbLen);
	return true;
}

MIR_APP_DLL(MHttpResponse *) Netlib_HttpTransaction(HNETLIBUSER nlu, MHttpRequest *nlhr)
{
	MMemoryChunkStorage storage;
	return HttpTransactionWorker(nlu, nlhr, storage);
}

/////////////////////////////////////////////////////////////////////////////////////////

MFileChunkStorage::MFileChunkStorage(const MFilePath &_1, pfnDownloadCallback _2, void *_3) :
	pCallback(_2),
	pCallbackInfo(_3)
{
	fileId = _wopen(_1, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
}

MFileChunkStorage::~MFileChunkStorage()
{
	if (fileId != -1) {
		_close(fileId);
		fileId = -1;
	}
}

void MFileChunkStorage::apply(MHttpResponse *nlhr)
{
	if (fileId != -1) {
		nlhr->resultCode = 200;
		nlhr->body = "OK";
	}
	else nlhr->resultCode = 500;
}

bool MFileChunkStorage::updateChunk(const void *pData, size_t cbLen)
{
	if (cbLen != (unsigned)_write(fileId, pData, unsigned(cbLen))) {
		_close(fileId);
		fileId = -1;
		return false;
	}

	if (pCallback) {
		size_t iProgress = _filelength(fileId);
		size_t nBlocks = iProgress / 65536;
		if (nBlocks != prevBlocks) {
			prevBlocks = nBlocks;
			pCallback(iProgress, pCallbackInfo);
		}
	}
	return true;
}

MIR_APP_DLL(MHttpResponse *) Netlib_DownloadFile(
	HNETLIBUSER nlu,
	MHttpRequest *nlhr,
	const MFilePath &wszFileName,
	pfnDownloadCallback pCallback,
	void *pCallbackInfo)
{
	MFileChunkStorage storage(wszFileName, pCallback, pCallbackInfo);
	if (!storage)
		return nullptr;

	return HttpTransactionWorker(nlu, nlhr, storage);
}
