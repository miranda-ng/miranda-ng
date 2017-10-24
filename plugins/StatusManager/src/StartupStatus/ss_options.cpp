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

#include "..\stdafx.h"

char* OptName(int i, const char* setting)
{
	static char buf[100];
	mir_snprintf(buf, "%d_%s", i, setting);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////

int SSCompareSettings(const TSSSetting *p1, const TSSSetting *p2)
{
	return mir_strcmp(p1->m_szName, p2->m_szName);
}

TSettingsList* GetCurrentProtoSettings()
{
	int count;
	PROTOACCOUNT **protos;
	Proto_EnumAccounts(&count, &protos);

	TSettingsList *result = new TSettingsList(count, SSCompareSettings);
	if (result == nullptr)
		return nullptr;

	for (int i = 0; i < count; i++)
		if (IsSuitableProto(protos[i]))
			result->insert(new TSSSetting(protos[i]));

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
	if (protoSettings.getCount() == 0)
		return nullptr;

	char *cmdl, *pnt;
	pnt = cmdl = (char*)malloc(mir_strlen(protoSettings[0].m_szName) + mir_strlen(GetStatusDesc(protoSettings[0].m_status)) + 4);

	for (int i = 0; i < protoSettings.getCount(); i++) {
		*pnt++ = '/';
		mir_strcpy(pnt, protoSettings[i].m_szName);
		pnt += mir_strlen(protoSettings[i].m_szName);
		*pnt++ = '=';
		mir_strcpy(pnt, GetStatusDesc(protoSettings[i].m_status));
		pnt += mir_strlen(GetStatusDesc(protoSettings[i].m_status));
		if (i != protoSettings.getCount() - 1) {
			*pnt++ = ' ';
			*pnt++ = '\0';
			cmdl = (char*)realloc(cmdl, mir_strlen(cmdl) + mir_strlen(protoSettings[i + 1].m_szName) + mir_strlen(GetStatusDesc(protoSettings[i + 1].m_status)) + 4);
			pnt = cmdl + mir_strlen(cmdl);
		}
	}

	if (db_get_b(0, SSMODULENAME, SETTING_SHOWDIALOG, FALSE) == TRUE) {
		*pnt++ = ' ';
		*pnt++ = '\0';
		cmdl = (char*)realloc(cmdl, mir_strlen(cmdl) + 12);
		pnt = cmdl + mir_strlen(cmdl);
		mir_strcpy(pnt, "/showdialog");
		pnt += 11;
		*pnt = '\0';
	}

	return cmdl;
}

static char* GetCMDL(TSettingsList& protoSettings)
{
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);

	char* cmdl = (char*)malloc(mir_strlen(path) + 4);
	mir_snprintf(cmdl, mir_strlen(path) + 4, "\"%s\" ", path);

	char* args = GetCMDLArguments(protoSettings);
	if (args) {
		cmdl = (char*)realloc(cmdl, mir_strlen(cmdl) + mir_strlen(args) + 1);
		mir_strcat(cmdl, args);
		free(args);
	}
	return cmdl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Link processing

static wchar_t* GetLinkDescription(TSettingsList& protoSettings)
{
	if (protoSettings.getCount() == 0)
		return nullptr;

	CMStringW result(SHORTCUT_DESC);
	for (int i = 0; i < protoSettings.getCount(); i++) {
		TSSSetting &p = protoSettings[i];

		wchar_t *status;
		if (p.m_status == ID_STATUS_LAST)
			status = TranslateT("<last>");
		else if (p.m_status == ID_STATUS_CURRENT)
			status = TranslateT("<current>");
		else if (p.m_status >= MIN_STATUS && p.m_status <= MAX_STATUS)
			status = pcli->pfnGetStatusModeDescription(p.m_status, 0);
		else
			status = nullptr;
		if (status == nullptr)
			status = TranslateT("<unknown>");

		result.AppendChar('\r');
		result.Append(p.m_tszAccName);
		result.AppendChar(':');
		result.AppendChar(' ');
		result.Append(status);
	}

	return mir_wstrndup(result, result.GetLength());
}

HRESULT CreateLink(TSettingsList& protoSettings)
{
	wchar_t savePath[MAX_PATH];
	if (SHGetSpecialFolderPath(nullptr, savePath, 0x10, FALSE))
		wcsncat_s(savePath, SHORTCUT_FILENAME, _countof(savePath) - mir_wstrlen(savePath));
	else
		mir_snwprintf(savePath, L".\\%s", SHORTCUT_FILENAME);

	// Get a pointer to the IShellLink interface.
	IShellLink *psl;
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
	if (SUCCEEDED(hres)) {
		char *args = GetCMDLArguments(protoSettings);
		wchar_t *desc = GetLinkDescription(protoSettings);

		// Set the path to the shortcut target, and add the
		// description.
		wchar_t path[MAX_PATH];
		GetModuleFileName(nullptr, path, _countof(path));
		psl->SetPath(path);
		psl->SetDescription(desc);
		psl->SetArguments(_A2T(args));

		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		IPersistFile *ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

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

INT_PTR CALLBACK CmdlOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TSettingsList* optionsProtoSettings;

	switch (msg) {
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
			if (OpenClipboard(hwndDlg)) {
				EmptyClipboard();

				char cmdl[2048];
				GetDlgItemTextA(hwndDlg, IDC_CMDL, cmdl, _countof(cmdl));
				HGLOBAL cmdlGlob = GlobalAlloc(GMEM_MOVEABLE, sizeof(cmdl));
				if (cmdlGlob == nullptr) {
					CloseClipboard();
					break;
				}
				LPTSTR cmdlStr = (LPTSTR)GlobalLock(cmdlGlob);
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

static INT_PTR CALLBACK StartupStatusOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bInitDone = FALSE;

	switch (msg) {
	case WM_INITDIALOG:
		bInitDone = FALSE;

		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SETPROFILE, db_get_b(0, SSMODULENAME, SETTING_SETPROFILE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OVERRIDE, db_get_b(0, SSMODULENAME, SETTING_OVERRIDE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWDIALOG, db_get_b(0, SSMODULENAME, SETTING_SHOWDIALOG, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETWINSTATE, db_get_b(0, SSMODULENAME, SETTING_SETWINSTATE, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETWINLOCATION, db_get_b(0, SSMODULENAME, SETTING_SETWINLOCATION, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETDOCKED, db_get_b(0, SSMODULENAME, SETTING_SETDOCKED, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SETWINSIZE, db_get_b(0, SSMODULENAME, SETTING_SETWINSIZE, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OFFLINECLOSE, db_get_b(0, SSMODULENAME, SETTING_OFFLINECLOSE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTODIAL, db_get_b(0, SSMODULENAME, SETTING_AUTODIAL, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOHANGUP, db_get_b(0, SSMODULENAME, SETTING_AUTOHANGUP, 0) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, db_get_dw(0, SSMODULENAME, SETTING_SETPROFILEDELAY, 500), FALSE);
		SetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, db_get_dw(0, SSMODULENAME, SETTING_DLGTIMEOUT, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_XPOS, db_get_dw(0, SSMODULENAME, SETTING_XPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_YPOS, db_get_dw(0, SSMODULENAME, SETTING_YPOS, 0), TRUE);
		SetDlgItemInt(hwndDlg, IDC_WIDTH, db_get_dw(0, SSMODULENAME, SETTING_WIDTH, 0), FALSE);
		SetDlgItemInt(hwndDlg, IDC_HEIGHT, db_get_dw(0, SSMODULENAME, SETTING_HEIGHT, 0), FALSE);
		{
			int val = db_get_b(0, SSMODULENAME, SETTING_DOCKED, DOCKED_NONE);
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

		EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
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
		SetTimer(hwndDlg, 0, 100, nullptr);
		bInitDone = TRUE;
		break;

	case WM_TIMER:
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
			SetDlgItemTextA(hwndDlg, IDC_CURWINSIZE, "");
			SetDlgItemTextA(hwndDlg, IDC_CURWINLOC, "");
			break;
		}
		else {
			wchar_t text[128];
			mir_snwprintf(text, TranslateT("size: %d x %d"),
				db_get_dw(0, MODULE_CLIST, SETTING_WIDTH, 0),
				db_get_dw(0, MODULE_CLIST, SETTING_HEIGHT, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINSIZE, text);

			mir_snwprintf(text, TranslateT("loc: %d x %d"),
				db_get_dw(0, MODULE_CLIST, SETTING_XPOS, 0),
				db_get_dw(0, MODULE_CLIST, SETTING_YPOS, 0));
			SetDlgItemText(hwndDlg, IDC_CURWINLOC, text);
		}
		break;

	case UM_REINITPROFILES:
		// creates profile combo box according to 'dat'
		SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_RESETCONTENT, 0, 0);
		{
			int defProfile;
			int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
			for (int i = 0; i < profileCount; i++) {
				wchar_t profileName[128];
				if (GetProfileName(i, (LPARAM)profileName))
					continue;

				int item = SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)profileName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETITEMDATA, item, i);
			}
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_SETCURSEL, defProfile, 0);
		}
		break;

	case UM_REINITDOCKED:
		EnableWindow(GetDlgItem(hwndDlg, IDC_SETDOCKED), db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 1));
		if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_SETDOCKED)))
			CheckDlgButton(hwndDlg, IDC_SETDOCKED, BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_DOCKED), IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
		break;

	case UM_REINITWINSTATE:
		{
			int val = db_get_b(0, SSMODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_RESETCONTENT, 0, 0);

			int item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hidden"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, (LPARAM)SETTING_STATE_HIDDEN);
			if (val == SETTING_STATE_HIDDEN)
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);

			if (!db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0)) {
				item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Minimized"));
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_MINIMIZED);
				if (val == SETTING_STATE_MINIMIZED)
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);
			}
			item = SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Normal"));
			SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_NORMAL);
			if (val == SETTING_STATE_NORMAL || (val == SETTING_STATE_MINIMIZED) && db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_SETCURSEL, item, 0);
		}
		break;

	case UM_REINITWINSIZE:
		EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTH), IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HEIGHT), !db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0) && IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == LBN_SELCHANGE || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == EN_CHANGE)
			if (bInitDone)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_SETPROFILE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILEDELAY), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			break;
		case IDC_SHOWDIALOG:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
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
				if (ps) {
					GetProfile(defProfile, *ps);
					for (int i = 0; i < ps->getCount(); i++)
						if ((*ps)[i].m_szMsg != nullptr)
							(*ps)[i].m_szMsg = wcsdup((*ps)[i].m_szMsg);

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

			db_set_b(0, SSMODULENAME, SETTING_SETPROFILE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE))
				db_set_dw(0, SSMODULENAME, SETTING_SETPROFILEDELAY, GetDlgItemInt(hwndDlg, IDC_SETPROFILEDELAY, nullptr, FALSE));

			if (IsDlgButtonChecked(hwndDlg, IDC_SETPROFILE) || IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
				db_set_w(0, SSMODULENAME, SETTING_DEFAULTPROFILE, (WORD)val);
			}
			db_set_b(0, SSMODULENAME, SETTING_OVERRIDE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE));
			db_set_b(0, SSMODULENAME, SETTING_SHOWDIALOG, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG));
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWDIALOG))
				db_set_dw(0, SSMODULENAME, SETTING_DLGTIMEOUT, GetDlgItemInt(hwndDlg, IDC_DLGTIMEOUT, nullptr, FALSE));

			db_set_b(0, SSMODULENAME, SETTING_SETWINSTATE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSTATE)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_WINSTATE, CB_GETCURSEL, 0, 0), 0);
				db_set_b(0, SSMODULENAME, SETTING_WINSTATE, (BYTE)val);
			}
			db_set_b(0, SSMODULENAME, SETTING_SETDOCKED, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETDOCKED)) {
				val = (int)SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_DOCKED, CB_GETCURSEL, 0, 0), 0);
				db_set_b(0, SSMODULENAME, SETTING_DOCKED, (BYTE)val);
			}
			db_set_b(0, SSMODULENAME, SETTING_SETWINLOCATION, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINLOCATION)) {
				db_set_dw(0, SSMODULENAME, SETTING_XPOS, GetDlgItemInt(hwndDlg, IDC_XPOS, nullptr, TRUE));
				db_set_dw(0, SSMODULENAME, SETTING_YPOS, GetDlgItemInt(hwndDlg, IDC_YPOS, nullptr, TRUE));
			}
			db_set_b(0, SSMODULENAME, SETTING_SETWINSIZE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE));
			if (IsDlgButtonChecked(hwndDlg, IDC_SETWINSIZE)) {
				db_set_dw(0, SSMODULENAME, SETTING_WIDTH, GetDlgItemInt(hwndDlg, IDC_WIDTH, nullptr, FALSE));
				db_set_dw(0, SSMODULENAME, SETTING_HEIGHT, GetDlgItemInt(hwndDlg, IDC_HEIGHT, nullptr, FALSE));
			}
			db_set_b(0, SSMODULENAME, SETTING_OFFLINECLOSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OFFLINECLOSE));
			db_set_b(0, SSMODULENAME, SETTING_AUTODIAL, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTODIAL));
			db_set_b(0, SSMODULENAME, SETTING_AUTOHANGUP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOHANGUP));
		}
		break;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// for db cleanup

static int DeleteSetting(const char *szSetting, LPARAM lParam)
{
	LIST<char> *p = (LIST<char> *)lParam;
	p->insert(mir_strdup(szSetting));
	return 0;
}

static int ClearDatabase(char* filter)
{
	LIST<char> arSettings(10);
	db_enum_settings(0, DeleteSetting, SSMODULENAME, &arSettings);

	for (int i = 0; i < arSettings.getCount(); i++) {
		if ((filter == nullptr) || (!strncmp(filter, arSettings[i], mir_strlen(filter))))
			db_unset(0, SSMODULENAME, arSettings[i]);
		mir_free(arSettings[i]);
	}

	if (filter == nullptr)
		db_unset(0, "AutoAway", "Confirm");

	return 0;
}


INT_PTR CALLBACK addProfileDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndParent;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndParent = (HWND)lParam;
		EnableWindow(GetDlgItem(hwndDlg, IDC_OK), FALSE);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_OK) {
			wchar_t profileName[128];
			GetDlgItemText(hwndDlg, IDC_PROFILENAME, profileName, _countof(profileName));
			SendMessage(hwndParent, UM_ADDPROFILE, 0, (LPARAM)profileName);
			// done and exit
			DestroyWindow(hwndDlg);
		}
		else if (LOWORD(wParam) == IDC_CANCEL) {
			DestroyWindow(hwndDlg);
		}
		else if (LOWORD(wParam) == IDC_PROFILENAME) {
			(SendDlgItemMessage(hwndDlg, IDC_PROFILENAME, EM_LINELENGTH, 0, 0) > 0) ? EnableWindow(GetDlgItem(hwndDlg, IDC_OK), TRUE) : EnableWindow(GetDlgItem(hwndDlg, IDC_OK), FALSE);
		}
		break;

	case WM_DESTROY:
		EnableWindow(hwndParent, TRUE);
		break;
	}

	return 0;
}

class CSSAdvancedOptDlg : public CDlgBase
{
	bool bNeedRebuildMenu;

	OBJLIST<PROFILEOPTIONS> arProfiles;

	void ReinitProfiles()
	{
		// creates profile combo box according to 'dat'
		cmbProfile.ResetContent();
		for (int i = 0; i < arProfiles.getCount(); i++) 
			cmbProfile.AddString(arProfiles[i].tszName, i);

		cmbProfile.SetCurSel(0);
		SetProfile();
	}

	void SetProfile()
	{
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		chkCreateTTB.SetState(arProfiles[sel].createTtb);
		chkShowDialog.SetState(arProfiles[sel].showDialog);
		chkCreateMMI.SetState(arProfiles[sel].createMmi);
		chkInSubmenu.SetState(arProfiles[sel].inSubMenu);
		chkInSubmenu.Enable(arProfiles[sel].createMmi);
		chkRegHotkey.SetState(arProfiles[sel].regHotkey);
		edtHotkey.SendMsg(HKM_SETHOTKEY, arProfiles[sel].hotKey, 0);
		edtHotkey.Enable(arProfiles[sel].regHotkey);

		// fill proto list
		lstAccount.ResetContent();
		TSettingsList& ar = *arProfiles[sel].ps;
		for (int i = 0; i < ar.getCount(); i++)
			lstAccount.AddString(ar[i].m_tszAccName, (LPARAM)&ar[i]);
		lstAccount.SetCurSel(0);

		SetProtocol();
	}

	void SetProtocol()
	{
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			// fill status box
			TSSSetting* ps = (TSSSetting*)lstAccount.GetItemData(idx);

			int flags = (CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_2, 0))&~(CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_5, 0));
			lstStatus.ResetContent();
			for (int i = 0; i < _countof(statusModeList); i++) {
				if ((flags & statusModePf2List[i]) || (statusModeList[i] == ID_STATUS_OFFLINE)) {
					int item = lstStatus.AddString(pcli->pfnGetStatusModeDescription(statusModeList[i], 0), statusModeList[i]);
					if (ps->m_status == statusModeList[i])
						lstStatus.SetCurSel(item);
				}
			}

			int item = lstStatus.AddString(TranslateT("<current>"), ID_STATUS_CURRENT);
			if (ps->m_status == ID_STATUS_CURRENT)
				lstStatus.SetCurSel(item);

			item = lstStatus.AddString(TranslateT("<last>"), ID_STATUS_LAST);
			if (ps->m_status == ID_STATUS_LAST)
				lstStatus.SetCurSel(item);
		}

		SetStatusMsg();
	}

	// set status message
	void SetStatusMsg()
	{
		bool bStatusMsg = false;
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			TSSSetting *ps = (TSSSetting*)lstAccount.GetItemData(idx);

			CheckRadioButton(m_hwnd, IDC_MIRANDAMSG, IDC_CUSTOMMSG, ps->m_szMsg != nullptr ? IDC_CUSTOMMSG : IDC_MIRANDAMSG);
			if (ps->m_szMsg != nullptr)
				edtStatusMsg.SetText(ps->m_szMsg);

			bStatusMsg = ((((CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG)) &&
				(CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(ps->m_status))) || (ps->m_status == ID_STATUS_CURRENT) || (ps->m_status == ID_STATUS_LAST));
		}
		chkMiranda.Enable(bStatusMsg);
		chkCustom.Enable(bStatusMsg);
		btnHelp.Enable(bStatusMsg && chkCustom.GetState());
		edtStatusMsg.Enable(bStatusMsg && chkCustom.GetState());
	}

	CCtrlEdit edtStatusMsg, edtHotkey;
	CCtrlCombo cmbProfile;
	CCtrlCheck chkMiranda, chkCustom, chkCreateMMI, chkInSubmenu, chkRegHotkey, chkCreateTTB, chkShowDialog;
	CCtrlButton btnAdd, btnDelete, btnHelp;
	CCtrlListBox lstStatus, lstAccount;

public:
	CSSAdvancedOptDlg() :
		CDlgBase(hInst, IDD_OPT_STATUSPROFILES),
		cmbProfile(this, IDC_PROFILE),
		btnAdd(this, IDC_ADDPROFILE),
		btnHelp(this, IDC_VARIABLESHELP),
		btnDelete(this, IDC_DELPROFILE),
		chkCustom(this, IDC_CUSTOMMSG),
		chkMiranda(this, IDC_MIRANDAMSG),
		chkCreateMMI(this, IDC_CREATEMMI),
		chkInSubmenu(this, IDC_INSUBMENU),
		chkRegHotkey(this, IDC_REGHOTKEY),
		chkCreateTTB(this, IDC_CREATETTB),
		chkShowDialog(this, IDC_SHOWDIALOG),
		edtHotkey(this, IDC_HOTKEY),
		edtStatusMsg(this, IDC_STATUSMSG),
		lstStatus(this, IDC_STATUS),
		lstAccount(this, IDC_PROTOCOL),
		arProfiles(5),
		bNeedRebuildMenu(false)
	{
		btnAdd.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Add);
		btnHelp.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Help);
		btnDelete.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Delete);

		cmbProfile.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Profile);

		chkCreateMMI.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_CreateMMI);
		chkInSubmenu.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_InSubmenu);
		chkCustom.OnChange = chkMiranda.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_StatusMsg);
		chkRegHotkey.OnChange = chkCreateTTB.OnChange = chkShowDialog.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Option);
		
		edtHotkey.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Hotkey);
		edtStatusMsg.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_EdtStatusMsg);

		lstStatus.OnSelChange = Callback(this, &CSSAdvancedOptDlg::onChange_Status);
		lstAccount.OnSelChange = Callback(this, &CSSAdvancedOptDlg::onChange_Account);
	}

	virtual void OnInitDialog() override
	{
		chkCreateMMI.SetText(TranslateT("Create a status menu item"));

		int defProfile;
		int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
		if (profileCount == 0) {
			profileCount = 1;
			defProfile = 0;
			NotifyChange();
		}

		for (int i = 0; i < profileCount; i++) {
			PROFILEOPTIONS *ppo = new PROFILEOPTIONS;
			ppo->ps = GetCurrentProtoSettings();
			TSettingsList& ar = *ppo->ps;

			if (GetProfile(i, ar) == -1) {
				/* create an empty profile */
				if (i == defProfile)
					ppo->tszName = mir_wstrdup(TranslateT("default"));
				else
					ppo->tszName = mir_wstrdup(TranslateT("unknown"));
			}
			else {
				for (int j = 0; j < ar.getCount(); j++)
					if (ar[j].m_szMsg != nullptr)
						ar[j].m_szMsg = wcsdup(ar[j].m_szMsg);

				ppo->tszName = db_get_wsa(0, SSMODULENAME, OptName(i, SETTING_PROFILENAME));
				if (ppo->tszName == nullptr) {
					if (i == defProfile)
						ppo->tszName = mir_wstrdup(TranslateT("default"));
					else
						ppo->tszName = mir_wstrdup(TranslateT("unknown"));
				}
				ppo->createTtb = db_get_b(0, SSMODULENAME, OptName(i, SETTING_CREATETTBBUTTON), 0);
				ppo->showDialog = db_get_b(0, SSMODULENAME, OptName(i, SETTING_SHOWCONFIRMDIALOG), 0);
				ppo->createMmi = db_get_b(0, SSMODULENAME, OptName(i, SETTING_CREATEMMITEM), 0);
				ppo->inSubMenu = db_get_b(0, SSMODULENAME, OptName(i, SETTING_INSUBMENU), 1);
				ppo->regHotkey = db_get_b(0, SSMODULENAME, OptName(i, SETTING_REGHOTKEY), 0);
				ppo->hotKey = db_get_w(0, SSMODULENAME, OptName(i, SETTING_HOTKEY), MAKEWORD((char)('0' + i), HOTKEYF_CONTROL | HOTKEYF_SHIFT));
			}
			arProfiles.insert(ppo);
		}
		if (hTTBModuleLoadedHook == nullptr)
			chkCreateTTB.Disable();

		ReinitProfiles();
		ShowWindow(GetDlgItem(m_hwnd, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELPEX) ? SW_SHOW : SW_HIDE);
	}

	virtual void OnApply() override
	{
		char setting[128];
		int oldCount = db_get_w(0, SSMODULENAME, SETTING_PROFILECOUNT, 0);
		for (int i = 0; i < oldCount; i++) {
			mir_snprintf(setting, "%d_", i);
			ClearDatabase(setting);
		}
		for (int i = 0; i < arProfiles.getCount(); i++) {
			PROFILEOPTIONS& po = arProfiles[i];
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_SHOWCONFIRMDIALOG), po.showDialog);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_CREATETTBBUTTON), po.createTtb);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_CREATEMMITEM), po.createMmi);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_INSUBMENU), po.inSubMenu);
			db_set_b(0, SSMODULENAME, OptName(i, SETTING_REGHOTKEY), po.regHotkey);
			db_set_w(0, SSMODULENAME, OptName(i, SETTING_HOTKEY), po.hotKey);
			db_set_ws(0, SSMODULENAME, OptName(i, SETTING_PROFILENAME), po.tszName);

			TSettingsList& ar = *po.ps;
			for (int j = 0; j < ar.getCount(); j++) {
				if (ar[j].m_szMsg != nullptr) {
					mir_snprintf(setting, "%s_%s", ar[j].m_szName, SETTING_PROFILE_STSMSG);
					db_set_ws(0, SSMODULENAME, OptName(i, setting), ar[j].m_szMsg);
				}
				db_set_w(0, SSMODULENAME, OptName(i, ar[j].m_szName), ar[j].m_status);
			}
		}
		db_set_w(0, SSMODULENAME, SETTING_PROFILECOUNT, (WORD)arProfiles.getCount());

		// Rebuild status menu
		if (bNeedRebuildMenu)
			pcli->pfnReloadProtoMenus();

		SSLoadMainOptions();
	}

	// add a profile
	void onClick_Add(CCtrlButton*)
	{			
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDPROFILE), m_hwnd, addProfileDlgProc, (LPARAM)m_hwnd);
		EnableWindow(m_hwnd, FALSE);
	}

	void onClick_Delete(CCtrlButton*)
	{
		// wparam == profile no
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		if (arProfiles.getCount() == 1) {
			MessageBox(nullptr, TranslateT("At least one profile must exist"), TranslateT("Status manager"), MB_OK);
			return;
		}

		arProfiles.remove(sel);

		int defProfile;
		GetProfileCount((WPARAM)&defProfile, 0);
		if (sel == defProfile) {
			MessageBox(nullptr, TranslateT("Your default profile will be changed"), TranslateT("Status manager"), MB_OK);
			db_set_w(0, SSMODULENAME, SETTING_DEFAULTPROFILE, 0);
		}
		ReinitProfiles();;
	}

	void onClick_Help(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, IDC_STATUSMSG, VHF_INPUT | VHF_EXTRATEXT | VHF_HELP | VHF_FULLFILLSTRUCT | VHF_HIDESUBJECTTOKEN, nullptr, "Protocol ID");
	}

	void onChange_Status(CCtrlListBox*)
	{
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			TSSSetting* ps = (TSSSetting*)lstAccount.GetItemData(idx);
			ps->m_status = lstStatus.GetItemData(lstStatus.GetCurSel());
		}
		SetStatusMsg();
	}

	void onChange_Account(CCtrlListBox*)
	{
		SetProtocol();
	}

	void onChange_Profile(CCtrlCombo*)
	{
		SetProfile();
	}

	void onChange_StatusMsg(CCtrlCheck*)
	{
		int len;
		TSSSetting* ps = (TSSSetting*)lstAccount.GetItemData(lstAccount.GetCurSel());
		if (ps->m_szMsg != nullptr)
			free(ps->m_szMsg);

		ps->m_szMsg = nullptr;
		if (chkCustom.GetState()) {
			len = edtStatusMsg.SendMsg(WM_GETTEXTLENGTH, 0, 0);
			ps->m_szMsg = (wchar_t*)calloc(sizeof(wchar_t), len + 1);
			GetDlgItemText(m_hwnd, IDC_STATUSMSG, ps->m_szMsg, (len + 1));
		}
		SetStatusMsg();
	}

	void onChange_Hotkey(CCtrlEdit*)
	{
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		arProfiles[sel].hotKey = edtHotkey.SendMsg(HKM_GETHOTKEY, 0, 0);
	}

	void onChange_EdtStatusMsg(CCtrlEdit*)
	{
		// update the status message in memory, this is done on each character tick, not nice
		// but it works
		TSSSetting* ps = (TSSSetting*)lstAccount.GetItemData(lstAccount.GetCurSel());
		if (ps->m_szMsg != nullptr) {
			if (*ps->m_szMsg)
				free(ps->m_szMsg);
			ps->m_szMsg = nullptr;
		}
		int len = edtStatusMsg.SendMsg(WM_GETTEXTLENGTH, 0, 0);
		ps->m_szMsg = (wchar_t*)calloc(sizeof(wchar_t), len + 1);
		GetDlgItemText(m_hwnd, IDC_STATUSMSG, ps->m_szMsg, (len + 1));
	}

	void onChange_CreateMMI(CCtrlCheck*)
	{
		chkInSubmenu.Enable(chkCreateMMI.GetState());
		onChange_InSubmenu(nullptr);
	}

	void onChange_InSubmenu(CCtrlCheck*)
	{
		bNeedRebuildMenu = TRUE;
		onChange_Option(nullptr);
	}

	void onChange_Option(CCtrlCheck*)
	{
		int sel = cmbProfile.GetItemData(cmbProfile.GetCurSel());
		PROFILEOPTIONS& po = arProfiles[sel];
		po.createMmi = chkCreateMMI.GetState();
		po.inSubMenu = chkInSubmenu.GetState();
		po.createTtb = chkCreateTTB.GetState();
		po.regHotkey = chkRegHotkey.GetState();
		po.showDialog = chkShowDialog.GetState();
		edtHotkey.Enable(chkRegHotkey.GetState());
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case UM_ADDPROFILE:
			wchar_t *tszName = (wchar_t*)lParam;
			if (tszName == nullptr)
				break;

			PROFILEOPTIONS* ppo = new PROFILEOPTIONS;
			ppo->tszName = mir_wstrdup(tszName);
			ppo->ps = GetCurrentProtoSettings();
			arProfiles.insert(ppo);

			ReinitProfiles();;
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

int StartupStatusOptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Startup status");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STARTUPSTATUS);
	odp.pfnDlgProc = StartupStatusOptDlgProc;
	Options_AddPage(wparam, &odp);

	odp.szTab.a = LPGEN("Status profiles");
	odp.pszTemplate = nullptr;
	odp.pfnDlgProc = nullptr;
	odp.pDialog = new CSSAdvancedOptDlg();
	Options_AddPage(wparam, &odp);
	return 0;
}
