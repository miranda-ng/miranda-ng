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

static char* szIndexHTMLTemplate = NULL;
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

	char  szBuf[10000];
	char* pszBuf = szBuf;

	char  szDestBuf[10000];
	char* pszDestBuf = szDestBuf;

	strncpy(pszBuf, szPluginPath, SIZEOF(szBuf)-1);
	strncat(pszBuf, szIndexHTMLTemplateFile, SIZEOF(szBuf)-1);

	HANDLE hFile = CreateFile(pszBuf, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
	    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, "HTTPServerIndex.html not found in Plugin Path", MSG_BOX_TITEL, MB_OK);
		return false;
	}

	DWORD dwBytesRead = 0;
	if (ReadFile(hFile, pszBuf, sizeof(szBuf), &dwBytesRead, NULL) || dwBytesRead <= 0) {
		while (dwBytesRead > (DWORD)(pszBuf - szBuf)) {
			if (*pszBuf == '[') {
				char* pszKeywordBegin = pszBuf + 1;
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
				if (!strcmp(pszKeywordBegin, "End"))              *pszDestBuf = SY_END;
				else if (!strcmp(pszKeywordBegin, "ForDirectoriesDo")) *pszDestBuf = SY_FOR_DIRS;
				else if (!strcmp(pszKeywordBegin, "DirectoryUrl"))     *pszDestBuf = SY_DIR_URL;
				else if (!strcmp(pszKeywordBegin, "DirectoryName"))    *pszDestBuf = SY_DIR_NAME;
				else if (!strcmp(pszKeywordBegin, "ForFilesDo"))       *pszDestBuf = SY_FOR_FILES;
				else if (!strcmp(pszKeywordBegin, "FileUrl"))          *pszDestBuf = SY_FILE_URL;
				else if (!strcmp(pszKeywordBegin, "FileName"))         *pszDestBuf = SY_FILE_NAME;
				else if (!strcmp(pszKeywordBegin, "FileSize"))         *pszDestBuf = SY_FILE_SIZE;
				else if (!strcmp(pszKeywordBegin, "FileCreated"))      *pszDestBuf = SY_FILE_CREATE_TIME;
				else if (!strcmp(pszKeywordBegin, "FileModified"))     *pszDestBuf = SY_FILE_MODIFY_TIME;
				else if (!strcmp(pszKeywordBegin, "IsEven"))           *pszDestBuf = SY_IS_EVEN;
				else if (!strcmp(pszKeywordBegin, "IsOdd"))            *pszDestBuf = SY_IS_ODD;
				else if (!strcmp(pszKeywordBegin, "IsFileType"))       *pszDestBuf = SY_IS_FILE_TYPE;
				else {
					LogEvent("Error in index template", "Unknown Tag");
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
					char* pcParamCount = (pszDestBuf++);
					char* pszParameterBegin = pszBuf + 1;

					do {
						if (*pszBuf == ',') {
							*pszBuf = ':';
							strncpy(pszDestBuf, pszParameterBegin, MAX_PARAM_LENGTH);
							pszDestBuf += MAX_PARAM_LENGTH;

							pszParameterBegin = pszBuf + 1;
							iParamCount++;
						}
						pszBuf++;
					} while (*pszBuf != ']' && *pszBuf != '\0');

					if (*pszBuf == '\0')
						break;

					*pszBuf = ':';
					strncpy(pszDestBuf, pszParameterBegin, MAX_PARAM_LENGTH);
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
				char* pszLevelEnd = pszBuf + 2; // skip for address reserved bytes

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
					LogEvent("Error in index template", "End is missing");
					break; // Error - End missing
				}

				// write jump address
				*((WORD*)(pszBuf)) = (WORD)((pszLevelEnd - pszBuf - 1) | 0x8000);
				pszBuf += 2;
			}
		}

		*(pszBuf) = '\0';

		//LogEvent("Template", szDestBuf);

		szIndexHTMLTemplate = new char[strlen(szDestBuf)+1];
		strcpy(szIndexHTMLTemplate, szDestBuf);
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

bool bCreateIndexHTML(const char * pszRealPath, const char * pszIndexPath,
    const char * pszSrvPath, DWORD /* dwRemoteIP */) {
#define RelativeJump(begin) { pszPos += *((WORD*)(begin+1)) & 0x7FFF; }

	if (szIndexHTMLTemplate == NULL)
		return false;

	// check if directory exists
	char szMask[MAX_PATH];
	strncpy(szMask, pszRealPath, MAX_PATH- 1);
	strncat(szMask, "*", MAX_PATH- 1);

	WIN32_FIND_DATAA fdFindFileData;
	HANDLE hFind = FindFirstFile(szMask, &fdFindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hFind);
	hFind = 0;

	HANDLE hFile = CreateFile(pszIndexPath, GENERIC_WRITE, FILE_SHARE_READ ,
	    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	char  szBuffer[10000];
	char* pszBuffer = szBuffer;
	DWORD dwBytesWritten = 0;

	char* pszPos = szIndexHTMLTemplate;

	byte  iCurrentAction = 0;
	char* pszLevelBegin[50];
	byte  iLevel = 0;

	char  szName[1000] = "";
	char  szURL[1000] = "";
	int   iFileSize = 0;
	FILETIME ftFileCreateTime;
	FILETIME ftFileModifyTime;
	bool  bEvenOdd = 0;
	bool  bKnownFileType = false;

	strcpy(szBuffer, pszSrvPath);
	char* pszTemp = strrchr(szBuffer, '/');
	if (pszTemp)
		*pszTemp = '\0';

	pszTemp = strrchr(szBuffer, '/');
	if (pszTemp)
		strcpy(szName, pszTemp + 1);

	if (szName[0] == '\0')
		strcpy(szName, "my Miranda Webserver");

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

				while (!strcmp(fdFindFileData.cFileName, ".") ||
				    !strncmp(fdFindFileData.cFileName, "@", 1) ||
				    (!strcmp(fdFindFileData.cFileName, "..") && !strcmp(pszSrvPath, "/")) || // hide .. in root
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
					strcpy(szName, fdFindFileData.cFileName);
					strcpy(szURL, fdFindFileData.cFileName);
					/*char* pszTmp = szURL;
					while(pszTmp = strchr(pszTmp, ' '))
					*pszTmp = '+';*/

					if (*pszPos == SY_FOR_DIRS) { // For Directories
						strcat(szURL, "/");
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
				pszBuffer += mir_snprintf(pszBuffer, 250, "%s", szName);
				break;
			}

			case SY_DIR_URL: {
			case SY_FILE_URL:
				pszBuffer += mir_snprintf(pszBuffer, 250, "%s", szURL);
				break;
			}

			case SY_FILE_CREATE_TIME:
			case SY_FILE_MODIFY_TIME: {
				SYSTEMTIME systemTime;
				FileTimeToSystemTime(
				  (*pszPos == SY_FILE_CREATE_TIME) ? &ftFileCreateTime : &ftFileModifyTime,
				  &systemTime);

				pszBuffer += mir_snprintf(pszBuffer, 100, "%i/%02i/%02i %i:%02i:%02i", 
					systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
				  systemTime.wMinute, systemTime.wSecond);
				break;
			}

			case SY_FILE_SIZE: {
				if ((iFileSize >> 10) == 0)
					pszBuffer += mir_snprintf(pszBuffer, 100, "%i Byte", iFileSize);
				else if ((iFileSize >> 20) == 0)
					pszBuffer += mir_snprintf(pszBuffer, 100, "%.1f KB", (float)(iFileSize) / 1024.0f);
				else
					pszBuffer += mir_snprintf(pszBuffer, 100, "%.1f MB", (float)(iFileSize) / (1024.0f * 1024.0f));
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
				char* pszParam = pszPos + 4;
				bool  bSkip = true;

				if (bKnownFileType == false) {
					if (*pszParam == '*') {
						bSkip = false;
					} else {
						for (byte i = 0; i < iParamCount; i++) {
							char szParam[MAX_PARAM_LENGTH+1];
							strncpy(szParam, pszParam, MAX_PARAM_LENGTH);
							szParam[MAX_PARAM_LENGTH] = '\0';
							char* pszTmp = strchr(szParam, ':');
							if (pszTmp)
								*pszTmp = '\0';

							char* pszExt = strrchr(szName, '.');

							if (pszExt && !_stricmp(pszExt + 1, szParam)) {
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