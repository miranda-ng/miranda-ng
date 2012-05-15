/*
 * $Id: http_gateway.h 3541 2006-08-18 21:18:33Z gena01 $
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

#ifndef _YAHOO_HTTP_GATEWAY_H_
#define _YAHOO_HTTP_GATEWAY_H_

#define HTTP_PROXY_VERSION  0x0443

int YAHOO_httpGatewayInit(HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr);
int YAHOO_httpGatewayWrapSend(HANDLE hConn, PBYTE buf, int len, int flags, MIRANDASERVICE pfnNetlibSend);
PBYTE YAHOO_httpGatewayUnwrapRecv(NETLIBHTTPREQUEST *nlhr, PBYTE buf, int bufLen, int *outBufLen, void *(*NetlibRealloc)(void *, size_t));

#endif 

