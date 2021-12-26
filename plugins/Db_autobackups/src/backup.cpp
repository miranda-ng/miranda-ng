#include "stdafx.h"

static UINT_PTR timer_id = 0;
static volatile long g_iState = 0;

static LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		{
			wchar_t *ptszPath = (wchar_t*)PUGetPluginData(hWnd);
			if (ptszPath != nullptr)
				ShellExecute(nullptr, L"open", ptszPath, nullptr, nullptr, SW_SHOW);

			PUDeletePopup(hWnd);
			break;
		}
	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		mir_free(PUGetPluginData(hWnd));
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

static void ShowPopup(const wchar_t *ptszText, wchar_t *ptszHeader, wchar_t *ptszPath)
{
	if (Miranda_IsTerminated())
		return;

	POPUPDATAW ppd;
	wcsncpy_s(ppd.lpwzText, ptszText, _TRUNCATE);
	wcsncpy_s(ppd.lpwzContactName, ptszHeader, _TRUNCATE);
	if (ptszPath != nullptr)
		ppd.PluginData = (void*)mir_wstrdup(ptszPath);
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = g_plugin.getIcon(IDI_BACKUP);
	PUAddPopupW(&ppd);
}

static INT_PTR CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) != BN_CLICKED || LOWORD(wParam) != IDCANCEL)
			break;
		// in the progress dialog, use the user data to indicate that the user has pressed cancel
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 1);
		return TRUE;
		break;
	}
	return FALSE;
}

static bool MakeZip_Dir(LPCWSTR szDir, LPCWSTR pwszProfile, LPCWSTR szDest, LPCWSTR pwszBackupFolder, HWND progress_dialog)
{
	HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);
	size_t count = 0, folderNameLen = mir_wstrlen(pwszBackupFolder);
	OBJLIST<ZipFile> lstFiles(15);

	wchar_t wszTempName[MAX_PATH];
	if (!GetTempPathW(_countof(wszTempName), wszTempName))
		return false;

	if (!GetTempFileNameW(wszTempName, L"mir_backup_", 0, wszTempName))
		return false;

	if (db_get_current()->Backup(wszTempName))
		return false;

	lstFiles.insert(new ZipFile(wszTempName, pwszProfile));

	CMStringW wszProfile;
	wszProfile.Format(L"%s\\%s", szDir, pwszProfile);

	for (auto it = fs::recursive_directory_iterator(fs::path(szDir)); it != fs::recursive_directory_iterator(); ++it) {
		const auto& file = it->path();
		if (fs::is_directory(file))
			continue;

		const std::wstring &filepath = file.wstring();
		if (wszProfile == filepath.c_str())
			continue;

		if (filepath.find(szDest) != std::wstring::npos || !mir_wstrncmpi(filepath.c_str(), pwszBackupFolder, folderNameLen))
			continue;

		const std::wstring rpath = filepath.substr(filepath.find(szDir) + mir_wstrlen(szDir) + 1);
		lstFiles.insert(new ZipFile(filepath, rpath));
		count++;
	}

	if (count == 0)
		return 1;

	CreateZipFile(szDest, lstFiles, [&](size_t i)->bool {
		SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100 * i / count), 0);
		return GetWindowLongPtr(progress_dialog, GWLP_USERDATA) != 1;
	});

	DeleteFileW(wszTempName);
	return 1;
}

static bool MakeZip(wchar_t *tszDest, wchar_t *dbname, HWND progress_dialog)
{
	HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);

	wchar_t wszTempName[MAX_PATH];
	if (!GetTempPathW(_countof(wszTempName), wszTempName))
		return false;

	if (!GetTempFileNameW(wszTempName, L"mir_backup_", 0, wszTempName))
		return false;

	if (db_get_current()->Backup(wszTempName))
		return false;

	OBJLIST<ZipFile> lstFiles(1);
	lstFiles.insert(new ZipFile(wszTempName, dbname));

	CreateZipFile(tszDest, lstFiles, [&](size_t)->bool { SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100), 0); return true; });
	DeleteFileW(wszTempName);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct backupFile
{
	wchar_t Name[MAX_PATH];
	FILETIME CreationTime;
};

static int Comp(const backupFile *p1, const backupFile *p2)
{
	if (p1->CreationTime.dwHighDateTime == p2->CreationTime.dwHighDateTime) // sort backwards
		return (p1->CreationTime.dwLowDateTime > p2->CreationTime.dwLowDateTime) ? -1 : 1;

	return (p1->CreationTime.dwHighDateTime > p2->CreationTime.dwHighDateTime) ? -1 : 1;
}

static int RotateBackups(wchar_t *backupfolder, wchar_t *dbname)
{
	if (g_plugin.num_backups == 0) // Rotation disabled?
		return 0; 

	CMStringW wszProfile(dbname);
	wszProfile.MakeLower();
	int idx = wszProfile.Find(L".dat");
	if (idx != -1)
		wszProfile.Truncate(idx);

	wchar_t backupfolderTmp[MAX_PATH];
	mir_snwprintf(backupfolderTmp, L"%s\\%s*.%s", backupfolder, wszProfile.c_str(), g_plugin.use_zip ? L"zip" : L"dat");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(backupfolderTmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	OBJLIST<backupFile> arFiles(10, Comp);
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		backupFile *bf = new backupFile();
		wcsncpy_s(bf->Name, FindFileData.cFileName, _TRUNCATE);
		bf->CreationTime = FindFileData.ftCreationTime;
		arFiles.insert(bf);
	}
		while (FindNextFile(hFind, &FindFileData));

	for (int i = g_plugin.num_backups-1; i < arFiles.getCount(); i++) {
		mir_snwprintf(backupfolderTmp, L"%s\\%s", backupfolder, arFiles[i].Name);
		DeleteFile(backupfolderTmp);
	}

	FindClose(hFind);
	return 0;
}

static int Backup(wchar_t *backup_filename)
{
	bool bZip = false;
	wchar_t dbname[MAX_PATH], dest_file[MAX_PATH];

	Profile_GetNameW(_countof(dbname), dbname);

	wchar_t backupfolder[MAX_PATH];
	PathToAbsoluteW(VARSW(g_plugin.folder), backupfolder);

	// ensure the backup folder exists (either create it or return non-zero signifying error)
	int err = CreateDirectoryTreeW(backupfolder);
	if (err != ERROR_ALREADY_EXISTS && err != 0) {
		mir_free(backupfolder);
		return 1;
	}

	if (backup_filename == nullptr) {
		bZip = g_plugin.use_zip != 0;
		RotateBackups(backupfolder, dbname);

		CMStringW wszFileName;
		if (ServiceExists(MS_VARS_FORMATSTRING))
			wszFileName = VARSW(ptrW(variables_parse(g_plugin.file_mask, nullptr, 0)));
		else
			wszFileName = VARSW(g_plugin.file_mask);

		wchar_t buffer[MAX_PATH];
		DWORD size = _countof(buffer);
		GetComputerName(buffer, &size);
		wszFileName.Replace(L"%compname%", buffer);

		SYSTEMTIME st;
		GetLocalTime(&st);
		mir_snwprintf(buffer, L"%02d.%02d.%02d@%02d-%02d-%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		wszFileName.Replace(L"%currtime%", buffer);

		mir_snwprintf(dest_file, L"%s\\%s.%s", backupfolder, wszFileName.c_str(), bZip ? L"zip" : L"dat");
	}
	else {
		wcsncpy_s(dest_file, backup_filename, _TRUNCATE);
		if (!mir_wstrcmp(wcsrchr(backup_filename, '.'), L".zip"))
			bZip = true;
	}
	
	if (!g_plugin.disable_popups)
		ShowPopup(dbname, TranslateT("Backup in progress"), nullptr);

	HWND progress_dialog = nullptr;
	if (!g_plugin.disable_progress) {
		progress_dialog = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_COPYPROGRESS), nullptr, DlgProcProgress);
		SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Copying database file..."));
	}

	BOOL res;
	if (bZip) {
		res = g_plugin.backup_profile
			? MakeZip_Dir(VARSW(L"%miranda_userdata%"), dbname, dest_file, backupfolder, progress_dialog)
			: MakeZip(dest_file, dbname, progress_dialog);
	}
	else res = db_get_current()->Backup(dest_file) == ERROR_SUCCESS;

	if (res) {
		if (!bZip) { // Set the backup file to the current time for rotator's correct work
			SYSTEMTIME st;
			GetSystemTime(&st);

			HANDLE hFile = CreateFile(dest_file, FILE_WRITE_ATTRIBUTES, NULL, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			FILETIME ft;
			SystemTimeToFileTime(&st, &ft);
			SetFileTime(hFile, nullptr, nullptr, &ft);
			CloseHandle(hFile);
		}
		
		if (progress_dialog) {
			SendDlgItemMessage(progress_dialog, IDC_PROGRESS, PBM_SETPOS, (WPARAM)(100), 0);
			UpdateWindow(progress_dialog);
		}
		g_plugin.setDword("LastBackupTimestamp", (uint32_t)time(0));
		NotifyEventHooks(g_plugin.hevBackup);

		if (g_plugin.use_cloudfile) {
			CFUPLOADDATA ui = { g_plugin.cloudfile_service, dest_file, L"Backups" };
			if (CallService(MS_CLOUDFILE_UPLOAD, (LPARAM)&ui))
				ShowPopup(TranslateT("Uploading to cloud failed"), TranslateT("Error"), nullptr);
		}

		wchar_t *pd = wcsrchr(dest_file, '\\');

		if (!g_plugin.disable_popups) {
			CMStringW puText;

			if (pd && mir_wstrlen(dest_file) > 50) {
				puText.Append(dest_file, pd - dest_file);
				puText.AppendChar('\n');
				puText.Append(pd + 1);
			}
			else puText = dest_file;

			// Now we need to know, which folder we made a backup. Let's break unnecessary variables :)
			if (pd) *pd = 0;
			ShowPopup(puText, TranslateT("Database backed up"), dest_file);
		}
	}
	else DeleteFileW(dest_file);

	if (progress_dialog)
		DestroyWindow(progress_dialog);
	return 0;
}

static void BackupThread(void *backup_filename)
{
	Backup((wchar_t*)backup_filename);
	InterlockedExchange(&g_iState, 0); // Backup done.
	mir_free(backup_filename);
}

int BackupStatus()
{
	return InterlockedCompareExchange(&g_iState, 1, 0);
}

void BackupStart(wchar_t *backup_filename)
{
	if (BackupStatus() != 0) { // Backup allready in process.
		ShowPopup(TranslateT("Database back up in process..."), TranslateT("Error"), nullptr);
		return;
	}

	if (mir_forkthread(BackupThread, mir_wstrdup(backup_filename)) == INVALID_HANDLE_VALUE)
		InterlockedExchange(&g_iState, 0); // Backup done.
}

VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	time_t t = time(0);
	time_t diff = t - (time_t)g_plugin.getDword("LastBackupTimestamp");

	int iMultiplicator;
	switch (g_plugin.period_type) {
	case PT_MINUTES: iMultiplicator = 60; break;
	case PT_HOURS: iMultiplicator = 3600; break;
	default: iMultiplicator = 86400; break; // days
	}

	if (diff > time_t(g_plugin.period) * iMultiplicator)
		BackupStart(nullptr);
}

int SetBackupTimer(void)
{
	if (timer_id != 0) {
		KillTimer(nullptr, timer_id);
		timer_id = 0;
	}
	if (g_plugin.backup_types & BT_PERIODIC)
		timer_id = SetTimer(nullptr, 0, (1000 * 60), TimerProc);
	return 0;
}
