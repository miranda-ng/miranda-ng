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
#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Icons init

static IconItem iconList[] =
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

	{	LPGEN("Foreground Color"),       "foreclr",     IDI_FOREGROUNDCOLOR },
	{	LPGEN("Background Color"),       "backclr",     IDI_BACKGROUNDCOLOR },
	{	LPGEN("Bold"),                   "bold",        IDI_BOLD            },
	{	LPGEN("Not Bold"),               "nbold",       IDI_NBOLD           },
	{	LPGEN("Italic"),                 "italic",      IDI_ITALIC          },
	{	LPGEN("Not Italic"),             "nitalic",     IDI_NITALIC         },
	{	LPGEN("Underline"),              "undrln",      IDI_UNDERLINE       },
	{	LPGEN("Not Underline"),          "nundrln",     IDI_NUNDERLINE      },
	{	LPGEN("Subscript"),              "sub_scrpt",   IDI_SUBSCRIPT       },
	{	LPGEN("Not Subscript"),          "nsub_scrpt",  IDI_NSUBSCRIPT      },
	{	LPGEN("Superscript"),            "sup_scrpt",   IDI_SUPERSCRIPT     },
	{	LPGEN("Not Superscript"),        "nsup_scrpt",  IDI_NSUPERSCRIPT    },
	{	LPGEN("Normal Script"),          "norm_scrpt",  IDI_NORMALSCRIPT    },
	{	LPGEN("Not Normal Script"),      "nnorm_scrpt", IDI_NNORMALSCRIPT   }
};

void InitIcons(void)
{
	Icon_Register(hInstance, "Protocols/AIM", iconList, 14, "AIM");
	Icon_Register(hInstance, "Protocols/AIM/"LPGEN("Profile Editor"), iconList+14, 14, "AIM");
}

HICON LoadIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "AIM_%s", name);
	return Skin_GetIcon(szSettingName, big);
}

HANDLE GetIconHandle(const char* name)
{
	for (int i=0; i < SIZEOF(iconList); i++)
		if ( !mir_strcmp(iconList[i].szName, name))
			return iconList[i].hIcolib;

	return NULL;
}

void ReleaseIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName ), "%s_%s", "AIM", name);
	Skin_ReleaseIcon(szSettingName, big);
}

void WindowSetIcon(HWND hWnd, const char* name)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)LoadIconEx( name, true ));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx( name ));
}

void WindowFreeIcon(HWND hWnd)
{
	Skin_ReleaseIcon((HICON)SendMessage(hWnd, WM_SETICON, ICON_BIG, 0));
	Skin_ReleaseIcon((HICON)SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Extra Icons

extern OBJLIST<CAimProto> g_Instances;

static HANDLE bot_icon, icq_icon, aol_icon, hiptop_icon;
static HANDLE admin_icon, confirmed_icon, unconfirmed_icon;

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

static void set_AT_icon(CAimProto* ppro, MCONTACT hContact)
{
	if (ppro->isChatRoom(hContact)) return;

	unsigned i = ppro->getByte(hContact, AIM_KEY_AC, 0) - 1;
	ExtraIcon_SetIcon(hExtraAT, hContact, (i < 5) ? GetIconHandle(extra_AT_icon_name[i]) : NULL);
}

static void set_ES_icon(CAimProto* ppro, MCONTACT hContact)
{
	if (ppro->isChatRoom(hContact)) return;

	unsigned i = ppro->getByte(hContact, AIM_KEY_ET, 0) - 1;
	ExtraIcon_SetIcon(hExtraAT, hContact, (i < 2) ? GetIconHandle(extra_ES_icon_name[i]) : NULL);
}

void set_contact_icon(CAimProto* ppro, MCONTACT hContact)
{
	if (!ppro->getByte(AIM_KEY_AT, 0)) set_AT_icon(ppro, hContact);
	if (!ppro->getByte(AIM_KEY_ES, 0)) set_ES_icon(ppro, hContact);
}

void remove_AT_icons(CAimProto* ppro)
{
	for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName))
		if (!ppro->isChatRoom(hContact)) 
			ExtraIcon_Clear(hExtraAT, hContact);
}

void remove_ES_icons(CAimProto* ppro)
{
	for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName))
		if (!ppro->isChatRoom(hContact)) 
			ExtraIcon_Clear(hExtraES, hContact);
}

void add_AT_icons(CAimProto* ppro)
{
	for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName))
		set_AT_icon(ppro, hContact);
}

void add_ES_icons(CAimProto* ppro)
{
	for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName))
		set_ES_icon(ppro, hContact);
}

void InitExtraIcons(void)
{
	hExtraAT = ExtraIcon_Register("aimaccounttype", LPGEN("AIM Account Type"), "AIM_aol");
	hExtraES = ExtraIcon_Register("aimextstatus", LPGEN("AIM Extended Status"), "AIM_hiptop");
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPreBuildContactMenu

int CAimProto::OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bIsChatRoom = isChatRoom(hContact);

	//see if we should add the html away message context menu items
	Menu_ShowItem(hHTMLAwayContextMenuItem, getWord(hContact, AIM_KEY_ST, ID_STATUS_OFFLINE) == ID_STATUS_AWAY && !bIsChatRoom);
	Menu_ShowItem(hAddToServerListContextMenuItem, !getBuddyId(hContact, 1) && state != 0 && !bIsChatRoom);

	DBVARIANT dbv;
	if (!getString(hContact, AIM_KEY_SN, &dbv)) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_NAME | CMIM_FLAGS;
		switch(pd_mode) {
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

		Menu_ModifyItem(hBlockContextMenuItem, &mi);
		db_free(&dbv);
	}
   
	return 0;
}

void CAimProto::InitMainMenus(void)
{
	//Do not put any services below HTML get away message!!!
	char service_name[200];

	CLISTMENUITEM mi = { sizeof(mi) };

	HGENMENU hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL) {
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = GetIconHandle("aim");
		mi.ptszName = m_tszUserName;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.popupPosition = 500090000;
		mi.position = 500090000;
		hRoot = hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else {
		RemoveMainMenus();
		hMenuRoot = NULL;
	}

	mi.pszService = service_name;
	mi.hParentMenu = hRoot;
	mi.flags = CMIF_CHILDPOPUP;

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/ManageAccount");
	CreateProtoService("/ManageAccount", &CAimProto::ManageAccount);
	mi.position = 201001;
	mi.icolibItem = GetIconHandle("aim");
	mi.pszName = LPGEN("Manage Account");
	hMainMenu[0] = Menu_AddProtoMenuItem(&mi);

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/InstantIdle");
	CreateProtoService("/InstantIdle",&CAimProto::InstantIdle);
	mi.position = 201002;
	mi.icolibItem = GetIconHandle("idle");
	mi.pszName = LPGEN("Instant Idle");
	hMainMenu[1] = Menu_AddProtoMenuItem(&mi);

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/JoinChatRoom");
	CreateProtoService("/JoinChatRoom", &CAimProto::JoinChatUI);
	mi.position = 201003;
	mi.icolibItem = GetIconHandle("aol");
	mi.pszName = LPGEN( "Join Chat Room" );
	hMainMenu[2] = Menu_AddProtoMenuItem(&mi);
}

void CAimProto::InitContactMenus(void)
{
	//Do not put any services below HTML get away message!!!
	char service_name[200];

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = service_name;
	mi.pszContactOwner = m_szModuleName;

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/GetHTMLAwayMsg");
	CreateProtoService("/GetHTMLAwayMsg",&CAimProto::GetHTMLAwayMsg);
	mi.position = -2000006000;
	mi.icolibItem = GetIconHandle("away");
	mi.pszName = LPGEN("Read &HTML Away Message");
	mi.flags = CMIF_NOTOFFLINE | CMIF_HIDDEN;
	hHTMLAwayContextMenuItem = Menu_AddContactMenuItem(&mi);

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/GetProfile");
	CreateProtoService("/GetProfile", &CAimProto::GetProfile);
	mi.position = -2000005090;
	mi.icolibItem = GetIconHandle("profile");
	mi.pszName = LPGEN("Read Profile");
	mi.flags = CMIF_NOTOFFLINE;
	hReadProfileMenuItem = Menu_AddContactMenuItem(&mi);

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/AddToServerList");
	CreateProtoService("/AddToServerList", &CAimProto::AddToServerList); 
	mi.position = -2000005080;
	mi.icolibItem = GetIconHandle("add");
	mi.pszName = LPGEN("Add To Server List");
	mi.flags = CMIF_NOTONLINE | CMIF_HIDDEN;
	hAddToServerListContextMenuItem = Menu_AddContactMenuItem(&mi);

	mir_snprintf(service_name, SIZEOF(service_name), "%s%s", m_szModuleName, "/BlockCommand");
	CreateProtoService("/BlockCommand", &CAimProto::BlockBuddy);
	mi.position = -2000005060;
	mi.icolibItem = GetIconHandle("block");
	mi.pszName = LPGEN("&Block");
	mi.flags = CMIF_HIDDEN;
	hBlockContextMenuItem = Menu_AddContactMenuItem(&mi);
}

void CAimProto::RemoveMainMenus(void)
{
	if (hMenuRoot)
		CallService(MO_REMOVEMENUITEM, (WPARAM)hMenuRoot, 0);
}

void CAimProto::RemoveContactMenus(void)
{
	CallService(MO_REMOVEMENUITEM, (WPARAM)hHTMLAwayContextMenuItem, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)hReadProfileMenuItem, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)hAddToServerListContextMenuItem, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)hBlockContextMenuItem, 0);
}
