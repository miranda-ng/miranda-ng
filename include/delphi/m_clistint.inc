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

{$IFNDEF M_CLISTINT}
{$DEFINE M_CLISTINT}

const
  HCONTACT_ISGROUP = $80000000;
  HCONTACT_ISINFO  = $FFFF0000;
type
  PCLIST_INTERFACE = ^TCLIST_INTERFACE;
  TCLIST_INTERFACE = record
    hwndContactList : HWND;
    hwndContactTree : HWND;
    hwndStatus      : HWND;
    hMenuMain       : HMENU;
    hInst           : HMODULE;

  (* clc.h *)
    pfnContactListControlWndProc       : function(hwnd:HWND; msg:uint; wParam:WPARAM; lParam:LPARAM):LRESULT; stdcall;

  (* clcidents.c *)
    pfnFindItem                        : procedure; cdecl;

    pfnGetRowsPriorTo                  : procedure; cdecl;
    pfnGetRowByIndex                   : procedure; cdecl;
    pfnGetContactHiddenStatus          : procedure; cdecl;
    pfnIsVisibleContact                : procedure; cdecl;

  (* clcitems.c *)
    pfnCreateClcContact                : procedure; cdecl;
    pfnAddGroup                        : procedure; cdecl;

    pfnFreeContact                     : procedure; cdecl;

    pfnAddInfoItemToGroup              : procedure; cdecl;
    pfnAddItemToGroup                  : procedure; cdecl;
    pfnAddContactToGroup               : procedure; cdecl;

    pfnAddContactToTree                : procedure; cdecl;
    pfnRebuildEntireList               : procedure; cdecl;
    pfnGetGroupContentsCount           : procedure; cdecl;
    pfnSortCLC                         : procedure; cdecl;

  (* clcmsgs.c *)
    pfnProcessExternalMessages         : procedure; cdecl;

  (* clcpaint.c *)
    pfnPaintClc                        : procedure; cdecl;

  (* clcutils.c *)
    pfnHitTest                         : procedure; cdecl;
    pfnScrollTo                        : procedure; cdecl;
    pfnRecalcScrollBar                 : procedure; cdecl;
    pfnSetGroupExpand                  : procedure; cdecl;
    pfnFindRowByText                   : procedure; cdecl;
    pfnBeginRenameSelection            : procedure; cdecl;
    pfnGetDefaultFontSetting           : procedure; cdecl;
    pfnLoadClcOptions                  : procedure; cdecl;

    pfnGetRowBottomY                   : procedure; cdecl;
    pfnGetRowHeight                    : procedure; cdecl;
    pfnGetRowTopY                      : procedure; cdecl;
    pfnGetRowTotalHeight               : procedure; cdecl;
    pfnRowHitTest                      : procedure; cdecl;

  (* clistmod.c *)
    pfnIconFromStatusMode              : function (szProto:PAnsiChar; status:int; hContact:TMCONTACT):int; cdecl;
  end;

function Clist_GetInterface : PCLIST_INTERFACE; stdcall; external AppDll;

var
  cli : PCLIST_INTERFACE;

{$ENDIF}
