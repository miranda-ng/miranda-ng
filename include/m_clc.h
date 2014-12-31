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

#ifndef M_CLC_H__
#define M_CLC_H__ 1

//This module is new in 0.1.2.1

#define CLISTCONTROL_CLASS  "CListControl"

//styles
#define CLS_MANUALUPDATE    0x0001	 //todo
#define CLS_SHOWHIDDEN      0x0002
#define CLS_HIDEOFFLINE     0x0004	 //hides all offline users
#define CLS_CHECKBOXES      0x0008
#define CLS_MULTICOLUMN	    0x0010   //not true multi-column, just for ignore/vis options
#define CLS_HIDEEMPTYGROUPS 0x0020   //note: this flag will be spontaneously removed if the 'new subgroup' menu item is clicked, for obvious reasons
#define CLS_USEGROUPS       0x0040
#define CLS_NOHIDEOFFLINE   0x0080	 //overrides CLS_HIDEOFFLINE and the per-group hideoffline setting
#define CLS_GREYALTERNATE   0x0100   //make every other line slightly grey
#define CLS_GROUPCHECKBOXES 0x0200   //put checkboxes on groups too (managed by CLC)
#define CLS_CONTACTLIST		0x0400   //this control will be the main contact list (v. 0.3.4.3+ 2004/11/02)

#define CLS_EX_DISABLEDRAGDROP     0x00000001
#define CLS_EX_EDITLABELS          0x00000002
#define CLS_EX_SHOWSELALWAYS       0x00000004
#define CLS_EX_TRACKSELECT         0x00000008
#define CLS_EX_SHOWGROUPCOUNTS     0x00000010
#define CLS_EX_DIVIDERONOFF        0x00000020
#define CLS_EX_HIDECOUNTSWHENEMPTY 0x00000040
#define CLS_EX_NOTRANSLUCENTSEL    0x00000080
#define CLS_EX_LINEWITHGROUPS      0x00000100
#define CLS_EX_QUICKSEARCHVISONLY  0x00000200
#define CLS_EX_SORTGROUPSALPHA     0x00000400
#define CLS_EX_NOSMOOTHSCROLLING   0x00000800

#define CLM_FIRST   0x1000    //this is the same as LVM_FIRST
#define CLM_LAST    0x1100

//messages, compare with equivalent TVM_s in the MSDN
#define CLM_ADDCONTACT        (CLM_FIRST+0)    //wParam = hContact
#define CLM_ADDGROUP          (CLM_FIRST+1)    //wParam = hGroup
#define CLM_AUTOREBUILD       (CLM_FIRST+2)
#define CLM_DELETEITEM        (CLM_FIRST+3)    //wParam = hItem
#define CLM_EDITLABEL         (CLM_FIRST+4)    //wParam = hItem
#define CLM_ENDEDITLABELNOW   (CLM_FIRST+5)    //wParam = cancel, 0 to save
#define CLM_ENSUREVISIBLE     (CLM_FIRST+6)    //wParam = hItem, lParam = partialOk
#define CLE_TOGGLE       -1
#define CLE_COLLAPSE     0
#define CLE_EXPAND       1
#define CLE_INVALID      0xFFFF
#define CLM_EXPAND            (CLM_FIRST+7)    //wParam = hItem, lParam = CLE_
#define CLM_FINDCONTACT       (CLM_FIRST+8)    //wParam = hContact, returns an hItem
#define CLM_FINDGROUP         (CLM_FIRST+9)    //wParam = hGroup, returns an hItem
#define CLM_GETBKCOLOR        (CLM_FIRST+10)   //returns a COLORREF
#define CLM_GETCHECKMARK      (CLM_FIRST+11)   //wParam = hItem, returns 1 or 0
#define CLM_GETCOUNT          (CLM_FIRST+12)   //returns the total number of items
#define CLM_GETEDITCONTROL    (CLM_FIRST+13)   //returns the HWND, or NULL
#define CLM_GETEXPAND         (CLM_FIRST+14)   //wParam = hItem, returns a CLE_, CLE_INVALID if not a group
#define CLM_GETEXTRACOLUMNS   (CLM_FIRST+15)   //returns number of extra columns
#define CLM_GETEXTRAIMAGE     (CLM_FIRST+16)   //wParam = hItem, lParam = MAKELPARAM(iColumn (0 based),0), returns iImage or EMPTY_EXTRA_ICON
#define CLM_GETEXTRAIMAGELIST (CLM_FIRST+17)   //returns HIMAGELIST
#define CLM_GETFONT           (CLM_FIRST+18)   //wParam = fontId, see clm_setfont. returns hFont.
#define CLM_GETINDENT         (CLM_FIRST+19)   //wParam = new group indent
#define CLM_GETISEARCHSTRING  (CLM_FIRST+20)   //lParam = (char*)pszStr, max 120 bytes, returns number of chars in string
#define CLM_GETITEMTEXT       (CLM_FIRST+21)   //wParam = hItem, lParam = (TCHAR*)pszStr, max 120 bytes
#define CLM_GETSCROLLTIME     (CLM_FIRST+22)   //returns time in ms
#define CLM_GETSELECTION      (CLM_FIRST+23)   //returns hItem
#define CLM_SETEXTRASPACE     (CLM_FIRST+24)   //wParam=extra space between icons

#define CLCHT_ABOVE        0x0001  //above client area
#define CLCHT_BELOW        0x0002  //below client area
#define CLCHT_TOLEFT       0x0004  //left of client area
#define CLCHT_TORIGHT      0x0008  //right of client area
#define CLCHT_NOWHERE      0x0010  //in client area, not on an item
#define CLCHT_ONITEMICON   0x0020
#define CLCHT_ONITEMCHECK  0x0040
#define CLCHT_ONITEMLABEL  0x0080
#define CLCHT_ONITEMINDENT 0x0100  //to the left of an item icon
#define CLCHT_ONITEMEXTRA  0x0200  //on an extra icon, HIBYTE(HIWORD()) says which
#define CLCHT_ONITEM       0x03E0
#define CLCHT_INLEFTMARGIN 0x0400
#define CLCHT_BELOWITEMS   0x0800  //in client area but below last item
#define CLM_HITTEST           (CLM_FIRST+25)   //lParam = MAKELPARAM(x,y) (relative to control), wParam = (PDWORD)&hitTest (see encoding of HitTest() in clc.h, can be NULL) returns hItem or NULL
#define CLM_SELECTITEM        (CLM_FIRST+26)   //wParam = hItem
#define CLB_TOPLEFT       0
#define CLB_STRETCHV      1
#define CLB_STRETCHH      2	 //and tile vertically
#define CLB_STRETCH       3
#define CLBM_TYPE         0x00FF
#define CLBF_TILEH        0x1000
#define CLBF_TILEV        0x2000
#define CLBF_PROPORTIONAL 0x4000
#define CLBF_SCROLL       0x8000
#define CLM_SETBKBITMAP       (CLM_FIRST+27)   //wParam = mode, lParam = hBitmap (don't delete it), NULL for none
#define CLM_SETBKCOLOR        (CLM_FIRST+28)   //wParam = a COLORREF, default is GetSysColor(COLOR_3DFACE)
#define CLM_SETCHECKMARK      (CLM_FIRST+29)   //wParam = hItem, lParam = 1 or 0
#define CLM_SETEXTRACOLUMNS   (CLM_FIRST+30)   //wParam = number of extra columns (zero to EXTRA_ICON_COUNT from clc.h, currently 16)
#define CLM_SETEXTRAIMAGE     (CLM_FIRST+31)   //wParam = hItem, lParam = MAKELPARAM(iColumn (0 based),iImage). iImage = EMPTY_EXTRA_ICON is a blank
#define CLM_SETEXTRAIMAGELIST (CLM_FIRST+32)   //lParam = HIMAGELIST

#define FONTID_CONTACTS    0
#define FONTID_INVIS       1
#define FONTID_OFFLINE     2
#define FONTID_NOTONLIST   3
#define FONTID_GROUPS      4
#define FONTID_GROUPCOUNTS 5
#define FONTID_DIVIDERS    6
#define FONTID_OFFINVIS    7
#define FONTID_STATUSMSG   8
#define FONTID_GROUPSCLOSED   9
#define FONTID_CONTACTSHOVER	10
#define FONTID_MAX				18

#define CLM_SETFONT           (CLM_FIRST+33)   //wParam = hFont, lParam = MAKELPARAM(fRedraw,fontId)
#define CLM_SETINDENT         (CLM_FIRST+34)   //wParam = new indent, default is 3 pixels
#define CLM_SETITEMTEXT       (CLM_FIRST+35)   //wParam = hItem, lParam = (char*)pszNewText
#define CLM_SETSCROLLTIME     (CLM_FIRST+36)   //wParam = time in ms, default 200

#define CLM_SETHIDEEMPTYGROUPS (CLM_FIRST+38)  //wParam = TRUE/FALSE
#define GREYF_UNFOCUS     0x80000000
#define MODEF_OFFLINE     0x40000000
//and use the PF2_ #defines from m_protosvc.h
#define CLM_SETGREYOUTFLAGS   (CLM_FIRST+39)   //wParam = new flags
#define CLM_GETHIDEOFFLINEROOT (CLM_FIRST+40)   //returns TRUE/FALSE
#define CLM_SETHIDEOFFLINEROOT (CLM_FIRST+41)   //wParam = TRUE/FALSE
#define CLM_SETUSEGROUPS      (CLM_FIRST+42)   //wParam = TRUE/FALSE
#define CLM_SETOFFLINEMODES   (CLM_FIRST+43)   //for 'hide offline', wParam = PF2_ flags and MODEF_OFFLINE
#define CLM_GETEXSTYLE        (CLM_FIRST+44)   //returns CLS_EX_ flags
#define CLM_SETEXSTYLE        (CLM_FIRST+45)   //wParam = CLS_EX_ flags
#define CLM_GETLEFTMARGIN     (CLM_FIRST+46)   //returns count of pixels
#define CLM_SETLEFTMARGIN     (CLM_FIRST+47)   //wParam = pixels
typedef struct {
	int cbSize;
	const TCHAR *pszText;
	HANDLE hParentGroup;
	DWORD flags;
	HICON hIcon;     //todo
} CLCINFOITEM;
#define CLCIIF_BELOWGROUPS    1     //put it between groups and contacts, default is at top
#define CLCIIF_BELOWCONTACTS  2     //put it at the bottom
#define CLCIIF_CHECKBOX       0x40  //give this item a check box
#define CLCIIF_GROUPFONT      0x80  //draw the item using FONTID_GROUPS

#define CLM_ADDINFOITEMA    (CLM_FIRST+48)   //lParam = &cii, returns hItem
#define CLM_ADDINFOITEMW    (CLM_FIRST+53)   //lParam = &cii, returns hItem
#if defined(_UNICODE)
	#define CLM_ADDINFOITEM CLM_ADDINFOITEMW
#else
	#define CLM_ADDINFOITEM CLM_ADDINFOITEMA
#endif

	//the order of info items is never changed, so make sure you add them in the
	//  order you want them to remain
#define CLCIT_INVALID    -1
#define CLCIT_GROUP    0
#define CLCIT_CONTACT  1
#define CLCIT_DIVIDER  2
#define CLCIT_INFO     3
#define CLM_GETITEMTYPE    (CLM_FIRST+49)	//wParam = hItem, returns a CLCIT_
#define CLGN_ROOT      0
#define CLGN_CHILD     1
#define CLGN_PARENT    2
#define CLGN_NEXT      3
#define CLGN_PREVIOUS  4
#define CLGN_NEXTCONTACT 5
#define CLGN_PREVIOUSCONTACT 6
#define CLGN_NEXTGROUP 7
#define CLGN_PREVIOUSGROUP 8

#define CLM_GETNEXTITEM    (CLM_FIRST+50)   //wParam = flag, lParam = hItem, returns an hItem
#define CLM_GETTEXTCOLOR   (CLM_FIRST+51)   //wParam = FONTID_, returns COLORREF
#define CLM_SETTEXTCOLOR   (CLM_FIRST+52)   //wParam = FONTID_, lParam = COLORREF

//notifications  (most are omitted because the control processes everything)
#define CLNF_ISGROUP   1
#define CLNF_ISINFO    2
typedef struct {
	NMHDR hdr;
	HANDLE hItem;
	int action;
	int iColumn;	//-1 if not on an extra column
	DWORD flags;
	POINT pt;
} NMCLISTCONTROL;
#define CLN_FIRST        (0U-100U)
#define CLN_EXPANDED     (CLN_FIRST-0)      //hItem = hGroup, action = CLE_*
#define CLN_LISTREBUILT  (CLN_FIRST-1)
#define CLN_ITEMCHECKED  (CLN_FIRST-2)      //todo	//hItem, action, flags valid
#define CLN_DRAGGING     (CLN_FIRST-3)		//hItem, pt, flags valid. only sent when cursor outside window, return nonzero if processed
#define CLN_DROPPED      (CLN_FIRST-4)		//hItem, pt, flags valid. only sent when cursor outside window, return nonzero if processed
#define CLN_LISTSIZECHANGE (CLN_FIRST-5)    //pt.y valid. the vertical height of the visible items in the list has changed.
#define CLN_OPTIONSCHANGED (CLN_FIRST-6)	//nothing valid. If you set some extended options they have been overwritten and should be re-set
#define CLN_DRAGSTOP     (CLN_FIRST-7)		//hItem, flags valid. sent when cursor goes back in to the window having been outside, return nonzero if processed
#define CLN_NEWCONTACT   (CLN_FIRST-8)      //hItem, flags valid. sent when a new contact is added without a full list rebuild
#define CLN_CONTACTMOVED (CLN_FIRST-9)      //hItem, flags valid. sent when contact is moved without a full list rebuild
#define CLN_CHECKCHANGED (CLN_FIRST-10)     //hItem, flags valid. sent when any check mark is changed, but only for one change if there are many
//NM_CLICK        							//hItem, iColumn, pt, flags valid
//NM_KEYDOWN                                //NMKEY structure, only sent when key is not already processed, return nonzero to prevent further processing

// clist window tree messages
#define M_CREATECLC  (WM_USER+1)
#define M_SETALLEXTRAICONS (WM_USER+2)

//an infotip for an item should be shown now
//wParam = 0
//lParam = (LPARAM)(CLCINFOTIP*)&it
//Return nonzero if you process this, because it makes no sense for more than
//one plugin to grab it.
//It is up to the plugin to decide the best place to put the infotip. Normally
//it's a few pixels below and to the right of the cursor
//This event is called after the mouse has been stationary over a contact for
//(by default) 200ms, but see below.
//Everything is in screen coordinates.
typedef struct {
	int cbSize;
	int isTreeFocused;   //so the plugin can provide an option
	int isGroup;     //0 if it's a contact, 1 if it's a group
	HANDLE hItem;	 //handle to group or contact
	POINT ptCursor;
	RECT rcItem;
} CLCINFOTIP;
#define ME_CLC_SHOWINFOTIP    "CLC/ShowInfoTip"

typedef struct {
	int cbSize;
	int isTreeFocused;   //so the plugin can provide an option
	HANDLE hItem;	 //handle to group or contact
	POINT ptCursor;
	RECT rcItem;
	int  extraIndex;
	HWND hwnd;
} CLCEXTRAINFOTIP;
#define ME_CLC_SHOWEXTRAINFOTIP    "CLC/ShowExtraInfoTip"

//it's time to destroy an infotip
//wParam = 0
//lParam = (LPARAM)(CLCINFOTIP*)&it
//Only cbSize, isGroup and hItem are set
//Return nonzero if you process this.
//This is sent when the mouse moves off a contact when clc/showinfotip has
//previously been called.
//If you don't want this behaviour, you should have grabbed the mouse capture
//yourself and made your own arrangements.
#define ME_CLC_HIDEINFOTIP    "CLC/HideInfoTip"

//set the hover time before the infotip hooks are called
//wParam = newTime
//lParam = 0
//Returns 0 on success or nonzero on failure
//The value of this setting is applied to all current CLC windows, and saved
//to be applied to all future windows, including after restarts.
//newTime is in ms.
//The default is 750ms.
#define MS_CLC_SETINFOTIPHOVERTIME    "CLC/SetInfoTipHoverTime"

//get the hover time before the infotip hooks are called
//wParam = lParam = 0
//Returns the time in ms
#define MS_CLC_GETINFOTIPHOVERTIME    "CLC/GetInfoTipHoverTime"

#endif // M_CLC_H__
