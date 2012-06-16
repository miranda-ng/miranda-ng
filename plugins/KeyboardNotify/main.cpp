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


  Description
  -----------
  This plugin for Miranda-IM notifies user of specified events (as incoming messages,
  incoming files, incoming URLs or other events). This plugin is based on the original one
  by Martin Öberg (aka strickz) and Std's modifications (mainly the idea of using direct
  port handling using a driver).
  It has many options allowing:
  a) To select on which events to react
  b) Under which conditions (eg: fullscreen mode, ScreenSaver running, workstation locked)
  c) To act only if the protocol receiving the event is under specified status
  d) For message events you can choose to be notified if the message window is open or not
  e) A notification feature allowing to be notified of pending events (unopen events)
  after specified period of time
  f) To select method for stopping the blinking (after x secs, if Miranda is re-attended,
  if Windows is re-attended, if all notified events are opened or when the notify conditions
  end)
  g) And several flashing options: select leds to blink, effects (all at the same time,
  in turn, in sequence and like KITT!), preview button
  It was designed to be flexible and performing several different tasks. It can be
  configured to act like the original one and has several functions from other Miranda's
  keyboard notifier plugins.
  It also provides a service to allow third party plugins use its notifier abilities.

  Options
  -------
  Options page Options->Plugins->Keyboard Flash. Tabbed: Protocols, Rules (when), Flashing (how), Themes and Ignore.

  Thanks
  ------
  - Pete for the numerous patches he sent, actively helping to improve the code and
  functionality
  - UnregistereD for great help in solving problem with Windows activity detection
  - Slacktarn, Sir_qwerty and Tweety for giving great help with ideas (most of the new
  features included in this plugin were suggested by them) and testing
  - The authors of AAA, PopUp+, KeyScrollNotify, original KeyboardNotify, Neweventnotify,
  IEView, NGEventNotify for part of their code used in this plugin.
  - Vampik fot the InTurn flashing option
  - Miranda IM developers for this amazing program
  - all other people from Miranda community

  History
  -------
  1.5.7.7:
    [!] Added support for Miranda 0.8.x.x.
  1.5.7.6:
    [!] Fixed bug in Ignore module.
  1.5.7.5:
    [!] Updated TriggerPlugin support for latest version.
  1.5.7.4:
    [*] Updated screenshot
	[*] Minor code cleaning
  1.5.7.3:
    [+] Added xstatus support
  1.5.7.2:
    [+] Added per contact Ignore options
  1.5.7.1:
    [!] Fix in Options for themes under WinXP+ (really essential feature)
  1.5.7.0:
    [+] Added support for Trigger plugin
  1.5.6.3:
    [-] Removed device presence validation: it is not needed now that the plugin works on USB (thanks Nick, aka Aestetic)
    [+] Added a new service to the API for 'normalizing' a custom flashing sequence string
    [-] Simplified the API (the extended version of the start blink service is no longer needed).
  1.5.6.2:
    [!] Fixed problem while trying to detect if message window is in foreground.
  1.5.6.1:
    [!] Fixed bug with keypress emulation and "until Windows is re-attended" option.
  1.5.6.0:
    [+] Option to emulate keypresses (for the USB people)
    [*] Changed the emergency key (to make it stop with PAUSE instead of SCROLL LOCK key).
  1.5.5.4:
    [*] Improved ListView control handling
    [*] Changed the default values (for the sake of new users).
  1.5.5.3:
    [*] More code optimization.
  1.5.5.2:
    [+] Support for Update plugin.
  1.5.5.1:
    [!] Minor source fixes.
  1.5.5.0:
    [+] New 'notify when' option: while defined programs are running (just like gamerstatus)
    [+] Extended the API to add two new services to disable and re-enable keyboard notifications (for use by bosskey plugin).
  1.5.4.4:
    [!] Fixed (hopefully) problems with some system configurations (with PS2 keyboards) where the KeyboardClass0 device was not the apropriate one (thanks pete!)
    [+] Extended the plugin API (requested bt tweety).
  1.5.4.3:
    [!] Fixed some compatibility issues with nconvers++ (thank you donatas for your help).
  1.5.4.2:
    [!] Fixed problem with Windows' activity detection under Win9X when using other plugins that do the same.
    [!] Fixed crash caused by incoming authorisation requests when metacontacts was enabled.
  1.5.4.1:
    [!] Some corrections on third party plugins events handling (now they are more assimilated to the 'other events')
    [*] Some code cleaning
    [!] Fixed problem with first message in Metacontacts recognition while checking for pending events (thank you again NirG)
  1.5.4.0:
    [+] New plugin API (thank you CriS for your ideas and great help)
    [!] Added Offline status to status check list (thank you Slaktarn for finding it).
  1.5.3.4:
    [!] Fixed Metacontacts recognition in checking and counting of pending events (thank you NirG for finding the problem)
    [!] Fixed problems with multiple instances of the plugin running (thank you tweety for reporting and testing).
  1.5.3.3:
    [!] Changed behaviour of Preview button to make it independent of the rules' options.
  1.5.3.2:
    [+] New dialog to asign leds to specific events for the trillian-like sequences.
  1.5.3.1:
    [!] Fixed bug of loosing any other until setting when selecting 'Events are opened'.
  1.5.3.0:
    [+] Applied pete's patches (thank you very much for your great work)
        - Use of GetLastInputInfo when possible for detecting Windows' activity
        - Made Windows' mouse hooks also aware of mouse clicking
        - Made Miranda re-attended option react on windows restoring and ignoring mouse hovering an unfocused window
        - New option for message events to avoid blinking if message window is focused
        - Made the plugin handle metacontact's special issues
    [!] Use of the new message API for windows detection when possible
    [+] New message event option to check last message timestamp (requested by D46MD)
    [+] Possibility of choosing more than one flash until option at the same time
    [+] New flashing effect to make the leds blink accordingly to number of events
    [+] Possibility of selecting/unselecting protocols (requested by tweety, usuful to avoid flashing on some protocols as rss)
  1.5.2.2:
    [!] scriver's message window detection (thanks D46MD for your great help)
    [!] corrected 'flash until' checking accordingly to pete's patch (thank you)
  1.5.2.1:
    [!] nconvers++'s message window detection
    [!] checked window detection for srmm, scriver, sramm and srmm_mod
  1.5.2.0:
    [+] Custom theme support
    [-] Custom order history
  1.5.1.0:
    [+] Custom order effect
    [+] Custom order history
  1.5.0.0:
    [+] Drivers aren't needed anymore
    [+] Status selection option
    [+] Miranda/Windows activity detection (thank you Peter Boon)
    [+] 'Opened events' stop method
    [+] x seconds stop method
    [+] Hooking database event for detecting incoming events
    [+] Notifier option for pending events
    [+] Checkbox for enabling disabling open messages notification
    [+] In sequence and KIT flashing effects
    [+] Preview button
    [+] Tabbed options
    [!] Several corrections/improvements in options page
    [!] Not selected leds will preserve their original state
  1.4.1.0: (by me and Vampik)
    [+] Notify on fullscreen, screensaver, worksation locked
    [!] Try to improve Win98 keysimulation routines
    [+] Added InTurn effect (thank you Vampik)
    [!] Corrected speed of blinking (thank you Vampik)
  1.4.0.0: (by Std, unreleased)
    [+] Added direct port handling using PortTalk.sys driver
  1.3.0.0: (by strickz)
    This is strickz' final release. It still uses keypress simulation. It was nice (thanks *g*)


*/

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "flash.h"
#include "ignore.h"
#include "keyboard.h"
#include "trigger.h"
#include "constants.h"
#include "protolist.h"
#include "EnumProc.h"
#include "utils.h"
#include "m_kbdnotify.h"
#include <newpluginapi.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clist.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_utils.h>
#include <m_icq.h>
#include <m_metacontacts.h>
#include <m_updater.h>
#pragma comment(lib, "advapi32.lib")

#define NCONVERS_BLINKID ((HANDLE)123456) //nconvers' random identifier used to flash an icon for "incoming message" on contact list

#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif

#ifndef WM_XBUTTONDBLCLK
#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef WM_NCXBUTTONDBLCLK
#define WM_NCXBUTTONDBLCLK 0x00AD
#endif


HINSTANCE hInst;
PLUGINLINK *pluginLink;
int hLangpack;
struct MM_INTERFACE mmi;

DWORD IDThread = 0;
HANDLE hThread = NULL;
HANDLE hFlashEvent;
HANDLE hExitEvent;

HANDLE hModulesLoaded = NULL;
HANDLE hMsgEventHook = NULL;
HANDLE hOptionsInitialize = NULL;
HANDLE hEnableService = NULL;
HANDLE hDisableService = NULL;
HANDLE hStartBlinkService = NULL;
HANDLE hEventsOpenedService = NULL;
HANDLE hFlashingEventService = NULL;
HANDLE hNormalizeSequenceService = NULL;

HHOOK hMirandaMouseHook = NULL;
HHOOK hMirandaKeyBoardHook = NULL;
HHOOK hMirandaWndProcHook = NULL;
UINT hReminderTimer = 0;

#pragma data_seg("Shared")
HHOOK hMouseHook = NULL;
HHOOK hKeyBoardHook = NULL;
BYTE bEmulateKeypresses = 0;
DWORD dwLastInput = 0;
POINT lastGlobalMousePos = {0, 0};
#pragma data_seg()
#pragma comment(linker, "/section:Shared,rws")

static BOOL (WINAPI * MyGetLastInputInfo)(PLASTINPUTINFO);


BYTE bFlashOnMsg;
BYTE bFlashOnURL;
BYTE bFlashOnFile;
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

PROTOCOL_LIST ProtoList = {0, NULL};
PROCESS_LIST ProcessList = {0, NULL};

double dWinVer;
BOOL bWindowsNT;

int nWaitDelay;
unsigned int nExternCount = 0;
BOOL bFlashingEnabled = TRUE;
BOOL bReminderDisabled = FALSE;

char *szMetaProto = NULL;
BYTE bMetaProtoEnabled = 0;

#define MIID_KBDNOTIFY	{0x119d7288, 0x2050, 0x448d, { 0x99, 0x00, 0xd8, 0x6a, 0xc7, 0x04, 0x26, 0xbf }}

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Keyboard Notify Ext.",
	PLUGIN_MAKE_VERSION(1,5,7,7),
	"Flashes your keyboard LEDs when a message has arrived",
	"TioDuke",
	"tioduke@yahoo.ca",
	"© 2002-2003 M.Öberg, 2004 Std, 2005-2008 TioDuke",
	"http://addons.miranda-im.org/",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	MIID_KBDNOTIFY //{119D7288-2050-448d-9900-D86AC70426BF}
};



int InitializeOptions(WPARAM,LPARAM);
void LoadSettings(void);
int HookWindowsHooks(void);
int UnhookWindowsHooks(void);
static LRESULT CALLBACK MouseHookFunction(int, WPARAM, LPARAM);
static LRESULT CALLBACK KeyBoardHookFunction(int, WPARAM, LPARAM);
static LRESULT CALLBACK MirandaMouseHookFunction(int, WPARAM, LPARAM);
static LRESULT CALLBACK MirandaKeyBoardHookFunction(int, WPARAM, LPARAM);
static LRESULT CALLBACK MirandaWndProcHookFunction(int, WPARAM, LPARAM);
BOOL CheckMsgWnd(HANDLE, BOOL *);


BOOL isMetaContactsSubContact(HANDLE hMetaContact, HANDLE hContact)
{
	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hMetaContact, 0);
	if (szProto && !strcmp(szMetaProto, szProto)) { // Safety check
		int i = DBGetContactSettingDword(hContact, szMetaProto, "ContactNumber", -1);
		if (i >= 0 && hContact == (HANDLE)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hMetaContact, i))
			return TRUE;
	}
	return FALSE;
}


BOOL checkOpenWindow(HANDLE hContact)
{
	BOOL found, focus;

	if (bFlashIfMsgOpen && !bFlashIfMsgWinNotTop)
		return TRUE;

	found = CheckMsgWnd(hContact, &focus);
	if (!found && szMetaProto && bMetaProtoEnabled) {
		HANDLE hMetaContact = (HANDLE)DBGetContactSettingDword(hContact, szMetaProto, "Handle", 0);
		if (hMetaContact && isMetaContactsSubContact(hMetaContact, hContact))
			found = CheckMsgWnd(hMetaContact, &focus);
	}
	if (!found)
		return TRUE;

	if (bFlashIfMsgOpen && !focus)
		return TRUE;

	return FALSE;
}


BOOL IsSaverOnNT4()
{
	HDESK hd = OpenDesktop(L"screen-saver", 0, FALSE, MAXIMUM_ALLOWED);

	if(hd == NULL)
		return GetLastError()==ERROR_ACCESS_DENIED;

	CloseDesktop(hd);
	return TRUE;
}


BOOL isScreenSaverRunning()
{
	BOOL screenSaverIsRunning=FALSE;

	if (bWindowsNT && dWinVer < 5) return IsSaverOnNT4();

	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &screenSaverIsRunning, FALSE);
	return screenSaverIsRunning;
}


/* this function is from the original idle module */
BOOL isWorkstationLocked()
{
	HDESK hd;
	char buf[MAX_PATH];

	if (!bWindowsNT) return FALSE;

	hd = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED); /* if it fails then the workstation is prolly locked anyway */
	if (hd == NULL) return TRUE;
	GetUserObjectInformation(hd, UOI_NAME, buf, sizeof(buf), NULL); /* if we got it (hmm,) get a name */
	CloseDesktop(hd);
	return strcmp(buf, "Winlogon")==0;
}


BOOL isFullScreen()
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);

	HWND hWnd = 0;
	while (hWnd = FindWindowEx(NULL, hWnd, NULL, NULL)) {
		RECT WindowRect;

		if (!(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
			continue;

		GetWindowRect(hWnd, &WindowRect);
		if ((w != (WindowRect.right - WindowRect.left)) || (h != (WindowRect.bottom - WindowRect.top)))
			continue;

		return TRUE;
	}

	return FALSE;
}


BOOL checkNotifyOptions()
{
	BOOL fullScreenMode, screenSaverIsRunning, workstationIsLocked, processesRunning;

	screenSaverIsRunning = isScreenSaverRunning();
	if (screenSaverIsRunning && bScreenSaverRunning)
			return TRUE;

	workstationIsLocked = isWorkstationLocked();
	if (workstationIsLocked && bWorkstationLocked)
			return TRUE;

	fullScreenMode = isFullScreen() && !screenSaverIsRunning;
	if (fullScreenMode && bFullScreenMode)
			return TRUE;

	processesRunning = areThereProcessesRunning();
	if (processesRunning && bProcessesAreRunning)
			return TRUE;

	return (!fullScreenMode && !screenSaverIsRunning && !workstationIsLocked && !processesRunning && bWorkstationActive);
}


BOOL isStatusEnabled(int status)
{
	switch (status) {
		case ID_STATUS_OFFLINE:		return wStatusMap & MAP_OFFLINE;
		case ID_STATUS_ONLINE:		return wStatusMap & MAP_ONLINE;
		case ID_STATUS_AWAY:		return wStatusMap & MAP_AWAY;
		case ID_STATUS_NA:			return wStatusMap & MAP_NA;
		case ID_STATUS_OCCUPIED:	return wStatusMap & MAP_OCCUPIED;
		case ID_STATUS_DND:			return wStatusMap & MAP_DND;
		case ID_STATUS_FREECHAT:	return wStatusMap & MAP_FREECHAT;
		case ID_STATUS_INVISIBLE:	return wStatusMap & MAP_INVISIBLE;
		case ID_STATUS_ONTHEPHONE:	return wStatusMap & MAP_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH:	return wStatusMap & MAP_OUTTOLUNCH;
		default:					return FALSE;
	}
}


BOOL checkGlobalStatus()
{
	return isStatusEnabled(CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
}


BOOL checkGlobalXstatus()
{
	ICQ_CUSTOM_STATUS xstatus={0};
	unsigned int i, protosSupporting; int status=0;

	for(i=0, protosSupporting=0; i < ProtoList.protoCount; i++) {
		if (!ProtoList.protoInfo[i].enabled || !ProtoList.protoInfo[i].xstatus.count) continue;

		protosSupporting++;
		// Retrieve xstatus for protocol
		xstatus.cbSize = sizeof(ICQ_CUSTOM_STATUS);
		xstatus.flags = CSSF_MASK_STATUS;
		xstatus.status = &status;
		CallProtoService(ProtoList.protoInfo[i].szProto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);

		if (ProtoList.protoInfo[i].xstatus.enabled[status]) return TRUE;
	}

	if (!protosSupporting)
		return TRUE;
	else
		return FALSE;
}


DBEVENTINFO createMsgEventInfo(HANDLE hContact)
{
	DBEVENTINFO einfo = {0};

	einfo.cbSize = sizeof(einfo);
	einfo.eventType = EVENTTYPE_MESSAGE;
	einfo.szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	return einfo;
}


DBEVENTINFO readEventInfo(HANDLE hDbEvent, HANDLE hContact)
{
	DBEVENTINFO einfo = {0};

	if (hDbEvent == NCONVERS_BLINKID) // we need to handle nconvers' blink event
		return createMsgEventInfo(hContact);

	einfo.cbSize = sizeof(einfo);
	einfo.cbBlob = 0;
	einfo.pBlob = NULL;
	CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&einfo);

	return einfo;
}


BOOL checkIgnore(HANDLE hContact, WORD eventType)
{
	return !IsIgnored(hContact, eventType);
}


BOOL checkProtocol(char *szProto)
{
	unsigned int i;

	if (!szProto)
		return FALSE;

	for(i=0; i < ProtoList.protoCount; i++)
		if (ProtoList.protoInfo[i].szProto && !strcmp(ProtoList.protoInfo[i].szProto, szProto))
			return ProtoList.protoInfo[i].enabled;

	return FALSE;
}


BOOL metaCheckProtocol(char *szProto, HANDLE hContact, WORD eventType)
{
	HANDLE hSubContact=NULL;

	if (szMetaProto && bMetaProtoEnabled && szProto && !strcmp(szMetaProto, szProto))
		if (hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0))
			szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact, 0);

	return checkProtocol(szProto) && checkIgnore(hSubContact?hSubContact:hContact, eventType);
}


BOOL checkUnopenEvents()
{
	int nIndex;
	CLISTEVENT *pCLEvent;

	if (nExternCount && bFlashOnOther)
		return TRUE;

	for (nIndex = 0; pCLEvent = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, -1, nIndex); nIndex++) {
		DBEVENTINFO einfo = readEventInfo(pCLEvent->hDbEvent, pCLEvent->hContact);

		if ((einfo.eventType == EVENTTYPE_MESSAGE && bFlashOnMsg)  ||
		    (einfo.eventType == EVENTTYPE_URL     && bFlashOnURL)  ||
		    (einfo.eventType == EVENTTYPE_FILE    && bFlashOnFile) ||
		    (einfo.eventType != EVENTTYPE_MESSAGE && einfo.eventType != EVENTTYPE_URL && einfo.eventType != EVENTTYPE_FILE && bFlashOnOther))

			if (metaCheckProtocol(einfo.szModule, pCLEvent->hContact, einfo.eventType))
				return TRUE;
	}

	return FALSE;
}


static void FlashThreadFunction()
{
	BOOL bEvent = FALSE;
	DWORD dwEventStarted, dwFlashStarted;
	BYTE data, unchangedLeds;

	while (TRUE) {
		unchangedLeds = (BYTE)(LedState(VK_PAUSE) * !bFlashLed[2] + ((LedState(VK_NUMLOCK) * !bFlashLed[0])<<1) + ((LedState(VK_CAPITAL) * !bFlashLed[1])<<2));
		GetAsyncKeyState(VK_PAUSE); // empty Pause/Break's keystroke buffer

		// Start flashing
		while(bEvent && bFlashingEnabled)
		{
			// Let's give the user the opportunity of finishing flashing manually :)
			if (GetAsyncKeyState(VK_PAUSE) & 1)
				break;

			if ((bFlashUntil & UNTIL_NBLINKS) && GetTickCount() > (dwFlashStarted + wBlinksNumber * 1000))
				break;
			if (bFlashUntil & UNTIL_REATTENDED) {
				if (bMirandaOrWindows == ACTIVE_WINDOWS && MyGetLastInputInfo && !bEmulateKeypresses) {
					LASTINPUTINFO lii;
					ZeroMemory(&lii, sizeof(lii));
					lii.cbSize = sizeof(lii);
					MyGetLastInputInfo(&lii);
					dwLastInput = lii.dwTime;
				}
				if (dwLastInput > dwEventStarted)
					break;
			}
			if ((bFlashUntil & UNTIL_EVENTSOPEN) && !checkUnopenEvents())
				break;
			if ((bFlashUntil & UNTIL_CONDITIONS) && (!checkNotifyOptions() || !checkGlobalStatus() || !checkGlobalXstatus()))
				break;

			data = getBlinkingLeds();
			ToggleKeyboardLights((BYTE)(data|unchangedLeds));

			// Wait for exit event
			if (WaitForSingleObject(hExitEvent, nWaitDelay) == WAIT_OBJECT_0)
				return;
		}
		RestoreLEDState();

		setFlashingSequence();
		bReminderDisabled = FALSE;

		// Wait for new event
		{
			DWORD dwEvent;
			HANDLE Objects[2];
			Objects[0] = hFlashEvent;
			Objects[1] = hExitEvent;
			dwEvent = WaitForMultipleObjects(2, Objects, FALSE, INFINITE);
			if ((dwEvent - WAIT_OBJECT_0) == 1)
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


BOOL checkMsgTimestamp(HANDLE hEventCurrent, DWORD timestampCurrent)
{
	HANDLE hEvent;

	if (!bFlashIfMsgOlder)
		return TRUE;

	for (hEvent=(HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hEventCurrent, 0); hEvent; hEvent=(HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hEvent, 0)) {
		DBEVENTINFO einfo = {0};

		einfo.cbSize = sizeof(einfo);
		einfo.cbBlob = 0;
		einfo.pBlob = NULL;
		CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&einfo);
		if ((einfo.timestamp + wSecondsOlder) <= timestampCurrent)
			return TRUE;
		if (einfo.eventType == EVENTTYPE_MESSAGE)
			return FALSE;
	}

	return TRUE;
}


BOOL contactCheckProtocol(char *szProto, HANDLE hContact, WORD eventType)
{
	if (szMetaProto && bMetaProtoEnabled && hContact) {
		HANDLE hMetaContact = (HANDLE)DBGetContactSettingDword(hContact, szMetaProto, "Handle", 0);
		if (hMetaContact && isMetaContactsSubContact(hMetaContact, hContact))
			return FALSE;
	}

	return (metaCheckProtocol(szProto, hContact, eventType));
}


BOOL checkStatus(char *szProto)
{
	if (!szProto)
		return checkGlobalStatus();

	return isStatusEnabled(CallProtoService(szProto, PS_GETSTATUS, 0, 0));
}


BOOL checkXstatus(char *szProto)
{
	unsigned int i; int status=0;
	ICQ_CUSTOM_STATUS xstatus={0};

	if (!szProto)
		return checkGlobalXstatus();

	for(i=0; i < ProtoList.protoCount; i++)
		if (ProtoList.protoInfo[i].szProto && !strcmp(ProtoList.protoInfo[i].szProto, szProto)) {
			if (!ProtoList.protoInfo[i].xstatus.count) return TRUE;

			// Retrieve xstatus for protocol
			xstatus.cbSize = sizeof(ICQ_CUSTOM_STATUS);
			xstatus.flags = CSSF_MASK_STATUS;
			xstatus.status = &status;
			CallProtoService(ProtoList.protoInfo[i].szProto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);

			return ProtoList.protoInfo[i].xstatus.enabled[status];
		}

	return TRUE;
}


// 'Pings' the FlashThread to keep the LEDs flashing.
static int PluginMessageEventHook(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO einfo = {0};
	HANDLE hContact = (HANDLE)wParam;
	HANDLE hEvent = (HANDLE)lParam;

	//get DBEVENTINFO without pBlob
	einfo.cbSize = sizeof(einfo);
	einfo.cbBlob = 0;
	einfo.pBlob = NULL;
	CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&einfo);

	if (!(einfo.flags & DBEF_SENT))
		if ((einfo.eventType == EVENTTYPE_MESSAGE && bFlashOnMsg && checkOpenWindow(hContact) && checkMsgTimestamp(hEvent, einfo.timestamp)) ||
		    (einfo.eventType == EVENTTYPE_URL     && bFlashOnURL)  ||
		    (einfo.eventType == EVENTTYPE_FILE    && bFlashOnFile) ||
		    (einfo.eventType != EVENTTYPE_MESSAGE && einfo.eventType != EVENTTYPE_URL && einfo.eventType != EVENTTYPE_FILE && bFlashOnOther)) {

			if (contactCheckProtocol(einfo.szModule, hContact, einfo.eventType) && checkNotifyOptions() && checkStatus(einfo.szModule) && checkXstatus(einfo.szModule))

				SetEvent(hFlashEvent);
		}

	return 0;
}


// **
// ** Checks for pending events. If it finds any, it pings the FlashThread to keep the LEDs flashing.
// **

static VOID CALLBACK ReminderTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	int nIndex;
	CLISTEVENT *pCLEvent;

	if (!bReminderDisabled && nExternCount && bFlashOnOther) {
		SetEvent(hFlashEvent);
		return;
	}

	for (nIndex = 0; !bReminderDisabled && (pCLEvent = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, -1, nIndex)); nIndex++) {
		DBEVENTINFO einfo = readEventInfo(pCLEvent->hDbEvent, pCLEvent->hContact);

		if ((einfo.eventType == EVENTTYPE_MESSAGE && bFlashOnMsg)  ||
		    (einfo.eventType == EVENTTYPE_URL     && bFlashOnURL)  ||
		    (einfo.eventType == EVENTTYPE_FILE    && bFlashOnFile) ||
		    (einfo.eventType != EVENTTYPE_MESSAGE && einfo.eventType != EVENTTYPE_URL && einfo.eventType != EVENTTYPE_FILE && bFlashOnOther))

			if (metaCheckProtocol(einfo.szModule, pCLEvent->hContact, einfo.eventType) && checkNotifyOptions() && checkStatus(einfo.szModule) && checkXstatus(einfo.szModule)) {

				SetEvent(hFlashEvent);
				return;
			}
	}

}


// Support for third-party plugins and mBot's scripts
static INT_PTR EnableService(WPARAM wParam, LPARAM lParam)
{
	bFlashingEnabled = TRUE;
	return 0;
}

static INT_PTR DisableService(WPARAM wParam, LPARAM lParam)
{
	bFlashingEnabled = FALSE;
	return 0;
}

static INT_PTR StartBlinkService(WPARAM wParam, LPARAM lParam)
{
	nExternCount += (unsigned int)wParam;
	if (bFlashOnOther && checkNotifyOptions() && checkGlobalStatus() && checkGlobalXstatus()) {
		if (lParam)
			useExternSequence((TCHAR *)lParam);
		SetEvent(hFlashEvent);
	}

	return 0;
}

static INT_PTR EventsWereOpenedService(WPARAM wParam, LPARAM lParam)
{
	if ((unsigned int)wParam > nExternCount)
		nExternCount = 0;
	else
		nExternCount -= (unsigned int)wParam;

	return 0;
}


static INT_PTR IsFlashingActiveService(WPARAM wParam, LPARAM lParam)
{
	if (!bReminderDisabled)
		return 0;

	return (int)getCurrentSequenceString();
}


INT_PTR NormalizeSequenceService(WPARAM wParam, LPARAM lParam)
{
	TCHAR strAux[MAX_PATH+1], *strIn = (TCHAR *)lParam;

	_snwprintf(strAux, MAX_PATH, _T("%s"), strIn);
	_snwprintf(strIn, MAX_PATH, _T("%s"), normalizeCustomString(strAux));

	return (int)strIn;
}


// Support for Trigger plugin
static void ForceEventsWereOpenedThread(void *eventMaxSeconds)
{
	Sleep(((WORD)eventMaxSeconds) * 1000);
	CallService(MS_KBDNOTIFY_EVENTSOPENED, 1, 0);
}


void StartBlinkAction(char *flashSequence, WORD eventMaxSeconds)
{
	DWORD threadID = 0;

	if (eventMaxSeconds)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ForceEventsWereOpenedThread, (void *)eventMaxSeconds, 0, &threadID);

	CallService(MS_KBDNOTIFY_STARTBLINK, 1, (LPARAM)flashSequence);
}


void createProcessList(void)
{
	DBVARIANT dbv;
	unsigned int i, count;

	count = (unsigned int)DBGetContactSettingWord(NULL, KEYBDMODULE, "processcount", 0);

	ProcessList.count = 0;
	ProcessList.szFileName = (TCHAR **)malloc(count * sizeof(TCHAR *));
	if (ProcessList.szFileName) {
		for(i=0; i < count; i++)
			if (DBGetContactSetting(NULL, KEYBDMODULE, fmtDBSettingName("process%d", i), &dbv))
				ProcessList.szFileName[i] = NULL;
			else {
				ProcessList.szFileName[i] = (TCHAR *)malloc(wcslen(dbv.ptszVal) + 1);
				if (ProcessList.szFileName[i])
					wcscpy(ProcessList.szFileName[i], dbv.ptszVal);
				DBFreeVariant(&dbv);
			}
		ProcessList.count = count;
	}

}


void destroyProcessList(void)
{
	unsigned int i, count;

	count = ProcessList.count;

	ProcessList.count = 0;
	for(i=0; i < count; i++)
		if (ProcessList.szFileName[i])
			free(ProcessList.szFileName[i]);

	if (ProcessList.szFileName)
		free(ProcessList.szFileName);
	ProcessList.szFileName = NULL;
}


void LoadSettings(void)
{
	unsigned int i;

	bFlashOnMsg = DBGetContactSettingByte(NULL, KEYBDMODULE, "onmsg", DEF_SETTING_ONMSG);
	bFlashOnURL = DBGetContactSettingByte(NULL, KEYBDMODULE, "onurl", DEF_SETTING_ONURL);
	bFlashOnFile = DBGetContactSettingByte(NULL, KEYBDMODULE, "onfile", DEF_SETTING_ONFILE);
	bFlashOnOther = DBGetContactSettingByte(NULL, KEYBDMODULE, "onother", DEF_SETTING_OTHER);
	bFullScreenMode = DBGetContactSettingByte(NULL, KEYBDMODULE, "fscreenmode", DEF_SETTING_FSCREEN);
	bScreenSaverRunning = DBGetContactSettingByte(NULL, KEYBDMODULE, "ssaverrunning", DEF_SETTING_SSAVER);
	bWorkstationLocked = (bWindowsNT ? DBGetContactSettingByte(NULL, KEYBDMODULE, "wstationlocked", DEF_SETTING_LOCKED):0);
	bProcessesAreRunning = DBGetContactSettingByte(NULL, KEYBDMODULE, "procsrunning", DEF_SETTING_PROCS);
	bWorkstationActive = DBGetContactSettingByte(NULL, KEYBDMODULE, "wstationactive", DEF_SETTING_ACTIVE);
	bFlashIfMsgOpen = DBGetContactSettingByte(NULL, KEYBDMODULE, "ifmsgopen", DEF_SETTING_IFMSGOPEN);
	bFlashIfMsgWinNotTop = DBGetContactSettingByte(NULL, KEYBDMODULE, "ifmsgnottop", DEF_SETTING_IFMSGNOTTOP);
	bFlashIfMsgOlder = DBGetContactSettingByte(NULL, KEYBDMODULE, "ifmsgolder", DEF_SETTING_IFMSGOLDER);
	wSecondsOlder = DBGetContactSettingWord(NULL, KEYBDMODULE, "secsolder", DEF_SETTING_SECSOLDER);
	bFlashUntil = DBGetContactSettingByte(NULL, KEYBDMODULE, "funtil", DEF_SETTING_FLASHUNTIL);
	wBlinksNumber = DBGetContactSettingWord(NULL, KEYBDMODULE, "nblinks", DEF_SETTING_NBLINKS);
	bMirandaOrWindows = DBGetContactSettingByte(NULL, KEYBDMODULE, "mirorwin", DEF_SETTING_MIRORWIN);
	wStatusMap = DBGetContactSettingWord(NULL, KEYBDMODULE, "status", DEF_SETTING_STATUS);
	wReminderCheck = DBGetContactSettingWord(NULL, KEYBDMODULE, "remcheck", DEF_SETTING_CHECKTIME);
	bFlashLed[0] = !!DBGetContactSettingByte(NULL, KEYBDMODULE, "fnum", DEF_SETTING_FLASHNUM);
	bFlashLed[1] = !!DBGetContactSettingByte(NULL, KEYBDMODULE, "fcaps", DEF_SETTING_FLASHCAPS);
	bFlashLed[2] = !!DBGetContactSettingByte(NULL, KEYBDMODULE, "fscroll", DEF_SETTING_FLASHSCROLL);
	bFlashEffect = DBGetContactSettingByte(NULL, KEYBDMODULE, "feffect", DEF_SETTING_FLASHEFFECT);
	bSequenceOrder = DBGetContactSettingByte(NULL, KEYBDMODULE, "order", DEF_SETTING_SEQORDER);
	wCustomTheme = DBGetContactSettingWord(NULL, KEYBDMODULE, "custom", DEF_SETTING_CUSTOMTHEME);
	bTrillianLedsMsg = DBGetContactSettingByte(NULL, KEYBDMODULE, "ledsmsg", DEF_SETTING_LEDSMSG);
	bTrillianLedsURL = DBGetContactSettingByte(NULL, KEYBDMODULE, "ledsurl", DEF_SETTING_LEDSURL);
	bTrillianLedsFile = DBGetContactSettingByte(NULL, KEYBDMODULE, "ledsfile", DEF_SETTING_LEDSFILE);
	bTrillianLedsOther = DBGetContactSettingByte(NULL, KEYBDMODULE, "ledsother", DEF_SETTING_LEDSOTHER);
	wStartDelay = DBGetContactSettingWord(NULL, KEYBDMODULE, "sdelay", DEF_SETTING_STARTDELAY);
	bFlashSpeed = DBGetContactSettingByte(NULL, KEYBDMODULE, "speed", DEF_SETTING_FLASHSPEED);
	switch (bFlashSpeed) {
		case 0:	 nWaitDelay = 1500; break;
		case 1:  nWaitDelay = 0750; break;
		case 2:  nWaitDelay = 0250; break;
		case 3:  nWaitDelay = 0150; break;
		case 4:  nWaitDelay = 0100; break;
		default: nWaitDelay = 0050; break;
	}
	setFlashingSequence();
	bEmulateKeypresses = DBGetContactSettingByte(NULL, KEYBDMODULE, "keypresses", DEF_SETTING_KEYPRESSES);
	bOverride = DBGetContactSettingByte(NULL, KEYBDMODULE, "override", DEF_SETTING_OVERRIDE);
	// Create hidden settings (for test button) if needed
	if (DBGetContactSettingByte(NULL, KEYBDMODULE, "testnum", -1) == -1)
		DBWriteContactSettingByte(NULL, KEYBDMODULE, "testnum", DEF_SETTING_TESTNUM);
	if (DBGetContactSettingByte(NULL, KEYBDMODULE, "testsecs", -1) == -1)
		DBWriteContactSettingByte(NULL, KEYBDMODULE, "testsecs", DEF_SETTING_TESTSECS);
	for(i=0; i < ProtoList.protoCount; i++)
		if (ProtoList.protoInfo[i].visible) {
			unsigned int j;
			ProtoList.protoInfo[i].enabled = DBGetContactSettingByte(NULL, KEYBDMODULE, ProtoList.protoInfo[i].szProto, DEF_SETTING_PROTOCOL);
			for(j=0; j < ProtoList.protoInfo[i].xstatus.count; j++)
				ProtoList.protoInfo[i].xstatus.enabled[j] = DBGetContactSettingByte(NULL, KEYBDMODULE, fmtDBSettingName("%sxstatus%d", ProtoList.protoInfo[i].szProto, j), DEF_SETTING_XSTATUS);
		}

	if (szMetaProto)
		bMetaProtoEnabled = DBGetContactSettingByte(NULL, szMetaProto, "Enabled", 1);

	destroyProcessList();
	createProcessList();
	UnhookWindowsHooks();
	HookWindowsHooks();
}


void GetWindowsVersion(void)
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi))) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *)&osvi))
			osvi.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
	}
	bWindowsNT = osvi.dwPlatformId==VER_PLATFORM_WIN32_NT;
	dWinVer = osvi.dwMajorVersion + osvi.dwMinorVersion / 10.0;
}


void updateXstatusProto(PROTOCOL_INFO *protoInfo)
{
	unsigned int i;
	char szServiceName[MAXMODULELABELLENGTH];
	ICQ_CUSTOM_STATUS xstatus={0};

	mir_snprintf(szServiceName, sizeof(szServiceName), "%s%s", protoInfo->szProto, PS_ICQ_GETCUSTOMSTATUSEX);
	if (!ServiceExists(szServiceName)) return;

	// Retrieve xstatus.count
	xstatus.cbSize = sizeof(ICQ_CUSTOM_STATUS);
	xstatus.flags = CSSF_STATUSES_COUNT;
	xstatus.wParam = &(protoInfo->xstatus.count);
	CallProtoService(protoInfo->szProto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);
	(protoInfo->xstatus.count)++;	// Don't forget about xstatus=0 (None)

	// Alloc and initiailize xstatus.enabled array
	protoInfo->xstatus.enabled = (BOOL *)malloc(protoInfo->xstatus.count * sizeof(BOOL));
	if (!protoInfo->xstatus.enabled)
		protoInfo->xstatus.count = 0;
	else
		for(i=0; i < protoInfo->xstatus.count; i++)
			protoInfo->xstatus.enabled[i] = FALSE;

}


void createProtocolList(void)
{
	unsigned int i;
	PROTOCOLDESCRIPTOR **proto;

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		szMetaProto = (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&ProtoList.protoCount, (LPARAM)&proto);
	ProtoList.protoInfo = (PROTOCOL_INFO *)malloc(ProtoList.protoCount * sizeof(PROTOCOL_INFO));
	if (!ProtoList.protoInfo)
		ProtoList.protoCount = 0;
	else
		for(i=0; i < ProtoList.protoCount; i++) {
			ProtoList.protoInfo[i].xstatus.count = 0;
			ProtoList.protoInfo[i].xstatus.enabled = NULL;
			ProtoList.protoInfo[i].szProto = (char *)malloc(strlen(proto[i]->szName) + 1);
			if (!ProtoList.protoInfo[i].szProto) {
				ProtoList.protoInfo[i].enabled = FALSE;
				ProtoList.protoInfo[i].visible = FALSE;
			} else {
				strcpy(ProtoList.protoInfo[i].szProto, proto[i]->szName);
				ProtoList.protoInfo[i].enabled = FALSE;
				if (proto[i]->type != PROTOTYPE_PROTOCOL)
					ProtoList.protoInfo[i].visible = FALSE;
				else
					if (szMetaProto && !strcmp(proto[i]->szName, szMetaProto))
						ProtoList.protoInfo[i].visible = FALSE;
					else {
						ProtoList.protoInfo[i].visible = TRUE;
						updateXstatusProto(&(ProtoList.protoInfo[i]));
					}
			}
		}

}


// We use the profile name to create the first part of each event name
// We do so to avoid problems between different instances of the plugin concurrently running
void createEventPrefix(TCHAR *prefixName, size_t maxLen)
{
	size_t len;
	TCHAR profileName[MAX_PATH+1], *str;

	getAbsoluteProfileName(profileName, MAX_PATH);

	while (str = wcschr(profileName, _T('\\')))
		*str = _T('/');
	if ((len = wcslen(profileName)) <= maxLen)
		wcscpy(prefixName, profileName);
	else {
		str = profileName + len - maxLen / 2;
		_snwprintf(prefixName, maxLen / 2, L"%s", profileName);
		wcscat(prefixName, str);
	}
}


// **
// ** Everything below is just Miranda init/uninit stuff
// **


static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	TCHAR eventPrefix[MAX_PATH+1], eventName[MAX_PATH+1];

	LoadProcsLibrary();
	if (bWindowsNT && dWinVer >= 5)
		MyGetLastInputInfo = (BOOL (WINAPI *)(PLASTINPUTINFO)) GetProcAddress(GetModuleHandle(L"user32"), "GetLastInputInfo");
	else
		MyGetLastInputInfo = NULL;

	createProtocolList();
	LoadSettings();

	// Create some synchronisation objects
	createEventPrefix(eventPrefix, MAX_PATH - 11);
	_snwprintf(eventName, sizeof(eventName), _T("%s/FlashEvent"), eventPrefix);
	hFlashEvent = CreateEvent(NULL, FALSE, FALSE, eventName);
	_snwprintf(eventName, sizeof(eventName), _T("%s/ExitEvent"), eventPrefix);
	hExitEvent = CreateEvent(NULL, FALSE, FALSE, eventName);

	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FlashThreadFunction, NULL, 0, &IDThread);

	hMsgEventHook = HookEvent(ME_DB_EVENT_ADDED, PluginMessageEventHook);
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, InitializeOptions);
	hEnableService = CreateServiceFunction(MS_KBDNOTIFY_ENABLE, EnableService);
	hDisableService = CreateServiceFunction(MS_KBDNOTIFY_DISABLE, DisableService);
	hStartBlinkService = CreateServiceFunction(MS_KBDNOTIFY_STARTBLINK, StartBlinkService);
	hEventsOpenedService = CreateServiceFunction(MS_KBDNOTIFY_EVENTSOPENED, EventsWereOpenedService);
	hFlashingEventService = CreateServiceFunction(MS_KBDNOTIFY_FLASHINGACTIVE, IsFlashingActiveService);
	hNormalizeSequenceService = CreateServiceFunction(MS_KBDNOTIFY_NORMALSEQUENCE, NormalizeSequenceService);

	RegisterAction();
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)KEYBDMODULE, 0);
	if (ServiceExists(MS_UPDATE_REGISTERFL))
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)2071, (LPARAM)&pluginInfo);

	return 0;
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

	hInst = hinstDLL;

	return TRUE;

}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);

	GetWindowsVersion();
	OpenKeyboardDevice();
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}



void destroyProtocolList(void)
{
	unsigned int i;

	for(i=0; i < ProtoList.protoCount; i++) {
		if (ProtoList.protoInfo[i].szProto)
			free(ProtoList.protoInfo[i].szProto);
		if (ProtoList.protoInfo[i].xstatus.enabled)
			free(ProtoList.protoInfo[i].xstatus.enabled);
	}

	ProtoList.protoCount = 0;
	if (ProtoList.protoInfo)
		free(ProtoList.protoInfo);
}


extern "C" __declspec(dllexport) int Unload(void)
{

	UnhookWindowsHooks();
	DeInitAction();
	if (hModulesLoaded)
		UnhookEvent(hModulesLoaded);
	if (hMsgEventHook)
		UnhookEvent(hMsgEventHook);
	if (hOptionsInitialize)
		UnhookEvent(hOptionsInitialize);
	if (hEnableService)
		DestroyServiceFunction(hEnableService);
	if (hDisableService)
		DestroyServiceFunction(hDisableService);
	if (hStartBlinkService)
		DestroyServiceFunction(hStartBlinkService);
	if (hEventsOpenedService)
		DestroyServiceFunction(hEventsOpenedService);
	if (hFlashingEventService)
		DestroyServiceFunction(hFlashingEventService);
	if (hNormalizeSequenceService)
		DestroyServiceFunction(hNormalizeSequenceService);

	// Wait for thread to exit
	SetEvent(hExitEvent);
	WaitForSingleObject(hThread, INFINITE);

	RestoreLEDState();
	CloseKeyboardDevice();

	UnloadProcsLibrary();
	destroyProcessList();
	destroyProtocolList();

	return 0;
}


// ========================== Windows hooks ==========================
int HookWindowsHooks()
{
	if (wReminderCheck)
		hReminderTimer = SetTimer(NULL,0, wReminderCheck * 60000, ReminderTimer);

	if (bFlashUntil & UNTIL_REATTENDED)
		switch (bMirandaOrWindows) {
			case ACTIVE_WINDOWS:
				if (!MyGetLastInputInfo || bEmulateKeypresses) {
					if (hMouseHook == NULL)
						hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookFunction, hInst, 0);
					if (hKeyBoardHook == NULL)
						hKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardHookFunction, hInst, 0);
				}
				break;
			case ACTIVE_MIRANDA:
				if (hMirandaMouseHook == NULL)
					hMirandaMouseHook = SetWindowsHookEx(WH_MOUSE, MirandaMouseHookFunction, NULL, GetCurrentThreadId());
				if (hMirandaKeyBoardHook == NULL)
					hMirandaKeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, MirandaKeyBoardHookFunction, NULL, GetCurrentThreadId());
				if (hMirandaWndProcHook == NULL)
					hMirandaWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, MirandaWndProcHookFunction, NULL, GetCurrentThreadId());
		}

	return 0;
}

int UnhookWindowsHooks()
{
	if (hReminderTimer)
		KillTimer(NULL, hReminderTimer);
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
	hMouseHook = hKeyBoardHook = hMirandaMouseHook = hMirandaKeyBoardHook = hMirandaWndProcHook = NULL;

	return 0;
}

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
			if (pt.x!=lastGlobalMousePos.x || pt.y!=lastGlobalMousePos.y) {
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
	static POINT lastMousePos = {0, 0};

	if (code >= 0) {
		/* Movement mouse messages are for some reason incoming in inactive/background window too, that is not input */
		DWORD pid;
		GetWindowThreadProcessId(GetForegroundWindow(), &pid);
		if(pid == GetCurrentProcessId()) {
			/* This should handle all mouse buttons ... */
			if ((wParam >= WM_NCLBUTTONDOWN && wParam <= WM_NCXBUTTONDBLCLK && wParam != 0x00AA) || (wParam >= WM_LBUTTONDOWN && wParam <= WM_XBUTTONDBLCLK))
 				dwLastInput = GetTickCount();
			/* ... and here it is either mouse move, hover, leave or something unexpected */
			else {
				PMOUSEHOOKSTRUCT mouseInfo = (PMOUSEHOOKSTRUCT)lParam;
				POINT pt = mouseInfo->pt;
				if (pt.x!=lastMousePos.x || pt.y!=lastMousePos.y) {
					lastMousePos = pt;
					dwLastInput = GetTickCount();
				}
			}
		}
	}

	return CallNextHookEx(hMirandaMouseHook, code, wParam, lParam);
}

static LRESULT CALLBACK MirandaKeyBoardHookFunction(int code, WPARAM wParam, LPARAM lParam) {

	if (code >= 0 && (!bEmulateKeypresses || (bEmulateKeypresses && wParam != VK_NUMLOCK && wParam != VK_CAPITAL && wParam != VK_SCROLL)))
		dwLastInput = GetTickCount();

	return CallNextHookEx(hMirandaKeyBoardHook, code, wParam, lParam);
}

static LRESULT CALLBACK MirandaWndProcHookFunction(int code, WPARAM wParam, LPARAM lParam) {

	if (code >= 0) {
		/* WM_ACTIVATEAPP with nonzero wParam means someone brought miranda to foreground, that equals to input */
		PCWPSTRUCT cwpInfo = (PCWPSTRUCT)lParam;
		if(cwpInfo->message == WM_ACTIVATEAPP && cwpInfo->wParam)
			dwLastInput = GetTickCount();
	}

 	return CallNextHookEx(hMirandaWndProcHook, code, wParam, lParam);
}


//===================== Check Window Message function =====================

// Took this snippet of code from "EventNotify" by micron-x, thx *g*
// and updated with NGEventNotify and pete's patch
// checks if the message-dialog window is already opened and returns:
//	TRUE  - Windows found
//	FALSE - No window found

HWND findMessageWindow(HANDLE hContact)
{
	HWND hwnd;
	TCHAR newtitle[256];
	char *szProto, *contactName, *szStatus;
	CONTACTINFO ci = {0};

	szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	contactName = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	szStatus = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, szProto==NULL?ID_STATUS_OFFLINE:DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);

	_snwprintf(newtitle, sizeof(newtitle), _T("%s (%s): %s"), contactName, szStatus, TranslateT("Message Received"));
	if(hwnd = FindWindow(NULL, newtitle))
		return hwnd;

	_snwprintf(newtitle, sizeof(newtitle), _T("%s %s"), contactName, szStatus);
	if(hwnd = FindWindow(NULL, newtitle))
		return hwnd;
	_snwprintf(newtitle, sizeof(newtitle), _T("%s (%s): %s"), contactName, szStatus, TranslateT("Message Session"));
	if(hwnd = FindWindow(NULL, newtitle))
		return hwnd;
	_snwprintf(newtitle, sizeof(newtitle), _T("%s (%s): %s"), contactName, szStatus, TranslateT("Message Session is typing..."));
	if(hwnd = FindWindow(NULL, newtitle))
		return hwnd;
	// search for the nconvers++ message window that uses the UIN
	ci.cbSize = sizeof(CONTACTINFO);
	ci.dwFlag = CNF_UNIQUEID;
	ci.hContact = hContact;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		switch(ci.type) {
			case CNFT_BYTE:
				_snwprintf(newtitle, sizeof(newtitle), _T("%s (%d) %s"), contactName, ci.bVal, szStatus);
				break;
			case CNFT_WORD:
				_snwprintf(newtitle, sizeof(newtitle), _T("%s (%d) %s"), contactName, ci.wVal, szStatus);
				break;
			case CNFT_DWORD:
				_snwprintf(newtitle, sizeof(newtitle), _T("%s (%d) %s"), contactName, ci.dVal, szStatus);
				break;
			case CNFT_ASCIIZ:
				_snwprintf(newtitle, sizeof(newtitle), _T("%s (%s) %s"), contactName, ci.pszVal, szStatus);
				break;
		}
		if(hwnd = FindWindow(NULL, newtitle))
			return hwnd;
	}

	return NULL;
}

BOOL CheckMsgWnd(HANDLE hContact, BOOL *focus)
{
	if (ServiceExists(MS_MSG_GETWINDOWDATA)) {	// use the new message API
		MessageWindowData mwd;
		MessageWindowInputData mwid;
		mwid.cbSize = sizeof(MessageWindowInputData);
		mwid.hContact = hContact;
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = hContact;
		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd) && mwd.hwndWindow) {
			*focus = mwd.uState & MSG_WINDOW_STATE_FOCUS;
			return TRUE;
		}
	} else {	// old way: find it by using the window class & title
		HWND hwnd;

		if(hwnd = findMessageWindow(hContact)) {
			*focus = hwnd==GetForegroundWindow();
			return TRUE;
		}
	}

	*focus = FALSE;
	return FALSE;
}


void countUnopenEvents(int *msgCount, int *fileCount, int *urlCount, int *otherCount)
{
	int nIndex;
	CLISTEVENT *pCLEvent;

	for (nIndex = 0; pCLEvent = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, -1, nIndex); nIndex++) {
		DBEVENTINFO einfo = readEventInfo(pCLEvent->hDbEvent, pCLEvent->hContact);

		if (metaCheckProtocol(einfo.szModule, pCLEvent->hContact, einfo.eventType))
			switch (einfo.eventType) {
				case EVENTTYPE_MESSAGE:
					if (bFlashOnMsg)
						(*msgCount)++;
					break;
				case EVENTTYPE_URL:
					if (bFlashOnURL)
						(*urlCount)++;
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
	if (bFlashOnOther)
		(*otherCount) += nExternCount;
}
