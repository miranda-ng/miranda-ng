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

static IconItem iconList1[] =
{
	// common popup
	{ LPGEN("With \"favorite\" overlay"), "favourite", IDI_PU_FAVOURITE },
	{ LPGEN("With \"fullscreen\" overlay"), "fullscreen", IDI_PU_FULLSCREEN },
	{ LPGEN("Popup history"), "history", IDI_HISTORY },

	// notifications
	{ LPGEN("Information"), "info", IDI_MB_INFO },
	{ LPGEN("Warning"), "warning", IDI_MB_WARN },
	{ LPGEN("Error"), "error", IDI_MB_STOP }
};

static IconItem iconList2[] =
{
	// option
	{ LPGEN("Refresh skin list"), "opt_reload", IDI_RELOAD },
	{ LPGEN("Popup placement"), "opt_resize", IDI_RESIZE },
	{ LPGEN("OK"), "opt_ok", IDI_ACT_OK },
	{ LPGEN("Cancel"), "opt_cancel", IDI_ACT_CLOSE },
	{ LPGEN("Popup group"), "opt_group", IDI_OPT_GROUP },
	{ LPGEN("Show default"), "opt_default", IDI_ACT_OK },
	{ LPGEN("Favorite contact"), "opt_favorite", IDI_OPT_FAVORITE },
	{ LPGEN("Show in fullscreen"), "opt_fullscreen", IDI_OPT_FULLSCREEN },
	{ LPGEN("Blocked contact"), "opt_block", IDI_OPT_BLOCK }
};

static IconItem iconList3[] =
{
	// action
	{ LPGEN("Quick reply"), "act_reply", IDI_ACT_REPLY },
	{ LPGEN("Pin popup"), "act_pin", IDI_ACT_PIN },
	{ LPGEN("Pinned popup"), "act_pinned", IDI_ACT_PINNED },
	{ LPGEN("Send message"), "act_message", IDI_ACT_MESSAGE },
	{ LPGEN("User details"), "act_info", IDI_ACT_INFO },
	{ LPGEN("Contact menu"), "act_menu", IDI_ACT_MENU },
	{ LPGEN("Add contact permanently"), "act_add", IDI_ACT_ADD },
	{ LPGEN("Dismiss popup"), "act_close", IDI_ACT_CLOSE },
	{ LPGEN("Copy to clipboard"), "act_copy", IDI_ACT_COPY }
};

void InitIcons()
{
	g_plugin.registerIcon(SECT_POPUP, iconList1, MODULENAME);
	g_plugin.registerIcon(SECT_POPUP SECT_POPUP_OPT, iconList2, MODULENAME);
	g_plugin.registerIcon(SECT_POPUP SECT_POPUP_ACT, iconList3, MODULENAME);
}

HICON LoadIconEx(int iconId, bool big)
{
	for (auto &it : iconList1)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib, big);

	for (auto &it : iconList2)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib, big);

	for (auto &it : iconList3)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib, big);

	return nullptr;
}

HANDLE GetIconHandle(int iconId)
{
	for (auto &it : iconList1)
		if (it.defIconID == iconId)
			return it.hIcolib;

	for (auto &it : iconList2)
		if (it.defIconID == iconId)
			return it.hIcolib;

	for (auto &it : iconList3)
		if (it.defIconID == iconId)
			return it.hIcolib;

	return nullptr;
}
