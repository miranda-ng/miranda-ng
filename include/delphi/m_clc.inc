{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

{$IFNDEF M_CLC}
{$DEFINE M_CLC}

const
  CLS_SHOWHIDDEN      = $0002;
  CLS_EX_SHOWSELALWAYS       = $00000004;
  CLM_FIRST = $1000; // this is the same as LVM_FIRST
  CLM_LAST  = $1100;

  CLM_FINDCONTACT       = (CLM_FIRST+8);  // wParam=hContact, returns an hItem
  CLM_GETCHECKMARK      = (CLM_FIRST+11); // wParam=hItem, returns 1 or 0
  CLM_GETSELECTION      = (CLM_FIRST+23); // returns hItem
  CLM_SETCHECKMARK      = (CLM_FIRST+29); // wParam=hItem, lParam=1 or 0
  CLM_SETHIDEEMPTYGROUPS = (CLM_FIRST+38); // wParam=TRUE/FALSE
  CLM_SETUSEGROUPS       = (CLM_FIRST+42); // wParam=TRUE/FALSE
  CLM_SETOFFLINEMODES    = (CLM_FIRST+43); // for 'hide offline', wParam=PF2_ flags and MODEF_OFFLINE
  CLM_GETEXSTYLE         = (CLM_FIRST+44); // returns CLS_EX_ flags
  CLM_SETEXSTYLE         = (CLM_FIRST+45); // wParam=CLS_EX_ flags
  CLN_FIRST          = (0-100);
  CLN_CHECKCHANGED   = (CLN_FIRST-10); // hItem,flags valid. sent when any check mark is changed, but only for one change if there are many
type
  PCLCINFOTIP = ^TCLCINFOTIP;
  TCLCINFOTIP = record
    cbSize       : int;
    isTreeFocused: int;     // so the plugin can provide an option
    isGroup      : int;     // 0 if it's contact, 1 if it's a group
    hItem        : THANDLE; // handle to group or contact
    ptCursor     : TPOINT;
    rcItem       : TRECT;
  end;


{$ENDIF}
