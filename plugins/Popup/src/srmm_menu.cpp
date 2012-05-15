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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/srmm_menu.cpp $
Revision       : $Revision: 1610 $
Last change on : $Date: 2010-06-23 00:55:13 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

/*************************************************************************************
	=== how does this work ===
	We have four icons -- one for each mode. and we do hide/show them depending
	on current active mode for user.
*************************************************************************************/

static HANDLE hDialogsList = NULL;
static HANDLE hIconPressed=0,hWindowEvent=0;

static int SrmmMenu_ProcessEvent(WPARAM wParam, LPARAM lParam);
static int SrmmMenu_ProcessIconClick(WPARAM wParam, LPARAM lParam);


void SrmmMenu_Load()
{
	if (ServiceExists(MS_MSG_ADDICON))
	{
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULNAME;
		sid.flags = 0;

		sid.dwId = 0;
		sid.szTooltip = Translate("Popup Mode: Auto");
		sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_POPUP_ON,0);
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		sid.dwId = 1;
		sid.szTooltip = Translate("Popup Mode: Favourite");
		sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_FAV,0);
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		sid.dwId = 2;
		sid.szTooltip = Translate("Popup Mode: Ignore fullscreen");
		sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_FULLSCREEN,0);
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		sid.dwId = 3;
		sid.szTooltip = Translate("Popup Mode: Block contact");
		sid.hIcon = sid.hIconDisabled = IcoLib_GetIcon(ICO_POPUP_OFF,0);
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
	
		hIconPressed = HookEvent(ME_MSG_ICONPRESSED, SrmmMenu_ProcessIconClick);
		hWindowEvent = HookEvent(ME_MSG_WINDOWEVENT, SrmmMenu_ProcessEvent);
/*
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			SrmmMenu_UpdateIcon(hContact);
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
*/
	}
}

void SrmmMenu_Unload()
{
	UnhookEvent(hIconPressed);
	UnhookEvent(hWindowEvent);
}

static void SrmmMenu_UpdateIcon(HANDLE hContact)
{
	if (!hContact) return;

	int mode = DBGetContactSettingByte(hContact, MODULNAME, "ShowMode", PU_SHOWMODE_AUTO);

	StatusIconData sid = {0};
	sid.cbSize = sizeof(sid);
	sid.szModule = MODULNAME;

	for (int i = 0; i < 4; ++i)
	{
		sid.dwId = i;
		sid.flags = (i == mode) ? 0 : MBF_HIDDEN;
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
	}
}

static int SrmmMenu_ProcessEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if ( event->uType == MSG_WINDOW_EVT_OPEN )
	{
		if (!hDialogsList)
			hDialogsList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

		WindowList_Add(hDialogsList, event->hwndWindow, event->hContact);
		SrmmMenu_UpdateIcon(event->hContact);
	}
	else if ( event->uType == MSG_WINDOW_EVT_CLOSING )
	{
		if (hDialogsList)
			WindowList_Remove(hDialogsList, event->hwndWindow);
	}

	return 0;
}

static int SrmmMenu_ProcessIconClick(WPARAM wParam, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (lstrcmpA(sicd->szModule, MODULNAME)) return 0;

	HANDLE hContact = (HANDLE)wParam;
	if (!hContact) return 0;

	int mode = DBGetContactSettingByte(hContact, MODULNAME, "ShowMode", PU_SHOWMODE_AUTO);

	if (sicd->flags&MBCF_RIGHTBUTTON)
	{
		HMENU hMenu = CreatePopupMenu();

		AppendMenu(hMenu, MF_STRING, 1+PU_SHOWMODE_AUTO,		TranslateT("Auto"));
		AppendMenu(hMenu, MF_STRING, 1+PU_SHOWMODE_FAVORITE,	TranslateT("Favourite"));
		AppendMenu(hMenu, MF_STRING, 1+PU_SHOWMODE_FULLSCREEN,	TranslateT("Ignore fullscreen"));
		AppendMenu(hMenu, MF_STRING, 1+PU_SHOWMODE_BLOCK,		TranslateT("Block"));

		CheckMenuItem(hMenu, 1+mode, MF_BYCOMMAND|MF_CHECKED);

		mode = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, WindowList_Find(hDialogsList, hContact), NULL);
		
		if (mode)
		{
			DBWriteContactSettingByte(hContact, MODULNAME, "ShowMode", mode-1);
			SrmmMenu_UpdateIcon(hContact);
		}
	} else
	{
		DBWriteContactSettingByte(hContact, MODULNAME, "ShowMode",
			(mode == PU_SHOWMODE_AUTO) ? PU_SHOWMODE_BLOCK : PU_SHOWMODE_AUTO);
		SrmmMenu_UpdateIcon(hContact);
	}

	return 0;
}
