/*
Scriver

Copyright 2000-2009 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#ifndef _STATUS_ICON_INC
#define _STATUS_ICON_INC

#include <windows.h>

int InitStatusIcons();
int DeinitStatusIcons();

INT_PTR  GetStatusIconsCount(HANDLE hContact);
void DrawStatusIcons(HANDLE hContact, HDC hdc, RECT r, int gap);
void CheckStatusIconClick(HANDLE hContact, HWND hwndFrom, POINT pt, RECT rc, int gap, int flags);
INT_PTR AddStickyStatusIcon(WPARAM wParam, LPARAM lParam);
INT_PTR ModifyStatusIcon(WPARAM wParam, LPARAM lParam);
#endif
