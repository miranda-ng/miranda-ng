/*
SRMM

Copyright 2000-2005 Miranda ICQ/IM project, 
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
#include <windows.h>
#include <stdio.h>
#include <richedit.h>
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
// list crap
typedef struct _RList {
	struct _RList *next;
	struct _RList *prev;
	TRichUtil *data;
} RList;

TRichUtil *rlist_find(RList *list, HWND hwnd);
RList *rlist_append(RList *list, TRichUtil *data);
RList *rlist_remove_link(RList *list, const RList *link);
RList *rlist_remove(RList * list, TRichUtil *data);
void rlist_free(RList * list);

TRichUtil *rlist_find(RList *list, HWND hwnd) {
	RList *n;
	
	if (hwnd==NULL) return NULL;
	for (n=list; n!=NULL; n=n->next) {
		if (n->data&&n->data->hwnd==hwnd) return n->data;
	}
	return NULL;
}

RList *rlist_append(RList *list, TRichUtil *data) {
	RList *n;
	RList *new_list = malloc(sizeof(RList));
	RList *attach_to = NULL;

	new_list->next = NULL;
	new_list->data = data;
	for (n=list; n!=NULL; n=n->next) {
		attach_to = n;
	}
	if (attach_to==NULL) {
		new_list->prev = NULL;
		return new_list;
	} 
	else {
		new_list->prev = attach_to;
		attach_to->next = new_list;
		return list;
	}
}

RList *rlist_remove_link(RList *list, const RList *link) {
	if (!link)
		return list;

	if (link->next)
		link->next->prev = link->prev;
	if (link->prev)
		link->prev->next = link->next;
	if (link==list)
		list = link->next;
	return list;
}

RList *rlist_remove(RList *list, TRichUtil *data) {
	RList *n;

	for (n=list; n!=NULL; n=n->next) {
		if (n->data==data) {
			RList *newlist = rlist_remove_link(list, n);
			free(n);
			return newlist;
		}
	}
	return list;
}

void rlist_free(RList *list) {
	RList *n = list;

	while (n!=NULL) {
		RList *next = n->next;
		free(n);
		n = next;
	}
}

// UxTheme Stuff
static HMODULE mTheme = 0;
static HANDLE  (WINAPI *MyOpenThemeData)(HWND,LPCWSTR) = 0;
static HRESULT (WINAPI *MyCloseThemeData)(HANDLE) = 0;
static BOOL    (WINAPI *MyIsThemeActive)() = 0;
static HRESULT (WINAPI *MyDrawThemeBackground)(HANDLE,HDC,int,int,const RECT*,const RECT *) = 0;
static HRESULT (WINAPI *MyGetThemeBackgroundContentRect)(HANDLE,HDC,int,int,const RECT *,RECT *) = 0;
static HRESULT (WINAPI *MyDrawThemeParentBackground)(HWND,HDC,RECT*) = 0;
static BOOL    (WINAPI *MyIsThemeBackgroundPartiallyTransparent)(HANDLE,int,int) = 0;

static RList *slist = NULL;
static CRITICAL_SECTION csRich;

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static RichUtil_ClearUglyBorder(TRichUtil *ru);

void RichUtil_Load() {
	mTheme = RIsWinVerXPPlus()?LoadLibraryA("uxtheme.dll"):0;
	InitializeCriticalSection(&csRich);
	if (!mTheme) return;
	MyOpenThemeData = (HANDLE (WINAPI *)(HWND, LPCWSTR))GetProcAddress(mTheme, "OpenThemeData");
	MyCloseThemeData = (HRESULT (WINAPI *)(HANDLE))GetProcAddress(mTheme, "CloseThemeData");
	MyIsThemeActive = (BOOL (WINAPI *)())GetProcAddress(mTheme, "IsThemeActive");
	MyDrawThemeBackground = (HRESULT (WINAPI *)(HANDLE, HDC, int, int, const RECT*, const RECT *))GetProcAddress(mTheme, "DrawThemeBackground");
	MyGetThemeBackgroundContentRect = (HRESULT (WINAPI *)(HANDLE, HDC, int, int,  const RECT *, RECT *))GetProcAddress(mTheme, "GetThemeBackgroundContentRect");
	MyDrawThemeParentBackground = (HRESULT (WINAPI *)(HWND, HDC, RECT*))GetProcAddress(mTheme, "DrawThemeParentBackground");
	MyIsThemeBackgroundPartiallyTransparent = (BOOL (WINAPI *)(HANDLE, int, int))GetProcAddress(mTheme, "IsThemeBackgroundPartiallyTransparent");
	if (!MyOpenThemeData||
			!MyCloseThemeData||
			!MyIsThemeActive||
			!MyDrawThemeBackground||
			!MyGetThemeBackgroundContentRect||
			!MyDrawThemeParentBackground||
			!MyIsThemeBackgroundPartiallyTransparent) {
		FreeLibrary(mTheme);
		mTheme=NULL;
	}
}

void RichUtil_Unload() {
	DeleteCriticalSection(&csRich);
	if (mTheme) {
		FreeLibrary(mTheme);
	}
}

int RichUtil_SubClass(HWND hwndEdit) {
	if (IsWindow(hwndEdit)) {
		TRichUtil *ru = (TRichUtil*)malloc(sizeof(TRichUtil));
		
		ZeroMemory(ru, sizeof(TRichUtil));
		ru->hwnd = hwndEdit;
		ru->hasUglyBorder = 0;
		EnterCriticalSection(&csRich);
		slist = rlist_append(slist, ru);
		LeaveCriticalSection(&csRich);
		SetWindowLongPtr(ru->hwnd, GWLP_USERDATA, (LONG_PTR)ru); // Ugly hack
		ru->origProc = (WNDPROC)SetWindowLongPtr(ru->hwnd, GWLP_WNDPROC, (LONG_PTR)&RichUtil_Proc);
		RichUtil_ClearUglyBorder(ru);
		return 1;
	}
	return 0;
}

static LRESULT CALLBACK RichUtil_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	TRichUtil *ru;
	
	EnterCriticalSection(&csRich);
	ru = rlist_find(slist, hwnd);
	LeaveCriticalSection(&csRich);
	switch(msg) {
		case WM_THEMECHANGED:
		case WM_STYLECHANGED:
		{
			RichUtil_ClearUglyBorder(ru);
			break;
		}
		case WM_NCPAINT:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
			if (ru->hasUglyBorder&&MyIsThemeActive()) {
				HANDLE hTheme = MyOpenThemeData(ru->hwnd, L"EDIT");

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
					if(MyIsThemeBackgroundPartiallyTransparent(hTheme, EP_EDITTEXT, ETS_NORMAL))
						MyDrawThemeParentBackground(hwnd, hdc, &rcBorder);
					if(!IsWindowEnabled(hwnd))
						nState = ETS_DISABLED;
					else if(SendMessage(hwnd, EM_GETOPTIONS, 0, 0) & ECO_READONLY)
						nState = ETS_READONLY;
					else nState = ETS_NORMAL;
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
			
			if (ru->hasUglyBorder&&MyIsThemeActive()) {
				HANDLE hTheme = MyOpenThemeData(hwnd, L"EDIT");

				if (hTheme) {
					RECT rcClient; 
					HDC hdc = GetDC(GetParent(hwnd));

					ZeroMemory(&rcClient, sizeof(RECT));
					if(MyGetThemeBackgroundContentRect(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &ncsParam->rgrc[0], &rcClient) == S_OK) {
						ru->rect.left = rcClient.left-ncsParam->rgrc[0].left;
						ru->rect.top = rcClient.top-ncsParam->rgrc[0].top;
						ru->rect.right = ncsParam->rgrc[0].right-rcClient.right;
						ru->rect.bottom = ncsParam->rgrc[0].bottom-rcClient.bottom;
						CopyRect(&ncsParam->rgrc[0], &rcClient);
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
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
			break;
		case WM_DESTROY:
		{
			LRESULT ret = CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);

			if(IsWindow(hwnd)) {
				if((WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC) == &RichUtil_Proc)
					SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ru->origProc);
			}
			EnterCriticalSection(&csRich);
			slist = rlist_remove(slist, ru);
			LeaveCriticalSection(&csRich);
			if (ru) free(ru);
			return ret;
		}
	}
	return CallWindowProc(ru->origProc, hwnd, msg, wParam, lParam);
}

static RichUtil_ClearUglyBorder(TRichUtil *ru) {
	if (mTheme&&MyIsThemeActive()&&GetWindowLongPtr(ru->hwnd, GWL_EXSTYLE)&WS_EX_CLIENTEDGE) {
		ru->hasUglyBorder = 1;
		SetWindowLongPtr(ru->hwnd, GWL_EXSTYLE, GetWindowLongPtr(ru->hwnd, GWL_EXSTYLE)^WS_EX_CLIENTEDGE);
	}
	// Redraw window since the style may have changed
	SetWindowPos(ru->hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
	RedrawWindow(ru->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
}
