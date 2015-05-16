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

#include "../hdr/modern_commonheaders.h"
#include "m_skin_eng.h"
#include "../hdr/modern_commonprototypes.h"
#include "../hdr/modern_sync.h"

// ALL THIS MODULE FUNCTION SHOULD BE EXECUTED FROM MAIN THREAD

#include "../hdr/modern_static_cluiframes_service.h" // contain services initialization and proxiation

static const int UNCOLLAPSED_FRAME_SIZE = 0;
static const int DEFAULT_TITLEBAR_HEIGHT = 18;

void _AniAva_OnModulesUnload();

//GLOBALS
FRAMEWND *g_pfwFrames = NULL;
int g_nFramesCount = 0;
int g_nTitleBarHeight = DEFAULT_TITLEBAR_HEIGHT;

// STATIC

enum {
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



static mir_cs _cluiFramesModuleCS;
static BOOL				_cluiFramesModuleCSInitialized = FALSE;
static BOOL				_fCluiFramesModuleNotStarted = TRUE;
static XPTHANDLE		_hFrameTitleTheme = NULL;
static int				_nClientFrameId = eUnknownId; //for fast access to frame with alclient properties
static int				_iNextFrameId = 100;
static HFONT			_hTitleBarFont = NULL;

// menus
static HGENMENU _hmiVisible, _hmiTBVisible, _hmiLock, _hmiColl;
static HGENMENU _hmiFloating, _hmiAlignRoot, _hmiAlignTop, _hmiAlignClient, _hmiAlignBottom;
static HGENMENU _hmiPosRoot, _hmiPosUp, _hmiPosDown, _hmiBorder, _hmiRoot;

// others
static int _nContactListHeight = 0;
static DWORD _dwLastStoreTick = 0;

HWND hWndExplorerToolBar;
static int GapBetweenFrames = 1;

BOOLEAN bMoveTogether;
int recurs_prevent = 0;
static BOOL PreventSizeCalling = FALSE;

static HBITMAP hBmpBackground;
static int backgroundBmpUse;
static COLORREF bkColour;
static COLORREF SelBkColour;
static BOOL bkUseWinColours;
BYTE AlignCOLLIconToLeft; //will hide frame icon
COLORREF sttGetColor(char * module, char * color, COLORREF defColor);
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
	if (sd1->order < sd2->order) { return(-1); };
	return (0);
};
int CLUIFrames_OnMoving(HWND hwnd, RECT *r)
{
	g_CluiData.mutexPreventDockMoving = 0;

	for (int i = 0; i < g_nFramesCount; i++) {
		if (!g_pfwFrames[i].floating && g_pfwFrames[i].OwnerWindow != NULL  && g_pfwFrames[i].OwnerWindow != (HWND)-2) {
			POINT pt = { 0 };
			RECT wr;
			FRAMEWND *Frame = &g_pfwFrames[i];

			GetWindowRect(hwnd, &wr);
			ClientToScreen(hwnd, &pt);
			int dx = (r->left - wr.left) + pt.x;
			int dy = (r->top - wr.top) + pt.y;
			int x = Frame->wndSize.left;
			int y = Frame->wndSize.top;
			SetWindowPos(Frame->OwnerWindow, NULL, x + dx, y + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOOWNERZORDER);
		};

	}
	g_CluiData.mutexPreventDockMoving = 1;
	AniAva_RedrawAllAvatars(FALSE);
	return 0;
}

int SetAlpha(BYTE Alpha)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		if (!g_pfwFrames[i].floating && g_pfwFrames[i].OwnerWindow != NULL  && g_pfwFrames[i].OwnerWindow != (HWND)-2 && g_pfwFrames[i].visible && !g_pfwFrames[i].needhide) {
			HWND hwnd = g_pfwFrames[i].OwnerWindow;
			long l = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			if (!(l & WS_EX_LAYERED)) {
				HWND parent = NULL;
				if (g_CluiData.fOnDesktop) {
					HWND hProgMan = FindWindow(_T("Progman"), NULL);
					if (IsWindow(hProgMan))
						parent = hProgMan;
				}

				CLUI_ShowWindowMod(hwnd, SW_HIDE);
				SetParent(hwnd, NULL);
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
	for (int i = 0; i < g_nFramesCount; i++)
		if (!g_pfwFrames[i].floating && g_pfwFrames[i].OwnerWindow != (HWND)0 && g_pfwFrames[i].OwnerWindow != (HWND)-2 && g_pfwFrames[i].visible && !g_pfwFrames[i].needhide)
			RedrawWindow(g_pfwFrames[i].hWnd, NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_INVALIDATE | RDW_FRAME);

	return 0;
}

int CLUIFrames_ActivateSubContainers(BOOL active)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		if (active && !g_pfwFrames[i].floating && g_pfwFrames[i].OwnerWindow != (HWND)0 && g_pfwFrames[i].OwnerWindow != (HWND)-2 && g_pfwFrames[i].visible && !g_pfwFrames[i].needhide) {
			if (db_get_b(NULL, "CList", "OnDesktop", SETTING_ONDESKTOP_DEFAULT)) {
				SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	}
	return 0;
}

int CLUIFrames_SetParentForContainers(HWND parent)
{
	g_CluiData.fOnDesktop = (parent && parent != pcli->hwndContactList);

	for (int i = 0; i < g_nFramesCount; i++) {
		if (!g_pfwFrames[i].floating && g_pfwFrames[i].OwnerWindow != (HWND)0 && g_pfwFrames[i].OwnerWindow != (HWND)-2 && g_pfwFrames[i].visible && !g_pfwFrames[i].needhide) {
			HWND hwnd = g_pfwFrames[i].OwnerWindow;
			SetParent(hwnd, parent);
		}
	}
	return 0;
}

int CLUIFrames_OnShowHide(int mode)
{
	int prevFrameCount;
	for (int i = 0; i < g_nFramesCount; i++) {
		if (!g_pfwFrames[i].floating && g_pfwFrames[i].OwnerWindow != (HWND)0 && g_pfwFrames[i].OwnerWindow != (HWND)-2) {
			// Try to avoid crash on exit due to unlock.
			HWND owner = g_pfwFrames[i].OwnerWindow;
			HWND Frmhwnd = g_pfwFrames[i].hWnd;
			BOOL visible = g_pfwFrames[i].visible;
			BOOL needhide = g_pfwFrames[i].needhide;
			needhide |= (!g_pfwFrames[i].collapsed || g_pfwFrames[i].height == 0);
			prevFrameCount = g_nFramesCount;
			ShowWindow(owner, (mode == SW_HIDE || !visible || needhide) ? SW_HIDE : mode);
			ShowWindow(Frmhwnd, (mode == SW_HIDE || !visible || needhide) ? SW_HIDE : mode);
		}

		if (mode != SW_HIDE) {
			SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			if (db_get_b(NULL, "CList", "OnDesktop", SETTING_ONDESKTOP_DEFAULT)) {
				SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else SetWindowPos(g_pfwFrames[i].OwnerWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	}

	if (mode != SW_HIDE)
		SetForegroundWindow(pcli->hwndContactList);
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

	if (hwnd == NULL) return(NULL);

	for (i = 0; i < g_nFramesCount; i++)
		if ((g_pfwFrames[i].floating) && (g_pfwFrames[i].ContainerWnd == hwnd))
			return(&g_pfwFrames[i]);

	return(NULL);
}


int QueueAllFramesUpdating(BYTE queue)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		if (!g_CluiData.fLayered) {
			if (queue)
				InvalidateRect(g_pfwFrames[i].hWnd, NULL, FALSE);
			else
				ValidateRect(g_pfwFrames[i].hWnd, NULL);
		}
		if (g_pfwFrames[i].PaintCallbackProc) {
			g_pfwFrames[i].bQueued = queue;
			if (g_pfwFrames[i].UpdateRgn) {
				DeleteObject(g_pfwFrames[i].UpdateRgn);
			}
			g_pfwFrames[i].UpdateRgn = 0;
		}
	}
	return queue;
}

int FindFrameID(HWND FrameHwnd)
{
	if (FrameHwnd == NULL)
		return 0;

	FRAMEWND *frm = FindFrameByItsHWND(FrameHwnd);
	return (frm) ? frm->id : 0;
}

FRAMEWND* FindFrameByItsHWND(HWND FrameHwnd)
{
	if (FrameHwnd == NULL) return(NULL);
	for (int i = 0; i < g_nFramesCount; i++)
		if (g_pfwFrames[i].hWnd == FrameHwnd)
			return &g_pfwFrames[i];

	return NULL;
}

static void DockThumbs(FRAMEWND *pThumbLeft, FRAMEWND *pThumbRight)
{
	if ((pThumbRight->dockOpt.hwndLeft == NULL) && (pThumbLeft->dockOpt.hwndRight == NULL)) {
		pThumbRight->dockOpt.hwndLeft = pThumbLeft->ContainerWnd;
		pThumbLeft->dockOpt.hwndRight = pThumbRight->ContainerWnd;
	}
}


static void UndockThumbs(FRAMEWND *pThumb1, FRAMEWND *pThumb2)
{
	if ((pThumb1 == NULL) || (pThumb2 == NULL))
		return;

	if (pThumb1->dockOpt.hwndRight == pThumb2->ContainerWnd)
		pThumb1->dockOpt.hwndRight = NULL;

	if (pThumb1->dockOpt.hwndLeft == pThumb2->ContainerWnd)
		pThumb1->dockOpt.hwndLeft = NULL;

	if (pThumb2->dockOpt.hwndRight == pThumb1->ContainerWnd)
		pThumb2->dockOpt.hwndRight = NULL;

	if (pThumb2->dockOpt.hwndLeft == pThumb1->ContainerWnd)
		pThumb2->dockOpt.hwndLeft = NULL;
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
	SIZE		sizeScreen;
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

	if (pThumb == NULL) return;

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

	bLeading = pThumb->dockOpt.hwndRight != NULL;

	if (bMoveTogether) {
		UndockThumbs(pThumb, FindFrameByWnd(pThumb->dockOpt.hwndLeft));
		GetWindowRect(pThumb->ContainerWnd, &rcOld);
	}

	memset(&fakeMainWindow, 0, sizeof(fakeMainWindow));
	fakeMainWindow.ContainerWnd = pcli->hwndContactList;
	fakeMainWindow.floating = TRUE;

	memset(&fakeTaskBarWindow, 0, sizeof(fakeTaskBarWindow));
	fakeTaskBarWindow.ContainerWnd = hWndExplorerToolBar;
	fakeTaskBarWindow.floating = TRUE;


	while (pCurThumb != NULL) {
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
		};
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

static char __inline *AS(char *str, const char *setting, char *addstr)
{
	if (str != NULL) {
		mir_strcpy(str, setting);
		mir_strcat(str, addstr);
	}
	return str;
}

static int DBLoadFrameSettingsAtPos(int pos, int Frameid)
{
	char sadd[15];
	char buf[255];
	//	char *oldtb;

	_itoa(pos, sadd, 10);

	//db_set_s(0, CLUIFrameModule,strcat("Name",sadd),Frames[Frameid].name);
	//bool
	g_pfwFrames[Frameid].collapsed = db_get_b(0, CLUIFrameModule, AS(buf, "Collapse", sadd), g_pfwFrames[Frameid].collapsed);

	g_pfwFrames[Frameid].Locked = db_get_b(0, CLUIFrameModule, AS(buf, "Locked", sadd), g_pfwFrames[Frameid].Locked);
	g_pfwFrames[Frameid].visible = db_get_b(0, CLUIFrameModule, AS(buf, "Visible", sadd), g_pfwFrames[Frameid].visible);
	g_pfwFrames[Frameid].TitleBar.ShowTitleBar = db_get_b(0, CLUIFrameModule, AS(buf, "TBVisile", sadd), g_pfwFrames[Frameid].TitleBar.ShowTitleBar);

	g_pfwFrames[Frameid].height = db_get_w(0, CLUIFrameModule, AS(buf, "Height", sadd), g_pfwFrames[Frameid].height);
	g_pfwFrames[Frameid].HeightWhenCollapsed = db_get_w(0, CLUIFrameModule, AS(buf, "HeightCollapsed", sadd), 0);
	g_pfwFrames[Frameid].align = db_get_w(0, CLUIFrameModule, AS(buf, "Align", sadd), g_pfwFrames[Frameid].align);

	g_pfwFrames[Frameid].FloatingPos.x = DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatX", sadd), 100, 0, 2048);
	g_pfwFrames[Frameid].FloatingPos.y = DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatY", sadd), 100, 0, 2048);
	g_pfwFrames[Frameid].FloatingSize.x = DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatW", sadd), 100, 0, 2048);
	g_pfwFrames[Frameid].FloatingSize.y = DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatH", sadd), 100, 0, 2048);

	g_pfwFrames[Frameid].floating = db_get_b(0, CLUIFrameModule, AS(buf, "Floating", sadd), 0);
	g_pfwFrames[Frameid].order = db_get_w(0, CLUIFrameModule, AS(buf, "Order", sadd), 0);

	g_pfwFrames[Frameid].UseBorder = db_get_b(0, CLUIFrameModule, AS(buf, "UseBorder", sadd), g_pfwFrames[Frameid].UseBorder);

	return 0;
}

static int DBStoreFrameSettingsAtPos(int pos, int Frameid)
{
	char sadd[16];
	char buf[255];

	_itoa(pos, sadd, 10);

	db_set_ws(0, CLUIFrameModule, AS(buf, "Name", sadd), g_pfwFrames[Frameid].name);
	//bool
	db_set_b(0, CLUIFrameModule, AS(buf, "Collapse", sadd), (BYTE)btoint(g_pfwFrames[Frameid].collapsed));
	db_set_b(0, CLUIFrameModule, AS(buf, "Locked", sadd), (BYTE)btoint(g_pfwFrames[Frameid].Locked));
	db_set_b(0, CLUIFrameModule, AS(buf, "Visible", sadd), (BYTE)btoint(g_pfwFrames[Frameid].visible));
	db_set_b(0, CLUIFrameModule, AS(buf, "TBVisile", sadd), (BYTE)btoint(g_pfwFrames[Frameid].TitleBar.ShowTitleBar));

	db_set_w(0, CLUIFrameModule, AS(buf, "Height", sadd), (WORD)g_pfwFrames[Frameid].height);
	db_set_w(0, CLUIFrameModule, AS(buf, "HeightCollapsed", sadd), (WORD)g_pfwFrames[Frameid].HeightWhenCollapsed);
	db_set_w(0, CLUIFrameModule, AS(buf, "Align", sadd), (WORD)g_pfwFrames[Frameid].align);
	//FloatingPos
	db_set_w(0, CLUIFrameModule, AS(buf, "FloatX", sadd), (WORD)g_pfwFrames[Frameid].FloatingPos.x);
	db_set_w(0, CLUIFrameModule, AS(buf, "FloatY", sadd), (WORD)g_pfwFrames[Frameid].FloatingPos.y);
	db_set_w(0, CLUIFrameModule, AS(buf, "FloatW", sadd), (WORD)g_pfwFrames[Frameid].FloatingSize.x);
	db_set_w(0, CLUIFrameModule, AS(buf, "FloatH", sadd), (WORD)g_pfwFrames[Frameid].FloatingSize.y);

	db_set_b(0, CLUIFrameModule, AS(buf, "Floating", sadd), (BYTE)btoint(g_pfwFrames[Frameid].floating));
	db_set_b(0, CLUIFrameModule, AS(buf, "UseBorder", sadd), (BYTE)btoint(g_pfwFrames[Frameid].UseBorder));
	db_set_w(0, CLUIFrameModule, AS(buf, "Order", sadd), (WORD)g_pfwFrames[Frameid].order);
	return 0;
}

static int LocateStorePosition(int Frameid, int maxstored)
{
	if (g_pfwFrames[Frameid].name == NULL)
		return -1;

	char settingname[255];
	for (int i = 0; i < maxstored; i++) {
		mir_snprintf(settingname, SIZEOF(settingname), "Name%d", i);
		DBVARIANT dbv = { 0 };
		if (db_get_ts(NULL, CLUIFrameModule, settingname, &dbv))
			continue;

		if (mir_tstrcmpi(dbv.ptszVal, g_pfwFrames[Frameid].name) == 0) {
			db_free(&dbv);
			return i;
		}
		db_free(&dbv);
	}
	return -1;
}

static int CLUIFramesLoadFrameSettings(int Frameid)
{
	int storpos, maxstored;

	if (_fCluiFramesModuleNotStarted) return -1;

	if (Frameid < 0 || Frameid >= g_nFramesCount) return -1;

	maxstored = db_get_w(0, CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1) return 0;

	storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1) return 0;

	DBLoadFrameSettingsAtPos(storpos, Frameid);
	return 0;
}

static int CLUIFramesStoreFrameSettings(int Frameid)
{
	int maxstored, storpos;

	if (_fCluiFramesModuleNotStarted) return -1;

	if (Frameid < 0 || Frameid >= g_nFramesCount) return -1;

	maxstored = db_get_w(0, CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1) maxstored = 0;

	storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1) { storpos = maxstored; maxstored++; }

	DBStoreFrameSettingsAtPos(storpos, Frameid);
	db_set_w(0, CLUIFrameModule, "StoredFrames", (WORD)maxstored);

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

static HMENU CLUIFramesCreateMenuForFrame(int frameid, int root, int popuppos, HGENMENU(*pfnAdd)(CLISTMENUITEM*))
{
	//TMO_MenuItem tmi;
	HGENMENU menuid;
	int framepos = id2pos(frameid);

	if (_fCluiFramesModuleNotStarted)
		return NULL;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Visible");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszContactOwner = (char *)0;
	mi.pszService = MS_CLIST_FRAMES_SHFRAME;
	menuid = pfnAdd(&mi);
	if (frameid == -1) _hmiVisible = menuid;
	else g_pfwFrames[framepos].MenuHandles.MIVisible = menuid;

	popuppos += 100000;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Show title");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SHFRAMETITLEBAR;
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) _hmiTBVisible = menuid;
	else g_pfwFrames[framepos].MenuHandles.MITBVisible = menuid;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Locked");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_ULFRAME;
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) _hmiLock = menuid;
	else g_pfwFrames[framepos].MenuHandles.MILock = menuid;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Expanded");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_UCOLLFRAME;
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) _hmiColl = menuid;
	else g_pfwFrames[framepos].MenuHandles.MIColl = menuid;

	//floating
	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Floating mode");
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszService = "Set_Floating";
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) _hmiFloating = menuid;
	else g_pfwFrames[framepos].MenuHandles.MIFloating = menuid;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Border");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SETUNBORDER;
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) _hmiBorder = menuid;
	else g_pfwFrames[framepos].MenuHandles.MIBorder = menuid;

	popuppos += 100000;

	{
		//alignment root
		mi.pszPopupName = (char *)root;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Align");
		mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTPOPUP;
		mi.pszService = "";
		mi.pszContactOwner = (char *)0;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiAlignRoot = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIAlignRoot = menuid;

		mi.flags = CMIF_CHILDPOPUP;
		//align top
		mi.pszPopupName = (char *)menuid;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Top");
		mi.pszService = CLUIFRAMESSETALIGNALTOP;
		mi.pszContactOwner = (char *)alTop;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiAlignTop = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIAlignTop = menuid;


		//align client
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Client");
		mi.pszService = CLUIFRAMESSETALIGNALCLIENT;
		mi.pszContactOwner = (char *)alClient;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiAlignClient = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIAlignClient = menuid;

		//align bottom
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Bottom");
		mi.pszService = CLUIFRAMESSETALIGNALBOTTOM;
		mi.pszContactOwner = (char *)alBottom;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiAlignBottom = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIAlignBottom = menuid;

	}

	{	//position
		//position root
		mi.pszPopupName = (char *)root;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Position");
		mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTPOPUP;
		mi.pszService = "";
		mi.pszContactOwner = (char *)0;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiPosRoot = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIPosRoot = menuid;
		//??????
		mi.pszPopupName = (char *)menuid;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Up");
		mi.flags = CMIF_CHILDPOPUP;
		mi.pszService = CLUIFRAMESMOVEUP;
		mi.pszContactOwner = (char *)1;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiPosUp = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIPosUp = menuid;

		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Down");
		mi.flags = CMIF_CHILDPOPUP;
		mi.pszService = CLUIFRAMESMOVEDOWN;
		mi.pszContactOwner = (char *)-1;
		menuid = pfnAdd(&mi);
		if (frameid == -1) _hmiPosDown = menuid;
		else g_pfwFrames[framepos].MenuHandles.MIPosDown = menuid;

	}

	return 0;
}

static int ModifyMItem(WPARAM wParam, LPARAM lParam)
{
	if ((int)wParam == -1) return 0;	   // FIXME

	return CallService(MS_CLIST_MODIFYMENUITEM, wParam, lParam);
};


static int CLUIFramesModifyContextMenuForFrame(WPARAM wParam, LPARAM)
{
	/* HOOK */
	if (MirandaExiting()) return 0;
	if (_fCluiFramesModuleNotStarted) return -1;
	int pos = id2pos(wParam);
	if (pos >= 0 && pos < g_nFramesCount) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_CHILDPOPUP | CMIF_TCHAR;
		if (g_pfwFrames[pos].visible) mi.flags |= CMIF_CHECKED;
		mi.ptszName = g_pfwFrames[pos].TitleBar.tbname ? g_pfwFrames[pos].TitleBar.tbname : g_pfwFrames[pos].name;
		ModifyMItem((WPARAM)_hmiVisible, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].Locked) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiLock, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].TitleBar.ShowTitleBar) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiTBVisible, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].floating) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiFloating, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_CluiData.fLayered) mi.flags |= CMIF_GRAYED;
		else if ((g_pfwFrames[pos].UseBorder)) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiBorder, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].align&alTop) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiAlignTop, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].align&alClient) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiAlignClient, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].align&alBottom) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)_hmiAlignBottom, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].collapsed) mi.flags |= CMIF_CHECKED;
		if ((!g_pfwFrames[pos].visible) || (g_pfwFrames[pos].Locked) || (pos == CLUIFramesGetalClientFrame())) mi.flags |= CMIF_GRAYED;
		ModifyMItem((WPARAM)_hmiColl, (LPARAM)&mi);
	}
	return 0;
}

static int CLUIFramesModifyMainMenuItems(WPARAM wParam, LPARAM)
{
	//hiword(wParam) = frameid,loword(wParam) = flag
	if (_fCluiFramesModuleNotStarted) return -1;

	int pos = id2pos(wParam);

	if (pos >= 0 && pos < g_nFramesCount) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_CHILDPOPUP | CMIF_TCHAR;
		if (g_pfwFrames[pos].visible) mi.flags |= CMIF_CHECKED;
		mi.ptszName = g_pfwFrames[pos].TitleBar.tbname ? g_pfwFrames[pos].TitleBar.tbname : g_pfwFrames[pos].name;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIVisible, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].Locked) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MILock, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].TitleBar.ShowTitleBar) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MITBVisible, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].floating) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIFloating, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;

		if (g_CluiData.fLayered) mi.flags |= CMIF_GRAYED;
		else if ((g_pfwFrames[pos].UseBorder)) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIBorder, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP | ((g_pfwFrames[pos].align&alClient) ? CMIF_GRAYED : 0);
		if (g_pfwFrames[pos].align&alTop) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIAlignTop, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].align&alClient) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIAlignClient, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP | ((g_pfwFrames[pos].align&alClient) ? CMIF_GRAYED : 0);
		if (g_pfwFrames[pos].align&alBottom) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIAlignBottom, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (g_pfwFrames[pos].collapsed) mi.flags |= CMIF_CHECKED;
		if ((!g_pfwFrames[pos].visible) || g_pfwFrames[pos].Locked || (pos == CLUIFramesGetalClientFrame())) mi.flags |= CMIF_GRAYED;
		Menu_ModifyItem(g_pfwFrames[pos].MenuHandles.MIColl, &mi);
	}

	return 0;
}


static INT_PTR _us_DoGetFrameOptions(WPARAM wParam, LPARAM)
{
	INT_PTR retval;
	BOOL bUnicodeText = (LOWORD(wParam) & FO_UNICODETEXT) != 0;
	wParam = MAKEWPARAM((LOWORD(wParam)) & ~FO_UNICODETEXT, HIWORD(wParam));

	if (_fCluiFramesModuleNotStarted) return -1;

	int pos = id2pos(HIWORD(wParam));
	if (pos < 0 || pos >= g_nFramesCount)
		return -1;

	switch (LOWORD(wParam)) {
	case FO_FLAGS:
		retval = 0;
		if (g_pfwFrames[pos].visible) retval |= F_VISIBLE;
		if (!g_pfwFrames[pos].collapsed) retval |= F_UNCOLLAPSED;
		if (g_pfwFrames[pos].Locked) retval |= F_LOCKED;
		if (g_pfwFrames[pos].TitleBar.ShowTitleBar) retval |= F_SHOWTB;
		if (g_pfwFrames[pos].TitleBar.ShowTitleBarTip) retval |= F_SHOWTBTIP;
		if (!g_CluiData.fLayered) {
			if (!(GetWindowLongPtr(g_pfwFrames[pos].hWnd, GWL_STYLE)&WS_BORDER))
				retval |= F_NOBORDER;
		}
		else if (!g_pfwFrames[pos].UseBorder)
			retval |= F_NOBORDER;
		break;

	case FO_NAME:
		if (bUnicodeText)
			retval = (INT_PTR)g_pfwFrames[pos].name;
		else
			retval = (INT_PTR)g_pfwFrames[pos].szName;
		break;

	case FO_TBNAME:
		if (bUnicodeText)
			retval = (INT_PTR)g_pfwFrames[pos].TitleBar.tbname;
		else
			retval = (INT_PTR)g_pfwFrames[pos].TitleBar.sztbname;
		break;

	case FO_TBTIPNAME:
		if (bUnicodeText)
			retval = (INT_PTR)g_pfwFrames[pos].TitleBar.tooltip;
		else
			retval = (INT_PTR)g_pfwFrames[pos].TitleBar.sztooltip;
		break;

	case FO_TBSTYLE:
		retval = GetWindowLongPtr(g_pfwFrames[pos].TitleBar.hwnd, GWL_STYLE);
		break;

	case FO_TBEXSTYLE:
		retval = GetWindowLongPtr(g_pfwFrames[pos].TitleBar.hwnd, GWL_EXSTYLE);
		break;

	case FO_ICON:
		retval = (INT_PTR)g_pfwFrames[pos].TitleBar.hicon;
		break;

	case FO_HEIGHT:
		retval = (INT_PTR)g_pfwFrames[pos].height;
		break;

	case FO_ALIGN:
		retval = (INT_PTR)g_pfwFrames[pos].align;
		break;
	case FO_FLOATING:
		retval = (INT_PTR)g_pfwFrames[pos].floating;
		break;
	default:
		retval = -1;
		break;
	}

	return retval;
}

static int UpdateTBToolTip(int framepos)
{
	TOOLINFO ti = { sizeof(ti) };
	ti.lpszText = g_pfwFrames[framepos].TitleBar.tooltip;
	ti.hinst = g_hInst;
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

		fw.collapsed = TRUE;
		if (lParam & F_UNCOLLAPSED) fw.collapsed = FALSE;

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

		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
		SetWindowPos(fw.TitleBar.hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE);
		return 0;

	case FO_NAME:
		if (lParam == 0)
			return -1;

		mir_free(fw.name);
		mir_free(fw.szName);
		if (bUnicodeText) {
			fw.name = mir_tstrdup((LPTSTR)lParam);
			fw.szName = mir_t2a((LPTSTR)lParam);
		}
		else {
			fw.szName = mir_strdup((char*)lParam);
			fw.name = mir_a2t((char *)lParam);
		}
		return 0;

	case FO_TBNAME:
		if (lParam == 0) { return(-1); }

		mir_free(fw.TitleBar.tbname);
		mir_free(fw.TitleBar.sztbname);
		if (bUnicodeText) {
			fw.TitleBar.tbname = mir_tstrdup((LPTSTR)lParam);
			fw.TitleBar.sztbname = mir_t2a((LPTSTR)lParam);
		}
		else {
			fw.TitleBar.sztbname = mir_strdup((char*)lParam);
			fw.TitleBar.tbname = mir_a2t((char*)lParam);
		}

		if (fw.floating && (fw.TitleBar.tbname != NULL))
			SetWindowText(fw.ContainerWnd, fw.TitleBar.tbname);
		return 0;

	case FO_TBTIPNAME:
		if (lParam == 0) { return(-1); }
		if (fw.TitleBar.tooltip != NULL) mir_free_and_nil(fw.TitleBar.tooltip);
		if (fw.TitleBar.sztooltip != NULL) mir_free_and_nil(fw.TitleBar.sztooltip);
		if (bUnicodeText) {
			fw.TitleBar.tooltip = mir_tstrdup((LPTSTR)lParam);
			fw.TitleBar.sztooltip = mir_t2a((LPTSTR)lParam);
		}
		else {
			fw.TitleBar.sztooltip = mir_strdup((char*)lParam);
			fw.TitleBar.tooltip = mir_a2t((char*)lParam);
		}
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
				CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	return -1;
}

//wparam = lparam = 0
static int _us_DoShowAllFrames(WPARAM, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		g_pfwFrames[i].visible = TRUE;
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	return 0;
}

//wparam = lparam = 0
static int _us_DoShowTitles(WPARAM, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		g_pfwFrames[i].TitleBar.ShowTitleBar = TRUE;
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	return 0;
}

//wparam = lparam = 0
static int _us_DoHideTitles(WPARAM, LPARAM)
{
	if (_fCluiFramesModuleNotStarted) return -1;

	for (int i = 0; i < g_nFramesCount; i++)
		g_pfwFrames[i].TitleBar.ShowTitleBar = FALSE;
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	return 0;
}

//wparam = frameid
static int _us_DoShowHideFrame(WPARAM wParam, LPARAM lParam)
{
	if (_fCluiFramesModuleNotStarted)
		return -1;

	int pos = (wParam == 0) ? lParam : id2pos(wParam);
	if (pos >= 0 && (int)pos < g_nFramesCount) {
		g_pfwFrames[pos].visible = !g_pfwFrames[pos].visible;
		if (g_pfwFrames[pos].OwnerWindow != (HWND)-2) {
			if (g_pfwFrames[pos].OwnerWindow)
				CLUI_ShowWindowMod(g_pfwFrames[pos].OwnerWindow, (g_pfwFrames[pos].visible &&  g_pfwFrames[pos].collapsed && IsWindowVisible(pcli->hwndContactList)) ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
			else if (g_pfwFrames[pos].visible) {
				g_pfwFrames[pos].OwnerWindow = CreateSubContainerWindow(pcli->hwndContactList, g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y, 10, 10);
				SetParent(g_pfwFrames[pos].hWnd, g_pfwFrames[pos].OwnerWindow);
				CLUI_ShowWindowMod(g_pfwFrames[pos].OwnerWindow, (g_pfwFrames[pos].visible && g_pfwFrames[pos].collapsed && IsWindowVisible(pcli->hwndContactList)) ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
			}
		}

		if (g_pfwFrames[pos].floating)
			CLUIFrameResizeFloatingFrame(pos);

		if (!g_pfwFrames[pos].floating)
			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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
			if (g_pfwFrames[i].floating || (!g_pfwFrames[i].visible) || (g_pfwFrames[i].align != curalign)) { continue; };

			sd[v].order = g_pfwFrames[i].order;
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
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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
	SetWindowPos(hw, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME);
	return 0;
}

//wparam = frameid
static int _us_DoCollapseFrame(WPARAM wParam, LPARAM lParam)
{
	int FrameId;

	if (_fCluiFramesModuleNotStarted) return -1;

	if (wParam == 0) {
		FrameId = lParam;
	}
	else {
		FrameId = id2pos(wParam);
	}
	if (FrameId >= 0 && FrameId < g_nFramesCount) {
		int oldHeight;

		// do not collapse/uncollapse client/locked/invisible frames
		if (g_pfwFrames[FrameId].align == alClient && !(g_pfwFrames[FrameId].Locked || (!g_pfwFrames[FrameId].visible) || g_pfwFrames[FrameId].floating)) {
			RECT rc;
			if (CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0)) { return 0; };
			if (!g_CluiData.fDocked && g_CluiData.fAutoSize) { return 0; };
			GetWindowRect(pcli->hwndContactList, &rc);

			if (g_pfwFrames[FrameId].collapsed == TRUE) {
				rc.bottom -= rc.top;
				rc.bottom -= g_pfwFrames[FrameId].height;
				g_pfwFrames[FrameId].HeightWhenCollapsed = g_pfwFrames[FrameId].height;
				g_pfwFrames[FrameId].collapsed = FALSE;
			}
			else {
				rc.bottom -= rc.top;
				rc.bottom += g_pfwFrames[FrameId].HeightWhenCollapsed;
				g_pfwFrames[FrameId].collapsed = TRUE;
			}

			SetWindowPos(pcli->hwndContactList, NULL, 0, 0, rc.right - rc.left, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

			CLUIFramesStoreAllFrames();


			return 0;

		}
		if (g_pfwFrames[FrameId].Locked || (!g_pfwFrames[FrameId].visible)) return 0;

		oldHeight = g_pfwFrames[FrameId].height;

		// if collapsed, uncollapse
		if (g_pfwFrames[FrameId].collapsed == TRUE) {
			g_pfwFrames[FrameId].HeightWhenCollapsed = g_pfwFrames[FrameId].height;
			g_pfwFrames[FrameId].height = UNCOLLAPSED_FRAME_SIZE;
			g_pfwFrames[FrameId].collapsed = FALSE;
		}
		// if uncollapsed, collapse
		else {
			g_pfwFrames[FrameId].height = g_pfwFrames[FrameId].HeightWhenCollapsed;
			g_pfwFrames[FrameId].collapsed = TRUE;
		}

		if (!g_pfwFrames[FrameId].floating) {

			if (!CLUIFramesFitInSize()) {
				//cant collapse,we can resize only for height < alclient frame height
				int alfrm = CLUIFramesGetalClientFrame();

				if (alfrm != -1) {
					g_pfwFrames[FrameId].collapsed = FALSE;
					if (g_pfwFrames[alfrm].height > 2 * UNCOLLAPSED_FRAME_SIZE) {
						oldHeight = g_pfwFrames[alfrm].height - UNCOLLAPSED_FRAME_SIZE;
						g_pfwFrames[FrameId].collapsed = TRUE;
					}
				}
				else {
					int i, sumheight = 0;
					for (i = 0; i < g_nFramesCount; i++) {
						if ((g_pfwFrames[i].align != alClient) && (!g_pfwFrames[i].floating) && (g_pfwFrames[i].visible) && (!g_pfwFrames[i].needhide)) {
							sumheight += (g_pfwFrames[i].height) + (g_nTitleBarHeight*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar)) + 2;
							return FALSE;
						}
						if (sumheight>_nContactListHeight - 0 - 2) {
							g_pfwFrames[FrameId].height = (_nContactListHeight - 0 - 2) - sumheight;
						}

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

		//CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
		if (!g_pfwFrames[FrameId].floating) {
			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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

	if (_hmiRoot != HGENMENU_ROOT) {
		CallService(MO_REMOVEMENUITEM, (WPARAM)_hmiRoot, 0);
		_hmiRoot = HGENMENU_ROOT;
	}

	// create root menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTHANDLE;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_FRAME);
	mi.position = 3000090000;
	mi.pszName = LPGEN("Frames");
	_hmiRoot = Menu_AddMainMenuItem(&mi);

	// create frames menu
	int separator = 3000200000;
	for (int i = 0; i < g_nFramesCount; i++) {
		mi.hIcon = g_pfwFrames[i].TitleBar.hicon;
		mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
		mi.position = separator;
		mi.hParentMenu = _hmiRoot;
		mi.ptszName = g_pfwFrames[i].TitleBar.tbname ? g_pfwFrames[i].TitleBar.tbname : g_pfwFrames[i].name;
		mi.pszService = 0;
		g_pfwFrames[i].MenuHandles.MainMenuItem = Menu_AddMainMenuItem(&mi);
		CLUIFramesCreateMenuForFrame(g_pfwFrames[i].id, (int)g_pfwFrames[i].MenuHandles.MainMenuItem, separator, Menu_AddMainMenuItem);
		CLUIFramesModifyMainMenuItems(g_pfwFrames[i].id, 0);
		CallService(MS_CLIST_FRAMEMENUNOTIFY, (WPARAM)g_pfwFrames[i].id, (LPARAM)g_pfwFrames[i].MenuHandles.MainMenuItem);
		separator++;
	}

	separator += 100000;

	// create "show all frames" menu
	mi.hIcon = NULL;
	mi.flags = CMIF_ROOTHANDLE;
	mi.position = separator++;
	mi.hParentMenu = _hmiRoot;
	mi.pszName = LPGEN("Show all frames");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMES;
	Menu_AddMainMenuItem(&mi);

	// create "show all titlebars" menu
	mi.position = separator++;
	mi.pszName = LPGEN("Show all title bars");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);

	// create "hide all titlebars" menu
	mi.position = separator++;
	mi.pszName = LPGEN("Hide all title bars");
	mi.pszService = MS_CLIST_FRAMES_HIDEALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);
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
static int _us_DoAddFrame(WPARAM wParam, LPARAM)
{
	int retval;
	LONG_PTR style;
	//char * CustomName = NULL;
	CLISTFrame *clfrm = (CLISTFrame *)wParam;

	if (pcli->hwndContactList == 0) return -1;
	if (_fCluiFramesModuleNotStarted) return -1;
	if (clfrm->cbSize != sizeof(CLISTFrame)) return -1;
	if (!(_hTitleBarFont)) _hTitleBarFont = CLUILoadTitleBarFont();

	g_pfwFrames = (FRAMEWND*)realloc(g_pfwFrames, sizeof(FRAMEWND)*(g_nFramesCount + 1));

	memset(&g_pfwFrames[g_nFramesCount], 0, sizeof(FRAMEWND));
	g_pfwFrames[g_nFramesCount].id = _iNextFrameId++;
	g_pfwFrames[g_nFramesCount].align = clfrm->align;
	g_pfwFrames[g_nFramesCount].hWnd = clfrm->hWnd;
	g_pfwFrames[g_nFramesCount].height = clfrm->height;
	g_pfwFrames[g_nFramesCount].TitleBar.hicon = clfrm->hIcon;
	//Frames[nFramescount].TitleBar.BackColour;
	g_pfwFrames[g_nFramesCount].floating = FALSE;
	if (clfrm->Flags&F_NO_SUBCONTAINER || !g_CluiData.fLayered)
		g_pfwFrames[g_nFramesCount].OwnerWindow = (HWND)-2;
	else g_pfwFrames[g_nFramesCount].OwnerWindow = 0;

	//override tbbtip
	//clfrm->Flags |= F_SHOWTBTIP;
	//
	if (db_get_b(0, CLUIFrameModule, "RemoveAllBorders", 0) == 1) {
		clfrm->Flags |= F_NOBORDER;
	};
	g_pfwFrames[g_nFramesCount].dwFlags = clfrm->Flags;

	if (clfrm->name == NULL || ((clfrm->Flags&F_UNICODE) ? mir_wstrlen(clfrm->wname) : mir_strlen(clfrm->name)) == 0) {
		g_pfwFrames[g_nFramesCount].name = (LPTSTR)mir_alloc(255 * sizeof(TCHAR));
		GetClassName(g_pfwFrames[g_nFramesCount].hWnd, g_pfwFrames[g_nFramesCount].name, 255);
	}
	else {
		g_pfwFrames[g_nFramesCount].name = (clfrm->Flags&F_UNICODE) ? mir_u2t(clfrm->wname) : mir_a2t(clfrm->name);
	}
	g_pfwFrames[g_nFramesCount].szName = mir_t2a(g_pfwFrames[g_nFramesCount].name);
	if (IsBadCodePtr((FARPROC)clfrm->TBname) || clfrm->TBname == NULL
		|| ((clfrm->Flags&F_UNICODE) ? mir_wstrlen(clfrm->TBwname) : mir_strlen(clfrm->TBname)) == 0)
		g_pfwFrames[g_nFramesCount].TitleBar.tbname = mir_tstrdup(g_pfwFrames[g_nFramesCount].name);
	else
		g_pfwFrames[g_nFramesCount].TitleBar.tbname = (clfrm->Flags&F_UNICODE) ? mir_u2t(clfrm->TBwname) : mir_a2t(clfrm->TBname);
	g_pfwFrames[g_nFramesCount].TitleBar.sztbname = mir_t2a(g_pfwFrames[g_nFramesCount].TitleBar.tbname);
	g_pfwFrames[g_nFramesCount].needhide = FALSE;
	g_pfwFrames[g_nFramesCount].TitleBar.ShowTitleBar = (clfrm->Flags&F_SHOWTB ? TRUE : FALSE);
	g_pfwFrames[g_nFramesCount].TitleBar.ShowTitleBarTip = (clfrm->Flags&F_SHOWTBTIP ? TRUE : FALSE);

	g_pfwFrames[g_nFramesCount].collapsed = (clfrm->Flags&F_UNCOLLAPSED) ? FALSE : TRUE;


	g_pfwFrames[g_nFramesCount].Locked = clfrm->Flags&F_LOCKED ? TRUE : FALSE;
	g_pfwFrames[g_nFramesCount].visible = clfrm->Flags&F_VISIBLE ? TRUE : FALSE;

	g_pfwFrames[g_nFramesCount].UseBorder = ((clfrm->Flags&F_NOBORDER) || g_CluiData.fLayered) ? FALSE : TRUE;

	//Frames[nFramescount].OwnerWindow = 0;

	g_pfwFrames[g_nFramesCount].TitleBar.hwnd
		= CreateWindow(CLUIFrameTitleBarClassName, g_pfwFrames[g_nFramesCount].name,
		(db_get_b(0, CLUIFrameModule, "RemoveAllTitleBarBorders", 1) ? 0 : WS_BORDER)
		| WS_CHILD | WS_CLIPCHILDREN |
		(g_pfwFrames[g_nFramesCount].TitleBar.ShowTitleBar ? WS_VISIBLE : 0) |
		WS_CLIPCHILDREN,
		0, 0, 0, 0, pcli->hwndContactList, NULL, g_hInst, NULL);
	SetWindowLongPtr(g_pfwFrames[g_nFramesCount].TitleBar.hwnd, GWLP_USERDATA, g_pfwFrames[g_nFramesCount].id);

	g_pfwFrames[g_nFramesCount].TitleBar.hwndTip
		= CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		pcli->hwndContactList, NULL, g_hInst,
		NULL);

	SetWindowPos(g_pfwFrames[g_nFramesCount].TitleBar.hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	{
		TOOLINFO ti = { sizeof(ti) };
		ti.lpszText = _T("");
		ti.hinst = g_hInst;
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.uId = (UINT_PTR)g_pfwFrames[g_nFramesCount].TitleBar.hwnd;
		SendMessage(g_pfwFrames[g_nFramesCount].TitleBar.hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

	SendMessageA(g_pfwFrames[g_nFramesCount].TitleBar.hwndTip, TTM_ACTIVATE, (WPARAM)g_pfwFrames[g_nFramesCount].TitleBar.ShowTitleBarTip, 0);

	g_pfwFrames[g_nFramesCount].oldstyles = GetWindowLongPtr(g_pfwFrames[g_nFramesCount].hWnd, GWL_STYLE);
	g_pfwFrames[g_nFramesCount].TitleBar.oldstyles = GetWindowLongPtr(g_pfwFrames[g_nFramesCount].TitleBar.hwnd, GWL_STYLE);
	//Frames[nFramescount].FloatingPos.x =

	retval = g_pfwFrames[g_nFramesCount].id;
	g_pfwFrames[g_nFramesCount].order = g_nFramesCount + 1;
	g_nFramesCount++;


	CLUIFramesLoadFrameSettings(id2pos(retval));
	if (g_pfwFrames[g_nFramesCount - 1].collapsed == FALSE)
		g_pfwFrames[g_nFramesCount - 1].height = 0;

	// create frame

	style = GetWindowLongPtr(g_pfwFrames[g_nFramesCount - 1].hWnd, GWL_STYLE);
	style &= (~WS_BORDER);
	style |= (((g_pfwFrames[g_nFramesCount - 1].UseBorder) && !g_CluiData.fLayered) ? WS_BORDER : 0);
	SetWindowLongPtr(g_pfwFrames[g_nFramesCount - 1].hWnd, GWL_STYLE, style);
	SetWindowLongPtr(g_pfwFrames[g_nFramesCount - 1].TitleBar.hwnd, GWL_STYLE, style& ~(WS_VSCROLL | WS_HSCROLL));
	SetWindowLongPtr(g_pfwFrames[g_nFramesCount - 1].TitleBar.hwnd, GWL_STYLE, GetWindowLongPtr(g_pfwFrames[g_nFramesCount - 1].TitleBar.hwnd, GWL_STYLE)&~(WS_VSCROLL | WS_HSCROLL));

	if (g_pfwFrames[g_nFramesCount - 1].order == 0) { g_pfwFrames[g_nFramesCount - 1].order = g_nFramesCount; };

	//need to enlarge parent
	{
		RECT mainRect;
		int mainHeight, minHeight;
		GetWindowRect(pcli->hwndContactList, &mainRect);
		mainHeight = mainRect.bottom - mainRect.top;
		minHeight = CLUIFrames_GetTotalHeight();
		if (mainHeight < minHeight) {
			BOOL Upward = FALSE;
			Upward = !g_CluiData.fDocked && g_CluiData.fAutoSize && db_get_b(NULL, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT);

			if (Upward)
				mainRect.top = mainRect.bottom - minHeight;
			else
				mainRect.bottom = mainRect.top + minHeight;
			SetWindowPos(pcli->hwndContactList, NULL, mainRect.left, mainRect.top, mainRect.right - mainRect.left, mainRect.bottom - mainRect.top, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		}
		GetWindowRect(pcli->hwndContactList, &mainRect);
		mainHeight = mainRect.bottom - mainRect.top;
	}
	_nClientFrameId = eUnknownId;//recalc it
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);

	if (g_pfwFrames[g_nFramesCount - 1].floating) {

		g_pfwFrames[g_nFramesCount - 1].floating = FALSE;
		//SetWindowPos(Frames[nFramescount-1].hw
		CLUIFrames_SetFrameFloat(retval, 1);//lparam = 1 use stored width and height
	}
	else
		CLUIFrames_SetFrameFloat(retval, 2);

	/** Remove	*/
	//_ExtFramesSrv_AddFrame( wParam,  lParam);

	return retval;
}

static int _us_DoRemoveFrame(WPARAM wParam, LPARAM)
{
	int pos;
	if (_fCluiFramesModuleNotStarted) return -1;

	pos = id2pos(wParam);

	if (pos < 0 || pos>g_nFramesCount) { return(-1); };

	mir_free_and_nil(g_pfwFrames[pos].name);
	mir_free_and_nil(g_pfwFrames[pos].szName);
	mir_free_and_nil(g_pfwFrames[pos].TitleBar.tbname);
	mir_free_and_nil(g_pfwFrames[pos].TitleBar.sztbname);
	mir_free_and_nil(g_pfwFrames[pos].TitleBar.tooltip);
	mir_free_and_nil(g_pfwFrames[pos].TitleBar.sztooltip);
	DestroyWindow(g_pfwFrames[pos].hWnd);
	g_pfwFrames[pos].hWnd = (HWND)-1;
	DestroyWindow(g_pfwFrames[pos].TitleBar.hwnd);
	if (g_pfwFrames[pos].TitleBar.hwndTip)
		DestroyWindow(g_pfwFrames[pos].TitleBar.hwndTip);
	g_pfwFrames[pos].TitleBar.hwnd = (HWND)-1;
	if (g_pfwFrames[pos].ContainerWnd && g_pfwFrames[pos].ContainerWnd != (HWND)-1) DestroyWindow(g_pfwFrames[pos].ContainerWnd);
	g_pfwFrames[pos].ContainerWnd = (HWND)-1;
	if (g_pfwFrames[pos].TitleBar.hmenu) DestroyMenu(g_pfwFrames[pos].TitleBar.hmenu);
	g_pfwFrames[pos].PaintCallbackProc = NULL;
	if (g_pfwFrames[pos].UpdateRgn) DeleteObject(g_pfwFrames[pos].UpdateRgn);

	if (g_pfwFrames[pos].OwnerWindow != (HWND)-1
		&& g_pfwFrames[pos].OwnerWindow != (HWND)-2
		&& g_pfwFrames[pos].OwnerWindow != (HWND)0)
		DestroyWindow(g_pfwFrames[pos].OwnerWindow);

	g_pfwFrames[pos].OwnerWindow = NULL;
	RemoveItemFromList(pos, &g_pfwFrames, &g_nFramesCount);

	CLUI__cliInvalidateRect(pcli->hwndContactList, NULL, TRUE);
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	CLUI__cliInvalidateRect(pcli->hwndContactList, NULL, TRUE);

	return 0;
};


static int CLUIFramesForceUpdateTB(const FRAMEWND *Frame)
{
	if (Frame->TitleBar.hwnd != 0) RedrawWindow(Frame->TitleBar.hwnd, NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	//UpdateWindow(Frame->TitleBar.hwnd);
	return 0;
}

static int CLUIFramesForceUpdateFrame(const FRAMEWND *Frame)
{
	if (Frame->hWnd != 0) {
		RedrawWindow(Frame->hWnd, NULL, NULL, RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_INVALIDATE);
		UpdateWindow(Frame->hWnd);
	};
	if (Frame->floating) {
		if (Frame->ContainerWnd != 0)  RedrawWindow(Frame->ContainerWnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
		//UpdateWindow(Frame->ContainerWnd);
	};
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

		ClientToScreen(pcli->hwndContactList, &Off);
		GetWindowRect(pcli->hwndContactList, &pr);

		if (Frame->visible && (!Frame->collapsed || Frame->wndSize.bottom - Frame->wndSize.top == 0)) {
			ShowWindowAsync(Frame->OwnerWindow, SW_HIDE);
			ShowWindowAsync(Frame->hWnd, SW_HIDE);
		}

		{
			SetWindowPos(Frame->OwnerWindow, NULL, Frame->wndSize.left + Off.x, Frame->wndSize.top + Off.y,
				Frame->wndSize.right - Frame->wndSize.left,
				Frame->wndSize.bottom - Frame->wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);   //- -= -=

			SetWindowPos(Frame->hWnd, NULL, 0, 0,
				Frame->wndSize.right - Frame->wndSize.left,
				Frame->wndSize.bottom - Frame->wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		// set titlebar position
		if (Frame->TitleBar.ShowTitleBar) {
			SetWindowPos(Frame->TitleBar.hwnd, NULL, Frame->wndSize.left, Frame->wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar,
				Frame->wndSize.right - Frame->wndSize.left,
				g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		if (Frame->visible && IsWindowVisible(pcli->hwndContactList) && Frame->collapsed && Frame->wndSize.bottom - Frame->wndSize.top != 0) {
			ShowWindow(Frame->OwnerWindow, SW_SHOW);
			ShowWindow(Frame->hWnd, SW_SHOW);
		}

	}
	else {
		// set frame position
		SetWindowPos(Frame->hWnd, NULL, Frame->wndSize.left, Frame->wndSize.top,
			Frame->wndSize.right - Frame->wndSize.left,
			Frame->wndSize.bottom - Frame->wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
		// set titlebar position
		if (Frame->TitleBar.ShowTitleBar) {
			SetWindowPos(Frame->TitleBar.hwnd, NULL, Frame->wndSize.left, Frame->wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar,
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
	int clientfrm;
	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = g_nTitleBarHeight*btoint(g_pfwFrames[clientfrm].TitleBar.ShowTitleBar);

	for (int i = 0; i < g_nFramesCount; i++) {
		if ((g_pfwFrames[i].align != alClient) && (!g_pfwFrames[i].floating) && (g_pfwFrames[i].visible) && (!g_pfwFrames[i].needhide)) {
			sumheight += (g_pfwFrames[i].height) + (g_nTitleBarHeight*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar)) + 2/*+btoint(Frames[i].UseBorder)*2*/;
			if (sumheight>_nContactListHeight - tbh - 2) {
				if (!g_CluiData.fDocked && g_CluiData.fAutoSize) {
					return TRUE; //Can be required to enlarge
				}
				return FALSE;
			}
		}
	}
	return TRUE;
}
int CLUIFrames_GetTotalHeight()
{
	if (pcli->hwndContactList == NULL) return 0;

	int sumheight = 0;
	RECT border;
	for (int i = 0; i < g_nFramesCount; i++) {
		if ((g_pfwFrames[i].visible) && (!g_pfwFrames[i].needhide) && (!g_pfwFrames[i].floating) && (pcli->hwndContactTree) && (g_pfwFrames[i].hWnd != pcli->hwndContactTree))
			sumheight += (g_pfwFrames[i].height) + (g_nTitleBarHeight*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar));
	};

	GetBorderSize(pcli->hwndContactList, &border);

	//GetWindowRect(pcli->hwndContactList,&winrect);
	//GetClientRect(pcli->hwndContactList,&clirect);
	//	clirect.bottom -= clirect.top;
	//	clirect.bottom += border.top+border.bottom;
	//allbord = (winrect.bottom-winrect.top)-(clirect.bottom-clirect.top);

	//TODO minsize
	sumheight += g_CluiData.TopClientMargin;
	sumheight += g_CluiData.BottomClientMargin;
	return  max(db_get_w(NULL, "CLUI", "MinHeight", SETTING_MINHEIGTH_DEFAULT),
		(sumheight + border.top + border.bottom));
}

int CLUIFramesGetMinHeight()
{
	int tbh = 0, sumheight = 0;
	RECT border;
	int allbord = 0;
	if (pcli->hwndContactList == NULL) return 0;


	// search for alClient frame and get the titlebar's height
	int clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = g_nTitleBarHeight*btoint(g_pfwFrames[clientfrm].TitleBar.ShowTitleBar);

	for (int i = 0; i < g_nFramesCount; i++) {
		if ((g_pfwFrames[i].align != alClient) && (g_pfwFrames[i].visible) && (!g_pfwFrames[i].needhide) && (!g_pfwFrames[i].floating)) {
			RECT wsize;

			GetWindowRect(g_pfwFrames[i].hWnd, &wsize);
			sumheight += (wsize.bottom - wsize.top) + (g_nTitleBarHeight*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar));
		}
	}

	GetBorderSize(pcli->hwndContactList, &border);

	//GetWindowRect(pcli->hwndContactList,&winrect);
	//GetClientRect(pcli->hwndContactList,&clirect);
	//	clirect.bottom -= clirect.top;
	//	clirect.bottom += border.top+border.bottom;
	//allbord = (winrect.bottom-winrect.top)-(clirect.bottom-clirect.top);

	//TODO minsize
	sumheight += g_CluiData.TopClientMargin;
	sumheight += g_CluiData.BottomClientMargin;
	return  max(db_get_w(NULL, "CLUI", "MinHeight", SETTING_MINHEIGTH_DEFAULT),
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


	g_CluiData.nGapBetweenTitlebar = (int)db_get_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	GapBetweenFrames = db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);

	if (g_nFramesCount < 1) return 0;
	newheight = newsize.bottom - newsize.top;

	// search for alClient frame and get the titlebar's height
	tbh = 0;
	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(g_pfwFrames[clientfrm].TitleBar.ShowTitleBar);

	for (i = 0; i < g_nFramesCount; i++) {
		if (!g_pfwFrames[i].floating) {
			g_pfwFrames[i].needhide = FALSE;
			g_pfwFrames[i].wndSize.left = newsize.left;
			g_pfwFrames[i].wndSize.right = newsize.right;

		};
	};
	{
		//sorting stuff
		sdarray = (SortData*)malloc(sizeof(SortData)*g_nFramesCount);
		if (sdarray == NULL) { return(-1); };
		for (i = 0; i < g_nFramesCount; i++) {
			sdarray[i].order = g_pfwFrames[i].order;
			sdarray[i].realpos = i;
		}
		qsort(sdarray, g_nFramesCount, sizeof(SortData), sortfunc);

	}

	drawitems = g_nFramesCount;

	while (sumheight>(newheight - tbh) && drawitems > 0) {
		sumheight = 0;
		drawitems = 0;
		for (i = 0; i < g_nFramesCount; i++) {
			if (((g_pfwFrames[i].align != alClient)) && (!g_pfwFrames[i].floating) && (g_pfwFrames[i].visible) && (!g_pfwFrames[i].needhide)) {
				drawitems++;
				curfrmtbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar);
				sumheight += (g_pfwFrames[i].height) + curfrmtbh + (i > 0 ? sepw : 0) + ((g_pfwFrames[i].UseBorder  && !g_CluiData.fLayered) ? 2 : 0);
				if (sumheight > newheight - tbh) {
					sumheight -= (g_pfwFrames[i].height) + curfrmtbh + (i > 0 ? sepw : 0);
					g_pfwFrames[i].needhide = !g_CluiData.fDocked && g_CluiData.fAutoSize ? FALSE : TRUE;
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
		if ((!g_pfwFrames[i].needhide) && (!g_pfwFrames[i].floating) && (g_pfwFrames[i].visible) && (g_pfwFrames[i].align == alTop)) {
			curfrmtbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar);
			g_pfwFrames[i].wndSize.top = prevframebottomline + (i > 0 ? sepw : 0) + (curfrmtbh);
			g_pfwFrames[i].wndSize.bottom = g_pfwFrames[i].height + g_pfwFrames[i].wndSize.top + ((g_pfwFrames[i].UseBorder  && !g_CluiData.fLayered) ? 2 : 0);
			g_pfwFrames[i].prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = g_pfwFrames[i].wndSize.bottom;
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
			if ((!g_pfwFrames[i].needhide) && (!g_pfwFrames[i].floating) && (g_pfwFrames[i].visible) && (g_pfwFrames[i].align == alClient)) {
				int oldh;
				g_pfwFrames[i].wndSize.top = prevframebottomline + (j > 0 ? sepw : 0) + (tbh);
				g_pfwFrames[i].wndSize.bottom = g_pfwFrames[i].wndSize.top + newheight - sumheight - tbh - (j > 0 ? sepw : 0);

				oldh = g_pfwFrames[i].height;
				g_pfwFrames[i].height = g_pfwFrames[i].wndSize.bottom - g_pfwFrames[i].wndSize.top;
				g_pfwFrames[i].prevvisframe = prevframe;
				prevframe = i;
				prevframebottomline = g_pfwFrames[i].wndSize.bottom;
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
		if ((g_pfwFrames[i].visible) && (!g_pfwFrames[i].floating) && (!g_pfwFrames[i].needhide) && (g_pfwFrames[i].align == alBottom)) {
			curfrmtbh = (g_nTitleBarHeight + g_CluiData.nGapBetweenTitlebar)*btoint(g_pfwFrames[i].TitleBar.ShowTitleBar);

			g_pfwFrames[i].wndSize.bottom = prevframebottomline - (j > 0 ? sepw : 0);
			g_pfwFrames[i].wndSize.top = g_pfwFrames[i].wndSize.bottom - g_pfwFrames[i].height - ((g_pfwFrames[i].UseBorder  && !g_CluiData.fLayered) ? 2 : 0);
			g_pfwFrames[i].prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = g_pfwFrames[i].wndSize.top/*-1*/ - curfrmtbh;
			if (prevframebottomline > newheight) {

			}
		}
	}
	for (i = 0; i < g_nFramesCount; i++)
		if (g_pfwFrames[i].TitleBar.ShowTitleBar)
			SetRect(&g_pfwFrames[i].TitleBar.wndSize, g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, g_pfwFrames[i].wndSize.right, g_pfwFrames[i].wndSize.top - g_CluiData.nGapBetweenTitlebar);
	if (sdarray != NULL) { free(sdarray); sdarray = NULL; };



	return 0;
}

static int SizeMoveNewSizes()
{
	int i;
	for (i = 0; i < g_nFramesCount; i++) {

		if (g_pfwFrames[i].floating) {
			CLUIFrameResizeFloatingFrame(i);
		}
		else {
			CLUIFrameMoveResize(&g_pfwFrames[i]);
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
		if ((mode == 1 && g_pfwFrames[i].OwnerWindow != (HWND)-2 && g_pfwFrames[i].OwnerWindow) ||
			(mode == 2 && g_pfwFrames[i].OwnerWindow == (HWND)-2) ||
			(mode == 3))
			if (g_pfwFrames[i].floating) {
				CLUIFrameResizeFloatingFrame(i);
			}
			else {
				CLUIFrameMoveResize(&g_pfwFrames[i]);
			};
	}
	if (IsWindowVisible(pcli->hwndContactList)) {
		ske_DrawNonFramedObjects(1, 0);
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
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
		return 0;
	}
	if (lParam&FU_FMPOS)
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 1);

	int pos = id2pos(wParam);
	if (pos < 0 || pos >= g_nFramesCount)
		return -1;
	if (lParam&FU_TBREDRAW)
		CLUIFramesForceUpdateTB(&g_pfwFrames[pos]);
	if (lParam&FU_FMREDRAW)
		CLUIFramesForceUpdateFrame(&g_pfwFrames[pos]);
	return 0;
}

int CLUIFrames_OnClistResize_mod(WPARAM, LPARAM mode)
{
	int tick;
	GapBetweenFrames = db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);
	g_CluiData.nGapBetweenTitlebar = db_get_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	if (_fCluiFramesModuleNotStarted) return -1;

	RECT nRect;
	GetClientRect(pcli->hwndContactList, &nRect);

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

	g_CluiData.nGapBetweenTitlebar = (int)db_get_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	GapBetweenFrames = db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);
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
		int dx = 0;//rcNewWindowRect.left-rcOldWindowRect.left;
		int dy = 0;//_window_rect.top-rcOldWindowRect.top;
		if (!g_pfwFrames[i].floating) {
			if (g_pfwFrames[i].visible && !g_pfwFrames[i].needhide && !IsWindowVisible(g_pfwFrames[i].hWnd)) {
				ShowWindow(g_pfwFrames[i].hWnd, SW_SHOW);
				if (g_pfwFrames[i].TitleBar.ShowTitleBar) ShowWindow(g_pfwFrames[i].TitleBar.hwnd, SW_SHOW);
			}
			if (g_pfwFrames[i].OwnerWindow && (INT_PTR)(g_pfwFrames[i].OwnerWindow) != -2) {
				if (!(mode & 2)) {
					if (NULL != g_pfwFrames[i].OwnerWindow) { /* Wine fix. */
						*PosBatch = DeferWindowPos(*PosBatch, g_pfwFrames[i].OwnerWindow, NULL, g_pfwFrames[i].wndSize.left + r->left, g_pfwFrames[i].wndSize.top + r->top,
							g_pfwFrames[i].wndSize.right - g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.bottom - g_pfwFrames[i].wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
						SetWindowPos(g_pfwFrames[i].hWnd, NULL, 0, 0,
							g_pfwFrames[i].wndSize.right - g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.bottom - g_pfwFrames[i].wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE/*|SWP_NOSENDCHANGING*/);
					}
				}
				//Frame
				if (g_pfwFrames[i].TitleBar.ShowTitleBar) {
					SetWindowPos(g_pfwFrames[i].TitleBar.hwnd, NULL, g_pfwFrames[i].wndSize.left + dx, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar + dy,
						g_pfwFrames[i].wndSize.right - g_pfwFrames[i].wndSize.left, g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
					SetRect(&g_pfwFrames[i].TitleBar.wndSize, g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, g_pfwFrames[i].wndSize.right, g_pfwFrames[i].wndSize.top - g_CluiData.nGapBetweenTitlebar);
					UpdateWindow(g_pfwFrames[i].TitleBar.hwnd);
				}
			}
			else {
				if (1) {
					int res = 0;
					// set frame position
					res = SetWindowPos(g_pfwFrames[i].hWnd, NULL, g_pfwFrames[i].wndSize.left + dx, g_pfwFrames[i].wndSize.top + dy,
						g_pfwFrames[i].wndSize.right - g_pfwFrames[i].wndSize.left,
						g_pfwFrames[i].wndSize.bottom - g_pfwFrames[i].wndSize.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
				}
				if (1) {
					// set titlebar position
					if (g_pfwFrames[i].TitleBar.ShowTitleBar) {
						SetWindowPos(g_pfwFrames[i].TitleBar.hwnd, NULL, g_pfwFrames[i].wndSize.left + dx, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar + dy,
							g_pfwFrames[i].wndSize.right - g_pfwFrames[i].wndSize.left, g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
						SetRect(&g_pfwFrames[i].TitleBar.wndSize, g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, g_pfwFrames[i].wndSize.right, g_pfwFrames[i].wndSize.top - g_CluiData.nGapBetweenTitlebar);

					}
				}
				UpdateWindow(g_pfwFrames[i].hWnd);
				if (g_pfwFrames[i].TitleBar.ShowTitleBar) UpdateWindow(g_pfwFrames[i].TitleBar.hwnd);
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
	g_CluiData.nGapBetweenTitlebar = (int)db_get_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);
	GapBetweenFrames = db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);

	for (int i = 0; i < g_nFramesCount; i++) {
		int dx = 0;//rcNewWindowRect.left-rcOldWindowRect.left;
		int dy = 0;//_window_rect.top-rcOldWindowRect.top;
		if (!g_pfwFrames[i].floating && g_pfwFrames[i].visible) {
			if (!(g_pfwFrames[i].OwnerWindow && (INT_PTR)(g_pfwFrames[i].OwnerWindow) != -2)) {
				RECT r;
				GetWindowRect(g_pfwFrames[i].hWnd, &r);
				if (r.top - wr->top != g_pfwFrames[i].wndSize.top || r.left - wr->left != g_pfwFrames[i].wndSize.left)
					SetWindowPos(g_pfwFrames[i].hWnd, NULL, g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
			if (g_pfwFrames[i].TitleBar.ShowTitleBar) {
				RECT r;
				GetWindowRect(g_pfwFrames[i].TitleBar.hwnd, &r);
				if (r.top - wr->top != g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar || r.left - wr->left != g_pfwFrames[i].wndSize.left) {
					SetWindowPos(g_pfwFrames[i].TitleBar.hwnd, NULL, g_pfwFrames[i].wndSize.left + dx, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar + dy,
						g_pfwFrames[i].wndSize.right - g_pfwFrames[i].wndSize.left, g_nTitleBarHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
					SetRect(&g_pfwFrames[i].TitleBar.wndSize, g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, g_pfwFrames[i].wndSize.right, g_pfwFrames[i].wndSize.top - g_CluiData.nGapBetweenTitlebar);
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
	GapBetweenFrames = db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);
	g_CluiData.nGapBetweenTitlebar = db_get_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT);

	if (_fCluiFramesModuleNotStarted) return -1;

	//need to enlarge parent
	{
		RECT mainRect;
		int mainHeight, minHeight;
		GetWindowRect(pcli->hwndContactList, &mainRect);
		mainHeight = mainRect.bottom - mainRect.top;
		minHeight = CLUIFrames_GetTotalHeight();
		if (mainHeight < minHeight) {
			BOOL Upward = FALSE;
			Upward = !g_CluiData.fDocked && g_CluiData.fAutoSize && db_get_b(NULL, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT);

			if (Upward)
				mainRect.top = mainRect.bottom - minHeight;
			else
				mainRect.bottom = mainRect.top + minHeight;
			SetWindowPos(pcli->hwndContactList, NULL, mainRect.left, mainRect.top, mainRect.right - mainRect.left, mainRect.bottom - mainRect.top, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		}
		GetWindowRect(pcli->hwndContactList, &mainRect);
		mainHeight = mainRect.bottom - mainRect.top;
	}
	GetClientRect(pcli->hwndContactList, &nRect);
	//$$$ Fixed borders
	if (lParam && lParam != 1 && lParam != 2) {
		RECT oldRect;
		POINT pt;
		RECT *newRect = (RECT *)lParam;
		int dl, dt, dr, db;
		GetWindowRect((HWND)wParam, &oldRect);
		pt.x = nRect.left;
		pt.y = nRect.top;
		ClientToScreen(pcli->hwndContactList, &pt);
		dl = pt.x - oldRect.left;
		dt = pt.y - oldRect.top;
		dr = (oldRect.right - oldRect.left) - (nRect.right - nRect.left) - dl;
		db = (oldRect.bottom - oldRect.top) - (nRect.bottom - nRect.top) - dt;
		nRect.left = newRect->left + dl;
		nRect.top = newRect->top + dt;
		nRect.bottom = newRect->bottom - db;
		nRect.right = newRect->right - dr;
	}


	//if (db_get_b(NULL,"CLUI","ShowSBar",SETTING_SHOWSBAR_DEFAULT))GetWindowRect(pcli->hwndStatus,&rcStatus);
	//else rcStatus.top = rcStatus.bottom = 0;
	// nRect.top--;
	/* $$$	rcStatus.top = rcStatus.bottom = 0;


	nRect.bottom -= nRect.top;
	nRect.bottom -= (rcStatus.bottom-rcStatus.top);
	nRect.right -= nRect.left;
	nRect.left = 0;
	nRect.top = 0;
	ContactListHeight = nRect.bottom; $$$*/

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

	//resizing = FALSE;

	tick = GetTickCount() - tick;

	if (pcli->hwndContactList != 0) CLUI__cliInvalidateRect(pcli->hwndContactList, NULL, TRUE);
	if (pcli->hwndContactList != 0) UpdateWindow(pcli->hwndContactList);

	if (lParam == 2) RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);


	Sleep(0);

	//dont save to database too many times
	if (GetTickCount() - _dwLastStoreTick > 1000) { CLUIFramesStoreAllFrames(); _dwLastStoreTick = GetTickCount(); };

	return 0;
}

int OnFrameTitleBarBackgroundChange(WPARAM, LPARAM)
{
	if (MirandaExiting()) return 0;
	{
		DBVARIANT dbv = { 0 };

		AlignCOLLIconToLeft = db_get_b(NULL, "FrameTitleBar", "AlignCOLLIconToLeft", CLCDEFAULT_COLLICONTOLEFT);

		bkColour = sttGetColor("FrameTitleBar", "BkColour", CLCDEFAULT_BKCOLOUR);
		bkUseWinColours = db_get_b(NULL, "FrameTitleBar", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		SelBkColour = sttGetColor("FrameTitleBar", "TextColour", CLCDEFAULT_TEXTCOLOUR);
		if (hBmpBackground) { DeleteObject(hBmpBackground); hBmpBackground = NULL; }
		if (g_CluiData.fDisableSkinEngine) {
			if (db_get_b(NULL, "FrameTitleBar", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
				if (!db_get_s(NULL, "FrameTitleBar", "BkBitmap", &dbv)) {
					hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
					db_free(&dbv);
				}
			}
			backgroundBmpUse = db_get_w(NULL, "FrameTitleBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
		}
	};

	CLUI__cliInvalidateRect(pcli->hwndContactList, 0, 0);

	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);

	return 0;
}

void DrawBackGround(HWND hwnd, HDC mhdc, HBITMAP hBmpBackground, COLORREF bkColour, DWORD backgroundBmpUse)
{
	HDC hdc;
	RECT clRect, *rcPaint;

	int yScroll = 0;
	PAINTSTRUCT paintst = { 0 };

	//InvalidateRect(hwnd, 0, FALSE);

	HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);

	if (mhdc) {
		hdc = mhdc;
		rcPaint = NULL;
	}
	else {
		hdc = BeginPaint(hwnd, &paintst);
		rcPaint = &(paintst.rcPaint);
	}

	GetClientRect(hwnd, &clRect);
	if (rcPaint == NULL) rcPaint = &clRect;
	if (rcPaint->right - rcPaint->left == 0 || rcPaint->top - rcPaint->bottom == 0) rcPaint = &clRect;
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBmpOsb = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), NULL);
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
			if (backgroundBmpUse & CLBF_PROPORTIONAL) {
				destw = clRect.right;
				//desth = destw*bmp.bmHeight / bmp.bmWidth;
			}
			else {
				destw = clRect.right;
				//desth = bmp.bmHeight;
			}
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

	{

		BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);

		SelectObject(hdcMem, hOldBmp);
		SelectObject(hdcMem, oFont);
		DeleteObject(hBmpOsb);
		DeleteDC(hdcMem);
		paintst.fErase = FALSE;
		//DeleteObject(hFont);
		if (!mhdc) {
			EndPaint(hwnd, &paintst);
		}
	}
}


int DrawTitleBar(HDC hdcMem2, RECT *rect, int Frameid)
{
	BOOL bThemed = FALSE;
	HDC hdcMem;
	RECT rc = *rect;
	HBITMAP b1 = NULL, b2 = NULL;
	hdcMem = CreateCompatibleDC(hdcMem2);

	SetBkMode(hdcMem, TRANSPARENT);
	HFONT hoTTBFont = (HFONT)SelectObject(hdcMem, _hTitleBarFont);
	ske_ResetTextEffect(hdcMem);
	ske_ResetTextEffect(hdcMem2);
	HBRUSH hBack = GetSysColorBrush(COLOR_3DFACE);
	HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem, hBack);

	int pos = id2pos(Frameid);

	if (pos >= 0 && pos < g_nFramesCount) {
		GetClientRect(g_pfwFrames[pos].TitleBar.hwnd, &rc);
		if (g_pfwFrames[pos].floating) {

			rc.bottom = rc.top + g_nTitleBarHeight;
			g_pfwFrames[pos].TitleBar.wndSize = rc;
		}
		else {
			g_pfwFrames[pos].TitleBar.wndSize = rc;
		}
		b1 = ske_CreateDIB32(rc.right - rc.left, rc.bottom - rc.top);
		b2 = (HBITMAP)SelectObject(hdcMem, b1);
		if (g_pfwFrames[pos].floating && !g_CluiData.fDisableSkinEngine) {
			FillRect(hdcMem, &rc, hBack);
			//SelectObject(hdcMem,hoBrush);
			SkinDrawGlyph(hdcMem, &rc, &rc, "Main,ID=FrameCaption");
		}
		else {
			if (g_CluiData.fDisableSkinEngine) {
				if (!hBmpBackground && bkUseWinColours && xpt_IsThemed(_hFrameTitleTheme)) {
					int state = CS_ACTIVE;
					// if (GetForegroundWindow() != pcli->hwndContactList) state = CS_INACTIVE;
					xpt_DrawThemeBackground(_hFrameTitleTheme, hdcMem, WP_SMALLCAPTION, state, &rc, &rc);
					bThemed = TRUE;
				}
				else
					DrawBackGround(g_pfwFrames[pos].TitleBar.hwnd, hdcMem, hBmpBackground, bkColour, backgroundBmpUse);
			}
			else if (!g_CluiData.fLayered) {
				ske_BltBackImage(g_pfwFrames[pos].TitleBar.hwnd, hdcMem, &rc);
			}
			else  BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcMem2, rect->left, rect->top, SRCCOPY);
			SkinDrawGlyph(hdcMem, &rc, &rc, "Main,ID=FrameCaption");
		}
		if (bThemed)
			SetTextColor(hdcMem, GetSysColor(COLOR_CAPTIONTEXT));
		else
			SetTextColor(hdcMem, SelBkColour);
		{
			RECT textrc = rc;
			if (!AlignCOLLIconToLeft) {

				if (g_pfwFrames[pos].TitleBar.hicon != NULL) {
					mod_DrawIconEx_helper(hdcMem, rc.left + 2, rc.top + ((g_nTitleBarHeight >> 1) - (GetSystemMetrics(SM_CYSMICON) >> 1)), g_pfwFrames[pos].TitleBar.hicon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
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
			ske_TextOut(hdcMem, textrc.left, textrc.top, g_pfwFrames[pos].TitleBar.tbname, (int)mir_tstrlen(g_pfwFrames[pos].TitleBar.tbname));
		}

		if (!AlignCOLLIconToLeft)
			mod_DrawIconEx_helper(hdcMem, g_pfwFrames[pos].TitleBar.wndSize.right - GetSystemMetrics(SM_CXSMICON) - 2, rc.top + ((g_nTitleBarHeight >> 1) - (GetSystemMetrics(SM_CXSMICON) >> 1)), g_pfwFrames[pos].collapsed ? LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN) : LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT), GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
		else
			mod_DrawIconEx_helper(hdcMem, rc.left, rc.top + ((g_nTitleBarHeight >> 1) - (GetSystemMetrics(SM_CXSMICON) >> 1)), g_pfwFrames[pos].collapsed ? LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN) : LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT), GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
	}
	{
		if (g_pfwFrames[pos].floating || (!g_CluiData.fLayered)) {
			HRGN rgn = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
			SelectClipRgn(hdcMem2, rgn);
			BitBlt(hdcMem2, rect->left, rect->top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
			DeleteObject(rgn);
		}
		else
			BitBlt(hdcMem2, rect->left, rect->top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
		//MyAlphaBlend(hdcMem2,rect.left,rect.top,rc.right-rc.left,rc.bottom-rc.top,hdcMem, 0, 0, rc.right-rc.left,rc.bottom-rc.top,bf);
	}

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
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_USER + 100:
		return 1;

	case WM_ENABLE:
		if (hwnd != 0) CLUI__cliInvalidateRect(hwnd, NULL, FALSE);
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), 0), Frameid))
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
			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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
			AppendMenu(hmenu, MF_SEPARATOR, 16, _T(""));
			AppendMenu(hmenu, g_pfwFrames[framepos].Locked ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_lock, TranslateT("Lock frame"));
			AppendMenu(hmenu, g_pfwFrames[framepos].visible ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_visible, TranslateT("Visible"));
			AppendMenu(hmenu, g_pfwFrames[framepos].TitleBar.ShowTitleBar ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_showtitlebar, TranslateT("Show title bar"));
			AppendMenu(hmenu, g_pfwFrames[framepos].floating ? (MF_STRING | MF_CHECKED) : MF_STRING, frame_menu_floating, TranslateT("Floating"));
		}

		TrackPopupMenu(hmenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, 0);
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
	{
		if (GetCapture() != hwnd) { break; };
		s_nCurDragBar = -1; s_nLastByPos = -1; s_nOldFrameHeight = -1; ReleaseCapture();
		break;
	};
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
			if (db_get_b(NULL, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
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


		break;
	};
	case WM_MOUSEMOVE:
	{
		POINT pt, pt2;
		RECT wndr;
		int pos;
		//tbinfo
		{
			char TBcapt[255];


			pos = id2pos(Frameid);

			if (pos != -1) {
				int oldflags;


				mir_snprintf(TBcapt, SIZEOF(TBcapt), "%s - h:%d, vis:%d, fl:%d, fl:(%d,%d,%d,%d),or: %d",
					g_pfwFrames[pos].szName, g_pfwFrames[pos].height, g_pfwFrames[pos].visible, g_pfwFrames[pos].floating,
					g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y,
					g_pfwFrames[pos].FloatingSize.x, g_pfwFrames[pos].FloatingSize.y,
					g_pfwFrames[pos].order
					);

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

				GetWindowRect(pcli->hwndContactList, &rcMiranda);
				//GetWindowRect( Frames[pos].ContainerWnd, &rcwnd );
				//IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )
				if (IsWindowVisible(pcli->hwndContactList) && IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
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

				GetWindowRect(pcli->hwndContactList, &rcMiranda);
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
				CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
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
		InvalidateRect(hwnd, NULL, FALSE);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_PAINT:
		if (g_pfwFrames[id2pos(Frameid)].floating || !g_CluiData.fLayered) {
			GetClientRect(hwnd, &rect);
			HDC paintDC = GetDC(hwnd);
			DrawTitleBar(paintDC, &rect, Frameid);
			ReleaseDC(hwnd, paintDC);
			ValidateRect(hwnd, NULL);
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
	if (g_pfwFrames[framepos].ContainerWnd == 0) { return 0; };
	GetClientRect(g_pfwFrames[framepos].ContainerWnd, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	g_pfwFrames[framepos].visible ? CLUI_ShowWindowMod(g_pfwFrames[framepos].ContainerWnd, SW_SHOW/*NOACTIVATE*/) : CLUI_ShowWindowMod(g_pfwFrames[framepos].ContainerWnd, SW_HIDE);



	if (g_pfwFrames[framepos].TitleBar.ShowTitleBar) {
		CLUI_ShowWindowMod(g_pfwFrames[framepos].TitleBar.hwnd, SW_SHOW/*NOACTIVATE*/);
		//if (Frames[framepos].Locked){return 0;};
		g_pfwFrames[framepos].height = height - DEFAULT_TITLEBAR_HEIGHT;

		SetWindowPos(g_pfwFrames[framepos].TitleBar.hwnd, HWND_TOP, 0, 0, width, DEFAULT_TITLEBAR_HEIGHT, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_DRAWFRAME);
		SetWindowPos(g_pfwFrames[framepos].hWnd, HWND_TOP, 0, DEFAULT_TITLEBAR_HEIGHT, width, height - DEFAULT_TITLEBAR_HEIGHT, SWP_SHOWWINDOW);

	}
	else {
		//SetWindowPos(Frames[framepos].TitleBar.hwnd,HWND_TOP, 0, 0, width,DEFAULT_TITLEBAR_HEIGHT,SWP_SHOWWINDOW|SWP_NOMOVE);
		//if (Frames[framepos].Locked){return 0;};
		g_pfwFrames[framepos].height = height;
		CLUI_ShowWindowMod(g_pfwFrames[framepos].TitleBar.hwnd, SW_HIDE);
		SetWindowPos(g_pfwFrames[framepos].hWnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);

	};
	//			CLUIFramesForceUpdateFrame(&Frames[framepos]);
	if (g_pfwFrames[framepos].ContainerWnd != 0) UpdateWindow(g_pfwFrames[framepos].ContainerWnd);
	//GetClientRect(Frames[framepos].TitleBar.hwnd,&Frames[framepos].TitleBar.wndSize);
	GetWindowRect(g_pfwFrames[framepos].hWnd, &g_pfwFrames[framepos].wndSize);
	//Frames[framepos].height = Frames[framepos].wndSize.bottom-Frames[framepos].wndSize.top;
	//GetClientRect(Frames[framepos].hWnd,&Frames[framepos].wndSize);
	//Frames[framepos].height = Frames[framepos].wndSize.bottom-Frames[framepos].wndSize.top;
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
			BYTE alpha;
			if ((wParam != WA_INACTIVE || ((HWND)lParam == hwnd) || GetParent((HWND)lParam) == hwnd)) {
				HWND hw = lParam ? GetParent((HWND)lParam) : 0;
				alpha = db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT);
				if (hw) SetWindowPos(hw, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
				CLUI_SmoothAlphaTransition(hwnd, alpha, 1);
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NOTIFY:
	case WM_PARENTNOTIFY:
	case WM_SYSCOMMAND:
		return SendMessage(pcli->hwndContactList, msg, wParam, lParam);

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
	HWND hwnd = CreateWindowEx(WS_EX_LAYERED, CLUIFrameSubContainerClassName, _T("SubContainerWindow"), WS_POPUP | (!g_CluiData.fLayered ? WS_BORDER : 0), x, y, width, height, parent, 0, g_hInst, 0);
	SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER));
	if (g_CluiData.fOnDesktop) {
		HWND hProgMan = FindWindow(_T("Progman"), NULL);
		if (IsWindow(hProgMan))
			SetParent(hwnd, hProgMan);
	}

	return hwnd;
}

static LRESULT CALLBACK CLUIFrameContainerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	int Frameid = (GetWindowLongPtr(hwnd, GWLP_USERDATA));
	memset(&rect, 0, sizeof(rect));

	switch (msg) {
	case WM_CREATE:
	{
		int framepos;

		framepos = id2pos(Frameid);
		//SetWindowPos(Frames[framepos].TitleBar.hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE  );

		return 0;
	};
	case WM_GETMINMAXINFO:
		//DefWindowProc(hwnd,msg,wParam,lParam);
	{
		int framepos;
		MINMAXINFO minmax;


		framepos = id2pos(Frameid);
		if (framepos < 0 || framepos >= g_nFramesCount) { break; };
		if (!g_pfwFrames[framepos].minmaxenabled) { break; };
		if (g_pfwFrames[framepos].ContainerWnd == 0) { break; };

		if (g_pfwFrames[framepos].Locked) {
			RECT rct;

			GetWindowRect(hwnd, &rct);
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rct.right - rct.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rct.bottom - rct.top;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rct.right - rct.left;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = rct.bottom - rct.top;
			//
			//return 0;
		};


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
			};

		}
		else {


			return(DefWindowProc(hwnd, msg, wParam, lParam));
		};



	}
	//return 0;

	case WM_MOVE:
	{
		int framepos;
		RECT rect;

		framepos = id2pos(Frameid);

		if (framepos < 0 || framepos >= g_nFramesCount) { break; };
		if (g_pfwFrames[framepos].ContainerWnd == 0) { return 0; };

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

			GetWindowRect(pcli->hwndContactList, &rcMiranda);
			//GetWindowRect( Frames[pos].ContainerWnd, &rcwnd );
			//IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )


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
				//ClientToScreen(Frames[pos].TitleBar.hwnd,&ofspt);
				GetWindowRect(g_pfwFrames[framepos].TitleBar.hwnd, &rcwnd);
				ofspt.x = curpt.x - ofspt.x; ofspt.y = curpt.y - ofspt.y;

				g_pfwFrames[framepos].FloatingPos.x = newpos.x;
				g_pfwFrames[framepos].FloatingPos.y = newpos.y;
				CLUIFrames_SetFrameFloat(Frameid, 0);
				//SetWindowPos(Frames[pos].ContainerWnd, 0, newpos.x,newpos.y, 0, 0, SWP_NOSIZE);


				newpt.x = 0; newpt.y = 0;
				ClientToScreen(g_pfwFrames[framepos].TitleBar.hwnd, &newpt);

				GetWindowRect(g_pfwFrames[framepos].hWnd, &rcwnd);
				SetCursorPos(newpt.x + (rcwnd.right - rcwnd.left) / 2, newpt.y + (rcwnd.bottom - rcwnd.top) / 2);
				GetCursorPos(&curpt);

				g_pfwFrames[framepos].TitleBar.oldpos = curpt;


				return 0;
			};

		};


		return 0;
	};

	case WM_SIZE:
	{
		int framepos;
		RECT rect;

		CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);

		framepos = id2pos(Frameid);

		if (framepos < 0 || framepos >= g_nFramesCount) { break; };
		if (g_pfwFrames[framepos].ContainerWnd == 0) { return 0; };
		CLUIFrameResizeFloatingFrame(framepos);

		GetWindowRect(g_pfwFrames[framepos].ContainerWnd, &rect);
		g_pfwFrames[framepos].FloatingPos.x = rect.left;
		g_pfwFrames[framepos].FloatingPos.y = rect.top;
		g_pfwFrames[framepos].FloatingSize.x = rect.right - rect.left;
		g_pfwFrames[framepos].FloatingSize.y = rect.bottom - rect.top;

		CLUIFramesStoreFrameSettings(framepos);


		return 0;
	};
	case WM_LBUTTONDOWN:
	{
		if (db_get_b(NULL, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
			POINT pt;
			GetCursorPos(&pt);
			return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		break;
	}
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		break;
	};

	case WM_DESTROY:
	{
		//{ CLUIFramesStoreAllFrames();};
		return 0;
	};
	/*
	case WM_COMMAND:
	case WM_NOTIFY:
	return(SendMessage(pcli->hwndContactList,msg,wParam,lParam));
	*/


	};
	return DefWindowProc(hwnd, msg, wParam, lParam);
};
static HWND CreateContainerWindow(HWND parent, int x, int y, int width, int height)
{
	return(CreateWindow(_T("FramesContainer"), _T("FramesContainer"), WS_POPUP | WS_THICKFRAME, x, y, width, height, parent, 0, g_hInst, 0));
};


static int _us_DoSetFrameFloat(WPARAM wParam, LPARAM lParam)
{
	HWND hwndtmp, hwndtooltiptmp;


	int pos = id2pos(wParam);
	if (pos >= 0 && pos < g_nFramesCount)

		if (g_pfwFrames[pos].floating || (lParam & 2)) {
			if (g_pfwFrames[pos].OwnerWindow != (HWND)-2 && g_pfwFrames[pos].visible) {
				if (g_pfwFrames[pos].OwnerWindow == 0) g_pfwFrames[pos].OwnerWindow = CreateSubContainerWindow(pcli->hwndContactList, g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y, 10, 10);
				CLUI_ShowWindowMod(g_pfwFrames[pos].OwnerWindow, (g_pfwFrames[pos].visible && g_pfwFrames[pos].collapsed && IsWindowVisible(pcli->hwndContactList)) ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE);
				SetParent(g_pfwFrames[pos].hWnd, g_pfwFrames[pos].OwnerWindow);
				SetParent(g_pfwFrames[pos].TitleBar.hwnd, pcli->hwndContactList);
				SetWindowLongPtr(g_pfwFrames[pos].OwnerWindow, GWLP_USERDATA, g_pfwFrames[pos].id);
				g_pfwFrames[pos].floating = FALSE;
				if (!(lParam & 2)) {
					DestroyWindow(g_pfwFrames[pos].ContainerWnd);
					g_pfwFrames[pos].ContainerWnd = 0;
				}
			}
			else {
				SetParent(g_pfwFrames[pos].hWnd, pcli->hwndContactList);
				SetParent(g_pfwFrames[pos].TitleBar.hwnd, pcli->hwndContactList);
				g_pfwFrames[pos].floating = FALSE;
				if (g_pfwFrames[pos].ContainerWnd) DestroyWindow(g_pfwFrames[pos].ContainerWnd);
				g_pfwFrames[pos].ContainerWnd = 0;
			}
		}
		else {
			RECT recttb, rectw, border;
			LONG_PTR temp;
			int neww, newh;
			BOOLEAN locked;

			g_pfwFrames[pos].oldstyles = GetWindowLongPtr(g_pfwFrames[pos].hWnd, GWL_STYLE);
			g_pfwFrames[pos].TitleBar.oldstyles = GetWindowLongPtr(g_pfwFrames[pos].TitleBar.hwnd, GWL_STYLE);
			locked = g_pfwFrames[pos].Locked;
			g_pfwFrames[pos].Locked = FALSE;
			g_pfwFrames[pos].minmaxenabled = FALSE;

			GetWindowRect(g_pfwFrames[pos].hWnd, &rectw);
			GetWindowRect(g_pfwFrames[pos].TitleBar.hwnd, &recttb);
			if (!g_pfwFrames[pos].TitleBar.ShowTitleBar) {
				recttb.top = recttb.bottom = recttb.left = recttb.right = 0;
			};

			g_pfwFrames[pos].ContainerWnd = CreateContainerWindow(pcli->hwndContactList, g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y, 10, 10);




			SetParent(g_pfwFrames[pos].hWnd, g_pfwFrames[pos].ContainerWnd);
			SetParent(g_pfwFrames[pos].TitleBar.hwnd, g_pfwFrames[pos].ContainerWnd);
			if (g_pfwFrames[pos].OwnerWindow != (HWND)-2 && g_pfwFrames[pos].OwnerWindow != 0) {
				DestroyWindow(g_pfwFrames[pos].OwnerWindow);
				g_pfwFrames[pos].OwnerWindow = 0;
			}

			GetBorderSize(g_pfwFrames[pos].ContainerWnd, &border);


			SetWindowLongPtr(g_pfwFrames[pos].ContainerWnd, GWLP_USERDATA, g_pfwFrames[pos].id);
			if ((lParam == 1)) {
				//if ((Frames[pos].FloatingPos.x != 0) && (Frames[pos].FloatingPos.y != 0))
				{
					if (g_pfwFrames[pos].FloatingPos.x < 0) { g_pfwFrames[pos].FloatingPos.x = 0; };
					if (g_pfwFrames[pos].FloatingPos.y < 0) { g_pfwFrames[pos].FloatingPos.y = 0; };

					SetWindowPos(g_pfwFrames[pos].ContainerWnd, HWND_TOPMOST, g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y, g_pfwFrames[pos].FloatingSize.x, g_pfwFrames[pos].FloatingSize.y, SWP_HIDEWINDOW | SWP_NOACTIVATE);
				}
			}
			else if (lParam == 0) {
				neww = rectw.right - rectw.left + border.left + border.right;
				newh = (rectw.bottom - rectw.top) + (recttb.bottom - recttb.top) + border.top + border.bottom;
				if (neww < 20) { neww = 40; };
				if (newh < 20) { newh = 40; };
				if (g_pfwFrames[pos].FloatingPos.x < 20) { g_pfwFrames[pos].FloatingPos.x = 40; };
				if (g_pfwFrames[pos].FloatingPos.y < 20) { g_pfwFrames[pos].FloatingPos.y = 40; };

				SetWindowPos(g_pfwFrames[pos].ContainerWnd, HWND_TOPMOST, g_pfwFrames[pos].FloatingPos.x, g_pfwFrames[pos].FloatingPos.y, neww, newh, SWP_HIDEWINDOW | SWP_NOACTIVATE);
			};


			SetWindowText(g_pfwFrames[pos].ContainerWnd, g_pfwFrames[pos].TitleBar.tbname);

			temp = GetWindowLongPtr(g_pfwFrames[pos].ContainerWnd, GWL_EXSTYLE);
			temp |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
			SetWindowLongPtr(g_pfwFrames[pos].ContainerWnd, GWL_EXSTYLE, temp);

			g_pfwFrames[pos].floating = TRUE;
			g_pfwFrames[pos].Locked = locked;

		}
		CLUIFramesStoreFrameSettings(pos);
		g_pfwFrames[pos].minmaxenabled = TRUE;
		hwndtooltiptmp = g_pfwFrames[pos].TitleBar.hwndTip;

		hwndtmp = g_pfwFrames[pos].ContainerWnd;

		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
		if (hwndtmp) SendMessage(hwndtmp, WM_SIZE, 0, 0);


		SetWindowPos(hwndtooltiptmp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		return 0;
}

int CLUIFrameOnModulesLoad(WPARAM, LPARAM)
{
	/* HOOK */
	CLUIFramesLoadMainMenu();
	CLUIFramesCreateMenuForFrame(-1, -1, 000010000, Menu_AddContextFrameMenuItem);
	return 0;
}

int CLUIFrameOnModulesUnload(WPARAM, LPARAM)
{
	g_CluiData.bSTATE = STATE_PREPEARETOEXIT;

	if (!_hmiVisible)
		return 0;

	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiVisible, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiTBVisible, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiLock, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiColl, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiFloating, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiAlignTop, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiAlignClient, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiAlignBottom, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiBorder, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiAlignRoot, 0);

	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiPosUp, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiPosDown, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiPosRoot, 0);

	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiVisible, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiTBVisible, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiLock, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiColl, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiFloating, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiBorder, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiAlignRoot, 0);
	CallService(MO_REMOVEMENUITEM, (LPARAM)_hmiPosRoot, 0);
	_hmiVisible = 0;

	_AniAva_OnModulesUnload();
	return 0;
}

int LoadCLUIFramesModule(void)
{
	_cluiFramesModuleCSInitialized = TRUE;

	WNDCLASS wndclass = { 0 };
	wndclass.style = CS_DBLCLKS;//|CS_HREDRAW|CS_VREDRAW ;
	wndclass.lpfnWndProc = CLUIFrameTitleBarProc;
	wndclass.hInstance = g_hInst;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wndclass.lpszClassName = CLUIFrameTitleBarClassName;
	RegisterClass(&wndclass);

	WNDCLASS subconclass = { 0 };
	subconclass.style = CS_DBLCLKS;//|CS_HREDRAW|CS_VREDRAW ;
	subconclass.lpfnWndProc = CLUIFrameSubContainerProc;
	subconclass.hInstance = g_hInst;
	subconclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	subconclass.lpszClassName = CLUIFrameSubContainerClassName;
	RegisterClass(&subconclass);

	//container helper
	WNDCLASS cntclass = { 0 };
	cntclass.style = CS_DBLCLKS/*|CS_HREDRAW|CS_VREDRAW*/ | CS_DROPSHADOW;
	cntclass.lpfnWndProc = CLUIFrameContainerWndProc;
	cntclass.hInstance = g_hInst;
	cntclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	cntclass.lpszClassName = _T("FramesContainer");
	RegisterClass(&cntclass);
	//end container helper

	GapBetweenFrames = db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT);

	g_nFramesCount = 0;

	InitFramesMenus();

	HookEvent(ME_SYSTEM_MODULESLOADED, CLUIFrameOnModulesLoad);
	HookEvent(ME_CLIST_PREBUILDFRAMEMENU, CLUIFramesModifyContextMenuForFrame);
	HookEvent(ME_CLIST_PREBUILDMAINMENU, CLUIFrameOnMainMenuBuild);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, CLUIFrameOnModulesUnload);

	CreateCluiFramesServices();

	hWndExplorerToolBar = FindWindowEx(0, 0, _T("Shell_TrayWnd"), NULL);
	OnFrameTitleBarBackgroundChange(0, 0);
	_fCluiFramesModuleNotStarted = FALSE;
	return 0;
}

static INT_PTR UnloadMainMenu()
{
	CLUIFrameOnModulesUnload(0, 0);
	if (_hmiRoot != HGENMENU_ROOT) {
		CallService(MO_REMOVEMENUITEM, (WPARAM)_hmiRoot, 0);
		_hmiRoot = HGENMENU_ROOT;
	}

	return (INT_PTR)_hmiRoot;
}

int UnLoadCLUIFramesModule(void)
{
	_fCluiFramesModuleNotStarted = TRUE;
	if (hBmpBackground) { DeleteObject(hBmpBackground); hBmpBackground = NULL; }
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	CLUIFramesStoreAllFrames();

	UnloadMainMenu();
	for (int i = 0; i < g_nFramesCount; i++) {
		if (g_pfwFrames[i].hWnd != pcli->hwndContactTree)
			DestroyWindow(g_pfwFrames[i].hWnd);

		g_pfwFrames[i].hWnd = (HWND)-1;
		DestroyWindow(g_pfwFrames[i].TitleBar.hwnd);
		g_pfwFrames[i].TitleBar.hwnd = (HWND)-1;
		if (g_pfwFrames[i].ContainerWnd && g_pfwFrames[i].ContainerWnd != (HWND)(-2) && g_pfwFrames[i].ContainerWnd != (HWND)(-1)) DestroyWindow(g_pfwFrames[i].ContainerWnd);
		g_pfwFrames[i].ContainerWnd = (HWND)-1;
		if (g_pfwFrames[i].TitleBar.hmenu) DestroyMenu(g_pfwFrames[i].TitleBar.hmenu);
		if (g_pfwFrames[i].OwnerWindow && g_pfwFrames[i].OwnerWindow != (HWND)(-2) && g_pfwFrames[i].OwnerWindow != (HWND)(-1))
			DestroyWindow(g_pfwFrames[i].OwnerWindow);
		g_pfwFrames[i].OwnerWindow = (HWND)-2;
		if (g_pfwFrames[i].UpdateRgn) DeleteObject(g_pfwFrames[i].UpdateRgn);

		mir_free_and_nil(g_pfwFrames[i].name);
		mir_free_and_nil(g_pfwFrames[i].szName);
		mir_free_and_nil(g_pfwFrames[i].TitleBar.tbname);
		mir_free_and_nil(g_pfwFrames[i].TitleBar.sztbname);
		mir_free_and_nil(g_pfwFrames[i].TitleBar.tooltip);
		mir_free_and_nil(g_pfwFrames[i].TitleBar.sztooltip);

	}
	free(g_pfwFrames);
	g_pfwFrames = NULL;
	g_nFramesCount = 0;
	UnregisterClass(CLUIFrameTitleBarClassName, g_hInst);
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
		frm->PaintCallbackProc = NULL;
	return 1;
}

int CLUIFrames_SetLayeredMode(BOOL fLayeredMode, HWND hwnd)
{
	for (int i = 0; i < g_nFramesCount; i++) {
		if (fLayeredMode) {
			if (g_pfwFrames[i].visible && GetParent(g_pfwFrames[i].hWnd) == pcli->hwndContactList && g_pfwFrames[i].PaintCallbackProc == NULL) {
				//create owner window
				g_pfwFrames[i].OwnerWindow = CreateSubContainerWindow(pcli->hwndContactList, g_pfwFrames[i].FloatingPos.x, g_pfwFrames[i].FloatingPos.y, 10, 10);
				SetParent(g_pfwFrames[i].hWnd, g_pfwFrames[i].OwnerWindow);
			}
		}
		else if (GetParent(g_pfwFrames[i].hWnd) == g_pfwFrames[i].OwnerWindow) {
			SetParent(g_pfwFrames[i].hWnd, hwnd);
			if ((INT_PTR)g_pfwFrames[i].OwnerWindow > 0) {
				DestroyWindow(g_pfwFrames[i].OwnerWindow);
				g_pfwFrames[i].OwnerWindow = (HWND)-2;
			}
		}
	}

	CLUIFrames_UpdateFrame((WPARAM)-1, 0);  //update all frames
	return 0;
}

int CLUIFrames_UpdateBorders()
{
	for (int i = 0; i < g_nFramesCount; i++) {
		if (!g_pfwFrames[i].floating) {
			LONG_PTR style = GetWindowLongPtr(g_pfwFrames[i].hWnd, GWL_STYLE) & (~WS_BORDER);
			if (!g_CluiData.fLayered && g_pfwFrames[i].UseBorder) style |= WS_BORDER;
			SetWindowLongPtr(g_pfwFrames[i].hWnd, GWL_STYLE, style);
			CLUIFramesModifyMainMenuItems(g_pfwFrames[i].id, 0);
			RedrawWindow(g_pfwFrames[i].hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	return 0;
}
