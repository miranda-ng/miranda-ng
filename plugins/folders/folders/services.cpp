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

#include "services.h"

#define DEFAULT_SECTION "Unknown"

char szCurrentProfilePath[MAX_FOLDERS_PATH] = {0};
char szCurrentProfile[MAX_FOLDERS_PATH] = {0};
char szMirandaPath[MAX_FOLDERS_PATH] = {0};
char szUserDataPath[MAX_FOLDERS_PATH] = {0};

wchar_t szCurrentProfilePathW[MAX_FOLDERS_PATH] = {0};
wchar_t szCurrentProfileW[MAX_FOLDERS_PATH] = {0};
wchar_t szMirandaPathW[MAX_FOLDERS_PATH] = {0};
wchar_t szUserDataPathW[MAX_FOLDERS_PATH] = {0};

HANDLE hsFoldersGetPath;
HANDLE hsFoldersGetSize;
HANDLE hsFoldersGetPathAlloc;
HANDLE hsFoldersRegisterPath;

int InitServices()
{
	ServiceExists(MS_DB_GETPROFILEPATH_BASIC) ? CallService(MS_DB_GETPROFILEPATH_BASIC, sizeof(szCurrentProfilePath), (LPARAM) szCurrentProfilePath) : CallService(MS_DB_GETPROFILEPATH, sizeof(szCurrentProfilePath), (LPARAM) szCurrentProfilePath);
	CallService(MS_DB_GETPROFILENAME, sizeof(szCurrentProfile), (LPARAM) szCurrentProfile);
	char *pos = strrchr(szCurrentProfile, '.');
	szCurrentProfile[pos - szCurrentProfile] = '\0'; //remove the extension (.dat)
	GetModuleFileNameA(GetModuleHandleA(NULL), szMirandaPath, sizeof(szMirandaPath));
	pos = strrchr(szMirandaPath, '\\');
	szMirandaPath[pos - szMirandaPath] = '\0'; //remove '\miranda32.exe'

	char *szTemp = Utils_ReplaceVars("%miranda_userdata%");
	mir_snprintf(szUserDataPath, MAX_FOLDERS_PATH, szTemp);
	mir_free(szTemp);

	MultiByteToWideChar(CP_ACP, 0, szCurrentProfilePath, -1, szCurrentProfilePathW, MAX_FOLDERS_PATH);
	MultiByteToWideChar(CP_ACP, 0, szCurrentProfile, -1, szCurrentProfileW, MAX_FOLDERS_PATH);
	MultiByteToWideChar(CP_ACP, 0, szMirandaPath, -1, szMirandaPathW, MAX_FOLDERS_PATH);
	MultiByteToWideChar(CP_ACP, 0, szUserDataPath, -1, szUserDataPathW, MAX_FOLDERS_PATH);

	hsFoldersGetPath = CreateServiceFunction(MS_FOLDERS_GET_PATH, GetPathService);
//	CreateServiceFunction(MS_FOLDERS_GET_PATH_APPEND, GetPathAppendService);
	hsFoldersGetSize = CreateServiceFunction(MS_FOLDERS_GET_SIZE, GetPathSizeService);
	hsFoldersGetPathAlloc = CreateServiceFunction(MS_FOLDERS_GET_PATH_ALLOC, GetPathAllocService);
//	CreateServiceFunction(MS_FOLDERS_GET_PATH_ALLOC_APPEND, GetPathAllocAppendService);
	hsFoldersRegisterPath = CreateServiceFunction(MS_FOLDERS_REGISTER_PATH, RegisterPathService);

	return 0;
}

int DestroyServices()
{
	DestroyServiceFunction(hsFoldersGetPath);
//	DestroyServiceFunction(MS_FOLDERS_GET_PATH_APPEND);
	DestroyServiceFunction(hsFoldersGetSize);
	DestroyServiceFunction(hsFoldersGetPathAlloc);
//	DestroyServiceFunction(MS_FOLDERS_GET_PATH_ALLOC_APPEND);
	DestroyServiceFunction(hsFoldersRegisterPath);
	return 0;
}

INT_PTR ExpandPath(char *szResult, char *format, int size)
{
	szResult[0] = '\0';
	char *input = NULL;

	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
		TCHAR* tmp_format = mir_a2t(format);
		TCHAR *vars_result_tmp = variables_parse(tmp_format, NULL, NULL);
		mir_free(tmp_format);
		char *vars_result = mir_t2a(vars_result_tmp);

		if (vars_result != NULL)
		{
			input = mir_strdup(vars_result);

			variables_free(vars_result_tmp);
		}
		mir_free(vars_result);
	}

	if (input == NULL)
	{
		input = mir_strdup(format);
	}

	char *core_result = Utils_ReplaceVars(input);
	strncpy(szResult, core_result, size);
	
	mir_free(core_result);

	StrReplace(szResult, PROFILE_PATH, szCurrentProfilePath);
	StrReplace(szResult, CURRENT_PROFILE, szCurrentProfile);
	StrReplace(szResult, MIRANDA_PATH, szMirandaPath);
	StrReplace(szResult, MIRANDA_USERDATA, szUserDataPath);

	StrTrim(szResult, "\t \\");

	mir_free(input);

	return strlen(szResult);
}

INT_PTR ExpandPathW(wchar_t *szResult, wchar_t *format, int size)
{
	szResult[0] = '\0';
	wchar_t *input = NULL;
	
	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
		TCHAR* tmp_format = mir_u2t(format);
		TCHAR *vars_result_tmp = variables_parse(tmp_format, NULL, NULL);
		mir_free(tmp_format);
		wchar_t *vars_result = mir_t2u(vars_result_tmp);
	
		if (vars_result != NULL)
		{
			input = mir_wstrdup(vars_result);

			variables_free(vars_result_tmp);
		}
		mir_free(vars_result);
	}

	if (input == NULL)
	{
		input = mir_wstrdup(format);
	}

	wchar_t *core_result = Utils_ReplaceVarsW(input);
	if (core_result)
	{
		wcsncpy(szResult, core_result, size);
	}
	else {
		wcsncpy(szResult, input, size);
	}

	mir_free(core_result);

	StrReplace(szResult, PROFILE_PATHW, szCurrentProfilePathW);
	StrReplace(szResult, CURRENT_PROFILEW, szCurrentProfileW);
	StrReplace(szResult, MIRANDA_PATHW, szMirandaPathW);
	StrReplace(szResult, MIRANDA_USERDATAW, szUserDataPathW);

	StrTrim(szResult, L"\t \\");

	mir_free(input);

	return wcslen(szResult);
}

INT_PTR GetPath(int hRegisteredFolder, char *szResult, int size)
{
	return lstRegisteredFolders.Expand(hRegisteredFolder, szResult, size);
	//return 0;
}

INT_PTR RegisterPathService(WPARAM wParam, LPARAM lParam)
{
	FOLDERSDATA tmp = *(FOLDERSDATA *) lParam;
	int res = 0;
	if (tmp.cbSize == sizeof(FOLDERSDATA))
		{
			res = lstRegisteredFolders.Add(tmp); //returns 1..n or 0 on error
		}
	return res;
}

INT_PTR GetPathSizeService(WPARAM wParam, LPARAM lParam)
{
	char tmp[MAX_FOLDER_SIZE * 4]; //dumb
	int res = lstRegisteredFolders.Expand(wParam, tmp, sizeof(tmp));
	size_t len = 0;
	if ((lstRegisteredFolders.Get(wParam)) && (lstRegisteredFolders.Get(wParam)->IsUnicode()))
		{
			len = wcslen((wchar_t *)tmp);
		}
		else{
			len = strlen(tmp);
		}
	if (lParam != NULL)
		{
			*((size_t *) lParam) = len;
		}
	return len;
}

INT_PTR GetPathService(WPARAM wParam, LPARAM lParam)
{
	FOLDERSGETDATA data = *(FOLDERSGETDATA *) lParam;
	size_t res = 1;
	if (data.cbSize == sizeof(FOLDERSGETDATA))
		{
			res = GetPath(wParam, data.szPath, data.nMaxPathSize); //dumb ...
		}
	return res;
}

INT_PTR GetPathAllocService(WPARAM wParam, LPARAM lParam)
{
	int size;
	int res = 1;
	FOLDERSGETALLOCDATA data = *(FOLDERSGETALLOCDATA *) lParam;
	if (data.cbSize == sizeof(FOLDERSGETALLOCDATA))
		{
			size = GetPathSizeService(wParam, (LPARAM) &size);
			char **buffer = data.szPath;
			*buffer = (char *) mir_alloc(size + 1);
			res = GetPath(wParam, *buffer, size);
		}
	return res;
}
/*
int GetPathAppendService(WPARAM wParam, LPARAM lParam)
{
	FOLDERSAPPENDDATA data = *(FOLDERSAPPENDDATA *) wParam;
	int res = GetPathService(data.hRegisteredPath, lParam);
	strcat((char *) lParam, data.szAppendData);
	return res;
}
*/
/*
int GetPathAllocAppendService(WPARAM wParam, LPARAM lParam)
{
	FOLDERSAPPENDDATA data = *(FOLDERSAPPENDDATA *) wParam;
	int res = GetPathAllocService(data.hRegisteredPath, lParam);
	strcat(*(char **) lParam, data.szAppendData);
	return res;
}
*/