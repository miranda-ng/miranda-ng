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

void CSteamProto::SendFriendMessage(uint32_t msgId, int64_t steamId, const char *pszMessage)
{
	CMStringA szId(FORMAT, "%d", msgId);

	CFriendMessagesSendMessageRequest request;
	request.chat_entry_type = (int)EChatEntryType::ChatMsg; request.has_chat_entry_type = true;
	request.client_message_id = szId.GetBuffer();
	request.contains_bbcode = request.has_contains_bbcode = true;
	request.steamid = steamId; request.has_steamid = true;
	request.message = (char *)pszMessage;
	WSSendService(FriendSendMessage, request);
}

void CSteamProto::OnMessageSent(const CFriendMessagesSendMessageResponse *pResponse)
{
	COwnMessage *pOwn;
	{
		mir_cslock lck(m_csOwnMessages);
		pOwn = m_arOwnMessages.find((COwnMessage *)&pResponse->ordinal);
	}

	if (pOwn) {
		uint32_t timestamp = (pResponse->has_server_timestamp) ? pResponse->server_timestamp : 0;
		if (timestamp > getDword(pOwn->hContact, DB_KEY_LASTMSGTS))
			setDword(pOwn->hContact, DB_KEY_LASTMSGTS, timestamp);

		pOwn->timestamp = timestamp;
		ProtoBroadcastAck(pOwn->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pOwn->iMessageId, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendHeartBeat()
{
	CMsgClientHeartBeat packet;
	packet.has_send_reply = true; packet.send_reply = false;
	WSSend(EMsg::ClientHeartBeat, packet);
}

void CSteamProto::SendLogout()
{
	CMsgClientLogOff packet;
	WSSend(EMsg::ClientLogOff, packet);
}
