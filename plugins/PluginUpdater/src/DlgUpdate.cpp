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
static uint32_t dwCheckThreadId;
static HANDLE hTimer;

class CUpdateDLg : public CDlgBase
{
	uint32_t dwThreadId = 0;
	CCtrlButton btnDetails, btnSelAll, btnSelNone, btnOk;
	CCtrlListView m_list;
	OBJLIST<FILEINFO> *m_todo;

	static void ApplyUpdates(CUpdateDLg *pDlg)
	{
		Thread_SetName("PluginUpdater: ApplyUpdates");

		// make a local copy not to crash if a window was closed
		OBJLIST<FILEINFO> todo(*pDlg->m_todo);

		// 1) If we need to escalate priviledges, launch a stub
		if (!PU::PrepareEscalation()) {
			pDlg->Close();
			return;
		}

		{
			ThreadWatch threadId(pDlg->dwThreadId);

			// Create needed folders after escalating priviledges. Folders creates when we actually install updates
			TFileName wszTempFolder, wszBackupFolder;
			CreateWorkFolders(wszTempFolder, wszBackupFolder);

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
					if (DownloadFile(pFileUrl, nlc) != ERROR_SUCCESS) {
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
			InitMasks();

			uint32_t dwErrorCode;
			for (auto &it : todo) {
				if (it->bEnabled) {
					if (it->bDeleteOnly) {
						// we need only to backup the old file
						TFileName wszBackFile;
						mir_snwprintf(wszBackFile, L"%s\\%s", wszBackupFolder, it->wszNewName + wcslen(g_mirandaPath) + 1);
						if (dwErrorCode = BackupFile(it->wszNewName, wszBackFile)) {
LBL_Error:
							RollbackChanges(wszBackupFolder);
							Skin_PlaySound("updatefailed");
							CMStringW wszError(FORMAT, TranslateT("Unpack operation failed with error code=%d, update terminated"), dwErrorCode);
							MessageBox(pDlg->GetHwnd(), wszError, TranslateT("Plugin Updater"), MB_OK | MB_ICONERROR);
							pDlg->Close();
							return;
						}
					}
					else {
						// if file name differs, we also need to backup the old file here
						// otherwise it would be replaced by unzip
						if (_wcsicmp(it->wszOldName, it->wszNewName)) {
							TFileName wszSrcPath, wszBackFile;
							mir_snwprintf(wszSrcPath, L"%s\\%s", g_mirandaPath.get(), it->wszOldName);
							mir_snwprintf(wszBackFile, L"%s\\%s", wszBackupFolder, it->wszOldName);
							if (dwErrorCode = BackupFile(wszSrcPath, wszBackFile))
								goto LBL_Error;
						}

						if (dwErrorCode = unzip(it->File.wszDiskPath, g_mirandaPath, wszBackupFolder, true))
							goto LBL_Error;

						PU::SafeDeleteFile(it->File.wszDiskPath);  // remove .zip after successful update
					}
				}
			}

			UninitMasks();
			RemoveBackupFolders();
			Skin_PlaySound("updatecompleted");

			if (g_plugin.bBackup)
				CallService(MS_AB_BACKUP, 0, 0);
		}

		// 5) Prepare Restart
		if (!g_plugin.bAutoRestart)
			if (IDYES != MessageBox(pDlg->GetHwnd(), TranslateT("Update complete. Press Yes to restart Miranda now or No to postpone a restart until the exit."), TranslateT("Plugin Updater"), MB_YESNO | MB_ICONQUESTION)) {
				pDlg->Close();
				return;
			}

		pDlg->Close();
		DoRestart();
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

			CMStringA szSetting(it->wszOldName);
			db_set_b(0, DB_MODULE_FILES, szSetting.MakeLower(), it->bEnabled = bEnable);
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
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_MENU));

		if (!PU::IsMirandaFolderWritable())
			Button_SetElevationRequiredState(btnOk.GetHwnd(), !PU::IsProcessElevated());

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
			lvI.iGroupId = (wcsstr(it->wszOldName, L"Plugins") != nullptr) ? 1 :
				((wcsstr(it->wszOldName, L"Languages") != nullptr) ? 3 :
				((wcsstr(it->wszOldName, L"Icons") != nullptr) ? 4 : 2));
			lvI.iSubItem = 0;
			lvI.lParam = (LPARAM)it;
			lvI.pszText = it->wszOldName;
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
			CMStringA szSetting(p->wszOldName);
			db_set_b(0, DB_MODULE_FILES, szSetting.MakeLower(), p->bEnabled = m_list.GetCheckState(nmlv->iItem));

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
		dwThreadId = 0;
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
	if (!PU::PrepareEscalation()) {
		delete &UpdateFiles;
		return;
	}

	// Create needed folders after escalating priviledges. Folders creates when we actually install updates
	TFileName wszTempFolder, wszBackupFolder;
	CreateWorkFolders(wszTempFolder, wszBackupFolder);

	// 2) Download all plugins
	HNETLIBCONN nlc = nullptr;
	// Count all updates that have been enabled
	int count = 0;
	for (auto &it : UpdateFiles) {
		if (it->bDeleteOnly)
			count++;
		else if (it->bEnabled) {
			// download update
			if (DownloadFile(&it->File, nlc) != ERROR_SUCCESS) {
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
	InitMasks();

	uint32_t dwErrorCode;
	for (auto &it : UpdateFiles) {
		if (it->bEnabled) {
			if (it->bDeleteOnly) {
				// we need only to backup the old file
				TFileName wszBackFile;
				mir_snwprintf(wszBackFile, L"%s\\%s", wszBackupFolder, it->wszNewName + wcslen(g_mirandaPath) + 1);
				if (dwErrorCode = BackupFile(it->wszNewName, wszBackFile)) {
LBL_Error:
					RollbackChanges(wszBackupFolder);
					Skin_PlaySound("updatefailed");
					delete &UpdateFiles;
					return;
				}
			}
			else {
				// if file name differs, we also need to backup the old file here
				// otherwise it would be replaced by unzip
				if (_wcsicmp(it->wszOldName, it->wszNewName)) {
					TFileName wszSrcPath, wszBackFile;
					mir_snwprintf(wszSrcPath, L"%s\\%s", g_mirandaPath.get(), it->wszOldName);
					mir_snwprintf(wszBackFile, L"%s\\%s", wszBackupFolder, it->wszOldName);
					if (dwErrorCode = BackupFile(wszSrcPath, wszBackFile))
						goto LBL_Error;
				}

				// remove .zip after successful update
				if (dwErrorCode = unzip(it->File.wszDiskPath, g_mirandaPath, wszBackupFolder, true))
					goto LBL_Error;
				PU::SafeDeleteFile(it->File.wszDiskPath);
			}
		}
	}

	UninitMasks();
	RemoveBackupFolders();

	delete &UpdateFiles;
	Skin_PlaySound("updatecompleted");

	g_plugin.bForceRedownload = false;
	g_plugin.bNeedRestart = true;
	g_plugin.bChangePlatform.Delete();

	if (g_plugin.bBackup)
		CallService(MS_AB_BACKUP, 0, 0);

	// 5) Prepare Restart
	if (g_plugin.bAutoRestart) {
		RestartPrompt(0);
		return;
	}

	wchar_t wszTitle[100];
	mir_snwprintf(wszTitle, TranslateT("%d component(s) was updated"), count);

	if (Popup_Enabled())
		ShowPopup(wszTitle, TranslateT("You need to restart your Miranda to apply installed updates."), POPUP_TYPE_MSG);
	else {
		if (Clist_TrayNotifyW(MODULEA, wszTitle, TranslateT("You need to restart your Miranda to apply installed updates."), NIIF_INFO, 30000))
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
	{ L"fixme.cmd",                      nullptr },
	{ L"mdbx_chk.exe",                   nullptr },
	{ L"mdbx_dump.exe",                  nullptr },
	{ L"mdbx_load.exe",                  nullptr },
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
	{ L"stdfile.dll",                    nullptr },
	{ L"stdhelp.dll",                    nullptr },
	{ L"stdauth.dll",                    nullptr },
	{ L"stdssl.dll",                     nullptr },

	{ L"advaimg.dll",                    nullptr },
	{ L"aim.dll",                        nullptr },
	{ L"extraicons.dll",                 nullptr },
	{ L"firstrun.dll",                   nullptr },
	{ L"flashavatars.dll",               nullptr },
	{ L"gender.dll",                     nullptr },
	{ L"gtalkext.dll",                   nullptr },
	{ L"icq.dll",                        nullptr },
	{ L"importtxt.dll",                  nullptr },
	{ L"langman.dll",                    nullptr },
	{ L"libcrypto-1_1.mir",              nullptr },
	{ L"libssl-1_1.mir",                 nullptr },
	{ L"libtox.dll",                     nullptr },
	{ L"lua53.dll",                      nullptr },
	{ L"metacontacts.dll",               nullptr },
	{ L"mra.dll",                        nullptr },
	{ L"modernopt.dll",                  nullptr },
	{ L"msn.dll",                        nullptr },
	{ L"msvcp100.dll",                   nullptr },
	{ L"msvcr100.dll",                   nullptr },
	{ L"mtextcontrol.dll",               nullptr },
	{ L"omegle.dll",                     nullptr },
	{ L"openssl.dll",                    nullptr },
	{ L"rate.dll",                       nullptr },
	{ L"spamotron.dll",                  nullptr },
	{ L"sms.dll",                        nullptr },
	{ L"tlen.dll",                       nullptr },
	{ L"xfire.dll",                      nullptr },
	{ L"yahoo.dll",                      nullptr },
	{ L"yahoogroups.dll",                nullptr },
	{ L"yapp.dll",                       nullptr },
	{ L"WART-*.exe",                     nullptr },
};

// Checks if file needs to be renamed and copies it in pNewName
// Returns true if smth. was copied
static bool CheckFileRename(const wchar_t *pwszFolder, const wchar_t *pwszOldName, wchar_t *pNewName)
{
	MFilePath fullOldPath;
	fullOldPath.Format(L"%s\\%s", pwszFolder, pwszOldName);

	for (auto &it : renameTable) {
		if (wildcmpiw(pwszOldName, it.oldName)) {
			if (it.newName == nullptr)
				*pNewName = 0;
			else {
				wcsncpy_s(pNewName, MAX_PATH, it.newName, _TRUNCATE);
				size_t cbLen = wcslen(it.newName) - 1;
				if (pNewName[cbLen] == '*')
					wcsncpy_s(pNewName + cbLen, MAX_PATH - cbLen, pwszOldName, _TRUNCATE);

				// don't try to rename a file to itself
				MFilePath fullNewPath;
				fullNewPath.Format(L"%s\\%s", g_mirandaPath.get(), pNewName);
				if (fullNewPath == fullOldPath)
					return false;
			}
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// We only update ".dll", ".exe", ".txt" and ".bat"

static wchar_t wszExtensionList[][5] = { L".dll", L".exe", L".txt", L".bat", L".cmd", L".mir" };

static bool isValidExtension(const wchar_t *pwszFileName)
{
	if (const wchar_t *pExt = wcsrchr(pwszFileName, '.'))
		for (auto &it : wszExtensionList)
			if (!_wcsicmp(pExt, it))
				return true;
	
	return false;
}

// We only scan subfolders "Plugins", "Icons", "Languages", "Libs", "Core"

static bool isValidDirectory(const wchar_t *pwszDirName)
{
	return !_wcsicmp(pwszDirName, L"Plugins") || !_wcsicmp(pwszDirName, L"Icons") || !_wcsicmp(pwszDirName, L"Languages") || !_wcsicmp(pwszDirName, L"Libs") || !_wcsicmp(pwszDirName, L"Core");
}

// Scans folders recursively

static int ScanFolder(const wchar_t *pwszFolder, size_t cbBaseLen, const wchar_t *pwszBaseUrl, SERVLIST &hashes, OBJLIST<FILEINFO> *UpdateFiles, int level = 0)
{
	Netlib_LogfW(g_hNetlibUser, L"Scanning folder %s", pwszFolder);

	int count = 0;

	MFilePath wszBuf(pwszFolder);
	for (auto &ff : wszBuf.search()) {
		TFileName wszNewName;

		if (ff.isDir()) {
			// Scan recursively all subfolders
			if (isValidDirectory(ff.getPath())) {
				wszBuf.Format(L"%s\\%s", pwszFolder, ff.getPath());
				count += ScanFolder(wszBuf, cbBaseLen, pwszBaseUrl, hashes, UpdateFiles, level + 1);
			}
			continue;
		}

		// create full name of the current file
		wszBuf.Format(L"%s\\%s", pwszFolder, ff.getPath());

		if (isValidExtension(ff.getPath())) {
			// calculate the current file's relative name and store it into wszNewName
			if (CheckFileRename(pwszFolder, ff.getPath(), wszNewName)) {
				if (wszNewName[0])
					Netlib_LogfW(g_hNetlibUser, L"File <%s> will be renamed to <%s>", wszBuf.c_str(), wszNewName);
				else
					Netlib_LogfW(g_hNetlibUser, L"File <%s> will be deleted", wszBuf.c_str());
			}
			else {
				if (level == 0) {
					// Rename Miranda*.exe
					wcsncpy_s(wszNewName, g_plugin.bChangePlatform && !mir_wstrcmpi(ff.getPath(), OLD_FILENAME) ? NEW_FILENAME : ff.getPath(), _TRUNCATE);
					wszBuf.Format(L"%s\\%s", pwszFolder, wszNewName);
				}
				else mir_snwprintf(wszNewName, L"%s\\%s", pwszFolder + cbBaseLen, ff.getPath());
			}
		}
		else {
			if (!wcsicmp(ff.getPath(), L"libeay32.mir") || !wcsicmp(ff.getPath(), L"ssleay32.mir")) // remove old OpenSSL modules
				wszNewName[0] = 0;
			else
				continue; // skip all another files
		}

		wchar_t *pwszUrl;
		int MyCRC;

		bool bDeleteOnly = (wszNewName[0] == 0);
		// this file is not marked for deletion
		if (!bDeleteOnly) {
			wchar_t *pName = wszNewName;
			ServListEntry *item = hashes.find((ServListEntry*)&pName);
			// Not in list? Check for trailing 'W' or 'w'
			if (item == nullptr) {
				wchar_t *p = wcsrchr(wszNewName, '.');
				if (p[-1] != 'w' && p[-1] != 'W') {
					Netlib_LogfW(g_hNetlibUser, L"File %s: Not found on server, skipping", ff.getPath());
					continue;
				}

				// remove trailing w or W and try again
				int iPos = int(p - wszNewName) - 1;
				strdelw(p - 1, 1);
				if ((item = hashes.find((ServListEntry*)&pName)) == nullptr) {
					Netlib_LogfW(g_hNetlibUser, L"File %s: Not found on server, skipping", ff.getPath());
					continue;
				}

				strdelw(wszNewName + iPos, 1);
			}

			// No need to hash a file if we are forcing a redownload anyway
			if (!g_plugin.bForceRedownload) {
				// try to hash the file
				char szMyHash[33];
				__try {
					CalculateModuleHash(wszBuf, szMyHash);
					// hashes are the same, skipping
					if (strcmp(szMyHash, item->m_szHash) == 0) {
						Netlib_LogfW(g_hNetlibUser, L"File %s: Already up-to-date, skipping", ff.getPath());
						continue;
					}
					else Netlib_LogfW(g_hNetlibUser, L"File %s: Update available", ff.getPath());
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					// smth went wrong, reload a file from scratch
				}
			}
			else Netlib_LogfW(g_hNetlibUser, L"File %s: Forcing redownload", ff.getPath());

			pwszUrl = item->m_name;
			MyCRC = item->m_crc;
		}
		else {
			// file was marked for deletion, add it to the list anyway
			Netlib_LogfW(g_hNetlibUser, L"File %s: Marked for deletion", ff.getPath());
			pwszUrl = L"";
			MyCRC = 0;
		}

		CMStringA szSetting(wszBuf.c_str() + cbBaseLen);
		int bEnabled = db_get_b(0, DB_MODULE_FILES, szSetting.MakeLower(), 1);
		if (bEnabled == 2)  // hidden database setting to exclude a plugin from list
			continue;

		// Yeah, we've got new version.
		FILEINFO *FileInfo = new FILEINFO;
		wcsncpy_s(FileInfo->wszOldName, wszBuf.c_str() + cbBaseLen, _TRUNCATE); // copy the relative old name
		FileInfo->bDeleteOnly = bDeleteOnly;
		if (FileInfo->bDeleteOnly) // save the full old name for deletion
			wcsncpy_s(FileInfo->wszNewName, wszBuf, _TRUNCATE);
		else
			wcsncpy_s(FileInfo->wszNewName, pwszUrl, _TRUNCATE);

		wchar_t buf[MAX_PATH];
		wcsncpy_s(buf, pwszUrl, _TRUNCATE);
		wchar_t *p = wcsrchr(buf, '.');
		if (p) *p = 0;
		p = wcsrchr(buf, '\\');
		p = (p) ? p + 1 : buf;
		_wcslwr(p);

		mir_snwprintf(FileInfo->File.wszDiskPath, L"%s\\Temp\\%s.zip", g_wszRoot, p);
		mir_snwprintf(FileInfo->File.wszDownloadURL, L"%s/%s.zip", pwszBaseUrl, buf);
		for (p = wcschr(FileInfo->File.wszDownloadURL, '\\'); p != nullptr; p = wcschr(p, '\\'))
			*p++ = '/';

		// remember whether the user has decided not to update this component with this particular new version
		FileInfo->bEnabled = bEnabled;

		FileInfo->File.CRCsum = MyCRC;
		UpdateFiles->insert(FileInfo);

		// If we are in the silent mode, only count enabled plugins, otherwise count all
		if (!g_plugin.bSilent || FileInfo->bEnabled)
			count++;
	}

	return count;
}

// Thread checks for updates
static void CheckUpdates(void *)
{
	Netlib_LogfW(g_hNetlibUser, L"Checking for updates");
	Thread_SetName("PluginUpdater: CheckUpdates");
	ThreadWatch threadId(dwCheckThreadId);

	TFileName wszTempPath;
	uint32_t dwLen = GetTempPath(_countof(wszTempPath), wszTempPath);
	if (wszTempPath[dwLen - 1] == '\\')
		wszTempPath[dwLen - 1] = 0;

	if (!g_plugin.bSilent)
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("Checking for new updates..."), POPUP_TYPE_INFO);

	ptrW updateUrl(GetDefaultUrl()), baseUrl;
	SERVLIST hashes(50, CompareHashes);
	bool success = ParseHashes(updateUrl, baseUrl, hashes);
	if (success) {
		if (hashes.getCount()) {
			FILELIST *UpdateFiles = new FILELIST(20);
			int count = ScanFolder(g_mirandaPath, mir_wstrlen(g_mirandaPath) + 1, baseUrl, hashes, UpdateFiles);
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

		// reset timer to next update
		g_plugin.dwLastUpdate = time(0);
		g_plugin.InitTimer(0);
	}
	else g_plugin.InitTimer(1); // update failed, postpone the timer

	hashes.destroy();
}

void DoCheck(bool bSilent)
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

void CALLBACK CheckUpdateOnStartup()
{
	if (g_plugin.bUpdateOnStartup) {
		if (g_plugin.bOnlyOnceADay)
			if (time(0) - g_plugin.dwLastUpdate < 86400)
				return;

		Netlib_LogfW(g_hNetlibUser, L"Update on startup started!");
		DoCheck();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMPlugin::Impl::onTimer(CTimer*)
{
	if (g_plugin.iNextCheck)
		if (time(0) >= g_plugin.iNextCheck)
			DoCheck();
}

void CMPlugin::InitTimer(int type)
{
	if (!bUpdateOnPeriod) {
		iNextCheck = 0;
		return;
	}

	// normal timer reset;
	if (type == 0) {
		iNextCheck = dwLastUpdate;
		if (iPeriodMeasure == 1)
			iNextCheck += g_plugin.iPeriod * 86400; // day
		else
			iNextCheck += g_plugin.iPeriod * 3600; // hour
	}
	else // failed last check, check again in two hours
		iNextCheck += 60 * 60 * 2;
}
