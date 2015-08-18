/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
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

static HGENMENU hEnableDisablePopupMenu;
static HGENMENU hEnableDisableMenu;

//============  MIRANDA PROTOCOL SERVICES  ============

// protocol service function for setting weather protocol status
INT_PTR WeatherSetStatus(WPARAM new_status, LPARAM)
{
	new_status = new_status != ID_STATUS_OFFLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;

	// if we don't want to show status for default station
	if (!opt.NoProtoCondition && status != new_status) {
		old_status = status;
		status = new_status != ID_STATUS_OFFLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);

		UpdateMenu(new_status != ID_STATUS_OFFLINE);
		if (new_status != ID_STATUS_OFFLINE)
			UpdateAll(FALSE, FALSE);
	}

	return 0;
}

// get capabilities protocol service function
INT_PTR WeatherGetCaps(WPARAM wParam, LPARAM)
{
	INT_PTR ret = 0;

	switch (wParam) {
	case PFLAGNUM_1:
		// support search and visible list
		ret = PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_EXTSEARCH | PF1_VISLIST | PF1_MODEMSGRECV;
		break;

	case PFLAGNUM_2:
		ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND |
			PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
		break;

	case PFLAGNUM_4:
		ret = PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEADDED |
			PF4_FORCEAUTH;
		break;

	case PFLAGNUM_5: /* this is PFLAGNUM_5 change when alpha SDK is released */
		ret = PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND |
			PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
		break;

	case PFLAG_UNIQUEIDTEXT:
		ret = (INT_PTR)Translate("Station ID");
		break;

	case PFLAG_UNIQUEIDSETTING:
		ret = (INT_PTR)"ID";
		break;
	}
	return ret;
}

// protocol service function to get weather protocol name
INT_PTR WeatherGetName(WPARAM wParam, LPARAM lParam)
{
	strncpy((char*)lParam, WEATHERPROTOTEXT, wParam - 1);
	*((char*)lParam + wParam - 1) = 0;
	return 0;
}

// protocol service function to get the current status of the protocol
INT_PTR WeatherGetStatus(WPARAM, LPARAM)
{
	return status;
}

// protocol service function to get the icon of the protocol
INT_PTR WeatherLoadIcon(WPARAM wParam, LPARAM)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx("main", FALSE)) : 0;
}

static void __cdecl AckThreadProc(HANDLE param)
{
	Sleep(100);
	ProtoBroadcastAck(WEATHERPROTONAME, (DWORD_PTR)param, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

// nothing to do here because weather proto do not need to retrieve contact info form network
// so just return a 0
INT_PTR WeatherGetInfo(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	mir_forkthread(AckThreadProc, (void*)ccs->hContact);
	return 0;
}

// avatars
static const TCHAR *statusStr[] = { _T("Light"), _T("Fog"), _T("SShower"), _T("Snow"), _T("RShower"), _T("Rain"), _T("PCloudy"), _T("Cloudy"), _T("Sunny"), _T("NA") };
static const WORD statusValue[] = { LIGHT, FOG, SSHOWER, SNOW, RSHOWER, RAIN, PCLOUDY, CLOUDY, SUNNY, NA };

INT_PTR WeatherGetAvatarInfo(WPARAM, LPARAM lParam)
{
	TCHAR szSearchPath[MAX_PATH], *chop;
	unsigned  i;
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;

	GetModuleFileName(GetModuleHandle(NULL), szSearchPath, _countof(szSearchPath));
	chop = _tcsrchr(szSearchPath, '\\');

	if (chop) *chop = '\0';
	else szSearchPath[0] = 0;

	int iStatus = db_get_w(pai->hContact, WEATHERPROTONAME, "StatusIcon", 0);
	for (i = 0; i < 10; i++)
		if (statusValue[i] == iStatus)
			break;

	if (i >= 10)
		return GAIR_NOAVATAR;

	pai->format = PA_FORMAT_PNG;
	mir_sntprintf(pai->filename, _countof(pai->filename), _T("%s\\Plugins\\Weather\\%s.png"), szSearchPath, statusStr[i]);
	if (_taccess(pai->filename, 4) == 0)
		return GAIR_SUCCESS;

	pai->format = PA_FORMAT_GIF;
	mir_sntprintf(pai->filename, _countof(pai->filename), _T("%s\\Plugins\\Weather\\%s.gif"), szSearchPath, statusStr[i]);
	if (_taccess(pai->filename, 4) == 0)
		return GAIR_SUCCESS;

	pai->format = PA_FORMAT_UNKNOWN;
	pai->filename[0] = 0;
	return GAIR_NOAVATAR;
}

void AvatarDownloaded(MCONTACT hContact)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = hContact;

	if (WeatherGetAvatarInfo(GAIF_FORCE, (LPARAM)&ai) == GAIR_SUCCESS)
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai, 0);
	else
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
}

static void __cdecl WeatherGetAwayMsgThread(void *arg)
{
	Sleep(100);

	MCONTACT hContact = (DWORD_PTR)arg;
	DBVARIANT dbv;
	if (!db_get_ts(hContact, "CList", "StatusMsg", &dbv)) {
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		db_free(&dbv);
	}
	else ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

static INT_PTR WeatherGetAwayMsg(WPARAM, LPARAM lParam)
{
	CCSDATA* ccs = (CCSDATA*)lParam;
	if (ccs == NULL)
		return 0;

	mir_forkthread(WeatherGetAwayMsgThread, (void*)ccs->hContact);
	return 1;
}

//============  PROTOCOL INITIALIZATION  ============
// protocol services
void InitServices(void)
{
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETCAPS, WeatherGetCaps);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETNAME, WeatherGetName);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_LOADICON, WeatherLoadIcon);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_SETSTATUS, WeatherSetStatus);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETSTATUS, WeatherGetStatus);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_BASICSEARCH, WeatherBasicSearch);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_SEARCHBYEMAIL, WeatherBasicSearch);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_ADDTOLIST, WeatherAddToList);
	CreateProtoServiceFunction(WEATHERPROTONAME, PSS_GETINFO, WeatherGetInfo);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETAVATARINFO, WeatherGetAvatarInfo);
	CreateProtoServiceFunction(WEATHERPROTONAME, PSS_GETAWAYMSG, WeatherGetAwayMsg);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_CREATEADVSEARCHUI, WeatherCreateAdvancedSearchUI);
	CreateProtoServiceFunction(WEATHERPROTONAME, PS_SEARCHBYADVANCED, WeatherAdvancedSearch);

	CreateProtoServiceFunction(WEATHERPROTONAME, MS_WEATHER_GETDISPLAY, GetDisplaySvcFunc);
}

//============  MENU INITIALIZATION  ============

void UpdateMenu(BOOL State)
{
	// update option setting
	opt.CAutoUpdate = State;
	db_set_b(NULL, WEATHERPROTONAME, "AutoUpdate", (BYTE)State);

	if (State) { // to enable auto-update
		Menu_ModifyItem(hEnableDisableMenu, LPGENT("Auto Update Enabled"), GetIconHandle("main"));
		opt.AutoUpdate = 1;
	}
	else { // to disable auto-update
		Menu_ModifyItem(hEnableDisableMenu, LPGENT("Auto Update Disabled"), GetIconHandle("disabled"));
		opt.AutoUpdate = 0;
	}

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTBButton, !State ? TTBST_PUSHED : 0);
}

void UpdatePopupMenu(BOOL State)
{
	// update option setting
	opt.UsePopup = State;
	db_set_b(NULL, WEATHERPROTONAME, "UsePopup", (BYTE)opt.UsePopup);

	if (State) // to enable popup
		Menu_ModifyItem(hEnableDisablePopupMenu, LPGENT("Disable &weather notification"), GetIconHandle("popup"));
	else // to disable popup
		Menu_ModifyItem(hEnableDisablePopupMenu, LPGENT("Enable &weather notification"), GetIconHandle("nopopup"));
}

// update the weather auto-update menu item when click on it
INT_PTR EnableDisableCmd(WPARAM wParam, LPARAM lParam)
{
	UpdateMenu(wParam == TRUE ? (BOOL)lParam : !opt.CAutoUpdate);
	return 0;
}

// update the weather popup menu item when click on it
INT_PTR MenuitemNotifyCmd(WPARAM, LPARAM)
{
	UpdatePopupMenu(!opt.UsePopup);
	return 0;
}

// adding weather contact menus
// copied and modified form "modified MSN Protocol"
void AddMenuItems(void)
{
	CMenuItem mi;

	// contact menu
	CreateServiceFunction(MS_WEATHER_UPDATE, UpdateSingleStation);
	mi.position = -0x7FFFFFFA;
	mi.hIcolibItem = GetIconHandle("update");
	mi.name.a = LPGEN("Update Weather");
	mi.pszService = MS_WEATHER_UPDATE;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	CreateServiceFunction(MS_WEATHER_REFRESH, UpdateSingleRemove);
	mi.position = -0x7FFFFFF9;
	mi.hIcolibItem = GetIconHandle("update2");
	mi.name.a = LPGEN("Remove Old Data then Update");
	mi.pszService = MS_WEATHER_REFRESH;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	CreateServiceFunction(MS_WEATHER_BRIEF, BriefInfoSvc);
	mi.position = -0x7FFFFFF8;
	mi.hIcolibItem = GetIconHandle("brief");
	mi.name.a = LPGEN("Brief Information");
	mi.pszService = MS_WEATHER_BRIEF;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	CreateServiceFunction(MS_WEATHER_COMPLETE, LoadForecast);
	mi.position = -0x7FFFFFF7;
	mi.hIcolibItem = GetIconHandle("read");
	mi.name.a = LPGEN("Read Complete Forecast");
	mi.pszService = MS_WEATHER_COMPLETE;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	CreateServiceFunction(MS_WEATHER_MAP, WeatherMap);
	mi.position = -0x7FFFFFF6;
	mi.hIcolibItem = GetIconHandle("map");
	mi.name.a = LPGEN("Weather Map");
	mi.pszService = MS_WEATHER_MAP;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	CreateServiceFunction(MS_WEATHER_LOG, ViewLog);
	mi.position = -0x7FFFFFF5;
	mi.hIcolibItem = GetIconHandle("log");
	mi.name.a = LPGEN("View Log");
	mi.pszService = MS_WEATHER_LOG;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	CreateServiceFunction(MS_WEATHER_EDIT, EditSettings);
	mi.position = -0x7FFFFFF4;
	mi.hIcolibItem = GetIconHandle("edit");
	mi.name.a = LPGEN("Edit Settings");
	mi.pszService = MS_WEATHER_EDIT;
	Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);

	// adding main menu items
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("Weather"), 500099000);

	CreateServiceFunction(MS_WEATHER_ENABLED, EnableDisableCmd);
	mi.name.a = LPGEN("Enable/Disable Weather Update");
	mi.hIcolibItem = GetIconHandle("main");
	mi.position = 10100001;
	mi.pszService = MS_WEATHER_ENABLED;
	hEnableDisableMenu = Menu_AddMainMenuItem(&mi);
	UpdateMenu(opt.AutoUpdate);

	CreateServiceFunction(MS_WEATHER_UPDATEALL, UpdateAllInfo);
	mi.position = 20100001;
	mi.hIcolibItem = GetIconHandle("update");
	mi.name.a = LPGEN("Update All Weather");
	mi.pszService = MS_WEATHER_UPDATEALL;
	Menu_AddMainMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_REFRESHALL, UpdateAllRemove);
	mi.position = 20100002;
	mi.hIcolibItem = GetIconHandle("update2");
	mi.name.a = LPGEN("Remove Old Data then Update All");
	mi.pszService = MS_WEATHER_REFRESHALL;
	Menu_AddMainMenuItem(&mi);

	// only run if popup service exists
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		CreateServiceFunction(WEATHERPROTONAME "/PopupMenu", MenuitemNotifyCmd);
		mi.name.a = LPGEN("Weather Notification");
		mi.hIcolibItem = GetIconHandle("popup");
		mi.position = 0;
		mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("Popups"), 0);
		mi.pszService = WEATHERPROTONAME "/PopupMenu";
		hEnableDisablePopupMenu = Menu_AddMainMenuItem(&mi);
		UpdatePopupMenu(opt.UsePopup);
	}

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		mi.pszService = "Weather/mwin_menu";
		CreateServiceFunction(mi.pszService, Mwin_MenuClicked);
		mi.position = -0x7FFFFFF0;
		mi.hIcolibItem = NULL;
		mi.root = NULL;
		mi.name.a = LPGEN("Display in a frame");
		hMwinMenu = Menu_AddContactMenuItem(&mi, WEATHERPROTONAME);
	}
}
