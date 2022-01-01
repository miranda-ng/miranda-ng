/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

Idea & portions of code by Artem Shpynov

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

#define IDI_ONLINE     104
#define IDI_OFFLINE    105
#define IDI_AWAY       128
#define IDI_FREE4CHAT  129
#define IDI_INVISIBLE  130
#define IDI_NA         131
#define IDI_DND        158
#define IDI_OCCUPIED   159

HIMAGELIST hAdvancedStatusIcon = nullptr;

struct
{
	char *mask;
	char*  proto;
}
static TransportProtoTable[] =
{
	{ "|*icq*|jit*", "ICQ" },
	{ "mrim*",       "MRA" },

	//request #3094
	{ "|gg*|gadu*",  "GaduGadu" },
	{ "tv*",         "TV" },
	{ "dict*",       "Dictionary" },
	{ "weather*",    "Weather" },
	{ "skype*",      "Skype" },
	{ "sms*",        "SMS" },
	{ "smtp*",       "SMTP" },

	//j2j
	{ "gtalk.*.*",   "GTalk" },
	{ "j2j.*.*",     "Jabber2Jabber" },

	//jabbim.cz - services
	{ "disk*",       "Jabber Disk" },
	{ "irc*",        "IRC" },
	{ "rss*",        "RSS" },
	{ "tlen*",       "Tlen" },

	// German social networks
	{ "studivz*",    "StudiVZ" },
	{ "schuelervz*", "SchuelerVZ" },
	{ "meinvz*",     "MeinVZ" }
};

///////////////////////////////////////////////////////////////////////////////
// CIconPool class

int CIconPool::CPoolItem::cmp(const CPoolItem *p1, const CPoolItem *p2)
{
	return mir_strcmp(p1->m_name, p2->m_name);
}

CIconPool::CPoolItem::CPoolItem() :
	m_name(nullptr), m_szIcolibName(nullptr), m_hIcolibItem(nullptr)
{
}

CIconPool::CPoolItem::~CPoolItem()
{
	if (m_hIcolibItem && m_szIcolibName) {
		IcoLib_RemoveIcon(m_szIcolibName);
		mir_free(m_szIcolibName);
	}

	mir_free(m_name);
}

CIconPool::CIconPool() :
	m_items(10, CIconPool::CPoolItem::cmp)
{
}

CIconPool::~CIconPool()
{
}

void CIconPool::RegisterIcon(const char *name, wchar_t *filename, int iconid, wchar_t *szSection, wchar_t *szDescription)
{
	char szSettingName[128];
	mir_snprintf(szSettingName, "jabber_%s", name);

	CPoolItem *item = new CPoolItem;
	item->m_name = mir_strdup(name);
	item->m_szIcolibName = mir_strdup(szSettingName);

	SKINICONDESC sid = {};
	sid.defaultFile.w = filename;
	sid.pszName = szSettingName;
	sid.section.w = szSection;
	sid.description.w = szDescription;
	sid.flags = SIDF_ALL_UNICODE;
	sid.iDefaultIndex = iconid;
	item->m_hIcolibItem = IcoLib_AddIcon(&sid, &g_plugin);
	m_items.insert(item);
}

HANDLE CIconPool::GetIcolibHandle(const char *name)
{
	if (CPoolItem *item = FindItemByName(name))
		return item->m_hIcolibItem;

	return nullptr;
}

char *CIconPool::GetIcolibName(const char *name)
{
	if (CPoolItem *item = FindItemByName(name))
		return item->m_szIcolibName;

	return nullptr;
}

HICON CIconPool::GetIcon(const char *name, bool big)
{
	if (CPoolItem *item = FindItemByName(name))
		return IcoLib_GetIconByHandle(item->m_hIcolibItem, big);

	return nullptr;
}

CIconPool::CPoolItem* CIconPool::FindItemByName(const char *name)
{
	CPoolItem item;
	item.m_name = mir_strdup(name);
	return m_items.find(&item);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Icons init

void CJabberProto::IconsInit(void)
{
	m_transportProtoTableStartIndex = (int *)mir_alloc(sizeof(int) * _countof(TransportProtoTable));
	for (int i = 0; i < _countof(TransportProtoTable); i++)
		m_transportProtoTableStartIndex[i] = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// internal functions

static bool MatchMask(const char *name, const char *mask)
{
	if (!mask || !name)
		return mask == name;

	if (*mask != '|')
		return wildcmpi(name, mask);

	char *temp = NEWSTR_ALLOCA(mask);
	for (int e = 1; mask[e] != '\0'; e++) {
		int s = e;
		while (mask[e] != '\0' && mask[e] != '|')
			e++;

		temp[e] = 0;
		if (wildcmpi(name, temp + s))
			return true;

		if (mask[e] == 0)
			return false;
	}

	return false;
}

static HICON ExtractIconFromPath(const char *path, BOOL *needFree)
{
	char file[MAX_PATH], fileFull[MAX_PATH];
	mir_strncpy(file, path, sizeof(file));

	int n;
	char *comma = strrchr(file, ',');
	if (comma == nullptr) n = 0;
	else { n = atoi(comma + 1); *comma = 0; }
	PathToAbsolute(file, fileFull);
	HICON hIcon = nullptr;
	ExtractIconExA(fileFull, n, nullptr, &hIcon, 1);
	if (needFree)
		*needFree = (hIcon != nullptr);

	return hIcon;
}

static HICON LoadTransportIcon(char *filename, int i, char *IconName, wchar_t *SectName, wchar_t *Description, int internalidx, BOOL *needFree)
{
	char szPath[MAX_PATH], szMyPath[MAX_PATH], szFullPath[MAX_PATH], *str;
	bool has_proto_icon = false;
	if (needFree)
		*needFree = false;
	GetModuleFileNameA(nullptr, szPath, MAX_PATH);
	str = strrchr(szPath, '\\');
	if (str != nullptr) *str = 0;
	mir_snprintf(szMyPath, "%s\\Icons\\%s", szPath, filename);
	mir_snprintf(szFullPath, "%s\\Icons\\%s,%d", szPath, filename, i);
	BOOL nf;
	HICON hi = ExtractIconFromPath(szFullPath, &nf);
	if (hi) has_proto_icon = true;
	if (hi && nf) DestroyIcon(hi);
	if (IconName != nullptr && SectName != nullptr) {
		SKINICONDESC sid = {};
		sid.hDefaultIcon = (has_proto_icon) ? nullptr : Skin_LoadProtoIcon(nullptr, -internalidx);
		sid.section.w = SectName;
		sid.pszName = IconName;
		sid.description.w = Description;
		sid.defaultFile.a = szMyPath;
		sid.iDefaultIndex = i;
		sid.flags = SIDF_UNICODE;
		g_plugin.addIcon(&sid);
	}
	return IcoLib_GetIcon(IconName);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int skinIconStatusToResourceId[] = { IDI_OFFLINE, IDI_ONLINE, IDI_AWAY, IDI_DND, IDI_NA, IDI_NA, IDI_OCCUPIED, IDI_FREE4CHAT, IDI_INVISIBLE };

int CJabberProto::LoadAdvancedIcons(int iID)
{
	char *proto = TransportProtoTable[iID].proto;
	HICON empty = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);

	wchar_t Group[255];
	mir_snwprintf(Group, LPGENW("Status icons")L"/%s/%S %s", m_tszUserName, proto, TranslateT("transport"));

	char defFile[MAX_PATH] = { 0 };
	mir_snprintf(defFile, "proto_%s.dll", proto);
	if (!hAdvancedStatusIcon)
		hAdvancedStatusIcon = Clist_GetImageList();

	int first = -1;
	mir_cslock lck(m_csModeMsgMutex);
	for (int i = 0; i < ID_STATUS_MAX - ID_STATUS_OFFLINE; i++) {
		char Uname[255];
		mir_snprintf(Uname, "%s_Transport_%s_%d", m_szModuleName, proto, i);

		BOOL needFree;
		HICON hicon = LoadTransportIcon(defFile, -skinIconStatusToResourceId[i], Uname, Group, Clist_GetStatusModeDescription(i + ID_STATUS_OFFLINE, 0), -(i + ID_STATUS_OFFLINE), &needFree);
		int index = (m_transportProtoTableStartIndex[iID] == -1) ? -1 : m_transportProtoTableStartIndex[iID] + i;
		int added = ImageList_ReplaceIcon(hAdvancedStatusIcon, index, hicon ? hicon : empty);
		if (first == -1)
			first = added;
		if (hicon && needFree)
			DestroyIcon(hicon);
	}

	if (m_transportProtoTableStartIndex[iID] == -1)
		m_transportProtoTableStartIndex[iID] = first;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CJabberProto::GetTransportProtoID(char *TransportDomain)
{
	for (int i = 0; i < _countof(TransportProtoTable); i++)
		if (MatchMask(TransportDomain, TransportProtoTable[i].mask))
			return i;

	return -1;
}

int CJabberProto::GetTransportStatusIconIndex(int iID, int Status)
{
	if (iID < 0 || iID >= _countof(TransportProtoTable))
		return -1;

	//icons not loaded - loading icons
	if (m_transportProtoTableStartIndex[iID] == -1)
		LoadAdvancedIcons(iID);

	//some fault on loading icons
	if (m_transportProtoTableStartIndex[iID] == -1)
		return -1;

	if (Status < ID_STATUS_OFFLINE)
		Status = ID_STATUS_OFFLINE;

	return m_transportProtoTableStartIndex[iID] + Status - ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a hook for the IcoLib plugin

int CJabberProto::OnReloadIcons(WPARAM, LPARAM)
{
	for (int i = 0; i < _countof(TransportProtoTable); i++)
		if (m_transportProtoTableStartIndex[i] != -1)
			LoadAdvancedIcons(i);

	ClearMoodIcons();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Prototype for Jabber and other protocols to return index of Advanced status
// wParam - MCONTACT of called protocol
// lParam - should be 0 (reserverd for futher usage)
// return value: -1 - no Advanced status
// : other - index of icons in clcimagelist.
// if imagelist require advanced painting status overlay(like xStatus)
// index should be shifted to HIWORD, LOWORD should be 0

INT_PTR __cdecl CJabberProto::OnGetAdvancedStatusIcon(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return -1;

	if (getByte(hContact, "IsTransported", 0)) {
		int iID = GetTransportProtoID(ptrA(getUStringA(hContact, "Transport")));
		if (iID < 0)
			return -1;

		uint16_t Status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
		if (Status < ID_STATUS_OFFLINE)
			Status = ID_STATUS_OFFLINE;
		else if (Status > ID_STATUS_INVISIBLE)
			Status = ID_STATUS_ONLINE;
		return GetTransportStatusIconIndex(iID, Status);
	}

	if (int xStatus = getByte(hContact, DBSETTING_XSTATUSID)) {
		int idx = GetMoodIconIdx(xStatus);
		if (idx != -1)
			return MAKELONG(0, idx); // shift icon id to the upper word
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   Transport check functions

bool CJabberProto::DBCheckIsTransportedContact(const char *jid, MCONTACT hContact)
{
	// check if transport is already set
	if (!jid || !hContact)
		return false;

	// strip domain part from jid
	char *domain = (char*)strchr(jid, '@');
	bool isAgent = (domain == nullptr);
	bool isTransported = false;
	if (domain != nullptr)
		domain = NEWSTR_ALLOCA(domain + 1);
	else
		domain = NEWSTR_ALLOCA(jid);

	char *resourcepos = strchr(domain, '/');
	if (resourcepos != nullptr)
		*resourcepos = '\0';

	for (auto &it : TransportProtoTable)
		if (MatchMask(domain, it.mask)) {
			GetTransportStatusIconIndex(GetTransportProtoID(domain), ID_STATUS_OFFLINE);
			isTransported = true;
			break;
		}

	if (m_lstTransports.getIndex(domain) == -1 && isAgent) {
		m_lstTransports.insert(mir_strdup(domain));
		setByte(hContact, "IsTransport", 1);
	}

	if (isTransported) {
		setUString(hContact, "Transport", domain);
		setByte(hContact, "IsTransported", 1);
	}
	return isTransported;
}

void CJabberProto::CheckAllContactsAreTransported()
{
	for (auto &hContact : AccContacts()) {
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid)
			DBCheckIsTransportedContact(jid, hContact);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Cross-instance shared icons

static IconItem sharedIconList1[] =
{
	{ LPGEN("Jabber"),                "jabber",           IDI_JABBER             },
	{ LPGEN("Privacy Lists"),         "privacylists",     IDI_PRIVACY_LISTS      },
	{ LPGEN("Bookmarks"),             "bookmarks",        IDI_BOOKMARKS          },
	{ LPGEN("Notes"),                 "notes",            IDI_NOTES              },
	{ LPGEN("Multi-User Conference"), "group",            IDI_GROUP              },
	{ LPGEN("Agents list"),           "Agents",           IDI_AGENTS             },
	{ LPGEN("Transports"),            "transport",        IDI_TRANSPORT          },
	{ LPGEN("Registered transports"), "transport_loc",    IDI_TRANSPORTL         },
	{ LPGEN("Personal vCard"),        "vcard",            IDI_VCARD              },
	{ LPGEN("Convert to room"),       "convert",          IDI_USER2ROOM          },
	{ LPGEN("Login/logout"),          "trlogonoff",       IDI_LOGIN              },
	{ LPGEN("Resolve nicks"),         "trresolve",        IDI_REFRESH            },
	{ LPGEN("Send note"),             "sendnote",         IDI_SEND_NOTE,         },
	{ LPGEN("Service Discovery"),     "servicediscovery", IDI_SERVICE_DISCOVERY  },
	{ LPGEN("AdHoc Command"),         "adhoc",            IDI_COMMAND            },
	{ LPGEN("XML Console"),           "xmlconsole",       IDI_CONSOLE            },
	{ LPGEN("OpenID Request"),        "openid",           IDI_HTTP_AUTH          },
	{ LPGEN("Add contact"),           "addcontact",       IDI_ADDCONTACT         },
	{ LPGEN("Delete"),                "delete",           IDI_DELETE             },
	{ LPGEN("Edit"),                  "edit",             IDI_EDIT               },
	{ LPGEN("Open"),                  "open",             IDI_OPEN               },
	{ LPGEN("Save"),                  "save",	            IDI_SAVE               }
};

static IconItem sharedIconList2[] =
{
	{ LPGEN("Discovery succeeded"),   "disco_ok",         IDI_DISCO_OK           },
	{ LPGEN("Discovery failed"),      "disco_fail",       IDI_DISCO_FAIL         },
	{ LPGEN("Discovery in progress"), "disco_progress",   IDI_DISCO_PROGRESS     },
	{ LPGEN("View as tree"),          "sd_view_tree",     IDI_VIEW_TREE          },
	{ LPGEN("View as list"),          "sd_view_list",     IDI_VIEW_LIST          },
	{ LPGEN("Apply filter"),          "sd_filter_apply",  IDI_FILTER_APPLY       },
	{ LPGEN("Reset filter"),          "sd_filter_reset",  IDI_FILTER_RESET       }
};

static IconItem sharedIconList3[] =
{
	{ LPGEN("Navigate home"),         "sd_nav_home",      IDI_NAV_HOME           },
	{ LPGEN("Refresh node"),          "sd_nav_refresh",   IDI_NAV_REFRESH        },
	{ LPGEN("Browse node"),           "sd_browse",        IDI_BROWSE             },
	{ LPGEN("RSS service"),           "node_rss",         IDI_NODE_RSS           },
	{ LPGEN("Server"),                "node_server",      IDI_NODE_SERVER        },
	{ LPGEN("Storage service"),       "node_store",       IDI_NODE_STORE         },
	{ LPGEN("Weather service"),       "node_weather",     IDI_NODE_WEATHER       }
};

static IconItem sharedIconList4[] =
{
	{ LPGEN("Generic privacy list"),  "pl_list_any",      IDI_PL_LIST_ANY        },
	{ LPGEN("Active privacy list"),   "pl_list_active",   IDI_PL_LIST_ACTIVE     },
	{ LPGEN("Default privacy list"),  "pl_list_default",  IDI_PL_LIST_DEFAULT    },
	{ LPGEN("Move up"),               "arrow_up",         IDI_ARROW_UP           },
	{ LPGEN("Move down"),             "arrow_down",       IDI_ARROW_DOWN         },
	{ LPGEN("Allow Messages"),        "pl_msg_allow",     IDI_PL_MSG_ALLOW       },
	{ LPGEN("Allow Presences (in)"),  "pl_prin_allow",    IDI_PL_PRIN_ALLOW      },
	{ LPGEN("Allow Presences (out)"), "pl_prout_allow",   IDI_PL_PROUT_ALLOW     },
	{ LPGEN("Allow Queries"),         "pl_iq_allow",      IDI_PL_QUERY_ALLOW     },
	{ LPGEN("Deny Messages"),         "pl_msg_deny",      IDI_PL_MSG_DENY        },
	{ LPGEN("Deny Presences (in)"),   "pl_prin_deny",     IDI_PL_PRIN_DENY       },
	{ LPGEN("Deny Presences (out)"),  "pl_prout_deny",    IDI_PL_PROUT_DENY      },
	{ LPGEN("Deny Queries"),          "pl_iq_deny",       IDI_PL_QUERY_DENY      }
};

void g_IconsInit()
{
	g_plugin.registerIcon(LPGEN("Protocols") "/" LPGEN("Jabber"), sharedIconList1, GLOBAL_SETTING_MODULE);
	g_plugin.registerIcon(LPGEN("Protocols") "/" LPGEN("Jabber") "/" LPGEN("Dialogs"), sharedIconList2, GLOBAL_SETTING_MODULE);
	g_plugin.registerIcon(LPGEN("Protocols") "/" LPGEN("Jabber") "/" LPGEN("Dialogs") "/" LPGEN("Discovery"), sharedIconList3, GLOBAL_SETTING_MODULE);
	g_plugin.registerIcon(LPGEN("Protocols") "/" LPGEN("Jabber") "/" LPGEN("Dialogs") "/" LPGEN("Privacy"), sharedIconList4, GLOBAL_SETTING_MODULE);
}
