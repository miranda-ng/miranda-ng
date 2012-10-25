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

HWND hwndDialog = NULL;
static bool bShowDetails;

void PopupAction(HWND hWnd, BYTE action)
{
	switch (action)
	{
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
	if ( !ServiceExists(MS_POPUP_ADDPOPUPEX) || !DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1) ) {
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

void DlgDownloadProc(FILEURL *pFileUrl, PopupDataText temp)
{
}

void SelectAll(HWND hDlg, bool bEnable)
{
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	for (int i=0; i < todo.getCount(); i++) {
		ListView_SetCheckState(hwndList, i, bEnable);
		todo[i].enabled = bEnable;
	}
}

static void SetStringText(HWND hWnd, size_t i, TCHAR *ptszText)
{
	ListView_SetItemText(hWnd, i, 1, ptszText);
}

static void ApplyUpdates(void *param)
{
	HWND hDlg = (HWND)param;
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	TCHAR tszBuff[2048], tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);
	CreateDirectory(tszFileBack, NULL);

	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);
	CreateDirectory(tszFileTemp, NULL);

	for (int i=0; i < todo.getCount(); ++i) {
		ListView_EnsureVisible(hwndList, i, FALSE);
		if ( !todo[i].enabled || todo[i].bDeleteOnly) {
			SetStringText(hwndList, i, TranslateT("Skipped"));
			continue;
		}

		// download update
		SetStringText(hwndList, i, TranslateT("Downloading..."));

		FILEURL *pFileUrl = &todo[i].File;
		if ( !DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath))
			SetStringText(hwndList, i, TranslateT("Failed!"));
		else
			SetStringText(hwndList, i, TranslateT("Succeeded."));
	}

	if (todo.getCount() == 0) {
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
		DestroyWindow(hDlg);
		return;
	}

	TCHAR *tszMirandaPath = Utils_ReplaceVarsT(_T("%miranda_path%"));

	for (int i = 0; i < todo.getCount(); i++) {
		if ( !todo[i].enabled)
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
			DeleteFile(p.File.tszDiskPath);  // remove .zip after successful update
	}

	DBWriteContactSettingByte(NULL, MODNAME, "RestartCount", 2);
	DestroyWindow(hDlg);
	CallFunctionAsync(RestartMe, 0);
}

static void ResizeVert(HWND hDlg, int yy)
{
	RECT r = { 0, 0, 244, yy };
	MapDialogRect(hDlg, &r);
	r.bottom += GetSystemMetrics(SM_CYSMCAPTION);
	SetWindowPos(hDlg, 0, 0, 0, r.right, r.bottom, SWP_NOMOVE | SWP_NOZORDER);
}

//
//   FUNCTION: IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as 
//   administrator. In other words, it dictates whether the primary access 
//   token of the process belongs to user account that is a member of the 
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group and it is elevated. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
//
BOOL IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority, 
		2, 
		SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_ADMINS, 
		0, 0, 0, 0, 0, 0, 
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

//
//   FUNCTION: IsProcessElevated()
//
//   PURPOSE: The function gets the elevation information of the current 
//   process. It dictates whether the process is elevated or not. Token 
//   elevation is only available on Windows Vista and newer operating 
//   systems, thus IsProcessElevated throws a C++ exception if it is called 
//   on systems prior to Windows Vista. It is not appropriate to use this 
//   function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if 
//   it is not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
//   which contains the Win32 error code of the failure. For example, if 
//   IsProcessElevated is called on systems prior to Windows Vista, the error 
//   code will be ERROR_INVALID_PARAMETER.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the 
//   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
//   TokenElevationTypeFull) of the process. It is different from 
//   TokenElevation in that, when UAC is turned off, elevation type always 
//   returns TokenElevationTypeDefault even though the process is elevated 
//   (Integrity Level == High). In other words, it is not safe to say if the 
//   process is elevated based on elevation type. Instead, we should use 
//   TokenElevation.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsProcessElevated())
//             wprintf (L"Process is elevated\n");
//         else
//             wprintf (L"Process is not elevated\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
//     }
//
BOOL IsProcessElevated()
{
	BOOL fIsElevated = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	HANDLE hToken = NULL;

	// Open the primary access token of the process with TOKEN_QUERY.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Retrieve token elevation information.
	TOKEN_ELEVATION elevation;
	DWORD dwSize;
	if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
	{
		// When the process is run on operating systems prior to Windows 
		// Vista, GetTokenInformation returns FALSE with the 
		// ERROR_INVALID_PARAMETER error code because TokenElevation is 
		// not supported on those operating systems.
		dwError = GetLastError();
		goto Cleanup;
	}

	fIsElevated = elevation.TokenIsElevated;

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsElevated;
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
				todo[i].enabled = true;
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
						todo[lvI.iItem].enabled = ListView_GetCheckState(hwndList, nmlv->iItem);

						bool enableOk = false;
						for(int i=0; i < todo.getCount(); ++i) {
							if(todo[i].enabled) {
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
			{
				EnableWindow( GetDlgItem(hDlg, IDOK), FALSE);
				EnableWindow( GetDlgItem(hDlg, IDC_SELALL), FALSE);
				EnableWindow( GetDlgItem(hDlg, IDC_SELNONE), FALSE);

				wchar_t szPath[MAX_PATH];
				GetModuleFileName(NULL, szPath, SIZEOF(szPath));
				TCHAR *ext = _tcsrchr(szPath, '.');
				if (ext != NULL)
					*ext = '\0';
				_tcscat(szPath, _T(".test"));
				HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == INVALID_HANDLE_VALUE)
				{
					// Check the current process's "run as administrator" status.
					// Elevate the process if it is not run as administrator.
					if (!IsRunAsAdmin())
					{
						wchar_t cmdLine[100];
						GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
						TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
						mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T(" /restart:%d /profile=%s"), GetCurrentProcessId(), profilename);
						// Launch itself as administrator.
						SHELLEXECUTEINFO sei = { sizeof(sei) };
						sei.lpVerb = L"runas";
						sei.lpFile = szPath;
						sei.lpParameters = cmdLine;
						sei.hwnd = hDlg;
						sei.nShow = SW_NORMAL;

						if (!ShellExecuteEx(&sei))
						{
							DWORD dwError = GetLastError();
							if (dwError == ERROR_CANCELLED)
							{
								// The user refused to allow privileges elevation.
								// Do nothing ...
							}
						}
						else
						{
							DestroyWindow(hDlg);  // Quit itself
							CallService("CloseAction", 0, 0);
							break;
						}
					}
				}
				else
				{
					CloseHandle(hFile);
					DeleteFile(szPath);
				}
				mir_forkthread(ApplyUpdates, hDlg);
				return TRUE;
			}

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
