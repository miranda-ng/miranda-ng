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

#include "stdafx.h"

#ifdef _DEBUG
#define SECS_PER_MINUTE		20 /* speedup */
#else
#define SECS_PER_MINUTE		60 /* default I believe */
#endif

/////////////////////////////////////////////////////////////////////////////////////////

CFakePlugin AAAPlugin(AAAMODULENAME);

static HANDLE hEvents[3];

static BOOL ignoreLockKeys = FALSE;
static BOOL ignoreSysKeys = FALSE;
static BOOL ignoreAltCombo = FALSE;
static BOOL monitorMouse = TRUE;
static BOOL monitorKeyboard = TRUE;
static HWND confirmDialog;
static int mouseStationaryTimer;
HHOOK hMirandaMouseHook = nullptr;
HHOOK hMirandaKeyBoardHook = nullptr;
#pragma data_seg("Shared")
uint32_t lastInput = 0;
POINT lastMousePos = { 0 };
HHOOK hMouseHook = nullptr;
HHOOK hKeyBoardHook = nullptr;
#pragma data_seg()
#pragma comment(linker, "/section:Shared,rws")
uint32_t lastMirandaInput = 0;
static UINT_PTR hAutoAwayTimer;
// prototypes
extern uint32_t StatusModeToProtoFlag(int status);

int AutoAwayOptInitialise(WPARAM wParam, LPARAM lParam);

static int ProcessProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS || ack->result != ACKRESULT_SUCCESS)
		return 0;

	log_debug(0, "ProcessProtoAck: ack->szModule: %s", ack->szModule);
	for (auto &it : protoList) {
		log_debug(0, "chk: %s", it->m_szName);
		if (!mir_strcmp(it->m_szName, ack->szModule)) {
			log_debug(0, "ack->szModule: %s p.statusChanged: %d", ack->szModule, it->bStatusChanged);
			if (!it->bStatusChanged)
				it->bManualStatus = true;

			it->bStatusChanged = false;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account control event

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

static int changeState(SMProto &setting, int mode, STATES newState)
{
	if (setting.curState == newState)
		return 0;

	setting.oldState = setting.curState;
	setting.curState = newState;

	log_debug(0, "%s state change: %s -> %s", setting.m_szName, status2descr(setting.oldState), status2descr(setting.curState));

	if (setting.curState != SET_ORGSTATUS && setting.curState != ACTIVE && setting.bStatusChanged) {
		/* change the awaymessage */
		if (setting.m_szMsg != nullptr) {
			mir_free(setting.m_szMsg);
			setting.m_szMsg = nullptr;
		}

		if (AAAPlugin.getByte(StatusModeToDbSetting(setting.aaaStatus, SETTING_MSGCUSTOM), FALSE))
			setting.m_szMsg = AAAPlugin.getWStringA(StatusModeToDbSetting(setting.aaaStatus, SETTING_STATUSMSG));
	}
	else if (setting.m_szMsg != nullptr) {
		mir_free(setting.m_szMsg);
		setting.m_szMsg = nullptr;
	}

	if (setting.optionFlags & FLAG_ENTERIDLE) {
		if (newState == ACTIVE) // we're returning back
			Idle_Enter(-1);
		else
			Idle_Enter(mode);
	}

	return 0;
}

static int getIdleMode(int options)
{
	if ((options & FLAG_ONSAVER) && IsScreenSaverRunning())
		return 1;

	if ((options & FLAG_ONLOCK) && IsWorkstationLocked())
		return 2;
	
	if ((options & FLAG_ONTS) && IsTerminalDisconnected())
		return 3;
	
	if ((options & FLAG_FULLSCREEN) && IsFullScreen())
		return 4;

	return 0;
}

static VOID CALLBACK AutoAwayTimer(HWND, UINT, UINT_PTR, DWORD)
{
	int statusChanged = FALSE;
	int confirm = FALSE;

	for (auto &it : protoList) {
		it->aaaStatus = ID_STATUS_DISABLED;

		if (it->optionFlags & FLAG_MONITORMIRANDA)
			mouseStationaryTimer = (GetTickCount() - lastMirandaInput) / 1000;
		else {
			LASTINPUTINFO ii = { sizeof(ii) };
			GetLastInputInfo(&ii);
			mouseStationaryTimer = (GetTickCount() - ii.dwTime) / 1000;
		}

		int sts1Time = it->awayTime * SECS_PER_MINUTE;
		int sts2Time = it->naTime * SECS_PER_MINUTE;
		int sts1setTime = it->sts1setTimer == 0 ? 0 : (GetTickCount() - it->sts1setTimer) / 1000;
		int currentMode = Proto_GetStatus(it->m_szName);

		int mode = getIdleMode(it->optionFlags);

		/* check states */
		if (it->curState == ACTIVE) {
			if (((mouseStationaryTimer >= sts1Time && (it->optionFlags & FLAG_ONMOUSE)) || mode) && currentMode != it->lv1Status && it->statusFlags & StatusModeToProtoFlag(currentMode)) {
				if (it->optionFlags & FLAG_ONMOUSE)
					mode = 5;

				/* from ACTIVE to STATUS1_SET */
				it->m_lastStatus = it->originalStatusMode = Proto_GetStatus(it->m_szName);
				it->aaaStatus = it->lv1Status;
				it->sts1setTimer = GetTickCount();
				sts1setTime = 0;
				it->bStatusChanged = statusChanged = true;
				changeState(*it, mode, STATUS1_SET);
			}
			else if (mouseStationaryTimer >= sts2Time && currentMode == it->lv1Status && currentMode != it->lv2Status && (it->optionFlags & FLAG_SETNA) && (it->statusFlags & StatusModeToProtoFlag(currentMode))) {
				/* from ACTIVE to STATUS2_SET */
				it->m_lastStatus = it->originalStatusMode = Proto_GetStatus(it->m_szName);
				it->aaaStatus = it->lv2Status;
				it->bStatusChanged = statusChanged = true;
				changeState(*it, mode, STATUS2_SET);
			}
		}

		if (it->curState == STATUS1_SET) {
			if ((mouseStationaryTimer < sts1Time && !mode) && !(it->optionFlags & FLAG_RESET)) {
				/* from STATUS1_SET to HIDDEN_ACTIVE */
				changeState(*it, mode, HIDDEN_ACTIVE);
				it->m_lastStatus = Proto_GetStatus(it->m_szName);
			}
			else if (((mouseStationaryTimer < sts1Time) && !mode) &&
				((it->optionFlags & FLAG_LV2ONINACTIVE) || (!(it->optionFlags & FLAG_SETNA))) &&
				(it->optionFlags & FLAG_RESET)) {
				/* from STATUS1_SET to SET_ORGSTATUS */
				changeState(*it, mode, SET_ORGSTATUS);
			}
			else if ((it->optionFlags & FLAG_SETNA) && sts1setTime >= sts2Time) {
				/* when set STATUS2, currentMode doesn't have to be in the selected status list (statusFlags) */
				/* from STATUS1_SET to STATUS2_SET */
				it->m_lastStatus = Proto_GetStatus(it->m_szName);
				it->aaaStatus = it->lv2Status;
				it->bStatusChanged = statusChanged = true;
				changeState(*it, mode, STATUS2_SET);
			}
		}

		if (it->curState == STATUS2_SET) {
			if (mouseStationaryTimer < sts2Time && !mode && (it->optionFlags & FLAG_RESET)) {
				/* from STATUS2_SET to SET_ORGSTATUS */
				changeState(*it, mode, SET_ORGSTATUS);
			}
			else if (mouseStationaryTimer < sts2Time && !mode && !(it->optionFlags & FLAG_RESET)) {
				/* from STATUS2_SET to HIDDEN_ACTIVE */
				/* Remember: after status1 is set, and "only on inactive" is NOT set, it implies !reset. */
				changeState(*it, mode, HIDDEN_ACTIVE);
				it->m_lastStatus = Proto_GetStatus(it->m_szName);
			}
		}

		if (it->curState == HIDDEN_ACTIVE) {
			if (it->bManualStatus) {
				/* HIDDEN_ACTIVE to ACTIVE */
				// it->bStatusChanged = false;
				changeState(*it, mode, ACTIVE);
				it->sts1setTimer = 0;
				it->bManualStatus = false;
			}
			else if ((it->optionFlags & FLAG_SETNA) && currentMode == it->lv1Status &&
				currentMode != it->lv2Status && (it->statusFlags & StatusModeToProtoFlag(currentMode)) &&
				(mouseStationaryTimer >= sts2Time || (sts1setTime >= sts2Time && !(it->optionFlags & FLAG_LV2ONINACTIVE)))) {
				/* HIDDEN_ACTIVE to STATUS2_SET */
				it->m_lastStatus = it->originalStatusMode = Proto_GetStatus(it->m_szName);
				it->aaaStatus = it->lv2Status;
				it->bStatusChanged = statusChanged = true;
				changeState(*it, mode, STATUS2_SET);
			}
		}
		if (it->curState == SET_ORGSTATUS) {
			/* SET_ORGSTATUS to ACTIVE */
			it->m_lastStatus = Proto_GetStatus(it->m_szName);
			it->aaaStatus = it->originalStatusMode;
			confirm = (it->optionFlags & FLAG_CONFIRM) ? TRUE : confirm;
			it->bStatusChanged = statusChanged = true;
			changeState(*it, mode, ACTIVE);
			it->sts1setTimer = 0;
		}
		it->bManualStatus = false;
	}

	if (confirm || statusChanged) {
		TProtoSettings ps(protoList); // make a copy of data not to pollute main array
		for (auto &it : ps)
			it->m_status = it->aaaStatus;

		if (confirm)
			confirmDialog = ShowConfirmDialogEx(&ps, AAAPlugin.getWord(SETTING_CONFIRMDELAY, 5));
		else if (statusChanged)
			SetStatusEx(ps);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Windows hooks 

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
				((GetKeyState(VK_RWIN) < 0) || (wParam == VK_RWIN))) {
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
				((GetKeyState(VK_RWIN) < 0) || (wParam == VK_RWIN))) {
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

static int HookWindowsHooks(int hookMiranda, int hookAll)
{
	if (hookMiranda) {
		if (monitorKeyboard && hMirandaKeyBoardHook == nullptr)
			hMirandaKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, MirandaKeyBoardHookFunction, nullptr, GetCurrentThreadId());
		if (monitorMouse && hMirandaMouseHook == nullptr)
			hMirandaMouseHook = SetWindowsHookEx(WH_MOUSE, MirandaMouseHookFunction, nullptr, GetCurrentThreadId());
	}
	if (hookAll) {
		if (monitorKeyboard && hKeyBoardHook == nullptr)
			hKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardHookFunction, nullptr, GetCurrentThreadId());
		if (monitorMouse && hMouseHook == nullptr)
			hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookFunction, nullptr, GetCurrentThreadId());
	}

	return 0;
}

static int UnhookWindowsHooks()
{
	UnhookWindowsHookEx(hMouseHook);
	UnhookWindowsHookEx(hKeyBoardHook);
	UnhookWindowsHookEx(hMirandaMouseHook);
	UnhookWindowsHookEx(hMirandaKeyBoardHook);

	hMouseHook = hKeyBoardHook = hMirandaMouseHook = hMirandaKeyBoardHook = nullptr;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load from DB

int LoadAutoAwaySetting(SMProto &autoAwaySetting, char *protoName)
{
	char setting[128];
	mir_snprintf(setting, "%s_OptionFlags", protoName);
	autoAwaySetting.optionFlags = AAAPlugin.getWord(setting, FLAG_LV2ONINACTIVE | FLAG_RESET | FLAG_ENTERIDLE);
	mir_snprintf(setting, "%s_AwayTime", protoName);
	autoAwaySetting.awayTime = AAAPlugin.getWord(setting, SETTING_AWAYTIME_DEFAULT);
	mir_snprintf(setting, "%s_NATime", protoName);
	autoAwaySetting.naTime = AAAPlugin.getWord(setting, SETTING_NATIME_DEFAULT);
	mir_snprintf(setting, "%s_StatusFlags", protoName);
	autoAwaySetting.statusFlags = AAAPlugin.getWord(setting, StatusModeToProtoFlag(ID_STATUS_ONLINE) | StatusModeToProtoFlag(ID_STATUS_FREECHAT));

	int flags;
	if (g_bAAASettingSame)
		flags = 0xFFFFFF;
	else
		flags = CallProtoService(protoName, PS_GETCAPS, PFLAGNUM_2, 0) & ~CallProtoService(protoName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);
	mir_snprintf(setting, "%s_Lv1Status", protoName);
	autoAwaySetting.lv1Status = AAAPlugin.getWord(setting, (flags & StatusModeToProtoFlag(ID_STATUS_AWAY)) ? ID_STATUS_AWAY : ID_STATUS_OFFLINE);
	mir_snprintf(setting, "%s_Lv2Status", protoName);
	autoAwaySetting.lv2Status = AAAPlugin.getWord(setting, (flags & StatusModeToProtoFlag(ID_STATUS_NA)) ? ID_STATUS_NA : ID_STATUS_OFFLINE);

	return 0;
}

void AAAUnloadOptions()
{
	UnhookWindowsHooks();
	if (hAutoAwayTimer != 0)
		KillTimer(nullptr, hAutoAwayTimer);
}

void AAALoadOptions()
{
	// if bOverride is enabled, samesettings will be ignored (for options loading)
	AAAUnloadOptions();

	bool monitorMiranda = false, monitorAll = false;

	ignoreLockKeys = AAAPlugin.getByte(SETTING_IGNLOCK, FALSE);
	ignoreSysKeys = AAAPlugin.getByte(SETTING_IGNSYSKEYS, FALSE);
	ignoreAltCombo = AAAPlugin.getByte(SETTING_IGNALTCOMBO, FALSE);
	monitorMouse = AAAPlugin.getByte(SETTING_MONITORMOUSE, TRUE) != 0;
	monitorKeyboard = AAAPlugin.getByte(SETTING_MONITORKEYBOARD, TRUE) != 0;
	lastInput = lastMirandaInput = GetTickCount();

	for (auto &it : protoList) {
		char *protoName;
		if (g_bAAASettingSame)
			protoName = SETTING_ALL;
		else
			protoName = it->m_szName;
		LoadAutoAwaySetting(*it, protoName);

		if (it->optionFlags & FLAG_MONITORMIRANDA)
			monitorMiranda = true;
		else if (ignoreLockKeys || ignoreSysKeys || ignoreAltCombo || (monitorMouse != monitorKeyboard))
			monitorAll = true;
	}

	HookWindowsHooks(monitorMiranda, monitorAll);
	hAutoAwayTimer = SetTimer(nullptr, 0, AAAPlugin.getWord(SETTING_AWAYCHECKTIMEINSECS, 5) * 1000, AutoAwayTimer);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Inits & stuff

static int AutoAwayShutdown(WPARAM, LPARAM)
{
	AAAUnloadOptions();
	return 0;
}

static int AAAModuleLoaded(WPARAM, LPARAM)
{
	hEvents[0] = HookEvent(ME_OPT_INITIALISE, AutoAwayOptInitialise);
	hEvents[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, AutoAwayShutdown);
	hEvents[2] = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
	
	mouseStationaryTimer = 0;
	lastInput = lastMirandaInput = GetTickCount();

	AAALoadOptions();
	return 0;
}

void AdvancedAutoAwayLoad()
{
	g_bAAASettingSame = AAAPlugin.getByte(SETTING_SAMESETTINGS);

	if (g_plugin.bMirandaLoaded)
		AAAModuleLoaded(0, 0);
	else
		HookEvent(ME_SYSTEM_MODULESLOADED, AAAModuleLoaded);
}

void AdvancedAutoAwayUnload()
{
	if (g_plugin.bMirandaLoaded)
		AutoAwayShutdown(0, 0);

	KillModuleOptions(&AAAPlugin);

	for (auto &it : hEvents) {
		UnhookEvent(it);
		it = nullptr;
	}

	protoList.destroy();
}
