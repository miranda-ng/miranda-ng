/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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
#include "commonheaders.h"
#include <m_url.h>
#include "url.h"

HANDLE hUrlWindowList = NULL;
static HANDLE hEventContactSettingChange = NULL;
static HANDLE hContactDeleted = NULL;
static HGENMENU hSRUrlMenuItem = NULL;

INT_PTR CALLBACK DlgProcUrlSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcUrlRecv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static INT_PTR ReadUrlCommand(WPARAM, LPARAM lParam)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_URLRECV), NULL, DlgProcUrlRecv, lParam);
	return 0;
}

static int UrlEventAdded(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get((HANDLE)lParam, &dbei);
	if (dbei.flags & (DBEF_SENT|DBEF_READ) || dbei.eventType != EVENTTYPE_URL)
		return 0;

	SkinPlaySound("RecvUrl");

	TCHAR szTooltip[256];
	mir_sntprintf(szTooltip, SIZEOF(szTooltip), TranslateT("URL from %s"), pcli->pfnGetContactDisplayName(hContact, 0));

	CLISTEVENT cle = { sizeof(cle) };
	cle.flags = CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = (HANDLE)lParam;
	cle.hIcon = LoadSkinIcon(SKINICON_EVENT_URL);
	cle.pszService = "SRUrl/ReadUrl";
	cle.ptszTooltip = szTooltip;
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	return 0;
}

static INT_PTR SendUrlCommand(WPARAM wParam, LPARAM)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_URLSEND), NULL, DlgProcUrlSend, wParam);
	return 0;
}

static void RestoreUnreadUrlAlerts(void)
{
	CLISTEVENT cle = { sizeof(cle) };
	cle.hIcon = LoadSkinIcon(SKINICON_EVENT_URL);
	cle.pszService = "SRUrl/ReadUrl";
	cle.flags = CLEF_TCHAR;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hDbEvent = db_event_firstUnread(hContact);
		while (hDbEvent) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			if ( !(dbei.flags&(DBEF_SENT|DBEF_READ)) && dbei.eventType == EVENTTYPE_URL) {
				TCHAR toolTip[256];
				mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("URL from %s"), pcli->pfnGetContactDisplayName(hContact, 0));

				cle.hContact = hContact;
				cle.hDbEvent = hDbEvent;
				cle.ptszTooltip = toolTip;
				CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
			}
			hDbEvent = db_event_next(hContact, hDbEvent);
		}
	}
}

static int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *szProto = GetContactProto(wParam);
	if (mir_strcmp(cws->szModule, "CList") && (szProto == NULL || mir_strcmp(cws->szModule, szProto)))
		return 0;

	WindowList_Broadcast(hUrlWindowList, DM_UPDATETITLE, 0, 0);
	return 0;
}

static int SRUrlPreBuildMenu(WPARAM wParam, LPARAM)
{
	bool bEnabled = false;
	char *szProto = GetContactProto(wParam);
	if (szProto != NULL)
		if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_URLSEND)
			bEnabled = true;

	Menu_ShowItem(hSRUrlMenuItem, bEnabled);
	return 0;
}

static int SRUrlModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -2000040000;
	mi.icolibItem = GetSkinIconHandle(SKINICON_EVENT_URL);
	mi.pszName = LPGEN("Web page address (&URL)");
	mi.pszService = MS_URL_SENDURL;
	hSRUrlMenuItem = Menu_AddContactMenuItem(&mi);

	RestoreUnreadUrlAlerts();
	return 0;
}

static int SRUrlShutdown(WPARAM, LPARAM)
{
	if (hEventContactSettingChange)
		UnhookEvent(hEventContactSettingChange);

	if (hContactDeleted)
		UnhookEvent(hContactDeleted);

	if (hUrlWindowList) {
		WindowList_Broadcast(hUrlWindowList, WM_CLOSE, 0, 0);
		WindowList_Destroy(hUrlWindowList);
	}
	return 0;
}

int UrlContactDeleted(WPARAM wParam, LPARAM)
{
	HWND h = WindowList_Find(hUrlWindowList, wParam);
	if (h)
		SendMessage(h, WM_CLOSE, 0, 0);

	return 0;
}

int LoadSendRecvUrlModule(void)
{
	hUrlWindowList = WindowList_Create();
	HookEvent(ME_SYSTEM_MODULESLOADED, SRUrlModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, UrlEventAdded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SRUrlPreBuildMenu);
	hEventContactSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	hContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, UrlContactDeleted);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, SRUrlShutdown);
	CreateServiceFunction(MS_URL_SENDURL, SendUrlCommand);
	CreateServiceFunction("SRUrl/ReadUrl", ReadUrlCommand);
	SkinAddNewSoundEx("RecvUrl", LPGEN("URL"), LPGEN("Incoming"));
	return 0;
}
