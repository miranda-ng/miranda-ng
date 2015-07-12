/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
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
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: clistevents.cpp 134 2010-10-01 10:23:10Z silvercircle $
 *
 */

#include <commonheaders.h>

static HWND hwndEventFrame = 0;

extern FRAMEWND *wndFrameEventArea;

extern HPEN g_hPenCLUIFrames;

HWND g_hwndEventArea = 0;

struct CListEvent {
	int imlIconIndex;
	int flashesDone;
	CLISTEVENT cle;

	int menuId;
	int imlIconOverlayIndex;
};

struct CListImlIcon {
	int index;
	HICON hIcon;
};

static int iconsOn;

HANDLE hNotifyFrame = (HANDLE) - 1;

CListEvent* CLC::fnCreateEvent()
{
	CListEvent *p = reinterpret_cast<CListEvent *>(mir_alloc(sizeof(struct CListEvent)));
	if(p)
		ZeroMemory(p, sizeof(struct CListEvent));

	return p;
}

void CLUI::hideShowNotifyFrame()
{
	int dwVisible = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hNotifyFrame), 0) & F_VISIBLE;
	int desired;

	if(cfg::dat.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY)
		desired = cfg::dat.notifyActive ? TRUE : FALSE;
	else
		desired = dwVisible;

	if(desired) {
		if(!dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)hNotifyFrame, 0);
	} else {
		if(dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)hNotifyFrame, 0);
	}
}

static CLISTEVENT* MyGetEvent(int iSelection)
{
	int i;

	for(i = 0; i < pcli->events.count; i++) {
		struct CListEvent* p = pcli->events.items[i];
		if(p->menuId == iSelection)
			return &p->cle;
	}
	return NULL;
}

/**
 * paint content of the event area frame
 * @param hDC			device context to use
 * @param rc			target rectangle (client coordinates)
 * @param hTheme		UxTheme handle to use for drawing text
 */
void PaintNotifyArea(HDC hDC, RECT *rc, HANDLE hTheme)
{
	int iCount;
	static int ev_lastIcon = 0;

	rc->left += 26;             // button
	iCount = GetMenuItemCount(cfg::dat.hMenuNotify);
	if(cfg::dat.hUpdateContact != 0) {
		wchar_t *szName = pcli->pfnGetContactDisplayName(cfg::dat.hUpdateContact, 0);
		int iIcon = CallService(MS_CLIST_GETCONTACTICON, (WPARAM) cfg::dat.hUpdateContact, 0);

		ImageList_DrawEx(CLC::hClistImages, iIcon, hDC, rc->left, (rc->bottom + rc->top - CYSMICON) / 2, CXSMICON, CYSMICON, CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc->left += 18;
		Gfx::renderText(hDC, hTheme, szName, rc, DT_VCENTER | DT_SINGLELINE, 0);
		ImageList_DrawEx(CLC::hClistImages, (int)cfg::dat.hIconNotify, hDC, 4, (rc->bottom + rc->top - 16) / 2, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
		ev_lastIcon = cfg::dat.hIconNotify;
	} else if(iCount > 0) {
		MENUITEMINFO mii = {0};
		struct NotifyMenuItemExData *nmi;
		wchar_t *szName;
		int iIcon;

		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(cfg::dat.hMenuNotify, iCount - 1, TRUE, &mii);
		nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
		szName = pcli->pfnGetContactDisplayName(nmi->hContact, 0);
		iIcon = CallService(MS_CLIST_GETCONTACTICON, (WPARAM) nmi->hContact, 0);
		ImageList_DrawEx(CLC::hClistImages, iIcon, hDC, rc->left, (rc->bottom + rc->top - CYSMICON) / 2, CXSMICON, CYSMICON, CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc->left += 18;
		ImageList_DrawEx(CLC::hClistImages, nmi->iIcon, hDC, 4, (rc->bottom + rc->top) / 2 - 8, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
		Gfx::renderText(hDC, hTheme, szName, rc, DT_VCENTER | DT_SINGLELINE, 0);
		ev_lastIcon = (int)nmi->hIcon;
	} else
		Gfx::renderText(hDC, hTheme, cfg::dat.szNoEvents, rc, DT_VCENTER | DT_SINGLELINE, 0);
}

/**
 * this handles the subclassing for the event area frame.
 */
LRESULT CALLBACK CLUI::eventAreaWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_CREATE: {
			hwndEventFrame = hwnd;
			return FALSE;
		}
		case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *lpi = (LPMEASUREITEMSTRUCT) lParam;
			MENUITEMINFOA mii = {0};

			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA | MIIM_ID;
			if(GetMenuItemInfoA(cfg::dat.hMenuNotify, lpi->itemID, FALSE, &mii) != 0) {
				if(mii.dwItemData == lpi->itemData) {
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
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;

			if(dis->hwndItem == (HWND) cfg::dat.hMenuNotify) {
				MENUITEMINFOA mii = {0};

				struct NotifyMenuItemExData *nmi = 0;
				int iIcon;
				HICON hIcon;

				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_DATA;
				if(GetMenuItemInfoA(cfg::dat.hMenuNotify, (UINT) dis->itemID, FALSE, &mii) != 0) {
					nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
					if(nmi) {
						iIcon = CallService(MS_CLIST_GETCONTACTICON, (WPARAM) nmi->hContact, 0);
						hIcon = ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL);
						pcli->pfnDrawMenuItem(dis, hIcon, nmi->hIcon);
						return TRUE;
					}
				}
			}
			break;
		}
		case WM_LBUTTONUP:
			if(cfg::dat.bEventAreaEnabled)
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_NOTIFYBUTTON, 0), 0);
			break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_NOTIFYBUTTON) {
				int iSelection;
				MENUITEMINFO mii = {0};
				POINT pt;
				struct NotifyMenuItemExData *nmi = 0;
				int iCount = GetMenuItemCount(cfg::dat.hMenuNotify);
				BOOL result;

				GetCursorPos(&pt);
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_DATA;
				if(iCount > 1)
					iSelection = TrackPopupMenu(cfg::dat.hMenuNotify, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
				else
					iSelection = GetMenuItemID(cfg::dat.hMenuNotify, 0);
				result = GetMenuItemInfo(cfg::dat.hMenuNotify, (UINT) iSelection, FALSE, &mii);
				if(result != 0) {
					nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
					if(nmi) {
						CLISTEVENT *cle = MyGetEvent(iSelection);
						if(cle) {
							CLISTEVENT *cle1 = NULL;
							CallService(cle->pszService, (WPARAM) NULL, (LPARAM) cle);
							// re-obtain the pointer, it may already be invalid/point to another event if the
							// event we're interested in was removed by the service (nasty one...)
							cle1 = MyGetEvent(iSelection);
							if(cle1 != NULL)
								CallService(MS_CLIST_REMOVEEVENT, (WPARAM) cle->hContact, (LPARAM) cle->hDbEvent);
						}
					}
				}
				break;
			}
			break;
		case WM_ERASEBKGND:
			return TRUE;

		case WM_LBUTTONDOWN: {
			POINT ptMouse, pt;
			RECT rcClient;

			if((GetKeyState(VK_CONTROL) & 0x8000) && wndFrameEventArea->floating) {
				GetCursorPos(&ptMouse);
				pt = ptMouse;
				ScreenToClient(hwnd, &ptMouse);
				GetClientRect(hwnd, &rcClient);
				return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
			}
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT 	ps;
			RECT 			rc, rcClient;
			HDC 			hdc = BeginPaint(hwnd, &ps);
			LONG 			dwLeft;
			HDC 			hdcMem;
			HANDLE			hbp = 0;
			TStatusItem*	item;
			HFONT 			hFontOld = 0;
			HANDLE 			hTheme = Api::pfnOpenThemeData(hwnd, L"BUTTON");

			GetClientRect(hwnd, &rc);
			rcClient = rc;
			if(wndFrameEventArea->floating) {
				hdcMem = hdc;
				FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_3DFACE));
			} else
				INIT_PAINT(hdc, rcClient, hdcMem);

			SetBkMode(hdcMem, TRANSPARENT);

			if(cfg::clcdat)
				hFontOld = reinterpret_cast<HFONT>(SelectObject(hdcMem, cfg::clcdat->fontInfo[FONTID_EVENTAREA].hFont));

			Gfx::drawBGFromSurface(hwnd, rc, hdcMem);
			item = &Skin::statusItems[ID_EXTBKEVTAREA];
			if(!item->IGNORED) {
				rc.top += item->MARGIN_TOP;
				rc.bottom -= item->MARGIN_BOTTOM;
				rc.left += item->MARGIN_LEFT;
				rc.right -= item->MARGIN_RIGHT;

				Gfx::renderSkinItem(hdcMem, &rc, item->imageItem);
				Gfx::setTextColor(item->TEXTCOLOR);
			}

			dwLeft = rc.left;

			PaintNotifyArea(hdcMem, &rc, hTheme);
			if(hFontOld)
				SelectObject(hdcMem, hFontOld);

			if(hbp)
				FINALIZE_PAINT(hbp, &rcClient, 0);

			ps.fErase = FALSE;
			EndPaint(hwnd, &ps);
			if(hTheme)
				Api::pfnCloseThemeData(hTheme);
			return 0;
		}
		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

CListEvent* CLC::AddEvent(CLISTEVENT *cle)
{
	CListEvent* p = coreCli.pfnAddEvent(cle);
	if(p == NULL)
		return NULL;

	if(p->cle.hContact != 0 && p->cle.hDbEvent != (MEVENT) 1 && !(p->cle.flags & CLEF_ONLYAFEW)) {
		int j;
		struct NotifyMenuItemExData *nmi = 0;
		char *szProto;
		wchar_t *szName;
		MENUITEMINFOW mii = {0};
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA | MIIM_BITMAP | MIIM_ID;
		if(p->cle.pszService && !strncmp("SRMsg/ReadMessage", p->cle.pszService, 17)) {
			// dup check only for msg events
			for(j = 0; j < GetMenuItemCount(cfg::dat.hMenuNotify); j++) {
				if(GetMenuItemInfo(cfg::dat.hMenuNotify, j, TRUE, &mii) != 0) {
					nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
					if(nmi != 0 && (HANDLE) nmi->hContact == (HANDLE) p->cle.hContact && nmi->iIcon == p->imlIconIndex)
						return p;
				}
			}
		}

		szProto = GetContactProto(p->cle.hContact);
		szName = pcli->pfnGetContactDisplayName(p->cle.hContact, 0);
		if(szProto && szName) {
			nmi = (struct NotifyMenuItemExData *) malloc(sizeof(struct NotifyMenuItemExData));
			if(nmi) {
				wchar_t szBuffer[128];
				wchar_t* szStatus = pcli->pfnGetStatusModeDescription(cfg::getWord(p->cle.hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
				wchar_t szwProto[64];
				MultiByteToWideChar(CP_ACP, 0, szProto, -1, szwProto, 64);
				szwProto[63] = 0;
				_snwprintf(szBuffer, _countof(szBuffer), L"%s: %s (%s)", szwProto, szName, szStatus);
				szBuffer[127] = 0;
				AppendMenu(cfg::dat.hMenuNotify, MF_BYCOMMAND | MF_STRING, cfg::dat.wNextMenuID, szBuffer);
				mii.hbmpItem = HBMMENU_CALLBACK;
				nmi->hContact = p->cle.hContact;
				nmi->iIcon = p->imlIconIndex;
				nmi->hIcon = p->cle.hIcon;
				nmi->hDbEvent = p->cle.hDbEvent;
				mii.dwItemData = (ULONG_PTR) nmi;
				mii.wID = cfg::dat.wNextMenuID;
				SetMenuItemInfo(cfg::dat.hMenuNotify, cfg::dat.wNextMenuID, FALSE, &mii);
				p-> menuId = cfg::dat.wNextMenuID;
				cfg::dat.wNextMenuID++;
				if(cfg::dat.wNextMenuID > 0x7fff)
					cfg::dat.wNextMenuID = 1;
				cfg::dat.hIconNotify = p->imlIconIndex;
			}
		}
	} else if(p->cle.hContact != 0 && (p->cle.flags & CLEF_ONLYAFEW)) {
		cfg::dat.hIconNotify = p->imlIconIndex;
		cfg::dat.hUpdateContact = p->cle.hContact;
	}
	if(cfg::dat.dwFlags & CLUI_STICKYEVENTS) {
		HANDLE hItem = (HANDLE) SendMessage(pcli->hwndContactTree, CLM_FINDCONTACT, (WPARAM) p->cle.hContact, 0);
		if(hItem) {
			SendMessage(pcli->hwndContactTree, CLM_SETSTICKY, (WPARAM) hItem, 1);
			pcli->pfnClcBroadcast(INTM_PROTOCHANGED, (WPARAM) p->cle.hContact, 0);
		}
	}
	if(pcli->events.count > 0) {
		cfg::dat.bEventAreaEnabled = TRUE;
		if(cfg::dat.notifyActive == 0) {
			cfg::dat.notifyActive = 1;
			CLUI::hideShowNotifyFrame();
		}
	}
	InvalidateRect(hwndEventFrame, NULL, FALSE);
	if(cfg::dat.bUseFloater & CLUI_USE_FLOATER && cfg::dat.bUseFloater & CLUI_FLOATER_EVENTS)
		SFL_Update(0, 0, 0, NULL, FALSE);

	return p;
}

// Removes an event from the contact list's queue
// wParam=(WPARAM)(HANDLE)hContact
// lParam=(LPARAM)(HANDLE)hDbEvent
// Returns 0 if the event was successfully removed, or nonzero if the event was not found
int CLC::RemoveEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	HANDLE hItem;
	int i;
	BOOL bUnstick = TRUE;

	// Find the event that should be removed
	for(i = 0; i < pcli->events.count; i++) {
		if((pcli->events.items[i]->cle.hContact == hContact) && (pcli->events.items[i]->cle.hDbEvent == hDbEvent)) {
			break;
		}
	}

	// Event was not found
	if(i == pcli->events.count)
		return 1;

	// remove event from the notify menu
	if(pcli->events.items[i]->menuId > 0) {
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		if(GetMenuItemInfo(cfg::dat.hMenuNotify, pcli->events.items[i]->menuId, FALSE, &mii) != 0) {
			struct NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
			if(nmi && nmi->hContact == hContact && nmi->hDbEvent == hDbEvent) {
				free(nmi);
				DeleteMenu(cfg::dat.hMenuNotify, pcli->events.items[i]->menuId, MF_BYCOMMAND);
			}
		}
	}

	coreCli.pfnRemoveEvent(hContact, hDbEvent);

	if(pcli->events.count == 0) {
		cfg::dat.bEventAreaEnabled = FALSE;
		if(cfg::dat.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY) {
			cfg::dat.notifyActive = 0;
			CLUI::hideShowNotifyFrame();
		}
	}

	if(bUnstick) {
		// clear "sticky" (sort) status

		hItem = (HANDLE) SendMessage(pcli->hwndContactTree, CLM_FINDCONTACT, (WPARAM)hContact, 0);
		if(hItem) {
			SendMessage(pcli->hwndContactTree, CLM_SETSTICKY, (WPARAM) hItem, 0);
			pcli->pfnClcBroadcast(INTM_PROTOCHANGED, (WPARAM)hContact, 0);
		}
	}

	if(hContact == cfg::dat.hUpdateContact || (INT_PTR)hDbEvent == 1)
		cfg::dat.hUpdateContact = 0;

	if(cfg::dat.notifyActive) {
		InvalidateRect(hwndEventFrame, NULL, FALSE);
		if(cfg::dat.bUseFloater & CLUI_USE_FLOATER && cfg::dat.bUseFloater & CLUI_FLOATER_EVENTS)
			SFL_Update(0, 0, 0, NULL, FALSE);
	}

	return 0;
}
