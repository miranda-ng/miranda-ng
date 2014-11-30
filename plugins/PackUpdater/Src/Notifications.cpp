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

HWND hDlgDld = NULL;
INT UpdatesCount = 0;

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

static INT_PTR CALLBACK PopupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
		if (pmpd > 0)
			mir_free(pmpd);
		return TRUE; //TRUE or FALSE is the same, it gets ignored.
	}
	return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK PopupDlgProc2(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		PopupAction(hDlg, MyOptions.LeftClickAction);
		break;

	case WM_CONTEXTMENU:
		PopupAction(hDlg, MyOptions.RightClickAction);
		break;

	case UM_FREEPLUGINDATA:
		LPMSGPOPUPDATA pmpd = (LPMSGPOPUPDATA)PUGetPluginData(hDlg);
		if (pmpd > 0)
			mir_free(pmpd);
		return TRUE; //TRUE or FALSE is the same, it gets ignored.
	}

	return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

static VOID MakePopupAction(POPUPACTION &pa, INT id)
{
	pa.cbSize = sizeof(POPUPACTION);
	pa.flags = PAF_ENABLED;
	pa.wParam = MAKEWORD(id, BN_CLICKED);
	pa.lParam = 0;
	switch (id) {
	case IDYES:
		pa.lchIcon = Skin_GetIcon("btn_ok");
		strncpy_s(pa.lpzTitle, MODNAME"/Yes", SIZEOF(pa.lpzTitle));
		break;
	case IDNO:
		pa.lchIcon = Skin_GetIcon("btn_cancel");
		strncpy_s(pa.lpzTitle, MODNAME"/No", SIZEOF(pa.lpzTitle));
		break;
	}
}

VOID show_popup(HWND hDlg, LPCTSTR Title, LPCTSTR Text, int Number, int ActType)
{
	LPMSGPOPUPDATA	pmpd = (LPMSGPOPUPDATA)mir_alloc(sizeof(MSGPOPUPDATA));
	if (!pmpd)
		return;

	POPUPDATAT_V2 pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.lchContact = NULL; //(HANDLE)wParam;
	pd.lchIcon = LoadSkinnedIcon(PopupsList[Number].Icon);
	mir_tstrncpy(pd.lptzText, Text, SIZEOF(pd.lptzText));
	mir_tstrncpy(pd.lptzContactName, Title, SIZEOF(pd.lptzContactName));
	switch (MyOptions.DefColors) {
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
		pd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	else
		pd.PluginWindowProc = (WNDPROC)PopupDlgProc2;
	pd.PluginData = pmpd;
	if (Number == 0)
		pd.iSeconds = -1;
	else
		pd.iSeconds = MyOptions.Timeout;
	pd.hNotification = NULL;
	pd.lpActions = pmpd->pa;

	pmpd->hDialog = hDlg;
	if (ActType == 1) {
		MakePopupAction(pmpd->pa[pd.actionCount++], IDYES);
		MakePopupAction(pmpd->pa[pd.actionCount++], IDNO);
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
		Number = 3;
		show_popup(hDlg, Title, Text, Number, 0);
		return TRUE;
	}
	return FALSE;
}

static void __stdcall CreateDownloadDialog(void*)
{
	if ( ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) && db_get_b(NULL,MODNAME, "Popups3", DEFAULT_POPUP_ENABLED))
		hDlgDld = CreateDialog(hInst, MAKEINTRESOURCE(IDD_POPUPDUMMI), NULL, DlgDownloadPop);
	else if (db_get_b(NULL,MODNAME, "Popups3M", DEFAULT_MESSAGE_ENABLED)) {
		mir_tstrncpy(tszDialogMsg, Text, SIZEOF(tszDialogMsg));
		hDlgDld = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DOWNLOAD), NULL, DlgDownload);
	}
}

static void __stdcall DestroyDownloadDialog(void*)
{
	DestroyWindow(hDlgDld);
}

void DlgDownloadProc()
{
	CallFunctionAsync(CreateDownloadDialog, 0);
	if (!DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath)) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("An error occurred while downloading the update.");
		if ( ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) && db_get_b(NULL, MODNAME, "Popups1", DEFAULT_POPUP_ENABLED)) {
			Number = 1;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (db_get_b(NULL, MODNAME, "Popups1M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONSTOP);
	}
	CallFunctionAsync(DestroyDownloadDialog, 0);
}

INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hDlg );
		SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		{
			LVCOLUMN lvc = {0};
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Component Name");
			lvc.cx = 145; // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT("Current Version");
			lvc.cx = 95; // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateT("New Version");
			lvc.cx = 82; // width of column in pixels
			ListView_InsertColumn(hwndList, 2, &lvc);

			//enumerate plugins, fill in list
			//bool one_enabled = false;
			ListView_DeleteAllItems(hwndList);

			LVITEM lvI = {0};

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items.
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_NORECOMPUTE;// | LVIF_IMAGE;

			vector<FILEINFO> &todo = *(vector<FILEINFO> *)lParam;
			for (int i = 0; i < (int)todo.size(); ++i) {
				lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_IMAGE;
				lvI.iSubItem = 0;
				lvI.lParam = (LPARAM)&todo[i];
				lvI.pszText = todo[i].tszDescr;
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);

				lvI.mask = LVIF_TEXT;// | LVIF_IMAGE;

				lvI.iSubItem = 1;
				lvI.pszText = todo[i].tszCurVer;
				ListView_SetItem(hwndList, &lvI);

				lvI.iSubItem = 2;
				lvI.pszText = todo[i].tszNewVer;
				ListView_SetItem(hwndList, &lvI);

				ListView_SetCheckState(hwndList, lvI.iItem, true);
				todo[i].enabled = true;
			}
			HWND hwOk = GetDlgItem(hDlg, IDOK);
			EnableWindow(hwOk, true/*one_enabled ? TRUE : FALSE*/);
			// do this after filling list - enables 'ITEMCHANGED' below
			SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
			Utils_RestoreWindowPositionNoSize(hDlg,0,MODNAME,"ConfirmWindow");
		}
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

					vector<FILEINFO> &todo = *(vector<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						todo[lvI.iItem].enabled = ListView_GetCheckState(hwndList, nmlv->iItem);

						bool enableOk = false;
						for(int i=0; i<(int)todo.size(); ++i) {
							if(todo[i].enabled) {
								enableOk = true;
								break;
							}
						}
						HWND hwOk = GetDlgItem(hDlg, IDOK);
						EnableWindow(hwOk, enableOk ? TRUE : FALSE);
					}
					if (nmlv->uNewState & LVIS_SELECTED) {
						if (mir_tstrcmp(todo[lvI.iItem].tszInfoURL, _T("")))
							EnableWindow(GetDlgItem(hDlg, IDC_INFO), TRUE);
						else
							EnableWindow(GetDlgItem(hDlg, IDC_INFO), FALSE);
						SetDlgItemText(hDlg, IDC_MESSAGE, TranslateTS(todo[lvI.iItem].tszMessage));
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
					vector<FILEINFO> &todo = *(vector<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					ShowWindow(hDlg, SW_HIDE);
					TCHAR tszBuff[2048] = {0}, tszFileDest[MAX_PATH] = {0}, tszFilePathDest[MAX_PATH] = {0}, tszFilePathBack[MAX_PATH] = {0}, tszFileName[MAX_PATH] = {0};
					TCHAR* tszExt = NULL;
					char szKey[64] = {0};
					vector<int> arFileType;
					vector<tString> arFilePath;
					vector<tString> arFileName;
					vector<tString> arAdvFolder;
					vector<tString> arExt;
					STARTUPINFO si;
					PROCESS_INFORMATION pi;

					SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
					Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ConfirmWindow");

					arFileType.clear();
					arFilePath.clear();
					arFileName.clear();
					arAdvFolder.clear();
					arExt.clear();
					for(int i=0; i<(int)todo.size(); ++i) {
						if(todo[i].enabled) {
							switch (todo[i].FileType) {
							case 1:
								mir_sntprintf(tszFileDest, SIZEOF(tszFileDest), _T("%s\\Pack"), tszRoot);
								CreateDirectory(tszFileDest, NULL);
								break;
							case 2:
								mir_sntprintf(tszFileDest, SIZEOF(tszFileDest), _T("%s\\Plugins"), tszRoot);
								CreateDirectory(tszFileDest, NULL);
								break;
							case 3:
								mir_sntprintf(tszFileDest, SIZEOF(tszFileDest), _T("%s\\Icons"), tszRoot);
								CreateDirectory(tszFileDest, NULL);
								break;
							case 4:
								mir_sntprintf(tszFileDest, SIZEOF(tszFileDest), _T("%s\\Others"), tszRoot);
								CreateDirectory(tszFileDest, NULL);
								break;
							case 5:
								mir_sntprintf(tszFileDest, SIZEOF(tszFileDest), _T("%s\\Others"), tszRoot);
								CreateDirectory(tszFileDest, NULL);
								break;
							default:
								mir_tstrncpy(tszFileDest, tszRoot, SIZEOF(tszFileDest));
								break;
							}
							mir_sntprintf(tszBuff, SIZEOF(tszBuff), _T("%s\\Backups"), tszRoot);
							CreateDirectory(tszBuff, NULL);
							mir_tstrncpy(tszFileName, todo[i].File.tszDiskPath, SIZEOF(tszFileName));
							mir_sntprintf(todo[i].File.tszDiskPath, SIZEOF(todo[i].File.tszDiskPath), _T("%s\\%s"), tszFileDest, tszFileName);
							UpdatesCount++;

							tszExt = &todo[i].File.tszDownloadURL[mir_tstrlen(todo[i].File.tszDownloadURL)-5];
							if (mir_tstrcmp(tszExt, _T(".html")) == 0) {
								char* szUrl = mir_t2a(todo[i].File.tszDownloadURL);
								CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)szUrl);
								mir_free(szUrl);
							}
							else {
								// download update
								pFileUrl = &todo[i].File;
								Title = TranslateT("Pack Updater");
								if (todo[i].FileType == 1)
									Text = TranslateT("Downloading pack updates...");
								else
									Text = TranslateT("Downloading update...");
								DlgDownloadProc();
								if (!DlgDld) {
									if (UpdatesCount)
										UpdatesCount--;
									continue;
								}
							}
							mir_tstrncpy(todo[i].tszCurVer, todo[i].tszNewVer, SIZEOF(todo[i].tszCurVer));
							mir_snprintf(szKey, SIZEOF(szKey), "File_%d_CurrentVersion", todo[i].FileNum);
							db_set_ts(NULL, MODNAME, szKey, todo[i].tszCurVer);
							arFileType.push_back(todo[i].FileType);
							arFilePath.push_back(todo[i].File.tszDiskPath);
							arFileName.push_back(tszFileName);
							arAdvFolder.push_back(todo[i].tszAdvFolder);
							arExt.push_back(tszExt);
							if (todo[i].FileType == 1)
								i = (int)todo.size();
						}
					}

					if (UpdatesCount > 1 && mir_tstrcmp(arExt[0].c_str(), _T(".html")) != 0)
						mir_tstrncpy(tszBuff, TranslateT("Downloads complete. Start updating? All your data will be saved and Miranda NG will be closed."), SIZEOF(tszBuff));
					else if (UpdatesCount == 1 && mir_tstrcmp(arExt[0].c_str(), _T(".html")) != 0)
						mir_tstrncpy(tszBuff, TranslateT("Download complete. Start updating? All your data will be saved and Miranda NG will be closed."), SIZEOF(tszBuff));
					if (UpdatesCount > 0 && mir_tstrcmp(arExt[0].c_str(), _T(".html")) != 0) {
						INT rc = -1;
						Title = TranslateT("Pack Updater");
						Text = tszBuff;
						if (ServiceExists(MS_POPUP_ADDPOPUPT) && ServiceExists(MS_POPUP_REGISTERACTIONS) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) && db_get_b(NULL,MODNAME, "Popups0", DEFAULT_POPUP_ENABLED) && (db_get_dw(NULL, "Popup", "Actions", 0) & 1))
							rc = DialogBox(hInst, MAKEINTRESOURCE(IDD_POPUPDUMMI), NULL, DlgMsgPop);
						else
							rc = MessageBox(NULL, tszBuff, Title, MB_YESNO | MB_ICONQUESTION);
						if (rc == IDYES) {
							for (int i = 0; i < UpdatesCount; i++) {
								TCHAR* tszUtilRootPlug = NULL; 
								TCHAR* tszUtilRootIco = NULL;
								TCHAR* tszUtilRoot = NULL;
								TCHAR  tszCurrentDir[MAX_PATH];

								switch (arFileType[i]) {
								case 0:
									break;
								case 1:
									if (Reminder == 2)
										db_set_b(NULL, MODNAME, "Reminder", 1);
									memset(&si, 0, sizeof(STARTUPINFO));
									memset(&pi, 0, sizeof(PROCESS_INFORMATION));
									si.cb = sizeof(STARTUPINFO);
									{
										_tcsncpy_s(tszCurrentDir, arFilePath[i].c_str(), _TRUNCATE);
										TCHAR *p = _tcsrchr(tszCurrentDir, '\\');
										if (p) *p = 0;
									}
									CreateProcess(arFilePath[i].c_str(), _T(""), NULL, NULL, FALSE, NULL, NULL, tszCurrentDir, &si, &pi);
									i = UpdatesCount;
									CallFunctionAsync(ExitMe, 0);
									break;
								case 2:
									tszUtilRootPlug = Utils_ReplaceVarsT(_T("%miranda_path%\\Plugins"));
									if (mir_tstrcmp(arAdvFolder[i].c_str(), _T("")) == 0)
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRootPlug, arFileName[i].c_str());
									else
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRootPlug, arAdvFolder[i].c_str(), arFileName[i].c_str());
									mir_sntprintf(tszFilePathBack, SIZEOF(tszFilePathBack), _T("%s\\Backups\\%s"), tszRoot, arFileName[i].c_str());
									MoveFile(tszFilePathDest, tszFilePathBack);
									MoveFile(arFilePath[i].c_str(), tszFilePathDest);
									mir_free(tszUtilRootPlug);
									if (i == UpdatesCount - 1)
										CallFunctionAsync(RestartMe, 0);
									break;
								case 3:
									tszUtilRootIco = Utils_ReplaceVarsT(_T("%miranda_path%\\Icons"));
									if (mir_tstrcmp(arAdvFolder[i].c_str(), _T("")) == 0)
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRootIco, arFileName[i].c_str());
									else
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRootIco, arAdvFolder[i].c_str(), arFileName[i].c_str());
									mir_sntprintf(tszFilePathBack, SIZEOF(tszFilePathBack), _T("%s\\Backups\\%s"), tszRoot, arFileName[i].c_str());
									MoveFile(tszFilePathDest, tszFilePathBack);
									MoveFile(arFilePath[i].c_str(), tszFilePathDest);
									mir_free(tszUtilRootIco);
									if (i == UpdatesCount - 1)
										CallFunctionAsync(RestartMe, 0);
									break;
								case 4:
									tszUtilRoot = Utils_ReplaceVarsT(_T("%miranda_path%"));
									if (mir_tstrcmp(arAdvFolder[i].c_str(), _T("")) == 0)
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRoot, arFileName[i].c_str());
									else
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRoot, arAdvFolder[i].c_str(), arFileName[i].c_str());
									mir_sntprintf(tszFilePathBack, SIZEOF(tszFilePathBack), _T("%s\\Backups\\%s"), tszRoot, arFileName[i].c_str());
									MoveFile(tszFilePathDest, tszFilePathBack);
									MoveFile(arFilePath[i].c_str(), tszFilePathDest);
									mir_free(tszUtilRoot);
									if (i == UpdatesCount - 1)
										CallFunctionAsync(RestartMe, 0);
									break;
								case 5:
									tszUtilRoot = Utils_ReplaceVarsT(_T("%miranda_path%"));
									if (mir_tstrcmp(arAdvFolder[i].c_str(), _T("")) == 0)
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRoot, arFileName[i].c_str());
									else
										mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRoot, arAdvFolder[i].c_str(), arFileName[i].c_str());
									mir_sntprintf(tszFilePathBack, SIZEOF(tszFilePathBack), _T("%s\\Backups\\%s"), tszRoot, arFileName[i].c_str());
									MoveFile(tszFilePathDest, tszFilePathBack);
									MoveFile(arFilePath[i].c_str(), tszFilePathDest);
									mir_free(tszUtilRoot);
									break;
								}
							}
						}
						else { //reminder for not installed pack update
							if (Reminder && (UpdatesCount == 1) && (arFileType[0] == 1))
								db_set_b(NULL, MODNAME, "Reminder", 2);
							mir_sntprintf(tszBuff, SIZEOF(tszBuff), TranslateT("You have chosen not to install the pack update immediately.\nYou can install it manually from this location:\n\n%s"), arFilePath[0].c_str());
							Title = TranslateT("Pack Updater");
							Text = tszBuff;
							if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) && db_get_b(NULL, MODNAME, "Popups2", DEFAULT_POPUP_ENABLED)) {
								Number = 2;
								show_popup(0, Title, Text, Number, 0);
							}
							else if (db_get_b(NULL, MODNAME, "Popups2M", DEFAULT_MESSAGE_ENABLED))
								MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
						}
					}
				}
				EndDialog(hDlg, IDOK);
				return TRUE;

			case IDCANCEL:
				SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ConfirmWindow");
				EndDialog(hDlg, IDCANCEL);
				return TRUE;

			case IDC_INFO:
				int sel = ListView_GetSelectionMark(hwndList);
				vector<FILEINFO> &todo = *(vector<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
				char* szUrl = mir_t2a(todo[sel].tszInfoURL);
				CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)szUrl);
				mir_free(szUrl);
			}
			break;
		}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		Number = 0;
		show_popup(hDlg, Title, Text, Number, 1);
		return TRUE;
	}
	ShowWindow(hDlg, SW_HIDE);
	return FALSE;
}
