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
	Icon_Register(hInstance, "Protocols/AIM/" LPGEN("Profile Editor"), iconList+14, 14, "AIM");
}

HICON LoadIconEx(const char *name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, "AIM_%s", name);
	return IcoLib_GetIcon(szSettingName, big);
}

HANDLE GetIconHandle(const char *name)
{
	for (int i=0; i < _countof(iconList); i++)
		if ( !mir_strcmp(iconList[i].szName, name))
			return iconList[i].hIcolib;

	return NULL;
}

void ReleaseIconEx(const char *name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, "%s_%s", "AIM", name);
	IcoLib_Release(szSettingName, big);
}

void Window_SetIcon(HWND hWnd, const char* name)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)LoadIconEx( name, true ));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx( name ));
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
	hExtraAT = ExtraIcon_RegisterIcolib("aimaccounttype", LPGEN("AIM Account Type"), "AIM_aol");
	hExtraES = ExtraIcon_RegisterIcolib("aimextstatus", LPGEN("AIM Extended Status"), "AIM_hiptop");
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPreBuildContactMenu

int CAimProto::OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bIsChatRoom = isChatRoom(hContact);

	//see if we should add the html away message context menu items
	Menu_ShowItem(m_hHTMLAwayContextMenuItem, getWord(hContact, AIM_KEY_ST, ID_STATUS_OFFLINE) == ID_STATUS_AWAY && !bIsChatRoom);
	Menu_ShowItem(m_hAddToServerListContextMenuItem, !getBuddyId(hContact, 1) && m_state != 0 && !bIsChatRoom);

	ptrA id(getStringA(hContact, AIM_KEY_SN));
	if (id == NULL)
		return 0;

	switch (m_pd_mode) {
	case 1:
		Menu_ModifyItem(m_hBlockContextMenuItem, LPGENT("&Block"));
		break;

	case 2:
		Menu_ModifyItem(m_hBlockContextMenuItem, LPGENT("&Unblock"));
		break;

	case 3:
		Menu_ModifyItem(m_hBlockContextMenuItem, m_allow_list.find_id(id) ? LPGENT("&Block") : LPGENT("&Unblock"));
		break;

	case 4:
		Menu_ModifyItem(m_hBlockContextMenuItem, m_block_list.find_id(id) ? LPGENT("&Unblock") : LPGENT("&Block"));
		break;

	default:
		Menu_ShowItem(m_hBlockContextMenuItem, false);
		break;
	}
	return 0;
}

void CAimProto::InitMainMenus(void)
{
	CMenuItem mi;
	mi.root = Menu_GetProtocolRoot(this);

	mi.pszService = "/ManageAccount";
	CreateProtoService(mi.pszService, &CAimProto::ManageAccount);
	mi.position = 201001;
	mi.hIcolibItem = GetIconHandle("aim");
	mi.name.a = LPGEN("Manage Account");
	m_hMainMenu[0] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/InstantIdle";
	CreateProtoService(mi.pszService, &CAimProto::InstantIdle);
	mi.position = 201002;
	mi.hIcolibItem = GetIconHandle("idle");
	mi.name.a = LPGEN("Instant Idle");
	m_hMainMenu[1] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/JoinChatRoom";
	CreateProtoService(mi.pszService, &CAimProto::JoinChatUI);
	mi.position = 201003;
	mi.hIcolibItem = GetIconHandle("aol");
	mi.name.a = LPGEN( "Join Chat Room" );
	m_hMainMenu[2] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CAimProto::InitContactMenus(void)
{
	CMenuItem mi;

	CreateProtoService("/GetHTMLAwayMsg",&CAimProto::GetHTMLAwayMsg);
	mi.pszService = "/GetHTMLAwayMsg";
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle("away");
	mi.name.a = LPGEN("Read &HTML Away Message");
	mi.flags = CMIF_NOTOFFLINE | CMIF_HIDDEN;
	m_hHTMLAwayContextMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);

	CreateProtoService("/GetProfile", &CAimProto::GetProfile);
	mi.pszService = "/GetProfile";
	mi.position = -2000005090;
	mi.hIcolibItem = GetIconHandle("profile");
	mi.name.a = LPGEN("Read Profile");
	mi.flags = CMIF_NOTOFFLINE;
	Menu_AddContactMenuItem(&mi, m_szModuleName);

	CreateProtoService("/AddToServerList", &CAimProto::AddToServerList); 
	mi.pszService = "/AddToServerList";
	mi.position = -2000005080;
	mi.hIcolibItem = GetIconHandle("add");
	mi.name.a = LPGEN("Add To Server List");
	mi.flags = CMIF_NOTONLINE | CMIF_HIDDEN;
	m_hAddToServerListContextMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);

	CreateProtoService("/BlockCommand", &CAimProto::BlockBuddy);
	mi.pszService = "/BlockCommand";
	mi.position = -2000005060;
	mi.hIcolibItem = GetIconHandle("block");
	mi.name.a = LPGEN("&Block");
	mi.flags = CMIF_HIDDEN;
	m_hBlockContextMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);
}
