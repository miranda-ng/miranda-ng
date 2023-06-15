/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void __cdecl CSteamProto::ServerThread(void *)
{
	// load web socket servers first if needed
	int iTimeDiff = db_get_dw(0, STEAM_MODULE, DBKEY_HOSTS_DATE);
	int iHostCount = db_get_dw(0, STEAM_MODULE, DBKEY_HOSTS_COUNT);
	if (!iHostCount || time(0) - iTimeDiff > 3600 * 24 * 7) { // once a week
		if (!SendRequest(new GetHostsRequest(), &CSteamProto::OnGotHosts)) {
			LoginFailed();
			return;
		}
	}

	srand(time(0));
	m_hServerConn = nullptr;

	CMStringA szHost;
	do {
		szHost.Format("Host%d", rand() % iHostCount);
		szHost = db_get_sm(0, STEAM_MODULE, szHost);
		szHost.Insert(0, "wss://");
		szHost += "/cmsocket/";
	}
	while (ServerThreadStub(szHost));
}

bool CSteamProto::ServerThreadStub(const char *szHost)
{
	NLHR_PTR pReply(WebSocket_Connect(m_hNetlibUser, szHost));
	if (pReply == nullptr) {
		debugLogA("websocket connection failed");
		return false;
	}

	if (pReply->resultCode != 101) {
		debugLogA("websocket connection failed: %d", pReply->resultCode);
		return false;
	}

	m_hServerConn = pReply->nlc;
	debugLogA("Websocket connection succeeded");

	// Send init packets
	Login();

	bool bExit = false;
	int offset = 0;
	MBinBuffer netbuf;

	while (!bExit) {
		if (m_bTerminated)
			break;

		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hServerConn, (char *)buf + offset, _countof(buf) - offset, 0);
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
				int result = Netlib_Recv(m_hServerConn, (char *)buf, _countof(buf), 0);
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
				if (hdr.bIsFinal)
					ProcessPacket((const uint8_t *)netbuf.data() + hdr.headerSize, hdr.payloadSize);
				break;

			case 8: // close
				debugLogA("server required to exit");
				bExit = true; // simply reconnect, don't exit
				break;

			case 9: // ping
				debugLogA("ping received");
				Netlib_Send(m_hServerConn, (char *)buf + hdr.headerSize, bufSize - int(hdr.headerSize), 0);
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

	Netlib_CloseHandle(m_hServerConn);
	m_hServerConn = nullptr;
	return bExit;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::ProcessPacket(const uint8_t *buf, size_t cbLen)
{
	uint32_t dwSign = *(uint32_t *)buf;
	EMsg msgType = (EMsg)(dwSign & ~STEAM_PROTOCOL_MASK);

	// now process the body
	if (msgType == EMsg::Multi) {
		buf += 8; cbLen -= 8;
		ProcessMulti(buf, cbLen);
	}
	else ProcessMessage(buf, cbLen);
}

void CSteamProto::ProcessMulti(const uint8_t *buf, size_t cbLen)
{
	proto::MsgMulti pMulti(buf, cbLen);
	if (pMulti == nullptr) {
		debugLogA("Unable to decode multi message, exiting");
		return;
	}

	debugLogA("processing %s multi message of size %d", (pMulti->size_unzipped) ? "zipped" : "normal", pMulti->message_body.len);
	
	ptrA tmp;
	if (pMulti->size_unzipped) {
		tmp = (char *)mir_alloc(pMulti->size_unzipped + 1);
		cbLen = FreeImage_ZLibGUnzip((uint8_t*)tmp.get(), pMulti->size_unzipped, pMulti->message_body.data, (unsigned)pMulti->message_body.len);
		if (!cbLen) {
			debugLogA("Unable to unzip multi message, exiting");
			return;
		}

		buf = (const uint8_t *)tmp.get();
	}
	else {
		buf = pMulti->message_body.data;
		cbLen = pMulti->message_body.len;
	}

	while ((int)cbLen > 0) {
		uint32_t cbPacketLen = *(uint32_t *)buf; buf += sizeof(uint32_t); cbLen -= sizeof(uint32_t);
		ProcessMessage(buf, cbPacketLen);
		buf += cbPacketLen; cbLen -= cbPacketLen;
	}
}

void CSteamProto::ProcessMessage(const uint8_t *buf, size_t cbLen)
{
	uint32_t dwSign = *(uint32_t *)buf; buf += sizeof(uint32_t); cbLen -= sizeof(uint32_t);
	EMsg msgType = (EMsg)(dwSign & ~STEAM_PROTOCOL_MASK);
	bool bIsProto = (dwSign & STEAM_PROTOCOL_MASK) != 0;

	CMsgProtoBufHeader hdr;

	if (msgType == EMsg::ChannelEncryptRequest || msgType == EMsg::ChannelEncryptResult) {
		hdr.has_jobid_source = hdr.has_jobid_target = true;
		hdr.jobid_source = *(int64_t *)buf; buf += sizeof(int64_t);
		hdr.jobid_target = *(int64_t *)buf; buf += sizeof(int64_t);
	}
	else if (bIsProto) {
		uint32_t hdrLen = *(uint32_t *)buf; buf += sizeof(uint32_t); cbLen -= sizeof(uint32_t);
		proto::MsgProtoBufHeader tmpHeader(buf, hdrLen);
		if (tmpHeader == nullptr) {
			debugLogA("Unable to decode message header, exiting");
			return;
		}

		memcpy(&hdr, tmpHeader, sizeof(hdr));
		buf += hdrLen; cbLen -= hdrLen;
	}
	else {
		debugLogA("Got unknown header, exiting");
		return;
	}

	MsgCallback pCallback = 0;
	{
		mir_cslock lck(m_csRequests);
		if (auto *pReq = m_arRequests.find((ProtoRequest *)&hdr.jobid_target)) {
			pCallback = pReq->pCallback;
			m_arRequests.remove(pReq);
		}
	}

	if (pCallback) {
		(this->*pCallback)(buf, cbLen);
		return;
	}

	// persistent callbacks
	switch (msgType) {
	case EMsg::ClientLogOnResponse:
		OnLoggedOn(buf, cbLen);
		break;
	}
}
