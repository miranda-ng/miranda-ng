/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"	

static HGENMENU ghMenuItem = nullptr;	
static HANDLE hPrebuildMenuHook = nullptr;	

/**	
* This function reads the homepage address of the contact.	
*	
* @param	hContact	- handle to contact to read email from	
*	
* @retval	URL to contacts homepage	
* @retval	NULL if contact provides no homepage	
**/	

static LPSTR Get(MCONTACT hContact)	
{	
	// ignore owner	
	if (hContact != NULL) {	
		LPCSTR pszProto = Proto_GetBaseAccountName(hContact);	
		if (pszProto != nullptr) {	
			LPCSTR e[2] = { SET_CONTACT_HOMEPAGE, SET_CONTACT_COMPANY_HOMEPAGE };	
			for (int i = 0; i < 2; i++) {	
				LPSTR pszHomepage = DB::Setting::GetAStringEx(hContact, USERINFO, pszProto, e[i]);	
				if (pszHomepage)	
					return pszHomepage;	
			}	
		}	
	}	
	return nullptr;	
}	

/**	
* Service function that opens the default browser and displays the homepage.	
*	
* @param	wParam		- handle to contact to send an email to	
* @param	lParam		- not used	
*	
* @retval	0 if email was sent	
* @retval	1 if no email can be sent	
**/	

static INT_PTR MenuCommand(WPARAM wParam, LPARAM lParam)	
{	
	LPSTR szUrl = Get(wParam);	
	if (szUrl) {	
		Utils_OpenUrl(szUrl);	
		mir_free(szUrl);	
	}	
	else MessageBox((HWND)lParam, TranslateT("User has no valid homepage"), TranslateT("View Homepage"), MB_OK);	

	return 0;	
}	

/***********************************************************************************************************	
 * Event Handler functions	
 ***********************************************************************************************************/	

/**	
* This function decides whether to show menuitem for sending emails or not.	
*	
* @param	wParam		- handle to contact to send an email to	
* @param	lParam		- not used	
*	
* @return	always 0	
**/	

static int OnPreBuildMenu(WPARAM wParam, LPARAM)	
{	
	LPSTR val = Get(wParam);	
	Menu_ShowItem(ghMenuItem, val != nullptr);	
	mir_free(val);	
	return 0;	
}	

/***********************************************************************************************************	
 * public Module Interface functions	
 ***********************************************************************************************************/	

/**	
* enable or disable menuitem	
*	
* @param	not used	
* @return	nothing	
**/	

void SvcHomepageRebuildMenu()	
{	
	if (!hPrebuildMenuHook)	
		hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildMenu);	

	if (!ghMenuItem) {	
		// insert contact menuitem	
		CMenuItem mi(&g_plugin);	
		SET_UID(mi, 0xdb78c16e, 0x50db, 0x4a40, 0x80, 0x0, 0xd1, 0xa4, 0x1c, 0x1b, 0xa3, 0x2d);	
		mi.position = -2000010000;	
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_GOTO);	
		mi.name.a = LPGEN("&Homepage");	
		mi.pszService = MS_USERINFO_HOMEPAGE_OPENURL;	
		ghMenuItem = Menu_AddContactMenuItem(&mi);	
		CreateServiceFunction(mi.pszService, MenuCommand);
	}
}	
