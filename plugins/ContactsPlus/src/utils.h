// ---------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
//
// Copyright © 2002 Dominus Procellarum
// Copyright © 2004-2008 Joe Kucera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ---------------------------------------------------------------------------

#ifndef __UTILS_H
#define __UTILS_H

extern HINSTANCE hInst;

// utils.cpp
size_t __fastcall strlennull(const char *string);
int __fastcall strcmpnull(const char *str1, const char *str2);
void __fastcall strcpynull(char *str1, const char *str2);
char* __fastcall null_strdup(const char *string);

TCHAR* GetContactUID(MCONTACT hContact);

void DrawProtocolIcon(HWND hwndDlg, LPARAM lParam, MCONTACT hContact);
void UpdateDialogTitle(HWND hwndDlg, MCONTACT hContact, TCHAR *pszTitleStart);
void UpdateDialogAddButton(HWND hwndDlg, MCONTACT hContact);

HICON InitMButton(HWND hDlg, int idButton, LPCSTR szIcon, char* szTip);

void DialogAddContactExecute(HWND hwndDlg, MCONTACT hNewContact);

HICON LoadContactProtoIcon(MCONTACT hContact);

void EnableDlgItem(HWND hwndDlg, UINT control, int state);

#endif /* __UTILS_H */