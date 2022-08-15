/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

	IDM_CONFIG, IDM_NICK, IDM_AVATAR, IDM_DESTROY, IDM_INVITE, IDM_BOOKMARKS, IDM_LEAVE, IDM_TOPIC,
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
	IDM_PRESENCE_FREE4CHAT = ID_STATUS_FREECHAT
};

struct TRoleOrAffiliationInfo
{
	int value;
	int id;
	wchar_t *pwszTitle;
	int min_role;
	int min_affiliation;

	bool check(JABBER_RESOURCE_STATUS *me, JABBER_RESOURCE_STATUS *him)
	{
		if (me->m_affiliation == AFFILIATION_OWNER) return true;
		if (me == him) return false;
		if (me->m_affiliation <= him->m_affiliation) return false;
		if (me->m_role < min_role) return false;
		if (me->m_affiliation < min_affiliation) return false;
		return true;
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

static const char *sttStatuses[] = { LPGEN("Visitors"), LPGEN("Participants"), LPGEN("Moderators"), LPGEN("Owners") };;

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
	if (item->si)
		return 1;

	Utf2T wszJid(item->jid);
	ptrA szNick(JabberNickFromJID(item->jid));
	SESSION_INFO *si = item->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszJid, Utf2T(szNick));
	if (si == nullptr)
		return 2;

	item->hContact = si->hContact;

	if (JABBER_LIST_ITEM *bookmark = ListGetItemPtr(LIST_BOOKMARK, item->jid)) {
		if (bookmark->name) {
			ptrW myHandle(db_get_wsa(si->hContact, "CList", "MyHandle"));
			if (myHandle == nullptr)
				db_set_ws(si->hContact, "CList", "MyHandle", bookmark->name);
		}
	}

	ptrA tszNick(getUStringA(si->hContact, "MyNick"));
	if (tszNick != nullptr) {
		if (!mir_strcmp(tszNick, szNick))
			delSetting(si->hContact, "MyNick");
		else
			setUString(si->hContact, "MyNick", item->nick);
	}
	else setUString(si->hContact, "MyNick", item->nick);

	ptrA passw(getUStringA(si->hContact, "Password"));
	if (mir_strcmp(passw, item->password)) {
		if (!item->password || !item->password[0])
			delSetting(si->hContact, "Password");
		else
			setUString(si->hContact, "Password", item->password);
	}

	item->bChatLogging = false;
	item->iChatInitTime = time(0);

	for (int i = _countof(sttStatuses) - 1; i >= 0; i--)
		Chat_AddGroup(si, TranslateW(Utf2T(sttStatuses[i])));

	Chat_Control(m_szModuleName, wszJid, (item->bAutoJoin && m_bAutoJoinHidden) ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszJid, SESSION_ONLINE);

	time_t lastDate = getDword(si->hContact, "LastGetVcard"), now = time(0);
	if (now - lastDate > 24 * 60 * 60) {
		SendGetVcard(si->hContact);
		setDword(si->hContact, "LastGetVcard", now);
	}
	return 0;
}

void CJabberProto::GcLogShowInformation(JABBER_LIST_ITEM *item, pResourceStatus &user, TJabberGcLogInfoType type)
{
	if (!item || !user || !item->bChatLogging)
		return;

	CMStringA buf;

	switch (type) {
	case INFO_BAN:
		if (m_bGcLogBans)
			buf.Format(TranslateU("User %s is now banned."), user->m_szResourceName.get());
		break;

	case INFO_STATUS:
		if (m_bGcLogStatuses) {
			wchar_t *ptszDescr = Clist_GetStatusModeDescription(user->m_iStatus, 0);
			if (user->m_szStatusMessage)
				buf.Format(TranslateU("User %s changed status to %s with message: %s"), user->m_szResourceName.get(), ptszDescr, user->m_szStatusMessage.get());
			else
				buf.Format(TranslateU("User %s changed status to %s"), user->m_szResourceName.get(), ptszDescr);
		}
		break;

	case INFO_CONFIG:
		if (m_bGcLogConfig)
			buf.Append(TranslateU("Room configuration was changed."));
		break;

	case INFO_AFFILIATION:
		if (m_bGcLogAffiliations) {
			const char *name = nullptr;
			switch (user->m_affiliation) {
			case AFFILIATION_NONE:		name = TranslateU("None"); break;
			case AFFILIATION_MEMBER:	name = TranslateU("Member"); break;
			case AFFILIATION_ADMIN:		name = TranslateU("Admin"); break;
			case AFFILIATION_OWNER:		name = TranslateU("Owner"); break;
			case AFFILIATION_OUTCAST:	name = TranslateU("Outcast"); break;
			}
			if (name)
				buf.Format(TranslateU("Affiliation of %s was changed to '%s'."), user->m_szResourceName.get(), name);
		}
		break;

	case INFO_ROLE:
		if (m_bGcLogRoles) {
			const char *name = nullptr;
			switch (user->m_role) {
			case ROLE_NONE:			name = TranslateU("None"); break;
			case ROLE_VISITOR:		name = TranslateU("Visitor"); break;
			case ROLE_PARTICIPANT:	name = TranslateU("Participant"); break;
			case ROLE_MODERATOR:    name = TranslateU("Moderator"); break;
			}

			if (name)
				buf.Format(TranslateU("Role of %s was changed to '%s'."), user->m_szResourceName.get(), name);
		}
		break;
	}

	if (!buf.IsEmpty()) {
		buf.Replace("%", "%%");

		GCEVENT gce = { m_szModuleName, item->jid, GC_EVENT_INFORMATION };
		gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
		gce.pszNick.a = gce.pszUID.a = user->m_szResourceName;
		gce.pszText.a = buf;
		gce.time = time(0);
		Chat_Event(&gce);
	}
}

void CJabberProto::GcLogUpdateMemberStatus(JABBER_LIST_ITEM *item, const char *resource, const char *nick, const char *jid, int action, const TiXmlElement *reason, int nStatusCode)
{
	int statusToSet = 0;

	const char *szReason = (reason) ? reason->GetText() : nullptr;
	if (szReason == nullptr) {
		if (nStatusCode == 322)
			szReason = TranslateU("because room is now members-only");
		else if (nStatusCode == 301)
			szReason = TranslateU("user banned");
	}

	ptrA myNick(mir_strdup(item->nick));
	if (myNick == nullptr)
		myNick = JabberNickFromJID(m_szJabberJID);

	GCEVENT gce = { m_szModuleName, item->jid, 0 };
	gce.dwFlags = GCEF_UTF8 | ((item->bChatLogging) ? GCEF_ADDTOLOG : 0);
	gce.pszNick.a = nick;
	gce.pszUID.a = resource;
	gce.pszUserInfo.a = jid;
	gce.pszText.a = szReason;
	gce.time = time(0);

	switch (gce.iType = action) {
	case GC_EVENT_PART:  break;
	case GC_EVENT_KICK:
		gce.pszStatus.a = TranslateU("Moderator");
		break;
	
	default:
		mir_cslock lck(m_csLists);
		for (auto &JS : item->arResources) {
			if (!mir_strcmp(resource, JS->m_szResourceName)) {
				if (action != GC_EVENT_JOIN) {
					switch (action) {
					case 0:
						gce.iType = GC_EVENT_ADDSTATUS;
					case GC_EVENT_REMOVESTATUS:
						gce.dwFlags &= ~GCEF_ADDTOLOG;
					}
					gce.pszText.a = TranslateU("Moderator");
				}
				gce.pszStatus.a = TranslateU(sttStatuses[JabberGcGetStatus(JS)]);
				gce.bIsMe = (mir_strcmp(nick, myNick) == 0);
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
		Chat_SetStatusEx(m_szModuleName, Utf2T(item->jid), flags, Utf2T(nick));

		gce.iType = GC_EVENT_SETCONTACTSTATUS;
		gce.pszText.a = nick;
		gce.pszUID.a = resource;
		gce.dwItemData = statusToSet;
		Chat_Event(&gce);
	}
}

void CJabberProto::GcQuit(JABBER_LIST_ITEM *item, int code, const TiXmlElement *reason)
{
	CMStringA szMessage;

	if (code != 307 && code != 301) {
		ptrA quitMessage(getUStringA("GcMsgQuit"));
		if (quitMessage != nullptr)
			szMessage = quitMessage;
		else
			szMessage = TranslateU(JABBER_GC_MSG_QUIT);
	}
	else {
		ptrA myNick(JabberNickFromJID(m_szJabberJID));
		GcLogUpdateMemberStatus(item, myNick, myNick, nullptr, GC_EVENT_KICK, reason);
	}

	Utf2T wszRoomJid(item->jid);
	if (code == 200)
		Chat_Terminate(m_szModuleName, wszRoomJid);
	else
		Chat_Control(m_szModuleName, wszRoomJid, SESSION_OFFLINE);

	Contact::Hide(item->hContact, false);
	item->si = nullptr;
	item->bChatLogging = false;

	if (m_bJabberOnline) {
		m_ThreadInfo->send(
			XmlNode("presence") << XATTR("to", MakeJid(item->jid, item->nick)) << XATTR("type", "unavailable")
			<< XCHILD("status", szMessage));

		ListRemove(LIST_CHATROOM, item->jid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu hooks

static gc_item *sttFindGcMenuItem(int nItems, gc_item *items, uint32_t id)
{
	for (int i = 0; i < nItems; i++)
		if (items[i].dwID == id)
			return items + i;
	
	return nullptr;
}

static void sttSetupGcMenuItem(int nItems, gc_item *items, uint32_t id, bool disabled)
{
	for (int i = 0; i < nItems; i++)
		if (!id || (items[i].dwID == id))
			items[i].bDisabled = disabled;
}

static void sttShowGcMenuItem(int nItems, gc_item *items, uint32_t id, int type)
{
	for (int i = 0; i < nItems; i++)
		if (!id || (items[i].dwID == id))
			items[i].uType = type;
}

static void sttSetupGcMenuItems(int nItems, gc_item *items, uint32_t *ids, bool disabled)
{
	for (; *ids; ++ids)
		sttSetupGcMenuItem(nItems, items, *ids, disabled);
}

static void sttShowGcMenuItems(int nItems, gc_item *items, uint32_t *ids, int type)
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
	{ LPGENW("Change &avatar"), IDM_AVATAR, MENU_POPUPITEM },
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

static uint32_t sttModeratorItems[] = { IDM_LST_PARTICIPANT, 0 };
static uint32_t sttAdminItems[] = { IDM_LST_MODERATOR, IDM_LST_MEMBER, IDM_LST_ADMIN, IDM_LST_OWNER, IDM_LST_BAN, 0 };
static uint32_t sttOwnerItems[] = { IDM_CONFIG, IDM_DESTROY, IDM_AVATAR, 0 };

int CJabberProto::JabberGcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, T2Utf(gcmi->pszID));
	if (item == nullptr)
		return 0;

	pResourceStatus me(nullptr), him(nullptr);
	for (auto &p : item->arResources) {
		if (!mir_strcmp(p->m_szResourceName, item->nick))
			me = p;
		if (!mir_strcmp(p->m_szResourceName, T2Utf(gcmi->pszUID)))
			him = p;
	}

	if (gcmi->Type == MENU_ON_LOG) {
		static wchar_t url_buf[1024] = { 0 };

		sttSetupGcMenuItem(_countof(sttLogListItems), sttLogListItems, 0, FALSE);

		int idx = IDM_LINK0;
		char *ptszStatusMsg = item->getTemp()->m_szStatusMessage;
		if (ptszStatusMsg && *ptszStatusMsg) {
			wchar_t *bufPtr = url_buf;
			for (char *p = strstr(ptszStatusMsg, "http"); p && *p; p = strstr(p + 1, "http")) {
				if (!strncmp(p, "http://", 7) || !strncmp(p, "https://", 8)) {
					mir_wstrncpy(bufPtr, Utf2T(p), _countof(url_buf) - (bufPtr - url_buf));
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
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		static uint32_t sttRJidItems[] = { IDM_RJID_VCARD, IDM_RJID_ADD, IDM_RJID_COPY, 0 };

		if (me && him) {
			int i, idx;
			BOOL force = GetAsyncKeyState(VK_CONTROL);
			sttSetupGcMenuItem(_countof(sttListItems), sttListItems, 0, FALSE);

			idx = IDM_LINK0;
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
				if (item = ListGetItemPtrFromIndex(i)) {
					if (!item->si)
						continue;

					gc_item *pItem = sttFindGcMenuItem(_countof(sttListItems), sttListItems, idx);
					pItem->uType = MENU_POPUPITEM;
					pItem->pszDesc = item->si->ptszID;
					if (++idx > IDM_LINK9)
						break;
				}

			for (; idx <= IDM_LINK9; ++idx)
				sttFindGcMenuItem(_countof(sttListItems), sttListItems, idx)->uType = 0;

			for (auto &it : sttAffiliationItems) {
				gc_item *pItem = sttFindGcMenuItem(_countof(sttListItems), sttListItems, it.id);
				pItem->uType = (him->m_affiliation == it.value) ? MENU_POPUPCHECK : MENU_POPUPITEM;
				pItem->bDisabled = !(force || it.check(me, him));
			}

			for (auto &it : sttRoleItems) {
				gc_item *pItem = sttFindGcMenuItem(_countof(sttListItems), sttListItems, it.id);
				pItem->uType = (him->m_role == it.value) ? MENU_POPUPCHECK : MENU_POPUPITEM;
				pItem->bDisabled = !(force || it.check(me, him));
			}

			if (him->m_szRealJid && *him->m_szRealJid) {
				mir_snwprintf(sttRJidBuf, TranslateT("Real &JID: %s"), Utf2T(him->m_szRealJid).get());
				if (wchar_t *tmp = wcschr(sttRJidBuf, '/')) *tmp = 0;

				if (MCONTACT hContact = HContactFromJID(him->m_szRealJid)) {
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
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttListItems), sttListItems, &g_plugin);
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
		HANDLE hItem;
		char jid[JABBER_MAX_JID_LEN];
	};

	LIST<JabberGcLogInviteDlgJidData> m_newJids;
	char *m_room;

	CCtrlButton  m_btnInvite;
	CCtrlEdit    m_txtNewJid;
	CCtrlMButton m_btnAddJid;
	CCtrlEdit    m_txtReason;
	CCtrlClc     m_clc;

	bool FindJid(const char *buf)
	{
		for (auto &it : m_newJids)
			if (!mir_strcmp(it->jid, buf))
				return true;
		
		return false;
	}

	void FilterList(CCtrlClc *)
	{
		for (auto &hContact : Contacts()) {
			char *proto = Proto_GetBaseAccountName(hContact);
			if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					m_clc.DeleteItem(hItem);
		}
	}

	void ResetListOptions(CCtrlClc *)
	{
		m_clc.SetHideEmptyGroups(true);
		m_clc.SetHideOfflineRoot(true);
	}

	void InviteUser(char *pUser, char *text)
	{
		XmlNode msg("message");
		TiXmlElement *invite = msg << XATTR("to", m_room) << XATTRID(m_proto->SerialNext())
			<< XCHILDNS("x", JABBER_FEAT_MUC_USER)
			<< XCHILD("invite") << XATTR("to", pUser);
		if (text)
			invite << XCHILD("reason", text);

		m_proto->m_ThreadInfo->send(msg);
	}

public:
	CGroupchatInviteDlg(CJabberProto *ppro, const char *room) :
		CSuper(ppro, IDD_GROUPCHAT_INVITE),
		m_newJids(1),
		m_btnInvite(this, IDC_INVITE),
		m_txtNewJid(this, IDC_NEWJID),
		m_btnAddJid(this, IDC_ADDJID, g_plugin.getIcon(IDI_ADDCONTACT), "Add"),
		m_txtReason(this, IDC_REASON),
		m_clc(this, IDC_CLIST)
	{
		m_room = mir_strdup(room);
		m_btnAddJid.OnClick = Callback(this, &CGroupchatInviteDlg::OnCommand_AddJid);
		m_btnInvite.OnClick = Callback(this, &CGroupchatInviteDlg::OnCommand_Invite);
		m_clc.OnNewContact =
			m_clc.OnListRebuilt = Callback(this, &CGroupchatInviteDlg::FilterList);
		m_clc.OnOptionsChanged = Callback(this, &CGroupchatInviteDlg::ResetListOptions);
	}

	~CGroupchatInviteDlg()
	{
		for (auto &it : m_newJids)
			mir_free(it);
		mir_free(m_room);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		SetDlgItemText(m_hwnd, IDC_HEADERBAR, CMStringW(FORMAT, TranslateT("Invite Users to\n%s"), Utf2T(m_room).get()));
		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_GROUP));

		SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_HIDEOFFLINE | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
		ResetListOptions(&m_clc);
		FilterList(&m_clc);
		return true;
	}

	void OnCommand_AddJid(CCtrlButton*)
	{
		ptrA szJid(m_txtNewJid.GetTextU());
		m_txtNewJid.SetTextA("");

		MCONTACT hContact = m_proto->HContactFromJID(szJid);
		if (hContact) {
			int hItem = SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem)
				SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, hItem, 1);
			return;
		}

		if (FindJid(szJid))
			return;

		JabberGcLogInviteDlgJidData *jidData = (JabberGcLogInviteDlgJidData *)mir_alloc(sizeof(JabberGcLogInviteDlgJidData));
		mir_strcpy(jidData->jid, szJid);
		
		CMStringW msg(FORMAT, TranslateT("%s (not on roster)"), Utf2T(szJid).get());
		CLCINFOITEM cii = { 0 };
		cii.cbSize = sizeof(cii);
		cii.flags = CLCIIF_CHECKBOX;
		cii.pszText = msg;
		jidData->hItem = m_clc.AddInfoItem(&cii);
		m_clc.SetCheck(jidData->hItem, 1);
		m_newJids.insert(jidData);
	}

	void OnCommand_Invite(CCtrlButton*)
	{
		if (!m_room) return;

		T2Utf text(ptrW(m_txtReason.GetText()));

		// invite users from roster
		for (auto &hContact : m_proto->AccContacts()) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (HANDLE hItem = m_clc.FindContact(hContact)) {
				if (m_clc.GetCheck(hItem)) {
					ptrA jid(m_proto->getUStringA(hContact, "jid"));
					if (jid != nullptr)
						InviteUser(jid, text);
				}
			}
		}

		// invite others
		for (auto &it : m_newJids)
			if (m_clc.GetCheck(it->hItem))
				InviteUser(it->jid, text);

		Close();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu processing

void CJabberProto::AdminSet(const char *to, const char *ns, const char *szItem, const char *itemVal, const char *var, const char *varVal)
{
	m_ThreadInfo->send(XmlNodeIq("set", SerialNext(), to) << XQUERY(ns) << XCHILD("item") << XATTR(szItem, itemVal) << XATTR(var, varVal));
}

void CJabberProto::AdminSetReason(const char *to, const char *ns, const char *szItem, const char *itemVal, const char *var, const char *varVal, const char *rsn)
{
	m_ThreadInfo->send(XmlNodeIq("set", SerialNext(), to) << XQUERY(ns) << XCHILD("item") << XATTR(szItem, itemVal) << XATTR(var, varVal) << XCHILD("reason", rsn));
}

void CJabberProto::AdminGet(const char *to, const char *ns, const char *var, const char *varVal, JABBER_IQ_HANDLER foo, void *pInfo)
{
	m_ThreadInfo->send(XmlNodeIq(AddIQ(foo, JABBER_IQ_TYPE_GET, to, pInfo))
		<< XQUERY(ns) << XCHILD("item") << XATTR(var, varVal));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Member info dialog

class CMemberInfoDlg : public CJabberDlgBase
{
	JABBER_LIST_ITEM *m_item;
	JABBER_RESOURCE_STATUS *m_me, *m_him;

	CCtrlCombo cmbRole, cmbAffiliation;
	CCtrlMButton btnRole, btnAffiliation;

public:
	CMemberInfoDlg(CJabberProto *ppro, JABBER_LIST_ITEM *item, JABBER_RESOURCE_STATUS *me, JABBER_RESOURCE_STATUS *him) :
		CJabberDlgBase(ppro, IDD_GROUPCHAT_INFO),
		m_item(item),
		m_me(me),
		m_him(him),
		cmbRole(this, IDC_TXT_ROLE),
		btnRole(this, IDC_BTN_ROLE, SKINICON_EVENT_FILE, "Apply"),
		cmbAffiliation(this, IDC_TXT_AFFILIATION),
		btnAffiliation(this, IDC_BTN_AFFILIATION, SKINICON_EVENT_FILE, "Apply")
	{
		cmbRole.OnSelChanged = Callback(this, &CMemberInfoDlg::onSelChange_Role);
		cmbAffiliation.OnSelChanged = Callback(this, &CMemberInfoDlg::onSelChange_Affiliation);

		btnRole.OnClick = Callback(this, &CMemberInfoDlg::onClick_Role);
		btnAffiliation.OnClick = Callback(this, &CMemberInfoDlg::onClick_Affiliation);
	}

	bool OnInitDialog() override
	{
		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_GROUP));

		LOGFONT lf;
		GetObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_NICK, WM_GETFONT, 0, 0), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		HFONT hfnt = CreateFontIndirect(&lf);
		SendDlgItemMessage(m_hwnd, IDC_TXT_NICK, WM_SETFONT, (WPARAM)hfnt, TRUE);

		btnRole.MakeFlat();
		btnAffiliation.MakeFlat();

		SendDlgItemMessage(m_hwnd, IDC_ICO_STATUS, STM_SETICON, (WPARAM)Skin_LoadProtoIcon(m_proto->m_szModuleName, m_him->m_iStatus), 0);

		char buf[256];
		mir_snprintf(buf, TranslateU("%s from\n%s"), m_him->m_szResourceName.get(), m_item->jid);
		SetDlgItemTextUtf(m_hwnd, IDC_HEADERBAR, buf);

		SetDlgItemTextUtf(m_hwnd, IDC_TXT_NICK, m_him->m_szResourceName);
		SetDlgItemTextUtf(m_hwnd, IDC_TXT_JID, m_him->m_szRealJid ? m_him->m_szRealJid : TranslateU("Real JID not available"));
		SetDlgItemTextUtf(m_hwnd, IDC_TXT_STATUS, m_him->m_szStatusMessage);

		for (auto &it : sttRoleItems)
			if ((it.value == m_him->m_role) || it.check(m_me, m_him)) {
				int idx = cmbRole.AddString(TranslateW(it.pwszTitle), it.value);
				if (it.value == m_him->m_role)
					cmbRole.SetCurSel(idx);
			}

		for (auto &it : sttAffiliationItems)
			if ((it.value == m_him->m_affiliation) || it.check(m_me, m_him)) {
				int idx = cmbAffiliation.AddString(TranslateW(it.pwszTitle), it.value);
				if (it.value == m_him->m_affiliation)
					cmbAffiliation.SetCurSel(idx);
			}

		btnRole.Disable();
		btnAffiliation.Disable();
		return true;
	}

	void OnDestroy() override
	{
		Window_FreeIcon_IcoLib(m_hwnd);
		IcoLib_ReleaseIcon((HICON)btnRole.SendMsg(BM_SETIMAGE, IMAGE_ICON, 0));
		IcoLib_ReleaseIcon((HICON)btnAffiliation.SendMsg(BM_SETIMAGE, IMAGE_ICON, 0));
	}

	void onSelChange_Affiliation(CCtrlCombo*)
	{
		int value = cmbAffiliation.GetCurData();
		btnAffiliation.Enable(m_him->m_affiliation != value);
	}

	void onClick_Affiliation(CCtrlButton *)
	{
		int value = cmbAffiliation.GetCurData();
		if (m_him->m_affiliation == value)
			return;

		char szBareJid[JABBER_MAX_JID_LEN];
		JabberStripJid(m_him->m_szRealJid, szBareJid, _countof(szBareJid));

		switch (value) {
		case AFFILIATION_NONE:
			if (m_him->m_szRealJid)
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "none");
			else
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "affiliation", "none");
			break;
		case AFFILIATION_MEMBER:
			if (m_him->m_szRealJid)
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "member");
			else
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "affiliation", "member");
			break;
		case AFFILIATION_ADMIN:
			if (m_him->m_szRealJid)
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "admin");
			else
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "affiliation", "admin");
			break;
		case AFFILIATION_OWNER:
			if (m_him->m_szRealJid)
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "owner");
			else
				m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "affiliation", "owner");
		}
	}

	void onSelChange_Role(CCtrlCombo *)
	{
		int value = cmbRole.GetCurData();
		btnRole.Enable(m_him->m_role != value);
	}

	void onClick_Role(CCtrlButton*)
	{
		int value = cmbRole.GetCurData();
		if (m_him->m_role == value)
			return;

		switch (value) {
		case ROLE_VISITOR:
			m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "role", "visitor");
			break;
		case ROLE_PARTICIPANT:
			m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "role", "participant");
			break;
		case ROLE_MODERATOR:
			m_proto->AdminSet(m_item->jid, JABBER_FEAT_MUC_ADMIN, "nick", m_him->m_szResourceName, "role", "moderator");
			break;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Nick list command processor

static void sttNickListHook(CJabberProto *ppro, JABBER_LIST_ITEM *item, GCHOOK* gch)
{
	pResourceStatus me(item->findResource(item->nick)), him(item->findResource(T2Utf(gch->ptszUID)));
	if (him == nullptr || me == nullptr)
		return;

	// 1 kick per second, prevents crashes...
	enum { BAN_KICK_INTERVAL = 1000 };
	static uint32_t dwLastBanKickTime = 0;

	CMStringW szBuffer, szTitle;

	if ((gch->dwData >= CLISTMENUIDMIN) && (gch->dwData <= CLISTMENUIDMAX)) {
		if (him->m_szRealJid && *him->m_szRealJid)
			if (MCONTACT hContact = ppro->HContactFromJID(him->m_szRealJid))
				Clist_MenuProcessCommand(gch->dwData, MPCF_CONTACTMENU, hContact);
		return;
	}

	switch (gch->dwData) {
	case IDM_SLAP:
		if (ppro->m_bJabberOnline) {
			ptrA szMessage(ppro->getUStringA("GcMsgSlap"));
			if (szMessage == nullptr)
				szMessage = mir_strdup(TranslateU(JABBER_GC_MSG_SLAP));

			CMStringA buf;
			// do not use snprintf to avoid possible problems with % symbol
			if (char *p = strstr(szMessage, "%s")) {
				*p = 0;
				buf.Format("%s%s%s", szMessage.get(), him->m_szResourceName.get(), p + 2);
			}
			else buf = szMessage;
			buf.Replace("%%", "%");

			ppro->m_ThreadInfo->send(
				XmlNode("message") << XATTR("to", item->jid) << XATTR("type", "groupchat")
				<< XCHILD("body", buf));
		}
		break;

	case IDM_VCARD:
		{
			CMStringA jid(MakeJid(item->jid, him->m_szResourceName));

			MCONTACT hContact = ppro->AddToListByJID(jid, PALF_TEMPORARY);
			ppro->setUString(hContact, "Nick", him->m_szResourceName);
			
			JABBER_LIST_ITEM *pTmp = ppro->ListAdd(LIST_VCARD_TEMP, jid, hContact);
			ppro->ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_szStatusMessage, him->m_iPriority);

			pTmp->findResource(him->m_szResourceName)->m_pCaps = ppro->ListGetItemPtr(LIST_CHATROOM, item->jid)->findResource(him->m_szResourceName)->m_pCaps;

			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case IDM_INFO:
		(new CMemberInfoDlg(ppro, item, me, him))->Show();
		break;

	case IDM_KICK:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			dwLastBanKickTime = GetTickCount();
			szBuffer.Format(L"%s: ", Utf2T(me->m_szResourceName).get());
			szTitle.Format(TranslateT("Reason to kick %s"), Utf2T(him->m_szResourceName).get());
			char *resourceName_copy = mir_strdup(him->m_szResourceName); // copy resource name to prevent possible crash if user list rebuilds
			if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
				ppro->m_ThreadInfo->send(
				XmlNodeIq("set", ppro->SerialNext(), item->jid) << XQUERY(JABBER_FEAT_MUC_ADMIN)
				<< XCHILD("item") << XATTR("nick", resourceName_copy) << XATTR("role", "none")
				<< XCHILD("reason", T2Utf(szBuffer)));

			mir_free(resourceName_copy);
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_SET_VISITOR:
		if (him->m_role != ROLE_VISITOR)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "role", "visitor");
		break;

	case IDM_SET_PARTICIPANT:
		if (him->m_role != ROLE_PARTICIPANT)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "role", "participant");
		break;

	case IDM_SET_MODERATOR:
		if (him->m_role != ROLE_MODERATOR)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "role", "moderator");
		break;

	case IDM_SET_NONE:
		if (him->m_affiliation != AFFILIATION_NONE) {
			if (him->m_szRealJid) {
				char szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_szRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "none");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "affiliation", "none");
		}
		break;

	case IDM_SET_MEMBER:
		if (him->m_affiliation != AFFILIATION_MEMBER) {
			if (him->m_szRealJid) {
				char szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_szRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "member");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "affiliation", "member");
		}
		break;

	case IDM_SET_ADMIN:
		if (him->m_affiliation != AFFILIATION_ADMIN) {
			if (him->m_szRealJid) {
				char szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_szRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "admin");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "affiliation", "admin");
		}
		break;

	case IDM_SET_OWNER:
		if (him->m_affiliation != AFFILIATION_OWNER) {
			if (him->m_szRealJid) {
				char szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_szRealJid, szBareJid, _countof(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "jid", szBareJid, "affiliation", "owner");
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, "nick", him->m_szResourceName, "affiliation", "owner");
		}
		break;

	case IDM_SET_BAN:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			if (him->m_szRealJid && *him->m_szRealJid) {
				char szVictimBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_szRealJid, szVictimBareJid, _countof(szVictimBareJid));

				szBuffer.Format(L"%s: ", Utf2T(me->m_szResourceName).get());
				szTitle.Format(TranslateT("Reason to ban %s"), Utf2T(him->m_szResourceName).get());

				if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
					ppro->m_ThreadInfo->send(
						XmlNodeIq("set", ppro->SerialNext(), item->jid) << XQUERY(JABBER_FEAT_MUC_ADMIN)
						<< XCHILD("item") << XATTR("jid", szVictimBareJid) << XATTR("affiliation", "outcast")
						<< XCHILD("reason", T2Utf(szBuffer)));
			}
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_LINK0: case IDM_LINK1: case IDM_LINK2: case IDM_LINK3: case IDM_LINK4:
	case IDM_LINK5: case IDM_LINK6: case IDM_LINK7: case IDM_LINK8: case IDM_LINK9:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			// copy resource name to prevent possible crash if user list rebuilds
			char *resourceName_copy(NEWSTR_ALLOCA(him->m_szResourceName));

			char *szInviteTo = nullptr;
			int idx = gch->dwData - IDM_LINK0;
			LISTFOREACH(i, ppro, LIST_CHATROOM)
			{
				if (JABBER_LIST_ITEM *pItem = ppro->ListGetItemPtrFromIndex(i)) {
					if (!pItem->si) continue;
					if (!idx--) {
						szInviteTo = pItem->jid;
						break;
					}
				}
			}

			if (!szInviteTo) break;

			szTitle.Format(TranslateT("Invite %s to %s"), Utf2T(him->m_szResourceName).get(), Utf2T(szInviteTo).get());
			if (!ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE))
				break;

			XmlNode msg("message");
			msg << XATTR("to", MakeJid(item->jid, resourceName_copy)) << XATTRID(ppro->SerialNext())
				<< XCHILD("x", T2Utf(szBuffer)) << XATTR("xmlns", JABBER_FEAT_DIRECT_MUC_INVITE) << XATTR("jid", szInviteTo)
				<< XCHILD("invite") << XATTR("from", item->nick);
			ppro->m_ThreadInfo->send(msg);
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_CPY_NICK:
		JabberCopyText(g_clistApi.hwndContactList, him->m_szResourceName);
		break;

	case IDM_RJID_COPY:
	case IDM_CPY_RJID:
		JabberCopyText(g_clistApi.hwndContactList, him->m_szRealJid);
		break;

	case IDM_CPY_INROOMJID:
		JabberCopyText(g_clistApi.hwndContactList, MakeJid(item->jid, him->m_szResourceName));
		break;

	case IDM_RJID_VCARD:
		if (him->m_szRealJid && *him->m_szRealJid) {
			char *jid = NEWSTR_ALLOCA(him->m_szRealJid);
			if (char *tmp = strchr(jid, '/'))
				*tmp = 0;

			MCONTACT hContact = ppro->AddToListByJID(jid, PALF_TEMPORARY);
			ppro->ListAdd(LIST_VCARD_TEMP, jid, hContact);
			ppro->ListAddResource(LIST_VCARD_TEMP, jid, him->m_iStatus, him->m_szStatusMessage, him->m_iPriority);

			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case IDM_RJID_ADD:
		if (him->m_szRealJid && *him->m_szRealJid) {
			Utf2T jid(him->m_szRealJid);
			PROTOSEARCHRESULT psr = {};
			psr.flags = PSR_UNICODE;
			psr.cbSize = sizeof(psr);
			psr.id.w = jid;
			if (auto *tmp = wcschr(psr.id.w, '/'))
				*tmp = 0;
			psr.nick.w = psr.id.w;
			Contact::AddBySearch(ppro->m_szModuleName, &psr, g_clistApi.hwndContactList);
		}
		break;
	}
}

static void sttLogListHook(CJabberProto *ppro, JABBER_LIST_ITEM *item, GCHOOK *gch)
{
	CMStringW szBuffer, szTitle;
	T2Utf roomJid(gch->si->ptszID);

	switch (gch->dwData) {
	case IDM_LST_PARTICIPANT:
		ppro->AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "role", "participant", &CJabberProto::OnIqResultMucGetVoiceList, gch->si->pDlg);
		break;

	case IDM_LST_MEMBER:
		ppro->AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "member", &CJabberProto::OnIqResultMucGetMemberList, gch->si->pDlg);
		break;

	case IDM_LST_MODERATOR:
		ppro->AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "role", "moderator", &CJabberProto::OnIqResultMucGetModeratorList, gch->si->pDlg);
		break;

	case IDM_LST_BAN:
		ppro->AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "outcast", &CJabberProto::OnIqResultMucGetBanList, gch->si->pDlg);
		break;

	case IDM_LST_ADMIN:
		ppro->AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "admin", &CJabberProto::OnIqResultMucGetAdminList, gch->si->pDlg);
		break;

	case IDM_LST_OWNER:
		ppro->AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "owner", &CJabberProto::OnIqResultMucGetOwnerList, gch->si->pDlg);
		break;

	case IDM_AVATAR:
		if (CallService(MS_AV_SETAVATARW, gch->si->hContact, 0) == 1) {
			CMStringW wszAvaPath(db_get_wsm(gch->si->hContact, "ContactPhoto", "File"));
			XmlNodeIq iq(ppro->AddIQ(nullptr, JABBER_IQ_TYPE_SET, roomJid));
			
			TiXmlElement *v = iq << XCHILDNS("vCard", JABBER_FEAT_VCARD_TEMP);
			ppro->AppendPhotoToVcard(v, true, wszAvaPath.GetBuffer(), gch->si->hContact);
			ppro->m_ThreadInfo->send(iq);

			wchar_t szAvatarName[MAX_PATH];
			ppro->GetAvatarFileName(gch->si->hContact, szAvatarName, _countof(szAvatarName));
			CallService(MS_AV_SETAVATARW, gch->si->hContact, (LPARAM)szAvatarName);
		}
		break;

	case IDM_TOPIC:
		szTitle.Format(TranslateT("Set topic for %s"), gch->si->ptszID);
		szBuffer = Utf2T(item->getTemp()->m_szStatusMessage);
		szBuffer.Replace(L"\n", L"\r\n");
		if (ppro->EnterString(szBuffer, szTitle, ESF_RICHEDIT, "gcTopic_"))
			ppro->m_ThreadInfo->send(
				XmlNode("message") << XATTR("to", roomJid) << XATTR("type", "groupchat") << XCHILD("subject", T2Utf(szBuffer)));
		break;

	case IDM_NICK:
		szTitle.Format(TranslateT("Change nickname in %s"), gch->si->ptszID);
		if (item->nick)
			szBuffer = Utf2T(item->nick);
		if (ppro->EnterString(szBuffer, szTitle, ESF_COMBO, "gcNick_"))
			if (ppro->ListGetItemPtr(LIST_CHATROOM, roomJid) != nullptr)
				ppro->SendPresenceTo(ppro->m_iStatus == ID_STATUS_INVISIBLE ? ID_STATUS_ONLINE : ppro->m_iStatus, MakeJid(roomJid, T2Utf(szBuffer)));
		break;

	case IDM_INVITE:
		{
			auto *pDlg = new CGroupchatInviteDlg(ppro, roomJid);
			if (gch->si->pDlg)
				pDlg->SetParent(gch->si->pDlg->GetHwnd());
			pDlg->Show();
		}
		break;

	case IDM_CONFIG:
		ppro->m_ThreadInfo->send(
			XmlNodeIq(ppro->AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, roomJid))
			<< XQUERY(JABBER_FEAT_MUC_OWNER));
		break;

	case IDM_BOOKMARKS:
		item = ppro->ListGetItemPtr(LIST_BOOKMARK, roomJid);
		if (item == nullptr) {
			item = ppro->ListGetItemPtr(LIST_CHATROOM, roomJid);
			if (item != nullptr) {
				replaceStr(item->type, "conference");
				MCONTACT hContact = ppro->HContactFromJID(item->jid);
				item->name = Clist_GetContactDisplayName(hContact);
				ppro->AddEditBookmark(item);
			}
		}
		break;

	case IDM_DESTROY:
		szTitle.Format(TranslateT("Reason to destroy %s"), gch->si->ptszID);
		if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
			ppro->m_ThreadInfo->send(
				XmlNodeIq("set", ppro->SerialNext(), roomJid) << XQUERY(JABBER_FEAT_MUC_OWNER)
				<< XCHILD("destroy") << XCHILD("reason", T2Utf(szBuffer)));
		__fallthrough;

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
			for (char *p = strstr(item->getTemp()->m_szStatusMessage, "http://"); p && *p; p = strstr(p + 1, "http://")) {
				if (idx == gch->dwData) {
					char *bufPtr, *url = mir_strdup(p);
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
		JabberCopyText(g_clistApi.hwndContactList, item->jid);
		break;

	case IDM_CPY_TOPIC:
		JabberCopyText(g_clistApi.hwndContactList, item->getTemp()->m_szStatusMessage);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a private message to a chat user

static void sttSendPrivateMessage(CJabberProto *ppro, JABBER_LIST_ITEM *item, const char *nick)
{
	MCONTACT hContact = ppro->DBCreateContact(MakeJid(item->jid, nick), nick, true, false);
	if (hContact != 0) {
		pResourceStatus r(item->findResource(nick));
		if (r)
			ppro->setWord(hContact, "Status", r->m_iStatus);

		Contact::Hide(hContact);
		ppro->setUString(hContact, "Nick", nick);
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

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	T2Utf roomJid(gch->si->ptszID);
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, roomJid);
	if (item == nullptr)
		return 1;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);
			Chat_UnescapeTags(gch->ptszText);
			GroupchatSendMsg(item, T2Utf(gch->ptszText));
		}
		break;

	case GC_USER_PRIVMESS:
		sttSendPrivateMessage(this, item, T2Utf(gch->ptszUID));
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

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const char *str, const char *rsn)
{
	const char *field = (jidListInfo->type == MUC_BANLIST || strchr(str, '@')) ? "jid" : "nick";
	char *roomJid = jidListInfo->roomJid;
	if (jidListInfo->type == MUC_BANLIST) {
		AdminSetReason(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "affiliation", "outcast", rsn);
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "outcast", &CJabberProto::OnIqResultMucGetBanList);
	}
}

void CJabberProto::AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const char *str)
{
	const char *field = (jidListInfo->type == MUC_BANLIST || strchr(str, '@')) ? "jid" : "nick";
	char *roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "role", "participant");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "role", "participant", &CJabberProto::OnIqResultMucGetVoiceList);
		break;
	case MUC_MEMBERLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "affiliation", "member");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "member", &CJabberProto::OnIqResultMucGetMemberList);
		break;
	case MUC_MODERATORLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "role", "moderator");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "role", "moderator", &CJabberProto::OnIqResultMucGetModeratorList);
		break;
	case MUC_BANLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "affiliation", "outcast");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "outcast", &CJabberProto::OnIqResultMucGetBanList);
		break;
	case MUC_ADMINLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "affiliation", "admin");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "admin", &CJabberProto::OnIqResultMucGetAdminList);
		break;
	case MUC_OWNERLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, "affiliation", "owner");
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, "affiliation", "owner", &CJabberProto::OnIqResultMucGetOwnerList);
		break;
	}
}

void CJabberProto::DeleteMucListItem(JABBER_MUC_JIDLIST_INFO *jidListInfo, const char *jid)
{
	switch (jidListInfo->type) {
	case MUC_VOICELIST:		// change role to visitor (from participant)
		AdminSet(jidListInfo->roomJid, JABBER_FEAT_MUC_ADMIN, "jid", jid, "role", "visitor");
		break;
	case MUC_BANLIST:		// change affiliation to none (from outcast)
	case MUC_MEMBERLIST:	// change affiliation to none (from member)
		AdminSet(jidListInfo->roomJid, JABBER_FEAT_MUC_ADMIN, "jid", jid, "affiliation", "none");
		break;
	case MUC_MODERATORLIST:	// change role to participant (from moderator)
		AdminSet(jidListInfo->roomJid, JABBER_FEAT_MUC_ADMIN, "jid", jid, "role", "participant");
		break;
	case MUC_ADMINLIST:		// change affiliation to member (from admin)
		AdminSet(jidListInfo->roomJid, JABBER_FEAT_MUC_ADMIN, "jid", jid, "affiliation", "member");
		break;
	case MUC_OWNERLIST:		// change affiliation to admin (from owner)
		AdminSet(jidListInfo->roomJid, JABBER_FEAT_MUC_ADMIN, "jid", jid, "affiliation", "admin");
		break;
	}
}
