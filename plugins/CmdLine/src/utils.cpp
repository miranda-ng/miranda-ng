/*
CmdLine plugin for Miranda IM

Copyright © 2007 Cristian Libotean

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

int Info(char *title, char *format, ...)
{
	char str[4096];
	va_list vararg;
	int tBytes;
	va_start(vararg, format);
	tBytes = mir_vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		{
			str[tBytes] = 0;
		}
	va_end(vararg);
	return MessageBoxA(0, str, title, MB_OK | MB_ICONINFORMATION);
}

int MyPUShowMessage(char *lpzText, BYTE kind)
{
	if (ServiceExists(MS_POPUP_SHOWMESSAGE))
		return PUShowMessage(lpzText, kind);

	char *title = (kind == SM_NOTIFY) ? Translate("Notify") : Translate("Warning");
	return MessageBoxA(NULL, lpzText, title, MB_OK | (kind == SM_NOTIFY) ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
}

#define HEX_SIZE 8

char *BinToHex(int size, PBYTE data)
{
	char *szresult = NULL;
	int maxSize = size * 2 + HEX_SIZE + 1;
	szresult = (char *) new char[ maxSize ];
	mir_snprintf(szresult, maxSize, "%0*X", HEX_SIZE, size);
	return bin2hex(data, size, szresult + HEX_SIZE);
}

void HexToBin(char *inData, ULONG &size, LPBYTE &outData)
{
	char buffer[32] = {0};
	strcpy(buffer, "0x");
	strncpy_s(buffer + 2, HEX_SIZE, inData, _TRUNCATE);
	sscanf(buffer, "%x", &size);
	outData = (unsigned char*)new char[size*2];

	char *tmp = inData + HEX_SIZE;
	buffer[4] = '\0'; //mark the end of the string
	for (UINT i = 0; i < size; i++) {
		strncpy_s(buffer + 2, 3, &tmp[i*2], _TRUNCATE);
		sscanf(buffer, "%x", &outData[i]);
	}
}

int GetStringFromDatabase(MCONTACT hContact, char *szModule, char *szSettingName, char *szError, char *szResult, size_t size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_ASCIIZ;
	if (db_get(hContact, szModule, szSettingName, &dbv) == 0)
		{
			res = 0;
			size_t tmp = mir_strlen(dbv.pszVal);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, dbv.pszVal, len);
			szResult[len] = '\0';
			mir_free(dbv.pszVal);
		}
		else{
			res = 1;
			if (szError)
				{
					size_t tmp = mir_strlen(szError);
					len = (tmp < size - 1) ? tmp : size - 1;
					strncpy(szResult, szError, len);
					szResult[len] = '\0';
				}
				else{
					szResult[0] = '\0';
				}
		}
	return res;
}

int GetStringFromDatabase(MCONTACT hContact, char *szModule, char *szSettingName, WCHAR *szError, WCHAR *szResult, size_t count)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_WCHAR;
	if (db_get_ws(hContact, szModule, szSettingName, &dbv) == 0)
		{
			res = 0;
			if (dbv.type != DBVT_WCHAR)
			{
				MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, szResult, (int) count);
			}
			else{
				size_t tmp = mir_wstrlen(dbv.pwszVal);
				len = (tmp < count - 1) ? tmp : count - 1;
				wcsncpy(szResult, dbv.pwszVal, len);
				szResult[len] = L'\0';
			}
			mir_free(dbv.pwszVal);
		}
		else{
			res = 1;
			if (szError)
				{
					size_t tmp = mir_wstrlen(szError);
					len = (tmp < count - 1) ? tmp : count - 1;
					wcsncpy(szResult, szError, len);
					szResult[len] = L'\0';
				}
				else{
					szResult[0] = L'\0';
				}
		}
	return res;
}

int GetStringFromDatabase(char *szSettingName, char *szError, char *szResult, size_t size)
{
	return GetStringFromDatabase(NULL, ModuleName, szSettingName, szError, szResult, size);
}

int GetStringFromDatabase(char *szSettingName, WCHAR *szError, WCHAR *szResult, size_t count)
{
	return GetStringFromDatabase(NULL, ModuleName, szSettingName, szError, szResult, count);
}

#pragma warning (disable: 4312)
char* GetContactName(MCONTACT hContact, char *szProto)
{
	char proto[200];

	CONTACTINFO ctInfo;
	memset(&ctInfo, 0, sizeof(ctInfo));
	ctInfo.cbSize = sizeof(ctInfo);
	if (szProto)
		ctInfo.szProto = szProto;
	else {
		GetContactProto(hContact, proto, sizeof(proto));
		ctInfo.szProto = proto;
	}
	ctInfo.dwFlag = CNF_DISPLAY;
	ctInfo.hContact = hContact;

	//_debug_message("retrieving contact name for %d", hContact);
	INT_PTR ret = CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) &ctInfo);
	if (ret)
		return NULL;

	//_debug_message("	contact name %s", ctInfo.pszVal);
	char *buffer = strdup((char*)ctInfo.pszVal);
	mir_free(ctInfo.pszVal);
	return buffer;
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
void GetContactProto(MCONTACT hContact, char *szProto, size_t size)
{
	GetStringFromDatabase(hContact, "Protocol", "p", NULL, szProto, size);
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
char* GetContactID(MCONTACT hContact)
{
	char protocol[256];
	GetContactProto(hContact, protocol, sizeof(protocol));

	return GetContactID(hContact, protocol);
}

char* GetContactID(MCONTACT hContact, char *szProto)
{
	CONTACTINFO ctInfo;
	memset(&ctInfo, 0, sizeof(ctInfo));
	ctInfo.cbSize = sizeof(ctInfo);
	ctInfo.szProto = szProto;
	ctInfo.dwFlag = CNF_UNIQUEID;
	ctInfo.hContact = hContact;
	INT_PTR ret = CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) &ctInfo);
	if (ret)
		return NULL;

	char *buffer;
	char tmp[16];
	switch (ctInfo.type) {
	case CNFT_BYTE:
		mir_snprintf(tmp, _countof(tmp), "%d", ctInfo.bVal);
		buffer = strdup(tmp);
		break;

	case CNFT_WORD:
		mir_snprintf(tmp, _countof(tmp), "%d", ctInfo.wVal);
		buffer = strdup(tmp);
		break;

	case CNFT_DWORD:
		mir_snprintf(tmp, _countof(tmp), "%ld", ctInfo.dVal);
		buffer = strdup(tmp);
		break;

	case CNFT_ASCIIZ:
	default:
		buffer = _strdup((char*)ctInfo.pszVal);
		break;
	}

	mir_free(ctInfo.pszVal);
	return buffer;
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
MCONTACT GetContactFromID(char *szID, char *szProto)
{
	char dispName[1024];
	char cProtocol[256];

	int found = 0;
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		GetContactProto(hContact, cProtocol, sizeof(cProtocol));
		char *szHandle = GetContactID(hContact, cProtocol);

		char *tmp = (char*) pcli->pfnGetContactDisplayName(hContact, 0);
		strncpy_s(dispName, tmp, _TRUNCATE);

		if ((szHandle) && ((mir_strcmpi(szHandle, szID) == 0) || (mir_strcmpi(dispName, szID) == 0)) && ((szProto == NULL) || (_stricmp(szProto, cProtocol) == 0)))
			found = 1;

		free(szHandle);

		if (found)
			return hContact;
	}

	return NULL;
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
MCONTACT GetContactFromID(char *szID, wchar_t *szProto)
{
	char protocol[1024];
	WideCharToMultiByte(CP_ACP, 0, szProto, -1, protocol, sizeof(protocol), NULL, NULL);
	return GetContactFromID(szID, protocol);
}
#pragma warning (default: 4312)

void ScreenToClient(HWND hWnd, LPRECT rect)
{
	POINT pt;
	int cx = rect->right - rect->left;
	int cy = rect->bottom - rect->top;
	pt.x = rect->left;
	pt.y = rect->top;
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;
	rect->right = pt.x + cx;
	rect->bottom = pt.y + cy;
}

void AnchorMoveWindow(HWND window, const WINDOWPOS *parentPos, int anchors)
{
	RECT rParent;
	RECT rChild;

	if (parentPos->flags & SWP_NOSIZE)
		return;

	GetWindowRect(parentPos->hwnd, &rParent);
	rChild = AnchorCalcPos(window, &rParent, parentPos, anchors);
	MoveWindow(window, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, FALSE);
}

RECT AnchorCalcPos(HWND window, const RECT *rParent, const WINDOWPOS *parentPos, int anchors)
{
	RECT rChild;
	RECT rTmp;

	GetWindowRect(window, &rChild);
	ScreenToClient(parentPos->hwnd, &rChild);

	int cx = rParent->right - rParent->left;
	int cy = rParent->bottom - rParent->top;
	if ((cx == parentPos->cx) && (cy == parentPos->cy))
		return rChild;

	if (parentPos->flags & SWP_NOSIZE)
		return rChild;

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
