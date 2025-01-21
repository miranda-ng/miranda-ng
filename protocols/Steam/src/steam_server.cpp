/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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
	WSSend(EMsg::ClientPICSProductInfoRequest, request);
}

void CSteamProto::OnGotAppInfo(const CMsgClientPICSProductInfoResponse &reply, const CMsgProtoBufHeader &)
{
	for (int i = 0; i < reply.n_apps; i++) {
		auto *pApp = reply.apps[i];

		if (pApp->buffer.len) {
			std::regex regex("\"name\"\\s*\"(.+?)\"");
			std::smatch match;
			std::string content((char *)pApp->buffer.data, pApp->buffer.len);
			if (std::regex_search(content, match, regex)) {
				std::string szName = match[1];
				CMStringA szSetting(FORMAT, "AppInfo_%d", pApp->appid);
				g_plugin.setUString(szSetting, szName.c_str());

				for (auto &cc : AccContacts()) {
					if (getDword(cc, "XStatusId") == pApp->appid) {
						setWString(cc, "XStatusName", TranslateT("Playing"));
						setUString(cc, "XStatusMsg", szName.c_str());
}	}	}	}	}	}

void CSteamProto::SendDeleteMessageRequest()
{
	if (m_deletedContact == INVALID_CONTACT_ID)
		return;

	m_impl.m_deleteMsg.Stop();

	MCONTACT hContact;
	OBJLIST<CChatRoomDeleteChatMessagesRequest__Message> msgs((int)m_deletedMessages.size());
	{
		mir_cslock lck(m_csChats);
		for (auto &it : m_deletedMessages) {
			CChatRoomDeleteChatMessagesRequest__Message msg;
			msg.has_server_timestamp = true; msg.server_timestamp = it;
			msgs.insert(new CChatRoomDeleteChatMessagesRequest__Message(msg));
		}

		hContact = m_deletedContact;
		m_deletedContact = INVALID_CONTACT_ID;
		m_deletedMessages.clear();
	}

	CChatRoomDeleteChatMessagesRequest request;
	request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
	request.chat_id = getDword(hContact, "ChatId"); request.has_chat_id = true;
	request.messages = msgs.getArray();
	request.n_messages = msgs.getCount();
	WSSendService(DeleteChatMessage, request);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendDeviceListRequest()
{
	CDeviceAuthGetOwnAuthorizedDevicesRequest request;
	request.steamid = m_iSteamId; request.has_steamid = true;
	WSSendService(GetOwnAuthorizedDevices, request);
}

void CSteamProto::OnGotDeviceList(const CDeviceAuthGetOwnAuthorizedDevicesResponse &, const CMsgProtoBufHeader &)
{
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

void CSteamProto::SendPersonaStatus(int status)
{
	CMsgClientChangeStatus request;
	request.persona_state = (int)MirandaToSteamState(status); request.has_persona_state = true;
	WSSend(EMsg::ClientChangeStatus, request);
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

/////////////////////////////////////////////////////////////////////////////////////////

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
