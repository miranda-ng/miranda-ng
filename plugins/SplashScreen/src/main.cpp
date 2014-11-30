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

#include "headers.h"

HINSTANCE hInst = 0;

int hLangpack;

static HMODULE hAdvaimg = NULL;

pfnConvertPng2dib png2dibConvertor = NULL;

BOOL bstartup = true; // startup?
BOOL bserviceinvoked = false;
BOOL bmodulesloaded = false; // modules are loaded
BOOL png2dibavail = true; // can we use png2dib service?

// path to miranda's dir, config file path, splash path, sound path
TCHAR szDllName[MAX_PATH], szSplashFile[MAX_PATH], szSoundFile[MAX_PATH], szhAdvaimgPath[MAX_PATH], szPrefix[128], inBuf[80];
TCHAR * szMirDir;
char szVersion[MAX_PATH];
#ifdef _DEBUG
	TCHAR szLogFile[MAX_PATH];
#endif
SPLASHOPTS options;
HWND hwndSplash;
HANDLE hSplashThread;

PLUGININFOEX pluginInfo={
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
	{0xc64cc8e0, 0xcf03, 0x474a, {0x8b, 0x11, 0x8b, 0xd4, 0x56, 0x5c, 0xcf, 0x04}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

void SplashMain()
{
	if (bstartup)
	{
		// Retrive path to exe of current running Miranda is located
		szMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));
		mir_sntprintf(szhAdvaimgPath, SIZEOF(szhAdvaimgPath), _T("%s\\plugins\\advaimg.dll"), szMirDir);
		CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVersion);

		#ifdef _DEBUG
			mir_sntprintf(szLogFile, SIZEOF(szLogFile), _T("%s\\%s.log"), szMirDir, _T(__PLUGIN_NAME));
			initLog();
			TCHAR* mirandaVerString = mir_a2t(szVersion);
			logMessage(_T("Miranda version"), mirandaVerString);
			mir_free(mirandaVerString);
			logMessage(_T("Dll Name"), _T(__FILENAME));
			logMessage(_T("Advaimg path"), szhAdvaimgPath);
		#endif

		ReadDbConfig();
	}

	if (bstartup & (options.active == 1))
	{
		if (hAdvaimg == NULL)
		{
			hAdvaimg = LoadLibrary(szhAdvaimgPath);
			if (hAdvaimg == NULL)
			{
				png2dibavail = false;
				bstartup = false;
			}
			if (hAdvaimg)
			{
				png2dibConvertor = (pfnConvertPng2dib) GetProcAddress(hAdvaimg, "mempng2dib");
				if (png2dibConvertor == NULL)
				{
					FreeLibrary(hAdvaimg); hAdvaimg = NULL;
					MessageBox(NULL,
					TranslateT("Your advaimg.dll is either obsolete or damaged. Get latest from Miranda alpha builds."),
					TranslateT("Error"),
					MB_OK | MB_ICONSTOP);
				}
				#ifdef _DEBUG
					if (png2dibConvertor)
						logMessage(_T("Loading advaimg"), _T("done"));
				#endif
			}
		}

		DBVARIANT dbv = {0};
		if (!db_get_ts(NULL, MODNAME, "VersionPrefix", &dbv))
		{
			_tcscpy_s(szPrefix, dbv.ptszVal);
			db_free(&dbv);
		}
		else
			_tcscpy_s(szPrefix, _T(""));

		if (!db_get_ts(NULL, MODNAME, "Path", &dbv))
		{
			_tcscpy_s(inBuf, dbv.ptszVal);
			db_free(&dbv);
		}
		else
			_tcscpy_s(inBuf, _T("splash\\splash.png"));

		TCHAR szExpandedSplashFile[MAX_PATH];
		ExpandEnvironmentStrings(inBuf, szExpandedSplashFile, SIZEOF(szExpandedSplashFile));
		_tcscpy_s(inBuf, szExpandedSplashFile);

		TCHAR *pos3 = 0;
		pos3 = _tcsrchr(inBuf, _T(':'));
		if (pos3 == NULL)
			mir_sntprintf(szSplashFile, SIZEOF(szSplashFile), _T("%s\\%s"), szMirDir, inBuf);
		else
			_tcscpy_s(szSplashFile, inBuf);

		if (!db_get_ts(NULL, MODNAME, "Sound", &dbv))
		{
			_tcscpy_s(inBuf, dbv.ptszVal);
			db_free(&dbv);
		}
		else
			_tcscpy_s(inBuf, _T("sounds\\startup.wav"));

		TCHAR szExpandedSoundFile[MAX_PATH];
		ExpandEnvironmentStrings(inBuf, szExpandedSoundFile, SIZEOF(szExpandedSoundFile));
		_tcscpy_s(inBuf, szExpandedSoundFile);

		TCHAR *pos2;
		pos2 = _tcschr(inBuf, _T(':'));
		if (pos2 == NULL)
			mir_sntprintf(szSoundFile, SIZEOF(szSoundFile), _T("%s\\%s"), szMirDir, inBuf);
		else
			_tcscpy_s(szSoundFile, inBuf);

		#ifdef _DEBUG
			logMessage(_T("SoundFilePath"), szSoundFile);
		#endif

		TCHAR szOldPath[MAX_PATH] = {0};

		if(options.random) // randomly select a splash file
		{
			int filescount = 0;
			TCHAR szSplashDir[MAX_PATH] = {0}, szSearch[MAX_PATH] = {0};
			TCHAR* p = 0;
			TCHAR files [255][50]; //TODO: make memory allocation dynamic

			_tcscpy_s(szSplashDir, szSplashFile);
			_tcscpy_s(szOldPath, szSplashFile);
			// find the last \ and null it out, this leaves no trailing slash
			p = _tcsrchr(szSplashDir, _T('\\'));
			if (p) *p = 0;
			// create the search filter
			mir_sntprintf(szSearch, SIZEOF(szSearch), _T("%s\\*.*"), szSplashDir);
			// FFFN will return filenames
			HANDLE hFind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATA ffd;
			hFind = FindFirstFile(szSearch, &ffd);
			if ( hFind != INVALID_HANDLE_VALUE )
			{
				do
				{
					if (!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) 
					{
						#ifdef _DEBUG
							logMessage(_T("Found file"), ffd.cFileName);
						#endif
						//files = new char[strlen(ffd.cFileName)];
						//files[filescount] = new char[strlen(ffd.cFileName)];
						TCHAR ext[5];
						tmemcpy(ext, ffd.cFileName + (_tcslen(ffd.cFileName)-4), 5);

						#ifdef _DEBUG
							logMessage(_T("Extention"), ext);
						#endif

						if (mir_tstrcmpi(ext, _T(".png")) & mir_tstrcmpi(ext, _T(".bmp")))
							continue;

						#ifdef _DEBUG
							logMessage(_T("File has valid ext"), ext);
						#endif
						_tcscpy_s(files[filescount++], ffd.cFileName);
					} //if
				} while (FindNextFile(hFind, &ffd));

				srand((unsigned) time(NULL));
				int r = 0;
				if (filescount) r = (rand() % filescount) + 1;

				mir_sntprintf(szSplashFile, SIZEOF(szSplashFile), _T("%s\\%s"), szSplashDir, files[r-1]);

				#ifdef _DEBUG
					logMessage(_T("final file"), szSplashFile);
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
		TCHAR buf [128];
	#endif
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	TCHAR * tszModule= mir_a2t(cws->szModule), *tszSetting = mir_a2t(cws->szSetting);
	if(options.inheritGS)
	{
		if (!mir_tstrcmp(tszModule, _T("Skin")) & !mir_tstrcmp(tszSetting, _T("UseSound")))
		{
			db_set_b(NULL, MODNAME, "PlaySound", cws->value.bVal);
			#ifdef _DEBUG
				cws->value.bVal ? _DebugPopup(NULL, _T("Sounds enabled."), _T("")) : _DebugPopup(NULL, _T("Sounds disabled."), _T(""));
				logMessage(_T("Module"), tszModule);
				logMessage(_T("Setting"), tszSetting);
				logMessage(_T("Value"), _itot(cws->value.bVal, buf, 10));
			#endif
		}
		if (!mir_tstrcmp(tszModule, _T("PluginDisable")) & (!mir_tstrcmp(tszSetting, szDllName)))
		{
			db_set_b(NULL, MODNAME, "Active", cws->value.bVal);
			#ifdef _DEBUG
				cws->value.bVal ? _DebugPopup(NULL, _T("Disabled."), "") : _DebugPopup(NULL, _T("Enabled."), _T(""));
				logMessage(_T("PlugDisableHook"), _T("Triggered"));
				logMessage(_T("Module"), tszModule);
				logMessage(_T("Setting"), tszSetting);
				logMessage(_T("Value"), _itot(cws->value.bVal, buf, 10));
			#endif
		}
	}
	mir_free(tszModule);
	mir_free(tszSetting);

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	bmodulesloaded = true; // all modules are loaded now, let other parts know about this fact

	if (hwndSplash)
	{
		if (PostMessage(hwndSplash, WM_LOADED, 0, 0))
		{
			#ifdef _DEBUG
				logMessage(_T("Posted WM_LOADED message"), _T("done"));
			#endif
		}
	}

	// Options initialize hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, PlugDisableHook);

	// Service to call splash
	CreateServiceFunction(MS_SHOWSPLASH, ShowSplashService);

	#ifdef _DEBUG
		logMessage(_T("Loading modules"), _T("done"));
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

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	SplashMain();
	mir_free(szMirDir);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hSplashThread)
		CloseHandle(hSplashThread);

	UnregisterClass(_T(SPLASH_CLASS), hInst);

	// Freeing loaded libraries
	if (hAdvaimg)
		FreeLibrary(hAdvaimg);

	#ifdef _DEBUG
		logMessage(_T("Unload"), _T("Job done"));
	#endif

	return 0;
}
