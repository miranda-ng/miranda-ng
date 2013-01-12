/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* String Conv */
WCHAR* a2u(const char *pszAnsi,BOOL fMirCp);
char* u2a(const WCHAR *pszUnicode,BOOL fMirCp);
TCHAR* s2t(const void *pszStr,DWORD fUnicode,BOOL fMirCp);
void* t2s(const TCHAR *pszStr,DWORD fUnicode,BOOL fMirCp);

 #define t2a(s) u2a(s,FALSE)
 #define a2t(s) a2u(s,FALSE)


/* Database */
BOOL EnumDbPrefixSettings(const char *pszModule,const char *pszSettingPrefix,char ***pSettings,int *pnSettingsCount);

/* Error Output */
void ShowInfoMessage(BYTE flags,const char *pszTitle,const char *pszTextFmt,...);
char* GetWinErrorDescription(DWORD dwLastError);
