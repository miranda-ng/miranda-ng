/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * plugin loading functions and global exports.
 *
 */

#include "commonheaders.h"

HINSTANCE g_hInst;
LOGFONT lfDefault = {0};

/*
 * miranda interfaces
 */

int hLangpack;
TIME_API tmi = {0};
CLIST_INTERFACE *pcli;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {6CA5F042-7A7F-47CC-A715-FC8C46FBF434}
	{0x6ca5f042, 0x7a7f, 0x47cc, {0xa7, 0x15, 0xfc, 0x8c, 0x46, 0xfb, 0xf4, 0x34}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_SRMM, MIID_CHAT, MIID_LAST};

extern "C" int __declspec(dllexport) Load(void)
{
	if (WinVerMajor() < 5) {
		MessageBox(0, TranslateT("This version of TabSRMM requires Windows 2000 or later."), _T("tabSRMM"), MB_OK | MB_ICONERROR);
		return 1;
	}

	mir_getCLI();
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfDefault), &lfDefault, FALSE);

	Chat_Load();

	return LoadSendRecvMessageModule();
}

extern "C" int __declspec(dllexport) Unload(void)
{
	FreeLogFonts();
	Chat_Unload();
	int iRet = SplitmsgShutdown();
	Skin->setupTabCloseBitmap(true);
	Skin->UnloadAeroTabs();
	CleanTempFiles();
	delete Skin;
	DestroyServiceFunction(hTypingNotify);
	delete sendLater;
	delete sendQueue;
	return iRet;
}

int _DebugTraceW(const wchar_t *fmt, ...)
{
	wchar_t 	debug[2048];
	int     	ibsize = 2047;
	SYSTEMTIME	st;
	va_list 	va;
	char		tszTime[50];
	va_start(va, fmt);

	GetLocalTime(&st);

	mir_snprintf(tszTime, 50, "%02d.%02d.%04d - %02d:%02d:%02d.%04d: ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);


	mir_vsntprintf(debug, ibsize - 10, fmt, va);
//#ifdef _DEBUG
	OutputDebugStringW(debug);
//#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
			char *szDebug = mir_utf8encodeT(debug);
			fputs(tszTime, f);
			fputs(szDebug, f);
			fputs("\n", f);
			fclose(f);
			if (szDebug)
				mir_free(szDebug);
		}
	}
//#endif
	return 0;
}

int _DebugTraceA(const char *fmt, ...)
{
	char    debug[2048];
	int     ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	lstrcpyA(debug, "TABSRMM: ");
	mir_vsnprintf(&debug[9], ibsize - 10, fmt, va);
#ifdef _DEBUG
 	OutputDebugStringA(debug);
#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\tabsrmm_debug.log", szDataPath);
		FILE *f = fopen(szLogFileName, "a+");
		if (f) {
			fputs(debug, f);
			fputs("\n", f);
			fclose(f);
		}
	}
#endif
	return 0;
}

/*
 * output a notification message.
 * may accept a hContact to include the contacts nickname in the notification message...
 * the actual message is using printf() rules for formatting and passing the arguments...
 *
 * can display the message either as systray notification (baloon popup) or using the
 * popup plugin.
 */

int _DebugPopup(HCONTACT hContact, const TCHAR *fmt, ...)
{
	va_list	va;
	TCHAR		debug[1024];
	int			ibsize = 1023;

	va_start(va, fmt);
	mir_vsntprintf(debug, ibsize, fmt, va);

	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY tn;
		TCHAR	szTitle[128];
		mir_sntprintf(szTitle, SIZEOF(szTitle), TranslateT("TabSRMM Message (%s)"),
			(hContact != 0) ? pcli->pfnGetContactDisplayName(hContact, 0) : TranslateT("Global"));

		tn.szProto = NULL;
		tn.cbSize = sizeof(tn);
		tn.tszInfoTitle = szTitle;
		tn.tszInfo = debug;
		tn.dwInfoFlags = NIIF_INFO;
		tn.dwInfoFlags |= NIIF_INTERN_UNICODE;
		tn.uTimeout = 1000 * 4;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&tn);
	}
	return 0;
}

INT_PTR CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	COLORREF url_visited = RGB(128, 0, 128);
	COLORREF url_unvisited = RGB(0, 0, 255);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TCHAR tStr[80];
			char  szVersion[512], *found = NULL, buildstr[50] = "";
			UINT  build_nr = 0;
			DWORD v = pluginInfo.version;

			CallService(MS_SYSTEM_GETVERSIONTEXT, 500, (LPARAM)szVersion);
			if ((found = strchr(szVersion, '#')) != NULL) {
				build_nr = atoi(found + 1);
				mir_snprintf(buildstr, 50, "[Build #%d]", build_nr);
			}
			TCHAR	*szBuildstr = mir_a2t(buildstr);
			mir_sntprintf(tStr, SIZEOF(tStr), _T("TabSRMM\n%s %d.%d.%d.%d (Unicode) %s"),
				_T("Version"), HIBYTE(HIWORD(v)), LOBYTE(HIWORD(v)), HIBYTE(LOWORD(v)), LOBYTE(LOWORD(v)),
				szBuildstr);
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, tStr);
			mir_free(szBuildstr);
		}
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE));
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_SUPPORT:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://miranda.or.at/");
			break;
		}
		break;
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	}
	return FALSE;
}
