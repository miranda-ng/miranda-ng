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

BOOL DlgDld;
int  Number = 0;
TCHAR tszDialogMsg[2048] = {0};
FILEINFO *pFileInfo = NULL;
HANDLE CheckThread = NULL, hNetlibUser = NULL;
POPUP_OPTIONS PopupOptions = {0};
aPopups PopupsList[POPUPS];

struct
{
	char *szIconName;
	char *szDescr;
	int   IconID;
}
static iconList[] =
{
	{ "check_update", LPGEN("Check for plugin updates"),    IDI_MENU },
	{ "btn_ok",			LPGEN("'Yes' Button"),              IDI_OK },
	{ "btn_cancel",   LPGEN("'No' Button"),               IDI_CANCEL }
};

void IcoLibInit()
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
	nlu.ptszDescriptiveName = TranslateT("Plugin Updater HTTP connection");
	nlu.szSettingsModule = MODNAME;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	return hNetlibUser != NULL;
}

void NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = NULL;
}

void InitPopupList()
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

void LoadOptions()
{
	PopupOptions.DefColors = DBGetContactSettingByte(NULL, MODNAME, "DefColors", DEFAULT_COLORS);
	PopupOptions.LeftClickAction= DBGetContactSettingByte(NULL, MODNAME, "LeftClickAction", DEFAULT_POPUP_LCLICK);
	PopupOptions.RightClickAction = DBGetContactSettingByte(NULL, MODNAME, "RightClickAction", DEFAULT_POPUP_RCLICK);
	PopupOptions.Timeout = DBGetContactSettingDword(NULL, MODNAME, "Timeout", DEFAULT_TIMEOUT_VALUE);
	
	opts.bUpdateOnStartup = DBGetContactSettingByte(NULL, MODNAME, "UpdateOnStartup", DEFAULT_UPDATEONSTARTUP);
	opts.bOnlyOnceADay = DBGetContactSettingByte(NULL, MODNAME, "OnlyOnceADay", DEFAULT_ONLYONCEADAY);
	opts.bUpdateOnPeriod = DBGetContactSettingByte(NULL, MODNAME, "UpdateOnPeriod", DEFAULT_UPDATEONPERIOD);
	opts.Period = DBGetContactSettingDword(NULL, MODNAME, "Period", DEFAULT_PERIOD);
	opts.bPeriodMeasure = DBGetContactSettingByte(NULL, MODNAME, "PeriodMeasure", DEFAULT_PERIODMEASURE);
	opts.bUpdateIcons = DBGetContactSettingByte(NULL, MODNAME, "UpdateIcons", DEFAULT_UPDATEICONS);
}

BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	HANDLE hFile = NULL;
	DWORD dwBytes;

	NETLIBHTTPREQUEST nlhr = {0};
	#if MIRANDA_VER < 0x0A00
		nlhr.cbSize = NETLIBHTTPREQUEST_V1_SIZE;
	#else
		nlhr.cbSize = sizeof(nlhr);
	#endif
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	char *szUrl = mir_t2a(tszURL);
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
	NETLIBHTTPREQUEST *pReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser,(LPARAM)&nlhr);
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

void __stdcall RestartMe(void*)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[100];
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
	TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), profilename);
	CallService("CloseAction", 0, 0);
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	mir_free(profilename);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL AllowUpdateOnStartup()
{
	if (opts.bOnlyOnceADay) {
		time_t now = time(NULL);
		time_t was = DBGetContactSettingDword(NULL, MODNAME, "LastUpdate", 0);

		if ((now - was) < 86400)
			return FALSE;
	}
	return TRUE;
}

LONG PeriodToMilliseconds(const int period, BYTE& periodMeasure)
{
	LONG result = period * 1000;
	switch(periodMeasure) {
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

void CALLBACK TimerAPCProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	DoCheck(1);
}

void InitTimer()
{
	CancelWaitableTimer(Timer);
	if (opts.bUpdateOnPeriod) {
		LONG interval = PeriodToMilliseconds(opts.Period, opts.bPeriodMeasure);

		_int64 qwDueTime = -10000i64 * interval;

		LARGE_INTEGER li = {0};
		li.LowPart = (DWORD) ( qwDueTime & 0xFFFFFFFF );
		li.HighPart = (LONG) ( qwDueTime >> 32 );

		SetWaitableTimer(Timer, &li, interval, TimerAPCProc, NULL, 0);
	}
}

void strdel(TCHAR *parBuffer, int len )
{
	TCHAR* p;
	for (p = parBuffer+len; *p != 0; p++)
		p[ -len ] = *p;

	p[ -len ] = '\0';
}

#if MIRANDA_VER < 0x0A00
char* rtrim(char *str)
{
	if (str == NULL)
		return NULL;

	char *p = strchr(str, 0);
	while (--p >= str) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}

void CreatePathToFileT(TCHAR *szFilePath)
{
	TCHAR *pszLastBackslash = _tcsrchr(szFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)szFilePath);
	*pszLastBackslash = '\\';
}
#endif
