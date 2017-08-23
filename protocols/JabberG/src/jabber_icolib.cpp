/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-17 Miranda NG project

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

#define IDI_ONLINE                      104
#define IDI_OFFLINE                     105
#define IDI_AWAY                        128
#define IDI_FREE4CHAT                   129
#define IDI_INVISIBLE                   130
#define IDI_NA                          131
#define IDI_DND                         158
#define IDI_OCCUPIED                    159
#define IDI_ONTHEPHONE                  1002
#define IDI_OUTTOLUNCH                  1003

HIMAGELIST hAdvancedStatusIcon = nullptr;

struct CTransportProtoTableItem
{
	wchar_t *mask;
	char*  proto;
};

static CTransportProtoTableItem TransportProtoTable[] =
{
	{ L"|*icq*|jit*",     "ICQ" },
	{ L"msn*",            "MSN" },
	{ L"yahoo*",          "YAHOO" },
	{ L"mrim*",           "MRA" },
	{ L"aim*",            "AIM" },
	//request #3094
	{ L"|gg*|gadu*",      "GaduGadu" },
	{ L"tv*",             "TV" },
	{ L"dict*",           "Dictionary" },
	{ L"weather*",        "Weather" },
	{ L"skype*",          "Skype" },
	{ L"sms*",            "SMS" },
	{ L"smtp*",           "SMTP" },
	//j2j
	{ L"gtalk.*.*",       "GTalk" },
	{ L"|xmpp.*.*|j2j.*.*","Jabber2Jabber" },
	//jabbim.cz - services
	{ L"disk*",           "Jabber Disk" },
	{ L"irc*",            "IRC" },
	{ L"rss*",            "RSS" },
	{ L"tlen*",           "Tlen" },

	// German social networks
	{ L"studivz*",        "StudiVZ" },
	{ L"schuelervz*",     "SchuelerVZ" },
	{ L"meinvz*",         "MeinVZ" },
};

static int skinIconStatusToResourceId[] = {IDI_OFFLINE,IDI_ONLINE,IDI_AWAY,IDI_DND,IDI_NA,IDI_NA,/*IDI_OCCUPIED,*/IDI_FREE4CHAT,IDI_INVISIBLE,IDI_ONTHEPHONE,IDI_OUTTOLUNCH};
static int skinStatusToJabberStatus[] = {0,1,2,3,4,4,6,7,2,2};

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

	if (m_name) mir_free(m_name);
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

	SKINICONDESC sid = { 0 };
	sid.defaultFile.w = filename;
	sid.pszName = szSettingName;
	sid.section.w = szSection;
	sid.description.w = szDescription;
	sid.flags = SIDF_ALL_UNICODE;
	sid.iDefaultIndex = iconid;
	item->m_hIcolibItem = IcoLib_AddIcon(&sid);

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

CIconPool::CPoolItem *CIconPool::FindItemByName(const char *name)
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

HANDLE CJabberProto::GetIconHandle(int iconId)
{
	if (iconId == IDI_JABBER)
		return m_hProtoIcon;

	return g_GetIconHandle(iconId);
}

HICON CJabberProto::LoadIconEx(const char* name, bool big)
{
	if (HICON result = g_LoadIconEx(name, big))
		return result;

	if (!mir_strcmp(name, "main"))
		return IcoLib_GetIconByHandle(m_hProtoIcon, big);

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// internal functions

static inline wchar_t qtoupper(wchar_t c)
{
	return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

static BOOL WildComparei(const wchar_t *name, const wchar_t *mask)
{
	const wchar_t *last = '\0';
	for (;; mask++, name++) {
		if (*mask != '?' && qtoupper(*mask) != qtoupper(*name))
			break;
		if (*name == '\0')
			return ((BOOL)!*mask);
	}

	if (*mask != '*')
		return FALSE;

	for (;; mask++, name++) {
		while (*mask == '*') {
			last = mask++;
			if (*mask == '\0')
				return ((BOOL)!*mask);   /* true */
		}

		if (*name == '\0')
			return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && qtoupper(*mask) != qtoupper(*name))
			name -= (size_t)(mask - last) - 1, mask = last;
	}
}

static BOOL MatchMask(const wchar_t *name, const wchar_t *mask)
{
	if (!mask || !name)
		return mask == name;

	if (*mask != '|')
		return WildComparei(name, mask);

	wchar_t *temp = NEWWSTR_ALLOCA(mask);
	for (int e = 1; mask[e] != '\0'; e++) {
		int s = e;
		while (mask[e] != '\0' && mask[e] != '|')
			e++;

		temp[e] = 0;
		if (WildComparei(name, temp + s))
			return TRUE;

		if (mask[e] == 0)
			return FALSE;
	}

	return FALSE;
}

static HICON ExtractIconFromPath(const char *path, BOOL * needFree)
{
	char *comma;
	char file[MAX_PATH], fileFull[MAX_PATH];
	int n;
	HICON hIcon;
	mir_strncpy(file, path, sizeof(file));
	comma = strrchr(file, ',');
	if (comma == nullptr) n = 0;
	else { n = atoi(comma + 1); *comma = 0; }
	PathToAbsolute(file, fileFull);
	hIcon = nullptr;
	ExtractIconExA(fileFull, n, nullptr, &hIcon, 1);
	if (needFree)
		*needFree = (hIcon != nullptr);

	return hIcon;
}

static HICON LoadTransportIcon(char *filename, int i, char *IconName, wchar_t *SectName, wchar_t *Description, int internalidx, BOOL *needFree)
{
	char szPath[MAX_PATH], szMyPath[MAX_PATH], szFullPath[MAX_PATH], *str;
	BOOL has_proto_icon = FALSE;
	if (needFree) *needFree = FALSE;
	GetModuleFileNameA(nullptr, szPath, MAX_PATH);
	str = strrchr(szPath, '\\');
	if (str != nullptr) *str = 0;
	mir_snprintf(szMyPath, "%s\\Icons\\%s", szPath, filename);
	mir_snprintf(szFullPath, "%s\\Icons\\%s,%d", szPath, filename, i);
	BOOL nf;
	HICON hi = ExtractIconFromPath(szFullPath, &nf);
	if (hi) has_proto_icon = TRUE;
	if (hi && nf) DestroyIcon(hi);
	if (IconName != nullptr && SectName != nullptr) {
		SKINICONDESC sid = { 0 };
		sid.hDefaultIcon = (has_proto_icon) ? nullptr : Skin_LoadProtoIcon(0, -internalidx);
		sid.section.w = SectName;
		sid.pszName = IconName;
		sid.description.w = Description;
		sid.defaultFile.a = szMyPath;
		sid.iDefaultIndex = i;
		sid.flags = SIDF_UNICODE;
		IcoLib_AddIcon(&sid);
	}
	return IcoLib_GetIcon(IconName);
}

int CJabberProto::LoadAdvancedIcons(int iID)
{
	char *proto = TransportProtoTable[iID].proto;
	char defFile[MAX_PATH] = { 0 };
	wchar_t Group[255];
	char Uname[255];
	int first = -1;
	HICON empty = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);

	mir_snwprintf(Group, LPGENW("Status icons")L"/%s/%S %s", m_tszUserName, proto, TranslateT("transport"));
	mir_snprintf(defFile, "proto_%s.dll", proto);
	if (!hAdvancedStatusIcon)
		hAdvancedStatusIcon = Clist_GetImageList();

	mir_cslock lck(m_csModeMsgMutex);
	for (int i = 0; i < ID_STATUS_ONTHEPHONE - ID_STATUS_OFFLINE; i++) {
		BOOL needFree;
		int n = skinStatusToJabberStatus[i];
		wchar_t *descr = pcli->pfnGetStatusModeDescription(n + ID_STATUS_OFFLINE, 0);
		mir_snprintf(Uname, "%s_Transport_%s_%d", m_szModuleName, proto, n);
		HICON hicon = LoadTransportIcon(defFile, -skinIconStatusToResourceId[i], Uname, Group, descr, -(n + ID_STATUS_OFFLINE), &needFree);
		int index = (m_transportProtoTableStartIndex[iID] == -1) ? -1 : m_transportProtoTableStartIndex[iID] + n;
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

int CJabberProto::GetTransportProtoID(wchar_t* TransportDomain)
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

	return m_transportProtoTableStartIndex[iID] + skinStatusToJabberStatus[Status - ID_STATUS_OFFLINE];
}

/////////////////////////////////////////////////////////////////////////////////////////
// a hook for the IcoLib plugin

int CJabberProto::OnReloadIcons(WPARAM, LPARAM)
{
	for (int i = 0; i < _countof(TransportProtoTable); i++)
		if (m_transportProtoTableStartIndex[i] != -1)
			LoadAdvancedIcons(i);

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

INT_PTR __cdecl CJabberProto::JGetAdvancedStatusIcon(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return -1;

	if (!getByte(hContact, "IsTransported", 0))
		return -1;

	int iID = GetTransportProtoID(ptrW(getWStringA(hContact, "Transport")));
	if (iID < 0)
		return -1;

	WORD Status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	if (Status < ID_STATUS_OFFLINE)
		Status = ID_STATUS_OFFLINE;
	else if (Status > ID_STATUS_INVISIBLE)
		Status = ID_STATUS_ONLINE;
	return GetTransportStatusIconIndex(iID, Status);
}

/////////////////////////////////////////////////////////////////////////////////////////
//   Transport check functions

BOOL CJabberProto::DBCheckIsTransportedContact(const wchar_t *jid, MCONTACT hContact)
{
	// check if transport is already set
	if (!jid || !hContact)
		return FALSE;

	// strip domain part from jid
	wchar_t *domain = wcschr((wchar_t*)jid, '@');
	BOOL isAgent = (domain == nullptr) ? TRUE : FALSE;
	BOOL isTransported = FALSE;
	if (domain != nullptr)
		domain = NEWWSTR_ALLOCA(domain + 1);
	else
		domain = NEWWSTR_ALLOCA(jid);

	wchar_t *resourcepos = wcschr(domain, '/');
	if (resourcepos != nullptr)
		*resourcepos = '\0';

	for (int i = 0; i < _countof(TransportProtoTable); i++)
		if (MatchMask(domain, TransportProtoTable[i].mask)) {
			GetTransportStatusIconIndex(GetTransportProtoID(domain), ID_STATUS_OFFLINE);
			isTransported = TRUE;
			break;
		}

	if (m_lstTransports.getIndex(domain) == -1 && isAgent) {
		m_lstTransports.insert(mir_wstrdup(domain));
		setByte(hContact, "IsTransport", 1);
	}

	if (isTransported) {
		setWString(hContact, "Transport", domain);
		setByte(hContact, "IsTransported", 1);
	}
	return isTransported;
}

void CJabberProto::CheckAllContactsAreTransported()
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		ptrW jid(getWStringA(hContact, "jid"));
		if (jid)
			DBCheckIsTransportedContact(jid, hContact);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Cross-instance shared icons

static IconItem sharedIconList1[] =
{
	{ LPGEN("Jabber"),			      "jabber",			  IDI_JABBER			 },
	{ LPGEN("Privacy Lists"),         "privacylists",     IDI_PRIVACY_LISTS      },
	{ LPGEN("Bookmarks"),             "bookmarks",        IDI_BOOKMARKS          },
	{ LPGEN("Notes"),                 "notes",            IDI_NOTES              },
	{ LPGEN("Multi-User Conference"), "group",            IDI_GROUP              },
	{ LPGEN("Agents list"),           "Agents",           IDI_AGENTS             },
	{ LPGEN("Transports"),            "transport",        IDI_TRANSPORT          },
	{ LPGEN("Registered transports"), "transport_loc",    IDI_TRANSPORTL         },
	{ LPGEN("Change password"),       "key",              IDI_KEYS               },
	{ LPGEN("Personal vCard"),        "vcard",            IDI_VCARD              },
	{ LPGEN("Convert to room"),       "convert",          IDI_USER2ROOM          },
	{ LPGEN("Login/logout"),          "trlogonoff",       IDI_LOGIN              },
	{ LPGEN("Resolve nicks"),         "trresolve",        IDI_REFRESH            },
	{ LPGEN("Send note"),             "sendnote",         IDI_SEND_NOTE,         },
	{ LPGEN("Service Discovery"),     "servicediscovery", IDI_SERVICE_DISCOVERY  },
	{ LPGEN("AdHoc Command"),         "adhoc",            IDI_COMMAND            },
	{ LPGEN("XML Console"),           "xmlconsole",       IDI_CONSOLE            },
	{ LPGEN("OpenID Request"),        "openid",           IDI_HTTP_AUTH          },
	{ LPGEN("Add contact"),			  "addcontact",       IDI_ADDCONTACT		 },
	{ LPGEN("Delete"),				  "delete",		      IDI_DELETE             },
	{ LPGEN("Edit"),				  "edit",		      IDI_EDIT               },
	{ LPGEN("Open"),				  "open",		      IDI_OPEN               },
	{ LPGEN("Save"),				  "save",		      IDI_SAVE               }
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
	Icon_Register(hInst, LPGEN("Protocols") "/" LPGEN("Jabber"), sharedIconList1, _countof(sharedIconList1), GLOBAL_SETTING_PREFIX);
	Icon_Register(hInst, LPGEN("Protocols") "/" LPGEN("Jabber") "/" LPGEN("Dialogs"), sharedIconList2, _countof(sharedIconList2), GLOBAL_SETTING_PREFIX);
	Icon_Register(hInst, LPGEN("Protocols") "/" LPGEN("Dialogs") "/" LPGEN("Discovery"), sharedIconList3, _countof(sharedIconList3), GLOBAL_SETTING_PREFIX);
	Icon_Register(hInst, LPGEN("Protocols") "/" LPGEN("Dialogs") "/" LPGEN("Privacy"), sharedIconList4, _countof(sharedIconList4), GLOBAL_SETTING_PREFIX);
}

HANDLE g_GetIconHandle(int iconId)
{
	int i;

	for (i = 0; i < _countof(sharedIconList1); i++)
		if (sharedIconList1[i].defIconID == iconId)
			return sharedIconList1[i].hIcolib;

	for (i = 0; i < _countof(sharedIconList2); i++)
		if (sharedIconList2[i].defIconID == iconId)
			return sharedIconList2[i].hIcolib;

	for (i = 0; i < _countof(sharedIconList3); i++)
		if (sharedIconList3[i].defIconID == iconId)
			return sharedIconList3[i].hIcolib;

	for (i = 0; i < _countof(sharedIconList4); i++)
		if (sharedIconList4[i].defIconID == iconId)
			return sharedIconList4[i].hIcolib;

	return nullptr;
}

HICON g_LoadIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, "%s_%s", GLOBAL_SETTING_PREFIX, name);
	return IcoLib_GetIcon(szSettingName, big);
}

void ImageList_AddIcon_Icolib(HIMAGELIST hIml, HICON hIcon)
{
	ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
}
