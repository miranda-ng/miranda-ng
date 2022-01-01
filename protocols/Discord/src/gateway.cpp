/*
Copyright © 2016-22 Miranda NG team

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

//////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

bool CDiscordProto::GatewaySend(const JSONNode &pRoot)
{
	if (m_hGatewayConnection == nullptr)
		return false;

	json_string szText = pRoot.write();
	debugLogA("Gateway send: %s", szText.c_str());
	WebSocket_SendText(m_hGatewayConnection, szText.c_str());
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// gateway worker thread

void CDiscordProto::GatewayThread(void*)
{
	while (GatewayThreadWorker())
		;
	ShutdownSession();
}

bool CDiscordProto::GatewayThreadWorker()
{
	NETLIBHTTPHEADER hdrs[] =
	{
		{ "Origin", "https://discord.com" },
		{ 0, 0 },
		{ 0, 0 },
	};

	if (!m_szWSCookie.IsEmpty()) {
		hdrs[1].szName = "Cookie";
		hdrs[1].szValue = m_szWSCookie.GetBuffer();
	}

	NLHR_PTR pReply(WebSocket_Connect(m_hGatewayNetlibUser, m_szGateway + "/?encoding=json&v=8", hdrs));
	if (pReply == nullptr) {
		debugLogA("Gateway connection failed, exiting");
		return false;
	}

	if (auto *pszNewCookie = Netlib_GetHeader(pReply, "Set-Cookie")) {
		char *p = strchr(pszNewCookie, ';');
		if (p) *p = 0;

		m_szWSCookie = pszNewCookie;
	}

	if (pReply->resultCode != 101) {
		// if there's no cookie & Miranda is bounced with error 404, simply apply the cookie and try again
		if (pReply->resultCode == 404) {
			if (hdrs[1].szName == nullptr)
				return true;

			m_szWSCookie.Empty(); // don't use the same cookie twice
		}
		return false;
	}

	// succeeded!
	debugLogA("Gateway connection succeeded");
	m_hGatewayConnection = pReply->nlc;

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
			bExit = !m_bTerminated;
			break;
		}
		if (bufSize < 0) {
			debugLogA("Gateway connection error, exiting");
			break;
		}

		WSHeader hdr;
		if (!WebSocket_InitHeader(hdr, buf, bufSize)) {
			offset += bufSize;
			continue;
		}
		offset = 0;

		debugLogA("Got packet: buffer = %d, opcode = %d, headerSize = %d, final = %d, masked = %d", bufSize, hdr.opCode, hdr.headerSize, hdr.bIsFinal, hdr.bIsMasked);

		// we have some additional data, not only opcode
		if ((size_t)bufSize > hdr.headerSize) {
			size_t currPacketSize = bufSize - hdr.headerSize;
			netbuf.append(buf, bufSize);
			while (currPacketSize < hdr.payloadSize) {
				int result = Netlib_Recv(m_hGatewayConnection, (char*)buf, _countof(buf), MSG_NODUMP);
				if (result == 0) {
					debugLogA("Gateway connection gracefully closed");
					bExit = !m_bTerminated;
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
						bExit = GatewayProcess(root);
				}
				break;

			case 8: // close
				debugLogA("server required to exit");
				bExit = true; // simply reconnect, don't exit
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
			if (!WebSocket_InitHeader(hdr, netbuf.data(), netbuf.length()))
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
	return bExit;
}

//////////////////////////////////////////////////////////////////////////////////////
// handles server commands

bool CDiscordProto::GatewayProcess(const JSONNode &pRoot)
{
	int opCode = pRoot["op"].as_int();
	switch (opCode) {
	case OPCODE_DISPATCH:  // process incoming command
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

	case OPCODE_RECONNECT:  // we need to reconnect asap
		debugLogA("we need to reconnect, leaving worker thread");
		return true;

	case OPCODE_INVALID_SESSION:  // session invalidated
		if (pRoot["d"].as_bool()) // session can be resumed
			GatewaySendResume();
		else {
			Sleep(5000); // 5 seconds - recommended timeout
			GatewaySendIdentify();
		}
		break;

	case OPCODE_HELLO: // hello
		m_iHartbeatInterval = pRoot["d"]["heartbeat_interval"].as_int();

		GatewaySendIdentify();
		break;
	
	case OPCODE_HEARTBEAT_ACK: // heartbeat ack
		break;

	default:
		debugLogA("ACHTUNG! Unknown opcode: %d, report it to developer", opCode);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
// requests to be sent to a gateway

void CDiscordProto::GatewaySendGuildInfo(CDiscordGuild *pGuild)
{
	if (!pGuild->arChannels.getCount())
		return;

	JSONNode a1(JSON_ARRAY); a1 << INT_PARAM("", 0) << INT_PARAM("", 99);

	CMStringA szId(FORMAT, "%lld", pGuild->arChannels[0]->id);
	JSONNode chl(JSON_ARRAY); chl.set_name(szId.c_str()); chl << a1;

	JSONNode channels; channels.set_name("channels"); channels << chl;

	JSONNode payload; payload.set_name("d");
	payload << SINT64_PARAM("guild_id", pGuild->id) << BOOL_PARAM("typing", true) << BOOL_PARAM("activities", true) << BOOL_PARAM("presences", true) << channels;
		
	JSONNode root;
	root << INT_PARAM("op", OPCODE_REQUEST_SYNC_CHANNEL) << payload;
	GatewaySend(root);
}

void CDiscordProto::GatewaySendHeartbeat()
{
	// we don't send heartbeat packets until we get logged in
	if (!m_iHartbeatInterval || !m_iGatewaySeq)
		return;

	JSONNode root;
	root << INT_PARAM("op", OPCODE_HEARTBEAT) << INT_PARAM("d", m_iGatewaySeq);
	GatewaySend(root);
}

void CDiscordProto::GatewaySendIdentify()
{
	if (m_szAccessToken == nullptr) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	char szOs[256];
	OS_GetDisplayString(szOs, _countof(szOs));
	
	char szVersion[256];
	Miranda_GetVersionText(szVersion, _countof(szVersion));

	JSONNode props; props.set_name("properties");
	props << CHAR_PARAM("os", szOs) << CHAR_PARAM("browser", "Chrome") << CHAR_PARAM("device", szVersion)
		<< CHAR_PARAM("referrer", "https://miranda-ng.org") << CHAR_PARAM("referring_domain", "miranda-ng.org");

	JSONNode payload; payload.set_name("d");
	payload << CHAR_PARAM("token", m_szAccessToken) << props << BOOL_PARAM("compress", false) << INT_PARAM("large_threshold", 250);

	JSONNode root;
	root << INT_PARAM("op", OPCODE_IDENTIFY) << payload;
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

bool CDiscordProto::GatewaySendStatus(int iStatus, const wchar_t *pwszStatusText)
{
	if (iStatus == ID_STATUS_OFFLINE) {
		Push(new AsyncHttpRequest(this, REQUEST_POST, "/auth/logout", nullptr));
		return true;
	}

	const char *pszStatus;
	switch (iStatus) {
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
		pszStatus = "idle"; break;
	case ID_STATUS_DND:
		pszStatus = "dnd"; break;
	case ID_STATUS_INVISIBLE:
		pszStatus = "invisible"; break;
	default:
		pszStatus = "online"; break;
	}

	JSONNode payload; payload.set_name("d");
	payload << INT64_PARAM("since", __int64(time(0)) * 1000) << BOOL_PARAM("afk", true) << CHAR_PARAM("status", pszStatus);
	if (pwszStatusText == nullptr)
		payload << CHAR_PARAM("game", nullptr);
	else {
		JSONNode game; game.set_name("game"); game << WCHAR_PARAM("name", pwszStatusText) << INT_PARAM("type", 0);
		payload << game;
	}
	
	JSONNode root; root << INT_PARAM("op", OPCODE_STATUS_UPDATE) << payload;
	return GatewaySend(root);
}
