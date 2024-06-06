
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

// Replacement for characters not allowed in file names.
const wchar_t cBadCharReplace = '_';

// g_sTimeFormat 
wstring g_sTimeFormat;

// path from options dialog
wstring g_sExportDir;

// The default filename. Used if no other file name is specified in DB.
wstring g_sDefaultFile;

// Used to store the width of the user name for a file.
// if a file contains events from many users the one user name
// may be shorter. so to make all event have the same first
// column width this map contains the largest user name
map<wstring, string::size_type, less<wstring> > clFileTo1ColWidth;

// default line width
int nMaxLineWidth = 80;

// This enum define the actions which MsgExport must take when a File is renamed
ENDialogAction g_enRenameAction;

// This enum define the actions which MsgExport must take when a user is delete
ENDialogAction g_enDeleteAction;

bool g_bUseJson;

DATABASELINK *g_pDriver = nullptr;

/////////////////////////////////////////////////////////////////////
// Member Function : nGetFormatCount
// Type            : Global
// Parameters      : pszToCheck - ?
// Returns         : int

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
// Member Function : _DBGetStringW
// Type            : Global
// Parameters      : hContact  - ?
//                   szModule  - ?
//                   szSetting - ?
//                   pszError  - ?
// Returns         : string
// Description     : Reads a string from the database 
//                   Just like those in database.h

wstring _DBGetStringW(MCONTACT hContact, const char *szModule, const char *szSetting, const wchar_t *pszError)
{
	DBVARIANT dbv = { 0 };
	if (db_get_ws(hContact, szModule, szSetting, &dbv))
		return pszError;

	wstring ret = dbv.pwszVal;
	db_free(&dbv);
	return ret;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceAll
// Type            : Global
// Parameters      : sSrc       - string to replace in
//                   pszReplace - what to replace
//                   sNew       - the string to insert instead of pszReplace
// Returns         : void
// Description     : will replace all occurrences of a string with another string
//                   used to replace %user%, and other user 

static void ReplaceAll(wstring &sSrc, const wchar_t *pszReplace, const wstring &sNew)
{
	string::size_type nCur = 0;
	while ((nCur = sSrc.find(pszReplace, nCur)) != sSrc.npos) {
		sSrc.replace(nCur, mir_wstrlen(pszReplace), sNew);
		nCur += sNew.size();
	}
}

static void ReplaceAll(wstring &sSrc, const wchar_t *pszReplace, const wchar_t *pszNew)
{
	wstring sNew = pszNew;
	ReplaceAll(sSrc, pszReplace, sNew);
}

/////////////////////////////////////////////////////////////////////
// Member Function : sGetErrorString
// Type            : Global
// Parameters      : dwError - ?
// Returns         : string

CMStringW sGetErrorString(uint32_t dwError)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		nullptr);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	CMStringW ret((LPCTSTR)lpMsgBuf);
	ret.Replace(L"\r", L" ");
	ret.Replace(L"\n", L" ");
	ret.Replace(L"  ", L" ");

	// Free the buffer.
	LocalFree(lpMsgBuf);
	return ret;
}

CMStringW sGetErrorString()
{
	return sGetErrorString(GetLastError());
}

void LogLastError(const wchar_t *pszError)
{
	uint32_t error = GetLastError();

	CMStringW sError = pszError;

	wchar_t szTemp[50];
	mir_snwprintf(szTemp, L"MsgExport error\r\nErrorCode: %d\r\n", error);
	sError += szTemp;
	sError += sGetErrorString(error);
	Netlib_LogW(nullptr, sError.c_str());
}

/////////////////////////////////////////////////////////////////////
// Member Function : nContactDeleted
// Type            : Global
// Parameters      : wparam - handle to the deleted Contact
//                   lparam - 0
// Returns         : int
// Description     : Called when an contact is about to be deleted

int nContactDeleted(WPARAM hContact, LPARAM)
{
	HWND hInternalWindow = WindowList_Find(hInternalWindowList, hContact);
	if (hInternalWindow)
		CloseWindow(hInternalWindow);

	if (g_enDeleteAction == eDANothing)
		return 0;

	wstring sFilePath = GetFilePathFromUser(hContact);

	// Test if there is another user using this file
	for (auto &hOtherContact : Contacts())
		if (hContact != hOtherContact && sFilePath == GetFilePathFromUser(hOtherContact))
			return 0; // we found another contact abort mission :-)

	// Test to see if there is a file to delete
	HANDLE hPrevFile = CreateFile(sFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hPrevFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hPrevFile);

		wchar_t szTemp[500];
		mir_snwprintf(szTemp, L"%s\r\n%s", TranslateT("User has been deleted. Do you want to delete the file?"), sFilePath.c_str());

		if (g_enDeleteAction == eDAAutomatic || MessageBox(nullptr, szTemp, MSG_BOX_TITEL, MB_YESNO) == IDYES) {
			if (!DeleteFile(sFilePath.c_str())) {
				mir_snwprintf(szTemp, L"%s\r\n%s", TranslateT("Failed to delete the file"), sFilePath.c_str());
				LogLastError(szTemp);
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////

wchar_t* GetMyOwnNick(MCONTACT hContact)
{
	wchar_t *p = Contact::GetInfo(CNF_DISPLAY, NULL, Proto_GetBaseAccountName(hContact));
	return (p != nullptr) ? p : mir_wstrdup(TranslateT("No_Nick"));
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceDBPath
// Type            : Global
// Parameters      : sRet - ?

void ReplaceDBPath(wstring &sRet)
{
	ReplaceAll(sRet, L"%dbpath%", VARSW(L"%miranda_userdata%"));
}

/////////////////////////////////////////////////////////////////////
// Member Function : GetFilePathFromUser
// Type            : Global
// Parameters      : hContact - Handle to user
// Returns         : string containing the complete file name and path

static bool CompareNoExt(const wstring &s1, const wstring &s2)
{
	wstring t1(s1), t2(s2);
	ReplaceAll(t1, L".txt.json", L".txt");
	ReplaceAll(t2, L".txt.json", L".txt");
	return t1 == t2;
}

wstring GetFilePathFromUser(MCONTACT hContact)
{
	wstring sFilePath = g_sExportDir + _DBGetStringW(hContact, MODULENAME, "FileName", g_sDefaultFile.c_str());
	if (g_bUseJson)
		sFilePath += L".json";

	bool bNickUsed = sFilePath.find(L"%nick%") != string::npos;

	ReplaceDefines(hContact, sFilePath);

	wstring sNoDBPath = sFilePath;

	ReplaceTimeVariables(sFilePath);
	ReplaceDBPath(sFilePath);

	// Previous file name check to see if it has changed !!
	wstring sPrevFileName = _DBGetStringW(hContact, MODULENAME, "PrevFileName", L"");
	if (!CompareNoExt(sNoDBPath, sPrevFileName)) {
		if (!sPrevFileName.empty()) {
			ReplaceDBPath(sPrevFileName);

			// Here we will try to avoid the (Unknown contact) in cases where the protocol for 
			// this user has been removed.
			if (bNickUsed && Clist_GetContactDisplayName(hContact, GCDNF_NOMYHANDLE) == nullptr)
				return sPrevFileName; // Then the filename must have changed from a correct path to one including the (Unknown contact)

			// file name has changed
			if (g_enRenameAction != eDANothing) {
				// we can not use FILE_SHARE_DELETE because this is not supported by win 98 / ME 
				HANDLE hPrevFile = CreateFile(sPrevFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				if (hPrevFile != INVALID_HANDLE_VALUE) {
					CloseHandle(hPrevFile);
					wchar_t szTemp[500];
					// There is a previous file we can move 
					// ask user ?
					bool bTryRename;

					if (g_enRenameAction != eDAAutomatic) {
						wstring sRemoteUser = Clist_GetContactDisplayName(hContact);
						mir_snwprintf(szTemp,
							TranslateT("File name for the user \"%s\" has changed!\n\nfrom:\t%s\nto:\t%s\n\nDo you wish to rename file?"),
							sRemoteUser.c_str(),
							sPrevFileName.c_str(),
							sFilePath.c_str());
						bTryRename = MessageBox(nullptr, szTemp, MSG_BOX_TITEL, MB_YESNO) == IDYES;
					}
					else bTryRename = true;

					if (bTryRename) {
						if (!MoveFile(sPrevFileName.c_str(), sFilePath.c_str())) {
							// this might be because the new path isn't created 
							// so we will try to create it 
							CreatePathToFileW(sFilePath.c_str());

							while (!MoveFile(sPrevFileName.c_str(), sFilePath.c_str())) {
								mir_snwprintf(szTemp,
									TranslateT("Failed to rename file\n\nfrom:\t%s\nto:\t%s\n\nFailed with error: %s"),
									sPrevFileName.c_str(),
									sFilePath.c_str(),
									sGetErrorString().c_str());

								if (MessageBox(nullptr, szTemp, MSG_BOX_TITEL, MB_RETRYCANCEL) != IDRETRY)
									break;
							}
						}
					}
				}
			}
		}

		// Store the Filename used so that we can check if it changes.
		g_plugin.setWString(hContact, "PrevFileName", sNoDBPath.c_str());
	}

	return sFilePath;
}

/////////////////////////////////////////////////////////////////////
// Member Function : FileNickFromHandle
// Type            : Global
// Parameters      : hContact - ?
// Returns         : string
// Description     : Replaces invalid file name chars

wstring FileNickFromHandle(MCONTACT hContact)
{
	wstring ret = Clist_GetContactDisplayName(hContact);
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

void ReplaceAllNoColon(wstring &sSrc, const wchar_t *pszReplace, wstring &sNew)
{
	wstring::size_type nCur = 0;
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

static wstring GetUniqueId(MCONTACT hContact, const char *szProto)
{
	ptrW uniqueId(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
	return (uniqueId == nullptr) ? L"(null)" : uniqueId;
}

void ReplaceDefines(MCONTACT hContact, wstring &sTarget)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);

	if (sTarget.find(L"%nick%") != string::npos)
		ReplaceAll(sTarget, L"%nick%", FileNickFromHandle(hContact));

	if (sTarget.find(L"%UIN%") != string::npos) {
		uint32_t dwUIN = db_get_dw(hContact, szProto, "UIN", 0);
		wstring sReplaceUin;
		if (dwUIN) {
			wchar_t sTmp[20];
			mir_snwprintf(sTmp, L"%d", dwUIN);
			sReplaceUin = sTmp;
		}
		else sReplaceUin = GetUniqueId(hContact, szProto);

		ReplaceAll(sTarget, L"%UIN%", sReplaceUin);
	}

	if (sTarget.find(L"%e-mail%") != string::npos) {
		wstring sEMail = _DBGetStringW(hContact, szProto, "e-mail", L"");
		if (sEMail.empty())
			sEMail = FileNickFromHandle(hContact);
		ReplaceAllNoColon(sTarget, L"%e-mail%", sEMail);
	}

	if (sTarget.find(L"%identifier%") != string::npos)
		ReplaceAll(sTarget, L"%identifier%", GetUniqueId(hContact, szProto));

	if (sTarget.find(L"%id%") != string::npos)
		ReplaceAll(sTarget, L"%id%", GetUniqueId(hContact, szProto));

	if (sTarget.find(L"%protocol%") != string::npos) {
		wstring tmp = _DBGetStringW(hContact, "Protocol", "p", L"");
		ReplaceAllNoColon(sTarget, L"%protocol%", tmp);
	}

	if (sTarget.find(L"%group%") != string::npos) {
		wstring sGroup = _DBGetStringW(hContact, "CList", "Group", L"");
		ReplaceAllNoColon(sTarget, L"%group%", sGroup);
	}

	// we don't touch the leading colon and \\ either
	string::size_type nCur = 2;
	while ((nCur = sTarget.find_first_of(L"/*?:<>|\"", nCur)) != sTarget.npos)
		sTarget[nCur] = cBadCharReplace;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceTimeVariables
// Type            : Global
// Parameters      : sRet - ?

void ReplaceTimeVariables(wstring &sRet)
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

void UpdateFileToColWidth()
{
	clFileTo1ColWidth.clear();

	for (auto &hContact : Contacts()) {
		wstring sNick = Clist_GetContactDisplayName(hContact);
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

void DisplayErrorDialog(const wchar_t *pszError, wstring &sFilePath)
{
	wstring sError = TranslateW(pszError);
	sError += sFilePath;
	sError += TranslateT("\nError: ");
	sError += sGetErrorString();
	sError += TranslateT("\nMessage has not been saved!\n");
	sError += TranslateT("Do you wish to save debug information?");
	MessageBox(nullptr, sError.c_str(), MSG_BOX_TITEL, MB_OK | MB_ICONERROR);
}

/////////////////////////////////////////////////////////////////////
// Member Function : nExportEvent
// Type            : Global
// Parameters      : wparam - handle to contact
//                   lparam - handle to the new DB event
// Returns         : int
// Description     : Called when an event is added to the DB
//                   Or from the Export All funktion

HANDLE openCreateFile(const wstring &sFilePath)
{
	SetLastError(0);

	HANDLE hFile = CreateFile(sFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		// this might be because the path isent created 
		// so we will try to create it 
		if (!CreatePathToFileW(sFilePath.c_str()))
			hFile = CreateFile(sFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	}

	return hFile;
}

bool bIsExportEnabled(MCONTACT hContact)
{
	if (!g_plugin.getByte(hContact, "EnableLog", 1))
		return false;

	const char *szProto = Proto_GetBaseAccountName(hContact);
	char szTemp[500];
	mir_snprintf(szTemp, "DisableProt_%s", szProto);
	if (!g_plugin.getByte(szTemp, 1))
		return false;

	return true;
}

int nExportEvent(WPARAM hContact, LPARAM hDbEvent)
{
	if (!bIsExportEnabled(hContact))
		return 0;
	
	// Open/create file for writing
	wstring sFilePath = GetFilePathFromUser(hContact);
	MDatabaseExport *pJson = nullptr;
	HANDLE hFile;

	if (g_bUseJson) {
		pJson = g_pDriver->Export(sFilePath.c_str());
		if (pJson == nullptr) {
			DisplayErrorDialog(LPGENW("Failed to open or create file:\n"), sFilePath);
			return 0;
		}

		hFile = pJson;
	}
	else {
		hFile = openCreateFile(sFilePath);
		if (hFile == INVALID_HANDLE_VALUE) {
			DisplayErrorDialog(LPGENW("Failed to open or create file:\n"), sFilePath);
			return 0;
		}
	}

	// Write the event
	bExportEvent(hContact, hDbEvent, hFile, sFilePath, false);

	// Close the file
	if (pJson) {
		pJson->EndExport();
		delete pJson;
	}
	else CloseHandle(hFile);

	return 0;
}
