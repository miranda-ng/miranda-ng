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
HWND hwndDialog = NULL;
static bool bShowDetails;

void PopupAction(HWND hWnd, BYTE action)
{
	switch (action) {
	case PCA_CLOSEPOPUP:
		break;
	case PCA_DONOTHING:
		return;
	}
	PUDeletePopUp(hWnd);
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
					PUDeletePopUp(hDlg);
					break;

				case IDNO:
					if (IsWindow(pmpd->hDialog))
						EndDialog(pmpd->hDialog, LOWORD(wParam));
					PUDeletePopUp(hDlg);
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

void ShowPopup(HWND hDlg, LPCTSTR ptszTitle, LPCTSTR ptszText, int Number, int ActType)
{	
	if ( !ServiceExists(MS_POPUP_ADDPOPUP) || !DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1) ) {
		char setting[100];
		mir_snprintf(setting, SIZEOF(setting), "Popups%dM", Number);
		if (DBGetContactSettingByte(NULL, MODNAME, setting, DEFAULT_MESSAGE_ENABLED)) {
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
		return;
	}

	LPMSGPOPUPDATA	pmpd = (LPMSGPOPUPDATA)mir_alloc(sizeof(MSGPOPUPDATA));
	if (!pmpd)
		return;

	POPUPDATAT_V2 pd = { 0 };
	pd.cbSize = sizeof(POPUPDATAT_V2);
	pd.lchContact = NULL; //(HANDLE)wParam;
	pd.lchIcon = LoadSkinnedIcon(PopupsList[Number].Icon);
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
	if (Number == 0 && ActType != 0)
		pd.PluginWindowProc = PopupDlgProc;
	else
		pd.PluginWindowProc = PopupDlgProc2;
	pd.PluginData = pmpd;
	if (Number == 0)
		pd.iSeconds = -1;
	else
		pd.iSeconds = PopupOptions.Timeout;
	pd.hNotification = NULL;
	pd.lpActions = pmpd->pa;

	pmpd->hDialog = hDlg;
	switch (ActType) {
	case 0:
		break;

	case 1:
		MakePopupAction(pmpd->pa[pd.actionCount++], IDYES);
		MakePopupAction(pmpd->pa[pd.actionCount++], IDNO);
		break;
	}

	CallService(MS_POPUP_ADDPOPUPT, (WPARAM) &pd, APF_NEWDATA);
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
	wchar_t szPath[MAX_PATH];
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
	_stprintf_s(tszPipeName, MAX_PATH, _T("\\\\.\\pipe\\Miranda_Pu_%d"), GetCurrentProcessId());
	hPipe = CreateNamedPipe(tszPipeName, PIPE_ACCESS_DUPLEX, PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		hPipe = NULL;
		return false;
	}

	wchar_t cmdLine[100], *p;
	GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
	if ((p = wcsrchr(szPath, '\\')) != 0)
		wcscpy(p+1, L"pu_stub.exe");
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), L"%d", GetCurrentProcessId());

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

void SelectAll(HWND hDlg, bool bEnable)
{
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	for (int i=0; i < todo.getCount(); i++) {
		ListView_SetCheckState(hwndList, i, bEnable);
		todo[i].bEnabled = bEnable;
	}
}

static void SetStringText(HWND hWnd, size_t i, TCHAR *ptszText)
{
	ListView_SetItemText(hWnd, i, 1, ptszText);
}

static void ApplyUpdates(void *param)
{
	HWND hDlg = (HWND)param;

	//////////////////////////////////////////////////////////////////////////////////////
	// if we need to escalate priviledges, launch a atub

	if ( !PrepareEscalation()) {
		DestroyWindow(hDlg);
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ok, let's unpack all zips

	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	TCHAR tszBuff[2048], tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);
	SafeCreateDirectory(tszFileBack);

	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);
	SafeCreateDirectory(tszFileTemp);

	for (int i=0; i < todo.getCount(); ++i) {
		ListView_EnsureVisible(hwndList, i, FALSE);
		if ( !todo[i].bEnabled) {
			SetStringText(hwndList, i, TranslateT("Skipped."));
			continue;
		}
		if (todo[i].bDeleteOnly) {
			SetStringText(hwndList, i, TranslateT("Will be deleted!"));
			continue;
		}

		// download update
		SetStringText(hwndList, i, TranslateT("Downloading..."));

		FILEURL *pFileUrl = &todo[i].File;
		if ( !DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath, pFileUrl->CRCsum))
			SetStringText(hwndList, i, TranslateT("Failed!"));
		else
			SetStringText(hwndList, i, TranslateT("Succeeded."));
	}

	if (todo.getCount() == 0) {
LBL_Exit:
		if (hPipe)
			CloseHandle(hPipe);
		DestroyWindow(hDlg);
		return;
	}

	PopupDataText temp;
	temp.Title = TranslateT("Plugin Updater");
	temp.Text = tszBuff;
	lstrcpyn(tszBuff, TranslateT("Download complete. Start updating? All your data will be saved and Miranda NG will be closed."), SIZEOF(tszBuff));
	int rc = MessageBox(NULL, temp.Text, temp.Title, MB_YESNO | MB_ICONQUESTION);
	if (rc != IDYES) {
		mir_sntprintf(tszBuff, SIZEOF(tszBuff), TranslateT("You have chosen not to install the plugin updates immediately.\nYou can install it manually from this location:\n\n%s"), tszFileTemp);
		ShowPopup(0, LPGENT("Plugin Updater"), tszBuff, 2, 0);
		goto LBL_Exit;
	}

	TCHAR *tszMirandaPath = Utils_ReplaceVarsT(_T("%miranda_path%"));

	for (int i = 0; i < todo.getCount(); i++) {
		if ( !todo[i].bEnabled)
			continue;

		TCHAR tszBackFile[MAX_PATH];
		FILEINFO& p = todo[i];
		if (p.bDeleteOnly) { // we need only to backup the old file
			TCHAR *ptszRelPath = p.tszNewName + _tcslen(tszMirandaPath) + 1;
			mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), tszFileBack, ptszRelPath);
			BackupFile(p.tszNewName, tszBackFile);
			continue;
		}
		
		// if file name differs, we also need to backup the old file here
		// otherwise it would be replaced by unzip
		if ( _tcsicmp(p.tszOldName, p.tszNewName)) {
			TCHAR tszSrcPath[MAX_PATH];
			mir_sntprintf(tszSrcPath, SIZEOF(tszSrcPath), _T("%s\\%s"), tszMirandaPath, p.tszOldName);
			mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), tszFileBack, p.tszOldName);
			BackupFile(tszSrcPath, tszBackFile);
		}

		if ( unzip(p.File.tszDiskPath, tszMirandaPath, tszFileBack))
			SafeDeleteFile(p.File.tszDiskPath);  // remove .zip after successful update
	}

	DBWriteContactSettingByte(NULL, MODNAME, "RestartCount", 2);
	CallFunctionAsync(RestartMe, 0);
	goto LBL_Exit;
}

static void ResizeVert(HWND hDlg, int yy)
{
	RECT r = { 0, 0, 244, yy };
	MapDialogRect(hDlg, &r);
	r.bottom += GetSystemMetrics(SM_CYSMCAPTION);
	SetWindowPos(hDlg, 0, 0, 0, r.right, r.bottom, SWP_NOMOVE | SWP_NOZORDER);
}

INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		hwndDialog = hDlg;
		TranslateDialogDefault( hDlg );
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		{
			OSVERSIONINFO osver = { sizeof(osver) };
			if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
			{
				wchar_t szPath[MAX_PATH];
				GetModuleFileName(NULL, szPath, SIZEOF(szPath));
				TCHAR *ext = _tcsrchr(szPath, '.');
				if (ext != NULL)
					*ext = '\0';
				_tcscat(szPath, _T(".test"));
				HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == INVALID_HANDLE_VALUE)
					// Running Windows Vista or later (major version >= 6).
					Button_SetElevationRequiredState(GetDlgItem(hDlg, IDOK), !IsProcessElevated());
				else
				{
					CloseHandle(hFile);
					DeleteFile(szPath);
				}
			}
			RECT r;
			GetClientRect(hwndList, &r);

			LVCOLUMN lvc = {0};
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Component Name");
			lvc.cx = 220; // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT("State");
			lvc.cx = 120 - GetSystemMetrics(SM_CXVSCROLL); // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			//enumerate plugins, fill in list
			//bool one_enabled = false;
			ListView_DeleteAllItems(hwndList);

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items.
			LVITEM lvI = {0};
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_NORECOMPUTE;// | LVIF_IMAGE;

			OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)lParam;
			for (int i = 0; i < todo.getCount(); ++i) {
				lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_IMAGE;
				lvI.iSubItem = 0;
				lvI.lParam = (LPARAM)&todo[i];
				lvI.pszText = todo[i].tszOldName;
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);

				// remember whether the user has decided not to update this component with this particular new version
				ListView_SetCheckState(hwndList, lvI.iItem, true);
				todo[i].bEnabled = true;
			}
			HWND hwOk = GetDlgItem(hDlg, IDOK);
			EnableWindow(hwOk, true);
		}

		bShowDetails = false;
		ResizeVert(hDlg, 60);

		// do this after filling list - enables 'ITEMCHANGED' below
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Utils_RestoreWindowPositionNoSize(hDlg, 0, MODNAME, "ConfirmWindow");
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->hwndFrom == hwndList) {
			switch (((LPNMHDR) lParam)->code) {
			case LVN_ITEMCHANGED:
				if (GetWindowLongPtr(hDlg, GWLP_USERDATA)) {
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

					LVITEM lvI = {0};
					lvI.iItem = nmlv->iItem;
					lvI.iSubItem = 0;
					lvI.mask = LVIF_PARAM;
					ListView_GetItem(hwndList, &lvI);

					OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						todo[lvI.iItem].bEnabled = ListView_GetCheckState(hwndList, nmlv->iItem);

						bool enableOk = false;
						for (int i=0; i < todo.getCount(); ++i) {
							if (todo[i].bEnabled) {
								enableOk = true;
								break;
							}
						}
						HWND hwOk = GetDlgItem(hDlg, IDOK);
						EnableWindow(hwOk, enableOk ? TRUE : FALSE);
					}
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		if (HIWORD( wParam ) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
			case IDOK:
				EnableWindow( GetDlgItem(hDlg, IDOK), FALSE);
				EnableWindow( GetDlgItem(hDlg, IDC_SELALL), FALSE);
				EnableWindow( GetDlgItem(hDlg, IDC_SELNONE), FALSE);

				mir_forkthread(ApplyUpdates, hDlg);
				return TRUE;

			case IDC_DETAILS:
				bShowDetails = !bShowDetails;
				ResizeVert(hDlg, bShowDetails ? 242 : 60);
				break;

			case IDC_SELALL:
				SelectAll(hDlg, true);
				break;

			case IDC_SELNONE:
				SelectAll(hDlg, false);
				break;

			case IDCANCEL:
				Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ConfirmWindow");
				DestroyWindow(hDlg);
				return TRUE;
			}
		}
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ConfirmWindow");
		hwndDialog = NULL;
		delete (OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
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
