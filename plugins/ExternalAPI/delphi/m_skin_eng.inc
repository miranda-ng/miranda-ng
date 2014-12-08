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

{$IFNDEF M_SKE}
{$DEFINE M_SKE}

const
//*defaults*/
//  DEFAULT_FIT_MODE   = FM_STRETCH;
//  DEFAULT_STYLE      = ST_BRUSH  ;
//  DEFAULT_BKCOLOUR    GetSysColor(COLOR_3DFACE);
//  DEFAULT_SELBKCOLOUR GetSysColor(COLOR_HIGHLIGHT);

  SIZING_MARGIN = 3;

//* Fit mode */
  FM_STRETCH   = 0;
  FM_TILE_HORZ = 1;
  FM_TILE_VERT = 2;
  FM_TILE_BOTH = 3;

//*Object types*/
  OT_ANY         = 0;
  OT_GLYPHOBJECT = 1;
  OT_FONTOBJECT  = 2;

//*STYLE INDEXEX*/
  ST_SKIP     = 0;
  ST_PARENT   = 1;
  ST_BRUSH    = 2;
  ST_IMAGE    = 3;
  ST_SOLARIZE = 4;  //Not used yet.
  ST_FRAGMENT = 5;
  ST_GRADIENT = 6;

//formats:
  ADT_TOP     = $00000000;
  ADT_LEFT    = $00000000;
  ADT_HCENTER = $00000001;
  ADT_RIGHT   = $00000002;
  ADT_VCENTER = $00000004;
  ADT_BOTTOM  = $00000008;
//  ADT_ECLIPSE = 64;


//*SERVICES*/

//toggle the 'hide offline contacts' flag and call CLUI   
//wParam=0
//lParam=0
  MS_CLIST_TOGGLEHIDEOFFLINE:PAnsiChar = 'CList/ToggleHideOffline';

  MS_CLIST_TOGGLEGROUPS:PAnsiChar = 'CList/ToggleGroups';

  MS_CLIST_TOGGLESOUNDS:PAnsiChar = 'CList/ToggleSounds';

// Add new object to skin object list.
// wParam = pointer to SKINOBJECTDESCRIPTOR structure
// lParam = 0 ( used for internal purposes: pointer to skin object list)
  MS_SKIN_REGISTEROBJECT:PAnsiChar = 'ModernList/RegisterObject';

// Add new object to skin object list.
// wParam = pointer to DEF_SKIN_OBJECT_PARAMS structure
// lParam = 0 ( used for internal purposes: pointer to skin object list)
  MS_SKIN_REGISTERDEFOBJECT:PAnsiChar = 'ModernList/RegisterDefObject';

type
  DEF_SKIN_OBJECT_PARAMS = record
    szObjectID:PAnsiChar;
    defStyle:byte;
    defColor:dword;
    //  Skin: PSKINOBJECTSLIST;
  end;

const
// Request painting glyph object 
// wParam = pointer to SKINDRAWREQUEST structure
// lParam = 0
  MS_SKIN_DRAWGLYPH:PAnsiChar = 'ModernList/DrawGlyph';


//* EVENTS */
  ME_SKIN_SERVICESCREATED:PAnsiChar = 'ModernList/ServicesCreated';

//* DRAWGLYPH Request structure */
type
  PSKINDRAWREQUEST = ^TSKINDRAWREQUEST;
  TSKINDRAWREQUEST = record
    szObjectID:array [0..254] of AnsiChar; // Unic Object ID (path) to paint
    rcDestRect:TRECT;                      // Rectangle to fit
    rcClipRect:TRECT;                      // Rectangle to paint in.
    hDC:HDC;                               // Handler to device context to paint in. 
  end;

//* SKINOBJECTDESCRIPTOR opbject descriptor structure */
type
  PSKINOBJECTDESCRIPTOR = ^TSKINOBJECTDESCRIPTOR;
  TSKINOBJECTDESCRIPTOR = record
    bType:byte;           // One of OT_* values.
    szObjectID:PAnsiChar; // Unic Object ID (path) [255] max
    Data:pointer;         // Pointer to GLYPHOBJECT strycture if bType==OT_GLYPHOBJECT
  end;

//* SKINOBJECTDESCRIPTOR opbject descriptor structure */
type
  PGLYPHOBJECT = ^TGLYPHOBJECT;
  TGLYPHOBJECT = record
    Style      :byte;        // One of ST_* values 
    hGlyph     :HBITMAP;     // Bitmap handler (for internal use only)
    dwTop,
    dwLeft,
    dwBottom,
    dwRight    :dword;       // Margins 
    szFileName :PAnsiChar;   // FileName of image
    dwColor    :dword;       // Fill color
    dwAlpha    :byte;        // Constant alpha-transparency level
    FitMode    :byte;        // One of FM_* values
    clipArea   :TPOINT;      // Object image rect on full image
    szclipArea :TSIZE;       // Object image rect on full image
    plTextList :PSortedList; // List of GLYPHTEXT
    bmWidth    :long;
    bmHeight   :long;
    bmBitsPixel:byte;
  end;

//* SKINTEXTDESCRIPTOR opbject descriptor structure */
type
  PGLYPHTEXT = ^TGLYPHTEXT;
  TGLYPHTEXT = record
    szGlyphTextID:PAnsiChar;
    stText       :TChar;
    stValueText  :TChar;
    dwFlags      :dword;
    dwColor      :dword;  // Color (InvAA)(RR)(GG)(BB)
    dwShadow     :dword;  //ToDo: Color2/Shaddow
    iLeft,iTop,iRight,iBottom:int;
    RelativeFlags:byte;
    szFontID     :PAnsiChar;
    hFont        :HFONT;
    szObjectName :PAnsiChar;
  end;

//* SKINTEXTDESCRIPTOR opbject descriptor structure */
type
  PSKINFONT = ^TSKINFONT;
  TSKINFONT = record
    szFontID:PAnsiChar;
    hFont   :HFONT;
  end;

//* HELPER FUNCTIONS */
(*
//Paint  ObjectID as parent background for frame hwndIn
int __inline SkinDrawWindowBack(HWND hwndIn, HDC hdc, RECT * rcClip, char * objectID)
{
	SKINDRAWREQUEST rq;
	POINT pt={0};
	RECT rc,r1;

	HWND hwnd=(HWND)CallService(MS_CLUI_GETHWND,0,0);
	if (!objectID) return 0;
	GetWindowRect(hwndIn,&r1);
	pt.x=r1.left;
	pt.y=r1.top;
	//ClientToScreen(hwndIn,&pt);
	GetWindowRect(hwnd,&rc);
	OffsetRect(&rc,-pt.x ,-pt.y);
	rq.hDC=hdc;
	rq.rcDestRect=rc;
	rq.rcClipRect=*rcClip;
	lstrcpynA(rq.szObjectID,objectID,sizeof(rq.szObjectID));
	///ske_Service_DrawGlyph((WPARAM)&rq,0);    //$$$
	return CallService(MS_SKIN_DRAWGLYPH,(WPARAM)&rq,0);
}


//Paint  ObjectID
int __inline SkinDrawGlyph(HDC hdc, RECT * rcSize, RECT * rcClip, char * objectID);

//Register object with predefined style
int __inline CreateGlyphedObjectDefStyle(char * ObjID,BYTE defStyle);
int __inline CreateGlyphedObjectDefColor(char * ObjID,dword defColor);
//Register default object
int __inline CreateGlyphedObject(char * ObjID);


//// Creating and registering objects
//int __inline CreateGlyphedObject(char * ObjID)
//{   
//    DEF_SKIN_OBJECT_PARAMS prm={0};
//    prm.defColor=DEFAULT_BKCOLOUR;
//    prm.defStyle=DEFAULT_STYLE;
//    prm.szObjectID=ObjID;   
//    return CallService(MS_SKIN_REGISTERDEFOBJECT,(WPARAM)&prm,0);
//}
static BOOL __inline ScreenToClientRect(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}

//int __inline CreateGlyphedObjectDefStyle(char * ObjID,BYTE defStyle)
//{
//    DEF_SKIN_OBJECT_PARAMS prm={0};
//    prm.defColor=DEFAULT_BKCOLOUR;
//    prm.defStyle=defStyle;
//    prm.szObjectID=ObjID;
//    return CallService(MS_SKIN_REGISTERDEFOBJECT,(WPARAM)&prm,0);
//}  
//int __inline CreateGlyphedObjectDefColor(char * ObjID,dword defColor)
//{
//    DEF_SKIN_OBJECT_PARAMS prm={0};
//    prm.defColor=defColor;
//    prm.defStyle=ST_BRUSH;
//    prm.szObjectID=ObjID;
//    return CallService(MS_SKIN_REGISTERDEFOBJECT,(WPARAM)&prm,0);
//}  
static int __inline SkinDrawGlyph(HDC hdc, RECT * rcSize, RECT * rcClip, char * objectID)
{
	SKINDRAWREQUEST rq;
	if (!objectID) return 0;
	rq.hDC=hdc;
	rq.rcDestRect=*rcSize;
	rq.rcClipRect=*rcClip;  
	lstrcpynA(rq.szObjectID,objectID,sizeof(rq.szObjectID));
	return CallService(MS_SKIN_DRAWGLYPH,(WPARAM)&rq,0);
}
*)
//#include "../hdr/modern_skin_selector.h"

//////////////////////////////////////////////
//                                          //
//  New Painting sequence servises          //
//                                          //
//////////////////////////////////////////////

type
  sPaintRequest = record
    dStructSize :dword;                 // size of structure
    hWnd        :HWND;                  // called by window
    hDC         :HDC;                   // context to draw on
    rcUpdate    :TRECT;                 // rectangle to be painted in (relative to Top-Left corner of Main window)
    dwFlags     :dword;                 // drawing flags
    CallbackData:pointer;               // Data for passing to callback procedure
    Reserved    :array [0..15] of byte; // reserved for farther usage;
  end;

const
// Request to register sub for callback painting frame area
// wParam = hWnd of called frame
// lParam = pointer to tPaintCallBackProc    (or NULL to remove)
// return 1 - succes, 0 - failure
  MS_SKINENG_REGISTERPAINTSUB:PAnsiChar = 'SkinEngine/ske_Service_RegisterFramePaintCallbackProcedure';

// Request to repaint frame or change/drop callback data immeadeately
// wParam = hWnd of called frame
// lParam = pointer to sPaintRequest (or NULL to redraw all)
  MS_SKINENG_UPTATEFRAMEIMAGE:PAnsiChar = 'SkinEngine/ske_Service_UpdateFrameImage';

// Request to repaint frame or change/drop callback data
// wParam = hWnd of called frame
// lParam = pointer to sPaintRequest (or NULL to redraw all)
// return 2 - already queued, data updated, 1-have been queued, 0 - failure
  MS_SKINENG_INVALIDATEFRAMEIMAGE:PAnsiChar = 'SkinEngine/ske_Service_InvalidateFrameImage';

// Callback proc type
type
  tPaintCallbackProc = function(hWnd:HWND; hDC:HDC; rcPaint:PRECT; rgnUpdate:HRGN;
       dFlags:dword; CallBackData:pointer):int;

// HELPER TO UPDATEIMAGEFRAME

(*
inline BOOL isSkinEngineEnabled()
{
	return ServiceExists(MS_SKINENG_REGISTERPAINTSUB) && !DBGetContactSettingByte(NULL, "ModernData", "DisableEngine", FALSE);
}


inline BOOL isLayeredEnabled()
{
	return isSkinEngineEnabled() && DBGetContactSettingByte(NULL, "ModernData", "EnableLayering", TRUE);
}

int __inline SkinEngUpdateImageFrame(HWND hwnd, RECT * rcUpdate, dword dwFlags, void * CallBackData)
{
  sPaintRequest sr={0};
  sr.dStructSize=sizeof(sPaintRequest);
  sr.hWnd=hwnd;
  if (rcUpdate)
    sr.rcUpdate=*rcUpdate;
  sr.dwFlags=dwFlags;
  sr.CallbackData=CallBackData;
  return CallService(MS_SKINENG_UPTATEFRAMEIMAGE,(WPARAM)hwnd,(LPARAM)&sr);
}

int __inline SkinEngInvalidateImageFrame(HWND hwnd, CONST RECT * rcUpdate, dword dwFlags, void * CallBackData)
{
  sPaintRequest sr={0};
  if (hwnd && !isLayeredEnabled()) return InvalidateRect(hwnd,rcUpdate,dwFlags);
  sr.dStructSize=sizeof(sPaintRequest);
  sr.hWnd=hwnd;
  if (rcUpdate)
    sr.rcUpdate=*rcUpdate;
  sr.dwFlags=dwFlags;
  sr.CallbackData=CallBackData;
  return CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE,(WPARAM)hwnd,(LPARAM)&sr);
}


int __inline SkinInvalidateFrame(HWND hWnd, CONST RECT* lpRect,BOOL bErase)
{
	return SkinEngInvalidateImageFrame(hWnd,lpRect,0,0);
}
*)
// Alpha channel GDI replacements/helpers

const
// 
// Paints text with correct alpha channel
// wParam - pointer to AlphaTextOutParams
  MS_SKINENG_ALPHATEXTOUT:PAnsiChar = 'SkinEngine/ske_AlphaTextOut';

type
  TAlphaTextOutParams = record
    HDC      :HDC;
    lpString :TChar;
    nCount   :int;
    lpRect   :PRECT;
    forma    :uint;
    ARGBcolor:dword;
    reserv   :array [0..15] of byte;
  end;
(*
int __inline AlphaText(HDC hDC, LPCTSTR lpString, int nCount, RECT * lpRect, UINT format, dword ARGBcolor)
{
  AlphaTextOutParams ap={0};
  ap.hDC=hDC;
  ap.lpString=lpString;
  ap.nCount=nCount;
  ap.lpRect=lpRect;
  ap.format=format;
  ap.ARGBcolor=ARGBcolor;
  return CallService(MS_SKINENG_ALPHATEXTOUT,(WPARAM)&ap,0);
}
*)
type
  ImageListFixParam = record
    himl :THANDLE; //HIMAGELIST;
    index:int;
    hicon:HICON;
  end;

type
  DrawIconFixParam = record
    hdc               :HDC;
    xLeft             :int;
    yTop              :int;
    hIcon             :HICON;
    cxWidth           :int;
    cyWidth           :int;
    istepIfAniCur     :uint;
    hbrFlickerFreeDraw:HBRUSH;
    diFlags           :uint;
  end;

const
//wParam - pointer to DrawIconFixParam
  MS_SKINENG_DRAWICONEXFIX:PAnsiChar = 'SkinEngine/DrawIconEx_Fix';
(*
int __inline mod_DrawIconEx_helper(HDC hdc,int xLeft,int yTop,HICON hIcon,int cxWidth,int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
  DrawIconFixParam p={0};
  p.hdc=hdc;
  p.xLeft=xLeft;
  p.yTop=yTop;
  p.hIcon=hIcon;
  p.cxWidth=cxWidth;
  p.cyWidth=cyWidth;
  p.istepIfAniCur=istepIfAniCur;
  p.hbrFlickerFreeDraw=hbrFlickerFreeDraw;
  p.diFlags=diFlags;
  return CallService(MS_SKINENG_DRAWICONEXFIX,(WPARAM)&p,0);
}
*)
const
//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
  MS_BACKGROUNDCONFIG_REGISTER:PAnsiChar = 'ModernBkgrCfg/Register';

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
  ME_BACKGROUNDCONFIG_CHANGED:PAnsiChar = 'ModernBkgrCfg/Changed';


{$ENDIF}
