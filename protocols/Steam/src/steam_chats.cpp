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

	for (int i = 0; i < reply.n_chat_room_groups; i++) {
		auto *pGroup = reply.chat_room_groups[i]->group_summary;

		SESSION_INFO *pOwner = 0;

		for (int k = 0; k < pGroup->n_chat_rooms; k++) {
			auto *pChat = pGroup->chat_rooms[k];
			CMStringW wszId(FORMAT, L"%lld_%d", pGroup->chat_group_id, pChat->chat_id);

			CMStringW wszTitle(Utf2T(pChat->chat_name));
			if (wszTitle.IsEmpty())
				wszTitle = Utf2T(pGroup->chat_group_name);
			
			auto *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszTitle);
			Chat_AddGroup(si, TranslateT("Owner"));
			Chat_AddGroup(si, TranslateT("Participant"));

			if (pOwner == 0) {
				for (int j = 0; j < pGroup->n_top_members; j++) {
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
					else
						wszNick = L"@" + wszUserId;
					
					gce.pszNick.w = wszNick;
					gce.pszStatus.w = (pGroup->top_members[j] == pGroup->accountid_owner) ? TranslateT("Owner") : TranslateT("Participant");
					Chat_Event(&gce);
				}
			}
			else si->pParent = pOwner;

			setDword(si->hContact, "ChatId", pChat->chat_id);

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

			uint32_t dwLastMsgId = getDword(si->hContact, DBKEY_LASTMSG);
			if (pChat->time_last_message > dwLastMsgId)
				SendGetChatHistory(si->hContact, dwLastMsgId);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::SendGetChatHistory(MCONTACT hContact, uint32_t iLastMsgId)
{
	CChatRoomGetMessageHistoryRequest request;
	request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
	request.chat_id = getDword(hContact, "ChatId"); request.has_chat_id = true;
	request.last_time = iLastMsgId;  request.has_last_time = true;
	WSSendService(GetChatHistory, request, (void*)hContact);
}

void CSteamProto::OnGetChatHistory(const CChatRoomGetMessageHistoryResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	if (auto *si = Chat_Find(UINT_PTR(GetRequestInfo(hdr.jobid_target)), m_szModuleName)) {
		uint32_t iLastMsg = getDword(si->hContact, DBKEY_LASTMSG);
		uint32_t iChatId = getDword(si->hContact, "ChatId");

		for (int i = (int)reply.n_messages - 1; i >= 0; i--) {
			auto *pMsg = reply.messages[i];
			if (pMsg->server_timestamp > iLastMsg)
				iLastMsg = pMsg->server_timestamp;

			// some slack, skip it
			if (pMsg->server_message)
				continue;

			char szMsgId[100], szUserId[100];
			mir_snprintf(szMsgId, "%d_%d", iChatId, pMsg->server_timestamp);
			_i64toa(AccountIdToSteamId(pMsg->sender), szUserId, 10);

			DB::EventInfo dbei(db_event_getById(m_szModuleName, szMsgId));
			dbei.flags |= DBEF_UTF;
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.szModule = m_szModuleName;
			replaceStr(dbei.pBlob, mir_strdup(pMsg->message));
			dbei.cbBlob = (int)mir_strlen(dbei.pBlob);
			dbei.timestamp = pMsg->server_timestamp;
			dbei.szId = szMsgId;
			dbei.szUserId = szUserId;

			if (dbei.getEvent())
				db_event_edit(dbei.getEvent(), &dbei, true);
			else
				db_event_add(si->hContact, &dbei);
		}

		setDword(si->hContact, DBKEY_LASTMSG, iLastMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::OnGetChatMessage(const CChatRoomIncomingChatMessageNotification &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	CMStringW wszId(FORMAT, L"%lld_%lld", reply.chat_group_id, reply.chat_id);
	if (auto *si = Chat_Find(wszId, m_szModuleName)) {
		char szMsgId[100], szUserId[100];
		mir_snprintf(szMsgId, "%lld_%d", reply.chat_id, reply.timestamp);
		_i64toa(reply.steamid_sender, szUserId, 10);

		DB::EventInfo dbei(db_event_getById(m_szModuleName, szMsgId));
		dbei.flags |= DBEF_UTF;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		replaceStr(dbei.pBlob, mir_strdup(reply.message));
		dbei.cbBlob = (int)mir_strlen(dbei.pBlob);
		dbei.timestamp = reply.timestamp;
		dbei.szId = szMsgId;
		dbei.szUserId = szUserId;

		if (dbei.getEvent())
			db_event_edit(dbei.getEvent(), &dbei, true);
		else
			db_event_add(si->hContact, &dbei);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CSteamProto::SvcLeaveChat(WPARAM hContact, LPARAM)
{
	CChatRoomLeaveChatRoomGroupRequest request;
	request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
	WSSendService(LeaveChatGroup, request, new uint64_t(request.chat_group_id));
	return 0;
}

void CSteamProto::OnLeftChat(const CChatRoomLeaveChatRoomGroupResponse&, const CMsgProtoBufHeader &hdr)
{
	if (auto *pGroupId = (int64_t *)GetRequestInfo(hdr.jobid_target)) {
		for (auto &cc : AccContacts()) {
			if (!Contact::IsGroupChat(cc) || GetId(cc, DBKEY_STEAM_ID) != *pGroupId)
				continue;

			CMStringW wszId(FORMAT, L"%lld_%lld", *pGroupId, GetId(cc, "ChatId"));
			if (auto *si = Chat_Find(wszId, m_szModuleName))
				Chat_Terminate(si);

			db_delete_contact(cc);
		}

		delete pGroupId;
	}
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

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);
			T2Utf szMessage(gch->ptszText);

			CChatRoomSendChatMessageRequest request;
			request.chat_group_id = _wtoi64(gch->si->ptszID); request.has_chat_group_id = true;
			request.chat_id = getDword(gch->si->hContact, "ChatId"); request.has_chat_id = true;
			request.echo_to_sender = request.has_echo_to_sender = true;
			request.message = szMessage;
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
