/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __UTILS_H__
# define __UTILS_H__

bool LoadContactsFromMask(MCONTACT hContact, int mode, int count);
int  CheckForDuplicate(MCONTACT *contact_list, MCONTACT lparam);
void OffsetWindow(HWND parent, HWND hwnd, int dx, int dy);
void LoadSessionToCombobox(CCtrlCombo &combo, bool bUser);
void SavePosition(HWND hWnd, char *wndName);
void LoadPosition(HWND hWnd, char *wndName);
int  CheckContactVisibility(MCONTACT hContact);
void RenameUserDefSession(int ses_count, wchar_t *ptszNewName);

#endif // __UTILS_H__