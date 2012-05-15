/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

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

static int hEnableDisablePopupMenu;
static int hEnableDisableMenu;

static HANDLE hService[27];

//============  MIRANDA PROTOCOL SERVICES  ============

// protocol service function for setting weather protocol status
INT_PTR WeatherSetStatus(WPARAM new_status, LPARAM lParam) 
{
	new_status = new_status != ID_STATUS_OFFLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;

	// if we don't want to show status for default station
	if (opt.NoProtoCondition && status != new_status) 
	{
		old_status = status;
		status = new_status != ID_STATUS_OFFLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);

		UpdateMenu(new_status != ID_STATUS_OFFLINE);
		if (new_status != ID_STATUS_OFFLINE) UpdateAll(FALSE, FALSE);
	}

	return 0;
}

// get capabilities protocol service function
INT_PTR WeatherGetCaps(WPARAM wParam, LPARAM lParam) 
{
	INT_PTR ret = 0;

	switch(wParam) 
	{
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
		//			if (!opt.NoProtoCondition) ret |= PF2_ONLINE;
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
	ProtoBroadcastAck(WEATHERPROTONAME, param, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

// nothing to do here because weather proto do not need to retrieve contact info form network
// so just return a 0
INT_PTR WeatherGetInfo(WPARAM wParam,LPARAM lParam) 
{
	CCSDATA *ccs = (CCSDATA *) lParam;
	mir_forkthread(AckThreadProc, ccs->hContact);
	return 0;
}


INT_PTR WeatherGetAvatarInfo(WPARAM wParam, LPARAM lParam) 
{
	static const char *statusStr[] = {
		"Light",
		"Fog",
		"SShower",
		"Snow",
		"RShower", 
		"Rain",
		"PCloudy",
		"Cloudy",
		"Sunny",
		"NA"
	};

	static const WORD statusValue[] = {
		LIGHT,
		FOG,
		SSHOWER,
		SNOW,
		RSHOWER,
		RAIN,
		PCLOUDY,
		CLOUDY,
		SUNNY,
		NA
	};
	char szSearchPath[MAX_PATH], *chop;
	WORD status;
	unsigned  i;
	PROTO_AVATAR_INFORMATION* ai = ( PROTO_AVATAR_INFORMATION* )lParam;

	GetModuleFileName(GetModuleHandle(NULL), szSearchPath, sizeof(szSearchPath));
	chop = strrchr(szSearchPath, '\\');

	if (chop) *chop = '\0';
	else szSearchPath[0] = 0;

	status = (WORD)DBGetContactSettingWord(ai->hContact, WEATHERPROTONAME, "StatusIcon",0);
	for (i=0; i<10; i++)
	{
		if (statusValue[i] == status) 
			break;
	}
	if (i >= 10) return GAIR_NOAVATAR;

	ai->format = PA_FORMAT_PNG;
	wsprintf(ai->filename, "%s\\Plugins\\Weather\\%s.png", szSearchPath, statusStr[i]);
	if (_access(ai->filename, 4) == 0) return GAIR_SUCCESS;

	ai->format = PA_FORMAT_GIF;
	wsprintf(ai->filename, "%s\\Plugins\\Weather\\%s.gif", szSearchPath, statusStr[i]);
	if (_access(ai->filename, 4) == 0) return GAIR_SUCCESS;

	ai->format = PA_FORMAT_UNKNOWN;
	ai->filename[0] = 0;
	return 	GAIR_NOAVATAR;
}


void AvatarDownloaded(HANDLE hContact)
{
	int haveAvatar;
	PROTO_AVATAR_INFORMATION AI = {0};
	AI.cbSize = sizeof(AI);
	AI.hContact = hContact;

//	ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);

	haveAvatar = WeatherGetAvatarInfo(GAIF_FORCE, (LPARAM)&AI) == GAIR_SUCCESS;

	if (haveAvatar)
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &AI, 0);
	else
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
}


static void __cdecl WeatherGetAwayMsgThread(HANDLE hContact)
{
	DBVARIANT dbv;
	Sleep(100);
	if (!DBGetContactSettingString(hContact, "CList", "StatusMsg", &dbv)) 
	{
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, 
			(HANDLE)1, (LPARAM)dbv.pszVal);
		DBFreeVariant( &dbv );
	}
	else 
		ProtoBroadcastAck(WEATHERPROTONAME, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

static INT_PTR WeatherGetAwayMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA* ccs = (CCSDATA*)lParam;
	if (ccs == NULL)
		return 0;

	mir_forkthread(WeatherGetAwayMsgThread, ccs->hContact);
	return 1;
}

//============  PROTOCOL INITIALIZATION  ============
// protocol services
void InitServices(void) 
{
	hService[0]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETCAPS, WeatherGetCaps);
	hService[1]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETNAME, WeatherGetName);
	hService[3]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_LOADICON, WeatherLoadIcon);
	hService[4]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_SETSTATUS, WeatherSetStatus);
	hService[5]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETSTATUS, WeatherGetStatus);
	hService[6]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_BASICSEARCH, WeatherBasicSearch);
	hService[7]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_SEARCHBYEMAIL, WeatherBasicSearch);
	hService[8]  = CreateProtoServiceFunction(WEATHERPROTONAME, PS_ADDTOLIST, WeatherAddToList);
	hService[9]  = CreateProtoServiceFunction(WEATHERPROTONAME, PSS_GETINFO, WeatherGetInfo);
	hService[10] = CreateProtoServiceFunction(WEATHERPROTONAME, PS_GETAVATARINFO, WeatherGetAvatarInfo);
	hService[11] = CreateProtoServiceFunction(WEATHERPROTONAME, PSS_GETAWAYMSG, WeatherGetAwayMsg);
	hService[12] = CreateProtoServiceFunction(WEATHERPROTONAME, PS_CREATEADVSEARCHUI, WeatherCreateAdvancedSearchUI);
	hService[13] = CreateProtoServiceFunction(WEATHERPROTONAME, PS_SEARCHBYADVANCED, WeatherAdvancedSearch);

	hService[14] = CreateProtoServiceFunction(WEATHERPROTONAME, MS_WEATHER_GETDISPLAY, GetDisplaySvcFunc);
}

void DestroyServices(void)
{
	unsigned i;

	for (i = sizeof(hService)/sizeof(HANDLE); i--; )
	{
		if (hService[i] != NULL)
			DestroyServiceFunction(hService[i]);
	}
}

//============  MENU INITIALIZATION  ============

void UpdateMenu(BOOL State) 
{
	// We're here to change something, so let's read the actual value.
	// TempDisable == FALSE means that right now the popups are enabled
	// and if we're here, we want to disable them.
	// The icon works this way:
	// if the notifications are disabled, the icon hasn't the red circle;
	// if the notifications are enabled, the icon has the red circle.

	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);

	if (State) 
	{	// to enable auto-update
		mi.pszName = "Auto Update Enabled";
		mi.icolibItem = GetIconHandle("main");
	}
	else 
	{	// to disable auto-update
		mi.pszName = "Auto Update Disabled";
		mi.icolibItem = GetIconHandle("disabled");
	}
	// update option setting
	opt.CAutoUpdate = State;
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "AutoUpdate", (BYTE)opt.AutoUpdate);
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_ICONFROMICOLIB;

	// update menu item
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hEnableDisableMenu,(LPARAM)&mi);
}

void UpdatePopupMenu(BOOL State) 
{
	// We're here to change something, so let's read the actual value.
	// TempDisable == FALSE means that right now the popups are enabled
	// and if we're here, we want to disable them.
	// The icon works this way:
	// if the notifications are disabled, the icon hasn't the red circle;
	// if the notifications are enabled, the icon has the red circle.

	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);

	if (State) 
	{	// to enable popup
		mi.pszName = "Disable &weather notification";
		mi.icolibItem = GetIconHandle("popup");
	}
	else 
	{	// to disable popup
		mi.pszName = "Enable &weather notification";
		mi.icolibItem = GetIconHandle("nopopup");
	}
	// update option setting
	opt.UsePopup = State;
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "UsePopUp", (BYTE)opt.UsePopup);
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_ICONFROMICOLIB;

	// update menu item
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hEnableDisablePopupMenu,(LPARAM)&mi);
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
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.pszContactOwner = WEATHERPROTONAME;
	mi.flags = CMIF_ICONFROMICOLIB;

	// contact menu
	hService[15] = CreateServiceFunction(MS_WEATHER_UPDATE, UpdateSingleStation);
	mi.position=-0x7FFFFFFA;
	mi.icolibItem = GetIconHandle("update");
	mi.pszName="Update Weather";
	mi.pszService = MS_WEATHER_UPDATE;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hService[16] = CreateServiceFunction(MS_WEATHER_REFRESH, UpdateSingleRemove);
	mi.position=-0x7FFFFFF9;
	mi.icolibItem = GetIconHandle("update2");
	mi.pszName="Remove Old Data then Update";
	mi.pszService = MS_WEATHER_REFRESH;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hService[17] = CreateServiceFunction(MS_WEATHER_BRIEF, BriefInfoSvc);
	mi.position=-0x7FFFFFF8;
	mi.icolibItem = GetIconHandle("brief");
	mi.pszName="Brief Information";
	mi.pszService = MS_WEATHER_BRIEF;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hService[18] = CreateServiceFunction(MS_WEATHER_COMPLETE, LoadForecast);
	mi.position=-0x7FFFFFF7;
	mi.icolibItem = GetIconHandle("read");
	mi.pszName="Read Complete Forecast";
	mi.pszService = MS_WEATHER_COMPLETE;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hService[19] = CreateServiceFunction(MS_WEATHER_MAP, WeatherMap);
	mi.position=-0x7FFFFFF6;
	mi.icolibItem = GetIconHandle("map");
	mi.pszName="Weather Map";
	mi.pszService = MS_WEATHER_MAP;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hService[20] = CreateServiceFunction(MS_WEATHER_LOG, ViewLog);
	mi.position=-0x7FFFFFF5;
	mi.icolibItem = GetIconHandle("log");
	mi.pszName="View Log";
	mi.pszService = MS_WEATHER_LOG;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hService[21] = CreateServiceFunction(MS_WEATHER_EDIT, EditSettings);
	mi.position=-0x7FFFFFF4;
	mi.icolibItem = GetIconHandle("edit");
	mi.pszName="Edit Settings";
	mi.pszService = MS_WEATHER_EDIT;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	// adding main menu items
	mi.pszPopupName = "Weather";
	mi.popupPosition = 500099000;

	hService[22] = CreateServiceFunction(MS_WEATHER_ENABLED, EnableDisableCmd);
	mi.pszName = "Enable/Disable Weather Update";
	mi.icolibItem = GetIconHandle("main");
	mi.position=10100001;
	mi.pszService = MS_WEATHER_ENABLED;
	hEnableDisableMenu = CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	UpdateMenu(opt.AutoUpdate);

	hService[23] = CreateServiceFunction(MS_WEATHER_UPDATEALL, UpdateAllInfo);
	mi.position=20100001;
	mi.icolibItem = GetIconHandle("update");
	mi.pszName="Update All Weather";
	mi.pszService = MS_WEATHER_UPDATEALL;
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	hService[24] = CreateServiceFunction(MS_WEATHER_REFRESHALL, UpdateAllRemove);
	mi.position=20100002;
	mi.icolibItem = GetIconHandle("update2");
	mi.pszName="Remove Old Data then Update All";
	mi.pszService = MS_WEATHER_REFRESHALL;
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	// only run if popup service exists
	if (ServiceExists(MS_POPUP_ADDPOPUP)) 
	{
		hService[25] = CreateServiceFunction(WEATHERPROTONAME "/PopupMenu", MenuitemNotifyCmd);
		mi.pszName = "Weather Notification";
		mi.icolibItem = GetIconHandle("popup");
		mi.position = 0;
		mi.pszPopupName = "PopUps";
		mi.pszService = WEATHERPROTONAME "/PopupMenu";
		hEnableDisablePopupMenu = CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
		UpdatePopupMenu(opt.UsePopup);
	}

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		hService[26] = CreateServiceFunction("Weather/mwin_menu", Mwin_MenuClicked);
		mi.position = -0x7FFFFFF0;
		mi.hIcon = NULL;
		mi.flags = 0;
		mi.pszName = "Display in a frame";
		mi.pszService = "Weather/mwin_menu";
		hMwinMenu = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
	}
}
