/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

#include "commonheaders.h"
#include "dlg_handlers.h"
#include "hooked_events.h"

HANDLE hModulesLoaded;
HANDLE hOptionsInitialise;
HANDLE hPreShutdown;

UINT_PTR hCheckTimer = NULL;
UINT_PTR hReconnectTimer = NULL;
UINT_PTR hFirstCheckTimer = NULL;

#define HOST "http://eblis.tla.ro/projects"

#define EXCHANGE_VERSION_URL HOST "/miranda/Exchange/updater/Exchange.html"
#define EXCHANGE_UPDATE_URL HOST "/miranda/Exchange/updater/Exchange.zip"
#define EXCHANGE_VERSION_PREFIX "Exchange notifier plugin version "

int HookEvents()
{
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hOptionsInitialise = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	hPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnSystemPreShutdown);
	
	return 0;
}

int UnhookEvents()
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hOptionsInitialise);
	UnhookEvent(hPreShutdown);
	
	KillTimers();
	
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{

	UpdateTimers();
	CLISTMENUITEM cl = {0};
	cl.cbSize = sizeof(CLISTMENUITEM);
	cl.hIcon = hiMailIcon;
	cl.position = 10000000;
	cl.pszService = MS_EXCHANGE_CHECKEMAIL;
	cl.flags = CMIF_TCHAR;
	cl.ptszName = LPGENT("Check exchange mailbox");
	Menu_AddMainMenuItem (&cl);
	
	hEmailsDlg = NULL; //CreateDialog(hInstance, MAKEINTRESOURCE(IDD_EMAILS), NULL, DlgProcEmails); //create emails window
	FirstTimeCheck();	
//	CheckEmail();
	return 0;
}

//add the exchange options dialog to miranda
int OnOptionsInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXCHANGE);
	odp.ptszTitle = LPGENT("Exchange notify");
	odp.ptszGroup = LPGENT("Plugins");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}

int OnSystemPreShutdown(WPARAM, LPARAM)
{
	if (hEmailsDlg)
	{
		SendMessage(hEmailsDlg, WM_CLOSE, 0, 0); //close the window if it's opened
	}
	
	exchangeServer.Disconnect();
	
	return 0;
}

void FirstTimeCheck()
{
	hFirstCheckTimer = SetTimer(NULL, 0, 5 * 1000, OnFirstCheckTimer);
}

int UpdateTimers()
{
	KillTimers();
	int interval;
	interval = db_get_dw(NULL, ModuleName, "Interval", DEFAULT_INTERVAL);
	interval *= 1000; //go from miliseconds to seconds
	hCheckTimer = SetTimer(NULL, 0, interval, (TIMERPROC) OnCheckTimer);
	
	int bReconnect = db_get_b(NULL, ModuleName, "Reconnect", 0);
	if (bReconnect) //user wants to forcefully reconnect every x minutes
		{
			interval = db_get_dw(NULL, ModuleName, "ReconnectInterval", DEFAULT_RECONNECT_INTERVAL);
			interval *= 1000 * 60; //go from miliseconds to seconds to minutes
			hReconnectTimer = SetTimer(NULL, 0, interval, (TIMERPROC) OnReconnectTimer);
		}
	
	return 0;
}

int KillTimers()
{
	if (hCheckTimer)
		{
			KillTimer(NULL, hCheckTimer);
			hCheckTimer = NULL;
		}
	if (hReconnectTimer)
		{
			KillTimer(NULL, hReconnectTimer);
			hReconnectTimer = NULL;
		}
	return 0;
}

VOID CALLBACK OnCheckTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
	/*if (exchangeServer.IsConnected())
		{
			exchangeServer.Check();
		}
		else{
			exchangeServer.Connect();
		}*/
	int bCheck = db_get_b(NULL, ModuleName, "Check", 1);
	
	if (bCheck) //only check if we were told to
		{
			ThreadCheckEmail(FALSE);
		}
}

VOID CALLBACK OnReconnectTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{

	_popupUtil("Forcefully reconnecting to Exchange server ...");
	
	exchangeServer.Reconnect(); //reconnect
}

VOID CALLBACK OnFirstCheckTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, hFirstCheckTimer);
	OnCheckTimer(hWnd, msg, idEvent, dwTime);
	
	hFirstCheckTimer = NULL;
}