/*
Favorite Contacts for Miranda IM

Copyright 2007 Victor Pavlychko

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

#include "headers.h"

UINT g_maxItemWidth = 0;
float g_widthMultiplier = 0;

TCHAR g_filter[1024] = { 0 };

HWND g_hwndMenuHost = NULL;

static TCHAR* sttGetGroupName(int id)
{
	if (id == 1) {
		if (g_Options.bUseGroups)
			return TranslateT("<no group>");
		return TranslateT("Favorite Contacts");
	}

	return pcli->pfnGetGroupName(id-1, NULL);
}

static BOOL sttMeasureItem_Group(LPMEASUREITEMSTRUCT lpmis, Options *options)
{
	HDC hdc = GetDC(g_hwndMenuHost);
	HFONT hfntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	TCHAR *name = sttGetGroupName(-INT_PTR(lpmis->itemData));
	if (!options->bSysColors)
		SelectObject(hdc, g_Options.hfntName);

	SIZE sz;
	GetTextExtentPoint32(hdc, name, mir_tstrlen(name), &sz);
	lpmis->itemHeight = sz.cy + 8;
	lpmis->itemWidth = sz.cx + 10;
	SelectObject(hdc, hfntSave);
	ReleaseDC(g_hwndMenuHost, hdc);
	return TRUE;
}

static BOOL sttMeasureItem_Contact(LPMEASUREITEMSTRUCT lpmis, Options *options)
{
	MCONTACT hContact = (MCONTACT)lpmis->itemData;

	lpmis->itemHeight = 4;
	lpmis->itemWidth = 8 + 10;
	lpmis->itemWidth += 20;

	SIZE sz;
	int textWidth = 0;

	HDC hdc = GetDC(g_hwndMenuHost);
	HFONT hfntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

	if (options->bSecondLine) {
		bool bFree = false;
		TCHAR *title = db_get_tsa(hContact, "CList", "StatusMsg");
		if (title == NULL) {
			char *proto = GetContactProto(hContact);
			int status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
			title = pcli->pfnGetStatusModeDescription(status, 0);
		}
		else bFree = true;

		if (!options->bSysColors)
			SelectObject(hdc, g_Options.hfntSecond);
		GetTextExtentPoint32(hdc, title, mir_tstrlen(title), &sz);
		textWidth = sz.cx;
		lpmis->itemHeight += sz.cy + 3;

		if (bFree)
			mir_free(title);
	}

	TCHAR *name = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

	if (!options->bSysColors) SelectObject(hdc, g_Options.hfntName);
	GetTextExtentPoint32(hdc, name, mir_tstrlen(name), &sz);
	textWidth = max(textWidth, sz.cx);

	SelectObject(hdc, hfntSave);
	ReleaseDC(g_hwndMenuHost, hdc);

	lpmis->itemWidth += textWidth;
	lpmis->itemHeight += sz.cy;

	if (options->bAvatars) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
		if (ace && (ace != (AVATARCACHEENTRY *)CALLSERVICE_NOTFOUND)) {
			int avatarWidth = lpmis->itemHeight;
			if (ace->bmWidth < ace->bmHeight)
				avatarWidth = lpmis->itemHeight * ace->bmWidth / ace->bmHeight;

			lpmis->itemWidth += avatarWidth + 5;
		}
	}

	if (lpmis->itemHeight < 18) lpmis->itemHeight = 18;

	return TRUE;
}

BOOL MenuMeasureItem(LPMEASUREITEMSTRUCT lpmis, Options *options)
{
	if (!options)
		options = &g_Options;

	if (!lpmis->itemData)
		return FALSE;

	BOOL res = FALSE;
	if (INT_PTR(lpmis->itemData) < 0)
		res = sttMeasureItem_Group(lpmis, options);
	else if (CallService(MS_DB_CONTACT_IS, lpmis->itemData, 0))
		res = sttMeasureItem_Contact(lpmis, options);

	if (res && (lpmis->itemWidth > g_maxItemWidth)) lpmis->itemWidth = g_maxItemWidth;
	if (res && g_widthMultiplier) lpmis->itemWidth *= g_widthMultiplier;

	return FALSE;
}

static BOOL sttDrawItem_Group(LPDRAWITEMSTRUCT lpdis, Options *options = NULL)
{
	lpdis->rcItem.top++;
	lpdis->rcItem.bottom--;

	HFONT hfntSave = (HFONT)SelectObject(lpdis->hDC, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(lpdis->hDC, TRANSPARENT);
	if (options->bSysColors) {
		FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else {
		HBRUSH hbr = CreateSolidBrush(g_Options.clBackSel);
		FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
		DeleteObject(hbr);
		SetTextColor(lpdis->hDC, g_Options.clLine1Sel);
	}

	TCHAR *name = sttGetGroupName(-INT_PTR(lpdis->itemData));
	if (!options->bSysColors)
		SelectObject(lpdis->hDC, g_Options.hfntName);
	DrawText(lpdis->hDC, name, mir_tstrlen(name), &lpdis->rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(lpdis->hDC, hfntSave);

	return TRUE;
}

void ImageList_DrawDimmed(HIMAGELIST himl, int i, HDC hdc, int left, int top, UINT fStyle)
{
	int dx, dy;
	ImageList_GetIconSize(himl, &dx, &dy);

	HDC dcMem = CreateCompatibleDC(hdc);
	HBITMAP hbm = CreateCompatibleBitmap(hdc, dx, dy);
	HBITMAP hbmOld = (HBITMAP)SelectObject(dcMem, hbm);
	BitBlt(dcMem, 0, 0, dx, dx, hdc, left, top, SRCCOPY);
	ImageList_Draw(himl, i, dcMem, 0, 0, fStyle);
	BLENDFUNCTION bf = { 0 };
	bf.SourceConstantAlpha = 180;
	GdiAlphaBlend(hdc, left, top, dx, dy, dcMem, 0, 0, dx, dy, bf);
	SelectObject(dcMem, hbmOld);
	DeleteObject(hbm);
	DeleteDC(dcMem);
}

static BOOL sttDrawItem_Contact(LPDRAWITEMSTRUCT lpdis, Options *options = NULL)
{
	MCONTACT hContact = (MCONTACT)lpdis->itemData;

	HDC hdcTemp = CreateCompatibleDC(lpdis->hDC);
	HBITMAP hbmTemp = CreateCompatibleBitmap(lpdis->hDC, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top);
	HBITMAP hbmSave = (HBITMAP)SelectObject(hdcTemp, hbmTemp);
	RECT rcSave = lpdis->rcItem;

	OffsetRect(&lpdis->rcItem, -lpdis->rcItem.left, -lpdis->rcItem.top);

	HFONT hfntSave = (HFONT)SelectObject(hdcTemp, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(hdcTemp, TRANSPARENT);
	COLORREF clBack, clLine1, clLine2;
	if (lpdis->itemState & ODS_SELECTED) {
		if (options->bSysColors) {
			FillRect(hdcTemp, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			clBack = GetSysColor(COLOR_HIGHLIGHT);
			clLine1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else {
			clBack = g_Options.clBackSel;
			clLine1 = g_Options.clLine1Sel;
			clLine2 = g_Options.clLine2Sel;
		}
	}
	else {
		if (options->bSysColors) {
			FillRect(hdcTemp, &lpdis->rcItem, GetSysColorBrush(COLOR_MENU));
			clBack = GetSysColor(COLOR_MENU);
			clLine1 = GetSysColor(COLOR_MENUTEXT);
		}
		else {
			clBack = g_Options.clBack;
			clLine1 = g_Options.clLine1;
			clLine2 = g_Options.clLine2;
		}
	}
	if (options->bSysColors) {
		clLine2 = RGB(
			(GetRValue(clLine1) * 66UL + GetRValue(clBack) * 34UL) / 100,
			(GetGValue(clLine1) * 66UL + GetGValue(clBack) * 34UL) / 100,
			(GetBValue(clLine1) * 66UL + GetBValue(clBack) * 34UL) / 100);
	}
	else {
		HBRUSH hbr = CreateSolidBrush(clBack);
		FillRect(hdcTemp, &lpdis->rcItem, hbr);
		DeleteObject(hbr);
	}

	lpdis->rcItem.left += 4;
	lpdis->rcItem.right -= 4;

	lpdis->rcItem.top += 2;
	lpdis->rcItem.bottom -= 2;

	char *proto = GetContactProto(hContact);

	HIMAGELIST hIml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	int iIcon = CallService(MS_CLIST_GETCONTACTICON, hContact, 0);

	if (db_get_dw(hContact, proto, "IdleTS", 0)) {
		ImageList_DrawDimmed(hIml, iIcon, hdcTemp,
									lpdis->rcItem.left, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
									ILD_TRANSPARENT);
	}
	else {
		ImageList_Draw(hIml, iIcon, hdcTemp,
							lpdis->rcItem.left, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
							ILD_TRANSPARENT);
	}

	lpdis->rcItem.left += 20;

	if (options->wMaxRecent && db_get_b(hContact, "FavContacts", "IsFavourite", 0)) {
		DrawIconEx(hdcTemp, lpdis->rcItem.right - 18, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
					  Skin_GetIconByHandle(iconList[0].hIcolib), 16, 16, 0, NULL, DI_NORMAL);
		lpdis->rcItem.right -= 20;
	}

	if (options->bAvatars) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
		if (ace && (ace != (AVATARCACHEENTRY *)CALLSERVICE_NOTFOUND)) {
			int avatarWidth = lpdis->rcItem.bottom - lpdis->rcItem.top;
			if (ace->bmWidth < ace->bmHeight)
				avatarWidth = (lpdis->rcItem.bottom - lpdis->rcItem.top) * ace->bmWidth / ace->bmHeight;

			AVATARDRAWREQUEST avdr = { 0 };
			avdr.cbSize = sizeof(avdr);
			avdr.hContact = hContact;
			avdr.hTargetDC = hdcTemp;
			avdr.rcDraw = lpdis->rcItem;
			if (options->bRightAvatars)
				avdr.rcDraw.left = avdr.rcDraw.right - avatarWidth;
			else
				avdr.rcDraw.right = avdr.rcDraw.left + avatarWidth;
			avdr.dwFlags = AVDRQ_FALLBACKPROTO;
			if (options->bAvatarBorder) {
				avdr.dwFlags |= AVDRQ_DRAWBORDER;
				avdr.clrBorder = clLine1;
				if (options->bNoTransparentBorder)
					avdr.dwFlags |= AVDRQ_HIDEBORDERONTRANSPARENCY;
				if (options->wAvatarRadius) {
					avdr.dwFlags |= AVDRQ_ROUNDEDCORNER;
					avdr.radius = (unsigned char)options->wAvatarRadius;
				}
			}
			avdr.alpha = 255;
			CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdr);

			if (options->bRightAvatars)
				lpdis->rcItem.right += avatarWidth + 5;
			else
				lpdis->rcItem.left += avatarWidth + 5;
		}
	}

	if (true) {
		TCHAR *name = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

		if (!options->bSysColors)
			SelectObject(hdcTemp, g_Options.hfntName);
		SetTextColor(hdcTemp, clLine1);
		DrawText(hdcTemp, name, mir_tstrlen(name), &lpdis->rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_TOP | DT_LEFT);

		SIZE sz;
		GetTextExtentPoint32(hdcTemp, name, mir_tstrlen(name), &sz);
		lpdis->rcItem.top += sz.cy + 3;
	}

	if (options->bSecondLine) {
		bool bFree = false;
		TCHAR *title = db_get_tsa(hContact, "CList", "StatusMsg");
		if (title == NULL) {
			int status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
			title = pcli->pfnGetStatusModeDescription(status, 0);
		}
		else bFree = true;

		if (!options->bSysColors) SelectObject(hdcTemp, g_Options.hfntSecond);
		SetTextColor(hdcTemp, clLine2);
		DrawText(hdcTemp, title, mir_tstrlen(title), &lpdis->rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_TOP | DT_LEFT);

		if (bFree) mir_free(title);
	}

	SelectObject(hdcTemp, hfntSave);
	BitBlt(lpdis->hDC, rcSave.left, rcSave.top, rcSave.right - rcSave.left, rcSave.bottom - rcSave.top, hdcTemp, 0, 0, SRCCOPY);

	SelectObject(hdcTemp, hbmSave);
	DeleteObject(hbmTemp);
	DeleteDC(hdcTemp);

	return TRUE;
}

BOOL MenuDrawItem(LPDRAWITEMSTRUCT lpdis, Options *options)
{
	if (!options)
		options = &g_Options;

	if (!lpdis->itemData)
		return FALSE;

	if (INT_PTR(lpdis->itemData) < 0)
		return sttDrawItem_Group(lpdis, options);

	if (CallService(MS_DB_CONTACT_IS, lpdis->itemData, 0))
		return sttDrawItem_Contact(lpdis, options);

	return FALSE;
}

static LRESULT CALLBACK MenuHostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hContact = NULL;

	switch (message) {
	case WM_MEASUREITEM:
	{
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
		if (lpmis->CtlType != ODT_MENU)
			return FALSE;

		if ((lpmis->itemID >= CLISTMENUIDMIN) && (lpmis->itemID <= CLISTMENUIDMAX))
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

		return MenuMeasureItem(lpmis);
	}

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->CtlType != ODT_MENU)
			return FALSE;

		if ((lpdis->itemID >= CLISTMENUIDMIN) && (lpdis->itemID <= CLISTMENUIDMAX))
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

		return MenuDrawItem(lpdis);
	}

	case WM_MENUCHAR:
		while (GetMenuItemCount((HMENU)lParam) > 1)
			RemoveMenu((HMENU)lParam, 1, MF_BYPOSITION);

		if (LOWORD(wParam) == VK_BACK) {
			if (int l = mir_tstrlen(g_filter))
				g_filter[l - 1] = 0;
		}
		else if (_istalnum(LOWORD(wParam))) {
			if (mir_tstrlen(g_filter) < SIZEOF(g_filter) - 1) {
				TCHAR s[] = { LOWORD(wParam), 0 };
				mir_tstrcat(g_filter, s);
			}
		}
		{
			int maxRecent = g_Options.wMaxRecent ? g_Options.wMaxRecent : 10;
			for (int i = 0, nRecent = 0; nRecent < maxRecent; ++i) {
				MCONTACT hContact = g_contactCache->get(i);
				if (!hContact) break;
				if (!g_contactCache->filter(i, g_filter)) continue;

				AppendMenu((HMENU)lParam, MF_OWNERDRAW, nRecent + 1, (LPCTSTR)hContact);
				++nRecent;
			}
		}
		return MAKELRESULT(1, MNC_SELECT);

	case WM_MENURBUTTONUP:
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo((HMENU)lParam, wParam, TRUE, &mii);
		MCONTACT hContact = (MCONTACT)mii.dwItemData;
		if (!CallService(MS_DB_CONTACT_IS, mii.dwItemData, 0))
			return FALSE;

		HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);

		POINT pt;
		GetCursorPos(&pt);
		HWND hwndSave = GetForegroundWindow();
		SetForegroundWindow(g_hwndMenuHost);
		int res = TrackPopupMenu(hMenu, TPM_RECURSE | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, g_hwndMenuHost, NULL);
		SetForegroundWindow(hwndSave);
		DestroyMenu(hMenu);

		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(res, MPCF_CONTACTMENU), hContact);
		return TRUE;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int ShowMenu(bool centered)
{
	HMENU hMenu = CreatePopupMenu();
	SIZE szMenu = { 0 };
	SIZE szColumn = { 0 };
	TCHAR *prevGroup = NULL;
	int idItem = 100;
	MCONTACT hContact;

	TFavContacts favList;
	favList.build();

	g_widthMultiplier = 0;

	g_maxItemWidth = GetSystemMetrics(SM_CXSCREEN);
	if (g_Options.bUseColumns)
		g_maxItemWidth /= favList.groupCount();

	prevGroup = NULL;
	for (int i = 0; i < favList.getCount(); ++i) {
		hContact = favList[i]->getHandle();

		MEASUREITEMSTRUCT mis = { 0 };
		mis.CtlID = 0;
		mis.CtlType = ODT_MENU;

		if (!prevGroup || mir_tstrcmp(prevGroup, favList[i]->getGroup())) {
			if (prevGroup && g_Options.bUseColumns) {
				szMenu.cx += szColumn.cx;
				szMenu.cy = max(szMenu.cy, szColumn.cy);
				szColumn.cx = szColumn.cy = 0;
			}

			int groupID = -((int)Clist_GroupExists(favList[i]->getGroup()) + 1);

			AppendMenu(hMenu,
						  MF_OWNERDRAW | MF_SEPARATOR | ((prevGroup && g_Options.bUseColumns) ? MF_MENUBREAK : 0),
						  ++idItem, (LPCTSTR)groupID);

			mis.itemData = groupID;
			mis.itemID = idItem;
			MenuMeasureItem(&mis);
			szColumn.cx = max(szColumn.cx, (int)mis.itemWidth);
			szColumn.cy += mis.itemHeight;
		}

		AppendMenu(hMenu, MF_OWNERDRAW, ++idItem, (LPCTSTR)hContact);

		mis.itemData = (DWORD)hContact;
		mis.itemID = idItem;
		MenuMeasureItem(&mis);
		szColumn.cx = max(szColumn.cx, (int)mis.itemWidth);
		szColumn.cy += mis.itemHeight;

		prevGroup = favList[i]->getGroup();
	}
	szMenu.cx += szColumn.cx;
	szMenu.cy = max(szMenu.cy, szColumn.cy);
	szColumn.cx = szColumn.cy = 0;

	int maxWidth = GetSystemMetrics(SM_CXSCREEN) * db_get_b(NULL, "FavContacts", "MenuWidth", 66) / 100;
	if (szMenu.cx > maxWidth) {
		g_widthMultiplier = (float)maxWidth / szMenu.cx;
		szMenu.cx *= g_widthMultiplier;
	}

	POINT pt;
	if (centered) {
		if ((pt.x = (GetSystemMetrics(SM_CXSCREEN) - szMenu.cx) / 2) < 0) pt.x = 0;
		if ((pt.y = (GetSystemMetrics(SM_CYSCREEN) - szMenu.cy) / 2) < 0) pt.y = 0;
	}
	else GetCursorPos(&pt);

	HWND hwndSave = GetForegroundWindow();
	SetForegroundWindow(g_hwndMenuHost);
	hContact = NULL;
	g_filter[0] = 0;

	if (int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, g_hwndMenuHost, NULL)) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(hMenu, res, FALSE, &mii);
		hContact = (MCONTACT)mii.dwItemData;
	}
	SetForegroundWindow(hwndSave);
	DestroyMenu(hMenu);

	if (hContact)
		CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);

	return 0;
}

void InitMenu()
{
	WNDCLASSEX wcl = { sizeof(wcl) };
	wcl.lpfnWndProc = MenuHostWndProc;
	wcl.hInstance = g_hInst;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszClassName = _T("FavContactsMenuHostWnd");
	RegisterClassEx(&wcl);

	g_hwndMenuHost = CreateWindow(_T("FavContactsMenuHostWnd"), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInst, NULL);
	SetWindowPos(g_hwndMenuHost, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);
}

void UninitMenu()
{
	if (g_hwndMenuHost)
		DestroyWindow(g_hwndMenuHost);
}
