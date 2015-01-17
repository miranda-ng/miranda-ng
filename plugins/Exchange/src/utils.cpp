/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

#include "commonheaders.h"
#include "utils.h"

#define ___DEB

int LogInit()
{
#ifdef ___DEBUGG
	FILE *fout = fopen(LOG_FILE, "wt");
	fclose(fout);
#endif
	return 0;
}

int Log(char *format, ...)
{
#ifdef ___DEBUGG
	char		str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen(LOG_FILE, "at");
	if (!fout)
		{
//			MessageBox(0, "can't open file", NULL, MB_OK);
			return -1;
		}

	va_start(vararg, format);

	tBytes = mir_vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		{
			str[tBytes] = 0;
		}

	va_end(vararg);
	if (str[strlen(str) - 1] != '\n')
		{
			strcat(str, "\n");
		}
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
	return MessageBoxA(0, str, title, MB_OK | MB_ICONINFORMATION);
}

#define HEX_SIZE 8

char *BinToHex(int size, PBYTE data)
{
	char *szresult = NULL;
	int maxSize = size * 2 + HEX_SIZE + 1;
	szresult = (char *) new char[ maxSize ];
	mir_snprintf(szresult, maxSize, "%0*X", HEX_SIZE, size);
	bin2hex(data, size, szresult + HEX_SIZE);
	return szresult;
}

void HexToBin(TCHAR *inData, ULONG &size, LPBYTE &outData)
{
	TCHAR buffer[32] = {0};
	_tcsncpy(buffer, _T("0x"),_countof(_T("0x")));
	_tcsncpy(buffer + 2, inData, HEX_SIZE);
	_stscanf(buffer, _T("%x"), &size);
	outData = (unsigned char*)new char[size*2];

	TCHAR *tmp = inData + HEX_SIZE;
	buffer[4] = '\0'; //mark the end of the string
	for (UINT i = 0; i < size; i++) {
		_tcsncpy(buffer + 2, &tmp[i * 2], 2);
		_stscanf(buffer, _T("%x"), &outData[i]);
	}
}

int GetStringFromDatabase(char *szSettingName, TCHAR *szError, TCHAR *szResult, int size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	int len;
	dbv.type = DBVT_ASCIIZ;
	if (db_get_ts(NULL, ModuleName, szSettingName, &dbv) == 0) {
		res = 0;
		int tmp = _tcslen(dbv.ptszVal);
		len = (tmp < size - 1) ? tmp : size - 1;
		_tcsncpy(szResult, dbv.ptszVal, len);
		szResult[len] = '\0';
		mir_free(dbv.ptszVal);
	}
	else {
		res = 1;
		int tmp = _tcslen(szError);
		len = (tmp < size - 1) ? tmp : size - 1;
		_tcsncpy(szResult, szError, len);
		szResult[len] = '\0';
	}
	return res;
}

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

DWORD WINAPI CheckEmailWorkerThread(LPVOID data)
{
	EnterCriticalSection(&csCheck);

	int bForceAttempt = (int) data;

	if (!exchangeServer.IsConnected())
		exchangeServer.Connect(bForceAttempt);

	exchangeServer.Check(bForceAttempt);

	LeaveCriticalSection(&csCheck);

	return 0;
}

int ThreadCheckEmail(int bForceAttempt)
{
	DWORD idThread;
	HANDLE hCheckThread = CreateThread(NULL, NULL, CheckEmailWorkerThread, (void *) bForceAttempt, 0, &idThread);
	CloseHandle(hCheckThread);

	return 0;
}

void _popupUtil(char* szMsg)
{
	POPUPDATAT ppd = {0};
	ppd.lchIcon = hiMailIcon;
	_tcscpy(ppd.lptzContactName, _T("Exchange notifier"));
	_tcscpy(ppd.lptzText, mir_a2t(szMsg));
	PUAddPopupT(&ppd); //show a popup to tell the user what we're doing.
}
