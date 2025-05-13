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

void CSteamProto::SendGetChatsRequest()
{
	CChatRoomGetMyChatRoomGroupsRequest request;
	WSSendService(GetMyChatRoomGroups, request);	
}

void CSteamProto::OnGetMyChats(const CChatRoomGetMyChatRoomGroupsResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	std::map<MCONTACT, bool> chatIds;
	for (unsigned i = 0; i < reply.n_chat_room_groups; i++) {
		auto *pGroup = reply.chat_room_groups[i]->group_summary;
		ProcessGroupChat(pGroup);

		for (unsigned k = 0; k < pGroup->n_chat_rooms; k++) {
			auto *pChat = pGroup->chat_rooms[k];
			CMStringW wszId(FORMAT, L"%lld_%lld", pGroup->chat_group_id, pChat->chat_id);

			if (auto *si = Chat_Find(wszId, m_szModuleName))
				chatIds[si->hContact] = true;
		}
	}

	// clean garbage
	for (auto &cc : AccContacts())
		if (Contact::IsGroupChat(cc, m_szModuleName) == GCW_CHATROOM && chatIds.find(cc) == chatIds.end())
			db_delete_contact(cc, CDF_DEL_CONTACT);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::OnChatChanged(const ChatRoomClientNotifyChatGroupUserStateChangedNotification &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	switch (reply.user_action) {
	case ECHAT_ROOM_MEMBER_STATE_CHANGE__k_EChatRoomMemberStateChange_Joined:
		ProcessGroupChat(reply.group_summary);
		break;

	case ECHAT_ROOM_MEMBER_STATE_CHANGE__k_EChatRoomMemberStateChange_Parted:
		LeaveGroupChat(reply.chat_group_id);
		break;
	}
}

void CSteamProto::ProcessGroupChat(const CChatRoomGetChatRoomGroupSummaryResponse *pGroup)
{
	CMStringW wszGrpName;
	if (pGroup->n_chat_rooms > 1 && pGroup->chat_group_name) {
		wszGrpName = CMStringW(m_wszGroupName) + L"\\" + Utf2T(pGroup->chat_group_name);
		if (!Clist_GroupExists(wszGrpName))
			Clist_GroupCreate(0, wszGrpName);
	}

	SESSION_INFO *pOwner = 0;

	for (unsigned k = 0; k < pGroup->n_chat_rooms; k++) {
		std::vector<uint64_t> ids;

		auto *pChat = pGroup->chat_rooms[k];
		CMStringW wszId(FORMAT, L"%lld_%lld", pGroup->chat_group_id, pChat->chat_id);

		CMStringW wszTitle(Utf2T(pChat->chat_name));
		if (wszTitle.IsEmpty())
			wszTitle = Utf2T(pGroup->chat_group_name);

		auto *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszTitle);
		if (pOwner == 0) {
			if (!si->arStatuses.getCount()) {
				Chat_AddGroup(si, TranslateT("Owner"));
				Chat_AddGroup(si, TranslateT("Participant"));

				for (unsigned j = 0; j < pGroup->n_top_members; j++) {
					uint64_t iSteamId = AccountIdToSteamId(pGroup->top_members[j]);
					CMStringW wszUserId(FORMAT, L"%lld", iSteamId), wszNick;

					GCEVENT gce = { si, GC_EVENT_JOIN };
					gce.pszUID.w = wszUserId;

					if (iSteamId == m_iSteamId) {
						gce.bIsMe = true;
						wszNick = getMStringW("Nick");
					}
					else if (MCONTACT hContact = GetContact(iSteamId))
						wszNick = Clist_GetContactDisplayName(hContact);
					else {
						CMStringA szSetting(FORMAT, "UserInfo_%lld", iSteamId);
						ptrW szName(g_plugin.getWStringA(szSetting));
						if (szName)
							wszNick = szName;
						else {
							ids.push_back(iSteamId);
							{
								mir_cslock lck(m_csChats);
								m_chatContactInfo[iSteamId] = si;
							}
							wszNick = L"@" + wszUserId;
						}
					}

					gce.pszNick.w = wszNick;
					gce.pszStatus.w = (pGroup->top_members[j] == pGroup->accountid_owner) ? TranslateT("Owner") : TranslateT("Participant");
					Chat_Event(&gce);
				}
			}
			pOwner = si;
		}
		else si->pParent = pOwner;

		setDword(si->hContact, DBKEY_CHAT_ID, pChat->chat_id);
		if (!wszGrpName.IsEmpty())
			Clist_SetGroup(si->hContact, wszGrpName);

		if (mir_strlen(pGroup->chat_group_tagline)) {
			Utf2T wszTopic(pGroup->chat_group_tagline);
			Chat_SetStatusbarText(si, wszTopic);

			GCEVENT gce = { si, GC_EVENT_TOPIC };
			gce.pszText.w = wszTopic;
			gce.time = time(0);
			Chat_Event(&gce);
		}

		Chat_Control(si, WINDOW_HIDDEN);
		Chat_Control(si, SESSION_ONLINE);

		if (!ids.empty())
			SendUserInfoRequest(ids);

		if (pChat->voice_allowed)
			ExtraIcon_SetIcon(hExtraXStatus, si->hContact, Skin_GetIconHandle(SKINICON_OTHER_SOUND));

		uint32_t dwLastMsgId = getDword(si->hContact, DBKEY_LASTMSG);
		if (pChat->time_last_message > dwLastMsgId)
			SendGetChatHistory(si->hContact, dwLastMsgId);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::OnGotClanInfo(const CMsgClientClanState &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	CMStringW wszId(FORMAT, L"%lld", reply.steamid_clan);
	auto *si = Chat_Find(wszId, m_szModuleName);
	if (si == nullptr) {
		si = Chat_NewSession(GCW_SERVER, m_szModuleName, wszId, reply.name_info ? Utf2T(reply.name_info->clan_name) : wszId.c_str());
		Chat_Control(si, WINDOW_HIDDEN);
		Chat_Control(si, SESSION_ONLINE);
	}

	GCEVENT gce = { si, GC_EVENT_INFORMATION };
	gce.time = time(0);

	if (reply.user_counts) {
		CMStringW wszText;
		auto &C = *reply.user_counts;

		if (C.has_members)
			wszText.AppendFormat(L"%d %s\r\n", C.members, TranslateT("total members"));
		if (C.has_online)
			wszText.AppendFormat(L"%d %s\r\n", C.online, TranslateT("online members"));
		if (C.has_chatting)
			wszText.AppendFormat(L"%d %s\r\n", C.chatting, TranslateT("chatting"));
		if (C.has_in_game)
			wszText.AppendFormat(L"%d %s\r\n", C.in_game, TranslateT("in game"));

		if (!wszText.IsEmpty()) {
			gce.pszText.w = wszText;
			Chat_Event(&gce);
		}
	}

	for (unsigned n = 0; n < reply.n_announcements; n++) {
		auto *E = reply.announcements[n];
		Utf2T wszText(E->headline);

		gce.time = E->event_time;
		gce.pszText.w = wszText;
		Chat_Event(&gce);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendGetChatHistory(MCONTACT hContact, uint32_t iLastMsgId)
{
	CChatRoomGetMessageHistoryRequest request;
	request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
	request.chat_id = getDword(hContact, DBKEY_CHAT_ID); request.has_chat_id = true;
	request.start_time = iLastMsgId;  request.has_start_time = true;
	WSSendService(GetChatHistory, request, (void*)hContact);
}

void CSteamProto::OnGetChatHistory(const CChatRoomGetMessageHistoryResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	std::vector<uint64_t> ids;

	if (auto *si = Chat_Find(UINT_PTR(GetRequestInfo(hdr.jobid_target)), m_szModuleName)) {
		uint32_t iLastMsg = getDword(si->hContact, DBKEY_LASTMSG);
		uint32_t iChatId = getDword(si->hContact, DBKEY_CHAT_ID);

		for (int i = (int)reply.n_messages - 1; i >= 0; i--) {
			auto *pMsg = reply.messages[i];
			if (pMsg->server_timestamp > iLastMsg)
				iLastMsg = pMsg->server_timestamp;

			// some slack, skip it
			if (pMsg->server_message)
				continue;

			auto iSteamId = AccountIdToSteamId(pMsg->sender);
			char szMsgId[100], szUserId[100];
			mir_snprintf(szMsgId, "%d_%d", iChatId, pMsg->server_timestamp);
			_i64toa(iSteamId, szUserId, 10);

			_A2T wszUserId(szUserId);
			USERINFO ui = {};
			ui.pszUID = wszUserId;
			if (!si->getUserList().find(&ui)) {
				ids.push_back(iSteamId);

				mir_cslock lck(m_csChats);
				m_chatContactInfo[iSteamId] = si;
			}

			CMStringA szText(pMsg->message);
			DecodeBbcodes(si, szText);

			DB::EventInfo dbei(db_event_getById(m_szModuleName, szMsgId));
			dbei.flags |= DBEF_UTF;
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.szModule = m_szModuleName;
			dbei.cbBlob = szText.GetLength();
			replaceStr(dbei.pBlob, szText.Detach());
			dbei.iTimestamp = pMsg->server_timestamp;
			dbei.szId = szMsgId;
			dbei.szUserId = szUserId;

			if (dbei.getEvent())
				db_event_edit(dbei.getEvent(), &dbei, true);
			else
				db_event_add(si->hContact, &dbei);
		}

		setDword(si->hContact, DBKEY_LASTMSG, iLastMsg);
	}

	if (!ids.empty())
		SendUserInfoRequest(ids);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::OnGetChatMessage(const CChatRoomIncomingChatMessageNotification &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	std::vector<uint64_t> ids;

	CMStringW wszId(FORMAT, L"%lld_%lld", reply.chat_group_id, reply.chat_id);
	if (auto *si = Chat_Find(wszId, m_szModuleName)) {
		char szMsgId[100], szUserId[100];
		mir_snprintf(szMsgId, "%lld_%d", reply.chat_id, reply.timestamp);
		_i64toa(reply.steamid_sender, szUserId, 10);

		CMStringA szText(reply.message);
		DecodeBbcodes(si, szText);

		_A2T wszUserId(szUserId);
		USERINFO ui = {};
		ui.pszUID = wszUserId;
		if (!si->getUserList().find(&ui)) {
			ids.push_back(reply.steamid_sender);

			mir_cslock lck(m_csChats);
			m_chatContactInfo[reply.steamid_sender] = si;
		}

		DB::EventInfo dbei(db_event_getById(m_szModuleName, szMsgId));
		dbei.flags |= DBEF_UTF;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		dbei.cbBlob = szText.GetLength();
		replaceStr(dbei.pBlob, szText.Detach());
		dbei.iTimestamp = reply.timestamp;
		dbei.szId = szMsgId;
		dbei.szUserId = szUserId;

		if (dbei.getEvent())
			db_event_edit(dbei.getEvent(), &dbei, true);
		else
			db_event_add(si->hContact, &dbei);
	}

	if (!ids.empty())
		SendUserInfoRequest(ids);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CSteamProto::SvcLeaveChat(WPARAM hContact, LPARAM)
{
	CChatRoomLeaveChatRoomGroupRequest request;
	if (Contact::IsGroupChat(hContact) == GCW_SERVER)
		request.chat_group_id = GetId(hContact, DBKEY_GROUP_ID);
	else
		request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID);
	request.has_chat_group_id = true;
	WSSendService(LeaveChatGroup, request);
	return 0;
}

void CSteamProto::LeaveGroupChat(int64_t chatGroupId)
{
	std::vector<MCONTACT> ids;

	for (auto &cc : AccContacts()) {
		if (!Contact::IsGroupChat(cc) || GetId(cc, DBKEY_STEAM_ID) != chatGroupId)
			continue;

		CMStringW wszId(FORMAT, L"%lld_%d", chatGroupId, getDword(cc, DBKEY_CHAT_ID));
		if (auto *si = Chat_Find(wszId, m_szModuleName))
			Chat_Terminate(si);

		ids.push_back(cc);
	}

	for (auto &cc: ids)
		db_delete_contact(cc, CDF_FROM_SERVER);
}

/////////////////////////////////////////////////////////////////////////////////////////

enum
{
	IDM_LEAVE = 1,
};

int CSteamProto::GcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	if (gch == nullptr)
		return 0;

	auto *si = gch->si;
	if (mir_strcmpi(si->pszModule, m_szModuleName))
		return 0;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			CMStringW wszText(gch->ptszText);
			wszText.TrimRight();
			EncodeBbcodes(si, wszText);
			T2Utf szText(wszText);

			CChatRoomSendChatMessageRequest request;
			request.chat_group_id = _wtoi64(si->ptszID); request.has_chat_group_id = true;
			request.chat_id = getDword(si->hContact, DBKEY_CHAT_ID); request.has_chat_id = true;
			request.echo_to_sender = request.has_echo_to_sender = true;
			request.message = szText;
			WSSendService(SendChatMessage, request);
		}
		break;

	case GC_USER_PRIVMESS:
		Chat_SendPrivateMessage(gch);
		break;

	case GC_USER_LOGMENU:
		Chat_LogMenu(gch);
		break;

	case GC_USER_NICKLISTMENU:
		break;
	}
	return 0;
}

void CSteamProto::Chat_SendPrivateMessage(GCHOOK *gch)
{
	uint64_t iSteamId = _wtoi64(gch->ptszUID);
	MCONTACT hContact = GetContact(iSteamId);
	if (!hContact) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.id.w = (wchar_t *)gch->ptszUID;
		psr.firstName.w = (wchar_t *)gch->ptszNick;

		hContact = AddToList(PALF_TEMPORARY, &psr);
		if (hContact == 0)
			return;

		setWString(hContact, "Nick", gch->ptszNick);
		Contact::Hide(hContact);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}

	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}

void CSteamProto::Chat_LogMenu(GCHOOK *gch)
{
	switch (gch->dwData) {
	case IDM_LEAVE:
		SvcLeaveChat(gch->si->hContact, 0);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENW("&Leave chat session"), IDM_LEAVE, MENU_ITEM }
};

int CSteamProto::GcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS *)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
	}
	return 0;
}
