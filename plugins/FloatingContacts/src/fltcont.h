
#ifndef __FLTCONT_H__
#define __FLTCONT_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
//

#define WND_CLASS					_T("MirandaThumbsWnd")
#define	WM_REFRESH_CONTACT			WM_USER + 0x100

#ifndef WS_EX_LAYERED
	#define WS_EX_LAYERED           0x00080000
#endif

#ifndef LWA_ALPHA
	#define LWA_ALPHA               0x00000002
#endif

#ifndef ULW_ALPHA
	#define ULW_ALPHA				0x00000002
#endif

#define TIMERID_SELECT_T			1
#define TIMERID_HOVER_T				2
#define TIMERID_TOTOP_T				3
#define TIMERID_LEAVE_T				4

/////////////////////////////////////////////////////////////////////////////
//

enum
{
	FLT_FONTID_CONTACTS,
	FLT_FONTID_INVIS,
	FLT_FONTID_OFFLINE,
	FLT_FONTID_OFFINVIS,
	FLT_FONTID_NOTONLIST,
	FLT_FONTIDS,
};

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

/////////////////////////////////////////////////////////////////////////////
//

#define FLT_DEFAULT_DRAWBORDER			TRUE
#define FLT_DEFAULT_LTEDGESCOLOR		GetSysColor(COLOR_3DHILIGHT)
#define FLT_DEFAULT_RBEDGESCOLOR		GetSysColor(COLOR_3DDKSHADOW)
#define FLT_DEFAULT_BKGNDCOLOR			GetSysColor(COLOR_3DFACE)
#define FLT_DEFAULT_BKGNDUSEBITMAP		FALSE
#define FLT_DEFAULT_BKGNDBITMAPOPT		CLB_STRETCH

/////////////////////////////////////////////////////////////////////////////
//

#define MODULE "FloatingContacts"

#define TOTOPTIME_P	1000
#define TOTOPTIME_MAX	(15*(60000/TOTOPTIME_P))
#define TOTOPTIME_DEF	(3*(60000/TOTOPTIME_P))

#define MAXRCOOR 32767.0
#define DB_POS_GETX(pos) (short)(((float)(short)(LOWORD(pos))*(float)GetSystemMetrics(SM_CXSCREEN))/MAXRCOOR+0.5)
#define DB_POS_GETY(pos) (short)(((float)(short)(HIWORD(pos))*(float)GetSystemMetrics(SM_CYSCREEN))/MAXRCOOR+0.5)
#define DB_POS_MAKE_XY(x, y) MAKELONG((short)(((float)x*MAXRCOOR)/(float)GetSystemMetrics(SM_CXSCREEN)+0.5), (short)(((float)y*MAXRCOOR)/(float)GetSystemMetrics(SM_CYSCREEN)+0.5))

extern HINSTANCE	hInst;

//extern BOOL			bHideOffline;
//extern BOOL			bHideAll;
//extern BOOL			bHideWhenFullscreen;
//extern BOOL			bMoveTogether;
//extern BOOL			bFixedWidth;
//extern int			nThumbWidth;
//extern BYTE			thumbAlpha;
//extern BOOL			bShowTip;
extern BOOL			bEnableTip;
//extern WORD			TimeIn;
//extern BOOL			bToTop;
//extern WORD			ToTopTime;
//extern BOOL			bHideWhenCListShow;

extern BOOL			bIsCListShow;
extern HWND			hwndMiranda;
extern HIMAGELIST	himlMiranda;
extern RECT			rcScreen;

extern HFONT		hFont[FLT_FONTIDS];
extern COLORREF		tColor[FLT_FONTIDS];

extern HPEN			hLTEdgesPen;
extern HPEN			hRBEdgesPen;
extern HBRUSH		hBkBrush;
extern DWORD		bkColor;
extern HBITMAP		hBmpBackground;
extern WORD			nBackgroundBmpUse;



typedef struct _FCOptions
{
	BYTE	thumbAlpha;
	BOOL	bHideOffline;
	BOOL	bHideAll;
	BOOL	bHideWhenFullscreen;
	BOOL	bMoveTogether;
	BOOL	bFixedWidth;
	int		nThumbWidth;
	BOOL	bShowTip;
	WORD	TimeIn;
	BOOL	bToTop;
	WORD	ToTopTime;
	BOOL	bHideWhenCListShow;
	BOOL	bUseSingleClick;
	BOOL	bShowIdle;
} 
FCOptions;

extern FCOptions fcOpt;

/////////////////////////////////////////////////////////////////////////////

static __forceinline BOOL ImageList_GetIconSize_my(HIMAGELIST himl, SIZE &sz)
{
	int cx, cy;
	BOOL res = ImageList_GetIconSize(himl, &cx, &cy);
	sz.cx = cx; sz.cy = cy;
	return res;
}

void RegHotkey				( MCONTACT hContact, HWND hwnd );
BOOL IsStatusVisible		( int status );
BOOL HideOnFullScreen		();
void SendMsgDialog			( HWND hwnd, TCHAR *pText );
void SaveContactsPos		( void );

/////////////////////////////////////////////////////////////////////////////

void ApplyOptionsChanges();

void OnStatusChanged();

void SetThumbsOpacity(BYTE btAlpha);

int OnOptionsInitialize(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////

#endif	// #ifndef __FLTCONT_H__

/////////////////////////////////////////////////////////////////////////////
// End Of File fltcont.h
