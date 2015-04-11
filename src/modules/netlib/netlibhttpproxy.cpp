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

typedef enum
{
	reqHelloGet,
	reqOldGet,
	reqOldPost,
	reqNewPost,
}
RequestType;

static int HttpGatewayReadSetResult(NetlibConnection *nlc, char *buf, int num, int peek)
{
	if (nlc->dataBufferLen == 0) return 0;

	int bytes = min(num, nlc->dataBufferLen);
	int rbytes = nlc->dataBufferLen - bytes;

	memcpy(buf, nlc->dataBuffer, bytes);
	if (!peek) {
		memmove(nlc->dataBuffer, nlc->dataBuffer + bytes, rbytes);
		nlc->dataBufferLen = rbytes;
	}

	return bytes;
}

void HttpGatewayRemovePacket(NetlibConnection *nlc, int pck)
{
	mir_cslock lck(nlc->csHttpSequenceNums);
	while (pck-- && nlc->pHttpProxyPacketQueue != NULL) {
		NetlibHTTPProxyPacketQueue *p = nlc->pHttpProxyPacketQueue;
		nlc->pHttpProxyPacketQueue = nlc->pHttpProxyPacketQueue->next;

		mir_free(p->dataBuffer);
		mir_free(p);
	}
}

static bool NetlibHttpGatewaySend(NetlibConnection *nlc, RequestType reqType, const char *buf, int len)
{
	NETLIBHTTPREQUEST nlhrSend = { 0 };
	char szUrl[512];

	nlhrSend.cbSize = sizeof(nlhrSend);
	nlhrSend.nlc = nlc;

	nlhrSend.pData = (char*)buf;
	nlhrSend.dataLength = len;

	nlhrSend.flags = NLHRF_GENERATEHOST | NLHRF_DUMPPROXY | NLHRF_SMARTAUTHHEADER | NLHRF_NOPROXY | NLHRF_REDIRECT;
	if (nlc->nlhpi.flags & NLHPIF_HTTP11)
		nlhrSend.flags |= NLHRF_HTTP11;

	switch (reqType) {
	case reqHelloGet:
		nlhrSend.requestType = REQUEST_GET;
		nlhrSend.szUrl = nlc->nlu->user.szHttpGatewayHello;
		break;

	case reqOldGet:
		nlhrSend.requestType = REQUEST_GET;
		nlhrSend.timeout = -1;
		if ((nlc->nlhpi.flags & NLHPIF_USEGETSEQUENCE) && (nlc->nlhpi.szHttpGetUrl != NULL)) {
			mir_cslock lck(nlc->csHttpSequenceNums);
			mir_snprintf(szUrl, SIZEOF(szUrl), "%s%u", nlc->nlhpi.szHttpGetUrl, nlc->nlhpi.firstGetSequence++);
			if (nlc->nlhpi.flags & NLHPIF_GETPOSTSAMESEQUENCE)
				nlc->nlhpi.firstPostSequence++;
			nlhrSend.szUrl = szUrl;
		}
		else nlhrSend.szUrl = nlc->nlhpi.szHttpGetUrl;
		break;

	case reqOldPost:
		nlhrSend.requestType = REQUEST_POST;
		if ((nlc->nlhpi.flags & NLHPIF_USEPOSTSEQUENCE) && (nlc->nlhpi.szHttpPostUrl != NULL)) {
			mir_snprintf(szUrl, SIZEOF(szUrl), "%s%u", nlc->nlhpi.szHttpPostUrl, nlc->nlhpi.firstPostSequence);
			nlhrSend.szUrl = szUrl;
		}
		else nlhrSend.szUrl = nlc->nlhpi.szHttpPostUrl;
		break;

	case reqNewPost:
		nlhrSend.requestType = REQUEST_POST;
		nlhrSend.szUrl = nlc->nlhpi.szHttpPostUrl;
		break;
	}

	if (nlc->usingDirectHttpGateway) {
		NETLIBOPENCONNECTION nloc;
		NetlibConnFromUrl(nlhrSend.szUrl, false, nloc);

		bool sameHost = mir_strcmp(nlc->nloc.szHost, nloc.szHost) == 0 && nlc->nloc.wPort == nloc.wPort;
		if (!sameHost) {
			NetlibDoClose(nlc);

			mir_free((char*)nlc->nloc.szHost);
			nlc->nloc = nloc;
			if (!NetlibDoConnect(nlc))
				return false;
		}
		else mir_free((char*)nloc.szHost);
	}

	nlhrSend.headersCount = 3;
	nlhrSend.headers = (NETLIBHTTPHEADER*)alloca(sizeof(NETLIBHTTPHEADER) * nlhrSend.headersCount);
	nlhrSend.headers[0].szName = "User-Agent";
	nlhrSend.headers[0].szValue = nlc->nlu->user.szHttpGatewayUserAgent;
	nlhrSend.headers[1].szName = "Cache-Control";
	nlhrSend.headers[1].szValue = "no-cache, no-store ";
	nlhrSend.headers[2].szName = "Pragma";
	nlhrSend.headers[2].szValue = "no-cache";
	return NetlibHttpSendRequest((WPARAM)nlc, (LPARAM)&nlhrSend) != SOCKET_ERROR;
}

static bool NetlibHttpGatewayStdPost(NetlibConnection *nlc, int &numPackets)
{
	int np = 0, len = 0;
	char *buf;
	NetlibHTTPProxyPacketQueue *p;
	{
		mir_cslock lck(nlc->csHttpSequenceNums);

		for (p = nlc->pHttpProxyPacketQueue; p != NULL && np < nlc->nlhpi.combinePackets; p = p->next) {
			np++;
			len += p->dataBufferLen;
		}

		int dlen = 0;
		buf = (char*)alloca(len);
		numPackets = np;

		for (p = nlc->pHttpProxyPacketQueue; np--; p = p->next) {
			memcpy(buf + dlen, p->dataBuffer, p->dataBufferLen);
			dlen += p->dataBufferLen;
		}
	}

	return NetlibHttpGatewaySend(nlc, reqNewPost, buf, len);
}

static bool NetlibHttpGatewayOscarPost(NetlibConnection *nlc, const char *buf, int len, int flags)
{
	NetlibConnection nlcSend = { 0 };
	nlcSend.handleType = NLH_CONNECTION;
	nlcSend.nlu = nlc->nlu;
	nlcSend.nlhpi = nlc->nlhpi;
	nlcSend.s = nlc->s2;
	nlcSend.usingHttpGateway = nlc->usingHttpGateway;
	nlcSend.szProxyServer = nlc->szProxyServer;
	nlcSend.wProxyPort = nlc->wProxyPort;
	nlcSend.proxyType = nlc->proxyType;

	if (!NetlibReconnect(&nlcSend)) return false;
	nlc->s2 = nlcSend.s;

	nlcSend.hOkToCloseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	NetlibInitializeNestedCS(&nlcSend.ncsRecv);
	NetlibInitializeNestedCS(&nlcSend.ncsSend);

	bool res = NetlibHttpGatewaySend(&nlcSend, reqOldPost, buf, len);
	if (res) {
		NETLIBHTTPREQUEST *nlhrReply = NetlibHttpRecv(&nlcSend, flags | MSG_RAW | MSG_DUMPPROXY, MSG_RAW | MSG_DUMPPROXY);
		if (nlhrReply != NULL) {
			if (nlhrReply->resultCode != 200) {
				NetlibHttpSetLastErrorUsingHttpResult(nlhrReply->resultCode);
				res = false;
			}
			NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
		}
		else res = false;
	}

	NetlibDeleteNestedCS(&nlcSend.ncsSend);
	NetlibDeleteNestedCS(&nlcSend.ncsRecv);
	CloseHandle(nlcSend.hOkToCloseEvent);

	nlc->s2 = nlcSend.s;
	mir_free((char*)nlcSend.nloc.szHost);

	mir_cslock lck(nlc->csHttpSequenceNums);
	nlc->nlhpi.firstPostSequence++;
	if (nlc->nlhpi.flags & NLHPIF_GETPOSTSAMESEQUENCE)
		nlc->nlhpi.firstGetSequence++;

	return res;
}

int NetlibHttpGatewayPost(NetlibConnection *nlc, const char *buf, int len, int flags)
{
	if (nlc->nlhpi.szHttpGetUrl != NULL)
		return NetlibHttpGatewayOscarPost(nlc, buf, len, flags) ? len : SOCKET_ERROR;

	/*
	 * Gena01 - many changes here, do compare against the other version.
	 *
	 * Change #1: simplify to use similar code to GET
	 * Change #2: we need to allow to parse POST reply if szHttpGetUrl is NULL
	 * Change #3: Keep connection open if we need to.
	 *
	 * Impact: NONE! Since currently miranda doesn't allow szHttpGetUrl to be NULL, it will not connect
	 *         with the new plugins that use this code.
	 */

	NetlibHTTPProxyPacketQueue *p = (NetlibHTTPProxyPacketQueue*)mir_alloc(sizeof(struct NetlibHTTPProxyPacketQueue));
	p->dataBuffer = (PBYTE)mir_alloc(len);
	memcpy(p->dataBuffer, buf, len);
	p->dataBufferLen = len;
	p->next = NULL;

	/*
	 * Now check to see where to insert this in our queue
	 */

	mir_cslock lck(nlc->csHttpSequenceNums);
	if (nlc->pHttpProxyPacketQueue == NULL)
		nlc->pHttpProxyPacketQueue = p;
	else {
		NetlibHTTPProxyPacketQueue *t = nlc->pHttpProxyPacketQueue;
		while (t->next != NULL)
			t = t->next;
		t->next = p;
	}

	/*
	 * Gena01 - fake a Send!! tell 'em all is ok. We catch errors in Recv.
	 */
	return len;
}

#define NETLIBHTTP_RETRYCOUNT   3
#define NETLIBHTTP_RETRYTIMEOUT 2000

int NetlibHttpGatewayRecv(NetlibConnection *nlc, char *buf, int len, int flags)
{
	bool peek = (flags & MSG_PEEK) != 0;

	if (nlc->dataBufferLen != 0 && (!peek || nlc->dataBufferLen >= len)) {
		return HttpGatewayReadSetResult(nlc, buf, len, peek);
	}

	for (int retryCount = 0; retryCount < NETLIBHTTP_RETRYCOUNT;) {
		if (nlc->nlhpi.szHttpGetUrl == NULL && retryCount == 0) {
			if (nlc->pollingTimeout == 0) nlc->pollingTimeout = 30;

			/* We Need to sleep/wait for the data to send before we do receive */
			for (int pollCount = nlc->pollingTimeout; pollCount--;) {
				if (nlc->pHttpProxyPacketQueue != NULL && GetTickCount() - nlc->lastPost > 1000)
					break;

				if (nlc->termRequested || (SleepEx(1000, TRUE) && Miranda_Terminated()))
					return SOCKET_ERROR;
			}

			nlc->lastPost = GetTickCount();
			if (nlc->pHttpProxyPacketQueue == NULL && nlc->nlu->user.pfnHttpGatewayWrapSend != NULL) {
				if (nlc->nlu->user.pfnHttpGatewayWrapSend(nlc, (PBYTE)"", 0, MSG_NOHTTPGATEWAYWRAP, NetlibSend) == SOCKET_ERROR)
					return SOCKET_ERROR;
			}
		}

		int numPackets = 0;
		if (nlc->nlhpi.szHttpGetUrl) {
			if (!NetlibHttpGatewaySend(nlc, reqOldGet, NULL, 0)) {
				if (GetLastError() == ERROR_ACCESS_DENIED || nlc->termRequested)
					break;

				++retryCount;
				continue;
			}
		}
		else {
			if (!NetlibHttpGatewayStdPost(nlc, numPackets)) {
				if (GetLastError() == ERROR_ACCESS_DENIED || nlc->termRequested)
					break;

				++retryCount;
				continue;
			}
		}
		NETLIBHTTPREQUEST *nlhrReply = NetlibHttpRecv(nlc, flags | MSG_RAW | MSG_DUMPPROXY, MSG_RAW | MSG_DUMPPROXY);
		if (nlhrReply == NULL) return SOCKET_ERROR;

		if (nlc->nlu->user.pfnHttpGatewayUnwrapRecv && !(flags & MSG_NOHTTPGATEWAYWRAP)) {
			nlhrReply->pData = (char*)nlc->nlu->user.pfnHttpGatewayUnwrapRecv(nlhrReply,
				(PBYTE)nlhrReply->pData, nlhrReply->dataLength, &nlhrReply->dataLength, mir_realloc);
		}

		if (nlhrReply->resultCode >= 300) {
			int resultCode = nlhrReply->resultCode;
			NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);

			if (nlc->nlhpi.szHttpGetUrl && resultCode != 404) {
				NetlibLogf(nlc->nlu, "Error received from proxy, retrying");
				continue;
			}
			else {
				NetlibLogf(nlc->nlu, "Error received from proxy, retry attempts exceeded (%u)", retryCount);
				SetLastError(ERROR_GEN_FAILURE);
				return SOCKET_ERROR;
			}
		}
		else {
			retryCount = 0;
			HttpGatewayRemovePacket(nlc, numPackets);
		}

		if (nlhrReply->dataLength) {
			if (peek) {
				int rbytes = nlc->dataBufferLen + nlhrReply->dataLength;

				nlc->dataBuffer = (PBYTE)mir_realloc(nlc->dataBuffer, rbytes);
				memcpy(nlc->dataBuffer + nlc->dataBufferLen, nlhrReply->pData, nlhrReply->dataLength);
				nlc->dataBufferLen = rbytes;

				NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);

				return HttpGatewayReadSetResult(nlc, buf, len, peek);
			}
			else {
				int bytes = min(len, nlhrReply->dataLength);
				int rbytes = nlhrReply->dataLength - bytes;

				memcpy(buf, nlhrReply->pData, bytes);

				nlc->dataBuffer = (PBYTE)mir_realloc(nlc->dataBuffer, rbytes);
				if (rbytes) memcpy(nlc->dataBuffer, nlhrReply->pData + bytes, rbytes);
				nlc->dataBufferLen = rbytes;

				NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
				return bytes;
			}
		}
		else {
			if ((peek && nlc->dataBufferLen != 0) || nlhrReply->pData) {
				NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
				return HttpGatewayReadSetResult(nlc, buf, len, peek);
			}
		}
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
	}

	SetLastError(ERROR_GEN_FAILURE);
	return SOCKET_ERROR;
}

int NetlibInitHttpConnection(NetlibConnection *nlc, NetlibUser *nlu, NETLIBOPENCONNECTION *nloc)
{
	NETLIBHTTPREQUEST *nlhrReply = NULL;
	{
		mir_cslock lck(nlc->csHttpSequenceNums);
		nlc->nlhpi.firstGetSequence = 1;
		nlc->nlhpi.firstPostSequence = 1;
	}

	if (nlu->user.szHttpGatewayHello != NULL) {
		nlc->usingHttpGateway = true;
		if (NetlibHttpGatewaySend(nlc, reqHelloGet, NULL, 0))
			nlhrReply = NetlibHttpRecv(nlc, MSG_DUMPPROXY | MSG_RAW, MSG_DUMPPROXY | MSG_RAW);
		nlc->usingHttpGateway = false;
		if (nlhrReply == NULL) return 0;

		if (nlhrReply->resultCode != 200) {
			NetlibHttpSetLastErrorUsingHttpResult(nlhrReply->resultCode);
			NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
			return 0;
		}
	}
	if (!nlu->user.pfnHttpGatewayInit(nlc, nloc, nlhrReply)) {
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
		return 0;
	}
	NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);

	/*
	 * Gena01 - Ok, we should be able to use just POST. Needed for Yahoo, NO GET requests
	 */
	if (nlc->nlhpi.szHttpPostUrl == NULL) {
		SetLastError(ERROR_BAD_FORMAT);
		return 0;
	}

	nlc->usingHttpGateway = true;

	//now properly connected
	if (nlu->user.pfnHttpGatewayBegin && !nlu->user.pfnHttpGatewayBegin(nlc, nloc))
		return 0;

	return 1;
}

INT_PTR NetlibHttpGatewaySetInfo(WPARAM wParam, LPARAM lParam)
{
	NETLIBHTTPPROXYINFO *nlhpi = (NETLIBHTTPPROXYINFO*)lParam;
	NetlibConnection *nlc = (struct NetlibConnection*)wParam;

	if (GetNetlibHandleType(nlc) != NLH_CONNECTION || nlhpi == NULL ||
		nlhpi->cbSize < (sizeof(NETLIBHTTPPROXYINFO) - sizeof(int)) ||
		nlhpi->szHttpPostUrl == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	mir_free(nlc->nlhpi.szHttpGetUrl);
	mir_free(nlc->nlhpi.szHttpPostUrl);

	nlc->nlhpi.combinePackets = 1;
	memcpy(&nlc->nlhpi, nlhpi, min(nlhpi->cbSize, sizeof(*nlhpi)));
	if (nlc->nlhpi.combinePackets == 0) nlc->nlhpi.combinePackets = 1;

	nlc->nlhpi.szHttpGetUrl = mir_strdup(nlc->nlhpi.szHttpGetUrl);
	nlc->nlhpi.szHttpPostUrl = mir_strdup(nlc->nlhpi.szHttpPostUrl);

	return 1;
}

INT_PTR NetlibHttpSetSticky(WPARAM wParam, LPARAM lParam)
{
	NetlibUser * nu = (NetlibUser*)wParam;
	if (GetNetlibHandleType(nu) != NLH_USER) return ERROR_INVALID_PARAMETER;
	mir_free(nu->szStickyHeaders);
	nu->szStickyHeaders = mir_strdup((char*)lParam); // pointer is ours
	return 0;
}

INT_PTR NetlibHttpSetPollingTimeout(WPARAM wParam, LPARAM lParam)
{
	int oldTimeout;
	NetlibConnection *nlc = (struct NetlibConnection*)wParam;
	if (GetNetlibHandleType(nlc) != NLH_CONNECTION) return -1;
	oldTimeout = nlc->pollingTimeout;
	nlc->pollingTimeout = lParam;
	return oldTimeout;
}
