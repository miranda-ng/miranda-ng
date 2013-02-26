{
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2004 by Marco van de Voort
    member of the Free Pascal development team.

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Original copyright statement follows.

 **************************************************************************
 *                                                                        *
 * commctrl.h - - Interface for the Windows Common Controls               *
 *                                                                        *
 * Version 1.2                                                            *
 *                                                                        *
 * Copyright (c) Microsoft Corporation. All rights reserved.              *
 *                                                                        *
 **************************************************************************

 Note: This is an initial translation, and probably _full_ of bugs, this
       is because my conversion tool was flawed, and a lot of postediting
       was needed, which is bound to introduce bugs and inconsequencies

       This is an huge unit, and its maintenance and compability is not
       a top-priority.
       I prefer patches as bugfix over bugreports, and
       preferably patches with lots of fixes at once.

       If you can't make a patch, at least research the bug thoroughly,
       and provide as much info as possible (which windows version,
       the C prototype from the sdk, etc)

       Help the FPC team, and try to do as much as possible yourself.
}


Unit CommCtrl;
Interface

{$Mode ObjFPC}
Uses Windows,CTypes,ActiveX;

// --------------------

// --------------------
CONST CommCtrlDLL = 'comctl32.dll';

// Some reasonal defaults.
// for XP only set NT to $%0501 and IE to $0600
//
// The NT conditional is only used for XP/no XP.

{$DEFINE WIN32_IE=$0500}
{$DEFINE IE3PLUS}
{$DEFINE IE4PLUS}
{$define IE5plus}
{$define WIN32XP} 
{$define win32vista} // till WC_STATICA
{$define ie501plus}
{$ifdef win32}
  {$define _win32}
{$endif win32}
{$define ie6plus}
{$define ntddi_vista}
{$define NTDDI_WIN7}

{$ifdef win64}
  {$define _win32}
{$endif win64}

{$DEFINE WIN32_WINNT=0}                     // NO XP
{$DEFINE __IStream_INTERFACE_DEFINED__}  // lpstream defined in activex afaik.

// Sanity check in source. Not translated:
//if (WIN32_IE < $0400) && defined(WIN32_WINNT) && (WIN32_WINNT >= $0500)}
// #error _WIN32_IE setting conflicts with _WIN32_WINNT setting

//
// Users of this header may define any number of these constants to avoid
// the definitions of each functional group.
//
//    NOTOOLBAR    Customizable bitmap-button toolbar control.
//    NOUPDOWN     Up and Down arrow increment/decrement control.
//    NOSTATUSBAR  Status bar control.
//    NOMENUHELP   APIs to help manage menus, especially with a status bar.
//    NOTRACKBAR   Customizable column-width tracking control.
//    NODRAGLIST   APIs to make a listbox source and sink drag&drop actions.
//    NOPROGRESS   Progress gas gauge.
//    NOHOTKEY     HotKey control
//    NOHEADER     Header bar control.
//    NOIMAGEAPIS  ImageList apis.
//    NOLISTVIEW   ListView control.
//    NOTREEVIEW   TreeView control.
//    NOTABCONTROL Tab control.
//    NOANIMATE    Animate control.
//    NOBUTTON     Button control.
//    NOSTATIC     Static control.
//    NOEDIT       Edit control.
//    NOLISTBOX    Listbox control.
//    NOCOMBOBOX   Combobox control.
//    NOSCROLLBAR  Scrollbar control.
//
//=============================================================================
// Moved items due to forward defining limitations

Const
         MAX_LINKID_TEXT                = 48;
         L_MAX_URL_LENGTH               = (2048 + 32 + length('://'));
Type
         tagLITEM             = Record
                                 mask         : UINT;
                                 iLink        : cint;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 szID         : Array [0..MAX_LINKID_TEXT-1] OF WCHAR;
                                 szUrl        : Array [0..L_MAX_URL_LENGTH-1] OF WCHAR;
                                 END;
         LITEM                = tagLITEM;
         PLITEM               = ^tagLITEM;
         TLITEM               = tagLITEM;
//         PLITEM               = ^tagLITEM;



// include <prsht.h>

Procedure InitCommonControls; stdcall; external commctrldll name 'InitCommonControls';

{$ifdef ie3plus}
TYPE

         tagINITCOMMONCONTROLSEX = Record
                                    dwSize       : DWORD;          // size of this structure
                                    dwICC        : DWORD;          // flags indicating which classes to be initialized
                                    END;
         _INITCOMMONCONTROLSEX = tagINITCOMMONCONTROLSEX;
         LPINITCOMMONCONTROLSEX = ^tagINITCOMMONCONTROLSEX;
         TINITCOMMONCONTROLSEX = tagINITCOMMONCONTROLSEX;
         PINITCOMMONCONTROLSEX = ^tagINITCOMMONCONTROLSEX;

CONST
         ICC_LISTVIEW_CLASSES           = $00000001;          // listview, header
         ICC_TREEVIEW_CLASSES           = $00000002;          // treeview, tooltips
         ICC_BAR_CLASSES                = $00000004;          // toolbar, statusbar, trackbar, tooltips
         ICC_TAB_CLASSES                = $00000008;          // tab, tooltips
         ICC_UPDOWN_CLASS               = $00000010;          // updown
         ICC_PROGRESS_CLASS             = $00000020;          // progress
         ICC_HOTKEY_CLASS               = $00000040;          // hotkey
         ICC_ANIMATE_CLASS              = $00000080;          // animate
         ICC_WIN95_CLASSES              = $000000FF;
         ICC_DATE_CLASSES               = $00000100;          // month picker, date picker, time picker, updown
         ICC_USEREX_CLASSES             = $00000200;          // comboex
         ICC_COOL_CLASSES               = $00000400;          // rebar (coolbar) control
{$ifdef ie4plus}
         ICC_INTERNET_CLASSES           = $00000800;
         ICC_PAGESCROLLER_CLASS         = $00001000;          // page scroller
         ICC_NATIVEFNTCTL_CLASS         = $00002000;          // native font control
{$ENDIF}
{$ifdef WIN32XP}
         ICC_STANDARD_CLASSES           = $00004000;
         ICC_LINK_CLASS                 = $00008000;
{$ENDIF}

function InitCommonControlsEx(var rec : TINITCOMMONCONTROLSEX):BOOL; stdcall; external commctrldll name 'InitCommonControlsEx';
{$ENDIF}      // _WIN32_IE >= 0x0300

CONST
         ODT_HEADER                     = 100;
         ODT_TAB                        = 101;
         ODT_LISTVIEW                   = 102;

//====== WM_NOTIFY codes (NMHDR.code values) ==================================


CONST
         NM_FIRST                       = (0-  0);          // generic to all controls
         NM_LAST                        = (0- 99);

         LVN_FIRST                      = (0-100);          // listview
         LVN_LAST                       = (0-199);

// Property sheet reserved      (0U-200U) -  (0U-299U) - see prsht.h

         HDN_FIRST                      = (0-300);          // header
         HDN_LAST                       = (0-399);

         TVN_FIRST                      = (0-400);          // treeview
         TVN_LAST                       = (0-499);

         TTN_FIRST                      = (0-520);          // tooltips
         TTN_LAST                       = (0-549);

         TCN_FIRST                      = (0-550);          // tab control
         TCN_LAST                       = (0-580);

// Shell reserved               (0U-580U) -  (0U-589U)

         CDN_FIRST                      = (0-601);          // common dialog (new)
         CDN_LAST                       = (0-699);

         TBN_FIRST                      = (0-700);          // toolbar
         TBN_LAST                       = (0-720);

         UDN_FIRST                      = (0-721);           // updown
         UDN_LAST                       = (0-740);
{$ifdef ie3plus}
         MCN_FIRST                      = (0-750);          // monthcal
         MCN_LAST                       = (0-759);

         DTN_FIRST                      = (0-760);          // datetimepick
         DTN_LAST                       = (0-799);

         CBEN_FIRST                     = (0-800);          // combo box ex
         CBEN_LAST                      = (0-830);

         RBN_FIRST                      = (0-831);          // rebar
         RBN_LAST                       = (0-859);
{$ENDIF}

{$ifdef ie4plus}
         IPN_FIRST                      = (0-860);          // internet address
         IPN_LAST                       = (0-879);          // internet address

         SBN_FIRST                      = (0-880);          // status bar
         SBN_LAST                       = (0-899);

         PGN_FIRST                      = (0-900);          // Pager Control
         PGN_LAST                       = (0-950);

{$ENDIF}

{$ifdef ie5plus}
{$IFNDEF WMN_FIRST}
         WMN_FIRST                      = (0-1000);
         WMN_LAST                       = (0-1200);
{$ENDIF}
{$ENDIF}

{$ifdef Win32XP}
         BCN_FIRST                      = (0-1250);
         BCN_LAST                       = (0-1350);
{$ENDIF}

{$ifdef win32vista}
	 TRBN_FIRST              	= cardinal(0-1501);       // trackbar
	 TRBN_LAST               	= cardinal(0-1519);
{$endif}
         MSGF_COMMCTRL_BEGINDRAG        = $4200;
         MSGF_COMMCTRL_SIZEHEADER       = $4201;
         MSGF_COMMCTRL_DRAGSELECT       = $4202;
         MSGF_COMMCTRL_TOOLBARCUST      = $4203;

//====== Ranges for control message IDs =======================================

         LVM_FIRST                      = $1000;              // ListView messages
         TV_FIRST                       = $1100;              // TreeView messages
         HDM_FIRST                      = $1200;              // Header messages
         TCM_FIRST                      = $1300;              // Tab control messages

{$ifdef ie4plus}
         PGM_FIRST                      = $1400;              // Pager control messages
{$ifdef win32xp}// actually 0x501=2003 or some sp?
         ECM_FIRST                      = $1500;              // Edit control messages
         BCM_FIRST                      = $1600;              // Button control messages
         CBM_FIRST                      = $1700;              // Combobox control messages
{$ENDIF}
         CCM_FIRST                      = $2000;              // Common control shared messages
         CCM_LAST                       = (CCM_FIRST + $200);


         CCM_SETBKCOLOR                 = (CCM_FIRST + 1);    // lParam is bkColor

TYPE
         tagCOLORSCHEME       = Record
                                 dwSize       : DWORD;
                                 clrBtnHighlight : COLORREF;          // highlight color
                                 clrBtnShadow : COLORREF;          // shadow color
                                 END;
         COLORSCHEME          = tagCOLORSCHEME;
         LPCOLORSCHEME        = ^tagCOLORSCHEME;
         TCOLORSCHEME         = tagCOLORSCHEME;
         PCOLORSCHEME         = ^tagCOLORSCHEME;

CONST
         CCM_SETCOLORSCHEME             = (CCM_FIRST + 2);    // lParam is color scheme
         CCM_GETCOLORSCHEME             = (CCM_FIRST + 3);    // fills in COLORSCHEME pointed to by lParam
         CCM_GETDROPTARGET              = (CCM_FIRST + 4);
         CCM_SETUNICODEFORMAT           = (CCM_FIRST + 5);
         CCM_GETUNICODEFORMAT           = (CCM_FIRST + 6);

{$ifdef ie5plus}
{$ifdef win32xp} 
         COMCTL32_VERSION               = 6;
{$ELSE}
         COMCTL32_VERSION               = 5;
{$ENDIF}

         CCM_SETVERSION                 = (CCM_FIRST + $7);
         CCM_GETVERSION                 = (CCM_FIRST + $8);
         CCM_SETNOTIFYWINDOW            = (CCM_FIRST + $9);   // wParam == hwndParent.
{$ifdef win32xp}
         CCM_SETWINDOWTHEME             = (CCM_FIRST + $b);
         CCM_DPISCALE                   = (CCM_FIRST + $c);   // wParam == Awareness
{$ENDIF}
{$ENDIF} // (_WIN32_IE >= 0x0500)

{$ENDIF} // (_WIN32_IE >= 0x0400)

{$ifdef ie4plus}
// for tooltips
         INFOTIPSIZE                    = 1024;
{$ENDIF}

//====== WM_NOTIFY Macros =====================================================

// Macro 8
Procedure HANDLE_WM_NOTIFY( hwnd : hwnd; wParam : cint;var  _lParam : NMHDR ;fn :Pointer);

// Macro 9
Function FORWARD_WM_NOTIFY( hwnd : hwnd; idFrom : cint;var  pnmhdr : NMHDR ; fn :pointer ):LRESULT;

//====== Generic WM_NOTIFY notification codes =================================


CONST
         NM_OUTOFMEMORY                 = (NM_FIRST-1);
         NM_CLICK                       = (NM_FIRST-2);       // uses NMCLICK struct
         NM_DBLCLK                      = (NM_FIRST-3);
         NM_RETURN                      = (NM_FIRST-4);
         NM_RCLICK                      = (NM_FIRST-5);       // uses NMCLICK struct
         NM_RDBLCLK                     = (NM_FIRST-6);
         NM_SETFOCUS                    = (NM_FIRST-7);
         NM_KILLFOCUS                   = (NM_FIRST-8);
{$ifdef ie3plus}
         NM_CUSTOMDRAW                  = (NM_FIRST-12);
         NM_HOVER                       = (NM_FIRST-13);
{$ENDIF}
{$ifdef ie4plus}
         NM_NCHITTEST                   = (NM_FIRST-14);      // uses NMMOUSE struct
         NM_KEYDOWN                     = (NM_FIRST-15);      // uses NMKEY struct
         NM_RELEASEDCAPTURE             = (NM_FIRST-16);
         NM_SETCURSOR                   = (NM_FIRST-17);      // uses NMMOUSE struct
         NM_CHAR                        = (NM_FIRST-18);      // uses NMCHAR struct
{$ENDIF}
{$ifdef ie401plus}
         NM_TOOLTIPSCREATED             = (NM_FIRST-19);      // notify of when the tooltips window is create
{$ENDIF}
{$ifdef ie5plus}
         NM_LDOWN                       = (NM_FIRST-20);
         NM_RDOWN                       = (NM_FIRST-21);
         NM_THEMECHANGED                = (NM_FIRST-22);
{$ENDIF}
{$ifdef win32vista}
	 NM_FONTCHANGED                  = (NM_FIRST-23);
	 NM_CUSTOMTEXT                   = (NM_FIRST-24);   // uses NMCUSTOMTEXT struct
	 NM_TVSTATEIMAGECHANGING         = (NM_FIRST-24);   // uses NMTVSTATEIMAGECHANGING struct, defined after HTREEITEM
{$endif}

{$IFNDEF CCSIZEOF_STRUCT}

// Macro 10
// #define CCSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
{$ENDIF}

//====== Generic WM_NOTIFY notification structures ============================
{$ifdef ie401plus}

TYPE

         tagNMTOOLTIPSCREATED = Record
                                 hdr          : NMHDR;
                                 hwndToolTips : HWND;
                                 END;
         NMTOOLTIPSCREATED    = tagNMTOOLTIPSCREATED;
         LPNMTOOLTIPSCREATED  = ^tagNMTOOLTIPSCREATED;
         TNMTOOLTIPSCREATED   = tagNMTOOLTIPSCREATED;
         PNMTOOLTIPSCREATED   = ^tagNMTOOLTIPSCREATED;

{$ENDIF}

{$ifdef ie4plus}
Type
         tagNMMOUSE           = Record
                                 hdr          : NMHDR;
                                 dwItemSpec   : DWORD_PTR;
                                 dwItemData   : DWORD_PTR;
                                 pt           : POINT;
                                 dwHitInfo    : LPARAM;          // any specifics about where on the item or control the mouse is
                                 END;
         NMMOUSE              = tagNMMOUSE;
         LPNMMOUSE            = ^tagNMMOUSE;
         TNMMOUSE             = tagNMMOUSE;
         PNMMOUSE             = ^tagNMMOUSE;


         NMCLICK              = NMMOUSE;
         LPNMCLICK            = LPNMMOUSE;

// Generic structure to request an object of a specific type.

         tagNMOBJECTNOTIFY    = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
{$IFDEF __IID_DEFINED__}
                                 piid         : PIID;
{$ELSE}
                                 piid         : Pointer;
{$ENDIF}
                                 pObject      : Pointer;
                                 hResult      : HRESULT;
                                 dwFlags      : DWORD;          // control specific flags (hints as to where in iItem it hit)
                                 END;
         NMOBJECTNOTIFY       = tagNMOBJECTNOTIFY;
         LPNMOBJECTNOTIFY     = ^tagNMOBJECTNOTIFY;
         TNMOBJECTNOTIFY      = tagNMOBJECTNOTIFY;
         PNMOBJECTNOTIFY      = ^tagNMOBJECTNOTIFY;


// Generic structure for a key

         tagNMKEY             = Record
                                 hdr          : NMHDR;
                                 nVKey        : UINT;
                                 uFlags       : UINT;
                                 END;
         NMKEY                = tagNMKEY;
         LPNMKEY              = ^tagNMKEY;
         TNMKEY               = tagNMKEY;
         PNMKEY               = ^tagNMKEY;


// Generic structure for a character

         tagNMCHAR            = Record
                                 hdr          : NMHDR;
                                 ch           : UINT;
                                 dwItemPrev   : DWORD;          // Item previously selected
                                 dwItemNext   : DWORD;          // Item to be selected
                                 END;
         NMCHAR               = tagNMCHAR;
         LPNMCHAR             = ^tagNMCHAR;
         TNMCHAR              = tagNMCHAR;
         PNMCHAR              = ^tagNMCHAR;

{$ifdef win32vista}
         tagNMCUSTOMTEXT      = Record
			         hdr      : NMHDR;
				 hDC      : HDC;
                                 lpString : LPCWSTR;
			         nCount   : cint;
			         lpRect   : LPRECT; 
			         uFormat  : UINT;
			         fLink    : bool;
				end;

         NMCUSTOMTEXT 		= tagNMCUSTOMTEXT;
	 LPNMCUSTOMTEXT 	= ^tagNMCUSTOMTEXT;
	 TNMCUSTOMTEXT 		= tagNMCUSTOMTEXT;
	 PNMCUSTOMTEXT 		= LPNMCUSTOMTEXT;
{$endif}
{$ENDIF}           // _WIN32_IE >= 0x0400


{$ifdef ie3plus}
//==================== CUSTOM DRAW ==========================================

// custom draw return flags
// values under 0x00010000 are reserved for global custom draw values.
// above that are for specific controls
CONST
         CDRF_DODEFAULT                 = $00000000;
         CDRF_NEWFONT                   = $00000002;
         CDRF_SKIPDEFAULT               = $00000004;


         CDRF_NOTIFYPOSTPAINT           = $00000010;
         CDRF_NOTIFYITEMDRAW            = $00000020;
{$ifdef ie4plus}
         CDRF_NOTIFYSUBITEMDRAW         = $00000020;          // flags are the same, we can distinguish by context
{$ENDIF}
         CDRF_NOTIFYPOSTERASE           = $00000040;
         CDRF_NOTIFYITEMERASE           = $00000080;  // according to lazarus headers. Can't find in sdk 6.0 (Vista sdk)


// drawstage flags
// values under 0x00010000 are reserved for global custom draw values.
// above that are for specific controls
         CDDS_PREPAINT                  = $00000001;
         CDDS_POSTPAINT                 = $00000002;
         CDDS_PREERASE                  = $00000003;
         CDDS_POSTERASE                 = $00000004;
// the 0x000010000 bit means it's individual item specific
         CDDS_ITEM                      = $00010000;
         CDDS_ITEMPREPAINT              = (CDDS_ITEM  OR  CDDS_PREPAINT);
         CDDS_ITEMPOSTPAINT             = (CDDS_ITEM  OR  CDDS_POSTPAINT);
         CDDS_ITEMPREERASE              = (CDDS_ITEM  OR  CDDS_PREERASE);
         CDDS_ITEMPOSTERASE             = (CDDS_ITEM  OR  CDDS_POSTERASE);
{$ifdef ie4plus}
         CDDS_SUBITEM                   = $00020000;
{$ENDIF}

// itemState flags
         CDIS_SELECTED                  = $0001;
         CDIS_GRAYED                    = $0002;
         CDIS_DISABLED                  = $0004;
         CDIS_CHECKED                   = $0008;
         CDIS_FOCUS                     = $0010;
         CDIS_DEFAULT                   = $0020;
         CDIS_HOT                       = $0040;
         CDIS_MARKED                    = $0080;
         CDIS_INDETERMINATE             = $0100;
{$ifdef win32xp}
         CDIS_SHOWKEYBOARDCUES          = $0200;
{$ENDIF}
{$ifdef win32vista}
	 CDIS_NEARHOT            	= $0400;
	 CDIS_OTHERSIDEHOT       	= $0800;
	 CDIS_DROPHILITED        	= $1000;
{$endif}

TYPE

         tagNMCUSTOMDRAWINFO  = Record
                                 hdr          : NMHDR;
                                 dwDrawStage  : DWORD;
                                 hdc          : HDC;
                                 rc           : RECT;
                                 dwItemSpec   : DWORD_PTR;          // this is control specific, but it's how to specify an item.  valid only with CDDS_ITEM bit set
                                 uItemState   : UINT;
                                 lItemlParam  : LPARAM;
                                 END;
         NMCUSTOMDRAW         = tagNMCUSTOMDRAWINFO;
         LPNMCUSTOMDRAW       = ^tagNMCUSTOMDRAWINFO;
         TNMCUSTOMDRAWINFO    = tagNMCUSTOMDRAWINFO;
         PNMCUSTOMDRAWINFO    = ^tagNMCUSTOMDRAWINFO;
         PNMCustomDraw        = PNMCUSTOMDRAWINFO;
         TNMCustomDraw        = tagNMCUSTOMDRAWINFO;

         tagNMTTCUSTOMDRAW    = Record
                                 nmcd         : NMCUSTOMDRAW;
                                 uDrawFlags   : UINT;
                                 END;
         NMTTCUSTOMDRAW       = tagNMTTCUSTOMDRAW;
         LPNMTTCUSTOMDRAW     = ^tagNMTTCUSTOMDRAW;
         TNMTTCUSTOMDRAW      = tagNMTTCUSTOMDRAW;
         PNMTTCUSTOMDRAW      = ^tagNMTTCUSTOMDRAW;


{$ENDIF}      // _WIN32_IE >= 0x0300


//====== IMAGE APIS ===========================================================

{$IFNDEF NOIMAGEAPIS}


CONST
         CLR_NONE                       = DWORD($FFFFFFFF);
         CLR_DEFAULT                    = DWORD($FF000000);


{$IFNDEF IMAGELISTDRAWPARAMS}
{$ifdef ie3plus}
TYPE
         _IMAGELISTDRAWPARAMS = Record
                                 cbSize       : DWORD;
                                 himl         : HIMAGELIST;
                                 i            : cint;
                                 hdcDst       : HDC;
                                 x            : cint;
                                 y            : cint;
                                 cx           : cint;
                                 cy           : cint;
                                 xBitmap      : cint;          // x offest from the upperleft of bitmap
                                 yBitmap      : cint;          // y offset from the upperleft of bitmap
                                 rgbBk        : COLORREF;
                                 rgbFg        : COLORREF;
                                 fStyle       : UINT;
                                 dwRop        : DWORD;
{$ifdef win32xp}
                                 fState       : DWORD;
                                 Frame        : DWORD;
                                 crEffect     : COLORREF;
{$ENDIF}
                                 END;
         tagIMAGELISTDRAWPARAMS  = _IMAGELISTDRAWPARAMS;
         IMAGELISTDRAWPARAMS  = _IMAGELISTDRAWPARAMS;
         LPIMAGELISTDRAWPARAMS = ^_IMAGELISTDRAWPARAMS;
         TIMAGELISTDRAWPARAMS = _IMAGELISTDRAWPARAMS;
         PIMAGELISTDRAWPARAMS = ^_IMAGELISTDRAWPARAMS;

// #define IMAGELISTDRAWPARAMS_V3_SIZE CCSIZEOF_STRUCT(IMAGELISTDRAWPARAMS, dwRop)

{$ENDIF}      // _WIN32_IE >= 0x0300
{$ENDIF}


CONST
         ILC_MASK                       = $00000001;
         ILC_COLOR                      = $00000000;
         ILC_COLORDDB                   = $000000FE;
         ILC_COLOR4                     = $00000004;
         ILC_COLOR8                     = $00000008;
         ILC_COLOR16                    = $00000010;
         ILC_COLOR24                    = $00000018;
         ILC_COLOR32                    = $00000020;
         ILC_PALETTE                    = $00000800;          // (not implemented)
{$ifdef win32xp}
         ILC_MIRROR                     = $00002000;          // Mirror the icons contained, if the process is mirrored
         ILC_PERITEMMIRROR              = $00008000;          // Causes the mirroring code to mirror each item when inserting a set of images, verses the whole strip
{$ENDIF}
{$ifdef win32vista}
         ILC_ORIGINALSIZE        	= $00010000;      // Imagelist should accept smaller than set images and apply OriginalSize based on image added
         ILC_HIGHQUALITYSCALE    	= $00020000;      // Imagelist should enable use of the high quality scaler.
{$endif}

function ImageList_Create(cx:cint;cy:cint;flags:UINT;cInitial:cint;cGrow:cint):HIMAGELIST; stdcall; external commctrldll name 'ImageList_Create';
function ImageList_Destroy(himl:HIMAGELIST):BOOL; stdcall; external commctrldll name 'ImageList_Destroy';

function ImageList_GetImageCount(himl:HIMAGELIST):cint; stdcall; external commctrldll name 'ImageList_GetImageCount';
{$ifdef ie3plus}
function ImageList_SetImageCount(himl:HIMAGELIST;uNewCount:UINT):BOOL; stdcall; external commctrldll name 'ImageList_SetImageCount';
{$ENDIF}

function ImageList_Add(himl:HIMAGELIST;hbmImage:HBITMAP;hbmMask:HBITMAP):cint; stdcall; external commctrldll name 'ImageList_Add';

function ImageList_ReplaceIcon(himl:HIMAGELIST;i:cint;hicon:HICON):cint; stdcall; external commctrldll name 'ImageList_ReplaceIcon';
function ImageList_SetBkColor(himl:HIMAGELIST;clrBk:COLORREF):COLORREF; stdcall; external commctrldll name 'ImageList_SetBkColor';
function ImageList_GetBkColor(himl:HIMAGELIST):COLORREF; stdcall; external commctrldll name 'ImageList_GetBkColor';
function ImageList_SetOverlayImage(himl:HIMAGELIST;iImage:cint;iOverlay:cint):BOOL; stdcall; external commctrldll name 'ImageList_SetOverlayImage';

// Macro 11
Function ImageList_AddIcon(Himl:HIMAGELIST;hicon:HICON):cint;

CONST
         ILD_NORMAL                     = $00000000;
         ILD_TRANSPARENT                = $00000001;
         ILD_MASK                       = $00000010;
         ILD_IMAGE                      = $00000020;
{$ifdef ie3plus}
         ILD_ROP                        = $00000040;
{$ENDIF}
         ILD_BLEND25                    = $00000002;
         ILD_BLEND50                    = $00000004;
         ILD_OVERLAYMASK                = $00000F00;

// Macro 12
// #define INDEXTOOVERLAYMASK(i)   ((i) << 8)

CONST
         ILD_PRESERVEALPHA              = $00001000;          // This preserves the alpha channel in dest
         ILD_SCALE                      = $00002000;          // Causes the image to be scaled to cx, cy instead of clipped
         ILD_DPISCALE                   = $00004000;
{$ifdef win32vista}
	 ILD_ASYNC               	= $00008000;
{$endif}

         ILD_SELECTED                   = ILD_BLEND50;
         ILD_FOCUS                      = ILD_BLEND25;
         ILD_BLEND                      = ILD_BLEND50;
         CLR_HILIGHT                    = CLR_DEFAULT;

         ILS_NORMAL                     = $00000000;
         ILS_GLOW                       = $00000001;
         ILS_SHADOW                     = $00000002;
         ILS_SATURATE                   = $00000004;
         ILS_ALPHA                      = $00000008;

{$ifdef win32vista}
         ILGT_NORMAL             	= $00000000;
	 ILGT_ASYNC              	= $00000001;
{$endif}

function ImageList_Draw(himl:HIMAGELIST;i:cint;hdcDst:HDC;x:cint;y:cint;fStyle:UINT):BOOL; stdcall; external commctrldll name 'ImageList_Draw';


{$IFDEF _WIN32}

{$ifdef win32vista}
const 
  HBITMAP_CALLBACK               =HBITMAP(-1);       // only for SparseImageList
{$endif}
function ImageList_Replace(himl:HIMAGELIST;i:cint;hbmImage:HBITMAP;hbmMask:HBITMAP):BOOL; stdcall; external commctrldll name 'ImageList_Replace';

function ImageList_AddMasked(himl:HIMAGELIST;hbmImage:HBITMAP;crMask:COLORREF):cint; stdcall; external commctrldll name 'ImageList_AddMasked';
function ImageList_DrawEx(himl:HIMAGELIST;i:cint;hdcDst:HDC;x:cint;y:cint;dx:cint;dy:cint;rgbBk:COLORREF;rgbFg:COLORREF;fStyle:UINT):BOOL; stdcall; external commctrldll name 'ImageList_DrawEx';
{$ifdef ie3plus}
function ImageList_DrawIndirect(pimldp:PIMAGELISTDRAWPARAMS):BOOL; stdcall; external commctrldll name 'ImageList_DrawIndirect';
{$ENDIF}
function ImageList_Remove(himl:HIMAGELIST;i:cint):BOOL; stdcall; external commctrldll name 'ImageList_Remove';
function ImageList_GetIcon(himl:HIMAGELIST;i:cint;flags:UINT):HICON; stdcall; external commctrldll name 'ImageList_GetIcon';
function ImageList_LoadImageA(hi:HINST;lpbmp:LPCSTR;cx:cint;cGrow:cint;crMask:COLORREF;uType:UINT;uFlags:UINT):HIMAGELIST; stdcall; external commctrldll name 'ImageList_LoadImageA';
function ImageList_LoadImageW(hi:HINST;lpbmp:LPCWSTR;cx:cint;cGrow:cint;crMask:COLORREF;uType:UINT;uFlags:UINT):HIMAGELIST; stdcall; external commctrldll name 'ImageList_LoadImageW';
function ImageList_LoadImage(hi:HINST;lpbmp:LPCSTR;cx:cint;cGrow:cint;crMask:COLORREF;uType:UINT;uFlags:UINT):HIMAGELIST; stdcall; external commctrldll name 'ImageList_LoadImageA';
function ImageList_LoadImage(hi:HINST;lpbmp:LPCWSTR;cx:cint;cGrow:cint;crMask:COLORREF;uType:UINT;uFlags:UINT):HIMAGELIST; stdcall; external commctrldll name 'ImageList_LoadImageW';

{$IFDEF UNICODE}
// function ImageList_LoadImageW(hi:HINSTANCE;lpbmp:LPCWSTR;cx:cint;cGrow:cint;crMask:COLORREF;uType:UINT;uFlags:UINT):HIMAGELIST; external commctrldll name 'ImageList_LoadImageW';
{$ELSE}
// function ImageList_LoadImageA(hi:HINSTANCE;lpbmp:LPCSTR;cx:cint;cGrow:cint;crMask:COLORREF;uType:UINT;uFlags:UINT):HIMAGELIST; external commctrldll name 'ImageList_LoadImageA';
{$ENDIF}

{$ifdef ie3plus}

CONST
         ILCF_MOVE                      = ($00000000);
         ILCF_SWAP                      = ($00000001);

function ImageList_Copy(himlDst:HIMAGELIST;iDst:cint;himlSrc:HIMAGELIST;iSrc:cint;uFlags:UINT):BOOL; stdcall; external commctrldll name 'ImageList_Copy';
{$ENDIF}

function ImageList_BeginDrag(himlTrack:HIMAGELIST;iTrack:cint;dxHotspot:cint;dyHotspot:cint):BOOL; stdcall; external commctrldll name 'ImageList_BeginDrag';
function ImageList_EndDrag:BOOL; stdcall; external commctrldll name 'ImageList_EndDrag';
function ImageList_DragEnter(hwndLock:HWND;x:cint;y:cint):BOOL; stdcall; external commctrldll name 'ImageList_DragEnter';
function ImageList_DragLeave(hwndLock:HWND):BOOL; stdcall; external commctrldll name 'ImageList_DragLeave';
function ImageList_DragMove(x:cint;y:cint):BOOL; stdcall; external commctrldll name 'ImageList_DragMove';
function ImageList_SetDragCursorImage(himlDrag:HIMAGELIST;iDrag:cint;dxHotspot:cint;dyHotspot:cint):BOOL; stdcall; external commctrldll name 'ImageList_SetDragCursorImage';

function ImageList_DragShowNolock(fShow:BOOL):BOOL; stdcall; external commctrldll name 'ImageList_DragShowNolock';
function ImageList_GetDragImage(ppt:PPOINT;pptHotspot:PPOINT):HIMAGELIST; stdcall; external commctrldll name 'ImageList_GetDragImage';

// Macro 13
Procedure ImageList_RemoveAll(himl:HIMAGELIST);

// Macro 14
function ImageList_ExtractIcon(hi:longint; himl:HIMAGELIST;i:longint):HICON;


// Macro 15
Procedure ImageList_LoadBitmap(hi:HInst;bmp:LPCTSTR;cx:cint;cGrow:cint;crMask:COLORREF);

{$IFDEF __IStream_INTERFACE_DEFINED__}

function ImageList_Read(pstm:ISTREAM):HIMAGELIST; stdcall; external commctrldll name 'ImageList_Read';
function ImageList_Write(himl:HIMAGELIST;pstm:ISTREAM):BOOL; stdcall; external commctrldll name 'ImageList_Write';

{$ifdef Win32XP}

CONST
         ILP_NORMAL                     = 0;                  // Writes or reads the stream using new sematics for this version of comctl32
         ILP_DOWNLEVEL                  = 1;                  // Write or reads the stream using downlevel sematics.


function ImageList_ReadEx(dwFlags:DWORD;pstm:ISTREAM;riid:REFIID;ppv:PPointer):HRESULT; stdcall; external commctrldll name 'ImageList_ReadEx';
function ImageList_WriteEx(himl:HIMAGELIST;dwFlags:DWORD;pstm:ISTREAM):HRESULT; stdcall; external commctrldll name 'ImageList_WriteEx';
{$ENDIF}

{$ENDIF}

{$IFNDEF IMAGEINFO}
TYPE

         _IMAGEINFO           = Record
                                 hbmImage     : HBITMAP;
                                 hbmMask      : HBITMAP;
                                 Unused1      : cint;
                                 Unused2      : cint;
                                 rcImage      : RECT;
                                 END;
         IMAGEINFO            = _IMAGEINFO;
         LPIMAGEINFO          = ^_IMAGEINFO;
         TIMAGEINFO           = _IMAGEINFO;
         PIMAGEINFO           = ^_IMAGEINFO;

{$ENDIF}

function ImageList_GetIconSize(himl:HIMAGELIST;cx:Pint;cy:Pint):BOOL; stdcall; external commctrldll name 'ImageList_GetIconSize';
function ImageList_GetIconSize(himl:HIMAGELIST;var cx:cint;var cy:cint):BOOL; stdcall; external commctrldll name 'ImageList_GetIconSize';
function ImageList_SetIconSize(himl:HIMAGELIST;cx:cint;cy:cint):BOOL; stdcall; external commctrldll name 'ImageList_SetIconSize';
function ImageList_GetImageInfo(himl:HIMAGELIST;i:cint;pImageInfo:PIMAGEINFO):BOOL; stdcall; external commctrldll name 'ImageList_GetImageInfo';
function ImageList_GetImageInfo(himl:HIMAGELIST;i:cint;var pImageInfo:_IMAGEINFO):BOOL; stdcall; external commctrldll name 'ImageList_GetImageInfo';
function ImageList_Merge(himl1:HIMAGELIST;i1:cint;himl2:HIMAGELIST;i2:cint;dx:cint;dy:cint):HIMAGELIST; stdcall; external commctrldll name 'ImageList_Merge';
{$ifdef ie4plus}
function ImageList_Duplicate(himl:HIMAGELIST):HIMAGELIST; stdcall; external commctrldll name 'ImageList_Duplicate';
{$ENDIF}


{$ENDIF}


{$ENDIF}


//====== HEADER CONTROL =======================================================

{$IFNDEF NOHEADER}

{$IFDEF _WIN32}

CONST
         WC_HEADERA                     = 'SysHeader32';
         WC_HEADERW                     = {L}'SysHeader32';
{$IFDEF UNICODE}
         WC_HEADER           = WC_HEADERW;
{$ELSE}
         WC_HEADER           = WC_HEADERA;
{$ENDIF}

{$ELSE}

CONST
         WC_HEADER                      = 'SysHeader';
{$ENDIF}

CONST

// begin_r_commctrl

         HDS_HORZ                       = $0000;
         HDS_BUTTONS                    = $0002;
{$ifdef ie3plus}
         HDS_HOTTRACK                   = $0004;
{$ENDIF}
         HDS_HIDDEN                     = $0008;

{$ifdef ie3plus}
         HDS_DRAGDROP                   = $0040;
         HDS_FULLDRAG                   = $0080;
{$ENDIF}
{$ifdef ie5plus}
         HDS_FILTERBAR                  = $0100;
{$ENDIF}

{$ifdef win32xp}
         HDS_FLAT                       = $0200;
{$ENDIF}
{$ifdef win32vista}
         HDS_CHECKBOXES          	= $0400;
	 HDS_NOSIZING            	= $0800;
	 HDS_OVERFLOW            	= $1000;
{$endif}
// end_r_commctrl

{$ifdef ie5plus}

         HDFT_ISSTRING                  = $0000;              // HD_ITEM.pvFilter points to a HD_TEXTFILTER
         HDFT_ISNUMBER                  = $0001;              // HD_ITEM.pvFilter points to a INT

         HDFT_HASNOVALUE                = $8000;              // clear the filter, by setting this bit

TYPE


         _HD_TEXTFILTERA      = Record
                                 pszText      : LPSTR;          // [in] pointer to the buffer containing the filter (ANSI)
                                 cchTextMax   : cint;          // [in] max size of buffer/edit control buffer
                                 END;
         HD_TEXTFILTERA       = _HD_TEXTFILTERA;
         LPHD_TEXTFILTERA     = ^_HD_TEXTFILTERA;
         THD_TEXTFILTERA      = _HD_TEXTFILTERA;
         PHD_TEXTFILTERA      = ^_HD_TEXTFILTERA;


         _HD_TEXTFILTERW      = Record
                                 pszText      : LPWSTR;          // [in] pointer to the buffer contiaining the filter (UNICODE)
                                 cchTextMax   : cint;          // [in] max size of buffer/edit control buffer
                                 END;
         HD_TEXTFILTERW       = _HD_TEXTFILTERW;
         LPHD_TEXTFILTERW     = ^_HD_TEXTFILTERW;
         THD_TEXTFILTERW      = _HD_TEXTFILTERW;
         PHD_TEXTFILTERW      = ^_HD_TEXTFILTERW;

{$IFDEF UNICODE}
         HD_TEXTFILTER       = HD_TEXTFILTERW;
         HDTEXTFILTER        = HD_TEXTFILTERW;
         LPHD_TEXTFILTER     = LPHD_TEXTFILTERW;
         LPHDTEXTFILTER      = LPHD_TEXTFILTERW;
{$ELSE}
         HD_TEXTFILTER       = HD_TEXTFILTERA;
         HDTEXTFILTER        = HD_TEXTFILTERA;
         LPHD_TEXTFILTER     = LPHD_TEXTFILTERA;
         LPHDTEXTFILTER      = LPHD_TEXTFILTERA;
{$ENDIF}


{$ENDIF}  // _WIN32_IE >= 0x0500

TYPE


         _HD_ITEMA            = Record
                                 mask         : UINT;
                                 cxy          : cint;
                                 pszText      : LPSTR;
                                 hbm          : HBITMAP;
                                 cchTextMax   : cint;
                                 fmt          : cint;
                                 lParam       : LPARAM;
{$ifdef ie3plus}
                                 iImage       : cint;          // index of bitmap in ImageList
                                 iOrder       : cint;          // where to draw this item
{$ENDIF}
{$ifdef ie5plus}
                                 _type        : UINT;          // [in] filter type (defined what pvFilter is a pointer to)
                                 pvFilter     : Pointer;       // [in] fillter data see above
{$ENDIF}
{$ifdef win32vista}
			         state	      : UINT;
{$endif}
                                 END;
         HDITEMA              = _HD_ITEMA;
         pHDITEMA             = ^_HD_ITEMA;

         LPHDITEMA            = ^_HD_ITEMA;
         THD_ITEMA            = _HD_ITEMA;
         PHD_ITEMA            = ^_HD_ITEMA;
         THDItem              = THD_ITEMA;
         PHDItem              = PHD_ITEMA;


// #define HDITEMA_V1_SIZE CCSIZEOF_STRUCT(HDITEMA, lParam)
// #define HDITEMW_V1_SIZE CCSIZEOF_STRUCT(HDITEMW, lParam)


         _HD_ITEMW            = Record
                                 mask         : UINT;
                                 cxy          : cint;
                                 pszText      : LPWSTR;
                                 hbm          : HBITMAP;
                                 cchTextMax   : cint;
                                 fmt          : cint;
                                 lParam       : LPARAM;
{$ifdef ie3plus}
                                 iImage       : cint;          // index of bitmap in ImageList
                                 iOrder       : cint;
{$ENDIF}
{$ifdef ie5plus}
                                 _type        : UINT;          // [in] filter type (defined what pvFilter is a pointer to)
                                 pvFilter     : Pointer;       // [in] fillter data see above
{$ENDIF}
{$ifdef win32vista}
			         state	      : UINT;
{$endif}
                                 END;
         HDITEMW              = _HD_ITEMW;
         pHDITEMW             = ^_HD_ITEMW;
         LPHDITEMW            = ^_HD_ITEMW;
         THD_ITEMW            = _HD_ITEMW;
         PHD_ITEMW            = ^_HD_ITEMW;


TYPE
{$IFDEF UNICODE}
         HDITEM              = HDITEMW;
         LPHDITEM            = LPHDITEMW;
//       HDITEM_V1_SIZE                 = HDITEMW_V1_SIZE;
{$ELSE}

         HDITEM              = HDITEMA;
         LPHDITEM            = LPHDITEMA;
//       HDITEM_V1_SIZE                 = HDITEMA_V1_SIZE;
{$ENDIF}

         HD_ITEM                        = HDITEM;


CONST

         HDI_WIDTH                      = $0001;
         HDI_HEIGHT                     = HDI_WIDTH;
         HDI_TEXT                       = $0002;
         HDI_FORMAT                     = $0004;
         HDI_LPARAM                     = $0008;
         HDI_BITMAP                     = $0010;
{$ifdef ie3plus}
         HDI_IMAGE                      = $0020;
         HDI_DI_SETITEM                 = $0040;
         HDI_ORDER                      = $0080;
{$ENDIF}
{$ifdef ie5plus}
         HDI_FILTER                     = $0100;
{$ENDIF}
{$ifdef win32vista}
	HDI_STATE               	= $0200;
{$endif}

         HDF_LEFT                       = $0000;
         HDF_RIGHT                      = $0001;
         HDF_CENTER                     = $0002;
         HDF_JUSTIFYMASK                = $0003;
         HDF_RTLREADING                 = $0004;

         HDF_OWNERDRAW                  = $8000;
         HDF_STRING                     = $4000;
         HDF_BITMAP                     = $2000;
{$ifdef ie3plus}
         HDF_BITMAP_ON_RIGHT            = $1000;
         HDF_IMAGE                      = $0800;
{$ENDIF}

{$ifdef win32xp}
         HDF_SORTUP                     = $0400;
         HDF_SORTDOWN                   = $0200;
{$ENDIF}
{$ifdef win32vista}
	 HDF_CHECKBOX            	= $0040;
	 HDF_CHECKED             	= $0080;
	 HDF_FIXEDWIDTH          	= $0100; // Can't resize the column; same as LVCFMT_FIXED_WIDTH
	 HDF_SPLITBUTTON      		= $1000000; // Column is a split button; same as LVCFMT_SPLITBUTTON
{$endif}

{$ifdef win32vista}
	 HDIS_FOCUSED            	= $00000001;
{$endif}


         HDM_GETITEMCOUNT               = (HDM_FIRST + 0);

// Macro 16
Function Header_GetItemCount( hwndHD : hwnd):cint;

CONST
         HDM_INSERTITEMA                = (HDM_FIRST + 1);
         HDM_INSERTITEMW                = (HDM_FIRST + 10);

{$IFDEF UNICODE}
         HDM_INSERTITEM                 = HDM_INSERTITEMW;
{$ELSE}
         HDM_INSERTITEM                 = HDM_INSERTITEMA;
{$ENDIF}

// Macro 17
Function Header_InsertItem( hwndHD : hwnd; i : cint;const phdi : HD_ITEM ):cint;


CONST
         HDM_DELETEITEM                 = (HDM_FIRST + 2);

// Macro 18
Function Header_DeleteItem( hwndHD : hwnd; i : cint):BOOL;

CONST
         HDM_GETITEMA                   = (HDM_FIRST + 3);
         HDM_GETITEMW                   = (HDM_FIRST + 11);

{$IFDEF UNICODE}
         HDM_GETITEM                    = HDM_GETITEMW;
{$ELSE}
         HDM_GETITEM                    = HDM_GETITEMA;
{$ENDIF}

// Macro 19
Function Header_GetItem( hwndHD : hwnd; i : cint;var  phdi : HD_ITEM ):BOOL;

CONST
         HDM_SETITEMA                   = (HDM_FIRST + 4);
         HDM_SETITEMW                   = (HDM_FIRST + 12);

{$IFDEF UNICODE}
         HDM_SETITEM                    = HDM_SETITEMW;
{$ELSE}
         HDM_SETITEM                    = HDM_SETITEMA;
{$ENDIF}

// Macro 20
Function Header_SetItem( hwndHD : hwnd; i : cint;const  phdi : HD_ITEM ):BOOL;

TYPE

         _HD_LAYOUT           = Record
                                 prc          : PRECT;
                                 pwpos        : PWINDOWPOS;
                                 END;
         HDLAYOUT             = _HD_LAYOUT;
         LPHDLAYOUT           = ^_HD_LAYOUT;
         THD_LAYOUT           = _HD_LAYOUT;
         PHD_LAYOUT           = ^_HD_LAYOUT;

CONST
         HDM_LAYOUT                     = (HDM_FIRST + 5);

{$ifdef ie3plus}
TYPE
         HD_LAYOUT                      = HDLAYOUT;
{$ELSE}
//         HDLAYOUT                       = HD_LAYOUT;
{$ENDIF}


// Macro 21
Function Header_Layout( hwndHD : hwnd;var  playout : HD_LAYOUT ):BOOL;

CONST
         HHT_NOWHERE                    = $0001;
         HHT_ONHEADER                   = $0002;
         HHT_ONDIVIDER                  = $0004;
         HHT_ONDIVOPEN                  = $0008;
{$ifdef ie5plus}
         HHT_ONFILTER                   = $0010;
         HHT_ONFILTERBUTTON             = $0020;
{$ENDIF}
         HHT_ABOVE                      = $0100;
         HHT_BELOW                      = $0200;
         HHT_TORIGHT                    = $0400;
         HHT_TOLEFT                     = $0800;
{$ifdef win32vista}
	 HHT_ONITEMSTATEICON     	= $1000;
	 HHT_ONDROPDOWN          	= $2000;
	 HHT_ONOVERFLOW          	= $4000;
{$endif}

TYPE
         _HD_HITTESTINFO      = Record
                                  case integer of
                                    1: (pt           : POINT;flags        : UINT;iItem        : cint);
                                    { delphi }
                                    2: (point        : POINT;dummyflags   : UINT;Item        : cint);
                                 END;
         HDHITTESTINFO        = _HD_HITTESTINFO;
         HD_HITTESTINFO       = _HD_HITTESTINFO;
         LPHDHITTESTINFO      = ^_HD_HITTESTINFO;
         THD_HITTESTINFO      = _HD_HITTESTINFO;
         PHD_HITTESTINFO      = ^_HD_HITTESTINFO;
         THDHitTestInfo       = THD_HITTESTINFO;
         PHDHitTestInfo       = LPHDHITTESTINFO;


CONST
         HDM_HITTEST                    = (HDM_FIRST + 6);

{$ifdef ie3plus}

         HDM_GETITEMRECT                = (HDM_FIRST + 7);

// Macro 22
Function Header_GetItemRect( hwnd : hwnd; iItem : WPARAM; lprc : LPARAM):BOOL;

CONST
         HDM_SETIMAGELIST               = (HDM_FIRST + 8);

// Macro 23
Function Header_SetImageList( hwnd : hwnd; himl : LPARAM):HIMAGELIST;

CONST
         HDM_GETIMAGELIST               = (HDM_FIRST + 9);

// Macro 24
Function Header_GetImageList( hwnd : hwnd):HIMAGELIST;

CONST
         HDM_ORDERTOINDEX               = (HDM_FIRST + 15);

// Macro 25
Function Header_OrderToIndex( hwnd : hwnd; i : WPARAM):cint;

CONST
         HDM_CREATEDRAGIMAGE            = (HDM_FIRST + 16);   // wparam = which item (by index)

// Macro 26
Function Header_CreateDragImage( hwnd : hwnd; i : WPARAM):HIMAGELIST;

CONST
         HDM_GETORDERARRAY              = (HDM_FIRST + 17);

// Macro 27
Function Header_GetOrderArray( hwnd : hwnd; iCount : WPARAM; lpi : LPARAM):BOOL;

CONST
         HDM_SETORDERARRAY              = (HDM_FIRST + 18);

// Macro 28
Function Header_SetOrderArray( hwnd : hwnd; iCount : WPARAM; lpi : PInteger):BOOL;inline;

// lparam = int array of size HDM_GETITEMCOUNT
// the array specifies the order that all items should be displayed.
// e.g.  { 2, 0, 1}
// says the index 2 item should be shown in the 0ths position
//      index 0 should be shown in the 1st position
//      index 1 should be shown in the 2nd position


CONST
         HDM_SETHOTDIVIDER              = (HDM_FIRST + 19);

// Macro 29
Function Header_SetHotDivider( hwnd : hwnd; fPos : WPARAM; dw : LPARAM):cint;

// convenience message for external dragdrop
// wParam = BOOL  specifying whether the lParam is a dwPos of the cursor
//              position or the index of which divider to hotlight
// lParam = depends on wParam  (-1 and wParm = FALSE turns off hotlight)
{$ENDIF}      // _WIN32_IE >= 0x0300

{$ifdef ie5plus}
CONST
         HDM_SETBITMAPMARGIN            = (HDM_FIRST + 20);

// Macro 30
Function Header_SetBitmapMargin( hwnd : hwnd; iWidth : WPARAM):cint;

CONST
         HDM_GETBITMAPMARGIN            = (HDM_FIRST + 21);

// Macro 31
Function Header_GetBitmapMargin( hwnd : hwnd):cint;

{$ENDIF}


{$ifdef ie4plus}

CONST
         HDM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;

// Macro 32
Function Header_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;

CONST
         HDM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;

// Macro 33
Function Header_GetUnicodeFormat( hwnd : hwnd):BOOL;

{$ENDIF}

{$ifdef ie5plus}
CONST
         HDM_SETFILTERCHANGETIMEOUT     = (HDM_FIRST+22);

// Macro 34
Function Header_SetFilterChangeTimeout( hwnd : hwnd; i : LPARAM):cint;

CONST
         HDM_EDITFILTER                 = (HDM_FIRST+23);

// Macro 35
Function Header_EditFilter( hwnd : hwnd; i : WPARAM; fDiscardChanges :cint ):cint;


// Clear filter takes -1 as a column value to indicate that all
// the filter should be cleared.  When this happens you will
// only receive a single filter changed notification.

CONST
         HDM_CLEARFILTER                = (HDM_FIRST+24);

// Macro 36
Function Header_ClearFilter( hwnd : hwnd; i : WPARAM):cint;

// Macro 37
Function Header_ClearAllFilters( hwnd : hwnd):cint;

{$ENDIF}

{$ifdef win32vista}
//  HDM_TRANSLATEACCELERATOR    = CCM_TRANSLATEACCELERATOR; // CCM_* not defined anywhere yet in w7 sdk

const
   HDM_GETITEMDROPDOWNRECT = (HDM_FIRST+25);
   HDM_GETOVERFLOWRECT     = (HDM_FIRST+26);
   HDM_GETFOCUSEDITEM      = (HDM_FIRST+27);
   HDM_SETFOCUSEDITEM      = (HDM_FIRST+28);

// macro 37a through 37d
function Header_GetItemDropDownRect(hwnd : hwnd;iItem:cint; lprc:lprect):bool;
function Header_GetOverflowRect( hwnd : hwnd; lprc:lprect):bool;
function Header_GetFocusedItem (hwnd : hwnd):cint;
function Header_SetFocusedItem (hwnd:hwnd; iItem:cint):BOOL;
{$endif}

CONST
         HDN_ITEMCHANGINGA              = (HDN_FIRST-0);
         HDN_ITEMCHANGINGW              = (HDN_FIRST-20);
         HDN_ITEMCHANGEDA               = (HDN_FIRST-1);
         HDN_ITEMCHANGEDW               = (HDN_FIRST-21);
         HDN_ITEMCLICKA                 = (HDN_FIRST-2);
         HDN_ITEMCLICKW                 = (HDN_FIRST-22);
         HDN_ITEMDBLCLICKA              = (HDN_FIRST-3);
         HDN_ITEMDBLCLICKW              = (HDN_FIRST-23);
         HDN_DIVIDERDBLCLICKA           = (HDN_FIRST-5);
         HDN_DIVIDERDBLCLICKW           = (HDN_FIRST-25);
         HDN_BEGINTRACKA                = (HDN_FIRST-6);
         HDN_BEGINTRACKW                = (HDN_FIRST-26);
         HDN_ENDTRACKA                  = (HDN_FIRST-7);
         HDN_ENDTRACKW                  = (HDN_FIRST-27);
         HDN_TRACKA                     = (HDN_FIRST-8);
         HDN_TRACKW                     = (HDN_FIRST-28);
{$ifdef ie3plus}
         HDN_GETDISPINFOA               = (HDN_FIRST-9);
         HDN_GETDISPINFOW               = (HDN_FIRST-29);
         HDN_BEGINDRAG                  = (HDN_FIRST-10);
         HDN_ENDDRAG                    = (HDN_FIRST-11);
{$ENDIF}
{$ifdef ie5plus}
         HDN_FILTERCHANGE               = (HDN_FIRST-12);
         HDN_FILTERBTNCLICK             = (HDN_FIRST-13);
{$ENDIF}
{$ifdef win32vista}
         HDN_BEGINFILTEREDIT            = (HDN_FIRST-14);
         HDN_ENDFILTEREDIT              = (HDN_FIRST-15);

         HDN_ITEMSTATEICONCLICK         = (HDN_FIRST-16);
         HDN_ITEMKEYDOWN                = (HDN_FIRST-17);
         HDN_DROPDOWN                   = (HDN_FIRST-18);
         HDN_OVERFLOWCLICK              = (HDN_FIRST-19);
{$endif}

{$IFDEF UNICODE}
         HDN_ITEMCHANGING               = HDN_ITEMCHANGINGW;
         HDN_ITEMCHANGED                = HDN_ITEMCHANGEDW;
         HDN_ITEMCLICK                  = HDN_ITEMCLICKW;
         HDN_ITEMDBLCLICK    = HDN_ITEMDBLCLICKW;
         HDN_DIVIDERDBLCLICK = HDN_DIVIDERDBLCLICKW;
         HDN_BEGINTRACK      = HDN_BEGINTRACKW;
         HDN_ENDTRACK        = HDN_ENDTRACKW;
         HDN_TRACK           = HDN_TRACKW;
{$ifdef ie3plus}
         HDN_GETDISPINFO     = HDN_GETDISPINFOW;
{$ENDIF}
{$ELSE}
         HDN_ITEMCHANGING    = HDN_ITEMCHANGINGA;
         HDN_ITEMCHANGED     = HDN_ITEMCHANGEDA;
         HDN_ITEMCLICK       = HDN_ITEMCLICKA;
         HDN_ITEMDBLCLICK    = HDN_ITEMDBLCLICKA;
         HDN_DIVIDERDBLCLICK = HDN_DIVIDERDBLCLICKA;
         HDN_BEGINTRACK      = HDN_BEGINTRACKA;
         HDN_ENDTRACK        = HDN_ENDTRACKA;
         HDN_TRACK           = HDN_TRACKA;
{$ifdef ie3plus}
         HDN_GETDISPINFO     = HDN_GETDISPINFOA;
{$ENDIF}
{$ENDIF}

TYPE
         tagNMHEADERA         = Record
                                 hdr          : NMHDR;
                                 case integer of
                                   1: (iItem        : cint;iButton      : cint;pitem        : PHDITEMA);
                                   { delphi: }
                                   2: (Item        : cint;Button      : cint);
                                 END;
         NMHEADERA            = tagNMHEADERA;
         LPNMHEADERA          = ^tagNMHEADERA;
         TNMHEADERA           = tagNMHEADERA;
         PNMHEADERA           = ^tagNMHEADERA;

         tagNMHEADERW         = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 iButton      : cint;
                                 pitem        : PHDITEMW;
                                 END;
         NMHEADERW            = tagNMHEADERW;
         LPNMHEADERW          = ^tagNMHEADERW;
         TNMHEADERW           = tagNMHEADERW;
         PNMHEADERW           = ^tagNMHEADERW;


{$IFDEF UNICODE}
         NMHEADER            = NMHEADERW;
         LPNMHEADER          = LPNMHEADERW;
{$ELSE}
         NMHEADER            = NMHEADERA;
         LPNMHEADER          = LPNMHEADERA;
{$ENDIF}


{$ifdef ie3plus}
         HD_NOTIFYA          = NMHEADERA;
         HD_NOTIFYW          = NMHEADERW;
{$ELSE}
//         tagNMHEADERA        = _HD_NOTIFY;
//         NMHEADERA           = HD_NOTIFYA;
//         tagHMHEADERW        = _HD_NOTIFYW;
//         NMHEADERW           = HD_NOTIFYW;
{$ENDIF}

         HD_NOTIFY           = NMHEADER;
         THDNotify           = HD_NOTIFY;
         PHDNotify           = ^HD_NOTIFY;

         tagNMHDDISPINFOW     = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 mask         : UINT;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 lParam       : LPARAM;
                                 END;
         NMHDDISPINFOW        = tagNMHDDISPINFOW;
         LPNMHDDISPINFOW      = ^tagNMHDDISPINFOW;
         TNMHDDISPINFOW       = tagNMHDDISPINFOW;
         PNMHDDISPINFOW       = ^tagNMHDDISPINFOW;


         tagNMHDDISPINFOA     = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 mask         : UINT;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 lParam       : LPARAM;
                                 END;
         NMHDDISPINFOA        = tagNMHDDISPINFOA;
         LPNMHDDISPINFOA      = ^tagNMHDDISPINFOA;
         TNMHDDISPINFOA       = tagNMHDDISPINFOA;
         PNMHDDISPINFOA       = ^tagNMHDDISPINFOA;



{$IFDEF UNICODE}
         NMHDDISPINFO        = NMHDDISPINFOW;
         LPNMHDDISPINFO      = LPNMHDDISPINFOW;
{$ELSE}
         NMHDDISPINFO        = NMHDDISPINFOA;
         LPNMHDDISPINFO      = LPNMHDDISPINFOA;
{$ENDIF}

{$ifdef ie5plus}
         tagNMHDFILTERBTNCLICK = Record
                                  hdr          : NMHDR;
                                  iItem        : cint;
                                  rc           : RECT;
                                  END;
         NMHDFILTERBTNCLICK   = tagNMHDFILTERBTNCLICK;
         LPNMHDFILTERBTNCLICK = ^tagNMHDFILTERBTNCLICK;
         TNMHDFILTERBTNCLICK  = tagNMHDFILTERBTNCLICK;
         PNMHDFILTERBTNCLICK  = ^tagNMHDFILTERBTNCLICK;

{$ENDIF}

{$ENDIF}      // NOHEADER


//====== TOOLBAR CONTROL ======================================================

{$IFNDEF NOTOOLBAR}

{$IFDEF _WIN32}

CONST
         TOOLBARCLASSNAMEW              = {L}'ToolbarWindow32';
         TOOLBARCLASSNAMEA              = 'ToolbarWindow32';

{$IFDEF  UNICODE}
         TOOLBARCLASSNAME    = TOOLBARCLASSNAMEW;
{$ELSE}
         TOOLBARCLASSNAME    = TOOLBARCLASSNAMEA;
{$ENDIF}

{$ELSE}
CONST
         TOOLBARCLASSNAME               = 'ToolbarWindow';
{$ENDIF}

TYPE

         _TBBUTTON            = Record
                                 iBitmap      : cint;
                                 idCommand    : cint;
                                 fsState      : BYTE;
                                 fsStyle      : BYTE;
{$IFDEF _WIN64}
                                 bReserved    : ARRAY[0..5] OF BYTE;          // padding for alignment
{$ELSE}
{$IFDEF _WIN32}
                                 bReserved    : ARRAY[0..1] OF BYTE;          // padding for alignment
{$ENDIF}
{$ENDIF}
                                 dwData       : DWORD_PTR;
                                 iString      : INT_PTR;
                                 END;
         TBBUTTON             = _TBBUTTON;
         PTBBUTTON            = {NEAR} ^_TBBUTTON; // ???
         LPTBBUTTON           = ^_TBBUTTON;
         TTBBUTTON            = _TBBUTTON;
//         PTBBUTTON            = ^_TBBUTTON;

         LPCTBBUTTON          = {const} ^TBBUTTON;


         _COLORMAP            = Record
                                 xfrom         : COLORREF;
                                 xto           : COLORREF;
                                 END;
         COLORMAP             = _COLORMAP;
         LPCOLORMAP           = ^_COLORMAP;
         TCOLORMAP            = _COLORMAP;
         PCOLORMAP            = ^_COLORMAP;

function CreateToolbarEx(hwnd:HWND;ws:DWORD;wID:UINT;nBitmaps:cint;
                         hBMInst:HINST;
                         wBMID:UINT_PTR;lpButtons:LPCTBBUTTON;iNumButtons:cint;dxButton:cint;
                         dyButton:cint;dxBitmap:cint;dyBitmap:cint;uStructSize:UINT):HWND; stdcall; external commctrldll name 'CreateToolbarEx';

function CreateMappedBitmap(hInstance:HINST;idBitmap:INT_PTR;wFlags:UINT;lpColorMap:LPCOLORMAP;iNumMaps:cint):HBITMAP; stdcall; external commctrldll name 'CreateMappedBitmap';

CONST
         CMB_MASKED                     = $02;
         TBSTATE_CHECKED                = $01;
         TBSTATE_PRESSED                = $02;
         TBSTATE_ENABLED                = $04;
         TBSTATE_HIDDEN                 = $08;
         TBSTATE_INDETERMINATE          = $10;
         TBSTATE_WRAP                   = $20;
{$ifdef ie3plus}
         TBSTATE_ELLIPSES               = $40;
{$ENDIF}
{$ifdef ie4plus}
         TBSTATE_MARKED                 = $80;
{$ENDIF}

         TBSTYLE_BUTTON                 = $0000;              // obsolete; use BTNS_BUTTON instead
         TBSTYLE_SEP                    = $0001;              // obsolete; use BTNS_SEP instead
         TBSTYLE_CHECK                  = $0002;              // obsolete; use BTNS_CHECK instead
         TBSTYLE_GROUP                  = $0004;              // obsolete; use BTNS_GROUP instead
         TBSTYLE_CHECKGROUP             = (TBSTYLE_GROUP  OR  TBSTYLE_CHECK);// obsolete; use BTNS_CHECKGROUP instead
{$ifdef ie3plus}
         TBSTYLE_DROPDOWN               = $0008;              // obsolete; use BTNS_DROPDOWN instead
{$ENDIF}
{$ifdef ie4plus}
         TBSTYLE_AUTOSIZE               = $0010;              // obsolete; use BTNS_AUTOSIZE instead
         TBSTYLE_NOPREFIX               = $0020;              // obsolete; use BTNS_NOPREFIX instead
{$ENDIF}

         TBSTYLE_TOOLTIPS               = $0100;
         TBSTYLE_WRAPABLE               = $0200;
         TBSTYLE_ALTDRAG                = $0400;
{$ifdef ie3plus}
         TBSTYLE_FLAT                   = $0800;
         TBSTYLE_LIST                   = $1000;
         TBSTYLE_CUSTOMERASE            = $2000;
{$ENDIF}
{$ifdef ie4plus}
         TBSTYLE_REGISTERDROP           = $4000;
         TBSTYLE_TRANSPARENT            = $8000;
         TBSTYLE_EX_DRAWDDARROWS        = $00000001;
{$ENDIF}

{$ifdef ie5plus}
         BTNS_BUTTON                    = TBSTYLE_BUTTON;     // 0x0000
         BTNS_SEP                       = TBSTYLE_SEP;        // 0x0001
         BTNS_CHECK                     = TBSTYLE_CHECK;      // 0x0002
         BTNS_GROUP                     = TBSTYLE_GROUP;      // 0x0004
         BTNS_CHECKGROUP                = TBSTYLE_CHECKGROUP; // (TBSTYLE_GROUP | TBSTYLE_CHECK)
         BTNS_DROPDOWN                  = TBSTYLE_DROPDOWN;   // 0x0008
         BTNS_AUTOSIZE                  = TBSTYLE_AUTOSIZE;   // 0x0010; automatically calculate the cx of the button
         BTNS_NOPREFIX                  = TBSTYLE_NOPREFIX;   // 0x0020; this button should not have accel prefix
{$ifdef ie501plus}
         BTNS_SHOWTEXT                  = $0040;              // ignored unless TBSTYLE_EX_MIXEDBUTTONS is set
{$ENDIF}  // 0x0501
         BTNS_WHOLEDROPDOWN             = $0080;              // draw drop-down arrow, but without split arrow section
{$ENDIF}

{$ifdef ie501plus}
         TBSTYLE_EX_MIXEDBUTTONS        = $00000008;
         TBSTYLE_EX_HIDECLIPPEDBUTTONS  = $00000010;          // don't show partially obscured buttons
{$ENDIF}  // 0x0501


{$ifdef win32xp}
         TBSTYLE_EX_DOUBLEBUFFER        = $00000080;          // Double Buffer the toolbar
{$ENDIF}

{$ifdef ie4plus}
// Custom Draw Structure
TYPE

         _NMTBCUSTOMDRAW      = Record
                                 nmcd         : NMCUSTOMDRAW;
                                 hbrMonoDither : HBRUSH;
                                 hbrLines     : HBRUSH;          // For drawing lines on buttons
                                 hpenLines    : HPEN;          // For drawing lines on buttons
                                 clrText      : COLORREF;          // Color of text
                                 clrMark      : COLORREF;          // Color of text bk when marked. (only if TBSTATE_MARKED)
                                 clrTextHighlight : COLORREF;          // Color of text when highlighted
                                 clrBtnFace   : COLORREF;          // Background of the button
                                 clrBtnHighlight : COLORREF;          // 3D highlight
                                 clrHighlightHotTrack : COLORREF;          // In conjunction with fHighlightHotTrack

                                 rcText       : RECT;          // Rect for text
                                 nStringBkMode : cint;
                                 nHLStringBkMode : cint;
{$ifdef win32xp}
                                 iListGap     : cint;
{$ENDIF}
                                 END;
         NMTBCUSTOMDRAW       = _NMTBCUSTOMDRAW;
         LPNMTBCUSTOMDRAW     = ^_NMTBCUSTOMDRAW;
         TNMTBCUSTOMDRAW      = _NMTBCUSTOMDRAW;
         PNMTBCUSTOMDRAW      = ^_NMTBCUSTOMDRAW;


// Toolbar custom draw return flags

CONST
         TBCDRF_NOEDGES                 = $00010000;          // Don't draw button edges
         TBCDRF_HILITEHOTTRACK          = $00020000;          // Use color of the button bk when hottracked
         TBCDRF_NOOFFSET                = $00040000;          // Don't offset button if pressed
         TBCDRF_NOMARK                  = $00080000;          // Don't draw default highlight of image/text for TBSTATE_MARKED
         TBCDRF_NOETCHEDEFFECT          = $00100000;          // Don't draw etched effect for disabled items
{$ENDIF}

{$ifdef ie5plus}
         TBCDRF_BLENDICON               = $00200000;          // Use ILD_BLEND50 on the icon image
         TBCDRF_NOBACKGROUND            = $00400000;          // Use ILD_BLEND50 on the icon image
{$ENDIF}
{$ifdef win32vista}
	 TBCDRF_USECDCOLORS          	= $00800000;  // Use CustomDrawColors to RenderText regardless of VisualStyle
{$endif}

CONST
         TB_ENABLEBUTTON                = (WM_USER + 1);
         TB_CHECKBUTTON                 = (WM_USER + 2);
         TB_PRESSBUTTON                 = (WM_USER + 3);
         TB_HIDEBUTTON                  = (WM_USER + 4);
         TB_INDETERMINATE               = (WM_USER + 5);
{$ifdef ie4plus}
         TB_MARKBUTTON                  = (WM_USER + 6);
{$ENDIF}
         TB_ISBUTTONENABLED             = (WM_USER + 9);
         TB_ISBUTTONCHECKED             = (WM_USER + 10);
         TB_ISBUTTONPRESSED             = (WM_USER + 11);
         TB_ISBUTTONHIDDEN              = (WM_USER + 12);
         TB_ISBUTTONINDETERMINATE       = (WM_USER + 13);
{$ifdef ie4plus}
         TB_ISBUTTONHIGHLIGHTED         = (WM_USER + 14);
{$ENDIF}
         TB_SETSTATE                    = (WM_USER + 17);
         TB_GETSTATE                    = (WM_USER + 18);
         TB_ADDBITMAP                   = (WM_USER + 19);

{$IFDEF _WIN32}

TYPE

         tagTBADDBITMAP       = Record
                                 hInst        : HINST;
                                 nID          : UINT_PTR;
                                 END;
         TBADDBITMAP          = tagTBADDBITMAP;
         LPTBADDBITMAP        = ^tagTBADDBITMAP;
         TTBADDBITMAP         = tagTBADDBITMAP;
         PTBADDBITMAP         = ^tagTBADDBITMAP;



CONST
         HINST_COMMCTRL                 = HINST(-1);
         IDB_STD_SMALL_COLOR            = 0;
         IDB_STD_LARGE_COLOR            = 1;
         IDB_VIEW_SMALL_COLOR           = 4;
         IDB_VIEW_LARGE_COLOR           = 5;
{$ifdef ie3plus}
         IDB_HIST_SMALL_COLOR           = 8;
         IDB_HIST_LARGE_COLOR           = 9;
{$ENDIF}
{$ifdef win32vista}
         IDB_HIST_NORMAL                = 12;
         IDB_HIST_HOT                   = 13;
         IDB_HIST_DISABLED              = 14;
         IDB_HIST_PRESSED               = 15;
{$endif}

// icon indexes for standard bitmap

         STD_CUT                        = 0;
         STD_COPY                       = 1;
         STD_PASTE                      = 2;
         STD_UNDO                       = 3;
         STD_REDOW                      = 4;
         STD_DELETE                     = 5;
         STD_FILENEW                    = 6;
         STD_FILEOPEN                   = 7;
         STD_FILESAVE                   = 8;
         STD_PRINTPRE                   = 9;
         STD_PROPERTIES                 = 10;
         STD_HELP                       = 11;
         STD_FIND                       = 12;
         STD_REPLACE                    = 13;
         STD_PRINT                      = 14;

// icon indexes for standard view bitmap

         VIEW_LARGEICONS                = 0;
         VIEW_SMALLICONS                = 1;
         VIEW_LIST                      = 2;
         VIEW_DETAILS                   = 3;
         VIEW_SORTNAME                  = 4;
         VIEW_SORTSIZE                  = 5;
         VIEW_SORTDATE                  = 6;
         VIEW_SORTTYPE                  = 7;
         VIEW_PARENTFOLDER              = 8;
         VIEW_NETCONNECT                = 9;
         VIEW_NETDISCONNECT             = 10;
         VIEW_NEWFOLDER                 = 11;
{$ifdef ie4plus}
         VIEW_VIEWMENU                  = 12;
{$ENDIF}

{$ifdef ie3plus}
         HIST_BACK                      = 0;
         HIST_FORWARD                   = 1;
         HIST_FAVORITES                 = 2;
         HIST_ADDTOFAVORITES            = 3;
         HIST_VIEWTREE                  = 4;
{$ENDIF}

{$ENDIF}

{$ifdef ie4plus}
         TB_ADDBUTTONSA                 = (WM_USER + 20);
         TB_INSERTBUTTONA               = (WM_USER + 21);
{$ELSE}
         TB_ADDBUTTONS                  = (WM_USER + 20);
         TB_INSERTBUTTON                = (WM_USER + 21);
{$ENDIF}

         TB_DELETEBUTTON                = (WM_USER + 22);
         TB_GETBUTTON                   = (WM_USER + 23);
         TB_BUTTONCOUNT                 = (WM_USER + 24);
         TB_COMMANDTOINDEX              = (WM_USER + 25);

{$IFDEF _WIN32}
TYPE

         tagTBSAVEPARAMSA     = Record
                                 hkr          : HKEY;
                                 pszSubKey    : LPCSTR;
                                 pszValueName : LPCSTR;
                                 END;
         TBSAVEPARAMSA        = tagTBSAVEPARAMSA;
         LPTBSAVEPARAMSA      = ^tagTBSAVEPARAMSA;
         TTBSAVEPARAMSA       = tagTBSAVEPARAMSA;
         PTBSAVEPARAMSA       = ^tagTBSAVEPARAMSA;


         tagTBSAVEPARAMSW     = Record
                                 hkr          : HKEY;
                                 pszSubKey    : LPCWSTR;
                                 pszValueName : LPCWSTR;
                                 END;
         TBSAVEPARAMSW        = tagTBSAVEPARAMSW;
         LPTBSAVEPARAMW       = ^tagTBSAVEPARAMSW;
         TTBSAVEPARAMSW       = tagTBSAVEPARAMSW;
         PTBSAVEPARAMSW       = ^tagTBSAVEPARAMSW;


{$IFDEF UNICODE}
         TBSAVEPARAMS        = TBSAVEPARAMSW;
         LPTBSAVEPARAMS      = LPTBSAVEPARAMSW;
{$ELSE}
         TBSAVEPARAMS        = TBSAVEPARAMSA;
         LPTBSAVEPARAMS      = LPTBSAVEPARAMSA;
{$ENDIF}

{$ENDIF}  // _WIN32


CONST
         TB_SAVERESTOREA                = (WM_USER + 26);
         TB_SAVERESTOREW                = (WM_USER + 76);
         TB_CUSTOMIZE                   = (WM_USER + 27);
         TB_ADDSTRINGA                  = (WM_USER + 28);
         TB_ADDSTRINGW                  = (WM_USER + 77);
         TB_GETITEMRECT                 = (WM_USER + 29);
         TB_BUTTONSTRUCTSIZE            = (WM_USER + 30);
         TB_SETBUTTONSIZE               = (WM_USER + 31);
         TB_SETBITMAPSIZE               = (WM_USER + 32);
         TB_AUTOSIZE                    = (WM_USER + 33);
         TB_GETTOOLTIPS                 = (WM_USER + 35);
         TB_SETTOOLTIPS                 = (WM_USER + 36);
         TB_SETPARENT                   = (WM_USER + 37);
         TB_SETROWS                     = (WM_USER + 39);
         TB_GETROWS                     = (WM_USER + 40);
         TB_SETCMDID                    = (WM_USER + 42);
         TB_CHANGEBITMAP                = (WM_USER + 43);
         TB_GETBITMAP                   = (WM_USER + 44);
         TB_GETBUTTONTEXTA              = (WM_USER + 45);
         TB_GETBUTTONTEXTW              = (WM_USER + 75);
         TB_REPLACEBITMAP               = (WM_USER + 46);
{$ifdef ie3plus}
         TB_SETINDENT                   = (WM_USER + 47);
         TB_SETIMAGELIST                = (WM_USER + 48);
         TB_GETIMAGELIST                = (WM_USER + 49);
         TB_LOADIMAGES                  = (WM_USER + 50);
         TB_GETRECT                     = (WM_USER + 51);     // wParam is the Cmd instead of index
         TB_SETHOTIMAGELIST             = (WM_USER + 52);
         TB_GETHOTIMAGELIST             = (WM_USER + 53);
         TB_SETDISABLEDIMAGELIST        = (WM_USER + 54);
         TB_GETDISABLEDIMAGELIST        = (WM_USER + 55);
         TB_SETSTYLE                    = (WM_USER + 56);
         TB_GETSTYLE                    = (WM_USER + 57);
         TB_GETBUTTONSIZE               = (WM_USER + 58);
         TB_SETBUTTONWIDTH              = (WM_USER + 59);
         TB_SETMAXTEXTROWS              = (WM_USER + 60);
         TB_GETTEXTROWS                 = (WM_USER + 61);
{$ENDIF}      // _WIN32_IE >= 0x0300

CONST
{$IFDEF UNICODE}

         TB_SAVERESTORE      = TB_SAVERESTOREW;
         TB_ADDSTRING        = TB_ADDSTRINGW;
{$ELSE}
         TB_GETBUTTONTEXT    = TB_GETBUTTONTEXTA;
         TB_SAVERESTORE      = TB_SAVERESTOREA;
         TB_ADDSTRING        = TB_ADDSTRINGA;
{$ENDIF}
{$ifdef ie4plus}

CONST
         TB_GETOBJECT                   = (WM_USER + 62);     // wParam == IID, lParam void **ppv
         TB_GETHOTITEM                  = (WM_USER + 71);
         TB_SETHOTITEM                  = (WM_USER + 72);     // wParam == iHotItem
         TB_SETANCHORHIGHLIGHT          = (WM_USER + 73);     // wParam == TRUE/FALSE
         TB_GETANCHORHIGHLIGHT          = (WM_USER + 74);
         TB_MAPACCELERATORA             = (WM_USER + 78);     // wParam == ch, lParam int * pidBtn

TYPE
         TBINSERTMARK         = Record
                                 iButton      : cint;
                                 dwFlags      : DWORD;
                                 END;
         LPTBINSERTMARK       = ^TBINSERTMARK;
         TTBINSERTMARK        = TBINSERTMARK;
         PTBINSERTMARK        = ^TBINSERTMARK;


CONST
         TBIMHT_AFTER                   = $00000001;          // TRUE = insert After iButton, otherwise before
         TBIMHT_BACKGROUND              = $00000002;          // TRUE iff missed buttons completely

         TB_GETINSERTMARK               = (WM_USER + 79);     // lParam == LPTBINSERTMARK
         TB_SETINSERTMARK               = (WM_USER + 80);     // lParam == LPTBINSERTMARK
         TB_INSERTMARKHITTEST           = (WM_USER + 81);     // wParam == LPPOINT lParam == LPTBINSERTMARK
         TB_MOVEBUTTON                  = (WM_USER + 82);
         TB_GETMAXSIZE                  = (WM_USER + 83);     // lParam == LPSIZE
         TB_SETEXTENDEDSTYLE            = (WM_USER + 84);     // For TBSTYLE_EX_*
         TB_GETEXTENDEDSTYLE            = (WM_USER + 85);     // For TBSTYLE_EX_*
         TB_GETPADDING                  = (WM_USER + 86);
         TB_SETPADDING                  = (WM_USER + 87);
         TB_SETINSERTMARKCOLOR          = (WM_USER + 88);
         TB_GETINSERTMARKCOLOR          = (WM_USER + 89);

         TB_SETCOLORSCHEME              = CCM_SETCOLORSCHEME; // lParam is color scheme
         TB_GETCOLORSCHEME              = CCM_GETCOLORSCHEME; // fills in COLORSCHEME pointed to by lParam

         TB_SETUNICODEFORMAT            = CCM_SETUNICODEFORMAT;
         TB_GETUNICODEFORMAT            = CCM_GETUNICODEFORMAT;

         TB_MAPACCELERATORW             = (WM_USER + 90);     // wParam == ch, lParam int * pidBtn

CONST
{$IFDEF UNICODE}
         TB_MAPACCELERATOR   = TB_MAPACCELERATORW;
{$ELSE}
         TB_MAPACCELERATOR   = TB_MAPACCELERATORA;
{$ENDIF}

{$ENDIF}  // _WIN32_IE >= 0x0400
TYPE
         TBREPLACEBITMAP      = Record
                                 hInstOld     : HINST;
                                 nIDOld       : UINT_PTR;
                                 hInstNew     : HINST;
                                 nIDNew       : UINT_PTR;
                                 nButtons     : cint;
                                 END;
         LPTBREPLACEBITMAP    = ^TBREPLACEBITMAP;
         TTBREPLACEBITMAP     = TBREPLACEBITMAP;
         PTBREPLACEBITMAP     = ^TBREPLACEBITMAP;


{$IFDEF _WIN32}

CONST
         TBBF_LARGE                     = $0001;
         TB_GETBITMAPFLAGS              = (WM_USER + 41);

{$ifdef ie4plus}
         TBIF_IMAGE                     = $00000001;
         TBIF_TEXT                      = $00000002;
         TBIF_STATE                     = $00000004;
         TBIF_STYLE                     = $00000008;
         TBIF_LPARAM                    = $00000010;
         TBIF_COMMAND                   = $00000020;
         TBIF_SIZE                      = $00000040;

{$ifdef ie5plus}
         TBIF_BYINDEX                   = $80000000;          // this specifies that the wparam in Get/SetButtonInfo is an index, not id
{$ENDIF}

TYPE

         TBBUTTONINFOA        = Record
                                 cbSize       : UINT;
                                 dwMask       : DWORD;
                                 idCommand    : cint;
                                 iImage       : cint;
                                 fsState      : BYTE;
                                 fsStyle      : BYTE;
                                 cx           : WORD;
                                 lParam       : DWORD_PTR;
                                 pszText      : LPSTR;
                                 cchText      : cint;
                                 END;
         LPTBBUTTONINFOA      = ^TBBUTTONINFOA;
         TTBBUTTONINFOA       = TBBUTTONINFOA;
         PTBBUTTONINFOA       = ^TBBUTTONINFOA;


         TBBUTTONINFOW        = Record
                                 cbSize       : UINT;
                                 dwMask       : DWORD;
                                 idCommand    : cint;
                                 iImage       : cint;
                                 fsState      : BYTE;
                                 fsStyle      : BYTE;
                                 cx           : WORD;
                                 lParam       : DWORD_PTR;
                                 pszText      : LPWSTR;
                                 cchText      : cint;
                                 END;
         LPTBBUTTONINFOW      = ^TBBUTTONINFOW;
         TTBBUTTONINFOW= TBBUTTONINFOW;
         PTBBUTTONINFOW= ^TBBUTTONINFOW;


{$IFDEF UNICODE}
         TBBUTTONINFO        = TBBUTTONINFOW;
         LPTBBUTTONINFO      = LPTBBUTTONINFOW;
{$ELSE}
         TBBUTTONINFO        = TBBUTTONINFOA;
         LPTBBUTTONINFO      = LPTBBUTTONINFOA;
{$ENDIF}
         TTBButtonInfo       = TBBUTTONINFO;
         PTBButtonInfo       = LPTBBUTTONINFO;


// BUTTONINFO APIs do NOT support the string pool.

CONST
         TB_GETBUTTONINFOW              = (WM_USER + 63);
         TB_SETBUTTONINFOW              = (WM_USER + 64);
         TB_GETBUTTONINFOA              = (WM_USER + 65);
         TB_SETBUTTONINFOA              = (WM_USER + 66);

{$IFDEF UNICODE}
CONST
         TB_GETBUTTONINFO    = TB_GETBUTTONINFOW;
         TB_SETBUTTONINFO    = TB_SETBUTTONINFOW;
{$ELSE}
CONST
         TB_GETBUTTONINFO    = TB_GETBUTTONINFOA;
         TB_SETBUTTONINFO    = TB_SETBUTTONINFOA;
{$ENDIF}

CONST
         TB_INSERTBUTTONW               = (WM_USER + 67);
         TB_ADDBUTTONSW                 = (WM_USER + 68);

         TB_HITTEST                     = (WM_USER + 69);

// New post Win95/NT4 for InsertButton and AddButton.  if iString member
// is a pointer to a string, it will be handled as a string like listview
// (although LPSTR_TEXTCALLBACK is not supported).

{$IFDEF UNICODE}
CONST
         TB_INSERTBUTTON     = TB_INSERTBUTTONW;
         TB_ADDBUTTONS       = TB_ADDBUTTONSW;
{$ELSE}
CONST
         TB_INSERTBUTTON     = TB_INSERTBUTTONA;
         TB_ADDBUTTONS       = TB_ADDBUTTONSA;
{$ENDIF}


CONST
         TB_SETDRAWTEXTFLAGS            = (WM_USER + 70);     // wParam == mask lParam == bit values

{$ENDIF}  // _WIN32_IE >= 0x0400

{$ifdef ie5plus}

         TB_GETSTRINGW                  = (WM_USER + 91);
         TB_GETSTRINGA                  = (WM_USER + 92);
{$IFDEF UNICODE}

         TB_GETSTRING        = TB_GETSTRINGW;
{$ELSE}
         TB_GETSTRING        = TB_GETSTRINGA;
{$ENDIF}


{$ENDIF}  // _WIN32_IE >= 0x0500

{$ifdef win32xp}

CONST
         TBMF_PAD                       = $00000001;
         TBMF_BARPAD                    = $00000002;
         TBMF_BUTTONSPACING             = $00000004;

TYPE

         TBMETRICS            = Record
                                 cbSize       : UINT;
                                 dwMask       : DWORD;
                                 cxPad        : cint;          // PAD
                                 cyPad        : cint;
                                 cxBarPad     : cint;          // BARPAD
                                 cyBarPad     : cint;
                                 cxButtonSpacing : cint;          // BUTTONSPACING
                                 cyButtonSpacing : cint;
                                 END;
         LPTBMETRICS          = ^TBMETRICS;
         TTBMETRICS           = TBMETRICS;
         PTBMETRICS           = ^TBMETRICS;

CONST
         TB_GETMETRICS                  = (WM_USER + 101);
         TB_SETMETRICS                  = (WM_USER + 102);
{$ENDIF}

{$ifdef win32vista}
         TB_SETPRESSEDIMAGELIST         = (WM_USER + 104);
         TB_GETPRESSEDIMAGELIST         = (WM_USER + 105);
{$endif}

{$ifdef win32xp}
         TB_SETWINDOWTHEME              = CCM_SETWINDOWTHEME;
{$ENDIF}

         TBN_GETBUTTONINFOA             = (TBN_FIRST-0);
         TBN_BEGINDRAG                  = (TBN_FIRST-1);
         TBN_ENDDRAG                    = (TBN_FIRST-2);
         TBN_BEGINADJUST                = (TBN_FIRST-3);
         TBN_ENDADJUST                  = (TBN_FIRST-4);
         TBN_RESET                      = (TBN_FIRST-5);
         TBN_QUERYINSERT                = (TBN_FIRST-6);
         TBN_QUERYDELETE                = (TBN_FIRST-7);
         TBN_TOOLBARCHANGE              = (TBN_FIRST-8);
         TBN_CUSTHELP                   = (TBN_FIRST-9);
{$ifdef ie3plus}
         TBN_DROPDOWN                   = (TBN_FIRST - 10);
{$ENDIF}
{$ifdef ie4plus}
         TBN_GETOBJECT                  = (TBN_FIRST - 12);

// Structure for TBN_HOTITEMCHANGE notification
//
TYPE

         tagNMTBHOTITEM       = Record
                                 hdr          : NMHDR;
                                 idOld        : cint;
                                 idNew        : cint;
                                 dwFlags      : DWORD;          // HICF_*
                                 END;
         NMTBHOTITEM          = tagNMTBHOTITEM;
         LPNMTBHOTITEM        = ^tagNMTBHOTITEM;
         TNMTBHOTITEM         = tagNMTBHOTITEM;
         PNMTBHOTITEM         = ^tagNMTBHOTITEM;


// Hot item change flags

CONST
         HICF_OTHER                     = $00000000;
         HICF_MOUSE                     = $00000001;          // Triggered by mouse
         HICF_ARROWKEYS                 = $00000002;          // Triggered by arrow keys
         HICF_ACCELERATOR               = $00000004;          // Triggered by accelerator
         HICF_DUPACCEL                  = $00000008;          // This accelerator is not unique
         HICF_ENTERING                  = $00000010;          // idOld is invalid
         HICF_LEAVING                   = $00000020;          // idNew is invalid
         HICF_RESELECT                  = $00000040;          // hot item reselected
         HICF_LMOUSE                    = $00000080;          // left mouse button selected
         HICF_TOGGLEDROPDOWN            = $00000100;          // Toggle button's dropdown state


         TBN_HOTITEMCHANGE              = (TBN_FIRST - 13);
         TBN_DRAGOUT                    = (TBN_FIRST - 14);   // this is sent when the user clicks down on a button then drags off the button
         TBN_DELETINGBUTTON             = (TBN_FIRST - 15);   // uses TBNOTIFY
         TBN_GETDISPINFOA               = (TBN_FIRST - 16);   // This is sent when the  toolbar needs  some display information
         TBN_GETDISPINFOW               = (TBN_FIRST - 17);   // This is sent when the  toolbar needs  some display information
         TBN_GETINFOTIPA                = (TBN_FIRST - 18);
         TBN_GETINFOTIPW                = (TBN_FIRST - 19);
         TBN_GETBUTTONINFOW             = (TBN_FIRST - 20);
{$ifdef ie5plus}
         TBN_RESTORE                    = (TBN_FIRST - 21);
         TBN_SAVE                       = (TBN_FIRST - 22);
         TBN_INITCUSTOMIZE              = (TBN_FIRST - 23);
         TBNRF_HIDEHELP                 = $00000001;
         TBNRF_ENDCUSTOMIZE             = $00000002;
{$ENDIF} // (_WIN32_IE >= 0x0500)



{$ifdef ie5plus}

TYPE

         tagNMTBSAVE          = Record
                                 hdr          : NMHDR;
                                 pData        : PDWORD;
                                 pCurrent     : PDWORD;
                                 cbData       : UINT;
                                 iItem        : cint;
                                 cButtons     : cint;
                                 tbButton     : TBBUTTON;
                                 END;
         NMTBSAVE             = tagNMTBSAVE;
         LPNMTBSAVE           = ^tagNMTBSAVE;
         TNMTBSAVE            = tagNMTBSAVE;
         PNMTBSAVE            = ^tagNMTBSAVE;


         tagNMTBRESTORE       = Record
                                 hdr          : NMHDR;
                                 pData        : PDWORD;
                                 pCurrent     : PDWORD;
                                 cbData       : UINT;
                                 iItem        : cint;
                                 cButtons     : cint;
                                 cbBytesPerRecord : cint;
                                 tbButton     : TBBUTTON;
                                 END;
         NMTBRESTORE          = tagNMTBRESTORE;
         LPNMTBRESTORE        = ^tagNMTBRESTORE;
         TNMTBRESTORE         = tagNMTBRESTORE;
         PNMTBRESTORE         = ^tagNMTBRESTORE;

{$ENDIF} // (_WIN32_IE >= 0x0500)

         tagNMTBGETINFOTIPA   = Record
                                 hdr          : NMHDR;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iItem        : cint;
                                 lParam       : LPARAM;
                                 END;
         NMTBGETINFOTIPA      = tagNMTBGETINFOTIPA;
         LPNMTBGETINFOTIPA    = ^tagNMTBGETINFOTIPA;
         TNMTBGETINFOTIPA     = tagNMTBGETINFOTIPA;
         PNMTBGETINFOTIPA     = ^tagNMTBGETINFOTIPA;


         tagNMTBGETINFOTIPW   = Record
                                 hdr          : NMHDR;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iItem        : cint;
                                 lParam       : LPARAM;
                                 END;
         NMTBGETINFOTIPW      = tagNMTBGETINFOTIPW;
         LPNMTBGETINFOTIPW    = ^tagNMTBGETINFOTIPW;
         TNMTBGETINFOTIPW     = tagNMTBGETINFOTIPW;
         PNMTBGETINFOTIPW     = ^tagNMTBGETINFOTIPW;


{$IFDEF UNICODE}
CONST
         TBN_GETINFOTIP      = TBN_GETINFOTIPW;
TYPE
         NMTBGETINFOTIP      = NMTBGETINFOTIPW;
         LPNMTBGETINFOTIP    = LPNMTBGETINFOTIPW;
{$ELSE}
CONST
         TBN_GETINFOTIP      = TBN_GETINFOTIPA;
TYPE
         NMTBGETINFOTIP      = NMTBGETINFOTIPA;
         LPNMTBGETINFOTIP    = LPNMTBGETINFOTIPA;
{$ENDIF}


CONST
         TBNF_IMAGE                     = $00000001;
         TBNF_TEXT                      = $00000002;
         TBNF_DI_SETITEM                = $10000000;

TYPE

         NMTBDISPINFOA        = Record
                                 hdr          : NMHDR;
                                 dwMask       : DWORD;          // [in] Specifies the values requested .[out] Client ask the data to be set for future use
                                 idCommand    : cint;          // [in] id of button we're requesting info for
                                 lParam       : DWORD_PTR;          // [in] lParam of button
                                 iImage       : cint;          // [out] image index
                                 pszText      : LPSTR;          // [out] new text for item
                                 cchText      : cint;          // [in] size of buffer pointed to by pszText
                                 END;
         LPNMTBDISPINFOA      = ^NMTBDISPINFOA;
         TNMTBDISPINFOA       = NMTBDISPINFOA;
         PNMTBDISPINFOA       = ^NMTBDISPINFOA;


         NMTBDISPINFOW        = Record
                                 hdr          : NMHDR;
                                 dwMask       : DWORD;          //[in] Specifies the values requested .[out] Client ask the data to be set for future use
                                 idCommand    : cint;          // [in] id of button we're requesting info for
                                 lParam       : DWORD_PTR;          // [in] lParam of button
                                 iImage       : cint;          // [out] image index
                                 pszText      : LPWSTR;          // [out] new text for item
                                 cchText      : cint;          // [in] size of buffer pointed to by pszText
                                 END;
         LPNMTBDISPINFOW      = ^NMTBDISPINFOW;
         TNMTBDISPINFOW       = NMTBDISPINFOW;
         PNMTBDISPINFOW       = ^NMTBDISPINFOW;

{$IFDEF UNICODE}
CONST
         TBN_GETDISPINFO     = TBN_GETDISPINFOW;
TYPE
         NMTBDISPINFO        = NMTBDISPINFOW;
         LPNMTBDISPINFO      = LPNMTBDISPINFOW;
{$ELSE}
CONST
         TBN_GETDISPINFO     = TBN_GETDISPINFOA;
TYPE
         NMTBDISPINFO        = NMTBDISPINFOA;
         LPNMTBDISPINFO      = LPNMTBDISPINFOA;
{$ENDIF}

// Return codes for TBN_DROPDOWN

CONST
         TBDDRET_DEFAULT                = 0;
         TBDDRET_NODEFAULT              = 1;
         TBDDRET_TREATPRESSED           = 2;                  // Treat as a standard press button

{$ENDIF}
{$IFDEF UNICODE}
         TBN_GETBUTTONINFO   = TBN_GETBUTTONINFOW;
{$ELSE}
         TBN_GETBUTTONINFO   = TBN_GETBUTTONINFOA;
{$ENDIF}
TYPE

{$ifdef ie3plus}
         tagNMTOOLBARA        = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 tbButton     : TBBUTTON;
                                 cchText      : cint;
                                 pszText      : LPSTR;
{$ifdef ie5plus}
                                 rcButton     : RECT;
{$ENDIF}
                                 END;
         NMTOOLBARA           = tagNMTOOLBARA;
         LPNMTOOLBARA         = ^tagNMTOOLBARA;
         LPNMTOOLBAR          = LPNMTOOLBARA;
         TNMTOOLBARA          = tagNMTOOLBARA;
         PNMTOOLBARA          = LPNMTOOLBARA;
{$ENDIF}
         LPTBNOTIFY          = LPNMTOOLBARA;
         TBNOTIFYA           = NMTOOLBARA;


{$ifdef ie3plus}
         tagNMTOOLBARW        = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 tbButton     : TBBUTTON;
                                 cchText      : cint;
                                 pszText      : LPWSTR;
{$ifdef ie5plus}
                                 rcButton     : RECT;
{$ENDIF}
                                 END;
         NMTOOLBARW           = tagNMTOOLBARW;
         LPNMTOOLBARW         = ^tagNMTOOLBARW;
         TNMTOOLBARW          = tagNMTOOLBARW;
         PNMTOOLBARW          = ^tagNMTOOLBARW;
{$ENDIF}


{$ifdef ie3plus}
         TBNOTIFYW           = NMTOOLBARW;
         LPTBNOTIFYA         = LPNMTOOLBARA;
{$ELSE}
         tagNMTOOLBARA       = tagTBNOTIFYA;
         NMTOOLBARA          = TBNOTIFYA;
         LPNMTOOLBARA        = LPTBNOTIFYA;
         tagNMTOOLBARW       = tagTBNOTIFYW;
         NMTOOLBARW          = TBNOTIFYW;
         LPNMTOOLBARW        = LPTBNOTIFYW;
{$ENDIF}


{$IFDEF UNICODE}
         NMTOOLBAR           = NMTOOLBARW;
         LPNMTOOLBAR         = LPNMTOOLBARW;
{$ELSE}
         NMTOOLBAR           = NMTOOLBARA;
         TNMTOOLBAR          = NMTOOLBARA;
         PNMTOOLBAR          = LPNMTOOLBARA;

{$ENDIF}

         TBNOTIFY                       = NMTOOLBAR;
{$ENDIF}

{$ENDIF}      // NOTOOLBAR

         LPTBNOTIFYW         = LPNMTOOLBARW;

{$ifdef ie3plus}
//====== REBAR CONTROL ========================================================

{$IFNDEF NOREBAR}

{$IFDEF _WIN32}

CONST
         REBARCLASSNAMEW                = {L}'ReBarWindow32';
         REBARCLASSNAMEA                = 'ReBarWindow32';

{$IFDEF  UNICODE}
         REBARCLASSNAME      = REBARCLASSNAMEW;
{$ELSE}
         REBARCLASSNAME      = REBARCLASSNAMEA;
{$ENDIF}

{$ELSE}

CONST
         REBARCLASSNAME                 = 'ReBarWindow';
{$ENDIF}

CONST
         RBIM_IMAGELIST                 = $00000001;

// begin_r_commctrl

{$ifdef ie4plus}
         RBS_TOOLTIPS                   = $0100;
         RBS_VARHEIGHT                  = $0200;
         RBS_BANDBORDERS                = $0400;
         RBS_FIXEDORDER                 = $0800;
         RBS_REGISTERDROP               = $1000;
         RBS_AUTOSIZE                   = $2000;
         RBS_VERTICALGRIPPER            = $4000;              // this always has the vertical gripper (default for horizontal mode)
         RBS_DBLCLKTOGGLE               = $8000;
{$ELSE}
         RBS_TOOLTIPS                   = $00000100;
         RBS_VARHEIGHT                  = $00000200;
         RBS_BANDBORDERS                = $00000400;
         RBS_FIXEDORDER                 = $00000800;
{$ENDIF}      // _WIN32_IE >= 0x0400


// end_r_commctrl

TYPE

         tagREBARINFO         = Record
                                 cbSize       : UINT;
                                 fMask        : UINT;
{$IFNDEF NOIMAGEAPIS}
                                 himl         : HIMAGELIST;
{$ELSE}
                                 himl         : HANDLE;
{$ENDIF}
                                 END;
         REBARINFO            = tagREBARINFO;
         LPREBARINFO          = ^tagREBARINFO;
         TREBARINFO           = tagREBARINFO;
         PREBARINFO           = ^tagREBARINFO;


CONST
         RBBS_BREAK                     = $00000001;          // break to new line
         RBBS_FIXEDSIZE                 = $00000002;          // band can't be sized
         RBBS_CHILDEDGE                 = $00000004;          // edge around top & bottom of child window
         RBBS_HIDDEN                    = $00000008;          // don't show
         RBBS_NOVERT                    = $00000010;          // don't show when vertical
         RBBS_FIXEDBMP                  = $00000020;          // bitmap doesn't move during band resize
{$ifdef ie4plus               }
         RBBS_VARIABLEHEIGHT            = $00000040;          // allow autosizing of this child vertically
         RBBS_GRIPPERALWAYS             = $00000080;          // always show the gripper
         RBBS_NOGRIPPER                 = $00000100;          // never show the gripper
{$ifdef ie5plus}
         RBBS_USECHEVRON                = $00000200;          // display drop-down button for this band if it's sized smaller than ideal width
{$ifdef IE501plus}
         RBBS_HIDETITLE                 = $00000400;          // keep band title hidden
         RBBS_TOPALIGN                  = $00000800;          // keep band title hidden
{$ENDIF} // 0x0501                        //
{$ENDIF} // 0x0500                        //
{$ENDIF} // 0x0400                        //

         RBBIM_STYLE                    = $00000001;
         RBBIM_COLORS                   = $00000002;
         RBBIM_TEXT                     = $00000004;
         RBBIM_IMAGE                    = $00000008;
         RBBIM_CHILD                    = $00000010;
         RBBIM_CHILDSIZE                = $00000020;
         RBBIM_SIZE                     = $00000040;
         RBBIM_BACKGROUND               = $00000080;
         RBBIM_ID                       = $00000100;
{$ifdef ie4plus}
         RBBIM_IDEALSIZE                = $00000200;
         RBBIM_LPARAM                   = $00000400;
         RBBIM_HEADERSIZE               = $00000800;          // control the size of the header
{$ENDIF}
{$ifdef win32vista}
	 RBBIM_CHEVRONLOCATION 		= $00001000;
	 RBBIM_CHEVRONSTATE    		= $00002000;
{$endif}

TYPE

         tagREBARBANDINFOA    = Record
                                 cbSize       : UINT;
                                 fMask        : UINT;
                                 fStyle       : UINT;
                                 clrFore      : COLORREF;
                                 clrBack      : COLORREF;
                                 lpText       : LPSTR;
                                 cch          : UINT;
                                 iImage       : cint;
                                 hwndChild    : HWND;
                                 cxMinChild   : UINT;
                                 cyMinChild   : UINT;
                                 cx           : UINT;
                                 hbmBack      : HBITMAP;
                                 wID          : UINT;
{$ifdef ie4plus}
                                 cyChild      : UINT;
                                 cyMaxChild   : UINT;
                                 cyIntegral   : UINT;
                                 cxIdeal      : UINT;
                                 lParam       : LPARAM;
                                 cxHeader     : UINT;
{$ENDIF}
{$ifdef win32vista}
			         rcChevronLocation : RECT;  // the rect is in client co-ord wrt hwndChild
    			         uChevronState     : cUINT;      // STATE_SYSTEM_*
{$endif}
                                 END;
         REBARBANDINFOA       = tagREBARBANDINFOA;
         LPREBARBANDINFOA     = ^tagREBARBANDINFOA;
         TREBARBANDINFOA      = tagREBARBANDINFOA;
         PREBARBANDINFOA      = ^tagREBARBANDINFOA;

         LPCREBARBANDINFOA    = ^REBARBANDINFOA;

// #define REBARBANDINFOA_V3_SIZE CCSIZEOF_STRUCT(REBARBANDINFOA, wID)
// #define REBARBANDINFOW_V3_SIZE CCSIZEOF_STRUCT(REBARBANDINFOW, wID)

         tagREBARBANDINFOW    = Record
                                 cbSize       : UINT;
                                 fMask        : UINT;
                                 fStyle       : UINT;
                                 clrFore      : COLORREF;
                                 clrBack      : COLORREF;
                                 lpText       : LPWSTR;
                                 cch          : UINT;
                                 iImage       : cint;
                                 hwndChild    : HWND;
                                 cxMinChild   : UINT;
                                 cyMinChild   : UINT;
                                 cx           : UINT;
                                 hbmBack      : HBITMAP;
                                 wID          : UINT;
{$ifdef ie4plus}
                                 cyChild      : UINT;
                                 cyMaxChild   : UINT;
                                 cyIntegral   : UINT;
                                 cxIdeal      : UINT;
                                 lParam       : LPARAM;
                                 cxHeader     : UINT;
{$ENDIF}
{$ifdef win32vista}
			         rcChevronLocation : RECT;  // the rect is in client co-ord wrt hwndChild
    			         uChevronState     : cUINT;      // STATE_SYSTEM_*
{$endif}

                                 END;
         REBARBANDINFOW       = tagREBARBANDINFOW;
         LPREBARBANDINFOW     = ^tagREBARBANDINFOW;
         TREBARBANDINFOW      = tagREBARBANDINFOW;
         PREBARBANDINFOW      = ^tagREBARBANDINFOW;

         LPCREBARBANDINFOW    = {const} ^REBARBANDINFOW;

{$IFDEF UNICODE}
         REBARBANDINFO       = REBARBANDINFOW;
         LPREBARBANDINFO     = LPREBARBANDINFOW;
         LPCREBARBANDINFO    = LPCREBARBANDINFOW;

//         REBARBANDINFO_V3_SIZE          = REBARBANDINFOW_V3_SIZE;
{$ELSE}

         REBARBANDINFO       = REBARBANDINFOA;
         LPREBARBANDINFO     = LPREBARBANDINFOA;
         LPCREBARBANDINFO    = LPCREBARBANDINFOA;

         TRebarBandInfo      = REBARBANDINFO;
         PRebarBandInfo      = ^LPREBARBANDINFO;

//         REBARBANDINFO_V3_SIZE          = REBARBANDINFOA_V3_SIZE;
{$ENDIF}

CONST
         RB_INSERTBANDA                 = (WM_USER +  1);
         RB_DELETEBAND                  = (WM_USER +  2);
         RB_GETBARINFO                  = (WM_USER +  3);
         RB_SETBARINFO                  = (WM_USER +  4);
{$ifdef IE4plus}
         RB_GETBANDINFO                 = (WM_USER +  5);
         RB_GETBANDINFO_PRE_IE4         = (WM_USER +  5);
{$ENDIF}
         RB_SETBANDINFOA                = (WM_USER +  6);
         RB_SETPARENT                   = (WM_USER +  7);
{$ifdef ie4plus}
         RB_HITTEST                     = (WM_USER +  8);
         RB_GETRECT                     = (WM_USER +  9);
{$ENDIF}
         RB_INSERTBANDW                 = (WM_USER +  10);
         RB_SETBANDINFOW                = (WM_USER +  11);
         RB_GETBANDCOUNT                = (WM_USER +  12);
         RB_GETROWCOUNT                 = (WM_USER +  13);
         RB_GETROWHEIGHT                = (WM_USER +  14);
{$ifdef ie4plus}
         RB_IDTOINDEX                   = (WM_USER +  16);    // wParam == id
         RB_GETTOOLTIPS                 = (WM_USER +  17);
         RB_SETTOOLTIPS                 = (WM_USER +  18);
         RB_SETBKCOLOR                  = (WM_USER +  19);    // sets the default BK color
         RB_GETBKCOLOR                  = (WM_USER +  20);    // defaults to CLR_NONE
         RB_SETTEXTCOLOR                = (WM_USER +  21);
         RB_GETTEXTCOLOR                = (WM_USER +  22);    // defaults to 0x00000000

{$ifdef Win32XP}
         RBSTR_CHANGERECT               = $0001;              // flags for RB_SIZETORECT
{$ENDIF}

         RB_SIZETORECT                  = (WM_USER +  23);    // resize the rebar/break bands and such to this rect (lparam)
{$ENDIF}      // _WIN32_IE >= 0x0400

         RB_SETCOLORSCHEME              = CCM_SETCOLORSCHEME; // lParam is color scheme
         RB_GETCOLORSCHEME              = CCM_GETCOLORSCHEME; // fills in COLORSCHEME pointed to by lParam

{$IFDEF UNICODE}
         RB_INSERTBAND       = RB_INSERTBANDW;
         RB_SETBANDINFO      = RB_SETBANDINFOW;
{$ELSE}
         RB_INSERTBAND       = RB_INSERTBANDA;
         RB_SETBANDINFO      = RB_SETBANDINFOA;
{$ENDIF}

{$ifdef ie4plus}
// for manual drag control
// lparam == cursor pos
        // -1 means do it yourself.
        // -2 means use what you had saved before

CONST
         RB_BEGINDRAG                   = (WM_USER + 24);
         RB_ENDDRAG                     = (WM_USER + 25);
         RB_DRAGMOVE                    = (WM_USER + 26);
         RB_GETBARHEIGHT                = (WM_USER + 27);
         RB_GETBANDINFOW                = (WM_USER + 28);
         RB_GETBANDINFOA                = (WM_USER + 29);

//TYPE
{$IFDEF UNICODE}
//         RB_GETBANDINFO      = RB_GETBANDINFOW;
{$ELSE}
//         RB_GETBANDINFO      = RB_GETBANDINFOA;
{$ENDIF}

CONST
         RB_MINIMIZEBAND                = (WM_USER + 30);
         RB_MAXIMIZEBAND                = (WM_USER + 31);

         RB_GETDROPTARGET               = (CCM_GETDROPTARGET);

         RB_GETBANDBORDERS              = (WM_USER + 34);     // returns in lparam = lprc the amount of edges added to band wparam

         RB_SHOWBAND                    = (WM_USER + 35);     // show/hide band
         RB_SETPALETTE                  = (WM_USER + 37);
         RB_GETPALETTE                  = (WM_USER + 38);
         RB_MOVEBAND                    = (WM_USER + 39);

         RB_SETUNICODEFORMAT            = CCM_SETUNICODEFORMAT;
         RB_GETUNICODEFORMAT            = CCM_GETUNICODEFORMAT;

{$ENDIF}      // _WIN32_IE >= 0x0400

{$ifdef win32xp}
         RB_GETBANDMARGINS              = (WM_USER + 40);
         RB_SETWINDOWTHEME              = CCM_SETWINDOWTHEME;
{$ENDIF}
{$ifdef win32vista}
	 RB_SETEXTENDEDSTYLE 		= (WM_USER + 41);
	 RB_GETEXTENDEDSTYLE 		= (WM_USER + 42);
{$endif}

{$ifdef ie5plus}
         RB_PUSHCHEVRON                 = (WM_USER + 43);
{$ENDIF}      // _WIN32_IE >= 0x0500

{$ifdef win32vista}
	 RB_SETBANDWIDTH     		= (WM_USER + 44);   // set width for docked band
{$endif}

         RBN_HEIGHTCHANGE               = (RBN_FIRST - 0);

{$ifdef ie4plus}
         RBN_GETOBJECT                  = (RBN_FIRST - 1);
         RBN_LAYOUTCHANGED              = (RBN_FIRST - 2);
         RBN_AUTOSIZE                   = (RBN_FIRST - 3);
         RBN_BEGINDRAG                  = (RBN_FIRST - 4);
         RBN_ENDDRAG                    = (RBN_FIRST - 5);
         RBN_DELETINGBAND               = (RBN_FIRST - 6);    // Uses NMREBAR
         RBN_DELETEDBAND                = (RBN_FIRST - 7);    // Uses NMREBAR
         RBN_CHILDSIZE                  = (RBN_FIRST - 8);

{$ifdef ie5plus}
         RBN_CHEVRONPUSHED              = (RBN_FIRST - 10);
{$ENDIF}      // _WIN32_IE >= 0x0500
{$ifdef win32vista}
 	 RBN_SPLITTERDRAG    		= (RBN_FIRST - 11);
{$endif}

{$ifdef ie5plus}
         RBN_MINMAX                     = (RBN_FIRST - 21);
{$ENDIF}

{$ifdef Win32XP}
         RBN_AUTOBREAK                  = (RBN_FIRST - 22);
{$ENDIF}

TYPE

         tagNMREBARCHILDSIZE  = Record
                                 hdr          : NMHDR;
                                 uBand        : UINT;
                                 wID          : UINT;
                                 rcChild      : RECT;
                                 rcBand       : RECT;
                                 END;
         NMREBARCHILDSIZE     = tagNMREBARCHILDSIZE;
         LPNMREBARCHILDSIZE   = ^tagNMREBARCHILDSIZE;
         TNMREBARCHILDSIZE    = tagNMREBARCHILDSIZE;
         PNMREBARCHILDSIZE    = ^tagNMREBARCHILDSIZE;


         tagNMREBAR           = Record
                                 hdr          : NMHDR;
                                 dwMask       : DWORD;          // RBNM_*
                                 uBand        : UINT;
                                 fStyle       : UINT;
                                 wID          : UINT;
                                 lParam       : LPARAM;
                                 END;
         NMREBAR              = tagNMREBAR;
         LPNMREBAR            = ^tagNMREBAR;
         TNMREBAR             = tagNMREBAR;
         PNMREBAR             = ^tagNMREBAR;

// Mask flags for NMREBAR

CONST
         RBNM_ID                        = $00000001;
         RBNM_STYLE                     = $00000002;
         RBNM_LPARAM                    = $00000004;

TYPE

         tagNMRBAUTOSIZE      = Record
                                 hdr          : NMHDR;
                                 fChanged     : BOOL;
                                 rcTarget     : RECT;
                                 rcActual     : RECT;
                                 END;
         NMRBAUTOSIZE         = tagNMRBAUTOSIZE;
         LPNMRBAUTOSIZE       = ^tagNMRBAUTOSIZE;
         TNMRBAUTOSIZE        = tagNMRBAUTOSIZE;
         PNMRBAUTOSIZE        = ^tagNMRBAUTOSIZE;


{$ifdef ie5plus}
         tagNMREBARCHEVRON    = Record
                                 hdr          : NMHDR;
                                 uBand        : UINT;
                                 wID          : UINT;
                                 lParam       : LPARAM;
                                 rc           : RECT;
                                 lParamNM     : LPARAM;
                                 END;
         NMREBARCHEVRON       = tagNMREBARCHEVRON;
         LPNMREBARCHEVRON     = ^tagNMREBARCHEVRON;
         TNMREBARCHEVRON      = tagNMREBARCHEVRON;
         PNMREBARCHEVRON      = ^tagNMREBARCHEVRON;

{$ENDIF}
{$ifdef win32vista}
	tagNMREBARSPLITTER    = record
				 hdr:      NMHDR;
				 rcSizing: RECT;
				 end;
	NMREBARSPLITTER       = tagNMREBARSPLITTER;
	LPNMREBARSPLITTER     = ^tagNMREBARSPLITTER;
	TNMREBARSPLITTER      = tagNMREBARSPLITTER;
	PNMREBARSPLITTER      = LPNMREBARSPLITTER;
{$endif}

{$ifdef Win32XP}
CONST
         RBAB_AUTOSIZE                  = $0001;              // These are not flags and are all mutually exclusive
         RBAB_ADDBAND                   = $0002;

TYPE

         tagNMREBARAUTOBREAK  = Record
                                 hdr          : NMHDR;
                                 uBand        : UINT;
                                 wID          : UINT;
                                 lParam       : LPARAM;
                                 uMsg         : UINT;
                                 fStyleCurrent : UINT;
                                 fAutoBreak   : BOOL;
                                 END;
         NMREBARAUTOBREAK     = tagNMREBARAUTOBREAK;
         LPNMREBARAUTOBREAK   = ^tagNMREBARAUTOBREAK;
         TNMREBARAUTOBREAK    = tagNMREBARAUTOBREAK;
         PNMREBARAUTOBREAK    = ^tagNMREBARAUTOBREAK;

{$ENDIF}

CONST
         RBHT_NOWHERE                   = $0001;
         RBHT_CAPTION                   = $0002;
         RBHT_CLIENT                    = $0003;
         RBHT_GRABBER                   = $0004;
{$ifdef ie5plus}
         RBHT_CHEVRON                   = $0008;
{$ENDIF}
{$ifdef win32vista}
	 RBHT_SPLITTER   		= $0010;
{$endif}

TYPE
         _RB_HITTESTINFO      = Record
                                 pt           : POINT;
                                 flags        : UINT;
                                 iBand        : cint;
                                 END;
         RBHITTESTINFO        = _RB_HITTESTINFO;
         LPRBHITTESTINFO      = ^_RB_HITTESTINFO;
         TRB_HITTESTINFO      = _RB_HITTESTINFO;
         PRB_HITTESTINFO      = ^_RB_HITTESTINFO;
         PRBHitTestInfo       = PRB_HITTESTINFO;
         TRBHitTestInfo       = TRB_HITTESTINFO;


{$ENDIF}      // _WIN32_IE >= 0x0400
{$ENDIF}      // NOREBAR
{$ENDIF}      // _WIN32_IE >= 0x0300

//====== TOOLTIPS CONTROL =====================================================

{$IFNDEF NOTOOLTIPS}

{$IFDEF _WIN32}

CONST
         TOOLTIPS_CLASSW                = {L}'tooltips_class32';
         TOOLTIPS_CLASSA                = 'tooltips_class32';

{$IFDEF UNICODE}
         TOOLTIPS_CLASS                 = TOOLTIPS_CLASSW;
{$ELSE}
         TOOLTIPS_CLASS                 = TOOLTIPS_CLASSA;
{$ENDIF}

{$ELSE}

CONST
         TOOLTIPS_CLASS                 = 'tooltips_class';
{$ENDIF}



TYPE

         tagTOOLINFOA         = Record
                                 cbSize       : UINT;
                                 uFlags       : UINT;
                                 hwnd         : HWND;
                                 uId          : UINT_PTR;
                                 rect         : RECT;
                                 hinst        : HINST;
                                 lpszText     : LPSTR;
{$ifdef ie3plus}
                                 lParam       : LPARAM;
{$ENDIF}
{$ifdef Win32XP}
                                 lpReserved   : Pointer;
{$ENDIF}
                                 END;
         TTTOOLINFOA          = tagTOOLINFOA;
         PTOOLINFOA           = {near} ^tagTOOLINFOA;
         LPTTTOOLINFOA        = ^tagTOOLINFOA;
         TTOOLINFOA           = tagTOOLINFOA;
         LPTOOLINFOA           = ^tagTOOLINFOA;


         tagTOOLINFOW         = Record
                                 cbSize       : UINT;
                                 uFlags       : UINT;
                                 hwnd         : HWND;
                                 uId          : UINT_PTR;
                                 rect         : RECT;
                                 hinst        : HINST;
                                 lpszText     : LPWSTR;
{$ifdef ie3plus}
                                 lParam       : LPARAM;
{$ENDIF}
{$ifdef Win32XP}
                                 lpReserved   : Pointer;
{$ENDIF}
                                 END;
         TTTOOLINFOW          = tagTOOLINFOW;
         PTOOLINFOW           = {near} ^tagTOOLINFOW;
         LPTTTOOLINFOW        = ^tagTOOLINFOW;
         TTOOLINFOW           = tagTOOLINFOW;
         LPTOOLINFOW           = ^tagTOOLINFOW;


{$IFDEF UNICODE}
         TTTOOLINFO          = TTTOOLINFOW;
         PTOOLINFO           = PTOOLINFOW;
         LPTTTOOLINFO        = LPTTTOOLINFOW;
//       TTTOOLINFO_V1_SIZE             = TTTOOLINFOW_V1_SIZE;
{$ELSE}
         PTOOLINFO           = PTOOLINFOA;
         TTTOOLINFO          = TTTOOLINFOA;
         LPTTTOOLINFO        = LPTTTOOLINFOA;
//       TTTOOLINFO_V1_SIZE             = TTTOOLINFOA_V1_SIZE;
{$ENDIF}


TYPE

{$ifdef ie3plus}
//         LPTOOLINFOA         = LPTTTOOLINFOA;
//         LPTOOLINFOW         = LPTTTOOLINFOW;
         TOOLINFOA           = TTTOOLINFOA;
         TOOLINFOW           = TTTOOLINFOW;
{$ELSE}
//         TTTOOLINFOA         = TOOLINFOA;
//         LPTTTOOLINFOA       = LPTOOLINFOA;
//         TTTOOLINFOW         = TOOLINFOW;
//         LPTTTOOLINFOW       = LPTOOLINFOW;
{$ENDIF}



         LPTOOLINFO          = LPTTTOOLINFO;

         TOOLINFO                       = TTTOOLINFO;

// #define TTTOOLINFOA_V1_SIZE CCSIZEOF_STRUCT(TTTOOLINFOA, lpszText)
// #define TTTOOLINFOW_V1_SIZE CCSIZEOF_STRUCT(TTTOOLINFOW, lpszText)
// #define TTTOOLINFOA_V2_SIZE CCSIZEOF_STRUCT(TTTOOLINFOA, lParam)
// #define TTTOOLINFOW_V2_SIZE CCSIZEOF_STRUCT(TTTOOLINFOW, lParam)
// #define TTTOOLINFOA_V3_SIZE CCSIZEOF_STRUCT(TTTOOLINFOA, lpReserved)
// #define TTTOOLINFOW_V3_SIZE CCSIZEOF_STRUCT(TTTOOLINFOW, lpReserved)


CONST
// begin_r_commctrl

         TTS_ALWAYSTIP                  = $01;
         TTS_NOPREFIX                   = $02;
{$ifdef ie5plus}
         TTS_NOANIMATE                  = $10;
         TTS_NOFADE                     = $20;
         TTS_BALLOON                    = $40;
         TTS_CLOSE                      = $80;
{$ENDIF}
{$ifdef win32vista}
	 TTS_USEVISUALSTYLE      	= $100;  // Use themed hyperlinks
{$endif}

// end_r_commctrl

         TTF_IDISHWND                   = $0001;

// Use this to center around trackpoint in trackmode
// -OR- to center around tool in normal mode.
// Use TTF_ABSOLUTE to place the tip exactly at the track coords when
// in tracking mode.  TTF_ABSOLUTE can be used in conjunction with TTF_CENTERTIP
// to center the tip absolutely about the track point.

         TTF_CENTERTIP                  = $0002;
         TTF_RTLREADING                 = $0004;
         TTF_SUBCLASS                   = $0010;
{$ifdef ie3plus}
         TTF_TRACK                      = $0020;
         TTF_ABSOLUTE                   = $0080;
         TTF_TRANSPARENT                = $0100;
{$ifdef ie501plus}
         TTF_PARSELINKS                 = $1000;
{$ENDIF} // _WIN32_IE >= 0x0501
         TTF_DI_SETITEM                 = $8000;              // valid only on the TTN_NEEDTEXT callback
{$ENDIF}      // _WIN32_IE >= 0x0300


         TTDT_AUTOMATIC                 = 0;
         TTDT_RESHOW                    = 1;
         TTDT_AUTOPOP                   = 2;
         TTDT_INITIAL                   = 3;

// ToolTip Icons (Set with TTM_SETTITLE)
         TTI_NONE                       = 0;
         TTI_INFO                       = 1;
         TTI_WARNING                    = 2;
         TTI_ERROR                      = 3;
{$ifdef win32vista}
	 TTI_INFO_LARGE          	= 4;
	 TTI_WARNING_LARGE       	= 5;
	 TTI_ERROR_LARGE         	= 6;
{$endif}

// Tool Tip Messages
         TTM_ACTIVATE                   = (WM_USER + 1);
         TTM_SETDELAYTIME               = (WM_USER + 3);
         TTM_ADDTOOLA                   = (WM_USER + 4);
         TTM_ADDTOOLW                   = (WM_USER + 50);
         TTM_DELTOOLA                   = (WM_USER + 5);
         TTM_DELTOOLW                   = (WM_USER + 51);
         TTM_NEWTOOLRECTA               = (WM_USER + 6);
         TTM_NEWTOOLRECTW               = (WM_USER + 52);
         TTM_RELAYEVENT                 = (WM_USER + 7);

         TTM_GETTOOLINFOA               = (WM_USER + 8);
         TTM_GETTOOLINFOW               = (WM_USER + 53);

         TTM_SETTOOLINFOA               = (WM_USER + 9);
         TTM_SETTOOLINFOW               = (WM_USER + 54);

         TTM_HITTESTA                   = (WM_USER + 10);
         TTM_HITTESTW                   = (WM_USER + 55);
         TTM_GETTEXTA                   = (WM_USER + 11);
         TTM_GETTEXTW                   = (WM_USER + 56);
         TTM_UPDATETIPTEXTA             = (WM_USER + 12);
         TTM_UPDATETIPTEXTW             = (WM_USER + 57);
         TTM_GETTOOLCOUNT               = (WM_USER + 13);
         TTM_ENUMTOOLSA                 = (WM_USER + 14);
         TTM_ENUMTOOLSW                 = (WM_USER + 58);
         TTM_GETCURRENTTOOLA            = (WM_USER + 15);
         TTM_GETCURRENTTOOLW            = (WM_USER + 59);
         TTM_WINDOWFROMPOINT            = (WM_USER + 16);
{$ifdef ie3plus}
         TTM_TRACKACTIVATE              = (WM_USER + 17);     // wParam = TRUE/FALSE start end  lparam = LPTOOLINFO
         TTM_TRACKPOSITION              = (WM_USER + 18);     // lParam = dwPos
         TTM_SETTIPBKCOLOR              = (WM_USER + 19);
         TTM_SETTIPTEXTCOLOR            = (WM_USER + 20);
         TTM_GETDELAYTIME               = (WM_USER + 21);
         TTM_GETTIPBKCOLOR              = (WM_USER + 22);
         TTM_GETTIPTEXTCOLOR            = (WM_USER + 23);
         TTM_SETMAXTIPWIDTH             = (WM_USER + 24);
         TTM_GETMAXTIPWIDTH             = (WM_USER + 25);
         TTM_SETMARGIN                  = (WM_USER + 26);     // lParam = lprc
         TTM_GETMARGIN                  = (WM_USER + 27);     // lParam = lprc
         TTM_POP                        = (WM_USER + 28);
{$ENDIF}
{$ifdef ie4plus}
         TTM_UPDATE                     = (WM_USER + 29);
{$ENDIF}
{$ifdef ie5plus}
         TTM_GETBUBBLESIZE              = (WM_USER + 30);
         TTM_ADJUSTRECT                 = (WM_USER + 31);
         TTM_SETTITLEA                  = (WM_USER + 32);     // wParam = TTI_*, lParam = char* szTitle
         TTM_SETTITLEW                  = (WM_USER + 33);     // wParam = TTI_*, lParam = wchar* szTitle
{$ENDIF}

{$ifdef Win32XP}
         TTM_POPUP                      = (WM_USER + 34);
         TTM_GETTITLE                   = (WM_USER + 35);     // wParam = 0, lParam = TTGETTITLE*

TYPE

         _TTGETTITLE          = Record
                                 dwSize       : DWORD;
                                 uTitleBitmap : UINT;
                                 cch          : UINT;
                                 pszTitle     : PWCHAR;
                                 END;
         TTGETTITLE           = _TTGETTITLE;
         PTTGETTITLE          = ^_TTGETTITLE;
         TTTGETTITLE          = _TTGETTITLE;
         PTTTGETTITLE          = ^TTTGETTITLE;

{$ENDIF}

const
{$IFDEF UNICODE}
         TTM_ADDTOOL         = TTM_ADDTOOLW;
         TTM_DELTOOL         = TTM_DELTOOLW;
         TTM_NEWTOOLRECT     = TTM_NEWTOOLRECTW;
         TTM_GETTOOLINFO     = TTM_GETTOOLINFOW;
         TTM_SETTOOLINFO     = TTM_SETTOOLINFOW;
         TTM_HITTEST         = TTM_HITTESTW;
         TTM_GETTEXT         = TTM_GETTEXTW;
         TTM_UPDATETIPTEXT   = TTM_UPDATETIPTEXTW;
         TTM_ENUMTOOLS       = TTM_ENUMTOOLSW;
         TTM_GETCURRENTTOOL  = TTM_GETCURRENTTOOLW;
{$ifdef ie5plus}
         TTM_SETTITLE        = TTM_SETTITLEW;
{$ENDIF}
{$ELSE}
         TTM_ADDTOOL         = TTM_ADDTOOLA;
         TTM_DELTOOL         = TTM_DELTOOLA;
         TTM_NEWTOOLRECT     = TTM_NEWTOOLRECTA;
         TTM_GETTOOLINFO     = TTM_GETTOOLINFOA;
         TTM_SETTOOLINFO     = TTM_SETTOOLINFOA;
         TTM_HITTEST         = TTM_HITTESTA;
         TTM_GETTEXT         = TTM_GETTEXTA;
         TTM_UPDATETIPTEXT   = TTM_UPDATETIPTEXTA;
         TTM_ENUMTOOLS       = TTM_ENUMTOOLSA;
         TTM_GETCURRENTTOOL  = TTM_GETCURRENTTOOLA;
{$ifdef ie5plus}
         TTM_SETTITLE        = TTM_SETTITLEA;
{$ENDIF}
{$ENDIF}

{$ifdef win32xp}
CONST
         TTM_SETWINDOWTHEME             = CCM_SETWINDOWTHEME;
{$ENDIF}


TYPE


         _TT_HITTESTINFOA     = Record
                                 hwnd         : HWND;
                                 pt           : POINT;
                                 ti           : TTTOOLINFOA;
                                 END;
         TTHITTESTINFOA       = _TT_HITTESTINFOA;
         LPTTHITTESTINFOA     = ^_TT_HITTESTINFOA;
         TTT_HITTESTINFOA     = _TT_HITTESTINFOA;
         PTT_HITTESTINFOA     = ^_TT_HITTESTINFOA;



         _TT_HITTESTINFOW     = Record
                                 hwnd         : HWND;
                                 pt           : POINT;
                                 ti           : TTTOOLINFOW;
                                 END;
         TTHITTESTINFOW       = _TT_HITTESTINFOW;
         LPTTHITTESTINFOW     = ^_TT_HITTESTINFOW;
         TTT_HITTESTINFOW     = _TT_HITTESTINFOW;
         PTT_HITTESTINFOW     = ^_TT_HITTESTINFOW;

{$IFDEF UNICODE}
         TTHITTESTINFO       = TTHITTESTINFOW;
         LPTTHITTESTINFO     = LPTTHITTESTINFOW;
{$ELSE}
         TTHITTESTINFO       = TTHITTESTINFOA;
         LPTTHITTESTINFO     = LPTTHITTESTINFOA;
{$ENDIF}
         LPHITTESTINFO       = LPTTHITTESTINFO;
{$ifdef ie3plus}
         LPHITTESTINFOW      = LPTTHITTESTINFOW;
         LPHITTESTINFOA      = LPTTHITTESTINFOA;
{$ELSE}
//         LPTTHITTESTINFOA    = LPHITTESTINFOA;
//         LPTTHITTESTINFOW    = LPHITTESTINFOW;
{$ENDIF}

CONST
         TTN_GETDISPINFOA               = (TTN_FIRST - 0);
         TTN_GETDISPINFOW               = (TTN_FIRST - 10);
         TTN_SHOW                       = (TTN_FIRST - 1);
         TTN_POP                        = (TTN_FIRST - 2);
         TTN_LINKCLICK                  = (TTN_FIRST - 3);

CONST
{$IFDEF UNICODE}
         TTN_GETDISPINFO     = TTN_GETDISPINFOW;
{$ELSE}
         TTN_GETDISPINFO     = TTN_GETDISPINFOA;
{$ENDIF}

CONST
         TTN_NEEDTEXT                   = TTN_GETDISPINFO;


         TTN_NEEDTEXTA       = TTN_GETDISPINFOA;
         TTN_NEEDTEXTW       = TTN_GETDISPINFOW;





TYPE


// #define NMTTDISPINFOA_V1_SIZE CCSIZEOF_STRUCT(NMTTDISPINFOA, uFlags)
// #define NMTTDISPINFOW_V1_SIZE CCSIZEOF_STRUCT(NMTTDISPINFOW, uFlags)

         tagNMTTDISPINFOA     = Record
                                 hdr          : NMHDR;
                                 lpszText     : LPSTR;
                                 szText       : Array[0..79] of char;
                                 hinst        : HINST;
                                 uFlags       : UINT;
{$ifdef ie3plus}
                                 lParam       : LPARAM;
{$ENDIF}
                                 END;
         NMTTDISPINFOA        = tagNMTTDISPINFOA;
         LPNMTTDISPINFOA      = ^tagNMTTDISPINFOA;
         TNMTTDISPINFOA       = tagNMTTDISPINFOA;
         PNMTTDISPINFOA       = ^tagNMTTDISPINFOA;


         tagNMTTDISPINFOW     = Record
                                 hdr          : NMHDR;
                                 lpszText     : LPWSTR;
                                 szText       : Array[0..79] of WCHAR;
                                 hinst        : HINST;
                                 uFlags       : UINT;
{$ifdef ie3plus}
                                 lParam       : LPARAM;
{$ENDIF}
                                 END;
         NMTTDISPINFOW        = tagNMTTDISPINFOW;
         LPNMTTDISPINFOW      = ^tagNMTTDISPINFOW;
         TNMTTDISPINFOW       = tagNMTTDISPINFOW;
         PNMTTDISPINFOW       = ^tagNMTTDISPINFOW;

{$ifdef ie3plus}
         TOOLTIPTEXTW        = NMTTDISPINFOW;
         TOOLTIPTEXTA        = NMTTDISPINFOA;
         LPTOOLTIPTEXTA      = LPNMTTDISPINFOA;
         LPTOOLTIPTEXTW      = LPNMTTDISPINFOW;
         TTOOLTIPTEXTW       = NMTTDISPINFOW;
         PTOOLTIPTEXTW       = LPNMTTDISPINFOW;
{$ELSE}
//         tagNMTTDISPINFOA    = tagTOOLTIPTEXTA;
         NMTTDISPINFOA       = TOOLTIPTEXTA;
//         LPNMTTDISPINFOA     = LPTOOLTIPTEXTA;
         tagNMTTDISPINFOW    = tagTOOLTIPTEXTW;
         NMTTDISPINFOW       = TOOLTIPTEXTW;
         LPNMTTDISPINFOW     = LPTOOLTIPTEXTW;
{$ENDIF}

TYPE

{$IFDEF UNICODE}
         NMTTDISPINFO        = NMTTDISPINFOW;
         LPNMTTDISPINFO      = LPNMTTDISPINFOW;
//         NMTTDISPINFO_V1_SIZE           = NMTTDISPINFOW_V1_SIZE;
{$ELSE}
         NMTTDISPINFO        = NMTTDISPINFOA;
         LPNMTTDISPINFO      = LPNMTTDISPINFOA;
//         NMTTDISPINFO_V1_SIZE           = NMTTDISPINFOA_V1_SIZE;
{$ENDIF}

         TOOLTIPTEXT         = NMTTDISPINFO;
         LPTOOLTIPTEXT       = LPNMTTDISPINFO;

{$ENDIF}      // NOTOOLTIPS


//====== STATUS BAR CONTROL ===================================================

{$IFNDEF NOSTATUSBAR}

// begin_r_commctrl
CONST
         SBARS_SIZEGRIP                 = $0100;
{$ifdef ie5plus}
         SBARS_TOOLTIPS                 = $0800;
{$ENDIF}

{$ifdef ie4plus}
// this is a status bar flag, preference to SBARS_TOOLTIPS
         SBT_TOOLTIPS                   = $0800;
{$ENDIF}

// end_r_commctrl

procedure DrawStatusTextA(hDC:HDC;lprc:LPRECT;pszText:LPCSTR;uFlags:UINT); stdcall; external commctrldll name 'DrawStatusTextA';
Procedure DrawStatusTextW(hDC:HDC;lprc:LPRECT;pszText:LPCWSTR;uFlags:UINT); stdcall; external commctrldll name 'DrawStatusTextW';

function CreateStatusWindowA(style:LONG;lpszText:LPCSTR;hwndParent:HWND;wID:UINT):HWND; stdcall; external commctrldll name 'CreateStatusWindowA';
function CreateStatusWindowW(style:LONG;lpszText:LPCWSTR;hwndParent:HWND;wID:UINT):HWND; stdcall; external commctrldll name 'CreateStatusWindowW';

function CreateStatusWindow(style:LONG;lpszText:LPCSTR;hwndParent:HWND;wID:UINT):HWND; stdcall; external commctrldll name 'CreateStatusWindowA';
procedure DrawStatusText(hDC:HDC;lprc:LPRECT;pszText:LPCSTR;uFlags:UINT); stdcall; external commctrldll name 'DrawStatusTextA';
function CreateStatusWindow(style:LONG;lpszText:LPCWSTR;hwndParent:HWND;wID:UINT):HWND; stdcall; external commctrldll name 'CreateStatusWindowW';
Procedure DrawStatusText(hDC:HDC;lprc:LPRECT;pszText:LPCWSTR;uFlags:UINT); stdcall; external commctrldll name 'DrawStatusTextW';

CONST
{$IFDEF _WIN32}
         STATUSCLASSNAMEW               = {L}'msctls_statusbar32';
         STATUSCLASSNAMEA               = 'msctls_statusbar32';

{$IFDEF UNICODE}
         STATUSCLASSNAME                = STATUSCLASSNAMEW;
{$ELSE}
         STATUSCLASSNAME                = STATUSCLASSNAMEA;
{$ENDIF}
{$ELSE}
         STATUSCLASSNAME                = 'msctls_statusbar';
{$ENDIF}

         SB_SETTEXTA                    = (WM_USER+1);
         SB_SETTEXTW                    = (WM_USER+11);
         SB_GETTEXTA                    = (WM_USER+2);
         SB_GETTEXTW                    = (WM_USER+13);
         SB_GETTEXTLENGTHA              = (WM_USER+3);
         SB_GETTEXTLENGTHW              = (WM_USER+12);


         SB_SETPARTS                    = (WM_USER+4);
         SB_GETPARTS                    = (WM_USER+6);
         SB_GETBORDERS                  = (WM_USER+7);
         SB_SETMINHEIGHT                = (WM_USER+8);
         SB_SIMPLE                      = (WM_USER+9);
         SB_GETRECT                     = (WM_USER+10);
{$ifdef ie3plus}
         SB_ISSIMPLE                    = (WM_USER+14);
{$ENDIF}
{$ifdef ie4plus}
         SB_SETICON                     = (WM_USER+15);
         SB_SETTIPTEXTA                 = (WM_USER+16);
         SB_SETTIPTEXTW                 = (WM_USER+17);
         SB_GETTIPTEXTA                 = (WM_USER+18);
         SB_GETTIPTEXTW                 = (WM_USER+19);
         SB_GETICON                     = (WM_USER+20);
         SB_SETUNICODEFORMAT            = CCM_SETUNICODEFORMAT;
         SB_GETUNICODEFORMAT            = CCM_GETUNICODEFORMAT;
{$ENDIF}
{$IFDEF UNICODE}
         SB_GETTEXT                     = SB_GETTEXTW;
         SB_SETTEXT                     = SB_SETTEXTW;
         SB_GETTEXTLENGTH               = SB_GETTEXTLENGTHW;
{$ifdef ie4plus}
         SB_SETTIPTEXT                  = SB_SETTIPTEXTW;
         SB_GETTIPTEXT                  = SB_GETTIPTEXTW;
{$ENDIF}
{$ELSE}
         SB_GETTEXT                     = SB_GETTEXTA;
         SB_SETTEXT                     = SB_SETTEXTA;
         SB_GETTEXTLENGTH               = SB_GETTEXTLENGTHA;
{$ifdef ie4plus}
         SB_SETTIPTEXT                  = SB_SETTIPTEXTA;
         SB_GETTIPTEXT                  = SB_GETTIPTEXTA;
{$ENDIF}
{$ENDIF}

         SBT_OWNERDRAW                  = $1000;
         SBT_NOBORDERS                  = $0100;
         SBT_POPOUT                     = $0200;
         SBT_RTLREADING                 = $0400;
{$ifdef ie5plus}
         SBT_NOTABPARSING               = $0800;
{$ENDIF}

         SB_SETBKCOLOR                  = CCM_SETBKCOLOR;     // lParam = bkColor

/// status bar notifications
{$ifdef ie4plus}
         SBN_SIMPLEMODECHANGE           = (SBN_FIRST - 0);
{$ENDIF}

{$ifdef ie5plus}
// refers to the data saved for simple mode
         SB_SIMPLEID                    = $00ff;
{$ENDIF}

{$ENDIF}      // NOSTATUSBAR

//====== MENU HELP ============================================================

{$IFNDEF NOMENUHELP}

Procedure MenuHelp(uMsg:UINT;wParam:WPARAM;lParam:LPARAM;hMainMenu:HMENU;hInst:HINST;hwndStatus:HWND;lpwIDs:PUINT); stdcall; external commctrldll name 'MenuHelp';
function ShowHideMenuCtl(hWnd:HWND;uFlags:UINT_PTR;lpInfo:LPINT):BOOL; stdcall; external commctrldll name 'ShowHideMenuCtl';
Procedure GetEffectiveClientRect(hWnd:HWND;lprc:LPRECT;lpInfo:LPINT); stdcall; external commctrldll name 'GetEffectiveClientRect';

CONST
         MINSYSCOMMAND                  = SC_SIZE;
{$ENDIF}


//====== TRACKBAR CONTROL =====================================================

{$IFNDEF NOTRACKBAR}

CONST

{$IFDEF _WIN32}

         TRACKBAR_CLASSA                = 'msctls_trackbar32';
         TRACKBAR_CLASSW                = {L}'msctls_trackbar32';

{$IFDEF UNICODE}
         TRACKBAR_CLASS                 = TRACKBAR_CLASSW;
{$ELSE}
         TRACKBAR_CLASS                 = TRACKBAR_CLASSA;
{$ENDIF}
{$ELSE}
         TRACKBAR_CLASS                 = 'msctls_trackbar';
{$ENDIF}

// begin_r_commctrl

         TBS_AUTOTICKS                  = $0001;
         TBS_VERT                       = $0002;
         TBS_HORZ                       = $0000;
         TBS_TOP                        = $0004;
         TBS_BOTTOM                     = $0000;
         TBS_LEFT                       = $0004;
         TBS_RIGHT                      = $0000;
         TBS_BOTH                       = $0008;
         TBS_NOTICKS                    = $0010;
         TBS_ENABLESELRANGE             = $0020;
         TBS_FIXEDLENGTH                = $0040;
         TBS_NOTHUMB                    = $0080;
{$ifdef ie3plus}
         TBS_TOOLTIPS                   = $0100;
{$ENDIF}
{$ifdef ie5plus}
         TBS_REVERSED                   = $0200;              // Accessibility hint: the smaller number (usually the min value) means "high" and the larger number (usually the max value) means "low"
{$ENDIF}

{$ifdef ie501plus}
         TBS_DOWNISLEFT                 = $0400;              // Down=Left and Up=Right (default is Down=Right and Up=Left)
{$ENDIF}
{$ifdef win32vista}
	 TBS_NOTIFYBEFOREMOVE    	= $0800;  // Trackbar should notify parent before repositioning the slider due to user action (enables snapping)
{$endif}
{$ifdef NTDDI_VISTA}
	 TBS_TRANSPARENTBKGND    	= $1000;  // Background is painted by the parent via WM_PRINTCLIENT
{$endif}

// end_r_commctrl

         TBM_GETPOS                     = (WM_USER);
         TBM_GETRANGEMIN                = (WM_USER+1);
         TBM_GETRANGEMAX                = (WM_USER+2);
         TBM_GETTIC                     = (WM_USER+3);
         TBM_SETTIC                     = (WM_USER+4);
         TBM_SETPOS                     = (WM_USER+5);
         TBM_SETRANGE                   = (WM_USER+6);
         TBM_SETRANGEMIN                = (WM_USER+7);
         TBM_SETRANGEMAX                = (WM_USER+8);
         TBM_CLEARTICS                  = (WM_USER+9);
         TBM_SETSEL                     = (WM_USER+10);
         TBM_SETSELSTART                = (WM_USER+11);
         TBM_SETSELEND                  = (WM_USER+12);
         TBM_GETPTICS                   = (WM_USER+14);
         TBM_GETTICPOS                  = (WM_USER+15);
         TBM_GETNUMTICS                 = (WM_USER+16);
         TBM_GETSELSTART                = (WM_USER+17);
         TBM_GETSELEND                  = (WM_USER+18);
         TBM_CLEARSEL                   = (WM_USER+19);
         TBM_SETTICFREQ                 = (WM_USER+20);
         TBM_SETPAGESIZE                = (WM_USER+21);
         TBM_GETPAGESIZE                = (WM_USER+22);
         TBM_SETLINESIZE                = (WM_USER+23);
         TBM_GETLINESIZE                = (WM_USER+24);
         TBM_GETTHUMBRECT               = (WM_USER+25);
         TBM_GETCHANNELRECT             = (WM_USER+26);
         TBM_SETTHUMBLENGTH             = (WM_USER+27);
         TBM_GETTHUMBLENGTH             = (WM_USER+28);
{$ifdef ie3plus}
         TBM_SETTOOLTIPS                = (WM_USER+29);
         TBM_GETTOOLTIPS                = (WM_USER+30);
         TBM_SETTIPSIDE                 = (WM_USER+31);
// TrackBar Tip Side flags
         TBTS_TOP                       = 0;
         TBTS_LEFT                      = 1;
         TBTS_BOTTOM                    = 2;
         TBTS_RIGHT                     = 3;

         TBM_SETBUDDY                   = (WM_USER+32);       // wparam = BOOL fLeft; (or right)
         TBM_GETBUDDY                   = (WM_USER+33);       // wparam = BOOL fLeft; (or right)
{$ENDIF}
{$ifdef ie4plus}
         TBM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;
         TBM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;
{$ENDIF}


         TB_LINEUP                      = 0;
         TB_LINEDOWN                    = 1;
         TB_PAGEUP                      = 2;
         TB_PAGEDOWN                    = 3;
         TB_THUMBPOSITION               = 4;
         TB_THUMBTRACK                  = 5;
         TB_TOP                         = 6;
         TB_BOTTOM                      = 7;
         TB_ENDTRACK                    = 8;


{$ifdef ie3plus}
// custom draw item specs
         TBCD_TICS                      = $0001;
         TBCD_THUMB                     = $0002;
         TBCD_CHANNEL                   = $0003;
{$ENDIF}
{$ifdef win32vista}
	 TRBN_THUMBPOSCHANGING       	= (TRBN_FIRST-1);
{$endif}
{$ENDIF} // trackbar

//====== DRAG LIST CONTROL ====================================================

{$IFNDEF NODRAGLIST}
TYPE

         tagDRAGLISTINFO      = Record
                                 uNotification : UINT;
                                 hWnd         : HWND;
                                 ptCursor     : POINT;
                                 END;
         DRAGLISTINFO         = tagDRAGLISTINFO;
         LPDRAGLISTINFO       = ^tagDRAGLISTINFO;
         TDRAGLISTINFO        = tagDRAGLISTINFO;
         PDRAGLISTINFO        = ^tagDRAGLISTINFO;



CONST
         DL_BEGINDRAG                   = (WM_USER+133);
         DL_DRAGGING                    = (WM_USER+134);
         DL_DROPPED                     = (WM_USER+135);
         DL_CANCELDRAG                  = (WM_USER+136);

         DL_CURSORSET                   = 0;
         DL_STOPCURSOR                  = 1;
         DL_COPYCURSOR                  = 2;
         DL_MOVECURSOR                  = 3;

         DRAGLISTMSGSTRING              = 'commctrl_DragListMsg'; // TEXT("commctrl_DragListMsg");

function MakeDragList(hLB:HWND):BOOL; stdcall; external commctrldll name 'MakeDragList';
Procedure DrawInsert(handParent:HWND;hLB:HWND;nItem:cint); stdcall; external commctrldll name 'DrawInsert';
function LBItemFromPt(hLB:HWND;pt:POINT;bAutoScroll:BOOL):cint; stdcall; external commctrldll name 'LBItemFromPt';

{$ENDIF}

//====== UPDOWN CONTROL =======================================================

{$IFNDEF NOUPDOWN}
CONST
{$IFDEF _WIN32}

         UPDOWN_CLASSA                  = 'msctls_updown32';
         UPDOWN_CLASSW                  = {L}'msctls_updown32';

{$IFDEF UNICODE}
         UPDOWN_CLASS                   = UPDOWN_CLASSW;
{$ELSE}
         UPDOWN_CLASS                   = UPDOWN_CLASSA;
{$ENDIF}
{$ELSE}
         UPDOWN_CLASS                   = 'msctls_updown';
{$ENDIF}

TYPE

         _UDACCEL             = Record
                                 nSec         : UINT;
                                 nInc         : UINT;
                                 END;
         UDACCEL              = _UDACCEL;
         LPUDACCEL            = ^_UDACCEL;
         TUDACCEL             = _UDACCEL;
         PUDACCEL             = ^_UDACCEL;

CONST
         UD_MAXVAL                      = $7fff;
         UD_MINVAL                      = (-UD_MAXVAL);

// begin_r_commctrl

         UDS_WRAP                       = $0001;
         UDS_SETBUDDYINT                = $0002;
         UDS_ALIGNRIGHT                 = $0004;
         UDS_ALIGNLEFT                  = $0008;
         UDS_AUTOBUDDY                  = $0010;
         UDS_ARROWKEYS                  = $0020;
         UDS_HORZ                       = $0040;
         UDS_NOTHOUSANDS                = $0080;
{$ifdef ie3plus}
         UDS_HOTTRACK                   = $0100;
{$ENDIF}

// end_r_commctrl

         UDM_SETRANGE                   = (WM_USER+101);
         UDM_GETRANGE                   = (WM_USER+102);
         UDM_SETPOS                     = (WM_USER+103);
         UDM_GETPOS                     = (WM_USER+104);
         UDM_SETBUDDY                   = (WM_USER+105);
         UDM_GETBUDDY                   = (WM_USER+106);
         UDM_SETACCEL                   = (WM_USER+107);
         UDM_GETACCEL                   = (WM_USER+108);
         UDM_SETBASE                    = (WM_USER+109);
         UDM_GETBASE                    = (WM_USER+110);
{$ifdef ie4plus}
         UDM_SETRANGE32                 = (WM_USER+111);
         UDM_GETRANGE32                 = (WM_USER+112);      // wParam & lParam are LPINT
         UDM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;
         UDM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;
{$ENDIF}
{$ifdef ie5plus}
         UDM_SETPOS32                   = (WM_USER+113);
         UDM_GETPOS32                   = (WM_USER+114);
{$ENDIF}

function CreateUpDownControl(dwStyle:DWORD;x:cint;y:cint;cx:cint;cy:cint;hParent:HWND;nID:cint;hInst:HINST;hBuddy:HWND;nUpper:cint;nLower:cint;nPos:cint):HWND; stdcall; external commctrldll name 'CreateUpDownControl';

TYPE
         _NM_UPDOWN           = Record
                                 hdr          : NMHDR;
                                 iPos         : cint;
                                 iDelta       : cint;
                                 END;
         NMUPDOWN             = _NM_UPDOWN;
         LPNMUPDOWN           = ^_NM_UPDOWN;
         TNM_UPDOWN           = _NM_UPDOWN;
         PNM_UPDOWN           = ^_NM_UPDOWN;

{$ifdef ie3plus}
         NM_UPDOWN                      = NMUPDOWN;
         LPNM_UPDOWN                    = LPNMUPDOWN;
{$ELSE}
         NMUPDOWN                       = NM_UPDOWN;
         LPNMUPDOWN                     = LPNM_UPDOWN;
{$ENDIF}

CONST
         UDN_DELTAPOS                   = (UDN_FIRST - 1);

{$ENDIF}  // NOUPDOWN


//====== PROGRESS CONTROL =====================================================

{$IFNDEF NOPROGRESS}

CONST

{$IFDEF _WIN32}

         PROGRESS_CLASSA                = 'msctls_progress32';
         PROGRESS_CLASSW                = {L}'msctls_progress32';

{$IFDEF UNICODE}
         PROGRESS_CLASS      = PROGRESS_CLASSW;
{$ELSE}
         PROGRESS_CLASS      = PROGRESS_CLASSA;
{$ENDIF}

{$ELSE}
         PROGRESS_CLASS                 = 'msctls_progress';
{$ENDIF}

// begin_r_commctrl

{$ifdef ie3plus}
         PBS_SMOOTH                     = $01;
         PBS_VERTICAL                   = $04;
{$ENDIF}

// end_r_commctrl

         PBM_SETRANGE                   = (WM_USER+1);
         PBM_SETPOS                     = (WM_USER+2);
         PBM_DELTAPOS                   = (WM_USER+3);
         PBM_SETSTEP                    = (WM_USER+4);
         PBM_STEPIT                     = (WM_USER+5);
{$ifdef ie3plus}
         PBM_SETRANGE32                 = (WM_USER+6);        // lParam = high, wParam = low

TYPE
         PBRANGE              = Record
                                 iLow         : cint;
                                 iHigh        : cint;
                                 END;
         PPBRANGE             = ^PBRANGE;
         TPBRANGE             = PBRANGE;
//         PPBRANGE             = ^PBRANGE;


CONST
         PBM_GETRANGE                   = (WM_USER+7);        // wParam = return (TRUE ? low : high). lParam = PPBRANGE or NULL
         PBM_GETPOS                     = (WM_USER+8);
{$ifdef ie4plus}
         PBM_SETBARCOLOR                = (WM_USER+9);        // lParam = bar color
{$ENDIF}      // _WIN32_IE >= 0x0400
         PBM_SETBKCOLOR                 = CCM_SETBKCOLOR;     // lParam = bkColor
{$ENDIF}      // _WIN32_IE >= 0x0300

{$ifdef win32xp}  //_WIN32_WINNT >= 0x0501
         PBS_MARQUEE                    = $08;
         PBM_SETMARQUEE                 = (WM_USER+10);
{$endif} //_WIN32_WINNT >= 0x0501

{$ifdef win32vista}
	 PBS_SMOOTHREVERSE       = $10;
{$endif}

{$ifdef win32vista}

 PBM_GETSTEP             = (WM_USER+13);
 PBM_GETBKCOLOR          = (WM_USER+14);
 PBM_GETBARCOLOR         = (WM_USER+15);
 PBM_SETSTATE            = (WM_USER+16); // wParam = PBST_[State] (NORMAL, ERROR, PAUSED)
 PBM_GETSTATE            = (WM_USER+17);

 PBST_NORMAL             = $0001;
 PBST_ERROR              = $0002;
 PBST_PAUSED             = $0003;
{$endif}

// end_r_commctrl


{$ENDIF}  // NOPROGRESS


//====== HOTKEY CONTROL =======================================================

{$IFNDEF NOHOTKEY}
         HOTKEYF_SHIFT                  = $01;
         HOTKEYF_CONTROL                = $02;
         HOTKEYF_ALT                    = $04;
{$IFDEF _MAC}
         HOTKEYF_EXT                    = $80;
{$ELSE}
         HOTKEYF_EXT                    = $08;
{$ENDIF}
         HKCOMB_NONE                    = $0001;
         HKCOMB_S                       = $0002;
         HKCOMB_C                       = $0004;
         HKCOMB_A                       = $0008;
         HKCOMB_SC                      = $0010;
         HKCOMB_SA                      = $0020;
         HKCOMB_CA                      = $0040;
         HKCOMB_SCA                     = $0080;

         HKM_SETHOTKEY                  = (WM_USER+1);
         HKM_GETHOTKEY                  = (WM_USER+2);
         HKM_SETRULES                   = (WM_USER+3);

{$IFDEF _WIN32}
         HOTKEY_CLASSA                  = 'msctls_hotkey32';
         HOTKEY_CLASSW                  = widestring('msctls_hotkey32');
{$IFDEF UNICODE}
         HOTKEY_CLASS                   = HOTKEY_CLASSW;
{$ELSE}
         HOTKEY_CLASS                   = HOTKEY_CLASSA;
{$ENDIF}
{$ELSE}
         HOTKEY_CLASS                   = 'msctls_hotkey';
{$ENDIF}

{$ENDIF}  // NOHOTKEY
         HOTKEYCLASS                    = HOTKEY_CLASSA;

// begin_r_commctrl

//====== COMMON CONTROL STYLES ================================================

         CCS_TOP                        = DWORD($00000001){L};
         CCS_NOMOVEY                    = DWORD($00000002){L};
         CCS_BOTTOM                     = DWORD($00000003){L};
         CCS_NORESIZE                   = DWORD($00000004){L};
         CCS_NOPARENTALIGN              = DWORD($00000008){L};
         CCS_ADJUSTABLE                 = DWORD($00000020){L};
         CCS_NODIVIDER                  = DWORD($00000040){L};
{$ifdef ie3plus}
         CCS_VERT                       = DWORD($00000080){L};
         CCS_LEFT                       = (CCS_VERT  OR  CCS_TOP);
         CCS_RIGHT                      = (CCS_VERT  OR  CCS_BOTTOM);
         CCS_NOMOVEX                    = (CCS_VERT  OR  CCS_NOMOVEY);
{$ENDIF}

// end_r_commctrl

//====== LISTVIEW CONTROL =====================================================

{$IFNDEF NOLISTVIEW}

{$IFDEF _WIN32}
         WC_LISTVIEWA                   = 'SysListView32';
         WC_LISTVIEWW                   = {L}'SysListView32';
{$IFDEF UNICODE}
         WC_LISTVIEW                    = WC_LISTVIEWW;
{$ELSE}
         WC_LISTVIEW                    = WC_LISTVIEWA;
{$ENDIF}
{$ELSE}
         WC_LISTVIEW                    = 'SysListView';
{$ENDIF}

// begin_r_commctrl

         LVS_ICON                       = $0000;
         LVS_REPORT                     = $0001;
         LVS_SMALLICON                  = $0002;
         LVS_LIST                       = $0003;
         LVS_TYPEMASK                   = $0003;
         LVS_SINGLESEL                  = $0004;
         LVS_SHOWSELALWAYS              = $0008;
         LVS_SORTASCENDING              = $0010;
         LVS_SORTDESCENDING             = $0020;
         LVS_SHAREIMAGELISTS            = $0040;
         LVS_NOLABELWRAP                = $0080;
         LVS_AUTOARRANGE                = $0100;
         LVS_EDITLABELS                 = $0200;
{$ifdef ie3plus}
         LVS_OWNERDATA                  = $1000;
{$ENDIF}
         LVS_NOSCROLL                   = $2000;

         LVS_TYPESTYLEMASK              = $fc00;

         LVS_ALIGNTOP                   = $0000;
         LVS_ALIGNLEFT                  = $0800;
         LVS_ALIGNMASK                  = $0c00;

         LVS_OWNERDRAWFIXED             = $0400;
         LVS_NOCOLUMNHEADER             = $4000;
         LVS_NOSORTHEADER               = $8000;

// end_r_commctrl

{$ifdef ie4plus}
         LVM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;

// Macro 38
Function ListView_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;

CONST
         LVM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;

// Macro 39
Function ListView_GetUnicodeFormat( hwnd : hwnd):BOOL;

{$ENDIF}
CONST
         LVM_GETBKCOLOR                 = (LVM_FIRST + 0);

// Macro 40
Function ListView_GetBkColor( hwnd : hwnd):COLORREF;

CONST
         LVM_SETBKCOLOR                 = (LVM_FIRST + 1);

// Macro 41
Function ListView_SetBkColor( hwnd : hwnd; clrBk : COLORREF):BOOL;

CONST
         LVM_GETIMAGELIST               = (LVM_FIRST + 2);

// Macro 42
Function ListView_GetImageList( hwnd : hwnd; iImageList : CINT):HIMAGELIST;

CONST
         LVSIL_NORMAL                   = 0;
         LVSIL_SMALL                    = 1;
         LVSIL_STATE                    = 2;
	 LVSIL_GROUPHEADER       	= 3;
         LVM_SETIMAGELIST               = (LVM_FIRST + 3);

// Macro 43
Function ListView_SetImageList( hwnd : hwnd; himl : HIMAGELIST; iImageList : WPARAM):HIMAGELIST;

CONST
         LVM_GETITEMCOUNT               = (LVM_FIRST + 4);

// Macro 44
Function ListView_GetItemCount( hwnd : hwnd):cint;

CONST
         LVIF_TEXT                      = $0001;
         LVIF_IMAGE                     = $0002;
         LVIF_PARAM                     = $0004;
         LVIF_STATE                     = $0008;
{$ifdef ie3plus}
         LVIF_INDENT                    = $0010;
         LVIF_NORECOMPUTE               = $0800;
{$ENDIF}
{$ifdef win32xp}
         LVIF_GROUPID                   = $0100;
         LVIF_COLUMNS                   = $0200;
{$ENDIF}
{$ifdef win32vista}
	 LVIF_COLFMT                    = $00010000; // The piColFmt member is valid in addition to puColumns
{$endif}

         LVIS_FOCUSED                   = $0001;
         LVIS_SELECTED                  = $0002;
         LVIS_CUT                       = $0004;
         LVIS_DROPHILITED               = $0008;
         LVIS_GLOW                      = $0010;
         LVIS_ACTIVATING                = $0020;

         LVIS_OVERLAYMASK               = $0F00;
         LVIS_STATEIMAGEMASK            = $F000;

// Macro 45
// #define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)


CONST
         I_INDENTCALLBACK               = (-1);
{$ENDIF}


{$ifdef win32xp}

CONST
         I_GROUPIDCALLBACK              = (-1);
         I_GROUPIDNONE                  = (-2);
{$ENDIF}


// #define LVITEMA_V1_SIZE CCSIZEOF_STRUCT(LVITEMA, lParam)
// #define LVITEMW_V1_SIZE CCSIZEOF_STRUCT(LVITEMW, lParam)

TYPE
         tagLVITEMA           = Record
                                 mask         : UINT;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 lParam       : LPARAM;
{$ifdef ie3plus}
                                 iIndent      : cint;
{$ENDIF}
{$ifdef win32xp}
                                 iGroupId     : cint;
                                 cColumns     : UINT;          // tile view columns
                                 puColumns    : PUINT;
{$ENDIF}
{$ifdef win32vista}
				 piColFmt : pcint;
				 iGroup   : cint; // readonly. only valid for owner data.
{$endif}
                                 END;
         LVITEMA              = tagLVITEMA;
         LPLVITEMA            = ^tagLVITEMA;
         TLVITEMA             = tagLVITEMA;
         PLVITEMA             = ^tagLVITEMA;


         tagLVITEMW           = Record
                                 mask         : UINT;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 lParam       : LPARAM;
{$ifdef ie3plus}
                                 iIndent      : cint;
{$ENDIF}
{$ifdef win32xp}
                                 iGroupId     : cint;
                                 cColumns     : UINT;          // tile view columns
                                 puColumns    : PUINT;
{$ENDIF}
{$ifdef win32vista}
				 piColFmt : pcint;
				 iGroup   : cint; // readonly. only valid for owner data.
{$endif}
                                 END;

         LVITEMW              = tagLVITEMW;
         LPLVITEMW            = ^tagLVITEMW;
         TLVITEMW             = tagLVITEMW;
         PLVITEMW             = ^tagLVITEMW;

{$IFDEF UNICODE}
         LVITEM              = LVITEMW;
         LPLVITEM            = LPLVITEMW;

//         LVITEM_V1_SIZE                 = LVITEMW_V1_SIZE;
{$ELSE}

TYPE
         LVITEM              = LVITEMA;
         LPLVITEM            = LPLVITEMA;

//         LVITEM_V1_SIZE                 = LVITEMA_V1_SIZE;
{$ENDIF}

Type
{$ifdef ie3plus}

         LV_ITEMA            = LVITEMA;
         LV_ITEMW            = LVITEMW;
{$ELSE}

         tagLVITEMA          = _LV_ITEMA;
         LVITEMA             = LV_ITEMA;
         tagLVITEMW          = _LV_ITEMW;
         LVITEMW             = LV_ITEMW;
{$ENDIF}
         LV_ITEM                        = LVITEM;
         TLVItem                        = LVITEM;
         PLVItem                        = LPLVITEM;
         TLV_ITEM                       = LVITEM;
         PLV_ITEM                       = PLVITEM;
CONST
         LPSTR_TEXTCALLBACKW = LPWSTR(-1);
         LPSTR_TEXTCALLBACKA = LPSTR (-1);
{$IFDEF UNICODE}
         LPSTR_TEXTCALLBACK  = LPSTR_TEXTCALLBACKW;
{$ELSE}
         LPSTR_TEXTCALLBACK  = LPSTR_TEXTCALLBACKA;
{$ENDIF}


CONST
         I_IMAGECALLBACK                = (-1);
{$ifdef ie501plus}
         I_IMAGENONE                    = (-2);
{$ENDIF}  // 0x0501

{$ifdef win32xp}
// For tileview
         I_COLUMNSCALLBACK              = UINT(-1);
{$ENDIF}

         LVM_GETITEMA                   = (LVM_FIRST + 5);
         LVM_GETITEMW                   = (LVM_FIRST + 75);

{$IFDEF UNICODE}
         LVM_GETITEM         = LVM_GETITEMW;
{$ELSE}
         LVM_GETITEM         = LVM_GETITEMA;
{$ENDIF}

// Macro 46
Function ListView_GetItem( hwnd : hwnd;var  pitem : LV_ITEM ):BOOL;

CONST
         LVM_SETITEMA                   = (LVM_FIRST + 6);
         LVM_SETITEMW                   = (LVM_FIRST + 76);

{$IFDEF UNICODE}
         LVM_SETITEM         = LVM_SETITEMW;
{$ELSE}
         LVM_SETITEM         = LVM_SETITEMA;
{$ENDIF}

// Macro 47
Function ListView_SetItem( hwnd : hwnd;const pitem : LV_ITEM ):BOOL;


CONST
         LVM_INSERTITEMA                = (LVM_FIRST + 7);
         LVM_INSERTITEMW                = (LVM_FIRST + 77);
{$IFDEF UNICODE}
         LVM_INSERTITEM      = LVM_INSERTITEMW;
{$ELSE}
         LVM_INSERTITEM      = LVM_INSERTITEMA;
{$ENDIF}

// Macro 48
Function ListView_InsertItem( hwnd : hwnd;Const pitem : LV_ITEM ):cint;

CONST
         LVM_DELETEITEM                 = (LVM_FIRST + 8);

// Macro 49
Function ListView_DeleteItem( hwnd : hwnd; i : cint):BOOL;

CONST
         LVM_DELETEALLITEMS             = (LVM_FIRST + 9);

// Macro 50
Function ListView_DeleteAllItems( hwnd : hwnd):BOOL;

CONST
         LVM_GETCALLBACKMASK            = (LVM_FIRST + 10);

// Macro 51
Function ListView_GetCallbackMask( hwnd : hwnd):BOOL;


CONST
         LVM_SETCALLBACKMASK            = (LVM_FIRST + 11);

// Macro 52
Function ListView_SetCallbackMask( hwnd : hwnd; mask : UINT):BOOL;


CONST
         LVNI_ALL                       = $0000;
         LVNI_FOCUSED                   = $0001;
         LVNI_SELECTED                  = $0002;
         LVNI_CUT                       = $0004;
         LVNI_DROPHILITED               = $0008;

         LVNI_ABOVE                     = $0100;
         LVNI_BELOW                     = $0200;
         LVNI_TOLEFT                    = $0400;
         LVNI_TORIGHT                   = $0800;


         LVM_GETNEXTITEM                = (LVM_FIRST + 12);

// Macro 53
Function ListView_GetNextItem( hwnd : hwnd; i : cint; flags : cint ):cint;

CONST
         LVFI_PARAM                     = $0001;
         LVFI_STRING                    = $0002;
         LVFI_PARTIAL                   = $0008;
         LVFI_WRAP                      = $0020;
         LVFI_NEARESTXY                 = $0040;


TYPE

         tagLVFINDINFOA       = Record
                                 flags        : UINT;
                                 psz          : LPCSTR;
                                 lParam       : LPARAM;
                                 pt           : POINT;
                                 vkDirection  : UINT;
                                 END;
         LVFINDINFOA          = tagLVFINDINFOA;
         LPFINDINFOA          = ^tagLVFINDINFOA;
         TLVFINDINFOA         = tagLVFINDINFOA;
         PLVFINDINFOA         = ^tagLVFINDINFOA;


         tagLVFINDINFOW       = Record
                                 flags        : UINT;
                                 psz          : LPCWSTR;
                                 lParam       : LPARAM;
                                 pt           : POINT;
                                 vkDirection  : UINT;
                                 END;
         LVFINDINFOW          = tagLVFINDINFOW;
         LPFINDINFOW          = ^tagLVFINDINFOW;
         TLVFINDINFOW         = tagLVFINDINFOW;
         PLVFINDINFOW         = ^tagLVFINDINFOW;

{$IFDEF UNICODE}
         LVFINDINFO          = LVFINDINFOW;
{$ELSE}
         LVFINDINFO          = LVFINDINFOA;
{$ENDIF}

{$ifdef ie3plus}
         LV_FINDINFOA        = LVFINDINFOA;
         LV_FINDINFOW        = LVFINDINFOW;
{$ELSE}
         tagLVFINDINFOA      = _LV_FINDINFOA;
         LVFINDINFOA         = LV_FINDINFOA;
         tagLVFINDINFOW      = _LV_FINDINFOW;
         LVFINDINFOW         = LV_FINDINFOW;
{$ENDIF}

         TLVFindInfo         = LVFINDINFO;
         PLVFindInfo         = ^LVFindInfo;

         LV_FINDINFO                    = LVFINDINFO;

CONST
         LVM_FINDITEMA                  = (LVM_FIRST + 13);
         LVM_FINDITEMW                  = (LVM_FIRST + 83);
{$IFDEF UNICODE}
         LVM_FINDITEM                   = LVM_FINDITEMW;
{$ELSE}
         LVM_FINDITEM                   = LVM_FINDITEMA;
{$ENDIF}

// Macro 54
Function ListView_FindItem( hwnd : hwnd; iStart : cint;const plvfi : LV_FINDINFO ):cint;

CONST
         LVIR_BOUNDS                    = 0;
         LVIR_ICON                      = 1;
         LVIR_LABEL                     = 2;
         LVIR_SELECTBOUNDS              = 3;


         LVM_GETITEMRECT                = (LVM_FIRST + 14);

// Macro 55 NI
Function ListView_GetItemRect( hwnd : hwnd; i : cint;var prc : TRect;code : cint):BOOL;

CONST
         LVM_SETITEMPOSITION            = (LVM_FIRST + 15);

// Macro 56
Function ListView_SetItemPosition( hwndLV : hwnd; i : cint; x : cint ; y : cint ):BOOL;

CONST
         LVM_GETITEMPOSITION            = (LVM_FIRST + 16);

// Macro 57
Function ListView_GetItemPosition( hwndLV : hwnd; i : cint;var  ppt : POINT ):BOOL;

CONST
         LVM_GETSTRINGWIDTHA            = (LVM_FIRST + 17);
         LVM_GETSTRINGWIDTHW            = (LVM_FIRST + 87);
{$IFDEF UNICODE}
         LVM_GETSTRINGWIDTH  = LVM_GETSTRINGWIDTHW;
{$ELSE}
         LVM_GETSTRINGWIDTH  = LVM_GETSTRINGWIDTHA;
{$ENDIF}

// Macro 58
Function ListView_GetStringWidth( hwndLV : hwnd; psz : LPCTSTR):cint;

CONST
         LVHT_NOWHERE                   = $0001;
         LVHT_ONITEMICON                = $0002;
         LVHT_ONITEMLABEL               = $0004;
         LVHT_ONITEMSTATEICON           = $0008;
         LVHT_ONITEM                    = (LVHT_ONITEMICON  OR  LVHT_ONITEMLABEL  OR  LVHT_ONITEMSTATEICON);

         LVHT_ABOVE                     = $0008;
         LVHT_BELOW                     = $0010;
         LVHT_TORIGHT                   = $0020;
         LVHT_TOLEFT                    = $0040;



// #define LVHITTESTINFO_V1_SIZE CCSIZEOF_STRUCT(LVHITTESTINFO, iItem)

TYPE

         tagLVHITTESTINFO     = Record
                                 pt           : POINT;
                                 flags        : UINT;
                                 iItem        : cint;
{$ifdef ie3plus}
                                 iSubItem     : cint;          // this is was NOT in win95.  valid only for LVM_SUBITEMHITTEST
{$ENDIF}
{$ifdef win32vista}
				 iGroup       : cint;  // readonly. index of group. only valid for owner data.
				                       // supports single item in multiple groups.
{$endif}
                                 END;
         LVHITTESTINFO        = tagLVHITTESTINFO;
         LPLVHITTESTINFO      = ^tagLVHITTESTINFO;
         TLVHITTESTINFO       = tagLVHITTESTINFO;
         PLVHITTESTINFO       = ^tagLVHITTESTINFO;

{$ifdef ie3plus}
         LV_HITTESTINFO                 = LVHITTESTINFO;
{$ELSE}
         tagLVHITTESTINFO               = _LV_HITTESTINFO;
         LVHITTESTINFO                  = LV_HITTESTINFO;
{$ENDIF}

CONST
         LVM_HITTEST                    = (LVM_FIRST + 18);

// Macro 59
Function ListView_HitTest( hwndLV : hwnd;var  pinfo : LV_HITTESTINFO ):cint;

CONST
         LVM_ENSUREVISIBLE              = (LVM_FIRST + 19);

// Macro 60
Function ListView_EnsureVisible( hwndLV : hwnd; i : cint; fPartialOK : cint ):BOOL;inline;
Function ListView_EnsureVisible( hwndLV : hwnd; i : cint; fPartialOK : BOOL ):BOOL;inline;

CONST
         LVM_SCROLL                     = (LVM_FIRST + 20);

// Macro 61
Function ListView_Scroll( hwndLV : hwnd; dx : cint; dy : cint):BOOL;

CONST
         LVM_REDRAWITEMS                = (LVM_FIRST + 21);

// Macro 62
Function ListView_RedrawItems( hwndLV : hwnd; iFirst : cint; iLast : cint):BOOL;

CONST
         LVA_DEFAULT                    = $0000;
         LVA_ALIGNLEFT                  = $0001;
         LVA_ALIGNTOP                   = $0002;
         LVA_ALIGNRIGHT                 = $0003;
         LVA_ALIGNBOTTOM                = $0004;
         LVA_SNAPTOGRID                 = $0005;
         LVA_SORTASCENDING              = $0100;
         LVA_SORTDESCENDING             = $0200;
         LVM_ARRANGE                    = (LVM_FIRST + 22);

// Macro 63
Function ListView_Arrange( hwndLV : hwnd; code : UINT):BOOL;

CONST
         LVM_EDITLABELA                 = (LVM_FIRST + 23);
         LVM_EDITLABELW                 = (LVM_FIRST + 118);
{$IFDEF UNICODE}

         LVM_EDITLABEL                  = LVM_EDITLABELW;
{$ELSE}
         LVM_EDITLABEL                  = LVM_EDITLABELA;
{$ENDIF}

// Macro 64
Function ListView_EditLabel( hwndLV : hwnd; i : cint):HWND;

CONST
         LVM_GETEDITCONTROL             = (LVM_FIRST + 24);

// Macro 65
Function ListView_GetEditControl( hwndLV : hwnd):HWND;


// #define LVCOLUMNA_V1_SIZE CCSIZEOF_STRUCT(LVCOLUMNA, iSubItem)
// #define LVCOLUMNW_V1_SIZE CCSIZEOF_STRUCT(LVCOLUMNW, iSubItem)

TYPE

         tagLVCOLUMNA         = Record
                                 mask         : UINT;
                                 fmt          : cint;
                                 cx           : cint;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iSubItem     : cint;
{$ifdef ie3plus}
                                 iImage       : cint;
                                 iOrder       : cint;
{$ENDIF}
{$ifdef win32vista}
		 		 cxmin 	      : cint; // min snap point
				 cxDefault    : cint;   // default snap point
				 cxIdeal      : cint;     // read only. ideal may not eqaul current width if auto sized (LVS_EX_AUTOSIZECOLUMNS) to a lesser width.
{$endif}
                                 END;
         LVCOLUMNA            = tagLVCOLUMNA;
         LPLVCOLUMNA          = ^tagLVCOLUMNA;
         TLVCOLUMNA           = tagLVCOLUMNA;
         PLVCOLUMNA           = ^tagLVCOLUMNA;


         tagLVCOLUMNW         = Record
                                 mask         : UINT;
                                 fmt          : cint;
                                 cx           : cint;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iSubItem     : cint;
{$ifdef ie3plus}
                                 iImage       : cint;
                                 iOrder       : cint;
{$ENDIF}
{$ifdef win32vista}
		 		 cxmin 	      : cint; // min snap point
				 cxDefault    : cint;   // default snap point
				 cxIdeal      : cint;     // read only. ideal may not eqaul current width if auto sized (LVS_EX_AUTOSIZECOLUMNS) to a lesser width.
{$endif}
                                 END;
         LVCOLUMNW            = tagLVCOLUMNW;
         LPLVCOLUMNW          = ^tagLVCOLUMNW;
         TLVCOLUMNW           = tagLVCOLUMNW;
         PLVCOLUMNW           = ^tagLVCOLUMNW;


{$IFDEF UNICODE}
         LVCOLUMN            = LVCOLUMNW;
         LPLVCOLUMN          = LPLVCOLUMNW;
//       LVCOLUMN_V1_SIZE               = LVCOLUMNW_V1_SIZE;
{$ELSE}
         LVCOLUMN            = LVCOLUMNA;
         LPLVCOLUMN          = LPLVCOLUMNA;
//       LVCOLUMN_V1_SIZE               = LVCOLUMNA_V1_SIZE;
{$ENDIF}

{$ifdef ie3plus}
         LV_COLUMNA          = LVCOLUMNA;
         LV_COLUMNW          = LVCOLUMNW;
{$ELSE}
         tagLVCOLUMNA        = _LV_COLUMNA;
         LVCOLUMNA           = LV_COLUMNA;
         tagLVCOLUMNW        = _LV_COLUMNW;
         LVCOLUMNW           = LV_COLUMNW;
{$ENDIF}


         LV_COLUMN                      = LVCOLUMN;
         TLVColumn                      = LVCOLUMN;
         PLVColumn                      = LPLVCOLUMN;

CONST
         LVCF_FMT                       = $0001;
         LVCF_WIDTH                     = $0002;
         LVCF_TEXT                      = $0004;
         LVCF_SUBITEM                   = $0008;
{$ifdef ie3plus}
         LVCF_IMAGE                     = $0010;
         LVCF_ORDER                     = $0020;
{$ENDIF}
{$ifdef win32vista}
	 LVCF_MINWIDTH           	= $0040;
	 LVCF_DEFAULTWIDTH       	= $0080;
	 LVCF_IDEALWIDTH         	= $0100;
{$endif}

         LVCFMT_LEFT                    = $0000;
         LVCFMT_RIGHT                   = $0001;
         LVCFMT_CENTER                  = $0002;
         LVCFMT_JUSTIFYMASK             = $0003;

{$ifdef ie3plus}
         LVCFMT_IMAGE                   = $0800;
         LVCFMT_BITMAP_ON_RIGHT         = $1000;
         LVCFMT_COL_HAS_IMAGES          = $8000;
{$ENDIF}
{$ifdef win32vista}
	 LVCFMT_FIXED_WIDTH          	= $00100;  // Can't resize the column; same as HDF_FIXEDWIDTH
	 LVCFMT_NO_DPI_SCALE         	= $40000;  // If not set, CCM_DPISCALE will govern scaling up fixed width
	 LVCFMT_FIXED_RATIO          	= $80000;  // Width will augment with the row height
{$endif}

         LVM_GETCOLUMNA                 = (LVM_FIRST + 25);
         LVM_GETCOLUMNW                 = (LVM_FIRST + 95);
{$IFDEF UNICODE}
         LVM_GETCOLUMN                  = LVM_GETCOLUMNW;
{$ELSE}
         LVM_GETCOLUMN                  = LVM_GETCOLUMNA;
{$ENDIF}

// Macro 66
Function ListView_GetColumn( hwnd : hwnd; iCol : cint;var  pcol : LV_COLUMN ):BOOL;

CONST
         LVM_SETCOLUMNA                 = (LVM_FIRST + 26);
         LVM_SETCOLUMNW                 = (LVM_FIRST + 96);
{$IFDEF UNICODE}
         LVM_SETCOLUMN                  = LVM_SETCOLUMNW;
{$ELSE}
         LVM_SETCOLUMN                  = LVM_SETCOLUMNA;
{$ENDIF}

// Macro 67
Function ListView_SetColumn( hwnd : hwnd; iCol : cint;Const pcol : LV_COLUMN ):BOOL;

CONST
         LVM_INSERTCOLUMNA              = (LVM_FIRST + 27);
         LVM_INSERTCOLUMNW              = (LVM_FIRST + 97);

{$IFDEF UNICODE} // was already not active (spaces) in .h
// #   define  LVM_INSERTCOLUMN    LVM_INSERTCOLUMNW
{$ELSE}
// #   define  LVM_INSERTCOLUMN    LVM_INSERTCOLUMNA
{$ENDIF}

// Macro 68
Function ListView_InsertColumn( hwnd : hwnd; iCol : cint;Const pcol : LV_COLUMN ):cint;

CONST
         LVM_DELETECOLUMN               = (LVM_FIRST + 28);

// Macro 69
Function ListView_DeleteColumn( hwnd : hwnd; iCol : cint):BOOL;

CONST
         LVM_GETCOLUMNWIDTH             = (LVM_FIRST + 29);

// Macro 70
Function ListView_GetColumnWidth( hwnd : hwnd; iCol : cint):cint;

CONST
         LVSCW_AUTOSIZE                 = -1;
         LVSCW_AUTOSIZE_USEHEADER       = -2;
         LVM_SETCOLUMNWIDTH             = (LVM_FIRST + 30);

// Macro 71
Function ListView_SetColumnWidth( hwnd : hwnd; iCol : cint; cx : cint ):BOOL;


{$ifdef ie3plus}

CONST
         LVM_GETHEADER                  = (LVM_FIRST + 31);

// Macro 72
Function ListView_GetHeader( hwnd : hwnd):HWND;

{$ENDIF}

CONST
         LVM_CREATEDRAGIMAGE            = (LVM_FIRST + 33);

// Macro 73
Function ListView_CreateDragImage( hwnd : hwnd; i : cint; lpptUpLeft : LPPOINT):HIMAGELIST;inline;
Function ListView_CreateDragImage( hwnd : hwnd; i : cint; const lpptUpLeft : POINT):HIMAGELIST;inline;

CONST
         LVM_GETVIEWRECT                = (LVM_FIRST + 34);

// Macro 74
Function ListView_GetViewRect( hwnd : hwnd;var  prc : RECT ):BOOL;

CONST
         LVM_GETTEXTCOLOR               = (LVM_FIRST + 35);

// Macro 75
Function ListView_GetTextColor( hwnd : hwnd):COLORREF;

CONST
         LVM_SETTEXTCOLOR               = (LVM_FIRST + 36);

// Macro 76
Function ListView_SetTextColor( hwnd : hwnd; clrText : COLORREF):BOOL;

CONST
         LVM_GETTEXTBKCOLOR             = (LVM_FIRST + 37);

// Macro 77
Function ListView_GetTextBkColor( hwnd : hwnd):COLORREF;

CONST
         LVM_SETTEXTBKCOLOR             = (LVM_FIRST + 38);

// Macro 78
Function ListView_SetTextBkColor( hwnd : hwnd; clrTextBk : COLORREF):BOOL;


CONST
         LVM_GETTOPINDEX                = (LVM_FIRST + 39);

// Macro 79
Function ListView_GetTopIndex( hwndLV : hwnd):cint;

CONST
         LVM_GETCOUNTPERPAGE            = (LVM_FIRST + 40);

// Macro 80
Function ListView_GetCountPerPage( hwndLV : hwnd):cint;

CONST
         LVM_GETORIGIN                  = (LVM_FIRST + 41);

// Macro 81
Function ListView_GetOrigin( hwndLV : hwnd;var  ppt : POINT ):BOOL;

CONST
         LVM_UPDATE                     = (LVM_FIRST + 42);

// Macro 82
Function ListView_Update( hwndLV : hwnd; i : WPARAM):BOOL;

CONST
         LVM_SETITEMSTATE               = (LVM_FIRST + 43);

// Macro 83
Procedure ListView_SetItemState(hwndLV :hwnd; i :cint ;data,mask:UINT);

{$ifdef ie3plus}
// Macro 84
Procedure ListView_SetCheckState( hwndLV : hwnd; i : cint ; fCheck : bool );

{$ENDIF}


CONST
         LVM_GETITEMSTATE               = (LVM_FIRST + 44);

// Macro 85
Function ListView_GetItemState( hwndLV : hwnd; i : WPARAM; mask : LPARAM):UINT;


{$ifdef ie3plus}

// Macro 86
Function ListView_GetCheckState( hwndLV : hwnd; i : WPARAM):UINT;

{$ENDIF}

CONST
         LVM_GETITEMTEXTA               = (LVM_FIRST + 45);
         LVM_GETITEMTEXTW               = (LVM_FIRST + 115);

{$IFDEF UNICODE}
         LVM_GETITEMTEXT                = LVM_GETITEMTEXTW;
{$ELSE}
         LVM_GETITEMTEXT                = LVM_GETITEMTEXTA;
{$ENDIF}

// Macro 87
Procedure ListView_GetItemText(hwndLV:hwnd; i : WPARAM ;iSubItem : cint ;pszText : Pointer;cchTextMax :cint);

CONST
         LVM_SETITEMTEXTA               = (LVM_FIRST + 46);
         LVM_SETITEMTEXTW               = (LVM_FIRST + 116);

{$IFDEF UNICODE}
         LVM_SETITEMTEXT                = LVM_SETITEMTEXTW;
{$ELSE}
         LVM_SETITEMTEXT                = LVM_SETITEMTEXTA;
{$ENDIF}

// Macro 88
Procedure ListView_SetItemText(hwndLV:hwnd; i : WPARAM ;iSubItem : cint ;pszText : Pointer);

{$ifdef ie3plus}
// these flags only apply to LVS_OWNERDATA listviews in report or list mode

CONST
         LVSICF_NOINVALIDATEALL         = $00000001;
         LVSICF_NOSCROLL                = $00000002;
{$ENDIF}

         LVM_SETITEMCOUNT               = (LVM_FIRST + 47);

// Macro 89
Procedure ListView_SetItemCount( hwndLV : hwnd; cItems : WPARAM);

{$ifdef ie3plus}

// Macro 90
Procedure ListView_SetItemCountEx( hwndLV : hwnd; cItems : WPARAM; dwFlags : LPARAM);

{$ENDIF}

TYPE
         PFNLVCOMPARE=Function(a,b,c:LPARAM):cint;
         TLVCompare = PFNLVCOMPARE;

CONST
         LVM_SORTITEMS                  = (LVM_FIRST + 48);

// Macro 91
Function ListView_SortItems( hwndLV : hwnd;pfnCompare : PFNLVCOMPARE; lPrm : LPARAM):BOOL;

CONST
         LVM_SETITEMPOSITION32          = (LVM_FIRST + 49);

// Macro 92
Procedure ListView_SetItemPosition32(hwndLV:hwnd; i:cint;x0,y0:long);

CONST
         LVM_GETSELECTEDCOUNT           = (LVM_FIRST + 50);

// Macro 93
Function ListView_GetSelectedCount( hwndLV : hwnd):UINT;

CONST
         LVM_GETITEMSPACING             = (LVM_FIRST + 51);

// Macro 94
Function ListView_GetItemSpacing( hwndLV : hwnd; fSmall : cint ):DWORD;

CONST
         LVM_GETISEARCHSTRINGA          = (LVM_FIRST + 52);
         LVM_GETISEARCHSTRINGW          = (LVM_FIRST + 117);

{$IFDEF UNICODE}
         LVM_GETISEARCHSTRING           = LVM_GETISEARCHSTRINGW;
{$ELSE}
         LVM_GETISEARCHSTRING           = LVM_GETISEARCHSTRINGA;
{$ENDIF}

// Macro 95
Function ListView_GetISearchString( hwndLV : hwnd; lpsz : LPTSTR):BOOL;


{$ifdef ie3plus}

CONST
         LVM_SETICONSPACING             = (LVM_FIRST + 53);
// -1 for cx and cy means we'll use the default (system settings)
// 0 for cx or cy means use the current setting (allows you to change just one param)

// Macro 96
Function ListView_SetIconSpacing( hwndLV : hwnd; cx, cy : cint ):DWORD;

CONST
         LVM_SETEXTENDEDLISTVIEWSTYLE   = (LVM_FIRST + 54);   // optional wParam == mask

// Macro 97
Function ListView_SetExtendedListViewStyle( hwndLV : hwnd; dw : cint ):DWORD;

{$ifdef ie4plus}

// Macro 98
Function ListView_SetExtendedListViewStyleEx( hwndLV : hwnd; dwMask, dw : cint ):DWORD;

{$ENDIF}

CONST
         LVM_GETEXTENDEDLISTVIEWSTYLE   = (LVM_FIRST + 55);

// Macro 99
Function ListView_GetExtendedListViewStyle( hwndLV : hwnd):DWORD;

CONST
         LVS_EX_GRIDLINES               = $00000001;
         LVS_EX_SUBITEMIMAGES           = $00000002;
         LVS_EX_CHECKBOXES              = $00000004;
         LVS_EX_TRACKSELECT             = $00000008;
         LVS_EX_HEADERDRAGDROP          = $00000010;
         LVS_EX_FULLROWSELECT           = $00000020;          // applies to report mode only
         LVS_EX_ONECLICKACTIVATE        = $00000040;
         LVS_EX_TWOCLICKACTIVATE        = $00000080;
{$ifdef ie4plus}
         LVS_EX_FLATSB                  = $00000100;
         LVS_EX_REGIONAL                = $00000200;
         LVS_EX_INFOTIP                 = $00000400;          // listview does InfoTips for you
         LVS_EX_UNDERLINEHOT            = $00000800;
         LVS_EX_UNDERLINECOLD           = $00001000;
         LVS_EX_MULTIWORKAREAS          = $00002000;
{$ENDIF}
{$ifdef ie5plus}
         LVS_EX_LABELTIP                = $00004000;          // listview unfolds partly hidden labels if it does not have infotip text
         LVS_EX_BORDERSELECT            = $00008000;          // border selection style instead of highlight
{$ENDIF}  // End (_WIN32_IE >= 0x0500)
{$ifdef win32xp}
         LVS_EX_DOUBLEBUFFER            = $00010000;
         LVS_EX_HIDELABELS              = $00020000;
         LVS_EX_SINGLEROW               = $00040000;
         LVS_EX_SNAPTOGRID              = $00080000;          // Icons automatically snap to grid.
         LVS_EX_SIMPLESELECT            = $00100000;          // Also changes overlay rendering to top right for icon mode.
{$ENDIF}
{$ifdef win32vista}
         LVS_EX_JUSTIFYCOLUMNS          = $00200000;  // Icons are lined up in columns that use up the whole view area.
         LVS_EX_TRANSPARENTBKGND        = $00400000;  // Background is painted by the parent via WM_PRINTCLIENT
         LVS_EX_TRANSPARENTSHADOWTEXT   = $00800000;  // Enable shadow text on transparent backgrounds only (useful with bitmaps)
         LVS_EX_AUTOAUTOARRANGE         = $01000000;  // Icons automatically arrange if no icon positions have been set
         LVS_EX_HEADERINALLVIEWS        = $02000000;  // Display column header in all view modes
         LVS_EX_AUTOCHECKSELECT         = $08000000;
         LVS_EX_AUTOSIZECOLUMNS         = $10000000;
         LVS_EX_COLUMNSNAPPOINTS        = $40000000;
         LVS_EX_COLUMNOVERFLOW          = $80000000;
{$endif}

         LVM_GETSUBITEMRECT             = (LVM_FIRST + 56);

// Macro 100
Function ListView_GetSubItemRect( hwnd : hwnd; iItem : cint;iSubItem, code :cint ;prc:LPRECT):BOOL;

CONST
         LVM_SUBITEMHITTEST             = (LVM_FIRST + 57);

// Macro 101
Function ListView_SubItemHitTest( hwnd : hwnd; plvhti : LPLVHITTESTINFO):cint;

CONST
         LVM_SETCOLUMNORDERARRAY        = (LVM_FIRST + 58);

// Macro 102
Function ListView_SetColumnOrderArray( hwnd : hwnd; iCount : WPARAM; pi : LPINT):BOOL;

CONST
         LVM_GETCOLUMNORDERARRAY        = (LVM_FIRST + 59);

// Macro 103
Function ListView_GetColumnOrderArray( hwnd : hwnd; iCount : WPARAM; pi : LPINT):BOOL;


CONST
         LVM_SETHOTITEM                 = (LVM_FIRST + 60);

// Macro 104
Function ListView_SetHotItem( hwnd : hwnd; i : WPARAM):cint;

CONST
         LVM_GETHOTITEM                 = (LVM_FIRST + 61);

// Macro 105
Function ListView_GetHotItem( hwnd : hwnd):cint;

CONST
         LVM_SETHOTCURSOR               = (LVM_FIRST + 62);

// Macro 106
Function ListView_SetHotCursor( hwnd : hwnd; hcur : LPARAM):HCURSOR;

CONST
         LVM_GETHOTCURSOR               = (LVM_FIRST + 63);

// Macro 107
Function ListView_GetHotCursor( hwnd : hwnd):HCURSOR;

CONST
         LVM_APPROXIMATEVIEWRECT        = (LVM_FIRST + 64);

// Macro 108
Function ListView_ApproximateViewRect( hwnd : hwnd; iWidth, iHeight, iCount : cint ):DWORD;

{$ENDIF}      // _WIN32_IE >= 0x0300

{$ifdef ie4plus}

CONST
         LV_MAX_WORKAREAS               = 16;
         LVM_SETWORKAREAS               = (LVM_FIRST + 65);

// Macro 109
Function ListView_SetWorkAreas( hwnd : hwnd; nWorkAreas : cint;var  prc : RECT ):BOOL;inline;
Function ListView_SetWorkAreas( hwnd : hwnd; nWorkAreas : cint;  prc : PRECT ):BOOL;inline;

CONST
         LVM_GETWORKAREAS               = (LVM_FIRST + 70);

// Macro 110
Function ListView_GetWorkAreas( hwnd : hwnd; nWorkAreas : cint;var  prc : RECT ):BOOL;

CONST
         LVM_GETNUMBEROFWORKAREAS       = (LVM_FIRST + 73);

// Macro 111
Function ListView_GetNumberOfWorkAreas( hwnd : hwnd;var  pnWorkAreas : UINT ):BOOL;

CONST
         LVM_GETSELECTIONMARK           = (LVM_FIRST + 66);

// Macro 112
Function ListView_GetSelectionMark( hwnd : hwnd):cint;

CONST
         LVM_SETSELECTIONMARK           = (LVM_FIRST + 67);

// Macro 113
Function ListView_SetSelectionMark( hwnd : hwnd; i : LPARAM):cint;

CONST
         LVM_SETHOVERTIME               = (LVM_FIRST + 71);

// Macro 114
Function ListView_SetHoverTime( hwndLV : hwnd; dwHoverTimeMs : LPARAM):DWORD;

CONST
         LVM_GETHOVERTIME               = (LVM_FIRST + 72);

// Macro 115
Function ListView_GetHoverTime( hwndLV : hwnd):DWORD;

CONST
         LVM_SETTOOLTIPS                = (LVM_FIRST + 74);

// Macro 116
Function ListView_SetToolTips( hwndLV : hwnd; hwndNewHwnd : WPARAM):HWND;

CONST
         LVM_GETTOOLTIPS                = (LVM_FIRST + 78);

// Macro 117
Function ListView_GetToolTips( hwndLV : hwnd):HWND;

CONST
         LVM_SORTITEMSEX                = (LVM_FIRST + 81);

// Macro 118
Function ListView_SortItemsEx( hwndLV : hwnd; _pfnCompare : PFNLVCOMPARE; _lPrm : LPARAM):BOOL;

TYPE

         tagLVBKIMAGEA        = Record
                                 ulFlags      : ULONG;          // LVBKIF_*
                                 hbm          : HBITMAP;
                                 pszImage     : LPSTR;
                                 cchImageMax  : UINT;
                                 xOffsetPercent : cint;
                                 yOffsetPercent : cint;
                                 END;
         LVBKIMAGEA           = tagLVBKIMAGEA;
         LPLVBKIMAGEA         = ^tagLVBKIMAGEA;
         TLVBKIMAGEA          = tagLVBKIMAGEA;
         PLVBKIMAGEA          = ^tagLVBKIMAGEA;

         tagLVBKIMAGEW        = Record
                                 ulFlags      : ULONG;          // LVBKIF_*
                                 hbm          : HBITMAP;
                                 pszImage     : LPWSTR;
                                 cchImageMax  : UINT;
                                 xOffsetPercent : cint;
                                 yOffsetPercent : cint;
                                 END;
         LVBKIMAGEW           = tagLVBKIMAGEW;
         LPLVBKIMAGEW         = ^tagLVBKIMAGEW;
         TLVBKIMAGEW          = tagLVBKIMAGEW;
         PLVBKIMAGEW          = ^tagLVBKIMAGEW;

CONST
         LVBKIF_SOURCE_NONE             = $00000000;
         LVBKIF_SOURCE_HBITMAP          = $00000001;
         LVBKIF_SOURCE_URL              = $00000002;
         LVBKIF_SOURCE_MASK             = $00000003;
         LVBKIF_STYLE_NORMAL            = $00000000;
         LVBKIF_STYLE_TILE              = $00000010;
         LVBKIF_STYLE_MASK              = $00000010;
{$ifdef win32xp}
         LVBKIF_FLAG_TILEOFFSET         = $00000100;
         LVBKIF_TYPE_WATERMARK          = $10000000;
{$ENDIF}

         LVM_SETBKIMAGEA                = (LVM_FIRST + 68);
         LVM_SETBKIMAGEW                = (LVM_FIRST + 138);
         LVM_GETBKIMAGEA                = (LVM_FIRST + 69);
         LVM_GETBKIMAGEW                = (LVM_FIRST + 139);

{$ifdef win32xp}
         LVM_SETSELECTEDCOLUMN          = (LVM_FIRST + 140);

// Macro 119
Procedure ListView_SetSelectedColumn( hwnd : hwnd; iCol : WPARAM);

CONST
         LVM_SETTILEWIDTH               = (LVM_FIRST + 141);

// Macro 120
Procedure ListView_SetTileWidth( hwnd : hwnd; cpWidth : WPARAM);

CONST
         LV_VIEW_ICON                   = $0000;
         LV_VIEW_DETAILS                = $0001;
         LV_VIEW_SMALLICON              = $0002;
         LV_VIEW_LIST                   = $0003;
         LV_VIEW_TILE                   = $0004;
         LV_VIEW_MAX                    = $0004;
         LVM_SETVIEW                    = (LVM_FIRST + 142);

// Macro 121
Function ListView_SetView( hwnd : hwnd; iView : DWORD):DWORD;

CONST
         LVM_GETVIEW                    = (LVM_FIRST + 143);

// Macro 122
Function ListView_GetView( hwnd : hwnd):DWORD;

CONST
         LVGF_NONE                      = $00000000;
         LVGF_HEADER                    = $00000001;
         LVGF_FOOTER                    = $00000002;
         LVGF_STATE                     = $00000004;
         LVGF_ALIGN                     = $00000008;
         LVGF_GROUPID                   = $00000010;

{$ifdef win32vista}
         LVGF_SUBTITLE                 = $00000100;  // pszSubtitle is valid
         LVGF_TASK                     = $00000200;  // pszTask is valid
         LVGF_DESCRIPTIONTOP           = $00000400;  // pszDescriptionTop is valid
         LVGF_DESCRIPTIONBOTTOM        = $00000800;  // pszDescriptionBottom is valid
         LVGF_TITLEIMAGE               = $00001000;  // iTitleImage is valid
         LVGF_EXTENDEDIMAGE            = $00002000;  // iExtendedImage is valid
         LVGF_ITEMS                    = $00004000;  // iFirstItem and cItems are valid
         LVGF_SUBSET                   = $00008000;  // pszSubsetTitle is valid
         LVGF_SUBSETITEMS              = $00010000;  // readonly, cItems holds count of items in visible subset, iFirstItem is valid
{$endif}

         LVGS_NORMAL                    = $00000000;
         LVGS_COLLAPSED                 = $00000001;
         LVGS_HIDDEN                    = $00000002;

         LVGA_HEADER_LEFT               = $00000001;
         LVGA_HEADER_CENTER             = $00000002;
         LVGA_HEADER_RIGHT              = $00000004;          // Don't forget to validate exclusivity
         LVGA_FOOTER_LEFT               = $00000008;
         LVGA_FOOTER_CENTER             = $00000010;
         LVGA_FOOTER_RIGHT              = $00000020;          // Don't forget to validate exclusivity

TYPE

         tagLVGROUP           = Record
                                 cbSize       : UINT;
                                 mask         : UINT;
                                 pszHeader    : LPWSTR;
                                 cchHeader    : cint;
                                 pszFooter    : LPWSTR;
                                 cchFooter    : cint;
                                 iGroupId     : cint;
                                 stateMask    : UINT;
                                 state        : UINT;
                                 uAlign       : UINT;
{$ifdef win32vista}
                                 pszSubtitle         : LPWSTR;
                                 cchSubtitle         : UINT;
                                 pszTask             : LPWSTR;
                                 cchTask             : UINT;
                                 pszDescriptionTop   : LPWSTR;
                                 cchDescriptionTop   : UINT;
                                 pszDescriptionBottom: LPWSTR;
                                 cchDescriptionBottom: UINT;
                                 iTitleImage         : cint;
                                 iExtendedImage      : cint;
                                 iFirstItem          : cint;   // Read only
                                 cItems              : UINT; // Read only
                                 pszSubsetTitle      : LPWSTR;// NULL if group is not subset
                                 cchSubsetTitle      : UINT;
{$endif}
                                 END;
         LVGROUP              = tagLVGROUP;
         PLVGROUP             = ^tagLVGROUP;
         TLVGROUP             = tagLVGROUP;


CONST
         LVM_INSERTGROUP                = (LVM_FIRST + 145);

// Macro 123
Procedure ListView_InsertGroup( hwnd : hwnd; index : WPARAM; pgrp : LPARAM);

CONST
         LVM_SETGROUPINFO               = (LVM_FIRST + 147);

// Macro 124
Procedure ListView_SetGroupInfo( hwnd : hwnd; iGroupId : WPARAM; pgrp : LPARAM);

CONST
         LVM_GETGROUPINFO               = (LVM_FIRST + 149);

// Macro 125
Procedure ListView_GetGroupInfo( hwnd : hwnd; iGroupId : WPARAM; pgrp : LPARAM);

CONST
         LVM_REMOVEGROUP                = (LVM_FIRST + 150);

// Macro 126
Procedure ListView_RemoveGroup( hwnd : hwnd; iGroupId : WPARAM);

CONST
         LVM_MOVEGROUP                  = (LVM_FIRST + 151);

// Macro 127
Procedure ListView_MoveGroup( hwnd : hwnd; iGroupId : WPARAM; toIndex : LPARAM);

CONST
         LVM_MOVEITEMTOGROUP            = (LVM_FIRST + 154);

// Macro 128
Procedure ListView_MoveItemToGroup( hwnd : hwnd; idItemFrom : WPARAM; idGroupTo : LPARAM);

CONST
         LVGMF_NONE                     = $00000000;
         LVGMF_BORDERSIZE               = $00000001;
         LVGMF_BORDERCOLOR              = $00000002;
         LVGMF_TEXTCOLOR                = $00000004;

TYPE

         tagLVGROUPMETRICS    = Record
                                 cbSize       : UINT;
                                 mask         : UINT;
                                 Left         : UINT;
                                 Top          : UINT;
                                 Right        : UINT;
                                 Bottom       : UINT;
                                 crLeft       : COLORREF;
                                 crTop        : COLORREF;
                                 crRight      : COLORREF;
                                 crBottom     : COLORREF;
                                 crHeader     : COLORREF;
                                 crFooter     : COLORREF;
                                 END;
         LVGROUPMETRICS       = tagLVGROUPMETRICS;
         PLVGROUPMETRICS      = ^tagLVGROUPMETRICS;
         TLVGROUPMETRICS      = tagLVGROUPMETRICS;

CONST
         LVM_SETGROUPMETRICS            = (LVM_FIRST + 155);

// Macro 129
Procedure ListView_SetGroupMetrics( hwnd : hwnd; pGroupMetrics : LPARAM);

CONST
         LVM_GETGROUPMETRICS            = (LVM_FIRST + 156);

// Macro 130
Procedure ListView_GetGroupMetrics( hwnd : hwnd; pGroupMetrics : LPARAM);

CONST
         LVM_ENABLEGROUPVIEW            = (LVM_FIRST + 157);

// Macro 131
Procedure ListView_EnableGroupView( hwnd : hwnd; fEnable : WPARAM);

TYPE
         PFNLVGROUPCOMPARE = function(i,j:cint;k:pointer):cint; StdCall;

CONST
         LVM_SORTGROUPS                 = (LVM_FIRST + 158);

// Macro 132
Procedure ListView_SortGroups( hwnd : hwnd; _pfnGroupCompate : WPARAM; _plv : LPARAM);

TYPE

         tagLVINSERTGROUPSORTED = Record
                                   pfnGroupCompare : PFNLVGROUPCOMPARE;
                                   pvData       : Pointer;
                                   lvGroup      : LVGROUP;
                                   END;
         LVINSERTGROUPSORTED  = tagLVINSERTGROUPSORTED;
         PLVINSERTGROUPSORTED = ^tagLVINSERTGROUPSORTED;
         TLVINSERTGROUPSORTED = tagLVINSERTGROUPSORTED;


CONST
         LVM_INSERTGROUPSORTED          = (LVM_FIRST + 159);

// Macro 133
Procedure ListView_InsertGroupSorted( hwnd : hwnd; structInsert : WPARAM);

CONST
         LVM_REMOVEALLGROUPS            = (LVM_FIRST + 160);

// Macro 134
Procedure ListView_RemoveAllGroups( hwnd : hwnd);

CONST
         LVM_HASGROUP                   = (LVM_FIRST + 161);

// Macro 135
Procedure ListView_HasGroup( hwnd : hwnd; dwGroupId : DWORD );

CONST
         LVTVIF_AUTOSIZE                = $00000000;
         LVTVIF_FIXEDWIDTH              = $00000001;
         LVTVIF_FIXEDHEIGHT             = $00000002;
         LVTVIF_FIXEDSIZE               = $00000003;
{$ifdef win32vista}
  	 LVTVIF_EXTENDED       		= $00000004;
{$endif}
         LVTVIM_TILESIZE                = $00000001;
         LVTVIM_COLUMNS                 = $00000002;
         LVTVIM_LABELMARGIN             = $00000004;


TYPE

         tagLVTILEVIEWINFO    = Record
                                 cbSize       : UINT;
                                 dwMask       : DWORD;          //LVTVIM_*
                                 dwFlags      : DWORD;          //LVTVIF_*
                                 sizeTile     : SIZE;
                                 cLines       : cint;
                                 rcLabelMargin : RECT;
                                 END;
         LVTILEVIEWINFO       = tagLVTILEVIEWINFO;
         PLVTILEVIEWINFO      = ^tagLVTILEVIEWINFO;
         TLVTILEVIEWINFO      = tagLVTILEVIEWINFO;


         tagLVTILEINFO        = Record
                                 cbSize       : UINT;
                                 iItem        : cint;
                                 cColumns     : UINT;
                                 puColumns    : PUINT;
				{$ifdef win32vista}
 				 piColFmt     : PCINT;
				{$endif}
                                 END;
         LVTILEINFO           = tagLVTILEINFO;
         PLVTILEINFO          = ^tagLVTILEINFO;
         TLVTILEINFO          = tagLVTILEINFO;


CONST
         LVM_SETTILEVIEWINFO            = (LVM_FIRST + 162);

// Macro 136
Procedure ListView_SetTileViewInfo( hwnd : hwnd; ptvi : LPARAM);

CONST
         LVM_GETTILEVIEWINFO            = (LVM_FIRST + 163);

// Macro 137
Procedure ListView_GetTileViewInfo( hwnd : hwnd; ptvi : LPARAM);

CONST
         LVM_SETTILEINFO                = (LVM_FIRST + 164);

// Macro 138
Procedure ListView_SetTileInfo( hwnd : hwnd; pti : LPARAM);

CONST
         LVM_GETTILEINFO                = (LVM_FIRST + 165);

// Macro 139
Procedure ListView_GetTileInfo( hwnd : hwnd; pti : LPARAM);

TYPE

         LVINSERTMARK         = Record
                                 cbSize       : UINT;
                                 dwFlags      : DWORD;
                                 iItem        : cint;
                                 dwReserved   : DWORD;
                                 END;
         LPLVINSERTMARK       = ^LVINSERTMARK;
         TLVINSERTMARK        = LVINSERTMARK;
         PLVINSERTMARK        = ^LVINSERTMARK;

CONST
         LVIM_AFTER                     = $00000001;          // TRUE = insert After iItem, otherwise before

         LVM_SETINSERTMARK              = (LVM_FIRST + 166);

// Macro 140
Function ListView_SetInsertMark( hwnd : hwnd; lvim : lparam ):BOOL;

CONST
         LVM_GETINSERTMARK              = (LVM_FIRST + 167);

// Macro 141
Function ListView_GetInsertMark( hwnd : hwnd; lvim : lparam ):BOOL;

CONST
         LVM_INSERTMARKHITTEST          = (LVM_FIRST + 168);

// Macro 142
Function ListView_InsertMarkHitTest( hwnd : hwnd; point : LPPOINT; lvim : LPLVINSERTMARK):cint;

CONST
         LVM_GETINSERTMARKRECT          = (LVM_FIRST + 169);

// Macro 143
Function ListView_GetInsertMarkRect( hwnd : hwnd; rc : LPRECT):cint;

CONST
         LVM_SETINSERTMARKCOLOR         = (LVM_FIRST + 170);

// Macro 144
Function ListView_SetInsertMarkColor( hwnd : hwnd; color : COLORREF):COLORREF;

CONST
         LVM_GETINSERTMARKCOLOR         = (LVM_FIRST + 171);

// Macro 145
Function ListView_GetInsertMarkColor( hwnd : hwnd):COLORREF;

TYPE

         tagLVSETINFOTIP      = Record
                                 cbSize       : UINT;
                                 dwFlags      : DWORD;
                                 pszText      : LPWSTR;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 END;
         LVSETINFOTIP         = tagLVSETINFOTIP;
         PLVSETINFOTIP        = ^tagLVSETINFOTIP;
         TLVSETINFOTIP        = tagLVSETINFOTIP;


CONST
         LVM_SETINFOTIP                 = (LVM_FIRST + 173);

// Macro 146
Function ListView_SetInfoTip( hwndLV : hwnd; plvInfoTip : LPARAM):BOOL;

CONST
         LVM_GETSELECTEDCOLUMN          = (LVM_FIRST + 174);

// Macro 147
Function ListView_GetSelectedColumn( hwnd : hwnd):UINT;

CONST
         LVM_ISGROUPVIEWENABLED         = (LVM_FIRST + 175);

// Macro 148
Function ListView_IsGroupViewEnabled( hwnd : hwnd):BOOL;

CONST
         LVM_GETOUTLINECOLOR            = (LVM_FIRST + 176);

// Macro 149
Function ListView_GetOutlineColor( hwnd : hwnd):COLORREF;

CONST
         LVM_SETOUTLINECOLOR            = (LVM_FIRST + 177);

// Macro 150
Function ListView_SetOutlineColor( hwnd : hwnd; color : COLORREF):COLORREF;

CONST
         LVM_CANCELEDITLABEL            = (LVM_FIRST + 179);

// Macro 151
Procedure ListView_CancelEditLabel( hwnd : hwnd);

// These next to methods make it easy to identify an item that can be repositioned
// within listview. For example: Many developers use the lParam to store an identifier that is
// unique. Unfortunatly, in order to find this item, they have to iterate through all of the items
// in the listview. Listview will maintain a unique identifier.  The upper bound is the size of a DWORD.

CONST
         LVM_MAPINDEXTOID               = (LVM_FIRST + 180);

// Macro 152
Function ListView_MapIndexToID( hwnd : hwnd; index : WPARAM):UINT;

CONST
         LVM_MAPIDTOINDEX               = (LVM_FIRST + 181);

// Macro 153
Function ListView_MapIDToIndex( hwnd : hwnd; id : WPARAM):UINT;

const    LVM_ISITEMVISIBLE    		= (LVM_FIRST + 182);

// macro 153b
function  ListView_IsItemVisible(hwnd:hwnd; aindex:cuint):cuint;
//    (UINT)SNDMSG((hwnd), LVM_ISITEMVISIBLE, (WPARAM)(index), (LPARAM)0)

{$ENDIF}

{$ifdef win32vista}
CONST
	LVM_GETEMPTYTEXT   		= (LVM_FIRST + 204);
	LVM_GETFOOTERRECT  		= (LVM_FIRST + 205);
	LVM_GETFOOTERINFO 		= (LVM_FIRST + 206);
	LVM_GETFOOTERITEMRECT 		= (LVM_FIRST + 207);
	LVM_GETFOOTERITEM 		= (LVM_FIRST + 208);
	LVM_GETITEMINDEXRECT    	= (LVM_FIRST + 209);
	LVM_SETITEMINDEXSTATE   	= (LVM_FIRST + 210);
	LVM_GETNEXTITEMINDEX    	= (LVM_FIRST + 211);

// footer flags
        LVFF_ITEMCOUNT         		= $00000001;

// footer item flags
        LVFIF_TEXT               	= $00000001;
        LVFIF_STATE              	= $00000002;

// footer item state
	LVFIS_FOCUSED            	= $0001;

TYPE
        tagLVFOOTERINFO                 = Record
					    mask      : CUINT;          // LVFF_*
					    pszText   : LPWSTR;
					    cchTextMax: CINT;
					    cItems    : CUINT;
                                           end;

        LVFOOTERINFO                    = tagLVFOOTERINFO;
        LPLVFOOTERINFO                  = ^tagLVFOOTERINFO;
        TLVFOOTERINFO                   = tagLVFOOTERINFO;
        PLVFOOTERINFO                   = LPLVFOOTERINFO;


        tagLVFOOTERITEM                 = Record
					    mask      : CUINT;          // LVFIF_*
					    iItem     : CINT;
					    pszText   : LPWSTR;
					    cchTextMax: CINT;
					    state     : CUINT;         // LVFIS_*
					    stateMask : CUINT;     // LVFIS_*
                                          end;

        LVFOOTERITEM                    = tagLVFOOTERITEM;
        LPLVFOOTERITEM                  = ^tagLVFOOTERITEM;
        TLVFOOTERITEM                   = tagLVFOOTERITEM;
        PLVFOOTERITEM                   = LPLVFOOTERITEM;

// supports a single item in multiple groups.
        tagLVITEMINDEX                  = Record
					   iItem      : CINT; // listview item index
					   iGroup     : CINT; // group index (must be -1 if group view is not enabled)
                                           end;
        LVITEMINDEX			= tagLVITEMINDEX;
        PLVITEMINDEX			= ^tagLVITEMINDEX;
        tLVITEMINDEX			= TAGLVITEMINDEX;


function ListView_SetGroupHeaderImageList(hwnd:HWNd;himl:HIMAGELIST):HIMAGELIST;

function ListView_GetGroupHeaderImageList(hwnd:HWND):HIMAGELIST;

function ListView_GetEmptyText(hwnd:HWND;pszText:LPWSTR; cchText:CUINT):BOOL;

function ListView_GetFooterRect(hwnd:HWND; prc:PRECT):BOOL;

function ListView_GetFooterInfo(hwnd:HWND;plvfi: LPLVFOOTERINFO ):BOOL;

function ListView_GetFooterItemRect(hwnd:HWND;iItem:CUINT;prc:PRECT):BOOL;

function ListView_GetFooterItem(hwnd:HWND;iItem:CUINT; pfi:PLVFOOTERITEM):BOOL;

function ListView_GetItemIndexRect(hwnd:hwnd; plvii:PLVITEMINDEX; iSubItem:clong; code:clong; prc:LPRECT) :BOOL;

function ListView_SetItemIndexState(hwndLV:HWND; plvii:PLVITEMINDEX; data:CUINT; mask:CUINT):HRESULT;

function ListView_GetNextItemIndex(hwnd:HWND;plvii:PLVITEMINDEX; flags:LPARAM):BOOL;

{$endif}

Type

{$IFDEF UNICODE}
         LVBKIMAGE           = LVBKIMAGEW;
         LPLVBKIMAGE         = LPLVBKIMAGEW;
CONST
         LVM_SETBKIMAGE      = LVM_SETBKIMAGEW;
         LVM_GETBKIMAGE      = LVM_GETBKIMAGEW;
{$ELSE}
         LVBKIMAGE           = LVBKIMAGEA;
         LPLVBKIMAGE         = LPLVBKIMAGEA;
CONST
         LVM_SETBKIMAGE      = LVM_SETBKIMAGEA;
         LVM_GETBKIMAGE      = LVM_GETBKIMAGEA;
{$ENDIF}


// Macro 154
Function ListView_SetBkImage( hwnd : hwnd; plvbki : LPARAM):BOOL;

// Macro 155
Function ListView_GetBkImage( hwnd : hwnd; plvbki : LPARAM):BOOL;

{$ENDIF}      // _WIN32_IE >= 0x0400

TYPE

         tagNMLISTVIEW        = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 uNewState    : UINT;
                                 uOldState    : UINT;
                                 uChanged     : UINT;
                                 ptAction     : POINT;
                                 lParam       : LPARAM;
                                 END;
         NMLISTVIEW           = tagNMLISTVIEW;
         LPNMLISTVIEW         = ^tagNMLISTVIEW;
         TNMLISTVIEW          = tagNMLISTVIEW;
         PNMLISTVIEW          = ^tagNMLISTVIEW;

{$ifdef IE4plus}
// NMITEMACTIVATE is used instead of NMLISTVIEW in IE >= 0x400
// therefore all the fields are the same except for extra uKeyFlags
// they are used to store key flags at the time of the single click with
// delayed activation - because by the time the timer goes off a user may
// not hold the keys (shift, ctrl) any more
         tagNMITEMACTIVATE    = Record
                                 hdr          : NMHDR;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 uNewState    : UINT;
                                 uOldState    : UINT;
                                 uChanged     : UINT;
                                 ptAction     : POINT;
                                 lParam       : LPARAM;
                                 uKeyFlags    : UINT;
                                 END;
         NMITEMACTIVATE       = tagNMITEMACTIVATE;
         LPNMITEMACTIVATE     = ^tagNMITEMACTIVATE;
         TNMITEMACTIVATE      = tagNMITEMACTIVATE;
         PNMITEMACTIVATE      = ^tagNMITEMACTIVATE;

// key flags stored in uKeyFlags

CONST
         LVKF_ALT                       = $0001;
         LVKF_CONTROL                   = $0002;
         LVKF_SHIFT                     = $0004;
{$ENDIF} //(_WIN32_IE >= 0x0400)
Type

{$ifdef ie3plus}
         LPNM_LISTVIEW       = LPNMLISTVIEW;
         NM_LISTVIEW         = NMLISTVIEW;
{$ELSE}
         tagNMLISTVIEW       = _NM_LISTVIEW;
         NMLISTVIEW          = NM_LISTVIEW;
         LPNMLISTVIEW        = LPNM_LISTVIEW;
{$ENDIF}


{$ifdef ie3plus}
// #define NMLVCUSTOMDRAW_V3_SIZE CCSIZEOF_STRUCT(NMLVCUSTOMDRW, clrTextBk)

TYPE

         tagNMLVCUSTOMDRAW    = Record
                                 nmcd         : NMCUSTOMDRAW;
                                 clrText      : COLORREF;
                                 clrTextBk    : COLORREF;
{$ifdef ie4plus}
                                 iSubItem     : cint;
{$ENDIF}
{$ifdef win32xp}
                                 dwItemType   : DWORD;

                                 clrFace      : COLORREF;
                                 iIconEffect  : cint;
                                 iIconPhase   : cint;
                                 iPartId      : cint;
                                 iStateId     : cint;

                                 rcText       : RECT;
                                 uAlign       : UINT;          // Alignment. Use LVGA_HEADER_CENTER, LVGA_HEADER_RIGHT, LVGA_HEADER_LEFT
{$ENDIF}
                                 END;
         NMLVCUSTOMDRAW       = tagNMLVCUSTOMDRAW;
         LPNMLVCUSTOMDRAW     = ^tagNMLVCUSTOMDRAW;
         TNMLVCUSTOMDRAW      = tagNMLVCUSTOMDRAW;
         PNMLVCUSTOMDRAW      = ^tagNMLVCUSTOMDRAW;


// dwItemType

CONST
         LVCDI_ITEM                     = $00000000;
         LVCDI_GROUP                    = $00000001;

// ListView custom draw return values
         LVCDRF_NOSELECT                = $00010000;
         LVCDRF_NOGROUPFRAME            = $00020000;

TYPE

         tagNMLVCACHEHINT     = Record
                                 hdr          : NMHDR;
                                 iFrom        : cint;
                                 iTo          : cint;
                                 END;
         NMLVCACHEHINT        = tagNMLVCACHEHINT;
         LPNMLVCACHEHINT      = ^tagNMLVCACHEHINT;
         TNMLVCACHEHINT       = tagNMLVCACHEHINT;
         PNMLVCACHEHINT       = ^tagNMLVCACHEHINT;


         LPNM_CACHEHINT      = LPNMLVCACHEHINT;
         PNM_CACHEHINT       = LPNMLVCACHEHINT;

         NM_CACHEHINT        = NMLVCACHEHINT;

         tagNMLVFINDITEMA     = Record
                                 hdr          : NMHDR;
                                 iStart       : cint;
                                 lvfi         : LVFINDINFOA;
                                 END;
         NMLVFINDITEMA        = tagNMLVFINDITEMA;
         LPNMLVFINDITEMA      = ^tagNMLVFINDITEMA;
         TNMLVFINDITEMA       = tagNMLVFINDITEMA;
         PNMLVFINDITEMA       = ^tagNMLVFINDITEMA;


         tagNMLVFINDITEMW     = Record
                                 hdr          : NMHDR;
                                 iStart       : cint;
                                 lvfi         : LVFINDINFOW;
                                 END;
         NMLVFINDITEMW        = tagNMLVFINDITEMW;
         LPNMLVFINDITEMW      = ^tagNMLVFINDITEMW;
         TNMLVFINDITEMW       = tagNMLVFINDITEMW;
         PNMLVFINDITEMW       = ^tagNMLVFINDITEMW;


         PNM_FINDITEMA       = LPNMLVFINDITEMA;
         LPNM_FINDITEMA      = LPNMLVFINDITEMA;
         NM_FINDITEMA        = NMLVFINDITEMA;

         PNM_FINDITEMW       = LPNMLVFINDITEMW;
         LPNM_FINDITEMW      = LPNMLVFINDITEMW;
         NM_FINDITEMW        = NMLVFINDITEMW;

{$IFDEF UNICODE}
         PNM_FINDITEM        = PNM_FINDITEMW;
         LPNM_FINDITEM       = LPNM_FINDITEMW;
         NM_FINDITEM         = NM_FINDITEMW;
         NMLVFINDITEM        = NMLVFINDITEMW;
         LPNMLVFINDITEM      = LPNMLVFINDITEMW;
{$ELSE}
         PNM_FINDITEM        = PNM_FINDITEMA;
         LPNM_FINDITEM       = LPNM_FINDITEMA;
         NM_FINDITEM         = NM_FINDITEMA;
         NMLVFINDITEM        = NMLVFINDITEMA;
         LPNMLVFINDITEM      = LPNMLVFINDITEMA;
{$ENDIF}
         PNMLVFindItem       = LPNMLVFINDITEM;
         TNMLVFindItem       = NMLVFINDITEM;

         tagNMLVODSTATECHANGE = Record
                                 hdr          : NMHDR;
                                 iFrom        : cint;
                                 iTo          : cint;
                                 uNewState    : UINT;
                                 uOldState    : UINT;
                                 END;
         NMLVODSTATECHANGE    = tagNMLVODSTATECHANGE;
         LPNMLVODSTATECHANGE  = ^tagNMLVODSTATECHANGE;
         TNMLVODSTATECHANGE   = tagNMLVODSTATECHANGE;
         PNMLVODSTATECHANGE   = ^tagNMLVODSTATECHANGE;


         PNM_ODSTATECHANGE   = LPNMLVODSTATECHANGE;
         LPNM_ODSTATECHANGE  = LPNMLVODSTATECHANGE;


         NM_ODSTATECHANGE               = NMLVODSTATECHANGE;
{$ENDIF}      // _WIN32_IE >= 0x0300

CONST
         LVN_ITEMCHANGING               = (LVN_FIRST-0);
         LVN_ITEMCHANGED                = (LVN_FIRST-1);
         LVN_INSERTITEM                 = (LVN_FIRST-2);
         LVN_DELETEITEM                 = (LVN_FIRST-3);
         LVN_DELETEALLITEMS             = (LVN_FIRST-4);
         LVN_BEGINLABELEDITA            = (LVN_FIRST-5);
         LVN_BEGINLABELEDITW            = (LVN_FIRST-75);
         LVN_ENDLABELEDITA              = (LVN_FIRST-6);
         LVN_ENDLABELEDITW              = (LVN_FIRST-76);
         LVN_COLUMNCLICK                = (LVN_FIRST-8);
         LVN_BEGINDRAG                  = (LVN_FIRST-9);
         LVN_BEGINRDRAG                 = (LVN_FIRST-11);

{$ifdef ie3plus}
         LVN_ODCACHEHINT                = (LVN_FIRST-13);
         LVN_ODFINDITEMA                = (LVN_FIRST-52);
         LVN_ODFINDITEMW                = (LVN_FIRST-79);

         LVN_ITEMACTIVATE               = (LVN_FIRST-14);
         LVN_ODSTATECHANGED             = (LVN_FIRST-15);

Const
{$IFDEF UNICODE}
         LVN_ODFINDITEM      = LVN_ODFINDITEMW;
{$ELSE}
         LVN_ODFINDITEM      = LVN_ODFINDITEMA;
{$ENDIF}
{$ENDIF}      // _WIN32_IE >= 0x0300


CONST
{$ifdef ie4plus}
         LVN_HOTTRACK                   = (LVN_FIRST-21);
{$ENDIF}

         LVN_GETDISPINFOA               = (LVN_FIRST-50);
         LVN_GETDISPINFOW               = (LVN_FIRST-77);
         LVN_SETDISPINFOA               = (LVN_FIRST-51);
         LVN_SETDISPINFOW               = (LVN_FIRST-78);

Const
{$IFDEF UNICODE}
         LVN_BEGINLABELEDIT  = LVN_BEGINLABELEDITW;
         LVN_ENDLABELEDIT    = LVN_ENDLABELEDITW;
         LVN_GETDISPINFO     = LVN_GETDISPINFOW;
         LVN_SETDISPINFO     = LVN_SETDISPINFOW;
{$ELSE}
         LVN_BEGINLABELEDIT  = LVN_BEGINLABELEDITA;
         LVN_ENDLABELEDIT    = LVN_ENDLABELEDITA;
         LVN_GETDISPINFO     = LVN_GETDISPINFOA;
         LVN_SETDISPINFO     = LVN_SETDISPINFOA;
{$ENDIF}

CONST
         LVIF_DI_SETITEM                = $1000;

TYPE

         tagLVDISPINFO       = Record
                                 hdr          : NMHDR;
                                 item         : LVITEMA;
                                 END;
         NMLVDISPINFOA       = tagLVDISPINFO;
         LPNMLVDISPINFOA     = ^tagLVDISPINFO;
         TLVDISPINFO         = tagLVDISPINFO;
         PLVDISPINFO         = ^tagLVDISPINFO;


         tagLVDISPINFOW      = Record
                                 hdr          : NMHDR;
                                 item         : LVITEMW;
                                 END;
         NMLVDISPINFOW       = tagLVDISPINFOW;
         LPNMLVDISPINFOW     = ^tagLVDISPINFOW;
         TLVDISPINFOW        = tagLVDISPINFOW;
         PLVDISPINFOW        = ^tagLVDISPINFOW;


{$IFDEF UNICODE}
         NMLVDISPINFO        = NMLVDISPINFOW;
{$ELSE}
         NMLVDISPINFO        = NMLVDISPINFOA;
{$ENDIF}

{$ifdef ie3plus}
         LV_DISPINFOA        = NMLVDISPINFOA;
         LV_DISPINFOW        = NMLVDISPINFOW;
{$ELSE}
         tagLVDISPINFO       = _LV_DISPINFO;
         NMLVDISPINFOA       = LV_DISPINFOA;
         tagLVDISPINFOW      = _LV_DISPINFOW;
         NMLVDISPINFOW       = LV_DISPINFOW;
{$ENDIF}


         LV_DISPINFO         = NMLVDISPINFO;

CONST
         LVN_KEYDOWN                    = (LVN_FIRST-55);



{$IFDEF _WIN32}
// include <pshpack1.h>
{$ENDIF}

TYPE
         tagLVKEYDOWN         = Record
                                 hdr          : NMHDR;
                                 wVKey        : WORD;
                                 flags        : UINT;
                                 END;
         NMLVKEYDOWN          = tagLVKEYDOWN;
         LPNMLVKEYDOWN        = ^tagLVKEYDOWN;
         TLVKEYDOWN           = tagLVKEYDOWN;
         PLVKEYDOWN           = ^tagLVKEYDOWN;


{$IFDEF _WIN32}
//include <poppack.h>
{$ENDIF}

{$ifdef ie3plus}
CONST
         LVN_MARQUEEBEGIN               = (LVN_FIRST-56);
{$ENDIF}

{$ifdef win32vista}
Type
	 tagNMLVLIN           = Record
                                  hdr:         NMHDR;
                                  link:        LITEM;
                                  iItem:       cint;
                                  iSubItem:    cint;
				end;
	 NMLVLINK	      = tagNMLVLIN;
	 TNMLVLINK	      = tagNMLVLIN;
	 PNMLVLINK	      = ^tagNMLVLIN;
{$endif}

{$ifdef ie4plus}
TYPE

         tagNMLVGETINFOTIPA   = Record
                                 hdr          : NMHDR;
                                 dwFlags      : DWORD;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 lParam       : LPARAM;
                                 END;
         NMLVGETINFOTIPA      = tagNMLVGETINFOTIPA;
         LPNMLVGETINFOTIPA    = ^tagNMLVGETINFOTIPA;
         TNMLVGETINFOTIPA     = tagNMLVGETINFOTIPA;
         PNMLVGETINFOTIPA     = ^tagNMLVGETINFOTIPA;


         tagNMLVGETINFOTIPW   = Record
                                 hdr          : NMHDR;
                                 dwFlags      : DWORD;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iItem        : cint;
                                 iSubItem     : cint;
                                 lParam       : LPARAM;
                                 END;
         NMLVGETINFOTIPW      = tagNMLVGETINFOTIPW;
         LPNMLVGETINFOTIPW    = ^tagNMLVGETINFOTIPW;
         TNMLVGETINFOTIPW     = tagNMLVGETINFOTIPW;
         PNMLVGETINFOTIPW     = ^tagNMLVGETINFOTIPW;

{$ifdef ie3plus}
         LV_KEYDOWN                     = NMLVKEYDOWN;
{$ELSE}
         tagLVKEYDOWN                   = _LV_KEYDOWN;
         NMLVKEYDOWN                    = LV_KEYDOWN;
{$ENDIF}


// NMLVGETINFOTIPA.dwFlag values

CONST
         LVGIT_UNFOLDED                 = $0001;

         LVN_GETINFOTIPA                = (LVN_FIRST-57);
         LVN_GETINFOTIPW                = (LVN_FIRST-58);
	 LVN_INCREMENTALSEARCHA   	= (LVN_FIRST-62);
	 LVN_INCREMENTALSEARCHW   	= (LVN_FIRST-63);

	 LVNSCH_DEFAULT  		= -1;
	 LVNSCH_ERROR    		= -2;
	 LVNSCH_IGNORE   		= -3;

{$ifdef win32vista}
	 LVN_COLUMNDROPDOWN       	= (LVN_FIRST-64);
	 LVN_COLUMNOVERFLOWCLICK  	= (LVN_FIRST-66);
{$endif}

Const
{$IFDEF UNICODE}
         LVN_GETINFOTIP      = LVN_GETINFOTIPW;
type
         NMLVGETINFOTIP      = NMLVGETINFOTIPW;
         LPNMLVGETINFOTIP    = LPNMLVGETINFOTIPW;
{$ELSE}
         LVN_GETINFOTIP      = LVN_GETINFOTIPA;
type
         NMLVGETINFOTIP      = NMLVGETINFOTIPA;
         LPNMLVGETINFOTIP    = LPNMLVGETINFOTIPA;
{$ENDIF}
{$ENDIF}      // _WIN32_IE >= 0x0400


{$ifdef win32xp} // actually 2003
         tagNMLVSCROLL        = Record
                                 hdr          : NMHDR;
                                 dx           : cint;
                                 dy           : cint;
                                 END;
         NMLVSCROLL           = tagNMLVSCROLL;
         LPNMLVSCROLL         = ^tagNMLVSCROLL;
         TNMLVSCROLL          = tagNMLVSCROLL;
         PNMLVSCROLL          = ^tagNMLVSCROLL;


CONST
         LVN_BEGINSCROLL                = (LVN_FIRST-80);
         LVN_ENDSCROLL                  = (LVN_FIRST-81);
// {$ENDIF}

{$ifdef win32vista}
	 LVN_LINKCLICK           	= (LVN_FIRST-84);
	 LVN_GETEMPTYMARKUP      	= (LVN_FIRST-87);

	 EMF_CENTERED            	= $00000001;  // render markup centered in the listview area

Type
	 tagNMLVEMPTYMARKUP   = Record
          			  hdr : NMHDR;
				  // out params from client back to listview
    			          dwFlags :DWORD;                      // EMF_*
     				  szMarkup : array[0..L_MAX_URL_LENGTH-1] of wchar;   // markup displayed
				end;
         NMLVEMPTYMARKUP      = tagNMLVEMPTYMARKUP;
         TNMLVEMPTYMARKUP     = tagNMLVEMPTYMARKUP;
         PNMLVEMPTYMARKUP     = ^tagNMLVEMPTYMARKUP;

{$endif}

{$ENDIF} // NOLISTVIEW

//====== TREEVIEW CONTROL =====================================================

{$IFNDEF NOTREEVIEW}

CONST
{$IFDEF _WIN32}
         WC_TREEVIEWA                   = 'SysTreeView32';
         WC_TREEVIEWW                   = {L}'SysTreeView32';

{$IFDEF UNICODE}
         WC_TREEVIEW                    = WC_TREEVIEWW;
{$ELSE}
         WC_TREEVIEW                    = WC_TREEVIEWA;
{$ENDIF}
{$ELSE}
         WC_TREEVIEW                    = 'SysTreeView';
{$ENDIF}

// begin_r_commctrl

         TVS_HASBUTTONS                 = $0001;
         TVS_HASLINES                   = $0002;
         TVS_LINESATROOT                = $0004;
         TVS_EDITLABELS                 = $0008;
         TVS_DISABLEDRAGDROP            = $0010;
         TVS_SHOWSELALWAYS              = $0020;
{$ifdef ie3plus}
         TVS_RTLREADING                 = $0040;

         TVS_NOTOOLTIPS                 = $0080;
         TVS_CHECKBOXES                 = $0100;
         TVS_TRACKSELECT                = $0200;
{$ifdef ie4plus}
         TVS_SINGLEEXPAND               = $0400;
         TVS_INFOTIP                    = $0800;
         TVS_FULLROWSELECT              = $1000;
         TVS_NOSCROLL                   = $2000;
         TVS_NONEVENHEIGHT              = $4000;
{$ENDIF}
{$ifdef ie5plus}
         TVS_NOHSCROLL                  = $8000;              // TVS_NOSCROLL overrides this
{$ENDIF}
{$ifdef win32vista}
         TVS_EX_MULTISELECT             = $0002;
         TVS_EX_DOUBLEBUFFER            = $0004;
         TVS_EX_NOINDENTSTATE           = $0008;
         TVS_EX_RICHTOOLTIP             = $0010;
         TVS_EX_AUTOHSCROLL             = $0020;
         TVS_EX_FADEINOUTEXPANDOS       = $0040;
         TVS_EX_PARTIALCHECKBOXES       = $0080;
         TVS_EX_EXCLUSIONCHECKBOXES     = $0100;
         TVS_EX_DIMMEDCHECKBOXES        = $0200;
         TVS_EX_DRAWIMAGEASYNC          = $0400;
{$endif}
{$ENDIF}

// end_r_commctrl

TYPE
         HTREEITEM           = ^TREEITEM;

CONST
         TVIF_TEXT                      = $0001;
         TVIF_IMAGE                     = $0002;
         TVIF_PARAM                     = $0004;
         TVIF_STATE                     = $0008;
         TVIF_HANDLE                    = $0010;
         TVIF_SELECTEDIMAGE             = $0020;
         TVIF_CHILDREN                  = $0040;
{$ifdef ie4plus}
         TVIF_INTEGRAL                  = $0080;
{$ENDIF}
{$ifdef win32vista}
 	 TVIF_STATEEX            	= $0100;	
	 TVIF_EXPANDEDIMAGE      	= $0200;
{$endif}
         TVIS_SELECTED                  = $0002;
         TVIS_CUT                       = $0004;
         TVIS_DROPHILITED               = $0008;
         TVIS_BOLD                      = $0010;
         TVIS_EXPANDED                  = $0020;
         TVIS_EXPANDEDONCE              = $0040;
{$ifdef ie3plus}
         TVIS_EXPANDPARTIAL             = $0080;
{$ENDIF}

         TVIS_OVERLAYMASK               = $0F00;
         TVIS_STATEIMAGEMASK            = $F000;
         TVIS_USERMASK                  = $F000;

// IE6
	 TVIS_EX_FLAT            	= $0001;
{$ifdef win32vista}
	 TVIS_EX_DISABLED        	= $0002;
{$endif}
	 TVIS_EX_ALL             	= $0002;
Type

// Structure for TreeView's NM_TVSTATEIMAGECHANGING notification
         tagNMTVSTATEIMAGECHANGING = Record
                                      hdr 		    : NMHDR;
                                      hti                   : HTREEITEM;
                                      iOldStateImageIndex   : cint;
                                      iNewStateImageIndex   : cint;
                                      end;
	 NMTVSTATEIMAGECHANGING    = tagNMTVSTATEIMAGECHANGING;
	 LPNMTVSTATEIMAGECHANGING  = ^tagNMTVSTATEIMAGECHANGING;

Const
         I_CHILDRENCALLBACK             = (-1);
Type
         tagTVITEMA           = Record
                                 mask         : UINT;
                                 hItem        : HTREEITEM;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 iSelectedImage : cint;
                                 cChildren    : cint;
                                 lParam       : LPARAM;
{$ifdef ie6plus}
                                 uStateEx     : cUINT;
                                 hwnd         : HWND;
                                 iExpandedImage  : cint;
{$endif}
{$ifdef NTDDI_WIN7}
				 iPadding        : cint;
{$endif}
                                 END;
         TVITEMA              = tagTVITEMA;
         LPTVITEMA            = ^tagTVITEMA;
         TTVITEMA             = tagTVITEMA;
         PTVITEMA             = ^tagTVITEMA;


         tagTVITEMW           = Record
                                 mask         : UINT;
                                 hItem        : HTREEITEM;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 iSelectedImage : cint;
                                 cChildren    : cint;
                                 lParam       : LPARAM;
{$ifdef ie6plus}
                                 uStateEx     : cUINT;
                                 hwnd         : HWND;
                                 iExpandedImage  : cint;
{$endif}
{$ifdef NTDDI_WIN7}
				 iPadding        : cint;
{$endif}
                                 END;
         TVITEMW              = tagTVITEMW;
         LPTVITEMW            = ^tagTVITEMW;
         TTVITEMW             = tagTVITEMW;
         PTVITEMW             = ^tagTVITEMW;


{$ifdef ie4plus}
// only used for Get and Set messages.  no notifies
         tagTVITEMEXA         = Record
                                 mask         : UINT;
                                 hItem        : HTREEITEM;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 iSelectedImage : cint;
                                 cChildren    : cint;
                                 lParam       : LPARAM;
                                 iIntegral    : cint;
                                 END;
         TVITEMEXA            = tagTVITEMEXA;
         LPTVITEMEXA          = ^tagTVITEMEXA;
         TTVITEMEXA           = tagTVITEMEXA;
         PTVITEMEXA           = ^tagTVITEMEXA;

// only used for Get and Set messages.  no notifies
         tagTVITEMEXW         = Record
                                 mask         : UINT;
                                 hItem        : HTREEITEM;
                                 state        : UINT;
                                 stateMask    : UINT;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 iSelectedImage : cint;
                                 cChildren    : cint;
                                 lParam       : LPARAM;
                                 iIntegral    : cint;
                                 END;
         TVITEMEXW            = tagTVITEMEXW;
         LPTVITEMEXW          = ^tagTVITEMEXW;
         TTVITEMEXW           = tagTVITEMEXW;
         PTVITEMEXW           = ^tagTVITEMEXW;


TYPE
{$ifdef ie3plus}
         LPTV_ITEMW          = LPTVITEMW;
         LPTV_ITEMA          = LPTVITEMA;
         TV_ITEMW            = TVITEMW;
         TV_ITEMA            = TVITEMA;
{$ELSE}
         tagTVITEMA          = _TV_ITEMA;
         TVITEMA             = TV_ITEMA;
         LPTVITEMA           = LPTV_ITEMA;
         tagTVITEMW          = _TV_ITEMW;
         TVITEMW             = TV_ITEMW;
         LPTVITEMW           = LPTV_ITEMW;
{$ENDIF}
         TTVItem             = TVITEMA;
         PTVItem             = LPTVITEMA;





{$IFDEF UNICODE}
         TVITEMEX             = TVITEMEXW;
         LPTVITEMEX           = LPTVITEMEXW;
{$ELSE}
         TVITEMEX             = TVITEMEXA;
         LPTVITEMEX           = LPTVITEMEXA;
{$ENDIF} // UNICODE


{$ENDIF}

{$IFDEF UNICODE}
         TVITEM              = TVITEMW;
         LPTVITEM            = LPTVITEMW;
{$ELSE}
         TVITEM              = TVITEMA;
         LPTVITEM            = LPTVITEMA;
{$ENDIF}

         LPTV_ITEM           = LPTVITEM;
         TV_ITEM             = TVITEM;

CONST
         TVI_ROOT                       = HTREEITEM(ULONG_PTR(-$10000));
         TVI_FIRST                      = HTREEITEM(ULONG_PTR(-$0FFFF));
         TVI_LAST                       = HTREEITEM(ULONG_PTR(-$0FFFE));
         TVI_SORT                       = HTREEITEM(ULONG_PTR(-$0FFFD));



// #define TVINSERTSTRUCTA_V1_SIZE CCSIZEOF_STRUCT(TVINSERTSTRUCTA, item)
// #define TVINSERTSTRUCTW_V1_SIZE CCSIZEOF_STRUCT(TVINSERTSTRUCTW, item)
Type
         tagTVINSERTSTRUCTA   = Record
                                 hParent      : HTREEITEM;
                                 hInsertAfter : HTREEITEM;
{$ifdef ie4plus}
                                 case boolean of
                                     false:   (itemex       : TVITEMEXA);
                                     True:    (item         : TV_ITEMA);
{$ELSE}
                                 item         : TV_ITEMA;
{$ENDIF}
                                 END;
         TVINSERTSTRUCTA      = tagTVINSERTSTRUCTA;
         LPTVINSERTSTRUCTA    = ^tagTVINSERTSTRUCTA;
         TTVINSERTSTRUCTA     = tagTVINSERTSTRUCTA;
         PTVINSERTSTRUCTA     = ^tagTVINSERTSTRUCTA;
         TTVINSERTSTRUCT      = TTVINSERTSTRUCTA;
         PTVINSERTSTRUCT      = PTVINSERTSTRUCTA;


         tagTVINSERTSTRUCTW   = Record
                                 hParent      : HTREEITEM;
                                 hInsertAfter : HTREEITEM;
{$ifdef ie4plus}
                                 case boolean of
                                     false:   (itemex       : TVITEMEXW);
                                     True:    (item         : TV_ITEMW);
{$ELSE}
                                 item         : TV_ITEMW;
{$ENDIF}
                                 END;
         TVINSERTSTRUCTW      = tagTVINSERTSTRUCTW;
         LPTVINSERTSTRUCTW    = ^tagTVINSERTSTRUCTW;
         TTVINSERTSTRUCTW     = tagTVINSERTSTRUCTW;
         PTVINSERTSTRUCTW     = ^tagTVINSERTSTRUCTW;


{$IFDEF UNICODE}
         TVINSERTSTRUCT      = TVINSERTSTRUCTW;
         LPTVINSERTSTRUCT    = LPTVINSERTSTRUCTW;
//       TVINSERTSTRUCT_V1_SIZE         = TVINSERTSTRUCTW_V1_SIZE;
{$ELSE}
         TVINSERTSTRUCT      = TVINSERTSTRUCTA;
         LPTVINSERTSTRUCT    = LPTVINSERTSTRUCTA;
//       TVINSERTSTRUCT_V1_SIZE         = TVINSERTSTRUCTA_V1_SIZE;
{$ENDIF}
CONST
         TVM_INSERTITEMA                = (TV_FIRST + 0);
         TVM_INSERTITEMW                = (TV_FIRST + 50);
{$IFDEF UNICODE}
         TVM_INSERTITEM      = TVM_INSERTITEMW;
{$ELSE}
         TVM_INSERTITEM      = TVM_INSERTITEMA;
{$ENDIF}

TYPE
{$ifdef ie3plus}
         LPTV_INSERTSTRUCTA  = LPTVINSERTSTRUCTA;
         LPTV_INSERTSTRUCTW  = LPTVINSERTSTRUCTW;
         TV_INSERTSTRUCTA    = TVINSERTSTRUCTA;
         TV_INSERTSTRUCTW    = TVINSERTSTRUCTW;
{$ELSE}
         tagTVINSERTSTRUCTA  = _TV_INSERTSTRUCTA;
         TVINSERTSTRUCTA     = TV_INSERTSTRUCTA;
         LPTVINSERTSTRUCTA   = LPTV_INSERTSTRUCTA;
         tagTVINSERTSTRUCTW  = _TV_INSERTSTRUCTW;
         TVINSERTSTRUCTW     = TV_INSERTSTRUCTW;
         LPTVINSERTSTRUCTW   = LPTV_INSERTSTRUCTW;
{$ENDIF}


         TV_INSERTSTRUCT     = TVINSERTSTRUCT;
         LPTV_INSERTSTRUCT   = LPTVINSERTSTRUCT;


// Macro 156
Function TreeView_InsertItem( hwnd : hwnd; lpis : LPTV_INSERTSTRUCT):HTREEITEM;inline;
Function TreeView_InsertItem( hwnd : hwnd; const lpis : TV_INSERTSTRUCT):HTREEITEM;inline;

CONST
         TVM_DELETEITEM                 = (TV_FIRST + 1);

// Macro 157
Function TreeView_DeleteItem( hwnd : hwnd; hitem : HTREEITEM):BOOL;

// Macro 158
Function TreeView_DeleteAllItems( hwnd : hwnd):BOOL;

CONST
         TVM_EXPAND                     = (TV_FIRST + 2);

// Macro 159
Function TreeView_Expand( hwnd : hwnd; hitem : HTREEITEM; code : WPARAM):BOOL;

CONST
         TVE_COLLAPSE                   = $0001;
         TVE_EXPAND                     = $0002;
         TVE_TOGGLE                     = $0003;
{$ifdef ie3plus}
         TVE_EXPANDPARTIAL              = $4000;
{$ENDIF}
         TVE_COLLAPSERESET              = $8000;


         TVM_GETITEMRECT                = (TV_FIRST + 4);

// Macro 160
Function TreeView_GetItemRect( hwnd : hwnd; hitem: HTREEITEM; code : WPARAM; prc : pRECT):BOOL;inline;
Function TreeView_GetItemRect( hwnd : hwnd; hitem: HTREEITEM; var prc : TRECT;code : Bool):BOOL;inline;

CONST
         TVM_GETCOUNT                   = (TV_FIRST + 5);

// Macro 161
Function TreeView_GetCount( hwnd : hwnd):UINT;

CONST
         TVM_GETINDENT                  = (TV_FIRST + 6);

// Macro 162
Function TreeView_GetIndent( hwnd : hwnd):UINT;

CONST
         TVM_SETINDENT                  = (TV_FIRST + 7);

// Macro 163
Function TreeView_SetIndent( hwnd : hwnd; indent : WPARAM):BOOL;

CONST
         TVM_GETIMAGELIST               = (TV_FIRST + 8);

// Macro 164
Function TreeView_GetImageList( hwnd : hwnd; iImage : cint ):HIMAGELIST;

CONST
         TVSIL_NORMAL                   = 0;
         TVSIL_STATE                    = 2;


         TVM_SETIMAGELIST               = (TV_FIRST + 9);

// Macro 165
Function TreeView_SetImageList( hwnd : hwnd; himl : HIMAGELIST; iImage : cint):HIMAGELIST;

CONST
         TVM_GETNEXTITEM                = (TV_FIRST + 10);

// Macro 166
Function TreeView_GetNextItem( hwnd : hwnd; hitem : HTREEITEM; code : WPARAM):HTREEITEM;

CONST
         TVGN_ROOT                      = $0000;
         TVGN_NEXT                      = $0001;
         TVGN_PREVIOUS                  = $0002;
         TVGN_PARENT                    = $0003;
         TVGN_CHILD                     = $0004;
         TVGN_FIRSTVISIBLE              = $0005;
         TVGN_NEXTVISIBLE               = $0006;
         TVGN_PREVIOUSVISIBLE           = $0007;
         TVGN_DROPHILITE                = $0008;
         TVGN_CARET                     = $0009;
{$ifdef ie4plus}
         TVGN_LASTVISIBLE               = $000A;
{$ENDIF}      // _WIN32_IE >= 0x0400
{$ifdef ie6plus}
	 TVGN_NEXTSELECTED       	= $000B;
{$endif}
{$ifdef win32xp}  // 0x501
         TVSI_NOSINGLEEXPAND            = $8000;              // Should not conflict with TVGN flags.
{$ENDIF}

function TreeView_GetChild(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
function TreeView_GetNextSibling(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
function TreeView_GetPrevSibling(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
function TreeView_GetParent(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
function TreeView_GetFirstVisible(hwnd:hwnd) : HTREEITEM;inline;
function TreeView_GetNextVisible(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
function TreeView_GetPrevVisible(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
function TreeView_GetSelection(hwnd:hwnd) : HTREEITEM;inline;
function TreeView_GetDropHilight(hwnd:hwnd) : HTREEITEM;inline;
function TreeView_GetDropHilite(hwnd:hwnd) : HTREEITEM;inline;
function TreeView_GetRoot(hwnd:hwnd) : HTREEITEM;inline;
function TreeView_GetLastVisible(hwnd:hwnd) : HTREEITEM;inline;

{$ifdef win32vista}
function  TreeView_GetNextSelected(hwnd:hwnd; hitem:HTREEITEM):HTREEITEM;inline;   
{$endif}

CONST
         TVM_SELECTITEM                 = (TV_FIRST + 11);

// Macro 178
Function TreeView_Select( hwnd : hwnd; hitem : HTREEITEM; code : WPARAM):BOOL;

// Macro 179
Procedure TreeView_SelectItem(hwnd:hwnd; hitem:HTREEITEM);

// Macro 180
Procedure TreeView_SelectDropTarget(hwnd:hwnd; hitem:HTREEITEM);

// Macro 181
Procedure TreeView_SelectSetFirstVisible(hwnd:hwnd; hitem:HTREEITEM);

CONST
         TVM_GETITEMA                   = (TV_FIRST + 12);
         TVM_GETITEMW                   = (TV_FIRST + 62);

{$IFDEF UNICODE}

TYPE
         TVM_GETITEM         = TVM_GETITEMW;
{$ELSE}
         TVM_GETITEM         = TVM_GETITEMA;
{$ENDIF}

// Macro 182


Function TreeView_GetItem( hwnd : hwnd;var  pitem : TV_ITEM ):BOOL;




CONST
         TVM_SETITEMA                   = (TV_FIRST + 13);
         TVM_SETITEMW                   = (TV_FIRST + 63);

{$IFDEF UNICODE}

TYPE
         TVM_SETITEM         = TVM_SETITEMW;
{$ELSE}
         TVM_SETITEM         = TVM_SETITEMA;
{$ENDIF}

// Macro 183


Function TreeView_SetItem( hwnd : hwnd;const  pitem : TV_ITEM ):BOOL;


CONST
         TVM_EDITLABELA                 = (TV_FIRST + 14);
         TVM_EDITLABELW                 = (TV_FIRST + 65);
{$IFDEF UNICODE}

TYPE
         TVM_EDITLABEL       = TVM_EDITLABELW;
{$ELSE}
         TVM_EDITLABEL       = TVM_EDITLABELA;
{$ENDIF}

// Macro 184


Function TreeView_EditLabel( hwnd : hwnd; hitem : HTREEITEM):HWND;




CONST
         TVM_GETEDITCONTROL             = (TV_FIRST + 15);
// Macro 185


Function TreeView_GetEditControl( hwnd : hwnd):HWND;




CONST
         TVM_GETVISIBLECOUNT            = (TV_FIRST + 16);
// Macro 186


Function TreeView_GetVisibleCount( hwnd : hwnd):UINT;




CONST
         TVM_HITTEST                    = (TV_FIRST + 17);

TYPE

         tagTVHITTESTINFO     = Record
                                 pt           : POINT;
                                 flags        : UINT;
                                 hItem        : HTREEITEM;
                                 END;
         TVHITTESTINFO        = tagTVHITTESTINFO;
         LPTVHITTESTINFO      = ^tagTVHITTESTINFO;
         TTVHITTESTINFO       = tagTVHITTESTINFO;
         PTVHITTESTINFO       = ^tagTVHITTESTINFO;


{$ifdef ie3plus}
         LPTV_HITTESTINFO    = LPTVHITTESTINFO;
         TV_HITTESTINFO                 = TVHITTESTINFO;
{$ELSE}
         tagTVHITTESTINFO               = _TV_HITTESTINFO;
         TVHITTESTINFO                  = TV_HITTESTINFO;

         LPTVHITTESTINFO     = LPTV_HITTESTINFO;
{$ENDIF}


CONST
         TVHT_NOWHERE                   = $0001;
         TVHT_ONITEMICON                = $0002;
         TVHT_ONITEMLABEL               = $0004;
         TVHT_ONITEM                    = (TVHT_ONITEMICON  OR  TVHT_ONITEMLABEL  OR  TVHT_ONITEMSTATEICON);
         TVHT_ONITEMINDENT              = $0008;
         TVHT_ONITEMBUTTON              = $0010;
         TVHT_ONITEMRIGHT               = $0020;
         TVHT_ONITEMSTATEICON           = $0040;

         TVHT_ABOVE                     = $0100;
         TVHT_BELOW                     = $0200;
         TVHT_TORIGHT                   = $0400;
         TVHT_TOLEFT                    = $0800;


         TVM_CREATEDRAGIMAGE            = (TV_FIRST + 18);

// Macro 187
Function TreeView_HitTest( hwnd : hwnd; lpht : LPTV_HITTESTINFO):HTREEITEM;inline;
Function TreeView_HitTest( hwnd : hwnd; var lpht : TV_HITTESTINFO):HTREEITEM;inline;


// Macro 188


Function TreeView_CreateDragImage( hwnd : hwnd; hitem : HTREEITEM):HIMAGELIST;




CONST
         TVM_SORTCHILDREN               = (TV_FIRST + 19);
// Macro 189


Function TreeView_SortChildren( hwnd : hwnd; hitem : HTREEITEM; recurse : WPARAM):BOOL;




CONST
         TVM_ENSUREVISIBLE              = (TV_FIRST + 20);
// Macro 190


Function TreeView_EnsureVisible( hwnd : hwnd; hitem : HTREEITEM):BOOL;





CONST
         TVM_ENDEDITLABELNOW            = (TV_FIRST + 22);
// Macro 192


Function TreeView_EndEditLabelNow( hwnd : hwnd; fCancel : WPARAM):BOOL;inline;
Function TreeView_EndEditLabelNow( hwnd : hwnd; fCancel : Bool):BOOL;inline;




CONST
         TVM_GETISEARCHSTRINGA          = (TV_FIRST + 23);
         TVM_GETISEARCHSTRINGW          = (TV_FIRST + 64);

{$IFDEF UNICODE}

TYPE
         TVM_GETISEARCHSTRING= TVM_GETISEARCHSTRINGW;
{$ELSE}
         TVM_GETISEARCHSTRING= TVM_GETISEARCHSTRINGA;
{$ENDIF}

{$ifdef ie3plus}

CONST
         TVM_SETTOOLTIPS                = (TV_FIRST + 24);
// Macro 193


Function TreeView_SetToolTips( hwnd : hwnd; hwndTT : WPARAM):HWND;


CONST
         TVM_GETTOOLTIPS                = (TV_FIRST + 25);
// Macro 194


Function TreeView_GetToolTips( hwnd : hwnd):HWND;

{$ENDIF}

// Macro 195


Function TreeView_GetISearchString( hwndTV : hwnd; lpsz : LPTSTR):BOOL;


{$ifdef ie4plus}

CONST
         TVM_SETINSERTMARK              = (TV_FIRST + 26);
// Macro 196


Function TreeView_SetInsertMark( hwnd : hwnd; hItem : LPARAM ; fAfter : WPARAM ):BOOL;



CONST
         TVM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;
// Macro 197


Function TreeView_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;



CONST
         TVM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;
// Macro 198


Function TreeView_GetUnicodeFormat( hwnd : hwnd):BOOL;


{$ENDIF}

{$ifdef ie4plus}

CONST
         TVM_SETITEMHEIGHT              = (TV_FIRST + 27);
// Macro 199


Function TreeView_SetItemHeight( hwnd : hwnd; iHeight : WPARAM):cint;


CONST
         TVM_GETITEMHEIGHT              = (TV_FIRST + 28);
// Macro 200


Function TreeView_GetItemHeight( hwnd : hwnd):cint;



CONST
         TVM_SETBKCOLOR                 = (TV_FIRST + 29);
// Macro 201


Function TreeView_SetBkColor( hwnd : hwnd; clr : LPARAM):COLORREF;



CONST
         TVM_SETTEXTCOLOR               = (TV_FIRST + 30);
// Macro 202


Function TreeView_SetTextColor( hwnd : hwnd; clr : LPARAM):COLORREF;



CONST
         TVM_GETBKCOLOR                 = (TV_FIRST + 31);

// Macro 203
Function TreeView_GetBkColor( hwnd : hwnd):COLORREF;


CONST
         TVM_GETTEXTCOLOR               = (TV_FIRST + 32);

// Macro 204
Function TreeView_GetTextColor( hwnd : hwnd):COLORREF;



CONST
         TVM_SETSCROLLTIME              = (TV_FIRST + 33);

// Macro 205
Function TreeView_SetScrollTime( hwnd : hwnd; uTime :wparam ):UINT;



CONST
         TVM_GETSCROLLTIME              = (TV_FIRST + 34);
// Macro 206


Function TreeView_GetScrollTime( hwnd : hwnd):UINT;




CONST
         TVM_SETINSERTMARKCOLOR         = (TV_FIRST + 37);
// Macro 207


Function TreeView_SetInsertMarkColor( hwnd : hwnd; clr : LPARAM):COLORREF;


CONST
         TVM_GETINSERTMARKCOLOR         = (TV_FIRST + 38);
// Macro 208


Function TreeView_GetInsertMarkColor( hwnd : hwnd):COLORREF;


{$ENDIF}  // (_WIN32_IE >= 0x0400)

{$ifdef ie5plus}
// tvm_?etitemstate only uses mask, state and stateMask.
// so unicode or ansi is irrelevant.
// Macro 209

Procedure TreeView_SetItemState(hwndTV:HWND;hti:HTreeItem;data:UINT;_mask:UINT);


// Macro 210

Procedure TreeView_SetCheckState( hwndTV : hwnd; hti : HTreeItem ; fCheck : bool );


CONST
         TVM_GETITEMSTATE               = (TV_FIRST + 39);

// Macro 211
Function TreeView_GetItemState( hwndTV : hwnd; hti : HTreeItem; statemask : UINT):UINT;


// Macro 212
Function TreeView_GetCheckState( hwndTV : hwnd; hti : HTreeItem):UINT;


CONST
         TVM_SETLINECOLOR               = (TV_FIRST + 40);
// Macro 213
Function TreeView_SetLineColor( hwnd : hwnd; clr : LPARAM):COLORREF;

CONST
         TVM_GETLINECOLOR               = (TV_FIRST + 41);

// Macro 214
Function TreeView_GetLineColor( hwnd : hwnd):COLORREF;


{$ENDIF}

{$ifdef Win32XP}

CONST
         TVM_MAPACCIDTOHTREEITEM        = (TV_FIRST + 42);

// Macro 215
Function TreeView_MapAccIDToHTREEITEM( hwnd : hwnd; id : UINT ):HTREEITEM;



CONST
         TVM_MAPHTREEITEMTOACCID        = (TV_FIRST + 43);

// Macro 216
Function TreeView_MapHTREEITEMToAccID( hwnd : hwnd; htreeitem : WPARAM):UINT;

{$ENDIF}

{$ifdef win32vista}
CONST 
         TVM_GETSELECTEDCOUNT       	= (TV_FIRST + 70);
	 TVM_SHOWINFOTIP            	= (TV_FIRST + 71);
	 TVM_GETITEMPARTRECT            = (TV_FIRST + 72);

Type
	 TVITEMPART 	= (TVGIPR_BUTTON    = $0001);
         pTVITEMPART	= ^TVITEMPART;

	 tagTVGETITEMPARTRECTINFO 	= Record
                                            hti :HTREEITEM ;
                                            prc :PRECT;
                                            partID :TVITEMPART;
					  end;
 	 TVGETITEMPARTRECTINFO 		= tagTVGETITEMPARTRECTINFO;

function  TreeView_GetSelectedCount(hwnd:hwnd):DWORD;
//    (DWORD)SNDMSG((hwnd), TVM_GETSELECTEDCOUNT, 0, 0)

function  TreeView_ShowInfoTip(hwnd:HWND; hitem:HTREEITEM):DWORD;
//    (DWORD)SNDMSG((hwnd), TVM_SHOWINFOTIP, 0, (LPARAM)(hitem))

function  TreeView_GetItemPartRect(hwnd:HWND; hitem:HTREEITEM; prc:prect; partid:TVITEMPART):bool;
//{ TVGETITEMPARTRECTINFO info; \
//  info.hti = (hitem); \
//  info.prc = (prc); \
//  info.partID = (partid); \
//  (BOOL)SNDMSG((hwnd), TVM_GETITEMPARTRECT, 0, (LPARAM)&info); \

{$endif}


TYPE
         PFNTVCOMPARE =function (lparam1:LPARAM;lparam2:LPARAM;lParamSort:LParam): cint; STDCALL;
         TTVCompare = PFNTVCOMPARE;


         tagTVSORTCB          = Record
                                 hParent      : HTREEITEM;
                                 lpfnCompare  : PFNTVCOMPARE;
                                 lParam       : LPARAM;
                                 END;
         TVSORTCB             = tagTVSORTCB;
         LPTVSORTCB           = ^tagTVSORTCB;
         TTVSORTCB            = tagTVSORTCB;
         PTVSORTCB            = ^tagTVSORTCB;

{$ifdef ie3plus}
         LPTV_SORTCB         = LPTVSORTCB;


         TV_SORTCB                      = TVSORTCB;
{$ELSE}
         tagTVSORTCB                    = _TV_SORTCB;
         TVSORTCB                       = TV_SORTCB;


         LPTVSORTCB          = LPTV_SORTCB;
{$ENDIF}


         tagNMTREEVIEWA       = Record
                                 hdr          : NMHDR;
                                 action       : UINT;
                                 itemOld      : TVITEMA;
                                 itemNew      : TVITEMA;
                                 ptDrag       : POINT;
                                 END;
         NMTREEVIEWA          = tagNMTREEVIEWA;
         LPNMTREEVIEWA        = ^tagNMTREEVIEWA;
         TNMTREEVIEWA         = tagNMTREEVIEWA;
         PNMTREEVIEWA         = ^tagNMTREEVIEWA;
         PNMTreeView          = PNMTreeViewA;
         TNMTreeView          = TNMTreeViewA;



         tagNMTREEVIEWW       = Record
                                 hdr          : NMHDR;
                                 action       : UINT;
                                 itemOld      : TVITEMW;
                                 itemNew      : TVITEMW;
                                 ptDrag       : POINT;
                                 END;
         NMTREEVIEWW          = tagNMTREEVIEWW;
         LPNMTREEVIEWW        = ^tagNMTREEVIEWW;
         TNMTREEVIEWW         = tagNMTREEVIEWW;
         PNMTREEVIEWW         = ^tagNMTREEVIEWW;



{$IFDEF UNICODE}
         NMTREEVIEW          = NMTREEVIEWW;
         LPNMTREEVIEW        = LPNMTREEVIEWW;
{$ELSE}
         NMTREEVIEW          = NMTREEVIEWA;
         LPNMTREEVIEW        = LPNMTREEVIEWA;
{$ENDIF}

{$ifdef ie3plus}
         LPNM_TREEVIEWA      = LPNMTREEVIEWA;
         LPNM_TREEVIEWW      = LPNMTREEVIEWW;
         NM_TREEVIEWW        = NMTREEVIEWW;
         NM_TREEVIEWA        = NMTREEVIEWA;
{$ELSE}
         tagNMTREEVIEWA      = _NM_TREEVIEWA;
         tagNMTREEVIEWW      = _NM_TREEVIEWW;
         NMTREEVIEWA         = NM_TREEVIEWA;
         NMTREEVIEWW         = NM_TREEVIEWW;
         LPNMTREEVIEWA       = LPNM_TREEVIEWA;
         LPNMTREEVIEWW       = LPNM_TREEVIEWW;
{$ENDIF}

         LPNM_TREEVIEW       = LPNMTREEVIEW;
         NM_TREEVIEW         = NMTREEVIEW;



CONST
         TVN_SELCHANGINGA               = (TVN_FIRST-1);
         TVN_SELCHANGINGW               = (TVN_FIRST-50);
         TVN_SELCHANGEDA                = (TVN_FIRST-2);
         TVN_SELCHANGEDW                = (TVN_FIRST-51);

         TVC_UNKNOWN                    = $0000;
         TVC_BYMOUSE                    = $0001;
         TVC_BYKEYBOARD                 = $0002;

         TVN_GETDISPINFOA               = (TVN_FIRST-3);
         TVN_GETDISPINFOW               = (TVN_FIRST-52);
         TVN_SETDISPINFOA               = (TVN_FIRST-4);
         TVN_SETDISPINFOW               = (TVN_FIRST-53);

         TVIF_DI_SETITEM                = $1000;

Type
         tagTVDISPINFOA       = Record
                                 hdr          : NMHDR;
                                 item         : TVITEMA;
                                 END;
         NMTVDISPINFOA        = tagTVDISPINFOA;
         LPNMTVDISPINFOA      = ^tagTVDISPINFOA;
         TTVDISPINFOA         = tagTVDISPINFOA;
         PTVDISPINFOA         = ^tagTVDISPINFOA;
         TTVDispInfo          = TTVDISPINFOA;
         PTVDispInfo          = PTVDISPINFOA;


         tagTVDISPINFOW       = Record
                                 hdr          : NMHDR;
                                 item         : TVITEMW;
                                 END;
         NMTVDISPINFOW        = tagTVDISPINFOW;
         LPNMTVDISPINFOW      = ^tagTVDISPINFOW;
         TTVDISPINFOW         = tagTVDISPINFOW;
         PTVDISPINFOW         = ^tagTVDISPINFOW;

{$IFDEF UNICODE}
         NMTVDISPINFO        = NMTVDISPINFOW;
         LPNMTVDISPINFO      = LPNMTVDISPINFOW;
{$ELSE}
         NMTVDISPINFO        = NMTVDISPINFOA;
         LPNMTVDISPINFO      = LPNMTVDISPINFOA;
{$ENDIF}

{$ifdef IE6plus}
 	 tagTVDISPINFOEXA    = Record
				hdr  : NMHDR;
				item :TVITEMEXA;
				end;
	 NMTVDISPINFOEXA     = tagTVDISPINFOEXA;
	 LPNMTVDISPINFOEXA   = ^tagTVDISPINFOEXA;

 	 tagTVDISPINFOEXW    = Record
				hdr  : NMHDR;
				item :TVITEMEXW;
				end;
	 NMTVDISPINFOEXW     = tagTVDISPINFOEXW;
	 LPNMTVDISPINFOEXW   = ^tagTVDISPINFOEXW;

{$IFDEF UNICODE}
         NMTVDISPINFOEX        = NMTVDISPINFOEXW;
         LPNMTVDISPINFOEX      = LPNMTVDISPINFOEXW;
{$ELSE}
         NMTVDISPINFOEX        = NMTVDISPINFOEXA;
         LPNMTVDISPINFOEX      = LPNMTVDISPINFOEXA;
{$ENDIF}

	 TV_DISPINFOEXA          = NMTVDISPINFOEXA;
	 TV_DISPINFOEXW          = NMTVDISPINFOEXW;
	 TV_DISPINFOEX           = NMTVDISPINFOEX;


{$endif}

{$ifdef ie3plus}
          TV_DISPINFOA        = NMTVDISPINFOA;
         TV_DISPINFOW        = NMTVDISPINFOW;
{$ELSE}
         tagTVDISPINFOA      = _TV_DISPINFOA;
         NMTVDISPINFOA       = TV_DISPINFOA;
         tagTVDISPINFOW      = _TV_DISPINFOW;
         NMTVDISPINFOW       = TV_DISPINFOW;
{$ENDIF}
         TV_DISPINFO                    = NMTVDISPINFO;


CONST
         TVM_SORTCHILDRENCB             = (TV_FIRST + 21);

// Macro 191
Function TreeView_SortChildrenCB( hwnd : hwnd;psort :lpTV_sortcb; recurse : WPARAM):BOOL;inline;
Function TreeView_SortChildrenCB( hwnd : hwnd;const psort :tagTVsortcb; recurse : WPARAM):BOOL;inline;

CONST
         TVN_ITEMEXPANDINGA             = (TVN_FIRST-5);
         TVN_ITEMEXPANDINGW             = (TVN_FIRST-54);
         TVN_ITEMEXPANDEDA              = (TVN_FIRST-6);
         TVN_ITEMEXPANDEDW              = (TVN_FIRST-55);
         TVN_BEGINDRAGA                 = (TVN_FIRST-7);
         TVN_BEGINDRAGW                 = (TVN_FIRST-56);
         TVN_BEGINRDRAGA                = (TVN_FIRST-8);
         TVN_BEGINRDRAGW                = (TVN_FIRST-57);
         TVN_DELETEITEMA                = (TVN_FIRST-9);
         TVN_DELETEITEMW                = (TVN_FIRST-58);
         TVN_BEGINLABELEDITA            = (TVN_FIRST-10);
         TVN_BEGINLABELEDITW            = (TVN_FIRST-59);
         TVN_ENDLABELEDITA              = (TVN_FIRST-11);
         TVN_ENDLABELEDITW              = (TVN_FIRST-60);
         TVN_KEYDOWN                    = (TVN_FIRST-12);

{$ifdef ie4plus}
         TVN_GETINFOTIPA                = (TVN_FIRST-13);
         TVN_GETINFOTIPW                = (TVN_FIRST-14);
         TVN_SINGLEEXPAND               = (TVN_FIRST-15);

         TVNRET_DEFAULT                 = 0;
         TVNRET_SKIPOLD                 = 1;
         TVNRET_SKIPNEW                 = 2;

{$ifdef win32vista}
	 TVN_ITEMCHANGINGA       	= (TVN_FIRST-16);
	 TVN_ITEMCHANGINGW       	= (TVN_FIRST-17);
	 TVN_ITEMCHANGEDA        	= (TVN_FIRST-18);
	 TVN_ITEMCHANGEDW        	= (TVN_FIRST-19);
	 TVN_ASYNCDRAW           	= (TVN_FIRST-20);
{$endif}

{$ENDIF} // 0x400



{$IFDEF _WIN32}
// #include <pshpack1.h>
{$ENDIF}

TYPE

         tagTVKEYDOWN         = Record
                                 hdr          : NMHDR;
                                 wVKey        : WORD;
                                 flags        : UINT;
                                 END;
         NMTVKEYDOWN          = tagTVKEYDOWN;
         LPNMTVKEYDOWN        = ^tagTVKEYDOWN;
         TTVKEYDOWN           = tagTVKEYDOWN;
         PTVKEYDOWN           = ^tagTVKEYDOWN;

{$ifdef ie3plus}
         TV_KEYDOWN                     = NMTVKEYDOWN;
{$ELSE}
         tagTVKEYDOWN                   = _TV_KEYDOWN;
         NMTVKEYDOWN                    = TV_KEYDOWN;
{$ENDIF}


{$IFDEF _WIN32}
// #include <poppack.h>
{$ENDIF}

Const

{$IFDEF UNICODE}
         TVN_SELCHANGING     = TVN_SELCHANGINGW;
         TVN_SELCHANGED      = TVN_SELCHANGEDW;
         TVN_GETDISPINFO     = TVN_GETDISPINFOW;
         TVN_SETDISPINFO     = TVN_SETDISPINFOW;
         TVN_ITEMEXPANDING   = TVN_ITEMEXPANDINGW;
         TVN_ITEMEXPANDED    = TVN_ITEMEXPANDEDW;
         TVN_BEGINDRAG       = TVN_BEGINDRAGW;
         TVN_BEGINRDRAG      = TVN_BEGINRDRAGW;
         TVN_DELETEITEM      = TVN_DELETEITEMW;
         TVN_BEGINLABELEDIT  = TVN_BEGINLABELEDITW;
         TVN_ENDLABELEDIT    = TVN_ENDLABELEDITW;
{$ELSE}
         TVN_SELCHANGING     = TVN_SELCHANGINGA;
         TVN_SELCHANGED      = TVN_SELCHANGEDA;
         TVN_GETDISPINFO     = TVN_GETDISPINFOA;
         TVN_SETDISPINFO     = TVN_SETDISPINFOA;
         TVN_ITEMEXPANDING   = TVN_ITEMEXPANDINGA;
         TVN_ITEMEXPANDED    = TVN_ITEMEXPANDEDA;
         TVN_BEGINDRAG       = TVN_BEGINDRAGA;
         TVN_BEGINRDRAG      = TVN_BEGINRDRAGA;
         TVN_DELETEITEM      = TVN_DELETEITEMA;
         TVN_BEGINLABELEDIT  = TVN_BEGINLABELEDITA;
         TVN_ENDLABELEDIT    = TVN_ENDLABELEDITA;
{$ENDIF}


{$ifdef ie3plus}
// #define NMTVCUSTOMDRAW_V3_SIZE CCSIZEOF_STRUCT(NMTVCUSTOMDRAW, clrTextBk)
Type
         tagNMTVCUSTOMDRAW    = Record
                                 nmcd         : NMCUSTOMDRAW;
                                 clrText      : COLORREF;
                                 clrTextBk    : COLORREF;
{$ifdef ie4plus}
                                 iLevel       : cint;
{$ENDIF}
                                 END;
         NMTVCUSTOMDRAW       = tagNMTVCUSTOMDRAW;
         LPNMTVCUSTOMDRAW     = ^tagNMTVCUSTOMDRAW;
         TNMTVCUSTOMDRAW      = tagNMTVCUSTOMDRAW;
         PNMTVCUSTOMDRAW      = ^tagNMTVCUSTOMDRAW;

{$ENDIF}


{$ifdef ie4plus}

// for tooltips

         tagNMTVGETINFOTIPA   = Record
                                 hdr          : NMHDR;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 hItem        : HTREEITEM;
                                 lParam       : LPARAM;
                                 END;
         NMTVGETINFOTIPA      = tagNMTVGETINFOTIPA;
         LPNMTVGETINFOTIPA    = ^tagNMTVGETINFOTIPA;
         TNMTVGETINFOTIPA     = tagNMTVGETINFOTIPA;
         PNMTVGETINFOTIPA     = ^tagNMTVGETINFOTIPA;


         tagNMTVGETINFOTIPW   = Record
                                 hdr          : NMHDR;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 hItem        : HTREEITEM;
                                 lParam       : LPARAM;
                                 END;
         NMTVGETINFOTIPW      = tagNMTVGETINFOTIPW;
         LPNMTVGETINFOTIPW    = ^tagNMTVGETINFOTIPW;
         TNMTVGETINFOTIPW     = tagNMTVGETINFOTIPW;
         PNMTVGETINFOTIPW     = ^tagNMTVGETINFOTIPW;


CONST
{$IFDEF UNICODE}
         TVN_GETINFOTIP      = TVN_GETINFOTIPW;
TYPE
         NMTVGETINFOTIP      = NMTVGETINFOTIPW;
         LPNMTVGETINFOTIP    = LPNMTVGETINFOTIPW;
{$ELSE}
         TVN_GETINFOTIP      = TVN_GETINFOTIPA;
TYPE
         NMTVGETINFOTIP      = NMTVGETINFOTIPA;
         LPNMTVGETINFOTIP    = LPNMTVGETINFOTIPA;
{$ENDIF}

// treeview's customdraw return meaning don't draw images.  valid on CDRF_NOTIFYITEMPREPAINT

CONST
         TVCDRF_NOIMAGES                = $00010000;

{$ENDIF}      // _WIN32_IE >= 0x0400

{$ifdef ie6plus}
Type
     tagTVITEMCHANGE = packed record
          hdr : NMHDR;
          uChanged : UINT;
          hItem : HTREEITEM;
          uStateNew : UINT;
          uStateOld : UINT;
          lParam : LPARAM;
       end;
     NMTVITEMCHANGE = tagTVITEMCHANGE;
     PNMTVITEMCHANGE = ^NMTVITEMCHANGE;

     tagNMTVASYNCDRAW = packed record
          hdr : NMHDR; 
          pimldp : PIMAGELISTDRAWPARAMS;   { the draw that failed }
          hr : HRESULT;                    { why it failed }
          hItem : HTREEITEM;               { item that failed to draw icon }
          lParam : LPARAM;                 { its data }
          dwRetFlags : DWORD;              { Out Params }
          iRetImageIndex : longint;        { What listview should do on return }
       end;                                { used if ADRF_DRAWIMAGE is returned }
     NMTVASYNCDRAW = tagNMTVASYNCDRAW;
     PNMTVASYNCDRAW = ^NMTVASYNCDRAW;

CONST
{$IFDEF UNICODE}
         TVN_ITEMCHANGING      = TVN_ITEMCHANGINGW;
         TVN_ITEMCHANGED       = TVN_ITEMCHANGEDW;
{$ELSE}
         TVN_ITEMCHANGING      = TVN_ITEMCHANGINGA;
         TVN_ITEMCHANGED       = TVN_ITEMCHANGEDA;
{$ENDIF}
{$endif}

{$ENDIF}      // NOTREEVIEW

{$ifdef ie3plus}

{$IFNDEF NOUSEREXCONTROLS}

////////////////////  ComboBoxEx ////////////////////////////////

         WC_COMBOBOXEXW                 = {L}'ComboBoxEx32';
         WC_COMBOBOXEXA                 = 'ComboBoxEx32';


{$IFDEF UNICODE}
         WC_COMBOBOXEX       = WC_COMBOBOXEXW;
{$ELSE}
         WC_COMBOBOXEX       = WC_COMBOBOXEXA;
{$ENDIF}

CONST
         CBEIF_TEXT                     = $00000001;
         CBEIF_IMAGE                    = $00000002;
         CBEIF_SELECTEDIMAGE            = $00000004;
         CBEIF_OVERLAY                  = $00000008;
         CBEIF_INDENT                   = $00000010;
         CBEIF_LPARAM                   = $00000020;
         CBEIF_DI_SETITEM               = $10000000;

TYPE

         tagCOMBOBOXEXITEMA   = Record
                                 mask         : UINT;
                                 iItem        : INT_PTR;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 iSelectedImage : cint;
                                 iOverlay     : cint;
                                 iIndent      : cint;
                                 lParam       : LPARAM;
                                 END;
         COMBOBOXEXITEMA      = tagCOMBOBOXEXITEMA;
         PCOMBOBOXEXITEMA     = ^tagCOMBOBOXEXITEMA;
         TCOMBOBOXEXITEMA     = tagCOMBOBOXEXITEMA;
//         PCOMBOBOXEXITEMA     = ^tagCOMBOBOXEXITEMA;

         PCCOMBOBOXEXITEMA = ^COMBOBOXEXITEMA;


         tagCOMBOBOXEXITEMW   = Record
                                 mask         : UINT;
                                 iItem        : INT_PTR;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 iSelectedImage : cint;
                                 iOverlay     : cint;
                                 iIndent      : cint;
                                 lParam       : LPARAM;
                                 END;
         COMBOBOXEXITEMW      = tagCOMBOBOXEXITEMW;
         PCOMBOBOXEXITEMW     = ^tagCOMBOBOXEXITEMW;
         TCOMBOBOXEXITEMW     = tagCOMBOBOXEXITEMW;
//         PCOMBOBOXEXITEMW     = ^tagCOMBOBOXEXITEMW;

         PCCOMBOBOXEXITEMW        = ^COMBOBOXEXITEMW;

{$IFDEF UNICODE}
         COMBOBOXEXITEM      = COMBOBOXEXITEMW;
         PCOMBOBOXEXITEM     = PCOMBOBOXEXITEMW;
         PCCOMBOBOXEXITEM    = PCCOMBOBOXEXITEMW;
{$ELSE}
         COMBOBOXEXITEM      = COMBOBOXEXITEMA;
         PCOMBOBOXEXITEM     = PCOMBOBOXEXITEMA;
         PCCOMBOBOXEXITEM    = PCCOMBOBOXEXITEMA;
{$ENDIF}
         TComboBoxExItem     = COMBOBOXEXITEM;


CONST
         CBEM_INSERTITEMA               = (WM_USER + 1);
         CBEM_SETIMAGELIST              = (WM_USER + 2);
         CBEM_GETIMAGELIST              = (WM_USER + 3);
         CBEM_GETITEMA                  = (WM_USER + 4);
         CBEM_SETITEMA                  = (WM_USER + 5);
         CBEM_DELETEITEM                = CB_DELETESTRING;
         CBEM_GETCOMBOCONTROL           = (WM_USER + 6);
         CBEM_GETEDITCONTROL            = (WM_USER + 7);
{$ifdef ie4plus}
         CBEM_SETEXSTYLE                = (WM_USER + 8);      // use  SETEXTENDEDSTYLE instead
         CBEM_SETEXTENDEDSTYLE          = (WM_USER + 14);     // lparam == new style, wParam (optional) == mask
         CBEM_GETEXSTYLE                = (WM_USER + 9);      // use GETEXTENDEDSTYLE instead
         CBEM_GETEXTENDEDSTYLE          = (WM_USER + 9);
         CBEM_SETUNICODEFORMAT          = CCM_SETUNICODEFORMAT;
         CBEM_GETUNICODEFORMAT          = CCM_GETUNICODEFORMAT;
{$ELSE}
         CBEM_SETEXSTYLE                = (WM_USER + 8);
         CBEM_GETEXSTYLE                = (WM_USER + 9);
{$ENDIF}
         CBEM_HASEDITCHANGED            = (WM_USER + 10);
         CBEM_INSERTITEMW               = (WM_USER + 11);
         CBEM_SETITEMW                  = (WM_USER + 12);
         CBEM_GETITEMW                  = (WM_USER + 13);

{$IFDEF UNICODE}

         CBEM_INSERTITEM     = CBEM_INSERTITEMW;
         CBEM_SETITEM        = CBEM_SETITEMW;
         CBEM_GETITEM        = CBEM_GETITEMW;
{$ELSE}
         CBEM_INSERTITEM     = CBEM_INSERTITEMA;
         CBEM_SETITEM        = CBEM_SETITEMA;
         CBEM_GETITEM        = CBEM_GETITEMA;
{$ENDIF}

{$ifdef win32xp}

         CBEM_SETWINDOWTHEME            = CCM_SETWINDOWTHEME;
{$ENDIF}

         CBES_EX_NOEDITIMAGE            = $00000001;
         CBES_EX_NOEDITIMAGEINDENT      = $00000002;
         CBES_EX_PATHWORDBREAKPROC      = $00000004;
{$ifdef ie4plus}
         CBES_EX_NOSIZELIMIT            = $00000008;
         CBES_EX_CASESENSITIVE          = $00000010;
{$ifdef win32vista}
	 CBES_EX_TEXTENDELLIPSIS      	= $00000020;
{$endif}
TYPE

         DummyStruct9         = Record
                                 hdr          : NMHDR;
                                 ceItem       : COMBOBOXEXITEMA;
                                 END;
         NMCOMBOBOXEXA        = DummyStruct9;
         PNMCOMBOBOXEXA       = ^DummyStruct9;
         TDummyStruct9        = DummyStruct9;
         PDummyStruct9        = ^DummyStruct9;


         DummyStruct10        = Record
                                 hdr          : NMHDR;
                                 ceItem       : COMBOBOXEXITEMW;
                                 END;
         NMCOMBOBOXEXW        = DummyStruct10;
         PNMCOMBOBOXEXW       = ^DummyStruct10;
         TDummyStruct10       = DummyStruct10;
         PDummyStruct10       = ^DummyStruct10;



{$ELSE}
         DummyStruct11        = Record
                                 hdr          : NMHDR;
                                 ceItem       : COMBOBOXEXITEM;
                                 END;
         NMCOMBOBOXEX         = DummyStruct11;
         PNMCOMBOBOXEX        = ^DummyStruct11;
         TDummyStruct11       = DummyStruct11;
         PDummyStruct11       = ^DummyStruct11;



CONST
         CBEN_GETDISPINFO               = (CBEN_FIRST - 0);

{$ENDIF}      // _WIN32_IE >= 0x0400
CONST
{$ifdef ie4plus}
         CBEN_GETDISPINFOA              = (CBEN_FIRST - 0);
{$ENDIF}
         CBEN_INSERTITEM                = (CBEN_FIRST - 1);
         CBEN_DELETEITEM                = (CBEN_FIRST - 2);
         CBEN_BEGINEDIT                 = (CBEN_FIRST - 4);
         CBEN_ENDEDITA                  = (CBEN_FIRST - 5);
         CBEN_ENDEDITW                  = (CBEN_FIRST - 6);

{$ifdef ie4plus}
         CBEN_GETDISPINFOW              = (CBEN_FIRST - 7);
{$ENDIF}

{$ifdef ie4plus}
         CBEN_DRAGBEGINA                = (CBEN_FIRST - 8);
         CBEN_DRAGBEGINW                = (CBEN_FIRST - 9);

{$IFDEF UNICODE}

         CBEN_DRAGBEGIN      = CBEN_DRAGBEGINW;
{$ELSE}
         CBEN_DRAGBEGIN      = CBEN_DRAGBEGINA;
{$ENDIF}

{$ENDIF}  //(_WIN32_IE >= 0x0400)
TYPE
{$IFDEF UNICODE}
         NMCOMBOBOXEX        = NMCOMBOBOXEXW;
         PNMCOMBOBOXEX       = PNMCOMBOBOXEXW;
CONST
         CBEN_GETDISPINFO    = CBEN_GETDISPINFOW;
{$ELSE}
         NMCOMBOBOXEX        = NMCOMBOBOXEXA;
         PNMCOMBOBOXEX       = PNMCOMBOBOXEXA;
CONST
         CBEN_GETDISPINFO    = CBEN_GETDISPINFOA;
{$ENDIF}

// lParam specifies why the endedit is happening
{$IFDEF UNICODE}
         CBEN_ENDEDIT        = CBEN_ENDEDITW;
{$ELSE}
         CBEN_ENDEDIT        = CBEN_ENDEDITA;
{$ENDIF}


CONST
         CBENF_KILLFOCUS                = 1;
         CBENF_RETURN                   = 2;
         CBENF_ESCAPE                   = 3;
         CBENF_DROPDOWN                 = 4;

         CBEMAXSTRLEN                   = 260;

{$ifdef ie4plus}
// CBEN_DRAGBEGIN sends this information ...

TYPE

         DummyStruct12        = Record
                                 hdr          : NMHDR;
                                 iItemid      : cint;
                                 szText       : Array[0..CBEMAXSTRLEN-1] OF WCHAR;
                                 END;
         NMCBEDRAGBEGINW      = DummyStruct12;
         LPNMCBEDRAGBEGINW    = ^DummyStruct12;
         PNMCBEDRAGBEGINW     = ^DummyStruct12;
         TDummyStruct12       = DummyStruct12;
         PDummyStruct12       = ^DummyStruct12;



         DummyStruct13        = Record
                                 hdr          : NMHDR;
                                 iItemid      : cint;
                                 szText       : Array[0..CBEMAXSTRLEN-1] OF char;
                                 END;
         NMCBEDRAGBEGINA      = DummyStruct13;
         LPNMCBEDRAGBEGINA    = ^DummyStruct13;
         PNMCBEDRAGBEGINA     = ^DummyStruct13;
         TDummyStruct13       = DummyStruct13;
         PDummyStruct13       = ^DummyStruct13;


{$IFDEF UNICODE}
         NMCBEDRAGBEGIN      = NMCBEDRAGBEGINW;
         LPNMCBEDRAGBEGIN    = LPNMCBEDRAGBEGINW;
         PNMCBEDRAGBEGIN     = PNMCBEDRAGBEGINW;
{$ELSE}
         NMCBEDRAGBEGIN      = NMCBEDRAGBEGINA;
         LPNMCBEDRAGBEGIN    = LPNMCBEDRAGBEGINA;
         PNMCBEDRAGBEGIN     = PNMCBEDRAGBEGINA;
{$ENDIF}
{$ENDIF}      // _WIN32_IE >= 0x0400

// CBEN_ENDEDIT sends this information...
// fChanged if the user actually did anything
// iNewSelection gives what would be the new selection unless the notify is failed
//                      iNewSelection may be CB_ERR if there's no match
         DummyStruct14        = Record
                                 hdr          : NMHDR;
                                 fChanged     : BOOL;
                                 iNewSelection : cint;
                                 szText       : Array[0..CBEMAXSTRLEN-1] OF WCHAR;
                                 iWhy         : cint;
                                 END;
         NMCBEENDEDITW        = DummyStruct14;
         LPNMCBEENDEDITW      = ^DummyStruct14;
         PNMCBEENDEDITW       = ^DummyStruct14;
         TDummyStruct14       = DummyStruct14;
         PDummyStruct14       = ^DummyStruct14;


         DummyStruct15        = Record
                                 hdr          : NMHDR;
                                 fChanged     : BOOL;
                                 iNewSelection : cint;
                                 szText       : Array[0..CBEMAXSTRLEN-1] OF char;
                                 iWhy         : cint;
                                 END;
         NMCBEENDEDITA        = DummyStruct15;
         LPNMCBEENDEDITA      = ^DummyStruct15;
         PNMCBEENDEDITA       = ^DummyStruct15;
         TDummyStruct15       = DummyStruct15;
         PDummyStruct15       = ^DummyStruct15;


{$IFDEF UNICODE}
         NMCBEENDEDIT        = NMCBEENDEDITW;
         LPNMCBEENDEDIT      = LPNMCBEENDEDITW;
         PNMCBEENDEDIT       = PNMCBEENDEDITW;
{$ELSE}
         NMCBEENDEDIT        = NMCBEENDEDITA;
         LPNMCBEENDEDIT      = LPNMCBEENDEDITA;
         PNMCBEENDEDIT       = PNMCBEENDEDITA;
{$ENDIF}

{$ENDIF}

{$ENDIF}      // _WIN32_IE >= 0x0300



//====== TAB CONTROL ==========================================================

{$IFNDEF NOTABCONTROL}

{$IFDEF _WIN32}


CONST
         WC_TABCONTROLA                 = 'SysTabControl32';
         WC_TABCONTROLW                 = {L}'SysTabControl32';

{$IFDEF UNICODE}

TYPE
         WC_TABCONTROL       = WC_TABCONTROLW;
{$ELSE}
         WC_TABCONTROL       = WC_TABCONTROLA;
{$ENDIF}

{$ELSE}

CONST
         WC_TABCONTROL                  = 'SysTabControl';
{$ENDIF}

// begin_r_commctrl

{$ifdef ie3plus}
         TCS_SCROLLOPPOSITE             = $0001;              // assumes multiline tab
         TCS_BOTTOM                     = $0002;
         TCS_RIGHT                      = $0002;
         TCS_MULTISELECT                = $0004;              // allow multi-select in button mode
{$ENDIF}
{$ifdef ie4plus}
         TCS_FLATBUTTONS                = $0008;
{$ENDIF}
         TCS_FORCEICONLEFT              = $0010;
         TCS_FORCELABELLEFT             = $0020;
{$ifdef ie3plus}
         TCS_HOTTRACK                   = $0040;
         TCS_VERTICAL                   = $0080;
{$ENDIF}
         TCS_TABS                       = $0000;
         TCS_BUTTONS                    = $0100;
         TCS_SINGLELINE                 = $0000;
         TCS_MULTILINE                  = $0200;
         TCS_RIGHTJUSTIFY               = $0000;
         TCS_FIXEDWIDTH                 = $0400;
         TCS_RAGGEDRIGHT                = $0800;
         TCS_FOCUSONBUTTONDOWN          = $1000;
         TCS_OWNERDRAWFIXED             = $2000;
         TCS_TOOLTIPS                   = $4000;
         TCS_FOCUSNEVER                 = $8000;

// end_r_commctrl

{$ifdef ie4plus}
// EX styles for use with TCM_SETEXTENDEDSTYLE
         TCS_EX_FLATSEPARATORS          = $00000001;
         TCS_EX_REGISTERDROP            = $00000002;
{$ENDIF}


         TCM_GETIMAGELIST               = (TCM_FIRST + 2);
// Macro 217


Function TabCtrl_GetImageList( hwnd : hwnd):HIMAGELIST;




CONST
         TCM_SETIMAGELIST               = (TCM_FIRST + 3);
// Macro 218


Function TabCtrl_SetImageList( hwnd : hwnd; himl : HIMAGELIST):HIMAGELIST;




CONST
         TCM_GETITEMCOUNT               = (TCM_FIRST + 4);
// Macro 219


Function TabCtrl_GetItemCount( hwnd : hwnd):cint;




CONST
         TCIF_TEXT                      = $0001;
         TCIF_IMAGE                     = $0002;
         TCIF_RTLREADING                = $0004;
         TCIF_PARAM                     = $0008;
{$ifdef ie3plus}
         TCIF_STATE                     = $0010;


         TCIS_BUTTONPRESSED             = $0001;
{$ENDIF}
{$ifdef ie4plus}
         TCIS_HIGHLIGHTED               = $0002;
{$ENDIF}
TYPE




         tagTCITEMHEADERA     = Record
                                 mask         : UINT;
                                 lpReserved1  : UINT;
                                 lpReserved2  : UINT;
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 END;
         TCITEMHEADERA        = tagTCITEMHEADERA;
         LPTCITEMHEADERA      = ^tagTCITEMHEADERA;
         TTCITEMHEADERA       = tagTCITEMHEADERA;
         PTCITEMHEADERA       = ^tagTCITEMHEADERA;


         tagTCITEMHEADERW     = Record
                                 mask         : UINT;
                                 lpReserved1  : UINT;
                                 lpReserved2  : UINT;
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 END;
         TCITEMHEADERW        = tagTCITEMHEADERW;
         LPTCITEMHEADERW      = ^tagTCITEMHEADERW;
         TTCITEMHEADERW       = tagTCITEMHEADERW;
         PTCITEMHEADERW       = ^tagTCITEMHEADERW;


{$IFDEF UNICODE}
         TCITEMHEADER        = TCITEMHEADERW;
         LPTCITEMHEADER      = LPTCITEMHEADERW;
{$ELSE}
         TCITEMHEADER        = TCITEMHEADERA;
         LPTCITEMHEADER      = LPTCITEMHEADERA;
{$ENDIF}


{$ifdef ie3plus}
         TC_ITEMHEADERA      = TCITEMHEADERA;
         TC_ITEMHEADERW      = TCITEMHEADERW;
{$ELSE}
         tagTCITEMHEADERA    = _TC_ITEMHEADERA;
         TCITEMHEADERA       = TC_ITEMHEADERA;
         tagTCITEMHEADERW    = _TC_ITEMHEADERW;
         TCITEMHEADERW       = TC_ITEMHEADERW;
{$ENDIF}


         TC_ITEMHEADER                  = TCITEMHEADER;





         tagTCITEMA           = Record
                                 mask         : UINT;
{$ifdef ie3plus}
                                 dwState      : DWORD;
                                 dwStateMask  : DWORD;
{$ELSE}
                                 lpReserved1  : UINT;
                                 lpReserved2  : UINT;
{$ENDIF}
                                 pszText      : LPSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 lParam       : LPARAM;
                                 END;
         TCITEMA              = tagTCITEMA;
         LPTCITEMA            = ^tagTCITEMA;
         TTCITEMA             = tagTCITEMA;
         PTCITEMA             = ^tagTCITEMA;


         tagTCITEMW           = Record
                                 mask         : UINT;
{$ifdef ie3plus}
                                 dwState      : DWORD;
                                 dwStateMask  : DWORD;
{$ELSE}
                                 lpReserved1  : UINT;
                                 lpReserved2  : UINT;
{$ENDIF}
                                 pszText      : LPWSTR;
                                 cchTextMax   : cint;
                                 iImage       : cint;
                                 lParam       : LPARAM;
                                 END;
         TCITEMW              = tagTCITEMW;
         LPTCITEMW            = ^tagTCITEMW;
         TTCITEMW             = tagTCITEMW;
         PTCITEMW             = ^tagTCITEMW;


{$IFDEF UNICODE}
         TCITEM              = TCITEMW;
         LPTCITEM            = LPTCITEMW;
{$ELSE}
         TCITEM              = TCITEMA;
         LPTCITEM            = LPTCITEMA;
{$ENDIF}


{$ifdef ie3plus}
         TC_ITEMA            = TCITEMA;
         TC_ITEMW            = TCITEMW;
{$ELSE}
         tagTCITEMA          = _TC_ITEMA;
         TCITEMA             = TC_ITEMA;
         tagTCITEMW          = _TC_ITEMW;
         TCITEMW             = TC_ITEMW;
{$ENDIF}


         TC_ITEM                        = TCITEM;




CONST
         TCM_GETITEMA                   = (TCM_FIRST + 5);
         TCM_GETITEMW                   = (TCM_FIRST + 60);

{$IFDEF UNICODE}

TYPE
         TCM_GETITEM         = TCM_GETITEMW;
{$ELSE}
         TCM_GETITEM         = TCM_GETITEMA;
{$ENDIF}

// Macro 220
Function TabCtrl_GetItem( hwnd : hwnd; iItem : cint;var  pitem : TC_ITEM ):BOOL;




CONST
         TCM_SETITEMA                   = (TCM_FIRST + 6);
         TCM_SETITEMW                   = (TCM_FIRST + 61);

{$IFDEF UNICODE}

TYPE
         TCM_SETITEM         = TCM_SETITEMW;
{$ELSE}
         TCM_SETITEM         = TCM_SETITEMA;
{$ENDIF}

// Macro 221
Function TabCtrl_SetItem( hwnd : hwnd; iItem : cint;var  pitem : TC_ITEM ):BOOL;




CONST
         TCM_INSERTITEMA                = (TCM_FIRST + 7);
         TCM_INSERTITEMW                = (TCM_FIRST + 62);

{$IFDEF UNICODE}
         TCM_INSERTITEM      = TCM_INSERTITEMW;
{$ELSE}
         TCM_INSERTITEM      = TCM_INSERTITEMA;
{$ENDIF}

// Macro 222
Function TabCtrl_InsertItem( hwnd : hwnd; iItem : cint;CONST  pitem : TC_ITEM ):cint;


CONST
         TCM_DELETEITEM                 = (TCM_FIRST + 8);

// Macro 223
Function TabCtrl_DeleteItem( hwnd : hwnd; i : cint):BOOL;

CONST
         TCM_DELETEALLITEMS             = (TCM_FIRST + 9);

// Macro 224
Function TabCtrl_DeleteAllItems( hwnd : hwnd):BOOL;

CONST
         TCM_GETITEMRECT                = (TCM_FIRST + 10);

// Macro 225
Function TabCtrl_GetItemRect( hwnd : hwnd; i : cint;var  prc : RECT ):BOOL;

CONST
         TCM_GETCURSEL                  = (TCM_FIRST + 11);
// Macro 226

Function TabCtrl_GetCurSel( hwnd : hwnd):cint;


CONST
         TCM_SETCURSEL                  = (TCM_FIRST + 12);

// Macro 227
Function TabCtrl_SetCurSel( hwnd : hwnd; i : WPARAM):cint;

CONST
         TCHT_NOWHERE                   = $0001;
         TCHT_ONITEMICON                = $0002;
         TCHT_ONITEMLABEL               = $0004;
         TCHT_ONITEM                    = (TCHT_ONITEMICON  OR  TCHT_ONITEMLABEL);

TYPE

         tagTCHITTESTINFO     = Record
                                 pt           : POINT;
                                 flags        : UINT;
                                 END;
         TCHITTESTINFO        = tagTCHITTESTINFO;
         LPTCHITTESTINFO      = ^tagTCHITTESTINFO;
         TTCHITTESTINFO       = tagTCHITTESTINFO;
         PTCHITTESTINFO       = ^tagTCHITTESTINFO;

{$ifdef ie3plus}
         LPTC_HITTESTINFO    = LPTCHITTESTINFO;
         TC_HITTESTINFO                 = TCHITTESTINFO;
{$ELSE}
         tagTCHITTESTINFO               = _TC_HITTESTINFO;
         TCHITTESTINFO                  = TC_HITTESTINFO;
         LPTCHITTESTINFO     = LPTC_HITTESTINFO;
{$ENDIF}


CONST
         TCM_HITTEST                    = (TCM_FIRST + 13);

// Macro 228
Function TabCtrl_HitTest( hwndTC : hwnd;var  pinfo : TC_HITTESTINFO ):cint;inline;
Function TabCtrl_HitTest( hwndTC : hwnd;pinfo : LPTCHITTESTINFO ):cint;inline;

CONST
         TCM_SETITEMEXTRA               = (TCM_FIRST + 14);

// Macro 229
Function TabCtrl_SetItemExtra( hwndTC : hwnd; cb : WPARAM):BOOL;




CONST
         TCM_ADJUSTRECT                 = (TCM_FIRST + 40);

// Macro 230
Function TabCtrl_AdjustRect( hwnd : hwnd; bLarger : BOOL;var  prc : RECT ):cint;




CONST
         TCM_SETITEMSIZE                = (TCM_FIRST + 41);

// Macro 231
Function TabCtrl_SetItemSize( hwnd : hwnd; x :wparam ; y : lparam ):DWORD;

CONST
         TCM_REMOVEIMAGE                = (TCM_FIRST + 42);

// Macro 232
Procedure TabCtrl_RemoveImage( hwnd : hwnd; i : wparam );




CONST
         TCM_SETPADDING                 = (TCM_FIRST + 43);

// Macro 233
Procedure TabCtrl_SetPadding( hwnd : hwnd; cx : wparam ; cy : lparam );




CONST
         TCM_GETROWCOUNT                = (TCM_FIRST + 44);

// Macro 234
Function TabCtrl_GetRowCount( hwnd : hwnd):cint;




CONST
         TCM_GETTOOLTIPS                = (TCM_FIRST + 45);

// Macro 235
Function TabCtrl_GetToolTips( hwnd : hwnd):HWND;




CONST
         TCM_SETTOOLTIPS                = (TCM_FIRST + 46);

// Macro 236
Procedure TabCtrl_SetToolTips( hwnd : hwnd; hwndTT : WPARAM);




CONST
         TCM_GETCURFOCUS                = (TCM_FIRST + 47);
// Macro 237


Function TabCtrl_GetCurFocus( hwnd : hwnd):cint;



CONST
         TCM_SETCURFOCUS                = (TCM_FIRST + 48);

// Macro 238
Procedure TabCtrl_SetCurFocus( hwnd : hwnd; i : LParam );


{$ifdef ie3plus}

CONST
         TCM_SETMINTABWIDTH             = (TCM_FIRST + 49);

// Macro 239
Function TabCtrl_SetMinTabWidth( hwnd : hwnd; x : WParam ):cint;


CONST
         TCM_DESELECTALL                = (TCM_FIRST + 50);

// Macro 240
Procedure TabCtrl_DeselectAll( hwnd : hwnd; fExcludeFocus : WPARAM );

{$ENDIF}

{$ifdef ie4plus}
CONST
         TCM_HIGHLIGHTITEM              = (TCM_FIRST + 51);

// Macro 241
Function TabCtrl_HighlightItem( hwnd : hwnd; i : WPARAM; fHighlight :bool):BOOL;



CONST
         TCM_SETEXTENDEDSTYLE           = (TCM_FIRST + 52);   // optional wParam == mask

// Macro 242
Function TabCtrl_SetExtendedStyle( hwnd : hwnd; dw :LPARAM ):DWORD;


CONST
         TCM_GETEXTENDEDSTYLE           = (TCM_FIRST + 53);

// Macro 243
Function TabCtrl_GetExtendedStyle( hwnd : hwnd):DWORD;



CONST
         TCM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;

// Macro 244
Function TabCtrl_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;



CONST
         TCM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;

// Macro 245
Function TabCtrl_GetUnicodeFormat( hwnd : hwnd):BOOL;


{$ENDIF}      // _WIN32_IE >= 0x0400


CONST
         TCN_KEYDOWN                    = (TCN_FIRST - 0);


{$IFDEF _WIN32}
// #include <pshpack1.h>
{$ENDIF}

TYPE

         tagTCKEYDOWN         = Record
                                 hdr          : NMHDR;
                                 wVKey        : WORD;
                                 flags        : UINT;
                                 END;
         NMTCKEYDOWN          = tagTCKEYDOWN;
         TTCKEYDOWN           = tagTCKEYDOWN;
         PTCKEYDOWN           = ^tagTCKEYDOWN;

{$ifdef ie3plus}
         TC_KEYDOWN                     = NMTCKEYDOWN;
{$ELSE}
         tagTCKEYDOWN                   = _TC_KEYDOWN;
         NMTCKEYDOWN                    = TC_KEYDOWN;
{$ENDIF}


{$IFDEF _WIN32}
// #include <poppack.h>
{$ENDIF}


CONST
         TCN_SELCHANGE                  = (TCN_FIRST - 1);
         TCN_SELCHANGING                = (TCN_FIRST - 2);
{$ifdef ie4plus}
         TCN_GETOBJECT                  = (TCN_FIRST - 3);
{$ENDIF}      // _WIN32_IE >= 0x0400
{$ifdef ie5plus}
         TCN_FOCUSCHANGE                = (TCN_FIRST - 4);
{$ENDIF}      // _WIN32_IE >= 0x0500
{$ENDIF}      // NOTABCONTROL

//====== ANIMATE CONTROL ======================================================

{$IFNDEF NOANIMATE}

{$IFDEF _WIN32}

         ANIMATE_CLASSW                 = {L}'SysAnimate32';
         ANIMATE_CLASSA                 = 'SysAnimate32';

{$IFDEF UNICODE}
         ANIMATE_CLASS       = ANIMATE_CLASSW;
{$ELSE}
         ANIMATE_CLASS       = ANIMATE_CLASSA;
{$ENDIF}

// begin_r_commctrl


CONST
         ACS_CENTER                     = $0001;
         ACS_TRANSPARENT                = $0002;
         ACS_AUTOPLAY                   = $0004;
{$ifdef ie3plus}
         ACS_TIMER                      = $0008;              // don't use threads... use timers
{$ENDIF}

// end_r_commctrl

         ACM_OPENA                      = (WM_USER+100);
         ACM_OPENW                      = (WM_USER+103);

{$IFDEF UNICODE}


         ACM_OPEN            = ACM_OPENW;
{$ELSE}
         ACM_OPEN            = ACM_OPENA;
{$ENDIF}


CONST
         ACM_PLAY                       = (WM_USER+101);
         ACM_STOP                       = (WM_USER+102);


         ACN_START                      = 1;
         ACN_STOP                       = 2;


// Macro 246


Function Animate_Create(hwndP :HWND;id:HMENU;dwStyle:dword;hInstance:HINST):HWND;

// Macro 247

// #define Animate_Open(hwnd, szName)          (BOOL)SNDMSG(hwnd, ACM_OPEN, 0, (LPARAM)(LPTSTR)(szName))
// Macro 248

// #define Animate_OpenEx(hwnd, hInst, szName) (BOOL)SNDMSG(hwnd, ACM_OPEN, (WPARAM)(hInst), (LPARAM)(LPTSTR)(szName))
// Macro 249

// #define Animate_Play(hwnd, from, to, rep)   (BOOL)SNDMSG(hwnd, ACM_PLAY, (WPARAM)(rep), (LPARAM)MAKELONG(from, to))
// Macro 250

// #define Animate_Stop(hwnd)                  (BOOL)SNDMSG(hwnd, ACM_STOP, 0, 0)
// Macro 251

// #define Animate_Close(hwnd)                 Animate_Open(hwnd, NULL)
// Macro 252

// #define Animate_Seek(hwnd, frame)           Animate_Play(hwnd, frame, frame, 1)
{$ENDIF}

{$ENDIF}      // NOANIMATE

{$ifdef ie3plus}
//====== MONTHCAL CONTROL ======================================================

{$IFNDEF NOMONTHCAL}
{$IFDEF _WIN32}


CONST
         MONTHCAL_CLASSW                = {L}'SysMonthCal32';
         MONTHCAL_CLASSA                = 'SysMonthCal32';


{$IFDEF UNICODE}
         MONTHCAL_CLASS      = MONTHCAL_CLASSW;
{$ELSE}
         MONTHCAL_CLASS      = MONTHCAL_CLASSA;
{$ENDIF}

TYPE
// bit-packed array of "bold" info for a month
// if a bit is on, that day is drawn bold

         MONTHDAYSTATE       = DWORD;
         LPMONTHDAYSTATE     = ^MONTHDAYSTATE;
         TMonthDayState      = MONTHDAYSTATE;
         PMonthDayState      = LPMONTHDAYSTATE;


CONST
         MCM_FIRST                      = $1000;

// BOOL MonthCal_GetCurSel(HWND hmc, LPSYSTEMTIME pst)
//   returns FALSE if MCS_MULTISELECT
//   returns TRUE and sets *pst to the currently selected date otherwise




CONST
         MCM_GETCURSEL                  = (MCM_FIRST + 1);

// Macro 253
// #define MonthCal_GetCurSel(hmc, pst)    (BOOL)SNDMSG(hmc, MCM_GETCURSEL, 0, (LPARAM)(pst))

function MonthCal_GetCurSel(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;

// BOOL MonthCal_SetCurSel(HWND hmc, LPSYSTEMTIME pst)
//   returns FALSE if MCS_MULTISELECT
//   returns TURE and sets the currently selected date to *pst otherwise



CONST
         MCM_SETCURSEL                  = (MCM_FIRST + 2);

// Macro 254
// #define MonthCal_SetCurSel(hmc, pst)    (BOOL)SNDMSG(hmc, MCM_SETCURSEL, 0, (LPARAM)(pst))
// DWORD MonthCal_GetMaxSelCount(HWND hmc)
//   returns the maximum number of selectable days allowed

function MonthCal_SetCurSel(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;inline;
function MonthCal_SetCurSel(hwndMC:HWND; var lpSysTime :TSYSTEMTIME):Bool;inline;


CONST
         MCM_GETMAXSELCOUNT             = (MCM_FIRST + 3);

// Macro 255
// #define MonthCal_GetMaxSelCount(hmc)    (DWORD)SNDMSG(hmc, MCM_GETMAXSELCOUNT, 0, 0L)
// BOOL MonthCal_SetMaxSelCount(HWND hmc, UINT n)
//   sets the max number days that can be selected iff MCS_MULTISELECT

function MonthCal_GetMaxSelCount(hwndMC:HWND):Bool;

CONST
         MCM_SETMAXSELCOUNT             = (MCM_FIRST + 4);

// Macro 256
// #define MonthCal_SetMaxSelCount(hmc, n) (BOOL)SNDMSG(hmc, MCM_SETMAXSELCOUNT, (WPARAM)(n), 0L)

function MonthCal_SetMaxSelCount(hwndMC:HWND;n:uint):Bool;

// BOOL MonthCal_GetSelRange(HWND hmc, LPSYSTEMTIME rgst)
//   sets rgst[0] to the first day of the selection range
//   sets rgst[1] to the last day of the selection range

CONST
         MCM_GETSELRANGE                = (MCM_FIRST + 5);

// Macro 257
// #define MonthCal_GetSelRange(hmc, rgst) SNDMSG(hmc, MCM_GETSELRANGE, 0, (LPARAM)(rgst))
// BOOL MonthCal_SetSelRange(HWND hmc, LPSYSTEMTIME rgst)
//   selects the range of days from rgst[0] to rgst[1]

function MonthCal_GetSelRange(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;


CONST
         MCM_SETSELRANGE                = (MCM_FIRST + 6);

// Macro 258
// #define MonthCal_SetSelRange(hmc, rgst) SNDMSG(hmc, MCM_SETSELRANGE, 0, (LPARAM)(rgst))
// DWORD MonthCal_GetMonthRange(HWND hmc, DWORD gmr, LPSYSTEMTIME rgst)
//   if rgst specified, sets rgst[0] to the starting date and
//      and rgst[1] to the ending date of the the selectable (non-grayed)
//      days if GMR_VISIBLE or all the displayed days (including grayed)
//      if GMR_DAYSTATE.
//   returns the number of months spanned by the above range.

function MonthCal_SetSelRange(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;


CONST
         MCM_GETMONTHRANGE              = (MCM_FIRST + 7);

// Macro 259
// #define MonthCal_GetMonthRange(hmc, gmr, rgst)  (DWORD)SNDMSG(hmc, MCM_GETMONTHRANGE, (WPARAM)(gmr), (LPARAM)(rgst))
// BOOL MonthCal_SetDayState(HWND hmc, int cbds, DAYSTATE *rgds)
//   cbds is the count of DAYSTATE items in rgds and it must be equal
//   to the value returned from MonthCal_GetMonthRange(hmc, GMR_DAYSTATE, NULL)
//   This sets the DAYSTATE bits for each month (grayed and non-grayed
//   days) displayed in the calendar. The first bit in a month's DAYSTATE
//   corresponts to bolding day 1, the second bit affects day 2, etc.

function MonthCal_GetMonthRange(hwndMC:HWND; gmr: DWORD;lpSysTime :LPSYSTEMTIME):Bool;


CONST
         MCM_SETDAYSTATE                = (MCM_FIRST + 8);

// Macro 260
// #define MonthCal_SetDayState(hmc, cbds, rgds)   SNDMSG(hmc, MCM_SETDAYSTATE, (WPARAM)(cbds), (LPARAM)(rgds))
// BOOL MonthCal_GetMinReqRect(HWND hmc, LPRECT prc)
//   sets *prc the minimal size needed to display one month
//   To display two months, undo the AdjustWindowRect calculation already done to
//   this rect, double the width, and redo the AdjustWindowRect calculation --
//   the monthcal control will display two calendars in this window (if you also
//   double the vertical size, you will get 4 calendars)
//   NOTE: if you want to gurantee that the "Today" string is not clipped,
//   get the MCM_GETMAXTODAYWIDTH and use the max of that width and this width

function MonthCal_SetDayState(hwndMC:HWND; gmr: Longint;lpDay :LPMONTHDAYSTATE):Bool;


CONST
         MCM_GETMINREQRECT              = (MCM_FIRST + 9);

// Macro 261
// #define MonthCal_GetMinReqRect(hmc, prc)        SNDMSG(hmc, MCM_GETMINREQRECT, 0, (LPARAM)(prc))
// set colors to draw control with -- see MCSC_ bits below

function MonthCal_GetMinReqRect(hwndMC:HWND; lpr :LPRect):Bool;inline;
function MonthCal_GetMinReqRect(hwndMC:HWND; var lpr :TRect):Bool;inline;


CONST
         MCM_SETCOLOR                   = (MCM_FIRST + 10);

// Macro 262
// #define MonthCal_SetColor(hmc, iColor, clr) SNDMSG(hmc, MCM_SETCOLOR, iColor, clr)

function MonthCal_SetColor(hwndMC:HWND; ic:longint;clr:COLORREF):DWORD;inline;


CONST
         MCM_GETCOLOR                   = (MCM_FIRST + 11);

// Macro 263
// #define MonthCal_GetColor(hmc, iColor) SNDMSG(hmc, MCM_GETCOLOR, iColor, 0)

function MonthCal_GetColor(hwndMC:HWND; ic:longint):Bool;


CONST
         MCSC_BACKGROUND                = 0;                  // the background color (between months)
         MCSC_TEXT                      = 1;                  // the dates
         MCSC_TITLEBK                   = 2;                  // background of the title
         MCSC_TITLETEXT                 = 3;
         MCSC_MONTHBK                   = 4;                  // background within the month cal
         MCSC_TRAILINGTEXT              = 5;                  // the text color of header & trailing days

// set what day is "today"   send NULL to revert back to real date
         MCM_SETTODAY                   = (MCM_FIRST + 12);

// Macro 264
// #define MonthCal_SetToday(hmc, pst)             SNDMSG(hmc, MCM_SETTODAY, 0, (LPARAM)(pst))
// get what day is "today"
// returns BOOL for success/failure

function MonthCal_SetToday(hwndMC:HWND; lps:LPSYSTEMTIME):Bool;


CONST
         MCM_GETTODAY                   = (MCM_FIRST + 13);

// Macro 265
// #define MonthCal_GetToday(hmc, pst)             (BOOL)SNDMSG(hmc, MCM_GETTODAY, 0, (LPARAM)(pst))
// determine what pinfo->pt is over

function MonthCal_GetToday(hwndMC:HWND; lps:LPSYSTEMTIME):Bool;


CONST
         MCM_HITTEST                    = (MCM_FIRST + 14);

TYPE

         DummyStruct16        = Record
                                 cbSize       : UINT;
                                 pt           : POINT;
                                 uHit         : UINT;          // out param
                                 st           : SYSTEMTIME;
{$ifdef NTDDI_VISTA}
				 rc 	      : RECT;
				 iOffset      : cint;
				 iRow         : cint;
				 iCol	      : cint;
{$endif}				 
                                 END;
         MCHITTESTINFO        = DummyStruct16;
         PMCHITTESTINFO       = ^DummyStruct16;
         TDummyStruct16       = DummyStruct16;
         PDummyStruct16       = ^DummyStruct16;


// Macro 266
FUNCTION MonthCal_HitTest( hmc :HWND ; pinfo : PMCHITTESTINFO):DWORD;


CONST
         MCHT_TITLE                     = $00010000;
         MCHT_CALENDAR                  = $00020000;
         MCHT_TODAYLINK                 = $00030000;

         MCHT_NEXT                      = $01000000;          // these indicate that hitting
         MCHT_PREV                      = $02000000;          // here will go to the next/prev month

         MCHT_NOWHERE                   = $00000000;

         MCHT_TITLEBK                   = (MCHT_TITLE);
         MCHT_TITLEMONTH                = (MCHT_TITLE  OR  $0001);
         MCHT_TITLEYEAR                 = (MCHT_TITLE  OR  $0002);
         MCHT_TITLEBTNNEXT              = (MCHT_TITLE  OR  MCHT_NEXT  OR  $0003);
         MCHT_TITLEBTNPREV              = (MCHT_TITLE  OR  MCHT_PREV  OR  $0003);

         MCHT_CALENDARBK                = (MCHT_CALENDAR);
         MCHT_CALENDARDATE              = (MCHT_CALENDAR  OR  $0001);
         MCHT_CALENDARDATENEXT          = (MCHT_CALENDARDATE  OR  MCHT_NEXT);
         MCHT_CALENDARDATEPREV          = (MCHT_CALENDARDATE  OR  MCHT_PREV);
         MCHT_CALENDARDAY               = (MCHT_CALENDAR  OR  $0002);
         MCHT_CALENDARWEEKNUM           = (MCHT_CALENDAR  OR  $0003);

// set first day of week to iDay:
// 0 for Monday, 1 for Tuesday, ..., 6 for Sunday
// -1 for means use locale info
         MCM_SETFIRSTDAYOFWEEK          = (MCM_FIRST + 15);
// Macro 267


function MonthCal_SetFirstDayOfWeek( hmc : HWND ; iDay :LONGINT ):DWORD;


// DWORD result...  low word has the day.  high word is bool if this is app set
// or not (FALSE == using locale info)

CONST
         MCM_GETFIRSTDAYOFWEEK          = (MCM_FIRST + 16);
// Macro 268


Function MonthCal_GetFirstDayOfWeek( hmc : HWND ):DWORD;


// DWORD MonthCal_GetRange(HWND hmc, LPSYSTEMTIME rgst)
//   modifies rgst[0] to be the minimum ALLOWABLE systemtime (or 0 if no minimum)
//   modifies rgst[1] to be the maximum ALLOWABLE systemtime (or 0 if no maximum)
//   returns GDTR_MIN|GDTR_MAX if there is a minimum|maximum limit

CONST
         MCM_GETRANGE                   = (MCM_FIRST + 17);
// Macro 269


Function MonthCal_GetRange( hmc : HWND ; rgst : LPSYSTEMTIME):DWORD;


// BOOL MonthCal_SetRange(HWND hmc, DWORD gdtr, LPSYSTEMTIME rgst)
//   if GDTR_MIN, sets the minimum ALLOWABLE systemtime to rgst[0], otherwise removes minimum
//   if GDTR_MAX, sets the maximum ALLOWABLE systemtime to rgst[1], otherwise removes maximum
//   returns TRUE on success, FALSE on error (such as invalid parameters)

CONST
         MCM_SETRANGE                   = (MCM_FIRST + 18);
// Macro 270


Function MonthCal_SetRange( hmc : HWND ; gd : DWORD; rgst : LPSYSTEMTIME):BOOL;


// int MonthCal_GetMonthDelta(HWND hmc)
//   returns the number of months one click on a next/prev button moves by

CONST
         MCM_GETMONTHDELTA              = (MCM_FIRST + 19);
// Macro 271


Function MonthCal_GetMonthDelta( hmc :hwnd ):cint;


// int MonthCal_SetMonthDelta(HWND hmc, int n)
//   sets the month delta to n. n==0 reverts to moving by a page of months
//   returns the previous value of n.

CONST
         MCM_SETMONTHDELTA              = (MCM_FIRST + 20);
// Macro 272


Function MonthCal_SetMonthDelta( hmc :hwnd ; n :cint ):cint;


// DWORD MonthCal_GetMaxTodayWidth(HWND hmc, LPSIZE psz)
//   sets *psz to the maximum width/height of the "Today" string displayed
//   at the bottom of the calendar (as long as MCS_NOTODAY is not specified)

CONST
         MCM_GETMAXTODAYWIDTH           = (MCM_FIRST + 21);
// Macro 273


Function MonthCal_GetMaxTodayWidth( hmc :hwnd ):DWORD;


{$ifdef ie4plus}

CONST
         MCM_SETUNICODEFORMAT           = CCM_SETUNICODEFORMAT;
// Macro 274


Function MonthCal_SetUnicodeFormat( hwnd : hwnd; fUnicode : bool):BOOL;


CONST
         MCM_GETUNICODEFORMAT           = CCM_GETUNICODEFORMAT;
// Macro 275


Function MonthCal_GetUnicodeFormat( hwnd : hwnd):BOOL;

{$ifdef NTDDI_VISTA}
Const
// View
         MCMV_MONTH      = 0;
         MCMV_YEAR       = 1;
         MCMV_DECADE     = 2;
         MCMV_CENTURY    = 3;
         MCMV_MAX        = MCMV_CENTURY;

         MCM_GETCURRENTVIEW 		= (MCM_FIRST + 22);
         MCM_GETCALENDARCOUNT 		= (MCM_FIRST + 23);
         MCM_GETCALENDARGRIDINFO 	= (MCM_FIRST + 24);
         MCM_GETCALID 			= (MCM_FIRST + 27);
         MCM_SETCALID 			= (MCM_FIRST + 28);
// Returns the min rect that will fit the max number of calendars for the passed in rect.
         MCM_SIZERECTTOMIN 		= (MCM_FIRST + 29);
         MCM_SETCALENDARBORDER 		= (MCM_FIRST + 30);
         MCM_GETCALENDARBORDER 		= (MCM_FIRST + 31);
         MCM_SETCURRENTVIEW 		= (MCM_FIRST + 32);

// Part
         MCGIP_CALENDARCONTROL      = 0;
         MCGIP_NEXT                 = 1;
         MCGIP_PREV                 = 2;
         MCGIP_FOOTER               = 3;
         MCGIP_CALENDAR             = 4;
         MCGIP_CALENDARHEADER       = 5;
         MCGIP_CALENDARBODY         = 6;
         MCGIP_CALENDARROW          = 7;
         MCGIP_CALENDARCELL         = 8;

         MCGIF_DATE                 = $00000001;
         MCGIF_RECT                 = $00000002;
         MCGIF_NAME                 = $00000004;

// Note: iRow of -1 refers to the row header and iCol of -1 refers to the col header.

Type
 tagMCGRIDINFO = record
          cbSize : UINT;
          dwPart : DWORD;
          dwFlags : DWORD;
          iCalendar : longint;
          iRow : longint;
          iCol : longint;
          bSelected : BOOL;
          stStart : SYSTEMTIME;
          stEnd : SYSTEMTIME;
          rc : RECT;
          pszName : LPWSTR;
          cchName : size_t;
       end;
     MCGRIDINFO   = tagMCGRIDINFO;
     TPMCGRIDINFO = MCGRIDINFO;
     PMCGRIDINFO  = ^MCGRIDINFO;
     LPMCGRIDINFO = PMCGRIDINFO;

function MonthCal_GetCurrentView(hmc:HWND):DWORD;
//        (DWORD)SNDMSG(hmc, MCM_GETCURRENTVIEW, 0, 0)

function MonthCal_GetCalendarCount(hmc:HWND):DWORD;
//        (DWORD)SNDMSG(hmc, MCM_GETCALENDARCOUNT, 0, 0)

function MonthCal_GetCalendarGridInfo(hmc:HWND; pmc:pMCGRIDINFO):BOOL;
//        (BOOL)SNDMSG(hmc, MCM_GETCALENDARGRIDINFO, 0, (LPARAM)(PMCGRIDINFO)(pmcGridInfo))

function MonthCal_GetCALID(hmc:HWND):CALID;
//        (CALID)SNDMSG(hmc, MCM_GETCALID, 0, 0)

function MonthCal_SetCALID(hmc:HWND; calid:cuint):LRESULT;
//        SNDMSG(hmc, MCM_SETCALID, (WPARAM)(calid), 0)

function MonthCal_SizeRectToMin(hmc:HWND; prc:prect):LRESULT;
//        SNDMSG(hmc, MCM_SIZERECTTOMIN, 0, (LPARAM)(prc))

function MonthCal_SetCalendarBorder(hmc:HWND; fset:bool; xyborder:cint):LRESULT;
//        SNDMSG(hmc, MCM_SETCALENDARBORDER, (WPARAM)(fset), (LPARAM)(xyborder))

function MonthCal_GetCalendarBorder(hmc:HWND):cint;
//        (int)SNDMSG(hmc, MCM_GETCALENDARBORDER, 0, 0)

function MonthCal_SetCurrentView(hmc:HWND; dwNewView:DWord):BOOL;
//        (BOOL)SNDMSG(hmc, MCM_SETCURRENTVIEW, 0, (LPARAM)(dwNewView))


{$endif}

{$ENDIF}

// MCN_SELCHANGE is sent whenever the currently displayed date changes
// via month change, year change, keyboard navigation, prev/next button
//
TYPE

         tagNMSELCHANGE       = Record
                                 nmhdr        : NMHDR;          // this must be first, so we don't break WM_NOTIFY
                                 stSelStart   : SYSTEMTIME;
                                 stSelEnd     : SYSTEMTIME;
                                 END;
         NMSELCHANGE          = tagNMSELCHANGE;
         LPNMSELCHANGE        = ^tagNMSELCHANGE;
         TNMSELCHANGE         = tagNMSELCHANGE;
         PNMSELCHANGE         = ^tagNMSELCHANGE;



CONST
         MCN_SELCHANGE                  = (MCN_FIRST + 1);

// MCN_GETDAYSTATE is sent for MCS_DAYSTATE controls whenever new daystate
// information is needed (month or year scroll) to draw bolding information.
// The app must fill in cDayState months worth of information starting from
// stStart date. The app may fill in the array at prgDayState or change
// prgDayState to point to a different array out of which the information
// will be copied. (similar to tooltips)
//
TYPE

         tagNMDAYSTATE        = Record
                                 nmhdr        : NMHDR;          // this must be first, so we don't break WM_NOTIFY
                                 stStart      : SYSTEMTIME;
                                 cDayState    : cint;
                                 prgDayState  : LPMONTHDAYSTATE;          // points to cDayState MONTHDAYSTATEs
                                 END;
         NMDAYSTATE           = tagNMDAYSTATE;
         LPNMDAYSTATE         = ^tagNMDAYSTATE;
         TNMDAYSTATE          = tagNMDAYSTATE;
         PNMDAYSTATE          = ^tagNMDAYSTATE;



CONST
         MCN_GETDAYSTATE                = (MCN_FIRST + 3);

// MCN_SELECT is sent whenever a selection has occured (via mouse or keyboard)
//
TYPE

         NMSELECT             = NMSELCHANGE;
         LPNMSELECT           = ^NMSELCHANGE;


CONST
         MCN_SELECT                     = (MCN_FIRST + 4);


// begin_r_commctrl

         MCS_DAYSTATE                   = $0001;
         MCS_MULTISELECT                = $0002;
         MCS_WEEKNUMBERS                = $0004;
{$ifdef ie4plus}
         MCS_NOTODAYCIRCLE              = $0008;
         MCS_NOTODAY                    = $0010;
{$ELSE}
         MCS_NOTODAY                    = $0008;
{$ENDIF}
{$ifdef NTDDI_Vista}
	 MCS_NOTRAILINGDATES  		= $0040;
	 MCS_SHORTDAYSOFWEEK  		= $0080;
	 MCS_NOSELCHANGEONNAV 		= $0100;
{$endif}

         GMR_VISIBLE                    = 0;                  // visible portion of display
         GMR_DAYSTATE                   = 1;                  // above plus the grayed out parts of
                                // partially displayed months


{$ENDIF} // _WIN32
{$ENDIF} // NOMONTHCAL


//====== DATETIMEPICK CONTROL ==================================================

{$IFNDEF NODATETIMEPICK}
{$IFDEF _WIN32}

         DATETIMEPICK_CLASSW            = {L}'SysDateTimePick32';
         DATETIMEPICK_CLASSA            = 'SysDateTimePick32';

{$IFDEF UNICODE}
         DATETIMEPICK_CLASS  = DATETIMEPICK_CLASSW;
{$ELSE}
         DATETIMEPICK_CLASS  = DATETIMEPICK_CLASSA;
{$ENDIF}

{$ifdef NTDDI_VISTA}
Type
	tagDATETIMEPICKERINFO = packed record
          cbSize : DWORD;
          rcCheck : RECT;
          stateCheck : DWORD;
          rcButton : RECT;
          stateButton : DWORD;
          hwndEdit : HWND;
          hwndUD : HWND;
          hwndDropDown : HWND;
       end;
     DATETIMEPICKERINFO   = tagDATETIMEPICKERINFO;
     PDATETIMEPICKERINFO  = ^DATETIMEPICKERINFO;
     LPDATETIMEPICKERINFO = PDATETIMEPICKERINFO;
     TDATETIMEPICKERINFO  = DATETIMEPICKERINFO;
{$endif}

CONST
         DTM_FIRST                      = $1000;

// DWORD DateTimePick_GetSystemtime(HWND hdp, LPSYSTEMTIME pst)
//   returns GDT_NONE if "none" is selected (DTS_SHOWNONE only)
//   returns GDT_VALID and modifies *pst to be the currently selected value
         DTM_GETSYSTEMTIME              = (DTM_FIRST + 1);
// Macro 276

// #define DateTime_GetSystemtime(hdp, pst)    (DWORD)SNDMSG(hdp, DTM_GETSYSTEMTIME, 0, (LPARAM)(pst))

// BOOL DateTime_SetSystemtime(HWND hdp, DWORD gd, LPSYSTEMTIME pst)
//   if gd==GDT_NONE, sets datetimepick to None (DTS_SHOWNONE only)
//   if gd==GDT_VALID, sets datetimepick to *pst
//   returns TRUE on success, FALSE on error (such as bad params)

CONST
         DTM_SETSYSTEMTIME              = (DTM_FIRST + 2);
// Macro 277

// #define DateTime_SetSystemtime(hdp, gd, pst)    (BOOL)SNDMSG(hdp, DTM_SETSYSTEMTIME, (WPARAM)(gd), (LPARAM)(pst))
function DateTime_SetSystemTime(hdp: HWND; gd: DWORD; const pst: TSystemTime): BOOL;inline;

// DWORD DateTime_GetRange(HWND hdp, LPSYSTEMTIME rgst)
//   modifies rgst[0] to be the minimum ALLOWABLE systemtime (or 0 if no minimum)
//   modifies rgst[1] to be the maximum ALLOWABLE systemtime (or 0 if no maximum)
//   returns GDTR_MIN|GDTR_MAX if there is a minimum|maximum limit

CONST
         DTM_GETRANGE                   = (DTM_FIRST + 3);
// Macro 278

// #define DateTime_GetRange(hdp, rgst)  (DWORD)SNDMSG(hdp, DTM_GETRANGE, 0, (LPARAM)(rgst))

// BOOL DateTime_SetRange(HWND hdp, DWORD gdtr, LPSYSTEMTIME rgst)
//   if GDTR_MIN, sets the minimum ALLOWABLE systemtime to rgst[0], otherwise removes minimum
//   if GDTR_MAX, sets the maximum ALLOWABLE systemtime to rgst[1], otherwise removes maximum
//   returns TRUE on success, FALSE on error (such as invalid parameters)

CONST
         DTM_SETRANGE                   = (DTM_FIRST + 4);
// Macro 279

// #define DateTime_SetRange(hdp, gd, rgst)  (BOOL)SNDMSG(hdp, DTM_SETRANGE, (WPARAM)(gd), (LPARAM)(rgst))
function DateTime_SetRange(hdp: HWND; gdtr: DWORD; rgst: PSystemTime): BOOL;inline;

// BOOL DateTime_SetFormat(HWND hdp, LPCTSTR sz)
//   sets the display formatting string to sz (see GetDateFormat and GetTimeFormat for valid formatting chars)
//   NOTE: 'X' is a valid formatting character which indicates that the application
//   will determine how to display information. Such apps must support DTN_WMKEYDOWN,
//   DTN_FORMAT, and DTN_FORMATQUERY.

CONST
         DTM_SETFORMATA                 = (DTM_FIRST + 5);
         DTM_SETFORMATW                 = (DTM_FIRST + 50);

{$IFDEF UNICODE}

TYPE
         DTM_SETFORMAT       = DTM_SETFORMATW;
{$ELSE}
         DTM_SETFORMAT       = DTM_SETFORMATA;
{$ENDIF}

// Macro 280

// #define DateTime_SetFormat(hdp, sz)  (BOOL)SNDMSG(hdp, DTM_SETFORMAT, 0, (LPARAM)(sz))



CONST
         DTM_SETMCCOLOR                 = (DTM_FIRST + 6);
// Macro 281

// #define DateTime_SetMonthCalColor(hdp, iColor, clr) SNDMSG(hdp, DTM_SETMCCOLOR, iColor, clr)
function DateTime_SetMonthCalColor(hdp: HWND; iColor: DWORD; clr: TColorRef): TColorRef;inline;

CONST
         DTM_GETMCCOLOR                 = (DTM_FIRST + 7);
// Macro 282

// #define DateTime_GetMonthCalColor(hdp, iColor) SNDMSG(hdp, DTM_GETMCCOLOR, iColor, 0)

// HWND DateTime_GetMonthCal(HWND hdp)
//   returns the HWND of the MonthCal popup window. Only valid
// between DTN_DROPDOWN and DTN_CLOSEUP notifications.

CONST
         DTM_GETMONTHCAL                = (DTM_FIRST + 8);
// Macro 283

// #define DateTime_GetMonthCal(hdp) (HWND)SNDMSG(hdp, DTM_GETMONTHCAL, 0, 0)
function DateTime_GetMonthCal(hdp: HWND): HWND;inline;

{$ifdef ie4plus}


CONST
         DTM_SETMCFONT                  = (DTM_FIRST + 9);

// Macro 284
procedure DateTime_SetMonthCalFont(hdp:HWND; hfont:HFONT; fRedraw:LPARAM);
//  SNDMSG(hdp, DTM_SETMCFONT, (WPARAM)(hfont), (LPARAM)(fRedraw))


CONST
         DTM_GETMCFONT                  = (DTM_FIRST + 10);

// Macro 285
function DateTime_GetMonthCalFont(hdp:HWND):HFONT;
// SNDMSG(hdp, DTM_GETMCFONT, 0, 0)

{$ifdef NTDDI_VISTA}
Const
	 DTM_SETMCSTYLE    		= (DTM_FIRST + 11);
	 DTM_GETMCSTYLE    		= (DTM_FIRST + 12);
	 DTM_CLOSEMONTHCAL 		= (DTM_FIRST + 13);
	 DTM_GETDATETIMEPICKERINFO 	= (DTM_FIRST + 14);
	 DTM_GETIDEALSIZE 		= (DTM_FIRST + 15);

function DateTime_SetMonthCalStyle(hdp:HWND; dwStyle:DWord):LResult;
// SNDMSG(hdp, DTM_SETMCSTYLE, 0, (LPARAM)dwStyle)

function DateTime_GetMonthCalStyle(hdp:HWND):LRESULT;
//  SNDMSG(hdp, DTM_GETMCSTYLE, 0, 0)

function DateTime_CloseMonthCal(hdp:HWND):LRESULT;
//  SNDMSG(hdp, DTM_CLOSEMONTHCAL, 0, 0)

// DateTime_GetDateTimePickerInfo(HWND hdp, DATETIMEPICKERINFO* pdtpi)
// Retrieves information about the selected date time picker.

function DateTime_GetDateTimePickerInfo(hdp:HWND; pdtpi:PDATETIMEPICKERINFO):LRESULT;
// SNDMSG(hdp, DTM_GETDATETIMEPICKERINFO, 0, (LPARAM)(pdtpi))

function DateTime_GetIdealSize(hdp:HWND; ps:PSIZE): LResult;
// (BOOL)SNDMSG((hdp), DTM_GETIDEALSIZE, 0, (LPARAM)(psize))

{$endif}


{$ENDIF}      // _WIN32_IE >= 0x0400

// begin_r_commctrl


CONST
         DTS_UPDOWN                     = $0001;              // use UPDOWN instead of MONTHCAL
         DTS_SHOWNONE                   = $0002;              // allow a NONE selection
         DTS_SHORTDATEFORMAT            = $0000;              // use the short date format (app must forward WM_WININICHANGE messages)
         DTS_LONGDATEFORMAT             = $0004;              // use the long date format (app must forward WM_WININICHANGE messages)
{$ifdef ie5plus}
         DTS_SHORTDATECENTURYFORMAT     = $000C;              // short date format with century (app must forward WM_WININICHANGE messages)
{$ENDIF} // (_WIN32_IE >= 0x500)
         DTS_TIMEFORMAT                 = $0009;              // use the time format (app must forward WM_WININICHANGE messages)
         DTS_APPCANPARSE                = $0010;              // allow user entered strings (app MUST respond to DTN_USERSTRING)
         DTS_RIGHTALIGN                 = $0020;              // right-align popup instead of left-align it

// end_r_commctrl

         DTN_DATETIMECHANGE             = (DTN_FIRST + 1);    // the systemtime has changed
TYPE

         tagNMDATETIMECHANGE  = Record
                                 nmhdr        : NMHDR;
                                 dwFlags      : DWORD;          // GDT_VALID or GDT_NONE
                                 st           : SYSTEMTIME;          // valid iff dwFlags==GDT_VALID
                                 END;
         NMDATETIMECHANGE     = tagNMDATETIMECHANGE;
         LPNMDATETIMECHANGE   = ^tagNMDATETIMECHANGE;
         TNMDATETIMECHANGE    = tagNMDATETIMECHANGE;
         PNMDATETIMECHANGE    = ^tagNMDATETIMECHANGE;



CONST
         DTN_USERSTRINGA                = (DTN_FIRST + 2);    // the user has entered a string
         DTN_USERSTRINGW                = (DTN_FIRST + 15);
TYPE

         tagNMDATETIMESTRINGA = Record
                                 nmhdr        : NMHDR;
                                 pszUserString : LPCSTR;          // string user entered
                                 st           : SYSTEMTIME;          // app fills this in
                                 dwFlags      : DWORD;          // GDT_VALID or GDT_NONE
                                 END;
         NMDATETIMESTRINGA    = tagNMDATETIMESTRINGA;
         LPNMDATETIMESTRINGA  = ^tagNMDATETIMESTRINGA;
         TNMDATETIMESTRINGA   = tagNMDATETIMESTRINGA;
         PNMDATETIMESTRINGA   = ^tagNMDATETIMESTRINGA;


         tagNMDATETIMESTRINGW = Record
                                 nmhdr        : NMHDR;
                                 pszUserString : LPCWSTR;          // string user entered
                                 st           : SYSTEMTIME;          // app fills this in
                                 dwFlags      : DWORD;          // GDT_VALID or GDT_NONE
                                 END;
         NMDATETIMESTRINGW    = tagNMDATETIMESTRINGW;
         LPNMDATETIMESTRINGW  = ^tagNMDATETIMESTRINGW;
         TNMDATETIMESTRINGW   = tagNMDATETIMESTRINGW;
         PNMDATETIMESTRINGW   = ^tagNMDATETIMESTRINGW;


{$IFDEF UNICODE}
CONST
         DTN_USERSTRING      = DTN_USERSTRINGW;
TYPE
         NMDATETIMESTRING    = NMDATETIMESTRINGW;
         LPNMDATETIMESTRING  = LPNMDATETIMESTRINGW;
{$ELSE}
CONST
         DTN_USERSTRING      = DTN_USERSTRINGA;
TYPE
         NMDATETIMESTRING    = NMDATETIMESTRINGA;
         LPNMDATETIMESTRING  = LPNMDATETIMESTRINGA;
{$ENDIF}
         TNMDateTimeString   = NMDATETIMESTRING;
         PNMDateTimeString   = LPNMDATETIMESTRING;



CONST
         DTN_WMKEYDOWNA                 = (DTN_FIRST + 3);    // modify keydown on app format field (X)
         DTN_WMKEYDOWNW                 = (DTN_FIRST + 16);
TYPE

         tagNMDATETIMEWMKEYDOWNA = Record
                                    nmhdr        : NMHDR;
                                    nVirtKey     : cint;          // virtual key code of WM_KEYDOWN which MODIFIES an X field
                                    pszFormat    : LPCSTR;          // format substring
                                    st           : SYSTEMTIME;          // current systemtime, app should modify based on key
                                    END;
         NMDATETIMEWMKEYDOWNA = tagNMDATETIMEWMKEYDOWNA;
         LPNMDATETIMEWMKEYDOWNA = ^tagNMDATETIMEWMKEYDOWNA;
         TNMDATETIMEWMKEYDOWNA = tagNMDATETIMEWMKEYDOWNA;
         PNMDATETIMEWMKEYDOWNA = ^tagNMDATETIMEWMKEYDOWNA;


         tagNMDATETIMEWMKEYDOWNW = Record
                                    nmhdr        : NMHDR;
                                    nVirtKey     : cint;          // virtual key code of WM_KEYDOWN which MODIFIES an X field
                                    pszFormat    : LPCWSTR;          // format substring
                                    st           : SYSTEMTIME;          // current systemtime, app should modify based on key
                                    END;
         NMDATETIMEWMKEYDOWNW = tagNMDATETIMEWMKEYDOWNW;
         LPNMDATETIMEWMKEYDOWNW = ^tagNMDATETIMEWMKEYDOWNW;
         TNMDATETIMEWMKEYDOWNW = tagNMDATETIMEWMKEYDOWNW;
         PNMDATETIMEWMKEYDOWNW = ^tagNMDATETIMEWMKEYDOWNW;


{$IFDEF UNICODE}
CONST
         DTN_WMKEYDOWN       = DTN_WMKEYDOWNW;
TYPE
         NMDATETIMEWMKEYDOWN = NMDATETIMEWMKEYDOWNW;
         LPNMDATETIMEWMKEYDOWN= LPNMDATETIMEWMKEYDOWNW;
{$ELSE}
CONST
         DTN_WMKEYDOWN       = DTN_WMKEYDOWNA;
TYPE
         NMDATETIMEWMKEYDOWN = NMDATETIMEWMKEYDOWNA;
         LPNMDATETIMEWMKEYDOWN= LPNMDATETIMEWMKEYDOWNA;
{$ENDIF}



CONST
         DTN_FORMATA                    = (DTN_FIRST + 4);    // query display for app format field (X)
         DTN_FORMATW                    = (DTN_FIRST + 17);
TYPE

         tagNMDATETIMEFORMATA = Record
                                 nmhdr        : NMHDR;
                                 pszFormat    : LPCSTR;          // format substring
                                 st           : SYSTEMTIME;      // current systemtime
                                 pszDisplay   : LPCSTR;          // string to display
                                 szDisplay    : Array [0..63] OF CHAR;          // buffer pszDisplay originally points at
                                 END;
         NMDATETIMEFORMATA    = tagNMDATETIMEFORMATA;
         LPNMDATETIMEFORMATA  = ^tagNMDATETIMEFORMATA;
         TNMDATETIMEFORMATA   = tagNMDATETIMEFORMATA;
         PNMDATETIMEFORMATA   = ^tagNMDATETIMEFORMATA;


         tagNMDATETIMEFORMATW = Record
                                 nmhdr        : NMHDR;
                                 pszFormat    : LPCWSTR;          // format substring
                                 st           : SYSTEMTIME;       // current systemtime
                                 pszDisplay   : LPCWSTR;          // string to display
                                 szDisplay    : Array [0..63] OF WCHAR;          // buffer pszDisplay originally points at
                                 END;
         NMDATETIMEFORMATW    = tagNMDATETIMEFORMATW;
         LPNMDATETIMEFORMATW  = ^tagNMDATETIMEFORMATW;
         TNMDATETIMEFORMATW   = tagNMDATETIMEFORMATW;
         PNMDATETIMEFORMATW   = ^tagNMDATETIMEFORMATW;


{$IFDEF UNICODE}
CONST
         DTN_FORMAT          = DTN_FORMATW;
TYPE
         NMDATETIMEFORMAT    = NMDATETIMEFORMATW;
         LPNMDATETIMEFORMAT  = LPNMDATETIMEFORMATW;
{$ELSE}
CONST
         DTN_FORMAT          = DTN_FORMATA;
TYPE
         NMDATETIMEFORMAT    = NMDATETIMEFORMATA;
         LPNMDATETIMEFORMAT  = LPNMDATETIMEFORMATA;
{$ENDIF}



CONST
         DTN_FORMATQUERYA               = (DTN_FIRST + 5);    // query formatting info for app format field (X)
         DTN_FORMATQUERYW               = (DTN_FIRST + 18);
TYPE

         tagNMDATETIMEFORMATQUERYA = Record
                                      nmhdr        : NMHDR;
                                      pszFormat    : LPCSTR;          // format substring
                                      szMax        : SIZE;          // max bounding rectangle app will use for this format string
                                      END;
         NMDATETIMEFORMATQUERYA = tagNMDATETIMEFORMATQUERYA;
         LPNMDATETIMEFORMATQUERYA = ^tagNMDATETIMEFORMATQUERYA;
         TNMDATETIMEFORMATQUERYA = tagNMDATETIMEFORMATQUERYA;
         PNMDATETIMEFORMATQUERYA = ^tagNMDATETIMEFORMATQUERYA;


         tagNMDATETIMEFORMATQUERYW = Record
                                      nmhdr        : NMHDR;
                                      pszFormat    : LPCWSTR;          // format substring
                                      szMax        : SIZE;          // max bounding rectangle app will use for this format string
                                      END;
         NMDATETIMEFORMATQUERYW = tagNMDATETIMEFORMATQUERYW;
         LPNMDATETIMEFORMATQUERYW = ^tagNMDATETIMEFORMATQUERYW;
         TNMDATETIMEFORMATQUERYW = tagNMDATETIMEFORMATQUERYW;
         PNMDATETIMEFORMATQUERYW = ^tagNMDATETIMEFORMATQUERYW;


{$IFDEF UNICODE}
CONST
         DTN_FORMATQUERY     = DTN_FORMATQUERYW;
TYPE
         NMDATETIMEFORMATQUERY= NMDATETIMEFORMATQUERYW;
         LPNMDATETIMEFORMATQUERY= LPNMDATETIMEFORMATQUERYW;
{$ELSE}
CONST
         DTN_FORMATQUERY     = DTN_FORMATQUERYA;
TYPE
         NMDATETIMEFORMATQUERY= NMDATETIMEFORMATQUERYA;
         LPNMDATETIMEFORMATQUERY= LPNMDATETIMEFORMATQUERYA;
{$ENDIF}



CONST
         DTN_DROPDOWN                   = (DTN_FIRST + 6);    // MonthCal has dropped down
         DTN_CLOSEUP                    = (DTN_FIRST + 7);    // MonthCal is popping up


         GDTR_MIN                       = $0001;
         GDTR_MAX                       = $0002;

         GDT_ERROR                      = -1;
         GDT_VALID                      = 0;
         GDT_NONE                       = 1;


{$ENDIF} // _WIN32
{$ENDIF} // NODATETIMEPICK


{$ifdef ie4plus}

{$IFNDEF NOIPADDRESS}

///////////////////////////////////////////////
///    IP Address edit control

// Messages sent to IPAddress controls

         IPM_CLEARADDRESS               = (WM_USER+100);      // no parameters
         IPM_SETADDRESS                 = (WM_USER+101);      // lparam = TCP/IP address
         IPM_GETADDRESS                 = (WM_USER+102);      // lresult = # of non black fields.  lparam = LPDWORD for TCP/IP address
         IPM_SETRANGE                   = (WM_USER+103);      // wparam = field, lparam = range
         IPM_SETFOCUS                   = (WM_USER+104);      // wparam = field
         IPM_ISBLANK                    = (WM_USER+105);      // no parameters

         WC_IPADDRESSW                  = {L}'SysIPAddress32';
         WC_IPADDRESSA                  = 'SysIPAddress32';

{$IFDEF UNICODE}

TYPE
         WC_IPADDRESS        = WC_IPADDRESSW;
{$ELSE}
         WC_IPADDRESS        = WC_IPADDRESSA;
{$ENDIF}


CONST
         IPN_FIELDCHANGED               = (IPN_FIRST - 0);
TYPE

         tagNMIPADDRESS       = Record
                                 hdr          : NMHDR;
                                 iField       : cint;
                                 iValue       : cint;
                                 END;
         NMIPADDRESS          = tagNMIPADDRESS;
         LPNMIPADDRESS        = ^tagNMIPADDRESS;
         TNMIPADDRESS         = tagNMIPADDRESS;
         PNMIPADDRESS         = ^tagNMIPADDRESS;


// The following is a useful macro for passing the range values in the
// IPM_SETRANGE message.

// Macro 286

// #define MAKEIPRANGE(low, high)    ((LPARAM)(WORD)(((BYTE)(high) << 8) + (BYTE)(low)))

// And this is a useful macro for making the IP Address to be passed
// as a LPARAM.

// Macro 287

// #define MAKEIPADDRESS(b1,b2,b3,b4)  ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

// Get individual number
// Macro 288

// #define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
// Macro 289

// #define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
// Macro 290

// #define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
// Macro 291

// #define FOURTH_IPADDRESS(x) (x & 0xff)


{$ENDIF} // NOIPADDRESS


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
///  ====================== Pager Control =============================
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

{$IFNDEF NOPAGESCROLLER}

//Pager Class Name

CONST
         WC_PAGESCROLLERW               = {L}'SysPager';
         WC_PAGESCROLLERA               = 'SysPager';

{$IFDEF UNICODE}

TYPE
         WC_PAGESCROLLER     = WC_PAGESCROLLERW;
{$ELSE}
         WC_PAGESCROLLER     = WC_PAGESCROLLERA;
{$ENDIF}


//---------------------------------------------------------------------------------------
// Pager Control Styles
//---------------------------------------------------------------------------------------
// begin_r_commctrl


CONST
         PGS_VERT                       = $00000000;
         PGS_HORZ                       = $00000001;
         PGS_AUTOSCROLL                 = $00000002;
         PGS_DRAGNDROP                  = $00000004;

// end_r_commctrl


//---------------------------------------------------------------------------------------
// Pager Button State
//---------------------------------------------------------------------------------------
//The scroll can be in one of the following control State
         PGF_INVISIBLE                  = 0;                  // Scroll button is not visible
         PGF_NORMAL                     = 1;                  // Scroll button is in normal state
         PGF_GRAYED                     = 2;                  // Scroll button is in grayed state
         PGF_DEPRESSED                  = 4;                  // Scroll button is in depressed state
         PGF_HOT                        = 8;                  // Scroll button is in hot state


// The following identifiers specifies the button control
         PGB_TOPORLEFT                  = 0;
         PGB_BOTTOMORRIGHT              = 1;

//---------------------------------------------------------------------------------------
// Pager Control  Messages
//---------------------------------------------------------------------------------------
         PGM_SETCHILD                   = (PGM_FIRST + 1);    // lParam == hwnd
// Macro 292


Procedure Pager_SetChild( hwnd : hwnd; hwndChild : LPARAM);



CONST
         PGM_RECALCSIZE                 = (PGM_FIRST + 2);
// Macro 293


Procedure Pager_RecalcSize( hwnd : hwnd);



CONST
         PGM_FORWARDMOUSE               = (PGM_FIRST + 3);
// Macro 294


Procedure Pager_ForwardMouse( hwnd : hwnd; bForward : WPARAM);



CONST
         PGM_SETBKCOLOR                 = (PGM_FIRST + 4);
// Macro 295


Function Pager_SetBkColor( hwnd : hwnd; clr : LPARAM):COLORREF;



CONST
         PGM_GETBKCOLOR                 = (PGM_FIRST + 5);
// Macro 296


Function Pager_GetBkColor( hwnd : hwnd):COLORREF;



CONST
         PGM_SETBORDER                  = (PGM_FIRST + 6);
// Macro 297


Function Pager_SetBorder( hwnd : hwnd; iBorder : LPARAM):cint;



CONST
         PGM_GETBORDER                  = (PGM_FIRST + 7);
// Macro 298


Function Pager_GetBorder( hwnd : hwnd):cint;



CONST
         PGM_SETPOS                     = (PGM_FIRST + 8);
// Macro 299


Function Pager_SetPos( hwnd : hwnd; iPos : LPARAM):cint;



CONST
         PGM_GETPOS                     = (PGM_FIRST + 9);
// Macro 300


Function Pager_GetPos( hwnd : hwnd):cint;



CONST
         PGM_SETBUTTONSIZE              = (PGM_FIRST + 10);
// Macro 301


Function Pager_SetButtonSize( hwnd : hwnd; iSize : LPARAM):cint;



CONST
         PGM_GETBUTTONSIZE              = (PGM_FIRST + 11);
// Macro 302


Function Pager_GetButtonSize( hwnd : hwnd):cint;



CONST
         PGM_GETBUTTONSTATE             = (PGM_FIRST + 12);
// Macro 303


Function Pager_GetButtonState( hwnd : hwnd; iButton : LPARAM):DWORD;



CONST
         PGM_GETDROPTARGET              = CCM_GETDROPTARGET;
// Macro 304


Procedure Pager_GetDropTarget( hwnd : hwnd; ppdt : LPARAM);

//---------------------------------------------------------------------------------------
//Pager Control Notification Messages
//---------------------------------------------------------------------------------------


// PGN_SCROLL Notification Message


CONST
         PGN_SCROLL                     = (PGN_FIRST-1);

         PGF_SCROLLUP                   = 1;
         PGF_SCROLLDOWN                 = 2;
         PGF_SCROLLLEFT                 = 4;
         PGF_SCROLLRIGHT                = 8;


//Keys down
         PGK_SHIFT                      = 1;
         PGK_CONTROL                    = 2;
         PGK_MENU                       = 4;


{$IFDEF _WIN32}
// #include <pshpack1.h>
{$ENDIF}

// This structure is sent along with PGN_SCROLL notifications
TYPE

         DummyStruct17        = Record
                                 hdr          : NMHDR;
                                 fwKeys       : WORD;          // Specifies which keys are down when this notification is send
                                 rcParent     : RECT;          // Contains Parent Window Rect
                                 iDir         : cint;          // Scrolling Direction
                                 iXpos        : cint;          // Horizontal scroll position
                                 iYpos        : cint;          // Vertical scroll position
                                 iScroll      : cint;          // [in/out] Amount to scroll
                                 END;
         NMPGSCROLL           = DummyStruct17;
         LPNMPGSCROLL         = ^DummyStruct17;
         TNMPGScroll          = NMPGSCROLL;
         PNMPGScroll          = LPNMPGSCROLL;


{$IFDEF _WIN32}
// #include <poppack.h>
{$ENDIF}

// PGN_CALCSIZE Notification Message


CONST
         PGN_CALCSIZE                   = (PGN_FIRST-2);

         PGF_CALCWIDTH                  = 1;
         PGF_CALCHEIGHT                 = 2;

TYPE

         DummyStruct18        = Record
                                 hdr          : NMHDR;
                                 dwFlag       : DWORD;
                                 iWidth       : cint;
                                 iHeight      : cint;
                                 END;
         NMPGCALCSIZE         = DummyStruct18;
         LPNMPGCALCSIZE       = ^DummyStruct18;
         TNMPGCalcSize        = DummyStruct18;
         PNMPGCalcSize        = LPNMPGCALCSIZE;



// PGN_HOTITEMCHANGE Notification Message


CONST
         PGN_HOTITEMCHANGE              = (PGN_FIRST-3);

{
The PGN_HOTITEMCHANGE notification uses these notification
flags defined in TOOLBAR:

         HICF_ENTERING                  = $00000010;          // idOld is invalid
         HICF_LEAVING                   = $00000020;          // idNew is invalid
}

// Structure for PGN_HOTITEMCHANGE notification
//
TYPE

         tagNMPGHOTITEM       = Record
                                 hdr          : NMHDR;
                                 idOld        : cint;
                                 idNew        : cint;
                                 dwFlags      : DWORD;          // HICF_*
                                 END;
         NMPGHOTITEM          = tagNMPGHOTITEM;
         LPNMPGHOTITEM        = ^tagNMPGHOTITEM;
         TNMPGHOTITEM         = tagNMPGHOTITEM;
         PNMPGHOTITEM         = ^tagNMPGHOTITEM;


{$ENDIF} // NOPAGESCROLLER

////======================  End Pager Control ==========================================

//
// === Native Font Control ===
//
{$IFNDEF NONATIVEFONTCTL}
//NativeFont Class Name

CONST
         WC_NATIVEFONTCTLW              = {L}'NativeFontCtl';
         WC_NATIVEFONTCTLA              = 'NativeFontCtl';

{$IFDEF UNICODE}

TYPE
         WC_NATIVEFONTCTL    = WC_NATIVEFONTCTLW;
{$ELSE}
         WC_NATIVEFONTCTL    = WC_NATIVEFONTCTLA;
{$ENDIF}

// begin_r_commctrl

// style definition

CONST
         NFS_EDIT                       = $0001;
         NFS_STATIC                     = $0002;
         NFS_LISTCOMBO                  = $0004;
         NFS_BUTTON                     = $0008;
         NFS_ALL                        = $0010;
         NFS_USEFONTASSOC               = $0020;

// end_r_commctrl

{$ENDIF} // NONATIVEFONTCTL
// === End Native Font Control ===

/// ====================== Button Control =============================

{$IFNDEF NOBUTTON}

{$IFDEF _WIN32}

// Button Class Name
         WC_BUTTONA                     = 'Button';
         WC_BUTTONW                     = {L}'Button';

CONST
{$IFDEF UNICODE}
         WC_BUTTON           = WC_BUTTONW;
{$ELSE}
         WC_BUTTON           = WC_BUTTONA;
{$ENDIF}

{$ELSE}

         WC_BUTTON                      = 'Button';
{$ENDIF}

{$ifdef win32xp}
         BUTTON_IMAGELIST_ALIGN_LEFT    = 0;
         BUTTON_IMAGELIST_ALIGN_RIGHT   = 1;
         BUTTON_IMAGELIST_ALIGN_TOP     = 2;
         BUTTON_IMAGELIST_ALIGN_BOTTOM  = 3;
         BUTTON_IMAGELIST_ALIGN_CENTER  = 4;                  // Doesn't draw text

TYPE

         DummyStruct19        = Record
                                 himl         : HIMAGELIST;          // Index: Normal, hot pushed, disabled. If count is less than 4, we use index 1
                                 margin       : RECT;          // Margin around icon.
                                 uAlign       : UINT;
                                 END;
         BUTTON_IMAGELIST     = DummyStruct19;
         PBUTTON_IMAGELIST    = ^DummyStruct19;
         TDummyStruct19       = DummyStruct19;
         PDummyStruct19       = ^DummyStruct19;



CONST
         BCM_GETIDEALSIZE               = (BCM_FIRST + $0001);
// Macro 305


Function Button_GetIdealSize( hwnd : hwnd; psize : LPARAM):BOOL;



CONST
         BCM_SETIMAGELIST               = (BCM_FIRST + $0002);
// Macro 306


Function Button_SetImageList( hwnd : hwnd; pbuttonImagelist : LPARAM):BOOL;



CONST
         BCM_GETIMAGELIST               = (BCM_FIRST + $0003);
// Macro 307


Function Button_GetImageList( hwnd : hwnd; pbuttonImagelist : LPARAM):BOOL;



CONST
         BCM_SETTEXTMARGIN              = (BCM_FIRST + $0004);
// Macro 308


Function Button_SetTextMargin( hwnd : hwnd; pmargin : LPARAM):BOOL;


CONST
         BCM_GETTEXTMARGIN              = (BCM_FIRST + $0005);
// Macro 309


Function Button_GetTextMargin( hwnd : hwnd; pmargin : LPARAM):BOOL;


TYPE

         tagNMBCHOTITEM       = Record
                                 hdr          : NMHDR;
                                 dwFlags      : DWORD;          // HICF_*
                                 END;
         NMBCHOTITEM          = tagNMBCHOTITEM;
         LPNMBCHOTITEM        = ^tagNMBCHOTITEM;
         TNMBCHOTITEM         = tagNMBCHOTITEM;
         PNMBCHOTITEM         = ^tagNMBCHOTITEM;



CONST
         BCN_HOTITEMCHANGE              = (BCN_FIRST + $0001);

         BST_HOT                        = $0200;
{$ifdef win32vista}
         BST_DROPDOWNPUSHED      = $0400;

// BUTTON STYLES
         BS_SPLITBUTTON          = $00000000C;  // This block L suffixed (unsigned)
         BS_DEFSPLITBUTTON       = $00000000D;
         BS_COMMANDLINK          = $00000000E;
         BS_DEFCOMMANDLINK       = $00000000F;

// SPLIT BUTTON INFO mask flags
         BCSIF_GLYPH             = $00001;
         BCSIF_IMAGE             = $00002;
         BCSIF_STYLE             = $00004;
         BCSIF_SIZE              = $00008;

// SPLIT BUTTON STYLE flags
         BCSS_NOSPLIT            = $00001;
         BCSS_STRETCH            = $00002;
         BCSS_ALIGNLEFT          = $00004;
         BCSS_IMAGE              = $00008;

         BCM_SETDROPDOWNSTATE    = (BCM_FIRST + $0006);
         BCM_SETSPLITINFO        = (BCM_FIRST + $0007);
         BCM_GETSPLITINFO        = (BCM_FIRST + $0008);
         BCM_SETNOTE             = (BCM_FIRST + $0009);
         BCM_GETNOTE             = (BCM_FIRST + $000A);
         BCM_GETNOTELENGTH       = (BCM_FIRST + $000B);
         BCM_SETSHIELD           = (BCM_FIRST + $000C);


// Value to pass to BCM_SETIMAGELIST to indicate that no glyph should be
// displayed
	 BCCL_NOGLYPH  		 = HIMAGELIST(-1);

	 BCN_DROPDOWN            = (BCN_FIRST + $0002);
Type

     tagBUTTON_SPLITINFO =  record
			      mask : UINT;
          		      himlGlyph : HIMAGELIST;
          		      uSplitStyle : UINT;
          		      size : SIZE;
			      end;
     BUTTON_SPLITINFO   = tagBUTTON_SPLITINFO;
     PBUTTON_SPLITINFO  = ^BUTTON_SPLITINFO;
     LPBUTTON_SPLITINFO = PBUTTON_SPLITINFO;

  { NOTIFICATION MESSAGES }

     tagNMBCDROPDOWN = packed record
          hdr : NMHDR;
          rcButton : RECT;
       end;
     NMBCDROPDOWN    = tagNMBCDROPDOWN;
     PNMBCDROPDOWN   = ^NMBCDROPDOWN;
     LPNMBCDROPDOWN  = PNMBCDROPDOWN;

// BUTTON MESSAGES

function Button_SetDropDownState(hwnd:HWND; fDropDown:BOOL) : BOOL;
//    (BOOL)SNDMSG((hwnd), BCM_SETDROPDOWNSTATE, (WPARAM)(fDropDown), 0)

function Button_SetSplitInfo(hwnd:HWND; pInfo:PBUTTON_SPLITINFO):BOOL;
//    (BOOL)SNDMSG((hwnd), BCM_SETSPLITINFO, 0, (LPARAM)(pInfo))

function Button_GetSplitInfo(hwnd:HWND; pInfo:PBUTTON_SPLITINFO) :BOOL;
//    (BOOL)SNDMSG((hwnd), BCM_GETSPLITINFO, 0, (LPARAM)(pInfo))

function Button_SetNote(hwnd:HWND; psz:LPCWSTR) :BOOL;
//    (BOOL)SNDMSG((hwnd), BCM_SETNOTE, 0, (LPARAM)(psz))

function Button_GetNote(hwnd:HWND; psz:LPCWSTR; pcc:cint) :BOOL;
//    (BOOL)SNDMSG((hwnd), BCM_GETNOTE, (WPARAM)pcc, (LPARAM)psz)

function Button_GetNoteLength(hwnd:HWND) :LRESULT;
//    (LRESULT)SNDMSG((hwnd), BCM_GETNOTELENGTH, 0, 0)

// Macro to use on a button or command link to display an elevated icon

function Button_SetElevationRequiredState(hwnd:HWND; fRequired:BOOL) :LRESULT;
//    (LRESULT)SNDMSG((hwnd), BCM_SETSHIELD, 0, (LPARAM)fRequired)

{$endif}
{$ENDIF}
{$ENDIF} // NOBUTTON


/// =====================  End Button Control =========================

/// ====================== Static Control =============================

{$IFNDEF NOSTATIC}

const
{$IFDEF _WIN32}

// Static Class Name
         WC_STATICA                     = 'Static';
         WC_STATICW                     = {L}'Static';

{$IFDEF UNICODE}

TYPE
         WC_STATIC           = WC_STATICW;
{$ELSE}
         WC_STATIC           = WC_STATICA;
{$ENDIF}

{$ELSE}

CONST
         WC_STATIC                      = 'Static';
{$ENDIF}

{$ENDIF} // NOSTATIC

/// =====================  End Static Control =========================

/// ====================== Edit Control =============================

{$IFNDEF NOEDIT}

{$IFDEF _WIN32}

// Edit Class Name
         WC_EDITA                       = 'Edit';
         WC_EDITW                       = {L}'Edit';

{$IFDEF UNICODE}

TYPE
         WC_EDIT             = WC_EDITW;
{$ELSE}
         WC_EDIT             = WC_EDITA;
{$ENDIF}

{$ELSE}

CONST
         WC_EDIT                        = 'Edit';
{$ENDIF}

{$ifdef win32xp}
         EM_SETCUEBANNER                = (ECM_FIRST + 1);    // Set the cue banner with the lParm = LPCWSTR
// Macro 310


Function Edit_SetCueBannerText( hwnd : hwnd; lpcwText : LPARAM):BOOL;


CONST
         EM_GETCUEBANNER                = (ECM_FIRST + 2);    // Set the cue banner with the lParm = LPCWSTR
// Macro 311


Function Edit_GetCueBannerText( hwnd : hwnd; lpwText : WPARAM; cchText : LPARAM):BOOL;


TYPE

         _tagEDITBALLOONTIP   = Record
                                 cbStruct     : DWORD;
                                 pszTitle     : LPCWSTR;
                                 pszText      : LPCWSTR;
                                 ttiIcon      : cint;          // From TTI_*
                                 END;
         EDITBALLOONTIP       = _tagEDITBALLOONTIP;
         PEDITBALLOONTIP      = ^_tagEDITBALLOONTIP;
         TEDITBALLOONTIP      = _tagEDITBALLOONTIP;



CONST
         EM_SHOWBALLOONTIP              = (ECM_FIRST + 3);    // Show a balloon tip associated to the edit control
// Macro 312


Function Edit_ShowBalloonTip( hwnd : hwnd; peditballoontip : LPARAM):BOOL;


CONST
         EM_HIDEBALLOONTIP              = (ECM_FIRST + 4);    // Hide any balloon tip associated with the edit control
// Macro 313


Function Edit_HideBalloonTip( hwnd : hwnd):BOOL;

{$ifdef win32vista}
const
	 EM_SETHILITE        		= (ECM_FIRST + 5);
	 EM_GETHILITE        		= (ECM_FIRST + 6);

// both are "unimplemented" in MSDN, so probably typing is off.
procedure Edit_SetHilite(hwndCtl:hwnd; ichStart:Wparam; ichEnd:lparam);
// ((void)SNDMSG((hwndCtl), EM_SETHILITE, (ichStart), (ichEnd)))

function Edit_GetHilite(hwndCtl:hwnd):DWORD;
// ((DWORD)SNDMSG((hwndCtl), EM_GETHILITE, 0L, 0L))
{$endif}

{$ENDIF}

{$ENDIF} // NOEDIT

/// =====================  End Edit Control =========================

/// ====================== Listbox Control =============================

{$IFNDEF NOLISTBOX}

{$IFDEF _WIN32}

// Listbox Class Name

CONST
         WC_LISTBOXA                    = 'ListBox';
         WC_LISTBOXW                    = {L}'ListBox';

{$IFDEF UNICODE}

TYPE
         WC_LISTBOX          = WC_LISTBOXW;
{$ELSE}
         WC_LISTBOX          = WC_LISTBOXA;
{$ENDIF}

{$ELSE}

CONST
         WC_LISTBOX                     = 'ListBox';
{$ENDIF}

{$ENDIF} // NOLISTBOX


/// =====================  End Listbox Control =========================

/// ====================== Combobox Control =============================

{$IFNDEF NOCOMBOBOX}

{$IFDEF _WIN32}

// Combobox Class Name
         WC_COMBOBOXA                   = 'ComboBox';
         WC_COMBOBOXW                   = {L}'ComboBox';

{$IFDEF UNICODE}

TYPE
         WC_COMBOBOX         = WC_COMBOBOXW;
{$ELSE}
         WC_COMBOBOX         = WC_COMBOBOXA;
{$ENDIF}

{$ELSE}

CONST
         WC_COMBOBOX                    = 'ComboBox';
{$ENDIF}

{$ENDIF} // NOCOMBOBOX


{$ifdef win32xp}

// custom combobox control messages
         CB_SETMINVISIBLE               = (CBM_FIRST + 1);
         CB_GETMINVISIBLE               = (CBM_FIRST + 2);

// Macro 314


Function ComboBox_SetMinVisible( hwnd : hwnd; iMinVisible : WPARAM):BOOL;


// Macro 315


Function ComboBox_GetMinVisible( hwnd : hwnd):cint;


{$ENDIF}

/// =====================  End Combobox Control =========================

/// ====================== Scrollbar Control ============================

{$IFNDEF NOSCROLLBAR}

{$IFDEF _WIN32}

// Scrollbar Class Name

CONST
         WC_SCROLLBARA                  = 'ScrollBar';
         WC_SCROLLBARW                  = {L}'ScrollBar';

{$IFDEF UNICODE}

TYPE
         WC_SCROLLBAR        = WC_SCROLLBARW;
{$ELSE}
         WC_SCROLLBAR        = WC_SCROLLBARA;
{$ENDIF}

{$ELSE}

CONST
         WC_SCROLLBAR                   = 'ScrollBar';
{$ENDIF}

{$ENDIF} // NOSCROLLBAR


/// ===================== End Scrollbar Control =========================


//====== SysLink control =========================================

{$IFDEF _WIN32}
{$ifdef win32xp}

         INVALID_LINK_INDEX             = (-1);



         WC_LINK                        = {L}'SysLink';

         LWS_TRANSPARENT                = $0001;
         LWS_IGNORERETURN               = $0002;

{$ifdef win32vista}
         LWS_NOPREFIX                   = $0004;
         LWS_USEVISUALSTYLE             = $0008;
         LWS_USECUSTOMTEXT              = $0010;
         LWS_RIGHT                      = $0020;
{$endif}

         LIF_ITEMINDEX                  = $00000001;
         LIF_STATE                      = $00000002;
         LIF_ITEMID                     = $00000004;
         LIF_URL                        = $00000008;

         LIS_FOCUSED                    = $00000001;
         LIS_ENABLED                    = $00000002;
         LIS_VISITED                    = $00000004;
{$ifdef win32vista}
	 LIS_HOTTRACK        		= $00000008;
	 LIS_DEFAULTCOLORS   		= $00000010; // Don't use any custom text colors
{$endif}

TYPE


         tagLHITTESTINFO      = Record
                                 pt           : POINT;
                                 item         : LITEM;
                                 END;
         LHITTESTINFO         = tagLHITTESTINFO;
         PLHITTESTINFO        = ^tagLHITTESTINFO;
         TLHITTESTINFO        = tagLHITTESTINFO;
//         PLHITTESTINFO        = ^tagLHITTESTINFO;


         tagNMLINK            = Record
                                 hdr          : NMHDR;
                                 item         : LITEM;
                                 END;
         NMLINK               = tagNMLINK;
         PNMLINK              = ^tagNMLINK;
         TNMLINK              = tagNMLINK;
//         PNMLINK              = ^tagNMLINK;


//  SysLink notifications
//  NM_CLICK   // wParam: control ID, lParam: PNMLINK, ret: ignored.

//  LinkWindow messages

CONST
         LM_HITTEST                     = (WM_USER+$300);     // wParam: n/a, lparam: PLHITTESTINFO, ret: BOOL
         LM_GETIDEALHEIGHT              = (WM_USER+$301);     // wParam: n/a, lparam: n/a, ret: cy
         LM_SETITEM                     = (WM_USER+$302);     // wParam: n/a, lparam: LITEM*, ret: BOOL
         LM_GETITEM                     = (WM_USER+$303);     // wParam: n/a, lparam: LITEM*, ret: BOOL

{$ENDIF}
{$ENDIF} // _WIN32
//====== End SysLink control =========================================


//
// === MUI APIs ===
//
{$IFNDEF NOMUI}
procedure InitMUILanguage(uiLang:LANGID); stdcall; external commctrldll name 'InitMUILanguage';


function GetMUILanguage:LANGID; stdcall; external commctrldll name 'GetMUILanguage';
{$ENDIF}  // NOMUI

{$ENDIF}      // _WIN32_IE >= 0x0400

{$IFDEF _WIN32}
//====== TrackMouseEvent  =====================================================

{$IFNDEF NOTRACKMOUSEEVENT}

//
// If the messages for TrackMouseEvent have not been defined then define them
// now.
//
{$IFNDEF WM_MOUSEHOVER}

CONST
         WM_MOUSEHOVER                  = $02A1;
         WM_MOUSELEAVE                  = $02A3;
{$ENDIF}

//
// If the TRACKMOUSEEVENT structure and associated flags havent been declared
// then declare them now.
//
{$IFNDEF TME_HOVER}

         TME_HOVER                      = $00000001;
         TME_LEAVE                      = $00000002;
{$ifdef win32xp}
         TME_NONCLIENT                  = $00000010;
{$ENDIF} { WINVER >= 0x0500 }
         TME_QUERY                      = $40000000;
         TME_CANCEL                     = $80000000;



         HOVER_DEFAULT                  = $FFFFFFFF;

TYPE

         tagTRACKMOUSEEVENT   = Record
                                 cbSize       : DWORD;
                                 dwFlags      : DWORD;
                                 hwndTrack    : HWND;
                                 dwHoverTime  : DWORD;
                                 END;
         TRACKMOUSEEVENT      = tagTRACKMOUSEEVENT;
         LPTRACKMOUSEEVENT    = ^tagTRACKMOUSEEVENT;
         TTRACKMOUSEEVENT     = tagTRACKMOUSEEVENT;
         PTRACKMOUSEEVENT     = ^tagTRACKMOUSEEVENT;


{$ENDIF} // !TME_HOVER



//
// Declare _TrackMouseEvent.  This API tries to use the window manager's
// implementation of TrackMouseEvent if it is present, otherwise it emulates.
//
function _TrackMouseEvent(lpEventTrack:LPTRACKMOUSEEVENT):BOOL; stdcall; external commctrldll name '_TrackMouseEvent';

{$ENDIF} // !NOTRACKMOUSEEVENT

{$ifdef ie4plus}

//====== Flat Scrollbar APIs=========================================
{$IFNDEF NOFLATSBAPIS}


CONST
         WSB_PROP_CYVSCROLL             = LONG($00000001);
         WSB_PROP_CXHSCROLL             = LONG($00000002);
         WSB_PROP_CYHSCROLL             = LONG($00000004);
         WSB_PROP_CXVSCROLL             = LONG($00000008);
         WSB_PROP_CXHTHUMB              = LONG($00000010);
         WSB_PROP_CYVTHUMB              = LONG($00000020);
         WSB_PROP_VBKGCOLOR             = LONG($00000040);
         WSB_PROP_HBKGCOLOR             = LONG($00000080);
         WSB_PROP_VSTYLE                = LONG($00000100);
         WSB_PROP_HSTYLE                = LONG($00000200);
         WSB_PROP_WINSTYLE              = LONG($00000400);
         WSB_PROP_PALETTE               = LONG($00000800);
         WSB_PROP_MASK                  = LONG($00000FFF);

         FSB_FLAT_MODE                  = 2;
         FSB_ENCARTA_MODE               = 1;
         FSB_REGULAR_MODE               = 0;

function FlatSB_EnableScrollBar(hwnd:HWND;code : cint;p3 : UINT):BOOL; stdcall; external commctrldll name 'FlatSB_EnableScrollBar';
function FlatSB_ShowScrollBar(hwnd:HWND;code : cint;p3 : BOOL):BOOL; stdcall; external commctrldll name 'FlatSB_ShowScrollBar';

function FlatSB_GetScrollRange(hwnd:HWND;code : cint;p3 : LPINT;p4 : LPINT):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollRange';
function FlatSB_GetScrollRange(hwnd:HWND;code : cint;var p3,p4 : cint):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollRange';
function FlatSB_GetScrollInfo(hwnd:HWND;code : cint;ScrollInfo : LPSCROLLINFO):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollInfo';
function FlatSB_GetScrollInfo(hwnd:HWND;code : cint;var ScrollInfo : TSCROLLINFO):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollInfo';

function FlatSB_GetScrollPos(hwnd:HWND;code : cint):cint; stdcall; external commctrldll name 'FlatSB_GetScrollPos';


function FlatSB_GetScrollProp(hwnd:HWND):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollProp';
{$IFDEF _WIN64}
function FlatSB_GetScrollPropPtr(hwnd:HWND;propIndex : cint;p3 : LPINT):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollPropPtr';
{$ELSE}
function FlatSB_GetScrollPropPtr(hwnd:HWND;code : cint):BOOL; stdcall; external commctrldll name 'FlatSB_GetScrollProp';
{$ENDIF}


function FlatSB_SetScrollPos(hWnd:HWND;nBar,nPos:cint;bRedraw:BOOL):cint; stdcall; external commctrldll name 'FlatSB_SetScrollPos';

function FlatSB_SetScrollInfo(hWnd:HWND;BarFlag:cint;const ScrollInfo:TScrollInfo;Redraw:BOOL):cint; stdcall; external commctrldll name 'FlatSB_SetScrollInfo';


function FlatSB_SetScrollRange(hWnd: HWND; nBar,nMinPos,nMaxPos: cint; bRedraw: BOOL):cint; stdcall; external commctrldll name 'FlatSB_SetScrollRange';
function FlatSB_SetScrollProp(p1: HWND; index : UINT; newValue: INT_PTR; p4: BOOL):BOOL; stdcall; external commctrldll name 'FlatSB_SetScrollProp';

function InitializeFlatSB(hWnd:HWND):BOOL; stdcall; external commctrldll name 'InitializeFlatSB';
function UninitializeFlatSB(hWnd:HWND):HRESULT; stdcall; external commctrldll name 'UninitializeFlatSB';

{$ENDIF}  //  NOFLATSBAPIS

{$ENDIF}      // _WIN32_IE >= 0x0400

{$ENDIF} { _WIN32 }

{$ENDIF}      // _WIN32_IE >= 0x0300

{$ifdef win32xp}
//
// subclassing stuff
//
TYPE
    subclassproc=function (hwnd:HWND;uMsg:cUINT; wParam:WPARAM;lparam:LPARAM;uISubClass : UINT_PTR;dwRefData:DWORD_PTR):LRESULT; stdcall;

{
     (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, = LRESULT
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    hwnd:HWND;uMsg:cUINT; wParam:WPARAM;lparam:LPARAM;uISubClass : CUINT_PTR;dwRefData:DWORD_PTR):LRESULT; stdcall;
}

function SetWindowSubclass(hWnd:HWND;pfnSubclass:SUBCLASSPROC;uIdSubclass:UINT_PTR;dwRefData:DWORD_PTR):BOOL; stdcall; external commctrldll name 'SetWindowSubclass';
function GetWindowSubclass(hWnd:HWND;pfnSubclass:SUBCLASSPROC;uIdSubclass:UINT_PTR;pdwRefData:PDWORD_PTR):BOOL; stdcall; external commctrldll name 'GetWindowSubclass';
function RemoveWindowSubclass(hWnd:HWND;pfnSubclass:SUBCLASSPROC;uIdSubclass:UINT_PTR):BOOL; stdcall; external commctrldll name 'RemoveWindowSubclass';

function DefSubclassProc(hWnd:HWND;uMsg:UINT;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall; external commctrldll name 'DefSubclassProc';
{$ENDIF}

{$ifdef NTDDI_VISTA}
type _LI_METRIC= (

   LIM_SMALL=0, // corresponds to SM_CXSMICON/SM_CYSMICON
   LIM_LARGE   // corresponds to SM_CXICON/SM_CYICON
		 );

Function LoadIconMetric( hinst:HINST; pszName:LPCWStr;lims:cint; var phico: HICON ):HRESULT; stdcall; external commctrldll name 'LoadIconMetric';
Function LoadIconWithScaleDown( hinst:HINST; pszName:LPCWStr;cx:cint;cy:cint;var phico: HICON ):HRESULT; stdcall; external commctrldll name 'LoadIconMetric';

{$endif}

{$ifdef win32xp}

function DrawShadowText(hdc:HDC;pszText:LPCWSTR;cch:UINT;prc:PRECT;dwFlags:DWORD;crText:COLORREF;crShadow:COLORREF;ixOffset:cint;iyOffset:cint):cint; stdcall; external commctrldll name 'DrawShadowText';
{$ENDIF}

// ===================== Task Dialog =========================

Type
  PFTASKDIALOGCALLBACK = function(hwnd: HWND; msg: UINT; wParam: WPARAM; lParam: LPARAM; lpRefData: LONG_PTR): HRESULT; stdcall;

  // _TASKDIALOG_FLAGS enum
const
  TDF_ENABLE_HYPERLINKS               = $0001;
  TDF_USE_HICON_MAIN                  = $0002;
  TDF_USE_HICON_FOOTER                = $0004;
  TDF_ALLOW_DIALOG_CANCELLATION       = $0008;
  TDF_USE_COMMAND_LINKS               = $0010;
  TDF_USE_COMMAND_LINKS_NO_ICON       = $0020;
  TDF_EXPAND_FOOTER_AREA              = $0040;
  TDF_EXPANDED_BY_DEFAULT             = $0080;
  TDF_VERIFICATION_FLAG_CHECKED       = $0100;
  TDF_SHOW_PROGRESS_BAR               = $0200;
  TDF_SHOW_MARQUEE_PROGRESS_BAR       = $0400;
  TDF_CALLBACK_TIMER                  = $0800;
  TDF_POSITION_RELATIVE_TO_WINDOW     = $1000;
  TDF_RTL_LAYOUT                      = $2000;
  TDF_NO_DEFAULT_RADIO_BUTTON         = $4000;
  TDF_CAN_BE_MINIMIZED                = $8000;

type
  TASKDIALOG_FLAGS = Integer;                         // Note: _TASKDIALOG_FLAGS is an int

  // _TASKDIALOG_MESSAGES enum
const
  TDM_NAVIGATE_PAGE                   = WM_USER+101;
  TDM_CLICK_BUTTON                    = WM_USER+102; // wParam = Button ID
  TDM_SET_MARQUEE_PROGRESS_BAR        = WM_USER+103; // wParam = 0 (nonMarque) wParam != 0 (Marquee)
  TDM_SET_PROGRESS_BAR_STATE          = WM_USER+104; // wParam = new progress state
  TDM_SET_PROGRESS_BAR_RANGE          = WM_USER+105; // lParam = MAKELPARAM(nMinRange, nMaxRange)
  TDM_SET_PROGRESS_BAR_POS            = WM_USER+106; // wParam = new position
  TDM_SET_PROGRESS_BAR_MARQUEE        = WM_USER+107; // wParam = 0 (stop marquee), wParam != 0 (start marquee), lparam = speed (milliseconds between repaints)
  TDM_SET_ELEMENT_TEXT                = WM_USER+108; // wParam = element (TASKDIALOG_ELEMENTS), lParam = new element text (LPCWSTR)
  TDM_CLICK_RADIO_BUTTON              = WM_USER+110; // wParam = Radio Button ID
  TDM_ENABLE_BUTTON                   = WM_USER+111; // lParam = 0 (disable), lParam != 0 (enable), wParam = Button ID
  TDM_ENABLE_RADIO_BUTTON             = WM_USER+112; // lParam = 0 (disable), lParam != 0 (enable), wParam = Radio Button ID
  TDM_CLICK_VERIFICATION              = WM_USER+113; // wParam = 0 (unchecked), 1 (checked), lParam = 1 (set key focus)
  TDM_UPDATE_ELEMENT_TEXT             = WM_USER+114; // wParam = element (TASKDIALOG_ELEMENTS), lParam = new element text (LPCWSTR)
  TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE = WM_USER+115; // wParam = Button ID, lParam = 0 (elevation not required), lParam != 0 (elevation required)
  TDM_UPDATE_ICON                     = WM_USER+116; // wParam = icon element (TASKDIALOG_ICON_ELEMENTS), lParam = new icon (hIcon if TDF_USE_HICON_* was set, PCWSTR otherwise)

  // _TASKDIALOG_NOTIFICATIONS enum
const
  TDN_CREATED                         = 0;
  TDN_NAVIGATED                       = 1;
  TDN_BUTTON_CLICKED                  = 2;            // wParam = Button ID
  TDN_HYPERLINK_CLICKED               = 3;            // lParam = (LPCWSTR)pszHREF
  TDN_TIMER                           = 4;            // wParam = Milliseconds since dialog created or timer reset
  TDN_DESTROYED                       = 5;
  TDN_RADIO_BUTTON_CLICKED            = 6;            // wParam = Radio Button ID
  TDN_DIALOG_CONSTRUCTED              = 7;
  TDN_VERIFICATION_CLICKED            = 8;             // wParam = 1 if checkbox checked, 0 if not, lParam is unused and always 0
  TDN_HELP                            = 9;
  TDN_EXPANDO_BUTTON_CLICKED          = 10;           // wParam = 0 (dialog is now collapsed), wParam != 0 (dialog is now expanded)

type
  _TASKDIALOG_BUTTON = packed record
      nButtonID: Integer;
      pszButtonText: PCWSTR;
  end;
  TASKDIALOG_BUTTON = _TASKDIALOG_BUTTON;
  TTASKDIALOG_BUTTON = TASKDIALOG_BUTTON;
  PTASKDIALOG_BUTTON = ^TASKDIALOG_BUTTON;

  // _TASKDIALOG_ELEMENTS enum
const
  TDE_CONTENT              = 0;
  TDE_EXPANDED_INFORMATION = 1;
  TDE_FOOTER               = 2;
  TDE_MAIN_INSTRUCTION     = 3;

  // _TASKDIALOG_ICON_ELEMENTS enum
  TDIE_ICON_MAIN   = 0;
  TDIE_ICON_FOOTER = 1;

  TD_WARNING_ICON         = MAKEINTRESOURCEW(Word(-1));
  TD_ERROR_ICON           = MAKEINTRESOURCEW(Word(-2));
  TD_INFORMATION_ICON     = MAKEINTRESOURCEW(Word(-3));
  TD_SHIELD_ICON          = MAKEINTRESOURCEW(Word(-4));
  TD_SHIELD_GRADIENT_ICON = MAKEINTRESOURCEW(Word(-5));
  TD_SHIELD_WARNING_ICON  = MAKEINTRESOURCEW(Word(-6));
  TD_SHIELD_ERROR_ICON    = MAKEINTRESOURCEW(Word(-7));
  TD_SHIELD_OK_ICON       = MAKEINTRESOURCEW(Word(-8));
  TD_SHIELD_GRAY_ICON     = MAKEINTRESOURCEW(Word(-9));

  // _TASKDIALOG_COMMON_BUTTON_FLAGS enum
  TDCBF_OK_BUTTON            = $0001; // selected control return value IDOK
  TDCBF_YES_BUTTON           = $0002; // selected control return value IDYES
  TDCBF_NO_BUTTON            = $0004; // selected control return value IDNO
  TDCBF_CANCEL_BUTTON        = $0008; // selected control return value IDCANCEL
  TDCBF_RETRY_BUTTON         = $0010; // selected control return value IDRETRY
  TDCBF_CLOSE_BUTTON         = $0020; // selected control return value IDCLOSE

type
  TASKDIALOG_COMMON_BUTTON_FLAGS = Integer;           // Note: _TASKDIALOG_COMMON_BUTTON_FLAGS is an int

  _TASKDIALOGCONFIG = packed record
    cbSize: UINT;
    hwndParent: HWND;
    hInstance: HINST;                                 // used for MAKEINTRESOURCE() strings
    dwFlags: TASKDIALOG_FLAGS;                        // TASKDIALOG_FLAGS (TDF_XXX) flags
    dwCommonButtons: TASKDIALOG_COMMON_BUTTON_FLAGS;  // TASKDIALOG_COMMON_BUTTON (TDCBF_XXX) flags
    pszWindowTitle: PCWSTR;                           // string or MAKEINTRESOURCE()
    case PtrInt of
      0: (hMainIcon: HICON);
      1: (
        pszMainIcon: PCWSTR;
        pszMainInstruction: PCWSTR;
        pszContent: PCWSTR;
        cButtons: UINT;
        pButtons: PTASKDIALOG_BUTTON;
        nDefaultButton: Integer;
        cRadioButtons: UINT;
        pRadioButtons: PTASKDIALOG_BUTTON;
        nDefaultRadioButton: Integer;
        pszVerificationText: PCWSTR;
        pszExpandedInformation: PCWSTR;
        pszExpandedControlText: PCWSTR;
        pszCollapsedControlText: PCWSTR;
        case PtrInt of
          0: (hFooterIcon: HICON);
          1: (
            pszFooterIcon: PCWSTR;
            pszFooter: PCWSTR;
            pfCallback: PFTASKDIALOGCALLBACK;
            lpCallbackData: LONG_PTR;
            cxWidth: UINT;                                // width of the Task Dialog's client area in DLU's. If 0, Task Dialog will calculate the ideal width.
          );
      );
  end;
  TASKDIALOGCONFIG = _TASKDIALOGCONFIG;
  PTASKDIALOGCONFIG = ^TASKDIALOGCONFIG;
  TTASKDIALOGCONFIG = TASKDIALOGCONFIG;

var
  TaskDialogIndirect: function(const pTaskConfig: PTASKDIALOGCONFIG; pnButton: PInteger; pnRadioButton: PInteger; pfVerificationFlagChecked: PBOOL): HRESULT; stdcall;
  TaskDialog: function(hwndParent: HWND; hInstance: HINST; pszWindowTitle: PCWSTR; pszMainInstruction: PCWSTR; pszContent: PCWSTR;
      dwCommonButtons: TASKDIALOG_COMMON_BUTTON_FLAGS; pszIcon: PCWSTR; pnButton: PInteger): HRESULT; stdcall;


// ==================== End TaskDialog =======================


IMPLEMENTATION

// Macro 8

//#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
//     (fn)((hwnd), (int)(wParam), (NMHDR *)(lParam))

Procedure HANDLE_WM_NOTIFY( hwnd : hwnd; wParam : cint;var _lParam : NMHDR ;fn:Pointer);

TYPE FnType=procedure ( hwnd : hwnd; wParam : cint;tst:LPARAM); stdcall;

Begin
 fnType(fn)(hwnd, wParam, LPARAM(@_lParam));
end;

// Macro 9

//#define FORWARD_WM_NOTIFY(hwnd, idFrom, pnmhdr, fn) \
//     (LRESULT)(fn)((hwnd), WM_NOTIFY, (WPARAM)(int)(idFrom), (LPARAM)(NMHDR *)(pnmhdr))

Function FORWARD_WM_NOTIFY( hwnd : hwnd; idFrom : cint;var  pnmhdr : NMHDR ; fn : pointer ):LRESULT;

TYPE FnType=Function( hwnd : hwnd; wParam : cint;wparam2:cint;lparam1:lparam):LResult; stdcall;

Begin
 Result:=LRESULT(fntype(fn)(hwnd, WM_NOTIFY, idFrom, LPARAM(@pnmhdr)));
end;

// Macro 10
// #define CCSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))


// Macro 11
// #define     ImageList_AddIcon(himl, hicon) ImageList_ReplaceIcon(himl, -1, hicon)

Function ImageList_AddIcon(Himl:HIMAGELIST;hicon:HICON):cint;
Begin
  Result:=ImageList_ReplaceIcon(himl,-1,hicon);
end;


// Macro 12
// #define INDEXTOOVERLAYMASK(i)   ((i) << 8)

// Macro 13
// #define     ImageList_RemoveAll(himl) ImageList_Remove(himl, -1)

Procedure ImageList_RemoveAll(himl:HIMAGELIST);
Begin
 ImageList_Remove(himl, -1)
End;

// Macro 14
// #define     ImageList_ExtractIcon(hi, himl, i) ImageList_GetIcon(himl, i, 0)

function  ImageList_ExtractIcon(hi:longint;  himl:HIMAGELIST;i:longint):HICON;

Begin
 result:=ImageList_GetIcon(himl, i,0);
end;


// Macro 15
// #define     ImageList_LoadBitmap(hi, lpbmp, cx, cGrow, crMask)
// ImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0)

Procedure ImageList_LoadBitmap(hi:HINST;bmp:LPCTSTR;cx:cint;cGrow:cint;crMask:COLORREF);

Begin
 ImageList_LoadImage(hi, bmp, cx, cGrow, crMask, IMAGE_BITMAP, 0);
End;

// Macro 16
// #define Header_GetItemCount(hwndHD) \
//     (int)SNDMSG((hwndHD), HDM_GETITEMCOUNT, 0, DWord(0))

Function Header_GetItemCount( hwndHD : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwndHD), HDM_GETITEMCOUNT, 0, LPARAM(0)))
end;


// Macro 17
//#define Header_InsertItem(hwndHD, i, phdi) \
//     (int)SNDMSG((hwndHD), HDM_INSERTITEM, (WPARAM)(int)(i), (LPARAM)(const HD_ITEM *)(phdi))

Function Header_InsertItem( hwndHD : hwnd; i : cint;const phdi : HD_ITEM ):cint;

Begin
 Result:=cint(SendMessage((hwndHD), HDM_INSERTITEM, (i), LPARAM(@phdi)));
end;

// Macro 18
//#define Header_DeleteItem(hwndHD, i) \
//     (BOOL)SNDMSG((hwndHD), HDM_DELETEITEM, (WPARAM)(int)(i), LPARAM(0))

Function Header_DeleteItem( hwndHD : hwnd; i : cint):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndHD), HDM_DELETEITEM, (i), LPARAM(0)));
end;


// Macro 19
// #define Header_GetItem(hwndHD, i, phdi) \
//     (BOOL)SNDMSG((hwndHD), HDM_GETITEM, (WPARAM)(int)(i), (LPARAM)(HD_ITEM *)(phdi))

Function Header_GetItem( hwndHD : hwnd; i : cint;var  phdi : HD_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndHD), HDM_GETITEM, (i), LPARAM(@phdi)));
end;


// Macro 20
// #define Header_SetItem(hwndHD, i, phdi) \
//     (BOOL)SNDMSG((hwndHD), HDM_SETITEM, (WPARAM)(int)(i), (LPARAM)(const HD_ITEM *)(phdi))

Function Header_SetItem( hwndHD : hwnd; i : cint;const phdi : HD_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndHD), HDM_SETITEM, (i), LPARAM(@phdi)));
end;


// Macro 21
// #define Header_Layout(hwndHD, playout) \
//     (BOOL)SNDMSG((hwndHD), HDM_LAYOUT, 0, (LPARAM)(HD_LAYOUT *)(playout))

Function Header_Layout( hwndHD : hwnd;var  playout : HD_LAYOUT ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndHD), HDM_LAYOUT, 0, LPARAM(@playout)))
end;


// Macro 22
// #define Header_GetItemRect(hwnd, iItem, lprc) \
//         (BOOL)SNDMSG((hwnd), HDM_GETITEMRECT, (WPARAM)(iItem), (LPARAM)(lprc))

Function Header_GetItemRect( hwnd : hwnd; iItem : WPARAM; lprc : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), HDM_GETITEMRECT, iItem, lprc));
end;


// Macro 23
// #define Header_SetImageList(hwnd, himl) \
//         (HIMAGELIST)SNDMSG((hwnd), HDM_SETIMAGELIST, 0, (LPARAM)(himl))

Function Header_SetImageList( hwnd : hwnd; himl : LPARAM):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), HDM_SETIMAGELIST, 0, himl))
end;


// Macro 24
// #define Header_GetImageList(hwnd) \
//         (HIMAGELIST)SNDMSG((hwnd), HDM_GETIMAGELIST, 0, 0)

Function Header_GetImageList( hwnd : hwnd):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), HDM_GETIMAGELIST, 0, 0))
end;


// Macro 25
// #define Header_OrderToIndex(hwnd, i) \
//         (int)SNDMSG((hwnd), HDM_ORDERTOINDEX, (WPARAM)(i), 0)

Function Header_OrderToIndex( hwnd : hwnd; i : WPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_ORDERTOINDEX, (i), 0))
end;


// Macro 26
// #define Header_CreateDragImage(hwnd, i) \
//         (HIMAGELIST)SNDMSG((hwnd), HDM_CREATEDRAGIMAGE, (WPARAM)(i), 0)

Function Header_CreateDragImage( hwnd : hwnd; i : WPARAM):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), HDM_CREATEDRAGIMAGE, (i), 0))
end;


// Macro 27
// #define Header_GetOrderArray(hwnd, iCount, lpi) \
//         (BOOL)SNDMSG((hwnd), HDM_GETORDERARRAY, (WPARAM)(iCount), (LPARAM)(lpi))

Function Header_GetOrderArray( hwnd : hwnd; iCount : WPARAM; lpi : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), HDM_GETORDERARRAY, iCount, lpi))
end;


// Macro 28
// #define Header_SetOrderArray(hwnd, iCount, lpi) \
//         (BOOL)SNDMSG((hwnd), HDM_SETORDERARRAY, (WPARAM)(iCount), (LPARAM)(lpi))

Function Header_SetOrderArray( hwnd : hwnd; iCount : WPARAM; lpi : PInteger):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwnd), HDM_SETORDERARRAY, iCount, LPARAM(lpi)))
end;


// Macro 29
// #define Header_SetHotDivider(hwnd, fPos, dw) \
//         (int)SNDMSG((hwnd), HDM_SETHOTDIVIDER, (WPARAM)(fPos), (LPARAM)(dw))

Function Header_SetHotDivider( hwnd : hwnd; fPos : WPARAM; dw : LPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_SETHOTDIVIDER, fPos, dw))
end;


{$ifdef ie5plus}
// Macro 30
// #define Header_SetBitmapMargin(hwnd, iWidth) \
//         (int)SNDMSG((hwnd), HDM_SETBITMAPMARGIN, (WPARAM)(iWidth), 0)

Function Header_SetBitmapMargin( hwnd : hwnd; iWidth : WPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_SETBITMAPMARGIN, iWidth, 0))
end;


// Macro 31
// #define Header_GetBitmapMargin(hwnd) \
//         (int)SNDMSG((hwnd), HDM_GETBITMAPMARGIN, 0, 0)

Function Header_GetBitmapMargin( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_GETBITMAPMARGIN, 0, 0))
end;

{$ENDIF}

// Macro 32
// #define Header_SetUnicodeFormat(hwnd, fUnicode)  \
//     (BOOL)SNDMSG((hwnd), HDM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0)

Function Header_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), HDM_SETUNICODEFORMAT, fUnicode, 0));
end;


// Macro 33
// #define Header_GetUnicodeFormat(hwnd)  \
//     (BOOL)SNDMSG((hwnd), HDM_GETUNICODEFORMAT, 0, 0)

Function Header_GetUnicodeFormat( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), HDM_GETUNICODEFORMAT, 0, 0))
end;


{$ifdef IE5plus}
// Macro 34
// #define Header_SetFilterChangeTimeout(hwnd, i) \
//         (int)SNDMSG((hwnd), HDM_SETFILTERCHANGETIMEOUT, 0, (LPARAM)(i))

Function Header_SetFilterChangeTimeout( hwnd : hwnd; i : LPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_SETFILTERCHANGETIMEOUT, 0, (i)))
end;


// Macro 35
// #define Header_EditFilter(hwnd, i, fDiscardChanges) \
//         (int)SNDMSG((hwnd), HDM_EDITFILTER, (WPARAM)(i), MAKELPARAM(fDiscardChanges, 0))

Function Header_EditFilter( hwnd : hwnd; i : WPARAM; fDiscardChanges :cint ):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_EDITFILTER, (i), MAKELPARAM(fDiscardChanges, 0)));
end;


// Macro 36
// #define Header_ClearFilter(hwnd, i) \
//         (int)SNDMSG((hwnd), HDM_CLEARFILTER, (WPARAM)(i), 0)

Function Header_ClearFilter( hwnd : hwnd; i : WPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_CLEARFILTER, (i), 0))
end;


// Macro 37
// #define Header_ClearAllFilters(hwnd) \
//         (int)SNDMSG((hwnd), HDM_CLEARFILTER, (WPARAM)-1, 0)

Function Header_ClearAllFilters( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), HDM_CLEARFILTER, WPARAM(-1), 0))
end;
{$endif}
{$ifdef win32vista}
// macro 37a ..37d
function Header_GetOverflowRect( hwnd : hwnd; lprc:lprect):bool;
begin
  result:=bool(sendmessage(hwnd, HDM_GETOVERFLOWRECT, 0, LPARAM(lprc)));
end;

function Header_GetFocusedItem(hwnd : hwnd):cint;
begin
  Result:=cint(SendMessage((hwnd), HDM_GETFOCUSEDITEM, WPARAM(0), LPARAM(0)));
end;

function Header_SetFocusedItem(hwnd:hwnd; iItem:cint):BOOL;
begin
  result:=bool(sendmessage(hwnd, HDM_SETFOCUSEDITEM, WPARAM(0),LPARAM(iItem)));
end;
function Header_GetItemDropDownRect(hwnd : hwnd;iItem:cint; lprc:lprect):bool;
begin
  result:=bool(sendmessage(hwnd, HDM_GETITEMDROPDOWNRECT, WPARAM(iItem), LPARAM(lprc)));
end;
{$endif}

// Macro 38
// #define ListView_SetUnicodeFormat(hwnd, fUnicode)  \
//     (BOOL)SNDMSG((hwnd), LVM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0)

Function ListView_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETUNICODEFORMAT, fUnicode, 0));
end;


// Macro 39
// #define ListView_GetUnicodeFormat(hwnd)  \
//     (BOOL)SNDMSG((hwnd), LVM_GETUNICODEFORMAT, 0, 0)

Function ListView_GetUnicodeFormat( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETUNICODEFORMAT, 0, 0))
end;


// Macro 40
// #define ListView_GetBkColor(hwnd)  \
//     (COLORREF)SNDMSG((hwnd), LVM_GETBKCOLOR, 0, LPARAM(0))

Function ListView_GetBkColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_GETBKCOLOR, 0, LPARAM(0)))
end;


// Macro 41
// #define ListView_SetBkColor(hwnd, clrBk) \
//     (BOOL)SNDMSG((hwnd), LVM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk))

Function ListView_SetBkColor( hwnd : hwnd; clrBk : COLORREF):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETBKCOLOR, 0, clrBk))
end;


// Macro 42
// #define ListView_GetImageList(hwnd, iImageList) \
//     (HIMAGELIST)SNDMSG((hwnd), LVM_GETIMAGELIST, (WPARAM)(INT)(iImageList), DWord(0))

Function ListView_GetImageList( hwnd : hwnd; iImageList : CINT):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), LVM_GETIMAGELIST, iImageList, LPARAM(0)))
end;


// Macro 43
// #define ListView_SetImageList(hwnd, himl, iImageList) \
//     (HIMAGELIST)SNDMSG((hwnd), LVM_SETIMAGELIST, (WPARAM)(iImageList), (LPARAM)(HIMAGELIST)(himl))

Function ListView_SetImageList( hwnd : hwnd; himl : HIMAGELIST; iImageList : WPARAM):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), LVM_SETIMAGELIST, iImageList, himl))
end;


// Macro 44
// #define ListView_GetItemCount(hwnd) \
//     (int)SNDMSG((hwnd), LVM_GETITEMCOUNT, 0, DWord(0))

Function ListView_GetItemCount( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_GETITEMCOUNT, 0, LPARAM(0)))
end;


// Macro 45
// #define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)

// Macro 46
// #define ListView_GetItem(hwnd, pitem) \
//     (BOOL)SNDMSG((hwnd), LVM_GETITEM, 0, (LPARAM)(LV_ITEM *)(pitem))

Function ListView_GetItem( hwnd : hwnd;var  pitem : LV_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETITEM, 0, LPARAM(@pitem)))
end;


// Macro 47
// #define ListView_SetItem(hwnd, pitem) \
//     (BOOL)SNDMSG((hwnd), LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)(pitem))

Function ListView_SetItem( hwnd : hwnd;const pitem : LV_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETITEM, 0, LPARAM(@pitem)))
end;


// Macro 48
// #define ListView_InsertItem(hwnd, pitem)   \
//     (int)SNDMSG((hwnd), LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM *)(pitem))

Function ListView_InsertItem( hwnd : hwnd;Const pitem : LV_ITEM ):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_INSERTITEM, 0, LPARAM(@pitem)))
end;


// Macro 49
// #define ListView_DeleteItem(hwnd, i) \
//     (BOOL)SNDMSG((hwnd), LVM_DELETEITEM, (WPARAM)(int)(i), DWord(0))

Function ListView_DeleteItem( hwnd : hwnd; i : cint):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_DELETEITEM, (i), LPARAM(0)))
end;


// Macro 50
// #define ListView_DeleteAllItems(hwnd) \
//     (BOOL)SNDMSG((hwnd), LVM_DELETEALLITEMS, 0, DWord(0))

Function ListView_DeleteAllItems( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_DELETEALLITEMS, 0, LPARAM(0)))
end;


// Macro 51
// #define ListView_GetCallbackMask(hwnd) \
//     (BOOL)SNDMSG((hwnd), LVM_GETCALLBACKMASK, 0, 0)

Function ListView_GetCallbackMask( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETCALLBACKMASK, 0, 0))
end;


// Macro 52
// #define ListView_SetCallbackMask(hwnd, mask) \
//     (BOOL)SNDMSG((hwnd), LVM_SETCALLBACKMASK, (WPARAM)(UINT)(mask), 0)

Function ListView_SetCallbackMask( hwnd : hwnd; mask : UINT):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETCALLBACKMASK, mask, 0))
end;


// Macro 53
// #define ListView_GetNextItem(hwnd, i, flags) \
//     (int)SNDMSG((hwnd), LVM_GETNEXTITEM, (WPARAM)(int)(i), MAKELPARAM((flags), 0))

Function ListView_GetNextItem( hwnd : hwnd; i : cint; flags : cint):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_GETNEXTITEM, (i), MAKELPARAM((flags), 0)))
end;


// Macro 54
// #define ListView_FindItem(hwnd, iStart, plvfi) \
//     (int)SNDMSG((hwnd), LVM_FINDITEM, (WPARAM)(int)(iStart), (LPARAM)(const LV_FINDINFO *)(plvfi))

Function ListView_FindItem( hwnd : hwnd; iStart : cint;const plvfi : LV_FINDINFO ):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_FINDITEM, iStart, LPARAM(@plvfi)))
end;


// Macro 55
// #define ListView_GetItemRect(hwnd, i, prc, code) \
//      (BOOL)SNDMSG((hwnd), LVM_GETITEMRECT, (WPARAM)(int)(i), \
//            ((prc) ? (((RECT *)(prc))->left = (code),(LPARAM)(RECT *)(prc)) : (LPARAM)(RECT *)NULL))

Function ListView_GetItemRect( hwnd : hwnd; i : cint;var prc : TRect;code : cint):BOOL;
begin
  if assigned(@prc) then
    begin
      prc.left:=Code;
      Result:=BOOL(SendMessage(hWnd,LVM_GETITEMRECT,i,LPARAM(@prc)));
    end
  else
    Result:=BOOL(SendMessage(hWnd,LVM_GETITEMRECT,i,0));
end;


// Macro 56
// #define ListView_SetItemPosition(hwndLV, i, x, y) \
//     (BOOL)SNDMSG((hwndLV), LVM_SETITEMPOSITION, (WPARAM)(int)(i), MAKELPARAM((x), (y)))

Function ListView_SetItemPosition( hwndLV : hwnd; i : cint; x : cint ; y : cint ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_SETITEMPOSITION, (i), MAKELPARAM((x), (y))))
end;


// Macro 57
// #define ListView_GetItemPosition(hwndLV, i, ppt) \
//     (BOOL)SNDMSG((hwndLV), LVM_GETITEMPOSITION, (WPARAM)(int)(i), (LPARAM)(POINT *)(ppt))

Function ListView_GetItemPosition( hwndLV : hwnd; i : cint;var  ppt : POINT ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_GETITEMPOSITION, (i), LPAram(@ppt)))
end;

// Macro 58
// #define ListView_GetStringWidth(hwndLV, psz) \
//     (int)SNDMSG((hwndLV), LVM_GETSTRINGWIDTH, 0, (LPARAM)(LPCTSTR)(psz))

Function ListView_GetStringWidth( hwndLV : hwnd; psz : LPCTSTR):cint;

Begin
 Result:=cint(SendMessage((hwndLV), LVM_GETSTRINGWIDTH, 0, LPARAM(psz)))
end;


// Macro 59
// #define ListView_HitTest(hwndLV, pinfo) \
//     (int)SNDMSG((hwndLV), LVM_HITTEST, 0, (LPARAM)(LV_HITTESTINFO *)(pinfo))

Function ListView_HitTest( hwndLV : hwnd;var  pinfo : LV_HITTESTINFO ):cint;

Begin
 Result:=cint(SendMessage((hwndLV), LVM_HITTEST, 0, LParam(@pinfo)))
end;


// Macro 60
// #define ListView_EnsureVisible(hwndLV, i, fPartialOK) \
//     (BOOL)SNDMSG((hwndLV), LVM_ENSUREVISIBLE, (WPARAM)(int)(i), MAKELPARAM((fPartialOK), 0))

Function ListView_EnsureVisible( hwndLV : hwnd; i : cint; fPartialOK : cint ):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_ENSUREVISIBLE, (i), MAKELPARAM((fPartialOK), 0)))
end;


Function ListView_EnsureVisible( hwndLV : hwnd; i : cint; fPartialOK : BOOL ):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_ENSUREVISIBLE, (i), LPARAM(fPartialOK)))
end;


// Macro 61
// #define ListView_Scroll(hwndLV, dx, dy) \
//     (BOOL)SNDMSG((hwndLV), LVM_SCROLL, (WPARAM)(int)(dx), (LPARAM)(int)(dy))

Function ListView_Scroll( hwndLV : hwnd; dx : cint; dy : cint):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_SCROLL, dx, dy))
end;


// Macro 62
// #define ListView_RedrawItems(hwndLV, iFirst, iLast) \
//     (BOOL)SNDMSG((hwndLV), LVM_REDRAWITEMS, (WPARAM)(int)(iFirst), (LPARAM)(int)(iLast))

Function ListView_RedrawItems( hwndLV : hwnd; iFirst : cint; iLast : cint):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_REDRAWITEMS, iFirst, iLast))
end;


// Macro 63
// #define ListView_Arrange(hwndLV, code) \
//     (BOOL)SNDMSG((hwndLV), LVM_ARRANGE, (WPARAM)(UINT)(code), DWord(0))

Function ListView_Arrange( hwndLV : hwnd; code : UINT):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_ARRANGE, code, LPARAM(0)))
end;


// Macro 64
// #define ListView_EditLabel(hwndLV, i) \
//     (HWND)SNDMSG((hwndLV), LVM_EDITLABEL, (WPARAM)(int)(i), DWord(0))

Function ListView_EditLabel( hwndLV : hwnd; i : cint):HWND;

Begin
 Result:=HWND(SendMessage((hwndLV), LVM_EDITLABEL, (i), LPARAM(0)))
end;


// Macro 65
// #define ListView_GetEditControl(hwndLV) \
//     (HWND)SNDMSG((hwndLV), LVM_GETEDITCONTROL, 0, DWord(0))

Function ListView_GetEditControl( hwndLV : hwnd):HWND;

Begin
 Result:=HWND(SendMessage((hwndLV), LVM_GETEDITCONTROL, 0, LPARAM(0)))
end;


// Macro 66
// #define ListView_GetColumn(hwnd, iCol, pcol) \
//     (BOOL)SNDMSG((hwnd), LVM_GETCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(LV_COLUMN *)(pcol))

Function ListView_GetColumn( hwnd : hwnd; iCol : cint;var  pcol : LV_COLUMN ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETCOLUMN, iCol, LPARAM(@pcol)))
end;


// Macro 67
// #define ListView_SetColumn(hwnd, iCol, pcol) \
//     (BOOL)SNDMSG((hwnd), LVM_SETCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN *)(pcol))

Function ListView_SetColumn( hwnd : hwnd; iCol : cint; Const pcol : LV_COLUMN ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETCOLUMN, iCol, LPARAM(@pcol)))
end;


// Macro 68
// #define ListView_InsertColumn(hwnd, iCol, pcol) \
//     (int)SNDMSG((hwnd), LVM_INSERTCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN *)(pcol))

Function ListView_InsertColumn( hwnd : hwnd; iCol : cint;const pcol : LV_COLUMN ):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_INSERTCOLUMN, iCol, LPARAM(@pcol)))
end;


// Macro 69
// #define ListView_DeleteColumn(hwnd, iCol) \
//     (BOOL)SNDMSG((hwnd), LVM_DELETECOLUMN, (WPARAM)(int)(iCol), 0)

Function ListView_DeleteColumn( hwnd : hwnd; iCol : cint):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_DELETECOLUMN, iCol, 0))
end;


// Macro 70
// #define ListView_GetColumnWidth(hwnd, iCol) \
//     (int)SNDMSG((hwnd), LVM_GETCOLUMNWIDTH, (WPARAM)(int)(iCol), 0)

Function ListView_GetColumnWidth( hwnd : hwnd; iCol : cint):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_GETCOLUMNWIDTH, iCol, 0))
end;


// Macro 71
// #define ListView_SetColumnWidth(hwnd, iCol, cx) \
//     (BOOL)SNDMSG((hwnd), LVM_SETCOLUMNWIDTH, (WPARAM)(int)(iCol), MAKELPARAM((cx), 0))

Function ListView_SetColumnWidth( hwnd : hwnd; iCol : cint; cx :cint ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETCOLUMNWIDTH, iCol, MAKELPARAM((cx), 0)))
end;


// Macro 72
// #define ListView_GetHeader(hwnd)\
//     (HWND)SNDMSG((hwnd), LVM_GETHEADER, 0, DWord(0))

Function ListView_GetHeader( hwnd : hwnd):HWND;

Begin
 Result:=Windows.HWND(SendMessage((hwnd), LVM_GETHEADER, 0, LPARAM(0)));
end;


// Macro 73
// #define ListView_CreateDragImage(hwnd, i, lpptUpLeft) \
//     (HIMAGELIST)SNDMSG((hwnd), LVM_CREATEDRAGIMAGE, (WPARAM)(int)(i), (LPARAM)(LPPOINT)(lpptUpLeft))

Function ListView_CreateDragImage( hwnd : hwnd; i : cint; lpptUpLeft : LPPOINT):HIMAGELIST;inline;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), LVM_CREATEDRAGIMAGE, WPARAM(i), LPARAM(lpptUpLeft)))
end;

Function ListView_CreateDragImage( hwnd : hwnd; i : cint; const lpptUpLeft : POINT):HIMAGELIST;inline;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), LVM_CREATEDRAGIMAGE, WPARAM(i), LPARAM(@lpptUpLeft)))
end;


// Macro 74
// #define ListView_GetViewRect(hwnd, prc) \
//     (BOOL)SNDMSG((hwnd), LVM_GETVIEWRECT, 0, (LPARAM)(RECT *)(prc))

Function ListView_GetViewRect( hwnd : hwnd;var  prc : RECT ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETVIEWRECT, 0, LPARAM(@prc)))
end;

// Macro 75
// #define ListView_GetTextColor(hwnd)  \
//     (COLORREF)SNDMSG((hwnd), LVM_GETTEXTCOLOR, 0, DWord(0))

Function ListView_GetTextColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_GETTEXTCOLOR, 0, LPARAM(0)))
end;


// Macro 76
// #define ListView_SetTextColor(hwnd, clrText) \
//     (BOOL)SNDMSG((hwnd), LVM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText))

Function ListView_SetTextColor( hwnd : hwnd; clrText : COLORREF):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETTEXTCOLOR, 0, clrText))
end;


// Macro 77
// #define ListView_GetTextBkColor(hwnd)  \
//     (COLORREF)SNDMSG((hwnd), LVM_GETTEXTBKCOLOR, 0, DWord(0))

Function ListView_GetTextBkColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_GETTEXTBKCOLOR, 0, LPARAM(0)))
end;


// Macro 78
// #define ListView_SetTextBkColor(hwnd, clrTextBk) \
//     (BOOL)SNDMSG((hwnd), LVM_SETTEXTBKCOLOR, 0, (LPARAM)(COLORREF)(clrTextBk))

Function ListView_SetTextBkColor( hwnd : hwnd; clrTextBk : COLORREF):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETTEXTBKCOLOR, 0, clrTextBk))
end;


// Macro 79
// #define ListView_GetTopIndex(hwndLV) \
//     (int)SNDMSG((hwndLV), LVM_GETTOPINDEX, 0, 0)

Function ListView_GetTopIndex( hwndLV : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwndLV), LVM_GETTOPINDEX, 0, 0))
end;


// Macro 80
// #define ListView_GetCountPerPage(hwndLV) \
//     (int)SNDMSG((hwndLV), LVM_GETCOUNTPERPAGE, 0, 0)

Function ListView_GetCountPerPage( hwndLV : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwndLV), LVM_GETCOUNTPERPAGE, 0, 0))
end;


// Macro 81
//
// #define ListView_GetOrigin(hwndLV, ppt) \
//     (BOOL)SNDMSG((hwndLV), LVM_GETORIGIN, (WPARAM)0, (LPARAM)(POINT *)(ppt))

Function ListView_GetOrigin( hwndLV : hwnd;var  ppt : POINT ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_GETORIGIN, WPARAM(0), LPARAM(@ppt)))
end;


// Macro 82
// #define ListView_Update(hwndLV, i) \
//     (BOOL)SNDMSG((hwndLV), LVM_UPDATE, (WPARAM)(i), DWord(0))

Function ListView_Update( hwndLV : hwnd; i : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_UPDATE, (i), LPARAM(0)))
end;


// Macro 83
// #define ListView_SetItemState(hwndLV, i, data, mask) \
// { LV_ITEM _ms_lvi;\
//   _ms_lvi.stateMask = mask;\
//   _ms_lvi.state = data;\
//   SNDMSG((hwndLV), LVM_SETITEMSTATE, (WPARAM)(i), (LPARAM)(LV_ITEM *)&_ms_lvi);\
// }


Procedure ListView_SetItemState(hwndLV :hwnd; i :cint ;data,mask:UINT);

Var _ms_lvi : LV_ITEM;

Begin
 _ms_lvi.stateMask:=mask;
 _ms_lvi.state:=data;
 SendMessage(hwndLV, LVM_SETITEMSTATE, i,LPARAM(@_ms_lvi));
end;


// Macro 84
// #define ListView_SetCheckState(hwndLV, i, fCheck) \
//   ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)

Procedure ListView_SetCheckState( hwndLV : hwnd; i : cint ; fCheck : BOOL );

var j:longint;
Begin
 IF not fCheck Then  // (or inc(longint(fCheck)) if you like it dirty)
  j:=1
 Else
  j:=2;
ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK(j), LVIS_STATEIMAGEMASK);
end;


// Macro 85
// #define ListView_GetItemState(hwndLV, i, mask) \
//    (UINT)SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)(i), (LPARAM)(mask))

Function ListView_GetItemState( hwndLV : hwnd; i : WPARAM; mask : LPARAM):UINT;

Begin
 Result:=UINT(SendMessage((hwndLV), LVM_GETITEMSTATE, (i), mask))
end;


// Macro 86
// #define ListView_GetCheckState(hwndLV, i) \
//    ((((UINT)(SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)(i), LVIS_STATEIMAGEMASK))) >> 12) -1)

Function ListView_GetCheckState( hwndLV : hwnd; i : WPARAM):UINT;

Begin
 Result:=(SendMessage((hwndLV), LVM_GETITEMSTATE, (i), LVIS_STATEIMAGEMASK) shr 12) -1;
end;


// Macro 87
// #define ListView_GetItemText(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
// { LV_ITEM _ms_lvi;\
//   _ms_lvi.iSubItem = iSubItem_;\
//   _ms_lvi.cchTextMax = cchTextMax_;\
//   _ms_lvi.pszText = pszText_;\
//   SNDMSG((hwndLV), LVM_GETITEMTEXT, (WPARAM)(i), (LPARAM)(LV_ITEM *)&_ms_lvi);\
// }

Procedure ListView_GetItemText(hwndLV:hwnd; i : WPARAM ;iSubItem : cint ;pszText : Pointer;cchTextMax :cint);

Var _ms_lvi : LV_ITEM;

Begin
     _ms_lvi.iSubItem := iSubItem;
     _ms_lvi.cchTextMax := cchTextMax;
     _ms_lvi.pszText := pszText;        // unicode dependant!
    SendMessage(hwndLV, LVM_GETITEMTEXT,  i, LPARAM(@_ms_LVI));
end;


// Macro 88
// #define ListView_SetItemText(hwndLV, i, iSubItem_, pszText_) \
// { LV_ITEM _ms_lvi;\
//   _ms_lvi.iSubItem = iSubItem_;\
//   _ms_lvi.pszText = pszText_;\
//   SNDMSG((hwndLV), LVM_SETITEMTEXT, (WPARAM)(i), (LPARAM)(LV_ITEM *)&_ms_lvi);\
// }

Procedure ListView_SetItemText(hwndLV:hwnd; i : WPARAM ;iSubItem : cint ;pszText : Pointer);

Var _ms_lvi : LV_ITEM;

Begin
     _ms_lvi.iSubItem := iSubItem;
     _ms_lvi.pszText := pszText;        // unicode dependant!
    SendMessage(hwndLV, LVM_SETITEMTEXT,  i, LPARAM(@_ms_lvi));
End;

// Macro 89
// #define ListView_SetItemCount(hwndLV, cItems) \
//   SNDMSG((hwndLV), LVM_SETITEMCOUNT, (WPARAM)(cItems), 0)

Procedure ListView_SetItemCount( hwndLV : hwnd; cItems : WPARAM);

Begin
 SendMessage((hwndLV), LVM_SETITEMCOUNT, cItems, 0)
end;


// Macro 90
// #define ListView_SetItemCountEx(hwndLV, cItems, dwFlags) \
//   SNDMSG((hwndLV), LVM_SETITEMCOUNT, (WPARAM)(cItems), (LPARAM)(dwFlags))

Procedure ListView_SetItemCountEx( hwndLV : hwnd; cItems : WPARAM; dwFlags : LPARAM);

Begin
 SendMessage((hwndLV), LVM_SETITEMCOUNT, cItems, dwFlags)
end;


// Macro 91
// #define ListView_SortItems(hwndLV, _pfnCompare, _lPrm) \
//   (BOOL)SNDMSG((hwndLV), LVM_SORTITEMS, (WPARAM)(LPARAM)(_lPrm), \
//   (LPARAM)(PFNLVCOMPARE)(_pfnCompare))

Function ListView_SortItems( hwndLV : hwnd;pfnCompare : PFNLVCOMPARE; lPrm : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_SORTITEMS, lPrm,  LPARAM(@pfncompare)));
end;


// Macro 92
// #define ListView_SetItemPosition32(hwndLV, i, x0, y0) \
// {   POINT ptNewPos; \
//     ptNewPos.x = x0; ptNewPos.y = y0; \
//     SNDMSG((hwndLV), LVM_SETITEMPOSITION32, (WPARAM)(int)(i), (LPARAM)&ptNewPos); \
// }

Procedure ListView_SetItemPosition32(hwndLV:hwnd; i:cint;x0,y0:long);

Var ptNewPos:POINT;

Begin
 ptNewPos.X:=x0; ptNewPos.Y:=y0;
 SendMessage(hwndlv, LVM_SETITEMPOSITION32, I,LPARAM(@ptNewPos));
end;


// Macro 93
// #define ListView_GetSelectedCount(hwndLV) \
//     (UINT)SNDMSG((hwndLV), LVM_GETSELECTEDCOUNT, 0, DWord(0))

Function ListView_GetSelectedCount( hwndLV : hwnd):UINT;

Begin
 Result:=SendMessage(hwndLV, LVM_GETSELECTEDCOUNT, 0, LPARAM(0));
end;

// Macro 94
// #define ListView_GetItemSpacing(hwndLV, fSmall) \
//         (DWORD)SNDMSG((hwndLV), LVM_GETITEMSPACING, fSmall, DWord(0))

Function ListView_GetItemSpacing( hwndLV : hwnd; fSmall : cint ):DWORD;

Begin
 Result:=LPARAM(SendMessage((hwndLV), LVM_GETITEMSPACING, fSmall, LPARAM(0)));
end;

// Macro 95
// #define ListView_GetISearchString(hwndLV, lpsz) \
//         (BOOL)SNDMSG((hwndLV), LVM_GETISEARCHSTRING, 0, (LPARAM)(LPTSTR)(lpsz))

Function ListView_GetISearchString( hwndLV : hwnd; lpsz : LPTSTR):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_GETISEARCHSTRING, 0, LPARAM(lpsz)))
end;

// Macro 96
// #define ListView_SetIconSpacing(hwndLV, cx, cy) \
//         (DWORD)SNDMSG((hwndLV), LVM_SETICONSPACING, 0, MAKELONG(cx,cy))

Function ListView_SetIconSpacing( hwndLV : hwnd; cx,cy : cint ):DWORD;

Begin
 Result:=DWORD(SendMessage((hwndLV), LVM_SETICONSPACING, 0, MAKELONG(cx,cy)))
end;


// Macro 97
// #define ListView_SetExtendedListViewStyle(hwndLV, dw)\
//         (DWORD)SNDMSG((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)

Function ListView_SetExtendedListViewStyle( hwndLV : hwnd; dw :cint ):DWORD;

Begin
 Result:=DWORD(SendMessage((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw))
end;


// Macro 98
// #define ListView_SetExtendedListViewStyleEx(hwndLV, dwMask, dw)\
//         (DWORD)SNDMSG((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dw)

Function ListView_SetExtendedListViewStyleEx( hwndLV : hwnd; dwMask, dw : cint ):DWORD;

Begin
 Result:=DWORD(SendMessage((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dw))
end;


// Macro 99
// #define ListView_GetExtendedListViewStyle(hwndLV)\
//         (DWORD)SNDMSG((hwndLV), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0)

Function ListView_GetExtendedListViewStyle( hwndLV : hwnd):DWORD;

Begin
 Result:=DWORD(SendMessage((hwndLV), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0))
end;


// Macro 100
// #define ListView_GetSubItemRect(hwnd, iItem, iSubItem, code, prc) \
//         (BOOL)SNDMSG((hwnd), LVM_GETSUBITEMRECT, (WPARAM)(int)(iItem), \
//                 ((prc) ? ((((LPRECT)(prc))->top = iSubItem), (((LPRECT)(prc))->left = code), (LPARAM)(prc)) : (LPARAM)(LPRECT)NULL))

Function ListView_GetSubItemRect( hwnd : hwnd; iItem : cint;iSubItem, code :cint ;prc:LPRECT):BOOL;

VAR LastParam : LPRECT;
Begin
 if prc<>nil then
    begin
       prc^.top:=iSubItem;
       prc^.left:=code;
       lastparam:=prc;
    end
 else
    lastparam:=nil;
 Result:=bool(SendMessage((hwnd), LVM_GETSUBITEMRECT, iItem, LPARAM(lastparam)));
End;

// Macro 101
// #define ListView_SubItemHitTest(hwnd, plvhti) \
//         (int)SNDMSG((hwnd), LVM_SUBITEMHITTEST, 0, (LPARAM)(LPLVHITTESTINFO)(plvhti))

Function ListView_SubItemHitTest( hwnd : hwnd; plvhti : LPLVHITTESTINFO):cint;
Begin
 Result:=cint(SendMessage((hwnd), LVM_SUBITEMHITTEST, 0, LParam(plvhti)))
end;


// Macro 102
// #define ListView_SetColumnOrderArray(hwnd, iCount, pi) \
//         (BOOL)SNDMSG((hwnd), LVM_SETCOLUMNORDERARRAY, (WPARAM)(iCount), (LPARAM)(LPINT)(pi))

Function ListView_SetColumnOrderArray( hwnd : hwnd; iCount : WPARAM; pi : LPINT):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETCOLUMNORDERARRAY, iCount, LPARAM(pi)));
end;


// Macro 103
// #define ListView_GetColumnOrderArray(hwnd, iCount, pi) \
//         (BOOL)SNDMSG((hwnd), LVM_GETCOLUMNORDERARRAY, (WPARAM)(iCount), (LPARAM)(LPINT)(pi))

Function ListView_GetColumnOrderArray( hwnd : hwnd; iCount : WPARAM; pi : LPINT):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETCOLUMNORDERARRAY, iCount, LPARAM(pi)))
end;


// Macro 104
// #define ListView_SetHotItem(hwnd, i) \
//         (int)SNDMSG((hwnd), LVM_SETHOTITEM, (WPARAM)(i), 0)

Function ListView_SetHotItem( hwnd : hwnd; i : WPARAM):cint;
Begin
 Result:=cint(SendMessage((hwnd), LVM_SETHOTITEM, (i), 0))
end;

// Macro 105
// #define ListView_GetHotItem(hwnd) \
//         (int)SNDMSG((hwnd), LVM_GETHOTITEM, 0, 0)

Function ListView_GetHotItem( hwnd : hwnd):cint;
Begin
 Result:=cint(SendMessage((hwnd), LVM_GETHOTITEM, 0, 0))
end;

// Macro 106
// #define ListView_SetHotCursor(hwnd, hcur) \
//         (HCURSOR)SNDMSG((hwnd), LVM_SETHOTCURSOR, 0, (LPARAM)(hcur))

Function ListView_SetHotCursor( hwnd : hwnd; hcur : LPARAM):HCURSOR;
Begin
 Result:=HCURSOR(SendMessage((hwnd), LVM_SETHOTCURSOR, 0, hcur))
end;

// Macro 107
// #define ListView_GetHotCursor(hwnd) \
//         (HCURSOR)SNDMSG((hwnd), LVM_GETHOTCURSOR, 0, 0)

Function ListView_GetHotCursor( hwnd : hwnd):HCURSOR;
Begin
 Result:=HCURSOR(SendMessage((hwnd), LVM_GETHOTCURSOR, 0, 0))
end;


// Macro 108
// #define ListView_ApproximateViewRect(hwnd, iWidth, iHeight, iCount) \
//         (DWORD)SNDMSG((hwnd), LVM_APPROXIMATEVIEWRECT, iCount, MAKELPARAM(iWidth, iHeight))

Function ListView_ApproximateViewRect( hwnd : hwnd; iWidth, iHeight, iCount : cint ):DWORD;
Begin
 Result:=DWORD(SendMessage((hwnd), LVM_APPROXIMATEVIEWRECT, iCount, MAKELPARAM(iWidth, iHeight)));
end;


// Macro 109
// #define ListView_SetWorkAreas(hwnd, nWorkAreas, prc) \
//     (BOOL)SNDMSG((hwnd), LVM_SETWORKAREAS, (WPARAM)(int)(nWorkAreas), (LPARAM)(RECT *)(prc))

Function ListView_SetWorkAreas( hwnd : hwnd; nWorkAreas : cint;var prc : RECT ):BOOL;inline;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETWORKAREAS, nWorkAreas, LPARAM(@prc)))
end;


Function ListView_SetWorkAreas( hwnd : hwnd; nWorkAreas : cint;  prc : PRECT ):BOOL;inline;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETWORKAREAS, nWorkAreas, LPARAM(prc)))
end;

// Macro 110
// #define ListView_GetWorkAreas(hwnd, nWorkAreas, prc) \
//     (BOOL)SNDMSG((hwnd), LVM_GETWORKAREAS, (WPARAM)(int)(nWorkAreas), (LPARAM)(RECT *)(prc))

Function ListView_GetWorkAreas( hwnd : hwnd; nWorkAreas : cint;var  prc : RECT ):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETWORKAREAS, nWorkAreas, LPARAM(@prc)))
end;


// Macro 111
// #define ListView_GetNumberOfWorkAreas(hwnd, pnWorkAreas) \
//     (BOOL)SNDMSG((hwnd), LVM_GETNUMBEROFWORKAREAS, 0, (LPARAM)(UINT *)(pnWorkAreas))

Function ListView_GetNumberOfWorkAreas( hwnd : hwnd;var pnWorkAreas : UINT ):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETNUMBEROFWORKAREAS, 0, LPARAM(@pnWorkAreas)))
end;


// Macro 112
// #define ListView_GetSelectionMark(hwnd) \
//     (int)SNDMSG((hwnd), LVM_GETSELECTIONMARK, 0, 0)

Function ListView_GetSelectionMark( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_GETSELECTIONMARK, 0, 0))
end;


// Macro 113
// #define ListView_SetSelectionMark(hwnd, i) \
//     (int)SNDMSG((hwnd), LVM_SETSELECTIONMARK, 0, (LPARAM)(i))

Function ListView_SetSelectionMark( hwnd : hwnd; i : LPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), LVM_SETSELECTIONMARK, 0, (i)))
end;


// Macro 114
// #define ListView_SetHoverTime(hwndLV, dwHoverTimeMs)\
//         (DWORD)SNDMSG((hwndLV), LVM_SETHOVERTIME, 0, (LPARAM)(dwHoverTimeMs))

Function ListView_SetHoverTime( hwndLV : hwnd; dwHoverTimeMs : LPARAM):DWORD;

Begin
 Result:=DWORD(SendMessage((hwndLV), LVM_SETHOVERTIME, 0, dwHoverTimeMs))
end;


// Macro 115
// #define ListView_GetHoverTime(hwndLV)\
//         (DWORD)SNDMSG((hwndLV), LVM_GETHOVERTIME, 0, 0)

Function ListView_GetHoverTime( hwndLV : hwnd):DWORD;

Begin
 Result:=DWORD(SendMessage((hwndLV), LVM_GETHOVERTIME, 0, 0))
end;


// Macro 116
// #define ListView_SetToolTips(hwndLV, hwndNewHwnd)\
//         (HWND)SNDMSG((hwndLV), LVM_SETTOOLTIPS, (WPARAM)(hwndNewHwnd), 0)

Function ListView_SetToolTips( hwndLV : hwnd; hwndNewHwnd : WPARAM):HWND;

Begin
 Result:=HWND(SendMessage((hwndLV), LVM_SETTOOLTIPS, hwndNewHwnd, 0))
end;


// Macro 117
// #define ListView_GetToolTips(hwndLV)\
//         (HWND)SNDMSG((hwndLV), LVM_GETTOOLTIPS, 0, 0)

Function ListView_GetToolTips( hwndLV : hwnd):HWND;

Begin
 Result:=HWND(SendMessage((hwndLV), LVM_GETTOOLTIPS, 0, 0))
end;


// Macro 118
// #define ListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
//   (BOOL)SNDMSG((hwndLV), LVM_SORTITEMSEX, (WPARAM)(LPARAM)(_lPrm), (LPARAM)(PFNLVCOMPARE)(_pfnCompare))

Function ListView_SortItemsEx( hwndLV : hwnd; _pfnCompare : PFNLVCOMPARE; _lPrm : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_SORTITEMSEX, _lPrm, LPAram(@_pfnCompare)))
end;

{$ifdef win32xp}
// Macro 119
// #define ListView_SetSelectedColumn(hwnd, iCol) \
//     SNDMSG((hwnd), LVM_SETSELECTEDCOLUMN, (WPARAM)iCol, 0)

Procedure ListView_SetSelectedColumn( hwnd : hwnd; iCol : WPARAM);

Begin
 SendMessage((hwnd), LVM_SETSELECTEDCOLUMN, iCol, 0)
end;

// Macro 120
// #define ListView_SetTileWidth(hwnd, cpWidth) \
//     SNDMSG((hwnd), LVM_SETTILEWIDTH, (WPARAM)cpWidth, 0)

Procedure ListView_SetTileWidth( hwnd : hwnd; cpWidth : WPARAM);

Begin
 SendMessage((hwnd), LVM_SETTILEWIDTH, cpWidth, 0)
end;


// Macro 121
// #define ListView_SetView(hwnd, iView) \
//     (DWORD)SNDMSG((hwnd), LVM_SETVIEW, (WPARAM)(DWORD)iView, 0)

Function ListView_SetView( hwnd : hwnd; iView : DWORD):DWORD;

Begin
 Result:=DWORD(SendMessage((hwnd), LVM_SETVIEW, iView, 0))
end;


// Macro 122
// #define ListView_GetView(hwnd) \
//     (DWORD)SNDMSG((hwnd), LVM_GETVIEW, 0, 0)

Function ListView_GetView( hwnd : hwnd):DWORD;
Begin
 Result:=DWORD(SendMessage((hwnd), LVM_GETVIEW, 0, 0))
end;


// Macro 123
// #define ListView_InsertGroup(hwnd, index, pgrp) \
//     SNDMSG((hwnd), LVM_INSERTGROUP, (WPARAM)index, (LPARAM)pgrp)

Procedure ListView_InsertGroup( hwnd : hwnd; index : WPARAM; pgrp : LPARAM);
Begin
 SendMessage((hwnd), LVM_INSERTGROUP, index, pgrp)
end;


// Macro 124
// #define ListView_SetGroupInfo(hwnd, iGroupId, pgrp) \
//     SNDMSG((hwnd), LVM_SETGROUPINFO, (WPARAM)iGroupId, (LPARAM)pgrp)

Procedure ListView_SetGroupInfo( hwnd : hwnd; iGroupId : WPARAM; pgrp : LPARAM);
Begin
 SendMessage((hwnd), LVM_SETGROUPINFO, iGroupId, pgrp)
end;


// Macro 125
// #define ListView_GetGroupInfo(hwnd, iGroupId, pgrp) \
//     SNDMSG((hwnd), LVM_GETGROUPINFO, (WPARAM)iGroupId, (LPARAM)pgrp)

Procedure ListView_GetGroupInfo( hwnd : hwnd; iGroupId : WPARAM; pgrp : LPARAM);
Begin
 SendMessage((hwnd), LVM_GETGROUPINFO, iGroupId, pgrp)
end;


// Macro 126
// #define ListView_RemoveGroup(hwnd, iGroupId) \
//     SNDMSG((hwnd), LVM_REMOVEGROUP, (WPARAM)iGroupId, 0)

Procedure ListView_RemoveGroup( hwnd : hwnd; iGroupId : WPARAM);
Begin
 SendMessage((hwnd), LVM_REMOVEGROUP, iGroupId, 0)
end;


// Macro 127
// #define ListView_MoveGroup(hwnd, iGroupId, toIndex) \
//     SNDMSG((hwnd), LVM_MOVEGROUP, (WPARAM)iGroupId, (LPARAM)toIndex)

Procedure ListView_MoveGroup( hwnd : hwnd; iGroupId : WPARAM; toIndex : LPARAM);
Begin
 SendMessage((hwnd), LVM_MOVEGROUP, iGroupId, toIndex)
end;


// Macro 128
// #define ListView_MoveItemToGroup(hwnd, idItemFrom, idGroupTo) \
//     SNDMSG((hwnd), LVM_MOVEITEMTOGROUP, (WPARAM)idItemFrom, (LPARAM)idGroupTo)

Procedure ListView_MoveItemToGroup( hwnd : hwnd; idItemFrom : WPARAM; idGroupTo : LPARAM);

Begin
 SendMessage((hwnd), LVM_MOVEITEMTOGROUP, idItemFrom, idGroupTo)
end;


// Macro 129
// #define ListView_SetGroupMetrics(hwnd, pGroupMetrics) \
//     SNDMSG((hwnd), LVM_SETGROUPMETRICS, 0, (LPARAM)pGroupMetrics)

Procedure ListView_SetGroupMetrics( hwnd : hwnd; pGroupMetrics : LPARAM);

Begin
 SendMessage((hwnd), LVM_SETGROUPMETRICS, 0, pGroupMetrics)
end;


// Macro 130
// #define ListView_GetGroupMetrics(hwnd, pGroupMetrics) \
//     SNDMSG((hwnd), LVM_GETGROUPMETRICS, 0, (LPARAM)pGroupMetrics)

Procedure ListView_GetGroupMetrics( hwnd : hwnd; pGroupMetrics : LPARAM);

Begin
SendMessage((hwnd), LVM_GETGROUPMETRICS, 0, pGroupMetrics)
end;


// Macro 131
// #define ListView_EnableGroupView(hwnd, fEnable) \
//     SNDMSG((hwnd), LVM_ENABLEGROUPVIEW, (WPARAM)fEnable, 0)

Procedure ListView_EnableGroupView( hwnd : hwnd; fEnable : WPARAM);

Begin
 SendMessage((hwnd), LVM_ENABLEGROUPVIEW, fEnable, 0)
end;


// Macro 132
// #define ListView_SortGroups(hwnd, _pfnGroupCompate, _plv) \
//     SNDMSG((hwnd), LVM_SORTGROUPS, (WPARAM)_pfnGroupCompate, (LPARAM)_plv)

Procedure ListView_SortGroups( hwnd : hwnd; _pfnGroupCompate : WPARAM; _plv : LPARAM);

Begin
 SendMessage((hwnd), LVM_SORTGROUPS, _pfnGroupCompate, _plv)
end;

// Macro 133
// #define ListView_InsertGroupSorted(hwnd, structInsert) \
//     SNDMSG((hwnd), LVM_INSERTGROUPSORTED, (WPARAM)structInsert, 0)

Procedure ListView_InsertGroupSorted( hwnd : hwnd; structInsert : WPARAM);

Begin
 SendMessage((hwnd), LVM_INSERTGROUPSORTED, structInsert, 0)
end;


// Macro 134
// #define ListView_RemoveAllGroups(hwnd) \
//     SNDMSG((hwnd), LVM_REMOVEALLGROUPS, 0, 0)

Procedure ListView_RemoveAllGroups( hwnd : hwnd);

Begin
SendMessage((hwnd), LVM_REMOVEALLGROUPS, 0, 0)
end;


// Macro 135
// #define ListView_HasGroup(hwnd, dwGroupId) \
//     SNDMSG((hwnd), LVM_HASGROUP, dwGroupId, 0)

Procedure ListView_HasGroup( hwnd : hwnd; dwGroupId :DWORD );

Begin
 SendMessage((hwnd), LVM_HASGROUP, dwGroupId, 0)
end;


// Macro 136
// #define ListView_SetTileViewInfo(hwnd, ptvi) \
//     SNDMSG((hwnd), LVM_SETTILEVIEWINFO, 0, (LPARAM)ptvi)

Procedure ListView_SetTileViewInfo( hwnd : hwnd; ptvi : LPARAM);

Begin
 SendMessage((hwnd), LVM_SETTILEVIEWINFO, 0, ptvi)
end;


// Macro 137
// #define ListView_GetTileViewInfo(hwnd, ptvi) \
//     SNDMSG((hwnd), LVM_GETTILEVIEWINFO, 0, (LPARAM)ptvi)

Procedure ListView_GetTileViewInfo( hwnd : hwnd; ptvi : LPARAM);

Begin
 SendMessage((hwnd), LVM_GETTILEVIEWINFO, 0, ptvi)
end;


// Macro 138
// #define ListView_SetTileInfo(hwnd, pti) \
//     SNDMSG((hwnd), LVM_SETTILEINFO, 0, (LPARAM)pti)

Procedure ListView_SetTileInfo( hwnd : hwnd; pti : LPARAM);

Begin
 SendMessage((hwnd), LVM_SETTILEINFO, 0, pti)
end;


// Macro 139
// #define ListView_GetTileInfo(hwnd, pti) \
//     SNDMSG((hwnd), LVM_GETTILEINFO, 0, (LPARAM)pti)

Procedure ListView_GetTileInfo( hwnd : hwnd; pti : LPARAM);

Begin
 SendMessage((hwnd), LVM_GETTILEINFO, 0, pti)
end;


// Macro 140
// #define ListView_SetInsertMark(hwnd, lvim) \
//     (BOOL)SNDMSG((hwnd), LVM_SETINSERTMARK, (WPARAM) 0, (LPARAM) (lvim))

Function ListView_SetInsertMark( hwnd : hwnd; lvim : lparam ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETINSERTMARK, 0, lvim));
end;


// Macro 141
// #define ListView_GetInsertMark(hwnd, lvim) \
//     (BOOL)SNDMSG((hwnd), LVM_GETINSERTMARK, (WPARAM) 0, (LPARAM) (lvim))

Function ListView_GetInsertMark( hwnd : hwnd; lvim : lparam ):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETINSERTMARK, WPARAM (0), LPARAM (lvim)));
end;


// Macro 142
// #define ListView_InsertMarkHitTest(hwnd, point, lvim) \
//    (int)SNDMSG((hwnd), LVM_INSERTMARKHITTEST, (WPARAM)(LPPOINT)(point), (LPARAM)(LPLVINSERTMARK)(lvim))

Function ListView_InsertMarkHitTest( hwnd : hwnd; point : LPPOINT; lvim : LPLVINSERTMARK):cint;
Begin
 Result:=cint(SendMessage((hwnd), LVM_INSERTMARKHITTEST, wparam(point), lparam(lvim)));
end;


// Macro 143
// #define ListView_GetInsertMarkRect(hwnd, rc) \
//     (int)SNDMSG((hwnd), LVM_GETINSERTMARKRECT, (WPARAM)0, (LPARAM)(LPRECT)(rc))

Function ListView_GetInsertMarkRect( hwnd : hwnd; rc : LPRECT):cint;
Begin
 Result:=cint(SendMessage((hwnd), LVM_GETINSERTMARKRECT, WPARAM(0), LPARAM(rc)))
end;


// Macro 144
// #define ListView_SetInsertMarkColor(hwnd, color) \
//     (COLORREF)SNDMSG((hwnd), LVM_SETINSERTMARKCOLOR, (WPARAM)0, (LPARAM)(COLORREF)(color))

Function ListView_SetInsertMarkColor( hwnd : hwnd; color : COLORREF):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_SETINSERTMARKCOLOR, WPARAM(0), color));
end;

// Macro 145
// #define ListView_GetInsertMarkColor(hwnd) \
//     (COLORREF)SNDMSG((hwnd), LVM_GETINSERTMARKCOLOR, (WPARAM)0, (LPARAM)0)

Function ListView_GetInsertMarkColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_GETINSERTMARKCOLOR, WPARAM(0), LPARAM(0)));
end;

// Macro 146
// #define ListView_SetInfoTip(hwndLV, plvInfoTip)\
//         (BOOL)SNDMSG((hwndLV), LVM_SETINFOTIP, (WPARAM)0, (LPARAM)plvInfoTip)

Function ListView_SetInfoTip( hwndLV : hwnd; plvInfoTip : LPARAM):BOOL;
Begin
 Result:=BOOL(SendMessage((hwndLV), LVM_SETINFOTIP, WPARAM(0), plvInfoTip));
end;

// Macro 147
// #define ListView_GetSelectedColumn(hwnd) \
//     (UINT)SNDMSG((hwnd), LVM_GETSELECTEDCOLUMN, 0, 0)

Function ListView_GetSelectedColumn( hwnd : hwnd):UINT;
Begin
 Result:=UINT(SendMessage((hwnd), LVM_GETSELECTEDCOLUMN, 0, 0));
end;

// Macro 148
// #define ListView_IsGroupViewEnabled(hwnd) \
//     (BOOL)SNDMSG((hwnd), LVM_ISGROUPVIEWENABLED, 0, 0)

Function ListView_IsGroupViewEnabled( hwnd : hwnd):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_ISGROUPVIEWENABLED, 0, 0));
end;

// Macro 149
// #define ListView_GetOutlineColor(hwnd) \
//     (COLORREF)SNDMSG((hwnd), LVM_GETOUTLINECOLOR, 0, 0)

Function ListView_GetOutlineColor( hwnd : hwnd):COLORREF;
Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_GETOUTLINECOLOR, 0, 0));
end;


// Macro 150
// #define ListView_SetOutlineColor(hwnd, color) \
//     (COLORREF)SNDMSG((hwnd), LVM_SETOUTLINECOLOR, (WPARAM)0, (LPARAM)(COLORREF)(color))

Function ListView_SetOutlineColor( hwnd : hwnd; color : COLORREF):COLORREF;
Begin
 Result:=COLORREF(SendMessage((hwnd), LVM_SETOUTLINECOLOR, WPARAM(0), color));
end;

// Macro 151
// #define ListView_CancelEditLabel(hwnd) \
//     (VOID)SNDMSG((hwnd), LVM_CANCELEDITLABEL, (WPARAM)0, (LPARAM)0)

procedure ListView_CancelEditLabel( hwnd : hwnd);
Begin
 SendMessage((hwnd), LVM_CANCELEDITLABEL, WPARAM(0), LPARAM(0));
end;

// Macro 152
// #define ListView_MapIndexToID(hwnd, index) \
//     (UINT)SNDMSG((hwnd), LVM_MAPINDEXTOID, (WPARAM)index, (LPARAM)0)

Function ListView_MapIndexToID( hwnd : hwnd; index : WPARAM):UINT;

Begin
 Result:=UINT(SendMessage((hwnd), LVM_MAPINDEXTOID, index, LPARAM(0)));
end;

// Macro 153
// #define ListView_MapIDToIndex(hwnd, id) \
//     (UINT)SNDMSG((hwnd), LVM_MAPIDTOINDEX, (WPARAM)id, (LPARAM)0)

Function ListView_MapIDToIndex( hwnd : hwnd; id : WPARAM):UINT;
Begin
 Result:=UINT(SendMessage((hwnd), LVM_MAPIDTOINDEX, id, LPARAM(0)));
end;

function  ListView_IsItemVisible(hwnd:hwnd; aindex:cuint):cuint;
begin
 Result:=UINT(SendMessage((hwnd),LVM_ISITEMVISIBLE, WPARAM(aindex), LPARAM(0)));
end;
{$ENDIF}

{$ifdef win32vista}

function ListView_SetGroupHeaderImageList(hwnd:HWNd;himl:HIMAGELIST):HIMAGELIST;
begin
 Result:=HIMAGELIST(SendMessage((hwnd),LVM_SETIMAGELIST, WPARAM(LVSIL_GROUPHEADER), LPARAM(HIMAGELIST((himl)))));
end;

function ListView_GetGroupHeaderImageList(hwnd:HWND):HIMAGELIST;
begin
 Result:=HIMAGELIST(SendMessage((hwnd),LVM_GETIMAGELIST, WPARAM(LVSIL_GROUPHEADER),LPARAM(0)));
end;

function ListView_GetEmptyText(hwnd:HWND;pszText:LPWSTR; cchText:CUINT):BOOL;
begin
 Result:=BOOL(SendMessage((hwnd),LVM_GETEMPTYTEXT, WPARAM(cchText), LPARAM(pszText)));
end;

function ListView_GetFooterRect(hwnd:HWND; prc:PRECT):BOOL;
begin
 Result:=BOOL(SendMessage((hwnd),LVM_GETFOOTERRECT, WPARAM(0), LPARAM(prc)));
end;

function ListView_GetFooterInfo(hwnd:HWND;plvfi: LPLVFOOTERINFO ):BOOL;
begin
 Result:=BOOL(SendMessage((hwnd),LVM_GETFOOTERINFO, WPARAM(0), LPARAM(plvfi)));
end;

function ListView_GetFooterItemRect(hwnd:HWND;iItem:CUINT;prc:PRECT):BOOL;
begin
 Result:=BOOL(SendMessage((hwnd),LVM_GETFOOTERITEMRECT, WPARAM(iItem), LPARAM(prc)));
end;

function ListView_GetFooterItem(hwnd:HWND;iItem:CUINT; pfi:PLVFOOTERITEM):BOOL;
begin
 Result:=BOOL(SendMessage((hwnd),LVM_GETFOOTERITEM, WPARAM(iItem), LPARAM(pfi)));
end;

// (hwnd), LVM_GETITEMINDEXRECT, (WPARAM)(LVITEMINDEX*)(plvii), \
//                ((prc) ? ((((LPRECT)(prc))->top = (iSubItem)), (((LPRECT)(prc))->left = (code)), (LPARAM)(prc)) : (LPARAM)(LPRECT)NULL))

function ListView_GetItemIndexRect(hwnd:hwnd; plvii:PLVITEMINDEX; iSubItem:clong; code:clong; prc:LPRECT) :BOOL;
begin
 if assigned(prc) then
  begin
   prc^.top:=iSubItem;
   prc^.left:=code;
  end;
 Result:=BOOL(SendMessage((hwnd), LVM_GETITEMINDEXRECT, WPARAM(pLVITEMINDEX(plvii)), LPARAM(PRC)));
end;

function ListView_SetItemIndexState(hwndLV:HWND; plvii:PLVITEMINDEX; data:CUINT; mask:CUINT):HRESULT;
 var macro_lvi: LV_ITEM ;
begin
  macro_lvi.stateMask := (mask);
  macro_lvi.state := (data);
  Result:=HRESULT(SendMessage((hwndLV),LVM_SETITEMINDEXSTATE, WPARAM(pLVITEMINDEX(plvii)), LPARAM(PLV_ITEM(@macro_lvi))));
end;

function ListView_GetNextItemIndex(hwnd:HWND;plvii:PLVITEMINDEX; flags:LPARAM):BOOL;
begin
 Result:=BOOL(SendMessage((hwnd),LVM_GETNEXTITEMINDEX, WPARAM(pLVITEMINDEX(plvii)), MAKELPARAM(flags, 0)));
end;
{$endif}

// Macro 154
// #define ListView_SetBkImage(hwnd, plvbki) \
//     (BOOL)SNDMSG((hwnd), LVM_SETBKIMAGE, 0, (LPARAM)(plvbki))

Function ListView_SetBkImage( hwnd : hwnd; plvbki : LPARAM):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_SETBKIMAGE, 0, plvbki));
end;

// Macro 155
// #define ListView_GetBkImage(hwnd, plvbki) \
//     (BOOL)SNDMSG((hwnd), LVM_GETBKIMAGE, 0, (LPARAM)(plvbki))

Function ListView_GetBkImage( hwnd : hwnd; plvbki : LPARAM):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), LVM_GETBKIMAGE, 0, plvbki));
end;

// Macro 156
// #define TreeView_InsertItem(hwnd, lpis) \
//     (HTREEITEM)SNDMSG((hwnd), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(lpis))

Function TreeView_InsertItem( hwnd : hwnd; lpis : LPTV_INSERTSTRUCT):HTREEITEM;inline;
Begin
 Result:=HTREEITEM(SendMessage((hwnd), TVM_INSERTITEM, 0, LPARAM(lpis)));
end;


Function TreeView_InsertItem( hwnd : hwnd; const lpis : TV_INSERTSTRUCT):HTREEITEM;inline;
Begin
 Result:=HTREEITEM(SendMessage((hwnd), TVM_INSERTITEM, 0, LPARAM(@lpis)));
end;

// Macro 157
// #define TreeView_DeleteItem(hwnd, hitem) \
//     (BOOL)SNDMSG((hwnd), TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)(hitem))

Function TreeView_DeleteItem( hwnd : hwnd; hitem : HTREEITEM):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), TVM_DELETEITEM, 0, LPARAM(hitem)));
end;


// Macro 158
// #define TreeView_DeleteAllItems(hwnd) \
//     (BOOL)SNDMSG((hwnd), TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT)

Function TreeView_DeleteAllItems( hwnd : hwnd):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), TVM_DELETEITEM, 0, LPARAM(HTREEITEM(TVI_ROOT))));
end;


// Macro 159
// #define TreeView_Expand(hwnd, hitem, code) \
//     (BOOL)SNDMSG((hwnd), TVM_EXPAND, (WPARAM)(code), (LPARAM)(HTREEITEM)(hitem))

Function TreeView_Expand( hwnd : hwnd; hitem : HTREEITEM; code : WPARAM):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), TVM_EXPAND, code, lparam(hitem)))
end;


// Macro 160
// #define TreeView_GetItemRect(hwnd, hitem, prc, code) \
//     (*(HTREEITEM *)prc = (hitem), (BOOL)SNDMSG((hwnd), TVM_GETITEMRECT, (WPARAM)(code), (LPARAM)(RECT *)(prc)))

Function TreeView_GetItemRect( hwnd : hwnd; hitem: HTREEITEM; code : WPARAM; prc : pRECT):BOOL;inline;
Begin
 HTREEITEM(prc):=HITEM;
 Result:=Bool(SendMessage((hwnd), TVM_GETITEMRECT, code, LPARAM(prc)));
end;

Function TreeView_GetItemRect( hwnd : hwnd; hitem: HTREEITEM; var prc : TRECT;code : Bool):BOOL;inline;
Begin
 HTREEITEM(Pointer(@prc)^):=HITEM;
 Result:=Bool(SendMessage((hwnd), TVM_GETITEMRECT, WPARAM(code), LPARAM(@prc)));
end;

// Macro 161
// #define TreeView_GetCount(hwnd) \
//     (UINT)SNDMSG((hwnd), TVM_GETCOUNT, 0, 0)

Function TreeView_GetCount( hwnd : hwnd):UINT;
Begin
 Result:=UINT(SendMessage((hwnd), TVM_GETCOUNT, 0, 0))
end;

// Macro 162
// #define TreeView_GetIndent(hwnd) \
//     (UINT)SNDMSG((hwnd), TVM_GETINDENT, 0, 0)

Function TreeView_GetIndent( hwnd : hwnd):UINT;
Begin
 Result:=UINT(SendMessage((hwnd), TVM_GETINDENT, 0, 0))
end;


// Macro 163
// #define TreeView_SetIndent(hwnd, indent) \
//     (BOOL)SNDMSG((hwnd), TVM_SETINDENT, (WPARAM)(indent), 0)

Function TreeView_SetIndent( hwnd : hwnd; indent : WPARAM):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SETINDENT, indent, 0))
end;


// Macro 164
// #define TreeView_GetImageList(hwnd, iImage) \
//     (HIMAGELIST)SNDMSG((hwnd), TVM_GETIMAGELIST, iImage, 0)

Function TreeView_GetImageList( hwnd : hwnd; iImage : cint ):HIMAGELIST;
Begin
 Result:=HIMAGELIST(SendMessage((hwnd), TVM_GETIMAGELIST, iImage, 0))
end;


// Macro 165
// #define TreeView_SetImageList(hwnd, himl, iImage) \
//     (HIMAGELIST)SNDMSG((hwnd), TVM_SETIMAGELIST, iImage, (LPARAM)(HIMAGELIST)(himl))

Function TreeView_SetImageList( hwnd : hwnd; himl : HIMAGELIST; iImage : cint ):HIMAGELIST;
Begin
  Result:=HIMAGELIST(SendMessage((hwnd), TVM_SETIMAGELIST, iImage, himl))
end;

// Macro 166
// #define TreeView_GetNextItem(hwnd, hitem, code) \
//     (HTREEITEM)SNDMSG((hwnd), TVM_GETNEXTITEM, (WPARAM)(code), (LPARAM)(HTREEITEM)(hitem))

Function TreeView_GetNextItem( hwnd : hwnd; hitem : HTREEITEM; code : WPARAM):HTREEITEM;

Begin
  Result:=HTREEITEM(SendMessage((hwnd), TVM_GETNEXTITEM, code, lparam(hitem)))
end;


// Macro 167

function TreeView_GetChild(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;

Begin
  Result:=TreeView_GetNextItem(hwnd, hitem, TVGN_CHILD)
End;
// Macro 168

// #define TreeView_GetNextSibling(hwnd:hwnd; hitem:HTREEITEM);
// (hwnd, hitem)
// TreeView_GetNextItem(hwnd, hitem, TVGN_NEXT)



function TreeView_GetNextSibling(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;
// (hwnd, hitem)
// TreeView_GetNextItem(hwnd, hitem, TVGN_NEXT)

Begin
  Result:=TreeView_getNextItem(hwnd,hitem,TVGN_NEXT);
end;

// Macro 169
function TreeView_GetPrevSibling(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;
begin
  Result:=TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUS);
end;

// Macro 170

function TreeView_GetParent(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;
begin
  Result:=TreeView_GetNextItem(hwnd, hitem, TVGN_PARENT)
end;

// Macro 171
// #define TreeView_GetFirstVisible(hwnd:hwnd);
// TreeView_GetNextItem(hwnd, NULL,  TVGN_FIRSTVISIBLE)

function TreeView_GetFirstVisible(hwnd:hwnd) : HTREEITEM;inline;
begin
  Result:=TreeView_GetNextItem(hwnd, NIL,  TVGN_FIRSTVISIBLE)
end;

// Macro 172

//#define TreeView_GetNextVisible(hwnd:hwnd; hitem:HTREEITEM);
//(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_NEXTVISIBLE)
function TreeView_GetNextVisible(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
begin
  Result:=TreeView_GetNextItem(hwnd, hitem, TVGN_NEXTVISIBLE)
end;

// Macro 173
//#define TreeView_GetPrevVisible(hwnd:hwnd; hitem:HTREEITEM);
// (hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUSVISIBLE)


function TreeView_GetPrevVisible(hwnd:hwnd; hitem:HTREEITEM) : HTREEITEM;inline;
Begin
  Result:=TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUSVISIBLE);
end;

// Macro 174

function TreeView_GetSelection(hwnd:hwnd) : HTREEITEM;inline;
begin
  Result:=TreeView_GetNextItem(hwnd, NIL,  TVGN_CARET);
end;

// Macro 175

//#define TreeView_GetDropHilight(hwnd:hwnd);
//TreeView_GetNextItem(hwnd, NULL,  TVGN_DROPHILITE)

function TreeView_GetDropHilight(hwnd:hwnd) : HTREEITEM;inline;

begin
  Result:=TreeView_GetNextItem(hwnd, NIL,  TVGN_DROPHILITE);
end;

function TreeView_GetDropHilite(hwnd:hwnd) : HTREEITEM;inline;

begin
  Result:=TreeView_GetNextItem(hwnd, NIL,  TVGN_DROPHILITE);
end;


// Macro 176

function TreeView_GetRoot(hwnd:hwnd) : HTREEITEM;inline;

begin
  Result:=TreeView_GetNextItem(hwnd, NIL,  TVGN_ROOT);
end;

// Macro 177
//#define TreeView_GetLastVisible(hwnd:hwnd);
//TreeView_GetNextItem(hwnd, NULL,  TVGN_LASTVISIBLE)

function TreeView_GetLastVisible(hwnd:hwnd) : HTREEITEM;inline;
begin
  Result:=TreeView_GetNextItem(hwnd, NIL,  TVGN_LASTVISIBLE)
end;

// Macro 178


Function TreeView_Select( hwnd : hwnd; hitem : HTREEITEM; code : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SELECTITEM, code, LPARAM(hitem)))
end;

// Macro 179
// #define TreeView_SelectItem(hwnd, hitem)            TreeView_Select(hwnd, hitem, TVGN_CARET)
// Macro 180
// #define TreeView_SelectDropTarget(hwnd, hitem)      TreeView_Select(hwnd, hitem, TVGN_DROPHILITE)
// Macro 181
// #define TreeView_SelectSetFirstVisible(hwnd, hitem) TreeView_Select(hwnd, hitem, TVGN_FIRSTVISIBLE)

// Macro 179
Procedure TreeView_SelectItem(hwnd:hwnd; hitem:HTREEITEM);
Begin
 TreeView_Select(hwnd, hitem, TVGN_CARET)
End;

// Macro 180
Procedure TreeView_SelectDropTarget(hwnd:hwnd; hitem:HTREEITEM);
Begin
 TreeView_Select(hwnd, hitem, TVGN_DROPHILITE)
End;

// Macro 181
Procedure TreeView_SelectSetFirstVisible(hwnd:hwnd; hitem:HTREEITEM);
Begin
 TreeView_Select(hwnd, hitem, TVGN_FIRSTVISIBLE)
End;

// Macro 182
// #define TreeView_GetItem(hwnd, pitem) \
//     (BOOL)SNDMSG((hwnd), TVM_GETITEM, 0, (LPARAM)(TV_ITEM *)(pitem))

Function TreeView_GetItem( hwnd : hwnd;var  pitem : TV_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_GETITEM, 0, LPARAM(@pitem)))
end;


// Macro 183

//#define TreeView_SetItem(hwnd, pitem) \
//     (BOOL)SNDMSG((hwnd), TVM_SETITEM, 0, (LPARAM)(const TV_ITEM *)(pitem))

Function TreeView_SetItem( hwnd : hwnd;const  pitem : TV_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SETITEM, 0, LParam(@pitem)))
end;


// Macro 184

//#define TreeView_EditLabel(hwnd, hitem) \
//     (HWND)SNDMSG((hwnd), TVM_EDITLABEL, 0, (LPARAM)(HTREEITEM)(hitem))

Function TreeView_EditLabel( hwnd : hwnd; hitem : HTREEITEM):HWND;

Begin
 Result:=Windows.HWND(SendMessage((hwnd), TVM_EDITLABEL, 0, LParam(hitem)))
end;


// Macro 185

//#define TreeView_GetEditControl(hwnd) \
//     (HWND)SNDMSG((hwnd), TVM_GETEDITCONTROL, 0, 0)

Function TreeView_GetEditControl( hwnd : hwnd):HWND;

Begin
 Result:=Windows.HWND(SendMessage((hwnd), TVM_GETEDITCONTROL, 0, 0))
end;


// Macro 186

//#define TreeView_GetVisibleCount(hwnd) \
//     (UINT)SNDMSG((hwnd), TVM_GETVISIBLECOUNT, 0, 0)

Function TreeView_GetVisibleCount( hwnd : hwnd):UINT;

Begin
 Result:=UINT(SendMessage((hwnd), TVM_GETVISIBLECOUNT, 0, 0))
end;


// Macro 187

//#define TreeView_HitTest(hwnd, lpht) \
//     (HTREEITEM)SNDMSG((hwnd), TVM_HITTEST, 0, (LPARAM)(LPTV_HITTESTINFO)(lpht))

Function TreeView_HitTest( hwnd : hwnd; lpht : LPTV_HITTESTINFO):HTREEITEM;inline;

Begin
 Result:=HTREEITEM(SendMessage((hwnd), TVM_HITTEST, 0, lparam(lpht)))
end;


Function TreeView_HitTest( hwnd : hwnd; var lpht : TV_HITTESTINFO):HTREEITEM;inline;

Begin
 Result:=HTREEITEM(SendMessage((hwnd), TVM_HITTEST, 0, lparam(@lpht)))
end;
// Macro 188

//#define TreeView_CreateDragImage(hwnd, hitem) \
//     (HIMAGELIST)SNDMSG((hwnd), TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(hitem))

Function TreeView_CreateDragImage( hwnd : hwnd; hitem : HTREEITEM):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), TVM_CREATEDRAGIMAGE, 0, Lparam(hitem)))
end;


// Macro 189

//#define TreeView_SortChildren(hwnd, hitem, recurse) \
//     (BOOL)SNDMSG((hwnd), TVM_SORTCHILDREN, (WPARAM)(recurse), (LPARAM)(HTREEITEM)(hitem))

Function TreeView_SortChildren( hwnd : hwnd; hitem : HTREEITEM; recurse : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SORTCHILDREN, recurse, LParam(hitem)))
end;


// Macro 190

//#define TreeView_EnsureVisible(hwnd, hitem) \
//     (BOOL)SNDMSG((hwnd), TVM_ENSUREVISIBLE, 0, (LPARAM)(HTREEITEM)(hitem))

Function TreeView_EnsureVisible( hwnd : hwnd; hitem : HTREEITEM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_ENSUREVISIBLE, 0, LParam(hitem)))
end;


// Macro 191

//#define TreeView_SortChildrenCB(hwnd, psort, recurse) \
//     (BOOL)SNDMSG((hwnd), TVM_SORTCHILDRENCB, (WPARAM)(recurse), \
//     (LPARAM)(LPTV_SORTCB)(psort))

Function TreeView_SortChildrenCB( hwnd : hwnd;psort :lpTV_sortcb; recurse : WPARAM):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SORTCHILDRENCB, recurse, LPARAM(psort)))
end;


Function TreeView_SortChildrenCB( hwnd : hwnd;const psort :tagTVsortcb; recurse : WPARAM):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SORTCHILDRENCB, recurse, LPARAM(@psort)))
end;


// Macro 192

//#define TreeView_EndEditLabelNow(hwnd, fCancel) \
//     (BOOL)SNDMSG((hwnd), TVM_ENDEDITLABELNOW, (WPARAM)(fCancel), 0)

Function TreeView_EndEditLabelNow( hwnd : hwnd; fCancel : WPARAM):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_ENDEDITLABELNOW, fCancel, 0))
end;


Function TreeView_EndEditLabelNow( hwnd : hwnd; fCancel : Bool):BOOL;inline;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_ENDEDITLABELNOW, WPARAM(fCancel), 0))
end;


// Macro 193

//#define TreeView_SetToolTips(hwnd,  hwndTT) \
//     (HWND)SNDMSG((hwnd), TVM_SETTOOLTIPS, (WPARAM)(hwndTT), 0)

Function TreeView_SetToolTips( hwnd : hwnd; hwndTT : WPARAM):HWND;

Begin
 Result:=Windows.HWND(SendMessage((hwnd), TVM_SETTOOLTIPS, hwndTT, 0))
end;


// Macro 194

//#define TreeView_GetToolTips(hwnd) \
//     (HWND)SNDMSG((hwnd), TVM_GETTOOLTIPS, 0, 0)

Function TreeView_GetToolTips( hwnd : hwnd):HWND;

Begin
 Result:=Windows.HWND(SendMessage((hwnd), TVM_GETTOOLTIPS, 0, 0))
end;


// Macro 195

//#define TreeView_GetISearchString(hwndTV, lpsz) \
//         (BOOL)SNDMSG((hwndTV), TVM_GETISEARCHSTRING, 0, (LPARAM)(LPTSTR)(lpsz))

Function TreeView_GetISearchString( hwndTV : hwnd; lpsz : LPTSTR):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndTV), TVM_GETISEARCHSTRING, 0, LPARAM(lpsz)))
end;


// Macro 196

//#define TreeView_SetInsertMark(hwnd, hItem, fAfter) \
//         (BOOL)SNDMSG((hwnd), TVM_SETINSERTMARK, (WPARAM) (fAfter), (LPARAM) (hItem))

Function TreeView_SetInsertMark( hwnd : hwnd; hItem : LParam ; fAfter : WParam ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SETINSERTMARK, (fAfter), (hItem)))
end;


// Macro 197

//#define TreeView_SetUnicodeFormat(hwnd, fUnicode)  \
//     (BOOL)SNDMSG((hwnd), TVM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0)

Function TreeView_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_SETUNICODEFORMAT, fUnicode, 0))
end;


// Macro 198

//#define TreeView_GetUnicodeFormat(hwnd)  \
//     (BOOL)SNDMSG((hwnd), TVM_GETUNICODEFORMAT, 0, 0)

Function TreeView_GetUnicodeFormat( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TVM_GETUNICODEFORMAT, 0, 0))
end;


// Macro 199

//#define TreeView_SetItemHeight(hwnd,  iHeight) \
//     (int)SNDMSG((hwnd), TVM_SETITEMHEIGHT, (WPARAM)(iHeight), 0)

Function TreeView_SetItemHeight( hwnd : hwnd; iHeight : WPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), TVM_SETITEMHEIGHT, iHeight, 0))
end;


// Macro 200

//#define TreeView_GetItemHeight(hwnd) \
//     (int)SNDMSG((hwnd), TVM_GETITEMHEIGHT, 0, 0)

Function TreeView_GetItemHeight( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), TVM_GETITEMHEIGHT, 0, 0))
end;


// Macro 201

//#define TreeView_SetBkColor(hwnd, clr) \
//     (COLORREF)SNDMSG((hwnd), TVM_SETBKCOLOR, 0, (LPARAM)(clr))

Function TreeView_SetBkColor( hwnd : hwnd; clr : LPARAM):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_SETBKCOLOR, 0, clr))
end;


// Macro 202

//#define TreeView_SetTextColor(hwnd, clr) \
//     (COLORREF)SNDMSG((hwnd), TVM_SETTEXTCOLOR, 0, (LPARAM)(clr))

Function TreeView_SetTextColor( hwnd : hwnd; clr : LPARAM):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_SETTEXTCOLOR, 0, clr))
end;


// Macro 203

//#define TreeView_GetBkColor(hwnd) \
//     (COLORREF)SNDMSG((hwnd), TVM_GETBKCOLOR, 0, 0)

Function TreeView_GetBkColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_GETBKCOLOR, 0, 0))
end;


// Macro 204

//#define TreeView_GetTextColor(hwnd) \
//     (COLORREF)SNDMSG((hwnd), TVM_GETTEXTCOLOR, 0, 0)

Function TreeView_GetTextColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_GETTEXTCOLOR, 0, 0))
end;


// Macro 205

//#define TreeView_SetScrollTime(hwnd, uTime) \
//     (UINT)SNDMSG((hwnd), TVM_SETSCROLLTIME, uTime, 0)

Function TreeView_SetScrollTime( hwnd : hwnd; uTime : wparam ):UINT;

Begin
 Result:=UINT(SendMessage((hwnd), TVM_SETSCROLLTIME, uTime, 0))
end;


// Macro 206

//#define TreeView_GetScrollTime(hwnd) \
//     (UINT)SNDMSG((hwnd), TVM_GETSCROLLTIME, 0, 0)

Function TreeView_GetScrollTime( hwnd : hwnd):UINT;

Begin
 Result:=UINT(SendMessage((hwnd), TVM_GETSCROLLTIME, 0, 0))
end;


// Macro 207

//#define TreeView_SetInsertMarkColor(hwnd, clr) \
//     (COLORREF)SNDMSG((hwnd), TVM_SETINSERTMARKCOLOR, 0, (LPARAM)(clr))

Function TreeView_SetInsertMarkColor( hwnd : hwnd; clr : LPARAM):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_SETINSERTMARKCOLOR, 0, clr))
end;


// Macro 208

//#define TreeView_GetInsertMarkColor(hwnd) \
//     (COLORREF)SNDMSG((hwnd), TVM_GETINSERTMARKCOLOR, 0, 0)

Function TreeView_GetInsertMarkColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_GETINSERTMARKCOLOR, 0, 0))
end;


// Macro 209

//#define TreeView_SetItemState(hwndTV, hti, data, _mask) \
// { TVITEM _ms_TVi;\
//   _ms_TVi.mask = TVIF_STATE; \
//   _ms_TVi.hItem = hti; \
//   _ms_TVi.stateMask = _mask;\
//   _ms_TVi.state = data;\
//   SNDMSG((hwndTV), TVM_SETITEM, 0, (LPARAM)(TV_ITEM *)&_ms_TVi);\
// }

Procedure TreeView_SetItemState(hwndTV:HWND;hti:HTreeItem;data:UINT;_mask:UINT);

var _ms_TVi : TVITEM;

Begin
 _ms_TVi.mask:=TVIF_STATE;
 _ms_TVi.hItem := hti;
 _ms_TVi.stateMask := _mask;
 _ms_TVi.state := data;
 SendMessage(hwndTV,TVM_SETITEM, 0, LPARAM(@_ms_TVi));
end;


// Macro 210

//#define TreeView_SetCheckState(hwndTV, hti, fCheck) \
//   TreeView_SetItemState(hwndTV, hti, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), TVIS_STATEIMAGEMASK)

Procedure TreeView_SetCheckState( hwndTV : hwnd; hti : HTreeItem ; fCheck : bool );

var j : cint;

Begin
 IF not fCheck Then  // (or inc(longint(fCheck)) if you like it dirty)
  j:=1
 Else
  j:=2;

 TreeView_SetItemState(hwndTV, hti, INDEXTOSTATEIMAGEMASK(j), TVIS_STATEIMAGEMASK)
end;


{$ifdef IE5plus}

// Macro 211

//#define TreeView_GetItemState(hwndTV, hti, mask) \
//    (UINT)SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), (LPARAM)(mask))

Function TreeView_GetItemState( hwndTV : hwnd; hti : HTreeItem; statemask : UINT):UINT;

Begin
 Result:=UINT(SendMessage((hwndTV), TVM_GETITEMSTATE, WPARAM(hti), LPARAM(statemask)))
end;

// Macro 212
// #define TreeView_GetCheckState(hwndTV, hti) \
//    ((((UINT)(SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), TVIS_STATEIMAGEMASK))) >> 12) -1)

Function TreeView_GetCheckState( hwndTV : hwnd; hti : HTreeItem):UINT;

Begin
 Result:=((UINT(SendMessage((hwndTV), TVM_GETITEMSTATE, wparam(hti), TVIS_STATEIMAGEMASK) shr 12) -1));
end;


// Macro 213

// #define TreeView_SetLineColor(hwnd, clr) \
//     (COLORREF)SNDMSG((hwnd), TVM_SETLINECOLOR, 0, (LPARAM)(clr))

Function TreeView_SetLineColor( hwnd : hwnd; clr : LPARAM):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_SETLINECOLOR, 0, clr))
end;


// Macro 214

// #define TreeView_GetLineColor(hwnd) \
//     (COLORREF)SNDMSG((hwnd), TVM_GETLINECOLOR, 0, 0)

Function TreeView_GetLineColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), TVM_GETLINECOLOR, 0, 0))
end;


// Macro 215
// #define TreeView_MapAccIDToHTREEITEM(hwnd, id) \
//     (HTREEITEM)SNDMSG((hwnd), TVM_MAPACCIDTOHTREEITEM, id, 0)

Function TreeView_MapAccIDToHTREEITEM( hwnd : hwnd; id :uint):HTREEITEM;

Begin
 Result:=HTREEITEM(SendMessage((hwnd), TVM_MAPACCIDTOHTREEITEM, id, 0))
end;


// Macro 216
// #define TreeView_MapHTREEITEMToAccID(hwnd, htreeitem) \
//     (UINT)SNDMSG((hwnd), TVM_MAPHTREEITEMTOACCID, (WPARAM)htreeitem, 0)

Function TreeView_MapHTREEITEMToAccID( hwnd : hwnd; htreeitem : WPARAM):UINT;

Begin
 Result:=UINT(SendMessage((hwnd), TVM_MAPHTREEITEMTOACCID, htreeitem, 0))
end;
{$endif}

{$ifdef win32vista}
function  TreeView_GetSelectedCount(hwnd:hwnd):DWORD;
Begin
 Result:=DWORD(SendMessage((hwnd),TVM_GETSELECTEDCOUNT, 0, 0));
end;

function  TreeView_ShowInfoTip(hwnd:HWND; hitem:HTREEITEM):DWORD;
Begin
 Result:=DWORD(SendMessage((hwnd),TVM_SHOWINFOTIP, 0, LPARAM(hitem)));
end;

function  TreeView_GetItemPartRect(hwnd:HWND; hitem:HTREEITEM; prc:prect; partid:TVITEMPART):bool;
var info : TVGETITEMPARTRECTINFO;
Begin
  info.hti := (hitem); 
  info.prc := (prc);
  info.partID := (partid);
  Result:=BOOL(SendMessage((hwnd), TVM_GETITEMPARTRECT, 0, LPARAM(@info))); 
end;
{$endif}

// Macro 217

//#define TabCtrl_GetImageList(hwnd) \
//     (HIMAGELIST)SNDMSG((hwnd), TCM_GETIMAGELIST, 0, DWord(0))

Function TabCtrl_GetImageList( hwnd : hwnd):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), TCM_GETIMAGELIST, 0, LPARAM(0)))
end;


// Macro 218

//#define TabCtrl_SetImageList(hwnd, himl) \
//     (HIMAGELIST)SNDMSG((hwnd), TCM_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)(himl))

Function TabCtrl_SetImageList( hwnd : hwnd; himl : HIMAGELIST):HIMAGELIST;

Begin
 Result:=HIMAGELIST(SendMessage((hwnd), TCM_SETIMAGELIST, 0, himl))
end;


// Macro 219

//#define TabCtrl_GetItemCount(hwnd) \
//     (int)SNDMSG((hwnd), TCM_GETITEMCOUNT, 0, DWord(0))

Function TabCtrl_GetItemCount( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_GETITEMCOUNT, 0, LPARAM(0)))
end;


// Macro 220

//#define TabCtrl_GetItem(hwnd, iItem, pitem) \
//     (BOOL)SNDMSG((hwnd), TCM_GETITEM, (WPARAM)(int)(iItem), (LPARAM)(TC_ITEM *)(pitem))

Function TabCtrl_GetItem( hwnd : hwnd; iItem : cint;var  pitem : TC_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_GETITEM, iItem, LPARAM(@pitem)))
end;


// Macro 221

//#define TabCtrl_SetItem(hwnd, iItem, pitem) \
//     (BOOL)SNDMSG((hwnd), TCM_SETITEM, (WPARAM)(int)(iItem), (LPARAM)(TC_ITEM *)(pitem))

Function TabCtrl_SetItem( hwnd : hwnd; iItem : cint;var  pitem : TC_ITEM ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_SETITEM, iItem, LPARAM(@pitem)))
end;


// Macro 222

//#define TabCtrl_InsertItem(hwnd, iItem, pitem)   \
//     (int)SNDMSG((hwnd), TCM_INSERTITEM, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEM *)(pitem))

Function TabCtrl_InsertItem( hwnd : hwnd; iItem : cint;const  pitem : TC_ITEM ):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_INSERTITEM, iItem, LPARAM(@pitem)));
end;


// Macro 223

//#define TabCtrl_DeleteItem(hwnd, i) \
//     (BOOL)SNDMSG((hwnd), TCM_DELETEITEM, (WPARAM)(int)(i), DWord(0))

Function TabCtrl_DeleteItem( hwnd : hwnd; i : cint):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_DELETEITEM, (i), LPARAM(0)))
end;


// Macro 224

//#define TabCtrl_DeleteAllItems(hwnd) \
//     (BOOL)SNDMSG((hwnd), TCM_DELETEALLITEMS, 0, DWord(0))

Function TabCtrl_DeleteAllItems( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_DELETEALLITEMS, 0, LPARAM(0)))
end;


// Macro 225

//#define TabCtrl_GetItemRect(hwnd, i, prc) \
//     (BOOL)SNDMSG((hwnd), TCM_GETITEMRECT, (WPARAM)(int)(i), (LPARAM)(RECT *)(prc))

Function TabCtrl_GetItemRect( hwnd : hwnd; i : cint;var  prc : RECT ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_GETITEMRECT, (i), LPARAM(@prc)))
end;


// Macro 226

//#define TabCtrl_GetCurSel(hwnd) \
//     (int)SNDMSG((hwnd), TCM_GETCURSEL, 0, 0)

Function TabCtrl_GetCurSel( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_GETCURSEL, 0, 0))
end;


// Macro 227

//#define TabCtrl_SetCurSel(hwnd, i) \
//     (int)SNDMSG((hwnd), TCM_SETCURSEL, (WPARAM)(i), 0)

Function TabCtrl_SetCurSel( hwnd : hwnd; i : WPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_SETCURSEL, (i), 0))
end;


// Macro 228

//#define TabCtrl_HitTest(hwndTC, pinfo) \
//     (int)SNDMSG((hwndTC), TCM_HITTEST, 0, (LPARAM)(TC_HITTESTINFO *)(pinfo))

Function TabCtrl_HitTest( hwndTC : hwnd;var  pinfo : TC_HITTESTINFO ):cint;inline;

Begin
 Result:=cint(SendMessage((hwndTC), TCM_HITTEST, 0, LPARAM(@pinfo)))
end;


Function TabCtrl_HitTest( hwndTC : hwnd;pinfo : LPTCHITTESTINFO ):cint;inline;

Begin
 Result:=cint(SendMessage((hwndTC), TCM_HITTEST, 0, LPARAM(@pinfo)))
end;


// Macro 229

//#define TabCtrl_SetItemExtra(hwndTC, cb) \
//     (BOOL)SNDMSG((hwndTC), TCM_SETITEMEXTRA, (WPARAM)(cb), DWord(0))

Function TabCtrl_SetItemExtra( hwndTC : hwnd; cb : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwndTC), TCM_SETITEMEXTRA, cb, LPARAM(0)))
end;


// Macro 230

//#define TabCtrl_AdjustRect(hwnd, bLarger, prc) \
//     (int)SNDMSG(hwnd, TCM_ADJUSTRECT, (WPARAM)(BOOL)(bLarger), (LPARAM)(RECT *)prc)

Function TabCtrl_AdjustRect( hwnd : hwnd; bLarger : BOOL;var  prc : RECT ):cint;

Begin
 Result:=cint(SendMessage(hwnd, TCM_ADJUSTRECT, Wparam(bLarger), Lparam(@prc)));
end;


// Macro 231

//#define TabCtrl_SetItemSize(hwnd, x, y) \
//     (DWORD)SNDMSG((hwnd), TCM_SETITEMSIZE, 0, MAKELPARAM(x,y))

Function TabCtrl_SetItemSize( hwnd : hwnd; x : wparam ; y : lparam ):DWORD;

Begin
 Result:=DWORD(SendMessage((hwnd), TCM_SETITEMSIZE, 0, MAKELPARAM(x,y)))
end;


// Macro 232

//#define TabCtrl_RemoveImage(hwnd, i) \
//         (void)SNDMSG((hwnd), TCM_REMOVEIMAGE, i, DWord(0))

Procedure TabCtrl_RemoveImage( hwnd : hwnd; i : WPARAM);

Begin
SendMessage((hwnd), TCM_REMOVEIMAGE, i, LPARAM(0))
end;


// Macro 233

//#define TabCtrl_SetPadding(hwnd,  cx, cy) \
//         (void)SNDMSG((hwnd), TCM_SETPADDING, 0, MAKELPARAM(cx, cy))

Procedure TabCtrl_SetPadding( hwnd : hwnd; cx : WPARAM ; cy : LPARAM );

Begin
SendMessage((hwnd), TCM_SETPADDING, 0, MAKELPARAM(cx, cy))
end;


// Macro 234

//#define TabCtrl_GetRowCount(hwnd) \
//         (int)SNDMSG((hwnd), TCM_GETROWCOUNT, 0, DWord(0))

Function TabCtrl_GetRowCount( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_GETROWCOUNT, 0, LPARAM(0)))
end;


// Macro 235

//#define TabCtrl_GetToolTips(hwnd) \
//         (HWND)SNDMSG((hwnd), TCM_GETTOOLTIPS, 0, DWord(0))

Function TabCtrl_GetToolTips( hwnd : hwnd):HWND;

Begin
 Result:=Windows.HWND(SendMessage((hwnd), TCM_GETTOOLTIPS, 0, LPARAM(0)))
end;


// Macro 236

//#define TabCtrl_SetToolTips(hwnd, hwndTT) \
//         (void)SNDMSG((hwnd), TCM_SETTOOLTIPS, (WPARAM)(hwndTT), DWord(0))

Procedure TabCtrl_SetToolTips( hwnd : hwnd; hwndTT : WPARAM);

Begin
SendMessage((hwnd), TCM_SETTOOLTIPS, hwndTT, LPARAM(0))
end;


// Macro 237

//#define TabCtrl_GetCurFocus(hwnd) \
//     (int)SNDMSG((hwnd), TCM_GETCURFOCUS, 0, 0)

Function TabCtrl_GetCurFocus( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_GETCURFOCUS, 0, 0))
end;


// Macro 238

//#define TabCtrl_SetCurFocus(hwnd, i) \
//     SNDMSG((hwnd),TCM_SETCURFOCUS, i, 0)

Procedure TabCtrl_SetCurFocus( hwnd : hwnd; i : LPARAM );

Begin
SendMessage((hwnd),TCM_SETCURFOCUS, i, 0)
end;


// Macro 239

//#define TabCtrl_SetMinTabWidth(hwnd, x) \
//         (int)SNDMSG((hwnd), TCM_SETMINTABWIDTH, 0, x)

Function TabCtrl_SetMinTabWidth( hwnd : hwnd; x : WPARAM ):cint;

Begin
 Result:=cint(SendMessage((hwnd), TCM_SETMINTABWIDTH, 0, x))
end;


// Macro 240

//#define TabCtrl_DeselectAll(hwnd, fExcludeFocus)\
//         (void)SNDMSG((hwnd), TCM_DESELECTALL, fExcludeFocus, 0)

Procedure TabCtrl_DeselectAll( hwnd : hwnd; fExcludeFocus : WPARAM );

Begin
SendMessage((hwnd), TCM_DESELECTALL, fExcludeFocus, 0)
end;


// Macro 241

//#define TabCtrl_HighlightItem(hwnd, i, fHighlight) \
//     (BOOL)SNDMSG((hwnd), TCM_HIGHLIGHTITEM, (WPARAM)(i), (LPARAM)MAKELONG (fHighlight, 0))

Function TabCtrl_HighlightItem( hwnd : hwnd; i : WPARAM; fHighlight :bool ):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_HIGHLIGHTITEM, (i), LPARAM(MAKELONG (cint(fHighlight), 0))))
end;


// Macro 242

//#define TabCtrl_SetExtendedStyle(hwnd, dw)\
//         (DWORD)SNDMSG((hwnd), TCM_SETEXTENDEDSTYLE, 0, dw)

Function TabCtrl_SetExtendedStyle( hwnd : hwnd; dw : LPARAM):DWORD;

Begin
 Result:=DWORD(SendMessage((hwnd), TCM_SETEXTENDEDSTYLE, 0,lparam(dw)))
end;


// Macro 243

//#define TabCtrl_GetExtendedStyle(hwnd)\
//         (DWORD)SNDMSG((hwnd), TCM_GETEXTENDEDSTYLE, 0, 0)

Function TabCtrl_GetExtendedStyle( hwnd : hwnd):DWORD;

Begin
 Result:=DWORD(SendMessage((hwnd), TCM_GETEXTENDEDSTYLE, 0, 0))
end;


// Macro 244

//#define TabCtrl_SetUnicodeFormat(hwnd, fUnicode)  \
//     (BOOL)SNDMSG((hwnd), TCM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0)

Function TabCtrl_SetUnicodeFormat( hwnd : hwnd; fUnicode : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_SETUNICODEFORMAT, fUnicode, 0))
end;


// Macro 245

//#define TabCtrl_GetUnicodeFormat(hwnd)  \
//     (BOOL)SNDMSG((hwnd), TCM_GETUNICODEFORMAT, 0, 0)

Function TabCtrl_GetUnicodeFormat( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), TCM_GETUNICODEFORMAT, 0, 0));
end;


// Macro 246

//#define Animate_Create(hwndP, id, dwStyle, hInstance)   \
//             CreateWindow(ANIMATE_CLASS, NULL,           \
//                 dwStyle, 0, 0, 0, 0, hwndP, (HMENU)(id), hInstance, NULL)

Function Animate_Create(hwndP :HWND;id:HMENU;dwStyle:dword;hInstance:HINST):HWND;

Begin
     result:=CreateWindow(ANIMATE_CLASS, NIL,
                 dwStyle, 0, 0, 0, 0, hwndP, id, hInstance, NIL);
end;


// Macro 247
//#define Animate_Open(hwnd, szName)  (BOOL)SNDMSG(hwnd, ACM_OPEN, 0, (LPARAM)(LPTSTR)(szName))

Function Animate_Open(hwndP :HWND;szName:LPTSTR):BOOL;

Begin
  Result:=BOOL(SendMessage(hwndp, ACM_OPEN, 0, LPARAM(szName)));
end;


// Macro 248
//#define Animate_OpenEx(hwnd, hInst, szName) (BOOL)SNDMSG(hwnd, ACM_OPEN, (WPARAM)(hInst), (LPARAM)(LPTSTR)(szName))

Function Animate_OpenEx(hwndP :HWND;HInst:HInst; szName:LPTSTR):BOOL;

Begin
  Result:=BOOL(SendMessage(hwndp, ACM_OPEN, hinst, LPARAM(szName)));
end;


// Macro 249
//#define Animate_Play(hwnd, from, to, rep)   (BOOL)SNDMSG(hwnd, ACM_PLAY, (WPARAM)(rep), (LPARAM)MAKELONG(from, to))

Function Animate_Play(hwndP :HWND;from,_to,rep:uint):BOOL;

Begin
  Result:=BOOL(SendMessage(hwndP, ACM_PLAY, wparam(rep), LPARAM(MAKELONG(From,_to))));
end;


// Macro 250
//#define Animate_Stop(hwnd)                  (BOOL)SNDMSG(hwnd, ACM_STOP, 0, 0)

Function Animate_stop(hwndP :HWND):BOOL;

Begin
  Result:=BOOL(SendMessage(hwndP, ACM_STOP, 0, 0));
end;


// Macro 251
//#define Animate_Close(hwnd)                 Animate_Open(hwnd, NULL)

Function Animate_close(hwndP :HWND):BOOL;

Begin
  Result:=BOOL(Animate_Open(hwndP,NIL));
end;


// Macro 252
//#define Animate_Seek(hwnd, frame)           Animate_Play(hwnd, frame, frame, 1)

Function Animate_Seek(hwndP :HWND;frame:uint):BOOL;

Begin
  Result:=BOOL(Animate_Play(hwndP,frame,frame,1));
end;

// Macro 253
//#define MonthCal_GetCurSel(hmc, pst)    (BOOL)SNDMSG(hmc, MCM_GETCURSEL, 0, (LPARAM)(pst))

function MonthCal_GetCurSel(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETCURSEL , 0, LPARAM(lpsystime)));
end;

// Macro 254
//#define MonthCal_SetCurSel(hmc, pst)    (BOOL)SNDMSG(hmc, MCM_SETCURSEL, 0, (LPARAM)(pst))

function MonthCal_SetCurSel(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool; inline;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_SETCURSEL, 0, LPARAM(lpsystime)));
end;

function MonthCal_SetCurSel(hwndMC:HWND; var lpSysTime :SYSTEMTIME):Bool; inline;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_SETCURSEL, 0, LPARAM(@lpsystime)));
end;

// Macro 255
//#define MonthCal_GetMaxSelCount(hmc)    (DWORD)SNDMSG(hmc, MCM_GETMAXSELCOUNT, 0, DWord(0))

function MonthCal_GetMaxSelCount(hwndMC:HWND):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETMAXSELCOUNT, 0, LPARAM(0)));
end;

// Macro 256
//#define MonthCal_SetMaxSelCount(hmc, n) (BOOL)SNDMSG(hmc, MCM_SETMAXSELCOUNT, (WPARAM)(n), DWord(0))

function MonthCal_SetMaxSelCount(hwndMC:HWND;n:uint):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_SETMAXSELCOUNT,WPARAM(N), LPARAM(0)));
end;

// Macro 257
//#define MonthCal_GetSelRange(hmc, rgst) SNDMSG(hmc, MCM_GETSELRANGE, 0, (LPARAM)(rgst))

function MonthCal_GetSelRange(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETSELRANGE,WPARAM(0), LPARAM(lpsystime)));
end;

// Macro 258
//#define MonthCal_SetSelRange(hmc, rgst) SNDMSG(hmc, MCM_SETSELRANGE, 0, (LPARAM)(rgst))

function MonthCal_SetSelRange(hwndMC:HWND; lpSysTime :LPSYSTEMTIME):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_SETSELRANGE,WPARAM(0), LPARAM(lpsystime)));
end;


// Macro 259
//#define MonthCal_GetMonthRange(hmc, gmr, rgst)  (DWORD)SNDMSG(hmc, MCM_GETMONTHRANGE, (WPARAM)(gmr), (LPARAM)(rgst))

function MonthCal_GetMonthRange(hwndMC:HWND; gmr: DWORD;lpSysTime :LPSYSTEMTIME):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETMONTHRANGE,WPARAM(gmr), LPARAM(lpsystime)));
end;

// Macro 260
//#define MonthCal_SetDayState(hmc, cbds, rgds)   SNDMSG(hmc, MCM_SETDAYSTATE, (WPARAM)(cbds), (LPARAM)(rgds))

function MonthCal_SetDayState(hwndMC:HWND; gmr: Longint;lpDay :LPMONTHDAYSTATE):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_SETDAYSTATE,WPARAM(gmr), LPARAM(lpDay)));
end;

// Macro 261
// #define MonthCal_GetMinReqRect(hmc, prc)        SNDMSG(hmc, MCM_GETMINREQRECT, 0, (LPARAM)(prc))

function MonthCal_GetMinReqRect(hwndMC:HWND; lpr :LPRect):Bool;inline;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETMINREQRECT,0, LPARAM(lpr)));
end;

function MonthCal_GetMinReqRect(hwndMC:HWND;var lpr :TRect):Bool;inline;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETMINREQRECT,0, LPARAM(@lpr)));
end;

// Macro 262
// #define MonthCal_SetColor(hmc, iColor, clr) SNDMSG(hmc, MCM_SETCOLOR, iColor, clr)

function MonthCal_SetColor(hwndMC:HWND; ic:longint;clr:COLORREF):DWORD;inline;

begin
     Result:=DWORD(SendMessage(hwndmc, MCM_SETCOLOR,ic, LPARAM(clr)));
end;

// Macro 263
// #define MonthCal_GetColor(hmc, iColor) SNDMSG(hmc, MCM_GETCOLOR, iColor, 0)

function MonthCal_GetColor(hwndMC:HWND; ic:longint):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETCOLOR,ic, 0));
end;

// Macro 264
// #define MonthCal_SetToday(hmc, pst)             SNDMSG(hmc, MCM_SETTODAY, 0, (LPARAM)(pst))

function MonthCal_SetToday(hwndMC:HWND; lps:LPSYSTEMTIME):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_SETTODAY,0, LPARAM(lps)));
end;

// Macro 265
// #define MonthCal_GetToday(hmc, pst)             (BOOL)SNDMSG(hmc, MCM_GETTODAY, 0, (LPARAM)(pst))

function MonthCal_GetToday(hwndMC:HWND; lps:LPSYSTEMTIME):Bool;

begin
     Result:=BOOL(SendMessage(hwndmc, MCM_GETTODAY,0, LPARAM(lps)));
end;


// Macro 266
// #define MonthCal_HitTest(hmc, pinfo) \
//         SNDMSG(hmc, MCM_HITTEST, 0, (LPARAM)(PMCHITTESTINFO)(pinfo))

FUNCTION MonthCal_HitTest( hmc :HWND ; pinfo : PMCHITTESTINFO):DWORD;

Begin
   Result:=DWORD(SendMessage(hmc, MCM_HITTEST, 0, LPARAM(pinfo)));
end;


// Macro 267
// #define MonthCal_SetFirstDayOfWeek(hmc, iDay) \
//         SNDMSG(hmc, MCM_SETFIRSTDAYOFWEEK, 0, iDay)

function MonthCal_SetFirstDayOfWeek( hmc : HWND ; iDay :LONGINT ):DWORD;

Begin
   Result:=SendMessage(hmc, MCM_SETFIRSTDAYOFWEEK, 0, iDay);
end;


// Macro 268
// #define MonthCal_GetFirstDayOfWeek(hmc) \
//         (DWORD)SNDMSG(hmc, MCM_GETFIRSTDAYOFWEEK, 0, 0)

Function MonthCal_GetFirstDayOfWeek( hmc : HWND ):DWORD;

Begin
 Result:=DWORD(SendMessage(hmc, MCM_GETFIRSTDAYOFWEEK, 0, 0))
end;


// Macro 269
// #define MonthCal_GetRange(hmc, rgst) \
//         (DWORD)SNDMSG(hmc, MCM_GETRANGE, 0, (LPARAM)(rgst))

Function MonthCal_GetRange( hmc : HWND ; rgst : LPSYSTEMTIME):DWORD;

Begin
  Result:=DWORD(SendMessage(hmc, MCM_GETRANGE, 0, lparam(rgst)));
end;


// Macro 270
// #define MonthCal_SetRange(hmc, gd, rgst) \
//         (BOOL)SNDMSG(hmc, MCM_SETRANGE, (WPARAM)(gd), (LPARAM)(rgst))

Function MonthCal_SetRange( hmc : HWND ; gd : DWORD; rgst : LPSYSTEMTIME):BOOL;

Begin
  Result:=BOOL(SendMessage(hmc, MCM_SETRANGE, gd,LPARAM(rgst)))
end;


// Macro 271
// #define MonthCal_GetMonthDelta(hmc) \
//         (int)SNDMSG(hmc, MCM_GETMONTHDELTA, 0, 0)

Function MonthCal_GetMonthDelta( hmc :hwnd ):cint;

Begin
 Result:=cint(SendMessage(hmc, MCM_GETMONTHDELTA, 0, 0))
end;


// Macro 272
// #define MonthCal_SetMonthDelta(hmc, n) \
//         (int)SNDMSG(hmc, MCM_SETMONTHDELTA, n, 0)

Function MonthCal_SetMonthDelta( hmc :hwnd ; n :cint ):cint;

Begin
 Result:=cint(SendMessage(hmc, MCM_SETMONTHDELTA, n, 0))
end;


// Macro 273

// #define MonthCal_GetMaxTodayWidth(hmc) \
//         (DWORD)SNDMSG(hmc, MCM_GETMAXTODAYWIDTH, 0, 0)

Function MonthCal_GetMaxTodayWidth( hmc :hwnd ):DWORD;

Begin
 Result:=DWORD(SendMessage(hmc, MCM_GETMAXTODAYWIDTH, 0, 0))
end;


// Macro 274
// #define MonthCal_SetUnicodeFormat(hwnd, fUnicode)  \
//     (BOOL)SNDMSG((hwnd), MCM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0)

Function MonthCal_SetUnicodeFormat( hwnd : hwnd; fUnicode : bool):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), MCM_SETUNICODEFORMAT, wparam(fUnicode), 0))
end;


// Macro 275

// #define MonthCal_GetUnicodeFormat(hwnd)  \
//     (BOOL)SNDMSG((hwnd), MCM_GETUNICODEFORMAT, 0, 0)

Function MonthCal_GetUnicodeFormat( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), MCM_GETUNICODEFORMAT, 0, 0))
end;


// Macro 276
// #define DateTime_GetSystemtime(hdp, pst)    (DWORD)SNDMSG(hdp, DTM_GETSYSTEMTIME, 0, (LPARAM)(pst))

// Macro 277

//#define DateTime_SetSystemtime(hdp, gd, pst)    (BOOL)SNDMSG(hdp, DTM_SETSYSTEMTIME, (WPARAM)(gd), (LPARAM)(pst))
function DateTime_SetSystemTime(hdp: HWND; gd: DWORD; const pst: TSystemTime): BOOL;inline;
begin
  result:=BOOL(SendMessage(hdp, DTM_SETSYSTEMTIME, WPARAM(gd), LPARAM(@pst)));
end;

// Macro 278

//#define DateTime_GetRange(hdp, rgst)  (DWORD)SNDMSG(hdp, DTM_GETRANGE, 0, (LPARAM)(rgst))

// Macro 279

//#define DateTime_SetRange(hdp, gd, rgst)  (BOOL)SNDMSG(hdp, DTM_SETRANGE, (WPARAM)(gd), (LPARAM)(rgst))
function DateTime_SetRange(hdp: HWND; gdtr: DWORD; rgst: PSystemTime): BOOL;inline;
begin
  result:=BOOL(SendMessage(hdp, DTM_SETRANGE, WPARAM(gdtr), LPARAM(rgst)));
end;

// Macro 280

//#define DateTime_SetFormat(hdp, sz)  (BOOL)SNDMSG(hdp, DTM_SETFORMAT, 0, (LPARAM)(sz))

// Macro 281

//#define DateTime_SetMonthCalColor(hdp, iColor, clr) SNDMSG(hdp, DTM_SETMCCOLOR, iColor, clr)
function DateTime_SetMonthCalColor(hdp: HWND; iColor: DWORD; clr: TColorRef): TColorRef;inline;
begin
  result:=TColorRef(SendMessage(hdp, DTM_SETMCCOLOR, iColor, clr));
end;

// Macro 282

//#define DateTime_GetMonthCalColor(hdp, iColor) SNDMSG(hdp, DTM_GETMCCOLOR, iColor, 0)

// Macro 283

//#define DateTime_GetMonthCal(hdp) (HWND)SNDMSG(hdp, DTM_GETMONTHCAL, 0, 0)
function DateTime_GetMonthCal(hdp: HWND): HWND;inline;
begin
  result:=HWND(SendMessage(hdp, DTM_GETMONTHCAL, 0, 0));
end;

// Macro 284

//#define DateTime_SetMonthCalFont(hdp, hfont, fRedraw) SNDMSG(hdp, DTM_SETMCFONT, (WPARAM)(hfont), (LPARAM)(fRedraw))

// Macro 285

//#define DateTime_GetMonthCalFont(hdp) SNDMSG(hdp, DTM_GETMCFONT, 0, 0)

// Macro 286

//#define MAKEIPRANGE(low, high)    ((LPARAM)(WORD)(((BYTE)(high) << 8) + (BYTE)(low)))

// Macro 287

//#define MAKEIPADDRESS(b1,b2,b3,b4)  ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

// Macro 288

//#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)

// Macro 289

//#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)

// Macro 290

//#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)

// Macro 291

//#define FOURTH_IPADDRESS(x) (x & 0xff)

// Macro 292

//#define Pager_SetChild(hwnd, hwndChild) \
//         (void)SNDMSG((hwnd), PGM_SETCHILD, 0, (LPARAM)(hwndChild))

Procedure Pager_SetChild( hwnd : hwnd; hwndChild : LPARAM);

Begin
SendMessage((hwnd), PGM_SETCHILD, 0, hwndChild)
end;


// Macro 293

//#define Pager_RecalcSize(hwnd) \
//         (void)SNDMSG((hwnd), PGM_RECALCSIZE, 0, 0)

Procedure Pager_RecalcSize( hwnd : hwnd);

Begin
SendMessage((hwnd), PGM_RECALCSIZE, 0, 0)
end;


// Macro 294

//#define Pager_ForwardMouse(hwnd, bForward) \
//         (void)SNDMSG((hwnd), PGM_FORWARDMOUSE, (WPARAM)(bForward), 0)

Procedure Pager_ForwardMouse( hwnd : hwnd; bForward : WPARAM);

Begin
SendMessage((hwnd), PGM_FORWARDMOUSE, bForward, 0)
end;


// Macro 295

//#define Pager_SetBkColor(hwnd, clr) \
//         (COLORREF)SNDMSG((hwnd), PGM_SETBKCOLOR, 0, (LPARAM)(clr))

Function Pager_SetBkColor( hwnd : hwnd; clr : LPARAM):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), PGM_SETBKCOLOR, 0, clr))
end;


// Macro 296

//#define Pager_GetBkColor(hwnd) \
//         (COLORREF)SNDMSG((hwnd), PGM_GETBKCOLOR, 0, 0)

Function Pager_GetBkColor( hwnd : hwnd):COLORREF;

Begin
 Result:=COLORREF(SendMessage((hwnd), PGM_GETBKCOLOR, 0, 0))
end;


// Macro 297

//#define Pager_SetBorder(hwnd, iBorder) \
//         (int)SNDMSG((hwnd), PGM_SETBORDER, 0, (LPARAM)(iBorder))

Function Pager_SetBorder( hwnd : hwnd; iBorder : LPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), PGM_SETBORDER, 0, iBorder))
end;


// Macro 298
// #define Pager_GetBorder(hwnd) \
//         (int)SNDMSG((hwnd), PGM_GETBORDER, 0, 0)

Function Pager_GetBorder( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), PGM_GETBORDER, 0, 0))
end;


// Macro 299

//#define Pager_SetPos(hwnd, iPos) \
//         (int)SNDMSG((hwnd), PGM_SETPOS, 0, (LPARAM)(iPos))

Function Pager_SetPos( hwnd : hwnd; iPos : LPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), PGM_SETPOS, 0, iPos))
end;


// Macro 300

//#define Pager_GetPos(hwnd) \
//         (int)SNDMSG((hwnd), PGM_GETPOS, 0, 0)

Function Pager_GetPos( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), PGM_GETPOS, 0, 0))
end;


// Macro 301

//#define Pager_SetButtonSize(hwnd, iSize) \
//         (int)SNDMSG((hwnd), PGM_SETBUTTONSIZE, 0, (LPARAM)(iSize))

Function Pager_SetButtonSize( hwnd : hwnd; iSize : LPARAM):cint;

Begin
 Result:=cint(SendMessage((hwnd), PGM_SETBUTTONSIZE, 0, iSize))
end;


// Macro 302

//#define Pager_GetButtonSize(hwnd) \
//         (int)SNDMSG((hwnd), PGM_GETBUTTONSIZE, 0,0)

Function Pager_GetButtonSize( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), PGM_GETBUTTONSIZE, 0,0))
end;


// Macro 303

//#define Pager_GetButtonState(hwnd, iButton) \
//         (DWORD)SNDMSG((hwnd), PGM_GETBUTTONSTATE, 0, (LPARAM)(iButton))

Function Pager_GetButtonState( hwnd : hwnd; iButton : LPARAM):DWORD;

Begin
 Result:=DWORD(SendMessage((hwnd), PGM_GETBUTTONSTATE, 0, iButton))
end;


// Macro 304
// #define Pager_GetDropTarget(hwnd, ppdt) \
//         (void)SNDMSG((hwnd), PGM_GETDROPTARGET, 0, (LPARAM)(ppdt))

Procedure Pager_GetDropTarget( hwnd : hwnd; ppdt : LPARAM);

Begin
SendMessage((hwnd), PGM_GETDROPTARGET, 0, ppdt)
end;


// Macro 305
// #define Button_GetIdealSize(hwnd, psize)\
//     (BOOL)SNDMSG((hwnd), BCM_GETIDEALSIZE, 0, (LPARAM)(psize))

Function Button_GetIdealSize( hwnd : hwnd; psize : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), BCM_GETIDEALSIZE, 0, psize))
end;


// Macro 306
// #define Button_SetImageList(hwnd, pbuttonImagelist)\
//     (BOOL)SNDMSG((hwnd), BCM_SETIMAGELIST, 0, (LPARAM)(pbuttonImagelist))

Function Button_SetImageList( hwnd : hwnd; pbuttonImagelist : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), BCM_SETIMAGELIST, 0, pbuttonImagelist))
end;

// Macro 307
// #define Button_GetImageList(hwnd, pbuttonImagelist)\
//     (BOOL)SNDMSG((hwnd), BCM_GETIMAGELIST, 0, (LPARAM)(pbuttonImagelist))

Function Button_GetImageList( hwnd : hwnd; pbuttonImagelist : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), BCM_GETIMAGELIST, 0, pbuttonImagelist))
end;


// Macro 308
// #define Button_SetTextMargin(hwnd, pmargin)\
//     (BOOL)SNDMSG((hwnd), BCM_SETTEXTMARGIN, 0, (LPARAM)(pmargin))

Function Button_SetTextMargin( hwnd : hwnd; pmargin : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), BCM_SETTEXTMARGIN, 0, pmargin))
end;


// Macro 309
// #define Button_GetTextMargin(hwnd, pmargin)\
//     (BOOL)SNDMSG((hwnd), BCM_GETTEXTMARGIN, 0, (LPARAM)(pmargin))

Function Button_GetTextMargin( hwnd : hwnd; pmargin : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), BCM_GETTEXTMARGIN, 0, pmargin))
end;


// Macro 310
// #define Edit_SetCueBannerText(hwnd, lpcwText) \
//         (BOOL)SNDMSG((hwnd), EM_SETCUEBANNER, 0, (LPARAM)(lpcwText))

Function Edit_SetCueBannerText( hwnd : hwnd; lpcwText : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), EM_SETCUEBANNER, 0, lpcwText))
end;


// Macro 311
//#define Edit_GetCueBannerText(hwnd, lpwText, cchText) \
//         (BOOL)SNDMSG((hwnd), EM_GETCUEBANNER, (WPARAM)(lpwText), (LPARAM)(cchText))

Function Edit_GetCueBannerText( hwnd : hwnd; lpwText : WPARAM; cchText : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), EM_GETCUEBANNER, lpwText, cchText))
end;


// Macro 312
// #define Edit_ShowBalloonTip(hwnd, peditballoontip) \
//         (BOOL)SNDMSG((hwnd), EM_SHOWBALLOONTIP, 0, (LPARAM)(peditballoontip))

Function Edit_ShowBalloonTip( hwnd : hwnd; peditballoontip : LPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), EM_SHOWBALLOONTIP, 0, peditballoontip))
end;


// Macro 313
// #define Edit_HideBalloonTip(hwnd) \
//         (BOOL)SNDMSG((hwnd), EM_HIDEBALLOONTIP, 0, 0)

Function Edit_HideBalloonTip( hwnd : hwnd):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), EM_HIDEBALLOONTIP, 0, 0))
end;


// Macro 314
//#define ComboBox_SetMinVisible(hwnd, iMinVisible) \
//             (BOOL)SNDMSG((hwnd), CB_SETMINVISIBLE, (WPARAM)iMinVisible, 0)

Function ComboBox_SetMinVisible( hwnd : hwnd; iMinVisible : WPARAM):BOOL;

Begin
 Result:=BOOL(SendMessage((hwnd), CB_SETMINVISIBLE, iMinVisible, 0))
end;


// Macro 315
// #define ComboBox_GetMinVisible(hwnd) \
//             (int)SNDMSG((hwnd), CB_GETMINVISIBLE, 0, 0)

Function ComboBox_GetMinVisible( hwnd : hwnd):cint;

Begin
 Result:=cint(SendMessage((hwnd), CB_GETMINVISIBLE, 0, 0))
end;

{$ifdef win32vista}
function  TreeView_GetNextSelected(hwnd:hwnd; hitem:HTREEITEM):HTREEITEM;inline;   
begin
 result:=TreeView_GetNextItem(hwnd, hitem,  TVGN_NEXTSELECTED)
end;
{$endif}

{$ifdef win32vista}
function Button_SetDropDownState(hwnd:HWND; fDropDown:BOOL) : BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), BCM_SETDROPDOWNSTATE, WPARAM(fDropDown), 0));
end;

function Button_SetSplitInfo(hwnd:HWND; pInfo:PBUTTON_SPLITINFO):BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), BCM_SETSPLITINFO, 0, LPARAM(pInfo)));
end;

function Button_GetSplitInfo(hwnd:HWND; pInfo:PBUTTON_SPLITINFO) :BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), BCM_GETSPLITINFO, 0, LPARAM(pInfo)));
end;

function Button_SetNote(hwnd:HWND; psz:LPCWSTR) :BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), BCM_SETNOTE, 0, LPARAM(psz)));
end;

function Button_GetNote(hwnd:HWND; psz:LPCWSTR; pcc:cint) :BOOL;
Begin
 Result:=BOOL(SendMessage((hwnd), BCM_GETNOTE, WPARAM(pcc), LPARAM(psz)));
end;

function Button_GetNoteLength(hwnd:HWND) :LRESULT;
Begin
 Result:=LRESULT(SendMessage((hwnd), BCM_GETNOTELENGTH, 0, 0));
end;

// Macro to use on a button or command link to display an elevated icon

function Button_SetElevationRequiredState(hwnd:HWND; fRequired:BOOL) :LRESULT;
Begin
 Result:=LRESULT(SendMessage((hwnd),  BCM_SETSHIELD, 0, LPARAM(fRequired)));
end;
{$endif}
{$ifdef win32vista}
procedure Edit_SetHilite(hwndCtl:hwnd; ichStart:Wparam; ichEnd:lparam);
begin
  sendmessage(hwndctl, EM_SETHILITE, (ichStart), (ichEnd));
end;

function Edit_GetHilite(hwndCtl:hwnd):Dword;
begin
 result:=SendMessage((hwndCtl), EM_GETHILITE, 0, 0);
end;
{$endif}

{$ifdef ntddi_vista}
function MonthCal_GetCurrentView(hmc:HWND):DWORD;
Begin
 Result:=DWord(SendMessage(hmc, MCM_GETCURRENTVIEW, 0, 0));
end;

function MonthCal_GetCalendarCount(hmc:HWND):DWORD;
Begin
 Result:=DWord(SendMessage(hmc,MCM_GETCALENDARCOUNT, 0, 0));
end;

function MonthCal_GetCalendarGridInfo(hmc:HWND; pmc:pMCGRIDINFO):BOOL;
Begin
 Result:=BOOL(SendMessage(hmc, MCM_GETCALENDARGRIDINFO, 0, LPARAM(PMCGRIDINFO(pmc))));
end;

function MonthCal_GetCALID(hmc:HWND):CALID;
Begin
  Result:=CALID(SendMessage(hmc, MCM_GETCALID, 0, 0));
end;

function MonthCal_SetCALID(hmc:HWND; calid:cuint):LRESULT;
Begin
 Result:=LRESULT(SendMessage(hmc, MCM_SETCALID, WPARAM(calid), 0));
end;

function MonthCal_SizeRectToMin(hmc:HWND; prc:prect):LRESULT;
Begin
 Result:=LRESULT(SendMessage(hmc, MCM_SIZERECTTOMIN, 0, LPARAM(prc)));
end;

function MonthCal_SetCalendarBorder(hmc:HWND; fset:bool; xyborder:cint):LRESULT;
Begin
 Result:=LRESULT(SendMessage(hmc, MCM_SETCALENDARBORDER, WPARAM(fset), LPARAM(xyborder)));
end;

function MonthCal_GetCalendarBorder(hmc:HWND):cint;
Begin
 Result:=cint(SendMessage(hmc, MCM_GETCALENDARBORDER, 0, 0));
end;

function MonthCal_SetCurrentView(hmc:HWND; dwNewView:DWord):BOOL;
Begin
 Result:=BOOL(SendMessage(hmc, MCM_SETCURRENTVIEW, 0, LPARAM(dwNewView)));
end;

{$endif}

{$ifdef NTDDI_VISTA}
function DateTime_SetMonthCalStyle(hdp:HWND; dwStyle:DWord):LResult;
Begin
 Result:=LRESULT(SendMessage(hdp,DTM_SETMCSTYLE, 0, LPARAM(dwStyle)));
end;

function DateTime_GetMonthCalStyle(hdp:HWND):LRESULT;
Begin
 Result:=LRESULT(SendMessage(hdp,DTM_GETMCSTYLE, 0, 0));
end;

function DateTime_CloseMonthCal(hdp:HWND):LRESULT;
Begin
 Result:=LRESULT(SendMessage(hdp,DTM_CLOSEMONTHCAL, 0, 0));
end;

// DateTime_GetDateTimePickerInfo(HWND hdp, DATETIMEPICKERINFO* pdtpi)
// Retrieves information about the selected date time picker.

function DateTime_GetDateTimePickerInfo(hdp:HWND; pdtpi:PDATETIMEPICKERINFO):LRESULT;
Begin
 Result:=LRESULT(SendMessage(hdp, DTM_GETDATETIMEPICKERINFO, 0, LPARAM(pdtpi)));
end;

function DateTime_GetIdealSize(hdp:HWND; ps:PSIZE): LResult;
Begin
 Result:=LRESULT(SendMessage(hdp, DTM_GETIDEALSIZE, 0, LPARAM(ps)));
end;

{$endif}

// Macro 284
procedure DateTime_SetMonthCalFont(hdp:HWND; hfont:HFONT; fRedraw:LPARAM);
begin
  SendMessage(hdp, DTM_SETMCFONT, WPARAM(hfont), LPARAM(fRedraw));
end;

// Macro 285
function DateTime_GetMonthCalFont(hdp:HWND):HFONT;
begin
 Result:=HFONT(SendMessage(hdp, DTM_GETMCFONT, 0, 0));
end;

const
  comctl32lib = 'comctl32.dll';

var
  comctl32handle: THandle = 0;

function _TaskDialogIndirect(const pTaskConfig: PTASKDIALOGCONFIG; pnButton: PInteger; pnRadioButton: PInteger; pfVerificationFlagChecked: PBOOL): HRESULT; stdcall;
begin
  Result := E_NOTIMPL;
end;

function _TaskDialog(hwndParent: HWND; hInstance: HINST; pszWindowTitle: PCWSTR; pszMainInstruction: PCWSTR; pszContent: PCWSTR;
    dwCommonButtons: TASKDIALOG_COMMON_BUTTON_FLAGS; pszIcon: PCWSTR; pnButton: PInteger): HRESULT; stdcall;
begin
  Result := E_NOTIMPL;
end;

procedure Initialize;
var
  p: Pointer;
begin
  // Defaults
  Pointer(TaskDialogIndirect) := @_TaskDialogIndirect;
  Pointer(TaskDialog) := @_TaskDialog;

  comctl32handle := LoadLibrary(comctl32lib);
  if comctl32handle <> 0 then
  begin
    p := GetProcAddress(comctl32handle, 'TaskDialogIndirect');
    if p <> nil
    then Pointer(TaskDialogIndirect) := p;

    p := GetProcAddress(comctl32handle, 'TaskDialog');
    if p <> nil
    then Pointer(TaskDialog) := p;
  end;
end;

procedure Finalize;
begin
  if comctl32handle <> 0 then
    FreeLibrary(comctl32handle);
  comctl32handle := 0;
end;

initialization
  Initialize;
Finalization
  Finalize;  
End.
