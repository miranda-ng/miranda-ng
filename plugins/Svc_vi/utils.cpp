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

//#define USE_LOG_FUNCTIONS

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include "utils.h"

/*
My usual MessageBoxes :-)
*/
void MB(char* message) {
	if (verbose) MessageBox(NULL, message, ModuleName, MB_OK | MB_ICONEXCLAMATION);
}

void Log(char* message) {
	if (ServiceExists(MS_POPUP_ADDPOPUP))
	{
		POPUPDATA pu = {0};
		pu.lchIcon = hiVIIcon;
		strncpy(pu.lptzContactName, Translate("Version Information"), MAX_CONTACTNAME);
		strncpy(pu.lptzText, message, MAX_SECONDLINE);
		PUAddPopUp(&pu);
	}
	else {
		MessageBox(NULL, message, ModuleName, MB_OK | MB_ICONINFORMATION);
	}
}

int SplitStringInfo(const char *szWholeText, char *szStartText, char *szEndText)
{
	const char *pos = strchr(szWholeText, '|');
	if (pos)
		{
			size_t index = pos - szWholeText;
			memmove(szStartText, szWholeText, index);
			szStartText[index] = '\0';
			StrTrim(szStartText, " ");
			memmove(szEndText, pos + 1, strlen(pos)); //copies the \0 as well ... :)
			StrTrim(szEndText, " ");
		}
		else{
			szStartText[0] = szEndText[0] = '\0';
		}
	return 0;
}

int GetStringFromDatabase(char *szSettingName, char *szError, char *szResult, size_t size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_ASCIIZ;
	if (DBGetContactSetting(NULL, ModuleName, szSettingName, &dbv) == 0)
		{
			res = 0;
			size_t tmp = strlen(dbv.pszVal);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, dbv.pszVal, len);
			szResult[len] = '\0';
			MirandaFree(dbv.pszVal);
		}
		else{
			res = 1;
			size_t tmp = strlen(szError);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, szError, len);
			szResult[len] = '\0';
		}
	return res;
}

char *RelativePathToAbsolute(char *szRelative, char *szAbsolute, size_t size)
{
	size_t len;

	if (size < MAX_PATH)
		{
			char buffer[MAX_PATH]; //new path should be at least MAX_PATH chars
			len = CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM) szRelative, (LPARAM) buffer);
			strncpy(szAbsolute, buffer, size);
		}
		else{
			len = CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM) szRelative, (LPARAM) szAbsolute);
		}
		
	return szAbsolute;
}

char *AbsolutePathToRelative(char *szAbsolute, char *szRelative, size_t size)
{
	size_t len;
	
	if (size < MAX_PATH)
		{
			char buffer[MAX_PATH];
			len = CallService(MS_UTILS_PATHTORELATIVE, (WPARAM) szAbsolute, (LPARAM) szRelative);
			strncpy(szRelative, buffer, size);
		}
		else{
			len = CallService(MS_UTILS_PATHTORELATIVE, (WPARAM) szAbsolute, (LPARAM) szRelative);
		}
		
	return szRelative;
}

void LogToFileInit()
{
#ifdef USE_LOG_FUNCTIONS
	DeleteFile("versioninfo.log");
#endif
}

void LogToFile(char *format, ...)
{
#ifdef USE_LOG_FUNCTIONS
	char str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen("versioninfo.log", "at");
	if (!fout)
		{
			Log("Can't open file versioninfo.log ...");
		}
	time_t currentTime = time(NULL);
	tm *timp = localtime(&currentTime);
	strftime(str, sizeof(str), "%d %b @ %H:%M:%S -> ", timp);
	fputs(str, fout);
	
	va_start(vararg, format);
	
	tBytes = _vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		{
			str[tBytes] = 0;
		}

	va_end(vararg);
	if (str[strlen(str) - 1] != '\n')
		{
			strcat(str, "\n");
		}
	
	fputs(str, fout);
	fclose(fout);
#endif	
}


#define GetFacility(dwError)  (HIWORD(dwError) && 0x0000111111111111)
#define GetErrorCode(dwError) (LOWORD(dwError))

void NotifyError(DWORD dwError, char* szSetting, int iLine) {
	char str[1024];
	mir_snprintf(str, sizeof(str), Translate("Ok, something went wrong in the \"%s\" setting. Report back the following values:\nFacility: %X\nError code: %X\nLine number: %d"), szSetting, GetFacility(dwError), GetErrorCode(dwError), iLine);
	Log(str);
}

char *StrTrim(char *szText, const char *szTrimChars)
{
	size_t i = strlen(szText) - 1;
	while ((i >= 0) && (strchr(szTrimChars, szText[i])))
		{
			szText[i--] = '\0';
		}
	i = 0;
	while (((unsigned int )i < strlen(szText)) && (strchr(szTrimChars, szText[i])))
		{
			i++;
		}
	if (i)
		{
 			size_t size = strlen(szText);
 			size_t j;
 			for (j = i; j <= size; j++) //shift the \0 as well
 				{
 					szText[j - i] = szText[j];
 				}
//			memmove(szText, szText + i, size - i + 1); //copy the string without the first i characters
		}
	return szText;
}

bool DoesDllExist(char *dllName)
{
	HMODULE dllHandle;
	dllHandle = LoadLibraryEx(dllName, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (dllHandle)
		{
			FreeLibrary(dllHandle);
			return true;
		}
	return false;
}

//========== From Cyreve ==========
PLUGININFOEX *GetPluginInfo(const char *filename,HINSTANCE *hPlugin)
{
	char szMirandaPath[MAX_PATH],szPluginPath[MAX_PATH];
	char *str2;
	PLUGININFOEX *(*MirandaPluginInfo)(DWORD);
	PLUGININFOEX *pPlugInfo;
	HMODULE hLoadedModule;
	DWORD mirandaVersion = CallService(MS_SYSTEM_GETVERSION,0,0);
	
	GetModuleFileName(GetModuleHandle(NULL),szMirandaPath,sizeof(szMirandaPath));
	str2=strrchr(szMirandaPath,'\\');
	if(str2!=NULL) *str2=0;

	hLoadedModule=GetModuleHandle(filename);
	if(hLoadedModule!=NULL) {
		*hPlugin=NULL;
		MirandaPluginInfo=(PLUGININFOEX *(*)(DWORD))GetProcAddress(hLoadedModule,"MirandaPluginInfo");
		return MirandaPluginInfo(mirandaVersion);
	}
	wsprintf(szPluginPath,"%s\\Plugins\\%s",szMirandaPath,filename);
	*hPlugin=LoadLibrary(szPluginPath);
	if(*hPlugin==NULL) return NULL;
	MirandaPluginInfo=(PLUGININFOEX *(*)(DWORD))GetProcAddress(*hPlugin,"MirandaPluginInfo");
	if(MirandaPluginInfo==NULL) {FreeLibrary(*hPlugin); *hPlugin=NULL; return NULL;}
	pPlugInfo=MirandaPluginInfo(mirandaVersion);
	if(pPlugInfo==NULL) {FreeLibrary(*hPlugin); *hPlugin=NULL; return NULL;}
	if(pPlugInfo->cbSize!=sizeof(PLUGININFOEX)) {FreeLibrary(*hPlugin); *hPlugin=NULL; return NULL;}
	return pPlugInfo;
}

//========== from Frank Cheng (wintime98) ==========
// I've changed something to suit VersionInfo :-)
#include <imagehlp.h>

void TimeStampToSysTime(DWORD Unix,SYSTEMTIME* SysTime)
{
	SYSTEMTIME S;
	DWORDLONG FileReal,UnixReal;
	S.wYear=1970;
	S.wMonth=1;
	S.wDay=1;
	S.wHour=0;
	S.wMinute=0;
	S.wSecond=0;
	S.wMilliseconds=0;
	SystemTimeToFileTime(&S,(FILETIME*)&FileReal);
	UnixReal = Unix;
	UnixReal*=10000000;
	FileReal+=UnixReal;
	FileTimeToSystemTime((FILETIME*)&FileReal,SysTime);
}

void GetModuleTimeStamp(char* pszDate, char* pszTime)
{
	char date[128],time[128],szModule[MAX_PATH];
	HANDLE mapfile,file;
	DWORD timestamp,filesize;
	LPVOID mapmem;
	SYSTEMTIME systime;
	GetModuleFileName(NULL,szModule,MAX_PATH);
	file = CreateFile(szModule,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	filesize = GetFileSize(file,NULL);
	mapfile = CreateFileMapping(file, NULL, PAGE_READONLY, 0, filesize, NULL);
	mapmem = MapViewOfFile(mapfile, FILE_MAP_READ, 0, 0, 0);
	timestamp = GetTimestampForLoadedLibrary((HINSTANCE)mapmem);
	TimeStampToSysTime(timestamp,&systime);
	GetTimeFormat(LOCALE_USER_DEFAULT,0,&systime,"HH':'mm':'ss",time,128);
	GetDateFormat(EnglishLocale,0,&systime,"dd' 'MMMM' 'yyyy",date,128);
	//MessageBox(NULL,date,time,0);
	lstrcpy(pszTime, time);
	lstrcpy(pszDate, date);
	UnmapViewOfFile(mapmem);
	CloseHandle(mapfile);
	CloseHandle(file);
}

//From Egodust or Cyreve... I don't really know.
PLUGININFOEX *CopyPluginInfo(PLUGININFOEX *piSrc)
{
	PLUGININFOEX *pi;
	if(piSrc==NULL) return NULL;
	pi=(PLUGININFOEX *)malloc(sizeof(PLUGININFOEX));
	
	*pi=*piSrc;
	pi->uuid = UUID_NULL;
	
	if (piSrc->cbSize >= sizeof(PLUGININFOEX))
	{
		pi->uuid = piSrc->uuid;
	}
	
	if (piSrc->cbSize >= sizeof(PLUGININFO))
	{
		if(pi->author) pi->author=_strdup(pi->author);
		if(pi->authorEmail) pi->authorEmail=_strdup(pi->authorEmail);
		if(pi->copyright) pi->copyright=_strdup(pi->copyright);
		if(pi->description) pi->description=_strdup(pi->description);
		if(pi->homepage) pi->homepage=_strdup(pi->homepage);
		if(pi->shortName) pi->shortName=_strdup(pi->shortName);
	}
	
	return pi;
}

void FreePluginInfo(PLUGININFOEX *pi)
{
	if(pi->author) free(pi->author);
	if(pi->authorEmail) free(pi->authorEmail);
	if(pi->copyright) free(pi->copyright);
	if(pi->description) free(pi->description);
	if(pi->homepage) free(pi->homepage);
	if(pi->shortName) free(pi->shortName);
	free(pi);
}

BOOL IsCurrentUserLocalAdministrator(void)
{
   BOOL   fReturn         = FALSE;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;

   HANDLE hToken              = NULL;
   HANDLE hImpersonationToken = NULL;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;


   /*
      Determine if the current thread is running as a user that is a member of
      the local admins group.  To do this, create a security descriptor that
      has a DACL which has an ACE that allows only local aministrators access.
      Then, call AccessCheck with the current thread's token and the security
      descriptor.  It will say whether the user could access an object if it
      had that security descriptor.  Note: you do not need to actually create
      the object.  Just checking access against the security descriptor alone
      will be sufficient.
   */
   const DWORD ACCESS_READ  = 1;
   const DWORD ACCESS_WRITE = 2;


   __try
   {

      /*
         AccessCheck() requires an impersonation token.  We first get a primary
         token and then create a duplicate impersonation token.  The
         impersonation token is not actually assigned to the thread, but is
         used in the call to AccessCheck.  Thus, this function itself never
         impersonates, but does use the identity of the thread.  If the thread
         was impersonating already, this function uses that impersonation context.
      */
      if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, TRUE, &hToken))
      {
         if (GetLastError() != ERROR_NO_TOKEN)
            __leave;

         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
            __leave;
      }

      if (!DuplicateToken (hToken, SecurityImpersonation, &hImpersonationToken))
          __leave;


      /*
        Create the binary representation of the well-known SID that
        represents the local administrators group.  Then create the security
        descriptor and DACL with an ACE that allows only local admins access.
        After that, perform the access check.  This will determine whether
        the current user is a local admin.
      */
      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin))
         __leave;

      psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         __leave;

      if (!InitializeSecurityDescriptor(psdAdmin, SECURITY_DESCRIPTOR_REVISION))
         __leave;

      // Compute size needed for the ACL.
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidAdmin) - sizeof(DWORD);

      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         __leave;

      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         __leave;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;

      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
         __leave;

      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         __leave;

      /*
         AccessCheck validates a security descriptor somewhat; set the group
         and owner so that enough of the security descriptor is filled out to
         make AccessCheck happy.
      */
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
         __leave;

      dwAccessDesired = ACCESS_READ;

      /*
         Initialize GenericMapping structure even though you
         do not use generic rights.
      */
      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired, &GenericMapping, &ps, &dwStructureSize, &dwStatus, &fReturn))
      {
         fReturn = FALSE;
         __leave;
      }
   }
   __finally
   {
      // Clean up.
      if (pACL) LocalFree(pACL);
      if (psdAdmin) LocalFree(psdAdmin);
      if (psidAdmin) FreeSid(psidAdmin);
      if (hImpersonationToken) CloseHandle (hImpersonationToken);
      if (hToken) CloseHandle (hToken);
   }

   return fReturn;
}

BOOL GetWindowsShell(char *shellPath, size_t shSize)
{
	OSVERSIONINFO vi = {0};
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&vi);

	char szShell[1024] = {0};
	DWORD size = sizeof(szShell);
	
	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\system.ini\\boot", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, "Shell", NULL, NULL, (LPBYTE) szShell, &size);
			_strlwr(szShell);
			HKEY hRootKey = (strstr(szShell, "sys:") == szShell) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
			RegCloseKey(hKey);
			
			strcpy(szShell, "<unknown>");
			if (RegOpenKeyEx(hRootKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
			{
				size = sizeof(szShell);
				RegQueryValueEx(hKey, "Shell", NULL, NULL, (LPBYTE) szShell, &size);
				RegCloseKey(hKey);
			}
		}
	}
	else{
		char szSystemIniPath[2048];
		GetWindowsDirectory(szSystemIniPath, sizeof(szSystemIniPath));
		size_t len = strlen(szSystemIniPath);
		if (len > 0)
		{
			if (szSystemIniPath[len - 1] == '\\') { szSystemIniPath[--len] = '\0'; }
			strcat(szSystemIniPath, "\\system.ini");
			GetPrivateProfileString("boot", "shell", "<unknown>", szShell, size, szSystemIniPath);
		}
	}
	
	char *pos = strrchr(szShell, '\\');
	char *res = (pos) ? pos + 1 : szShell;
	strncpy(shellPath, res, shSize);
	
	return TRUE;
}

BOOL GetInternetExplorerVersion(char *ieVersion, size_t ieSize)
{
	HKEY hKey;
	char ieVer[1024];
	DWORD size = sizeof(ieVer);
	char ieBuild[64] = {0};
	
	strncpy(ieVer, "<not installed>", size);
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, "Version", NULL, NULL, (LPBYTE) ieVer, &size) == ERROR_SUCCESS)
		{
			char *pos = strchr(ieVer, '.');
			if (pos)
			{
				pos = strchr(pos + 1, '.');
				if (pos) { *pos = 0; }
				strncpy(ieBuild, pos + 1, sizeof(ieBuild));
				pos = strchr(ieBuild, '.');
				if (pos) { *pos = 0; }
			}
		}
		else{
			size = sizeof(ieVer);
			if (RegQueryValueEx(hKey, "Build", NULL, NULL, (LPBYTE) ieVer, &size) == ERROR_SUCCESS)
			{
				char *pos = ieVer + 1;
				strncpy(ieBuild, pos, sizeof(ieBuild));
				*pos = 0;
				pos = strchr(ieBuild, '.');
				if (pos) { *pos = 0; }
			}
			else{
				strncpy(ieVer, "<unknown version>", size);
			}
		}
		RegCloseKey(hKey);
	}
	
	strncpy(ieVersion, ieVer, ieSize);
	if (strlen(ieBuild) > 0)
	{
		strncat(ieVersion, ".", ieSize);
		strncat(ieVersion, ieBuild, ieSize);
		//strncat(ieVersion, ")", ieSize);
	}
	
	return TRUE;
}


char *GetLanguageName(LANGID language)
{
	
	LCID lc = MAKELCID(language, SORT_DEFAULT);
	
	return GetLanguageName(lc);
}

extern char *GetLanguageName(LCID locale)
{
	static char name[1024];
	
	GetLocaleInfo(locale, LOCALE_SENGLANGUAGE, name, sizeof(name));
	
	return name;
}

BOOL UUIDToString(MUUID uuid, char *str, size_t len)
{
	if ((len < sizeof(MUUID)) || (!str))
	{
		return 0;
	}
	
	mir_snprintf(str, len, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
	
	return 1;
}

BOOL IsUUIDNull(MUUID uuid)
{
	int i;
	for (i = 0; i < sizeof(uuid.d); i++)
	{
		if (uuid.d[i])
		{
			return 0;
		}
	}
	
	return ((uuid.a == 0) && (uuid.b == 0) && (uuid.c == 0));
}