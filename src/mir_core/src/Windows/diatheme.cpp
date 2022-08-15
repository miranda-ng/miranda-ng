/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../stdafx.h"
#include "diatheme.h"

// The following code was borrowed from Notepad2 sources and adapted for Miranda NG

typedef HTHEME (WINAPI *OTD)(HWND hwnd,LPCWSTR pszClassList);
typedef HRESULT (WINAPI *GTSF)(HTHEME hTheme,int iFontId,LOGFONT *plf);
typedef HRESULT (WINAPI *CTD)(HTHEME hTheme);

BOOL GetThemedDialogFont(LPWSTR lpFaceName, WORD *wSize)
{
	BOOL bSucceed = FALSE;

	HDC hDC = GetDC(nullptr);
	int iLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(nullptr, hDC);

	if (HMODULE hModUxTheme = GetModuleHandle(L"uxtheme.dll")) {
		OTD _OpenThemeData = (OTD)GetProcAddress(hModUxTheme, "OpenThemeData");
		GTSF _GetThemeSysFont = (GTSF)GetProcAddress(hModUxTheme, "GetThemeSysFont");
		CTD _CloseThemeData = (CTD)GetProcAddress(hModUxTheme, "CloseThemeData");

		if (_CloseThemeData && _GetThemeSysFont && _OpenThemeData) {
			if (HTHEME hTheme = _OpenThemeData(NULL, L"WINDOWSTYLE;WINDOW")) {
				LOGFONT lf;
				if (S_OK == _GetThemeSysFont(hTheme,/*TMT_MSGBOXFONT*/805, &lf)) {
					if (lf.lfHeight < 0)
						lf.lfHeight = -lf.lfHeight;
					*wSize = (WORD)MulDiv(lf.lfHeight, 72, iLogPixelsY);
					if (*wSize == 0)
						*wSize = 8;
					wcsncpy_s(lpFaceName, LF_FACESIZE, lf.lfFaceName, _TRUNCATE);
					bSucceed = TRUE;
				}
				_CloseThemeData(hTheme);
			}
		}
	}

	if (!bSucceed) {
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		if (ncm.lfMessageFont.lfHeight < 0)
			ncm.lfMessageFont.lfHeight = -ncm.lfMessageFont.lfHeight;
		*wSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight, 72, iLogPixelsY);
		if (*wSize == 0)
			*wSize = 8;
			
		wcsncpy_s(lpFaceName, LF_FACESIZE, ncm.lfMessageFont.lfFaceName, _TRUNCATE);
	}

	return TRUE;
}

BOOL DialogTemplate_IsDialogEx(const DLGTEMPLATE *pTemplate)
{
	return ((DLGTEMPLATEEX *)pTemplate)->signature == 0xFFFF;
}

BOOL DialogTemplate_HasFont(const DLGTEMPLATE *pTemplate)
{
	return (DS_SETFONT & (DialogTemplate_IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX *)pTemplate)->style : pTemplate->style));
}

int DialogTemplate_FontAttrSize(BOOL bDialogEx)
{
	return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
}

BYTE *DialogTemplate_GetFontSizeField(const DLGTEMPLATE *pTemplate)
{
	BOOL bDialogEx = DialogTemplate_IsDialogEx(pTemplate);

	WORD *pw;
	if (bDialogEx)
		pw = (WORD *)((DLGTEMPLATEEX *)pTemplate + 1);
	else
		pw = (WORD *)(pTemplate + 1);

	if (*pw == (WORD)-1)
		pw += 2;
	else
		while (*pw++);

	if (*pw == (WORD)-1)
		pw += 2;
	else
		while (*pw++);

	while (*pw++);

	return (BYTE *)pw;
}

DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance)
{
	WCHAR wchFaceName[LF_FACESIZE];

	HRSRC hRsrc = FindResource(hInstance, lpDialogTemplateID, RT_DIALOG);
	if (hRsrc == nullptr)
		return nullptr;

	HGLOBAL hRsrcMem = LoadResource(hInstance, hRsrc);
	if (hRsrcMem == nullptr)
		return nullptr;

	DLGTEMPLATE *pRsrcMem = (DLGTEMPLATE *)LockResource(hRsrcMem);
	if (pRsrcMem == nullptr)
		return nullptr;

	size_t dwTemplateSize = (UINT)SizeofResource(hInstance, hRsrc);
	UnlockResource(hRsrcMem);
	FreeResource(hRsrcMem);

	if (dwTemplateSize == 0)
		return nullptr;

	auto *pTemplate = (DLGTEMPLATE *)mir_alloc(dwTemplateSize + LF_FACESIZE * 2);
	memcpy(pTemplate, pRsrcMem, dwTemplateSize);
	UnlockResource(hRsrcMem);
	FreeResource(hRsrcMem);

	WORD wFontSize;
	if (!GetThemedDialogFont(wchFaceName, &wFontSize))
		return(pTemplate);

	BOOL bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
	BOOL bHasFont = DialogTemplate_HasFont(pTemplate);
	size_t cbFontAttr = DialogTemplate_FontAttrSize(bDialogEx);

	if (bDialogEx)
		((DLGTEMPLATEEX *)pTemplate)->style |= DS_SHELLFONT;
	else
		pTemplate->style |= DS_SHELLFONT;

	size_t cbNew = cbFontAttr + ((mir_wstrlen(wchFaceName) + 1) * sizeof(WCHAR));
	BYTE *pbNew = (BYTE *)wchFaceName;

	BYTE *pb = DialogTemplate_GetFontSizeField(pTemplate);
	size_t cbOld = (int)(bHasFont ? cbFontAttr + 2 * (mir_wstrlen((WCHAR *)(pb + cbFontAttr)) + 1) : 0);

	BYTE *pOldControls = (BYTE *)(((DWORD_PTR)pb + cbOld + 3) & ~(DWORD_PTR)3);
	BYTE *pNewControls = (BYTE *)(((DWORD_PTR)pb + cbNew + 3) & ~(DWORD_PTR)3);

	WORD nCtrl = bDialogEx ? (WORD)((DLGTEMPLATEEX *)pTemplate)->cDlgItems : (WORD)pTemplate->cdit;
	if (cbNew != cbOld && nCtrl > 0)
		MoveMemory(pNewControls, pOldControls, dwTemplateSize - (pOldControls - (BYTE *)pTemplate));

	*(WORD *)pb = wFontSize;
	MoveMemory(pb + cbFontAttr, pbNew, cbNew - cbFontAttr);
	return pTemplate;
}
