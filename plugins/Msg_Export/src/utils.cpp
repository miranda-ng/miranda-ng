
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

#include "stdafx.h"

// Default error string used upon errors 
const wchar_t *pszGroupError = LPGENW("No_Group");
const wchar_t *pszDbPathError = L".";

// Replacement for chareteres not alowed in file names.
const wchar_t cBadCharReplace = '_';

// sTimeFormat 
tstring sTimeFormat;

// path from options dialog
tstring sExportDir;

// The default filename. Used if no other file name is specifyed in DB.
tstring sDefaultFile;

// path used then %dbpath% is used in export file path
tstring sDBPath = pszDbPathError;

// path to miranda exe file used when to avoid relative paths
tstring sMirandaPath = pszDbPathError;

// Used to store the width of the user name for a file.
// if a file contains events from many users the one user name
// may be shorter. so to make all event have the same first
// column width this map contains the largest user name
map<tstring, string::size_type, less<tstring> > clFileTo1ColWidth;

// default line width
int nMaxLineWidth = 80;

const wchar_t *pszReplaceList[] =
{
	L"%FirstName%",
	L"%LastName%",
	L"%e-mail%",
	L"%Nick%",
	L"%City%",
	L"%State%",
	L"%Phone%",
	L"%Homepage%",
	L"%About%"
};
const char *pszReplaceListA[] =
{
	"FirstName",
	"LastName",
	"e-mail",
	"Nick",
	"City",
	"State",
	"Phone",
	"Homepage",
	"About"
};

// Alowes this plugin to replace the history function of miranda !!
bool bReplaceHistory = false;

// This enum define the actions which MsgExport must take when a File is renamed
ENDialogAction enRenameAction = eDAPromptUser;

// This enum define the actions which MsgExport must take when a user is delete
ENDialogAction enDeleteAction = eDAPromptUser;

// If true MsgExport will use << and >> insted of the nick in the exported format
bool bUseLessAndGreaterInExport = false;

bool bAppendNewLine = true;
bool bUseUtf8InNewFiles = true;

const char szUtf8ByteOrderHeader[] = "\xEF\xBB\xBF";
bool bIsUtf8Header(BYTE * pucByteOrder)
{
	return memcmp(pucByteOrder, szUtf8ByteOrderHeader, 3) == 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ShowDebugInfo
// Type            : Global
// Parameters      : None
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 021228, 28 December 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ShowDebugInfo()
{
	tstring sDebug = L"Debug information\r\nsDBPath :";
	sDebug += sDBPath;
	sDebug += L"\r\nsMirandaPath :";
	sDebug += sMirandaPath;
	sDebug += L"\r\nsDefaultFile :";
	sDebug += sDefaultFile;

	sDebug += L"\r\nGetFilePathFromUser(NULL) :";
	sDebug += GetFilePathFromUser(NULL);

	MessageBox(NULL, sDebug.c_str(), MSG_BOX_TITEL, MB_OK);
}


/////////////////////////////////////////////////////////////////////
// Member Function : nGetFormatCount
// Type            : Global
// Parameters      : pszToCheck - ?
// Returns         : int
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 030107, 07 January 2003
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int nGetFormatCount(const wchar_t *pszToCheck)
{
	if (!pszToCheck || pszToCheck[0] == 0)
		return 0;

	int nCount = 0;
	for (; pszToCheck[1] != 0; pszToCheck++) {
		if (pszToCheck[0] == '%' && pszToCheck[1] != '%')
			nCount++;
	}
	return nCount;
}

/////////////////////////////////////////////////////////////////////
// Member Function : sGetErrorString
// Type            : Global
// Parameters      : dwError - ?
// Returns         : string
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 021012, 12 October 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

tstring sGetErrorString(DWORD dwError)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	tstring ret = (LPCTSTR)lpMsgBuf;
	ReplaceAll(ret, L"\r", L" ");
	ReplaceAll(ret, L"\n", L" ");
	ReplaceAll(ret, L"  ", L" ");

	// Free the buffer.
	LocalFree(lpMsgBuf);
	return ret;
}

tstring sGetErrorString()
{
	return sGetErrorString(GetLastError());
}

void DisplayLastError(const wchar_t *pszError)
{
	tstring sError = pszError;
	DWORD error = GetLastError();

	wchar_t szTemp[50];
	mir_snwprintf(szTemp, L"\r\nErrorCode: %d\r\n", error);
	sError += szTemp;
	sError += sGetErrorString(error);
	MessageBox(NULL, sError.c_str(), MSG_BOX_TITEL, MB_OK);
}

/////////////////////////////////////////////////////////////////////
// Member Function : _DBGetString
// Type            : Global
// Parameters      : hContact  - ?
//                   szModule  - ?
//                   szSetting - ?
//                   pszError  - ?
// Returns         : string
// Description     : Reads a string from the database 
//                   Just like those in database.h
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

tstring _DBGetStringW(MCONTACT hContact, const char *szModule, const char *szSetting, const wchar_t *pszError)
{
	tstring ret;
	DBVARIANT dbv = { 0 };
	//db_get
	if (!db_get_ws(hContact, szModule, szSetting, &dbv)) {
		if (dbv.type != DBVT_WCHAR) {
			MessageBox(NULL, TranslateT("Database: Attempt to get wrong type of value, string"), MSG_BOX_TITEL, MB_OK);
			ret = pszError;
		}
		else ret = (wchar_t*)dbv.pszVal;
	}
	else ret = pszError;
	
	db_free(&dbv);
	return ret;
}

string _DBGetStringA(MCONTACT hContact, const char *szModule, const char *szSetting, const char *pszError)
{
	string ret;
	DBVARIANT dbv = { 0 };
	if (!db_get(hContact, szModule, szSetting, &dbv)) {
		if (dbv.type != DBVT_ASCIIZ) {
			MessageBox(NULL, TranslateT("Database: Attempt to get wrong type of value, string"), MSG_BOX_TITEL, MB_OK);
			ret = pszError;
		}
		else ret = dbv.pszVal;
	}
	else ret = pszError;
	
	db_free(&dbv);
	return ret;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceAll
// Type            : Global
// Parameters      : sSrc       - string to replace in
//                   pszReplace - what to replace
//                   sNew       - the string to insert insted of pszReplace
// Returns         : void
// Description     : will replace all acurances of a string with another string
//                   used to replace %user%, and other user 
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ReplaceAll(tstring &sSrc, const wchar_t *pszReplace, const tstring &sNew)
{
	string::size_type nCur = 0;
	while ((nCur = sSrc.find(pszReplace, nCur)) != sSrc.npos) {
		sSrc.replace(nCur, mir_wstrlen(pszReplace), sNew);
		nCur += sNew.size();
	}
}

void ReplaceAll(tstring &sSrc, const wchar_t *pszReplace, const wchar_t *pszNew)
{
	tstring sNew = pszNew;
	ReplaceAll(sSrc, pszReplace, sNew);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bCreatePathToFile
// Type            : Global
// Parameters      : sFilePath - File name to create path to ( file name may be empty ( i.e. c:\Folder\ )
// Returns         : Returns true if the path is created or already exists 
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020525, 25 May 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

bool bCreatePathToFile(tstring sFilePath)
{
	string::size_type nPos = sFilePath.rfind('\\');
	if (nPos != string::npos) {
		if (nPos + 1 < sFilePath.size())
			sFilePath.erase(nPos + 1);
	}
	else // cant find
		return false;

	// create directory
	if (!CreateDirectory(sFilePath.c_str(), NULL)) {
		DWORD dwE = GetLastError();
		if (dwE == 183) // Cannot create a file when that file already exists. 
			return true;

		if (!bCreatePathToFile(sFilePath.substr(0, nPos)))
			return false;

		// try again 
		if (!CreateDirectory(sFilePath.c_str(), NULL))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteToFile
// Type            : Global
// Parameters      : hFile  - ?
//                   pszSrc - in UTF8 or ANSII
//                   nLen   - ?
// Returns         : Returns true if all the data was written to the file
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020629, 29 June 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

bool bWriteToFile(HANDLE hFile, const char *pszSrc, int nLen = -1)
{
	if (nLen < 0)
		nLen = (int)mir_strlen(pszSrc);

	DWORD dwBytesWritten;
	return WriteFile(hFile, pszSrc, nLen, &dwBytesWritten, NULL) && (dwBytesWritten == (DWORD)nLen);
}


/////////////////////////////////////////////////////////////////////
// Member Function : bWriteTextToFile
// Type            : Global
// Parameters      : hFile     - ?
//                   pszSrc    - ?
//                   bUtf8File - ?
// Returns         : Returns true if 
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 060130, 30 januar 2006
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

static bool bWriteTextToFile(HANDLE hFile, const wchar_t *pszSrc, bool bUtf8File, int nLen = -1)
{
	if (nLen != -1) {
		wchar_t *tmp = (wchar_t*)alloca(sizeof(wchar_t)*(nLen + 1));
		mir_wstrncpy(tmp, pszSrc, nLen + 1);
		pszSrc = tmp;
	}

	if (!bUtf8File) {
		// We need to downgrade text to ansi
		ptrA pszAstr(mir_u2a(pszSrc));
		return bWriteToFile(hFile, pszAstr, -1);
	}

	T2Utf pszUtf8(pszSrc);
	return bWriteToFile(hFile, pszUtf8, -1);
}


static bool bWriteTextToFile(HANDLE hFile, const char *pszSrc, bool bUtf8File, int nLen = -1)
{
	if (!bUtf8File)
		return bWriteToFile(hFile, pszSrc, nLen);

	if (nLen != -1) {
		char *tmp = (char*)alloca(nLen + 1);
		mir_strncpy(tmp, pszSrc, nLen + 1);
		pszSrc = tmp;
	}

	ptrA pszUtf8(mir_utf8encode(pszSrc));
	return bWriteToFile(hFile, pszUtf8, -1);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteNewLine
// Type            : Global
// Parameters      : hFile   - ?
//                   nIndent - ?
// Returns         : Returns true if all the data was written to the file
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020629, 29 June 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

const char szNewLineIndent[] = "\r\n                                                                                                   ";
bool bWriteNewLine(HANDLE hFile, DWORD dwIndent)
{
	if (dwIndent > sizeof(szNewLineIndent) - 2)
		dwIndent = sizeof(szNewLineIndent) - 2;
	
	return bWriteToFile(hFile, szNewLineIndent, dwIndent + 2);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteHexToFile
// Type            : Global
// Parameters      : hFile - ?
//                         - ?
//                   nSize - ?
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 021203, 03 December 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

bool bWriteHexToFile(HANDLE hFile, void * pData, int nSize)
{
	char cBuf[10];
	BYTE *p = (BYTE*)pData;
	for (int n = 0; n < nSize; n++) {
		mir_snprintf(cBuf, "%.2X ", p[n]);
		if (!bWriteToFile(hFile, cBuf, 3))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bReadMirandaDirAndPath
// Type            : Global
// Parameters      : None
// Returns         : void
// Description     : Used to set the internal path.
//                   Handles the reading from the mirandaboot.ini to get the %dbpath%
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

bool bReadMirandaDirAndPath()
{
	wchar_t szDBPath[MAX_PATH], tmp[MAX_PATH];
	mir_wstrcpy(szDBPath, pszDbPathError);
	PathToAbsoluteW(L"miranda32.exe", tmp);
	sMirandaPath = tmp;
	sMirandaPath.erase(sMirandaPath.find_last_of(L"\\"));
	Profile_GetPathW(MAX_PATH, szDBPath);
	sDBPath = szDBPath;
	Profile_GetNameW(MAX_PATH, szDBPath);
	sDBPath.append(L"\\").append(szDBPath);
	sDBPath.erase(sDBPath.size() - 4);
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceDBPath
// Type            : Global
// Parameters      : sRet - ?
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 021020, 20 October 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ReplaceDBPath(tstring &sRet)
{
	ReplaceAll(sRet, L"%dbpath%", sDBPath);
	// Try to firure out if it is a relative path ( ..\..\MsgExport\ )
	if (sRet.size() <= 2 || !(sRet[1] == ':' ||
		(sRet[0] == '\\' && sRet[1] == '\\'))) {
		// Relative path
		// we will prepend the mirande exe path to avoid problems 
		// if the current directory changes ( User receives a file )
		sRet = sMirandaPath + sRet;
	}
}


/////////////////////////////////////////////////////////////////////
// Member Function : GetFilePathFromUser
// Type            : Global
// Parameters      : hContact - Handle to user
// Returns         : string contaning the compleate file name and path
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

tstring GetFilePathFromUser(MCONTACT hContact)
{
	tstring sFilePath = sExportDir + _DBGetString(hContact, MODULE, "FileName", sDefaultFile.c_str());

	bool bNickUsed = sFilePath.find(L"%nick%") != string::npos;

	ReplaceDefines(hContact, sFilePath);

	tstring sNoDBPath = sFilePath;

	ReplaceTimeVariables(sFilePath);
	ReplaceDBPath(sFilePath);

	// Previous file name check to see if it has changed !!
	tstring sPrevFileName = _DBGetString(hContact, MODULE, "PrevFileName", L"");
	if (sNoDBPath != sPrevFileName) {
		if (!sPrevFileName.empty()) {
			ReplaceDBPath(sPrevFileName);

			// Here we will try to avoide the (Unknown Contact) in cases where the protocol for 
			// this user has been removed.
			if (bNickUsed && (wcsstr(pcli->pfnGetContactDisplayName(hContact, 0), LPGENW("(Unknown Contact)")) != 0))
				return sPrevFileName; // Then the filename must have changed from a correct path to one including the (Unknown Contact)

			// file name has changed
			if (enRenameAction != eDANothing) {

				// we can not use FILE_SHARE_DELETE because this is not supported by 
				// win 98 / ME 
				HANDLE hPrevFile = CreateFile(sPrevFileName.c_str(),
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

				if (hPrevFile != INVALID_HANDLE_VALUE) {
					CloseHandle(hPrevFile);
					wchar_t szTemp[500];
					// There is a previous file we can move 
					// ask user ?
					bool bTryRename;

					if (enRenameAction != eDAAutomatic) {
						tstring sRemoteUser = pcli->pfnGetContactDisplayName(hContact, 0);
						mir_snwprintf(szTemp,
							TranslateT("File name for the user \"%s\" has changed!\n\nfrom:\t%s\nto:\t%s\n\nDo you wish to rename file?"),
							sRemoteUser.c_str(),
							sPrevFileName.c_str(),
							sFilePath.c_str());
						bTryRename = MessageBox(NULL, szTemp, MSG_BOX_TITEL, MB_YESNO) == IDYES;
					}
					else
						bTryRename = true;


					if (bTryRename) {
						if (!MoveFile(sPrevFileName.c_str(), sFilePath.c_str())) {
							// this might be because the new path isent created 
							// so we will try to create it 
							bCreatePathToFile(sFilePath);

							while (!MoveFile(sPrevFileName.c_str(), sFilePath.c_str())) {
								mir_snwprintf(szTemp,
									TranslateT("Failed to rename file\n\nfrom:\t%s\nto:\t%s\n\nFailed with error: %s"),
									sPrevFileName.c_str(),
									sFilePath.c_str(),
									sGetErrorString().c_str());

								if (MessageBox(NULL, szTemp, MSG_BOX_TITEL, MB_RETRYCANCEL) != IDRETRY)
									break;
							}
						}
					}
				}
			}
		}

		// Store the Filename used so that we can check if it changes.
		db_set_ws(hContact, MODULE, "PrevFileName", sNoDBPath.c_str());
	}

	return sFilePath;
}


/////////////////////////////////////////////////////////////////////
// Member Function : FileNickFromHandle
// Type            : Global
// Parameters      : hContact - ?
// Returns         : string
// Description     : Replaceses invalid file name chars
// References      : -
// Remarks         : -
// Created         : 030107, 07 January 2003
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

tstring FileNickFromHandle(MCONTACT hContact)
{
	tstring ret = pcli->pfnGetContactDisplayName(hContact, 0);
	string::size_type nCur = 0;
	while ((nCur = ret.find_first_of(L":\\", nCur)) != ret.npos)
		ret[nCur] = cBadCharReplace;
	return ret;
}


/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceAllNoColon
// Type            : Global
// Parameters      : sSrc       - ?
//                   pszReplace - ?
//                   sNew       - ?
// Returns         : void
// Description     : Removes any ':' in the new string
//                   
// References      : -
// Remarks         : -
// Created         : 040205, 05 februar 2004
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ReplaceAllNoColon(tstring &sSrc, const wchar_t *pszReplace, tstring &sNew)
{
	tstring::size_type nCur = 0;
	while ((nCur = sNew.find_first_of(':', nCur)) != sNew.npos)
		sNew[nCur] = cBadCharReplace;
	ReplaceAll(sSrc, pszReplace, sNew);
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceDefines
// Type            : Global
// Parameters      : hContact - Handle to user
//                   sTarget  - String with either %user% or %UIN%, to replace in
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020525, 25 May 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ReplaceDefines(MCONTACT hContact, tstring & sTarget)
{
	if (sTarget.find(L"%nick%") != string::npos)
		ReplaceAll(sTarget, L"%nick%", FileNickFromHandle(hContact));

	bool bUINUsed = sTarget.find(L"%UIN%") != string::npos;
	bool bEMailUsed = sTarget.find(L"%e-mail%") != string::npos;
	bool bProtoUsed = sTarget.find(L"%protocol%") != string::npos;
	bool bIdentifierUsed = sTarget.find(L"%identifier%") != string::npos;

	if (bUINUsed || bEMailUsed || bProtoUsed || bIdentifierUsed) {
		string sProto = _DBGetStringA(hContact, "Protocol", "p", "");
		if (bUINUsed || (bIdentifierUsed && sProto == "ICQ")) {
			DWORD dwUIN = db_get_dw(hContact, sProto.c_str(), "UIN", 0);
			tstring sReplaceUin;
			if (dwUIN) {
				wchar_t sTmp[20];
				mir_snwprintf(sTmp, L"%d", dwUIN);
				sReplaceUin = sTmp;
			}
			else sReplaceUin = FileNickFromHandle(hContact);

			if (bUINUsed)
				ReplaceAll(sTarget, L"%UIN%", sReplaceUin);
			if (bIdentifierUsed && sProto == "ICQ") {
				bIdentifierUsed = false;
				ReplaceAll(sTarget, L"%identifier%", sReplaceUin);
			}
		}

		if (bEMailUsed || (bIdentifierUsed && sProto == "MSN")) {
			tstring sEMail = _DBGetString(hContact, sProto.c_str(), "e-mail", L"");
			if (sEMail.empty()) {
				sEMail = _DBGetString(hContact, "MSN", "e-mail", L"");
				if (sEMail.empty()) {
					// We can't finde the E-mail address we will use the the nick
					sEMail = FileNickFromHandle(hContact);
				}
			}
			if (bEMailUsed)
				ReplaceAllNoColon(sTarget, L"%e-mail%", sEMail);
			if (bIdentifierUsed && sProto == "MSN") {
				bIdentifierUsed = false;
				ReplaceAllNoColon(sTarget, L"%identifier%", sEMail);
			}
		}

		if (bIdentifierUsed && sProto == "Jabber") {
			tstring sReplace = _DBGetString(hContact, "Jabber", "jid", L"");
			if (sReplace.empty()) {
				sReplace = FileNickFromHandle(hContact);
			}
			bIdentifierUsed = false;
			ReplaceAll(sTarget, L"%identifier%", sReplace);
		}

		if (bProtoUsed) {
			tstring tmp = _DBGetString(hContact, "Protocol", "p", L"");
			ReplaceAllNoColon(sTarget, L"%protocol%", tmp);
		}

		if (bIdentifierUsed) // It has still not been replaced we will just use nick
			ReplaceAll(sTarget, L"%nick%", FileNickFromHandle(hContact));
	}

	if (sTarget.find(L"%group%") != string::npos) {
		tstring sGroup = _DBGetString(hContact, "CList", "Group", L"");
		ReplaceAllNoColon(sTarget, L"%group%", sGroup);
	}

	// We can't replace the : here because if the user uses C:\... in the file path 
	// this will also be replaced 
	string::size_type nCur = 0;
	while ((nCur = sTarget.find_first_of(L"/*?<>|\"", nCur)) != sTarget.npos)
		sTarget[nCur] = cBadCharReplace;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceTimeVariables
// Type            : Global
// Parameters      : sRet - ?
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 040219, 19 februar 2004
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ReplaceTimeVariables(tstring &sRet)
{
	if (sRet.find(L"%year%") != string::npos ||
		sRet.find(L"%month%") != string::npos ||
		sRet.find(L"%day%") != string::npos) {
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);
		wchar_t sTmp[20];

		mir_snwprintf(sTmp, L"%d", stTime.wYear);
		ReplaceAll(sRet, L"%year%", sTmp);
		mir_snwprintf(sTmp, L"%.2d", stTime.wMonth);
		ReplaceAll(sRet, L"%month%", sTmp);
		mir_snwprintf(sTmp, L"%.2d", stTime.wDay);
		ReplaceAll(sRet, L"%day%", sTmp);
	}
}

/////////////////////////////////////////////////////////////////////
// Member Function : UpdateFileToColWidth
// Type            : Global
// Parameters      : None
// Returns         : void
// Description     : updates clFileTo1ColWidth,
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void UpdateFileToColWidth()
{
	clFileTo1ColWidth.clear();

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		tstring sNick = pcli->pfnGetContactDisplayName(hContact, 0);
		string::size_type &rnValue = clFileTo1ColWidth[GetFilePathFromUser(hContact)];
		if (rnValue < sNick.size())
			rnValue = sNick.size();
	}
}

/////////////////////////////////////////////////////////////////////
// Member Function : DisplayErrorDialog
// Type            : Global
// Parameters      : pszError - ?
//                   sFile    - ?
//                   dbei     - ?
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 021203, 03 December 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void DisplayErrorDialog(const wchar_t *pszError, tstring& sFilePath, DBEVENTINFO * dbei)
{
	tstring sError = TranslateW(pszError);
	sError += sFilePath;
	sError += TranslateT("\nError: ");
	sError += sGetErrorString();
	sError += TranslateT("\nMessage has not been saved!\n");
	sError += TranslateT("Do you wish to save debug information?");
	if (MessageBox(NULL, sError.c_str(), MSG_BOX_TITEL, MB_YESNO) == IDYES) {
		OPENFILENAME ofn;       // common dialog box structure
		wchar_t szFile[260];       // buffer for file name
		mir_wstrcpy(szFile, L"DebugInfo.txt");

		// Initialize OPENFILENAME
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		//ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = _countof(szFile);
		ofn.lpstrFilter = TranslateT("All\0*.*\0Text\0*.TXT\0\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = 0 /*OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST*/;
		ofn.lpstrDefExt = L"TXT";

		// Display the Open dialog box. 
		if (GetSaveFileName(&ofn)) {
			HANDLE hf = CreateFile(ofn.lpstrFile, GENERIC_WRITE,
				0, (LPSECURITY_ATTRIBUTES)NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				(HANDLE)NULL); // file handle

			bWriteTextToFile(hf, sError.c_str(), false);
			if (dbei) {
				bWriteToFile(hf, "\r\ndbei          :");

				bWriteHexToFile(hf, dbei, sizeof(DBEVENTINFO));
				if (dbei->pBlob) {
					bWriteToFile(hf, "\r\ndbei.pBlob    :");
					bWriteHexToFile(hf, dbei->pBlob, min(dbei->cbBlob, 10000));
				}
				if (dbei->szModule) {
					bWriteToFile(hf, "\r\ndbei.szModule :");
					bWriteToFile(hf, dbei->szModule);
				}
			}
			CloseHandle(hf);
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Member Function : ExportDBEventInfo
// Type            : Global
// Parameters      : hContact  - handle to contact
//                   hFile     - handle to file
//                   sFilePath - path to file
//                   dbei      - Event to export
// Returns         : false on serious error, when file should be closed to not lost/overwrite any data
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 050429, 29 april 2005
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

bool ExportDBEventInfo(MCONTACT hContact, HANDLE hFile, tstring sFilePath, DBEVENTINFO &dbei, bool bAppendOnly)
{
	tstring sLocalUser;
	tstring sRemoteUser;
	string::size_type nFirstColumnWidth;

	if (bUseLessAndGreaterInExport) {
		sLocalUser = L"<<";
		sRemoteUser = L">>";
		nFirstColumnWidth = 4;
	}
	else {
		sLocalUser = ptrW(GetMyOwnNick(hContact));
		sRemoteUser = pcli->pfnGetContactDisplayName(hContact, 0);
		nFirstColumnWidth = max(sRemoteUser.size(), clFileTo1ColWidth[sFilePath]);
		nFirstColumnWidth = max(sLocalUser.size(), nFirstColumnWidth);
		nFirstColumnWidth += 2;
	}

	wchar_t szTemp[500];
	bool bWriteUTF8Format = false;

	if (bAppendOnly) {
		bWriteUTF8Format = bUseUtf8InNewFiles;
	}
	else {
		DWORD dwHighSize = 0;
		DWORD dwLowSize = GetFileSize(hFile, &dwHighSize);

		if (dwLowSize == INVALID_FILE_SIZE || dwLowSize != 0 || dwHighSize != 0) {
			DWORD dwDataRead = 0;
			BYTE ucByteOrder[3];
			if (ReadFile(hFile, ucByteOrder, 3, &dwDataRead, NULL))
				bWriteUTF8Format = bIsUtf8Header(ucByteOrder);

			DWORD dwPtr = SetFilePointer(hFile, 0, 0, FILE_END);
			if (dwPtr == INVALID_SET_FILE_POINTER) {
				// we need to aborte mission here because if we continue we risk 
				// overwriting old log.
				DisplayErrorDialog(LPGENW("Failed to move to the end of the file :\n"), sFilePath, NULL);
				return false;
			}
		}
		else {
			bWriteUTF8Format = bUseUtf8InNewFiles;
			if (bWriteUTF8Format) {
				if (!bWriteToFile(hFile, szUtf8ByteOrderHeader, sizeof(szUtf8ByteOrderHeader) - 1)) {
					DisplayErrorDialog(LPGENW("Failed to UTF8 byte order code to file :\n"), sFilePath, NULL);
					return false;
				}
			}
			tstring output = L"------------------------------------------------\r\n"
				LPGENW("      History for\r\n")
				LPGENW("User      : %User%\r\n")
				LPGENW("Protocol  : %Proto%\r\n")
				LPGENW("UIN       : %UIN%\r\n")
				LPGENW("FirstName : %FirstName%\r\n")
				LPGENW("LastName  : %LastName%\r\n")
				LPGENW("Age       : %Age%\r\n")
				LPGENW("Gender    : %Gender%\r\n")
				LPGENW("e-mail    : %e-mail%\r\n")
				LPGENW("Nick      : %Nick%\r\n")
				LPGENW("City      : %City%\r\n")
				LPGENW("State     : %State%\r\n")
				LPGENW("Phone     : %Phone%\r\n")
				LPGENW("Homepage  : %Homepage%\r\n")
				LPGENW("- About -\r\n%About%\r\n")
				L"------------------------------------------------\r\n";

			// This is written this way because I expect this will become a string the user may set 
			// in the options dialog.
			ReplaceAll(output, L"%User%", sRemoteUser);

			string sProto = _DBGetStringA(hContact, "Protocol", "p", "");
			ReplaceAll(output, L"%Proto%", _DBGetString(hContact, "Protocol", "p", L""));

			for (int nCur = 0; nCur < 9; nCur++)
				ReplaceAll(output, pszReplaceList[nCur], _DBGetString(hContact, sProto.c_str(), pszReplaceListA[nCur], L""));

			ptrW id(Contact_GetInfo(CNF_UNIQUEID, hContact, sProto.c_str()));
			if (id != NULL)
				ReplaceAll(output, L"%UIN%", id);

			mir_snwprintf(szTemp, L"%d", db_get_w(hContact, sProto.c_str(), "Age", 0));
			ReplaceAll(output, L"%Age%", szTemp);

			szTemp[0] = (wchar_t)db_get_b(hContact, sProto.c_str(), "Gender", 0);
			szTemp[1] = 0;
			ReplaceAll(output, L"%Gender%", szTemp);

			if (!bWriteTextToFile(hFile, output.data(), bWriteUTF8Format, (int)output.size())) {
				DisplayErrorDialog(LPGENW("Failed to write user details to file :\n"), sFilePath, NULL);
				return false;
			}
		}
	}

	// Get time stamp 
	int nIndent = mir_snwprintf(szTemp, L"%-*s", nFirstColumnWidth, dbei.flags & DBEF_SENT ? sLocalUser.c_str() : sRemoteUser.c_str());

	TimeZone_ToStringT(dbei.timestamp, sTimeFormat.c_str(), &szTemp[nIndent], _countof(szTemp) - nIndent - 2);

	nIndent = (int)mir_wstrlen(szTemp);
	szTemp[nIndent++] = ' ';

	// Write first part of line with name and timestamp
	if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, nIndent)) {
		DisplayErrorDialog(LPGENW("Failed to write timestamp and username to file :\n"), sFilePath, &dbei);
		return false;
	}

	if (dbei.pBlob != NULL && dbei.cbBlob >= 2) {
		dbei.pBlob[dbei.cbBlob] = 0;

		switch (dbei.eventType) {
		case EVENTTYPE_MESSAGE:
			{
				wchar_t *msg = DbEvent_GetTextW(&dbei, CP_ACP);
				if (!bWriteIndentedToFile(hFile, nIndent, msg, bWriteUTF8Format)) {
					DisplayErrorDialog(LPGENW("Failed to write message to the file :\n"), sFilePath, &dbei);
				}
				mir_free(msg);
			}
			break;

		case EVENTTYPE_URL:
		case EVENTTYPE_FILE:
			{
				const wchar_t *pszType;
				const char *pszData;

				if (dbei.eventType == EVENTTYPE_URL) {
					pszType = LPGENW("URL: ");
					pszData = (char *)dbei.pBlob;
				}
				else {
					pszType = LPGENW("File: ");
					pszData = (char *)(dbei.pBlob + sizeof(DWORD));
				}

				bool bWriteOk = false;

				int nLen = (int)mir_strlen(pszData);
				if ((pszData - (char *)dbei.pBlob) + nLen < (int)dbei.cbBlob) {
					if (bWriteTextToFile(hFile, pszType, bWriteUTF8Format) &&
						bWriteIndentedToFile(hFile, nIndent, pszData, bWriteUTF8Format)) {
						pszData += nLen + 1;
						if ((pszData - (char *)dbei.pBlob) >= (int)dbei.cbBlob) {
							bWriteOk = true;
						}
						else {
							nLen = (int)mir_strlen(pszData);
							if ((pszData - (char *)dbei.pBlob) + nLen < (int)dbei.cbBlob) {
								if (bWriteNewLine(hFile, nIndent) &&
									bWriteTextToFile(hFile, LPGENW("Description: "), bWriteUTF8Format) &&
									bWriteIndentedToFile(hFile, nIndent, pszData, bWriteUTF8Format)) {
									bWriteOk = true;
								}
							}
						}
					}
				}

				if (!bWriteOk)
					DisplayErrorDialog(LPGENW("Failed to write URL/File to the file :\n"), sFilePath, &dbei);
			}
			break;

		case EVENTTYPE_AUTHREQUEST:
		case EVENTTYPE_ADDED:
			{
				const wchar_t *pszTypes[] = {
					LPGENW("Nick      :"),
					LPGENW("FirstName :"),
					LPGENW("LastName  :"),
					LPGENW("e-mail    :"),
					LPGENW("Reason    :") };

				if (dbei.cbBlob < 8 || dbei.cbBlob > 5000) {
					int n = mir_snwprintf(szTemp, TranslateT("Invalid Database event received. Type %d, size %d"), dbei.eventType, dbei.cbBlob);
					if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n))
						DisplayErrorDialog(LPGENW("Failed to write Invalid Database event the file :\n"), sFilePath, &dbei);
					break;
				}

				bool bWriteOk = false;

				int nStringCount;
				const wchar_t *pszTitle;
				char *pszCurBlobPos;
				if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {	// request 
					//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
					nStringCount = 5;
					pszCurBlobPos = (char *)dbei.pBlob + sizeof(DWORD) * 2;
					pszTitle = LPGENW("The following user made an authorization request:");
				}
				else {  // Added
					//blob is: uin(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
					pszCurBlobPos = (char *)dbei.pBlob + sizeof(DWORD);
					nStringCount = 4;
					pszTitle = LPGENW("The following user added you to their contact list:");
				}

				if (bWriteTextToFile(hFile, pszTitle, bWriteUTF8Format) &&
					bWriteNewLine(hFile, nIndent) &&
					bWriteTextToFile(hFile, LPGENW("UIN       :"), bWriteUTF8Format)) {
					DWORD uin = *((PDWORD)(dbei.pBlob));
					int n = mir_snwprintf(szTemp, L"%d", uin);
					if (bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n)) {
						char *pszEnd = (char *)(dbei.pBlob + sizeof(dbei));
						for (int i = 0; i < nStringCount && pszCurBlobPos < pszEnd; i++) {
							if (*pszCurBlobPos) {
								if (!bWriteNewLine(hFile, nIndent) ||
									!bWriteTextToFile(hFile, TranslateW(pszTypes[i]), bWriteUTF8Format) ||
									!bWriteIndentedToFile(hFile, nIndent, pszCurBlobPos, bWriteUTF8Format)) {
									break;
								}
								pszCurBlobPos += mir_strlen(pszCurBlobPos);
							}
							pszCurBlobPos++;
						}
						bWriteOk = true;
					}
				}

				if (!bWriteOk)
					DisplayErrorDialog(LPGENW("Failed to write AUTHREQUEST or ADDED to the file :\n"), sFilePath, &dbei);
			}
			break;

		case ICQEVENTTYPE_EMAILEXPRESS:
		case ICQEVENTTYPE_WEBPAGER:
			{
				//e-mail express 
				//db event added to NULL contact
				//blob format is:
				//ASCIIZ    text, usually of the form "Subject: %s\r\n%s"
				//ASCIIZ    from name
				//ASCIIZ    from e-mail

				//www pager
				//db event added to NULL contact
				//blob format is:
				//ASCIIZ    text, usually "Sender IP: xxx.xxx.xxx.xxx\r\n%s"
				//ASCIIZ    from name
				//ASCIIZ    from e-mail
				const char* pszStr = (const char*)dbei.pBlob;

				if (dbei.eventType == ICQEVENTTYPE_EMAILEXPRESS)
					bWriteTextToFile(hFile, LPGENW("EmailExpress from:"), bWriteUTF8Format);
				else
					bWriteTextToFile(hFile, LPGENW("WebPager from:"), bWriteUTF8Format);

				bWriteNewLine(hFile, nIndent);

				size_t nMsgLenght = mir_strlen(pszStr) + 1;
				if (nMsgLenght < dbei.cbBlob) {
					size_t nFriendlyLen = mir_strlen(&pszStr[nMsgLenght]);
					bWriteTextToFile(hFile, &pszStr[nMsgLenght], bWriteUTF8Format, (int)nFriendlyLen);
					size_t nEmailOffset = nMsgLenght + nFriendlyLen + 1;
					if (nEmailOffset < dbei.cbBlob) {
						bWriteTextToFile(hFile, L"<", bWriteUTF8Format);
						size_t nEmailLen = mir_strlen(&pszStr[nEmailOffset]);
						bWriteTextToFile(hFile, &pszStr[nEmailOffset], bWriteUTF8Format, (int)nEmailLen);
						bWriteTextToFile(hFile, L">", bWriteUTF8Format);
					}
				}
				else bWriteTextToFile(hFile, LPGENW("No from address"), bWriteUTF8Format);

				if (!bWriteNewLine(hFile, nIndent) ||
					!bWriteIndentedToFile(hFile, nIndent, pszStr, bWriteUTF8Format)) {
					DisplayErrorDialog(LPGENW("Failed to write EmailExpress to the file :\n"), sFilePath, &dbei);
				}
			}
			break;

		case ICQEVENTTYPE_SMS:
			if (!bWriteIndentedToFile(hFile, nIndent, (const char*)dbei.pBlob, bWriteUTF8Format))
				DisplayErrorDialog(LPGENW("Failed to write SMS to the file :\n"), sFilePath, &dbei);
			break;

		default:
			int n = mir_snwprintf(szTemp, TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
			if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n))
				DisplayErrorDialog(LPGENW("Failed to write Unknown event to the file :\n"), sFilePath, &dbei);
			break;
		}
	}
	else {
		int n = mir_snwprintf(szTemp, TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
		bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n);
	}

	bWriteToFile(hFile, bAppendNewLine ? "\r\n\r\n" : "\r\n");
	UpdateFileViews(sFilePath.c_str());
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : nExportEvent
// Type            : Global
// Parameters      : wparam - handle to contact
//                   lparam - handle to the new DB event
// Returns         : int
// Description     : Called when an event is added to the DB
//                   Or from the Export All funktion
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

HANDLE openCreateFile(tstring sFilePath)
{
	GetLastError();// Clear last error !!

	HANDLE hFile = CreateFile(sFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		// this might be because the path isent created 
		// so we will try to create it 
		if (bCreatePathToFile(sFilePath)) {
			hFile = CreateFile(sFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}
	}

	return hFile;
}

bool bIsExportEnabled(MCONTACT hContact)
{
	if (!db_get_b(hContact, MODULE, "EnableLog", 1))
		return false;

	const char *szProto = GetContactProto(hContact);
	char szTemp[500];
	mir_snprintf(szTemp, "DisableProt_%s", szProto);
	if (!db_get_b(NULL, MODULE, szTemp, 1))
		return false;

	return true;
}

int nExportEvent(WPARAM hContact, LPARAM hDbEvent)
{
	if (!bIsExportEnabled(hContact))
		return 0;
	
	// Open/create file for writing
	tstring sFilePath = GetFilePathFromUser(hContact);
	HANDLE hFile = openCreateFile(sFilePath);
	if (hFile == INVALID_HANDLE_VALUE) {
		DisplayErrorDialog(LPGENW("Failed to open or create file :\n"), sFilePath, NULL);
		return 0;
	}

	// Write the event
	bExportEvent((MCONTACT)hContact, (MEVENT)hDbEvent, hFile, sFilePath, false);

	// Close the file
	CloseHandle(hFile);

	return 0;
}

bool bExportEvent(MCONTACT hContact, MEVENT hDbEvent, HANDLE hFile, tstring sFilePath, bool bAppendOnly)
{
	DBEVENTINFO dbei = {};
	int nSize = db_event_getBlobSize(hDbEvent);
	if (nSize > 0) {
		dbei.cbBlob = nSize;
		dbei.pBlob = (PBYTE)malloc(dbei.cbBlob + 2);
		dbei.pBlob[dbei.cbBlob] = 0;
		dbei.pBlob[dbei.cbBlob + 1] = 0;
		// Double null terminate, this shut pervent most errors 
		// where the blob received has an invalid format
	}

	bool result = true;
	if (!db_event_get(hDbEvent, &dbei)) {
		// Write the event
		result = ExportDBEventInfo(hContact, hFile, sFilePath, dbei, bAppendOnly);
	}
	if (dbei.pBlob)
		free(dbei.pBlob);
	return result;
}

#ifdef _UNICODE
bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const char *pszSrc, bool bUtf8File)
{
	int nLen = (int)mir_strlen(pszSrc);
	wchar_t * pszWstr = new wchar_t[nLen + 1];
	bool bRet = false;
	if (MultiByteToWideChar(CP_ACP, 0, pszSrc, nLen, pszWstr, nLen) == nLen) {
		pszWstr[nLen] = NULL;
		bRet = bWriteIndentedToFile(hFile, nIndent, pszWstr, bUtf8File);
	}
	delete[] pszWstr;

	return bRet;
}
#endif

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteIndentedToFile
// Type            : Global
// Parameters      : hFile   - ?
//                   nIndent - ?
//                   pszSrc  - 
// Returns         : Returns true if 
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020629, 29 June 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const wchar_t *pszSrc, bool bUtf8File)
{
	if (pszSrc == NULL)
		return true;

	bool bOk = true;
	bool bFirstLine = true;

	while (*pszSrc) {	// first we will scan forward in string to finde either new line or "max line with"
		int nLineLen = 0;
		do {
			if (pszSrc[nLineLen] == '\n' || pszSrc[nLineLen] == '\r')
				break;

			// if user set nMaxLineWidth to 0, we don't break anything, otherwise check the length
			if (nMaxLineWidth != 0 && nLineLen >= nMaxLineWidth) {
				// ok the line was not broken. we need to force a break
				// we will scan backwards again to finde a space !!
				// then we will look for a ? and so on.

				const wchar_t ac[] = { ' ', '?', '-', '.', ',' };
				for (int y = 0; y < _countof(ac); y++) {
					for (int n = nLineLen; n > 0; n--) {
						if (pszSrc[n] == ac[y]) {
							nLineLen = n;
							goto SuperBreak;
						}
					}
				}
				break;
			}
			nLineLen++;
		} while (pszSrc[nLineLen]);

		// trim away traling spaces !!
		if (nLineLen > 0) {
			while (pszSrc[nLineLen - 1] == ' ')
				nLineLen--;
		}

	SuperBreak:
		// nLineLen should contain the number af chars we need to write to the file 
		if (nLineLen > 0) {
			if (!bFirstLine)
				if (!bWriteNewLine(hFile, nIndent))
					bOk = false;

			if (!bWriteTextToFile(hFile, pszSrc, bUtf8File, nLineLen))
				bOk = false;
		}
		bFirstLine = false;

		// skip any noice chars, MAC style '\r' '\r' '\n' 
		// and excess spaces 
		const wchar_t *pszPrev = pszSrc;
		pszSrc += nLineLen;
		while (*pszSrc == ' ' || *pszSrc == '\n' || *pszSrc == '\r')
			pszSrc++;

		if (pszPrev == pszSrc) {
			// this is an programming error we have not moved forward in string 
			MessageBox(NULL, L"Programming error on line __LINE__ please report this", MSG_BOX_TITEL, MB_OK);
			break;
		}
	}

	// if bOk if false file writing failed 
	return bOk;
}

/////////////////////////////////////////////////////////////////////
// Member Function : nContactDeleted
// Type            : Global
// Parameters      : wparam - handle to the deleted Contact
//                   lparam - 0
// Returns         : int
// Description     : Called when an contact is about to be deleted
//                   
// References      : -
// Remarks         : -
// Created         : 021222, 22 December 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int nContactDeleted(WPARAM wparam, LPARAM /*lparam*/)
{
	MCONTACT hContact = (MCONTACT)wparam;

	HWND hInternalWindow = WindowList_Find(hInternalWindowList, hContact);
	if (hInternalWindow)
		CloseWindow(hInternalWindow);

	if (enDeleteAction == eDANothing)
		return 0;

	tstring sFilePath = GetFilePathFromUser(hContact);

	// Test if there is another user using this file
	for (MCONTACT hOtherContact = db_find_first(); hOtherContact; hOtherContact = db_find_next(hOtherContact))
		if (hContact != hOtherContact && sFilePath == GetFilePathFromUser(hOtherContact))
			return 0; // we found another contact abort mission :-)

	// Test to see if there is a file to delete
	HANDLE hPrevFile = CreateFile(sFilePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hPrevFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hPrevFile);

		wchar_t szTemp[500];
		mir_snwprintf(szTemp, L"%s\r\n%s",
			TranslateT("User has been deleted. Do you want to delete the file?"), sFilePath.c_str());

		if (enDeleteAction == eDAAutomatic ||
			MessageBox(NULL, szTemp, MSG_BOX_TITEL, MB_YESNO) == IDYES) {
			if (!DeleteFile(sFilePath.c_str())) {
				mir_snwprintf(szTemp,
					L"%s\r\n%s",
					TranslateT("Failed to delete the file"),
					sFilePath.c_str());

				DisplayLastError(szTemp);
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : SaveSettings
// Type            : Global
// Parameters      : None
// Returns         : void
// Description     : Save Settings
//                   
// References      : -
// Remarks         : -
// Created         : 020429, 29 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void SaveSettings()
{
	db_set_w(NULL, MODULE, "MaxLineWidth", (WORD)nMaxLineWidth);
	db_set_ws(NULL, MODULE, "ExportDir", sExportDir.c_str());
	db_set_ws(NULL, MODULE, "DefaultFile", sDefaultFile.c_str());
	db_set_ws(NULL, MODULE, "TimeFormat", sTimeFormat.c_str());

	db_set_ws(NULL, MODULE, "FileViewerPrg", sFileViewerPrg.c_str());
	db_set_b(NULL, MODULE, "UseInternalViewer", bUseInternalViewer());
	db_set_b(NULL, MODULE, "ReplaceHistory", bReplaceHistory);
	db_set_b(NULL, MODULE, "AppendNewLine", bAppendNewLine);
	db_set_b(NULL, MODULE, "UseUtf8InNewFiles", bUseUtf8InNewFiles);
	db_set_b(NULL, MODULE, "UseLessAndGreaterInExport", bUseLessAndGreaterInExport);

	db_set_b(NULL, MODULE, "RenameAction", (BYTE)enRenameAction);
	db_set_b(NULL, MODULE, "DeleteAction", (BYTE)enDeleteAction);
}

/////////////////////////////////////////////////////////////////////

wchar_t* GetMyOwnNick(MCONTACT hContact)
{
	wchar_t *p = Contact_GetInfo(CNF_DISPLAY, NULL, GetContactProto(hContact));
	return (p != NULL) ? p : mir_wstrdup(TranslateT("No_Nick"));
}
