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

eIndexCreationMode indexCreationMode;

static TCHAR *szIndexHTMLTemplate = NULL;
static const int MAX_PARAM_LENGTH = 5;

// signs below 32 are not used anyway
enum Symbol {
	SY_END = 15,
	SY_FOR_DIRS,
	SY_DIR_URL,
	SY_DIR_NAME,
	SY_FOR_FILES,
	SY_FILE_URL,
	SY_FILE_NAME,
	SY_FILE_SIZE,
	SY_FILE_CREATE_TIME,
	SY_FILE_MODIFY_TIME,
	SY_IS_EVEN,
	SY_IS_ODD,
	SY_IS_FILE_TYPE
};

/////////////////////////////////////////////////////////////////////
// Member Function : LoadIndexHTMLTemplate
// Type            : Global
// Parameters      : -
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 050901, 05 august 2005
// Developer       : Houdini
/////////////////////////////////////////////////////////////////////

bool LoadIndexHTMLTemplate() {
	if (szIndexHTMLTemplate != NULL)
		return true;

	TCHAR szBuf[10000];
	TCHAR *pszBuf = szBuf;

	TCHAR szDestBuf[10000];
	TCHAR *pszDestBuf = szDestBuf;

	_tccpy(pszBuf, szPluginPath);
	_tcscat(pszBuf, szIndexHTMLTemplateFile);

	HANDLE hFile = CreateFile(pszBuf, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
	    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, _T("HTTPServerIndex.html not found in Plugin Path"), MSG_BOX_TITEL, MB_OK);
		return false;
	}

	DWORD dwBytesRead = 0;
	if (ReadFile(hFile, pszBuf, SIZEOF(szBuf), &dwBytesRead, NULL) || dwBytesRead <= 0) {
		while (dwBytesRead > (DWORD)(pszBuf - szBuf)) {
			if (*pszBuf == '[') {
				TCHAR* pszKeywordBegin = pszBuf + 1;
				bool  bHasParameters = false;

				do {
					pszBuf++;
				} while (*pszBuf != ']' && *pszBuf != '\0' && *pszBuf != '=');

				if (*pszBuf == '\0')
					break;

				if (*pszBuf == '=')
					bHasParameters = true;

				*pszBuf = '\0';
				*pszDestBuf = '#';

				// signs below 32 are not used anyway
				if (!_tcscmp(pszKeywordBegin, _T("End")))              *pszDestBuf = SY_END;
				else if (!_tcscmp(pszKeywordBegin, _T("ForDirectoriesDo"))) *pszDestBuf = SY_FOR_DIRS;
				else if (!_tcscmp(pszKeywordBegin, _T("DirectoryUrl")))     *pszDestBuf = SY_DIR_URL;
				else if (!_tcscmp(pszKeywordBegin, _T("DirectoryName")))    *pszDestBuf = SY_DIR_NAME;
				else if (!_tcscmp(pszKeywordBegin, _T("ForFilesDo")))       *pszDestBuf = SY_FOR_FILES;
				else if (!_tcscmp(pszKeywordBegin, _T("FileUrl")))          *pszDestBuf = SY_FILE_URL;
				else if (!_tcscmp(pszKeywordBegin, _T("FileName")))         *pszDestBuf = SY_FILE_NAME;
				else if (!_tcscmp(pszKeywordBegin, _T("FileSize")))         *pszDestBuf = SY_FILE_SIZE;
				else if (!_tcscmp(pszKeywordBegin, _T("FileCreated")))      *pszDestBuf = SY_FILE_CREATE_TIME;
				else if (!_tcscmp(pszKeywordBegin, _T("FileModified")))     *pszDestBuf = SY_FILE_MODIFY_TIME;
				else if (!_tcscmp(pszKeywordBegin, _T("IsEven")))           *pszDestBuf = SY_IS_EVEN;
				else if (!_tcscmp(pszKeywordBegin, _T("IsOdd")))            *pszDestBuf = SY_IS_ODD;
				else if (!_tcscmp(pszKeywordBegin, _T("IsFileType")))       *pszDestBuf = SY_IS_FILE_TYPE;
				else {
					LogEvent(_T("Error in index template"), _T("Unknown Tag"));
					// unknown tag
				}

				// remove some useless returns
				if (*pszDestBuf == SY_END)
					while (*(pszBuf + 1) == '\r' || *(pszBuf + 1) == '\n')
						pszBuf++;

				pszDestBuf++;

				// these tags require an End - reserve space for relative jump address (2 byte)
				switch (*(pszDestBuf - 1)) {
					case SY_FOR_DIRS:
					case SY_FOR_FILES:
					case SY_IS_EVEN:
					case SY_IS_ODD:
					case SY_IS_FILE_TYPE: {
							*((WORD*)(pszDestBuf)) = 0x7070;
							pszDestBuf += 2;
						}
				}

				if (bHasParameters) {
					// max MAX_PARAM_LENGTH chars per param (terminated with : when shorter than MAX_PARAM_LENGTH)
					byte  iParamCount = 1;
					TCHAR *pcParamCount = pszDestBuf++;
					TCHAR *pszParameterBegin = pszBuf + 1;

					do {
						if (*pszBuf == ',') {
							*pszBuf = ':';
							_tcsncpy(pszDestBuf, pszParameterBegin, MAX_PARAM_LENGTH);
							pszDestBuf += MAX_PARAM_LENGTH;

							pszParameterBegin = pszBuf + 1;
							iParamCount++;
						}
						pszBuf++;
					} while (*pszBuf != ']' && *pszBuf != '\0');

					if (*pszBuf == '\0')
						break;

					*pszBuf = ':';
					_tcsncpy(pszDestBuf, pszParameterBegin, MAX_PARAM_LENGTH);
					pszDestBuf += MAX_PARAM_LENGTH;

					*pcParamCount = iParamCount;
				}
			} else {
				*(pszDestBuf++) = *pszBuf;
			}

			pszBuf++;
		}

		*(pszDestBuf) = '\0';

		pszBuf = szDestBuf;

		while (*pszBuf != '\0') {
			byte iLevel = 0;
			switch (*pszBuf) {
					// these tags require an End - precalculate address of End
				case SY_FOR_DIRS:
				case SY_FOR_FILES:
				case SY_IS_EVEN:
				case SY_IS_ODD:
				case SY_IS_FILE_TYPE:
					iLevel++;
					break;
			}

			pszBuf++;

			// begin of iLevel - find End of iLevel
			if (iLevel > 0) {
				TCHAR *pszLevelEnd = pszBuf + 2; // skip for address reserved bytes

				// skip parameters of IsFileType
				if (*(pszBuf - 1) == SY_IS_FILE_TYPE) {
					pszLevelEnd++;
					pszLevelEnd += *(pszLevelEnd - 1) * MAX_PARAM_LENGTH;
				}

				while (*pszLevelEnd != '\0' && iLevel > 0) {
					switch (*pszLevelEnd) {
						case SY_FOR_DIRS:
						case SY_FOR_FILES:
						case SY_IS_EVEN:
						case SY_IS_ODD:
							iLevel++;
							pszLevelEnd += 2;
							break;

						case SY_IS_FILE_TYPE:
							iLevel++;
							pszLevelEnd += 2;
							pszLevelEnd += 1;
							pszLevelEnd += *(pszLevelEnd) * MAX_PARAM_LENGTH;
							break;

						case SY_END:
							iLevel--;
							break;
					}

					pszLevelEnd++;
				}

				if (*pszLevelEnd == '\0') {
					LogEvent(_T("Error in index template"), _T("End is missing"));
					break; // Error - End missing
				}

				// write jump address
				*((WORD*)(pszBuf)) = (WORD)((pszLevelEnd - pszBuf - 1) | 0x8000);
				pszBuf += 2;
			}
		}

		*(pszBuf) = '\0';

		//LogEvent("Template", szDestBuf);

		szIndexHTMLTemplate = new TCHAR[_tcslen(szDestBuf) + 1];
		_tcscpy(szIndexHTMLTemplate, szDestBuf);
	}

	CloseHandle(hFile);
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : FreeIndexHTMLTemplate
// Type            : Global
// Parameters      : -
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 050901, 05 august 2005
// Developer       : Houdini
/////////////////////////////////////////////////////////////////////

void FreeIndexHTMLTemplate() {
	if (szIndexHTMLTemplate != NULL) {
		delete[] szIndexHTMLTemplate;
		szIndexHTMLTemplate = NULL;
	}
}

/////////////////////////////////////////////////////////////////////
// Member Function : bCreateIndexHTML
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

bool bCreateIndexHTML(const TCHAR *pszRealPath, const TCHAR *pszIndexPath, const TCHAR *pszSrvPath, DWORD /* dwRemoteIP */)
{
#define RelativeJump(begin) { pszPos += *((WORD*)(begin+1)) & 0x7FFF; }

	if (szIndexHTMLTemplate == NULL)
		return false;

	// check if directory exists
	TCHAR szMask[MAX_PATH];
	_tcscpy(szMask, pszRealPath);
	_tcscat(szMask, _T("*"));

	WIN32_FIND_DATA fdFindFileData;
	HANDLE hFind = FindFirstFile(szMask, &fdFindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hFind);
	hFind = 0;

	HANDLE hFile = CreateFile(pszIndexPath, GENERIC_WRITE, FILE_SHARE_READ ,
	    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	TCHAR szBuffer[10000];
	TCHAR *pszBuffer = szBuffer;
	DWORD dwBytesWritten = 0;

	TCHAR *pszPos = szIndexHTMLTemplate;

	byte  iCurrentAction = 0;
	TCHAR *pszLevelBegin[50];
	byte  iLevel = 0;

	TCHAR  szName[1000] = _T("");
	TCHAR  szURL[1000] = _T("");
	int   iFileSize = 0;
	FILETIME ftFileCreateTime;
	FILETIME ftFileModifyTime;
	bool  bEvenOdd = 0;
	bool  bKnownFileType = false;

	_tcscpy(szBuffer, pszSrvPath);
	TCHAR *pszTemp = _tcsrchr(szBuffer, '/');
	if (pszTemp)
		*pszTemp = '\0';

	pszTemp = _tcsrchr(szBuffer, '/');
	if (pszTemp)
		_tcscpy(szName, pszTemp + 1);

	if (szName[0] == '\0')
		_tcscpy(szName, _T("my Miranda Webserver"));

	do {
		switch (*pszPos) {
			case SY_FOR_FILES:
			case SY_FOR_DIRS: {
				if (hFind == 0) {
					pszLevelBegin[iLevel++] = pszPos;
					iCurrentAction = *pszPos;

					hFind = FindFirstFile(szMask, &fdFindFileData);
					if (hFind == 0) {
						iCurrentAction = 0;
						RelativeJump(pszLevelBegin[iLevel-1]);
						break;
					}
				} else {
					if (!FindNextFile(hFind, &fdFindFileData)) {
						FindClose(hFind);
						hFind = 0;
						iCurrentAction = 0;
						RelativeJump(pszLevelBegin[iLevel-1]);
						break;
					}
				}

				while (!_tcscmp(fdFindFileData.cFileName, _T(".")) ||
				    !_tcsncmp(fdFindFileData.cFileName, _T("@"), 1) ||
				    (!_tcscmp(fdFindFileData.cFileName, _T("..")) && !_tcscmp(pszSrvPath, _T("/"))) || // hide .. in root
				    ((*pszPos == 19) == ((fdFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0))) {
					if (!FindNextFile(hFind, &fdFindFileData)) {
						FindClose(hFind);
						hFind = 0;
						iCurrentAction = 0;
						RelativeJump(pszLevelBegin[iLevel-1]);
						break;
					}
				}

				if (hFind) {
					_tcscpy(szName, fdFindFileData.cFileName);
					_tcscpy(szURL, fdFindFileData.cFileName);
					/*char* pszTmp = szURL;
					while(pszTmp = strchr(pszTmp, ' '))
					*pszTmp = '+';*/

					if (*pszPos == SY_FOR_DIRS) { // For Directories
						_tcscat(szURL, _T("/"));
					} else { // For Files
						iFileSize = fdFindFileData.nFileSizeLow;
						ftFileCreateTime = fdFindFileData.ftCreationTime;
						ftFileModifyTime = fdFindFileData.ftLastWriteTime;
					}

					bKnownFileType = false;
					bEvenOdd = !bEvenOdd;
					pszPos += 2;
				}

				break;
			}

			case SY_END: { // End
				if (iLevel <= 0)
					break; // Error

				if (iCurrentAction == SY_FOR_DIRS || iCurrentAction == SY_FOR_FILES) { // For loops
					pszPos = pszLevelBegin[iLevel-1] - 1; // jump to begin
				} else {
					iLevel--;
					if (iLevel > 0)
						iCurrentAction = *pszLevelBegin[iLevel-1];
					else
						iCurrentAction = 0;
				}

				break;
			}

			case SY_FILE_NAME:
			case SY_DIR_NAME: {
				pszBuffer += mir_sntprintf(pszBuffer, 250, _T("%s"), szName);
				break;
			}

			case SY_DIR_URL: {
			case SY_FILE_URL:
				pszBuffer += mir_sntprintf(pszBuffer, 250, _T("%s"), szURL);
				break;
			}

			case SY_FILE_CREATE_TIME:
			case SY_FILE_MODIFY_TIME: {
				SYSTEMTIME systemTime;
				FileTimeToSystemTime(
				  (*pszPos == SY_FILE_CREATE_TIME) ? &ftFileCreateTime : &ftFileModifyTime,
				  &systemTime);

				pszBuffer += mir_sntprintf(pszBuffer, 100, _T("%i/%02i/%02i %i:%02i:%02i"), 
					systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
				  systemTime.wMinute, systemTime.wSecond);
				break;
			}

			case SY_FILE_SIZE: {
				if ((iFileSize >> 10) == 0)
					pszBuffer += mir_sntprintf(pszBuffer, 100, _T("%i Byte"), iFileSize);
				else if ((iFileSize >> 20) == 0)
					pszBuffer += mir_sntprintf(pszBuffer, 100, _T("%.1f KB"), (float)(iFileSize) / 1024.0f);
				else
					pszBuffer += mir_sntprintf(pszBuffer, 100, _T("%.1f MB"), (float)(iFileSize) / (1024.0f * 1024.0f));
				break;
			}

			case SY_IS_EVEN:
			case SY_IS_ODD: {
				pszLevelBegin[iLevel++] = pszPos;
				iCurrentAction = *pszPos;

				if (bEvenOdd != (*pszPos - SY_IS_EVEN == 1)) { // SY_IS_EVEN+1 == SY_IS_ODD
					RelativeJump(pszLevelBegin[iLevel-1]);
				} else {
					pszPos += 2;
				}
				break;
			}

			case SY_IS_FILE_TYPE: {
				pszLevelBegin[iLevel++] = pszPos;
				iCurrentAction = *pszPos;

				byte  iParamCount = *(pszPos + 3);
				TCHAR* pszParam = pszPos + 4;
				bool  bSkip = true;

				if (bKnownFileType == false) {
					if (*pszParam == '*') {
						bSkip = false;
					} else {
						for (byte i = 0; i < iParamCount; i++) {
							TCHAR szParam[MAX_PARAM_LENGTH+1];
							_tcsncpy(szParam, pszParam, MAX_PARAM_LENGTH);
							szParam[MAX_PARAM_LENGTH] = '\0';
							TCHAR *pszTmp = _tcschr(szParam, ':');
							if (pszTmp)
								*pszTmp = '\0';

							TCHAR *pszExt = _tcsrchr(szName, '.');

							if (pszExt && !_tcsicmp(pszExt + 1, szParam)) {
								bSkip = false;
								break;
							}
							pszParam += MAX_PARAM_LENGTH;
						}
					}
				}

				if (bSkip) {
					RelativeJump(pszLevelBegin[iLevel-1]);
				} else {
					bKnownFileType = true;
					pszPos += 2;
					pszPos += 1;
					pszPos += *(pszPos) * MAX_PARAM_LENGTH;
				}
				break;
			}

			default: {
				*(pszBuffer++) = *pszPos;
			}
		}

		pszPos++;

		// flush the buffer from time to time
		if (*pszPos == '\0' || pszBuffer - szBuffer > 8000) {
			if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, NULL))
				break;

			pszBuffer = szBuffer;
		}
	} while (*pszPos != '\0');

	if (hFind != 0)
		FindClose(hFind);

	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return TRUE;
}