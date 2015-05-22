/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HWND hViewWnd;
extern HINSTANCE hInst;

HDWP MyResizeWindow(HDWP hDwp, HWND hwndDlg, HWND hwndCtrl, int nHorizontalOffset, int nVerticalOffset, int nWidthOffset, int nHeightOffset)
{
	if (NULL == hwndDlg) /* Wine fix. */
		return hDwp;
	// get current bounding rectangle
	RECT rcinit;
	GetWindowRect(hwndCtrl, &rcinit);

	// get current top left point
	POINT pt;
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	return DeferWindowPos(hDwp, hwndCtrl, NULL,
		pt.x + nHorizontalOffset,
		pt.y + nVerticalOffset,
		rcinit.right - rcinit.left + nWidthOffset,
		rcinit.bottom - rcinit.top + nHeightOffset,
		SWP_NOZORDER);
}

BOOL MyResizeGetOffset(HWND hwndCtrl, int nWidth, int nHeight, int* nDx, int* nDy)
{
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndCtrl, &rcinit);

	// calculate offsets
	*nDx = nWidth - (rcinit.right - rcinit.left);
	*nDy = nHeight - (rcinit.bottom - rcinit.top);

	return rcinit.bottom != rcinit.top && nHeight > 0;
}

INT_PTR CALLBACK DlgProcView(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(IDI_VI, true));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx(IDI_VI));

			CHARFORMAT2 chf;
			chf.cbSize = sizeof(chf);
			SendDlgItemMessage(hwndDlg, IDC_VIEWVERSIONINFO, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM)&chf);
			mir_tstrcpy(chf.szFaceName, TEXT("Courier New"));
			SendDlgItemMessage(hwndDlg, IDC_VIEWVERSIONINFO, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&chf);

			CMString buffer;
			PrintVersionInfo(buffer, (unsigned int)lParam);
			SetDlgItemText(hwndDlg, IDC_VIEWVERSIONINFO, buffer.c_str());
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		}

		if (lParam & VI_FLAG_PRNDLL)
			SetWindowText(hwndDlg, TranslateT("View Version Information (with DLLs)"));

		Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, PluginName, "ViewInfo_");
		ShowWindow(hwndDlg, SW_SHOW);
		break;

	case WM_SIZE:
		GetWindowRect(GetDlgItem(hwndDlg, IDC_FILEVER), &rc);

		int dx, dy;
		if (MyResizeGetOffset(GetDlgItem(hwndDlg, IDC_VIEWVERSIONINFO), LOWORD(lParam) - 20, HIWORD(lParam) - 30 - (rc.bottom - rc.top), &dx, &dy)) {
			HDWP hDwp = BeginDeferWindowPos(4);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_FILEVER), 0, dy, 0, 0);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_CLIPVER), dx / 2, dy, 0, 0);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDCANCEL), dx, dy, 0, 0);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_VIEWVERSIONINFO), 0, 0, dx, dy);
			EndDeferWindowPos(hDwp);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
			mmi->ptMinTrackSize.x = 400; // The minimum width in points
			mmi->ptMinTrackSize.y = 300; // The minimum height in points
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLIPVER:
			CallService(MS_CRASHDUMPER_STORETOCLIP, 0, GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			break;

		case IDC_FILEVER:
			CallService(MS_CRASHDUMPER_STORETOFILE, 0, GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CONTEXTMENU:
		{
			HWND hView = GetDlgItem(hwndDlg, IDC_VIEWVERSIONINFO);
			GetWindowRect(hView, &rc);

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (PtInRect(&rc, pt)) {
				static const CHARRANGE all = { 0, -1 };

				HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
				HMENU hSubMenu = GetSubMenu(hMenu, 0);
				TranslateMenu(hSubMenu);

				CHARRANGE sel;
				SendMessage(hView, EM_EXGETSEL, 0, (LPARAM)&sel);
				if (sel.cpMin == sel.cpMax)
					EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);

				switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
				case IDM_COPY:
					SendMessage(hView, WM_COPY, 0, 0);
					break;

				case IDM_COPYALL:
					SendMessage(hView, EM_EXSETSEL, 0, (LPARAM)&all);
					SendMessage(hView, WM_COPY, 0, 0);
					SendMessage(hView, EM_EXSETSEL, 0, (LPARAM)&sel);
					break;

				case IDM_SELECTALL:
					SendMessage(hView, EM_EXSETSEL, 0, (LPARAM)&all);
					break;
				}
				DestroyMenu(hMenu);
			}
		}
		break;

	case WM_DESTROY:
		hViewWnd = NULL;
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		Utils_SaveWindowPosition(hwndDlg, NULL, PluginName, "ViewInfo_");
		if (servicemode)
			PostQuitMessage(0);
		break;
	}
	return FALSE;
}


void DestroyAllWindows(void)
{
	if (hViewWnd != NULL) {
		DestroyWindow(hViewWnd);
		hViewWnd = NULL;
	}
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DBVARIANT dbv;
			if (db_get_s(NULL, PluginName, "Username", &dbv) == 0) {
				SetDlgItemTextA(hwndDlg, IDC_USERNAME, dbv.pszVal);
				db_free(&dbv);
			}
			if (db_get_s(NULL, PluginName, "Password", &dbv) == 0) {
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			CheckDlgButton(hwndDlg, IDC_UPLOADCHN, db_get_b(NULL, PluginName, "UploadChanged", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLASSICDATES, clsdates ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DATESUBFOLDER, dtsubfldr ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CATCHCRASHES, catchcrashes ? BST_CHECKED : BST_UNCHECKED);
			if (!catchcrashes) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CLASSICDATES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DATESUBFOLDER), FALSE);
			}
			if (needrestart)
				ShowWindow(GetDlgItem(hwndDlg, IDC_RESTARTNOTE), SW_SHOW);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CATCHCRASHES:
			if (IsDlgButtonChecked(hwndDlg, IDC_CATCHCRASHES)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CLASSICDATES), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DATESUBFOLDER), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CLASSICDATES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DATESUBFOLDER), FALSE);
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_RESTARTNOTE), SW_SHOW);
			needrestart = 1;
		}
		if ((HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY) {
			char szSetting[100];
			GetDlgItemTextA(hwndDlg, IDC_USERNAME, szSetting, SIZEOF(szSetting));
			db_set_s(NULL, PluginName, "Username", szSetting);

			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, szSetting, SIZEOF(szSetting));
			db_set_s(NULL, PluginName, "Password", szSetting);

			db_set_b(NULL, PluginName, "UploadChanged", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_UPLOADCHN));

			clsdates = IsDlgButtonChecked(hwndDlg, IDC_CLASSICDATES) == BST_CHECKED;
			if (clsdates)
				db_set_b(NULL, PluginName, "ClassicDates", 1);
			else
				db_set_b(NULL, PluginName, "ClassicDates", 0);
			dtsubfldr = IsDlgButtonChecked(hwndDlg, IDC_DATESUBFOLDER) == BST_CHECKED;
			if (dtsubfldr)
				db_set_b(NULL, PluginName, "SubFolders", 1);
			else
				db_set_b(NULL, PluginName, "SubFolders", 0);
			catchcrashes = IsDlgButtonChecked(hwndDlg, IDC_CATCHCRASHES) == BST_CHECKED;
			if (catchcrashes)
				db_set_b(NULL, PluginName, "CatchCrashes", 1);
			else
				db_set_b(NULL, PluginName, "CatchCrashes", 0);
		}
		break;
	}
	return FALSE;
}


LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case WM_COMMAND:
		switch ((int)PUGetPluginData(hWnd)) {
		case 0:
			OpenAuthUrl("http://vi.miranda-ng.org/detail/%s");
			break;

		case 1:
			OpenAuthUrl("http://vi.miranda-ng.org/global/%s");
			break;

		case 3:
			TCHAR path[MAX_PATH];
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\VersionInfo.txt"), VersionInfoFolder);
			ShellExecute(NULL, TEXT("open"), path, NULL, NULL, SW_SHOW);
			break;

		}
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		Skin_ReleaseIcon((HICON)SendMessage(hWnd, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0));
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ShowMessage(int type, const TCHAR* format, ...)
{
	POPUPDATAT pi = { 0 };

	va_list va;
	va_start(va, format);
	int len = mir_vsntprintf(pi.lptzText, SIZEOF(pi.lptzText) - 1, format, va);
	pi.lptzText[len] = 0;
	va_end(va);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		mir_tstrcpy(pi.lptzContactName, TEXT(PluginName));
		pi.lchIcon = LoadIconEx(IDI_VI);
		pi.PluginWindowProc = DlgProcPopup;
		pi.PluginData = (void*)type;

		PUAddPopupT(&pi);
	}
	else MessageBox(NULL, pi.lptzText, TEXT(PluginName), MB_OK | MB_ICONINFORMATION);
}
