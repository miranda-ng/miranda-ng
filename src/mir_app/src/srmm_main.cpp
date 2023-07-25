/*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HCURSOR g_hCurHyperlinkHand;
HANDLE hHookIconsChanged, hHookIconPressedEvt, hHookSrmmEvent;

static HANDLE hHookEmptyHistory;
static HGENMENU hmiEmpty;

void LoadSrmmToolbarModule();
void UnloadSrmmToolbarModule();

/////////////////////////////////////////////////////////////////////////////////////////
// Empty history service for main menu

static INT_PTR svcEmptyHistory(WPARAM hContact, LPARAM lParam)
{
	if (NotifyEventHooks(hHookEmptyHistory))
		return 2;

	if (lParam == 0)
		if (IDYES != MessageBoxW(nullptr, TranslateT("Are you sure to remove all events from history?"), L"Miranda", MB_YESNO | MB_ICONQUESTION))
			return 1;

	DB::ECPTR pCursor(DB::Events(hContact));
	while (pCursor.FetchNext())
		pCursor.DeleteEvent();
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hmiEmpty, db_event_first(hContact) != 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Built-in hook to redraw RTF-based event logs on event's change/deletion

static int OnRedrawLog(WPARAM hContact, LPARAM)
{
	if (auto *pDlg = Srmm_FindDialog(hContact))
		pDlg->ScheduleRedrawLog();

	if (db_mc_isSub(hContact))
		if (auto *pDlg = Srmm_FindDialog(db_mc_getMeta(hContact)))
			pDlg->ScheduleRedrawLog();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SrmmModulesLoaded()
{
	// menu item
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x0d4306aa, 0xe31e, 0x46ee, 0x89, 0x88, 0x3a, 0x2e, 0x05, 0xa6, 0xf3, 0xbc);
	mi.pszService = MS_HISTORY_EMPTY;
	mi.name.a = LPGEN("Empty history");
	mi.position = 1000090001;
	mi.hIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	hmiEmpty = Menu_AddContactMenuItem(&mi);

	// create menu item in main menu for empty system history
	SET_UID(mi, 0x633AD23C, 0x24B5, 0x4914, 0xB2, 0x40, 0xAD, 0x9F, 0xAC, 0xB5, 0x64, 0xED);
	mi.position = 500060002;
	mi.name.a = LPGEN("Empty system history");
	mi.pszService = MS_HISTORY_EMPTY;
	mi.hIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadSrmmModule()
{
	g_hCurHyperlinkHand = LoadCursor(nullptr, IDC_HAND);

	LoadSrmmToolbarModule();

	CreateServiceFunction(MS_HISTORY_EMPTY, svcEmptyHistory);
	hHookEmptyHistory = CreateHookableEvent(ME_HISTORY_EMPTY);

	hHookSrmmEvent = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookIconsChanged = CreateHookableEvent(ME_MSG_ICONSCHANGED);
	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);

	HookEvent(ME_DB_EVENT_EDITED, OnRedrawLog);
	HookEvent(ME_DB_EVENT_DELETED, OnRedrawLog);
	return 0;
}

void UnloadSrmmModule()
{
	DestroyHookableEvent(hHookIconsChanged);
	DestroyHookableEvent(hHookSrmmEvent);
	DestroyHookableEvent(hHookIconPressedEvt);

	DestroyCursor(g_hCurHyperlinkHand);

	UnloadSrmmToolbarModule();
}
