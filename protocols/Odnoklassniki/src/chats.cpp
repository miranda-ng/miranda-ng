/*
Copyright (C) 2026 Miranda NG team

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

// Existing OK group chats are Miranda chat rooms: one per conversation of type CHAT, keyed by
// its conversation id, with the participants in the nick list. New group chats are not created
// from Miranda.

#include "stdafx.h"

SESSION_INFO* COdnoklassnikiProto::StartChat(const OkConversation &cnv)
{
	Utf2T wszId(cnv.id);
	CMStringW wszTitle(cnv.topic);
	if (wszTitle.IsEmpty())
		wszTitle = TranslateT("Group chat");

	bool bNew = false;
	SESSION_INFO *si = Chat_Find(wszId, m_szModuleName);
	if (si == nullptr) {
		si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszTitle);
		if (si == nullptr)
			return nullptr;

		setString(si->hContact, DB_KEY_CONV, cnv.id);
		Chat_AddGroup(si, TranslateT("Participants"));
		bNew = true;
	}
	else if (wszTitle != si->ptszName)
		Chat_ChangeSessionName(si, wszTitle);

	for (auto &uid : cnv.participants) {
		Utf2T wszUid(uid);
		bool bKnown = false;
		for (auto &U : si->getUserList())
			if (!mir_wstrcmp(U->pszUID, wszUid)) {
				bKnown = true;
				break;
			}
		if (bKnown)
			continue;

		CMStringW wszNick(uid == m_szMyUid ? TranslateT("Me") : GetUserName(uid));
		if (wszNick.IsEmpty())
			wszNick = wszUid;

		GCEVENT gce = { si, GC_EVENT_JOIN };
		gce.pszUID.w = wszUid;
		gce.pszNick.w = wszNick;
		gce.pszStatus.w = TranslateT("Participants");
		gce.bIsMe = (uid == m_szMyUid);
		Chat_Event(&gce);   // no time: the nick list is updated without a "joined" line
	}

	// OK group chats are small personal ones, so by default every message blinks the tray and
	// the contact list and shows a popup, not only highlights (Miranda's global chat default).
	// It is written once as the room's own setting - what the user later picks in the room's
	// filter dialog is kept
	if (!getByte(si->hContact, "NotifyDefaults")) {
		setByte(si->hContact, "NotifyDefaults", 1);
		int iFlags = GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT;
		db_set_b(si->hContact, CHAT_MODULE, "FilterEnabled", 1);
		db_set_dw(si->hContact, CHAT_MODULE, "TrayIconMask", db_get_dw(si->hContact, CHAT_MODULE, "TrayIconMask") | iFlags);
		db_set_dw(si->hContact, CHAT_MODULE, "TrayIconFlags", db_get_dw(si->hContact, CHAT_MODULE, "TrayIconFlags") | iFlags);
		db_set_dw(si->hContact, CHAT_MODULE, "PopupMask", db_get_dw(si->hContact, CHAT_MODULE, "PopupMask") | iFlags);
		db_set_dw(si->hContact, CHAT_MODULE, "PopupFlags", db_get_dw(si->hContact, CHAT_MODULE, "PopupFlags") | iFlags);
		bNew = true;
	}

	// Done once: SESSION_INITDONE would call ShowRoom, which brings the room's tab to the
	// front on every poll. WINDOW_HIDDEN finishes the set-up and leaves opening it to the user
	if (bNew) {
		Chat_SetFilters(si);
		Chat_Control(si, WINDOW_HIDDEN);
	}

	if (m_onlineChats.insert(cnv.id).second)
		Chat_Control(si, SESSION_ONLINE);
	return si;
}

// what the user types into a chat room window
int COdnoklassnikiProto::GcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	if (gch == nullptr || gch->si == nullptr || mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	if (gch->iType == GC_USER_MESSAGE && mir_wstrlen(gch->ptszText) > 0) {
		CMStringW wszText(gch->ptszText);
		wszText.TrimRight();
		if (!wszText.IsEmpty())
			SendMsg(gch->si->hContact, 0, T2Utf(wszText));
	}
	return 1;
}
