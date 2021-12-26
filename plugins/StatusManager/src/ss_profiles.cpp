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

#include "stdafx.h"

#define MAX_MMITEMS		6

struct PROFILECE
{
	int profile;
	ptrA szProto;
	ptrW msg;
};

static OBJLIST<PROFILECE> arProfiles(1);

extern int protoCount;

static int menuprofiles[MAX_MMITEMS];
static int mcount = 0;

static UINT_PTR releaseTtbTimerId = 0;

static HANDLE hTBModuleLoadedHook;
static HANDLE hMessageHook = nullptr;

static HWND hMessageWindow = nullptr;
static HKINFO *hkInfo = nullptr;
static int hkiCount = 0;

static HANDLE* ttbButtons = nullptr;
static int ttbButtonCount = 0;

HANDLE hTTBModuleLoadedHook;

static INT_PTR profileService(WPARAM, LPARAM, LPARAM param)
{
	LoadAndSetProfile((WPARAM)menuprofiles[param], 0);
	return 0;
}

static int CreateMainMenuItems(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	mi.position = 2000100000;
	mi.flags = CMIF_UNICODE;
	mcount = 0;
	int count = GetProfileCount(0, 0);
	for (int i = 0; i < count && mcount < MAX_MMITEMS; i++) {
		wchar_t profilename[128];
		if (!SSPlugin.getByte(OptName(i, SETTING_CREATEMMITEM), 0) || GetProfileName(i, (LPARAM)profilename))
			continue;

		if (SSPlugin.getByte(OptName(i, SETTING_INSUBMENU), 1) && !mi.root) {
			mi.root = g_plugin.addRootMenu(MO_STATUS, LPGENW("Status profiles"), 2000100000);
			Menu_ConfigureItem(mi.root, MCI_OPT_UID, "1AB30D51-BABA-4B27-9288-1A12278BAD8D");
		}

		char servicename[128];
		mir_snprintf(servicename, "%s%d", MS_SS_MENUSETPROFILEPREFIX, mcount);
		CreateServiceFunctionParam(servicename, profileService, mcount);

		mi.name.w = profilename;
		mi.position = 2000100000 + mcount;
		mi.pszService = servicename;
		if (Menu_AddStatusMenuItem(&mi))
			menuprofiles[mcount++] = i;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetProfileName(WPARAM wParam, LPARAM lParam)
{
	int profile = (int)wParam;
	if (profile < 0) // get default profile
		profile = SSPlugin.getWord(SETTING_DEFAULTPROFILE, 0);

	int count = SSPlugin.getWord(SETTING_PROFILECOUNT, 0);
	if (profile >= count && count > 0)
		return -1;

	wchar_t* buf = (wchar_t*)lParam;
	if (count == 0) {
		wcsncpy(buf, TranslateT("default"), 128 - 1);
		return 0;
	}

	DBVARIANT dbv;
	char setting[80];
	mir_snprintf(setting, "%d_%s", profile, SETTING_PROFILENAME);
	if (SSPlugin.getWString(setting, &dbv))
		return -1;

	wcsncpy(buf, dbv.pwszVal, 128 - 1); buf[127] = 0;
	db_free(&dbv);
	return 0;
}

INT_PTR GetProfileCount(WPARAM wParam, LPARAM)
{
	int *def = (int*)wParam;
	int count = SSPlugin.getWord(SETTING_PROFILECOUNT, 0);
	if (def != nullptr) {
		*def = SSPlugin.getWord(SETTING_DEFAULTPROFILE, 0);
		if (*def >= count)
			*def = 0;
	}

	return count;
}

wchar_t* GetStatusMessage(int profile, const char *szProto)
{
	char dbSetting[80];
	mir_snprintf(dbSetting, "%d_%s_%s", profile, szProto, SETTING_PROFILE_STSMSG);

	for (auto &p : arProfiles) {
		if (p->profile == profile && !mir_strcmp(p->szProto, szProto)) {
			p->msg = SSPlugin.getWStringA(dbSetting);
			return p->msg;
		}
	}

	PROFILECE *pce = new PROFILECE;
	pce->profile = profile;
	pce->szProto = mir_strdup(szProto);
	pce->msg = SSPlugin.getWStringA(dbSetting);
	arProfiles.insert(pce);

	return pce->msg;
}

void FillStatus(SMProto &ps, int profile)
{
	// load status
	char setting[80];
	mir_snprintf(setting, "%d_%s", profile, ps.m_szName);
	int iStatus = SSPlugin.getWord(setting, 0);
	if (iStatus < MIN_STATUS || iStatus > MAX_STATUS)
		iStatus = DEFAULT_STATUS;
	ps.m_status = iStatus;

	// load last status
	mir_snprintf(setting, "%s%s", PREFIX_LAST, ps.m_szName);
	iStatus = SSPlugin.getWord(setting, 0);
	if (iStatus < MIN_STATUS || iStatus > MAX_STATUS)
		iStatus = DEFAULT_STATUS;
	ps.m_lastStatus = iStatus;

	ps.m_szMsg = GetStatusMessage(profile, ps.m_szName);
	if (ps.m_szMsg)
		ps.m_szMsg = mir_wstrdup(ps.m_szMsg);
}

int GetProfile(int profile, TProtoSettings &arSettings)
{
	if (profile < 0) // get default profile
		profile = SSPlugin.getWord(SETTING_DEFAULTPROFILE, 0);

	int count = SSPlugin.getWord(SETTING_PROFILECOUNT, 0);
	if (profile >= count && count > 0)
		return -1;

	for (auto &it : arSettings)
		FillStatus(*it, profile);

	return (arSettings.getCount() == 0) ? -1 : 0;
}

static void CALLBACK releaseTtbTimerFunction(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(nullptr, releaseTtbTimerId);
	for (int i = 0; i < ttbButtonCount; i++)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)ttbButtons[i], 0);
}

INT_PTR LoadAndSetProfile(WPARAM iProfileNo, LPARAM)
{
	// wParam == profile no.
	int profile = (int)iProfileNo;
	TProtoSettings ps(protoList);
	if (!GetProfile(profile, ps)) {
		profile = (profile >= 0) ? profile : SSPlugin.getWord(SETTING_DEFAULTPROFILE, 0);

		char setting[64];
		mir_snprintf(setting, "%d_%s", profile, SETTING_SHOWCONFIRMDIALOG);
		if (!SSPlugin.getByte(setting, 0))
			SetStatusEx(ps);
		else
			ShowConfirmDialogEx(&ps, SSPlugin.getDword(SETTING_DLGTIMEOUT, 5));
	}

	// add timer here
	if (hTTBModuleLoadedHook)
		releaseTtbTimerId = SetTimer(nullptr, 0, 100, releaseTtbTimerFunction);

	return 0;
}

static UINT GetFsModifiers(uint16_t wHotKey)
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

static uint32_t CALLBACK MessageWndProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	if (msg == WM_HOTKEY) {
		for (int i = 0; i < hkiCount; i++)
			if ((int)hkInfo[i].id == wParam)
				LoadAndSetProfile((WPARAM)hkInfo[i].profile, 0);
	}

	return TRUE;
}

void UnregisterHotKeys()
{
	if (hkInfo != nullptr) {
		for (int i = 0; i < hkiCount; i++) {
			UnregisterHotKey(hMessageWindow, (int)hkInfo[i].id);
			GlobalDeleteAtom(hkInfo[i].id);
		}
		mir_free(hkInfo);
	}
	DestroyWindow(hMessageWindow);

	hkiCount = 0;
	hkInfo = nullptr;
	hMessageWindow = nullptr;
}

// assumes UnregisterHotKeys was called before
int RegisterHotKeys()
{
	hMessageWindow = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);

	int count = GetProfileCount(0, 0);
	for (int i = 0; i < count; i++) {
		if (!SSPlugin.getByte(OptName(i, SETTING_REGHOTKEY), 0))
			continue;

		uint16_t wHotKey = SSPlugin.getWord(OptName(i, SETTING_HOTKEY), 0);
		hkInfo = (HKINFO*)mir_realloc(hkInfo, (hkiCount + 1)*sizeof(HKINFO));
		if (hkInfo == nullptr)
			return -1;

		char atomname[255];
		mir_snprintf(atomname, "StatusProfile_%d", i);
		hkInfo[hkiCount].id = GlobalAddAtomA(atomname);
		if (hkInfo[hkiCount].id == 0)
			continue;

		hkInfo[hkiCount].profile = i;
		hkiCount++;
		RegisterHotKey(hMessageWindow, (int)hkInfo[hkiCount - 1].id, GetFsModifiers(wHotKey), LOBYTE(wHotKey));
	}

	if (hkiCount == 0)
		UnregisterHotKeys();

	return 0;
}

int LoadProfileModule()
{
	CreateServiceFunction(MS_SS_LOADANDSETPROFILE, LoadAndSetProfile);
	return 0;
}

int InitProfileModule()
{
	hTTBModuleLoadedHook = HookEvent(ME_TTB_MODULELOADED, CreateTopToolbarButtons);

	HookEvent(ME_CLIST_PREBUILDSTATUSMENU, CreateMainMenuItems);

	CreateMainMenuItems(0, 0);
	RegisterHotKeys();
	return 0;
}

int DeinitProfilesModule()
{
	arProfiles.destroy();

	UnregisterHotKeys();
	RemoveTopToolbarButtons();
	return 0;
}
