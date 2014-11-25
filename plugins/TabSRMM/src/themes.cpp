/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * Implements the skinning engine and most parts of the aero support in
 * tabSRMM 3.x+
 *
 */

#include "commonheaders.h"

static SKINDESC my_default_skin[] = {
	IDR_SKIN_AERO, _T("tabskin_aero.png"),
	IDR_SKIN_AERO_GLOW, _T("tabskin_aero_glow.png"),
	IDR_SKIN_AERO_SWITCHBAR, _T("tabskin_aero_button.png"),
	IDR_SKIN_LOGO, _T("unknown.png")
};

CSkin* Skin = 0;


static void __inline gradientVertical(UCHAR *ubRedFinal, UCHAR *ubGreenFinal, UCHAR *ubBlueFinal,
									  ULONG ulBitmapHeight, UCHAR ubRed, UCHAR ubGreen, UCHAR ubBlue, UCHAR ubRed2,
									  UCHAR ubGreen2, UCHAR ubBlue2, DWORD FLG_GRADIENT, BOOL transparent, UINT32 y, UCHAR *ubAlpha);

static void __inline gradientHorizontal(UCHAR *ubRedFinal, UCHAR *ubGreenFinal, UCHAR *ubBlueFinal,
										ULONG ulBitmapWidth, UCHAR ubRed, UCHAR ubGreen, UCHAR ubBlue,  UCHAR ubRed2,
										UCHAR ubGreen2, UCHAR ubBlue2, DWORD FLG_GRADIENT, BOOL transparent, UINT32 x, UCHAR *ubAlpha);


UINT 		nextButtonID;
ButtonSet 	g_ButtonSet = {0};

#define   	NR_MAXSKINICONS 100

/*
 * initialize static class data
*/

int 		  CSkin::m_bAvatarBorderType = 0;
COLORREF	  CSkin::m_avatarBorderClr = 0;
COLORREF	  CSkin::m_tmp_tb_low = 0, CSkin::m_tmp_tb_high = 0;
COLORREF	  CSkin::m_sideBarContainerBG;
BLENDFUNCTION CSkin::m_default_bf = {0};				// a global blend function, used in various places
														// when you use it, reset SourceConstantAlpha to 255 and
														// the blending mode to AC_SRC_ALPHA.

bool 			CSkin::m_bClipBorder = false, CSkin::m_DisableScrollbars = false,
				CSkin::m_skinEnabled = false, CSkin::m_frameSkins = false;

char 			CSkin::m_SkinnedFrame_left = 0, CSkin::m_SkinnedFrame_right = 0,
				CSkin::m_SkinnedFrame_bottom = 0, CSkin::m_SkinnedFrame_caption = 0;

char 			CSkin::m_realSkinnedFrame_left = 0;
char 			CSkin::m_realSkinnedFrame_right = 0;
char 			CSkin::m_realSkinnedFrame_bottom = 0;
char 			CSkin::m_realSkinnedFrame_caption = 0;

int 			CSkin::m_titleBarLeftOff = 0, CSkin::m_titleButtonTopOff = 0, CSkin::m_captionOffset = 0, CSkin::m_captionPadding = 0,
				CSkin::m_titleBarRightOff = 0, CSkin::m_sidebarTopOffset = 0, CSkin::m_sidebarBottomOffset = 0, CSkin::m_bRoundedCorner = 0;

CImageItem* 	CSkin::m_switchBarItem = 0,
			   *CSkin::m_tabTop = 0,
			   *CSkin::m_tabBottom = 0,
			   *CSkin::m_tabGlowTop = 0,
			   *CSkin::m_tabGlowBottom = 0;

bool 			CSkin::m_fAeroSkinsValid = false;

SIZE 			CSkin::m_titleBarButtonSize = {0};

COLORREF 		CSkin::m_ContainerColorKey = 0, CSkin::m_dwmColorRGB = 0, CSkin::m_DefaultFontColor = 0;
HBRUSH 	 		CSkin::m_ContainerColorKeyBrush = 0, CSkin::m_MenuBGBrush = 0;

HPEN 			CSkin::m_SkinLightShadowPen = 0, CSkin::m_SkinDarkShadowPen = 0;

HICON			CSkin::m_closeIcon = 0, CSkin::m_maxIcon = 0, CSkin::m_minIcon = 0;

UINT 			CSkin::m_aeroEffect = 0;
DWORD 			CSkin::m_glowSize = 0;
HBRUSH  		CSkin::m_BrushBack = 0, CSkin::m_BrushFill = 0;

HBITMAP 		CSkin::m_tabCloseBitmap = 0, CSkin::m_tabCloseOldBitmap = 0;
HDC				CSkin::m_tabCloseHDC = 0;

/*
 * aero effects
 */

AeroEffect  	CSkin::m_currentAeroEffect;
AeroEffect* 	CSkin::m_pCurrentAeroEffect = 0;

AeroEffect  CSkin::m_aeroEffects[AERO_EFFECT_LAST] = {
	{
		LPGENT("No effect"), 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0
	},
	{
		LPGENT("Milky Glass"),
		0xf5f5f5, 									/* base color */
		0xaaaaaa,									/* gradient color */
		70,											/* base alpha */
		0,											/* final alpha */
		CORNER_ALL,									/* corner type */
		GRADIENT_TB + 1,							/* gradient type */
		8,											/* corner radius */
		8,											/* glow size (0 means no glowing text, colors can be used) */
		0,											/* background color (black = transparency) */
		0xf0f0f0,									/* toolbar first color (if 0, use custom gradient color) */
		0, 											/* toolbar 2nd gradient color (0 = use aero theme color, -1 = use custom gradient color  */
		AeroEffectCallback_Milk						/* callback function to render the effect */
	},
	{
		LPGENT("Carbon"),
		0xf0f0f0,
		0x000000,
		75,
		0,
		CORNER_ALL,
		GRADIENT_TB + 1,
		6,
		8,
		0,
		0xf0f0f0,
		0,
		AeroEffectCallback_Carbon
	},
	{
		LPGENT("Semi transparent, custom colors"),
		0xffffff,
		0x444444,
		60,
		0,
		CORNER_ALL,
		GRADIENT_TB + 1,
		6,
		0,
		0x0,
		0xf0f0f0,
		0,
		AeroEffectCallback_Solid
	},
	{
		LPGENT("Silver shadow"),
		0xffffff,
		0xa0a0a0,
		80,
		220,
		0,
		GRADIENT_TB + 1,
		1,
		0,
		0xc0c0c0,
		0xf0f0f0,
		0x707070,
		AeroEffectCallback_Solid
	},
	{
		LPGENT("Custom (use own gradient colors)"),
		0xffffff,
		0xa0a0a0,
		80,
		220,
		0,
		GRADIENT_TB + 1,
		1,
		0,
		0xc0c0c0,
		-1,
		-1,
		AeroEffectCallback_Solid
	}
};

/*
 * definition of the availbale skin items
 */

CSkinItem SkinItems[] = {
	{_T("Container"), "TSKIN_Container", ID_EXTBKCONTAINER,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Toolbar"), "TSKIN_Container", ID_EXTBKBUTTONBAR,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("{-}Buttonpressed"), "TSKIN_BUTTONSPRESSED", ID_EXTBKBUTTONSPRESSED,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Buttonnotpressed"), "TSKIN_BUTTONSNPRESSED", ID_EXTBKBUTTONSNPRESSED,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Buttonmouseover"), "TSKIN_BUTTONSMOUSEOVER", ID_EXTBKBUTTONSMOUSEOVER,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Infopanelfield"), "TSKIN_INFOPANELFIELD", ID_EXTBKINFOPANEL,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Titlebutton"), "TSKIN_TITLEBUTTON", ID_EXTBKTITLEBUTTON,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Titlebuttonmouseover"), "TSKIN_TITLEBUTTONHOVER", ID_EXTBKTITLEBUTTONMOUSEOVER,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Titlebuttonpressed"), "TSKIN_TITLEBUTTONPRESSED", ID_EXTBKTITLEBUTTONPRESSED,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabpage"), "TSKIN_TABPAGE", ID_EXTBKTABPAGE,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabitem"), "TSKIN_TABITEM", ID_EXTBKTABITEM,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabitem_active"), "TSKIN_TABITEMACTIVE", ID_EXTBKTABITEMACTIVE,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabitem_bottom"), "TSKIN_TABITEMBOTTOM", ID_EXTBKTABITEMBOTTOM,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabitem_active_bottom"), "TSKIN_TABITEMACTIVEBOTTOM", ID_EXTBKTABITEMACTIVEBOTTOM,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Frame"), "TSKIN_FRAME", ID_EXTBKFRAME,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("MessageLog"), "TSKIN_MLOG", ID_EXTBKHISTORY,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("InputArea"), "TSKIN_INPUT", ID_EXTBKINPUTAREA,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("FrameInactive"), "TSKIN_FRAMEINACTIVE", ID_EXTBKFRAMEINACTIVE,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabitem_hottrack"), "TSKIN_TABITEMHOTTRACK", ID_EXTBKTABITEMHOTTRACK,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Tabitem_hottrack_bottom"), "TSKIN_TABITEMHOTTRACKBOTTOM", ID_EXTBKTABITEMHOTTRACKBOTTOM,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Statusbarpanel"), "TSKIN_STATUSBARPANEL", ID_EXTBKSTATUSBARPANEL,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Statusbar"), "TSKIN_STATUSBAR", ID_EXTBKSTATUSBAR,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("Userlist"), "TSKIN_USERLIST", ID_EXTBKUSERLIST,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {_T("InfoPanelBackground"), "TSKIN_INFOPANELBG", ID_EXTBKINFOPANELBG,
		8, CLCDEFAULT_CORNER,
		0xf0f0f0, 0x42b1ff, 1, CLCDEFAULT_TEXTCOLOR, 40, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE, 0, 0
	}, {_T("Sidebar Background"), "TSKIN_SIDEBARBG", ID_EXTBKSIDEBARBG,
		CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
		0xb2e1ff, 0xb2e1ff, 1, CLCDEFAULT_TEXTCOLOR, 40, CLCDEFAULT_MRGN_LEFT,
		CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE, 0, 0
	}
};

static CSkinItem _defInfoPanel = {
	_T("InfoPanelBackground"), "TSKIN_INFOPANELBG", ID_EXTBKINFOPANELBG,
	8, CLCDEFAULT_CORNER,
	0xf0f0f0, 0x62caff, 0, CLCDEFAULT_TEXTCOLOR, 255, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE, 0, 0
};

static BYTE __inline percent_to_byte(UINT32 percent)
{
	return(BYTE)((FLOAT)(((FLOAT) percent) / 100) * 255);
}

static COLORREF __inline revcolref(COLORREF colref)
{
	return RGB(GetBValue(colref), GetGValue(colref), GetRValue(colref));
}

static DWORD __inline argb_from_cola(COLORREF col, UINT32 alpha)
{
	return((BYTE) percent_to_byte(alpha) << 24 | col);
}


void TSAPI  DrawAlpha(HDC hDC, PRECT rc, DWORD clr_base, int alpha, DWORD clr_dest, BYTE clr_dest_trans, BYTE bGradient,
	BYTE bCorner, DWORD dwRadius, CImageItem *imageItem)
{
	HBRUSH BrMask;
	HBRUSH holdbrush;
	HDC hdc;
	BLENDFUNCTION bf;
	HBITMAP hbitmap;
	HBITMAP holdbitmap;
	BITMAPINFO bmi;
	VOID *pvBits;
	UINT32 x, y;
	ULONG ulBitmapWidth, ulBitmapHeight;
	UCHAR ubAlpha = 0xFF;
	UCHAR ubRedFinal = 0xFF;
	UCHAR ubGreenFinal = 0xFF;
	UCHAR ubBlueFinal = 0xFF;
	UCHAR ubRed;
	UCHAR ubGreen;
	UCHAR ubBlue;
	UCHAR ubRed2;
	UCHAR ubGreen2;
	UCHAR ubBlue2;
	FLOAT fAlphaFactor;

	int realx;

	if (rc == NULL)
		return;
	LONG realHeight = (rc->bottom - rc->top);
	LONG realWidth = (rc->right - rc->left);
	LONG realHeightHalf = realHeight >> 1;

	if (imageItem) {
		imageItem->Render(hDC, rc, false);
		return;
	}

	if (rc->right < rc->left || rc->bottom < rc->top || (realHeight <= 0) || (realWidth <= 0))
		return;

	/*
	 * use GDI fast gradient drawing when no corner radi exist
	 */

	if (bCorner == 0 && dwRadius == 0) {
		GRADIENT_RECT grect;
		TRIVERTEX tvtx[2];
		int orig = 1, dest = 0;

		if (bGradient & GRADIENT_LR || bGradient & GRADIENT_TB) {
			orig = 0;
			dest = 1;
		}

		tvtx[0].x = rc->left;
		tvtx[0].y = rc->top;
		tvtx[1].x = rc->right;
		tvtx[1].y = rc->bottom;

		tvtx[orig].Red = (COLOR16)GetRValue(clr_base) << 8;
		tvtx[orig].Blue = (COLOR16)GetBValue(clr_base) << 8;
		tvtx[orig].Green = (COLOR16)GetGValue(clr_base) << 8;
		tvtx[orig].Alpha = (COLOR16) alpha << 8;

		tvtx[dest].Red = (COLOR16)GetRValue(clr_dest) << 8;
		tvtx[dest].Blue = (COLOR16)GetBValue(clr_dest) << 8;
		tvtx[dest].Green = (COLOR16)GetGValue(clr_dest) << 8;
		tvtx[dest].Alpha = (COLOR16) alpha << 8;

		grect.UpperLeft = 0;
		grect.LowerRight = 1;

		GdiGradientFill(hDC, tvtx, 2, &grect, 1, (bGradient & GRADIENT_TB || bGradient & GRADIENT_BT) ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
		return;
	}

	hdc = CreateCompatibleDC(hDC);
	if (!hdc)
		return;

	ZeroMemory(&bmi, sizeof(BITMAPINFO));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	if (bGradient & GRADIENT_ACTIVE && (bGradient & GRADIENT_LR || bGradient & GRADIENT_RL)) {
		bmi.bmiHeader.biWidth = ulBitmapWidth = realWidth;
		bmi.bmiHeader.biHeight = ulBitmapHeight = 1;
	} else if (bGradient & GRADIENT_ACTIVE && (bGradient & GRADIENT_TB || bGradient & GRADIENT_BT)) {
		bmi.bmiHeader.biWidth = ulBitmapWidth = 1;
		bmi.bmiHeader.biHeight = ulBitmapHeight = realHeight;
	} else {
		bmi.bmiHeader.biWidth = ulBitmapWidth = 1;
		bmi.bmiHeader.biHeight = ulBitmapHeight = 1;
	}

	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

	if (ulBitmapWidth <= 0 || ulBitmapHeight <= 0)
		return;

	hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
	if (hbitmap == NULL || pvBits == NULL) {
		DeleteDC(hdc);
		return;
	}

	holdbitmap = (HBITMAP)SelectObject(hdc, hbitmap);

	// convert basecolor to RGB and then merge alpha so its ARGB
	clr_base = argb_from_cola(revcolref(clr_base), alpha);
	clr_dest = argb_from_cola(revcolref(clr_dest), alpha);

	ubRed = (UCHAR)(clr_base >> 16);
	ubGreen = (UCHAR)(clr_base >> 8);
	ubBlue = (UCHAR) clr_base;

	ubRed2 = (UCHAR)(clr_dest >> 16);
	ubGreen2 = (UCHAR)(clr_dest >> 8);
	ubBlue2 = (UCHAR) clr_dest;

	//DRAW BASE - make corner space 100% transparent
	for (y = 0; y < ulBitmapHeight; y++) {
		for (x = 0 ; x < ulBitmapWidth ; x++) {
			if (bGradient & GRADIENT_ACTIVE) {
				if (bGradient & GRADIENT_LR || bGradient & GRADIENT_RL) {
					realx = x + realHeightHalf;
					realx = (ULONG) realx > ulBitmapWidth ? ulBitmapWidth : realx;
					gradientHorizontal(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapWidth, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, bGradient, clr_dest_trans, realx, &ubAlpha);
				} else if (bGradient & GRADIENT_TB || bGradient & GRADIENT_BT)
					gradientVertical(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapHeight, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, bGradient, clr_dest_trans, y, &ubAlpha);

				fAlphaFactor = (float) ubAlpha / (float) 0xff;
				((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR)(ubRedFinal * fAlphaFactor) << 16) | ((UCHAR)(ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR)(ubBlueFinal * fAlphaFactor));
			} else {
				ubAlpha = percent_to_byte(alpha);
				ubRedFinal = ubRed;
				ubGreenFinal = ubGreen;
				ubBlueFinal = ubBlue;
				fAlphaFactor = (float) ubAlpha / (float) 0xff;

				((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR)(ubRedFinal * fAlphaFactor) << 16) | ((UCHAR)(ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR)(ubBlueFinal * fAlphaFactor));
			}
		}
	}
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = (UCHAR)(clr_base >> 24);
	bf.AlphaFormat = AC_SRC_ALPHA; // so it will use our specified alpha value

	GdiAlphaBlend(hDC, rc->left + realHeightHalf, rc->top, (realWidth - realHeightHalf * 2), realHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf);

	SelectObject(hdc, holdbitmap);
	DeleteObject(hbitmap);

	// corners
	BrMask = CreateSolidBrush(RGB(0xFF, 0x00, 0xFF));
	{
		bmi.bmiHeader.biWidth = ulBitmapWidth = realHeightHalf;
		bmi.bmiHeader.biHeight = ulBitmapHeight = realHeight;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		if (ulBitmapWidth <= 0 || ulBitmapHeight <= 0) {
			DeleteDC(hdc);
			DeleteObject(BrMask);
			return;
		}

		// TL+BL CORNER
		hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

		if (hbitmap == 0 || pvBits == NULL)  {
			DeleteObject(BrMask);
			DeleteDC(hdc);
			return;
		}

		holdbrush = (HBRUSH)SelectObject(hdc, BrMask);
		holdbitmap = (HBITMAP)SelectObject(hdc, hbitmap);
		RoundRect(hdc, -1, -1, ulBitmapWidth * 2 + 1, (realHeight + 1), dwRadius, dwRadius);

		for (y = 0; y < ulBitmapHeight; y++) {
			for (x = 0; x < ulBitmapWidth; x++) {
				if (((((UINT32 *) pvBits)[x + y * ulBitmapWidth]) << 8) == 0xFF00FF00 || (y < ulBitmapHeight >> 1 && !(bCorner & CORNER_BL && bCorner & CORNER_ACTIVE)) || (y > ulBitmapHeight >> 2 && !(bCorner & CORNER_TL && bCorner & CORNER_ACTIVE))) {
					if (bGradient & GRADIENT_ACTIVE) {
						if (bGradient & GRADIENT_LR || bGradient & GRADIENT_RL)
							gradientHorizontal(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, realWidth, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, bGradient, clr_dest_trans, x, &ubAlpha);
						else if (bGradient & GRADIENT_TB || bGradient & GRADIENT_BT)
							gradientVertical(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapHeight, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, bGradient, clr_dest_trans, y, &ubAlpha);

						fAlphaFactor = (float) ubAlpha / (float) 0xff;
						((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR)(ubRedFinal * fAlphaFactor) << 16) | ((UCHAR)(ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR)(ubBlueFinal * fAlphaFactor));
					} else {
						ubAlpha = percent_to_byte(alpha);
						ubRedFinal = ubRed;
						ubGreenFinal = ubGreen;
						ubBlueFinal = ubBlue;
						fAlphaFactor = (float) ubAlpha / (float) 0xff;

						((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR)(ubRedFinal * fAlphaFactor) << 16) | ((UCHAR)(ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR)(ubBlueFinal * fAlphaFactor));
					}
				}
			}
		}
		GdiAlphaBlend(hDC, rc->left, rc->top, ulBitmapWidth, ulBitmapHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf);
		SelectObject(hdc, holdbitmap);
		DeleteObject(hbitmap);

		// TR+BR CORNER
		hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

		//SelectObject(hdc, BrMask); // already BrMask?
		holdbitmap = (HBITMAP)SelectObject(hdc, hbitmap);
		RoundRect(hdc, -1 - ulBitmapWidth, -1, ulBitmapWidth + 1, (realHeight + 1), dwRadius, dwRadius);

		for (y = 0; y < ulBitmapHeight; y++) {
			for (x = 0; x < ulBitmapWidth; x++) {
				if (((((UINT32 *) pvBits)[x + y * ulBitmapWidth]) << 8) == 0xFF00FF00 || (y < ulBitmapHeight >> 1 && !(bCorner & CORNER_BR && bCorner & CORNER_ACTIVE)) || (y > ulBitmapHeight >> 1 && !(bCorner & CORNER_TR && bCorner & CORNER_ACTIVE))) {
					if (bGradient & GRADIENT_ACTIVE) {
						if (bGradient & GRADIENT_LR || bGradient & GRADIENT_RL) {
							realx = x + realWidth;
							realx = realx > realWidth ? realWidth : realx;
							gradientHorizontal(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, realWidth, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, bGradient, clr_dest_trans, realx, &ubAlpha);
						} else if (bGradient & GRADIENT_TB || bGradient & GRADIENT_BT)
							gradientVertical(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapHeight, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, bGradient, clr_dest_trans, y, &ubAlpha);

						fAlphaFactor = (float) ubAlpha / (float) 0xff;
						((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR)(ubRedFinal * fAlphaFactor) << 16) | ((UCHAR)(ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR)(ubBlueFinal * fAlphaFactor));
					} else {
						ubAlpha = percent_to_byte(alpha);
						ubRedFinal = ubRed;
						ubGreenFinal = ubGreen;
						ubBlueFinal = ubBlue;
						fAlphaFactor = (float) ubAlpha / (float) 0xff;

						((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR)(ubRedFinal * fAlphaFactor) << 16) | ((UCHAR)(ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR)(ubBlueFinal * fAlphaFactor));
					}
				}
			}
		}
		GdiAlphaBlend(hDC, rc->right - realHeightHalf, rc->top, ulBitmapWidth, ulBitmapHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf);
	}
	SelectObject(hdc, holdbitmap);
	DeleteObject(hbitmap);
	SelectObject(hdc, holdbrush);
	DeleteObject(BrMask);
	DeleteDC(hdc);
}

void __inline gradientHorizontal(UCHAR *ubRedFinal, UCHAR *ubGreenFinal, UCHAR *ubBlueFinal, ULONG ulBitmapWidth, UCHAR ubRed, UCHAR ubGreen, UCHAR ubBlue, UCHAR ubRed2, UCHAR ubGreen2, UCHAR ubBlue2, DWORD FLG_GRADIENT, BOOL transparent, UINT32 x, UCHAR *ubAlpha)
{
	FLOAT fSolidMulti, fInvSolidMulti;

	// solid to transparent
	if (transparent) {
		*ubAlpha = (UCHAR)((float) x / (float) ulBitmapWidth * 255);
		*ubAlpha = FLG_GRADIENT & GRADIENT_LR ? 0xFF - (*ubAlpha) : (*ubAlpha);
		*ubRedFinal = ubRed;
		*ubGreenFinal = ubGreen;
		*ubBlueFinal = ubBlue;
	} else { // solid to solid2
		if (FLG_GRADIENT & GRADIENT_LR) {
			fSolidMulti = ((float) x / (float) ulBitmapWidth);
			fInvSolidMulti = 1 - fSolidMulti;
		} else {
			fInvSolidMulti = ((float) x / (float) ulBitmapWidth);
			fSolidMulti = 1 - fInvSolidMulti;
		}

		*ubRedFinal = (UCHAR)((((float) ubRed * (float) fInvSolidMulti)) + (((float) ubRed2 * (float) fSolidMulti)));
		*ubGreenFinal = (UCHAR)((UCHAR)(((float) ubGreen * (float) fInvSolidMulti)) + (((float) ubGreen2 * (float) fSolidMulti)));
		*ubBlueFinal = (UCHAR)((((float) ubBlue * (float) fInvSolidMulti)) + (UCHAR)(((float) ubBlue2 * (float) fSolidMulti)));

		*ubAlpha = 0xFF;
	}
}

void __inline gradientVertical(UCHAR *ubRedFinal, UCHAR *ubGreenFinal, UCHAR *ubBlueFinal, ULONG ulBitmapHeight, UCHAR ubRed, UCHAR ubGreen, UCHAR ubBlue, UCHAR ubRed2, UCHAR ubGreen2, UCHAR ubBlue2, DWORD FLG_GRADIENT, BOOL transparent, UINT32 y, UCHAR *ubAlpha)
{
	FLOAT fSolidMulti, fInvSolidMulti;

	// solid to transparent
	if (transparent) {
		*ubAlpha = (UCHAR)((float) y / (float) ulBitmapHeight * 255);
		*ubAlpha = FLG_GRADIENT & GRADIENT_BT ? 0xFF - *ubAlpha : *ubAlpha;
		*ubRedFinal = ubRed;
		*ubGreenFinal = ubGreen;
		*ubBlueFinal = ubBlue;
	} else { // solid to solid2
		if (FLG_GRADIENT & GRADIENT_BT) {
			fSolidMulti = ((float) y / (float) ulBitmapHeight);
			fInvSolidMulti = 1 - fSolidMulti;
		} else {
			fInvSolidMulti = ((float) y / (float) ulBitmapHeight);
			fSolidMulti = 1 - fInvSolidMulti;
		}

		*ubRedFinal = (UCHAR)((((float) ubRed * (float) fInvSolidMulti)) + (((float) ubRed2 * (float) fSolidMulti)));
		*ubGreenFinal = (UCHAR)((((float) ubGreen * (float) fInvSolidMulti)) + (((float) ubGreen2 * (float) fSolidMulti)));
		*ubBlueFinal = (UCHAR)(((float) ubBlue * (float) fInvSolidMulti)) + (UCHAR)(((float) ubBlue2 * (float) fSolidMulti));

		*ubAlpha = 0xFF;
	}
}

/**
 * Renders the image item to the given target device context and rectangle
 *
 * @param hdc    HDC: target device context
 * @param rc     RECT *: client rectangle inside the target DC.
 * @param fIgnoreGlyph: bool: will ignore any glyph item. Set it to true when
 * 						using this function from _outside_ a skin
 */
void __fastcall CImageItem::Render(const HDC hdc, const RECT *rc, bool fIgnoreGlyph) const
{
	BYTE l = m_bLeft, r = m_bRight, t = m_bTop, b = m_bBottom;
	LONG width = rc->right - rc->left;
	LONG height = rc->bottom - rc->top;
	BOOL isGlyph = ((m_dwFlags & IMAGE_GLYPH) && Skin->haveGlyphItem());
	BOOL fCleanUp = TRUE;
	HDC hdcSrc = 0;
	HBITMAP hbmOld;
	LONG srcOrigX = isGlyph ? m_glyphMetrics[0] : 0;
	LONG srcOrigY = isGlyph ? m_glyphMetrics[1] : 0;

	if (m_hdc == 0) {
		hdcSrc = CreateCompatibleDC(hdc);
		hbmOld = (HBITMAP)SelectObject(hdcSrc, isGlyph ? Skin->getGlyphItem()->getHbm() : m_hbm);
	} else {
		if (fIgnoreGlyph)
			hdcSrc = m_hdc;
		else
			hdcSrc = isGlyph ? Skin->getGlyphItem()->getDC() : m_hdc;
		fCleanUp = FALSE;
	}

	if (m_dwFlags & IMAGE_FLAG_DIVIDED) {
		// top 3 items

		GdiAlphaBlend(hdc, rc->left, rc->top, l, t, hdcSrc, srcOrigX, srcOrigY, l, t, m_bf);
		GdiAlphaBlend(hdc, rc->left + l, rc->top, width - l - r, t, hdcSrc, srcOrigX + l, srcOrigY, m_inner_width, t, m_bf);
		GdiAlphaBlend(hdc, rc->right - r, rc->top, r, t, hdcSrc, srcOrigX + (m_width - r), srcOrigY, r, t, m_bf);

		// middle 3 items

		GdiAlphaBlend(hdc, rc->left, rc->top + t, l, height - t - b, hdcSrc, srcOrigX, srcOrigY + t, l, m_inner_height, m_bf);

		if ((m_dwFlags & IMAGE_FILLSOLID) && m_fillBrush) {
			RECT rcFill;
			rcFill.left = rc->left + l;
			rcFill.top = rc->top + t;
			rcFill.right = rc->right - r;
			rcFill.bottom = rc->bottom - b;
			FillRect(hdc, &rcFill, m_fillBrush);
		} else
			GdiAlphaBlend(hdc, rc->left + l, rc->top + t, width - l - r, height - t - b, hdcSrc, srcOrigX + l, srcOrigY + t, m_inner_width, m_inner_height, m_bf);

		GdiAlphaBlend(hdc, rc->right - r, rc->top + t, r, height - t - b, hdcSrc, srcOrigX + (m_width - r), srcOrigY + t, r, m_inner_height, m_bf);

		// bottom 3 items

		GdiAlphaBlend(hdc, rc->left, rc->bottom - b, l, b, hdcSrc, srcOrigX, srcOrigY + (m_height - b), l, b, m_bf);
		GdiAlphaBlend(hdc, rc->left + l, rc->bottom - b, width - l - r, b, hdcSrc, srcOrigX + l, srcOrigY + (m_height - b), m_inner_width, b, m_bf);
		GdiAlphaBlend(hdc, rc->right - r, rc->bottom - b, r, b, hdcSrc, srcOrigX + (m_width - r), srcOrigY + (m_height - b), r, b, m_bf);
	}
	else switch (m_bStretch) {
	case IMAGE_STRETCH_H:
		// tile image vertically, stretch to width
		{
			LONG top = rc->top;

			do {
				if (top + m_height <= rc->bottom) {
					GdiAlphaBlend(hdc, rc->left, top, width, m_height, hdcSrc, srcOrigX, srcOrigY, m_width, m_height, m_bf);
					top += m_height;
				} else {
					GdiAlphaBlend(hdc, rc->left, top, width, rc->bottom - top, hdcSrc, srcOrigX, srcOrigY, m_width, rc->bottom - top, m_bf);
					break;
				}
			} while (true);
			break;
		}
	case IMAGE_STRETCH_V:
		// tile horizontally, stretch to height
		{
			LONG left = rc->left;

			do {
				if (left + m_width <= rc->right) {
					GdiAlphaBlend(hdc, left, rc->top, m_width, height, hdcSrc, srcOrigX, srcOrigY, m_width, m_height, m_bf);
					left += m_width;
				} else {
					GdiAlphaBlend(hdc, left, rc->top, rc->right - left, height, hdcSrc, srcOrigX, srcOrigY, rc->right - left, m_height, m_bf);
					break;
				}
			} while (true);
			break;
		}
	case IMAGE_STRETCH_B:
		// stretch the image in both directions...
		GdiAlphaBlend(hdc, rc->left, rc->top, width, height, hdcSrc, srcOrigX, srcOrigY, m_width, m_height, m_bf);
		break;
	}

	if (fCleanUp) {
		SelectObject(hdcSrc, hbmOld);
		DeleteDC(hdcSrc);
	}
}

static CSkinItem StatusItem_Default = {
	_T("Container"), "EXBK_Offline", ID_EXTBKCONTAINER,
	CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
};

static struct {
	TCHAR *szIniKey, *szIniName;
	char *szSetting;
	unsigned int size;
	int defaultval;
} _tagSettings[] = {
	_T("Global"), _T("SbarHeight"), "S_sbarheight", 1, 22,
	_T("ClientArea"), _T("Left"), "S_tborder_outer_left", 1, 0,
	_T("ClientArea"), _T("Right"), "S_tborder_outer_right", 1, 0,
	_T("ClientArea"), _T("Top"), "S_tborder_outer_top", 1, 0,
	_T("ClientArea"), _T("Bottom"), "S_tborder_outer_bottom", 1, 0,
	_T("ClientArea"), _T("Inner"), "S_tborder", 1, 0,
	_T("Global"), _T("TabTextNormal"), "S_tab_txt_normal", 5, 0,
	_T("Global"), _T("TabTextActive"), "S_tab_txt_active", 5, 0,
	_T("Global"), _T("TabTextUnread"), "S_tab_txt_unread", 5, 0,
	_T("Global"), _T("TabTextHottrack"), "S_tab_txt_hottrack", 5, 0,
	NULL, NULL, NULL, 0, 0
};

void CImageItem::Create(const TCHAR *szImageFile)
{
	HBITMAP hbm = LoadPNG(szImageFile);
	BITMAP bm;

	m_hdc = 0;
	m_hbmOld = 0;
	m_hbm = 0;

	if (hbm) {
		m_hbm = hbm;
		m_bf.BlendFlags = 0;
		m_bf.BlendOp = AC_SRC_OVER;
		m_bf.AlphaFormat = 0;

		GetObject(hbm, sizeof(bm), &bm);
		if (bm.bmBitsPixel == 32 && m_dwFlags & IMAGE_PERPIXEL_ALPHA) {
			CImageItem::PreMultiply(m_hbm, 1);
			m_bf.AlphaFormat = AC_SRC_ALPHA;
		}
		m_width = bm.bmWidth;
		m_height = bm.bmHeight;
		m_inner_height = m_height - m_bTop - m_bBottom;
		m_inner_width = m_width - m_bLeft - m_bRight;
		if (m_bTop && m_bBottom && m_bLeft && m_bRight) {
			m_dwFlags |= IMAGE_FLAG_DIVIDED;
			if (m_inner_height <= 0 || m_inner_width <= 0) {
				DeleteObject(hbm);
				m_hbm = 0;
				return;
			}
		}
	}
}

/**
 * Reads the definitions for an image item from the given .tsk file
 * It does _not_ create the image itself, a call to CImageItem::Create() must be done
 * to read the image in memory and prepare
 *
 * @param szFilename char*: full path and filename to the .TSK file
 *
 * @return char*: full path and filename to the .png image which represents this image item.
         * caller MUST delete it.
 */
TCHAR* CImageItem::Read(const TCHAR *szFilename)
{
	TCHAR 	buffer[501];
	TCHAR 	szDrive[MAX_PATH], szPath[MAX_PATH];
	TCHAR	*szFinalName = 0;

	GetPrivateProfileString(m_szName, _T("Glyph"), _T("None"), buffer, 500, szFilename);
	if (_tcscmp(buffer, _T("None"))) {
		_stscanf(buffer, _T("%d,%d,%d,%d"), &m_glyphMetrics[0], &m_glyphMetrics[1],
			   &m_glyphMetrics[2], &m_glyphMetrics[3]);
		if (m_glyphMetrics[2] > m_glyphMetrics[0] && m_glyphMetrics[3] > m_glyphMetrics[1]) {
			m_dwFlags |= IMAGE_GLYPH;
			m_glyphMetrics[2] = (m_glyphMetrics[2] - m_glyphMetrics[0]) + 1;
			m_glyphMetrics[3] = (m_glyphMetrics[3] - m_glyphMetrics[1]) + 1;
		}
	}
	GetPrivateProfileString(m_szName, _T("Image"), _T("None"), buffer, 500, szFilename);
	if (_tcscmp(buffer, _T("None")) || m_dwFlags & IMAGE_GLYPH) {
		szFinalName = new TCHAR[MAX_PATH];
		//strncpy(m_szName, &m_szName[1], sizeof(m_szName));
		//m_szName[sizeof(m_szName) - 1] = 0;
		_tsplitpath(szFilename, szDrive, szPath, NULL, NULL);
		mir_sntprintf(szFinalName, MAX_PATH, _T("%s\\%s%s"), szDrive, szPath, buffer);
		if (!PathFileExists(szFinalName)) {
			delete[] szFinalName;
			szFinalName = 0;
		}
		m_alpha = GetPrivateProfileInt(m_szName, _T("Alpha"), 100, szFilename);
		m_alpha = min(m_alpha, 100);
		m_alpha = (BYTE)((FLOAT)(((FLOAT) m_alpha) / 100) * 255);
		m_bf.SourceConstantAlpha = m_alpha;
		m_bLeft = GetPrivateProfileInt(m_szName, _T("Left"), 0, szFilename);
		m_bRight = GetPrivateProfileInt(m_szName, _T("Right"), 0, szFilename);
		m_bTop = GetPrivateProfileInt(m_szName, _T("Top"), 0, szFilename);
		m_bBottom = GetPrivateProfileInt(m_szName, _T("Bottom"), 0, szFilename);
		if (m_dwFlags & IMAGE_GLYPH) {
			m_width = m_glyphMetrics[2];
			m_height = m_glyphMetrics[3];
			m_inner_height = m_glyphMetrics[3] - m_bTop - m_bBottom;
			m_inner_width = m_glyphMetrics[2] - m_bRight - m_bLeft;

			if (m_bTop && m_bBottom && m_bLeft && m_bRight)
				m_dwFlags |= IMAGE_FLAG_DIVIDED;
			m_bf.BlendFlags = 0;
			m_bf.BlendOp = AC_SRC_OVER;
			m_bf.AlphaFormat = 0;
			m_dwFlags |= IMAGE_PERPIXEL_ALPHA;
			m_bf.AlphaFormat = AC_SRC_ALPHA;
			if (m_inner_height <= 0 || m_inner_width <= 0) {
				if (szFinalName) {
					delete[] szFinalName;
					szFinalName = 0;
				}
				return(szFinalName);
			}
		}
		GetPrivateProfileString(m_szName, _T("Fillcolor"), _T("None"), buffer, 500, szFilename);
		if (_tcscmp(buffer, _T("None"))) {
			COLORREF fillColor = CSkin::HexStringToLong(buffer);
			m_fillBrush = CreateSolidBrush(fillColor);
			m_dwFlags |= IMAGE_FILLSOLID;
		} else
			m_fillBrush = 0;
		GetPrivateProfileString(m_szName, _T("Colorkey"), _T("None"), buffer, 500, szFilename);
		if (_tcscmp(buffer, _T("None"))) {
			CSkin::m_ContainerColorKey = CSkin::HexStringToLong(buffer);
			if (CSkin::m_ContainerColorKeyBrush)
				DeleteObject(CSkin::m_ContainerColorKeyBrush);
			CSkin::m_ContainerColorKeyBrush = CreateSolidBrush(CSkin::m_ContainerColorKey);
		}
		GetPrivateProfileString(m_szName, _T("Stretch"), _T("None"), buffer, 500, szFilename);
		if (buffer[0] == 'B' || buffer[0] == 'b')
			m_bStretch = IMAGE_STRETCH_B;
		else if (buffer[0] == 'h' || buffer[0] == 'H')
			m_bStretch = IMAGE_STRETCH_V;
		else if (buffer[0] == 'w' || buffer[0] == 'W')
			m_bStretch = IMAGE_STRETCH_H;
		m_hbm = 0;
		if (GetPrivateProfileInt(m_szName, _T("Perpixel"), 0, szFilename))
			m_dwFlags |= IMAGE_PERPIXEL_ALPHA;

		return(szFinalName);
	}
	return 0;
}

/**
 * Free all resources allocated by an image item
 */
void CImageItem::Free()
{
	if (m_hdc ) {
		::SelectObject(m_hdc, m_hbmOld);
		::DeleteDC(m_hdc);
	}
	if (m_hbm)
		::DeleteObject(m_hbm);

	if (m_fillBrush)
		::DeleteObject(m_fillBrush);

	ZeroMemory(this, sizeof(CImageItem));
}

/**
 * Set the alpha value for a 32bit RGBA bitmap to the given value
 *
 * @param hBitmap	bitmap handle
 * @param bAlpha	new alpha value (0 -> fully transparent, 255 -> opaque)
 * 					default value is 255
 */
void CImageItem::SetBitmap32Alpha(HBITMAP hBitmap, BYTE bAlpha)
{
	BITMAP bmp;
	int x, y;
	BOOL fixIt = TRUE;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	BYTE *p = (BYTE *)mir_alloc(dwLen);
	if (p == NULL)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	for (y = 0; y < bmp.bmHeight; ++y) {
		BYTE *px = p + bmp.bmWidth * 4 * y;

		for (x = 0; x < bmp.bmWidth; ++x) {
			px[3] = bAlpha;
			px += 4;
		}
	}
	SetBitmapBits(hBitmap, bmp.bmWidth * bmp.bmHeight * 4, p);
	mir_free(p);
}

void CImageItem::PreMultiply(HBITMAP hBitmap, int mode)
{
	BITMAP bmp;
	::GetObject(hBitmap, sizeof(bmp), &bmp);

	int width = bmp.bmWidth;
	int height = bmp.bmHeight;
	DWORD dwLen = width * height * 4;
	BYTE *p = (BYTE *)mir_alloc(dwLen);
	if (p) {
		::GetBitmapBits(hBitmap, dwLen, p);
		for (int y = 0; y < height; ++y) {
			BYTE *px = p + width * 4 * y;

			for (int x = 0; x < width; ++x) {
				if (mode) {
					BYTE alpha = px[3];
					px[0] = px[0] * alpha / 255;
					px[1] = px[1] * alpha / 255;
					px[2] = px[2] * alpha / 255;
				} else
					px[3] = 255;
				px += 4;
			}
		}
		dwLen = ::SetBitmapBits(hBitmap, dwLen, p);
		mir_free(p);
	}
}

void CImageItem::Colorize(HBITMAP hBitmap, BYTE dr, BYTE dg, BYTE db, BYTE alpha)
{
	BYTE 	*p = NULL;
	DWORD 	dwLen;
	int 	width, height, x, y;
	BITMAP 	bmp;

	float r = (float)dr / 2.55;
	float g = (float)dg / 2.55;
	float b = (float)db / 2.55;

	::GetObject(hBitmap, sizeof(bmp), &bmp);
	width = bmp.bmWidth;
	height = bmp.bmHeight;
	dwLen = width * height * 4;
	p = (BYTE *)mir_alloc(dwLen);
	if (p) {
		::GetBitmapBits(hBitmap, dwLen, p);
		for (y = 0; y < height; ++y) {
			BYTE *px = p + width * 4 * y;

			for (x = 0; x < width; ++x) {
				px[0] = (int)(px[0] + b) > 255 ? 255 : px[0] + b;
				px[1] = (int)(px[1] + g) > 255 ? 255 : px[1] + g;
				px[2] = (int)(px[2] + r) > 255 ? 255 : px[2] + r;
				px[3] += alpha;
				px += 4;
			}
		}
		dwLen = ::SetBitmapBits(hBitmap, dwLen, p);
		mir_free(p);
	}
}

/**
 * load PNG image using core service (advaimg)
 */

HBITMAP TSAPI CImageItem::LoadPNG(const TCHAR *szFilename)
{
	HBITMAP hBitmap = 0;
	hBitmap = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)szFilename, IMGL_TCHAR);
	return hBitmap;
}


/**
 * set filename and load parameters from the database
 * called on:
 * ) init
 * ) manual loading on user's request
 */
void CSkin::setFileName()
{
	DBVARIANT dbv;
	if (0 == db_get_ts(0, SRMSGMOD_T, "ContainerSkin", &dbv)) {
		PathToAbsoluteT(dbv.ptszVal, m_tszFileName, M.getSkinPath());
		db_free(&dbv);
	}
	else
		m_tszFileName[0] = 0;

	m_fLoadOnStartup = M.GetByte("useskin", 0) ? true : false;
}
/**
 * initialize the skin object
 */

void CSkin::Init(bool fStartup)
{
	m_ImageItems = 0;
	m_SkinItems = ::SkinItems;
	m_fLoadOnStartup = false;
	m_skinEnabled = m_frameSkins = false;
	m_bAvatarBorderType = AVBORDER_NORMAL;
	m_avatarBorderClr = ::GetSysColor(COLOR_3DDKSHADOW);
	m_sideBarContainerBG = ::GetSysColor(COLOR_3DFACE);

	m_SkinItems[ID_EXTBKINFOPANELBG] = _defInfoPanel;
	/*
	 * read current skin name from db
	 */

	m_DisableScrollbars = M.GetByte("disableVScroll", 0) ? true : false;

	setFileName();
	m_aeroEffect = M.GetDword("aerostyle", AERO_EFFECT_MILK);
	if (m_fLoadOnStartup && fStartup)
		Load();
}

/**
 * throws a warning to close all message windows before a skin can
 * be loaded. user can cancel it
 * @return: bool: true if windows were closed (or none was open) -> skin can be loaded
 *
 */
bool CSkin::warnToClose() const
{
	if (::pFirstContainer == NULL)
		return true;

	if (MessageBox(0,
			TranslateT("All message containers need to close before the skin can be changed\nProceed?"),
			TranslateT("Change skin"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return false;

	CloseAllContainers();
	return true;
}

/**
 * mir_free the aero tab bitmaps
 * only called on exit, NOT when a skin is unloaded as these elements
 * are always needed (even without a skin)
 */
void CSkin::UnloadAeroTabs()
{
	if (m_tabTop) {
		delete m_tabTop;
		m_tabTop = NULL;
	}

	if (m_tabBottom) {
		delete m_tabBottom;
		m_tabBottom = NULL;
	}

	if (m_tabGlowTop) {
		delete m_tabGlowTop;
		m_tabGlowTop = NULL;
	}

	if (m_tabGlowBottom) {
		delete m_tabGlowBottom;
		m_tabGlowTop = NULL;
	}

	if (m_switchBarItem) {
		delete m_switchBarItem;
		m_switchBarItem = NULL;
	}
}

/**
 * Unload the skin. Free everything allocated.
 * Called when:
 * * user unloads the skin from the dialog box
 * * a new skin is loaded by user's request.
 */

void CSkin::Unload()
{
	// do nothing when user decides to not close any window
	if (warnToClose() == false)
		return;				  						

	/*
	 * delete all image items
	 */

	m_skinEnabled = m_frameSkins = false;

	CImageItem *tmp = m_ImageItems, *nextItem;
	while(tmp) {
		nextItem = tmp->getNextItem();
		delete tmp;
		tmp = nextItem;
	}

	m_ImageItems = 0;
	m_glyphItem.Free();

	if (m_ContainerColorKeyBrush)
		::DeleteObject(m_ContainerColorKeyBrush);
	if (m_MenuBGBrush)
		::DeleteObject(m_MenuBGBrush);
	if (m_BrushBack)
		::DeleteObject(m_BrushBack);

	m_ContainerColorKeyBrush = m_MenuBGBrush = 0;

	if (m_SkinLightShadowPen)
		::DeleteObject(m_SkinLightShadowPen);
	m_SkinLightShadowPen = 0;

	if (m_SkinDarkShadowPen)
		::DeleteObject(m_SkinDarkShadowPen);
	m_SkinDarkShadowPen = 0;

	if (m_SkinItems) {
		for (int i=0; i < ID_EXTBK_LAST; i++) {
			m_SkinItems[i].IGNORED = 1;
			m_SkinItems[i].imageItem = 0;
		}
		m_SkinItems[ID_EXTBKINFOPANELBG] = _defInfoPanel;
	}

	ZeroMemory(this, sizeof(CSkin));

	m_SkinItems = ::SkinItems;
	setFileName();

	m_bClipBorder = m_DisableScrollbars = false;
	m_SkinnedFrame_left = m_SkinnedFrame_right = m_SkinnedFrame_bottom = m_SkinnedFrame_caption = 0;
	m_realSkinnedFrame_left = m_realSkinnedFrame_right = m_realSkinnedFrame_bottom = m_realSkinnedFrame_caption = 0;

	m_titleBarLeftOff = m_titleButtonTopOff = m_captionOffset = m_captionPadding =
		m_titleBarRightOff = m_sidebarTopOffset = m_sidebarBottomOffset = m_bRoundedCorner = 0;

	m_titleBarButtonSize.cx = m_titleBarButtonSize.cy = 0;
	m_ContainerColorKey = 0;
	m_ContainerColorKeyBrush = m_MenuBGBrush = 0;
	m_skinEnabled = m_frameSkins = false;

	if (m_closeIcon)
		::DestroyIcon(m_closeIcon);
	if (m_maxIcon)
		::DestroyIcon(m_maxIcon);
	if (m_minIcon)
		::DestroyIcon(m_minIcon);

	m_closeIcon = m_maxIcon = m_minIcon = 0;

	for (int i=0; i < m_nrSkinIcons; i++)
		if (m_skinIcons[i].phIcon )
			::DestroyIcon(*(m_skinIcons[i].phIcon));
	mir_free(m_skinIcons);

	if (!g_bShutdown) {
		M.getAeroState();				// refresh after unload
		::FreeTabConfig();
		::ReloadTabConfig();
	}

	m_bAvatarBorderType = AVBORDER_NORMAL;
	m_avatarBorderClr = ::GetSysColor(COLOR_3DDKSHADOW);
	m_sideBarContainerBG = ::GetSysColor(COLOR_3DFACE);

	m_DisableScrollbars = M.GetByte("disableVScroll", 0) ? true : false;
}

void CSkin::LoadIcon(const TCHAR *szSection, const TCHAR *name, HICON *hIcon)
{
	TCHAR buffer[512];
	if (hIcon)
		return;
	if (*hIcon != 0)
		DestroyIcon(*hIcon);
	GetPrivateProfileString(szSection, name, _T("none"), buffer, 250, m_tszFileName);
	buffer[500] = 0;

	if (_tcsicmp(buffer, _T("none"))) {
		TCHAR szDrive[MAX_PATH], szDir[MAX_PATH], szImagePath[MAX_PATH];

		_tsplitpath(m_tszFileName, szDrive, szDir, NULL, NULL);
		mir_sntprintf(szImagePath, MAX_PATH, _T("%s\\%s\\%s"), szDrive, szDir, buffer);
		*hIcon = (HICON)LoadImage(0, szImagePath, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	}
}


/**
 * Read a skin item (not a image item - these are handled elsewhere)
 * Reads all values from the .ini style .tsk file and fills the
 * structure.
 *
 * @param id     int: zero-based index into the table of predefined skin items
 * @param szItem char *: the section name in the ini file which holds the definition for this
            *    item.
 */
void CSkin::ReadItem(const int id, const TCHAR *szItem)
{
	TCHAR buffer[512];
	TCHAR def_color[20];
	COLORREF clr;
	CSkinItem *defaults = &StatusItem_Default;

	CSkinItem	*this_item = &m_SkinItems[id];

	this_item->ALPHA = (int)GetPrivateProfileInt(szItem, _T("Alpha"), defaults->ALPHA, m_tszFileName);
	this_item->ALPHA = min(this_item->ALPHA, 100);

	clr = RGB(GetBValue(defaults->COLOR), GetGValue(defaults->COLOR), GetRValue(defaults->COLOR));
	mir_sntprintf(def_color, 15, _T("%6.6x"), clr);
	GetPrivateProfileString(szItem, _T("Color1"), def_color, buffer, 400, m_tszFileName);
	this_item->COLOR = HexStringToLong(buffer);

	clr = RGB(GetBValue(defaults->COLOR2), GetGValue(defaults->COLOR2), GetRValue(defaults->COLOR2));
	mir_sntprintf(def_color, 15, _T("%6.6x"), clr);
	GetPrivateProfileString(szItem, _T("Color2"), def_color, buffer, 400, m_tszFileName);
	this_item->COLOR2 = HexStringToLong(buffer);

	this_item->COLOR2_TRANSPARENT = (BYTE)GetPrivateProfileInt(szItem, _T("COLOR2_TRANSPARENT"), defaults->COLOR2_TRANSPARENT, m_tszFileName);

	this_item->CORNER = defaults->CORNER & CORNER_ACTIVE ? defaults->CORNER : 0;
	GetPrivateProfileString(szItem, _T("Corner"), _T("None"), buffer, 400, m_tszFileName);
	if (_tcsstr(buffer, _T("tl")))
		this_item->CORNER |= CORNER_TL;
	if (_tcsstr(buffer, _T("tr")))
		this_item->CORNER |= CORNER_TR;
	if (_tcsstr(buffer, _T("bl")))
		this_item->CORNER |= CORNER_BL;
	if (_tcsstr(buffer, _T("br")))
		this_item->CORNER |= CORNER_BR;
	if (this_item->CORNER)
		this_item->CORNER |= CORNER_ACTIVE;

	this_item->GRADIENT = defaults->GRADIENT & GRADIENT_ACTIVE ?  defaults->GRADIENT : 0;
	GetPrivateProfileString(szItem, _T("Gradient"), _T("None"), buffer, 400, m_tszFileName);
	if (_tcsstr(buffer, _T("left")))
		this_item->GRADIENT = GRADIENT_RL;
	else if (_tcsstr(buffer, _T("right")))
		this_item->GRADIENT = GRADIENT_LR;
	else if (_tcsstr(buffer, _T("up")))
		this_item->GRADIENT = GRADIENT_BT;
	else if (_tcsstr(buffer, _T("down")))
		this_item->GRADIENT = GRADIENT_TB;
	if (this_item->GRADIENT)
		this_item->GRADIENT |= GRADIENT_ACTIVE;

	this_item->MARGIN_LEFT = GetPrivateProfileInt(szItem, _T("Left"), defaults->MARGIN_LEFT, m_tszFileName);
	this_item->MARGIN_RIGHT = GetPrivateProfileInt(szItem, _T("Right"), defaults->MARGIN_RIGHT, m_tszFileName);
	this_item->MARGIN_TOP = GetPrivateProfileInt(szItem, _T("Top"), defaults->MARGIN_TOP, m_tszFileName);
	this_item->MARGIN_BOTTOM = GetPrivateProfileInt(szItem, _T("Bottom"), defaults->MARGIN_BOTTOM, m_tszFileName);
	this_item->BORDERSTYLE = GetPrivateProfileInt(szItem, _T("Radius"), defaults->BORDERSTYLE, m_tszFileName);

	GetPrivateProfileString(szItem, _T("Textcolor"), _T("ffffffff"), buffer, 400, m_tszFileName);
	this_item->TEXTCOLOR = HexStringToLong(buffer);
	this_item->IGNORED = 0;
}

/**
 * stub to read a single item. Called from CSkin::LoadItems()
 * The real work is done by the CImageItem::Read().
 *
 * @param itemname char *: image item name, also section name in the .tsk file
 */
void CSkin::ReadImageItem(const TCHAR *itemname)
{
	TCHAR buffer[512], szItemNr[30];

	CImageItem tmpItem(itemname);

	TCHAR *szImageFileName = tmpItem.Read(m_tszFileName);

	if (!_tcsicmp(itemname, _T("$glyphs")) && szImageFileName != 0) {		// the glyph item MUST have a valid image
		tmpItem.Create(szImageFileName);
		if (tmpItem.getHbm()) {
			m_glyphItem = tmpItem;
			m_fHaveGlyph = true;
		}
		tmpItem.Clear();
		delete[] szImageFileName;
		return;
	}

	/*
	 * handle the assignments of image items to skin items
	 */

	for (int n = 0;;n++) {
		mir_sntprintf(szItemNr, 30, _T("Item%d"), n);
		GetPrivateProfileString(itemname, szItemNr, _T("None"), buffer, 500, m_tszFileName);
		if (!_tcscmp(buffer, _T("None")))
			break;
		for (int i=0; i <= ID_EXTBK_LAST; i++) {
			if (!_tcsicmp(SkinItems[i].szName[0] == '{' ? &SkinItems[i].szName[3] : SkinItems[i].szName, buffer)) {
				if (!(tmpItem.getFlags() & IMAGE_GLYPH)) {
					if (szImageFileName)
						tmpItem.Create(szImageFileName);
					else {
						tmpItem.Clear();
						return;							// no reference to the glpyh image and no valid image file name -> invalid item
					}
				}
				if (tmpItem.getHbm() || (tmpItem.getFlags() & IMAGE_GLYPH)) {
					CImageItem *newItem = new CImageItem(tmpItem);
					SkinItems[i].imageItem = newItem;
					if (m_ImageItems == NULL)
						m_ImageItems = newItem;
					else {
						CImageItem *pItem = m_ImageItems;

						while (pItem->getNextItem() != 0)
							pItem = pItem->getNextItem();
						pItem->setNextItem(newItem);
					}
				}
			}
		}
	}
	tmpItem.Clear();
	if (szImageFileName)
		delete[] szImageFileName;
}

/**
 * Load the skin from the .tsk file
 * It reads and initializes all static values for the skin. Afterwards
 * it calls ReadItems() to read additional skin information like image items,
 * buttons and icons.
 */
void CSkin::Load(void)
{
	if (warnToClose() == false)
		return;

	if (m_skinEnabled) {
		Unload();
		m_skinEnabled = false;
	}

	m_fHaveGlyph = false;

	if ( !m_tszFileName[0] || !::PathFileExists(m_tszFileName))
		return;

	TCHAR *p;
	TCHAR *szSections = (TCHAR*)mir_alloc(6004);
	int i = 1, j = 0;
	UINT  data;
	TCHAR buffer[500];

	if (!(GetPrivateProfileInt(_T("Global"), _T("Version"), 0, m_tszFileName) >= 1 && GetPrivateProfileInt(_T("Global"), _T("Signature"), 0, m_tszFileName) == 101))
		return;

	i = 0;
	while (_tagSettings[i].szIniKey != NULL) {
		data = 0;
		data = GetPrivateProfileInt(_tagSettings[i].szIniKey, _tagSettings[i].szIniName,
									_tagSettings[i].defaultval, m_tszFileName);
		switch (_tagSettings[i].size) {
		case 1:
			db_set_b(0, SRMSGMOD_T, _tagSettings[i].szSetting, (BYTE)data);
			break;
		case 4:
			db_set_dw(0, SRMSGMOD_T, _tagSettings[i].szSetting, data);
			break;
		case 2:
			db_set_w(NULL, SRMSGMOD_T, _tagSettings[i].szSetting, (WORD)data);
			break;
		case 5:
			GetPrivateProfileString(_tagSettings[i].szIniKey, _tagSettings[i].szIniName, _T("000000"),
				buffer, 10, m_tszFileName);
			db_set_dw(0, SRMSGMOD_T, _tagSettings[i].szSetting, HexStringToLong(buffer));
			break;
		}
		i++;
	}

	m_DisableScrollbars = M.GetByte("disableVScroll", 0) ? true : false;

	ZeroMemory(szSections, 6000);
	p = szSections;
	GetPrivateProfileSectionNames(szSections, 3000, m_tszFileName);
	szSections[3001] = szSections[3000] = 0;
	p = szSections;
	while (lstrlen(p) > 1) {
		if (p[0] != '%') {
			p += (lstrlen(p) + 1);
			continue;
		}
		for (i=0; i <= ID_EXTBK_LAST; i++) {
			if (!_tcsicmp(&p[1], SkinItems[i].szName[0] == '{' ? &SkinItems[i].szName[3] : SkinItems[i].szName)) {
				ReadItem(i, p);
				break;
			}
		}
		p += (lstrlen(p) + 1);
		j++;
	}

	if (j > 0) {
		m_skinEnabled = true;
		M.getAeroState();		// refresh aero state (set to false when a skin is successfully loaded and active)
	}

	GetPrivateProfileString(_T("Avatars"), _T("BorderColor"), _T("000000"), buffer, 20, m_tszFileName);
	m_avatarBorderClr = (COLORREF)HexStringToLong(buffer);

	GetPrivateProfileString(_T("Global"), _T("SideBarBG"), _T("None"), buffer, 20, m_tszFileName);
	if (_tcscmp(buffer, _T("None")))
		m_sideBarContainerBG = (COLORREF)HexStringToLong(buffer);
	else
		m_sideBarContainerBG = SkinItems[ID_EXTBKSIDEBARBG].COLOR;

	m_bAvatarBorderType = GetPrivateProfileInt(_T("Avatars"), _T("BorderType"), 1, m_tszFileName);

	LoadIcon(_T("Global"), _T("CloseGlyph"), &CSkin::m_closeIcon);
	LoadIcon(_T("Global"), _T("MaximizeGlyph"), &CSkin::m_maxIcon);
	LoadIcon(_T("Global"), _T("MinimizeGlyph"), &CSkin::m_minIcon);

	m_frameSkins = GetPrivateProfileInt(_T("Global"), _T("framelessmode"), 0, m_tszFileName) ? true : false;
	m_DisableScrollbars = GetPrivateProfileInt(_T("Global"), _T("NoScrollbars"), 0, m_tszFileName) ? true : false;

	m_SkinnedFrame_left = GetPrivateProfileInt(_T("WindowFrame"), _T("left"), 4, m_tszFileName);
	m_SkinnedFrame_right = GetPrivateProfileInt(_T("WindowFrame"), _T("right"), 4, m_tszFileName);
	m_SkinnedFrame_caption = GetPrivateProfileInt(_T("WindowFrame"), _T("Caption"), 24, m_tszFileName);
	m_SkinnedFrame_bottom = GetPrivateProfileInt(_T("WindowFrame"), _T("bottom"), 4, m_tszFileName);

	m_titleBarButtonSize.cx = GetPrivateProfileInt(_T("WindowFrame"), _T("TitleButtonWidth"), 24, m_tszFileName);
	m_titleBarButtonSize.cy = GetPrivateProfileInt(_T("WindowFrame"), _T("TitleButtonHeight"), 12, m_tszFileName);
	m_titleButtonTopOff = GetPrivateProfileInt(_T("WindowFrame"), _T("TitleButtonTopOffset"), 0, m_tszFileName);

	m_titleBarRightOff = GetPrivateProfileInt(_T("WindowFrame"), _T("TitleBarRightOffset"), 0, m_tszFileName);
	m_titleBarLeftOff = GetPrivateProfileInt(_T("WindowFrame"), _T("TitleBarLeftOffset"), 0, m_tszFileName);

	m_captionOffset = GetPrivateProfileInt(_T("WindowFrame"), _T("CaptionOffset"), 3, m_tszFileName);
	m_captionPadding = GetPrivateProfileInt(_T("WindowFrame"), _T("CaptionPadding"), 0, m_tszFileName);
	m_sidebarTopOffset = GetPrivateProfileInt(_T("ClientArea"), _T("SidebarTop"), -1, m_tszFileName);
	m_sidebarBottomOffset = GetPrivateProfileInt(_T("ClientArea"), _T("SidebarBottom"), -1, m_tszFileName);

	m_bClipBorder = GetPrivateProfileInt(_T("WindowFrame"), _T("ClipFrame"), 0, m_tszFileName) ? true : false;

	BYTE radius_tl, radius_tr, radius_bl, radius_br;
	TCHAR 	szFinalName[MAX_PATH];
	TCHAR 	szDrive[MAX_PATH], szPath[MAX_PATH];

	radius_tl = GetPrivateProfileInt(_T("WindowFrame"), _T("RadiusTL"), 0, m_tszFileName);
	radius_tr = GetPrivateProfileInt(_T("WindowFrame"), _T("RadiusTR"), 0, m_tszFileName);
	radius_bl = GetPrivateProfileInt(_T("WindowFrame"), _T("RadiusBL"), 0, m_tszFileName);
	radius_br = GetPrivateProfileInt(_T("WindowFrame"), _T("RadiusBR"), 0, m_tszFileName);

	CSkin::m_bRoundedCorner = radius_tl;

	GetPrivateProfileString(_T("Theme"), _T("File"), _T("None"), buffer, MAX_PATH, m_tszFileName);

	_tsplitpath(m_tszFileName, szDrive, szPath, NULL, NULL);
	mir_sntprintf(szFinalName, MAX_PATH, _T("%s\\%s\\%s"), szDrive, szPath, buffer);
	if (PathFileExists(szFinalName)) {
		ReadThemeFromINI(szFinalName, 0, FALSE, m_fLoadOnStartup ? 0 : M.GetByte("skin_loadmode", 0));
		CacheLogFonts();
		CacheMsgLogIcons();
	}

	GetPrivateProfileString(_T("Global"), _T("MenuBarBG"), _T("None"), buffer, 20, m_tszFileName);
	data = HexStringToLong(buffer);
	if (m_MenuBGBrush) {
		DeleteObject(m_MenuBGBrush);
		m_MenuBGBrush = 0;
	}
	if (_tcscmp(buffer, _T("None")))
		m_MenuBGBrush = CreateSolidBrush(data);

	GetPrivateProfileString(_T("Global"), _T("LightShadow"), _T("000000"), buffer, 20, m_tszFileName);
	data = HexStringToLong(buffer);
	CSkin::m_SkinLightShadowPen = CreatePen(PS_SOLID, 1, RGB(GetRValue(data), GetGValue(data), GetBValue(data)));
	GetPrivateProfileString(_T("Global"), _T("DarkShadow"), _T("000000"), buffer, 20, m_tszFileName);
	data = HexStringToLong(buffer);
	CSkin::m_SkinDarkShadowPen = CreatePen(PS_SOLID, 1, RGB(GetRValue(data), GetGValue(data), GetBValue(data)));

	SkinCalcFrameWidth();

	GetPrivateProfileString(_T("Global"), _T("FontColor"), _T("None"), buffer, 20, m_tszFileName);
	if (_tcscmp(buffer, _T("None")))
		CSkin::m_DefaultFontColor = HexStringToLong(buffer);
	else
		CSkin::m_DefaultFontColor = GetSysColor(COLOR_BTNTEXT);
	buffer[499] = 0;
	mir_free(szSections);

	LoadItems();
	::FreeTabConfig();
	::ReloadTabConfig();
}

#define SECT_BUFFER_SIZE 2500

/**
 * Load additional skin items (like image items, buttons, icons etc.)
 * This is called AFTER ReadItems() has read the basic skin items
 */
void CSkin::LoadItems()
{
	TCHAR *szSections = NULL;
	TCHAR *p, *p1;
	TIconDesc tmpIconDesc = {0};

	CImageItem *pItem = m_ImageItems;

	if (m_skinIcons == NULL)
		m_skinIcons = (TIconDescW *)mir_calloc(sizeof(TIconDescW) * NR_MAXSKINICONS);

	m_nrSkinIcons = 0;

	szSections = (TCHAR*)mir_alloc((SECT_BUFFER_SIZE + 2) * sizeof(TCHAR));
	ZeroMemory(szSections, (SECT_BUFFER_SIZE + 2) * sizeof(TCHAR));

	GetPrivateProfileSection(_T("Icons"), szSections, SECT_BUFFER_SIZE, m_tszFileName);
	szSections[SECT_BUFFER_SIZE] = 0;

	p = szSections;
	while (lstrlen(p) > 1) {
		p1 = _tcschr(p, (int)'=');
		if (p1)
			*p1 = 0;
		if (m_nrSkinIcons < NR_MAXSKINICONS && p1) {
			LoadIcon(_T("Icons"), p, (HICON *)&tmpIconDesc.uId);
			if (tmpIconDesc.uId) {
				ZeroMemory(&m_skinIcons[m_nrSkinIcons], sizeof(TIconDesc));
				m_skinIcons[m_nrSkinIcons].uId = tmpIconDesc.uId;
				m_skinIcons[m_nrSkinIcons].phIcon = (HICON *)(&m_skinIcons[m_nrSkinIcons].uId);
				m_skinIcons[m_nrSkinIcons].szName = (TCHAR*)mir_alloc(sizeof(TCHAR) * (lstrlen(p) + 1));
				lstrcpy(m_skinIcons[m_nrSkinIcons].szName, p);
				m_nrSkinIcons++;
			}
		}
		if (p1)
			*p1 = '=';
		p += (lstrlen(p) + 1);
	}

	ZeroMemory(szSections, (SECT_BUFFER_SIZE + 2) * sizeof(TCHAR));
	GetPrivateProfileSectionNames(szSections, SECT_BUFFER_SIZE, m_tszFileName);
	szSections[SECT_BUFFER_SIZE] = 0;

	p = szSections;
	while (lstrlen(p) > 1) {
		if (p[0] == '$')
			ReadImageItem(p);
		p += (lstrlen(p) + 1);
	}
	nextButtonID = IDC_TBFIRSTUID;

	p = szSections;
	/*
	while (lstrlen(p) > 1) {
		if (p[0] == '!')
		 	ReadButtonItem(p);
		p += (lstrlen(p) + 1);
	}
	*/
	mir_free(szSections);
	g_ButtonSet.top = GetPrivateProfileInt(_T("ButtonArea"), _T("top"), 0, m_tszFileName);
	g_ButtonSet.bottom = GetPrivateProfileInt(_T("ButtonArea"), _T("bottom"), 0, m_tszFileName);
	g_ButtonSet.left = GetPrivateProfileInt(_T("ButtonArea"), _T("left"), 0, m_tszFileName);
	g_ButtonSet.right = GetPrivateProfileInt(_T("ButtonArea"), _T("right"), 0, m_tszFileName);
}

/**
 * setup and cache the bitmap for the close button on tabs and switch bar
 * buttons.
 * re-created when:
 * ) theme changes
 * ) icons change (via ico lib service)
 *
 * @param fDeleteOnly:	only delete GDI resources (this is ONLY used at plugin shutdown)
 */
void CSkin::setupTabCloseBitmap(bool fDeleteOnly)
{
	if (m_tabCloseHDC || fDeleteOnly) {
		if (m_tabCloseHDC) {
			::SelectObject(m_tabCloseHDC, m_tabCloseOldBitmap);
			::DeleteObject(m_tabCloseBitmap);
			::DeleteDC(m_tabCloseHDC);
		}
		if (fDeleteOnly)
			return;
	}

	bool fFree = false;
	RECT rc = {0, 0, 20, 20};
	HDC  dc = ::GetDC(PluginConfig.g_hwndHotkeyHandler);
	m_tabCloseHDC = ::CreateCompatibleDC(dc);

	if (M.isAero())
		m_tabCloseBitmap = CreateAeroCompatibleBitmap(rc, m_tabCloseHDC);
	else
		m_tabCloseBitmap = ::CreateCompatibleBitmap(dc, 20, 20);

	m_tabCloseOldBitmap = reinterpret_cast<HBITMAP>(::SelectObject(m_tabCloseHDC, m_tabCloseBitmap));

	if (M.isVSThemed() || M.isAero()) {
		::FillRect(m_tabCloseHDC, &rc, M.isAero() ? reinterpret_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)) : ::GetSysColorBrush(COLOR_3DFACE));

		HANDLE hTheme = OpenThemeData(PluginConfig.g_hwndHotkeyHandler, L"BUTTON");
		rc.left--; rc.right++;
		rc.top--; rc.bottom++;
		DrawThemeParentBackground(PluginConfig.g_hwndHotkeyHandler, m_tabCloseHDC, &rc);
		DrawThemeBackground(hTheme, m_tabCloseHDC, 1, 1, &rc, &rc);
		CloseThemeData(hTheme);
	}
	else if (CSkin::m_skinEnabled)
		CSkin::DrawItem(m_tabCloseHDC, &rc, &SkinItems[ID_EXTBKBUTTONSNPRESSED]);
	else {
		::FillRect(m_tabCloseHDC, &rc, ::GetSysColorBrush(COLOR_3DFACE));
		::DrawFrameControl(m_tabCloseHDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_MONO);
	}
	::DrawIconEx(m_tabCloseHDC, 2, 2, PluginConfig.g_buttonBarIcons[ICON_BUTTON_CANCEL], 16, 16, 0, 0, DI_NORMAL);
	::SelectObject(m_tabCloseHDC, m_tabCloseOldBitmap);

	HBITMAP hbmTemp = ResizeBitmap(m_tabCloseBitmap, 16, 16, fFree);
	::DeleteObject(m_tabCloseBitmap);
	m_tabCloseBitmap = hbmTemp;
	CImageItem::PreMultiply(m_tabCloseBitmap, 1);
	m_tabCloseOldBitmap = reinterpret_cast<HBITMAP>(::SelectObject(m_tabCloseHDC, m_tabCloseBitmap));

	::ReleaseDC(PluginConfig.g_hwndHotkeyHandler, dc);
}
/**
 * load and setup some images which are used to draw tabs in aero mode
 * there is one image for tabs (it is flipped vertically for bottom tabs)
 * and one image for the glowing effect on tabs (also flipped for bottom
 * tabs).
 *
 * support for custom images added 3.0.0.34
 * user can place images with a custom_ prefix into the images folder and
 * they will be loaded instead the default ones.
 *
 * the 3rd image acts as background for switch bar buttons
 *
 * this is executed when:
 * ) dwm mode changes
 * ) aero effect is changed by the user
 * ) glow colorization is changed by user's request
 */
void CSkin::setupAeroSkins()
{
	M.getAeroState();
	UnloadAeroTabs();

	if (!m_fAeroSkinsValid)
		return;

	TCHAR	tszFilename[MAX_PATH], tszBasePath[MAX_PATH];
	_tcsncpy_s(tszBasePath, M.getDataPath(), _TRUNCATE);
	if (tszBasePath[lstrlen(tszBasePath) - 1] != '\\')
		_tcscat(tszBasePath, _T("\\"));

	// load unknown avatar..
	if (0 == PluginConfig.g_hbmUnknown) {
		mir_sntprintf(tszFilename, MAX_PATH, _T("%scustom_unknown.png"), tszBasePath);
		if (!PathFileExists(tszFilename))
			mir_sntprintf(tszFilename, MAX_PATH, _T("%sunknown.png"), tszBasePath);
		PluginConfig.g_hbmUnknown = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)tszFilename, IMGL_TCHAR);
		if (PluginConfig.g_hbmUnknown == 0) {
			HDC dc = GetDC(0);
			PluginConfig.g_hbmUnknown = CreateCompatibleBitmap(dc, 20, 20);
			ReleaseDC(0, dc);
		}
	}

	mir_sntprintf(tszFilename, MAX_PATH, _T("%scustom_tabskin_aero.png"), tszBasePath);
	if (!PathFileExists(tszFilename))
		mir_sntprintf(tszFilename, MAX_PATH, _T("%stabskin_aero.png"), tszBasePath);

	BOOL isOpaque = false;
	if (CMimAPI::m_pfnDwmGetColorizationColor && M.isAero())
		CMimAPI::m_pfnDwmGetColorizationColor(&m_dwmColor, &isOpaque);
	else
		m_dwmColor = PluginConfig.m_fillColor;

	float fr = (float)((m_dwmColor & 0x00ff0000) >> 16);
	float fg = (float)((m_dwmColor & 0x0000ff00) >> 8);
	float fb = (float)((m_dwmColor & 0x000000ff));
	BYTE alphafactor = 255 - ((m_dwmColor & 0xff000000) >> 24);

	/*
	 * a bit tricky, because for low alpha settings (high DWM transparency), the dwm
	 * color is almost dark gray, so we need to intensify the strongest color a bit more than
	 * the others. This will give us a good match for the dwm color that can be used
	 * to render non transparent stuff (i.e. the tool bar).
	 *
	 * TODO: this isn't perfect yet, for some colors, the result is a bit off...
	 */

	if (!isOpaque && alphafactor > 150 && !(fr == fg && fg == fb)) {
		float fmax =  max(max(fr,fg), fb);

		if (fmax == fr) {
			fr *= (alphafactor / 100 * 2.2);
			fr = min(fr, 255);
			fb = min(fb * alphafactor / 100, 255);
			fg = min(fg * alphafactor / 100, 255);
		} else if (fmax == fg) {
			fg *= (alphafactor / 100 * 2.2);
			fg = min(fg, 255);
			fr = min(fr * alphafactor / 100, 255);
			fb = min(fb * alphafactor / 100, 255);
		} else {
			fb *= (alphafactor / 100 * 2.2);
			fb = min(fb, 255);
			fr = min(fr * alphafactor / 100, 255);
			fg = min(fg * alphafactor / 100, 255);
		}
	}

	m_dwmColorRGB = RGB((BYTE)fr, (BYTE)fg, (BYTE)fb);

	FIBITMAP *fib = (FIBITMAP *)CallService(MS_IMG_LOAD, (WPARAM)tszFilename, IMGL_TCHAR | IMGL_RETURNDIB);

	HBITMAP hbm = FIF->FI_CreateHBITMAPFromDIB(fib);

	CImageItem::Colorize(hbm, GetRValue(m_dwmColorRGB),
						 GetGValue(m_dwmColorRGB),
						 GetBValue(m_dwmColorRGB));

	CImageItem::PreMultiply(hbm, 1);

	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	m_tabTop = new CImageItem(4, 4, 4, 4, 0, hbm, IMAGE_FLAG_DIVIDED | IMAGE_PERPIXEL_ALPHA,
							  0, 255, 30, 80, 50, 100);

	m_tabTop->setAlphaFormat(AC_SRC_ALPHA, 255);
	m_tabTop->setMetrics(bm.bmWidth, bm.bmHeight);


	/*
	 * created inverted bitmap for bottom tabs
	 */

	FIF->FI_FlipVertical(fib);

	hbm = FIF->FI_CreateHBITMAPFromDIB(fib);

	CImageItem::Colorize(hbm, GetRValue(m_dwmColorRGB),
						 GetGValue(m_dwmColorRGB),
						 GetBValue(m_dwmColorRGB));

	CImageItem::PreMultiply(hbm, 1);
	FIF->FI_Unload(fib);

	GetObject(hbm, sizeof(bm), &bm);
	m_tabBottom = new CImageItem(4, 4, 4, 4, 0, hbm, IMAGE_FLAG_DIVIDED | IMAGE_PERPIXEL_ALPHA,
								 0, 255, 30, 80, 50, 100);

	m_tabBottom->setAlphaFormat(AC_SRC_ALPHA, 255);
	m_tabBottom->setMetrics(bm.bmWidth, bm.bmHeight);


	mir_sntprintf(tszFilename, MAX_PATH, _T("%scustom_tabskin_aero_glow.png"), tszBasePath);
	if (!PathFileExists(tszFilename))
		mir_sntprintf(tszFilename, MAX_PATH, _T("%stabskin_aero_glow.png"), tszBasePath);

	fib = (FIBITMAP *)CallService(MS_IMG_LOAD, (WPARAM)tszFilename, IMGL_TCHAR | IMGL_RETURNDIB);

	COLORREF glowColor = M.GetDword(FONTMODULE, "aeroGlow", RGB(40, 40, 255));
	hbm = FIF->FI_CreateHBITMAPFromDIB(fib);
	CImageItem::Colorize(hbm, GetRValue(glowColor), GetGValue(glowColor), GetBValue(glowColor));
	CImageItem::PreMultiply(hbm, 1);

	GetObject(hbm, sizeof(bm), &bm);
	m_tabGlowTop = new CImageItem(4, 4, 4, 4, 0, hbm, IMAGE_FLAG_DIVIDED | IMAGE_PERPIXEL_ALPHA,
								  0, 255, 30, 80, 50, 100);

	m_tabGlowTop->setAlphaFormat(AC_SRC_ALPHA, 255);
	m_tabGlowTop->setMetrics(bm.bmWidth, bm.bmHeight);

	FIF->FI_FlipVertical(fib);

	hbm = FIF->FI_CreateHBITMAPFromDIB(fib);
	CImageItem::Colorize(hbm, GetRValue(glowColor), GetGValue(glowColor), GetBValue(glowColor));
	CImageItem::PreMultiply(hbm, 1);
	FIF->FI_Unload(fib);

	GetObject(hbm, sizeof(bm), &bm);
	m_tabGlowBottom = new CImageItem(4, 4, 4, 4, 0, hbm, IMAGE_FLAG_DIVIDED | IMAGE_PERPIXEL_ALPHA,
									 0, 255, 30, 80, 50, 100);

	m_tabGlowBottom->setAlphaFormat(AC_SRC_ALPHA, 255);
	m_tabGlowBottom->setMetrics(bm.bmWidth, bm.bmHeight);

	/*
	 * background item for the button switch bar
	 */
	mir_sntprintf(tszFilename, MAX_PATH, _T("%scustom_tabskin_aero_button.png"), tszBasePath);
	if (!PathFileExists(tszFilename))
		mir_sntprintf(tszFilename, MAX_PATH, _T("%stabskin_aero_button.png"), tszBasePath);

	hbm  = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)tszFilename, IMGL_TCHAR);

	CImageItem::Colorize(hbm, GetRValue(m_dwmColorRGB),
					 	 GetGValue(m_dwmColorRGB),
					 	 GetBValue(m_dwmColorRGB));

	CImageItem::PreMultiply(hbm, 1);

	GetObject(hbm, sizeof(bm), &bm);

	m_switchBarItem = new CImageItem(4, 4, 4, 4, 0, hbm, IMAGE_FLAG_DIVIDED | IMAGE_PERPIXEL_ALPHA,
									 0, 255, 2, 12, 10, 20);

	m_switchBarItem->setAlphaFormat(AC_SRC_ALPHA, 255);
	m_switchBarItem->setMetrics(bm.bmWidth, bm.bmHeight);
}

/**
 * Calculate window frame borders for a skin with the ability to paint the window frame.
 * Uses system metrics to determine predefined window borders and caption bar size.
 */
void CSkin::SkinCalcFrameWidth()
{
	int xBorder, yBorder, yCaption;

	xBorder = GetSystemMetrics(SM_CXSIZEFRAME);
	yBorder = GetSystemMetrics(SM_CYSIZEFRAME);
	yCaption = GetSystemMetrics(SM_CYCAPTION);

	m_realSkinnedFrame_left = m_SkinnedFrame_left - xBorder;
	m_realSkinnedFrame_right = m_SkinnedFrame_right - xBorder;
	m_realSkinnedFrame_bottom = m_SkinnedFrame_bottom - yBorder;
	m_realSkinnedFrame_caption = m_SkinnedFrame_caption - yCaption;
}


/**
 * Draws part of the background to the foreground control
 *
 * @param hwndClient HWND: target window
 * @param hwnd       HWND: source window (usually the parent, needed for transforming client coordinates
 * @param pContainer ContainerWindowData *: needed to access the cached DC of the container window
 * @param rcClient   RECT *: client rectangle (target area)
 * @param hdcTarget  HDC: device context of the target window
 */
void CSkin::SkinDrawBG(HWND hwndClient, HWND hwnd, TContainerData *pContainer, RECT *rcClient, HDC hdcTarget)
{
	RECT rcWindow;
	POINT pt;
	LONG width = rcClient->right - rcClient->left;
	LONG height = rcClient->bottom - rcClient->top;
	HDC dc;

	::GetWindowRect(hwndClient, &rcWindow);
	pt.x = rcWindow.left + rcClient->left;
	pt.y = rcWindow.top + rcClient->top;
	::ScreenToClient(hwnd, &pt);
	if (pContainer)
		dc = pContainer->cachedDC;
	else
		dc = ::GetDC(hwnd);
	pt.y = max(pt.y, 0);
	::BitBlt(hdcTarget, rcClient->left, rcClient->top, width, height, dc, pt.x, pt.y, SRCCOPY);
	if (!pContainer)
		::ReleaseDC(hwnd, dc);
}

/**
 * Draws part of the background to the foreground control
 * same as above, but can use any source DC, not just the
 * container
 *
 * @param hwndClient HWND: target window
 * @param hwnd       HWND: source window (usually the parent, needed for transforming client coordinates
 * @param pContainer ContainerWindowData *: needed to access the cached DC of the container window
 * @param rcClient   RECT *: client rectangle (target area)
 * @param hdcTarget  HDC: device context of the target window
 */

void CSkin::SkinDrawBGFromDC(HWND hwndClient, HWND hwnd, RECT *rcClient, HDC hdcTarget)
{
	RECT rcWindow;
	POINT pt;
	LONG width = rcClient->right - rcClient->left;
	LONG height = rcClient->bottom - rcClient->top;
	HDC	 hdcSrc = ::GetDC(hwnd);

	::GetWindowRect(hwndClient, &rcWindow);
	pt.x = rcWindow.left + rcClient->left;
	pt.y = rcWindow.top + rcClient->top;
	::ScreenToClient(hwnd, &pt);
	::StretchBlt(hdcTarget, rcClient->left, rcClient->top, width, height, hdcSrc, pt.x, pt.y, width, height, SRCCOPY | CAPTUREBLT);
	::ReleaseDC(hwnd, hdcSrc);
}

/**
 * draw an icon "Dimmed" (small amount of transparency applied)
*/

void CSkin::DrawDimmedIcon(HDC hdc, LONG left, LONG top, LONG dx, LONG dy, HICON hIcon, BYTE alpha)
{
	HDC dcMem = ::CreateCompatibleDC(hdc);
	HBITMAP hbm = ::CreateCompatibleBitmap(hdc, dx, dy), hbmOld = 0;

	hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(dcMem, hbm));
	::DrawIconEx(dcMem, 0, 0, hIcon, dx, dy, 0, 0, DI_NORMAL);
	m_default_bf.SourceConstantAlpha = alpha;
	hbm = (HBITMAP)SelectObject(dcMem, hbmOld);
	CImageItem::PreMultiply(hbm, 1);						// for AlphaBlend()...
	hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(dcMem, hbm));
	GdiAlphaBlend(hdc, left, top, dx, dy, dcMem, 0, 0, dx, dy, m_default_bf);
	m_default_bf.SourceConstantAlpha = 255;
	SelectObject(dcMem, hbmOld);
	DeleteObject(hbm);
	DeleteDC(dcMem);
}

UINT CSkin::NcCalcRichEditFrame(HWND hwnd, const TWindowData *mwdat, UINT skinID, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROC MyWndProc)
{
	NCCALCSIZE_PARAMS *nccp = (NCCALCSIZE_PARAMS *)lParam;
	BOOL bReturn = FALSE;

	if (CSkin::m_DisableScrollbars) {
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_VSCROLL);
		EnableScrollBar(hwnd, SB_VERT, ESB_DISABLE_BOTH);
		ShowScrollBar(hwnd, SB_VERT, FALSE);
	}

	LRESULT orig = mir_callNextSubclass(hwnd, MyWndProc, msg, wParam, lParam);
	if (0 == mwdat)
		return orig;

	if (CSkin::m_skinEnabled) {
		CSkinItem *item = &SkinItems[skinID];
		if (!item->IGNORED)
			return WVR_REDRAW;
	}
	if (mwdat->hTheme && wParam) {
		RECT rcClient;
		HDC hdc = GetDC(GetParent(hwnd));

		if (GetThemeBackgroundContentRect(mwdat->hTheme, hdc, 1, 1, &nccp->rgrc[0], &rcClient) == S_OK) {
			if (EqualRect(&rcClient, &nccp->rgrc[0]))
				InflateRect(&rcClient, -1, -1);
			CopyRect(&nccp->rgrc[0], &rcClient);
			bReturn = TRUE;
		}
		ReleaseDC(GetParent(hwnd), hdc);
		if (bReturn)
			return WVR_REDRAW;
		else
			return orig;
	}
	if ((mwdat->sendMode & SMODE_MULTIPLE || mwdat->sendMode & SMODE_CONTAINER ||
				  mwdat->fEditNotesActive || mwdat->sendMode & SMODE_SENDLATER) && skinID == ID_EXTBKINPUTAREA) {
		InflateRect(&nccp->rgrc[0], -1, -1);
		return WVR_REDRAW;
	}
	return orig;
}

/*
 * process WM_NCPAINT for the rich edit control. Draw a visual style border and avoid classic static edge / client edge
 * may also draw a colorized border around the control
 */

UINT CSkin::DrawRichEditFrame(HWND hwnd, const TWindowData *mwdat, UINT skinID, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROC OldWndProc)
{
	// do default processing (otherwise, NO scrollbar as it is painted in NC_PAINT)
	LRESULT result = mir_callNextSubclass(hwnd, OldWndProc, msg, wParam, lParam);
	if (0 == mwdat)
		return result;

	BOOL isEditNotesReason = ((mwdat->fEditNotesActive) && (skinID == ID_EXTBKINPUTAREA));
	BOOL isSendLaterReason = ((mwdat->sendMode & SMODE_SENDLATER) && (skinID == ID_EXTBKINPUTAREA));
	BOOL isMultipleReason = ((skinID == ID_EXTBKINPUTAREA) && (mwdat->sendMode & SMODE_MULTIPLE || mwdat->sendMode & SMODE_CONTAINER));

	HDC hdc = GetWindowDC(hwnd);
	RECT rcWindow;
	POINT pt;
	LONG left_off, top_off, right_off, bottom_off;
	LONG dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	LONG dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

	GetWindowRect(hwnd, &rcWindow);
	pt.x = pt.y = 0;
	ClientToScreen(hwnd, &pt);
	left_off = pt.x - rcWindow.left;
	if (dwStyle & WS_VSCROLL && dwExStyle & WS_EX_RTLREADING)
		left_off -= PluginConfig.m_ncm.iScrollWidth;
	top_off = pt.y - rcWindow.top;

	CSkinItem *item = &SkinItems[skinID];
	if (CSkin::m_skinEnabled && !item->IGNORED) {
		right_off = item->MARGIN_RIGHT;
		bottom_off = item->MARGIN_BOTTOM;
	}
	else {
		right_off = left_off;
		bottom_off = top_off;
	}

	rcWindow.right -= rcWindow.left;
	rcWindow.bottom -= rcWindow.top;
	rcWindow.left = rcWindow.top = 0;

	ExcludeClipRect(hdc, left_off, top_off, rcWindow.right - right_off, rcWindow.bottom - bottom_off);
	if (CSkin::m_skinEnabled && !item->IGNORED) {
		ReleaseDC(hwnd, hdc);
		return result;
	}
	if (isMultipleReason || isEditNotesReason || isSendLaterReason) {
		HBRUSH br = CreateSolidBrush(isMultipleReason ? RGB(255, 130, 130) : (isEditNotesReason ? RGB(80, 255, 80) : RGB(80, 80, 255)));
		FillRect(hdc, &rcWindow, br);
		DeleteObject(br);
	} else {
		if (PluginConfig.m_cRichBorders) {
			HBRUSH br = CreateSolidBrush(PluginConfig.m_cRichBorders);
			FillRect(hdc, &rcWindow, br);
			DeleteObject(br);
		}
		else
			DrawThemeBackground(mwdat->hTheme, hdc, 1, 1, &rcWindow, &rcWindow);
	}
	ReleaseDC(hwnd, hdc);
	return result;
}

/**
 * convert a html-style color string (without the #) to a 32bit COLORREF value
 *
 * @param szSource TCHAR*: the color value as string. format:
 *  			   html-style without the leading #. e.g.
 *  			   "f3e355"
 *
 * @return COLORREF representation of the string value.
 */
DWORD __fastcall CSkin::HexStringToLong(const TCHAR *szSource)
{
	TCHAR *stopped;
	COLORREF clr = _tcstol(szSource, &stopped, 16);
	if (clr == -1)
		return clr;
	return(RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)));
}

/**
 * Render text to the given HDC. This function is aero aware and will use uxtheme DrawThemeTextEx() when needed.
 * Paramaters are pretty much comparable to GDI DrawText() API
 *
 * @return
 */

int CSkin::RenderText(HDC hdc, HANDLE hTheme, const TCHAR *szText, RECT *rc, DWORD dtFlags, const int iGlowSize, COLORREF clr, bool fForceAero)
{
	if ((PluginConfig.m_bIsVista && !CSkin::m_skinEnabled && hTheme) || fForceAero) {
		DTTOPTS dto = {0};
		dto.dwSize = sizeof(dto);
		if (iGlowSize && (M.isAero() || fForceAero)) {
			dto.iGlowSize = iGlowSize;
			dto.dwFlags = DTT_COMPOSITED|DTT_GLOWSIZE;
		}
		else {
			dto.dwFlags = DTT_TEXTCOLOR|DTT_COMPOSITED;//|DTT_SHADOWTYPE|DTT_SHADOWOFFSET|DTT_SHADOWCOLOR|DTT_BORDERSIZE|DTT_BORDERCOLOR;
			dto.crText = clr;
		}
		dto.iBorderSize = 10;
		return(CMimAPI::m_pfnDrawThemeTextEx(hTheme, hdc, BP_PUSHBUTTON, PBS_NORMAL, szText, -1, dtFlags, rc, &dto));
	}
	else {
		::SetTextColor(hdc, clr);
		return(::DrawText(hdc, szText, -1, rc, dtFlags));
	}
}

/**
 * Resize a bitmap using image service. The function does not care about the image aspect ratio.
 * The caller is responsible to submit proper values for the desired height and width.
 *
 * @param hBmpSrc  HBITMAP: the source bitmap
 * @param width    LONG: width of the destination bitmap
 * @param height   LONG: height of the new bitmap
 * @param mustFree bool: indicates that the new bitmap had been
                   *resized and either the source or destination
                   *bitmap should be freed.
 *
 * @return HBTIAMP: handle to a bitmap with the desired size.
 */
HBITMAP CSkin::ResizeBitmap(HBITMAP hBmpSrc, LONG width, LONG height, bool &mustFree)
{
	BITMAP	bm;

	GetObject(hBmpSrc, sizeof(bm), &bm);
	if (bm.bmHeight != height || bm.bmWidth != width) {
		::ResizeBitmap rb;
		rb.size = sizeof(rb);
		rb.fit = RESIZEBITMAP_STRETCH;
		rb.max_height = height;
		rb.max_width = width;
		rb.hBmp = hBmpSrc;

		HBITMAP hbmNew = (HBITMAP)CallService(MS_IMG_RESIZE, (WPARAM)&rb, 0);
		if (hbmNew != hBmpSrc)
			mustFree = true;
		return(hbmNew);
	}
	else {
		mustFree = false;
		return(hBmpSrc);
	}
}

/**
 * Draw the given skin item to the target rectangle and dc
 *
 * @param hdc    HDC: device context
 * @param rc     RECT: target rectangle
 * @param item   CSkinItem*: fully initialiized skin item
 *
 * @return bool: true if the item has been painted, false if not
 *  	   (only reason: the ignore flag in the item is set).
 */
bool __fastcall CSkin::DrawItem(const HDC hdc, const RECT *rc, const CSkinItem *item)
{
	if (!item->IGNORED) {
		::DrawAlpha(hdc, const_cast<RECT *>(rc), item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT,
				  item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
		return true;
	}
	return false;
}

/**
 * Create a 32bit RGBA bitmap, compatible for rendering with alpha channel.
 * Required by anything which would render on a transparent aero surface.
 * the image is a "bottom up" bitmap, as it has a negative
 * height. This is a requirement for some UxTheme APIs (e.g.
 * DrawThemeTextEx).
 *
 * @param rc     RECT &: the rectangle describing the target area.
 * @param dc     The device context for which the bitmap should be created.
 *
 * @return HBITMAP: handle to the bitmap created.
 */
HBITMAP CSkin::CreateAeroCompatibleBitmap(const RECT &rc, HDC dc)
{
	BITMAPINFO dib = {0};

	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    dib.bmiHeader.biWidth = rc.right - rc.left;
    dib.bmiHeader.biHeight = -(rc.bottom - rc.top);
    dib.bmiHeader.biPlanes = 1;
    dib.bmiHeader.biBitCount = 32;
    dib.bmiHeader.biCompression = BI_RGB;

    return(CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0 ));
}

/**
 * Map a given rectangle within the window, specified by hwndClient
 * to the client area of another window.
 *
 * @param hwndClient HWND: Client window
 * @param hwndParent HWND: The window to which the coordinates should be mapped
 * @param rc         RECT &: Rectangular area within the client area of hwndClient.
 *
                    *It will receive the transformed coordinates, relative to the client area of hwndParent
 */
void CSkin::MapClientToParent(HWND hwndClient, HWND hwndParent, RECT &rc)
{
	POINT pt;

	LONG  cx = rc.right - rc.left;
	LONG  cy = rc.bottom - rc.top;
	pt.x = rc.left; pt.y = rc.top;

	::ClientToScreen(hwndClient, &pt);
	::ScreenToClient(hwndParent, &pt);

	rc.top = pt.y;
	rc.left = pt.x;
	rc.right = rc.left + cx;
	rc.bottom = rc.top + cy;
}

/**
 * Draw the background for the message window tool bar
 *
 * @param dat      _MessageWindowData *: structure describing the message session
 *
 * @param hdc      HDC: handle to the device context in which painting should occur.
 * @param rcWindow RECT &: The window rectangle of the message dialog window
 */
void CSkin::RenderToolbarBG(const TWindowData *dat, HDC hdc, const RECT &rcWindow)
{
	if (dat) {
		if (dat->pContainer->dwFlags & CNT_HIDETOOLBAR)
			return;

		bool	 bAero = M.isAero();
		bool	 fTbColorsValid = PluginConfig.m_tbBackgroundHigh && PluginConfig.m_tbBackgroundLow;
		BYTE	 bAlphaOffset = 0;
		BOOL 	fMustDrawNonThemed = ((bAero || fTbColorsValid) && !M.GetByte(SRMSGMOD_T, "forceThemedToolbar", 0));
		RECT 	rc, rcToolbar;
		POINT	pt;

		if (!(dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR)) {
			::GetWindowRect(::GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_CHAT ? IDC_CHAT_LOG : IDC_LOG), &rc);
			pt.y = rc.bottom + 0;
			::ScreenToClient(dat->hwnd, &pt);
			rcToolbar.top = pt.y;
			rcToolbar.left = 0;
			rcToolbar.right = rcWindow.right;

			if (dat->bType == SESSIONTYPE_IM) {
				if (dat->dwFlags & MWF_ERRORSTATE)
					rcToolbar.top += ERRORPANEL_HEIGHT;
				if (dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED || dat->bNotOnList) {
					rcToolbar.top += 20;
					RECT	rcAdd;
					rcAdd.left = 0; rcAdd.right = rcToolbar.right - rcToolbar.left;
					rcAdd.bottom = rcToolbar.top - 1;
					rcAdd.top = rcAdd.bottom - 20;
					::DrawEdge(hdc, &rcAdd, BDR_RAISEDINNER | BDR_RAISEDOUTER, BF_RECT | BF_SOFT | BF_FLAT);
				}
			}

			::GetWindowRect(::GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_CHAT ? IDC_CHAT_MESSAGE : IDC_MESSAGE), &rc);
			pt.y = rc.top - (dat->bIsAutosizingInput ? 1 : 2);
			::ScreenToClient(dat->hwnd, &pt);
			rcToolbar.bottom = pt.y;
		}
		else {
			GetWindowRect(::GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_CHAT ? IDC_CHAT_MESSAGE : IDC_MESSAGE), &rc);
			pt.y = rc.bottom - (dat->bType == SESSIONTYPE_IM ? 2 : 2);
			ScreenToClient(dat->hwnd, &pt);
			rcToolbar.top = pt.y + 1;
			rcToolbar.left = 0;
			rcToolbar.right = rcWindow.right;
			rcToolbar.bottom = rcWindow.bottom;
		}
		LONG cx = rcToolbar.right - rcToolbar.left;
		LONG cy = rcToolbar.bottom - rcToolbar.top;

		RECT rcCachedToolbar = {0};
		rcCachedToolbar.right = cx;
		rcCachedToolbar.bottom = cy;

		if (dat->pContainer->cachedToolbarDC == 0)
			dat->pContainer->cachedToolbarDC = ::CreateCompatibleDC(hdc);

		if (dat->pContainer->szOldToolbarSize.cx != cx || dat->pContainer->szOldToolbarSize.cy != cy) {
			if (dat->pContainer->oldhbmToolbarBG) {
				::SelectObject(dat->pContainer->cachedToolbarDC, dat->pContainer->oldhbmToolbarBG);
				::DeleteObject(dat->pContainer->hbmToolbarBG);
			}
			dat->pContainer->hbmToolbarBG = CSkin::CreateAeroCompatibleBitmap(rcCachedToolbar, hdc);// ::CreateCompatibleBitmap(hdc, cx, cy);
			dat->pContainer->oldhbmToolbarBG = reinterpret_cast<HBITMAP>(::SelectObject(dat->pContainer->cachedToolbarDC, dat->pContainer->hbmToolbarBG));
		}
		dat->pContainer->szOldToolbarSize.cx = cx;
		dat->pContainer->szOldToolbarSize.cy = cy;

		if (!fMustDrawNonThemed && M.isVSThemed()) {
			DrawThemeBackground(dat->hThemeToolbar, dat->pContainer->cachedToolbarDC, 6, 1, &rcCachedToolbar, &rcCachedToolbar);
			dat->pContainer->bTBRenderingMode = 1;				// tell TSButton how to render the tool bar buttons
		}
		else {
			dat->pContainer->bTBRenderingMode = (M.isVSThemed() ? 1 : 0);
			m_tmp_tb_high = PluginConfig.m_tbBackgroundHigh ? PluginConfig.m_tbBackgroundHigh :
					((bAero && m_pCurrentAeroEffect) ? m_pCurrentAeroEffect->m_clrToolbar : ::GetSysColor(COLOR_3DFACE));
			m_tmp_tb_low = PluginConfig.m_tbBackgroundLow ? PluginConfig.m_tbBackgroundLow :
					((bAero && m_pCurrentAeroEffect) ? m_pCurrentAeroEffect->m_clrToolbar2 : ::GetSysColor(COLOR_3DFACE));

			bAlphaOffset = PluginConfig.m_tbBackgroundHigh ? 40 : 0;
			::DrawAlpha(dat->pContainer->cachedToolbarDC, &rcCachedToolbar, m_tmp_tb_high, 55 + bAlphaOffset, m_tmp_tb_low, 0, 9, 0, 0, 0);
		}

		::BitBlt(hdc, rcToolbar.left, rcToolbar.top, cx, cy,
				 dat->pContainer->cachedToolbarDC, 0, 0, SRCCOPY);
	}
}

/**
 * Initiate a buffered paint operation
 *
 * @param hdcSrc The source device context (usually obtained by BeginPaint())
 * @param rc     RECT&: the target rectangle that receives the painting
 * @param hdcOut HDC& (out) receives the buffered device context handle
 *
 * @return
 */
HANDLE CSkin::InitiateBufferedPaint(const HDC hdcSrc, RECT& rc, HDC& hdcOut)
{
	HANDLE hbp = CMimAPI::m_pfnBeginBufferedPaint(hdcSrc, &rc, BPBF_TOPDOWNDIB, NULL, &hdcOut);
	return(hbp);
}

/**
 * finalize buffered paint cycle and apply (if applicable) the global alpha value
 *
 * @param hbp    HANDLE: handle of the buffered paint context
 * @param rc     RECT*: target rectangly where alpha value should be applied
 */
void CSkin::FinalizeBufferedPaint(HANDLE hbp, RECT *rc)
{
	if (m_pCurrentAeroEffect && m_pCurrentAeroEffect->m_finalAlpha > 0)
		CMimAPI::m_pfnBufferedPaintSetAlpha(hbp, rc, m_pCurrentAeroEffect->m_finalAlpha);
	CMimAPI::m_pfnEndBufferedPaint(hbp, TRUE);
}
/**
 * Apply an effect to a aero glass area
 *
 * @param hdc    HDC: device context
 * @param rc     RECT*: target rectangle
 * @param iEffectArea
 *  			 int: area identifier (specifies which area we are drawing, so that allows to
 *  			 have different effects on different areas).
 *  			 Area can be the status bar, info panel, menu
 *  			 bar etc.
 * @param hbp    HANDLE: handle to a buffered paint identifier.
 *  			 default is none, needed forsome special
 *  			 effects. default paramenter is 0
 */

void CSkin::ApplyAeroEffect(const HDC hdc, const RECT *rc, int iEffectArea, HANDLE hbp)
{
	if (m_pCurrentAeroEffect == 0 || m_aeroEffect == AERO_EFFECT_NONE)
		return;

	if (m_pCurrentAeroEffect->pfnEffectRenderer)
		m_pCurrentAeroEffect->pfnEffectRenderer(hdc, rc, iEffectArea);
}

/** aero effect callbacks
 *
 */

void CSkin::AeroEffectCallback_Milk(const HDC hdc, const RECT *rc, int iEffectArea)
{
	if (iEffectArea < 0x1000) {
		int 	alpha = (iEffectArea == AERO_EFFECT_AREA_INFOPANEL) ? m_pCurrentAeroEffect->m_baseAlpha : 40;
		if (iEffectArea == AERO_EFFECT_AREA_MENUBAR)
			alpha = 90;
		BYTE 	color2_trans = (iEffectArea == AERO_EFFECT_AREA_MENUBAR) ? 0 : 1;
		DWORD   corner = (iEffectArea == AERO_EFFECT_AREA_INFOPANEL) ? m_pCurrentAeroEffect->m_cornerRadius : 6;

		DrawAlpha(hdc, const_cast<RECT *>(rc), m_pCurrentAeroEffect->m_baseColor, alpha, m_pCurrentAeroEffect->m_gradientColor,
				  color2_trans, m_pCurrentAeroEffect->m_gradientType, m_pCurrentAeroEffect->m_cornerType, corner, 0);
	}
}

void CSkin::AeroEffectCallback_Carbon(const HDC hdc, const RECT *rc, int iEffectArea)
{
	if (iEffectArea < 0x1000)
		DrawAlpha(hdc, const_cast<RECT *>(rc), m_pCurrentAeroEffect->m_baseColor, m_pCurrentAeroEffect->m_baseAlpha,
				  m_pCurrentAeroEffect->m_gradientColor, 0, m_pCurrentAeroEffect->m_gradientType,
				  m_pCurrentAeroEffect->m_cornerType, m_pCurrentAeroEffect->m_cornerRadius, 0);
}

void CSkin::AeroEffectCallback_Solid(const HDC hdc, const RECT *rc, int iEffectArea)
{
	if (iEffectArea < 0x1000) {
		if (iEffectArea == AERO_EFFECT_AREA_SIDEBAR_LEFT)
			::DrawAlpha(hdc, const_cast<RECT *>(rc), m_pCurrentAeroEffect->m_baseColor, m_pCurrentAeroEffect->m_baseAlpha,
						m_pCurrentAeroEffect->m_gradientColor, 0, GRADIENT_TB + 1,
						0, 2, 0);
		else
			::DrawAlpha(hdc, const_cast<RECT *>(rc), m_pCurrentAeroEffect->m_baseColor, m_pCurrentAeroEffect->m_baseAlpha,
						m_pCurrentAeroEffect->m_gradientColor, 0, m_pCurrentAeroEffect->m_gradientType,
						m_pCurrentAeroEffect->m_cornerType, m_pCurrentAeroEffect->m_cornerRadius, 0);
	}
	else {
		BYTE	bGradient = (iEffectArea & AERO_EFFECT_AREA_TAB_BOTTOM ? GRADIENT_BT : GRADIENT_TB) + 1;
		::DrawAlpha(hdc, const_cast<RECT *>(rc), m_pCurrentAeroEffect->m_baseColor, 70,
				  m_pCurrentAeroEffect->m_gradientColor, 1, bGradient,
				  m_pCurrentAeroEffect->m_cornerType, m_pCurrentAeroEffect->m_cornerRadius, 0);
	}
}

void CSkin::initAeroEffect()
{
	if (m_BrushBack) {
		::DeleteObject(m_BrushBack);
		m_BrushBack = 0;
	}
	if (PluginConfig.m_bIsVista && m_aeroEffect > AERO_EFFECT_NONE && m_aeroEffect < AERO_EFFECT_LAST) {
		m_currentAeroEffect = m_aeroEffects[m_aeroEffect];
		m_pCurrentAeroEffect = &m_currentAeroEffect;
		m_glowSize = m_pCurrentAeroEffect->m_glowSize;

		if (m_pCurrentAeroEffect->m_clrToolbar == -1)
			m_pCurrentAeroEffect->m_clrToolbar = PluginConfig.m_ipBackgroundGradientHigh;

		if (m_pCurrentAeroEffect->m_clrToolbar2 == -1)
			m_pCurrentAeroEffect->m_clrToolbar2 = PluginConfig.m_ipBackgroundGradient;
		else if (m_pCurrentAeroEffect->m_clrToolbar2 == 0)
			m_pCurrentAeroEffect->m_clrToolbar2 = m_dwmColorRGB;

		if (m_aeroEffect == AERO_EFFECT_CUSTOM || m_aeroEffect == AERO_EFFECT_SOLID) {
			m_pCurrentAeroEffect->m_baseColor = PluginConfig.m_ipBackgroundGradientHigh;
			m_pCurrentAeroEffect->m_gradientColor = PluginConfig.m_ipBackgroundGradient;
			if (m_aeroEffect == AERO_EFFECT_CUSTOM)
				m_pCurrentAeroEffect->m_clrBack = PluginConfig.m_ipBackgroundGradientHigh;
		}

		m_BrushBack = ::CreateSolidBrush(m_pCurrentAeroEffect->m_clrBack);
	} else {
		m_pCurrentAeroEffect = 0;
		m_glowSize = 10;
		m_BrushBack = ::CreateSolidBrush(0);
	}

	for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
		InvalidateRect(GetDlgItem(p->hwnd, IDC_MSGTABS), NULL, TRUE);
		InvalidateRect(p->hwnd, NULL, TRUE);
		if (IsWindow(GetDlgItem(p->hwnd, 5000)))
			InvalidateRect(GetDlgItem(p->hwnd, 5000), NULL, TRUE);
	}
}

void CSkin::setAeroEffect(LRESULT effect)
{
	if (effect == -1)
		effect = static_cast<LRESULT>(M.GetDword(SRMSGMOD_T, "aerostyle", AERO_EFFECT_NONE));

	if (effect >= 0 && effect < AERO_EFFECT_LAST)
		m_aeroEffect = (UINT)effect;
	else
		m_aeroEffect = AERO_EFFECT_NONE;

	initAeroEffect();
	db_set_dw(0, SRMSGMOD_T, "aerostyle", m_aeroEffect);
}

/**
 * extract the aero skin images from the DLL and store them in
 * the private data folder.
 * runs at every startup
 */
void CSkin::extractSkinsAndLogo(bool fForceOverwrite) const
{
	TCHAR tszBasePath[MAX_PATH];
	mir_sntprintf(tszBasePath, MAX_PATH, _T("%s"), M.getDataPath());
	if (tszBasePath[lstrlen(tszBasePath) - 1] != '\\')
		_tcscat(tszBasePath, _T("\\"));

	CreateDirectoryTreeT(tszBasePath);

	m_fAeroSkinsValid = true;

	for (int i=0; i < SIZEOF(my_default_skin); i++)
		if ( !Utils::extractResource(g_hInst, my_default_skin[i].ulID, _T("SKIN_GLYPH"), tszBasePath, my_default_skin[i].tszName, fForceOverwrite))
			m_fAeroSkinsValid = false;
}

/**
 * redraw the splitter area between the message input and message log
 * area only
 */
void CSkin::UpdateToolbarBG(TWindowData *dat, DWORD dwRdwOptFlags)
{
	RECT	rcUpdate, rcTmp;
	POINT	pt;

	if (dat) {
		::GetWindowRect(::GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG), &rcTmp);

		pt.x = rcTmp.left;
		pt.y = rcTmp.top;
		::ScreenToClient(dat->hwnd, &pt);

		rcUpdate.left = 0;
		rcUpdate.top = pt.y;

		::GetClientRect(dat->hwnd, &rcTmp);
		rcUpdate.right = rcTmp.right;
		rcUpdate.bottom = rcTmp.bottom;

		if (M.isAero() || M.isDwmActive())
			dat->fLimitedUpdate = true; 	// skip unrelevant window updates when we have buffered paint avail
		::RedrawWindow(dat->hwnd, &rcUpdate, 0, RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
		::BB_RedrawButtons(dat);
		dat->fLimitedUpdate = false;
	}
}

/**
 * fill a background area with the default color. This can be either the configured
 * fill color or default system color.
 *
 * @param hdc: device context
 * @param rc:  area to fill.
 */
void CSkin::FillBack(const HDC hdc, RECT* rc)
{
	if (0 == CSkin::m_BrushFill) {
		if (PluginConfig.m_fillColor)
			CSkin::m_BrushFill = ::CreateSolidBrush(PluginConfig.m_fillColor);
	}

	if (PluginConfig.m_fillColor)
		::FillRect(hdc, rc, CSkin::m_BrushFill);
	else
		::FillRect(hdc, rc, GetSysColorBrush(COLOR_3DFACE));
}
