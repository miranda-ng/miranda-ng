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


#include "CVersionInfo.h"
//#include "AggressiveOptimize.h"

#include "common.h"

//using namespace std;

BOOL (WINAPI *MyGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
BOOL (WINAPI *MyIsWow64Process)(HANDLE, PBOOL);
void (WINAPI *MyGetSystemInfo)(LPSYSTEM_INFO);
BOOL (WINAPI *MyGlobalMemoryStatusEx)(LPMEMORYSTATUSEX lpBuffer) = NULL;

LANGID (WINAPI *MyGetUserDefaultUILanguage)() = NULL;
LANGID (WINAPI *MyGetSystemDefaultUILanguage)() = NULL;

static int ValidExtension(char *fileName, char *extension)
{
	char *dot = strrchr(fileName, '.');
	if ((dot != NULL) && (lstrcmpiA(dot + 1, extension) == 0))
	{
		if (dot[strlen(extension) + 1] == 0)
		{
			return 1;
		}
	}

	return 0;
}

/*
int EnumSettings(const char *setting, LPARAM lParam)
{
	char *name = (char *) lParam;
	char *tmp = _strdup(setting);
	_strlwr(tmp);
	
	int res = 0;
	
	if (strcmp(tmp, name) == 0)
	{
		strcpy((char *) lParam, setting);
		res = 1;
	}
	
	free(tmp);
	
	return res;
}

int IsPluginDisabled(char *fileName)
{
	char *name = _strdup(fileName);
	_strlwr(name);
	DBCONTACTENUMSETTINGS dbEnum = {0};
	dbEnum.pfnEnumProc = EnumSettings;
	dbEnum.lParam = (LPARAM) name;
	dbEnum.szModule = "PluginDisable";
	
	int res = -1;
	
	CallService(MS_DB_CONTACT_ENUMSETTINGS, NULL, (LPARAM) &dbEnum);
	DBVARIANT dbv = {0};
	dbv.type = DBVT_BYTE;
	
	int exists = !(DBGetContactSetting(NULL, "PluginDisable", name, &dbv));
	if (exists)
	{
		res = dbv.bVal;
	}
	free(name);
 
	return res;
}
*/

void FillLocalTime(std::string &output, FILETIME *fileTime)
{
	TIME_ZONE_INFORMATION tzInfo = {0};
	FILETIME local = {0};
	SYSTEMTIME sysTime;
  char date[1024];
  char time[256];
	
  FileTimeToLocalFileTime(fileTime, &local);
  FileTimeToSystemTime(&local, &sysTime);

	GetDateFormat(EnglishLocale, 0, &sysTime, "dd' 'MMM' 'yyyy", date, sizeof(date));
	GetTimeFormat(NULL, TIME_FORCE24HOURFORMAT, &sysTime, "HH':'mm':'ss", time, sizeof(time)); //americans love 24hour format ;)
	output = std::string(date) + " at " + std::string(time);
	
	int res = GetTimeZoneInformation(&tzInfo);
	char tzName[32] = {0};
	char tzOffset[64] = {0};
	int offset = 0;
	switch (res)
	{
		case TIME_ZONE_ID_DAYLIGHT:
		{
			offset = -(tzInfo.Bias + tzInfo.DaylightBias);
			WideCharToMultiByte(CP_ACP, 0, tzInfo.DaylightName, -1, tzName, sizeof(tzName), NULL, NULL);
		
			break;
		}
		
		case TIME_ZONE_ID_STANDARD:
		{
			WideCharToMultiByte(CP_ACP, 0, tzInfo.StandardName, -1, tzName, sizeof(tzName), NULL, NULL);
			offset = -(tzInfo.Bias + tzInfo.StandardBias);
		
			break;
		}
		
		case TIME_ZONE_ID_UNKNOWN:
		{
			WideCharToMultiByte(CP_ACP, 0, tzInfo.StandardName, -1, tzName, sizeof(tzName), NULL, NULL);
			offset = -tzInfo.Bias;
		
			break;
		}
	}
	
	sprintf(tzOffset, "UTC %+02d:%02d", offset / 60, offset % 60);
	output += " (" /*+ std::string(tzName) + ", " */ + std::string(tzOffset) + ")";
}

CVersionInfo::CVersionInfo() {
	luiFreeDiskSpace = 0;
	bDEPEnabled = 0;
};

CVersionInfo::~CVersionInfo() {

	listInactivePlugins.clear();
	listActivePlugins.clear();
	listUnloadablePlugins.clear();
	
	lpzMirandaVersion.~basic_string();
	lpzNightly.~basic_string();
	lpzUnicodeBuild.~basic_string();
	lpzBuildTime.~basic_string();
	lpzOSVersion.~basic_string();
	lpzMirandaPath.~basic_string();
	lpzCPUName.~basic_string();
	lpzCPUIdentifier.~basic_string();
};

void CVersionInfo::Initialize() {
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

bool CVersionInfo::GetMirandaVersion() {
	const BYTE str_size = 64;
	char str[str_size];
	//Miranda version

	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)str_size, (LPARAM)(char*)str);
	this->lpzMirandaVersion = std::string(str);
	//Is it a nightly?
	if (lpzMirandaVersion.find("alpha",0) != std::string::npos)
		lpzNightly = "Yes";
	else
		lpzNightly = "No";
	if (lpzMirandaVersion.find("Unicode", 0) != std::string::npos)
		{
			lpzUnicodeBuild = "Yes";
		}
		else{
			lpzUnicodeBuild = "No";
		}
	char time[128], date[128];
	GetModuleTimeStamp(date, time);
	lpzBuildTime = std::string(time) + " on " + std::string(date);
	return TRUE;
}

bool CVersionInfo::GetOSVersion() {
	//Operating system informations
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	//Now fill the private members.
	char aux[256];
	wsprintf(aux, "%d.%d.%d %s", osvi.dwMajorVersion, osvi.dwMinorVersion, LOWORD(osvi.dwBuildNumber), osvi.szCSDVersion);
	lpzOSVersion = std::string(aux);
	//OSName
	//Let's read the registry.
	HKEY hKey;
	char szKey[MAX_PATH], szValue[MAX_PATH];


	lstrcpyn(szKey, "Hardware\\Description\\System\\CentralProcessor\\0", MAX_PATH);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			DWORD type, size, result;

			lstrcpyn(szValue, "Identifier", MAX_PATH);
			result = RegQueryValueEx(hKey, szValue, 0, &type, NULL, &size);
			if (result == ERROR_SUCCESS)
				{
					char *aux = (char *) malloc(size);
					result = RegQueryValueEx(hKey, szValue, 0, &type, (LPBYTE) aux, &size);
					lpzCPUIdentifier = std::string(aux);
					free(aux);
				}
				else{
					NotifyError(GetLastError(), "RegQueryValueEx()", __LINE__);
					lpzCPUIdentifier = "<Error in RegQueryValueEx()>";
				}
			lstrcpyn(szValue, "ProcessorNameString", MAX_PATH);
			result = RegQueryValueEx(hKey, szValue, 0, &type, NULL, &size); //get the size
			if (result == ERROR_SUCCESS)
				{
//					MessageBox(0, "ProcessorNameString available in the registry", "Info", 0);
					char *aux = (char *) malloc(size);
					result = RegQueryValueEx(hKey, szValue, 0, &type, (LPBYTE) aux, &size);
					lpzCPUName = std::string(aux);
					free(aux);
				}
				else{ //else try to use cpuid instruction to get the proc name
					char szName[49] = "<cpuid extension N/A>";
					#if (!defined(WIN64) && !defined(_WIN64))
						__asm
							{
								push eax //save the registers
								push ebx
								push ecx
								push edx

								xor eax, eax //get simple name
								cpuid
								mov DWORD PTR szName[0], ebx
								mov DWORD PTR szName[4], edx
								mov DWORD PTR szName[8], ecx
								mov DWORD PTR szName[12], 0
								
								mov eax, 0x80000000 //try to get pretty name
								cpuid
								
								cmp eax, 0x80000004
								jb end //if we don't have the extension end the check
								
								mov DWORD PTR szName[0], 0 //make the string null

								mov eax, 0x80000002 //first part of the string
								cpuid
								mov DWORD PTR szName[0], eax
								mov DWORD PTR szName[4], ebx
								mov DWORD PTR szName[8], ecx
								mov DWORD PTR szName[12], edx

								mov eax, 0x80000003 //second part of the string
								cpuid
								mov DWORD PTR szName[16], eax
								mov DWORD PTR szName[20], ebx
								mov DWORD PTR szName[24], ecx
								mov DWORD PTR szName[28], edx

								mov eax, 0x80000004 //third part of the string
								cpuid
								mov DWORD PTR szName[32], eax
								mov DWORD PTR szName[36], ebx
								mov DWORD PTR szName[40], ecx
								mov DWORD PTR szName[44], edx
								
							end:
								pop edx //load them back
								pop ecx
								pop ebx
								pop eax
							}
						szName[sizeof(szName) - 1] = '\0';
					#endif
					if (strlen(szName) == 0)
						{
							lpzCPUName = "<name N/A>";
						}
						else{
							lpzCPUName = std::string(szName);
						}
				}
		}

	bDEPEnabled = IsProcessorFeaturePresent(PF_NX_ENABLED);
	/*
	if (!bDEPEnabled)
		{
			int x = 0;
			__asm
				{
					push eax
					push ebx
					push ecx
					push edx
					
					mov eax, 0x80000001
					cpuid
					mov eax, 1
					shl eax, 20
					xor edx, eax
					cmp edx, 0
					je end_DEP
					mov x, 1
					
				end_DEP:
					pop edx
					pop ecx
					pop ebx
					pop eax
				}
			bDEPEnabled = x;
		}
	*/
	switch (osvi.dwPlatformId) {
		case VER_PLATFORM_WIN32_WINDOWS:
			lstrcpyn(szKey, "Software\\Microsoft\\Windows\\CurrentVersion", MAX_PATH);
			lstrcpyn(szValue, "Version", MAX_PATH);
			break;
		case VER_PLATFORM_WIN32_NT:

			lstrcpyn(szKey, "Software\\Microsoft\\Windows NT\\CurrentVersion", MAX_PATH);
			lstrcpyn(szValue, "ProductName", MAX_PATH);
			break;
	}

	RegCloseKey(hKey);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey,0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS) {
		DWORD type, size, result;
		//Get the size of the value we'll read.
		result = RegQueryValueEx((HKEY)hKey,szValue,(LPDWORD)NULL, (LPDWORD)&type,(LPBYTE)NULL,
				(LPDWORD)&size);
		if (result == ERROR_SUCCESS) {
			//Read it.
			char *aux = (char*)malloc(size);
			result = RegQueryValueEx((HKEY)hKey,szValue,(LPDWORD)NULL, (LPDWORD)&type,(LPBYTE)aux,(LPDWORD)&size);
			lpzOSName = std::string(aux);
			free(aux);
		}
		else {
			NotifyError(GetLastError(), "RegQueryValueEx()", __LINE__);
			lpzOSName = "<Error in RegQueryValueEx()>";
		}
	}
	else {
		NotifyError(GetLastError(), "RegOpenKeyEx()", __LINE__);
		lpzOSName = "<Error in RegOpenKeyEx()>";
	}
	
	//Now we can improve it if we can.
	switch (LOWORD(osvi.dwBuildNumber)) {
		case 950: lpzOSName = "Microsoft Windows 95"; break;
		case 1111: lpzOSName = "Microsoft Windows 95 OSR2"; break;
		case 1998: lpzOSName = "Microsoft Windows 98"; break;
		case 2222: lpzOSName = "Microsoft Windows 98 SE"; break;
		case 3000: lpzOSName = "Microsoft Windows ME"; break; //Even if this is wrong, we have already read it in the registry.
		case 1381: lpzOSName = "Microsoft Windows NT"; break; //What about service packs?
		case 2195: lpzOSName = "Microsoft Windows 2000"; break; //What about service packs?
		case 2600: lpzOSName = "Microsoft Windows XP"; break;
		case 3790:
		{
			if (GetSystemMetrics(89)) { //R2 ?
				lpzOSName = "Microsoft Windows 2003 R2";
			}
			else{
				lpzOSName = "Microsoft Windows 2003";
			}
				
			break; //added windows 2003 info
		}
	}

	return TRUE;
}

bool CVersionInfo::GetHWSettings() {
	//Free space on Miranda Partition.
	char szMirandaPath[MAX_PATH] = { 0 };
	{ char *str2;
		GetModuleFileName(GetModuleHandle(NULL),szMirandaPath,sizeof(szMirandaPath));
		str2=strrchr(szMirandaPath,'\\');
		if(str2!=NULL) *str2=0;
	}
	HMODULE hKernel32;
	hKernel32 = LoadLibrary("kernel32.dll");
	if (hKernel32) {
		MyGetDiskFreeSpaceEx = (BOOL (WINAPI *)(LPCTSTR,PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER))GetProcAddress(hKernel32, "GetDiskFreeSpaceExA");
		MyIsWow64Process = (BOOL (WINAPI *) (HANDLE, PBOOL)) GetProcAddress(hKernel32, "IsWow64Process");
		MyGetSystemInfo = (void (WINAPI *) (LPSYSTEM_INFO)) GetProcAddress(hKernel32, "GetNativeSystemInfo");
		MyGlobalMemoryStatusEx = (BOOL (WINAPI *) (LPMEMORYSTATUSEX)) GetProcAddress(hKernel32, "GlobalMemoryStatusEx");
		if (!MyGetSystemInfo)
		{
			MyGetSystemInfo = GetSystemInfo;
		}
		
		FreeLibrary(hKernel32);
	}
	if (MyGetDiskFreeSpaceEx) {
		ULARGE_INTEGER FreeBytes, a, b;
		MyGetDiskFreeSpaceEx(szMirandaPath, &FreeBytes, &a, &b);
		//Now we need to convert it.
		__int64 aux = FreeBytes.QuadPart;
		aux /= (1024*1024);
		luiFreeDiskSpace = (unsigned long int)aux;
	}
	else {
		luiFreeDiskSpace = 0;
	}
	
	char szInfo[1024];
	GetWindowsShell(szInfo, sizeof(szInfo));
	lpzShell = szInfo;
	GetInternetExplorerVersion(szInfo, sizeof(szInfo));
	lpzIEVersion = szInfo;
	
	
	lpzAdministratorPrivileges = (IsCurrentUserLocalAdministrator()) ? "Yes" : "No";
	
	bIsWOW64 = 0;
	if (MyIsWow64Process)
	{
		if (!MyIsWow64Process(GetCurrentProcess(), &bIsWOW64))
		{
			bIsWOW64 = 0;
		}
	}
	
	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	luiProcessors = sysInfo.dwNumberOfProcessors;
	
	//Installed RAM
	if (MyGlobalMemoryStatusEx) //windows 2000+
	{
		MEMORYSTATUSEX ms = {0};
		ms.dwLength = sizeof(ms);
		MyGlobalMemoryStatusEx(&ms);
		luiRAM = (unsigned int) ((ms.ullTotalPhys / (1024 * 1024)) + 1);
	}
	else{
		MEMORYSTATUS ms = {0};
		ZeroMemory(&ms, sizeof(ms));
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);
		luiRAM = (ms.dwTotalPhys/(1024*1024))+1; //Ugly hack!!!!
	}

	return TRUE;
}

bool CVersionInfo::GetProfileSettings()
{
	char profileName[MAX_PATH] = {0};
	char profilePath[MAX_PATH] = {0};
	ServiceExists(MS_DB_GETPROFILEPATH_BASIC) ? CallService(MS_DB_GETPROFILEPATH_BASIC, MAX_PATH, (LPARAM) profilePath) : CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM) profilePath); //get the profile path
	CallService(MS_DB_GETPROFILENAME, MAX_PATH, (LPARAM) profileName); //get the profile name
//	strcat(profileName, ".dat"); //add the .dat extension to the profile name
	lpzProfilePath = std::string(profilePath); //save the profile path
	
	strcat(profilePath, "\\"); //add the last \\ to the path
	strcat(profilePath, profileName); //now profilePath is drive:\path\profilename.dat
	HANDLE fin = CreateFile(profilePath, FILE_READ_ATTRIBUTES | FILE_READ_EA | STANDARD_RIGHTS_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fin == INVALID_HANDLE_VALUE) { fin = CreateFile(profilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); }
	if (fin != INVALID_HANDLE_VALUE)
		{
			DWORD size;
			FILETIME creation, modified, accessed;
			size = GetFileSize(fin, NULL);
		  TCHAR number[1024];
		  char tmp[1024];
		  double fileSize = (double) size / 1024;
		  
		  sprintf(tmp, "%f", fileSize); 
		  GetNumberFormat(EnglishLocale, NULL, tmp, NULL, number, 1024);
		  
		  lpzProfileSize = std::string(number) + " KBytes";
		  GetFileTime(fin, &creation, &accessed, &modified);
		  FillLocalTime(lpzProfileCreationDate, &creation);
		  
			CloseHandle(fin);
		}
		else{
			DWORD error = GetLastError();
			char tmp[1024];
			sprintf(tmp, "%d", error);
			lpzProfileCreationDate = "<error " + std::string(tmp) + " at FileOpen>" + std::string(profilePath);
			lpzProfileSize = "<error " + std::string(tmp) + " at FileOpen>" + std::string(profilePath);
		}
		
	return true;
}

static char szSystemLocales[4096] = {0};
static WORD systemLangID;
#define US_LANG_ID 0x00000409

BOOL CALLBACK EnumSystemLocalesProc(char *szLocale)
{
	DWORD locale = atoi(szLocale);
	char *name = GetLanguageName(locale);
	if (!strstr(szSystemLocales, name))
	{
		strcat(szSystemLocales, name);
		strcat(szSystemLocales, ", ");
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
	lpzOSLanguages = "(UI | Locale (User/System)) : ";
	
	LANGID UILang;
	
	OSVERSIONINFO os = {0};
	os.dwOSVersionInfoSize = sizeof(os);
	GetVersionEx(&os);
	switch (os.dwMajorVersion)
	{
		case 4: //Win 95-Me, NT
		{
			if (os.dwPlatformId == VER_PLATFORM_WIN32_NT) //Win NT
			{
				HMODULE hLib = LoadLibrary("ntdll.dll");

				if (hLib)
				{
					EnumResourceLanguages(hLib, RT_VERSION, MAKEINTRESOURCE(1), EnumResLangProc, NULL);

					FreeLibrary(hLib);

					if (systemLangID == US_LANG_ID)
					{
						UINT uiACP;

						uiACP = GetACP();
						switch (uiACP)
						{
							case 874:     // Thai code page activated, it's a Thai enabled system
								systemLangID = MAKELANGID(LANG_THAI, SUBLANG_DEFAULT);
								break;

							case 1255:    // Hebrew code page activated, it's a Hebrew enabled system
								systemLangID = MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT);
								break;

							case 1256:    // Arabic code page activated, it's a Arabic enabled system
								systemLangID = MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA);
								break;
								
							default:
								break;
						}
					}
				}
			}
			else{ //Win 95-Me
				HKEY hKey = NULL;
				char szLangID[128] = "0x";
				DWORD size = sizeof(szLangID) - 2;
				char err[512];
				
				if (RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\Desktop\\ResourceLocale", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
				{
					if (RegQueryValueEx(hKey, "", 0, NULL, (LPBYTE) &szLangID + 2, &size) == ERROR_SUCCESS)
					{
						sscanf(szLangID, "%lx", &systemLangID);
					}
					else{
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), LANG_SYSTEM_DEFAULT, err, size, NULL);
						MessageBox(0, err, "Error at RegQueryValueEx()", MB_OK);
					}
					RegCloseKey(hKey);
				}
				else{
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), LANG_SYSTEM_DEFAULT, err, size, NULL);
					MessageBox(0, err, "Error at RegOpenKeyEx()", MB_OK);
				}
			}

			lpzOSLanguages += GetLanguageName(systemLangID);

			break;
		}
		
		case 5: //Win 2000, XP
		default:
		{
			HMODULE hKernel32 = LoadLibrary("kernel32.dll");
			if (hKernel32)
			{
				MyGetUserDefaultUILanguage = (LANGID (WINAPI *)()) GetProcAddress(hKernel32, "GetUserDefaultUILanguage");
				MyGetSystemDefaultUILanguage = (LANGID (WINAPI *)()) GetProcAddress(hKernel32, "GetSystemDefaultUILanguage");
				
				FreeLibrary(hKernel32);
			}
		
			if ((MyGetUserDefaultUILanguage) && (MyGetSystemDefaultUILanguage))
			{
				UILang = MyGetUserDefaultUILanguage();
				lpzOSLanguages += GetLanguageName(UILang);
				lpzOSLanguages += "/";
				UILang = MyGetSystemDefaultUILanguage();
				lpzOSLanguages += GetLanguageName(UILang);
			}
			else{
				lpzOSLanguages += "Missing functions in kernel32.dll (GetUserDefaultUILanguage, GetSystemDefaultUILanguage)";
			}
			
			break;
		}
	}
	
	lpzOSLanguages += " | ";
	lpzOSLanguages += GetLanguageName(LOCALE_USER_DEFAULT);
	lpzOSLanguages += "/";
	lpzOSLanguages += GetLanguageName(LOCALE_SYSTEM_DEFAULT);

	if (DBGetContactSettingByte(NULL, ModuleName, "ShowInstalledLanguages", 0))
	{
		szSystemLocales[0] = '\0';
		lpzOSLanguages += " [";
		EnumSystemLocales(EnumSystemLocalesProc, LCID_INSTALLED);
		if (strlen(szSystemLocales) > 2)
		{
			szSystemLocales[strlen(szSystemLocales) - 2] = '\0';
		}
		lpzOSLanguages += szSystemLocales;
		lpzOSLanguages += "]";
	}
	
	return true;
}

int SaveInfo(const char *data, const char *lwrData, const char *search, char *dest, int size)
{
	const char *pos = strstr(lwrData, search);
	int res = 1;
	if (pos == lwrData)
	{
		strncpy(dest, &data[strlen(search)], size);
		res = 0;
	}
	
	return res;
}

bool CVersionInfo::GetLangpackInfo()
{
	char langpackPath[MAX_PATH] = {0};
	char search[MAX_PATH] = {0};
	char *p;
	lpzLangpackModifiedDate = "";	
	GetModuleFileName(GetModuleHandle(NULL), langpackPath, sizeof(langpackPath));
	p = strrchr(langpackPath, '\\');
	if (p)
	{
		WIN32_FIND_DATA data = {0};
		HANDLE hLangpack;
		
		p[1] = '\0';
		strcpy(search, langpackPath);
		strcat(search, "langpack_*.txt");
		hLangpack = FindFirstFile(search, &data);
		if (hLangpack != INVALID_HANDLE_VALUE)
		{
			char buffer[1024];
			char temp[1024];
			//FILETIME localWriteTime;
			//SYSTEMTIME sysTime;
			FillLocalTime(lpzLangpackModifiedDate, &data.ftLastWriteTime);
					
			char locale[128] = {0};
			char language[128] = {0};
			char version[128] = {0};
			strcpy(version, "N/A");
			char *p;
			strncpy(temp, data.cFileName, sizeof(temp));
			p = strrchr(temp, '.');
			p[0] = '\0';
			strncpy(language, strchr(temp, '_') + 1, sizeof(language));
			
			strcat(langpackPath, data.cFileName);
			FILE *fin = fopen(langpackPath, "rt");
			
			if (fin)
			{
				size_t len;
				while (!feof(fin))
				{
					fgets(buffer, sizeof(buffer), fin);
					len = strlen(buffer);
					if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
					strncpy(temp, buffer, sizeof(temp));
					_strlwr(temp);
					if (SaveInfo(buffer, temp, "language: ", language, sizeof(language)))
					{
						if (SaveInfo(buffer, temp, "locale: ", locale, sizeof(locale)))
						{
							p = strstr(buffer, "; FLID: ");
							if (p)
							{
								int ok = 1;
								int i;
								for (i = 0; ((i < 3) && (ok)); i++)
								{
									p = strrchr(temp, '.');
									if (p)
									{
										p[0] = '\0';
									}
									else{
										ok = 0;
									}
								}
								p = strrchr(temp, ' ');
								if ((ok) && (p))
								{
									strncpy(version, &buffer[p - temp + 1], sizeof(version));
								}
								else{
									strncpy(version, "<unknown>", sizeof(version));
								}
							}
						}
					}
				}
				
				lpzLangpackInfo = std::string(language) + " [" + std::string(locale) + "]";
				if (strlen(version) > 0)
				{
					lpzLangpackInfo += " v. " + std::string(version);
				}
				fclose(fin);
			}
			else{
				int err = GetLastError();
				lpzLangpackInfo = "<error> Could not open file " + std::string(data.cFileName);
			}
			FindClose(hLangpack);
		}
		else{
			lpzLangpackInfo = "No language pack installed";
		}
	}	
	
	return true;
}

std::string GetPluginTimestamp(FILETIME *fileTime)
{
	SYSTEMTIME sysTime;
	FileTimeToSystemTime(fileTime, &sysTime); //convert the file tyme to system time
	//char time[256];
	char date[256]; //lovely
	GetDateFormat(EnglishLocale, 0, &sysTime, "dd' 'MMM' 'yyyy", date, sizeof(date));
	//GetTimeFormat(NULL, TIME_FORCE24HOURFORMAT, &sysTime, "HH':'mm':'ss", time, sizeof(time)); //americans love 24hour format :)
	std::string timedate(date);
	return timedate;
}

bool CVersionInfo::GetPluginLists() {

	HANDLE hFind;
	char szMirandaPath[MAX_PATH] = { 0 }, szSearchPath[MAX_PATH] = { 0 }; //For search purpose
	WIN32_FIND_DATA fd;
	char szMirandaPluginsPath[MAX_PATH] = { 0 }, szPluginPath[MAX_PATH] =  { 0 }; //For info reading purpose
	BYTE PluginIsEnabled = 0;
	HINSTANCE hInstPlugin = NULL;
	PLUGININFOEX *(*MirandaPluginInfo)(DWORD); //These two are used to get informations from the plugin.
	PLUGININFOEX *pluginInfo = NULL; //Read above.
	DWORD mirandaVersion = 0;
	BOOL asmCheckOK = FALSE;
	DWORD loadError;
//	SYSTEMTIME sysTime; //for timestamp

	mirandaVersion=(DWORD)CallService(MS_SYSTEM_GETVERSION,0,0);

	{	char *str2;
		GetModuleFileName(GetModuleHandle(NULL),szMirandaPath,sizeof(szMirandaPath));
		str2=strrchr(szMirandaPath,'\\');
		if(str2!=NULL) *str2=0;
	}
	lpzMirandaPath = std::string(szMirandaPath);
	//We got Miranda path, now we'll use it for two different purposes.
	//1) finding plugins.
	//2) Reading plugin infos
	lstrcpyn(szSearchPath,szMirandaPath, MAX_PATH); //We got the path, now we copy it into szSearchPath. We'll use szSearchPath as am auxiliary variable, while szMirandaPath will keep a "fixed" value.
	lstrcat(szSearchPath,"\\Plugins\\*.dll");
	
	lstrcpyn(szMirandaPluginsPath, szMirandaPath, MAX_PATH);
	lstrcat(szMirandaPluginsPath,"\\Plugins\\");
	
	hFind=FindFirstFile(szSearchPath,&fd);
	LogToFile("Starting to load plugins");
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			if (verbose) PUShowMessage(fd.cFileName, SM_NOTIFY);
			if (!ValidExtension(fd.cFileName, "dll"))
			{
				continue; //do not report plugins that do not have extension .dll
			}
			
			hInstPlugin = GetModuleHandle(fd.cFileName); //try to get the handle of the module
												
			if (hInstPlugin) //if we got it then the dll is loaded (enabled)
				{
					PluginIsEnabled = 1;
					LogToFile("Plugin '%s' is enabled and loaded", fd.cFileName);
				}
				else{
					PluginIsEnabled = 0;
					LogToFile("Plugin '%s' is not loaded, going to load it", fd.cFileName);
					lstrcpyn(szPluginPath, szMirandaPluginsPath, MAX_PATH); // szPluginPath becomes "drive:\path\Miranda\Plugins\"
					lstrcat(szPluginPath, fd.cFileName); // szPluginPath becomes "drive:\path\Miranda\Plugins\popup.dll"
					hInstPlugin = LoadLibrary(szPluginPath);
/*				
					PluginIsEnabled = !(
							DBGetContactSettingByte(NULL, "PluginDisable", fd.cFileName, FALSE)
							|| DBGetContactSettingByte(NULL, "PluginDisable", CharUpper(fd.cFileName), FALSE)
							|| DBGetContactSettingByte(NULL, "PluginDisable", CharLower(fd.cFileName), FALSE)

					}; //If the DB does not contain that value, the plugin is enabled => default: FALSE; action: !read
*/											
					//Let's read the informations.
/*				
				}
			if (PluginIsEnabled) {
				hInstPlugin = GetModuleHandle(fd.cFileName);
			}
			else{ 
				lstrcpyn(szPluginPath, szMirandaPluginsPath, MAX_PATH); // szPluginPath becomes "drive:\path\Miranda\Plugins\"
				lstrcat(szPluginPath, fd.cFileName); // szPluginPath becomes "drive:\path\Miranda\Plugins\popup.dll"
				hInstPlugin = LoadLibrary(szPluginPath);
			}
*/			
				}
			if (!hInstPlugin) { //It wasn't loaded.
				loadError = GetLastError();
				int bUnknownError = 1; //assume plugin didn't load because of unknown error
				//Some error messages.
				//get the dlls the plugin statically links to 
				if (DBGetContactSettingByte(NULL, ModuleName, "CheckForDependencies", TRUE))
					{
						std::string linkedModules = "";
						
						lstrcpyn(szPluginPath, szMirandaPluginsPath, MAX_PATH); // szPluginPath becomes "drive:\path\Miranda\Plugins\"
						lstrcat(szPluginPath, fd.cFileName); // szPluginPath becomes "drive:\path\Miranda\Plugins\popup.dll"
						if (GetLinkedModulesInfo(szPluginPath, linkedModules))
							{
								LogToFile("Plugin %s has unresolved dependencies, adding to unloadable list", szPluginPath);
								std::string time = GetPluginTimestamp(&fd.ftLastWriteTime);
								CPlugin thePlugin = CPlugin(std::string(fd.cFileName), "<unknown>", UUID_NULL, "", 0, time, linkedModules);
								AddPlugin(thePlugin, listUnloadablePlugins);
								bUnknownError = 0; //we know why the plugin didn't load
							}
					}
				if (bUnknownError) //if cause is unknown then report it
					{
						LogToFile("Plugin %s doesn't load", szPluginPath);
						std::string time = GetPluginTimestamp(&fd.ftLastWriteTime);
						char buffer[4096];
						char error[2048];
						//DWORD_PTR arguments[2] = {loadError, 0};
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, loadError, 0, error, sizeof(error), NULL);
						sprintf(buffer, "    Error %ld - %s", loadError, error);
						CPlugin thePlugin = CPlugin(std::string(fd.cFileName), "<unknown>", UUID_NULL, "", 0, time, buffer);
						AddPlugin(thePlugin, listUnloadablePlugins);
					}
			}
			else { //It was successfully loaded.
				LogToFile("Plugin '%s' was loaded successfully", fd.cFileName);
				MirandaPluginInfo = (PLUGININFOEX *(*)(DWORD))GetProcAddress(hInstPlugin, "MirandaPluginInfoEx");
				if (!MirandaPluginInfo)
				{
					MirandaPluginInfo = (PLUGININFOEX *(*)(DWORD))GetProcAddress(hInstPlugin, "MirandaPluginInfo"); 
				}
				if (!MirandaPluginInfo) { //There is no function: it's not a valid plugin. Let's move on to the next file.
					continue;
				}
				else {
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
	//						jmp a2
							a1:
	//						a2:
						}
					#else
						asmCheckOK = TRUE;
					#endif
					if (asmCheckOK)
						pluginInfo = CopyPluginInfo(MirandaPluginInfo(mirandaVersion));
					else {
						ZeroMemory(&pluginInfo, sizeof(pluginInfo));
						MessageBox(NULL, fd.cFileName, "Invalid plugin", MB_OK);
					}
				}
			}
			//Let's get the info.
			if (MirandaPluginInfo != NULL) {//a valid miranda plugin
				if (pluginInfo != NULL) {
					LogToFile("Plugin '%s' is a miranda plugin", fd.cFileName);
					//We have loaded the informations into pluginInfo.
					std::string timedate = GetPluginTimestamp(&fd.ftLastWriteTime);
					CPlugin thePlugin = CPlugin(std::string(fd.cFileName),std::string(pluginInfo->shortName), pluginInfo->uuid, std::string((pluginInfo->flags & 1) ? "Unicode aware" : ""), (DWORD) pluginInfo->version, timedate, "");

					if (PluginIsEnabled)
					{
						AddPlugin(thePlugin, listActivePlugins);
					}
					else {
						if ((IsUUIDNull(pluginInfo->uuid)) && (mirandaVersion >= PLUGIN_MAKE_VERSION(0,8,0,9)))
						{
							thePlugin.SetErrorMessage("    Plugin does not have an UUID and will not work with Miranda 0.8.\r\n");
							AddPlugin(thePlugin, listUnloadablePlugins);
						}
						else{
							AddPlugin(thePlugin, listInactivePlugins);
						}
						FreeLibrary(hInstPlugin); //We don't need it anymore.
					}
					FreePluginInfo(pluginInfo);
					MirandaPluginInfo = NULL;
#ifdef _DEBUG
					if (verbose)
					{
						char szMsg[4096] = { 0 };
						wsprintf(szMsg, "Done with: %s", fd.cFileName);
						PUShowMessage(szMsg, SM_NOTIFY);
					}
#endif
				}
				else{//pluginINFO == NULL
					LogToFile("Plugin '%s' refuses to load", fd.cFileName);
					pluginInfo = CopyPluginInfo(MirandaPluginInfo(PLUGIN_MAKE_VERSION(9, 9, 9, 9))); //let's see if the plugin likes this miranda version
					char *szShortName = "<unknown>";
					std::string time = GetPluginTimestamp(&fd.ftLastWriteTime); //get the plugin timestamp;
					DWORD version = 0;
					if (pluginInfo)
					{
						szShortName = pluginInfo->shortName;
						version = pluginInfo->version;
					}

					CPlugin thePlugin = CPlugin(std::string(fd.cFileName), std::string(szShortName), (pluginInfo) ? (pluginInfo->uuid) : UUID_NULL, std::string(((pluginInfo) && (pluginInfo->flags & 1)) ? "Unicode aware" : ""), version, time, "    Plugin refuses to load. Miranda version too old.");
					
					AddPlugin(thePlugin, listUnloadablePlugins);
					if (pluginInfo)
					{
						FreePluginInfo(pluginInfo);
					}
				}
			}
		} while (FindNextFile(hFind,&fd));
		FindClose(hFind);
	}
	LogToFile("Done loading plugins");
	return TRUE;
}

bool CVersionInfo::AddPlugin(CPlugin &aPlugin, std::list<CPlugin> &aList) {
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
			else { //It's greater: we need to insert it.
				pos++;
			}
		}
	}
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

bool CVersionInfo::GetLinkedModulesInfo(char *moduleName, std::string &linkedModules)
{
	LogToFile("Checking dll %s for unresolved dll dependencies ...", moduleName);
	LOADED_IMAGE image;
	ULONG importTableSize;
	IMAGE_IMPORT_DESCRIPTOR *importData;
	//HMODULE dllModule;
	linkedModules = "";
	bool result = false;
//	LogToFile("  Before LoadLibraryEx", moduleName);
//	dllModule = LoadLibraryEx(moduleName, NULL, DONT_RESOLVE_DLL_REFERENCES);
	//dllModule = 0;
	//LogToFile("  Before MapAndLoad (dll handle %ld)", moduleName, dllModule);
	if (MapAndLoad(moduleName, NULL, &image, TRUE, TRUE) == FALSE)
		{
			char tmp[20];
			sprintf(tmp, "%d", GetLastError());
			LogToFile("  MapAndLoad failed with error %s", tmp);
			linkedModules = "<error " + std::string(tmp) + " at MapAndLoad()>\r\n";
//			FreeLibrary(dllModule);
			return result;
		}
	LogToFile("  Before ImageDirectoryEntryToData (base address %ld)", image.MappedAddress);
	importData = (IMAGE_IMPORT_DESCRIPTOR *) ImageDirectoryEntryToData(image.MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_IMPORT, &importTableSize);
	if (!importData)
		{
			char tmp[20];
			sprintf(tmp, "%d", GetLastError());
			LogToFile("  ImageDirectoryEntryToData failed with error %s", tmp);
			linkedModules = "<error " + std::string(tmp) + " at ImageDirectoryEntryToDataEx()>\r\n";
		}
		else{
			LogToFile("  Checking dll dependencies");
			while (importData->Name)
				{
					char *moduleName;
					moduleName = GetStringFromRVA(importData->Name, &image);
					LogToFile("    Checking link to dll %s", moduleName);
					if (!DoesDllExist(moduleName))
						{
							LogToFile("      Dll %s not found, adding to list", moduleName);
							linkedModules.append("    Plugin statically links to missing dll file: " + std::string(moduleName) + "\r\n");
							result = true;
						}
						else{
							LogToFile("      Dll %s found", moduleName);
						}
					LogToFile("    Moving to next import entry");
					importData++; //go to next record
				}
		}
	LogToFile("  Done checking dependencies");
	LogToFile("  Cleaning up; before {FreeLibrary()} and UnMapAndLoad");
//	FreeLibrary(dllModule);
	UnMapAndLoad(&image); //unload the image
	return result;
}

std::string CVersionInfo::GetListAsString(std::list<CPlugin> &aList, DWORD flags, int beautify) {
	std::list<CPlugin>::iterator pos = aList.begin();
	std::string out = "";
#ifdef _DEBUG
	if (verbose) PUShowMessage("CVersionInfo::GetListAsString, begin.", SM_NOTIFY);
#endif

	char szHeader[32] = {0};
	char szFooter[32] = {0};
	if ((((flags & VISF_FORUMSTYLE) == VISF_FORUMSTYLE) || beautify) && (DBGetContactSettingByte(NULL, ModuleName, "BoldVersionNumber", TRUE)))
		{
			GetStringFromDatabase("BoldBegin", "[b]", szHeader, sizeof(szHeader));
			GetStringFromDatabase("BoldEnd", "[/b]", szFooter, sizeof(szFooter));
		}
	
	while (pos != aList.end()) {
		out.append(std::string((*pos).getInformations(flags, szHeader, szFooter)));
		pos++;
	}
#ifdef _DEBUG
	if (verbose) PUShowMessage("CVersionInfo::GetListAsString, end.", SM_NOTIFY);
#endif
	return out;
};

void CVersionInfo::BeautifyReport(int beautify, char *szBeautifyText, char *szNonBeautifyText, std::string &out)
{
	if (beautify)
		{
			out.append(szBeautifyText);
		}
		else{
			out.append(szNonBeautifyText);
		}
}

void CVersionInfo::AddInfoHeader(int suppressHeader, int forumStyle, int beautify, std::string &out)
{
	if (forumStyle) //forum style
		{
			char szSize[256];
			char szQuote[256];

			GetStringFromDatabase("SizeBegin", "[size=1]", szSize, sizeof(szSize));
			GetStringFromDatabase("QuoteBegin", "[quote]", szQuote, sizeof(szQuote));
			out.append(szQuote);
			out.append(szSize);
		}
		else{
			out = "";
		}
	if (!suppressHeader)
		{
			out.append("Miranda IM - VersionInformation plugin by Hrk, modified by Eblis\r\n");
			if (!forumStyle)
				{
					out.append("Miranda's homepage: http://www.miranda-im.org/\r\n"); //changed homepage
					out.append("Miranda tools: http://miranda-im.org/download/\r\n\r\n"); //was missing a / before download
				}
		}
	char buffer[1024]; //for beautification
	GetStringFromDatabase("BeautifyHorizLine", "<hr />", buffer, sizeof(buffer));	
	BeautifyReport(beautify, buffer, "", out);
	GetStringFromDatabase("BeautifyBlockStart", "<blockquote>", buffer, sizeof(buffer));
	BeautifyReport(beautify, buffer, "", out);
	if (!suppressHeader)
		{
			//Time of report:
			char lpzTime[12]; 	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL,"HH':'mm':'ss",lpzTime, sizeof(lpzTime));
			char lpzDate[32]; 	GetDateFormat(EnglishLocale, 0, NULL,"dd' 'MMMM' 'yyyy",lpzDate, sizeof(lpzDate));
			out.append("Report generated at: " + std::string(lpzTime) + " on " + std::string(lpzDate) + "\r\n\r\n");
		}
	//Operating system
	out.append("CPU: " + lpzCPUName + " [" + lpzCPUIdentifier + "]");
	if (bDEPEnabled)
		{
			out.append(" [DEP enabled]");
		}
	if (luiProcessors > 1)
		{
			char noProcs[128];
			sprintf(noProcs, " [%d CPUs]", luiProcessors);
			out.append(noProcs);
		}
	out.append("\r\n");
	
	//RAM
	char szRAM[64]; wsprintf(szRAM, "%d", luiRAM);
	out.append("Installed RAM: " + std::string(szRAM) + " MBytes\r\n");
	
	//operating system
	out.append("Operating System: " + lpzOSName + " [version: " + lpzOSVersion + "]\r\n");
	
	//shell, IE, administrator
	out.append("Shell: " + lpzShell + ", Internet Explorer " + lpzIEVersion + "\r\n");
	out.append("Administrator privileges: " + lpzAdministratorPrivileges + "\r\n");
	
	//languages
	out.append("OS Languages: " + lpzOSLanguages + "\r\n");
	
	
	//FreeDiskSpace
	if (luiFreeDiskSpace) {
		char szDiskSpace[64]; wsprintf(szDiskSpace, "%d", luiFreeDiskSpace);
		out.append("Free disk space on Miranda partition: " + std::string(szDiskSpace) + " MBytes\r\n");
	}
	
	//Miranda
	out.append("Miranda path: "	+ lpzMirandaPath + "\r\n");
	
	out.append("Miranda IM version: " + lpzMirandaVersion);
	if (bIsWOW64)
	{
		out.append(" [running inside WOW64]");
	}
	if (bServiceMode)
	{
		out.append(" [service mode]");
	}
	out.append("\r\nBuild time: " + lpzBuildTime + "\r\n");
	out.append("Profile path: " + lpzProfilePath + "\r\n");
	//if (lpzProfileSize.find("error", 0) == std::string::npos)	//only show the profile size of no error occured
	//{
		out.append("Profile size: " + lpzProfileSize + "\r\n");
		out.append("Profile creation date: " + lpzProfileCreationDate + "\r\n");
	//}
	out.append("Language pack: " + lpzLangpackInfo);
	out.append((lpzLangpackModifiedDate.size() > 0) ? ", modified: " + lpzLangpackModifiedDate : "");
	out.append("\r\n");
		
	out.append("Nightly: " + lpzNightly + "\r\n");
	out.append("Unicode core: " + lpzUnicodeBuild);
	
	GetStringFromDatabase("BeautifyBlockEnd", "</blockquote>", buffer, sizeof(buffer));
	BeautifyReport(beautify, buffer, "\r\n", out);
	//out.append("\r\n");
}

void CVersionInfo::AddInfoFooter(int suppressFooter, int forumStyle, int beautify, std::string &out)
{
	//End of report
	char buffer[1024]; //for beautification purposes
	GetStringFromDatabase("BeautifyHorizLine", "<hr />", buffer, sizeof(buffer));
	if (!suppressFooter)
		{
			BeautifyReport(beautify, buffer, "\r\n", out);
			out.append("\r\nEnd of report.\r\n");
		}
	if (!forumStyle)
		{
			if (!suppressFooter)
				{
					out.append(Translate("If you are going to use this report to submit a bug, remember to check the website for questions or help the developers may need.\r\nIf you don't check your bug report and give feedback, it will not be fixed!"));
				}
		}
		else{
			char szSize[256];
			char szQuote[256];

			GetStringFromDatabase("SizeEnd", "[/size]", szSize, sizeof(szSize));
			GetStringFromDatabase("QuoteEnd", "[/quote]", szQuote, sizeof(szQuote));
			out.append(szSize);
			out.append(szQuote);
		}
}

static void AddSectionAndCount(std::list<CPlugin> list, char *listText, std::string &out)
{
	char tmp[64];
	sprintf(tmp, " (%u)", list.size());
	out.append(listText);
	out.append(tmp);
	out.append(":");
}

std::string CVersionInfo::GetInformationsAsString(int bDisableForumStyle) {
	//Begin of report
	std::string out;
	int forumStyle = (bDisableForumStyle) ? 0 : DBGetContactSettingByte(NULL, ModuleName, "ForumStyle", TRUE);
	int showUUID = DBGetContactSettingByte(NULL, ModuleName, "ShowUUIDs", FALSE);
	int beautify = DBGetContactSettingByte(NULL, ModuleName, "Beautify", 0) & (!forumStyle);
	int suppressHeader = DBGetContactSettingByte(NULL, ModuleName, "SuppressHeader", TRUE);

	DWORD flags = (forumStyle) | (showUUID << 1);

	AddInfoHeader(suppressHeader, forumStyle, beautify, out);
	char normalPluginsStart[1024]; //for beautification purposes, for normal plugins text (start)
	char normalPluginsEnd[1024]; //for beautification purposes, for normal plugins text (end)
	char horizLine[1024]; //for beautification purposes
	char buffer[1024]; //for beautification purposes
	
	char headerHighlightStart[10] = "";
	char headerHighlightEnd[10] = "";
	if (forumStyle)
		{
			char start[128], end[128];
			GetStringFromDatabase("BoldBegin", "[b]", start, sizeof(start));
			GetStringFromDatabase("BoldEnd", "[/b]", end, sizeof(end));
			strncpy(headerHighlightStart, start, sizeof(headerHighlightStart));
			strncpy(headerHighlightEnd, end, sizeof(headerHighlightEnd));
		}
	
	//Plugins: list of active (enabled) plugins.
	GetStringFromDatabase("BeautifyHorizLine", "<hr />", horizLine, sizeof(horizLine));
	BeautifyReport(beautify, horizLine, "\r\n", out);
	GetStringFromDatabase("BeautifyActiveHeaderBegin", "<b><font size=\"-1\" color=\"DarkGreen\">", buffer, sizeof(buffer));
	BeautifyReport(beautify, buffer, headerHighlightStart, out);
	AddSectionAndCount(listActivePlugins, "Active Plugins", out);
	GetStringFromDatabase("BeautifyActiveHeaderEnd", "</font></b>", buffer, sizeof(buffer));
	BeautifyReport(beautify, buffer, headerHighlightEnd, out);
	out.append("\r\n");
	
	GetStringFromDatabase("BeautifyPluginsBegin", "<font size=\"-2\" color=\"black\">", normalPluginsStart, sizeof(normalPluginsStart));
	BeautifyReport(beautify, normalPluginsStart, "", out);
	out.append(GetListAsString(listActivePlugins, flags, beautify));
	GetStringFromDatabase("BeautifyPluginsEnd", "</font>", normalPluginsEnd, sizeof(normalPluginsEnd));
	BeautifyReport(beautify, normalPluginsEnd, "", out);
	//Plugins: list of inactive (disabled) plugins.
	if ((!forumStyle) && ((DBGetContactSettingByte(NULL, ModuleName, "ShowInactive", TRUE)) || (bServiceMode))) {
		BeautifyReport(beautify, horizLine, "\r\n", out);
		GetStringFromDatabase("BeautifyInactiveHeaderBegin", "<b><font size=\"-1\" color=\"DarkRed\">", buffer, sizeof(buffer));
		BeautifyReport(beautify, buffer, headerHighlightStart, out);
		AddSectionAndCount(listInactivePlugins, "Inactive Plugins", out);
		GetStringFromDatabase("BeautifyInactiveHeaderEnd", "</font></b>", buffer, sizeof(buffer));
		BeautifyReport(beautify, buffer, headerHighlightEnd, out);
		out.append("\r\n");
		BeautifyReport(beautify, normalPluginsStart, "", out);
		out.append(GetListAsString(listInactivePlugins, flags, beautify));
		BeautifyReport(beautify, normalPluginsEnd, "", out);
	}
	if (listUnloadablePlugins.size() > 0)
		{
			//out.append("\r\n");
			BeautifyReport(beautify, horizLine, "\r\n", out);
			GetStringFromDatabase("BeautifyUnloadableHeaderBegin", "<b><font size=\"-1\"><font color=\"Red\">", buffer, sizeof(buffer));
			BeautifyReport(beautify, buffer, headerHighlightStart, out);
			AddSectionAndCount(listUnloadablePlugins, "Unloadable Plugins", out);
			GetStringFromDatabase("BeautifyUnloadableHeaderEnd", "</font></b>", buffer, sizeof(buffer));
			BeautifyReport(beautify, buffer, headerHighlightEnd, out);
			out.append("\r\n");
			BeautifyReport(beautify, normalPluginsStart, "", out);
			out.append(GetListAsString(listUnloadablePlugins, flags, beautify));
			BeautifyReport(beautify, normalPluginsEnd, "", out);			
		}
	AddInfoFooter(suppressHeader, forumStyle, beautify, out);
	return out;
}

//========== Print functions =====

void CVersionInfo::PrintInformationsToFile(const char *info)
{
	char buffer[MAX_PATH];
	char outputFileName[MAX_PATH];
	if (bFoldersAvailable)
	{
		FoldersGetCustomPath(hOutputLocation,  buffer, sizeof(buffer), "%miranda_path%");
		strcat(buffer, "\\VersionInfo.txt");
	}
	else{
		GetStringFromDatabase("OutputFile", "VersionInfo.txt", buffer, sizeof(buffer));
	}

	RelativePathToAbsolute(buffer, outputFileName, sizeof(buffer));
	
	FILE *fp = fopen(outputFileName, "wb");
	if (fp != NULL) {
		fprintf(fp, info);
		fclose(fp);
		char mex[512];
		mir_snprintf(mex, sizeof(mex), Translate("Information successfully written to file: \"%s\"."), outputFileName);
		Log(mex);
	}
	else {
		char mex[512];
		mir_snprintf(mex, sizeof(mex), Translate("Error during the creation of file \"%s\". Disk may be full or write protected."), outputFileName);
		Log(mex);
	}
}

void CVersionInfo::PrintInformationsToFile() {
	PrintInformationsToFile((*this).GetInformationsAsString().c_str());
}

void CVersionInfo::PrintInformationsToMessageBox() {
	MessageBox(NULL, (*this).GetInformationsAsString().c_str(), ModuleName, MB_OK);
	return;
}

void CVersionInfo::PrintInformationsToOutputDebugString() {
	OutputDebugString((*this).GetInformationsAsString().c_str());
	return;
}

#include "resource.h"
//extern HINSTANCE hInst;

void CVersionInfo::PrintInformationsToDialogBox() {
//	HWND parent = DBGetContactSettingByte(NULL, ModuleName, "ShowInTaskbar", TRUE)?GetDesktopWindow():NULL;
	HWND parent = NULL;
	HWND DialogBox = CreateDialogParam(hInst,
			MAKEINTRESOURCE(IDD_DIALOGINFO),
			parent, DialogBoxProc, (LPARAM) this);
	
	SetDlgItemText(DialogBox, IDC_TEXT, (*this).GetInformationsAsString().c_str());
	return;
}

void CVersionInfo::PrintInformationsToClipboard(bool showLog) {

	if (GetOpenClipboardWindow()) {
		Log(Translate("The clipboard is not available, retry."));
	}
	else {
		OpenClipboard(NULL);
		//Ok, let's begin, then.
		EmptyClipboard();
		//Storage data we'll use.
		LPTSTR lptstrCopy;
		std::string aux = (*this).GetInformationsAsString();
		size_t length = aux.length() + 1;
		HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, length + 5);
		//Lock memory, copy it, release it.
		lptstrCopy = (LPTSTR)GlobalLock(hData);
		memcpy(lptstrCopy, aux.c_str(), length);
		lptstrCopy[length] = '\0';
		GlobalUnlock(hData);
		//Now set the clipboard data.
		SetClipboardData(CF_TEXT, hData);
		//Remove the lock on the clipboard.
		CloseClipboard();
		if (showLog)
			{
				Log(Translate("Information successfully copied into clipboard."));
			}
	}
	return;
}

/*int MyReceive(SOCKET sClient, char *message, int size, int bShow)
{
	int len = recv(sClient, message, size - 1, 0);
	int success = 0;
	if (len != SOCKET_ERROR)
		{
			message[len] = '\0';
			if (bShow)
				{
					PUShowMessage(message, SM_NOTIFY);
				}
			char *pos = message;
			while ((pos = strchr(pos + 1, '\n')))
				{
					success++;
				}
			if (success <= 0) success = 1; //make sure success is at least 1
		}
		else{
			success = 0;
			closesocket(sClient);
		} 
	return success;
}*/

#define UPLOAD_ERROR() {PUShowMessage("Error while trying to upload data", SM_WARNING); return 1;}

DWORD WINAPI UploadWorkerTread(LPVOID param)
{
	//PUShowMessage("Uploading to site", SM_NOTIFY);
	/*
	char *text = (char *) param;
	char message[2048];
	
	char server[1024];
	int port;
	char user[512];
	char password[512];
	
	GetStringFromDatabase("UploadServer", "vi.cass.cz", server, sizeof(server));
	port = DBGetContactSettingWord(NULL, ModuleName, "UploadPort", DEFAULT_UPLOAD_PORT);
	GetStringFromDatabase("UploadUser", "", user, sizeof(user));
	GetStringFromDatabase("UploadPassword", "", password, sizeof(password));
	CallService(MS_DB_CRYPT_DECODESTRING, sizeof(password), (LPARAM) password);
	
	SOCKET sClient = socket(AF_INET, SOCK_STREAM, 0);
	if (!sClient)
		{
			MB("Could not create connection socket ...");
			return 1;
		}
		
	sockaddr_in addr = {0};
	hostent *localHost = gethostbyname(server);
	char *localIP = (localHost) ? inet_ntoa(*(struct in_addr *) *localHost->h_addr_list) : server;
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(localIP);
	addr.sin_port = htons(port);
	
	int res = connect(sClient, (sockaddr *) &addr, sizeof(addr));
	if (res)
		{
			char buffer[1024];
			mir_snprintf(buffer, sizeof(buffer), "Could not connect to server '%s' on port %d", server, port);
			MB(buffer);
			return 1;
		}
	res = MyReceive(sClient, message, sizeof(message), TRUE); //get the welcome message
	switch (res)
		{
			case 1:
				if (!MyReceive(sClient, message, sizeof(message), 0)) UPLOAD_ERROR(); //get the enter username message
				break;
				
			case 2: //the enter user message was received already
				break;
				
			default: //assume error by default
				UPLOAD_ERROR();
		}
	
	send(sClient, user, strlen(user), 0);
	if (!MyReceive(sClient, message, sizeof(message), FALSE)) UPLOAD_ERROR(); //get the enter password message
	send(sClient, password, strlen(password), 0);
	if (!MyReceive(sClient, message, sizeof(message), FALSE)) UPLOAD_ERROR(); //get the upload data message
	send(sClient, text, strlen(text) + 1, 0); //data message needs to send \0 so the server knows when to stop.
	if (!MyReceive(sClient, message, sizeof(message), TRUE)) UPLOAD_ERROR();
	closesocket(sClient);
	//PUShowMessage("Done uploading to site", SM_NOTIFY);
	
	if (text)
		{
			free(text);
		}
	*/
	return 0;
}

void CVersionInfo::UploadToSite(char *text){
	
	DWORD threadID;
	HANDLE thread;
	char *data = NULL;
	if (!text)
		{
			data = _strdup(GetInformationsAsString().c_str());
		}
		else{
			data = _strdup(text);
		}	
	
	thread = CreateThread(NULL, NULL, UploadWorkerTread, data, 0, &threadID); //the thread will free the buffer
	if (!thread)
		{
			MB("Upload worker thread could not be created");
		}
	if ((thread != NULL) && (thread != INVALID_HANDLE_VALUE))
	{
		CloseHandle(thread);
	}
}