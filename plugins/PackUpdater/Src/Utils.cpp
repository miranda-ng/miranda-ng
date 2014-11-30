/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "Common.h"

vector<FILEINFO> Files;
BOOL DlgDld;
INT FileCount = 0, CurrentFile = 0, Number = 0;
BYTE Reminder, AutoUpdate;
BYTE UpdateOnStartup, UpdateOnPeriod, OnlyOnceADay, PeriodMeasure;
INT Period;
TCHAR tszDialogMsg[2048] = {0};
FILEINFO* pFileInfo = NULL;
FILEURL* pFileUrl = NULL;
HANDLE hCheckThread = NULL, hNetlibUser = NULL;
MYOPTIONS MyOptions = {0};
aPopups PopupsList[POPUPS];
LPCTSTR Title = {0}, Text = {0};

IconItem iconList[] =
{
	{ LPGEN("Check for pack updates"),    "check_update", IDI_MENU },
	{ LPGEN("Clear pack updates folder"), "empty_folder", IDI_DELETE },
	{ LPGEN("'Yes' Button"),              "btn_ok",       IDI_OK },
	{ LPGEN("'No' Button"),               "btn_cancel",   IDI_CANCEL }
};

VOID IcoLibInit()
{
	Icon_Register(hInst, MODULEA, iconList, SIZEOF(iconList));
}

BOOL NetlibInit()
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = TranslateT("Pack Updater HTTP connection");
	nlu.szSettingsModule = MODNAME;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	return hNetlibUser != NULL;
}

VOID NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = NULL;
}

VOID InitPopupList()
{
	int index = 0;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups0Tx", COLOR_TX_DEFAULT);

	index = 1;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups1Tx", COLOR_TX_DEFAULT);

	index = 2;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups2Tx", COLOR_TX_DEFAULT);

	index = 3;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups3Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups3Tx", COLOR_TX_DEFAULT);
}

VOID LoadOptions()
{
	MyOptions.DefColors = db_get_b(NULL, MODNAME, "DefColors", DEFAULT_COLORS);
	MyOptions.LeftClickAction= db_get_b(NULL, MODNAME, "LeftClickAction", DEFAULT_POPUP_LCLICK);
	MyOptions.RightClickAction = db_get_b(NULL, MODNAME, "RightClickAction", DEFAULT_POPUP_RCLICK);
	MyOptions.Timeout = db_get_dw(NULL, MODNAME, "Timeout", DEFAULT_TIMEOUT_VALUE);
	UpdateOnStartup = db_get_b(NULL, MODNAME, "UpdateOnStartup", DEFAULT_UPDATEONSTARTUP);
	OnlyOnceADay = db_get_b(NULL, MODNAME, "OnlyOnceADay", DEFAULT_ONLYONCEADAY);
	UpdateOnPeriod = db_get_b(NULL, MODNAME, "UpdateOnPeriod", DEFAULT_UPDATEONPERIOD);
	Period = db_get_dw(NULL, MODNAME, "Period", DEFAULT_PERIOD);
	PeriodMeasure = db_get_b(NULL, MODNAME, "PeriodMeasure", DEFAULT_PERIODMEASURE);
	Reminder = db_get_b(NULL, MODNAME, "Reminder", DEFAULT_REMINDER);
	FileCount = db_get_dw(NULL, MODNAME, "FileCount", DEFAULT_FILECOUNT);
}

BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	HANDLE hFile = NULL;
	DWORD dwBytes;

	NETLIBHTTPREQUEST nlhr = {0};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_REDIRECT | NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	char* szUrl = mir_t2a(tszURL);
	nlhr.szUrl = szUrl;
	nlhr.headersCount = 4;
	nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName   = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	nlhr.headers[1].szName  = "Connection";
	nlhr.headers[1].szValue = "close";
	nlhr.headers[2].szName  = "Cache-Control";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName  = "Pragma";
	nlhr.headers[3].szValue = "no-cache";

	bool ret = false;
	NETLIBHTTPREQUEST* pReply = NULL;
	pReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser,(LPARAM)&nlhr);

	if (pReply) {
		if (200 == pReply->resultCode && pReply->dataLength > 0) {
			hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
			ret = true;
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)pReply);
	}

	mir_free(szUrl);
	mir_free(nlhr.headers);

	if (hFile)
		CloseHandle(hFile);
	DlgDld = ret;
	return ret;
}

VOID __stdcall ExitMe(void*)
{
	CallService("CloseAction", 0, 0);
}

VOID __stdcall RestartMe(void*)
{
	CallService(MS_SYSTEM_RESTART, 1, 0);
}

BOOL Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

BOOL IsPluginDisabled(TCHAR* filename)
{
	char* fname = mir_t2a(filename);
	int res = db_get_b(NULL, "PluginDisable", fname, 0);
	mir_free(fname);
	return res;
}

INT getVer(const TCHAR* verStr)
{
	INT v1 = 0, v2 = 0, v3 = 0, v4 = 0;
	_stscanf(verStr, _T("%d.%d.%d.%d"), &v1, &v2, &v3, &v4);
	return v1*10000000 + v2*100000 + v3*1000 + v4;
}

static void CheckUpdates(void *)
{
	TCHAR tszBuff[2048] = {0}, tszFileInfo[30] = {0}, tszTmpIni[MAX_PATH] = {0};
	char szKey[64] = {0};
	INT upd_ret;
	DBVARIANT dbVar = {0};
	vector<FILEINFO> UpdateFiles;

	if (!Exists(tszRoot))
		CreateDirectory(tszRoot, NULL);
	Files.clear();
	Reminder = db_get_b(NULL, MODNAME, "Reminder", DEFAULT_REMINDER);
	FileCount = db_get_dw(NULL, MODNAME, "FileCount", DEFAULT_FILECOUNT);

	// Load files info
	db_get_ts(NULL, MODNAME, "File_VersionURL", &dbVar);
	if (lstrcmp(dbVar.ptszVal, NULL) == 0) { // URL is not set
		Title=TranslateT("Pack Updater");
		Text = TranslateT("URL for checking updates not found.");
		if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) && db_get_b(NULL, MODNAME, "Popups1", DEFAULT_POPUP_ENABLED)) {
			Number = 1;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (db_get_b(NULL, MODNAME, "Popups1M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONSTOP);
		db_free(&dbVar);
		hCheckThread = NULL;
		return;
	}
	// Download version info
	pFileUrl = (FILEURL *)mir_alloc(sizeof(*pFileUrl));
	mir_tstrncpy(pFileUrl->tszDownloadURL, dbVar.ptszVal, SIZEOF(pFileUrl->tszDownloadURL));
	mir_sntprintf(tszBuff, SIZEOF(tszBuff), _T("%s\\tmp.ini"), tszRoot);
	mir_tstrncpy(pFileUrl->tszDiskPath, tszBuff, SIZEOF(pFileUrl->tszDiskPath));
	mir_tstrncpy(tszTmpIni, tszBuff, SIZEOF(tszTmpIni));
	Title = TranslateT("Pack Updater");
	Text = TranslateT("Downloading version info...");
	DlgDownloadProc();
	mir_free(pFileUrl);
	if (!DlgDld) {
		hCheckThread = NULL;
		return;
	}

	for (CurrentFile = 0; CurrentFile < FileCount; CurrentFile++) {
		FILEINFO FileInfo = {_T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), {_T(""), _T("")}};

		dbVar.ptszVal = NULL;
		mir_snprintf(szKey, SIZEOF(szKey), "File_%d_CurrentVersion", CurrentFile + 1);
		db_get_ts(NULL, MODNAME, szKey, &dbVar);
		if (lstrcmp(dbVar.ptszVal, NULL) == 0) {
			db_free(&dbVar);
			mir_tstrncpy(FileInfo.tszCurVer, _T(""), SIZEOF(FileInfo.tszCurVer));
		}
		else mir_tstrncpy(FileInfo.tszCurVer, dbVar.ptszVal, SIZEOF(FileInfo.tszCurVer));

		dbVar.ptszVal = NULL;
		mir_snprintf(szKey, SIZEOF(szKey), "File_%d_LastVersion", CurrentFile + 1);
		db_get_ts(NULL, MODNAME, szKey, &dbVar);
		if (lstrcmp(dbVar.ptszVal, NULL) == 0) {
			db_free(&dbVar);
			mir_tstrncpy(FileInfo.tszLastVer, _T(""), SIZEOF(FileInfo.tszLastVer));
		}
		else mir_tstrncpy(FileInfo.tszLastVer, dbVar.ptszVal, SIZEOF(FileInfo.tszLastVer));

		Files.push_back(FileInfo);

		// Read version info
		mir_sntprintf(tszFileInfo, SIZEOF(tszFileInfo), _T("FileInfo_%d"), CurrentFile + 1);
		GetPrivateProfileString(tszFileInfo, _T("FileVersion"), _T(""), Files[CurrentFile].tszNewVer, SIZEOF(Files[CurrentFile].tszNewVer), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, _T("Message"), _T(""), Files[CurrentFile].tszMessage, SIZEOF(Files[CurrentFile].tszMessage), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, _T("DownloadURL"), _T(""), Files[CurrentFile].File.tszDownloadURL, SIZEOF(Files[CurrentFile].File.tszDownloadURL), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, _T("AdvFolder"), _T(""), Files[CurrentFile].tszAdvFolder, SIZEOF(Files[CurrentFile].tszAdvFolder), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, _T("Descr"), _T(""), Files[CurrentFile].tszDescr, SIZEOF(Files[CurrentFile].tszDescr), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, _T("DiskFileName"), _T(""), tszBuff, MAX_PATH, tszTmpIni);

		if (_tcsstr(tszBuff, _T("\\"))) { //check update name
			Title = TranslateT("Pack Updater");
			Text = TranslateT("Name of Update's file is not supported.");
			if ( ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) &&  db_get_b(NULL, MODNAME, "Popups1", DEFAULT_POPUP_ENABLED)) {
				Number = 1;
				show_popup(0, Title, Text, Number, 0);
			}
			else if (db_get_b(NULL, MODNAME, "Popups1M", DEFAULT_MESSAGE_ENABLED))
				MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
			continue;
		} // end check update name
		mir_tstrncpy(Files[CurrentFile].File.tszDiskPath, tszBuff, SIZEOF(Files[CurrentFile].File.tszDiskPath));
		GetPrivateProfileString(tszFileInfo, _T("InfoURL"), _T(""), Files[CurrentFile].tszInfoURL, SIZEOF(Files[CurrentFile].tszInfoURL), tszTmpIni);
		Files[CurrentFile].FileType = GetPrivateProfileInt(tszFileInfo, _T("FileType"), 0, tszTmpIni);
		Files[CurrentFile].Force = GetPrivateProfileInt(tszFileInfo, _T("Force"), 0, tszTmpIni);
		Files[CurrentFile].FileNum = CurrentFile+1;

		if (Files[CurrentFile].FileType == 2) {
			TCHAR pluginFolgerName[MAX_PATH];
			if (lstrcmp(Files[CurrentFile].tszAdvFolder, _T("")) == 0)
				mir_sntprintf(tszBuff, SIZEOF(tszBuff), _T("Plugins\\%s"), Files[CurrentFile].File.tszDiskPath);
			else
				mir_sntprintf(tszBuff, SIZEOF(tszBuff), _T("Plugins\\%s\\%s"), Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
			PathToAbsoluteT(tszBuff, pluginFolgerName);
			if ((IsPluginDisabled(Files[CurrentFile].File.tszDiskPath) || !Exists(pluginFolgerName))) //check if plugin disabled or not exists
				continue;
		}
		// Compare versions
		if (getVer(Files[CurrentFile].tszCurVer) < getVer(Files[CurrentFile].tszNewVer)) { // Yeah, we've got new version.
			TCHAR* tszSysRoot = Utils_ReplaceVarsT(_T("%SystemRoot%"));
			TCHAR* tszProgFiles = Utils_ReplaceVarsT(_T("%ProgramFiles%"));

			if (Files[CurrentFile].FileType != 1 && !IsUserAnAdmin() && (_tcsstr(tszRoot, tszSysRoot) || _tcsstr(tszRoot, tszProgFiles))) {
				MessageBox(NULL, TranslateT("Update is not possible!\nYou have no Administrator's rights.\nPlease run Miranda NG with Administrator's rights."), Title, MB_ICONINFORMATION);
				DeleteFile(tszTmpIni);
				hCheckThread = NULL;
				return;
			} // user have not admin's rights

			//добавить проверку на существование файла
			TCHAR tszFilePathDest[MAX_PATH] = {0};
			TCHAR* tszUtilRootPlug = NULL;
			TCHAR* tszUtilRootIco = NULL;
			TCHAR* tszUtilRoot = NULL;

			switch (Files[CurrentFile].FileType) {
			case 0:
			case 1:
				break;
			case 2:
				tszUtilRootPlug = Utils_ReplaceVarsT(_T("%miranda_path%\\Plugins"));
				if (lstrcmp(Files[CurrentFile].tszAdvFolder, _T("")) == 0)
					mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRootPlug, Files[CurrentFile].File.tszDiskPath);
				else
					mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRootPlug, Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
				mir_free(tszUtilRootPlug);
				break;
			case 3:
				tszUtilRootIco = Utils_ReplaceVarsT(_T("%miranda_path%\\Icons"));
				if (lstrcmp(Files[CurrentFile].tszAdvFolder, _T("")) == 0)
					mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRootIco, Files[CurrentFile].File.tszDiskPath);
				else
					mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRootIco, Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
				mir_free(tszUtilRootIco);
				break;
			case 4:
			case 5:
				tszUtilRoot = Utils_ReplaceVarsT(_T("%miranda_path%"));
				if (lstrcmp(Files[CurrentFile].tszAdvFolder, _T("")) == 0)
					mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s"), tszUtilRoot, Files[CurrentFile].File.tszDiskPath);
				else
					mir_sntprintf(tszFilePathDest, SIZEOF(tszFilePathDest), _T("%s\\%s\\%s"), tszUtilRoot, Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
				mir_free(tszUtilRoot);
				break;
			}//end* switch (Files[CurrentFile].FileType)

			if (Files[CurrentFile].Force || Exists(tszFilePathDest))
				UpdateFiles.push_back(Files[CurrentFile]);
			// Save last version
			mir_tstrncpy(Files[CurrentFile].tszLastVer, Files[CurrentFile].tszNewVer, SIZEOF(Files[CurrentFile].tszLastVer));
			mir_snprintf(szKey, SIZEOF(szKey), "File_%d_LastVersion", CurrentFile + 1);
			db_set_ts(NULL, MODNAME, szKey, Files[CurrentFile].tszLastVer);

			mir_free(tszSysRoot);
			mir_free(tszProgFiles);
		} // end compare versions
	} //end checking all files in for ()

	// Show dialog
	if (UpdateFiles.size()>0)
		upd_ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UPDATE), GetDesktopWindow(), DlgUpdate, (LPARAM)&UpdateFiles);
	DeleteFile(tszTmpIni);
	if (upd_ret == IDCANCEL) {
		hCheckThread = NULL;
		return;
	}
	
	if (!UpdatesCount && !Silent) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("No updates found.");
		if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) &&  db_get_b(NULL, MODNAME, "Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (db_get_b(NULL, MODNAME, "Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
	}

	if (!FileCount) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("No files for update.");
		if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) &&  db_get_b(NULL, MODNAME, "Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (db_get_b(NULL, MODNAME, "Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
	}
	hCheckThread = NULL;
}

void DoCheck(int iFlag)
{
	if (hCheckThread != NULL) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("Update checking already started!");
		if ( ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) &&  db_get_b(NULL, MODNAME, "Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (db_get_b(NULL, MODNAME, "Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
	}
	else if (iFlag) {
		hCheckThread = mir_forkthread(CheckUpdates, 0);
		db_set_dw(NULL, MODNAME, "LastUpdate", time(NULL));
	}
}

BOOL AllowUpdateOnStartup()
{
	if(OnlyOnceADay) {
		time_t now = time(NULL);
		time_t was = db_get_dw(NULL, MODNAME, "LastUpdate", 0);

		if((now - was) < 86400)
			return FALSE;
	}
	return TRUE;
}

LONG PeriodToMilliseconds(const INT period, BYTE& periodMeasure)
{
	LONG result = period * 1000;
	switch(periodMeasure) {
	case 1:
		// day
		result *= 60 * 60 * 24;
		break;

	default:
		// hour
		if(periodMeasure != 0)
			periodMeasure = 0;
		result *= 60 * 60;
		break;
	}
	return result;
}

VOID CALLBACK TimerAPCProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	DoCheck(TRUE);
}

VOID InitTimer()
{
	CancelWaitableTimer(Timer);
	if(UpdateOnPeriod) {
		LONG interval = PeriodToMilliseconds(Period, PeriodMeasure);

		_int64 qwDueTime = -10000i64 * interval;

		LARGE_INTEGER li = {0};
		li.LowPart = (DWORD) ( qwDueTime & 0xFFFFFFFF );
		li.HighPart = (LONG) ( qwDueTime >> 32 );

		SetWaitableTimer(Timer, &li, interval, TimerAPCProc, NULL, 0);
	}
}
