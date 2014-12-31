/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include <commonheaders.h>
#include "../cluiframes/cluiframes.h"

static HWND hwndEventFrame = 0;
HFONT __fastcall ChangeToFont(HDC hdc, struct ClcData *dat, int id, int *fontHeight);

extern FRAMEWND *wndFrameEventArea;

extern HPEN g_hPenCLUIFrames;

HWND g_hwndEventArea = 0;

struct CListEvent
{
	int imlIconIndex;
	int flashesDone;
	CLISTEVENT cle;

	int menuId;
	int imlIconOverlayIndex;
};

struct CListImlIcon
{
	int index;
	HICON hIcon;
};

static int iconsOn;
extern HIMAGELIST hCListImages;

HANDLE hNotifyFrame = (HANDLE)-1;

CListEvent* fnCreateEvent( void )
{
	CListEvent *p = reinterpret_cast<CListEvent *>(mir_alloc(sizeof(CListEvent)));
	if (p)
		memset(p, 0, sizeof(CListEvent));

	return p;
}

void HideShowNotifyFrame()
{
	int dwVisible = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hNotifyFrame), 0) & F_VISIBLE;
    int desired;

    if (cfg::dat.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY)
        desired = cfg::dat.notifyActive ? TRUE : FALSE;
    else
        desired = dwVisible;

    if (desired) {
		if ( !dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)hNotifyFrame, 0);
	}
	else {
		if (dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)hNotifyFrame, 0);
	}
}

static CLISTEVENT* MyGetEvent(int iSelection)
{
	int i;

	for (i = 0; i < pcli->events.count; i++) {
		CListEvent* p = pcli->events.items[i];
		if (p->menuId == iSelection)
			return &p->cle;
	}
	return NULL;
}

LRESULT CALLBACK EventAreaWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
	case WM_CREATE:
		hwndEventFrame = hwnd;
		return FALSE;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *lpi = (LPMEASUREITEMSTRUCT) lParam;
			MENUITEMINFOA mii = {0};

			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA | MIIM_ID;
			if (GetMenuItemInfoA(cfg::dat.hMenuNotify, lpi->itemID, FALSE, &mii) != 0) {
				if (mii.dwItemData == lpi->itemData) {
					lpi->itemWidth = 8 + 16;
					lpi->itemHeight = 0;
					return TRUE;
				}
			}
			break;
		}
    case WM_NCCALCSIZE:
        return FrameNCCalcSize(hwnd, DefWindowProc, wParam, lParam,
			  wndFrameEventArea ? wndFrameEventArea->TitleBar.ShowTitleBar : 0);
    case WM_NCPAINT:
        return FrameNCPaint(hwnd, DefWindowProc, wParam, lParam,
			  wndFrameEventArea ? wndFrameEventArea->TitleBar.ShowTitleBar : 0);
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;

			if (dis->hwndItem == (HWND) cfg::dat.hMenuNotify) {
				MENUITEMINFOA mii = {0};

				struct NotifyMenuItemExData *nmi = 0;
				int iIcon;
				HICON hIcon;

				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_DATA;
				if (GetMenuItemInfoA(cfg::dat.hMenuNotify, (UINT) dis->itemID, FALSE, &mii) != 0) {
					nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
					if (nmi) {
						iIcon = pcli->pfnGetContactIcon(nmi->hContact);
						hIcon = ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
						pcli->pfnDrawMenuItem(dis, hIcon, nmi->hIcon);
						return TRUE;
					}
				}
			}
			break;
		}
	case WM_LBUTTONUP:
		if (cfg::dat.bEventAreaEnabled)
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_NOTIFYBUTTON, 0), 0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTIFYBUTTON) {
			int iSelection;
			MENUITEMINFO mii = {0};
			POINT pt;
			struct NotifyMenuItemExData *nmi = 0;
			int iCount = GetMenuItemCount(cfg::dat.hMenuNotify);
			BOOL result;

			GetCursorPos(&pt);
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA;
			if (iCount > 1)
				iSelection = TrackPopupMenu(cfg::dat.hMenuNotify, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
			else
				iSelection = GetMenuItemID(cfg::dat.hMenuNotify, 0);
			result = GetMenuItemInfo(cfg::dat.hMenuNotify, (UINT) iSelection, FALSE, &mii);
			if (result != 0) {
				nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
				if (nmi) {
					CLISTEVENT *cle = MyGetEvent(iSelection);
					if (cle) {
						CLISTEVENT *cle1 = NULL;
						CallService(cle->pszService, (WPARAM) NULL, (LPARAM) cle);
						// re-obtain the pointer, it may already be invalid/point to another event if the
						// event we're interested in was removed by the service (nasty one...)
						cle1 = MyGetEvent(iSelection);
						if (cle1 != NULL)
							CallService(MS_CLIST_REMOVEEVENT, (WPARAM) cle->hContact, (LPARAM) cle->hDbEvent);
					}
				}
			}
			break;
		}
		break;
	case WM_ERASEBKGND:
		return TRUE;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			RECT rc, rcClient;
			GetClientRect(hwnd, &rc);
			rcClient = rc;

			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			HBITMAP hbmold = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hbm));
			SetBkMode(hdcMem, TRANSPARENT);

			HFONT hFontOld = 0;
			if (cfg::clcdat) {
				int height;
				hFontOld = ChangeToFont(hdcMem, cfg::clcdat, FONTID_EVENTAREA, &height);
			}

			if (cfg::dat.bWallpaperMode)
				SkinDrawBg(hwnd, hdcMem);

			StatusItems_t *item = arStatusItems[ID_EXTBKEVTAREA - ID_STATUS_OFFLINE];
			if (item->IGNORED)
				FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_3DFACE));
			else {
				rc.top += item->MARGIN_TOP; rc.bottom -= item->MARGIN_BOTTOM;
				rc.left += item->MARGIN_LEFT; rc.right -= item->MARGIN_RIGHT;

				DrawAlpha(hdcMem, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT,
					item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
				SetTextColor(hdcMem, item->TEXTCOLOR);
			}

			LONG dwLeft = rc.left;

			PaintNotifyArea(hdcMem, &rc);
			if (cfg::dat.dwFlags & CLUI_FRAME_EVENTAREASUNKEN) {
				rc.left = dwLeft;
				InflateRect(&rc, -2, -2);
				DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
			}
			BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, hbmold);
			if (hFontOld)
				SelectObject(hdcMem, hFontOld);
			DeleteObject(hbm);
			DeleteDC(hdcMem);
			ps.fErase = FALSE;
			EndPaint(hwnd, &ps);
			return 0;
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

CListEvent* AddEvent(CLISTEVENT *cle)
{
	CListEvent *p = coreCli.pfnAddEvent(cle);
	if (p == NULL)
		return NULL;

	if (p->cle.hContact != 0 && p->cle.hDbEvent != (HANDLE)1 && !(p->cle.flags & CLEF_ONLYAFEW)) {
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_DATA | MIIM_BITMAP | MIIM_ID;
		if (p->cle.pszService && !strncmp("SRMsg/ReadMessage", p->cle.pszService, 17)) {
			// dup check only for msg events
			for (int j = 0; j < GetMenuItemCount(cfg::dat.hMenuNotify); j++) {
				if (GetMenuItemInfo(cfg::dat.hMenuNotify, j, TRUE, &mii) != 0) {
					NotifyMenuItemExData *nmi = (NotifyMenuItemExData*) mii.dwItemData;
					if (nmi != 0 && (HANDLE)nmi->hContact == (HANDLE)p->cle.hContact && nmi->iIcon == p->imlIconIndex)
						return p;
		}	}	}

		char *szProto = GetContactProto(p->cle.hContact);
		TCHAR *szName = pcli->pfnGetContactDisplayName(p->cle.hContact, 0);
		if (szProto && szName) {
			NotifyMenuItemExData *nmi = (NotifyMenuItemExData*) malloc(sizeof(NotifyMenuItemExData));
			if (nmi) {
				TCHAR szBuffer[128];
				TCHAR* szStatus = pcli->pfnGetStatusModeDescription(cfg::getWord(p->cle.hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);

				TCHAR szwProto[64];
				MultiByteToWideChar(CP_ACP, 0, szProto, -1, szwProto, 64);
				szwProto[63] = 0;
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s: %s (%s)"), szwProto, szName, szStatus);

				szBuffer[127] = 0;
				AppendMenu(cfg::dat.hMenuNotify, MF_BYCOMMAND | MF_STRING, cfg::dat.wNextMenuID, szBuffer);
				mii.hbmpItem = HBMMENU_CALLBACK;
				nmi->hContact = p->cle.hContact;
				nmi->iIcon = p->imlIconIndex;
				nmi->hIcon = p->cle.hIcon;
				nmi->hDbEvent = p->cle.hDbEvent;
				mii.dwItemData = (ULONG_PTR)nmi;
				mii.wID = cfg::dat.wNextMenuID;
				SetMenuItemInfo(cfg::dat.hMenuNotify, cfg::dat.wNextMenuID, FALSE, &mii);
				p->menuId = cfg::dat.wNextMenuID;
				cfg::dat.wNextMenuID++;
				if (cfg::dat.wNextMenuID > 0x7fff)
					cfg::dat.wNextMenuID = 1;
				cfg::dat.hIconNotify = p->imlIconIndex;
			}
		}
	}
	else if (p->cle.hContact != 0 && (p->cle.flags & CLEF_ONLYAFEW)) {
		cfg::dat.hIconNotify = p->imlIconIndex;
		cfg::dat.hUpdateContact = p->cle.hContact;
	}

	if (cfg::dat.dwFlags & CLUI_STICKYEVENTS) {
		HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree, CLM_FINDCONTACT, (WPARAM) p->cle.hContact, 0);
		if (hItem) {
			SendMessage(pcli->hwndContactTree, CLM_SETSTICKY, (WPARAM) hItem, 1);
			pcli->pfnClcBroadcast(INTM_PROTOCHANGED, (WPARAM) p->cle.hContact, 0);
		}
	}

	if (pcli->events.count > 0) {
		cfg::dat.bEventAreaEnabled = TRUE;
		if (cfg::dat.notifyActive == 0) {
			cfg::dat.notifyActive = 1;
			HideShowNotifyFrame();
		}
	}
	InvalidateRect(hwndEventFrame, NULL, FALSE);

	return p;
}

// Removes an event from the contact list's queue
// wParam=(MCONTACT)hContact
// lParam=(LPARAM)(HANDLE)hDbEvent
// Returns 0 if the event was successfully removed, or nonzero if the event was not found
int RemoveEvent(MCONTACT hContact, HANDLE hDbEvent)
{
	// Find the event that should be removed
	int i;
	for (i = 0; i < pcli->events.count; i++)
		if ((pcli->events.items[i]->cle.hContact == hContact) && (pcli->events.items[i]->cle.hDbEvent == hDbEvent))
			break;

	// Event was not found
	if (i == pcli->events.count)
		return 1;

	// remove event from the notify menu
	if (pcli->events.items[i]->menuId > 0) {
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		if (GetMenuItemInfo(cfg::dat.hMenuNotify, pcli->events.items[i]->menuId, FALSE, &mii) != 0) {
			struct NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
			if (nmi && nmi->hContact == hContact && nmi->hDbEvent == hDbEvent) {
				free(nmi);
				DeleteMenu(cfg::dat.hMenuNotify, pcli->events.items[i]->menuId, MF_BYCOMMAND);
	}	}	}

	coreCli.pfnRemoveEvent(hContact, hDbEvent);

	if (pcli->events.count == 0) {
		cfg::dat.bEventAreaEnabled = FALSE;
		if (cfg::dat.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY) {
			cfg::dat.notifyActive = 0;
			HideShowNotifyFrame();
	}	}

	// clear "sticky" (sort) status
	HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree, CLM_FINDCONTACT, hContact, 0);
	if (hItem) {
		SendMessage(pcli->hwndContactTree, CLM_SETSTICKY, (WPARAM)hItem, 0);
		pcli->pfnClcBroadcast(INTM_PROTOCHANGED, hContact, 0);
	}

	if (hContact == cfg::dat.hUpdateContact || (INT_PTR)hDbEvent == 1)
		cfg::dat.hUpdateContact = 0;

	if (cfg::dat.notifyActive)
		InvalidateRect(hwndEventFrame, NULL, FALSE);

	return 0;
}
