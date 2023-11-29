/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

void CTelegramProto::InitGroupChat(TG_USER *pUser, const TD::chat *pChat)
{
	if (pUser->m_si)
		return;
		
	wchar_t wszId[100];
	_i64tow(pUser->id, wszId, 10);

	SESSION_INFO *si;
	Utf2T wszNick(pChat->title_.c_str());

	if (pUser->bLoadMembers) {
		si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszNick, pUser);
		if (!si->pStatuses) {
			Chat_AddGroup(si, TranslateT("Creator"));
			Chat_AddGroup(si, TranslateT("Admin"));
			Chat_AddGroup(si, TranslateT("Participant"));

			// push async query to fetch users
			if (m_arBasicGroups.find((TG_BASIC_GROUP*)&pUser->id))
				SendQuery(new TD::getBasicGroupFullInfo(pUser->id), &CTelegramProto::StartGroupChat, pUser);
			else
				SendQuery(new TD::getSupergroupMembers(pUser->id, 0, 0, 100), &CTelegramProto::StartGroupChat, pUser);
		}
		else {
			Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
			Chat_Control(si, SESSION_ONLINE);
		}
	}
	else {
		if (si = Chat_NewSession(GCW_CHANNEL, m_szModuleName, wszId, wszNick, pUser)) {
			if (!si->pStatuses) {
				Chat_AddGroup(si, TranslateT("SuperAdmin"));
				Chat_AddGroup(si, TranslateT("Visitor"));

				ptrW wszMyId(getWStringA(DBKEY_ID)), wszMyNick(Contact::GetInfo(CNF_DISPLAY, 0, m_szModuleName));

				GCEVENT gce = { si, GC_EVENT_JOIN };
				gce.pszUID.w = wszMyId;
				gce.pszNick.w = wszMyNick;
				gce.bIsMe = true;
				gce.pszStatus.w = TranslateT("Visitor");
				Chat_Event(&gce);

				gce.bIsMe = false;
				gce.pszUID.w = wszId;
				gce.pszNick.w = wszNick;
				gce.pszStatus.w = TranslateT("SuperAdmin");
				Chat_Event(&gce);
			}

			Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
			Chat_Control(si, SESSION_ONLINE);
		}
	}
	
	pUser->m_si = si;
}

void CTelegramProto::StartGroupChat(td::ClientManager::Response &response, void *pUserData)
{
	if (!response.object)
		return;

	TD::array<TD::object_ptr<TD::chatMember>> *pMembers;

	switch (response.object->get_id()) {
	case TD::basicGroupFullInfo::ID:
		pMembers = &((TD::basicGroupFullInfo *)response.object.get())->members_;
		break;

	case TD::chatMembers::ID:
		pMembers = &((TD::chatMembers *)response.object.get())->members_;
		break;

	default:
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::basicGroupFullInfo::ID);
		return;
	}

	auto *pUser = (TG_USER *)pUserData;

	for (auto &it : *pMembers) {
		auto *pMember = it.get();
		const wchar_t *pwszRole;

		switch (pMember->status_->get_id()) {
		case TD::chatMemberStatusCreator::ID:
			pwszRole = TranslateT("Creator");
			break;
		case TD::chatMemberStatusAdministrator::ID:
			pwszRole = TranslateT("Admin");
			break;
		case TD::chatMemberStatusMember::ID:
		default:
			pwszRole = TranslateT("Participant");
			break;
		}

		if (pMember->member_id_->get_id() != TD::messageSenderUser::ID)
			continue;

		int64_t memberId = ((TD::messageSenderUser *)pMember->member_id_.get())->user_id_;
		auto *pChatUser = FindUser(memberId);
		if (pChatUser == nullptr)
			continue;

		wchar_t wszUserId[100];
		_i64tow(memberId, wszUserId, 10);

		GCEVENT gce = { pUser->m_si, GC_EVENT_JOIN };
		gce.pszUID.w = wszUserId;
		gce.pszStatus.w = pwszRole;

		switch (pChatUser->hContact) {
		case 0:
			gce.bIsMe = true;
			__fallthrough;

		case INVALID_CONTACT_ID:
			gce.pszNick.w = pChatUser->wszNick.c_str();
			break;

		default:
			gce.pszNick.w = Clist_GetContactDisplayName(pChatUser->hContact);
			break;
		}

		Chat_Event(&gce);
	}

	Chat_Control(pUser->m_si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(pUser->m_si, SESSION_ONLINE);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTelegramProto::GcMuteHook(WPARAM hContact, LPARAM mode)
{
	if (Proto_IsProtoOnContact(hContact, m_szModuleName)) {
		if (auto *pUser = FindUser(GetId(hContact))) {
			auto settings = TD::make_object<TD::chatNotificationSettings>();
			memcpy(settings.get(), &pUser->notificationSettings, sizeof(pUser->notificationSettings));

			switch (mode) {
			case CHATMODE_MUTE:
				settings->use_default_mute_for_ = false;
				settings->mute_for_ = 45000000;
				break;

			default:
				settings->use_default_mute_for_ = true;
				settings->mute_for_ = 0;
				break;
			}
			SendQuery(new TD::setChatNotificationSettings(pUser->chatId, std::move(settings)));
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

enum
{
	IDM_LEAVE = 1,
};

int CTelegramProto::GcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	auto userId = _wtoi64(gch->si->ptszID);

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);
			Chat_UnescapeTags(gch->ptszText);
			if (auto *pUser = FindUser(userId))
				SendTextMessage(pUser->chatId, T2Utf(gch->ptszText));
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

	return 1;
}

void CTelegramProto::Chat_LogMenu(GCHOOK *gch)
{
	switch (gch->dwData) {
	case IDM_LEAVE:
		SvcLeaveChat(gch->si->hContact, 0);
		break;
	}
}

INT_PTR CTelegramProto::SvcLeaveChat(WPARAM hContact, LPARAM)
{
	int64_t id = GetId(hContact);
	if (auto *pUser = FindUser(id)) {
		pUser->m_si = nullptr;
		SendQuery(new TD::leaveChat(pUser->chatId));
	}

	wchar_t wszId[100];
	_i64tow(id, wszId, 10);
	if (auto *si = Chat_Find(wszId, m_szModuleName))
		Chat_Terminate(si);

	db_delete_contact(hContact);
	return 0;
}

void CTelegramProto::Chat_SendPrivateMessage(GCHOOK *gch)
{
	MCONTACT hContact;
	TD::int53 userId = _wtoi64(gch->ptszUID);
	auto *pUser = FindUser(userId);
	if (pUser == nullptr || pUser->hContact == INVALID_CONTACT_ID) {
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
	else hContact = pUser->hContact;

	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::GcChangeMember(TG_USER *pChat, TD::int53 userId, bool bJoined)
{
	if (pChat->m_si == nullptr)
		return;

	if (auto *pMember = FindUser(userId)) {
		CMStringW wszId(FORMAT, L"%lld", pMember->id), wszNick(pMember->getDisplayName());

		GCEVENT gce = { pChat->m_si, (bJoined) ? GC_EVENT_JOIN : GC_EVENT_PART };
		gce.pszUID.w = wszId;
		gce.pszNick.w = wszNick;
		gce.bIsMe = false;
		gce.pszStatus.w = TranslateT("Visitor");
		Chat_Event(&gce);
	}
}

void CTelegramProto::GcChangeTopic(TG_USER *pChat, const wchar_t *pwszNewTopic)
{
	if (pChat->m_si == nullptr || pwszNewTopic == nullptr)
		return;

	GCEVENT gce = { pChat->m_si, GC_EVENT_TOPIC };
	gce.pszText.w = pwszNewTopic;
	Chat_Event(&gce);
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENW("&Leave chat session"), IDM_LEAVE, MENU_ITEM }
};

int CTelegramProto::GcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS *)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	auto *pUser = FindUser(_wtoi64(gcmi->pszID));
	if (pUser == nullptr)
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
	}
	return 0;
}
