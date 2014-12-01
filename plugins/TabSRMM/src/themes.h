/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// The class CSkin implements the skinning engine and loads skins from
// their skin definition files (.tsk).
//
// CImageItem implements a single rectangular skin item with an image
// and its rendering.

#ifndef __THEMES_H
#define __THEMES_H

HBITMAP IMG_LoadLogo(const TCHAR *szName);

class CSideBarButton;

struct TSButtonCtrl : public MButtonCtrl
{
	HICON   hIconPrivate, overlay;
	bool    bToolbarButton;			// is a toolbar button (important for aero background rendering)
	bool    bTitleButton;
	bool    bDimmed;

	TContainerData *pContainer;
	CSideBarButton *sitem;
};

void CustomizeButton(HWND hwndButton);

#define BUTTONSETASDIMMED        (BUTTONSETASFLATBTN + 11)
#define BUTTONSETCONTAINER       (BUTTONSETASFLATBTN + 12)
#define BUTTONSETASTITLE         (BUTTONSETASFLATBTN + 13)
#define BUTTONSETASNORMAL        (BUTTONSETASFLATBTN + 14)
#define BUTTONGETSTATEID         (BUTTONSETASFLATBTN + 15)
#define BUTTONSETASTOOLBARBUTTON (BUTTONSETASFLATBTN + 21)
#define BUTTONSETASSIDEBARBUTTON (BUTTONSETASFLATBTN + 22)
#define BUTTONSETOVERLAYICON	   (BUTTONSETASFLATBTN + 23)

struct AeroEffect
{
	TCHAR    tszName[40];
	DWORD    m_baseColor;
	DWORD    m_gradientColor;
	BYTE     m_baseAlpha;
	BYTE     m_finalAlpha;
	BYTE     m_cornerType;
	BYTE     m_gradientType;
	DWORD    m_cornerRadius;
	DWORD    m_glowSize;
	COLORREF m_clrBack, m_clrToolbar, m_clrToolbar2;

	void (TSAPI	*pfnEffectRenderer)(const HDC hdc, const RECT *rc, int iEffectArea);
};
/**
 * CImageItem implementes image-based skin items. These items are loaded
 * from a skin file definition (.tsk file) and are then linked to one or
 * more skin items.
 */
class CImageItem
{
public:
	CImageItem()
	{
		memset(this, 0, sizeof(CImageItem));
	}
	CImageItem(const CImageItem& From)
	{
		*this = From;
		m_nextItem = 0;
	}
	CImageItem(const TCHAR *szName)
	{
		memset(this, 0, sizeof(CImageItem));
		mir_sntprintf(m_szName, 40, szName);
		m_szName[39] = 0;
	}

	CImageItem(BYTE bottom, BYTE left, BYTE top, BYTE right, HDC hdc, HBITMAP hbm, DWORD dwFlags,
			   HBRUSH brush, BYTE alpha, LONG inner_height, LONG inner_width, LONG height, LONG width)
	{
		m_bBottom = bottom;
		m_bLeft = left,
		m_bTop = top;
		m_bRight = right;
		m_hdc = hdc;
		m_hbm = hbm;
		m_dwFlags = dwFlags;
		m_fillBrush = brush;
		m_inner_height = inner_height;
		m_inner_width = inner_width;
		m_height = height;
		m_width = width;

		m_bf.SourceConstantAlpha = alpha;
		m_bf.AlphaFormat = 0;
		m_bf.BlendOp = AC_SRC_OVER;
		m_bf.BlendFlags = 0;
	}
	~CImageItem()
	{
		Free();
	}

	void			Clear()
	{
		m_hdc = 0; m_hbm = 0; m_hbmOld = 0;
		m_fillBrush = (HBRUSH)0;
	}

	void			setBitmap(const HBITMAP hbm)
	{
		m_hbm = hbm;
	}

	void			setAlphaFormat(const BYTE bFormat, const BYTE bConstantAlpha)
	{
		m_bf.AlphaFormat = bFormat;
		m_bf.SourceConstantAlpha = bConstantAlpha;
	}

	void			setMetrics(const LONG width, const LONG height)
	{
		m_height = height;
		m_width = width;

		m_inner_height = m_height - m_bBottom - m_bTop;
		m_inner_width = m_width - m_bLeft - m_bRight;
		if (!(m_dwFlags & IMAGE_FLAG_DIVIDED))
			m_bStretch = IMAGE_STRETCH_B;
	}

	void			Free();
	CImageItem*		getNextItem() const { return(m_nextItem); }
	void			setNextItem(CImageItem *item) { m_nextItem = item; }
	HBITMAP			getHbm() const { return(m_hbm); }
	DWORD			getFlags() const { return(m_dwFlags); }
	HDC				getDC() const { return(m_hdc); }
	const BLENDFUNCTION&	getBF() const
	{
		const BLENDFUNCTION &bf = m_bf;
		return(bf);
	}
	const TCHAR*      getName() const { return (m_szName); }
	TCHAR*            Read(const TCHAR *szFilename);
	void              Create(const TCHAR *szImageFile);
	void __fastcall   Render(const HDC hdc, const RECT *rc, bool fIgnoreGlyph) const;
	static void TSAPI	PreMultiply(HBITMAP hBitmap, int mode);
	static void TSAPI	SetBitmap32Alpha(HBITMAP hBitmap, BYTE bAlpha = 255);
	static void TSAPI	Colorize(HBITMAP hBitmap, BYTE dr, BYTE dg, BYTE db, BYTE alpha = 0);
	static HBITMAP TSAPI LoadPNG(const TCHAR *szFilename);

public:
	bool			m_fValid;									// verified item, indicates that all parameters are valid
private:
	TCHAR 		 	m_szName[40];								// everything wants a name, an image item doesn't need one though
	HBITMAP 		m_hbm;										// the bitmap handle
	BYTE    		m_bLeft, m_bRight, m_bTop, m_bBottom;      	// sizing margins for the outer 8 image parts
	BYTE    		m_alpha;									// constant alpha for the entire image, applied via m_bf. sums with perpixel alpha
	DWORD   		m_dwFlags;									// flags
	HDC     		m_hdc;										// *can* hold a pre-created hdc to speed up rendering
	HBITMAP 		m_hbmOld;									// old bitmap, needs to be selected into m_hdc before destroying it
	LONG    		m_inner_height, m_inner_width;				// dimensions of the inner image part
	LONG    		m_width, m_height;							// width and height of the image, in pixels
	BLENDFUNCTION 	m_bf;										// for AlphaBlend()
	BYTE    		m_bStretch;									// stretch mode (unused in tabSRMM
	HBRUSH  		m_fillBrush;								// brush to fill the inner part (faster) dwFlags & IMAGE_FILLSOLID must be set
	LONG    		m_glyphMetrics[4];							// these coordinates point into the glyph image (if IMAGE_GLYPH is set)
	CImageItem*		m_nextItem;									// next item in a set of image items (usually the skin set)
};

/**
 * Implements the skinning engine. There is only one instance of this class and
 * it always holds the currently loaded skin (if any).
 */
class CSkin
{
public:
	enum {
		AERO_EFFECT_NONE = 0,
		AERO_EFFECT_MILK = 1,
		AERO_EFFECT_CARBON = 2,
		AERO_EFFECT_SOLID = 3,
		AERO_EFFECT_WHITE = 4,
		AERO_EFFECT_CUSTOM = 5,
		AERO_EFFECT_LAST = 6
	};
	enum {
		AERO_EFFECT_AREA_MENUBAR = 0,
		AERO_EFFECT_AREA_STATUSBAR = 1,
		AERO_EFFECT_AREA_INFOPANEL = 2,
		AERO_EFFECT_AREA_TAB_ACTIVE = 3,
		AERO_EFFECT_AREA_TAB_HOVER = 4,
		AERO_EFFECT_AREA_TAB_NORMAL = 5,
		AERO_EFFECT_AREA_SIDEBAR_LEFT = 6,
		AERO_EFFECT_AREA_SIDEBAR_RIGHT = 7,
		AERO_EFFECT_AREA_TAB_TOP = 0x1000,
		AERO_EFFECT_AREA_TAB_BOTTOM = 0x2000
	};

	enum {
		DEFAULT_GLOW_SIZE = 10
	};

	/*
	 * avatar border types (skinned mode only)
	 */
	enum {
		AVBORDER_NONE = 0,
		AVBORDER_NORMAL = 1,
		AVBORDER_ROUNDED = 2
	};

	CSkin()
	{
		memset(this, 0, sizeof(CSkin));
		m_default_bf.SourceConstantAlpha = 255;
		m_default_bf.AlphaFormat = AC_SRC_ALPHA;
		m_default_bf.BlendOp = AC_SRC_OVER;
	}

	~CSkin()
	{
		Unload();
	}

	void  Init(bool fStartup = false);
	void  Load(void);
	void  Unload();
	void  UnloadAeroTabs();
	void  setFileName();
	void  ReadItem(const int id, const TCHAR *section);
	void  LoadItems();
	void  LoadIcon(const TCHAR *szSection, const TCHAR *name, HICON &hIcon);
	void  ReadImageItem(const TCHAR *szItemName);
	void  ReadButtonItem(const TCHAR *itemName) const;
	bool  haveGlyphItem() const { return(m_fHaveGlyph); }
	int   getNrIcons() const { return(m_nrSkinIcons); }
	DWORD getDwmColor() const { return(m_dwmColor); }

	const TIconDescW* getIconDesc(const int id) const { return(&m_skinIcons[id]); }
	/**
	 * get the glyph image item (a single PNG image, containing a number of textures
	 * for the skin.
	 *
	 * @return CImageItem&: reference to the glyph item. Cannot be
	 *  	   modified.
	 *
	 */
	const CImageItem*		getGlyphItem() const
	{
		return(m_fHaveGlyph ? &m_glyphItem : 0);
	}
	bool					warnToClose() const;
	COLORREF				getColorKey() const { return(m_ContainerColorKey); }

	void					setupAeroSkins();
	void 					extractSkinsAndLogo(bool fForceOverwrite = false) const;
	void					setupTabCloseBitmap(bool fDeleteOnly = false);

	/*
	 * static member functions
	 */
	static void TSAPI		SkinDrawBGFromDC(HWND hwndClient, HWND hwnd, RECT *rcClient, HDC hdcTarget);
	static void TSAPI		SkinDrawBG(HWND hwndClient, HWND hwnd, TContainerData *pContainer, RECT *rcClient, HDC hdcTarget);
	static void TSAPI		DrawDimmedIcon(HDC hdc, LONG left, LONG top, LONG dx, LONG dy, HICON hIcon, BYTE alpha);
	static DWORD __fastcall HexStringToLong(const TCHAR *szSource);
	static UINT TSAPI		DrawRichEditFrame(HWND hwnd, const TWindowData *mwdat, UINT skinID, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROC OldWndProc);
	static UINT TSAPI		NcCalcRichEditFrame(HWND hwnd, const TWindowData *mwdat, UINT skinID, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROC OldWndProc);
	static HBITMAP TSAPI 	CreateAeroCompatibleBitmap(const RECT &rc, HDC dc);
	static int TSAPI		RenderText(HDC hdc, HANDLE hTheme, const TCHAR *szText, RECT *rc, DWORD dtFlags, const int iGlowSize = DEFAULT_GLOW_SIZE, COLORREF clr = 0, bool fForceAero = false);
	static void TSAPI		MapClientToParent(HWND hwndClient, HWND hwndParent, RECT &rc);
	static void TSAPI		RenderToolbarBG(const TWindowData *dat, HDC hdc, const RECT &rcWindow);
	static HBITMAP TSAPI	ResizeBitmap(HBITMAP hBmpSrc, LONG width, LONG height, bool &mustFree);
	static void	TSAPI		ApplyAeroEffect(const HDC hdc, const RECT* rc, int iEffectArea, HANDLE hbp = 0);
	static void	TSAPI		setAeroEffect(const LRESULT effect);
	static void	TSAPI		initAeroEffect();
	static HANDLE TSAPI		InitiateBufferedPaint(const HDC hdcSrc, RECT& rc, HDC& hdcOut);
	static void TSAPI		FinalizeBufferedPaint(HANDLE hbp, RECT *rc);
	static bool __fastcall	DrawItem(const HDC hdc, const RECT *rc, const CSkinItem *item);
	static void	TSAPI		UpdateToolbarBG(TWindowData *dat, DWORD dwRdwOptFlags = 0);
	static void TSAPI		FillBack(const HDC hdc, RECT* rc);

public:
	static bool		m_DisableScrollbars, m_bClipBorder;
	static char     m_SkinnedFrame_left, m_SkinnedFrame_right, m_SkinnedFrame_bottom, m_SkinnedFrame_caption;
	static char     m_realSkinnedFrame_left, m_realSkinnedFrame_right, m_realSkinnedFrame_bottom, m_realSkinnedFrame_caption;
	static HPEN     m_SkinLightShadowPen, m_SkinDarkShadowPen;
	static int 		m_titleBarLeftOff, m_titleButtonTopOff, m_captionOffset, m_captionPadding,
					m_titleBarRightOff, m_sidebarTopOffset, m_sidebarBottomOffset, m_bRoundedCorner;
	static SIZE		m_titleBarButtonSize;
	static int		m_bAvatarBorderType;
	static COLORREF m_avatarBorderClr, m_tmp_tb_low, m_tmp_tb_high;
	static COLORREF m_sideBarContainerBG;
	static COLORREF m_ContainerColorKey, m_DefaultFontColor;
	static HBRUSH 	m_ContainerColorKeyBrush, m_MenuBGBrush;
	static bool		m_skinEnabled;
	static bool		m_frameSkins;
	static HICON	m_closeIcon, m_minIcon, m_maxIcon;
	static BLENDFUNCTION m_default_bf;										// general purpose bf, dynamically modified when needed

	/*
	 * cached bitmap for tab close button
	 */

	static HBITMAP	m_tabCloseBitmap, m_tabCloseOldBitmap;
	static HDC		m_tabCloseHDC;

	/*
	 * controls the aero effect. Set by initAeroEffect()
	 */

	static UINT			m_aeroEffect;										// effect id, initAeroEffect() is using it to set
																			// the parameters below.
	static AeroEffect	m_aeroEffects[AERO_EFFECT_LAST];
	static AeroEffect	m_currentAeroEffect;
	static AeroEffect*	m_pCurrentAeroEffect;
	static DWORD		m_glowSize;
	static HBRUSH		m_BrushBack, m_BrushFill;

	static COLORREF		m_dwmColorRGB;

	static CImageItem *m_switchBarItem,	*m_tabTop, *m_tabBottom, *m_tabGlowTop,	*m_tabGlowBottom;
	static bool			m_fAeroSkinsValid;

private:
	TCHAR			m_tszFileName[MAX_PATH];				// full path and filename of the currently loaded skin
	CSkinItem*		m_SkinItems;
	CImageItem*		m_ImageItems;							// the list of image item objects
	CImageItem		m_glyphItem;

	bool			m_fLoadOnStartup;						// load the skin on plugin initialization.
	bool			m_fHaveGlyph;
	void 			SkinCalcFrameWidth();
	TIconDescW		*m_skinIcons;
	int				m_nrSkinIcons;
	DWORD			m_dwmColor;

private:
	static	void TSAPI AeroEffectCallback_Milk(const HDC hdc, const RECT *rc, int iEffectArea);
	static	void TSAPI AeroEffectCallback_Carbon(const HDC hdc, const RECT *rc, int iEffectArea);
	static	void TSAPI AeroEffectCallback_Solid(const HDC hdc, const RECT *rc, int iEffectArea);
};

/*
 * window data for the tab control window class
 */

struct TabControlData
{
	BOOL    m_VisualStyles;
	HWND    hwnd;
	DWORD   dwStyle;
	DWORD   cx, cy;
	HANDLE  hTheme, hThemeButton, hbp;
	BYTE    m_xpad;
	TContainerData *pContainer;
	BOOL    bDragging;
	int     iBeginIndex;
	int		iHoveredCloseIcon;
	HWND    hwndDrag;
	TWindowData *dragDat;
	HIMAGELIST himlDrag;
	BOOL    bRefreshWithoutClip;
	BOOL    fSavePos;
	BOOL    fTipActive;
	BOOL	fAeroTabs;
	BOOL	fCloseButton;
	TWindowData* helperDat;				// points to the client data of the active tab
	CImageItem*			helperItem, *helperGlowItem;				// aero ui, holding the skin image for the tabs
};

extern CSkin *Skin;

#endif /* __THEMES_H */
