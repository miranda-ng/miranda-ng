/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2012-14  Miranda NG project

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

#include "jabber.h"
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
	TCHAR *title_en;
	int min_role;
	int min_affiliation;

	TCHAR *title;

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
		this->title = TranslateTS(this->title_en);
	}
};

static TRoleOrAffiliationInfo sttAffiliationItems[] =
{
	{ AFFILIATION_NONE,   IDM_SET_NONE,   LPGENT("None"),   ROLE_NONE, AFFILIATION_ADMIN },
	{ AFFILIATION_MEMBER, IDM_SET_MEMBER, LPGENT("Member"), ROLE_NONE, AFFILIATION_ADMIN },
	{ AFFILIATION_ADMIN,  IDM_SET_ADMIN,  LPGENT("Admin"),  ROLE_NONE, AFFILIATION_OWNER },
	{ AFFILIATION_OWNER,  IDM_SET_OWNER,  LPGENT("Owner"),  ROLE_NONE, AFFILIATION_OWNER },
};

static TRoleOrAffiliationInfo sttRoleItems[] =
{
	{ ROLE_VISITOR,     IDM_SET_VISITOR,     LPGENT("Visitor"),     ROLE_MODERATOR, AFFILIATION_NONE  },
	{ ROLE_PARTICIPANT, IDM_SET_PARTICIPANT, LPGENT("Participant"), ROLE_MODERATOR, AFFILIATION_NONE  },
	{ ROLE_MODERATOR,   IDM_SET_MODERATOR,   LPGENT("Moderator"),   ROLE_MODERATOR, AFFILIATION_ADMIN },
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGcInit - initializes the new chat

static const TCHAR *sttStatuses[] = { LPGENT("Visitors"), LPGENT("Participants"), LPGENT("Moderators"), LPGENT("Owners") };

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
	int i;

	// translate string for menus (this can't be done in initializer)
	for (i=0; i < SIZEOF(sttAffiliationItems); i++) sttAffiliationItems[i].translate();
	for (i=0; i < SIZEOF(sttRoleItems); i++) sttRoleItems[i].translate();

	ptrT szNick(JabberNickFromJID(item->jid));

	GCSESSION gcw = { sizeof(GCSESSION) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = szNick;
	gcw.ptszID = item->jid;
	CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw);

	MCONTACT hContact = HContactFromJID(item->jid);
	if (hContact != NULL) {
		if (JABBER_LIST_ITEM *bookmark = ListGetItemPtr(LIST_BOOKMARK, item->jid))
			if (bookmark->name) {
				ptrT myHandle( db_get_tsa(hContact, "CList", "MyHandle"));
				if (myHandle == NULL)
					db_set_ts(hContact, "CList", "MyHandle", bookmark->name);
			}

		ptrT tszNick( getTStringA(hContact, "MyNick"));
		if (tszNick != NULL) {
			if (!lstrcmp(tszNick, szNick))
				delSetting(hContact, "MyNick");
			else
				setTString(hContact, "MyNick", item->nick);
		}
		else setTString(hContact, "MyNick", item->nick);

		ptrT passw( getTStringA(hContact, "Password"));
		if (lstrcmp_null(passw, item->password)) {
			if (!item->password || !item->password[0])
				delSetting(hContact, "Password");
			else
				setTString(hContact, "Password", item->password);
		}
	}

	item->bChatActive = TRUE;

	GCDEST gcd = { m_szModuleName, item->jid, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	for (i = SIZEOF(sttStatuses)-1; i >= 0; i--) {
		gce.ptszStatus = TranslateTS(sttStatuses[i]);
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, (item->bAutoJoin && m_options.AutoJoinHidden) ? WINDOW_HIDDEN : SESSION_INITDONE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	return 0;
}

void CJabberProto::GcLogCreate(JABBER_LIST_ITEM *item)
{
	if (item->bChatActive)
		return;

	GcInit(item);
}

void CJabberProto::GcLogShowInformation(JABBER_LIST_ITEM *item, pResourceStatus &user, TJabberGcLogInfoType type)
{
	if (!item || !user || (item->bChatActive != 2)) return;

	CMString buf;

	switch (type) {
	case INFO_BAN:
		if (m_options.GcLogBans)
			buf.Format(TranslateT("User %s is now banned."), user->m_tszResourceName);
		break;

	case INFO_STATUS:
		if (m_options.GcLogStatuses) {
			TCHAR *ptszDescr = pcli->pfnGetStatusModeDescription(user->m_iStatus, 0);
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
			TCHAR *name = NULL;
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
			TCHAR *name = NULL;
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
		buf.Replace(_T("%"), _T("%%"));

		GCDEST gcd = { m_szModuleName, item->jid, GC_EVENT_INFORMATION };
		GCEVENT gce = { sizeof(gce), &gcd };
		gce.ptszNick = user->m_tszResourceName;
		gce.ptszUID = user->m_tszResourceName;
		gce.ptszText = buf;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.time = time(0);
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}
}

void CJabberProto::GcLogUpdateMemberStatus(JABBER_LIST_ITEM *item, const TCHAR *resource, const TCHAR *nick, const TCHAR *jid, int action, HXML reason, int nStatusCode)
{
	int statusToSet = 0;

	const TCHAR *szReason = xmlGetText(reason);
	if (szReason == NULL) {
		if (nStatusCode == 322)
			szReason = TranslateT("because room is now members-only");
		else if (nStatusCode == 301)
			szReason = TranslateT("user banned");
	}

	ptrT myNick(mir_tstrdup(item->nick));
	if (myNick == NULL)
		myNick = JabberNickFromJID(m_szJabberJID);

	GCDEST gcd = { m_szModuleName, item->jid };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszNick = nick;
	gce.ptszUID = resource;
	if (jid != NULL)
		gce.ptszUserInfo = jid;
	gce.ptszText = szReason;
	if (item->bChatActive == 2) {
		gce.dwFlags |= GCEF_ADDTOLOG;
		gce.time = time(0);
	}

	switch (gcd.iType = action) {
	case GC_EVENT_PART:  break;
	case GC_EVENT_KICK:
		gce.ptszStatus = TranslateT("Moderator");
		break;
	default:
		mir_cslock lck(m_csLists);
		for (int i = 0; i < item->arResources.getCount(); i++) {
			JABBER_RESOURCE_STATUS *JS = item->arResources[i];
			if (!lstrcmp(resource, JS->m_tszResourceName)) {
				if (action != GC_EVENT_JOIN) {
					switch (action) {
					case 0:
						gcd.iType = GC_EVENT_ADDSTATUS;
					case GC_EVENT_REMOVESTATUS:
						gce.dwFlags &= ~GCEF_ADDTOLOG;
					}
					gce.ptszText = TranslateT("Moderator");
				}
				gce.ptszStatus = TranslateTS(sttStatuses[JabberGcGetStatus(JS)]);
				gce.bIsMe = (lstrcmp(nick, myNick) == 0);
				statusToSet = JS->m_iStatus;
				break;
	}	}	}

	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

	if (statusToSet != 0) {
		gce.ptszText = nick;
		if (statusToSet == ID_STATUS_AWAY || statusToSet == ID_STATUS_NA || statusToSet == ID_STATUS_DND)
			gce.dwItemData = 3;
		else
			gce.dwItemData = 1;
		gcd.iType = GC_EVENT_SETSTATUSEX;
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

		gce.ptszUID = resource;
		gce.dwItemData = statusToSet;
		gcd.iType = GC_EVENT_SETCONTACTSTATUS;
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}
}

void CJabberProto::GcQuit(JABBER_LIST_ITEM *item, int code, HXML reason)
{
	TCHAR *szMessage = NULL;

	if (code != 307 && code != 301) {
		ptrT tszMessage(getTStringA("GcMsgQuit"));
		if (tszMessage != NULL)
			szMessage = NEWTSTR_ALLOCA(tszMessage);
		else
			szMessage = TranslateTS(JABBER_GC_MSG_QUIT);
	}
	else {
		ptrT myNick(JabberNickFromJID(m_szJabberJID));
		GcLogUpdateMemberStatus(item, myNick, myNick, NULL, GC_EVENT_KICK, reason);
	}

	GCDEST gcd = { m_szModuleName, item->jid, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszUID = item->jid;
	gce.ptszText = xmlGetText(reason);
	CallServiceSync(MS_GC_EVENT, (code == 200) ? SESSION_TERMINATE : SESSION_OFFLINE, (LPARAM)&gce);

	db_unset(HContactFromJID(item->jid), "CList", "Hidden");
	item->bChatActive = FALSE;

	if (m_bJabberOnline) {
		TCHAR szPresenceTo[JABBER_MAX_JID_LEN];
		mir_sntprintf(szPresenceTo, SIZEOF(szPresenceTo), _T("%s/%s"), item->jid, item->nick);

		m_ThreadInfo->send(
			XmlNode(_T("presence")) << XATTR(_T("to"), szPresenceTo) << XATTR(_T("type"), _T("unavailable"))
			<< XCHILD(_T("status"), szMessage));

		ListRemove(LIST_CHATROOM, item->jid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu hooks

static gc_item *sttFindGcMenuItem(GCMENUITEMS *items, DWORD id)
{
	for (int i=0; i < items->nItems; i++)
		if (items->Item[i].dwID == id)
			return items->Item + i;
	return NULL;
}

static void sttSetupGcMenuItem(GCMENUITEMS *items, DWORD id, bool disabled)
{
	for (int i=0; i < items->nItems; i++)
		if (!id || (items->Item[i].dwID == id))
			items->Item[i].bDisabled = disabled;
}

static void sttShowGcMenuItem(GCMENUITEMS *items, DWORD id, int type)
{
	for (int i=0; i < items->nItems; i++)
		if (!id || (items->Item[i].dwID == id))
			items->Item[i].uType = type;
}

static void sttSetupGcMenuItems(GCMENUITEMS *items, DWORD *ids, bool disabled)
{
	for (; *ids; ++ids)
		sttSetupGcMenuItem(items, *ids, disabled);
}

static void sttShowGcMenuItems(GCMENUITEMS *items, DWORD *ids, int type)
{
	for (; *ids; ++ids)
		sttShowGcMenuItem(items, *ids, type);
}

static gc_item sttLogListItems[] =
{
	{ LPGENT("Change &nickname"),     IDM_NICK,               MENU_ITEM           },
   { LPGENT("&Invite a user"),       IDM_INVITE,             MENU_ITEM           },
   { NULL,                           0,                      MENU_SEPARATOR      },

   { LPGENT("&Roles"),               IDM_ROLE,               MENU_NEWPOPUP       },
   { LPGENT("&Participant list"),    IDM_LST_PARTICIPANT,    MENU_POPUPITEM      },
   { LPGENT("&Moderator list"),      IDM_LST_MODERATOR,      MENU_POPUPITEM      },

   { LPGENT("&Affiliations"),        IDM_AFFLTN,             MENU_NEWPOPUP       },
   { LPGENT("&Member list"),         IDM_LST_MEMBER,         MENU_POPUPITEM      },
   { LPGENT("&Admin list"),          IDM_LST_ADMIN,          MENU_POPUPITEM      },
   { LPGENT("&Owner list"),          IDM_LST_OWNER,          MENU_POPUPITEM      },
   { NULL,                           0,                      MENU_POPUPSEPARATOR },
   { LPGENT("Outcast list (&ban)"),  IDM_LST_BAN,            MENU_POPUPITEM      },

   { LPGENT("&Room options"),        0,                      MENU_NEWPOPUP       },
   { LPGENT("View/change &topic"),   IDM_TOPIC,              MENU_POPUPITEM      },
   { LPGENT("Add to &bookmarks"),    IDM_BOOKMARKS,          MENU_POPUPITEM      },
   { LPGENT("&Configure..."),        IDM_CONFIG,             MENU_POPUPITEM      },
   { LPGENT("&Destroy room"),        IDM_DESTROY,            MENU_POPUPITEM      },

   { NULL,                           0,                      MENU_SEPARATOR      },

   { LPGENT("Lin&ks"),               0,                      MENU_NEWPOPUP       },
   { NULL,                           IDM_LINK0,              0                   },
   { NULL,                           IDM_LINK1,              0                   },
   { NULL,                           IDM_LINK2,              0                   },
   { NULL,                           IDM_LINK3,              0                   },
   { NULL,                           IDM_LINK4,              0                   },
   { NULL,                           IDM_LINK5,              0                   },
   { NULL,                           IDM_LINK6,              0                   },
   { NULL,                           IDM_LINK7,              0                   },
   { NULL,                           IDM_LINK8,              0                   },
   { NULL,                           IDM_LINK9,              0                   },

   { LPGENT("Copy room &JID"),       IDM_CPY_RJID,           MENU_ITEM           },
   { LPGENT("Copy room topic"),      IDM_CPY_TOPIC,          MENU_ITEM           },
   { NULL,                           0,                      MENU_SEPARATOR      },

   { LPGENT("&Send presence"),       0,                      MENU_NEWPOPUP},
   { LPGENT("Online"),               IDM_PRESENCE_ONLINE,    MENU_POPUPITEM      },
   { LPGENT("Away"),                 IDM_PRESENCE_AWAY,      MENU_POPUPITEM      },
   { LPGENT("NA"),                   IDM_PRESENCE_NA,        MENU_POPUPITEM      },
   { LPGENT("DND"),                  IDM_PRESENCE_DND,       MENU_POPUPITEM      },
   { LPGENT("Free for chat"),        IDM_PRESENCE_FREE4CHAT, MENU_POPUPITEM      },

   { LPGENT("&Leave chat session"),  IDM_LEAVE,              MENU_ITEM           }
};

static TCHAR sttRJidBuf[JABBER_MAX_JID_LEN] = {0};
static gc_item sttListItems[] =
{
	{ LPGENT("&Slap"),                IDM_SLAP,               MENU_ITEM           },   // 0
	{ LPGENT("&User details"),        IDM_VCARD,              MENU_ITEM           },   // 1
	{ LPGENT("Member &info"),         IDM_INFO,               MENU_ITEM           },   // 2

	{ sttRJidBuf,                     0,                      MENU_NEWPOPUP       },   // 3 -> accessed explicitly by index!!!
	{ LPGENT("User &details"),        IDM_RJID_VCARD,         MENU_POPUPITEM      },
	{ LPGENT("&Add to roster"),       IDM_RJID_ADD,           MENU_POPUPITEM      },
	{ LPGENT("&Copy to clipboard"),   IDM_RJID_COPY,          MENU_POPUPITEM      },

	{ LPGENT("Invite to room"),       0,                      MENU_NEWPOPUP       },
	{ NULL,                           IDM_LINK0,              0                   },
	{ NULL,                           IDM_LINK1,              0                   },
	{ NULL,                           IDM_LINK2,              0                   },
	{ NULL,                           IDM_LINK3,              0                   },
	{ NULL,                           IDM_LINK4,              0                   },
	{ NULL,                           IDM_LINK5,              0                   },
	{ NULL,                           IDM_LINK6,              0                   },
	{ NULL,                           IDM_LINK7,              0                   },
	{ NULL,                           IDM_LINK8,              0                   },
	{ NULL,                           IDM_LINK9,              0                   },

	{ NULL,                           0,                      MENU_SEPARATOR      },

	{ LPGENT("Set &role"),            IDM_ROLE,               MENU_NEWPOPUP       },
	{ LPGENT("&Visitor"),             IDM_SET_VISITOR,        MENU_POPUPITEM      },
	{ LPGENT("&Participant"),         IDM_SET_PARTICIPANT,    MENU_POPUPITEM      },
	{ LPGENT("&Moderator"),           IDM_SET_MODERATOR,      MENU_POPUPITEM      },

	{ LPGENT("Set &affiliation"),     IDM_AFFLTN,             MENU_NEWPOPUP       },
	{ LPGENT("&None"),                IDM_SET_NONE,           MENU_POPUPITEM      },
	{ LPGENT("&Member"),              IDM_SET_MEMBER,         MENU_POPUPITEM      },
	{ LPGENT("&Admin"),               IDM_SET_ADMIN,          MENU_POPUPITEM      },
	{ LPGENT("&Owner"),               IDM_SET_OWNER,          MENU_POPUPITEM      },
	{ NULL,                           0,                      MENU_POPUPSEPARATOR },
	{ LPGENT("Outcast (&ban)"),       IDM_SET_BAN,            MENU_POPUPITEM      },

	{ LPGENT("&Kick"),                IDM_KICK,               MENU_ITEM           },
	{ NULL,                           0,                      MENU_SEPARATOR      },
	{ LPGENT("Copy &nickname"),       IDM_CPY_NICK,           MENU_ITEM           },
	{ LPGENT("Copy real &JID"),       IDM_CPY_RJID,           MENU_ITEM           },
	{ LPGENT("Copy in-room JID"),     IDM_CPY_INROOMJID,      MENU_ITEM           }
};

int CJabberProto::JabberGcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == NULL)
		return 0;

	if (lstrcmpiA(gcmi->pszModule, m_szModuleName))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, gcmi->pszID);
	if (item == NULL)
		return 0;

	pResourceStatus me(NULL), him(NULL);
	for (int i=0; i < item->arResources.getCount(); i++) {
		JABBER_RESOURCE_STATUS *p = item->arResources[i];
		if (!lstrcmp(p->m_tszResourceName, item->nick))   me = p;
		if (!lstrcmp(p->m_tszResourceName, gcmi->pszUID)) him = p;
	}

	if (gcmi->Type == MENU_ON_LOG) {
		static TCHAR url_buf[1024] = {0};

		gcmi->nItems = SIZEOF(sttLogListItems);
		gcmi->Item = sttLogListItems;

		static DWORD sttModeratorItems[] = { IDM_LST_PARTICIPANT, 0 };
		static DWORD sttAdminItems[] = { IDM_LST_MODERATOR, IDM_LST_MEMBER, IDM_LST_ADMIN, IDM_LST_OWNER, IDM_LST_BAN, 0 };
		static DWORD sttOwnerItems[] = { IDM_CONFIG, IDM_DESTROY, 0 };

		sttSetupGcMenuItem(gcmi, 0, FALSE);

		int idx = IDM_LINK0;
		TCHAR *ptszStatusMsg = item->getTemp()->m_tszStatusMessage;
		if (ptszStatusMsg && *ptszStatusMsg) {
			TCHAR *bufPtr = url_buf;
			for (TCHAR *p = _tcsstr(ptszStatusMsg, _T("http://")); p && *p; p = _tcsstr(p+1, _T("http://"))) {
				lstrcpyn(bufPtr, p, SIZEOF(url_buf) - (bufPtr - url_buf));
				gc_item *pItem = sttFindGcMenuItem(gcmi, idx);
				pItem->pszDesc = bufPtr;
				pItem->uType = MENU_POPUPITEM;
				for (; *bufPtr && !_istspace(*bufPtr); ++bufPtr) ;
				*bufPtr++ = 0;

				if (++idx > IDM_LINK9)
					break;
			}
		}
		for (; idx <= IDM_LINK9; ++idx)
			sttFindGcMenuItem(gcmi, idx)->uType = 0;

		if (!GetAsyncKeyState(VK_CONTROL)) {
			if (me) {
				sttSetupGcMenuItems(gcmi, sttModeratorItems, (me->m_role < ROLE_MODERATOR));
				sttSetupGcMenuItems(gcmi, sttAdminItems, (me->m_affiliation < AFFILIATION_ADMIN));
				sttSetupGcMenuItems(gcmi, sttOwnerItems, (me->m_affiliation < AFFILIATION_OWNER));
			}
			if (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)
				sttSetupGcMenuItem(gcmi, IDM_BOOKMARKS, FALSE);
		}
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		gcmi->nItems = SIZEOF(sttListItems);
		gcmi->Item = sttListItems;

		static DWORD sttRJidItems[] = { IDM_RJID_VCARD, IDM_RJID_ADD, IDM_RJID_COPY, 0 };

		if (me && him) {
			int i, idx;
			BOOL force = GetAsyncKeyState(VK_CONTROL);
			sttSetupGcMenuItem(gcmi, 0, FALSE);

			idx = IDM_LINK0;
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
				if (item = ListGetItemPtrFromIndex(i)) {
					gc_item *pItem = sttFindGcMenuItem(gcmi, idx);
					pItem->pszDesc = item->jid;
					pItem->uType = MENU_POPUPITEM;
					if (++idx > IDM_LINK9) break;
				}

			for (; idx <= IDM_LINK9; ++idx)
				sttFindGcMenuItem(gcmi, idx)->uType = 0;

			for (i=0; i < SIZEOF(sttAffiliationItems); i++) {
				gc_item *item = sttFindGcMenuItem(gcmi, sttAffiliationItems[i].id);
				item->uType = (him->m_affiliation == sttAffiliationItems[i].value) ? MENU_POPUPCHECK : MENU_POPUPITEM;
				item->bDisabled = !(force || sttAffiliationItems[i].check(me, him));
			}

			for (i=0; i < SIZEOF(sttRoleItems); i++) {
				gc_item *item = sttFindGcMenuItem(gcmi, sttRoleItems[i].id);
				item->uType = (him->m_role == sttRoleItems[i].value) ? MENU_POPUPCHECK : MENU_POPUPITEM;
				item->bDisabled = !(force || sttRoleItems[i].check(me, him));
			}

			if (him->m_tszRealJid && *him->m_tszRealJid) {
				mir_sntprintf(sttRJidBuf, SIZEOF(sttRJidBuf), TranslateT("Real &JID: %s"), him->m_tszRealJid);
				if (TCHAR *tmp = _tcschr(sttRJidBuf, _T('/'))) *tmp = 0;

				if (MCONTACT hContact = HContactFromJID(him->m_tszRealJid)) {
					gcmi->Item[3].uType = MENU_HMENU;
					gcmi->Item[3].dwID = CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
					sttShowGcMenuItems(gcmi, sttRJidItems, 0);
				}
				else {
					gcmi->Item[3].uType = MENU_NEWPOPUP;
					sttShowGcMenuItems(gcmi, sttRJidItems, MENU_POPUPITEM);
				}

				sttSetupGcMenuItem(gcmi, IDM_CPY_RJID, FALSE);
			}
			else {
				gcmi->Item[3].uType = 0;
				sttShowGcMenuItems(gcmi, sttRJidItems, 0);

				sttSetupGcMenuItem(gcmi, IDM_CPY_RJID, TRUE);
			}

			if (!force) {
				if (me->m_role < ROLE_MODERATOR || (me->m_affiliation <= him->m_affiliation))
					sttSetupGcMenuItem(gcmi, IDM_KICK, TRUE);

				if ((me->m_affiliation < AFFILIATION_ADMIN) ||
					(me->m_affiliation == AFFILIATION_ADMIN) && (me->m_affiliation <= him->m_affiliation))
					sttSetupGcMenuItem(gcmi, IDM_SET_BAN, TRUE);
			}
		}
		else {
			sttSetupGcMenuItem(gcmi, 0, TRUE);
			gcmi->Item[2].uType = 0;
			sttShowGcMenuItems(gcmi, sttRJidItems, 0);
		}
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
		TCHAR jid[JABBER_MAX_JID_LEN];
	};

	LIST<JabberGcLogInviteDlgJidData> m_newJids;
	TCHAR *m_room;

	CCtrlButton  m_btnInvite;
	CCtrlEdit    m_txtNewJid;
	CCtrlMButton m_btnAddJid;
	CCtrlEdit    m_txtReason;
	CCtrlClc     m_clc;

	void FilterList(CCtrlClc *)
	{
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			char *proto = GetContactProto(hContact);
			if (lstrcmpA(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					m_clc.DeleteItem(hItem);
		}
	}

	void ResetListOptions(CCtrlClc *)
	{
		m_clc.SetBkBitmap(0, NULL);
		m_clc.SetBkColor(GetSysColor(COLOR_WINDOW));
		m_clc.SetGreyoutFlags(0);
		m_clc.SetLeftMargin(4);
		m_clc.SetIndent(10);
		m_clc.SetHideEmptyGroups(1);
		m_clc.SetHideOfflineRoot(1);
		for (int i=0; i <= FONTID_MAX; i++)
			m_clc.SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
	}

	void InviteUser(TCHAR *pUser, TCHAR *text)
	{
		XmlNode msg(_T("message"));
		HXML invite = msg << XATTR(_T("to"), m_room) << XATTRID(m_proto->SerialNext())
			<< XCHILDNS(_T("x"), JABBER_FEAT_MUC_USER)
				<< XCHILD(_T("invite")) << XATTR(_T("to"), pUser);
		if (text)
			invite << XCHILD(_T("reason"), text);

		m_proto->m_ThreadInfo->send(msg);
	}

public:
	CGroupchatInviteDlg(CJabberProto *ppro, const TCHAR *room) :
		CSuper(ppro, IDD_GROUPCHAT_INVITE, NULL),
		m_newJids(1),
		m_btnInvite(this, IDC_INVITE),
		m_txtNewJid(this, IDC_NEWJID),
		m_btnAddJid(this, IDC_ADDJID, ppro->LoadIconEx("addroster"), "Add"),
		m_txtReason(this, IDC_REASON),
		m_clc(this, IDC_CLIST)
	{
		m_room = mir_tstrdup(room);
		m_btnAddJid.OnClick = Callback(this, &CGroupchatInviteDlg::OnCommand_AddJid);
		m_btnInvite.OnClick = Callback(this, &CGroupchatInviteDlg::OnCommand_Invite);
		m_clc.OnNewContact = 
			m_clc.OnListRebuilt = Callback(this, &CGroupchatInviteDlg::FilterList);
		m_clc.OnOptionsChanged = Callback(this, &CGroupchatInviteDlg::ResetListOptions);
	}

	~CGroupchatInviteDlg()
	{
		for (int i=0; i < m_newJids.getCount(); i++)
			mir_free(m_newJids[i]);
		mir_free(m_room);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		SetDlgItemText(m_hwnd, IDC_HEADERBAR, CMString(FORMAT, TranslateT("%s\nSend group chat invitation."), m_room));
		WindowSetIcon(m_hwnd, m_proto, "group");

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE,
			GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE)|CLS_HIDEOFFLINE|CLS_CHECKBOXES|CLS_HIDEEMPTYGROUPS|CLS_USEGROUPS|CLS_GREYALTERNATE|CLS_GROUPCHECKBOXES);
		SendMessage(GetDlgItem(m_hwnd, IDC_CLIST), CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP|CLS_EX_TRACKSELECT, 0);
		ResetListOptions(&m_clc);
		FilterList(&m_clc);
	}

	void OnCommand_AddJid(CCtrlButton*)
	{
		TCHAR buf[JABBER_MAX_JID_LEN];
		m_txtNewJid.GetText(buf, SIZEOF(buf));
		m_txtNewJid.SetTextA("");

		MCONTACT hContact = m_proto->HContactFromJID(buf);
		if (hContact)
		{
			int hItem = SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem)
				SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, hItem, 1);
			return;
		}

		int i;
		for (i=0; i < m_newJids.getCount(); i++)
			if (!lstrcmp(m_newJids[i]->jid, buf))
				break;
		if (i != m_newJids.getCount())
			return;

		JabberGcLogInviteDlgJidData *jidData = (JabberGcLogInviteDlgJidData *)mir_alloc(sizeof(JabberGcLogInviteDlgJidData));
		lstrcpy(jidData->jid, buf);
		CLCINFOITEM cii = { 0 };
		cii.cbSize = sizeof(cii);
		cii.flags = CLCIIF_CHECKBOX;
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("%s (not on roster)"), jidData->jid);
		cii.pszText = buf;
		jidData->hItem = SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, jidData->hItem, 1);
		m_newJids.insert(jidData);
	}

	void OnCommand_Invite(CCtrlButton*)
	{
		if (!m_room) return;

		TCHAR *text = m_txtReason.GetText();
		HWND hwndList = GetDlgItem(m_hwnd, IDC_CLIST);

		// invite users from roster
		for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (int hItem = SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0)) {
				if (SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
					ptrT jid(m_proto->getTStringA(hContact, "jid"));
					if (jid != NULL)
						InviteUser(jid, text);
				}
			}
		}

		// invite others
		for (int i = 0; i < m_newJids.getCount(); i++)
		if (SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)m_newJids[i]->hItem, 0))
			InviteUser(m_newJids[i]->jid, text);

		mir_free(text);
		Close();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Context menu processing

void CJabberProto::AdminSet(const TCHAR *to, const TCHAR *ns, const TCHAR *szItem, const TCHAR *itemVal, const TCHAR *var, const TCHAR *varVal)
{
	m_ThreadInfo->send(XmlNodeIq(_T("set"), SerialNext(), to) << XQUERY(ns) << XCHILD(_T("item")) << XATTR(szItem, itemVal) << XATTR(var, varVal));
}

void CJabberProto::AdminSetReason(const TCHAR *to, const TCHAR *ns, const TCHAR *szItem, const TCHAR *itemVal, const TCHAR *var, const TCHAR *varVal, const TCHAR *rsn)
{
	m_ThreadInfo->send(XmlNodeIq(_T("set"), SerialNext(), to) << XQUERY(ns) << XCHILD(_T("item")) << XATTR(szItem, itemVal) << XATTR(var, varVal) << XCHILD(_T("reason"), rsn));
}

void CJabberProto::AdminGet(const TCHAR *to, const TCHAR *ns, const TCHAR *var, const TCHAR *varVal, JABBER_IQ_HANDLER foo)
{
	m_ThreadInfo->send(XmlNodeIq(AddIQ(foo, JABBER_IQ_TYPE_GET, to))
		<< XQUERY(ns) << XCHILD(_T("item")) << XATTR(var, varVal));
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
	int value, i, idx;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		dat = (TUserInfoData *)lParam;

		WindowSetIcon(hwndDlg, dat->ppro, "group");
		{
			LOGFONT lf;
			GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_TXT_NICK, WM_GETFONT, 0, 0), sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			HFONT hfnt = CreateFontIndirect(&lf);
			SendDlgItemMessage(hwndDlg, IDC_TXT_NICK, WM_SETFONT, (WPARAM)hfnt, TRUE);
		}

		SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_FILE));
		SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BUTTONADDTOOLTIP, (WPARAM)"Apply", 0);

		SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_FILE));
		SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BUTTONADDTOOLTIP, (WPARAM)"Apply", 0);

		SendDlgItemMessage(hwndDlg, IDC_ICO_STATUS, STM_SETICON, (WPARAM)LoadSkinnedProtoIcon(dat->ppro->m_szModuleName, dat->him->m_iStatus), 0);

		TCHAR buf[256];
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Member Info: %s"), dat->him->m_tszResourceName);
		SetWindowText(hwndDlg, buf);

		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Member Information\n%s from %s"), dat->him->m_tszResourceName, dat->item->jid);
		SetDlgItemText(hwndDlg, IDC_HEADERBAR, buf);

		SetDlgItemText(hwndDlg, IDC_TXT_NICK, dat->him->m_tszResourceName);
		SetDlgItemText(hwndDlg, IDC_TXT_JID, dat->him->m_tszRealJid ? dat->him->m_tszRealJid : TranslateT("Real JID not available"));
		SetDlgItemText(hwndDlg, IDC_TXT_STATUS, dat->him->m_tszStatusMessage);

		for (i = 0; i < SIZEOF(sttRoleItems); i++) {
			if ((sttRoleItems[i].value == dat->him->m_role) || sttRoleItems[i].check(dat->me, dat->him)) {
				SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_SETITEMDATA,
					idx = SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_ADDSTRING, 0, (LPARAM)sttRoleItems[i].title),
					sttRoleItems[i].value);
				if (sttRoleItems[i].value == dat->him->m_role)
					SendDlgItemMessage(hwndDlg, IDC_TXT_ROLE, CB_SETCURSEL, idx, 0);
			}
		}
		for (i = 0; i < SIZEOF(sttAffiliationItems); i++) {
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

			TCHAR szBareJid[JABBER_MAX_JID_LEN];
			JabberStripJid(dat->him->m_tszRealJid, szBareJid, SIZEOF(szBareJid));

			switch (value) {
			case AFFILIATION_NONE:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("none"));
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("affiliation"), _T("none"));
				break;
			case AFFILIATION_MEMBER:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("member"));
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("affiliation"), _T("member"));
				break;
			case AFFILIATION_ADMIN:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("admin"));
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("affiliation"), _T("admin"));
				break;
			case AFFILIATION_OWNER:
				if (dat->him->m_tszRealJid)
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("owner"));
				else
					dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("affiliation"), _T("owner"));
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
				dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("role"), _T("visitor"));
				break;
			case ROLE_PARTICIPANT:
				dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("role"), _T("participant"));
				break;
			case ROLE_MODERATOR:
				dat->ppro->AdminSet(dat->item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), dat->him->m_tszResourceName, _T("role"), _T("moderator"));
				break;
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		WindowFreeIcon(hwndDlg);
		g_ReleaseIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_BTN_AFFILIATION, BM_SETIMAGE, IMAGE_ICON, 0));
		g_ReleaseIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_BTN_ROLE, BM_SETIMAGE, IMAGE_ICON, 0));
		TUserInfoData *dat = (TUserInfoData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
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
	if (him == NULL || me == NULL)
		return;

	// 1 kick per second, prevents crashes...
	enum { BAN_KICK_INTERVAL = 1000 };
	static DWORD dwLastBanKickTime = 0;

	CMString szBuffer, szTitle;

	if ((gch->dwData >= CLISTMENUIDMIN) && (gch->dwData <= CLISTMENUIDMAX)) {
		if (him->m_tszRealJid && *him->m_tszRealJid)
			if (MCONTACT hContact = ppro->HContactFromJID(him->m_tszRealJid))
				CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(gch->dwData, MPCF_CONTACTMENU), hContact);
		return;
	}

	switch (gch->dwData) {
	case IDM_SLAP:
		if (ppro->m_bJabberOnline) {
			ptrT szMessage(ppro->getTStringA("GcMsgSlap"));
			if (szMessage == NULL)
				szMessage = mir_tstrdup(TranslateTS(JABBER_GC_MSG_SLAP));

			TCHAR buf[256];
			// do not use snprintf to avoid possible problems with % symbol
			if (TCHAR *p = _tcsstr(szMessage, _T("%s"))) {
				*p = 0;
				mir_sntprintf(buf, SIZEOF(buf), _T("%s%s%s"), szMessage, him->m_tszResourceName, p + 2);
			}
			else lstrcpyn(buf, szMessage, SIZEOF(buf));
			UnEscapeChatTags(buf);

			ppro->m_ThreadInfo->send(
				XmlNode(_T("message")) << XATTR(_T("to"), item->jid) << XATTR(_T("type"), _T("groupchat"))
				<< XCHILD(_T("body"), buf));
		}
		break;

	case IDM_VCARD:
		{
			JABBER_SEARCH_RESULT jsr = { 0 };
			mir_sntprintf(jsr.jid, SIZEOF(jsr.jid), _T("%s/%s"), item->jid, him->m_tszResourceName);
			jsr.hdr.cbSize = sizeof(JABBER_SEARCH_RESULT);

			JABBER_LIST_ITEM *item = ppro->ListAdd(LIST_VCARD_TEMP, jsr.jid);
			ppro->ListAddResource(LIST_VCARD_TEMP, jsr.jid, him->m_iStatus, him->m_tszStatusMessage, him->m_iPriority);

			MCONTACT hContact = (MCONTACT)CallProtoService(ppro->m_szModuleName, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&jsr);
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
			HWND hwndInfo = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_GROUPCHAT_INFO), NULL, sttUserInfoDlgProc, (LPARAM)dat);
			ShowWindow(hwndInfo, SW_SHOW);
		}
		break;

	case IDM_KICK:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			dwLastBanKickTime = GetTickCount();
			szBuffer.Format(_T("%s: "), me->m_tszResourceName);
			szTitle.Format(TranslateT("Reason to kick %s"), him->m_tszResourceName);
			TCHAR *resourceName_copy = mir_tstrdup(him->m_tszResourceName); // copy resource name to prevent possible crash if user list rebuilds
			if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
				ppro->m_ThreadInfo->send(
					XmlNodeIq(_T("set"), ppro->SerialNext(), item->jid) << XQUERY(JABBER_FEAT_MUC_ADMIN)
						<< XCHILD(_T("item")) << XATTR(_T("nick"), resourceName_copy) << XATTR(_T("role"), _T("none"))
						<< XCHILD(_T("reason"), szBuffer));

			mir_free(resourceName_copy);
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_SET_VISITOR:
		if (him->m_role != ROLE_VISITOR)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("role"), _T("visitor"));
		break;

	case IDM_SET_PARTICIPANT:
		if (him->m_role != ROLE_PARTICIPANT)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("role"), _T("participant"));
		break;

	case IDM_SET_MODERATOR:
		if (him->m_role != ROLE_MODERATOR)
			ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("role"), _T("moderator"));
		break;

	case IDM_SET_NONE:
		if (him->m_affiliation != AFFILIATION_NONE) {
			if (him->m_tszRealJid) {
				TCHAR szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, SIZEOF(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("none"));
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("affiliation"), _T("none"));
		}
		break;

	case IDM_SET_MEMBER:
		if (him->m_affiliation != AFFILIATION_MEMBER) {
			if (him->m_tszRealJid) {
				TCHAR szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, SIZEOF(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("member"));
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("affiliation"), _T("member"));
		}
		break;

	case IDM_SET_ADMIN:
		if (him->m_affiliation != AFFILIATION_ADMIN) {
			if (him->m_tszRealJid) {
				TCHAR szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, SIZEOF(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("admin"));
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("affiliation"), _T("admin"));
		}
		break;

	case IDM_SET_OWNER:
		if (him->m_affiliation != AFFILIATION_OWNER) {
			if (him->m_tszRealJid) {
				TCHAR szBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szBareJid, SIZEOF(szBareJid));
				ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("jid"), szBareJid, _T("affiliation"), _T("owner"));
			}
			else ppro->AdminSet(item->jid, JABBER_FEAT_MUC_ADMIN, _T("nick"), him->m_tszResourceName, _T("affiliation"), _T("owner"));
		}
		break;

	case IDM_SET_BAN:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			if (him->m_tszRealJid && *him->m_tszRealJid) {
				TCHAR szVictimBareJid[JABBER_MAX_JID_LEN];
				JabberStripJid(him->m_tszRealJid, szVictimBareJid, SIZEOF(szVictimBareJid));

				szBuffer.Format(_T("%s: "), me->m_tszResourceName);
				szTitle.Format(TranslateT("Reason to ban %s"), him->m_tszResourceName);

				if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
					ppro->m_ThreadInfo->send(
						XmlNodeIq(_T("set"), ppro->SerialNext(), item->jid) << XQUERY(JABBER_FEAT_MUC_ADMIN)
							<< XCHILD(_T("item")) << XATTR(_T("jid"), szVictimBareJid) << XATTR(_T("affiliation"), _T("outcast"))
							<< XCHILD(_T("reason"), szBuffer));
			}
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_LINK0: case IDM_LINK1: case IDM_LINK2: case IDM_LINK3: case IDM_LINK4:
	case IDM_LINK5: case IDM_LINK6: case IDM_LINK7: case IDM_LINK8: case IDM_LINK9:
		if ((GetTickCount() - dwLastBanKickTime) > BAN_KICK_INTERVAL) {
			TCHAR *resourceName_copy = NEWTSTR_ALLOCA(him->m_tszResourceName); // copy resource name to prevent possible crash if user list rebuilds

			TCHAR *szInviteTo = 0;
			int idx = gch->dwData - IDM_LINK0;
			LISTFOREACH(i, ppro, LIST_CHATROOM)
			{
				if (JABBER_LIST_ITEM *item = ppro->ListGetItemPtrFromIndex(i))
					if (!idx--) {
						szInviteTo = item->jid;
						break;
					}
			}

			if (!szInviteTo) break;

			szTitle.Format(TranslateT("Invite %s to %s"), him->m_tszResourceName, szInviteTo);
			if (!ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE))
				break;

			szTitle.Format(_T("%s/%s"), item->jid, resourceName_copy);

			XmlNode msg(_T("message"));
			HXML invite = msg << XATTR(_T("to"), szTitle) << XATTRID(ppro->SerialNext())
				<< XCHILD(_T("x"), szBuffer)
					<< XATTR(_T("xmlns"), JABBER_FEAT_DIRECT_MUC_INVITE)
					<< XATTR(_T("jid"), szInviteTo)
						<< XCHILD(_T("invite")) << XATTR(_T("from"), item->nick);
			ppro->m_ThreadInfo->send(msg);
		}
		dwLastBanKickTime = GetTickCount();
		break;

	case IDM_CPY_NICK:
		JabberCopyText((HWND)CallService(MS_CLUI_GETHWND, 0, 0), him->m_tszResourceName);
		break;

	case IDM_RJID_COPY:
	case IDM_CPY_RJID:
		JabberCopyText((HWND)CallService(MS_CLUI_GETHWND, 0, 0), him->m_tszRealJid);
		break;

	case IDM_CPY_INROOMJID:
		szBuffer.Format(_T("%s/%s"), item->jid, him->m_tszResourceName);
		JabberCopyText((HWND)CallService(MS_CLUI_GETHWND, 0, 0), szBuffer);
		break;

	case IDM_RJID_VCARD:
		if (him->m_tszRealJid && *him->m_tszRealJid) {
			MCONTACT hContact;
			JABBER_SEARCH_RESULT jsr = { 0 };
			jsr.hdr.cbSize = sizeof(JABBER_SEARCH_RESULT);
			mir_sntprintf(jsr.jid, SIZEOF(jsr.jid), _T("%s"), him->m_tszRealJid);
			if (TCHAR *tmp = _tcschr(jsr.jid, _T('/'))) *tmp = 0;

			JABBER_LIST_ITEM *item = ppro->ListAdd(LIST_VCARD_TEMP, jsr.jid);
			ppro->ListAddResource(LIST_VCARD_TEMP, jsr.jid, him->m_iStatus, him->m_tszStatusMessage, him->m_iPriority);

			hContact = (MCONTACT)CallProtoService(ppro->m_szModuleName, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&jsr);
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;

	case IDM_RJID_ADD:
		if (him->m_tszRealJid && *him->m_tszRealJid) {
			JABBER_SEARCH_RESULT jsr = { 0 };
			jsr.hdr.cbSize = sizeof(JABBER_SEARCH_RESULT);
			jsr.hdr.flags = PSR_TCHAR;
			mir_sntprintf(jsr.jid, SIZEOF(jsr.jid), _T("%s"), him->m_tszRealJid);
			if (TCHAR *tmp = _tcschr(jsr.jid, _T('/'))) *tmp = 0;
			jsr.hdr.nick = jsr.jid;

			ADDCONTACTSTRUCT acs = { 0 };
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = ppro->m_szModuleName;
			acs.psr = (PROTOSEARCHRESULT *)&jsr;
			CallService(MS_ADDCONTACT_SHOW, (WPARAM)CallService(MS_CLUI_GETHWND, 0, 0), (LPARAM)&acs);
		}
		break;
	}
}

static void sttLogListHook(CJabberProto *ppro, JABBER_LIST_ITEM *item, GCHOOK* gch)
{
	CMString szBuffer, szTitle;

	switch (gch->dwData) {
	case IDM_LST_PARTICIPANT:
		ppro->AdminGet(gch->pDest->ptszID, JABBER_FEAT_MUC_ADMIN, _T("role"), _T("participant"), &CJabberProto::OnIqResultMucGetVoiceList);
		break;

	case IDM_LST_MEMBER:
		ppro->AdminGet(gch->pDest->ptszID, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("member"), &CJabberProto::OnIqResultMucGetMemberList);
		break;

	case IDM_LST_MODERATOR:
		ppro->AdminGet(gch->pDest->ptszID, JABBER_FEAT_MUC_ADMIN, _T("role"), _T("moderator"), &CJabberProto::OnIqResultMucGetModeratorList);
		break;

	case IDM_LST_BAN:
		ppro->AdminGet(gch->pDest->ptszID, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("outcast"), &CJabberProto::OnIqResultMucGetBanList);
		break;

	case IDM_LST_ADMIN:
		ppro->AdminGet(gch->pDest->ptszID, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("admin"), &CJabberProto::OnIqResultMucGetAdminList);
		break;

	case IDM_LST_OWNER:
		ppro->AdminGet(gch->pDest->ptszID, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("owner"), &CJabberProto::OnIqResultMucGetOwnerList);
		break;

	case IDM_TOPIC:
		szTitle.Format(TranslateT("Set topic for %s"), gch->pDest->ptszID);
		szBuffer = item->getTemp()->m_tszStatusMessage;
		szBuffer.Replace(_T("\n"), _T("\r\n"));
		if (ppro->EnterString(szBuffer, szTitle, ESF_RICHEDIT, "gcTopic_"))
			ppro->m_ThreadInfo->send(
				XmlNode(_T("message")) << XATTR(_T("to"), gch->pDest->ptszID) << XATTR(_T("type"), _T("groupchat"))
					<< XCHILD(_T("subject"), szBuffer));
		break;

	case IDM_NICK:
		szTitle.Format(TranslateT("Change nickname in %s"), gch->pDest->ptszID);
		if (item->nick)
			szBuffer = item->nick;
		if (ppro->EnterString(szBuffer, szTitle, ESF_COMBO, "gcNick_")) {
			JABBER_LIST_ITEM *item = ppro->ListGetItemPtr(LIST_CHATROOM, gch->pDest->ptszID);
			if (item != NULL) {
				TCHAR text[1024];
				mir_sntprintf(text, SIZEOF(text), _T("%s/%s"), gch->pDest->ptszID, szBuffer);
				ppro->SendPresenceTo(ppro->m_iStatus == ID_STATUS_INVISIBLE ? ID_STATUS_ONLINE : ppro->m_iStatus, text, NULL);
			}
		}
		break;

	case IDM_INVITE:
		(new CGroupchatInviteDlg(ppro, gch->pDest->ptszID))->Show();
		break;

	case IDM_CONFIG:
		ppro->m_ThreadInfo->send(
			XmlNodeIq(ppro->AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, gch->pDest->ptszID))
				<< XQUERY(JABBER_FEAT_MUC_OWNER));
		break;

	case IDM_BOOKMARKS:
	{
		JABBER_LIST_ITEM *item = ppro->ListGetItemPtr(LIST_BOOKMARK, gch->pDest->ptszID);
		if (item == NULL) {
			item = ppro->ListGetItemPtr(LIST_CHATROOM, gch->pDest->ptszID);
			if (item != NULL) {
				item->type = _T("conference");
				MCONTACT hContact = ppro->HContactFromJID(item->jid);
				item->name = pcli->pfnGetContactDisplayName(hContact, 0);
				ppro->AddEditBookmark(item);
			}
		}
		break;
	}
	case IDM_DESTROY:
		szTitle.Format(TranslateT("Reason to destroy %s"), gch->pDest->ptszID);
		if (ppro->EnterString(szBuffer, szTitle, ESF_MULTILINE, "gcReason_"))
			ppro->m_ThreadInfo->send(
				XmlNodeIq(_T("set"), ppro->SerialNext(), gch->pDest->ptszID) << XQUERY(JABBER_FEAT_MUC_OWNER)
					<< XCHILD(_T("destroy")) << XCHILD(_T("reason"), szBuffer));

	case IDM_LEAVE:
		ppro->GcQuit(item, 200, NULL);
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
		unsigned idx = IDM_LINK0;
		for (TCHAR *p = _tcsstr(item->getTemp()->m_tszStatusMessage, _T("http://")); p && *p; p = _tcsstr(p+1, _T("http://"))) {
			if (idx == gch->dwData) {
				char *bufPtr, *url = mir_t2a(p);
				for (bufPtr = url; *bufPtr && !isspace(*bufPtr); ++bufPtr) ;
				*bufPtr++ = 0;
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)url);
				mir_free(url);
				break;
			}

			if (++idx > IDM_LINK9) break;
		}

		break;
	}

	case IDM_CPY_RJID:
		JabberCopyText((HWND)CallService(MS_CLUI_GETHWND, 0, 0), item->jid);
		break;

	case IDM_CPY_TOPIC:
		JabberCopyText((HWND)CallService(MS_CLUI_GETHWND, 0, 0), item->getTemp()->m_tszStatusMessage);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sends a private message to a chat user

static void sttSendPrivateMessage(CJabberProto *ppro, JABBER_LIST_ITEM *item, const TCHAR *nick)
{
	TCHAR szFullJid[JABBER_MAX_JID_LEN];
	mir_sntprintf(szFullJid, SIZEOF(szFullJid), _T("%s/%s"), item->jid, nick);
	MCONTACT hContact = ppro->DBCreateContact(szFullJid, NULL, TRUE, FALSE);
	if (hContact != NULL) {
		pResourceStatus r(item->findResource(nick));
		if (r)
			ppro->setWord(hContact, "Status", r->m_iStatus);

		db_set_b(hContact, "CList", "Hidden", 1);
		ppro->setTString(hContact, "Nick", nick);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// General chat event processing hook

int CJabberProto::JabberGcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK* gch = (GCHOOK*)lParam;
	if (gch == NULL)
		return 0;

	if (lstrcmpiA(gch->pDest->pszModule, m_szModuleName))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, gch->pDest->ptszID);
	if (item == NULL)
		return 0;

	switch (gch->pDest->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && lstrlen(gch->ptszText) > 0) {
			rtrimt(gch->ptszText);

			if (m_bJabberOnline) {
				TCHAR *buf = NEWTSTR_ALLOCA(gch->ptszText);
				UnEscapeChatTags(buf);
				m_ThreadInfo->send(
					XmlNode(_T("message")) << XATTR(_T("to"), item->jid) << XATTR(_T("type"), _T("groupchat"))
						<< XCHILD(_T("body"), buf));
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

void CJabberProto::AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const TCHAR* str, const TCHAR* rsn)
{
	const TCHAR *field = (jidListInfo->type == MUC_BANLIST || _tcschr(str, '@')) ? _T("jid") : _T("nick");
	TCHAR *roomJid = jidListInfo->roomJid;
	if (jidListInfo->type == MUC_BANLIST) {
		AdminSetReason(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("affiliation"), _T("outcast"), rsn);
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("outcast"), &CJabberProto::OnIqResultMucGetBanList);
	}
}

void CJabberProto::AddMucListItem(JABBER_MUC_JIDLIST_INFO* jidListInfo, const TCHAR* str)
{
	const TCHAR *field = (jidListInfo->type == MUC_BANLIST || _tcschr(str, '@')) ? _T("jid") : _T("nick");
	TCHAR *roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("role"), _T("participant"));
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("role"), _T("participant"), &CJabberProto::OnIqResultMucGetVoiceList);
		break;
	case MUC_MEMBERLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("affiliation"), _T("member"));
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("member"), &CJabberProto::OnIqResultMucGetMemberList);
		break;
	case MUC_MODERATORLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("role"), _T("moderator"));
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("role"), _T("moderator"), &CJabberProto::OnIqResultMucGetModeratorList);
		break;
	case MUC_BANLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("affiliation"), _T("outcast"));
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("outcast"), &CJabberProto::OnIqResultMucGetBanList);
		break;
	case MUC_ADMINLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("affiliation"), _T("admin"));
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("admin"), &CJabberProto::OnIqResultMucGetAdminList);
		break;
	case MUC_OWNERLIST:
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, field, str, _T("affiliation"), _T("owner"));
		AdminGet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("affiliation"), _T("owner"), &CJabberProto::OnIqResultMucGetOwnerList);
		break;
	}
}

void CJabberProto::DeleteMucListItem(JABBER_MUC_JIDLIST_INFO *jidListInfo, const TCHAR *jid)
{
	TCHAR *roomJid = jidListInfo->roomJid;

	switch (jidListInfo->type) {
	case MUC_VOICELIST:		// change role to visitor (from participant)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("jid"), jid, _T("role"), _T("visitor"));
		break;
	case MUC_BANLIST:		// change affiliation to none (from outcast)
	case MUC_MEMBERLIST:	// change affiliation to none (from member)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("jid"), jid, _T("affiliation"), _T("none"));
		break;
	case MUC_MODERATORLIST:	// change role to participant (from moderator)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("jid"), jid, _T("role"), _T("participant"));
		break;
	case MUC_ADMINLIST:		// change affiliation to member (from admin)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("jid"), jid, _T("affiliation"), _T("member"));
		break;
	case MUC_OWNERLIST:		// change affiliation to admin (from owner)
		AdminSet(roomJid, JABBER_FEAT_MUC_ADMIN, _T("jid"), jid, _T("affiliation"), _T("admin"));
		break;
	}
}
