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

static int CountSettings(const char *szSetting,LPARAM lParam);
static int DeleteSetting(const char *szSetting,LPARAM lParam);
static int ClearDatabase(char* filter);

/////////////////////////////////////////////////////////////////////////////////////////

static TSettingsList* GetCurrentProtoSettings()
{
	int count;
	PROTOACCOUNT **protos;
	ProtoEnumAccounts(&count, &protos);

	TSettingsList *result = new TSettingsList(count, CompareSettings);
	if (result == NULL)
		return NULL;

	for (int i=0; i < count; i++)
		if ( IsSuitableProto(protos[i]))
			result->insert( new TSSSetting(protos[i]));

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

	if ( db_get_b( NULL, MODULENAME, SETTING_SHOWDIALOG, FALSE ) == TRUE ) {
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
	mir_snprintf(cmdl, strlen(path) + 4, "\"%s\" ", path);

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

static TCHAR* GetLinkDescription(TSettingsList& protoSettings)
{
	if ( protoSettings.getCount() == 0 )
		return NULL;

	CMString result(SHORTCUT_DESC);
	for (int i=0; i < protoSettings.getCount(); i++) {
		TSSSetting &p = protoSettings[i];

		TCHAR *status;
		if ( p.status == ID_STATUS_LAST)
			status = TranslateT("<last>");
		else if (p.status == ID_STATUS_CURRENT)
			status = TranslateT("<current>");
		else if (p.status >= MIN_STATUS && p.status <= MAX_STATUS )
			status = pcli->pfnGetStatusModeDescription(p.status, 0);
		if (status == NULL)
			status = TranslateT("<unknown>");

		result.AppendChar('\r');
		result.Append(p.tszAccName);
		result.AppendChar(':');
		result.AppendChar(' ');
		result.Append(status);
	}

	return mir_tstrndup(result, result.GetLength());
}

HRESULT CreateLink(TSettingsList& protoSettings)
{
	TCHAR savePath[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, savePath, 0x10, FALSE))
		_tcsncat(savePath, SHORTCUT_FILENAME, SIZEOF(savePath));
	else
		mir_sntprintf(savePath, SIZEOF(savePath), _T(".\\%s"), SHORTCUT_FILENAME);

	// Get a pointer to the IShellLink interface.
	IShellLink *psl;
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, ( void** )&psl);
	if (SUCCEEDED(hres)) {
		char *args = GetCMDLArguments(protoSettings);
		TCHAR *desc = GetLinkDescription(protoSettings);

		// Set the path to the shortcut target, and add the
		// description.
		TCHAR path[MAX_PATH];
		GetModuleFileName(NULL, path, SIZEOF(path));
		psl->SetPath(path);
		psl->SetDescription(desc);
		psl->SetArguments( _A2T(args));

		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		IPersistFile *ppf;
		hres = psl->QueryInterface(IID_IPersistFile, ( void** )&ppf);

		if (SUCCEEDED(hres)) {
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(savePath, TRUE);
			ppf->Release();
		}
		psl->Release();
		free(args);
		free(desc);
	}

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
				GetDlgItemTextA(hwndDlg,IDC_CMDL, cmdl, SIZEOF(cmdl));
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
		CheckDlgButton(hwndDlg, IDC_SETPROFILE, db_get_b(NULL, MODULENAME, SETTING_SETPROFILE, 1)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_OVERRIDE, db_get_b(NULL, MODULENAME, SETTING_OVERRIDE, 1)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SHOWDIALOG, db_get_b(NULL, MODULENAME, SETTING_SHOWDIALOG, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETWINSTATE, db_get_b(NULL, MODULENAME, SETTING_SETWINSTATE, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETWINLOCATION, db_get_b(NULL, MODULENAME, SETTING_SETWINLOCATION, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETDOCKED, db_get_b(NULL, MODULENAME, SETTING_SETDOCKED, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_SETWINSIZE, db_get_b(NULL, MODULENAME, SETTING_SETWINSIZE, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_OFFLINECLOSE, db_get_b(NULL, MODULENAME, SETTING_OFFLINECLOSE, 1)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_AUTODIAL, db_get_b(NULL, MODULENAME, SETTING_AUTODIAL, 0)?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_AUTOHANGUP, db_get_b(NULL, MODULENAME, SETTING_AUTOHANGUP, 0)?TRUE:FALSE);
		SetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, db_get_dw(NULL, MODULENAME, SETTING_SETPROFILEDELAY, 500), FALSE);
		SetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, db_get_dw(NULL, MODULENAME, SETTING_DLGTIMEOUT, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_XPOS, db_get_dw(NULL, MODULENAME, SETTING_XPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_YPOS, db_get_dw(NULL, MODULENAME, SETTING_YPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_WIDTH, db_get_dw(NULL, MODULENAME, SETTING_WIDTH, 0), FALSE);
		SetDlgItemInt(hwndDlg, IDC_HEIGHT, db_get_dw(NULL, MODULENAME, SETTING_HEIGHT, 0), FALSE);
		{
			int val = db_get_b(NULL, MODULENAME, SETTING_DOCKED, DOCKED_NONE);
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
				db_get_dw(NULL, MODULE_CLIST, SETTING_WIDTH, 0),
				db_get_dw(NULL, MODULE_CLIST, SETTING_HEIGHT, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINSIZE, text);

			mir_sntprintf(text, SIZEOF(text), TranslateT("loc: %d x %d"),
				db_get_dw(NULL, MODULE_CLIST, SETTING_XPOS, 0),
				db_get_dw(NULL, MODULE_CLIST, SETTING_YPOS, 0));
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
				TCHAR profileName[128];
				if ( GetProfileName(i, (LPARAM)profileName))
					continue;

				int item = SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)profileName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETITEMDATA, item, i);
			}
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETCURSEL, defProfile, 0);
		}
		break;

	case UM_REINITDOCKED:
		EnableWindow(GetDlgItem(hwndDlg, IDC_SETDOCKED), db_get_b(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 1));
		if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_SETDOCKED)))
			CheckDlgButton(hwndDlg, IDC_SETDOCKED, BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_DOCKED), IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
		break;

	case UM_REINITWINSTATE:
		{
			int val = db_get_b(NULL, MODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_RESETCONTENT, 0, 0);

			int item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hidden"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, (LPARAM)SETTING_STATE_HIDDEN);
			if (val == SETTING_STATE_HIDDEN)
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);

			if (!db_get_b(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 0)) {
				item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Minimized"));
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_MINIMIZED);
				if (val == SETTING_STATE_MINIMIZED)
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);
			}
			item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Normal"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_NORMAL);
			if ( val == SETTING_STATE_NORMAL || (val == SETTING_STATE_MINIMIZED) && db_get_b(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);
		}
		break;

	case UM_REINITWINSIZE:
		EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTH), IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HEIGHT), !db_get_b(NULL, MODULE_CLUI, SETTING_AUTOSIZE, 0)&&IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));

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
							(*ps)[i].szMsg = _tcsdup( (*ps)[i].szMsg );

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

			db_set_b(NULL, MODULENAME, SETTING_SETPROFILE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE))
				db_set_dw(NULL, MODULENAME, SETTING_SETPROFILEDELAY, GetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, NULL, FALSE));

			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				db_set_w(NULL, MODULENAME, SETTING_DEFAULTPROFILE, (WORD)val);
			}
			db_set_b(NULL, MODULENAME, SETTING_OVERRIDE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE));
			db_set_b(NULL, MODULENAME, SETTING_SHOWDIALOG, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG))
				db_set_dw(NULL, MODULENAME, SETTING_DLGTIMEOUT, GetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, NULL, FALSE));

			db_set_b(NULL, MODULENAME, SETTING_SETWINSTATE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETCURSEL, 0, 0), 0);
				db_set_b(NULL, MODULENAME, SETTING_WINSTATE, (BYTE)val);
			}
			db_set_b(NULL, MODULENAME, SETTING_SETDOCKED, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETCURSEL, 0, 0), 0);
				db_set_b(NULL, MODULENAME, SETTING_DOCKED, (BYTE)val);
			}
			db_set_b(NULL, MODULENAME, SETTING_SETWINLOCATION, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION)) {
				db_set_dw(NULL, MODULENAME, SETTING_XPOS, GetDlgItemInt(hwndDlg, IDC_XPOS, NULL, TRUE));
				db_set_dw(NULL, MODULENAME, SETTING_YPOS, GetDlgItemInt(hwndDlg, IDC_YPOS, NULL, TRUE));
			}
			db_set_b(NULL, MODULENAME, SETTING_SETWINSIZE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
				db_set_dw(NULL, MODULENAME, SETTING_WIDTH, GetDlgItemInt(hwndDlg, IDC_WIDTH, NULL, FALSE));
				db_set_dw(NULL, MODULENAME, SETTING_HEIGHT, GetDlgItemInt(hwndDlg, IDC_HEIGHT, NULL, FALSE));
			}
			db_set_b(NULL, MODULENAME, SETTING_OFFLINECLOSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OFFLINECLOSE));
			db_set_b(NULL, MODULENAME, SETTING_AUTODIAL, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTODIAL));
			db_set_b(NULL, MODULENAME, SETTING_AUTOHANGUP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOHANGUP));
		}
		break;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

static OBJLIST<PROFILEOPTIONS> arProfiles(5);

static INT_PTR CALLBACK StatusProfilesOptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static BOOL bNeedRebuildMenu = FALSE;
	static BOOL bInitDone = FALSE;

	switch(msg) {
	case WM_INITDIALOG:
		bInitDone = false;

		TranslateDialogDefault(hwndDlg);
		SetDlgItemText(hwndDlg, IDC_CREATEMMI, TranslateT("Create a status menu item"));
		{
			int defProfile;
			int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
			if (profileCount == 0) {
				profileCount = 1;
				defProfile = 0;
			}

			for (int i=0; i < profileCount; i++) {
				PROFILEOPTIONS *ppo = new PROFILEOPTIONS;
				ppo->ps = GetCurrentProtoSettings();
				TSettingsList& ar = *ppo->ps;

				if ( GetProfile(i, ar) == -1) {
					/* create an empty profile */
					if (i == defProfile)
						ppo->tszName = mir_tstrdup( TranslateT("default"));
					else
						ppo->tszName = mir_tstrdup( TranslateT("unknown"));
				}
				else {
					for (int j=0; j < ar.getCount(); j++)
						if ( ar[j].szMsg != NULL)
							ar[j].szMsg = _tcsdup( ar[j].szMsg );

					ppo->tszName = db_get_tsa(NULL, MODULENAME, OptName(i, SETTING_PROFILENAME));
					if (ppo->tszName == NULL) {
						if (i == defProfile)
							ppo->tszName = mir_tstrdup( TranslateT("default"));
						else
							ppo->tszName = mir_tstrdup( TranslateT("unknown"));
					}
					ppo->createTtb = db_get_b(NULL, MODULENAME, OptName(i, SETTING_CREATETTBBUTTON), 0);
					ppo->showDialog = db_get_b(NULL, MODULENAME, OptName(i, SETTING_SHOWCONFIRMDIALOG), 0);
					ppo->createMmi = db_get_b(NULL, MODULENAME, OptName(i, SETTING_CREATEMMITEM), 0);
					ppo->inSubMenu = db_get_b(NULL, MODULENAME, OptName(i, SETTING_INSUBMENU), 1);
					ppo->regHotkey = db_get_b(NULL, MODULENAME, OptName(i, SETTING_REGHOTKEY), 0);
					ppo->hotKey = db_get_w(NULL, MODULENAME, OptName(i, SETTING_HOTKEY), MAKEWORD((char)('0'+i), HOTKEYF_CONTROL|HOTKEYF_SHIFT));
				}
				arProfiles.insert(ppo);
			}
			if (hTTBModuleLoadedHook == NULL)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CREATETTB), FALSE);

			SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
			ShowWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELPEX)?SW_SHOW:SW_HIDE);
			bInitDone = true;
		}
		break;

	case UM_REINITPROFILES:
		bInitDone = false;
		{
			// creates profile combo box according to 'dat'
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_RESETCONTENT, 0, 0);
			for (int i=0; i < arProfiles.getCount(); i++ ) {
				int item = SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)arProfiles[i].tszName);
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
			CheckDlgButton(hwndDlg, IDC_CREATETTB, arProfiles[sel].createTtb?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWDIALOG, arProfiles[sel].showDialog?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CREATEMMI, arProfiles[sel].createMmi?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_INSUBMENU, arProfiles[sel].inSubMenu?BST_CHECKED:BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_INSUBMENU), IsDlgButtonChecked(hwndDlg, IDC_CREATEMMI));
			CheckDlgButton(hwndDlg, IDC_REGHOTKEY, arProfiles[sel].regHotkey?BST_CHECKED:BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_SETHOTKEY, arProfiles[sel].hotKey, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), IsDlgButtonChecked(hwndDlg, IDC_REGHOTKEY));
			SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, LB_RESETCONTENT, 0, 0);

			// fill proto list
			TSettingsList& ar = *arProfiles[sel].ps;
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
						int item = SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(statusModeList[i], 0));
						SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETITEMDATA, (WPARAM)item, (LPARAM)statusModeList[i]);
						if (ps->status == statusModeList[i])
							SendDlgItemMessage(hwndDlg, IDC_STATUS, LB_SETCURSEL, (WPARAM)item, 0);
					}
				}

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
					SetDlgItemText(hwndDlg, IDC_STATUSMSG, ps->szMsg);

				bStatusMsg = ( (((CallProtoService(ps->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND&~PF1_INDIVMODEMSG)) &&
					(CallProtoService(ps->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(ps->status))) || (ps->status == ID_STATUS_CURRENT) || (ps->status == ID_STATUS_LAST));
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIRANDAMSG), bStatusMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMMSG), bStatusMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), bStatusMsg&&IsDlgButtonChecked(hwndDlg, IDC_CUSTOMMSG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), bStatusMsg&&IsDlgButtonChecked(hwndDlg, IDC_CUSTOMMSG));
		}
		break;

	case UM_ADDPROFILE:
		{
			TCHAR *tszName = (TCHAR*)lParam;
			if (tszName == NULL)
				break;

			PROFILEOPTIONS* ppo = new PROFILEOPTIONS;
			ppo->tszName = mir_tstrdup(tszName);
			ppo->ps = GetCurrentProtoSettings();
			arProfiles.insert(ppo);

			SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		}
		break;

	case UM_DELPROFILE: {
		// wparam == profile no
		int i=(int)wParam;

		if ( arProfiles.getCount() == 1) {
			MessageBox(NULL, TranslateT("At least one profile must exist"), TranslateT("StartupStatus"), MB_OK);
			break;
		}

		arProfiles.remove(i);

		int defProfile;
		GetProfileCount((WPARAM)&defProfile, 0);
		if (i == defProfile) {
			MessageBox(NULL, TranslateT("Your default profile will be changed"), TranslateT("StartupStatus"), MB_OK);
			db_set_w(NULL, MODULENAME, SETTING_DEFAULTPROFILE, 0);
		}
		SendMessage(hwndDlg, UM_REINITPROFILES, 0, 0);
		break;
	}

	case WM_COMMAND:
		if ( ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == LBN_SELCHANGE)) && ((HWND)lParam == GetFocus()))
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
					ps->szMsg = (TCHAR*)calloc(sizeof(TCHAR), len+1);
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
				ps->szMsg = (TCHAR*)calloc(sizeof(TCHAR), len+1);
				SendDlgItemMessage(hwndDlg, IDC_STATUSMSG, WM_GETTEXT, (WPARAM)len+1, (LPARAM)ps->szMsg);
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
				int sel = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				PROFILEOPTIONS& po = arProfiles[sel];
				po.createMmi = IsDlgButtonChecked(hwndDlg, IDC_CREATEMMI);
				po.inSubMenu = IsDlgButtonChecked(hwndDlg, IDC_INSUBMENU);
				po.createTtb = IsDlgButtonChecked(hwndDlg, IDC_CREATETTB);
				po.regHotkey = IsDlgButtonChecked(hwndDlg, IDC_REGHOTKEY);
				po.showDialog = IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG);
				EnableWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), IsDlgButtonChecked(hwndDlg, IDC_REGHOTKEY));
			}
			break;

		case IDC_HOTKEY:
			if (HIWORD(wParam) == EN_CHANGE) {
				int sel = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				arProfiles[sel].hotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);
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
			char setting[128];
			int i, oldCount = db_get_w(NULL, MODULENAME, SETTING_PROFILECOUNT, 0);
			for (i=0; i < oldCount; i++) {
				mir_snprintf(setting, SIZEOF(setting), "%d_", i);
				ClearDatabase(setting);
			}
			for (i=0; i < arProfiles.getCount(); i++) {
				PROFILEOPTIONS& po = arProfiles[i];
				db_set_b(NULL, MODULENAME, OptName(i, SETTING_SHOWCONFIRMDIALOG), po.showDialog);
				db_set_b(NULL, MODULENAME, OptName(i, SETTING_CREATETTBBUTTON), po.createTtb);
				db_set_b(NULL, MODULENAME, OptName(i, SETTING_CREATEMMITEM), po.createMmi);
				db_set_b(NULL, MODULENAME, OptName(i, SETTING_INSUBMENU), po.inSubMenu);
				db_set_b(NULL, MODULENAME, OptName(i, SETTING_REGHOTKEY), po.regHotkey);
				db_set_w(NULL, MODULENAME, OptName(i, SETTING_HOTKEY), po.hotKey);
				db_set_ts(NULL, MODULENAME, OptName(i, SETTING_PROFILENAME), po.tszName);

				TSettingsList& ar = *po.ps;
				for (int j=0; j < ar.getCount(); j++) {
					if ( ar[j].szMsg != NULL ) {
						mir_snprintf(setting, SIZEOF(setting), "%s_%s", ar[j].szName, SETTING_PROFILE_STSMSG);
						db_set_ts(NULL, MODULENAME, OptName(i, setting), ar[j].szMsg);
					}
					db_set_w(NULL, MODULENAME, OptName(i, ar[j].szName), ar[j].status);
				}
			}
			db_set_w(NULL, MODULENAME, SETTING_PROFILECOUNT, (WORD)arProfiles.getCount());

			// Rebuild status menu
			if (bNeedRebuildMenu)
				pcli->pfnReloadProtoMenus();

			LoadMainOptions();
		}
		break;

	case WM_DESTROY:
		arProfiles.destroy();
		break;
	}

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
				TCHAR profileName[128];
				GetDlgItemText(hwndDlg, IDC_PROFILENAME, profileName, SIZEOF(profileName));
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
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszGroup = LPGEN("Status");
	odp.pszTitle = LPGEN("StartupStatus");
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTab = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STARTUPSTATUS);
	odp.pfnDlgProc = StartupStatusOptDlgProc;
	Options_AddPage(wparam,&odp);

	odp.pszTab = LPGEN("Status profiles");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STATUSPROFILES);
	odp.pfnDlgProc = StatusProfilesOptDlgProc;
	Options_AddPage(wparam,&odp);
	return 0;
}

static int ClearDatabase(char* filter)
{
	DBCONTACTENUMSETTINGS dbces;
	char** settings;
	int i, settingCount = 0;

	settingIndex = 0;
	dbces.szModule = MODULENAME;
	dbces.lParam = (LPARAM)&settingCount;
	dbces.pfnEnumProc = CountSettings;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);

	settings = (char**)malloc(settingCount*sizeof(char*));
	dbces.lParam = (LPARAM)&settings;
	dbces.pfnEnumProc = DeleteSetting;
	CallService(MS_DB_CONTACT_ENUMSETTINGS,0,(LPARAM)&dbces);
	for (i=0; i < settingCount; i++) {
		if ((filter == NULL) || (!strncmp(filter, settings[i], strlen(filter))))
			db_unset(NULL, MODULENAME, settings[i]);
		free(settings[i]);
	}
	free(settings);
	// < v0.0.0.9
	if (filter == NULL)	db_unset(NULL, "AutoAway", "Confirm");

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

char* OptName(int i, const char* setting)
{
	static char buf[100];
	mir_snprintf(buf, SIZEOF(buf), "%d_%s", i, setting);
	return buf;
}
