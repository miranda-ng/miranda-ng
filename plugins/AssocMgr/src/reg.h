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

#pragma once

/* Backup to DB */
void CleanupRegTreeBackupSettings(void);
/* Class */
BOOL AddRegClass(const char *pszClassName, const wchar_t *pszTypeDescription, const wchar_t *pszIconLoc, const wchar_t *pszAppName, const wchar_t *pszRunCmd, const wchar_t *pszDdeCmd, const wchar_t *pszDdeApp, const wchar_t *pszDdeTopic, const wchar_t *pszVerbDesc, BOOL fBrowserAutoOpen, BOOL fUrlProto, BOOL fIsShortcut);
BOOL RemoveRegClass(const char *pszClassName);
BOOL IsRegClass(const char *pszClassName, const wchar_t *pszRunCmd);
HICON LoadRegClassSmallIcon(struct ASSOCDATA *pszClassName);
/* Extension */
BOOL AddRegFileExt(const char *pszFileExt, const char *pszClassName, const char *pszMimeType, BOOL fIsText);
void RemoveRegFileExt(const char *pszFileExt, const char *pszClassName);
BOOL IsRegFileExt(const char *pszFileExt, const char *pszClassName);
/* Mime Type */
BOOL AddRegMimeType(const char *pszMimeType, const char *pszFileExt);
void RemoveRegMimeType(const char *pszMimeType, const char *pszFileExt);
/* Open-With App */
void AddRegOpenWith(const wchar_t *pszAppFileName, BOOL fAllowOpenWith, const wchar_t *pszAppName, const wchar_t *pszIconLoc, const wchar_t *pszRunCmd, const wchar_t *pszDdeCmd, const wchar_t *pszDdeApp, const wchar_t *pszDdeTopic);
void RemoveRegOpenWith(const wchar_t *pszAppFileName);
void AddRegOpenWithExtEntry(const wchar_t *pszAppFileName, const char *pszFileExt, const wchar_t *pszFileDesc);
void RemoveRegOpenWithExtEntry(const wchar_t *pszAppFileName, const char *pszFileExt);
/* Autostart */
BOOL AddRegRunEntry(const wchar_t *pszAppName, const wchar_t *pszRunCmd);
BOOL RemoveRegRunEntry(const wchar_t *pszAppName, const wchar_t *pszRunCmd);
BOOL IsRegRunEntry(const wchar_t *pszAppName, const wchar_t *pszRunCmd);

/* Strings */
char* MakeFileClassName(const char *pszFileExt);
char* MakeUrlClassName(const char *pszUrl);

wchar_t* MakeRunCommand(BOOL fMirExe, BOOL fFixedDbProfile);
wchar_t* MakeIconLocation(HMODULE hModule, uint16_t nIconResID);
wchar_t* MakeAppFileName(BOOL fMirExe);
