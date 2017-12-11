/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-17 Miranda NG project (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#define IDLEMOD "Idle"

static CMOption<BYTE> g_bIdleCheck(IDLEMOD, "UserIdleCheck", 0);
static CMOption<BYTE> g_bIdleMethod(IDLEMOD, "IdleMethod", 0);
static CMOption<BYTE> g_bIdleOnSaver(IDLEMOD, "IdleOnSaver", 0);
static CMOption<BYTE> g_bIdleOnFullScr(IDLEMOD, "IdleOnFullScr", 0);
static CMOption<BYTE> g_bIdleOnLock(IDLEMOD, "IdleOnLock", 0);
static CMOption<BYTE> g_bIdlePrivate(IDLEMOD, "IdlePrivate", 0);
static CMOption<BYTE> g_bIdleSoundsOff(IDLEMOD, "IdleSoundsOff", 1);
static CMOption<BYTE> g_bIdleOnTerminal(IDLEMOD, "IdleOnTerminalDisconnect", 0);
static CMOption<BYTE> g_bIdleStatusLock(IDLEMOD, "IdleStatusLock", 0);
static CMOption<BYTE> g_bAAEnable(IDLEMOD, "AAEnable", 0);
static CMOption<WORD> g_bAAStatus(IDLEMOD, "AAStatus", 0);
static CMOption<DWORD> g_iIdleTime1st(IDLEMOD, "IdleTime1st", 10);

static bool bModuleInitialized = false;

struct IdleObject
{
	UINT_PTR hTimer;
	int idleType;
	int bIsIdle;
};

static const WORD aa_Status[] = { ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED, ID_STATUS_DND, ID_STATUS_ONTHEPHONE, ID_STATUS_OUTTOLUNCH };

static IdleObject gIdleObject;
static HANDLE hIdleEvent;

void CALLBACK IdleTimer(HWND hwnd, UINT umsg, UINT_PTR idEvent, DWORD dwTime);

static void IdleObject_Create(IdleObject *obj)
{
	memset(obj, 0, sizeof(IdleObject));
	obj->hTimer = SetTimer(nullptr, 0, 2000, IdleTimer);
}

static void IdleObject_Destroy(IdleObject *obj)
{
	if (obj->bIsIdle)
		NotifyEventHooks(hIdleEvent, 0, 0);
	obj->bIsIdle = false;
	KillTimer(nullptr, obj->hTimer);
}

static int IdleObject_IsUserIdle()
{
	if (g_bIdleMethod) {
		DWORD dwTick;
		CallService(MS_SYSTEM_GETIDLE, 0, (LPARAM)&dwTick);
		return GetTickCount() - dwTick > (g_iIdleTime1st * 60 * 1000);
	}

	LASTINPUTINFO ii = { sizeof(ii) };
	if (GetLastInputInfo(&ii))
		return GetTickCount() - ii.dwTime > (g_iIdleTime1st * 60 * 1000);

	return FALSE;
}

static void IdleObject_Tick(IdleObject *obj)
{
	bool idle = false;
	int idleType = 0, flags = 0;

	if (g_bIdleCheck && IdleObject_IsUserIdle()) {
		idleType = 1; idle = true;
	}
	else if (g_bIdleOnSaver && IsScreenSaverRunning()) {
		idleType = 2; idle = true;
	}
	else if (g_bIdleOnFullScr && IsFullScreen()) {
		idleType = 5; idle = true;
	}
	else if (g_bIdleOnLock && IsWorkstationLocked()) {
		idleType = 3; idle = true;
	}
	else if (g_bIdleOnTerminal && IsTerminalDisconnected()) {
		idleType = 4; idle = true;
	}

	if (g_bIdlePrivate)
		flags |= IDF_PRIVACY;

	if (!obj->bIsIdle && idle) {
		obj->bIsIdle = true;
		obj->idleType = idleType;
		NotifyEventHooks(hIdleEvent, 0, IDF_ISIDLE | flags);
	}
	
	if (obj->bIsIdle && !idle) {
		obj->bIsIdle = false;
		obj->idleType = 0;
		NotifyEventHooks(hIdleEvent, 0, flags);
	}
}

void CALLBACK IdleTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	if (gIdleObject.hTimer == idEvent)
		IdleObject_Tick(&gIdleObject);
}

int IdleGetStatusIndex(WORD status)
{
	for (int j = 0; j < _countof(aa_Status); j++)
		if (aa_Status[j] == status)
			return j;

	return 0;
}

class COptionsDlg : public CPluginDlgBase
{
	CCtrlEdit edt1sttime;
	CCtrlSpin spinIdle;
	CCtrlCombo cmbAAStatus;
	CCtrlCheck chkShort, chkOnWindows, chkOnMiranda, chkScreenSaver, chkFullScreen, chkLocked;
	CCtrlCheck chkPrivate, chkStatusLock, chkTerminal, chkSoundsOff, chkShortIdle;

	void ShowHide()
	{
		BOOL bChecked = chkShort.GetState();
		chkOnWindows.Enable(bChecked);
		chkOnMiranda.Enable(bChecked);
		edt1sttime.Enable(bChecked);

		bChecked = chkShortIdle.GetState();
		cmbAAStatus.Enable(bChecked);
		chkStatusLock.Enable(bChecked);
	}

public:
	COptionsDlg() :
		CPluginDlgBase(hInst, IDD_OPT_IDLE, IDLEMOD),
		edt1sttime(this, IDC_IDLE1STTIME),
		spinIdle(this, IDC_IDLESPIN),
		cmbAAStatus(this, IDC_AASTATUS),
		chkShort(this, IDC_IDLESHORT),
		chkLocked(this, IDC_LOCKED),
		chkPrivate(this, IDC_IDLEPRIVATE),
		chkTerminal(this, IDC_IDLETERMINAL),
		chkOnWindows(this, IDC_IDLEONWINDOWS),
		chkSoundsOff(this, IDC_IDLESOUNDSOFF),
		chkOnMiranda(this, IDC_IDLEONMIRANDA),
		chkShortIdle(this, IDC_AASHORTIDLE),
		chkStatusLock(this, IDC_IDLESTATUSLOCK),
		chkFullScreen(this, IDC_FULLSCREEN),
		chkScreenSaver(this, IDC_SCREENSAVER)
	{
		CreateLink(chkShort, g_bIdleCheck);
		CreateLink(chkLocked, g_bIdleOnLock);
		CreateLink(chkPrivate, g_bIdlePrivate);
		CreateLink(chkTerminal, g_bIdleOnTerminal);
		CreateLink(chkShortIdle, g_bAAEnable);
		CreateLink(chkOnMiranda, g_bIdleMethod);
		CreateLink(chkSoundsOff, g_bIdleSoundsOff);
		CreateLink(chkStatusLock, g_bIdleStatusLock);
		CreateLink(chkFullScreen, g_bIdleOnFullScr);
		CreateLink(chkScreenSaver, g_bIdleOnSaver);

		chkShortIdle.OnChange = chkShort.OnChange = Callback(this, &COptionsDlg::onChange);
	}

	virtual void OnInitDialog() override
	{
		chkOnWindows.SetState(!g_bIdleMethod);

		spinIdle.SetRange(60, 1);
		spinIdle.SetPosition(g_iIdleTime1st);

		for (int j = 0; j < _countof(aa_Status); j++)
			cmbAAStatus.AddString(pcli->pfnGetStatusModeDescription(aa_Status[j], 0));
		cmbAAStatus.SetCurSel(IdleGetStatusIndex(g_bAAStatus));

		ShowHide();
	}

	virtual void OnApply() override
	{
		g_iIdleTime1st = spinIdle.GetPosition();

		int curSel = cmbAAStatus.GetCurSel();
		if (curSel != CB_ERR)
			g_bAAStatus = aa_Status[curSel];

		// destroy any current idle and reset settings.
		IdleObject_Destroy(&gIdleObject);
		IdleObject_Create(&gIdleObject);
	}

	void onChange(CCtrlCheck*)
	{
		ShowHide();
	}
};

static int IdleOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_IDLE);
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Idle");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new COptionsDlg();
	Options_AddPage(wParam, &odp);
	return 0;
}

static INT_PTR IdleGetInfo(WPARAM, LPARAM lParam)
{
	MIRANDA_IDLE_INFO *mii = (MIRANDA_IDLE_INFO*)lParam;
	if (!mii || mii->cbSize != sizeof(MIRANDA_IDLE_INFO))
		return 1;

	mii->idleTime = g_iIdleTime1st;
	mii->privacy = g_bIdlePrivate;
	mii->aaStatus = (g_bAAEnable) ? g_bAAStatus : 0;
	mii->aaLock = g_bIdleStatusLock;
	mii->idlesoundsoff = g_bIdleSoundsOff;
	mii->idleType = gIdleObject.idleType;
	return 0;
}

int LoadIdleModule(void)
{
	bModuleInitialized = TRUE;

	hIdleEvent = CreateHookableEvent(ME_IDLE_CHANGED);
	IdleObject_Create(&gIdleObject);
	CreateServiceFunction(MS_IDLE_GETIDLEINFO, IdleGetInfo);
	HookEvent(ME_OPT_INITIALISE, IdleOptInit);
	return 0;
}

void UnloadIdleModule()
{
	if (!bModuleInitialized) return;

	IdleObject_Destroy(&gIdleObject);
	DestroyHookableEvent(hIdleEvent);
	hIdleEvent = nullptr;
}
