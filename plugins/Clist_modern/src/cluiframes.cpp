/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "modern_sync.h"

// ALL THIS MODULE FUNCTION SHOULD BE EXECUTED FROM MAIN THREAD

#include "modern_static_cluiframes_service.h" // contain services initialization and proxiation

static const int UNCOLLAPSED_FRAME_SIZE = 0;
static const int DEFAULT_TITLEBAR_HEIGHT = 18;

void _AniAva_OnModulesUnload();
void InitGroupMenus(void);

// GLOBALS
FRAMEWND *g_pfwFrames = nullptr;
int g_nFramesCount = 0;
int g_nTitleBarHeight = DEFAULT_TITLEBAR_HEIGHT;

// STATIC

enum
{
	frame_menu_lock = 1,
	frame_menu_visible,
	frame_menu_showtitlebar,
	frame_menu_floating
}; //legacy menu support

enum { eUnknownId = -1 };

static int	_us_DoSetFrameFloat(WPARAM wParam, LPARAM lParam);
static int	CLUIFrameResizeFloatingFrame(int framepos);
static HWND CreateSubContainerWindow(HWND parent, int x, int y, int width, int height);
static BOOL CLUIFramesFitInSize();
static int  RemoveItemFromList(int pos, FRAMEWND **lpFrames, int *FrameItemCount);
static int  _us_DoSetFramePaintProc(WPARAM wParam, LPARAM lParam);

static mir_cs    _cluiFramesModuleCS;
static BOOL      _cluiFramesModuleCSInitialized = FALSE;
static BOOL      _fCluiFramesModuleNotStarted = TRUE;
static XPTHANDLE _hFrameTitleTheme = nullptr;
static int       _nClientFrameId = eUnknownId; //for fast access to frame with alclient properties
static int       _iNextFrameId = 100;
static HFONT     _hTitleBarFont = nullptr;

// menus
static FrameMenuHandles cont = {};
static LIST<TMO_IntMenuItem> g_frameMenus(10);

// others
static int _nContactListHeight = 0;
static uint32_t _dwLastStoreTick = 0;

HWND hWndExplorerToolBar;
static int GapBetweenFrames = 1;

BOOLEAN bMoveTogether;
int recurs_prevent = 0;
static BOOL sttPreventSizeCalling = FALSE;

static HBITMAP sttBmpBackground;
static int sttBackgroundBmpUse;
static COLORREF sttBkColour;
static COLORREF sttSelBkColour;
static BOOL sttBkUseWinColours;

uint8_t AlignCOLLIconToLeft; //will hide frame icon

//for old multiwindow
#define MPCF_CONTEXTFRAMEMENU		3
POINT ptOld;
short	nLeft = 0;
short	nTop = 0;

static int sortfunc(const void *a, const void *b)
{
	SortData *sd1, *sd2;
	sd1 = (SortData *)a;
	sd2 = (SortData *)b;
	if (sd1->order > sd2->order) { return(1); };
	if (sd1->order < sd2->order) return -1;;
	return (0);
};
int CLUIFrames_OnMoving(HWND hwnd, RECT *r)
{
	g_CluiData.mutexPreventDockMoving = 0;

	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating && F.OwnerWindow != nullptr  && F.OwnerWindow != (HWND)-2) {
			POINT pt = { 0 };
			RECT wr;

			GetWindowRect(hwnd, &wr);
			ClientToScreen(hwnd, &pt);
			int dx = (r->left - wr.left) + pt.x;
			int dy = (r->top - wr.top) + pt.y;
			int x = F.wndSize.left;
			int y = F.wndSize.top;
			SetWindowPos(F.OwnerWindow, nullptr, x + dx, y + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOOWNERZORDER);
		}
	}

	g_CluiData.mutexPreventDockMoving = 1;
	AniAva_RedrawAllAvatars(FALSE);
	return 0;
}

int SetAlpha(uint8_t Alpha)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating && F.OwnerWindow != nullptr  && F.OwnerWindow != (HWND)-2 && F.visible && !F.needhide) {
			HWND hwnd = F.OwnerWindow;
			long l = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			if (!(l & WS_EX_LAYERED)) {
				HWND parent = nullptr;
				if (g_CluiData.fOnDesktop) {
					HWND hProgMan = FindWindow(L"Progman", nullptr);
					if (IsWindow(hProgMan))
						parent = hProgMan;
				}

				CLUI_ShowWindowMod(hwnd, SW_HIDE);
				SetParent(hwnd, nullptr);
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, l | WS_EX_LAYERED);
				SetParent(hwnd, parent);
				if (l&WS_VISIBLE)  CLUI_ShowWindowMod(hwnd, SW_SHOW);
			}
			SetLayeredWindowAttributes(hwnd, g_CluiData.dwKeyColor, Alpha, LWA_ALPHA | LWA_COLORKEY);
		}
	}
	AniAva_RedrawAllAvatars(FALSE);
	return 0;
}


int CLUIFrames_RepaintSubContainers()
{
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating && F.OwnerWindow != (HWND)nullptr && F.OwnerWindow != (HWND)-2 && F.visible && !F.needhide)
			RedrawWindow(F.hWnd, nullptr, nullptr, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_INVALIDATE | RDW_FRAME);
	}

	return 0;
}

int CLUIFrames_ActivateSubContainers(BOOL active)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (active && !F.floating && F.OwnerWindow != (HWND)nullptr && F.OwnerWindow != (HWND)-2 && F.visible && !F.needhide) {
			if (g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT)) {
				SetWindowPos(F.OwnerWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				SetWindowPos(F.OwnerWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else SetWindowPos(F.OwnerWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	}
	return 0;
}

int CLUIFrames_SetParentForContainers(HWND parent)
{
	g_CluiData.fOnDesktop = (parent && parent != g_clistApi.hwndContactList);

	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating && F.OwnerWindow != (HWND)nullptr && F.OwnerWindow != (HWND)-2 && F.visible && !F.needhide) {
			HWND hwnd = F.OwnerWindow;
			SetParent(hwnd, parent);
		}
	}
	return 0;
}

int CLUIFrames_OnShowHide(int mode)
{
	int prevFrameCount;
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating && F.OwnerWindow != (HWND)nullptr && F.OwnerWindow != (HWND)-2) {
			// Try to avoid crash on exit due to unlock.
			HWND owner = F.OwnerWindow;
			HWND Frmhwnd = F.hWnd;
			BOOL visible = F.visible;
			BOOL needhide = F.needhide;
			needhide |= (!F.collapsed || F.height == 0);
			prevFrameCount = g_nFramesCount;
			ShowWindow(owner, (mode == SW_HIDE || !visible || needhide) ? SW_HIDE : mode);
			ShowWindow(Frmhwnd, (mode == SW_HIDE || !visible || needhide) ? SW_HIDE : mode);
		}

		if (mode != SW_HIDE) {
			SetWindowPos(F.OwnerWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			if (g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT)) {
				SetWindowPos(F.OwnerWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				SetWindowPos(F.OwnerWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else SetWindowPos(F.OwnerWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	}

	if (mode != SW_HIDE)
		SetForegroundWindow(g_clistApi.hwndContactList);
	AniAva_RedrawAllAvatars(TRUE);
	return 0;
}

static int RemoveItemFromList(int pos, FRAMEWND **lpFrames, int *FrameItemCount)
{
	memmove(&((*lpFrames)[pos]), &((*lpFrames)[pos + 1]), sizeof(FRAMEWND)*(*FrameItemCount - pos - 1));
	(*FrameItemCount)--;
	(*lpFrames) = (FRAMEWND*)realloc((*lpFrames), sizeof(FRAMEWND)*(*FrameItemCount));
	return 0;
}

static int id2pos(int id)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		if (g_pfwFrames[i].id == id)
			return i;

	return -1;
};

static int btoint(BOOLEAN b)
{
	if (b) return 1;
	return 0;
}


static FRAMEWND* FindFrameByWnd(HWND hwnd)
{
	int i;

	if (hwnd == nullptr) return(nullptr);

	for (i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (F.floating && F.ContainerWnd == hwnd)
			return &F;
	}

	return nullptr;
}


int QueueAllFramesUpdating(bool queue)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!g_CluiData.fLayered) {
			if (queue)
				InvalidateRect(F.hWnd, nullptr, FALSE);
			else
				ValidateRect(F.hWnd, nullptr);
		}
		if (F.PaintCallbackProc) {
			F.bQueued = queue;
			if (F.UpdateRgn) {
				DeleteObject(F.UpdateRgn);
			}
			F.UpdateRgn = nullptr;
		}
	}
	return queue;
}

int FindFrameID(HWND FrameHwnd)
{
	if (FrameHwnd == nullptr)
		return 0;

	FRAMEWND *frm = FindFrameByItsHWND(FrameHwnd);
	return (frm) ? frm->id : 0;
}

FRAMEWND* FindFrameByItsHWND(HWND FrameHwnd)
{
	if (FrameHwnd == nullptr) return(nullptr);
	for (int i = 0; i < g_nFramesCount; i++)
		if (g_pfwFrames[i].hWnd == FrameHwnd)
			return &g_pfwFrames[i];

	return nullptr;
}

static void DockThumbs(FRAMEWND *pThumbLeft, FRAMEWND *pThumbRight)
{
	if ((pThumbRight->dockOpt.hwndLeft == nullptr) && (pThumbLeft->dockOpt.hwndRight == nullptr)) {
		pThumbRight->dockOpt.hwndLeft = pThumbLeft->ContainerWnd;
		pThumbLeft->dockOpt.hwndRight = pThumbRight->ContainerWnd;
	}
}


static void UndockThumbs(FRAMEWND *pThumb1, FRAMEWND *pThumb2)
{
	if ((pThumb1 == nullptr) || (pThumb2 == nullptr))
		return;

	if (pThumb1->dockOpt.hwndRight == pThumb2->ContainerWnd)
		pThumb1->dockOpt.hwndRight = nullptr;

	if (pThumb1->dockOpt.hwndLeft == pThumb2->ContainerWnd)
		pThumb1->dockOpt.hwndLeft = nullptr;

	if (pThumb2->dockOpt.hwndRight == pThumb1->ContainerWnd)
		pThumb2->dockOpt.hwndRight = nullptr;

	if (pThumb2->dockOpt.hwndLeft == pThumb1->ContainerWnd)
		pThumb2->dockOpt.hwndLeft = nullptr;
}

static void PositionThumb(FRAMEWND *pThumb, short nX, short nY)
{
	FRAMEWND	*pCurThumb = &g_pfwFrames[0];
	FRAMEWND	*pDockThumb = pThumb;
	FRAMEWND	fakeMainWindow;
	FRAMEWND	fakeTaskBarWindow;
	RECT		rc;
	RECT		rcThumb;
	RECT		rcOld;
	int			nNewX;
	int			nNewY;
	int			nOffs = 10;
	int			nWidth;
	int			nHeight;
	POINT		pt;
	RECT		rcLeft;
	RECT		rcTop;
	RECT		rcRight;
	RECT		rcBottom;
	BOOL		bDocked;
	BOOL		bDockedLeft;
	BOOL		bDockedRight;
	BOOL		bLeading;
	int			frmidx = 0;

	if (pThumb == nullptr) return;

	SIZE sizeScreen;
	sizeScreen.cx = GetSystemMetrics(SM_CXSCREEN);
	sizeScreen.cy = GetSystemMetrics(SM_CYSCREEN);

	// Get thumb dimnsions
	GetWindowRect(pThumb->ContainerWnd, &rcThumb);
	nWidth = rcThumb.right - rcThumb.left;
	nHeight = rcThumb.bottom - rcThumb.top;

	// Docking to the edges of the screen
	nNewX = nX < nOffs ? 0 : nX;
	nNewX = nNewX > (sizeScreen.cx - nWidth - nOffs) ? (sizeScreen.cx - nWidth) : nNewX;
	nNewY = nY < nOffs ? 0 : nY;
	nNewY = nNewY > (sizeScreen.cy - nHeight - nOffs) ? (sizeScreen.cy - nHeight) : nNewY;

	bLeading = pThumb->dockOpt.hwndRight != nullptr;

	if (bMoveTogether) {
		UndockThumbs(pThumb, FindFrameByWnd(pThumb->dockOpt.hwndLeft));
		GetWindowRect(pThumb->ContainerWnd, &rcOld);
	}

	memset(&fakeMainWindow, 0, sizeof(fakeMainWindow));
	fakeMainWindow.ContainerWnd = g_clistApi.hwndContactList;
	fakeMainWindow.floating = TRUE;

	memset(&fakeTaskBarWindow, 0, sizeof(fakeTaskBarWindow));
	fakeTaskBarWindow.ContainerWnd = hWndExplorerToolBar;
	fakeTaskBarWindow.floating = TRUE;


	while (pCurThumb != nullptr) {
		if (pCurThumb->floating) {

			if (pCurThumb != pThumb) {
				GetWindowRect(pThumb->ContainerWnd, &rcThumb);
				OffsetRect(&rcThumb, nX - rcThumb.left, nY - rcThumb.top);

				GetWindowRect(pCurThumb->ContainerWnd, &rc);

				// These are rects we will dock into

				rcLeft.left = rc.left - nOffs;
				rcLeft.top = rc.top - nOffs;
				rcLeft.right = rc.left + nOffs;
				rcLeft.bottom = rc.bottom + nOffs;

				rcTop.left = rc.left - nOffs;
				rcTop.top = rc.top - nOffs;
				rcTop.right = rc.right + nOffs;
				rcTop.bottom = rc.top + nOffs;

				rcRight.left = rc.right - nOffs;
				rcRight.top = rc.top - nOffs;
				rcRight.right = rc.right + nOffs;
				rcRight.bottom = rc.bottom + nOffs;

				rcBottom.left = rc.left - nOffs;
				rcBottom.top = rc.bottom - nOffs;
				rcBottom.right = rc.right + nOffs;
				rcBottom.bottom = rc.bottom + nOffs;


				bDockedLeft = FALSE;
				bDockedRight = FALSE;

				// Upper-left
				pt.x = rcThumb.left;
				pt.y = rcThumb.top;
				bDocked = FALSE;

				if (PtInRect(&rcRight, pt)) {
					nNewX = rc.right;
					bDocked = TRUE;
				}

				if (PtInRect(&rcBottom, pt)) {
					nNewY = rc.bottom;

					if (PtInRect(&rcLeft, pt)) {
						nNewX = rc.left;
					}
				}

				if (PtInRect(&rcTop, pt)) {
					nNewY = rc.top;
					bDockedLeft = bDocked;
				}

				// Upper-right
				pt.x = rcThumb.right;
				pt.y = rcThumb.top;
				bDocked = FALSE;

				if (!bLeading && PtInRect(&rcLeft, pt)) {
					if (!bDockedLeft) {
						nNewX = rc.left - nWidth;
						bDocked = TRUE;
					}
					else if (rc.right == rcThumb.left) {
						bDocked = TRUE;
					}
				}


				if (PtInRect(&rcBottom, pt)) {
					nNewY = rc.bottom;

					if (PtInRect(&rcRight, pt)) {
						nNewX = rc.right - nWidth;
					}
				}

				if (!bLeading && PtInRect(&rcTop, pt)) {
					nNewY = rc.top;
					bDockedRight = bDocked;
				}

				if (bMoveTogether) {
					if (bDockedRight) {
						DockThumbs(pThumb, pCurThumb);
					}

					if (bDockedLeft) {
						DockThumbs(pCurThumb, pThumb);
					}
				}

				// Lower-left
				pt.x = rcThumb.left;
				pt.y = rcThumb.bottom;

				if (PtInRect(&rcRight, pt)) {
					nNewX = rc.right;
				}

				if (PtInRect(&rcTop, pt)) {
					nNewY = rc.top - nHeight;

					if (PtInRect(&rcLeft, pt)) {
						nNewX = rc.left;
					}
				}


				// Lower-right
				pt.x = rcThumb.right;
				pt.y = rcThumb.bottom;

				if (!bLeading && PtInRect(&rcLeft, pt)) {
					nNewX = rc.left - nWidth;
				}

				if (!bLeading && PtInRect(&rcTop, pt)) {
					nNewY = rc.top - nHeight;

					if (PtInRect(&rcRight, pt)) {
						nNewX = rc.right - nWidth;
					}
				}
			}

		};
		frmidx++;
		if (pCurThumb->ContainerWnd == fakeTaskBarWindow.ContainerWnd) { break; };
		if (pCurThumb->ContainerWnd == fakeMainWindow.ContainerWnd) {
			pCurThumb = &fakeTaskBarWindow; continue;
		}
		if (frmidx == g_nFramesCount) {
			pCurThumb = &fakeMainWindow; continue;
		}

		pCurThumb = &g_pfwFrames[frmidx];
	}

	// Adjust coords once again
	nNewX = nNewX < nOffs ? 0 : nNewX;
	nNewX = nNewX > (sizeScreen.cx - nWidth - nOffs) ? (sizeScreen.cx - nWidth) : nNewX;
	nNewY = nNewY < nOffs ? 0 : nNewY;
	nNewY = nNewY > (sizeScreen.cy - nHeight - nOffs) ? (sizeScreen.cy - nHeight) : nNewY;


	SetWindowPos(pThumb->ContainerWnd,
		HWND_TOPMOST,
		nNewX,
		nNewY,
		0,
		0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);


	// OK, move all g_CluiData.fDocked thumbs
	if (bMoveTogether) {
		pDockThumb = FindFrameByWnd(pDockThumb->dockOpt.hwndRight);

		PositionThumb(pDockThumb, (short)(nNewX + nWidth), (short)nNewY);
	}
}



static void GetBorderSize(HWND hwnd, RECT *rect)
{
	RECT wr, cr;
	POINT pt1, pt2;
	//  RECT r = {0};
	//  *rect = r;
	//  return;
	GetWindowRect(hwnd, &wr);
	GetClientRect(hwnd, &cr);
	pt1.y = cr.top; pt1.x = cr.left;
	pt2.y = cr.bottom; pt2.x = cr.right;

	ClientToScreen(hwnd, &pt1);
	ClientToScreen(hwnd, &pt2);

	cr.top = pt1.y; cr.left = pt1.x;
	cr.bottom = pt2.y; cr.right = pt2.x;

	rect->top = cr.top - wr.top;
	rect->left = cr.left - wr.left;
	rect->right = wr.right - cr.right;
	rect->bottom = wr.bottom - cr.bottom;
	//if (rect->top+rect->bottom>10){rect->top = rect->bottom = 2;};
	//if (rect->left+rect->right>10){rect->top = rect->bottom = 2;};

};

static int LocateStorePosition(int Frameid, int maxstored)
{
	if (g_pfwFrames[Frameid].name == nullptr)
		return -1;

	char settingname[255];
	for (int i = 0; i < maxstored; i++) {
		mir_snprintf(settingname, "Name%d", i);
		DBVARIANT dbv = { 0 };
		if (db_get_ws(0, CLUIFrameModule, settingname, &dbv))
			continue;

		if (mir_wstrcmpi(dbv.pwszVal, g_pfwFrames[Frameid].name) == 0) {
			db_free(&dbv);
			return i;
		}
		db_free(&dbv);
	}
	return -1;
}

static int CLUIFramesLoadFrameSettings(int Frameid)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	if (Frameid < 0 || Frameid >= g_nFramesCount)
		return -1;

	int maxstored = db_get_w(0, CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1)
		return 0;

	int storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1)
		return 0;

	FRAMEWND &F = g_pfwFrames[Frameid];
	CMStringA buf;
	F.collapsed = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Collapse%d", storpos), F.collapsed);

	F.Locked = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Locked%d", storpos), F.Locked);
	F.visible = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Visible%d", storpos), F.visible);
	F.TitleBar.ShowTitleBar = 0 != db_get_b(0, CLUIFrameModule, buf.Format("TBVisile%d", storpos), F.TitleBar.ShowTitleBar);

	F.height = db_get_w(0, CLUIFrameModule, buf.Format("Height%d", storpos), F.height);
	F.HeightWhenCollapsed = db_get_w(0, CLUIFrameModule, buf.Format("HeightCollapsed%d", storpos), 0);
	F.align = db_get_w(0, CLUIFrameModule, buf.Format("Align%d", storpos), F.align);

	F.FloatingPos.x = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatX%d", storpos), 100, 0, 2048);
	F.FloatingPos.y = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatY%d", storpos), 100, 0, 2048);
	F.FloatingSize.x = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatW%d", storpos), 100, 0, 2048);
	F.FloatingSize.y = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatH%d", storpos), 100, 0, 2048);

	F.floating = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Floating%d", storpos), 0);
	F.order = db_get_w(0, CLUIFrameModule, buf.Format("Order%d", storpos), 0);

	F.UseBorder = 0 != db_get_b(0, CLUIFrameModule, buf.Format("UseBorder%d", storpos), F.UseBorder);
	return 0;
}

static int CLUIFramesStoreFrameSettings(int Frameid)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	if (Frameid < 0 || Frameid >= g_nFramesCount)
		return -1;

	int maxstored = db_get_w(0, CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1)
		maxstored = 0;

	int storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1) {
		storpos = maxstored;
		maxstored++;
	}

	FRAMEWND &F = g_pfwFrames[Frameid];
	CMStringA buf;
	db_set_ws(0, CLUIFrameModule, buf.Format("Name%d", storpos), F.name);

	db_set_b(0, CLUIFrameModule, buf.Format("Collapse%d", storpos), (uint8_t)btoint(F.collapsed));
	db_set_b(0, CLUIFrameModule, buf.Format("Locked%d", storpos), (uint8_t)btoint(F.Locked));
	db_set_b(0, CLUIFrameModule, buf.Format("Visible%d", storpos), (uint8_t)btoint(F.visible));
	db_set_b(0, CLUIFrameModule, buf.Format("TBVisile%d", storpos), (uint8_t)btoint(F.TitleBar.ShowTitleBar));

	db_set_w(0, CLUIFrameModule, buf.Format("Height%d", storpos), (uint16_t)F.height);
	db_set_w(0, CLUIFrameModule, buf.Format("HeightCollapsed%d", storpos), (uint16_t)F.HeightWhenCollapsed);
	db_set_w(0, CLUIFrameModule, buf.Format("Align%d", storpos), (uint16_t)F.align);

	db_set_w(0, CLUIFrameModule, buf.Format("FloatX%d", storpos), (uint16_t)F.FloatingPos.x);
	db_set_w(0, CLUIFrameModule, buf.Format("FloatY%d", storpos), (uint16_t)F.FloatingPos.y);
	db_set_w(0, CLUIFrameModule, buf.Format("FloatW%d", storpos), (uint16_t)F.FloatingSize.x);
	db_set_w(0, CLUIFrameModule, buf.Format("FloatH%d", storpos), (uint16_t)F.FloatingSize.y);

	db_set_b(0, CLUIFrameModule, buf.Format("Floating%d", storpos), (uint8_t)btoint(F.floating));
	db_set_b(0, CLUIFrameModule, buf.Format("UseBorder%d", storpos), (uint8_t)btoint(F.UseBorder));
	db_set_w(0, CLUIFrameModule, buf.Format("Order%d", storpos), (uint16_t)F.order);

	db_set_w(0, CLUIFrameModule, "StoredFrames", (uint16_t)maxstored);
	return 0;
}

static int CLUIFramesStoreAllFrames()
{
	if (_fCluiFramesModuleNotStarted) return -1;
	for (int i = 0; i < g_nFramesCount; i++)
		CLUIFramesStoreFrameSettings(i);
	return 0;
}

static int CLUIFramesGetalClientFrame(void)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	if (_nClientFrameId != eUnknownId) {
		/* this value could become invalid if RemoveItemFromList was called,
		* so we double-check */
		if (_nClientFrameId < g_nFramesCount) {
			if (g_pfwFrames[_nClientFrameId].align == alClient) {
				return _nClientFrameId;
			}
		}
	}

	for (int i = 0; i < g_nFramesCount; i++)
		if (g_pfwFrames[i].align == alClient) {
			_nClientFrameId = i;
			return i;
		}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static HGENMENU addFrameMenuItem(TMO_MenuItem *pmi, int frameid, bool bMain)
{
	HGENMENU res = (bMain) ? Menu_AddMainMenuItem(pmi) : Menu_AddContextFrameMenuItem(pmi);
	if (pmi->pszService != nullptr)
		Menu_ConfigureItem(res, MCI_OPT_EXECPARAM, frameid);
	return res;
}

static HMENU CLUIFramesCreateMenuForFrame(int frameid, HGENMENU root, int popuppos, bool bMain)
{
	if (_fCluiFramesModuleNotStarted)
		return nullptr;

	int framepos = id2pos(frameid);
	FrameMenuHandles &fmh = (frameid == -1) ? cont : g_pfwFrames[framepos].MenuHandles;

	CMenuItem mi(framepos != -1 ? g_pfwFrames[framepos].pPlugin : &g_plugin);
	mi.root = root;
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Visible");
	mi.flags = CMIF_SYSTEM | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SHFRAME;
	fmh.MIVisible = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Show title");
	mi.pszService = MS_CLIST_FRAMES_SHFRAMETITLEBAR;
	fmh.MITBVisible = addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Locked");
	mi.pszService = MS_CLIST_FRAMES_ULFRAME;
	fmh.MILock = addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Expanded");
	mi.pszService = MS_CLIST_FRAMES_UCOLLFRAME;
	fmh.MIColl = addFrameMenuItem(&mi, frameid, bMain);

	// floating
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Floating mode");
	mi.flags = CMIF_SYSTEM;
	mi.pszService = "Set_Floating";
	fmh.MIFloating = addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Border");
	mi.flags = CMIF_SYSTEM | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SETUNBORDER;
	fmh.MIBorder = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	// alignment root
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Align");
	mi.flags = CMIF_SYSTEM;
	mi.pszService = nullptr;
	fmh.MIAlignRoot = addFrameMenuItem(&mi, frameid, bMain);

	// align top
	mi.root = fmh.MIAlignRoot;
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Top");
	mi.pszService = CLUIFRAMESSETALIGNALTOP;
	fmh.MIAlignTop = addFrameMenuItem(&mi, frameid, bMain);

	// align client
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Client");
	mi.pszService = CLUIFRAMESSETALIGNALCLIENT;
	fmh.MIAlignClient = addFrameMenuItem(&mi, frameid, bMain);

	// align bottom
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Bottom");
	mi.pszService = CLUIFRAMESSETALIGNALBOTTOM;
	fmh.MIAlignBottom = addFrameMenuItem(&mi, frameid, bMain);

	// position root
	mi.root = root;
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Position");
	mi.pszService = nullptr;
	mi.root = addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Up");
	mi.pszService = CLUIFRAMESMOVEUP;
	addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Down");
	mi.pszService = CLUIFRAMESMOVEDOWN;
	addFrameMenuItem(&mi, frameid, bMain);
	return nullptr;
}

static int CLUIFramesModifyContextMenuForFrame(WPARAM wParam, LPARAM)
{
	/* HOOK */
	if (MirandaExiting()) return 0;
	if (_fCluiFramesModuleNotStarted) return -1;
	int pos = id2pos(wParam);
	if (pos >= 0 && pos < g_nFramesCount) {
		FRAMEWND &p = g_pfwFrames[pos];

		Menu_ModifyItem(cont.MIVisible, p.TitleBar.tbname ? p.TitleBar.tbname : p.name);
		Menu_SetChecked(cont.MIVisible, p.visible);
		Menu_SetChecked(cont.MILock, p.Locked);
		Menu_SetChecked(cont.MITBVisible, p.TitleBar.ShowTitleBar);
		Menu_SetChecked(cont.MIFloating, p.floating);

		if (g_CluiData.fLayered)
			Menu_EnableItem(cont.MIBorder, false);
		else
			Menu_SetChecked(cont.MIBorder, p.UseBorder);

		Menu_SetChecked(cont.MIAlignTop, (p.align & alTop) != 0);
		Menu_SetChecked(cont.MIAlignClient, (p.align & alClient) != 0);
		Menu_SetChecked(cont.MIAlignBottom, (p.align & alBottom) != 0);

		Menu_SetChecked(cont.MIColl, p.collapsed);
		Menu_EnableItem(cont.MIColl, p.visible && !p.Locked && pos != CLUIFramesGetalClientFrame());
	}
	return 0;
}

static int CLUIFramesModifyMainMenuItems(WPARAM wParam, LPARAM)
{
	// hiword(wParam) = frameid,loword(wParam) = flag
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = id2pos(wParam);
	if (pos >= 0 && pos < g_nFramesCount) {
		FRAMEWND &p = g_pfwFrames[pos];

		Menu_ModifyItem(p.MenuHandles.MIVisible, p.TitleBar.tbname ? p.TitleBar.tbname : p.name);
		Menu_SetChecked(p.MenuHandles.MIVisible, p.visible);
		Menu_SetChecked(p.MenuHandles.MILock, p.Locked);
		Menu_SetChecked(p.MenuHandles.MITBVisible, p.TitleBar.ShowTitleBar);
		Menu_SetChecked(p.MenuHandles.MIFloating, p.floating);

		if (g_CluiData.fLayered)
			Menu_EnableItem(p.MenuHandles.MIBorder, false);
		else
			Menu_SetChecked(p.MenuHandles.MIBorder, p.UseBorder);

		Menu_EnableItem(p.MenuHandles.MIAlignTop, (p.align & alClient) == 0);
		Menu_SetChecked(p.MenuHandles.MIAlignTop, (p.align & alTop) != 0);
		Menu_EnableItem(p.MenuHandles.MIAlignBottom, (p.align & alClient) == 0);
		Menu_SetChecked(p.MenuHandles.MIAlignBottom, (p.align & alBottom) != 0);
		Menu_SetChecked(p.MenuHandles.MIAlignClient, (p.align & alClient) != 0);

		Menu_SetChecked(p.MenuHandles.MIColl, p.collapsed);
		Menu_EnableItem(p.MenuHandles.MIColl, p.visible && !p.Locked && pos != CLUIFramesGetalClientFrame());
	}

	return 0;
}


static INT_PTR _us_DoGetFrameOptions(WPARAM wParam, LPARAM)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = id2pos(HIWORD(wParam));
	if (pos < 0 || pos >= g_nFramesCount)
		return -1;

	FRAMEWND &F = g_pfwFrames[pos];
	switch (LOWORD(wParam) & ~FO_UNICODETEXT) {
	case FO_NAME:
		return (INT_PTR)F.name;

	case FO_TBNAME:
		return (INT_PTR)F.TitleBar.tbname;

	case FO_TBTIPNAME:
		return (INT_PTR)F.TitleBar.tooltip;

	case FO_TBSTYLE:
		return GetWindowLongPtr(F.TitleBar.hwnd, GWL_STYLE);

	case FO_TBEXSTYLE:
		return GetWindowLongPtr(F.TitleBar.hwnd, GWL_EXSTYLE);

	case FO_ICON:
		return (INT_PTR)F.TitleBar.hicon;

	case FO_HEIGHT:
		return F.height;

	case FO_ALIGN:
		return F.align;

	case FO_FLOATING:
		return F.floating;

	case FO_FLAGS:
		INT_PTR retval = 0;
		if (F.visible) retval |= F_VISIBLE;
		if (!F.collapsed) retval |= F_UNCOLLAPSED;
		if (F.Locked) retval |= F_LOCKED;
		if (F.TitleBar.ShowTitleBar) retval |= F_SHOWTB;
		if (F.TitleBar.ShowTitleBarTip) retval |= F_SHOWTBTIP;
		if (!g_CluiData.fLayered) {
			if (!(GetWindowLongPtr(F.hWnd, GWL_STYLE)&WS_BORDER))
				retval |= F_NOBORDER;
		}
		else if (!F.UseBorder)
			retval |= F_NOBORDER;
		return retval;
	}

	return -1;
}

static int UpdateTBToolTip(int framepos)
{
	TOOLINFO ti = { sizeof(ti) };
	ti.lpszText = g_pfwFrames[framepos].TitleBar.tooltip;
	ti.hinst = g_plugin.getInst();
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId = (UINT_PTR)g_pfwFrames[framepos].TitleBar.hwnd;
	return SendMessage(g_pfwFrames[framepos].TitleBar.hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
}

//hiword(wParam) = frameid,loword(wParam) = flag
static int _us_DoSetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	int retval; // value to be returned
	BOOL bUnicodeText = (LOWORD(wParam) & FO_UNICODETEXT) != 0;
	wParam = MAKEWPARAM((LOWORD(wParam))& ~FO_UNICODETEXT, HIWORD(wParam));
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = id2pos(HIWORD(wParam));
	if (pos < 0 || pos >= g_nFramesCount)
		return -1;

	FRAMEWND &fw = g_pfwFrames[pos];

	switch (LOWORD(wParam)) {
	case FO_FLAGS:
		fw.dwFlags = lParam;
		fw.visible = FALSE;
		if (lParam & F_VISIBLE) fw.visible = TRUE;

		fw.collapsed = true;
		if (lParam & F_UNCOLLAPSED) fw.collapsed = false;

		fw.Locked = FALSE;
		if (lParam & F_LOCKED) fw.Locked = TRUE;

		fw.UseBorder = TRUE;
		if (lParam & F_NOBORDER) fw.UseBorder = FALSE;

		fw.TitleBar.ShowTitleBar = FALSE;
		if (lParam & F_SHOWTB) fw.TitleBar.ShowTitleBar = TRUE;

		fw.TitleBar.ShowTitleBarTip = FALSE;
		if (lParam & F_SHOWTBTIP) fw.TitleBar.ShowTitleBarTip = TRUE;

		SendMessageA(fw.TitleBar.hwndTip, TTM_ACTIVATE, (WPARAM)fw.TitleBar.ShowTitleBarTip, 0);
		{
			LONG_PTR style = GetWindowLongPtr(fw.hWnd, GWL_STYLE);
			style &= (~WS_BORDER);
			if (!(lParam & F_NOBORDER) && !g_CluiData.fLayered)
				style |= WS_BORDER;

			SetWindowLongPtr(fw.hWnd, GWL_STYLE, style);
			SetWindowLongPtr(fw.TitleBar.hwnd, GWL_STYLE, style& ~(WS_VSCROLL | WS_HSCROLL));
		}

		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		SetWindowPos(fw.TitleBar.hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE);
		return 0;

	case FO_NAME:
		if (lParam == 0)
			return -1;

		mir_free(fw.name);
		if (bUnicodeText)
			fw.name = mir_wstrdup((LPTSTR)lParam);
		else
			fw.name = mir_a2u((char *)lParam);
		return 0;

	case FO_TBNAME:
		if (lParam == 0)
			return -1;

		mir_free(fw.TitleBar.tbname);
		if (bUnicodeText)
			fw.TitleBar.tbname = mir_wstrdup((LPTSTR)lParam);
		else
			fw.TitleBar.tbname = mir_a2u((char*)lParam);

		if (fw.floating && (fw.TitleBar.tbname != nullptr))
			SetWindowText(fw.ContainerWnd, TranslateW_LP(fw.TitleBar.tbname, fw.pPlugin));
		return 0;

	case FO_TBTIPNAME:
		if (lParam == 0)
			return -1;

		if (fw.TitleBar.tooltip != nullptr)
			replaceStrW(fw.TitleBar.tooltip, nullptr);
		if (bUnicodeText)
			fw.TitleBar.tooltip = mir_wstrdup((LPTSTR)lParam);
		else
			fw.TitleBar.tooltip = mir_a2u((char*)lParam);

		UpdateTBToolTip(pos);
		return 0;

	case FO_TBSTYLE:
		SetWindowLongPtr(fw.TitleBar.hwnd, GWL_STYLE, lParam& ~(WS_VSCROLL | WS_HSCROLL));
		return 0;

	case FO_TBEXSTYLE:
		SetWindowLongPtr(fw.TitleBar.hwnd, GWL_EXSTYLE, lParam);
		return 0;

	case FO_ICON:
		fw.TitleBar.hicon = (HICON)lParam;
		return 0;

	case FO_HEIGHT:
		if (lParam < 0) { return -1; }

		if (fw.collapsed) {
			int oldHeight = fw.height;
			retval = fw.height;
			fw.height = lParam;
			if (!CLUIFramesFitInSize()) fw.height = retval;
			retval = fw.height;
			if (fw.height != oldHeight && !fw.floating)
				CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		}
		else {
			retval = fw.HeightWhenCollapsed;
			fw.HeightWhenCollapsed = lParam;
			if (!CLUIFramesFitInSize()) fw.HeightWhenCollapsed = retval;
			retval = fw.HeightWhenCollapsed;

		}
		return retval;

	case FO_FLOATING:
		if (lParam < 0) { return -1; }

		{
			int id = fw.id;
			fw.floating = !(lParam);


			CLUIFrames_SetFrameFloat(id, 1);//lparam = 1 use stored width and height
			return(wParam);
		}

	case FO_ALIGN:
		if (!(lParam&alTop || lParam&alBottom || lParam&alClient)) {
			TRACE("Wrong align option \r\n");
			return (-1);
		};

		if ((lParam&alClient) && (CLUIFramesGetalClientFrame() >= 0)) {	//only one alClient frame possible
			_nClientFrameId = eUnknownId;//recalc it

			return -1;
		}
		fw.align = lParam;
		return 0;
	}

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return -1;
}

//wparam = lparam = 0
static int _us_DoShowAllFrames(WPARAM, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		g_pfwFrames[i].visible = TRUE;
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return 0;
}

//wparam = lparam = 0
static int _us_DoShowTitles(WPARAM, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		g_pfwFrames[i].TitleBar.ShowTitleBar = TRUE;
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return 0;
}

//wparam = lparam = 0
static int _us_DoHideTitles(WPARAM, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		g_pfwFrames[i].TitleBar.ShowTitleBar = FALSE;
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return 0;
}

//wparam = frameid
static int _us_DoShowHideFrame(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = (wParam == 0) ? lParam : id2pos(wParam);
	if (pos >= 0 && pos < g_nFramesCount) {
		auto &F = g_pfwFrames[pos];
		F.visible = !F.visible;
		CLUIFramesStoreFrameSettings(pos);

		if (F.OwnerWindow != (HWND)-2) {
			if (F.OwnerWindow)
				CLUI_ShowWindowMod(F.OwnerWindow, (F.visible &&  F.collapsed && IsWindowVisible(g_clistApi.hwndContactList)) ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
			else if (F.visible) {
				F.OwnerWindow = CreateSubContainerWindow(g_clistApi.hwndContactList, F.FloatingPos.x, F.FloatingPos.y, 10, 10);
				SetParent(F.hWnd, F.OwnerWindow);
				CLUI_ShowWindowMod(F.OwnerWindow, (F.visible && F.collapsed && IsWindowVisible(g_clistApi.hwndContactList)) ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
			}
		}

		if (F.floating)
			CLUIFrameResizeFloatingFrame(pos);

		if (!F.floating)
			CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	}
	return 0;
}

//wparam = frameid
static int _us_DoShowHideFrameTitle(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = (wParam == 0) ? lParam : id2pos(wParam);
	if (pos >= 0 && (int)pos < g_nFramesCount)
		g_pfwFrames[pos].TitleBar.ShowTitleBar = !g_pfwFrames[pos].TitleBar.ShowTitleBar;

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return 0;
}

//wparam = frameid
//lparam = -1 up ,1 down
static int _us_DoMoveFrame(WPARAM wParam, LPARAM lParam)
{
	int i, curpos, curalign, v, tmpval;

	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = id2pos(wParam);
	if (pos >= 0 && (int)pos < g_nFramesCount) {
		curpos = g_pfwFrames[pos].order;
		curalign = g_pfwFrames[pos].align;
		v = 0;
		SortData *sd = (SortData*)malloc(sizeof(SortData)*g_nFramesCount);
		memset(sd, 0, sizeof(SortData)*g_nFramesCount);
		for (i = 0; i < g_nFramesCount; i++) {
			FRAMEWND &F = g_pfwFrames[i];
			if (F.floating || (!F.visible) || (F.align != curalign))
				continue;

			sd[v].order = F.order;
			sd[v++].realpos = i;
		}
		if (v == 0) {
			free(sd);
			return 0;
		}
		qsort(sd, v, sizeof(SortData), sortfunc);
		for (i = 0; i < v; i++)
			g_pfwFrames[sd[i].realpos].order = i + 1; //to be sure that order is incremental
		for (i = 0; i < v; i++) {
			if (sd[i].realpos == pos) {
				if (lParam == -1) {
					if (i >= v - 1) break;
					tmpval = g_pfwFrames[sd[i + 1].realpos].order;
					g_pfwFrames[sd[i + 1].realpos].order = g_pfwFrames[pos].order;
					g_pfwFrames[pos].order = tmpval;
					break;
				}

				if (lParam == 1) {
					if (i < 1) break;
					tmpval = g_pfwFrames[sd[i - 1].realpos].order;
					g_pfwFrames[sd[i - 1].realpos].order = g_pfwFrames[pos].order;
					g_pfwFrames[pos].order = tmpval;
					break;
				}
			}
		}

		free(sd);
		CLUIFramesStoreFrameSettings(pos);
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	}

	return 0;
}

static int _us_DoMoveFrameUp(WPARAM wParam, LPARAM)
{
	return CLUIFrames_MoveFrame(wParam, (LPARAM)+1);
}

static int _us_DoMoveFrameDown(WPARAM wParam, LPARAM)
{
	return CLUIFrames_MoveFrame(wParam, (LPARAM)-1);
}

//wparam = frameid
//lparam = alignment
static int _us_DoSetFrameAlign(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	CLUIFrames_SetFrameOptions(MAKEWPARAM(FO_ALIGN, wParam), lParam);
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return 0;
}

static int _us_DoAlignFrameTop(WPARAM wParam, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	return CLUIFrames_SetFrameAlign(wParam, alTop);
}

static int _us_DoAlignFrameBottom(WPARAM wParam, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	return CLUIFrames_SetFrameAlign(wParam, alBottom);
}

static int _us_DoAlignFrameClient(WPARAM wParam, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	return CLUIFrames_SetFrameAlign(wParam, alClient);
}

//wparam = frameid
static int _us_DoLockFrame(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	int pos = (wParam == 0) ? lParam : id2pos(wParam);
	if (pos >= 0 && (int)pos < g_nFramesCount) {
		g_pfwFrames[pos].Locked = !g_pfwFrames[pos].Locked;
		CLUIFramesStoreFrameSettings(pos);
	}

	return 0;
}

//wparam = frameid
static int _us_DoSetFrameBorder(WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	if (_fCluiFramesModuleNotStarted) return -1;

	int FrameId = (wParam == 0) ? lParam : id2pos(wParam);
	if (FrameId == -1)
		return -1;

	int oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, wParam), 0);
	if (oldflags & F_NOBORDER)
		oldflags &= ~F_NOBORDER;
	else
		oldflags |= F_NOBORDER;

	HWND hw = g_pfwFrames[FrameId].hWnd;
	GetWindowRect(hw, &rc);

	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, wParam), oldflags);
	SetWindowPos(hw, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME);
	return 0;
}

//wparam = frameid
static int _us_DoCollapseFrame(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int FrameId = (wParam == 0) ? lParam : id2pos(wParam);
	if (FrameId >= 0 && FrameId < g_nFramesCount) {
		// do not collapse/uncollapse client/locked/invisible frames
		if (g_pfwFrames[FrameId].align == alClient && !(g_pfwFrames[FrameId].Locked || (!g_pfwFrames[FrameId].visible) || g_pfwFrames[FrameId].floating)) {
			if (Clist_IsDocked())
				return 0;

			if (g_CluiData.fDocked || !g_CluiData.fAutoSize) {
				RECT rc;
				GetWindowRect(g_clistApi.hwndContactList, &rc);

				if (g_pfwFrames[FrameId].collapsed == true) {
					rc.bottom -= rc.top;
					rc.bottom -= g_pfwFrames[FrameId].height;
					g_pfwFrames[FrameId].HeightWhenCollapsed = g_pfwFrames[FrameId].height;
					g_pfwFrames[FrameId].collapsed = false;
				}
				else {
					rc.bottom -= rc.top;
					rc.bottom += g_pfwFrames[FrameId].HeightWhenCollapsed;
					g_pfwFrames[FrameId].collapsed = true;
				}

				SetWindowPos(g_clistApi.hwndContactList, nullptr, 0, 0, rc.right - rc.left, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

				CLUIFramesStoreAllFrames();
			}
			return 0;
		}

		if (g_pfwFrames[FrameId].Locked || (!g_pfwFrames[FrameId].visible))
			return 0;

		int oldHeight = g_pfwFrames[FrameId].height;

		// if collapsed, uncollapse
		if (g_pfwFrames[FrameId].collapsed == true) {
			g_pfwFrames[FrameId].HeightWhenCollapsed = g_pfwFrames[FrameId].height;
			g_pfwFrames[FrameId].height = UNCOLLAPSED_FRAME_SIZE;
			g_pfwFrames[FrameId].collapsed = false;
		}
		// if uncollapsed, collapse
		else {
			g_pfwFrames[FrameId].height = g_pfwFrames[FrameId].HeightWhenCollapsed;
			g_pfwFrames[FrameId].collapsed = true;
		}

		if (!g_pfwFrames[FrameId].floating) {

			if (!CLUIFramesFitInSize()) {
				//cant collapse,we can resize only for height < alclient frame height
				int alfrm = CLUIFramesGetalClientFrame();

				if (alfrm != -1) {
					g_pfwFrames[FrameId].collapsed = false;
					if (g_pfwFrames[alfrm].height > 2 * UNCOLLAPSED_FRAME_SIZE) {
						oldHeight = g_pfwFrames[alfrm].height - UNCOLLAPSED_FRAME_SIZE;
						g_pfwFrames[FrameId].collapsed = true;
					}
				}
				else {
					int i, sumheight = 0;
					for (i = 0; i < g_nFramesCount; i++) {
						FRAMEWND &F = g_pfwFrames[i];
						if ((F.align != alClient) && (!F.floating) && (F.visible) && (!F.needhide)) {
							sumheight += (F.height) + (g_nTitleBarHeight*btoint(F.TitleBar.ShowTitleBar)) + 2;
							return FALSE;
						}
						if (sumheight > _nContactListHeight - 0 - 2)
							g_pfwFrames[FrameId].height = (_nContactListHeight - 0 - 2) - sumheight;
					}
				}

				g_pfwFrames[FrameId].height = oldHeight;

				if (g_pfwFrames[FrameId].collapsed == FALSE) {

					if (!g_pfwFrames[FrameId].floating) {
					}
					else {
						//SetWindowPos(Frames[FrameId].hWnd,HWND_TOP, 0, 0, Frames[FrameId].wndSize.right-Frames[FrameId].wndSize.left,Frames[FrameId].height,SWP_SHOWWINDOW|SWP_NOMOVE);
						SetWindowPos(g_pfwFrames[FrameId].ContainerWnd, HWND_TOP, 0, 0, g_pfwFrames[FrameId].wndSize.right - g_pfwFrames[FrameId].wndSize.left + 6, g_pfwFrames[FrameId].height + DEFAULT_TITLEBAR_HEIGHT + 4, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE);
					};


					return -1;
				};//redraw not needed
			}
		};//floating test

		//CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList,0);
		if (!g_pfwFrames[FrameId].floating) {
			CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		}
		else {
			//SetWindowPos(Frames[FrameId].hWnd,HWND_TOP, 0, 0, Frames[FrameId].wndSize.right-Frames[FrameId].wndSize.left,Frames[FrameId].height,SWP_SHOWWINDOW|SWP_NOMOVE);
			RECT contwnd;
			GetWindowRect(g_pfwFrames[FrameId].ContainerWnd, &contwnd);
			contwnd.top = contwnd.bottom - contwnd.top;//height
			contwnd.left = contwnd.right - contwnd.left;//width

			contwnd.top -= (oldHeight - g_pfwFrames[FrameId].height);//newheight
			SetWindowPos(g_pfwFrames[FrameId].ContainerWnd, HWND_TOP, 0, 0, contwnd.left, contwnd.top, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE);
		};
		CLUIFramesStoreAllFrames();
		return 0;
	}
	else
		return -1;
}

static int CLUIFramesLoadMainMenu()
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	for (auto &it : g_frameMenus)
		Menu_RemoveItem(it);
	g_frameMenus.destroy();

	// create frames menu
	CMenuItem mi(&g_plugin);
	mi.root = cont.MainMenuItem;
	int separator = 3000200000;
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		mi.hIcolibItem = F.TitleBar.hicon;
		mi.flags = CMIF_SYSTEM | CMIF_UNICODE;
		mi.position = separator++;
		mi.name.w = F.TitleBar.tbname ? F.TitleBar.tbname : F.name;
		mi.pszService = nullptr;
		g_frameMenus.insert(F.MenuHandles.MainMenuItem = Menu_AddMainMenuItem(&mi));
		CLUIFramesCreateMenuForFrame(F.id, F.MenuHandles.MainMenuItem, separator, true);
		CLUIFramesModifyMainMenuItems(F.id, 0);
		CallService(MS_CLIST_FRAMEMENUNOTIFY, (WPARAM)F.id, (LPARAM)F.MenuHandles.MainMenuItem);
	}
	return 0;
}

static HFONT CLUILoadTitleBarFont()
{
	char facename[] = "MS Shell Dlg";
	HFONT hfont;
	LOGFONTA logfont;
	memset(&logfont, 0, sizeof(logfont));
	memcpy(logfont.lfFaceName, facename, sizeof(facename));
	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = -10;
	logfont.lfCharSet = DEFAULT_CHARSET;
	hfont = CreateFontIndirectA(&logfont);
	return hfont;
}


//wparam = (CLISTFrame*)clfrm
static int _us_DoAddFrame(WPARAM wParam, LPARAM lParam)
{
	CLISTFrame *clfrm = (CLISTFrame *)wParam;

	if (g_clistApi.hwndContactList == nullptr) return -1;
	if (_fCluiFramesModuleNotStarted) return -1;
	if (clfrm->cbSize != sizeof(CLISTFrame)) return -1;
	if (!(_hTitleBarFont)) _hTitleBarFont = CLUILoadTitleBarFont();

	g_pfwFrames = (FRAMEWND*)realloc(g_pfwFrames, sizeof(FRAMEWND)*(g_nFramesCount + 1));

	FRAMEWND &F = g_pfwFrames[g_nFramesCount];
	memset(&F, 0, sizeof(FRAMEWND));
	F.id = _iNextFrameId++;
	F.align = clfrm->align;
	F.hWnd = clfrm->hWnd;
	F.height = clfrm->height;
	F.TitleBar.hicon = clfrm->hIcon;
	F.pPlugin = (HPLUGIN)lParam;
	F.floating = FALSE;
	if (clfrm->Flags & F_NO_SUBCONTAINER || !g_CluiData.fLayered)
		F.OwnerWindow = (HWND)-2;
	else
		F.OwnerWindow = nullptr;

	if (db_get_b(0, CLUIFrameModule, "RemoveAllBorders", 0) == 1)
		clfrm->Flags |= F_NOBORDER;

	F.dwFlags = clfrm->Flags;

	if (clfrm->szName.a == nullptr || ((clfrm->Flags & F_UNICODE) ? mir_wstrlen(clfrm->szName.w) : mir_strlen(clfrm->szName.a)) == 0) {
		F.name = (LPTSTR)mir_alloc(255 * sizeof(wchar_t));
		GetClassName(F.hWnd, F.name, 255);
	}
	else F.name = (clfrm->Flags & F_UNICODE) ? mir_wstrdup(clfrm->szName.w) : mir_a2u(clfrm->szName.a);

	if (IsBadCodePtr((FARPROC)clfrm->szTBname.a) || clfrm->szTBname.a == nullptr
		|| ((clfrm->Flags & F_UNICODE) ? mir_wstrlen(clfrm->szTBname.w) : mir_strlen(clfrm->szTBname.a)) == 0)
		F.TitleBar.tbname = mir_wstrdup(F.name);
	else
		F.TitleBar.tbname = (clfrm->Flags & F_UNICODE) ? mir_wstrdup(clfrm->szTBname.w) : mir_a2u(clfrm->szTBname.a);

	F.needhide = FALSE;
	F.TitleBar.ShowTitleBar = (clfrm->Flags & F_SHOWTB ? TRUE : FALSE);
	F.TitleBar.ShowTitleBarTip = (clfrm->Flags & F_SHOWTBTIP ? TRUE : FALSE);

	F.collapsed = (clfrm->Flags & F_UNCOLLAPSED) ? FALSE : TRUE;
	F.Locked = clfrm->Flags & F_LOCKED ? TRUE : FALSE;
	F.visible = clfrm->Flags & F_VISIBLE ? TRUE : FALSE;
	F.UseBorder = ((clfrm->Flags & F_NOBORDER) || g_CluiData.fLayered) ? FALSE : TRUE;

	//Frames[nFramescount].OwnerWindow = 0;

	F.TitleBar.hwnd = CreateWindow(CLUIFrameTitleBarClassName, TranslateW_LP(F.name, F.pPlugin),
		(db_get_b(0, CLUIFrameModule, "RemoveAllTitleBarBorders", 1) ? 0 : WS_BORDER)
		| WS_CHILD | WS_CLIPCHILDREN |
		(F.TitleBar.ShowTitleBar ? WS_VISIBLE : 0) |
		WS_CLIPCHILDREN,
		0, 0, 0, 0, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);
	SetWindowLongPtr(F.TitleBar.hwnd, GWLP_USERDATA, F.id);

	F.TitleBar.hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		g_clistApi.hwndContactList, nullptr, g_plugin.getInst(),
		nullptr);

	SetWindowPos(F.TitleBar.hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	{
		TOOLINFO ti = { sizeof(ti) };
		ti.lpszText = L"";
		ti.hinst = g_plugin.getInst();
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.uId = (UINT_PTR)F.TitleBar.hwnd;
		SendMessage(F.TitleBar.hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

	SendMessageA(F.TitleBar.hwndTip, TTM_ACTIVATE, (WPARAM)F.TitleBar.ShowTitleBarTip, 0);

	F.oldstyles = GetWindowLongPtr(F.hWnd, GWL_STYLE);
	F.TitleBar.oldstyles = GetWindowLongPtr(F.TitleBar.hwnd, GWL_STYLE);
	//Frames[nFramescount].FloatingPos.x =

	int retval = F.id;
	F.order = g_nFramesCount + 1;
	g_nFramesCount++;

	CLUIFramesLoadFrameSettings(id2pos(retval));
	if (F.collapsed == FALSE)
		F.height = 0;

	// create frame
	LONG_PTR style = GetWindowLongPtr(F.hWnd, GWL_STYLE);
	style &= (~WS_BORDER);
	style |= (((F.UseBorder) && !g_CluiData.fLayered) ? WS_BORDER : 0);
	SetWindowLongPtr(F.hWnd, GWL_STYLE, style);
	SetWindowLongPtr(F.TitleBar.hwnd, GWL_STYLE, style& ~(WS_VSCROLL | WS_HSCROLL));
	SetWindowLongPtr(F.TitleBar.hwnd, GWL_STYLE, GetWindowLongPtr(F.TitleBar.hwnd, GWL_STYLE)&~(WS_VSCROLL | WS_HSCROLL));

	if (F.order == 0)
		F.order = g_nFramesCount;

	// need to enlarge parent
	RECT mainRect;
	int mainHeight, minHeight;
	GetWindowRect(g_clistApi.hwndContactList, &mainRect);
	mainHeight = mainRect.bottom - mainRect.top;
	minHeight = CLUIFrames_GetTotalHeight();
	if (mainHeight < minHeight) {
		BOOL Upward = !g_CluiData.fDocked && g_CluiData.fAutoSize && db_get_b(0, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT);
		if (Upward)
			mainRect.top = mainRect.bottom - minHeight;
		else
			mainRect.bottom = mainRect.top + minHeight;
		SetWindowPos(g_clistApi.hwndContactList, nullptr, mainRect.left, mainRect.top, mainRect.right - mainRect.left, mainRect.bottom - mainRect.top, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
	}
	GetWindowRect(g_clistApi.hwndContactList, &mainRect);
	mainHeight = mainRect.bottom - mainRect.top;

	_nClientFrameId = eUnknownId; // recalc it
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);

	if (F.floating) {
		F.floating = false;
		CLUIFrames_SetFrameFloat(retval, 1); // lparam = 1 use stored width and height
	}
	else CLUIFrames_SetFrameFloat(retval, 2);

	return retval;
}

static int _us_DoRemoveFrame(WPARAM wParam, LPARAM)
{
	int pos;
	if (_fCluiFramesModuleNotStarted) return -1;

	pos = id2pos(wParam);

	if (pos < 0 || pos>g_nFramesCount) return -1;;

	FRAMEWND &F = g_pfwFrames[pos];
	replaceStrW(F.name, nullptr);
	replaceStrW(F.TitleBar.tbname, nullptr);
	replaceStrW(F.TitleBar.tooltip, nullptr);
	DestroyWindow(F.hWnd);
	F.hWnd = (HWND)-1;
	DestroyWindow(F.TitleBar.hwnd);
	if (F.TitleBar.hwndTip)
		DestroyWindow(F.TitleBar.hwndTip);
	F.TitleBar.hwnd = (HWND)-1;
	if (F.ContainerWnd && F.ContainerWnd != (HWND)-1) DestroyWindow(F.ContainerWnd);
	F.ContainerWnd = (HWND)-1;
	if (F.TitleBar.hmenu) DestroyMenu(F.TitleBar.hmenu);
	F.PaintCallbackProc = nullptr;
	if (F.UpdateRgn) DeleteObject(F.UpdateRgn);

	if (F.OwnerWindow != (HWND)-1 && F.OwnerWindow != (HWND)-2 && F.OwnerWindow != nullptr)
		DestroyWindow(F.OwnerWindow);

	F.OwnerWindow = nullptr;
	RemoveItemFromList(pos, &g_pfwFrames, &g_nFramesCount);

	cliInvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	cliInvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);

	return 0;
};


static int CLUIFramesForceUpdateTB(const FRAMEWND *Frame)
{
	if (Frame->TitleBar.hwnd != nullptr)
		RedrawWindow(Frame->TitleBar.hwnd, nullptr, nullptr, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	//UpdateWindow(Frame->TitleBar.hwnd);
	return 0;
}

static int CLUIFramesForceUpdateFrame(const FRAMEWND *Frame)
{
	if (Frame->hWnd != nullptr) {
		RedrawWindow(Frame->hWnd, nullptr, nullptr, RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_INVALIDATE);
		UpdateWindow(Frame->hWnd);
	}

	if (Frame->floating && Frame->ContainerWnd != nullptr)
		RedrawWindow(Frame->ContainerWnd, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	return 0;
}

static int CLUIFrameMoveResize(const FRAMEWND *Frame)
{
	// we need to show or hide the frame?
	if (Frame->visible && (!Frame->needhide)) {
		if (Frame->OwnerWindow != (HWND)-2 && Frame->OwnerWindow) {
			//          CLUI_ShowWindowMod(Frame->OwnerWindow,SW_SHOW);
		}
		CLUI_ShowWindowMod(Frame->hWnd, SW_SHOW/*NOACTIVATE*/);
		CLUI_ShowWindowMod(Frame->TitleBar.hwnd, Frame->TitleBar.ShowTitleBar == TRUE ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
	}
	else {
		if (Frame->OwnerWindow && Frame->OwnerWindow != (HWND)(-1) && Frame->OwnerWindow != (HWND)(-2)) {
			CLUI_ShowWindowMod(Frame->OwnerWindow, SW_HIDE);
		}
		CLUI_ShowWindowMod(Frame->hWnd, SW_HIDE);
		CLUI_ShowWindowMod(Frame->TitleBar.hwnd, SW_HIDE);
		return 0;
	}

	if (Frame->OwnerWindow && Frame->OwnerWindow != (HWND)-2) {
		RECT pr;
		POINT Off = { 0 };

		ClientToScreen(g_clistApi.hwndContactList, &Off);
		GetWindowRect(g_clistApi.hwndContactList, &pr);

		if (Frame->visible && (!Frame->collapsed || Frame->wndSize.bottom - Frame->wndSize.top == 0)) {
			ShowWindowAsync(Frame->OwnerWindow, SW_HIDE);
			ShowWindowAsync(Frame->hWnd, SW_HIDE);
		}

		{
			SetWindowPos(Frame->OwnerWindow, nullptr, Frame->wndSize.left + Off.x, Frame->wndSize.top + Off.y,
				Frame->wndSize.right - Frame->wndSize.left,
				Frame->wndSize.bottom - Frame->wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);   //- -= -=

			SetWindowPos(Frame->hWnd, nullptr, 0, 0,
				Frame->wndSize.right - Frame->wndSize.left,
				Frame->wndSize.bottom - Frame->wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		// set titlebar position
		if (Frame->TitleBar.ShowTitleBar) {
			SetWindowPos(Frame->TitleBar.hwnd, nullptr, Frame->wndSize.left, Frame->wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar,
				Frame->wndSize.right - Frame->wndSize.left,
				g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		if (Frame->visible && IsWindowVisible(g_clistApi.hwndContactList) && Frame->collapsed && Frame->wndSize.bottom - Frame->wndSize.top != 0) {
			ShowWindow(Frame->OwnerWindow, SW_SHOW);
			ShowWindow(Frame->hWnd, SW_SHOW);
		}

	}
	else {
		// set frame position
		SetWindowPos(Frame->hWnd, nullptr, Frame->wndSize.left, Frame->wndSize.top,
			Frame->wndSize.right - Frame->wndSize.left,
			Frame->wndSize.bottom - Frame->wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
		// set titlebar position
		if (Frame->TitleBar.ShowTitleBar) {
			SetWindowPos(Frame->TitleBar.hwnd, nullptr, Frame->wndSize.left, Frame->wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar,
				Frame->wndSize.right - Frame->wndSize.left,
				g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE);

		}
	}
	//	Sleep(0);
	return 0;
}

static BOOL CLUIFramesFitInSize(void)
{
	int sumheight = 0;
	int tbh = 0; // title bar height
	int clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = g_nTitleBarHeight * btoint(g_pfwFrames[clientfrm].TitleBar.ShowTitleBar);

	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if ((F.align != alClient) && (!F.floating) && (F.visible) && (!F.needhide)) {
			sumheight += (F.height) + (g_nTitleBarHeight*btoint(F.TitleBar.ShowTitleBar)) + 2/*+btoint(Frames[i].UseBorder)*2*/;
			if (sumheight > _nContactListHeight - tbh - 2) {
				if (!g_CluiData.fDocked && g_CluiData.fAutoSize)
					return TRUE; //Can be required to enlarge

				return FALSE;
			}
		}
	}
	return TRUE;
}

int CLUIFrames_GetTotalHeight()
{
	if (g_clistApi.hwndContactList == nullptr) return 0;

	int sumheight = 0;
	RECT border;
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if ((F.visible) && (!F.needhide) && (!F.floating) && (g_clistApi.hwndContactTree) && (F.hWnd != g_clistApi.hwndContactTree))
			sumheight += (F.height) + (g_nTitleBarHeight*btoint(F.TitleBar.ShowTitleBar));
	}

	GetBorderSize(g_clistApi.hwndContactList, &border);

	//TODO minsize
	sumheight += g_CluiData.TopClientMargin;
	sumheight += g_CluiData.BottomClientMargin;
	return  max(db_get_w(0, "CLUI", "MinHeight", SETTING_MINHEIGTH_DEFAULT),
		(sumheight + border.top + border.bottom));
}

int CLUIFramesGetMinHeight()
{
	int tbh = 0, sumheight = 0;
	RECT border;
	int allbord = 0;
	if (g_clistApi.hwndContactList == nullptr) return 0;


	// search for alClient frame and get the titlebar's height
	int clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = g_nTitleBarHeight*btoint(g_pfwFrames[clientfrm].TitleBar.ShowTitleBar);

	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if ((F.align != alClient) && (F.visible) && (!F.needhide) && (!F.floating)) {
			RECT wsize;

			GetWindowRect(F.hWnd, &wsize);
			sumheight += (wsize.bottom - wsize.top) + (g_nTitleBarHeight*btoint(F.TitleBar.ShowTitleBar));
		}
	}

	GetBorderSize(g_clistApi.hwndContactList, &border);

	//GetWindowRect(g_clistApi.hwndContactList,&winrect);
	//GetClientRect(g_clistApi.hwndContactList,&clirect);
	//	clirect.bottom -= clirect.top;
	//	clirect.bottom += border.top+border.bottom;
	//allbord = (winrect.bottom-winrect.top)-(clirect.bottom-clirect.top);

	//TODO minsize
	sumheight += g_CluiData.TopClientMargin;
	sumheight += g_CluiData.BottomClientMargin;
	return  max(db_get_w(0, "CLUI", "MinHeight", SETTING_MINHEIGTH_DEFAULT),
		(sumheight + border.top + border.bottom + allbord + tbh));
}




static int CLUIFramesResizeFrames(const RECT newsize)
{
	int sumheight = 9999999, newheight;
	int prevframe, prevframebottomline;
	int tbh, curfrmtbh;
	int drawitems;
	int clientfrm;
	int i, j;
	int sepw = GapBetweenFrames;
	int topBorder = newsize.top;
	SortData *sdarray;

	g_CluiData.nGapBetweenTitlebar = (int)db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	GapBetweenFrames = db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);

	if (g_nFramesCount < 1) return 0;
	newheight = newsize.bottom - newsize.top;

	// search for alClient frame and get the titlebar's height
	tbh = 0;
	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(g_pfwFrames[clientfrm].TitleBar.ShowTitleBar);

	for (i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating) {
			F.needhide = FALSE;
			F.wndSize.left = newsize.left;
			F.wndSize.right = newsize.right;
		}
	}
	{
		//sorting stuff
		sdarray = (SortData*)malloc(sizeof(SortData)*g_nFramesCount);
		if (sdarray == nullptr) return -1;;
		for (i = 0; i < g_nFramesCount; i++) {
			sdarray[i].order = g_pfwFrames[i].order;
			sdarray[i].realpos = i;
		}
		qsort(sdarray, g_nFramesCount, sizeof(SortData), sortfunc);

	}

	drawitems = g_nFramesCount;

	while (sumheight > (newheight - tbh) && drawitems > 0) {
		sumheight = 0;
		drawitems = 0;
		for (i = 0; i < g_nFramesCount; i++) {
			FRAMEWND &F = g_pfwFrames[i];
			if (((F.align != alClient)) && (!F.floating) && (F.visible) && (!F.needhide)) {
				drawitems++;
				curfrmtbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(F.TitleBar.ShowTitleBar);
				sumheight += (F.height) + curfrmtbh + (i > 0 ? sepw : 0) + ((F.UseBorder && !g_CluiData.fLayered) ? 2 : 0);
				if (sumheight > newheight - tbh) {
					sumheight -= (F.height) + curfrmtbh + (i > 0 ? sepw : 0);
					F.needhide = g_CluiData.fDocked || !g_CluiData.fAutoSize;
					drawitems--;
					break;
				}
			}
		}
	}

	prevframe = -1;
	prevframebottomline = topBorder;
	for (j = 0; j < g_nFramesCount; j++) {
		//move all alTop frames
		i = sdarray[j].realpos;
		FRAMEWND &F = g_pfwFrames[i];
		if ((!F.needhide) && (!F.floating) && (F.visible) && (F.align == alTop)) {
			curfrmtbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar) * btoint(F.TitleBar.ShowTitleBar);
			F.wndSize.top = prevframebottomline + (i > 0 ? sepw : 0) + (curfrmtbh);
			F.wndSize.bottom = F.height + F.wndSize.top + ((F.UseBorder && !g_CluiData.fLayered) ? 2 : 0);
			F.prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = F.wndSize.bottom;
			if (prevframebottomline > newheight) {
				//prevframebottomline -= Frames[i].height+(curfrmtbh+1);
				//Frames[i].needhide = TRUE;
			}
		}
	}

	if (sumheight < newheight) {
		for (j = 0; j < g_nFramesCount; j++) {
			//move alClient frame
			i = sdarray[j].realpos;
			FRAMEWND &F = g_pfwFrames[i];
			if ((!F.needhide) && (!F.floating) && (F.visible) && (F.align == alClient)) {
				int oldh;
				F.wndSize.top = prevframebottomline + (j > 0 ? sepw : 0) + (tbh);
				F.wndSize.bottom = F.wndSize.top + newheight - sumheight - tbh - (j > 0 ? sepw : 0);

				oldh = F.height;
				F.height = F.wndSize.bottom - F.wndSize.top;
				F.prevvisframe = prevframe;
				prevframe = i;
				prevframebottomline = F.wndSize.bottom;
				if (prevframebottomline > newheight) {
					//prevframebottomline -= Frames[i].height+(tbh+1);
					//Frames[i].needhide = TRUE;
				}
				break;
			}
		}
	}

	//newheight
	prevframebottomline = newheight + sepw + topBorder;
	//prevframe = -1;
	for (j = g_nFramesCount - 1; j >= 0; j--) {
		//move all alBottom frames
		i = sdarray[j].realpos;
		FRAMEWND &F = g_pfwFrames[i];
		if ((F.visible) && (!F.floating) && (!F.needhide) && (F.align == alBottom)) {
			curfrmtbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(F.TitleBar.ShowTitleBar);

			F.wndSize.bottom = prevframebottomline - (j > 0 ? sepw : 0);
			F.wndSize.top = F.wndSize.bottom - F.height - ((F.UseBorder && !g_CluiData.fLayered) ? 2 : 0);
			F.prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = F.wndSize.top/*-1*/ - curfrmtbh;
			if (prevframebottomline > newheight) {

			}
		}
	}

	for (i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (F.TitleBar.ShowTitleBar)
			SetRect(&F.TitleBar.wndSize, F.wndSize.left, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, F.wndSize.right, F.wndSize.top - g_CluiData.nGapBetweenTitlebar);
	}

	if (sdarray != nullptr)
		free(sdarray);

	return 0;
}

static int SizeMoveNewSizes()
{
	int i;
	for (i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (F.floating) {
			CLUIFrameResizeFloatingFrame(i);
		}
		else {
			CLUIFrameMoveResize(&F);
		};
	}
	return 0;
}


static int CLUIFramesResize(RECT newsize)
{
	CLUIFramesResizeFrames(newsize);
	SizeMoveNewSizes();
	return 0;
}
int CLUIFrames_ApplyNewSizes(int mode)
{
	int i;
	g_CluiData.mutexPreventDockMoving = 0;
	for (i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if ((mode == 1 && F.OwnerWindow != (HWND)-2 && F.OwnerWindow) ||
			(mode == 2 && F.OwnerWindow == (HWND)-2) ||
			(mode == 3))
			if (F.floating) {
				CLUIFrameResizeFloatingFrame(i);
			}
			else {
				CLUIFrameMoveResize(&g_pfwFrames[i]);
			};
	}
	if (IsWindowVisible(g_clistApi.hwndContactList)) {
		ske_DrawNonFramedObjects(1, nullptr);
		CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, 0, 0);
	}
	g_CluiData.mutexPreventDockMoving = 1;
	return 0;
}

static int _us_DoUpdateFrame(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	if (wParam == -1) {
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		return 0;
	}

	if (lParam & FU_FMPOS)
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 1);

	int pos = id2pos(wParam);
	if (pos < 0 || pos >= g_nFramesCount)
		return -1;
	if (lParam & FU_TBREDRAW)
		CLUIFramesForceUpdateTB(&g_pfwFrames[pos]);
	if (lParam & FU_FMREDRAW)
		CLUIFramesForceUpdateFrame(&g_pfwFrames[pos]);
	return 0;
}

int CLUIFrames_OnClistResize_mod(WPARAM, LPARAM mode)
{
	int tick;
	GapBetweenFrames = db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);
	g_CluiData.nGapBetweenTitlebar = db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	if (_fCluiFramesModuleNotStarted) return -1;

	RECT nRect;
	GetClientRect(g_clistApi.hwndContactList, &nRect);

	nRect.left += g_CluiData.LeftClientMargin;
	nRect.right -= g_CluiData.RightClientMargin;
	nRect.top += g_CluiData.TopClientMargin;
	nRect.bottom -= g_CluiData.BottomClientMargin;
	//	g_CluiData.mutexPreventDockMoving = 0;
	tick = GetTickCount();
	CLUIFramesResize(nRect);
	if (mode == 0) CLUIFrames_ApplyNewSizes(3);

	tick = GetTickCount() - tick;

	Sleep(0);

	//dont save to database too many times
	if (GetTickCount() - _dwLastStoreTick > 1000) { CLUIFramesStoreAllFrames(); _dwLastStoreTick = GetTickCount(); };

	return 0;
}

int SizeFramesByWindowRect(RECT *r, HDWP * PosBatch, int mode)
{
	RECT nRect;
	if (_fCluiFramesModuleNotStarted) return -1;

	g_CluiData.nGapBetweenTitlebar = (int)db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	GapBetweenFrames = db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);
	nRect.left = 0;
	nRect.top = 0;
	nRect.right = r->right - r->left;
	nRect.bottom = r->bottom - r->top;
	nRect.left += g_CluiData.LeftClientMargin;
	nRect.right -= g_CluiData.RightClientMargin;
	nRect.top += g_CluiData.TopClientMargin;
	nRect.bottom -= g_CluiData.BottomClientMargin;
	CLUIFramesResizeFrames(nRect);
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		int dx = 0;//rcNewWindowRect.left-rcOldWindowRect.left;
		int dy = 0;//_window_rect.top-rcOldWindowRect.top;
		if (!F.floating) {
			if (F.visible && !F.needhide && !IsWindowVisible(F.hWnd)) {
				ShowWindow(F.hWnd, SW_SHOW);
				if (F.TitleBar.ShowTitleBar) ShowWindow(F.TitleBar.hwnd, SW_SHOW);
			}
			if (F.OwnerWindow && (INT_PTR)(F.OwnerWindow) != -2) {
				if (!(mode & 2)) {
					if (nullptr != F.OwnerWindow) { /* Wine fix. */
						*PosBatch = DeferWindowPos(*PosBatch, F.OwnerWindow, nullptr, F.wndSize.left + r->left, F.wndSize.top + r->top,
							F.wndSize.right - F.wndSize.left, F.wndSize.bottom - F.wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
						SetWindowPos(F.hWnd, nullptr, 0, 0,
							F.wndSize.right - F.wndSize.left, F.wndSize.bottom - F.wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE/*|SWP_NOSENDCHANGING*/);
					}
				}
				//Frame
				if (F.TitleBar.ShowTitleBar) {
					SetWindowPos(F.TitleBar.hwnd, nullptr, F.wndSize.left + dx, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar + dy,
						F.wndSize.right - F.wndSize.left, g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
					SetRect(&F.TitleBar.wndSize, F.wndSize.left, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, F.wndSize.right, F.wndSize.top - g_CluiData.nGapBetweenTitlebar);
					UpdateWindow(F.TitleBar.hwnd);
				}
			}
			else {
				if (1) {
					int res = 0;
					// set frame position
					res = SetWindowPos(F.hWnd, nullptr, F.wndSize.left + dx, F.wndSize.top + dy,
						F.wndSize.right - F.wndSize.left,
						F.wndSize.bottom - F.wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
				}
				if (1) {
					// set titlebar position
					if (F.TitleBar.ShowTitleBar) {
						SetWindowPos(F.TitleBar.hwnd, nullptr, F.wndSize.left + dx, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar + dy,
							F.wndSize.right - F.wndSize.left, g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
						SetRect(&F.TitleBar.wndSize, F.wndSize.left, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, F.wndSize.right, F.wndSize.top - g_CluiData.nGapBetweenTitlebar);

					}
				}
				UpdateWindow(F.hWnd);
				if (F.TitleBar.ShowTitleBar) UpdateWindow(F.TitleBar.hwnd);
			};
		}

	}
	if (GetTickCount() - _dwLastStoreTick > 1000) {
		CLUIFramesStoreAllFrames();
		_dwLastStoreTick = GetTickCount();
	}
	return 0;
}

int CheckFramesPos(RECT *wr)
{
	//CALLED only FROM MainWindow procedure at CLUI.c
	if (_fCluiFramesModuleNotStarted)
		return -1;
	g_CluiData.nGapBetweenTitlebar = (int)db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	GapBetweenFrames = db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);

	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		int dx = 0;//rcNewWindowRect.left-rcOldWindowRect.left;
		int dy = 0;//_window_rect.top-rcOldWindowRect.top;
		if (!F.floating && F.visible) {
			if (!(F.OwnerWindow && (INT_PTR)(F.OwnerWindow) != -2)) {
				RECT r;
				GetWindowRect(F.hWnd, &r);
				if (r.top - wr->top != F.wndSize.top || r.left - wr->left != F.wndSize.left)
					SetWindowPos(F.hWnd, nullptr, F.wndSize.left, F.wndSize.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
			if (F.TitleBar.ShowTitleBar) {
				RECT r;
				GetWindowRect(F.TitleBar.hwnd, &r);
				if (r.top - wr->top != F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar || r.left - wr->left != F.wndSize.left) {
					SetWindowPos(F.TitleBar.hwnd, nullptr, F.wndSize.left + dx, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar + dy,
						F.wndSize.right - F.wndSize.left, g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
					SetRect(&F.TitleBar.wndSize, F.wndSize.left, F.wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, F.wndSize.right, F.wndSize.top - g_CluiData.nGapBetweenTitlebar);
				}
			}
		}
	}

	return 0;
}

int CLUIFramesOnClistResize(WPARAM wParam, LPARAM lParam)
{
	RECT nRect;
	int tick;
	GapBetweenFrames = db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);
	g_CluiData.nGapBetweenTitlebar = db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);

	if (_fCluiFramesModuleNotStarted) return -1;

	//need to enlarge parent
	{
		RECT mainRect;
		int mainHeight, minHeight;
		GetWindowRect(g_clistApi.hwndContactList, &mainRect);
		mainHeight = mainRect.bottom - mainRect.top;
		minHeight = CLUIFrames_GetTotalHeight();
		if (mainHeight < minHeight) {
			bool Upward = !g_CluiData.fDocked && g_CluiData.fAutoSize && db_get_b(0, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT);
			if (Upward)
				mainRect.top = mainRect.bottom - minHeight;
			else
				mainRect.bottom = mainRect.top + minHeight;
			SetWindowPos(g_clistApi.hwndContactList, nullptr, mainRect.left, mainRect.top, mainRect.right - mainRect.left, mainRect.bottom - mainRect.top, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		}
		GetWindowRect(g_clistApi.hwndContactList, &mainRect);
		mainHeight = mainRect.bottom - mainRect.top;
	}
	GetClientRect(g_clistApi.hwndContactList, &nRect);
	//$$$ Fixed borders
	if (lParam && lParam != 1 && lParam != 2) {
		RECT oldRect;
		POINT pt;
		RECT *newRect = (RECT *)lParam;
		int dl, dt, dr, db;
		GetWindowRect((HWND)wParam, &oldRect);
		pt.x = nRect.left;
		pt.y = nRect.top;
		ClientToScreen(g_clistApi.hwndContactList, &pt);
		dl = pt.x - oldRect.left;
		dt = pt.y - oldRect.top;
		dr = (oldRect.right - oldRect.left) - (nRect.right - nRect.left) - dl;
		db = (oldRect.bottom - oldRect.top) - (nRect.bottom - nRect.top) - dt;
		nRect.left = newRect->left + dl;
		nRect.top = newRect->top + dt;
		nRect.bottom = newRect->bottom - db;
		nRect.right = newRect->right - dr;
	}
	
	nRect.left += g_CluiData.LeftClientMargin;
	nRect.right -= g_CluiData.RightClientMargin;
	nRect.top += g_CluiData.TopClientMargin;
	nRect.bottom -= g_CluiData.BottomClientMargin;

	if (nRect.bottom < nRect.top)
		nRect.bottom = nRect.top;
	_nContactListHeight = nRect.bottom - nRect.top; //$$

	tick = GetTickCount();

	CLUIFramesResize(nRect);

	CLUIFrames_ApplyNewSizes(2);
	CLUIFrames_ApplyNewSizes(1);

	tick = GetTickCount() - tick;

	if (g_clistApi.hwndContactList != nullptr) cliInvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
	if (g_clistApi.hwndContactList != nullptr) UpdateWindow(g_clistApi.hwndContactList);

	if (lParam == 2) RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);


	Sleep(0);

	// dont save to database too many times
	if (GetTickCount() - _dwLastStoreTick > 1000) {
		CLUIFramesStoreAllFrames();
		_dwLastStoreTick = GetTickCount();
	}

	return 0;
}

int OnFrameTitleBarBackgroundChange(WPARAM, LPARAM)
{
	if (MirandaExiting()) return 0;

	AlignCOLLIconToLeft = db_get_b(0, "FrameTitleBar", "AlignCOLLIconToLeft", CLCDEFAULT_COLLICONTOLEFT);

	sttBkColour = cliGetColor("FrameTitleBar", "BkColour", CLCDEFAULT_BKCOLOUR);
	sttBkUseWinColours = db_get_b(0, "FrameTitleBar", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
	sttSelBkColour = cliGetColor("FrameTitleBar", "TextColour", CLCDEFAULT_TEXTCOLOUR);

	if (sttBmpBackground) {
		DeleteObject(sttBmpBackground);
		sttBmpBackground = nullptr;
	}
	if (g_CluiData.fDisableSkinEngine) {
		if (db_get_b(0, "FrameTitleBar", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmapName(db_get_wsa(0, "FrameTitleBar", "BkBitmap"));
			if (tszBitmapName)
				sttBmpBackground = Bitmap_Load(tszBitmapName);
		}
		sttBackgroundBmpUse = db_get_w(0, "FrameTitleBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}

	cliInvalidateRect(g_clistApi.hwndContactList, nullptr, 0);

	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);
	return 0;
}

void DrawBackGround(HWND hwnd, HDC mhdc, HBITMAP hBmpBackground, COLORREF bkColour, uint32_t backgroundBmpUse)
{
	HDC hdc;
	RECT clRect, *rcPaint;

	int yScroll = 0;
	PAINTSTRUCT paintst = {};

	HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);

	if (mhdc) {
		hdc = mhdc;
		rcPaint = nullptr;
	}
	else {
		hdc = BeginPaint(hwnd, &paintst);
		rcPaint = &(paintst.rcPaint);
	}

	GetClientRect(hwnd, &clRect);
	if (rcPaint == nullptr)
		rcPaint = &clRect;
	if (rcPaint->right - rcPaint->left == 0 || rcPaint->top - rcPaint->bottom == 0)
		rcPaint = &clRect;
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBmpOsb = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), nullptr);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmpOsb);
	HFONT oFont = (HFONT)SelectObject(hdcMem, hFont);
	SetBkMode(hdcMem, TRANSPARENT);
	SetStretchBltMode(hdcMem, HALFTONE);
	HBRUSH hBrush = CreateSolidBrush(bkColour);
	FillRect(hdcMem, rcPaint, hBrush);
	DeleteObject(hBrush);
	if (hBmpBackground) {
		BITMAP bmp;
		int destw, desth;

		GetObject(hBmpBackground, sizeof(bmp), &bmp);
		HDC hdcBmp = CreateCompatibleDC(hdcMem);
		SelectObject(hdcBmp, hBmpBackground);
		int y = backgroundBmpUse & CLBF_SCROLL ? -yScroll : 0;
		int maxx = backgroundBmpUse & CLBF_TILEH ? clRect.right : 1;
		int maxy = backgroundBmpUse & CLBF_TILEV ? rcPaint->bottom : y + 1;
		switch (backgroundBmpUse & CLBM_TYPE) {
		case CLB_STRETCH:
			if (backgroundBmpUse & CLBF_PROPORTIONAL) {
				if (clRect.right*bmp.bmHeight < clRect.bottom*bmp.bmWidth) {
					desth = clRect.bottom;
					destw = desth*bmp.bmWidth / bmp.bmHeight;
				}
				else {
					destw = clRect.right;
					//desth = destw*bmp.bmHeight / bmp.bmWidth;
				}
			}
			else {
				destw = clRect.right;
				//desth = clRect.bottom;
			}
			break;
		case CLB_STRETCHH:
			destw = clRect.right;
			break;
		case CLB_STRETCHV:
			if (backgroundBmpUse & CLBF_PROPORTIONAL) {
				desth = clRect.bottom;
				destw = desth*bmp.bmWidth / bmp.bmHeight;
			}
			else {
				destw = bmp.bmWidth;
				//desth = clRect.bottom;
			}
			break;
		default:    //clb_topleft
			destw = bmp.bmWidth;
			//desth = bmp.bmHeight;
			break;
		}
		desth = clRect.bottom - clRect.top;
		for (; y < maxy; y += desth) {
			if (y < rcPaint->top - desth) continue;
			for (int x = 0; x < maxx; x += destw)
				StretchBlt(hdcMem, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		}
		DeleteDC(hdcBmp);
	}

	BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);

	SelectObject(hdcMem, hOldBmp);
	SelectObject(hdcMem, oFont);
	DeleteObject(hBmpOsb);
	DeleteDC(hdcMem);
	paintst.fErase = FALSE;
	if (!mhdc)
		EndPaint(hwnd, &paintst);
}

int DrawTitleBar(HDC hdcMem2, RECT *rect, int Frameid)
{
	int pos = id2pos(Frameid);
	if (pos < 0 || pos >= g_nFramesCount)
		return 0;

	FRAMEWND &F = g_pfwFrames[pos];

	bool bThemed = FALSE;
	HDC hdcMem;
	RECT rc = *rect;
	HBITMAP b1 = nullptr, b2 = nullptr;
	hdcMem = CreateCompatibleDC(hdcMem2);

	SetBkMode(hdcMem, TRANSPARENT);
	HFONT hoTTBFont = (HFONT)SelectObject(hdcMem, _hTitleBarFont);
	ske_ResetTextEffect(hdcMem);
	ske_ResetTextEffect(hdcMem2);
	HBRUSH hBack = GetSysColorBrush(COLOR_3DFACE);
	HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem, hBack);

	GetClientRect(F.TitleBar.hwnd, &rc);
	if (F.floating) {
		rc.bottom = rc.top + g_nTitleBarHeight;
		F.TitleBar.wndSize = rc;
	}
	else {
		F.TitleBar.wndSize = rc;
	}
	b1 = ske_CreateDIB32(rc.right - rc.left, rc.bottom - rc.top);
	b2 = (HBITMAP)SelectObject(hdcMem, b1);
	if (F.floating && !g_CluiData.fDisableSkinEngine) {
		FillRect(hdcMem, &rc, hBack);
		//SelectObject(hdcMem,hoBrush);
		SkinDrawGlyph(hdcMem, &rc, &rc, "Main,ID=FrameCaption");
	}
	else {
		if (g_CluiData.fDisableSkinEngine) {
			if (!sttBmpBackground && sttBkUseWinColours && xpt_IsThemed(_hFrameTitleTheme)) {
				int state = CS_ACTIVE;
				// if (GetForegroundWindow() != g_clistApi.hwndContactList) state = CS_INACTIVE;
				xpt_DrawThemeBackground(_hFrameTitleTheme, hdcMem, WP_SMALLCAPTION, state, &rc, &rc);
				bThemed = true;
			}
			else DrawBackGround(F.TitleBar.hwnd, hdcMem, sttBmpBackground, sttBkColour, sttBackgroundBmpUse);
		}
		else if (!g_CluiData.fLayered) {
			ske_BltBackImage(F.TitleBar.hwnd, hdcMem, &rc);
		}
		else  BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcMem2, rect->left, rect->top, SRCCOPY);
		SkinDrawGlyph(hdcMem, &rc, &rc, "Main,ID=FrameCaption");
	}
	
	if (bThemed)
		SetTextColor(hdcMem, GetSysColor(COLOR_CAPTIONTEXT));
	else
		SetTextColor(hdcMem, sttSelBkColour);

	RECT textrc = rc;
	if (!AlignCOLLIconToLeft) {
		if (F.TitleBar.hicon != nullptr) {
			ske_DrawIconEx(hdcMem, rc.left + 2, rc.top + ((g_nTitleBarHeight >> 1) - (GetSystemMetrics(SM_CYSMICON) >> 1)), F.TitleBar.hicon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, nullptr, DI_NORMAL);
			textrc.left += GetSystemMetrics(SM_CXSMICON) + 4;
			textrc.top += 2;
		}
		else {
			textrc.left += 2;
			textrc.top += 2;
		}
	}
	else {
		textrc.left += GetSystemMetrics(SM_CXSMICON) + 2;
		textrc.top += 2;
	}

	wchar_t *wszTitle = TranslateW_LP(F.TitleBar.tbname, F.pPlugin);
	ske_TextOut(hdcMem, textrc.left, textrc.top, wszTitle, (int)mir_wstrlen(wszTitle));

	if (!AlignCOLLIconToLeft)
		ske_DrawIconEx(hdcMem, F.TitleBar.wndSize.right - GetSystemMetrics(SM_CXSMICON) - 2, rc.top + ((g_nTitleBarHeight >> 1) - (GetSystemMetrics(SM_CXSMICON) >> 1)), F.collapsed ? Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN) : Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT), GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, nullptr, DI_NORMAL);
	else
		ske_DrawIconEx(hdcMem, rc.left, rc.top + ((g_nTitleBarHeight >> 1) - (GetSystemMetrics(SM_CXSMICON) >> 1)), F.collapsed ? Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN) : Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT), GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, nullptr, DI_NORMAL);

	if (F.floating || (!g_CluiData.fLayered)) {
		HRGN rgn = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
		SelectClipRgn(hdcMem2, rgn);
		BitBlt(hdcMem2, rect->left, rect->top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
		DeleteObject(rgn);
	}
	else BitBlt(hdcMem2, rect->left, rect->top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, b2);
	DeleteObject(b1);
	SelectObject(hdcMem, hoTTBFont);
	SelectObject(hdcMem, hoBrush);
	DeleteDC(hdcMem);
	return 0;
}

static LRESULT CALLBACK CLUIFrameTitleBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int s_nLastByPos = -1;
	static int s_nOldFrameHeight = -1;
	static int s_nCurDragBar = -1;

	RECT rect;
	int Frameid, Framemod, direction;
	int xpos, ypos;

	Frameid = (GetWindowLongPtr(hwnd, GWLP_USERDATA));
	memset(&rect, 0, sizeof(rect));

	switch (msg) {
	case WM_CREATE:
		if (!_hFrameTitleTheme)
			_hFrameTitleTheme = xpt_AddThemeHandle(hwnd, L"WINDOW");
		SendMessage(hwnd, WM_SETFONT, (WPARAM)_hTitleBarFont, 0);
		return FALSE;

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		return Menu_DrawItem(lParam);

	case WM_USER + 100:
		return 1;

	case WM_ENABLE:
		if (hwnd != nullptr)
			cliInvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_COMMAND:
		if (Clist_MenuProcessCommand(LOWORD(wParam), 0, Frameid))
			break;

		if (HIWORD(wParam) == 0) {//mouse events for self created menu
			int framepos = id2pos(Frameid);
			if (framepos == -1) { break; };

			switch (LOWORD(wParam)) {
			case frame_menu_lock:
				g_pfwFrames[framepos].Locked = !g_pfwFrames[framepos].Locked;
				break;
			case frame_menu_visible:
				g_pfwFrames[framepos].visible = !g_pfwFrames[framepos].visible;
				break;
			case frame_menu_showtitlebar:
				g_pfwFrames[framepos].TitleBar.ShowTitleBar = !g_pfwFrames[framepos].TitleBar.ShowTitleBar;
				break;
			case frame_menu_floating:
				CLUIFrames_SetFrameFloat(Frameid, 0);
				break;
			}
			CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		}
		break;

	case WM_RBUTTONDOWN:
		{
			HMENU hmenu;
			POINT pt;
			GetCursorPos(&pt);

			if (ServiceExists(MS_CLIST_MENUBUILDFRAMECONTEXT))
				hmenu = (HMENU)CallService(MS_CLIST_MENUBUILDFRAMECONTEXT, Frameid, 0);
			else {
				//legacy menu support
				int framepos = id2pos(Frameid);

				if (framepos == -1) { break; };
				hmenu = CreatePopupMenu();
				//				Frames[Frameid].TitleBar.hmenu = hmenu;
				AppendMenu(hmenu, MF_STRING | MF_DISABLED | MF_GRAYED, 15, g_pfwFrames[framepos].name);
				AppendMenu(hmenu, MF_SEPARATOR, 16, L"");
				AppendMenu(hmenu, g_pfwFrames[framepos].Locked ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_lock, TranslateT("Lock frame"));
				AppendMenu(hmenu, g_pfwFrames[framepos].visible ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_visible, TranslateT("Visible"));
				AppendMenu(hmenu, g_pfwFrames[framepos].TitleBar.ShowTitleBar ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_showtitlebar, TranslateT("Show title bar"));
				AppendMenu(hmenu, g_pfwFrames[framepos].floating ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_floating, TranslateT("Floating"));
			}

			TrackPopupMenu(hmenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
			DestroyMenu(hmenu);
		}
		break;

	case WM_LBUTTONDBLCLK:
		Framemod = -1;
		s_nLastByPos = -1; s_nOldFrameHeight = -1; ReleaseCapture();
		CallService(MS_CLIST_FRAMES_UCOLLFRAME, Frameid, 0);
		s_nLastByPos = -1; s_nOldFrameHeight = -1; ReleaseCapture();
		break;

	case WM_LBUTTONUP:
		if (GetCapture() != hwnd)
			break;
		s_nCurDragBar = -1; s_nLastByPos = -1; s_nOldFrameHeight = -1; ReleaseCapture();
		break;

	case WM_LBUTTONDOWN:
		{

			int framepos = id2pos(Frameid);

			if (framepos == -1) { break; };
			{
				if (g_pfwFrames[framepos].floating) {

					POINT pt;
					GetCursorPos(&pt);
					g_pfwFrames[framepos].TitleBar.oldpos = pt;
				}
			}

			//ScreenToClient(Frames[framepos].ContainerWnd,&Frames[framepos].TitleBar.oldpos);

			if ((!(wParam&MK_CONTROL)) && g_pfwFrames[framepos].Locked && (!(g_pfwFrames[framepos].floating))) {
				if (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
					POINT pt;
					int res;
					//pt = nm->pt;
					GetCursorPos(&pt);
					res = SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
					return res;
				}
			};

			if (g_pfwFrames[framepos].floating) {
				RECT rc;
				GetCursorPos(&ptOld);
				//ClientToScreen(hwnd,&ptOld);
				GetWindowRect(hwnd, &rc);

				nLeft = (short)rc.left;
				nTop = (short)rc.top;
			};

			SetCapture(hwnd);
		}
		break;
	
	case WM_MOUSEMOVE:
		{
			POINT pt, pt2;
			RECT wndr;
			int pos;
			//tbinfo
			{
				wchar_t TBcapt[255];
				pos = id2pos(Frameid);
				if (pos != -1) {
					int oldflags;

					mir_snwprintf(TBcapt, L"%s - h:%d, vis:%d, fl:%d, fl:(%d,%d,%d,%d),or: %d",
						g_pfwFrames[pos].name, g_pfwFrames[pos].height, g_pfwFrames[pos].visible, g_pfwFrames[pos].floating,
						g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y,
						g_pfwFrames[pos].FloatingSize.x, g_pfwFrames[pos].FloatingSize.y,
						g_pfwFrames[pos].order);

					oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, g_pfwFrames[pos].id), 0);
					if (!(oldflags&F_SHOWTBTIP)) {
						oldflags |= F_SHOWTBTIP;
						//CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,Frames[pos].id),(LPARAM)oldflags);
					};
					//CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,Frames[pos].id),(LPARAM)TBcapt);
				};



			}
			//
			if ((wParam&MK_LBUTTON)/* && (wParam&MK_CONTROL)*/) {
				RECT rcMiranda;
				RECT rcwnd, rcOverlap;
				POINT newpt, ofspt, curpt, newpos;
				//if (GetCapture() != hwnd){break;};
				//curdragbar = -1;lbypos = -1;oldframeheight = -1;ReleaseCapture();

				pos = id2pos(Frameid);
				if (g_pfwFrames[pos].floating) {

					GetCursorPos(&curpt);
					rcwnd.bottom = curpt.y + 5;
					rcwnd.top = curpt.y;
					rcwnd.left = curpt.x;
					rcwnd.right = curpt.x + 5;

					GetWindowRect(g_clistApi.hwndContactList, &rcMiranda);
					//GetWindowRect( Frames[pos].ContainerWnd, &rcwnd );
					//IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )
					if (IsWindowVisible(g_clistApi.hwndContactList) && IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
						int id = g_pfwFrames[pos].id;




						ofspt.x = 0; ofspt.y = 0;
						ClientToScreen(g_pfwFrames[pos].TitleBar.hwnd, &ofspt);
						ofspt.x = curpt.x - ofspt.x; ofspt.y = curpt.y - ofspt.y;

						CLUIFrames_SetFrameFloat(id, 0);
						newpt.x = 0; newpt.y = 0;
						ClientToScreen(g_pfwFrames[pos].TitleBar.hwnd, &newpt);
						SetCursorPos(newpt.x + ofspt.x, newpt.y + ofspt.y);
						GetCursorPos(&curpt);

						g_pfwFrames[pos].TitleBar.oldpos = curpt;

						return 0;
					};

				}
				else {
					int id = g_pfwFrames[pos].id;

					GetCursorPos(&curpt);
					rcwnd.bottom = curpt.y + 5;
					rcwnd.top = curpt.y;
					rcwnd.left = curpt.x;
					rcwnd.right = curpt.x + 5;

					GetWindowRect(g_clistApi.hwndContactList, &rcMiranda);
					//GetWindowRect( Frames[pos].ContainerWnd, &rcwnd );
					//IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )


					if (!IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
						GetCursorPos(&curpt);
						GetWindowRect(g_pfwFrames[pos].hWnd, &rcwnd);
						rcwnd.left = rcwnd.right - rcwnd.left;
						rcwnd.top = rcwnd.bottom - rcwnd.top;
						newpos.x = curpt.x; newpos.y = curpt.y;
						if (curpt.x >= (rcMiranda.right - 1)) { newpos.x = curpt.x + 5; };
						if (curpt.x <= (rcMiranda.left + 1)) { newpos.x = curpt.x - (rcwnd.left) - 5; };
						if (curpt.y >= (rcMiranda.bottom - 1)) { newpos.y = curpt.y + 5; };
						if (curpt.y <= (rcMiranda.top + 1)) { newpos.y = curpt.y - (rcwnd.top) - 5; };
						ofspt.x = 0; ofspt.y = 0;
						//ClientToScreen(Frames[pos].TitleBar.hwnd,&ofspt);
						GetWindowRect(g_pfwFrames[pos].TitleBar.hwnd, &rcwnd);
						ofspt.x = curpt.x - ofspt.x; ofspt.y = curpt.y - ofspt.y;

						g_pfwFrames[pos].FloatingPos.x = newpos.x;
						g_pfwFrames[pos].FloatingPos.y = newpos.y;
						CLUIFrames_SetFrameFloat(id, 0);
						//SetWindowPos(Frames[pos].ContainerWnd, 0, newpos.x,newpos.y, 0, 0, SWP_NOSIZE);


						newpt.x = 0; newpt.y = 0;
						ClientToScreen(g_pfwFrames[pos].TitleBar.hwnd, &newpt);

						GetWindowRect(g_pfwFrames[pos].hWnd, &rcwnd);
						SetCursorPos(newpt.x + (rcwnd.right - rcwnd.left) / 2, newpt.y + (rcwnd.bottom - rcwnd.top) / 2);
						GetCursorPos(&curpt);

						g_pfwFrames[pos].TitleBar.oldpos = curpt;
						return 0;
					}
				}
			}

			if (wParam & MK_LBUTTON) {
				int newh = -1, prevold;

				if (GetCapture() != hwnd) { break; };


				pos = id2pos(Frameid);

				if (g_pfwFrames[pos].floating) {
					GetCursorPos(&pt);
					if ((g_pfwFrames[pos].TitleBar.oldpos.x != pt.x) || (g_pfwFrames[pos].TitleBar.oldpos.y != pt.y)) {

						pt2 = pt;
						ScreenToClient(hwnd, &pt2);
						GetWindowRect(g_pfwFrames[pos].ContainerWnd, &wndr);
						{
							int dX, dY;
							POINT ptNew;

							ptNew.x = pt.x;
							ptNew.y = pt.y;
							//ClientToScreen( hwnd, &ptNew );

							dX = ptNew.x - ptOld.x;
							dY = ptNew.y - ptOld.y;

							nLeft += (short)dX;
							nTop += (short)dY;

							if (!(wParam&MK_CONTROL))
								PositionThumb(&g_pfwFrames[pos], nLeft, nTop);
							else
								SetWindowPos(g_pfwFrames[pos].ContainerWnd,
									HWND_TOPMOST,
									nLeft,
									nTop,
									0,
									0,
									SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

							ptOld = ptNew;
						}

						pt.x = nLeft;
						pt.y = nTop;
						g_pfwFrames[pos].TitleBar.oldpos = pt;
					};

					//break;
					return 0;
				}

				if (g_pfwFrames[pos].prevvisframe != -1) {
					GetCursorPos(&pt);

					if ((g_pfwFrames[pos].TitleBar.oldpos.x == pt.x) && (g_pfwFrames[pos].TitleBar.oldpos.y == pt.y))
						break;

					ypos = rect.top + pt.y; xpos = rect.left + pt.x;
					Framemod = -1;

					if (g_pfwFrames[pos].align == alBottom) {
						direction = -1;
						Framemod = pos;
					}
					else {
						direction = 1;
						Framemod = g_pfwFrames[pos].prevvisframe;
					}
					if (g_pfwFrames[Framemod].Locked) break;
					if (s_nCurDragBar != -1 && s_nCurDragBar != pos) break;

					if (s_nLastByPos == -1) {
						s_nCurDragBar = pos;
						s_nLastByPos = ypos;
						s_nOldFrameHeight = g_pfwFrames[Framemod].height;
						SetCapture(hwnd);
						break;
					}

					newh = s_nOldFrameHeight + direction*(ypos - s_nLastByPos);
					if (newh > 0) {
						prevold = g_pfwFrames[Framemod].height;
						g_pfwFrames[Framemod].height = newh;
						if (!CLUIFramesFitInSize()) { g_pfwFrames[Framemod].height = prevold; return TRUE; }
						g_pfwFrames[Framemod].height = newh;
						if (newh > 3) g_pfwFrames[Framemod].collapsed = TRUE;

					}
					g_pfwFrames[pos].TitleBar.oldpos = pt;
				}

				if (newh > 0)
					CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
				break;
			}
			s_nCurDragBar = -1; s_nLastByPos = -1; s_nOldFrameHeight = -1; ReleaseCapture();
		}
		break;

	case WM_PRINT:
	case WM_PRINTCLIENT:
		GetClientRect(hwnd, &rect);
		if (!g_CluiData.fLayered)
			ske_BltBackImage(hwnd, (HDC)wParam, &rect);

		DrawTitleBar((HDC)wParam, &rect, Frameid);
		break;

	case WM_SIZE:
		InvalidateRect(hwnd, nullptr, FALSE);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_PAINT:
		if (g_pfwFrames[id2pos(Frameid)].floating || !g_CluiData.fLayered) {
			GetClientRect(hwnd, &rect);
			HDC paintDC = GetDC(hwnd);
			DrawTitleBar(paintDC, &rect, Frameid);
			ReleaseDC(hwnd, paintDC);
			ValidateRect(hwnd, nullptr);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}
static int CLUIFrameResizeFloatingFrame(int framepos)
{
	RECT rect;

	if (!g_pfwFrames[framepos].floating) { return 0; };
	if (g_pfwFrames[framepos].ContainerWnd == nullptr) { return 0; };
	GetClientRect(g_pfwFrames[framepos].ContainerWnd, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	g_pfwFrames[framepos].visible ? CLUI_ShowWindowMod(g_pfwFrames[framepos].ContainerWnd, SW_SHOW/*NOACTIVATE*/) : CLUI_ShowWindowMod(g_pfwFrames[framepos].ContainerWnd, SW_HIDE);



	if (g_pfwFrames[framepos].TitleBar.ShowTitleBar) {
		CLUI_ShowWindowMod(g_pfwFrames[framepos].TitleBar.hwnd, SW_SHOW/*NOACTIVATE*/);
		g_pfwFrames[framepos].height = height - DEFAULT_TITLEBAR_HEIGHT;

		SetWindowPos(g_pfwFrames[framepos].TitleBar.hwnd, HWND_TOP, 0, 0, width, DEFAULT_TITLEBAR_HEIGHT, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_DRAWFRAME);
		SetWindowPos(g_pfwFrames[framepos].hWnd, HWND_TOP, 0, DEFAULT_TITLEBAR_HEIGHT, width, height - DEFAULT_TITLEBAR_HEIGHT, SWP_SHOWWINDOW);

	}
	else {
		g_pfwFrames[framepos].height = height;
		CLUI_ShowWindowMod(g_pfwFrames[framepos].TitleBar.hwnd, SW_HIDE);
		SetWindowPos(g_pfwFrames[framepos].hWnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);

	};
	if (g_pfwFrames[framepos].ContainerWnd != nullptr) UpdateWindow(g_pfwFrames[framepos].ContainerWnd);
	GetWindowRect(g_pfwFrames[framepos].hWnd, &g_pfwFrames[framepos].wndSize);
	return 0;
};

static int CLUIFrameOnMainMenuBuild(WPARAM, LPARAM)
{
	if (MirandaExiting()) return 0;
	CLUIFramesLoadMainMenu();

	return 0;
}

static LRESULT CALLBACK CLUIFrameSubContainerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ACTIVATE:
		if (g_bTransparentFlag) {
			uint8_t alpha;
			if ((wParam != WA_INACTIVE || ((HWND)lParam == hwnd) || GetParent((HWND)lParam) == hwnd)) {
				HWND hw = lParam ? GetParent((HWND)lParam) : nullptr;
				alpha = g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT);
				if (hw) SetWindowPos(hw, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
				CLUI_SmoothAlphaTransition(hwnd, alpha, 1);
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NOTIFY:
	case WM_PARENTNOTIFY:
	case WM_SYSCOMMAND:
		return SendMessage(g_clistApi.hwndContactList, msg, wParam, lParam);

	case WM_MOVE:
		if (g_CluiData.fDocked)
			return 1;
		break;

	case WM_WINDOWPOSCHANGING:
		if (g_CluiData.mutexPreventDockMoving) {
			WINDOWPOS *wp;
			wp = (WINDOWPOS*)lParam;
			wp->flags |= SWP_NOMOVE/*|SWP_NOSIZE*/;
			wp->flags &= (wp->flags&~SWP_NOACTIVATE);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		break;

	case WM_WINDOWPOSCHANGED:
		if (g_CluiData.fDocked && g_CluiData.mutexPreventDockMoving)
			return 0;
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_ERASEBKGND:
		return 1;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND CreateSubContainerWindow(HWND parent, int x, int y, int width, int height)
{
	HWND hwnd = CreateWindowEx(WS_EX_LAYERED, CLUIFrameSubContainerClassName, L"SubContainerWindow", WS_POPUP | (!g_CluiData.fLayered ? WS_BORDER : 0), x, y, width, height, parent, nullptr, g_plugin.getInst(), nullptr);
	SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER));
	if (g_CluiData.fOnDesktop) {
		HWND hProgMan = FindWindow(L"Progman", nullptr);
		if (IsWindow(hProgMan))
			SetParent(hwnd, hProgMan);
	}

	return hwnd;
}

static LRESULT CALLBACK CLUIFrameContainerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int framepos, Frameid = GetWindowLongPtr(hwnd, GWLP_USERDATA);

	RECT rect;
	memset(&rect, 0, sizeof(rect));

	switch (msg) {
	case WM_CREATE:
		framepos = id2pos(Frameid);
		return 0;

	case WM_GETMINMAXINFO:
		MINMAXINFO minmax;
		framepos = id2pos(Frameid);
		if (framepos < 0 || framepos >= g_nFramesCount) { break; };
		if (!g_pfwFrames[framepos].minmaxenabled) { break; };
		if (g_pfwFrames[framepos].ContainerWnd == nullptr) { break; };

		if (g_pfwFrames[framepos].Locked) {
			RECT rct;

			GetWindowRect(hwnd, &rct);
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rct.right - rct.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rct.bottom - rct.top;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rct.right - rct.left;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = rct.bottom - rct.top;
			//
			//return 0;
		}

		memset(&minmax, 0, sizeof(minmax));
		if (SendMessage(g_pfwFrames[framepos].hWnd, WM_GETMINMAXINFO, 0, (LPARAM)&minmax) == 0) {
			RECT border;
			int tbh = g_nTitleBarHeight*btoint(g_pfwFrames[framepos].TitleBar.ShowTitleBar);
			GetBorderSize(hwnd, &border);
			if (minmax.ptMaxTrackSize.x != 0 && minmax.ptMaxTrackSize.y != 0) {

				((LPMINMAXINFO)lParam)->ptMinTrackSize.x = minmax.ptMinTrackSize.x;
				((LPMINMAXINFO)lParam)->ptMinTrackSize.y = minmax.ptMinTrackSize.y;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = minmax.ptMaxTrackSize.x + border.left + border.right;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = minmax.ptMaxTrackSize.y + tbh + border.top + border.bottom;
			}
		}
		else return(DefWindowProc(hwnd, msg, wParam, lParam));

	case WM_MOVE:
		framepos = id2pos(Frameid);
		if (framepos < 0 || framepos >= g_nFramesCount)
			break;
		if (g_pfwFrames[framepos].ContainerWnd == nullptr)
			return 0;

		GetWindowRect(g_pfwFrames[framepos].ContainerWnd, &rect);
		g_pfwFrames[framepos].FloatingPos.x = rect.left;
		g_pfwFrames[framepos].FloatingPos.y = rect.top;
		g_pfwFrames[framepos].FloatingSize.x = rect.right - rect.left;
		g_pfwFrames[framepos].FloatingSize.y = rect.bottom - rect.top;

		CLUIFramesStoreFrameSettings(framepos);

		if (g_pfwFrames[framepos].floating) {
			POINT curpt, ofspt, newpt, newpos;
			RECT rcwnd, rcOverlap, rcMiranda;

			GetCursorPos(&curpt);
			rcwnd.bottom = curpt.y + 5;
			rcwnd.top = curpt.y;
			rcwnd.left = curpt.x;
			rcwnd.right = curpt.x + 5;

			GetWindowRect(g_clistApi.hwndContactList, &rcMiranda);
			if (IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
				GetCursorPos(&curpt);
				GetWindowRect(g_pfwFrames[framepos].hWnd, &rcwnd);
				rcwnd.left = rcwnd.right - rcwnd.left;
				rcwnd.top = rcwnd.bottom - rcwnd.top;
				newpos.x = curpt.x; newpos.y = curpt.y;
				if (curpt.x >= (rcMiranda.right - 1)) { newpos.x = curpt.x + 5; };
				if (curpt.x <= (rcMiranda.left + 1)) { newpos.x = curpt.x - (rcwnd.left) - 5; };
				if (curpt.y >= (rcMiranda.bottom - 1)) { newpos.y = curpt.y + 5; };
				if (curpt.y <= (rcMiranda.top + 1)) { newpos.y = curpt.y - (rcwnd.top) - 5; };
				ofspt.x = 0; ofspt.y = 0;
				GetWindowRect(g_pfwFrames[framepos].TitleBar.hwnd, &rcwnd);
				ofspt.x = curpt.x - ofspt.x; ofspt.y = curpt.y - ofspt.y;

				g_pfwFrames[framepos].FloatingPos.x = newpos.x;
				g_pfwFrames[framepos].FloatingPos.y = newpos.y;
				CLUIFrames_SetFrameFloat(Frameid, 0);

				newpt.x = 0; newpt.y = 0;
				ClientToScreen(g_pfwFrames[framepos].TitleBar.hwnd, &newpt);

				GetWindowRect(g_pfwFrames[framepos].hWnd, &rcwnd);
				SetCursorPos(newpt.x + (rcwnd.right - rcwnd.left) / 2, newpt.y + (rcwnd.bottom - rcwnd.top) / 2);
				GetCursorPos(&curpt);

				g_pfwFrames[framepos].TitleBar.oldpos = curpt;
				return 0;
			}
		}

		return 0;

	case WM_SIZE:
		CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);

		framepos = id2pos(Frameid);

		if (framepos < 0 || framepos >= g_nFramesCount) { break; };
		if (g_pfwFrames[framepos].ContainerWnd == nullptr) { return 0; };
		CLUIFrameResizeFloatingFrame(framepos);

		GetWindowRect(g_pfwFrames[framepos].ContainerWnd, &rect);
		g_pfwFrames[framepos].FloatingPos.x = rect.left;
		g_pfwFrames[framepos].FloatingPos.y = rect.top;
		g_pfwFrames[framepos].FloatingSize.x = rect.right - rect.left;
		g_pfwFrames[framepos].FloatingSize.y = rect.bottom - rect.top;

		CLUIFramesStoreFrameSettings(framepos);
		return 0;

	case WM_LBUTTONDOWN:
		if (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
			POINT pt;
			GetCursorPos(&pt);
			return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND CreateContainerWindow(HWND parent, int x, int y, int width, int height)
{
	return CreateWindow(L"FramesContainer", L"FramesContainer", WS_POPUP | WS_THICKFRAME, x, y, width, height, parent, nullptr, g_plugin.getInst(), nullptr);
}

static int _us_DoSetFrameFloat(WPARAM wParam, LPARAM lParam)
{
	HWND hwndtmp, hwndtooltiptmp;

	int pos = id2pos(wParam);
	if (pos < 0 || pos >= g_nFramesCount)
		return 0;

	auto &F = g_pfwFrames[pos];
	if (F.floating || (lParam & 2)) {
		if (F.OwnerWindow != (HWND)-2 && F.visible) {
			if (F.OwnerWindow == nullptr) F.OwnerWindow = CreateSubContainerWindow(g_clistApi.hwndContactList, F.FloatingPos.x, F.FloatingPos.y, 10, 10);
			CLUI_ShowWindowMod(F.OwnerWindow, (F.visible && F.collapsed && IsWindowVisible(g_clistApi.hwndContactList)) ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
			SetParent(F.hWnd, F.OwnerWindow);
			SetParent(F.TitleBar.hwnd, g_clistApi.hwndContactList);
			SetWindowLongPtr(F.OwnerWindow, GWLP_USERDATA, F.id);
			F.floating = FALSE;
			if (!(lParam & 2)) {
				DestroyWindow(F.ContainerWnd);
				F.ContainerWnd = nullptr;
			}
		}
		else {
			SetParent(F.hWnd, g_clistApi.hwndContactList);
			SetParent(F.TitleBar.hwnd, g_clistApi.hwndContactList);
			F.floating = FALSE;
			if (F.ContainerWnd) DestroyWindow(F.ContainerWnd);
			F.ContainerWnd = nullptr;
		}
	}
	else {
		RECT recttb, rectw, border;
		LONG_PTR temp;
		int neww, newh;

		F.oldstyles = GetWindowLongPtr(F.hWnd, GWL_STYLE);
		F.TitleBar.oldstyles = GetWindowLongPtr(F.TitleBar.hwnd, GWL_STYLE);
		bool locked = F.Locked;
		F.Locked = FALSE;
		F.minmaxenabled = FALSE;

		GetWindowRect(F.hWnd, &rectw);
		GetWindowRect(F.TitleBar.hwnd, &recttb);
		if (!F.TitleBar.ShowTitleBar)
			recttb.top = recttb.bottom = recttb.left = recttb.right = 0;

		F.ContainerWnd = CreateContainerWindow(g_clistApi.hwndContactList, F.FloatingPos.x, F.FloatingPos.y, 10, 10);

		SetParent(F.hWnd, F.ContainerWnd);
		SetParent(F.TitleBar.hwnd, F.ContainerWnd);
		if (F.OwnerWindow != (HWND)-2 && F.OwnerWindow != nullptr) {
			DestroyWindow(F.OwnerWindow);
			F.OwnerWindow = nullptr;
		}

		GetBorderSize(F.ContainerWnd, &border);

		SetWindowLongPtr(F.ContainerWnd, GWLP_USERDATA, F.id);
		if ((lParam == 1)) {
			if (F.FloatingPos.x < 0) { F.FloatingPos.x = 0; };
			if (F.FloatingPos.y < 0) { F.FloatingPos.y = 0; };

			SetWindowPos(F.ContainerWnd, HWND_TOPMOST, F.FloatingPos.x, F.FloatingPos.y, F.FloatingSize.x, F.FloatingSize.y, SWP_HIDEWINDOW | SWP_NOACTIVATE);
		}
		else if (lParam == 0) {
			neww = rectw.right - rectw.left + border.left + border.right;
			newh = (rectw.bottom - rectw.top) + (recttb.bottom - recttb.top) + border.top + border.bottom;
			if (neww < 20) { neww = 40; };
			if (newh < 20) { newh = 40; };
			if (F.FloatingPos.x < 20) { F.FloatingPos.x = 40; };
			if (F.FloatingPos.y < 20) { F.FloatingPos.y = 40; };

			SetWindowPos(F.ContainerWnd, HWND_TOPMOST, F.FloatingPos.x, F.FloatingPos.y, neww, newh, SWP_HIDEWINDOW | SWP_NOACTIVATE);
		}

		SetWindowText(F.ContainerWnd, F.TitleBar.tbname);

		temp = GetWindowLongPtr(F.ContainerWnd, GWL_EXSTYLE);
		temp |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
		SetWindowLongPtr(F.ContainerWnd, GWL_EXSTYLE, temp);

		F.floating = TRUE;
		F.Locked = locked;
	}

	CLUIFramesStoreFrameSettings(pos);
	F.minmaxenabled = TRUE;
	hwndtooltiptmp = F.TitleBar.hwndTip;

	hwndtmp = F.ContainerWnd;

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	if (hwndtmp)
		SendMessage(hwndtmp, WM_SIZE, 0, 0);

	SetWindowPos(hwndtooltiptmp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	return 0;
}

int CLUIFrameOnModulesLoad(WPARAM, LPARAM)
{
	/* HOOK */
	CLUIFramesLoadMainMenu();
	CLUIFramesCreateMenuForFrame(-1, nullptr, 000010000, false);
	return 0;
}

int CLUIFrameOnModulesUnload(WPARAM, LPARAM)
{
	g_CluiData.bSTATE = STATE_PREPARETOEXIT;

	if (cont.MIVisible) {
		cont.MIVisible = nullptr;
		_AniAva_OnModulesUnload();
	}
	return 0;
}

int LoadCLUIFramesModule(void)
{
	_cluiFramesModuleCSInitialized = TRUE;

	WNDCLASS wndclass = { 0 };
	wndclass.style = CS_DBLCLKS;//|CS_HREDRAW|CS_VREDRAW ;
	wndclass.lpfnWndProc = CLUIFrameTitleBarProc;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wndclass.lpszClassName = CLUIFrameTitleBarClassName;
	RegisterClass(&wndclass);

	WNDCLASS subconclass = { 0 };
	subconclass.style = CS_DBLCLKS;//|CS_HREDRAW|CS_VREDRAW ;
	subconclass.lpfnWndProc = CLUIFrameSubContainerProc;
	subconclass.hInstance = g_plugin.getInst();
	subconclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	subconclass.lpszClassName = CLUIFrameSubContainerClassName;
	RegisterClass(&subconclass);

	//container helper
	WNDCLASS cntclass = { 0 };
	cntclass.style = CS_DBLCLKS/*|CS_HREDRAW|CS_VREDRAW*/ | CS_DROPSHADOW;
	cntclass.lpfnWndProc = CLUIFrameContainerWndProc;
	cntclass.hInstance = g_plugin.getInst();
	cntclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	cntclass.lpszClassName = L"FramesContainer";
	RegisterClass(&cntclass);
	//end container helper

	GapBetweenFrames = db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);

	g_nFramesCount = 0;
	InitGroupMenus();
	{
		// create root menu
		CMenuItem mi(&g_plugin);
		SET_UID(mi, 0x3931AC4, 0x7A32, 0x4D9C, 0x99, 0x92, 0x94, 0xD4, 0xB5, 0x9B, 0xD6, 0xB6);
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FRAME);
		mi.position = 3000090000;
		mi.name.a = LPGEN("Frames");
		cont.MainMenuItem = Menu_AddMainMenuItem(&mi);
		UNSET_UID(mi);

		mi.root = cont.MainMenuItem;
		mi.hIcolibItem = nullptr;
		mi.flags = CMIF_UNMOVABLE;

		// create "show all frames" menu
		mi.uid.d[7]++;
		mi.position = 4000090000;
		mi.name.a = LPGEN("Show all frames");
		mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMES;
		Menu_AddMainMenuItem(&mi);

		// create "show all titlebars" menu
		mi.uid.d[7]++;
		mi.position++;
		mi.name.a = LPGEN("Show all title bars");
		mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMESTB;
		Menu_AddMainMenuItem(&mi);

		// create "hide all titlebars" menu
		mi.uid.d[7]++;
		mi.position++;
		mi.name.a = LPGEN("Hide all title bars");
		mi.pszService = MS_CLIST_FRAMES_HIDEALLFRAMESTB;
		Menu_AddMainMenuItem(&mi);
	}

	HookEvent(ME_SYSTEM_MODULESLOADED, CLUIFrameOnModulesLoad);
	HookEvent(ME_CLIST_PREBUILDFRAMEMENU, CLUIFramesModifyContextMenuForFrame);
	HookEvent(ME_CLIST_PREBUILDMAINMENU, CLUIFrameOnMainMenuBuild);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, CLUIFrameOnModulesUnload);

	CreateCluiFramesServices();

	hWndExplorerToolBar = FindWindowEx(nullptr, nullptr, L"Shell_TrayWnd", nullptr);
	OnFrameTitleBarBackgroundChange(0, 0);
	_fCluiFramesModuleNotStarted = FALSE;
	return 0;
}

int UnLoadCLUIFramesModule(void)
{
	_fCluiFramesModuleNotStarted = TRUE;
	if (sttBmpBackground) {
		DeleteObject(sttBmpBackground);
		sttBmpBackground = nullptr;
	}
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	CLUIFramesStoreAllFrames();

	CLUIFrameOnModulesUnload(0, 0);
	if (cont.MainMenuItem != nullptr) {
		Menu_RemoveItem(cont.MainMenuItem);
		cont.MainMenuItem = nullptr;
	}

	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (F.hWnd != g_clistApi.hwndContactTree)
			DestroyWindow(F.hWnd);

		F.hWnd = (HWND)-1;
		DestroyWindow(F.TitleBar.hwnd);
		F.TitleBar.hwnd = (HWND)-1;
		if (F.ContainerWnd && F.ContainerWnd != (HWND)(-2) && F.ContainerWnd != (HWND)(-1))
			DestroyWindow(F.ContainerWnd);
		F.ContainerWnd = (HWND)-1;
		if (F.TitleBar.hmenu)
			DestroyMenu(F.TitleBar.hmenu);
		if (F.OwnerWindow && F.OwnerWindow != (HWND)(-2) && F.OwnerWindow != (HWND)(-1))
			DestroyWindow(F.OwnerWindow);
		F.OwnerWindow = (HWND)-2;
		if (F.UpdateRgn)
			DeleteObject(F.UpdateRgn);

		mir_free(F.name);
		mir_free(F.TitleBar.tbname);
		mir_free(F.TitleBar.tooltip);
	}

	free(g_pfwFrames);
	g_pfwFrames = nullptr;
	g_nFramesCount = 0;
	UnregisterClass(CLUIFrameTitleBarClassName, g_plugin.getInst());
	DeleteObject(_hTitleBarFont);

	_cluiFramesModuleCSInitialized = FALSE;
	return 0;
}

static int _us_DoSetFramePaintProc(WPARAM wParam, LPARAM lParam)
{
	if (!wParam) return 0;

	// LOOK REQUIRED OR SYNC CALL NEEDED
	FRAMEWND *frm = FindFrameByItsHWND((HWND)wParam);
	if (!frm) return 0;
	if (lParam)
		frm->PaintCallbackProc = (tPaintCallbackProc)lParam;
	else
		frm->PaintCallbackProc = nullptr;
	return 1;
}

int CLUIFrames_SetLayeredMode(BOOL fLayeredMode, HWND hwnd)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (fLayeredMode) {
			if (F.visible && GetParent(F.hWnd) == g_clistApi.hwndContactList && F.PaintCallbackProc == nullptr) {
				//create owner window
				F.OwnerWindow = CreateSubContainerWindow(g_clistApi.hwndContactList, F.FloatingPos.x, F.FloatingPos.y, 10, 10);
				SetParent(F.hWnd, F.OwnerWindow);
			}
		}
		else if (GetParent(F.hWnd) == F.OwnerWindow) {
			SetParent(F.hWnd, hwnd);
			if ((INT_PTR)F.OwnerWindow > 0) {
				DestroyWindow(F.OwnerWindow);
				F.OwnerWindow = (HWND)-2;
			}
		}
	}

	CLUIFrames_UpdateFrame((WPARAM)-1, 0);  //update all frames
	return 0;
}

int CLUIFrames_UpdateBorders()
{
	for (int i = 0; i < g_nFramesCount; i++) {
		FRAMEWND &F = g_pfwFrames[i];
		if (!F.floating) {
			LONG_PTR style = GetWindowLongPtr(F.hWnd, GWL_STYLE) & (~WS_BORDER);
			if (!g_CluiData.fLayered && F.UseBorder) style |= WS_BORDER;
			SetWindowLongPtr(F.hWnd, GWL_STYLE, style);
			CLUIFramesModifyMainMenuItems(F.id, 0);
			RedrawWindow(F.hWnd, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	return 0;
}
