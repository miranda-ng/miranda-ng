/*
Copyright © 2016-17 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CDiscordProto::OnReceiveGateway(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200) {
		ShutdownSession();
		return;
	}

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root) {
		ShutdownSession();
		return;
	}

	m_szGateway = root["url"].as_mstring();
	ForkThread(&CDiscordProto::GatewayThread, NULL);
}

void CDiscordProto::GatewaySend(int opCode, const char *szBuf)
{
	if (m_hGatewayConnection == NULL)
		return;

	BYTE header[20];
	size_t datalen, strLen = mir_strlen(szBuf);
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

	ptrA sendBuf((char*)mir_alloc(strLen + datalen));
	memcpy(sendBuf, header, datalen);
	if (strLen)
		memcpy(sendBuf.get() + datalen, szBuf, strLen);
	Netlib_Send(m_hGatewayConnection, sendBuf, int(strLen + datalen), 0);
}

void CDiscordProto::GatewayThread(void*)
{
	// connect to the gateway server
	if (!mir_strncmp(m_szGateway, "wss://", 6))
		m_szGateway.Delete(0, 6);

	NETLIBOPENCONNECTION conn = { 0 };
	conn.cbSize = sizeof(conn);
	conn.szHost = m_szGateway;
	conn.flags = NLOCF_V2 | NLOCF_SSL;
	conn.timeout = 5;

	int pos = m_szGateway.Find(':');
	if (pos != -1) {
		conn.wPort = atoi(m_szGateway.GetBuffer() + pos + 1);
		m_szGateway.Truncate(pos);
	}
	else conn.wPort = 443;

	m_hGatewayConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hGatewayNetlibUser, (LPARAM)&conn);
	if (m_hGatewayConnection == NULL) {
		debugLogA("Gateway connection failed to connect to %s:%d, exiting", m_szGateway.c_str(), conn.wPort);
	LBL_Fatal:
		ShutdownSession();
		return;
	}
	{
		CMStringA szBuf;
		szBuf.AppendFormat("GET https://%s/?encoding=etf&v=6 HTTP/1.1\r\n", m_szGateway.c_str());
		szBuf.AppendFormat("Host: %s\r\n", m_szGateway.c_str());
		szBuf.AppendFormat("Upgrade: websocket\r\n");
		szBuf.AppendFormat("Pragma: no-cache\r\n");
		szBuf.AppendFormat("Cache-Control: no-cache\r\n");
		szBuf.AppendFormat("Connection: Upgrade\r\n");
		szBuf.AppendFormat("Sec-WebSocket-Key: KFShSwLlp4E6C7JZc5h4sg==\r\n");
		szBuf.AppendFormat("Sec-WebSocket-Version: 13\r\n");
		szBuf.AppendFormat("Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n");
		szBuf.AppendFormat("\r\n");
		if (Netlib_Send(m_hGatewayConnection, szBuf, szBuf.GetLength(), MSG_DUMPASTEXT) == SOCKET_ERROR) {
			debugLogA("Error establishing gateway connection to %s:%d, send failed", m_szGateway.c_str(), conn.wPort);
			goto LBL_Fatal;
		}
	}
	{
		char buf[1024];
		int bufSize = Netlib_Recv(m_hGatewayConnection, buf, _countof(buf), MSG_DUMPASTEXT);
		if (bufSize <= 0) {
			debugLogA("Error establishing gateway connection to %s:%d, read failed", m_szGateway.c_str(), conn.wPort);
			goto LBL_Fatal;
		}

		int status = 0;
		if (sscanf(buf, "HTTP/1.1 %d", &status) != 1 || status != 101) {
			debugLogA("Error establishing gateway connection to %s:%d, status %d", m_szGateway.c_str(), conn.wPort, status);
			goto LBL_Fatal;
		}
	}

	debugLogA("Gateway connection succeeded");

	bool bExit = false;
	int offset = 0;
	unsigned char *dataBuf = NULL;
	size_t dataBufSize = 0;

	while (!bExit) {
		if (m_bTerminated)
			break;

		NETLIBSELECT sel = {};
		sel.cbSize = sizeof(sel);
		sel.dwTimeout = 1000;
		sel.hReadConns[0] = m_hGatewayConnection;
		if (CallService(MS_NETLIB_SELECT, 0, (LPARAM)&sel) == 0) // timeout, send a hartbeat packet
			GatewaySend(0, "{ \"op\":1, \"d\":(null) }");

		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hGatewayConnection, (char*)buf+offset, _countof(buf) - offset, 0);
		if (bufSize == 0) {
			debugLogA("Gateway connection gracefully closed");
			break;
		}
		if (bufSize < 0) {
			debugLogA("Gateway connection error, exiting");
			break;
		}
		if (bufSize < 2) {
			offset = bufSize;
			continue;
		}
		offset = 0;

		bool bIsFinal = (buf[0] & 0x80) != 0;
		bool bIsMasked = (buf[1] & 0x80) != 0;
		int opCode = buf[0] & 0x0F, firstByte = buf[1] & 0x7F;
		int headerSize = 2 + (firstByte == 0x7E ? 2 : 0) + (firstByte == 0x7F ? 8 : 0) + (bIsMasked ? 4 : 0);
		if (bufSize < headerSize) {
			offset = bufSize;
			continue;
		}

		int dataShift;
		uint64_t payloadSize = 0;
		switch (firstByte) {
		case 0x7F:
			payloadSize += ((uint64_t)buf[2]) << 56;
			payloadSize += ((uint64_t)buf[3]) << 48;
			payloadSize += ((uint64_t)buf[4]) << 40;
			payloadSize += ((uint64_t)buf[5]) << 32;
			payloadSize += ((uint64_t)buf[6]) << 24;
			payloadSize += ((uint64_t)buf[7]) << 16;
			payloadSize += ((uint64_t)buf[8]) << 8;
			payloadSize += ((uint64_t)buf[9]);
			dataShift = 10;
			break;

		case 0x7E:
			payloadSize += ((uint64_t)buf[2]) << 8;
			payloadSize += ((uint64_t)buf[3]);
			dataShift = 4;
			break;

		default:
			payloadSize = firstByte;
			dataShift = 2;
		}

		debugLogA("Got packet: buffer = %d, opcode = %d, headerSize = %d, final = %d, masked = %d", bufSize, opCode, headerSize, bIsFinal, bIsMasked);

		if (bufSize > headerSize) {
			size_t newSize = dataBufSize + bufSize - headerSize;
			dataBuf = (unsigned char*)mir_realloc(dataBuf, newSize);
			memcpy(dataBuf + dataBufSize, buf + headerSize, bufSize - headerSize);
			dataBufSize = newSize;
			debugLogA("data buffer reallocated to %d bytes", dataBufSize);
		}

		if (dataBufSize < payloadSize)
			continue;

		switch (opCode){
		case 0: // text packet
		case 1: // binary packet
		case 2: // continuation
			if (bIsFinal) {
				// process a packet here
				z_stream stream = {};
				stream.next_in = dataBuf + headerSize;
				stream.avail_in = bufSize - headerSize;
				deflate(&stream, true);

				mir_free(dataBuf); dataBuf = NULL;
				dataBufSize = 0;
			}
			break;

		case 8: // close
			debugLogA("server required to exit", dataBufSize);
			bExit = true;
			break;

		case 9: // ping
			debugLogA("ping received", dataBufSize);
			Netlib_Send(m_hGatewayConnection, (char*)buf + headerSize, bufSize - headerSize, 0);
			break;
		}
	}

	Netlib_CloseHandle(m_hGatewayConnection);
	m_hGatewayConnection = NULL;
	ShutdownSession();
}
