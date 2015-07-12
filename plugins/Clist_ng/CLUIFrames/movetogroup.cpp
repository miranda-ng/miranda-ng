/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 * -----------------------------------------------------------------------
 * The CLUI frames module was first written many years ago for Miranda 0.2
 * as part of the multiwindow patch. Later it was converted to a contact
 * list plugin (clist_mw) on which all modern contact list plugins for
 * miranda are still somewhat based.
 *
 * original author of the CLUI frames module is a guy with the nick name
 * Bethoven
 *
 * $Id: movetogroup.cpp 109 2010-09-06 00:48:29Z silvercircle $
 *
 */

#include <commonheaders.h>

HANDLE hOnCntMenuBuild;
HGENMENU hPriorityItem = 0, hFloatingItem = 0;

static int OnContactMenuBuild(WPARAM wParam, LPARAM lParam)
{
	CMenuItem 	mi;
	BYTE 			bSetting;

	bSetting = cfg::getByte("CList", "flt_enabled", 0);
	if (bSetting && !hFloatingItem) {
		mi.position=200000;
		mi.pszService="CList/SetContactFloating";
		mi.name.a = LPGEN("&Floating Contact");
		if (pcli) {
			if (SendMessage(pcli->hwndContactTree, CLM_QUERYFLOATINGCONTACT, wParam, 0))
				mi.flags=CMIF_CHECKED;
		}
		hFloatingItem = Menu_AddContactMenuItem(&mi);
	}
	else if (!bSetting && hFloatingItem) {
		//CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hFloatingItem, 0);
		hFloatingItem = 0;
	}
	else {
		if (pcli) {
			if (SendMessage(pcli->hwndContactTree, CLM_QUERYFLOATINGCONTACT, wParam, 0)) {
				mi.flags |= CMIF_CHECKED;
				Menu_SetChecked(hFloatingItem, true);
			}
		}
	}
	return 0;
}

int MTG_OnmodulesLoad(WPARAM wParam,LPARAM lParam)
{
	hOnCntMenuBuild=HookEvent(ME_CLIST_PREBUILDCONTACTMENU,OnContactMenuBuild);
	return 0;
}

int UnloadMoveToGroup(void)
{
	if (hOnCntMenuBuild)
		UnhookEvent(hOnCntMenuBuild);

	return 0;
}
