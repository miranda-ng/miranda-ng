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

void CSteamProto::SendAppInfoRequest(uint32_t appId)
{
	CMsgClientPICSProductInfoRequest__AppInfo appInfo;
	appInfo.appid = appId; appInfo.has_appid = true;
	auto *pInfo = &appInfo;

	CMsgClientPICSProductInfoRequest request;
	request.n_apps = 1;
	request.apps = &pInfo;
	request.has_meta_data_only = request.meta_data_only = true;
	WSSend(EMsg::ClientPICSProductInfoRequest, request);
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

void CSteamProto::SendUserInfoRequest(uint64_t id)
{
	std::vector<uint64_t> ids;
	ids.push_back(id & 0xFFFFFFFFll);
	SendUserInfoRequest(ids);
}

void CSteamProto::SendUserInfoRequest(const std::vector<uint64_t> &ids)
{
	CMsgClientRequestFriendData request;
	request.persona_state_requested = -1; request.has_persona_state_requested = true;
	request.n_friends = ids.size();
	request.friends = (uint64_t*)&*ids.begin();
	WSSend(EMsg::ClientRequestFriendData, request);
}

void CSteamProto::SendUserAddRequest(uint64_t id)
{
	CMsgClientAddFriend request;
	request.has_steamid_to_add = true; request.steamid_to_add = id;
	WSSend(EMsg::ClientAddFriend, request);
}

void CSteamProto::SendUserRemoveRequest(MCONTACT hContact)
{
	CMsgClientRemoveFriend request;
	request.has_friendid = true; request.friendid = SteamIdToAccountId(GetId(hContact, DBKEY_STEAM_ID));
	WSSend(EMsg::ClientRemoveFriend, request);
}

void CSteamProto::SendUserIgnoreRequest(MCONTACT hContact, bool bIgnore)
{
	MBinBuffer payload;
	payload << m_iSteamId << SteamIdToAccountId(GetId(hContact, DBKEY_STEAM_ID)) << uint8_t(bIgnore);
	WSSendRaw(EMsg::ClientSetIgnoreFriend, payload);
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
