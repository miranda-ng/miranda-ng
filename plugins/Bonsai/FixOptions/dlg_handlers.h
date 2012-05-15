/*
Bonsai plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#ifndef M_BONSAI_DLGHANDLERS_H
#define M_BONSAI_DLGHANDLERS_H

#include "commonheaders.h"

#define ListView_GetItemTextW(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
{ LVITEMW _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.cchTextMax = cchTextMax_;\
  _ms_lvi.pszText = pszText_;\
  SNDMSG((hwndLV), LVM_GETITEMTEXTW, (WPARAM)(i), (LPARAM)(LVITEMW *)&_ms_lvi);\
}

#define ListView_SetItemTextW(hwndLV, i, iSubItem_, pszText_) \
{ LVITEMW _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.pszText = pszText_;\
  SNDMSG((hwndLV), LVM_SETITEMTEXTW, (WPARAM)(i), (LPARAM)(LVITEMW *)&_ms_lvi);\
}

#define ListView_InsertItemW(hwnd, pitem)   \
    (int)SNDMSG((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LVITEMW *)(pitem))

#define FOM_UPDATEPLUGINS (WM_USER + 10)

extern HWND hOptDlg;

INT_PTR CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif