/*
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
*/

#ifndef M_CUSTOM_FOLDERS_H
#define M_CUSTOM_FOLDERS_H

#define FOLDERS_API 501 //dunno why it's here but it is :)

#define PROFILE_PATH "%profile_path%"
#define CURRENT_PROFILE "%current_profile%"
#define MIRANDA_PATH "%miranda_path%"
#define PLUGINS_PATH "%miranda_path%" "\\plugins"
#define MIRANDA_USERDATA "%miranda_userdata%"

#define PROFILE_PATHW    L"%profile_path%"
#define CURRENT_PROFILEW L"%current_profile%"
#define MIRANDA_PATHW    L"%miranda_path%"
#define PLUGINS_PATHW L"%miranda_path%" L"\\plugins"
#define MIRANDA_USERDATAW L"%miranda_userdata%"

#define FOLDER_AVATARS                 PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\avatars"
#define FOLDER_VCARDS                  PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\vcards"
#define FOLDER_LOGS                    PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\logs"
#define FOLDER_RECEIVED_FILES          PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\received files"
#define FOLDER_DOCS                    MIRANDA_PATHW L"\\" L"docs"
#define FOLDER_CONFIG                  PLUGINS_PATHW L"\\" L"config"
#define FOLDER_SCRIPTS                 MIRANDA_PATHW L"\\" L"scripts"
#define FOLDER_UPDATES                 MIRANDA_PATHW L"\\" L"updates"

#define FOLDER_CUSTOMIZE               MIRANDA_PATHW L"\\" L"customize"
#define FOLDER_CUSTOMIZE_SOUNDS        FOLDER_CUSTOMIZE L"\\sounds"
#define FOLDER_CUSTOMIZE_ICONS         FOLDER_CUSTOMIZE L"\\icons"
#define FOLDER_CUSTOMIZE_SMILEYS       FOLDER_CUSTOMIZE L"\\smileys"
#define FOLDER_CUSTOMIZE_SKINS         FOLDER_CUSTOMIZE L"\\skins"
#define FOLDER_CUSTOMIZE_THEMES        FOLDER_CUSTOMIZE L"\\themes"

#define FOLDERS_NAME_MAX_SIZE 64  // maximum name and section size

#define FF_UNICODE 0x00000001

struct FOLDERSDATA
{
	DWORD flags;                   // FF_* flags
	HPLUGIN plugin;                // plugin which owns a folder
	LPCSTR szSection;              // section name, if it doesn't exist it will be created otherwise it will just add this entry to it
	LPCSTR szName;                 // entry name - will be shown in options
	MAllCStrings szFormat;         // default string format. Fallback string in case there's no entry in the database for this folder. 
	                               // This should be the initial value for the path, users will be able to change it later.
											 // String is dup()'d so you can free it later. If you set the unicode string don't forget to set the flag accordingly.
};

/*Folders/Register/Path service
  wParam - not used, must be 0
  lParam - (LPARAM) (const FOLDERDATA *) - Data structure filled with
  the necessary information.
  Returns a handle to the registered path or 0 on error.
  You need to use this to call the other services.
  */
#define MS_FOLDERS_REGISTER_PATH "Folders/Register/Path"

  /*Folders/Get/PathSize service
	 wParam - (WPARAM) (int) - handle to registered path
	 lParam - (LPARAM) (int *) - pointer to the variable that receives the size of the path
	 string (not including the null character). Depending on the flags set when creating the path
	 it will either call mir_strlen() or mir_wstrlen() to get the length of the string.
	 Returns the size of the buffer.
	 */
#define MS_FOLDERS_GET_SIZE "Folders/Get/PathSize"

struct FOLDERSGETDATA
{
	int nMaxPathSize;      // maximum size of buffer. This represents the number of characters that can be copied to it (so for unicode strings you don't send the number of bytes but the length of the string).
	MAllStrings szPath;    // pointer to the buffer that receives the path without the last "\\"
	DWORD flags;           // FF_* flags
};

/*Folders/Get/Path service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (FOLDERSGETDATA *) pointer to a FOLDERSGETDATA that has all the relevant fields filled.
  Should return 0 on success, or nonzero otherwise.
  */
#define MS_FOLDERS_GET_PATH "Folders/Get/Path"

  /*Folders/On/Path/Changed
	  wParam - (WPARAM) 0
	  lParam - (LPARAM) 0
	  Triggered when the folders change, you should reget the paths you registered.
	  */
#define ME_FOLDERS_PATH_CHANGED "Folders/On/Path/Changed"

__inline static HANDLE FoldersRegisterCustomPath(const char *section, const char *name, const char *defaultPath)
{
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH))
		return nullptr;

	FOLDERSDATA fd = { sizeof(fd) };
	fd.plugin = (HPLUGIN)&g_plugin;
	fd.szSection = section;
	fd.szName = name;
	fd.szFormat.a = defaultPath;
	return (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);
}

#ifdef _UNICODE
__inline static HANDLE FoldersRegisterCustomPathW(const char *section, const char *name, const wchar_t *defaultPathW)
{
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH))
		return nullptr;

	FOLDERSDATA fd = { sizeof(fd) };
	fd.plugin = (HPLUGIN)&g_plugin;
	fd.szSection = section;
	fd.szName = name;
	fd.szFormat.w = defaultPathW;
	fd.flags = FF_UNICODE;
	return (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);
}
#endif //_UNICODE

__inline static INT_PTR FoldersGetCustomPath(HANDLE hFolderEntry, char *path, const int size, const char *notFound)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPath.a = path;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		char buffer[MAX_PATH];
		PathToAbsolute(notFound, buffer);
		strncpy_s(path, size, buffer, _TRUNCATE);
	}

	return res;
}

#ifdef _UNICODE
__inline static INT_PTR FoldersGetCustomPathW(HANDLE hFolderEntry, wchar_t *pathW, const int size, const wchar_t *notFoundW)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPath.w = pathW;
	fgd.flags = FF_UNICODE;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		wchar_t buffer[MAX_PATH];
		PathToAbsoluteW(notFoundW, buffer);
		wcsncpy_s(pathW, size, buffer, _TRUNCATE);
	}

	return res;
}
#endif //_UNICODE

__inline static INT_PTR FoldersGetCustomPathEx(HANDLE hFolderEntry, char *path, const int size, char *notFound, char *fileName)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPath.a = path;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		char buffer[MAX_PATH];
		PathToAbsolute(notFound, buffer);
		strncpy_s(path, size, buffer, _TRUNCATE);
	}

	if (path[0] != '\0')
		mir_strncat(path, "\\", size - mir_strlen(path));

	if (fileName)
		mir_strncat(path, fileName, size - mir_strlen(path));

	return res;
}

#ifdef _UNICODE
__inline static INT_PTR FoldersGetCustomPathExW(HANDLE hFolderEntry, wchar_t *pathW, const int size, wchar_t *notFoundW, wchar_t *fileNameW)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPath.w = pathW;
	fgd.flags = FF_UNICODE;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		wchar_t buffer[MAX_PATH];
		PathToAbsoluteW(notFoundW, buffer);
		wcsncpy_s(pathW, size, buffer, _TRUNCATE);
	}

	if (pathW[0] != '\0')
		mir_wstrncat(pathW, L"\\", size - mir_wstrlen(pathW));

	if (fileNameW)
		mir_wstrncat(pathW, fileNameW, size - mir_wstrlen(pathW));

	return res;
}
#endif //_UNICODE

#endif //M_CUSTOM_FOLDERS_H
