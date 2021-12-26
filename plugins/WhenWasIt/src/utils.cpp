/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#include "stdafx.h"

int LogInit()
{
#ifdef _DEBUG
	FILE *fout = fopen(LOG_FILE, "wt");
	fclose(fout);
#endif
	return 0;
}

int Log(char *format, ...)
{
#ifdef _DEBUG
	char		str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen(LOG_FILE, "at");
	if (!fout)
		return -1;

	time_t tNow = time(0);
	struct tm *now = localtime(&tNow);
	strftime(str, sizeof(str), "%d %b %Y @ %H:%M:%S: ", now);
	fputs(str, fout);
	va_start(vararg, format);

	tBytes = mir_vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		str[tBytes] = 0;

	va_end(vararg);
	if (str[mir_strlen(str) - 1] != '\n')
		mir_strcat(str, "\n");
	fputs(str, fout);
	fclose(fout);
#endif
	return 0;
}

int Info(char *title, char *format, ...)
{
	char str[4096];
	va_list vararg;
	int tBytes;
	va_start(vararg, format);
	tBytes = mir_vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		str[tBytes] = 0;
	va_end(vararg);
	return MessageBoxA(nullptr, str, title, MB_OK | MB_ICONINFORMATION);
}

#define HEX_SIZE 8

char* BinToHex(int size, uint8_t *data)
{
	char *szresult = nullptr;
	char buffer[32] = { 0 }; //should be more than enough
	int maxSize = size * 2 + HEX_SIZE + 1;
	szresult = (char *) new char[maxSize];
	memset(szresult, 0, maxSize);
	mir_snprintf(buffer, "%0*X", HEX_SIZE, size);
	mir_strcpy(szresult, buffer);

	for (int i = 0; i < size; i++) {
		mir_snprintf(buffer, "%02X", data[i]);
		mir_strcpy(szresult + (HEX_SIZE + i * 2), buffer);
	}
	return szresult;
}

void HexToBin(char *inData, ULONG &size, LPBYTE &outData)
{
	char buffer[32] = { 0 };
	mir_strcpy(buffer, "0x");
	strncpy(buffer + 2, inData, HEX_SIZE);
	sscanf(buffer, "%x", &size);
	outData = (unsigned char*)new char[size * 2];

	char *tmp = inData + HEX_SIZE;
	buffer[4] = '\0'; //mark the end of the string
	for (UINT i = 0; i < size; i++) {
		strncpy(buffer + 2, &tmp[i * 2], 2);
		sscanf(buffer, "%hhx", &outData[i]);
	}
}

int GetStringFromDatabase(MCONTACT hContact, char *szModule, char *szSettingName, char *szError, char *szResult, size_t size)
{
	DBVARIANT dbv = { 0 };
	int res = 1;
	size_t len;
	dbv.type = DBVT_ASCIIZ;
	if (db_get(hContact, szModule, szSettingName, &dbv) == 0) {
		res = 0;
		size_t tmp = mir_strlen(dbv.pszVal);
		len = (tmp < size - 1) ? tmp : size - 1;
		strncpy(szResult, dbv.pszVal, len);
		szResult[len] = '\0';
		mir_free(dbv.pszVal);
	}
	else {
		res = 1;
		if (szError) {
			size_t tmp = mir_strlen(szError);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, szError, len);
			szResult[len] = '\0';
		}
		else szResult[0] = '\0';
	}
	return res;
}

int GetStringFromDatabase(char *szSettingName, char *szError, char *szResult, size_t size)
{
	return GetStringFromDatabase(0, MODULENAME, szSettingName, szError, szResult, size);
}

wchar_t* GetContactID(MCONTACT hContact)
{
	return GetContactID(hContact, nullptr);
}

wchar_t* GetContactID(MCONTACT hContact, char *szProto)
{
	ptrW res(Contact_GetInfo(CNF_UNIQUEID, hContact, szProto));
	return (res) ? wcsdup(res) : nullptr;
}

MCONTACT GetContactFromID(wchar_t *szID, char *szProto)
{
	for (auto &hContact : Contacts()) {
		char *m_szProto = Proto_GetBaseAccountName(hContact);
		wchar_t *szHandle = GetContactID(hContact, szProto);
		if (szHandle) {
			bool found = (!mir_wstrcmpi(szHandle, szID) && !_stricmp(szProto, m_szProto));
			free(szHandle);
			if (found)
				return hContact;
		}
	}
	return NULL;
}

MCONTACT GetContactFromID(wchar_t *szID, wchar_t *szProto)
{
	char protocol[1024];
	WideCharToMultiByte(CP_ACP, 0, szProto, -1, protocol, sizeof(protocol), nullptr, nullptr);
	return GetContactFromID(szID, protocol);
}

void ScreenToClient(HWND hWnd, LPRECT rect)
{
	int cx = rect->right - rect->left;
	int cy = rect->bottom - rect->top;
	POINT pt = { rect->left, rect->top };
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;
	rect->right = pt.x + cx;
	rect->bottom = pt.y + cy;
}

void AnchorMoveWindow(HWND window, const WINDOWPOS *parentPos, int anchors)
{
	if (parentPos->flags & SWP_NOSIZE)
		return;

	RECT rParent;
	GetWindowRect(parentPos->hwnd, &rParent);

	RECT rChild = AnchorCalcPos(window, &rParent, parentPos, anchors);
	MoveWindow(window, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, FALSE);
}

RECT AnchorCalcPos(HWND window, const RECT *rParent, const WINDOWPOS *parentPos, int anchors)
{
	RECT rChild;
	GetWindowRect(window, &rChild);
	ScreenToClient(parentPos->hwnd, &rChild);

	int cx = rParent->right - rParent->left;
	int cy = rParent->bottom - rParent->top;
	if ((cx == parentPos->cx) && (cy == parentPos->cy))
		return rChild;

	if (parentPos->flags & SWP_NOSIZE)
		return rChild;

	RECT rTmp;
	rTmp.left = parentPos->x - rParent->left;
	rTmp.right = (parentPos->x + parentPos->cx) - rParent->right;
	rTmp.bottom = (parentPos->y + parentPos->cy) - rParent->bottom;
	rTmp.top = parentPos->y - rParent->top;

	cx = (rTmp.left) ? -rTmp.left : rTmp.right;
	cy = (rTmp.top) ? -rTmp.top : rTmp.bottom;

	rChild.right += cx;
	rChild.bottom += cy;
	//expanded the window accordingly, now we need to enforce the anchors
	if ((anchors & ANCHOR_LEFT) && (!(anchors & ANCHOR_RIGHT)))
		rChild.right -= cx;

	if ((anchors & ANCHOR_TOP) && (!(anchors & ANCHOR_BOTTOM)))
		rChild.bottom -= cy;

	if ((anchors & ANCHOR_RIGHT) && (!(anchors & ANCHOR_LEFT)))
		rChild.left += cx;

	if ((anchors & ANCHOR_BOTTOM) && (!(anchors & ANCHOR_TOP)))
		rChild.top += cy;

	return rChild;
}
