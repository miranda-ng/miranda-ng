/*
				KeyboardNotify plugin v1.5 for Miranda IM
				_________________________________________

  Copyright (C) 2002,2003  Martin Öberg
  Copyright (C) 2004	   Std
  Copyright (C) 2005,2006  TioDuke (tioduke@yahoo.ca)


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

#define NCONVERS_BLINKID ((MEVENT)123456) //nconvers' random identifier used to flash an icon for "incoming message" on contact list

CMPlugin g_plugin;

DWORD IDThread = 0;
HANDLE hThread = nullptr;
HANDLE hFlashEvent;
HANDLE hExitEvent;

HHOOK hMirandaMouseHook = nullptr;
HHOOK hMirandaKeyBoardHook = nullptr;
HHOOK hMirandaWndProcHook = nullptr;
UINT hReminderTimer = 0;

HHOOK hMouseHook = nullptr;
HHOOK hKeyBoardHook = nullptr;
BYTE  bEmulateKeypresses = 0;
DWORD dwLastInput = 0;
POINT lastGlobalMousePos = { 0, 0 };

BYTE bFlashOnMsg;
BYTE bFlashOnFile;
BYTE bFlashOnGC;
BYTE bFlashOnOther;
BYTE bFullScreenMode;
BYTE bScreenSaverRunning;
BYTE bWorkstationLocked;
BYTE bProcessesAreRunning;
BYTE bWorkstationActive;
BYTE bFlashIfMsgOpen;
BYTE bFlashIfMsgWinNotTop;
BYTE bFlashIfMsgOlder;
WORD wSecondsOlder;
BYTE bFlashUntil;
WORD wBlinksNumber;
BYTE bMirandaOrWindows;
WORD wStatusMap;
WORD wReminderCheck;
BYTE bFlashLed[3];
BYTE bFlashEffect;
BYTE bSequenceOrder;
WORD wCustomTheme;
WORD wStartDelay;
BYTE bFlashSpeed;
BYTE bOverride;
BYTE bTrillianLedsMsg;
BYTE bTrillianLedsURL;
BYTE bTrillianLedsFile;
BYTE bTrillianLedsOther;

PROTOCOL_LIST ProtoList = { 0, nullptr };
PROCESS_LIST ProcessList = { 0, nullptr };

int nWaitDelay;
unsigned int nExternCount = 0;
BOOL bFlashingEnabled = TRUE;
BOOL bReminderDisabled = FALSE;

BYTE bMetaProtoEnabled = 0;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//{119D7288-2050-448D-9900-D86AC70426BF}
	{0x119d7288, 0x2050, 0x448d, {0x99, 0x00, 0xd8, 0x6a, 0xc7, 0x04, 0x26, 0xbf}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("keybdnotify", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int InitializeOptions(WPARAM, LPARAM);
BOOL CheckMsgWnd(MCONTACT, BOOL *);

BOOL checkOpenWindow(MCONTACT hContact)
{
	if (bFlashIfMsgOpen && !bFlashIfMsgWinNotTop)
		return TRUE;

	BOOL focus, found = CheckMsgWnd(hContact, &focus);
	if (!found && bMetaProtoEnabled)
		found = CheckMsgWnd(db_mc_getMeta(hContact), &focus);

	if (!found)
		return TRUE;

	if (bFlashIfMsgOpen && !focus)
		return TRUE;

	return FALSE;
}

BOOL checkNotifyOptions()
{
	BOOL screenSaverIsRunning = IsScreenSaverRunning();
	if (screenSaverIsRunning && bScreenSaverRunning)
		return TRUE;

	BOOL workstationIsLocked = IsWorkstationLocked();
	if (workstationIsLocked && bWorkstationLocked)
		return TRUE;

	BOOL fullScreenMode = IsFullScreen() && !screenSaverIsRunning;
	if (fullScreenMode && bFullScreenMode)
		return TRUE;

	BOOL processesRunning = areThereProcessesRunning();
	if (processesRunning && bProcessesAreRunning)
		return TRUE;

	return (!fullScreenMode && !screenSaverIsRunning && !workstationIsLocked && !processesRunning && bWorkstationActive);
}

BOOL isStatusEnabled(int status)
{
	switch (status) {
		case ID_STATUS_OFFLINE:    return wStatusMap & MAP_OFFLINE;
		case ID_STATUS_ONLINE:     return wStatusMap & MAP_ONLINE;
		case ID_STATUS_AWAY:       return wStatusMap & MAP_AWAY;
		case ID_STATUS_NA:         return wStatusMap & MAP_NA;
		case ID_STATUS_OCCUPIED:   return wStatusMap & MAP_OCCUPIED;
		case ID_STATUS_DND:        return wStatusMap & MAP_DND;
		case ID_STATUS_FREECHAT:   return wStatusMap & MAP_FREECHAT;
		case ID_STATUS_INVISIBLE:  return wStatusMap & MAP_INVISIBLE;
	}
	return FALSE;
}

BOOL checkGlobalStatus()
{
	return isStatusEnabled(CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
}

BOOL checkGlobalXstatus()
{
	int protosSupporting = 0, status = 0;

	for (int i = 0; i < ProtoList.protoCount; i++) {
		if (!ProtoList.protoInfo[i].enabled || !ProtoList.protoInfo[i].xstatus.count)
			continue;

		protosSupporting++;
		// Retrieve xstatus for protocol
		CUSTOM_STATUS xstatus = { sizeof(CUSTOM_STATUS) };
		xstatus.flags = CSSF_MASK_STATUS;
		xstatus.status = &status;
		CallProtoService(ProtoList.protoInfo[i].szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);

		if (ProtoList.protoInfo[i].xstatus.enabled[status])
			return TRUE;
	}

	return protosSupporting == 0;
}

DBEVENTINFO createMsgEventInfo(MCONTACT hContact)
{
	DBEVENTINFO einfo = {};
	einfo.eventType = EVENTTYPE_MESSAGE;
	einfo.szModule = Proto_GetBaseAccountName(hContact);
	return einfo;
}

DBEVENTINFO readEventInfo(MEVENT hDbEvent, MCONTACT hContact)
{
	if (hDbEvent == NCONVERS_BLINKID) // we need to handle nconvers' blink event
		return createMsgEventInfo(hContact);

	DBEVENTINFO einfo = {};
	db_event_get(hDbEvent, &einfo);
	return einfo;
}

BOOL checkIgnore(MCONTACT hContact, WORD eventType)
{
	return !IsIgnored(hContact, eventType);
}

BOOL checkProtocol(const char *szProto)
{
	if (!szProto)
		return FALSE;

	for (int i = 0; i < ProtoList.protoCount; i++)
		if (ProtoList.protoInfo[i].szProto && !mir_strcmp(ProtoList.protoInfo[i].szProto, szProto))
			return ProtoList.protoInfo[i].enabled;

	return FALSE;
}

BOOL metaCheckProtocol(const char *szProto, MCONTACT hContact, WORD eventType)
{
	MCONTACT hSubContact = NULL;

	if (bMetaProtoEnabled && szProto && !mir_strcmp(META_PROTO, szProto))
		if (hSubContact = db_mc_getMostOnline(hContact))
			szProto = Proto_GetBaseAccountName(hSubContact);

	return checkProtocol(szProto) && checkIgnore(hSubContact ? hSubContact : hContact, eventType);
}


BOOL checkUnopenEvents()
{
	int nIndex;
	CLISTEVENT *pCLEvent;

	if (nExternCount && bFlashOnOther)
		return TRUE;

	for (nIndex = 0; pCLEvent = g_clistApi.pfnGetEvent(-1, nIndex); nIndex++) {
		DBEVENTINFO einfo = readEventInfo(pCLEvent->hDbEvent, pCLEvent->hContact);

		if ((einfo.eventType == EVENTTYPE_MESSAGE && bFlashOnMsg) ||
			(einfo.eventType == EVENTTYPE_FILE    && bFlashOnFile) ||
			(einfo.eventType != EVENTTYPE_MESSAGE && einfo.eventType != EVENTTYPE_FILE && bFlashOnOther))

			if (metaCheckProtocol(einfo.szModule, pCLEvent->hContact, einfo.eventType))
				return TRUE;
	}

	return FALSE;
}

static void __cdecl FlashThreadFunction(void*)
{
	BOOL bEvent = FALSE;
	DWORD dwEventStarted = 0, dwFlashStarted = 0;
	BYTE data, unchangedLeds;
	
	Thread_SetName("KeyboardNotify: FlashThreadFunction");
	MThreadHandle threadLock(hThread);

	while (true) {
		unchangedLeds = (BYTE)(LedState(VK_PAUSE) * !bFlashLed[2] + ((LedState(VK_NUMLOCK) * !bFlashLed[0]) << 1) + ((LedState(VK_CAPITAL) * !bFlashLed[1]) << 2));
		GetAsyncKeyState(VK_PAUSE); // empty Pause/Break's keystroke buffer

		// Start flashing
		while (bEvent && bFlashingEnabled) {
			// Let's give the user the opportunity of finishing flashing manually :)
			if (GetAsyncKeyState(VK_PAUSE) & 1)
				break;

			if ((bFlashUntil & UNTIL_NBLINKS) && GetTickCount() > (dwFlashStarted + wBlinksNumber * 1000))
				break;
			if (bFlashUntil & UNTIL_REATTENDED) {
				if (bMirandaOrWindows == ACTIVE_WINDOWS && !bEmulateKeypresses) {
					LASTINPUTINFO ii = { sizeof(ii) };
					GetLastInputInfo(&ii);
					dwLastInput = ii.dwTime;
				}
				if (dwLastInput > dwEventStarted)
					break;
			}
			if ((bFlashUntil & UNTIL_EVENTSOPEN) && !checkUnopenEvents())
				break;
			if ((bFlashUntil & UNTIL_CONDITIONS) && (!checkNotifyOptions() || !checkGlobalStatus() || !checkGlobalXstatus()))
				break;

			data = getBlinkingLeds();
			ToggleKeyboardLights((BYTE)(data | unchangedLeds));

			// Wait for exit event
			if (WaitForSingleObject(hExitEvent, nWaitDelay) == WAIT_OBJECT_0) {
				Netlib_Logf(0, "%s: got exit signal1", g_plugin.getModule());
				return;
			}
		}
		RestoreLEDState();

		setFlashingSequence();
		bReminderDisabled = FALSE;

		// Wait for new event
		HANDLE Objects[2];
		Objects[0] = hFlashEvent;
		Objects[1] = hExitEvent;
		if (WaitForMultipleObjects(_countof(Objects), Objects, FALSE, INFINITE) != WAIT_OBJECT_0) {
			Netlib_Logf(0, "%s: got exit signal2", g_plugin.getModule());
			return;
		}

		bEvent = TRUE;
		bReminderDisabled = TRUE;
		dwEventStarted = GetTickCount();
		// Wait StartDelay seconds
		if (wStartDelay > 0)
			Sleep(wStartDelay * 1000);
		dwFlashStarted = GetTickCount();
	}
}

BOOL checkMsgTimestamp(MCONTACT hContact, MEVENT hEventCurrent, DWORD timestampCurrent)
{
	if (!bFlashIfMsgOlder)
		return TRUE;

	for (MEVENT hEvent = db_event_prev(hContact, hEventCurrent); hEvent; hEvent = db_event_prev(hContact, hEvent)) {
		DBEVENTINFO einfo = {};
		if (!db_event_get(hEvent, &einfo)) {
			if ((einfo.timestamp + wSecondsOlder) <= timestampCurrent)
				return TRUE;
			if (einfo.eventType == EVENTTYPE_MESSAGE)
				return FALSE;
		}
	}

	return TRUE;
}


BOOL contactCheckProtocol(const char *szProto, MCONTACT hContact, WORD eventType)
{
	if (bMetaProtoEnabled && hContact) {
		MCONTACT hMetaContact = (MCONTACT)db_get_dw(hContact, META_PROTO, "Handle", 0);
		if (hMetaContact && db_mc_getMeta(hContact) == hMetaContact)
			return FALSE;
	}

	return metaCheckProtocol(szProto, hContact, eventType);
}


BOOL checkStatus(const char *szProto)
{
	if (!szProto)
		return checkGlobalStatus();

	return isStatusEnabled(Proto_GetStatus(szProto));
}


BOOL checkXstatus(const char *szProto)
{
	int status = 0;

	if (!szProto)
		return checkGlobalXstatus();

	for (int i = 0; i < ProtoList.protoCount; i++)
		if (ProtoList.protoInfo[i].szProto && !mir_strcmp(ProtoList.protoInfo[i].szProto, szProto)) {
			if (!ProtoList.protoInfo[i].xstatus.count) return TRUE;

			// Retrieve xstatus for protocol
			CUSTOM_STATUS xstatus = { sizeof(CUSTOM_STATUS) };
			xstatus.flags = CSSF_MASK_STATUS;
			xstatus.status = &status;
			CallProtoService(ProtoList.protoInfo[i].szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);

			return ProtoList.protoInfo[i].xstatus.enabled[status];
		}

	return TRUE;
}


// 'Pings' the FlashThread to keep the LEDs flashing.
static int PluginMessageEventHook(WPARAM hContact, LPARAM hEvent)
{
	//get DBEVENTINFO without pBlob
	DBEVENTINFO einfo = {};
	if (!db_event_get(hEvent, &einfo) && !(einfo.flags & DBEF_SENT))
		if ((einfo.eventType == EVENTTYPE_MESSAGE && bFlashOnMsg && checkOpenWindow(hContact) && checkMsgTimestamp(hContact, hEvent, einfo.timestamp)) ||
			(einfo.eventType == EVENTTYPE_FILE    && bFlashOnFile) ||
			(einfo.eventType != EVENTTYPE_MESSAGE && einfo.eventType != EVENTTYPE_FILE && bFlashOnOther)) {

			if (contactCheckProtocol(einfo.szModule, hContact, einfo.eventType) && checkNotifyOptions() && checkStatus(einfo.szModule) && checkXstatus(einfo.szModule))
				SetEvent(hFlashEvent);
		}

	return 0;
}

// monitors group chat events
static int OnGcEvent(WPARAM, LPARAM lParam)
{
	auto *gce = (GCEVENT *)lParam;
	if (gce->iType == GC_EVENT_MESSAGE && bFlashOnGC)
		if (SESSION_INFO *si = g_chatApi.SM_FindSession(gce->pszID.w, gce->pszModule))
			if (contactCheckProtocol(si->pszModule, si->hContact, EVENTTYPE_MESSAGE) && checkNotifyOptions() && checkStatus(si->pszModule))
				SetEvent(hFlashEvent);

	return 0;
}

// **
// ** Checks for pending events. If it finds any, it pings the FlashThread to keep the LEDs flashing.
// **

static VOID CALLBACK ReminderTimer(HWND, UINT, UINT_PTR, DWORD)
{
	int nIndex;
	CLISTEVENT *pCLEvent;

	if (!bReminderDisabled && nExternCount && bFlashOnOther) {
		SetEvent(hFlashEvent);
		return;
	}

	for (nIndex = 0; !bReminderDisabled && (pCLEvent = g_clistApi.pfnGetEvent(-1, nIndex)); nIndex++) {
		DBEVENTINFO einfo = readEventInfo(pCLEvent->hDbEvent, pCLEvent->hContact);

		if ((einfo.eventType == EVENTTYPE_MESSAGE && bFlashOnMsg) ||
			(einfo.eventType == EVENTTYPE_FILE    && bFlashOnFile) ||
			(einfo.eventType != EVENTTYPE_MESSAGE && einfo.eventType != EVENTTYPE_FILE && bFlashOnOther))

			if (metaCheckProtocol(einfo.szModule, pCLEvent->hContact, einfo.eventType) && checkNotifyOptions() && checkStatus(einfo.szModule) && checkXstatus(einfo.szModule)) {

				SetEvent(hFlashEvent);
				return;
			}
	}
}

// Support for third-party plugins and mBot's scripts
static INT_PTR EnableService(WPARAM, LPARAM)
{
	bFlashingEnabled = TRUE;
	return 0;
}

static INT_PTR DisableService(WPARAM, LPARAM)
{
	bFlashingEnabled = FALSE;
	return 0;
}

static INT_PTR StartBlinkService(WPARAM wParam, LPARAM lParam)
{
	nExternCount += (unsigned int)wParam;
	if (bFlashOnOther && checkNotifyOptions() && checkGlobalStatus() && checkGlobalXstatus()) {
		if (lParam)
			useExternSequence((wchar_t *)lParam);
		SetEvent(hFlashEvent);
	}

	return 0;
}

static INT_PTR EventsWereOpenedService(WPARAM wParam, LPARAM)
{
	if ((unsigned int)wParam > nExternCount)
		nExternCount = 0;
	else
		nExternCount -= (unsigned int)wParam;

	return 0;
}

static INT_PTR IsFlashingActiveService(WPARAM, LPARAM)
{
	if (!bReminderDisabled)
		return 0;

	return (INT_PTR)getCurrentSequenceString();
}

INT_PTR NormalizeSequenceService(WPARAM, LPARAM lParam)
{
	wchar_t strAux[MAX_PATH + 1], *strIn = (wchar_t*)lParam;

	wcsncpy_s(strAux, strIn, _TRUNCATE);
	wcsncpy_s(strIn, MAX_PATH, normalizeCustomString(strAux), _TRUNCATE);

	return (INT_PTR)strIn;
}

// Support for Trigger plugin
static void __cdecl ForceEventsWereOpenedThread(void *eventMaxSeconds)
{
	Sleep(((UINT_PTR)eventMaxSeconds) * 1000);
	CallService(MS_KBDNOTIFY_EVENTSOPENED, 1, 0);
}

void StartBlinkAction(char *flashSequence, WORD eventMaxSeconds)
{
	if (eventMaxSeconds)
		mir_forkthread(ForceEventsWereOpenedThread, (void *)eventMaxSeconds);

	CallService(MS_KBDNOTIFY_STARTBLINK, 1, (LPARAM)flashSequence);
}

void createProcessList(void)
{
	int count = g_plugin.getWord("processcount", 0);

	ProcessList.count = 0;
	ProcessList.szFileName = (wchar_t **)mir_alloc(count * sizeof(wchar_t *));
	if (ProcessList.szFileName) {
		for (int i = 0; i < count; i++)
			ProcessList.szFileName[i] = g_plugin.getWStringA(fmtDBSettingName("process%d", i));

		ProcessList.count = count;
	}
}

void destroyProcessList(void)
{
	if (ProcessList.szFileName == nullptr)
		return;

	for (int i = 0; i < ProcessList.count; i++)
		if (ProcessList.szFileName[i])
			mir_free(ProcessList.szFileName[i]);

	mir_free(ProcessList.szFileName);
	ProcessList.count = 0;
	ProcessList.szFileName = nullptr;
}

//========================== Windows hooks ==========================

static LRESULT CALLBACK MouseHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0) {
		/* This should handle all mouse buttons ... */
		if ((wParam >= WM_NCLBUTTONDOWN && wParam <= WM_NCXBUTTONDBLCLK && wParam != 0x00AA) || (wParam >= WM_LBUTTONDOWN && wParam <= WM_XBUTTONDBLCLK))
			dwLastInput = GetTickCount();
		/* ... and here it is either mouse move, hover, leave or something unexpected */
		else {
			PMOUSEHOOKSTRUCT mouseInfo = (PMOUSEHOOKSTRUCT)lParam;
			POINT pt = mouseInfo->pt;
			if (pt.x != lastGlobalMousePos.x || pt.y != lastGlobalMousePos.y) {
				lastGlobalMousePos = pt;
				dwLastInput = GetTickCount();
			}
		}
	}

	return CallNextHookEx(hMouseHook, code, wParam, lParam);
}

static LRESULT CALLBACK KeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0 && (!bEmulateKeypresses || (bEmulateKeypresses && wParam != VK_NUMLOCK && wParam != VK_CAPITAL && wParam != VK_SCROLL)))
		dwLastInput = GetTickCount();

	return CallNextHookEx(hKeyBoardHook, code, wParam, lParam);
}

static LRESULT CALLBACK MirandaMouseHookFunction(int code, WPARAM wParam, LPARAM lParam)
{
	static POINT lastMousePos = { 0, 0 };

	if (code >= 0) {
		/* Movement mouse messages are for some reason incoming in inactive/background window too, that is not input */
		DWORD pid;
		GetWindowThreadProcessId(GetForegroundWindow(), &pid);
		if (pid == GetCurrentProcessId()) {
			/* This should handle all mouse buttons ... */
			if ((wParam >= WM_NCLBUTTONDOWN && wParam <= WM_NCXBUTTONDBLCLK && wParam != 0x00AA) || (wParam >= WM_LBUTTONDOWN && wParam <= WM_XBUTTONDBLCLK))
				dwLastInput = GetTickCount();
			/* ... and here it is either mouse move, hover, leave or something unexpected */
			else {
				PMOUSEHOOKSTRUCT mouseInfo = (PMOUSEHOOKSTRUCT)lParam;
				POINT pt = mouseInfo->pt;
				if (pt.x != lastMousePos.x || pt.y != lastMousePos.y) {
					lastMousePos = pt;
					dwLastInput = GetTickCount();
				}
			}
		}
	}

	return CallNextHookEx(hMirandaMouseHook, code, wParam, lParam);
}

static LRESULT CALLBACK MirandaKeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam)
{

	if (code >= 0 && (!bEmulateKeypresses || (bEmulateKeypresses && wParam != VK_NUMLOCK && wParam != VK_CAPITAL && wParam != VK_SCROLL)))
		dwLastInput = GetTickCount();

	return CallNextHookEx(hMirandaKeyBoardHook, code, wParam, lParam);
}

static LRESULT CALLBACK MirandaWndProcHookFunction(int code, WPARAM wParam, LPARAM lParam)
{

	if (code >= 0) {
		/* WM_ACTIVATEAPP with nonzero wParam means someone brought miranda to foreground, that equals to input */
		PCWPSTRUCT cwpInfo = (PCWPSTRUCT)lParam;
		if (cwpInfo->message == WM_ACTIVATEAPP && cwpInfo->wParam)
			dwLastInput = GetTickCount();
	}

	return CallNextHookEx(hMirandaWndProcHook, code, wParam, lParam);
}

int HookWindowsHooks()
{
	if (wReminderCheck)
		hReminderTimer = SetTimer(nullptr, 0, wReminderCheck * 60000, ReminderTimer);

	if (bFlashUntil & UNTIL_REATTENDED) {
		switch (bMirandaOrWindows) {
		case ACTIVE_WINDOWS:
			if (bEmulateKeypresses) {
				if (hMouseHook == nullptr)
					hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookFunction, nullptr, GetCurrentThreadId());
				if (hKeyBoardHook == nullptr)
					hKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardHookFunction, nullptr, GetCurrentThreadId());
			}
			break;

		case ACTIVE_MIRANDA:
			if (hMirandaMouseHook == nullptr)
				hMirandaMouseHook = SetWindowsHookEx(WH_MOUSE, MirandaMouseHookFunction, nullptr, GetCurrentThreadId());
			if (hMirandaKeyBoardHook == nullptr)
				hMirandaKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, MirandaKeyBoardHookFunction, nullptr, GetCurrentThreadId());
			if (hMirandaWndProcHook == nullptr)
				hMirandaWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, MirandaWndProcHookFunction, nullptr, GetCurrentThreadId());
		}
	}

	return 0;
}

int UnhookWindowsHooks()
{
	if (hReminderTimer)
		KillTimer(nullptr, hReminderTimer);
	if (hMouseHook)
		UnhookWindowsHookEx(hMouseHook);
	if (hKeyBoardHook)
		UnhookWindowsHookEx(hKeyBoardHook);
	if (hMirandaMouseHook)
		UnhookWindowsHookEx(hMirandaMouseHook);
	if (hMirandaKeyBoardHook)
		UnhookWindowsHookEx(hMirandaKeyBoardHook);
	if (hMirandaWndProcHook)
		UnhookWindowsHookEx(hMirandaWndProcHook);

	hReminderTimer = 0;
	hMouseHook = hKeyBoardHook = hMirandaMouseHook = hMirandaKeyBoardHook = hMirandaWndProcHook = nullptr;
	return 0;
}

void LoadSettings(void)
{
	bFlashOnMsg = g_plugin.getByte("onmsg", DEF_SETTING_ONMSG);
	bFlashOnFile = g_plugin.getByte("onfile", DEF_SETTING_ONFILE);
	bFlashOnGC = g_plugin.getByte("ongcmsg", DEF_SETTING_ONMSGGC);
	bFlashOnOther = g_plugin.getByte("onother", DEF_SETTING_OTHER);
	bFullScreenMode = g_plugin.getByte("fscreenmode", DEF_SETTING_FSCREEN);
	bScreenSaverRunning = g_plugin.getByte("ssaverrunning", DEF_SETTING_SSAVER);
	bWorkstationLocked = g_plugin.getByte("wstationlocked", DEF_SETTING_LOCKED);
	bProcessesAreRunning = g_plugin.getByte("procsrunning", DEF_SETTING_PROCS);
	bWorkstationActive = g_plugin.getByte("wstationactive", DEF_SETTING_ACTIVE);
	bFlashIfMsgOpen = g_plugin.getByte("ifmsgopen", DEF_SETTING_IFMSGOPEN);
	bFlashIfMsgWinNotTop = g_plugin.getByte("ifmsgnottop", DEF_SETTING_IFMSGNOTTOP);
	bFlashIfMsgOlder = g_plugin.getByte("ifmsgolder", DEF_SETTING_IFMSGOLDER);
	wSecondsOlder = g_plugin.getWord("secsolder", DEF_SETTING_SECSOLDER);
	bFlashUntil = g_plugin.getByte("funtil", DEF_SETTING_FLASHUNTIL);
	wBlinksNumber = g_plugin.getWord("nblinks", DEF_SETTING_NBLINKS);
	bMirandaOrWindows = g_plugin.getByte("mirorwin", DEF_SETTING_MIRORWIN);
	wStatusMap = g_plugin.getWord("status", DEF_SETTING_STATUS);
	wReminderCheck = g_plugin.getWord("remcheck", DEF_SETTING_CHECKTIME);
	bFlashLed[0] = g_plugin.getByte("fnum", DEF_SETTING_FLASHNUM);
	bFlashLed[1] = g_plugin.getByte("fcaps", DEF_SETTING_FLASHCAPS);
	bFlashLed[2] = g_plugin.getByte("fscroll", DEF_SETTING_FLASHSCROLL);
	bFlashEffect = g_plugin.getByte("feffect", DEF_SETTING_FLASHEFFECT);
	bSequenceOrder = g_plugin.getByte("order", DEF_SETTING_SEQORDER);
	wCustomTheme = g_plugin.getWord("custom", DEF_SETTING_CUSTOMTHEME);
	bTrillianLedsMsg = g_plugin.getByte("ledsmsg", DEF_SETTING_LEDSMSG);
	bTrillianLedsURL = g_plugin.getByte("ledsurl", DEF_SETTING_LEDSURL);
	bTrillianLedsFile = g_plugin.getByte("ledsfile", DEF_SETTING_LEDSFILE);
	bTrillianLedsOther = g_plugin.getByte("ledsother", DEF_SETTING_LEDSOTHER);
	wStartDelay = g_plugin.getWord("sdelay", DEF_SETTING_STARTDELAY);
	bFlashSpeed = g_plugin.getByte("speed", DEF_SETTING_FLASHSPEED);
	switch (bFlashSpeed) {
	case 0:	 nWaitDelay = 1500; break;
	case 1:  nWaitDelay = 0750; break;
	case 2:  nWaitDelay = 0250; break;
	case 3:  nWaitDelay = 0150; break;
	case 4:  nWaitDelay = 0100; break;
	default: nWaitDelay = 0050; break;
	}
	setFlashingSequence();
	bEmulateKeypresses = g_plugin.getByte("keypresses", DEF_SETTING_KEYPRESSES);
	bOverride = g_plugin.getByte("override", DEF_SETTING_OVERRIDE);
	// Create hidden settings (for test button) if needed
	if (g_plugin.getByte("testnum", -1) == -1)
		g_plugin.setByte("testnum", DEF_SETTING_TESTNUM);
	if (g_plugin.getByte("testsecs", -1) == -1)
		g_plugin.setByte("testsecs", DEF_SETTING_TESTSECS);
	for (int i = 0; i < ProtoList.protoCount; i++)
		if (ProtoList.protoInfo[i].visible) {
			unsigned int j;
			ProtoList.protoInfo[i].enabled = g_plugin.getByte(ProtoList.protoInfo[i].szProto, DEF_SETTING_PROTOCOL);
			for (j = 0; j < ProtoList.protoInfo[i].xstatus.count; j++)
				ProtoList.protoInfo[i].xstatus.enabled[j] = g_plugin.getByte(fmtDBSettingName("%sxstatus%d", ProtoList.protoInfo[i].szProto, j), DEF_SETTING_XSTATUS);
		}

	bMetaProtoEnabled = db_mc_isEnabled();

	destroyProcessList();
	createProcessList();
	UnhookWindowsHooks();
	HookWindowsHooks();
}


void updateXstatusProto(PROTOCOL_INFO *protoInfo)
{
	if (!ProtoServiceExists(protoInfo->szProto, PS_GETCUSTOMSTATUSEX))
		return;

	// Retrieve xstatus.count
	CUSTOM_STATUS xstatus = { sizeof(xstatus) };
	xstatus.flags = CSSF_STATUSES_COUNT;
	xstatus.wParam = &(protoInfo->xstatus.count);
	CallProtoService(protoInfo->szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);
	(protoInfo->xstatus.count)++;	// Don't forget about xstatus=0 (None)

	// Alloc and initiailize xstatus.enabled array
	protoInfo->xstatus.enabled = (BOOL *)mir_alloc(protoInfo->xstatus.count * sizeof(BOOL));
	if (!protoInfo->xstatus.enabled)
		protoInfo->xstatus.count = 0;
	else
		for (unsigned i = 0; i < protoInfo->xstatus.count; i++)
			protoInfo->xstatus.enabled[i] = FALSE;
}


void createProtocolList(void)
{
	PROTOACCOUNT **proto;
	Proto_EnumAccounts(&ProtoList.protoCount, &proto);
	ProtoList.protoInfo = (PROTOCOL_INFO *)mir_alloc(ProtoList.protoCount * sizeof(PROTOCOL_INFO));
	if (!ProtoList.protoInfo) {
		ProtoList.protoCount = 0;
		return;
	}

	for (int i = 0; i < ProtoList.protoCount; i++) {
		ProtoList.protoInfo[i].xstatus.count = 0;
		ProtoList.protoInfo[i].xstatus.enabled = nullptr;
		ProtoList.protoInfo[i].szProto = mir_strdup(proto[i]->szModuleName);
		if (!ProtoList.protoInfo[i].szProto) {
			ProtoList.protoInfo[i].enabled = FALSE;
			ProtoList.protoInfo[i].visible = FALSE;
		}
		else {
			ProtoList.protoInfo[i].enabled = FALSE;
			if (!mir_strcmp(proto[i]->szModuleName, META_PROTO))
				ProtoList.protoInfo[i].visible = FALSE;
			else {
				ProtoList.protoInfo[i].visible = TRUE;
				updateXstatusProto(&(ProtoList.protoInfo[i]));
			}
		}
	}
}

static void destroyProtocolList(void)
{
	if (ProtoList.protoInfo == nullptr)
		return;

	for (int i = 0; i < ProtoList.protoCount; i++) {
		if (ProtoList.protoInfo[i].szProto)
			mir_free(ProtoList.protoInfo[i].szProto);
		if (ProtoList.protoInfo[i].xstatus.enabled)
			mir_free(ProtoList.protoInfo[i].xstatus.enabled);
	}

	mir_free(ProtoList.protoInfo);
	ProtoList.protoCount = 0;
	ProtoList.protoInfo = nullptr;
}



// We use the profile name to create the first part of each event name
// We do so to avoid problems between different instances of the plugin concurrently running
void createEventPrefix(wchar_t *prefixName, size_t maxLen)
{
	size_t len;
	wchar_t profileName[MAX_PATH + 1], *str;

	getAbsoluteProfileName(profileName, MAX_PATH);

	while (str = wcschr(profileName, '\\'))
		*str = '/';
	if ((len = mir_wstrlen(profileName)) <= maxLen)
		wcsncpy_s(prefixName, maxLen, profileName, _TRUNCATE);
	else {
		str = profileName + len - maxLen / 2;
		wcsncpy_s(prefixName, (maxLen / 2), profileName, _TRUNCATE);
		mir_wstrcat(prefixName, str);
	}
}


BOOL CheckMsgWnd(MCONTACT hContact, BOOL *focus)
{
	if (hContact) {
		MessageWindowData mwd;
		if (!Srmm_GetWindowData(hContact, mwd) && mwd.hwndWindow) {
			*focus = mwd.uState & MSG_WINDOW_STATE_FOCUS;
			return TRUE;
		}
	}

	*focus = FALSE;
	return FALSE;
}


void countUnopenEvents(int *msgCount, int *fileCount, int *otherCount)
{
	int nIndex;
	CLISTEVENT *pCLEvent;

	for (nIndex = 0; pCLEvent = g_clistApi.pfnGetEvent(-1, nIndex); nIndex++) {
		DBEVENTINFO einfo = readEventInfo(pCLEvent->hDbEvent, pCLEvent->hContact);

		if (metaCheckProtocol(einfo.szModule, pCLEvent->hContact, einfo.eventType)) {
			switch (einfo.eventType) {
			case EVENTTYPE_MESSAGE:
				if (bFlashOnMsg)
					(*msgCount)++;
				break;
			case EVENTTYPE_FILE:
				if (bFlashOnFile)
					(*fileCount)++;
				break;
			default:
				if (bFlashOnOther)
					(*otherCount)++;
			}
		}
	}
	if (bFlashOnOther)
		(*otherCount) += nExternCount;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static int OnMetaChanged(WPARAM wParam, LPARAM)
{
	bMetaProtoEnabled = wParam;
	return 0;
}

static int OnPreshutdown(WPARAM, LPARAM)
{
	Netlib_Logf(0, "%s: got ME_SYSTEM_PRESHUTDOWN", g_plugin.getModule());
	SetEvent(hExitEvent);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	wchar_t eventPrefix[MAX_PATH + 1], eventName[MAX_PATH + 1];

	createProtocolList();
	LoadSettings();

	// Create some synchronisation objects
	createEventPrefix(eventPrefix, MAX_PATH - 11);
	mir_snwprintf(eventName, L"%s/FlashEvent", eventPrefix);
	hFlashEvent = CreateEvent(nullptr, FALSE, FALSE, eventName);
	mir_snwprintf(eventName, L"%s/ExitEvent", eventPrefix);
	hExitEvent = CreateEvent(nullptr, FALSE, FALSE, eventName);

	mir_forkthread(FlashThreadFunction);

	CreateServiceFunction(MS_KBDNOTIFY_ENABLE, EnableService);
	CreateServiceFunction(MS_KBDNOTIFY_DISABLE, DisableService);
	CreateServiceFunction(MS_KBDNOTIFY_STARTBLINK, StartBlinkService);
	CreateServiceFunction(MS_KBDNOTIFY_EVENTSOPENED, EventsWereOpenedService);
	CreateServiceFunction(MS_KBDNOTIFY_FLASHINGACTIVE, IsFlashingActiveService);
	CreateServiceFunction(MS_KBDNOTIFY_NORMALSEQUENCE, NormalizeSequenceService);
	return 0;
}

int CMPlugin::Load()
{
	OpenKeyboardDevice();

	HookEvent(ME_MC_ENABLED, OnMetaChanged);
	HookEvent(ME_GC_HOOK_EVENT, OnGcEvent);
	HookEvent(ME_DB_EVENT_ADDED, PluginMessageEventHook);
	HookEvent(ME_OPT_INITIALISE, InitializeOptions);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreshutdown);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

int CMPlugin::Unload()
{
	UnhookWindowsHooks();

	// Wait for thread to exit
	if (hThread)
		WaitForSingleObject(hThread, INFINITE);

	if (hExitEvent)
		CloseHandle(hExitEvent);
	if (hFlashEvent)
		CloseHandle(hFlashEvent);

	RestoreLEDState();
	CloseKeyboardDevice();

	destroyProcessList();
	destroyProtocolList();
	return 0;
}
