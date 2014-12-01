/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: TSButton.cpp 11848 2010-05-27 14:57:22Z silvercircle $
 *
 * A skinnable button class for tabSRMM.
 *
 */

#include "commonheaders.h"

#define PBS_PUSHDOWNPRESSED 6

static LRESULT CALLBACK TSButtonWndProc(HWND hwnd, UINT  msg, WPARAM wParam, LPARAM lParam);

// External theme methods and properties
static BLENDFUNCTION bf_buttonglyph;
static HDC hdc_buttonglyph = 0;
static HBITMAP hbm_buttonglyph, hbm_buttonglyph_old;

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLID       100
#define BUTTON_POLLDELAY    50

int TSAPI TBStateConvert2Flat(int state)
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

int TSAPI RBStateConvert2Flat(int state)
{
	switch (state) {
	case PBS_NORMAL:
		return 1;
	case PBS_HOT:
		return 2;
	case PBS_PRESSED:
		return 3;
	case PBS_DISABLED:
		return 1;
	case PBS_DEFAULTED:
		return 1;
	}
	return 1;
}

/**
 * convert button state (hot, pressed, normal) to REBAR part state ids
 *
 * @param state  int: button state
 *
 * @return int: state item id
 */

static void PaintWorker(TSButtonCtrl *ctl, HDC hdcPaint)
{
	if (hdc_buttonglyph == 0) {
		hdc_buttonglyph = CreateCompatibleDC(hdcPaint);
		hbm_buttonglyph = CreateCompatibleBitmap(hdcPaint, 16, 16);
		hbm_buttonglyph_old = (HBITMAP)SelectObject(hdc_buttonglyph, hbm_buttonglyph);
		bf_buttonglyph.BlendFlags = 0;
		bf_buttonglyph.SourceConstantAlpha = 120;
		bf_buttonglyph.BlendOp = AC_SRC_OVER;
		bf_buttonglyph.AlphaFormat = 0;
	}
	
	if (hdcPaint == NULL)
		return;

	HDC      hdcMem;
	HBITMAP  hbmMem, hOld;
	HANDLE   hbp = 0;
	bool     bAero = M.isAero();

	TWindowData *dat = (TWindowData*)GetWindowLongPtr(GetParent(ctl->hwnd), GWLP_USERDATA);

	RECT rcClient, rcContent;
	GetClientRect(ctl->hwnd, const_cast<RECT *>(&rcClient));
	CopyRect(&rcContent, &rcClient);

	if (CMimAPI::m_haveBufferedPaint)
		hbp = CMimAPI::m_pfnBeginBufferedPaint(hdcPaint, &rcContent, BPBF_TOPDOWNDIB, NULL, &hdcMem);
	else {
		hdcMem = CreateCompatibleDC(hdcPaint);
		hbmMem = CreateCompatibleBitmap(hdcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
		hOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
	}

	CSkin::FillBack(hdcMem, &rcContent);

	if (ctl->bIsPushBtn && ctl->bIsPushed)
		ctl->stateId = PBS_PRESSED;

	if (ctl->bIsFlat) {
		if (ctl->pContainer && CSkin::m_skinEnabled) {
			CSkinItem *item, *realItem = 0;
			if (ctl->bTitleButton)
				item = &SkinItems[ctl->stateId == PBS_NORMAL ? ID_EXTBKTITLEBUTTON : (ctl->stateId == PBS_HOT ? ID_EXTBKTITLEBUTTONMOUSEOVER : ID_EXTBKTITLEBUTTONPRESSED)];
			else {
				item = &SkinItems[(ctl->stateId == PBS_NORMAL || ctl->stateId == PBS_DISABLED) ? ID_EXTBKBUTTONSNPRESSED : (ctl->stateId == PBS_HOT ? ID_EXTBKBUTTONSMOUSEOVER : ID_EXTBKBUTTONSPRESSED)];
				realItem = item;
			}
			CSkin::SkinDrawBG(ctl->hwnd, ctl->pContainer->hwnd,  ctl->pContainer, &rcContent, hdcMem);
			if (!item->IGNORED) {
				RECT rc1 = rcClient;
				rc1.left += item->MARGIN_LEFT;
				rc1.right -= item->MARGIN_RIGHT;
				rc1.top += item->MARGIN_TOP;
				rc1.bottom -= item->MARGIN_BOTTOM;
				CSkin::DrawItem(hdcMem, &rc1, item);
			}
			else goto flat_themed;
		}
		else {
flat_themed:
			int state = IsWindowEnabled(ctl->hwnd) ? (ctl->stateId == PBS_NORMAL && ctl->bIsDefault ? PBS_DEFAULTED : ctl->stateId) : PBS_DISABLED;

			if (ctl->bToolbarButton) {
				if (dat) {
					RECT	rcWin;
					GetWindowRect(ctl->hwnd, &rcWin);
					POINT 	pt;
					pt.x = rcWin.left;
					ScreenToClient(dat->hwnd, &pt);
					BitBlt(hdcMem, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
							dat->pContainer->cachedToolbarDC, pt.x, 1, SRCCOPY);
				}
			}
			if (ctl->hThemeToolbar && ctl->bIsThemed && 1 == dat->pContainer->bTBRenderingMode) {
				if (bAero || PluginConfig.m_WinVerMajor >= 6)
					DrawThemeBackground(ctl->hThemeToolbar, hdcMem, 8, RBStateConvert2Flat(state), &rcClient, &rcClient);
				else
					DrawThemeBackground(ctl->hThemeToolbar, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), &rcClient, &rcClient);
			}
			else {
				CSkin::m_switchBarItem->setAlphaFormat(AC_SRC_ALPHA, state == PBS_HOT ? 220 : 180);
				if (state == PBS_HOT || state == PBS_PRESSED) {
					if (state == PBS_PRESSED) {
						RECT	rc = rcClient;
						InflateRect(&rc, -1, -1);
						HBRUSH bBack = CreateSolidBrush(PluginConfig.m_tbBackgroundLow ? PluginConfig.m_tbBackgroundLow : GetSysColor(COLOR_3DDKSHADOW));
						FillRect(hdcMem, &rc, bBack);
						DeleteObject(bBack);
					}
					CSkin::m_switchBarItem->Render(hdcMem, &rcClient, true);
				}
			}
		}
	}
	else {
		if (ctl->pContainer && CSkin::m_skinEnabled) {
			CSkinItem *item, *realItem = 0;
			if (ctl->bTitleButton)
				item = &SkinItems[ctl->stateId == PBS_NORMAL ? ID_EXTBKTITLEBUTTON : (ctl->stateId == PBS_HOT ? ID_EXTBKTITLEBUTTONMOUSEOVER : ID_EXTBKTITLEBUTTONPRESSED)];
			else {
				item = &SkinItems[(ctl->stateId == PBS_NORMAL || ctl->stateId == PBS_DISABLED) ? ID_EXTBKBUTTONSNPRESSED : (ctl->stateId == PBS_HOT ? ID_EXTBKBUTTONSMOUSEOVER : ID_EXTBKBUTTONSPRESSED)];
				realItem = item;
			}
			CSkin::SkinDrawBG(ctl->hwnd, ctl->pContainer->hwnd,  ctl->pContainer, &rcClient, hdcMem);
			if (!item->IGNORED) {
				RECT rc1 = rcClient;
				rc1.left += item->MARGIN_LEFT;
				rc1.right -= item->MARGIN_RIGHT;
				rc1.top += item->MARGIN_TOP;
				rc1.bottom -= item->MARGIN_BOTTOM;
				CSkin::DrawItem(hdcMem, &rc1, item);
			} else
				goto nonflat_themed;
		}
		else {
nonflat_themed:
			int state = IsWindowEnabled(ctl->hwnd) ? (ctl->stateId == PBS_NORMAL && ctl->bIsDefault ? PBS_DEFAULTED : ctl->stateId) : PBS_DISABLED;

			if (ctl->hThemeButton && ctl->bIsThemed && 0 == PluginConfig.m_fillColor) {
				DrawThemeBackground(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, state, &rcClient, &rcClient);
				GetThemeBackgroundContentRect(ctl->hThemeToolbar, hdcMem, BP_PUSHBUTTON, PBS_NORMAL, &rcClient, &rcContent);
			}
			else {
				CSkin::m_switchBarItem->setAlphaFormat(AC_SRC_ALPHA, state == PBS_NORMAL ? 140 : 240);
				if (state == PBS_PRESSED) {
					RECT	rc = rcClient;
					InflateRect(&rc, -1, -1);
					HBRUSH bBack = CreateSolidBrush(PluginConfig.m_tbBackgroundLow ? PluginConfig.m_tbBackgroundLow : GetSysColor(COLOR_3DDKSHADOW));
					FillRect(hdcMem, &rc, bBack);
					DeleteObject(bBack);
				}
				CSkin::m_switchBarItem->Render(hdcMem, &rcClient, true);
			}

			// Draw focus rectangle if button has focus
			if (ctl->focus) {
				RECT focusRect = rcClient;
				InflateRect(&focusRect, -3, -3);
				DrawFocusRect(hdcMem, &focusRect);
			}
		}
	}

	/*
	 * render content
	 */
	if (ctl->arrow) {
		rcContent.top += 2;
		rcContent.bottom -= 2;
		rcContent.left = rcClient.right - 12;
		rcContent.right = rcContent.left;

		DrawIconEx(hdcMem, rcClient.right - 15, (rcClient.bottom - rcClient.top) / 2 - (PluginConfig.m_smcyicon / 2),
					PluginConfig.g_buttonBarIcons[ICON_DEFAULT_PULLDOWN], 16, 16, 0, 0, DI_NORMAL);
	}

	if (ctl->hIcon || ctl->hIconPrivate) {
		int ix = (rcClient.right - rcClient.left) / 2 - 8;
		int iy = (rcClient.bottom - rcClient.top) / 2 - 8;
		HICON hIconNew = ctl->hIconPrivate != 0 ? ctl->hIconPrivate : ctl->hIcon;

		if (ctl->stateId == PBS_PRESSED) {
			ix++;
			iy++;
		}

		if (ctl->arrow)
			ix -= 4;

		if (ctl->bDimmed && PluginConfig.m_IdleDetect)
			CSkin::DrawDimmedIcon(hdcMem, ix, iy, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, hIconNew, 180);
		else {
			if (ctl->stateId != PBS_DISABLED) {
				DrawIconEx(hdcMem, ix, iy, hIconNew, 16, 16, 0, 0, DI_NORMAL);
				if (ctl->overlay)
					DrawIconEx(hdcMem, ix, iy, ctl->overlay, 16, 16, 0, 0, DI_NORMAL);
			}
			else {
				BitBlt(hdc_buttonglyph, 0, 0, 16, 16, hdcMem, ix, iy, SRCCOPY);
				DrawIconEx(hdc_buttonglyph, 0, 0, hIconNew, 16, 16, 0, 0, DI_NORMAL);
				if (ctl->overlay)
					DrawIconEx(hdc_buttonglyph, 0, 0, ctl->overlay, 16, 16, 0, 0, DI_NORMAL);
 				GdiAlphaBlend(hdcMem, ix, iy, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, hdc_buttonglyph, 0, 0, 16, 16, bf_buttonglyph);
			}
		}
	}
	else if (GetWindowTextLength(ctl->hwnd)) {
		// Draw the text and optinally the arrow
		RECT rcText;
		CopyRect(&rcText, &rcClient);

		TCHAR szText[MAX_PATH];
		GetWindowText(ctl->hwnd, szText, SIZEOF(szText));
		SetBkMode(hdcMem, TRANSPARENT);
		HFONT hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);
		if (ctl->pContainer && CSkin::m_skinEnabled)
			SetTextColor(hdcMem, IsWindowEnabled(ctl->hwnd) ? CSkin::m_DefaultFontColor : GetSysColor(COLOR_GRAYTEXT));
		else {
			if (PluginConfig.m_genericTxtColor)
				SetTextColor(hdcMem, PluginConfig.m_genericTxtColor);
			else
				SetTextColor(hdcMem, IsWindowEnabled(ctl->hwnd) || !ctl->hThemeButton ? GetSysColor(COLOR_BTNTEXT) : GetSysColor(COLOR_GRAYTEXT));
		}

		SIZE sz;
		GetTextExtentPoint32(hdcMem, szText, (int)mir_tstrlen(szText), &sz);
		if (ctl->cHot) {
			SIZE szHot;
			GetTextExtentPoint32A(hdcMem, "&", 1, &szHot);
			sz.cx -= szHot.cx;
		}
		if (ctl->arrow)
			DrawState(hdcMem, NULL, NULL, (LPARAM)ctl->arrow, 0, rcClient.right - rcClient.left - 5 - PluginConfig.m_smcxicon + (!ctl->hThemeButton && ctl->stateId == PBS_PRESSED ? 1 : 0), (rcClient.bottom - rcClient.top) / 2 - PluginConfig.m_smcyicon / 2 + (!ctl->hThemeButton && ctl->stateId == PBS_PRESSED ? 1 : 0), PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, IsWindowEnabled(ctl->hwnd) ? DST_ICON : DST_ICON | DSS_DISABLED);
		SelectObject(hdcMem, ctl->hFont);
		DrawState(hdcMem, NULL, NULL, (LPARAM)szText, mir_tstrlen(szText), (rcText.right - rcText.left - sz.cx) / 2 + (!ctl->hThemeButton && ctl->stateId == PBS_PRESSED ? 1 : 0), ctl->hThemeButton ? (rcText.bottom - rcText.top - sz.cy) / 2 : (rcText.bottom - rcText.top - sz.cy) / 2 - (ctl->stateId == PBS_PRESSED ? 0 : 1), sz.cx, sz.cy, IsWindowEnabled(ctl->hwnd) || ctl->hThemeButton ? DST_PREFIXTEXT | DSS_NORMAL : DST_PREFIXTEXT | DSS_DISABLED);
		SelectObject(hdcMem, hOldFont);
	}

	if (hbp)
		CMimAPI::m_pfnEndBufferedPaint(hbp, TRUE);
	else {
		BitBlt(hdcPaint, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
	}
}

static LRESULT CALLBACK TSButtonWndProc(HWND hwndDlg, UINT msg,  WPARAM wParam, LPARAM lParam)
{
	TSButtonCtrl *bct = (TSButtonCtrl*)GetWindowLongPtr(hwndDlg, 0);
	switch (msg) {
	case WM_DESTROY:
		if (bct)
			if (bct->hIconPrivate)
				DestroyIcon(bct->hIconPrivate);
		break;

	case WM_NCPAINT:
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdcPaint = BeginPaint(hwndDlg, &ps);
			if (hdcPaint) {
				if (bct->sitem)
					bct->sitem->RenderThis(hdcPaint);
				else
					PaintWorker(bct, hdcPaint);
				EndPaint(hwndDlg, &ps);
			}
		}
		return 0;

	case BM_SETIMAGE:
		if (wParam == IMAGE_ICON) {
			if (bct->hIconPrivate)
				DestroyIcon(bct->hIconPrivate);

			ICONINFO ii;
			GetIconInfo((HICON)lParam, &ii);

			BITMAP bm;
			GetObject(ii.hbmColor, sizeof(bm), &bm);
			if (bm.bmWidth != PluginConfig.m_smcxicon || bm.bmHeight != PluginConfig.m_smcyicon) {
				HIMAGELIST hImageList = ImageList_Create(PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, ILC_COLOR32 | ILC_MASK, 1, 0);
				ImageList_AddIcon(hImageList, (HICON)lParam);
				bct->hIconPrivate = ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
				ImageList_RemoveAll(hImageList);
				ImageList_Destroy(hImageList);
				bct->hIcon = 0;
			}
			else {
				bct->hIcon = (HICON)lParam;
				bct->hIconPrivate = 0;
			}

			DeleteObject(ii.hbmMask);
			DeleteObject(ii.hbmColor);
			bct->hBitmap = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		else if (wParam == IMAGE_BITMAP) {
			bct->hBitmap = (HBITMAP)lParam;
			if (bct->hIconPrivate)
				DestroyIcon(bct->hIconPrivate);
			bct->hIcon = bct->hIconPrivate = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		return 0;

	case BUTTONSETARROW: // turn arrow on/off
		bct->arrow = (HICON)wParam;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		return 0;

	case BUTTONSETASDIMMED:
		bct->bDimmed = (wParam != 0);
		break;

	case BUTTONSETCONTAINER:
		bct->pContainer = (TContainerData*)wParam;
		break;

	case BUTTONSETASTITLE:
		bct->bTitleButton = TRUE;
		break;

	case BUTTONSETASNORMAL:
		bct->stateId = (wParam) ? PBS_NORMAL : PBS_DISABLED;
		InvalidateRect(bct->hwnd, NULL, FALSE);
		break;

	case BUTTONGETSTATEID:
		return bct->stateId;

	case BUTTONSETASTOOLBARBUTTON:
		bct->bToolbarButton = (wParam != 0);
		break;

	case BUTTONSETASSIDEBARBUTTON:
		bct->sitem = reinterpret_cast<CSideBarButton *>(wParam);
		break;

	case BUTTONSETOVERLAYICON:
		bct->overlay = (HICON)wParam;
		break;

	case WM_CONTEXTMENU:
		if (bct->sitem)
			bct->sitem->invokeContextMenu();
		break;

	case WM_MBUTTONUP:
		if (bct->sitem)
			if (bct->sitem->getDat())
				SendMessage(bct->sitem->getDat()->hwnd, WM_CLOSE, 1, 0);
		break;

	case WM_LBUTTONDOWN:
		if (bct->sitem) {
			if (bct->sitem->testCloseButton() != -1)
				return TRUE;
			bct->stateId = PBS_PRESSED;
			InvalidateRect(bct->hwnd, NULL, TRUE);
			bct->sitem->activateSession();
		}

		if (bct->arrow) {
			RECT rc;
			GetClientRect(bct->hwnd, &rc);
			if (LOWORD(lParam) < rc.right - 12 && bct->stateId != PBS_DISABLED)
				bct->stateId = PBS_PRESSED;
			else if (LOWORD(lParam) > rc.right - 12) {
				if (GetDlgCtrlID(hwndDlg) == IDOK || bct->stateId != PBS_DISABLED) {
					WORD w = (WORD)((int)bct->arrow & 0x0000ffff);
					SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(w, BN_CLICKED), (LPARAM)hwndDlg);
				}
			}
			InvalidateRect(bct->hwnd, NULL, TRUE);
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		if (bct->sitem) {
			if (bct->sitem->testCloseButton() != -1) {
				SendMessage(bct->sitem->getDat()->hwnd, WM_CLOSE, 1, 0);
				return TRUE;
			}
		}
		break;

	case WM_MOUSEMOVE:
		if (bct->arrow && bct->stateId == PBS_HOT)
			InvalidateRect(bct->hwnd, NULL, TRUE);

		if (bct->sitem) {
			if (bct->sitem->testCloseButton() != -1) {
				if (bct->sitem->m_sideBar->getHoveredClose() != bct->sitem) {
					bct->sitem->m_sideBar->setHoveredClose(bct->sitem);
					InvalidateRect(hwndDlg, 0, FALSE);
				}
			}
			else {
				bct->sitem->m_sideBar->setHoveredClose(0);
				InvalidateRect(hwndDlg, 0, FALSE);
			}
		}
		break;

	case WM_TIMER: // use a timer to check if they have did a mouseout
		if (wParam == BUTTON_POLLID) {
			RECT rc;
			POINT pt;
			GetWindowRect(hwndDlg, &rc);
			GetCursorPos(&pt);
			if (!PtInRect(&rc, pt)) { // mouse must be gone, trigger mouse leave
				PostMessage(hwndDlg, WM_MOUSELEAVE, 0, 0L);
				KillTimer(hwndDlg, BUTTON_POLLID);
				if (bct->sitem) {
					bct->sitem->m_sideBar->setHoveredClose(0);
					InvalidateRect(hwndDlg, 0, FALSE);
				}
			}
		}
		return 0;
	}

	return mir_callNextSubclass(hwndDlg, TSButtonWndProc, msg, wParam, lParam);
}

int TSAPI UnloadTSButtonModule()
{
	if (hdc_buttonglyph) {
		SelectObject(hdc_buttonglyph, hbm_buttonglyph_old);
		DeleteObject(hbm_buttonglyph);
		DeleteDC(hdc_buttonglyph);
	}
	return 0;
}

void CustomizeButton(HWND hwndButton)
{
	SendMessage(hwndButton, BUTTONSETCUSTOMPAINT, sizeof(TSButtonCtrl), 0);

	mir_subclassWindow(hwndButton, TSButtonWndProc);

	TSButtonCtrl *bct = (TSButtonCtrl*)GetWindowLongPtr(hwndButton, 0);
	if (bct)
		bct->hThemeToolbar = (M.isAero() || IsWinVerVistaPlus()) ? OpenThemeData(bct->hwnd, L"MENU") : OpenThemeData(bct->hwnd, L"TOOLBAR");

}
