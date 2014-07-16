/*
Miranda IM
Copyright (C) 2002 Robert Rainwater

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

#include <m_button_int.h>
#include <m_toptoolbar.h>

HWND g_hwndToolbarFrame;
HANDLE hToolbarFrame = (HANDLE)-1;

struct CluiTopButton BTNS[] = 
{
	{ IDC_TBTOPMENU,           "CLN_topmenu",      NULL, LPGEN("Show menu"),             1, 1, 1 },
	{ IDC_TBHIDEOFFLINE,       "CLN_online",       NULL, LPGEN("Hide offline contacts"), 0, 1, 0 },
	{ IDC_TBHIDEGROUPS,        "CLN_groups",       NULL, LPGEN("Enable group mode"),     0, 1, 0 },
	{ IDC_TBFINDANDADD,        "CLN_findadd",      NULL, LPGEN("Find and add contacts"), 1, 1, 0 },
	{ IDC_TBACCOUNTS,          "CLN_accounts",     NULL, LPGEN("Accounts"),              1, 1, 0 },
	{ IDC_TBOPTIONS,           "CLN_options",      NULL, LPGEN("Open preferences"),      1, 1, 0 },
	{ IDC_TBSOUND,         "CLN_sound", "CLN_soundsoff", LPGEN("Disable sounds"),        0, 1, 0 },
	{ IDC_TBMINIMIZE,          "CLN_minimize",     NULL, LPGEN("Minimize contact list"), 1, 0, 0 },
	{ IDC_TBTOPSTATUS,         "CLN_topstatus",    NULL, LPGEN("Status menu"),           1, 0, 1 },

	{ IDC_TBSELECTVIEWMODE,    "CLN_CLVM_select",  NULL, LPGEN("Select view mode"),      1, 0, 1 },
	{ IDC_TBCONFIGUREVIEWMODE, "CLN_CLVM_options", NULL, LPGEN("Setup view modes"),      1, 0, 0 },
	{ IDC_TBCLEARVIEWMODE,     "CLN_CLVM_reset",   NULL, LPGEN("Clear view mode"),       1, 0, 0 }
};

static int g_index = -1;

static void InitDefaultButtons()
{
	for (int i = 0; i < SIZEOF(BTNS); i++ ) {
		TTBButton tbb = { sizeof(tbb) };

		g_index = i;
		if (BTNS[i].pszButtonID) {
			if (!BTNS[i].isPush)
				tbb.dwFlags |= TTBBF_ASPUSHBUTTON;

			tbb.pszTooltipUp = tbb.name = LPGEN(BTNS[i].pszButtonName);
			tbb.hIconHandleUp = Skin_GetIconHandle(BTNS[i].pszButtonID);
			if (BTNS[i].pszButtonDn)
				tbb.hIconHandleDn = Skin_GetIconHandle(BTNS[i].pszButtonDn);
		}
		else tbb.dwFlags |= TTBBF_ISSEPARATOR;

		tbb.dwFlags |= (BTNS[i].isVis ? TTBBF_VISIBLE : 0);
		BTNS[i].hButton = TopToolbar_AddButton(&tbb);
	}
	g_index = -1;

	ClcSetButtonState(IDC_TBHIDEOFFLINE, db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT));
	ClcSetButtonState(IDC_TBHIDEGROUPS, db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT));
	ClcSetButtonState(IDC_TBSOUND, db_get_b(NULL, "Skin", "UseSound", 1) ? BST_UNCHECKED : BST_CHECKED);
}

void ClcSetButtonState(int ctrlid, int status)
{
	for (int i = 0; i < SIZEOF(BTNS); i++)
		if (BTNS[i].ctrlid == ctrlid) {
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)BTNS[i].hButton, status ? TTBST_PUSHED : TTBST_RELEASED);
			break;
		}
}

HWND ClcGetButtonWindow(int ctrlid)
{
	for (int i = 0; i < SIZEOF(BTNS); i++)
		if (BTNS[i].ctrlid == ctrlid)
			return BTNS[i].hwndButton;

	return NULL;
}

int ClcGetButtonId(HWND hwnd)
{
	for (int i = 0; i < SIZEOF(BTNS); i++)
		if (BTNS[i].hwndButton == hwnd)
			return BTNS[i].ctrlid;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct MButtonExtension : public MButtonCtrl
{
	HICON hIconPrivate;
	TCHAR szText[128];
	SIZE sLabel;
	HIMAGELIST hIml;
	int iIcon;
	ButtonItem *buttonItem;
	LONG lastGlyphMetrics[4];
	bool bIsTTButton;
};

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLID			100
#define BUTTON_POLLDELAY		50

static int TBStateConvert2Flat(int state)
{
	switch (state) {
	case PBS_NORMAL:
		return TS_NORMAL;
	case PBS_HOT:
		return TS_HOT;
	case PBS_PRESSED:
		return TS_PRESSED;
	case PBS_DISABLED:
		return TS_DISABLED;
	case PBS_DEFAULTED:
		return TS_NORMAL;
	}
	return TS_NORMAL;
}

static void PaintWorker(MButtonExtension *ctl, HDC hdcPaint)
{
	if (hdcPaint) {
		HDC hdcMem;
		HBITMAP hbmMem;
		HBITMAP hbmOld = 0;
		RECT rcClient;
		HFONT hOldFont = 0;
		int xOffset = 0;

		GetClientRect(ctl->hwnd, &rcClient);
		hdcMem = CreateCompatibleDC(hdcPaint);
		hbmMem = CreateCompatibleBitmap(hdcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
		hbmOld = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hbmMem));

		hOldFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, ctl->hFont));
		// If its a push button, check to see if it should stay pressed
		if (ctl->bIsPushBtn && ctl->bIsPushed)
			ctl->stateId = PBS_PRESSED;

		// Draw the flat button
		if (ctl->bIsFlat) {
			if (ctl->hThemeToolbar && ctl->bIsThemed) {
				RECT rc = rcClient;
				int state = IsWindowEnabled(ctl->hwnd) ? (ctl->stateId == PBS_NORMAL && ctl->bIsDefault ? PBS_DEFAULTED : ctl->stateId) : PBS_DISABLED;

				if (ctl->bIsTTButton) {
					POINT pt;
					RECT rcParent;
					GetWindowRect(ctl->hwnd, &rcParent);
					pt.x = rcParent.left;
					pt.y = rcParent.top;

					ScreenToClient(g_hwndToolbarFrame, &pt);
					BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, cfg::dat.hdcToolbar, pt.x, pt.y, SRCCOPY);
				}
				else
					SkinDrawBg(ctl->hwnd, hdcMem);

				if (IsThemeBackgroundPartiallyTransparent(ctl->hThemeToolbar, TP_BUTTON, TBStateConvert2Flat(state))) {
					DrawThemeParentBackground(ctl->hwnd, hdcMem, &rc);
				}
				DrawThemeBackground(ctl->hThemeToolbar, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), &rc, &rc);
			}
			else {
				HBRUSH hbr;
				RECT rc = rcClient;

				if (ctl->buttonItem) {
					RECT rcParent;
					POINT pt;
					HWND hwndParent = pcli->hwndContactList;
					ImageItem *imgItem = ctl->stateId == PBS_HOT ? ctl->buttonItem->imgHover : (ctl->stateId == PBS_PRESSED ? ctl->buttonItem->imgPressed : ctl->buttonItem->imgNormal);
					LONG *glyphMetrics = ctl->stateId == PBS_HOT ? ctl->buttonItem->hoverGlyphMetrics : (ctl->stateId == PBS_PRESSED ? ctl->buttonItem->pressedGlyphMetrics : ctl->buttonItem->normalGlyphMetrics);

					GetWindowRect(ctl->hwnd, &rcParent);
					pt.x = rcParent.left;
					pt.y = rcParent.top;

					ScreenToClient(pcli->hwndContactList, &pt);

					BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, cfg::dat.hdcBg, pt.x, pt.y, SRCCOPY);
					if (imgItem)
						DrawAlpha(hdcMem, &rc, 0, 0, 0, 0, 0, 0, 0, imgItem);
					if (g_glyphItem) {
						GdiAlphaBlend(hdcMem, (rc.right - glyphMetrics[2]) / 2, (rc.bottom - glyphMetrics[3]) / 2,
							glyphMetrics[2], glyphMetrics[3], g_glyphItem->hdc,
							glyphMetrics[0], glyphMetrics[1], glyphMetrics[2],
							glyphMetrics[3], g_glyphItem->bf);
					}
				}
				else if (ctl->bIsSkinned) { // skinned
					RECT rcParent;
					POINT pt;
					HWND hwndParent = pcli->hwndContactList;
					StatusItems_t *item;
					int item_id;

					GetWindowRect(ctl->hwnd, &rcParent);
					pt.x = rcParent.left;
					pt.y = rcParent.top;

					ScreenToClient(pcli->hwndContactList, &pt);

					if (ctl->bIsTTButton)
						item_id = ctl->stateId == PBS_HOT ? ID_EXTBKTBBUTTONMOUSEOVER : (ctl->stateId == PBS_PRESSED ? ID_EXTBKTBBUTTONSPRESSED : ID_EXTBKTBBUTTONSNPRESSED);
					else
						item_id = ctl->stateId == PBS_HOT ? ID_EXTBKBUTTONSMOUSEOVER : (ctl->stateId == PBS_PRESSED ? ID_EXTBKBUTTONSPRESSED : ID_EXTBKBUTTONSNPRESSED);
					item = arStatusItems[item_id - ID_STATUS_OFFLINE];

					if (pt.y < 10 || cfg::dat.bWallpaperMode)
						BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, cfg::dat.hdcBg, pt.x, pt.y, SRCCOPY);
					else
						FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_3DFACE));

					if (ctl->bIsTTButton) {
						GetWindowRect(ctl->hwnd, &rcParent);
						pt.x = rcParent.left;
						pt.y = rcParent.top;

						ScreenToClient(g_hwndToolbarFrame, &pt);
						BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, cfg::dat.hdcToolbar, pt.x, pt.y, SRCCOPY);
					}

					SetTextColor(hdcMem, item->TEXTCOLOR);
					if (!item->IGNORED) {
						rc.top += item->MARGIN_TOP; rc.bottom -= item->MARGIN_BOTTOM;
						rc.left += item->MARGIN_LEFT; rc.right -= item->MARGIN_RIGHT;
						DrawAlpha(hdcMem, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT,
							item->CORNER, item->BORDERSTYLE, item->imageItem);
					}
				}
				else {
					if (ctl->stateId == PBS_PRESSED || ctl->stateId == PBS_HOT)
						hbr = GetSysColorBrush(COLOR_3DFACE);
					else {
						HDC dc;
						HWND hwndParent;

						hwndParent = GetParent(ctl->hwnd);
						dc = GetDC(hwndParent);
						hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwndParent);
						ReleaseDC(hwndParent, dc);
					}
					if (hbr) {
						FillRect(hdcMem, &rc, hbr);
						DeleteObject(hbr);
					}
				}
				if (!ctl->bIsSkinned && ctl->buttonItem == 0) {
					if (ctl->stateId == PBS_HOT || ctl->focus) {
						if (ctl->bIsPushed)
							DrawEdge(hdcMem, &rc, EDGE_ETCHED, BF_RECT | BF_SOFT);
						else
							DrawEdge(hdcMem, &rc, BDR_RAISEDOUTER, BF_RECT | BF_SOFT);
					}
					else if (ctl->stateId == PBS_PRESSED)
						DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT | BF_SOFT);
				}
			}
		}
		else {
			// Draw background/border
			if (ctl->hThemeButton && ctl->bIsThemed) {
				int state = IsWindowEnabled(ctl->hwnd) ? (ctl->stateId == PBS_NORMAL && ctl->bIsDefault ? PBS_DEFAULTED : ctl->stateId) : PBS_DISABLED;
				POINT pt;
				RECT rcParent, rc = rcClient;

				GetWindowRect(ctl->hwnd, &rcParent);
				pt.x = rcParent.left;
				pt.y = rcParent.top;
				ScreenToClient(pcli->hwndContactList, &pt);
				BitBlt(hdcMem, 0, 0, rcClient.right, rcClient.bottom, cfg::dat.hdcBg, pt.x, pt.y, SRCCOPY);

				if (ctl->bIsTTButton) {
					GetWindowRect(ctl->hwnd, &rcParent);
					pt.x = rcParent.left;
					pt.y = rcParent.top;

					ScreenToClient(g_hwndToolbarFrame, &pt);
					BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, cfg::dat.hdcToolbar, pt.x, pt.y, SRCCOPY);
				}

				if (IsThemeBackgroundPartiallyTransparent(ctl->hThemeButton, BP_PUSHBUTTON, state)) {
					DrawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);
				}
				DrawThemeBackground(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, state, &rcClient, &rcClient);
			}
			else {
				UINT uState = DFCS_BUTTONPUSH | ((ctl->stateId == PBS_HOT) ? DFCS_HOT : 0) | ((ctl->stateId == PBS_PRESSED) ? DFCS_PUSHED : 0);
				if (ctl->bIsDefault && ctl->stateId == PBS_NORMAL)
					uState |= DLGC_DEFPUSHBUTTON;
				DrawFrameControl(hdcMem, &rcClient, DFC_BUTTON, uState);
			}

			// Draw focus rectangle if button has focus
			if (ctl->focus) {
				RECT focusRect = rcClient;
				InflateRect(&focusRect, -3, -3);
				DrawFocusRect(hdcMem, &focusRect);
			}
		}

		// If we have an icon or a bitmap, ignore text and only draw the image on the button
		if (ctl->hIcon || ctl->hIconPrivate || ctl->iIcon) {
			int ix = (rcClient.right - rcClient.left) / 2 - (g_cxsmIcon / 2);
			int iy = (rcClient.bottom - rcClient.top) / 2 - (g_cxsmIcon / 2);
			HICON hIconNew = ctl->hIconPrivate != 0 ? ctl->hIconPrivate : ctl->hIcon;
			if (ctl->szText[0] == 0) {
				if (ctl->iIcon)
					ImageList_DrawEx(ctl->hIml, ctl->iIcon, hdcMem, ix, iy, g_cxsmIcon, g_cysmIcon, CLR_NONE, CLR_NONE, ILD_NORMAL);
				else
					DrawState(hdcMem, NULL, NULL, (LPARAM)hIconNew, 0, ix, iy, g_cxsmIcon, g_cysmIcon, IsWindowEnabled(ctl->hwnd) ? DST_ICON | DSS_NORMAL : DST_ICON | DSS_DISABLED);
				ctl->sLabel.cx = ctl->sLabel.cy = 0;
			}
			else {
				GetTextExtentPoint32(hdcMem, ctl->szText, lstrlen(ctl->szText), &ctl->sLabel);

				if (g_cxsmIcon + ctl->sLabel.cx + 8 > rcClient.right - rcClient.left)
					ctl->sLabel.cx = (rcClient.right - rcClient.left) - g_cxsmIcon - 8;
				else
					ctl->sLabel.cx += 4;

				ix = (rcClient.right - rcClient.left) / 2 - ((g_cxsmIcon + ctl->sLabel.cx) / 2);
				if (ctl->iIcon)
					ImageList_DrawEx(ctl->hIml, ctl->iIcon, hdcMem, ix, iy, g_cxsmIcon, g_cysmIcon, CLR_NONE, CLR_NONE, ILD_NORMAL);
				else
					DrawState(hdcMem, NULL, NULL, (LPARAM)hIconNew, 0, ix, iy, g_cxsmIcon, g_cysmIcon, IsWindowEnabled(ctl->hwnd) ? DST_ICON | DSS_NORMAL : DST_ICON | DSS_DISABLED);
				xOffset = ix + g_cxsmIcon + 4;
			}
		}
		else if (ctl->hBitmap) {
			BITMAP bminfo;
			int ix, iy;

			GetObject(ctl->hBitmap, sizeof(bminfo), &bminfo);
			ix = (rcClient.right - rcClient.left) / 2 - (bminfo.bmWidth / 2);
			iy = (rcClient.bottom - rcClient.top) / 2 - (bminfo.bmHeight / 2);
			if (ctl->stateId == PBS_PRESSED) {
				ix++;
				iy++;
			}
			DrawState(hdcMem, NULL, NULL, (LPARAM) ctl->hBitmap, 0, ix, iy, bminfo.bmWidth, bminfo.bmHeight, IsWindowEnabled(ctl->hwnd) ? DST_BITMAP : DST_BITMAP | DSS_DISABLED);
		}
		if (GetWindowTextLength(ctl->hwnd)) {
			// Draw the text and optinally the arrow
			RECT rcText;

			CopyRect(&rcText, &rcClient);
			SetBkMode(hdcMem, TRANSPARENT);
			// XP w/themes doesn't used the glossy disabled text.  Is it always using COLOR_GRAYTEXT?  Seems so.
			if (!ctl->bIsSkinned)
				SetTextColor(hdcMem, IsWindowEnabled(ctl->hwnd) || !ctl->hThemeButton ? GetSysColor(COLOR_BTNTEXT) : GetSysColor(COLOR_GRAYTEXT));
			if (ctl->arrow)
				DrawState(hdcMem, NULL, NULL, (LPARAM)ctl->arrow, 0, rcClient.right - rcClient.left - 5 - g_cxsmIcon + (!ctl->hThemeButton && ctl->stateId == PBS_PRESSED ? 1 : 0), (rcClient.bottom - rcClient.top) / 2 - g_cysmIcon / 2 + (!ctl->hThemeButton && ctl->stateId == PBS_PRESSED ? 1 : 0), g_cxsmIcon, g_cysmIcon, IsWindowEnabled(ctl->hwnd) ? DST_ICON : DST_ICON | DSS_DISABLED);
			DrawState(hdcMem, NULL, NULL, (LPARAM)ctl->szText, 0, xOffset + (!ctl->hThemeButton && ctl->stateId == PBS_PRESSED ? 1 : 0), ctl->hThemeButton ? (rcText.bottom - rcText.top - ctl->sLabel.cy) / 2 + 1 : (rcText.bottom - rcText.top - ctl->sLabel.cy) / 2 + (ctl->stateId == PBS_PRESSED ? 1 : 0), ctl->sLabel.cx, ctl->sLabel.cy, IsWindowEnabled(ctl->hwnd) || ctl->hThemeButton ? DST_PREFIXTEXT | DSS_NORMAL : DST_PREFIXTEXT | DSS_DISABLED);
		}
		if (hOldFont)
			SelectObject(hdcMem, hOldFont);
		BitBlt(hdcPaint, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hbmOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
		DeleteObject(hbmOld);
	}
}

static LRESULT CALLBACK TSButtonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MButtonExtension *bct = (MButtonExtension*)GetWindowLongPtr(hwnd, 0);

	switch (msg) {
	case WM_DESTROY:
		if (bct && bct->hIconPrivate)
			DestroyIcon(bct->hIconPrivate);
		break;

	case WM_SETTEXT:
		_tcsncpy_s(bct->szText, (TCHAR*)lParam, _TRUNCATE);
		break;

	case BM_GETIMAGE:
		if (wParam == IMAGE_ICON)
			return (LRESULT)(bct->hIconPrivate ? bct->hIconPrivate : bct->hIcon);
		break;

	case BM_SETIMAGE:
		if (!lParam)
			break;

		bct->hIml = 0;
		bct->iIcon = 0;
		if (wParam == IMAGE_ICON) {
			ICONINFO ii = {0};
			BITMAP bm = {0};

			if (bct->hIconPrivate) {
				DestroyIcon(bct->hIconPrivate);
				bct->hIconPrivate = 0;
			}

			GetIconInfo((HICON) lParam, &ii);
			GetObject(ii.hbmColor, sizeof(bm), &bm);
			if (bm.bmWidth > g_cxsmIcon || bm.bmHeight > g_cysmIcon) {
				HIMAGELIST hImageList;
				hImageList = ImageList_Create(g_cxsmIcon, g_cysmIcon, ILC_COLOR32 | ILC_MASK, 1, 0);
				ImageList_AddIcon(hImageList, (HICON) lParam);
				bct->hIconPrivate = ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
				ImageList_RemoveAll(hImageList);
				ImageList_Destroy(hImageList);
				bct->hIcon = 0;
			}
			else {
				bct->hIcon = (HICON) lParam;
				bct->hIconPrivate = 0;
			}

			DeleteObject(ii.hbmMask);
			DeleteObject(ii.hbmColor);
			bct->hBitmap = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		else if (wParam == IMAGE_BITMAP) {
			bct->hBitmap = (HBITMAP) lParam;
			if (bct->hIconPrivate)
				DestroyIcon(bct->hIconPrivate);
			bct->hIcon = bct->hIconPrivate = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		return 1;

	case BUTTONSETIMLICON:
		if (bct->hIconPrivate)
			DestroyIcon(bct->hIconPrivate);
		bct->hIml = (HIMAGELIST)wParam;
		bct->iIcon = (int)lParam;
		bct->hIcon = bct->hIconPrivate = 0;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case BUTTONSETSKINNED:
		bct->bIsSkinned = wParam != 0;
		bct->bIsThemed = bct->bIsSkinned ? FALSE : bct->bIsThemed;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case BUTTONSETBTNITEM:
		bct->buttonItem = (ButtonItem *)lParam;
		break;

	case BUTTONSETTTBUTTON:
		bct->bIsTTButton = wParam != 0;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_NCHITTEST:
		switch (SendMessage(pcli->hwndContactList, WM_NCHITTEST, wParam, lParam)) {
		case HTLEFT:
		case HTRIGHT:
		case HTBOTTOM:
		case HTTOP:
		case HTTOPLEFT:
		case HTTOPRIGHT:
		case HTBOTTOMLEFT:
		case HTBOTTOMRIGHT:
			return HTTRANSPARENT;
		}
	}
	return mir_callNextSubclass(hwnd, TSButtonWndProc, msg, wParam, lParam);
}

static LRESULT CALLBACK ToolbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		return TRUE;

	case WM_NCPAINT:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			RECT rc, rcClient;
			GetClientRect(hwnd, &rc);
			rcClient = rc;

			if (cfg::dat.hdcToolbar) {
				SelectObject(cfg::dat.hdcToolbar, cfg::dat.hbmToolbarOld);
				DeleteObject(cfg::dat.hbmToolbar);
				DeleteDC(cfg::dat.hdcToolbar);
				cfg::dat.hdcToolbar = NULL;
			}
			cfg::dat.hdcToolbar = CreateCompatibleDC(hdc);
			cfg::dat.hbmToolbar = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
			cfg::dat.hbmToolbarOld = reinterpret_cast<HBITMAP>(SelectObject(cfg::dat.hdcToolbar, cfg::dat.hbmToolbar));
			HDC hdcMem = cfg::dat.hdcToolbar;
			SetBkMode(hdcMem, TRANSPARENT);

			if (cfg::dat.bWallpaperMode)
				SkinDrawBg(hwnd, hdcMem);

			StatusItems_t *item = arStatusItems[ID_EXTBKBUTTONBAR - ID_STATUS_OFFLINE];
			if (item->IGNORED)
				FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_3DFACE));
			else {
				rc.top += item->MARGIN_TOP;
				rc.bottom -= item->MARGIN_BOTTOM;
				rc.left += item->MARGIN_LEFT;
				rc.right -= item->MARGIN_RIGHT;

				DrawAlpha(hdcMem, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT,
					item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
			}

			BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);
			ps.fErase = FALSE;
			EndPaint(hwnd, &ps);
			return 0;
		}

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			int iCtrlId = ClcGetButtonId((HWND)lParam);
			// standard buttons are processed in the main window
			if (iCtrlId) {
				SendMessage(pcli->hwndContactList, msg, MAKELONG(iCtrlId, BN_CLICKED), lParam);
				return 0;
			}
		}

	case WM_DESTROY:
		if (cfg::dat.hdcToolbar && lParam == NULL) { // lParam is NULL when toolbar frame is being deleted
			SelectObject(cfg::dat.hdcToolbar, cfg::dat.hbmToolbarOld);
			DeleteObject(cfg::dat.hbmToolbar);
			DeleteDC(cfg::dat.hdcToolbar);
			cfg::dat.hdcToolbar = NULL;
		}
		break;
	}

	return mir_callNextSubclass(hwnd, ToolbarWndProc, msg, wParam, lParam);
}

static void CustomizeToolbar(HANDLE hButton, HWND hWnd, LPARAM)
{
	if (hButton == TTB_WINDOW_HANDLE) {
		mir_subclassWindow(hWnd, ToolbarWndProc);

		g_hwndToolbarFrame = hWnd;
		TTBCtrl* pTBInfo = (TTBCtrl*)GetWindowLongPtr(hWnd, 0);
		pTBInfo->bHardUpdate = FALSE;

		InitDefaultButtons();
		SetButtonToSkinned();
		return;
	}
	
	SendMessage(hWnd, BUTTONSETCUSTOMPAINT, sizeof(MButtonExtension), (LPARAM)PaintWorker);
	mir_subclassWindow(hWnd, TSButtonWndProc);

	MButtonExtension *bct = (MButtonExtension*)GetWindowLongPtr(hWnd, 0);
	if (g_index != -1) { // adding built-in button
		BTNS[g_index].hwndButton = hWnd;
		if (BTNS[g_index].isAction)
			bct->bSendOnDown = true;
		if (!BTNS[g_index].isPush)
			bct->bIsPushBtn = true;
	}
	else {
		bool bSkinned = cfg::dat.bSkinnedButtonMode != 0;
		CustomizeButton(hWnd, bSkinned, !bSkinned, bSkinned, true);
	}
}

void CustomizeButton(HWND hWnd, bool bIsSkinned, bool bIsThemed, bool bIsFlat, bool bIsTTButton)
{
	SendMessage(hWnd, BUTTONSETCUSTOMPAINT, sizeof(MButtonExtension), (LPARAM)PaintWorker);
	mir_subclassWindow(hWnd, TSButtonWndProc);

	SendMessage(hWnd, BUTTONSETSKINNED, bIsSkinned, 0);
	SendMessage(hWnd, BUTTONSETASTHEMEDBTN, bIsThemed, 0);
	SendMessage(hWnd, BUTTONSETASFLATBTN, bIsFlat, 0);
	SendMessage(hWnd, BUTTONSETTTBUTTON, bIsTTButton, 0);
}

static int Nicer_CustomizeToolbar(WPARAM, LPARAM)
{	
	TopToolbar_SetCustomProc(CustomizeToolbar, 0);
	return 0;
}

static int Nicer_ReloadToolbar(WPARAM wParam, LPARAM)
{	
	PLUGININFOEX *pInfo = (PLUGININFOEX*)wParam;
	if (!_stricmp(pInfo->shortName, "TopToolBar"))
		TopToolbar_SetCustomProc(CustomizeToolbar, 0);
	return 0;
}

void LoadButtonModule()
{
	HookEvent(ME_SYSTEM_MODULELOAD, Nicer_ReloadToolbar);
	HookEvent(ME_SYSTEM_MODULESLOADED, Nicer_CustomizeToolbar);
}
