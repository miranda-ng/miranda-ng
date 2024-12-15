/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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
			Logout();
			return;
		}
		iHostCount = db_get_dw(0, STEAM_MODULE, DBKEY_HOSTS_COUNT);
	}

	srand(time(0));
	m_ws = nullptr;

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
	WebSocket<CSteamProto> ws(this);

	NLHR_PTR pReply(ws.connect(m_hNetlibUser, szHost));
	if (pReply == nullptr) {
		debugLogA("websocket connection failed");
		return false;
	}

	if (pReply->resultCode != 101) {
		debugLogA("websocket connection failed: %d", pReply->resultCode);
		return false;
	}

	m_ws = &ws;

	debugLogA("Websocket connection succeeded");

	// Send init packets
	Login();

	ws.run();

	Logout();
	m_impl.m_heartBeat.Stop();
	m_ws = nullptr;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WebSocket<CSteamProto>::process(const uint8_t *buf, size_t cbLen)
{
	uint32_t dwSign = *(uint32_t *)buf;
	EMsg msgType = (EMsg)(dwSign & ~STEAM_PROTOCOL_MASK);

	// now process the body
	if (msgType == EMsg::Multi) {
		buf += 8; cbLen -= 8;
		p->ProcessMulti(buf, cbLen);
	}
	else p->ProcessMessage(buf, cbLen);
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

	if (msgType == EMsg::ChannelEncryptRequest || msgType == EMsg::ChannelEncryptResult) {
		CMsgProtoBufHeader hdr;
		hdr.has_jobid_source = hdr.has_jobid_target = true;
		hdr.jobid_source = *(int64_t *)buf; buf += sizeof(int64_t);
		hdr.jobid_target = *(int64_t *)buf; buf += sizeof(int64_t);
		debugLogA("encrypted results cannot be processed, ignoring");
		return;
	}
	
	if (!bIsProto) {
		debugLogA("Got unknown packet, exiting");
		Netlib_Dump(HNETLIBCONN(m_ws->getConn()), buf, cbLen, false, 0);
		return;
	}

	uint32_t hdrLen = *(uint32_t *)buf; buf += sizeof(uint32_t); cbLen -= sizeof(uint32_t);
	proto::MsgProtoBufHeader hdr(buf, hdrLen);
	if (hdr == nullptr) {
		debugLogA("Unable to decode message header, exiting");
		return;
	}

	buf += hdrLen; cbLen -= hdrLen;

	if (hdr->has_client_sessionid)
		m_iSessionId = hdr->client_sessionid;

	MsgCallback pCallback = 0;
	{
		mir_cslock lck(m_csRequests);
		if (auto *pReq = m_arRequests.find((ProtoRequest *)&hdr->jobid_target)) {
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
		OnClientLogon(buf, cbLen);
		break;

	case EMsg::ServiceMethodResponse:
		ProcessServiceResponce(buf, cbLen, hdr->target_job_name);
		break;

	case EMsg::ClientLoggedOff:
		debugLogA("received logout request");
		Logout();
		break;

	default:
		Netlib_Dump(HNETLIBCONN(m_ws->getConn()), buf, cbLen, false, 0);
	}
}

void CSteamProto::ProcessServiceResponce(const uint8_t *buf, size_t cbLen, const char *pszServiceName)
{
	char *tmpName = NEWSTR_ALLOCA(pszServiceName);
	char *p = strchr(tmpName, '.');
	if (!p) {
		debugLogA("Invalid service function: %s", pszServiceName);
		return;
	}

	*p = 0;
	auto it = g_plugin.services.find(tmpName);
	if (it == g_plugin.services.end()) {
		debugLogA("Unregistered service module: %s", tmpName);
		return;
	}
	*p = '.';

	auto pHandler = g_plugin.serviceHandlers.find(tmpName);
	if (pHandler == g_plugin.serviceHandlers.end()) {
		debugLogA("Unsupported service function: %s", pszServiceName);
		return;
	}

	if (char *p1 = strchr(++p, '#'))
		*p1 = 0;

	if (auto *pMethod = protobuf_c_service_descriptor_get_method_by_name(it->second, p)) {
		auto *pDescr = pMethod->output;
		
		if (auto *pMessage = protobuf_c_message_unpack(pDescr, 0, cbLen, buf)) {
			debugLogA("Processing service message: %s\n%s", pszServiceName, protobuf_c_text_to_string(*pMessage).c_str());

			(this->*(pHandler->second))(pMessage);
			protobuf_c_message_free_unpacked(pMessage, 0);
		}
	}
	else debugLogA("Unregistered service method: %s", pszServiceName);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::WSSend(EMsg msgType, const ProtobufCppMessage &msg)
{
	CMsgProtoBufHeader hdr;
	hdr.has_client_sessionid = hdr.has_steamid = hdr.has_jobid_source = hdr.has_jobid_target = true;
	hdr.steamid = m_iSteamId;
	hdr.client_sessionid = m_iSessionId;

	switch (msgType) {
	case EMsg::ClientHello:
		hdr.jobid_source = -1;
		break;

	default:
		hdr.jobid_source = getRandomInt();
		break;
	}

	hdr.jobid_target = -1;

	WSSendHeader(msgType, hdr, msg);
}

void CSteamProto::WSSendHeader(EMsg msgType, const CMsgProtoBufHeader &hdr, const ProtobufCppMessage &msg)
{
	debugLogA("Message sent:\n%s", protobuf_c_text_to_string(msg).c_str());

	uint32_t hdrLen = (uint32_t)protobuf_c_message_get_packed_size(&hdr);
	MBinBuffer hdrbuf(hdrLen);
	protobuf_c_message_pack(&hdr, (uint8_t *)hdrbuf.data());
	hdrbuf.appendBefore(&hdrLen, sizeof(hdrLen));

	uint32_t type = (uint32_t)msgType;
	type |= STEAM_PROTOCOL_MASK;
	hdrbuf.appendBefore(&type, sizeof(type));

	MBinBuffer body(protobuf_c_message_get_packed_size(&msg));
	protobuf_c_message_pack(&msg, body.data());

	hdrbuf.append(body);
	m_ws->sendBinary(hdrbuf.data(), hdrbuf.length());
}

void CSteamProto::WSSendService(const char *pszServiceName, const ProtobufCppMessage &msg, bool bAnon)
{
	CMsgProtoBufHeader hdr;
	hdr.has_client_sessionid = hdr.has_steamid = hdr.has_jobid_source = hdr.has_jobid_target = true;
	hdr.client_sessionid = bAnon ? 0 : m_iSessionId;
	hdr.jobid_source = getRandomInt();
	hdr.jobid_target = -1;
	hdr.target_job_name = (char *)pszServiceName;
	hdr.realm = 1; hdr.has_realm = true;
	WSSendHeader(bAnon ? EMsg::ServiceMethodCallFromClientNonAuthed : EMsg::ServiceMethodCallFromClient, hdr, msg);
}
