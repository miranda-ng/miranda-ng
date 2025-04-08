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

#include "stdafx.h"

#define DEFAULT_SECTION "Unknown"

wchar_t szCurrentProfilePath[MAX_FOLDERS_PATH];
wchar_t szCurrentProfile[MAX_FOLDERS_PATH];
wchar_t szMirandaPath[MAX_FOLDERS_PATH];
wchar_t szUserDataPath[MAX_FOLDERS_PATH];

INT_PTR RegisterPathService(WPARAM, LPARAM lParam)
{
	FOLDERSDATA *data = (FOLDERSDATA*)lParam;
	if (data == nullptr)
		return NULL;

	CFolderItem *pNew;
	if (data->flags & FF_UNICODE)
		pNew = new CFolderItem(data->plugin, data->szSection, data->szName, data->szFormat.w);
	else
		pNew = new CFolderItem(data->plugin, data->szSection, data->szName, _A2T(data->szFormat.a));

	lstRegisteredFolders.insert(pNew);
	return (INT_PTR)pNew;
}

INT_PTR GetPathSizeService(WPARAM wParam, LPARAM lParam)
{
	CFolderItem *p = (CFolderItem*)wParam;
	size_t len = (lstRegisteredFolders.getIndex(p) != -1) ? p->Expand().GetLength() : 0;

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

	CMStringW buf(p->Expand());
	if (data->flags & FF_UNICODE)
		wcsncpy_s(data->szPath.w, data->nMaxPathSize, buf, _TRUNCATE);
	else
		strncpy_s(data->szPath.a, data->nMaxPathSize, _T2A(buf), _TRUNCATE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int InitServices()
{
	Profile_GetPathW(_countof(szCurrentProfilePath), szCurrentProfilePath);
	Profile_GetNameW(_countof(szCurrentProfile), szCurrentProfile);
	wchar_t *pos = wcsrchr(szCurrentProfile, '.'); if (pos) *pos = 0;

	GetModuleFileName(GetModuleHandleA("mir_app.mir"), szMirandaPath, _countof(szMirandaPath));
	pos = wcsrchr(szMirandaPath, '\\'); if (pos) *pos = 0;

	wchar_t *szTemp = Utils_ReplaceVarsW(L"%miranda_userdata%");
	mir_snwprintf(szUserDataPath, szTemp);
	mir_free(szTemp);

	CreateServiceFunction(MS_FOLDERS_GET_PATH, GetPathService);
	CreateServiceFunction(MS_FOLDERS_GET_SIZE, GetPathSizeService);
	CreateServiceFunction(MS_FOLDERS_REGISTER_PATH, RegisterPathService);
	return 0;
}
