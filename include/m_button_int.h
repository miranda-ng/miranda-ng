/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_BUTTON_INT_H__
#define M_BUTTON_INT_H__ 1

#include <m_button.h>

interface IAccPropServices;

typedef void (__cdecl *pfnPainterFunc)(struct MButtonCtrl*, HDC);

struct MButtonCtrl
{
	HWND     hwnd;
	int      stateId; // button state
	int      focus;   // has focus (1 or 0)
	HFONT    hFont;   // font
	HICON    arrow;   // uses down arrow
	HICON    hIcon;
	HBITMAP  hBitmap;
	HTHEME   hThemeButton;
	HTHEME   hThemeToolbar;
	TCHAR    cHot;
	HWND     hwndToolTips;

	bool     bIsPushBtn,  // button has two states
				bIsPushed,   // is button pushed or not
				bIsDefault,  // default button
				bIsFlat,     // flat button
				bIsThemed,   // themed button
				bIsSkinned,  // skinned button
				bSendOnDown; // sends BN_CLICKED on WM_LBUTTONDOWN

	pfnPainterFunc fnPainter; // custom button painter

	IAccPropServices* pAccPropServices;
};

#endif // M_BUTTON_INT_H__
