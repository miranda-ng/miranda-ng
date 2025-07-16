#pragma once

#ifndef modern_statusbar_h__
#define modern_statusbar_h__

#include "m_xpTheme.h"

int ModernDrawStatusBar(HWND hwnd, HDC hDC);
int ModernDrawStatusBarWorker(HWND hWnd, HDC hDC);

struct STATUSBARDATA
{
	RECT     rectBorders;
	int      extraspace;
	uint8_t  align;
	uint8_t  vAlign;
	bool     bSameWidth;
	bool     bShowProtoIcon;
	bool     bShowProtoName;
	bool     bShowStatusName;
	bool     bConnectingIcon;
	HFONT    BarFont;
	uint32_t fontColor;
	uint8_t  TextEffectID;
	uint32_t TextEffectColor1;
	uint32_t TextEffectColor2;
	uint8_t  xStatusMode;     // 0-only main, 1-xStatus, 2-main as overlay
	uint8_t  nProtosPerLine;
	bool     bShowProtoEmails;

	HBITMAP  hBmpBackground;
	COLORREF bkColour;
	uint32_t backgroundBmpUse;
	BOOL     bkUseWinColors;

	XPTHANDLE hTheme;

	BOOL     perProtoConfig;
	uint8_t  SBarRightClk;
};

#endif // modern_statusbar_h__
