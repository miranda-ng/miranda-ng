/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#ifndef M_ske_H_INC
#define M_ske_H_INC

#include <m_clui.h>

/*defaults*/
#define DEFAULT_FIT_MODE    FM_STRETCH
#define DEFAULT_STYLE       ST_BRUSH
#define DEFAULT_BKCOLOUR    GetSysColor(COLOR_3DFACE)
#define DEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)
#define SIZING_MARGIN 3

/* Fit mode */
#define FM_STRETCH      0
#define FM_TILE_HORZ    1
#define FM_TILE_VERT    2
#define FM_TILE_BOTH    3

/*Object types*/
#define OT_ANY          0
#define OT_GLYPHOBJECT  1
#define OT_FONTOBJECT   2

/*STYLE INDEXEX*/
#define ST_SKIP         0
#define ST_PARENT       1
#define ST_BRUSH        2
#define ST_IMAGE        3
#define ST_SOLARIZE     4  //Not used yet.
#define ST_FRAGMENT     5
#define ST_GRADIENT     6

//formats:
#define ADT_TOP                      0x00000000
#define ADT_LEFT                     0x00000000
#define ADT_HCENTER                  0x00000001
#define ADT_RIGHT                    0x00000002
#define ADT_VCENTER                  0x00000004
#define ADT_BOTTOM                   0x00000008
//#define ADT_ECLIPSE 64


/*SERVICES*/

//toggle the 'hide offline contacts' flag and call CLUI
//wParam=0
//lParam=0
#define MS_CLIST_TOGGLEHIDEOFFLINE  "CList/ToggleHideOffline"

#define MS_CLIST_TOGGLEGROUPS  "CList/ToggleGroups"

#define MS_CLIST_TOGGLESOUNDS  "CList/ToggleSounds"

// Add new object to skin object list.
// wParam = pointer to SKINOBJECTDESCRIPTOR structure
// lParam = 0 ( used for internal purposes: pointer to skin object list)
#define MS_SKIN_REGISTEROBJECT "ModernList/RegisterObject"

// Add new object to skin object list.
// wParam = pointer to DEF_SKIN_OBJECT_PARAMS structure
// lParam = 0 ( used for internal purposes: pointer to skin object list)
#define MS_SKIN_REGISTERDEFOBJECT "ModernList/RegisterDefObject"

typedef struct s_DEF_SKIN_OBJECT_PARAMS
{
  char * szObjectID;
  BYTE defStyle;
  DWORD defColor;
  //    SKINOBJECTSLIST * Skin;
} DEF_SKIN_OBJECT_PARAMS;


// Request painting glyph object
// wParam = pointer to SKINDRAWREQUEST structure
// lParam = 0
#define MS_SKIN_DRAWGLYPH "ModernList/DrawGlyph"



/* EVENTS */
#define ME_SKIN_SERVICESCREATED "ModernList/ServicesCreated"

/* DRAWGLYPH Request structure */
typedef struct s_SKINDRAWREQUEST
{
  char szObjectID[255];      // Unic Object ID (path) to paint
  RECT rcDestRect;           // Rectangle to fit
  RECT rcClipRect;           // Rectangle to paint in.
  HDC hDC;                   // Handler to device context to paint in.
} SKINDRAWREQUEST,*LPSKINDRAWREQUEST;

/* SKINOBJECTDESCRIPTOR opbject descriptor structure */
typedef struct tagSKINOBJECTDESCRIPTOR
{
  BYTE    bType;              // One of OT_* values.
  char*   szObjectID;         // Unic Object ID (path) [255] max
  LPVOID  Data;               // Pointer to GLYPHOBJECT strycture if bType==OT_GLYPHOBJECT
} SKINOBJECTDESCRIPTOR, *LPSKINOBJECTDESCRIPTOR;

/* SKINOBJECTDESCRIPTOR opbject descriptor structure */
typedef struct s_GLYPHOBJECT
{
  BYTE Style;                                // One of ST_* values
  HBITMAP hGlyph;                            // Bitmap handler (for internal use only)
  DWORD dwTop, dwLeft, dwBottom, dwRight;    // Margins
  char* szFileName;                          // FileName of image
  DWORD dwColor;                             // Fill color
  BYTE dwAlpha;                              // Constant alpha-transparency level
  BYTE FitMode;                              // One of FM_* values
  POINT clipArea;                            // Object image rect on full image
  SIZE szclipArea;                           // Object image rect on full image
  SortedList * plTextList;                   // List of GLYPHTEXT
  LONG bmWidth;
  LONG bmHeight;
  BYTE bmBitsPixel;
} GLYPHOBJECT,*LPGLYPHOBJECT;

/* SKINTEXTDESCRIPTOR opbject descriptor structure */
typedef struct s_GLYPHTEXT
{
  char        * szGlyphTextID;
  TCHAR       * stText;
  TCHAR       * stValueText;
  DWORD         dwFlags;
  DWORD         dwColor;                     // Color (InvAA)(RR)(GG)(BB)
  DWORD         dwShadow;                    //ToDo: Color2/Shaddow
  int           iLeft,iTop,iRight,iBottom;
  BYTE          RelativeFlags;
  char        * szFontID;
  HFONT         hFont;
  char		  * szObjectName;
}GLYPHTEXT,*LPGLYPHTEXT;

/* SKINTEXTDESCRIPTOR opbject descriptor structure */
typedef struct s_SKINFONT
{
  char        * szFontID;
  HFONT         hFont;
}SKINFONT, *LPSKINFONT;

/* HELPER FUNCTIONS */

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



//Register object with predefined style
int __inline CreateGlyphedObjectDefStyle(char * ObjID,BYTE defStyle);
int __inline CreateGlyphedObjectDefColor(char * ObjID,DWORD defColor);
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
//int __inline CreateGlyphedObjectDefColor(char * ObjID,DWORD defColor)
//{
//    DEF_SKIN_OBJECT_PARAMS prm={0};
//    prm.defColor=defColor;
//    prm.defStyle=ST_BRUSH;
//    prm.szObjectID=ObjID;
//    return CallService(MS_SKIN_REGISTERDEFOBJECT,(WPARAM)&prm,0);
//}

//Paint  ObjectID
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
//#include "../hdr/modern_skin_selector.h"

//////////////////////////////////////////////
//                                          //
//  New Painting sequence servises          //
//                                          //
//////////////////////////////////////////////

typedef struct sPAINT_REQUEST
{
  DWORD dStructSize;      //size of structure
  HWND  hWnd;             //called by window
  HDC   hDC;              //context to draw on
  RECT  rcUpdate;         //rectangle to be painted in (relative to Top-Left corner of Main window)
  DWORD dwFlags;          //drawing flags
  void * CallbackData;    //Data for passing to callback procedure
  char Reserved[16];      //reserved for farther usage;
} sPaintRequest;

// Request to register sub for callback painting frame area
// wParam = hWnd of called frame
// lParam = pointer to tPaintCallBackProc    (or NULL to remove)
// return 1 - succes, 0 - failure
#define MS_SKINENG_REGISTERPAINTSUB "SkinEngine/ske_Service_RegisterFramePaintCallbackProcedure"

// Request to repaint frame or change/drop callback data immeadeately
// wParam = hWnd of called frame
// lParam = pointer to sPaintRequest (or NULL to redraw all)
#define MS_SKINENG_UPTATEFRAMEIMAGE "SkinEngine/ske_Service_UpdateFrameImage"

// Request to repaint frame or change/drop callback data
// wParam = hWnd of called frame
// lParam = pointer to sPaintRequest (or NULL to redraw all)
// return 2 - already queued, data updated, 1-have been queued, 0 - failure
#define MS_SKINENG_INVALIDATEFRAMEIMAGE "SkinEngine/ske_Service_InvalidateFrameImage"

// Callback proc type
typedef int (/*__stdcall*/ *tPaintCallbackProc)(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgnUpdate, DWORD dFlags, void * CallBackData);
//tPaintCallbackProc PaintCallbackProc;

// HELPER TO UPDATEIMAGEFRAME


__inline BOOL isSkinEngineEnabled()
{
	return ServiceExists(MS_SKINENG_REGISTERPAINTSUB) && !db_get_b(NULL, "ModernData", "DisableEngine", FALSE);
}


__inline BOOL isLayeredEnabled()
{
	return isSkinEngineEnabled() && db_get_b(NULL, "ModernData", "EnableLayering", TRUE);
}

int __inline SkinEngUpdateImageFrame(HWND hwnd, RECT * rcUpdate, DWORD dwFlags, void * CallBackData)
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

int __inline SkinEngInvalidateImageFrame(HWND hwnd, CONST RECT * rcUpdate, DWORD dwFlags, void * CallBackData)
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


int __inline SkinInvalidateFrame(HWND hWnd, CONST RECT* lpRect)
{
	return SkinEngInvalidateImageFrame(hWnd, lpRect, 0, 0);
}

//
// Paints text with correct alpha channel
// wParam - pointer to AlphaTextOutParams
#define MS_SKINENG_ALPHATEXTOUT "SkinEngine/ske_AlphaTextOut"
typedef struct _AlphaTextOutParams
{
  HDC hDC;
  LPCTSTR lpString;
  int nCount;
  RECT * lpRect;
  UINT format;
  DWORD ARGBcolor;
  char reserv[16];
}AlphaTextOutParams;

int __inline AlphaText(HDC hDC, LPCTSTR lpString, int nCount, RECT * lpRect, UINT format, DWORD ARGBcolor)
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

typedef struct _ImageListFixParam
{
  HIMAGELIST himl;
  int index;
  HICON hicon;
}ImageListFixParam;

typedef struct _DrawIconFixParam
{
  HDC hdc;
  int xLeft;
  int yTop;
  HICON hIcon;
  int cxWidth;
  int cyWidth;
  UINT istepIfAniCur;
  HBRUSH hbrFlickerFreeDraw;
  UINT diFlags;
} DrawIconFixParam;
//wParam - pointer to DrawIconFixParam
#define MS_SKINENG_DRAWICONEXFIX "SkinEngine/DrawIconEx_Fix"

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




//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
#define MS_BACKGROUNDCONFIG_REGISTER "ModernBkgrCfg/Register"

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
#define ME_BACKGROUNDCONFIG_CHANGED "ModernBkgrCfg/Changed"



#endif
