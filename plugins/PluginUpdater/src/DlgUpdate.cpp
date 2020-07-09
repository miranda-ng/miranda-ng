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

static bool bShowDetails;
static HWND hwndDialog;
static DWORD dwCheckThreadId;
static HANDLE hTimer;

class CUpdateDLg : public CDlgBase
{
	DWORD dwThreadId = 0;
	CCtrlButton btnDetails, btnSelAll, btnSelNone, btnOk;
	CCtrlListView m_list;
	OBJLIST<FILEINFO> *m_todo;

	static void ApplyUpdates(CUpdateDLg *pDlg)
	{
		Thread_SetName("PluginUpdater: ApplyUpdates");

		// make a local copy not to crash if a window was closed
		OBJLIST<FILEINFO> todo(*pDlg->m_todo);

		// 1) If we need to escalate priviledges, launch a stub
		if (!PrepareEscalation()) {
			pDlg->Close();
			return;
		}

		VARSW tszMirandaPath(L"%miranda_path%");
		{
			ThreadWatch threadId(pDlg->dwThreadId);
			AutoHandle pipe(hPipe);
			//create needed folders after escalating priviledges. Folders creates when we actually install updates
			wchar_t tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];
			mir_snwprintf(tszFileBack, L"%s\\Backups", g_tszRoot);
			SafeCreateDirectory(tszFileBack);
			mir_snwprintf(tszFileTemp, L"%s\\Temp", g_tszRoot);
			SafeCreateDirectory(tszFileTemp);

			// 2) Download all plugins
			HNETLIBCONN nlc = nullptr;
			for (int i = 0; i < todo.getCount(); i++) {
				pDlg->m_list.EnsureVisible(i, false);
				if (!todo[i].bEnabled) {
					pDlg->m_list.SetItemText(i, 1, TranslateT("Skipped."));
				}
				else if (todo[i].bDeleteOnly) {
					pDlg->m_list.SetItemText(i, 1, TranslateT("Will be deleted!"));
				}
				else {
					// download update
					pDlg->m_list.SetItemText(i, 1, TranslateT("Downloading..."));

					FILEURL *pFileUrl = &todo[i].File;
					if (!DownloadFile(pFileUrl, nlc)) {
						pDlg->m_list.SetItemText(i, 1, TranslateT("Failed!"));

						// interrupt update as we require all components to be updated
						Netlib_CloseHandle(nlc);
						pDlg->ShowError();
						Skin_PlaySound("updatefailed");
						return;
					}
					pDlg->m_list.SetItemText(i, 1, TranslateT("Succeeded."));
				}
			}
			Netlib_CloseHandle(nlc);

			// 3) Unpack all zips
			for (auto &it : todo) {
				if (it->bEnabled) {
					if (it->bDeleteOnly) {
						// we need only to backup the old file
						wchar_t *ptszRelPath = it->tszNewName + wcslen(tszMirandaPath) + 1, tszBackFile[MAX_PATH];
						mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, ptszRelPath);
						BackupFile(it->tszNewName, tszBackFile);
					}
					else {
						// if file name differs, we also need to backup the old file here
						// otherwise it would be replaced by unzip
						if (_wcsicmp(it->tszOldName, it->tszNewName)) {
							wchar_t tszSrcPath[MAX_PATH], tszBackFile[MAX_PATH];
							mir_snwprintf(tszSrcPath, L"%s\\%s", tszMirandaPath.get(), it->tszOldName);
							mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, it->tszOldName);
							BackupFile(tszSrcPath, tszBackFile);
						}

						if (unzip(it->File.tszDiskPath, tszMirandaPath, tszFileBack, true))
							SafeDeleteFile(it->File.tszDiskPath);  // remove .zip after successful update
					}
				}
			}
			Skin_PlaySound("updatecompleted");

			g_plugin.setByte(DB_SETTING_RESTART_COUNT, 5);

			if (g_plugin.bBackup)
				CallService(MS_AB_BACKUP, 0, 0);

			if (g_plugin.bChangePlatform) {
				wchar_t mirandaPath[MAX_PATH];
				GetModuleFileName(nullptr, mirandaPath, _countof(mirandaPath));
				g_plugin.setWString("OldBin2", mirandaPath);

				g_plugin.delSetting(DB_SETTING_CHANGEPLATFORM);
			}
			else {
				ptrW oldbin(g_plugin.getWStringA("OldBin2"));
				if (oldbin) {
					SafeDeleteFile(oldbin);
					g_plugin.delSetting("OldBin2");
				}
			}
		}

		// 5) Prepare Restart
		if (!g_plugin.bAutoRestart)
			if (IDYES != MessageBox(pDlg->GetHwnd(), TranslateT("Update complete. Press Yes to restart Miranda now or No to postpone a restart until the exit."), TranslateT("Plugin Updater"), MB_YESNO | MB_ICONQUESTION)) {
				pDlg->Close();
				return;
			}

		pDlg->Close();
		BOOL bRestartCurrentProfile = g_plugin.getBool("RestartCurrentProfile", true);
		if (g_plugin.bChangePlatform) {
			wchar_t mirstartpath[MAX_PATH];
#ifdef _WIN64
			mir_snwprintf(mirstartpath, L"%s\\miranda32.exe", tszMirandaPath.get());
#else
			mir_snwprintf(mirstartpath, L"%s\\miranda64.exe", tszMirandaPath.get());
#endif
			CallServiceSync(MS_SYSTEM_RESTART, bRestartCurrentProfile, (LPARAM)mirstartpath);
		}
		else CallServiceSync(MS_SYSTEM_RESTART, bRestartCurrentProfile);
	}

	void ResizeVert(int yy)
	{
		RECT r = { 0, 0, 244, yy };
		MapDialogRect(m_hwnd, &r);
		r.bottom += GetSystemMetrics(SM_CYSMCAPTION);
		SetWindowPos(m_hwnd, nullptr, 0, 0, r.right, r.bottom, SWP_NOMOVE | SWP_NOZORDER);
	}

	void SelectAll(bool bEnable)
	{
		for (auto &it : *m_todo) {
			m_list.SetCheckState(m_todo->indexOf(&it), bEnable);

			CMStringA szSetting(it->tszOldName);
			db_set_b(0, DB_MODULE_FILES, StrToLower(szSetting.GetBuffer()), it->bEnabled = bEnable);
		}
	}

public:
	CUpdateDLg(OBJLIST<FILEINFO> *param) :
		CDlgBase(g_plugin, IDD_UPDATE),
		m_list(this, IDC_LIST_UPDATES),
		btnOk(this, IDOK),
		btnSelAll(this, IDC_SELALL),
		btnSelNone(this, IDC_SELNONE),
		btnDetails(this, IDC_DETAILS),
		m_todo(param)
	{
		m_list.OnItemChanged = Callback(this, &CUpdateDLg::onItemCHanged_List);

		btnSelAll.OnClick = Callback(this, &CUpdateDLg::onClick_SelAll);
		btnSelNone.OnClick = Callback(this, &CUpdateDLg::onClick_SelNone);
		btnDetails.OnClick = Callback(this, &CUpdateDLg::onClick_Details);
	}

	bool OnInitDialog() override
	{
		hwndDialog = m_hwnd;
		m_list.SendMsg(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		Window_SetIcon_IcoLib(m_hwnd, iconList[0].hIcolib);

		if (IsWinVerVistaPlus()) {
			wchar_t szPath[MAX_PATH];
			GetModuleFileName(nullptr, szPath, _countof(szPath));
			wchar_t *ext = wcsrchr(szPath, '.');
			if (ext != nullptr)
				*ext = '\0';
			wcscat(szPath, L".test");
			HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
				// Running Windows Vista or later (major version >= 6).
				Button_SetElevationRequiredState(btnOk.GetHwnd(), !IsProcessElevated());
			else {
				CloseHandle(hFile);
				DeleteFile(szPath);
			}
		}

		// Initialize the LVCOLUMN structure.
		// The mask specifies that the format, width, text, and
		// subitem members of the structure are valid.
		LVCOLUMN lvc = {};
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		lvc.fmt = LVCFMT_LEFT;

		lvc.iSubItem = 0;
		lvc.pszText = TranslateT("Component Name");
		lvc.cx = 220; // width of column in pixels
		m_list.InsertColumn(0, &lvc);

		lvc.iSubItem = 1;
		lvc.pszText = TranslateT("State");
		lvc.cx = 120 - GetSystemMetrics(SM_CXVSCROLL); // width of column in pixels
		m_list.InsertColumn(1, &lvc);

		// enumerate plugins, fill in list
		m_list.DeleteAllItems();

		LVGROUP lvg;
		lvg.cbSize = sizeof(LVGROUP);
		lvg.mask = LVGF_HEADER | LVGF_GROUPID;

		lvg.pszHeader = TranslateT("Plugins");
		lvg.iGroupId = 1;
		m_list.InsertGroup(0, &lvg);

		lvg.pszHeader = TranslateT("Miranda NG Core");
		lvg.iGroupId = 2;
		m_list.InsertGroup(0, &lvg);

		lvg.pszHeader = TranslateT("Languages");
		lvg.iGroupId = 3;
		m_list.InsertGroup(0, &lvg);

		lvg.pszHeader = TranslateT("Icons");
		lvg.iGroupId = 4;
		m_list.InsertGroup(0, &lvg);

		m_list.EnableGroupView(true);

		bool enableOk = false;
		for (auto &it : *m_todo) {
			LVITEM lvI = { 0 };
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_GROUPID | LVIF_NORECOMPUTE;
			lvI.iGroupId = (wcsstr(it->tszOldName, L"Plugins") != nullptr) ? 1 :
				((wcsstr(it->tszOldName, L"Languages") != nullptr) ? 3 :
				((wcsstr(it->tszOldName, L"Icons") != nullptr) ? 4 : 2));
			lvI.iSubItem = 0;
			lvI.lParam = (LPARAM)it;
			lvI.pszText = it->tszOldName;
			lvI.iItem = m_todo->indexOf(&it);
			m_list.InsertItem(&lvI);

			m_list.SetCheckState(lvI.iItem, it->bEnabled);
			if (it->bEnabled)
				enableOk = true;

			m_list.SetItemText(lvI.iItem, 1, it->bDeleteOnly ? TranslateT("Deprecated!") : TranslateT("Update found!"));
		}

		if (enableOk)
			btnOk.Enable();

		bShowDetails = false;
		ResizeVert(60);

		Utils_RestoreWindowPositionNoSize(m_hwnd, 0, MODULENAME, "ConfirmWindow");
		return true;
	}

	bool OnApply() override
	{
		btnOk.Disable();
		btnSelAll.Disable();
		btnSelNone.Disable();

		if (m_todo->getCount() > 0)
			mir_forkThread<CUpdateDLg>(ApplyUpdates, this);
		return false;
	}

	bool OnClose() override
	{
		return dwThreadId == 0; // allow to close window only when thread is inactive
	}

	void OnDestroy() override
	{
		Window_FreeIcon_IcoLib(m_hwnd);
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ConfirmWindow");
		hwndDialog = nullptr;
		delete m_todo;
	}

	void onItemCHanged_List(CCtrlListView::TEventInfo *ev)
	{
		NMLISTVIEW *nmlv = ev->nmlv;
		if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
			LVITEM lvI = {};
			lvI.iItem = nmlv->iItem;
			lvI.mask = LVIF_PARAM;
			m_list.GetItem(&lvI);

			FILEINFO *p = (FILEINFO *)lvI.lParam;
			CMStringA szSetting(p->tszOldName);
			db_set_b(0, DB_MODULE_FILES, StrToLower(szSetting.GetBuffer()), p->bEnabled = m_list.GetCheckState(nmlv->iItem));

			// Toggle the Download button
			bool enableOk = false;
			for (auto &it : *m_todo) {
				if (it->bEnabled) {
					enableOk = true;
					break;
				}
			}
			btnOk.Enable(enableOk);
		}
	}

	void onClick_Details(CCtrlButton *)
	{
		if (bShowDetails) {
			ResizeVert(60);
			btnDetails.SetText(TranslateT("Details >>"));
			bShowDetails = false;
		}
		else {
			ResizeVert(242);
			btnDetails.SetText(TranslateT("<< Details"));
			bShowDetails = true;
		}
	}

	void onClick_SelAll(CCtrlButton *)
	{
		SelectAll(true);
	}

	void onClick_SelNone(CCtrlButton *)
	{
		SelectAll(false);
	}

	void ShowError()
	{
		MessageBox(m_hwnd, TranslateT("Update failed! One of the components wasn't downloaded correctly. Try it again later."), TranslateT("Plugin Updater"), MB_OK | MB_ICONERROR);
		Close();
	}
};

static void __stdcall LaunchDialog(void *param)
{
	auto *UpdateFiles = (OBJLIST<FILEINFO> *)param;
	auto *pDlg = new CUpdateDLg(UpdateFiles);
	pDlg->SetParent(GetDesktopWindow());
	pDlg->Create();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Silent update procedure

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
	HNETLIBCONN nlc = nullptr;
	// Count all updates that have been enabled
	int count = 0;
	for (auto &it : UpdateFiles) {
		if (it->bDeleteOnly)
			count++;
		else if (it->bEnabled) {
			// download update
			if (!DownloadFile(&it->File, nlc)) {
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
	for (auto &it : UpdateFiles) {
		if (it->bEnabled) {
			if (it->bDeleteOnly) {
				// we need only to backup the old file
				wchar_t *ptszRelPath = it->tszNewName + wcslen(tszMirandaPath) + 1, tszBackFile[MAX_PATH];
				mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, ptszRelPath);
				BackupFile(it->tszNewName, tszBackFile);
			}
			else {
				// if file name differs, we also need to backup the old file here
				// otherwise it would be replaced by unzip
				if (_wcsicmp(it->tszOldName, it->tszNewName)) {
					wchar_t tszSrcPath[MAX_PATH], tszBackFile[MAX_PATH];
					mir_snwprintf(tszSrcPath, L"%s\\%s", tszMirandaPath.get(), it->tszOldName);
					mir_snwprintf(tszBackFile, L"%s\\%s", tszFileBack, it->tszOldName);
					BackupFile(tszSrcPath, tszBackFile);
				}

				// remove .zip after successful update
				if (unzip(it->File.tszDiskPath, tszMirandaPath, tszFileBack, true))
					SafeDeleteFile(it->File.tszDiskPath);
			}
		}
	}
	delete &UpdateFiles;
	Skin_PlaySound("updatecompleted");

	g_plugin.bForceRedownload = false;
	g_plugin.bChangePlatform = false;
	g_plugin.delSetting(DB_SETTING_CHANGEPLATFORM);

	g_plugin.setByte(DB_SETTING_RESTART_COUNT, 5);
	g_plugin.setByte(DB_SETTING_NEED_RESTART, 1);

	// 5) Prepare Restart
	wchar_t tszTitle[100];
	mir_snwprintf(tszTitle, TranslateT("%d component(s) was updated"), count);

	if (Popup_Enabled())
		ShowPopup(tszTitle, TranslateT("You need to restart your Miranda to apply installed updates."), POPUP_TYPE_MSG);
	else {
		if (Clist_TrayNotifyW(MODULEA, tszTitle, TranslateT("You need to restart your Miranda to apply installed updates."), NIIF_INFO, 30000))
			// Error, let's try to show MessageBox as last way to inform user about successful update
			RestartPrompt(0);
	}
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
	{ L"dbtool.exe",                     nullptr },
	{ L"dbtool_sa.exe",                  nullptr },
	{ L"dbchecker.bat",                  nullptr },
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
	{ L"dropbox.dll",                    L"Plugins\\cloudfile.dll" },
	{ L"popup.dll",                      L"Plugins\\popupplus.dll" },
	{ L"libaxolotl.mir",                 L"Libs\\libsignal.mir" },

	{ L"dbx_mmap_sa.dll",                L"Plugins\\dbx_mmap.dll" },
	{ L"dbx_tree.dll",                   L"Plugins\\dbx_mmap.dll" },
	{ L"rc4.dll",                        nullptr },
	{ L"athena.dll",                     nullptr },
	{ L"skypekit.exe",                   nullptr },
	{ L"mir_app.dll",                    nullptr },
	{ L"mir_core.dll",                   nullptr },
	{ L"zlib.dll",                       nullptr },

	{ L"quotes.dll",                     L"Plugins\\currencyrates.dll" },
	{ L"proto_quotes.dll",               L"Icons\\proto_currencyrates.dll" },

	{ L"proto_newsaggr.dll",             L"Icons\\proto_newsaggregator.dll" },
	{ L"clienticons_*.dll",              L"Icons\\fp_icons.dll" },
	{ L"fp_*.dll",                       L"Icons\\fp_icons.dll" },
	{ L"xstatus_icq.dll",                nullptr },

	{ L"langpack_*.txt",                 L"Languages\\*" },

	{ L"pcre16.dll",                     nullptr },
	{ L"clist_classic.dll",              nullptr },
	{ L"chat.dll",                       nullptr },
	{ L"srmm.dll",                       nullptr },
	{ L"stdchat.dll",                    nullptr },
	{ L"stdurl.dll",                     nullptr },
	{ L"stdidle.dll",                    nullptr },
	{ L"stdhelp.dll",                    nullptr },
	{ L"stdauth.dll",                    nullptr },

	{ L"advaimg.dll",                    nullptr },
	{ L"aim.dll",                        nullptr },
	{ L"extraicons.dll",                 nullptr },
	{ L"firstrun.dll",                   nullptr },
	{ L"flashavatars.dll",               nullptr },
	{ L"gender.dll",                     nullptr },
	{ L"gtalkext.dll",                   nullptr },
	{ L"importtxt.dll",                  nullptr },
	{ L"langman.dll",                    nullptr },
	{ L"libtox.dll",                     nullptr },
	{ L"lua53.dll",                      nullptr },
	{ L"metacontacts.dll",               nullptr },
	{ L"mra.dll",                        nullptr },
	{ L"modernopt.dll",                  nullptr },
	{ L"msn.dll",                        nullptr },
	{ L"msvcp100.dll",                   nullptr },
	{ L"msvcr100.dll",                   nullptr },
	{ L"mtextcontrol.dll",               nullptr },
	{ L"sms.dll",                        nullptr },
	{ L"tlen.dll",                       nullptr },
	{ L"whatsapp.dll",                   nullptr },
	{ L"xfire.dll",                      nullptr },
	{ L"yahoo.dll",                      nullptr },
	{ L"yahoogroups.dll",                nullptr },
	{ L"yapp.dll",                       nullptr },
	{ L"WART-*.exe",                     nullptr },
};

// Checks if file needs to be renamed and copies it in pNewName
// Returns true if smth. was copied
static bool CheckFileRename(const wchar_t *ptszOldName, wchar_t *pNewName)
{
	for (auto &it : renameTable) {
		if (wildcmpiw(ptszOldName, it.oldName)) {
			wchar_t *ptszDest = it.newName;
			if (ptszDest == nullptr)
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
// We only update ".dll", ".exe", ".txt" and ".bat"

static bool isValidExtension(const wchar_t *ptszFileName)
{
	const wchar_t *pExt = wcsrchr(ptszFileName, '.');

	return (pExt != nullptr) && (!_wcsicmp(pExt, L".dll") || !_wcsicmp(pExt, L".exe") || !_wcsicmp(pExt, L".txt") || !_wcsicmp(pExt, L".bat"));
}

// We only scan subfolders "Plugins", "Icons", "Languages", "Libs", "Core"

static bool isValidDirectory(const wchar_t *ptszDirName)
{
	return !_wcsicmp(ptszDirName, L"Plugins") || !_wcsicmp(ptszDirName, L"Icons") || !_wcsicmp(ptszDirName, L"Languages") || !_wcsicmp(ptszDirName, L"Libs") || !_wcsicmp(ptszDirName, L"Core");
}

// Scans folders recursively

static int ScanFolder(const wchar_t *tszFolder, size_t cbBaseLen, const wchar_t *tszBaseUrl, SERVLIST &hashes, OBJLIST<FILEINFO> *UpdateFiles, int level = 0)
{
	wchar_t tszBuf[MAX_PATH];
	mir_snwprintf(tszBuf, L"%s\\*", tszFolder);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(tszBuf, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	Netlib_LogfW(hNetlibUser, L"Scanning folder %s", tszFolder);

	int count = 0;
	do {
		wchar_t tszNewName[MAX_PATH];

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Scan recursively all subfolders
			if (isValidDirectory(ffd.cFileName)) {
				mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
				count += ScanFolder(tszBuf, cbBaseLen, tszBaseUrl, hashes, UpdateFiles, level + 1);
			}
			continue;
		}
		
		if (isValidExtension(ffd.cFileName)) {
			// calculate the current file's relative name and store it into tszNewName
			if (CheckFileRename(ffd.cFileName, tszNewName)) {
				Netlib_LogfW(hNetlibUser, L"File %s will be renamed to %s.", ffd.cFileName, tszNewName);
				// Yes, we need the old file name, because this will be hashed later
				mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
			}
			else {
				if (level == 0) {
					// Rename Miranda*.exe
					wcsncpy_s(tszNewName, g_plugin.bChangePlatform && !mir_wstrcmpi(ffd.cFileName, OLD_FILENAME) ? NEW_FILENAME : ffd.cFileName, _TRUNCATE);
					mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, tszNewName);
				}
				else {
					mir_snwprintf(tszNewName, L"%s\\%s", tszFolder + cbBaseLen, ffd.cFileName);
					mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
				}
			}
		}
		else {
			// the only exclusion is Libs\\libmdbx.mir
			if (level == 1 && !wcsicmp(ffd.cFileName, L"libmdbx.mir")) {
				tszNewName[0] = 0;
				mir_snwprintf(tszBuf, L"%s\\%s", tszFolder, ffd.cFileName);
			}
			else continue; // skip all another filea
		}

		wchar_t *ptszUrl;
		int MyCRC;

		bool bDeleteOnly = (tszNewName[0] == 0);
		// this file is not marked for deletion
		if (!bDeleteOnly) {
			wchar_t *pName = tszNewName;
			ServListEntry *item = hashes.find((ServListEntry*)&pName);
			// Not in list? Check for trailing 'W' or 'w'
			if (item == nullptr) {
				wchar_t *p = wcsrchr(tszNewName, '.');
				if (p[-1] != 'w' && p[-1] != 'W') {
					Netlib_LogfW(hNetlibUser, L"File %s: Not found on server, skipping", ffd.cFileName);
					continue;
				}

				// remove trailing w or W and try again
				int iPos = int(p - tszNewName) - 1;
				strdelw(p - 1, 1);
				if ((item = hashes.find((ServListEntry*)&pName)) == nullptr) {
					Netlib_LogfW(hNetlibUser, L"File %s: Not found on server, skipping", ffd.cFileName);
					continue;
				}

				strdelw(tszNewName + iPos, 1);
			}

			// No need to hash a file if we are forcing a redownload anyway
			if (!g_plugin.bForceRedownload) {
				// try to hash the file
				char szMyHash[33];
				__try {
					CalculateModuleHash(tszBuf, szMyHash);
					// hashes are the same, skipping
					if (strcmp(szMyHash, item->m_szHash) == 0) {
						Netlib_LogfW(hNetlibUser, L"File %s: Already up-to-date, skipping", ffd.cFileName);
						continue;
					}
					else Netlib_LogfW(hNetlibUser, L"File %s: Update available", ffd.cFileName);
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					// smth went wrong, reload a file from scratch
				}
			}
			else Netlib_LogfW(hNetlibUser, L"File %s: Forcing redownload", ffd.cFileName);

			ptszUrl = item->m_name;
			MyCRC = item->m_crc;
		}
		else {
			// file was marked for deletion, add it to the list anyway
			Netlib_LogfW(hNetlibUser, L"File %s: Marked for deletion", ffd.cFileName);
			ptszUrl = L"";
			MyCRC = 0;
		}

		CMStringA szSetting(tszBuf + cbBaseLen);
		int bEnabled = db_get_b(0, DB_MODULE_FILES, StrToLower(szSetting.GetBuffer()), 1);
		if (bEnabled == 2)  // hidden database setting to exclude a plugin from list
			continue;

		// Yeah, we've got new version.
		FILEINFO *FileInfo = new FILEINFO;
		// copy the relative old name
		wcsncpy_s(FileInfo->tszOldName, tszBuf + cbBaseLen, _TRUNCATE);
		FileInfo->bDeleteOnly = bDeleteOnly;
		if (FileInfo->bDeleteOnly) // save the full old name for deletion
			wcsncpy_s(FileInfo->tszNewName, tszBuf, _TRUNCATE);
		else
			wcsncpy_s(FileInfo->tszNewName, ptszUrl, _TRUNCATE);

		wcsncpy_s(tszBuf, ptszUrl, _TRUNCATE);
		wchar_t *p = wcsrchr(tszBuf, '.');
		if (p) *p = 0;
		p = wcsrchr(tszBuf, '\\');
		p = (p) ? p + 1 : tszBuf;
		_wcslwr(p);

		mir_snwprintf(FileInfo->File.tszDiskPath, L"%s\\Temp\\%s.zip", g_tszRoot, p);
		mir_snwprintf(FileInfo->File.tszDownloadURL, L"%s/%s.zip", tszBaseUrl, tszBuf);
		for (p = wcschr(FileInfo->File.tszDownloadURL, '\\'); p != nullptr; p = wcschr(p, '\\'))
			*p++ = '/';

		// remember whether the user has decided not to update this component with this particular new version
		FileInfo->bEnabled = bEnabled;

		FileInfo->File.CRCsum = MyCRC;
		UpdateFiles->insert(FileInfo);

		// If we are in the silent mode, only count enabled plugins, otherwise count all
		if (!g_plugin.bSilent || FileInfo->bEnabled)
			count++;
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
	ThreadWatch threadId(dwCheckThreadId);

	wchar_t tszTempPath[MAX_PATH];
	DWORD dwLen = GetTempPath(_countof(tszTempPath), tszTempPath);
	if (tszTempPath[dwLen - 1] == '\\')
		tszTempPath[dwLen - 1] = 0;

	if (!g_plugin.bSilent)
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("Checking for new updates..."), POPUP_TYPE_INFO);

	ptrW updateUrl(GetDefaultUrl()), baseUrl;
	SERVLIST hashes(50, CompareHashes);
	bool success = ParseHashes(updateUrl, baseUrl, hashes);
	if (success) {
		FILELIST *UpdateFiles = new FILELIST(20);
		VARSW dirname(L"%miranda_path%");
		int count = ScanFolder(dirname, mir_wstrlen(dirname) + 1, baseUrl, hashes, UpdateFiles);
		if (count == 0) {
			if (!g_plugin.bSilent)
				ShowPopup(TranslateT("Plugin Updater"), TranslateT("No updates found."), POPUP_TYPE_INFO);
			delete UpdateFiles;
		}
		else {
			// Show dialog
			if (g_plugin.bSilentMode && g_plugin.bSilent)
				mir_forkthread(DlgUpdateSilent, UpdateFiles);
			else
				CallFunctionAsync(LaunchDialog, UpdateFiles);
		}
	}

	CallFunctionAsync(InitTimer, (success ? nullptr : (void*)2));

	hashes.destroy();
}

static void DoCheck(bool bSilent = true)
{
	if (dwCheckThreadId)
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("Update checking already started!"), POPUP_TYPE_INFO);
	else if (hwndDialog) {
		ShowWindow(hwndDialog, SW_SHOW);
		SetForegroundWindow(hwndDialog);
		SetFocus(hwndDialog);
	}
	else {
		g_plugin.bSilent = bSilent;
		g_plugin.setDword(DB_SETTING_LAST_UPDATE, time(0));

		mir_forkthread(CheckUpdates);
	}
}

void UninitCheck()
{
	CancelWaitableTimer(hTimer);
	CloseHandle(hTimer);
	if (hwndDialog != nullptr)
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

void CheckUpdateOnStartup()
{
	if (g_plugin.bUpdateOnStartup) {
		if (g_plugin.bOnlyOnceADay) {
			time_t now = time(0),
				was = g_plugin.getDword(DB_SETTING_LAST_UPDATE, 0);

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

static LONGLONG PeriodToMilliseconds(const int period, CMOption<int> &periodMeasure)
{
	LONGLONG result = period * 1000LL;
	switch (periodMeasure) {
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

void __stdcall InitTimer(void *type)
{
	if (!g_plugin.bUpdateOnPeriod)
		return;

	LONGLONG interval;

	switch ((INT_PTR)type) {
	case 0: // default, plan next check relative to last check
		{
			time_t now = time(0);
			time_t was = g_plugin.getDword(DB_SETTING_LAST_UPDATE, 0);

			interval = PeriodToMilliseconds(g_plugin.iPeriod, g_plugin.iPeriodMeasure);
			interval -= (now - was) * 1000;
			if (interval <= 0)
				interval = 1000; // no last update or too far in the past -> do it now
		}
		break;

	case 2: // failed last check, check again in two hours
		interval = 1000 * 60 * 60 * 2;
		break;

	default: // options changed, use set interval from now
		interval = PeriodToMilliseconds(g_plugin.iPeriod, g_plugin.iPeriodMeasure);
	}

	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	LARGE_INTEGER li;
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	li.QuadPart += interval * 10000LL;
	SetWaitableTimer(hTimer, &li, 0, TimerAPCProc, nullptr, 0);
}

void CreateTimer()
{
	hTimer = CreateWaitableTimer(nullptr, FALSE, nullptr);
	InitTimer(0);
}
