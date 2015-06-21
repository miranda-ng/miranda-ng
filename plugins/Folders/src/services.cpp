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
	if (data->cbSize != sizeof(FOLDERSGETDATA))
		return 1;

	CMString buf(p->Expand());
	if (data->flags & FF_UNICODE)
		_tcsncpy_s(data->szPathT, data->nMaxPathSize, buf, _TRUNCATE);
	else
		strncpy_s(data->szPath, data->nMaxPathSize, _T2A(buf), _TRUNCATE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int InitServices()
{
	CallService(MS_DB_GETPROFILEPATHT, _countof(szCurrentProfilePath), (LPARAM)szCurrentProfilePath);
	CallService(MS_DB_GETPROFILENAMET, _countof(szCurrentProfile), (LPARAM)szCurrentProfile);
	TCHAR *pos = _tcsrchr(szCurrentProfile, '.'); if (pos) *pos = 0;

	GetModuleFileName(GetModuleHandleA("mir_app.mir"), szMirandaPath, _countof(szMirandaPath));
	pos = _tcsrchr(szMirandaPath, '\\'); if (pos) *pos = 0;

	TCHAR *szTemp = Utils_ReplaceVarsT(_T("%miranda_userdata%"));
	mir_sntprintf(szUserDataPath, _countof(szUserDataPath), szTemp);
	mir_free(szTemp);

	CreateServiceFunction(MS_FOLDERS_GET_PATH, GetPathService);
	CreateServiceFunction(MS_FOLDERS_GET_SIZE, GetPathSizeService);
	CreateServiceFunction(MS_FOLDERS_REGISTER_PATH, RegisterPathService);
	return 0;
}
