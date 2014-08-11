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

#define TO_WIDE(x)     L ## x

#define PROFILE_PATHW    L"%profile_path%"
#define CURRENT_PROFILEW L"%current_profile%"
#define MIRANDA_PATHW    L"%miranda_path%"

#define FOLDER_AVATARS                 PROFILE_PATH "\\" CURRENT_PROFILE "\\avatars"
#define FOLDER_VCARDS                  PROFILE_PATH "\\" CURRENT_PROFILE "\\vcards"
#define FOLDER_LOGS                    PROFILE_PATH "\\" CURRENT_PROFILE "\\logs"
#define FOLDER_RECEIVED_FILES          PROFILE_PATH "\\" CURRENT_PROFILE "\\received files"
#define FOLDER_DOCS                    MIRANDA_PATH "\\" "docs"

#define FOLDER_CONFIG                  PLUGINS_PATH "\\" "config"

#define FOLDER_SCRIPTS                 MIRANDA_PATH "\\" "scripts"

#define FOLDER_UPDATES                 MIRANDA_PATH "\\" "updates"

#define FOLDER_CUSTOMIZE               MIRANDA_PATH "\\" "customize"
#define FOLDER_CUSTOMIZE_SOUNDS        FOLDER_CUSTOMIZE "\\sounds"
#define FOLDER_CUSTOMIZE_ICONS         FOLDER_CUSTOMIZE "\\icons"
#define FOLDER_CUSTOMIZE_SMILEYS       FOLDER_CUSTOMIZE "\\smileys"
#define FOLDER_CUSTOMIZE_SKINS         FOLDER_CUSTOMIZE "\\skins"
#define FOLDER_CUSTOMIZE_THEMES        FOLDER_CUSTOMIZE "\\themes"


#define FOLDERS_NAME_MAX_SIZE 64  //maximum name and section size

#define FF_UNICODE 0x00000001

typedef struct{
  int cbSize;                                  //size of struct
  char szSection[FOLDERS_NAME_MAX_SIZE];       //section name, if it doesn't exist it will be created otherwise it will just add this entry to it
  char szName[FOLDERS_NAME_MAX_SIZE];          //entry name - will be shown in options
  union{
    const char *szFormat;                      //default string format. Fallback string in case there's no entry in the database for this folder. This should be the initial value for the path, users will be able to change it later.
    const wchar_t *szFormatW;                  //String is dup()'d so you can free it later. If you set the unicode string don't forget to set the flag accordingly.
    const TCHAR *szFormatT;
  };
  DWORD flags;                                 //FF_* flags
} FOLDERSDATA;

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
} FOLDERSGETDATA;

/*Folders/Get/Path service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (FOLDERSGETDATA *) pointer to a FOLDERSGETDATA that has all the relevant fields filled.
  Should return 0 on success, or nonzero otherwise.
*/
#define MS_FOLDERS_GET_PATH "Folders/Get/Path"

typedef struct{
  int cbSize;
  union{
    char **szPath;                    //address of a string variable (char *) or (wchar_t*) where the path should be stored (the last \ won't be copied).
    wchar_t **szPathW;                //unicode version of string.
    TCHAR **szPathT;
	};
} FOLDERSGETALLOCDATA;

/*Folders/GetRelativePath/Alloc service
  wParam - (WPARAM) (int) - Handle to registered path
  lParam - (LPARAM) (FOLDERSALLOCDATA *) data
  This service is the same as MS_FOLDERS_GET_PATH with the difference that this service
  allocates the needed space for the buffer. It uses miranda's memory functions for that and you need
  to use those to free the resulting buffer.
  Should return 0 on success, or nonzero otherwise. Currently it only returns 0.
*/
#define MS_FOLDERS_GET_PATH_ALLOC "Folders/Get/Path/Alloc"


/*Folders/On/Path/Changed
	wParam - (WPARAM) 0
	lParam - (LPARAM) 0
	Triggered when the folders change, you should reget the paths you registered.
*/
#define ME_FOLDERS_PATH_CHANGED "Folders/On/Path/Changed"

#ifndef FOLDERS_NO_HELPER_FUNCTIONS
//#include "../../../include/newpluginapi.h"

__inline static int FoldersRegisterCustomPath(const char *section, const char *name, const char *defaultPath)
{
	FOLDERSDATA fd = {0};
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH)) return 1;
	fd.cbSize = sizeof(FOLDERSDATA);
	strncpy(fd.szSection, section, FOLDERS_NAME_MAX_SIZE);
	fd.szSection[FOLDERS_NAME_MAX_SIZE - 1] = '\0';
	strncpy(fd.szName, name, FOLDERS_NAME_MAX_SIZE);
	fd.szName[FOLDERS_NAME_MAX_SIZE - 1] = '\0';
	fd.szFormat = defaultPath;
	return CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &fd);
}

__inline static int FoldersRegisterCustomPathW(const char *section, const char *name, const wchar_t *defaultPathW)
{
	FOLDERSDATA fd = {0};
	if (!ServiceExists(MS_FOLDERS_REGISTER_PATH)) return 1;
	fd.cbSize = sizeof(FOLDERSDATA);
	strncpy(fd.szSection, section, FOLDERS_NAME_MAX_SIZE);
	fd.szSection[FOLDERS_NAME_MAX_SIZE - 1] = '\0'; //make sure it's NULL terminated
	strncpy(fd.szName, name, FOLDERS_NAME_MAX_SIZE);
	fd.szName[FOLDERS_NAME_MAX_SIZE - 1] = '\0'; //make sure it's NULL terminated
	fd.szFormatW = defaultPathW;
	fd.flags = FF_UNICODE;
	return CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &fd);
}

__inline static int FoldersGetCustomPath(HANDLE hFolderEntry, char *path, const int size, char *notFound)
{
	FOLDERSGETDATA fgd = {0};
	int res;
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = size;
	fgd.szPath = path;
	res = CallService(MS_FOLDERS_GET_PATH, (WPARAM) hFolderEntry, (LPARAM) &fgd);
	if (res)
		{
			strncpy(path, notFound, size);
			path[size - 1] = '\0'; //make sure it's NULL terminated
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

# ifdef _UNICODE
#  define FoldersGetCustomPathT FoldersGetCustomPathW
#  define FoldersRegisterCustomPathT FoldersRegisterCustomPathW
#else
#  define FoldersGetCustomPathT FoldersGetCustomPath
#  define FoldersRegisterCustomPathT FoldersRegisterCustomPath
#endif

#endif

#endif //M_CUSTOM_FOLDERS_H