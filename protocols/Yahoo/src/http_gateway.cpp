/*
 * $Id: http_gateway.cpp 9232 2009-03-26 18:11:02Z ghazan $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#include "yahoo.h"

#ifdef HTTP_GATEWAY

int YAHOO_httpGatewayInit(HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr)
{
	NETLIBHTTPPROXYINFO nlhpi;

	debugLogA("YAHOO_httpGatewayInit!!!");
	
	memset(&nlhpi, 0, sizeof(nlhpi));
	nlhpi.cbSize = sizeof(nlhpi);
	nlhpi.szHttpPostUrl = "http://shttp.msg.yahoo.com/notify/";
	
	return CallService(MS_NETLIB_SETHTTPPROXYINFO, (WPARAM)hConn, (LPARAM)&nlhpi);
}

int YAHOO_httpGatewayWrapSend(HANDLE hConn, PBYTE buf, int len, int flags, MIRANDASERVICE pfnNetlibSend)
{
	debugLogA("YAHOO_httpGatewayWrapSend!!! Len: %d", len);

	if (len == 0 && m_id > 0) { // we need to send something!!!
		int n;
		char *z = yahoo_webmessenger_idle_packet(m_id, &n);
		int ret = 0;
		
		if (z != NULL) {
			debugLogA("YAHOO_httpGatewayWrapSend!!! Got Len: %d", n);
			NETLIBBUFFER tBuf = { ( char* )z, n, flags };
			ret = pfnNetlibSend((LPARAM)hConn, (WPARAM) &tBuf );
			FREE(z);
		} else {
			debugLogA("YAHOO_httpGatewayWrapSend!!! GOT NULL???");
		}
		
		return ret;
	} else {
		NETLIBBUFFER tBuf = { ( char* )buf, len, flags };
		
		return pfnNetlibSend((LPARAM)hConn, (WPARAM) &tBuf );
	}
}

PBYTE YAHOO_httpGatewayUnwrapRecv(NETLIBHTTPREQUEST *nlhr, PBYTE buf, int len, int *outBufLen, void *(*NetlibRealloc)(void *, size_t))
{
    debugLogA("YAHOO_httpGatewayUnwrapRecv!!! Len: %d", len);

    debugLogA("Got headers: %d", nlhr->headersCount);
    /* we need to get the first 4 bytes! */
	if (len < 4) 
		return NULL;

	ylad->rpkts = buf[0] + buf[1] *256;
	debugLogA("Got packets: %d", ylad->rpkts);
	
    if (len == 4) {
        *outBufLen = 0;
        return buf;
    } else  if ( (buf[4] == 'Y') && (buf[5] == 'M') && (buf[6] == 'S') && (buf[7] == 'G')) {
		MoveMemory( buf, buf + 4, len - 4);
		*outBufLen = len-4;// we take off 4 bytes from the beginning
		 
		return buf;                 
    } else
        return NULL; /* Break connection, something went wrong! */
     
}

#endif
