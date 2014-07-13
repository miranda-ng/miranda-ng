// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#ifndef __ICQ_HTTP_H
#define __ICQ_HTTP_H

#define HTTP_PROXY_VERSION  0x0443

#define HTTP_PACKETTYPE_HELLOREPLY   2
#define HTTP_PACKETTYPE_LOGIN        3
#define HTTP_PACKETTYPE_LOGINREPLY   4    /* contains 1 byte: 0 */
#define HTTP_PACKETTYPE_FLAP         5
#define HTTP_PACKETTYPE_CLOSE        6    /* contains no data */
#define HTTP_PACKETTYPE_CLOSEREPLY   7    /* contains 1 byte: 0 */

int icq_httpGatewayInit(HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr);
int icq_httpGatewayBegin(HANDLE hConn, NETLIBOPENCONNECTION *nloc);
int icq_httpGatewayWrapSend(HANDLE hConn, PBYTE buf, int len, int flags, MIRANDASERVICE pfnNetlibSend);
PBYTE icq_httpGatewayUnwrapRecv(NETLIBHTTPREQUEST *nlhr, PBYTE buf, int bufLen, int *outBufLen, void *(*NetlibRealloc)(void *, size_t));
int icq_httpGatewayWalkTo(HANDLE hConn, NETLIBOPENCONNECTION* nloc);

#endif /* __ICQ_HTTP_H */
