/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
This file contain the source related to weather protocol services
as required for a Miranda protocol. Also, it contains functions for
building/changing the weather menu items.
*/

#include "stdafx.h"

static HGENMENU hEnableDisableMenu;

extern VARSW g_pwszIconsName;

/////////////////////////////////////////////////////////////////////////////////////////
// avatars

// these data shall go in the same order as EWeatherCondition members
struct
{
	const wchar_t *pwszName;
	int iconIdx, status, clistIconId;
}
static statusIcons[MAX_COND] = 
{
	{ L"Sunny",   104,  ID_STATUS_ONLINE },
	{ L"NA",      105,  ID_STATUS_OFFLINE }, 
	{ L"PCloudy", 128,  ID_STATUS_AWAY },
	{ L"Cloudy",  131,  ID_STATUS_NA }, 
	{ L"Rain",    159,  ID_STATUS_OCCUPIED }, 
	{ L"RShower", 129,  ID_STATUS_OCCUPIED }, 
	{ L"Fog",     1003, ID_STATUS_DND }, 
	{ L"Snow",    158,  ID_STATUS_FREECHAT }, 
	{ L"SShower", 1002, ID_STATUS_FREECHAT }, 
	{ L"Light",   130,  ID_STATUS_INVISIBLE },
};

INT_PTR CWeatherProto::GetAvatarInfoSvc(WPARAM, LPARAM lParam)
{
	wchar_t szSearchPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(nullptr), szSearchPath, _countof(szSearchPath));

	wchar_t *chop = wcsrchr(szSearchPath, '\\');
	if (chop)
		*chop = '\0';
	else
		szSearchPath[0] = 0;

	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;
	int iCond = getWord(pai->hContact, "StatusIcon", -1);
	if (iCond < 0 || iCond >= MAX_COND)
		return GAIR_NOAVATAR;

	pai->format = PA_FORMAT_PNG;
	mir_snwprintf(pai->filename, L"%s\\Plugins\\Weather\\%s.png", szSearchPath, statusIcons[iCond].pwszName);
	if (_waccess(pai->filename, 4) == 0)
		return GAIR_SUCCESS;

	pai->format = PA_FORMAT_GIF;
	mir_snwprintf(pai->filename, L"%s\\Plugins\\Weather\\%s.gif", szSearchPath, statusIcons[iCond].pwszName);
	if (_waccess(pai->filename, 4) == 0)
		return GAIR_SUCCESS;

	pai->format = PA_FORMAT_UNKNOWN;
	pai->filename[0] = 0;
	return GAIR_NOAVATAR;
}

void CWeatherProto::AvatarDownloaded(MCONTACT hContact)
{
	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = hContact;

	if (GetAvatarInfoSvc(GAIF_FORCE, (LPARAM)&ai) == GAIR_SUCCESS)
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
	else
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// advanced status icons

void ClearStatusIcons()
{
	for (auto &it : statusIcons)
		it.clistIconId = 0;
}

int CWeatherProto::MapCondToStatus(MCONTACT hContact)
{
	int iCond = getWord(hContact, "StatusIcon", -1);
	if (iCond < 0 || iCond >= MAX_COND)
		return ID_STATUS_OFFLINE;

	return statusIcons[iCond].status;
}

HICON CWeatherProto::GetStatusIcon(MCONTACT hContact)
{
	int iCond = getWord(hContact, "StatusIcon", -1);
	if (iCond < 0 || iCond >= MAX_COND)
		return nullptr;

	auto &pIcon = statusIcons[iCond];
	if (pIcon.clistIconId == 0)
		pIcon.clistIconId = ImageList_AddIcon(Clist_GetImageList(), LoadIconA(g_plugin.hIconsDll, MAKEINTRESOURCEA(pIcon.iconIdx)));

	return ImageList_GetIcon(Clist_GetImageList(), pIcon.clistIconId, ILD_NORMAL);
}

HICON CWeatherProto::GetStatusIconBig(MCONTACT hContact)
{
	int iCond = getWord(hContact, "StatusIcon", -1);
	if (iCond < 0 || iCond >= MAX_COND)
		return nullptr;

	HICON hIcon;
	auto &pIcon = statusIcons[iCond];
	ExtractIconExW(g_pwszIconsName, -pIcon.iconIdx, &hIcon, 0, 1);
	return hIcon;
}

INT_PTR CWeatherProto::AdvancedStatusIconSvc(WPARAM hContact, LPARAM)
{
	if (!hContact || !g_plugin.hIconsDll)
		return -1;

	int iCond = getWord(hContact, "StatusIcon", -1);
	if (iCond < 0 || iCond >= MAX_COND)
		return -1;

	auto &pIcon = statusIcons[iCond];
	if (pIcon.clistIconId == 0)
		pIcon.clistIconId = ImageList_AddIcon(Clist_GetImageList(), LoadIconA(g_plugin.hIconsDll, MAKEINTRESOURCEA(pIcon.iconIdx)));

	return MAKELONG(0, pIcon.clistIconId);
}

/////////////////////////////////////////////////////////////////////////////////////////
// menus

void CWeatherProto::UpdateMenu(BOOL State)
{
	// update option setting
	opt.CAutoUpdate = State;
	setByte("AutoUpdate", (uint8_t)State);

	if (State) { // to enable auto-update
		Menu_ModifyItem(hEnableDisableMenu, LPGENW("Auto Update Enabled"), g_plugin.getIconHandle(IDI_ICON));
		opt.AutoUpdate = 1;
	}
	else { // to disable auto-update
		Menu_ModifyItem(hEnableDisableMenu, LPGENW("Auto Update Disabled"), g_plugin.getIconHandle(IDI_DISABLED));
		opt.AutoUpdate = 0;
	}

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTBButton, !State ? TTBST_PUSHED : 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update the weather auto-update menu item when click on it

INT_PTR CWeatherProto::EnableDisableCmd(WPARAM wParam, LPARAM lParam)
{
	UpdateMenu(wParam == TRUE ? (BOOL)lParam : !opt.CAutoUpdate);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// adding weather contact menus

void CWeatherProto::InitMenuItems()
{
	CMenuItem mi(&g_plugin);

	// contact menu
	SET_UID(mi, 0x266ef52b, 0x869a, 0x4cac, 0xa9, 0xf8, 0xea, 0x5b, 0xb8, 0xab, 0xe0, 0x24);
	mi.position = -0x7FFFFFFA;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_UPDATE);
	mi.name.a = LPGEN("Update Weather");
	mi.pszService = "/Update";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::UpdateSingleStation);

	SET_UID(mi, 0x45361b4, 0x8de, 0x44b4, 0x8f, 0x11, 0x9b, 0xe9, 0x6e, 0xa8, 0x83, 0x54);
	mi.position = -0x7FFFFFF9;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_UPDATE2);
	mi.name.a = LPGEN("Remove Old Data then Update");
	mi.pszService = "/Refresh";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::UpdateSingleRemove);

	SET_UID(mi, 0x4232975e, 0xb181, 0x46a5, 0xb7, 0x6e, 0xd2, 0x5f, 0xef, 0xb8, 0xc4, 0x4d);
	mi.position = -0x7FFFFFF8;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_S);
	mi.name.a = LPGEN("Brief Information");
	mi.pszService = "/Brief";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::BriefInfo);

	SET_UID(mi, 0x3d6ed729, 0xd49a, 0x4ae9, 0x8e, 0x2, 0x9f, 0xe0, 0xf0, 0x2c, 0xcc, 0xb1);
	mi.position = -0x7FFFFFF7;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_READ);
	mi.name.a = LPGEN("Read Complete Forecast");
	mi.pszService = "/CompleteForecast";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::LoadForecast);

	SET_UID(mi, 0xc4b6c5e0, 0x13c3, 0x4e02, 0x8a, 0xeb, 0xeb, 0x8a, 0xe2, 0x66, 0x40, 0xd4);
	mi.position = -0x7FFFFFF6;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAP);
	mi.name.a = LPGEN("Weather Map");
	mi.pszService = "/Map";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::WeatherMap);

	SET_UID(mi, 0xee3ad7f4, 0x3377, 0x4e4c, 0x8f, 0x3c, 0x3b, 0xf5, 0xd4, 0x86, 0x28, 0x25);
	mi.position = -0x7FFFFFF5;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_LOG);
	mi.name.a = LPGEN("View Log");
	mi.pszService = "/Log";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::ViewLog);

	SET_UID(mi, 0x1b01cd6a, 0xe5ee, 0x42b4, 0xa1, 0x6d, 0x43, 0xb9, 0x4, 0x58, 0x43, 0x2e);
	mi.position = -0x7FFFFFF4;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_EDIT);
	mi.name.a = LPGEN("Edit Settings");
	mi.pszService = "/Edit";
	Menu_AddContactMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CWeatherProto::EditSettings);

	// adding main menu items
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Weather"), 500099000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "82809D2F-2CF0-4E15-9350-D257A7748552");

	SET_UID(mi, 0x5ad16188, 0xe0a0, 0x4c31, 0x85, 0xc3, 0xe4, 0x85, 0x79, 0x7e, 0x4b, 0x9c);
	mi.name.a = LPGEN("Enable/Disable Weather Update");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON);
	mi.position = 10100001;
	mi.pszService = "/EnableDisable";
	hEnableDisableMenu = Menu_AddMainMenuItem(&mi);
	UpdateMenu(opt.AutoUpdate);
	CreateProtoService(mi.pszService, &CWeatherProto::EnableDisableCmd);

	SET_UID(mi, 0x2b1c2054, 0x2991, 0x4025, 0x87, 0x73, 0xb6, 0xf7, 0x85, 0xac, 0xc7, 0x37);
	mi.position = 20100001;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_UPDATE);
	mi.name.a = LPGEN("Update All Weather");
	mi.pszService = "/UpdateAll";
	Menu_AddMainMenuItem(&mi);
	CreateProtoService(mi.pszService, &CWeatherProto::UpdateAllInfo);

	SET_UID(mi, 0x8234c00e, 0x788e, 0x424f, 0xbc, 0xc4, 0x2, 0xfd, 0x67, 0x58, 0x2d, 0x19);
	mi.position = 20100002;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_UPDATE2);
	mi.name.a = LPGEN("Remove Old Data then Update All");
	mi.pszService = "/RefreshAll";
	Menu_AddMainMenuItem(&mi);
	CreateProtoService(mi.pszService, &CWeatherProto::UpdateAllRemove);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		SET_UID(mi, 0xe193fe9b, 0xf6ad, 0x41ac, 0x95, 0x29, 0x45, 0x4, 0x44, 0xb1, 0xeb, 0x5d);
		mi.pszService = "/mwin_menu";
		CreateProtoService(mi.pszService, &CWeatherProto::Mwin_MenuClicked);
		mi.position = -0x7FFFFFF0;
		mi.hIcolibItem = nullptr;
		mi.root = nullptr;
		mi.name.a = LPGEN("Display in a frame");
		hMwinMenu = Menu_AddContactMenuItem(&mi, m_szModuleName);
	}
}
