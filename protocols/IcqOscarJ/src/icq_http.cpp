// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006 Joe Kucera
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
//  DESCRIPTION:
//
//  HTTP Gateway Handling routines
// -----------------------------------------------------------------------------

#include "icqoscar.h"

int icq_httpGatewayInit(HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr)
{
	// initial response from ICQ http gateway
	size_t wLen, wVersion, wType;
	WORD wIpLen;
	DWORD dwSid1, dwSid2, dwSid3, dwSid4;
	BYTE *buf;
	char szSid[33], szHttpServer[256], szHttpGetUrl[300], szHttpPostUrl[300];
	NETLIBHTTPPROXYINFO nlhpi = {0};

	if (nlhr->dataLength < 31)
	{
		SetLastError(ERROR_INVALID_DATA);
		return 0;
	}

	buf = (PBYTE)nlhr->pData;
	unpackWord(&buf, &wLen);
	unpackWord(&buf, &wVersion);    /* always 0x0443 */
	unpackWord(&buf, &wType);       /* hello reply */
	buf += 6;  /* dunno */
	unpackDWord(&buf, &dwSid1);
	unpackDWord(&buf, &dwSid2);
	unpackDWord(&buf, &dwSid3);
	unpackDWord(&buf, &dwSid4);
	mir_snprintf(szSid, 33, "%08x%08x%08x%08x", dwSid1, dwSid2, dwSid3, dwSid4);
	unpackWord(&buf, &wIpLen);

	if(nlhr->dataLength < 30 + wIpLen || wIpLen == 0 || wIpLen > sizeof(szHttpServer) - 1)
	{
		SetLastError(ERROR_INVALID_DATA);
		return 0;
	}

	SetGatewayIndex(hConn, 1); // new master connection begins here

	memcpy(szHttpServer, buf, wIpLen);
	szHttpServer[wIpLen] = '\0';

	nlhpi.cbSize = sizeof(nlhpi);
	nlhpi.flags = NLHPIF_USEPOSTSEQUENCE;
	nlhpi.szHttpGetUrl = szHttpGetUrl;
	nlhpi.szHttpPostUrl = szHttpPostUrl;
	nlhpi.firstPostSequence = 1;
	mir_snprintf(szHttpGetUrl, 300, "http://%s/monitor?sid=%s", szHttpServer, szSid);
	mir_snprintf(szHttpPostUrl, 300, "http://%s/data?sid=%s&seq=", szHttpServer, szSid);

	return CallService(MS_NETLIB_SETHTTPPROXYINFO, (WPARAM)hConn, (LPARAM)&nlhpi);
}



int icq_httpGatewayBegin(HANDLE hConn, NETLIBOPENCONNECTION* nloc)
{ // open our "virual data connection"
	icq_packet packet;
	size_t serverNameLen;

	serverNameLen = mir_strlen(nloc->szHost);

	packet.wLen = (WORD)(serverNameLen + 4);
	write_httphdr(&packet, HTTP_PACKETTYPE_LOGIN, GetGatewayIndex(hConn));
	packWord(&packet, (WORD)serverNameLen);
	packBuffer(&packet, (LPBYTE)nloc->szHost, serverNameLen);
	packWord(&packet, nloc->wPort);
	INT_PTR res = Netlib_Send(hConn, (char*)packet.pData, packet.wLen, MSG_DUMPPROXY|MSG_NOHTTPGATEWAYWRAP);
	SAFE_FREE((void**)&packet.pData);

	return res != SOCKET_ERROR;
}



int icq_httpGatewayWrapSend(HANDLE hConn, PBYTE buf, int len, int flags, MIRANDASERVICE pfnNetlibSend)
{
	PBYTE sendBuf = buf;
	int sendLen = len;
	int sendResult = 0;

	while (sendLen > 0)
	{ // imitate polite behaviour of icq5.1 and split large packets
		icq_packet packet;
		WORD curLen;
		int curResult;

		if (sendLen > 512) curLen = 512; else curLen = (WORD)sendLen;
		// send wrapped data
		packet.wLen = curLen;
		write_httphdr(&packet, HTTP_PACKETTYPE_FLAP, GetGatewayIndex(hConn));
		packBuffer(&packet, sendBuf, curLen);

		NETLIBBUFFER nlb={ (char*)packet.pData, packet.wLen, flags };
		curResult = pfnNetlibSend((WPARAM)hConn, (LPARAM)&nlb);
		
		SAFE_FREE((void**)&packet.pData);

		// sending failed, end loop
		if (curResult <= 0)
			return curResult;
		// calculare real number of data bytes sent
		if (curResult > 14) sendResult += curResult - 14;
		// move on
		sendLen -= curLen;
		sendBuf += curLen;
	}

	return sendResult;
}



PBYTE icq_httpGatewayUnwrapRecv(NETLIBHTTPREQUEST* nlhr, PBYTE buf, int len, int* outBufLen, void *(*NetlibRealloc)(void *, size_t))
{
	size_t wLen, wType;
	DWORD dwPackSeq;
	PBYTE tbuf;
	size_t i = 0;

	tbuf = buf;
	for(;;)
	{
		if (tbuf - buf + 2 > len)
			break;
		unpackWord(&tbuf, &wLen);
		if (wLen < 12)
			break;
		if (tbuf - buf + wLen > len)
			break;
		tbuf += 2;    /* version */
		unpackWord(&tbuf, &wType);
		tbuf += 4;    /* flags */
		unpackDWord(&tbuf, &dwPackSeq);
		if (wType == HTTP_PACKETTYPE_FLAP)
		{ // it is normal data packet
			size_t copyBytes = wLen - 12;
			if (copyBytes > len - i)
			{
				/* invalid data - do our best to get something out of it */
				copyBytes = len - i;
			}
			memcpy(buf + i, tbuf, copyBytes);
			i += copyBytes;
		}
		else if (wType == HTTP_PACKETTYPE_LOGINREPLY)
		{ // our "virtual connection" was established, good
			BYTE bRes;

			unpackByte(&tbuf, &bRes);
			wLen -= 1;
			if (!bRes)
				Netlib_Logf( NULL, "Gateway Connection #%d Established.", dwPackSeq);
			else
				Netlib_Logf( NULL, "Gateway Connection #%d Failed, error: %d", dwPackSeq, bRes);
		}
		else if (wType == HTTP_PACKETTYPE_CLOSEREPLY)
		{ // "virtual connection" closed - only received if any other "virual connection" still active
			Netlib_Logf( NULL, "Gateway Connection #%d Closed.", dwPackSeq);
		}
		tbuf += wLen - 12;
	}
	*outBufLen = (int)i;

	return buf;
}



int icq_httpGatewayWalkTo(HANDLE hConn, NETLIBOPENCONNECTION* nloc)
{ // this is bad simplification - for avatars to work we need to handle
	// two "virtual connections" at the same time
	icq_packet packet;
	DWORD dwGatewaySeq = GetGatewayIndex(hConn);

	packet.wLen = 0;
	write_httphdr(&packet, HTTP_PACKETTYPE_CLOSE, dwGatewaySeq);
	Netlib_Send(hConn, (char*)packet.pData, packet.wLen, MSG_DUMPPROXY|MSG_NOHTTPGATEWAYWRAP);
	// we closed virtual connection, open new one
	dwGatewaySeq++;
	SetGatewayIndex(hConn, dwGatewaySeq);
	return icq_httpGatewayBegin(hConn, nloc);
}
