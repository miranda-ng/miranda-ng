{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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
}

{$IFNDEF M_BUTTON_INT}
{$DEFINE M_BUTTON_INT}

type
  IAccPropServices = interface
  end;

type
  pMButtonCtrl = pointer;
  pfnPainterFunc = procedure (mb:pMButtonCtrl; dc:HDC); cdecl;

type
  tMButtonCtrl = record
    hwnd:HWND;
    stateId:int; // button state
    focus:int;   // has focus (1 or 0)
    hFont:HFONT; // font
    arrow:HICON; // uses down arrow
    hIcon:HICON;
    hBitmap:HBITMAP;
    hThemeButton:THANDLE;  // HTHEME
    hThemeToolbar:THANDLE; // HTHEME
    cHot:WideChar;
    hwndToolTips:HWND;

    bIsPushBtn,  // button has two states
    bIsPushed,   // is button pushed or not
    bIsDefault,  // default button
    bIsFlat,     // flat button
    bIsThemed,   // themed button
    bIsSkinned,  // skinned button
    bSendOnDown: // sends BN_CLICKED on WM_LBUTTONDOWN
    ByteBool;
    fnPainter:pfnPainterFunc; // custom button painter

    pAccPropServices:^IAccPropServices;
  end;

{$ENDIF}
