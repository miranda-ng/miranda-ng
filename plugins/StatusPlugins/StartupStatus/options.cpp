/*
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../commonstatus.h"
#include "startupstatus.h"
#include "../resource.h"

// for db cleanup
static int settingIndex;

// prototypes
INT_PTR CALLBACK CmdlOptionsDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK OptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK addProfileDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);

BOOL MyGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);
static int CountSettings(const char *szSetting,LPARAM lParam);
static int DeleteSetting(const char *szSetting,LPARAM lParam);
static int ClearDatabase(char* filter);

int LoadMainOptions();

/////////////////////////////////////////////////////////////////////////////////////////

static TSettingsList* GetCurrentProtoSettings()
{
	int count;
	PROTOACCOUNT** protos;
	ProtoEnumAccounts( &count, &protos );
	
	TSettingsList* result = new TSettingsList( count, CompareSettings );
	if ( result == NULL )
		return NULL;

	for ( int i=0; i < count; i++ )
		if ( IsSuitableProto( protos[i] ))
			result->insert( new TSSSetting( protos[i] ));

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Command line processing

static char* GetStatusDesc(int status)
{
	switch (status) {
		case ID_STATUS_AWAY:       return "away";
		case ID_STATUS_NA:         return "na";
		case ID_STATUS_DND:        return "dnd";
		case ID_STATUS_OCCUPIED:   return "occupied";
		case ID_STATUS_FREECHAT:   return "freechat";
		case ID_STATUS_ONLINE:     return "online";
		case ID_STATUS_OFFLINE:    return "offline";
		case ID_STATUS_INVISIBLE:  return "invisible";
		case ID_STATUS_ONTHEPHONE: return "onthephone";
		case ID_STATUS_OUTTOLUNCH: return "outtolunch";
		case ID_STATUS_LAST:       return "last";
	}
	return "offline";
}

static char* GetCMDLArguments(TSettingsList& protoSettings)
{
	if ( protoSettings.getCount() == NULL )
		return NULL;

	char *cmdl, *pnt;
	pnt = cmdl = ( char* )malloc(strlen(protoSettings[0].szName) + strlen(GetStatusDesc(protoSettings[0].status)) + 4);

	for (int i=0; i < protoSettings.getCount(); i++ ) {
		*pnt++ = '/';
		strcpy(pnt, protoSettings[i].szName);
		pnt += strlen(protoSettings[i].szName);
		*pnt++ = '=';
		strcpy(pnt, GetStatusDesc(protoSettings[i].status));
		pnt += strlen(GetStatusDesc(protoSettings[i].status));
		if (i != protoSettings.getCount()-1) {
			*pnt++ = ' ';
			*pnt++ = '\0';
			cmdl = ( char* )realloc(cmdl, strlen(cmdl) + strlen(protoSettings[i+1].szName) + strlen(GetStatusDesc(protoSettings[i+1].status)) + 4);
			pnt = cmdl + strlen(cmdl);
	}	}
		
	if ( DBGetContactSettingByte( NULL, MODULENAME, SETTING_SHOWDIALOG, FALSE ) == TRUE ) {
		*pnt++ = ' ';
		*pnt++ = '\0';
		cmdl = ( char* )realloc(cmdl, strlen(cmdl) + 12);
		pnt = cmdl + strlen(cmdl);
		strcpy(pnt, "/showdialog");
		pnt += 11;
		*pnt = '\0';
	}
	
	return cmdl;
}

static char* GetCMDL(TSettingsList& protoSettings)
{
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);

	char* cmdl = ( char* )malloc(strlen(path) + 4);
	_snprintf(cmdl, strlen(path) + 4, "\"%s\" ", path);

	char* args = GetCMDLArguments(protoSettings);
	if ( args ) {
		cmdl = ( char* )realloc(cmdl, strlen(cmdl) + strlen(args) + 1);
		strcat(cmdl, args);
		free(args);
	}
	return cmdl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Link processing

static char* GetLinkDescription(TSettingsList& protoSettings)
{
	if ( protoSettings.getCount() == 0 )  
		return NULL;

	char *pnt, *desc;
	pnt = desc = ( char* )malloc(strlen(SHORTCUT_DESC) + 1);
	strcpy(desc, SHORTCUT_DESC);
	pnt = desc + strlen(desc);
	for ( int i=0; i < protoSettings.getCount(); i++ ) {
		char szName[128];
		CallProtoService(protoSettings[i].szName, PS_GETNAME, sizeof(szName), (LPARAM)szName );

		char* status;
		if ( protoSettings[i].status == ID_STATUS_LAST )
			status = Translate("<last>");
		else if ( protoSettings[i].status == ID_STATUS_CURRENT )
			status = Translate("<current>");
		else if ( protoSettings[i].status >= MIN_STATUS && protoSettings[i].status <= MAX_STATUS )
			status = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)protoSettings[i].status, (LPARAM)0);
		if (status == NULL)
			status = "<unknown>";
	
		desc = ( char* )realloc(desc, strlen(desc) + strlen(szName) + strlen(status) + 4);
		pnt = desc + strlen(desc);
		*pnt++ = '\r';
		strcpy(pnt, szName);
		pnt += strlen(szName);
		*pnt++ = ':';
		*pnt++ = ' ';
		strcpy(pnt, status);
	}
	
	return desc;
}

HRESULT CreateLink(TSettingsList& protoSettings) 
{
	HRESULT hres;
	IShellLink* psl;
	TCHAR savePath[MAX_PATH];
	char *args = GetCMDLArguments(protoSettings);
	char *desc = GetLinkDescription(protoSettings);

	if (MyGetSpecialFolderPath(NULL, savePath, 0x10, FALSE))
		_tcscat(savePath, _T(SHORTCUT_FILENAME));
	else
		_stprintf(savePath, _T(".\\%s"), _T(SHORTCUT_FILENAME));

	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, ( void** )&psl);
	if (SUCCEEDED(hres)) {
		// Set the path to the shortcut target, and add the
		// description.
		TCHAR path[MAX_PATH];
		GetModuleFileName(NULL, path, SIZEOF(path));
		psl->SetPath(path);
		#if defined( _UNICODE )
			TCHAR* p = mir_a2t( desc );
			psl->SetDescription( p );
			mir_free( p );
			p = mir_a2t( args );
			psl->SetArguments( p );
			mir_free( p );
		#else
			psl->SetDescription(desc);
			psl->SetArguments(args);
		#endif

		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		IPersistFile* ppf;
		hres = psl->QueryInterface(IID_IPersistFile, ( void** )&ppf);

		if (SUCCEEDED(hres)) {
			#if defined( _UNICODE )
				WCHAR* wsz = savePath;
			#else
				WCHAR wsz[MAX_PATH];
			
				// Ensure that the string is ANSI.
				MultiByteToWideChar(CP_ACP, 0, savePath, -1, wsz, MAX_PATH);
			#endif
			
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(wsz, TRUE);
			ppf->Release();
		}
		psl->Release();
	}

	free(args);
	free(desc);
	return hres;
}

INT_PTR CALLBACK CmdlOptionsDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{	
	static TSettingsList* optionsProtoSettings;
	
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			optionsProtoSettings = (TSettingsList*)lParam;
			char* cmdl = GetCMDL(*optionsProtoSettings);
			SetDlgItemTextA(hwndDlg, IDC_CMDL, cmdl);
			free(cmdl);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COPY:
			if ( OpenClipboard( hwndDlg )) {
				EmptyClipboard();

				char cmdl[2048];
				GetDlgItemTextA(hwndDlg,IDC_CMDL, cmdl, sizeof(cmdl));
				HGLOBAL cmdlGlob = GlobalAlloc(GMEM_MOVEABLE, sizeof(cmdl));
				if (cmdlGlob == NULL) {
					CloseClipboard();
					break;
				}
				LPTSTR cmdlStr = ( LPTSTR )GlobalLock(cmdlGlob);
				memcpy(cmdlStr, &cmdl, sizeof(cmdl));
				GlobalUnlock(cmdlGlob);
				SetClipboardData(CF_TEXT, cmdlGlob);
				CloseClipboard();
			}
			break;

		case IDC_SHORTCUT:
			CreateLink(*optionsProtoSettings);
			break;
					
		case IDC_OK:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	
	case WM_DESTROY:
		delete optionsProtoSettings; optionsProtoSettings = 0;
		break;
	}
	
	return 0;
}

static INT_PTR CALLBACK StartupStatusOptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static BOOL bInitDone = FALSE;

	switch(msg) {
	case WM_INITDIALOG:
		bInitDone = FALSE;

		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SETPROFILE, DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETPROFILE, 1)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_OVERRIDE, DBGetContactSettingByte(NULL, MODULENAME, SETTING_OVERRIDE, 1)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SHOWDIALOG, DBGetContactSettingByte(NULL, MODULENAME, SETTING_SHOWDIALOG, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETWINSTATE, DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSTATE, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETWINLOCATION, DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINLOCATION, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETDOCKED, DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETDOCKED, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETWINSIZE, DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSIZE, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_OFFLINECLOSE, DBGetContactSettingByte(NULL, MODULENAME, SETTING_OFFLINECLOSE, 1)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_AUTODIAL, DBGetContactSettingByte(NULL, MODULENAME, SETTING_AUTODIAL, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_AUTOHANGUP, DBGetContactSettingByte(NULL, MODULENAME, SETTING_AUTOHANGUP, 0)?TRUE:FALSE);
		SetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, DBGetContactSettingDword(NULL, MODULENAME, SETTING_SETPROFILEDELAY, 500), FALSE);
		SetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, DBGetContactSettingDword(NULL, MODULENAME, SETTING_DLGTIMEOUT, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_XPOS, DBGetContactSettingDword(NULL, MODULENAME, SETTING_XPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_YPOS, DBGetContactSettingDword(NULL, MODULENAME, SETTING_YPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_WIDTH, DBGetContactSettingDword(NULL, MODULENAME, SETTING_WIDTH, 0), FALSE);
		SetDlgItemInt(hwndDlg, IDC_HEIGHT, DBGetContactSettingDword(NULL, MODULENAME, SETTING_HEIGHT, 0), FALSE);
		{
			int val = DBGetContactSettingByte(NULL, MODULENAME, SETTING_DOCKED, DOCKED_NONE);
			int item = SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
			SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_LEFT);
			if (val == DOCKED_LEFT)
				SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

			item = SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
			SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_RIGHT);
			if (val == DOCKED_RIGHT)
				SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

			item = SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));
			SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_NONE);
			if (val == DOCKED_NONE)
				SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE)||IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILEDELAY), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIMEOUT), IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
		EnableWindow(GetDlgItem(hwndDlg, IDC_WINSTATE), IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_XPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
		EnableWindow(GetDlgItem(hwndDlg, IDC_YPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		SendMessage(hwndDlg, UM_REINITDOCKED, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSTATE, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSIZE, 0, 0);
		SetTimer(hwndDlg, 0, 100, NULL);
		bInitDone = TRUE;
		break;

	case WM_TIMER: 
		if ( !IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION) && !IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
			SetDlgItemTextA(hwndDlg, IDC_CURWINSIZE, "");
			SetDlgItemTextA(hwndDlg, IDC_CURWINLOC, "");
			break;
		}
		else {
			TCHAR text[128];
			mir_sntprintf(text, SIZEOF(text), TranslateT("size: %d x %d"), 
				DBGetContactSettingDword(NULL, MODULE_CLIST, SETTING_WIDTH, 0), 
				DBGetContactSettingDword(NULL, MODULE_CLIST, SETTING_HEIGHT, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINSIZE, text);

			mir_sntprintf(text, SIZEOF(text), TranslateT("loc: %d x %d"),
				DBGetContactSettingDword(NULL, MODULE_CLIST, SETTING_XPOS, 0),
				DBGetContactSettingDword(NULL, MODULE_CLIST, SETTING_YPOS, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINLOC, text);
	   }
		break;
		
	case UM_REINITPROFILES:
		// creates profile combo box according to 'dat'
		SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_RESETCONTENT, 0, 0);
		{
			int defProfile;
			int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
			for ( int i=0; i < profileCount; i++ ) {
				char profileName[128];
				if (GetProfileName((WPARAM)i, (LPARAM)profileName))
					continue;

				int item = SendDlgItemMessageA(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)profileName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETITEMDATA, (WPARAM)item, (LPARAM)i);
			}
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETCURSEL, (WPARAM)defProfile, 0);
		}
		break;
	
	case UM_REINITDOCKED:
		EnableWindow(GetDlgItem(hwndDlg, IDC_SETDOCKED), DBGetContactSettingByte(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 1));
		if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_SETDOCKED)))
			CheckDlgButton(hwndDlg, IDC_SETDOCKED, BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_DOCKED), IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
		break;

	case UM_REINITWINSTATE:
		{
			int val = DBGetContactSettingByte(NULL, MODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_RESETCONTENT, 0, 0);

			int item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hidden"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, (WPARAM)item, (LPARAM)SETTING_STATE_HIDDEN);
			if (val == SETTING_STATE_HIDDEN)
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, (WPARAM)item, 0);

			if (!DBGetContactSettingByte(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 0)) {
				item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Minimized"));
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, (WPARAM)item, (LPARAM)SETTING_STATE_MINIMIZED);
				if (val == SETTING_STATE_MINIMIZED)
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, (WPARAM)item, 0);
			}
			item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Normal"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, (WPARAM)item, (LPARAM)SETTING_STATE_NORMAL);
			if ( val == SETTING_STATE_NORMAL || (val == SETTING_STATE_MINIMIZED) && DBGetContactSettingByte(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, (WPARAM)item, 0);
		}
		break;

	case UM_REINITWINSIZE:
		EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTH), IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HEIGHT), !DBGetContactSettingByte(NULL, MODULE_CLUI, SETTING_AUTOSIZE, 0)&&IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
		
	case WM_COMMAND:
		if ( HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == LBN_SELCHANGE || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == EN_CHANGE )
			if ( bInitDone )
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_SETPROFILE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE)||IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILEDELAY), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			break;
		case IDC_SHOWDIALOG:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE)||IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIMEOUT), IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			break;
		case IDC_SETWINSTATE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_WINSTATE), IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
			break;
		case IDC_SETDOCKED:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DOCKED), IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
			break;
		case IDC_SETWINLOCATION:
			EnableWindow(GetDlgItem(hwndDlg, IDC_XPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_YPOS), IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			break;
		case IDC_SETWINSIZE:
			SendMessage(hwndDlg, UM_REINITWINSIZE, 0, 0);
			break;
		case IDC_SHOWCMDL:
			{
				int defProfile = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);

				TSettingsList* ps = GetCurrentProtoSettings();
				if ( ps ) {
					GetProfile( defProfile, *ps );
					for ( int i=0; i < ps->getCount(); i++ )
						if ( (*ps)[i].szMsg != NULL )
							(*ps)[i].szMsg = _strdup( (*ps)[i].szMsg );

					CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CMDLOPTIONS), hwndDlg, CmdlOptionsDlgProc, (LPARAM)ps);
				}
				break;
			}
		}
		break;
		
	case WM_SHOWWINDOW:
		if (wParam == FALSE)
			break;

		bInitDone = FALSE;
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		SendMessage(hwndDlg, UM_REINITDOCKED, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSTATE, 0, 0);
		SendMessage(hwndDlg, UM_REINITWINSIZE, 0, 0);
		bInitDone = TRUE;
		break;
			
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int val;
			
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_SETPROFILE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE))
				DBWriteContactSettingDword(NULL, MODULENAME, SETTING_SETPROFILEDELAY, GetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, NULL, FALSE));

			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				DBWriteContactSettingWord(NULL, MODULENAME, SETTING_DEFAULTPROFILE, (WORD)val);
			}
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_OVERRIDE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE));
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_SHOWDIALOG, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG))
				DBWriteContactSettingDword(NULL, MODULENAME, SETTING_DLGTIMEOUT, GetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, NULL, FALSE));

			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_SETWINSTATE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETCURSEL, 0, 0), 0);
				DBWriteContactSettingByte(NULL, MODULENAME, SETTING_WINSTATE, (BYTE)val);
			}
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_SETDOCKED, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETCURSEL, 0, 0), 0);
				DBWriteContactSettingByte(NULL, MODULENAME, SETTING_DOCKED, (BYTE)val);
			}
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_SETWINLOCATION, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION)) {
				DBWriteContactSettingDword(NULL, MODULENAME, SETTING_XPOS, GetDlgItemInt(hwndDlg, IDC_XPOS, NULL, TRUE));
				DBWriteContactSettingDword(NULL, MODULENAME, SETTING_YPOS, GetDlgItemInt(hwndDlg, IDC_YPOS, NULL, TRUE));
			}
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_SETWINSIZE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
				DBWriteContactSettingDword(NULL, MODULENAME, SETTING_WIDTH, GetDlgItemInt(hwndDlg, IDC_WIDTH, NULL, FALSE));
				DBWriteContactSettingDword(NULL, MODULENAME, SETTING_HEIGHT, GetDlgItemInt(hwndDlg, IDC_HEIGHT, NULL, FALSE));
			}
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_OFFLINECLOSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OFFLINECLOSE));
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_AUTODIAL, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTODIAL));
			DBWriteContactSettingByte(NULL, MODULENAME, SETTING_AUTOHANGUP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOHANGUP));
		}
		break;
	}
	
	return FALSE;
}

static INT_PTR CALLBACK StatusProfilesOptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	static PROFILEOPTIONS *dat = NULL;
	static int profileCount = 0;
	static BOOL bNeedRebuildMenu = FALSE;
	static BOOL bInitDone = FALSE;
	
	dat = (PROFILEOPTIONS *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch(msg) {
	case WM_INITDIALOG: {
		int i, j, defProfile;

		bInitDone = false;

		TranslateDialogDefault(hwndDlg);
		if (ServiceExists(MS_CLIST_ADDSTATUSMENUITEM))
			SetDlgItemText(hwndDlg, IDC_CREATEMMI, TranslateT("Create a status menu item"));

		profileCount = GetProfileCount((WPARAM)&defProfile, 0);
		if (profileCount == 0) {
			profileCount = 1;
			defProfile = 0;
		}
		dat = ( PROFILEOPTIONS* )malloc(profileCount*sizeof(PROFILEOPTIONS));
		ZeroMemory(dat, profileCount*sizeof(PROFILEOPTIONS));
		for (i=0;i<profileCount;i++) {
			dat[i].ps = GetCurrentProtoSettings();
			TSettingsList& ar = *dat[i].ps;

			if ( GetProfile( i, ar ) == -1) {
				/* create an empty profile */
				if (i == defProfile)
					dat[i].szName = _strdup(Translate("default"));
				else 
					dat[i].szName = _strdup(Translate("unknown"));
			}
			else {
				for ( j=0; j < ar.getCount(); j++ )
					if ( ar[j].szMsg != NULL)
						ar[j].szMsg = _strdup( ar[j].szMsg );

				dat[i].szName = dbi_getAs(i, SETTING_PROFILENAME, NULL);
				if (dat[i].szName == NULL) {
					if (i == defProfile)
						dat[i].szName = _strdup(Translate("default"));
					else
						dat[i].szName = _strdup(Translate("unknown"));
				}
				dat[i].createTtb = dbi_getb(i, SETTING_CREATETTBBUTTON, 0);
				dat[i].showDialog = dbi_getb(i, SETTING_SHOWCONFIRMDIALOG, 0);
				dat[i].createMmi = dbi_getb(i, SETTING_CREATEMMITEM, 0);
				dat[i].inSubMenu = dbi_getb(i, SETTING_INSUBMENU, 1);
				dat[i].regHotkey = dbi_getb(i, SETTING_REGHOTKEY, 0);
				dat[i].hotKey = dbi_getw(i, SETTING_HOTKEY, MAKEWORD((char)('0'+i), HOTKEYF_CONTROL|HOTKEYF_SHIFT));
			}
		}
		if ( !ServiceExists( MS_TTB_ADDBUTTON ) && !ServiceExists( MS_TB_ADDBUTTON ))
			EnableWindow(GetDlgItem(hwndDlg, IDC_CREATETTB), FALSE);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		ShowWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELPEX)?SW_SHOW:SW_HIDE);
		bInitDone = true;
		break;
	}

	case UM_REINITPROFILES: 
		bInitDone = false;
		{
			// creates profile combo box according to 'dat'
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_RESETCONTENT, 0, 0);
			for (int i=0; i < profileCount; i++ ) {
				int item = SendDlgItemMessageA(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)dat[i].szName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETITEMDATA, (WPARAM)item, (LPARAM)i);
			}
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, UM_SETPROFILE, 0, 0);
		}
		bInitDone = true;
		break;
	
	case UM_SETPROFILE:
		{
			int sel = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, 
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
			CheckDlgButton(hwndDlg, IDC_CREATETTB, dat[sel].createTtb?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWDIALOG, dat[sel].showDialog?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CREATEMMI, dat[sel].createMmi?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_INSUBMENU, dat[sel].inSubMenu?BST_CHECKED:BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_INSUBMENU), IsDlgButtonChecked(hwndDlg, IDC_CREATEMMI));
			CheckDlgButton(hwndDlg, IDC_REGHOTKEY, dat[sel].regHotkey?BST_CHECKED:BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_SETHOTKEY, dat[sel].hotKey, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), IsDlgButtonChecked(hwndDlg, IDC_REGHOTKEY));
			SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_RESETCONTENT, 0, 0);

			// fill proto list
			TSettingsList& ar = *dat[sel].ps;
			for ( int i=0; i < ar.getCount(); i++ ) {
				int item = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_ADDSTRING, 0, (LPARAM)ar[i].tszAccName );
				SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_SETITEMDATA, (WPARAM)item, (LPARAM)&ar[i]);
			}
			SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, UM_SETPROTOCOL, 0, 0);
		}		
		break;

	case UM_SETPROTOCOL:
		{
			int idx = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETCURSEL, 0, 0);
			if ( idx != -1 ) {
				// fill status box
				TSSSetting* ps = ( TSSSetting* )SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETITEMDATA, idx, 0);

				int flags = (CallProtoService(ps->szName, PS_GETCAPS, PFLAGNUM_2, 0))&~(CallProtoService(ps->szName, PS_GETCAPS, PFLAGNUM_5, 0));
				SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_RESETCONTENT, 0, 0);
				for ( int i=0; i < SIZEOF(statusModeList); i++ ) {
					if ( (flags&statusModePf2List[i]) || (statusModeList[i] == ID_STATUS_OFFLINE)) {
						TCHAR* szStatus = ( TCHAR* )CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, statusModeList[i], GSMDF_TCHAR);
						if ( szStatus != NULL ) {
							int item = SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_ADDSTRING, 0, (LPARAM)szStatus);
							SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETITEMDATA, (WPARAM)item, (LPARAM)statusModeList[i]);
							if (ps->status == statusModeList[i])
								SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETCURSEL, (WPARAM)item, 0);
				}	}	}

				int item = SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_ADDSTRING, 0, (LPARAM)TranslateT("<current>"));
				SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETITEMDATA, (WPARAM)item, (LPARAM)ID_STATUS_CURRENT);
				if (ps->status == ID_STATUS_CURRENT)
					SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETCURSEL, (WPARAM)item, 0);

				item = SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_ADDSTRING, 0, (LPARAM)TranslateT("<last>"));
				SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETITEMDATA, (WPARAM)item, (LPARAM)ID_STATUS_LAST);
				if (ps->status == ID_STATUS_LAST)
					SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETCURSEL, (WPARAM)item, 0);
			}

			SendMessage(hwndDlg, UM_SETSTATUSMSG, 0, 0);
		}
		break;

	case UM_SETSTATUSMSG:
		{
			// set status message
			BOOL bStatusMsg = FALSE;
			int idx = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETCURSEL, 0, 0);
			if ( idx != -1 ) {
				TSSSetting* ps = ( TSSSetting* )SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETITEMDATA, idx, 0);

				CheckRadioButton(hwndDlg, IDC_MIRANDAMSG, IDC_CUSTOMMSG, ps->szMsg!=NULL?IDC_CUSTOMMSG:IDC_MIRANDAMSG);
				if (ps->szMsg != NULL)
					SetDlgItemTextA(hwndDlg, IDC_STATUSMSG, ps->szMsg);

				bStatusMsg = ( (((CallProtoService(ps->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND&~PF1_INDIVMODEMSG)) && 
					(CallProtoService(ps->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(ps->status))) || (ps->status == ID_STATUS_CURRENT) || (ps->status == ID_STATUS_LAST) );
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIRANDAMSG), bStatusMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMMSG), bStatusMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), bStatusMsg&&IsDlgButtonChecked(hwndDlg, IDC_CUSTOMMSG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), bStatusMsg&&IsDlgButtonChecked(hwndDlg, IDC_CUSTOMMSG));
		}
		break;
		
	case UM_ADDPROFILE:
		{
			char *szName = (char *)lParam;
			if (szName == NULL)
				break;

			dat = ( PROFILEOPTIONS* )realloc(dat, (profileCount+1)*sizeof(PROFILEOPTIONS));
			ZeroMemory(&dat[profileCount], sizeof(PROFILEOPTIONS));
			dat[profileCount].szName = _strdup(szName);
			dat[profileCount].ps = GetCurrentProtoSettings();
			profileCount += 1;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		}
		break;

	case UM_DELPROFILE: {
		// wparam == profile no
		int i, defProfile;

		i = (int)wParam;
		if (profileCount == 1) {
			MessageBox(NULL, TranslateT("At least one profile must exist"), TranslateT("StartupStatus"), MB_OK);
			break;
		}

		if (dat[i].ps != NULL)
			dat[i].ps->destroy();

		if (dat[i].szName != NULL)
			free(dat[i].szName);

		MoveMemory(&dat[i], &dat[i+1], (profileCount-i-1)*sizeof(PROFILEOPTIONS));		
		dat = ( PROFILEOPTIONS* )realloc(dat, (profileCount-1)*sizeof(PROFILEOPTIONS));
		profileCount -= 1;
		GetProfileCount((WPARAM)&defProfile, 0);
		if (i == defProfile) {
			MessageBox(NULL, TranslateT("Your default profile will be changed"), TranslateT("StartupStatus"), MB_OK);
			DBWriteContactSettingWord(NULL, MODULENAME, SETTING_DEFAULTPROFILE, 0);
			defProfile = 0;
		}
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		break;
	}

	case WM_COMMAND:
		if ( ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == LBN_SELCHANGE)) && ((HWND)lParam == GetFocus()) )
			if ( bInitDone )
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,0,0);

		switch (LOWORD(wParam)) {
		case IDC_STATUS:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int idx = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETCURSEL, 0, 0);
				if ( idx != -1 ) {
					TSSSetting* ps = ( TSSSetting* )SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETITEMDATA, idx, 0);
					ps->status = (int)SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_GETITEMDATA, 
						SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_GETCURSEL, 0, 0), 0);
				}
				SendMessage(hwndDlg, UM_SETSTATUSMSG, 0, 0);
			}
			break;

		case IDC_PROFILE:
			if (HIWORD(wParam) != CBN_SELCHANGE)
				break;

			SendMessage(hwndDlg, UM_SETPROFILE, 0, 0);
			break;
			
		case IDC_PROTOCOL:
			if (HIWORD(wParam) != LBN_SELCHANGE)
				break;

			SendMessage(hwndDlg, UM_SETPROTOCOL, 0, 0);
			break;

		case IDC_MIRANDAMSG:
		case IDC_CUSTOMMSG:
			{
				int len;
				TSSSetting* ps = ( TSSSetting* )SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETCURSEL, 0, 0), 0);
				if (ps->szMsg != NULL)
					free(ps->szMsg);

				ps->szMsg = NULL;
				if (IsDlgButtonChecked(hwndDlg, IDC_CUSTOMMSG)) {
					len = SendDlgItemMessage(hwndDlg, IDC_STATUSMSG, WM_GETTEXTLENGTH, 0, 0);
					ps->szMsg = (char *)malloc(len+1);
					memset(ps->szMsg, '\0', len+1);
					SendDlgItemMessage(hwndDlg, IDC_STATUSMSG, WM_GETTEXT, (WPARAM)len+1, (LPARAM)ps->szMsg);				
				}
				SendMessage(hwndDlg, UM_SETSTATUSMSG, 0, 0);
			}
			break;

		case IDC_STATUSMSG:
			if (HIWORD(wParam) == EN_CHANGE) {
				// update the status message in memory, this is done on each character tick, not nice
				// but it works
				TSSSetting* ps = ( TSSSetting* )SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_GETCURSEL, 0, 0), 0);
				if (ps->szMsg != NULL) {
					if ( *ps->szMsg )
						free(ps->szMsg);
					ps->szMsg = NULL;
				}
				int len = SendDlgItemMessageA(hwndDlg, IDC_STATUSMSG, WM_GETTEXTLENGTH, 0, 0);
				ps->szMsg = (char *)malloc(len+1);
				memset(ps->szMsg, '\0', len+1);
				SendDlgItemMessageA(hwndDlg, IDC_STATUSMSG, WM_GETTEXT, (WPARAM)len+1, (LPARAM)ps->szMsg);				
			}
			break;
		
		case IDC_CREATEMMI:
			EnableWindow(GetDlgItem(hwndDlg, IDC_INSUBMENU), IsDlgButtonChecked(hwndDlg, IDC_CREATEMMI));
		case IDC_INSUBMENU:
			bNeedRebuildMenu = TRUE;
		case IDC_REGHOTKEY:
		case IDC_CREATETTB:
		case IDC_SHOWDIALOG:
			{
				int sel = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				dat[sel].createMmi = IsDlgButtonChecked(hwndDlg, IDC_CREATEMMI);
				dat[sel].inSubMenu = IsDlgButtonChecked(hwndDlg, IDC_INSUBMENU);
				dat[sel].createTtb = IsDlgButtonChecked(hwndDlg, IDC_CREATETTB);
				dat[sel].regHotkey = IsDlgButtonChecked(hwndDlg, IDC_REGHOTKEY);
				dat[sel].showDialog = IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG);
				EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), IsDlgButtonChecked(hwndDlg, IDC_REGHOTKEY));
			}
			break;

		case IDC_HOTKEY:
			if (HIWORD(wParam) == EN_CHANGE) {
				int sel = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				dat[sel].hotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);
			}
			break;
		
		case IDC_ADDPROFILE:
			// add a profile
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDPROFILE), hwndDlg, addProfileDlgProc, (LPARAM)hwndDlg);
			EnableWindow(hwndDlg, FALSE);
			break;

		case IDC_DELPROFILE:
			{
				int sel = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, 
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				SendMessage(hwndDlg, UM_DELPROFILE, (WPARAM)sel, 0);
			}
			break;

		case IDC_VARIABLESHELP:
			variables_showhelp(hwndDlg, IDC_STATUSMSG, VHF_INPUT|VHF_EXTRATEXT|VHF_HELP|VHF_FULLFILLSTRUCT|VHF_HIDESUBJECTTOKEN, NULL, "Protocol ID");
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int oldCount, i, j;
			char setting[128];
			
			oldCount = DBGetContactSettingWord(NULL, MODULENAME, SETTING_PROFILECOUNT, 0);		
			for (i=0;i<oldCount;i++) {
				mir_snprintf(setting, sizeof(setting), "%d_", i);
				ClearDatabase(setting);
			}
			for (i=0;i<profileCount;i++) {
				dbi_setb(i, SETTING_SHOWCONFIRMDIALOG, dat[i].showDialog);
				dbi_setb(i, SETTING_CREATETTBBUTTON, dat[i].createTtb);
				dbi_setb(i, SETTING_CREATEMMITEM, dat[i].createMmi);
				dbi_setb(i, SETTING_INSUBMENU, dat[i].inSubMenu);
				dbi_setb(i, SETTING_REGHOTKEY, dat[i].regHotkey);
				dbi_setw(i, SETTING_HOTKEY, dat[i].hotKey);
				dbi_setAs(i, SETTING_PROFILENAME, dat[i].szName);

				TSettingsList& ar = *dat[i].ps;
				for ( j=0; j < ar.getCount(); j++ ) {
					if ( ar[j].szMsg != NULL ) {
						mir_snprintf(setting, sizeof(setting), "%s_%s", ar[j].szName, SETTING_PROFILE_STSMSG);
						dbi_setAs(i, setting, ar[j].szMsg);
					}
					dbi_setw(i, ar[j].szName, ar[j].status);
				}
			}
			DBWriteContactSettingWord(NULL, MODULENAME, SETTING_PROFILECOUNT, (WORD)profileCount);

			if (bNeedRebuildMenu) {
				// Rebuild status menu
				CLIST_INTERFACE* pcli = ( CLIST_INTERFACE* )CallService( MS_CLIST_RETRIEVE_INTERFACE, 0, 0 );
				if ( pcli && pcli->version > 4 )
					pcli->pfnReloadProtoMenus();				
			}
			LoadMainOptions();
		}
		break;

	case WM_DESTROY:
		{
			for ( int i=0; i < profileCount; i++ ) {
				delete dat[i].ps;

				if (dat[i].szName != NULL)
					free(dat[i].szName);
			}
			free( dat ); dat = NULL;
			break;
	}	}
	
	return 0;
}

INT_PTR CALLBACK addProfileDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HWND hwndParent;
	
	switch(msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			hwndParent = (HWND)lParam;
			EnableWindow(GetDlgItem(hwndDlg, IDC_OK), FALSE);
		break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_OK) {
				char profileName[128];
				GetDlgItemTextA(hwndDlg, IDC_PROFILENAME, profileName, sizeof(profileName));
				SendMessage(hwndParent, UM_ADDPROFILE, 0, (LPARAM)profileName);
				// done and exit
				DestroyWindow(hwndDlg);	
			}
			else if (LOWORD(wParam) == IDC_CANCEL) {
				DestroyWindow(hwndDlg);
			}
			else if (LOWORD(wParam) == IDC_PROFILENAME) {
				(SendDlgItemMessage(hwndDlg,IDC_PROFILENAME,EM_LINELENGTH,0,0) > 0)?EnableWindow(GetDlgItem(hwndDlg, IDC_OK), TRUE):EnableWindow(GetDlgItem(hwndDlg, IDC_OK), FALSE);
			}
		break;

		case WM_DESTROY:
			EnableWindow(hwndParent, TRUE);
		break;
	}

	return 0;
}

int OptionsInit(WPARAM wparam,LPARAM lparam)
{	
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize      = sizeof(odp);
	odp.hInstance   = hInst;
	odp.pszGroup    = LPGEN("Status");
	odp.pszTitle    = LPGEN("StartupStatus");
	odp.flags       = ODPF_BOLDGROUPS;

	odp.pszTab      = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STARTUPSTATUS);
	odp.pfnDlgProc  = StartupStatusOptDlgProc;
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);

	odp.pszTab      = LPGEN("Status Profiles");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STATUSPROFILES);
	odp.pfnDlgProc  = StatusProfilesOptDlgProc;
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);

	return 0;
}

// from: http://www.codeproject.com/winhelp/msdnintegrator/
#ifdef _UNICODE
	static const CHAR _szSpecialFolderPath[] = "SHGetSpecialFolderPathW";
#else
	static const CHAR _szSpecialFolderPath[] = "SHGetSpecialFolderPathA";
#endif

BOOL MyGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate)
{
	typedef int (WINAPI* _SHGET)(HWND, LPTSTR, int, BOOL);

	// Get a function pointer to SHGetSpecialFolderPath(...) from
	// Shell32.dll, if this returns NULL then check ShFolder.dll...
	_SHGET pfn = (_SHGET)GetProcAddress( LoadLibraryA( "Shell32.dll" ), _szSpecialFolderPath);
	if (pfn == NULL) {
		// Try to get the function pointer from ShFolder.dll, if pfn is still
		// NULL then we have bigger problems...
		pfn = (_SHGET)GetProcAddress( LoadLibraryA("ShFolder.dll"), _szSpecialFolderPath);
		if (pfn == NULL)
			return FALSE;
	}

	return pfn(hwndOwner, lpszPath, nFolder, fCreate);
}

// end: from: http://www.codeproject.com/winhelp/msdnintegrator/
static int ClearDatabase(char* filter)
{
	DBCONTACTENUMSETTINGS dbces;
	char** settings;
	int i, settingCount;

	settingCount = 0;
	settingIndex = 0;
	dbces.szModule = MODULENAME;
	dbces.lParam = (LPARAM)&settingCount;
	dbces.pfnEnumProc = CountSettings;
	CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)NULL,(LPARAM)&dbces);
	
	settings = ( char** )malloc(settingCount*sizeof(char*));
	dbces.lParam = (LPARAM)&settings;
	dbces.pfnEnumProc = DeleteSetting;
	CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)NULL,(LPARAM)&dbces);
	for (i=0;i<settingCount;i++) {
		if ((filter == NULL) || (!strncmp(filter, settings[i], strlen(filter))))
			DBDeleteContactSetting(NULL, MODULENAME, settings[i]);
		free(settings[i]);
	}
	free(settings);
	// < v0.0.0.9
	if (filter == NULL)	DBDeleteContactSetting(NULL, "AutoAway", "Confirm");

	return 0;
}

static int CountSettings(const char *szSetting,LPARAM lParam)
{
	*(int *)lParam += 1;

	return 0;
}

static int DeleteSetting(const char *szSetting,LPARAM lParam)
{
	char** settings = (char**)*(char ***)lParam;
	settings[settingIndex] = ( char* )malloc(strlen(szSetting)+1);
	strcpy(settings[settingIndex], szSetting);
	settingIndex += 1;
	
	return 0;
}
