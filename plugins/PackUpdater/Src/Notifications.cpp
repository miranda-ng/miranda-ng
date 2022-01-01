/*
Copyright (C) 2011-22 Mataes

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

#include "stdafx.h"

HWND hDlgDld = nullptr;
INT UpdatesCount = 0;

void PopupAction(HWND hWnd, uint8_t action)
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

static void MakePopupAction(POPUPACTION &pa, INT id)
{
	pa.cbSize = sizeof(POPUPACTION);
	pa.flags = PAF_ENABLED;
	pa.wParam = MAKEWORD(id, BN_CLICKED);
	pa.lParam = 0;
	switch (id) {
	case IDYES:
		pa.lchIcon = IcoLib_GetIcon("btn_ok");
		strncpy_s(pa.lpzTitle, MODULENAME"/Yes", _countof(pa.lpzTitle));
		break;
	case IDNO:
		pa.lchIcon = IcoLib_GetIcon("btn_cancel");
		strncpy_s(pa.lpzTitle, MODULENAME"/No", _countof(pa.lpzTitle));
		break;
	}
}

void show_popup(HWND hDlg, LPCTSTR pszTitle, LPCTSTR pszText, int iNumber, int ActType)
{
	LPMSGPOPUPDATA	pmpd = (LPMSGPOPUPDATA)mir_alloc(sizeof(MSGPOPUPDATA));
	if (!pmpd)
		return;

	POPUPDATAW ppd;
	ppd.lchIcon = Skin_LoadIcon(PopupsList[iNumber].Icon);
	mir_wstrncpy(ppd.lpwzText, pszText, _countof(ppd.lpwzText));
	mir_wstrncpy(ppd.lpwzContactName, pszTitle, _countof(ppd.lpwzContactName));
	switch (MyOptions.DefColors) {
	case byCOLOR_WINDOWS:
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		break;
	case byCOLOR_OWN:
		ppd.colorBack = PopupsList[iNumber].colorBack;
		ppd.colorText = PopupsList[iNumber].colorText;
		break;
	case byCOLOR_POPUP:
		ppd.colorBack = ppd.colorText = 0;
		break;
	}
	if (iNumber == 0 && ActType != 0)
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	else
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc2;
	ppd.PluginData = pmpd;
	if (iNumber == 0)
		ppd.iSeconds = -1;
	else
		ppd.iSeconds = MyOptions.Timeout;
	ppd.hNotification = nullptr;
	ppd.lpActions = pmpd->pa;

	pmpd->hDialog = hDlg;
	if (ActType == 1) {
		MakePopupAction(pmpd->pa[ppd.actionCount++], IDYES);
		MakePopupAction(pmpd->pa[ppd.actionCount++], IDNO);
	}
	PUAddPopupW(&ppd);
}

INT_PTR CALLBACK DlgDownload(HWND hDlg, UINT message, WPARAM, LPARAM)
{
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_LABEL, tszDialogMsg);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_PB), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hDlg, IDC_PB), GWL_STYLE) | PBS_MARQUEE);
		SendDlgItemMessage(hDlg, IDC_PB, PBM_SETMARQUEE, 1, 50);
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgDownloadPop(HWND hDlg, UINT uMsg, WPARAM, LPARAM)
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
	if (Popup_Enabled() && g_plugin.getByte("Popups3", DEFAULT_POPUP_ENABLED))
		hDlgDld = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POPUPDUMMI), nullptr, DlgDownloadPop);
	else if (g_plugin.getByte("Popups3M", DEFAULT_MESSAGE_ENABLED)) {
		mir_wstrncpy(tszDialogMsg, Text, _countof(tszDialogMsg));
		hDlgDld = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DOWNLOAD), nullptr, DlgDownload);
	}
}

static void __stdcall DestroyDownloadDialog(void*)
{
	DestroyWindow(hDlgDld);
}

void DlgDownloadProc()
{
	CallFunctionAsync(CreateDownloadDialog, nullptr);
	if (!DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath)) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("An error occurred while downloading the update.");
		if (Popup_Enabled() && g_plugin.getByte("Popups1", DEFAULT_POPUP_ENABLED)) {
			Number = 1;
			show_popup(nullptr, Title, Text, Number, 0);
		}
		else if (g_plugin.getByte("Popups1M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(nullptr, Text, Title, MB_ICONSTOP);
	}
	CallFunctionAsync(DestroyDownloadDialog, nullptr);
}

INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		{
			LVCOLUMN lvc = { 0 };
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

			LVITEM lvI = { 0 };

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
			Utils_RestoreWindowPositionNoSize(hDlg, 0, MODULENAME, "ConfirmWindow");
		}
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == hwndList) {
			switch (((LPNMHDR)lParam)->code) {
			case LVN_ITEMCHANGED:
				if (GetWindowLongPtr(hDlg, GWLP_USERDATA)) {
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

					LVITEM lvI = { 0 };

					lvI.iItem = nmlv->iItem;
					lvI.iSubItem = 0;
					lvI.mask = LVIF_PARAM;
					ListView_GetItem(hwndList, &lvI);

					vector<FILEINFO> &todo = *(vector<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						todo[lvI.iItem].enabled = ListView_GetCheckState(hwndList, nmlv->iItem);

						bool enableOk = false;
						for (int i = 0; i < (int)todo.size(); ++i) {
							if (todo[i].enabled) {
								enableOk = true;
								break;
							}
						}
						HWND hwOk = GetDlgItem(hDlg, IDOK);
						EnableWindow(hwOk, enableOk ? TRUE : FALSE);
					}
					if (nmlv->uNewState & LVIS_SELECTED) {
						if (mir_wstrcmp(todo[lvI.iItem].tszInfoURL, L""))
							EnableWindow(GetDlgItem(hDlg, IDC_INFO), TRUE);
						else
							EnableWindow(GetDlgItem(hDlg, IDC_INFO), FALSE);
						SetDlgItemText(hDlg, IDC_MESSAGE, TranslateW(todo[lvI.iItem].tszMessage));
					}
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDOK:
			{
				vector<FILEINFO> &todo = *(vector<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
				ShowWindow(hDlg, SW_HIDE);
				wchar_t tszBuff[2048] = { 0 }, tszFileDest[MAX_PATH] = { 0 }, tszFilePathDest[MAX_PATH] = { 0 }, tszFilePathBack[MAX_PATH] = { 0 }, tszFileName[MAX_PATH] = { 0 };
				wchar_t* tszExt = nullptr;
				char szKey[64] = { 0 };
				vector<int> arFileType;
				vector<tString> arFilePath;
				vector<tString> arFileName;
				vector<tString> arAdvFolder;
				vector<tString> arExt;
				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hDlg, NULL, MODULENAME, "ConfirmWindow");

				arFileType.clear();
				arFilePath.clear();
				arFileName.clear();
				arAdvFolder.clear();
				arExt.clear();
				for (int i = 0; i < (int)todo.size(); ++i) {
					if (todo[i].enabled) {
						switch (todo[i].FileType) {
						case 1:
							mir_snwprintf(tszFileDest, L"%s\\Pack", tszRoot);
							CreateDirectory(tszFileDest, nullptr);
							break;
						case 2:
							mir_snwprintf(tszFileDest, L"%s\\Plugins", tszRoot);
							CreateDirectory(tszFileDest, nullptr);
							break;
						case 3:
							mir_snwprintf(tszFileDest, L"%s\\Icons", tszRoot);
							CreateDirectory(tszFileDest, nullptr);
							break;
						case 4:
							mir_snwprintf(tszFileDest, L"%s\\Others", tszRoot);
							CreateDirectory(tszFileDest, nullptr);
							break;
						case 5:
							mir_snwprintf(tszFileDest, L"%s\\Others", tszRoot);
							CreateDirectory(tszFileDest, nullptr);
							break;
						default:
							mir_wstrncpy(tszFileDest, tszRoot, _countof(tszFileDest));
							break;
						}
						mir_snwprintf(tszBuff, L"%s\\Backups", tszRoot);
						CreateDirectory(tszBuff, nullptr);
						mir_wstrncpy(tszFileName, todo[i].File.tszDiskPath, _countof(tszFileName));
						mir_snwprintf(todo[i].File.tszDiskPath, L"%s\\%s", tszFileDest, tszFileName);
						UpdatesCount++;

						tszExt = &todo[i].File.tszDownloadURL[mir_wstrlen(todo[i].File.tszDownloadURL) - 5];
						if (mir_wstrcmp(tszExt, L".html") == 0) {
							char* szUrl = mir_u2a(todo[i].File.tszDownloadURL);
							Utils_OpenUrl(szUrl);
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
						mir_wstrncpy(todo[i].tszCurVer, todo[i].tszNewVer, _countof(todo[i].tszCurVer));
						mir_snprintf(szKey, "File_%d_CurrentVersion", todo[i].FileNum);
						g_plugin.setWString(szKey, todo[i].tszCurVer);
						arFileType.push_back(todo[i].FileType);
						arFilePath.push_back(todo[i].File.tszDiskPath);
						arFileName.push_back(tszFileName);
						arAdvFolder.push_back(todo[i].tszAdvFolder);
						arExt.push_back(tszExt);
						if (todo[i].FileType == 1)
							i = (int)todo.size();
					}
				}

				if (UpdatesCount > 1 && mir_wstrcmp(arExt[0].c_str(), L".html") != 0)
					mir_wstrncpy(tszBuff, TranslateT("Downloads complete. Start updating? All your data will be saved and Miranda NG will be closed."), _countof(tszBuff));
				else if (UpdatesCount == 1 && mir_wstrcmp(arExt[0].c_str(), L".html") != 0)
					mir_wstrncpy(tszBuff, TranslateT("Download complete. Start updating? All your data will be saved and Miranda NG will be closed."), _countof(tszBuff));
				if (UpdatesCount > 0 && mir_wstrcmp(arExt[0].c_str(), L".html") != 0) {
					INT rc = -1;
					Title = TranslateT("Pack Updater");
					Text = tszBuff;
					if (Popup_Enabled() && g_plugin.getByte("Popups0", DEFAULT_POPUP_ENABLED) && (db_get_dw(0, "Popup", "Actions", 0) & 1))
						rc = DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POPUPDUMMI), nullptr, DlgMsgPop);
					else
						rc = MessageBox(nullptr, tszBuff, Title, MB_YESNO | MB_ICONQUESTION);
					if (rc == IDYES) {
						for (int i = 0; i < UpdatesCount; i++) {
							wchar_t* tszUtilRootPlug = nullptr;
							wchar_t* tszUtilRootIco = nullptr;
							wchar_t* tszUtilRoot = nullptr;
							wchar_t  tszCurrentDir[MAX_PATH];

							switch (arFileType[i]) {
							case 0:
								break;
							case 1:
								if (Reminder == 2)
									g_plugin.setByte("Reminder", 1);
								memset(&si, 0, sizeof(STARTUPINFO));
								memset(&pi, 0, sizeof(PROCESS_INFORMATION));
								si.cb = sizeof(STARTUPINFO);
								{
									wcsncpy_s(tszCurrentDir, arFilePath[i].c_str(), _TRUNCATE);
									wchar_t *p = wcsrchr(tszCurrentDir, '\\');
									if (p) *p = 0;
								}
								CreateProcess(arFilePath[i].c_str(), L"", nullptr, nullptr, FALSE, NULL, nullptr, tszCurrentDir, &si, &pi);
								i = UpdatesCount;
								CallFunctionAsync(ExitMe, nullptr);
								break;
							case 2:
								tszUtilRootPlug = Utils_ReplaceVarsW(L"%miranda_path%\\Plugins");
								if (mir_wstrcmp(arAdvFolder[i].c_str(), L"") == 0)
									mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRootPlug, arFileName[i].c_str());
								else
									mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRootPlug, arAdvFolder[i].c_str(), arFileName[i].c_str());
								mir_snwprintf(tszFilePathBack, L"%s\\Backups\\%s", tszRoot, arFileName[i].c_str());
								MoveFile(tszFilePathDest, tszFilePathBack);
								MoveFile(arFilePath[i].c_str(), tszFilePathDest);
								mir_free(tszUtilRootPlug);
								if (i == UpdatesCount - 1)
									CallFunctionAsync(RestartMe, nullptr);
								break;
							case 3:
								tszUtilRootIco = Utils_ReplaceVarsW(L"%miranda_path%\\Icons");
								if (mir_wstrcmp(arAdvFolder[i].c_str(), L"") == 0)
									mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRootIco, arFileName[i].c_str());
								else
									mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRootIco, arAdvFolder[i].c_str(), arFileName[i].c_str());
								mir_snwprintf(tszFilePathBack, L"%s\\Backups\\%s", tszRoot, arFileName[i].c_str());
								MoveFile(tszFilePathDest, tszFilePathBack);
								MoveFile(arFilePath[i].c_str(), tszFilePathDest);
								mir_free(tszUtilRootIco);
								if (i == UpdatesCount - 1)
									CallFunctionAsync(RestartMe, nullptr);
								break;
							case 4:
								tszUtilRoot = Utils_ReplaceVarsW(L"%miranda_path%");
								if (mir_wstrcmp(arAdvFolder[i].c_str(), L"") == 0)
									mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRoot, arFileName[i].c_str());
								else
									mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRoot, arAdvFolder[i].c_str(), arFileName[i].c_str());
								mir_snwprintf(tszFilePathBack, L"%s\\Backups\\%s", tszRoot, arFileName[i].c_str());
								MoveFile(tszFilePathDest, tszFilePathBack);
								MoveFile(arFilePath[i].c_str(), tszFilePathDest);
								mir_free(tszUtilRoot);
								if (i == UpdatesCount - 1)
									CallFunctionAsync(RestartMe, nullptr);
								break;
							case 5:
								tszUtilRoot = Utils_ReplaceVarsW(L"%miranda_path%");
								if (mir_wstrcmp(arAdvFolder[i].c_str(), L"") == 0)
									mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRoot, arFileName[i].c_str());
								else
									mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRoot, arAdvFolder[i].c_str(), arFileName[i].c_str());
								mir_snwprintf(tszFilePathBack, L"%s\\Backups\\%s", tszRoot, arFileName[i].c_str());
								MoveFile(tszFilePathDest, tszFilePathBack);
								MoveFile(arFilePath[i].c_str(), tszFilePathDest);
								mir_free(tszUtilRoot);
								break;
							}
						}
					}
					else { //reminder for not installed pack update
						if (Reminder && (UpdatesCount == 1) && (arFileType[0] == 1))
							g_plugin.setByte("Reminder", 2);
						mir_snwprintf(tszBuff, TranslateT("You have chosen not to install the pack update immediately.\nYou can install it manually from this location:\n\n%s"), arFilePath[0].c_str());
						Title = TranslateT("Pack Updater");
						Text = tszBuff;
						if (Popup_Enabled() && g_plugin.getByte("Popups2", DEFAULT_POPUP_ENABLED)) {
							Number = 2;
							show_popup(nullptr, Title, Text, Number, 0);
						}
						else if (g_plugin.getByte("Popups2M", DEFAULT_MESSAGE_ENABLED))
							MessageBox(nullptr, Text, Title, MB_ICONINFORMATION);
					}
				}
			}
			EndDialog(hDlg, IDOK);
			return TRUE;

			case IDCANCEL:
				SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hDlg, NULL, MODULENAME, "ConfirmWindow");
				EndDialog(hDlg, IDCANCEL);
				return TRUE;

			case IDC_INFO:
				int sel = ListView_GetSelectionMark(hwndList);
				vector<FILEINFO> &todo = *(vector<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
				Utils_OpenUrlW(todo[sel].tszInfoURL);
			}
			break;
		}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM, LPARAM)
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
