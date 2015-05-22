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
#include "../commonstatus.h"
#include "startupstatus.h"
#include "../resource.h"
#include <commctrl.h>

#include <m_icolib.h>

#define MAX_MMITEMS		6

extern HINSTANCE hInst;
extern int protoCount;

static int menuprofiles[MAX_MMITEMS];
static HANDLE hProfileServices[MAX_MMITEMS];
static int mcount = 0;

static PROFILECE *pce = NULL;
static int pceCount = 0;

static UINT_PTR releaseTtbTimerId = 0;

static HANDLE hTBModuleLoadedHook;
static HANDLE hLoadAndSetProfileService;
static HANDLE hMessageHook = NULL;

static HWND hMessageWindow = NULL;
static HKINFO *hkInfo = NULL;
static int hkiCount = 0;

static HANDLE* ttbButtons = NULL;
static int ttbButtonCount = 0;

HANDLE hTTBModuleLoadedHook;

// these are some lame functions, if you have a better solution, mail me. :s
static INT_PTR profileService0(WPARAM, LPARAM)
{
	LoadAndSetProfile((WPARAM)menuprofiles[0], 0);
	return 0;
}

static INT_PTR profileService1(WPARAM, LPARAM)
{
	LoadAndSetProfile((WPARAM)menuprofiles[1], 0);
	return 0;
}

static INT_PTR profileService2(WPARAM, LPARAM)
{
	LoadAndSetProfile((WPARAM)menuprofiles[2], 0);
	return 0;
}

static INT_PTR profileService3(WPARAM, LPARAM)
{
	LoadAndSetProfile((WPARAM)menuprofiles[3], 0);
	return 0;
}

static INT_PTR profileService4(WPARAM, LPARAM)
{
	LoadAndSetProfile((WPARAM)menuprofiles[4], 0);
	return 0;
}

static INT_PTR profileService5(WPARAM, LPARAM)
{
	LoadAndSetProfile((WPARAM)menuprofiles[5], 0);
	return 0;
}

static int CreateMainMenuItems(WPARAM, LPARAM)
{
	char servicename[128];
	int i, count;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.popupPosition = 2000100000;
	mi.flags = CMIF_TCHAR;
	mcount = 0;
	count = GetProfileCount(0, 0);
	for (i=0; i < count; i++) {
		TCHAR profilename[128];
		if ( !db_get_b(NULL, MODULENAME, OptName(i, SETTING_CREATEMMITEM), 0) || GetProfileName(i, (LPARAM)profilename))
			continue;

		if ( db_get_b(NULL, MODULENAME, OptName(i, SETTING_INSUBMENU), 1))
			mi.ptszPopupName = LPGENT("Status profiles");

		mi.ptszName = profilename;
		mi.position = 2000100000 + mcount;
		mir_snprintf(servicename, SIZEOF(servicename), "%s%d", MS_SS_MENUSETPROFILEPREFIX, mcount);
		switch(mcount) {
		case 0:
			hProfileServices[mcount] = CreateServiceFunction(servicename, profileService0);
			break;
		case 1:
			hProfileServices[mcount] = CreateServiceFunction(servicename, profileService1);
			break;
		case 2:
			hProfileServices[mcount] = CreateServiceFunction(servicename, profileService2);
			break;
		case 3:
			hProfileServices[mcount] = CreateServiceFunction(servicename, profileService3);
			break;
		case 4:
			hProfileServices[mcount] = CreateServiceFunction(servicename, profileService4);
			break;
		case 5:
			hProfileServices[mcount] = CreateServiceFunction(servicename, profileService5);
			break;
		default:
			MessageBox(NULL, TranslateT("There is a maximum of 6 menu items"), TranslateT("StartupStatus"), MB_OK);
			return 0;
		}
		mi.pszService = servicename;
		if ( Menu_AddStatusMenuItem(&mi)) {
			menuprofiles[mcount] = i;
			mcount += 1;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetProfileName(WPARAM wParam, LPARAM lParam)
{
	int profile = (int)wParam;
	if (profile < 0) // get default profile
		profile = db_get_w(NULL, MODULENAME, SETTING_DEFAULTPROFILE, 0);

	int count = db_get_w(NULL, MODULENAME, SETTING_PROFILECOUNT, 0);
	if (profile >= count && count > 0)
		return -1;

	TCHAR* buf = (TCHAR*)lParam;
	if (count == 0) {
		_tcsncpy(buf, TranslateT("default"), 128-1);
		return 0;
	}

	DBVARIANT dbv;
	char setting[80];
	mir_snprintf(setting, SIZEOF(setting), "%d_%s", profile, SETTING_PROFILENAME);
	if ( db_get_ts(NULL, MODULENAME, setting, &dbv))
		return -1;

	_tcsncpy(buf, dbv.ptszVal, 128-1); buf[127] = 0;
	db_free(&dbv);
	return 0;
}

INT_PTR GetProfileCount(WPARAM wParam, LPARAM)
{
	int *def = (int*)wParam;
	int count = db_get_w(NULL, MODULENAME, SETTING_PROFILECOUNT, 1);
	if (def != 0) {
		*def = db_get_w(NULL, MODULENAME, SETTING_DEFAULTPROFILE, 0);
		if (*def >= count)
			*def = 0;
	}

	return count;
}

TCHAR *GetStatusMessage(int profile, char *szProto)
{
	char dbSetting[80];
	DBVARIANT dbv;

	for ( int i=0; i < pceCount; i++ ) {
		if ( (pce[i].profile == profile) && (!strcmp(pce[i].szProto, szProto))) {
			mir_snprintf(dbSetting, SIZEOF(dbSetting), "%d_%s_%s", profile, szProto, SETTING_PROFILE_STSMSG);
			if (!db_get_ts(NULL, MODULENAME, dbSetting, &dbv)) { // reload from db
				pce[i].msg = ( TCHAR* )realloc(pce[i].msg, sizeof(TCHAR)*(mir_tstrlen(dbv.ptszVal)+1));
				if (pce[i].msg != NULL) {
					_tcscpy(pce[i].msg, dbv.ptszVal);
				}
				db_free(&dbv);
			}
			else {
				if (pce[i].msg != NULL) {
					free(pce[i].msg);
					pce[i].msg = NULL;
				}
			}
			return pce[i].msg;
		}
	}
	pce = (PROFILECE*)realloc(pce, (pceCount+1)*sizeof(PROFILECE));
	if (pce == NULL)
		return NULL;

	pce[pceCount].profile = profile;
	pce[pceCount].szProto = _strdup(szProto);
	pce[pceCount].msg = NULL;
	mir_snprintf(dbSetting, SIZEOF(dbSetting), "%d_%s_%s", profile, szProto, SETTING_PROFILE_STSMSG);
	if (!db_get_ts(NULL, MODULENAME, dbSetting, &dbv)) {
		pce[pceCount].msg = _tcsdup(dbv.ptszVal);
		db_free(&dbv);
	}
	pceCount++;

	return pce[pceCount-1].msg;
}

int GetProfile( int profile, TSettingsList& arSettings )
{
	if ( profile < 0 ) // get default profile
		profile = db_get_w(NULL, MODULENAME, SETTING_DEFAULTPROFILE, 0);

	int count = db_get_w(NULL, MODULENAME, SETTING_PROFILECOUNT, 0);
	if ( profile >= count && count > 0 )
		return -1;

	arSettings.destroy();

	// if count == 0, continue so the default profile will be returned
	PROTOACCOUNT** protos;
	ProtoEnumAccounts( &count, &protos );

	for ( int i=0; i < count; i++ )
		if ( IsSuitableProto( protos[i] ))
			arSettings.insert( new TSSSetting( profile, protos[i] ));

	return ( arSettings.getCount() == 0 ) ? -1 : 0;
}

static void CALLBACK releaseTtbTimerFunction(HWND hwnd,UINT message, UINT_PTR idEvent,DWORD dwTime)
{
	KillTimer(NULL, releaseTtbTimerId);
	for(int i=0; i < ttbButtonCount; i++)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)ttbButtons[i], 0);
}

INT_PTR LoadAndSetProfile(WPARAM wParam, LPARAM lParam)
{
	// wParam == profile no.
	int profileCount = GetProfileCount(0, 0);
	int profile = (int)wParam;

	TSettingsList profileSettings( 10, CompareSettings );
	if ( !GetProfile( profile, profileSettings)) {
		profile = (profile >= 0)?profile:db_get_w(NULL, MODULENAME, SETTING_DEFAULTPROFILE, 0);

		char setting[64];
		mir_snprintf(setting, SIZEOF(setting), "%d_%s", profile, SETTING_SHOWCONFIRMDIALOG);
		if (!db_get_b(NULL, MODULENAME, setting, 0))
			CallService(MS_CS_SETSTATUSEX,(WPARAM)&profileSettings, 0);
		else
			CallService(MS_CS_SHOWCONFIRMDLGEX, (WPARAM)&profileSettings, (LPARAM)db_get_dw(NULL, MODULENAME, SETTING_DLGTIMEOUT, 5));
	}

	// add timer here
	if (hTTBModuleLoadedHook)
		releaseTtbTimerId = SetTimer(NULL, 0, 100, releaseTtbTimerFunction);

	return 0;
}

static UINT GetFsModifiers(WORD wHotKey)
{
	UINT fsm = 0;
	if (HIBYTE(wHotKey)&HOTKEYF_ALT)
        fsm |= MOD_ALT;
	if (HIBYTE(wHotKey)&HOTKEYF_CONTROL)
        fsm |= MOD_CONTROL;
    if (HIBYTE(wHotKey)&HOTKEYF_SHIFT)
        fsm |= MOD_SHIFT;
    if (HIBYTE(wHotKey)&HOTKEYF_EXT)
        fsm |= MOD_WIN;

	return fsm;
}

static DWORD CALLBACK MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ( msg == WM_HOTKEY ) {
		for (int i=0; i < hkiCount; i++ )
			if ((int)hkInfo[i].id == wParam)
				LoadAndSetProfile((WPARAM)hkInfo[i].profile, 0);
	}

	return TRUE;
}

static int UnregisterHotKeys()
{
	if (hkInfo != NULL) {
		for (int i = 0; i < hkiCount; i ++) {
			UnregisterHotKey(hMessageWindow, (int)hkInfo[i].id);
			GlobalDeleteAtom(hkInfo[i].id);
		}
		free(hkInfo);
	}
	DestroyWindow(hMessageWindow);

	hkiCount = 0;
	hkInfo = NULL;
	hMessageWindow = NULL;

	return 0;
}

// assumes UnregisterHotKeys was called before
static int RegisterHotKeys()
{
	hMessageWindow = CreateWindowEx(0, _T("STATIC"), NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);

	int count = GetProfileCount(0, 0);
	for ( int i=0; i < count; i++ ) {
		if (!db_get_b(NULL, MODULENAME, OptName(i, SETTING_REGHOTKEY), 0))
			continue;

		WORD wHotKey = db_get_w(NULL, MODULENAME, OptName(i, SETTING_HOTKEY), 0);
		hkInfo = ( HKINFO* )realloc(hkInfo, (hkiCount+1)*sizeof(HKINFO));
		if (hkInfo == NULL)
			return -1;

		char atomname[255];
		mir_snprintf(atomname, SIZEOF(atomname), "StatusProfile_%d", i);
		hkInfo[hkiCount].id = GlobalAddAtomA(atomname);
		if (hkInfo[hkiCount].id == 0)
			continue;

		hkInfo[hkiCount].profile = i;
		hkiCount ++;
		RegisterHotKey(hMessageWindow, (int)hkInfo[hkiCount-1].id, GetFsModifiers(wHotKey), LOBYTE(wHotKey));
	}

	if (hkiCount == 0)
		UnregisterHotKeys();

	return 0;
}

int LoadMainOptions()
{
	if (hTTBModuleLoadedHook) {
		RemoveTopToolbarButtons();
		CreateTopToolbarButtons(0,0);
	}

	UnregisterHotKeys();
	RegisterHotKeys();
	return 0;
}

int LoadProfileModule()
{
	hLoadAndSetProfileService = CreateServiceFunction(MS_SS_LOADANDSETPROFILE, LoadAndSetProfile);
	return 0;
}

int InitProfileModule()
{
	hTTBModuleLoadedHook = HookEvent(ME_TTB_MODULELOADED, CreateTopToolbarButtons);

	HookEvent( ME_CLIST_PREBUILDSTATUSMENU,  CreateMainMenuItems);

	CreateMainMenuItems(0,0);
	RegisterHotKeys();
	return 0;
}

int DeinitProfilesModule()
{
	for ( int i=0; i < mcount; i++ )
		DestroyServiceFunction(hProfileServices[i]);

	if ( pce ) {
		for ( int i=0; i < pceCount; i++ )
			free( pce[i].szProto );
		free( pce );
	}

	UnregisterHotKeys();
	RemoveTopToolbarButtons();

	DestroyServiceFunction(hLoadAndSetProfileService);
	return 0;
}
