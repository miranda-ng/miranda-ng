#pragma once

#ifndef modern_statusbar_h__
#define modern_statusbar_h__

#include "modern_commonprototypes.h"
#include "../m_api/m_xpTheme.h"

int ModernDrawStatusBar(HWND hwnd, HDC hDC);
int ModernDrawStatusBarWorker(HWND hWnd, HDC hDC);

typedef struct tagSTATUSBARDATA 
{
  BOOL sameWidth;
  RECT rectBorders;
  BYTE extraspace;
  BYTE Align;
  BYTE VAlign;
  BYTE showProtoIcon;
  BYTE showProtoName;
  BYTE showStatusName;
  HFONT BarFont;
  DWORD fontColor;
  BYTE connectingIcon;
  BYTE TextEffectID;
  DWORD TextEffectColor1;
  DWORD TextEffectColor2;
  BYTE xStatusMode;     // 0-only main, 1-xStatus, 2-main as overlay
  BYTE nProtosPerLine;
  BYTE showProtoEmails;

  HBITMAP hBmpBackground;
  COLORREF bkColour;
  DWORD backgroundBmpUse;
  BOOL  bkUseWinColors;

  XPTHANDLE hTheme;

  BOOL perProtoConfig;
  BYTE SBarRightClk;

} STATUSBARDATA;

#endif // modern_statusbar_h__
