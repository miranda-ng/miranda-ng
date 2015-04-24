#if !defined(HISTORYSTATS_GUARD_BANDCTRLDEFS_H)
#define HISTORYSTATS_GUARD_BANDCTRLDEFS_H

#include "stdafx.h"

/*
 * BandCtrlDefs
 */

class BandCtrlDefs
{
public:
	enum Message {
		BCM_SETLAYOUT        = WM_USER +  0, // (int nLayout, #)               -> #
		BCM_ADDBUTTON        = WM_USER +  1, // (#, BCBUTTON* pButton)         -> HANDLE hButton
		BCM_ISBUTTONCHECKED  = WM_USER +  2, // (HANDLE hButton, #)            -> BOOL bChecked
		BCM_CHECKBUTTON      = WM_USER +  3, // (HANDLE hButton, BOOL bCheck)  -> #
		BCM_GETBUTTONDATA    = WM_USER +  4, // (HANDLE hButton, #)            -> INT_PTR dwData
		BCM_SETBUTTONDATA    = WM_USER +  5, // (HANDLE hButton, INT_PTR dwData) -> #
		BCM_ISBUTTONVISIBLE  = WM_USER +  6, // (HANDLE hButton, #)            -> BOOL bVisible
		BCM_SHOWBUTTON       = WM_USER +  7, // (HANDLE hButton, BOOL bShow)   -> #
		BCM_GETBUTTONRECT    = WM_USER +  8, // (HANDLE hButton, RECT* pRect)  -> #
		BCM_ISBUTTONENABLED  = WM_USER +  9, // (HANDLE hButton, #)            -> BOOL bEnabled
		BCM_ENABLEBUTTON     = WM_USER + 10, // (HANDLE hButton, BOOL bEnable) -> #
	};

	enum Notification {
		BCN_CLICKED  = NM_LAST - 1, // -> NMBANDCTRL
		BCN_DROP_DOWN = NM_LAST - 2  // -> NMBANDCTRL
	};

	enum ButtonFlags {
		BCF_RIGHT    = 0x001,
		BCF_CHECKED  = 0x002,
		BCF_HIDDEN   = 0x004,
		BCF_TOOLTIP  = 0x008,
		BCF_TEXT     = 0x010,
		BCF_ICON     = 0x020,
		BCF_DATA     = 0x040,
		BCF_DROPDOWN = 0x080,
		BCF_DISABLED = 0x100,
	};

	struct BCBUTTON {
		DWORD dwFlags;
		HICON hIcon;
		const TCHAR *m_szText;
		const TCHAR *m_szTooltip;
		INT_PTR dwData;
	};

	struct NMBANDCTRL {
		NMHDR hdr;
		HANDLE hButton;
		INT_PTR dwData;
	};
};

#endif // HISTORYSTATS_GUARD_BANDCTRLDEFS_H
