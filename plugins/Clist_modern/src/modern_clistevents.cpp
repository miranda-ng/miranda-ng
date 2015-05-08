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

#include "hdr/modern_commonheaders.h"
#include "m_clui.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_clcpaint.h"

/**************************************************/
/*   Notify Event Area Frame implementation       */
/**************************************************/

/* Declarations */
static HANDLE hNotifyFrame = NULL;
static int EventArea_PaintCallbackProc(HWND hWnd, HDC hDC, RECT *rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData);
static int EventArea_Draw(HWND hwnd, HDC hDC);
static int EventArea_DrawWorker(HWND hwnd, HDC hDC);
static void EventArea_HideShowNotifyFrame();
static LRESULT CALLBACK EventArea_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int EventArea_Create(HWND hCluiWnd);
int EventArea_UnloadModule();
void EventArea_ConfigureEventArea();

/**************************************************/

HWND g_hwndEventArea = 0;

struct CListEvent {
	int imlIconIndex;
	int flashesDone;
	CLISTEVENT cle;

	int menuId;
	int imlIconOverlayIndex;
};

static CListEvent *event;
static int eventCount;
static int disableTrayFlash;
static int disableIconFlash;


struct CListImlIcon {
	int index;
	HICON hIcon;
};

static struct CListImlIcon *imlIcon;
static int imlIconCount;
static UINT flashTimerId;
static int iconsOn;


struct NotifyMenuItemExData {
	MCONTACT hContact;
	int iIcon;              // icon index in the image list
	HICON hIcon;            // corresponding icon handle
	MEVENT hDbEvent;
};

static CLISTEVENT* MyGetEvent(int iSelection)
{
	for (int i = 0; i < pcli->events.count; i++) {
		CListEvent *p = pcli->events.items[i];
		if (p->menuId == iSelection)
			return &p->cle;
	}
	return NULL;
}


CListEvent* cliCreateEvent(void)
{
	CListEvent *p = (CListEvent *)mir_calloc(sizeof(CListEvent));
	return p;
}

CListEvent* cli_AddEvent(CLISTEVENT *cle)
{
	CListEvent* p = corecli.pfnAddEvent(cle);
	if (p == NULL)
		return NULL;

	if (p->cle.hContact != 0 && p->cle.hDbEvent != 1 && !(p->cle.flags & CLEF_ONLYAFEW)) {
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_DATA | MIIM_BITMAP | MIIM_ID;
		if (p->cle.pszService &&
			(!strncmp("SRMsg/ReadMessage", p->cle.pszService, SIZEOF("SRMsg/ReadMessage")) ||
			!strncmp("GChat/DblClickEvent", p->cle.pszService, SIZEOF("GChat/DblClickEvent"))))
		{
			// dup check only for msg events
			for (int j = 0; j < GetMenuItemCount(g_CluiData.hMenuNotify); j++) {
				if (GetMenuItemInfo(g_CluiData.hMenuNotify, j, TRUE, &mii) != 0) {
					NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
					if (nmi != 0 && (HANDLE)nmi->hContact == (HANDLE)p->cle.hContact && nmi->iIcon == p->imlIconIndex)
						return p;
				}
			}
		}

		char *szProto = GetContactProto(p->cle.hContact);
		TCHAR *szName = pcli->pfnGetContactDisplayName(p->cle.hContact, 0);
		if (szProto && szName) {
			NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) malloc(sizeof(struct NotifyMenuItemExData));
			if (nmi) {
				TCHAR szBuffer[128];
				TCHAR* szStatus = pcli->pfnGetStatusModeDescription(db_get_w(p->cle.hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
				TCHAR szwProto[64];
				MultiByteToWideChar(CP_ACP, 0, szProto, -1, szwProto, 64);
				szwProto[63] = 0;
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s: %s (%s)"), szwProto, szName, szStatus);
				szBuffer[127] = 0;
				AppendMenu(g_CluiData.hMenuNotify, MF_BYCOMMAND | MF_STRING, g_CluiData.wNextMenuID, szBuffer);
				mii.hbmpItem = HBMMENU_CALLBACK;
				nmi->hContact = p->cle.hContact;
				nmi->iIcon = p->imlIconIndex;
				nmi->hIcon = p->cle.hIcon;
				nmi->hDbEvent = p->cle.hDbEvent;
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
	else if (p->cle.hContact != 0 && (p->cle.flags & CLEF_ONLYAFEW)) {
		g_CluiData.iIconNotify = p->imlIconIndex;
		g_CluiData.hUpdateContact = p->cle.hContact;
	}

	if (pcli->events.count > 0) {
		g_CluiData.bEventAreaEnabled = TRUE;
		if (g_CluiData.bNotifyActive == FALSE) {
			g_CluiData.bNotifyActive = TRUE;
			EventArea_HideShowNotifyFrame();
		}
	}

	CLUI__cliInvalidateRect(g_CluiData.hwndEventFrame, NULL, FALSE);
	return p;
}


int cli_RemoveEvent(MCONTACT hContact, MEVENT hDbEvent)
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
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		if (GetMenuItemInfo(g_CluiData.hMenuNotify, pcli->events.items[i]->menuId, FALSE, &mii) != 0) {
			struct NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
			if (nmi && nmi->hContact == hContact && nmi->hDbEvent == hDbEvent) {
				free(nmi);
				DeleteMenu(g_CluiData.hMenuNotify, pcli->events.items[i]->menuId, MF_BYCOMMAND);
			}
		}
	}

	int res = corecli.pfnRemoveEvent(hContact, hDbEvent);

	if (pcli->events.count == 0) {
		g_CluiData.bNotifyActive = FALSE;
		EventArea_HideShowNotifyFrame();
	}

	if (hContact == g_CluiData.hUpdateContact || (INT_PTR)hDbEvent == 1)
		g_CluiData.hUpdateContact = 0;
	CLUI__cliInvalidateRect(g_CluiData.hwndEventFrame, NULL, FALSE);
	return res;
}


/* Implementations */

struct event_area_t
{
	HBITMAP  hBmpBackground;
	COLORREF bkColour;
	int      useWinColors;
	int      backgroundBmpUse;

	event_area_t() :
		hBmpBackground(NULL),
		bkColour(CLCDEFAULT_BKCOLOUR),
		useWinColors(CLCDEFAULT_USEWINDOWSCOLOURS),
		backgroundBmpUse(CLCDEFAULT_USEBITMAP)
	{}
};

static event_area_t event_area;

static BOOL sttDrawEventAreaBackground(HWND hwnd, HDC hdc, RECT *rect)
{
	BOOL bFloat = (GetParent(hwnd) != pcli->hwndContactList);
	if (g_CluiData.fDisableSkinEngine || !g_CluiData.fLayered || bFloat)
	{
		RECT rc;

		if (rect) rc = *rect;
		else	  GetClientRect(hwnd, &rc);

		if (!event_area.hBmpBackground && !event_area.useWinColors)
		{
			HBRUSH hbr = CreateSolidBrush(event_area.bkColour);
			FillRect(hdc, &rc, hbr);
			DeleteObject(hbr);
		}
		else
		{
			DrawBackGround(hwnd, hdc, event_area.hBmpBackground, event_area.bkColour, event_area.backgroundBmpUse);
		}
	}
	return TRUE;
}

COLORREF sttGetColor(char * module, char * color, COLORREF defColor); //clcutils

static int  ehhEventAreaBackgroundSettingsChanged(WPARAM, LPARAM)
{
	if (event_area.hBmpBackground)
	{
		DeleteObject(event_area.hBmpBackground);
		event_area.hBmpBackground = NULL;
	}
	if (g_CluiData.fDisableSkinEngine)
	{
		DBVARIANT dbv;
		event_area.bkColour = sttGetColor("EventArea", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (db_get_b(NULL, "EventArea", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			if (!db_get_s(NULL, "EventArea", "BkBitmap", &dbv)) {
				event_area.hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
			}
		}
		event_area.useWinColors = db_get_b(NULL, "EventArea", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		event_area.backgroundBmpUse = db_get_w(NULL, "EventArea", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}
	PostMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
	return 0;
}

void EventArea_ConfigureEventArea()
{
	int iCount = pcli->events.count;

	g_CluiData.dwFlags &= ~(CLUI_FRAME_AUTOHIDENOTIFY | CLUI_FRAME_SHOWALWAYS);
	if (db_get_b(NULL, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 1) g_CluiData.dwFlags |= CLUI_FRAME_AUTOHIDENOTIFY;
	if (db_get_b(NULL, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 2) g_CluiData.dwFlags |= CLUI_FRAME_SHOWALWAYS;

	if (g_CluiData.dwFlags & CLUI_FRAME_SHOWALWAYS)
		g_CluiData.bNotifyActive = 1;
	else if (g_CluiData.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY)
		g_CluiData.bNotifyActive = iCount > 0 ? 1 : 0;
	else
		g_CluiData.bNotifyActive = 0;

	EventArea_HideShowNotifyFrame();
}


static int EventArea_PaintCallbackProc(HWND hWnd, HDC hDC, RECT *, HRGN, DWORD, void *)
{
	return EventArea_Draw(hWnd, hDC);
}

static int EventArea_Draw(HWND hwnd, HDC hDC)
{
	if (hwnd == (HWND)-1) return 0;
	if (GetParent(hwnd) == pcli->hwndContactList)
		return EventArea_DrawWorker(hwnd, hDC);

	CLUI__cliInvalidateRect(hwnd, NULL, FALSE);
	return 0;
}

static int EventArea_DrawWorker(HWND hWnd, HDC hDC)
{
	RECT rc;
	HFONT hOldFont;
	GetClientRect(hWnd, &rc);
	if (g_CluiData.fDisableSkinEngine)
		sttDrawEventAreaBackground(hWnd, hDC, &rc);
	else
		SkinDrawGlyph(hDC, &rc, &rc, "Main,ID=EventArea");

	hOldFont = g_clcPainter.ChangeToFont(hDC, NULL, FONTID_EVENTAREA, NULL);
	SetBkMode(hDC, TRANSPARENT);

	int iCount = GetMenuItemCount(g_CluiData.hMenuNotify);
	rc.left += 26;
	if (g_CluiData.hUpdateContact != 0) {
		TCHAR *szName = pcli->pfnGetContactDisplayName(g_CluiData.hUpdateContact, 0);
		int iIcon = cli_GetContactIcon(g_CluiData.hUpdateContact);

		ske_ImageList_DrawEx(g_himlCListClc, iIcon, hDC, rc.left, (rc.bottom + rc.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc.left += 18;
		ske_DrawText(hDC, szName, -1, &rc, DT_VCENTER | DT_SINGLELINE);
		ske_ImageList_DrawEx(g_himlCListClc, (int)g_CluiData.iIconNotify, hDC, 4, (rc.bottom + rc.top - 16) / 2, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
	}
	else if (iCount > 0) {
		MENUITEMINFO mii = { 0 };
		struct NotifyMenuItemExData *nmi;
		TCHAR *szName;
		int iIcon;

		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(g_CluiData.hMenuNotify, iCount - 1, TRUE, &mii);
		nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
		szName = pcli->pfnGetContactDisplayName(nmi->hContact, 0);
		iIcon = cli_GetContactIcon(nmi->hContact);
		ske_ImageList_DrawEx(g_himlCListClc, iIcon, hDC, rc.left, (rc.bottom + rc.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc.left += 18;
		ske_ImageList_DrawEx(g_himlCListClc, nmi->iIcon, hDC, 4, (rc.bottom + rc.top) / 2 - 8, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
		ske_DrawText(hDC, szName, -1, &rc, DT_VCENTER | DT_SINGLELINE);
	}
	else {
		HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 16, 16, 0);
		TCHAR *ptszEvents = TranslateT("No events");
		ske_DrawText(hDC, ptszEvents, (int)mir_tstrlen(ptszEvents), &rc, DT_VCENTER | DT_SINGLELINE);
		ske_DrawIconEx(hDC, 4, (rc.bottom + rc.top - 16) / 2, hIcon, 16, 16, 0, 0, DI_NORMAL | DI_COMPAT);
		DestroyIcon(hIcon);
	}

	ske_ResetTextEffect(hDC);
	SelectObject(hDC, hOldFont);
	return 0;
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
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)hNotifyFrame, 0);
	}
	else {
		if (dwVisible)
			CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)hNotifyFrame, 0);
	}
}

int EventArea_Create(HWND hCluiWnd)
{

	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("Event area background")"/EventArea"), 0);
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, ehhEventAreaBackgroundSettingsChanged);
	ehhEventAreaBackgroundSettingsChanged(0, 0);

	WNDCLASS wndclass = { 0 };
	TCHAR pluginname[] = _T("EventArea");
	int h = GetSystemMetrics(SM_CYSMICON) + 2;
	if (GetClassInfo(g_hInst, pluginname, &wndclass) == 0) {
		wndclass.style = 0;
		wndclass.lpfnWndProc = EventArea_WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = g_hInst;
		wndclass.hIcon = NULL;
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = pluginname;
		RegisterClass(&wndclass);
	}
	g_CluiData.hwndEventFrame = CreateWindow(pluginname, pluginname, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		0, 0, 0, h, hCluiWnd, NULL, g_hInst, NULL);

	// register frame
	CLISTFrame Frame = { sizeof(Frame) };
	Frame.hWnd = g_CluiData.hwndEventFrame;
	Frame.align = alBottom;
	Frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
	Frame.Flags = (db_get_b(NULL, "CLUI", "ShowEventArea", SETTING_SHOWEVENTAREAFRAME_DEFAULT) ? F_VISIBLE : 0) | F_LOCKED | F_NOBORDER | F_NO_SUBCONTAINER | F_TCHAR;
	Frame.height = h;
	Frame.tname = _T("EventArea"); //do not translate
	Frame.TBtname = TranslateT("Event area");
	hNotifyFrame = (HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);
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
			NotifyMenuItemExData *nmi = 0;

			MENUITEMINFOA mii = { 0 };
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA;
			if (GetMenuItemInfoA(g_CluiData.hMenuNotify, (UINT)dis->itemID, FALSE, &mii) != 0) {
				nmi = (NotifyMenuItemExData *)mii.dwItemData;
				if (nmi) {
					int iIcon = cli_GetContactIcon(nmi->hContact);
					ske_ImageList_DrawEx(g_himlCListClc, nmi->iIcon, dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
					ske_ImageList_DrawEx(g_himlCListClc, iIcon, dis->hDC, 2 + GetSystemMetrics(SM_CXSMICON) + 2, (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), CLR_NONE, CLR_NONE, ILD_NORMAL);
					return TRUE;
				}
			}
		}
		break;
	}
	case WM_LBUTTONUP:
		if (g_CluiData.bEventAreaEnabled)
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_NOTIFYBUTTON, 0), 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTIFYBUTTON) {
			int iSelection;
			MENUITEMINFO mii = { 0 };
			POINT pt;
			struct NotifyMenuItemExData *nmi = 0;
			int iCount = GetMenuItemCount(g_CluiData.hMenuNotify);
			BOOL result;

			GetCursorPos(&pt);
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_DATA;
			if (iCount > 1)
				iSelection = TrackPopupMenu(g_CluiData.hMenuNotify, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
			else
				iSelection = GetMenuItemID(g_CluiData.hMenuNotify, 0);
			result = GetMenuItemInfo(g_CluiData.hMenuNotify, (UINT)iSelection, FALSE, &mii);
			if (result != 0) {
				nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
				if (nmi) {
					CLISTEVENT *cle = MyGetEvent(iSelection);
					if (cle) {
						CLISTEVENT *cle1 = NULL;
						CallService(cle->pszService, (WPARAM)NULL, (LPARAM)cle);
						// re-obtain the pointer, it may already be invalid/point to another event if the
						// event we're interested in was removed by the service (nasty one...)
						cle1 = MyGetEvent(iSelection);
						if (cle1 != NULL)
							CallService(MS_CLIST_REMOVEEVENT, (WPARAM)cle->hContact, (LPARAM)cle->hDbEvent);
					}
				}
			}
			break;
		}
		break;

	case WM_SIZE:
		if (!g_CluiData.fLayered)
			InvalidateRect(hwnd, NULL, FALSE);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		if (GetParent(hwnd) == pcli->hwndContactList && g_CluiData.fLayered)
			CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)hwnd, 0);
		else if (GetParent(hwnd) == pcli->hwndContactList && !g_CluiData.fLayered) {
			HDC hdc, hdc2;
			HBITMAP hbmp, hbmpo;
			RECT rc = { 0 };
			GetClientRect(hwnd, &rc);
			rc.right++;
			rc.bottom++;
			hdc = GetDC(hwnd);
			hdc2 = CreateCompatibleDC(hdc);
			hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);
			ske_BltBackImage(hwnd, hdc2, &rc);
			EventArea_DrawWorker(hwnd, hdc2);
			BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
				hdc2, rc.left, rc.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);
			SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
			ReleaseDC(hwnd, hdc);
			ValidateRect(hwnd, NULL);
		}
		else {
			HDC hdc, hdc2;
			HBITMAP hbmp, hbmpo;
			RECT rc;
			PAINTSTRUCT ps;
			HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
			GetClientRect(hwnd, &rc);
			hdc = BeginPaint(hwnd, &ps);
			hdc2 = CreateCompatibleDC(hdc);
			hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);
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
