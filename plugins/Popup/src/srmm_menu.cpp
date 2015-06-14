/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

#include "headers.h"

/*************************************************************************************
	=== how does this work ===
	We have four icons -- one for each mode. and we do hide/show them depending
	on current active mode for user.
	*************************************************************************************/

static HANDLE hDialogsList = NULL;

static void SrmmMenu_UpdateIcon(MCONTACT hContact)
{
	if (!hContact)
		return;

	int mode = db_get_b(hContact, MODULNAME, "ShowMode", PU_SHOWMODE_AUTO);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULNAME;

	for (int i = 0; i < 4; i++) {
		sid.dwId = i;
		sid.flags = (i == mode) ? 0 : MBF_HIDDEN;
		Srmm_ModifyIcon(hContact, &sid);
	}
}

static int SrmmMenu_ProcessEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *mwevent = (MessageWindowEventData *)lParam;

	if (mwevent->uType == MSG_WINDOW_EVT_OPEN) {
		if (!hDialogsList)
			hDialogsList = WindowList_Create();

		WindowList_Add(hDialogsList, mwevent->hwndWindow, mwevent->hContact);
		SrmmMenu_UpdateIcon(mwevent->hContact);
	}
	else if (mwevent->uType == MSG_WINDOW_EVT_CLOSING) {
		if (hDialogsList)
			WindowList_Remove(hDialogsList, mwevent->hwndWindow);
	}

	return 0;
}

static int SrmmMenu_ProcessIconClick(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (mir_strcmp(sicd->szModule, MODULNAME))
		return 0;

	if (!hContact)
		return 0;

	int mode = db_get_b(hContact, MODULNAME, "ShowMode", PU_SHOWMODE_AUTO);

	if (sicd->flags & MBCF_RIGHTBUTTON) {
		HMENU hMenu = CreatePopupMenu();

		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_AUTO, TranslateT("Auto"));
		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_FAVORITE, TranslateT("Favorite"));
		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_FULLSCREEN, TranslateT("Ignore fullscreen"));
		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_BLOCK, TranslateT("Block"));

		CheckMenuItem(hMenu, 1 + mode, MF_BYCOMMAND | MF_CHECKED);

		mode = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, WindowList_Find(hDialogsList, hContact), NULL);
		if (mode) {
			db_set_b(hContact, MODULNAME, "ShowMode", mode - 1);
			SrmmMenu_UpdateIcon(hContact);
		}
	}
	else {
		db_set_b(hContact, MODULNAME, "ShowMode", (mode == PU_SHOWMODE_AUTO) ? PU_SHOWMODE_BLOCK : PU_SHOWMODE_AUTO);
		SrmmMenu_UpdateIcon(hContact);
	}

	return 0;
}

void SrmmMenu_Load()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULNAME;

	sid.dwId = 0;
	sid.szTooltip = LPGEN("Popup Mode: Auto");
	sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_POPUP_ON, 0);
	Srmm_AddIcon(&sid);

	sid.dwId = 1;
	sid.szTooltip = LPGEN("Popup Mode: Favorite");
	sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_FAV, 0);
	Srmm_AddIcon(&sid);

	sid.dwId = 2;
	sid.szTooltip = LPGEN("Popup Mode: Ignore fullscreen");
	sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_FULLSCREEN, 0);
	Srmm_AddIcon(&sid);

	sid.dwId = 3;
	sid.szTooltip = LPGEN("Popup Mode: Block contact");
	sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_POPUP_OFF, 0);
	Srmm_AddIcon(&sid);

	HookEvent(ME_MSG_ICONPRESSED, SrmmMenu_ProcessIconClick);
	HookEvent(ME_MSG_WINDOWEVENT, SrmmMenu_ProcessEvent);
}

void SrmmMenu_Unload()
{
	WindowList_Destroy(hDialogsList);
}
