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
	
	auto iSourceId = WSSendService(FriendSendMessage, request);
	mir_cslock lck(m_csOwnMessages);
	if (COwnMessage *pOwn = m_arOwnMessages.find((COwnMessage *)&msgId))
		pOwn->iSourceId = iSourceId;
}

void CSteamProto::OnMessageSent(const CFriendMessagesSendMessageResponse &reply, const CMsgProtoBufHeader &hdr)
{
	COwnMessage tmp(0, 0);
	{
		mir_cslock lck(m_csOwnMessages);
		for (auto &it : m_arOwnMessages)
			if (it->iSourceId == hdr.jobid_target) {
				tmp = *it;
				m_arOwnMessages.remove(m_arOwnMessages.indexOf(&it));
				break;
			}
	}

	if (!tmp.hContact)
		return;

	if (hdr.failed()) {
		CMStringW wszMessage(FORMAT, TranslateT("Message sending has failed with error %d"), hdr.eresult);
		ProtoBroadcastAck(tmp.hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tmp.iMessageId, (LPARAM)wszMessage.c_str());
	}
	else {
		uint32_t timestamp = (reply.has_server_timestamp) ? reply.server_timestamp : 0;
		if (timestamp > getDword(tmp.hContact, DB_KEY_LASTMSGTS))
			setDword(tmp.hContact, DB_KEY_LASTMSGTS, timestamp);

		tmp.timestamp = timestamp;
		ProtoBroadcastAck(tmp.hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tmp.iMessageId, 0);
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
