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

static const wchar_t* getRoleById(uint32_t ID)
{
	switch (ID) {
	case TD::chatMemberStatusCreator::ID:
		return TranslateT("Creator");

	case TD::chatMemberStatusAdministrator::ID:
		return TranslateT("Admin");

	case TD::chatMemberStatusMember::ID:
	default:
		return TranslateT("Participant");
	}
}

void CTelegramProto::InitGroupChat(TG_USER *pUser, const wchar_t *pwszTitle)
{
	if (pUser->m_si)
		return;
		
	wchar_t wszId[100];
	_i64tow(pUser->id, wszId, 10);

	SESSION_INFO *si;
	if (pwszTitle == nullptr)
		pwszTitle = pUser->wszNick;

	if (pUser->bLoadMembers) {
		pUser->m_si = si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, pwszTitle, pUser);
		if (!si->arStatuses.getCount()) {
			Chat_AddGroup(si, TranslateT("Creator"));
			Chat_AddGroup(si, TranslateT("Admin"));
			Chat_AddGroup(si, TranslateT("Participant"));

			// push async query to fetch users
			if (m_arBasicGroups.find((TG_BASIC_GROUP*)&pUser->id))
				SendQuery(new TD::getBasicGroupFullInfo(pUser->id), &CTelegramProto::StartGroupChat, pUser);
			else
				SendQuery(new TD::getSupergroupMembers(pUser->id, 0, 0, 100), &CTelegramProto::StartGroupChat, pUser);
		}
		else GcRun(pUser);
	}
	else {
		pUser->m_si = si = Chat_NewSession(GCW_CHANNEL, m_szModuleName, wszId, pwszTitle, pUser);
		if (!si->arStatuses.getCount()) {
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
			gce.pszNick.w = pwszTitle;
			gce.pszStatus.w = TranslateT("SuperAdmin");
			Chat_Event(&gce);
		}

		GcRun(pUser);
	}
}

void CTelegramProto::GcRun(TG_USER *pChat)
{
	pChat->bStartChat = false;
	Chat_Control(pChat->m_si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(pChat->m_si, SESSION_ONLINE);
}

void CTelegramProto::StartGroupChat(td::ClientManager::Response &response, void *pUserData)
{
	if (!response.object)
		return;

	auto *pChat = (TG_USER *)pUserData;

	switch (response.object->get_id()) {
	case TD::basicGroupFullInfo::ID:
		GcAddMembers(pChat, ((TD::basicGroupFullInfo *)response.object.get())->members_, false);
		break;

	case TD::chatMembers::ID:
		GcAddMembers(pChat, ((TD::chatMembers *)response.object.get())->members_, false);
		break;

	default:
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::basicGroupFullInfo::ID);
		return;
	}

	if (pChat->bStartChat)
		GcRun(pChat);
}

void CTelegramProto::GcAddMembers(TG_USER *pChat, const TD::array<TD::object_ptr<TD::chatMember>> &pMembers, bool bSilent)
{
	for (auto &it : pMembers) {
		auto *pMember = it.get();
		if (pMember->member_id_->get_id() != TD::messageSenderUser::ID)
			continue;

		int64_t memberId = ((TD::messageSenderUser *)pMember->member_id_.get())->user_id_;
		auto *pChatUser = FindUser(memberId);
		if (pChatUser == nullptr)
			continue;

		wchar_t wszUserId[100];
		_i64tow(memberId, wszUserId, 10);

		GCEVENT gce = { pChat->m_si, GC_EVENT_JOIN };
		gce.pszUID.w = wszUserId;
		gce.pszStatus.w = getRoleById(pMember->status_->get_id());
		if (bSilent)
			gce.dwFlags = GCEF_SILENT;

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

		if (*gce.pszNick.w == '@')
			gce.pszNick.w++;

		Chat_Event(&gce);
	}

	Chat_Control(pChat->m_si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(pChat->m_si, SESSION_ONLINE);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CTelegramProto::GcMuteHook(WPARAM hContact, LPARAM mode)
{
	if (Proto_IsProtoOnContact(hContact, m_szModuleName)) {
		if (auto *pUser = FindUser(GetId(hContact))) {
			auto settings = TD::make_object<TD::chatNotificationSettings>();
			memcpy(settings.get(), &pUser->notificationSettings, sizeof(pUser->notificationSettings));

			TD::int32 defaultMute = GetDefaultMute(pUser);
			TD::int32 newMute = (mode == CHATMODE_MUTE) ? 421689178 : 0;
			settings->use_default_mute_for_ = (newMute == defaultMute);
			settings->mute_for_ = newMute;
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

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);
			if (auto *pUser = (TG_USER *)gch->si->pItemData) {
				TD::int53 replyId = 0;
				if (auto *pDlg = gch->si->pDlg) {
					DB::EventInfo dbei(pDlg->m_hQuoteEvent, false);
					if (dbei)
						replyId = dbei2id(dbei);
				}
				SendTextMessage(pUser->chatId, GetId(gch->si->hContact, DBKEY_THREAD), replyId, T2Utf(gch->ptszText));
			}
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

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnLeaveChat(td::ClientManager::Response &, void *pUserInfo)
{
	auto *pUser = (TG_USER *)pUserInfo;

	wchar_t wszId[100];
	_i64tow(pUser->id, wszId, 10);
	if (auto *si = Chat_Find(wszId, m_szModuleName))
		Chat_Terminate(si);

	db_delete_contact(pUser->hContact);
}

INT_PTR CTelegramProto::SvcLeaveChat(WPARAM hContact, LPARAM)
{
	int64_t id = GetId(hContact);
	if (auto *pUser = FindUser(id)) {
		pUser->m_si = nullptr;
		SendQuery(new TD::leaveChat(pUser->chatId), &CTelegramProto::OnLeaveChat, pUser);
	}
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
	else hContact = GetRealContact(pUser);

	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::GcChangeMember(TG_USER *pChat, const char *adminId, TD::int53 userId, bool bJoined)
{
	if (pChat->m_si == nullptr)
		return;

	if (auto *pMember = FindUser(userId)) {
		CMStringW wszId(FORMAT, L"%lld", pMember->id), wszNick(pMember->getDisplayName());
		Utf2T wszAdminId(adminId);

		GCEVENT gce = { pChat->m_si, (bJoined) ? GC_EVENT_JOIN : GC_EVENT_PART };
		gce.pszUID.w = wszId;
		gce.pszNick.w = wszNick;
		gce.bIsMe = false;
		gce.time = time(0);
		gce.pszStatus.w = TranslateT("Participant");
		gce.pszText.w = wszAdminId;
		Chat_Event(&gce);
	}
}

void CTelegramProto::GcChangeTopic(TG_USER *pChat, const std::string &szNewTopic)
{
	if (pChat->m_si == nullptr)
		return;

	Utf2T wszTopic(szNewTopic.c_str());
	GCEVENT gce = { pChat->m_si, GC_EVENT_TOPIC };
	gce.pszText.w = wszTopic;
	gce.time = time(0);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Server data

void CTelegramProto::ProcessBasicGroup(TD::updateBasicGroup *pObj)
{
	auto *pBasicGroup = pObj->basic_group_.get();

	auto iStatusId = pBasicGroup->status_->get_id();
	if (iStatusId == TD::chatMemberStatusBanned::ID) {
		if (pBasicGroup->upgraded_to_supergroup_id_) {
			auto *pUser = FindUser(pBasicGroup->upgraded_to_supergroup_id_);
			if (pUser) {
				pUser->bLoadMembers = true;
				if (pUser->m_si)
					pUser->m_si->bHasNicklist = true;

				if (auto *pOldUser = FindUser(pBasicGroup->id_)) {
					pUser->hContact = pOldUser->hContact;
					SetId(pUser->hContact, pBasicGroup->upgraded_to_supergroup_id_);
				}
			}
		}

		debugLogA("We are banned here, skipping");
		return;
	}

	TG_BASIC_GROUP tmp(pBasicGroup->id_, 0);
	auto *pGroup = m_arBasicGroups.find(&tmp);
	if (pGroup == nullptr) {
		pGroup = new TG_BASIC_GROUP(tmp.id, std::move(pObj->basic_group_));
		m_arBasicGroups.insert(pGroup);
	}
	else pGroup->group = std::move(pObj->basic_group_);

	TG_USER *pUser;
	if (iStatusId == TD::chatMemberStatusLeft::ID) {
		pUser = AddFakeUser(tmp.id, true);
		pUser->wszLastName.Format(TranslateT("%d member(s)"), pGroup->group->member_count_);
	}
	else pUser = AddUser(tmp.id, true);

	pUser->bLoadMembers = true;
}

void CTelegramProto::ProcessBasicGroupInfo(TG_USER *pChat, TD::basicGroupFullInfo *pInfo)
{
	if (!pInfo->description_.empty()) {
		setUString(pChat->hContact, "About", pInfo->description_.c_str());
		GcChangeTopic(pChat, pInfo->description_);
	}

	g_chatApi.UM_RemoveAll(pChat->m_si);
	GcAddMembers(pChat, pInfo->members_, true);
}

void CTelegramProto::ProcessSuperGroupInfo(TG_USER *pUser, TD::supergroupFullInfo *pInfo)
{
	setDword(pUser->hContact, "MemberCount", pInfo->member_count_);

	if (auto *pLink = pInfo->invite_link_.get())
		setUString(pUser->hContact, "Link", pLink->invite_link_.c_str());
	else if (auto *pGroup = FindSuperGroup(pUser->id)) {
		if (pGroup->group->usernames_) {
			CMStringA szLink(FORMAT, "https://t.me/%s", pGroup->group->usernames_->editable_username_.c_str());
			setString(pUser->hContact, "Link", szLink);
		}
		else delSetting(pUser->hContact, "Link");
	}
	else delSetting(pUser->hContact, "Link");

	if (!pInfo->description_.empty()) {
		setUString(pUser->hContact, "About", pInfo->description_.c_str());
		GcChangeTopic(pUser, pInfo->description_);
	}
}

void CTelegramProto::ProcessSuperGroup(TD::updateSupergroup *pObj)
{
	auto iStatusId = pObj->supergroup_->status_->get_id();
	if (iStatusId == TD::chatMemberStatusBanned::ID) {
		debugLogA("We are banned here, skipping");
		return;
	}

	auto id = pObj->supergroup_->id_;
	auto *pGroup = FindSuperGroup(id);
	if (pGroup == nullptr) {
		pGroup = new TG_SUPER_GROUP(id, std::move(pObj->supergroup_));
		m_arSuperGroups.insert(pGroup);
	}
	else pGroup->group = std::move(pObj->supergroup_);

	if (iStatusId == TD::chatMemberStatusLeft::ID) {
		auto *pUser = AddFakeUser(id, true);
		pUser->isForum = pGroup->group->is_forum_;
		if (pUser->hContact == INVALID_CONTACT_ID) {
			// cache some information for the search
			if (pUser->wszNick.IsEmpty())
				pUser->wszNick = Utf2T(getName(pGroup->group->usernames_.get()));
			pUser->wszLastName.Format(TranslateT("%d member(s)"), pGroup->group->member_count_);
		}
		else RemoveFromClist(pUser);
	}
	else {
		auto *pChat = AddUser(id, true);
		pChat->isForum = pGroup->group->is_forum_;
		if (!pGroup->group->is_channel_)
			pChat->bLoadMembers = true;

		if (!Contact::OnList(pChat->hContact))
			Contact::PutOnList(pChat->hContact);

		if (pChat->bStartChat)
			InitGroupChat(pChat, pChat->wszNick);

		if (pChat->m_si) {
			CMStringW wszUserId(FORMAT, L"%lld", m_iOwnId);

			GCEVENT gce = { pChat->m_si, GC_EVENT_SETSTATUS };
			gce.pszUID.w = wszUserId;
			gce.time = time(0);
			gce.bIsMe = true;
			gce.pszStatus.w = getRoleById(iStatusId);
			gce.pszText.w = TranslateT("Admin");
			Chat_Event(&gce);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// forums

void CTelegramProto::OnGetTopics(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::forumTopics::ID)
		return;

	auto *pUser = (TG_USER *)pUserInfo;

	auto *pInfo = (TD::forumTopics *)response.object.get();
	if (pInfo->topics_.size() >= 100)
		SendQuery(new TD::getForumTopics(pUser->chatId, "", pInfo->next_offset_date_, pInfo->next_offset_message_id_, pInfo->next_offset_forum_topic_id_, 100),
			&CTelegramProto::OnGetTopics, pUser);
}

void CTelegramProto::ProcessForum(TD::updateForumTopicInfo *pForum)
{
	auto *pInfo = pForum->info_.get();
	auto *pUser = FindChat(pInfo->chat_id_);
	if (!pUser) {
		debugLogA("Uknown chat id %lld, skipping", pInfo->chat_id_);
		return;
	}

	if (pUser->m_si == nullptr) {
		debugLogA("No parent chat for id %lld, skipping", pInfo->chat_id_);
		return;
	}

	if (pInfo->is_general_) {
		SetId(pUser->m_si->hContact, pForum->info_->forum_topic_id_, DBKEY_THREAD);
		return;
	}

	wchar_t wszId[100];
	mir_snwprintf(wszId, L"%lld_%lld", pInfo->chat_id_, pForum->info_->forum_topic_id_);

	auto *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, Utf2T(pForum->info_->name_.c_str()), pUser);
	si->pParent = pUser->m_si;

	SetId(si->hContact, pForum->info_->forum_topic_id_, DBKEY_THREAD);
	SetId(si->hContact, pUser->id, DBKEY_OWNER);

	Chat_Mute(si->hContact, Chat_IsMuted(pUser->hContact));
	Clist_SetGroup(si->hContact, ptrW(Clist_GetGroup(pUser->hContact)));

	Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(si, SESSION_ONLINE);
}
