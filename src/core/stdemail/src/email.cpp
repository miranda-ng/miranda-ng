/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

static HGENMENU hEMailMenuItem;

void __cdecl SendEmailThread(char *szUrl)
{
	ShellExecuteA(nullptr, "open", (char*)szUrl, "", "", SW_SHOW);
	mir_free(szUrl);
	return;
}

static INT_PTR SendEMailCommand(WPARAM hContact, LPARAM lParam)
{
	DBVARIANT dbv;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr || db_get_s(hContact, szProto, "e-mail", &dbv)) {
		if (db_get_s(hContact, "UserInfo", "Mye-mail0", &dbv)) {
			MessageBox((HWND)lParam, TranslateT("User has not registered an e-mail address"), TranslateT("Send e-mail"), MB_OK);
			return 1;
		}
	}
	char *szUrl = (char*)mir_alloc(mir_strlen(dbv.pszVal)+8);
	mir_strcpy(szUrl, "mailto:");
	mir_strcat(szUrl, dbv.pszVal);
	mir_free(dbv.pszVal);
	mir_forkThread<char>(SendEmailThread, szUrl);
	return 0;
}

static int EMailPreBuildMenu(WPARAM hContact, LPARAM)
{
	bool bEnabled = true;
	DBVARIANT dbv = { 0 };
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr || db_get_s(hContact, szProto, "e-mail", &dbv))
		if (db_get_s(hContact, "UserInfo", "Mye-mail0", &dbv))
			bEnabled = false;

	Menu_ShowItem(hEMailMenuItem, bEnabled);
	if (dbv.pszVal) db_free(&dbv);
	return 0;
}

int LoadSendRecvEMailModule(void)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x61d8e25a, 0x92e, 0x4470, 0x84, 0x57, 0x5e, 0x52, 0x17, 0x7f, 0xfa, 0x3);
	mi.position = -2000010000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SENDEMAIL);
	mi.name.a = LPGEN("&E-mail");
	mi.pszService = MS_EMAIL_SENDEMAIL;
	hEMailMenuItem = Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_EMAIL_SENDEMAIL, SendEMailCommand);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, EMailPreBuildMenu);
	return 0;
}
