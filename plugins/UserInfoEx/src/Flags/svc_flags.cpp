/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

part of this code based on:
Miranda IM Country Flags Plugin Copyright �2006-2007 H. Herkenrath

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

#include "..\commonheaders.h"

#define M_ENABLE_SUBCTLS			(WM_APP+1)

bool g_bUseUnknownFlag = false, g_bShowStatusIconFlag = false;

/* Misc */
int		nCountriesCount;
struct	CountryListEntry *countries;
static	HANDLE hExtraIconSvc;

static int OnContactSettingChanged(WPARAM wParam,LPARAM lParam);

static LIST<MsgWndData> gMsgWndList(10, NumericKeySortT);

/***********************************************************************************************************
 * service functions
 ***********************************************************************************************************/

static INT_PTR ServiceDetectContactOriginCountry(WPARAM wParam,LPARAM lParam)
{
	WORD countryNumber;
	char *pszProto = GetContactProto(wParam);
	/* UserinfoEx */
	if (countryNumber = db_get_w(wParam, USERINFO, SET_CONTACT_ORIGIN_COUNTRY, 0))
		return countryNumber;
	if (countryNumber = db_get_w(wParam, USERINFO, SET_CONTACT_COUNTRY, 0))
		return countryNumber;
	if (countryNumber = db_get_w(wParam, USERINFO, SET_CONTACT_COMPANY_COUNTRY, 0))
		return countryNumber;
	/* fallback proto settings */
	if (countryNumber = db_get_w(wParam, pszProto, "Country", 0))
		return countryNumber;
	if (countryNumber = db_get_w(wParam, pszProto, "CompanyCountry", 0))
		return countryNumber;

	return (INT_PTR)0xFFFF;
}

/***********************************************************************************************************
 * Clist Extra Image functions
 ***********************************************************************************************************/

static void CALLBACK SetExtraImage(LPARAM lParam)
{
	/* get contact's country */
	int countryNumber = ServiceDetectContactOriginCountry(lParam, 0);
	ExtraIcon_SetIcon(hExtraIconSvc, lParam, (countryNumber != 0xFFFF || g_bUseUnknownFlag) ? LoadFlagHandle(countryNumber) : NULL);
}

static int OnCListApplyIcons(WPARAM wParam, LPARAM)
{
	SetExtraImage(wParam);
	return 0;
}

/***********************************************************************************************************
 * message winsow status icon functions
 ***********************************************************************************************************/

MsgWndData::MsgWndData(HWND hwnd, MCONTACT hContact)
{
	m_hwnd = hwnd;
	m_hContact = hContact;
	m_countryID = (int)ServiceDetectContactOriginCountry((WPARAM)m_hContact, 0);

	FlagsIconSet();
}

MsgWndData::~MsgWndData()
{
}

void MsgWndData::FlagsIconSet()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODNAMEFLAGS;
	if (!g_bShowStatusIconFlag)
		sid.flags = MBF_HIDDEN;
	if (m_countryID != 0xFFFF || g_bUseUnknownFlag) {
		sid.hIcon = LoadFlagIcon(m_countryID);
		sid.szTooltip = Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, m_countryID, 0));
	}
	else sid.flags = MBF_HIDDEN;
	
	Srmm_ModifyIcon(m_hContact, &sid);
}

void UpdateStatusIcons()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODNAMEFLAGS;
	if (!g_bShowStatusIconFlag)
		sid.flags = MBF_HIDDEN;
	Srmm_ModifyIcon(NULL, &sid);

	/* enum all opened message windows */
	for (int i = 0; i < gMsgWndList.getCount(); i++)
		gMsgWndList[i]->FlagsIconSet();
}

//hookProc ME_MSG_WINDOWEVENT
static int OnMsgWndEvent(WPARAM wParam, LPARAM lParam)
{
	MsgWndData *msgwnd;
	MessageWindowEventData *msgwe = (MessageWindowEventData*)lParam;
	/* sanity check */
	if (msgwe->hContact == NULL)
		return 0;

	switch (msgwe->uType) {
	case MSG_WINDOW_EVT_OPENING:
		msgwnd = gMsgWndList.find((MsgWndData*)&msgwe->hContact);
		if (msgwnd == NULL) {
			msgwnd = new MsgWndData(msgwe->hwndWindow, msgwe->hContact);
			gMsgWndList.insert(msgwnd);
		}
		break;

	case MSG_WINDOW_EVT_CLOSE:
		int i = gMsgWndList.getIndex((MsgWndData*)&msgwe->hContact);
		if (i != -1) {
			delete gMsgWndList[i];
			gMsgWndList.remove(i);
		}
		break;
	}
	return 0;
}

// hookProc ME_SKIN2_ICONSCHANGED
static int OnStatusIconsChanged(WPARAM wParam, LPARAM lParam)
{
	UpdateStatusIcons();
	return 0;
}

/***********************************************************************************************************
 * misc functions
 ***********************************************************************************************************/

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	/* user details update */
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
	if (!lstrcmpA(dbcws->szSetting, SET_CONTACT_COUNTRY) ||
		 !lstrcmpA(dbcws->szSetting, SET_CONTACT_ORIGIN_COUNTRY) ||
		 !lstrcmpA(dbcws->szSetting, SET_CONTACT_COMPANY_COUNTRY))
	{
		/* Extra Image */
		SetExtraImage(hContact);

		/* Status Icon */
		int i = gMsgWndList.getIndex((MsgWndData*)&hContact);
		if (i != -1) {
			gMsgWndList[i]->ContryIDchange((int)ServiceDetectContactOriginCountry(hContact, 0));
			gMsgWndList[i]->FlagsIconSet();
		}
	}
	return 0;
}

/***********************************************************************************************************
 * module loading & unloading
 ***********************************************************************************************************/

/**
* This function initially loads all required stuff for Flags.
*
* @param	none
*
* @return	nothing
**/

void SvcFlagsLoadModule()
{
	if (CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&nCountriesCount, (LPARAM)&countries))
		nCountriesCount = 0;
	InitIcons();			/* load in iconlib */
	
	CreateServiceFunction(MS_FLAGS_DETECTCONTACTORIGINCOUNTRY, ServiceDetectContactOriginCountry);
	
	// init settings
	g_bUseUnknownFlag = db_get_b(NULL, MODNAMEFLAGS, "UseUnknownFlag", SETTING_USEUNKNOWNFLAG_DEFAULT) != 0;
	g_bShowStatusIconFlag = db_get_b(NULL, MODNAMEFLAGS, "ShowStatusIconFlag", SETTING_SHOWSTATUSICONFLAG_DEFAULT) != 0;

	HookEvent(ME_SKIN2_ICONSCHANGED, OnStatusIconsChanged);

	// get local langID for descIcon (try to use user local Flag as icon)
	DWORD langid = 0;
	int r = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICOUNTRY | LOCALE_RETURN_NUMBER, (LPTSTR)&langid, sizeof(langid) / sizeof(TCHAR));
	if (!CallService(MS_UTILS_GETCOUNTRYBYNUMBER, langid, 0))
		langid = 1;

	char szId[20];
	mir_snprintf(szId, SIZEOF(szId), (langid == 0xFFFF) ? "%s_0x%X" : "%s_%i", "flags", langid); /* buffer safe */
	hExtraIconSvc = ExtraIcon_Register("Flags", LPGEN("Flags (uinfoex)"), szId);
}

/**
* This function is called by Miranda just after loading all system modules.
*
* @param	none
*
* @return	nothing
**/

void SvcFlagsOnModulesLoaded()
{
	/* Status Icon */
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODNAMEFLAGS;
	if (!g_bShowStatusIconFlag)
		sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnCListApplyIcons);
	HookEvent(ME_MSG_WINDOWEVENT, OnMsgWndEvent);
}

/**
* This function unloads the module.
*
* @param	none
*
* @return	nothing
**/

void SvcFlagsUnloadModule()
{
	//Uninit message winsow
	for (int i = 0; i < gMsgWndList.getCount(); i++) {
		//this should not happen
		delete gMsgWndList[i];
		gMsgWndList.remove(i);
	}
	gMsgWndList.destroy();

	// Uninit misc
	UninitIcons();
}
