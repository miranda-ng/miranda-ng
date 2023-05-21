/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org),
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
	CListImlIcon(HICON _icon) :
		hIcon(_icon),
		index(-1)
	{}

	HICON hIcon;
	int index;
};

static int CompareImlIcons(const CListImlIcon *p1, const CListImlIcon *p2)
{
	if (p1->hIcon == p2->hIcon)
		return 0;
	return (UINT_PTR(p1->hIcon) < UINT_PTR(p2->hIcon)) ? -1 : 1;
}

static OBJLIST<CListImlIcon> arImlIcons(10, CompareImlIcons);

/////////////////////////////////////////////////////////////////////////////////////////

static UINT_PTR flashTimerId;
static int iconsOn;
static bool g_bEnableTrayFlash;
static bool g_bEnableIconFlash;
static volatile long iEventOrder = 0;

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareEvents(const CListEvent *p1, const CListEvent *p2)
{
	int flag1 = p1->flags & CLEF_URGENT, flag2 = p2->flags & CLEF_URGENT;
	if (flag1 != flag2)
		return flag2 - flag1; // reverse sort order

	return p1->iOrder - p2->iOrder;
}

OBJLIST<CListEvent> g_cliEvents(10, CompareEvents);

static const char* GetEventProtocol(const CListEvent &ev)
{
	if (ev.hContact != 0)
		return Proto_GetBaseAccountName(ev.hContact);

	return (ev.flags & CLEF_PROTOCOLGLOBAL) ? ev.moduleName : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

int fnGetImlIconIndex(HICON hIcon)
{
	auto *p = arImlIcons.find((CListImlIcon*)&hIcon);
	if (p != nullptr)
		return p->index;

	p = new CListImlIcon(hIcon);
	arImlIcons.insert(p);
	return p->index = ImageList_AddIcon(hCListImages, hIcon);
}

static void ShowOneEventInTray(const CListEvent &ev)
{
	TrayIconUpdateWithImageList((iconsOn || !g_bEnableTrayFlash) ? ev.imlIconIndex : 0, ev.szTooltip.w, GetEventProtocol(ev));
}

static void ShowEventsInTray()
{
	int nTrayCnt = g_clistApi.trayIconCount;
	if (!g_cliEvents.getCount() || !nTrayCnt)
		return;

	if (g_cliEvents.getCount() == 1 || nTrayCnt == 1) {
		ShowOneEventInTray(g_cliEvents[0]); //for only one icon in tray show topmost event
		return;
	}

	// in case if we have several icons in tray and several events with different protocols
	// lets use several icon to show events from protocols in different icons
	mir_cslock lck(trayLockCS);
	char **pTrayProtos = (char**)_alloca(sizeof(char*)*g_clistApi.trayIconCount);
	int nTrayProtoCnt = 0;
	for (int i = 0; i < g_clistApi.trayIconCount; i++)
		if (g_clistApi.trayIcon[i].id != 0 && g_clistApi.trayIcon[i].szProto)
			pTrayProtos[nTrayProtoCnt++] = g_clistApi.trayIcon[i].szProto;

	for (auto &ev : g_cliEvents) {
		const char *iEventProto = GetEventProtocol(*ev);

		int j;
		for (j = 0; j < nTrayProtoCnt; j++)
			if (iEventProto && pTrayProtos[j] && !mir_strcmp(pTrayProtos[j], iEventProto))
				break;
		if (j >= nTrayProtoCnt)   // event was not found so assume first icon
			j = 0;
		if (pTrayProtos[j])       // if not already set
			ShowOneEventInTray(*ev); // show it
		pTrayProtos[j] = nullptr;    // and clear slot
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adds an event to the contact list's queue

static VOID CALLBACK IconFlashTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	ShowEventsInTray();

	for (int i = 0; i < g_cliEvents.getCount(); i++) {
		auto &e = g_cliEvents[i];
		int j;
		for (j = 0; j < i; j++)
			if (g_cliEvents[j].hContact == e.hContact)
				break;
		if (j >= i)
			Clist_ChangeContactIcon(e.hContact, iconsOn || !g_bEnableTrayFlash ? e.imlIconIndex : 0);

		// decrease eflashes in any case - no need to collect all events
		if (e.flags & CLEF_ONLYAFEW)
			if (0 >= --e.flashesDone)
				Clist_RemoveEvent(e.hContact, e.hDbEvent);
	}

	if (g_cliEvents.getCount() == 0) {
		KillTimer(nullptr, idEvent);
		TrayIconSetToBase(nullptr);
	}

	iconsOn = !iconsOn;
}

static INT_PTR CALLBACK DoAddEvent(void *param)
{
	CLISTEVENT *cle = (CLISTEVENT*)param;

	CListEvent *p = new CListEvent();
	memcpy(p, cle, sizeof(*cle));
	p->iOrder = InterlockedIncrement(&iEventOrder);
	p->imlIconIndex = fnGetImlIconIndex(p->hIcon);
	p->flashesDone = 12;
	p->pszService = mir_strdup(p->pszService);
	if (p->flags & CLEF_UNICODE)
		p->szTooltip.w = mir_wstrdup(p->szTooltip.w);
	else
		p->szTooltip.w = mir_a2u(p->szTooltip.a); //if no flag defined it handled as unicode
	g_cliEvents.insert(p);

	if (g_cliEvents.getCount() == 1) {
		iconsOn = 1;
		flashTimerId = SetTimer(nullptr, 0, db_get_w(0, "CList", "IconFlashTime", 550), IconFlashTimer);
		TrayIconUpdateWithImageList(p->imlIconIndex, p->szTooltip.w, GetEventProtocol(*p));
	}
	Clist_ChangeContactIcon(cle->hContact, p->imlIconIndex);
	return (INT_PTR)p;
}

CListEvent* fnAddEvent(CLISTEVENT *cle)
{
	if (cle == nullptr)
		return nullptr;

	return (CListEvent*)CallFunctionSync(DoAddEvent, cle);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Removes an event from the contact list's queue
// Returns 0 if the event was successfully removed, or nonzero if the event was not found

int fnRemoveEvent(CListEvent *pEvent)
{
	// save hContact, because pEvent should be freed
	auto hContact = pEvent->hContact;

	// Update contact's icon
	char *szProto = Proto_GetBaseAccountName(hContact);
	Clist_ChangeContactIcon(hContact, Clist_GetContactIcon(hContact));

	// Free any memory allocated to the event
	g_cliEvents.remove(pEvent);

	// count same protocoled events
	int nSameProto = 0;
	for (auto &it : g_cliEvents) {
		const char *szEventProto = GetEventProtocol(*it);
		if (szEventProto && szProto && !mir_strcmp(szEventProto, szProto))
			nSameProto++;
	}

	if (g_cliEvents.getCount() == 0 || nSameProto == 0) {
		if (g_cliEvents.getCount() == 0)
			KillTimer(nullptr, flashTimerId);
		TrayIconSetToBase(hContact == 0 ? nullptr : szProto);
	}
	else {
		if (g_cliEvents[0].hContact == 0)
			szProto = nullptr;
		else
			szProto = Proto_GetBaseAccountName(g_cliEvents[0].hContact);
		TrayIconUpdateWithImageList(iconsOn ? g_cliEvents[0].imlIconIndex : 0, g_cliEvents[0].szTooltip.w, szProto);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(CListEvent *) Clist_FindEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	for (auto &it : g_cliEvents) {
		if ((hContact == it->hContact || hContact == -1) && it->hDbEvent == hDbEvent)
			return it;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_RemoveEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (auto *pEvent = Clist_FindEvent(hContact, hDbEvent)) {
		g_clistApi.pfnFreeEvent(pEvent);
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(CListEvent *) Clist_GetEvent(MCONTACT hContact, int idx)
{
	if (hContact == INVALID_CONTACT_ID) {
		if (idx >= g_cliEvents.getCount())
			return nullptr;
		return &g_cliEvents[idx];
	}

	for (auto &it : g_cliEvents)
		if (it->hContact == hContact)
			if (idx-- == 0)
				return it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EventsProcessContactDoubleClick(MCONTACT hContact)
{
	for (auto &it : g_cliEvents) {
		if (it->hContact == hContact) {
			CallService(it->pszService, 0, (LPARAM)it);
			g_clistApi.pfnFreeEvent(it);
			return 0;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_EventsProcessTrayDoubleClick(int index)
{
	BOOL click_in_first_icon = FALSE;
	if (g_cliEvents.getCount() == 0)
		return 1;

	CListEvent *pEvent = nullptr;
	{
		mir_cslock lck(trayLockCS);
		if (g_clistApi.trayIconCount > 1 && index > 0) {
			char *szProto = nullptr;
			for (int i = 0; i < g_clistApi.trayIconCount; i++) {
				if (g_clistApi.trayIcon[i].id == index) {
					szProto = g_clistApi.trayIcon[i].szProto;
					if (i == 0)
						click_in_first_icon = TRUE;
					break;
				}
			}
			if (szProto) {
				for (auto &it : g_cliEvents) {
					const char *eventProto = GetEventProtocol(*it);
					if (!eventProto || !_strcmpi(eventProto, szProto)) {
						pEvent = it;
						break;
					}
				}

				// let's process backward try to find first event without desired proto in tray
				if (pEvent == nullptr) {
					if (click_in_first_icon) {
						for (auto &it : g_cliEvents) {
							const char *eventProto = GetEventProtocol(*it);
							if (!eventProto)
								continue;

							int j;
							for (j = 0; j < g_clistApi.trayIconCount; j++)
								if (g_clistApi.trayIcon[j].szProto && !_strcmpi(eventProto, g_clistApi.trayIcon[j].szProto))
									break;

							if (j == g_clistApi.trayIconCount) {
								pEvent = it;
								break;
							}
						}
					}
					if (pEvent == nullptr) //not found
						return 1;	//continue processing to show contact list
				}
			}
		}
	}

	// by default use the first event
	if (pEvent == nullptr)
		pEvent = &g_cliEvents[0];

	// copy info in case that events' array could be shifted by the service call
	MCONTACT hContact = pEvent->hContact;
	MEVENT hDbEvent = pEvent->hDbEvent;
	CallService(pEvent->pszService, 0, (LPARAM)pEvent);
	Clist_RemoveEvent(hContact, hDbEvent);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

static int CListEventMarkedRead(WPARAM hContact, LPARAM hDbEvent)
{
	Clist_RemoveEvent(hContact, hDbEvent);
	return 0;
}

static int CListEventSettingsChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == 0 && cws && cws->szModule && cws->szSetting && strcmp(cws->szModule, "CList") == 0) {
		if (strcmp(cws->szSetting, "EnableTrayFlash") == 0)
			g_bEnableTrayFlash = (int)cws->value.bVal;
		else if (strcmp(cws->szSetting, "EnableIconBlink") == 0)
			g_bEnableIconFlash = (int)cws->value.bVal;
	}
	return 0;
}

int InitCListEvents(void)
{
	g_clistApi.events = &g_cliEvents;

	if (db_get_b(0, MODULENAME, "DisableTrayFlash")) {
		Clist::EnableTrayFlash = false;
		db_unset(0, MODULENAME, "DisableTrayFlash");
	}

	if (db_get_b(0, MODULENAME, "DisableIconBlink")) {
		Clist::EnableIconBlink = false;
		db_unset(0, MODULENAME, "DisableIconBlink");
	}

	g_bEnableTrayFlash = Clist::EnableTrayFlash;
	g_bEnableIconFlash = Clist::EnableIconBlink;

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, CListEventSettingsChanged);
	HookEvent(ME_DB_EVENT_MARKED_READ, CListEventMarkedRead);
	return 0;
}

void UninitCListEvents(void)
{
	if (g_cliEvents.getCount())
		KillTimer(nullptr, flashTimerId);
	g_cliEvents.destroy();

	arImlIcons.destroy();
}
