/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-17 Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_iq.h"
#include "jabber_caps.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Global definitions

enum {
	IDM_CANCEL,

	IDM_ROLE, IDM_AFFLTN,

	IDM_CONFIG, IDM_NICK, IDM_DESTROY, IDM_INVITE, IDM_BOOKMARKS, IDM_LEAVE, IDM_TOPIC,
	IDM_LST_PARTICIPANT, IDM_LST_MODERATOR,
	IDM_LST_MEMBER, IDM_LST_ADMIN, IDM_LST_OWNER, IDM_LST_BAN,

	IDM_MESSAGE, IDM_SLAP, IDM_VCARD, IDM_INFO, IDM_KICK,
	IDM_RJID, IDM_RJID_ADD, IDM_RJID_VCARD, IDM_RJID_COPY,
	IDM_SET_VISITOR, IDM_SET_PARTICIPANT, IDM_SET_MODERATOR,
	IDM_SET_NONE, IDM_SET_MEMBER, IDM_SET_ADMIN, IDM_SET_OWNER, IDM_SET_BAN,
	IDM_CPY_NICK, IDM_CPY_TOPIC, IDM_CPY_RJID, IDM_CPY_INROOMJID,

	IDM_LINK0, IDM_LINK1, IDM_LINK2, IDM_LINK3, IDM_LINK4, IDM_LINK5, IDM_LINK6, IDM_LINK7, IDM_LINK8, IDM_LINK9,

	IDM_PRESENCE_ONLINE = ID_STATUS_ONLINE,
	IDM_PRESENCE_AWAY = ID_STATUS_AWAY,
	IDM_PRESENCE_NA = ID_STATUS_NA,
	IDM_PRESENCE_DND = ID_STATUS_DND,
	IDM_PRESENCE_FREE4CHAT = ID_STATUS_FREECHAT,
};

struct TRoleOrAffiliationInfo
{
	int value;
	int id;
	wchar_t *title_en;
	int min_role;
	int min_affiliation;

	wchar_t *title;

	BOOL check(JABBER_RESOURCE_STATUS *me, JABBER_RESOURCE_STATUS *him)
	{
		if (me->m_affiliation == AFFILIATION_OWNER) return TRUE;
		if (me == him) return FALSE;
		if (me->m_affiliation <= him->m_affiliation) return FALSE;
		if (me->m_role < this->min_role) return FALSE;
		if (me->m_affiliation < this->min_affiliation) return FALSE;
		return TRUE;
	}
	void translate()
	{
		this->title = TranslateW(this->title_en);
	}
};

static TRoleOrAffiliationInfo sttAffiliationItems[] =
{
	{ AFFILIATION_NONE,   IDM_SET_NONE,   LPGENW("None"),   ROLE_NONE, AFFILIATION_ADMIN },
	{ AFFILIATION_MEMBER, IDM_SET_MEMBER, LPGENW("Member"), ROLE_NONE, AFFILIATION_ADMIN },
	{ AFFILIATION_ADMIN,  IDM_SET_ADMIN,  LPGENW("Admin"),  ROLE_NONE, AFFILIATION_OWNER },
	{ AFFILIATION_OWNER,  IDM_SET_OWNER,  LPGENW("Owner"),  ROLE_NONE, AFFILIATION_OWNER },
};

static TRoleOrAffiliationInfo sttRoleItems[] =
{
	{ ROLE_VISITOR,     IDM_SET_VISITOR,     LPGENW("Visitor"),     ROLE_MODERATOR, AFFILIATION_NONE  },
	{ ROLE_PARTICIPANT, IDM_SET_PARTICIPANT, LPGENW("Participant"), ROLE_MODERATOR, AFFILIATION_NONE  },
	{ ROLE_MODERATOR,   IDM_SET_MODERATOR,   LPGENW("Moderator"),   ROLE_MODERATOR, AFFILIATION_ADMIN },
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGcInit - initializes the new chat

static const wchar_t *sttStatuses[] = { LPGENW("Visitors"), LPGENW("Participants"), LPGENW("Moderators"), LPGENW("Owners") };

int JabberGcGetStatus(JABBER_GC_AFFILIATION a, JABBER_GC_ROLE r)
{
	if (a == AFFILIATION_OWNER)
		return 3;

	switch (r) {
		case ROLE_MODERATOR:   return 2;
		case ROLE_PARTICIPANT: return 1;
	}
	return 0;
}

int JabberGcGetStatus(JABBER_RESOURCE_STATUS *r)
{
	return JabberGcGetStatus(r->m_affiliation, r->m_role);
}

int CJabberProto::GcInit(JABBER_LIST_ITEM *item)
{
	if (item->bChatActive)
		return 1;

	// translate string for menus (this can't be done in initializer)
	for (int i = 0; i < _countof(sttAffiliationItems); i++)
		sttAffiliationItems[i].translate();
	for (int i = 0; i < _countof(sttRoleItems); i++)
		sttRoleItems[i].translate();

	ptrW szNick(JabberNickFromJID(item->jid));
	GCSessionInfoBase *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, item->jid, szNick);
	if (si != nullptr) {
		item->hContact = si->hContact;

		if (JABBER_LIST_ITEM *bookmark = ListGetItemPtr(LIST_BOOKMARK, item->jid)) {
			if (bookmark->name) {
				ptrW myHandle(db_get_wsa(si->hContact, "CList", "MyHandle"));
				if (myHandle == nullptr)
					db_set_ws(si->hContact, "CList", "MyHandle", bookmark->name);
			}
		}

		ptrW tszNick(getWStringA(si->hContact, "MyNick"));
		if (tszNick != nullptr) {
			if (!mir_wstrcmp(tszNick, szNick))
				delSetting(si->hContact, "MyNick");
			else
				setWString(si->hContact, "MyNick", item->nick);
		}
		else setWString(si->hContact, "MyNick", item->nick);

		ptrW passw(getWStringA(si->hContact, "Password"));
		if (lstrcmp_null(passw, item->password)) {
			if (!item->password || !item->password[0])
				delSetting(si->hContact, "Password");
			else
				setWString(si->hContact, "Password", item->password);
		}
	}

	item->bChatActive = true;

	for (int i = _countof(sttStatuses) - 1; i >= 0; i--)
		Chat_AddGroup(m_szModuleName, item->jid, TranslateW(sttStatuses[i]));

	Chat_Control(m_szModuleName, item->jid, (item->bAutoJoin && m_options.AutoJoinHidden) ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, item->jid, SESSION_ONLINE);
	return 0;
}

void CJabberProto::GcLogShowInformation(JABBER_LIST_ITEM *item, pResourceStatus &user, TJabberGcLogInfoType type)
{
	if (!item || !user || (item->bChatActive != 2)) return;

	CMStringW buf;

	switch (type) {
	case INFO_BAN:
		if (m_options.GcLogBans)
			buf.Format(TranslateT("User %s is now banned."), user->m_tszResourceName);
		break;

	case INFO_STATUS:
		if (m_options.GcLogStatuses) {
			wchar_t *ptszDescr = pcli->pfnGetStatusModeDescription(user->m_iStatus, 0);
			if (user->m_tszStatusMessage)
				buf.Format(TranslateT("User %s changed status to %s with message: %s"),
				user->m_tszResourceName, ptszDescr, user->m_tszStatusMessage);
			else
				buf.Format(TranslateT("User %s changed status to %s"), user->m_tszResourceName, ptszDescr);
		}
		break;

	case INFO_CONFIG:
		if (m_options.GcLogConfig)
			buf.Format(TranslateT("Room configuration was changed."));
		break;

	case INFO_AFFILIATION:
		if (m_options.GcLogAffiliations) {
			wchar_t *name = nullptr;
			switch (user->m_affiliation) {
			case AFFILIATION_NONE:		name = TranslateT("None"); break;
			case AFFILIATION_MEMBER:	name = TranslateT("Member"); break;
			case AFFILIATION_ADMIN:		name = TranslateT("Admin"); break;
			case AFFILIATION_OWNER:		name = TranslateT("Owner"); break;
			case AFFILIATION_OUTCAST:	name = TranslateT("Outcast"); break;
			}
			if (name)
				buf.Format(TranslateT("Affiliation of %s was changed to '%s'."), user->m_tszResourceName, name);
		}
		break;

	case INFO_ROLE:
		if (m_options.GcLogRoles) {
			wchar_t *name = nullptr;
			switch (user->m_role) {
			case ROLE_NONE:			name = TranslateT("None"); break;
			case ROLE_VISITOR:		name = TranslateT("Visitor"); break;
			case ROLE_PARTICIPANT:	name = TranslateT("Participant"); break;
			case ROLE_MODERATOR:    name = TranslateT("Moderator"); break;
			}

			if (name)
				buf.Format(TranslateT("Role of %s was changed to '%s'."), user->m_tszResourceName, name);
		}
		break;
	}

	if (!buf.IsEmpty()) {
		buf.Replace(L"%", L"%%");

		GCEVENT gce = { m_szModuleName, item->jid, GC_EVENT_INFORMATION };
		gce.ptszNick = user->m_tszResourceName;
		gce.ptszUID = user->m_tszResourceName;
		gce.ptszText = buf;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.time = time(0);
		Chat_Event(&gce);
	}
}

void CJabberProto::GcLogUpdateMemberStatus(JABBER_LIST_ITEM *item, const wchar_t *resource, const wchar_t *nick, const wchar_t *jid, int action, HXML reason, int nStatusCode)
{
	int statusToSet = 0;

	const wchar_t *szReason = XmlGetText(reason);
	if (szReason == nullptr) {
		if (nStatusCode == 322)
			szReason = TranslateT("because room is now members-only");
		else if (nStatusCode == 301)
			szReason = TranslateT("user banned");
	}

	ptrW myNick(mir_wstrdup(item->nick));
	if (myNick == nullptr)
		myNick = JabberNickFromJID(m_szJabberJID);

	GCEVENT gce = { m_szModuleName, item->jid };
	gce.ptszNick = nick;
	gce.ptszUID = resource;
	if (jid != nullptr)
		gce.ptszUserInfo = jid;
	gce.ptszText = szReason;
	if (item->bChatActive == 2) {
		gce.dwFlags |= GCEF_ADDTOLOG;
		gce.time = time(0);
	}

	switch (gce.iType = action) {
	case GC_EVENT_PART:  break;
	case GC_EVENT_KICK:
		gce.ptszStatus = TranslateT("Moderator");
		break;
	
	default:
		mir_cslock lck(m_csLists);
		for (int i = 0; i < item->arResources.getCount(); i++) {
			JABBER_RESOURCE_STATUS *JS = item->arResources[i];
			if (!mir_wstrcmp(resource, JS->m_tszResourceName)) {
				if (action != GC_EVENT_JOIN) {
					switch (action) {
					case 0:
						gce.iType = GC_EVENT_ADDSTATUS;
					case GC_EVENT_REMOVESTATUS:
						gce.dwFlags &= ~GCEF_ADDTOLOG;
					}
					gce.ptszText = TranslateT("Moderator");
				}
				gce.ptszStatus = TranslateW(sttStatuses[JabberGcGetStatus(JS)]);
				gce.bIsMe = (mir_wstrcmp(nick, myNick) == 0);
				statusToSet = JS->m_iStatus;
				break;
			}
		}
	}

	Chat_Event(&gce);

	if (statusToSet != 0) {
		int flags = GC_SSE_ONLYLISTED;
		if (statusToSet == ID_STATUS_AWAY || statusToSet == ID_STATUS_NA || statusToSet == ID_STATUS_DND)
			flags += GC_SSE_ONLINE;
		Chat_SetStatusEx(m_szModuleName, item->jid, flags, nick);

		gce.iType = GC_EVENT_SETCONTACTSTATUS;
		gce.ptszText = nick;
		gce.ptszUID = resource;
		gce.dwItemData = statusToSet;
		Chat_Event(&gce);
	}
}

void CJabberProto::GcQuit(JABBER_LIST_ITEM *item, int code, HXML reason)
{
	wchar_t *szMessage = nullptr;

	if (code != 307 && code != 301) {
		ptrW tszMessage(getWStringA("GcMsgQuit"));
		if (tszMessage != nullptr)
			szMessage = NEWWSTR_ALLOCA(tszMessage);
		else
			szMessage = TranslateW(JABBER_GC_MSG_QUIT);
	}
	else {
		ptrW myNick(JabberNickFromJID(m_szJabberJID));
		GcLogUpdateMemberStatus(item, myNick, myNick, nullptr, GC_EVENT_KICK, reason);
	}

	if (code == 200)
		Chat_Terminate(m_szModuleName, item->jid);
	else
		Chat_Control(m_szModuleName, item->jid, SESSION_OFFLINE);

	db_unset(item->hContact, "CList", "Hidden");
	item->bChatActive = false;

	if (m_bJabberOnline) {
		wchar_t szPresenceTo[JABBER_MAX_JID_LEN];
		mir_snwprintf(szPresenceTo, L"%s/%s", item->jid, item->nick);

		m_ThreadInfo->send(
			XmlNode(L"presence") << XATTR(L"to", szPresenceTo) << XATTR(L"type", L"unavailable")
			<< XCHILD(L"status", szMessage));

		ListRemove(LIST_CHATROOM, item->jid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu hooks

static gc_item *sttFindGcMenuItem(int nItems, gc_item *items, DWORD id)
{
	for (int i = 0; i < nItems; i++)
		if (items[i].dwID == id)
			return items + i;
	
	return nullptr;
}

static void sttSetupGcMenuItem(int nItems, gc_item *items, DWORD id, bool disabled)
{
	for (int i = 0; i < nItems; i++)
		if (!id || (items[i].dwID == id))
			items[i].bDisabled = disabled;
}

static void sttShowGcMenuItem(int nItems, gc_item *items, DWORD id, int type)
{
	for (int i = 0; i < nItems; i++)
		if (!id || (items[i].dwID == id))
			items[i].uType = type;
}

static void sttSetupGcMenuItems(int nItems, gc_item *items, DWORD *ids, bool disabled)
{
	for (; *ids; ++ids)
		sttSetupGcMenuItem(nItems, items, *ids, disabled);
}

static void sttShowGcMenuItems(int nItems, gc_item *items, DWORD *ids, int type)
{
	for (; *ids; ++ids)
		sttShowGcMenuItem(nItems, items, *ids, type);
}

static gc_item sttLogListItems[] =
{
	{ LPGENW("Change &nickname"), IDM_NICK, MENU_ITEM },
	{ LPGENW("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },

	{ LPGENW("&Roles"), IDM_ROLE, MENU_NEWPOPUP },
	{ LPGENW("&Participant list"), IDM_LST_PARTICIPANT, MENU_POPUPITEM },
	{ LPGENW("&Moderator list"), IDM_LST_MODERATOR, MENU_POPUPITEM },

	{ LPGENW("&Affiliations"), IDM_AFFLTN, MENU_NEWPOPUP },
	{ LPGENW("&Member list"), IDM_LST_MEMBER, MENU_POPUPITEM },
	{ LPGENW("&Admin list"), IDM_LST_ADMIN, MENU_POPUPITEM },
	{ LPGENW("&Owner list"), IDM_LST_OWNER, MENU_POPUPITEM },
	{ nullptr, 0, MENU_POPUPSEPARATOR },
	{ LPGENW("Outcast list (&ban)"), IDM_LST_BAN, MENU_POPUPITEM },

	{ LPGENW("&Room options"), 0, MENU_NEWPOPUP },
	{ LPGENW("View/change &topic"), IDM_TOPIC, MENU_POPUPITEM },
	{ LPGENW("Add to &bookmarks"), IDM_BOOKMARKS, MENU_POPUPITEM },
	{ LPGENW("&Configure..."), IDM_CONFIG, MENU_POPUPITEM },
	{ LPGENW("&Destroy room"), IDM_DESTROY, MENU_POPUPITEM },

	{ nullptr, 0, MENU_SEPARATOR },

	{ LPGENW("Lin&ks"), 0, MENU_NEWPOPUP },
	{ nullptr, IDM_LINK0, 0 },
	{ nullptr, IDM_LINK1, 0 },
	{ nullptr, IDM_LINK2, 0 },
	{ nullptr, IDM_LINK3, 0 },
	{ nullptr, IDM_LINK4, 0 },
	{ nullptr, IDM_LINK5, 0 },
	{ nullptr, IDM_LINK6, 0 },
	{ nullptr, IDM_LINK7, 0 },
	{ nullptr, IDM_LINK8, 0 },
	{ nullptr, IDM_LINK9, 0 },

	{ LPGENW("Copy room &JID"), IDM_CPY_RJID, MENU_ITEM },
	{ LPGENW("Copy room topic"), IDM_CPY_TOPIC, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },

	{ LPGENW("&Send presence"), 0, MENU_NEWPOPUP },
	{ LPGENW("Online"), IDM_PRESENCE_ONLINE, MENU_POPUPITEM },
	{ LPGENW("Away"), IDM_PRESENCE_AWAY, MENU_POPUPITEM },
	{ LPGENW("Not available"), IDM_PRESENCE_NA, MENU_POPUPITEM },
	{ LPGENW("Do not disturb"), IDM_PRESENCE_DND, MENU_POPUPITEM },
	{ LPGENW("Free for chat"), IDM_PRESENCE_FREE4CHAT, MENU_POPUPITEM },

	{ LPGENW("&Leave chat session"), IDM_LEAVE, MENU_ITEM }
};

static wchar_t sttRJidBuf[JABBER_MAX_JID_LEN] = { 0 };
static gc_item sttListItems[] =
{
	{ LPGENW("&Slap"), IDM_SLAP, MENU_ITEM },   // 0
	{ LPGENW("&User details"), IDM_VCARD, MENU_ITEM },   // 1
	{ LPGENW("Member &info"), IDM_INFO, MENU_ITEM },   // 2

	{ sttRJidBuf, 0, MENU_NEWPOPUP },   // 3 -> accessed explicitly by index!!!
	{ LPGENW("User &details"), IDM_RJID_VCARD, MENU_POPUPITEM },
	{ LPGENW("&Add to roster"), IDM_RJID_ADD, MENU_POPUPITEM },
	{ LPGENW("&Copy to clipboard"), IDM_RJID_COPY, MENU_POPUPITEM },

	{ LPGENW("Invite to room"), 0, MENU_NEWPOPUP },
	{ nullptr, IDM_LINK0, 0 },
	{ nullptr, IDM_LINK1, 0 },
	{ nullptr, IDM_LINK2, 0 },
	{ nullptr, IDM_LINK3, 0 },
	{ nullptr, IDM_LINK4, 0 },
	{ nullptr, IDM_LINK5, 0 },
	{ nullptr, IDM_LINK6, 0 },
	{ nullptr, IDM_LINK7, 0 },
	{ nullptr, IDM_LINK8, 0 },
	{ nullptr, IDM_LINK9, 0 },

	{ nullptr, 0, MENU_SEPARATOR },

	{ LPGENW("Set &role"), IDM_ROLE, MENU_NEWPOPUP },
	{ LPGENW("&Visitor"), IDM_SET_VISITOR, MENU_POPUPITEM },
	{ LPGENW("&Participant"), IDM_SET_PARTICIPANT, MENU_POPUPITEM },
	{ LPGENW("&Moderator"), IDM_SET_MODERATOR, MENU_POPUPITEM },

	{ LPGENW("Set &affiliation"), IDM_AFFLTN, MENU_NEWPOPUP },
	{ LPGENW("&None"), IDM_SET_NONE, MENU_POPUPITEM },
	{ LPGENW("&Member"), IDM_SET_MEMBER, MENU_POPUPITEM },
	{ LPGENW("&Admin"), IDM_SET_ADMIN, MENU_POPUPITEM },
	{ LPGENW("&Owner"), IDM_SET_OWNER, MENU_POPUPITEM },
	{ nullptr, 0, MENU_POPUPSEPARATOR },
	{ LPGENW("Outcast (&ban)"), IDM_SET_BAN, MENU_POPUPITEM },

	{ LPGENW("&Kick"), IDM_KICK, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },
	{ LPGENW("Copy &nickname"), IDM_CPY_NICK, MENU_ITEM },
	{ LPGENW("Copy real &JID"), IDM_CPY_RJID, MENU_ITEM },
	{ LPGENW("Copy in-room JID"), IDM_CPY_INROOMJID, MENU_ITEM }
};

int CJabberProto::JabberGcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, gcmi->pszID);
	if (item == nullptr)
		return 0;

	pResourceStatus me(nullptr), him(nullptr);
	for (int i = 0; i < item->arResources.getCount(); i++) {
		JABBER_RESOURCE_STATUS *p = item->arResources[i];
		if (!mir_wstrcmp(p->m_tszResourceName, item->nick))
			me = p;
		if (!mir_wstrcmp(p->m_tszResourceName, gcmi->pszUID))
			him = p;
	}

	if (gcmi->Type == MENU_ON_LOG) {
		static wchar_t url_buf[1024] = { 0 };

		static DWORD sttModeratorItems[] = { IDM_LST_PARTICIPANT, 0 };
		static DWORD sttAdminItems[] = { IDM_LST_MODERATOR, IDM_LST_MEMBER, IDM_LST_ADMIN, IDM_LST_OWNER, IDM_LST_BAN, 0 };
		static DWORD sttOwnerItems[] = { IDM_CONFIG, IDM_DESTROY, 0 };

		sttSetupGcMenuItem(_countof(sttLogListItems), sttLogListItems, 0, FALSE);

		int idx = IDM_LINK0;
		wchar_t *ptszStatusMsg = item->getTemp()->m_tszStatusMessage;
		if (ptszStatusMsg && *ptszStatusMsg) {
			wchar_t *bufPtr = url_buf;
			for (wchar_t *p = wcsstr(ptszStatusMsg, L"http"); p && *p; p = wcsstr(p + 1, L"http")) {
				if (!wcsncmp(p, L"http://", 7) || !wcsncmp(p, L"https://", 8)) {
					mir_wstrncpy(bufPtr, p, _countof(url_buf) - (bufPtr - url_buf));
					gc_item *pItem = sttFindGcMenuItem(_countof(sttLogListItems), sttLogListItems, idx);
					pItem->pszDesc = bufPtr;
					pItem->uType = MENU_POPUPITEM;
					for (; *bufPtr && !iswspace(*bufPtr); ++bufPtr);
					*bufPtr++ = 0;

					if (++idx > IDM_LINK9)
						break;
				}
			}
		}
		for (; idx <= IDM_LINK9; ++idx)
			sttFindGcMenuItem(_countof(sttLogListItems), sttLogListItems, idx)->uType = 0;

		if (!GetAsyncKeyState(VK_CONTROL)) {
			if (me) {
				sttSetupGcMenuItems(_countof(sttLogListItems), sttLogListItems, sttModeratorItems, (me->m_role < ROLE_MODERATOR));
				sttSetupGcMenuItems(_countof(sttLogListItems), sttLogListItems, sttAdminItems, (me->m_affiliation < AFFILIATION_ADMIN));
				sttSetupGcMenuItems(_countof(sttLogListItems), sttLogListItems, sttOwnerItems, (me->m_affiliation < AFFILIATION_OWNER));
			}
			if (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)
				sttSetupGcMenuItem(_countof(sttLogListItems), sttLogListItems, IDM_BOOKMARKS, FALSE);
		}
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		static DWORD sttRJidItems[] = { IDM_RJID_VCARD, IDM_RJID_ADD, IDM_RJID_COPY, 0 };

		if (me && him) {
			int i, idx;
			BOOL force = GetAsyncKeyState(VK_CONTROL);
			sttSetupGcMenuItem(_countof(sttListItems), sttListItems, 0, FALSE);

			idx = IDM_LINK0;
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
				if (item = ListGetItemPtrFromIndex(i)) {
					if (!item->bChatActive)
						continue;

					gc_item *pItem = sttFindGcMenuItem(_countof(sttListItems), sttListItems, idx);
					pItem->pszDesc = item->jid;
					pItem->uType = MENU_POPUPITEM;
					if (++idx > IDM_LINK9)
						break;
				}

			for (; idx <= IDM_LINK9; ++idx)
				sttFindGcMenuItem(_countof(sttListItems), sttListItems, idx)->uType = 0;

			for (i = 0; i < _countof(sttAffiliationItems); i++) {
				gc_item *pItem = sttFindGcMenuItem(_countof(sttListItems), sttListItems, sttAffiliationItems[i].id);
				pItem->uType = (him->m_affiliation == sttAffiliationItems[i].value) ? MENU_POPUPCHECK : MENU_POPUPITEM;
				pItem->bDisabled = !(force || sttAffiliationItems[i].check(me, him));
			}

			for (i = 0; i < _countof(sttRoleItems); i++) {
				gc_item *pItem = sttFindGcMenuItem(_countof(sttListItems), sttListItems, sttRoleItems[i].id);
				pItem->uType = (him->m_role == sttRoleItems[i].value) ? MENU_POPUPCHECK : MENU_POPUPITEM;
				pItem->bDisabled = !(force || sttRoleItems[i].check(me, him));
			}

			if (him->m_tszRealJid && *him->m_tszRealJid) {
				mir_snwprintf(sttRJidBuf, TranslateT("Real &JID: %s"), him->m_tszRealJid);
				if (wchar_t *tmp = wcschr(sttRJidBuf, '/')) *tmp = 0;

				if (MCONTACT hContact = HContactFromJID(him->m_tszRealJid)) {
					sttListItems[3].uType = MENU_HMENU;
					sttListItems[3].dwID = (INT_PTR)Menu_BuildContactMenu(hContact);
					sttShowGcMenuItems(_countof(sttListItems), sttListItems, sttRJidItems, 0);
				}
				else {
					sttListItems[3].uType = MENU_NEWPOPUP;
					sttShowGcMenuItems(_countof(sttListItems), sttListItems, sttRJidItems, MENU_POPUPITEM);
				}

				sttSetupGcMenuItem(_countof(sttListItems), sttListItems, IDM_CPY_RJID, FALSE);
			}
			else {
				sttListItems[3].uType = 0;
				sttShowGcMenuItems(_countof(sttListItems), sttListItems, sttRJidItems, 0);

				sttSetupGcMenuItem(_countof(sttListItems), sttListItems, IDM_CPY_RJID, TRUE);
			}

			if (!force) {
				if (me->m_role < ROLE_MODERATOR || (me->m_affiliation <= him->m_affiliation))
					sttSetupGcMenuItem(_countof(sttListItems), sttListItems, IDM_KICK, TRUE);

				if ((me->m_affiliation < AFFILIATION_ADMIN) ||
					(me->m_affiliation == AFFILIATION_ADMIN) && (me->m_affiliation <= him->m_affiliation))
					sttSetupGcMenuItem(_countof(sttListItems), sttListItems, IDM_SET_BAN, TRUE);
			}
		}
		else {
			sttSetupGcMenuItem(_countof(sttListItems), sttListItems, 0, TRUE);
			sttListItems[2].uType = 0;
			sttShowGcMenuItems(_countof(sttListItems), sttListItems, sttRJidItems, 0);
		}
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttListItems), sttListItems);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Conference invitation dialog

class CGroupchatInviteDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	struct JabberGcLogInviteDlgJidData
	{
		int hItem;
		wchar_t jid[JABBER_MAX_JID_LEN];
	};

	LIST<JabberGcLogInviteDlgJidData> m_newJids;
	wchar_t *m_room;

	CCtrlButton  m_btnInvite;
	CCtrlEdit    m_txtNewJid;
	CCtrlMButton m_btnAddJid;
	CCtrlEdit    m_txtReason;
	CCtrlClc     m_clc;

	void FilterList(CCtrlClc *)
	{
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			char *proto = GetContactProto(hContact);
			if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					m_clc.DeleteItem(hItem);
		}
	}

	void ResetListOptions(CCtrlClc *)
	{
		m_clc.SetBkBitmap(0, nullptr);
		m_clc.SetBkColor(GetSysColor(COLOR_WINDOW));
		m_clc.SetGreyoutFlags(0);
		m_clc.SetLeftMargin(4);
		m_clc.SetIndent(10);
		m_clc.SetHideEmptyGroups(1);
		m_clc.SetHideOfflineRoot(1);
		for (int i = 0; i <= FONTID_MAX; i++)
			m_clc.SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
	}

	void InviteUser(wchar_t *pUser, wchar_t *text)
	{
		XmlNode msg(L"message");
		HXML invite = msg << XATTR(L"to", m_room) << XATTRID(m_proto->SerialNext())
			<< XCHILDNS(L"x", JABBER_FEAT_MUC_USER)
			<< XCHILD(L"invite") << XATTR(L"to", pUser);
		if (text)
			invite << XCHILD(L"reason", text);

		m_proto->m_ThreadInfo->send(msg);
	}

public:
	CGroupchatInviteDlg(CJabberProto *ppro, const wchar_t *room) :
		CSuper(ppro, IDD_GROUPCHAT_INVITE, nullptr),
		m_newJids(1),
		m_btnInvite(this, IDC_INVITE),
		m_txtNewJid(this, IDC_NEWJID),
		m_btnAddJid(this, IDC_ADDJID, ppro->LoadIconEx("addroster"), "Add"),
		m_txtReason(this, IDC_REASON),
		m_clc(this, IDC_CLIST)
	{
		m_room = mir_wstrdup(room);
		m_btnAddJid.OnClick = Callback(this, &CGroupchatInviteDlg::OnCommand_AddJid);
		m_btnInvite.OnClick = Callback(this, &CGroupchatInviteDlg::OnCommand_Invite);
		m_clc.OnNewContact =
			m_clc.OnListRebuilt = Callback(this, &CGroupchatInviteDlg::FilterList);
		m_clc.OnOptionsChanged = Callback(this, &CGroupchatInviteDlg::ResetListOptions);
	}

	~CGroupchatInviteDlg()
	{
		for (int i = 0; i < m_newJids.getCount(); i++)
			mir_free(m_newJids[i]);
		mir_free(m_room);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		SetDlgItemText(m_hwnd, IDC_HEADERBAR, CMStringW(FORMAT, TranslateT("Invite Users to\n%s"), m_room));
		Window_SetIcon_IcoLib(m_hwnd, g_GetIconHandle(IDI_GROUP));

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE,
			GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_HIDEOFFLINE | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
		ResetListOptions(&m_clc);
		FilterList(&m_clc);
	}

	void OnCommand_AddJid(CCtrlButton*)
	{
		wchar_t buf[JABBER_MAX_JID_LEN];
		m_txtNewJid.GetText(buf, _countof(buf));
		m_txtNewJid.SetTextA("");

		MCONTACT hContact = m_proto->HContactFromJID(buf);
		if (hContact) {
			int hItem = SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem)
				SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, hItem, 1);
			return;
		}

		int i;
		for (i = 0; i < m_newJids.getCount(); i++)
			if (!mir_wstrcmp(m_newJids[i]->jid, buf))
				break;
		if (i != m_newJids.getCount())
			return;

		JabberGcLogInviteDlgJidData *jidData = (JabberGcLogInviteDlgJidData *)mir_alloc(sizeof(JabberGcLogInviteDlgJidData));
		mir_wstrcpy(jidData->jid, buf);
		CLCINFOITEM cii = { 0 };
		cii.cbSize = sizeof(cii);
		cii.flags = CLCIIF_CHECKBOX;
		mir_snwprintf(buf, TranslateT("%s (not on roster)"), jidData->jid);
		cii.pszText = buf;
		jidData->hItem = SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, jidData->hItem, 1);
		m_newJids.insert(jidData);
	}

	void OnCommand_Invite(CCtrlButton*)
	{
		if (!m_room) return;

		ptrW text(m_txtReason.GetText());
		HWND hwndList = GetDlgItem(m_hwnd, IDC_CLIST);

		// invite users from roster
		for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (int hItem = SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0)) {
				if (SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
					ptrW jid(m_proto->getWStringA(hContact, "jid"));
					if (jid != nullptr)
						InviteUser(jid, text);
				}
			}
		}

		// invite others
		for (int i = 0; i < m_newJids.getCount(); i++)
			if (SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)m_newJids[i]->hItem, 0))
				InviteUser(m_newJids[i]->jid, text);

		Close();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu processing

void CJabberProto::AdminSet(const wchar_t *to, const wchar_t *ns, const wchar_t *szItem, const wchar_t *itemVal, const wchar_t *var, const wchar_t *varVal)
{
	m_ThreadInfo->send(XmlNodeIq(L"set", SerialNext(), to) << XQUERY(ns) << XCHILD(L"item") << XATTR(szItem, itemVal) << XATTR(var, varVal));
}

void CJabberProto::AdminSetReason(const wchar_t *to, const wchar_t *ns, const wchar_t *szItem, const wchar_t *itemVal, const wchar_t *var, const wchar_t *varVal, const wchar_t *rsn)
{
	m_ThreadInfo->send(XmlNodeIq(L"set", SerialNext(), to) << XQUERY(ns) << XCHILD(L"item") << XATTR(szItem, itemVal) << XATTR(var, varVal) << XCHILD(L"reason", rsn));
}

void CJabberProto::AdminGet(const wchar_t *to, const wchar_t *ns, const wchar_t *var, const wchar_t *varVal, JABBER_IQ_HANDLER foo)
{
	m_ThreadInfo->send(XmlNodeIq(AddIQ(foo, JABBER_IQ_TYPE_GET, to))
		<< XQUERY(ns) << XCHILD(L"item") << XATTR(var, varVal));
}

// Member info dialog
struct TUserInfoData
{
	CJabberProto *ppro;
	JABBER_LIST_ITEM *item;
	JABBER_RESOURCE_STATUS *me, *him;
};

static INT_PTR CALLBACK sttUserInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TUserInfoData *dat = (TUserInfoData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	int value, idx;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		dat = (TUserInfoData *)lParam;

		Window_SetIcon_IcoLib(hwndDlg, g_GetIconHandle(IDI_GROUP));
		{
			LOGFONT lf;
			GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_TXT_NICK, WM_GETFONT, 0, 0), sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			HFONT hfnt = CreateFontIndirect(&lf);
			SendDlgItemMessage(hwndDlg, IDC_TXT_NICK, WM_SETFONT, (WPARAM)hfnt, TRUE);
		}

		SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_FILE));
		SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BUTTONADDTOOLTIP, (WPARAM)"Apply", 0);

		SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_FILE));
		SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BUTTONADDTOOLTIP, (WPARAM)"Apply", 0);

		SendDlgItemMessage(hwndDlg, IDC_ICO_STATUS, STM_SETICON, (WPARAM)Skin_LoadProtoIcon(dat->ppro->m_szModuleName, dat->him->m_iStatus), 0);

		wchar_t buf[256];
		mir_snwprintf(buf, TranslateT("%s from\n%s"), dat->him->m_tszResourceName, dat->item->jid);
		SetDlgItemText(hwndDlg, IDC_HEADERBAR, buf);

		SetDlgItemText(hwndDlg, IDC_TXT_NICK, dat->him->m_tszResourceName);
		SetDlgItemText(hwndDlg, IDC_TXT_JID, dat->him->m_tszRealJid ? dat->him->m_tszRealJid : TranslateT("Real JID not available"));
		SetDlgItemText(hwndDlg, IDC_TXT_STATUS, dat->him->m_tszStatusMessage);

		for (int i = 0; i < _countof(sttRoleItems); i++) {
			if ((sttRoleItems[i].value == dat->him->m_role) || sttRoleItems[i].check(dat->me, dat->him)) {
				SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_SETITEMDATA,
					idx = SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_ADDSTRING, 0, (LPARAM)sttRoleItems[i].title),
					sttRoleItems[i].value);
				if (sttRoleItems[i].value == dat->him->m_role)
					SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_SETCURSEL, idx, 0);
			}
		}
		for (int i = 0; i < _countof(sttAffiliationItems); i++) {
			if ((sttAffiliationItems[i].value == dat->him->m_affiliation) || sttAffiliationItems[i].check(dat->me, dat->him)) {
				SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_SETITEMDATA,
					idx = SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_ADDSTRING, 0, (LPARAM)sttAffiliationItems[i].title),
					sttAffiliationItems[i].value);
				if (sttAffiliationItems[i].value == dat->him->m_affiliation)
					SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_SETCURSEL, idx, 0);
			}
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_ROLE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_AFFILIATION), FALSE);
		break;

	case WM_COMMAND:
		if (!dat)
			break;

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;

		case IDC_TXT_AFFILIATION:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				value = SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_GETCURSEL, 0, 0), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_AFFILIATION), dat->him->m_affiliation != value);
			}
			break;

		case IDC_BTN_AFFILIATION:
			value = SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_GETITEMDATA,
				SendDlgItemMessage(hwndDlg, IDC_TXT_AFFILIATION, CB_GETCURSEL, 0, 0), 0);
			if (dat->him->m_affiliation == value)
				break;

			wchar_t szBareJid[JABBER_MAX_JID_LEN];
			JabberStripJid(dat->him->m_tszRealJid, szBareJid, _countof(szBareJid));

			switch (value) {
			case AFFILIATION_NONE:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"none");
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"affiliation", L"none");
				break;
			case AFFILIATION_MEMBER:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"member");
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"affiliation", L"member");
				break;
			case AFFILIATION_ADMIN:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"admin");
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"affiliation", L"admin");
				break;
			case AFFILIATION_OWNER:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"owner");
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"affiliation", L"owner");
			}
			break;

		case IDC_TXT_ROLE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				value = SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_GETCURSEL, 0, 0), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_ROLE), dat->him->m_role != value);
			}
			break;

		case IDC_BTN_ROLE:
			value = SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_GETITEMDATA,
				SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_GETCURSEL, 0, 0), 0);
			if (dat->him->m_role == value)
				break;

			switch (value) {
			case ROLE_VISITOR:
				dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"role", L"visitor");
				break;
			case ROLE_PARTICIPANT:
				dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"role", L"participant");
				break;
			case ROLE_MODERATOR:
				dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", dat->him->m_tszResourceName, L"role", L"moderator");
				break;
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BM_SETIMAGE, IMAGE_ICON, 0));
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BM_SETIMAGE, IMAGE_ICON, 0));
		if (dat) {
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			mir_free(dat);
		}
		break;
	}
	return FALSE;
}

static void sttNickListHook(CJabberProto *ppro, JABBER_LIST_ITEM *item, GCHOOK* gch)
{
	pResourceStatus me(item->findResource(item->nick)), him(item->findResource(gch->ptszUID));
	if (him == nullptr || me == nullptr)
		return;

	// 1 kick per second, prevents crashes...
	enum { BAN_KICK_INTERVAL = 1000 };
	static DWORD dwLastBanKickTime = 0;

	CMStringW szBuffer, szTitle;

	if ((gch->dwData >= CLISTMENUIDMIN) && (gch->dwData <= CLISTMENUIDMAX)) {
		if (him->m_tszRealJid && *him->m_tszRealJid)
			if (MCONTACT hContact = ppro->HContactFromJID(him->m_tszRealJid))
				Clist_MenuProcessCommand(gch->dwData, MPCF_CONTACTMENU, hContact);
		return;
	}

	switch (gch->dwData) {
	case IDM_SLAP:
		if (ppro->m_bJabberOnline) {
			ptrW szMessage(ppro->getWStringA("GcMsgSlap"));
			if (szMessage == nullptr)
				szMessage = mir_wstrdup(TranslateW(JABBER_GC_MSG_SLAP));

			wchar_t buf[256];
			// do not use snprintf to avoid possible problems with % symbol
			if (wchar_t *p = wcsstr(szMessage, L"%s")) {
				*p = 0;
				mir_snwprintf(buf, L"%s%s%s", szMessage, him->m_tszResourceName, p + 2);
			}
			else mir_wstrncpy(buf, szMessage, _countof(buf));
			Chat_UnescapeTags(buf);

			ppro->m_ThreadInfo->send(
				XmlNode(L"message") << XATTR(L"to", item->jid) << XATTR(L"type", L"groupchat")
				<< XCHILD(L"body", buf));
		}
		break;

	case IDM_VCARD:
		{
			CMStringW jid(FORMAT, L"%s/%s", item->jid, him->m_tszResourceName);

			MCONTACT hContact = ppro->AddToListByJID(jid, PALF_TEMPORARY);
			ppro->setWString(hContact, "Nick", him->m_tszResourceName);
			
			JABBER_LIST_ITEM *pTmp = ppro->ListAdd(LIST_VCARD_TEMP, jid, hContact);
			ppro->ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_tszStatusMessage, him->m_iPriority);

			pTmp->findResource(him->m_tszResourceName)->m_pCaps = ppro->ListGetItemPtr(LIST_CHATROOM, item->jid)->findResource(him->m_tszResourceName)->m_pCaps;

			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case IDM_INFO:
		{
			TUserInfoData *dat = (TUserInfoData *)mir_alloc(sizeof(TUserInfoData));
			dat->me = me;
			dat->him = him;
			dat->item = item;
			dat->ppro = ppro;
			HWND hwndInfo = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_GROUPCHAT_INFO), nullptr, sttUserInfoDlgProc, (LPARAM)dat);
			ShowWindow(hwndInfo, SW_SHOW);
		}
		break;

	case IDM_KICK:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			dwLastBanKickTime = GetTickCount();
			szBuffer.Format(L"%s: ", me->m_tszResourceName);
			szTitle.Format(TranslateT("Reason to kick %s"), him->m_tszResourceName);
			wchar_t *resourceName_copy = mir_wstrdup(him->m_tszResourceName); // copy resource name to prevent possible crash if user list rebuilds
			if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
				ppro->m_ThreadInfo->send(
				XmlNodeIq(L"set", ppro->SerialNext(), item->jid) << XQUERY(JABBER_FEAT_MUC_ADMIN)
				<< XCHILD(L"item") << XATTR(L"nick", resourceName_copy) << XATTR(L"role", L"none")
				<< XCHILD(L"reason", szBuffer));

			mir_free(resourceName_copy);
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_SET_VISITOR:
		if (him->m_role != ROLE_VISITOR)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"role", L"visitor");
		break;

	case IDM_SET_PARTICIPANT:
		if (him->m_role != ROLE_PARTICIPANT)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"role", L"participant");
		break;

	case IDM_SET_MODERATOR:
		if (him->m_role != ROLE_MODERATOR)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"role", L"moderator");
		break;

	case IDM_SET_NONE:
		if (him->m_affiliation != AFFILIATION_NONE) {
			if (him->m_tszRealJid) {
				wchar_t szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"none");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"affiliation", L"none");
		}
		break;

	case IDM_SET_MEMBER:
		if (him->m_affiliation != AFFILIATION_MEMBER) {
			if (him->m_tszRealJid) {
				wchar_t szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"member");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"affiliation", L"member");
		}
		break;

	case IDM_SET_ADMIN:
		if (him->m_affiliation != AFFILIATION_ADMIN) {
			if (him->m_tszRealJid) {
				wchar_t szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"admin");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"affiliation", L"admin");
		}
		break;

	case IDM_SET_OWNER:
		if (him->m_affiliation != AFFILIATION_OWNER) {
			if (him->m_tszRealJid) {
				wchar_t szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"jid", szBareJid, L"affiliation", L"owner");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, L"nick", him->m_tszResourceName, L"affiliation", L"owner");
		}
		break;

	case IDM_SET_BAN:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			if (him->m_tszRealJid && *him->m_tszRealJid) {
				wchar_t szVictimBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szVictimBareJid, _countof(szVictimBareJid));

				szBuffer.Format(L"%s: ", me->m_tszResourceName);
				szTitle.Format(TranslateT("Reason to ban %s"), him->m_tszResourceName);

				if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
					ppro->m_ThreadInfo->send(
					XmlNodeIq(L"set", ppro->SerialNext(), item->jid) << XQUERY(JABBER_FEAT_MUC_ADMIN)
					<< XCHILD(L"item") << XATTR(L"jid", szVictimBareJid) << XATTR(L"affiliation", L"outcast")
					<< XCHILD(L"reason", szBuffer));
			}
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_LINK0: case IDM_LINK1: case IDM_LINK2: case IDM_LINK3: case IDM_LINK4:
	case IDM_LINK5: case IDM_LINK6: case IDM_LINK7: case IDM_LINK8: case IDM_LINK9:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			wchar_t *resourceName_copy = NEWWSTR_ALLOCA(him->m_tszResourceName); // copy resource name to prevent possible crash if user list rebuilds

			wchar_t *szInviteTo = 0;
			int idx = gch->dwData - IDM_LINK0;
			LISTFOREACH(i, ppro, LIST_CHATROOM)
			{
				if (JABBER_LIST_ITEM *pItem = ppro->ListGetItemPtrFromIndex(i)) {
					if (!pItem->bChatActive) continue;
					if (!idx--) {
						szInviteTo = pItem->jid;
						break;
					}
				}
			}

			if (!szInviteTo) break;

			szTitle.Format(TranslateT("Invite %s to %s"), him->m_tszResourceName, szInviteTo);
			if (!ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE))
				break;

			szTitle.Format(L"%s/%s", item->jid, resourceName_copy);

			XmlNode msg(L"message");
			msg << XATTR(L"to", szTitle) << XATTRID(ppro->SerialNext())
				<< XCHILD(L"x", szBuffer) << XATTR(L"xmlns", JABBER_FEAT_DIRECT_MUC_INVITE) << XATTR(L"jid", szInviteTo)
				<< XCHILD(L"invite") << XATTR(L"from", item->nick);
			ppro->m_ThreadInfo->send(msg);
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_CPY_NICK:
		JabberCopyText(pcli->hwndContactList, him->m_tszResourceName);
		break;

	case IDM_RJID_COPY:
	case IDM_CPY_RJID:
		JabberCopyText(pcli->hwndContactList, him->m_tszRealJid);
		break;

	case IDM_CPY_INROOMJID:
		szBuffer.Format(L"%s/%s", item->jid, him->m_tszResourceName);
		JabberCopyText(pcli->hwndContactList, szBuffer);
		break;

	case IDM_RJID_VCARD:
		if (him->m_tszRealJid && *him->m_tszRealJid) {
			wchar_t *jid = NEWWSTR_ALLOCA(him->m_tszRealJid);
			if (wchar_t *tmp = wcschr(jid, '/'))
				*tmp = 0;

			MCONTACT hContact = ppro->AddToListByJID(jid, PALF_TEMPORARY);
			ppro->ListAdd(LIST_VCARD_TEMP, jid, hContact);
			ppro->ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_tszStatusMessage, him->m_iPriority);

			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case IDM_RJID_ADD:
		if (him->m_tszRealJid && *him->m_tszRealJid) {
			PROTOSEARCHRESULT psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.id.w = NEWWSTR_ALLOCA(him->m_tszRealJid);
			if (wchar_t *tmp = wcschr(psr.id.w, '/'))
				*tmp = 0;
			psr.nick.w = psr.id.w;

			ADDCONTACTSTRUCT acs = { 0 };
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = ppro->m_szModuleName;
			acs.psr = &psr;
			CallService(MS_ADDCONTACT_SHOW, (WPARAM)pcli->hwndContactList, (LPARAM)&acs);
		}
		break;
	}
}

static void sttLogListHook(CJabberProto *ppro, JABBER_LIST_ITEM *item, GCHOOK* gch)
{
	CMStringW szBuffer, szTitle;

	switch (gch->dwData) {
	case IDM_LST_PARTICIPANT:
		ppro->AdminGet(gch->ptszID, JABBER_FEAT_MUC_ADMIN, L"role", L"participant", &CJabberProto::OnIqResultMucGetVoiceList);
		break;

	case IDM_LST_MEMBER:
		ppro->AdminGet(gch->ptszID, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"member", &CJabberProto::OnIqResultMucGetMemberList);
		break;

	case IDM_LST_MODERATOR:
		ppro->AdminGet(gch->ptszID, JABBER_FEAT_MUC_ADMIN, L"role", L"moderator", &CJabberProto::OnIqResultMucGetModeratorList);
		break;

	case IDM_LST_BAN:
		ppro->AdminGet(gch->ptszID, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"outcast", &CJabberProto::OnIqResultMucGetBanList);
		break;

	case IDM_LST_ADMIN:
		ppro->AdminGet(gch->ptszID, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"admin", &CJabberProto::OnIqResultMucGetAdminList);
		break;

	case IDM_LST_OWNER:
		ppro->AdminGet(gch->ptszID, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"owner", &CJabberProto::OnIqResultMucGetOwnerList);
		break;

	case IDM_TOPIC:
		szTitle.Format(TranslateT("Set topic for %s"), gch->ptszID);
		szBuffer = item->getTemp()->m_tszStatusMessage;
		szBuffer.Replace(L"\n", L"\r\n");
		if (ppro->EnterString(szBuffer, szTitle, ESF_RICHEDIT, "gcTopic_"))
			ppro->m_ThreadInfo->send(
				XmlNode(L"message") << XATTR(L"to", gch->ptszID) << XATTR(L"type", L"groupchat") << XCHILD(L"subject", szBuffer));
		break;

	case IDM_NICK:
		szTitle.Format(TranslateT("Change nickname in %s"), gch->ptszID);
		if (item->nick)
			szBuffer = item->nick;
		if (ppro->EnterString(szBuffer, szTitle, ESF_COMBO, "gcNick_")) {
			if (ppro->ListGetItemPtr(LIST_CHATROOM, gch->ptszID) != nullptr) {
				wchar_t text[1024];
				mir_snwprintf(text, L"%s/%s", gch->ptszID, szBuffer.c_str());
				ppro->SendPresenceTo(ppro->m_iStatus == ID_STATUS_INVISIBLE ? ID_STATUS_ONLINE : ppro->m_iStatus, text, nullptr);
			}
		}
		break;

	case IDM_INVITE:
		(new CGroupchatInviteDlg(ppro, gch->ptszID))->Show();
		break;

	case IDM_CONFIG:
		ppro->m_ThreadInfo->send(
			XmlNodeIq(ppro->AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, gch->ptszID))
			<< XQUERY(JABBER_FEAT_MUC_OWNER));
		break;

	case IDM_BOOKMARKS:
		item = ppro->ListGetItemPtr(LIST_BOOKMARK, gch->ptszID);
		if (item == nullptr) {
			item = ppro->ListGetItemPtr(LIST_CHATROOM, gch->ptszID);
			if (item != nullptr) {
				item->type = L"conference";
				MCONTACT hContact = ppro->HContactFromJID(item->jid);
				item->name = pcli->pfnGetContactDisplayName(hContact, 0);
				ppro->AddEditBookmark(item);
			}
		}
		break;

	case IDM_DESTROY:
		szTitle.Format(TranslateT("Reason to destroy %s"), gch->ptszID);
		if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
			ppro->m_ThreadInfo->send(
			XmlNodeIq(L"set", ppro->SerialNext(), gch->ptszID) << XQUERY(JABBER_FEAT_MUC_OWNER)
			<< XCHILD(L"destroy") << XCHILD(L"reason", szBuffer));

		// fall through
	case IDM_LEAVE:
		ppro->GcQuit(item, 200, nullptr);
		break;

	case IDM_PRESENCE_ONLINE:
	case IDM_PRESENCE_AWAY:
	case IDM_PRESENCE_NA:
	case IDM_PRESENCE_DND:
	case IDM_PRESENCE_FREE4CHAT:
		if (MCONTACT h = ppro->HContactFromJID(item->jid))
			ppro->OnMenuHandleDirectPresence((WPARAM)h, 0, gch->dwData);
		break;

	case IDM_LINK0: case IDM_LINK1: case IDM_LINK2: case IDM_LINK3: case IDM_LINK4:
	case IDM_LINK5: case IDM_LINK6: case IDM_LINK7: case IDM_LINK8: case IDM_LINK9:
		{
			int idx = IDM_LINK0;
			for (wchar_t *p = wcsstr(item->getTemp()->m_tszStatusMessage, L"http://"); p && *p; p = wcsstr(p + 1, L"http://")) {
				if (idx == gch->dwData) {
					char *bufPtr, *url = mir_u2a(p);
					for (bufPtr = url; *bufPtr && !isspace(*bufPtr); ++bufPtr);
					*bufPtr++ = 0;
					Utils_OpenUrl(url);
					mir_free(url);
					break;
				}

				if (++idx > IDM_LINK9) break;
			}
		}
		break;

	case IDM_CPY_RJID:
		JabberCopyText(pcli->hwndContactList, item->jid);
		break;

	case IDM_CPY_TOPIC:
		JabberCopyText(pcli->hwndContactList, item->getTemp()->m_tszStatusMessage);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a private message to a chat user

static void sttSendPrivateMessage(CJabberProto *ppro, JABBER_LIST_ITEM *item, const wchar_t *nick)
{
	wchar_t szFullJid[JABBER_MAX_JID_LEN];
	mir_snwprintf(szFullJid, L"%s/%s", item->jid, nick);
	MCONTACT hContact = ppro->DBCreateContact(szFullJid, nullptr, true, false);
	if (hContact != 0) {
		pResourceStatus r(item->findResource(nick));
		if (r)
			ppro->setWord(hContact, "Status", r->m_iStatus);

		db_set_b(hContact, "CList", "Hidden", 1);
		ppro->setWString(hContact, "Nick", nick);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// General chat event processing hook

int CJabberProto::JabberGcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->pszModule, m_szModuleName))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, gch->ptszID);
	if (item == nullptr)
		return 0;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);

			if (m_bJabberOnline) {
				wchar_t tszID[100];
				int64_t id = (_time64(0) << 16) + (GetTickCount() & 0xFFFF);

				wchar_t *buf = NEWWSTR_ALLOCA(gch->ptszText);
				Chat_UnescapeTags(buf);
				m_ThreadInfo->send(
					XmlNode(L"message") << XATTR(L"id", _i64tot(id, tszID, 36)) << XATTR(L"to", item->jid) << XATTR(L"type", L"groupchat")
					<< XCHILD(L"body", buf));
			}
		}
		break;

	case GC_USER_PRIVMESS:
		sttSendPrivateMessage(this, item, gch->ptszUID);
		break;

	case GC_USER_LOGMENU:
		sttLogListHook(this, item, gch);
		break;

	case GC_USER_NICKLISTMENU:
		sttNickListHook(this, item, gch);
		break;

	case GC_USER_CHANMGR:
		m_ThreadInfo->send(
			XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, item->jid))
			<< XQUERY(JABBER_FEAT_MUC_OWNER));
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const wchar_t* str, const wchar_t* rsn)
{
	const wchar_t *field = (jidListInfo->type == MUC_BANLIST || wcschr(str, '@')) ? L"jid" : L"nick";
	wchar_t *roomJid = jidListInfo->roomJid;
	if (jidListInfo->type == MUC_BANLIST) {
		AdminSetReason(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"affiliation", L"outcast", rsn);
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"outcast", &CJabberProto::OnIqResultMucGetBanList);
	}
}

void CJabberProto::AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const wchar_t* str)
{
	const wchar_t *field = (jidListInfo->type == MUC_BANLIST || wcschr(str, '@')) ? L"jid" : L"nick";
	wchar_t *roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"role", L"participant");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"role", L"participant", &CJabberProto::OnIqResultMucGetVoiceList);
		break;
	case MUC_MEMBERLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"affiliation", L"member");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"member", &CJabberProto::OnIqResultMucGetMemberList);
		break;
	case MUC_MODERATORLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"role", L"moderator");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"role", L"moderator", &CJabberProto::OnIqResultMucGetModeratorList);
		break;
	case MUC_BANLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"affiliation", L"outcast");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"outcast", &CJabberProto::OnIqResultMucGetBanList);
		break;
	case MUC_ADMINLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"affiliation", L"admin");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"admin", &CJabberProto::OnIqResultMucGetAdminList);
		break;
	case MUC_OWNERLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, L"affiliation", L"owner");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, L"affiliation", L"owner", &CJabberProto::OnIqResultMucGetOwnerList);
		break;
	}
}

void CJabberProto::DeleteMucListItem(JABBER_MUC_JIDLIST_INFO *jidListInfo, const wchar_t *jid)
{
	wchar_t *roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:		// change role to visitor (from participant)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, L"jid", jid, L"role", L"visitor");
		break;
	case MUC_BANLIST:		// change affiliation to none (from outcast)
	case MUC_MEMBERLIST:	// change affiliation to none (from member)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, L"jid", jid, L"affiliation", L"none");
		break;
	case MUC_MODERATORLIST:	// change role to participant (from moderator)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, L"jid", jid, L"role", L"participant");
		break;
	case MUC_ADMINLIST:		// change affiliation to member (from admin)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, L"jid", jid, L"affiliation", L"member");
		break;
	case MUC_OWNERLIST:		// change affiliation to admin (from owner)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, L"jid", jid, L"affiliation", L"admin");
		break;
	}
}
