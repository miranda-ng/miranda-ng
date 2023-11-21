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

#include "stdafx.h"

/*************************************************************************************
	=== how does this work ===
	We have four icons -- one for each mode. and we do hide/show them depending
	on current active mode for user.
	*************************************************************************************/

static void SrmmMenu_UpdateIcon(MCONTACT hContact)
{
	if (!hContact)
		return;

	int mode = g_plugin.getByte(hContact, "ShowMode", PU_SHOWMODE_AUTO);

	for (int i = 0; i < 3; i++) {
		uint32_t dwFlags = 0;
		if (i == 0 && mode == PU_SHOWMODE_BLOCK)
			dwFlags = MBF_DISABLED;
		else if (i != mode)
			dwFlags = MBF_HIDDEN;
		Srmm_SetIconFlags(hContact, MODULENAME, i, dwFlags);
	}
}

static int SrmmMenu_ProcessEvent(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;

	if (uType == MSG_WINDOW_EVT_OPEN)
		SrmmMenu_UpdateIcon(pDlg->m_hContact);
	return 0;
}

static int SrmmMenu_ProcessIconClick(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (mir_strcmp(sicd->szModule, MODULENAME))
		return 0;

	if (!hContact)
		return 0;

	int mode = g_plugin.getByte(hContact, "ShowMode", PU_SHOWMODE_AUTO);

	if (sicd->flags & MBCF_RIGHTBUTTON) {
		HMENU hMenu = CreatePopupMenu();

		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_AUTO, TranslateT("Auto"));
		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_FAVORITE, TranslateT("Favorite"));
		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_FULLSCREEN, TranslateT("Ignore fullscreen"));
		AppendMenu(hMenu, MF_STRING, 1 + PU_SHOWMODE_BLOCK, TranslateT("Block"));

		CheckMenuItem(hMenu, 1 + mode, MF_BYCOMMAND | MF_CHECKED);

		mode = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, Srmm_FindWindow(hContact), nullptr);
		if (mode) {
			g_plugin.setByte(hContact, "ShowMode", mode - 1);
			SrmmMenu_UpdateIcon(hContact);
		}
	}
	else {
		g_plugin.setByte(hContact, "ShowMode", (mode == PU_SHOWMODE_AUTO) ? PU_SHOWMODE_BLOCK : PU_SHOWMODE_AUTO);
		SrmmMenu_UpdateIcon(hContact);
	}

	return 0;
}

void SrmmMenu_Load()
{
	StatusIconData sid = {};
	sid.szModule = MODULENAME;

	sid.dwId = 0;
	sid.szTooltip.a = LPGEN("Popup mode: Auto");
	sid.hIcon = Skin_LoadIcon(SKINICON_OTHER_POPUP);
	sid.hIconDisabled = Skin_LoadIcon(SKINICON_OTHER_NOPOPUP);
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 1;
	sid.szTooltip.a = LPGEN("Popup mode: Favorite");
	sid.hIcon = sid.hIconDisabled = g_plugin.getIcon(IDI_OPT_FAVORITE);
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 2;
	sid.szTooltip.a = LPGEN("Popup mode: Ignore fullscreen");
	sid.hIcon = sid.hIconDisabled = g_plugin.getIcon(IDI_OPT_FULLSCREEN);
	Srmm_AddIcon(&sid, &g_plugin);

	HookEvent(ME_MSG_ICONPRESSED, SrmmMenu_ProcessIconClick);
	HookEvent(ME_MSG_WINDOWEVENT, SrmmMenu_ProcessEvent);
}
