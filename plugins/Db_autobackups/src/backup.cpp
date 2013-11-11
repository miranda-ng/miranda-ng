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
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)1);
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
	for (s = sorce, r = ret; *s && r - ret < MAX_PATH; s++, r++){
		if (*s != _T('\\'))
			*r = *s;
		else{
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

	char* tmp = mir_u2a(tszSource);

	ptrA szSourceName(mir_strdup(strrchr(tmp, '\\') + 1));
	ptrT tszDestPath(DoubleSlash((TCHAR*)tszDest));

	mir_free(tmp);

	WIN32_FILE_ATTRIBUTE_DATA fad = {0};
	if ( GetFileAttributesEx( tszSource, GetFileExInfoStandard, &fad ) )
	{
		SYSTEMTIME st;
		FileTimeToLocalFileTime(  &fad.ftLastWriteTime, &fad.ftLastWriteTime );
		FileTimeToSystemTime( &fad.ftLastWriteTime, &st );

		HANDLE hSrc = CreateFile( tszSource, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hSrc != INVALID_HANDLE_VALUE )
		{
			if ( zipFile hZip = zipOpen2_64(tszDestPath, APPEND_STATUS_CREATE, NULL, NULL) )
			{
				zip_fileinfo fi = {0};
				fi.tmz_date.tm_sec = st.wSecond;
				fi.tmz_date.tm_min = st.wMinute;
				fi.tmz_date.tm_hour = st.wHour;
				fi.tmz_date.tm_mday = st.wDay;
				fi.tmz_date.tm_mon = st.wMonth;
				fi.tmz_date.tm_year = st.wYear;

				int res = zipOpenNewFileInZip( hZip, szSourceName, &fi, NULL, 0, NULL, 0, "",  Z_DEFLATED, Z_BEST_COMPRESSION );
				if ( res == ZIP_OK )
				{
					DWORD buf_length = 256 * 1024;	// 256 KB
					HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);
					UINT i = 0;
					if ( void* buf = mir_alloc( buf_length ) )
					{
						for (;;)
						{
							DWORD read = 0;
							if ( ! ReadFile( hSrc, buf, buf_length, &read, NULL ) )
								break;

							if ( read == 0 )
							{
								// EOF
								ret = true;
								break;
							}

							res = zipWriteInFileInZip( hZip, buf, read );
							if ( res != ZIP_OK )
								break;

							SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100 / ((int)fad.nFileSizeLow / buf_length) * ++i), 0);
						}					

						mir_free( buf );
					}
					zipCloseFileInZip( hZip );
				}
				char szComment[128];
				mir_snprintf(szComment, SIZEOF(szComment), "Miranda NG Database\r\nCreated by: %s %d.%d.%d.%d\r\n", __PLUGIN_NAME, __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM); 
				zipClose( hZip, szComment);
			}
			CloseHandle( hSrc );
		}
	}

	return ret;
}

INT_PTR DBSaveAs(WPARAM wParam, LPARAM lParam)
{
	TCHAR fname_buff[MAX_PATH], szFilter[128];
	OPENFILENAME ofn = {0};
	CallService(MS_DB_GETPROFILENAMET,MAX_PATH,(LPARAM)fname_buff);

	int i = mir_sntprintf(szFilter, 64, _T("%s (*.dat)"), TranslateT("Miranda databases")) + 1;
	_tcscpy(szFilter + i, _T("*.dat")); 
	i += 6;
	i += mir_sntprintf(szFilter + i, 48, _T("%s (*.*)"), TranslateT("All Files")) + 1;
	_tcscpy(szFilter + i, _T("*"));
	szFilter[i + 2] = 0;

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = fname_buff;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = _T("dat");

	if (GetSaveFileName(&ofn))
		mir_forkthread(BackupThread, (void*)fname_buff);

	return 0;
}

struct FileNameFound_Tag
{
	TCHAR Name[MAX_PATH];
	FILETIME CreationTime;
}FileNameFound;

int RotateBackups(DWORD start_time)
{
	TCHAR backupfilename1[MAX_PATH] = {0}, backupfolderTmp[MAX_PATH] = {0};
	unsigned int i = 0;
	HWND hProgBar = GetDlgItem(progress_dialog, IDC_PROGRESS);
	MSG msg;

	WIN32_FIND_DATA FindFileData;

	TCHAR *backupfolder = Utils_ReplaceVarsT(options.folder);

	mir_sntprintf(backupfolderTmp, SIZEOF(backupfolderTmp), _T("%s\\*"), backupfolder);
	HANDLE hFind = FindFirstFile(backupfolderTmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
		return 0;
	_tcscpy(FileNameFound.Name, _T(""));
	while (FindNextFile(hFind, &FindFileData))
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		else if (_tcsicmp(&FindFileData.cFileName[_tcslen(FindFileData.cFileName)-4], _T(".bak")) == 0 || _tcsicmp(&FindFileData.cFileName[_tcslen(FindFileData.cFileName)-4], _T(".zip")) == 0)
		{
			if (_tcsicmp(FileNameFound.Name, _T("")) == 0)
			{
				_tcscpy(FileNameFound.Name, FindFileData.cFileName);
				FileNameFound.CreationTime = FindFileData.ftCreationTime;
			}
			else if ((FindFileData.ftCreationTime.dwHighDateTime < FileNameFound.CreationTime.dwHighDateTime) || (FindFileData.ftCreationTime.dwHighDateTime == FileNameFound.CreationTime.dwHighDateTime && FindFileData.ftCreationTime.dwLowDateTime < FileNameFound.CreationTime.dwLowDateTime))
			{
				_tcscpy(FileNameFound.Name, FindFileData.cFileName);
				FileNameFound.CreationTime = FindFileData.ftCreationTime;
			}
			while(PeekMessage(&msg, progress_dialog, 0, 0, PM_REMOVE) != 0)
			{
				if (!IsDialogMessage(progress_dialog, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			SendMessage(hProgBar, PBM_SETPOS, (WPARAM)(100 / options.num_backups * ++i), 0);
			//UpdateWindow(progress_dialog);
		}
	}

	FindClose(hFind);
	if (i >= options.num_backups)
	{
		mir_sntprintf(backupfilename1, MAX_PATH, _T("%s\\%s"), backupfolder, FileNameFound.Name);
		DeleteFile(backupfilename1);
	}
	mir_free(backupfolder);
	return 0;
}

void BackupThread(void* backup_filename)
{
	Backup((TCHAR*)backup_filename);
}

int Backup(TCHAR* backup_filename)
{
	TCHAR source_file[MAX_PATH] = {0}, dest_file[MAX_PATH] = {0};
	DWORD start_time = GetTickCount();
	BOOL bZip = FALSE;

	CallService(MS_DB_GETPROFILENAMET, MAX_PATH, (LPARAM)dbname);

	if (backup_filename == NULL)
	{
		SYSTEMTIME st;
		TCHAR buffer[MAX_COMPUTERNAME_LENGTH+1];
		DWORD size = sizeof(buffer);
		bZip = options.use_zip;

		TCHAR *backupfolder = Utils_ReplaceVarsT(options.folder);
		// ensure the backup folder exists (either create it or return non-zero signifying error)
		int err = CreateDirectoryTreeT(backupfolder);
		if (err != ERROR_ALREADY_EXISTS && err != 0) {
			return 1;
		}

		GetLocalTime(&st);
		GetComputerName(buffer, &size);
		mir_sntprintf(dest_file, MAX_PATH, _T("%s\\%s_%02d.%02d.%02d@%02d-%02d-%02d_%s.%s"), backupfolder, dbname, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buffer, bZip? _T("zip") : _T("bak"));
		mir_free(backupfolder);
	}
	else
		lstrcpyn(dest_file, backup_filename, MAX_PATH);

	if (!options.disable_popups)
		ShowPopup(dbname, TranslateT("Backup in progress"));

	if (!options.disable_progress) {
		progress_dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);
		SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Rotating backup files..."));
	}

	RotateBackups(start_time);

	SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Copying database file..."));
	SendMessage(GetDlgItem(progress_dialog, IDC_PROGRESS), PBM_SETPOS, (WPARAM)0, 0);
	UpdateWindow(progress_dialog);

	mir_sntprintf(source_file, MAX_PATH, _T("%s\\%s"), profilePath, dbname);
	TCHAR *pathtmp = Utils_ReplaceVarsT(source_file);
	BOOL res = 0;
	if(bZip)
		res = MakeZip(pathtmp, dest_file);
	else
		res = CopyFile(pathtmp, dest_file, 0);
	if (res)
	{
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
				for(i = (int)dest_file_len - 1; dest_file[i] != _T('\\'); i--);

				lstrcpyn(puText, dest_file, i + 2);
				lstrcat(puText, _T("\n"));
				lstrcat(puText, dest_file + i + 1);
			}
			else
				puText = mir_tstrdup(dest_file);

			ShowPopup(puText, TranslateT("Database backed up"));
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
