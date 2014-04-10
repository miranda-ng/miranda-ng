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

void Log(const TCHAR* message)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		POPUPDATAT pu = {0};
		pu.lchIcon = hiVIIcon;
		_tcsncpy(pu.lptzContactName, TranslateT("Version Information"), MAX_CONTACTNAME);
		_tcsncpy(pu.lptzText, message, MAX_SECONDLINE);
		PUAddPopupT(&pu);
	}
	else MessageBox(NULL, message, _T("VersionInfo"), MB_OK | MB_ICONINFORMATION);
}

int SplitStringInfo(const TCHAR *szWholeText, TCHAR *szStartText, TCHAR *szEndText)
{
	const TCHAR *pos = _tcschr(szWholeText, '|');
	if (pos) {
		size_t index = pos - szWholeText;
		lstrcpyn(szStartText, szWholeText, (int)index);
		szStartText[index] = '\0';
		StrTrim(szStartText, _T(" "));
		lstrcpyn(szEndText, pos + 1, (int)_tcslen(pos)); //copies the \0 as well ... :)
		StrTrim(szEndText, _T(" "));
	}
	else szStartText[0] = szEndText[0] = '\0';

	return 0;
}

int GetStringFromDatabase(char *szSettingName, TCHAR *szError, TCHAR *szResult, size_t size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	if ( db_get_ts(NULL, ModuleName, szSettingName, &dbv) == 0) {
		res = 0;
		size_t tmp = _tcslen(dbv.ptszVal);
		len = (tmp < size - 1) ? tmp : size - 1;
		_tcsncpy(szResult, dbv.ptszVal, len);
		szResult[len] = '\0';
		mir_free(dbv.ptszVal);
	}
	else {
		res = 1;
		size_t tmp = _tcslen(szError);
		len = (tmp < size - 1) ? tmp : size - 1;
		_tcsncpy(szResult, szError, len);
		szResult[len] = '\0';
	}
	return res;
}

#define GetFacility(dwError)  (HIWORD(dwError) && 0x0000111111111111)
#define GetErrorCode(dwError) (LOWORD(dwError))

void NotifyError(DWORD dwError, const TCHAR* szSetting, int iLine)
{
	TCHAR str[1024];
	mir_sntprintf(str, SIZEOF(str), TranslateT("Something went wrong in the \"%s\" setting. Report back the following values:\nFacility: %X\nError code: %X\nLine number: %d"), szSetting, GetFacility(dwError), GetErrorCode(dwError), iLine);
	Log(str);
}

TCHAR *StrTrim(TCHAR *szText, const TCHAR *szTrimChars)
{
	size_t i = _tcslen(szText) - 1;
	while (i >= 0 && _tcschr(szTrimChars, szText[i]))
		szText[i--] = '\0';

	i = 0;
	while (((unsigned int )i < _tcslen(szText)) && _tcschr(szTrimChars, szText[i]))
		i++;

	if (i) {
		size_t size = _tcslen(szText);
		size_t j;
		for (j = i; j <= size; j++) //shift the \0 as well
			szText[j - i] = szText[j];
	}
	return szText;
}

bool DoesDllExist(char *dllName)
{
	HMODULE dllHandle;
	dllHandle = LoadLibraryExA(dllName, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (dllHandle)
		{
			FreeLibrary(dllHandle);
			return true;
		}
	return false;
}

void GetISO8061Time(SYSTEMTIME* stLocal, LPTSTR lpszString, DWORD dwSize)
{
	SYSTEMTIME loctime;
	if (stLocal == NULL) 
	{
		stLocal = &loctime;
		GetLocalTime(stLocal);
	}

	GetDateFormat(LOCALE_INVARIANT, 0, stLocal, TEXT("d MMM yyyy"), lpszString, dwSize);
	int dlen = (int)_tcslen(lpszString);
	GetTimeFormat(LOCALE_INVARIANT, 0, stLocal, TEXT(" H:mm:ss"), lpszString+dlen, dwSize-dlen);
}

void GetModuleTimeStamp(LPTSTR lpszString, DWORD dwSize)
{
	TCHAR tszModule[MAX_PATH];
	GetModuleFileName(NULL, tszModule, SIZEOF(tszModule));
	WIN32_FIND_DATA fd;
	if (FindFirstFile(tszModule, &fd) != INVALID_HANDLE_VALUE) {
		FILETIME ftLocal;
		SYSTEMTIME stLocal;
		FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &stLocal);
		GetISO8061Time(&stLocal, lpszString, dwSize);
	}
}

//From Egodust or Cyreve... I don't really know.
PLUGININFOEX *CopyPluginInfo(PLUGININFOEX *piSrc)
{
	if(piSrc==NULL) 
		return NULL;

	PLUGININFOEX *pi = (PLUGININFOEX *)malloc(sizeof(PLUGININFOEX));
	*pi = *piSrc;
	
	if (piSrc->cbSize >= sizeof(PLUGININFOEX))
		pi->uuid = piSrc->uuid;
	else
		pi->uuid = UUID_NULL;
		
	if (pi->author) pi->author = _strdup(pi->author);
	if (pi->authorEmail) pi->authorEmail = _strdup(pi->authorEmail);
	if (pi->copyright) pi->copyright = _strdup(pi->copyright);
	if (pi->description) pi->description = _strdup(pi->description);
	if (pi->homepage) pi->homepage = _strdup(pi->homepage);
	if (pi->shortName) pi->shortName = _strdup(pi->shortName);
	return pi;
}

void FreePluginInfo(PLUGININFOEX *pi)
{
	if (pi->author) free(pi->author);
	if (pi->authorEmail) free(pi->authorEmail);
	if (pi->copyright) free(pi->copyright);
	if (pi->description) free(pi->description);
	if (pi->homepage) free(pi->homepage);
	if (pi->shortName) free(pi->shortName);
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

BOOL GetWindowsShell(TCHAR *shellPath, size_t shSize)
{
	OSVERSIONINFO vi = {0};
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&vi);

	TCHAR szShell[1024] = {0};
	DWORD size = SIZEOF(szShell);
	
	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\system.ini\\boot"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			RegQueryValueEx(hKey, _T("Shell"), NULL, NULL, (LPBYTE) szShell, &size);
			_tcslwr(szShell);
			HKEY hRootKey = ( _tcsstr(szShell, _T("sys:")) == szShell) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
			RegCloseKey(hKey);
			
			_tcscpy(szShell, _T("<unknown>"));
			if (RegOpenKeyEx(hRootKey, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
			{
				size = SIZEOF(szShell);
				RegQueryValueEx(hKey, _T("Shell"), NULL, NULL, (LPBYTE) szShell, &size);
				RegCloseKey(hKey);
			}
		}
	}
	else{
		TCHAR szSystemIniPath[2048];
		GetWindowsDirectory(szSystemIniPath, SIZEOF(szSystemIniPath));
		size_t len = lstrlen(szSystemIniPath);
		if (len > 0)
		{
			if (szSystemIniPath[len - 1] == '\\') { szSystemIniPath[--len] = '\0'; }
			_tcscat(szSystemIniPath, _T("\\system.ini"));
			GetPrivateProfileString( _T("boot"), _T("shell"), _T("<unknown>"), szShell, size, szSystemIniPath);
		}
	}
	
	TCHAR *pos = _tcsrchr(szShell, '\\');
	TCHAR *res = (pos) ? pos + 1 : szShell;
	_tcsncpy(shellPath, res, shSize);
	
	return TRUE;
}

BOOL GetInternetExplorerVersion(TCHAR *ieVersion, size_t ieSize)
{
	HKEY hKey;
	TCHAR ieVer[1024] = {0}, ieBuild[64] = {0}, iVer[64] = {0};
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Internet Explorer"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD size;
		size = SIZEOF(ieVer);
		if (RegQueryValueEx(hKey, TEXT("Version"), NULL, NULL, (LPBYTE) ieVer, &size) != ERROR_SUCCESS)
			ieVer[0] = 0;
		size = SIZEOF(ieBuild);
		if (RegQueryValueEx(hKey, TEXT("Build"), NULL, NULL, (LPBYTE) ieBuild, &size) != ERROR_SUCCESS)
			ieBuild[0] = 0;
		size = SIZEOF(iVer);
		if (RegQueryValueEx(hKey, TEXT("IVer"), NULL, NULL, (LPBYTE) iVer, &size) != ERROR_SUCCESS)
			iVer[0] = 0;

		RegCloseKey(hKey);
	}

	if (ieVer[0] == 0)
	{
		if (iVer[0] == 0)
			_tcsncpy(ieVersion, _T("<not installed>"), ieSize);
		else if (_tcscmp(iVer, _T("100")) == 0)
			_tcsncpy(ieVersion, _T("1.0"), ieSize);
		else if (_tcscmp(iVer, _T("101")) == 0)
			_tcsncpy(ieVersion, _T("NT"), ieSize);
		else if (_tcscmp(iVer, _T("102")) == 0)
			_tcsncpy(ieVersion, _T("2.0"), ieSize);
		else if (_tcscmp(iVer, TEXT("103")) == 0)
			_tcsncpy(ieVersion, _T("3.0"), ieSize);
	} else 
		_tcsncpy(ieVersion, ieVer, ieSize);

	if (ieBuild[0] != 0)
	{
		_tcsncat(ieVersion, _T(" (build "), ieSize);
		_tcsncat(ieVersion, ieBuild, ieSize);
		_tcsncat(ieVersion, _T(")"), ieSize);
	}
	
	return TRUE;
}

TCHAR *GetLanguageName(LANGID language)
{
	LCID lc = MAKELCID(language, SORT_DEFAULT);
	return GetLanguageName(lc);
}

extern TCHAR *GetLanguageName(LCID locale)
{
	static TCHAR name[1024];
	GetLocaleInfo(locale, LOCALE_SENGLANGUAGE, name, SIZEOF(name));
	return name;
}

BOOL UUIDToString(MUUID uuid, TCHAR *str, size_t len)
{
	if ( len < sizeof(MUUID) || !str )
		return 0;
	
	mir_sntprintf(str, len, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"), uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
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