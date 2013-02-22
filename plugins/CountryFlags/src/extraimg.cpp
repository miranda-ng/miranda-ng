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
	if ( db_get_b(NULL,"Flags","UseIpToCountry",SETTING_USEIPTOCOUNTRY_DEFAULT))
		countryNumber=ServiceIpToCountry(DBGetContactSettingDword((HANDLE)wParam,pszProto,"RealIP",0),0);
	/* fallback */
	if (countryNumber == 0xFFFF)
		countryNumber=DBGetContactSettingWord((HANDLE)wParam,pszProto,"Country",0);
	if (countryNumber == 0 || countryNumber == 0xFFFF)
		countryNumber=DBGetContactSettingWord((HANDLE)wParam,pszProto,"CompanyCountry",0);
	return (countryNumber == 0) ? 0xFFFF : countryNumber;
}

/************************* Extra Image ****************************/

#define EXTRAIMAGE_REFRESHDELAY  100  /* time for which setting changes are buffered */

static HANDLE hExtraIcon;

static void CALLBACK SetExtraImage(HANDLE hContact)
{
	if ( !db_get_b(NULL, "Flags", "ShowExtraImgFlag", SETTING_SHOWEXTRAIMGFLAG_DEFAULT))
		return;
		
	int countryNumber = ServiceDetectContactOriginCountry((WPARAM)hContact, 0);
	char szId[20];
	wsprintfA(szId, (countryNumber == 0xFFFF) ? "%s0x%X" : "%s%i", "flags_", countryNumber);
	ExtraIcon_SetIcon(hExtraIcon, hContact, szId);
}

// always call in context of main thread
static void RemoveExtraImages(void)
{
	HANDLE hContact = db_find_first();
	while (hContact != NULL) {
		ExtraIcon_Clear(hExtraIcon, hContact);
		hContact = db_find_next(hContact);
	}
}

// always call in context of main thread
static void EnsureExtraImages(void)
{
	HANDLE hContact = db_find_first();
	while (hContact != NULL) {
		SetExtraImage(hContact);
		hContact = db_find_next(hContact);
	}
}

static void CALLBACK UpdateExtraImages(LPARAM lParam)
{
	if ( db_get_b(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT))
		EnsureExtraImages();
	else
		RemoveExtraImages();
}

static int ExtraImageApply(WPARAM wParam,LPARAM lParam)
{
	if ( db_get_b(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT)) 
		SetExtraImage((HANDLE)wParam); /* unbuffered */
	return 0;
}

/************************* Status Icon ****************************/

#define STATUSICON_REFRESHDELAY  100  /* time for which setting changes are buffered */

// always call in context of main thread
static void __fastcall SetStatusIcon(HANDLE hContact,int countryNumber)
{
	HICON hIcon = NULL;

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = "Flags";
	
	if (countryNumber != 0xFFFF || db_get_b(NULL, "Flags", "UseUnknownFlag", SETTING_USEUNKNOWNFLAG_DEFAULT)) {
		/* copy icon as status icon API will call DestroyIcon() on it */
		hIcon = LoadFlagIcon(countryNumber);
		sid.hIcon = (hIcon != NULL) ? CopyIcon(hIcon) : NULL;
		Skin_ReleaseIcon(hIcon); /* does NULL check */
		hIcon = sid.hIcon;
		/* ensure status icon is registered */
		sid.dwId = countryNumber;
		sid.szTooltip = Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber,0));
		if (CallService(MS_MSG_MODIFYICON, 0, (LPARAM)&sid)) /* not yet registered? */
			CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
	}	

	/* disable all other flags for this contact */
	sid.hIcon = NULL;
	sid.szTooltip = NULL;
	sid.flags = MBF_HIDDEN;
	for (int i=0; i < nCountriesCount; ++i) {
		sid.dwId = countries[i].id;
		if (countryNumber == countries[i].id && hIcon != NULL)
			sid.flags = 0;
		else
			sid.flags = MBF_HIDDEN;
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
	}
}

// always call in context of main thread
static void __fastcall UnsetStatusIcon(HANDLE hContact,int countryNumber)
{
	StatusIconData sid;
	sid.cbSize=sizeof(sid);
	sid.szModule="Flags";
	sid.dwId=countryNumber;
	sid.hIconDisabled=sid.hIcon=NULL;
	sid.szTooltip=NULL;
	sid.flags=MBF_HIDDEN;
	CallService(MS_MSG_MODIFYICON,(WPARAM)hContact,(LPARAM)&sid); /* registered? */
	/* can't call MS_MSG_REMOVEICON here as the icon might be
	 * in use by other contacts simultanously, removing them all at exit */
}

static int MsgWndEvent(WPARAM wParam,LPARAM lParam)
{
	MessageWindowEventData *msgwe=(MessageWindowEventData*)lParam;
	switch(msgwe->uType) {
		case MSG_WINDOW_EVT_OPENING:
		case MSG_WINDOW_EVT_CLOSE:
		{	int countryNumber;
			if (msgwe->hContact == NULL || !ServiceExists(MS_MSG_ADDICON)) break; /* sanity check */
			countryNumber=ServiceDetectContactOriginCountry((WPARAM)msgwe->hContact,0);
			if ( db_get_b(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT)) {
				if (msgwe->uType == MSG_WINDOW_EVT_OPENING) SetStatusIcon(msgwe->hContact,countryNumber);
				else UnsetStatusIcon(msgwe->hContact,countryNumber);
			}
			/* ensure it is hidden, RemoveStatusIcons() only enums currently opened ones  */
			else UnsetStatusIcon(msgwe->hContact,countryNumber);
		}
	}
	return 0;
}

static void CALLBACK UpdateStatusIcons(LPARAM lParam)
{
	MessageWindowInputData msgwi; /* input */
	MessageWindowData msgw; /* output */
	BOOL fShow;
	int countryNumber;

	msgwi.cbSize=sizeof(msgwi);
	msgw.cbSize=sizeof(msgw);
	msgwi.uFlags=MSG_WINDOW_UFLAG_MSG_BOTH;
	/* enum all opened message windows */
	fShow=db_get_b(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT);
	msgwi.hContact=db_find_first();
	while (msgwi.hContact != NULL) {
		/* is a message window opened for this contact? */
		if (!CallService(MS_MSG_GETWINDOWDATA,(WPARAM)&msgwi,(LPARAM)&msgw) && msgw.uState&MSG_WINDOW_STATE_EXISTS) {
			countryNumber=ServiceDetectContactOriginCountry((WPARAM)msgwi.hContact,0);
			if (fShow) SetStatusIcon(msgwi.hContact,countryNumber);
			else UnsetStatusIcon(msgwi.hContact,countryNumber);
		}
		msgwi.hContact = db_find_next(msgw.hContact);
	}
}

static int StatusIconsChanged(WPARAM wParam,LPARAM lParam)
{
	if (ServiceExists(MS_MSG_ADDICON))
		if ( db_get_b(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT))
			CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
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
			if (ServiceExists(MS_MSG_ADDICON)) val=db_get_b(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT) != 0;
			else EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG),val=FALSE);
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG,val);
			/* Extra Image */
			val=db_get_b(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT) != 0;
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG,val);
			/* Unknown Flag */
			val=db_get_b(NULL,"Flags","UseUnknownFlag",SETTING_USEUNKNOWNFLAG_DEFAULT) != 0;
			CheckDlgButton(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG,val);
			/* IP-to-country */
			val=db_get_b(NULL,"Flags","UseIpToCountry",SETTING_USEIPTOCOUNTRY_DEFAULT) != 0;
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
			DBWriteContactSettingByte(NULL,"Flags","UseUnknownFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG) != 0));
			DBWriteContactSettingByte(NULL,"Flags","UseIpToCountry",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USEIPTOCOUNTRY) != 0));
			/* Status Icon */
			if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG)))
				DBWriteContactSettingByte(NULL,"Flags","ShowStatusIconFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG) != 0));
			/* Extra Image */
			if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG)))
				DBWriteContactSettingByte(NULL,"Flags","ShowExtraImgFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG) != 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static int ExtraImgOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
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
		if (!lstrcmpA(dbcws->szModule,"Flags")) {
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
			   if (ServiceExists(MS_MSG_ADDICON))
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
		if (ServiceExists(MS_MSG_ADDICON))
		   CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
	}
	return 0;
}

/************************* Misc ***********************************/

static int ExtraImgModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	/* Options */
	if (ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)"Flags",0);

	/* Extra Image */
	hExtraIcon = ExtraIcon_Register("flags_extra", "Country flag");
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY,ExtraImageApply);

	/* Status Icon */
	HookEvent(ME_MSG_WINDOWEVENT,MsgWndEvent);
	return 0;
}

void InitExtraImg(void)
{
	/* Services */
	hServiceDetectContactOrigin=CreateServiceFunction(MS_FLAGS_DETECTCONTACTORIGINCOUNTRY,ServiceDetectContactOriginCountry);
	/* Misc */
	HookEvent(ME_SYSTEM_MODULESLOADED,ExtraImgModulesLoaded);
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
