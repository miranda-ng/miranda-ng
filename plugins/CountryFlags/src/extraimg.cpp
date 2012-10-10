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
/* Extra Image */
static HANDLE hHookExtraRebuild,hHookExtraApply;
/* Status Icon */
static HANDLE hHookMsgWndEvent,hHookIconsChanged;
/* Options */
static HANDLE hHookOptInit,hHookSettingChanged;
/* Misc */
extern HINSTANCE hInst;
extern int nCountriesCount;
extern struct CountryListEntry *countries;
static HANDLE hHookModulesLoaded;

/************************* Services *******************************/

static INT_PTR ServiceDetectContactOriginCountry(WPARAM wParam,LPARAM lParam)
{
	int countryNumber=0xFFFF;
	char *pszProto;
	UNREFERENCED_PARAMETER(lParam);
	pszProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	/* ip detect */
	if(DBGetContactSettingByte(NULL,"Flags","UseIpToCountry",SETTING_USEIPTOCOUNTRY_DEFAULT))
		countryNumber=ServiceIpToCountry(DBGetContactSettingDword((HANDLE)wParam,pszProto,"RealIP",0),0);
	/* fallback */
	if(countryNumber==0xFFFF)
		countryNumber=DBGetContactSettingWord((HANDLE)wParam,pszProto,"Country",0);
	if(countryNumber==0 || countryNumber==0xFFFF)
		countryNumber=DBGetContactSettingWord((HANDLE)wParam,pszProto,"CompanyCountry",0);
	return (countryNumber==0)?0xFFFF:countryNumber;
}

/************************* Extra Image ****************************/

#define EXTRAIMAGE_REFRESHDELAY  100  /* time for which setting changes are buffered */

static HANDLE *phExtraImages;
static BYTE idExtraColumn;

static void CALLBACK SetExtraImage(LPARAM lParam)
{
	IconExtraColumn iec;
	int countryNumber,index;
	if(DBGetContactSettingByte(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT))  {
		/* get contact's country */
		iec.hImage=INVALID_HANDLE_VALUE;
		countryNumber=ServiceDetectContactOriginCountry((WPARAM)lParam,0);
		/* get icon */
		if(phExtraImages!=NULL) /* too early? */
			if(countryNumber!=0xFFFF || DBGetContactSettingByte(NULL,"Flags","UseUnknownFlag",SETTING_USEUNKNOWNFLAG_DEFAULT)) {
				index=CountryNumberToIndex(countryNumber);
				/* icon not yet loaded? */
				if(phExtraImages[index]==INVALID_HANDLE_VALUE) {
					HICON hIcon;
					hIcon=LoadFlagIcon(countryNumber);
					if(hIcon!=NULL) phExtraImages[index]=(HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON,(WPARAM)hIcon,0);
					CallService(MS_SKIN2_RELEASEICON,(WPARAM)hIcon,0); /* does NULL check */
				}
				iec.hImage=phExtraImages[index];
			}
		/* choose column */
		iec.cbSize=sizeof(iec);
		iec.ColumnType=idExtraColumn;
		CallService(MS_CLIST_EXTRA_SET_ICON,(WPARAM)lParam,(LPARAM)&iec);
	}
}

// always call in context of main thread
static void RemoveExtraImages(void)
{
	IconExtraColumn iec;
	register HANDLE hContact;
	/* choose column */
	iec.cbSize=sizeof(iec);
	iec.ColumnType=idExtraColumn;
	iec.hImage=INVALID_HANDLE_VALUE;
	/* enum all contacts */
	hContact=db_find_first();
	while(hContact!=NULL) {
		/* invalidate icon */
		CallService(MS_CLIST_EXTRA_SET_ICON,(WPARAM)hContact,(LPARAM)&iec);
		hContact=db_find_next(hContact);
	}
}

// always call in context of main thread
static void EnsureExtraImages(void)
{
	register HANDLE hContact;
	BYTE idMaxExtraCol,idExtraColumnNew;
	/* choose column */
	idMaxExtraCol=(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_EXTRACOLUMNCOUNT); /* 1-based count */
	if(idMaxExtraCol==(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_USEREXTRASTART)) /* same flags if not present */
		idMaxExtraCol=EXTRA_ICON_ADV2; /* zero if not present */
	idExtraColumnNew=DBGetContactSettingRangedByte(NULL,"Flags","ExtraImgFlagColumn",SETTING_EXTRAIMGFLAGCOLUMN_DEFAULT,1,idMaxExtraCol);
	/* clear previous column */
	if(idExtraColumnNew!=idExtraColumn) RemoveExtraImages();
	idExtraColumn=idExtraColumnNew;
	/* enum all contacts */
	hContact=db_find_first();
	while(hContact!=NULL) {
		CallFunctionBuffered(SetExtraImage,(LPARAM)hContact,TRUE,EXTRAIMAGE_REFRESHDELAY);
		hContact=db_find_next(hContact);
	}
}

static void CALLBACK UpdateExtraImages(LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	if(DBGetContactSettingByte(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT))
		EnsureExtraImages();
	else RemoveExtraImages();
}

static int ExtraListRebuild(WPARAM wParam,LPARAM lParam)
{
	BYTE idMaxExtraCol;
	int i;
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	OutputDebugStringA("REBUILD EXTRA\n");
	/* invalidate icons */
	if(phExtraImages!=NULL)
		for(i=0;i<nCountriesCount;++i)
			phExtraImages[i]=INVALID_HANDLE_VALUE;
	/* choose column */
	idMaxExtraCol=(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_EXTRACOLUMNCOUNT); /* 1-based count */
	if(idMaxExtraCol==(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_USEREXTRASTART)) /* same flags if not present */
		idMaxExtraCol=EXTRA_ICON_ADV2; /* zero if not present */
	idExtraColumn=DBGetContactSettingRangedByte(NULL,"Flags","ExtraImgFlagColumn",SETTING_EXTRAIMGFLAGCOLUMN_DEFAULT,1,idMaxExtraCol);
	return 0;
}

static int ExtraImageApply(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	OutputDebugStringA("APPLY EXTRA\n");
	if(DBGetContactSettingByte(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT)) 
		SetExtraImage((LPARAM)wParam); /* unbuffered */
	return 0;
}

/************************* Status Icon ****************************/

#define STATUSICON_REFRESHDELAY  100  /* time for which setting changes are buffered */

// always call in context of main thread
static void FASTCALL SetStatusIcon(HANDLE hContact,int countryNumber)
{
	int i;
	HICON hIcon=NULL;
	StatusIconData sid;
	
	if(countryNumber!=0xFFFF || DBGetContactSettingByte(NULL,"Flags","UseUnknownFlag",SETTING_USEUNKNOWNFLAG_DEFAULT)) {
		/* copy icon as status icon API will call DestroyIcon() on it */
		hIcon=LoadFlagIcon(countryNumber);
		sid.hIcon=(hIcon!=NULL)?CopyIcon(hIcon):NULL;
		CallService(MS_SKIN2_RELEASEICON,(WPARAM)hIcon,0); /* does NULL check */
		hIcon=sid.hIcon;
		/* ensure status icon is registered */
		sid.cbSize=sizeof(sid);
		sid.szModule="Flags";
		sid.dwId=countryNumber;
		sid.hIconDisabled=NULL;
		sid.flags=0;
		sid.szTooltip=Translate((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber,0));
		if(CallService(MS_MSG_MODIFYICON,0,(LPARAM)&sid)) /* not yet registered? */
			CallService(MS_MSG_ADDICON,0,(LPARAM)&sid);
	}	
	/* disable all other flags for this contact */
	sid.hIcon=NULL;
	sid.szTooltip=NULL;
	sid.flags=MBF_HIDDEN;
	for(i=0;i<nCountriesCount;++i) {
		sid.dwId=countries[i].id;
		if(countryNumber==countries[i].id && hIcon!=NULL) sid.flags=0;
		else sid.flags=MBF_HIDDEN;
		CallService(MS_MSG_MODIFYICON,(WPARAM)hContact,(LPARAM)&sid);
	}
}

// always call in context of main thread
static void FASTCALL UnsetStatusIcon(HANDLE hContact,int countryNumber)
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
	UNREFERENCED_PARAMETER(wParam);
	switch(msgwe->uType) {
		case MSG_WINDOW_EVT_OPENING:
		case MSG_WINDOW_EVT_CLOSE:
		{	int countryNumber;
			if(msgwe->hContact==NULL || !ServiceExists(MS_MSG_ADDICON)) break; /* sanity check */
			countryNumber=ServiceDetectContactOriginCountry((WPARAM)msgwe->hContact,0);
			if(DBGetContactSettingByte(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT)) {
				if(msgwe->uType==MSG_WINDOW_EVT_OPENING) SetStatusIcon(msgwe->hContact,countryNumber);
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
	UNREFERENCED_PARAMETER(lParam);

	msgwi.cbSize=sizeof(msgwi);
	msgw.cbSize=sizeof(msgw);
	msgwi.uFlags=MSG_WINDOW_UFLAG_MSG_BOTH;
	/* enum all opened message windows */
	fShow=DBGetContactSettingByte(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT);
	msgwi.hContact=db_find_first();
	while(msgwi.hContact!=NULL) {
		/* is a message window opened for this contact? */
		if (!CallService(MS_MSG_GETWINDOWDATA,(WPARAM)&msgwi,(LPARAM)&msgw) && msgw.uState&MSG_WINDOW_STATE_EXISTS) {
			countryNumber=ServiceDetectContactOriginCountry((WPARAM)msgwi.hContact,0);
			if(fShow) SetStatusIcon(msgwi.hContact,countryNumber);
			else UnsetStatusIcon(msgwi.hContact,countryNumber);
		}
		msgwi.hContact = db_find_next(msgw.hContact);
	}
}

static int StatusIconsChanged(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if(ServiceExists(MS_MSG_ADDICON))
		if(DBGetContactSettingByte(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT))
			CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
	return 0;
}

/************************* Options ************************************/

#define M_ENABLE_SUBCTLS  (WM_APP+1)

static INT_PTR CALLBACK ExtraImgOptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	switch(msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			/* init checkboxes */
			{	BOOL val;
				/* Status Icon */
				if(ServiceExists(MS_MSG_ADDICON)) val=DBGetContactSettingByte(NULL,"Flags","ShowStatusIconFlag",SETTING_SHOWSTATUSICONFLAG_DEFAULT)!=0;
				else EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG),val=FALSE);
				CheckDlgButton(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG,val);
				/* Extra Image */
				if(ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) val=DBGetContactSettingByte(NULL,"Flags","ShowExtraImgFlag",SETTING_SHOWEXTRAIMGFLAG_DEFAULT)!=0;
				else EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG),val=FALSE);
				CheckDlgButton(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG,val);
				/* Unknown Flag */
				val=DBGetContactSettingByte(NULL,"Flags","UseUnknownFlag",SETTING_USEUNKNOWNFLAG_DEFAULT)!=0;
				CheckDlgButton(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG,val);
				/* IP-to-country */
				val=DBGetContactSettingByte(NULL,"Flags","UseIpToCountry",SETTING_USEIPTOCOUNTRY_DEFAULT)!=0;
				CheckDlgButton(hwndDlg,IDC_CHECK_USEIPTOCOUNTRY,val);
			}
			/* init combobox */
			{	HWND hwndCombo;
				TCHAR szItem[64];
				BYTE idColumn,idSavedColumn;
				BYTE idMaxExtraCol,idAdvExtraColStart;
				int index;
				hwndCombo=GetDlgItem(hwndDlg,IDC_COMBO_EXTRAIMGFLAGCOLUMN);
				idSavedColumn=DBGetContactSettingByte(NULL,"Flags","ExtraImgFlagColumn",SETTING_EXTRAIMGFLAGCOLUMN_DEFAULT);
				idMaxExtraCol=(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_EXTRACOLUMNCOUNT); /* 1-based count */
				idAdvExtraColStart=(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_USEREXTRASTART); /* 1-based id */
				/* init */
				SendMessage(hwndCombo,CB_SETLOCALE,(LCID)CallService(MS_LANGPACK_GETLOCALE,0,0),0); /* for sort order */
				SendMessage(hwndCombo,CB_INITSTORAGE,idMaxExtraCol-idAdvExtraColStart+3,(idMaxExtraCol-idAdvExtraColStart+3)*SIZEOF(szItem));
				/* Advanced #1,#2 */
				{	const BYTE columnIds[]={EXTRA_ICON_ADV1,EXTRA_ICON_ADV2};
					for(idColumn=0;idColumn<SIZEOF(columnIds);++idColumn) {
						mir_sntprintf(szItem,SIZEOF(szItem),TranslateT("Advanced #%u"),idColumn+1); /* buffer safe */
						index=SendMessage(hwndCombo,CB_ADDSTRING,0,(LPARAM)szItem);
						if(index!=LB_ERR) {
							SendMessage(hwndCombo,CB_SETITEMDATA,index,columnIds[idColumn]);
							if(idColumn==0 || columnIds[idColumn]==idSavedColumn) SendMessage(hwndCombo,CB_SETCURSEL,index,0);
						}
					}
				}
				/* Advanced #3+: clist_modern */
				if(idMaxExtraCol!=idAdvExtraColStart) /* same flags if not present */
					for(idColumn=idAdvExtraColStart;idColumn<=idMaxExtraCol;++idColumn) {
						mir_sntprintf(szItem,SIZEOF(szItem),TranslateT("Advanced #%u"),idColumn-idAdvExtraColStart+3); /* buffer safe */
						index=SendMessage(hwndCombo,CB_ADDSTRING,0,(LPARAM)szItem);
						if(index!=LB_ERR) {
							SendMessage(hwndCombo,CB_SETITEMDATA,index,idColumn);
							if(idColumn==idSavedColumn) SendMessage(hwndCombo,CB_SETCURSEL,index,0);
						}
					}
			}
			SendMessage(hwndDlg,M_ENABLE_SUBCTLS,0,0);
			return TRUE; /* default focus */
		case M_ENABLE_SUBCTLS:
		{	BOOL checked=IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TEXT_EXTRAIMGFLAGCOLUMN),checked);
			EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO_EXTRAIMGFLAGCOLUMN),checked);
			if (!checked) checked=IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG);
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
					DBWriteContactSettingByte(NULL,"Flags","UseUnknownFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USEUNKNOWNFLAG)!=0));
					DBWriteContactSettingByte(NULL,"Flags","UseIpToCountry",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USEIPTOCOUNTRY)!=0));
					/* Status Icon */
					if(IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG)))
						DBWriteContactSettingByte(NULL,"Flags","ShowStatusIconFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWSTATUSICONFLAG)!=0));
					/* Extra Image */
					if(IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG)))
						DBWriteContactSettingByte(NULL,"Flags","ShowExtraImgFlag",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOWEXTRAIMGFLAG)!=0));
					{	int index;
						index=SendDlgItemMessage(hwndDlg,IDC_COMBO_EXTRAIMGFLAGCOLUMN,CB_GETCURSEL,0,0);
						if(index!=LB_ERR) DBWriteContactSettingByte(NULL,"Flags","ExtraImgFlagColumn",(BYTE)SendDlgItemMessage(hwndDlg,IDC_COMBO_EXTRAIMGFLAGCOLUMN,CB_GETITEMDATA,index,0));
					}
					return TRUE;
			}
			break;
	}
	return FALSE;
}

static UINT expertOnlyControls[]={IDC_CHECK_USEIPTOCOUNTRY};
static int ExtraImgOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
  	UNREFERENCED_PARAMETER(lParam);
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXTRAIMG);
	odp.position = 900000002;
	odp.pszGroup = LPGEN("Contact List");  /* autotranslated */
	odp.pszTitle = LPGEN("Country Flags"); /* autotranslated */
	odp.pszTab = LPGEN("Country Flags");   /* autotranslated, can be made a tab */
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = ExtraImgOptDlgProc;
	odp.expertOnlyControls = expertOnlyControls;
	odp.nExpertOnlyControls = SIZEOF(expertOnlyControls);
	Options_AddPage(wParam, &odp);
	return 0;
}

static int ExtraImgSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
	if ((HANDLE)wParam==NULL) {
		if (!lstrcmpA(dbcws->szModule,"Flags")) {
			/* Extra Image */
			if (!lstrcmpA(dbcws->szSetting,"ShowExtraImgFlag") ||
			   !lstrcmpA(dbcws->szSetting,"ExtraImgFlagColumn") ||
			   !lstrcmpA(dbcws->szSetting,"UseUnknownFlag") ||
			   !lstrcmpA(dbcws->szSetting,"UseIpToCountry"))
				if(ServiceExists(MS_CLIST_EXTRA_SET_ICON))
					CallFunctionBuffered(UpdateExtraImages,0,FALSE,EXTRAIMAGE_REFRESHDELAY);
			/* Status Icon */
			if (!lstrcmpA(dbcws->szSetting,"ShowStatusIconFlag") ||
			   !lstrcmpA(dbcws->szSetting,"UseUnknownFlag") ||
			   !lstrcmpA(dbcws->szSetting,"UseIpToCountry"))
			   if(ServiceExists(MS_MSG_ADDICON))
				   CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
		}
	}
	/* user details update */
	else if (!lstrcmpA(dbcws->szSetting,"RealIP") ||
	        !lstrcmpA(dbcws->szSetting,"Country") ||
	        !lstrcmpA(dbcws->szSetting,"CompanyCountry")) {
		/* Extra Image */
		if(ServiceExists(MS_CLIST_EXTRA_SET_ICON))
		   CallFunctionBuffered(SetExtraImage,(LPARAM)wParam,TRUE,EXTRAIMAGE_REFRESHDELAY);
		/* Status Icon */
		if(ServiceExists(MS_MSG_ADDICON))
		   CallFunctionBuffered(UpdateStatusIcons,0,FALSE,STATUSICON_REFRESHDELAY);
	}
	return 0;
}

/************************* Misc ***********************************/

static int ExtraImgModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	/* Options */
	if(ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)"Flags",0);
	/* Extra Image */
	if(ServiceExists(MS_CLIST_EXTRA_SET_ICON)) {
		int i;
		BYTE idMaxExtraCol;
		phExtraImages=(HANDLE*)mir_alloc(nCountriesCount*sizeof(HANDLE));
		/* invalidate icons */
		if(phExtraImages!=NULL)
			for(i=0;i<nCountriesCount;++i)
				phExtraImages[i]=INVALID_HANDLE_VALUE;
		/* choose column */
		idMaxExtraCol=(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_EXTRACOLUMNCOUNT); /* 1-based count */
		if(idMaxExtraCol==(BYTE)CallService(MS_CLUI_GETCAPS,0,CLUIF2_USEREXTRASTART)) /* same flags if not present */
			idMaxExtraCol=EXTRA_ICON_ADV2; /* zero if not present */
		idExtraColumn=DBGetContactSettingRangedByte(NULL,"Flags","ExtraImgFlagColumn",SETTING_EXTRAIMGFLAGCOLUMN_DEFAULT,1,idMaxExtraCol);
		/* hook */
		hHookExtraRebuild=HookEvent(ME_CLIST_EXTRA_LIST_REBUILD,ExtraListRebuild);
		hHookExtraApply=HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY,ExtraImageApply);
	}
	/* Status Icon */
	hHookMsgWndEvent=HookEvent(ME_MSG_WINDOWEVENT,MsgWndEvent);
	return 0;
}

void InitExtraImg(void)
{
	/* Services */
	hServiceDetectContactOrigin=CreateServiceFunction(MS_FLAGS_DETECTCONTACTORIGINCOUNTRY,ServiceDetectContactOriginCountry);
	/* Misc */
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,ExtraImgModulesLoaded);
	/* Extra Image */
	phExtraImages=NULL;
	hHookExtraRebuild=hHookExtraApply=NULL;
	/* Status icon */
	hHookMsgWndEvent=NULL;
	hHookIconsChanged=HookEvent(ME_SKIN2_ICONSCHANGED,StatusIconsChanged);
	/* Options */
	hHookOptInit=HookEvent(ME_OPT_INITIALISE,ExtraImgOptInit);
	hHookSettingChanged=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,ExtraImgSettingChanged);
}

void UninitExtraImg(void)
{
	/* Services */
	DestroyServiceFunction(hServiceDetectContactOrigin);
	/* Misc */
	UnhookEvent(hHookModulesLoaded);
	/* Extra Image */
	UnhookEvent(hHookSettingChanged);
	UnhookEvent(hHookExtraRebuild);
	UnhookEvent(hHookExtraApply);
	mir_free(phExtraImages); /* does NULL check */
	/* Status icon */
	UnhookEvent(hHookMsgWndEvent);
	UnhookEvent(hHookIconsChanged);
	/* Options */
	UnhookEvent(hHookOptInit);
	UnhookEvent(hHookSettingChanged);
}
