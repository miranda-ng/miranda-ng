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

static const char szXmlHeader1[] =
	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n"
	"<?xml-stylesheet type=\"text/xsl\" href=\"";

static const char szXmlHeader2[] = 
	"\"?>\r\n"
	"<config>\r\n";

static const char szXmlTail[] = "</config>";

static void ReplaceSign(char* pszSrc, int MaxLength, const char pszReplace,
	const char * pszNew)
{
	static char szBuffer[1024];
	char* pszSign = strchr(pszSrc, pszReplace);

	if (pszSign) {
		strncpy(szBuffer, pszSrc, _countof(szBuffer) - 1);

		do {
			mir_strcpy(szBuffer + (pszSign - pszSrc), pszNew);
			mir_strcpy(szBuffer + (pszSign - pszSrc) + mir_strlen(pszNew), pszSign + 1);
			*pszSign = ' ';

		} while (pszSign = strchr(pszSrc, pszReplace));

		strncpy(pszSrc, szBuffer, MaxLength);

		pszSrc[MaxLength - 1] = '\0';
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

bool bCreateIndexXML(const char * pszRealPath, const char * pszIndexPath, const char * pszSrvPath, uint32_t dwRemoteIP)
{
	char szMask[MAX_PATH + 1];
	mir_snprintf(szMask, "%s*", pszRealPath);

	WIN32_FIND_DATAA fdFindFileData;
	HANDLE hFind = FindFirstFile(szMask, &fdFindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	HANDLE hFile = CreateFile(pszIndexPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
		OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return FALSE;
	}

	const int BUFFER_SIZE = 1000;
	char  szBuffer[BUFFER_SIZE + 1];
	char* pszBuffer = szBuffer;
	char  szFileName[MAX_PATH + 1] = "";
	char* pszExt;
	DWORD dwBytesWritten = 0;

	// Generate Dirname
	strncpy(szBuffer, pszSrvPath, BUFFER_SIZE);
	char* pszTemp = strrchr(szBuffer, '/');
	if (pszTemp)
		*pszTemp = '\0';

	pszTemp = strrchr(szBuffer, '/');
	if (pszTemp)
		strncpy(szFileName, pszTemp + 1, MAX_PATH);

	// Write Header
	WriteFile(hFile, szXmlHeader1, sizeof(szXmlHeader1) - 1, &dwBytesWritten, nullptr);

	// check if a index.xsl exists in the same directory otherwise use the global
	mir_snprintf(szMask, "%s%s", pszRealPath, "index.xsl");

	HANDLE hFileExists = CreateFile(szMask, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFileExists == INVALID_HANDLE_VALUE) {
		strncpy(szBuffer, "/index.xsl", BUFFER_SIZE);
	}
	else {
		CloseHandle(hFileExists);
		strncpy(szBuffer, "index.xsl", BUFFER_SIZE);
	}

	WriteFile(hFile, szBuffer, (uint32_t)mir_strlen(szBuffer), &dwBytesWritten, nullptr);

	WriteFile(hFile, szXmlHeader2, sizeof(szXmlHeader2) - 1, &dwBytesWritten, nullptr);

	// Write dirname
	ReplaceSign(szFileName, MAX_PATH, '&', "&amp;");
	pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
		"  <dirname>%s</dirname>\r\n", szFileName);
	WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, nullptr);

	// Find files and directories
	do {
		if (mir_strcmp(fdFindFileData.cFileName, ".") &&
			strncmp(fdFindFileData.cFileName, "@", 1) &&
			(mir_strcmp(fdFindFileData.cFileName, "..") || mir_strcmp(pszSrvPath, "/"))) { // hide .. in root
			pszBuffer = szBuffer;

			mir_strcpy(szFileName, fdFindFileData.cFileName);
			ReplaceSign(szFileName, MAX_PATH, '&', "&amp;");

			if (fdFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
					"  <item name=\"%s\" isdir=\"true\"/>\r\n", szFileName);
			}
			else {
				pszExt = strrchr(szFileName, '.');

				if (pszExt != nullptr) {
					*pszExt = '\0';
					pszExt++;
				}

				pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
					"  <item name=\"%s\" ext=\"%s\" size=\"%i\" ",
					szFileName, (pszExt == nullptr) ? "" : pszExt, fdFindFileData.nFileSizeLow);

				SYSTEMTIME systemTime;
				FileTimeToSystemTime(&fdFindFileData.ftCreationTime, &systemTime);
				pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
					"created=\"%i/%02i/%02i %i:%02i:%02i\" ",
					systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
					systemTime.wMinute, systemTime.wSecond);

				FileTimeToSystemTime(&fdFindFileData.ftLastWriteTime, &systemTime);
				pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
					"modified=\"%i/%02i/%02i %i:%02i:%02i\" ",
					systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
					systemTime.wMinute, systemTime.wSecond);

				pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
					"/>\r\n");
			}

			if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, nullptr))
				break;
		}

	} while (FindNextFile(hFind, &fdFindFileData));

	if (hFind != nullptr)
		FindClose(hFind);

	// Add other shared files & directories
	for (CLFileShareNode * pclCur = pclFirstNode; pclCur; pclCur = pclCur->pclNext) {
		if (!((pclCur->st.dwAllowedIP ^ dwRemoteIP) & pclCur->st.dwAllowedMask) &&  // hide inaccessible shares
			(size_t)(pclCur->nGetSrvPathLen()) > mir_strlen(pszSrvPath) &&
			!strstr(pclCur->st.pszRealPath, "\\@") &&
			!strncmp(pclCur->st.pszSrvPath, pszSrvPath, mir_strlen(pszSrvPath))) {
			pszBuffer = szBuffer;

			mir_strcpy(szFileName, &pclCur->st.pszSrvPath[mir_strlen(pszSrvPath)]);
			ReplaceSign(szFileName, MAX_PATH, '&', "&amp;");

			if (pclCur->bIsDirectory()) {
				szFileName[mir_strlen(szFileName) - 1] = '\0';
				if (!strchr(szFileName, '/')) { // only one level deeper
					pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
						"  <item name=\"%s\" isdir=\"true\"/>\r\n", szFileName);

					if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, nullptr))
						break;
				}
			}
			else {
				if (!strchr(szFileName, '/') &&   // only one level deeper
					strncmp(pszRealPath, pclCur->st.pszRealPath, mir_strlen(pszRealPath))) { // no duplicates
					pszExt = strrchr(szFileName, '.');

					if (pszExt != nullptr) {
						*pszExt = '\0';
						pszExt++;
					}

					uint32_t dwFileSize = 0;
					FILETIME ftFileCreateTime;
					FILETIME ftFileAccessTime;
					FILETIME ftFileModifyTime;
					HANDLE hFileS = CreateFile(pclCur->st.pszRealPath, GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
					if (hFileS != INVALID_HANDLE_VALUE) {
						dwFileSize = GetFileSize(hFileS, nullptr);
						GetFileTime(hFileS, &ftFileCreateTime, &ftFileAccessTime, &ftFileModifyTime);
						CloseHandle(hFileS);
					}

					pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
						"  <item name=\"%s\" ext=\"%s\" size=\"%i\" ",
						szFileName, (pszExt == nullptr) ? "" : pszExt, dwFileSize);

					SYSTEMTIME systemTime;
					FileTimeToSystemTime(&ftFileCreateTime, &systemTime);
					pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
						"created=\"%i/%02i/%02i %i:%02i:%02i\" ",
						systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
						systemTime.wMinute, systemTime.wSecond);

					FileTimeToSystemTime(&ftFileModifyTime, &systemTime);
					pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
						"modified=\"%i/%02i/%02i %i:%02i:%02i\" ",
						systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour,
						systemTime.wMinute, systemTime.wSecond);

					pszBuffer += mir_snprintf(pszBuffer, BUFFER_SIZE - (pszBuffer - szBuffer),
						"/>\r\n");

					if (!WriteFile(hFile, szBuffer, pszBuffer - szBuffer, &dwBytesWritten, nullptr))
						break;
				}
			}
		}
	}

	WriteFile(hFile, szXmlTail, sizeof(szXmlTail) - 1, &dwBytesWritten, nullptr);

	SetEndOfFile(hFile);
	CloseHandle(hFile);

	return TRUE;
}
