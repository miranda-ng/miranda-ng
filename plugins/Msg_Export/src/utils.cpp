
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

#include "Glob.h"

// Default error string used upon errors 
const TCHAR *pszNickError = LPGENT("No_Nick");
const TCHAR *pszGroupError = LPGENT("No_Group");
const TCHAR *pszDbPathError = _T(".");

// Replacement for chareteres not alowed in file names.
const TCHAR cBadCharReplace = _T('_');

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

const TCHAR *pszReplaceList[] =
{
	_T("%FirstName%"),
	_T("%LastName%"),
	_T("%e-mail%"),
	_T("%Nick%"),
	_T("%City%"),
	_T("%State%"),
	_T("%Phone%"),
	_T("%Homepage%"),
	_T("%About%")
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
	tstring sDebug = _T("Debug information\r\nsDBPath :");
	sDebug += sDBPath;
	sDebug += _T("\r\nsMirandaPath :");
	sDebug += sMirandaPath;
	sDebug += _T("\r\nsDefaultFile :");
	sDebug += sDefaultFile;

	sDebug += _T("\r\nGetFilePathFromUser(NULL) :");
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

int nGetFormatCount(const TCHAR *pszToCheck)
{
	if (!pszToCheck || pszToCheck[0] == 0)
		return 0;

	int nCount = 0;
	for (; pszToCheck[1] != 0; pszToCheck++)
	{
		if (pszToCheck[0] == '%' && pszToCheck[1] != '%')
			nCount++;
	}
	return nCount;
}

/////////////////////////////////////////////////////////////////////
// Member Function : CheckedTranslate
// Type            : Global
// Parameters      : szEng        - ?
//                   nFormatCount - ?
// Returns         : TCHAR *
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 030107, 07 January 2003
// Developer       : KN   
/////////////////////////////////////////////////////////////////////
/*
TCHAR *CheckedTranslate( const TCHAR *szEng, int nFormatCount )//= -1
{
TCHAR *szRet = TranslateTS( szEng);
if (szEng == szRet )
return (TCHAR*)szEng;

if (nFormatCount == -1 )
nFormatCount = nGetFormatCount( szEng);

if (nFormatCount != nGetFormatCount( szRet))
{
tstring sError = _T("The language pack you are using has an error in the transalation of\r\n");
sError += szEng;
sError += _T("\r\n---------------      It was translated to      ---------------\r\n");
sError += szRet;
MessageBox(NULL, sError.c_str(),MSG_BOX_TITEL,MB_OK);
return (TCHAR*)szEng;
}
return szRet;
}
*/
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
	ReplaceAll(ret, _T("\r"), _T(" "));
	ReplaceAll(ret, _T("\n"), _T(" "));
	ReplaceAll(ret, _T("  "), _T(" "));

	// Free the buffer.
	LocalFree(lpMsgBuf);
	return ret;
}

tstring sGetErrorString()
{
	return sGetErrorString(GetLastError());
}

void DisplayLastError(const TCHAR *pszError)
{
	tstring sError = pszError;
	DWORD error = GetLastError();

	TCHAR szTemp[50];
	mir_sntprintf(szTemp, SIZEOF(szTemp), _T("\r\nErrorCode: %d\r\n"), error);
	sError += szTemp;
	sError += sGetErrorString(error);
	MessageBox(NULL, sError.c_str(), MSG_BOX_TITEL, MB_OK);
}


/////////////////////////////////////////////////////////////////////
// Member Function : NickFromHandle
// Type            : Global
// Parameters      : hContact - ?
// Returns         : TCHAR*
// Description     : Reads a Nick from the database and returns a 
//                   pointer to this.
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

const TCHAR* NickFromHandle(MCONTACT hContact)
{
	const TCHAR *psz = (const TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
	if (psz)
		return psz;
	return pszNickError;
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

tstring _DBGetStringW(MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *pszError)
{
	tstring ret;
	DBVARIANT dbv = { 0 };
	//db_get
	if (!db_get_ws(hContact, szModule, szSetting, &dbv))
	{
		if (dbv.type != DBVT_WCHAR)
		{
			MessageBox(NULL, TranslateT("Database: Attempt to get wrong type of value, string"), MSG_BOX_TITEL, MB_OK);
			ret = pszError;
		}
		else
		{
			ret = (TCHAR*)dbv.pszVal;
		}
	}
	else
		ret = pszError;
	db_free(&dbv);
	return ret;
}

string _DBGetStringA(MCONTACT hContact, const char *szModule, const char *szSetting, const char *pszError)
{
	string ret;
	DBVARIANT dbv = { 0 };
	if (!db_get(hContact, szModule, szSetting, &dbv))
	{
		if (dbv.type != DBVT_ASCIIZ)
		{
			MessageBox(NULL, TranslateT("Database: Attempt to get wrong type of value, string"), MSG_BOX_TITEL, MB_OK);
			ret = pszError;
		}
		else
		{
			ret = dbv.pszVal;
		}
	}
	else
		ret = pszError;
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

void ReplaceAll(tstring &sSrc, const TCHAR *pszReplace, const tstring &sNew)
{
	string::size_type nCur = 0;
	while ((nCur = sSrc.find(pszReplace, nCur)) != sSrc.npos)
	{
		sSrc.replace(nCur, mir_tstrlen(pszReplace), sNew);
		nCur += sNew.size();
	}
}

void ReplaceAll(tstring &sSrc, const TCHAR *pszReplace, const TCHAR *pszNew)
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
	if (nPos != string::npos)
	{
		if (nPos + 1 < sFilePath.size())
			sFilePath.erase(nPos + 1);
	}
	else
	{
		// cant find \ 
		return false;
	}

	// create directory
	if (!CreateDirectory(sFilePath.c_str(), NULL))
	{
		DWORD dwE = GetLastError();
		if (dwE == 183) // Cannot create a file when that file already exists. 
			return true;
		if (!bCreatePathToFile(sFilePath.substr(0, nPos)))
			return false;

		// try again 
		if (!CreateDirectory(sFilePath.c_str(), NULL))
		{
			return false;
		}
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

static bool bWriteTextToFile(HANDLE hFile, const TCHAR *pszSrc, bool bUtf8File, int nLen = -1)
{
	if (nLen != -1) {
		TCHAR *tmp = (TCHAR*)alloca(sizeof(TCHAR)*(nLen + 1));
		mir_tstrncpy(tmp, pszSrc, nLen);
		tmp[nLen] = 0;
		pszSrc = tmp;
	}

	if (!bUtf8File) {
		// We need to downgrade text to ansi
		ptrA pszAstr(mir_t2a(pszSrc));
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
		mir_strncpy(tmp, pszSrc, nLen);
		tmp[nLen] = 0;
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
	BYTE * p = (BYTE*)pData;
	for (int n = 0; n < nSize; n++)
	{
		mir_snprintf(cBuf, SIZEOF(cBuf), "%.2X ", p[n]);
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
	TCHAR szDBPath[MAX_PATH], tmp[MAX_PATH];
	mir_tstrcpy(szDBPath, pszDbPathError);
	PathToAbsoluteT(_T("miranda32.exe"), tmp);
	sMirandaPath = tmp;
	sMirandaPath.erase(sMirandaPath.find_last_of(_T("\\")));
	CallService(MS_DB_GETPROFILEPATHT, (WPARAM)MAX_PATH - 1, (LPARAM)szDBPath);
	sDBPath = szDBPath;
	CallService(MS_DB_GETPROFILENAMET, (WPARAM)MAX_PATH - 1, (LPARAM)szDBPath);
	sDBPath.append(_T("\\")).append(szDBPath);
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
	ReplaceAll(sRet, _T("%dbpath%"), sDBPath);
	// Try to firure out if it is a relative path ( ..\..\MsgExport\ )
	if (sRet.size() <= 2 || !(sRet[1] == ':' ||
		(sRet[0] == '\\' && sRet[1] == '\\')))
	{
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

	bool bNickUsed = sFilePath.find(_T("%nick%")) != string::npos;

	ReplaceDefines(hContact, sFilePath);

	tstring sNoDBPath = sFilePath;

	ReplaceTimeVariables(sFilePath);
	ReplaceDBPath(sFilePath);

	// Previous file name check to see if it has changed !!
	tstring sPrevFileName = _DBGetString(hContact, MODULE, "PrevFileName", _T(""));
	if (sNoDBPath != sPrevFileName)
	{
		if (!sPrevFileName.empty())
		{
			ReplaceDBPath(sPrevFileName);

			// Here we will try to avoide the (Unknown Contact) in cases where the protocol for 
			// this user has been removed.
			if (bNickUsed && (_tcsstr(NickFromHandle(hContact), LPGENT("(Unknown Contact)")) != 0))
			{
				// Then the filename must have changed from a correct path to one including the (Unknown Contact)
				return sPrevFileName;
			}

			// file name has changed

			if (enRenameAction != eDANothing)
			{

				// we can not use FILE_SHARE_DELETE because this is not supported by 
				// win 98 / ME 
				HANDLE hPrevFile = CreateFile(sPrevFileName.c_str(),
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

				if (hPrevFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hPrevFile);
					TCHAR szTemp[500];
					// There is a previous file we can move 
					// ask user ?
					bool bTryRename;

					if (enRenameAction != eDAAutomatic)
					{
						tstring sRemoteUser = NickFromHandle(hContact);
						mir_sntprintf(szTemp, SIZEOF(szTemp),
							TranslateT("File name for the user \"%s\" has changed!\n\nfrom:\t%s\nto:\t%s\n\nDo you wish to rename file?"),
							sRemoteUser.c_str(),
							sPrevFileName.c_str(),
							sFilePath.c_str());
						bTryRename = MessageBox(NULL, szTemp, MSG_BOX_TITEL, MB_YESNO) == IDYES;
					}
					else
						bTryRename = true;


					if (bTryRename)
					{
						if (!MoveFile(sPrevFileName.c_str(), sFilePath.c_str()))
						{
							// this might be because the new path isent created 
							// so we will try to create it 
							bCreatePathToFile(sFilePath);

							while (!MoveFile(sPrevFileName.c_str(), sFilePath.c_str()))
							{
								mir_sntprintf(szTemp, SIZEOF(szTemp),
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
		db_set_ts(hContact, MODULE, "PrevFileName", sNoDBPath.c_str());
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
	tstring ret = NickFromHandle(hContact);
	string::size_type nCur = 0;
	while ((nCur = ret.find_first_of(_T(":\\"), nCur)) != ret.npos)
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

void ReplaceAllNoColon(tstring &sSrc, const TCHAR *pszReplace, tstring &sNew)
{
	tstring::size_type nCur = 0;
	while ((nCur = sNew.find_first_of(_T(':'), nCur)) != sNew.npos)
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
	if (sTarget.find(_T("%nick%")) != string::npos)
	{
		ReplaceAll(sTarget, _T("%nick%"), FileNickFromHandle(hContact));
	}

	bool bUINUsed = sTarget.find(_T("%UIN%")) != string::npos;
	bool bEMailUsed = sTarget.find(_T("%e-mail%")) != string::npos;
	bool bProtoUsed = sTarget.find(_T("%protocol%")) != string::npos;
	bool bIdentifierUsed = sTarget.find(_T("%identifier%")) != string::npos;

	if (bUINUsed || bEMailUsed || bProtoUsed || bIdentifierUsed)
	{
		string sProto = _DBGetStringA(hContact, "Protocol", "p", "");
		if (bUINUsed || (bIdentifierUsed && sProto == "ICQ"))
		{
			DWORD dwUIN = db_get_dw(hContact, sProto.c_str(), "UIN", 0);
			tstring sReplaceUin;
			if (dwUIN)
			{
				TCHAR sTmp[20];
				mir_sntprintf(sTmp, SIZEOF(sTmp), _T("%d"), dwUIN);
				sReplaceUin = sTmp;
			}
			else
			{
				sReplaceUin = FileNickFromHandle(hContact);
			}

			if (bUINUsed)
				ReplaceAll(sTarget, _T("%UIN%"), sReplaceUin);
			if (bIdentifierUsed && sProto == "ICQ")
			{
				bIdentifierUsed = false;
				ReplaceAll(sTarget, _T("%identifier%"), sReplaceUin);
			}
		}

		if (bEMailUsed || (bIdentifierUsed && sProto == "MSN"))
		{
			tstring sEMail = _DBGetString(hContact, sProto.c_str(), "e-mail", _T(""));
			if (sEMail.empty())
			{
				sEMail = _DBGetString(hContact, "MSN", "e-mail", _T(""));
				if (sEMail.empty())
				{
					// We can't finde the E-mail address we will use the the nick
					sEMail = FileNickFromHandle(hContact);
				}
			}
			if (bEMailUsed)
				ReplaceAllNoColon(sTarget, _T("%e-mail%"), sEMail);
			if (bIdentifierUsed && sProto == "MSN")
			{
				bIdentifierUsed = false;
				ReplaceAllNoColon(sTarget, _T("%identifier%"), sEMail);
			}
		}
		if (bIdentifierUsed && sProto == "Jabber")
		{
			tstring sReplace = _DBGetString(hContact, "Jabber", "jid", _T(""));
			if (sReplace.empty())
			{
				sReplace = FileNickFromHandle(hContact);
			}
			bIdentifierUsed = false;
			ReplaceAll(sTarget, _T("%identifier%"), sReplace);
		}
		if (bProtoUsed)
		{
			tstring tmp = _DBGetString(hContact, "Protocol", "p", _T(""));
			ReplaceAllNoColon(sTarget, _T("%protocol%"), tmp);
		}
		if (bIdentifierUsed)
		{
			// It has still not been replaced we will just use nick
			ReplaceAll(sTarget, _T("%nick%"), FileNickFromHandle(hContact));
		}
	}

	if (sTarget.find(_T("%group%")) != string::npos)
	{
		tstring sGroup = _DBGetString(hContact, "CList", "Group", _T(""));
		ReplaceAllNoColon(sTarget, _T("%group%"), sGroup);
	}

	// We can't replace the : here because if the user uses C:\... in the file path 
	// this will also be replaced 
	string::size_type nCur = 0;
	while ((nCur = sTarget.find_first_of(_T("/*?<>|\""), nCur)) != sTarget.npos)
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
	if (sRet.find(_T("%year%")) != string::npos ||
		sRet.find(_T("%month%")) != string::npos ||
		sRet.find(_T("%day%")) != string::npos)
	{
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);
		TCHAR sTmp[20];

		mir_sntprintf(sTmp, SIZEOF(sTmp), _T("%d"), stTime.wYear);
		ReplaceAll(sRet, _T("%year%"), sTmp);
		mir_sntprintf(sTmp, SIZEOF(sTmp), _T("%.2d"), stTime.wMonth);
		ReplaceAll(sRet, _T("%month%"), sTmp);
		mir_sntprintf(sTmp, SIZEOF(sTmp), _T("%.2d"), stTime.wDay);
		ReplaceAll(sRet, _T("%day%"), sTmp);
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
		tstring sNick = NickFromHandle(hContact);
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

void DisplayErrorDialog(const TCHAR *pszError, tstring& sFilePath, DBEVENTINFO * dbei)
{
	tstring sError = TranslateTS(pszError);
	sError += sFilePath;
	sError += TranslateT("\nError: ");
	sError += sGetErrorString();
	sError += TranslateT("\nMessage has not been saved!\n");
	sError += TranslateT("Do you wish to save debug information?");
	if (MessageBox(NULL, sError.c_str(), MSG_BOX_TITEL, MB_YESNO) == IDYES)
	{
		OPENFILENAME ofn;       // common dialog box structure
		TCHAR szFile[260];       // buffer for file name
		mir_tstrcpy(szFile, _T("DebugInfo.txt"));

		// Initialize OPENFILENAME
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		//ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = SIZEOF(szFile);
		ofn.lpstrFilter = TranslateT("All\0*.*\0Text\0*.TXT\0\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = 0 /*OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST*/;
		ofn.lpstrDefExt = _T("TXT");

		// Display the Open dialog box. 

		if (GetSaveFileName(&ofn))
		{
			HANDLE hf = CreateFile(ofn.lpstrFile, GENERIC_WRITE,
				0, (LPSECURITY_ATTRIBUTES)NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				(HANDLE)NULL); // file handle

			bWriteTextToFile(hf, sError.c_str(), false);
			if (dbei)
			{
				bWriteToFile(hf, "\r\ndbei          :");

				bWriteHexToFile(hf, dbei, sizeof(DBEVENTINFO));
				if (dbei->pBlob)
				{
					bWriteToFile(hf, "\r\ndbei.pBlob    :");
					bWriteHexToFile(hf, dbei->pBlob, min(dbei->cbBlob, 10000));
				}
				if (dbei->szModule)
				{
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
// Parameters      : hContact - handle to contact
//                   dbei     - Event to export
// Returns         : void
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 050429, 29 april 2005
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

void ExportDBEventInfo(MCONTACT hContact, DBEVENTINFO &dbei)
{
	TCHAR szTemp[500];
	tstring sFilePath = GetFilePathFromUser(hContact);

	GetLastError();// Clear last error !!

	HANDLE hFile = CreateFile(sFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// this might be because the path isent created 
		// so we will try to create it 
		if (bCreatePathToFile(sFilePath))
		{
			hFile = CreateFile(sFilePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}
	}

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DisplayErrorDialog(LPGENT("Failed to open or create file :\n"), sFilePath, NULL);
		return;
	}

	tstring sLocalUser;
	tstring sRemoteUser;
	string::size_type nFirstColumnWidth;

	if (bUseLessAndGreaterInExport)
	{
		sLocalUser = _T("<<");
		sRemoteUser = _T(">>");
		nFirstColumnWidth = 4;
	}
	else
	{
		sLocalUser = NickFromHandle(0);
		sRemoteUser = NickFromHandle(hContact);
		nFirstColumnWidth = max(sRemoteUser.size(), clFileTo1ColWidth[sFilePath]);
		nFirstColumnWidth = max(sLocalUser.size(), nFirstColumnWidth);
		nFirstColumnWidth += 2;
	}

	bool bWriteUTF8Format = false;

	{
		DWORD dwLowSize;
		DWORD dwHighSize = 0;

		dwLowSize = GetFileSize(hFile, &dwHighSize);

		if (dwLowSize == INVALID_FILE_SIZE || dwLowSize != 0 || dwHighSize != 0)
		{
			DWORD dwDataRead = 0;
			BYTE ucByteOrder[3];
			if (ReadFile(hFile, ucByteOrder, 3, &dwDataRead, NULL))
			{
				bWriteUTF8Format = bIsUtf8Header(ucByteOrder);
			}
			DWORD dwPtr = SetFilePointer(hFile, 0, 0, FILE_END);
			if (dwPtr == INVALID_SET_FILE_POINTER)
			{
				// we need to aborte mission here because if we continue we risk 
				// overwriting old log.
				DisplayErrorDialog(LPGENT("Failed to move to the end of the file :\n"), sFilePath, NULL);
				CloseHandle(hFile);
				return;
			}
		}
		else
		{
			bWriteUTF8Format = bUseUtf8InNewFiles;
			if (bWriteUTF8Format)
			{
				if (!bWriteToFile(hFile, szUtf8ByteOrderHeader, sizeof(szUtf8ByteOrderHeader) - 1))
				{
					DisplayErrorDialog(LPGENT("Failed to UTF8 byte order code to file :\n"), sFilePath, NULL);
					CloseHandle(hFile);
					return;
				}
			}
			tstring output = _T("------------------------------------------------\r\n")
				LPGENT("      History for\r\n")
				LPGENT("User      : %User%\r\n")
				LPGENT("Protocol  : %Proto%\r\n")
				LPGENT("UIN       : %UIN%\r\n")
				LPGENT("FirstName : %FirstName%\r\n")
				LPGENT("LastName  : %LastName%\r\n")
				LPGENT("Age       : %Age%\r\n")
				LPGENT("Gender    : %Gender%\r\n")
				LPGENT("e-mail    : %e-mail%\r\n")
				LPGENT("Nick      : %Nick%\r\n")
				LPGENT("City      : %City%\r\n")
				LPGENT("State     : %State%\r\n")
				LPGENT("Phone     : %Phone%\r\n")
				LPGENT("Homepage  : %Homepage%\r\n")
				LPGENT("- About -\r\n%About%\r\n")
				_T("------------------------------------------------\r\n");

			// This is written this way because I expect this will become a string the user may set 
			// in the options dialog.
			ReplaceAll(output, _T("%User%"), sRemoteUser);

			string sProto = _DBGetStringA(hContact, "Protocol", "p", "");
			ReplaceAll(output, _T("%Proto%"), _DBGetString(hContact, "Protocol", "p", _T("")));

			for (int nCur = 0; nCur < 9; nCur++)
			{
				ReplaceAll(output, pszReplaceList[nCur], _DBGetString(hContact, sProto.c_str(), pszReplaceListA[nCur], _T("")));
			}

			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), db_get_dw(hContact, sProto.c_str(), "UIN", 0));
			ReplaceAll(output, _T("%UIN%"), szTemp);

			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), db_get_w(hContact, sProto.c_str(), "Age", 0));
			ReplaceAll(output, _T("%Age%"), szTemp);

			szTemp[0] = (TCHAR)db_get_b(hContact, sProto.c_str(), "Gender", 0);
			szTemp[1] = 0;
			ReplaceAll(output, _T("%Gender%"), szTemp);

			if (!bWriteTextToFile(hFile, output.data(), bWriteUTF8Format, (int)output.size()))
			{
				DisplayErrorDialog(LPGENT("Failed to write user details to file :\n"), sFilePath, NULL);
				CloseHandle(hFile);
				return;
			}
		}
	}

	int nIndent;
	{  // Get time stamp 

		nIndent = mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%-*s"),
			nFirstColumnWidth,
			dbei.flags & DBEF_SENT ? sLocalUser.c_str() : sRemoteUser.c_str());

		DBTIMETOSTRINGT dbtts;
		dbtts.cbDest = sizeof(szTemp) - nIndent - 2;
		dbtts.szDest = &szTemp[nIndent];
		dbtts.szFormat = (TCHAR*)sTimeFormat.c_str();

		CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, dbei.timestamp, (LPARAM)&dbtts);

		nIndent = (int)mir_tstrlen(szTemp);
		szTemp[nIndent++] = ' ';

		// Write first part of line with name and timestamp
		if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, nIndent))
		{
			DisplayErrorDialog(LPGENT("Failed to write timestamp and username to file :\n"), sFilePath, &dbei);
			CloseHandle(hFile);
			return;
		}
	}

	if (dbei.pBlob != NULL && dbei.cbBlob >= 2)
	{
		dbei.pBlob[dbei.cbBlob] = 0;

		switch (dbei.eventType)
		{
		case EVENTTYPE_MESSAGE:
		{
			TCHAR* msg = DbGetEventTextT(&dbei, CP_ACP);
			if (!bWriteIndentedToFile(hFile, nIndent, msg, bWriteUTF8Format))
			{
				DisplayErrorDialog(LPGENT("Failed to write message to the file :\n"), sFilePath, &dbei);
			}
			mir_free(msg);
			break;
			/*
							const char *pszData = (const char*)dbei.pBlob;
							bool bConvertedToUtf8 = false;
							if (bWriteUTF8Format )// Write UTF-8 format in file ?
							{
							int nAnsiLen = mir_strlen((char *) dbei.pBlob)+1;
							if (nAnsiLen < (int)dbei.cbBlob )
							{
							// Message is also encoded in unicode UTF-16/UCS-2, little endian.
							if (WideCharToMultiByte( CP_UTF8, 0, (wchar_t*)&dbei.pBlob[ nAnsiLen ], nAnsiLen, szTemp, sizeof(szTemp), 0, 0))
							{
							pszData = szTemp;
							bConvertedToUtf8 = true;
							}
							}
							// We need to write in UTF8 format so we have to convert ansi string to UTF8
							}
							if ( !bWriteIndentedToFile( hFile, nIndent, pszData, bWriteUTF8Format))
							{
							DisplayErrorDialog( _T("Failed to write message to the file :\n"), sFilePath, &dbei);
							}
							break;*/
		}
		case EVENTTYPE_URL:
		case EVENTTYPE_FILE:
		{
			const TCHAR *pszType;
			const char *pszData;

			if (dbei.eventType == EVENTTYPE_URL)
			{
				pszType = LPGENT("URL: ");
				pszData = (char *)dbei.pBlob;
			}
			else
			{
				pszType = LPGENT("File: ");
				pszData = (char *)(dbei.pBlob + sizeof(DWORD));
			}

			bool bWriteOk = false;

			int nLen = (int)mir_strlen(pszData);
			if ((pszData - (char *)dbei.pBlob) + nLen < (int)dbei.cbBlob)
			{
				if (bWriteTextToFile(hFile, pszType, bWriteUTF8Format) &&
					bWriteIndentedToFile(hFile, nIndent, pszData, bWriteUTF8Format))
				{
					pszData += nLen + 1;
					if ((pszData - (char *)dbei.pBlob) >= (int)dbei.cbBlob)
					{
						bWriteOk = true;
					}
					else
					{
						nLen = (int)mir_strlen(pszData);
						if ((pszData - (char *)dbei.pBlob) + nLen < (int)dbei.cbBlob)
						{
							if (bWriteNewLine(hFile, nIndent) &&
								bWriteTextToFile(hFile, LPGENT("Description: "), bWriteUTF8Format) &&
								bWriteIndentedToFile(hFile, nIndent, pszData, bWriteUTF8Format))
							{
								bWriteOk = true;
							}
						}
					}
				}
			}

			if (!bWriteOk)
				DisplayErrorDialog(LPGENT("Failed to write URL/File to the file :\n"), sFilePath, &dbei);
			break;
		}
		case EVENTTYPE_AUTHREQUEST:
		case EVENTTYPE_ADDED:
		{
			const TCHAR *pszTypes[] = {
				LPGENT("Nick      :"),
				LPGENT("FirstName :"),
				LPGENT("LastName  :"),
				LPGENT("e-mail    :"),
				LPGENT("Reason    :") };

			if (dbei.cbBlob < 8 || dbei.cbBlob > 5000)
			{
				int n = mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Invalid Database event received. Type %d, size %d"), dbei.eventType, dbei.cbBlob);
				if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n))
					DisplayErrorDialog(LPGENT("Failed to write Invalid Database event the file :\n"), sFilePath, &dbei);
				break;
			}

			bool bWriteOk = false;

			int nStringCount;
			const TCHAR *pszTitle;
			char *pszCurBlobPos;
			if (dbei.eventType == EVENTTYPE_AUTHREQUEST)
			{	// request 
				//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
				nStringCount = 5;
				pszCurBlobPos = (char *)dbei.pBlob + sizeof(DWORD) * 2;
				pszTitle = LPGENT("The following user made an authorization request:");
			}
			else
			{  // Added
				//blob is: uin(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
				pszCurBlobPos = (char *)dbei.pBlob + sizeof(DWORD);
				nStringCount = 4;
				pszTitle = LPGENT("The following user added you to their contact list:");
			}

			if (bWriteTextToFile(hFile, pszTitle, bWriteUTF8Format) &&
				bWriteNewLine(hFile, nIndent) &&
				bWriteTextToFile(hFile, LPGENT("UIN       :"), bWriteUTF8Format))
			{
				DWORD uin = *((PDWORD)(dbei.pBlob));
				int n = mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), uin);
				if (bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n))
				{
					char *pszEnd = (char *)(dbei.pBlob + dbei.cbSize);
					for (int n = 0; n < nStringCount && pszCurBlobPos < pszEnd; n++)
					{
						if (*pszCurBlobPos)
						{
							if (!bWriteNewLine(hFile, nIndent) ||
								!bWriteTextToFile(hFile, TranslateTS(pszTypes[n]), bWriteUTF8Format) ||
								!bWriteIndentedToFile(hFile, nIndent, pszCurBlobPos, bWriteUTF8Format))
							{
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
				DisplayErrorDialog(LPGENT("Failed to write AUTHREQUEST or ADDED to the file :\n"), sFilePath, &dbei);

			break;
		}
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
				bWriteTextToFile(hFile, LPGENT("EmailExpress from:"), bWriteUTF8Format);
			else
				bWriteTextToFile(hFile, LPGENT("WebPager from:"), bWriteUTF8Format);

			bWriteNewLine(hFile, nIndent);

			size_t nMsgLenght = mir_strlen(pszStr) + 1;
			if (nMsgLenght < dbei.cbBlob)
			{
				size_t nFriendlyLen = mir_strlen(&pszStr[nMsgLenght]);
				bWriteTextToFile(hFile, &pszStr[nMsgLenght], bWriteUTF8Format, (int)nFriendlyLen);
				size_t nEmailOffset = nMsgLenght + nFriendlyLen + 1;
				if (nEmailOffset < dbei.cbBlob)
				{
					bWriteTextToFile(hFile, _T("<"), bWriteUTF8Format);
					size_t nEmailLen = mir_strlen(&pszStr[nEmailOffset]);
					bWriteTextToFile(hFile, &pszStr[nEmailOffset], bWriteUTF8Format, (int)nEmailLen);
					bWriteTextToFile(hFile, _T(">"), bWriteUTF8Format);
				}
			}
			else
			{
				bWriteTextToFile(hFile, LPGENT("No from address"), bWriteUTF8Format);
			}

			if (!bWriteNewLine(hFile, nIndent) ||
				!bWriteIndentedToFile(hFile, nIndent, pszStr, bWriteUTF8Format))
			{
				DisplayErrorDialog(LPGENT("Failed to write EmailExpress to the file :\n"), sFilePath, &dbei);
			}
			break;
		}
		case ICQEVENTTYPE_SMS:
		{
			if (!bWriteIndentedToFile(hFile, nIndent, (const char*)dbei.pBlob, bWriteUTF8Format))
			{
				DisplayErrorDialog(LPGENT("Failed to write SMS to the file :\n"), sFilePath, &dbei);
			}
			break;
		}
		default:
		{
			int n = mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
			if (!bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n))
			{
				DisplayErrorDialog(LPGENT("Failed to write Unknown event to the file :\n"), sFilePath, &dbei);
			}
			break;
		}
		}
	}
	else
	{
		int n = mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Unknown event type %d, size %d"), dbei.eventType, dbei.cbBlob);
		bWriteTextToFile(hFile, szTemp, bWriteUTF8Format, n);
	}
	bWriteToFile(hFile, bAppendNewLine ? "\r\n\r\n" : "\r\n");

	CloseHandle(hFile);

	UpdateFileViews(sFilePath.c_str());
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


int nExportEvent(WPARAM hContact, LPARAM hDbEvent)
{
	if (!db_get_b(hContact, MODULE, "EnableLog", 1))
		return 0;

	DBEVENTINFO dbei = { sizeof(dbei) };
	int nSize = db_event_getBlobSize(hDbEvent);
	if (nSize > 0) {
		dbei.cbBlob = nSize;
		dbei.pBlob = (PBYTE)malloc(dbei.cbBlob + 2);
		dbei.pBlob[dbei.cbBlob] = 0;
		dbei.pBlob[dbei.cbBlob + 1] = 0;
		// Double null terminate, this shut pervent most errors 
		// where the blob received has an invalid format
	}

	if (!db_event_get(hDbEvent, &dbei)) {
		char szTemp[500];
		mir_snprintf(szTemp, SIZEOF(szTemp), "DisableProt_%s", dbei.szModule);
		if (db_get_b(NULL, MODULE, szTemp, 1))
			ExportDBEventInfo(hContact, dbei);
	}
	if (dbei.pBlob)
		free(dbei.pBlob);
	return 0;
}

#ifdef _UNICODE
bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const char *pszSrc, bool bUtf8File)
{
	int nLen = (int)mir_strlen(pszSrc);
	wchar_t * pszWstr = new wchar_t[nLen + 1];
	bool bRet = false;
	if (MultiByteToWideChar(CP_ACP, 0, pszSrc, nLen, pszWstr, nLen) == nLen)
	{
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

bool bWriteIndentedToFile(HANDLE hFile, int nIndent, const TCHAR *pszSrc, bool bUtf8File)
{
	if (pszSrc == NULL)
		return true;

	bool bOk = true;
	bool bFirstLine = true;

	while (*pszSrc)
	{	// first we will scan forward in string to finde either new line or "max line with"
		int nLineLen = 0;
		do
		{
			if (pszSrc[nLineLen] == _T('\n') || pszSrc[nLineLen] == _T('\r'))
				break;

			if (nLineLen >= nMaxLineWidth)
			{	// ok the line was not broken. we need to force a break
				// we will scan backwards again to finde a space !!
				// then we will look for a ? and so on.

				const TCHAR ac[] = { _T(' '), _T('?'), _T('-'), _T('.'), _T(',') };
				for (int y = 0; y < SIZEOF(ac); y++)
				{
					for (int n = nLineLen; n > 0; n--)
					{
						if (pszSrc[n] == ac[y])
						{
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
		if (nLineLen > 0)
		{
			while (pszSrc[nLineLen - 1] == ' ')
				nLineLen--;
		}

	SuperBreak:


		// nLineLen should contain the number af chars we need to write to the file 
		if (nLineLen > 0)
		{
			if (!bFirstLine)
			{
				if (!bWriteNewLine(hFile, nIndent))
				{
					bOk = false;
				}
			}
			/*			if (bUtf8Src )
						{
						// Programming error writing UTF8 string to ansi file
						if ( !bUtf8File )
						{
						MessageBox(NULL, _T("Programming error writing UTF8 string to ansi file") ,MSG_BOX_TITEL,MB_OK);
						// bUtf8File must be true here
						}
						if (!bWriteToFile( hFile, pszSrc, nLineLen))
						{
						bOk = false;
						}
						}
						else*/
			{// Text format !!
				if (!bWriteTextToFile(hFile, pszSrc, bUtf8File, nLineLen))
					bOk = false;
			}
		}
		bFirstLine = false;

		// skip any noice chars, MAC style '\r' '\r' '\n' 
		// and excess spaces 
		const TCHAR *pszPrev = pszSrc;
		pszSrc += nLineLen;
		while (*pszSrc == _T(' ') || *pszSrc == _T('\n') || *pszSrc == _T('\r'))
			pszSrc++;

		if (pszPrev == pszSrc)
		{
			// this is an programming error we have not moved forward in string 
			MessageBox(NULL, _T("Programming error on line __LINE__ please report this"), MSG_BOX_TITEL, MB_OK);
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

	if (hPrevFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPrevFile);

		TCHAR szTemp[500];
		mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s\r\n%s"),
			TranslateT("User has been deleted. Do you want to delete the file?"), sFilePath.c_str());

		if (enDeleteAction == eDAAutomatic ||
			MessageBox(NULL, szTemp, MSG_BOX_TITEL, MB_YESNO) == IDYES)
		{
			if (!DeleteFile(sFilePath.c_str()))
			{
				mir_sntprintf(szTemp, SIZEOF(szTemp),
					_T("%s\r\n%s"),
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
	db_set_ts(NULL, MODULE, "ExportDir", sExportDir.c_str());
	db_set_ts(NULL, MODULE, "DefaultFile", sDefaultFile.c_str());
	db_set_ts(NULL, MODULE, "TimeFormat", sTimeFormat.c_str());

	db_set_ts(NULL, MODULE, "FileViewerPrg", sFileViewerPrg.c_str());
	db_set_b(NULL, MODULE, "UseInternalViewer", bUseInternalViewer());
	db_set_b(NULL, MODULE, "ReplaceHistory", bReplaceHistory);
	db_set_b(NULL, MODULE, "AppendNewLine", bAppendNewLine);
	db_set_b(NULL, MODULE, "UseUtf8InNewFiles", bUseUtf8InNewFiles);
	db_set_b(NULL, MODULE, "UseLessAndGreaterInExport", bUseLessAndGreaterInExport);

	db_set_b(NULL, MODULE, "RenameAction", (BYTE)enRenameAction);
	db_set_b(NULL, MODULE, "DeleteAction", (BYTE)enDeleteAction);
}

