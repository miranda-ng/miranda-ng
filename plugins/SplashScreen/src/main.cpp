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

static HMODULE hUserDll = NULL;
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
HANDLE hShowSplashService, hTestService;
#ifdef _DEBUG
	TCHAR szLogFile[MAX_PATH];
#endif
SPLASHOPTS options;
HWND hwndSplash;
BOOL (WINAPI *MyUpdateLayeredWindow)
   (HWND hwnd, HDC hdcDST, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc,
    COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
HANDLE hSplashThread, hModulesLoaded, hPlugDisableHook, hOptInit, hSystemOKToExit;

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
		szMirDir = Utils_ReplaceVarsT(L"%miranda_path%");
		mir_sntprintf(szhAdvaimgPath, SIZEOF(szhAdvaimgPath), L"%s\\plugins\\advaimg.dll", szMirDir);
		CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVersion);

		#ifdef _DEBUG
			mir_sntprintf(szLogFile, SIZEOF(szLogFile), L"%s\\%s.log", szMirDir, _T(__PLUGIN_NAME));
			initLog();
			TCHAR* mirandaVerString = mir_a2t(szVersion);
			logMessage(L"Miranda version", mirandaVerString);
			mir_free(mirandaVerString);
			logMessage(L"Dll Name", _T(__FILENAME));
			logMessage(L"Advaimg path", szhAdvaimgPath);
		#endif

		ReadIniConfig();
	}

	if (bstartup & (options.active == 1))
	{
		if (options.runonce)
		{
			DBWriteContactSettingByte(NULL, MODNAME, "Active", 0);
			DBWriteContactSettingByte(NULL, MODNAME, "DisableAfterStartup", 0);
		}

		if (hUserDll == NULL)
		{
			hUserDll = LoadLibrary(L"user32.dll");
			if (hUserDll)
				MyUpdateLayeredWindow = (BOOL (WINAPI *)(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD))GetProcAddress(hUserDll, "UpdateLayeredWindow");
		}

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
						logMessage(L"Loading advaimg", L"done");
				#endif
			}
		}

		//for 9x "alfa" testing
		DBVARIANT dbv = {0};
		DBGetContactSettingTString(NULL, MODNAME, "VersionPrefix", &dbv);
		if (lstrcmp(dbv.ptszVal, NULL) == 0)
		{
			_tcscpy_s(szPrefix, L"");
			DBFreeVariant(&dbv);
		}
		else
			_tcscpy_s(szPrefix, dbv.ptszVal);
		dbv.ptszVal = NULL;

		DBGetContactSettingTString(NULL, MODNAME, "Path", &dbv);
		if (lstrcmp(dbv.ptszVal, NULL) == 0)
		{
			_tcscpy_s(inBuf, L"splash\\splash.png");
			DBFreeVariant(&dbv);
		}
		else
			_tcscpy_s(inBuf, dbv.ptszVal);
		dbv.ptszVal = NULL;

		TCHAR szExpandedSplashFile[MAX_PATH];
		ExpandEnvironmentStrings(inBuf, szExpandedSplashFile, SIZEOF(szExpandedSplashFile));
		lstrcpy(inBuf, szExpandedSplashFile);

		TCHAR *pos3 = 0;
		pos3 = _tcsrchr(inBuf, _T(':'));
		if (pos3 == NULL)
			mir_sntprintf(szSplashFile, SIZEOF(szSplashFile), L"%s\\%s", szMirDir, inBuf);
		else
			lstrcpy(szSplashFile, inBuf);

		DBGetContactSettingTString(NULL, MODNAME, "Sound", &dbv);
		if (lstrcmp(dbv.ptszVal, NULL) == 0)
		{
			_tcscpy_s(inBuf, L"sounds\\startup.wav");
			DBFreeVariant(&dbv);
		}
		else
			_tcscpy_s(inBuf, dbv.ptszVal);

		TCHAR szExpandedSoundFile[MAX_PATH];
		ExpandEnvironmentStrings(inBuf, szExpandedSoundFile, SIZEOF(szExpandedSoundFile));
		lstrcpy(inBuf, szExpandedSoundFile);

		TCHAR *pos2;
		pos2 = _tcschr(inBuf, _T(':'));
		if (pos2 == NULL)
			mir_sntprintf(szSoundFile, SIZEOF(szSoundFile), L"%s\\%s", szMirDir, inBuf);
		else
			lstrcpy(szSoundFile, inBuf);

		#ifdef _DEBUG
			logMessage(L"SoundFilePath", szSoundFile);
		#endif

		TCHAR szOldPath[MAX_PATH] = {0};

		if(options.random) // randomly select a splash file
		{
			int filescount = 0;
			TCHAR szSplashDir[MAX_PATH] = {0}, szSearch[MAX_PATH] = {0};
			TCHAR* p = 0;
			TCHAR files [255][50]; //TODO: make memory allocation dynamic

			lstrcpy(szSplashDir, szSplashFile);
			lstrcpy(szOldPath, szSplashFile);
			// find the last \ and null it out, this leaves no trailing slash
			p = _tcsrchr(szSplashDir, _T('\\'));
			if (p) *p = 0;
			// create the search filter
			mir_sntprintf(szSearch, SIZEOF(szSearch), L"%s\\*.*", szSplashDir);
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
							logMessage(L"Found file", ffd.cFileName);
						#endif
						//files = new char[strlen(ffd.cFileName)];
						//files[filescount] = new char[strlen(ffd.cFileName)];
						TCHAR ext[5];
						tmemcpy(ext, ffd.cFileName + (_tcslen(ffd.cFileName)-4), 5);

						#ifdef _DEBUG
							logMessage(L"Extention", ext);
						#endif

						if (lstrcmpi(ext, L".png") & lstrcmpi(ext, L".bmp"))
							continue;

						#ifdef _DEBUG
							logMessage(L"File has valid ext", ext);
						#endif
						_tcscpy_s(files[filescount++], ffd.cFileName);
					} //if
				} while (FindNextFile(hFind, &ffd));

				srand((unsigned) time(NULL));
				int r = 0;
				if (filescount) r = (rand() % filescount) + 1;

				mir_sntprintf(szSplashFile, SIZEOF(szSplashFile), L"%s\\%s", szSplashDir, files[r-1]);

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

int onSystemOKToExit(WPARAM wParam,LPARAM lParam)
{
	// Hooked events need to be unhooked
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hSystemOKToExit);
	UnhookEvent(hPlugDisableHook);
	UnhookEvent(hOptInit);

	DestroyServiceFunction(hShowSplashService);
	#ifdef _DEBUG
		DestroyServiceFunction(hTestService);
	#endif

	return 0;
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
		if (!lstrcmp(tszModule, L"Skin") & !lstrcmp(tszSetting, L"UseSound"))
		{
			DBWriteContactSettingByte(NULL, MODNAME, "PlaySound", cws->value.bVal);
			#ifdef _DEBUG
				cws->value.bVal ? _DebugPopup(NULL, L"Sounds enabled.", L"") : _DebugPopup(NULL, L"Sounds disabled.", L"");
				logMessage(L"Module", tszModule);
				logMessage(L"Setting", tszSetting);
				logMessage(L"Value", _itot(cws->value.bVal, buf, 10));
			#endif
		}
		if (!lstrcmp(tszModule, L"PluginDisable") & (!lstrcmp(tszSetting, szDllName)))
		{
			DBWriteContactSettingByte(NULL, MODNAME, "Active", cws->value.bVal);
			#ifdef _DEBUG
				cws->value.bVal ? _DebugPopup(NULL, L"Disabled.", "") : _DebugPopup(NULL, L"Enabled.", L"");
				logMessage(L"PlugDisableHook", L"Triggered");
				logMessage(L"Module", tszModule);
				logMessage(L"Setting", tszSetting);
				logMessage(L"Value", _itot(cws->value.bVal, buf, 10));
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
				logMessage(L"Posted WM_LOADED message", L"done");
			#endif
		}
	}

	// Options initialize hook
	hOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);

	hPlugDisableHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, PlugDisableHook);

	// Service to call splash
	hShowSplashService = CreateServiceFunction(MS_SHOWSPLASH, ShowSplashService);

	#ifdef _DEBUG
		hTestService = CreateServiceFunction("Splash/Test", TestService);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR;
		mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		mi.hotKey = 0;
		mi.position = -0x7FFFFFFF;
		mi.pszName = LPGEN("Call Splash Service");
		mi.pszService = "Splash/Test";
		Menu_AddMainMenuItem(&mi);
	#endif

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

	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hSystemOKToExit = HookEvent(ME_SYSTEM_OKTOEXIT,onSystemOKToExit);

	SplashMain();
	mir_free(szMirDir);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hSplashThread) CloseHandle(hSplashThread);

	UnregisterClass(_T(SPLASH_CLASS), hInst);

	// Freeing loaded libraries
	if (hUserDll) FreeLibrary(hUserDll);
	if (hAdvaimg) FreeLibrary(hAdvaimg);

	#ifdef _DEBUG
		logMessage(L"Unload", L"Job done");
	#endif

	return 0;
}