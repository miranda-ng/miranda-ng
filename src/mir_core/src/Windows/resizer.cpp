/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "../stdafx.h"

#pragma pack(2)

struct START_OF_DLGITEMTEMPLATEEX
{
	uint32_t  helpID;
	uint32_t  exStyle;
	uint32_t  style;
	short  x, y, cx, cy;
	uint32_t  id;
};

struct START_OF_DLGTEMPLATEEX
{
	uint16_t   dlgVer;
	uint16_t   signature;
	uint32_t  helpID;
	uint32_t  exStyle;
	uint32_t  style;
	uint16_t   cDlgItems;
	short  x, y, cx, cy;
};

MIR_CORE_DLL(int) Utils_ResizeDialog(HWND hwndDlg, HINSTANCE hInstance, LPCSTR lpTemplate, DIALOGRESIZERPROC pfnResizer, LPARAM lParam)
{
	DLGTEMPLATE *pTemplate = (DLGTEMPLATE*)LockResource(LoadResource(hInstance, FindResourceA(hInstance, lpTemplate, MAKEINTRESOURCEA(5))));
	START_OF_DLGTEMPLATEEX *pTemplateEx = (START_OF_DLGTEMPLATEEX*)pTemplate;
	int extendedDlg = pTemplateEx->signature == 0xFFFF;
	if (extendedDlg && pTemplateEx->dlgVer != 1)
		return 1;

	PWORD pWord = (extendedDlg) ? (PWORD)(pTemplateEx + 1) : (PWORD)(pTemplate + 1);
	if (*pWord == 0xFFFF) pWord += 2; else while (*pWord++); // menu
	if (*pWord == 0xFFFF) pWord += 2; else while (*pWord++); // class
	while (*pWord++);       // skip title
	if (extendedDlg) {
		if (pTemplateEx->style & DS_SETFONT) {
			pWord += 3;       // font size, weight, italic
			while (*pWord++); // font name
		}
	}
	else {
		if (pTemplate->style & DS_SETFONT) {
			pWord++;          // font size
			while (*pWord++); // font name
		}
	}

	UTILRESIZECONTROL urc;
	urc.cbSize = sizeof(UTILRESIZECONTROL);

	RECT rc;
	rc.left = 0; rc.top = 0;
	if (extendedDlg) {
		rc.right = pTemplateEx->cx;
		rc.bottom = pTemplateEx->cy;
	}
	else {
		rc.right = pTemplate->cx;
		rc.bottom = pTemplate->cy;
	}
	
	MapDialogRect(hwndDlg, &rc);
	urc.dlgOriginalSize.cx = rc.right; urc.dlgOriginalSize.cy = rc.bottom;
	GetClientRect(hwndDlg, &rc);
	urc.dlgNewSize.cx = rc.right; urc.dlgNewSize.cy = rc.bottom;

	int itemCount = (extendedDlg) ? pTemplateEx->cDlgItems : pTemplate->cdit;
	
	HDWP hDwp = BeginDeferWindowPos(itemCount);
	for (int i = 0; i < itemCount; i++) {
		if ((UINT_PTR)pWord & 2) pWord++;       //dword align

		if (extendedDlg) {
			START_OF_DLGITEMTEMPLATEEX *pItemEx = (START_OF_DLGITEMTEMPLATEEX*)pWord;
			pWord = (PWORD)(pItemEx + 1);

			urc.wId = pItemEx->id;
			urc.rcItem.left = pItemEx->x; urc.rcItem.top = pItemEx->y;
			urc.rcItem.right = urc.rcItem.left + pItemEx->cx; urc.rcItem.bottom = urc.rcItem.top + pItemEx->cy;
		}
		else {
			DLGITEMTEMPLATE *pItem = (DLGITEMTEMPLATE*)pWord;
			pWord = (PWORD)(pItem + 1);

			urc.wId = pItem->id;
			urc.rcItem.left = pItem->x; urc.rcItem.top = pItem->y;
			urc.rcItem.right = urc.rcItem.left + pItem->cx; urc.rcItem.bottom = urc.rcItem.top + pItem->cy;
		}
		if (*pWord == 0xFFFF) pWord += 2; else while (*pWord++); // menu
		if (*pWord == 0xFFFF) pWord += 2; else while (*pWord++); // class
		pWord += 1 + (1 + *pWord) / 2;     //creation data

		if (urc.wId == 65535) // using this breaks the dwp, so just ignore it
			continue;  

		MapDialogRect(hwndDlg, &urc.rcItem);
		int procResult = (pfnResizer)(hwndDlg, lParam, &urc);
		if (procResult & RD_ANCHORX_RIGHT) {
			urc.rcItem.left += urc.dlgNewSize.cx - urc.dlgOriginalSize.cx;
			urc.rcItem.right += urc.dlgNewSize.cx - urc.dlgOriginalSize.cx;
		}
		else if (procResult & RD_ANCHORX_WIDTH)
			urc.rcItem.right += urc.dlgNewSize.cx - urc.dlgOriginalSize.cx;
		else if (procResult & RD_ANCHORX_CENTRE) {
			urc.rcItem.left += (urc.dlgNewSize.cx - urc.dlgOriginalSize.cx) / 2;
			urc.rcItem.right += (urc.dlgNewSize.cx - urc.dlgOriginalSize.cx) / 2;
		}
		if (procResult & RD_ANCHORY_BOTTOM) {
			urc.rcItem.top += urc.dlgNewSize.cy - urc.dlgOriginalSize.cy;
			urc.rcItem.bottom += urc.dlgNewSize.cy - urc.dlgOriginalSize.cy;
		}
		else if (procResult & RD_ANCHORY_HEIGHT)
			urc.rcItem.bottom += urc.dlgNewSize.cy - urc.dlgOriginalSize.cy;
		else if (procResult & RD_ANCHORY_CENTRE) {
			urc.rcItem.top += (urc.dlgNewSize.cy - urc.dlgOriginalSize.cy) / 2;
			urc.rcItem.bottom += (urc.dlgNewSize.cy - urc.dlgOriginalSize.cy) / 2;
		}

		HWND hCtrl = GetDlgItem(hwndDlg, urc.wId);
		if (hCtrl != nullptr && urc.wId != UINT(-1))
			hDwp = DeferWindowPos(hDwp, hCtrl, nullptr, urc.rcItem.left, urc.rcItem.top, urc.rcItem.right - urc.rcItem.left, urc.rcItem.bottom - urc.rcItem.top, SWP_NOZORDER);
	}
	EndDeferWindowPos(hDwp);
	return 0;
}
