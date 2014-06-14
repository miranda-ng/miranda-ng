//This file is part of HTTPServer a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen
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

static const char szXmlHeader1[] =	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n"
    "<?xml-stylesheet type=\"text/xsl\" href=\"";

static const char szXmlHeader2[] =	"\"?>\r\n"
    "<config>\r\n";

static const char szXmlTail[] =	"</config>";


static void ReplaceSign(TCHAR *pszSrc, int MaxLength, const TCHAR pszReplace, const TCHAR *pszNew)
{
	static TCHAR szBuffer[1024];
	TCHAR *pszSign = _tcschr(pszSrc, pszReplace);

	if (pszSign) {
		_tcscpy(szBuffer, pszSrc);

		do {
			_tcscpy(szBuffer + (pszSign - pszSrc), pszNew);
			_tcscpy(szBuffer + (pszSign - pszSrc) + _tcslen(pszNew), pszSign + 1);
			*pszSign = ' ';

		} while (pszSign = _tcschr(pszSrc, pszReplace));

		_tcsncpy(pszSrc, szBuffer, MaxLength);

		pszSrc[MaxLength-1] = '\0';
	}
}


/////////////////////////////////////////////////////////////////////
// Member Function : bCreateIndexXML
// Type            : Global
// Parameters      : pszRealPath - ?
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 050821, 05 august 2005
// Developer       : Houdini
/////////////////////////////////////////////////////////////////////

bool bCreateIndexXML(const TCHAR *pszRealPath, const TCHAR *pszIndexPath, const TCHAR *pszSrvPath, DWORD dwRemoteIP)
{
	TCHAR szMask[MAX_PATH+1];
	_tcscpy(szMask, pszRealPath);
	_tcscat(szMask, _T("*"));

	WIN32_FIND_DATA fdFindFileData;
	HANDLE hFind = FindFirstFile(szMask, &fdFindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	HANDLE hFile = CreateFile(pszIndexPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return FALSE;
	}

	const int BUFFER_SIZE = 1000;
	TCHAR  szBuffer[BUFFER_SIZE+1];
	TCHAR *pszBuffer = szBuffer;
	TCHAR  szFileName[MAX_PATH+1] = _T("");
	TCHAR* pszExt;
	DWORD dwBytesWritten = 0;

	// Generate Dirname
	_tcscpy(szBuffer, pszSrvPath);
	TCHAR *pszTemp = _tcsrchr(szBuffer, '/');
	if (pszTemp)
		*pszTemp = '\0';

	pszTemp = _tcsrchr(szBuffer, '/');
	if (pszTemp)
		_tcscpy(szFileName, pszTemp + 1);

	// Write Header
	WriteFile(hFile, szXmlHeader1, sizeof(szXmlHeader1) - 1, &dwBytesWritten, NULL);

	// check if a index.xsl exists in the same directory otherwise use the global
	_tcscpy(szMask, pszRealPath);
	_tcscat(szMask, _T("index.xsl"));

	HANDLE hFileExists = CreateFile(szMask, GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFileExists == INVALID_HANDLE_VALUE) {
		_tcscpy(szBuffer, _T("/index.xsl"));
	} else {
		CloseHandle(hFileExists);
		_tcscpy(szBuffer, _T("index.xsl"));
	}

	WriteFile(hFile, szBuffer, (DWORD)_tcslen(szBuffer), &dwBytesWritten, NULL);

	WriteFile(hFile, szXmlHeader2, sizeof(szXmlHeader2) - 1, &dwBytesWritten, NULL);

	// Write dirname
	ReplaceSign(szFileName, MAX_PATH, '&', _T("&amp;"));
	pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
		_T("  <dirname>%s</dirname>\r\n"), szFileName);
	WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, NULL);

	// Find files and directories
	do {
		if (_tcscmp(fdFindFileData.cFileName, _T(".")) &&
			_tcsncmp(fdFindFileData.cFileName, _T("@"), _tcslen(_T("@"))) &&
			(_tcscmp(fdFindFileData.cFileName, _T("..")) || _tcscmp(pszSrvPath, _T("/")))) { // hide .. in root
			pszBuffer = szBuffer;

			_tcscpy(szFileName, fdFindFileData.cFileName);
			ReplaceSign(szFileName, MAX_PATH, '&', _T("&amp;"));

			if (fdFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
					_T("  <item name=\"%s\" isdir=\"true\"/>\r\n"), szFileName);
			} else {
				pszExt = _tcsrchr(szFileName, '.');

				if (pszExt != NULL) {
					*pszExt = '\0';
					pszExt++;
				}

				pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
					_T("  <item name=\"%s\" ext=\"%s\" size=\"%i\" "),
				  szFileName, (pszExt == NULL) ? _T("") : pszExt, fdFindFileData.nFileSizeLow);

				SYSTEMTIME systemTime;
				FileTimeToSystemTime(&fdFindFileData.ftCreationTime, &systemTime);
				pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
					_T("created=\"%i/%02i/%02i %i:%02i:%02i\" "),
					systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
					systemTime.wMinute, systemTime.wSecond);

				FileTimeToSystemTime(&fdFindFileData.ftLastWriteTime, &systemTime);
				pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
					_T("modified=\"%i/%02i/%02i %i:%02i:%02i\" "),
					systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
					systemTime.wMinute, systemTime.wSecond);

				pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
					_T("/>\r\n"));
			}

			if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, NULL))
				break;
		}

	} while (FindNextFile(hFind, &fdFindFileData));

	if (hFind != 0)
		FindClose(hFind);

	// Add other shared files & directories
	for (CLFileShareNode * pclCur = pclFirstNode; pclCur ; pclCur = pclCur->pclNext) {
		if (!((pclCur->st.dwAllowedIP ^ dwRemoteIP) & pclCur->st.dwAllowedMask) &&  // hide inaccessible shares
		    (size_t)(pclCur->nGetSrvPathLen()) > _tcslen(pszSrvPath) &&
		    !_tcsstr(pclCur->st.pszRealPath, _T("\\@")) &&
		    !_tcsncmp(pclCur->st.pszSrvPath, pszSrvPath, _tcslen(pszSrvPath))) {
			pszBuffer = szBuffer;

			_tcscpy(szFileName, &pclCur->st.pszSrvPath[_tcslen(pszSrvPath)]);
			ReplaceSign(szFileName, MAX_PATH, '&', _T("&amp;"));

			if (pclCur->bIsDirectory()) {
				szFileName[_tcslen(szFileName)-1] = '\0';
				if (!_tcschr(szFileName, '/')) { // only one level deeper
					pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
						_T("  <item name=\"%s\" isdir=\"true\"/>\r\n"), szFileName);

					if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, NULL))
						break;
				}
			} else {
				if (!_tcschr(szFileName, '/') &&   // only one level deeper
				    _tcsncmp(pszRealPath, pclCur->st.pszRealPath, _tcslen(pszRealPath))) { // no duplicates
					pszExt = _tcsrchr(szFileName, '.');

					if (pszExt != NULL) {
						*pszExt = '\0';
						pszExt++;
					}

					DWORD dwFileSize = 0;
					FILETIME ftFileCreateTime;
					FILETIME ftFileAccessTime;
					FILETIME ftFileModifyTime;
					HANDLE hFileS = CreateFile(pclCur->st.pszRealPath, GENERIC_READ, 
						FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFileS != INVALID_HANDLE_VALUE) {
						dwFileSize = GetFileSize(hFileS, NULL);
						GetFileTime(hFileS, &ftFileCreateTime, &ftFileAccessTime, &ftFileModifyTime);
						CloseHandle(hFileS);
					}

					pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
						_T("  <item name=\"%s\" ext=\"%s\" size=\"%i\" "),
						szFileName, (pszExt == NULL) ? _T("") : pszExt, dwFileSize);

					SYSTEMTIME systemTime;
					FileTimeToSystemTime(&ftFileCreateTime, &systemTime);
					pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
						_T("created=\"%i/%02i/%02i %i:%02i:%02i\" "),
						systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
						systemTime.wMinute, systemTime.wSecond);

					FileTimeToSystemTime(&ftFileModifyTime, &systemTime);
					pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
						_T("modified=\"%i/%02i/%02i %i:%02i:%02i\" "),
						systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
						systemTime.wMinute, systemTime.wSecond);

					pszBuffer += mir_sntprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer), 
						_T("/>\r\n"));

					if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, NULL))
						break;
				}
			}
		}
	}

	WriteFile(hFile, szXmlTail, sizeof(szXmlTail) - 1, &dwBytesWritten, NULL);

	SetEndOfFile(hFile);
	CloseHandle(hFile);

	return TRUE;
}