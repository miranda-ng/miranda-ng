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

/* Backup to DB */
void CleanupRegTreeBackupSettings(void);
/* Class */
BOOL AddRegClass(const char *pszClassName,const TCHAR *pszTypeDescription,const TCHAR *pszIconLoc,const TCHAR *pszAppName,const TCHAR *pszRunCmd,const TCHAR *pszDdeCmd,const TCHAR *pszDdeApp,const TCHAR *pszDdeTopic,const TCHAR *pszVerbDesc,BOOL fBrowserAutoOpen,BOOL fUrlProto,BOOL fIsShortcut);
BOOL RemoveRegClass(const char *pszClassName);
BOOL IsRegClass(const char *pszClassName,const TCHAR *pszRunCmd);
HICON LoadRegClassSmallIcon(const char *pszClassName);
/* Extension */
BOOL AddRegFileExt(const char *pszFileExt,const char *pszClassName,const char *pszMimeType,BOOL fIsText);
void RemoveRegFileExt(const char *pszFileExt,const char *pszClassName);
BOOL IsRegFileExt(const char *pszFileExt,const char *pszClassName);
/* Mime Type */
BOOL AddRegMimeType(const char *pszMimeType,const char *pszFileExt,const TCHAR *pszDescription);
void RemoveRegMimeType(const char *pszMimeType,const char *pszFileExt);
/* Open-With App */
void AddRegOpenWith(const TCHAR *pszAppFileName,BOOL fAllowOpenWith,const TCHAR *pszAppName,const TCHAR *pszIconLoc,const TCHAR *pszRunCmd,const TCHAR *pszDdeCmd,const TCHAR *pszDdeApp,const TCHAR *pszDdeTopic);
void RemoveRegOpenWith(const TCHAR *pszAppFileName);
void AddRegOpenWithExtEntry(const TCHAR *pszAppFileName,const char *pszFileExt,const TCHAR *pszFileDesc);
void RemoveRegOpenWithExtEntry(const TCHAR *pszAppFileName,const char *pszFileExt);
/* Autostart */
BOOL AddRegRunEntry(const TCHAR *pszAppName,const TCHAR *pszRunCmd);
BOOL RemoveRegRunEntry(const TCHAR *pszAppName,const TCHAR *pszRunCmd);
BOOL IsRegRunEntry(const TCHAR *pszAppName,const TCHAR *pszRunCmd);

/* Strings */
char *MakeFileClassName(const char *pszFileExt);
char *MakeUrlClassName(const char *pszUrl);
TCHAR *MakeRunCommand(BOOL fMirExe,BOOL fFixedDbProfile);
TCHAR *MakeIconLocation(HMODULE hModule,WORD nIconResID);
TCHAR *MakeAppFileName(BOOL fMirExe);