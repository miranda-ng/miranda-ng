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
* This function reads the email address of the contact.	
*	
* @param	hContact		- handle to contact to read email from	
*	
* @retval	email address	
* @retval	NULL, if contact does not provide any email address	
**/	

static LPSTR Get(MCONTACT hContact)	
{	
	// ignore owner	
	if (hContact != NULL) {	
		LPCSTR pszProto = Proto_GetBaseAccountName(hContact);	

		if (pszProto != nullptr) {	
			LPCSTR e[2][4] = {	
				{ SET_CONTACT_EMAIL,			SET_CONTACT_EMAIL0,			SET_CONTACT_EMAIL1,			"Mye-mail0"},	
				{ SET_CONTACT_COMPANY_EMAIL,	SET_CONTACT_COMPANY_EMAIL0,	SET_CONTACT_COMPANY_EMAIL1,	"MyCompanye-mail0"}	
			};	

			for (int i = 0; i < 2; i++) {	
				for (int j = 0; j < 4; j++) {	
					LPSTR pszEMail = DB::Setting::GetAStringEx(hContact, USERINFO, pszProto, e[i][j]);	
					if (pszEMail) {	
						if (strchr(pszEMail, '@'))	
							return pszEMail;	

						mir_free(pszEMail);	
					}	
				}	
			}	
		}	
	}	
	return nullptr;	
}	

/**	
* Service function that sends emails	
*	
* @param	wParam			- handle to contact to send an email to	
* @param	lParam			- not used	
*	
* @retval	0 if email was sent	
* @retval	1 if no email can be sent	
**/	

static INT_PTR MenuCommand(WPARAM wParam,LPARAM lParam)	
{	
	int result = 0;	
	LPSTR val = nullptr;	

	__try 	
	{	
		val = Get(wParam);	
		if (val) {	
			LPSTR szUrl;	
			INT_PTR len;	

			len = mir_strlen(val) + mir_strlen("mailto:");	

			szUrl = (LPSTR)_alloca(len + 1);	

			mir_snprintf(szUrl, len + 1, "mailto:%s", val);	
			mir_free(val);	

			Utils_OpenUrl(szUrl);	
		}	
		else {	
			result = 1;	
			MsgBox((HWND)lParam, MB_OK, LPGENW("Send e-mail"), nullptr, LPGENW("Memory allocation error!"));	
		}	
	}	
	__except(GetExceptionCode()==EXCEPTION_ACCESS_VIOLATION ? 	
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 	
	{	
		mir_free(val);	
		result = 1;	
		MsgErr((HWND)lParam, LPGENW("Memory allocation error!"));	
	}	
	return result;	
}	

/***********************************************************************************************************	
 * Event Handler functions	
 ***********************************************************************************************************/	

/**	
* This function decides whether to show menuitem for sending emails or not.	
*	
* @param	wParam			- handle to contact to send an email to	
* @param	lParam			- not used	
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

void SvcEMailRebuildMenu()	
{	
	if (g_plugin.bEmailService) {	
		if (!hPrebuildMenuHook) 	
			hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildMenu);	

		if (!ghMenuItem) {	
			// insert contact menuitem	
			CMenuItem mi(&g_plugin);	
			SET_UID(mi, 0x61d8e25a, 0x92e, 0x4470, 0x84, 0x57, 0x5e, 0x52, 0x17, 0x7f, 0xfa, 0x3);	
			mi.position = -2000010000;	
			mi.hIcolibItem = g_plugin.getIconHandle(IDI_DLG_EMAIL);	
			mi.name.a = "&E-mail";	
			mi.pszService = MS_EMAIL_SENDEMAIL;	
			ghMenuItem = Menu_AddContactMenuItem(&mi);	
		}	
	}	
	else {	
		if (hPrebuildMenuHook)	
			UnhookEvent(ME_CLIST_PREBUILDCONTACTMENU), hPrebuildMenuHook = NULL;	

		if (ghMenuItem) {	
			Menu_RemoveItem(ghMenuItem);	
			ghMenuItem = nullptr;	
		}	
	}	
}	

/**	
* This function initially loads the module upon startup.	
**/	

void SvcEMailLoadModule()	
{	
	if (g_plugin.bEmailService) {	
		// create own email send command	
		DestroyServiceFunction(MS_EMAIL_SENDEMAIL);	
		CreateServiceFunction(MS_EMAIL_SENDEMAIL, MenuCommand);	
	}	
}	
