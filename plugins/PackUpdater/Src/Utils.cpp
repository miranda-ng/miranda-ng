/*
Copyright (C) 2011-22 Mataes

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

#include "stdafx.h"

vector<FILEINFO> Files;
BOOL DlgDld;
INT FileCount = 0, CurrentFile = 0, Number = 0;
uint8_t Reminder, UpdateOnStartup, UpdateOnPeriod, OnlyOnceADay, PeriodMeasure;
INT Period;
wchar_t tszDialogMsg[2048] = { 0 };
FILEINFO* pFileInfo = nullptr;
FILEURL* pFileUrl = nullptr;
HANDLE hCheckThread = nullptr;
HNETLIBUSER hNetlibUser = nullptr;
MYOPTIONS MyOptions = { 0 };
aPopups PopupsList[POPUPS];
LPCTSTR Title = {}, Text = {};

IconItem iconList[] =
{
	{ LPGEN("Check for pack updates"), "check_update", IDI_MENU },
	{ LPGEN("Clear pack updates folder"), "empty_folder", IDI_DELETE },
	{ LPGEN("'Yes' Button"), "btn_ok", IDI_OK },
	{ LPGEN("'No' Button"), "btn_cancel", IDI_CANCEL }
};

void IcoLibInit()
{
	g_plugin.registerIcon(MODULEA, iconList);
}

BOOL NetlibInit()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = TranslateT("Pack Updater HTTP connections");
	nlu.szSettingsModule = MODULENAME;
	hNetlibUser = Netlib_RegisterUser(&nlu);

	return hNetlibUser != nullptr;
}

void NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = nullptr;
}

void InitPopupList()
{
	int index = 0;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = g_plugin.getDword("Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = g_plugin.getDword("Popups0Tx", COLOR_TX_DEFAULT);

	index = 1;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = g_plugin.getDword("Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = g_plugin.getDword("Popups1Tx", COLOR_TX_DEFAULT);

	index = 2;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = g_plugin.getDword("Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = g_plugin.getDword("Popups2Tx", COLOR_TX_DEFAULT);

	index = 3;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = g_plugin.getDword("Popups3Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = g_plugin.getDword("Popups3Tx", COLOR_TX_DEFAULT);
}

void LoadOptions()
{
	MyOptions.DefColors = g_plugin.getByte("DefColors", DEFAULT_COLORS);
	MyOptions.LeftClickAction = g_plugin.getByte("LeftClickAction", DEFAULT_POPUP_LCLICK);
	MyOptions.RightClickAction = g_plugin.getByte("RightClickAction", DEFAULT_POPUP_RCLICK);
	MyOptions.Timeout = g_plugin.getDword("Timeout", DEFAULT_TIMEOUT_VALUE);
	UpdateOnStartup = g_plugin.getByte("UpdateOnStartup", DEFAULT_UPDATEONSTARTUP);
	OnlyOnceADay = g_plugin.getByte("OnlyOnceADay", DEFAULT_ONLYONCEADAY);
	UpdateOnPeriod = g_plugin.getByte("UpdateOnPeriod", DEFAULT_UPDATEONPERIOD);
	Period = g_plugin.getDword("Period", DEFAULT_PERIOD);
	PeriodMeasure = g_plugin.getByte("PeriodMeasure", DEFAULT_PERIODMEASURE);
	Reminder = g_plugin.getByte("Reminder", DEFAULT_REMINDER);
	FileCount = g_plugin.getDword("FileCount", DEFAULT_FILECOUNT);
}

BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_REDIRECT | NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	char* szUrl = mir_u2a(tszURL);
	nlhr.szUrl = szUrl;
	nlhr.headersCount = 4;
	nlhr.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = NETLIB_USER_AGENT;
	nlhr.headers[1].szName = "Connection";
	nlhr.headers[1].szValue = "close";
	nlhr.headers[2].szName = "Cache-Control";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName = "Pragma";
	nlhr.headers[3].szValue = "no-cache";

	bool ret = false;
	NLHR_PTR pReply(Netlib_HttpTransaction(hNetlibUser, &nlhr));
	if (pReply) {
		if (200 == pReply->resultCode && pReply->dataLength > 0) {
			HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			DWORD dwBytes;
			WriteFile(hFile, pReply->pData, (uint32_t)pReply->dataLength, &dwBytes, nullptr);
			ret = true;
			if (hFile)
				CloseHandle(hFile);
		}
	}

	mir_free(szUrl);
	mir_free(nlhr.headers);

	DlgDld = ret;
	return ret;
}

void __stdcall ExitMe(void*)
{
	CallService("CloseAction", 0, 0);
}

void __stdcall RestartMe(void*)
{
	CallService(MS_SYSTEM_RESTART, 1, 0);
}

BOOL Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

size_t getVer(const wchar_t* verStr)
{
	int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
	swscanf(verStr, L"%d.%d.%d.%d", &v1, &v2, &v3, &v4);
	return v1 * 10000000 + v2 * 100000 + v3 * 1000 + v4;
}

static void CheckUpdates(void *)
{
	wchar_t tszBuff[2048] = { 0 }, tszFileInfo[30] = { 0 }, tszTmpIni[MAX_PATH] = { 0 };
	char szKey[64] = { 0 };
	vector<FILEINFO> UpdateFiles;

	if (!Exists(tszRoot))
		CreateDirectory(tszRoot, nullptr);
	Files.clear();
	Reminder = g_plugin.getByte("Reminder", DEFAULT_REMINDER);
	FileCount = g_plugin.getDword("FileCount", DEFAULT_FILECOUNT);

	// Load files info
	ptrW tszDownloadURL(g_plugin.getWStringA("File_VersionURL"));
	if (!tszDownloadURL) { // URL is not set
		Title = TranslateT("Pack Updater");
		Text = TranslateT("URL for checking updates not found.");
		if (Popup_Enabled() && g_plugin.getByte("Popups1", DEFAULT_POPUP_ENABLED)) {
			Number = 1;
			show_popup(nullptr, Title, Text, Number, 0);
		}
		else if (g_plugin.getByte("Popups1M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(nullptr, Text, Title, MB_ICONSTOP);
		hCheckThread = nullptr;
		return;
	}
	// Download version info
	pFileUrl = (FILEURL *)mir_alloc(sizeof(*pFileUrl));
	mir_wstrncpy(pFileUrl->tszDownloadURL, tszDownloadURL, _countof(pFileUrl->tszDownloadURL));
	mir_snwprintf(tszBuff, L"%s\\tmp.ini", tszRoot);
	mir_wstrncpy(pFileUrl->tszDiskPath, tszBuff, _countof(pFileUrl->tszDiskPath));
	mir_wstrncpy(tszTmpIni, tszBuff, _countof(tszTmpIni));
	Title = TranslateT("Pack Updater");
	Text = TranslateT("Downloading version info...");
	DlgDownloadProc();
	mir_free(pFileUrl);
	if (!DlgDld) {
		hCheckThread = nullptr;
		return;
	}

	for (CurrentFile = 0; CurrentFile < FileCount; CurrentFile++) {
		FILEINFO FileInfo = { L"", L"", L"", L"", L"", L"", L"", { L"", L"" } };

		mir_snprintf(szKey, "File_%d_CurrentVersion", CurrentFile + 1);
		ptrW tszCurVer(g_plugin.getWStringA(szKey));
		if (tszCurVer)
			mir_wstrncpy(FileInfo.tszCurVer, tszCurVer, _countof(FileInfo.tszCurVer));
		else 
			mir_wstrncpy(FileInfo.tszCurVer, L"", _countof(FileInfo.tszCurVer));

		mir_snprintf(szKey, "File_%d_LastVersion", CurrentFile + 1);
		ptrW tszLastVer(g_plugin.getWStringA(szKey));
		if (tszLastVer)
			mir_wstrncpy(FileInfo.tszLastVer, tszLastVer, _countof(FileInfo.tszLastVer));
		else
			mir_wstrncpy(FileInfo.tszLastVer, L"", _countof(FileInfo.tszLastVer));

		Files.push_back(FileInfo);

		// Read version info
		mir_snwprintf(tszFileInfo, L"FileInfo_%d", CurrentFile + 1);
		GetPrivateProfileString(tszFileInfo, L"FileVersion", L"", Files[CurrentFile].tszNewVer, _countof(Files[CurrentFile].tszNewVer), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, L"Message", L"", Files[CurrentFile].tszMessage, _countof(Files[CurrentFile].tszMessage), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, L"DownloadURL", L"", Files[CurrentFile].File.tszDownloadURL, _countof(Files[CurrentFile].File.tszDownloadURL), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, L"AdvFolder", L"", Files[CurrentFile].tszAdvFolder, _countof(Files[CurrentFile].tszAdvFolder), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, L"Descr", L"", Files[CurrentFile].tszDescr, _countof(Files[CurrentFile].tszDescr), tszTmpIni);
		GetPrivateProfileString(tszFileInfo, L"DiskFileName", L"", tszBuff, MAX_PATH, tszTmpIni);

		if (wcsstr(tszBuff, L"\\")) { //check update name
			Title = TranslateT("Pack Updater");
			Text = TranslateT("Name of Update's file is not supported.");
			if (Popup_Enabled() && g_plugin.getByte("Popups1", DEFAULT_POPUP_ENABLED)) {
				Number = 1;
				show_popup(nullptr, Title, Text, Number, 0);
			}
			else if (g_plugin.getByte("Popups1M", DEFAULT_MESSAGE_ENABLED))
				MessageBox(nullptr, Text, Title, MB_ICONINFORMATION);
			continue;
		} // end check update name
		mir_wstrncpy(Files[CurrentFile].File.tszDiskPath, tszBuff, _countof(Files[CurrentFile].File.tszDiskPath));
		GetPrivateProfileString(tszFileInfo, L"InfoURL", L"", Files[CurrentFile].tszInfoURL, _countof(Files[CurrentFile].tszInfoURL), tszTmpIni);
		Files[CurrentFile].FileType = GetPrivateProfileInt(tszFileInfo, L"FileType", 0, tszTmpIni);
		Files[CurrentFile].Force = GetPrivateProfileInt(tszFileInfo, L"Force", 0, tszTmpIni);
		Files[CurrentFile].FileNum = CurrentFile + 1;

		if (Files[CurrentFile].FileType == 2) {
			if (mir_wstrcmp(Files[CurrentFile].tszAdvFolder, L"") == 0)
				mir_snwprintf(tszBuff, L"Plugins\\%s", Files[CurrentFile].File.tszDiskPath);
			else
				mir_snwprintf(tszBuff, L"Plugins\\%s\\%s", Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
			wchar_t pluginFolderName[MAX_PATH];
			PathToAbsoluteW(tszBuff, pluginFolderName);
			if (!Files[CurrentFile].Force && (IsPluginOnWhiteList(_T2A(Files[CurrentFile].File.tszDiskPath)) || !Exists(pluginFolderName))) //check if plugin disabled or not exists
				continue;
		}
		// Compare versions
		if (getVer(Files[CurrentFile].tszCurVer) < getVer(Files[CurrentFile].tszNewVer)) { // Yeah, we've got new version.
			VARSW tszSysRoot(L"%SystemRoot%");
			VARSW tszProgFiles(L"%ProgramFiles%");

			if (Files[CurrentFile].FileType != 1 && !IsUserAnAdmin() && (wcsstr(tszRoot, tszSysRoot) || wcsstr(tszRoot, tszProgFiles))) {
				MessageBox(nullptr, TranslateT("Update is not possible!\nYou have no Administrator's rights.\nPlease run Miranda NG with Administrator's rights."), Title, MB_ICONINFORMATION);
				DeleteFile(tszTmpIni);
				hCheckThread = nullptr;
				return;
			} // user have not admin's rights

			//добавить проверку на существование файла
			wchar_t tszFilePathDest[MAX_PATH] = { 0 };
			switch (Files[CurrentFile].FileType) {
			case 0:
			case 1:
				break;
			case 2: {
				VARSW tszUtilRootPlug(L"%miranda_path%\\Plugins");
				if (mir_wstrcmp(Files[CurrentFile].tszAdvFolder, L"") == 0)
					mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRootPlug, Files[CurrentFile].File.tszDiskPath);
				else
					mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRootPlug, Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
				}
				break;
			case 3: {
				VARSW tszUtilRootIco(L"%miranda_path%\\Icons");
				if (mir_wstrcmp(Files[CurrentFile].tszAdvFolder, L"") == 0)
					mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRootIco, Files[CurrentFile].File.tszDiskPath);
				else
					mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRootIco, Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
				}
				break;
			case 4:
			case 5: {
				wchar_t *tszUtilRoot = VARSW(L"%miranda_path%");
				if (mir_wstrcmp(Files[CurrentFile].tszAdvFolder, L"") == 0)
					mir_snwprintf(tszFilePathDest, L"%s\\%s", tszUtilRoot, Files[CurrentFile].File.tszDiskPath);
				else
					mir_snwprintf(tszFilePathDest, L"%s\\%s\\%s", tszUtilRoot, Files[CurrentFile].tszAdvFolder, Files[CurrentFile].File.tszDiskPath);
				}
				break;
			}//end* switch (Files[CurrentFile].FileType)

			if (Files[CurrentFile].Force || Exists(tszFilePathDest))
				UpdateFiles.push_back(Files[CurrentFile]);
			// Save last version
			mir_wstrncpy(Files[CurrentFile].tszLastVer, Files[CurrentFile].tszNewVer, _countof(Files[CurrentFile].tszLastVer));
			mir_snprintf(szKey, "File_%d_LastVersion", CurrentFile + 1);
			g_plugin.setWString(szKey, Files[CurrentFile].tszLastVer);
		} // end compare versions
	} //end checking all files in for ()

	// Show dialog
	INT upd_ret = 0;
	if (UpdateFiles.size() > 0)
		upd_ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UPDATE), GetDesktopWindow(), DlgUpdate, (LPARAM)&UpdateFiles);
	DeleteFile(tszTmpIni);
	if (upd_ret == IDCANCEL) {
		hCheckThread = nullptr;
		return;
	}

	if (!UpdatesCount && !Silent) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("No updates found.");
		if (Popup_Enabled() && g_plugin.getByte("Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(nullptr, Title, Text, Number, 0);
		}
		else if (g_plugin.getByte("Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(nullptr, Text, Title, MB_ICONINFORMATION);
	}

	if (!FileCount) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("No files for update.");
		if (Popup_Enabled() && g_plugin.getByte("Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(nullptr, Title, Text, Number, 0);
		}
		else if (g_plugin.getByte("Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(nullptr, Text, Title, MB_ICONINFORMATION);
	}
	hCheckThread = nullptr;
}

void DoCheck(int iFlag)
{
	if (hCheckThread != nullptr) {
		Title = TranslateT("Pack Updater");
		Text = TranslateT("Update checking already started!");
		if (Popup_Enabled() && g_plugin.getByte("Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(nullptr, Title, Text, Number, 0);
		}
		else if (g_plugin.getByte("Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(nullptr, Text, Title, MB_ICONINFORMATION);
	}
	else if (iFlag) {
		hCheckThread = mir_forkthread(CheckUpdates);
		g_plugin.setDword("LastUpdate", time(0));
	}
}

BOOL AllowUpdateOnStartup()
{
	if (OnlyOnceADay) {
		time_t now = time(0);
		time_t was = g_plugin.getDword("LastUpdate", 0);

		if ((now - was) < 86400)
			return FALSE;
	}
	return TRUE;
}

LONG PeriodToMilliseconds(const INT period, uint8_t& periodMeasure)
{
	LONG result = period * 1000;
	switch (periodMeasure) {
	case 1:
		// day
		result *= 60 * 60 * 24;
		break;

	default:
		// hour
		if (periodMeasure != 0)
			periodMeasure = 0;
		result *= 60 * 60;
		break;
	}
	return result;
}

void CALLBACK TimerAPCProc(LPVOID, DWORD, DWORD)
{
	DoCheck(TRUE);
}

void InitTimer()
{
	CancelWaitableTimer(Timer);
	if (UpdateOnPeriod) {
		LONG interval = PeriodToMilliseconds(Period, PeriodMeasure);

		_int64 qwDueTime = -10000i64 * interval;

		LARGE_INTEGER li = { 0 };
		li.LowPart = (uint32_t)(qwDueTime & 0xFFFFFFFF);
		li.HighPart = (LONG)(qwDueTime >> 32);

		SetWaitableTimer(Timer, &li, interval, TimerAPCProc, nullptr, 0);
	}
}
