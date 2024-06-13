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
	delete m_ws;

	m_timer.StopSafe();

	if (m_encoder) {
		opus_encoder_destroy(m_encoder);
		m_encoder = nullptr;
	}

	if (m_repacketizer) {
		opus_repacketizer_destroy(m_repacketizer);
		m_repacketizer = nullptr;
	}

	if (m_hBind) {
		Netlib_CloseHandle(m_hBind);
		m_hBind = nullptr;
	}
}

void CDiscordVoiceCall::write(int op, JSONNode &d)
{
	if (m_ws == nullptr)
		return;

	d.set_name("d");

	JSONNode payload;
	payload << INT_PARAM("op", op) << d;

	auto json = payload.write();
	ppro->debugLogA("Voice JSON sent: %s", json.c_str());

	mir_cslock lck(m_cs);
	m_ws->sendText(json.c_str());
}

void JsonWebSocket<CDiscordVoiceCall>::process(const JSONNode &json)
{
	p->process(json);
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
	nlb.iType = SOCK_DGRAM;  // UDP connection
	nlb.pExtra = this;
	nlb.pfnNewConnection = &GetConnection;
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

	MHttpHeaders hdrs;
	hdrs.AddHeader("Origin", "https://discord.com");

	JsonWebSocket<CDiscordVoiceCall> ws(pCall);

	NLHR_PTR pReply(ws.connect(m_hGatewayNetlibUser, pCall->szEndpoint + "/?encoding=json&v=8", &hdrs));
	if (!pReply || pReply->resultCode != 101) {
		debugLogA("Voice gateway connection failed, exiting");
		return;
	}

	pCall->m_ws = &ws;
	ws.run();
	pCall->m_ws = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UDP part

void CDiscordVoiceCall::GetConnection(HNETLIBCONN /*hNewConnection*/, uint32_t /*dwRemoteIP*/, void *pExtra)
{
	auto *pThis = (CDiscordVoiceCall *)pExtra;
	pThis->ppro->debugLogA("boo!");
}
