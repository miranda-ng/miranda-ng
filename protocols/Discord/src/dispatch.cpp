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

extern HWND g_hwndHeartbeat;

void CALLBACK CDiscordProto::HeartbeatTimerProc(HWND, UINT, UINT_PTR id, DWORD)
{
	((CDiscordProto*)id)->GatewaySendHeartbeat();
}

static void __stdcall sttStartTimer(void *param)
{
	CDiscordProto *ppro = (CDiscordProto*)param;
	SetTimer(g_hwndHeartbeat, (UINT_PTR)param, ppro->getHeartbeatInterval(), &CDiscordProto::HeartbeatTimerProc);
}

void CDiscordProto::OnCommandReady(const JSONNode &pRoot)
{
	GatewaySendHeartbeat();
	CallFunctionAsync(sttStartTimer, this);

	m_szGatewaySessionId = pRoot["session_id"].as_mstring();

	const JSONNode &relations = pRoot["relationships"];
	for (auto it = relations.begin(); it != relations.end(); ++it) {
		const JSONNode &p = *it;

		const JSONNode &user = p["user"];
		if (user)
			PrepareUser(user);
	}

	const JSONNode &channels = pRoot["private_channels"];
	for (auto it = channels.begin(); it != channels.end(); ++it) {
		const JSONNode &p = *it;

		const JSONNode &user = p["recipient"];
		if (!user)
			continue;

		CDiscordUser *pUser = PrepareUser(user);
		pUser->lastMessageId = _wtoi64(p["last_message_id"].as_mstring());
		pUser->channelId = _wtoi64(p["id"].as_mstring());
		pUser->bIsPrivate = true;

		setId(pUser->hContact, DB_KEY_CHANNELID, pUser->channelId);
	}
}
