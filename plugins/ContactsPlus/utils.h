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
extern PLUGINLINK *pluginLink;

// Compatibility functions
int SRCCallProtoService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam);
int SRCCallContactService(HANDLE hContact, const char *szProtoService, WPARAM wParam, LPARAM lParam);

// utils.cpp
void __fastcall SAFE_FREE(void** p);
size_t __fastcall strlennull(const char *string);
int __fastcall strcmpnull(const char *str1, const char *str2);
char* __fastcall null_strdup(const char *string);

char *GetContactProto(HANDLE hContact);
char *GetContactUID(HANDLE hContact, int bTchar);
TCHAR *GetContactDisplayNameT(HANDLE hContact);
TCHAR* MirandaStatusToStringT(int mirandaStatus);

HANDLE __fastcall SRCFindFirstContact();
HANDLE __fastcall SRCFindNextContact(HANDLE hContact);
int DBGetContactSettingT(HANDLE hContact, const char *szModule, const char* szSetting, DBVARIANT *dbv);
TCHAR* DBGetContactSettingStringT(HANDLE hContact, const char *szModule,const char* szSetting, TCHAR* szDef);
int DBWriteContactSettingStringT(HANDLE hContact, const char *szModule, const char* szSetting, TCHAR* szValue);

void DrawProtocolIcon(HWND hwndDlg, LPARAM lParam, HANDLE hContact);
void UpdateDialogTitle(HWND hwndDlg, HANDLE hContact, char* pszTitleStart);
void UpdateDialogAddButton(HWND hwndDlg, HANDLE hContact);

HICON InitMButton(HWND hDlg, int idButton, LPCSTR szIcon, char* szTip);

void DialogAddContactExecute(HWND hwndDlg, HANDLE hNewContact);

HICON LoadContactProtoIcon(HANDLE hContact);

void EnableDlgItem(HWND hwndDlg, UINT control, int state);

/// Unicode 2 in 1 Framework
size_t __fastcall strlenT(const TCHAR *string);
TCHAR* __fastcall strdupT(const TCHAR *string);
int __fastcall strcmpT(const TCHAR *string1, const TCHAR *string2);
TCHAR* __fastcall strcpyT(TCHAR* dest, const TCHAR* src);
TCHAR* __fastcall strncpyT(TCHAR* dest, const TCHAR* src, size_t len);
TCHAR* __fastcall strcatT(TCHAR* dest, const TCHAR* src);
int _snprintfT(TCHAR *buffer, size_t count, const char* fmt, ...);

LRESULT SendMessageT(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
TCHAR* GetWindowTextT(HWND hWnd);
BOOL SetWindowTextT(HWND hWnd, TCHAR* lpString);
TCHAR* GetDlgItemTextT(HWND hDlg, int nIDDlgItem);
BOOL SetDlgItemTextT(HWND hDlg, int nIDDlgItem, TCHAR* lpString);
HWND CreateDialogParamT(HINSTANCE hInstance, const char* szTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
int ListView_InsertColumnT(HWND hwnd, int iCol, const LPLVCOLUMN pcol);
void ListView_SetItemTextT(HWND hwnd, int i, int iSubItem, TCHAR* pszText);

TCHAR* __fastcall SRCTranslateT(const char* src, const WCHAR* unibuf);

void InitI18N(void);
TCHAR* ansi_to_tchar(const char* string, int codepage = CP_ACP);
char* tchar_to_ansi(const TCHAR* src);
TCHAR* utf8_to_tchar(const unsigned char* utf);
unsigned char *make_utf8_string(const wchar_t *unicode);


#endif /* __UTILS_H */