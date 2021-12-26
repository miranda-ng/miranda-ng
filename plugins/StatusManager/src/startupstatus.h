/*
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __STARTUPSTATUSHEADER
#define __STARTUPSTATUSHEADER

#define SSMODULENAME "StartupStatus"

struct PROFILEOPTIONS : public MZeroedObject
{
	__inline PROFILEOPTIONS() :
		ps(protoList)
	{}

	__inline ~PROFILEOPTIONS()
	{
		mir_free(tszName);
	}

	wchar_t *tszName;
	TProtoSettings ps;
	BOOL showDialog;
	BOOL createTtb;
	BOOL createMmi;
	BOOL inSubMenu;
	BOOL regHotkey;
	uint16_t hotKey;
};

typedef struct {
	ATOM id;
	int profile;
} HKINFO;

#define UM_ADDPROFILE       WM_USER + 5

#define CLUIINTM_REDRAW (WM_USER+100)

#define MODULE_CLIST              "CList"
#define MODULE_CLUI               "CLUI"
#define SETTING_STATUS            "Status"

#define SETTING_SETWINSTATE       "SetState"
#define SETTING_WINSTATE          "State"

#define SETTING_SETDOCKED         "SetDocked"
#define SETTING_DOCKED            "Docked"

#define SETTING_SHOWDIALOG        "ShowDialog"
#define SETTING_OFFLINECLOSE      "OfflineOnClose"
#define SETTING_SETPROFILE        "SetStatusOnStartup"
#define SETTING_AUTODIAL          "AutoDial"
#define SETTING_AUTOHANGUP        "AutoHangup"

#define SETTING_TOOLWINDOW        "ToolWindow"

#define SETTING_OVERRIDE          "AllowOverride"

#define SETTING_SETWINLOCATION    "SetWinLoc"
#define SETTING_XPOS              "x"
#define SETTING_YPOS              "y"

#define SETTING_SETWINSIZE        "SetWinSize"
#define SETTING_WIDTH             "Width"
#define SETTING_HEIGHT            "Height"
#define SETTING_AUTOSIZE          "AutoSize"

#define SETTING_PROFILECOUNT      "ProfileCount"
#define SETTING_DEFAULTPROFILE    "DefaultProfile"
#define SETTING_PROFILENAME       "ProfileName"
#define SETTING_CREATETTBBUTTON   "CreateTTBButton"
#define SETTING_PROFILE_STSMSG    "StatusMsg"
#define SETTING_SHOWCONFIRMDIALOG "profile_ShowDialog"
#define SETTING_CREATEMMITEM      "CreateMMItem"
#define SETTING_INSUBMENU         "InSubMenu"
#define SETTING_REGHOTKEY         "RegHotKey"
#define SETTING_HOTKEY            "HotKey"
#define SETTING_PROFILENO         "ProfileNo"

#define SETTING_SETPROFILEDELAY "SetStatusDelay"
#define SETTING_DLGTIMEOUT      "DialogTimeout"

#define SHORTCUT_DESC      L"Miranda NG"
#define SHORTCUT_FILENAME  L"\\Miranda NG.lnk"

#define DOCKED_NONE  0
#define DOCKED_LEFT  1
#define DOCKED_RIGHT 2

#define MS_SS_MENUSETPROFILEPREFIX		"StartupStatus/SetProfile_"

// options
int StartupStatusOptionsInit(WPARAM wparam,LPARAM lparam);
char* OptName(int i, const char *setting);

// startupstatus
void StartupStatusLoad();
void StartupStatusUnload();

void SS_LoadDynamic(SMProto *);

// profile
void FillStatus(SMProto &ps, int profile);
int GetProfile(int profileID, TProtoSettings &arSettings);
wchar_t* GetStatusMessage(int profile, const char *szProto);

INT_PTR LoadAndSetProfile(WPARAM wParam, LPARAM lParam);
INT_PTR GetProfileCount(WPARAM wParam, LPARAM lParam);
INT_PTR GetProfileName(WPARAM wParam, LPARAM lParam);

extern HANDLE hTTBModuleLoadedHook;
void RemoveTopToolbarButtons();
int  CreateTopToolbarButtons(WPARAM wParam, LPARAM lParam);

int RegisterHotKeys();
void UnregisterHotKeys();

int LoadProfileModule();
int InitProfileModule();
int DeinitProfilesModule();

#endif //__STARTUPSTATUSHEADER
