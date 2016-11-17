#include "stdafx.h"

static UINT_PTR	timer_id = 0;
volatile long m_state = 0;


LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
	{
		wchar_t* ptszPath = (wchar_t*)PUGetPluginData(hWnd);
		if (ptszPath != 0)
			ShellExecute(0, L"open", ptszPath, NULL, NULL, SW_SHOW);

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
void ShowPopup(wchar_t* ptszText, wchar_t* ptszHeader, wchar_t* ptszPath)
{
	POPUPDATAT ppd = { 0 };

	wcsncpy_s(ppd.lptzText, ptszText, _TRUNCATE);
	wcsncpy_s(ppd.lptzContactName, ptszHeader, _TRUNCATE);
	if (ptszPath != NULL)
		ppd.PluginData = (void*)mir_wstrdup(ptszPath);
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = IcoLib_GetIcon(iconList[0].szName);

	PUAddPopupT(&ppd);
}

INT_PTR CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
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

wchar_t* DoubleSlash(wchar_t *sorce)
{
	wchar_t *ret, *r, *s;

	ret = (wchar_t*)mir_alloc((MAX_PATH * sizeof(wchar_t)));
	if (ret == NULL)
		return NULL;
	for (s = sorce, r = ret; *s && (r - ret) < (MAX_PATH - 1); s++, r++) {
		if (*s != '\\')
			*r = *s;
		else {
			*r = '\\';
			r++;
			*r = '\\';
		}
	}
	r[0] = 0;
	return ret;
}

bool MakeZip_Dir(LPCSTR szDir, LPCTSTR szDest, LPCSTR /* szDbName */, HWND progress_dialog)
{
	HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);
	size_t count = 0;
	OBJLIST<ZipFile> lstFiles(15);

	for (auto it = fs::recursive_directory_iterator(fs::path(szDir)); it != fs::recursive_directory_iterator(); ++it)
	{
		const auto& file = it->path();
		if (!fs::is_directory(file) && file.string().find(fs::path((char*)_T2A(szDest)).string().c_str()) == std::string::npos)
		{
			const std::string &filepath = file.string();
			const std::string rpath = filepath.substr(filepath.find(szDir) + mir_strlen(szDir) + 1);

			lstFiles.insert(new ZipFile(filepath, rpath));
			count++;
		}
	}
	if (count == 0) 
		return 1;

	CreateZipFile(_T2A(szDest), lstFiles, [&](size_t i)->bool
	{ 
		SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100 * i / count), 0); 
		return GetWindowLongPtr(progress_dialog, GWLP_USERDATA) != 1;
	});

	return 1;
}

bool MakeZip(wchar_t *tszSource, wchar_t *tszDest, wchar_t *dbname, HWND progress_dialog)
{
	HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);

	ptrA szSourceName(mir_u2a(dbname));
	ptrW tszDestPath(DoubleSlash(tszDest));
	OBJLIST<ZipFile> lstFiles(15);
	lstFiles.insert(new ZipFile((char*)_T2A(tszSource), (char*)szSourceName));

	CreateZipFile(_T2A(tszDest), lstFiles, [&](size_t)->bool{ SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100), 0); return true; });

	return true;
}


struct backupFile
{
	wchar_t Name[MAX_PATH];
	FILETIME CreationTime;
};

int Comp(const void *i, const void *j)
{
	backupFile *pi = (backupFile*)i;
	backupFile *pj = (backupFile*)j;

	if (pi->CreationTime.dwHighDateTime > pj->CreationTime.dwHighDateTime ||
		(pi->CreationTime.dwHighDateTime == pj->CreationTime.dwHighDateTime && pi->CreationTime.dwLowDateTime > pj->CreationTime.dwLowDateTime))
		return -1;
	else
		return 1;
}

int RotateBackups(wchar_t *backupfolder, wchar_t *dbname)
{
	backupFile *bf = NULL, *bftmp;
	HANDLE hFind;
	wchar_t backupfolderTmp[MAX_PATH];
	WIN32_FIND_DATA FindFileData;

	if (options.num_backups == 0)
		return 0; /* Roration disabled. */
	mir_snwprintf(backupfolderTmp, L"%s\\%s*", backupfolder, dbname);
	hFind = FindFirstFile(backupfolderTmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	int i = 0;
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		bftmp = (backupFile*)mir_realloc(bf, ((i + 1) * sizeof(backupFile)));
		if (bftmp == NULL)
			goto err_out;
		bf = bftmp;
		wcsncpy_s(bf[i].Name, FindFileData.cFileName, _TRUNCATE);
		bf[i].CreationTime = FindFileData.ftCreationTime;
		i ++;
	} while (FindNextFile(hFind, &FindFileData));
	if (i > 0)
		qsort(bf, i, sizeof(backupFile), Comp); /* Sort the list of found files by date in descending order. */
	for (; i >= options.num_backups; i --) {
		mir_snwprintf(backupfolderTmp, L"%s\\%s", backupfolder, bf[(i - 1)].Name);
		DeleteFile(backupfolderTmp);
	}
err_out:
	FindClose(hFind);
	mir_free(bf);
	return 0;
}


int Backup(wchar_t *backup_filename)
{
	bool bZip = false;
	wchar_t dbname[MAX_PATH], source_file[MAX_PATH] = { 0 }, dest_file[MAX_PATH];
	HWND progress_dialog = NULL;
	SYSTEMTIME st;

	Profile_GetNameW(_countof(dbname), dbname);

	if (backup_filename == NULL) {
		int err;
		wchar_t *backupfolder, buffer[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = _countof(buffer);

		bZip = options.use_zip != 0;
		backupfolder = Utils_ReplaceVarsW(options.folder);
		// ensure the backup folder exists (either create it or return non-zero signifying error)
		err = CreateDirectoryTreeW(backupfolder);
		if (err != ERROR_ALREADY_EXISTS && err != 0) {
			mir_free(backupfolder);
			return 1;
		}
		RotateBackups(backupfolder, dbname);

		GetLocalTime(&st);
		GetComputerName(buffer, &size);
		mir_snwprintf(dest_file, L"%s\\%s_%02d.%02d.%02d@%02d-%02d-%02d_%s.%s", backupfolder, dbname, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buffer, bZip ? L"zip" : L"dat");
		mir_free(backupfolder);
	}
	else {
		wcsncpy_s(dest_file, backup_filename, _TRUNCATE);
		if (!mir_wstrcmp(wcsrchr(backup_filename, '.'), L".zip"))
			bZip = true;
	}
	if (!options.disable_popups)
		ShowPopup(dbname, TranslateT("Backup in progress"), NULL);

	if (!options.disable_progress)
		progress_dialog = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);

	SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Copying database file..."));

	mir_snwprintf(source_file, L"%s\\%s", profilePath, dbname);
	wchar_t *pathtmp = Utils_ReplaceVarsW(source_file);
	BOOL res = 0;
	if (bZip)
	{
		res = options.backup_profile 
			? MakeZip_Dir(_T2A(profilePath), dest_file, _T2A(dbname), progress_dialog) 
			: MakeZip(pathtmp, dest_file, dbname, progress_dialog);
	}
	else
		res = CopyFile(pathtmp, dest_file, 0);
	if (res) {
		if (!bZip) { // Set the backup file to the current time for rotator's correct  work
			FILETIME ft;
			HANDLE hFile = CreateFile(dest_file, FILE_WRITE_ATTRIBUTES, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetSystemTime(&st);
			SystemTimeToFileTime(&st, &ft);
			SetFileTime(hFile, NULL, NULL, &ft);
			CloseHandle(hFile);
		}
		SendDlgItemMessage(progress_dialog, IDC_PROGRESS, PBM_SETPOS, (WPARAM)(100), 0);
		UpdateWindow(progress_dialog);
		db_set_dw(0, "AutoBackups", "LastBackupTimestamp", (DWORD)time(0));

		if (options.use_dropbox)
		{
			DropboxUploadInfo ui = { dest_file, L"Backups" };
			if (CallService(MS_DROPBOX_UPLOAD, NULL, (LPARAM)&ui))
				ShowPopup(TranslateT("Uploading to Dropbox failed"), TranslateT("Error"), nullptr);
		}

		if (!options.disable_popups) {
			size_t dest_file_len = mir_wstrlen(dest_file);
			wchar_t *puText;

			if (dest_file_len > 50) {
				size_t i;
				puText = (wchar_t*)mir_alloc(sizeof(wchar_t) * (dest_file_len + 2));
				for (i = (dest_file_len - 1); dest_file[i] != '\\'; i--)
					;
				//wcsncpy_s(dest_file, backup_filename, _TRUNCATE);
				mir_wstrncpy(puText, dest_file, (i + 2));
				mir_wstrcat(puText, L"\n");
				mir_wstrcat(puText, (dest_file + i + 1));
			}
			else
				puText = mir_wstrdup(dest_file);

			// Now we need to know, which folder we made a backup. Let's break unnecessary variables :)
			while (dest_file[--dest_file_len] != L'\\')
				;
			dest_file[dest_file_len] = 0;
			ShowPopup(puText, TranslateT("Database backed up"), dest_file);
			mir_free(puText);
		}
	}
	else
		DeleteFile(dest_file);
	mir_free(pathtmp);

	DestroyWindow(progress_dialog);
	return 0;
}

void BackupThread(void *backup_filename)
{
	Backup((wchar_t*)backup_filename);
	InterlockedExchange(&m_state, 0); /* Backup done. */
	mir_free(backup_filename);
}

void BackupStart(wchar_t *backup_filename)
{
	wchar_t *tm = NULL;
	LONG cur_state;

	cur_state = InterlockedCompareExchange(&m_state, 1, 0);
	if (cur_state != 0) { /* Backup allready in process. */
		ShowPopup(TranslateT("Database back up in process..."), TranslateT("Error"), NULL); /* Show error message :) */
		return;
	}
	if (backup_filename != NULL)
		tm = mir_wstrdup(backup_filename);
	if (mir_forkthread(BackupThread, (void*)tm) == INVALID_HANDLE_VALUE) {
		InterlockedExchange(&m_state, 0); /* Backup done. */
		mir_free(tm);
	}
}

VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	time_t t = time(NULL);
	time_t diff = t - (time_t)db_get_dw(0, "AutoBackups", "LastBackupTimestamp", 0);
	if (diff > (time_t)(options.period * (options.period_type == PT_MINUTES ? 60 : (options.period_type == PT_HOURS ? (60 * 60) : (60 * 60 * 24)))))
		BackupStart(NULL);
}

int SetBackupTimer(void)
{
	if (timer_id != 0) {
		KillTimer(0, timer_id);
		timer_id = 0;
	}
	if (options.backup_types & BT_PERIODIC)
		timer_id = SetTimer(0, 0, (1000 * 60), TimerProc);
	return 0;
}
