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

{Type
  P_menuProto           = ^_menuProto;
  PClcCacheEntryBase    = ^ClcCacheEntryBase;
  PClcContact           = ^ClcContact;
  PClcData              = ^ClcData;
  PClcFontInfo          = ^ClcFontInfo;
  PClcGroup             = ^ClcGroup;
  PClcProtoStatus       = ^ClcProtoStatus;
  PCLIST_INTERFACE      = ^CLIST_INTERFACE;
  PCListEvent           = ^CListEvent;
  PCLCCacheEntry        = ^CLCCacheEntry;
  PContactList          = ^ContactList;
  PEventList            = ^EventList;
  PMenuProto            = ^MenuProto;
  PMIRANDASYSTRAYNOTIFY = ^MIRANDASYSTRAYNOTIFY;
  PPROTOCOLDESCRIPTOR   = ^PROTOCOLDESCRIPTOR;
  PtrayIconInfo_t       = ^trayIconInfo_t;
}
const
  HCONTACT_ISGROUP = $80000000;
  HCONTACT_ISINFO  = $FFFF0000;
  MAX_TIP_SIZE     = 2048;
  EXTRA_ICON_COUNT = 10;

const
  INTM_NAMECHANGED         = WM_USER+10;
  INTM_ICONCHANGED         = WM_USER+11;
  INTM_GROUPCHANGED        = WM_USER+12;
  INTM_CONTACTADDED        = WM_USER+14;
  INTM_CONTACTDELETED      = WM_USER+15;
  INTM_HIDDENCHANGED       = WM_USER+16;
  INTM_INVALIDATE          = WM_USER+17;
  INTM_APPARENTMODECHANGED = WM_USER+18;
  INTM_SETINFOTIPHOVERTIME = WM_USER+19;
  INTM_NOTONLISTCHANGED    = WM_USER+20;
  INTM_RELOADOPTIONS       = WM_USER+21;
  INTM_NAMEORDERCHANGED    = WM_USER+22;
  INTM_IDLECHANGED         = WM_USER+23;
  INTM_SCROLLBARCHANGED    = WM_USER+24;
  INTM_PROTOCHANGED        = WM_USER+25;

  TIMERID_RENAME           = 10;
  TIMERID_DRAGAUTOSCROLL   = 11;
  TIMERID_INFOTIP          = 13;
  TIMERID_REBUILDAFTER     = 14;
  TIMERID_DELAYEDRESORTCLC = 15;
  TIMERID_TRAYHOVER        = 16;
  TIMERID_TRAYHOVER_2      = 17;

  GROUP_ALLOCATE_STEP = 8;

type
  PClcContact = ^tClcContact;
  tClcContact = record
      {undefined structure}
  end;

  PClcData = ^tClcData;
  tClcData = record
      {undefined structure}
  end;

{ templates, where are you...  }

  PContactList = ^tContactList;
  tContactList = record
    items     : ^PClcContact;
    count     : int;
    limit     : int;
    increment : int;
    sortFunc  : pointer;
  end;

  PEventList = ^TEventList;
  TEventList = record
    items     : ^PCListEvent;
    count     : int;
    limit     : int;
    increment : int;
    sortFunc  : pointer;
  end;

  pClcGroup = ^tClcGroup;
  tClcGroup = record
    cl           : tContactList;
    expanded     : int;
    hideOffline  : int;
    groupId      : int;
    parent       : PClcGroup;
    scanIndex    : int;
    totalMembers : int;
  end;

  pClcFontInfo = ^tClcFontInfo;
  tClcFontInfo = record
    hFont      : HFONT;
    fontHeight : int;
    changed    : int;
    colour     : COLORREF;
  end;

type
  tCLCCB = record
    case boolean of
     false: (
       iImage:int;
       hContact:TMCONTACT);
     true : (
       groupId:word;
       group  :pClcGroup;);
  end;
  pClcContactBase = ^tClcContactBase;
  tClcContactBase = record
    _type      :byte;
    flags      :byte;
    data       :tCLCCB;
    iExtraImage:array [0..EXTRA_ICON_COUNT-1] of word;
    szText     :array [0..120-EXTRA_ICON_COUNT-1] of WideChar;
    proto      :PAnsiChar; // MS_PROTO_GETBASEPROTO
  end;

type
  pClcDataBase = ^tClcDataBase;
  tClcDataBase = record
    list                  :TClcGroup;
    rowHeight             :int;
    yScroll               :int;
    selection             :int;
    fontInfo              :array [0..FONTID_MAX] of TClcFontInfo;
    scrollTime            :int;
    himlHighlight         :THANDLE; // HIMAGELIST in CommCtrl
    groupIndent           :int;
    szQuickSearch         :array [0..127] of WideChar;
    iconXSpace            :int;
    hwndRenameEdit        :HWND;
    bkColour,
    selBkColour,
    selTextColour,
    hotTextColour,
    quickSearchColour     :TCOLORREF;
    iDragItem,
    iInsertionMark        :int;
    dragStage             :int;
    ptDragStart           :TPOINT;
    dragAutoScrolling     :int;
    dragAutoScrollHeight  :int;
    leftMargin            :int;
    insertionMarkHitHeight:int;
    hBmpBackground        :HBITMAP;
    backgroundBmpUse,
    bkChanged             :int;
    iHotTrack             :int;
    gammaCorrection       :int;
    greyoutFlags          :dword; // see m_clc.h
    offlineModes          :dword;
    exStyle               :dword;
    ptInfoTip             :TPOINT;
    infoTipTimeout        :int;
    hInfoTipItem          :DWORD;
    himlExtraColumns      :THANDLE; // HIMAGELIST in CommCtrl
    extraColumnsCount     :int;
    extraColumnSpacing    :int;
    checkboxSize          :int;
	 bShowSelAlways, bShowIdle, bNoVScrollbar, bUseWindowsColours : byte;
	 bNeedsResort, bFilterSearch, bLockScrollbar : byte;
  end;

  PtrayIconInfo_t = ^trayIconInfo_t;
  trayIconInfo_t = record
    id          : int;
    szProto     : PAnsiChar;
    hBaseIcon   : HICON;
    isBase      : int;
    ptszToolTip : TChar;
  end;

{This is DLL-based unique name }

  P_menuProto = ^T_menuProto;
  T_menuProto = record
    szProto : PAnsiChar;
    pMenu   : HGENMENU;
    hIcon   : HICON;
  end;
  TMenuProto = T_menuProto;
  PMenuProto = ^TMenuProto;
{ constants  }

const
  DRAGSTAGE_NOTMOVED     = 0;
  DRAGSTAGE_ACTIVE       = 1;
  DRAGSTAGEM_STAGE       = $00FF;
  DRAGSTAGEF_MAYBERENAME = $8000;
  DRAGSTAGEF_OUTSIDE     = $4000;

  CONTACTF_ONLINE    = 1;
  CONTACTF_INVISTO   = 2;
  CONTACTF_VISTO     = 4;
  CONTACTF_NOTONLIST = 8;
  CONTACTF_CHECKED   = 16;
  CONTACTF_IDLE      = 32;

  DROPTARGET_OUTSIDE   = 0;
  DROPTARGET_ONSELF    = 1;
  DROPTARGET_ONNOTHING = 2;
  DROPTARGET_ONGROUP   = 3;
  DROPTARGET_ONCONTACT = 4;
  DROPTARGET_INSERTION = 5;

  CLCDEFAULT_ROWHEIGHT         = 16;
  CLCDEFAULT_EXSTYLE           = CLS_EX_EDITLABELS or CLS_EX_TRACKSELECT or
                                 CLS_EX_SHOWGROUPCOUNTS or CLS_EX_HIDECOUNTSWHENEMPTY or
                                 CLS_EX_TRACKSELECT or CLS_EX_NOTRANSLUCENTSEL;
                                 //plus CLS_EX_NOSMOOTHSCROLL is got from the system
  CLCDEFAULT_SCROLLTIME        = 150;
  CLCDEFAULT_GROUPINDENT       = 5;
  CLCDEFAULT_USEBITMAP         = 0;
  CLCDEFAULT_BKBMPUSE          = CLB_STRETCH;
  CLCDEFAULT_OFFLINEMODES      = MODEF_OFFLINE;
  CLCDEFAULT_GREYOUTFLAGS      = 0;
  CLCDEFAULT_SELBLEND          = 1;
  CLCDEFAULT_FULLGREYOUTFLAGS  = MODEF_OFFLINE or PF2_INVISIBLE or GREYF_UNFOCUS;
  CLCDEFAULT_QUICKSEARCHCOLOUR = $0000FFFF; //RGB(255,255,0);
  CLCDEFAULT_LEFTMARGIN        = 0;
  CLCDEFAULT_RIGHTMARGIN       = 2;
  CLCDEFAULT_GAMMACORRECT      = 1;
  CLCDEFAULT_SHOWIDLE          = 1;
  CLCDEFAULT_USEWINDOWSCOLOURS = 0;

  TRAYICON_ID_BASE = 100;
  TIM_CALLBACK     = WM_USER+1857;
  TIM_CREATE       = WM_USER+1858;

  {**************************************************************************
   * CLIST_INTERFACE structure definition
   ************************************************************************** }

const
  GCDNF_NOMYHANDLE = 1; // will never return the user's custom name
  GCDNF_NOCACHE    = 4; // will not use the cache

type
  PClcProtoStatus = ^TClcProtoStatus;
  TClcProtoStatus = record
    szProto :PAnsiChar;
    dwStatus:dword;
  end;

type
  PClcCacheEntryBase = ^TClcCacheEntryBase;
  TClcCacheEntryBase = record
    hContact:TMCONTACT;
    name    :TCHAR;
    group   :TCHAR;
    isHidden:int;
  end;

  PClcCacheEntry = ^TCLCCacheEntry;
  TCLCCacheEntry = record
  end;

type
  PCLIST_INTERFACE = ^TCLIST_INTERFACE;
  TCLIST_INTERFACE = record
    hwndContactList : HWND;
    hwndContactTree : HWND;
    hwndStatus      : HWND;
    hMenuMain       : HMENU;
    hInst           : HMODULE;
    hClcProtoCount  : int;
    clcProto        : PClcProtoStatus;

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
    pfnShowHide                        : procedure; cdecl;
                                       
  (* clistsettings.c *)
    pfnCreateCacheItem                 : procedure; cdecl;
    pfnCheckCacheItem                  : procedure; cdecl;
    pfnFreeCacheItem                   : procedure; cdecl;

    pfnInvalidateDisplayNameCacheEntry : procedure; cdecl;

  (* clisttray.c *)
    pfnTrayIconPauseAutoHide           : procedure; cdecl;
    pfnTrayIconProcessMessage          : procedure; cdecl;
                                       
  (* clui.c *)                         
    pfnContactListWndProc              : procedure; cdecl;
                                       
    pfnCluiProtocolStatusChanged       : procedure; cdecl;
    pfnLoadCluiGlobalOpts              : procedure; cdecl;
    pfnInvalidateRect                  : procedure; cdecl;
    pfnOnCreateClc                     : procedure; cdecl;
                                       
  (* contact.c *)                      
    pfnCompareContacts                 : procedure; cdecl;
    pfnSetHideOffline                  : procedure; cdecl;
                                      
  (* docking.c *)                     
    pfnDocking_ProcessWindowMessage    : procedure; cdecl;

  (*************************************************************************************
  * version 2 - events processing
  *************************************************************************************)

    events : PEventList;

    pfnAddEvent                        : procedure; cdecl;
    pfnGetEvent                        : procedure; cdecl;

    pfnRemoveEvent                     : function (hContact:TMCONTACT; hDbEvent:TMEVENT):int; cdecl;
  end;

function Clist_GetInterface : PCLIST_INTERFACE; stdcall; external AppDll;

var
  cli : PCLIST_INTERFACE;

{$ENDIF}
