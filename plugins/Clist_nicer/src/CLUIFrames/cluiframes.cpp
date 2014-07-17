/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include <commonheaders.h>
#include "cluiframes.h"
HFONT __fastcall ChangeToFont(HDC hdc, struct ClcData *dat, int id, int *fontHeight);

extern HWND g_hwndViewModeFrame, g_hwndEventArea;
extern int mf_updatethread_running;

extern HANDLE hThreadMFUpdate;

void MF_UpdateThread(LPVOID);

HANDLE hStatusBarShowToolTipEvent, hStatusBarHideToolTipEvent;
HANDLE g_hEventThread = 0;

//not needed,now use MS_CLIST_FRAMEMENUNOTIFY service
//HANDLE hPreBuildFrameMenuEvent;//external event from clistmenus

LOGFONT TitleBarLogFont = {0};

extern HINSTANCE g_hInst;

//we use dynamic frame list,
//but who wants so huge number of frames ??
#define MAX_FRAMES		40

#define UNCOLLAPSED_FRAME_SIZE		0

//legacy menu support
#define frame_menu_lock				1
#define frame_menu_visible			2
#define frame_menu_showtitlebar		3
#define frame_menu_floating			4
#define frame_menu_skinned			5

static int UpdateTBToolTip(int framepos);
INT_PTR CLUIFrameSetFloat(WPARAM wParam, LPARAM lParam);
int CLUIFrameResizeFloatingFrame(int framepos);
extern int InitFramesMenus(void);
static int CLUIFramesReSort();

boolean FramesSysNotStarted = TRUE;
HPEN g_hPenCLUIFrames = 0;

static SortData g_sd[MAX_FRAMES];

static HHOOK g_hFrameHook = 0;

static int sortfunc(const void *a, const void *b)
{
	SortData *sd1, *sd2;
	sd1 = (SortData *)a;
	sd2 = (SortData *)b;
	if (sd1->order > sd2->order)
		return 1;
	if (sd1->order < sd2->order)
		return -1;
	return 0;
}

static FRAMEWND *Frames = NULL;

FRAMEWND *wndFrameCLC = NULL, *wndFrameEventArea = NULL, *wndFrameViewMode = NULL;

static int nFramescount = 0;
static int alclientFrame = -1;//for fast access to frame with alclient properties
static int NextFrameId = 100;

static int TitleBarH = DEFAULT_TITLEBAR_HEIGHT;
static boolean resizing = FALSE;

// menus
static HANDLE contMIVisible, contMITitle, contMITBVisible, contMILock, contMIColl, contMIFloating;
static HANDLE contMIAlignRoot;
static HANDLE contMIAlignTop, contMIAlignClient, contMIAlignBottom;
static HANDLE contMIBorder, contMISkinned;
static HGENMENU MainMIRoot = (HGENMENU)-1;

// others
static int ContactListHeight;
static int LastStoreTick = 0;

static int lbypos = -1;
static int oldframeheight = -1;
static int curdragbar = -1;
static mir_cs csFrameHook;

static BOOLEAN CLUIFramesFitInSize(void);
HWND hWndExplorerToolBar;
static int GapBetweenFrames = 1;

static int RemoveItemFromList(int pos, FRAMEWND **lpFrames, int *FrameItemCount)
{
	memcpy(&((*lpFrames)[pos]), &((*lpFrames)[pos + 1]), sizeof(FRAMEWND) * (*FrameItemCount - pos - 1));
	(*FrameItemCount)--;
	return 0;
}

static int id2pos(int id)
{
	int i;

	if (FramesSysNotStarted)
		return -1;

	for (i = 0; i < nFramescount; i++) {
		if (Frames[i].id == id)
			return i;
	}
	return -1;
}

int __forceinline btoint(BOOLEAN b)
{
	return (b ? 1 : 0);
}

static FRAMEWND *FindFrameByWnd(HWND hwnd)
{
	if (hwnd == NULL)
		return NULL;

	BOOL bFound = FALSE;
	for (int i = 0; i < nFramescount; i++)
		if ((Frames[i].floating) && (Frames[i].ContainerWnd == hwnd))
			return (&Frames[i]);

	return NULL;
}

static void DockThumbs(FRAMEWND *pThumbLeft, FRAMEWND *pThumbRight, BOOL bMoveLeft)
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

BOOLEAN bMoveTogether;

static void PositionThumb(FRAMEWND *pThumb, short nX, short nY)
{
	FRAMEWND *pCurThumb = &Frames[0];
	FRAMEWND *pDockThumb = pThumb;
	FRAMEWND fakeMainWindow;
	FRAMEWND fakeTaskBarWindow;
	RECT rc;
	RECT rcThumb;
	RECT rcOld;
	SIZE sizeScreen;
	int nOffs = 10;
	POINT pt;
	RECT rcLeft;
	RECT rcTop;
	RECT rcRight;
	RECT rcBottom;
	int frmidx = 0;

	if (pThumb == NULL)
		return;

	sizeScreen.cx = GetSystemMetrics(SM_CXSCREEN);
	sizeScreen.cy = GetSystemMetrics(SM_CYSCREEN);

	// Get thumb dimnsions
	GetWindowRect(pThumb->ContainerWnd, &rcThumb);
	int nWidth = rcThumb.right - rcThumb.left;
	int nHeight = rcThumb.bottom - rcThumb.top;

	// Docking to the edges of the screen
	int nNewX = nX < nOffs ? 0 : nX;
	nNewX = nNewX > (sizeScreen.cx - nWidth - nOffs) ? (sizeScreen.cx - nWidth) : nNewX;
	int nNewY = nY < nOffs ? 0 : nY;
	nNewY = nNewY > (sizeScreen.cy - nHeight - nOffs) ? (sizeScreen.cy - nHeight) : nNewY;

	bool bLeading = pThumb->dockOpt.hwndRight != NULL;

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

				bool bDockedLeft = false, bDockedRight = false, bDocked = false;

				// Upper-left
				pt.x = rcThumb.left;
				pt.y = rcThumb.top;

				if (PtInRect(&rcRight, pt)) {
					nNewX = rc.right;
					bDocked = true;
				}

				if (PtInRect(&rcBottom, pt)) {
					nNewY = rc.bottom;
					if (PtInRect(&rcLeft, pt))
						nNewX = rc.left;
				}

				if (PtInRect(&rcTop, pt)) {
					nNewY = rc.top;
					bDockedLeft = bDocked;
				}

				// Upper-right
				pt.x = rcThumb.right;
				pt.y = rcThumb.top;
				bDocked = false;

				if (!bLeading && PtInRect(&rcLeft, pt)) {
					if (!bDockedLeft) {
						nNewX = rc.left - nWidth;
						bDocked = true;
					}
					else if (rc.right == rcThumb.left)
						bDocked = true;
				}


				if (PtInRect(&rcBottom, pt)) {
					nNewY = rc.bottom;
					if (PtInRect(&rcRight, pt))
						nNewX = rc.right - nWidth;
				}

				if (!bLeading && PtInRect(&rcTop, pt)) {
					nNewY = rc.top;
					bDockedRight = bDocked;
				}

				if (bMoveTogether) {
					if (bDockedRight)
						DockThumbs(pThumb, pCurThumb, TRUE);

					if (bDockedLeft)
						DockThumbs(pCurThumb, pThumb, FALSE);
				}

				// Lower-left
				pt.x = rcThumb.left;
				pt.y = rcThumb.bottom;

				if (PtInRect(&rcRight, pt))
					nNewX = rc.right;

				if (PtInRect(&rcTop, pt)) {
					nNewY = rc.top - nHeight;

					if (PtInRect(&rcLeft, pt))
						nNewX = rc.left;
				}


				// Lower-right
				pt.x = rcThumb.right;
				pt.y = rcThumb.bottom;

				if (!bLeading && PtInRect(&rcLeft, pt))
					nNewX = rc.left - nWidth;

				if (!bLeading && PtInRect(&rcTop, pt)) {
					nNewY = rc.top - nHeight;

					if (PtInRect(&rcRight, pt))
						nNewX = rc.right - nWidth;
				}
			}
		}

		frmidx++;
		if (pCurThumb->ContainerWnd == fakeTaskBarWindow.ContainerWnd)
			break;

		if (pCurThumb->ContainerWnd == fakeMainWindow.ContainerWnd) {
			pCurThumb = &fakeTaskBarWindow;
			continue;
		}
		if (frmidx == nFramescount) {
			pCurThumb = &fakeMainWindow;
			continue;
		}
		pCurThumb = &Frames[frmidx];
	}

	// Adjust coords once again
	nNewX = nNewX < nOffs ? 0 : nNewX;
	nNewX = nNewX > (sizeScreen.cx - nWidth - nOffs) ? (sizeScreen.cx - nWidth) : nNewX;
	nNewY = nNewY < nOffs ? 0 : nNewY;
	nNewY = nNewY > (sizeScreen.cy - nHeight - nOffs) ? (sizeScreen.cy - nHeight) : nNewY;
	SetWindowPos(pThumb->ContainerWnd, 0, nNewX, nNewY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// OK, move all docked thumbs
	if (bMoveTogether) {
		pDockThumb = FindFrameByWnd(pDockThumb->dockOpt.hwndRight);
		PositionThumb(pDockThumb, (short)(nNewX + nWidth), (short)nNewY);
	}
}

void GetBorderSize(HWND hwnd, RECT *rect)
{
	RECT wr, cr;
	POINT pt1, pt2;

	GetWindowRect(hwnd, &wr);
	GetClientRect(hwnd, &cr);
	pt1.y = cr.top;
	pt1.x = cr.left;
	pt2.y = cr.bottom;
	pt2.x = cr.right;

	ClientToScreen(hwnd, &pt1);
	ClientToScreen(hwnd, &pt2);

	cr.top = pt1.y;
	cr.left = pt1.x;
	cr.bottom = pt2.y;
	cr.right = pt2.x;

	rect->top = cr.top - wr.top;
	rect->left = cr.left - wr.left;
	rect->right = wr.right - cr.right;
	rect->bottom = wr.bottom - cr.bottom;
}

//append string
char __forceinline *AS(char *str, const char *setting, char *addstr)
{
	if (str != NULL) {
		strcpy(str, setting);
		strcat(str, addstr);
	}
	return str;
}

int DBLoadFrameSettingsAtPos(int pos, int Frameid)
{
	char sadd[15];
	char buf[255];

	_itoa(pos, sadd, 10);

	Frames[Frameid].collapsed				= cfg::getByte(CLUIFrameModule, AS(buf, "Collapse", sadd), Frames[Frameid].collapsed);

	Frames[Frameid].Locked					= cfg::getByte(CLUIFrameModule, AS(buf, "Locked", sadd), Frames[Frameid].Locked);
	Frames[Frameid].visible					= cfg::getByte(CLUIFrameModule, AS(buf, "Visible", sadd), Frames[Frameid].visible);
	Frames[Frameid].TitleBar.ShowTitleBar	= cfg::getByte(CLUIFrameModule, AS(buf, "TBVisile", sadd), Frames[Frameid].TitleBar.ShowTitleBar);

	Frames[Frameid].height					= cfg::getWord(CLUIFrameModule, AS(buf, "Height", sadd), Frames[Frameid].height);
	Frames[Frameid].HeightWhenCollapsed		= cfg::getWord(CLUIFrameModule, AS(buf, "HeightCollapsed", sadd), 0);
	Frames[Frameid].align					= cfg::getWord(CLUIFrameModule, AS(buf, "Align", sadd), Frames[Frameid].align);

	Frames[Frameid].FloatingPos.x		= DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatX", sadd), 100, 0, 1024);
	Frames[Frameid].FloatingPos.y		= DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatY", sadd), 100, 0, 1024);
	Frames[Frameid].FloatingSize.x		= DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatW", sadd), 100, 0, 1024);
	Frames[Frameid].FloatingSize.y		= DBGetContactSettingRangedWord(0, CLUIFrameModule, AS(buf, "FloatH", sadd), 100, 0, 1024);

	Frames[Frameid].floating			= cfg::getByte(CLUIFrameModule, AS(buf, "Floating", sadd), 0);
	Frames[Frameid].order				= cfg::getWord(CLUIFrameModule, AS(buf, "Order", sadd), 0);

	Frames[Frameid].UseBorder			= cfg::getByte(CLUIFrameModule, AS(buf, "UseBorder", sadd), Frames[Frameid].UseBorder);
	Frames[Frameid].Skinned				= cfg::getByte(CLUIFrameModule, AS(buf, "Skinned", sadd), Frames[Frameid].Skinned);
	return 0;
}

int DBStoreFrameSettingsAtPos(int pos, int Frameid)
{
	char sadd[16];
	char buf[255];

	_itoa(pos, sadd, 10);

	cfg::writeTString(0, CLUIFrameModule, AS(buf, "Name", sadd), Frames[Frameid].name);
	//boolean
	cfg::writeByte(0, CLUIFrameModule, AS(buf, "Collapse", sadd), (BYTE)btoint(Frames[Frameid].collapsed));
	cfg::writeByte(0, CLUIFrameModule, AS(buf, "Locked", sadd), (BYTE)btoint(Frames[Frameid].Locked));
	cfg::writeByte(0, CLUIFrameModule, AS(buf, "Visible", sadd), (BYTE)btoint(Frames[Frameid].visible));
	cfg::writeByte(0, CLUIFrameModule, AS(buf, "TBVisile", sadd), (BYTE)btoint(Frames[Frameid].TitleBar.ShowTitleBar));

	cfg::writeWord(CLUIFrameModule, AS(buf, "Height", sadd), (WORD)Frames[Frameid].height);
	cfg::writeWord(CLUIFrameModule, AS(buf, "HeightCollapsed", sadd), (WORD)Frames[Frameid].HeightWhenCollapsed);
	cfg::writeWord(CLUIFrameModule, AS(buf, "Align", sadd), (WORD)Frames[Frameid].align);
	//FloatingPos
	cfg::writeWord(CLUIFrameModule, AS(buf, "FloatX", sadd), (WORD)Frames[Frameid].FloatingPos.x);
	cfg::writeWord(CLUIFrameModule, AS(buf, "FloatY", sadd), (WORD)Frames[Frameid].FloatingPos.y);
	cfg::writeWord(CLUIFrameModule, AS(buf, "FloatW", sadd), (WORD)Frames[Frameid].FloatingSize.x);
	cfg::writeWord(0, CLUIFrameModule, AS(buf, "FloatH", sadd), (WORD)Frames[Frameid].FloatingSize.y);

	cfg::writeByte(0, CLUIFrameModule, AS(buf, "Floating", sadd), (BYTE)btoint(Frames[Frameid].floating));
	cfg::writeByte(0, CLUIFrameModule, AS(buf, "UseBorder", sadd), (BYTE)btoint(Frames[Frameid].UseBorder));
	cfg::writeWord(0, CLUIFrameModule, AS(buf, "Order", sadd), (WORD)Frames[Frameid].order);

	cfg::writeByte(CLUIFrameModule, AS(buf, "Skinned", sadd), Frames[Frameid].Skinned);
	return 0;
}

int LocateStorePosition(int Frameid, int maxstored)
{
	if (Frames[Frameid].name == NULL) return -1;

	for (int i = 0; i < maxstored; i++) {
		char settingname[255];
		mir_snprintf(settingname, sizeof(settingname), "Name%d", i);
		ptrT frmname(db_get_tsa(0, CLUIFrameModule, settingname));
		if (frmname == NULL) continue;
		if (lstrcmpi(frmname, Frames[Frameid].name) == 0)
			return i;
	}
	return -1;
}

int CLUIFramesLoadFrameSettings(int Frameid)
{
	int storpos, maxstored;

	if (FramesSysNotStarted) return -1;

	if (Frameid < 0 || Frameid >= nFramescount)
		return -1;

	maxstored = cfg::getWord(CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1)
		return 0;

	storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1)
		return 0;

	DBLoadFrameSettingsAtPos(storpos, Frameid);
	return 0;
}

int CLUIFramesStoreFrameSettings(int Frameid)
{
	int maxstored, storpos;

	if (FramesSysNotStarted)
		return -1;

	if (Frameid < 0 || Frameid >= nFramescount)
		return -1;

	maxstored = cfg::getWord(CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1)
		maxstored = 0;

	storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1) {
		storpos = maxstored;
		maxstored++;
	}

	DBStoreFrameSettingsAtPos(storpos, Frameid);
	cfg::writeWord(CLUIFrameModule, "StoredFrames", (WORD)maxstored);
	return 0;
}

int CLUIFramesStoreAllFrames()
{
	int i;

	if (FramesSysNotStarted)
		return -1;

	if (cfg::shutDown)
		return -1;

	mir_cslock lck(csFrameHook);
	for (i = 0;i < nFramescount;i++)
		CLUIFramesStoreFrameSettings(i);
	return 0;
}

// Get client frame
int CLUIFramesGetalClientFrame(void)
{
	int i;
	if (FramesSysNotStarted)
		return -1;

	if (alclientFrame != -1) {
		/* this value could become invalid if RemoveItemFromList was called,
		 * so we double-check */
		if (alclientFrame < nFramescount) {
			if (Frames[alclientFrame].align == alClient) {
				return alclientFrame;
			}
		}
	}

	for (i = 0;i < nFramescount;i++)
		if (Frames[i].align == alClient) {
			alclientFrame = i;
			return i;
		}
	return -1;
}

HMENU CLUIFramesCreateMenuForFrame(int frameid, int root, int popuppos, HGENMENU (*pfnAdd )( CLISTMENUITEM* ))
{
	HGENMENU menuid;
	int framepos = id2pos(frameid);

	if (FramesSysNotStarted)
		return NULL;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&FrameTitle");
	mi.flags = CMIF_CHILDPOPUP | CMIF_GRAYED;
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMITitle = menuid;
	else Frames[framepos].MenuHandles.MITitle = menuid;

	popuppos += 100000;
	mi.hIcon = NULL;
	mi.cbSize = sizeof(mi);
	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Visible");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszContactOwner = 0;
	mi.pszService = MS_CLIST_FRAMES_SHFRAME;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMIVisible = menuid;
	else Frames[framepos].MenuHandles.MIVisible = menuid;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Show title bar");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SHFRAMETITLEBAR;
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMITBVisible = menuid;
	else Frames[framepos].MenuHandles.MITBVisible = menuid;


	popuppos += 100000;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Locked");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_ULFRAME;
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMILock = menuid;
	else Frames[framepos].MenuHandles.MILock = menuid;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Collapsed");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_UCOLLFRAME;
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMIColl = menuid;
	else Frames[framepos].MenuHandles.MIColl = menuid;

	//floating
	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Floating mode");
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszService = "Set_Floating";
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMIFloating = menuid;
	else Frames[framepos].MenuHandles.MIFloating = menuid;


	popuppos += 100000;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Border");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SETUNBORDER;
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMIBorder = menuid;
	else Frames[framepos].MenuHandles.MIBorder = menuid;

	popuppos += 100000;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Skinned frame");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SETSKINNED;
	mi.pszContactOwner = 0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMISkinned = menuid;
	else Frames[framepos].MenuHandles.MISkinned = menuid;

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
		if (frameid == -1) contMIAlignRoot = menuid;
		else Frames[framepos].MenuHandles.MIAlignRoot = menuid;

		mi.flags = CMIF_CHILDPOPUP;
		//align top
		mi.pszPopupName = (char *)menuid;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Top");
		mi.pszService = CLUIFRAMESSETALIGNALTOP;
		mi.pszContactOwner = (char *)alTop;
		menuid = pfnAdd(&mi);
		if (frameid == -1) contMIAlignTop = menuid;
		else Frames[framepos].MenuHandles.MIAlignTop = menuid;


		//align client
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Client");
		mi.pszService = CLUIFRAMESSETALIGNALCLIENT;
		mi.pszContactOwner = (char *)alClient;
		menuid = pfnAdd(&mi);
		if (frameid == -1) contMIAlignClient = menuid;
		else Frames[framepos].MenuHandles.MIAlignClient = menuid;

		//align bottom
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Bottom");
		mi.pszService = CLUIFRAMESSETALIGNALBOTTOM;
		mi.pszContactOwner = (char *)alBottom;
		menuid = pfnAdd(&mi);
		if (frameid == -1) contMIAlignBottom = menuid;
		else Frames[framepos].MenuHandles.MIAlignBottom = menuid;
	}

	{   //position
		//position root
		mi.pszPopupName = (char *)root;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Position");
		mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTPOPUP;
		mi.pszService = "";
		mi.pszContactOwner = (char *)0;
		menuid = pfnAdd(&mi);

		mi.pszPopupName = (char *)menuid;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Up");
		mi.flags = CMIF_CHILDPOPUP;
		mi.pszService = CLUIFRAMESMOVEUP;
		mi.pszContactOwner = (char *)1;
		pfnAdd(&mi);

		mi.pszPopupName = (char *)menuid;
		mi.popupPosition = frameid;
		mi.position = popuppos++;
		mi.pszName = LPGEN("&Down");
		mi.flags = CMIF_CHILDPOPUP;
		mi.pszService = CLUIFRAMESMOVEDOWN;
		mi.pszContactOwner = (char *) - 1;
		pfnAdd(&mi);
	}

	return 0;
}

int ModifyMItem(WPARAM wParam, LPARAM lParam)
{
	return CallService(MS_CLIST_MODIFYMENUITEM, wParam, lParam);
}

static int CLUIFramesModifyContextMenuForFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos((INT_PTR)wParam);
	if (pos >= 0 && pos < nFramescount) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_NAME | CMIF_CHILDPOPUP | CMIF_TCHAR;
		mi.ptszName = Frames[pos].TitleBar.tbname ? Frames[pos].TitleBar.tbname : Frames[pos].name;
		ModifyMItem((WPARAM)contMITitle, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].visible) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIVisible, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].Locked) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMILock, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].TitleBar.ShowTitleBar) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMITBVisible, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].floating) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIFloating, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if ((Frames[pos].UseBorder)) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIBorder, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if ((Frames[pos].Skinned)) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMISkinned, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].align&alTop) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIAlignTop, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].align&alClient) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIAlignClient, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].align&alBottom) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIAlignBottom, (LPARAM)&mi);


		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if ( !Frames[pos].collapsed) mi.flags |= CMIF_CHECKED;
		if ((!Frames[pos].visible) || (Frames[pos].Locked) || (pos == CLUIFramesGetalClientFrame())) mi.flags |= CMIF_GRAYED;
		ModifyMItem((WPARAM)contMIColl, (LPARAM)&mi);
	}
	return 0;
}

INT_PTR CLUIFramesModifyMainMenuItems(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos((INT_PTR)wParam);

	if (pos >= 0 && pos < nFramescount) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_NAME | CMIF_CHILDPOPUP | CMIF_TCHAR;
		mi.ptszName = Frames[pos].TitleBar.tbname ? Frames[pos].TitleBar.tbname : Frames[pos].name;
		Menu_ModifyItem(Frames[pos].MenuHandles.MITitle, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].visible) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIVisible, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].Locked) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MILock, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].TitleBar.ShowTitleBar) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MITBVisible, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].floating) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIFloating, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if ((Frames[pos].UseBorder)) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIBorder, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if ((Frames[pos].Skinned)) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MISkinned, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP | ((Frames[pos].align & alClient) ? CMIF_GRAYED : 0);
		if (Frames[pos].align&alTop) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIAlignTop, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if (Frames[pos].align&alClient) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIAlignClient, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP | ((Frames[pos].align & alClient) ? CMIF_GRAYED : 0);
		if (Frames[pos].align&alBottom) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIAlignBottom, &mi);

		mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP;
		if ( !Frames[pos].collapsed) mi.flags |= CMIF_CHECKED;
		if ((!Frames[pos].visible) || Frames[pos].Locked || (pos == CLUIFramesGetalClientFrame())) mi.flags |= CMIF_GRAYED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIColl, &mi);
	}
	return 0;
}

INT_PTR CLUIFramesGetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos(HIWORD(wParam));
	if (pos < 0 || pos >= nFramescount)
		return -1;

	switch (LOWORD(wParam)) {
	case FO_NAME:
		return (INT_PTR)Frames[pos].name;

	case FO_TBNAME:
		return (INT_PTR)Frames[pos].TitleBar.tbname;

	case FO_TBTIPNAME:
		return (INT_PTR)Frames[pos].TitleBar.tooltip;

	case FO_TBSTYLE:
		return GetWindowLongPtr(Frames[pos].TitleBar.hwnd, GWL_STYLE);

	case FO_TBEXSTYLE:
		return GetWindowLongPtr(Frames[pos].TitleBar.hwnd, GWL_EXSTYLE);

	case FO_ICON:
		return (INT_PTR)Frames[pos].TitleBar.hicon;

	case FO_HEIGHT:
		return (INT_PTR)Frames[pos].height;

	case FO_ALIGN:
		return (INT_PTR)Frames[pos].align;

	case FO_FLOATING:
		return (int)Frames[pos].floating;

	case FO_FLAGS:
		INT_PTR dwFlags = 0;
		if (Frames[pos].visible) dwFlags |= F_VISIBLE;
		if (!Frames[pos].collapsed) dwFlags |= F_UNCOLLAPSED;
		if (Frames[pos].Locked) dwFlags |= F_LOCKED;
		if (Frames[pos].TitleBar.ShowTitleBar) dwFlags |= F_SHOWTB;
		if (Frames[pos].TitleBar.ShowTitleBarTip) dwFlags |= F_SHOWTBTIP;
		if (Frames[pos].Skinned) dwFlags |= F_SKINNED;
		if (!(GetWindowLongPtr(Frames[pos].hWnd, GWL_STYLE)&WS_BORDER)) dwFlags |= F_NOBORDER;
		return dwFlags;
	}

	return -1;
}

INT_PTR CLUIFramesSetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	int retval; // value to be returned

	if (FramesSysNotStarted)
		return -1;

	mir_cslockfull lck(csFrameHook);
	int pos = id2pos(HIWORD(wParam));
	if (pos < 0 || pos >= nFramescount)
		return -1;

	switch (LOWORD(wParam) & ~FO_UNICODETEXT) {
	case FO_FLAGS:
		{
			int flag = lParam;
			LONG_PTR style;

			Frames[pos].dwFlags = flag;
			Frames[pos].visible = FALSE;
			if (flag & F_VISIBLE) Frames[pos].visible = TRUE;

			Frames[pos].collapsed = TRUE;
			if (flag & F_UNCOLLAPSED) Frames[pos].collapsed = FALSE;

			Frames[pos].Locked = FALSE;
			if (flag & F_LOCKED) Frames[pos].Locked = TRUE;

			Frames[pos].UseBorder = TRUE;
			if (flag & F_NOBORDER) Frames[pos].UseBorder = FALSE;

			Frames[pos].TitleBar.ShowTitleBar = FALSE;
			if (flag & F_SHOWTB) Frames[pos].TitleBar.ShowTitleBar = TRUE;

			Frames[pos].TitleBar.ShowTitleBarTip = FALSE;
			if (flag & F_SHOWTBTIP) Frames[pos].TitleBar.ShowTitleBarTip = TRUE;

			SendMessage(Frames[pos].TitleBar.hwndTip, TTM_ACTIVATE, (WPARAM)Frames[pos].TitleBar.ShowTitleBarTip, 0);

			style = GetWindowLongPtr(Frames[pos].hWnd, GWL_STYLE);
			style |= WS_BORDER;
			style |= CLS_SKINNEDFRAME;

			if (flag & F_NOBORDER)
				style &= (~WS_BORDER);

			Frames[pos].Skinned = FALSE;
			if (flag & F_SKINNED)
				Frames[pos].Skinned = TRUE;

			if ( !(flag & F_SKINNED))
				style &= ~CLS_SKINNEDFRAME;

			SetWindowLongPtr(Frames[pos].hWnd, GWL_STYLE, (LONG_PTR)style);
			SetWindowLongPtr(Frames[pos].TitleBar.hwnd, GWL_STYLE, (LONG_PTR)style & ~(WS_VSCROLL | WS_HSCROLL));
			lck.unlock();

			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
			SetWindowPos(Frames[pos].TitleBar.hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
		return 0;

	case FO_NAME:
		if (lParam == 0)
			return -1;

		mir_free(Frames[pos].name);
		Frames[pos].name = (wParam & FO_UNICODETEXT) ? mir_u2t((LPWSTR)lParam) : mir_a2t((LPSTR)lParam);
		return 0;

	case FO_TBNAME:
		if (lParam == 0)
			return -1;

		mir_free(Frames[pos].TitleBar.tbname);
		Frames[pos].TitleBar.tbname = (wParam & FO_UNICODETEXT) ? mir_u2t((LPWSTR)lParam) : mir_a2t((LPSTR)lParam);
		lck.unlock();

		if (Frames[pos].floating && (Frames[pos].TitleBar.tbname != NULL))
			SetWindowText(Frames[pos].ContainerWnd, Frames[pos].TitleBar.tbname);
		return 0;

	case FO_TBTIPNAME:
		if (lParam == 0)
			return -1;

		mir_free(Frames[pos].TitleBar.tooltip);
		Frames[pos].TitleBar.tooltip = (wParam & FO_UNICODETEXT) ? mir_u2t((LPWSTR)lParam) : mir_a2t((LPSTR)lParam);
		UpdateTBToolTip(pos);
		return 0;

	case FO_TBSTYLE:
		SetWindowLongPtr(Frames[pos].TitleBar.hwnd, GWL_STYLE, lParam);
		return 0;

	case FO_TBEXSTYLE:
		SetWindowLongPtr(Frames[pos].TitleBar.hwnd, GWL_EXSTYLE, lParam);
		return 0;

	case FO_ICON:
		Frames[pos].TitleBar.hicon = (HICON)lParam;
		return 0;

	case FO_HEIGHT:
		if (lParam < 0)
			return -1;

		if (Frames[pos].Skinned) {
			int uID = (Frames[pos].TitleBar.ShowTitleBar ? ID_EXTBKOWNEDFRAMEBORDERTB - ID_STATUS_OFFLINE : ID_EXTBKOWNEDFRAMEBORDER - ID_STATUS_OFFLINE);
			lParam += (arStatusItems[uID]->MARGIN_BOTTOM + arStatusItems[uID]->MARGIN_TOP);
		}
		if (Frames[pos].collapsed)	{
			int oldHeight = Frames[pos].height;
			retval = Frames[pos].height;
			Frames[pos].height = lParam;
			if ( !CLUIFramesFitInSize())
				Frames[pos].height = retval;
			retval = Frames[pos].height;

			if (Frames[pos].height != oldHeight) {
				CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
				if (Frames[pos].Skinned)
					RedrawWindow(Frames[pos].hWnd, 0, 0, RDW_FRAME|RDW_UPDATENOW|RDW_INVALIDATE);
			}
		}
		else {
			retval = Frames[pos].HeightWhenCollapsed;
			Frames[pos].HeightWhenCollapsed = lParam;
			if ( !CLUIFramesFitInSize())
				Frames[pos].HeightWhenCollapsed = retval;
			retval = Frames[pos].HeightWhenCollapsed;
		}
		return retval;

	case FO_FLOATING:
		if (lParam < 0)
			return -1;
		else {
			int id = Frames[pos].id;
			Frames[pos].floating = !(lParam);
			lck.unlock();

			CLUIFrameSetFloat(id, 1);//lparam=1 use stored width and height
		}
		return wParam;

	case FO_ALIGN:
		if ( !(lParam&alTop || lParam&alBottom || lParam&alClient))
			return -1;

		if ((lParam&alClient) && (CLUIFramesGetalClientFrame() >= 0)) {  //only one alClient frame possible
			alclientFrame = -1;//recalc it
			return -1;
		}
		Frames[pos].align = lParam;
		return 0;
	}
	lck.unlock();

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	return -1;
}

static INT_PTR CLUIFramesShowAll(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i = 0;i < nFramescount;i++)
		Frames[i].visible = TRUE;
	
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	return 0;
}

INT_PTR CLUIFramesShowAllTitleBars(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i = 0;i < nFramescount;i++) {
		Frames[i].TitleBar.ShowTitleBar = TRUE;
		SetWindowPos(Frames[i].hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

//wparam=lparam=0
INT_PTR CLUIFramesHideAllTitleBars(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i = 0;i < nFramescount;i++) {
		Frames[i].TitleBar.ShowTitleBar = FALSE;
		SetWindowPos(Frames[i].hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesShowHideFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	int pos;
	{
		mir_cslock lck(csFrameHook);
		pos = id2pos((INT_PTR)wParam);
		if (pos >= 0 && !lstrcmp(Frames[pos].name, _T("My contacts")))
			Frames[pos].visible = 1;
		else {
			if (pos >= 0 && (int)pos < nFramescount)
				Frames[pos].visible = !Frames[pos].visible;
			if (Frames[pos].floating)
				CLUIFrameResizeFloatingFrame(pos);
		}
	}
	
	if (!Frames[pos].floating)
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesShowHideFrameTitleBar(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	{
		mir_cslock lck(csFrameHook);
		int pos = id2pos((INT_PTR)wParam);
		if (pos >= 0 && (int)pos < nFramescount) {
			Frames[pos].TitleBar.ShowTitleBar = !Frames[pos].TitleBar.ShowTitleBar;
			SetWindowPos(Frames[pos].hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
	}

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}


//wparam=frameid
//lparam=-1 up ,1 down
INT_PTR CLUIFramesMoveUpDown(WPARAM wParam, LPARAM lParam)
{
	int i, tmpval;

	if (FramesSysNotStarted)
		return -1;

	mir_cslockfull lck(csFrameHook);
	int pos = id2pos((INT_PTR)wParam);
	if (pos < 0 || pos >= nFramescount)
		return 0;

	int curpos = Frames[pos].order;
	int curalign = Frames[pos].align;
	int v = 0;
	memset(g_sd, 0, sizeof(SortData) * MAX_FRAMES);
	for (i = 0;i < nFramescount;i++) {
		if (Frames[i].floating || (!Frames[i].visible) || (Frames[i].align != curalign))
			continue;
		g_sd[v].order = Frames[i].order;
		g_sd[v].realpos = i;
		v++;
	}
	if (v == 0)
		return 0;

	qsort(g_sd, v, sizeof(SortData), sortfunc);
	for (i = 0;i < v;i++) {
		if (g_sd[i].realpos == pos) {
			if (lParam == -1) {
				if (i < 1) break;
				tmpval = Frames[g_sd[i-1].realpos].order;
				Frames[g_sd[i-1].realpos].order = Frames[pos].order;
				Frames[pos].order = tmpval;
				break;
			}
			if (lParam == 1) {
				if (i > v - 1) break;
				tmpval = Frames[g_sd[i+1].realpos].order;
				Frames[g_sd[i+1].realpos].order = Frames[pos].order;
				Frames[pos].order = tmpval;
				break;
			}
		}
	}
	lck.unlock();

	CLUIFramesReSort();
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
	return 0;
}

static INT_PTR CLUIFramesMoveUp(WPARAM wParam, LPARAM lParam)
{
	return(CLUIFramesMoveUpDown(wParam, -1));
}

static INT_PTR CLUIFramesMoveDown(WPARAM wParam, LPARAM lParam)
{
	return(CLUIFramesMoveUpDown(wParam, 1));
}


//wparam=frameid
//lparam=alignment
INT_PTR CLUIFramesSetAlign(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;

	CLUIFramesSetFrameOptions(MAKEWPARAM(FO_ALIGN, wParam), lParam);
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}
INT_PTR CLUIFramesSetAlignalTop(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;

	return CLUIFramesSetAlign(wParam, alTop);
}
INT_PTR CLUIFramesSetAlignalBottom(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;

	return CLUIFramesSetAlign(wParam, alBottom);
}
INT_PTR CLUIFramesSetAlignalClient(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;

	return CLUIFramesSetAlign(wParam, alClient);
}


//wparam=frameid
INT_PTR CLUIFramesLockUnlockFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos((INT_PTR)wParam);
	if (pos >= 0 && (int)pos < nFramescount) {
		Frames[pos].Locked = !Frames[pos].Locked;
		CLUIFramesStoreFrameSettings(pos);
	}
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesSetUnSetBorder(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	HWND hw;
	int FrameId, oldflags;
	{
		mir_cslock lck(csFrameHook);
		FrameId = id2pos((INT_PTR)wParam);
		if (FrameId == -1)
			return -1;

		oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, wParam), 0);
		if (oldflags & F_NOBORDER)
			oldflags &= (~F_NOBORDER);
		else
			oldflags |= F_NOBORDER;

		hw = Frames[FrameId].hWnd;
	}

	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, wParam), oldflags);
	SetWindowPos(hw, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_NOZORDER);
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesSetUnSetSkinned(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	HWND hw;
	int FrameId, oldflags;
	{
		mir_cslock lck(csFrameHook);
		FrameId = id2pos((INT_PTR)wParam);
		if (FrameId == -1)
			return -1;

		oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, wParam), 0);
		if (oldflags & F_SKINNED)
			oldflags &= ~F_SKINNED;
		else
			oldflags |= F_SKINNED;

		hw = Frames[FrameId].hWnd;
	}

	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, wParam), oldflags);
	SetWindowPos(hw, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_NOZORDER);
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesCollapseUnCollapseFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	TitleBarH = cfg::dat.titleBarHeight;

	mir_cslockfull lck(csFrameHook);
	int FrameId = id2pos((INT_PTR)wParam);
	if (FrameId < 0 || FrameId >= nFramescount)
		return -1;
		
	int oldHeight;

	// do not collapse/uncollapse client/locked/invisible frames
	if (Frames[FrameId].align == alClient && !(Frames[FrameId].Locked || (!Frames[FrameId].visible) || Frames[FrameId].floating)) {
		RECT rc;
		if (CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0))
			return 0;

		if (cfg::getByte("CLUI", "AutoSize", 0))
			return 0;

		GetWindowRect(pcli->hwndContactList, &rc);

		if (Frames[FrameId].collapsed == TRUE) {
			rc.bottom -= rc.top;
			rc.bottom -= Frames[FrameId].height;
			Frames[FrameId].HeightWhenCollapsed = Frames[FrameId].height;
			Frames[FrameId].collapsed = FALSE;
		}
		else {
			rc.bottom -= rc.top;
			rc.bottom += Frames[FrameId].HeightWhenCollapsed;
			Frames[FrameId].collapsed = TRUE;
		}

		SetWindowPos(pcli->hwndContactList, NULL, 0, 0, rc.right - rc.left, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

		CLUIFramesStoreAllFrames();
		lck.unlock();
		RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		return 0;

	}
	if (Frames[FrameId].Locked || (!Frames[FrameId].visible))
		return 0;

	oldHeight = Frames[FrameId].height;

	// if collapsed, uncollapse
	if (Frames[FrameId].collapsed == TRUE) {
		Frames[FrameId].HeightWhenCollapsed = Frames[FrameId].height;
		Frames[FrameId].height = UNCOLLAPSED_FRAME_SIZE;
		Frames[FrameId].collapsed = FALSE;
	}
	// if uncollapsed, collapse
	else {
		Frames[FrameId].height = Frames[FrameId].HeightWhenCollapsed;
		Frames[FrameId].collapsed = TRUE;
	}

	if (!Frames[FrameId].floating) {

		if (!CLUIFramesFitInSize()) {
			//cant collapse,we can resize only for height<alclient frame height
			int alfrm = CLUIFramesGetalClientFrame();

			if (alfrm != -1) {
				Frames[FrameId].collapsed = FALSE;
				if (Frames[alfrm].height > 2 * UNCOLLAPSED_FRAME_SIZE) {
					oldHeight = Frames[alfrm].height - UNCOLLAPSED_FRAME_SIZE;
					Frames[FrameId].collapsed = TRUE;
				}
			}
			else {
				int i, sumheight = 0;

				for (i = 0; i < nFramescount; i++) {
					if ((Frames[i].align != alClient) && (!Frames[i].floating) && (Frames[i].visible) && (!Frames[i].needhide)) {
						sumheight += (Frames[i].height) + (TitleBarH * btoint(Frames[i].TitleBar.ShowTitleBar)) + 2;
						return FALSE;
					}
					if (sumheight > ContactListHeight - 0 - 2)
						Frames[FrameId].height = (ContactListHeight - 0 - 2) - sumheight;
				}
			}
			Frames[FrameId].height = oldHeight;
			if (Frames[FrameId].collapsed == FALSE) {
				if (Frames[FrameId].floating)
					SetWindowPos(Frames[FrameId].ContainerWnd, HWND_TOP, 0, 0, Frames[FrameId].wndSize.right - Frames[FrameId].wndSize.left + 6, Frames[FrameId].height + DEFAULT_TITLEBAR_HEIGHT + 4, SWP_SHOWWINDOW | SWP_NOMOVE);
				return -1;
			}
		}
	}
	lck.unlock();
	if (!Frames[FrameId].floating)
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	else {
		RECT contwnd;
		GetWindowRect(Frames[FrameId].ContainerWnd, &contwnd);
		contwnd.top = contwnd.bottom - contwnd.top;//height
		contwnd.left = contwnd.right - contwnd.left;//width

		contwnd.top -= (oldHeight - Frames[FrameId].height);//newheight
		SetWindowPos(Frames[FrameId].ContainerWnd, HWND_TOP, 0, 0, contwnd.left, contwnd.top, SWP_SHOWWINDOW | SWP_NOMOVE);
	}
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	CLUIFramesStoreAllFrames();
	return 0;
}

static int CLUIFramesLoadMainMenu()
{
	if (FramesSysNotStarted)
		return -1;

	if (MainMIRoot != (HANDLE)-1) {
		CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)MainMIRoot, 0);
		MainMIRoot = (HGENMENU)-1;
	}

	// create root menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_FRAME);
	mi.flags = CMIF_ROOTHANDLE;
	mi.position = 3000090000;
	mi.pszPopupName = (char*)-1;
	mi.pszName = LPGEN("Frames");
	mi.pszService = 0;
	MainMIRoot = Menu_AddMainMenuItem(&mi);

	// create frames menu
	int separator = (int)3000200000;
	for (int i = 0; i < nFramescount; i++) {
		mi.hIcon = Frames[i].TitleBar.hicon;
		mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
		mi.position = separator;
		mi.hParentMenu = MainMIRoot;
		mi.ptszName = Frames[i].TitleBar.tbname ? Frames[i].TitleBar.tbname : Frames[i].name;
		mi.pszService = 0;
		Frames[i].MenuHandles.MainMenuItem = Menu_AddMainMenuItem(&mi);
		CLUIFramesCreateMenuForFrame(Frames[i].id, (int)Frames[i].MenuHandles.MainMenuItem, separator, Menu_AddMainMenuItem);
		CLUIFramesModifyMainMenuItems(Frames[i].id, 0);
		//NotifyEventHooks(hPreBuildFrameMenuEvent,i,(LPARAM)Frames[i].MenuHandles.MainMenuItem);
		CallService(MS_CLIST_FRAMEMENUNOTIFY, (WPARAM)Frames[i].id, (LPARAM)Frames[i].MenuHandles.MainMenuItem);
		separator++;
	}

	separator += 100000;

	// create "show all frames" menu
	mi.hIcon = NULL;//LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_MIRANDA));
	mi.flags = CMIF_ROOTHANDLE;
	mi.position = separator++;
	mi.hParentMenu = MainMIRoot;
	mi.pszName = LPGEN("Show all frames");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMES;
	Menu_AddMainMenuItem(&mi);

	// create "show all titlebars" menu
	mi.hIcon = NULL;//LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_HELP));
	mi.position = separator++;
	mi.hParentMenu = MainMIRoot;
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszName = LPGEN("Show all title bars");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);

	// create "hide all titlebars" menu
	mi.hIcon = NULL;//LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_HELP));
	mi.position = separator++;
	mi.hParentMenu = MainMIRoot;
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszName = LPGEN("Hide all title bars");
	mi.pszService = MS_CLIST_FRAMES_HIDEALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);

	return 0;
}

static HFONT CLUILoadTitleBarFont()
{
	char facename[] = "MS Shell Dlg";
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	memcpy(logfont.lfFaceName, facename, sizeof(facename));
	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = -10;
	return CreateFontIndirect(&logfont);
}

static int UpdateTBToolTip(int framepos)
{
	TOOLINFO ti;

	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.lpszText = Frames[framepos].TitleBar.tooltip;
	ti.hinst = g_hInst;
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId = (UINT_PTR)Frames[framepos].TitleBar.hwnd;

	return SendMessage(Frames[framepos].TitleBar.hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
};

int FrameNCPaint(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar)
{
	HDC hdc;
	RECT rcWindow, rc;
	HWND hwndParent = GetParent(hwnd);
	LRESULT result;

	if (hwndParent != pcli->hwndContactList || !cfg::dat.bSkinnedScrollbar)
		result = CallWindowProc(oldWndProc, hwnd, WM_NCPAINT, wParam, lParam);
	else
		result = 0;

	if (pcli && pcli->hwndContactList && GetParent(hwnd) == pcli->hwndContactList) {
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SKINNEDFRAME) {
			StatusItems_t *item = (arStatusItems.getCount() != 0) ? (hasTitleBar ? arStatusItems[ID_EXTBKOWNEDFRAMEBORDERTB - ID_STATUS_OFFLINE] : arStatusItems[ID_EXTBKOWNEDFRAMEBORDER - ID_STATUS_OFFLINE]) : 0;
			if (item == 0)
				return 0;

			GetWindowRect(hwnd, &rcWindow);
			rc.left = rc.top = 0;
			rc.right = rcWindow.right - rcWindow.left;
			rc.bottom = rcWindow.bottom - rcWindow.top;

			HBITMAP hbmDraw, hbmOld;
			HDC realDC = hdc = GetWindowDC(hwnd);
			if (hwnd == pcli->hwndContactTree) {
				realDC = CreateCompatibleDC(hdc);
				hbmDraw = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				hbmOld = reinterpret_cast<HBITMAP>(SelectObject(realDC, hbmDraw));
			}

			ExcludeClipRect(realDC, item->MARGIN_LEFT, item->MARGIN_TOP, rc.right - item->MARGIN_RIGHT, rc.bottom - item->MARGIN_BOTTOM);

			BitBlt(realDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, cfg::dat.hdcBg, rcWindow.left - cfg::dat.ptW.x, rcWindow.top - cfg::dat.ptW.y, SRCCOPY);

			DrawAlpha(realDC, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);

			if (hwnd == pcli->hwndContactTree) {
				ExcludeClipRect(hdc, item->MARGIN_LEFT, item->MARGIN_TOP, rc.right - item->MARGIN_RIGHT, rc.bottom - item->MARGIN_BOTTOM);
				BitBlt(hdc, 0, 0, rc.right, rc.bottom, realDC, 0, 0, SRCCOPY);
				SelectObject(realDC, hbmOld);
				DeleteObject(hbmDraw);
				DeleteDC(realDC);
			}
			ReleaseDC(hwnd, hdc);
			return 0;
		}

		if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_BORDER) {
			hdc = GetWindowDC(hwnd);
			HPEN hPenOld = reinterpret_cast<HPEN>(SelectObject(hdc, g_hPenCLUIFrames));
			GetWindowRect(hwnd, &rcWindow);
			rc.left = rc.top = 0;
			rc.right = rcWindow.right - rcWindow.left;
			rc.bottom = rcWindow.bottom - rcWindow.top;
			HBRUSH brold = reinterpret_cast<HBRUSH>(SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)));
			Rectangle(hdc, 0, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top);
			SelectObject(hdc, hPenOld);
			SelectObject(hdc, brold);
			ReleaseDC(hwnd, hdc);
			return 0;
		}
	}
	return result;
}

int FrameNCCalcSize(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar)
{
	StatusItems_t *item = (arStatusItems.getCount() != 0) ? (hasTitleBar ? arStatusItems[ID_EXTBKOWNEDFRAMEBORDERTB - ID_STATUS_OFFLINE] : arStatusItems[ID_EXTBKOWNEDFRAMEBORDER - ID_STATUS_OFFLINE]) : 0;
	LRESULT orig = oldWndProc ? CallWindowProc(oldWndProc, hwnd, WM_NCCALCSIZE, wParam, lParam) : 0;
	NCCALCSIZE_PARAMS *nccp = (NCCALCSIZE_PARAMS *)lParam;
	DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

	if (item == 0 || pcli == 0)
		return orig;

	if (item->IGNORED || !(dwStyle & CLS_SKINNEDFRAME) || GetParent(hwnd) != pcli->hwndContactList)
		return orig;

	nccp->rgrc[0].left += item->MARGIN_LEFT;
	nccp->rgrc[0].right -= item->MARGIN_RIGHT;
	nccp->rgrc[0].bottom -= item->MARGIN_BOTTOM;
	nccp->rgrc[0].top += item->MARGIN_TOP;
	return WVR_REDRAW;
}

static LRESULT CALLBACK FramesSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;

	WNDPROC oldWndProc = 0;
	BOOL    hasTitleBar = FALSE;

	for (i = 0; i < nFramescount; i++) {
		if (Frames[i].hWnd == hwnd) {
			oldWndProc = Frames[i].wndProc;
			hasTitleBar = Frames[i].TitleBar.ShowTitleBar;
		}
	}
	switch (msg) {
	case WM_NCPAINT:
		return FrameNCPaint(hwnd, oldWndProc ? oldWndProc : DefWindowProc, wParam, lParam, hasTitleBar);

	case WM_NCCALCSIZE:
		return FrameNCCalcSize(hwnd, oldWndProc, wParam, lParam, hasTitleBar);

	case WM_PRINTCLIENT:
		return 0;
	}

	if (oldWndProc)
		return CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*
 * re-sort all frames and correct frame ordering
 */

static int CLUIFramesReSort()
{
	int v = 0, i;
	int order = 1;

	mir_cslock lck(csFrameHook);
	memset(g_sd, 0, sizeof(SortData) * MAX_FRAMES);
	for (i = 0; i < nFramescount; i++) {
		if (Frames[i].align != alTop)
			continue;
		g_sd[v].order = Frames[i].order;
		g_sd[v].realpos = i;
		v++;
	}
	if (v > 0) {
		qsort(g_sd, v, sizeof(SortData), sortfunc);
		for (i = 0; i < v; i++)
			Frames[g_sd[i].realpos].order = order++;
	}

	memset(g_sd, 0, sizeof(SortData) * MAX_FRAMES);
	v = 0;
	for (i = 0; i < nFramescount; i++) {
		if (Frames[i].align != alBottom)
			continue;
		g_sd[v].order = Frames[i].order;
		g_sd[v].realpos = i;
		v++;
	}
	if (v > 0) {
		qsort(g_sd, v, sizeof(SortData), sortfunc);
		for (i = 0; i < v; i++)
			Frames[g_sd[i].realpos].order = order++;
	}
	CLUIFramesStoreAllFrames();
	return 0;
}

//wparam=(CLISTFrame*)clfrm
INT_PTR CLUIFramesAddFrame(WPARAM wParam, LPARAM lParam)
{
	int style;
	char * CustomName = NULL;
	CLISTFrame *clfrm = (CLISTFrame *)wParam;

	if (pcli->hwndContactList == 0) return -1;
	if (FramesSysNotStarted) return -1;
	if (clfrm->cbSize != sizeof(CLISTFrame)) return -1;

	mir_cslockfull lck(csFrameHook);
	if (nFramescount >= MAX_FRAMES)
		return -1;

	if (Frames == NULL) {
		Frames = (FRAMEWND*)malloc(sizeof(FRAMEWND) * (MAX_FRAMES + 2));
		ZeroMemory(Frames, sizeof(FRAMEWND) * (MAX_FRAMES + 2));
	}
	memset(&Frames[nFramescount], 0, sizeof(FRAMEWND));

	Frames[nFramescount].id = NextFrameId++;
	Frames[nFramescount].align = clfrm->align;
	Frames[nFramescount].hWnd = clfrm->hWnd;
	Frames[nFramescount].height = clfrm->height;
	Frames[nFramescount].TitleBar.hicon = clfrm->hIcon;
	//Frames[nFramescount].TitleBar.BackColour;
	Frames[nFramescount].floating = FALSE;

	if (clfrm->Flags & F_NO_SUBCONTAINER)
		Frames[nFramescount].OwnerWindow = (HWND)-2;
	else
		Frames[nFramescount].OwnerWindow = pcli->hwndContactList;

	SetClassLong(clfrm->hWnd, GCL_STYLE, GetClassLong(clfrm->hWnd, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));
	SetWindowLongPtr(clfrm->hWnd, GWL_STYLE, GetWindowLongPtr(clfrm->hWnd, GWL_STYLE) | WS_CLIPCHILDREN);

	if (GetCurrentThreadId() == GetWindowThreadProcessId(clfrm->hWnd, NULL)) {
		if (clfrm->hWnd != pcli->hwndContactTree && clfrm->hWnd != g_hwndViewModeFrame && clfrm->hWnd != g_hwndEventArea) {
			Frames[nFramescount].wndProc = (WNDPROC)GetWindowLongPtr(clfrm->hWnd, GWLP_WNDPROC);
			SetWindowLongPtr(clfrm->hWnd, GWLP_WNDPROC, (LONG_PTR)FramesSubClassProc);
		}
	}

	if (clfrm->hWnd == g_hwndEventArea)
		wndFrameEventArea = &Frames[nFramescount];
	else if (clfrm->hWnd == pcli->hwndContactTree)
		wndFrameCLC = &Frames[nFramescount];
	else if (clfrm->hWnd == g_hwndViewModeFrame)
		wndFrameViewMode = &Frames[nFramescount];

	Frames[nFramescount].dwFlags = clfrm->Flags;

	if (clfrm->name == NULL || ((clfrm->Flags&F_UNICODE) ? lstrlenW(clfrm->wname) : lstrlenA(clfrm->name)) == 0) {
		TCHAR ptszClassName[256];
		GetClassName(Frames[nFramescount].hWnd, ptszClassName, SIZEOF(ptszClassName));
		Frames[nFramescount].name = mir_tstrdup(ptszClassName);
	}
	else Frames[nFramescount].name = (clfrm->Flags & F_UNICODE) ? mir_u2t(clfrm->wname) : mir_a2t(clfrm->name);

	if (IsBadCodePtr((FARPROC)clfrm->TBname) || clfrm->TBname == NULL
		 || ((clfrm->Flags&F_UNICODE) ? lstrlenW(clfrm->TBwname) : lstrlenA(clfrm->TBname)) == 0)
		 Frames[nFramescount].TitleBar.tbname = mir_tstrdup(Frames[nFramescount].name);
	else
		Frames[nFramescount].TitleBar.tbname = (clfrm->Flags & F_UNICODE) ? mir_u2t(clfrm->TBwname) : mir_a2t(clfrm->TBname);
	Frames[nFramescount].needhide = FALSE;
	Frames[nFramescount].TitleBar.ShowTitleBar = (clfrm->Flags & F_SHOWTB ? TRUE : FALSE);
	Frames[nFramescount].TitleBar.ShowTitleBarTip = (clfrm->Flags & F_SHOWTBTIP ? TRUE : FALSE);

	Frames[nFramescount].collapsed = clfrm->Flags & F_UNCOLLAPSED ? FALSE : TRUE;
	Frames[nFramescount].Locked = clfrm->Flags & F_LOCKED ? TRUE : FALSE;
	Frames[nFramescount].visible = clfrm->Flags & F_VISIBLE ? TRUE : FALSE;

	Frames[nFramescount].UseBorder = (clfrm->Flags & F_NOBORDER) ? FALSE : TRUE;
	Frames[nFramescount].Skinned = (clfrm->Flags & F_SKINNED) ? TRUE : FALSE;

	// create frame
	Frames[nFramescount].TitleBar.hwnd =
		CreateWindow(CLUIFrameTitleBarClassName, Frames[nFramescount].name,
		(cfg::getByte(CLUIFrameModule, "RemoveAllTitleBarBorders", 1) ? 0 : WS_BORDER)
		| WS_CHILD | WS_CLIPCHILDREN | (Frames[nFramescount].TitleBar.ShowTitleBar ? WS_VISIBLE : 0) |
		WS_CLIPCHILDREN, 0, 0, 0, 0, pcli->hwndContactList, NULL, g_hInst, NULL);

	SetWindowLongPtr(Frames[nFramescount].TitleBar.hwnd, GWLP_USERDATA, Frames[nFramescount].id);

	Frames[nFramescount].TitleBar.hwndTip = CreateWindowExA(0, TOOLTIPS_CLASSA, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
																			  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
																			  pcli->hwndContactList, NULL, g_hInst, NULL);

	SetWindowPos(Frames[nFramescount].TitleBar.hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	{
		TOOLINFOA ti = { 0 };
		ti.cbSize = sizeof(ti);
		ti.lpszText = "";
		ti.hinst = g_hInst;
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.uId = (UINT_PTR)Frames[nFramescount].TitleBar.hwnd;
		SendMessageA(Frames[nFramescount].TitleBar.hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

	SendMessage(Frames[nFramescount].TitleBar.hwndTip, TTM_ACTIVATE, (WPARAM)Frames[nFramescount].TitleBar.ShowTitleBarTip, 0);

	Frames[nFramescount].oldstyles = GetWindowLongPtr(Frames[nFramescount].hWnd, GWL_STYLE);
	Frames[nFramescount].TitleBar.oldstyles = GetWindowLongPtr(Frames[nFramescount].TitleBar.hwnd, GWL_STYLE);

	int retval = Frames[nFramescount].id;
	Frames[nFramescount].order = nFramescount + 1;
	nFramescount++;

	CLUIFramesLoadFrameSettings(id2pos(retval));
	style = GetWindowLongPtr(Frames[nFramescount - 1].hWnd, GWL_STYLE);
	style &= ~(WS_BORDER);
	style |= ((Frames[nFramescount - 1].UseBorder) ? WS_BORDER : 0);

	style |= Frames[nFramescount - 1].Skinned ? CLS_SKINNEDFRAME : 0;

	SetWindowLongPtr(Frames[nFramescount - 1].hWnd, GWL_STYLE, style);
	SetWindowLongPtr(Frames[nFramescount - 1].TitleBar.hwnd, GWL_STYLE, style & ~(WS_VSCROLL | WS_HSCROLL));

	if (Frames[nFramescount - 1].order == 0)
		Frames[nFramescount - 1].order = nFramescount;

	lck.unlock();

	alclientFrame = -1;//recalc it
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);

	if (Frames[nFramescount - 1].floating) {
		Frames[nFramescount - 1].floating = FALSE;
		CLUIFrameSetFloat(retval, 1);//lparam=1 use stored width and height
	}
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return retval;
}

static INT_PTR CLUIFramesRemoveFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	{
		mir_cslock lck(csFrameHook);
		int pos = id2pos((INT_PTR)wParam);
		if (pos < 0 || pos > nFramescount)
			return -1;

		FRAMEWND* F = &Frames[pos];
		if (F->hWnd == g_hwndEventArea)
			wndFrameEventArea = NULL;
		else if (F->hWnd == pcli->hwndContactTree)
			wndFrameCLC = NULL;
		else if (F->hWnd == g_hwndViewModeFrame)
			wndFrameViewMode = NULL;

		mir_free(F->name);
		mir_free(F->TitleBar.tbname);
		mir_free(F->TitleBar.tooltip);

		DestroyWindow(F->hWnd);
		F->hWnd = (HWND)-1;
		DestroyWindow(F->TitleBar.hwnd);
		F->TitleBar.hwnd = (HWND)-1;
		DestroyWindow(F->ContainerWnd);
		F->ContainerWnd = (HWND)-1;
		DestroyMenu(F->TitleBar.hmenu);

		RemoveItemFromList(pos, &Frames, &nFramescount);
	}

	if (!cfg::shutDown) {
		InvalidateRect(pcli->hwndContactList, NULL, TRUE);
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
		RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	return 0;
}

INT_PTR CLUIFramesForceUpdateTB(const FRAMEWND *Frame)
{
	if (Frame->TitleBar.hwnd != 0)
		RedrawWindow(Frame->TitleBar.hwnd, NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	return 0;
}

INT_PTR CLUIFramesForceUpdateFrame(const FRAMEWND *Frame)
{
	if (Frame->hWnd != 0)
		RedrawWindow(Frame->hWnd, NULL, NULL, RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_INVALIDATE);

	if (Frame->floating)
		if (Frame->ContainerWnd != 0)  RedrawWindow(Frame->ContainerWnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	return 0;
}

int CLUIFrameMoveResize(const FRAMEWND *Frame)
{
	TitleBarH = cfg::dat.titleBarHeight;
	// we need to show or hide the frame?
	if (Frame->visible && (!Frame->needhide)) {
		ShowWindow(Frame->hWnd, SW_SHOW);
		ShowWindow(Frame->TitleBar.hwnd, Frame->TitleBar.ShowTitleBar == TRUE ? SW_SHOW : SW_HIDE);
	}
	else {
		ShowWindow(Frame->hWnd, SW_HIDE);
		ShowWindow(Frame->TitleBar.hwnd, SW_HIDE);
		return 0;
	}

	SetWindowPos(Frame->hWnd, NULL, Frame->wndSize.left + cfg::dat.bCLeft, Frame->wndSize.top + cfg::dat.topOffset,
					 (Frame->wndSize.right - Frame->wndSize.left),
					 (Frame->wndSize.bottom - Frame->wndSize.top), SWP_NOZORDER | SWP_NOREDRAW);
	if (Frame->TitleBar.ShowTitleBar) {
		SetWindowPos(Frame->TitleBar.hwnd, NULL, Frame->wndSize.left + cfg::dat.bCLeft, Frame->wndSize.top + cfg::dat.topOffset - TitleBarH,
						 (Frame->wndSize.right - Frame->wndSize.left),
						 TitleBarH + (Frame->UseBorder ? (!Frame->collapsed ? (Frame->align == alClient ? 0 : 2) : 1) : 0), SWP_NOZORDER);
	}
	return 0;
}

BOOLEAN CLUIFramesFitInSize(void)
{
	int i;
	int sumheight = 0;
	int tbh = 0; // title bar height
	int clientfrm;

	TitleBarH = cfg::dat.titleBarHeight;

	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = TitleBarH * btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

	for (i = 0; i < nFramescount; i++) {
		if ((Frames[i].align != alClient) && (!Frames[i].floating) && (Frames[i].visible) && (!Frames[i].needhide)) {
			sumheight += (Frames[i].height) + (TitleBarH * btoint(Frames[i].TitleBar.ShowTitleBar)) + 2/*+btoint(Frames[i].UseBorder)*2*/;
			if (sumheight > ContactListHeight - tbh - 2)
				return FALSE;
		}
	}
	return TRUE;
}

int CLUIFramesGetMinHeight()
{
	if (pcli->hwndContactList == NULL)
		return 0;

	int i, tbh, clientfrm, sumheight = 0;
	RECT border;
	int allbord = 0;
	{
		mir_cslock lck(csFrameHook);

		TitleBarH = cfg::dat.titleBarHeight;
		// search for alClient frame and get the titlebar's height
		tbh = 0;
		clientfrm = CLUIFramesGetalClientFrame();
		if (clientfrm != -1)
			tbh = TitleBarH * btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

		for (i = 0; i < nFramescount; i++) {
			if ((Frames[i].align != alClient) && (Frames[i].visible) && (!Frames[i].needhide) && (!Frames[i].floating)) {
				RECT wsize;

				GetWindowRect(Frames[i].hWnd, &wsize);
				sumheight += (wsize.bottom - wsize.top) + (TitleBarH * btoint(Frames[i].TitleBar.ShowTitleBar)) + 3;
			}
		}
	}

	GetBorderSize(pcli->hwndContactList, &border);
	return(sumheight + border.top + border.bottom + allbord + tbh + 3);
}

int SizeMoveNewSizes()
{
	for (int i = 0; i < nFramescount; i++) {
		if (Frames[i].floating)
			CLUIFrameResizeFloatingFrame(i);
		else
			CLUIFrameMoveResize(&Frames[i]);
	}
	return 0;
}

/*
 * changed Nightwish
 * gap calculation was broken. Now, it doesn't calculate and store the gaps in Frames[] anymore.
 * instead, it remembers the smallest wndSize.top value (which has to be the top frame) and then passes
 * the gap to all following frame(s) to the actual resizing function which just adds the gap to
 * wndSize.top and corrects the frame height accordingly.

 * Title bar gap has been removed (can be simulated by using a clist_nicer skin item for frame title bars
 * and setting the bottom margin of the skin item
 */

int CLUIFramesResize(const RECT newsize)
{
	int sumheight = 9999999, newheight;
	int prevframe, prevframebottomline;
	int tbh, curfrmtbh;
	int drawitems;
	int clientfrm, clientframe = -1;
	int i, j;
	int sepw;
	int topOff = 0, botOff = 0, last_bottomtop;

	GapBetweenFrames = cfg::dat.gapBetweenFrames;
	sepw = GapBetweenFrames;

	if (nFramescount < 1 || cfg::shutDown)
		return 0;

	newheight = newsize.bottom - newsize.top;
	TitleBarH = cfg::dat.titleBarHeight;

	// search for alClient frame and get the titlebar's height
	tbh = 0;
	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = (TitleBarH)* btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

	for (i = 0; i < nFramescount; i++) {
		if (!Frames[i].floating) {
			Frames[i].needhide = FALSE;
			Frames[i].wndSize.left = 0;
			Frames[i].wndSize.right = newsize.right - newsize.left;
		}
	}
	{
		//sorting stuff
		memset(g_sd, 0, sizeof(SortData) * MAX_FRAMES);
		for (i = 0; i < nFramescount; i++) {
			g_sd[i].order = Frames[i].order;
			g_sd[i].realpos = i;
		}
		qsort(g_sd, nFramescount, sizeof(SortData), sortfunc);

	}
	drawitems = nFramescount;
	while (sumheight >(newheight - tbh) && drawitems > 0) {
		sumheight = 0;
		drawitems = 0;
		for (i = 0; i < nFramescount; i++) {
			if (((Frames[i].align != alClient)) && (!Frames[i].floating) && (Frames[i].visible) && (!Frames[i].needhide)) {
				drawitems++;
				curfrmtbh = (TitleBarH)* btoint(Frames[i].TitleBar.ShowTitleBar);
				sumheight += (Frames[i].height) + curfrmtbh + (i > 0 ? sepw : 0) + (Frames[i].UseBorder ? 2 : 0);
				if (sumheight > newheight - tbh) {
					sumheight -= (Frames[i].height) + curfrmtbh + (i > 0 ? sepw : 0);
					Frames[i].needhide = TRUE;
					drawitems--;
					break;
				}
			}
		}
	}

	prevframe = -1;
	prevframebottomline = 0;
	for (j = 0; j < nFramescount; j++) {
		//move all alTop frames
		i = g_sd[j].realpos;
		if ((!Frames[i].needhide) && (!Frames[i].floating) && (Frames[i].visible) && (Frames[i].align == alTop)) {
			curfrmtbh = (TitleBarH)* btoint(Frames[i].TitleBar.ShowTitleBar);
			Frames[i].wndSize.top = prevframebottomline + (prevframebottomline > 0 ? sepw : 0) + (curfrmtbh);
			Frames[i].wndSize.bottom = Frames[i].height + Frames[i].wndSize.top + (Frames[i].UseBorder ? 2 : 0);
			Frames[i].prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = Frames[i].wndSize.bottom;
			topOff = prevframebottomline;
		}
	}

	if (sumheight < newheight) {
		for (j = 0; j < nFramescount; j++) {
			//move alClient frame
			i = g_sd[j].realpos;
			if ((!Frames[i].needhide) && (!Frames[i].floating) && (Frames[i].visible) && (Frames[i].align == alClient)) {
				int oldh;
				Frames[i].wndSize.top = prevframebottomline + (prevframebottomline > 0 ? sepw : 0) + (tbh);
				Frames[i].wndSize.bottom = Frames[i].wndSize.top + newheight - sumheight - tbh - ((prevframebottomline > 0) ? sepw : 0);
				clientframe = i;
				oldh = Frames[i].height;
				Frames[i].height = Frames[i].wndSize.bottom - Frames[i].wndSize.top;
				Frames[i].prevvisframe = prevframe;
				prevframe = i;
				prevframebottomline = Frames[i].wndSize.bottom;
				if (prevframebottomline > newheight) {
					//prevframebottomline-=Frames[i].height+(tbh+1);
					//Frames[i].needhide=TRUE;
				}
				break;
			}
		}
	}

	//newheight
	prevframebottomline = last_bottomtop = newheight;
	//prevframe=-1;
	for (j = nFramescount - 1; j >= 0; j--) {
		//move all alBottom frames
		i = g_sd[j].realpos;
		if ((Frames[i].visible) && (!Frames[i].floating) && (!Frames[i].needhide) && (Frames[i].align == alBottom)) {
			curfrmtbh = (TitleBarH)* btoint(Frames[i].TitleBar.ShowTitleBar);
			Frames[i].wndSize.bottom = prevframebottomline - ((prevframebottomline < newheight) ? sepw : 0);
			Frames[i].wndSize.top = Frames[i].wndSize.bottom - Frames[i].height - (Frames[i].UseBorder ? 2 : 0);
			Frames[i].prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = Frames[i].wndSize.top - curfrmtbh;
			botOff = prevframebottomline;
			last_bottomtop = Frames[i].wndSize.top - curfrmtbh;
		}
	}

	// correct client frame bottom gap if there is no other top frame.
	if (clientframe != -1) {
		Frames[clientframe].wndSize.bottom = last_bottomtop - (last_bottomtop < newheight ? sepw : 0);
		Frames[clientframe].height = Frames[clientframe].wndSize.bottom - Frames[clientframe].wndSize.top;
	}
	return 0;
}

INT_PTR CLUIFramesUpdateFrame(WPARAM wParam, LPARAM lParam)
{
	int pos;
	if (FramesSysNotStarted)
		return -1;
	
	if (wParam == -1) {
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
		return 0;
	}
	
	if (lParam & FU_FMPOS)
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 1);

	mir_cslock lck(csFrameHook);
	pos = id2pos((INT_PTR)wParam);
	if (pos < 0 || pos >= nFramescount)
		return -1;

	if (lParam & FU_TBREDRAW)
		CLUIFramesForceUpdateTB(&Frames[pos]);
	if (lParam & FU_FMREDRAW)
		CLUIFramesForceUpdateFrame(&Frames[pos]);
	return 0;
}

int dock_prevent_moving = 0;

int CLUIFramesApplyNewSizes(int mode)
{
	dock_prevent_moving = 0;

	for (int i = 0; i < nFramescount; i++) {
		if ((mode == 1 && Frames[i].OwnerWindow != (HWND)-2 && Frames[i].OwnerWindow) ||
			 (mode == 2 && Frames[i].OwnerWindow == (HWND)-2) || (mode == 3))
			if (Frames[i].floating)
				CLUIFrameResizeFloatingFrame(i);
			else
				CLUIFrameMoveResize(&Frames[i]);
	}
	dock_prevent_moving = 1;
	return 0;
}

RECT old_window_rect = { 0 }, new_window_rect = { 0 };

int SizeFramesByWindowRect(RECT *r)
{
	RECT nRect;
	DWORD noSize = 0;

	if (FramesSysNotStarted)
		return -1;

	TitleBarH = cfg::dat.titleBarHeight;

	mir_cslock lck(csFrameHook);
	GapBetweenFrames = cfg::dat.gapBetweenFrames;

	nRect = *r;

	nRect.bottom -= (cfg::dat.statusBarHeight + cfg::dat.bottomOffset);
	nRect.right -= cfg::dat.bCRight;
	nRect.left = cfg::dat.bCLeft;
	nRect.top = cfg::dat.topOffset;
	ContactListHeight = nRect.bottom - nRect.top;

	CLUIFramesResize(nRect);
	{
		int i;
		for (i = 0; i < nFramescount; i++) {
			int dx;
			int dy;
			dx = new_window_rect.left - old_window_rect.left;
			dy = new_window_rect.top - old_window_rect.top;
			if (!Frames[i].floating) {
				if (Frames[i].OwnerWindow && Frames[i].OwnerWindow != (HWND)-2) {
					/*
					if (Frames[i].wndSize.right - Frames[i].wndSize.left == Frames[i].oldWndSize.right - Frames[i].oldWndSize.left &&
					Frames[i].wndSize.bottom - Frames[i].wndSize.top == Frames[i].oldWndSize.bottom - Frames[i].oldWndSize.top)
					noSize = SWP_NOSIZE;
					else {
					noSize = 0;
					CopyRect(&Frames[i].oldWndSize, &Frames[i].wndSize);
					}*/
					SetWindowPos(Frames[i].hWnd, NULL, Frames[i].wndSize.left + cfg::dat.bCLeft, Frames[i].wndSize.top + cfg::dat.topOffset,
									 (Frames[i].wndSize.right - Frames[i].wndSize.left),
									 (Frames[i].wndSize.bottom - Frames[i].wndSize.top), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS | noSize);

					if (Frames[i].TitleBar.ShowTitleBar) {
						SetWindowPos(Frames[i].TitleBar.hwnd, NULL, Frames[i].wndSize.left + cfg::dat.bCLeft, Frames[i].wndSize.top + cfg::dat.topOffset - TitleBarH,
										 (Frames[i].wndSize.right - Frames[i].wndSize.left),
										 TitleBarH + (Frames[i].UseBorder ? (!Frames[i].collapsed ? (Frames[i].align == alClient ? 0 : 2) : 1) : 0), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
					}
				}
				else {
					int res = 0;
					// set frame position
					SetWindowPos(Frames[i].hWnd, NULL, Frames[i].wndSize.left + cfg::dat.bCLeft, Frames[i].wndSize.top + cfg::dat.topOffset,
									 (Frames[i].wndSize.right - Frames[i].wndSize.left),
									 (Frames[i].wndSize.bottom - Frames[i].wndSize.top), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOCOPYBITS | SWP_NOREDRAW);

					// set titlebar position
					if (Frames[i].TitleBar.ShowTitleBar) {
						SetWindowPos(Frames[i].TitleBar.hwnd, NULL, Frames[i].wndSize.left + cfg::dat.bCLeft, Frames[i].wndSize.top + cfg::dat.topOffset - TitleBarH,
										 (Frames[i].wndSize.right - Frames[i].wndSize.left),
										 TitleBarH + (Frames[i].UseBorder ? (!Frames[i].collapsed ? (Frames[i].align == alClient ? 0 : 2) : 1) : 0), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
					}
					//UpdateWindow(Frames[i].hWnd);
					if (Frames[i].TitleBar.ShowTitleBar)
						UpdateWindow(Frames[i].TitleBar.hwnd);
				};
			}

		}
		if (GetTickCount() - LastStoreTick > 1000) {
			CLUIFramesStoreAllFrames();
			LastStoreTick = GetTickCount();
		}
	}
	return 0;
}

int CLUIFramesOnClistResize(WPARAM wParam, LPARAM lParam)
{
	GapBetweenFrames = cfg::dat.gapBetweenFrames;

	if (FramesSysNotStarted || cfg::shutDown)
		return -1;

	RECT nRect, rcStatus;
	int tick;
	{
		mir_cslock lck(csFrameHook);

		GetClientRect(pcli->hwndContactList, &nRect);
		if (lParam && lParam != 1) {
			RECT oldRect;
			POINT pt;
			RECT * newRect = (RECT *)lParam;
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

		rcStatus.top = rcStatus.bottom = 0;

		nRect.bottom -= (cfg::dat.statusBarHeight + cfg::dat.bottomOffset);
		nRect.right -= cfg::dat.bCRight;
		nRect.left = cfg::dat.bCLeft;
		nRect.top = cfg::dat.topOffset;
		ContactListHeight = nRect.bottom - nRect.top;

		tick = GetTickCount();

		CLUIFramesResize(nRect);
		CLUIFramesApplyNewSizes(3);
	}

	tick = GetTickCount() - tick;

	if (pcli->hwndContactList != 0)
		InvalidateRect(pcli->hwndContactList, NULL, TRUE);
	if (pcli->hwndContactList != 0)
		UpdateWindow(pcli->hwndContactList);

	Sleep(0);

	if (GetTickCount() - LastStoreTick > 2000) {
		CLUIFramesStoreAllFrames();
		LastStoreTick = GetTickCount();
	}
	return 0;
}

static  HBITMAP hBmpBackground;
static int backgroundBmpUse;
static COLORREF bkColour;
static COLORREF SelBkColour;
boolean AlignCOLLIconToLeft; //will hide frame icon

int OnFrameTitleBarBackgroundChange()
{
	DBVARIANT dbv;

	AlignCOLLIconToLeft = cfg::getByte("FrameTitleBar", "AlignCOLLIconToLeft", 0);

	bkColour = cfg::getDword("FrameTitleBar", "BkColour", CLCDEFAULT_BKCOLOUR);

	if (hBmpBackground) {
		DeleteObject(hBmpBackground);
		hBmpBackground = NULL;
	}
	if (cfg::getByte("FrameTitleBar", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
		if (!db_get(NULL, "FrameTitleBar", "BkBitmap", &dbv)) {
			hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
			mir_free(dbv.pszVal);
		}
	}
	backgroundBmpUse = cfg::getWord("FrameTitleBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);

	CLUIFramesOnClistResize(0, 0);
	return 0;
}

static int DrawTitleBar(HDC dc, RECT rect, int Frameid)
{
	StatusItems_t *item = arStatusItems[ID_EXTBKFRAMETITLE - ID_STATUS_OFFLINE];

	/*
	 * no need to redraw anything while shutting down
	 */
	if (cfg::shutDown)
		return 0;

	TitleBarH = cfg::dat.titleBarHeight;
	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hBmpOsb = CreateCompatibleBitmap(dc, rect.right, rect.bottom);
	HBITMAP hoBmp = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hBmpOsb));

	SetBkMode(hdcMem, TRANSPARENT);

	HBRUSH hBack = GetSysColorBrush(COLOR_3DFACE);
	HBRUSH hoBrush = reinterpret_cast<HBRUSH>(SelectObject(hdcMem, hBack));
	{
		mir_cslock lck(csFrameHook);
		int pos = id2pos(Frameid);
		if (pos >= 0 && pos < nFramescount) {
			HFONT oFont;
			int fHeight, fontTop;
			GetClientRect(Frames[pos].TitleBar.hwnd, &Frames[pos].TitleBar.wndSize);

			if (cfg::clcdat) {
				oFont = ChangeToFont(hdcMem, cfg::clcdat, FONTID_FRAMETITLE, &fHeight);
			}
			else {
				oFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, GetStockObject(DEFAULT_GUI_FONT)));
				fHeight = 10;
			}
			fontTop = (TitleBarH - fHeight) / 2;

			if (cfg::dat.bWallpaperMode && !Frames[pos].floating)
				SkinDrawBg(Frames[pos].TitleBar.hwnd, hdcMem);

			if (!item->IGNORED) {
				RECT rc = Frames[pos].TitleBar.wndSize;
				rc.top += item->MARGIN_TOP;
				rc.bottom -= item->MARGIN_BOTTOM;
				rc.left += item->MARGIN_LEFT;
				rc.right -= item->MARGIN_RIGHT;
				DrawAlpha(hdcMem, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT,
							 item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
				SetTextColor(hdcMem, item->TEXTCOLOR);
			}
			else if (cfg::clcdat) {
				FillRect(hdcMem, &rect, hBack);
				SetTextColor(hdcMem, cfg::clcdat->fontInfo[FONTID_FRAMETITLE].colour);
			}
			else {
				FillRect(hdcMem, &rect, hBack);
				SetTextColor(hdcMem, GetSysColor(COLOR_BTNTEXT));
			}

			if (!AlignCOLLIconToLeft) {
				if (Frames[pos].TitleBar.hicon != NULL) {
					DrawIconEx(hdcMem, 6 + cfg::dat.bClipBorder, ((TitleBarH >> 1) - 8), Frames[pos].TitleBar.hicon, 16, 16, 0, NULL, DI_NORMAL);
					TextOut(hdcMem, 24 + cfg::dat.bClipBorder, fontTop, Frames[pos].TitleBar.tbname, lstrlen(Frames[pos].TitleBar.tbname));
				}
				else TextOut(hdcMem, 6 + cfg::dat.bClipBorder, fontTop, Frames[pos].TitleBar.tbname, lstrlen(Frames[pos].TitleBar.tbname));
			}
			else TextOut(hdcMem, 18 + cfg::dat.bClipBorder, fontTop, Frames[pos].TitleBar.tbname, lstrlen(Frames[pos].TitleBar.tbname));

			if (!AlignCOLLIconToLeft)
				DrawIconEx(hdcMem, Frames[pos].TitleBar.wndSize.right - 22, ((TitleBarH >> 1) - 8), Frames[pos].collapsed ? LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN) : LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT), 16, 16, 0, NULL, DI_NORMAL);
			else
				DrawIconEx(hdcMem, 0, ((TitleBarH >> 1) - 8), Frames[pos].collapsed ? LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN) : LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT), 16, 16, 0, NULL, DI_NORMAL);
			SelectObject(hdcMem, oFont);
		}
	}

	BitBlt(dc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hdcMem, rect.left, rect.top, SRCCOPY);
	SelectObject(hdcMem, hoBmp);
	SelectObject(hdcMem, hoBrush);
	DeleteDC(hdcMem);
	DeleteObject(hBack);
	DeleteObject(hBmpOsb);
	return 0;
}

#define MPCF_CONTEXTFRAMEMENU		3
POINT ptOld;
short   nLeft = 0;
short   nTop = 0;

LRESULT CALLBACK CLUIFrameTitleBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	int Frameid, Framemod, direction;
	int xpos, ypos, framepos;

	Frameid = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	memset(&rect, 0, sizeof(rect));

	switch (msg) {
	case WM_CREATE:
		return FALSE;
	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_ENABLE:
		if (hwnd != 0) InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	case WM_SIZE:
		return 0;

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), 0), Frameid))
			break;

		if (HIWORD(wParam) == 0) {//mouse events for self created menu
			framepos = id2pos(Frameid);
			if (framepos == -1)
				break;

			switch (LOWORD(wParam)) {
			case frame_menu_lock:
				Frames[framepos].Locked = !Frames[framepos].Locked;
				break;
			case frame_menu_visible:
				Frames[framepos].visible = !Frames[framepos].visible;
				break;
			case frame_menu_showtitlebar:
				Frames[framepos].TitleBar.ShowTitleBar = !Frames[framepos].TitleBar.ShowTitleBar;
				break;
			case frame_menu_floating:
				CLUIFrameSetFloat(Frameid, 0);
				break;
			}
			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
		}
		break;

	case WM_RBUTTONDOWN:
		{
			HMENU hmenu;
			if (ServiceExists(MS_CLIST_MENUBUILDFRAMECONTEXT))
				hmenu = (HMENU)CallService(MS_CLIST_MENUBUILDFRAMECONTEXT, Frameid, 0);
			else {
				framepos = id2pos(Frameid);

				mir_cslock lck(csFrameHook);
				if (framepos == -1)
					break;

				hmenu = CreatePopupMenu();
				AppendMenu(hmenu, MF_STRING | MF_DISABLED | MF_GRAYED, 15, Frames[framepos].name);
				AppendMenu(hmenu, MF_SEPARATOR, 16, _T(""));

				if (Frames[framepos].Locked)
					AppendMenu(hmenu, MF_STRING | MF_CHECKED, frame_menu_lock, TranslateT("Lock frame"));
				else
					AppendMenu(hmenu, MF_STRING, frame_menu_lock, TranslateT("Lock frame"));

				if (Frames[framepos].visible)
					AppendMenu(hmenu, MF_STRING | MF_CHECKED, frame_menu_visible, TranslateT("Visible"));
				else
					AppendMenu(hmenu, MF_STRING, frame_menu_visible, TranslateT("Visible"));

				if (Frames[framepos].TitleBar.ShowTitleBar)
					AppendMenu(hmenu, MF_STRING | MF_CHECKED, frame_menu_showtitlebar, TranslateT("Show title bar"));
				else
					AppendMenu(hmenu, MF_STRING, frame_menu_showtitlebar, TranslateT("Show title bar"));

				if (Frames[framepos].Skinned)
					AppendMenu(hmenu, MF_STRING | MF_CHECKED, frame_menu_skinned, TranslateT("Skinned frame"));
				else
					AppendMenu(hmenu, MF_STRING, frame_menu_skinned, TranslateT("Skinned frame"));

				if (Frames[framepos].floating)
					AppendMenu(hmenu, MF_STRING | MF_CHECKED, frame_menu_floating, TranslateT("Floating"));
				else
					AppendMenu(hmenu, MF_STRING, frame_menu_floating, TranslateT("Floating"));
			}
			POINT pt;
			GetCursorPos(&pt);
			TrackPopupMenu(hmenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, 0);
			DestroyMenu(hmenu);
		}
		break;

	case WM_LBUTTONDBLCLK:
		Framemod = -1;
		lbypos = -1;
		oldframeheight = -1;
		ReleaseCapture();
		CallService(MS_CLIST_FRAMES_UCOLLFRAME, Frameid, 0);
		lbypos = -1;
		oldframeheight = -1;
		ReleaseCapture();
		break;

	case WM_LBUTTONUP:
		if (GetCapture() != hwnd)
			break;

		curdragbar = -1;
		lbypos = -1;
		oldframeheight = -1;
		ReleaseCapture();
		RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		break;

	case WM_LBUTTONDOWN:
		framepos = id2pos(Frameid);
		{
			mir_cslock lck(csFrameHook);
			if (framepos == -1)
				break;

			if (Frames[framepos].floating) {
				POINT pt;
				GetCursorPos(&pt);
				Frames[framepos].TitleBar.oldpos = pt;
			}

			if ((!(wParam&MK_CONTROL)) && Frames[framepos].Locked && (!(Frames[framepos].floating))) {
				if (cfg::getByte("CLUI", "ClientAreaDrag", 0)) {
					POINT pt;
					GetCursorPos(&pt);
					return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
				}
			}
			if (Frames[framepos].floating) {
				RECT rc;
				GetCursorPos(&ptOld);
				GetWindowRect(hwnd, &rc);
				nLeft = (short)rc.left;
				nTop = (short)rc.top;
			}
		}
		SetCapture(hwnd);
		break;

	case WM_MOUSEMOVE:
		{
			char TBcapt[255];

			mir_cslock lck(csFrameHook);
			int pos = id2pos(Frameid);
			if (pos != -1) {
				int oldflags;
				mir_snprintf(TBcapt, SIZEOF(TBcapt), "%s - h:%d, vis:%d, fl:%d, fl:(%d,%d,%d,%d),or: %d",
								 Frames[pos].name, Frames[pos].height, Frames[pos].visible, Frames[pos].floating,
								 Frames[pos].FloatingPos.x, Frames[pos].FloatingPos.y,
								 Frames[pos].FloatingSize.x, Frames[pos].FloatingSize.y,
								 Frames[pos].order
								 );

				oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, Frames[pos].id), 0);
				if (!(oldflags&F_SHOWTBTIP))
					oldflags |= F_SHOWTBTIP;
			}
		}
		if (wParam & MK_LBUTTON) {
			RECT rcMiranda;
			RECT rcwnd, rcOverlap;
			POINT newpt, ofspt, curpt, newpos;

			mir_cslockfull lck(csFrameHook);

			int pos = id2pos(Frameid);
			if (Frames[pos].floating) {
				GetCursorPos(&curpt);
				rcwnd.bottom = curpt.y + 5;
				rcwnd.top = curpt.y;
				rcwnd.left = curpt.x;
				rcwnd.right = curpt.x + 5;

				GetWindowRect(pcli->hwndContactList, &rcMiranda);
				if (IsWindowVisible(pcli->hwndContactList) && IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
					int id = Frames[pos].id;

					lck.unlock();
					ofspt.x = 0;
					ofspt.y = 0;
					ClientToScreen(Frames[pos].TitleBar.hwnd, &ofspt);
					ofspt.x = curpt.x - ofspt.x;
					ofspt.y = curpt.y - ofspt.y;

					CLUIFrameSetFloat(id, 0);
					newpt.x = 0;
					newpt.y = 0;
					ClientToScreen(Frames[pos].TitleBar.hwnd, &newpt);
					SetCursorPos(newpt.x + ofspt.x, newpt.y + ofspt.y);
					GetCursorPos(&curpt);

					lck.lock();
					Frames[pos].TitleBar.oldpos = curpt;
					return 0;
				}
			}
			else {
				int id = Frames[pos].id;

				GetCursorPos(&curpt);
				rcwnd.bottom = curpt.y + 5;
				rcwnd.top = curpt.y;
				rcwnd.left = curpt.x;
				rcwnd.right = curpt.x + 5;

				GetWindowRect(pcli->hwndContactList, &rcMiranda);

				if (!IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
					lck.unlock();
					GetCursorPos(&curpt);
					GetWindowRect(Frames[pos].hWnd, &rcwnd);
					rcwnd.left = rcwnd.right - rcwnd.left;
					rcwnd.top = rcwnd.bottom - rcwnd.top;
					newpos.x = curpt.x;
					newpos.y = curpt.y;
					if (curpt.x >= (rcMiranda.right - 1))
						newpos.x = curpt.x + 5;
					if (curpt.x <= (rcMiranda.left + 1))
						newpos.x = curpt.x - (rcwnd.left) - 5;
					if (curpt.y >= (rcMiranda.bottom - 1))
						newpos.y = curpt.y + 5;
					if (curpt.y <= (rcMiranda.top + 1))
						newpos.y = curpt.y - (rcwnd.top) - 5;

					ofspt.x = 0;
					ofspt.y = 0;
					GetWindowRect(Frames[pos].TitleBar.hwnd, &rcwnd);
					ofspt.x = curpt.x - ofspt.x;
					ofspt.y = curpt.y - ofspt.y;
					Frames[pos].FloatingPos.x = newpos.x;
					Frames[pos].FloatingPos.y = newpos.y;
					CLUIFrameSetFloat(id, 0);

					lck.lock();
					newpt.x = 0;
					newpt.y = 0;
					ClientToScreen(Frames[pos].TitleBar.hwnd, &newpt);
					GetWindowRect(Frames[pos].hWnd, &rcwnd);
					SetCursorPos(newpt.x + (rcwnd.right - rcwnd.left) / 2, newpt.y + (rcwnd.bottom - rcwnd.top) / 2);
					GetCursorPos(&curpt);
					Frames[pos].TitleBar.oldpos = curpt;
					return 0;
				}
			}
		}
		if (wParam & MK_LBUTTON) {
			int newh = -1, prevold;

			if (GetCapture() != hwnd)
				break;

			POINT pt, pt2;
			mir_cslockfull lck(csFrameHook);
			int pos = id2pos(Frameid);

			if (Frames[pos].floating) {
				RECT wndr;
				GetCursorPos(&pt);
				if ((Frames[pos].TitleBar.oldpos.x != pt.x) || (Frames[pos].TitleBar.oldpos.y != pt.y)) {
					pt2 = pt;
					ScreenToClient(hwnd, &pt2);
					GetWindowRect(Frames[pos].ContainerWnd, &wndr);

					POINT ptNew = pt;

					nLeft += (short)ptNew.x - ptOld.x;
					nTop += (short)ptNew.y - ptOld.y;

					if (!(wParam & MK_CONTROL))
						PositionThumb(&Frames[pos], nLeft, nTop);
					else
						SetWindowPos(Frames[pos].ContainerWnd, 0, nLeft, nTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

					ptOld = ptNew;

					pt.x = nLeft;
					pt.y = nTop;
					Frames[pos].TitleBar.oldpos = pt;
				}
				return 0;
			}
			if (Frames[pos].prevvisframe != -1) {
				GetCursorPos(&pt);

				if ((Frames[pos].TitleBar.oldpos.x == pt.x) && (Frames[pos].TitleBar.oldpos.y == pt.y))
					break;

				ypos = rect.top + pt.y;
				xpos = rect.left + pt.x;
				Framemod = -1;

				if (Frames[pos].align == alBottom) {
					direction = -1;
					Framemod = pos;
				}
				else {
					direction = 1;
					Framemod = Frames[pos].prevvisframe;
				}
				if (Frames[Framemod].Locked)
					break;
				if (curdragbar != -1 && curdragbar != pos)
					break;

				if (lbypos == -1) {
					curdragbar = pos;
					lbypos = ypos;
					oldframeheight = Frames[Framemod].height;
					SetCapture(hwnd);
					break;
				}
				newh = oldframeheight + direction * (ypos - lbypos);
				if (newh > 0) {
					prevold = Frames[Framemod].height;
					Frames[Framemod].height = newh;
					if (!CLUIFramesFitInSize()) {
						Frames[Framemod].height = prevold;
						return TRUE;
					}
					Frames[Framemod].height = newh;
					if (newh > 3) Frames[Framemod].collapsed = TRUE;

				}
				Frames[pos].TitleBar.oldpos = pt;
			}
			lck.unlock();

			if (newh > 0)
				CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
			break;
		}
		curdragbar = -1;
		lbypos = -1;
		oldframeheight = -1;
		ReleaseCapture();
		break;

	case WM_NCPAINT:
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_BORDER) {
			HDC hdc = GetWindowDC(hwnd);
			HPEN hPenOld = reinterpret_cast<HPEN>(SelectObject(hdc, g_hPenCLUIFrames));
			RECT rcWindow, rc;
			HBRUSH brold;

			CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
			GetWindowRect(hwnd, &rcWindow);
			rc.left = rc.top = 0;
			rc.right = rcWindow.right - rcWindow.left;
			rc.bottom = rcWindow.bottom - rcWindow.top;
			brold = reinterpret_cast<HBRUSH>(SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)));
			Rectangle(hdc, 0, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top);
			SelectObject(hdc, hPenOld);
			SelectObject(hdc, brold);
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		break;

	case WM_PRINT:
	case WM_PRINTCLIENT:
		GetClientRect(hwnd, &rect);
		DrawTitleBar((HDC)wParam, rect, Frameid);

	case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC paintDC = BeginPaint(hwnd, &paintStruct);
			rect = paintStruct.rcPaint;
			DrawTitleBar(paintDC, rect, Frameid);
			EndPaint(hwnd, &paintStruct);
		}
		return 0;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

int CLUIFrameResizeFloatingFrame(int framepos)
{
	if (!Frames[framepos].floating)
		return 0;
	if (Frames[framepos].ContainerWnd == 0)
		return 0;

	RECT rect;
	GetClientRect(Frames[framepos].ContainerWnd, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	int floatingHeight = cfg::dat.titleBarHeight;

	if (floatingHeight <= 0 || floatingHeight > 50)
		floatingHeight = 18;

	Frames[framepos].visible ? ShowWindow(Frames[framepos].ContainerWnd, SW_SHOWNOACTIVATE) : ShowWindow(Frames[framepos].ContainerWnd, SW_HIDE);

	if (Frames[framepos].TitleBar.ShowTitleBar) {
		ShowWindow(Frames[framepos].TitleBar.hwnd, SW_SHOWNOACTIVATE);
		Frames[framepos].height = height - floatingHeight;
		SetWindowPos(Frames[framepos].TitleBar.hwnd, HWND_TOP, 0, 0, width, floatingHeight, SWP_SHOWWINDOW | SWP_DRAWFRAME | SWP_NOACTIVATE);
		InvalidateRect(Frames[framepos].TitleBar.hwnd, NULL, FALSE);
		SetWindowPos(Frames[framepos].hWnd, HWND_TOP, 0, floatingHeight, width, height - floatingHeight, SWP_SHOWWINDOW | SWP_NOACTIVATE);

	}
	else {
		Frames[framepos].height = height;
		ShowWindow(Frames[framepos].TitleBar.hwnd, SW_HIDE);
		SetWindowPos(Frames[framepos].hWnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}

	if (Frames[framepos].ContainerWnd != 0)
		UpdateWindow(Frames[framepos].ContainerWnd);
	GetWindowRect(Frames[framepos].hWnd, &Frames[framepos].wndSize);

	if (Frames[framepos].TitleBar.ShowTitleBar)
		RedrawWindow(Frames[framepos].TitleBar.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);

	RedrawWindow(Frames[framepos].hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
	return 0;
}

static int CLUIFrameOnMainMenuBuild(WPARAM wParam, LPARAM lParam)
{
	CLUIFramesLoadMainMenu();
	return 0;
}

LRESULT CALLBACK CLUIFrameContainerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int framepos;
	RECT rect;
	INT_PTR Frameid = GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_CREATE:
		{
			mir_cslockfull lck(csFrameHook);
			framepos = id2pos(Frameid);
		}
		return 0;

	case WM_GETMINMAXINFO:
		TitleBarH = cfg::dat.titleBarHeight;
		{
			mir_cslock lck(csFrameHook);
			framepos = id2pos(Frameid);
			if (framepos < 0 || framepos >= nFramescount)
				break;

			if (!Frames[framepos].minmaxenabled)
				break;

			if (Frames[framepos].ContainerWnd == 0)
				break;

			if (Frames[framepos].Locked) {
				RECT rct;
				GetWindowRect(hwnd, &rct);
				((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rct.right - rct.left;
				((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rct.bottom - rct.top;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rct.right - rct.left;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = rct.bottom - rct.top;
			}

			MINMAXINFO minmax;
			memset(&minmax, 0, sizeof(minmax));
			if (SendMessage(Frames[framepos].hWnd, WM_GETMINMAXINFO, 0, (LPARAM)&minmax) != 0)
				return DefWindowProc(hwnd, msg, wParam, lParam);

			RECT border;
			int tbh = TitleBarH * btoint(Frames[framepos].TitleBar.ShowTitleBar);
			GetBorderSize(hwnd, &border);
			if (minmax.ptMaxTrackSize.x != 0 && minmax.ptMaxTrackSize.y != 0) {
				((LPMINMAXINFO)lParam)->ptMinTrackSize.x = minmax.ptMinTrackSize.x;
				((LPMINMAXINFO)lParam)->ptMinTrackSize.y = minmax.ptMinTrackSize.y;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = minmax.ptMaxTrackSize.x + border.left + border.right;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = minmax.ptMaxTrackSize.y + tbh + border.top + border.bottom;
			}
		}

	case WM_MOVE:
		{
			mir_cslock lck(csFrameHook);
			framepos = id2pos(Frameid);
			if (framepos < 0 || framepos >= nFramescount)
				break;

			if (Frames[framepos].ContainerWnd == 0)
				return 0;

			GetWindowRect(Frames[framepos].ContainerWnd, &rect);
			Frames[framepos].FloatingPos.x = rect.left;
			Frames[framepos].FloatingPos.y = rect.top;
			Frames[framepos].FloatingSize.x = rect.right - rect.left;
			Frames[framepos].FloatingSize.y = rect.bottom - rect.top;
			CLUIFramesStoreFrameSettings(framepos);
		}
		return 0;

	case WM_SIZE:
		{
			mir_cslock lck(csFrameHook);
			framepos = id2pos(Frameid);
			if (framepos < 0 || framepos >= nFramescount)
				break;

			if (Frames[framepos].ContainerWnd == 0)
				return 0;

			CLUIFrameResizeFloatingFrame(framepos);

			RECT rect;
			GetWindowRect(Frames[framepos].ContainerWnd, &rect);
			Frames[framepos].FloatingPos.x = rect.left;
			Frames[framepos].FloatingPos.y = rect.top;
			Frames[framepos].FloatingSize.x = rect.right - rect.left;
			Frames[framepos].FloatingSize.y = rect.bottom - rect.top;

			CLUIFramesStoreFrameSettings(framepos);
		}
		return 0;

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
	return(CreateWindowA("FramesContainer", "aaaa", WS_POPUP | WS_THICKFRAME, x, y, width, height, parent, 0, g_hInst, 0));
}

INT_PTR CLUIFrameSetFloat(WPARAM wParam, LPARAM lParam)
{
	HWND hwndtmp, hwndtooltiptmp;
	{
		mir_cslock lck(csFrameHook);
		wParam = id2pos((INT_PTR)wParam);
		if ((int)wParam >= 0 && (int)wParam < nFramescount) {
			if (Frames[wParam].floating) {
				SetParent(Frames[wParam].hWnd, pcli->hwndContactList);
				SetParent(Frames[wParam].TitleBar.hwnd, pcli->hwndContactList);
				Frames[wParam].floating = FALSE;
				DestroyWindow(Frames[wParam].ContainerWnd);
				Frames[wParam].ContainerWnd = 0;
			}
			else {
				RECT recttb, rectw, border;
				int temp;
				int neww, newh;
				BOOLEAN locked;

				Frames[wParam].oldstyles = GetWindowLongPtr(Frames[wParam].hWnd, GWL_STYLE);
				Frames[wParam].TitleBar.oldstyles = GetWindowLongPtr(Frames[wParam].TitleBar.hwnd, GWL_STYLE);
				locked = Frames[wParam].Locked;
				Frames[wParam].Locked = FALSE;
				Frames[wParam].minmaxenabled = FALSE;

				GetWindowRect(Frames[wParam].hWnd, &rectw);
				GetWindowRect(Frames[wParam].TitleBar.hwnd, &recttb);
				if (!Frames[wParam].TitleBar.ShowTitleBar)
					recttb.top = recttb.bottom = recttb.left = recttb.right = 0;

				Frames[wParam].ContainerWnd = CreateContainerWindow(pcli->hwndContactList, Frames[wParam].FloatingPos.x, Frames[wParam].FloatingPos.y, 10, 10);

				SetParent(Frames[wParam].hWnd, Frames[wParam].ContainerWnd);
				SetParent(Frames[wParam].TitleBar.hwnd, Frames[wParam].ContainerWnd);

				GetBorderSize(Frames[wParam].ContainerWnd, &border);

				SetWindowLongPtr(Frames[wParam].ContainerWnd, GWLP_USERDATA, Frames[wParam].id);
				if ((lParam == 1)) {
					if ((Frames[wParam].FloatingPos.x != 0) && (Frames[wParam].FloatingPos.y != 0)) {
						if (Frames[wParam].FloatingPos.x < 20)
							Frames[wParam].FloatingPos.x = 40;

						if (Frames[wParam].FloatingPos.y < 20)
							Frames[wParam].FloatingPos.y = 40;

						SetWindowPos(Frames[wParam].ContainerWnd, HWND_TOPMOST, Frames[wParam].FloatingPos.x, Frames[wParam].FloatingPos.y, Frames[wParam].FloatingSize.x, Frames[wParam].FloatingSize.y, SWP_HIDEWINDOW);
					}
					else SetWindowPos(Frames[wParam].ContainerWnd, HWND_TOPMOST, 120, 120, 140, 140, SWP_HIDEWINDOW);
				}
				else {
					neww = rectw.right - rectw.left + border.left + border.right;
					newh = (rectw.bottom - rectw.top) + (recttb.bottom - recttb.top) + border.top + border.bottom;
					if (neww < 20)
						neww = 40;

					if (newh < 20)
						newh = 40;

					if (Frames[wParam].FloatingPos.x < 20)
						Frames[wParam].FloatingPos.x = 40;

					if (Frames[wParam].FloatingPos.y < 20)
						Frames[wParam].FloatingPos.y = 40;

					SetWindowPos(Frames[wParam].ContainerWnd, HWND_TOPMOST, Frames[wParam].FloatingPos.x, Frames[wParam].FloatingPos.y, neww, newh, SWP_HIDEWINDOW);
				}
				SetWindowText(Frames[wParam].ContainerWnd, Frames[wParam].TitleBar.tbname);
				temp = GetWindowLongPtr(Frames[wParam].ContainerWnd, GWL_EXSTYLE);
				temp |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
				SetWindowLongPtr(Frames[wParam].ContainerWnd, GWL_EXSTYLE, temp);
				Frames[wParam].floating = TRUE;
				Frames[wParam].Locked = locked;
			}
		}

		CLUIFramesStoreFrameSettings(wParam);
		Frames[wParam].minmaxenabled = TRUE;
		hwndtooltiptmp = Frames[wParam].TitleBar.hwndTip;

		hwndtmp = Frames[wParam].ContainerWnd;
	}

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	SendMessage(hwndtmp, WM_SIZE, 0, 0);
	SetWindowPos(hwndtooltiptmp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return 0;
}

TCHAR g_ptszEventName[100];

static int CLUIFrameOnModulesLoad(WPARAM wParam, LPARAM lParam)
{
	mir_sntprintf(g_ptszEventName, SIZEOF(g_ptszEventName), _T("mf_update_evt_%d"), GetCurrentThreadId());
	g_hEventThread = CreateEvent(NULL, TRUE, FALSE, g_ptszEventName);
	hThreadMFUpdate = mir_forkthread(MF_UpdateThread, NULL);
	SetThreadPriority(hThreadMFUpdate, THREAD_PRIORITY_IDLE);
	CLUIFramesLoadMainMenu();
	CLUIFramesCreateMenuForFrame(-1, -1, 000010000, Menu_AddContextFrameMenuItem);
	return 0;
}

static int CLUIFrameOnModulesUnload(WPARAM wParam, LPARAM lParam)
{
	mf_updatethread_running = FALSE;

	SetThreadPriority(hThreadMFUpdate, THREAD_PRIORITY_NORMAL);
	SetEvent(g_hEventThread);
	WaitForSingleObject(hThreadMFUpdate, 2000);
	CloseHandle(g_hEventThread);

	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIVisible, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMITitle, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMITBVisible, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMILock, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIColl, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIFloating, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIAlignRoot, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIAlignTop, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIAlignClient, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIAlignBottom, 0);
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, (LPARAM)contMIBorder, 0);
	return 0;
}

/*
 * wparam=hIcon
 * return hImage on success,-1 on failure
 */

void RegisterCLUIFrameClasses()
{
	WNDCLASS wndclass;
	WNDCLASS cntclass;

	wndclass.style = CS_DBLCLKS;//|CS_HREDRAW|CS_VREDRAW ;
	wndclass.lpfnWndProc = CLUIFrameTitleBarProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_hInst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = CLUIFrameTitleBarClassName;
	RegisterClass(&wndclass);

	cntclass.style = CS_DBLCLKS/*|CS_HREDRAW|CS_VREDRAW*/ | CS_DROPSHADOW;
	cntclass.lpfnWndProc = CLUIFrameContainerWndProc;
	cntclass.cbClsExtra = 0;
	cntclass.cbWndExtra = 0;
	cntclass.hInstance = g_hInst;
	cntclass.hIcon = NULL;
	cntclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	cntclass.hbrBackground = NULL;
	cntclass.lpszMenuName = NULL;
	cntclass.lpszClassName = _T("FramesContainer");
	RegisterClass(&cntclass);
}

int LoadCLUIFramesModule(void)
{
	GapBetweenFrames = cfg::dat.gapBetweenFrames;

	nFramescount = 0;
	InitFramesMenus();

	HookEvent(ME_SYSTEM_MODULESLOADED, CLUIFrameOnModulesLoad);
	HookEvent(ME_CLIST_PREBUILDFRAMEMENU, CLUIFramesModifyContextMenuForFrame);
	HookEvent(ME_CLIST_PREBUILDMAINMENU, CLUIFrameOnMainMenuBuild);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, CLUIFrameOnModulesUnload);

	CreateServiceFunction(MS_CLIST_FRAMES_ADDFRAME, CLUIFramesAddFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_REMOVEFRAME, CLUIFramesRemoveFrame);

	CreateServiceFunction(MS_CLIST_FRAMES_SETFRAMEOPTIONS, CLUIFramesSetFrameOptions);
	CreateServiceFunction(MS_CLIST_FRAMES_GETFRAMEOPTIONS, CLUIFramesGetFrameOptions);
	CreateServiceFunction(MS_CLIST_FRAMES_UPDATEFRAME, CLUIFramesUpdateFrame);

	CreateServiceFunction(MS_CLIST_FRAMES_SHFRAMETITLEBAR, CLUIFramesShowHideFrameTitleBar);
	CreateServiceFunction(MS_CLIST_FRAMES_SHOWALLFRAMESTB, CLUIFramesShowAllTitleBars);
	CreateServiceFunction(MS_CLIST_FRAMES_HIDEALLFRAMESTB, CLUIFramesHideAllTitleBars);
	CreateServiceFunction(MS_CLIST_FRAMES_SHFRAME, CLUIFramesShowHideFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_SHOWALLFRAMES, CLUIFramesShowAll);

	CreateServiceFunction(MS_CLIST_FRAMES_ULFRAME, CLUIFramesLockUnlockFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_UCOLLFRAME, CLUIFramesCollapseUnCollapseFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_SETUNBORDER, CLUIFramesSetUnSetBorder);
	CreateServiceFunction(MS_CLIST_FRAMES_SETSKINNED, CLUIFramesSetUnSetSkinned);

	CreateServiceFunction(CLUIFRAMESSETALIGN, CLUIFramesSetAlign);
	CreateServiceFunction(CLUIFRAMESMOVEDOWN, CLUIFramesMoveDown);
	CreateServiceFunction(CLUIFRAMESMOVEUP, CLUIFramesMoveUp);

	CreateServiceFunction(CLUIFRAMESSETALIGNALTOP, CLUIFramesSetAlignalTop);
	CreateServiceFunction(CLUIFRAMESSETALIGNALCLIENT, CLUIFramesSetAlignalClient);
	CreateServiceFunction(CLUIFRAMESSETALIGNALBOTTOM, CLUIFramesSetAlignalBottom);

	CreateServiceFunction("Set_Floating", CLUIFrameSetFloat);
	hWndExplorerToolBar = FindWindowExA(0, 0, "Shell_TrayWnd", NULL);
	OnFrameTitleBarBackgroundChange();

	FramesSysNotStarted = FALSE;
	g_hPenCLUIFrames = CreatePen(PS_SOLID, 1, cfg::getDword("CLUI", "clr_frameborder", GetSysColor(COLOR_3DDKSHADOW)));
	return 0;
}

void LoadExtraIconModule()
{
	hStatusBarShowToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP);
	hStatusBarHideToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP);
}

int UnLoadCLUIFramesModule(void)
{
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	CLUIFramesStoreAllFrames();
	DeleteObject(g_hPenCLUIFrames);

	mir_cslock lck(csFrameHook);
	FramesSysNotStarted = TRUE;
	for (int i = 0; i < nFramescount; i++) {
		DestroyWindow(Frames[i].hWnd);
		Frames[i].hWnd = (HWND)-1;
		DestroyWindow(Frames[i].TitleBar.hwnd);
		Frames[i].TitleBar.hwnd = (HWND)-1;
		DestroyWindow(Frames[i].ContainerWnd);
		Frames[i].ContainerWnd = (HWND)-1;
		DestroyMenu(Frames[i].TitleBar.hmenu);

		if (Frames[i].name != NULL)
			mir_free(Frames[i].name);
		if (Frames[i].TitleBar.tbname != NULL)
			mir_free(Frames[i].TitleBar.tbname);
	}
	free(Frames);
	Frames = NULL;
	nFramescount = 0;
	UnregisterClass(CLUIFrameTitleBarClassName, g_hInst);
	return 0;
}
