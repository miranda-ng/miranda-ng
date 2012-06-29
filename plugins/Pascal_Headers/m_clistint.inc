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

{$IFDEF M_API_UNIT}

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
  MAXEXTRACOLUMNS  = 16;
  MAX_TIP_SIZE     = 2048;

function IsHContactGroup  (h:THANDLE):bool;
function IsHContactInfo   (h:THANDLE):bool;
function IsHContactContact(h:THANDLE):bool;

const
  INTM_NAMECHANGED         = WM_USER+10;
  INTM_ICONCHANGED         = WM_USER+11;
  INTM_GROUPCHANGED        = WM_USER+12;
  INTM_GROUPSCHANGED       = WM_USER+13;
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
  PClcContact = ^ClcContact;
  ClcContact = record
      {undefined structure}
    end;

  PClcData = ^ClcData;
  ClcData = record
      {undefined structure}
    end;

{ templates, where are you...  }

  PContactList = ^ContactList;
  ContactList = record
    items     : ^PClcContact;
    count     : int;
    limit     : int;
    increment : int;
    sortFunc  : pointer;
  end;

  PEventList = ^EventList;
  EventList = record
    items     : ^PCListEvent;
    count     : int;
    limit     : int;
    increment : int;
    sortFunc  : pointer;
  end;

  PClcGroup = ^ClcGroup;
  ClcGroup = record
    cl           : ContactList;
    expanded     : int;
    hideOffline  : int;
    groupId      : int;
    parent       : PClcGroup;
    scanIndex    : int;
    totalMembers : int;
  end;

  PClcFontInfo = ^ClcFontInfo;
  ClcFontInfo = record
    hFont      : HFONT;
    fontHeight : int;
    changed    : int;
    colour     : COLORREF;
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

  P_menuProto = ^_menuProto;
  _menuProto = record
    szProto : PAnsiChar;
    pMenu   : HGENMENU;
    hIcon   : HICON;
  end;
  MenuProto  = _menuProto;
  PMenuProto = ^MenuProto;
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
  CLCDEFAULT_GAMMACORRECT      = 1;
  CLCDEFAULT_SHOWIDLE          = 1;
  CLCDEFAULT_USEWINDOWSCOLOURS = 0;

  TRAYICON_ID_BASE = 100;
  TIM_CALLBACK     = WM_USER+1857;
  TIM_CREATE       = WM_USER+1858;

function CLCDEFAULT_BKCOLOUR:dword;
function CLCDEFAULT_TEXTCOLOUR:dword;
function CLCDEFAULT_SELBKCOLOUR:dword;
function CLCDEFAULT_SELTEXTCOLOUR:dword;
function CLCDEFAULT_HOTTEXTCOLOUR:dword;

const
{
  Miranda 0.4.3.0+
  retrieves the pointer to a CLIST_INTERFACE structure
  NOTE: valid only for the clist clone building, not for the regular use
}
  MS_CLIST_RETRIEVE_INTERFACE:PansiChar = 'CList/RetrieveInterface';

  {**************************************************************************
   * CLIST_INTERFACE structure definition
   ************************************************************************** }

type
  PClcProtoStatus = ^ClcProtoStatus;
  ClcProtoStatus = record
    szProto :PAnsiChar;
    dwStatus:dword;
  end;

type
  PClcCacheEntryBase = pointer;
  PClcCacheEntryBaseA = ^ClcCacheEntryBaseA;
  ClcCacheEntryBaseA = record
    hContact:THANDLE;
    name    :TCHAR;
    group   :TCHAR;
    isHidden:int;
  end;
  PClcCacheEntryBaseW = ^ClcCacheEntryBaseW;
  ClcCacheEntryBaseW = record
    hContact:THANDLE;
    name    :TCHAR;
    szName  :PAnsiChar;
    group   :TCHAR;
    isHidden:int;
  end;

type
  PCLIST_INTERFACE = ^CLIST_INTERFACE;
  CLIST_INTERFACE = record
    version         : int;
    hwndContactList : HWND;
    hwndContactTree : HWND;
    hwndStatus      : HWND;
    hMenuMain       : HMENU;
    hInst           : HMODULE;
    hClcProtoCount  : int;
    clcProto        : PClcProtoStatus;

  (* clc.h *)
    pfnClcOptionsChanged   : procedure ;cdecl;
    pfnClcBroadcast        : procedure (_para1:int; _para2:WPARAM; _para3:LPARAM); cdecl;
    pfnBuildGroupPopupMenu : function  (_para1:PClcGroup):HMENU; cdecl;

    pfnContactListControlWndProc:function(hwnd:HWND; msg:uint; wParam:WPARAM; lParam:LPARAM):LRESULT; stdcall;

  (* clcfiledrop.c *)
    pfnRegisterFileDropping   : procedure (hwnd:HWND); cdecl;
    pfnUnregisterFileDropping : procedure (hwnd:HWND); cdecl;

  (* clcidents.c *)
    pfnGetRowsPriorTo      : function (var group:ClcGroup; var subgroup:ClcGroup;
                             contactIndex:int):int; cdecl;
    pfnFindItem            : function (hwnd:HWND; var dat:ClcData; hItem:THANDLE;
                             var contact:PClcContact; var subgroup:PClcGroup; 
                             var isVisible:int):int; cdecl;
    pfnGetRowByIndex       : function (var dat:ClcData; testindex:int;
                             var contact:PClcContact; var subgroup:PClcGroup):int; cdecl;
    pfnContactToHItem      : function (var contact:ClcContact):THANDLE;cdecl;
    pfnContactToItemHandle : function (var contact:ClcContact; var nmFlags:DWORD):THANDLE; cdecl;

  (* clcitems.c *)
    pfnCreateClcContact    : function :PClcContact; cdecl;
    pfnAddGroup            : function (hwnd:HWND; var dat:ClcData; szName:TChar;
                             flags:DWORD; groupId:int; calcTotalMembers:int):PClcGroup; cdecl;
    pfnRemoveItemFromGroup : function (hwnd:HWND; var group:ClcGroup;
                             var contact:ClcContact; updateTotalCount:int):PClcGroup; cdecl;

    pfnFreeContact : procedure (var contact:ClcContact); cdecl;
    pfnFreeGroup   : procedure (var group:ClcGroup); cdecl;

    pfnAddInfoItemToGroup      : function  (var group:ClcGroup; flags:int; pszText:TChar):int; cdecl;
    pfnAddItemToGroup          : function  (var group:ClcGroup; iAboveItem:int):int; cdecl;
    pfnAddContactToGroup       : function  (var dat:ClcData; var group:ClcGroup;
                                 hContact:THANDLE):int; cdecl;
    pfnAddContactToTree        : procedure (hwnd:HWND; var dat:ClcData; hContact:THANDLE;
                                 updateTotalCount:int; checkHideOffline:int); cdecl;
    pfnDeleteItemFromTree      : procedure (hwnd:HWND; hItem:THANDLE); cdecl;
    pfnRebuildEntireList       : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnGetGroupContentsCount   : function  (var group:ClcGroup; visibleOnly:int):int; cdecl;
    pfnSortCLC                 : procedure (hwnd:HWND; var dat:ClcData; useInsertionSort:int); cdecl;
    pfnSaveStateAndRebuildList : procedure (hwnd:HWND; var dat:ClcData); cdecl;

  (* clcmsgs.c *)
    pfnProcessExternalMessages : function (hwnd:HWND; var dat:ClcData; msg:UINT;
                                 wParam:WPARAM; lParam:LPARAM):LRESULT; cdecl;

  (* clcpaint.c *)
    pfnPaintClc : procedure (hwnd:HWND; var dat:ClcData; hdc:HDC; var rcPaint:TRECT);  cdecl;

  (* clcutils.c *)
    pfnGetGroupCountsText         : function (var dat:ClcData; var contact:ClcContact):PAnsiChar; cdecl;
    pfnHitTest                    : function (hwnd:HWND; var dat:ClcData; testx:int;
                                    testy:int; var contact:PClcContact;
                                    var group:PClcGroup; var flags:DWORD):int; cdecl;
    pfnScrollTo                   : procedure (hwnd:HWND; var dat:ClcData; desty:int; noSmooth:int); cdecl;
    pfnEnsureVisible              : procedure (hwnd:HWND; var dat:ClcData; iItem:int; partialOk:int); cdecl;
    pfnRecalcScrollBar            : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnSetGroupExpand             : procedure (hwnd:HWND; var dat:ClcData; var group:ClcGroup; newState:int); cdecl;
    pfnDoSelectionDefaultAction   : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnFindRowByText              : function  (hwnd:HWND; var dat:ClcData; text:TChar; prefixOk:int):int; cdecl;
    pfnEndRename                  : procedure (hwnd:HWND; var dat:ClcData; save:int); cdecl;
    pfnDeleteFromContactList      : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnBeginRenameSelection       : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnCalcEipPosition            : procedure (var dat:ClcData; var contact:ClcContact; var group:ClcGroup; var result:TPOINT); cdecl;
    pfnGetDropTargetInformation   : function  (hwnd:HWND; var dat:ClcData; pt:TPOINT):int; cdecl;
    pfnClcStatusToPf2             : function  (status:int):int; cdecl;
    pfnIsHiddenMode               : function  (var dat:ClcData; status:int):int; cdecl;
    pfnHideInfoTip                : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnNotifyNewContact           : procedure (hwnd:HWND; hContact:THANDLE); cdecl;
    pfnGetDefaultExStyle          : function  :DWORD; cdecl;
    pfnGetDefaultFontSetting      : procedure (i:int; var lf:LOGFONT; var colour:COLORREF); cdecl;
    pfnGetFontSetting             : procedure (i:int; var lf:LOGFONT; var colour:COLORREF); cdecl;
    pfnLoadClcOptions             : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnRecalculateGroupCheckboxes : procedure (hwnd:HWND; var dat:ClcData); cdecl;
    pfnSetGroupChildCheckboxes    : procedure (var group:ClcGroup; checked:int); cdecl;
    pfnInvalidateItem             : procedure (hwnd:HWND; var dat:ClcData; iItem:int); cdecl;

    pfnGetRowBottomY     : function (var dat:ClcData; item:int):int; cdecl;
    pfnGetRowHeight      : function (var dat:ClcData; item:int):int; cdecl;
    pfnGetRowTopY        : function (var dat:ClcData; item:int):int; cdecl;
    pfnGetRowTotalHeight : function (var dat:ClcData):int; cdecl;
    pfnRowHitTest        : function (var dat:ClcData; y:int):int; cdecl;

  (* clistevents.c *)
    pfnEventsProcessContactDoubleClick : function (hContact:THANDLE):int; cdecl;
    pfnEventsProcessTrayDoubleClick    : function (_para1:int):int; cdecl;

  (* clistmod.c *)
    pfnIconFromStatusMode       : function (szProto:PAnsiChar; status:int; hContact:THANDLE):int; cdecl;
    pfnShowHide                 : function (_para1:WPARAM; _para2:LPARAM):int; cdecl;
    pfnGetStatusModeDescription : function (mode:int; flags:int):TChar; cdecl;

  (* clistsettings.c *)
    pfnGetCacheEntry   : function (hContact:THANDLE):PClcCacheEntryBase; cdecl;
    pfnCreateCacheItem : function (hContact:THANDLE):PClcCacheEntryBase; cdecl;
    pfnCheckCacheItem  : procedure (_para1:PClcCacheEntryBase); cdecl;
    pfnFreeCacheItem   : procedure (_para1:PClcCacheEntryBase); cdecl;

    pfnGetContactDisplayName           : function (hContact:THANDLE; mode:int):TChar; cdecl;
    pfnInvalidateDisplayNameCacheEntry : procedure (hContact:THANDLE); cdecl;

  (* clisttray.c *)
    pfnTrayIconUpdateWithImageList : procedure (iImage:int; szNewTip:TChar;
                                     szPreferredProto:PAnsiChar); cdecl;
    pfnTrayIconUpdateBase          : procedure (szChangedProto:PAnsiChar); cdecl;
    pfnTrayIconSetToBase           : procedure (szPreferredProto:PAnsiChar); cdecl;
    pfnTrayIconIconsChanged        : procedure ; cdecl;
    pfnTrayIconPauseAutoHide       : function (wParam:WPARAM; lParam:LPARAM):int; cdecl;
    pfnTrayIconProcessMessage      : function (wParam:WPARAM; lParam:LPARAM):int_ptr; cdecl;
    pfnCListTrayNotify             : function (_para1:PMIRANDASYSTRAYNOTIFY):int; cdecl;

  (* clui.c *)
    pfnContactListWndProc:function(hwnd:HWND; msg:uint; wParam:WPARAM; lParam:LPARAM):int; stdcall;

    pfnCluiProtocolStatusChanged : procedure (status:int; szProto:PAnsiChar); cdecl;
    pfnDrawMenuItem              : procedure (_para1:PDRAWITEMSTRUCT; _para2:HICON; 
                                   _para3:HICON); cdecl;
    pfnLoadCluiGlobalOpts        : procedure ; cdecl;
    pfnInvalidateRect            : function (hwnd:HWND; var lpRect:TRECT; bErase:BOOL):BOOL; cdecl;
    pfnOnCreateClc               : procedure ; cdecl;

  (* contact.c *)
    pfnChangeContactIcon : procedure (hContact:THANDLE; iIcon:int; add:int); cdecl;
    pfnLoadContactTree   : procedure ; cdecl;
    pfnCompareContacts   : function (var contact1:ClcContact; var contact2:ClcContact):int; cdecl;
    pfnSortContacts      : procedure ; cdecl;
    pfnSetHideOffline    : function (wParam:WPARAM; lParam:LPARAM):int; cdecl;

  (* docking.c *)
    pfnDocking_ProcessWindowMessage : function (wParam:WPARAM; lParam:LPARAM):int; cdecl;

  (* group.c *)
    pfnGetGroupName : function (idx:int; var pdwFlags:DWORD):TChar; cdecl;
    pfnRenameGroup  : function (groupID:int; newName:TChar):int; cdecl;

  (* keyboard.c *)
    pfnHotKeysRegister       : function (hwnd:HWND):int; cdecl;
    pfnHotKeysUnregister     : procedure (hwnd:HWND); cdecl;
    pfnHotKeysProcess        : function (hwnd:HWND; wParam:WPARAM; lParam:LPARAM):int; cdecl;
    pfnHotkeysProcessMessage : function (wParam:WPARAM; lParam:LPARAM):int; cdecl;

  (*************************************************************************************
  * version 2 - events processing
  *************************************************************************************)

    events : EventList;

    pfnCreateEvent : function :PCListEvent; cdecl;
    pfnFreeEvent   : procedure (_para1:PCListEvent); cdecl;

    pfnAddEvent : function (_para1:PCLISTEVENT):PCListEvent; cdecl;
    pfnGetEvent : function (hContact:THANDLE; idx:int):PCLISTEVENT; cdecl;

    pfnRemoveEvent     : function (hContact:THANDLE; hDbEvent:THANDLE):int; cdecl;
    pfnGetImlIconIndex : function (hIcon:HICON):int; cdecl;

  (*************************************************************************************
  * version 3 additions
  *************************************************************************************)

    pfnGetWindowVisibleState : function (hWnd:HWND; iStepX:int; iStepY:int):int; cdecl;

  (*************************************************************************************
  * version 4 additions (0.7.0.x) - genmenu
  *************************************************************************************)

    menuProtos     : PMenuProto;
    menuProtoCount : int;

    hPreBuildStatusMenuEvent : THANDLE;
    currentStatusMenuItem    : int;
    currentDesiredStatusMode : int;
    bDisplayLocked           : BOOL;

    pfnGetProtocolMenu : function (_para1:PAnsiChar):HGENMENU; cdecl;
    pfnStub2           : function (_para1:int):int; cdecl;

    pfnGetProtocolVisibility : function (_para1:PAnsiChar):int; cdecl;
    pfnGetProtoIndexByPos    : function (var proto:PPROTOCOLDESCRIPTOR;
                               protoCnt:int; Pos:int):int; cdecl;
    pfnReloadProtoMenus      : procedure ; cdecl;

  (*************************************************************************************
  * version 5 additions (0.7.0.x) - tray icons
  *************************************************************************************)

    trayIcon          : PtrayIconInfo_t;
    trayIconCount     : int;
    shellVersion      : int;
    cycleTimerId      : uint_ptr;
    cycleStep         : int;
    szTip             : TChar;
    bTrayMenuOnScreen : BOOL;

    pfnGetIconFromStatusMode  : function  (hContact:THANDLE; szProto:PAnsiChar;
                                status:int):HICON; cdecl;

    pfnInitTray               : procedure ; cdecl;
    pfnTrayIconAdd            : function  (hwnd:HWND; szProto:PAnsiChar; szIconProto:PAnsiChar;
                                status:int):int; cdecl;
    pfnTrayIconDestroy        : function  (hwnd:HWND):int; cdecl;
    pfnTrayIconInit           : function  (hwnd:HWND):int; cdecl;
    pfnTrayIconMakeTooltip    : function  (szPrefix:TChar; szProto:PAnsiChar):TChar; cdecl;
    pfnTrayIconRemove         : procedure (hwnd:HWND; szProto:PAnsiChar); cdecl;
    pfnTrayIconSetBaseInfo    : function  (hIcon:HICON; szPreferredProto:PAnsiChar):int; cdecl;
    pfnTrayIconTaskbarCreated : procedure (hwnd:HWND); cdecl;
    pfnTrayIconUpdate         : function  (hNewIcon:HICON; szNewTip:TChar;
                                szPreferredProto:PAnsiChar; isBase:int):int; cdecl;

    pfnUninitTray : procedure ; cdecl;
    pfnLockTray   : procedure ; cdecl;
    pfnUnlockTray : procedure ; cdecl;

    pfnTrayCycleTimerProc : procedure(hwnd:HWND;message:uint;idEvent:uint_ptr;dwTime:DWORD); stdcall;

  (*************************************************************************************
  * version 6 additions (0.8.0.x) - accounts
  *************************************************************************************)
    pfnGetAccountIndexByPos : function (pos:int):int; cdecl;
    pfnConvertMenu: function( clitem:PCLISTMENUITEM; moitem:PMO_MenuItem):int; cdecl;
  end;

{
var
  cli : CLIST_INTERFACE;cvar;external;
}

{$ELSE}

function IsHContactGroup(h:THANDLE):bool;
begin
  result:=(h xor HCONTACT_ISGROUP)<(HCONTACT_ISGROUP xor HCONTACT_ISINFO);
end;
function IsHContactInfo(h:THANDLE):bool;
begin
  result:=(h and HCONTACT_ISINFO)=HCONTACT_ISINFO;
end;
function IsHContactContact(h:THANDLE):bool;
begin
 result:=(h and HCONTACT_ISGROUP)=0;
end;

function CLCDEFAULT_BKCOLOUR:dword;
begin
  result:=GetSysColor(COLOR_3DFACE);
end;
function CLCDEFAULT_TEXTCOLOUR:dword;
begin
  result:=GetSysColor(COLOR_WINDOWTEXT);
end;
function CLCDEFAULT_SELBKCOLOUR:dword;
begin
  result:=GetSysColor(COLOR_HIGHLIGHT);
end;
function CLCDEFAULT_SELTEXTCOLOUR:dword;
begin
  result:=GetSysColor(COLOR_HIGHLIGHTTEXT);
end;
function CLCDEFAULT_HOTTEXTCOLOUR:dword;
begin
{
  if IsWinVer98Plus then
    result:=RGB(0,0,255)
  else
}
    result:=GetSysColor(COLOR_HOTLIGHT);
end;
{$ENDIF}
