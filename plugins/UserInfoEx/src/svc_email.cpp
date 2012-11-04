/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/svc_email.cpp $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/
#include "commonheaders.h"

static HANDLE ghMenuItem			= NULL;
static HANDLE ghExtraIconDef		= INVALID_HANDLE_VALUE;
static HANDLE ghExtraIconSvc		= INVALID_HANDLE_VALUE;

static HANDLE hChangedHook			= NULL;
static HANDLE hApplyIconHook		= NULL;
static HANDLE hRebuildIconsHook		= NULL;

/**
 * This function reads the email address of the contact.
 *
 * @param	hContact		- handle to contact to read email from
 *
 * @retval	email address
 * @retval	NULL, if contact does not provide any email address
 **/
static LPSTR Get(HANDLE hContact)
{
	// ignore owner
	if (hContact != NULL) 
	{
		LPCSTR pszProto = DB::Contact::Proto(hContact);
		
		if (pszProto != NULL) 
		{
			LPCSTR	e[2][4] = {
				{ SET_CONTACT_EMAIL,			SET_CONTACT_EMAIL0,			SET_CONTACT_EMAIL1,			"Mye-mail0"},
				{ SET_CONTACT_COMPANY_EMAIL,	SET_CONTACT_COMPANY_EMAIL0,	SET_CONTACT_COMPANY_EMAIL1,	"MyCompanye-mail0"}
			};

			INT i, j;
			LPSTR pszEMail;

			for (i = 0; i < 2; i++)
			{
				for (j = 0; j < 4; j++)
				{
					pszEMail = DB::Setting::GetAStringEx(hContact, USERINFO, pszProto, e[i][j]);
					if (pszEMail)
					{
						if (strchr(pszEMail, '@'))
						{
							return pszEMail;
						}
						mir_free(pszEMail);
					}
				}
			}
		}
	}
	return NULL;
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
	INT result;
	LPSTR val = NULL;

	__try 
	{
		val = Get((HANDLE) wParam);
		if (val)
		{
			LPSTR szUrl;
			INT_PTR len;

			len = mir_strlen(val) + strlen("mailto:");

			szUrl = (LPSTR)_alloca(len + 1);

			mir_snprintf(szUrl, len + 1, "mailto:%s", val);
			mir_free(val);

			result = CallService(MS_UTILS_OPENURL, 1, (LPARAM)szUrl);
		}
		else
		{
			result = 1;
			MsgBox((HWND)lParam, MB_OK, LPGENT("Send e-mail"), NULL, LPGENT("Memory allocation error!"));
		}
	}
	__except(GetExceptionCode()==EXCEPTION_ACCESS_VIOLATION ? 
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
		if (val)
		{
			mir_free(val);
		}

		result = 1;
		MsgErr((HWND)lParam, LPGENT("Memory allocation error!"));
	}
	return result;
}

/***********************************************************************************************************
 * Event Handler functions
 ***********************************************************************************************************/

/**
 * Notification handler for clist extra icons to be applied for a contact.
 *
 * @param	wParam			- handle to the contact whose extra icon is to apply
 * @param	lParam			- not used
 **/
static INT OnCListApplyIcons(WPARAM wParam, LPARAM lParam)
{
	LPSTR val = Get((HANDLE)wParam);

	EXTRAICON ico = { sizeof(ico) };
	ico.hContact=(HANDLE)wParam;
	ico.hExtraIcon=ghExtraIconSvc;
	ico.icoName=val?ICO_BTN_EMAIL:(char *)0;
	mir_free(val);
	CallService(MS_EXTRAICON_SET_ICON, (WPARAM)&ico, 0);
	return 0;
}

/**
 * Notification handler for changed contact settings
 *
 * @param	wParam			- (HANDLE)hContact
 * @param	lParam			- (DBCONTACTWRITESETTING*)pdbcws
 **/
static INT OnContactSettingChanged(HANDLE hContact, DBCONTACTWRITESETTING* pdbcws)
{
	if (hContact && pdbcws && pdbcws->szSetting && 
			((pdbcws->value.type & DBVTF_VARIABLELENGTH) || (pdbcws->value.type == DBVT_DELETED)) &&
			(!mir_strncmp(pdbcws->szSetting, SET_CONTACT_EMAIL, 6) ||
			 !mir_strncmp(pdbcws->szSetting, SET_CONTACT_COMPANY_EMAIL, 13) ||
			 !mir_strncmp(pdbcws->szSetting, "mye-mail0", 9)))
	{
		OnCListApplyIcons((WPARAM)hContact, 0);
	}
	return 0;
}

/**
 * This function decides whether to show menuitem for sending emails or not.
 *
 * @param	wParam			- handle to contact to send an email to
 * @param	lParam			- not used
 *
 * @return	always 0
 **/
static INT OnPreBuildMenu(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi;
	LPSTR val;
	
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;

	val = Get((HANDLE)wParam);
	if (val) 
	{
		mir_free(val);
	}
	else 
	{
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)ghMenuItem, (LPARAM)&mi);
	return 0;
}

/***********************************************************************************************************
 * public Module Interface functions
 ***********************************************************************************************************/

/**
 * This function enables or disables menuitems.
 **/
VOID SvcEMailRebuildMenu()
{
	static HANDLE hPrebuildMenuHook = NULL;

	if (DB::Setting::GetByte(SET_EXTENDED_EMAILSERVICE, TRUE)) 
	{
		if (!hPrebuildMenuHook) 
		{
			hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildMenu);
		}

		if (!ghMenuItem) 
		{
			CLISTMENUITEM mi;

			// insert contact menuitem
			ZeroMemory(&mi, sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.position = -2000010000;
			mi.hIcon = IcoLib_GetIcon(ICO_BTN_EMAIL);
			mi.pszName = "&E-mail";
			mi.pszService = MS_EMAIL_SENDEMAIL;
			ghMenuItem = Menu_AddContactMenuItem(&mi);
		}
	}
	else 
	{
		if (hPrebuildMenuHook) 
		{
			UnhookEvent(ME_CLIST_PREBUILDCONTACTMENU);
			hPrebuildMenuHook = NULL;
		}
		if (ghMenuItem) 
		{
			CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)ghMenuItem, NULL);
			ghMenuItem = NULL;
		}
	}
}

/**
 * Force all icons to be reloaded.
 *
 * @param	wParam			- handle to the contact whose extra icon is to apply
 * @param	lParam			- not used
 **/
VOID SvcEMailApplyCListIcons()
{
	HANDLE hContact;

	//walk through all the contacts stored in the DB
	for (hContact = DB::Contact::FindFirst();	hContact != NULL;	hContact = DB::Contact::FindNext(hContact))
	{
		OnCListApplyIcons((WPARAM)hContact, 0);
	}
}

/**
 * Enable or disable the replacement of clist extra icons.
 *
 * @param	bEnable			- determines whether icons are enabled or not
 * @param	bUpdateDB		- if true the database setting is updated, too.
 **/
VOID SvcEMailEnableExtraIcons(BOOLEAN bEnable, BOOLEAN bUpdateDB) 
{
	if (bUpdateDB)
		DB::Setting::WriteByte(SET_CLIST_EXTRAICON_EMAIL, bEnable);

	if (bEnable) { // E-mail checkt
		// hook events
		if (hChangedHook == NULL) 
			hChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOK)OnContactSettingChanged);

		if (hApplyIconHook == NULL) 
			hApplyIconHook = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnCListApplyIcons);

		if (ghExtraIconSvc == INVALID_HANDLE_VALUE) {
			EXTRAICON_INFO ico = { sizeof(ico) };
			ico.type = EXTRAICON_TYPE_ICOLIB;
			ico.name = "email";	//must be the same as the group name in extraicon
			ico.description= "E-mail (uinfoex)";
			ico.descIcon = ICO_BTN_EMAIL;
			ghExtraIconSvc = (HANDLE)CallService(MS_EXTRAICON_REGISTER, (WPARAM)&ico, 0);
			ZeroMemory(&ico,sizeof(ico));
		}
	}
	else { // E-mail uncheckt
		if (hChangedHook) {
			UnhookEvent(hChangedHook); 
			hChangedHook = NULL;
		}			
		if (hApplyIconHook) {
			UnhookEvent(hApplyIconHook); 
			hApplyIconHook = NULL;
		}			
		if (hRebuildIconsHook) {
			UnhookEvent(hRebuildIconsHook); 
			hRebuildIconsHook = NULL;
		}
	}
	SvcEMailApplyCListIcons();
}

/**
 * This function initially loads the module uppon startup.
 **/
VOID SvcEMailOnModulesLoaded()
{
	SvcEMailEnableExtraIcons(
		DB::Setting::GetByte(SET_CLIST_EXTRAICON_EMAIL, 
		DEFVAL_CLIST_EXTRAICON_EMAIL), FALSE);
}

/**
 * This function initially loads the module uppon startup.
 **/
VOID SvcEMailLoadModule()
{
	if (DB::Setting::GetByte(SET_EXTENDED_EMAILSERVICE, TRUE)) {
		// create own email send command
		if (!myDestroyServiceFunction(MS_EMAIL_SENDEMAIL))
			CreateServiceFunction(MS_EMAIL_SENDEMAIL, MenuCommand);
	}
}

/**
 * This function unloads the Email module.
 *
 * @param	none
 *
 * @return	nothing
 **/
VOID SvcEMailUnloadModule()
{	
	// unhook event handlers
	UnhookEvent(hChangedHook);		hChangedHook		= NULL;
	UnhookEvent(hApplyIconHook);	hApplyIconHook		= NULL;
	UnhookEvent(hRebuildIconsHook);	hRebuildIconsHook	= NULL;
}
