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

struct
{
	const wchar_t *pwszFolder, *pwszGroup;
}
static groupList[] = {
	{ L"Plugins",   LPGENW("Plugins")         },
	{ L"Core",      LPGENW("Miranda NG Core") },
	{ L"Languages", LPGENW("Languages")       },
	{ L"Icons",     LPGENW("Icons")           },
	{ L"Libs",      LPGENW("Libraries")       },
};

static int file2group(const wchar_t *pwszFileName)
{
	for (int i = 0; i < _countof(groupList); i++)
		if (wcsstr(pwszFileName, groupList[i].pwszFolder))
			return i+1;

	return 2; // core
}

static bool isValidDirectory(const wchar_t *pwszDirName)
{
	for (auto &it : groupList)
		if (!_wcsicmp(pwszDirName, it.pwszFolder))
			return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
				if (!it->bEnabled)
					continue;

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

		LVGROUP lvg = {};
		lvg.cbSize = sizeof(LVGROUP);
		lvg.mask = LVGF_HEADER | LVGF_GROUPID;

		for (auto &it : groupList) {
			lvg.pszHeader = TranslateW(it.pwszGroup);
			lvg.iGroupId++;
			m_list.InsertGroup(0, &lvg);
		}
		m_list.EnableGroupView(true);

		bool enableOk = false;
		for (auto &it : *m_todo) {
			LVITEM lvI = { 0 };
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_GROUPID | LVIF_NORECOMPUTE;
			lvI.iGroupId = file2group(it->wszOldName);
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

// Scans folders recursively

static int ScanFolder(const wchar_t *pwszFolder, size_t cbBaseLen, ServerConfig &config, OBJLIST<FILEINFO> *UpdateFiles, int level)
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
				count += ScanFolder(wszBuf, cbBaseLen, config, UpdateFiles, level + 1);
			}
			continue;
		}

		// create full name of the current file
		wszBuf.Format(L"%s\\%s", pwszFolder, ff.getPath());

		if (isValidExtension(ff.getPath())) {
			// calculate the current file's relative name and store it into wszNewName
			if (config.CheckRename(pwszFolder, ff.getPath(), wszNewName)) {
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
			auto *item = config.FindHash(pName);
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
				if ((item = config.FindHash(pName)) == nullptr) {
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
					if (!strcmp(szMyHash, item->m_szHash)) {
						// check missing dependencies
						bool bMissingDeps = false;
						if (auto *pPacket = config.FindPacket(item->m_name)) {
							for (auto &dep : pPacket->arDepends) {
								CMStringW wszDepFile(FORMAT, L"%s\\%s", g_mirandaPath.get(), dep);
								if (_waccess(wszDepFile, 0))
									bMissingDeps = true;
							}
						}

						if (!bMissingDeps) {
							Netlib_LogfW(g_hNetlibUser, L"File %s: Already up-to-date, skipping", ff.getPath());
							continue;
						}
						Netlib_LogfW(g_hNetlibUser, L"File %s: Installing missing dependencies", ff.getPath());
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
		FILEINFO *FileInfo = new FILEINFO(wszBuf.c_str() + cbBaseLen, bDeleteOnly ? wszBuf : pwszUrl, pwszUrl, config);
		FileInfo->bDeleteOnly = bDeleteOnly;

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

	ServerConfig config;
	bool success = config.Load();
	if (success) {
		if (config.arHashes.getCount()) {
			FILELIST *UpdateFiles = new FILELIST(20);
			int count = ScanFolder(g_mirandaPath, mir_wstrlen(g_mirandaPath) + 1, config, UpdateFiles, 0);
			if (count == 0) {
				if (!g_plugin.bSilent)
					ShowPopup(TranslateT("Plugin Updater"), TranslateT("No updates found."), POPUP_TYPE_INFO);
				delete UpdateFiles;
			}
			else {
				for (auto &it : *UpdateFiles) {
					if (auto *pPacket = config.FindPacket(it->wszOldName)) {
						for (auto &dep : pPacket->arDepends) {
							// if a dependency file doesn't exist, add it forcibly
							CMStringW wszDepFile(FORMAT, L"%s\\%s", g_mirandaPath.get(), dep);
							if (_waccess(wszDepFile, 0)) {
								if (auto *pHash = config.FindHash(dep)) {
									auto *FileInfo = new FILEINFO(dep, pHash->m_name, pHash->m_name, config);
									FileInfo->File.CRCsum = pHash->m_crc;
									UpdateFiles->insert(FileInfo);
								}
							}
						}
					}
				}

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
