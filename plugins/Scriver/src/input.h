/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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
#ifndef INPUT_H
#define INPUT_H

extern void InputAreaContextMenu(HWND hwnd, WPARAM wParam, LPARAM lParam, MCONTACT hContact);
extern void RegisterKeyBindings();
extern int InputAreaShortcuts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, CommonWindowData *windowData);
BOOL HandleLinkClick(HINSTANCE hInstance, HWND hwndDlg, HWND hwndFocus, ENLINK *lParam);

#endif
