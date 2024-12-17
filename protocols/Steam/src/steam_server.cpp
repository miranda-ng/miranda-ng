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

void CSteamProto::OnGotNotification(const CSteamNotificationNotificationsReceivedNotification &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.eresult != 1)
		return;

	debugLogA("got %d notifications", reply.n_notifications);

	for (int i = 0; i < reply.n_notifications; i++) {
		auto *N = reply.notifications[i];
		debugLogA("notification type %d: %s", N->notification_type, N->body_data);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendPersonaStatus(int status)
{
	CMsgClientChangeStatus request;
	request.persona_state = (int)MirandaToSteamState(status); request.has_persona_state = true;
	WSSend(EMsg::ClientChangeStatus, request);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendFriendActiveSessions()
{
	CFriendsMessagesGetActiveMessageSessionsRequest request;
	request.has_lastmessage_since = true;
	request.has_only_sessions_with_messages = false; request.only_sessions_with_messages = true;
	WSSendService(FriendGetActiveSessions, request);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendUserInfoRequest(uint64_t id, bool bRetrieveState)
{
	std::vector<uint64_t> ids;
	ids.push_back(id & 0xFFFFFFFFll);
	SendUserInfoRequest(ids, bRetrieveState);
}

void CSteamProto::SendUserInfoRequest(const std::vector<uint64_t> &ids, bool bRetrieveState)
{
	CMsgClientRequestFriendData request;
	request.persona_state_requested = bRetrieveState; request.has_persona_state_requested = true;
	request.n_friends = ids.size();
	request.friends = (uint64_t*)&*ids.begin();
	WSSend(EMsg::ClientRequestFriendData, request);
}

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
