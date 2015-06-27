/*
  Name: NewEventNotify - Plugin for Miranda IM
  File: mi.c - Manages item(s) in the Miranda Menu
  Version: 0.0.4
  Description: Notifies you about some events
  Author: icebreaker, <icebreaker@newmail.net>
  Date: 22.07.02 19:56 / Update: 24.07.02 01:39
  Copyright: (C) 2002 Starzinger Michael

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "neweventnotify.h"

HGENMENU hMenuitemNotify;
BOOL bNotify;

static INT_PTR MenuitemNotifyCmd(WPARAM wParam,LPARAM lParam)
{
	bNotify = !bNotify;
	MenuitemUpdate(bNotify);

	//write changes to options->bDisable and into database
	Opt_DisableNEN(!bNotify);
	return 0;
}

int MenuitemUpdate(BOOL bStatus)
{
	if (bStatus)
		Menu_ModifyItem(hMenuitemNotify, _T(MENUITEM_DISABLE), LoadIcon(hInst, MAKEINTRESOURCE(IDI_ENABLED)));
	else
		Menu_ModifyItem(hMenuitemNotify, _T(MENUITEM_ENABLE), LoadIcon(hInst, MAKEINTRESOURCE(IDI_DISABLED)));
	return 0;
}

int MenuitemInit(BOOL bStatus)
{
	CreateServiceFunction(MS_NEN_MENUNOTIFY, MenuitemNotifyCmd);

	CLISTMENUITEM mi = { 0 };
	mi.hParentMenu = Menu_CreateRoot(MO_MAIN, LPGENT("Popups"), 0);
	mi.position = 1;
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ENABLED));
	mi.pszService = MS_NEN_MENUNOTIFY;
	mi.flags = 0;
	hMenuitemNotify = Menu_AddMainMenuItem(&mi);

	bNotify = bStatus;
	MenuitemUpdate(bNotify);
	return 0;
}
