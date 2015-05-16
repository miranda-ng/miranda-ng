
//This file is part of Msg_Export a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#ifndef MSG_EXP_UTILS_H
#define MSG_EXP_UTILS_H

#define tstring wstring
#define _DBGetString _DBGetStringW

enum ENDialogAction
{
	eDAPromptUser,
	eDAAutomatic,
	eDANothing
};

extern ENDialogAction enRenameAction;
extern ENDialogAction enDeleteAction;

extern int nMaxLineWidth;
extern tstring sExportDir;
extern tstring sDefaultFile;
extern tstring sTimeFormat;
extern map<tstring, string::size_type, less<tstring> > clFileTo1ColWidth;

extern bool bAppendNewLine;
extern bool bUseUtf8InNewFiles;
extern bool bUseLessAndGreaterInExport;

extern bool bReplaceHistory;

tstring sGetErrorString(DWORD dwError);
tstring sGetErrorString();
void DisplayLastError(const TCHAR *pszError);

TCHAR *CheckedTranslate(const TCHAR *szEng, int nFormatCount = -1);

void SaveSettings();
void ShowDebugInfo();

int nExportEvent(WPARAM wparam, LPARAM lparam);
int nContactDeleted(WPARAM wparam, LPARAM lparam);

const TCHAR *NickFromHandle(MCONTACT hContact);

tstring __inline _DBGetStringW(MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *pszError);
string __inline _DBGetStringA(MCONTACT hContact, const char *szModule, const char *szSetting, const char *pszError);

void ReplaceAll(tstring &sSrc, const TCHAR *pszReplace, const tstring &sNew);
void ReplaceAll(tstring &sSrc, const TCHAR *pszReplace, const TCHAR *pszNew);

void UpdateFileToColWidth();

bool bReadMirandaDirAndPath();
tstring GetFilePathFromUser(MCONTACT hContact);

void ReplaceDefines(MCONTACT hContact, tstring & sTarget);
void ReplaceTimeVariables(tstring &sRet);

bool bCreatePathToFile(tstring sFilePath);

bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const char *pszSrc, bool bUtf8File);
bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const TCHAR *pszSrc, bool bUtf8File);
bool bWriteNewLine(HANDLE hFile, DWORD dwIndent);
bool bIsUtf8Header(BYTE * pucByteOrder);

#endif
