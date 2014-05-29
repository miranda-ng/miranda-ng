/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt. If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "common.h"

HANDLE hPipe = NULL;

void PopupAction(HWND hWnd, BYTE action)
{
	switch (action) {
	case PCA_CLOSEPOPUP:
		break;
	case PCA_DONOTHING:
		return;
	}
	PUDeletePopup(hWnd);
}

static LRESULT CALLBACK PopupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case UM_POPUPACTION:
		if (HIWORD(wParam) == BN_CLICKED) {
			LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
			if (pmpd) {
				switch (LOWORD(wParam)) {
				case IDYES:
					if (IsWindow(pmpd->hDialog))
						EndDialog(pmpd->hDialog, LOWORD(wParam));
					PUDeletePopup(hDlg);
					break;

				case IDNO:
					if (IsWindow(pmpd->hDialog))
						EndDialog(pmpd->hDialog, LOWORD(wParam));
					PUDeletePopup(hDlg);
					break;
				}
			}
		}
		break;

	case UM_FREEPLUGINDATA:
		{
			LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
			if (pmpd > 0)
				mir_free(pmpd);
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}
		break;
	}

	return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

static LRESULT CALLBACK PopupDlgProc2(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		PopupAction(hDlg, PopupOptions.LeftClickAction);
		break;

	case WM_CONTEXTMENU:
		PopupAction(hDlg, PopupOptions.RightClickAction);
		break;

	case UM_FREEPLUGINDATA:
		{
			LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
			if (pmpd > 0)
				mir_free(pmpd);
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}
		break;
	}

	return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

static void MakePopupAction(POPUPACTION &pa, int id)
{
	pa.cbSize = sizeof(POPUPACTION);
	pa.flags = PAF_ENABLED;
	pa.wParam = MAKEWORD(id, BN_CLICKED);
	pa.lParam = 0;
	switch (id) {
	case IDYES:
		pa.lchIcon = Skin_GetIcon("btn_ok");
		strncpy_s(pa.lpzTitle, MODNAME"/Yes", MAX_ACTIONTITLE);
		break;

	case IDNO:
		pa.lchIcon = Skin_GetIcon("btn_cancel");
		strncpy_s(pa.lpzTitle, MODNAME"/No", MAX_ACTIONTITLE);
		break;
	}
}

void ShowPopup(HWND hDlg, LPCTSTR ptszTitle, LPCTSTR ptszText, int Number, int ActType, bool NoMessageBox)
{	
	if (ServiceExists(MS_POPUP_ADDPOPUPT) && (db_get_b(NULL, "Popup", "ModuleIsEnabled", 0) || db_get_b(NULL, "YAPP", "Enabled", 0))) {
		LPMSGPOPUPDATA	pmpd = (LPMSGPOPUPDATA)mir_alloc(sizeof(MSGPOPUPDATA));
		if (!pmpd)
			return;

		POPUPDATAT_V2 pd = { 0 };
		pd.cbSize = sizeof(pd);
		pd.lchContact = NULL; //(HANDLE)wParam;
		pd.lchIcon = LoadSkinnedIcon(PopupsList[Number].Icon);
		pd.PluginWindowProc = (Number == 0 && ActType != 0) ? PopupDlgProc : PopupDlgProc2;
		pd.PluginData = pmpd;
		pd.iSeconds = (Number == 0) ? -1 : PopupOptions.Timeout;
		pd.hNotification = NULL;
		pd.lpActions = pmpd->pa;

		lstrcpyn(pd.lptzText, TranslateTS(ptszText), MAX_SECONDLINE);
		lstrcpyn(pd.lptzContactName, TranslateTS(ptszTitle), MAX_CONTACTNAME);

		switch (PopupOptions.DefColors) {
		case byCOLOR_WINDOWS:
			pd.colorBack = GetSysColor(COLOR_BTNFACE);
			pd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;
		case byCOLOR_OWN:
			pd.colorBack = PopupsList[Number].colorBack;
			pd.colorText = PopupsList[Number].colorText;
			break;
		case byCOLOR_POPUP:
			pd.colorBack = pd.colorText = 0;
			break;
		}

		pmpd->hDialog = hDlg;
		
		if (ActType == 1) {
			MakePopupAction(pmpd->pa[pd.actionCount++], IDYES);
			MakePopupAction(pmpd->pa[pd.actionCount++], IDNO);
		}

		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&pd, APF_NEWDATA);
	} else if (!NoMessageBox) {
		char setting[100];
		mir_snprintf(setting, SIZEOF(setting), "Popups%dM", Number);
		if (db_get_b(NULL, MODNAME, setting, DEFAULT_MESSAGE_ENABLED)) {
			int iMsgType;
			switch( Number ) {
				case 1: iMsgType = MB_ICONSTOP; break;
				case 2: iMsgType = MB_ICONINFORMATION; break;
				case 3: iMsgType = MB_ICONQUESTION; break;
				case 4: return;
				default: iMsgType = 0;
			}
			MessageBox(hDlg, TranslateTS(ptszText), TranslateTS(ptszTitle), iMsgType);
		}
	}
}

INT_PTR CALLBACK DlgDownload(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hDlg, IDC_LABEL), tszDialogMsg);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PB), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hDlg, IDC_PB), GWL_STYLE) | PBS_MARQUEE);
		SendMessage(GetDlgItem(hDlg, IDC_PB), PBM_SETMARQUEE, 1, 50);
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgDownloadPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		PopupDataText *temp = (PopupDataText*)lParam;
		ShowPopup(hDlg, temp->Title, temp->Text, 3, 0);
		return TRUE;
	}
	return FALSE;
}

bool PrepareEscalation()
{
	// First try to create a file near Miranda32.exe
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, SIZEOF(szPath));
	TCHAR *ext = _tcsrchr(szPath, '.');
	if (ext != NULL)
		*ext = '\0';
	_tcscat(szPath, _T(".test"));
	HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		// we are admins or UAC is disable, cool
		CloseHandle(hFile);
		DeleteFile(szPath);
		return true;
	}

	// Check the current process's "run as administrator" status.
	if ( IsRunAsAdmin())
		return true;

	// Elevate the process. Create a pipe for a stub first
	TCHAR tszPipeName[MAX_PATH];
	mir_sntprintf(tszPipeName, MAX_PATH, _T("\\\\.\\pipe\\Miranda_Pu_%d"), GetCurrentProcessId());
	hPipe = CreateNamedPipe(tszPipeName, PIPE_ACCESS_DUPLEX, PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		hPipe = NULL;
		return false;
	}

	TCHAR cmdLine[100], *p;
	GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
	if ((p = _tcsrchr(szPath, '\\')) != 0)
		_tcscpy(p+1, _T("pu_stub.exe"));
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("%d"), GetCurrentProcessId());

	// Launch a stub
	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.lpVerb = L"runas";
	sei.lpFile = szPath;
	sei.lpParameters = cmdLine;
	sei.hwnd = NULL;
	sei.nShow = SW_NORMAL;
	if (ShellExecuteEx(&sei)) {
		if (hPipe != NULL)
			ConnectNamedPipe(hPipe, NULL);
		return true;
	}

	DWORD dwError = GetLastError();
	if (dwError == ERROR_CANCELLED)
	{
		// The user refused to allow privileges elevation.
		// Do nothing ...
	}
	return false;
}

int ImageList_AddIconFromIconLib(HIMAGELIST hIml, const char *name)
{
	HICON icon = Skin_GetIconByHandle(Skin_GetIconHandle(name));
	int res = ImageList_AddIcon(hIml, icon);
	Skin_ReleaseIcon(icon);
	return res;
}

INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		PopupDataText *temp = (PopupDataText*)lParam;
		ShowPopup(hDlg, temp->Title, temp->Text, 0, 1);
		return TRUE;
	}
	ShowWindow(hDlg, SW_HIDE);
	return FALSE;
}
