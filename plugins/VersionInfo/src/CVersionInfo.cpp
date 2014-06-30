/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

static int ValidExtension(TCHAR *fileName, TCHAR *extension)
{
	TCHAR *dot = _tcschr(fileName, '.');
	if ( dot != NULL && !lstrcmpi(dot + 1, extension))
		if (dot[lstrlen(extension) + 1] == 0)
			return 1;

	return 0;
}

void FillLocalTime(std::tstring &output, FILETIME *fileTime)
{
	TIME_ZONE_INFORMATION tzInfo = {0};
	FILETIME local = {0};
	SYSTEMTIME sysTime;
	TCHAR date[1024];
	TCHAR time[256];

	FileTimeToLocalFileTime(fileTime, &local);
	FileTimeToSystemTime(&local, &sysTime);

	GetDateFormat(EnglishLocale, 0, &sysTime, _T("dd' 'MMM' 'yyyy"), date, SIZEOF(date));
	GetTimeFormat(NULL, TIME_FORCE24HOURFORMAT, &sysTime, _T("HH':'mm':'ss"), time, SIZEOF(time)); //americans love 24hour format ;)
	output = std::tstring(date) + _T(" at ") + std::tstring(time);

	int res = GetTimeZoneInformation(&tzInfo);
	char tzName[32] = {0};
	TCHAR tzOffset[64] = {0};
	int offset = 0;
	switch (res) {
	case TIME_ZONE_ID_DAYLIGHT:
		offset = -(tzInfo.Bias + tzInfo.DaylightBias);
		WideCharToMultiByte(CP_ACP, 0, tzInfo.DaylightName, -1, tzName, SIZEOF(tzName), NULL, NULL);
		break;

	case TIME_ZONE_ID_STANDARD:
		WideCharToMultiByte(CP_ACP, 0, tzInfo.StandardName, -1, tzName, SIZEOF(tzName), NULL, NULL);
		offset = -(tzInfo.Bias + tzInfo.StandardBias);
		break;

	case TIME_ZONE_ID_UNKNOWN:
		WideCharToMultiByte(CP_ACP, 0, tzInfo.StandardName, -1, tzName, SIZEOF(tzName), NULL, NULL);
		offset = -tzInfo.Bias;
		break;
	}

	mir_sntprintf(tzOffset, SIZEOF(tzOffset), _T("UTC %+02d:%02d"), offset / 60, offset % 60);
	output += _T(" (") + std::tstring(tzOffset) + _T(")");
}

CVersionInfo::CVersionInfo()
{
	luiFreeDiskSpace = 0;
	bDEPEnabled = 0;
}

CVersionInfo::~CVersionInfo()
{
	listInactivePlugins.clear();
	listActivePlugins.clear();
	listUnloadablePlugins.clear();

	lpzMirandaVersion.~basic_string();
	lpzNightly.~basic_string();
	lpzUnicodeBuild.~basic_string();
	lpzBuildTime.~basic_string();
	lpzMirandaPath.~basic_string();
	lpzCPUName.~basic_string();
	lpzCPUIdentifier.~basic_string();
};

void CVersionInfo::Initialize()
{
#ifdef _DEBUG
		if (verbose) PUShowMessage("Before GetMirandaVersion().", SM_NOTIFY);
#endif
	GetMirandaVersion();

#ifdef _DEBUG
		if (verbose) PUShowMessage("Before GetProfileSettings().", SM_NOTIFY);
#endif
	GetProfileSettings();

#ifdef _DEBUG
		if (verbose) PUShowMessage("Before GetLangpackInfo().", SM_NOTIFY);
#endif
	GetOSLanguages();
	GetLangpackInfo();

#ifdef _DEBUG
	if (verbose) PUShowMessage("Before GetPluginLists().", SM_NOTIFY);
#endif
	GetPluginLists();

#ifdef _DEBUG
	if (verbose) PUShowMessage("Before GetOSVersion().", SM_NOTIFY);
#endif
	GetOSVersion();

#ifdef _DEBUG
	if (verbose) PUShowMessage("Before GetHWSettings().", SM_NOTIFY);
#endif
	GetHWSettings();

#ifdef _DEBUG
	if (verbose) PUShowMessage("Done with GetHWSettings().", SM_NOTIFY);
#endif
}

bool CVersionInfo::GetMirandaVersion()
{
	//Miranda version
	const BYTE str_size = 64;
	char str[str_size];
	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)str_size, (LPARAM)str);
	this->lpzMirandaVersion = _A2T(str);
	//Is it a nightly?
	if (lpzMirandaVersion.find( _T("alpha"), 0) != std::string::npos)
		lpzNightly = _T("Yes");
	else
		lpzNightly = _T("No");

	lpzUnicodeBuild = _T("Yes");

	TCHAR mirtime[128];
	GetModuleTimeStamp(mirtime, 128);
	lpzBuildTime = mirtime;
	return TRUE;
}

bool CVersionInfo::GetOSVersion()
{
	//Operating system informations
	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	//OSName
	//Let's read the registry.
	HKEY hKey;
	TCHAR szKey[MAX_PATH], szValue[MAX_PATH];
	switch (osvi.dwPlatformId) {
	case VER_PLATFORM_WIN32_WINDOWS:
		lstrcpyn(szKey, _T("Software\\Microsoft\\Windows\\CurrentVersion"), MAX_PATH);
		lstrcpyn(szValue, _T("Version"), MAX_PATH);
		break;

	case VER_PLATFORM_WIN32_NT:
		lstrcpyn(szKey, _T("Software\\Microsoft\\Windows NT\\CurrentVersion"), MAX_PATH);
		lstrcpyn(szValue, _T("ProductName"), MAX_PATH);
		break;
	}

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		DWORD type, size, result;
		//Get the size of the value we'll read.
		result = RegQueryValueEx((HKEY)hKey, szValue, (LPDWORD)NULL, (LPDWORD)&type, (LPBYTE)NULL, (LPDWORD)&size);
		if (result == ERROR_SUCCESS) {
			//Read it.
			TCHAR *aux = new TCHAR[size+1];
			result = RegQueryValueEx((HKEY)hKey, szValue, (LPDWORD)NULL, (LPDWORD)&type, (LPBYTE)aux, (LPDWORD)&size);
			lpzOSName.append(_T("Microsoft "));
			lpzOSName.append(aux);
			lpzOSName.append(_T(" Edition"));
			delete[] aux;
		}
		else {
			NotifyError(GetLastError(), _T("RegQueryValueEx()"), __LINE__);
			lpzOSName = _T("<Error in RegQueryValueEx()>");
		}
		RegCloseKey(hKey);
	}
	else {
		NotifyError(GetLastError(), _T("RegOpenKeyEx()"), __LINE__);
		lpzOSName = _T("<Error in RegOpenKeyEx()>");
	}

	//Now we can improve it if we can.
	switch (LOWORD(osvi.dwBuildNumber)) {
	case 950: lpzOSName = _T("Microsoft Windows 95"); break;
	case 1111: lpzOSName = _T("Microsoft Windows 95 OSR2"); break;
	case 1998: lpzOSName = _T("Microsoft Windows 98"); break;
	case 2222: lpzOSName = _T("Microsoft Windows 98 SE"); break;
	case 3000: lpzOSName = _T("Microsoft Windows ME"); break; //Even if this is wrong, we have already read it in the registry.
	case 1381: lpzOSName = _T("Microsoft Windows NT"); break; //What about service packs?
	case 2195: lpzOSName = _T("Microsoft Windows 2000"); break; //What about service packs?
	case 2600: lpzOSName = _T("Microsoft Windows XP"); break;
	case 3790:
		if (GetSystemMetrics(89)) //R2 ?
			lpzOSName = _T("Microsoft Windows 2003 R2");
		else
			lpzOSName = _T("Microsoft Windows 2003");

		break; //added windows 2003 info
	}

	SYSTEM_INFO si = {0};
	GetNativeSystemInfo(&si);

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		lpzOSName.append(_T(", 64-bit "));
	else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
		lpzOSName.append(_T(", 32-bit "));

	lpzOSName.append(osvi.szCSDVersion);
	lpzOSName.append(_T(" (build "));
	TCHAR buildno[MAX_PATH];
	_itot(osvi.dwBuildNumber, buildno, 10);
	lpzOSName.append(buildno);
	lpzOSName.append(_T(")"));

	return TRUE;
}

bool CVersionInfo::GetHWSettings()
{
	//CPU Info
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Hardware\\Description\\System\\CentralProcessor\\0"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		TCHAR cpuIdent[512] = {0}, cpuName[512] = {0};
		DWORD size;
		size = SIZEOF(cpuName);
		if (RegQueryValueEx(hKey, TEXT("ProcessorNameString"), NULL, NULL, (LPBYTE) cpuName, &size) != ERROR_SUCCESS)
			_tcscpy(cpuName, _T("Unknown"));
		lpzCPUName = cpuName;
		size = SIZEOF(cpuIdent);
		if (RegQueryValueEx(hKey, TEXT("Identifier"), NULL, NULL, (LPBYTE) cpuIdent, &size) != ERROR_SUCCESS)
			if (RegQueryValueEx(hKey, TEXT("VendorIdentifier"), NULL, NULL, (LPBYTE) cpuIdent, &size) != ERROR_SUCCESS)
				_tcscpy(cpuIdent, _T("Unknown"));
		lpzCPUIdentifier = cpuIdent;

		RegCloseKey(hKey);
	}

	while (true)
	{
		std::tstring::size_type pos = lpzCPUName.find(_T("  "));
		if (pos != std::tstring::npos)
			lpzCPUName.replace(lpzCPUName.begin() + pos, lpzCPUName.begin() + pos + 2, _T(" "));
		else
			break;
	}

	bDEPEnabled = IsProcessorFeaturePresent(PF_NX_ENABLED);

	//Free space on Miranda Partition.
	TCHAR szMirandaPath[MAX_PATH] = { 0 };
	GetModuleFileName(GetModuleHandle(NULL), szMirandaPath, SIZEOF(szMirandaPath));
	TCHAR* str2 = _tcsrchr(szMirandaPath,'\\');
	if ( str2 != NULL)
		*str2=0;
	ULARGE_INTEGER FreeBytes, a, b;
	GetDiskFreeSpaceEx(szMirandaPath, &FreeBytes, &a, &b);
	//Now we need to convert it.
	__int64 aux = FreeBytes.QuadPart;
	aux /= (1024*1024);
	luiFreeDiskSpace = (unsigned long int)aux;

	TCHAR szInfo[1024];
	GetWindowsShell(szInfo, SIZEOF(szInfo));
	lpzShell = szInfo;
	GetInternetExplorerVersion(szInfo, SIZEOF(szInfo));
	lpzIEVersion = szInfo;


	lpzAdministratorPrivileges = (IsCurrentUserLocalAdministrator()) ? _T("Yes") : _T("No");

	bIsWOW64 = 0;
	if (!IsWow64Process(GetCurrentProcess(), &bIsWOW64))
		bIsWOW64 = 0;

	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	luiProcessors = sysInfo.dwNumberOfProcessors;

	//Installed RAM
	MEMORYSTATUSEX ms = {0};
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatusEx(&ms);
	luiRAM = (unsigned int) ((ms.ullTotalPhys / (1024 * 1024)) + 1);

	return TRUE;
}

bool CVersionInfo::GetProfileSettings()
{
	TCHAR* tszProfileName = Utils_ReplaceVarsT(_T("%miranda_userdata%\\%miranda_profilename%.dat"));
	lpzProfilePath = tszProfileName;

	WIN32_FIND_DATA fd;
	if ( FindFirstFile(tszProfileName, &fd) != INVALID_HANDLE_VALUE ) {
		TCHAR number[40];
		mir_sntprintf(number, SIZEOF(number), _T("%.2f KBytes"), double(fd.nFileSizeLow) / 1024);
		lpzProfileSize = number;

		FILETIME ftLocal;
		SYSTEMTIME stLocal;
		TCHAR lpszString[128];
		FileTimeToLocalFileTime(&fd.ftCreationTime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &stLocal);
		GetISO8061Time(&stLocal, lpszString, 128);
		lpzProfileCreationDate = lpszString;
	}
	else {
		DWORD error = GetLastError();
		TCHAR tmp[1024];
		mir_sntprintf(tmp, SIZEOF(tmp), _T("%d"), error);
		lpzProfileCreationDate = _T("<error ") + std::tstring(tmp) + _T(" at FileOpen>") + std::tstring(tszProfileName);
		lpzProfileSize = _T("<error ") + std::tstring(tmp) + _T(" at FileOpen>") + std::tstring(tszProfileName);
	}

	mir_free( tszProfileName );
	return true;
}

static TCHAR szSystemLocales[4096] = {0};
static WORD systemLangID;
#define US_LANG_ID 0x00000409

BOOL CALLBACK EnumSystemLocalesProc(TCHAR *szLocale)
{
	DWORD locale = _ttoi(szLocale);
	TCHAR *name = GetLanguageName(locale);
	if ( !_tcsstr(szSystemLocales, name)) {
		_tcscat(szSystemLocales, name);
		_tcscat(szSystemLocales, _T(", "));
	}

	return TRUE;
}

BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam)
{
	if (!lpszName)
	return FALSE;

	if (wIDLanguage !=  US_LANG_ID)
	systemLangID = wIDLanguage;

	return TRUE;
}

bool CVersionInfo::GetOSLanguages()
{
	lpzOSLanguages = _T("(UI | Locale (User/System)) : ");

	LANGID UILang;

	UILang = GetUserDefaultUILanguage();
	lpzOSLanguages += GetLanguageName(UILang);
	lpzOSLanguages += _T("/");
	UILang = GetSystemDefaultUILanguage();
	lpzOSLanguages += GetLanguageName(UILang);

	lpzOSLanguages += _T(" | ");
	lpzOSLanguages += GetLanguageName(LOCALE_USER_DEFAULT);
	lpzOSLanguages += _T("/");
	lpzOSLanguages += GetLanguageName(LOCALE_SYSTEM_DEFAULT);

	if (db_get_b(NULL, ModuleName, "ShowInstalledLanguages", 0)) {
		szSystemLocales[0] = '\0';
		lpzOSLanguages += _T(" [");
		EnumSystemLocales(EnumSystemLocalesProc, LCID_INSTALLED);
		if (_tcslen(szSystemLocales) > 2)
			szSystemLocales[ _tcslen(szSystemLocales) - 2] = '\0';

		lpzOSLanguages += szSystemLocales;
		lpzOSLanguages += _T("]");
	}

	return true;
}

int SaveInfo(const char *data, const char *lwrData, const char *search, TCHAR *dest, int size)
{
	const char *pos = strstr(lwrData, search);
	int res = 1;
	if (pos == lwrData) {
		_tcsncpy(dest, _A2T(&data[strlen(search)]), size);
		res = 0;
	}

	return res;
}

bool CVersionInfo::GetLangpackInfo()
{
	LCID packlcid = Langpack_GetDefaultLocale();
	
	if (packlcid != LOCALE_USER_DEFAULT) {
		TCHAR lang[MAX_PATH], ctry[MAX_PATH];
		if(GetLocaleInfo(packlcid, LOCALE_SENGLANGUAGE, lang, MAX_PATH)) {
			if(GetLocaleInfo(packlcid, LOCALE_SISO3166CTRYNAME, ctry, MAX_PATH)) {
				TCHAR langpackInfo[MAX_PATH];
				mir_sntprintf(langpackInfo,SIZEOF(langpackInfo),TEXT("%s (%s) [%04x]"), lang, ctry, packlcid);
				lpzLangpackInfo = langpackInfo;
			}
			else
				lpzLangpackInfo.append(lang);
		}
		else
			lpzLangpackInfo = _T("Locale id invalid");
	}
	else 
		lpzLangpackInfo = _T("No language pack installed");
	return true;
}

/*bool CVersionInfo::GetWeatherInfo()
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	LPTSTR fname = _tcsrchr(path, TEXT('\\'));
	if (fname == NULL)
		fname = path;
	_tcscat(fname, _T("\\plugins\\weather\\*.ini"));

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do 
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		crs_sntprintf(fname, MAX_PATH-(fname-path), TEXT("\\plugins\\weather\\%s"), FindFileData.cFileName);
		HANDLE hDumpFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hDumpFile != INVALID_HANDLE_VALUE) 
		{
			char buf[8192];

			DWORD bytes = 0;
			ReadFile(hDumpFile, buf, 8190, &bytes, NULL);
			buf[bytes] = 0;

			char* ver = strstr(buf, "Version=");
			if (ver != NULL)
			{
				char *endid = strchr(ver, '\r');
				if (endid != NULL) *endid = 0;
				else
				{
					endid = strchr(ver, '\n');
					if (endid != NULL) *endid = 0;
				}
				ver += 8;
			}

			char *id = strstr(buf, "Name=");
			if (id != NULL)
			{
				char *endid = strchr(id, '\r');
				if (endid != NULL) *endid = 0;
				else
				{
					endid = strchr(id, '\n');
					if (endid != NULL) *endid = 0;
				}
				id += 5; 
			}

			TCHAR timebuf[30] = TEXT("");
			GetLastWriteTime(&FindFileData.ftLastWriteTime, timebuf, 30);


			static const TCHAR format[] = TEXT(" %s v.%s%S%s [%s] - %S\r\n");

			buffer.appendfmt(format, FindFileData.cFileName, 
				(flags & VI_FLAG_FORMAT) ? TEXT("[b]") : TEXT(""),
				ver,
				(flags & VI_FLAG_FORMAT) ? TEXT("[/b]") : TEXT(""),
				timebuf, id);
			CloseHandle(hDumpFile);
		}
	}
	while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
}*/

std::tstring GetPluginTimestamp(FILETIME *fileTime)
{
	SYSTEMTIME sysTime;
	FILETIME ftLocal;
	FileTimeToLocalFileTime(fileTime, &ftLocal);
	FileTimeToSystemTime(&ftLocal, &sysTime); //convert the file tyme to system time
	TCHAR date[256]; //lovely
	GetISO8061Time(&sysTime, date, 256);
	return date;
}

bool CVersionInfo::GetPluginLists()
{
	HANDLE hFind;
	TCHAR szMirandaPath[MAX_PATH] = { 0 }, szSearchPath[MAX_PATH] = { 0 }; //For search purpose
	WIN32_FIND_DATA fd;
	TCHAR szMirandaPluginsPath[MAX_PATH] = { 0 }, szPluginPath[MAX_PATH] =  { 0 }; //For info reading purpose
	BYTE PluginIsEnabled = 0;
	HINSTANCE hInstPlugin = NULL;
	PLUGININFOEX *(*MirandaPluginInfo)(DWORD); //These two are used to get informations from the plugin.
	PLUGININFOEX *pluginInfo = NULL; //Read above.
	DWORD mirandaVersion = 0;
	BOOL asmCheckOK = FALSE;
	DWORD loadError;
	//	SYSTEMTIME sysTime; //for timestamp

	bWeatherPlugin = false;
	mirandaVersion = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);
	{
		GetModuleFileName(GetModuleHandle(NULL), szMirandaPath, SIZEOF(szMirandaPath));
		TCHAR* str2 = _tcsrchr(szMirandaPath,'\\');
		if(str2!=NULL) *str2=0;
	}
	lpzMirandaPath = szMirandaPath;

	//We got Miranda path, now we'll use it for two different purposes.
	//1) finding plugins.
	//2) Reading plugin infos
	lstrcpyn(szSearchPath,szMirandaPath, MAX_PATH); //We got the path, now we copy it into szSearchPath. We'll use szSearchPath as am auxiliary variable, while szMirandaPath will keep a "fixed" value.
	lstrcat(szSearchPath, _T("\\Plugins\\*.dll"));

	lstrcpyn(szMirandaPluginsPath, szMirandaPath, MAX_PATH);
	lstrcat(szMirandaPluginsPath, _T("\\Plugins\\"));

	hFind=FindFirstFile(szSearchPath,&fd);
	if ( hFind != INVALID_HANDLE_VALUE) {
		do {
			if (verbose) PUShowMessageT(fd.cFileName, SM_NOTIFY);
			if (!ValidExtension(fd.cFileName, _T("dll")))
				continue; //do not report plugins that do not have extension .dll

			if (_tcsicmp(fd.cFileName, _T("weather.dll")) == 0)
				bWeatherPlugin = true;

			hInstPlugin = GetModuleHandle(fd.cFileName); //try to get the handle of the module

			if (hInstPlugin) //if we got it then the dll is loaded (enabled)
				PluginIsEnabled = 1;
			else {
				PluginIsEnabled = 0;
				lstrcpyn(szPluginPath, szMirandaPluginsPath, MAX_PATH); // szPluginPath becomes "drive:\path\Miranda\Plugins\"
				lstrcat(szPluginPath, fd.cFileName); // szPluginPath becomes "drive:\path\Miranda\Plugins\popup.dll"
				hInstPlugin = LoadLibrary(szPluginPath);
			}
			if (!hInstPlugin) { //It wasn't loaded.
				loadError = GetLastError();
				int bUnknownError = 1; //assume plugin didn't load because of unknown error
				//Some error messages.
				//get the dlls the plugin statically links to
				if (db_get_b(NULL, ModuleName, "CheckForDependencies", TRUE))
				{
					std::tstring linkedModules;

					lstrcpyn(szPluginPath, szMirandaPluginsPath, MAX_PATH); // szPluginPath becomes "drive:\path\Miranda\Plugins\"
					lstrcat(szPluginPath, fd.cFileName); // szPluginPath becomes "drive:\path\Miranda\Plugins\popup.dll"
					if (GetLinkedModulesInfo(szPluginPath, linkedModules)) {
						std::tstring time = GetPluginTimestamp(&fd.ftLastWriteTime);
						CPlugin thePlugin(fd.cFileName, _T("<unknown>"), UUID_NULL, _T(""), 0, time.c_str(), linkedModules.c_str());
						AddPlugin(thePlugin, listUnloadablePlugins);
						bUnknownError = 0; //we know why the plugin didn't load
					}
				}
				if (bUnknownError) { //if cause is unknown then report it
					std::tstring time = GetPluginTimestamp(&fd.ftLastWriteTime);
					TCHAR buffer[4096];
					TCHAR error[2048];
					//DWORD_PTR arguments[2] = {loadError, 0};
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, loadError, 0, error, SIZEOF(error), NULL);
					mir_sntprintf(buffer, SIZEOF(buffer), _T("    Error %ld - %s"), loadError, error);
					CPlugin thePlugin( fd.cFileName, _T("<unknown>"), UUID_NULL, _T(""), 0, time.c_str(), buffer);
					AddPlugin(thePlugin, listUnloadablePlugins);
				}
			}
			else { //It was successfully loaded.
				MirandaPluginInfo = (PLUGININFOEX *(*)(DWORD))GetProcAddress(hInstPlugin, "MirandaPluginInfoEx");
				if (!MirandaPluginInfo)
					MirandaPluginInfo = (PLUGININFOEX *(*)(DWORD))GetProcAddress(hInstPlugin, "MirandaPluginInfo");

				if (!MirandaPluginInfo) //There is no function: it's not a valid plugin. Let's move on to the next file.
					continue;

				//It's a valid plugin, since we could find MirandaPluginInfo
				#if (!defined(WIN64) && !defined(_WIN64))
					asmCheckOK = FALSE;
					__asm {
						push mirandaVersion
						push mirandaVersion
						call MirandaPluginInfo
						pop eax
						pop eax
						cmp eax, mirandaVersion
						jne a1
						mov asmCheckOK, 0xffffffff
					a1:
					}
				#else
					asmCheckOK = TRUE;
				#endif
				if (asmCheckOK)
					pluginInfo = CopyPluginInfo(MirandaPluginInfo(mirandaVersion));
				else {
					ZeroMemory(&pluginInfo, sizeof(pluginInfo));
					MessageBox(NULL, fd.cFileName, _T("Invalid plugin"), MB_OK);
				}
			}

			//Let's get the info.
			if (MirandaPluginInfo == NULL || pluginInfo == NULL) {
				FreeLibrary(hInstPlugin); //We don't need it anymore.
				continue;
			}

			//We have loaded the informations into pluginInfo.
			std::tstring timedate = GetPluginTimestamp(&fd.ftLastWriteTime);
			CPlugin thePlugin(fd.cFileName, _A2T(pluginInfo->shortName), pluginInfo->uuid, /*(pluginInfo->flags & 1) ? _T("Unicode aware") :*/ _T(""), (DWORD) pluginInfo->version, timedate.c_str(), _T(""));

			if (PluginIsEnabled)
				AddPlugin(thePlugin, listActivePlugins);
			else {
				AddPlugin(thePlugin, listInactivePlugins);
				FreeLibrary(hInstPlugin); //We don't need it anymore.
			}
			FreePluginInfo(pluginInfo);
			MirandaPluginInfo = NULL;
		}
			while (FindNextFile(hFind,&fd));
		FindClose(hFind);
	}
	return TRUE;
}

bool CVersionInfo::AddPlugin(CPlugin &aPlugin, std::list<CPlugin> &aList)
{
	std::list<CPlugin>::iterator pos = aList.begin();
	bool inserted = FALSE;

	if (aList.begin() == aList.end()) { //It's empty
		aList.push_back(aPlugin);
		return TRUE;
	}
	else { //It's not empty
		while (pos != aList.end()) {
			//It can be either < or >, not equal.
			if (aPlugin < (*pos)) {
				aList.insert(pos, aPlugin);
				return TRUE;
			}

			//It's greater: we need to insert it.
			pos++;
	}	}

	if (inserted == FALSE) {
		aList.push_back(aPlugin);
		return TRUE;
	}
	return TRUE;
};

static char *GetStringFromRVA(DWORD RVA, const LOADED_IMAGE *image)
{
	char *moduleName;
	moduleName = (char *) ImageRvaToVa(image->FileHeader, image->MappedAddress, RVA, NULL);
	return moduleName;
}

bool CVersionInfo::GetLinkedModulesInfo(TCHAR *moduleName, std::tstring &linkedModules)
{
	LOADED_IMAGE image;
	ULONG importTableSize;
	IMAGE_IMPORT_DESCRIPTOR *importData;
	//HMODULE dllModule;
	linkedModules = _T("");
	bool result = false;
	TCHAR szError[20];
	char* szModuleName = mir_t2a(moduleName);
	if (MapAndLoad(szModuleName, NULL, &image, TRUE, TRUE) == FALSE) {
		mir_sntprintf(szError, SIZEOF(szError), _T("%d"), GetLastError());
		mir_free(szModuleName);
		linkedModules = _T("<error ") + std::tstring(szError) + _T(" at MapAndLoad()>\r\n");
		return result;
	}
	mir_free(szModuleName);
	importData = (IMAGE_IMPORT_DESCRIPTOR *) ImageDirectoryEntryToData(image.MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_IMPORT, &importTableSize);
	if (!importData) {
		mir_sntprintf(szError, SIZEOF(szError), _T("%d"), GetLastError());
		linkedModules = _T("<error ") + std::tstring(szError) + _T(" at ImageDirectoryEntryToDataEx()>\r\n");
	}
	else {
		while (importData->Name) {
			char *moduleName;
			moduleName = GetStringFromRVA(importData->Name, &image);
			if (!DoesDllExist(moduleName)) {
				linkedModules.append( _T("    Plugin statically links to missing dll file: ") + std::tstring(_A2T(moduleName)) + _T("\r\n"));
				result = true;
			}

			importData++; //go to next record
	}	}

	//	FreeLibrary(dllModule);
	UnMapAndLoad(&image); //unload the image
	return result;
}

std::tstring CVersionInfo::GetListAsString(std::list<CPlugin> &aList, DWORD flags, int beautify) {
	std::list<CPlugin>::iterator pos = aList.begin();
	std::tstring out = _T("");
#ifdef _DEBUG
	if (verbose) PUShowMessage("CVersionInfo::GetListAsString, begin.", SM_NOTIFY);
#endif

	TCHAR szHeader[32] = {0};
	TCHAR szFooter[32] = {0};
	if ((((flags & VISF_FORUMSTYLE) == VISF_FORUMSTYLE) || beautify) && (db_get_b(NULL, ModuleName, "BoldVersionNumber", TRUE))) {
		GetStringFromDatabase("BoldBegin", _T("[b]"), szHeader, SIZEOF(szHeader));
		GetStringFromDatabase("BoldEnd", _T("[/b]"), szFooter, SIZEOF(szFooter));
	}

	while (pos != aList.end()) {
		out.append(std::tstring((*pos).getInformations(flags, szHeader, szFooter)));
		pos++;
	}
	#ifdef _DEBUG
		if (verbose) PUShowMessage("CVersionInfo::GetListAsString, end.", SM_NOTIFY);
	#endif
	return out;
};

void CVersionInfo::BeautifyReport(int beautify, LPCTSTR szBeautifyText, LPCTSTR szNonBeautifyText, std::tstring &out)
{
	if (beautify)
		out.append(szBeautifyText);
	else
		out.append(szNonBeautifyText);
}

void CVersionInfo::AddInfoHeader(int suppressHeader, int forumStyle, int beautify, std::tstring &out)
{
	if (forumStyle) { //forum style
		TCHAR szSize[256], szQuote[256];

		GetStringFromDatabase("SizeBegin", _T("[quote]"), szSize, SIZEOF(szSize));
		GetStringFromDatabase("QuoteBegin", _T("[spoiler=VersionInfo]"), szQuote, SIZEOF(szQuote));
		out.append(szQuote);
		out.append(szSize);
	}
	else out = _T("");

	if (!suppressHeader) {
		out.append( _T("Miranda NG - VersionInformation plugin by Hrk, modified by Eblis\r\n"));
		if (!forumStyle) {
			out.append( _T("Miranda's homepage: http://miranda-ng.org/\r\n")); //changed homepage
			out.append( _T("Miranda tools: http://miranda-ng.org/\r\n\r\n")); //was missing a / before download
	}	}

	TCHAR buffer[1024]; //for beautification
	GetStringFromDatabase("BeautifyHorizLine", _T("<hr />"), buffer, SIZEOF(buffer));
	BeautifyReport(beautify, buffer, _T(""), out);
	GetStringFromDatabase("BeautifyBlockStart", _T("<blockquote>"), buffer, SIZEOF(buffer));
	BeautifyReport(beautify, buffer, _T(""), out);
	if (!suppressHeader) {
		//Time of report:
		TCHAR lpzTime[12]; 	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("HH':'mm':'ss"), lpzTime, SIZEOF(lpzTime));
		TCHAR lpzDate[32]; 	GetDateFormat(EnglishLocale, 0, NULL, _T("dd' 'MMMM' 'yyyy"), lpzDate, SIZEOF(lpzDate));
		out.append( _T("Report generated at: ") + std::tstring(lpzTime) + _T(" on ") + std::tstring(lpzDate) + _T("\r\n\r\n"));
	}

	//Operating system
	out.append(_T("CPU: ") + lpzCPUName + _T(" [") + lpzCPUIdentifier + _T("]"));
	if (bDEPEnabled)
		out.append(_T(" [DEP enabled]"));

	if (luiProcessors > 1) {
		TCHAR noProcs[128];
		mir_sntprintf(noProcs, SIZEOF(noProcs), _T(" [%d CPUs]"), luiProcessors);
		out.append(noProcs);
	}
	out.append( _T("\r\n"));

	//RAM
	TCHAR szRAM[64];
	mir_sntprintf(szRAM, SIZEOF(szRAM), _T("%d"), luiRAM);
	out.append( _T("Installed RAM: ") + std::tstring(szRAM) + _T(" MBytes\r\n"));

	//operating system
	out.append( _T("Operating System: ") + lpzOSName + _T("\r\n"));

	//shell, IE, administrator
	out.append( _T("Shell: ") + lpzShell + _T("\r\n"));
	out.append( _T("Internet Explorer: ") + lpzIEVersion + _T("\r\n"));
	out.append( _T("Administrator privileges: ") + lpzAdministratorPrivileges + _T("\r\n"));

	//languages
	out.append( _T("OS Languages: ") + lpzOSLanguages + _T("\r\n"));

	//FreeDiskSpace
	if (luiFreeDiskSpace) {
		TCHAR szDiskSpace[64];
		mir_sntprintf(szDiskSpace, SIZEOF(szDiskSpace), _T("%d"), luiFreeDiskSpace);
		out.append( _T("Free disk space on Miranda partition: ") + std::tstring(szDiskSpace) + _T(" MBytes\r\n\r\n"));
	}

	//Miranda
	out.append( _T("Miranda path: ")	+ lpzMirandaPath + _T("\r\n"));
	out.append( _T("Miranda NG version: ") + lpzMirandaVersion);
	if (bIsWOW64)
		out.append( _T(" [running inside WOW64]"));

	out.append( _T("\r\nBuild time: ") + lpzBuildTime + _T("\r\n"));
	out.append( _T("Profile path: ") + lpzProfilePath + _T("\r\n"));
	out.append( _T("Profile size: ") + lpzProfileSize + _T("\r\n"));
	out.append( _T("Profile creation date: ") + lpzProfileCreationDate + _T("\r\n"));
	out.append( _T("Language pack: ") + lpzLangpackInfo);
	out.append((lpzLangpackModifiedDate.size() > 0) ? _T(", modified: ") + lpzLangpackModifiedDate : _T(""));
	out.append( _T("\r\n"));
	out.append(_T("Service Mode: "));
	if (bServiceMode)
		out.append( _T("Yes"));
	else
		out.append( _T("No"));
	
	// out.append( _T("Nightly: ") + lpzNightly + _T("\r\n"));
	// out.append( _T("Unicode core: ") + lpzUnicodeBuild);

	GetStringFromDatabase("BeautifyBlockEnd", _T("</blockquote>"), buffer, SIZEOF(buffer));
	BeautifyReport(beautify, buffer, _T("\r\n"), out);
}

void CVersionInfo::AddInfoFooter(int suppressFooter, int forumStyle, int beautify, std::tstring &out)
{
	//End of report
	TCHAR buffer[1024]; //for beautification purposes
	GetStringFromDatabase("BeautifyHorizLine", _T("<hr />"), buffer, SIZEOF(buffer));
	if (!suppressFooter) {
		BeautifyReport(beautify, buffer, _T("\r\n"), out);
		out.append( _T("\r\nEnd of report.\r\n"));
	}

	if (!forumStyle) {
		if (!suppressFooter)
			out.append( TranslateT("If you are going to use this report to submit a bug, remember to check the website for questions or help the developers may need.\r\nIf you don't check your bug report and give feedback, it will not be fixed!"));
	}
	else {
		TCHAR szSize[256], szQuote[256];
		GetStringFromDatabase("SizeEnd", _T("[/quote]"), szSize, SIZEOF(szSize));
		GetStringFromDatabase("QuoteEnd", _T("[/spoiler]"), szQuote, SIZEOF(szQuote));
		out.append(szSize);
		out.append(szQuote);
	}
}

static void AddSectionAndCount(std::list<CPlugin> list, LPCTSTR listText, std::tstring &out)
{
	TCHAR tmp[64];
	mir_sntprintf(tmp, SIZEOF(tmp), _T(" (%u)"), list.size());
	out.append(listText);
	out.append(tmp);
	out.append( _T(":"));
}

std::tstring CVersionInfo::GetInformationsAsString(int bDisableForumStyle) {
	//Begin of report
	std::tstring out;
	int forumStyle = (bDisableForumStyle) ? 0 : db_get_b(NULL, ModuleName, "ForumStyle", TRUE);
	int showUUID = db_get_b(NULL, ModuleName, "ShowUUIDs", FALSE);
	int beautify = db_get_b(NULL, ModuleName, "Beautify", 0) & (!forumStyle);
	int suppressHeader = db_get_b(NULL, ModuleName, "SuppressHeader", TRUE);

	DWORD flags = (forumStyle) | (showUUID << 1);

	AddInfoHeader(suppressHeader, forumStyle, beautify, out);
	TCHAR normalPluginsStart[1024]; //for beautification purposes, for normal plugins text (start)
	TCHAR normalPluginsEnd[1024]; //for beautification purposes, for normal plugins text (end)
	TCHAR horizLine[1024]; //for beautification purposes
	TCHAR buffer[1024]; //for beautification purposes

	TCHAR headerHighlightStart[10] = _T("");
	TCHAR headerHighlightEnd[10] = _T("");
	if (forumStyle) {
		TCHAR start[128], end[128];
		GetStringFromDatabase("BoldBegin", _T("[b]"), start, SIZEOF(start));
		GetStringFromDatabase("BoldEnd", _T("[/b]"), end, SIZEOF(end));
		_tcsncpy(headerHighlightStart, start, SIZEOF(headerHighlightStart));
		_tcsncpy(headerHighlightEnd, end, SIZEOF(headerHighlightEnd));
	}

	//Plugins: list of active (enabled) plugins.
	GetStringFromDatabase("BeautifyHorizLine", _T("<hr />"), horizLine, SIZEOF(horizLine));
	BeautifyReport(beautify, horizLine, _T("\r\n"), out);
	GetStringFromDatabase("BeautifyActiveHeaderBegin", _T("<b><font size=\"-1\" color=\"DarkGreen\">"), buffer, SIZEOF(buffer));
	BeautifyReport(beautify, buffer, headerHighlightStart, out);
	AddSectionAndCount(listActivePlugins, _T("Active Plugins"), out);
	GetStringFromDatabase("BeautifyActiveHeaderEnd", _T("</font></b>"), buffer, SIZEOF(buffer));
	BeautifyReport(beautify, buffer, headerHighlightEnd, out);
	out.append( _T("\r\n"));

	GetStringFromDatabase("BeautifyPluginsBegin", _T("<font size=\"-2\" color=\"black\">"), normalPluginsStart, SIZEOF(normalPluginsStart));
	BeautifyReport(beautify, normalPluginsStart, _T(""), out);
	out.append(GetListAsString(listActivePlugins, flags, beautify));
	GetStringFromDatabase("BeautifyPluginsEnd", _T("</font>"), normalPluginsEnd, SIZEOF(normalPluginsEnd));
	BeautifyReport(beautify, normalPluginsEnd, _T(""), out);
	//Plugins: list of inactive (disabled) plugins.
	if ((!forumStyle) && ((db_get_b(NULL, ModuleName, "ShowInactive", TRUE)) || (bServiceMode))) {
		BeautifyReport(beautify, horizLine, _T("\r\n"), out);
		GetStringFromDatabase("BeautifyInactiveHeaderBegin", _T("<b><font size=\"-1\" color=\"DarkRed\">"), buffer, SIZEOF(buffer));
		BeautifyReport(beautify, buffer, headerHighlightStart, out);
		AddSectionAndCount(listInactivePlugins, _T("Unloadable Plugins"), out);
		GetStringFromDatabase("BeautifyInactiveHeaderEnd", _T("</font></b>"), buffer, SIZEOF(buffer));
		BeautifyReport(beautify, buffer, headerHighlightEnd, out);
		out.append( _T("\r\n"));
		BeautifyReport(beautify, normalPluginsStart, _T(""), out);
		out.append(GetListAsString(listInactivePlugins, flags, beautify));
		BeautifyReport(beautify, normalPluginsEnd, _T(""), out);
	}
	if (listUnloadablePlugins.size() > 0) {
		BeautifyReport(beautify, horizLine, _T("\r\n"), out);
		GetStringFromDatabase("BeautifyUnloadableHeaderBegin", _T("<b><font size=\"-1\"><font color=\"Red\">"), buffer, SIZEOF(buffer));
		BeautifyReport(beautify, buffer, headerHighlightStart, out);
		AddSectionAndCount(listUnloadablePlugins, _T("Unloadable Plugins"), out);
		GetStringFromDatabase("BeautifyUnloadableHeaderEnd", _T("</font></b>"), buffer, SIZEOF(buffer));
		BeautifyReport(beautify, buffer, headerHighlightEnd, out);
		out.append( _T("\r\n"));
		BeautifyReport(beautify, normalPluginsStart, _T(""), out);
		out.append(GetListAsString(listUnloadablePlugins, flags, beautify));
		BeautifyReport(beautify, normalPluginsEnd, _T(""), out);
	}
	if (bWeatherPlugin) {
		out.append(_T("\r\nWeather ini files:\r\n-------------------------------------------------------------------------------\r\n"));
	}

	AddInfoFooter(suppressHeader, forumStyle, beautify, out);
	return out;
}

//========== Print functions =====

void CVersionInfo::PrintInformationsToFile(const TCHAR *info)
{
	TCHAR buffer[MAX_PATH], outputFileName[MAX_PATH];
	if (hOutputLocation) {
		FoldersGetCustomPathT(hOutputLocation,  buffer, SIZEOF(buffer), _T("%miranda_path%"));
		_tcscat(buffer, _T("\\VersionInfo.txt"));
	}
	else GetStringFromDatabase("OutputFile", _T("VersionInfo.txt"), buffer, SIZEOF(buffer));

	PathToAbsoluteT(buffer, outputFileName);

	FILE *fp = _tfopen(outputFileName, _T("wb"));
	if ( fp != NULL ) {
		char* utf = mir_utf8encodeT( info );
		fputs( "\xEF\xBB\xBF", fp);
		fputs( utf, fp );
		fclose(fp);

		TCHAR mex[512];
		mir_sntprintf(mex, SIZEOF(mex), TranslateT("Information successfully written to file: \"%s\"."), outputFileName);
		Log(mex);
	}
	else {
		TCHAR mex[512];
		mir_sntprintf(mex, SIZEOF(mex), TranslateT("Error during the creation of file \"%s\". Disk may be full or write protected."), outputFileName);
		Log(mex);
	}
}

void CVersionInfo::PrintInformationsToFile()
{
	PrintInformationsToFile( GetInformationsAsString().c_str());
}

void CVersionInfo::PrintInformationsToMessageBox()
{
	MessageBox(NULL, GetInformationsAsString().c_str(), _T("VersionInfo"), MB_OK);
}

void CVersionInfo::PrintInformationsToOutputDebugString()
{
	OutputDebugString( GetInformationsAsString().c_str());
}


void CVersionInfo::PrintInformationsToDialogBox()
{
	HWND DialogBox = CreateDialogParam(hInst,
			MAKEINTRESOURCE(IDD_DIALOGINFO),
			NULL, DialogBoxProc, (LPARAM) this);

	SetDlgItemText(DialogBox, IDC_TEXT, GetInformationsAsString().c_str());
}

void CVersionInfo::PrintInformationsToClipboard(bool showLog)
{
	if (GetOpenClipboardWindow()) {
		Log( TranslateT("The clipboard is not available, retry."));
		return;
	}

	OpenClipboard(NULL);
	//Ok, let's begin, then.
	EmptyClipboard();
	//Storage data we'll use.
	LPTSTR lptstrCopy;
	std::tstring aux = GetInformationsAsString();
	size_t length = aux.length() + 1;
	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, length*sizeof(TCHAR) + 5);
	//Lock memory, copy it, release it.
	lptstrCopy = (LPTSTR)GlobalLock(hData);
	lstrcpy(lptstrCopy, aux.c_str());
	lptstrCopy[length] = '\0';
	GlobalUnlock(hData);
	//Now set the clipboard data.

		SetClipboardData(CF_UNICODETEXT, hData);

	//Remove the lock on the clipboard.
	CloseClipboard();
	if (showLog)
		Log( TranslateT("Information successfully copied into clipboard."));
}
