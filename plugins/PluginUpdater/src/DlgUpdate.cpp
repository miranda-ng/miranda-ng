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

#include "stdafx.h"

#define UM_ERROR (WM_USER+1)

static bool bShowDetails;
static HWND hwndDialog;
static HANDLE hCheckThread, hTimer;

static void SelectAll(HWND hDlg, bool bEnable)
{
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	for (int i=0; i < todo.getCount(); i++) {
		ListView_SetCheckState(hwndList, i, bEnable);
		db_set_b(NULL, DB_MODULE_FILES, StrToLower(_T2A(todo[i].tszOldName)), todo[i].bEnabled = bEnable);
	}
}

static void SetStringText(HWND hWnd, size_t i, wchar_t *ptszText)
{
	ListView_SetItemText(hWnd, i, 1, ptszText);
}

static void ApplyUpdates(void *param)
{
	Thread_SetName("PluginUpdater: ApplyUpdates");

	HWND hDlg = (HWND)param;
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (todo.getCount() == 0) {
		return;
	}

	// 1) If we need to escalate priviledges, launch a stub
	if (!PrepareEscalation()) {
		PostMessage(hDlg, WM_CLOSE, 0, 0);
		return;
	}

	AutoHandle pipe(hPipe);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);
	//create needed folders after escalating priviledges. Folders creates when we actually install updates
	wchar_t tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];
	mir_snwprintf(tszFileBack, L"%s\\Backups", g_tszRoot);
	SafeCreateDirectory(tszFileBack);
	mir_snwprintf(tszFileTemp, L"%s\\Temp", g_tszRoot);
	SafeCreateDirectory(tszFileTemp);

	// 2) Download all plugins
	HNETLIBCONN nlc = NULL;
	for (int i=0; i < todo.getCount(); i++) {
		ListView_EnsureVisible(hwndList, i, FALSE);
		if (!todo[i].bEnabled) {
			SetStringText(hwndList, i, TranslateT("Skipped."));
		}
		else if (todo[i].bDeleteOnly) {
			SetStringText(hwndList, i, TranslateT("Will be deleted!"));
		}
		else {
			// download update
			SetStringText(hwndList, i, TranslateT("Downloading..."));

			FILEURL *pFileUrl = &todo[i].File;
			if (!DownloadFile(pFileUrl, nlc)) {
				SetStringText(hwndList, i, TranslateT("Failed!"));

				// interrupt update as we require all components to be updated
				Netlib_CloseHandle(nlc);
				PostMessage(hDlg, UM_ERROR, 0, 0);
				Skin_PlaySound("updatefailed");
				return;
			}
			SetStringText(hwndList, i, TranslateT("Succeeded."));
		}
	}
	Netlib_CloseHandle(nlc);

	// 3) Unpack all zips
	VARSW tszMirandaPath(L"%miranda_path%");
	for (int i = 0; i < todo.getCount(); i++) {
		FILEINFO& p = todo[i];
		if (p.bEnabled) {
			if (p.bDeleteOnly) {
				// we need only to backup the old file
				wchar_t *ptszRelPath = p.tszNewName + wcslen(tszMirandaPath) + 1, tszBackFile[MAX_PATH];
				mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, ptszRelPath);
				BackupFile(p.tszNewName, tszBackFile);
			}
			else {
				// if file name differs, we also need to backup the old file here
				// otherwise it would be replaced by unzip
				if (_wcsicmp(p.tszOldName, p.tszNewName)) {
					wchar_t tszSrcPath[MAX_PATH], tszBackFile[MAX_PATH];
					mir_snwprintf(tszSrcPath, L"%s\\%s", tszMirandaPath, p.tszOldName);
					mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, p.tszOldName);
					BackupFile(tszSrcPath, tszBackFile);
				}

				if ( unzip(p.File.tszDiskPath, tszMirandaPath, tszFileBack,true))
					SafeDeleteFile(p.File.tszDiskPath);  // remove .zip after successful update
			}
		}
	}
	Skin_PlaySound("updatecompleted");

#if MIRANDA_VER < 0x0A00
	// 4) Change title of clist
	ptrW title(db_get_wsa(NULL, "CList", "TitleText"));
	if (!lstrcmpi(title, L"Miranda IM"))
		db_set_ws(NULL, "CList", "TitleText", L"Miranda NG");
#endif

	db_set_b(NULL, MODNAME, DB_SETTING_RESTART_COUNT, 5);

#if MIRANDA_VER >= 0x0A00
	if (opts.bBackup)
		CallService(MS_AB_BACKUP, 0, 0);
#endif

	if (opts.bChangePlatform) {
		wchar_t mirandaPath[MAX_PATH];
		GetModuleFileName(NULL, mirandaPath, _countof(mirandaPath));
		db_set_ws(NULL, MODNAME, "OldBin2", mirandaPath);

		db_unset(NULL, MODNAME, DB_SETTING_CHANGEPLATFORM);
	}
	else {
		ptrW oldbin(db_get_wsa(NULL, MODNAME, "OldBin2"));
		if (oldbin) {
			SafeDeleteFile(oldbin);
			db_unset(NULL, MODNAME, "OldBin2");
		}
	}

	if (opts.bForceRedownload) {
		opts.bForceRedownload = 0;
		db_unset(NULL, MODNAME, DB_SETTING_REDOWNLOAD);
	}

	// 5) Prepare Restart
	int rc = MessageBox(hDlg, TranslateT("Update complete. Press Yes to restart Miranda now or No to postpone a restart until the exit."), TranslateT("Plugin Updater"), MB_YESNO | MB_ICONQUESTION);
	PostMessage(hDlg, WM_CLOSE, 0, 0);
	if (rc == IDYES) {
#if MIRANDA_VER >= 0x0A00
		BOOL bRestartCurrentProfile = db_get_b(NULL, MODNAME, "RestartCurrentProfile", 1) ? 1 : 0;
		if (opts.bChangePlatform) {
			wchar_t mirstartpath[MAX_PATH];

#ifdef _WIN64
			mir_snwprintf(mirstartpath, L"%s\\miranda32.exe", tszMirandaPath);
#else
			mir_snwprintf(mirstartpath, L"%s\\miranda64.exe", tszMirandaPath);
#endif
			CallServiceSync(MS_SYSTEM_RESTART, bRestartCurrentProfile, (LPARAM)mirstartpath);
		}
		else
			CallServiceSync(MS_SYSTEM_RESTART, bRestartCurrentProfile);
#else
		CallFunctionAsync(RestartMe, 0);
#endif
	}
}

static void ResizeVert(HWND hDlg, int yy)
{
	RECT r = { 0, 0, 244, yy };
	MapDialogRect(hDlg, &r);
	r.bottom += GetSystemMetrics(SM_CYSMCAPTION);
	SetWindowPos(hDlg, 0, 0, 0, r.right, r.bottom, SWP_NOMOVE | SWP_NOZORDER);
}

static INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
#if MIRANDA_VER >= 0x0A00
		Window_SetIcon_IcoLib(hDlg, iconList[0].hIcolib);
#else
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIcon("check_update"));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIcon("check_update",1));
#endif
		{
			OSVERSIONINFO osver = { sizeof(osver) };
			if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
			{
				wchar_t szPath[MAX_PATH];
				GetModuleFileName(NULL, szPath, _countof(szPath));
				wchar_t *ext = wcsrchr(szPath, '.');
				if (ext != NULL)
					*ext = '\0';
				wcscat(szPath, L".test");
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
			ListView_DeleteAllItems(hwndList);
			///
			LVGROUP lvg;
			lvg.cbSize = sizeof(LVGROUP);
			lvg.mask = LVGF_HEADER | LVGF_GROUPID;

			lvg.pszHeader = TranslateT("Plugins");
			lvg.iGroupId = 1;
			ListView_InsertGroup(hwndList, 0, &lvg);

			lvg.pszHeader = TranslateT("Miranda NG Core");
			lvg.iGroupId = 2;
			ListView_InsertGroup(hwndList, 0, &lvg);

			lvg.pszHeader = TranslateT("Languages");
			lvg.iGroupId = 3;
			ListView_InsertGroup(hwndList, 0, &lvg);

			lvg.pszHeader = TranslateT("Icons");
			lvg.iGroupId = 4;
			ListView_InsertGroup(hwndList, 0, &lvg);

			ListView_EnableGroupView(hwndList, TRUE);

			bool enableOk = false;
			OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)lParam;
			for (int i = 0; i < todo.getCount(); ++i) {
				LVITEM lvI = {0};
				lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_GROUPID | LVIF_NORECOMPUTE;
				lvI.iGroupId = (wcsstr(todo[i].tszOldName, L"Plugins") != NULL) ? 1 :
					((wcsstr(todo[i].tszOldName, L"Languages") != NULL) ? 3 :
						((wcsstr(todo[i].tszOldName, L"Icons") != NULL) ? 4 : 2));
				lvI.iSubItem = 0;
				lvI.lParam = (LPARAM)&todo[i];
				lvI.pszText = todo[i].tszOldName;
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);

				ListView_SetCheckState(hwndList, lvI.iItem, todo[i].bEnabled);
				if (todo[i].bEnabled)
					enableOk = true;

				SetStringText(hwndList,i,todo[i].bDeleteOnly ? TranslateT("Deprecated!") : TranslateT("Update found!"));
			}
			if(enableOk)
				EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
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
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						LVITEM lvI = {0};
						lvI.iItem = nmlv->iItem;
						lvI.iSubItem = 0;
						lvI.mask = LVIF_PARAM;
						ListView_GetItem(hwndList, &lvI);

						FILEINFO *p = (FILEINFO*)lvI.lParam;
						db_set_b(NULL, DB_MODULE_FILES, StrToLower(_T2A(p->tszOldName)), p->bEnabled = ListView_GetCheckState(hwndList, nmlv->iItem));

						// Toggle the Download button
						bool enableOk = false;
						OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
						for (int i=0; i < todo.getCount(); ++i) {
							if (todo[i].bEnabled) {
								enableOk = true;
								break;
							}
						}
						EnableWindow(GetDlgItem(hDlg, IDOK), enableOk ? TRUE : FALSE);
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
				if (bShowDetails) {
					ResizeVert(hDlg, 60);
					SetDlgItemText(hDlg, IDC_DETAILS, TranslateT("Details >>"));
					bShowDetails = false;
				}
				else {
					ResizeVert(hDlg, 242);
					SetDlgItemText(hDlg, IDC_DETAILS, TranslateT("<< Details"));
					bShowDetails = true;
				}
				break;

			case IDC_SELALL:
				SelectAll(hDlg, true);
				break;

			case IDC_SELNONE:
				SelectAll(hDlg, false);
				break;

			case IDCANCEL:
				DestroyWindow(hDlg);
				return TRUE;
			}
		}
		break;

	case UM_ERROR:
		MessageBox(hDlg, TranslateT("Update failed! One of the components wasn't downloaded correctly. Try it again later."), TranslateT("Plugin Updater"), MB_OK | MB_ICONERROR);
		DestroyWindow(hDlg);
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hDlg);
		Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ConfirmWindow");
		hwndDialog = NULL;
		delete (OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
#if MIRANDA_VER >= 0x0A00
		db_set_dw(NULL, MODNAME, DB_SETTING_LAST_UPDATE, time(NULL));
#endif
		mir_forkthread(InitTimer, 0);
		break;
	}

	return FALSE;
}

static void DlgUpdateSilent(void *param)
{
	Thread_SetName("PluginUpdater: DlgUpdateSilent");

	OBJLIST<FILEINFO> &UpdateFiles = *(OBJLIST<FILEINFO> *)param;
	if (UpdateFiles.getCount() == 0) {
		delete &UpdateFiles;
		return;
	}

	// 1) If we need to escalate priviledges, launch a stub
	if (!PrepareEscalation()) {
		delete &UpdateFiles;
		return;
	}

	AutoHandle pipe(hPipe);
	//create needed folders after escalating priviledges. Folders creates when we actually install updates
	wchar_t tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_snwprintf(tszFileBack, L"%s\\Backups", g_tszRoot);
	SafeCreateDirectory(tszFileBack);

	mir_snwprintf(tszFileTemp, L"%s\\Temp", g_tszRoot);
	SafeCreateDirectory(tszFileTemp);

	// 2) Download all plugins
	HNETLIBCONN nlc = NULL;
	// Count all updates that have been enabled
	int count = 0;
	for (int i = 0; i < UpdateFiles.getCount(); i++) {
		if (UpdateFiles[i].bEnabled && !UpdateFiles[i].bDeleteOnly) {
			// download update
			FILEURL *pFileUrl = &UpdateFiles[i].File;
			if (!DownloadFile(pFileUrl, nlc)) {
				// interrupt update as we require all components to be updated
				Netlib_CloseHandle(nlc);
				Skin_PlaySound("updatefailed");
				delete &UpdateFiles;
				return;
			}
			count++;
		}

	}
	Netlib_CloseHandle(nlc);

	// All available updates have been disabled
	if (count == 0) {
		delete &UpdateFiles;
		return;
	}

	// 3) Unpack all zips
	VARSW tszMirandaPath(L"%miranda_path%");
	for (int i = 0; i < UpdateFiles.getCount(); i++) {
		FILEINFO& p = UpdateFiles[i];
		if (p.bEnabled) {
			if (p.bDeleteOnly) {
				// we need only to backup the old file
				wchar_t *ptszRelPath = p.tszNewName + wcslen(tszMirandaPath) + 1, tszBackFile[MAX_PATH];
				mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, ptszRelPath);
				BackupFile(p.tszNewName, tszBackFile);
			}
			else {
				// if file name differs, we also need to backup the old file here
				// otherwise it would be replaced by unzip
				if (_wcsicmp(p.tszOldName, p.tszNewName)) {
					wchar_t tszSrcPath[MAX_PATH], tszBackFile[MAX_PATH];
					mir_snwprintf(tszSrcPath, L"%s\\%s", tszMirandaPath, p.tszOldName);
					mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, p.tszOldName);
					BackupFile(tszSrcPath, tszBackFile);
				}

				// remove .zip after successful update
				if (unzip(p.File.tszDiskPath, tszMirandaPath, tszFileBack, true))
					SafeDeleteFile(p.File.tszDiskPath);
			}
		}
	}
	delete &UpdateFiles;
	Skin_PlaySound("updatecompleted");

#if MIRANDA_VER < 0x0A00
	// 4) Change title of clist
	ptrW title = db_get_wsa(NULL, "CList", "TitleText");
	if (!_wcsicmp(title, L"Miranda IM"))
		db_set_ws(NULL, "CList", "TitleText", L"Miranda NG");
#endif

	opts.bForceRedownload = false;
	db_unset(NULL, MODNAME, DB_SETTING_REDOWNLOAD);

	opts.bChangePlatform = false;
	db_unset(NULL, MODNAME, DB_SETTING_CHANGEPLATFORM);

	db_set_b(NULL, MODNAME, DB_SETTING_RESTART_COUNT, 5);
	db_set_b(NULL, MODNAME, DB_SETTING_NEED_RESTART, 1);

	// 5) Prepare Restart
	wchar_t tszTitle[100];
	mir_snwprintf(tszTitle, TranslateT("%d component(s) was updated"), count);

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
		ShowPopup(tszTitle,TranslateT("You need to restart your Miranda to apply installed updates."),POPUP_TYPE_MSG);
	else {
		if (Clist_TrayNotifyW(MODULEA, tszTitle, TranslateT("You need to restart your Miranda to apply installed updates."), NIIF_INFO, 30000)) {
			// Error, let's try to show MessageBox as last way to inform user about successful update
			wchar_t tszText[200];
			mir_snwprintf(tszText, L"%s\n\n%s", TranslateT("You need to restart your Miranda to apply installed updates."), TranslateT("Would you like to restart it now?"));

			if (MessageBox(NULL, tszText, tszTitle, MB_ICONINFORMATION | MB_YESNO) == IDYES)
#if MIRANDA_VER >= 0x0A00
				CallServiceSync(MS_SYSTEM_RESTART, db_get_b(NULL, MODNAME, "RestartCurrentProfile", 1) ? 1 : 0, 0);
#else
				CallFunctionAsync(RestartMe, 0);
#endif
		}
	}
}

static void __stdcall LaunchDialog(void *param)
{
	if (opts.bSilentMode && opts.bSilent)
		mir_forkthread(DlgUpdateSilent, param);
	else
		hwndDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_UPDATE), GetDesktopWindow(), DlgUpdate, (LPARAM)param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// building file list in the separate thread

struct
{
	wchar_t *oldName, *newName;
}
static renameTable[] =
{
	{ L"svc_dbepp.dll",                  L"Plugins\\dbeditorpp.dll" },
	{ L"svc_crshdmp.dll",                L"Plugins\\crashdumper.dll" },
	{ L"crashdmp.dll",                   L"Plugins\\crashdumper.dll" },
	{ L"crashrpt.dll",                   L"Plugins\\crashdumper.dll" },
	{ L"attache.dll",                    L"Plugins\\crashdumper.dll" },
	{ L"svc_vi.dll",                     L"Plugins\\crashdumper.dll" },
	{ L"crashrpt.dll",                   L"Plugins\\crashdumper.dll" },
	{ L"versioninfo.dll",                L"Plugins\\crashdumper.dll" },
	{ L"advsplashscreen.dll",            L"Plugins\\splashscreen.dll" },
	{ L"import_sa.dll",                  L"Plugins\\import.dll" },
	{ L"newnr.dll",                      L"Plugins\\notesreminders.dll" },
	{ L"dbtool.exe",                     L"Plugins\\dbchecker.dll" },
	{ L"dbtool_sa.exe",                  L"Plugins\\dbchecker.dll" },
	{ L"clist_mw.dll",                   L"Plugins\\clist_nicer.dll" },
	{ L"bclist.dll",                     L"Plugins\\clist_blind.dll" },
	{ L"otr.dll",                        L"Plugins\\mirotr.dll" },
	{ L"ttnotify.dll",                   L"Plugins\\tooltipnotify.dll" },
	{ L"newstatusnotify.dll",            L"Plugins\\newxstatusnotify.dll" },
	{ L"rss.dll",                        L"Plugins\\newsaggregator.dll" },
	{ L"dbx_3x.dll",                     L"Plugins\\dbx_mmap.dll" },
	{ L"actman30.dll",                   L"Plugins\\actman.dll" },
	{ L"skype.dll",                      L"Plugins\\skypeweb.dll" },
	{ L"skypeclassic.dll",               L"Plugins\\skypeweb.dll" },
	{ L"historysweeper.dll",             L"Plugins\\historysweeperlight.dll" },
	{ L"advancedautoaway.dll",           L"Plugins\\statusmanager.dll" },
	{ L"keepstatus.dll",                 L"Plugins\\statusmanager.dll" },
	{ L"startupstatus.dll",              L"Plugins\\statusmanager.dll" },

#if MIRANDA_VER >= 0x0A00
	{ L"dbx_mmap_sa.dll",                L"Plugins\\dbx_mmap.dll" },
	{ L"dbx_tree.dll",                   L"Plugins\\dbx_mmap.dll" },
	{ L"rc4.dll",                        NULL },
	{ L"athena.dll",                     NULL },
	{ L"skypekit.exe",                   NULL },
	{ L"mir_app.dll",                    NULL },
	{ L"mir_core.dll",                   NULL },
	{ L"zlib.dll",                       NULL },
#endif

	{ L"proto_newsaggr.dll",             L"Icons\\proto_newsaggregator.dll" },
	{ L"clienticons_*.dll",              L"Icons\\fp_icons.dll" },
	{ L"fp_*.dll",                       L"Icons\\fp_icons.dll" },

	{ L"langpack_*.txt",                 L"Languages\\*" },

	{ L"pcre16.dll",                     NULL },
	{ L"clist_classic.dll",              NULL },
	{ L"chat.dll",                       NULL },
	{ L"srmm.dll",                       NULL },
	{ L"stdchat.dll",                    NULL },
	{ L"stdurl.dll",                     NULL },
	{ L"extraicons.dll",                 NULL },
	{ L"firstrun.dll",                   NULL },
	{ L"flashavatars.dll",               NULL },
	{ L"gender.dll",                     NULL },
	{ L"gtalkext.dll",                   NULL },
	{ L"langman.dll",                    NULL },
	{ L"metacontacts.dll",               NULL },
	{ L"modernopt.dll",                  NULL },
	{ L"msvcp100.dll",                   NULL },
	{ L"msvcr100.dll",                   NULL },
	{ L"xfire.dll",                      NULL },
	{ L"yahoo.dll",                      NULL },
	{ L"yahoogroups.dll",                NULL },
	{ L"WART-*.exe",                     NULL },
};

// Checks if file needs to be renamed and copies it in pNewName
// Returns true if smth. was copied
static bool CheckFileRename(const wchar_t *ptszOldName, wchar_t *pNewName)
{
	for (int i = 0; i < _countof(renameTable); i++) {
		if (wildcmpiw(ptszOldName, renameTable[i].oldName)) {
			wchar_t *ptszDest = renameTable[i].newName;
			if (ptszDest == NULL)
				*pNewName = 0;
			else {
				wcsncpy_s(pNewName, MAX_PATH, ptszDest, _TRUNCATE);
				size_t cbLen = wcslen(ptszDest) - 1;
				if (pNewName[cbLen] == '*')
					wcsncpy_s(pNewName + cbLen, MAX_PATH - cbLen, ptszOldName, _TRUNCATE);
			}
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// We only update ".dll", ".exe" and ".ico"
static bool isValidExtension(const wchar_t *ptszFileName)
{
	const wchar_t *pExt = wcsrchr(ptszFileName, '.');

	return (pExt != NULL) && (!_wcsicmp(pExt, L".dll") || !_wcsicmp(pExt, L".exe") || !_wcsicmp(pExt, L".txt"));
}

// We only scan subfolders "Plugins", "Icons", "Languages", "Libs", "Core"
static bool isValidDirectory(const wchar_t *ptszDirName)
{
	return !_wcsicmp(ptszDirName, L"Plugins") || !_wcsicmp(ptszDirName, L"Icons") || !_wcsicmp(ptszDirName, L"Languages") || !_wcsicmp(ptszDirName, L"Libs") || !_wcsicmp(ptszDirName, L"Core");
}

// Scans folders recursively
static int ScanFolder(const wchar_t *tszFolder, size_t cbBaseLen, const wchar_t *tszBaseUrl, SERVLIST& hashes, OBJLIST<FILEINFO> *UpdateFiles, int level = 0)
{
	wchar_t tszBuf[MAX_PATH];
	mir_snwprintf(tszBuf, L"%s\\*", tszFolder);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(tszBuf, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	Netlib_LogfW(hNetlibUser,L"Scanning folder %s", tszFolder);

	int count = 0;
	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Scan recursively all subfolders
			if (isValidDirectory(ffd.cFileName)) {
				mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
				count += ScanFolder(tszBuf, cbBaseLen, tszBaseUrl, hashes, UpdateFiles, level + 1);
			}
		}
		else if (isValidExtension(ffd.cFileName)) {
			// calculate the current file's relative name and store it into tszNewName
			wchar_t tszNewName[MAX_PATH];
			if (CheckFileRename(ffd.cFileName, tszNewName)) {
				Netlib_LogfW(hNetlibUser, L"File %s will be renamed to %s.", ffd.cFileName, tszNewName);
				// Yes, we need the old file name, because this will be hashed later
				mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
			}
			else {
				if (level == 0) {
					// Rename Miranda*.exe
					wcsncpy_s(tszNewName, opts.bChangePlatform && !mir_wstrcmpi(ffd.cFileName, OLD_FILENAME) ? NEW_FILENAME : ffd.cFileName, _TRUNCATE);
					mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, tszNewName);
				}
				else {
					mir_snwprintf(tszNewName, L"%s\\%s", tszFolder + cbBaseLen, ffd.cFileName);
					mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
				}
			}

			wchar_t *ptszUrl;
			int MyCRC;

			bool bDeleteOnly = (tszNewName[0] == 0);
			// this file is not marked for deletion
			if (!bDeleteOnly) {
				wchar_t *pName = tszNewName;
				ServListEntry *item = hashes.find((ServListEntry*)&pName);
				// Not in list? Check for trailing 'W' or 'w'
				if (item == NULL) {
					wchar_t *p = wcsrchr(tszNewName, '.');
					if (p[-1] != 'w' && p[-1] != 'W') {
						Netlib_LogfW(hNetlibUser, L"File %s: Not found on server, skipping", ffd.cFileName);
						continue;
					}

					// remove trailing w or W and try again
					int iPos = int(p - tszNewName) - 1;
					strdelw(p - 1, 1);
					if ((item = hashes.find((ServListEntry*)&pName)) == NULL) {
						Netlib_LogfW(hNetlibUser, L"File %s: Not found on server, skipping", ffd.cFileName);
						continue;
					}

					strdelw(tszNewName + iPos, 1);
				}

#if MIRANDA_VER >= 0x0A00
				// No need to hash a file if we are forcing a redownload anyway
				if (!opts.bForceRedownload) {
					// try to hash the file
					char szMyHash[33];
					__try {
						CalculateModuleHash(tszBuf, szMyHash);
						// hashes are the same, skipping
						if (strcmp(szMyHash, item->m_szHash) == 0) {
							Netlib_LogfW(hNetlibUser, L"File %s: Already up-to-date, skipping", ffd.cFileName);
							continue;
						}
						else
							Netlib_LogfW(hNetlibUser, L"File %s: Update available", ffd.cFileName);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						// smth went wrong, reload a file from scratch
					}
				}
				else
					Netlib_LogfW(hNetlibUser, L"File %s: Forcing redownload", ffd.cFileName);
#endif

				ptszUrl = item->m_name;
				MyCRC = item->m_crc;
			}
			else {
				// file was marked for deletion, add it to the list anyway
				Netlib_LogfW(hNetlibUser, L"File %s: Marked for deletion", ffd.cFileName);
				ptszUrl = L"";
				MyCRC = 0;
			}

			// Yeah, we've got new version.
			FILEINFO *FileInfo = new FILEINFO;
			// copy the relative old name
			wcsncpy(FileInfo->tszOldName, tszBuf + cbBaseLen, _countof(FileInfo->tszOldName));
			FileInfo->bDeleteOnly = bDeleteOnly;
			if (FileInfo->bDeleteOnly) {
				// save the full old name for deletion
				wcsncpy(FileInfo->tszNewName, tszBuf, _countof(FileInfo->tszNewName));
			}
			else {
				wcsncpy(FileInfo->tszNewName, ptszUrl, _countof(FileInfo->tszNewName));
			}

			wcsncpy(tszBuf, ptszUrl, _countof(tszBuf));
			wchar_t *p = wcsrchr(tszBuf, '.');
			if (p) *p = 0;
			p = wcsrchr(tszBuf, '\\');
			p = (p) ? p + 1 : tszBuf;
			_wcslwr(p);

			mir_snwprintf(FileInfo->File.tszDiskPath, L"%s\\Temp\\%s.zip", g_tszRoot, p);
			mir_snwprintf(FileInfo->File.tszDownloadURL, L"%s/%s.zip", tszBaseUrl, tszBuf);
			for (p = wcschr(FileInfo->File.tszDownloadURL, '\\'); p != 0; p = wcschr(p, '\\'))
				*p++ = '/';

			// remember whether the user has decided not to update this component with this particular new version
			FileInfo->bEnabled = db_get_b(NULL, DB_MODULE_FILES, StrToLower(_T2A(FileInfo->tszOldName)), 1);

			FileInfo->File.CRCsum = MyCRC;
			UpdateFiles->insert(FileInfo);

			// If we are in the silent mode, only count enabled plugins, otherwise count all
			if (!opts.bSilent || FileInfo->bEnabled)
				count++;
		}
	}
		while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return count;
}

// Thread checks for updates
static void CheckUpdates(void *)
{
	Netlib_LogfW(hNetlibUser, L"Checking for updates");
	Thread_SetName("PluginUpdater: CheckUpdates");

	wchar_t tszTempPath[MAX_PATH];
	DWORD dwLen = GetTempPath(_countof(tszTempPath), tszTempPath);
	if (tszTempPath[dwLen - 1] == '\\')
		tszTempPath[dwLen - 1] = 0;

	if (!opts.bSilent)
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("Checking for new updates..."), POPUP_TYPE_INFO);

	ptrW updateUrl(GetDefaultUrl()), baseUrl;
	SERVLIST hashes(50, CompareHashes);
	bool success = ParseHashes(updateUrl, baseUrl, hashes);
	if (success) {
		FILELIST *UpdateFiles = new FILELIST(20);
		VARSW dirname(L"%miranda_path%");
		int count = ScanFolder(dirname, lstrlen(dirname) + 1, baseUrl, hashes, UpdateFiles);

		// Show dialog
		if (count == 0) {
			if (!opts.bSilent)
				ShowPopup(TranslateT("Plugin Updater"), TranslateT("No updates found."), POPUP_TYPE_INFO);
			delete UpdateFiles;
		}
		else CallFunctionAsync(LaunchDialog, UpdateFiles);
	}

	mir_forkthread(InitTimer, (success ? 0 : (void*)2));

	hashes.destroy();
	hCheckThread = NULL;
}

static void DoCheck(bool bSilent = true)
{
	if (hCheckThread)
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("Update checking already started!"), POPUP_TYPE_INFO);
	else if (hwndDialog) {
		ShowWindow(hwndDialog, SW_SHOW);
		SetForegroundWindow(hwndDialog);
		SetFocus(hwndDialog);
	}
	else {
		opts.bSilent = bSilent;
#if MIRANDA_VER >= 0x0A00
		db_set_dw(NULL, MODNAME, DB_SETTING_LAST_UPDATE, time(NULL));
#endif
		hCheckThread = mir_forkthread(CheckUpdates, 0);
	}
}

void UninitCheck()
{
	CancelWaitableTimer(hTimer);
	CloseHandle(hTimer);
	if (hwndDialog != NULL)
		DestroyWindow(hwndDialog);
}

// menu item command
static INT_PTR MenuCommand(WPARAM, LPARAM)
{
	Netlib_LogfW(hNetlibUser, L"Update started manually!");
	DoCheck(false);
	return 0;
}

void InitCheck()
{
	CreateServiceFunction(MS_PU_CHECKUPDATES, MenuCommand);
}

void UnloadCheck()
{
	if (hCheckThread)
		hCheckThread = NULL;
}

void CheckUpdateOnStartup()
{
	if (opts.bUpdateOnStartup) {
		if (opts.bOnlyOnceADay) {
			time_t now = time(NULL),
				was = db_get_dw(NULL, MODNAME, DB_SETTING_LAST_UPDATE, 0);

			if ((now - was) < 86400)
				return;
		}
		Netlib_LogfW(hNetlibUser, L"Update on startup started!");
		DoCheck();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void CALLBACK TimerAPCProc(void *, DWORD, DWORD)
{
	DoCheck();
}

static LONGLONG PeriodToMilliseconds(const int period, BYTE &periodMeasure)
{
	LONGLONG result = period * 1000LL;
	switch(periodMeasure) {
	case 1:
		// day
		result *= 60 * 60 * 24;
		break;

	default:
		// hour
		if (periodMeasure != 0)
			periodMeasure = 0;
		result *= 60 * 60;
	}
	return result;
}

void InitTimer(void *type)
{
	if (!opts.bUpdateOnPeriod)
		return;
	Thread_SetName("PluginUpdater: InitTimer");

	LONGLONG interval;

	switch ((INT_PTR)type) {
	case 0: // default, plan next check relative to last check
		{
			time_t now = time(NULL);
			time_t was = db_get_dw(NULL, MODNAME, DB_SETTING_LAST_UPDATE, 0);

			interval = PeriodToMilliseconds(opts.Period, opts.bPeriodMeasure);
			interval -= (now - was) * 1000;
			if (interval <= 0)
				interval = 1000; // no last update or too far in the past -> do it now
		}
		break;

	case 2: // failed last check, check again in two hours
		interval = 1000 * 60 * 60 * 2;
		break;

	default: // options changed, use set interval from now
		interval = PeriodToMilliseconds(opts.Period, opts.bPeriodMeasure);
	}

	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	LARGE_INTEGER li;
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	li.QuadPart += interval * 10000LL;
	SetWaitableTimer(hTimer, &li, 0, TimerAPCProc, NULL, 0);

	// Wait in an alertable state for the timer to go off.
	SleepEx(INFINITE, TRUE);
}

void CreateTimer() {
	hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	mir_forkthread(InitTimer, 0);
}