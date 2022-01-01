/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "modern_clcpaint.h"

/**************************************************/
/*   Notify Event Area Frame implementation       */
/**************************************************/

/* Declarations */
static int hNotifyFrame = 0;

/**************************************************/

struct CListImlIcon
{
	int index;
	HICON hIcon;
};

static struct CListImlIcon *imlIcon;
static int imlIconCount;
static UINT flashTimerId;
static int iconsOn;


struct NotifyMenuItemExData
{
	MCONTACT hContact;
	int iIcon;              // icon index in the image list
	HICON hIcon;            // corresponding icon handle
	MEVENT hDbEvent;
};

static CLISTEVENT* MyGetEvent(int iSelection)
{
	for (auto &it : *g_clistApi.events)
		if (it->menuId == iSelection)
			return it;

	return nullptr;
}

static void EventArea_HideShowNotifyFrame()
{
	int dwVisible = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hNotifyFrame), 0) & F_VISIBLE;

	int desired;
	if (g_CluiData.dwFlags & CLUI_FRAME_SHOWALWAYS)
		desired = TRUE;
	else if (g_CluiData.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY)
		desired = g_CluiData.bNotifyActive ? TRUE : FALSE;
	else
		desired = FALSE;

	if (desired) {
		if (!dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, hNotifyFrame, 0);
	}
	else {
		if (dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, hNotifyFrame, 0);
	}
}

CListEvent* cli_AddEvent(CLISTEVENT *cle)
{
	CListEvent *p = corecli.pfnAddEvent(cle);
	if (p == nullptr)
		return nullptr;

	if (p->hContact != 0 && p->hDbEvent != 1 && !(p->flags & CLEF_ONLYAFEW)) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA | MIIM_BITMAP | MIIM_ID;
		if (p->pszService &&
			(!strncmp(MS_MSG_READMESSAGE, p->pszService, _countof(MS_MSG_READMESSAGE)) ||
			 !strncmp("GChat/DblClickEvent", p->pszService, _countof("GChat/DblClickEvent")))) {
			// dup check only for msg events
			for (int j = 0; j < GetMenuItemCount(g_CluiData.hMenuNotify); j++) {
				if (GetMenuItemInfo(g_CluiData.hMenuNotify, j, TRUE, &mii) != 0) {
					NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
					if (nmi != nullptr && (HANDLE)nmi->hContact == (HANDLE)p->hContact && nmi->iIcon == p->imlIconIndex)
						return p;
				}
			}
		}

		char *szProto = Proto_GetBaseAccountName(p->hContact);
		wchar_t *szName = Clist_GetContactDisplayName(p->hContact);
		if (szProto && szName) {
			NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) malloc(sizeof(struct NotifyMenuItemExData));
			if (nmi) {
				wchar_t szBuffer[128];
				wchar_t* szStatus = Clist_GetStatusModeDescription(db_get_w(p->hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
				wchar_t szwProto[64];
				MultiByteToWideChar(CP_ACP, 0, szProto, -1, szwProto, 64);
				szwProto[63] = 0;
				mir_snwprintf(szBuffer, L"%s: %s (%s)", szwProto, szName, szStatus);
				szBuffer[127] = 0;
				AppendMenu(g_CluiData.hMenuNotify, MF_BYCOMMAND | MF_STRING, g_CluiData.wNextMenuID, szBuffer);
				mii.hbmpItem = HBMMENU_CALLBACK;
				nmi->hContact = p->hContact;
				nmi->iIcon = p->imlIconIndex;
				nmi->hIcon = p->hIcon;
				nmi->hDbEvent = p->hDbEvent;
				mii.dwItemData = (ULONG_PTR)nmi;
				mii.wID = g_CluiData.wNextMenuID;
				SetMenuItemInfo(g_CluiData.hMenuNotify, g_CluiData.wNextMenuID, FALSE, &mii);
				p->menuId = g_CluiData.wNextMenuID;
				g_CluiData.wNextMenuID++;
				if (g_CluiData.wNextMenuID > 0x7fff)
					g_CluiData.wNextMenuID = 1;
				g_CluiData.iIconNotify = p->imlIconIndex;
			}
		}
	}
	else if (p->hContact != 0 && (p->flags & CLEF_ONLYAFEW)) {
		g_CluiData.iIconNotify = p->imlIconIndex;
		g_CluiData.hUpdateContact = p->hContact;
	}

	if (g_clistApi.events->getCount() > 0) {
		g_CluiData.bEventAreaEnabled = true;
		if (g_CluiData.bNotifyActive == false) {
			g_CluiData.bNotifyActive = true;
			EventArea_HideShowNotifyFrame();
		}
	}

	cliInvalidateRect(g_CluiData.hwndEventFrame, nullptr, FALSE);
	return p;
}


int cli_RemoveEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	// Find the event that should be removed
	CListEvent *pEvent = nullptr;
	for (auto &it : *g_clistApi.events)
		if (it->hContact == hContact && it->hDbEvent == hDbEvent) {
			pEvent = it;
			break;
		}

	// Event was not found
	if (pEvent == nullptr)
		return 1;

	// remove event from the notify menu
	int iMenuId = pEvent->menuId;
	if (iMenuId > 0) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		if (GetMenuItemInfo(g_CluiData.hMenuNotify, iMenuId, FALSE, &mii) != 0) {
			struct NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
			if (nmi && nmi->hContact == hContact && nmi->hDbEvent == hDbEvent) {
				free(nmi);
				DeleteMenu(g_CluiData.hMenuNotify, iMenuId, MF_BYCOMMAND);
			}
		}
	}

	int res = corecli.pfnRemoveEvent(hContact, hDbEvent);

	if (g_clistApi.events->getCount() == 0) {
		g_CluiData.bNotifyActive = false;
		EventArea_HideShowNotifyFrame();
	}

	if (hContact == g_CluiData.hUpdateContact || (INT_PTR)hDbEvent == 1)
		g_CluiData.hUpdateContact = 0;
	cliInvalidateRect(g_CluiData.hwndEventFrame, nullptr, FALSE);
	return res;
}


/* Implementations */

struct event_area_t
{
	HBITMAP  hBmpBackground = nullptr;
	COLORREF bkColour = CLCDEFAULT_BKCOLOUR;
	int      useWinColors = CLCDEFAULT_USEWINDOWSCOLOURS;
	int      backgroundBmpUse = CLCDEFAULT_USEBITMAP;
};

static event_area_t event_area;

static BOOL sttDrawEventAreaBackground(HWND hwnd, HDC hdc, RECT *rect)
{
	BOOL bFloat = (GetParent(hwnd) != g_clistApi.hwndContactList);
	if (g_CluiData.fDisableSkinEngine || !g_CluiData.fLayered || bFloat) {
		RECT rc;
		if (rect)
			rc = *rect;
		else
			GetClientRect(hwnd, &rc);

		if (!event_area.hBmpBackground && !event_area.useWinColors) {
			HBRUSH hbr = CreateSolidBrush(event_area.bkColour);
			FillRect(hdc, &rc, hbr);
			DeleteObject(hbr);
		}
		else DrawBackGround(hwnd, hdc, event_area.hBmpBackground, event_area.bkColour, event_area.backgroundBmpUse);
	}
	return TRUE;
}

static int ehhEventAreaBackgroundSettingsChanged(WPARAM, LPARAM)
{
	if (event_area.hBmpBackground) {
		DeleteObject(event_area.hBmpBackground);
		event_area.hBmpBackground = nullptr;
	}

	if (g_CluiData.fDisableSkinEngine) {
		event_area.bkColour = cliGetColor("EventArea", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (db_get_b(0, "EventArea", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmap(db_get_wsa(0, "EventArea", "BkBitmap"));
			if (tszBitmap != nullptr)
				event_area.hBmpBackground = Bitmap_Load(tszBitmap);
		}
		event_area.useWinColors = db_get_b(0, "EventArea", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		event_area.backgroundBmpUse = db_get_w(0, "EventArea", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}
	PostMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
	return 0;
}

void EventArea_ConfigureEventArea()
{
	int iCount = g_clistApi.events->getCount();

	g_CluiData.dwFlags &= ~(CLUI_FRAME_AUTOHIDENOTIFY | CLUI_FRAME_SHOWALWAYS);
	if (db_get_b(0, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 1) g_CluiData.dwFlags |= CLUI_FRAME_AUTOHIDENOTIFY;
	if (db_get_b(0, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 2) g_CluiData.dwFlags |= CLUI_FRAME_SHOWALWAYS;

	if (g_CluiData.dwFlags & CLUI_FRAME_SHOWALWAYS)
		g_CluiData.bNotifyActive = true;
	else if (g_CluiData.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY)
		g_CluiData.bNotifyActive = iCount > 0;
	else
		g_CluiData.bNotifyActive = false;

	EventArea_HideShowNotifyFrame();
}

/////////////////////////////////////////////////////////////////////////////////////////

static int EventArea_DrawWorker(HWND hWnd, HDC hDC)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	if (g_CluiData.fDisableSkinEngine)
		sttDrawEventAreaBackground(hWnd, hDC, &rc);
	else
		SkinDrawGlyph(hDC, &rc, &rc, "Main,ID=EventArea");

	HFONT hOldFont = g_clcPainter.ChangeToFont(hDC, nullptr, FONTID_EVENTAREA, nullptr);
	SetBkMode(hDC, TRANSPARENT);

	int iCount = GetMenuItemCount(g_CluiData.hMenuNotify);
	rc.left += 26;
	if (g_CluiData.hUpdateContact != 0) {
		wchar_t *szName = Clist_GetContactDisplayName(g_CluiData.hUpdateContact);
		int iIcon = Clist_GetContactIcon(g_CluiData.hUpdateContact);

		ske_ImageList_DrawEx(g_himlCListClc, iIcon, hDC, rc.left, (rc.bottom + rc.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc.left += 18;
		ske_DrawText(hDC, szName, -1, &rc, DT_VCENTER | DT_SINGLELINE);
		ske_ImageList_DrawEx(g_himlCListClc, (int)g_CluiData.iIconNotify, hDC, 4, (rc.bottom + rc.top - 16) / 2, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
	}
	else if (iCount > 0) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(g_CluiData.hMenuNotify, iCount - 1, TRUE, &mii);

		NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
		wchar_t *szName = Clist_GetContactDisplayName(nmi->hContact);
		int iIcon = Clist_GetContactIcon(nmi->hContact);
		ske_ImageList_DrawEx(g_himlCListClc, iIcon, hDC, rc.left, (rc.bottom + rc.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc.left += 18;
		ske_ImageList_DrawEx(g_himlCListClc, nmi->iIcon, hDC, 4, (rc.bottom + rc.top) / 2 - 8, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
		ske_DrawText(hDC, szName, -1, &rc, DT_VCENTER | DT_SINGLELINE);
	}
	else {
		HICON hIcon = (HICON)LoadImage(g_hMirApp, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 16, 16, 0);
		wchar_t *ptszEvents = TranslateT("No events");
		ske_DrawText(hDC, ptszEvents, (int)mir_wstrlen(ptszEvents), &rc, DT_VCENTER | DT_SINGLELINE);
		ske_DrawIconEx(hDC, 4, (rc.bottom + rc.top - 16) / 2, hIcon, 16, 16, 0, nullptr, DI_NORMAL | DI_COMPAT);
		DestroyIcon(hIcon);
	}

	ske_ResetTextEffect(hDC);
	SelectObject(hDC, hOldFont);
	return 0;
}

static int EventArea_Draw(HWND hwnd, HDC hDC)
{
	if (hwnd == (HWND)-1) return 0;
	if (GetParent(hwnd) == g_clistApi.hwndContactList)
		return EventArea_DrawWorker(hwnd, hDC);

	cliInvalidateRect(hwnd, nullptr, FALSE);
	return 0;
}

static int EventArea_PaintCallbackProc(HWND hWnd, HDC hDC, RECT *, HRGN, uint32_t, void *)
{
	return EventArea_Draw(hWnd, hDC);
}

static LRESULT CALLBACK EventArea_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *lpi = (LPMEASUREITEMSTRUCT)lParam;
			MENUITEMINFOA mii = { 0 };
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA | MIIM_ID;
			if (GetMenuItemInfoA(g_CluiData.hMenuNotify, lpi->itemID, FALSE, &mii) != 0) {
				if (mii.dwItemData == lpi->itemData) {
					lpi->itemWidth = 8 + 16;
					lpi->itemHeight = 0;
					return TRUE;
				}
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == (HWND)g_CluiData.hMenuNotify) {
				MENUITEMINFOA mii = { 0 };
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_DATA;
				if (GetMenuItemInfoA(g_CluiData.hMenuNotify, (UINT)dis->itemID, FALSE, &mii) != 0) {
					NotifyMenuItemExData *nmi = (NotifyMenuItemExData *)mii.dwItemData;
					if (nmi) {
						int iIcon = Clist_GetContactIcon(nmi->hContact);
						ske_ImageList_DrawEx(g_himlCListClc, nmi->iIcon, dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
						ske_ImageList_DrawEx(g_himlCListClc, iIcon, dis->hDC, 2 + GetSystemMetrics(SM_CXSMICON) + 2, (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
						return TRUE;
					}
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		if (g_CluiData.bEventAreaEnabled)
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_NOTIFYBUTTON, 0), 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTIFYBUTTON) {
			POINT pt;
			GetCursorPos(&pt);

			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA;

			int iSelection;
			if (GetMenuItemCount(g_CluiData.hMenuNotify) > 1)
				iSelection = TrackPopupMenu(g_CluiData.hMenuNotify, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
			else
				iSelection = GetMenuItemID(g_CluiData.hMenuNotify, 0);
			BOOL result = GetMenuItemInfo(g_CluiData.hMenuNotify, (UINT)iSelection, FALSE, &mii);
			if (result != 0) {
				NotifyMenuItemExData *nmi = (NotifyMenuItemExData*)mii.dwItemData;
				if (nmi) {
					CLISTEVENT *cle = MyGetEvent(iSelection);
					if (cle) {
						CLISTEVENT *cle1 = nullptr;
						CallService(cle->pszService, (WPARAM)nullptr, (LPARAM)cle);
						// re-obtain the pointer, it may already be invalid/point to another event if the
						// event we're interested in was removed by the service (nasty one...)
						cle1 = MyGetEvent(iSelection);
						if (cle1 != nullptr)
							g_clistApi.pfnRemoveEvent(cle->hContact, cle->hDbEvent);
					}
				}
			}
			break;
		}
		break;

	case WM_SIZE:
		if (!g_CluiData.fLayered)
			InvalidateRect(hwnd, nullptr, FALSE);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		if (GetParent(hwnd) == g_clistApi.hwndContactList && g_CluiData.fLayered)
			CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)hwnd, 0);
		else if (GetParent(hwnd) == g_clistApi.hwndContactList && !g_CluiData.fLayered) {
			RECT rc = { 0 };
			GetClientRect(hwnd, &rc);
			rc.right++;
			rc.bottom++;
			HDC hdc = GetDC(hwnd);
			HDC hdc2 = CreateCompatibleDC(hdc);
			HBITMAP hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			HBITMAP hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);
			ske_BltBackImage(hwnd, hdc2, &rc);
			EventArea_DrawWorker(hwnd, hdc2);
			BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
				hdc2, rc.left, rc.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);
			SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
			ReleaseDC(hwnd, hdc);
			ValidateRect(hwnd, nullptr);
		}
		else {
			RECT rc;
			GetClientRect(hwnd, &rc);
			HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdc2 = CreateCompatibleDC(hdc);
			HBITMAP hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			HBITMAP hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);
			FillRect(hdc2, &ps.rcPaint, br);
			EventArea_DrawWorker(hwnd, hdc2);
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
				hdc2, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);
			ps.fErase = FALSE;
			EndPaint(hwnd, &ps);
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

int EventArea_Create(HWND hCluiWnd)
{
	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("Event area background")"/EventArea"), 0);
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, ehhEventAreaBackgroundSettingsChanged);
	ehhEventAreaBackgroundSettingsChanged(0, 0);

	WNDCLASS wndclass = { 0 };
	wchar_t pluginname[] = L"EventArea";
	int h = GetSystemMetrics(SM_CYSMICON) + 2;
	if (GetClassInfo(g_plugin.getInst(), pluginname, &wndclass) == 0) {
		wndclass.style = 0;
		wndclass.lpfnWndProc = EventArea_WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = g_plugin.getInst();
		wndclass.hIcon = nullptr;
		wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		wndclass.lpszMenuName = nullptr;
		wndclass.lpszClassName = pluginname;
		RegisterClass(&wndclass);
	}
	g_CluiData.hwndEventFrame = CreateWindow(pluginname, pluginname, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		0, 0, 0, h, hCluiWnd, nullptr, g_plugin.getInst(), nullptr);

	// register frame
	CLISTFrame Frame = { sizeof(Frame) };
	Frame.hWnd = g_CluiData.hwndEventFrame;
	Frame.align = alBottom;
	Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	Frame.Flags = (db_get_b(0, "CLUI", "ShowEventArea", SETTING_SHOWEVENTAREAFRAME_DEFAULT) ? F_VISIBLE : 0) | F_LOCKED | F_NOBORDER | F_NO_SUBCONTAINER;
	Frame.height = h;
	Frame.szName.a = "EventArea";
	Frame.szTBname.a = LPGEN("Event area");
	hNotifyFrame = g_plugin.addFrame(&Frame);
	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)Frame.hWnd, (LPARAM)EventArea_PaintCallbackProc); //$$$$$ register sub for frame
	CallService(MS_CLIST_FRAMES_UPDATEFRAME, -1, 0);
	EventArea_HideShowNotifyFrame();

	g_CluiData.hMenuNotify = CreatePopupMenu();
	g_CluiData.wNextMenuID = 1;
	EventArea_ConfigureEventArea();
	return 0;
}

int EventArea_UnloadModule()
{
	// remove frame window
	// remove all events data from menu
	DestroyMenu(g_CluiData.hMenuNotify);
	return 0;
}
