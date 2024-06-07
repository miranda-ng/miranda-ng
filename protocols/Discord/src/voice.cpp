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

CDiscordVoiceState::CDiscordVoiceState(const JSONNode &node)
{
	m_userId = ::getId(node["user_id"]);
	m_channelId = ::getId(node["channel_id"]);
	m_sessionId = node["session_id"].as_mstring();
	m_bDeaf = node["deaf"].as_bool();
	m_bMute = node["mute"].as_bool();
	m_bSuppress = node["suppress"].as_bool();
	m_bSelfDeaf = node["self_deaf"].as_bool();
	m_bSelfMute = node["self_mute"].as_bool();
	m_nSelfVideo = node["self_video"].as_bool();
}

CDiscordVoiceCall* CDiscordProto::FindCall(SnowFlake channelId)
{
	mir_cslock lck(m_csVoice);
	if (auto *pCall = arVoiceCalls.find((CDiscordVoiceCall *)&channelId))
		return pCall;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// voice server commands

void CDiscordProto::VoiceChannelConnect(MCONTACT hContact)
{
	SnowFlake channelId = getId(hContact, DB_KEY_CHANNELID);
	if (!hContact || !channelId)
		return;

	if (auto *pUser = FindUserByChannel(channelId)) {
		if (auto *pGuild = pUser->pGuild) {
			{
				mir_cslock lck(m_csVoice);

				// a voice call for this guild is already being establishing, exit
				if (pGuild->pVoiceCall)
					return;

				pGuild->pVoiceCall = new CDiscordVoiceCall();
				pGuild->pVoiceCall->guildId = pGuild->m_id;
				pGuild->pVoiceCall->channelId = channelId;
			}
			JSONNode payload;
			payload << INT64_PARAM("guild_id", pGuild->m_id) << INT64_PARAM("channel_id", channelId)
				<< BOOL_PARAM("self_mute", false) << BOOL_PARAM("self_deaf", false);
			GatewaySendVoice(payload);
		}
	}
}

void CDiscordProto::TryVoiceStart(CDiscordGuild *pGuild)
{
	if (auto *pCall = pGuild->pVoiceCall) {
		// not enough data, waiting for the second command
		if (pCall->szSessionId.IsEmpty() || pCall->szToken.IsEmpty() || pCall->szEndpoint.IsEmpty())
			return;

		// transfer a call from guild to the concrete channel
		pGuild->pVoiceCall = nullptr;
		if (auto *pUser = FindUserByChannel(pCall->channelId)) {
			arVoiceCalls.insert(pCall);
			ForkThread(&CDiscordProto::VoiceClientThread, pCall);
		}
		else {
			delete pCall;
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// call operations (voice & video)

void CDiscordProto::OnCommandCallCreated(const JSONNode &pRoot)
{
	for (auto &it : pRoot["voice_states"]) {
		SnowFlake channelId = ::getId(it["channel_id"]);
		auto *pUser = FindUserByChannel(channelId);
		if (pUser == nullptr) {
			debugLogA("Call from unknown channel %lld, skipping", channelId);
			continue;
		}

		if (auto *pCall = FindCall(channelId)) {
			pCall->startTime = time(0);
		}
		else debugLogA("Unregistered call received from channel %lld, skipping", channelId);
	}
}

void CDiscordProto::OnCommandCallDeleted(const JSONNode &pRoot)
{
	SnowFlake channelId = ::getId(pRoot["channel_id"]);
	auto *pUser = FindUserByChannel(channelId);
	if (pUser == nullptr) {
		debugLogA("Call from unknown channel %lld, skipping", channelId);
		return;
	}

	if (auto *pCall = FindCall(channelId)) {
		// int currTime = time(0);
		for (auto &call : arVoiceCalls.rev_iter()) {
			if (call->channelId == channelId) {
				arVoiceCalls.removeItem(&call);
				break;
			}
		}
	}
	else debugLogA("Unregistered call received from channel %lld, skipping", channelId);
}

void CDiscordProto::OnCommandCallUpdated(const JSONNode&)
{
}

void CDiscordProto::OnCommandVoiceServerUpdate(const JSONNode &pRoot)
{
	if (auto *pGuild = FindGuild(::getId(pRoot["guild_id"]))) {
		mir_cslock lck(m_csVoice);
		if (auto *pCall = pGuild->pVoiceCall) {
			pCall->szToken = pRoot["token"].as_mstring();
			pCall->szEndpoint = pRoot["endpoint"].as_mstring();
			TryVoiceStart(pGuild);
		}
	}
}

void CDiscordProto::OnCommandVoiceStateUpdate(const JSONNode &pRoot)
{
	CDiscordVoiceState vs(pRoot);

	if (auto *pGuild = FindGuild(::getId(pRoot["guild_id"]))) {
		if (vs.m_channelId == 0) {
			for (auto &it : pGuild->arVoiceStates.rev_iter())
				if (it->m_userId == vs.m_userId)
					pGuild->arVoiceStates.removeItem(&it);

			// if (vs.m_userId == m_ownId)
			//		disconnect voiсe from guild
		}
		else {
			auto *pVS = pGuild->arVoiceStates.find(&vs);
			if (pVS)
				*pVS = vs;
			else
				pGuild->arVoiceStates.insert(new CDiscordVoiceState(vs));

			// if our voice call to this guild is in progress, assign session id & call it
			if (vs.m_userId == m_ownId) {
				mir_cslock lck(m_csVoice);
				if (auto *pCall = pGuild->pVoiceCall) {
					pCall->szSessionId = vs.m_sessionId;
					TryVoiceStart(pGuild);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Events & services

INT_PTR CDiscordProto::VoiceCaps(WPARAM, LPARAM)
{
	return VOICE_CAPS_VOICE | VOICE_CAPS_CALL_CONTACT;
}

INT_PTR CDiscordProto::VoiceCanCall(WPARAM hContact, LPARAM)
{
	if (auto *pUser = FindUser(getId(hContact, DB_KEY_ID)))
		if (pUser->bIsPrivate || pUser->bIsVoice)
			return TRUE;

	return FALSE;
}

INT_PTR CDiscordProto::VoiceCallCreate(WPARAM hContact, LPARAM)
{
	VoiceChannelConnect(hContact);
	return 0;
}

INT_PTR CDiscordProto::VoiceCallAnswer(WPARAM, LPARAM)
{
	return 0;
}

INT_PTR CDiscordProto::VoiceCallCancel(WPARAM, LPARAM)
{
	return 0;
}

int CDiscordProto::OnVoiceState(WPARAM wParam, LPARAM)
{
	auto *pVoice = (VOICE_CALL *)wParam;
	if (mir_strcmp(pVoice->moduleName, m_szModuleName))
		return 0;

	CDiscordVoiceCall *pCall = nullptr;
	for (auto &it : arVoiceCalls)
		if (it->szSessionId == pVoice->id) {
			pCall = it;
			break;
		}

	if (pCall == nullptr) {
		debugLogA("Unknown call: %s, exiting", pVoice->id);
		return 0;
	}

	debugLogA("Call %s state changed to %d", pVoice->id, pVoice->state);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::InitVoip(bool bEnable)
{
	if (!g_plugin.bVoiceService)
		return;

	if (bEnable) {
		VOICE_MODULE vsr = {};
		vsr.cbSize = sizeof(VOICE_MODULE);
		vsr.description = m_tszUserName;
		vsr.name = m_szModuleName;
		vsr.icon = g_plugin.getIconHandle(IDI_MAIN);
		vsr.flags = VOICE_CAPS_VOICE | VOICE_CAPS_CALL_CONTACT;
		CallService(MS_VOICESERVICE_REGISTER, (WPARAM)&vsr, 0);
	}
	else {
		// TerminateSession();
		CallService(MS_VOICESERVICE_UNREGISTER, (WPARAM)m_szModuleName, 0);
	}
}
