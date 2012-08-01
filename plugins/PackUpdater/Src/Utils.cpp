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

//vector<FILEINFO> Files;
BOOL DlgDld;
INT /*CurrentFile = 0,*/ Number = 0;
BYTE Reminder, AutoUpdate;
BYTE UpdateOnStartup, UpdateOnPeriod, OnlyOnceADay, PeriodMeasure;
INT Period;
TCHAR tszDialogMsg[2048] = {0};
FILEINFO* pFileInfo = NULL;
//FILEURL* pFileUrl = NULL;
HANDLE CheckThread = NULL, hNetlibUser = NULL;
MYOPTIONS MyOptions = {0};
aPopups PopupsList[POPUPS];

int CalculateModuleHash(const TCHAR* tszFileName, char* dest);

typedef map<string, string> hashMap;
typedef pair<string, string> hashItem;

struct
{
	char*  szIconName;
	char*  szDescr;
	int    IconID;
}
static iconList[] =
{
	{ "check_update", LPGEN("Check for pack updates"),    IDI_MENU },
	{ "empty_folder", LPGEN("Clear pack updates folder"), IDI_DELETE },
	{ "btn_ok",			LPGEN("'Yes' Button"),              IDI_OK },
	{ "btn_cancel",   LPGEN("'No' Button"),               IDI_CANCEL }
};

VOID IcoLibInit()
{
	TCHAR destfile[MAX_PATH];
	GetModuleFileName(hInst, destfile, MAX_PATH);

	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_PATH_TCHAR;
	sid.cx = sid.cy = 16;
	sid.ptszDefaultFile = destfile;
	sid.pszSection = MODNAME;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		sid.pszName = iconList[i].szIconName;
		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].IconID;
		Skin_AddIcon(&sid);
	}
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
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups0Tx", COLOR_TX_DEFAULT);

	index = 1;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups1Tx", COLOR_TX_DEFAULT);

	index = 2;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups2Tx", COLOR_TX_DEFAULT);

	index = 3;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups3Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups3Tx", COLOR_TX_DEFAULT);
}

VOID LoadOptions()
{
	MyOptions.DefColors = DBGetContactSettingByte(NULL, MODNAME, "DefColors", DEFAULT_COLORS);
	MyOptions.LeftClickAction= DBGetContactSettingByte(NULL, MODNAME, "LeftClickAction", DEFAULT_POPUP_LCLICK);
	MyOptions.RightClickAction = DBGetContactSettingByte(NULL, MODNAME, "RightClickAction", DEFAULT_POPUP_RCLICK);
	MyOptions.Timeout = DBGetContactSettingDword(NULL, MODNAME, "Timeout", DEFAULT_TIMEOUT_VALUE);
	UpdateOnStartup = DBGetContactSettingByte(NULL, MODNAME, "UpdateOnStartup", DEFAULT_UPDATEONSTARTUP);
	OnlyOnceADay = DBGetContactSettingByte(NULL, MODNAME, "OnlyOnceADay", DEFAULT_ONLYONCEADAY);
	UpdateOnPeriod = DBGetContactSettingByte(NULL, MODNAME, "UpdateOnPeriod", DEFAULT_UPDATEONPERIOD);
	Period = DBGetContactSettingDword(NULL, MODNAME, "Period", DEFAULT_PERIOD);
	PeriodMeasure = DBGetContactSettingByte(NULL, MODNAME, "PeriodMeasure", DEFAULT_PERIODMEASURE);
	Reminder = DBGetContactSettingByte(NULL, MODNAME, "Reminder", DEFAULT_REMINDER);
}

BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	HANDLE hFile = NULL;
	DWORD dwBytes;

	NETLIBHTTPREQUEST nlhr = {0};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
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
	NETLIBHTTPREQUEST* pReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser,(LPARAM)&nlhr);
	if (pReply) {
		if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
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
	CallService(MS_SYSTEM_RESTART, 0, 0);
}

BOOL Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

BOOL IsPluginDisabled(const char *filename)
{
	return DBGetContactSettingByte(NULL, "PluginDisable", filename, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void ScanFolder(const TCHAR* tszFolder, hashMap& hashes, vector<FILEINFO>& UpdateFiles)
{
	TCHAR tszMask[MAX_PATH], tszFileBack[MAX_PATH];
	mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s\\*"), tszFolder);
	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(tszMask, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		if (!_tcscmp(ffd.cFileName, _T(".")) || !_tcscmp(ffd.cFileName, _T("..")))
			continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s\\%s"), tszFolder, ffd.cFileName);
			ScanFolder(tszMask, hashes, UpdateFiles);
			continue;
		}

		TCHAR *p = _tcsrchr(ffd.cFileName, '.');
		if (!p) continue;
		if ( _tcsicmp(p, _T(".dll")) && _tcsicmp(p, _T(".exe")))
			continue;

		char szFileName[MAX_PATH];
		strncpy(szFileName, _T2A(ffd.cFileName), SIZEOF(szFileName));
		if ( IsPluginDisabled(szFileName)) //check if plugin disabled
			continue;

		TCHAR *plugname = ffd.cFileName;
		FILEINFO FileInfo = { 0 };
		DBVARIANT dbv;
		if ( !DBGetContactSettingString(NULL, MODNAME, szFileName, &dbv)) {
			//считать хэш файла
			lstrcpynA(FileInfo.curhash, dbv.pszVal, SIZEOF(FileInfo.curhash));
			_strlwr(FileInfo.curhash);
			DBFreeVariant(&dbv);
		}
		else {
			mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s\\%s"), tszFolder, ffd.cFileName);
			CalculateModuleHash(tszMask, FileInfo.curhash);
			DBWriteContactSettingString(NULL, MODNAME, szFileName, FileInfo.curhash);
		}

		// Read version info
		hashMap::iterator boo = hashes.find(szFileName);
		if (boo == hashes.end())
			continue;

		strncpy(FileInfo.newhash, boo->second.c_str(), SIZEOF(FileInfo.newhash));

		// Compare versions
		if ( strcmp(FileInfo.curhash, FileInfo.newhash)) { // Yeah, we've got new version.
			_tcscpy(FileInfo.tszDescr, ffd.cFileName);

			*p = 0;
			mir_sntprintf(FileInfo.File.tszDownloadURL, SIZEOF(FileInfo.File.tszDownloadURL), _T("%s%s.zip"), _T(DEFAULT_UPDATE_URL), ffd.cFileName);
			_tcslwr(FileInfo.File.tszDownloadURL);

			mir_sntprintf(FileInfo.File.tszDiskPath, SIZEOF(FileInfo.File.tszDiskPath), _T("%s\\%s.zip"), tszFileBack, ffd.cFileName);

			UpdateFiles.push_back(FileInfo);
		} // end compare versions
	}
		while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

static void CheckUpdates(void *)
{
	TCHAR tszBuff[2048] = {0}, /*tszFileInfo[30] = {0},*/ tszTmpIni[MAX_PATH] = {0};
	char szKey[64] = {0};
	INT upd_ret;
	DBVARIANT dbVar = {0};
	vector<FILEINFO> UpdateFiles;

	if (!Exists(tszRoot))
		CreateDirectoryTreeT(tszRoot);

	//Files.clear();
	Reminder = DBGetContactSettingByte(NULL, MODNAME, "Reminder", DEFAULT_REMINDER);

	// Load files info
	if (DBGetContactSettingTString(NULL, MODNAME, "UpdateURL", &dbVar)) { // URL is not set 
		LPCTSTR Title=TranslateT("Pack Updater");
		LPCTSTR Text = TranslateT("URL for checking updates not found.");
		if (ServiceExists(MS_POPUP_ADDPOPUPEX) && DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1) && DBGetContactSettingByte(NULL, MODNAME, "Popups1", DEFAULT_POPUP_ENABLED)) {
			Number = 1;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (DBGetContactSettingByte(NULL, MODNAME, "Popups1M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONSTOP);
		DBFreeVariant(&dbVar);
		CheckThread = NULL;
		return;
	}

	// Download version info
	FILEURL *pFileUrl = (FILEURL *)mir_alloc(sizeof(*pFileUrl));
	lstrcpyn(pFileUrl->tszDownloadURL, dbVar.ptszVal, SIZEOF(pFileUrl->tszDownloadURL));
	DBFreeVariant(&dbVar);
	mir_sntprintf(tszBuff, SIZEOF(tszBuff), _T("%s\\tmp.ini"), tszRoot);
	lstrcpyn(pFileUrl->tszDiskPath, tszBuff, SIZEOF(pFileUrl->tszDiskPath));
	lstrcpyn(tszTmpIni, tszBuff, SIZEOF(tszTmpIni));
	PopupDataText temp;
	temp.Title = TranslateT("Pack Updater");
	temp.Text = TranslateT("Downloading version info...");
	DlgDownloadProc(pFileUrl, temp);
	mir_free(pFileUrl);
	if (!DlgDld) {
		CheckThread = NULL;
		return;
	}

	FILE* fp = _tfopen(tszTmpIni, _T("r"));
	if (!fp)
		return;

	hashMap hashes;
	char str[200];
	while(fgets(str, SIZEOF(str), fp) != NULL) {
		rtrim(str);
		char* p = strchr(str, ' ');
		if (p == NULL)
			continue;

		*p++ = 0;
		_strlwr(p);
		hashes[str] = p;
	}
	fclose(fp);
	DeleteFile(tszTmpIni);

	TCHAR *dirname = Utils_ReplaceVarsT(_T("%miranda_path%"));
	ScanFolder(dirname, hashes, UpdateFiles);
	mir_free(dirname);

	// Show dialog
	if (UpdateFiles.size() > 0)
		upd_ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UPDATE), GetDesktopWindow(), DlgUpdate, (LPARAM)&UpdateFiles);
	if (upd_ret == IDCANCEL) {
		CheckThread = NULL;
		return;
	}

	if (!UpdateFiles.size() && !Silent) {
		LPCTSTR Title = TranslateT("Pack Updater");
		LPCTSTR Text = TranslateT("No updates found.");
		if (ServiceExists(MS_POPUP_ADDPOPUPEX) && DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1) &&  DBGetContactSettingByte(NULL, MODNAME, "Popups2", DEFAULT_POPUP_ENABLED)) {
			Number = 2;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (DBGetContactSettingByte(NULL, MODNAME, "Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
	}

	CheckThread = NULL;
}

void DoCheck(int iFlag, int iFlag2)
{
	if (iFlag2) {
		LPCTSTR Title = TranslateT("Pack Updater");
		LPCTSTR Text = TranslateT("Update checking already started!");
		if (ServiceExists(MS_POPUP_ADDPOPUPEX) && DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1) &&  DBGetContactSettingByte(NULL, MODNAME, "Popups2", DEFAULT_POPUP_ENABLED))
		{
			Number = 2;
			show_popup(0, Title, Text, Number, 0);
		}
		else if (DBGetContactSettingByte(NULL, MODNAME, "Popups2M", DEFAULT_MESSAGE_ENABLED))
			MessageBox(NULL, Text, Title, MB_ICONINFORMATION);
	}
	else if (iFlag) {
		CheckThread = mir_forkthread(CheckUpdates, 0);
		DBWriteContactSettingDword(NULL, MODNAME, "LastUpdate", time(NULL));
	}
}

BOOL AllowUpdateOnStartup()
{
	if (OnlyOnceADay) {
		time_t now = time(NULL);
		time_t was = DBGetContactSettingDword(NULL, MODNAME, "LastUpdate", 0);

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
	DoCheck(1, (int)CheckThread);
}

VOID InitTimer()
{
	CancelWaitableTimer(Timer);
	if (UpdateOnPeriod) {
		LONG interval = PeriodToMilliseconds(Period, PeriodMeasure);

		_int64 qwDueTime = -10000i64 * interval;

		LARGE_INTEGER li = {0};
		li.LowPart = (DWORD) ( qwDueTime & 0xFFFFFFFF );
		li.HighPart = (LONG) ( qwDueTime >> 32 );

		SetWaitableTimer(Timer, &li, interval, TimerAPCProc, NULL, 0);
	}
}
