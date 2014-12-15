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

#ifdef _UNICODE
#define PROFILE_PATHT		PROFILE_PATHW
#define CURRENT_PROFILET	CURRENT_PROFILEW
#define MIRANDA_PATHT		MIRANDA_PATHW
#define PLUGINS_PATHT		PLUGINS_PATHW
#define MIRANDA_USERDATAT	MIRANDA_USERDATAW
#else
#define PROFILE_PATHT		PROFILE_PATH
#define CURRENT_PROFILET	CURRENT_PROFILE
#define MIRANDA_PATHT		MIRANDA_PATH
#define PLUGINS_PATHT		PLUGINS_PATH
#define MIRANDA_USERDATAT	MIRANDA_USERDATA
#endif

#define FOLDER_AVATARS                 PROFILE_PATHT _T("\\") CURRENT_PROFILET _T("\\avatars")
#define FOLDER_VCARDS                  PROFILE_PATHT _T("\\") CURRENT_PROFILET _T("\\vcards")
#define FOLDER_LOGS                    PROFILE_PATHT _T("\\") CURRENT_PROFILET _T("\\logs")
#define FOLDER_RECEIVED_FILES          PROFILE_PATHT _T("\\") CURRENT_PROFILET _T("\\received files")
#define FOLDER_DOCS                    MIRANDA_PATHT _T("\\") _T("docs")
#define FOLDER_CONFIG                  PLUGINS_PATHT _T("\\") _T("config")
#define FOLDER_SCRIPTS                 MIRANDA_PATHT _T("\\") _T("scripts")
#define FOLDER_UPDATES                 MIRANDA_PATHT _T("\\") _T("updates")

#define FOLDER_CUSTOMIZE               MIRANDA_PATHT _T("\\") _T("customize")
#define FOLDER_CUSTOMIZE_SOUNDS        FOLDER_CUSTOMIZE _T("\\sounds")
#define FOLDER_CUSTOMIZE_ICONS         FOLDER_CUSTOMIZE _T("\\icons")
#define FOLDER_CUSTOMIZE_SMILEYS       FOLDER_CUSTOMIZE _T("\\smileys")
#define FOLDER_CUSTOMIZE_SKINS         FOLDER_CUSTOMIZE _T("\\skins")
#define FOLDER_CUSTOMIZE_THEMES        FOLDER_CUSTOMIZE _T("\\themes")

#define TO_WIDE(x)     L ## x

#define FOLDERS_NAME_MAX_SIZE 64  //maximum name and section size

#define FF_UNICODE 0x00000001

#if defined (UNICODE)
#define FF_TCHAR	FF_UNICODE
#else
#define FF_TCHAR	0
#endif

typedef struct
{
	int    cbSize;                 //size of struct
	LPCSTR szSection;              //section name, if it doesn't exist it will be created otherwise it will just add this entry to it
	LPCSTR szName;                 //entry name - will be shown in options
	union {
		const char *szFormat;       //default string format. Fallback string in case there's no entry in the database for this folder. This should be the initial value for the path, users will be able to change it later.
		const wchar_t *szFormatW;   //String is dup()'d so you can free it later. If you set the unicode string don't forget to set the flag accordingly.
		const TCHAR *szFormatT;
	};
	DWORD flags;                   //FF_* flags
	union {
		const char *szUserName;     //for display purposes. if NULL, plugins gets it as the translated szName
		const wchar_t *szUserNameW; //String is dup()'d so you can free it later. If you set the unicode string don't forget to set the flag accordingly.
		const TCHAR *szUserNameT;
	};
}
FOLDERSDATA;

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
  it will either call strlen() or wcslen() to get the length of the string.
  Returns the size of the buffer.
  */
#define MS_FOLDERS_GET_SIZE "Folders/Get/PathSize"

typedef struct{
	int cbSize;
	int nMaxPathSize;                     //maximum size of buffer. This represents the number of characters that can be copied to it (so for unicode strings you don't send the number of bytes but the length of the string).
	union{
		char *szPath;                     //pointer to the buffer that receives the path without the last "\\"
		wchar_t *szPathW;                 //unicode version of the buffer.
		TCHAR *szPathT;
	};
	DWORD flags;                       //FF_* flags
} FOLDERSGETDATA;

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
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH)) return 0;
	FOLDERSDATA fd = { sizeof(fd) };
	fd.szSection = section;
	fd.szName = name;
	fd.szFormat = defaultPath;
	return (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);
}

#ifdef _UNICODE
__inline static HANDLE FoldersRegisterCustomPathW(const char *section, const char *name, const wchar_t *defaultPathW, const wchar_t *userNameW = NULL)
{
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH)) return 0;
	FOLDERSDATA fd = { sizeof(fd) };
	fd.szSection = section;
	fd.szName = name;
	fd.szFormatW = defaultPathW;
	fd.szUserNameW = userNameW;
	fd.flags = FF_UNICODE;
	return (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);
}
#endif //_UNICODE

__inline static INT_PTR FoldersGetCustomPath(HANDLE hFolderEntry, char *path, const int size, const char *notFound)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPath = path;
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
	fgd.szPathW = pathW;
	fgd.flags = FF_UNICODE;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		wchar_t buffer[MAX_PATH];
		PathToAbsoluteW(notFoundW, buffer);
		_tcsncpy_s(pathW, size, buffer, _TRUNCATE);
	}

	return res;
}
#endif //_UNICODE

__inline static INT_PTR FoldersGetCustomPathEx(HANDLE hFolderEntry, char *path, const int size, char *notFound, char *fileName)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPath = path;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		char buffer[MAX_PATH];
		PathToAbsolute(notFound, buffer);
		strncpy_s(path, size, buffer, _TRUNCATE);
	}

	if (path[0] != '\0')
		strncat(path, "\\", size);

	if (fileName)
		strncat(path, fileName, size);

	return res;
}

#ifdef _UNICODE
__inline static INT_PTR FoldersGetCustomPathExW(HANDLE hFolderEntry, wchar_t *pathW, const int size, wchar_t *notFoundW, wchar_t *fileNameW)
{
	FOLDERSGETDATA fgd = { sizeof(fgd) };
	fgd.nMaxPathSize = size;
	fgd.szPathW = pathW;
	fgd.flags = FF_UNICODE;
	INT_PTR res = CallService(MS_FOLDERS_GET_PATH, (WPARAM)hFolderEntry, (LPARAM)&fgd);
	if (res) {
		wchar_t buffer[MAX_PATH];
		PathToAbsoluteW(notFoundW, buffer);
		wcsncpy_s(pathW, size, buffer, _TRUNCATE);
	}

	if (pathW[0] != '\0')
		wcsncat(pathW, L"\\", size);

	if (fileNameW)
		wcsncat(pathW, fileNameW, size);

	return res;
}
#endif //_UNICODE

# ifdef _UNICODE
#  define FoldersGetCustomPathT FoldersGetCustomPathW
#  define FoldersGetCustomPathExT FoldersGetCustomPathExW
#  define FoldersRegisterCustomPathT FoldersRegisterCustomPathW
#else
#  define FoldersGetCustomPathT FoldersGetCustomPath
#  define FoldersGetCustomPathExT FoldersGetCustomPath
#  define FoldersRegisterCustomPathT FoldersRegisterCustomPath
#endif


#endif //M_CUSTOM_FOLDERS_H