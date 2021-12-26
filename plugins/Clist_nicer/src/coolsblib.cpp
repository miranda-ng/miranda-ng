/*
	
	Cool Scrollbar Library Version 1.2

	Module: coolsblib.c
	Copyright (c) J Brown 2001

	This code is freeware, however, you may not publish
	this code elsewhere or charge any money for it. This code
	is supplied as-is. I make no guarantees about the suitability
	of this code - use at your own risk.
	
	It would be nice if you credited me, in the event
	that you use this code in a product.

	VERSION HISTORY:

	 V1.2: TreeView problem fixed by Diego Tartara
		   Small problem in thumbsize calculation also fixed (thanks Diego!)

	 V1.1: Added support for Right-left windows
	       Changed calling convention of APIs to WINAPI (__stdcall)
		   Completely standalone (no need for c-runtime)

	 V1.0: Apr 2001: Initial Version

*/

#include "stdafx.h"
#include "coolscroll.h"
#include "userdefs.h"
#include "coolsb_internal.h"

static wchar_t szPropStr[] = L"CoolSBSubclassPtr";

LRESULT CALLBACK CoolSBWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

SCROLLWND* GetScrollWndFromHwnd(HWND hwnd)
{
	return (SCROLLWND *)GetProp(hwnd, szPropStr);
}

SCROLLBAR* GetScrollBarFromHwnd(HWND hwnd, UINT nBar)
{
	SCROLLWND *sw = GetScrollWndFromHwnd(hwnd);
	if (!sw)
		return nullptr;

	if (nBar == SB_HORZ)
		return &sw->sbarHorz;
	if (nBar == SB_VERT)
		return &sw->sbarVert;
	return nullptr;
}

BOOL WINAPI CoolSB_IsCoolScrollEnabled(HWND hwnd)
{
	if (GetScrollWndFromHwnd(hwnd))
		return TRUE;
	return FALSE;
}

BOOL GetScrollRect(SCROLLWND *sw, UINT nBar, HWND hwnd, RECT *rect);

//
//	Special support for USER32.DLL patching (using Detours library)
//	The only place we call a real scrollbar API is in InitializeCoolSB,
//	where we call EnableScrollbar.
//	
//	We HAVE to call the origial EnableScrollbar function, 
//	so we need to be able to set a pointer to this func when using
//	using Detours (or any other LIB??)
//

typedef BOOL(WINAPI *WPROC)(HWND, UINT, UINT);

static WPROC pEnableScrollBar = nullptr;

static void RedrawNonClient(HWND hwnd, BOOL fFrameChanged)
{
	if (fFrameChanged == FALSE) {
		/*
		RECT rect;
		HRGN hrgn1, hrgn2;

		SCROLLWND *sw = GetScrollWndFromHwnd(hwnd);

		GetScrollRect(sw, SB_HORZ, hwnd, &rect);
		hrgn1 = CreateRectRgnIndirect(&rect);

		GetScrollRect(sw, SB_VERT, hwnd, &rect);
		hrgn2 = CreateRectRgnIndirect(&rect);

		CombineRgn(hrgn1, hrgn2, hrgn1, RGN_OR);

		SendMessage(hwnd, WM_NCPAINT, (WPARAM)hrgn1, 0);

		DeleteObject(hrgn1);
		DeleteObject(hrgn2);*/

		SendMessage(hwnd, WM_NCPAINT, 1, 0);
	}
	else {
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
			| SWP_FRAMECHANGED | SWP_DRAWFRAME);
	}
}

//
//	return the default minimum size of a scrollbar thumb
//
int WINAPI CoolSB_GetDefaultMinThumbSize(void)
{
	uint32_t dwVersion = GetVersion();

	// set the minimum thumb size for a scrollbar. This
	// differs between NT4 and 2000, so need to check to see
	// which platform we are running under
	if (dwVersion < 0x80000000)              // Windows NT/2000
	{
		if (LOBYTE(LOWORD(dwVersion)) >= 5)
			return MINTHUMBSIZE_2000;
		return MINTHUMBSIZE_NT4;
	}

	return MINTHUMBSIZE_NT4;
}

static SCROLLINFO *GetScrollInfoFromHwnd(HWND hwnd, int fnBar)
{
	SCROLLBAR *sb = GetScrollBarFromHwnd(hwnd, fnBar);

	if (sb == nullptr)
		return FALSE;

	if (fnBar == SB_HORZ) {
		return &sb->scrollInfo;
	}
	else if (fnBar == SB_VERT) {
		return &sb->scrollInfo;
	}
	else
		return nullptr;
}
//
//	Initialize the cool scrollbars for a window by subclassing it
//	and using the coolsb window procedure instead
//
BOOL WINAPI InitializeCoolSB(HWND hwnd)
{
	SCROLLWND *sw;
	SCROLLINFO *si;
	INITCOMMONCONTROLSEX ice;
	TOOLINFO ti;
	RECT rect;
	uint32_t dwCurStyle;
	//BOOL fDisabled;

	if (pEnableScrollBar == nullptr)
		pEnableScrollBar = EnableScrollBar;

	GetClientRect(hwnd, &rect);

	//if we have already initialized Cool Scrollbars for this window,
	//then stop the user from doing it again
	if (GetScrollWndFromHwnd(hwnd) != nullptr) {
		return FALSE;
	}

	//allocate a private scrollbar structure which we 
	//will use to keep track of the scrollbar data
	sw = (SCROLLWND *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SCROLLWND));

	si = &sw->sbarHorz.scrollInfo;
	si->cbSize = sizeof(SCROLLINFO);
	si->fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_HORZ, si);

	si = &sw->sbarVert.scrollInfo;
	si->cbSize = sizeof(SCROLLINFO);
	si->fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, si);

	//check to see if the window has left-aligned scrollbars
	if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_LEFTSCROLLBAR)
		sw->fLeftScrollbar = TRUE;
	else
		sw->fLeftScrollbar = FALSE;

	dwCurStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

	SetProp(hwnd, szPropStr, (HANDLE)sw);

	//try to enable the scrollbar arrows - if the return value is
	//non-zero, then the scrollbars were previously disabled
	//fDisabled = pEnableScrollBar(hwnd, SB_HORZ, ESB_ENABLE_BOTH);

	//scrollbars will automatically get enabled, even if
	//they aren't to start with....sorry, but there isn't an
	//easy alternative.
	if (dwCurStyle & WS_HSCROLL)
		sw->sbarHorz.fScrollFlags = CSBS_VISIBLE;

	if (dwCurStyle & WS_VSCROLL)
		sw->sbarVert.fScrollFlags = CSBS_VISIBLE;

	//need to be able to distinguish between horizontal and vertical
	//scrollbars in some instances
	sw->sbarHorz.nBarType = SB_HORZ;
	sw->sbarVert.nBarType = SB_VERT;

	sw->sbarHorz.fFlatScrollbar = CSBS_NORMAL;
	sw->sbarVert.fFlatScrollbar = CSBS_NORMAL;

	//set the default arrow sizes for the scrollbars
	sw->sbarHorz.nArrowLength = SYSTEM_METRIC;
	sw->sbarHorz.nArrowWidth = SYSTEM_METRIC;
	sw->sbarVert.nArrowLength = SYSTEM_METRIC;
	sw->sbarVert.nArrowWidth = SYSTEM_METRIC;

	sw->bPreventStyleChange = FALSE;

	mir_subclassWindow(hwnd, CoolSBWndProc);

	CoolSB_SetMinThumbSize(hwnd, SB_BOTH, CoolSB_GetDefaultMinThumbSize());

#ifdef COOLSB_TOOLTIPS
	ice.dwSize = sizeof(ice);
	ice.dwICC  = ICC_BAR_CLASSES;
	InitCommonControlsEx(&ice);

	sw->hwndToolTip = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, TOOLTIPS_CLASS, L"",
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwnd, NULL, GetModuleHandle(0),
		NULL);

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND;
	ti.hwnd   = hwnd;
	ti.uId    = (UINT)hwnd;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	ti.hinst   = GetModuleHandle(0);

	SendMessage(sw->hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

#else

	UNREFERENCED_PARAMETER(ice);
	UNREFERENCED_PARAMETER(ti);
	sw->hwndToolTip = nullptr;

#endif

	//send the window a frame changed message to update the scrollbars
	RedrawNonClient(hwnd, TRUE);

	return TRUE;
}

BOOL WINAPI CoolSB_EnableScrollBar(HWND hwnd, int wSBflags, UINT wArrows)
{
	SCROLLBAR *sbar;
	UINT oldstate;
	BOOL bFailed = FALSE;

	if (!CoolSB_IsCoolScrollEnabled(hwnd))
		return EnableScrollBar(hwnd, wSBflags, wArrows);

	if ((wSBflags == SB_HORZ || wSBflags == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ))) {
		oldstate = sbar->fScrollFlags;

		//clear any existing state, and OR in the disabled flags
		sbar->fScrollFlags = (sbar->fScrollFlags & ~ESB_DISABLE_BOTH) | wArrows;

		if (oldstate == sbar->fScrollFlags)
			bFailed = TRUE;

	}

	if ((wSBflags == SB_VERT || wSBflags == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT))) {
		oldstate = sbar->fScrollFlags;

		//clear any existing state, and OR in the disabled flags
		sbar->fScrollFlags = (sbar->fScrollFlags & ~ESB_DISABLE_BOTH) | wArrows;

		if (oldstate == sbar->fScrollFlags)
			bFailed = TRUE;
	}

	return !bFailed;
}

BOOL WINAPI CoolSB_GetScrollInfo(HWND hwnd, int fnBar, LPSCROLLINFO lpsi)
{
	BOOL copied = FALSE;

	if (!lpsi)
		return FALSE;

	SCROLLINFO *mysi = GetScrollInfoFromHwnd(hwnd, fnBar);
	if (!mysi)
		return GetScrollInfo(hwnd, fnBar, lpsi);

	if (lpsi->fMask & SIF_PAGE) {
		lpsi->nPage = mysi->nPage;
		copied = TRUE;
	}

	if (lpsi->fMask & SIF_POS) {
		lpsi->nPos = mysi->nPos;
		copied = TRUE;
	}

	if (lpsi->fMask & SIF_TRACKPOS) {
		lpsi->nTrackPos = mysi->nTrackPos;
		copied = TRUE;
	}

	if (lpsi->fMask & SIF_RANGE) {
		lpsi->nMin = mysi->nMin;
		lpsi->nMax = mysi->nMax;
		copied = TRUE;
	}

	return copied;
}

int	WINAPI CoolSB_GetScrollPos(HWND hwnd, int nBar)
{
	SCROLLINFO *mysi = GetScrollInfoFromHwnd(hwnd, nBar);
	if (!mysi)
		return GetScrollPos(hwnd, nBar);

	return mysi->nPos;
}

BOOL WINAPI CoolSB_GetScrollRange(HWND hwnd, int nBar, LPINT lpMinPos, LPINT lpMaxPos)
{
	if (!lpMinPos || !lpMaxPos)
		return FALSE;

	SCROLLINFO *mysi = GetScrollInfoFromHwnd(hwnd, nBar);
	if (!mysi)
		return GetScrollRange(hwnd, nBar, lpMinPos, lpMaxPos);

	*lpMinPos = mysi->nMin;
	*lpMaxPos = mysi->nMax;

	return TRUE;
}

int	WINAPI CoolSB_SetScrollInfo(HWND hwnd, int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw)
{
	BOOL fRecalcFrame = FALSE;

	if (!lpsi)
		return FALSE;

	SCROLLINFO *mysi = GetScrollInfoFromHwnd(hwnd, fnBar);
	if (!mysi)
		return SetScrollInfo(hwnd, fnBar, lpsi, fRedraw);

	if (lpsi->fMask & SIF_RANGE) {
		mysi->nMin = lpsi->nMin;
		mysi->nMax = lpsi->nMax;
	}

	//The nPage member must specify a value from 0 to nMax - nMin +1. 
	if (lpsi->fMask & SIF_PAGE) {
		UINT t = (UINT)(mysi->nMax - mysi->nMin + 1);
		mysi->nPage = min(lpsi->nPage, t);
	}

	//The nPos member must specify a value between nMin and nMax - max(nPage - 1, 0).
	if (lpsi->fMask & SIF_POS) {
		mysi->nPos = max(lpsi->nPos, mysi->nMin);
		mysi->nPos = min((UINT)mysi->nPos, mysi->nMax - max(mysi->nPage - 1, 0));
	}

	SCROLLBAR *sbar = GetScrollBarFromHwnd(hwnd, fnBar);
	if (sbar == nullptr)
		return 0;

	if ((lpsi->fMask & SIF_DISABLENOSCROLL) || (sbar->fScrollFlags & CSBS_THUMBALWAYS)) {
		if (!sbar->fScrollVisible) {
			CoolSB_ShowScrollBar(hwnd, fnBar, TRUE);
			fRecalcFrame = TRUE;
		}
	}
	else {
		if (mysi->nPage > (UINT)mysi->nMax
			|| mysi->nPage == (UINT)mysi->nMax && mysi->nMax == 0
			|| mysi->nMax <= mysi->nMin) {
			if (sbar->fScrollVisible) {
				CoolSB_ShowScrollBar(hwnd, fnBar, FALSE);
				fRecalcFrame = TRUE;
			}
		}
		else {
			if (!sbar->fScrollVisible) {
				CoolSB_ShowScrollBar(hwnd, fnBar, TRUE);
				fRecalcFrame = TRUE;
			}
		}
	}

	if (fRedraw && !CoolSB_IsThumbTracking(hwnd))
		RedrawNonClient(hwnd, fRecalcFrame);

	return mysi->nPos;
}


int WINAPI CoolSB_SetScrollPos(HWND hwnd, int nBar, int nPos, BOOL fRedraw)
{
	int oldpos;

	SCROLLINFO *mysi = GetScrollInfoFromHwnd(hwnd, nBar);
	if (!mysi)
		return SetScrollPos(hwnd, nBar, nPos, fRedraw);

	//validate and set the scollbar position
	oldpos = mysi->nPos;
	mysi->nPos = max(nPos, mysi->nMin);
	mysi->nPos = min((UINT)mysi->nPos, mysi->nMax - max(mysi->nPage - 1, 0));

	if (fRedraw && !CoolSB_IsThumbTracking(hwnd))
		RedrawNonClient(hwnd, FALSE);

	return oldpos;
}

int WINAPI CoolSB_SetScrollRange(HWND hwnd, int nBar, int nMinPos, int nMaxPos, BOOL fRedraw)
{
	SCROLLINFO *mysi = GetScrollInfoFromHwnd(hwnd, nBar);
	if (!mysi)
		return SetScrollRange(hwnd, nBar, nMinPos, nMaxPos, fRedraw);

	if (CoolSB_IsThumbTracking(hwnd))
		return mysi->nPos;

	//hide the scrollbar if nMin == nMax
	//nMax-nMin must not be greater than MAXLONG
	mysi->nMin = nMinPos;
	mysi->nMax = nMaxPos;

	if (fRedraw)
		RedrawNonClient(hwnd, FALSE);

	return TRUE;
}

//
//	Show or hide the specified scrollbars
//
BOOL WINAPI CoolSB_ShowScrollBar(HWND hwnd, int wBar, BOOL fShow)
{
	SCROLLBAR *sbar;
	BOOL bFailed = FALSE;
	uint32_t dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

	if (!CoolSB_IsCoolScrollEnabled(hwnd))
		return ShowScrollBar(hwnd, wBar, fShow);

	if ((wBar == SB_HORZ || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ))) {
		sbar->fScrollFlags = sbar->fScrollFlags & ~CSBS_VISIBLE;
		sbar->fScrollFlags |= fShow ? CSBS_VISIBLE : 0;
		//bFailed = TRUE;

		if (fShow)	SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle | WS_HSCROLL);
		else		SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~WS_HSCROLL);
	}

	if ((wBar == SB_VERT || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT))) {
		sbar->fScrollFlags = sbar->fScrollFlags & ~CSBS_VISIBLE;
		sbar->fScrollFlags |= fShow ? CSBS_VISIBLE : 0;
		//bFailed = TRUE;

		if (fShow)	SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle | WS_VSCROLL);
		else		SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~WS_VSCROLL);
	}

	if (bFailed) {
		return FALSE;
	}
	else {
		//uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
		//style |= WS_VSCROLL;

		//if (s
		//SetWindowLongPtr(hwnd, GWL_STYLE, style);

		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOACTIVATE | SWP_FRAMECHANGED);

		return TRUE;
	}
}

//
//	Remove cool scrollbars from the specified window.
//
void WINAPI UninitializeCoolSB(HWND hwnd)
{
	SCROLLWND *sw = GetScrollWndFromHwnd(hwnd);
	if (!sw)
		return;

	RemoveProp(hwnd, szPropStr);
	//SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);

	//finally, release the memory needed for the cool scrollbars
	HeapFree(GetProcessHeap(), 0, sw);

	//Force WM_NCCALCSIZE and WM_NCPAINT so the original scrollbars can kick in
	RedrawNonClient(hwnd, TRUE);
}

//
//	Set the size of the scrollbars
//
BOOL WINAPI CoolSB_SetSize(HWND hwnd, int wBar, int nLength, int nWidth)
{
	SCROLLBAR *sbar;

	if (nLength == 0 || nWidth == 0)
		return FALSE;

	if (nLength < -8 || nWidth < -8)
		return FALSE;

	if (nLength > 256 || nWidth > 256)
		return FALSE;

	if (!GetScrollWndFromHwnd(hwnd))
		return FALSE;

	if ((wBar == SB_HORZ || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ))) {
		sbar->nArrowLength = nLength;
		sbar->nArrowWidth = nWidth;
	}

	if ((wBar == SB_VERT || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT))) {
		sbar->nArrowLength = nLength;
		sbar->nArrowWidth = nWidth;
	}

	RedrawNonClient(hwnd, TRUE);
	return TRUE;
}

//
//	Alter the display mode of the scrollbars
//	wBar   - SB_HORZ / SB_VERT / SB_BOTH
//	nStyle - CSBF_NORMAL / CSBF_FLAT / CSBF_HOTTRACKED
//
BOOL WINAPI CoolSB_SetStyle(HWND hwnd, int wBar, UINT nStyle)
{
	SCROLLBAR *sbar;

	if (!GetScrollWndFromHwnd(hwnd))
		return FALSE;

	if ((wBar == SB_HORZ || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ)))
		sbar->fFlatScrollbar = nStyle;

	if ((wBar == SB_VERT || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT)))
		sbar->fFlatScrollbar = nStyle;

	RedrawNonClient(hwnd, FALSE);
	return TRUE;
}

//
//	Set if the thumb is always visible, even if there is no data to
//  scroll. Setting this keeps the scrollbar enabled, but the thumb
//  covers the whole area
//	
BOOL WINAPI CoolSB_SetThumbAlways(HWND hwnd, int wBar, BOOL fThumbAlways)
{
	SCROLLBAR *sbar;

	if (!GetScrollWndFromHwnd(hwnd))
		return FALSE;

	if ((wBar == SB_HORZ || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ))) {
		if (fThumbAlways)
			sbar->fScrollFlags |= CSBS_THUMBALWAYS;
		else
			sbar->fScrollFlags &= ~CSBS_THUMBALWAYS;
	}

	if ((wBar == SB_VERT || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT))) {
		if (fThumbAlways)
			sbar->fScrollFlags |= CSBS_THUMBALWAYS;
		else
			sbar->fScrollFlags &= ~CSBS_THUMBALWAYS;
	}

	RedrawNonClient(hwnd, FALSE);

	return TRUE;
}

//
//	Set the minimum size, in pixels, that the thumb box will shrink to.
//
BOOL WINAPI CoolSB_SetMinThumbSize(HWND hwnd, UINT wBar, UINT size)
{
	SCROLLBAR *sbar;

	if (!GetScrollWndFromHwnd(hwnd))
		return FALSE;

	if (size == -1)
		size = CoolSB_GetDefaultMinThumbSize();

	if ((wBar == SB_HORZ || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ)))
		sbar->nMinThumbSize = size;

	if ((wBar == SB_VERT || wBar == SB_BOTH) && (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT)))
		sbar->nMinThumbSize = size;

	return TRUE;
}
