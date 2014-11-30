/*
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// implements the "Container" window which acts as a toplevel window
// for message sessions.
 */

#include "commonheaders.h"

#define CONTAINER_KEY "TAB_ContainersW"
#define CONTAINER_SUBKEY "containerW"
#define CONTAINER_PREFIX "CNTW_"

TContainerData *pFirstContainer = 0;        // the linked list of struct ContainerWindowData
TContainerData *pLastActiveContainer = NULL;

static  bool	fForceOverlayIcons = false;

static int ServiceParamsOK(ButtonItem *item, WPARAM *wParam, LPARAM *lParam, MCONTACT hContact)
{
	if (item->dwFlags & BUTTON_PASSHCONTACTW || item->dwFlags & BUTTON_PASSHCONTACTL || item->dwFlags & BUTTON_ISCONTACTDBACTION) {
		if (hContact == 0)
			return 0;
		if (item->dwFlags & BUTTON_PASSHCONTACTW)
			*wParam = hContact;
		else if (item->dwFlags & BUTTON_PASSHCONTACTL)
			*lParam = hContact;
		return 1;
	}
	return 1;                                       // doesn't need a paramter
}

// Windows Vista+
// extend the glassy area to get aero look for the status bar, tab bar, info panel
// and outer margins.
 
void TSAPI SetAeroMargins(TContainerData *pContainer)
{
	if ( !M.isAero() || !pContainer || CSkin::m_skinEnabled) {
		pContainer->MenuBar->setAero(false);
		return;
	}

	TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
	if (!dat)
		return;

	RECT	rcWnd;
	if (dat->bType == SESSIONTYPE_IM) {
		if (dat->Panel->isActive())
			GetWindowRect(GetDlgItem(dat->hwnd, IDC_LOG), &rcWnd);
		else
			GetWindowRect(dat->hwnd, &rcWnd);
	}
	else {
		if (dat->Panel->isActive())
			GetWindowRect(GetDlgItem(dat->hwnd, IDC_CHAT_LOG), &rcWnd);
		else
			GetWindowRect(dat->hwnd, &rcWnd);
	}

	POINT	pt = { rcWnd.left, rcWnd.top };
	ScreenToClient(pContainer->hwnd, &pt);

	MARGINS m;
	m.cyTopHeight = pt.y;
	pContainer->MenuBar->setAero(true);

	// bottom part
	GetWindowRect(dat->hwnd, &rcWnd);
	pt.x = rcWnd.left;

	LONG sbar_left, sbar_right;
	if (!pContainer->SideBar->isActive()) {
		pt.y = rcWnd.bottom + ((pContainer->iChilds > 1 || !(pContainer->dwFlags & CNT_HIDETABS)) ? pContainer->tBorder : 0);
		sbar_left = 0, sbar_right = 0;
	}
	else {
		pt.y = rcWnd.bottom;
		sbar_left = (pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT ? pContainer->SideBar->getWidth() : 0);
		sbar_right = (pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_RIGHT ? pContainer->SideBar->getWidth() : 0);
	}
	ScreenToClient(pContainer->hwnd, &pt);
	GetClientRect(pContainer->hwnd, &rcWnd);
	m.cyBottomHeight = (rcWnd.bottom - pt.y);

	if (m.cyBottomHeight < 0 || m.cyBottomHeight >= rcWnd.bottom)
		m.cyBottomHeight = 0;

	m.cxLeftWidth = pContainer->tBorder_outer_left;
	m.cxRightWidth = pContainer->tBorder_outer_right;
	m.cxLeftWidth += sbar_left;
	m.cxRightWidth += sbar_right;

	if (memcmp(&m, &pContainer->mOld, sizeof(MARGINS)) != 0) {
		pContainer->mOld = m;
		CMimAPI::m_pfnDwmExtendFrameIntoClientArea(pContainer->hwnd, &m);
	}
}

// CreateContainer MUST allocate a ContainerWindowData and pass its address
// to CreateDialogParam() via the LPARAM. It also adds the struct to the linked list
// of containers.
//
// The WM_DESTROY handler of the container DlgProc is responsible for mir_free()'ing the
// pointer and for removing the struct from the linked list.
 
TContainerData* TSAPI CreateContainer(const TCHAR *name, int iTemp, MCONTACT hContactFrom)
{
	if (CMimAPI::m_shutDown)
		return NULL;

	TContainerData *pContainer = (TContainerData*)mir_calloc(sizeof(TContainerData));
	_tcsncpy(pContainer->szName, name, CONTAINER_NAMELEN + 1);
	AppendToContainerList(pContainer);

	if (M.GetByte("limittabs", 0) && !_tcscmp(name, _T("default")))
		iTemp |= CNT_CREATEFLAG_CLONED;
	
	// save container name to the db
	if (!M.GetByte("singlewinmode", 0)) {
		int iFirstFree = -1, iFound = FALSE, i = 0;
		do {
			char szCounter[10];
			itoa(i, szCounter, 10);
			ptrT tszName(db_get_tsa(NULL, CONTAINER_KEY, szCounter));
			if (tszName == NULL) {
				if (iFirstFree != -1) {
					pContainer->iContainerIndex = iFirstFree;
					itoa(iFirstFree, szCounter, 10);
				}
				else pContainer->iContainerIndex = i;

				db_set_ts(NULL, CONTAINER_KEY, szCounter, name);
				BuildContainerMenu();
				break;
			}

			if (!_tcsncmp(tszName, name, CONTAINER_NAMELEN)) {
				pContainer->iContainerIndex = i;
				iFound = TRUE;
			}
			else if (!_tcsncmp(tszName, _T("**mir_free**"), CONTAINER_NAMELEN))
				iFirstFree = i;
		}
			while (++i && iFound == FALSE);
	}
	else {
		iTemp |= CNT_CREATEFLAG_CLONED;
		pContainer->iContainerIndex = 1;
	}

	if (iTemp & CNT_CREATEFLAG_MINIMIZED)
		pContainer->dwFlags = CNT_CREATE_MINIMIZED;
	if (iTemp & CNT_CREATEFLAG_CLONED) {
		pContainer->dwFlags |= CNT_CREATE_CLONED;
		pContainer->hContactFrom = hContactFrom;
	}
	pContainer->hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGCONTAINER), NULL, DlgProcContainer, (LPARAM)pContainer);
	return pContainer;
}

static LRESULT CALLBACK ContainerWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TContainerData *pContainer = (TContainerData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	BOOL bSkinned = CSkin::m_skinEnabled ? TRUE : FALSE;

	switch (msg) {
	case WM_NCPAINT: {
		PAINTSTRUCT ps;
		HDC hdcReal;
		RECT rcClient;
		LONG width, height;
		HDC hdc;
		CSkinItem *item = &SkinItems[0], *item_normal, *item_pressed, *item_hot;
		HICON hIcon;
		HFONT hOldFont = 0;
		TEXTMETRIC tm;

		if (!pContainer || !bSkinned)
			break;

		if (CSkin::m_frameSkins) {
			RECT rcWindow, rcClient;
			HDC dcFrame = GetDCEx(hwndDlg, 0, DCX_WINDOW|/*DCX_INTERSECTRGN|*/0x10000); // GetWindowDC(hwndDlg);
			POINT pt, pt1;
			LONG clip_top, clip_left;
			HRGN rgn = 0;
			CSkinItem *item;
			TCHAR szWindowText[512];
			RECT rcText;
			HDC dcMem = CreateCompatibleDC(pContainer->cachedDC ? pContainer->cachedDC : dcFrame);
			HBITMAP hbmMem, hbmOld;
			int i;
			DRAWITEMSTRUCT dis = {0};

			GetWindowRect(hwndDlg, &rcWindow);
			GetClientRect(hwndDlg, &rcClient);
			pt.y = 0;
			pt.x = 0;
			ClientToScreen(hwndDlg, &pt);
			pt1.x = rcClient.right;
			pt1.y = rcClient.bottom;
			ClientToScreen(hwndDlg, &pt1);
			clip_top = pt.y - rcWindow.top;
			clip_left = pt.x - rcWindow.left;

			rcWindow.right = rcWindow.right - rcWindow.left;
			rcWindow.bottom = rcWindow.bottom - rcWindow.top;
			rcWindow.left = rcWindow.top = 0;

			hbmMem = CreateCompatibleBitmap(dcFrame, rcWindow.right, rcWindow.bottom);
			hbmOld = (HBITMAP)SelectObject(dcMem, hbmMem);

			ExcludeClipRect(dcFrame, clip_left, clip_top, clip_left + (pt1.x - pt.x), clip_top + (pt1.y - pt.y));
			ExcludeClipRect(dcMem, clip_left, clip_top, clip_left + (pt1.x - pt.x), clip_top + (pt1.y - pt.y));
			item = pContainer->ncActive ? &SkinItems[ID_EXTBKFRAME] : &SkinItems[ID_EXTBKFRAMEINACTIVE];

			CSkin::DrawItem(dcMem, &rcWindow, item);

			GetWindowText(hwndDlg, szWindowText, SIZEOF(szWindowText));
			szWindowText[511] = 0;
			hOldFont = (HFONT)SelectObject(dcMem, PluginConfig.hFontCaption);
			GetTextMetrics(dcMem, &tm);
			SetTextColor(dcMem, CInfoPanel::m_ipConfig.clrs[IPFONTCOUNT - 1]);
			SetBkMode(dcMem, TRANSPARENT);
			rcText.left =20 + CSkin::m_SkinnedFrame_left + CSkin::m_bClipBorder + CSkin::m_titleBarLeftOff;//26;
			rcText.right = rcWindow.right - 3 * CSkin::m_titleBarButtonSize.cx - 11 - CSkin::m_titleBarRightOff;
			rcText.top = CSkin::m_captionOffset + CSkin::m_bClipBorder;
			rcText.bottom = rcText.top + tm.tmHeight;
			rcText.left += CSkin::m_captionPadding;
			DrawText(dcMem, szWindowText, -1, &rcText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
			SelectObject(dcMem, hOldFont);

			// icon			
			hIcon = (HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0);
			DrawIconEx(dcMem, 4 + CSkin::m_SkinnedFrame_left + CSkin::m_bClipBorder + CSkin::m_titleBarLeftOff, rcText.top + (rcText.bottom - rcText.top) / 2 - 8, hIcon, 16, 16, 0, 0, DI_NORMAL);

			// title buttons
			pContainer->rcClose.top = pContainer->rcMin.top = pContainer->rcMax.top = CSkin::m_titleButtonTopOff;
			pContainer->rcClose.bottom = pContainer->rcMin.bottom = pContainer->rcMax.bottom = CSkin::m_titleButtonTopOff + CSkin::m_titleBarButtonSize.cy;

			pContainer->rcClose.right = rcWindow.right - 10 - CSkin::m_titleBarRightOff;
			pContainer->rcClose.left = pContainer->rcClose.right - CSkin::m_titleBarButtonSize.cx;

			pContainer->rcMax.right = pContainer->rcClose.left - 2;
			pContainer->rcMax.left = pContainer->rcMax.right - CSkin::m_titleBarButtonSize.cx;

			pContainer->rcMin.right = pContainer->rcMax.left - 2;
			pContainer->rcMin.left = pContainer->rcMin.right - CSkin::m_titleBarButtonSize.cx;

			item_normal = &SkinItems[ID_EXTBKTITLEBUTTON];
			item_hot = &SkinItems[ID_EXTBKTITLEBUTTONMOUSEOVER];
			item_pressed = &SkinItems[ID_EXTBKTITLEBUTTONPRESSED];

			for (i=0; i < 3; i++) {
				RECT *rc = 0;
				HICON hIcon;

				switch (i) {
				case 0:
					rc = &pContainer->rcMin;
					hIcon = CSkin::m_minIcon;
					break;
				case 1:
					rc = &pContainer->rcMax;
					hIcon = CSkin::m_maxIcon;
					break;
				case 2:
					rc = &pContainer->rcClose;
					hIcon = CSkin::m_closeIcon;
					break;
				}
				if (rc) {
					item = pContainer->buttons[i].isPressed ? item_pressed : (pContainer->buttons[i].isHot ? item_hot : item_normal);
					CSkin::DrawItem(dcMem, rc, item);
					DrawIconEx(dcMem, rc->left + ((rc->right - rc->left) / 2 - 8), rc->top + ((rc->bottom - rc->top) / 2 - 8), hIcon, 16, 16, 0, 0, DI_NORMAL);
				}
			}
			SetBkMode(dcMem, TRANSPARENT);
			BitBlt(dcFrame, 0, 0, rcWindow.right, rcWindow.bottom, dcMem, 0, 0, SRCCOPY);
			SelectObject(dcMem, hbmOld);
			DeleteObject(hbmMem);
			DeleteDC(dcMem);
			ReleaseDC(hwndDlg, dcFrame);
		}
		else mir_callNextSubclass(hwndDlg, ContainerWndProc, msg, wParam, lParam);

		hdcReal = BeginPaint(hwndDlg, &ps);

		GetClientRect(hwndDlg, &rcClient);
		width = rcClient.right - rcClient.left;
		height = rcClient.bottom - rcClient.top;
		if (width != pContainer->oldDCSize.cx || height != pContainer->oldDCSize.cy) {
			CSkinItem *sbaritem = &SkinItems[ID_EXTBKSTATUSBAR];
			BOOL statusBarSkinnd = !(pContainer->dwFlags & CNT_NOSTATUSBAR) && !sbaritem->IGNORED;
			LONG sbarDelta = statusBarSkinnd ? pContainer->statusBarHeight : 0;

			pContainer->oldDCSize.cx = width;
			pContainer->oldDCSize.cy = height;

			if (pContainer->cachedDC) {
				SelectObject(pContainer->cachedDC, pContainer->oldHBM);
				DeleteObject(pContainer->cachedHBM);
				DeleteDC(pContainer->cachedDC);
			}
			pContainer->cachedDC = CreateCompatibleDC(hdcReal);
			pContainer->cachedHBM = CreateCompatibleBitmap(hdcReal, width, height);
			pContainer->oldHBM = (HBITMAP)SelectObject(pContainer->cachedDC, pContainer->cachedHBM);

			hdc = pContainer->cachedDC;

			if (!CSkin::DrawItem(hdc, &rcClient, item))
				FillRect(hdc, &rcClient, GetSysColorBrush(COLOR_3DFACE));

			if (sbarDelta) {
				rcClient.top = rcClient.bottom - sbarDelta;
				CSkin::DrawItem(hdc, &rcClient, sbaritem);
			}
		}
		BitBlt(hdcReal, 0, 0, width, height, pContainer->cachedDC, 0, 0, SRCCOPY);
		EndPaint(hwndDlg, &ps);
		return 0;
						  }
	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCMOUSEHOVER:
	case WM_NCMOUSEMOVE:
		if (pContainer && CSkin::m_frameSkins) {
			POINT pt;
			RECT rcWindow;
			BOOL isMin, isMax, isClose;
			int i;

			GetCursorPos(&pt);
			GetWindowRect(hwndDlg, &rcWindow);

			CopyMemory(&pContainer->oldbuttons[0], &pContainer->buttons[0], sizeof(TitleBtn) * 3);
			ZeroMemory(&pContainer->buttons[0], sizeof(TitleBtn) * 3);
			isMin = isMax = isClose = FALSE;

			if (pt.x >= (rcWindow.left + pContainer->rcMin.left) && pt.x <= (rcWindow.left + pContainer->rcClose.right) && pt.y < rcWindow.top + 24 && wParam != HTTOPRIGHT) {
				LRESULT result = 0; //DefWindowProc(hwndDlg, msg, wParam, lParam);
				HDC hdc = GetWindowDC(hwndDlg);
				LONG left = rcWindow.left;

				pt.y = 10;
				isMin = pt.x >= left + pContainer->rcMin.left && pt.x <= left + pContainer->rcMin.right;
				isMax = pt.x >= left + pContainer->rcMax.left && pt.x <= left + pContainer->rcMax.right;
				isClose = pt.x >= left + pContainer->rcClose.left && pt.x <= left + pContainer->rcClose.right;

				if (msg == WM_NCMOUSEMOVE) {
					if (isMax)
						pContainer->buttons[BTN_MAX].isHot = TRUE;
					else if (isMin)
						pContainer->buttons[BTN_MIN].isHot = TRUE;
					else if (isClose)
						pContainer->buttons[BTN_CLOSE].isHot = TRUE;
				}
				else if (msg == WM_NCLBUTTONDOWN) {
					if (isMax)
						pContainer->buttons[BTN_MAX].isPressed = TRUE;
					else if (isMin)
						pContainer->buttons[BTN_MIN].isPressed = TRUE;
					else if (isClose)
						pContainer->buttons[BTN_CLOSE].isPressed = TRUE;
				}
				else if (msg == WM_NCLBUTTONUP) {
					if (isMin)
						SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					else if (isMax) {
						if (IsZoomed(hwndDlg))
							PostMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
						else
							PostMessage(hwndDlg, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
					}
					else if (isClose)
						PostMessage(hwndDlg, WM_SYSCOMMAND, SC_CLOSE, 0);
				}
				for (i=0; i < 3; i++) {
					if (pContainer->buttons[i].isHot != pContainer->oldbuttons[i].isHot) {
						RECT *rc = 0;
						HICON hIcon;

						switch (i) {
						case 0:
							rc = &pContainer->rcMin;
							hIcon = CSkin::m_minIcon;
							break;
						case 1:
							rc = &pContainer->rcMax;
							hIcon = CSkin::m_maxIcon;
							break;
						case 2:
							rc = &pContainer->rcClose;
							hIcon = CSkin::m_closeIcon;
							break;
						}
						if (rc) {
							CSkinItem *item = &SkinItems[pContainer->buttons[i].isPressed ? ID_EXTBKTITLEBUTTONPRESSED : (pContainer->buttons[i].isHot ? ID_EXTBKTITLEBUTTONMOUSEOVER : ID_EXTBKTITLEBUTTON)];
							CSkin::DrawItem(hdc, rc, item);
							DrawIconEx(hdc, rc->left + ((rc->right - rc->left) / 2 - 8), rc->top + ((rc->bottom - rc->top) / 2 - 8), hIcon, 16, 16, 0, 0, DI_NORMAL);
						}
					}
				}
				ReleaseDC(hwndDlg, hdc);
				return result;
			}
			else {
				LRESULT result = DefWindowProc(hwndDlg, msg, wParam, lParam);
				RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
				return result;
			}
		}
		break;

	case WM_SETCURSOR:
		if (CSkin::m_frameSkins && (HWND)wParam == hwndDlg) {
			DefWindowProc(hwndDlg, msg, wParam, lParam);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
			return 1;
		}
		break;

	case WM_NCCALCSIZE:
		if (!CSkin::m_frameSkins)
			break;

		if (wParam) {
			RECT *rc;
			NCCALCSIZE_PARAMS *ncsp = (NCCALCSIZE_PARAMS *)lParam;

			DefWindowProc(hwndDlg, msg, wParam, lParam);
			rc = &ncsp->rgrc[0];

			rc->left += CSkin::m_realSkinnedFrame_left;
			rc->right -= CSkin::m_realSkinnedFrame_right;
			rc->bottom -= CSkin::m_realSkinnedFrame_bottom;
			rc->top += CSkin::m_realSkinnedFrame_caption;
			return TRUE;
		}

		return DefWindowProc(hwndDlg, msg, wParam, lParam);

	case WM_NCACTIVATE:
		if (pContainer) {
			pContainer->ncActive = wParam;
			if (bSkinned && CSkin::m_frameSkins) {
				SendMessage(hwndDlg, WM_NCPAINT, 0, 0);
				return 1;
			}
		}
		break;
	case WM_SETTEXT:
	case WM_SETICON:
		if (CSkin::m_frameSkins) {
			DefWindowProc(hwndDlg, msg, wParam, lParam);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
			return 0;
		}
		break;

	case WM_NCHITTEST:
		{
			RECT r;
			POINT pt;
			int k = 0;
			int clip = CSkin::m_bClipBorder;

			if (!pContainer)
				break;

			if (!(pContainer->dwFlags & CNT_NOTITLE))
				break;

			GetWindowRect(hwndDlg, &r);
			GetCursorPos(&pt);
			if (pt.y <= r.bottom && pt.y >= r.bottom - clip - 6) {
				if (pt.x > r.left + clip + 10 && pt.x < r.right - clip - 10)
					return HTBOTTOM;
				if (pt.x < r.left + clip + 10)
					return HTBOTTOMLEFT;
				if (pt.x > r.right - clip - 10)
					return HTBOTTOMRIGHT;

			}
			else if (pt.y >= r.top && pt.y <= r.top + 6) {
				if (pt.x > r.left + clip + 10 && pt.x < r.right - clip - 10)
					return HTTOP;
				if (pt.x < r.left + clip + 10)
					return HTTOPLEFT;
				if (pt.x > r.right - clip - 10)
					return HTTOPRIGHT;
			}
			else if (pt.x >= r.left && pt.x <= r.left + clip + 6)
				return HTLEFT;
			else if (pt.x >= r.right - clip - 6 && pt.x <= r.right)
				return HTRIGHT;
		}
		return(DefWindowProc(hwndDlg, WM_NCHITTEST, wParam, lParam));

	case 0xae:						// must be some undocumented message - seems it messes with the title bar...
		if (CSkin::m_frameSkins)
			return 0;
	}
	return mir_callNextSubclass(hwndDlg, ContainerWndProc, msg, wParam, lParam);
}

// container window procedure...
 
static BOOL fHaveTipper = FALSE;

static INT_PTR CALLBACK DlgProcContainer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int iItem = 0;
	TCITEM item;

	TContainerData *pContainer = (TContainerData*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	BOOL bSkinned = CSkin::m_skinEnabled ? TRUE : FALSE;
	HWND hwndTab = GetDlgItem(hwndDlg, IDC_MSGTABS);

	switch (msg) {
	case WM_INITDIALOG:
		{
			bool bAero = M.isAero();
			BOOL isFlat = M.GetByte("tbflat", 1);
			BOOL isThemed = !M.GetByte("nlflat", 0);

			fHaveTipper = ServiceExists("mToolTip/ShowTip");
			fForceOverlayIcons = M.GetByte("forceTaskBarStatusOverlays", 0) ? true : false;

			pContainer = (TContainerData*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pContainer);
			mir_subclassWindow(hwndDlg, ContainerWndProc);

			pContainer->hwnd = hwndDlg;
			DWORD dwCreateFlags = pContainer->dwFlags;

			pContainer->isCloned = (pContainer->dwFlags & CNT_CREATE_CLONED);
			pContainer->fPrivateThemeChanged = FALSE;

			SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);          // set options...
			pContainer->dwFlags |= dwCreateFlags;

			LoadOverrideTheme(pContainer);
			DWORD ws = GetWindowLongPtr(hwndTab, GWL_STYLE);
			if (pContainer->dwFlagsEx & TCF_FLAT)
				ws |= TCS_BUTTONS;

			memset((void*)&pContainer->mOld, -1000, sizeof(MARGINS));

			if (pContainer->dwFlagsEx & TCF_SINGLEROWTABCONTROL) {
				ws &= ~TCS_MULTILINE;
				ws |= TCS_SINGLELINE;
				ws |= TCS_FIXEDWIDTH;
			}
			else {
				ws &= ~TCS_SINGLELINE;
				ws |= TCS_MULTILINE;
				if (ws & TCS_BUTTONS)
					ws |= TCS_FIXEDWIDTH;
			}
			SetWindowLongPtr(hwndTab, GWL_STYLE, ws);

			pContainer->buttonItems = g_ButtonSet.items;

			pContainer->dwFlags = ((pContainer->dwFlagsEx & (TCF_SBARLEFT | TCF_SBARRIGHT)) ?
				pContainer->dwFlags | CNT_SIDEBAR : pContainer->dwFlags & ~CNT_SIDEBAR);

			pContainer->SideBar = new CSideBar(pContainer);
			pContainer->MenuBar = new CMenuBar(hwndDlg, pContainer);

			SetClassLongPtr(hwndDlg, GCL_STYLE, GetClassLongPtr(hwndDlg, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));
			SetClassLongPtr(hwndTab, GCL_STYLE, GetClassLongPtr(hwndTab, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));

			SetClassLongPtr(hwndDlg, GCL_STYLE, GetClassLongPtr(hwndDlg, GCL_STYLE) & ~CS_DROPSHADOW);

			// additional system menu items...
			HMENU hSysmenu = GetSystemMenu(hwndDlg, FALSE);
			int iMenuItems = GetMenuItemCount(hSysmenu);

			InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_SEPARATOR, 0, _T(""));
			InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_STRING, IDM_STAYONTOP, TranslateT("Stay on Top"));
			if (!CSkin::m_frameSkins)
				InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_STRING, IDM_NOTITLE, TranslateT("Hide title bar"));
			InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_SEPARATOR, 0, _T(""));
			InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_STRING, IDM_MOREOPTIONS, TranslateT("Container options..."));
			SetWindowText(hwndDlg, TranslateT("Message Session..."));
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)PluginConfig.g_iconContainer);

			// make the tab control the controlling parent window for all message dialogs
			
			ws = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_MSGTABS), GWL_EXSTYLE);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_MSGTABS), GWL_EXSTYLE, ws | WS_EX_CONTROLPARENT);

			LONG x_pad = M.GetByte("x-pad", 3) + (pContainer->dwFlagsEx & TCF_CLOSEBUTTON ? 7 : 0);
			LONG y_pad = M.GetByte("y-pad", 3) + ((pContainer->dwFlags & CNT_TABSBOTTOM) ? 1 : 0);

			if (pContainer->dwFlagsEx & TCF_FLAT)
				y_pad++; //(pContainer->dwFlags & CNT_TABSBOTTOM ? 1 : 2);

			TabCtrl_SetPadding(GetDlgItem(hwndDlg, IDC_MSGTABS), x_pad, y_pad);

			TabCtrl_SetImageList(GetDlgItem(hwndDlg, IDC_MSGTABS), PluginConfig.g_hImageList);

			SendMessage(hwndDlg, DM_CONFIGURECONTAINER, 0, 10);

			// context menu
			pContainer->hMenuContext = PluginConfig.g_hMenuContext;
			
			// tab tooltips...
			if (!fHaveTipper || M.GetByte("d_tooltips", 0) == 0) {
				pContainer->hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_hInst, (LPVOID) NULL);

				if (pContainer->hwndTip) {
					SetWindowPos(pContainer->hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					TabCtrl_SetToolTips(GetDlgItem(hwndDlg, IDC_MSGTABS), pContainer->hwndTip);
				}
			}
			else pContainer->hwndTip = 0;

			if (pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
				WINDOWPLACEMENT wp = {0};
				wp.length = sizeof(wp);

				SetWindowLongPtr(hwndDlg, GWL_STYLE, GetWindowLongPtr(hwndDlg, GWL_STYLE) & ~WS_VISIBLE);
				ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);
				SendMessage(hwndDlg, DM_RESTOREWINDOWPOS, 0, 0);
				//GetClientRect(hwndDlg, &pContainer->rcSaved);
				ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);
				GetWindowPlacement(hwndDlg, &wp);
				pContainer->rcSaved.left = pContainer->rcSaved.top = 0;
				pContainer->rcSaved.right = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				pContainer->rcSaved.bottom = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			}
			else {
				SendMessage(hwndDlg, DM_RESTOREWINDOWPOS, 0, 0);
				ShowWindow(hwndDlg, SW_SHOWNORMAL);
			}
		}

		// prevent ugly back background being visible while tabbed clients are created
		if (M.isAero()) {
			MARGINS m = {-1};
			CMimAPI::m_pfnDwmExtendFrameIntoClientArea(hwndDlg, &m);
		}
		return TRUE;

	case DM_RESTOREWINDOWPOS:
		// retrieve the container window geometry information from the database.
		if (pContainer->isCloned && pContainer->hContactFrom != 0 && !(pContainer->dwFlags & CNT_GLOBALSIZE)) {
			if (Utils_RestoreWindowPosition(hwndDlg, pContainer->hContactFrom, SRMSGMOD_T, "split")) {
				if (Utils_RestoreWindowPositionNoMove(hwndDlg, pContainer->hContactFrom, SRMSGMOD_T, "split"))
					if (Utils_RestoreWindowPosition(hwndDlg, NULL, SRMSGMOD_T, "split"))
						if (Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, SRMSGMOD_T, "split"))
							SetWindowPos(hwndDlg, 0, 50, 50, 450, 300, SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		else {
			if (pContainer->dwFlags & CNT_GLOBALSIZE) {
				if (Utils_RestoreWindowPosition(hwndDlg, NULL, SRMSGMOD_T, "split"))
					if (Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, SRMSGMOD_T, "split"))
						SetWindowPos(hwndDlg, 0, 50, 50, 450, 300, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else {
				char szCName[CONTAINER_NAMELEN + 20];
				mir_snprintf(szCName, sizeof(szCName), "%s%d", CONTAINER_PREFIX, pContainer->iContainerIndex);
				if (Utils_RestoreWindowPosition(hwndDlg, NULL, SRMSGMOD_T, szCName)) {
					if (Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, SRMSGMOD_T, szCName))
						if (Utils_RestoreWindowPosition(hwndDlg, NULL, SRMSGMOD_T, "split"))
							if (Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, SRMSGMOD_T, "split"))
								SetWindowPos(hwndDlg, 0, 50, 50, 450, 300, SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
		}
		return 0;

	case WM_SIZE:
		if (IsIconic(hwndDlg))
			pContainer->dwFlags |= CNT_DEFERREDSIZEREQUEST;
		else {
			RECT rcClient, rcUnadjusted;
			TCITEM item = {0};
			POINT pt = {0};

			GetClientRect(hwndDlg, &rcClient);
			pContainer->MenuBar->getClientRect();

			if (pContainer->hwndStatus) {
				TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
				SendMessage(pContainer->hwndStatus, WM_USER + 101, 0, (LPARAM)dat);

				RECT rcs;
				GetWindowRect(pContainer->hwndStatus, &rcs);
				pContainer->statusBarHeight = (rcs.bottom - rcs.top) + 1;
				SendMessage(pContainer->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
			}
			else pContainer->statusBarHeight = 0;

			CopyRect(&pContainer->rcSaved, &rcClient);
			rcUnadjusted = rcClient;

			pContainer->MenuBar->Resize(LOWORD(lParam), HIWORD(lParam), FALSE);
			LONG rebarHeight = pContainer->MenuBar->getHeight();
			pContainer->MenuBar->Show((pContainer->dwFlags & CNT_NOMENUBAR) ? SW_HIDE : SW_SHOW);

			LONG sbarWidth = pContainer->SideBar->getWidth();
			LONG sbarWidth_left = pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT ? sbarWidth : 0;

			if (lParam) {
				DWORD	dwSWPFlags = SWP_NOACTIVATE|SWP_NOZORDER |SWP_DEFERERASE | SWP_NOCOPYBITS; // | SWP_NOSENDCHANGING  | SWP_ASYNCWINDOWPOS;
				if (pContainer->dwFlags & CNT_TABSBOTTOM)
					SetWindowPos(hwndTab, 0, pContainer->tBorder_outer_left + sbarWidth_left, pContainer->tBorder_outer_top + rebarHeight,
						(rcClient.right - rcClient.left) - (pContainer->tBorder_outer_left + pContainer->tBorder_outer_right + sbarWidth),
						(rcClient.bottom - rcClient.top) - pContainer->statusBarHeight - (pContainer->tBorder_outer_top + pContainer->tBorder_outer_bottom) - rebarHeight, dwSWPFlags);
				else
					SetWindowPos(hwndTab, 0, pContainer->tBorder_outer_left + sbarWidth_left, pContainer->tBorder_outer_top + rebarHeight,
						(rcClient.right - rcClient.left) - (pContainer->tBorder_outer_left + pContainer->tBorder_outer_right + sbarWidth),
						(rcClient.bottom - rcClient.top) - pContainer->statusBarHeight - (pContainer->tBorder_outer_top + pContainer->tBorder_outer_bottom) - rebarHeight, dwSWPFlags);
			}

			pContainer->SideBar->resizeScrollWnd(sbarWidth_left ? pContainer->tBorder_outer_left : rcClient.right - pContainer->tBorder_outer_right - (sbarWidth - 2),
				pContainer->tBorder_outer_top + rebarHeight, 0,
				(rcClient.bottom - rcClient.top) - pContainer->statusBarHeight - (pContainer->tBorder_outer_top + pContainer->tBorder_outer_bottom) - rebarHeight);

			AdjustTabClientRect(pContainer, &rcClient);

			BOOL sizeChanged = (((rcClient.right - rcClient.left) != pContainer->preSIZE.cx) || ((rcClient.bottom - rcClient.top) != pContainer->preSIZE.cy));
			if (sizeChanged) {
				pContainer->preSIZE.cx = rcClient.right - rcClient.left;
				pContainer->preSIZE.cy = rcClient.bottom - rcClient.top;
			}

			// we care about all client sessions, but we really resize only the active tab (hwndActive)
			// we tell inactive tabs to resize theirselves later when they get activated (DM_CHECKSIZE
			// just queues a resize request)
			int nCount = TabCtrl_GetItemCount(hwndTab);

			for (int i=0; i < nCount; i++) {
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab, i, &item);
				if ((HWND)item.lParam == pContainer->hwndActive) {
					SetWindowPos((HWND)item.lParam, 0, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top),
						SWP_NOSENDCHANGING|SWP_NOACTIVATE/*|SWP_NOCOPYBITS*/);
					if (!pContainer->bSizingLoop && sizeChanged) {
						TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
						DM_ScrollToBottom(dat, 0, 1);
					}
				}
				else if (sizeChanged)
					SendMessage((HWND)item.lParam, DM_CHECKSIZE, 0, 0);
			}
			pContainer->SideBar->scrollIntoView();

			if (!M.isAero()) {					// aero mode uses buffered paint, no forced redraw needed
				RedrawWindow(hwndTab, NULL, NULL, RDW_INVALIDATE | (pContainer->bSizingLoop ? RDW_ERASE : 0));
				RedrawWindow(hwndDlg, NULL, NULL, (bSkinned ? RDW_FRAME : 0) | RDW_INVALIDATE | ((pContainer->bSizingLoop || wParam == SIZE_RESTORED ) ? RDW_ERASE : 0));
			}

			if (pContainer->hwndStatus)
				InvalidateRect(pContainer->hwndStatus, NULL, FALSE);

			if ((CSkin::m_bClipBorder != 0 || CSkin::m_bRoundedCorner) && CSkin::m_frameSkins) {
				HRGN rgn;
				int clip = CSkin::m_bClipBorder;

				RECT rcWindow;
				GetWindowRect(hwndDlg, &rcWindow);

				if (CSkin::m_bRoundedCorner)
					rgn = CreateRoundRectRgn(clip, clip, (rcWindow.right - rcWindow.left) - clip + 1,
						(rcWindow.bottom - rcWindow.top) - clip + 1, CSkin::m_bRoundedCorner + clip, CSkin::m_bRoundedCorner + clip);
				else
					rgn = CreateRectRgn(clip, clip, (rcWindow.right - rcWindow.left) - clip, (rcWindow.bottom - rcWindow.top) - clip);
				SetWindowRgn(hwndDlg, rgn, TRUE);
			}
			else if (CSkin::m_frameSkins)
				SetWindowRgn(hwndDlg, NULL, TRUE);
		}
		break;

	case WM_NOTIFY:
		if (pContainer->MenuBar) {
			LRESULT processed = pContainer->MenuBar->processMsg(msg, wParam, lParam);
			if (processed != -1) {
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, processed);
				return(processed);
			}
		}

		if (pContainer->hwndStatus != 0 && ((LPNMHDR)lParam)->hwndFrom == pContainer->hwndStatus) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
			case NM_RCLICK:
				unsigned int nParts, nPanel;
				NMMOUSE *nm = (NMMOUSE*)lParam;
				RECT rc;

				nParts = SendMessage(pContainer->hwndStatus, SB_GETPARTS, 0, 0);
				if (nm->dwItemSpec == 0xFFFFFFFE) {
					nPanel = 2;
					SendMessage(pContainer->hwndStatus, SB_GETRECT, nPanel, (LPARAM)&rc);
					if (nm->pt.x > rc.left && nm->pt.x < rc.right)
						goto panel_found;
					else
						return FALSE;
				}
				else nPanel = nm->dwItemSpec;
panel_found:
				if (nPanel == 2) {
					TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
					SendMessage(pContainer->hwndStatus, SB_GETRECT, nPanel, (LPARAM)&rc);
					if (dat)
						SI_CheckStatusIconClick(dat, pContainer->hwndStatus, nm->pt, rc, 2, ((LPNMHDR)lParam)->code);
				}
				else if (((LPNMHDR)lParam)->code == NM_RCLICK) {
					POINT pt;
					GetCursorPos(&pt);
					MCONTACT hContact = 0;
					SendMessage(pContainer->hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
					if (hContact) {
						int iSel = 0;
						HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
						iSel = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
						if (iSel)
							CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(iSel), MPCF_CONTACTMENU), hContact);
						DestroyMenu(hMenu);
					}
				}
				return TRUE;
			}
			break;
		}

		switch (((LPNMHDR)lParam)->code) {
		case TCN_SELCHANGE:
			ZeroMemory(&item, sizeof(item));
			iItem = TabCtrl_GetCurSel(hwndTab);
			item.mask = TCIF_PARAM;
			if (TabCtrl_GetItem(hwndTab, iItem, &item)) {
				if ((HWND)item.lParam != pContainer->hwndActive) {
					if (pContainer->hwndActive && IsWindow(pContainer->hwndActive))
						ShowWindow(pContainer->hwndActive, SW_HIDE);
				}
				pContainer->hwndActive = (HWND)item.lParam;
				SendMessage((HWND)item.lParam, DM_SAVESIZE, 0, 1);
				ShowWindow((HWND)item.lParam, SW_SHOW);
				if (!IsIconic(hwndDlg))
					SetFocus(pContainer->hwndActive);
			}
			SendMessage(hwndTab, EM_VALIDATEBOTTOM, 0, 0);
			return 0;

		// tooltips
		case NM_RCLICK:
			{
				int iItem;
				bool fFromSidebar = false;
				TCITEM item = { 0 };
				TWindowData *dat = 0;

				POINT pt, pt1;
				GetCursorPos(&pt);
				pt1 = pt;
				HMENU subMenu = GetSubMenu(pContainer->hMenuContext, 0);

				if (((LPNMHDR)lParam)->idFrom == IDC_MSGTABS) {
					if ((iItem = GetTabItemFromMouse(hwndTab, &pt)) == -1)
						break;

					item.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwndTab, iItem, &item);
					if (item.lParam && IsWindow((HWND)item.lParam))
						dat = (TWindowData*)GetWindowLongPtr((HWND)item.lParam, GWLP_USERDATA);
				}
				// sent from a sidebar button (RMB click) instead of the tab control
				else if (((LPNMHDR)lParam)->idFrom == 5000) {
					TSideBarNotify* n = reinterpret_cast<TSideBarNotify *>(lParam);
					dat = const_cast<TWindowData *>(n->dat);
					fFromSidebar = true;
				}

				if (dat)
					MsgWindowUpdateMenu(dat, subMenu, MENU_TABCONTEXT);

				int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt1.x, pt1.y, 0, hwndDlg, NULL);
				if (iSelection >= IDM_CONTAINERMENU) {
					DBVARIANT dbv = {0};
					char szIndex[10];
					itoa(iSelection - IDM_CONTAINERMENU, szIndex, 10);
					if (iSelection - IDM_CONTAINERMENU >= 0) {
						ptrT tszName(db_get_tsa(NULL, CONTAINER_KEY, szIndex));
						if (tszName != NULL)
							SendMessage((HWND)item.lParam, DM_CONTAINERSELECTED, 0, tszName);
					}
					return 1;
				}
				switch (iSelection) {
				case ID_TABMENU_CLOSETAB:
					if (fFromSidebar)
						SendMessage(dat->hwnd, WM_CLOSE, 1, 0);
					else
						SendMessage(hwndDlg, DM_CLOSETABATMOUSE, 0, (LPARAM)&pt1);
					break;
				case ID_TABMENU_CLOSEOTHERTABS:
					CloseOtherTabs(hwndTab, *dat);
					break;
				case ID_TABMENU_SAVETABPOSITION:
					db_set_dw(dat->hContact, SRMSGMOD_T, "tabindex", dat->iTabID * 100);
					break;
				case ID_TABMENU_CLEARSAVEDTABPOSITION:
					db_unset(dat->hContact, SRMSGMOD_T, "tabindex");
					break;
				case ID_TABMENU_LEAVECHATROOM:
					if (dat && dat->bType == SESSIONTYPE_CHAT) {
						SESSION_INFO *si = dat->si;
						if (si && dat->hContact) {
							char *szProto = GetContactProto(dat->hContact);
							if (szProto)
								CallProtoService( szProto, PS_LEAVECHAT, dat->hContact, 0 );
						}
					}
					break;
				case ID_TABMENU_ATTACHTOCONTAINER:
					if ((iItem = GetTabItemFromMouse(hwndTab, &pt1)) == -1)
						break;
					ZeroMemory(&item, sizeof(item));
					item.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwndTab, iItem, &item);
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SELECTCONTAINER), hwndDlg, SelectContainerDlgProc, (LPARAM)item.lParam);
					break;
				case ID_TABMENU_CONTAINEROPTIONS:
					if (pContainer->hWndOptions == 0)
						CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), hwndDlg, DlgProcContainerOptions, (LPARAM)pContainer);
					break;
				case ID_TABMENU_CLOSECONTAINER:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				}
				InvalidateRect(hwndTab, NULL, FALSE);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		{
			bool fProcessContactMenu = pContainer->MenuBar->isContactMenu();
			bool fProcessMainMenu = pContainer->MenuBar->isMainMenu();
			pContainer->MenuBar->Cancel();

			MCONTACT hContact;
			TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
			DWORD dwOldFlags = pContainer->dwFlags;
			ButtonItem *pItem = pContainer->buttonItems;

			if (dat) {
				DWORD dwOldMsgWindowFlags = dat->dwFlags;
				DWORD dwOldEventIsShown = dat->dwFlagsEx;

				if (fProcessContactMenu)
					return(CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact));
				else if (fProcessMainMenu) {
					return(CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_MAINMENU), 0));
				}
				else if (MsgWindowMenuHandler(dat, LOWORD(wParam), MENU_PICMENU) == 1)
					break;
			}
			SendMessage(pContainer->hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
			if (LOWORD(wParam) == IDC_TBFIRSTUID - 1)
				break;

			switch (LOWORD(wParam)) {
			case IDC_TOGGLESIDEBAR:
				{
					RECT rc;
					LONG dwNewLeft;
					BOOL skinnedMode = bSkinned | (IsThemeActive() ? 1 : 0);

					GetWindowRect(hwndDlg, &rc);

					bool fVisible = pContainer->SideBar->isVisible();
					if (fVisible) {
						dwNewLeft = pContainer->SideBar->getWidth();
						pContainer->SideBar->setVisible(false);
					}
					else {
						pContainer->SideBar->setVisible(true);
						dwNewLeft = -(pContainer->SideBar->getWidth());
					}

					pContainer->preSIZE.cx = pContainer->preSIZE.cy = 0;
					pContainer->oldDCSize.cx = pContainer->oldDCSize.cy = 0;

					PostMessage(hwndDlg, WM_SIZE, 0, 1);
				}
				break;

			case IDC_SIDEBARDOWN:
			case IDC_SIDEBARUP:
				{
					HWND hwnd = GetFocus();
					pContainer->SideBar->processScrollerButtons(LOWORD(wParam));
					SetFocus(hwnd);
				}
				break;

			default:
				Utils::CmdDispatcher(Utils::CMD_CONTAINER, hwndDlg, LOWORD(wParam), wParam, lParam, 0, pContainer);
			}
			if (pContainer->dwFlags != dwOldFlags)
				SendMessage(hwndDlg, DM_CONFIGURECONTAINER, 0, 0);
		}
		break;

	case WM_ENTERSIZEMOVE:
		{
			RECT rc;
			GetClientRect(GetDlgItem(hwndDlg, IDC_MSGTABS), &rc);

			SIZE sz;
			sz.cx = rc.right - rc.left;
			sz.cy = rc.bottom - rc.top;
			pContainer->oldSize = sz;
			pContainer->bSizingLoop = TRUE;
		}
		break;

	case WM_EXITSIZEMOVE:
		{
			RECT rc;
			GetClientRect(GetDlgItem(hwndDlg, IDC_MSGTABS), &rc);
			if (!((rc.right - rc.left) == pContainer->oldSize.cx && (rc.bottom - rc.top) == pContainer->oldSize.cy)) {
				TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
				DM_ScrollToBottom(dat, 0, 0);
				SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
			}
			pContainer->bSizingLoop = FALSE;
		}
		break;

	// determine minimum and maximum size limits
	// 1) for maximizing the window when the "vertical maximize" option is set
	// 2) to limit the minimum height when manually resizing the window
	// (this avoids overlapping of controls inside the window and ensures
	// that at least 2 lines of the message log are always visible).
	case WM_GETMINMAXINFO:
		{
			RECT rc, rcWindow, rcClient = {0};
			POINT pt;

			MINMAXINFO *mmi = (MINMAXINFO *) lParam;
			mmi->ptMinTrackSize.x = 275;
			mmi->ptMinTrackSize.y = 130;
			GetClientRect(GetDlgItem(hwndDlg, IDC_MSGTABS), &rc);
			if (pContainer->hwndActive)								// at container creation time, there is no hwndActive yet..
				GetClientRect(pContainer->hwndActive, &rcClient);
			GetWindowRect(hwndDlg, &rcWindow);
			pt.y = rc.top;
			TabCtrl_AdjustRect(GetDlgItem(hwndDlg, IDC_MSGTABS), FALSE, &rc);
			// uChildMinHeight holds the min height for the client window only
			// so let's add the container's vertical padding (title bar, tab bar,
			// window border, status bar) to this value
			if (pContainer->hwndActive)
				mmi->ptMinTrackSize.y = pContainer->uChildMinHeight + (pContainer->hwndActive ? ((rcWindow.bottom - rcWindow.top) - rcClient.bottom) : 0);

			if (pContainer->dwFlags & CNT_VERTICALMAX || (GetKeyState(VK_CONTROL) & 0x8000)) {
				RECT rcDesktop = {0};
				BOOL fDesktopValid = FALSE;
				int monitorXOffset = 0;
				WINDOWPLACEMENT wp = {0};

				HMONITOR hMonitor = MonitorFromWindow(hwndDlg, 2);
				if (hMonitor) {
					MONITORINFO mi = { 0 };
					mi.cbSize = sizeof(mi);
					GetMonitorInfoA(hMonitor, &mi);
					rcDesktop = mi.rcWork;
					OffsetRect(&rcDesktop, -mi.rcMonitor.left, -mi.rcMonitor.top);
					monitorXOffset = mi.rcMonitor.left;
					fDesktopValid = TRUE;
				}
				if (!fDesktopValid)
					SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);

				wp.length = sizeof(wp);
				GetWindowPlacement(hwndDlg, &wp);
				mmi->ptMaxSize.y = rcDesktop.bottom - rcDesktop.top;
				mmi->ptMaxSize.x = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				mmi->ptMaxPosition.x = wp.rcNormalPosition.left - monitorXOffset;
				mmi->ptMaxPosition.y = 0;
				if (IsIconic(hwndDlg)) {
					mmi->ptMaxPosition.x += rcDesktop.left;
					mmi->ptMaxPosition.y += rcDesktop.top;
				}

				// protect against invalid values...
				if (mmi->ptMinTrackSize.y < 50 || mmi->ptMinTrackSize.y > rcDesktop.bottom)
					mmi->ptMinTrackSize.y = 130;
			}
		}
		return 0;

	case DM_UPDATETITLE:
		{
			MCONTACT hContact = 0;
			TWindowData *dat = NULL;

			if (lParam) {               // lParam != 0 means sent by a chat window
				TCHAR szText[512];
				dat = (TWindowData*)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
				GetWindowText((HWND)wParam, szText, SIZEOF(szText));
				szText[SIZEOF(szText)-1] = 0;
				SetWindowText(hwndDlg, szText);
				if (dat)
					SendMessage(hwndDlg, DM_SETICON, (WPARAM)dat, (LPARAM)(dat->hTabIcon != dat->hTabStatusIcon ? dat->hTabIcon : dat->hTabStatusIcon));
				return 0;
			}
			if (wParam == 0) {           // no hContact given - obtain the hContact for the active tab
				if (pContainer->hwndActive && IsWindow(pContainer->hwndActive))
					SendMessage(pContainer->hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
				else
					break;
				dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
			}
			else {
				HWND hwnd = M.FindWindow(wParam);
				if (hwnd == 0) {
					SESSION_INFO *si = SM_FindSessionByHCONTACT(wParam);
					if (si) {
						SendMessage(si->hWnd, GC_UPDATETITLE, 0, 0);
						return 0;
					}
				}
				hContact = wParam;
				if (hwnd && hContact)
					dat = (TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			}
			if (dat) {
				SendMessage(hwndDlg, DM_SETICON, (WPARAM)dat, (LPARAM)(dat->hXStatusIcon ? dat->hXStatusIcon : dat->hTabStatusIcon));
				const TCHAR *szNewTitle = Utils::FormatTitleBar(dat, pContainer->settings->szTitleFormat);
				if (szNewTitle) {
					SetWindowText(hwndDlg, szNewTitle);
					mir_free((void*)szNewTitle);
				}
			}
		}
		return 0;

	case WM_TIMER:
		if (wParam == TIMERID_HEARTBEAT) {
			TWindowData *dat = 0;
			if (GetForegroundWindow() != hwndDlg && (pContainer->settings->autoCloseSeconds > 0) && !pContainer->fHidden) {
				BOOL fResult = TRUE;
				BroadCastContainer(pContainer, DM_CHECKAUTOHIDE, (WPARAM)pContainer->settings->autoCloseSeconds, (LPARAM)&fResult);

				if (fResult && 0 == pContainer->hWndOptions)
					PostMessage(hwndDlg, WM_CLOSE, 1, 0);
			}
			dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
			if (dat && dat->bType == SESSIONTYPE_IM) {
				if (dat->idle && pContainer->hwndActive && IsWindow(pContainer->hwndActive))
					dat->Panel->Invalidate(TRUE);
			}
			else if (dat)
				SendMessage(dat->hwnd, GC_UPDATESTATUSBAR, 0, 0);
		}
		else if (wParam == TIMERID_HOVER) {
			RECT rcWindow;
			GetWindowRect(hwndDlg, &rcWindow);
		}
		break;

	case WM_SYSCOMMAND:
		switch (wParam) {
		case IDM_STAYONTOP:
			SetWindowPos(hwndDlg, (pContainer->dwFlags & CNT_STICKY) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			CheckMenuItem(GetSystemMenu(hwndDlg, FALSE), IDM_STAYONTOP, (pContainer->dwFlags & CNT_STICKY) ? MF_BYCOMMAND | MF_UNCHECKED : MF_BYCOMMAND | MF_CHECKED);
			ApplyContainerSetting(pContainer, CNT_STICKY, pContainer->dwFlags & CNT_STICKY ? 0 : 1, false);
			break;
		case IDM_NOTITLE:
			pContainer->oldSize.cx = 0;
			pContainer->oldSize.cy = 0;

			CheckMenuItem(GetSystemMenu(hwndDlg, FALSE), IDM_NOTITLE, (pContainer->dwFlags & CNT_NOTITLE) ? MF_BYCOMMAND | MF_UNCHECKED : MF_BYCOMMAND | MF_CHECKED);
			ApplyContainerSetting(pContainer, CNT_NOTITLE, pContainer->dwFlags & CNT_NOTITLE ? 0 : 1, false);
			break;
		case IDM_MOREOPTIONS:
			if (IsIconic(pContainer->hwnd))
				SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			if (pContainer->hWndOptions == 0)
				CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), hwndDlg, DlgProcContainerOptions, (LPARAM)pContainer);
			break;
		case SC_MAXIMIZE:
			pContainer->oldSize.cx = pContainer->oldSize.cy = 0;
			break;
		case SC_RESTORE:
			pContainer->oldSize.cx = pContainer->oldSize.cy = 0;
			memset((void*)&pContainer->mOld, -1000, sizeof(MARGINS));
			break;
		case SC_MINIMIZE:
			TWindowData *dat = reinterpret_cast<TWindowData *>(GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA));
			if (dat) {
				//GetWindowRect(GetDlgItem(pContainer->hwndActive, dat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG), &pContainer->rcLogSaved);
				GetWindowRect(pContainer->hwndActive, &pContainer->rcLogSaved);
				pContainer->ptLogSaved.x = pContainer->rcLogSaved.left;
				pContainer->ptLogSaved.y = pContainer->rcLogSaved.top;
				ScreenToClient(hwndDlg, &pContainer->ptLogSaved);
			}
		}
		break;

	case DM_SELECTTAB:
		switch (wParam) {
		int iItems, iCurrent, iNewTab;

		case DM_SELECT_BY_HWND:
			ActivateTabFromHWND(hwndTab, (HWND)lParam);
			break;

		case DM_SELECT_NEXT:
		case DM_SELECT_PREV:
		case DM_SELECT_BY_INDEX:
			iItems = TabCtrl_GetItemCount(hwndTab);
			iCurrent = TabCtrl_GetCurSel(hwndTab);

			if (iItems == 1)
				break;
			if (wParam == DM_SELECT_PREV)
				iNewTab = iCurrent ? iCurrent - 1 : iItems - 1;     // cycle if current is already the leftmost tab..
			else if (wParam == DM_SELECT_NEXT)
				iNewTab = (iCurrent == (iItems - 1)) ? 0 : iCurrent + 1;
			else if (wParam == DM_SELECT_BY_INDEX) {
				if ((int)lParam > iItems)
					break;
				iNewTab = lParam - 1;
			}

			if (iNewTab != iCurrent) {
				TabControlData *tabdat = (TabControlData *)GetWindowLongPtr(hwndTab, GWLP_USERDATA);
				ZeroMemory(&item, sizeof(item));
				item.mask = TCIF_PARAM;
				if (TabCtrl_GetItem(hwndTab, iNewTab, &item)) {
					TabCtrl_SetCurSel(hwndTab, iNewTab);
					ShowWindow(pContainer->hwndActive, SW_HIDE);
					pContainer->hwndActive = (HWND)item.lParam;
					ShowWindow((HWND)item.lParam, SW_SHOW);
					SetFocus(pContainer->hwndActive);
				}
			}
			break;
		}
		break;

	case WM_INITMENUPOPUP:
		pContainer->MenuBar->setActive(reinterpret_cast<HMENU>(wParam));
		break;

	case WM_LBUTTONDOWN:
		if (pContainer->dwFlags & CNT_NOTITLE) {
			POINT pt;
			GetCursorPos(&pt);
			return SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		break;

	// pass the WM_ACTIVATE msg to the active message dialog child
	case WM_NCACTIVATE:
		if (IsWindowVisible(hwndDlg))
			pContainer->fHidden = false;
		break;

	case WM_ACTIVATE:
		if (pContainer == NULL)
			break;

		if (LOWORD(wParam == WA_INACTIVE))
			BroadCastContainer(pContainer, DM_CHECKINFOTIP, wParam, lParam);

		if (LOWORD(wParam == WA_INACTIVE) && (HWND)lParam != PluginConfig.g_hwndHotkeyHandler && GetParent((HWND)lParam) != hwndDlg) {
			BOOL fTransAllowed = !bSkinned || PluginConfig.m_bIsVista;

			if (pContainer->dwFlags & CNT_TRANSPARENCY && fTransAllowed) {
				SetLayeredWindowAttributes(hwndDlg, Skin->getColorKey(), (BYTE)HIWORD(pContainer->settings->dwTransparency), (pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
			}
		}
		pContainer->hwndSaved = 0;

		if (LOWORD(wParam) != WA_ACTIVE) {
			pContainer->MenuBar->Cancel();
			break;
		}

	case WM_MOUSEACTIVATE:
		if (pContainer != NULL) {
			TCITEM item;
			int curItem = 0;
			BOOL  fTransAllowed = !bSkinned || PluginConfig.m_WinVerMajor >= 6;

			FlashContainer(pContainer, 0, 0);
			pContainer->dwFlashingStarted = 0;
			pLastActiveContainer = pContainer;
			if (pContainer->dwFlags & CNT_DEFERREDTABSELECT) {
				pContainer->dwFlags &= ~CNT_DEFERREDTABSELECT;
				SendMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);

				NMHDR nmhdr = { hwndTab, IDC_MSGTABS, TCN_SELCHANGE };
				SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nmhdr);     // do it via a WM_NOTIFY / TCN_SELCHANGE to simulate user-activation
			}
			if (pContainer->dwFlags & CNT_DEFERREDSIZEREQUEST) {
				pContainer->dwFlags &= ~CNT_DEFERREDSIZEREQUEST;
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
			}

			if (pContainer->dwFlags & CNT_TRANSPARENCY && fTransAllowed) {
				DWORD trans = LOWORD(pContainer->settings->dwTransparency);
				SetLayeredWindowAttributes(hwndDlg, Skin->getColorKey(), (BYTE)trans, (pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
			}
			if (pContainer->dwFlags & CNT_NEED_UPDATETITLE) {
				MCONTACT hContact = 0;
				pContainer->dwFlags &= ~CNT_NEED_UPDATETITLE;
				if (pContainer->hwndActive) {
					SendMessage(pContainer->hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
					if (hContact)
						SendMessage(hwndDlg, DM_UPDATETITLE, hContact, 0);
				}
			}
			ZeroMemory(&item, sizeof(item));
			item.mask = TCIF_PARAM;
			if ((curItem = TabCtrl_GetCurSel(hwndTab)) >= 0)
				TabCtrl_GetItem(hwndTab, curItem, &item);
			if (pContainer->dwFlags & CNT_DEFERREDCONFIGURE && curItem >= 0) {
				pContainer->dwFlags &= ~CNT_DEFERREDCONFIGURE;
				pContainer->hwndActive = (HWND)item.lParam;
				SendMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
				if (pContainer->hwndActive != 0 && IsWindow(pContainer->hwndActive)) {
					ShowWindow(pContainer->hwndActive, SW_SHOW);
					SetFocus(pContainer->hwndActive);
					SendMessage(pContainer->hwndActive, WM_ACTIVATE, WA_ACTIVE, 0);
					RedrawWindow(pContainer->hwndActive, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
				}
			}
			else if (curItem >= 0)
				SendMessage((HWND)item.lParam, WM_ACTIVATE, WA_ACTIVE, 0);
		}
		break;

	case DM_CLOSETABATMOUSE:
		{
			HWND hwndCurrent = pContainer->hwndActive;
			POINT *pt = (POINT *)lParam;
			if ((iItem = GetTabItemFromMouse(hwndTab, pt)) == -1)
				break;

			TCITEM item = {0};
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, iItem, &item);
			if (item.lParam) {
				if ((HWND)item.lParam != hwndCurrent) {
					pContainer->bDontSmartClose = TRUE;
					SendMessage((HWND)item.lParam, WM_CLOSE, 0, 1);
					RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
					pContainer->bDontSmartClose = FALSE;
				}
				else SendMessage((HWND)item.lParam, WM_CLOSE, 0, 1);
			}
		}
		break;

	case WM_PAINT:
		if (bSkinned || M.isAero()) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwndDlg, &ps);
			EndPaint(hwndDlg, &ps);
			return 0;
		}
		break;

	case WM_ERASEBKGND:
		// avoid flickering of the menu bar when aero is active
		if (pContainer) {
			HDC hdc = (HDC)wParam;
			RECT rc;
			GetClientRect(hwndDlg, &rc);

			if (M.isAero()) {
				HDC hdcMem;
				HANDLE  hbp = CMimAPI::m_pfnBeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, 0, &hdcMem);
				FillRect(hdcMem, &rc, CSkin::m_BrushBack);
				CSkin::FinalizeBufferedPaint(hbp, &rc);
			}
			else {
				if (CSkin::m_skinEnabled)
					CSkin::DrawItem(hdc, &rc, &SkinItems[ID_EXTBKCONTAINER]);
				else {
					CSkin::FillBack(hdc, &rc);
					if (pContainer->SideBar->isActive() && pContainer->SideBar->isVisible()) {

						HPEN hPen = ::CreatePen(PS_SOLID, 1, PluginConfig.m_cRichBorders ? PluginConfig.m_cRichBorders : ::GetSysColor(COLOR_3DSHADOW));
						HPEN hOldPen = reinterpret_cast<HPEN>(::SelectObject(hdc, hPen));
						LONG x = (pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT ? pContainer->SideBar->getWidth() - 2 + pContainer->tBorder_outer_left :
							rc.right - pContainer->SideBar->getWidth() + 1 - pContainer->tBorder_outer_right);
						::MoveToEx(hdc, x, rc.top, 0);
						::LineTo(hdc, x, rc.bottom);
						::SelectObject(hdc, hOldPen);
						::DeleteObject(hPen);
					}
				}
			}
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1);
			return TRUE;
		}
		break;

	case DM_OPTIONSAPPLIED:
		{
			char 		szCname[40];
			TCHAR 		szTitleFormat[200];
			TCHAR*		szThemeName = NULL;
			DBVARIANT 	dbv = {0};

			szTitleFormat[0] = 0;

			if (pContainer->isCloned && pContainer->hContactFrom != 0) {
				pContainer->settings = &PluginConfig.globalContainerSettings;

				pContainer->szRelThemeFile[0] = pContainer->szAbsThemeFile[0] = 0;
				mir_snprintf(szCname, 40, "%s_theme", CONTAINER_PREFIX);
				if (!db_get_ts(pContainer->hContactFrom, SRMSGMOD_T, szCname, &dbv))
					szThemeName = dbv.ptszVal;
			}
			else {
				Utils::ReadPrivateContainerSettings(pContainer);
				if (szThemeName == NULL) {
					mir_snprintf(szCname, 40, "%s%d_theme", CONTAINER_PREFIX, pContainer->iContainerIndex);
					if (!db_get_ts(NULL, SRMSGMOD_T, szCname, &dbv))
						szThemeName = dbv.ptszVal;
				}
			}
			Utils::SettingsToContainer(pContainer);

			if (szThemeName != NULL) {
				PathToAbsoluteT(szThemeName, pContainer->szAbsThemeFile, M.getDataPath());
				mir_sntprintf(pContainer->szRelThemeFile, MAX_PATH, _T("%s"), szThemeName);
				db_free(&dbv);
			}
			else pContainer->szAbsThemeFile[0] = pContainer->szRelThemeFile[0] = 0;

			pContainer->ltr_templates = pContainer->rtl_templates = 0;
		}
		break;

	case DM_STATUSBARCHANGED:
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		{
			RECT rc;
			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg,  0, rc.left, rc.top, rc.right - rc.left, (rc.bottom - rc.top) + 1, SWP_NOZORDER | SWP_NOACTIVATE);
			SetWindowPos(hwndDlg,  0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
		}
		if (pContainer->hwndStatus != 0 && pContainer->hwndActive != 0)
			PostMessage(pContainer->hwndActive, DM_STATUSBARCHANGED, 0, 0);
		return 0;

	case DM_CONFIGURECONTAINER:
		{
			DWORD ws, wsold, ex = 0, exold = 0;
			HMENU hSysmenu = GetSystemMenu(hwndDlg, FALSE);
			MCONTACT hContact = 0;
			int i=0;
			UINT sBarHeight;
			bool bAero = M.isAero();

			ws = wsold = GetWindowLongPtr(hwndDlg, GWL_STYLE);
			if (!CSkin::m_frameSkins) {
				ws = (pContainer->dwFlags & CNT_NOTITLE) ?
					((IsWindowVisible(hwndDlg) ? WS_VISIBLE : 0) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME | (CSkin::m_frameSkins ? WS_SYSMENU : WS_SYSMENU | WS_SIZEBOX)) :
					ws | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
			}

			SetWindowLongPtr(hwndDlg, GWL_STYLE, ws);

			pContainer->tBorder = M.GetByte((bSkinned ? "S_tborder" : "tborder"), 2);
			pContainer->tBorder_outer_left = g_ButtonSet.left + M.GetByte((bSkinned ? "S_tborder_outer_left" : "tborder_outer_left"), 2);
			pContainer->tBorder_outer_right = g_ButtonSet.right + M.GetByte((bSkinned ? "S_tborder_outer_right" : "tborder_outer_right"), 2);
			pContainer->tBorder_outer_top = g_ButtonSet.top + M.GetByte((bSkinned ? "S_tborder_outer_top" : "tborder_outer_top"), 2);
			pContainer->tBorder_outer_bottom = g_ButtonSet.bottom + M.GetByte((bSkinned ? "S_tborder_outer_bottom" : "tborder_outer_bottom"), 2);
			sBarHeight = (UINT)M.GetByte((bSkinned ? "S_sbarheight" : "sbarheight"), 0);

			if (LOBYTE(LOWORD(GetVersion())) >= 5) {
				BOOL fTransAllowed = !bSkinned || PluginConfig.m_WinVerMajor >= 6;

				ex = exold = GetWindowLongPtr(hwndDlg, GWL_EXSTYLE);
				ex = (pContainer->dwFlags & CNT_TRANSPARENCY && (!CSkin::m_skinEnabled || fTransAllowed)) ? ex | WS_EX_LAYERED : ex & ~(WS_EX_LAYERED);

				SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, ex);
				if (pContainer->dwFlags & CNT_TRANSPARENCY && fTransAllowed) {
					DWORD trans = LOWORD(pContainer->settings->dwTransparency);
					SetLayeredWindowAttributes(hwndDlg, Skin->getColorKey(), (BYTE)trans, (/* pContainer->bSkinned ? LWA_COLORKEY : */ 0) | (pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
				}
			}

			if (!CSkin::m_frameSkins)
				CheckMenuItem(hSysmenu, IDM_NOTITLE, (pContainer->dwFlags & CNT_NOTITLE) ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);

			CheckMenuItem(hSysmenu, IDM_STAYONTOP, pContainer->dwFlags & CNT_STICKY ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);
			SetWindowPos(hwndDlg, (pContainer->dwFlags & CNT_STICKY) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			if (ws != wsold) {
				RECT rc;
				GetWindowRect(hwndDlg, &rc);
				if ((ws & WS_CAPTION) != (wsold & WS_CAPTION)) {
					SetWindowPos(hwndDlg,  0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOCOPYBITS);
					RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
					if (pContainer->hwndActive != 0) {
						TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
						DM_ScrollToBottom(dat, 0, 0);
					}
				}
			}

			pContainer->dwFlags = ((pContainer->dwFlagsEx & (TCF_SBARLEFT | TCF_SBARRIGHT)) ?
				pContainer->dwFlags | CNT_SIDEBAR : pContainer->dwFlags & ~CNT_SIDEBAR);

			pContainer->SideBar->Init();

			ws = wsold = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_MSGTABS), GWL_STYLE);
			if (pContainer->dwFlags & CNT_TABSBOTTOM)
				ws |= (TCS_BOTTOM);
			else
				ws &= ~(TCS_BOTTOM);
			if ((ws & (TCS_BOTTOM | TCS_MULTILINE)) != (wsold & (TCS_BOTTOM | TCS_MULTILINE))) {
				SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_MSGTABS), GWL_STYLE, ws);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_MSGTABS), NULL, NULL, RDW_INVALIDATE);
			}

			if (pContainer->dwFlags & CNT_NOSTATUSBAR) {
				if (pContainer->hwndStatus) {
					DestroyWindow(pContainer->hwndStatus);
					pContainer->hwndStatus = 0;
					pContainer->statusBarHeight = 0;
					SendMessage(hwndDlg, DM_STATUSBARCHANGED, 0, 0);
				}
			}
			else if (pContainer->hwndStatus == 0) {
				pContainer->hwndStatus = CreateWindowEx(0, _T("TSStatusBarClass"), NULL, SBT_TOOLTIPS | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);

				if (sBarHeight && bSkinned)
					SendMessage(pContainer->hwndStatus, SB_SETMINHEIGHT, sBarHeight, 0);
			}
			if (pContainer->hwndActive != 0) {
				hContact = 0;
				SendMessage(pContainer->hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
				if (hContact)
					SendMessage(hwndDlg, DM_UPDATETITLE, hContact, 0);
			}
			SendMessage(hwndDlg, WM_SIZE, 0, 1);
			BroadCastContainer(pContainer, DM_CONFIGURETOOLBAR, 0, 1);
		}
		return 0;

		// search the first and most recent unread events in all client tabs...
		// return all information via a RECENTINFO structure (tab indices,
		// window handles and timestamps).
		case DM_QUERYRECENT:
		{
			int iItems = TabCtrl_GetItemCount(hwndTab);
			TCITEM item = {0};

			DWORD dwTimestamp, dwMostRecent = 0;

			RECENTINFO *ri = (RECENTINFO *)lParam;
			ri->iFirstIndex = ri->iMostRecent = -1;
			ri->dwFirst = ri->dwMostRecent = 0;
			ri->hwndFirst = ri->hwndMostRecent = 0;

			for (int i=0; i < iItems; i++) {
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab,  i, &item);
				SendMessage((HWND)item.lParam, DM_QUERYLASTUNREAD, 0, (LPARAM)&dwTimestamp);
				if (dwTimestamp > ri->dwMostRecent) {
					ri->dwMostRecent = dwTimestamp;
					ri->iMostRecent = i;
					ri->hwndMostRecent = (HWND)item.lParam;
					if (ri->iFirstIndex == -1) {
						ri->iFirstIndex = i;
						ri->dwFirst = dwTimestamp;
						ri->hwndFirst = (HWND)item.lParam;
					}
				}
			}
		}
		return 0;

		// search tab with either next or most recent unread message and select it
		case DM_QUERYPENDING:
		{
			RECENTINFO ri;
			SendMessage(hwndDlg, DM_QUERYRECENT, 0, (LPARAM)&ri);

			NMHDR nmhdr;
			nmhdr.code = TCN_SELCHANGE;

			if (wParam == DM_QUERY_NEXT && ri.iFirstIndex != -1) {
				TabCtrl_SetCurSel(hwndTab,  ri.iFirstIndex);
				SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nmhdr);
			}
			if (wParam == DM_QUERY_MOSTRECENT && ri.iMostRecent != -1) {
				TabCtrl_SetCurSel(hwndTab, ri.iMostRecent);
				SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nmhdr);
			}
		}
		return 0;

	case DM_SETICON:
		{
			TWindowData *dat = (TWindowData*)wParam;
			HICON hIconMsg = PluginConfig.g_IconMsgEvent;
			HICON hIconBig = (dat && dat->cache) ? LoadSkinnedProtoIconBig(dat->cache->getProto(), dat->cache->getStatus()) : 0;

			if (Win7Taskbar->haveLargeIcons()) {
				if ((HICON)lParam == PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING] || (HICON)lParam == hIconMsg) {
					Win7Taskbar->setOverlayIcon(hwndDlg, lParam);
					if (GetForegroundWindow() != hwndDlg)
						SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, lParam);
					if ((HICON)lParam == hIconMsg)
						pContainer->hIconTaskbarOverlay = hIconMsg;
					break;
				}

				if (dat) {
					if (dat->hTaskbarIcon == 0)
						dat->hTaskbarIcon = ((dat->pContainer->dwFlags & CNT_AVATARSONTASKBAR) ? Utils::iconFromAvatar(dat) : 0);
					else {
						if (!(dat->pContainer->dwFlags & CNT_AVATARSONTASKBAR)) {
							DestroyIcon(dat->hTaskbarIcon);
							dat->hTaskbarIcon = 0;
						}
					}

					if (dat->hTaskbarIcon) {
						SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)dat->hTaskbarIcon);
						SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, lParam);
						Win7Taskbar->setOverlayIcon(hwndDlg, (LPARAM)(dat->hTabIcon ? (LPARAM)dat->hTabIcon : lParam));
					}
					else {
						if (0 == hIconBig || (HICON)CALLSERVICE_NOTFOUND == hIconBig)
							SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)lParam);
						else
							SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
						SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, lParam);
						if (dat->pContainer->hIconTaskbarOverlay)
							Win7Taskbar->setOverlayIcon(hwndDlg, (LPARAM)dat->pContainer->hIconTaskbarOverlay);
						else if (Win7Taskbar->haveAlwaysGroupingMode() && fForceOverlayIcons)
							Win7Taskbar->setOverlayIcon(hwndDlg, lParam);
						else
							Win7Taskbar->clearOverlayIcon(hwndDlg);
					}
					return 0;
				}
			}
			
			// default handling (no win7 taskbar)
			if ((HICON)lParam == PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]) {              // always set typing icon, but don't save it...
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)PluginConfig.g_IconTypingEventBig);
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, lParam);
				break;
			}
			if (reinterpret_cast<HICON>(lParam) == hIconMsg)
				hIconBig = LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE);

			if (pContainer->hIcon == STICK_ICON_MSG && (HICON)lParam != hIconMsg && pContainer->dwFlags & CNT_NEED_UPDATETITLE) {
				lParam = (LPARAM)hIconMsg;
				hIconBig = LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE);
			}
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, lParam);
			if (0 != hIconBig && reinterpret_cast<HICON>(CALLSERVICE_NOTFOUND) != hIconBig)
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, LPARAM(hIconBig));
			pContainer->hIcon = (lParam == (LPARAM)hIconMsg) ? STICK_ICON_MSG : 0;
		}
		return 0;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			int cx = PluginConfig.m_smcxicon;
			int cy = PluginConfig.m_smcyicon;
			int id = LOWORD(dis->itemID);

			if (dis->hwndItem == pContainer->hwndStatus && !(pContainer->dwFlags & CNT_NOSTATUSBAR)) {
				TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);
				if (dat)
					DrawStatusIcons(dat, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
		}
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case DM_QUERYCLIENTAREA:
		{
			RECT *rc = (RECT *)lParam;
			if (rc) {
				if (!IsIconic(hwndDlg))
					GetClientRect(hwndDlg, rc);
				else
					CopyRect(rc, &pContainer->rcSaved);
				AdjustTabClientRect(pContainer, rc);
			}
		}
		return 0;

	case WM_DESTROY:
		pContainer->hwnd = 0;
		pContainer->hwndActive = 0;
		pContainer->hMenuContext = 0;
		if (pContainer->hwndStatus)
			DestroyWindow(pContainer->hwndStatus);

		// mir_free private theme...
		if (pContainer->theme.isPrivate) {
			mir_free(pContainer->ltr_templates);
			mir_free(pContainer->rtl_templates);
			mir_free(pContainer->theme.logFonts);
			mir_free(pContainer->theme.fontColors);
			mir_free(pContainer->theme.rtfFonts);
		}

		if (pContainer->hwndTip)
			DestroyWindow(pContainer->hwndTip);
		RemoveContainerFromList(pContainer);
		SM_RemoveContainer(pContainer);
		if (pContainer->cachedDC) {
			SelectObject(pContainer->cachedDC, pContainer->oldHBM);
			DeleteObject(pContainer->cachedHBM);
			DeleteDC(pContainer->cachedDC);
		}
		if (pContainer->cachedToolbarDC) {
			SelectObject(pContainer->cachedToolbarDC, pContainer->oldhbmToolbarBG);
			DeleteObject(pContainer->hbmToolbarBG);
			DeleteDC(pContainer->cachedToolbarDC);
		}
		return 0;

	case WM_NCDESTROY:
		if (pContainer) {
			delete pContainer->MenuBar;
			delete pContainer->SideBar;
			if (pContainer->settings != &PluginConfig.globalContainerSettings)
				mir_free(pContainer->settings);
			mir_free(pContainer);
		}
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;

	case WM_CLOSE:
		if (PluginConfig.m_HideOnClose && !lParam) {
			ShowWindow(hwndDlg, SW_HIDE);
			pContainer->fHidden = true;
		}
		else {
			if (TabCtrl_GetItemCount(hwndTab) > 1) {
				LRESULT res = CWarning::show(CWarning::WARN_CLOSEWINDOW, MB_YESNOCANCEL | MB_ICONQUESTION);
				if (IDNO == res || IDCANCEL == res)
					break;
			}

			// dont ask if container is empty (no tabs)
			if (lParam == 0 && TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_MSGTABS)) > 0) {
				int clients = TabCtrl_GetItemCount(hwndTab), iOpenJobs = 0;

				TCITEM item = { 0 };
				item.mask = TCIF_PARAM;
				for (int i = 0; i < clients; i++) {
					TabCtrl_GetItem(hwndTab, i, &item);
					if (item.lParam && IsWindow((HWND)item.lParam))
						SendMessage((HWND)item.lParam, DM_CHECKQUEUEFORCLOSE, 0, (LPARAM)&iOpenJobs);
				}
				if (iOpenJobs && pContainer) {
					if (pContainer->exFlags & CNT_EX_CLOSEWARN)
						return TRUE;

					pContainer->exFlags |= CNT_EX_CLOSEWARN;
					LRESULT result = SendQueue::WarnPendingJobs(iOpenJobs);
					pContainer->exFlags &= ~CNT_EX_CLOSEWARN;
					if (result == IDNO)
						return TRUE;
				}
			}

			// save geometry information to the database...
			if (!(pContainer->dwFlags & CNT_GLOBALSIZE)) {
				WINDOWPLACEMENT wp = { 0 };
				wp.length = sizeof(wp);
				if (GetWindowPlacement(hwndDlg, &wp) != 0) {
					if (pContainer->isCloned && pContainer->hContactFrom != 0) {
						TCITEM item = { 0 };
						item.mask = TCIF_PARAM;
						TabCtrl_GetItem(hwndTab, TabCtrl_GetCurSel(hwndTab), &item);

						MCONTACT hContact;
						SendMessage((HWND)item.lParam, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
						db_set_b(hContact, SRMSGMOD_T, "splitmax", (BYTE)((wp.showCmd == SW_SHOWMAXIMIZED) ? 1 : 0));

						for (int i = 0; i < TabCtrl_GetItemCount(hwndTab); i++) {
							if (TabCtrl_GetItem(hwndTab, i, &item)) {
								SendMessage((HWND)item.lParam, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
								db_set_dw(hContact, SRMSGMOD_T, "splitx", wp.rcNormalPosition.left);
								db_set_dw(hContact, SRMSGMOD_T, "splity", wp.rcNormalPosition.top);
								db_set_dw(hContact, SRMSGMOD_T, "splitwidth", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
								db_set_dw(hContact, SRMSGMOD_T, "splitheight", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
							}
						}
					}
					else {
						char szCName[40];
						mir_snprintf(szCName, 40, "%s%dx", CONTAINER_PREFIX, pContainer->iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.left);
						mir_snprintf(szCName, 40, "%s%dy", CONTAINER_PREFIX, pContainer->iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.top);
						mir_snprintf(szCName, 40, "%s%dwidth", CONTAINER_PREFIX, pContainer->iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.right - wp.rcNormalPosition.left);
						mir_snprintf(szCName, 40, "%s%dheight", CONTAINER_PREFIX, pContainer->iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

						db_set_b(0, SRMSGMOD_T, "splitmax", (BYTE)((wp.showCmd == SW_SHOWMAXIMIZED) ? 1 : 0));
					}
				}
			}

			// clear temp flags which should NEVER be saved...
			if (pContainer->isCloned && pContainer->hContactFrom != 0) {
				TCITEM item = {0};
				item.mask = TCIF_PARAM;
				pContainer->dwFlags &= ~(CNT_DEFERREDCONFIGURE | CNT_CREATE_MINIMIZED | CNT_DEFERREDSIZEREQUEST | CNT_CREATE_CLONED);
				for (int i=0; i < TabCtrl_GetItemCount(hwndTab); i++) {
					if (TabCtrl_GetItem(hwndTab, i, &item)) {
						MCONTACT hContact;
						SendMessage((HWND)item.lParam, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);

						char szCName[40];
						mir_snprintf(szCName, 40, "%s_theme", CONTAINER_PREFIX);
						if (mir_tstrlen(pContainer->szRelThemeFile) > 1) {
							if (pContainer->fPrivateThemeChanged == TRUE) {
								PathToRelativeT(pContainer->szRelThemeFile, pContainer->szAbsThemeFile, M.getDataPath());
								db_set_ts(hContact, SRMSGMOD_T, szCName, pContainer->szRelThemeFile);
								pContainer->fPrivateThemeChanged = FALSE;
							}
						}
						else {
							db_unset(hContact, SRMSGMOD_T, szCName);
							pContainer->fPrivateThemeChanged = FALSE;
						}
					}
				}
			}
			else Utils::SaveContainerSettings(pContainer, CONTAINER_PREFIX);
			DestroyWindow(hwndDlg);
		}
	}
	return FALSE;
}

// search the list of tabs and return the tab (by index) which "belongs" to the given
// hwnd. The hwnd is the handle of a message dialog childwindow. At creation,
// the dialog handle is stored in the TCITEM.lParam field, because we need
// to know the owner of the tab.
// 
// hwndTab: handle of the tab control itself.
// hwnd: handle of a message dialog.
//
// returns the tab index (zero based), -1 if no tab is found (which SHOULD not
// really happen, but who knows... ;))

int TSAPI GetTabIndexFromHWND(HWND hwndTab, HWND hwnd)
{
	int iItems = TabCtrl_GetItemCount(hwndTab);

	TCITEM item = { 0 };
	item.mask = TCIF_PARAM;
	for (int i=0; i < iItems; i++) {
		TabCtrl_GetItem(hwndTab, i, &item);
		if ((HWND)item.lParam == hwnd)
			return i;
	}
	return -1;
}

// search the list of tabs and return the tab (by index) which "belongs" to the given
// hwnd. The hwnd is the handle of a message dialog childwindow. At creation,
// the dialog handle is stored in the TCITEM.lParam field, because we need
// to know the owner of the tab.
// 
// hwndTab: handle of the tab control itself.
// hwnd: handle of a message dialog.
// 
// returns the tab index (zero based), -1 if no tab is found (which SHOULD not
// really happen, but who knows... ;))

HWND TSAPI GetHWNDFromTabIndex(HWND hwndTab, int idx)
{
	int iItems = TabCtrl_GetItemCount(hwndTab);

	TCITEM item = { 0 };
	item.mask = TCIF_PARAM;
	for (int i=0; i < iItems; i++) {
		TabCtrl_GetItem(hwndTab, i, &item);
		if (i == idx)
			return (HWND)item.lParam;
	}
	return NULL;
}

// activates the tab belonging to the given client HWND (handle of the actual
// message window.

int TSAPI ActivateTabFromHWND(HWND hwndTab, HWND hwnd)
{
	int iItem = GetTabIndexFromHWND(hwndTab, hwnd);
	if (iItem >= 0) {
		TabCtrl_SetCurSel(hwndTab, iItem);

		NMHDR nmhdr = { 0 };
		nmhdr.code = TCN_SELCHANGE;
		SendMessage(GetParent(hwndTab), WM_NOTIFY, 0, (LPARAM)&nmhdr);     // do it via a WM_NOTIFY / TCN_SELCHANGE to simulate user-activation
		return iItem;
	}
	return -1;
}

// returns the index of the tab under the mouse pointer. Used for
// context menu popup and tooltips
// pt: mouse coordinates, obtained from GetCursorPos()

int TSAPI GetTabItemFromMouse(HWND hwndTab, POINT *pt)
{
	ScreenToClient(hwndTab, pt);

	TCHITTESTINFO tch;
	tch.pt = *pt;
	tch.flags = 0;
	return TabCtrl_HitTest(hwndTab, &tch);
}

// enumerates tabs and closes all of them, but the one in dat
void TSAPI CloseOtherTabs(HWND hwndTab, TWindowData &dat)
{
	for (int idxt = 0; idxt < dat.pContainer->iChilds; ) {
		HWND otherTab = GetHWNDFromTabIndex(hwndTab, idxt);
		if (otherTab != NULL && otherTab != dat.hwnd)
			SendMessage(otherTab, WM_CLOSE, 1, 0);
		else
			++idxt;
	}
}

// cut off contact name to the option value set via Options->Tabbed messaging
// some people were requesting this, because really long contact list names
// are causing extraordinary wide tabs and these are looking ugly and wasting
// screen space.
//
// size = max length of target string

int TSAPI CutContactName(const TCHAR *oldname, TCHAR *newname, unsigned int size)
{
	int cutMax = PluginConfig.m_CutContactNameTo;

	if (mir_tstrlen(oldname) <= cutMax) {
		mir_tstrncpy(newname, oldname, size);
		newname[size - 1] = 0;
	}
	else {
		TCHAR fmt[20];
		mir_sntprintf(fmt, 18, _T("%%%d.%ds..."), cutMax, cutMax);
		mir_sntprintf(newname, size, fmt, oldname);
		newname[size - 1] = 0;
	}
	return 0;
}

// functions for handling the linked list of struct ContainerWindowData *foo

static TContainerData* TSAPI AppendToContainerList(TContainerData *pContainer)
{
	if (!pFirstContainer) {
		pFirstContainer = pContainer;
		pFirstContainer->pNext = NULL;
		return pFirstContainer;
	}
	
	TContainerData *p = pFirstContainer;
	while (p->pNext != 0)
		p = p->pNext;
	p->pNext = pContainer;
	pContainer->pNext = NULL;
	return p;
}

TContainerData* TSAPI FindContainerByName(const TCHAR *name)
{
	if (name == NULL || mir_tstrlen(name) == 0)
		return 0;

	if (M.GetByte("singlewinmode", 0)) // single window mode - always return 0 and force a new container
		return NULL;

	for (TContainerData *p = pFirstContainer; p; p = p->pNext)
		if (!_tcsncmp(p->szName, name, CONTAINER_NAMELEN))
			return p;

	// error, didn't find it.
	return NULL;
}

static TContainerData* TSAPI RemoveContainerFromList(TContainerData *pContainer)
{
	if (pContainer == pFirstContainer) {
		if (pContainer->pNext != NULL)
			pFirstContainer = pContainer->pNext;
		else
			pFirstContainer = NULL;

		if (pLastActiveContainer == pContainer)     // make sure, we don't reference this container anymore
			pLastActiveContainer = pFirstContainer;

		return pFirstContainer;
	}

	for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
		if (p->pNext == pContainer) {
			p->pNext = p->pNext->pNext;

			if (pLastActiveContainer == pContainer)     // make sure, we don't reference this container anymore
				pLastActiveContainer = pFirstContainer;

			return 0;
		}
	}
	return NULL;
}

// calls the TabCtrl_AdjustRect to calculate the "real" client area of the tab.
// also checks for the option "hide tabs when only one tab open" and adjusts
// geometry if necessary
// rc is the RECT obtained by GetClientRect(hwndTab)

void TSAPI AdjustTabClientRect(TContainerData *pContainer, RECT *rc)
{
	HWND hwndTab = GetDlgItem(pContainer->hwnd, IDC_MSGTABS);
	DWORD tBorder = pContainer->tBorder;
	DWORD dwStyle = GetWindowLongPtr(hwndTab, GWL_STYLE);

	RECT rcTab, rcTabOrig;
	GetClientRect(hwndTab, &rcTab);
	DWORD dwBottom = rcTab.bottom;
	DWORD dwTop = rcTab.top;
	if (!(pContainer->dwFlags & CNT_SIDEBAR) && (pContainer->iChilds > 1 || !(pContainer->dwFlags & CNT_HIDETABS))) {
		rcTabOrig = rcTab;
		TabCtrl_AdjustRect(hwndTab, FALSE, &rcTab);
		DWORD dwTopPad = rcTab.top - rcTabOrig.top;

		rc->left += tBorder;
		rc->right -= tBorder;

		if (dwStyle & TCS_BUTTONS) {
			if (pContainer->dwFlags & CNT_TABSBOTTOM) {
				int nCount = TabCtrl_GetItemCount(hwndTab);
				if (nCount > 0) {
					RECT rcItem;
					TabCtrl_GetItemRect(hwndTab, nCount - 1, &rcItem);
					rc->bottom = rcItem.top;
				}
			}
			else {
				rc->top += (dwTopPad - 2);
				rc->bottom = rcTabOrig.bottom;
			}
		}
		else {
			if (pContainer->dwFlags & CNT_TABSBOTTOM)
				rc->bottom = rcTab.bottom + 2;
			else {
				rc->top += (dwTopPad - 2);
				rc->bottom = rcTabOrig.bottom;
			}
		}

		rc->top += tBorder;
		rc->bottom -= tBorder;
	}
	else {
		rc->bottom = rcTab.bottom;
		rc->top = rcTab.top;
	}
	rc->right -= (pContainer->tBorder_outer_left + pContainer->tBorder_outer_right);
	if (pContainer->SideBar->isVisible())
		rc->right -= pContainer->SideBar->getWidth();
}

// retrieve the container name for the given contact handle.
// if none is assigned, return the name of the default container

int TSAPI GetContainerNameForContact(MCONTACT hContact, TCHAR *szName, int iNameLen)
{
	// single window mode using cloned (temporary) containers
	if (M.GetByte("singlewinmode", 0)) {
		_tcsncpy_s(szName, iNameLen, _T("Message Session"), _TRUNCATE);
		return 0;
	}

	// use clist group names for containers...
	if (M.GetByte("useclistgroups", 0)) {
		ptrT tszGroup(db_get_tsa(hContact, "CList", "Group"));
		if (tszGroup == NULL) {
			_tcsncpy_s(szName, iNameLen, _T("default"), _TRUNCATE);
			return 0;
		}

		_tcsncpy_s(szName, iNameLen, tszGroup, _TRUNCATE);
		return 1;
	}

	ptrT tszContainerName(db_get_tsa(hContact, SRMSGMOD_T, CONTAINER_SUBKEY));
	if (tszContainerName == NULL) {
		_tcsncpy_s(szName, iNameLen, _T("default"), _TRUNCATE);
		return 0;
	}

	_tcsncpy_s(szName, iNameLen, tszContainerName, _TRUNCATE);
	return 1;
}

void TSAPI DeleteContainer(int iIndex)
{
	char szIndex[10];
	itoa(iIndex, szIndex, 10);
	ptrT tszContainerName(db_get_tsa(NULL, CONTAINER_KEY, szIndex));
	if (tszContainerName == NULL)
		return;

	db_set_ts(NULL, CONTAINER_KEY, szIndex, _T("**mir_free**"));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ptrT tszValue(db_get_tsa(hContact, SRMSGMOD_T, CONTAINER_SUBKEY));
		if (!mir_tstrcmp(tszValue, tszContainerName))
			db_unset(hContact, SRMSGMOD_T, CONTAINER_SUBKEY);
	}

	char szSetting[CONTAINER_NAMELEN + 30];
	mir_snprintf(szSetting, CONTAINER_NAMELEN + 15, "%s%d_Flags", CONTAINER_PREFIX, iIndex);
	db_unset(NULL, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, CONTAINER_NAMELEN + 15, "%s%d_Trans", CONTAINER_PREFIX, iIndex);
	db_unset(NULL, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, CONTAINER_NAMELEN + 15, "%s%dwidth", CONTAINER_PREFIX, iIndex);
	db_unset(NULL, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, CONTAINER_NAMELEN + 15, "%s%dheight", CONTAINER_PREFIX, iIndex);
	db_unset(NULL, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, CONTAINER_NAMELEN + 15, "%s%dx", CONTAINER_PREFIX, iIndex);
	db_unset(NULL, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, CONTAINER_NAMELEN + 15, "%s%dy", CONTAINER_PREFIX, iIndex);
	db_unset(NULL, SRMSGMOD_T, szSetting);
}

void TSAPI RenameContainer(int iIndex, const TCHAR *szNew)
{
	if (mir_tstrlen(szNew) == 0)
		return;
	
	char szIndex[10];
	itoa(iIndex, szIndex, 10);
	ptrT tszContainerName(db_get_tsa(NULL, CONTAINER_KEY, szIndex));
	if (tszContainerName == NULL)
		return;

	db_set_ts(NULL, CONTAINER_KEY, szIndex, szNew);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ptrT tszValue(db_get_tsa(hContact, SRMSGMOD_T, CONTAINER_SUBKEY));
		if (!mir_tstrcmp(tszValue, tszContainerName))
			db_set_ts(hContact, SRMSGMOD_T, CONTAINER_SUBKEY, szNew);
	}
}

HMENU TSAPI BuildContainerMenu()
{
	if (PluginConfig.g_hMenuContainer != 0) {
		HMENU submenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);
		RemoveMenu(submenu, 6, MF_BYPOSITION);
		DestroyMenu(PluginConfig.g_hMenuContainer);
		PluginConfig.g_hMenuContainer = 0;
	}

	// no container attach menu, if we are using the "clist group mode"
	if (M.GetByte("useclistgroups", 0) || M.GetByte("singlewinmode", 0))
		return NULL;

	HMENU hMenu = CreateMenu();
	int i = 0;
	while (true) {
		char szCounter[10];
		itoa(i, szCounter, 10);
		ptrT tszName(db_get_tsa(NULL, CONTAINER_KEY, szCounter));
		if (tszName == NULL)
			break;

		if (_tcsncmp(tszName, _T("**mir_free**"), CONTAINER_NAMELEN))
			AppendMenu(hMenu, MF_STRING, IDM_CONTAINERMENU + i, !_tcscmp(tszName, _T("default")) ? TranslateT("Default container") : tszName);
		i++;
	}		

	InsertMenu(PluginConfig.g_hMenuContext, ID_TABMENU_ATTACHTOCONTAINER, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)hMenu, TranslateT("Attach to"));
	PluginConfig.g_hMenuContainer = hMenu;
	return hMenu;
}

// flashes the container
// iMode != 0: turn on flashing
// iMode == 0: turn off flashing

void TSAPI FlashContainer(TContainerData *pContainer, int iMode, int iCount)
{
	if (pContainer->dwFlags & CNT_NOFLASH)                  // container should never flash
		return;

	FLASHWINFO fwi;
	fwi.cbSize = sizeof(fwi);
	fwi.uCount = 0;

	if (iMode) {
		fwi.dwFlags = FLASHW_ALL;
		if (pContainer->dwFlags & CNT_FLASHALWAYS)
			fwi.dwFlags |= FLASHW_TIMER;
		else
			fwi.uCount = (iCount == 0) ? M.GetByte("nrflash", 4) : iCount;
		fwi.dwTimeout = M.GetDword("flashinterval", 1000);

	}
	else fwi.dwFlags = FLASHW_STOP;

	fwi.hwnd = pContainer->hwnd;
	pContainer->dwFlashingStarted = GetTickCount();
	FlashWindowEx(&fwi);
}

void TSAPI ReflashContainer(TContainerData *pContainer)
{
	DWORD dwStartTime = pContainer->dwFlashingStarted;

	if (GetForegroundWindow() == pContainer->hwnd || GetActiveWindow() == pContainer->hwnd)       // dont care about active windows
		return;

	if (pContainer->dwFlags & CNT_NOFLASH || pContainer->dwFlashingStarted == 0)
		return;                                                                                 // dont care about containers which should never flash

	if (pContainer->dwFlags & CNT_FLASHALWAYS)
		FlashContainer(pContainer, 1, 0);
	else {
		// recalc the remaining flashes
		DWORD dwInterval = M.GetDword("flashinterval", 1000);
		int iFlashesElapsed = (GetTickCount() - dwStartTime) / dwInterval;
		DWORD dwFlashesDesired = M.GetByte("nrflash", 4);
		if (iFlashesElapsed < (int)dwFlashesDesired)
			FlashContainer(pContainer, 1, dwFlashesDesired - iFlashesElapsed);
		else {
			BOOL isFlashed = FlashWindow(pContainer->hwnd, TRUE);
			if (!isFlashed)
				FlashWindow(pContainer->hwnd, TRUE);
		}
	}
	pContainer->dwFlashingStarted = dwStartTime;
}

// broadcasts a message to all child windows (tabs/sessions)

void TSAPI BroadCastContainer(const TContainerData *pContainer, UINT message, WPARAM wParam, LPARAM lParam, BYTE bType)
{
	HWND hwndTab = GetDlgItem(pContainer->hwnd, IDC_MSGTABS);

	TCITEM item = { 0 };
	item.mask = TCIF_PARAM;

	int nCount = TabCtrl_GetItemCount(hwndTab);
	for (int i = 0; i < nCount; i++) {
		TabCtrl_GetItem(hwndTab, i, &item);
		if (IsWindow((HWND)item.lParam)) {
			if (bType == SESSIONTYPE_ANY)
				SendMessage((HWND)item.lParam, message, wParam, lParam);
			else {
				TWindowData *dat = (TWindowData*)GetWindowLongPtr((HWND)item.lParam, GWLP_USERDATA);
				if (dat && dat->bType == bType)
					SendMessage((HWND)item.lParam, message, wParam, lParam);
			}
		}
	}
}

void TSAPI CloseAllContainers()
{
	BOOL fOldHideSetting = PluginConfig.m_HideOnClose;

	while (pFirstContainer != NULL) {
		if (!IsWindow(pFirstContainer->hwnd))
			pFirstContainer = pFirstContainer->pNext;
		else {
			PluginConfig.m_HideOnClose = FALSE;
			::SendMessage(pFirstContainer->hwnd, WM_CLOSE, 0, 1);
		}
	}

	PluginConfig.m_HideOnClose = fOldHideSetting;
}
