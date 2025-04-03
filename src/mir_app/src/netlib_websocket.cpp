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

#include "../../libs/zlib/src/zlib.h"

struct WSHeader
{
	WSHeader()
	{
		memset(this, 0, sizeof(*this));
	}

	bool bIsFinal, bIsMasked;
	int opCode, firstByte;
	size_t payloadSize, headerSize;

	bool init(const void *pData, size_t bufSize)
	{
		if (bufSize < 2)
			return false;

		auto *buf = (const uint8_t *)pData;
		bIsFinal = (buf[0] & 0x80) != 0;
		bIsMasked = (buf[1] & 0x80) != 0;
		opCode = buf[0] & 0x0F;
		firstByte = buf[1] & 0x7F;
		headerSize = 2 + (firstByte == 0x7E ? 2 : 0) + (firstByte == 0x7F ? 8 : 0) + (bIsMasked ? 4 : 0);
		if (bufSize < headerSize)
			return false;

		uint64_t tmpSize = 0;
		switch (firstByte) {
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
			tmpSize = firstByte;
		}
		payloadSize = tmpSize;
		return true;
	}
};

MWebSocket::MWebSocket()
{
}

MWebSocket::~MWebSocket()
{
	if (m_hConn)
		Netlib_CloseHandle(m_hConn);
}

////////////////////////////////////////////////////////////////////////////////////////

MHttpResponse* MWebSocket::connect(HANDLE nlu, const char *szHost, const MHttpHeaders *pHeaders)
{
	m_nlu = (HNETLIBUSER)nlu;
	m_bTerminated = false;

	CMStringA tmpHost(szHost);

	// connect to the gateway server
	if (!mir_strncmp(tmpHost, "wss://", 6))
		tmpHost.Delete(0, 6);

	MHttpRequest nlhr(REQUEST_GET);
	nlhr.flags = NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_SSL;
	nlhr.m_szUrl = tmpHost.GetBuffer();
	nlhr.AddHeader("Accept", "*/*");
	nlhr.AddHeader("Upgrade", "websocket");
	nlhr.AddHeader("Pragma", "no-cache");
	nlhr.AddHeader("Cache-Control", "no-cache");
	nlhr.AddHeader("Connection", "keep-alive, Upgrade");

	uint8_t binNonce[16];
	Utils_GetRandom(binNonce, sizeof(binNonce));
	nlhr.AddHeader("Sec-WebSocket-Key", ptrA(mir_base64_encode(binNonce, sizeof(binNonce))));
	nlhr.AddHeader("Sec-WebSocket-Version", "13");
	nlhr.AddHeader("Sec-WebSocket-Extensions", "permessage-deflate; client_max_window_bits");
	
	if (pHeaders)
		for (auto &it: *pHeaders)
			nlhr.AddHeader(it->szName, it->szValue);

	auto *pReply = Netlib_HttpTransaction(m_nlu, &nlhr);
	if (pReply == nullptr) {
		Netlib_Logf(m_nlu, "Error establishing WebSocket connection to %s, send failed", tmpHost.c_str());
		return nullptr;
	}

	m_hConn = pReply->nlc;
	if (pReply->resultCode != 101)
		Netlib_Logf(m_nlu, "Error establishing WebSocket connection to %s, status %d", tmpHost.c_str(), pReply->resultCode);

	return pReply;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void WebSocket_Send(HNETLIBCONN nlc, const void *pData, int64_t dataLen, uint8_t opCode)
{
	uint8_t header[20];
	size_t cbLen;

	header[0] = 0x80 + (opCode & 0x7F);
	if (dataLen < 126) {
		header[1] = (dataLen & 0xFF);
		cbLen = 2;
	}
	else if (dataLen < 65536) {
		header[1] = 0x7E;
		header[2] = (dataLen >> 8) & 0xFF;
		header[3] = dataLen & 0xFF;
		cbLen = 4;
	}
	else {
		header[1] = 0x7F;
		header[2] = (dataLen >> 56) & 0xff;
		header[3] = (dataLen >> 48) & 0xff;
		header[4] = (dataLen >> 40) & 0xff;
		header[5] = (dataLen >> 32) & 0xff;
		header[6] = (dataLen >> 24) & 0xff;
		header[7] = (dataLen >> 16) & 0xff;
		header[8] = (dataLen >> 8) & 0xff;
		header[9] = dataLen & 0xff;
		cbLen = 10;
	}

	union {
		uint32_t dwMask;
		uint8_t arMask[4];
	};

	dwMask = crc32(rand(), (uint8_t*)pData, (unsigned)dataLen);
	memcpy(header + cbLen, arMask, _countof(arMask));
	cbLen += _countof(arMask);
	header[1] |= 0x80;

	ptrA sendBuf((char*)mir_alloc(dataLen + cbLen));
	memcpy(sendBuf, header, cbLen);
	if (dataLen) {
		memcpy(sendBuf.get() + cbLen, pData, dataLen);
		for (int i = 0; i < dataLen; i++)
			sendBuf[i + cbLen] ^= arMask[i & 3];
	}
	Netlib_Send(nlc, sendBuf, int(dataLen + cbLen), MSG_NODUMP);
}

void MWebSocket::sendText(const char *pData)
{
	if (m_hConn && pData) {
		Netlib_Dump(m_hConn, pData, strlen(pData), true, 0);

		mir_cslock lck(m_cs);
		WebSocket_Send(m_hConn, pData, strlen(pData), 1);
	}
}

void MWebSocket::sendBinary(const void *pData, size_t dataLen)
{
	if (m_hConn && pData) {
		mir_cslock lck(m_cs);
		WebSocket_Send(m_hConn, pData, dataLen, 2);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void MWebSocket::terminate()
{
	m_bTerminated = true;

	if (m_hConn) {
		Netlib_Shutdown(m_hConn);
		m_hConn = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void MWebSocket::run()
{
	int offset = 0;
	MBinBuffer netbuf;

	while (!m_bTerminated) {
		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hConn, (char *)buf + offset, _countof(buf) - offset, MSG_NODUMP);
		if (bufSize == 0) {
			Netlib_Log(m_nlu, "Websocket connection gracefully closed");
			break;
		}
		if (bufSize < 0) {
			Netlib_Log(m_nlu, "Websocket connection error, exiting");
			break;
		}

		WSHeader hdr;
		if (!hdr.init(buf, bufSize)) {
			offset += bufSize;
			continue;
		}
		offset = 0;

		// we have some additional data, not only opcode
		if ((size_t)bufSize > hdr.headerSize) {
			size_t currPacketSize = bufSize - hdr.headerSize;
			netbuf.append(buf, bufSize);
			while (currPacketSize < hdr.payloadSize) {
				int result = Netlib_Recv(m_hConn, (char *)buf, _countof(buf), MSG_NODUMP);
				if (result == 0) {
					Netlib_Log(m_nlu, "Websocket connection gracefully closed");
					break;
				}
				if (result < 0) {
					Netlib_Log(m_nlu, "Websocket connection error, exiting");
					break;
				}
				currPacketSize += result;
				netbuf.append(buf, result);
			}
		}

		// read all payloads from the current buffer, one by one
		size_t prevSize = 0;
		while (true) {
			switch (hdr.opCode) {
			case 0: // text packet
			case 1: // binary packet
			case 2: // continuation
				if (hdr.bIsFinal) {
					// process a packet here
					process((uint8_t*)netbuf.data() + hdr.headerSize, (int)hdr.payloadSize);
				}
				break;

			case 8: // close
				Netlib_Log(m_nlu, "server required to exit");
				m_bTerminated = true; // simply reconnect, don't exit
				break;

			case 9: // ping
				Netlib_Logf(m_nlu, "ping received: %d bytes", int(hdr.payloadSize));
				if (hdr.payloadSize)
					Netlib_Send(m_hConn, (char *)buf + hdr.headerSize, (int)hdr.payloadSize, MSG_NODUMP);
				break;
			}

			if (hdr.bIsFinal)
				netbuf.remove(hdr.headerSize + hdr.payloadSize);

			if (netbuf.length() == 0)
				break;

			// if we have not enough data for header, continue reading
			if (!hdr.init(netbuf.data(), netbuf.length()))
				break;

			// if we have not enough data for data, continue reading
			if (hdr.headerSize + hdr.payloadSize > netbuf.length())
				break;

			if (prevSize == netbuf.length()) {
				netbuf.remove(prevSize);
				break;
			}

			prevSize = netbuf.length();
		}
	}
}

void MJsonWebSocket::process(const uint8_t *buf, size_t cbLen)
{
	CMStringA szJson((char*)buf, (int)cbLen);
	Netlib_Log(m_nlu, szJson);

	JSONNode root = JSONNode::parse(szJson);
	if (root)
		process(root);
}
