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

static HANDLE ghExtraIconF = INVALID_HANDLE_VALUE;
static HANDLE ghExtraIconM = INVALID_HANDLE_VALUE;
static HANDLE ghExtraIconSvc = INVALID_HANDLE_VALUE;

static HANDLE hChangedHook = NULL;
static HANDLE hApplyIconHook = NULL;

bool g_eiGender = false;

BYTE GenderOf(MCONTACT hContact, LPCSTR pszProto)
{
	DBVARIANT dbv;
	if (DB::Setting::GetAsIsEx(hContact, USERINFO, pszProto, SET_CONTACT_GENDER, &dbv) == 0) {
		// gender must be byte and either M or F
		if (dbv.type == DBVT_BYTE && (dbv.bVal == 'M' || dbv.bVal == 'F'))
			return dbv.bVal;	

		db_free(&dbv);
	}
	return 0;
}

/**
* This function gets the gender of the contact from the database.
*
* @param	hContact		- handle to contact to read email from
*
* @retval	F	- contact is female
* @retval	M	- contact is male
* @retval	0	- contact does not provide its gender
**/

BYTE GenderOf(MCONTACT hContact)
{
	return GenderOf(hContact, DB::Contact::Proto(hContact));
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
	if (ghExtraIconSvc != INVALID_HANDLE_VALUE) {
		char *icoName;
		switch (GenderOf(hContact)) {
			case 'M':  icoName = ICO_COMMON_MALE;   break;
			case 'F':  icoName = ICO_COMMON_FEMALE; break;
			default:   icoName = NULL;
		}
		ExtraIcon_SetIcon(ghExtraIconSvc, hContact, icoName);
	}
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
	if (hContact && pdbcws && (pdbcws->value.type <= DBVT_BYTE) && !mir_strcmp(pdbcws->szSetting, SET_CONTACT_GENDER))
		OnCListApplyIcons(hContact, 0);

	return 0;
}

/**
* Enable or disable the replacement of clist extra icons.
*
* @param	bEnable			- determines whether icons are enabled or not
* @param	bUpdateDB		- if true the database setting is updated, too.
**/

bool SvcGenderEnableExtraIcons(bool bEnable, bool bUpdateDB) 
{
	bool bChanged;

	if (bUpdateDB) {
		bChanged = g_eiGender != bEnable;
		db_set_b(NULL, MODNAME, SET_CLIST_EXTRAICON_GENDER2, g_eiGender = bEnable);
	}
	else bChanged = g_eiGender = db_get_b(NULL, MODNAME, SET_CLIST_EXTRAICON_GENDER2, 0) != 0;

	if (g_eiGender) { // Gender checked or dropdown select
		if (ghExtraIconSvc == INVALID_HANDLE_VALUE)
			ghExtraIconSvc = ExtraIcon_Register("gender", LPGEN("Gender (uinfoex)"), ICO_COMMON_MALE);

		// hook events
		if (hChangedHook == NULL) 
			hChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOK)OnContactSettingChanged);

		if (hApplyIconHook == NULL) 
			hApplyIconHook = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, (MIRANDAHOOK)OnCListApplyIcons);
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
* This function unloads the module.
*
* @param	none
*
* @return	nothing
**/

void SvcGenderUnloadModule()
{	
	// unhook event handlers
	UnhookEvent(hChangedHook);	hChangedHook = NULL;
	UnhookEvent(hApplyIconHook); hApplyIconHook = NULL;
}
