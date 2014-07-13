/*
Copyright 2000-2010 Miranda IM project,
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

#include "commonheaders.h"

/*
	To initialize this library, call:
		RichUtil_Load();
	Before the application exits, call:
		RichUtil_Unload();

	Then to use the library (it draws the xp border around it), you need
	to make sure you control has the WS_EX_STATICEDGE flag.  Then you just
	subclass it with:
		RichUtil_SubClass(hwndEdit);

	If no xptheme is present, the window isn't subclassed the SubClass function
	just returns.  And if WS_EX_STATICEDGE isn't present, the subclass does nothing.
	Otherwise it removes the border and draws it by itself.
*/

static SortedList sListInt;

static int RichUtil_CmpVal(void *p1, void *p2)
{
	TRichUtil *tp1 = (TRichUtil *)p1;
	TRichUtil *tp2 = (TRichUtil *)p2;
	return (int)((INT_PTR)tp1->hwnd - (INT_PTR)tp2->hwnd);
}

static mir_cs csRich;

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void RichUtil_ClearUglyBorder(TRichUtil *ru);

void RichUtil_Load(void)
{
	sListInt.increment = 10;
	sListInt.sortFunc = RichUtil_CmpVal;
}

void RichUtil_Unload(void)
{
	List_Destroy(&sListInt);
}

int RichUtil_SubClass(HWND hwndEdit)
{
	if (IsWindow(hwndEdit)) {
		int idx;

		TRichUtil *ru = (TRichUtil *)mir_calloc(sizeof(TRichUtil));

		ru->hwnd = hwndEdit;
		ru->hasUglyBorder = 0;
		{
			mir_cslock lck(csRich);
			if (!List_GetIndex(&sListInt, ru, &idx))
				List_Insert(&sListInt, ru, idx);
		}
		mir_subclassWindow(ru->hwnd, RichUtil_Proc);
		RichUtil_ClearUglyBorder(ru);
		return 1;
	}
	return 0;
}

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRichUtil *ru = NULL, tru;
	int idx;
	LRESULT ret;

	tru.hwnd = hwnd;
	{
		mir_cslock lck(csRich);
		if (List_GetIndex(&sListInt, &tru, &idx))
			ru = (TRichUtil *)sListInt.items[idx];
	}

	switch (msg) {
	case WM_THEMECHANGED:
	case WM_STYLECHANGED:
		RichUtil_ClearUglyBorder(ru);
		break;

	case WM_NCPAINT:
		ret = mir_callNextSubclass(hwnd, RichUtil_Proc, msg, wParam, lParam);
		if (ru->hasUglyBorder && IsThemeActive())
		{
			HANDLE hTheme = OpenThemeData(ru->hwnd, L"EDIT");

			if (hTheme) {
				RECT rcBorder;
				RECT rcClient;
				int nState;
				HDC hdc = GetWindowDC(ru->hwnd);

				GetWindowRect(hwnd, &rcBorder);
				rcBorder.right -= rcBorder.left; rcBorder.bottom -= rcBorder.top;
				rcBorder.left = rcBorder.top = 0;
				CopyRect(&rcClient, &rcBorder);
				rcClient.left += ru->rect.left;
				rcClient.top += ru->rect.top;
				rcClient.right -= ru->rect.right;
				rcClient.bottom -= ru->rect.bottom;
				ExcludeClipRect(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

				if (IsThemeBackgroundPartiallyTransparent(hTheme, EP_EDITTEXT, ETS_NORMAL))
					DrawThemeParentBackground(hwnd, hdc, &rcBorder);

				if (!IsWindowEnabled(hwnd))
					nState = ETS_DISABLED;
				else if (SendMessage(hwnd, EM_GETOPTIONS, 0, 0) & ECO_READONLY)
					nState = ETS_READONLY;
				else nState = ETS_NORMAL;

				DrawThemeBackground(hTheme, hdc, EP_EDITTEXT, nState, &rcBorder, NULL);
				CloseThemeData(hTheme);
				ReleaseDC(hwnd, hdc);
				return 0;
			}
		}
		return ret;

	case WM_NCCALCSIZE:
		{
			ret = mir_callNextSubclass(hwnd, RichUtil_Proc, msg, wParam, lParam);
			NCCALCSIZE_PARAMS *ncsParam = (NCCALCSIZE_PARAMS *)lParam;

			if (ru->hasUglyBorder && IsThemeActive()) {
				HANDLE hTheme = OpenThemeData(hwnd, L"EDIT");
				if (hTheme) {
					RECT rcClient = {0};
					HDC hdc = GetDC(GetParent(hwnd));

					if (GetThemeBackgroundContentRect(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &ncsParam->rgrc[0], &rcClient) == S_OK) {
						ru->rect.left = rcClient.left - ncsParam->rgrc[0].left;
						ru->rect.top = rcClient.top - ncsParam->rgrc[0].top;
						ru->rect.right = ncsParam->rgrc[0].right - rcClient.right;
						ru->rect.bottom = ncsParam->rgrc[0].bottom - rcClient.bottom;
						ncsParam->rgrc[0] = rcClient;

						CloseThemeData(hTheme);
						ReleaseDC(GetParent(hwnd), hdc);
						return WVR_REDRAW;
					}
					ReleaseDC(GetParent(hwnd), hdc);
					CloseThemeData(hTheme);
				}
			}
		}
		return ret;

	case WM_ENABLE:
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
		break;

	case WM_GETDLGCODE:
		return mir_callNextSubclass(hwnd, RichUtil_Proc, msg, wParam, lParam) & ~DLGC_HASSETSEL;

	case WM_NCDESTROY:
		ret = mir_callNextSubclass(hwnd, RichUtil_Proc, msg, wParam, lParam);
		{
			mir_cslock lck(csRich);
			List_Remove(&sListInt, idx);
		}
		mir_free(ru);
		return ret;
	}
	return mir_callNextSubclass(hwnd, RichUtil_Proc, msg, wParam, lParam);
}

static void RichUtil_ClearUglyBorder(TRichUtil *ru)
{
	if (IsThemeActive() && GetWindowLongPtr(ru->hwnd, GWL_EXSTYLE) & WS_EX_STATICEDGE) {
		ru->hasUglyBorder = 1;
		SetWindowLongPtr(ru->hwnd, GWL_EXSTYLE, GetWindowLongPtr(ru->hwnd, GWL_EXSTYLE) ^ WS_EX_STATICEDGE);
	}

	// Redraw window since the style may have changed
	SetWindowPos(ru->hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	RedrawWindow(ru->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
}
