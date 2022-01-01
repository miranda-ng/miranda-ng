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

/////////////////////////////////////////////////////////////////////////////////////////
// call operations (voice & video)

void CDiscordProto::OnCommandCallCreated(const JSONNode &pRoot)
{
	for (auto &it : pRoot["voice_states"]) {
		SnowFlake channelId = ::getId(pRoot["channel_id"]);
		auto *pUser = FindUserByChannel(channelId);
		if (pUser == nullptr) {
			debugLogA("Call from unknown channel %lld, skipping", channelId);
			continue;
		}

		auto *pCall = new CDiscordVoiceCall();
		pCall->szId = it["session_id"].as_mstring();
		pCall->channelId = channelId;
		pCall->startTime = time(0);
		arVoiceCalls.insert(pCall);

		char *szMessage = TranslateU("Incoming call");
		DBEVENTINFO dbei = {};
		dbei.szModule = m_szModuleName;
		dbei.timestamp = pCall->startTime;
		dbei.eventType = EVENT_INCOMING_CALL;
		dbei.cbBlob = uint32_t(mir_strlen(szMessage) + 1);
		dbei.pBlob = (uint8_t *)szMessage;
		dbei.flags = DBEF_UTF;
		db_event_add(pUser->hContact, &dbei);
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

	int elapsed = 0, currTime = time(0);
	for (auto &call : arVoiceCalls.rev_iter())
		if (call->channelId == channelId) {
			elapsed = currTime - call->startTime;
			arVoiceCalls.removeItem(&call);
			break;
		}

	if (!elapsed) {
		debugLogA("Call from channel %lld isn't registered, skipping", channelId);
		return;
	}

	CMStringA szMessage(FORMAT, TranslateU("Call ended, %d seconds long"), elapsed);
	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = currTime;
	dbei.eventType = EVENT_CALL_FINISHED;
	dbei.cbBlob = uint32_t(szMessage.GetLength() + 1);
	dbei.pBlob = (uint8_t *)szMessage.c_str();
	dbei.flags = DBEF_UTF;
	db_event_add(pUser->hContact, &dbei);
}

void CDiscordProto::OnCommandCallUpdated(const JSONNode &pRoot)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// Events & services

INT_PTR __cdecl CDiscordProto::VoiceCaps(WPARAM, LPARAM)
{
	return VOICE_CAPS_VOICE | VOICE_CAPS_CALL_CONTACT;
}

int __cdecl CDiscordProto::OnVoiceState(WPARAM wParam, LPARAM)
{
	auto *pVoice = (VOICE_CALL *)wParam;
	if (mir_strcmp(pVoice->moduleName, m_szModuleName))
		return 0;

	CDiscordVoiceCall *pCall = nullptr;
	for (auto &it : arVoiceCalls)
		if (it->szId == pVoice->id) {
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
