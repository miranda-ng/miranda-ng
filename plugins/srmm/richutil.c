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
#include "richutil.h"

/*
	To initialize this library, call:
		RichUtil_Load();
	Before the application exits, call:
		RichUtil_Unload();
	
	Then to use the library (it draws the xp border around it), you need
	to make sure you control has the WS_EX_CLIENTEDGE flag.  Then you just 
	subclass it with:
		RichUtil_SubClass(hwndEdit);
	
	If no xptheme is present, the window isn't subclassed the SubClass function
	just returns.  And if WS_EX_CLIENTEDGE isn't present, the subclass does nothing.
	Otherwise it removes the border and draws it by itself.
*/

static SortedList sListInt;

static int RichUtil_CmpVal(void *p1, void *p2) 
{
	TRichUtil *tp1 = (TRichUtil*)p1;
	TRichUtil *tp2 = (TRichUtil*)p2;
	return (int)((INT_PTR)tp1->hwnd - (INT_PTR)tp2->hwnd);
}

// UxTheme Stuff
static HMODULE mTheme;

static HANDLE  (WINAPI *MyOpenThemeData)(HWND, LPCWSTR);
static HRESULT (WINAPI *MyCloseThemeData)(HANDLE);
static BOOL    (WINAPI *MyIsThemeActive)(VOID);
static HRESULT (WINAPI *MyDrawThemeBackground)(HANDLE, HDC, int, int, LPCRECT, LPCRECT);
static HRESULT (WINAPI *MyGetThemeBackgroundContentRect)(HANDLE, HDC, int, int, LPCRECT, LPRECT);
static HRESULT (WINAPI *MyDrawThemeParentBackground)(HWND, HDC, LPRECT);
static BOOL    (WINAPI *MyIsThemeBackgroundPartiallyTransparent)(HANDLE, int, int);

static CRITICAL_SECTION csRich;

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void RichUtil_ClearUglyBorder(TRichUtil *ru);

void RichUtil_Load(void) 
{
	sListInt.increment = 10;
	sListInt.sortFunc = RichUtil_CmpVal;

	mTheme = IsWinVerXPPlus() ? GetModuleHandleA("uxtheme") : 0;

	InitializeCriticalSection(&csRich);
	if (!mTheme) return;

	MyOpenThemeData = (HANDLE (WINAPI *)(HWND, LPCWSTR))GetProcAddress(mTheme, "OpenThemeData");
	MyCloseThemeData = (HRESULT (WINAPI *)(HANDLE))GetProcAddress(mTheme, "CloseThemeData");
	MyIsThemeActive = (BOOL (WINAPI *)(VOID))GetProcAddress(mTheme, "IsThemeActive");
	MyDrawThemeBackground = (HRESULT (WINAPI *)(HANDLE, HDC, int, int, const RECT*, const RECT *))GetProcAddress(mTheme, "DrawThemeBackground");
	MyGetThemeBackgroundContentRect = (HRESULT (WINAPI *)(HANDLE, HDC, int, int,  const RECT *, RECT *))GetProcAddress(mTheme, "GetThemeBackgroundContentRect");
	MyDrawThemeParentBackground = (HRESULT (WINAPI *)(HWND, HDC, RECT*))GetProcAddress(mTheme, "DrawThemeParentBackground");
	MyIsThemeBackgroundPartiallyTransparent = (BOOL (WINAPI *)(HANDLE, int, int))GetProcAddress(mTheme, "IsThemeBackgroundPartiallyTransparent");

	if (!MyOpenThemeData ||
		!MyCloseThemeData ||
		!MyIsThemeActive ||
		!MyDrawThemeBackground ||
		!MyGetThemeBackgroundContentRect ||
		!MyDrawThemeParentBackground ||
		!MyIsThemeBackgroundPartiallyTransparent) 
	{
		FreeLibrary(mTheme);
		mTheme = NULL;
	}
}

void RichUtil_Unload(void) 
{
	li.List_Destroy(&sListInt);
	DeleteCriticalSection(&csRich);
	if (mTheme)
		FreeLibrary(mTheme);
}

int RichUtil_SubClass(HWND hwndEdit) 
{
	if (IsWindow(hwndEdit)) 
	{
		int idx;

		TRichUtil *ru = (TRichUtil*)mir_calloc(sizeof(TRichUtil));
		
		ru->hwnd = hwndEdit;
		ru->hasUglyBorder = 0;

		EnterCriticalSection(&csRich);
		if (!li.List_GetIndex(&sListInt, ru, &idx))
			li.List_Insert(&sListInt, ru, idx);
		LeaveCriticalSection(&csRich);

		ru->origProc = (WNDPROC)SetWindowLongPtr(ru->hwnd, GWLP_WNDPROC, (LONG_PTR)&RichUtil_Proc);
		RichUtil_ClearUglyBorder(ru);
		return 1;
	}
	return 0;
}

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRichUtil *ru;

	EnterCriticalSection(&csRich);
	ru = li.List_Find(&sListInt, (TRichUtil*)&hwnd);
	LeaveCriticalSection(&csRich);

	if (ru == NULL) return 0;

	switch(msg) 
	{
		case WM_CHAR:
		{
			HWND hwndMsg = GetDlgItem(GetParent(hwnd), IDC_MESSAGE);
			if (hwndMsg != hwnd)
			{
				SetFocus(hwndMsg);
				SendMessage(hwndMsg, WM_CHAR, wParam, lParam);
			}
			break;
		}
		
		case WM_THEMECHANGED:
		case WM_STYLECHANGED:
		{
			RichUtil_ClearUglyBorder(ru);
			break;
		}

		case WM_NCPAINT:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
			if (ru->hasUglyBorder && MyIsThemeActive()) 
			{
				HANDLE hTheme = MyOpenThemeData(ru->hwnd, L"EDIT");
				if (hTheme) 
				{
					RECT rcBorder;
					RECT rcClient;
					int nState;
					HDC hdc = GetWindowDC(ru->hwnd);
					LONG style = GetWindowLong(hwnd, GWL_STYLE);

					GetWindowRect(hwnd, &rcBorder);
					rcBorder.right -= rcBorder.left; rcBorder.bottom -= rcBorder.top;
					rcBorder.left = rcBorder.top = 0;
					CopyRect(&rcClient, &rcBorder);
					rcClient.left += ru->rect.left;
					rcClient.top += ru->rect.top;
					rcClient.right -= ru->rect.right;
					rcClient.bottom -= ru->rect.bottom;
					ExcludeClipRect(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

					if (MyIsThemeBackgroundPartiallyTransparent(hTheme, EP_EDITTEXT, ETS_NORMAL))
						MyDrawThemeParentBackground(hwnd, hdc, &rcBorder);

					if (style & WS_DISABLED)
						nState = ETS_DISABLED;
					else if (style & ES_READONLY)
						nState = ETS_READONLY;
					else 
						nState = ETS_NORMAL;

					MyDrawThemeBackground(hTheme, hdc, EP_EDITTEXT, nState, &rcBorder, NULL);
					MyCloseThemeData(hTheme);
					ReleaseDC(hwnd, hdc);
					return 0;
				}
			}
			return ret;
		}
		case WM_NCCALCSIZE:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
			NCCALCSIZE_PARAMS *ncsParam = (NCCALCSIZE_PARAMS*)lParam;
			
			if (ru->hasUglyBorder && MyIsThemeActive()) 
			{
				HANDLE hTheme = MyOpenThemeData(hwnd, L"EDIT");

				if (hTheme) 
				{
					RECT rcClient ={0}; 
					HDC hdc = GetDC(GetParent(hwnd));

					if (MyGetThemeBackgroundContentRect(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &ncsParam->rgrc[0], &rcClient) == S_OK) 
					{
						ru->rect.left = rcClient.left-ncsParam->rgrc[0].left;
						ru->rect.top = rcClient.top-ncsParam->rgrc[0].top;
						ru->rect.right = ncsParam->rgrc[0].right-rcClient.right;
						ru->rect.bottom = ncsParam->rgrc[0].bottom-rcClient.bottom;
						ncsParam->rgrc[0] = rcClient;

						MyCloseThemeData(hTheme);
						ReleaseDC(GetParent(hwnd), hdc);
						return WVR_REDRAW;
					}
					ReleaseDC(GetParent(hwnd), hdc);
					MyCloseThemeData(hTheme);
				}
			}
			return ret;
		}

		case WM_ENABLE:
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
			break;

		case WM_GETDLGCODE:
			return CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam) & ~DLGC_HASSETSEL;

		case WM_NCDESTROY:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);

			if (IsWindow(hwnd)) 
			{
				if((WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC) == &RichUtil_Proc)
					SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ru->origProc);
			}

			EnterCriticalSection(&csRich);
			li.List_RemovePtr(&sListInt, ru);
			LeaveCriticalSection(&csRich);

			mir_free(ru);
			return ret;
		}
	}
	return CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
}

static void RichUtil_ClearUglyBorder(TRichUtil *ru)
{
	if (mTheme && MyIsThemeActive() && GetWindowLongPtr(ru->hwnd, GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
	{
		ru->hasUglyBorder = 1;
		SetWindowLongPtr(ru->hwnd, GWL_EXSTYLE, GetWindowLongPtr(ru->hwnd, GWL_EXSTYLE) ^ WS_EX_CLIENTEDGE);
	}
	// Redraw window since the style may have changed
	SetWindowPos(ru->hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	RedrawWindow(ru->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
}
