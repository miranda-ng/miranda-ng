/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "cluiframes.h"
HFONT __fastcall ChangeToFont(HDC hdc, struct ClcData *dat, int id, int *fontHeight);

extern HWND g_hwndViewModeFrame, g_hwndEventArea;
extern int mf_updatethread_running;

extern HANDLE hThreadMFUpdate;

void MF_UpdateThread(LPVOID);

HANDLE hStatusBarShowToolTipEvent, hStatusBarHideToolTipEvent;
HANDLE g_hEventThread = nullptr;

LOGFONT TitleBarLogFont = { 0 };

// we use dynamic frame list,
// but who wants so huge number of frames ??
#define MAX_FRAMES		40

#define UNCOLLAPSED_FRAME_SIZE		0

// legacy menu support
#define frame_menu_lock				1
#define frame_menu_visible			2
#define frame_menu_showtitlebar		3
#define frame_menu_floating			4
#define frame_menu_skinned			5

static int UpdateTBToolTip(int framepos);
INT_PTR CLUIFrameSetFloat(WPARAM wParam, LPARAM lParam);
int CLUIFrameResizeFloatingFrame(int framepos);
static int CLUIFramesReSort();

boolean FramesSysNotStarted = TRUE;
HPEN g_hPenCLUIFrames = nullptr;

static SortData g_sd[MAX_FRAMES];

static HHOOK g_hFrameHook = nullptr;

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

static FRAMEWND *Frames = nullptr;

FRAMEWND *wndFrameCLC = nullptr, *wndFrameEventArea = nullptr, *wndFrameViewMode = nullptr;

static int nFramescount = 0;
static int alclientFrame = -1;//for fast access to frame with alclient properties
static int NextFrameId = 100;

static int TitleBarH = DEFAULT_TITLEBAR_HEIGHT;
static boolean resizing = FALSE;

// menus
static FrameMenuHandles cont;
static LIST<TMO_IntMenuItem> g_frameMenus(10);

// others
static int ContactListHeight;
static int LastStoreTick = 0;

static int lbypos = -1;
static int oldframeheight = -1;
static int curdragbar = -1;
static mir_cs csFrameHook;

static bool CLUIFramesFitInSize(void);
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

int __forceinline btoint(bool b)
{
	return (b ? 1 : 0);
}

static FRAMEWND* FindFrameByWnd(HWND hwnd)
{
	if (hwnd == nullptr)
		return nullptr;

	for (int i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		if (F.floating && F.ContainerWnd == hwnd)
			return &F;
	}

	return nullptr;
}

static void DockThumbs(FRAMEWND *pThumbLeft, FRAMEWND *pThumbRight, BOOL)
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

	if (pThumb == nullptr)
		return;

	sizeScreen.cx = GetSystemMetrics(SM_CXSCREEN);
	sizeScreen.cy = GetSystemMetrics(SM_CYSCREEN);

	// Get thumb dimnsions
	GetWindowRect(pThumb->ContainerWnd, &rcThumb);
	int nWidth = rcThumb.right - rcThumb.left;
	int nHeight = rcThumb.bottom - rcThumb.top;

	// Docking to the edges of the screen
	int nNewX = nX < nOffs ? 0 : nX;
	nNewX = nNewX >(sizeScreen.cx - nWidth - nOffs) ? (sizeScreen.cx - nWidth) : nNewX;
	int nNewY = nY < nOffs ? 0 : nY;
	nNewY = nNewY >(sizeScreen.cy - nHeight - nOffs) ? (sizeScreen.cy - nHeight) : nNewY;

	bool bLeading = pThumb->dockOpt.hwndRight != nullptr;

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
	SetWindowPos(pThumb->ContainerWnd, nullptr, nNewX, nNewY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

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

int DBLoadFrameSettingsAtPos(int pos, int Frameid)
{
	CMStringA buf;

	Frames[Frameid].collapsed = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Collapse%d", pos), Frames[Frameid].collapsed);

	Frames[Frameid].Locked = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Locked%d", pos), Frames[Frameid].Locked);
	Frames[Frameid].visible = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Visible%d", pos), Frames[Frameid].visible);
	Frames[Frameid].TitleBar.ShowTitleBar = 0 != db_get_b(0, CLUIFrameModule, buf.Format("TBVisile%d", pos), Frames[Frameid].TitleBar.ShowTitleBar);

	Frames[Frameid].height = db_get_w(0, CLUIFrameModule, buf.Format("Height%d", pos), Frames[Frameid].height);
	Frames[Frameid].HeightWhenCollapsed = db_get_w(0, CLUIFrameModule, buf.Format("HeightCollapsed%d", pos), 0);
	Frames[Frameid].align = db_get_w(0, CLUIFrameModule, buf.Format("Align%d", pos), Frames[Frameid].align);

	Frames[Frameid].FloatingPos.x = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatX%d", pos), 100, 0, 1024);
	Frames[Frameid].FloatingPos.y = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatY%d", pos), 100, 0, 1024);
	Frames[Frameid].FloatingSize.x = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatW%d", pos), 100, 0, 1024);
	Frames[Frameid].FloatingSize.y = DBGetContactSettingRangedWord(0, CLUIFrameModule, buf.Format("FloatH%d", pos), 100, 0, 1024);

	Frames[Frameid].floating = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Floating%d", pos), 0);
	Frames[Frameid].order = db_get_w(0, CLUIFrameModule, buf.Format("Order%d", pos), 0);

	Frames[Frameid].UseBorder = 0 != db_get_b(0, CLUIFrameModule, buf.Format("UseBorder%d", pos), Frames[Frameid].UseBorder);
	Frames[Frameid].Skinned = 0 != db_get_b(0, CLUIFrameModule, buf.Format("Skinned%d", pos), Frames[Frameid].Skinned);
	return 0;
}

int DBStoreFrameSettingsAtPos(int pos, int Frameid)
{
	CMStringA buf;

	db_set_ws(0, CLUIFrameModule, buf.Format("Name%d", pos), Frames[Frameid].name);
	//boolean
	db_set_b(0, CLUIFrameModule, buf.Format("Collapse%d", pos), (uint8_t)btoint(Frames[Frameid].collapsed));
	db_set_b(0, CLUIFrameModule, buf.Format("Locked%d", pos), (uint8_t)btoint(Frames[Frameid].Locked));
	db_set_b(0, CLUIFrameModule, buf.Format("Visible%d", pos), (uint8_t)btoint(Frames[Frameid].visible));
	db_set_b(0, CLUIFrameModule, buf.Format("TBVisile%d", pos), (uint8_t)btoint(Frames[Frameid].TitleBar.ShowTitleBar));

	db_set_w(0, CLUIFrameModule, buf.Format("Height%d", pos), (uint16_t)Frames[Frameid].height);
	db_set_w(0, CLUIFrameModule, buf.Format("HeightCollapsed%d", pos), (uint16_t)Frames[Frameid].HeightWhenCollapsed);
	db_set_w(0, CLUIFrameModule, buf.Format("Align%d", pos), (uint16_t)Frames[Frameid].align);
	//FloatingPos
	db_set_w(0, CLUIFrameModule, buf.Format("FloatX%d", pos), (uint16_t)Frames[Frameid].FloatingPos.x);
	db_set_w(0, CLUIFrameModule, buf.Format("FloatY%d", pos), (uint16_t)Frames[Frameid].FloatingPos.y);
	db_set_w(0, CLUIFrameModule, buf.Format("FloatW%d", pos), (uint16_t)Frames[Frameid].FloatingSize.x);
	db_set_w(0, CLUIFrameModule, buf.Format("FloatH%d", pos), (uint16_t)Frames[Frameid].FloatingSize.y);

	db_set_b(0, CLUIFrameModule, buf.Format("Floating%d", pos), (uint8_t)btoint(Frames[Frameid].floating));
	db_set_b(0, CLUIFrameModule, buf.Format("UseBorder%d", pos), (uint8_t)btoint(Frames[Frameid].UseBorder));
	db_set_w(0, CLUIFrameModule, buf.Format("Order%d", pos), (uint16_t)Frames[Frameid].order);

	db_set_b(0, CLUIFrameModule, buf.Format("Skinned%d", pos), Frames[Frameid].Skinned);
	return 0;
}

int LocateStorePosition(int Frameid, int maxstored)
{
	if (Frames[Frameid].name == nullptr) return -1;

	for (int i = 0; i < maxstored; i++) {
		char settingname[255];
		mir_snprintf(settingname, "Name%d", i);
		ptrW frmname(db_get_wsa(0, CLUIFrameModule, settingname));
		if (frmname == NULL) continue;
		if (mir_wstrcmpi(frmname, Frames[Frameid].name) == 0)
			return i;
	}
	return -1;
}

int CLUIFramesLoadFrameSettings(int Frameid)
{
	if (FramesSysNotStarted) return -1;

	if (Frameid < 0 || Frameid >= nFramescount)
		return -1;

	int maxstored = db_get_w(0, CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1)
		return 0;

	int storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1)
		return 0;

	DBLoadFrameSettingsAtPos(storpos, Frameid);
	return 0;
}

int CLUIFramesStoreFrameSettings(int Frameid)
{
	if (FramesSysNotStarted)
		return -1;

	if (Frameid < 0 || Frameid >= nFramescount)
		return -1;

	int maxstored = db_get_w(0, CLUIFrameModule, "StoredFrames", -1);
	if (maxstored == -1)
		maxstored = 0;

	int storpos = LocateStorePosition(Frameid, maxstored);
	if (storpos == -1) {
		storpos = maxstored;
		maxstored++;
	}

	DBStoreFrameSettingsAtPos(storpos, Frameid);
	db_set_w(0, CLUIFrameModule, "StoredFrames", (uint16_t)maxstored);
	return 0;
}

int CLUIFramesStoreAllFrames()
{
	if (FramesSysNotStarted)
		return -1;

	if (cfg::shutDown)
		return -1;

	mir_cslock lck(csFrameHook);
	for (int i = 0; i < nFramescount; i++)
		CLUIFramesStoreFrameSettings(i);
	return 0;
}

// Get client frame
int CLUIFramesGetalClientFrame(void)
{
	if (FramesSysNotStarted)
		return -1;

	if (alclientFrame != -1) {
		/* this value could become invalid if RemoveItemFromList was called,
		 * so we double-check */
		if (alclientFrame < nFramescount)
			if (Frames[alclientFrame].align == alClient)
				return alclientFrame;
	}

	for (int i = 0; i < nFramescount; i++)
		if (Frames[i].align == alClient) {
			alclientFrame = i;
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

HMENU CLUIFramesCreateMenuForFrame(int frameid, HGENMENU root, int popuppos, bool bMain)
{
	if (FramesSysNotStarted)
		return nullptr;

	int framepos = id2pos(frameid);
	FrameMenuHandles &fmh = (frameid == -1) ? cont : Frames[framepos].MenuHandles;

	CMenuItem mi((frameid == -1) ? &g_plugin : Frames[framepos].pPlugin);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.root = root;
	mi.position = popuppos++;
	mi.name.a = LPGEN("&FrameTitle");
	mi.flags = CMIF_SYSTEM | CMIF_GRAYED;
	fmh.MITitle = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	mi.hIcolibItem = nullptr;
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Visible");
	mi.flags = CMIF_SYSTEM | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SHFRAME;
	fmh.MIVisible = addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Show title bar");
	mi.pszService = MS_CLIST_FRAMES_SHFRAMETITLEBAR;
	fmh.MITBVisible = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Locked");
	mi.pszService = MS_CLIST_FRAMES_ULFRAME;
	fmh.MILock = addFrameMenuItem(&mi, frameid, bMain);

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Collapsed");
	mi.pszService = MS_CLIST_FRAMES_UCOLLFRAME;
	fmh.MIColl = addFrameMenuItem(&mi, frameid, bMain);

	// floating
	mi.position = popuppos++;
	mi.name.a = LPGEN("&Floating mode");
	mi.flags = CMIF_SYSTEM;
	mi.pszService = "Set_Floating";
	fmh.MIFloating = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Border");
	mi.flags = CMIF_SYSTEM | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_SETUNBORDER;
	fmh.MIBorder = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	mi.position = popuppos++;
	mi.name.a = LPGEN("&Skinned frame");
	mi.pszService = MS_CLIST_FRAMES_SETSKINNED;
	fmh.MISkinned = addFrameMenuItem(&mi, frameid, bMain);

	popuppos += 100000;

	// alignment root
	mi.root = root;
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
	if (FramesSysNotStarted)
		return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos(wParam);
	if (pos >= 0 && pos < nFramescount) {
		FRAMEWND &p = Frames[pos];
		Menu_ModifyItem(cont.MITitle, p.TitleBar.tbname ? p.TitleBar.tbname : p.name);
		Menu_SetChecked(cont.MIVisible, p.visible);
		Menu_SetChecked(cont.MILock, p.Locked);
		Menu_SetChecked(cont.MITBVisible, p.TitleBar.ShowTitleBar);
		Menu_SetChecked(cont.MIFloating, p.floating);
		Menu_SetChecked(cont.MIBorder, p.UseBorder);
		Menu_SetChecked(cont.MISkinned, p.Skinned);
		Menu_SetChecked(cont.MIAlignTop, (p.align & alTop) != 0);
		Menu_SetChecked(cont.MIAlignClient, (p.align & alClient) != 0);
		Menu_SetChecked(cont.MIAlignBottom, (p.align & alBottom) != 0);

		Menu_SetChecked(cont.MIColl, !p.collapsed);
		Menu_EnableItem(cont.MIColl, p.visible && !p.Locked && pos != CLUIFramesGetalClientFrame());
	}
	return 0;
}

INT_PTR CLUIFramesModifyMainMenuItems(WPARAM frameId, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos(frameId);

	if (pos >= 0 && pos < nFramescount) {
		FRAMEWND &p = Frames[pos];
		Menu_ModifyItem(p.MenuHandles.MITitle, p.TitleBar.tbname ? p.TitleBar.tbname : p.name);

		Menu_SetChecked(p.MenuHandles.MIVisible, p.visible);
		Menu_SetChecked(p.MenuHandles.MILock, p.Locked);
		Menu_SetChecked(p.MenuHandles.MITBVisible, p.TitleBar.ShowTitleBar);
		Menu_SetChecked(p.MenuHandles.MIFloating, p.floating);
		Menu_SetChecked(p.MenuHandles.MIBorder, p.UseBorder);
		Menu_SetChecked(p.MenuHandles.MISkinned, p.Skinned);

		Menu_EnableItem(p.MenuHandles.MIAlignTop, (p.align & alClient) == 0);
		Menu_SetChecked(p.MenuHandles.MIAlignTop, (p.align & alTop) != 0);

		Menu_SetChecked(p.MenuHandles.MIAlignClient, (p.align & alClient) != 0);

		Menu_EnableItem(p.MenuHandles.MIAlignTop, (p.align & alClient) == 0);
		Menu_SetChecked(p.MenuHandles.MIAlignTop, (p.align & alBottom) != 0);

		Menu_SetChecked(p.MenuHandles.MIColl, !p.collapsed);
		Menu_EnableItem(p.MenuHandles.MIColl, p.visible && !p.Locked && pos != CLUIFramesGetalClientFrame());
	}
	return 0;
}

INT_PTR CLUIFramesGetFrameOptions(WPARAM wParam, LPARAM)
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
		return (INT_PTR)Frames[pos].floating;

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

			if (!(flag & F_SKINNED))
				style &= ~CLS_SKINNEDFRAME;

			SetWindowLongPtr(Frames[pos].hWnd, GWL_STYLE, (LONG_PTR)style);
			SetWindowLongPtr(Frames[pos].TitleBar.hwnd, GWL_STYLE, (LONG_PTR)style & ~(WS_VSCROLL | WS_HSCROLL));
			lck.unlock();

			CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
			SetWindowPos(Frames[pos].TitleBar.hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
		return 0;

	case FO_NAME:
		if (lParam == 0)
			return -1;

		mir_free(Frames[pos].name);
		Frames[pos].name = (wParam & FO_UNICODETEXT) ? mir_wstrdup((LPWSTR)lParam) : mir_a2u((LPSTR)lParam);
		return 0;

	case FO_TBNAME:
		if (lParam == 0)
			return -1;

		mir_free(Frames[pos].TitleBar.tbname);
		Frames[pos].TitleBar.tbname = (wParam & FO_UNICODETEXT) ? mir_wstrdup((LPWSTR)lParam) : mir_a2u((LPSTR)lParam);
		lck.unlock();

		if (Frames[pos].floating && (Frames[pos].TitleBar.tbname != nullptr))
			SetWindowText(Frames[pos].ContainerWnd, Frames[pos].TitleBar.tbname);
		return 0;

	case FO_TBTIPNAME:
		if (lParam == 0)
			return -1;

		mir_free(Frames[pos].TitleBar.tooltip);
		Frames[pos].TitleBar.tooltip = (wParam & FO_UNICODETEXT) ? mir_wstrdup((LPWSTR)lParam) : mir_a2u((LPSTR)lParam);
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
		if (Frames[pos].collapsed) {
			int oldHeight = Frames[pos].height;
			retval = Frames[pos].height;
			Frames[pos].height = lParam;
			if (!CLUIFramesFitInSize())
				Frames[pos].height = retval;
			retval = Frames[pos].height;

			if (Frames[pos].height != oldHeight) {
				CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
				if (Frames[pos].Skinned)
					RedrawWindow(Frames[pos].hWnd, nullptr, nullptr, RDW_FRAME | RDW_UPDATENOW | RDW_INVALIDATE);
			}
		}
		else {
			retval = Frames[pos].HeightWhenCollapsed;
			Frames[pos].HeightWhenCollapsed = lParam;
			if (!CLUIFramesFitInSize())
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
		if (!(lParam&alTop || lParam&alBottom || lParam&alClient))
			return -1;

		if ((lParam&alClient) && (CLUIFramesGetalClientFrame() >= 0)) {  //only one alClient frame possible
			alclientFrame = -1;//recalc it
			return -1;
		}
		Frames[pos].align = lParam;
		return 0;
	}
	lck.unlock();

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return -1;
}

static INT_PTR CLUIFramesShowAll(WPARAM, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i = 0; i < nFramescount; i++)
		Frames[i].visible = TRUE;

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	return 0;
}

INT_PTR CLUIFramesShowAllTitleBars(WPARAM, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		F.TitleBar.ShowTitleBar = TRUE;
		SetWindowPos(F.hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

INT_PTR CLUIFramesHideAllTitleBars(WPARAM, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		F.TitleBar.ShowTitleBar = FALSE;
		SetWindowPos(F.hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

INT_PTR CLUIFramesShowHideFrame(WPARAM frameId, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	int pos;
	{
		mir_cslock lck(csFrameHook);
		pos = id2pos(frameId);
		if (pos >= 0 && !mir_wstrcmp(Frames[pos].name, L"My contacts"))
			Frames[pos].visible = 1;
		else {
			if (pos >= 0 && (int)pos < nFramescount)
				Frames[pos].visible = !Frames[pos].visible;
			if (Frames[pos].floating)
				CLUIFrameResizeFloatingFrame(pos);
		}
	}

	if (!Frames[pos].floating)
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

INT_PTR CLUIFramesShowHideFrameTitleBar(WPARAM frameId, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	{
		mir_cslock lck(csFrameHook);
		int pos = id2pos(frameId);
		if (pos >= 0 && (int)pos < nFramescount) {
			Frames[pos].TitleBar.ShowTitleBar = !Frames[pos].TitleBar.ShowTitleBar;
			SetWindowPos(Frames[pos].hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
	}

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

// lparam=-1 up ,1 down
INT_PTR CLUIFramesMoveUpDown(WPARAM frameId, LPARAM lParam)
{
	int i, tmpval;

	if (FramesSysNotStarted)
		return -1;

	mir_cslockfull lck(csFrameHook);
	int pos = id2pos(frameId);
	if (pos < 0 || pos >= nFramescount)
		return 0;

	int curalign = Frames[pos].align;
	int v = 0;
	memset(g_sd, 0, sizeof(SortData) * MAX_FRAMES);
	for (i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		if (F.floating || (!F.visible) || (F.align != curalign))
			continue;
		g_sd[v].order = F.order;
		g_sd[v].realpos = i;
		v++;
	}
	if (v == 0)
		return 0;

	qsort(g_sd, v, sizeof(SortData), sortfunc);
	for (i = 0; i < v; i++) {
		if (g_sd[i].realpos == pos) {
			if (lParam == -1) {
				if (i < 1) break;
				tmpval = Frames[g_sd[i - 1].realpos].order;
				Frames[g_sd[i - 1].realpos].order = Frames[pos].order;
				Frames[pos].order = tmpval;
				break;
			}
			if (lParam == 1) {
				if (i > v - 1) break;
				tmpval = Frames[g_sd[i + 1].realpos].order;
				Frames[g_sd[i + 1].realpos].order = Frames[pos].order;
				Frames[pos].order = tmpval;
				break;
			}
		}
	}
	lck.unlock();

	CLUIFramesReSort();
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
	return 0;
}

static INT_PTR CLUIFramesMoveUp(WPARAM frameId, LPARAM)
{
	return CLUIFramesMoveUpDown(frameId, -1);
}

static INT_PTR CLUIFramesMoveDown(WPARAM frameId, LPARAM)
{
	return CLUIFramesMoveUpDown(frameId, 1);
}

//lparam=alignment
INT_PTR CLUIFramesSetAlign(WPARAM frameId, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;

	CLUIFramesSetFrameOptions(MAKEWPARAM(FO_ALIGN, frameId), lParam);
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

INT_PTR CLUIFramesSetAlignalTop(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted) return -1;

	return CLUIFramesSetAlign(wParam, alTop);
}

INT_PTR CLUIFramesSetAlignalBottom(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted) return -1;

	return CLUIFramesSetAlign(wParam, alBottom);
}

INT_PTR CLUIFramesSetAlignalClient(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted) return -1;

	return CLUIFramesSetAlign(wParam, alClient);
}

//wparam=frameid
INT_PTR CLUIFramesLockUnlockFrame(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	mir_cslock lck(csFrameHook);
	int pos = id2pos(wParam);
	if (pos >= 0 && (int)pos < nFramescount) {
		Frames[pos].Locked = !Frames[pos].Locked;
		CLUIFramesStoreFrameSettings(pos);
	}
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesSetUnSetBorder(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	HWND hw;
	int FrameId, oldflags;
	{
		mir_cslock lck(csFrameHook);
		FrameId = id2pos(wParam);
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
	SetWindowPos(hw, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_NOZORDER);
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesSetUnSetSkinned(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	HWND hw;
	int FrameId, oldflags;
	{
		mir_cslock lck(csFrameHook);
		FrameId = id2pos(wParam);
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
	SetWindowPos(hw, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_NOZORDER);
	return 0;
}

//wparam=frameid
INT_PTR CLUIFramesCollapseUnCollapseFrame(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	TitleBarH = cfg::dat.titleBarHeight;

	mir_cslockfull lck(csFrameHook);
	int FrameId = id2pos(wParam);
	if (FrameId < 0 || FrameId >= nFramescount)
		return -1;

	int oldHeight;

	// do not collapse/uncollapse client/locked/invisible frames
	if (Frames[FrameId].align == alClient && !(Frames[FrameId].Locked || (!Frames[FrameId].visible) || Frames[FrameId].floating)) {
		RECT rc;
		if (Clist_IsDocked())
			return 0;

		if (db_get_b(0, "CLUI", "AutoSize", 0))
			return 0;

		GetWindowRect(g_clistApi.hwndContactList, &rc);

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

		SetWindowPos(g_clistApi.hwndContactList, nullptr, 0, 0, rc.right - rc.left, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

		CLUIFramesStoreAllFrames();
		lck.unlock();
		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
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
					FRAMEWND &F = Frames[i];
					if ((F.align != alClient) && (!F.floating) && (F.visible) && (!F.needhide)) {
						sumheight += (F.height) + (TitleBarH * btoint(F.TitleBar.ShowTitleBar)) + 2;
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
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	else {
		RECT contwnd;
		GetWindowRect(Frames[FrameId].ContainerWnd, &contwnd);
		contwnd.top = contwnd.bottom - contwnd.top;//height
		contwnd.left = contwnd.right - contwnd.left;//width

		contwnd.top -= (oldHeight - Frames[FrameId].height);//newheight
		SetWindowPos(Frames[FrameId].ContainerWnd, HWND_TOP, 0, 0, contwnd.left, contwnd.top, SWP_SHOWWINDOW | SWP_NOMOVE);
	}
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	CLUIFramesStoreAllFrames();
	return 0;
}

static int CLUIFramesLoadMainMenu()
{
	if (FramesSysNotStarted)
		return -1;

	for (auto &it : g_frameMenus)
		Menu_RemoveItem(it);
	g_frameMenus.destroy();

	// create frames menu
	CMenuItem mi(&g_plugin);
	mi.root = cont.MainMenuItem;
	mi.flags = CMIF_UNICODE | CMIF_SYSTEM;
	int separator = (int)3000200000;
	for (int i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		mi.hIcolibItem = F.TitleBar.hicon;
		mi.position = separator;
		mi.name.w = F.TitleBar.tbname ? F.TitleBar.tbname : F.name;
		mi.pszService = nullptr;
		g_frameMenus.insert(F.MenuHandles.MainMenuItem = Menu_AddMainMenuItem(&mi));
		CLUIFramesCreateMenuForFrame(F.id, F.MenuHandles.MainMenuItem, separator, true);
		CLUIFramesModifyMainMenuItems(F.id, 0);
		CallService(MS_CLIST_FRAMEMENUNOTIFY, (WPARAM)F.id, (LPARAM)F.MenuHandles.MainMenuItem);
		separator++;
	}
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

	memset(&ti, 0, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.lpszText = Frames[framepos].TitleBar.tooltip;
	ti.hinst = g_plugin.getInst();
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId = (UINT_PTR)Frames[framepos].TitleBar.hwnd;

	return SendMessage(Frames[framepos].TitleBar.hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
};

int FrameNCPaint(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar)
{
	RECT rcWindow, rc;
	HWND hwndParent = GetParent(hwnd);
	LRESULT result = 0;

	if (hwndParent != g_clistApi.hwndContactList || !cfg::dat.bSkinnedScrollbar)
		result = CallWindowProc(oldWndProc, hwnd, WM_NCPAINT, wParam, lParam);
	if (!g_clistApi.hwndContactList || hwndParent != g_clistApi.hwndContactList)
		return result;

	if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SKINNEDFRAME) {
		StatusItems_t *item = (arStatusItems.getCount() != 0) ? (hasTitleBar ? arStatusItems[ID_EXTBKOWNEDFRAMEBORDERTB - ID_STATUS_OFFLINE] : arStatusItems[ID_EXTBKOWNEDFRAMEBORDER - ID_STATUS_OFFLINE]) : nullptr;
		if (item == nullptr)
			return 0;

		GetWindowRect(hwnd, &rcWindow);
		rc.left = rc.top = 0;
		rc.right = rcWindow.right - rcWindow.left;
		rc.bottom = rcWindow.bottom - rcWindow.top;

		HDC hdc = GetWindowDC(hwnd);
		if (hwnd == g_clistApi.hwndContactTree) {
			HDC realDC = CreateCompatibleDC(hdc);
			HBITMAP hbmDraw = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			HBITMAP hbmOld = reinterpret_cast<HBITMAP>(SelectObject(realDC, hbmDraw));

			ExcludeClipRect(realDC, item->MARGIN_LEFT, item->MARGIN_TOP, rc.right - item->MARGIN_RIGHT, rc.bottom - item->MARGIN_BOTTOM);
			BitBlt(realDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, cfg::dat.hdcBg, rcWindow.left - cfg::dat.ptW.x, rcWindow.top - cfg::dat.ptW.y, SRCCOPY);
			DrawAlpha(realDC, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);

			ExcludeClipRect(hdc, item->MARGIN_LEFT, item->MARGIN_TOP, rc.right - item->MARGIN_RIGHT, rc.bottom - item->MARGIN_BOTTOM);
			BitBlt(hdc, 0, 0, rc.right, rc.bottom, realDC, 0, 0, SRCCOPY);
			SelectObject(realDC, hbmOld);
			DeleteObject(hbmDraw);
			DeleteDC(realDC);
		}
		else {
			ExcludeClipRect(hdc, item->MARGIN_LEFT, item->MARGIN_TOP, rc.right - item->MARGIN_RIGHT, rc.bottom - item->MARGIN_BOTTOM);
			BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, cfg::dat.hdcBg, rcWindow.left - cfg::dat.ptW.x, rcWindow.top - cfg::dat.ptW.y, SRCCOPY);
			DrawAlpha(hdc, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
		}
		ReleaseDC(hwnd, hdc);
		return 0;
	}

	if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_BORDER) {
		HDC hdc = GetWindowDC(hwnd);
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

	return result;
}

int FrameNCCalcSize(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar)
{
	StatusItems_t *item = (arStatusItems.getCount() != 0) ? (hasTitleBar ? arStatusItems[ID_EXTBKOWNEDFRAMEBORDERTB - ID_STATUS_OFFLINE] : arStatusItems[ID_EXTBKOWNEDFRAMEBORDER - ID_STATUS_OFFLINE]) : nullptr;
	LRESULT orig = oldWndProc ? CallWindowProc(oldWndProc, hwnd, WM_NCCALCSIZE, wParam, lParam) : 0;
	NCCALCSIZE_PARAMS *nccp = (NCCALCSIZE_PARAMS *)lParam;
	uint32_t dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

	if (item == nullptr)
		return orig;

	if (item->IGNORED || !(dwStyle & CLS_SKINNEDFRAME) || GetParent(hwnd) != g_clistApi.hwndContactList)
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

	WNDPROC oldWndProc = nullptr;
	BOOL    hasTitleBar = FALSE;

	for (i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		if (F.hWnd == hwnd) {
			oldWndProc = F.wndProc;
			hasTitleBar = F.TitleBar.ShowTitleBar;
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
		FRAMEWND &F = Frames[i];
		if (F.align != alTop)
			continue;
		g_sd[v].order = F.order;
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
		FRAMEWND &F = Frames[i];
		if (F.align != alBottom)
			continue;
		g_sd[v].order = F.order;
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
	CLISTFrame *clfrm = (CLISTFrame *)wParam;

	if (g_clistApi.hwndContactList == nullptr) return -1;
	if (FramesSysNotStarted) return -1;
	if (clfrm->cbSize != sizeof(CLISTFrame)) return -1;

	mir_cslockfull lck(csFrameHook);
	if (nFramescount >= MAX_FRAMES)
		return -1;

	if (Frames == nullptr) {
		Frames = (FRAMEWND*)malloc(sizeof(FRAMEWND) * (MAX_FRAMES + 2));
		memset(Frames, 0, (sizeof(FRAMEWND) * (MAX_FRAMES + 2)));
	}
	memset(&Frames[nFramescount], 0, sizeof(FRAMEWND));

	Frames[nFramescount].id = NextFrameId++;
	Frames[nFramescount].align = clfrm->align;
	Frames[nFramescount].hWnd = clfrm->hWnd;
	Frames[nFramescount].height = clfrm->height;
	Frames[nFramescount].TitleBar.hicon = clfrm->hIcon;
	Frames[nFramescount].floating = false;
	Frames[nFramescount].pPlugin = (HPLUGIN)lParam;

	if (clfrm->Flags & F_NO_SUBCONTAINER)
		Frames[nFramescount].OwnerWindow = (HWND)-2;
	else
		Frames[nFramescount].OwnerWindow = g_clistApi.hwndContactList;

	SetClassLong(clfrm->hWnd, GCL_STYLE, GetClassLong(clfrm->hWnd, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));
	SetWindowLongPtr(clfrm->hWnd, GWL_STYLE, GetWindowLongPtr(clfrm->hWnd, GWL_STYLE) | WS_CLIPCHILDREN);

	if (GetCurrentThreadId() == GetWindowThreadProcessId(clfrm->hWnd, nullptr)) {
		if (clfrm->hWnd != g_clistApi.hwndContactTree && clfrm->hWnd != g_hwndViewModeFrame && clfrm->hWnd != g_hwndEventArea) {
			Frames[nFramescount].wndProc = (WNDPROC)GetWindowLongPtr(clfrm->hWnd, GWLP_WNDPROC);
			SetWindowLongPtr(clfrm->hWnd, GWLP_WNDPROC, (LONG_PTR)FramesSubClassProc);
		}
	}

	if (clfrm->hWnd == g_hwndEventArea)
		wndFrameEventArea = &Frames[nFramescount];
	else if (clfrm->hWnd == g_clistApi.hwndContactTree)
		wndFrameCLC = &Frames[nFramescount];
	else if (clfrm->hWnd == g_hwndViewModeFrame)
		wndFrameViewMode = &Frames[nFramescount];

	Frames[nFramescount].dwFlags = clfrm->Flags;

	if (clfrm->szName.a == nullptr || ((clfrm->Flags & F_UNICODE) ? mir_wstrlen(clfrm->szName.w) : mir_strlen(clfrm->szName.a)) == 0) {
		wchar_t ptszClassName[256];
		GetClassName(Frames[nFramescount].hWnd, ptszClassName, _countof(ptszClassName));
		Frames[nFramescount].name = mir_wstrdup(ptszClassName);
	}
	else Frames[nFramescount].name = (clfrm->Flags & F_UNICODE) ? mir_wstrdup(clfrm->szName.w) : mir_a2u(clfrm->szName.a);

	if (IsBadCodePtr((FARPROC)clfrm->szTBname.a) || clfrm->szTBname.a == nullptr
		|| ((clfrm->Flags & F_UNICODE) ? mir_wstrlen(clfrm->szTBname.w) : mir_strlen(clfrm->szTBname.a)) == 0)
		Frames[nFramescount].TitleBar.tbname = mir_wstrdup(Frames[nFramescount].name);
	else
		Frames[nFramescount].TitleBar.tbname = (clfrm->Flags & F_UNICODE) ? mir_wstrdup(clfrm->szTBname.w) : mir_a2u(clfrm->szTBname.a);
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
		(db_get_b(0, CLUIFrameModule, "RemoveAllTitleBarBorders", 1) ? 0 : WS_BORDER)
		| WS_CHILD | WS_CLIPCHILDREN | (Frames[nFramescount].TitleBar.ShowTitleBar ? WS_VISIBLE : 0) |
		WS_CLIPCHILDREN, 0, 0, 0, 0, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);

	SetWindowLongPtr(Frames[nFramescount].TitleBar.hwnd, GWLP_USERDATA, Frames[nFramescount].id);

	Frames[nFramescount].TitleBar.hwndTip = CreateWindowExA(0, TOOLTIPS_CLASSA, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);

	SetWindowPos(Frames[nFramescount].TitleBar.hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	{
		TOOLINFOA ti = { 0 };
		ti.cbSize = sizeof(ti);
		ti.lpszText = "";
		ti.hinst = g_plugin.getInst();
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
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);

	if (Frames[nFramescount - 1].floating) {
		Frames[nFramescount - 1].floating = FALSE;
		CLUIFrameSetFloat(retval, 1);//lparam=1 use stored width and height
	}
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return retval;
}

static INT_PTR CLUIFramesRemoveFrame(WPARAM wParam, LPARAM)
{
	if (FramesSysNotStarted)
		return -1;

	{
		mir_cslock lck(csFrameHook);
		int pos = id2pos(wParam);
		if (pos < 0 || pos > nFramescount)
			return -1;

		FRAMEWND* F = &Frames[pos];
		if (F->hWnd == g_hwndEventArea)
			wndFrameEventArea = nullptr;
		else if (F->hWnd == g_clistApi.hwndContactTree)
			wndFrameCLC = nullptr;
		else if (F->hWnd == g_hwndViewModeFrame)
			wndFrameViewMode = nullptr;

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
		InvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	return 0;
}

INT_PTR CLUIFramesForceUpdateTB(const FRAMEWND *Frame)
{
	if (Frame->TitleBar.hwnd != nullptr)
		RedrawWindow(Frame->TitleBar.hwnd, nullptr, nullptr, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	return 0;
}

INT_PTR CLUIFramesForceUpdateFrame(const FRAMEWND *Frame)
{
	if (Frame->hWnd != nullptr)
		RedrawWindow(Frame->hWnd, nullptr, nullptr, RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_INVALIDATE);

	if (Frame->floating)
		if (Frame->ContainerWnd != nullptr)  RedrawWindow(Frame->ContainerWnd, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
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

	SetWindowPos(Frame->hWnd, nullptr, Frame->wndSize.left + cfg::dat.bCLeft, Frame->wndSize.top + cfg::dat.topOffset,
		(Frame->wndSize.right - Frame->wndSize.left),
		(Frame->wndSize.bottom - Frame->wndSize.top), SWP_NOZORDER | SWP_NOREDRAW);
	if (Frame->TitleBar.ShowTitleBar) {
		SetWindowPos(Frame->TitleBar.hwnd, nullptr, Frame->wndSize.left + cfg::dat.bCLeft, Frame->wndSize.top + cfg::dat.topOffset - TitleBarH,
			(Frame->wndSize.right - Frame->wndSize.left),
			TitleBarH + (Frame->UseBorder ? (!Frame->collapsed ? (Frame->align == alClient ? 0 : 2) : 1) : 0), SWP_NOZORDER);
	}
	return 0;
}

bool CLUIFramesFitInSize(void)
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
		FRAMEWND &F = Frames[i];
		if ((F.align != alClient) && (!F.floating) && (F.visible) && (!F.needhide)) {
			sumheight += (F.height) + (TitleBarH * btoint(F.TitleBar.ShowTitleBar)) + 2/*+btoint(F.UseBorder)*2*/;
			if (sumheight > ContactListHeight - tbh - 2)
				return FALSE;
		}
	}
	return TRUE;
}

int CLUIFramesGetMinHeight()
{
	if (g_clistApi.hwndContactList == nullptr)
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
			FRAMEWND &F = Frames[i];
			if ((F.align != alClient) && (F.visible) && (!F.needhide) && (!F.floating)) {
				RECT wsize;

				GetWindowRect(F.hWnd, &wsize);
				sumheight += (wsize.bottom - wsize.top) + (TitleBarH * btoint(F.TitleBar.ShowTitleBar)) + 3;
			}
		}
	}

	GetBorderSize(g_clistApi.hwndContactList, &border);
	return(sumheight + border.top + border.bottom + allbord + tbh + 3);
}

int SizeMoveNewSizes()
{
	for (int i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		if (F.floating)
			CLUIFrameResizeFloatingFrame(i);
		else
			CLUIFrameMoveResize(&F);
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
	int sumheight = 9999999;
	int clientframe = -1;
	int i, j;
	int topOff = 0, botOff = 0, last_bottomtop;

	GapBetweenFrames = cfg::dat.gapBetweenFrames;
	int sepw = GapBetweenFrames;

	if (nFramescount < 1 || cfg::shutDown)
		return 0;

	int newheight = newsize.bottom - newsize.top;
	TitleBarH = cfg::dat.titleBarHeight;

	// search for alClient frame and get the titlebar's height
	int tbh = 0;
	int clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = (TitleBarH)* btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

	for (i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		if (!F.floating) {
			F.needhide = FALSE;
			F.wndSize.left = 0;
			F.wndSize.right = newsize.right - newsize.left;
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
	int drawitems = nFramescount;
	while (sumheight >(newheight - tbh) && drawitems > 0) {
		sumheight = 0;
		drawitems = 0;
		for (i = 0; i < nFramescount; i++) {
			FRAMEWND &F = Frames[i];
			if (((F.align != alClient)) && (!F.floating) && (F.visible) && (!F.needhide)) {
				drawitems++;
				int curfrmtbh = (TitleBarH)* btoint(F.TitleBar.ShowTitleBar);
				sumheight += (F.height) + curfrmtbh + (i > 0 ? sepw : 0) + (F.UseBorder ? 2 : 0);
				if (sumheight > newheight - tbh) {
					sumheight -= (F.height) + curfrmtbh + (i > 0 ? sepw : 0);
					F.needhide = TRUE;
					drawitems--;
					break;
				}
			}
		}
	}

	int prevframe = -1;
	int prevframebottomline = 0;
	for (j = 0; j < nFramescount; j++) {
		// move all alTop frames
		i = g_sd[j].realpos;
		FRAMEWND &F = Frames[i];
		if ((!F.needhide) && (!F.floating) && (F.visible) && (F.align == alTop)) {
			int curfrmtbh = (TitleBarH)* btoint(F.TitleBar.ShowTitleBar);
			F.wndSize.top = prevframebottomline + (prevframebottomline > 0 ? sepw : 0) + (curfrmtbh);
			F.wndSize.bottom = F.height + F.wndSize.top + (F.UseBorder ? 2 : 0);
			F.prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = F.wndSize.bottom;
			topOff = prevframebottomline;
		}
	}

	if (sumheight < newheight) {
		for (j = 0; j < nFramescount; j++) {
			// move alClient frame
			i = g_sd[j].realpos;
			FRAMEWND &F = Frames[i];
			if ((!F.needhide) && (!F.floating) && (F.visible) && (F.align == alClient)) {
				int oldh;
				F.wndSize.top = prevframebottomline + (prevframebottomline > 0 ? sepw : 0) + (tbh);
				F.wndSize.bottom = F.wndSize.top + newheight - sumheight - tbh - ((prevframebottomline > 0) ? sepw : 0);
				clientframe = i;
				oldh = F.height;
				F.height = F.wndSize.bottom - F.wndSize.top;
				F.prevvisframe = prevframe;
				prevframe = i;
				prevframebottomline = F.wndSize.bottom;
				if (prevframebottomline > newheight) {
					// prevframebottomline-=F.height+(tbh+1);
					// F.needhide=TRUE;
				}
				break;
			}
		}
	}

	// newheight
	prevframebottomline = last_bottomtop = newheight;
	for (j = nFramescount - 1; j >= 0; j--) {
		// move all alBottom frames
		i = g_sd[j].realpos;
		FRAMEWND &F = Frames[i];
		if ((F.visible) && (!F.floating) && (!F.needhide) && (F.align == alBottom)) {
			int curfrmtbh = (TitleBarH)* btoint(F.TitleBar.ShowTitleBar);
			F.wndSize.bottom = prevframebottomline - ((prevframebottomline < newheight) ? sepw : 0);
			F.wndSize.top = F.wndSize.bottom - F.height - (F.UseBorder ? 2 : 0);
			F.prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = F.wndSize.top - curfrmtbh;
			botOff = prevframebottomline;
			last_bottomtop = F.wndSize.top - curfrmtbh;
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
	if (FramesSysNotStarted)
		return -1;

	if (wParam == -1) {
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
		return 0;
	}

	if (lParam & FU_FMPOS)
		CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 1);

	mir_cslock lck(csFrameHook);
	int pos = id2pos(wParam);
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
		FRAMEWND &F = Frames[i];
		if ((mode == 1 && F.OwnerWindow != (HWND)-2 && F.OwnerWindow) ||
			(mode == 2 && F.OwnerWindow == (HWND)-2) || (mode == 3))
			if (F.floating)
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
	if (FramesSysNotStarted)
		return -1;

	TitleBarH = cfg::dat.titleBarHeight;

	mir_cslock lck(csFrameHook);
	GapBetweenFrames = cfg::dat.gapBetweenFrames;

	RECT nRect = *r;
	nRect.bottom -= (cfg::dat.statusBarHeight + cfg::dat.bottomOffset);
	nRect.right -= cfg::dat.bCRight;
	nRect.left = cfg::dat.bCLeft;
	nRect.top = cfg::dat.topOffset;
	ContactListHeight = nRect.bottom - nRect.top;

	CLUIFramesResize(nRect);
	{
		int i;
		for (i = 0; i < nFramescount; i++) {
			FRAMEWND &F = Frames[i];
			if (!F.floating) {
				if (F.OwnerWindow && F.OwnerWindow != (HWND)-2) {
					SetWindowPos(F.hWnd, nullptr, F.wndSize.left + cfg::dat.bCLeft, F.wndSize.top + cfg::dat.topOffset,
						(F.wndSize.right - F.wndSize.left),
						(F.wndSize.bottom - F.wndSize.top), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);

					if (F.TitleBar.ShowTitleBar) {
						SetWindowPos(F.TitleBar.hwnd, nullptr, F.wndSize.left + cfg::dat.bCLeft, F.wndSize.top + cfg::dat.topOffset - TitleBarH,
							(F.wndSize.right - F.wndSize.left),
							TitleBarH + (F.UseBorder ? (!F.collapsed ? (F.align == alClient ? 0 : 2) : 1) : 0), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
					}
				}
				else {
					// set frame position
					SetWindowPos(F.hWnd, nullptr, F.wndSize.left + cfg::dat.bCLeft, F.wndSize.top + cfg::dat.topOffset,
						(F.wndSize.right - F.wndSize.left),
						(F.wndSize.bottom - F.wndSize.top), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOCOPYBITS | SWP_NOREDRAW);

					// set titlebar position
					if (F.TitleBar.ShowTitleBar) {
						SetWindowPos(F.TitleBar.hwnd, nullptr, F.wndSize.left + cfg::dat.bCLeft, F.wndSize.top + cfg::dat.topOffset - TitleBarH,
							(F.wndSize.right - F.wndSize.left),
							TitleBarH + (F.UseBorder ? (!F.collapsed ? (F.align == alClient ? 0 : 2) : 1) : 0), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
					}
					if (F.TitleBar.ShowTitleBar)
						UpdateWindow(F.TitleBar.hwnd);
				}
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

		GetClientRect(g_clistApi.hwndContactList, &nRect);
		if (lParam && lParam != 1) {
			RECT oldRect;
			POINT pt;
			RECT * newRect = (RECT *)lParam;
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

	if (g_clistApi.hwndContactList != nullptr)
		InvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
	if (g_clistApi.hwndContactList != nullptr)
		UpdateWindow(g_clistApi.hwndContactList);

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
	AlignCOLLIconToLeft = db_get_b(0, "FrameTitleBar", "AlignCOLLIconToLeft", 0);
	bkColour = db_get_dw(0, "FrameTitleBar", "BkColour", CLCDEFAULT_BKCOLOUR);

	if (hBmpBackground) {
		DeleteObject(hBmpBackground);
		hBmpBackground = nullptr;
	}
	if (db_get_b(0, "FrameTitleBar", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
		ptrW tszBitmapName(db_get_wsa(0, "FrameTitleBar", "BkBitmap"));
		if (tszBitmapName != NULL)
			hBmpBackground = Bitmap_Load(tszBitmapName);
	}
	backgroundBmpUse = db_get_w(0, "FrameTitleBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);

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

			const wchar_t *pwszTitle = TranslateW_LP(Frames[pos].TitleBar.tbname, Frames[pos].pPlugin);
			int iTitleLen = (int)mir_wstrlen(pwszTitle);

			if (!AlignCOLLIconToLeft) {
				if (Frames[pos].TitleBar.hicon != nullptr) {
					DrawIconEx(hdcMem, 6 + cfg::dat.bClipBorder, ((TitleBarH >> 1) - 8), Frames[pos].TitleBar.hicon, 16, 16, 0, nullptr, DI_NORMAL);
					TextOut(hdcMem, 24 + cfg::dat.bClipBorder, fontTop, pwszTitle, iTitleLen);
				}
				else TextOut(hdcMem, 6 + cfg::dat.bClipBorder, fontTop, pwszTitle, iTitleLen);
			}
			else TextOut(hdcMem, 18 + cfg::dat.bClipBorder, fontTop, pwszTitle, iTitleLen);

			if (!AlignCOLLIconToLeft)
				DrawIconEx(hdcMem, Frames[pos].TitleBar.wndSize.right - 22, ((TitleBarH >> 1) - 8), Frames[pos].collapsed ? Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN) : Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT), 16, 16, 0, nullptr, DI_NORMAL);
			else
				DrawIconEx(hdcMem, 0, ((TitleBarH >> 1) - 8), Frames[pos].collapsed ? Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN) : Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT), 16, 16, 0, nullptr, DI_NORMAL);
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
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		return Menu_DrawItem(lParam);

	case WM_ENABLE:
		if (hwnd != nullptr) InvalidateRect(hwnd, nullptr, FALSE);
		return 0;
	case WM_SIZE:
		return 0;

	case WM_COMMAND:
		if (Clist_MenuProcessCommand(LOWORD(wParam), 0, Frameid))
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
			CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
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
				AppendMenu(hmenu, MF_SEPARATOR, 16, L"");

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
			TrackPopupMenu(hmenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
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
		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
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
				if (db_get_b(0, "CLUI", "ClientAreaDrag", 0)) {
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
			mir_cslock lck(csFrameHook);
			int pos = id2pos(Frameid);
			if (pos != -1) {
				int oldflags;
				char TBcapt[255];
				mir_snprintf(TBcapt, "%s - h:%d, vis:%d, fl:%d, fl:(%d,%d,%d,%d),or: %d",
					Frames[pos].name, Frames[pos].height, Frames[pos].visible, Frames[pos].floating,
					Frames[pos].FloatingPos.x, Frames[pos].FloatingPos.y,
					Frames[pos].FloatingSize.x, Frames[pos].FloatingSize.y,
					Frames[pos].order);

				oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, Frames[pos].id), 0);
				if (!(oldflags & F_SHOWTBTIP))
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

				GetWindowRect(g_clistApi.hwndContactList, &rcMiranda);
				if (IsWindowVisible(g_clistApi.hwndContactList) && IntersectRect(&rcOverlap, &rcwnd, &rcMiranda)) {
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

				GetWindowRect(g_clistApi.hwndContactList, &rcMiranda);

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
						SetWindowPos(Frames[pos].ContainerWnd, nullptr, nLeft, nTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

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
				CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
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
	if (Frames[framepos].ContainerWnd == nullptr)
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
		InvalidateRect(Frames[framepos].TitleBar.hwnd, nullptr, FALSE);
		SetWindowPos(Frames[framepos].hWnd, HWND_TOP, 0, floatingHeight, width, height - floatingHeight, SWP_SHOWWINDOW | SWP_NOACTIVATE);

	}
	else {
		Frames[framepos].height = height;
		ShowWindow(Frames[framepos].TitleBar.hwnd, SW_HIDE);
		SetWindowPos(Frames[framepos].hWnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}

	if (Frames[framepos].ContainerWnd != nullptr)
		UpdateWindow(Frames[framepos].ContainerWnd);
	GetWindowRect(Frames[framepos].hWnd, &Frames[framepos].wndSize);

	if (Frames[framepos].TitleBar.ShowTitleBar)
		RedrawWindow(Frames[framepos].TitleBar.hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);

	RedrawWindow(Frames[framepos].hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
	return 0;
}

static int CLUIFrameOnMainMenuBuild(WPARAM, LPARAM)
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

			if (Frames[framepos].ContainerWnd == nullptr)
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

			if (Frames[framepos].ContainerWnd == nullptr)
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

			if (Frames[framepos].ContainerWnd == nullptr)
				return 0;

			CLUIFrameResizeFloatingFrame(framepos);

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
	return(CreateWindowA("FramesContainer", "aaaa", WS_POPUP | WS_THICKFRAME, x, y, width, height, parent, nullptr, g_plugin.getInst(), nullptr));
}

INT_PTR CLUIFrameSetFloat(WPARAM wParam, LPARAM lParam)
{
	HWND hwndtmp, hwndtooltiptmp;
	{
		mir_cslock lck(csFrameHook);
		wParam = id2pos(wParam);
		if ((int)wParam >= 0 && (int)wParam < nFramescount) {
			if (Frames[wParam].floating) {
				SetParent(Frames[wParam].hWnd, g_clistApi.hwndContactList);
				SetParent(Frames[wParam].TitleBar.hwnd, g_clistApi.hwndContactList);
				Frames[wParam].floating = FALSE;
				DestroyWindow(Frames[wParam].ContainerWnd);
				Frames[wParam].ContainerWnd = nullptr;
			}
			else {
				RECT recttb, rectw, border;
				int temp;
				int neww, newh;

				Frames[wParam].oldstyles = GetWindowLongPtr(Frames[wParam].hWnd, GWL_STYLE);
				Frames[wParam].TitleBar.oldstyles = GetWindowLongPtr(Frames[wParam].TitleBar.hwnd, GWL_STYLE);
				bool locked = Frames[wParam].Locked;
				Frames[wParam].Locked = FALSE;
				Frames[wParam].minmaxenabled = FALSE;

				GetWindowRect(Frames[wParam].hWnd, &rectw);
				GetWindowRect(Frames[wParam].TitleBar.hwnd, &recttb);
				if (!Frames[wParam].TitleBar.ShowTitleBar)
					recttb.top = recttb.bottom = recttb.left = recttb.right = 0;

				Frames[wParam].ContainerWnd = CreateContainerWindow(g_clistApi.hwndContactList, Frames[wParam].FloatingPos.x, Frames[wParam].FloatingPos.y, 10, 10);

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

	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	SendMessage(hwndtmp, WM_SIZE, 0, 0);
	SetWindowPos(hwndtooltiptmp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return 0;
}

wchar_t g_ptszEventName[100];

static int CLUIFrameOnModulesLoad(WPARAM, LPARAM)
{
	mir_snwprintf(g_ptszEventName, L"mf_update_evt_%d", GetCurrentThreadId());
	g_hEventThread = CreateEvent(nullptr, TRUE, FALSE, g_ptszEventName);
	hThreadMFUpdate = mir_forkthread(MF_UpdateThread);
	SetThreadPriority(hThreadMFUpdate, THREAD_PRIORITY_IDLE);
	CLUIFramesLoadMainMenu();
	CLUIFramesCreateMenuForFrame(-1, nullptr, 000010000, false);
	return 0;
}

static int CLUIFrameLangChanged(WPARAM, LPARAM)
{
	ApplyViewMode(0);
	g_clistApi.pfnInvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
	return 0;
}

static int CLUIFrameOnModulesUnload(WPARAM, LPARAM)
{
	mf_updatethread_running = FALSE;

	SetThreadPriority(hThreadMFUpdate, THREAD_PRIORITY_NORMAL);
	SetEvent(g_hEventThread);
	WaitForSingleObject(hThreadMFUpdate, 2000);
	CloseHandle(g_hEventThread);

	Menu_RemoveItem(cont.MIVisible);
	Menu_RemoveItem(cont.MITitle);
	Menu_RemoveItem(cont.MITBVisible);
	Menu_RemoveItem(cont.MILock);
	Menu_RemoveItem(cont.MIColl);
	Menu_RemoveItem(cont.MIFloating);
	Menu_RemoveItem(cont.MIAlignRoot);
	Menu_RemoveItem(cont.MIAlignTop);
	Menu_RemoveItem(cont.MIAlignClient);
	Menu_RemoveItem(cont.MIAlignBottom);
	Menu_RemoveItem(cont.MIBorder);
	return 0;
}

/*
 * wparam=hIcon
 * return hImage on success,-1 on failure
 */

int LoadCLUIFramesModule(void)
{
	GapBetweenFrames = cfg::dat.gapBetweenFrames;

	nFramescount = 0;

	WNDCLASS wndclass = {};
	wndclass.style = CS_DBLCLKS;
	wndclass.lpfnWndProc = CLUIFrameTitleBarProc;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.lpszClassName = CLUIFrameTitleBarClassName;
	RegisterClass(&wndclass);

	WNDCLASS cntclass = {};
	cntclass.style = CS_DBLCLKS | CS_DROPSHADOW;
	cntclass.lpfnWndProc = CLUIFrameContainerWndProc;
	cntclass.hInstance = g_plugin.getInst();
	cntclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	cntclass.lpszClassName = L"FramesContainer";
	RegisterClass(&cntclass);

	// create root menu
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x3931AC4, 0x7A32, 0x4D9C, 0x99, 0x92, 0x94, 0xD4, 0xB5, 0x9B, 0xD6, 0xB6);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FRAME);
	mi.position = 3000090000;
	mi.name.a = LPGEN("Frames");
	mi.pszService = nullptr;
	cont.MainMenuItem = Menu_AddMainMenuItem(&mi);
	UNSET_UID(mi);

	mi.root = cont.MainMenuItem;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
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
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HELP);
	mi.name.a = LPGEN("Show all title bars");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);

	// create "hide all titlebars" menu
	mi.uid.d[7]++;
	mi.position++;
	mi.name.a = LPGEN("Hide all title bars");
	mi.pszService = MS_CLIST_FRAMES_HIDEALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_SYSTEM_MODULESLOADED, CLUIFrameOnModulesLoad);
	HookEvent(ME_CLIST_PREBUILDFRAMEMENU, CLUIFramesModifyContextMenuForFrame);
	HookEvent(ME_CLIST_PREBUILDMAINMENU, CLUIFrameOnMainMenuBuild);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, CLUIFrameOnModulesUnload);
	HookEvent(ME_LANGPACK_CHANGED, CLUIFrameLangChanged);

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
	hWndExplorerToolBar = FindWindowExA(nullptr, nullptr, "Shell_TrayWnd", nullptr);
	OnFrameTitleBarBackgroundChange();

	FramesSysNotStarted = FALSE;
	g_hPenCLUIFrames = CreatePen(PS_SOLID, 1, db_get_dw(0, "CLUI", "clr_frameborder", GetSysColor(COLOR_3DDKSHADOW)));
	return 0;
}

void LoadExtraIconModule()
{
	hStatusBarShowToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP);
	hStatusBarHideToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP);
}

int UnLoadCLUIFramesModule(void)
{
	CLUIFramesOnClistResize((WPARAM)g_clistApi.hwndContactList, 0);
	CLUIFramesStoreAllFrames();
	DeleteObject(g_hPenCLUIFrames);

	mir_cslock lck(csFrameHook);
	FramesSysNotStarted = TRUE;
	for (int i = 0; i < nFramescount; i++) {
		FRAMEWND &F = Frames[i];
		DestroyWindow(F.hWnd);
		F.hWnd = (HWND)-1;
		DestroyWindow(F.TitleBar.hwnd);
		F.TitleBar.hwnd = (HWND)-1;
		DestroyWindow(F.ContainerWnd);
		F.ContainerWnd = (HWND)-1;
		DestroyMenu(F.TitleBar.hmenu);

		if (F.name != nullptr)
			mir_free(F.name);
		if (F.TitleBar.tbname != nullptr)
			mir_free(F.TitleBar.tbname);
	}
	free(Frames);
	Frames = nullptr;
	nFramescount = 0;
	UnregisterClass(CLUIFrameTitleBarClassName, g_plugin.getInst());
	return 0;
}
