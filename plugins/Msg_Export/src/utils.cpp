
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

// path used then %dbpath% is used in export file path
wstring g_sDBPath = pszDbPathError;

// path to miranda exe file used when to avoid relative paths
wstring g_sMirandaPath = pszDbPathError;

// Used to store the width of the user name for a file.
// if a file contains events from many users the one user name
// may be shorter. so to make all event have the same first
// column width this map contains the largest user name
map<wstring, string::size_type, less<wstring> > clFileTo1ColWidth;

// default line width
int nMaxLineWidth = 80;

// Allow this plugin to replace the history function of miranda !!
bool g_bReplaceHistory;

// This enum define the actions which MsgExport must take when a File is renamed
ENDialogAction g_enRenameAction;

// This enum define the actions which MsgExport must take when a user is delete
ENDialogAction g_enDeleteAction;

// If true MsgExport will use << and >> instead of the nick in the exported format
bool g_bUseLessAndGreaterInExport;

bool g_bAppendNewLine;
bool g_bUseUtf8InNewFiles;
bool g_bUseJson;

const char szUtf8ByteOrderHeader[] = "\xEF\xBB\xBF";
bool bIsUtf8Header(uint8_t * pucByteOrder)
{
	return memcmp(pucByteOrder, szUtf8ByteOrderHeader, 3) == 0;
}

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
// Member Function : bWriteToFile
// Type            : Global
// Parameters      : hFile  - ?
//                   pszSrc - in UTF8 or ANSII
//                   nLen   - ?
// Returns         : Returns true if all the data was written to the file

static bool bWriteToFile(HANDLE hFile, const char *pszSrc, int nLen = -1)
{
	if (nLen < 0)
		nLen = (int)mir_strlen(pszSrc);

	DWORD dwBytesWritten;
	return WriteFile(hFile, pszSrc, nLen, &dwBytesWritten, nullptr) && (dwBytesWritten == (uint32_t)nLen);
}


/////////////////////////////////////////////////////////////////////
// Member Function : bWriteTextToFile
// Type            : Global
// Parameters      : hFile     - ?
//                   pszSrc    - ?
//                   bUtf8File - ?
// Returns         : Returns true if 

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

	return bWriteToFile(hFile, T2Utf(pszSrc), -1);
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

	return bWriteToFile(hFile, ptrA(mir_utf8encode(pszSrc)), -1);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteNewLine
// Type            : Global
// Parameters      : hFile   - ?
//                   nIndent - ?
// Returns         : Returns true if all the data was written to the file

const char szNewLineIndent[] = "\r\n                                                                                                   ";
bool bWriteNewLine(HANDLE hFile, uint32_t dwIndent)
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

bool bWriteHexToFile(HANDLE hFile, void * pData, int nSize)
{
	char cBuf[10];
	uint8_t *p = (uint8_t*)pData;
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

bool bReadMirandaDirAndPath()
{
	wchar_t szDBPath[MAX_PATH], tmp[MAX_PATH];
	wcsncpy_s(szDBPath, pszDbPathError, _TRUNCATE);
	PathToAbsoluteW(L"miranda32.exe", tmp);
	g_sMirandaPath = tmp;
	g_sMirandaPath.erase(g_sMirandaPath.find_last_of(L"\\"));
	Profile_GetPathW(MAX_PATH, szDBPath);
	g_sDBPath = szDBPath;
	Profile_GetNameW(MAX_PATH, szDBPath);
	g_sDBPath.append(L"\\").append(szDBPath);
	g_sDBPath.erase(g_sDBPath.size() - 4);
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ReplaceDBPath
// Type            : Global
// Parameters      : sRet - ?

void ReplaceDBPath(wstring &sRet)
{
	ReplaceAll(sRet, L"%dbpath%", g_sDBPath);
	// Try to figure out if it is a relative path ( ..\..\MsgExport\ )
	if (sRet.size() <= 2 || !(sRet[1] == ':' ||
		(sRet[0] == '\\' && sRet[1] == '\\'))) {
		// Relative path
		// we will prepend the mirande exe path to avoid problems 
		// if the current directory changes ( User receives a file )
		sRet = g_sMirandaPath + sRet;
	}
}


/////////////////////////////////////////////////////////////////////
// Member Function : GetFilePathFromUser
// Type            : Global
// Parameters      : hContact - Handle to user
// Returns         : string containing the complete file name and path

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
	if (sNoDBPath != sPrevFileName) {
		if (!sPrevFileName.empty()) {
			ReplaceDBPath(sPrevFileName);

			// Here we will try to avoid the (Unknown contact) in cases where the protocol for 
			// this user has been removed.
			if (bNickUsed && Clist_GetContactDisplayName(hContact, GCDNF_NOMYHANDLE) == nullptr)
				return sPrevFileName; // Then the filename must have changed from a correct path to one including the (Unknown contact)

			// file name has changed
			if (g_enRenameAction != eDANothing) {

				// we can not use FILE_SHARE_DELETE because this is not supported by 
				// win 98 / ME 
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

void DisplayErrorDialog(const wchar_t *pszError, wstring &sFilePath, DBEVENTINFO *dbei)
{
	wstring sError = TranslateW(pszError);
	sError += sFilePath;
	sError += TranslateT("\nError: ");
	sError += sGetErrorString();
	sError += TranslateT("\nMessage has not been saved!\n");
	sError += TranslateT("Do you wish to save debug information?");
	if (MessageBox(nullptr, sError.c_str(), MSG_BOX_TITEL, MB_YESNO) == IDYES) {
		wchar_t szFile[260];       // buffer for file name
		wcsncpy_s(szFile, L"DebugInfo.txt", _TRUNCATE);

		// Initialize OPENFILENAME
		OPENFILENAME ofn = {};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = _countof(szFile);
		ofn.lpstrFilter = TranslateT("All\0*.*\0Text\0*.TXT\0\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = L"TXT";

		// Display the Open dialog box. 
		if (GetSaveFileName(&ofn)) {
			HANDLE hf = CreateFile(ofn.lpstrFile, GENERIC_WRITE,
				0, (LPSECURITY_ATTRIBUTES)nullptr,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				(HANDLE)nullptr); // file handle

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

const char *pSettings[] =
{
	LPGEN("FirstName"),
	LPGEN("LastName"),
	LPGEN("e-mail"),
	LPGEN("Nick"),
	LPGEN("Age"),
	LPGEN("Gender"),
	LPGEN("City"),
	LPGEN("State"),
	LPGEN("Phone"),
	LPGEN("Homepage"),
	LPGEN("About")
};

static wchar_t* getEventString(DBEVENTINFO &dbei, char *&buf)
{
	char *in = buf;
	buf += mir_strlen(buf) + 1;
	return (dbei.flags & DBEF_UTF) ? mir_utf8decodeW(in) : mir_a2u(in);
}

static bool ExportDBEventInfo(MCONTACT hContact, HANDLE hFile, const wstring &sFilePath, DBEVENTINFO &dbei, bool bAppendOnly)
{
	wstring sLocalUser;
	wstring sRemoteUser;
	string::size_type nFirstColumnWidth;

	if (g_bUseLessAndGreaterInExport) {
		sLocalUser = L"<<";
		sRemoteUser = L">>";
		nFirstColumnWidth = 4;
	}
	else {
		sLocalUser = ptrW(GetMyOwnNick(hContact));
		sRemoteUser = Clist_GetContactDisplayName(hContact);
		nFirstColumnWidth = max(sRemoteUser.size(), clFileTo1ColWidth[sFilePath]);
		nFirstColumnWidth = max(sLocalUser.size(), nFirstColumnWidth);
		nFirstColumnWidth += 2;
	}

	wchar_t szTemp[500];
	bool bWriteUTF8Format = false;

	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr) {
		Netlib_Logf(0, MODULENAME ": cannot write message for a contact %d without protocol", hContact);
		return false;
	}

	if (bAppendOnly) {
		bWriteUTF8Format = g_bUseUtf8InNewFiles;

		if (g_bUseJson) {
			SetFilePointer(hFile, -3, nullptr, FILE_END);
			bWriteToFile(hFile, ",", 1);
		}
	}
	else {
		DWORD dwHighSize = 0;
		DWORD dwLowSize = GetFileSize(hFile, &dwHighSize);
		if (dwLowSize == INVALID_FILE_SIZE || dwLowSize != 0 || dwHighSize != 0) {
			DWORD dwDataRead = 0;
			uint8_t ucByteOrder[3];
			if (ReadFile(hFile, ucByteOrder, 3, &dwDataRead, nullptr))
				bWriteUTF8Format = bIsUtf8Header(ucByteOrder);

			DWORD dwPtr = SetFilePointer(hFile, g_bUseJson ? -3 : 0, nullptr, FILE_END);
			if (dwPtr == INVALID_SET_FILE_POINTER)
				return false;

			if (g_bUseJson)
				bWriteToFile(hFile, ",", 1);		
		}
		else {
			if (g_bUseJson) {
				JSONNode pRoot, pInfo, pHist(JSON_ARRAY);
				pInfo.set_name("info");
				pInfo.push_back(JSONNode("user", T2Utf(sRemoteUser.c_str()).get()));
				pInfo.push_back(JSONNode("proto", szProto));

				ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
				if (id != NULL)
					pInfo.push_back(JSONNode("uin", T2Utf(id).get()));

				szTemp[0] = (wchar_t)db_get_b(hContact, szProto, "Gender", 0);
				if (szTemp[0]) {
					szTemp[1] = 0;
					pInfo.push_back(JSONNode("gender", T2Utf(szTemp).get()));
				}

				int age = db_get_w(hContact, szProto, "Age", 0);
				if (age != 0)
					pInfo.push_back(JSONNode("age", age));

				for (auto &it : pSettings) {
					wstring szValue = _DBGetStringW(hContact, szProto, it, L"");
					if (!szValue.empty())
						pInfo.push_back(JSONNode(it, T2Utf(szValue.c_str()).get()));
				}
				pRoot.push_back(pInfo);

				pHist.set_name("history");
				pRoot.push_back(pHist);

				std::string output = pRoot.write_formatted();
				if (!bWriteTextToFile(hFile, output.c_str(), false, (int)output.size()))
					return false;

				SetFilePointer(hFile, -3, nullptr, FILE_CURRENT);
			}
			else {
				bWriteUTF8Format = g_bUseUtf8InNewFiles;
				if (bWriteUTF8Format)
					if (!bWriteToFile(hFile, szUtf8ByteOrderHeader, sizeof(szUtf8ByteOrderHeader) - 1))
						return false;

				CMStringW output = L"------------------------------------------------\r\n";
				output.AppendFormat(L"%s\r\n", TranslateT("      History for"));

				// This is written this way because I expect this will become a string the user may set 
				// in the options dialog.
				output.AppendFormat(L"%-10s: %s\r\n", TranslateT("User"), sRemoteUser.c_str());
				output.AppendFormat(L"%-10s: %S\r\n", TranslateT("Account"), szProto);

				ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
				if (id != NULL)
					output.AppendFormat(L"%-10s: %s\r\n", TranslateT("User ID"), id.get());

				szTemp[0] = (wchar_t)db_get_b(hContact, szProto, "Gender", 0);
				if (szTemp[0]) {
					szTemp[1] = 0;
					output.AppendFormat(L"%-10s: %s\r\n", TranslateT("Gender"), szTemp);
				}

				int age = db_get_w(hContact, szProto, "Age", 0);
				if (age != 0)
					output.AppendFormat(L"%-10s: %d\r\n", TranslateT("Age"), age);

				for (auto &it : pSettings) {
					wstring szValue = _DBGetStringW(hContact, szProto, it, L"");
					if (!szValue.empty()) {
						mir_snwprintf(szTemp, L"%-10s: %s\r\n", TranslateW(_A2T(it)), szValue.c_str());
						output += szTemp;
					}
				}

				output += L"------------------------------------------------\r\n";

				if (!bWriteTextToFile(hFile, output, bWriteUTF8Format, output.GetLength()))
					return false;
			}
		}
	}

	if (g_bUseJson) {
		JSONNode pRoot;
		pRoot.push_back(JSONNode("type", dbei.eventType));
		if (mir_strcmp(dbei.szModule, szProto))
			pRoot.push_back(JSONNode("module", dbei.szModule));

		TimeZone_PrintTimeStamp(UTC_TIME_HANDLE, dbei.timestamp, L"I", szTemp, _countof(szTemp), 0);
		pRoot.push_back(JSONNode("isotime", T2Utf(szTemp).get()));

		std::string flags;
		if (dbei.flags & DBEF_SENT)
			flags += "m";
		if (dbei.flags & DBEF_READ)
			flags += "r";
		pRoot.push_back(JSONNode("flags", flags));

		if (dbei.eventType == EVENTTYPE_FILE) {
			char *p = (char*)dbei.pBlob + sizeof(uint32_t);
			ptrW wszFileName(getEventString(dbei, p));
			ptrW wszDescr(getEventString(dbei, p));

			pRoot << WCHAR_PARAM("file", wszFileName);
			if (mir_wstrlen(wszDescr))
				pRoot << WCHAR_PARAM("descr", wszDescr);
		}
		else {
			ptrW msg(DbEvent_GetTextW(&dbei, CP_ACP));
			if (msg)
				pRoot.push_back(JSONNode("body", T2Utf(msg).get()));
		}

		std::string output = pRoot.write_formatted();
		output += "\n]}";

		if (!bWriteTextToFile(hFile, output.c_str(), false, (int)output.size()))
			return false;

		return true;
	}

	// Get time stamp 
	int nIndent = mir_snwprintf(szTemp, L"%-*s", nFirstColumnWidth, dbei.flags & DBEF_SENT ? sLocalUser.c_str() : sRemoteUser.c_str());

	TimeZone_ToStringT(dbei.timestamp, g_sTimeFormat.c_str(), &szTemp[nIndent], _countof(szTemp) - nIndent - 2);

	nIndent = (int)mir_wstrlen(szTemp);
	szTemp[nIndent++] = ' ';

	// Write first part of line with name and timestamp
	if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, nIndent))
		return false;

	if (dbei.pBlob != nullptr && dbei.cbBlob >= 2) {
		dbei.pBlob[dbei.cbBlob] = 0;

		switch (dbei.eventType) {
		case EVENTTYPE_MESSAGE:
			bWriteIndentedToFile(hFile, nIndent, ptrW(DbEvent_GetTextW(&dbei, CP_ACP)), bWriteUTF8Format);
			break;

		case EVENTTYPE_FILE:
			{
				char *p = (char*)dbei.pBlob + sizeof(uint32_t);
				ptrW wszFileName(getEventString(dbei, p));
				ptrW wszDescr(getEventString(dbei, p));

				const wchar_t *pszType = LPGENW("File: ");
				bWriteTextToFile(hFile, pszType, bWriteUTF8Format);
				bWriteIndentedToFile(hFile, nIndent, wszFileName, bWriteUTF8Format);

				if (mir_wstrlen(wszDescr)) {
					bWriteNewLine(hFile, nIndent);
					bWriteTextToFile(hFile, LPGENW("Description: "), bWriteUTF8Format);
					bWriteIndentedToFile(hFile, nIndent, wszDescr, bWriteUTF8Format);
				}
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
					bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n);
					break;
				}

				int nStringCount;
				const wchar_t *pszTitle;
				char *pszCurBlobPos;
				if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {	// request 
					//blob is: uin(uint32_t), hContact(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
					nStringCount = 5;
					pszCurBlobPos = (char *)dbei.pBlob + sizeof(uint32_t) * 2;
					pszTitle = LPGENW("The following user made an authorization request:");
				}
				else {  // Added
					//blob is: uin(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
					pszCurBlobPos = (char *)dbei.pBlob + sizeof(uint32_t);
					nStringCount = 4;
					pszTitle = LPGENW("The following user added you to their contact list:");
				}

				if (bWriteTextToFile(hFile, pszTitle, bWriteUTF8Format) &&
					bWriteNewLine(hFile, nIndent) &&
					bWriteTextToFile(hFile, LPGENW("UIN       :"), bWriteUTF8Format)) {
					uint32_t uin = *((PDWORD)(dbei.pBlob));
					int n = mir_snwprintf(szTemp, L"%d", uin);
					if (bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n)) {
						char *pszEnd = (char *)(dbei.pBlob + sizeof(dbei));
						for (int i = 0; i < nStringCount && pszCurBlobPos < pszEnd; i++) {
							if (*pszCurBlobPos) {
								if (!bWriteNewLine(hFile, nIndent) ||
									!bWriteTextToFile(hFile, TranslateW(pszTypes[i]), bWriteUTF8Format) ||
									!bWriteIndentedToFile(hFile, nIndent, _A2T(pszCurBlobPos), bWriteUTF8Format)) {
									break;
								}
								pszCurBlobPos += mir_strlen(pszCurBlobPos);
							}
							pszCurBlobPos++;
						}
					}
				}
			}
			break;

		default:
			int n = mir_snwprintf(szTemp, TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
			bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n);
			break;
		}
	}
	else {
		int n = mir_snwprintf(szTemp, TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
		bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n);
	}

	bWriteToFile(hFile, g_bAppendNewLine ? "\r\n\r\n" : "\r\n");
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
	HANDLE hFile = openCreateFile(sFilePath);
	if (hFile == INVALID_HANDLE_VALUE) {
		DisplayErrorDialog(LPGENW("Failed to open or create file:\n"), sFilePath, nullptr);
		return 0;
	}

	// Write the event
	bExportEvent((MCONTACT)hContact, (MEVENT)hDbEvent, hFile, sFilePath, false);

	// Close the file
	CloseHandle(hFile);

	return 0;
}

bool bExportEvent(MCONTACT hContact, MEVENT hDbEvent, HANDLE hFile, const wstring &sFilePath, bool bAppendOnly)
{
	bool result = true;

	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (!db_event_get(hDbEvent, &dbei)) {
		if (db_mc_isMeta(hContact))
			hContact = db_event_getContact(hDbEvent);

		// Write the event
		result = ExportDBEventInfo(hContact, hFile, sFilePath, dbei, bAppendOnly);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bWriteIndentedToFile
// Type            : Global
// Parameters      : hFile   - ?
//                   nIndent - ?
//                   pszSrc  - 
// Returns         : Returns true if 

bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const wchar_t *pszSrc, bool bUtf8File)
{
	if (pszSrc == nullptr)
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
				for (auto &it : ac) {
					for (int n = nLineLen; n > 0; n--) {
						if (pszSrc[n] == it) {
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
			MessageBox(nullptr, L"Programming error on line __LINE__ please report this", MSG_BOX_TITEL, MB_OK);
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
