#include "headers.h"
#include "..\Zlib\src\zip.h"

TCHAR dbname[MAX_PATH];
HWND progress_dialog;

static UINT_PTR timer_id;

INT_PTR CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
		HWND prog = GetDlgItem(hwndDlg, IDC_PROGRESS);
		TranslateDialogDefault( hwndDlg );
		SendMessage(prog, PBM_SETPOS, 0, 0);
		}
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED && LOWORD( wParam ) == IDCANCEL ) {
			// in the progress dialog, use the user data to indicate that the user has pressed cancel
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 1);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
TCHAR* DoubleSlash(TCHAR *sorce)
{
	TCHAR *ret = (TCHAR*)mir_calloc(MAX_PATH);
	TCHAR *r, *s;
	for (s = sorce, r = ret; *s && r - ret < MAX_PATH - 1; s++, r++){
		if (*s != _T('\\'))
			*r = *s;
		else
		{
			*r = _T('\\');
			r++;
			*r = _T('\\');
		}
	}
	return ret;
}

bool MakeZip(LPCTSTR tszSource, LPCTSTR tszDest)
{
	bool ret = false;

	ptrA szSourceName(mir_u2a(dbname));
	ptrT tszDestPath(DoubleSlash((TCHAR*)tszDest));

	WIN32_FILE_ATTRIBUTE_DATA fad = {0};
	SYSTEMTIME st;
	GetFileAttributesEx(tszSource, GetFileExInfoStandard, &fad);
	FileTimeToLocalFileTime(&fad.ftLastWriteTime, &fad.ftLastWriteTime);
	FileTimeToSystemTime(&fad.ftLastWriteTime, &st);

	HANDLE hSrc = CreateFile( tszSource, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hSrc == INVALID_HANDLE_VALUE)
		return ret;

	if (zipFile hZip = zipOpen2_64(tszDestPath, APPEND_STATUS_CREATE, NULL, NULL))
	{
		zip_fileinfo fi = {0};
		fi.tmz_date.tm_sec = st.wSecond;
		fi.tmz_date.tm_min = st.wMinute;
		fi.tmz_date.tm_hour = st.wHour;
		fi.tmz_date.tm_mday = st.wDay;
		fi.tmz_date.tm_mon = st.wMonth-1;
		fi.tmz_date.tm_year = st.wYear;

		int res = zipOpenNewFileInZip( hZip, szSourceName, &fi, NULL, 0, NULL, 0, "",  Z_DEFLATED, Z_BEST_COMPRESSION );
		if (res == ZIP_OK)
		{
			DWORD buf_length = 256 * 1024;	// 256 KB
			HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);
			DWORD dwTotalBytes = 0;
			MSG msg;
			if (void* buf = mir_alloc( buf_length ))
			{
				while (GetWindowLongPtr(progress_dialog, GWLP_USERDATA) != 1)
				{
					DWORD dwRead = 0;
					if (!ReadFile(hSrc, buf, buf_length, &dwRead, NULL))
						break;

					if (dwRead == 0) // EOF
					{
						ret = true;
						break;
					}
					res = zipWriteInFileInZip(hZip, buf, dwRead);
					if (res != ZIP_OK)
						break;

					while(PeekMessage(&msg, progress_dialog, 0, 0, PM_REMOVE) != 0)
					{
						if (!IsDialogMessage(progress_dialog, &msg))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
					dwTotalBytes += dwRead;
					SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100.0 * double(dwTotalBytes) / double(fad.nFileSizeLow)), 0);
				}
				mir_free(buf);
			}
			zipCloseFileInZip(hZip);
		}
		char szComment[128];
		mir_snprintf(szComment, SIZEOF(szComment), "%s\r\n%s %s %d.%d.%d.%d\r\n", Translate("Miranda NG database"), Translate("Created by:"), __PLUGIN_NAME, __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
		zipClose( hZip, szComment);
	}
	CloseHandle(hSrc);
	return ret;
}

INT_PTR DBSaveAs(WPARAM wParam, LPARAM lParam)
{
	TCHAR fname_buff[MAX_PATH], tszFilter[200];
	OPENFILENAME ofn = {0};
	CallService(MS_DB_GETPROFILENAMET,MAX_PATH,(LPARAM)fname_buff);

	mir_sntprintf(tszFilter, SIZEOF(tszFilter), _T("%s (*.dat)%c*.dat%c%s (*.zip)%c*.zip%c%s (*.*)%c*%c"), 
		TranslateT("Miranda NG databases"), 0, 0, 
		TranslateT("Compressed Miranda NG databases"), 0, 0, 
		TranslateT("All files"), 0, 0);

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = fname_buff;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
	ofn.lpstrFilter = tszFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = _T("dat");

	if (GetSaveFileName(&ofn))
		mir_forkthread(BackupThread, (void*)fname_buff);

	return 0;
}

struct backupFile
{
	TCHAR Name[MAX_PATH];
	FILETIME CreationTime;
};

int Comp(const void *i, const void *j)
{
	backupFile *pi = (backupFile*)i;
	backupFile *pj = (backupFile*)j;

	if (pi->CreationTime.dwHighDateTime > pj->CreationTime.dwHighDateTime ||  (pi->CreationTime.dwHighDateTime == pj->CreationTime.dwHighDateTime && pi->CreationTime.dwLowDateTime > pj->CreationTime.dwLowDateTime))
		return -1;
	else
		return 1;
}

int RotateBackups()
{
	TCHAR backupfilename1[MAX_PATH] = {0}, backupfolderTmp[MAX_PATH] = {0};
	unsigned int i = 0;
	WIN32_FIND_DATA FindFileData;
	TCHAR *backupfolder = Utils_ReplaceVarsT(options.folder);

	mir_sntprintf(backupfolderTmp, SIZEOF(backupfolderTmp), _T("%s\\%s*"), backupfolder, dbname);
	HANDLE hFind = FindFirstFile(backupfolderTmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	backupFile *bf = (backupFile*)mir_calloc(sizeof(backupFile));

	while(bf)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (FindNextFile(hFind, &FindFileData))
				continue;
			else break;
		}
		_tcscpy(bf[i].Name, FindFileData.cFileName);
		bf[i].CreationTime = FindFileData.ftCreationTime;
		if (FindNextFile(hFind, &FindFileData))
			bf = (backupFile*)mir_realloc(bf, (++i + 1) * sizeof(backupFile));
		else break;
	}
	FindClose(hFind);
	if (i > 0)
		qsort(bf, i + 1, sizeof(backupFile), Comp); //Sort the list of found files by date in descending order

	for(;i >= options.num_backups - 1; i--){
		mir_sntprintf(backupfilename1, MAX_PATH, _T("%s\\%s"), backupfolder, bf[i].Name);
		DeleteFile(backupfilename1);
	}
	mir_free(backupfolder);
	mir_free(bf);
	return 0;
}

void BackupThread(void* backup_filename)
{
	Backup((TCHAR*)backup_filename);
}

int Backup(TCHAR* backup_filename)
{
	TCHAR source_file[MAX_PATH] = { 0 }, dest_file[MAX_PATH] = { 0 };
	bool bZip = false;

	CallService(MS_DB_GETPROFILENAMET, MAX_PATH, (LPARAM)dbname);

	if (backup_filename == NULL)
	{
		TCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = sizeof(buffer);
		bZip = options.use_zip != 0;

		TCHAR *backupfolder = Utils_ReplaceVarsT(options.folder);
		// ensure the backup folder exists (either create it or return non-zero signifying error)
		int err = CreateDirectoryTreeT(backupfolder);
		if (err != ERROR_ALREADY_EXISTS && err != 0)
			return 1;

		SYSTEMTIME st;
		GetLocalTime(&st);
		GetComputerName(buffer, &size);
		mir_sntprintf(dest_file, MAX_PATH, _T("%s\\%s_%02d.%02d.%02d@%02d-%02d-%02d_%s.%s"), backupfolder, dbname, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buffer, bZip ? _T("zip") : _T("dat"));
		mir_free(backupfolder);
		RotateBackups();
	}
	else
	{
		lstrcpyn(dest_file, backup_filename, MAX_PATH);
		if (!_tcscmp(_tcsrchr(backup_filename, _T('.')), _T(".zip")))
			bZip = true;
	}
	if (!options.disable_popups)
		ShowPopup(dbname, TranslateT("Backup in progress"), NULL);

	if (!options.disable_progress)
		progress_dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);

	SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Copying database file..."));

	mir_sntprintf(source_file, MAX_PATH, _T("%s\\%s"), profilePath, dbname);
	TCHAR *pathtmp = Utils_ReplaceVarsT(source_file);
	BOOL res = 0;
	if (bZip)
		res = MakeZip(pathtmp, dest_file);
	else
		res = CopyFile(pathtmp, dest_file, 0);
	if (res)
	{
		if (!bZip)
		{ // Set the backup file to the current time for rotator's correct  work
			FILETIME ft;
			SYSTEMTIME st;
			HANDLE hFile = CreateFile(dest_file, FILE_WRITE_ATTRIBUTES, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetSystemTime(&st);
			SystemTimeToFileTime(&st, &ft);
			SetFileTime(hFile, NULL, NULL, &ft);
			CloseHandle(hFile);
		}
		SendMessage(GetDlgItem(progress_dialog, IDC_PROGRESS), PBM_SETPOS, (WPARAM)(100), 0);
		UpdateWindow(progress_dialog);
		db_set_dw(0, "AutoBackups", "LastBackupTimestamp", (DWORD)time(0));
		if (!options.disable_popups)
		{
			size_t dest_file_len = lstrlen(dest_file);
			TCHAR *puText;
			if (dest_file_len > 50)
			{
				int i;
				puText = (TCHAR*)mir_alloc(sizeof(TCHAR) * (dest_file_len + 2));
				for (i = (int)dest_file_len - 1; dest_file[i] != _T('\\'); i--);

				lstrcpyn(puText, dest_file, i + 2);
				lstrcat(puText, _T("\n"));
				lstrcat(puText, dest_file + i + 1);
			}
			else
				puText = mir_tstrdup(dest_file);

			// Now we need to know, which folder we made a backup. Let's break unnecessary variables :)
			while (dest_file[--dest_file_len] != L'\\');
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

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	time_t t = time(NULL);
	time_t diff = t - (time_t)db_get_dw(0, "AutoBackups", "LastBackupTimestamp", (DWORD)t);
	if (diff > (time_t)(options.period * (options.period_type == PT_MINUTES ? 60 : (options.period_type == PT_HOURS ? 60 * 60 : 60 * 60 * 24 ))))
		mir_forkthread(BackupThread, NULL);
}

int SetBackupTimer(void)
{
	if (options.backup_types & BT_PERIODIC)
	{
		if (timer_id == 0)
			timer_id = SetTimer(0, timer_id, 1000 * 60, TimerProc);
	}
	else if (timer_id != 0)
	{
		KillTimer(0, timer_id);
		timer_id = 0;
	}
	return 0;
}

INT_PTR ABService(WPARAM wParam, LPARAM lParam)
{
	mir_forkthread(BackupThread, (void*)wParam);
	return 0;
}
