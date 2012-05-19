#include "common.h"
#include "services.h"

UpdateList update_list;
CRITICAL_SECTION list_cs;

DWORD daily_timer_id, startup_timer_id;
HANDLE hMainMenuItem, hGroupMenuItem;
HANDLE hStartupDone;

bool checking;

#define NUM_SERVICES		13
HANDLE hService[NUM_SERVICES];

int CompareFileNameStruct(const FileNameStruct *p1, const FileNameStruct *p2)
{
    return p1->file_id - p2->file_id;
}

VOID CALLBACK CheckTimerProcDaily(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if(options.check_daily)
		CallService(MS_UPDATE_CHECKFORUPDATES, (WPARAM)(is_idle && options.no_conf_idle), 0);
}

// returns true if any downloaded dll is active
bool DownloadUpdates(UpdateList &todo, FilenameMap *map, bool dlls_only) {

	bool dll_enabled_or_langpack = false;

	bool use_popup = options.popup_notify && ArePopupsEnabled();
	
	// iterate through the updates we need to check

	if (use_popup) 
	{
		ShowPopup(0, TranslateT("Downloading Updates"), _T(""), POPFLAG_SAVEHWND, -1);
	} 
	else 
	{
		CreateProgressWindow();

		SendMessage(hwndProgress, WM_SETTEXT, 0, (LPARAM)TranslateT("Progress - Downloading updates..."));
		SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Downloading"), 0);
	}

	TCHAR msg[512];
	TCHAR *temp_str;
	bool a_download_succeeded = false;

	for (int index = 0; index < todo.getCount(); index++) 
	{
		// remember if the user has decided not to install this version
		char stored_setting[256];
		mir_snprintf(stored_setting, SIZEOF(stored_setting), "DisabledVer%s", todo[index].update.szComponentName);
		DBVARIANT dbv;
		bool download = todo[index].update_options.enabled;
		if(!DBGetContactSettingString(0, "Updater", stored_setting, &dbv)) 
		{
			if(dbv.pszVal && strcmp(dbv.pszVal, todo[index].newVersion) == 0)
				download = false;
			else
				DBDeleteContactSetting(0, "Updater", stored_setting);
			DBFreeVariant(&dbv);
		}

		if(download) 
		{
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("Downloading plugin: %s"), (temp_str = GetTString(todo[index].update.szComponentName)));
			mir_free(temp_str);
		} 
		else 
		{
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("Skipping plugin: %s"), (temp_str = GetTString(todo[index].update.szComponentName)));
			mir_free(temp_str);
		}

		if (!use_popup) 
		{
			SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)msg, 0);
			PostMessage(hwndProgress, WMU_SETPROGRESS, (WPARAM)(int)(index * 100.0 / todo.getCount()), 0);
		} //else if(hwndPop) // disabled - just annoying
			//ChangePopupText(hwndPop, msg);


		if (download) 
		{
			bool got_file = false;
			if(todo[index].update_options.use_beta) 
			{
				// download from i->update.szBetaUpdateURL to temp folder
				got_file = GetFile(todo[index].update.szBetaUpdateURL, options.temp_folder, todo[index].update.szComponentName, todo[index].newVersion, dlls_only);
			} else {
				got_file = GetFile(todo[index].update.szUpdateURL, options.temp_folder, todo[index].update.szComponentName, todo[index].newVersion, dlls_only);
			}

			if(got_file) 
			{
				a_download_succeeded = true;
				if (todo[index].file_id != -1) 
				{
                    FileNameStruct* fns = map->find((FileNameStruct*)&todo[index].file_id);
					if (todo[index].cat == MC_PLUGINS || todo[index].cat == MC_UNKNOWN)
                        dll_enabled_or_langpack |= RearrangeDlls(todo[index].shortName, fns->list);
					else if(todo[index].cat == MC_LOCALIZATION) 
					{
						RearrangeLangpacks(todo[index].shortName, fns->list);
						dll_enabled_or_langpack = true;
					}
				} 
				else 
				{
					dll_enabled_or_langpack = true;
				}
			}
		}

		if (!use_popup && hwndProgress == 0)
		{
			RemoveFolder(options.temp_folder);
			break; // user closed progress window - cancel
		}
	}
	

	ProgressWindowDone();
	// postmessage here causes a lockup on exit! bah popups!!
	//if(hwndPop) PostMessage(hwndPop, WMU_CLOSEPOP, 0, 0);
	if (hwndPop) SendMessage(hwndPop, WMU_CLOSEPOP, 0, 0);

	if(!a_download_succeeded) 
	{
		for(int i = 0; i < todo.getCount(); ++i)
			free(todo[i].newVersion);
		todo.destroy();
	}

	return dll_enabled_or_langpack;
}

void __stdcall sttNotifyStartup( void* dwParam ) {
	NotifyEventHooks(hStartupDone, 0, 0);
}

void RestoreStatus() {
	//NotifyEventHooks(hStartupDone, 0, 0);
	// do this in a seperate thread, in case we're called from an event hook to prevent double-lock on core hook critical section (csHooks)
	CallFunctionAsync(sttNotifyStartup, NULL);

	if(options.start_offline) {
		// restore global status - only works on startup since we remove the SavedGlobalStatus parameter
		WORD clist_status = DBGetContactSettingWord(0, "CList", "Status", ID_STATUS_OFFLINE),
			saved_global_status = DBGetContactSettingWord(0, MODULE, "SavedGlobalStatus", clist_status);
		if(clist_status == ID_STATUS_OFFLINE && saved_global_status != clist_status) {
#ifdef DEBUG_POPUPS
			PUShowMessage("Updater: restoring status", SM_NOTIFY);
#endif
			CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)saved_global_status, 0);		
		}
	}
	DBDeleteContactSetting(0, MODULE, "SavedGlobalStatus");
}

bool WriteLastCheckTime(void)
{
	SYSTEMTIME now;
	GetSystemTime(&now);

	DBWriteContactSettingWord(0, MODULE, "LastUpdateDay", now.wDay);
	DBWriteContactSettingWord(0, MODULE, "LastUpdateMonth", now.wMonth);
	DBWriteContactSettingWord(0, MODULE, "LastUpdateYear", now.wYear);

	return true;
}

void CheckForUpdatesWorker(void *param)
{
	if (checking) return;
	/*
	// this check doesn't work on some systems - not sure which or why
	if(!(GetSystemMetrics(SM_NETWORK) & 1)) {
		ShowError(TranslateT("No network - aborting update check"));
		NLog("worker thread aborting - no network");
		return 1; // no network
	}
	*/

	NLog("CheckForUpdatesWorker thread starting");

	checking = true;

	WriteLastCheckTime();

	bool confirm = !(((DWORD)param & 1) == 1);
	bool restart = !(((DWORD)param & 2) == 2); // if restart is false, then we're doing an 'update and shutdown'

	if(hwndOptions) PostMessage(hwndOptions, WMU_CHECKING, 0, 0);

	bool use_popup = options.popup_notify && ArePopupsEnabled();
	
	FilenameMap fn_map(5, CompareFileNameStruct);
	
	if (use_popup) 
	{
		ShowPopup(0, TranslateT("Checking for Updates"), _T(""), POPFLAG_SAVEHWND, -1);
	} 
	else 
	{
		CreateProgressWindow();

		SendMessage(hwndProgress, WM_SETTEXT, 0, (LPARAM)TranslateT("Progress - Checking for updates..."));
		if (options.use_xml_backend) SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Downloading XML data"), 0);
		else SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Checking for updates"), 0);

	}
	
	EnterCriticalSection(&list_cs);

	if (options.use_xml_backend)	
    {
		if (UpdateXMLData(MC_PLUGINS)) // prevent double error messages (in some cases)
		{
			// iterate through the registered plugins
			if (!use_popup) SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Scanning plugins folder"), 0);
			ScanPlugins(&fn_map, &update_list);

			if (UpdateXMLData(MC_LOCALIZATION)) 
			{
				if (!use_popup) SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Scanning language packs"), 0);
				ScanLangpacks(&fn_map, &update_list);
			}

			if(!use_popup) SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Updating component file listing ids"), 0);
			UpdateFLIDs(update_list);
		}

		if (!use_popup) SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)TranslateT("Checking for updates"), 0);
	}

	UpdateList update_list2(update_list);
	LeaveCriticalSection(&list_cs);

	int count = update_list2.getCount(), index = 0;
	TCHAR msg[512];
	TCHAR *temp_str;

	UpdateList todo;

	for(index = 0; index < count; index++) 
	{
		if(update_list2[index].update_options.enabled) 
		{
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("Checking plugin: %s"), (temp_str = GetTString(update_list2[index].update.szComponentName)));
			mir_free(temp_str);
		} 
		else 
		{
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("Skipping plugin: %s"), (temp_str = GetTString(update_list2[index].update.szComponentName)));
			mir_free(temp_str);
		}

		if(!use_popup) {
			SendMessage(hwndProgress, WMU_SETMESSAGE, (WPARAM)msg, 0);
			SendMessage(hwndProgress, WMU_SETPROGRESS, (WPARAM)(int)(index * 100.0 / count), 0);
		} //else if(hwndPop) // disabled - just annoying
			//ChangePopupText(hwndPop, msg);

		if (update_list2[index].update_options.enabled) 
		{
			char *nv;
			bool beta;
			if (nv =  UpdateRequired(update_list2[index], &beta)) 
			{
				todo.insert(new UpdateInternal(update_list2[index]));			
				todo[todo.getCount()-1].newVersion = nv;
				todo[todo.getCount()-1].update_options.use_beta = beta;
			}
		}
		if (!use_popup && hwndProgress == NULL) 
		{
			RemoveFolder(options.temp_folder);
			break; // user closed progress window - cancel
		}
	}

	ProgressWindowDone();
	if (hwndPop) PostMessage(hwndPop, WMU_CLOSEPOP, 0 , 0);
	
	if(options.use_xml_backend) {
		FreeXMLData(MC_PLUGINS);
		FreeXMLData(MC_LOCALIZATION);
	}

	bool restore_status = true;

	if (todo.getCount()) 
	{
		int cd_ret = CD_OK;
		if (confirm) 
		{
			if (use_popup) 
			{
				ShowPopup(0, TranslateT("Updates Available"), TranslateT("Updated Miranda components detected.\nClick here to install."), POPFLAG_SAVEHWND, -1);
				DWORD ret;
				while ((ret = WaitForSingleObject(hEventPop, 200)) == WAIT_TIMEOUT && !Miranda_Terminated());

				if (!pop_cancelled && ret == WAIT_OBJECT_0 && !Miranda_Terminated()) 
				{
					cd_ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CONFIRMUPDATES), GetDesktopWindow(), DlgProcConfirm, (LPARAM)&todo);
				} 
				else 
				{
					if(hwndOptions) PostMessage(hwndOptions, WMU_DONECHECKING, 0, 0);
					RestoreStatus();
					for(int i=0; i<todo.getCount(); ++i)
						free(todo[i].newVersion);
					checking = false;
					Netlib_CloseHandle(hNetlibHttp);
					hNetlibHttp = NULL;
					return;
				}
			} 
			else
				cd_ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CONFIRMUPDATES), GetDesktopWindow(), DlgProcConfirm, (LPARAM)&todo);
		}



		if (!confirm || cd_ret == CD_CONFALL || cd_ret == CD_NOINSTALL || cd_ret == CD_OK) 
		{	
			bool conf_all = (cd_ret == CD_CONFALL), no_install = (cd_ret == CD_NOINSTALL);

			// ensure the backup folder exists (either create it or return non-zero signifying error)
			if (options.backup && !CreatePath(options.backup_folder)) 
			{
				//MessageBox(0, Translate("Could not create backup folder"), Translate("Error"), MB_OK | MB_ICONERROR);
				ShowError(TranslateT("Could not create backup folder"));
				if (hwndOptions) PostMessage(hwndOptions, WMU_DONECHECKING, 0, 0);
				RestoreStatus();
				for (int i=0; i<todo.getCount(); ++i)
					free(todo[i].newVersion);
				checking = false;
				Netlib_CloseHandle(hNetlibHttp);
				hNetlibHttp = NULL;
				return;
			}

			bool dll_enabled_or_langpack = DownloadUpdates(todo, &fn_map, confirm ? false : options.auto_dll_only);
			if (todo.getCount() && !no_install) 
			{
				if (!conf_all || DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIRMCOMPONENTS), GetDesktopWindow(), DlgProcConfirmComponents) == IDOK) 
				{
					if (!dll_enabled_or_langpack && restart)
					{ 
						// we're not doing an 'update and shutdown', and we're not updating any active dlls...so just install

						// get folders
						TCHAR root_folder[MAX_PATH], plugins_folder[MAX_PATH];

						GetRootDir(root_folder);

						_tcscpy(plugins_folder, root_folder);
						_tcscat(plugins_folder, _T("\\Plugins"));

						// move files
						MoveFiles(0, options.temp_folder, plugins_folder, options.backup_folder, root_folder);
						
						// rescan to get correct version numbers
						ScanPlugins(0, 0);	
					} 
					else 
					{
						if (ExternProcess(restart) == 0) // if restarting, don't restore status
							restore_status = false;
					}
				}
			}
		}

		for (int i=0; i<todo.getCount(); ++i)
			free(todo[i].newVersion);

	} 
	else if (!restart) 
	{
		PostMessage((HWND)CallService(MS_CLUI_GETHWND, 0, 0), WM_COMMAND, ID_ICQ_EXIT, 0);
	}	

	if (restore_status && restart) // restore status if we're not doing an 'update and shutdown', and the update was unsuccessful (or there was nothing to update, or no need to shutdown)
	{
		checking = false;
		if (hwndOptions) PostMessage(hwndOptions, WMU_DONECHECKING, 0, 0);
		RestoreStatus();
	}
	Netlib_CloseHandle(hNetlibHttp);
	hNetlibHttp = NULL;
}

int FindFileInList(const char *name)
{
	int res = -1;
	for (int i=0; i < update_list.getCount(); ++i)
	{
		if (strcmp(update_list[i].update.szComponentName, name) == 0)
		{
			res = i;
			break;
		}
	}
	return res;
}

INT_PTR Restart(WPARAM wParam, LPARAM lParam) {
	ExternProcess(true);
	return 0;
}

INT_PTR UpdateAndExit(WPARAM wParam, LPARAM lParam) {
	return CallService(MS_UPDATE_CHECKFORUPDATES, 0, 1);
}

// if wParam, don't confirm
// if lParam, install and shutdown but don't restart
INT_PTR CheckForUpdates(WPARAM wParam, LPARAM lParam) {
	if(daily_timer_id) KillTimer(0, daily_timer_id);
	daily_timer_id = SetTimer(0, 0, 24 * 60 * 60 * 1000, CheckTimerProcDaily);

	DWORD param = ((wParam ? 1 : 0) | (lParam ? 2 : 0));

	mir_forkthread(CheckForUpdatesWorker, (void*)param);		
	NLog("CheckForUpdates service called");

	return 0;
}

INT_PTR EnumerateUpdates(WPARAM wParam, LPARAM lParam) {
	UpdateEnumerateFunc func = (UpdateEnumerateFunc)wParam;

	EnterCriticalSection(&list_cs);
	for(int i=0; i<update_list.getCount(); ++i) {
		func(update_list[i].update.szComponentName, &(update_list[i].update_options), lParam);
	}
	LeaveCriticalSection(&list_cs);

	return 0;
}

void LoadUpdateOptions(char *szComponentName, UpdateOptions *update_options) {
	char buff[256];
	sprintf(buff, "Enable%s", szComponentName);
	update_options->enabled = (DBGetContactSettingByte(0, MODULE, buff, 1) == 1);
	if(!update_options->fixed) {
		sprintf(buff, "UseBeta%s", szComponentName);
		update_options->use_beta = (DBGetContactSettingByte(0, MODULE, buff, 0) == 1);
	}
}

void SaveUpdateOptions(char *szComponentName, UpdateOptions *update_options) {
	char buff[256];
	sprintf(buff, "Enable%s", szComponentName);
	DBWriteContactSettingByte(0, MODULE, buff, update_options->enabled ? 1 : 0);
	if(!update_options->fixed) {
		sprintf(buff, "UseBeta%s", szComponentName);
		DBWriteContactSettingByte(0, MODULE, buff, update_options->use_beta ? 1 : 0);
	}
}

INT_PTR SetUpdateOptions(WPARAM wParam, LPARAM lParam) {
	char *szComponentName = (char *)wParam;
	UpdateOptions *uo = (UpdateOptions *)lParam;
	TCHAR *temp1 = 0;

	bool found = false;
	EnterCriticalSection(&list_cs);
	for (int i=0; i<update_list.getCount(); ++i) {
		if(strcmp(update_list[i].update.szComponentName, szComponentName) == 0
			|| _tcscmp(temp1 = GetTString(update_list[i].update.szComponentName), (TCHAR *)szComponentName) == 0) // when set via options, szComponentName is translated and potentially already converted to unicode
		{
			found = true;
			update_list[i].update_options = *uo;
			SaveUpdateOptions(update_list[i].update.szComponentName, &update_list[i].update_options);
			if(update_list[i].file_id == -1 && !uo->use_beta) {
				update_list[i].file_id = CheckForFileID(update_list[i].update.szUpdateURL, update_list[i].update.szVersionURL, update_list[i].update.szComponentName);
			}
			break;
		}
	}
	LeaveCriticalSection(&list_cs);
	mir_free(temp1);

	return found ? 0 : 1;
}

INT_PTR GetUpdateOptions(WPARAM wParam, LPARAM lParam) {
	char *szComponentName = (char *)wParam;
	UpdateOptions *uo = (UpdateOptions *)lParam;
	TCHAR *temp1 = 0;

	bool found = false;
	EnterCriticalSection(&list_cs);
	for (int i=0; i<update_list.getCount(); ++i) {
		if(strcmp(update_list[i].update.szComponentName, szComponentName) == 0
			|| _tcscmp((temp1 = GetTString(update_list[i].update.szComponentName)), (TCHAR *)szComponentName) == 0) // when set via options, szComponentName is translated and potentially already converted to unicode
		{
			found = true;
			*uo = update_list[i].update_options;
			break;
		}
	}
	LeaveCriticalSection(&list_cs);
	mir_free(temp1);

	return found ? 0 : 1;
}

bool RegisterForFileListing(int file_id, PLUGININFO *pluginInfo, bool auto_register) 
{
	return RegisterForFileListing(file_id, pluginInfo->shortName, pluginInfo->version, auto_register, MC_PLUGINS);
}

bool IsRegistered(int file_id) 
{
	EnterCriticalSection(&list_cs);
	for (int i=0; i<update_list.getCount(); ++i) 
	{
		if (update_list[i].file_id == file_id) 
		{
			LeaveCriticalSection(&list_cs);
			return true; // plugin already registered
		}
	}
	LeaveCriticalSection(&list_cs);

	return false;
}

bool RegisterForFileListing(int file_id, const char *fl_name, DWORD version, bool auto_register, const Category cat) 
{
	// allow multiple registration of same plugin only if new plugin not automatically registered
	// if multiple registration of an auto registered plugin occurs, use newest file id and version
	EnterCriticalSection(&list_cs);
	int i = FindFileInList(fl_name);
	if (i >= 0 && !auto_register)
		update_list.remove(i);
	else if (i >= 0)
	{
		if (update_list[i].auto_register) 
		{
			update_list[i].file_id = file_id;		// in case plugin file id changes (i.e. scan from xml data will overwrite settings read from db on startup)
			char version_str[16];
			update_list[i].update.pbVersion = (BYTE *)safe_strdup(CreateVersionString(version, version_str));
			update_list[i].update.cpbVersion = (int)strlen(version_str);
		}
		LeaveCriticalSection(&list_cs);
		
		// plugin already registered - set file id if AUTOREGISTER
		if (update_list[i].update.szUpdateURL && strcmp(update_list[i].update.szUpdateURL, UPDATER_AUTOREGISTER) == 0) 
		{
			update_list[i].file_id = file_id;
			char *buff = (char *)safe_alloc(strlen(MIM_DOWNLOAD_URL_PREFIX) + 9);
			sprintf(buff, MIM_DOWNLOAD_URL_PREFIX "%d", file_id);
			update_list[i].update.szUpdateURL = buff;
			update_list[i].shortName = safe_strdup(update_list[i].update.szComponentName);

			if(update_list[i].update.szBetaVersionURL)
			{
				update_list[i].update_options.fixed = false;
				LoadUpdateOptions(update_list[i].update.szComponentName, &update_list[i].update_options);
			}
		}
		return false;
	}
	LeaveCriticalSection(&list_cs);

	UpdateInternal update_internal = {0};
	char version_str[16];
	char *buff;

	update_internal.cat = cat;
	update_internal.shortName = safe_strdup(fl_name);

	update_internal.update.szComponentName = safe_strdup(fl_name);
	update_internal.update.pbVersion = (BYTE *)safe_strdup(CreateVersionString(version, version_str));
	update_internal.update.cpbVersion = (int)strlen(version_str);

	buff = (char *)safe_alloc(strlen(MIM_DOWNLOAD_URL_PREFIX) + 9);
	sprintf(buff, MIM_DOWNLOAD_URL_PREFIX "%d", file_id);
	update_internal.update.szUpdateURL = buff;

	///////// only used when not using the xml backend ////////////	
	buff = (char *)safe_alloc(strlen("class=\"fileNameHeader\">") + strlen(fl_name) + 2);
	sprintf(buff, "class=\"fileNameHeader\">%s ", fl_name);
	update_internal.update.pbVersionPrefix = (BYTE *)buff;
	update_internal.update.cpbVersionPrefix = (int)strlen(buff);

	buff = (char *)safe_alloc(strlen(MIM_VIEW_URL_PREFIX) + 9);
	sprintf(buff, MIM_VIEW_URL_PREFIX "%d", file_id);
	update_internal.update.szVersionURL = buff;
	///////////////////////////////////////////////////////////////

	// same as register, except for fileID
	update_internal.file_id = file_id;
	update_internal.auto_register = auto_register;
	update_internal.update_options.fixed = true;
	update_internal.update_options.use_beta = false;

	LoadUpdateOptions(update_internal.update.szComponentName, &update_internal.update_options);

	EnterCriticalSection(&list_cs);
	update_list.insert(new UpdateInternal(update_internal));
	LeaveCriticalSection(&list_cs);

	return true;
}

void DoStartupProcess() {
	if(checking) return;

	if(OldXMLDataExists(MC_PLUGINS) && OldXMLDataExists(MC_LOCALIZATION)) {
		if(options.check_on_startup) {
			SYSTEMTIME now;
			GetSystemTime(&now);

			if(options.check_once) {
				if(DBGetContactSettingWord(0, MODULE, "LastUpdateDay", 32) == 32) {
					CallService(MS_UPDATE_CHECKFORUPDATES, 0, 0);
				} else {
					if(now.wDay != DBGetContactSettingWord(0, MODULE, "LastUpdateDay", now.wDay)
						|| now.wMonth != DBGetContactSettingWord(0, MODULE, "LastUpdateMonth", now.wMonth)
						|| now.wYear != DBGetContactSettingWord(0, MODULE, "LastUpdateYear", now.wYear))
					{
						CallService(MS_UPDATE_CHECKFORUPDATES, 0, 0);
					} else {
						if(LoadOldXMLData(MC_PLUGINS, false)) {
							ScanPlugins(0, 0);
							FreeXMLData(MC_PLUGINS);
						}
						if(LoadOldXMLData(MC_LOCALIZATION, false)) {
							ScanLangpacks(0, 0);
							FreeXMLData(MC_LOCALIZATION);
						}

						RestoreStatus();
					}
				}
			} else {
				CallService(MS_UPDATE_CHECKFORUPDATES, 0, 0);
			}
		} else {
			if(LoadOldXMLData(MC_PLUGINS, false)) {
				ScanPlugins(0, 0);
				FreeXMLData(MC_PLUGINS);
			}
			if(LoadOldXMLData(MC_LOCALIZATION, false)) {
				ScanLangpacks(0, 0);
				FreeXMLData(MC_LOCALIZATION);
			}

			RestoreStatus();
		}
	} else {
		if(options.check_on_startup) CallService(MS_UPDATE_CHECKFORUPDATES, 0, 0);
		else RestoreStatus();
	}

}


INT_PTR Register(WPARAM wParam, LPARAM lParam) {

	Update *update = (Update *)lParam;
	UpdateInternal update_internal = {0};

	// remove registered plugin if already there
	EnterCriticalSection(&list_cs);
	int i = FindFileInList(update->szComponentName);
	if (i >= 0) update_list.remove(i);
	LeaveCriticalSection(&list_cs);

	if(update->szComponentName == 0 || update->pbVersion == 0)
		return 1;
	if(update->szVersionURL == 0 && (update->szUpdateURL == 0 || strcmp(update->szUpdateURL, UPDATER_AUTOREGISTER) != 0) && update->szBetaVersionURL == 0) // both betas and normal updates disabled - complain
		return 1;

	update_internal.cat = MC_UNKNOWN;

	// duplicate all the data...sigh
	update_internal.update.szComponentName = safe_strdup(update->szComponentName);

	if(update->szVersionURL) update_internal.update.szVersionURL = safe_strdup(update->szVersionURL);
	if(update->szUpdateURL) update_internal.update.szUpdateURL = safe_strdup(update->szUpdateURL);

	// if revision update url is from the fl, and we can find the file_id, use xml data if available
	// otherwise set this to -1 to check url's specified
	if(update_internal.update.szUpdateURL)
		update_internal.file_id = CheckForFileID(update_internal.update.szUpdateURL, update_internal.update.szVersionURL, update_internal.update.szComponentName);
	else
		update_internal.file_id = -1;
	
	if(update_internal.file_id != -1) { // ensure correct format for file listing version string search data
		char *buff = (char *)safe_alloc(strlen("class=\"fileNameHeader\">") + strlen(update->szComponentName) + 2);
		sprintf(buff, "class=\"fileNameHeader\">%s ", update->szComponentName);
		update_internal.update.pbVersionPrefix = (BYTE *)buff;
		update_internal.update.cpbVersionPrefix = (int)strlen(buff);

		update_internal.shortName = safe_strdup(update->szComponentName);
	} else {
		if(update->pbVersionPrefix) update_internal.update.pbVersionPrefix = safe_bytedup(update->pbVersionPrefix, update->cpbVersionPrefix);
		update_internal.update.cpbVersionPrefix = update->cpbVersionPrefix;
	}

	// leave beta alone
	if(update->szBetaVersionURL) update_internal.update.szBetaVersionURL = safe_strdup(update->szBetaVersionURL);
	if(update->pbBetaVersionPrefix) update_internal.update.pbBetaVersionPrefix = safe_bytedup(update->pbBetaVersionPrefix, update->cpbBetaVersionPrefix);
	update_internal.update.cpbBetaVersionPrefix = update->cpbBetaVersionPrefix;
	if(update->szBetaUpdateURL) update_internal.update.szBetaUpdateURL = safe_strdup(update->szBetaUpdateURL);

	update_internal.update.pbVersion = safe_bytedup(update->pbVersion, update->cpbVersion);
	update_internal.update.cpbVersion = update->cpbVersion;

	if(update->cbSize > UPDATE_V1_SIZE && update->szBetaChangelogURL) 
		update_internal.update.szBetaChangelogURL = safe_strdup(update->szBetaChangelogURL);

	update_internal.update_options.fixed = (update->szVersionURL == 0 || strcmp(update->szUpdateURL, UPDATER_AUTOREGISTER) == 0 || update->szBetaVersionURL == 0); // set 'fixed' flag
	update_internal.update_options.use_beta = (update->szVersionURL == 0 || strcmp(update->szUpdateURL, UPDATER_AUTOREGISTER) == 0);
	LoadUpdateOptions(update_internal.update.szComponentName, &update_internal.update_options);

	EnterCriticalSection(&list_cs);
	update_list.insert(new UpdateInternal(update_internal));
	LeaveCriticalSection(&list_cs);

	//if(strcmp(update_internal.update.szComponentName, "My Details") == 0) {
	//	MessageBox(0, "My Details registered", "msg", MB_OK);
	//}

	return 0;
}

INT_PTR RegisterFL(WPARAM wParam, LPARAM lParam) {
	int file_id = (INT_PTR)wParam;
	PLUGININFO *pluginInfo  = (PLUGININFO *)lParam;

	// remove registered plugin if already there
	EnterCriticalSection(&list_cs);
	int i = FindFileInList(pluginInfo->shortName);
	if (i >= 0) update_list.remove(i);
	LeaveCriticalSection(&list_cs);

	RegisterForFileListing(file_id, pluginInfo, false);

	return 0;
}

INT_PTR Unregister(WPARAM wParam, LPARAM lParam) {
	char *szComponentName = (char *)lParam;

	// remove registered plugin if found
	EnterCriticalSection(&list_cs);
	int i = FindFileInList(szComponentName);
	if (i >= 0) update_list.remove(i);
	LeaveCriticalSection(&list_cs);

	return 0;
}

INT_PTR CheckForUpdatesTrgr(WPARAM wParam, LPARAM lParam) {
	if(wParam & ACT_PERFORM) {
#ifdef DEBUG_POPUPS
		PUShowMessage("Updater: triggered check", SM_NOTIFY);
#endif
		CallService(MS_UPDATE_CHECKFORUPDATES, (WPARAM)(is_idle && options.no_conf_idle), 0);
	}
	return 0;
}

int ServicesModulesLoaded2(WPARAM wParam, LPARAM lParam) {
	// do initial update etc. depending on options and time
	KillTimer(0, startup_timer_id);
	DoStartupProcess();
	return 0;
}

VOID CALLBACK StartupTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	KillTimer(0, startup_timer_id);
	ServicesModulesLoaded2(0, 0);
}

HANDLE hEventServicesModulesLoaded2;

int ServicesModulesLoaded(WPARAM wParam, LPARAM lParam) {
	// add main menu item

	CLISTMENUITEM menu = {0};

	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ALL | CMIF_ICONFROMICOLIB;
	menu.icolibItem = GetIconHandle(I_CHKUPD);

	menu.pszName = "Check for updates";
	menu.pszService= MS_UPDATE_CHECKFORUPDATES;
	menu.position = 500010000;

#ifdef MS_CLIST_ADDGROUPMENUITEM
	if (ServiceExists(MS_CLIST_ADDGROUPMENUITEM)) {
		GroupMenuParam gmp = {0};
		hGroupMenuItem = (HANDLE)CallService(MS_CLIST_ADDGROUPMENUITEM,(WPARAM)&gmp,(LPARAM)&menu);
	}
#endif
	hMainMenuItem = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);

	hEventServicesModulesLoaded2 = HookEvent(ME_SYSTEM_MODULESLOADED, ServicesModulesLoaded2);
	startup_timer_id = SetTimer(0, 0, 5000, StartupTimerProc);

	return 0;
}

INT_PTR EnableStatusControl(WPARAM wParam, LPARAM lParam)
{
	BOOL enable = (BOOL)wParam;

	options.start_offline = (enable == TRUE);

	return 0;
}

INT_PTR IsUpdateSupported(WPARAM wParam, LPARAM lParam) 
{
	char *szComponentName = (char *)lParam;

	EnterCriticalSection(&list_cs);
	bool res = FindFileInList(szComponentName) >= 0;
	LeaveCriticalSection(&list_cs);

	return res;
}

int ServicesPreShutdown(WPARAM wParam, LPARAM lParam) {
	Netlib_Shutdown(hNetlibHttp);
	return 0;
}

HANDLE hEventPreShutDown, hEventServicesModulesLoaded;
void InitServices() {

	InitializeCriticalSection(&list_cs);

	int i = 0;
	hService[i++] = CreateServiceFunction(MS_UPDATE_REGISTER, Register);
	hService[i++] = CreateServiceFunction(MS_UPDATE_REGISTERFL, RegisterFL);
	hService[i++] = CreateServiceFunction(MS_UPDATE_UNREGISTER, Unregister);
	hService[i++] = CreateServiceFunction(MS_UPDATE_CHECKFORUPDATESTRGR, CheckForUpdatesTrgr);
	hService[i++] = CreateServiceFunction(MS_UPDATE_CHECKFORUPDATES, CheckForUpdates);
	hService[i++] = CreateServiceFunction(MS_UPDATE_SETUPDATEOPTIONS, SetUpdateOptions);
	hService[i++] = CreateServiceFunction(MS_UPDATE_GETUPDATEOPTIONS, GetUpdateOptions);
	hService[i++] = CreateServiceFunction(MS_UPDATE_ENUMERATE, EnumerateUpdates);
	hService[i++] = CreateServiceFunction(MS_UPDATE_ENABLESTATUSCONTROL, EnableStatusControl);
	hService[i++] = CreateServiceFunction(MS_UPDATE_MENURESTART, Restart);
	hService[i++] = CreateServiceFunction(MS_UPDATE_MENUUPDATEANDEXIT, UpdateAndExit);
	hService[i++] = CreateServiceFunction(MS_UPDATE_MENUCHECKFORUPDATES, CheckForUpdates);
	hService[i++] = CreateServiceFunction(MS_UPDATE_ISUPDATESUPPORTED, IsUpdateSupported);

	hStartupDone = CreateHookableEvent(ME_UPDATE_STARTUPDONE);

	hEventPreShutDown = HookEvent(ME_SYSTEM_PRESHUTDOWN, ServicesPreShutdown);

	daily_timer_id = SetTimer(0, 0, 24 * 60 * 60 * 1000, CheckTimerProcDaily);

	hEventServicesModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ServicesModulesLoaded);
}

void DeinitServices() {
	UnhookEvent(hEventPreShutDown);
	UnhookEvent(hEventServicesModulesLoaded);
	UnhookEvent(hEventServicesModulesLoaded2);

	if(daily_timer_id) KillTimer(0, daily_timer_id);

	DestroyHookableEvent(hStartupDone);

	for(int i = 0; i < NUM_SERVICES; i++)
		if(hService[i])
			DestroyServiceFunction(hService[i]);

	DeleteCriticalSection(&list_cs);
	update_list.destroy();
}


