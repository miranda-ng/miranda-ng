/*
Splash Screen Plugin for Miranda NG (www.miranda-ng.org)
(c) 2004-2007 nullbie, (c) 2005-2007 Thief

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

CLIST_INTERFACE *pcli;
HINSTANCE hInst = 0;
int hLangpack;

static HMODULE hAdvaimg = NULL;

BOOL bstartup = true; // startup?
BOOL bserviceinvoked = false;
BOOL bmodulesloaded = false; // modules are loaded
BOOL png2dibavail = true; // can we use png2dib service?

// path to miranda's dir, config file path, splash path, sound path
wchar_t szDllName[MAX_PATH], szSplashFile[MAX_PATH], szSoundFile[MAX_PATH], szhAdvaimgPath[MAX_PATH], szPrefix[128], inBuf[80];
wchar_t *szMirDir;
char szVersion[MAX_PATH];
#ifdef _DEBUG
wchar_t szLogFile[MAX_PATH];
#endif
SPLASHOPTS options;
HWND hwndSplash;

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
	// C64CC8E0-CF03-474A-8B11-8BD4565CCF04
	{ 0xc64cc8e0, 0xcf03, 0x474a, { 0x8b, 0x11, 0x8b, 0xd4, 0x56, 0x5c, 0xcf, 0x04 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

void SplashMain()
{
	if (bstartup) {
		// Retrive path to exe of current running Miranda is located
		szMirDir = Utils_ReplaceVarsW(L"%miranda_path%");
		mir_snwprintf(szhAdvaimgPath, L"%s\\plugins\\advaimg.dll", szMirDir);
		Miranda_GetVersionText(szVersion, MAX_PATH);

		#ifdef _DEBUG
		mir_snwprintf(szLogFile, L"%s\\%s.log", szMirDir, _A2W(__PLUGIN_NAME));
		initLog();
		wchar_t *mirandaVerString = mir_a2u(szVersion);
		logMessage(L"Miranda version", mirandaVerString);
		mir_free(mirandaVerString);
		logMessage(L"Dll Name", _A2W(__FILENAME));
		logMessage(L"Advaimg path", szhAdvaimgPath);
		#endif

		ReadDbConfig();
	}

	if (bstartup & (options.active == 1)) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ws(NULL, MODNAME, "VersionPrefix", &dbv)) {
			mir_wstrcpy(szPrefix, dbv.ptszVal);
			db_free(&dbv);
		}
		else
			mir_wstrcpy(szPrefix, L"");

		if (!db_get_ws(NULL, MODNAME, "Path", &dbv)) {
			mir_wstrcpy(inBuf, dbv.ptszVal);
			db_free(&dbv);
		}
		else mir_wstrcpy(inBuf, L"splash\\splash.png");

		wchar_t szExpandedSplashFile[MAX_PATH];
		ExpandEnvironmentStrings(inBuf, szExpandedSplashFile, _countof(szExpandedSplashFile));
		mir_wstrcpy(inBuf, szExpandedSplashFile);

		wchar_t *pos3 = 0;
		pos3 = wcsrchr(inBuf, ':');
		if (pos3 == NULL)
			mir_snwprintf(szSplashFile, L"%s\\%s", szMirDir, inBuf);
		else
			mir_wstrcpy(szSplashFile, inBuf);

		if (!db_get_ws(NULL, MODNAME, "Sound", &dbv)) {
			mir_wstrcpy(inBuf, dbv.ptszVal);
			db_free(&dbv);
		}
		else mir_wstrcpy(inBuf, L"sounds\\startup.wav");

		wchar_t szExpandedSoundFile[MAX_PATH];
		ExpandEnvironmentStrings(inBuf, szExpandedSoundFile, _countof(szExpandedSoundFile));
		mir_wstrcpy(inBuf, szExpandedSoundFile);

		wchar_t *pos2;
		pos2 = wcschr(inBuf, ':');
		if (pos2 == NULL)
			mir_snwprintf(szSoundFile, L"%s\\%s", szMirDir, inBuf);
		else
			mir_wstrcpy(szSoundFile, inBuf);

		#ifdef _DEBUG
		logMessage(L"SoundFilePath", szSoundFile);
		#endif

		wchar_t szOldPath[MAX_PATH] = { 0 };

		if (options.random) // randomly select a splash file
		{
			int filescount = 0;
			wchar_t szSplashDir[MAX_PATH] = { 0 }, szSearch[MAX_PATH] = { 0 };
			wchar_t *p = 0;
			wchar_t files[255][50]; //TODO: make memory allocation dynamic

			mir_wstrcpy(szSplashDir, szSplashFile);
			mir_wstrcpy(szOldPath, szSplashFile);
			// find the last \ and null it out, this leaves no trailing slash
			p = wcsrchr(szSplashDir, '\\');
			if (p) *p = 0;
			// create the search filter
			mir_snwprintf(szSearch, L"%s\\*.*", szSplashDir);
			// FFFN will return filenames
			HANDLE hFind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATA ffd;
			hFind = FindFirstFile(szSearch, &ffd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
						#ifdef _DEBUG
						logMessage(L"Found file", ffd.cFileName);
						#endif
						//files = new char[mir_strlen(ffd.cFileName)];
						//files[filescount] = new char[mir_strlen(ffd.cFileName)];
						wchar_t ext[5];
						wmemcpy(ext, ffd.cFileName + (mir_wstrlen(ffd.cFileName) - 4), 5);

						#ifdef _DEBUG
						logMessage(L"Extention", ext);
						#endif

						if (mir_wstrcmpi(ext, L".png") & mir_wstrcmpi(ext, L".bmp"))
							continue;

						#ifdef _DEBUG
						logMessage(L"File has valid ext", ext);
						#endif
						mir_wstrcpy(files[filescount++], ffd.cFileName);
					} //if
				} while (FindNextFile(hFind, &ffd));

				srand((unsigned)time(NULL));
				int r = 0;
				if (filescount) r = (rand() % filescount) + 1;

				mir_snwprintf(szSplashFile, L"%s\\%s", szSplashDir, files[r - 1]);

				#ifdef _DEBUG
				logMessage(L"final file", szSplashFile);
				#endif
				FindClose(hFind);
			} //if
		}

		// Call splash display routine
		ShowSplash(false);
	}
	bstartup = false;
}

int PlugDisableHook(WPARAM wParam, LPARAM lParam)
{
	#ifdef _DEBUG
	wchar_t buf[128];
	#endif
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (options.inheritGS) {
		if (!strcmp(cws->szModule, "Skin") && !strcmp(cws->szSetting, "UseSound")) {
			db_set_b(NULL, MODNAME, "PlaySound", cws->value.bVal);
			#ifdef _DEBUG
			cws->value.bVal ? _DebugPopup(NULL, L"Sounds enabled.", L"") : _DebugPopup(NULL, L"Sounds disabled.", L"");
			logMessage(L"Module", _A2T(cws->szModule));
			logMessage(L"Setting", _A2T(cws->szSetting));
			logMessage(L"Value", _itow(cws->value.bVal, buf, 10));
			#endif
		}
		if (!strcmp(cws->szModule, "PluginDisable") && !strcmp(cws->szSetting, _T2A(szDllName))) {
			db_set_b(NULL, MODNAME, "Active", cws->value.bVal);
			#ifdef _DEBUG
			cws->value.bVal ? _DebugPopup(NULL, L"Disabled.", "") : _DebugPopup(NULL, L"Enabled.", L"");
			logMessage(L"PlugDisableHook", L"Triggered");
			logMessage(L"Module", _A2T(cws->szModule));
			logMessage(L"Setting", _A2T(cws->szSetting));
			logMessage(L"Value", _itow(cws->value.bVal, buf, 10));
			#endif
		}
	}

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	bmodulesloaded = true; // all modules are loaded now, let other parts know about this fact

	if (hwndSplash) {
		if (PostMessage(hwndSplash, WM_LOADED, 0, 0)) {
			#ifdef _DEBUG
			logMessage(L"Posted WM_LOADED message", L"done");
			#endif
		}
	}

	// Options initialize hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, PlugDisableHook);

	// Service to call splash
	CreateServiceFunction(MS_SHOWSPLASH, ShowSplashService);

	#ifdef _DEBUG
	logMessage(L"Loading modules", L"done");
	#endif

	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	SplashMain();
	mir_free(szMirDir);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnregisterClass(SPLASH_CLASS, hInst);

	// Freeing loaded libraries
	if (hAdvaimg)
		FreeLibrary(hAdvaimg);

	#ifdef _DEBUG
	logMessage(L"Unload", L"Job done");
	#endif

	return 0;
}
