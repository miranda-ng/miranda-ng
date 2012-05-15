#include "headers.h"
#include <commctrl.h>

TCHAR dbname[MAX_PATH];

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

INT_PTR DBSaveAs(WPARAM wParam, LPARAM lParam)
{
	HWND progress_dialog = 0;
	TCHAR fname_buff[MAX_PATH], szFilter[128];
	int i;
	OPENFILENAME ofn = {0};
	CallService(MS_DB_GETPROFILENAMET,MAX_PATH,(LPARAM)fname_buff);

	i = mir_sntprintf(szFilter, 64, _T("%s (*.dat)"), TranslateT("Miranda Databases")) + 1;
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
		Backup(fname_buff);

	return 0;
}

struct FileNameFound_Tag
{
	TCHAR Name[MAX_PATH];
	FILETIME CreationTime;
}FileNameFound;

int RotateBackups(HWND progress_dialog, DWORD start_time)
{
	TCHAR backupfilename1[MAX_PATH] = {0}, backupfilename2[MAX_PATH] = {0}, backupfolderTmp[MAX_PATH] = {0};
	TCHAR* backupfolder;
	unsigned int i = 0;
	HWND prog = GetDlgItem(progress_dialog, IDC_PROGRESS);
	MSG msg;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	backupfolder = Utils_ReplaceVarsT(options.folder);
	
	mir_sntprintf(backupfolderTmp, SIZEOF(backupfolderTmp), _T("%s\\*"), backupfolder);
	hFind = FindFirstFile(backupfolderTmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
		return 0;
	_tcscpy(FileNameFound.Name, _T(""));
	while (FindNextFile(hFind, &FindFileData))
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		else if (_tcsicmp(&FindFileData.cFileName[_tcslen(FindFileData.cFileName)-4], _T(".bak")) == 0)
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
			i++;
			while(PeekMessage(&msg, progress_dialog, 0, 0, PM_REMOVE) != 0)
			{
				if(!IsDialogMessage(progress_dialog, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(100  * (options.num_backups - i) / options.num_backups), 0);
			UpdateWindow(progress_dialog);
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

int Backup(TCHAR* backup_filename)
{
	TCHAR source_file[MAX_PATH] = {0}, dest_file[MAX_PATH] = {0};
	TCHAR* backupfolder,* pathtmp,* puText;
	HWND progress_dialog;
	DWORD start_time = GetTickCount();
	int i;
	size_t dest_file_len;

	CallService(MS_DB_GETPROFILENAMET, MAX_PATH, (LPARAM)dbname);

	if (backup_filename == NULL)
	{
		int err = 0;

		SYSTEMTIME st;
		TCHAR buffer[MAX_COMPUTERNAME_LENGTH+1];
		DWORD size = sizeof(buffer);

		backupfolder = Utils_ReplaceVarsT(options.folder);
		// ensure the backup folder exists (either create it or return non-zero signifying error)
		err = CreateDirectoryTree(backupfolder);
		if(err != ERROR_ALREADY_EXISTS && err != 0) {
			return 1;
		}

		GetLocalTime(&st);
		GetComputerName(buffer, &size);
		mir_sntprintf(dest_file, MAX_PATH, _T("%s\\%s_%02d.%02d.%02d@%02d-%02d-%02d_%s.bak"), backupfolder, dbname, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buffer);
		mir_free(backupfolder);
	}
	else
		lstrcpyn(dest_file, backup_filename, MAX_PATH);

	if(!options.disable_popups)
		ShowPopup(dbname, TranslateT("Backup in Progress"));

	if(!options.disable_progress) {
		progress_dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, (DLGPROC)DlgProcProgress);
		SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Rotating backup files..."));
	}

	RotateBackups(progress_dialog, start_time);

	SetDlgItemText(progress_dialog, 0xDAED, TranslateT("Copying database file..."));
	SendMessage(progress_dialog, PBM_SETPOS, (WPARAM)(int)(0), 0);
	UpdateWindow(progress_dialog);

	mir_sntprintf(source_file, MAX_PATH, _T("%s\\%s"), profilePath, dbname);
	pathtmp = Utils_ReplaceVarsT(source_file);
	if (CopyFile(pathtmp, dest_file, 0))
	{
		SendMessage(progress_dialog, PBM_SETPOS, (WPARAM)(int)(100), 0);
		UpdateWindow(progress_dialog);
		DBWriteContactSettingDword(0, "AutoBackups", "LastBackupTimestamp", (DWORD)time(0));
		if(!options.disable_popups)
		{
			dest_file_len = lstrlen(dest_file);
			if(dest_file_len > 50)
			{
				puText = mir_alloc(sizeof(TCHAR) * (dest_file_len + 2));
				for(i = (int)dest_file_len - 1; dest_file[i] != _T('\\'); i--);

				lstrcpyn(puText, dest_file, i + 2);
				lstrcat(puText, _T("\n"));
				lstrcat(puText, dest_file + i + 1);
			}
			else
				puText = mir_tstrdup(dest_file);

			ShowPopup(puText, TranslateT("Database backuped"));
			mir_free(puText);
		}
	}
	else
		DeleteFile(dest_file);
	mir_free(pathtmp);

	DestroyWindow(progress_dialog);
	return 0;
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	time_t t = time(0), diff = t - (time_t)DBGetContactSettingDword(0, "AutoBackups", "LastBackupTimestamp", (DWORD)t);
	if(diff > (time_t)(options.period * (options.period_type == PT_MINUTES ? 60 : (options.period_type == PT_HOURS ? 60 * 60 : 60 * 60 * 24 ) )))
		Backup(NULL);
}

int SetBackupTimer(void)
{
	if(options.backup_types & BT_PERIODIC)
	{
		if(timer_id == 0)
			timer_id = SetTimer(0, 0, 1000 * 60, TimerProc);
	} 
	else if(timer_id != 0) 
	{
		KillTimer(0, timer_id);
		timer_id = 0;
	}
	return 0;
}

INT_PTR ABService(WPARAM wParam, LPARAM lParam)
{
	Backup((TCHAR*)wParam);
	return 0;
}
