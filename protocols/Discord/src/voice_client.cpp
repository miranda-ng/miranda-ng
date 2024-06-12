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

CDiscordVoiceCall::CDiscordVoiceCall(CDiscordProto *pOwner) :
	ppro(pOwner),
	m_arModes(1),
	m_timer(Miranda_GetSystemWindow(), UINT_PTR(this))
{
	int iError = 0;
	m_encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &iError);

	m_repacketizer = opus_repacketizer_create();

	m_timer.OnEvent = Callback(this, &CDiscordVoiceCall::onTimer);
}

CDiscordVoiceCall::~CDiscordVoiceCall()
{
	m_timer.StopSafe();

	if (m_encoder) {
		opus_encoder_destroy(m_encoder);
		m_encoder = nullptr;
	}

	if (m_repacketizer) {
		opus_repacketizer_destroy(m_repacketizer);
		m_repacketizer = nullptr;
	}

	if (m_hConn) {
		Netlib_CloseHandle(m_hConn);
		m_hConn = nullptr;
	}

	if (m_hBind) {
		Netlib_CloseHandle(m_hBind);
		m_hBind = nullptr;
	}
}

bool CDiscordVoiceCall::connect(HNETLIBUSER hServer)
{
	int nLoops = 0;
	time_t lastLoopTime = time(0);

	while (true) {
		time_t currTime = time(0);
		if (currTime - lastLoopTime > 3)
			nLoops = 0;

		nLoops++;
		if (nLoops > 5)
			break;

		lastLoopTime = currTime;

		MHttpHeaders hdrs;
		hdrs.AddHeader("Origin", "https://discord.com");

		NLHR_PTR pReply(WebSocket_Connect(hServer, szEndpoint + "/?encoding=json&v=8", &hdrs));
		if (pReply && pReply->resultCode == 101) {
			m_hConn = pReply->nlc;
			return true;
		}

		SleepEx(5000, TRUE);
	}

	return false;
}

void CDiscordVoiceCall::write(int op, JSONNode &d)
{
	d.set_name("d");

	JSONNode payload;
	payload << INT_PARAM("op", op) << d;

	auto json = payload.write();
	ppro->debugLogA("Voice JSON sent: %s", json.c_str());

	mir_cslock lck(m_cs);
	WebSocket_SendText(m_hConn, json.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////

void CDiscordVoiceCall::process(const JSONNode &node)
{
	int op = node["op"].as_int();
	auto &d = node["d"];

	switch (op) {
	case 2:
		processStreams(d);
		break;
	case 8:
		processHello(d);
		break;
	}
}

void CDiscordVoiceCall::processHello(const JSONNode &d)
{
	// HELLO
	if (int iTineout = d["heartbeat_interval"].as_int())
		m_timer.StartSafe(iTineout);

	JSONNode json;
	json << INT64_PARAM("server_id", guildId)
		<< CHAR_PARAM("session_id", szSessionId) << CHAR_PARAM("token", szToken);

	if (m_arModes.getCount()) {
		// resume the old session
		write(7, json);
	}
	else {
		// let's create new session
		json << INT64_PARAM("user_id", ppro->getId(DB_KEY_ID));
		write(0, json);
	}

	startTime = time(0);
}

void CDiscordVoiceCall::processStreams(const JSONNode &d)
{
	m_szIp = d["ip"].as_mstring();
	m_iPort = d["port"].as_int();
	m_iSsrc = d["ssrc"].as_int();

	for (auto &it : d["modes"])
		m_arModes.insert(newStr(it.as_string().c_str()));

	ppro->debugLogA("Voice session established, UDP address = %s:%d", m_szIp.c_str(), m_iPort);

	CMStringA szExternalIp;
	if (auto *p = Netlib_GetMyIp(true)) {
		szExternalIp = p->szIp[0];
		mir_free(p);
	}

	NETLIBBIND nlb = {};
	nlb.pfnNewConnection = &GetConnection;
	nlb.pExtra = this;
	m_hBind = Netlib_BindPort(ppro->m_hGatewayNetlibUser, &nlb);
	if (m_hBind == nullptr) {
		ppro->debugLogA("UDP port binding failed, exiting");
		m_bTerminated = true;
		return;
	}

	JSONNode data; data.set_name("data");
	data << CHAR_PARAM("address", szExternalIp) << INT_PARAM("port", nlb.wExPort) << CHAR_PARAM("mode", "xsalsa20_poly1305");
	
	JSONNode json;
	json << CHAR_PARAM("protocol", "UDP") << data;

	write(1, json);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void CDiscordProto::VoiceClientThread(void *param)
{
	Thread_SetName("VoiceClientThread");

	auto *pCall = (CDiscordVoiceCall *)param;
	debugLogA("Entering voice websocket thread");

	if (!pCall->connect(m_hGatewayNetlibUser)) {
		debugLogA("Voice gateway connection failed, exiting");
		return;
	}

	int offset = 0;
	MBinBuffer netbuf;

	while (*pCall) {
		if (m_bTerminated)
			break;

		unsigned char buf[2048];
		int bufSize = Netlib_Recv(pCall->m_hConn, (char *)buf + offset, _countof(buf) - offset, MSG_NODUMP);
		if (bufSize == 0) {
			debugLogA("Voice gateway connection gracefully closed");
			pCall->m_bTerminated = !m_bTerminated;
			break;
		}
		if (bufSize < 0) {
			debugLogA("Voice gateway connection error, exiting");
			break;
		}

		WSHeader hdr;
		if (!WebSocket_InitHeader(hdr, buf, bufSize)) {
			offset += bufSize;
			continue;
		}
		offset = 0;

		// we have some additional data, not only opcode
		if ((size_t)bufSize > hdr.headerSize) {
			size_t currPacketSize = bufSize - hdr.headerSize;
			netbuf.append(buf, bufSize);
			while (currPacketSize < hdr.payloadSize) {
				int result = Netlib_Recv(pCall->m_hConn, (char *)buf, _countof(buf), MSG_NODUMP);
				if (result == 0) {
					debugLogA("Voice gateway connection gracefully closed");
					pCall->m_bTerminated = !m_bTerminated;
					break;
				}
				if (result < 0) {
					debugLogA("Voice gateway connection error, exiting");
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
					CMStringA szJson((char *)netbuf.data() + hdr.headerSize, (int)hdr.payloadSize);
					debugLogA("Voice JSON received:\n%s", szJson.c_str());
					JSONNode root = JSONNode::parse(szJson);
					if (root)
						pCall->process(root);
				}
				break;

			case 8: // close
				debugLogA("Voice server required to exit");
				pCall->m_bTerminated = true; // simply reconnect, don't exit
				break;

			case 9: // ping
				debugLogA("ping received");
				Netlib_Send(pCall->m_hConn, (char *)buf + hdr.headerSize, bufSize - int(hdr.headerSize), 0);
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

			if (prevSize == netbuf.length()) {
				netbuf.remove(prevSize);
				debugLogA("dropping current packet, exiting");
				break;
			}

			prevSize = netbuf.length();
		}
	}

	debugLogA("Exiting voice websocket thread");
	Netlib_CloseHandle(pCall->m_hConn); pCall->m_hConn = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UDP part

void CDiscordVoiceCall::GetConnection(HNETLIBCONN /*hNewConnection*/, uint32_t /*dwRemoteIP*/, void *pExtra)
{
	auto *pThis = (CDiscordVoiceCall *)pExtra;
	pThis->ppro->debugLogA("boo!");
}
