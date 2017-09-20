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

struct WSHeader
{
	WSHeader()
	{
		memset(this, 0, sizeof(*this));
	}

	bool init(BYTE *buf, size_t bufSize)
	{
		if (bufSize < 2)
			return false;

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

	bool bIsFinal, bIsMasked;
	int opCode, firstByte;
	size_t payloadSize, headerSize;
};

//////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

void CDiscordProto::GatewaySend(const JSONNode &pRoot, int opCode)
{
	if (m_hGatewayConnection == nullptr)
		return;

	json_string szText = pRoot.write();

	BYTE header[20];
	size_t datalen;
	uint64_t strLen = szText.length();

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
		uLong dwMask;
		Bytef arMask[4];
	};
	dwMask = crc32(rand(), (Bytef*)szText.c_str(), (uInt)szText.length());
	memcpy(header + datalen, arMask, _countof(arMask));
	datalen += _countof(arMask);
	header[1] |= 0x80;

	ptrA sendBuf((char*)mir_alloc(strLen + datalen));
	memcpy(sendBuf, header, datalen);
	if (strLen) {
		memcpy(sendBuf.get() + datalen, szText.c_str(), strLen);
		for (size_t i = 0; i < strLen; i++)
			sendBuf[i + datalen] ^= arMask[i & 3];
	}
	Netlib_Send(m_hGatewayConnection, sendBuf, int(strLen + datalen), 0);
}

//////////////////////////////////////////////////////////////////////////////////////
// gateway worker thread

void CDiscordProto::GatewayThread(void*)
{
	GatewayThreadWorker();
	ShutdownSession();
}

void CDiscordProto::GatewayThreadWorker()
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

	m_hGatewayConnection = Netlib_OpenConnection(m_hGatewayNetlibUser, &conn);
	if (m_hGatewayConnection == nullptr) {
		debugLogA("Gateway connection failed to connect to %s:%d, exiting", m_szGateway.c_str(), conn.wPort);
		return;
	}
	{
		CMStringA szBuf;
		szBuf.AppendFormat("GET https://%s/?encoding=json&v=6 HTTP/1.1\r\n", m_szGateway.c_str());
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
			return;
		}
	}
	{
		char buf[1024];
		int bufSize = Netlib_Recv(m_hGatewayConnection, buf, _countof(buf), MSG_DUMPASTEXT);
		if (bufSize <= 0) {
			debugLogA("Error establishing gateway connection to %s:%d, read failed", m_szGateway.c_str(), conn.wPort);
			return;
		}

		int status = 0;
		if (sscanf(buf, "HTTP/1.1 %d", &status) != 1 || status != 101) {
			debugLogA("Error establishing gateway connection to %s:%d, status %d", m_szGateway.c_str(), conn.wPort, status);
			return;
		}
	}

	debugLogA("Gateway connection succeeded");

	bool bExit = false;
	int offset = 0;
	MBinBuffer netbuf;

	while (!bExit) {
		if (m_bTerminated)
			break;

		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hGatewayConnection, (char*)buf + offset, _countof(buf) - offset, MSG_NODUMP);
		if (bufSize == 0) {
			debugLogA("Gateway connection gracefully closed");
			break;
		}
		if (bufSize < 0) {
			debugLogA("Gateway connection error, exiting");
			break;
		}

		WSHeader hdr;
		if (!hdr.init(buf, bufSize)) {
			offset += bufSize;
			continue;
		}
		offset = 0;

		debugLogA("Got packet: buffer = %d, opcode = %d, headerSize = %d, final = %d, masked = %d", bufSize, hdr.opCode, hdr.headerSize, hdr.bIsFinal, hdr.bIsMasked);

		// we have some additional data, not only opcode
		if (bufSize > hdr.headerSize) {
			size_t currPacketSize = bufSize - hdr.headerSize;
			netbuf.append(buf, bufSize);
			while (currPacketSize < hdr.payloadSize) {
				int result = Netlib_Recv(m_hGatewayConnection, (char*)buf, _countof(buf), MSG_NODUMP);
				if (result == 0) {
					debugLogA("Gateway connection gracefully closed");
					break;
				}
				if (result < 0) {
					debugLogA("Gateway connection error, exiting");
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
					CMStringA szJson(netbuf.data() + hdr.headerSize, (int)hdr.payloadSize);
					debugLogA("JSON received:\n%s", szJson.c_str());
					JSONNode root = JSONNode::parse(szJson);
					if (root)
						GatewayProcess(root);
				}
				break;

			case 8: // close
				debugLogA("server required to exit");
				bExit = true;
				break;

			case 9: // ping
				debugLogA("ping received");
				Netlib_Send(m_hGatewayConnection, (char*)buf + hdr.headerSize, bufSize - int(hdr.headerSize), 0);
				break;
			}

			if (hdr.bIsFinal)
				netbuf.remove(hdr.headerSize + hdr.payloadSize);

			if (netbuf.length() == 0)
				break;

			// if we have not enough data for header, continue reading
			if (!hdr.init((BYTE*)netbuf.data(), netbuf.length()))
				break;

			// if we have not enough data for data, continue reading
			if (hdr.headerSize + hdr.payloadSize > netbuf.length())
				break;

			debugLogA("Got inner packet: buffer = %d, opcode = %d, headerSize = %d, payloadSize = %d, final = %d, masked = %d", netbuf.length(), hdr.opCode, hdr.headerSize, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);
			if (prevSize == netbuf.length()) {
				netbuf.remove(prevSize);
				debugLogA("dropping current packet, exiting");
				break;
			}

			prevSize = netbuf.length();
		}
	}

	Netlib_CloseHandle(m_hGatewayConnection);
	m_hGatewayConnection = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////
// handles server commands

void CDiscordProto::GatewayProcess(const JSONNode &pRoot)
{
	int opCode = pRoot["op"].as_int();
	switch (opCode) {
	case 0:  // process incoming command
		{
			int iSeq = pRoot["s"].as_int();
			if (iSeq != 0)
				m_iGatewaySeq = iSeq;

			CMStringW wszCommand = pRoot["t"].as_mstring();
			debugLogA("got a server command to dispatch: %S", wszCommand.c_str());
			
			GatewayHandlerFunc pFunc = GetHandler(wszCommand);
			if (pFunc)
				(this->*pFunc)(pRoot["d"]);
		}
		break;

	case 9:  // session invalidated
		if (pRoot["d"].as_bool()) // session can be resumed
			GatewaySendResume();
		else {
			Sleep(5000); // 5 seconds - recommended timeout
			GatewaySendIdentify();
		}
		break;

	case 10: // hello
		m_iHartbeatInterval = pRoot["d"]["heartbeat_interval"].as_int();

		GatewaySendIdentify();
		break;
	
	case 11: // heartbeat ack
		break;

	default:
		debugLogA("ACHTUNG! Unknown opcode: %d, report it to developer", opCode);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// requests to be sent to a gateway

void CDiscordProto::GatewaySendHeartbeat()
{
	// we don't send heartbeat packets until we get logged in
	if (!m_iHartbeatInterval || !m_iGatewaySeq)
		return;

	JSONNode root;
	root << INT_PARAM("op", 1) << INT_PARAM("d", m_iGatewaySeq);
	GatewaySend(root);
}

void CDiscordProto::GatewaySendIdentify()
{
	wchar_t wszOs[256];
	GetOSDisplayString(wszOs, _countof(wszOs));
	
	char szVersion[256];
	Miranda_GetVersionText(szVersion, _countof(szVersion));

	JSONNode props; props.set_name("properties");
	props << WCHAR_PARAM("os", wszOs) << CHAR_PARAM("browser", "Chrome") << CHAR_PARAM("device", szVersion)
		<< CHAR_PARAM("referrer", "https://miranda-ng.org") << CHAR_PARAM("referring_domain", "miranda-ng.org");

	JSONNode payload; payload.set_name("d");
	payload << CHAR_PARAM("token", m_szAccessToken) << props << BOOL_PARAM("compress", false) << INT_PARAM("large_threshold", 250);

	JSONNode root;
	root << INT_PARAM("op", 2) << payload;
	GatewaySend(root);
}

void CDiscordProto::GatewaySendGuildInfo(SnowFlake id)
{
	JSONNode payload(JSON_ARRAY); payload.set_name("d");
	payload << INT64_PARAM("", id);

	JSONNode root;
	root << INT_PARAM("op", 12) << payload;
	GatewaySend(root);
}

void CDiscordProto::GatewaySendResume()
{
	char szRandom[40];
	uint8_t random[16];
	Utils_GetRandom(random, _countof(random));
	bin2hex(random, _countof(random), szRandom);

	JSONNode root;
	root << CHAR_PARAM("token", szRandom) << CHAR_PARAM("session_id", m_szGatewaySessionId) << INT_PARAM("seq", m_iGatewaySeq);
	GatewaySend(root);
}
