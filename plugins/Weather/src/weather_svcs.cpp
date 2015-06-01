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

#include "weather.h"

static HGENMENU hEnableDisablePopupMenu;
static HGENMENU hEnableDisableMenu;

//============  MIRANDA PROTOCOL SERVICES  ============

// protocol service function for setting weather protocol status
INT_PTR WeatherSetStatus(WPARAM new_status, LPARAM lParam)
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
INT_PTR WeatherGetCaps(WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = 0;

	switch(wParam) {
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
INT_PTR WeatherGetName(WPARAM wParam,LPARAM lParam)
{
	strncpy((char*)lParam,WEATHERPROTOTEXT,wParam-1);
	*((char*)lParam + wParam-1) = 0;
	return 0;
}

// protocol service function to get the current status of the protocol
INT_PTR WeatherGetStatus(WPARAM wParam,LPARAM lParam)
{
	return status;
}

// protocol service function to get the icon of the protocol
INT_PTR WeatherLoadIcon(WPARAM wParam,LPARAM lParam)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx("main", FALSE)) : 0;
}

static void __cdecl AckThreadProc(HANDLE param)
{
	Sleep(100);
	ProtoBroadcastAck(WEATHERPROTONAME, (MCONTACT)param, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

// nothing to do here because weather proto do not need to retrieve contact info form network
// so just return a 0
INT_PTR WeatherGetInfo(WPARAM,LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *) lParam;
	mir_forkthread(AckThreadProc, (void*)ccs->hContact);
	return 0;
}

// avatars
static const TCHAR *statusStr[] = { _T("Light"), _T("Fog"), _T("SShower"), _T("Snow"), _T("RShower"), _T("Rain"), _T("PCloudy"), _T("Cloudy"), _T("Sunny"), _T("NA") };
static const WORD statusValue[] = { LIGHT, FOG, SSHOWER, SNOW, RSHOWER, RAIN, PCLOUDY, CLOUDY, SUNNY, NA };

INT_PTR WeatherGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	TCHAR szSearchPath[MAX_PATH], *chop;
	WORD status;
	unsigned  i;
	PROTO_AVATAR_INFORMATION* ai = ( PROTO_AVATAR_INFORMATION* )lParam;

	GetModuleFileName(GetModuleHandle(NULL), szSearchPath, SIZEOF(szSearchPath));
	chop = _tcsrchr(szSearchPath, '\\');

	if (chop) *chop = '\0';
	else szSearchPath[0] = 0;

	status = (WORD)db_get_w(ai->hContact, WEATHERPROTONAME, "StatusIcon",0);
	for (i=0; i<10; i++)
		if (statusValue[i] == status)
			break;

	if (i >= 10)
		return GAIR_NOAVATAR;

	ai->format = PA_FORMAT_PNG;
	mir_sntprintf(ai->filename, SIZEOF(ai->filename), _T("%s\\Plugins\\Weather\\%s.png"), szSearchPath, statusStr[i]);
	if ( _taccess(ai->filename, 4) == 0)
		return GAIR_SUCCESS;

	ai->format = PA_FORMAT_GIF;
	mir_sntprintf(ai->filename, SIZEOF(ai->filename), _T("%s\\Plugins\\Weather\\%s.gif"), szSearchPath, statusStr[i]);
	if ( _taccess(ai->filename, 4) == 0)
		return GAIR_SUCCESS;

	ai->format = PA_FORMAT_UNKNOWN;
	ai->filename[0] = 0;
	return GAIR_NOAVATAR;
}


void AvatarDownloaded(MCONTACT hContact)
{
	PROTO_AVATAR_INFORMATION AI = {0};
	AI.hContact = hContact;

	if (WeatherGetAvatarInfo(GAIF_FORCE, (LPARAM)&AI) == GAIR_SUCCESS)
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &AI, 0);
	else
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
}


static void __cdecl WeatherGetAwayMsgThread(void *hContact)
{
	Sleep(100);

	DBVARIANT dbv;
	if (!db_get_ts((MCONTACT)hContact, "CList", "StatusMsg", &dbv)) {
		ProtoBroadcastAck(WEATHERPROTONAME, (MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		db_free( &dbv );
	}
	else ProtoBroadcastAck(WEATHERPROTONAME, (MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

static INT_PTR WeatherGetAwayMsg(WPARAM wParam, LPARAM lParam)
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

	CLISTMENUITEM mi = { sizeof(mi) };

	if (State) { // to enable auto-update
		mi.pszName = LPGEN("Auto Update Enabled");
		mi.icolibItem = GetIconHandle("main");
		opt.AutoUpdate = 1;
	}
	else { // to disable auto-update
		mi.pszName = LPGEN("Auto Update Disabled");
		mi.icolibItem = GetIconHandle("disabled");
		opt.AutoUpdate = 0;
	}

	mi.flags = CMIM_ICON | CMIM_NAME;
	Menu_ModifyItem(hEnableDisableMenu, &mi);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTBButton, !State ? TTBST_PUSHED : 0);

}

void UpdatePopupMenu(BOOL State)
{
	// update option setting
	opt.UsePopup = State;
	db_set_b(NULL, WEATHERPROTONAME, "UsePopup", (BYTE)opt.UsePopup);

	CLISTMENUITEM mi = { sizeof(mi) };
	if (State)
	{	// to enable popup
		mi.pszName = LPGEN("Disable &weather notification");
		mi.icolibItem = GetIconHandle("popup");
	}
	else
	{	// to disable popup
		mi.pszName = LPGEN("Enable &weather notification");
		mi.icolibItem = GetIconHandle("nopopup");
	}

	mi.flags = CMIM_ICON | CMIM_NAME;
	Menu_ModifyItem(hEnableDisablePopupMenu, &mi);
}

// update the weather auto-update menu item when click on it
INT_PTR EnableDisableCmd(WPARAM wParam,LPARAM lParam)
{
	UpdateMenu(wParam == TRUE ? (BOOL)lParam : !opt.CAutoUpdate);
	return 0;
}

// update the weather popup menu item when click on it
INT_PTR MenuitemNotifyCmd(WPARAM wParam,LPARAM lParam)
{
	UpdatePopupMenu(!opt.UsePopup);
	return 0;
}

// adding weather contact menus
// copied and modified form "modified MSN Protocol"
void AddMenuItems(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszContactOwner = WEATHERPROTONAME;

	// contact menu
	CreateServiceFunction(MS_WEATHER_UPDATE, UpdateSingleStation);
	mi.position = -0x7FFFFFFA;
	mi.icolibItem = GetIconHandle("update");
	mi.pszName = LPGEN("Update Weather");
	mi.pszService = MS_WEATHER_UPDATE;
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_REFRESH, UpdateSingleRemove);
	mi.position = -0x7FFFFFF9;
	mi.icolibItem = GetIconHandle("update2");
	mi.pszName = LPGEN("Remove Old Data then Update");
	mi.pszService = MS_WEATHER_REFRESH;
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_BRIEF, BriefInfoSvc);
	mi.position = -0x7FFFFFF8;
	mi.icolibItem = GetIconHandle("brief");
	mi.pszName = LPGEN("Brief Information");
	mi.pszService = MS_WEATHER_BRIEF;
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_COMPLETE, LoadForecast);
	mi.position = -0x7FFFFFF7;
	mi.icolibItem = GetIconHandle("read");
	mi.pszName = LPGEN("Read Complete Forecast");
	mi.pszService = MS_WEATHER_COMPLETE;
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_MAP, WeatherMap);
	mi.position = -0x7FFFFFF6;
	mi.icolibItem = GetIconHandle("map");
	mi.pszName = LPGEN("Weather Map");
	mi.pszService = MS_WEATHER_MAP;
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_LOG, ViewLog);
	mi.position = -0x7FFFFFF5;
	mi.icolibItem = GetIconHandle("log");
	mi.pszName = LPGEN("View Log");
	mi.pszService = MS_WEATHER_LOG;
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_EDIT, EditSettings);
	mi.position = -0x7FFFFFF4;
	mi.icolibItem = GetIconHandle("edit");
	mi.pszName = LPGEN("Edit Settings");
	mi.pszService = MS_WEATHER_EDIT;
	Menu_AddContactMenuItem(&mi);

	// adding main menu items
	mi.pszPopupName = LPGEN("Weather");
	mi.popupPosition = 500099000;

	CreateServiceFunction(MS_WEATHER_ENABLED, EnableDisableCmd);
	mi.pszName = LPGEN("Enable/Disable Weather Update");
	mi.icolibItem = GetIconHandle("main");
	mi.position = 10100001;
	mi.pszService = MS_WEATHER_ENABLED;
	hEnableDisableMenu = Menu_AddMainMenuItem(&mi);
	UpdateMenu(opt.AutoUpdate);

	CreateServiceFunction(MS_WEATHER_UPDATEALL, UpdateAllInfo);
	mi.position = 20100001;
	mi.icolibItem = GetIconHandle("update");
	mi.pszName = LPGEN("Update All Weather");
	mi.pszService = MS_WEATHER_UPDATEALL;
	Menu_AddMainMenuItem(&mi);

	CreateServiceFunction(MS_WEATHER_REFRESHALL, UpdateAllRemove);
	mi.position = 20100002;
	mi.icolibItem = GetIconHandle("update2");
	mi.pszName = LPGEN("Remove Old Data then Update All");
	mi.pszService = MS_WEATHER_REFRESHALL;
	Menu_AddMainMenuItem(&mi);

	// only run if popup service exists
	if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
		CreateServiceFunction(WEATHERPROTONAME "/PopupMenu", MenuitemNotifyCmd);
		mi.pszName = LPGEN("Weather Notification");
		mi.icolibItem = GetIconHandle("popup");
		mi.position = 0;
		mi.pszPopupName = LPGEN("Popups");
		mi.pszService = WEATHERPROTONAME "/PopupMenu";
		hEnableDisablePopupMenu = Menu_AddMainMenuItem(&mi);
		UpdatePopupMenu(opt.UsePopup);
	}

	if ( ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		CreateServiceFunction("Weather/mwin_menu", Mwin_MenuClicked);
		mi.position = -0x7FFFFFF0;
		mi.hIcon = NULL;
		mi.flags = 0;
		mi.pszName = LPGEN("Display in a frame");
		mi.pszService = "Weather/mwin_menu";
		hMwinMenu = Menu_AddContactMenuItem(&mi);
	}
}
