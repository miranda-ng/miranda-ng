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

void SetInSessionOrder(MCONTACT hContact,int mode,int count,unsigned int ordernum);
void AddInSessionOrder(MCONTACT hContact,int mode,int ordernum,int writemode);
int GetInSessionOrder(MCONTACT hContact,int mode,int count);
void AddSessionMark(MCONTACT hContact,int mode,char bit);
void RemoveSessionMark(MCONTACT hContact,int mode,int marknum);
void SetSessionMark(MCONTACT hContact,int mode,char bit,unsigned int marknum);
bool LoadContactsFromMask(MCONTACT hContact, int mode, int count);
int AddToCurSession(MCONTACT hContact, LPARAM lparam);
int DelFromCurSession(MCONTACT hContact,LPARAM lparam);
int CheckForDuplicate(MCONTACT contact_list[], MCONTACT lparam);
BOOL ResaveSettings(char* szName,int iFirst,int iLimit,TCHAR* pszPrevSetting);
void OffsetWindow(HWND parent, HWND hwnd, int dx, int dy);
int LoadSessionToCombobox (HWND hdlg,BOOL mode,int iLimit,char* pszSetting,int iFirstNum);
int MarkUserDefSession(int ses_count,BYTE bCheck);
BYTE IsMarkedUserDefSession(int ses_count);
void SavePosition(HWND hWnd, char *wndName);
void LoadPosition(HWND hWnd, char *wndName);
int CheckContactVisibility(MCONTACT hContact);
void RenameUserDefSession(int ses_count,TCHAR* ptszNewName);
int FillFavoritesMenu (HMENU hMenu,int iLimit);

#endif // __UTILS_H__