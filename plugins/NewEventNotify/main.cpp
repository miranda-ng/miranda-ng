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

int g_IsSrmmServiceAvail = 0;
int g_IsSrmmWindowAPI = 0;
extern PLUGIN_DATA* PopUpList[20];

//---------------------------
//---Some global variables for the plugin

HINSTANCE hInst;
PLUGIN_OPTIONS pluginOptions;
int hLangpack;
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"NewEventNotify",
	PLUGIN_MAKE_VERSION(0, VER_MAJOR, VER_MINOR, VER_BUILD),
	"Notifies you when you receive a message, url, file or any other event by displaying a popup. Uses the PopUp-Plugin by hrk. "
	"Original plugin was written by icebreaker, modified by Prezes and some "
	"bugfixes were made by vj, vlko, Nightwish, TheLeech and others. More fixes & updates by Joe @ Whale",
	"icebreaker, Prezes, vj, vlko, Joe @ Whale",
	"jokusoftware@miranda-im.org",
	"GNU GPL",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=3637",
	UNICODE_AWARE,
	{0x3503D584, 0x6234, 0x4BEF, {0xA5, 0x53, 0x6C, 0x1B, 0x9C, 0xD4, 0x71, 0xF2 } } // {3503D584-6234-4BEF-A553-6C1B9CD471F2}
};

//---------------------------
//---Hooks

//---Handles to my hooks, needed to unhook them again
HANDLE hHookedInit;
HANDLE hHookedOpt;
HANDLE hHookedNewEvent;
HANDLE hHookedDeletedEvent;

//---Called when a new event is added to the database
int HookedNewEvent(WPARAM wParam, LPARAM lParam)
//wParam: contact-handle
//lParam: dbevent-handle
{
	HANDLE hContact = (HANDLE)wParam;
	DBEVENTINFO dbe = {0};
	PLUGIN_DATA* pdata;
	DBEVENTTYPEDESCR* pei;

	//are popups currently enabled?
	if (pluginOptions.bDisable)
		return 0;

	//get DBEVENTINFO without pBlob
	dbe.cbSize = sizeof(dbe);
	CallService(MS_DB_EVENT_GET, (WPARAM)lParam, (LPARAM)&dbe);

	// Nightwish (no popups for RSS contacts at all...)
	if (pluginOptions.bNoRSS) 
	{						
		if (dbe.szModule != NULL) {
			if (!strncmp(dbe.szModule, "RSS", 3))
				return 0;
		}
	}

	//do not show popups for sub-contacts
	if (hContact && ServiceExists(MS_MC_GETMETACONTACT) && CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0))
		return 0;

	//is it an event info about online/offline status user
	if (dbe.eventType == 25368)
		return 0;

	//custom database event types
	if (ServiceExists(MS_DB_EVENT_GETTYPE))
	{
		pei = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbe.szModule, (LPARAM)dbe.eventType);
		if (pei && pei->cbSize >= DBEVENTTYPEDESCR_SIZE && pei->flags & DETF_NONOTIFY)
		// ignore events according to flags
			return 0;
    }

	//if event was allready read don't show it
	if (pluginOptions.bReadCheck && (dbe.flags & DBEF_READ))
		return 0;

	//is it an event sent by the user? -> don't show
	if (dbe.flags & DBEF_SENT)
	{
		if (pluginOptions.bHideSend && NumberPopupData(hContact, EVENTTYPE_MESSAGE) != -1)
		{ // JK, only message event, do not influence others
			pdata = PopUpList[NumberPopupData(hContact, EVENTTYPE_MESSAGE)];
			PopupAct(pdata->hWnd, MASK_DISMISS, pdata); // JK, only dismiss, i.e. do not kill event (e.g. file transfer)
		}		
		return 0; 
	}
	//which status do we have, are we allowed to post popups?
	//UNDER CONSTRUCTION!!!
	CallService(MS_CLIST_GETSTATUSMODE, 0, 0); /// TODO: JK: ????
	
	if (dbe.eventType == EVENTTYPE_MESSAGE && (pluginOptions.bMsgWindowCheck && hContact && CheckMsgWnd(hContact)))
		return 0;

	//is another popup for this contact already present? -> merge message popups if enabled
	if (dbe.eventType == EVENTTYPE_MESSAGE && (pluginOptions.bMergePopup && NumberPopupData(hContact, EVENTTYPE_MESSAGE) != -1))
	{ // JK, only merge with message events, do not mess with others
		PopupUpdate(hContact, (HANDLE)lParam);
	}
	else
	{	//now finally show a popup
		PopupShow(&pluginOptions, hContact, (HANDLE)lParam, (UINT)dbe.eventType);
	}
	return 0;
}

//---Called when all the modules are loaded
int HookedInit(WPARAM wParam, LPARAM lParam)
{
	hHookedNewEvent = HookEvent(ME_DB_EVENT_ADDED, HookedNewEvent);
	// Plugin sweeper support
	if (ServiceExists("PluginSweeper/Add"))
    CallService("PluginSweeper/Add", (WPARAM)MODULE, (LPARAM)MODULE);

	if (ServiceExists(MS_MSG_GETWINDOWDATA))
		g_IsSrmmWindowAPI = 1;
	else
		g_IsSrmmWindowAPI = 0;

	if (ServiceExists(MS_MSG_MOD_MESSAGEDIALOGOPENED))
		g_IsSrmmServiceAvail = 1;
	else
		g_IsSrmmServiceAvail = 0;

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

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_EVENTNOTIFY, MIID_LAST};

extern "C" __declspec(dllexport) int Load(void)
{
	hHookedInit = HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
	hHookedOpt = HookEvent(ME_OPT_INITIALISE, HookedOptions);

	mir_getLP(&pluginInfo);

	OptionsInit(&pluginOptions);
	pluginOptions.hInst = hInst;

	if (pluginOptions.bMenuitem)
		MenuitemInit(!pluginOptions.bDisable);

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
	hInst=hinstDLL;
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

int CheckMsgWnd(HANDLE hContact)
{
	if (g_IsSrmmWindowAPI) {
		MessageWindowData mwd;
		MessageWindowInputData mwid;
		mwid.cbSize = sizeof(MessageWindowInputData);
		mwid.hContact = hContact;
		mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = hContact;
		if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM) &mwid, (LPARAM) &mwd)) {
			if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS)) return 1;
		}
	}
	if (g_IsSrmmServiceAvail) {   // use the service provided by tabSRMM
		if (CallService(MS_MSG_MOD_MESSAGEDIALOGOPENED, (WPARAM)hContact, 0))
			return 1;
		else 
			return 0;
	}
	else 
	{	// old way: find it by using the window class & title
		TCHAR newtitle[256];
		char *szProto;
		TCHAR *contactName, *szStatus;

		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		contactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
		szStatus = (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, szProto?DBGetContactSettingWord(hContact,szProto,"Status",ID_STATUS_OFFLINE):ID_STATUS_OFFLINE, GSMDF_TCHAR);

		// vj: This code was added by preeze and it does not work:
		// vlko: it maybe work with other plugins 
		mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s  (%s)"), contactName, szStatus);
		if(FindWindow(_T("TMsgWindow"), newtitle))
			return 2;

		mir_sntprintf(newtitle, SIZEOF(newtitle), _T("[%s  (%s)]"), contactName, szStatus);
		if(FindWindow(_T("TfrmContainer"), newtitle))
			return 1;

		// vj: I have restored this code from original plugin's source: (NewEventNotify 0.0.4)
		mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s (%s): %s"), contactName, szStatus, TranslateT("Message Session"));
		if(FindWindow(_T("#32770"), newtitle)) // JK, this works for old SRMMs (1.0.4.x) and for mine SRMMJ
			return 1;

		mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s (%s): %s"), contactName, szStatus, TranslateT("Message Received"));
		if(FindWindow(_T("#32770"), newtitle))
			return 2;

		return 0;
	}
}