/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
#include "clc.h"

struct CListImlIcon
{
	int index;
	HICON hIcon;
};
static struct CListImlIcon *imlIcon;
static int imlIconCount;

static UINT_PTR flashTimerId;
static int iconsOn;
static int disableTrayFlash;
static int disableIconFlash;

OBJLIST<CListEvent> g_cliEvents(10);

int fnGetImlIconIndex(HICON hIcon)
{
	int i;
	for (i=0; i < imlIconCount; i++)
		if (imlIcon[i].hIcon == hIcon)
			return imlIcon[i].index;

	imlIcon = (struct CListImlIcon *) mir_realloc(imlIcon, sizeof(struct CListImlIcon) * (imlIconCount + 1));
	imlIconCount++;
	imlIcon[i].hIcon = hIcon;
	imlIcon[i].index = ImageList_AddIcon(hCListImages, hIcon);
	return imlIcon[i].index;
}

static char* GetEventProtocol(int idx)
{
	if (!g_cliEvents.getCount() || idx < 0 && idx >= g_cliEvents.getCount())
		return nullptr;

	CListEvent &ev = g_cliEvents[idx];
	if (ev.hContact != 0)
		return GetContactProto(ev.hContact);
		
	return (ev.flags & CLEF_PROTOCOLGLOBAL) ? ev.lpszProtocol : nullptr;
}

static void ShowOneEventInTray(int idx)
{
	cli.pfnTrayIconUpdateWithImageList((iconsOn || disableTrayFlash) ? g_cliEvents[idx].imlIconIndex : 0, g_cliEvents[idx].szTooltip.w, GetEventProtocol(idx));
}

static void ShowEventsInTray()
{
	int nTrayCnt = cli.trayIconCount;
	if (!g_cliEvents.getCount() || !nTrayCnt)  return;
	if (g_cliEvents.getCount() == 1 || nTrayCnt == 1) {
		ShowOneEventInTray(0); //for only one icon in tray show topmost event
		return;
	}

	// in case if we have several icons in tray and several events with different protocols
	// lets use several icon to show events from protocols in different icons
	mir_cslock lck(trayLockCS);
	char **pTrayProtos = (char**)_alloca(sizeof(char*)*cli.trayIconCount);
	int nTrayProtoCnt = 0;
	for (int i = 0; i < cli.trayIconCount; i++)
		if (cli.trayIcon[i].id != 0 && cli.trayIcon[i].szProto)
			pTrayProtos[nTrayProtoCnt++] = cli.trayIcon[i].szProto;

	for (int i = 0; i < g_cliEvents.getCount(); i++) {
		char *iEventProto = GetEventProtocol(i);

		int j;
		for (j = 0; j < nTrayProtoCnt; j++)
			if (iEventProto && pTrayProtos[j] && !mir_strcmp(pTrayProtos[j], iEventProto))
				break;
		if (j >= nTrayProtoCnt)   // event was not found so assume first icon
			j = 0;
		if (pTrayProtos[j])       // if not already set
			ShowOneEventInTray(i); // show it
		pTrayProtos[j] = nullptr;    // and clear slot
	}
}

static VOID CALLBACK IconFlashTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	ShowEventsInTray();

	for (int i=0; i < g_cliEvents.getCount(); i++) {
		int j;
		for (j = 0; j < i; j++)
			if (g_cliEvents[j].hContact == g_cliEvents[i].hContact)
				break;
		if (j >= i)
			cli.pfnChangeContactIcon(g_cliEvents[i].hContact, iconsOn || disableIconFlash ? g_cliEvents[i].imlIconIndex : 0);
		
		// decrease eflashes in any case - no need to collect all events
		if (g_cliEvents[i].flags & CLEF_ONLYAFEW)
			if (0 >= --g_cliEvents[i].flashesDone)
				cli.pfnRemoveEvent(g_cliEvents[i].hContact, g_cliEvents[i].hDbEvent);
	}

	if (g_cliEvents.getCount() == 0) {
		KillTimer(nullptr, idEvent);
		cli.pfnTrayIconSetToBase(nullptr);
	}

	iconsOn = !iconsOn;
}

CListEvent* fnAddEvent(CLISTEVENT *cle)
{
	if (cle == nullptr)
		return nullptr;

	int i;
	if (cle->flags & CLEF_URGENT) {
		for (i=0; i < g_cliEvents.getCount(); i++)
			if (!(g_cliEvents[i].flags & CLEF_URGENT))
				break;
	}
	else i = g_cliEvents.getCount();

	CListEvent *p = new CListEvent();
	g_cliEvents.insert(p, i);
	memcpy(p, cle, sizeof(*cle));
	p->imlIconIndex = fnGetImlIconIndex(g_cliEvents[i].hIcon);
	p->flashesDone = 12;
	p->pszService = mir_strdup(g_cliEvents[i].pszService);
	if (p->flags & CLEF_UNICODE)
		p->szTooltip.w = mir_wstrdup(p->szTooltip.w);
	else
		p->szTooltip.w = mir_a2u(p->szTooltip.a); //if no flag defined it handled as unicode
	if (g_cliEvents.getCount() == 1) {
		char *szProto;
		if (cle->hContact == 0) {
			if (cle->flags & CLEF_PROTOCOLGLOBAL)
				szProto = cle->lpszProtocol;
			else
				szProto = nullptr;
		}
		else szProto = GetContactProto(cle->hContact);

		iconsOn = 1;
		flashTimerId = SetTimer(nullptr, 0, db_get_w(0, "CList", "IconFlashTime", 550), IconFlashTimer);
		cli.pfnTrayIconUpdateWithImageList(p->imlIconIndex, p->szTooltip.w, szProto);
	}
	cli.pfnChangeContactIcon(cle->hContact, p->imlIconIndex);
	return p;
}

// Removes an event from the contact list's queue
// Returns 0 if the event was successfully removed, or nonzero if the event was not found
int fnRemoveEvent(MCONTACT hContact, MEVENT dbEvent)
{
	// Find the event that should be removed
	int i;
	for (i = 0; i < g_cliEvents.getCount(); i++) {
		CListEvent &e = g_cliEvents[i];
		if (e.hContact == hContact && e.hDbEvent == dbEvent)
			break;
	}

	// Event was not found
	if (i == g_cliEvents.getCount())
		return 1;

	// Update contact's icon
	char *szProto = GetContactProto(hContact);
	cli.pfnChangeContactIcon(g_cliEvents[i].hContact, cli.pfnGetContactIcon(g_cliEvents[i].hContact));

	// Free any memory allocated to the event
	g_cliEvents.remove(i);

	// count same protocoled events
	int nSameProto = 0;
	char *szEventProto;
	for (i = 0; i < g_cliEvents.getCount(); i++) {
		if (g_cliEvents[i].hContact)
			szEventProto = GetContactProto((g_cliEvents[i].hContact));
		else if (g_cliEvents[i].flags & CLEF_PROTOCOLGLOBAL)
			szEventProto = (char*)g_cliEvents[i].lpszProtocol;
		else
			szEventProto = nullptr;
		if (szEventProto && szProto && !mir_strcmp(szEventProto, szProto))
			nSameProto++;
	}

	if (g_cliEvents.getCount() == 0 || nSameProto == 0) {
		if (g_cliEvents.getCount() == 0)
			KillTimer(nullptr, flashTimerId);
		cli.pfnTrayIconSetToBase(hContact == 0 ? nullptr : szProto);
	}
	else {
		if (g_cliEvents[0].hContact == 0)
			szProto = nullptr;
		else
			szProto = GetContactProto(g_cliEvents[0].hContact);
		cli.pfnTrayIconUpdateWithImageList(iconsOn ? g_cliEvents[0].imlIconIndex : 0, g_cliEvents[0].szTooltip.w, szProto);
	}

	return 0;
}

CLISTEVENT* fnGetEvent(MCONTACT hContact, int idx)
{
	if (hContact == INVALID_CONTACT_ID) {
		if (idx >= g_cliEvents.getCount())
			return nullptr;
		return &g_cliEvents[idx];
	}

	for (int i=0; i < g_cliEvents.getCount(); i++)
		if (g_cliEvents[i].hContact == hContact)
			if (idx-- == 0)
				return &g_cliEvents[i];
	return nullptr;
}

int fnEventsProcessContactDoubleClick(MCONTACT hContact)
{
	for (int i = 0; i < g_cliEvents.getCount(); i++) {
		if (g_cliEvents[i].hContact == hContact) {
			MEVENT hDbEvent = g_cliEvents[i].hDbEvent;
			CallService(g_cliEvents[i].pszService, 0, (LPARAM)&g_cliEvents[i]);
			cli.pfnRemoveEvent(hContact, hDbEvent);
			return 0;
		}
	}

	return 1;
}

int fnEventsProcessTrayDoubleClick(int index)
{
	BOOL click_in_first_icon = FALSE;
	if (g_cliEvents.getCount() == 0)
		return 1;

	int eventIndex = 0;

	mir_cslockfull lck(trayLockCS);
	if (cli.trayIconCount > 1 && index > 0) {
		int i;
		char *szProto = nullptr;
		for (i = 0; i < cli.trayIconCount; i++) {
			if (cli.trayIcon[i].id == index) {
				szProto = cli.trayIcon[i].szProto;
				if (i == 0)
					click_in_first_icon = TRUE;
				break;
			}
		}
		if (szProto) {
			for (i = 0; i < g_cliEvents.getCount(); i++) {
				char *eventProto = nullptr;
				if (g_cliEvents[i].hContact)
					eventProto = GetContactProto(g_cliEvents[i].hContact);
				if (!eventProto)
					eventProto = g_cliEvents[i].lpszProtocol;

				if (!eventProto || !_strcmpi(eventProto, szProto)) {
					eventIndex = i;
					break;
				}
			}

			// let's process backward try to find first event without desired proto in tray
			if (i == g_cliEvents.getCount()) {
				if (click_in_first_icon) {
					for (i = 0; i < g_cliEvents.getCount(); i++) {
						char *eventProto = nullptr;
						if (g_cliEvents[i].hContact)
							eventProto = GetContactProto(g_cliEvents[i].hContact);
						if (!eventProto)
							eventProto = g_cliEvents[i].lpszProtocol;
						if (!eventProto)
							continue;

						int j;
						for (j = 0; j < cli.trayIconCount; j++)
							if (cli.trayIcon[j].szProto && !_strcmpi(eventProto, cli.trayIcon[j].szProto))
								break;

						if (j == cli.trayIconCount) {
							eventIndex = i;
							break;
						}
					}
				}
				if (i == g_cliEvents.getCount()) //not found
					return 1;	//continue processing to show contact list
			}
		}
	}
	lck.unlock();

	MCONTACT hContact = g_cliEvents[eventIndex].hContact;
	MEVENT hDbEvent = g_cliEvents[eventIndex].hDbEvent;
	//	; may be better to show send msg?
	CallService(g_cliEvents[eventIndex].pszService, 0, (LPARAM)&g_cliEvents[eventIndex]);
	cli.pfnRemoveEvent(hContact, hDbEvent);
	return 0;
}

static int RemoveEventsForContact(WPARAM wParam, LPARAM)
{
	for (int i = g_cliEvents.getCount()-1; i >= 0; i--) {
		CListEvent &e = g_cliEvents[i];
		if (e.hContact == wParam)
			cli.pfnRemoveEvent(wParam, e.hDbEvent);
	}

	return 0;
}

static int CListEventSettingsChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == 0 && cws && cws->szModule && cws->szSetting && strcmp(cws->szModule, "CList") == 0) {
		if (strcmp(cws->szSetting, "DisableTrayFlash") == 0)
			disableTrayFlash = (int)cws->value.bVal;
		else if (strcmp(cws->szSetting, "NoIconBlink") == 0)
			disableIconFlash = (int)cws->value.bVal;
	}
	return 0;
}

/***************************************************************************************/

int InitCListEvents(void)
{
	cli.events = &g_cliEvents;

	disableTrayFlash = db_get_b(0, "CList", "DisableTrayFlash", 0);
	disableIconFlash = db_get_b(0, "CList", "NoIconBlink", 0);
	HookEvent(ME_DB_CONTACT_DELETED, RemoveEventsForContact);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, CListEventSettingsChanged);
	return 0;
}

void UninitCListEvents(void)
{
	if (g_cliEvents.getCount())
		KillTimer(nullptr, flashTimerId);
	g_cliEvents.destroy();

	if (imlIcon != nullptr)
		mir_free(imlIcon);
}
