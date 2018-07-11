/*
Copyright (C) 2012-18 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void ShowThePreview()
{
	if (!ServiceExists(MS_POPUP_ADDPOPUPT)) {
		MessageBox(nullptr, TranslateT("Popup plugin not found!"), TranslateT("Error"), MB_OK | MB_ICONSTOP);
		return;
	}

	if (WumfOptions.AlertFolders) {
		ShowThePopup(nullptr, L"Guest", L"C:\\My Share");
		Sleep(300);
		ShowThePopup(nullptr, L"Guest", L"C:\\My Share\\Photos");
		Sleep(300);
	}
	ShowThePopup(nullptr, L"Guest", L"C:\\Share\\My Photos\\photo.jpg");
	Sleep(300);
	if (WumfOptions.AlertFolders) {
		ShowThePopup(nullptr, L"User", L"C:\\My Share");
		Sleep(300);
		ShowThePopup(nullptr, L"User", L"C:\\My Share\\Movies");
		Sleep(300);
	}
	ShowThePopup(nullptr, L"User", L"C:\\My Share\\Movies\\The Two Towers.avi");
	Sleep(300);
	if (WumfOptions.AlertFolders) {
		ShowThePopup(nullptr, L"Administrator", L"C:\\Distributives");
		Sleep(300);
		ShowThePopup(nullptr, L"Administrator", L"C:\\Distributives\\Win2k");
		Sleep(300);
	}
	ShowThePopup(nullptr, L"Administrator", L"C:\\Distributives\\Win2k\\setup.exe");
}

void DisableDelayOptions(HWND hwndDlg)
{
	CheckDlgButton(hwndDlg, IDC_DELAY_INF,BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_DELAY_SET,BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_DELAY_DEF,BST_CHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_INF), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SET), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_DEF), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TX_DELAY_SEC), FALSE);
}

void ChooseFile(HWND hwndDlg)
{
	wchar_t szFile[MAX_PATH]; szFile[0]=0;

	// Initialize OPENFILENAME
	OPENFILENAME ofn = {0};       // common dialog box structure
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndDlg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = _countof(szFile);
	ofn.lpstrFilter = L"All files (*.*)\0*.*\0Text files (*.txt)\0*.txt\0Log files (*.log)\0*.log\0\0";
	ofn.nFilterIndex = 2;
	ofn.Flags = OFN_CREATEPROMPT;
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn)) {
		HANDLE hf = CreateFile(szFile,GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hf != INVALID_HANDLE_VALUE) {
			SetDlgItemText(hwndDlg,IDC_FILE,szFile);
			mir_wstrncpy(WumfOptions.LogFile, szFile, MAX_PATH);
			CloseHandle(hf);
		}
	}
	else if (CommDlgExtendedError() != 0) {
		wchar_t str[256];
		mir_snwprintf(str, TranslateT("Common Dialog Error 0x%lx"), CommDlgExtendedError());
		MessageBox(hwndDlg, str, TranslateT("Error"), MB_OK | MB_ICONSTOP);
	}
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WORD wControlId = LOWORD(wparam);
	WORD wNotifyCode = HIWORD(wparam);
	int seconds;

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_COLOR_WIN, WumfOptions.UseWinColor ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_COLOR_DEF, WumfOptions.UseDefColor ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_COLOR_SET, WumfOptions.SelectColor ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BACK), WumfOptions.SelectColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TEXT), WumfOptions.SelectColor);
		if (WumfOptions.SelectColor) {
			SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_SETCOLOUR,0,WumfOptions.ColorBack);
			SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_SETCOLOUR,0,WumfOptions.ColorText);
		}
		if ( !ServiceExists(MS_POPUP_ADDPOPUPT)) {
			DisableDelayOptions(hwndDlg);
			break;
		}
		CheckDlgButton(hwndDlg, IDC_DELAY_INF, WumfOptions.DelayInf ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DELAY_DEF, WumfOptions.DelayDef ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DELAY_SET, WumfOptions.DelaySet ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), WumfOptions.DelaySet);
		SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);
		//Logging & alerts
		CheckDlgButton(hwndDlg, IDC_LOG_FOLDER, WumfOptions.LogFolders ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALERT_FOLDER, WumfOptions.AlertFolders ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_UNC, WumfOptions.LogUNC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALERT_UNC, WumfOptions.AlertUNC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_COMP, WumfOptions.LogComp ? BST_CHECKED : BST_UNCHECKED);

		if (WumfOptions.LogToFile) {
			CheckDlgButton(hwndDlg,IDC_LOG_INTO_FILE,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), TRUE);
			SetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile);
		}
		else {
			CheckDlgButton(hwndDlg,IDC_LOG_INTO_FILE,BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), FALSE);
			SetDlgItemText(hwndDlg, IDC_FILE, L"");
		}
		break;

	case WM_COMMAND:
		switch(wNotifyCode) {
		case BN_CLICKED :
			switch(wControlId) {
			case IDC_DELAY_SET:
			case IDC_DELAY_DEF:
			case IDC_DELAY_INF:
				WumfOptions.DelaySet = (IsDlgButtonChecked(hwndDlg, IDC_DELAY_SET) == BST_CHECKED);	
				WumfOptions.DelayDef = (IsDlgButtonChecked(hwndDlg, IDC_DELAY_DEF) == BST_CHECKED);	
				WumfOptions.DelayInf = (IsDlgButtonChecked(hwndDlg, IDC_DELAY_INF) == BST_CHECKED);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), WumfOptions.DelaySet);
				SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, TRUE);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_COLOR_SET:
			case IDC_COLOR_DEF:
			case IDC_COLOR_WIN:
				WumfOptions.SelectColor = (IsDlgButtonChecked(hwndDlg, IDC_COLOR_SET) == BST_CHECKED);	
				WumfOptions.UseDefColor = (IsDlgButtonChecked(hwndDlg, IDC_COLOR_DEF) == BST_CHECKED);	
				WumfOptions.UseWinColor = (IsDlgButtonChecked(hwndDlg, IDC_COLOR_WIN) == BST_CHECKED);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BACK),WumfOptions.SelectColor);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TEXT), WumfOptions.SelectColor);
				SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_SETCOLOUR,0,WumfOptions.ColorBack);
				SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_SETCOLOUR,0,WumfOptions.ColorText);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
				/* not implemented */
			case IDC_LOG_COMP:
			case IDC_ALERT_COMP:
			case IDC_LOG_UNC:
			case IDC_ALERT_UNC:
				MessageBox(nullptr, TranslateT("Not implemented yet..."), L"WUMF", MB_OK | MB_ICONINFORMATION);
				break;
				/* end */
			case IDC_LOG_INTO_FILE:
				WumfOptions.LogToFile = (IsDlgButtonChecked(hwndDlg, IDC_LOG_INTO_FILE) == BST_CHECKED);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), WumfOptions.LogToFile);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), WumfOptions.LogToFile);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_SEL_FILE:
				ChooseFile(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_LOG_FOLDER:
				WumfOptions.LogFolders = (IsDlgButtonChecked(hwndDlg, IDC_LOG_FOLDER) == BST_CHECKED);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_ALERT_FOLDER:
				WumfOptions.AlertFolders = (IsDlgButtonChecked(hwndDlg, IDC_ALERT_FOLDER) == BST_CHECKED);
				break;
			case IDC_PREVIEW:
				ShowThePreview();
				break;
			case IDC_CONN:
				CallService(MS_WUMF_CONNECTIONSSHOW, 0, 0);
				break;
			}
			break;

		case CPN_COLOURCHANGED:						
			WumfOptions.ColorText = SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_GETCOLOUR,0,0);
			WumfOptions.ColorBack = SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_GETCOLOUR,0,0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case EN_CHANGE:
			switch(wControlId) {
			case IDC_DELAY_SEC:
				seconds = GetDlgItemInt(hwndDlg, IDC_DELAY_SEC, nullptr, FALSE);
				if (seconds > LIFETIME_MAX)
					WumfOptions.DelaySec = LIFETIME_MAX;
				else if (seconds < LIFETIME_MIN)
					WumfOptions.DelaySec = LIFETIME_MIN;
				else if (seconds <= LIFETIME_MAX || seconds >= LIFETIME_MIN)
					WumfOptions.DelaySec = seconds;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_FILE:
				GetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile, _countof(WumfOptions.LogFile));
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		case EN_KILLFOCUS:
			switch(wControlId) {
			case IDC_DELAY_SEC:
				SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);								
				break;
			}
			break;
		}
		break;				

	case WM_NOTIFY: 
		switch(((LPNMHDR)lparam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lparam)->code) {
			case PSN_RESET:
				LoadOptions();
				return TRUE;

			case PSN_APPLY:
				db_set_dw(NULL, MODULENAME, COLOR_TEXT, (DWORD)WumfOptions.ColorText);
				db_set_dw(NULL, MODULENAME, COLOR_BACK, (DWORD)WumfOptions.ColorBack);
				db_set_b(NULL, MODULENAME, COLOR_DEF, (BYTE)WumfOptions.UseDefColor);
				db_set_b(NULL, MODULENAME, COLOR_WIN, (BYTE)WumfOptions.UseWinColor);
				db_set_b(NULL, MODULENAME, COLOR_SET, (BYTE)WumfOptions.SelectColor );
				db_set_b(NULL, MODULENAME, DELAY_DEF, (BYTE)WumfOptions.DelayDef);
				db_set_b(NULL, MODULENAME, DELAY_INF, (BYTE)WumfOptions.DelayInf);
				db_set_b(NULL, MODULENAME, DELAY_SET, (BYTE)WumfOptions.DelaySet);
				db_set_b(NULL, MODULENAME, DELAY_SEC, (BYTE)WumfOptions.DelaySec);
				db_set_b(NULL, MODULENAME, LOG_INTO_FILE, (BYTE)WumfOptions.LogToFile);
				db_set_b(NULL, MODULENAME, LOG_FOLDER, (BYTE)WumfOptions.LogFolders);
				db_set_b(NULL, MODULENAME, ALERT_FOLDER, (BYTE)WumfOptions.AlertFolders);
				db_set_b(NULL, MODULENAME, LOG_UNC, (BYTE)WumfOptions.LogUNC);
				db_set_b(NULL, MODULENAME, ALERT_UNC, (BYTE)WumfOptions.AlertUNC);
				db_set_b(NULL, MODULENAME, LOG_COMP, (BYTE)WumfOptions.LogComp);
				db_set_b(NULL, MODULENAME, ALERT_COMP, (BYTE)WumfOptions.AlertComp);
				GetDlgItemText(hwndDlg, IDC_FILE, WumfOptions.LogFile, _countof(WumfOptions.LogFile));
				db_set_ws(NULL, MODULENAME, OPT_FILE, WumfOptions.LogFile);
			}
		}
		break;
	}
	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 945000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.a = LPGEN("Who uses my files");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.szGroup.a = LPGEN("Services");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wparam, &odp);
	return 0;
}
