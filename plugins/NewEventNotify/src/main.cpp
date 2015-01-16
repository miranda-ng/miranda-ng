/*
  Name: NewEventNotify - Plugin for Miranda IM
  File: main.c - Main DLL procedures
  Version: 0.2.2.2
  Description: Notifies you about some events
  Author: jokusoftware, <jokusoftware@miranda-im.org>
  Date: 18.07.02 13:59 / Update: 22.10.07 19:56
  Copyright: (C) 2002 Starzinger Michael

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

#include "neweventnotify.h"

int g_IsSrmmWindowAPI = 0;
extern PLUGIN_DATA* PopupList[20];

//---------------------------
//---Some global variables for the plugin

HINSTANCE hInst;
PLUGIN_OPTIONS pluginOptions;
int hLangpack;
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {3503D584-6234-4BEF-A553-6C1B9CD471F2}
	{0x3503D584, 0x6234, 0x4BEF, {0xA5, 0x53, 0x6C, 0x1B, 0x9C, 0xD4, 0x71, 0xF2}}
};

//---------------------------
//---Hooks

//---Handles to my hooks, needed to unhook them again
HANDLE hHookedInit;
HANDLE hHookedOpt;
HANDLE hHookedNewEvent;
HANDLE hHookedDeletedEvent;

//---Called when a new event is added to the database
//wParam: contact-handle
//lParam: dbevent-handle

int HookedNewEvent(WPARAM hContact, LPARAM hDbEvent)
{
	//are popups currently enabled?
	if (pluginOptions.bDisable)
		return 0;

	//get DBEVENTINFO without pBlob
	DBEVENTINFO dbe = { sizeof(dbe) };
	db_event_get(hDbEvent, &dbe);

	//do not show popups for sub-contacts
	if (hContact && db_mc_isSub(hContact))
		return 0;

	//custom database event types
	if (ServiceExists(MS_DB_EVENT_GETTYPE)) {
		DBEVENTTYPEDESCR *pei = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbe.szModule, (LPARAM)dbe.eventType);
		// ignore events according to flags
		if (pei && pei->flags & DETF_NONOTIFY)
			return 0;
    }

	//if event was allready read don't show it
	if (pluginOptions.bReadCheck && (dbe.flags & DBEF_READ))
		return 0;

	//is it an event sent by the user? -> don't show
	if (dbe.flags & DBEF_SENT) {
		// JK, only message event, do not influence others
		if (pluginOptions.bHideSend && NumberPopupData(hContact, EVENTTYPE_MESSAGE) != -1) {
			PLUGIN_DATA *pdata = PopupList[NumberPopupData(hContact, EVENTTYPE_MESSAGE)];
			PopupAct(pdata->hWnd, MASK_DISMISS, pdata); // JK, only dismiss, i.e. do not kill event (e.g. file transfer)
		}		
		return 0; 
	}
	// which status do we have, are we allowed to post popups?
	// UNDER CONSTRUCTION!!!
	CallService(MS_CLIST_GETSTATUSMODE, 0, 0); /// TODO: JK: ????
	if (dbe.eventType == EVENTTYPE_MESSAGE && (pluginOptions.bMsgWindowCheck && hContact && CheckMsgWnd(hContact)))
		return 0;

	//is another popup for this contact already present? -> merge message popups if enabled
	if (dbe.eventType == EVENTTYPE_MESSAGE && (pluginOptions.bMergePopup && NumberPopupData(hContact, EVENTTYPE_MESSAGE) != -1))
		PopupUpdate(hContact, hDbEvent);
	else
		PopupShow(&pluginOptions, hContact, hDbEvent, (UINT)dbe.eventType);

	return 0;
}

//---Called when all the modules are loaded
int HookedInit(WPARAM, LPARAM)
{
	hHookedNewEvent = HookEvent(ME_DB_EVENT_ADDED, HookedNewEvent);
	// Plugin sweeper support
	if (ServiceExists("PluginSweeper/Add"))
		CallService("PluginSweeper/Add", (WPARAM)MODULE, (LPARAM)MODULE);

	if (pluginOptions.bMenuitem)
		MenuitemInit(!pluginOptions.bDisable);

	g_IsSrmmWindowAPI = ServiceExists(MS_MSG_GETWINDOWDATA) != 0;
	return 0;
}

//---Called when an options dialog has to be created
int HookedOptions(WPARAM wParam, LPARAM lParam)
{
	OptionsAdd(hInst, wParam);
	return 0;
}

//---------------------------
//---Exported Functions

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	hHookedInit = HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
	hHookedOpt = HookEvent(ME_OPT_INITIALISE, HookedOptions);

	mir_getLP(&pluginInfo);

	OptionsInit(&pluginOptions);
	pluginOptions.hInst = hInst;

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hHookedNewEvent);
	UnhookEvent(hHookedOpt);
	UnhookEvent(hHookedInit);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

//-------------------------------------
//---Check Window Message function

// Took this snippet of code from "EventNotify" by micron-x, thx *g*
// checks if the message-dialog window is already opened
// return values:
//	0 - No window found
//	1 - Split-mode window found
//	2 - Single-mode window found

int CheckMsgWnd(MCONTACT hContact)
{
	if (g_IsSrmmWindowAPI) {
		MessageWindowData mwd;
		MessageWindowInputData mwid;
		mwid.cbSize = sizeof(MessageWindowInputData);
		mwid.hContact = hContact;
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = hContact;
		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM) &mwid, (LPARAM) &mwd))
			if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS))
				return 1;
	}

	return 0;
}
