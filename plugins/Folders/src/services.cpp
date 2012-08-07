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

TCHAR szCurrentProfilePath[MAX_FOLDERS_PATH];
TCHAR szCurrentProfile[MAX_FOLDERS_PATH];
TCHAR szMirandaPath[MAX_FOLDERS_PATH];
TCHAR szUserDataPath[MAX_FOLDERS_PATH];

HANDLE hsFoldersGetPath;
HANDLE hsFoldersGetSize;
HANDLE hsFoldersGetPathAlloc;
HANDLE hsFoldersRegisterPath;

INT_PTR ExpandPath(TCHAR *szResult, TCHAR *format, int size)
{
	szResult[0] = '\0';

	TCHAR *input = NULL;
	if ( ServiceExists(MS_VARS_FORMATSTRING))
		input = variables_parse(format, NULL, NULL);

	if (input == NULL)
		input = mir_tstrdup(format);

	TCHAR *core_result = Utils_ReplaceVarsT(input);
	_tcsncpy(szResult, core_result, size);
	
	mir_free(core_result);

	StrReplace(szResult, PROFILE_PATHT,     szCurrentProfilePath);
	StrReplace(szResult, CURRENT_PROFILET,  szCurrentProfile);
	StrReplace(szResult, MIRANDA_PATHT,     szMirandaPath);
	StrReplace(szResult, MIRANDA_USERDATAT, szUserDataPath);

	StrTrim(szResult, _T("\t \\"));

	mir_free(input);

	return _tcslen(szResult);
}

INT_PTR RegisterPathService(WPARAM wParam, LPARAM lParam)
{
	FOLDERSDATA *tmp = (FOLDERSDATA *) lParam;
	if (tmp == NULL || tmp->cbSize != sizeof(FOLDERSDATA))
		return NULL;

	return lstRegisteredFolders.Add(tmp); //returns 1..n or 0 on error
}

INT_PTR GetPathSizeService(WPARAM wParam, LPARAM lParam)
{
	TCHAR tmp[MAX_FOLDER_SIZE];
	int res = lstRegisteredFolders.Expand(wParam, tmp, SIZEOF(tmp));
	size_t len = _tcslen(tmp);

	if (lParam != NULL)
		*((size_t *) lParam) = len;

	return len;
}

INT_PTR GetPathService(WPARAM wParam, LPARAM lParam)
{
	FOLDERSGETDATA* data = (FOLDERSGETDATA *) lParam;
	if (data->cbSize != sizeof(FOLDERSGETDATA))
		return 1;

	if (data->flags & FF_UNICODE)
		return lstRegisteredFolders.Expand(wParam, data->szPathT, data->nMaxPathSize);

	TCHAR buf[MAX_FOLDER_SIZE];
	if ( lstRegisteredFolders.Expand(wParam, buf, MAX_FOLDER_SIZE))
		return 1;

	strncpy(data->szPath, _T2A(buf), data->nMaxPathSize);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int InitServices()
{
	CallService(MS_DB_GETPROFILEPATHT, SIZEOF(szCurrentProfilePath), (LPARAM)szCurrentProfilePath);
	CallService(MS_DB_GETPROFILENAMET, SIZEOF(szCurrentProfile), (LPARAM)szCurrentProfile);
	TCHAR *pos = _tcsrchr(szCurrentProfile, '.'); if (pos) *pos = 0;

	GetModuleFileName( GetModuleHandle(NULL), szMirandaPath, SIZEOF(szMirandaPath));
	pos = _tcsrchr(szMirandaPath, '\\'); if (pos) *pos = 0;

	TCHAR *szTemp = Utils_ReplaceVarsT( _T("%miranda_userdata%"));
	mir_sntprintf(szUserDataPath, MAX_FOLDERS_PATH, szTemp);
	mir_free(szTemp);

	hsFoldersGetPath = CreateServiceFunction(MS_FOLDERS_GET_PATH, GetPathService);
	hsFoldersGetSize = CreateServiceFunction(MS_FOLDERS_GET_SIZE, GetPathSizeService);
	hsFoldersRegisterPath = CreateServiceFunction(MS_FOLDERS_REGISTER_PATH, RegisterPathService);
	return 0;
}

int DestroyServices()
{
	DestroyServiceFunction(hsFoldersGetPath);
	DestroyServiceFunction(hsFoldersGetSize);
	DestroyServiceFunction(hsFoldersRegisterPath);
	return 0;
}
