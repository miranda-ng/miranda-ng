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

#include "flags.h"

/* Services */
static HANDLE hServiceDetectContactOrigin;

/* Misc */
extern HINSTANCE hInst;
extern int nCountriesCount;
extern struct CountryListEntry *countries;

/************************* Services *******************************/

static INT_PTR ServiceDetectContactOriginCountry(WPARAM wParam,LPARAM lParam)
{
	int countryNumber = 0xFFFF;
	char *pszProto = GetContactProto((HANDLE)wParam);
	/* ip detect */
	if ( db_get_b(NULL, MODULENAME, "UseIpToCountry", SETTING_USEIPTOCOUNTRY_DEFAULT))
		countryNumber = ServiceIpToCountry(db_get_dw((HANDLE)wParam,pszProto,"RealIP",0),0);
	/* fallback */
	if (countryNumber == 0xFFFF)
		countryNumber = db_get_w((HANDLE)wParam,pszProto,"Country",0);
	if (countryNumber == 0 || countryNumber == 0xFFFF)
		countryNumber = db_get_w((HANDLE)wParam,pszProto,"CompanyCountry",0);
	return (countryNumber == 0) ? 0xFFFF : countryNumber;
}

/************************* Extra Image ****************************/

#define EXTRAIMAGE_REFRESHDELAY  100  /* time for which setting changes are buffered */

static HANDLE hExtraIcon;

static void CALLBACK SetExtraImage(HANDLE hContact)
{
	if ( !db_get_b(NULL, MODULENAME, "ShowExtraImgFlag", SETTING_SHOWEXTRAIMGFLAG_DEFAULT))
		return;
		
	int countryNumber = ServiceDetectContactOriginCountry((WPARAM)hContact, 0);
	char szId[20];
	wsprintfA(szId, (countryNumber == 0xFFFF) ? "%s0x%X" : "%s%i", "flags_", countryNumber);
	ExtraIcon_SetIcon(hExtraIcon, hContact, szId);
}

// always call in context of main thread
static void RemoveExtraImages(void)
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		ExtraIcon_Clear(hExtraIcon, hContact);
}

// always call in context of main thread
static void EnsureExtraImages(void)
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		SetExtraImage(hContact);
}

static void CALLBACK UpdateExtraImages(LPARAM lParam)
{
	if ( db_get_b(NULL, MODULENAME, "ShowExtraImgFlag", SETTING_SHOWEXTRAIMGFLAG_DEFAULT))
		EnsureExtraImages();
	else
		RemoveExtraImages();
}

static int ExtraImageApply(WPARAM wParam,LPARAM lParam)
{
	if ( db_get_b(NULL, MODULENAME, "ShowExtraImgFlag", SETTING_SHOWEXTRAIMGFLAG_DEFAULT)) 
		SetExtraImage((HANDLE)wParam); /* unbuffered */
	return 0;
}

/************************* Status Icon ****************************/

#define STATUSICON_REFRESHDELAY  100  /* time for which setting changes are buffered */

// always call in context of main thread
static void __fastcall SetStatusIcon(HANDLE hContact,int countryNumber)
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULENAME;

	if (countryNumber != 0xFFFF || db_get_b(NULL, MODULENAME, "UseUnknownFlag", SETTING_USEUNKNOWNFLAG_DEFAULT)) {
		/* copy icon as status icon API will call DestroyIcon() on it */
		sid.hIcon = LoadFlagIcon(countryNumber);
		sid.szTooltip = Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber,0));
	}	
	else sid.flags = MBF_HIDDEN;

	CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);

	if (sid.hIcon)
		Skin_ReleaseIcon(sid.hIcon);
}

// always call in context of main thread
static void __fastcall UnsetStatusIcon(HANDLE hContact)
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
}

static int MsgWndEvent(WPARAM wParam,LPARAM lParam)
{
	MessageWindowEventData *msgwe=(MessageWindowEventData*)lParam;
	switch(msgwe->uType) {
	case MSG_WINDOW_EVT_OPENING:
	case MSG_WINDOW_EVT_CLOSE:
		if ( db_get_b(NULL, MODULENAME, "ShowStatusIconFlag", SETTING_SHOWSTATUSICONFLAG_DEFAULT)) {
			int countryNumber = ServiceDetectContactOriginCountry((WPARAM)msgwe->hContact, 0);
			if (msgwe->uType == MSG_WINDOW_EVT_OPENING && countryNumber != 0xFFFF)
				SetStatusIcon(msgwe->hContact,countryNumber);
			else
				UnsetStatusIcon(msgwe->hContact);
		}
		/* ensure it is hidden, RemoveStatusIcons() only enums currently opened ones  */
		else UnsetStatusIcon(msgwe->hContact);
	}
	return 0;
}

static void CALLBACK UpdateStatusIcons(LPARAM lParam)
{
	BOOL fShow = db_get_b(NULL,MODULENAME,"ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT);

	MessageWindowInputData msgwi = { sizeof(msgwi) };
	msgwi.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	msgwi.hContact = db_find_first();
	while (msgwi.hContact != NULL) {
		/* is a message window opened for this contact? */
		MessageWindowData msgw; /* output */
		msgw.cbSize = sizeof(msgw);
		if (!CallService(MS_MSG_GETWINDOWDATA,(WPARAM)&msgwi,(LPARAM)&msgw) && msgw.uState & MSG_WINDOW_STATE_EXISTS) {
			if (fShow) {
				int countryNumber = ServiceDetectContactOriginCountry((WPARAM)msgwi.hContact, 0);
				SetStatusIcon(msgwi.hContact, countryNumber);
			}
			else UnsetStatusIcon(msgwi.hContact);
		}
		msgwi.hContact = db_find_next(msgw.hContact);
	}
}

static int StatusIconsChanged(WPARAM wParam,LPARAM lParam)
{
	if (ServiceExists(MS_MSG_MODIFYICON))
		if ( db_get_b(NULL, MODULENAME, "ShowStatusIconFlag", SETTING_SHOWSTATUSICONFLAG_DEFAULT))
			CallFunctionBuffered(UpdateStatusIcons, 0, FALSE, STATUSICON_REFRESHDELAY);
	return 0;
}

/************************* Options ************************************/

#define M_ENABLE_SUBCTLS  (WM_APP+1)

static INT_PTR CALLBACK ExtraImgOptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		/* init checkboxes */
		{
			BOOL val;
			/* Status Icon */
			if (ServiceExists(MS_MSG_REMOVEICON))
				val = db_get_b(NULL, MODULENAME, "ShowStatusIconFlag", SETTING_SHOWSTATUSICONFLAG_DEFAULT) != 0;
			else
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG),val=FALSE);
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG,val);
			/* Extra Image */
			val = db_get_b(NULL,MODULENAME,"ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT) != 0;
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG,val);
			/* Unknown Flag */
			val = db_get_b(NULL,MODULENAME,"UseUnknownFlag",SETTING_USEUNKNOWNFLAG_DEFAULT) != 0;
			CheckDlgButton(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG,val);
			/* IP-to-country */
			val = db_get_b(NULL,MODULENAME,"UseIpToCountry",SETTING_USEIPTOCOUNTRY_DEFAULT) != 0;
			CheckDlgButton(hwndDlg,IDC_CHECK_USEIPTOCOUNTRY,val);
		}
		SendMessage(hwndDlg,M_ENABLE_SUBCTLS,0,0);
		return TRUE; /* default focus */

	case M_ENABLE_SUBCTLS:
		{
			BOOL checked = IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TEXT_EXTRAIMGFLAGCOLUMN),checked);
			if (!checked)
				checked = IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG),checked);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_USEIPTOCOUNTRY),checked);
			return TRUE;
		}

	case WM_COMMAND:
		PostMessage(hwndDlg,M_ENABLE_SUBCTLS,0,0);
		PostMessage(GetParent(hwndDlg),PSM_CHANGED,0,0); /* enable apply */
		return FALSE;

	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY: /* setting change hook will pick these up  */
			db_set_b(NULL,MODULENAME,"UseUnknownFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG) != 0));
			db_set_b(NULL,MODULENAME,"UseIpToCountry",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USEIPTOCOUNTRY) != 0));
			/* Status Icon */
			if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG)))
				db_set_b(NULL,MODULENAME,"ShowStatusIconFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG) != 0));
			/* Extra Image */
			if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG)))
				db_set_b(NULL,MODULENAME,"ShowExtraImgFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG) != 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static int ExtraImgOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXTRAIMG);
	odp.position = 900000002;
	odp.pszGroup = LPGEN("Icons");  /* autotranslated */
	odp.pszTitle = LPGEN("Country Flags"); /* autotranslated */
	odp.pszTab = LPGEN("Country Flags");   /* autotranslated, can be made a tab */
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = ExtraImgOptDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int ExtraImgSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
	if ((HANDLE)wParam == NULL) {
		if (!lstrcmpA(dbcws->szModule,MODULENAME)) {
			/* Extra Image */
			if (!lstrcmpA(dbcws->szSetting,"ShowExtraImgFlag") ||
			    !lstrcmpA(dbcws->szSetting,"ExtraImgFlagColumn") ||
			    !lstrcmpA(dbcws->szSetting,"UseUnknownFlag") ||
			    !lstrcmpA(dbcws->szSetting,"UseIpToCountry"))
				CallFunctionBuffered(UpdateExtraImages,0,FALSE,EXTRAIMAGE_REFRESHDELAY);
			/* Status Icon */
			if (!lstrcmpA(dbcws->szSetting,"ShowStatusIconFlag") ||
			   !lstrcmpA(dbcws->szSetting,"UseUnknownFlag") ||
			   !lstrcmpA(dbcws->szSetting,"UseIpToCountry"))
			   if (ServiceExists(MS_MSG_REMOVEICON))
				   CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
		}
	}
	/* user details update */
	else if (!lstrcmpA(dbcws->szSetting,"RealIP") ||
	        !lstrcmpA(dbcws->szSetting,"Country") ||
	        !lstrcmpA(dbcws->szSetting,"CompanyCountry")) {
		/* Extra Image */
	   SetExtraImage((HANDLE)wParam);
		/* Status Icon */
		if (ServiceExists(MS_MSG_REMOVEICON))
		   CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
	}
	return 0;
}

/************************* Misc ***********************************/

static int ExtraImgModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	/* Options */
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)MODULENAME,0);

	/* Extra Image */
	hExtraIcon = ExtraIcon_Register("flags_extra", LPGEN("Country flag"));
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY,ExtraImageApply);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULENAME; // dwID = 0
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	/* Status Icon */
	HookEvent(ME_MSG_WINDOWEVENT, MsgWndEvent);
	return 0;
}

void InitExtraImg(void)
{
	/* Services */
	hServiceDetectContactOrigin = CreateServiceFunction(MS_FLAGS_DETECTCONTACTORIGINCOUNTRY,ServiceDetectContactOriginCountry);
	/* Misc */
	HookEvent(ME_SYSTEM_MODULESLOADED, ExtraImgModulesLoaded);
	/* Status icon */
	HookEvent(ME_SKIN2_ICONSCHANGED,StatusIconsChanged);
	/* Options */
	HookEvent(ME_OPT_INITIALISE,ExtraImgOptInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED,ExtraImgSettingChanged);
}

void UninitExtraImg(void)
{
	/* Services */
	DestroyServiceFunction(hServiceDetectContactOrigin);
}
