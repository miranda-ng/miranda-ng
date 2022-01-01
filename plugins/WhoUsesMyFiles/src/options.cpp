/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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
	uint16_t wControlId = LOWORD(wparam);
	uint16_t wNotifyCode = HIWORD(wparam);
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

		CheckDlgButton(hwndDlg, IDC_DELAY_INF, WumfOptions.DelayInf ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DELAY_DEF, WumfOptions.DelayDef ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DELAY_SET, WumfOptions.DelaySet ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), WumfOptions.DelaySet);
		SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);
		//Logging & alerts
		CheckDlgButton(hwndDlg, IDC_LOG_FOLDER, WumfOptions.LogFolders ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALERT_FOLDER, WumfOptions.AlertFolders ? BST_CHECKED : BST_UNCHECKED);

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
				g_plugin.setDword(COLOR_TEXT, (uint32_t)WumfOptions.ColorText);
				g_plugin.setDword(COLOR_BACK, (uint32_t)WumfOptions.ColorBack);
				g_plugin.setByte(COLOR_DEF, (uint8_t)WumfOptions.UseDefColor);
				g_plugin.setByte(COLOR_WIN, (uint8_t)WumfOptions.UseWinColor);
				g_plugin.setByte(COLOR_SET, (uint8_t)WumfOptions.SelectColor );
				g_plugin.setByte(DELAY_DEF, (uint8_t)WumfOptions.DelayDef);
				g_plugin.setByte(DELAY_INF, (uint8_t)WumfOptions.DelayInf);
				g_plugin.setByte(DELAY_SET, (uint8_t)WumfOptions.DelaySet);
				g_plugin.setByte(DELAY_SEC, (uint8_t)WumfOptions.DelaySec);
				g_plugin.setByte(LOG_INTO_FILE, (uint8_t)WumfOptions.LogToFile);
				g_plugin.setByte(LOG_FOLDER, (uint8_t)WumfOptions.LogFolders);
				g_plugin.setByte(ALERT_FOLDER, (uint8_t)WumfOptions.AlertFolders);
				GetDlgItemText(hwndDlg, IDC_FILE, WumfOptions.LogFile, _countof(WumfOptions.LogFile));
				g_plugin.setWString(OPT_FILE, WumfOptions.LogFile);
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
