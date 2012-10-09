{
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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
}

{$IFNDEF M_FOLDERS}
{$DEFINE M_FOLDERS}

const
  FOLDERS_API = 501; //dunno why it's here but it is :)

  PROFILE_PATH     = '%profile_path%';
  CURRENT_PROFILE  = '%current_profile%';
  MIRANDA_PATH     = '%miranda_path%';
  PLUGINS_PATH     = '%miranda_path%\plugins';
  MIRANDA_USERDATA = '%miranda_userdata%';

  PROFILE_PATHW     = '%profile_path%';
  CURRENT_PROFILEW  = '%current_profile%';
  MIRANDA_PATHW     = '%miranda_path%';
  MIRANDA_USERDATAW = '%miranda_userdata%';

  FOLDER_AVATARS        = PROFILE_PATH+'\'+CURRENT_PROFILE+'\avatars';
  FOLDER_VCARDS         = PROFILE_PATH+'\'+CURRENT_PROFILE+'\vcards';
  FOLDER_LOGS           = PROFILE_PATH+'\'+CURRENT_PROFILE+'\logs';
  FOLDER_RECEIVED_FILES = PROFILE_PATH+'\'+CURRENT_PROFILE+'\received files';
  FOLDER_DOCS           = MIRANDA_PATH+'\'+'docs';

  FOLDER_CONFIG  = PLUGINS_PATH+'\config';
  FOLDER_SCRIPTS = MIRANDA_PATH+'\scripts';
  FOLDER_UPDATES = MIRANDA_PATH+'\updates';

  FOLDER_CUSTOMIZE         = MIRANDA_PATH+'\customize';
  FOLDER_CUSTOMIZE_SOUNDS  = FOLDER_CUSTOMIZE+'\sounds';
  FOLDER_CUSTOMIZE_ICONS   = FOLDER_CUSTOMIZE+'\icons';
  FOLDER_CUSTOMIZE_SMILEYS = FOLDER_CUSTOMIZE+'\smileys';
  FOLDER_CUSTOMIZE_SKINS   = FOLDER_CUSTOMIZE+'\skins';
  FOLDER_CUSTOMIZE_THEMES  = FOLDER_CUSTOMIZE+'\themes';

  FOLDERS_NAME_MAX_SIZE = 64;  //maximum name and section size

  FF_UNICODE = $00000001;

type
  TFOLDERSDATA = record
    cbSize:integer;                                  //size of struct
    //section name, if it doesn't exist it will be created otherwise it will just add this entry to it
    szSection:array [0..FOLDERS_NAME_MAX_SIZE-1] of AnsiChar;
    szName   :array [0..FOLDERS_NAME_MAX_SIZE-1] of AnsiChar;  //entry name - will be shown in options
    szFormat :TCHAR; // default string format. Fallback string in case
                     // there's no entry in the database for this
                     // folder. This should be the initial value for
                     // the path, users will be able to change it later.
    flags    :DWORD; // FF_* flags
  end;

const
{
  Folders/Register/Path service
  wParam - not used, must be 0
  lParam - (LPARAM) (const FOLDERDATA *) - Data structure filled with
  the necessary information.
  Returns a handle to the registered path or 0 on error.
  You need to use this to call the other services.
}
  MS_FOLDERS_REGISTER_PATH = 'Folders/Register/Path';

{
  Folders/Get/PathSize service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (int *) - pointer to the variable that receives the size of the path
  string (not including the null character). Depending on the flags set when creating the path
  it will either call strlen() or wcslen() to get the length of the string.
  Returns the size of the buffer.
}
  MS_FOLDERS_GET_SIZE = 'Folders/Get/PathSize';

type
  TFOLDERSGETDATA = record
    cbSize:integer;
    nMaxPathSize:integer; // maximum size of buffer. This represents the number
                          // of characters that can be copied to it (so for
                          // unicode strings you don't send the number of
                          // bytes but the length of the string).
		szPath:TChar;         //pointer to the buffer that receives the path without the last "\\"
  end;

const
{
  Folders/Get/Path service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (FOLDERSGETDATA *) pointer to a FOLDERSGETDATA that has all the relevant fields filled.
  Should return 0 on success, or nonzero otherwise.
}
  MS_FOLDERS_GET_PATH = 'Folders/Get/Path';

type
  TFOLDERSGETALLOCDATA = record
    cbSize:integer;
    szPath:^TCHAR; // address of a string variable where the path should be
                   // stored (the last \ won't be copied).
  end;

const
{
  Folders/GetRelativePath/Alloc service
  wParam - (WPARAM) (int) - Handle to registered path
  lParam - (LPARAM) (FOLDERSALLOCDATA *) data
  This service is the same as MS_FOLDERS_GET_PATH with the difference that this service
  allocates the needed space for the buffer. It uses miranda's memory functions for that and you need
  to use those to free the resulting buffer.
  Should return 0 on success, or nonzero otherwise. Currently it only returns 0.
}
  MS_FOLDERS_GET_PATH_ALLOC = 'Folders/Get/Path/Alloc';

{
  Folders/On/Path/Changed
	wParam - (WPARAM) 0
	lParam - (LPARAM) 0
	Triggered when the folders change, you should reget the paths you registered.
}
  ME_FOLDERS_PATH_CHANGED = 'Folders/On/Path/Changed';

(*
#ifndef FOLDERS_NO_HELPER_FUNCTIONS

#ifndef M_UTILS_H__
#error The helper functions require that m_utils.h be included in the project. Please include that file if you want to use the helper functions. If you don't want to use the functions just define FOLDERS_NO_HELPER_FUNCTIONS.
#endif

//#include "../../../include/newpluginapi.h"

__inline static HANDLE FoldersRegisterCustomPath(const AnsiChar *section, const AnsiChar *name, const AnsiChar *defaultPath)
{
	FOLDERSDATA fd = {0};
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH)) return 0;
	fd.cbSize = sizeof(FOLDERSDATA);
	strncpy(fd.szSection, section, FOLDERS_NAME_MAX_SIZE);
	fd.szSection[FOLDERS_NAME_MAX_SIZE - 1] = '\0';
	strncpy(fd.szName, name, FOLDERS_NAME_MAX_SIZE);
	fd.szName[FOLDERS_NAME_MAX_SIZE - 1] = '\0';
	fd.szFormat = defaultPath;
	return (HANDLE) CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &fd);
}

__inline static HANDLE FoldersRegisterCustomPathW(const AnsiChar *section, const AnsiChar *name, const wchar_t *defaultPathW)
{
	FOLDERSDATA fd = {0};
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH)) return 0;
	fd.cbSize = sizeof(FOLDERSDATA);
	strncpy(fd.szSection, section, FOLDERS_NAME_MAX_SIZE);
	fd.szSection[FOLDERS_NAME_MAX_SIZE - 1] = '\0'; //make sure it's NULL terminated
	strncpy(fd.szName, name, FOLDERS_NAME_MAX_SIZE);
	fd.szName[FOLDERS_NAME_MAX_SIZE - 1] = '\0'; //make sure it's NULL terminated
	fd.szFormatW = defaultPathW;
	fd.flags = FF_UNICODE;
	return (HANDLE) CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &fd);
}

__inline static int FoldersGetCustomPath(HANDLE hFolderEntry, AnsiChar *path, const int size, AnsiChar *notFound)
{
	FOLDERSGETDATA fgd = {0};
	int res;
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = size;
	fgd.szPath = path;
	res = CallService(MS_FOLDERS_GET_PATH, (WPARAM) hFolderEntry, (LPARAM) &fgd);
	if (res)
	{
		AnsiChar buffer[MAX_PATH];
		CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM) notFound, (LPARAM) buffer);
		mir_snprintf(path, size, "%s", buffer);
	}
		
	return res;
}

__inline static int FoldersGetCustomPathW(HANDLE hFolderEntry, wchar_t *pathW, const int count, wchar_t *notFoundW)
{
	FOLDERSGETDATA fgd = {0};
	int res;
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = count;
	fgd.szPathW = pathW;
	res = CallService(MS_FOLDERS_GET_PATH, (WPARAM) hFolderEntry, (LPARAM) &fgd);
	if (res)
	{
		wcsncpy(pathW, notFoundW, count);
		pathW[count - 1] = '\0';
	}
		
	return res;
}

__inline static int FoldersGetCustomPathEx(HANDLE hFolderEntry, AnsiChar *path, const int size, AnsiChar *notFound, AnsiChar *fileName)
{
	FOLDERSGETDATA fgd = {0};
	int res;
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = size;
	fgd.szPath = path;
	res = CallService(MS_FOLDERS_GET_PATH, (WPARAM) hFolderEntry, (LPARAM) &fgd);
	if (res)
	{
		AnsiChar buffer[MAX_PATH];
		CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM) notFound, (LPARAM) buffer);
		mir_snprintf(path, size, "%s", buffer);
	}
	if (strlen(path) > 0)
	{
		strcat(path, "\\");
	}
	else{
		path[0] = '\0';
	}
	
	if (fileName)
	{
		strcat(path, fileName);
	}
	
	return res;
}

__inline static int FoldersGetCustomPathExW(HANDLE hFolderEntry, wchar_t *pathW, const int count, wchar_t *notFoundW, wchar_t *fileNameW)
{
	FOLDERSGETDATA fgd = {0};
	int res;
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = count;
	fgd.szPathW = pathW;
	res = CallService(MS_FOLDERS_GET_PATH, (WPARAM) hFolderEntry, (LPARAM) &fgd);
	if (res)
	{
		wcsncpy(pathW, notFoundW, count);
		pathW[count - 1] = '\0';
	}
	
	if (wcslen(pathW) > 0)
	{
		wcscat(pathW, L"\\");
	}
	else{
		pathW[0] = L'\0';
	}
	
	if (fileNameW)
	{
		wcscat(pathW, fileNameW);
	}
	
	return res;
}

#endif
*)
{$ENDIF}
