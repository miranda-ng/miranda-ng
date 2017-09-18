/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// plugin loading functions and global exports.

#include "stdafx.h"

HINSTANCE g_hInst;
LOGFONT lfDefault = { 0 };

/*
 * miranda interfaces
 */

int hLangpack;
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
	{ 0x6ca5f042, 0x7a7f, 0x47cc, { 0xa7, 0x15, 0xfc, 0x8c, 0x46, 0xfb, 0xf4, 0x34 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRMM, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	if (WinVerMajor() < 5) {
		MessageBox(0, TranslateT("This version of TabSRMM requires Windows 2000 or later."), L"tabSRMM", MB_OK | MB_ICONERROR);
		return 1;
	}

	pcli = Clist_GetInterface();
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

	mir_snprintf(tszTime, "%02d.%02d.%04d - %02d:%02d:%02d.%04d: ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);


	mir_vsnwprintf(debug, ibsize - 10, fmt, va);
	//#ifdef _DEBUG
	OutputDebugStringW(debug);
	//#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		Profile_GetPathA(MAX_PATH, szDataPath);
		mir_snprintf(szLogFileName, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
			fputs(tszTime, f);
			fputs(T2Utf(debug), f);
			fputs("\n", f);
			fclose(f);
		}
	}
	//#endif
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
int _DebugPopup(MCONTACT hContact, const wchar_t *fmt, ...)
{
	va_list	va;
	wchar_t		debug[1024];
	int			ibsize = 1023;

	va_start(va, fmt);
	mir_vsnwprintf(debug, ibsize, fmt, va);

	wchar_t	szTitle[128];
	mir_snwprintf(szTitle, TranslateT("TabSRMM message (%s)"),
		(hContact != 0) ? pcli->pfnGetContactDisplayName(hContact, 0) : TranslateT("Global"));

	Clist_TrayNotifyW(nullptr, szTitle, debug, NIIF_INFO, 1000 * 4);
	return 0;
}
