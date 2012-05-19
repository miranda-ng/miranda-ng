#include "common.h"
#include "extern.h"

// write data needed by the external process, and restart miranda
// returns 1 if any error, 0 if shutdown is imminent
int ExternProcess(bool restart) 
{
	//HWND hWndMiranda = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	// spawn a process that will:
	//		-- wait for miranda to exit
	//		-- move downloaded plugins from the temp folder to the Plugins folder, possibly backing up old ones
	//		-- restart miranda

	char msg[1024];
	mir_snprintf(msg, SIZEOF(msg), "spawning external process, restart = %s", restart ? "true" : "false");
	NLog(msg);

	TCHAR data_filename[MAX_PATH];
	mir_sntprintf(data_filename, SIZEOF(data_filename), _T("%s\\ud_data.txt"), options.data_folder);

	// write data to file for external process to use
	HANDLE hDatFile = CreateFile(data_filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
	if (hDatFile == INVALID_HANDLE_VALUE) 
	{
		//MessageBox(0, TranslateT("Could not create data file for restart."), TranslateT("Error"), MB_OK | MB_ICONERROR);
		ShowError(TranslateT("Could not create data file for restart"));
		return 1;
	}

	TCHAR db_pathT[MAX_PATH];

	if (CallService(MS_SYSTEM_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(0,9,0,12))
	{
		TCHAR *p = Utils_ReplaceVarsT(_T("%miranda_profile%\\%miranda_profilename%\\%miranda_profilename%.dat"));
		mir_sntprintf(db_pathT, SIZEOF(db_pathT), _T("%s"), p);
		mir_free(p);
	}
	else
	{
		char db_name[100], db_path[MAX_PATH];

		// Get Miranda exe path and profile
		CallService(MS_DB_GETPROFILENAME, SIZEOF(db_name), (WPARAM)db_name);
		CallService(MS_DB_GETPROFILEPATH, SIZEOF(db_path), (WPARAM)db_path);

		TCHAR *t1, *t2;
		mir_sntprintf(db_pathT, SIZEOF(db_pathT), _T("%s\\%s"), (t1=mir_a2t(db_path)), (t2=mir_a2t(db_name)));
		mir_free(t1); mir_free(t2);
	}

	// get plugin folder
	TCHAR* plugins_folder = Utils_ReplaceVarsT(_T("%miranda_path%\\Plugins"));

	TCHAR mir_exe[MAX_PATH];
	GetModuleFileName(NULL, mir_exe, SIZEOF(mir_exe));

	unsigned long bytes_written;
	static const TCHAR tnl[] = _T("\r\n");

	WriteFile(hDatFile, mir_exe, (unsigned)_tcslen(mir_exe) * sizeof(TCHAR), &bytes_written, FALSE); 
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, plugins_folder, (unsigned)_tcslen(plugins_folder) * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, db_pathT, (unsigned)_tcslen(db_pathT) * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, options.temp_folder, (unsigned)_tcslen(options.temp_folder) * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);
	if(options.backup)
		WriteFile(hDatFile, options.backup_folder, (unsigned)_tcslen(options.backup_folder) * sizeof(TCHAR), &bytes_written, FALSE);
	else
		WriteFile(hDatFile, _T("no backups"),(unsigned) _tcslen(_T("no backups")) * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);

	TCHAR buf[64];
	mir_sntprintf(buf, SIZEOF(buf), _T("%d"), (unsigned int)GetCurrentProcessId());
	//MessageBox(0, buf, "Writing process id", MB_OK);
	WriteFile(hDatFile, buf, (unsigned)_tcslen(buf) * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);
	
	mir_sntprintf(buf, SIZEOF(buf), restart ? _T("restart") : _T("no_restart"));
	WriteFile(hDatFile, buf, (unsigned)_tcslen(buf) * sizeof(TCHAR), &bytes_written, FALSE);
	WriteFile(hDatFile, tnl, 2 * sizeof(TCHAR), &bytes_written, FALSE);

	CloseHandle(hDatFile);

	mir_free(plugins_folder);

	/*
	if(!CallService(MS_SYSTEM_OKTOEXIT,0,0)) {
		DeleteFile(data_filename);
		MessageBox(0, TranslateT("Miranda's not 'OK TO EXIT'."), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return;
	}
	*/

	TCHAR szParams[MAX_PATH], szBuf[MAX_PATH], szProcDir[MAX_PATH];

	// try to fire up external process from new dll (if present), so we can overwrite the old one
	mir_sntprintf(szBuf, SIZEOF(szBuf), _T("%s\\plugins\\updater.dll"), options.temp_folder);
	if (_taccess(szBuf, 0))
	{
		mir_sntprintf(szBuf, SIZEOF(szBuf), _T("%s\\updater.dll"), options.temp_folder);
		if (_taccess(szBuf, 0))
		{
			GetModuleFileName(hInst, szBuf, MAX_PATH);
		}
	}

	GetRootDir(szProcDir); _tcscat(szProcDir, _T("\\"));

	if (IsWinVerXPPlus())
	{
		mir_sntprintf(szParams, SIZEOF(szParams), _T("RUNDLL32.EXE \"%s\",ExternalUpdate %s"), szBuf, data_filename);
	}
	else
	{
		TCHAR* p = _tcsrchr(szBuf, _T('\\')); if (p) *p = 0;

		// rundll32 hates spaces in the <dll name> arg, but quotes aren't allowed in earlier versions...
		// GetShortPath can return paths with spaces (at least on XP with 8.3 filenames disabled)...
		// so we must 'CreateProcess' with the updater.dll location as the startup directory and pass only updater.dll as the arg

		mir_sntprintf(szParams, SIZEOF(szParams), _T("RUNDLL32.EXE .\\plugins\\updater.dll,ExternalUpdate %s"), data_filename);
	}

	BOOL res;

	if (IsAdminRequired())
	{
		SHELLEXECUTEINFO info = {0};
		info.cbSize = sizeof(info);

		TCHAR *p = _tcschr(szParams, ' '); if (p) *(p++) = 0;

		info.lpVerb = _T("runas");
		info.lpFile = szParams;
		info.lpParameters = p;
		info.lpDirectory = szProcDir;
		info.nShow = SW_HIDE;
		
		res = ShellExecuteEx(&info);
		info.cbSize = sizeof(info);

	}
	else
	{
		PROCESS_INFORMATION pi = {0};
		STARTUPINFO si = {0};
		si.cb = sizeof(si);

		res = CreateProcess(0, szParams, 0, 0, 0, 
			CREATE_NO_WINDOW | DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 
			0, szProcDir, &si, &pi);
		
		if (res)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
	}

	if (res)
	{
		PostMessage((HWND)CallService(MS_CLUI_GETHWND, 0, 0), WM_COMMAND, ID_ICQ_EXIT, 0);
	}
	else
	{
		TCHAR msg[256];
		mir_sntprintf(msg, SIZEOF(msg), _T("Error code: %d"), GetLastError());
		MessageBox(0, msg, TranslateT("CreateProcess"), MB_OK | MB_ICONERROR);
	} 

	return !res;
}

#ifdef _UD_LOGGING
void mWriteFile(HANDLE hFile, char *line) {
	unsigned long bytes_written;
	const char *nl = "\r\n";

	WriteFile(hFile, line, (unsigned)strlen(line), &bytes_written, FALSE);
	WriteFile(hFile, nl, 2, &bytes_written, FALSE);
}

void mWriteFile(HANDLE hFile, wchar_t *line) {
	unsigned long bytes_written;
	const char *nl = "\r\n";

	char buf[267];
	WideCharToMultiByte(CP_ACP, 0, line, -1, buf, SIZEOF(buf), NULL, NULL);

	WriteFile(hFile, buf, (unsigned)strlen(buf), &bytes_written, FALSE);
	WriteFile(hFile, nl, 2, &bytes_written, FALSE);
}
#endif

// move all files in src_folder to dst_folder - put replaced files in backup folder
// if a file in src_folder is a directory, copy it's contents to the same dir in the root folder and
// set that dir as the new root (so that dirs in dirs go in the right place)
void MoveFiles(HANDLE hLogFile, TCHAR *src_folder, TCHAR *dst_folder, TCHAR *backup_folder, TCHAR *root_folder)
{
	// move files from src_folder to dst_folder

	if(!src_folder || _tcslen(src_folder) == 0) {
		MessageBox(0, _T("Your 'temporary files' folder is set to NULL. Install aborted."), _T("Updater Error"), MB_OK | MB_ICONERROR);
		return;
	}

	TCHAR szFilesPath[MAX_PATH], szOldFileName[MAX_PATH], szNewFileName[MAX_PATH], szBackupFileName[MAX_PATH];

	bool do_backups = backup_folder ? (_tcscmp(backup_folder, _T("no backups")) != 0) : false;

	// ensure the destination folder exists
	if (!CreatePath(dst_folder))
		return;

	_sntprintf(szFilesPath, SIZEOF(szFilesPath), _T("%s\\*.*"), src_folder); 
	szFilesPath[SIZEOF(szFilesPath) - 1] = 0;

	bool move_file;

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(szFilesPath, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (findData.cFileName[0] != '.') 
			{

				_sntprintf(szOldFileName, SIZEOF(szOldFileName), _T("%s\\%s"), src_folder, findData.cFileName); 
				szOldFileName[SIZEOF(szOldFileName) - 1] = 0;

				if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				{
					// use szNewFileName as destination and new root folder
					if (_tcslen(findData.cFileName) < SIZEOF(ROOT_FILES_FOLDER) - 1 || _tcsicmp(findData.cFileName, ROOT_FILES_FOLDER)) 
					{
						_sntprintf(szNewFileName, SIZEOF(szNewFileName), _T("%s\\%s"), root_folder, findData.cFileName); 
						szNewFileName[SIZEOF(szNewFileName) - 1] = 0;
					}
					else
						_tcscpy(szNewFileName, root_folder);

					// recurse
					MoveFiles(hLogFile, szOldFileName, szNewFileName, backup_folder, szNewFileName);

				}
				else 
				{
					// exception for langpack files - move to root_folder
					// exception for dbtool.exe (e.g. translated) - move to root_folder
					if((_tcsnicmp(findData.cFileName, _T("dbtool.exe"), _tcslen(_T("dbtool.exe"))) == 0)
						|| (_tcsnicmp(findData.cFileName, _T("langpack_"), _tcslen(_T("langpack_"))) == 0))
					{
						_sntprintf(szNewFileName, SIZEOF(szNewFileName), _T("%s\\%s"), root_folder, findData.cFileName); 
						szNewFileName[SIZEOF(szNewFileName) - 1] = 0;
					}
					else
					{
						_sntprintf(szNewFileName, SIZEOF(szNewFileName), _T("%s\\%s"), dst_folder, findData.cFileName); 
						szNewFileName[SIZEOF(szNewFileName) - 1] = 0;
					}

					move_file = false;
					if (do_backups) 
					{
						_sntprintf(szBackupFileName, SIZEOF(szBackupFileName), _T("%s\\%s"), backup_folder, findData.cFileName); 
						szBackupFileName[SIZEOF(szBackupFileName) - 1] = 0;

						move_file = true;
						DeleteFile(szBackupFileName);
						if(!MoveFile(szNewFileName, szBackupFileName)) 
						{
						//	MessageBox(0, szNewFileName, __T("Could not backup!"), MB_OK | MB_ICONWARNING);
						}
					} 
					else 
					{
						move_file = true;
						if (!DeleteFile(szNewFileName)) 
						{
						//	MessageBox(0, szNewFileName, _T("Could not delete!"), MB_OK | MB_ICONWARNING);
						}
					}

					if (move_file) 
					{
						if (!MoveFile(szOldFileName, szNewFileName)) 
						{
							//MessageBox(0, szOldFileName, _T("Could not move!"), MB_OK | MB_ICONWARNING);

							// try a copy - possibly win98 etc. will not move the updater.dll when it is being used by this process
							CopyFile(szOldFileName, szNewFileName, FALSE);
							DeleteFile(szOldFileName); // docs say it is marked for delete and actually removed when the last handle is closed...hmm
						}
					} 
					else 
						DeleteFile(szOldFileName);
				}
			}
		} 
		while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}

	RemoveDirectory(src_folder);
}

bool ReadTLine(HANDLE hDatFile, TCHAR *line, int bsize, int &offset) {
	unsigned long bytes_read;
	BOOL bResult;
	while((bResult = ReadFile(hDatFile, line + offset, sizeof(TCHAR), &bytes_read, 0)) && offset < bsize && bytes_read == sizeof(TCHAR) && line[offset] && (line[offset] != _T('\n') || (offset > 0 && line[offset - 1] != _T('\r')))) offset++;

#ifndef _UNICODE
	if(offset == 1 && line[1] == 0) {
		wchar_t wline[MAX_PATH];
		wline[0] = *(wchar_t *)line;

		while((bResult = ReadFile(hDatFile, wline + offset, sizeof(wchar_t), &bytes_read, 0)) && offset < bsize && bytes_read == sizeof(wchar_t) && wline[offset] && (wline[offset] != L'\n' || (offset > 0 && wline[offset - 1] != L'\r'))) offset++;
		if(offset > 0) wline[offset - 1] = 0; // cut off /r/n

		WideCharToMultiByte(CP_ACP, 0, wline, -1, line, bsize, 0, 0);
	}
#endif
	if(offset > 0) line[offset - 1] = 0; // cut off /r/n
	return true;
}

void CALLBACK ExternalUpdate(HWND hwnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow) 
{
	//MessageBox(0, _T("ExternalUpdate"), _T("Updater"), MB_OK);
	HANDLE hDatFile = CreateFileA(lpszCmdLine, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(hDatFile == INVALID_HANDLE_VALUE) {
		char msg[1024];
		_snprintf(msg, SIZEOF(msg), "Could not read data file:\n%s", lpszCmdLine);
		MessageBoxA(0, msg, "Updater Error", MB_OK | MB_ICONERROR);
	} else {

		TCHAR *mir_exe = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR)),
			  *plugins_folder = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR)), 
			  *temp_folder = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR)),
			  *backup_folder = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR)),
			  *root_folder = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR)),
			  *db_path = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR)), 
			  *pid = (TCHAR *)malloc(64 * sizeof(TCHAR));
	
		bool restart = true;

		{
			int i = 0, offset;
			TCHAR line[MAX_PATH];
			BOOL bResult = TRUE;
			do {
				offset = 0;
				ReadTLine(hDatFile, line, MAX_PATH, offset);

				switch(i) {
					case 0: _tcsncpy(mir_exe, line, MAX_PATH); break;
					case 1: _tcsncpy(plugins_folder, line, MAX_PATH); break;
					case 2: _tcsncpy(db_path, line, MAX_PATH); break;
					case 3: _tcsncpy(temp_folder, line, MAX_PATH); break;
					case 4: _tcsncpy(backup_folder, line, MAX_PATH); break;
					case 5: _tcsncpy(pid, line, 64); break;
					case 6: 
						restart = (_tcsncmp(line, _T("restart"), 7) == 0); 
						offset = 0; // end loop
						break;
					default:
						offset = 0; // end loop
				}
				i++;
			} while(offset > 0);
		}
		CloseHandle(hDatFile);
// use data file to log to
#ifndef _UD_LOGGING
		DeleteFileA(lpszCmdLine);
#else
		HANDLE hDatFile = CreateFileA(lpszCmdLine, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
#endif

#ifdef _UD_LOGGING
		mWriteFile(hDatFile, "Inside external process..."); 
#endif

		_tcscpy(root_folder, mir_exe);
		TCHAR *p = _tcsrchr(root_folder, _T('\\')); if (p) *p = 0;

		// ensure miranda has exited
		DWORD mpi = (DWORD)_ttol(pid);

		bool exited = false;

#ifdef _UD_LOGGING
		char logmsg[1024];
		sprintf(logmsg, "Opening process #%d...", mpi);
		mWriteFile(hDatFile, logmsg); 
#endif

		HANDLE hMiranda = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, mpi);
		if(hMiranda) {
			int mbFlags, idRetry, idCancel, idContinue;
			const int MAX_SIZE = 2048;
			TCHAR message[MAX_SIZE];
			int exitStatus;
			
			OSVERSIONINFO vi = {0};
			vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&vi);
			_tcsncpy(message, _T("Miranda did not exit - cannot install or restart.\n"), MAX_SIZE);
			
			if (vi.dwMajorVersion = 5) {
				//windows 2000+
				mbFlags = 0x00000006L; //MB_CANCELTRYCONTINUE;
				idRetry = 10; //IDTRYAGAIN
				idCancel = IDCANCEL;
				idContinue = 11; //IDCONTINUE
				_tcsncat(message, _T("Press 'Try again' to check Miranda's status again, press 'Continue' to kill the process or press 'Cancel' to abort."), MAX_SIZE);
			} else { 
				//windows 98, me
				mbFlags = MB_ABORTRETRYIGNORE;
				idRetry = IDRETRY;
				idCancel = IDCANCEL;
				idContinue = IDIGNORE;
				_tcsncat(message, _T("Press 'Retry' to check Miranda's status again, press 'Ignore' to kill the process or press 'Abort' to abort."), MAX_SIZE);
			}
			
			while ((exitStatus = WaitForSingleObjectEx(hMiranda, 20 * 1000, FALSE)) == WAIT_TIMEOUT) {
				int res = MessageBox(0, message, _T("Updater Error"), mbFlags | MB_ICONERROR); 

				if (res == idContinue) {
					//if the user chooses Continue then kill the application
					TerminateProcess(hMiranda, 1);
					if((exitStatus = WaitForSingleObjectEx(hMiranda, 5 * 1000, FALSE)) == WAIT_TIMEOUT) {
						//hMiranda = OpenProcess(SYNCHRONIZE, FALSE, mpi);
						//if(hMiranda) {
						//CloseHandle(hMiranda);
						MessageBox(0, _T("It seems Miranda is still running. Aborting update."), _T("Updater Error"), MB_OK | MB_ICONERROR);
					} else {
#ifdef _UD_LOGGING
						mWriteFile(hDatFile, "Wait for miranda processs to 'Terminate' interrupted - assuming it exited"); 
#endif
					}
				} else {
					if (res == idRetry) {
						//if the user selected 'Try again' then wait a bit more.
						continue; //wait again
					}
				}
				
				break; //don't update anymore (happens when user choses 'Continue' or 'Cacel'
			}
#ifdef _UD_LOGGING
				mWriteFile(hDatFile, "Wait for miranda processs interrupted - assuming it exited"); 
#endif
				exited = (exitStatus != WAIT_TIMEOUT);
				CloseHandle(hMiranda);
		} else {
#ifdef _UD_LOGGING
			mWriteFile(hDatFile, "Could not open miranda processs - assuming it exited"); 
#endif
			//MessageBox(0, "Could not open Miranda process", "Update Error", MB_OK | MB_ICONERROR);
			exited = true;
		}

		if(exited) {
#ifdef _UD_LOGGING
			mWriteFile(hDatFile, "Miranda exited - moving files"); 
			MoveFiles(hDatFile, temp_folder, plugins_folder, backup_folder, root_folder);
#else
			MoveFiles(0, temp_folder, plugins_folder, backup_folder, root_folder);
			RemoveDirectory(temp_folder);
#endif
			// move files

			// restart miranda
			if(restart) {
#ifdef _UD_LOGGING
				mWriteFile(hDatFile, "Restarting"); 
#endif
				TCHAR szArgs[MAX_PATH];
				//wsprintf(szArgs, "\"%s\" \"%s\"", db_path, db_name);
				_sntprintf(szArgs, SIZEOF(szArgs), _T("\"%s\" \"%s\""), mir_exe, db_path); // includes name, dummy instead of executable?
				//wsprintf(szArgs, "\"%s\"", db_name);


				PROCESS_INFORMATION pi = {0};
				STARTUPINFO si = {0};
				si.cb = sizeof(si);

				if (!CreateProcess(mir_exe, szArgs, 0, 0, 0, DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) 
				{
					MessageBox(0, _T("Failed to restart Miranda"), _T("Updater Error"), MB_OK | MB_ICONERROR);
					//MessageBox(0, szArgs, mir_exe, MB_OK);
				}
				else
				{
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
				//ShellExecute(0, 0, mir_exe, szArgs, 0, SW_NORMAL);
			}
		}
#ifdef _UD_LOGGING
		else mWriteFile(hDatFile, "Miranda did not exit"); 
#endif

		free(pid);
		free(db_path);
		free(root_folder);
		free(backup_folder);
		free(temp_folder);
		free(plugins_folder);
		free(mir_exe);

#ifdef _UD_LOGGING
		CloseHandle(hDatFile);
#endif
	}

	FreeLibraryAndExitThread(hInstance, TRUE);
}
