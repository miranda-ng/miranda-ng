/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda IM project,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "chat.h"
#include <math.h>

static int RTFColorToIndex(int *pIndex, int iCol, SESSION_INFO *si)
{
	MODULEINFO *pMod = pci->MM_FindModule(si->pszModule);
	for (int i = 0; i < pMod->nColorCount; i++)
		if (pIndex[i] == iCol)
			return i;

	return -1;
}

static void CreateColorMap(char* Text, int *pIndex, SESSION_INFO *si)
{
	int iIndex = 1;

	static const char* lpszFmt = "\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];";
	char szRed[10], szGreen[10], szBlue[10];

	char *p1 = strstr(Text, "\\colortbl");
	if (!p1)
		return;

	char *pEnd = strchr(p1, '}');
	char *p2 = strstr(p1, "\\red");

	while (p2 && p2 < pEnd) {
		if (sscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			MODULEINFO *pMod = pci->MM_FindModule(si->pszModule);
			for (int i = 0; i < pMod->nColorCount; i++)
				if (pMod->crColors[i] == RGB(atoi(szRed), atoi(szGreen), atoi(szBlue)))
					pIndex[i] = iIndex;
		}
		iIndex++;
		p1 = p2;
		p1++;
		p2 = strstr(p1, "\\red");
	}
}

static int ReadInteger( const char* p, int* result )
{
	char temp[10];
	int i=0;
	while (isdigit(*p))
		temp[i++] = *p++;
	temp[i] = 0;

	if ( result != NULL )
		*result = atoi( temp );

	return i;
}

TCHAR* DoRtfToTags(char* pszText, SESSION_INFO *si)
{
	char *p1;
	int*  pIndex;
	int i, iRemoveChars, cp = CP_ACP;
	char InsertThis[50];
	BOOL bJustRemovedRTF = TRUE;
	BOOL bTextHasStarted = FALSE;
	int iUcMode = 0;

	if (!pszText)
		return FALSE;

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	pIndex = (int *)mir_alloc(sizeof(int)* pci->MM_FindModule(si->pszModule)->nColorCount);
	for (i = 0; i < pci->MM_FindModule(si->pszModule)->nColorCount; i++)
		pIndex[i] = -1;

	CreateColorMap(pszText, pIndex, si);

	// scan the file for rtf commands and remove or parse them
	p1 = strstr(pszText, "\\pard");
	if (p1 == NULL) {
		mir_free(pIndex);
		return FALSE;
	}

	p1 += 5;

	memmove(pszText, p1, strlen(p1) + 1);
	p1 = pszText;

	// iterate through all characters, if rtf control character found then take action
	while (*p1 != '\0') {
		InsertThis[0] = 0;
		iRemoveChars = 0;

		switch (*p1) {
		case '\\':
			if (!memcmp(p1, "\\cf", 3)) { // foreground color
				int iCol, iInd;
				iRemoveChars = 3 + ReadInteger(p1 + 3, &iCol);
				iInd = RTFColorToIndex(pIndex, iCol, si);
				bJustRemovedRTF = TRUE;

//				if (bTextHasStarted || iInd >= 0)
//					mir_snprintf(InsertThis, SIZEOF(InsertThis), (iInd >= 0) ? "%%c%02u" : "%%C", iInd);
			}
			else if (!memcmp(p1, "\\highlight", 10)) { //background color
				int iCol, iInd;
				iRemoveChars = 10 + ReadInteger(p1 + 10, &iCol);
				iInd = RTFColorToIndex(pIndex, iCol, si);
				bJustRemovedRTF = TRUE;

//				if (bTextHasStarted || iInd >= 0)
//					mir_snprintf(InsertThis, SIZEOF(InsertThis), (iInd >= 0) ? "%%f%02u" : "%%F", iInd);
			}
			else if (!memcmp(p1, "\\lang", 5)) { // language id
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = 5 + ReadInteger(p1 + 5, NULL);
			}
			else if (!memcmp(p1, "\\par", 4)) { // newline
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = 4;
				strcpy(InsertThis, "\n");
			}
			else if (!memcmp(p1, "\\line", 5)) { // newline
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = 5;
				strcpy(InsertThis, "\n");
			}
			else if (!memcmp(p1, "\\bullet", 7)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				strcpy(InsertThis, "\xE2\x80\xA2");
			}
			else if (!memcmp(p1, "\\b", 2)) { //bold
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = (p1[2] != '0') ? 2 : 3;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), (p1[2] != '0') ? "%%b" : "%%B");
			}
			else if (!memcmp(p1, "\\i", 2)) { // italics
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = (p1[2] != '0') ? 2 : 3;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), (p1[2] != '0') ? "%%i" : "%%I");
			}
			else if (!memcmp(p1, "\\uc", 3)) { // number of Unicode chars
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iUcMode = p1[3] - '0';
				iRemoveChars = 4;
			}
			else if (!memcmp(p1, "\\ul", 3)) { // underlined
				bTextHasStarted = bJustRemovedRTF = TRUE;
				if (p1[3] == 'n')
					iRemoveChars = 7;
				else if (p1[3] == '0')
					iRemoveChars = 4;
				else
					iRemoveChars = 3;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), (p1[3] != '0' && p1[3] != 'n') ? "%%u" : "%%U");
			}
			else if (p1[1] == 'f' && isdigit(p1[2])) { // unicode char
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = 2 + ReadInteger(p1 + 2, NULL);
			}
			else if (!memcmp(p1, "\\tab", 4)) { // tab
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 4;
				strcpy(InsertThis, " ");
			}
			else if (!memcmp(p1, "\\endash", 7)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				strcpy(InsertThis, "\xE2\x80\x93");
			}
			else if (!memcmp(p1, "\\emdash", 7)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				strcpy(InsertThis, "\xE2\x80\x94");
			}
			else if (!memcmp(p1, "\\lquote", 7)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				strcpy(InsertThis, "\xE2\x80\x98");
			}
			else if (!memcmp(p1, "\\rquote", 7)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				strcpy(InsertThis, "\xE2\x80\x99");
			}
			else if (!memcmp(p1, "\\ldblquote", 10)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 10;
				strcpy(InsertThis, "\xe2\x80\x9c");
			}
			else if (!memcmp(p1, "\\rdblquote", 10)) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 10;
				strcpy(InsertThis, "\xe2\x80\x9d");
			}
			else if (p1[1] == '\\' || p1[1] == '{' || p1[1] == '}') { // escaped characters
				bTextHasStarted = TRUE;
				bJustRemovedRTF = FALSE;
				iRemoveChars = 2;
				mir_snprintf(InsertThis, SIZEOF(InsertThis), "%c", p1[1]);
			}
			else if (p1[1] == '~') { // non-breaking space
				bTextHasStarted = TRUE;
				bJustRemovedRTF = FALSE;
				iRemoveChars = 2;
				strcpy(InsertThis, "\xC2\xA0");
			}
			else if (p1[1] == '\'') { // special character
				char tmp[4], *p3 = tmp;
				bTextHasStarted = TRUE;
				bJustRemovedRTF = FALSE;
				if (p1[2] != ' ' && p1[2] != '\\') {
					*p3++ = p1[2];
					iRemoveChars = 3;
					if (p1[3] != ' ' && p1[3] != '\\') {
						*p3++ = p1[3];
						iRemoveChars++;
					}
					*p3 = 0;
					sscanf(tmp, "%x", InsertThis);

					InsertThis[1] = 0;
				}
				else iRemoveChars = 2;
			}
			else if (bJustRemovedRTF) { // remove unknown RTF command
				int j = 1;
				bJustRemovedRTF = TRUE;
				while (p1[j] != ' ' && p1[j] != '\\' && p1[j] != '\0')
					j++;
				iRemoveChars = j;
			}
			break;

		case '{': // other RTF control characters
		case '}':
			iRemoveChars = 1;
			break;

		case '\r': case '\n':
			bTextHasStarted = TRUE;
			bJustRemovedRTF = FALSE;
			iRemoveChars = 1;
			break;

		case '%': // escape chat -> protocol control character
			bTextHasStarted = TRUE;
			bJustRemovedRTF = FALSE;
			iRemoveChars = 1;
			strcpy(InsertThis, "%%");
			break;
		case ' ': // remove spaces following a RTF command
			if (bJustRemovedRTF)
				iRemoveChars = 1;
			bJustRemovedRTF = FALSE;
			bTextHasStarted = TRUE;
			break;

		default: // other text that should not be touched
			bTextHasStarted = TRUE;
			bJustRemovedRTF = FALSE;
			break;
		}

		// move the memory and paste in new commands instead of the old RTF
		if (InsertThis[0] || iRemoveChars) {
			size_t len = strlen(InsertThis);
			memmove(p1 + len, p1 + iRemoveChars, strlen(p1) - iRemoveChars + 1);
			memcpy(p1, InsertThis, len);
			p1 += len;
		}
		else p1++;
	}

	mir_free(pIndex);

	return mir_utf8decodeW(pszText);
}

static DWORD CALLBACK Message_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static DWORD dwRead;
	char ** ppText = (char **)dwCookie;

	if (*ppText == NULL) {
		*ppText = (char *)mir_alloc(cb + 1);
		memcpy(*ppText, pbBuff, cb);
		(*ppText)[cb] = 0;
		*pcb = cb;
		dwRead = cb;
	}
	else {
		char  *p = (char *)mir_alloc(dwRead + cb + 1);
		memcpy(p, *ppText, dwRead);
		memcpy(p + dwRead, pbBuff, cb);
		p[dwRead + cb] = 0;
		mir_free(*ppText);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
	}

	return 0;
}

char* Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si)
{
	EDITSTREAM stream;
	char* pszText = NULL;
	DWORD dwFlags;

	if (hwndDlg == 0 || si == 0)
		return NULL;

	ZeroMemory(&stream, sizeof(stream));
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD_PTR)&pszText; // pass pointer to pointer

	dwFlags = SF_RTFNOOBJS | SFF_PLAINRTF | SF_USECODEPAGE | (CP_UTF8 << 16);
	SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_STREAMOUT, dwFlags, (LPARAM)& stream);
	return pszText; // pszText contains the text
}
