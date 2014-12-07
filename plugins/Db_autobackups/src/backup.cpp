#include "headers.h"
#include "..\Zlib\src\zip.h"


static UINT_PTR	timer_id = 0;
static LONG	m_state = 0;


LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
	{
		TCHAR* ptszPath = (TCHAR*)PUGetPluginData(hWnd);
		if (ptszPath != 0)
			ShellExecute(0, _T("open"), ptszPath, NULL, NULL, SW_SHOW);

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
void ShowPopup(TCHAR* ptszText, TCHAR* ptszHeader, TCHAR* ptszPath)
{
	POPUPDATAT ppd = { 0 };

	_tcsncpy_s(ppd.lptzText, ptszText, _TRUNCATE);
	_tcsncpy_s(ppd.lptzContactName, ptszHeader, _TRUNCATE);
	if (ptszPath != NULL)
		ppd.PluginData = (void*)mir_tstrdup(ptszPath);
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = Skin_GetIconByHandle(iconList[0].hIcolib);

	PUAddPopupT(&ppd);
}

INT_PTR CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg, IDC_PROGRESS), PBM_SETPOS, 0, 0);
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
TCHAR* DoubleSlash(TCHAR *sorce)
{
	TCHAR *ret, *r, *s;

	ret = (TCHAR*)mir_alloc((MAX_PATH * sizeof(TCHAR)));
	if (ret == NULL)
		return NULL;
	for (s = sorce, r = ret; *s && (r - ret) < (MAX_PATH - 1); s ++, r ++) {
		if (*s != _T('\\'))
			*r = *s;
		else {
			*r = _T('\\');
			r++;
			*r = _T('\\');
		}
	}
	r[0] = 0;
	return ret;
}

bool MakeZip(TCHAR *tszSource, TCHAR *tszDest, TCHAR *dbname, HWND progress_dialog)
{
	bool ret = false;
	HANDLE hSrc;
	zipFile hZip;
	SYSTEMTIME st;
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	zip_fileinfo fi = { 0 };
	HWND hProgBar;
	DWORD dwRead;
	MSG msg;
	char buf[(256 * 1024)];	// 256 KB
	DWORDLONG dwSrcFileSize, dwTotalBytes = 0;

	ptrA szSourceName(mir_u2a(dbname));
	ptrT tszDestPath(DoubleSlash(tszDest));

	hSrc = CreateFile(tszSource, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSrc == INVALID_HANDLE_VALUE)
		return ret;
	if (GetFileAttributesEx(tszSource, GetFileExInfoStandard, &fad) == FALSE)
		goto err_out;
	dwSrcFileSize = ((DWORDLONG)fad.nFileSizeLow | (((DWORDLONG)fad.nFileSizeHigh) << 32));
	if (dwSrcFileSize == 0) /* Prevent division by zero error. */
		goto err_out;
	FileTimeToLocalFileTime(&fad.ftLastWriteTime, &fad.ftLastWriteTime);
	FileTimeToSystemTime(&fad.ftLastWriteTime, &st);
	hZip = zipOpen2_64(tszDestPath, APPEND_STATUS_CREATE, NULL, NULL);
	if (hZip == NULL)
		goto err_out;
	fi.tmz_date.tm_sec = st.wSecond;
	fi.tmz_date.tm_min = st.wMinute;
	fi.tmz_date.tm_hour = st.wHour;
	fi.tmz_date.tm_mday = st.wDay;
	fi.tmz_date.tm_mon = (st.wMonth - 1);
	fi.tmz_date.tm_year = st.wYear;

	if (zipOpenNewFileInZip(hZip, szSourceName, &fi, NULL, 0, NULL, 0, "",  Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK) {
		hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);
		while (GetWindowLongPtr(progress_dialog, GWLP_USERDATA) != 1) {
			if (!ReadFile(hSrc, buf, sizeof(buf), &dwRead, NULL))
				break;
			if (dwRead == 0) { // EOF
				ret = true;
				break;
			}
			if (zipWriteInFileInZip(hZip, buf, dwRead) != ZIP_OK)
				break;
			dwTotalBytes += dwRead;
			SendMessage(hProgBar, PBM_SETPOS, (WPARAM)((100 * dwTotalBytes) / dwSrcFileSize), 0);
			while (PeekMessage(&msg, progress_dialog, 0, 0, PM_REMOVE) != 0) {
				if (!IsDialogMessage(progress_dialog, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		zipCloseFileInZip(hZip);
	}
	if (ret) {
		mir_snprintf(buf, SIZEOF(buf), "%s\r\n%s %s %d.%d.%d.%d\r\n",
			Translate("Miranda NG database"), Translate("Created by:"),
			__PLUGIN_NAME, __MAJOR_VERSION,	__MINOR_VERSION, __RELEASE_NUM,	__BUILD_NUM);
	} else {
		buf[0] = 0;
	}
	zipClose(hZip, buf);

err_out:
	CloseHandle(hSrc);

	return ret;
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

	if (pi->CreationTime.dwHighDateTime > pj->CreationTime.dwHighDateTime ||
	    (pi->CreationTime.dwHighDateTime == pj->CreationTime.dwHighDateTime && pi->CreationTime.dwLowDateTime > pj->CreationTime.dwLowDateTime))
		return -1;
	else
		return 1;
}

int RotateBackups(TCHAR *backupfolder, TCHAR *dbname)
{
	size_t i = 0;
	backupFile *bf = NULL, *bftmp;
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	TCHAR backupfolderTmp[MAX_PATH];

	mir_sntprintf(backupfolderTmp, SIZEOF(backupfolderTmp), _T("%s\\%s*"), backupfolder, dbname);
	hFind = FindFirstFile(backupfolderTmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		goto err_out;
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		i ++;
		bftmp = (backupFile*)mir_realloc(bf, ((i + 1) * sizeof(backupFile)));
		if (bftmp == NULL)
			goto err_out;
		bf = bftmp;
		_tcsncpy_s(bf[i].Name, FindFileData.cFileName, _TRUNCATE);
		bf[i].CreationTime = FindFileData.ftCreationTime;
	} while (FindNextFile(hFind, &FindFileData));

	if (i > 0)
		qsort(bf, (i + 1), sizeof(backupFile), Comp); //Sort the list of found files by date in descending order
	for (; i >= (options.num_backups - 1); i --) {
		mir_sntprintf(backupfolderTmp, SIZEOF(backupfolderTmp), _T("%s\\%s"), backupfolder, bf[i].Name);
		DeleteFile(backupfolderTmp);
	}
err_out:
	FindClose(hFind);
	mir_free(bf);
	return 0;
}


int Backup(TCHAR* backup_filename)
{
	bool bZip = false;
	TCHAR dbname[MAX_PATH], source_file[MAX_PATH] = { 0 }, dest_file[MAX_PATH];
	HWND progress_dialog;
	SYSTEMTIME st;

	CallService(MS_DB_GETPROFILENAMET, SIZEOF(dbname), (LPARAM)dbname);

	if (backup_filename == NULL) {
		int err;
		TCHAR *backupfolder, buffer[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = SIZEOF(buffer);

		bZip = options.use_zip != 0;
		backupfolder = Utils_ReplaceVarsT(options.folder);
		// ensure the backup folder exists (either create it or return non-zero signifying error)
		err = CreateDirectoryTreeT(backupfolder);
		if (err != ERROR_ALREADY_EXISTS && err != 0) {
			mir_free(backupfolder);
			return 1;
		}
		RotateBackups(backupfolder, dbname);

		GetLocalTime(&st);
		GetComputerName(buffer, &size);
		mir_sntprintf(dest_file, MAX_PATH, _T("%s\\%s_%02d.%02d.%02d@%02d-%02d-%02d_%s.%s"), backupfolder, dbname, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buffer, bZip ? _T("zip") : _T("dat"));
		mir_free(backupfolder);
	} else {
		_tcsncpy_s(dest_file, backup_filename, _TRUNCATE);
		if (!_tcscmp(_tcsrchr(backup_filename, _T('.')), _T(".zip")))
			bZip = true;
	}
	if (!options.disable_popups)
		ShowPopup(dbname, TranslateT("Backup in progress"), NULL);

	if (!options.disable_progress)
		progress_dialog = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);

	SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Copying database file..."));

	mir_sntprintf(source_file, SIZEOF(source_file), _T("%s\\%s"), profilePath, dbname);
	TCHAR *pathtmp = Utils_ReplaceVarsT(source_file);
	BOOL res = 0;
	if (bZip)
		res = MakeZip(pathtmp, dest_file, dbname, progress_dialog);
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
		SendMessage(GetDlgItem(progress_dialog, IDC_PROGRESS), PBM_SETPOS, (WPARAM)(100), 0);
		UpdateWindow(progress_dialog);
		db_set_dw(0, "AutoBackups", "LastBackupTimestamp", (DWORD)time(0));
		if (!options.disable_popups) {
			size_t dest_file_len = mir_tstrlen(dest_file);
			TCHAR *puText;
			if (dest_file_len > 50) {
				size_t i;
				puText = (TCHAR*)mir_alloc(sizeof(TCHAR) * (dest_file_len + 2));
				for (i = (dest_file_len - 1); dest_file[i] != _T('\\'); i--)
					;
				//_tcsncpy_s(dest_file, backup_filename, _TRUNCATE);
				mir_tstrncpy(puText, dest_file, (i + 2));
				mir_tstrcat(puText, _T("\n"));
				mir_tstrcat(puText, (dest_file + i + 1));
			} else
				puText = mir_tstrdup(dest_file);

			// Now we need to know, which folder we made a backup. Let's break unnecessary variables :)
			while (dest_file[-- dest_file_len] != L'\\')
				;
			dest_file[dest_file_len] = 0;
			ShowPopup(puText, TranslateT("Database backed up"), dest_file);
			mir_free(puText);
		}
	} else
		DeleteFile(dest_file);
	mir_free(pathtmp);

	DestroyWindow(progress_dialog);
	return 0;
}

void BackupThread(void *backup_filename)
{
	Backup((TCHAR*)backup_filename);
	InterlockedExchange((volatile LONG*)&m_state, 0); /* Backup done. */
	mir_free(backup_filename);
}

void BackupStart(TCHAR *backup_filename)
{
	TCHAR *tm = NULL;
	LONG cur_state;

	cur_state = InterlockedCompareExchange((volatile LONG*)&m_state, 1, 0);
	if (cur_state != 0) { /* Backup allready in process. */
		ShowPopup(TranslateT("Database back up in process..."), TranslateT("Error"), NULL); /* Show error message :) */
		return;
	}
	if (backup_filename != NULL)
		tm = mir_tstrdup(backup_filename);
	if (mir_forkthread(BackupThread, (void*)tm) == INVALID_HANDLE_VALUE) {
		InterlockedExchange((volatile LONG*)&m_state, 0); /* Backup done. */
		mir_free(tm);
	}
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
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
