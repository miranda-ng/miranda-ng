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

void CTelegramProto::InitGroupChat(TG_USER *pUser, const TD::chat *pChat, bool bUpdateMembers)
{
	if (pUser->m_si)
		return;
		
	wchar_t wszId[100];
	_i64tow(pUser->id, wszId, 10);
	SESSION_INFO *si;

	if (bUpdateMembers) {
		si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, Utf2T(pChat->title_.c_str()), pUser);
		Chat_AddGroup(si, TranslateT("Creator"));
		Chat_AddGroup(si, TranslateT("Admin"));
		Chat_AddGroup(si, TranslateT("Participant"));

		// push async query to fetch users
		SendQuery(new TD::getBasicGroupFullInfo(pUser->id), &CTelegramProto::StartGroupChat, pUser);
	}
	else {
		si = Chat_NewSession(GCW_CHANNEL, m_szModuleName, wszId, Utf2T(pChat->title_.c_str()), pUser);
		Chat_AddGroup(si, TranslateT("SuperAdmin"));
		Chat_AddGroup(si, TranslateT("Visitor"));

		ptrW wszUserId(getWStringA(DBKEY_ID)), wszNick(Contact::GetInfo(CNF_DISPLAY, 0, m_szModuleName));

		GCEVENT gce = { si, GC_EVENT_JOIN };
		gce.pszUID.w = wszUserId;
		gce.pszNick.w = wszNick;
		gce.bIsMe = true;
		gce.pszStatus.w = TranslateT("Visitor");
		Chat_Event(&gce);

		gce.bIsMe = false;
		gce.pszUID.w = L"---";
		gce.pszNick.w = TranslateT("Admin");
		gce.pszStatus.w = TranslateT("SuperAdmin");
		Chat_Event(&gce);

		Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
		Chat_Control(si, SESSION_ONLINE);
	}
	
	pUser->m_si = si;
}

void CTelegramProto::StartGroupChat(td::ClientManager::Response &response, void *pUserData)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::basicGroupFullInfo::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::basicGroupFullInfo::ID);
		return;
	}

	auto *pInfo = ((TD::basicGroupFullInfo *)response.object.get());
	auto *pUser = (TG_USER *)pUserData;

	for (auto &it : pInfo->members_) {
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
			SendTextMessage(-userId, T2Utf(gch->ptszText));
		}
		break;

	case GC_USER_PRIVMESS:
		Chat_SendPrivateMessage(gch);
		break;

	case GC_USER_LOGMENU:
		break;

	case GC_USER_NICKLISTMENU:
		break;
	}

	return 1;
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

int CTelegramProto::GcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS *)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	auto *pUser = FindUser(T2Utf(gcmi->pszID));
	if (pUser == nullptr)
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
	}
	return 0;
}
