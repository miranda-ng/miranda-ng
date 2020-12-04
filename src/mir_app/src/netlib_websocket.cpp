/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org),
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

#include "../../libs/zlib/src/zlib.h"

MIR_APP_DLL(NETLIBHTTPREQUEST*) WebSocket_Connect(HNETLIBUSER nlu, const char *szHost, NETLIBHTTPHEADER *pHeaders)
{
	CMStringA tmpHost(szHost);

	// connect to the gateway server
	if (!mir_strncmp(tmpHost, "wss://", 6))
		tmpHost.Delete(0, 6);

	auto *nlr = new MHttpRequest;
	nlr->flags = NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_SSL;
	nlr->szUrl = tmpHost.GetBuffer();
	nlr->AddHeader("Accept", "*/*");
	nlr->AddHeader("Upgrade", "websocket");
	nlr->AddHeader("Pragma", "no-cache");
	nlr->AddHeader("Cache-Control", "no-cache");
	nlr->AddHeader("Connection", "keep-alive, Upgrade");

	uint8_t binNonce[16];
	Utils_GetRandom(binNonce, sizeof(binNonce));
	nlr->AddHeader("Sec-WebSocket-Key", ptrA(mir_base64_encode(binNonce, sizeof(binNonce))));
	nlr->AddHeader("Sec-WebSocket-Version", "13");
	nlr->AddHeader("Sec-WebSocket-Extensions", "permessage-deflate; client_max_window_bits");
	
	if (pHeaders) {
		while (pHeaders->szName != nullptr) {
			nlr->AddHeader(pHeaders->szName, pHeaders->szValue);
			pHeaders++;
		}
	}

	auto *pReply = Netlib_HttpTransaction(nlu, nlr);
	delete nlr;

	if (pReply == nullptr) {
		Netlib_Logf(nlu, "Error establishing WebSocket connection to %s, send failed", tmpHost.c_str());
		return nullptr;
	}

	if (pReply->resultCode != 101) {
		Netlib_Logf(nlu, "Error establishing WebSocket connection to %s, status %d", tmpHost.c_str(), pReply->resultCode);
		Netlib_FreeHttpRequest(pReply);
		return nullptr;
	}

	return pReply;
}

MIR_APP_DLL(bool) WebSocket_InitHeader(WSHeader &hdr, const void *pData, size_t bufSize)
{
	if (bufSize < 2)
		return false;

	auto *buf = (const BYTE *)pData;
	hdr.bIsFinal = (buf[0] & 0x80) != 0;
	hdr.bIsMasked = (buf[1] & 0x80) != 0;
	hdr.opCode = buf[0] & 0x0F;
	hdr.firstByte = buf[1] & 0x7F;
	hdr.headerSize = 2 + (hdr.firstByte == 0x7E ? 2 : 0) + (hdr.firstByte == 0x7F ? 8 : 0) + (hdr.bIsMasked ? 4 : 0);
	if (bufSize < hdr.headerSize)
		return false;

	uint64_t tmpSize = 0;
	switch (hdr.firstByte) {
	case 0x7F:
		tmpSize += ((uint64_t)buf[2]) << 56;
		tmpSize += ((uint64_t)buf[3]) << 48;
		tmpSize += ((uint64_t)buf[4]) << 40;
		tmpSize += ((uint64_t)buf[5]) << 32;
		tmpSize += ((uint64_t)buf[6]) << 24;
		tmpSize += ((uint64_t)buf[7]) << 16;
		tmpSize += ((uint64_t)buf[8]) << 8;
		tmpSize += ((uint64_t)buf[9]);
		break;

	case 0x7E:
		tmpSize += ((uint64_t)buf[2]) << 8;
		tmpSize += ((uint64_t)buf[3]);
		break;

	default:
		tmpSize = hdr.firstByte;
	}
	hdr.payloadSize = tmpSize;
	return true;
}

MIR_APP_DLL(void) WebSocket_Send(HNETLIBCONN nlc, const void *pData, size_t strLen)
{
	BYTE header[20];
	int opCode = 1;
	size_t datalen;

	header[0] = 0x80 + (opCode & 0x7F);
	if (strLen < 126) {
		header[1] = (strLen & 0xFF);
		datalen = 2;
	}
	else if (strLen < 65536) {
		header[1] = 0x7E;
		header[2] = (strLen >> 8) & 0xFF;
		header[3] = strLen & 0xFF;
		datalen = 4;
	}
	else {
		header[1] = 0x7F;
		header[2] = (strLen >> 56) & 0xff;
		header[3] = (strLen >> 48) & 0xff;
		header[4] = (strLen >> 40) & 0xff;
		header[5] = (strLen >> 32) & 0xff;
		header[6] = (strLen >> 24) & 0xff;
		header[7] = (strLen >> 16) & 0xff;
		header[8] = (strLen >> 8) & 0xff;
		header[9] = strLen & 0xff;
		datalen = 10;
	}

	union {
		uint32_t dwMask;
		uint8_t arMask[4];
	};
	
	dwMask = crc32(rand(), (uint8_t*)pData, (unsigned)strLen);
	memcpy(header + datalen, arMask, _countof(arMask));
	datalen += _countof(arMask);
	header[1] |= 0x80;

	ptrA sendBuf((char*)mir_alloc(strLen + datalen));
	memcpy(sendBuf, header, datalen);
	if (strLen) {
		memcpy(sendBuf.get() + datalen, pData, strLen);
		for (size_t i = 0; i < strLen; i++)
			sendBuf[i + datalen] ^= arMask[i & 3];
	}
	Netlib_Send(nlc, sendBuf, int(strLen + datalen), MSG_NODUMP);
}
