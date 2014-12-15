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

#include "commonheaders.h"

#define DEFAULT_SECTION "Unknown"

TCHAR szCurrentProfilePath[MAX_FOLDERS_PATH];
TCHAR szCurrentProfile[MAX_FOLDERS_PATH];
TCHAR szMirandaPath[MAX_FOLDERS_PATH];
TCHAR szUserDataPath[MAX_FOLDERS_PATH];

INT_PTR ExpandPath(TCHAR *szResult, TCHAR *format, int size)
{
	szResult[0] = '\0';

	TCHAR *input = NULL;
	if (ServiceExists(MS_VARS_FORMATSTRING))
		input = variables_parse(format, NULL, NULL);

	if (input == NULL)
		input = mir_tstrdup(format);

	TCHAR *core_result = Utils_ReplaceVarsT(input);
	_tcsncpy(szResult, core_result, size);

	mir_free(core_result);

	StrReplace(szResult, PROFILE_PATHT, szCurrentProfilePath);
	StrReplace(szResult, CURRENT_PROFILET, szCurrentProfile);
	StrReplace(szResult, MIRANDA_PATHT, szMirandaPath);
	StrReplace(szResult, MIRANDA_USERDATAT, szUserDataPath);

	StrTrim(szResult, _T("\t \\"));

	mir_free(input);

	return _tcslen(szResult);
}

INT_PTR RegisterPathService(WPARAM, LPARAM lParam)
{
	FOLDERSDATA *data = (FOLDERSDATA*)lParam;
	if (data == NULL)
		return NULL;

	if (data->cbSize != sizeof(FOLDERSDATA))
		return NULL;

	CFolderItem *pNew;
	if (data->flags & FF_UNICODE)
		pNew = new CFolderItem(data->szSection, data->szName, data->szFormatW, data->szUserNameW);
	else
		pNew = new CFolderItem(data->szSection, data->szName, _A2T(data->szFormat), _A2T(data->szUserName));

	lstRegisteredFolders.insert(pNew);
	return (INT_PTR)pNew;
}

INT_PTR GetPathSizeService(WPARAM wParam, LPARAM lParam)
{
	size_t len;

	CFolderItem *p = (CFolderItem*)wParam;
	if (lstRegisteredFolders.getIndex(p) != -1) {
		TCHAR tmp[MAX_FOLDER_SIZE];
		p->Expand(tmp, SIZEOF(tmp));
		len = _tcslen(tmp);
	}
	else len = 0;

	if (lParam != NULL)
		*((size_t*)lParam) = len;

	return len;
}

INT_PTR GetPathService(WPARAM wParam, LPARAM lParam)
{
	CFolderItem *p = (CFolderItem*)wParam;
	if (lstRegisteredFolders.getIndex(p) == -1)
		return 1;

	FOLDERSGETDATA* data = (FOLDERSGETDATA*)lParam;
	if (data->cbSize != sizeof(FOLDERSGETDATA))
		return 1;

	if (data->flags & FF_UNICODE) {
		p->Expand(data->szPathT, data->nMaxPathSize);
		return 0;
	}

	TCHAR buf[MAX_FOLDER_SIZE];
	p->Expand(buf, SIZEOF(buf));
	strncpy(data->szPath, _T2A(buf), data->nMaxPathSize);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int InitServices()
{
	CallService(MS_DB_GETPROFILEPATHT, SIZEOF(szCurrentProfilePath), (LPARAM)szCurrentProfilePath);
	CallService(MS_DB_GETPROFILENAMET, SIZEOF(szCurrentProfile), (LPARAM)szCurrentProfile);
	TCHAR *pos = _tcsrchr(szCurrentProfile, '.'); if (pos) *pos = 0;

	GetModuleFileName(GetModuleHandle(NULL), szMirandaPath, SIZEOF(szMirandaPath));
	pos = _tcsrchr(szMirandaPath, '\\'); if (pos) *pos = 0;

	TCHAR *szTemp = Utils_ReplaceVarsT(_T("%miranda_userdata%"));
	mir_sntprintf(szUserDataPath, SIZEOF(szUserDataPath), szTemp);
	mir_free(szTemp);

	CreateServiceFunction(MS_FOLDERS_GET_PATH, GetPathService);
	CreateServiceFunction(MS_FOLDERS_GET_SIZE, GetPathSizeService);
	CreateServiceFunction(MS_FOLDERS_REGISTER_PATH, RegisterPathService);
	return 0;
}
