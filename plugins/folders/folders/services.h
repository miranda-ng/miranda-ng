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

#ifndef M_FOLDERS_PROVIDED_SERVICES_H
#define M_FOLDERS_PROVIDED_SERVICES_H

#include "commonheaders.h"
#include "m_folders.h"

#define MAX_FOLDERS_PATH 512

extern char szCurrentProfilePath[MAX_FOLDERS_PATH];
extern char szCurrentProfile[MAX_FOLDERS_PATH];
extern char szMirandaPath[MAX_FOLDERS_PATH];

extern wchar_t szCurrentProfilePathW[MAX_FOLDERS_PATH];
extern wchar_t szCurrentProfileW[MAX_FOLDERS_PATH];
extern wchar_t szMirandaPathW[MAX_FOLDERS_PATH];

int InitServices();
int DestroyServices();
INT_PTR ExpandPath(char *szResult, char *format, int size);
INT_PTR ExpandPathW(wchar_t *szResult, wchar_t *format, int size);
INT_PTR GetPath(int hRegisteredFolder, char *szResult, int size);

INT_PTR RegisterPathService(WPARAM wParam, LPARAM lParam);
INT_PTR GetPathSizeService(WPARAM wParam, LPARAM lParam);
INT_PTR GetPathService(WPARAM wParam, LPARAM lParam);
INT_PTR GetPathAllocService(WPARAM wParam, LPARAM lParam);
//int GetPathAppendService(WPARAM wParam, LPARAM lParam);
//int GetPathAllocAppendService(WPARAM wParam, LPARAM lParam);

#endif //M_FOLDERS_PROVIDED_SERVICES_H