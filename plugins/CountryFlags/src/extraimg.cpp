/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-1007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/* Services */
static HANDLE hServiceDetectContactOrigin;

/************************* Services *******************************/

static INT_PTR ServiceDetectContactOriginCountry(WPARAM hContact, LPARAM)
{
	int countryNumber = 0xFFFF;
	char *pszProto = Proto_GetBaseAccountName(hContact);
	/* ip detect */
	if (bUseIpToCountry)
		countryNumber = ServiceIpToCountry(db_get_dw(hContact, pszProto, "RealIP", 0), 0);
	/* fallback */
	if (countryNumber == 0xFFFF)
		countryNumber = db_get_w(hContact, pszProto, "Country", 0);
	if (countryNumber == 0 || countryNumber == 0xFFFF)
		countryNumber = db_get_w(hContact, pszProto, "CompanyCountry", 0);
	return (countryNumber == 0) ? 0xFFFF : countryNumber;
}

/************************* Extra Image ****************************/

#define EXTRAIMAGE_REFRESHDELAY  100  /* time for which setting changes are buffered */

static HANDLE hExtraIcon;

static void CALLBACK SetExtraImage(MCONTACT hContact)
{
	if (!bShowExtraIcon)
		return;

	int countryNumber = ServiceDetectContactOriginCountry(hContact, 0);
	if (countryNumber == 0xFFFF && !bUseUnknown)
		ExtraIcon_Clear(hExtraIcon, hContact);
	else {
		char szId[20];
		mir_snprintf(szId, (countryNumber == 0xFFFF) ? "%s0x%X" : "%s%i", "flags_", countryNumber);
		ExtraIcon_SetIconByName(hExtraIcon, hContact, szId);
	}
}

// always call in context of main thread
static void RemoveExtraImages(void)
{
	for (auto &hContact : Contacts())
		ExtraIcon_Clear(hExtraIcon, hContact);
}

// always call in context of main thread
static void EnsureExtraImages(void)
{
	for (auto &hContact : Contacts())
		SetExtraImage(hContact);
}

void UpdateExtraImages()
{
	if (bShowExtraIcon)
		EnsureExtraImages();
	else
		RemoveExtraImages();
}

/************************* Status Icon ****************************/

#define STATUSICON_REFRESHDELAY  100  /* time for which setting changes are buffered */

// always call in context of main thread
static void __fastcall SetStatusIcon(MCONTACT hContact, int countryNumber)
{
	if (countryNumber != 0xFFFF || bUseUnknown)
		/* copy icon as status icon API will call DestroyIcon() on it */
		Srmm_ModifyIcon(hContact, MODULENAME, 0, LoadFlagIcon(countryNumber), _A2T((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, countryNumber, 0)));
	else
		Srmm_SetIconFlags(hContact, MODULENAME, 0, MBF_HIDDEN);
}

static int MsgWndEvent(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;

	switch (uType) {
	case MSG_WINDOW_EVT_OPENING:
	case MSG_WINDOW_EVT_CLOSE:
		if (bShowStatusIcon) {
			int countryNumber = ServiceDetectContactOriginCountry(pDlg->m_hContact, 0);
			if (uType == MSG_WINDOW_EVT_OPENING && countryNumber != 0xFFFF)
				SetStatusIcon(pDlg->m_hContact, countryNumber);
			else
				Srmm_SetIconFlags(pDlg->m_hContact, MODULENAME, 0, MBF_HIDDEN);
		}
		// ensure it is hidden, RemoveStatusIcons() only enums currently opened ones
		else 
			Srmm_SetIconFlags(pDlg->m_hContact, MODULENAME, 0, MBF_HIDDEN);
	}
	return 0;
}

void CALLBACK UpdateStatusIcons(LPARAM)
{
	for (auto &hContact : Contacts()) {
		/* is a message window opened for this contact? */
		MessageWindowData msgw; /* output */
		if (!Srmm_GetWindowData(hContact, msgw) && msgw.uState & MSG_WINDOW_STATE_EXISTS) {
			if (bShowStatusIcon) {
				int countryNumber = ServiceDetectContactOriginCountry(hContact, 0);
				SetStatusIcon(hContact, countryNumber);
			}
			else Srmm_SetIconFlags(hContact, MODULENAME, 0, MBF_HIDDEN);
		}
	}
}

static int StatusIconsChanged(WPARAM, LPARAM)
{
	if (bShowStatusIcon)
		CallFunctionBuffered(UpdateStatusIcons, 0, FALSE, STATUSICON_REFRESHDELAY);
	return 0;
}

static int ExtraImgSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact) {
		/* user details update */
		if (!strcmp(dbcws->szSetting, "RealIP") || !strcmp(dbcws->szSetting, "Country") || !strcmp(dbcws->szSetting, "CompanyCountry")) {
			/* Extra Image */
			SetExtraImage(hContact);
			/* Status Icon */
			CallFunctionBuffered(UpdateStatusIcons, 0, FALSE, STATUSICON_REFRESHDELAY);
		}
	}
	return 0;
}

/************************* Misc ***********************************/

static int ExtraImgModulesLoaded(WPARAM, LPARAM)
{
	/* Status Icon */
	StatusIconData sid = {};
	sid.szModule = MODULENAME; // dwID = 0
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid, &g_plugin);

	HookEvent(ME_MSG_WINDOWEVENT, MsgWndEvent);
	return 0;
}

void InitExtraImg(void)
{
	CreateServiceFunction(MS_FLAGS_DETECTCONTACTORIGINCOUNTRY, ServiceDetectContactOriginCountry);

	HookEvent(ME_SYSTEM_MODULESLOADED, ExtraImgModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED, StatusIconsChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ExtraImgSettingChanged);

	/* Extra Image */
	hExtraIcon = ExtraIcon_RegisterIcolib("flags_extra", LPGEN("Country flag"), "flags_0");
	if (bShowExtraIcon)
		EnsureExtraImages();
}
