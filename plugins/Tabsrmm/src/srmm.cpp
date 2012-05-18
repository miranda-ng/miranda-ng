/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
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
 * $Id: srmm.cpp 13596 2011-04-15 19:07:23Z george.hazan $
 *
 * plugin loading functions and global exports.
 *
 */

#include "commonheaders.h"

extern int 	LoadSendRecvMessageModule(void);
extern int 	SplitmsgShutdown(void);
extern void LogErrorMessage(HWND hwndDlg, struct TWindowData *dat, int i, TCHAR *szMsg);
extern int  Chat_Load(PLUGINLINK *link), Chat_Unload();
extern void FreeLogFonts();

PLUGINLINK *pluginLink;
HINSTANCE g_hInst;
LOGFONT lfDefault = {0};

/*
 * miranda interfaces
 */

struct LIST_INTERFACE li;
struct MM_INTERFACE mmi;
int    hLangpack;
TIME_API tmi = {0};

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
#ifdef __GNUWIN32__
	"TabSRMM (MINGW32)",
#else
#ifdef _WIN64
	"TabSRMM (x64, Unicode)",
#else
#ifdef _UNICODE
	"TabSRMM (Unicode)",
#else
	"TabSRMM",
#endif
#endif
#endif
	PLUGIN_MAKE_VERSION(_VER_MAJOR, _VER_MINOR, _VER_REVISION, _VER_BUILD),
	"IM and group chat module for Miranda IM.",
	"The Miranda developers team and contributors",
	"silvercircle _at_ gmail _dot_ com",
	"2000-2010 Miranda Project and contributors. See readme.txt for more.",
	"http://miranda.or.at",
	UNICODE_AWARE,
	DEFMOD_SRMESSAGE,            // replace internal version (if any)
	{0x6ca5f042, 0x7a7f, 0x47cc, { 0xa7, 0x15, 0xfc, 0x8c, 0x46, 0xfb, 0xf4, 0x34 }} //{6CA5F042-7A7F-47cc-A715-FC8C46FBF434}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	CMimAPI::m_MimVersion = mirandaVersion;

	if(WinVerMajor() < 5) {
		MessageBox(0, _T("This version of tabSRMM requires Windows 2000 or later."), _T("tabSRMM"), MB_OK | MB_ICONERROR);
		return(0);
	}
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 6, 0)) {
		MessageBox(0, _T("This version of tabSRMM requires Miranda 0.8.5 or later. The plugin cannot be loaded."), _T("tabSRMM"), MB_OK | MB_ICONERROR);
		return(0);
	}
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_SRMM, MIID_CHAT, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK * link)
{
	pluginLink = link;

	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	CTranslator::preTranslateAll();

	M = new CMimAPI();

	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfDefault), &lfDefault, FALSE);

	Chat_Load(pluginLink);

	return LoadSendRecvMessageModule();
}

extern "C" int __declspec(dllexport) Unload(void)
{
	int iRet;
#if defined(__USE_EX_HANDLERS)
	__try {
#endif
		FreeLogFonts();
		Chat_Unload();
		iRet = SplitmsgShutdown();
		Skin->setupTabCloseBitmap(true);
		Skin->UnloadAeroTabs();
		CleanTempFiles();
		delete Skin;
		DestroyServiceFunction(hTypingNotify);
		delete sendLater;
		delete sendQueue;
		delete M;
#if defined(__USE_EX_HANDLERS)
	}
	__except(CGlobals::Ex_ShowDialog(GetExceptionInformation(), __FILE__, __LINE__, L"SHUTDOWN_STAGE_UNLOAD", false)) {
		return(0);
	}
#endif
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


	_vsnwprintf(debug, ibsize - 10, fmt, va);
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
			char *szDebug = M->utf8_encodeW(debug);
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
	_vsnprintf(&debug[9], ibsize - 10, fmt, va);
#ifdef _DEBUG
 	OutputDebugStringA(debug);
#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
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

int _DebugPopup(HANDLE hContact, const TCHAR *fmt, ...)
{
	va_list	va;
	TCHAR		debug[1024];
	int			ibsize = 1023;

	va_start(va, fmt);
	_vsntprintf(debug, ibsize, fmt, va);

	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY tn;
		TCHAR	szTitle[128];

		tn.szProto = NULL;
		tn.cbSize = sizeof(tn);
		mir_sntprintf(szTitle, safe_sizeof(szTitle), TranslateT("tabSRMM Message (%s)"), (hContact != 0) ? (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR) : TranslateT("Global"));
		tn.tszInfoTitle = szTitle;
		tn.tszInfo = debug;
		tn.dwInfoFlags = NIIF_INFO;
		tn.dwInfoFlags |= NIIF_INTERN_UNICODE;
		tn.uTimeout = 1000 * 4;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & tn);
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
				char 			str[64];
				TCHAR			tStr[80];
				char 			szVersion[512], *found = NULL, buildstr[50] = "";
				UINT 			build_nr = 0;
				DWORD 			v = pluginInfo.version;

				mir_snprintf(str, sizeof(str), Translate("Built %s %s"), __DATE__, __TIME__);
				SetDlgItemTextA(hwndDlg, IDC_BUILDTIME, str);

				CallService(MS_SYSTEM_GETVERSIONTEXT, 500, (LPARAM)szVersion);
				if ((found = strchr(szVersion, '#')) != NULL) {
					build_nr = atoi(found + 1);
					mir_snprintf(buildstr, 50, "[Build #%d]", build_nr);
				}
				TCHAR	*szBuildstr = mir_a2t(buildstr);
				mir_sntprintf(tStr, safe_sizeof(tStr), _T("TabSRMM\n%s %d.%d.%d.%d (Unicode) %s"),
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
				case IDC_RESETWARNINGS:
					M->WriteDword(SRMSGMOD_T, "cWarningsL", 0);
					M->WriteDword(SRMSGMOD_T, "cWarningsH", 0);
					break;
			}
			break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			SetTextColor((HDC)wParam, RGB(60, 60, 150));
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
		default:
			break;
	}
	return FALSE;
}
