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
*/

#include "commonheaders.h"

static HGENMENU ghMenuItem = NULL;
static HANDLE ghExtraIconDef = INVALID_HANDLE_VALUE;
static HANDLE ghExtraIconSvc = INVALID_HANDLE_VALUE;

static HANDLE hChangedHook = NULL;
static HANDLE hApplyIconHook = NULL;

bool g_eiEmail = false;

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
		LPCSTR pszProto = DB::Contact::Proto(hContact);
		
		if (pszProto != NULL) {
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
	int result;
	LPSTR val = NULL;

	__try 
	{
		val = Get(wParam);
		if (val) {
			LPSTR szUrl;
			INT_PTR len;

			len = mir_strlen(val) + strlen("mailto:");

			szUrl = (LPSTR)_alloca(len + 1);

			mir_snprintf(szUrl, len + 1, "mailto:%s", val);
			mir_free(val);

			result = CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)szUrl);
		}
		else {
			result = 1;
			MsgBox((HWND)lParam, MB_OK, LPGENT("Send e-mail"), NULL, LPGENT("Memory allocation error!"));
		}
	}
	__except(GetExceptionCode()==EXCEPTION_ACCESS_VIOLATION ? 
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
		mir_free(val);
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

static int OnCListApplyIcons(WPARAM wParam, LPARAM lParam)
{
	LPSTR val = Get(wParam);
	ExtraIcon_SetIcon(ghExtraIconSvc, wParam, (val) ? ICO_BTN_EMAIL : 0);
	mir_free(val);
	return 0;
}

/**
* Notification handler for changed contact settings
*
* @param	wParam			- (HANDLE)hContact
* @param	lParam			- (DBCONTACTWRITESETTING*)pdbcws
**/

static int OnContactSettingChanged(MCONTACT hContact, DBCONTACTWRITESETTING* pdbcws)
{
	if (hContact && pdbcws && pdbcws->szSetting && 
			((pdbcws->value.type & DBVTF_VARIABLELENGTH) || (pdbcws->value.type == DBVT_DELETED)) &&
			(!mir_strncmp(pdbcws->szSetting, SET_CONTACT_EMAIL, 6) ||
			 !mir_strncmp(pdbcws->szSetting, SET_CONTACT_COMPANY_EMAIL, 13) ||
			 !mir_strncmp(pdbcws->szSetting, "mye-mail0", 9)))
	{
		OnCListApplyIcons(hContact, 0);
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

static int OnPreBuildMenu(WPARAM wParam, LPARAM lParam)
{
	LPSTR val = Get(wParam);
	Menu_ShowItem(ghMenuItem, val != NULL);
	mir_free(val);
	return 0;
}

/***********************************************************************************************************
 * public Module Interface functions
 ***********************************************************************************************************/

void SvcEMailRebuildMenu()
{
	static HANDLE hPrebuildMenuHook = NULL;

	if (db_get_b(NULL, MODNAME, SET_EXTENDED_EMAILSERVICE, TRUE)) {
		if (!hPrebuildMenuHook) 
			hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildMenu);

		if (!ghMenuItem) {
			// insert contact menuitem
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.position = -2000010000;
			mi.hIcon = Skin_GetIcon(ICO_BTN_EMAIL);
			mi.pszName = "&E-mail";
			mi.pszService = MS_EMAIL_SENDEMAIL;
			ghMenuItem = Menu_AddContactMenuItem(&mi);
		}
	}
	else {
		if (hPrebuildMenuHook)
			UnhookEvent(ME_CLIST_PREBUILDCONTACTMENU), hPrebuildMenuHook = NULL;

		if (ghMenuItem) {
			CallService(MO_REMOVEMENUITEM, (WPARAM)ghMenuItem, NULL);
			ghMenuItem = NULL;
		}
	}
}

/**
* Enable or disable the replacement of clist extra icons.
*
* @param	bEnable			- determines whether icons are enabled or not
* @param	bUpdateDB		- if true the database setting is updated, too.
**/

bool SvcEMailEnableExtraIcons(bool bEnable, bool bUpdateDB)
{
	bool bChanged;

	if (bUpdateDB) {
		bChanged = g_eiEmail != bEnable;
		db_set_b(NULL, MODNAME, SET_CLIST_EXTRAICON_EMAIL, g_eiEmail = bEnable);
	}
	else bChanged = g_eiEmail = db_get_b(NULL, MODNAME, SET_CLIST_EXTRAICON_EMAIL, DEFVAL_CLIST_EXTRAICON_EMAIL) != 0;

	if (g_eiEmail) { // E-mail checked
		// hook events
		if (hChangedHook == NULL) 
			hChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOK)OnContactSettingChanged);

		if (hApplyIconHook == NULL) 
			hApplyIconHook = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnCListApplyIcons);

		if (ghExtraIconSvc == INVALID_HANDLE_VALUE)
			ghExtraIconSvc = ExtraIcon_Register("email", LPGEN("E-mail (uinfoex)"), ICO_BTN_EMAIL);
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
	}

	return bChanged;
}

/**
* This function initially loads the module upon startup.
**/

void SvcEMailLoadModule()
{
	SvcEMailEnableExtraIcons();
	if (db_get_b(NULL, MODNAME, SET_EXTENDED_EMAILSERVICE, TRUE)) {
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

void SvcEMailUnloadModule()
{	
	// unhook event handlers
	UnhookEvent(hChangedHook); hChangedHook = NULL;
	UnhookEvent(hApplyIconHook); hApplyIconHook = NULL;
}
