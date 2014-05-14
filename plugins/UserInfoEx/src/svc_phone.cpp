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

enum EPhoneType 
{
	PHONE_NONE,
	PHONE_NORMAL,
	PHONE_SMS
};

static HANDLE ghMenuItem = NULL;
static HANDLE ghExtraIconDef[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
static HANDLE ghExtraIconSvc = INVALID_HANDLE_VALUE;

static HANDLE hChangedHook = NULL;
static HANDLE hApplyIconHook = NULL;

bool g_eiPhone = false;

/**
* This function reads the contact's phone number from database and returns its type.
*
* @param	 hContact		- handle to contact to read email from
*
* @retval	PHONE_SMS:		The phone supports sms, so is a cellular
* @retval	PHONE_NORMAL:	The phone is a normal phone
* @retval	PHONE_NONE:		The contact does not provide any phone number
**/

static INT_PTR Get(MCONTACT hContact)
{
	INT_PTR nType = PHONE_NONE;

	// ignore owner
	if (hContact != NULL) {
		LPCSTR pszProto = DB::Contact::Proto(hContact);
		if (pszProto != NULL) {
			LPCSTR e[2][4] = {
				{ SET_CONTACT_CELLULAR,         SET_CONTACT_PHONE,         "MyPhone0"        },
				{ SET_CONTACT_COMPANY_CELLULAR, SET_CONTACT_COMPANY_PHONE, "MyCompanyPhone0" }
			};

			for (int i = 0; (i < 2) && (nType == PHONE_NONE); i++) {
				for (int j = 0; (j < 3) && (nType == PHONE_NONE); j++) {
					LPSTR pszPhone = DB::Setting::GetAStringEx(hContact, USERINFO, pszProto, e[i][j]);
					if (pszPhone) {
						nType = (strstr(pszPhone, " SMS")) ? PHONE_SMS : PHONE_NORMAL;
						MIR_FREE(pszPhone);
						break;
					}
				}
			}
		}
	}
	return nType;
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

static int OnCListApplyIcons(MCONTACT hContact, LPARAM)
{
	char *icoName;
	switch (Get(hContact)) {
		case PHONE_NORMAL:  icoName = ICO_BTN_PHONE;     break;
		case PHONE_SMS:     icoName = ICO_BTN_CELLULAR;  break;
		default:            icoName = NULL;
	}
	ExtraIcon_SetIcon(ghExtraIconSvc, hContact, icoName);
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
			(!strcmp(pdbcws->szSetting, SET_CONTACT_PHONE) ||
			 !strcmp(pdbcws->szSetting, SET_CONTACT_CELLULAR) ||
			 !strcmp(pdbcws->szSetting, SET_CONTACT_COMPANY_PHONE) ||
			 !strcmp(pdbcws->szSetting, SET_CONTACT_COMPANY_CELLULAR) ||
			 !strncmp(pdbcws->szSetting, "MyPhone0", 8)))
		OnCListApplyIcons(hContact, 0);

	return 0;
}

/**
* Enable or disable the replacement of clist extra icons.
*
* @param	bEnable			- determines whether icons are enabled or not
* @param	bUpdateDB		- if true the database setting is updated, too.
**/

bool SvcPhoneEnableExtraIcons(bool bEnable, bool bUpdateDB)
{
	bool bChanged;

	if (bUpdateDB) {
		bChanged = g_eiPhone != bEnable;
		db_set_b(NULL, MODNAME, SET_CLIST_EXTRAICON_PHONE, g_eiPhone = bEnable);
	}
	else bChanged = g_eiPhone = db_get_b(NULL, MODNAME, SET_CLIST_EXTRAICON_PHONE, DEFVAL_CLIST_EXTRAICON_PHONE) != 0;

	// force module enabled, if extraicon plugin was found
	if (g_eiPhone) {
		// hook events
		if (hChangedHook == NULL) 
			hChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOK)OnContactSettingChanged);

		if (hApplyIconHook == NULL) 
			hApplyIconHook = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, (MIRANDAHOOK)OnCListApplyIcons);

		if (ghExtraIconSvc == INVALID_HANDLE_VALUE)
			ghExtraIconSvc = ExtraIcon_Register("sms", LPGEN("Phone (uinfoex)"), ICO_BTN_CELLULAR);
	}
	else {
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
* This function unloads the Email module.
*
* @param	none
*
* @return	nothing
**/

void SvcPhoneUnloadModule()
{	
	// unhook event handlers
	UnhookEvent(hChangedHook); hChangedHook = NULL;
	UnhookEvent(hApplyIconHook); hApplyIconHook = NULL;
}
