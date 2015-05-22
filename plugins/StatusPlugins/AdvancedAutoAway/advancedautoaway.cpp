/*
    AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
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

	
	Some code is copied from Miranda's AutoAway module
*/
#include "../commonstatus.h"
#include "advancedautoaway.h"
#include "../resource.h"
#include <commctrl.h>

#ifdef _DEBUG
	#define SECS_PER_MINUTE		20 /* speedup */
#else
	#define SECS_PER_MINUTE		60 /* default I believe */
#endif

/////////////////////////////////////////////////////////////////////////////////////////

int CompareSettings(const TAAAProtoSetting *p1, const TAAAProtoSetting *p2)
{
	return mir_strcmp(p1->szName, p2->szName);
}

TAAAProtoSettingList autoAwaySettings(10, CompareSettings);

TAAAProtoSetting::TAAAProtoSetting(PROTOACCOUNT *pa)
{
	cbSize = sizeof(PROTOCOLSETTINGEX);
	szName = pa->szModuleName;
	tszAccName = pa->tszAccountName;
	lastStatus = status = originalStatusMode = ID_STATUS_CURRENT;
	szMsg = NULL;
	curState = ACTIVE;
	mStatus = FALSE;
}

TAAAProtoSetting::~TAAAProtoSetting()
{
	free(szMsg);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern HANDLE hStateChangedEvent;

static BOOL ignoreLockKeys = FALSE;
static BOOL ignoreSysKeys = FALSE;
static BOOL ignoreAltCombo = FALSE;
static BOOL monitorMouse = TRUE;
static BOOL monitorKeyboard = TRUE;
static HWND confirmDialog;
static int mouseStationaryTimer;
HHOOK hMirandaMouseHook = NULL;
HHOOK hMirandaKeyBoardHook = NULL;
#pragma data_seg("Shared")
DWORD lastInput = 0;
POINT lastMousePos = {0};
HHOOK hMouseHook = NULL;
HHOOK hKeyBoardHook = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:Shared,rws")
DWORD lastMirandaInput = 0;
static UINT_PTR hAutoAwayTimer;
// prototypes
extern DWORD StatusModeToProtoFlag(int status);
extern int InitCommonStatus();
static int HookWindowsHooks(int hookMiranda, int hookAll);
static int UnhookWindowsHooks();
static LRESULT CALLBACK MouseHookFunction(int code, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK KeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MirandaMouseHookFunction(int code, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MirandaKeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam);
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved);

static VOID CALLBACK AutoAwayTimer(HWND hwnd,UINT message,UINT_PTR idEvent,DWORD dwTime);
extern int AutoAwayOptInitialise(WPARAM wParam,LPARAM lParam);
extern int AutoAwayMsgOptInitialise(WPARAM wParam,LPARAM lParam);
extern int SetStatus(WPARAM wParam, LPARAM lParam);
extern int ShowConfirmDialog(WPARAM wParam, LPARAM lParam);
extern char *StatusModeToDbSetting(int status,const char *suffix);

/////////////////////////////////////////////////////////////////////////////////////////
// Load from DB

void LoadOptions(TAAAProtoSettingList &loadSettings, BOOL override)
{
	// if override is enabled, samesettings will be ignored (for options loading)
	int monitorMiranda = FALSE; // use windows hooks?
	int monitorAll = FALSE; // use windows hooks?

	if (!override)
		UnhookWindowsHooks();
	if (hAutoAwayTimer != 0)
		KillTimer(NULL, hAutoAwayTimer);

	ignoreLockKeys = db_get_b(NULL, MODULENAME, SETTING_IGNLOCK, FALSE);
	ignoreSysKeys = db_get_b(NULL, MODULENAME, SETTING_IGNSYSKEYS, FALSE);
	ignoreAltCombo = db_get_b(NULL, MODULENAME, SETTING_IGNALTCOMBO, FALSE);
	monitorMouse = db_get_b(NULL, MODULENAME, SETTING_MONITORMOUSE, TRUE);
	monitorKeyboard = db_get_b(NULL, MODULENAME, SETTING_MONITORKEYBOARD, TRUE);
	lastInput = lastMirandaInput = GetTickCount();

	for (int i = 0; i < loadSettings.getCount(); i++) {
		char* protoName;
		if ((db_get_b(NULL, MODULENAME, SETTING_SAMESETTINGS, 0)) && !override)
			protoName = SETTING_ALL;
		else
			protoName = loadSettings[i].szName;
		LoadAutoAwaySetting(loadSettings[i], protoName);
		if (!override) {
			if (loadSettings[i].optionFlags & FLAG_MONITORMIRANDA)
				monitorMiranda = TRUE;
			else if (ignoreLockKeys || ignoreSysKeys || ignoreAltCombo || (monitorMouse != monitorKeyboard))
				monitorAll = TRUE;
		}
	}

	if (db_get_b(NULL, "Idle", "AAEnable", 0))
		return;

	HookWindowsHooks(monitorMiranda, monitorAll);
	hAutoAwayTimer = SetTimer(NULL, 0, db_get_w(NULL, MODULENAME, SETTING_AWAYCHECKTIMEINSECS, 5) * 1000, AutoAwayTimer);
}

int LoadAutoAwaySetting(TAAAProtoSetting &autoAwaySetting, char* protoName)
{
	char setting[128];
	mir_snprintf(setting, SIZEOF(setting), "%s_OptionFlags", protoName);
	autoAwaySetting.optionFlags = db_get_w(NULL, MODULENAME, setting, FLAG_LV2ONINACTIVE | FLAG_RESET);
	mir_snprintf(setting, SIZEOF(setting), "%s_AwayTime", protoName);
	autoAwaySetting.awayTime = db_get_w(NULL, MODULENAME, setting, SETTING_AWAYTIME_DEFAULT);
	mir_snprintf(setting, SIZEOF(setting), "%s_NATime", protoName);
	autoAwaySetting.naTime = db_get_w(NULL, MODULENAME, setting, SETTING_NATIME_DEFAULT);
	mir_snprintf(setting, SIZEOF(setting), "%s_StatusFlags", protoName);
	autoAwaySetting.statusFlags = db_get_w(NULL, MODULENAME, setting, StatusModeToProtoFlag(ID_STATUS_ONLINE) | StatusModeToProtoFlag(ID_STATUS_FREECHAT));

	int flags;
	if (db_get_b(NULL, MODULENAME, SETTING_SAMESETTINGS, 0))
		flags = 0xFFFFFF;
	else
		flags = CallProtoService(protoName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(protoName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);
	mir_snprintf(setting, SIZEOF(setting), "%s_Lv1Status", protoName);
	autoAwaySetting.lv1Status = db_get_w(NULL, MODULENAME, setting, (flags&StatusModeToProtoFlag(ID_STATUS_AWAY)) ? ID_STATUS_AWAY : ID_STATUS_OFFLINE);
	mir_snprintf(setting, SIZEOF(setting), "%s_Lv2Status", protoName);
	autoAwaySetting.lv2Status = db_get_w(NULL, MODULENAME, setting, (flags&StatusModeToProtoFlag(ID_STATUS_NA)) ? ID_STATUS_NA : ID_STATUS_OFFLINE);

	return 0;
}

static int ProcessProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS || ack->result != ACKRESULT_SUCCESS)
		return 0;

	log_debugA("ProcessProtoAck: ack->szModule: %s", ack->szModule);
	for (int i = 0; i < autoAwaySettings.getCount(); i++) {
		TAAAProtoSetting &p = autoAwaySettings[i];
		log_debugA("chk: %s", p.szName);
		if (!mir_strcmp(p.szName, ack->szModule)) {
			log_debugA("ack->szModule: %s p.statusChanged: %d", ack->szModule, p.statusChanged);
			if (!p.statusChanged)
				p.mStatus = TRUE;

			p.statusChanged = FALSE;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account control event

int OnAccChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;
	switch (wParam) {
	case PRAC_ADDED:
		autoAwaySettings.insert(new TAAAProtoSetting(pa));
		break;

	case PRAC_REMOVED:
		for (int i = 0; i < autoAwaySettings.getCount(); i++) {
			if (!mir_strcmp(autoAwaySettings[i].szName, pa->szModuleName)) {
				autoAwaySettings.remove(i);
				break;
			}
		}
		break;
	}

	return 0;
}

static char* status2descr(int status)
{
	switch (status) {
		case ACTIVE:        return "ACTIVE";
		case STATUS1_SET:   return "STATUS1_SET";
		case STATUS2_SET:   return "STATUS2_SET";
		case SET_ORGSTATUS: return "SET_ORGSTATUS";
		case HIDDEN_ACTIVE: return "HIDDEN_ACTIVE";
	}
	return "ERROR";
}

static int changeState(TAAAProtoSetting &setting, STATES newState)
{
	if (setting.curState == newState)
		return 0;

	setting.oldState = setting.curState;
	setting.curState = newState;

	log_debugA("%s state change: %s -> %s", setting.szName, status2descr(setting.oldState), status2descr(setting.curState));

	NotifyEventHooks(hStateChangedEvent, 0, (LPARAM)(AUTOAWAYSETTING*)&setting);
	if (setting.curState != SET_ORGSTATUS && setting.curState != ACTIVE && setting.statusChanged) {
		/* change the awaymessage */
		if (setting.szMsg != NULL) {
			free(setting.szMsg);
			setting.szMsg = NULL;
		}

		if (db_get_b(NULL, MODULENAME, StatusModeToDbSetting(setting.status, SETTING_MSGCUSTOM), FALSE)) {
			DBVARIANT dbv;
			if (!db_get_ts(NULL, MODULENAME, StatusModeToDbSetting(setting.status, SETTING_STATUSMSG), &dbv)) {
				setting.szMsg = _tcsdup(dbv.ptszVal);
				db_free(&dbv);
			}
		}
	}
	else if (setting.szMsg != NULL) {
		free(setting.szMsg);
		setting.szMsg = NULL;
	}

	return 0;
}

static VOID CALLBACK AutoAwayTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	int statusChanged = FALSE;
	int confirm = FALSE;

	for (int i = 0; i < autoAwaySettings.getCount(); i++) {
		TAAAProtoSetting& aas = autoAwaySettings[i];
		aas.status = ID_STATUS_DISABLED;

		BOOL bTrigger = false;

		if (aas.optionFlags & FLAG_MONITORMIRANDA)
			mouseStationaryTimer = (GetTickCount() - lastMirandaInput) / 1000;
		else {
			LASTINPUTINFO ii = { sizeof(ii) };
			GetLastInputInfo(&ii);
			mouseStationaryTimer = (GetTickCount() - ii.dwTime) / 1000;
		}

		int sts1Time = aas.awayTime * SECS_PER_MINUTE;
		int sts2Time = aas.naTime * SECS_PER_MINUTE;
		int sts1setTime = aas.sts1setTimer == 0 ? 0 : (GetTickCount() - aas.sts1setTimer) / 1000;
		int currentMode = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);

		if (aas.optionFlags & FLAG_ONSAVER)
			bTrigger |= IsScreenSaverRunning();
		if (aas.optionFlags & FLAG_ONLOCK)
			bTrigger |= IsWorkstationLocked();
		if (aas.optionFlags & FLAG_FULLSCREEN)
			bTrigger |= IsFullScreen();

		/* check states */
		if (aas.curState == ACTIVE) {
			if (((mouseStationaryTimer >= sts1Time && (aas.optionFlags & FLAG_ONMOUSE)) || bTrigger) && currentMode != aas.lv1Status && aas.statusFlags&StatusModeToProtoFlag(currentMode)) {
				/* from ACTIVE to STATUS1_SET */
				aas.lastStatus = aas.originalStatusMode = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
				aas.status = aas.lv1Status;
				aas.sts1setTimer = GetTickCount();
				sts1setTime = 0;
				aas.statusChanged = statusChanged = TRUE;
				changeState(aas, STATUS1_SET);
			}
			else if (mouseStationaryTimer >= sts2Time && currentMode == aas.lv1Status && currentMode != aas.lv2Status && (aas.optionFlags & FLAG_SETNA) && (aas.statusFlags & StatusModeToProtoFlag(currentMode))) {
				/* from ACTIVE to STATUS2_SET */
				aas.lastStatus = aas.originalStatusMode = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
				aas.status = aas.lv2Status;
				aas.statusChanged = statusChanged = TRUE;
				changeState(aas, STATUS2_SET);
			}
		}

		if (aas.curState == STATUS1_SET) {
			if ((mouseStationaryTimer < sts1Time && !bTrigger) && !(aas.optionFlags & FLAG_RESET)) {
				/* from STATUS1_SET to HIDDEN_ACTIVE */
				changeState(aas, HIDDEN_ACTIVE);
				aas.lastStatus = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
			}
			else if (((mouseStationaryTimer < sts1Time) && !bTrigger) &&
						((aas.optionFlags & FLAG_LV2ONINACTIVE) || (!(aas.optionFlags&FLAG_SETNA))) &&
						(aas.optionFlags & FLAG_RESET)) {
				/* from STATUS1_SET to SET_ORGSTATUS */
				changeState(aas, SET_ORGSTATUS);
			}
			else if ((aas.optionFlags & FLAG_SETNA) && sts1setTime >= sts2Time) {
				/* when set STATUS2, currentMode doesn't have to be in the selected status list (statusFlags) */
				/* from STATUS1_SET to STATUS2_SET */
				aas.lastStatus = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
				aas.status = aas.lv2Status;
				aas.statusChanged = statusChanged = TRUE;
				changeState(aas, STATUS2_SET);
			}
		}

		if (aas.curState == STATUS2_SET) {
			if (mouseStationaryTimer < sts2Time && !bTrigger && (aas.optionFlags & FLAG_RESET)) {
				/* from STATUS2_SET to SET_ORGSTATUS */
				changeState(aas, SET_ORGSTATUS);
			}
			else if (mouseStationaryTimer < sts2Time && !bTrigger && !(aas.optionFlags & FLAG_RESET)) {
				/* from STATUS2_SET to HIDDEN_ACTIVE */
				/* Remember: after status1 is set, and "only on inactive" is NOT set, it implies !reset. */
				changeState(aas, HIDDEN_ACTIVE);
				aas.lastStatus = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
			}
		}

		if (aas.curState == HIDDEN_ACTIVE) {
			if (aas.mStatus) {
				/* HIDDEN_ACTIVE to ACTIVE */
				//aas.statusChanged = FALSE;
				changeState(aas, ACTIVE);
				aas.sts1setTimer = 0;
				aas.mStatus = FALSE;
			}
			else if ((aas.optionFlags & FLAG_SETNA) && currentMode == aas.lv1Status &&
						currentMode != aas.lv2Status && (aas.statusFlags & StatusModeToProtoFlag(currentMode)) &&
						(mouseStationaryTimer >= sts2Time || (sts1setTime >= sts2Time && !(aas.optionFlags & FLAG_LV2ONINACTIVE)))) {
				/* HIDDEN_ACTIVE to STATUS2_SET */
				aas.lastStatus = aas.originalStatusMode = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
				aas.status = aas.lv2Status;
				aas.statusChanged = statusChanged = TRUE;
				changeState(aas, STATUS2_SET);
			}
		}
		if (aas.curState == SET_ORGSTATUS) {
			/* SET_ORGSTATUS to ACTIVE */
			aas.lastStatus = CallProtoService(aas.szName, PS_GETSTATUS, 0, 0);
			aas.status = aas.originalStatusMode;
			confirm = (aas.optionFlags&FLAG_CONFIRM) ? TRUE : confirm;
			aas.statusChanged = statusChanged = TRUE;
			changeState(aas, ACTIVE);
			aas.sts1setTimer = 0;
		}
		autoAwaySettings[i].mStatus = FALSE;
	}

	if (confirm || statusChanged) {
		TAAAProtoSettingList ps = autoAwaySettings;
		for (int i = 0; i < ps.getCount(); i++) {
			if (ps[i].szMsg)
				ps[i].szMsg = _tcsdup(ps[i].szMsg);

			if (ps[i].status == ID_STATUS_DISABLED)
				ps[i].szName = "";
		}

		if (confirm)
			confirmDialog = (HWND)CallService(MS_CS_SHOWCONFIRMDLGEX, (WPARAM)&ps, db_get_w(NULL, MODULENAME, SETTING_CONFIRMDELAY, 5));
		else if (statusChanged)
			CallService(MS_CS_SETSTATUSEX, (WPARAM)&ps, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Windows hooks 

static int HookWindowsHooks(int hookMiranda, int hookAll)
{
	if (hookMiranda) {
		if (monitorKeyboard && hMirandaKeyBoardHook == NULL)
			hMirandaKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, MirandaKeyBoardHookFunction, NULL, GetCurrentThreadId());
		if (monitorMouse && hMirandaMouseHook == NULL)
			hMirandaMouseHook = SetWindowsHookEx(WH_MOUSE, MirandaMouseHookFunction, NULL, GetCurrentThreadId());
	}
	if (hookAll) {
		if (monitorKeyboard && hKeyBoardHook == NULL)
			hKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardHookFunction, 0, GetCurrentThreadId());
		if (monitorMouse && hMouseHook == NULL)
			hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookFunction, 0, GetCurrentThreadId());
	}

	return 0;
}

static int UnhookWindowsHooks()
{
	UnhookWindowsHookEx(hMouseHook);
	UnhookWindowsHookEx(hKeyBoardHook);
	UnhookWindowsHookEx(hMirandaMouseHook);
	UnhookWindowsHookEx(hMirandaKeyBoardHook);

	hMouseHook = hKeyBoardHook = hMirandaMouseHook = hMirandaKeyBoardHook = NULL;
	return 0;
}

static LRESULT CALLBACK MirandaMouseHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0) {
		PMOUSEHOOKSTRUCT mouseInfo = (PMOUSEHOOKSTRUCT)lParam;
		POINT pt = mouseInfo->pt;

		/* TioDuke's KeyBoardNotifyExt: only update if a Miranda window is focused */
		DWORD pid;
		GetWindowThreadProcessId(GetForegroundWindow(), &pid);
		if (pid != GetCurrentProcessId())
			return CallNextHookEx(hMirandaMouseHook, code, wParam, lParam);

		if (pt.x != lastMousePos.x || pt.y != lastMousePos.y) {
			lastMousePos = pt;
			lastMirandaInput = GetTickCount();
		}
	}

	return CallNextHookEx(hMirandaMouseHook, code, wParam, lParam);
}

static LRESULT CALLBACK MirandaKeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0) {
		if (ignoreAltCombo) {
			if (((GetKeyState(VK_MENU) < 0) || (wParam == VK_MENU)) ||
				 ((GetKeyState(VK_TAB) < 0) || (wParam == VK_TAB)) ||
				 ((GetKeyState(VK_SHIFT) < 0) || (wParam == VK_SHIFT)) ||
				 ((GetKeyState(VK_CONTROL) < 0) || (wParam == VK_CONTROL)) ||
				 ((GetKeyState(VK_ESCAPE) < 0) || (wParam == VK_ESCAPE)) ||
				 ((GetKeyState(VK_LWIN) < 0) || (wParam == VK_LWIN)) ||
				((GetKeyState(VK_RWIN) < 0) || (wParam == VK_RWIN)))
			{
				return CallNextHookEx(hMirandaKeyBoardHook, code, wParam, lParam);
			}
		}

		switch (wParam) {
		case VK_NUMLOCK:
		case VK_CAPITAL:
		case VK_SCROLL:
			if (!ignoreLockKeys)
				lastMirandaInput = GetTickCount();
			break;

		case VK_TAB:
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
		case VK_ESCAPE:
		case VK_LWIN:
		case VK_RWIN:
			if (!ignoreSysKeys)
				lastMirandaInput = GetTickCount();
			break;

		default:
			lastMirandaInput = GetTickCount();
			break;
		}
	}

	return CallNextHookEx(hMirandaKeyBoardHook, code, wParam, lParam);
}

static LRESULT CALLBACK MouseHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0) {
		PMOUSEHOOKSTRUCT mouseInfo = (PMOUSEHOOKSTRUCT)lParam;
		POINT pt = mouseInfo->pt;

		/* TioDuke's KeyBoardNotifyExt: also grab clicks */
		if ((wParam >= WM_NCLBUTTONDOWN && wParam <= WM_NCXBUTTONDBLCLK && wParam != 0x00AA) || (wParam >= WM_LBUTTONDOWN && wParam <= WM_XBUTTONDBLCLK))
			lastInput = GetTickCount();

		if (pt.x != lastMousePos.x || pt.y != lastMousePos.y) {
			lastMousePos = pt;
			lastInput = GetTickCount();
		}
	}

	return CallNextHookEx(hMouseHook, code, wParam, lParam);
}

static LRESULT CALLBACK KeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0) {
		if (ignoreAltCombo) {
			if (((GetKeyState(VK_MENU) < 0) || (wParam == VK_MENU)) ||
				 ((GetKeyState(VK_TAB) < 0) || (wParam == VK_TAB)) ||
				 ((GetKeyState(VK_SHIFT) < 0) || (wParam == VK_SHIFT)) ||
				 ((GetKeyState(VK_CONTROL) < 0) || (wParam == VK_CONTROL)) ||
				 ((GetKeyState(VK_ESCAPE) < 0) || (wParam == VK_ESCAPE)) ||
				 ((GetKeyState(VK_LWIN) < 0) || (wParam == VK_LWIN)) ||
				((GetKeyState(VK_RWIN) < 0) || (wParam == VK_RWIN)))
			{
				return CallNextHookEx(hKeyBoardHook, code, wParam, lParam);
			}
		}

		switch (wParam) {
		case VK_NUMLOCK:
		case VK_CAPITAL:
		case VK_SCROLL:
			if (!ignoreLockKeys)
				lastInput = GetTickCount();
			break;

		case VK_TAB:
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
		case VK_ESCAPE:
		case VK_LWIN:
		case VK_RWIN:
			if (!ignoreSysKeys)
				lastInput = GetTickCount();
			break;

		default:
			lastInput = GetTickCount();
			break;
		}
	}

	return CallNextHookEx(hKeyBoardHook, code, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Inits & stuff

static int AutoAwayShutdown(WPARAM wParam, LPARAM lParam)
{
	KillTimer(NULL, hAutoAwayTimer);

#ifdef TRIGGERPLUGIN
	DeInitTrigger();
#endif
	UnhookWindowsHooks();
	DestroyHookableEvent(hStateChangedEvent);

	autoAwaySettings.destroy();
	return 0;
}

int CSModuleLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccChanged);
	HookEvent(ME_OPT_INITIALISE, AutoAwayOptInitialise);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, AutoAwayShutdown);
	HookEvent(ME_PROTO_ACK, ProcessProtoAck);
	mouseStationaryTimer = 0;
	lastInput = lastMirandaInput = GetTickCount();

	////////////////////////////////////////////////////////////////////////////////////////

	protoList = (OBJLIST<PROTOCOLSETTINGEX>*)&autoAwaySettings;

	int count;
	PROTOACCOUNT** protos;
	ProtoEnumAccounts(&count, &protos);

	for (int i = 0; i < count; i++)
		if (IsSuitableProto(protos[i]))
			autoAwaySettings.insert(new TAAAProtoSetting(protos[i]));

	////////////////////////////////////////////////////////////////////////////////////////

	LoadOptions(autoAwaySettings, FALSE);
	return 0;
}
