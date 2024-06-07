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

struct CDiscordVoiceClient
{
	CDiscordVoiceCall *m_config;

	OpusEncoder *m_encoder;
	OpusRepacketizer *m_repacketizer;

	bool m_terminating = false;

	CDiscordVoiceClient(CDiscordVoiceCall *pCall);
	~CDiscordVoiceClient();
};

CDiscordVoiceClient::CDiscordVoiceClient(CDiscordVoiceCall *pCall) :
	m_config(pCall)
{
	int iError = 0;
	m_encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &iError);

	m_repacketizer = opus_repacketizer_create();
}

CDiscordVoiceClient::~CDiscordVoiceClient()
{
	if (m_encoder)
		opus_encoder_destroy(m_encoder);

	if (m_repacketizer)
		opus_repacketizer_destroy(m_repacketizer);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void CDiscordProto::VoiceClientThread(void *param)
{
	auto *pCall = (CDiscordVoiceCall *)param;
	pCall->startTime = time(0);

	int nLoops = 0;
	time_t lastLoopTime = time(0);

	CDiscordVoiceClient vc(pCall);
	do {
		time_t currTime = time(0);
		if (currTime - lastLoopTime > 3)
			nLoops = 0;
		
		nLoops++;
		if (nLoops > 5) {
			debugLogA("Too many connection attempts, breaking websocket");
			break;
		}

		lastLoopTime = currTime;
		if (!vc.m_terminating) {
			MHttpHeaders hdrs;
			hdrs.AddHeader("Origin", "https://discord.com");

			NLHR_PTR pReply(WebSocket_Connect(m_hGatewayNetlibUser, pCall->szEndpoint + "/?encoding=json&v=8", &hdrs));
			if (pReply == nullptr) {
				debugLogA("Gateway connection failed, exiting");
				return;
			}

			SleepEx(5000, TRUE);
		}

	} while (!vc.m_terminating);

}
