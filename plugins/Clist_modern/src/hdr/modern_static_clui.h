/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

/************************************************************************/
/**     FILE CONTAINS HEADER PART FOR .../modernb/clui.c file          **/
/**                                                                    **/
/**         !!!  DO NOT INCLUDE IN TO OTHER FILES  !!!                 **/
/************************************************************************/

/* Definitions */
#pragma once

#define TM_AUTOALPHA                1
#define TM_DELAYEDSIZING            2
#define TM_BRINGOUTTIMEOUT          3
#define TM_BRINGINTIMEOUT           4
#define TM_UPDATEBRINGTIMER         5
#define TM_SMOTHALPHATRANSITION     20
#define TM_WINDOWUPDATE             100
#define TM_STATUSBARUPDATE          200

#define MS_CLUI_SHOWMAINMENU    "CList/ShowMainMenu"
#define MS_CLUI_SHOWSTATUSMENU  "CList/ShowStatusMenu"

#define AC_SRC_NO_PREMULT_ALPHA     0x01
#define AC_SRC_NO_ALPHA             0x02
#define AC_DST_NO_PREMULT_ALPHA     0x10
#define AC_DST_NO_ALPHA             0x20

#define ANIMATION_STEP              40

#define AEROGLASS_MINALPHA          24

/* Declaration of prototypes in other modules */

int ClcUnloadModule();
int ClcGetShortData(ClcData* pData, SHORTDATA *pShortData);
int ClcEnterDragToScroll(HWND hwnd, int Y);

int CListMod_ContactListShutdownProc(WPARAM wParam, LPARAM lParam);
int CListMod_HideWindow(HWND hwndContactList, int mode);

int CListSettings_GetCopyFromCache(ClcCacheEntry *pDest, DWORD flag);
int CListSettings_SetToCache(ClcCacheEntry *pSrc, DWORD flag);

int CLUIServices_LoadModule(void);
INT_PTR CLUIServices_SortList(WPARAM wParam, LPARAM lParam);

void Docking_GetMonitorRectFromWindow(HWND hWnd, RECT *rc);

int EventArea_Create(HWND hCluiWnd);

int ExtraImage_ExtraIDToColumnNum(int extra);

void GroupMenus_Init();

int ModernSkinButtonLoadModule();
int ModernSkinButton_ReposButtons(HWND parent, BYTE draw, RECT *r);

void ske_ApplyTranslucency();
HBITMAP ske_CreateDIB32(int cx, int cy);
HBITMAP ske_CreateDIB32Point(int cx, int cy, void ** bits);
int ske_JustUpdateWindowImage();
void ske_LoadSkinFromDB(void);
int ske_RedrawCompleteWindow();
int ske_UpdateWindowImage();
int ske_ValidateFrameImageProc(RECT *r);

HWND StatusBar_Create(HWND parent);

void RowHeight_InitModernRow();

int UnhookAll();

/* Module function prototypes */

int CLUI_IsInMainWindow(HWND hwnd);
int CLUI_SizingOnBorder(POINT pt, int size);
int CLUI_SmoothAlphaTransition(HWND hwnd, BYTE GoalAlpha, BOOL wParam);
int CLUI_TestCursorOnBorders();

static int CLUI_SmoothAlphaThreadTransition();

/*      structs         */

struct CHECKFILLING
{
	HDC hDC;
	RECT rcRect;
};

int CheckFramesPos(RECT *wr);			//cluiframes.c
int CLUIFrames_ApplyNewSizes(int mode); //cluiframes.c
int CLUIFrames_GetTotalHeight();		//cluiframes.c
int CLUIFrames_RepaintSubContainers();  //cluiframes.c
int CLUIFramesGetMinHeight();			//cluiframes.c

int SizeFramesByWindowRect(RECT *r, HDWP * PosBatch, int mode);	//cluiframes.c

int InitSkinHotKeys();
BOOL amWakeThread();
BOOL gtaWakeThread();
void CreateViewModeFrame();
