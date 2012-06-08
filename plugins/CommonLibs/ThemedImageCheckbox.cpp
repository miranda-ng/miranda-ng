/*
	ThemedImageCheckbox.cpp
	Copyright (c) 2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Common.h"
#include "ThemedImageCheckbox.h"
#include "Themes.h"
#include "win2k.h"

#define WM_THEMECHANGED 0x031A

#define CG_CHECKBOX_VERTINDENT 2
#define CG_CHECKBOX_INDENT 1
#define CG_CHECKBOX_WIDTH 16
#define CG_IMAGE_INDENT 7
#define CG_ADDITIONAL_WIDTH 3

// states
#define CGS_UNCHECKED BST_UNCHECKED
#define CGS_CHECKED BST_CHECKED
#define CGS_INDETERMINATE BST_INDETERMINATE
#define CGS_PRESSED BST_PUSHED // values above and including CGS_PRESSED must coincide with BST_ constants for BM_GETSTATE to work properly
#define CGS_HOVERED 8

// state masks
#define CGSM_ISCHECKED 3 // mask for BM_GETCHECK
#define CGSM_GETSTATE 7 // mask to get only valid values for BM_GETSTATE

#ifndef lengthof
#define lengthof(s) (sizeof(s) / sizeof(*s))
#endif

class CCheckboxData
{
public:
	CCheckboxData(): OldWndProc(NULL), Style(0), State(0), hBitmap(NULL), hIcon(NULL) {};

	WNDPROC OldWndProc;
	int Style; // BS_CHECKBOX, BS_AUTOCHECKBOX, BS_3STATE or BS_AUTO3STATE
	int State;
	HBITMAP hBitmap;
	HICON hIcon;
};

static int CALLBACK CheckboxWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CCheckboxData *dat = (CCheckboxData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!dat)
	{
		return 0;
	}
	switch (Msg)
	{
		case BM_CLICK:
		{
			SendMessage(hWnd, WM_LBUTTONDOWN, 0, 0);
			SendMessage(hWnd, WM_LBUTTONUP, 0, 0);
			return 0;
		} break;
		case BM_GETCHECK:
		{
			return dat->State & CGSM_ISCHECKED;
		} break;
		case BM_SETCHECK:
		{
			if ((wParam != BST_UNCHECKED && wParam != BST_CHECKED && wParam != BST_INDETERMINATE) || (wParam == BST_INDETERMINATE && dat->Style != BS_3STATE && dat->Style != BS_AUTO3STATE))
			{ // invalid value
				wParam = BST_CHECKED;
			}
			dat->State &= ~CGSM_ISCHECKED;
			dat->State |= wParam;
			InvalidateRect(hWnd, NULL, false);
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), BN_CLICKED), (LPARAM)hWnd);
			return 0;
		} break;
		case BM_SETSTATE:
		{
			if (wParam)
			{
				dat->State |= CGS_PRESSED;
			} else
			{
				dat->State &= ~CGS_PRESSED;
			}
			InvalidateRect(hWnd, NULL, false);
			return 0;
		} break;
		case BM_GETSTATE:
		{
			return (dat->State & CGSM_GETSTATE) | ((GetFocus() == hWnd) ? BST_FOCUS : 0);
		} break;
		case BM_SETIMAGE:
		{
			int PrevHandle = 0;
			switch (wParam)
			{
				case IMAGE_BITMAP:
				{
					PrevHandle = (int)dat->hBitmap;
					dat->hBitmap = (HBITMAP)lParam;
				} break;
				case IMAGE_ICON:
				{
					PrevHandle = (int)dat->hIcon;
					dat->hIcon = (HICON)lParam;
				} break;
				default:
				{
					return 0;
				}
			}
			InvalidateRect(hWnd, NULL, false);
			return PrevHandle;
		} break;
		case BM_GETIMAGE:
		{
			switch (wParam)
			{
				case IMAGE_BITMAP:
				{
					return (int)dat->hBitmap;
				} break;
				case IMAGE_ICON:
				{
					return (int)dat->hIcon;
				} break;
			}
			return 0;
		} break;
		case WM_GETDLGCODE:
		{
			return DLGC_BUTTON;
		} break;
		case WM_THEMECHANGED:
		case WM_ENABLE:
		{
			InvalidateRect(hWnd, NULL, false);
			return 0;
		} break;
		case WM_KEYDOWN:
		{
			if (wParam == VK_SPACE)
			{
				SendMessage(hWnd, BM_SETSTATE, true, 0);
			}
			return 0;
		} break;
		case WM_KEYUP:
		{
			if (wParam == VK_SPACE)
			{
				SendMessage(hWnd, BM_SETCHECK, (SendMessage(hWnd, BM_GETCHECK, 0, 0) + 1) % ((dat->Style == BS_AUTO3STATE) ? 3 : 2), 0);
				SendMessage(hWnd, BM_SETSTATE, false, 0);
			}
			return 0;
		} break;
		case WM_CAPTURECHANGED:
		{
			SendMessage(hWnd, BM_SETSTATE, false, 0);
			return 0;
		} break;
		case WM_ERASEBKGND:
		{
			return true;
		} break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			SetFocus(hWnd);
			SendMessage(hWnd, BM_SETSTATE, true, 0);
			SetCapture(hWnd);
			return 0;
		} break;
		case WM_LBUTTONUP:
		{
			if (GetCapture() == hWnd)
			{
				ReleaseCapture();
			}
			SendMessage(hWnd, BM_SETSTATE, false, 0);
			if (dat->State & CGS_HOVERED && (dat->Style == BS_AUTOCHECKBOX || dat->Style == BS_AUTO3STATE))
			{
				SendMessage(hWnd, BM_SETCHECK, (SendMessage(hWnd, BM_GETCHECK, 0, 0) + 1) % ((dat->Style == BS_AUTO3STATE) ? 3 : 2), 0);
			}
			return 0;
		} break;
		case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = hWnd;
			_TrackMouseEvent(&tme);

			POINT pt;
			GetCursorPos(&pt);
			if ((WindowFromPoint(pt) == hWnd) ^ ((dat->State & CGS_HOVERED) != 0))
			{
				dat->State ^= CGS_HOVERED;
				InvalidateRect(hWnd, NULL, false);
			}
			return 0;
		} break;
		case WM_MOUSELEAVE:
		{
			if (dat->State & CGS_HOVERED)
			{
				dat->State &= ~CGS_HOVERED;
				InvalidateRect(hWnd, NULL, false);
			}
			return 0;
		} break;
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_SYSCOLORCHANGE:
		{
			InvalidateRect(hWnd, NULL, false);
			return 0;
		} break;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);
			RECT rc;
			GetClientRect(hWnd, &rc);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
			HTHEME hTheme = pOpenThemeData ? pOpenThemeData(hWnd, L"BUTTON") : NULL;
			if (hTheme && pDrawThemeParentBackground)
			{
				pDrawThemeParentBackground(hWnd, hdcMem, NULL);
			} else
			{
				FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_3DFACE));
			}
			int StateID = 0;
#define CBSCHECK_UNCHECKED 1
#define CBSCHECK_CHECKED 5
#define CBSCHECK_MIXED 9
#define CBSSTATE_NORMAL 0
#define CBSSTATE_HOT 1
#define CBSSTATE_PRESSED 2
#define CBSSTATE_DISABLED 3
			switch (SendMessage(hWnd, BM_GETCHECK, 0, 0))
			{
				case BST_CHECKED:
				{
					StateID += CBSCHECK_CHECKED;
				} break;
				case BST_UNCHECKED:
				{
					StateID += CBSCHECK_UNCHECKED;
				} break;
				case BST_INDETERMINATE:
				{
					StateID += CBSCHECK_MIXED;
				} break;
			}
			if (!IsWindowEnabled(hWnd))
			{
				StateID += CBSSTATE_DISABLED;
			} else if (dat->State & CGS_PRESSED && (GetCapture() != hWnd || dat->State & CGS_HOVERED))
			{
				StateID += CBSSTATE_PRESSED;
			} else if (dat->State & CGS_PRESSED || dat->State & CGS_HOVERED)
			{
				StateID += CBSSTATE_HOT;
			}
			rc.left += CG_CHECKBOX_INDENT;
			rc.right = rc.left + CG_CHECKBOX_WIDTH; // left-align the image in the client area
			rc.top += CG_CHECKBOX_VERTINDENT;
			rc.bottom = rc.top + CG_CHECKBOX_WIDTH; // exact rc dimensions are necessary for DrawFrameControl to draw correctly
			if (hTheme && pDrawThemeBackground)
			{
				pDrawThemeBackground(hTheme, hdcMem, BP_CHECKBOX, StateID, &rc, &rc);
			} else
			{
				int dfcStates[] =
				 {0, 0, DFCS_PUSHED, DFCS_INACTIVE,
					DFCS_CHECKED, DFCS_CHECKED, DFCS_CHECKED | DFCS_PUSHED, DFCS_CHECKED | DFCS_INACTIVE,
					DFCS_BUTTON3STATE | DFCS_CHECKED, DFCS_BUTTON3STATE | DFCS_CHECKED, DFCS_BUTTON3STATE | DFCS_INACTIVE | DFCS_CHECKED | DFCS_PUSHED, DFCS_BUTTON3STATE | DFCS_INACTIVE | DFCS_CHECKED | DFCS_PUSHED};
				_ASSERT(StateID >= 1 && StateID <= lengthof(dfcStates));
				DrawFrameControl(hdcMem, &rc, DFC_BUTTON, dfcStates[StateID - 1]);
			}

			GetClientRect(hWnd, &rc);
			RECT rcImage = rc;
			LPARAM hImage = NULL;
			DWORD DSFlags;
			HIMAGELIST hImageList = NULL;
			if (dat->hBitmap)
			{
				BITMAP bminfo;
				GetObject(dat->hBitmap, sizeof(bminfo), &bminfo);
				rcImage.right = bminfo.bmWidth;
				rcImage.bottom = bminfo.bmHeight;
				DSFlags = DST_BITMAP;
				hImage = (LPARAM)dat->hBitmap;
			} else
			{
				rcImage.right = GetSystemMetrics(SM_CXSMICON);
				rcImage.bottom = GetSystemMetrics(SM_CYSMICON);
				DSFlags = DST_ICON;
				if (dat->hIcon)
				{
					hImageList = ImageList_Create(rcImage.right, rcImage.bottom, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 1, 0);
					ImageList_AddIcon(hImageList, dat->hIcon);
					hImage = (LPARAM)ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
				}
			} // rcImage.right and rcImage.bottom are width and height, not absolute coordinates
			rcImage.left += CG_CHECKBOX_INDENT + CG_CHECKBOX_WIDTH + CG_IMAGE_INDENT;
			rcImage.top += (rc.bottom - rcImage.bottom) / 2;
			DrawState(hdcMem, NULL, NULL, hImage, 0, rcImage.left, rcImage.top, rcImage.right, rcImage.bottom, DSFlags | (IsWindowEnabled(hWnd) ? DSS_NORMAL : DSS_DISABLED));
			if (hImageList)
			{
				ImageList_RemoveAll(hImageList);
				ImageList_Destroy(hImageList);
				DestroyIcon((HICON)hImage);
			}
			if (GetFocus() == hWnd)
			{
				rcImage.right += rcImage.left;
				rcImage.bottom += rcImage.top;
				InflateRect(&rcImage, 1, 1);
				DrawFocusRect(hdcMem, &rcImage);
			}
			if (hTheme && pCloseThemeData)
			{
				pCloseThemeData(hTheme);
			}
		  BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, hbmOld);
			DeleteObject(hbmMem);
			DeleteDC(hdcMem);
			EndPaint(hWnd, &ps);
			return 0;
		} break;
		case WM_DESTROY:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
			CallWindowProc(dat->OldWndProc, hWnd, Msg, wParam, lParam);
			delete dat;
			return 0;
		} break;
	}
	return CallWindowProc(dat->OldWndProc, hWnd, Msg, wParam, lParam);
}

int MakeThemedImageCheckbox(HWND hWndCheckbox)
{ // workaround to make checkbox with BS_ICON or BS_BITMAP work with windows themes enabled
	CCheckboxData *dat = new CCheckboxData();
	dat->OldWndProc = (WNDPROC)GetWindowLongPtr(hWndCheckbox, GWLP_WNDPROC);
	dat->State = SendMessage(hWndCheckbox, BM_GETSTATE, 0, 0);
	long Style = GetWindowLongPtr(hWndCheckbox, GWL_STYLE);
	_ASSERT(Style & BS_ICON || Style & BS_BITMAP);
	dat->Style = Style & (BS_CHECKBOX | BS_AUTOCHECKBOX | BS_3STATE | BS_AUTO3STATE);
	_ASSERT(dat->Style == BS_CHECKBOX || dat->Style == BS_AUTOCHECKBOX || dat->Style == BS_3STATE || dat->Style == BS_AUTO3STATE);
  Style &= ~(BS_CHECKBOX | BS_AUTOCHECKBOX | BS_3STATE | BS_AUTO3STATE);
  Style |= BS_OWNERDRAW;
	SetWindowLongPtr(hWndCheckbox, GWL_STYLE, Style);
	SetWindowLongPtr(hWndCheckbox, GWLP_USERDATA, (LONG)dat);
	SetWindowLongPtr(hWndCheckbox, GWLP_WNDPROC, (LONG)CheckboxWndProc);
	return 0;
}
