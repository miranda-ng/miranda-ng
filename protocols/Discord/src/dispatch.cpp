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

#pragma pack(4)

struct CDiscordCommand
{
	const wchar_t *szCommandId;
	GatewayHandlerFunc pFunc;
}
static handlers[] = // these structures must me sorted alphabetically
{
	{ L"MESSAGE_CREATE", &CDiscordProto::OnCommandMessage },
	{ L"MESSAGE_UPDATE", &CDiscordProto::OnCommandMessage },

	{ L"PRESENCE_UPDATE", &CDiscordProto::OnCommandPresence },

	{ L"READY", &CDiscordProto::OnCommandReady },

	{ L"TYPING_START", &CDiscordProto::OnCommandTyping },
};

static int __cdecl pSearchFunc(const void *p1, const void *p2)
{
	return wcscmp(((CDiscordCommand*)p1)->szCommandId, ((CDiscordCommand*)p2)->szCommandId);
}

GatewayHandlerFunc CDiscordProto::GetHandler(const wchar_t *pwszCommand)
{
	CDiscordCommand tmp = { pwszCommand, NULL };
	CDiscordCommand *p = (CDiscordCommand*)bsearch(&tmp, handlers, _countof(handlers), sizeof(handlers[0]), pSearchFunc);
	return (p != NULL) ? p->pFunc : NULL;
}

//////////////////////////////////////////////////////////////////////////////////////
// reading a new message

void CDiscordProto::OnCommandMessage(const JSONNode &pRoot)
{
	PROTORECVEVENT recv = {};

	CDiscordUser *pUser = PrepareUser(pRoot["author"]);
	SnowFlake channelId = _wtoi64(pRoot["channel_id"].as_mstring());
	CMStringW msgId = pRoot["id"].as_mstring();
	CMStringW wszText = pRoot["content"].as_mstring();

	// if a message has myself as an author, mark it as sent
	if (pUser->id == 0)
		return;

	const JSONNode &edited = pRoot["edited_timestamp"];
	if (!edited.isnull())
		wszText.AppendFormat(L" (%s %s)", TranslateT("edited at"), edited.as_mstring().c_str());

	if (pUser->channelId != channelId) {
		debugLogA("failed to process a groupchat message, exiting");
		return;
	}
	
	ptrA buf(mir_utf8encodeW(wszText));
	recv.timestamp = (DWORD)StringToDate(pRoot["timestamp"].as_mstring());
	recv.szMessage = buf;
	recv.lParam = (LPARAM)msgId.c_str();
	ProtoChainRecvMsg(pUser->hContact, &recv);
}

//////////////////////////////////////////////////////////////////////////////////////
// someone changed its status

void CDiscordProto::OnCommandPresence(const JSONNode &pRoot)
{
	CDiscordUser *pUser = PrepareUser(pRoot["user"]);
	if (pUser == NULL)
		return;

	int iStatus;
	CMStringW wszStatus = pRoot["status"].as_mstring();
	if (wszStatus == L"idle")
		iStatus = ID_STATUS_IDLE;
	else if (wszStatus == L"online")
		iStatus = ID_STATUS_ONLINE;
	else if (wszStatus == L"offline")
		iStatus = ID_STATUS_OFFLINE;
	else
		iStatus = 0;

	if (iStatus != 0)
		setWord(pUser->hContact, "Status", iStatus);

	CMStringW wszGame = pRoot["game"]["name"].as_mstring();
	if (!wszGame.IsEmpty())
		setWString(pUser->hContact, "XStatusMsg", wszGame);
	else
		delSetting(pUser->hContact, "XStatusMsg");		
}

//////////////////////////////////////////////////////////////////////////////////////
// gateway session start

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

//////////////////////////////////////////////////////////////////////////////////////
// UTN support

void CDiscordProto::OnCommandTyping(const JSONNode &pRoot)
{
	SnowFlake userId = _wtoi64(pRoot["user_id"].as_mstring());
	SnowFlake channelId = _wtoi64(pRoot["channel_id"].as_mstring());
	debugLogA("user typing notification: userid=%lld, channelid=%lld", userId, channelId);

	CDiscordUser *pUser = FindUser(userId);
	if (pUser == NULL) {
		debugLogA("user with id=%lld is not found", userId);
		return;
	}

	if (pUser->channelId == channelId) {
		debugLogA("user is typing in his private channel");
		CallService(MS_PROTO_CONTACTISTYPING, pUser->hContact, 20);
	}
	else {
		debugLogA("user is typing in a group channel, skipped");
	}
}
