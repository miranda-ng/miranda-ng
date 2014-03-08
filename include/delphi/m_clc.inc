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
  CLISTCONTROL_CLASS = 'CListControl';

  // styles

  CLS_MANUALUPDATE    = $0001; // todo
  CLS_SHOWHIDDEN      = $0002;
  CLS_HIDEOFFLINE     = $0004; // hides all offline users
  CLS_CHECKBOXES      = $0008;
  CLS_MULTICOLUMN     = $0010; // not true multi-column, just for ignore/vis options
  CLS_HIDEEMPTYGROUPS = $0020; // note: this flag will be spontaneously removed if the 'new subgroup' menu item is clicked, for obvious reasons
  CLS_USEGROUPS       = $0040;
  CLS_NOHIDEOFFLINE   = $0080; // overrides CLS_HIDEOFFLINE and the per-group hideoffline setting
  CLS_GREYALTERNATE   = $0100; // make every other line slightly grey
  CLS_GROUPCHECKBOXES = $0200; // put checkboxes on groups too (managed by CLC)
  CLS_CONTACTLIST     = $0400; // this control will be the main contact list (v. 0.3.4.3+ 2004/11/02)

  CLS_EX_DISABLEDRAGDROP     = $00000001;
  CLS_EX_EDITLABELS          = $00000002;
  CLS_EX_SHOWSELALWAYS       = $00000004;
  CLS_EX_TRACKSELECT         = $00000008;
  CLS_EX_SHOWGROUPCOUNTS     = $00000010;
  CLS_EX_DIVIDERONOFF        = $00000020;
  CLS_EX_HIDECOUNTSWHENEMPTY = $00000040;
  CLS_EX_NOTRANSLUCENTSEL    = $00000080;
  CLS_EX_LINEWITHGROUPS      = $00000100;
  CLS_EX_QUICKSEARCHVISONLY  = $00000200;
  CLS_EX_SORTGROUPSALPHA     = $00000400;
  CLS_EX_NOSMOOTHSCROLLING   = $00000800;

  CLM_FIRST = $1000; // this is the same as LVM_FIRST
  CLM_LAST  = $1100;

// messages, compare with equivalent TVM_* in the WINAPI

  CLM_ADDCONTACT      = (CLM_FIRST+0); // wParam=hContact
  CLM_ADDGROUP        = (CLM_FIRST+1); // wParam=hGroup
  CLM_AUTOREBUILD     = (CLM_FIRST+2);
  CLM_DELETEITEM      = (CLM_FIRST+3); // wParam=hItem
  CLM_EDITLABEL       = (CLM_FIRST+4); // wParam=hItem
  CLM_ENDEDITLABELNOW = (CLM_FIRST+5); // wParam=cancel, 0 to save
  CLM_ENSUREVISIBLE   = (CLM_FIRST+6); // wParam=hItem, lParam=partialOk

  CLE_TOGGLE   = -1;
  CLE_COLLAPSE = 0;
  CLE_EXPAND   = 1;
  CLE_INVALID  = $FFFF;

  CLM_EXPAND            = (CLM_FIRST+7);  // wParam=hItem, lParam=CLE_
  CLM_FINDCONTACT       = (CLM_FIRST+8);  // wParam=hContact, returns an hItem
  CLM_FINDGROUP         = (CLM_FIRST+9);  // wParam=hGroup, returns an hItem
  CLM_GETBKCOLOR        = (CLM_FIRST+10); // returns a COLORREF
  CLM_GETCHECKMARK      = (CLM_FIRST+11); // wParam=hItem, returns 1 or 0
  CLM_GETCOUNT          = (CLM_FIRST+12); // returns the total number of items

  CLM_GETEDITCONTROL    = (CLM_FIRST+13); // returns the HWND, or NULL
  CLM_GETEXPAND         = (CLM_FIRST+14); // wParam=hItem, returns a CLE_, CLE_INVALID if not a group
  CLM_GETEXTRACOLUMNS   = (CLM_FIRST+15); // returns number of extra columns
  CLM_GETEXTRAIMAGE     = (CLM_FIRST+16); // wParam=hItem, lParam=MAKELPARAM(iColumn (0 based),0), returns iImage or EMPTY_EXTRA_ICON
  CLM_GETEXTRAIMAGELIST = (CLM_FIRST+17); // returns HIMAGELIST
  CLM_GETFONT           = (CLM_FIRST+18); // wParam=fontId, see clm_setfont. returns hFont.
  CLM_GETINDENT         = (CLM_FIRST+19); // wParam=new group indent
  CLM_GETISEARCHSTRING  = (CLM_FIRST+20); // lParam=(AnsiChar*)pszStr, max 120 bytes, returns number of chars in string
  CLM_GETITEMTEXT       = (CLM_FIRST+21); // wParam=hItem, lParam=(TChar*)pszStr, max 120 bytes
  CLM_GETSCROLLTIME     = (CLM_FIRST+22); // returns time in ms
  CLM_GETSELECTION      = (CLM_FIRST+23); // returns hItem
  CLM_SETEXTRASPACE     = (CLM_FIRST+24); // wParam=extra space between icons

  CLCHT_ABOVE        = $0001; // above client area
  CLCHT_BELOW        = $0002; // below client area
  CLCHT_TOLEFT       = $0004; // left of client area
  CLCHT_TORIGHT      = $0008; // right of client area
  CLCHT_NOWHERE      = $0010; // in client area, not on an item
  CLCHT_ONITEMICON   = $0020;
  CLCHT_ONITEMCHECK  = $0040;
  CLCHT_ONITEMLABEL  = $0080;
  CLCHT_ONITEMINDENT = $0100; // to the left of an item icon
  CLCHT_ONITEMEXTRA  = $0200; // on an extra icon, HIBYTE(HIWORD()) says which
  CLCHT_ONITEM       = $03E0;
  CLCHT_INLEFTMARGIN = $0400;
  CLCHT_BELOWITEMS   = $0800; // in client area but below last item

  CLM_HITTEST    = (CLM_FIRST+25); // lParam=MAKELPARAM(x,y) (relative to control), wParam=(PDWORD)&hitTest (see encoding of HitTest() in clc.h, can be NULL) returns hItem or NULL
  CLM_SELECTITEM = (CLM_FIRST+26); // wParam=hItem

  CLB_TOPLEFT  = 0;
  CLB_STRETCHV = 1;
  CLB_STRETCHH = 2; // and tile vertically
  CLB_STRETCH  = 3;

  CLBM_TYPE         = $00FF;
  CLBF_TILEH        = $1000;
  CLBF_TILEV        = $2000;
  CLBF_PROPORTIONAL = $4000;
  CLBF_SCROLL       = $8000;

  CLM_SETBKBITMAP       = (CLM_FIRST+27); // wParam=mode, lParam=hBitmap (don't delete it), NULL for none
  CLM_SETBKCOLOR        = (CLM_FIRST+28); // wParam=a COLORREF, default is GetSysColor(COLOR_3DFACE)
  CLM_SETCHECKMARK      = (CLM_FIRST+29); // wParam=hItem, lParam=1 or 0
  CLM_SETEXTRACOLUMNS   = (CLM_FIRST+30); // wParam = number of extra columns (zero to EXTRA_ICON_COUNT from clc.h, currently 16)
  CLM_SETEXTRAIMAGE     = (CLM_FIRST+31); // wParam=hItem, lParam=MAKELPARAM(iColumn (0 based),iImage). iImage=EMPTY_EXTRA_ICON is a blank
  CLM_SETEXTRAIMAGELIST = (CLM_FIRST+32); // lParam=HIMAGELIST

  FONTID_CONTACTS      = 0;
  FONTID_INVIS         = 1;
  FONTID_OFFLINE       = 2;
  FONTID_NOTONLIST     = 3;
  FONTID_GROUPS        = 4;
  FONTID_GROUPCOUNTS   = 5;
  FONTID_DIVIDERS      = 6;
  FONTID_OFFINVIS      = 7;
  FONTID_STATUSMSG     = 8;
  FONTID_GROUPSCLOSED  = 9;
  FONTID_CONTACTSHOVER = 10;
  FONTID_MAX           = 18;

  CLM_SETFONT            = (CLM_FIRST+33); // wParam=hFont, lParam=MAKELPARAM(fRedraw,fontId)
  CLM_SETINDENT          = (CLM_FIRST+34); // wParam=new indent, default is 3 pixels
  CLM_SETITEMTEXT        = (CLM_FIRST+35); // wParam=hItem, lParam=(AnsiChar*)pszNewText
  CLM_SETSCROLLTIME      = (CLM_FIRST+36); // wParam=time in ms, default 200
  CLM_SETHIDEEMPTYGROUPS = (CLM_FIRST+38); // wParam=TRUE/FALSE

  GREYF_UNFOCUS = $80000000;
  MODEF_OFFLINE = $40000000;

  // and use the PF2_ #defines from m_protosvc.inc
  CLM_SETGREYOUTFLAGS    = (CLM_FIRST+39); // wParam=new flags
  CLM_GETHIDEOFFLINEROOT = (CLM_FIRST+40); // returns TRUE/FALSE
  CLM_SETHIDEOFFLINEROOT = (CLM_FIRST+41); // wParam=TRUE/FALSE
  CLM_SETUSEGROUPS       = (CLM_FIRST+42); // wParam=TRUE/FALSE
  CLM_SETOFFLINEMODES    = (CLM_FIRST+43); // for 'hide offline', wParam=PF2_ flags and MODEF_OFFLINE
  CLM_GETEXSTYLE         = (CLM_FIRST+44); // returns CLS_EX_ flags
  CLM_SETEXSTYLE         = (CLM_FIRST+45); // wParam=CLS_EX_ flags
  CLM_GETLEFTMARGIN      = (CLM_FIRST+46); // returns count of pixels
  CLM_SETLEFTMARGIN      = (CLM_FIRST+47); // wParam=pixels

// the order of info items is never changed, so make sure you add them in the
// order you want them to remain
                                                                                      
const
  CLM_ADDINFOITEMA = (CLM_FIRST+48); // lParam=&TCLCINFOITEM, returns hItem
  CLM_GETITEMTYPE  = (CLM_FIRST+49); // wParam=hItem, returns a CLCIT_
  CLM_GETNEXTITEM  = (CLM_FIRST+50); // wParam=flag, lParam=hItem, returns an hItem
  CLM_GETTEXTCOLOR = (CLM_FIRST+51); // wParam=FONTID_, returns COLORREF
  CLM_SETTEXTCOLOR = (CLM_FIRST+52); // wParam=FONTID_, lParam=COLORREF

  CLM_ADDINFOITEMW = (CLM_FIRST+53); // lParam=&TCLCINFOITEM, returns hItem
  CLM_ADDINFOITEM = CLM_ADDINFOITEMW;

  CLCIIF_BELOWGROUPS   = 1;   // put it between groups and contacts, default is at top
  CLCIIF_BELOWCONTACTS = 2;   // put it at the bottom
  CLCIIF_CHECKBOX      = $40; // give this item a check box
  CLCIIF_GROUPFONT     = $80; // draw the item using FONTID_GROUPS

  CLCIT_INVALID = -1;
  CLCIT_GROUP   = 0;
  CLCIT_CONTACT = 1;
  CLCIT_DIVIDER = 2;
  CLCIT_INFO    = 3;

  CLGN_ROOT            = 0;
  CLGN_CHILD           = 1;
  CLGN_PARENT          = 2;
  CLGN_NEXT            = 3;
  CLGN_PREVIOUS        = 4;
  CLGN_NEXTCONTACT     = 5;
  CLGN_PREVIOUSCONTACT = 6;
  CLGN_NEXTGROUP       = 7;
  CLGN_PREVIOUSGROUP   = 8;

  CLNF_ISGROUP = 1;
  CLNF_ISINFO  = 2;

  CLN_FIRST          = (0-100);
  CLN_EXPANDED       = (CLN_FIRST-0);  // hItem=hGroup, action=CLE_*
  CLN_LISTREBUILT    = (CLN_FIRST-1);
  CLN_ITEMCHECKED    = (CLN_FIRST-2);  // todo  // hItem,action,flags valid
  CLN_DRAGGING       = (CLN_FIRST-3);  // hItem,pt,flags valid. only sent when cursor outside window, return nonzero if processed
  CLN_DROPPED        = (CLN_FIRST-4);  // hItem,pt,flags valid. only sent when cursor outside window, return nonzero if processed
  CLN_LISTSIZECHANGE = (CLN_FIRST-5);  // pt.y valid. the vertical height of the visible items in the list has changed.
  CLN_OPTIONSCHANGED = (CLN_FIRST-6);  // nothing valid. If you set some extended options they have been overwritten and should be re-set
  CLN_DRAGSTOP       = (CLN_FIRST-7);  // hItem,flags valid. sent when cursor goes back in to the window having been outside, return nonzero if processed
  CLN_NEWCONTACT     = (CLN_FIRST-8);  // hItem,flags valid. sent when a new contact is added without a full list rebuild
  CLN_CONTACTMOVED   = (CLN_FIRST-9);  // hItem,flags valid. sent when contact is moved without a full list rebuild
  CLN_CHECKCHANGED   = (CLN_FIRST-10); // hItem,flags valid. sent when any check mark is changed, but only for one change if there are many

// clist window tree messages
  M_CREATECLC        = WM_USER+1;
  M_SETALLEXTRAICONS = WM_USER+2;

type
  PCLCINFOITEM = ^TCLCINFOITEM;
  TCLCINFOITEM = record
    cbSize      : int;
    pszText     : TChar;
    hParentGroup: THANDLE;
    flags       : dword;
    hIcon       : HICON; // todo
  end;

  PNMCLISTCONTROL = ^TNMCLISTCONTROL;
  TNMCLISTCONTROL = record
    hdr    : TNMHDR;        // depends on Windows.pas
    hItem  : THANDLE;
    action : int;
    iColumn: int;       // -1 if not on an extra column
    flags  : dword;
    pt     : TPoint;         // depends on Windows.pas
  end;

  PCLCINFOTIP = ^TCLCINFOTIP;
  TCLCINFOTIP = record
    cbSize       : int;
    isTreeFocused: int;     // so the plugin can provide an option
    isGroup      : int;     // 0 if it's contact, 1 if it's a group
    hItem        : THANDLE; // handle to group or contact
    ptCursor     : TPOINT;
    rcItem       : TRECT;
  end;

  PCLCEXTRAINFOTIP = ^TCLCEXTRAINFOTIP;
  TCLCEXTRAINFOTIP = record
    cbSize       : int;
    isTreeFocused: int;     // so the plugin can provide an option
    hItem        : THANDLE; // handle to group or contact
    ptCursor     : TPOINT;
    rcItem       : TRECT;
    extraIndex   : int;
    hwnd         : HWND;
  end;

const
  {
    wParam : 0
    lParam : Pointer to a TCLCINFOTIP structure
    Affect : An InfoTip for an item should be shown now, see notes
    Returns: [non zero] if you process this, because it makes no sense
             for more than one module to process this.
    Notes  : It's upto the module where to put the InfoTip, Normally
             it's a few pixels below and to the right of the cursor.
             -
             This event is called after the mouse ehas been stationary over
             a contact for (by default) 200ms
  }
  ME_CLC_SHOWINFOTIP     :PAnsiChar = 'CLC/ShowInfoTip';
  ME_CLC_SHOWEXTRAINFOTIP:PAnsiChar = 'CLC/ShowExtraInfoTip';

  {
    wParam : 0
    lParam : Pointer to an initialised TCLCINFOTIP
    Affect : It's time to destroy an infotip, see notes
    Notes  : Only cbSize, isGroup, hItem are set
    notes  : This is sent when the mouse moves off a contact when ME_CLC_SHOWINFOTIP
             has previously been called.
             -
             If you don't want this behaviour, you should have grabbed the mouse
             capture yourself --
  }
  ME_CLC_HIDEINFOTIP:PAnsiChar = 'CLC/HideInfoTip';

  {
    wParam : new_time
    lParam : 0
    Affect : Set a new hover time before the info tip hooks are called, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : The value of this setting is applid to all current CLC windows
             and saved to b applied to all future windows, it is persistent.
             -
             Time is in milliseconds, default is 750ms
  }
  MS_CLC_SETINFOTIPHOVERTIME:PAnsiChar = 'CLC/SetInfoTipHoverTime';

  {
    wParam : 0
    lParam : 0
    Affect : get the hover time before the infotip hooks are called
    returns: the hover time in MS
  }
  MS_CLC_GETINFOTIPHOVERTIME:PAnsiChar = 'CLC/GetInfoTipHoverTime';

{$ENDIF}
