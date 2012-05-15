/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "commonheaders.h"

static HANDLE hEMailMenuItem;

void SendEmailThread(void *szUrl)
{
	ShellExecuteA(NULL,"open",( char* )szUrl,"","",SW_SHOW);
	mir_free(szUrl);
	return;
}

static INT_PTR SendEMailCommand(WPARAM wParam,LPARAM lParam)
{
	DBVARIANT dbv;
	char *szUrl;
	char *szProto;

	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	if(szProto==NULL || DBGetContactSettingString((HANDLE)wParam,szProto,"e-mail",&dbv)) {
		if(DBGetContactSettingString((HANDLE)wParam,"UserInfo","Mye-mail0",&dbv)) {
			MessageBox((HWND)lParam,TranslateT("User has not registered an e-mail address"),TranslateT("Send e-mail"),MB_OK);
			return 1;
		}
	}
	szUrl=(char*)mir_alloc(lstrlenA(dbv.pszVal)+8);
	lstrcpyA(szUrl,"mailto:");
	lstrcatA(szUrl,dbv.pszVal);
	mir_free(dbv.pszVal);
	forkthread(SendEmailThread,0,szUrl);
	return 0;
}

static int EMailPreBuildMenu(WPARAM wParam, LPARAM)
{
	CLISTMENUITEM mi;
	DBVARIANT dbv = { 0 };
	char *szProto;

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;

	szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (szProto == NULL || DBGetContactSettingString((HANDLE)wParam, szProto, "e-mail",& dbv))
		if (DBGetContactSettingString((HANDLE)wParam, "UserInfo", "Mye-mail0", &dbv))
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hEMailMenuItem, (LPARAM)&mi);
	if (dbv.pszVal) DBFreeVariant(&dbv);
	return 0;
}

int LoadSendRecvEMailModule(void)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = -2000010000;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle( SKINICON_OTHER_SENDEMAIL );
	mi.pszName = LPGEN("&E-mail");
	mi.pszService = MS_EMAIL_SENDEMAIL;
	hEMailMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	CreateServiceFunction(MS_EMAIL_SENDEMAIL, SendEMailCommand);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, EMailPreBuildMenu);
	return 0;
}
