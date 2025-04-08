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
	if (!m_bConnected)
		return false;

	m_ws.sendText(pRoot.write().c_str());
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
	bool bHasCookie = false;
	MHttpHeaders hdrs;
	hdrs.AddHeader("Origin", "https://discord.com");
	if (!m_szWSCookie.IsEmpty()) {
		bHasCookie = true;
		hdrs.AddHeader("Cookie", m_szWSCookie);
	}

	NLHR_PTR pReply(m_ws.connect(m_hGatewayNetlibUser, m_szGateway + "/?encoding=json&v=8", &hdrs));
	if (pReply == nullptr) {
		debugLogA("Gateway connection failed, exiting");
		return false;
	}

	m_szWSCookie = pReply->GetCookies();

	if (pReply->resultCode != 101) {
		// if there's no cookie & Miranda is bounced with error 404, simply apply the cookie and try again
		if (pReply->resultCode == 404) {
			if (!bHasCookie)
				return true;

			m_szWSCookie.Empty(); // don't use the same cookie twice
		}
		return false;
	}

	// succeeded!
	debugLogA("Gateway connection succeeded");

	m_bConnected = true;
	m_ws.run();
	m_bConnected = false;
	return !m_bTerminated;
}

//////////////////////////////////////////////////////////////////////////////////////
// handles server commands

void JsonWebSocket<CDiscordProto>::process(const JSONNode &json)
{
	int opCode = json["op"].as_int();
	switch (opCode) {
	case OPCODE_DISPATCH:  // process incoming command
		{
			int iSeq = json["s"].as_int();
			if (iSeq != 0)
				p->m_iGatewaySeq = iSeq;

			CMStringW wszCommand = json["t"].as_mstring();
			p->debugLogA("got a server command to dispatch: %S", wszCommand.c_str());

			GatewayHandlerFunc pFunc = p->GetHandler(wszCommand);
			if (pFunc)
				(p->*pFunc)(json["d"]);
		}
		break;

	case OPCODE_RECONNECT:  // we need to reconnect asap
		p->debugLogA("we need to reconnect, leaving worker thread");
		p->m_ws.terminate();
		return;

	case OPCODE_INVALID_SESSION:  // session invalidated
		if (json["d"].as_bool()) // session can be resumed
			p->GatewaySendResume();
		else {
			Sleep(5000); // 5 seconds - recommended timeout
			p->GatewaySendIdentify();
		}
		break;

	case OPCODE_HELLO: // hello
		p->m_iHartbeatInterval = json["d"]["heartbeat_interval"].as_int();

		p->GatewaySendIdentify();
		break;

	case OPCODE_HEARTBEAT_ACK: // heartbeat ack
		break;

	default:
		p->debugLogA("ACHTUNG! Unknown opcode: %d, report it to developer", opCode);
	}
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
	payload << SINT64_PARAM("guild_id", pGuild->m_id) << BOOL_PARAM("typing", true) << BOOL_PARAM("activities", true) << BOOL_PARAM("presences", true) << channels;
		
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

/////////////////////////////////////////////////////////////////////////////////////////

bool CDiscordProto::GatewaySendStatus(int iStatus, const wchar_t *pwszStatusText)
{
	// if (iStatus == ID_STATUS_OFFLINE)
	//	return true;

	const char *pszStatus;
	switch (iStatus) {
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
		pszStatus = "idle"; break;
	case ID_STATUS_DND:
		pszStatus = "dnd"; break;
	case ID_STATUS_INVISIBLE:
		pszStatus = "invisible"; break;
	case ID_STATUS_OFFLINE:
		pszStatus = "offline"; break;
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

/////////////////////////////////////////////////////////////////////////////////////////

bool CDiscordProto::GatewaySendVoice(JSONNode &payload)
{
	payload.set_name("d");

	JSONNode root; root << INT_PARAM("op", OPCODE_VOICE_UPDATE) << payload;
	return GatewaySend(root);
}
