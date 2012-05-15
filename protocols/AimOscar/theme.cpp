/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aim.h"

#include <m_cluiframes.h>
#include "m_extraicons.h"

#include "theme.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Icons init

struct _tag_iconList
{
	const char*  szDescr;
	const char*  szName;
	int          defIconID;
	const char*  szSection;
}
static const iconList[] =
{
	{	LPGEN("ICQ"),                    "icq",         IDI_ICQ             },
	{	LPGEN("Add"),                    "add",         IDI_ADD             },
	{	LPGEN("Block"),                  "block",       IDI_BLOCK           },
	{	LPGEN("Profile"),                "profile",     IDI_PROFILE         },
	{	LPGEN("AOL Mail"),               "mail",        IDI_MAIL            },
	{	LPGEN("AIM Icon"),               "aim",         IDI_AIM             },
	{	LPGEN("Hiptop"),                 "hiptop",      IDI_HIPTOP          },
	{	LPGEN("AOL Bot"),                "bot",         IDI_BOT             },
	{	LPGEN("Admin"),                  "admin",       IDI_ADMIN           },
	{	LPGEN("Confirmed"),              "confirm",     IDI_CONFIRMED       },
	{	LPGEN("Not Confirmed"),          "uconfirm",    IDI_UNCONFIRMED     },
	{	LPGEN("Blocked list"),           "away",        IDI_AWAY            },
	{	LPGEN("Idle"),                   "idle",        IDI_IDLE            },
	{	LPGEN("AOL"),                    "aol",         IDI_AOL             },

	{	LPGEN("Foreground Color"),       "foreclr",     IDI_FOREGROUNDCOLOR, LPGEN("Profile Editor") },
	{	LPGEN("Background Color"),       "backclr",     IDI_BACKGROUNDCOLOR, LPGEN("Profile Editor") },
	{	LPGEN("Bold"),                   "bold",        IDI_BOLD,            LPGEN("Profile Editor") },
	{	LPGEN("Not Bold"),               "nbold",       IDI_NBOLD,           LPGEN("Profile Editor") },
	{	LPGEN("Italic"),                 "italic",      IDI_ITALIC,          LPGEN("Profile Editor") },
	{	LPGEN("Not Italic"),             "nitalic",     IDI_NITALIC,         LPGEN("Profile Editor") },
	{	LPGEN("Underline"),              "undrln",      IDI_UNDERLINE,       LPGEN("Profile Editor") },
	{	LPGEN("Not Underline"),          "nundrln",     IDI_NUNDERLINE,      LPGEN("Profile Editor") },
	{	LPGEN("Subscript"),              "sub_scrpt",   IDI_SUBSCRIPT,       LPGEN("Profile Editor") },
	{	LPGEN("Not Subscript"),          "nsub_scrpt",  IDI_NSUBSCRIPT,      LPGEN("Profile Editor") },
	{	LPGEN("Superscript"),            "sup_scrpt",   IDI_SUPERSCRIPT,     LPGEN("Profile Editor") },
	{	LPGEN("Not Superscript"),        "nsup_scrpt",  IDI_NSUPERSCRIPT,    LPGEN("Profile Editor") },
	{	LPGEN("Normal Script"),          "norm_scrpt",  IDI_NORMALSCRIPT,    LPGEN("Profile Editor") },
	{	LPGEN("Not Normal Script"),      "nnorm_scrpt", IDI_NNORMALSCRIPT,   LPGEN("Profile Editor") },
};

static HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons(void)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInstance, szFile, SIZEOF(szFile));

	char szSettingName[100];
	char szSectionName[100];

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.pszSection = szSectionName;
	sid.flags = SIDF_PATH_TCHAR;

	for (int i = 0; i < SIZEOF(iconList); i++) 
	{
		mir_snprintf(szSettingName, sizeof(szSettingName), "AIM_%s", iconList[i].szName);

		if (iconList[i].szSection)
			mir_snprintf(szSectionName, sizeof(szSectionName), "%s/%s/%s", LPGEN("Protocols"), LPGEN("AIM"), iconList[i].szSection);
		else
			mir_snprintf(szSectionName, sizeof(szSectionName), "%s/%s", LPGEN("Protocols"), LPGEN("AIM"));

		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}	
}

HICON LoadIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "AIM_%s", name);
	return (HICON)CallService(MS_SKIN2_GETICON, big, (LPARAM)szSettingName);
}

HANDLE GetIconHandle(const char* name)
{
	for (unsigned i=0; i < SIZEOF(iconList); i++)
		if (strcmp(iconList[i].szName, name) == 0)
			return hIconLibItem[i];
	return NULL;
}

void ReleaseIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName ), "%s_%s", "AIM", name);
	CallService(big ? MS_SKIN2_RELEASEICONBIG : MS_SKIN2_RELEASEICON, 0, (LPARAM)szSettingName);
}

void WindowSetIcon(HWND hWnd, const char* name)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG,   ( LPARAM )LoadIconEx( name, true ));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, ( LPARAM )LoadIconEx( name ));
}

void WindowFreeIcon(HWND hWnd)
{
	CallService(MS_SKIN2_RELEASEICON, SendMessage(hWnd, WM_SETICON, ICON_BIG, 0), 0);
	CallService(MS_SKIN2_RELEASEICON, SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Extra Icons

extern OBJLIST<CAimProto> g_Instances;

static HANDLE bot_icon, icq_icon, aol_icon, hiptop_icon;
static HANDLE admin_icon, confirmed_icon, unconfirmed_icon;

static HANDLE hListRebuld, hIconApply;
static HANDLE hExtraAT, hExtraES;

static const char* extra_AT_icon_name[5] =
{
	"uconfirm",
	"confirm",
	"icq",
	"aol",
	"admin",
};

static const char* extra_ES_icon_name[2] =
{
	"bot",
	"hiptop",
};

static HANDLE extra_AT_icon_handle[5];
static HANDLE extra_ES_icon_handle[2];

static void load_extra_icons(void)
{
	if (!ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) return;

	unsigned i;

	for (i = 0; i < SIZEOF(extra_AT_icon_handle); ++i)
	{
		extra_AT_icon_handle[i] = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)LoadIconEx(extra_AT_icon_name[i]), 0);
		ReleaseIconEx(extra_AT_icon_name[i]);
	}

	for (i = 0; i < SIZEOF(extra_ES_icon_handle); ++i)
	{
		extra_ES_icon_handle[i] = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)LoadIconEx(extra_ES_icon_name[i]), 0);
		ReleaseIconEx(extra_ES_icon_name[i]);
	}
}

static void set_extra_icon(HANDLE hContact, HANDLE hImage, int column_type)
{
	IconExtraColumn iec;
	iec.cbSize = sizeof(iec);
	iec.hImage = hImage;
	iec.ColumnType = column_type;
	CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)hContact, (LPARAM)&iec);
}

static void clear_AT_icon(HANDLE hContact)
{
	if (hExtraAT)
		ExtraIcon_SetIcon(hExtraAT, hContact, (char*)NULL);
	else
		set_extra_icon(hContact, (HANDLE)-1, EXTRA_ICON_ADV2);
}

static void clear_ES_icon(HANDLE hContact)
{
	if (hExtraES)
		ExtraIcon_SetIcon(hExtraES, hContact, (char*)NULL);
	else
		set_extra_icon(hContact, (HANDLE)-1, EXTRA_ICON_ADV3);
}

static void set_AT_icon(CAimProto* ppro, HANDLE hContact)
{
	if (ppro->getByte(hContact, "ChatRoom", 0)) return;

	unsigned i = ppro->getByte(hContact, AIM_KEY_AC, 0) - 1;

	if (hExtraAT)
	{
		if (i < 5)
		{
			char name[64];
			mir_snprintf(name, sizeof(name), "AIM_%s", extra_AT_icon_name[i]);
			ExtraIcon_SetIcon(hExtraAT, hContact, name);
		}
		else
			ExtraIcon_SetIcon(hExtraAT, hContact, (char*)NULL);
	}
	else
		set_extra_icon(hContact, i < 5 ? extra_AT_icon_handle[i] : (HANDLE)-1, EXTRA_ICON_ADV2);
}

static void set_ES_icon(CAimProto* ppro, HANDLE hContact)
{
	if (ppro->getByte(hContact, "ChatRoom", 0)) return;

	unsigned i = ppro->getByte(hContact, AIM_KEY_ET, 0) - 1;

	if (hExtraES)
	{
		if (i < 2)
		{
			char name[64];
			mir_snprintf(name, sizeof(name), "AIM_%s", extra_ES_icon_name[i]);
			ExtraIcon_SetIcon(hExtraES, hContact, name);
		}
		else
			ExtraIcon_SetIcon(hExtraES, hContact, (char*)NULL);
	}
	else
		set_extra_icon(hContact, i < 2 ? extra_ES_icon_handle[i] : (HANDLE)-1, EXTRA_ICON_ADV3);
}

void set_contact_icon(CAimProto* ppro, HANDLE hContact)
{
	if (!ppro->getByte(AIM_KEY_AT, 0)) set_AT_icon(ppro, hContact);
	if (!ppro->getByte(AIM_KEY_ES, 0)) set_ES_icon(ppro, hContact);
}

static int OnExtraIconsRebuild(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	load_extra_icons();
	return 0;
}

static int OnExtraIconsApply(WPARAM wParam, LPARAM /*lParam*/)
{
	if (!ServiceExists(MS_CLIST_EXTRA_SET_ICON)) return 0;

	HANDLE hContact = (HANDLE)wParam;

	CAimProto *ppro = NULL;
	for (int i = 0; i < g_Instances.getCount(); ++i)
	{
		if (g_Instances[i].is_my_contact(hContact))
		{
			ppro = &g_Instances[i];
			break;
		}
	}

	if (ppro) set_contact_icon(ppro, hContact);

	return 0;
}

void remove_AT_icons(CAimProto* ppro)
{
	if(!ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) return;
	
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if (ppro->is_my_contact(hContact) && !ppro->getByte(hContact, "ChatRoom", 0)) 
			clear_AT_icon(hContact);

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
}

void remove_ES_icons(CAimProto* ppro)
{
	if(!ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) return;

	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if (ppro->is_my_contact(hContact) && !ppro->getByte(hContact, "ChatRoom", 0)) 
			clear_ES_icon(hContact);
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
}

void add_AT_icons(CAimProto* ppro)
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if (ppro->is_my_contact(hContact)) 
			set_AT_icon(ppro, hContact);

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
}

void add_ES_icons(CAimProto* ppro)
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if (ppro->is_my_contact(hContact)) 
			set_ES_icon(ppro, hContact);

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
}

void InitExtraIcons(void)
{
	hExtraAT = ExtraIcon_Register("aimaccounttype", LPGEN("AIM Account Type"), "AIM_aol");
	hExtraES = ExtraIcon_Register("aimextstatus", LPGEN("AIM Extended Status"), "AIM_hiptop");

	if (hExtraAT == NULL)
	{
		hListRebuld = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, OnExtraIconsRebuild);
		hIconApply  = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY,  OnExtraIconsApply);
	}
}

void DestroyExtraIcons(void)
{
	UnhookEvent(hIconApply);
	UnhookEvent(hListRebuld);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Themes

#define MGPROC(x) GetProcAddress(themeAPIHandle,x)

HMODULE  themeAPIHandle = NULL; // handle to uxtheme.dll
HANDLE   (WINAPI *MyOpenThemeData)(HWND,LPCWSTR) = 0;
HRESULT  (WINAPI *MyCloseThemeData)(HANDLE) = 0;
HRESULT  (WINAPI *MyDrawThemeBackground)(HANDLE,HDC,int,int,const RECT *,const RECT *) = 0;

void InitThemeSupport(void)
{
	if (!IsWinVerXPPlus()) return;

	themeAPIHandle = GetModuleHandleA("uxtheme");
	if (themeAPIHandle)
	{
		MyOpenThemeData = (HANDLE (WINAPI *)(HWND,LPCWSTR))MGPROC("OpenThemeData");
		MyCloseThemeData = (HRESULT (WINAPI *)(HANDLE))MGPROC("CloseThemeData");
		MyDrawThemeBackground = (HRESULT (WINAPI *)(HANDLE,HDC,int,int,const RECT *,const RECT *))MGPROC("DrawThemeBackground");
	}
}

void DestroyThemeSupport(void)
{
	DestroyExtraIcons();
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPreBuildContactMenu

int CAimProto::OnPreBuildContactMenu(WPARAM wParam,LPARAM /*lParam*/)
{
	HANDLE hContact = (HANDLE)wParam;
	bool isChatRoom = getByte(hContact, "ChatRoom", 0) != 0;

	CLISTMENUITEM mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);

	//see if we should add the html away message context menu items
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE;
	if (getWord(hContact, AIM_KEY_ST, ID_STATUS_OFFLINE) != ID_STATUS_AWAY || isChatRoom)
		mi.flags |= CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hHTMLAwayContextMenuItem,(LPARAM)&mi);

	mi.flags = CMIM_FLAGS | CMIF_NOTONLINE;
	if (getBuddyId(hContact, 1) || state == 0 || isChatRoom)
		mi.flags |= CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hAddToServerListContextMenuItem,(LPARAM)&mi);

	DBVARIANT dbv;
	if (!getString(hContact, AIM_KEY_SN, &dbv)) 
	{
		mi.flags = CMIM_NAME | CMIM_FLAGS;
		switch(pd_mode)
		{
		case 1:
			mi.pszName = LPGEN("&Block");
			break;

		case 2:
			mi.pszName = LPGEN("&Unblock");
			break;

		case 3:
			mi.pszName = (char*)(allow_list.find_id(dbv.pszVal) ? LPGEN("&Block") : LPGEN("&Unblock"));
			break;

		case 4:
			mi.pszName = (char*)(block_list.find_id(dbv.pszVal) ? LPGEN("&Unblock") : LPGEN("&Block"));
			break;

		default:
			mi.pszName = LPGEN("&Block");
			mi.flags |= CMIF_HIDDEN;
			break;
		}

		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hBlockContextMenuItem, (LPARAM)&mi);
		DBFreeVariant(&dbv);
	}
   
	return 0;
}

void CAimProto::InitMainMenus(void)
{
	//Do not put any services below HTML get away message!!!
	char service_name[200];

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);

	HGENMENU hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL)
	{
		mi.flags = CMIF_ROOTPOPUP | CMIF_ICONFROMICOLIB | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = GetIconHandle("aim");
		mi.ptszName = m_tszUserName;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.popupPosition = 500090000;
		mi.position = 500090000;
		hRoot = hMenuRoot = (HGENMENU)CallService(MS_CLIST_ADDPROTOMENUITEM,  (WPARAM)0, (LPARAM)&mi);
	}
	else
	{
		RemoveMainMenus();
		hMenuRoot = NULL;
	}

	mi.pszService = service_name;
	mi.hParentMenu = hRoot;
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_CHILDPOPUP;

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/ManageAccount");
	CreateProtoService("/ManageAccount", &CAimProto::ManageAccount);
	mi.position = 201001;
	mi.icolibItem = GetIconHandle("aim");
	mi.pszName = LPGEN("Manage Account");
	hMainMenu[0] = (HGENMENU)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/InstantIdle");
	CreateProtoService("/InstantIdle",&CAimProto::InstantIdle);
	mi.position = 201002;
	mi.icolibItem = GetIconHandle("idle");
	mi.pszName = LPGEN("Instant Idle");
	hMainMenu[1] = (HGENMENU)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/JoinChatRoom");
	CreateProtoService("/JoinChatRoom", &CAimProto::JoinChatUI);
	mi.position = 201003;
	mi.icolibItem = GetIconHandle("aol");
	mi.pszName = LPGEN( "Join Chat Room" );
	hMainMenu[2] = (HGENMENU)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);
}

void CAimProto::InitContactMenus(void)
{
	//Do not put any services below HTML get away message!!!
	char service_name[200];

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.pszService = service_name;
	mi.pszContactOwner = m_szModuleName;

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/GetHTMLAwayMsg");
	CreateProtoService("/GetHTMLAwayMsg",&CAimProto::GetHTMLAwayMsg);
	mi.position = -2000006000;
	mi.icolibItem = GetIconHandle("away");
	mi.pszName = LPGEN("Read &HTML Away Message");
	mi.flags = CMIF_NOTOFFLINE | CMIF_HIDDEN | CMIF_ICONFROMICOLIB;
	hHTMLAwayContextMenuItem = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/GetProfile");
	CreateProtoService("/GetProfile", &CAimProto::GetProfile);
	mi.position = -2000005090;
	mi.icolibItem = GetIconHandle("profile");
	mi.pszName = LPGEN("Read Profile");
	mi.flags = CMIF_NOTOFFLINE | CMIF_ICONFROMICOLIB;
	hReadProfileMenuItem = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/AddToServerList");
	CreateProtoService("/AddToServerList", &CAimProto::AddToServerList); 
	mi.position = -2000005080;
	mi.icolibItem = GetIconHandle("add");
	mi.pszName = LPGEN("Add To Server List");
	mi.flags = CMIF_NOTONLINE | CMIF_HIDDEN | CMIF_ICONFROMICOLIB;
	hAddToServerListContextMenuItem = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mir_snprintf(service_name, sizeof(service_name), "%s%s", m_szModuleName, "/BlockCommand");
	CreateProtoService("/BlockCommand", &CAimProto::BlockBuddy);
	mi.position = -2000005060;
	mi.icolibItem = GetIconHandle("block");
	mi.pszName = LPGEN("&Block");
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_HIDDEN;
	hBlockContextMenuItem = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
}

void CAimProto::RemoveMainMenus(void)
{
	if (hMenuRoot)
		CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)hMenuRoot, 0);
}

void CAimProto::RemoveContactMenus(void)
{
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hHTMLAwayContextMenuItem, 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hReadProfileMenuItem, 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hAddToServerListContextMenuItem, 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hBlockContextMenuItem, 0);
}
