/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "../commonheaders.h"

extern HINSTANCE g_hInst;

//we use dynamic frame list,
//but who wants so huge number of frames ??
#define MAX_FRAMES		16

#define UNCOLLAPSED_FRAME_SIZE		0
#define DEFAULT_TITLEBAR_HEIGHT		20

//legacy menu support
#define frame_menu_lock				1
#define frame_menu_visible			2
#define frame_menu_showtitlebar		3
#define frame_menu_floating			4
static int UpdateTBToolTip(int framepos);
INT_PTR CLUIFrameSetFloat(WPARAM wParam, LPARAM lParam);
int CLUIFrameResizeFloatingFrame(int framepos);
extern int InitFramesMenus(void);
int GapBetweenTitlebar;

LOGFONTA TitleBarLogFont = {0};

boolean FramesSysNotStarted = TRUE;

static int sortfunc(const void *a,const void *b)
{
	SortData *sd1,*sd2;
	sd1 = (SortData *)a;
	sd2 = (SortData *)b;
	if (sd1->order > sd2->order){return(1);}
	if (sd1->order < sd2->order){return(-1);}
	//if (sd1->order == sd2->order){return 0;}
	return 0;
}

//static FRAMEWND Frames[MAX_FRAMES];
static FRAMEWND *Frames = NULL;

static int nFramescount = 0;
static int alclientFrame = -1;//for fast access to frame with alclient properties
static int NextFrameId = 100;

HFONT TitleBarFont;
static int TitleBarH = DEFAULT_TITLEBAR_HEIGHT;
static boolean resizing = FALSE;

// menus
static HANDLE contMIVisible,contMITitle,contMITBVisible,contMILock,contMIColl,contMIFloating;
static HANDLE contMIAlignRoot;
static HANDLE contMIAlignTop,contMIAlignClient,contMIAlignBottom;
static HANDLE contMIBorder;
static HGENMENU MainMIRoot = (HGENMENU)-1;

// others
static int ContactListHeight;
static int LastStoreTick = 0;

static int lbypos = -1;
static int oldframeheight = -1;
static int curdragbar = -1;
static CRITICAL_SECTION csFrameHook;

static BOOLEAN CLUIFramesFitInSize(void);
static int RemoveItemFromList(int pos,FRAMEWND **lpFrames,int *FrameItemCount);
HWND hWndExplorerToolBar;
static int GapBetweenFrames = 1;

static int RemoveItemFromList(int pos,FRAMEWND **lpFrames,int *FrameItemCount)
{
	memcpy(&((*lpFrames)[pos]),&((*lpFrames)[pos+1]),sizeof(FRAMEWND)*(*FrameItemCount-pos-1));
	(*FrameItemCount)--;
	(*lpFrames) = (FRAMEWND*)realloc((*lpFrames),sizeof(FRAMEWND)*(*FrameItemCount));
	return 0;
}

static int id2pos(int id)
{
	if (FramesSysNotStarted)
		return -1;

	for (int i=0;i<nFramescount;i++)
		if (Frames[i].id == id)
			return i;

	return -1;
}

static int btoint(BOOLEAN b)
{
	if (b) return 1;
	return 0;
}

static void __inline lockfrm()
{
	EnterCriticalSection(&csFrameHook);
}

static void __inline ulockfrm()
{
	LeaveCriticalSection(&csFrameHook);
}

//////////screen docking,code  from "floating contacts" plugin.

static FRAMEWND* FindFrameByWnd( HWND hwnd )
{
	if ( hwnd == NULL )
		return NULL;

	for (int i=0; i < nFramescount; i++)
		if ( Frames[i].floating && Frames[i].ContainerWnd == hwnd )
			return &Frames[i];

	return NULL;
}

static void DockThumbs( FRAMEWND *pThumbLeft, FRAMEWND *pThumbRight, BOOL bMoveLeft )
{
	if ( pThumbRight->dockOpt.hwndLeft == NULL && pThumbLeft->dockOpt.hwndRight == NULL ) {
		pThumbRight->dockOpt.hwndLeft = pThumbLeft->ContainerWnd;
		pThumbLeft->dockOpt.hwndRight = pThumbRight->ContainerWnd;
	}
}

static void UndockThumbs( FRAMEWND *pThumb1, FRAMEWND *pThumb2 )
{
	if ( pThumb1 == NULL || pThumb2 == NULL )
		return;

	if ( pThumb1->dockOpt.hwndRight == pThumb2->ContainerWnd )
		pThumb1->dockOpt.hwndRight = NULL;

	if ( pThumb1->dockOpt.hwndLeft == pThumb2->ContainerWnd )
		pThumb1->dockOpt.hwndLeft = NULL;

	if ( pThumb2->dockOpt.hwndRight == pThumb1->ContainerWnd )
		pThumb2->dockOpt.hwndRight = NULL;

	if ( pThumb2->dockOpt.hwndLeft == pThumb1->ContainerWnd )
		pThumb2->dockOpt.hwndLeft = NULL;
}

BOOLEAN bMoveTogether;

static void PositionThumb( FRAMEWND *pThumb, short nX, short nY )
{
	FRAMEWND	*pCurThumb = &Frames[0];
	FRAMEWND	*pDockThumb = pThumb;
	FRAMEWND	fakeMainWindow;
	FRAMEWND	fakeTaskBarWindow;
	RECT     rc;
	RECT     rcThumb;
	RECT     rcOld;
	SIZE     sizeScreen;
	int      nNewX;
	int      nNewY;
	int      nOffs    = 10;
	int      nWidth;
	int      nHeight;
	POINT    pt;
	RECT     rcLeft;
	RECT     rcTop;
	RECT     rcRight;
	RECT     rcBottom;
	BOOL     bDocked;
	BOOL     bDockedLeft;
	BOOL     bDockedRight;
	BOOL     bLeading;
	int      frmidx = 0;

	if ( pThumb == NULL )
		return;

	sizeScreen.cx = GetSystemMetrics( SM_CXSCREEN );
	sizeScreen.cy = GetSystemMetrics( SM_CYSCREEN );

	// Get thumb dimnsions
	GetWindowRect( pThumb->ContainerWnd, &rcThumb );
	nWidth = rcThumb.right - rcThumb.left;
	nHeight = rcThumb.bottom - rcThumb.top;

	// Docking to the edges of the screen
	nNewX = nX < nOffs ? 0 : nX;
	nNewX = nNewX > ( sizeScreen.cx - nWidth - nOffs ) ? ( sizeScreen.cx - nWidth ) : nNewX;
	nNewY = nY < nOffs ? 0 : nY;
	nNewY = nNewY > ( sizeScreen.cy - nHeight - nOffs ) ? ( sizeScreen.cy - nHeight ) : nNewY;

	bLeading = pThumb->dockOpt.hwndRight != NULL;

	if ( bMoveTogether ) {
		UndockThumbs( pThumb,  FindFrameByWnd( pThumb->dockOpt.hwndLeft ));
		GetWindowRect( pThumb->ContainerWnd, &rcOld );
	}

	memset(&fakeMainWindow,0,sizeof(fakeMainWindow));
	fakeMainWindow.ContainerWnd = pcli->hwndContactList;
	fakeMainWindow.floating = TRUE;

	memset(&fakeTaskBarWindow,0,sizeof(fakeTaskBarWindow));
	fakeTaskBarWindow.ContainerWnd = hWndExplorerToolBar;
	fakeTaskBarWindow.floating = TRUE;

	while( pCurThumb != NULL ) {
		if (pCurThumb->floating) {
			if ( pCurThumb != pThumb ) {
				GetWindowRect( pThumb->ContainerWnd, &rcThumb );
				OffsetRect( &rcThumb, nX - rcThumb.left, nY - rcThumb.top );

				GetWindowRect( pCurThumb->ContainerWnd, &rc );

				// These are rects we will dock into
				rcLeft.left	 = rc.left - nOffs;
				rcLeft.top	 = rc.top - nOffs;
				rcLeft.right = rc.left + nOffs;
				rcLeft.bottom = rc.bottom + nOffs;

				rcTop.left	 = rc.left - nOffs;
				rcTop.top	 = rc.top - nOffs;
				rcTop.right	 = rc.right + nOffs;
				rcTop.bottom = rc.top + nOffs;

				rcRight.left = rc.right - nOffs;
				rcRight.top	 = rc.top - nOffs;
				rcRight.right = rc.right + nOffs;
				rcRight.bottom = rc.bottom + nOffs;

				rcBottom.left = rc.left - nOffs;
				rcBottom.top = rc.bottom - nOffs;
				rcBottom.right = rc.right + nOffs;
				rcBottom.bottom = rc.bottom + nOffs;


				bDockedLeft	 = FALSE;
				bDockedRight = FALSE;

				// Upper-left
				pt.x = rcThumb.left;
				pt.y = rcThumb.top;
				bDocked = FALSE;

				if ( PtInRect( &rcRight, pt )) {
					nNewX = rc.right;
					bDocked = TRUE;
				}

				if ( PtInRect( &rcBottom, pt )) {
					nNewY = rc.bottom;
					if ( PtInRect( &rcLeft, pt ))
						nNewX = rc.left;
				}

				if ( PtInRect( &rcTop, pt )) {
					nNewY	 = rc.top;
					bDockedLeft = bDocked;
				}

				// Upper-right
				pt.x = rcThumb.right;
				pt.y = rcThumb.top;
				bDocked = FALSE;

				if ( !bLeading && PtInRect( &rcLeft, pt )) {
					if ( !bDockedLeft ) {
						nNewX = rc.left - nWidth;
						bDocked = TRUE;
					}
					else if ( rc.right == rcThumb.left )
						bDocked = TRUE;
				}

				if ( PtInRect( &rcBottom, pt )) {
					nNewY = rc.bottom;
					if ( PtInRect( &rcRight, pt ))
						nNewX = rc.right - nWidth;
				}

				if ( !bLeading && PtInRect( &rcTop, pt )) {
					nNewY = rc.top;
					bDockedRight = bDocked;
				}

				if ( bMoveTogether ) {
					if ( bDockedRight )
						DockThumbs( pThumb, pCurThumb, TRUE );

					if ( bDockedLeft )
						DockThumbs( pCurThumb, pThumb, FALSE );
				}

				// Lower-left
				pt.x = rcThumb.left;
				pt.y = rcThumb.bottom;

				if ( PtInRect( &rcRight, pt ))
					nNewX = rc.right;

				if ( PtInRect( &rcTop, pt )) {
					nNewY = rc.top - nHeight;
					if ( PtInRect( &rcLeft, pt ))
						nNewX = rc.left;
				}

				// Lower-right
				pt.x = rcThumb.right;
				pt.y = rcThumb.bottom;

				if ( !bLeading && PtInRect( &rcLeft, pt ))
					nNewX = rc.left - nWidth;

				if ( !bLeading && PtInRect( &rcTop, pt )) {
					nNewY = rc.top - nHeight;
					if ( PtInRect( &rcRight, pt ))
						nNewX = rc.right - nWidth;
				}
			}
		}

		frmidx++;
		if (pCurThumb->ContainerWnd = fakeTaskBarWindow.ContainerWnd){break;}
		if (pCurThumb->ContainerWnd = fakeMainWindow.ContainerWnd){
			pCurThumb = &fakeTaskBarWindow;continue;}
		if (frmidx = nFramescount){
			pCurThumb = &fakeMainWindow;continue;
		}

		pCurThumb = &Frames[frmidx];
	}

	// Adjust coords once again
	nNewX = nNewX < nOffs ? 0 : nNewX;
	nNewX = nNewX > ( sizeScreen.cx - nWidth - nOffs ) ? ( sizeScreen.cx - nWidth ) : nNewX;
	nNewY = nNewY < nOffs ? 0 : nNewY;
	nNewY = nNewY > ( sizeScreen.cy - nHeight - nOffs ) ? ( sizeScreen.cy - nHeight ) : nNewY;

	SetWindowPos(	pThumb->ContainerWnd, HWND_TOPMOST, nNewX, nNewY, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

	// OK, move all docked thumbs
	if ( bMoveTogether ) {
		pDockThumb = FindFrameByWnd( pDockThumb->dockOpt.hwndRight );
		PositionThumb( pDockThumb, (short)( nNewX + nWidth ), (short)nNewY );
	}
}

//////////

void GetBorderSize(HWND hwnd,RECT *rect)
{
	RECT wr,cr;
	POINT pt1,pt2;

	GetWindowRect(hwnd,&wr);
	GetClientRect(hwnd,&cr);
	pt1.y = cr.top;pt1.x = cr.left;
	pt2.y = cr.bottom;pt2.x = cr.right;

	ClientToScreen(hwnd,&pt1);
	ClientToScreen(hwnd,&pt2);

	cr.top = pt1.y;cr.left = pt1.x;
	cr.bottom = pt2.y;cr.right = pt2.x;

	rect->top = cr.top-wr.top;
	rect->left = cr.left-wr.left;
	rect->right = wr.right-cr.right;
	rect->bottom = wr.bottom-cr.bottom;
}

//append string
char __inline *AS(char *str,const char *setting,char *addstr)
{
	if (str != NULL) {
		strcpy(str,setting);
		strcat(str,addstr);
	}
	return str;
}

int DBLoadFrameSettingsAtPos(int pos,int Frameid)
{
	char sadd[15];
	char buf[255];

	_itoa(pos,sadd,10);

	//boolean
	Frames[Frameid].collapsed = db_get_b(0,CLUIFrameModule,AS(buf,"Collapse",sadd),Frames[Frameid].collapsed);

	Frames[Frameid].Locked					 = db_get_b(0,CLUIFrameModule,AS(buf,"Locked",sadd),Frames[Frameid].Locked);
	Frames[Frameid].visible					 = db_get_b(0,CLUIFrameModule,AS(buf,"Visible",sadd),Frames[Frameid].visible);
	Frames[Frameid].TitleBar.ShowTitleBar	 = db_get_b(0,CLUIFrameModule,AS(buf,"TBVisile",sadd),Frames[Frameid].TitleBar.ShowTitleBar);

	Frames[Frameid].height					 = db_get_w(0,CLUIFrameModule,AS(buf,"Height",sadd),Frames[Frameid].height);
	Frames[Frameid].HeightWhenCollapsed		 = db_get_w(0,CLUIFrameModule,AS(buf,"HeightCollapsed",sadd),0);
	Frames[Frameid].align					 = db_get_w(0,CLUIFrameModule,AS(buf,"Align",sadd),Frames[Frameid].align);

	Frames[Frameid].FloatingPos.x		 = DBGetContactSettingRangedWord(0,CLUIFrameModule,AS(buf,"FloatX",sadd),100,0,1024);
	Frames[Frameid].FloatingPos.y		 = DBGetContactSettingRangedWord(0,CLUIFrameModule,AS(buf,"FloatY",sadd),100,0,1024);
	Frames[Frameid].FloatingSize.x		 = DBGetContactSettingRangedWord(0,CLUIFrameModule,AS(buf,"FloatW",sadd),100,0,1024);
	Frames[Frameid].FloatingSize.y		 = DBGetContactSettingRangedWord(0,CLUIFrameModule,AS(buf,"FloatH",sadd),100,0,1024);

	Frames[Frameid].floating			 = db_get_b(0,CLUIFrameModule,AS(buf,"Floating",sadd),0);
	Frames[Frameid].order				 = db_get_w(0,CLUIFrameModule,AS(buf,"Order",sadd),0);

	Frames[Frameid].UseBorder			 = db_get_b(0,CLUIFrameModule,AS(buf,"UseBorder",sadd),Frames[Frameid].UseBorder);

	return 0;
}

int DBStoreFrameSettingsAtPos(int pos,int Frameid)
{
	char sadd[16];
	char buf[255];

	_itoa(pos,sadd,10);

	db_set_ts(0,CLUIFrameModule,AS(buf,"Name",sadd),Frames[Frameid].name);
	//boolean
	db_set_b(0,CLUIFrameModule,AS(buf,"Collapse",sadd),(BYTE)btoint(Frames[Frameid].collapsed));
	db_set_b(0,CLUIFrameModule,AS(buf,"Locked",sadd),(BYTE)btoint(Frames[Frameid].Locked));
	db_set_b(0,CLUIFrameModule,AS(buf,"Visible",sadd),(BYTE)btoint(Frames[Frameid].visible));
	db_set_b(0,CLUIFrameModule,AS(buf,"TBVisile",sadd),(BYTE)btoint(Frames[Frameid].TitleBar.ShowTitleBar));

	db_set_w(0,CLUIFrameModule,AS(buf,"Height",sadd),(WORD)Frames[Frameid].height);
	db_set_w(0,CLUIFrameModule,AS(buf,"HeightCollapsed",sadd),(WORD)Frames[Frameid].HeightWhenCollapsed);
	db_set_w(0,CLUIFrameModule,AS(buf,"Align",sadd),(WORD)Frames[Frameid].align);
	//FloatingPos
	db_set_w(0,CLUIFrameModule,AS(buf,"FloatX",sadd),(WORD)Frames[Frameid].FloatingPos.x);
	db_set_w(0,CLUIFrameModule,AS(buf,"FloatY",sadd),(WORD)Frames[Frameid].FloatingPos.y);
	db_set_w(0,CLUIFrameModule,AS(buf,"FloatW",sadd),(WORD)Frames[Frameid].FloatingSize.x);
	db_set_w(0,CLUIFrameModule,AS(buf,"FloatH",sadd),(WORD)Frames[Frameid].FloatingSize.y);

	db_set_b(0,CLUIFrameModule,AS(buf,"Floating",sadd),(BYTE)btoint(Frames[Frameid].floating));
	db_set_b(0,CLUIFrameModule,AS(buf,"UseBorder",sadd),(BYTE)btoint(Frames[Frameid].UseBorder));
	db_set_w(0,CLUIFrameModule,AS(buf,"Order",sadd),(WORD)Frames[Frameid].order);
	//db_set_s(0,CLUIFrameModule,AS(buf,"TBName",sadd),Frames[Frameid].TitleBar.tbname);
	return 0;
}

int LocateStorePosition(int Frameid,int maxstored)
{
	if (Frames[Frameid].name == NULL)
		return -1;

	for (int i=0; i < maxstored; i++) {
		char settingname[255];
		mir_snprintf(settingname, sizeof(settingname), "Name%d",i);
		LPTSTR frmname = db_get_tsa(0,CLUIFrameModule,settingname);
		if (frmname == NULL)
			continue;

		if (lstrcmpi(frmname,Frames[Frameid].name) == 0) {
			mir_free(frmname);
			return i;
		}
		mir_free(frmname);
	}
	return -1;
}

int CLUIFramesLoadFrameSettings(int Frameid)
{
	if (Frameid < 0 || Frameid >= nFramescount)
		return -1;

	int maxstored = db_get_w(0,CLUIFrameModule,"StoredFrames",-1);
	if (maxstored == -1)
		return 0;

	int storpos = LocateStorePosition(Frameid,maxstored);
	if (storpos == -1)
		return 0;

	DBLoadFrameSettingsAtPos(storpos,Frameid);
	return 0;
}

int CLUIFramesStoreFrameSettings(int Frameid)
{
	if (Frameid < 0 || Frameid >= nFramescount)
		return -1;

	int maxstored = db_get_w(0,CLUIFrameModule,"StoredFrames",-1);
	if (maxstored == -1)
		maxstored = 0;

	int storpos = LocateStorePosition(Frameid,maxstored);
	if (storpos == -1) {
		storpos = maxstored;
		maxstored++;
	}

	DBStoreFrameSettingsAtPos(storpos,Frameid);
	db_set_w(0,CLUIFrameModule,"StoredFrames",(WORD)maxstored);
	//ulockfrm();
	return 0;
}

int CLUIFramesStoreAllFrames()
{
	lockfrm();
	for (int i=0;i<nFramescount;i++)
		CLUIFramesStoreFrameSettings(i);
	ulockfrm();
	return 0;
}

// Get client frame
int CLUIFramesGetalClientFrame(void)
{
	if (alclientFrame != -1) {
		/* this value could become invalid if RemoveItemFromList was called,
		* so we double-check */
		if (alclientFrame < nFramescount)
			if (Frames[alclientFrame].align == alClient)
				return alclientFrame;
	}

	for (int i=0; i < nFramescount; i++)
		if (Frames[i].align == alClient) {
			alclientFrame = i;
			return i;
		}

	return -1;
}

HMENU CLUIFramesCreateMenuForFrame(int frameid,int root,int popuppos,HGENMENU (*pfnAdd )( CLISTMENUITEM* ))
{
	HGENMENU menuid;
	int framepos = id2pos(frameid);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_CLIENTMIRANDA));
	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&FrameTitle");
	mi.flags = CMIF_CHILDPOPUP|CMIF_GRAYED;
	mi.pszContactOwner = (char *)0;
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
	mi.pszContactOwner = (char *)0;
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
	mi.pszContactOwner = (char *)0;
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
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMILock = menuid;
	else Frames[framepos].MenuHandles.MILock = menuid;

	mi.pszPopupName = (char *)root;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Collapsed");
	mi.flags = CMIF_CHILDPOPUP | CMIF_CHECKED;
	mi.pszService = MS_CLIST_FRAMES_UCOLLFRAME;
	mi.pszContactOwner = (char *)0;
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
	mi.pszContactOwner = (char *)0;
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
	mi.pszContactOwner = (char *)0;
	menuid = pfnAdd(&mi);
	if (frameid == -1) contMIBorder = menuid;
	else Frames[framepos].MenuHandles.MIBorder = menuid;

	popuppos += 100000;

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

	//position
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
	mi.pszService = CLUIFRAMESMOVEUPDOWN;
	mi.pszContactOwner = (char *)1;
	pfnAdd(&mi);

	mi.pszPopupName = (char *)menuid;
	mi.popupPosition = frameid;
	mi.position = popuppos++;
	mi.pszName = LPGEN("&Down");
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszService = CLUIFRAMESMOVEUPDOWN;
	mi.pszContactOwner = (char *)-1;
	pfnAdd(&mi);

	return 0;
}

INT_PTR ModifyMItem(WPARAM wParam, LPARAM lParam)
{
	return CallService(MS_CLIST_MODIFYMENUITEM, wParam, lParam);
}

static int CLUIFramesModifyContextMenuForFrame(WPARAM wParam, LPARAM lParam)
{
	lockfrm();
	int pos = id2pos(wParam);

	if (pos >= 0 && pos < nFramescount) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_NAME|CMIF_CHILDPOPUP|CMIF_TCHAR;
		mi.ptszName = Frames[pos].TitleBar.tbname ? Frames[pos].TitleBar.tbname : Frames[pos].name;
		ModifyMItem((WPARAM)contMITitle,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].visible) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIVisible,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].Locked) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMILock,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].TitleBar.ShowTitleBar) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMITBVisible,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].floating) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIFloating,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if ((Frames[pos].UseBorder)) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIBorder,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].align&alTop) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIAlignTop,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].align&alClient) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIAlignClient,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].align&alBottom) mi.flags |= CMIF_CHECKED;
		ModifyMItem((WPARAM)contMIAlignBottom,(LPARAM)&mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].collapsed) mi.flags |= CMIF_CHECKED;
		if ((!Frames[pos].visible)||(Frames[pos].Locked)||(pos == CLUIFramesGetalClientFrame())) mi.flags |= CMIF_GRAYED;
		ModifyMItem((WPARAM)contMIColl,(LPARAM)&mi);
	}
	ulockfrm();
	return 0;
}

INT_PTR CLUIFramesModifyMainMenuItems(WPARAM wParam, LPARAM lParam)
{
	lockfrm();

	int pos = id2pos(wParam);
	if (pos >= 0 && pos < nFramescount) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_NAME|CMIF_CHILDPOPUP|CMIF_TCHAR;
		mi.ptszName = Frames[pos].TitleBar.tbname ? Frames[pos].TitleBar.tbname : Frames[pos].name;
		Menu_ModifyItem(Frames[pos].MenuHandles.MITitle, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].visible) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIVisible, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].Locked) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MILock, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].TitleBar.ShowTitleBar) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MITBVisible, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].floating) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIFloating, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if ((Frames[pos].UseBorder)) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIBorder, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP|((Frames[pos].align&alClient)?CMIF_GRAYED:0);
		if (Frames[pos].align&alTop) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIAlignTop, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].align&alClient) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIAlignClient, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP|((Frames[pos].align&alClient)?CMIF_GRAYED:0);
		if (Frames[pos].align&alBottom) mi.flags |= CMIF_CHECKED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIAlignBottom, &mi);

		mi.flags = CMIM_FLAGS|CMIF_CHILDPOPUP;
		if (Frames[pos].collapsed) mi.flags |= CMIF_CHECKED;
		if ((!Frames[pos].visible)||Frames[pos].Locked||(pos == CLUIFramesGetalClientFrame())) mi.flags |= CMIF_GRAYED;
		Menu_ModifyItem(Frames[pos].MenuHandles.MIColl, &mi);
	}
	ulockfrm();
	return 0;
}

//hiword(wParam) = frameid,loword(wParam) = flag
INT_PTR CLUIFramesGetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return 0;

	lockfrm();
	int pos = id2pos(HIWORD(wParam));
	if (pos < 0 || pos >= nFramescount) {
		ulockfrm();
		return -1;
	}

	INT_PTR retval;

	switch(LOWORD(wParam)) {
	case FO_FLAGS:
		retval = 0;
		if (Frames[pos].visible) retval |= F_VISIBLE;
		if ( !Frames[pos].collapsed) retval |= F_UNCOLLAPSED;
		if (Frames[pos].Locked) retval |= F_LOCKED;
		if (Frames[pos].TitleBar.ShowTitleBar) retval |= F_SHOWTB;
		if (Frames[pos].TitleBar.ShowTitleBarTip) retval |= F_SHOWTBTIP;
		if ( !(GetWindowLongPtr(Frames[pos].hWnd,GWL_STYLE)&WS_BORDER)) retval |= F_NOBORDER;
		break;

	case FO_NAME:
		retval = (INT_PTR)Frames[pos].name;
		break;

	case FO_TBNAME:
		retval = (INT_PTR)Frames[pos].TitleBar.tbname;
		break;

	case FO_TBTIPNAME:
		retval = (INT_PTR)Frames[pos].TitleBar.tooltip;
		break;

	case FO_TBSTYLE:
		retval = GetWindowLongPtr(Frames[pos].TitleBar.hwnd,GWL_STYLE);
		break;

	case FO_TBEXSTYLE:
		retval = GetWindowLongPtr(Frames[pos].TitleBar.hwnd,GWL_EXSTYLE);
		break;

	case FO_ICON:
		retval = (INT_PTR)Frames[pos].TitleBar.hicon;
		break;

	case FO_HEIGHT:
		retval = (INT_PTR)Frames[pos].height;
		break;

	case FO_ALIGN:
		retval = (INT_PTR)Frames[pos].align;
		break;
	case FO_FLOATING:
		retval = (INT_PTR)Frames[pos].floating;
		break;
	default:
		retval = -1;
		break;
	}
	ulockfrm();
	return retval;
}

//hiword(wParam) = frameid,loword(wParam) = flag
INT_PTR CLUIFramesSetFrameOptions(WPARAM wParam, LPARAM lParam)
{
	lockfrm();
	int pos = id2pos(HIWORD(wParam));
	if (pos<0||pos>=nFramescount) {
		ulockfrm();
		return -1;
	}

	INT_PTR retval; // value to be returned

	switch(LOWORD(wParam) & ~FO_UNICODETEXT) {
	case FO_FLAGS:{
		int flag = lParam;
		LONG_PTR style;

		Frames[pos].dwFlags = flag;
		Frames[pos].visible = FALSE;
		if (flag&F_VISIBLE) Frames[pos].visible = TRUE;

		Frames[pos].collapsed = TRUE;
		if (flag&F_UNCOLLAPSED) Frames[pos].collapsed = FALSE;

		Frames[pos].Locked = FALSE;
		if (flag&F_LOCKED) Frames[pos].Locked = TRUE;

		Frames[pos].UseBorder = TRUE;
		if (flag&F_NOBORDER) Frames[pos].UseBorder = FALSE;

		Frames[pos].TitleBar.ShowTitleBar = FALSE;
		if (flag&F_SHOWTB) Frames[pos].TitleBar.ShowTitleBar = TRUE;

		Frames[pos].TitleBar.ShowTitleBarTip = FALSE;
		if (flag&F_SHOWTBTIP) Frames[pos].TitleBar.ShowTitleBarTip = TRUE;

		SendMessage(Frames[pos].TitleBar.hwndTip,TTM_ACTIVATE,(WPARAM)Frames[pos].TitleBar.ShowTitleBarTip,0);

		style = GetWindowLongPtr(Frames[pos].hWnd,GWL_STYLE);
		style |= WS_BORDER;
		if (flag&F_NOBORDER) {style &= (~WS_BORDER);}
			SetWindowLongPtr(Frames[pos].hWnd,GWL_STYLE,(LONG_PTR)style);
		ulockfrm();
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
		return 0;
	}

	case FO_NAME:
		if (lParam == 0) {ulockfrm(); return -1;}
		mir_free(Frames[pos].name);
		Frames[pos].name = mir_tstrdup((LPTSTR)lParam);
		ulockfrm();
		return 0;

	case FO_TBNAME:
		if (lParam == 0) {ulockfrm(); return(-1);}
		mir_free(Frames[pos].TitleBar.tbname);
		Frames[pos].TitleBar.tbname = mir_tstrdup((LPTSTR)lParam);
		ulockfrm();
		if (Frames[pos].floating&&(Frames[pos].TitleBar.tbname != NULL)){SetWindowText(Frames[pos].ContainerWnd,Frames[pos].TitleBar.tbname);}
		return 0;

	case FO_TBTIPNAME:
		if (lParam == 0) {ulockfrm(); return(-1);}
		mir_free(Frames[pos].TitleBar.tooltip);
		Frames[pos].TitleBar.tooltip = mir_tstrdup((LPTSTR)lParam);
		UpdateTBToolTip(pos);
		ulockfrm();
		return 0;

	case FO_TBSTYLE:
		SetWindowLongPtr(Frames[pos].TitleBar.hwnd,GWL_STYLE,lParam);
		ulockfrm();
		return 0;

	case FO_TBEXSTYLE:
		SetWindowLongPtr(Frames[pos].TitleBar.hwnd,GWL_EXSTYLE,lParam);
		ulockfrm();
		return 0;

	case FO_ICON:
		Frames[pos].TitleBar.hicon = (HICON)lParam;
		ulockfrm();
		return 0;

	case FO_HEIGHT:
		if (lParam<0) {ulockfrm(); return -1;}
		retval = Frames[pos].height;
		Frames[pos].height = lParam;
		if ( !CLUIFramesFitInSize()) Frames[pos].height = retval;
		retval = Frames[pos].height;
		ulockfrm();

		return retval;

	case FO_FLOATING:
		if (lParam<0) {ulockfrm(); return -1;}

		{
			int id = Frames[pos].id;
			Frames[pos].floating = !(lParam);
			ulockfrm();

			CLUIFrameSetFloat(id,1);//lparam = 1 use stored width and height
			return wParam;
		}

	case FO_ALIGN:
		if ( !(lParam&alTop || lParam&alBottom || lParam&alClient)) {
			OutputDebugStringA("Wrong align option \r\n");
			return (-1);
		}

		if ((lParam&alClient)&&(CLUIFramesGetalClientFrame()>=0)) {	//only one alClient frame possible
			alclientFrame = -1;//recalc it
			ulockfrm();
			return -1;
		}
		Frames[pos].align = lParam;

		ulockfrm();
		return 0;
	}
	ulockfrm();
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return -1;
}

//wparam = lparam = 0
static INT_PTR CLUIFramesShowAll(WPARAM wParam, LPARAM lParam)
{
	for (int i=0;i<nFramescount;i++)
		Frames[i].visible = TRUE;
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return 0;
}

//wparam = lparam = 0
INT_PTR CLUIFramesShowAllTitleBars(WPARAM wParam, LPARAM lParam)
{
	for (int i=0;i<nFramescount;i++)
		Frames[i].TitleBar.ShowTitleBar = TRUE;
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return 0;
}

//wparam = lparam = 0
INT_PTR CLUIFramesHideAllTitleBars(WPARAM wParam, LPARAM lParam)
{
	for (int i=0;i<nFramescount;i++)
		Frames[i].TitleBar.ShowTitleBar = FALSE;
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return 0;
}

//wparam = frameid
INT_PTR CLUIFramesShowHideFrame(WPARAM wParam, LPARAM lParam)
{
	lockfrm();
	int pos = id2pos(wParam);
	if ( pos >= 0 && (int)pos < nFramescount)
		Frames[pos].visible = !Frames[pos].visible;
	if (Frames[pos].floating)
		CLUIFrameResizeFloatingFrame(pos);
	ulockfrm();

	if ( !Frames[pos].floating)
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return 0;
}

//wparam = frameid
INT_PTR CLUIFramesShowHideFrameTitleBar(WPARAM wParam, LPARAM lParam)
{
	lockfrm();
	int pos = id2pos(wParam);
	if ( pos >= 0 && (int)pos < nFramescount)
		Frames[pos].TitleBar.ShowTitleBar = !Frames[pos].TitleBar.ShowTitleBar;

	ulockfrm();

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return 0;
}

//wparam = frameid
//lparam = -1 up ,1 down
INT_PTR CLUIFramesMoveUpDown(WPARAM wParam, LPARAM lParam)
{
	int pos,i,curpos,curalign,v,tmpval;

	lockfrm();
	pos = id2pos(wParam);
	if (pos >= 0 &&(int)pos < nFramescount)	{
		SortData *sd;
		curpos = Frames[pos].order;
		curalign = Frames[pos].align;
		v = 0;
		sd = (SortData*)malloc(sizeof(SortData)*nFramescount);
		memset(sd,0,sizeof(SortData)*nFramescount);
		for (i=0; i < nFramescount; i++) {
			if (Frames[i].floating||(!Frames[i].visible)||(Frames[i].align != curalign))
				continue;

			sd[v].order = Frames[i].order;
			sd[v].realpos = i;
			v++;
		}
		if (v == 0) {
			ulockfrm();
			return 0;
		}
		qsort(sd,v,sizeof(SortData),sortfunc);
		for (i=0; i < v; i++) {
			if (sd[i].realpos == pos) {
				if (lParam == -1) {
					if (i >= v-1)
						break;

					tmpval = Frames[sd[i+1].realpos].order;
					Frames[sd[i+1].realpos].order = Frames[pos].order;
					Frames[pos].order = tmpval;
					break;
				}
				if (lParam == +1)	{
					if (i < 1)
						break;

					tmpval = Frames[sd[i-1].realpos].order;
					Frames[sd[i-1].realpos].order = Frames[pos].order;
					Frames[pos].order = tmpval;
					break;
				}
			}
		}

		free(sd);
		CLUIFramesStoreFrameSettings(pos);
		CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	}
	ulockfrm();
	return 0;
}

//wparam = frameid
//lparam = alignment
INT_PTR CLUIFramesSetAlign(WPARAM wParam, LPARAM lParam)
{
	CLUIFramesSetFrameOptions(MAKEWPARAM(FO_ALIGN,wParam),lParam);
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	return 0;
}

INT_PTR CLUIFramesSetAlignalTop(WPARAM wParam, LPARAM lParam)
{
	return CLUIFramesSetAlign(wParam,alTop);
}

INT_PTR CLUIFramesSetAlignalBottom(WPARAM wParam, LPARAM lParam)
{
	return CLUIFramesSetAlign(wParam,alBottom);
}

INT_PTR CLUIFramesSetAlignalClient(WPARAM wParam, LPARAM lParam)
{
	return CLUIFramesSetAlign(wParam,alClient);
}

//wparam = frameid
INT_PTR CLUIFramesLockUnlockFrame(WPARAM wParam, LPARAM lParam)
{
	lockfrm();
	int pos = id2pos(wParam);
	if (pos >= 0 && (int)pos < nFramescount) {
		Frames[pos].Locked = !Frames[pos].Locked;
		CLUIFramesStoreFrameSettings(pos);
	}
	ulockfrm();
	return 0;
}

//wparam = frameid
INT_PTR CLUIFramesSetUnSetBorder(WPARAM wParam, LPARAM lParam)
{
	int oldflags;

	lockfrm();
	int FrameId = id2pos(wParam);
	if (FrameId == -1) {
		ulockfrm();
		return -1;
	}

	boolean flt = oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,wParam),0);
	if (oldflags & F_NOBORDER)
		oldflags &= (~F_NOBORDER);
	else
		oldflags |= F_NOBORDER;

	HWND hw = Frames[FrameId].hWnd;
	RECT rc;
	GetWindowRect(hw, &rc);

	ulockfrm();
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,wParam),oldflags);

	SetWindowPos(hw,0,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_DRAWFRAME);
	return 0;
}

//wparam = frameid
INT_PTR CLUIFramesCollapseUnCollapseFrame(WPARAM wParam, LPARAM lParam)
{
	int FrameId;

	lockfrm();
	FrameId = id2pos(wParam);
	if (FrameId >= 0 && FrameId < nFramescount) {
		int oldHeight;

		// do not collapse/uncollapse client/locked/invisible frames
		if (Frames[FrameId].align == alClient&&!(Frames[FrameId].Locked||(!Frames[FrameId].visible)||Frames[FrameId].floating)) {
			RECT rc;
			if (CallService(MS_CLIST_DOCKINGISDOCKED,0,0)) {
				ulockfrm();
				return 0;
			}
			if ( db_get_b(NULL,"CLUI","AutoSize",0)) {
				ulockfrm();
				return 0;
			}

			GetWindowRect(pcli->hwndContactList, &rc);

			if (Frames[FrameId].collapsed == TRUE)	{
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

			SetWindowPos(pcli->hwndContactList,NULL,0,0,rc.right-rc.left,rc.bottom,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);

			CLUIFramesStoreAllFrames();
			ulockfrm();
			return 0;
		}

		if (Frames[FrameId].Locked||(!Frames[FrameId].visible))
			return 0;

		oldHeight = Frames[FrameId].height;

		// if collapsed, uncollapse
		if (Frames[FrameId].collapsed == TRUE)	{
			Frames[FrameId].HeightWhenCollapsed = Frames[FrameId].height;
			Frames[FrameId].height = UNCOLLAPSED_FRAME_SIZE;
			Frames[FrameId].collapsed = FALSE;
		}
		// if uncollapsed, collapse
		else {
			Frames[FrameId].height = Frames[FrameId].HeightWhenCollapsed;
			Frames[FrameId].collapsed = TRUE;
		}

		if ( !Frames[FrameId].floating) {
			if ( !CLUIFramesFitInSize()) {
				//cant collapse,we can resize only for height<alclient frame height
				int alfrm = CLUIFramesGetalClientFrame();

				if (alfrm != -1) {
					Frames[FrameId].collapsed = FALSE;
					if (Frames[alfrm].height>2*UNCOLLAPSED_FRAME_SIZE) {
						oldHeight = Frames[alfrm].height-UNCOLLAPSED_FRAME_SIZE;
						Frames[FrameId].collapsed = TRUE;
					}
				}
				else {
					int i,sumheight = 0;
					for (i=0; i < nFramescount; i++) {
						if ((Frames[i].align != alClient)&&(!Frames[i].floating)&&(Frames[i].visible)&&(!Frames[i].needhide)) {
							sumheight += (Frames[i].height)+(TitleBarH*btoint(Frames[i].TitleBar.ShowTitleBar))+2;
							return FALSE;
						}

						if (sumheight>ContactListHeight-0-2)
							Frames[FrameId].height = (ContactListHeight-0-2)-sumheight;
					}
				}

				Frames[FrameId].height = oldHeight;

				if (Frames[FrameId].collapsed == FALSE) {
					if (Frames[FrameId].floating)
						SetWindowPos(Frames[FrameId].ContainerWnd,HWND_TOP,0,0,Frames[FrameId].wndSize.right-Frames[FrameId].wndSize.left+6,Frames[FrameId].height+DEFAULT_TITLEBAR_HEIGHT+4,SWP_SHOWWINDOW|SWP_NOMOVE);

					ulockfrm();
					return -1;
				}
			}
		}

		ulockfrm();

		if ( !Frames[FrameId].floating)
			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
		else {
			//SetWindowPos(Frames[FrameId].hWnd,HWND_TOP,0,0,Frames[FrameId].wndSize.right-Frames[FrameId].wndSize.left,Frames[FrameId].height,SWP_SHOWWINDOW|SWP_NOMOVE);
			RECT contwnd;
			GetWindowRect(Frames[FrameId].ContainerWnd,&contwnd);
			contwnd.top = contwnd.bottom-contwnd.top;//height
			contwnd.left = contwnd.right-contwnd.left;//width

			contwnd.top -= (oldHeight-Frames[FrameId].height);//newheight
			SetWindowPos(Frames[FrameId].ContainerWnd,HWND_TOP,0,0,contwnd.left,contwnd.top,SWP_SHOWWINDOW|SWP_NOMOVE);
		}
		CLUIFramesStoreAllFrames();
		return 0;
	}
	else return -1;

	ulockfrm();
	return 0;
}

static int CLUIFramesLoadMainMenu()
{
	if (MainMIRoot != (HGENMENU)-1) {
		CallService(MS_CLIST_REMOVEMAINMENUITEM,(WPARAM)MainMIRoot,0);
		MainMIRoot = (HGENMENU)-1;
	}

	// create root menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_FRAME);
	mi.flags = CMIF_ROOTPOPUP;
	mi.position = 3000090000;
	mi.pszPopupName = (char*)-1;
	mi.pszName = LPGEN("Frames");
	mi.pszService = 0;
	MainMIRoot = Menu_AddMainMenuItem(&mi);

	// create frames menu
	int separator = 3000200000;
 	for (int i=0; i < nFramescount; i++) {
		mi.hIcon = Frames[i].TitleBar.hicon;
		mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTPOPUP | CMIF_TCHAR;
		mi.position = separator;
		mi.hParentMenu = MainMIRoot;
		mi.ptszName = Frames[i].TitleBar.tbname ? Frames[i].TitleBar.tbname : Frames[i].name;
		mi.pszService = 0;
		Frames[i].MenuHandles.MainMenuItem = Menu_AddMainMenuItem(&mi);
		CLUIFramesCreateMenuForFrame(Frames[i].id,(int)Frames[i].MenuHandles.MainMenuItem,separator,Menu_AddMainMenuItem);
		CLUIFramesModifyMainMenuItems(Frames[i].id,0);
		//NotifyEventHooks(hPreBuildFrameMenuEvent,i,(LPARAM)Frames[i].MenuHandles.MainMenuItem);
		CallService(MS_CLIST_FRAMEMENUNOTIFY,(WPARAM)Frames[i].id,(LPARAM)Frames[i].MenuHandles.MainMenuItem);
		separator++;
	}

	separator += 100000;

	// create "show all frames" menu
	mi.hIcon = NULL;
	mi.flags = CMIF_CHILDPOPUP;
	mi.position = separator++;
	mi.hParentMenu = MainMIRoot;
	mi.pszName = LPGEN("Show all frames");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMES;
	Menu_AddMainMenuItem(&mi);

	// create "show all titlebars" menu
	mi.hIcon = NULL;
	mi.position = separator++;
	mi.hParentMenu = MainMIRoot;
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszName = LPGEN("Show all title bars");
	mi.pszService = MS_CLIST_FRAMES_SHOWALLFRAMESTB;
	Menu_AddMainMenuItem(&mi);

	// create "hide all titlebars" menu
	mi.hIcon = NULL;
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
	HFONT hfont;
	LOGFONT logfont;
	memset(&logfont,0,sizeof(logfont));
	memcpy(logfont.lfFaceName,facename,sizeof(facename));
	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = -10;
	hfont = CreateFontIndirect(&logfont);
	return hfont;
}

static int UpdateTBToolTip(int framepos)
{
	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(ti);
	ti.lpszText = Frames[framepos].TitleBar.tooltip;
	ti.hinst = g_hInst;
	ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS ;
	ti.uId = (UINT_PTR)Frames[framepos].TitleBar.hwnd;

	return SendMessage(Frames[framepos].TitleBar.hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
}

//wparam = (CLISTFrame*)clfrm
INT_PTR CLUIFramesAddFrame(WPARAM wParam, LPARAM lParam)
{
	int retval;
	LONG_PTR style;
	CLISTFrame *clfrm = (CLISTFrame *)wParam;

	if (pcli->hwndContactList == 0) return -1;
	if (FramesSysNotStarted) return -1;
	if (clfrm->cbSize != sizeof(CLISTFrame)) return -1;
	if ( !(TitleBarFont)) TitleBarFont = CLUILoadTitleBarFont();

	lockfrm();
	if (nFramescount>=MAX_FRAMES) { ulockfrm(); return -1;}
	Frames = (FRAMEWND*)realloc(Frames,sizeof(FRAMEWND)*(nFramescount+1));

	memset(&Frames[nFramescount],0,sizeof(FRAMEWND));
	Frames[nFramescount].id = NextFrameId++;
	Frames[nFramescount].align = clfrm->align;
	Frames[nFramescount].hWnd = clfrm->hWnd;
	Frames[nFramescount].height = clfrm->height;
	Frames[nFramescount].TitleBar.hicon = clfrm->hIcon;
	Frames[nFramescount].floating = FALSE;

	//override tbbtip
	//clfrm->Flags != F_SHOWTBTIP;
	//
	if ( db_get_b(0,CLUIFrameModule,"RemoveAllBorders",0) == 1)
		clfrm->Flags |= F_NOBORDER;

	Frames[nFramescount].dwFlags = clfrm->Flags;

	if (clfrm->name == NULL||((clfrm->Flags&F_UNICODE) ? lstrlenW(clfrm->wname) : lstrlenA(clfrm->name)) == 0) {
		Frames[nFramescount].name = (LPTSTR)malloc(255 * sizeof(TCHAR));
		GetClassName(Frames[nFramescount].hWnd,Frames[nFramescount].name,255);
	}
	else Frames[nFramescount].name = (clfrm->Flags&F_UNICODE) ? mir_u2t(clfrm->wname) : mir_a2t(clfrm->name);

	if (IsBadCodePtr((FARPROC)clfrm->TBname) || clfrm->TBname == NULL
		|| ((clfrm->Flags&F_UNICODE) ? lstrlenW(clfrm->TBwname) : lstrlenA(clfrm->TBname)) == 0)
		Frames[nFramescount].TitleBar.tbname = mir_tstrdup(Frames[nFramescount].name);
	else
		Frames[nFramescount].TitleBar.tbname = (clfrm->Flags&F_UNICODE) ? mir_u2t(clfrm->TBwname) : mir_a2t(clfrm->TBname);
	Frames[nFramescount].needhide = FALSE;
	Frames[nFramescount].TitleBar.ShowTitleBar = (clfrm->Flags&F_SHOWTB?TRUE:FALSE);
	Frames[nFramescount].TitleBar.ShowTitleBarTip = (clfrm->Flags&F_SHOWTBTIP?TRUE:FALSE);

	Frames[nFramescount].collapsed = clfrm->Flags & F_UNCOLLAPSED ? FALSE:TRUE;
	Frames[nFramescount].Locked = clfrm->Flags & F_LOCKED ? TRUE : FALSE;
	Frames[nFramescount].visible = clfrm->Flags & F_VISIBLE ? TRUE : FALSE;

	Frames[nFramescount].UseBorder = (clfrm->Flags&F_NOBORDER)?FALSE:TRUE;

	// create frame
	Frames[nFramescount].TitleBar.hwnd = CreateWindow(CLUIFrameTitleBarClassName,Frames[nFramescount].name,
		( db_get_b(0,CLUIFrameModule,"RemoveAllTitleBarBorders",0)?0:WS_BORDER) |WS_CHILD|WS_CLIPCHILDREN|
		(Frames[nFramescount].TitleBar.ShowTitleBar?WS_VISIBLE:0)| WS_CLIPCHILDREN,
		0,0,0,0,pcli->hwndContactList,NULL,g_hInst,NULL);
	SetWindowLongPtr(Frames[nFramescount].TitleBar.hwnd,GWLP_USERDATA,Frames[nFramescount].id);

	Frames[nFramescount].TitleBar.hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		pcli->hwndContactList, NULL, g_hInst, NULL);

	SetWindowPos(Frames[nFramescount].TitleBar.hwndTip, HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(ti);
	ti.lpszText = _T("");
	ti.hinst = g_hInst;
	ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS ;
	ti.uId = (UINT_PTR)Frames[nFramescount].TitleBar.hwnd;
	SendMessage(Frames[nFramescount].TitleBar.hwndTip,TTM_ADDTOOL,0,(LPARAM)&ti);

	SendMessage(Frames[nFramescount].TitleBar.hwndTip,TTM_ACTIVATE,(WPARAM)Frames[nFramescount].TitleBar.ShowTitleBarTip,0);

	Frames[nFramescount].oldstyles = GetWindowLongPtr(Frames[nFramescount].hWnd,GWL_STYLE);
	Frames[nFramescount].TitleBar.oldstyles = GetWindowLongPtr(Frames[nFramescount].TitleBar.hwnd,GWL_STYLE);
	//Frames[nFramescount].FloatingPos.x =

	retval = Frames[nFramescount].id;
	Frames[nFramescount].order = nFramescount+1;
	nFramescount++;

	CLUIFramesLoadFrameSettings(id2pos(retval));
	style = GetWindowLongPtr(Frames[nFramescount-1].hWnd,GWL_STYLE);
	style &= (~WS_BORDER);
	style |= ((Frames[nFramescount-1].UseBorder)?WS_BORDER:0);
	SetWindowLongPtr(Frames[nFramescount-1].hWnd,GWL_STYLE,style);

	if (Frames[nFramescount-1].order == 0){Frames[nFramescount-1].order = nFramescount;}
	ulockfrm();

	alclientFrame = -1;//recalc it
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);

	if (Frames[nFramescount-1].floating) {
		Frames[nFramescount-1].floating = FALSE;
		CLUIFrameSetFloat(retval,1);//lparam = 1 use stored width and height
	}

	return retval;
}

static INT_PTR CLUIFramesRemoveFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted)
		return -1;

	lockfrm();
	int pos = id2pos(wParam);

	if (pos<0||pos>nFramescount){ulockfrm();return(-1);}

	mir_free(Frames[pos].name);
	mir_free(Frames[pos].TitleBar.tbname);
	mir_free(Frames[pos].TitleBar.tooltip);

	DestroyWindow(Frames[pos].hWnd);
	Frames[pos].hWnd = (HWND)-1;
	DestroyWindow(Frames[pos].TitleBar.hwnd);
	Frames[pos].TitleBar.hwnd = (HWND)-1;
	DestroyWindow(Frames[pos].ContainerWnd);
	Frames[pos].ContainerWnd = (HWND)-1;
	DestroyMenu(Frames[pos].TitleBar.hmenu);

	RemoveItemFromList(pos,&Frames,&nFramescount);

	ulockfrm();
	InvalidateRect(pcli->hwndContactList,NULL,TRUE);
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	InvalidateRect(pcli->hwndContactList,NULL,TRUE);

	return 0;
}


int CLUIFramesForceUpdateTB(const FRAMEWND *Frame)
{
	if (Frame->TitleBar.hwnd != 0)
		RedrawWindow(Frame->TitleBar.hwnd,NULL,NULL,RDW_ALLCHILDREN|RDW_UPDATENOW|RDW_ERASE|RDW_INVALIDATE|RDW_FRAME);
	return 0;
}

int CLUIFramesForceUpdateFrame(const FRAMEWND *Frame)
{
	if (Frame->hWnd != 0) {
		RedrawWindow(Frame->hWnd,NULL,NULL,RDW_UPDATENOW|RDW_FRAME|RDW_ERASE|RDW_INVALIDATE);
		UpdateWindow(Frame->hWnd);
	}
	if (Frame->floating) {
		if (Frame->ContainerWnd != 0)
			RedrawWindow(Frame->ContainerWnd,NULL,NULL,RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_ERASE|RDW_INVALIDATE|RDW_FRAME);
	}
	return 0;
}

int CLUIFrameMoveResize(const FRAMEWND *Frame)
{
	//int b;
	// we need to show or hide the frame?
	if (Frame->visible&&(!Frame->needhide)) {
		ShowWindow(Frame->hWnd,SW_SHOW);
		ShowWindow(Frame->TitleBar.hwnd,Frame->TitleBar.ShowTitleBar == TRUE?SW_SHOW:SW_HIDE);
	}
	else {
		ShowWindow(Frame->hWnd,SW_HIDE);
		ShowWindow(Frame->TitleBar.hwnd,SW_HIDE);
		return 0;
	}

	// set frame position
	SetWindowPos(Frame->hWnd,NULL,Frame->wndSize.left,Frame->wndSize.top,
					Frame->wndSize.right-Frame->wndSize.left,
					Frame->wndSize.bottom-Frame->wndSize.top,SWP_NOZORDER|SWP_NOREDRAW);

	// set titlebar position
	if (Frame->TitleBar.ShowTitleBar)
		SetWindowPos(Frame->TitleBar.hwnd,NULL,Frame->wndSize.left,Frame->wndSize.top-TitleBarH-GapBetweenTitlebar,
					Frame->wndSize.right-Frame->wndSize.left,
					TitleBarH,SWP_NOZORDER|SWP_NOREDRAW	);

	return 0;
}

BOOLEAN CLUIFramesFitInSize(void)
{
	int sumheight = 0;
	int tbh = 0; // title bar height

	int clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = TitleBarH*btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

	for (int i=0;i<nFramescount;i++) {
		if ((Frames[i].align != alClient)&&(!Frames[i].floating)&&(Frames[i].visible)&&(!Frames[i].needhide)) {
			sumheight += (Frames[i].height)+(TitleBarH*btoint(Frames[i].TitleBar.ShowTitleBar))+2;
			if (sumheight>ContactListHeight-tbh-2)
				return FALSE;
		}
	}
	return TRUE;
}

int CLUIFramesGetMinHeight()
{
	int i,tbh,clientfrm,sumheight = 0;
	RECT border;
	int allbord = 0;
	if (pcli->hwndContactList == NULL) return 0;
	lockfrm();

	// search for alClient frame and get the titlebar's height
	tbh = 0;
	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = TitleBarH*btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

	for (i=0; i < nFramescount; i++) {
		if ((Frames[i].align != alClient)&&(Frames[i].visible)&&(!Frames[i].needhide)&&(!Frames[i].floating)) {
			RECT wsize;

			GetWindowRect(Frames[i].hWnd,&wsize);
			sumheight += (wsize.bottom-wsize.top)+(TitleBarH*btoint(Frames[i].TitleBar.ShowTitleBar))+3;
		}
	}
	ulockfrm();
	GetBorderSize(pcli->hwndContactList,&border);
	return (sumheight+border.top+border.bottom+allbord+tbh+3);
}

int CLUIFramesResize(const RECT newsize)
{
	int sumheight = 9999999,newheight;
	int prevframe,prevframebottomline;
	int tbh,curfrmtbh;
	int drawitems;
	int clientfrm;
	int i,j;
	int sepw = GapBetweenFrames;
	SortData *sdarray;

	GapBetweenTitlebar = (int)db_get_dw(NULL,"CLUIFrames","GapBetweenTitleBar",1);
	GapBetweenFrames = db_get_dw(NULL,"CLUIFrames","GapBetweenFrames",1);
	TitleBarH = db_get_dw(NULL,"CLUIFrames","TitleBarH",DEFAULT_TITLEBAR_HEIGHT);

	if (nFramescount < 1)
		return 0;

	newheight = newsize.bottom-newsize.top;

	// search for alClient frame and get the titlebar's height
	tbh = 0;
	clientfrm = CLUIFramesGetalClientFrame();
	if (clientfrm != -1)
		tbh = (TitleBarH+GapBetweenTitlebar)*btoint(Frames[clientfrm].TitleBar.ShowTitleBar);

	for (i=0; i < nFramescount; i++) {
		if ( !Frames[i].floating) {
			Frames[i].needhide = FALSE;
			Frames[i].wndSize.left = 0;
			Frames[i].wndSize.right = newsize.right-0;
		}
	}

	//sorting stuff
	sdarray = (SortData*)malloc(sizeof(SortData)*nFramescount);
	if (sdarray == NULL){return(-1);}
	for (i=0;i<nFramescount;i++) {
		sdarray[i].order = Frames[i].order;
		sdarray[i].realpos = i;
	}
	qsort(sdarray,nFramescount,sizeof(SortData),sortfunc);

	drawitems = nFramescount;

	while(sumheight>(newheight-tbh)&&drawitems>0) {
		sumheight = 0;
		drawitems = 0;
		for (i=0;i<nFramescount;i++)	{
			if (((Frames[i].align != alClient))&&(!Frames[i].floating)&&(Frames[i].visible)&&(!Frames[i].needhide)) {
				drawitems++;
				curfrmtbh = (TitleBarH+GapBetweenTitlebar)*btoint(Frames[i].TitleBar.ShowTitleBar);
				sumheight += (Frames[i].height)+curfrmtbh+sepw+(Frames[i].UseBorder?2:0);
				if (sumheight>newheight-tbh) {
					sumheight -= (Frames[i].height)+curfrmtbh+sepw;
					Frames[i].needhide = TRUE;
					drawitems--;
					break;
				}
			}
		}
	}

	prevframe = -1;
	prevframebottomline = 0;
	for (j = 0;j<nFramescount;j++) {
		//move all alTop frames
		i = sdarray[j].realpos;
		if ((!Frames[i].needhide)&&(!Frames[i].floating)&&(Frames[i].visible)&&(Frames[i].align == alTop)) {
			curfrmtbh = (TitleBarH+GapBetweenTitlebar)*btoint(Frames[i].TitleBar.ShowTitleBar);
			Frames[i].wndSize.top = prevframebottomline+sepw+(curfrmtbh);
			Frames[i].wndSize.bottom = Frames[i].height+Frames[i].wndSize.top+(Frames[i].UseBorder?2:0);
			Frames[i].prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = Frames[i].wndSize.bottom;
			if (prevframebottomline>newheight){
				//prevframebottomline -= Frames[i].height+(curfrmtbh+1);
				//Frames[i].needhide = TRUE;
			}
		}
	}

	if (sumheight<newheight) {
		for (j = 0;j<nFramescount;j++)	{
			//move alClient frame
			i = sdarray[j].realpos;
			if ((!Frames[i].needhide)&&(!Frames[i].floating)&&(Frames[i].visible)&&(Frames[i].align == alClient)) {
				int oldh;
				Frames[i].wndSize.top = prevframebottomline+sepw+(tbh);
				Frames[i].wndSize.bottom = Frames[i].wndSize.top+newheight-sumheight-tbh-sepw;

				oldh = Frames[i].height;
				Frames[i].height = Frames[i].wndSize.bottom-Frames[i].wndSize.top;
				Frames[i].prevvisframe = prevframe;
				prevframe = i;
				prevframebottomline = Frames[i].wndSize.bottom;
				if (prevframebottomline>newheight) {
					//prevframebottomline -= Frames[i].height+(tbh+1);
					//Frames[i].needhide = TRUE;
				}
				break;
			}
		}
	}

	//newheight
	prevframebottomline = newheight+sepw;
	//prevframe = -1;
	for (j = nFramescount-1;j>=0;j--) {
		//move all alBottom frames
		i = sdarray[j].realpos;
		if ((Frames[i].visible)&&(!Frames[i].floating)&&(!Frames[i].needhide)&&(Frames[i].align == alBottom)) {
			curfrmtbh = (TitleBarH+GapBetweenTitlebar)*btoint(Frames[i].TitleBar.ShowTitleBar);

			Frames[i].wndSize.bottom = prevframebottomline-sepw;
			Frames[i].wndSize.top = Frames[i].wndSize.bottom-Frames[i].height-(Frames[i].UseBorder?2:0);
			Frames[i].prevvisframe = prevframe;
			prevframe = i;
			prevframebottomline = Frames[i].wndSize.top/*-1*/-curfrmtbh;
			if (prevframebottomline>newheight) {

			}
		}
	}

	if (sdarray != NULL) {
		free(sdarray);
		sdarray = NULL;
	}

	for (i=0;i<nFramescount;i++) {
		if (Frames[i].floating)
			CLUIFrameResizeFloatingFrame(i);
		else
			CLUIFrameMoveResize(&Frames[i]);
	}
	return 0;
}

INT_PTR CLUIFramesUpdateFrame(WPARAM wParam, LPARAM lParam)
{
	if (FramesSysNotStarted) return -1;
	if (wParam == -1) { CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0); return 0;}
	if (lParam&FU_FMPOS)	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,1);
	lockfrm();
	int pos = id2pos(wParam);
	if (pos < 0 || pos >= nFramescount) { ulockfrm(); return -1;}
	if (lParam & FU_TBREDRAW)	CLUIFramesForceUpdateTB(&Frames[pos]);
	if (lParam & FU_FMREDRAW)	CLUIFramesForceUpdateFrame(&Frames[pos]);
	ulockfrm();

	return 0;
}

int CLUIFramesOnClistResize(WPARAM wParam, LPARAM lParam)
{
	RECT nRect,rcStatus;
	int tick,i;

	if (FramesSysNotStarted) return -1;
	lockfrm();

	GetClientRect(pcli->hwndContactList,&nRect);

	rcStatus.top = rcStatus.bottom = 0;

	nRect.bottom -= nRect.top;
	nRect.bottom -= (rcStatus.bottom-rcStatus.top);
	nRect.right -= nRect.left;
	nRect.left = 0;
	nRect.top = 0;
	ContactListHeight = nRect.bottom;

	tick = GetTickCount();

	CLUIFramesResize(nRect);

	for (i=0; i < nFramescount; i++) {
		CLUIFramesForceUpdateFrame(&Frames[i]);
		CLUIFramesForceUpdateTB(&Frames[i]);
	}

	//resizing = FALSE;
	ulockfrm();
	tick = GetTickCount()-tick;

	if (pcli->hwndContactList != 0) InvalidateRect(pcli->hwndContactList,NULL,TRUE);
	if (pcli->hwndContactList != 0) UpdateWindow(pcli->hwndContactList);

	Sleep(0);

	//dont save to database too many times
	if (GetTickCount() - LastStoreTick > 1000) {
		CLUIFramesStoreAllFrames();
		LastStoreTick = GetTickCount();
	}
	return 0;
}

static	HBITMAP hBmpBackground;
static int backgroundBmpUse;
static COLORREF bkColour;
static COLORREF SelBkColour;
boolean AlignCOLLIconToLeft; //will hide frame icon

int OnFrameTitleBarBackgroundChange(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;

	AlignCOLLIconToLeft = db_get_b(NULL,"FrameTitleBar","AlignCOLLIconToLeft",0);

	bkColour = db_get_dw(NULL,"FrameTitleBar","BkColour",CLCDEFAULT_BKCOLOUR);
	//SelBkColour = db_get_dw(NULL,"FrameTitleBar","SelBkColour",0);

	if (hBmpBackground) {DeleteObject(hBmpBackground); hBmpBackground = NULL;}
	if ( db_get_b(NULL,"FrameTitleBar","UseBitmap",CLCDEFAULT_USEBITMAP)) {
		if ( !db_get(NULL,"FrameTitleBar","BkBitmap",&dbv)) {
			hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP,0,(LPARAM)dbv.pszVal);
			mir_free(dbv.pszVal);
		}
	}
	backgroundBmpUse = db_get_w(NULL,"FrameTitleBar","BkBmpUse",CLCDEFAULT_BKBMPUSE);

	CLUIFramesOnClistResize(0,0);
	return 0;
}

void DrawBackGroundTTB(HWND hwnd,HDC mhdc)
{
	HDC hdcMem,hdc;
	RECT clRect,*rcPaint;

	int yScroll = 0;
	int y;
	PAINTSTRUCT paintst = {0};
	HBITMAP hBmpOsb,hOldBmp;
	DWORD style = GetWindowLongPtr(hwnd,GWL_STYLE);
	int grey = 0;
	HFONT oFont;
	HBRUSH hBrushAlternateGrey = NULL;

	HFONT hFont;

	//InvalidateRect(hwnd,0,FALSE);

	hFont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);

	if (mhdc) {
		hdc = mhdc;
		rcPaint = NULL;
	}
	else {
		hdc = BeginPaint(hwnd,&paintst);
		rcPaint = &(paintst.rcPaint);
	}

	GetClientRect(hwnd,&clRect);
	if (rcPaint == NULL) rcPaint = &clRect;
	if (rcPaint->right-rcPaint->left == 0||rcPaint->top-rcPaint->bottom == 0) rcPaint = &clRect;
	y = -yScroll;
	hdcMem = CreateCompatibleDC(hdc);
	hBmpOsb = CreateBitmap(clRect.right,clRect.bottom,1,GetDeviceCaps(hdc,BITSPIXEL),NULL);
	hOldBmp = (HBITMAP)SelectObject(hdcMem,hBmpOsb);
	oFont = (HFONT)SelectObject(hdcMem,hFont);
	SetBkMode(hdcMem,TRANSPARENT);
	{
		HBRUSH hBrush = CreateSolidBrush(bkColour);
		HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem,hBrush);
		FillRect(hdcMem,rcPaint,hBrush);
		SelectObject(hdcMem,hoBrush);
		DeleteObject(hBrush);
		if (hBmpBackground) {
			BITMAP bmp;
			HDC hdcBmp;
			int x,y;
			int maxx,maxy;
			int destw,desth;

			GetObject(hBmpBackground,sizeof(bmp),&bmp);
			hdcBmp = CreateCompatibleDC(hdcMem);
			SelectObject(hdcBmp,hBmpBackground);
			y = backgroundBmpUse&CLBF_SCROLL?-yScroll:0;
			maxx = backgroundBmpUse&CLBF_TILEH?clRect.right:1;
			maxy = backgroundBmpUse&CLBF_TILEV?maxy = rcPaint->bottom:y+1;
			switch(backgroundBmpUse&CLBM_TYPE) {
				case CLB_STRETCH:
					if (backgroundBmpUse&CLBF_PROPORTIONAL) {
						if (clRect.right*bmp.bmHeight<clRect.bottom*bmp.bmWidth) {
							desth = clRect.bottom;
							destw = desth*bmp.bmWidth/bmp.bmHeight;
						}
						else {
							destw = clRect.right;
							desth = destw*bmp.bmHeight/bmp.bmWidth;
						}
					}
					else {
						destw = clRect.right;
						desth = clRect.bottom;
					}
					break;
				case CLB_STRETCHH:
					if (backgroundBmpUse&CLBF_PROPORTIONAL) {
						destw = clRect.right;
						desth = destw*bmp.bmHeight/bmp.bmWidth;
					}
					else {
						destw = clRect.right;
						desth = bmp.bmHeight;
					}
					break;
				case CLB_STRETCHV:
					if (backgroundBmpUse&CLBF_PROPORTIONAL) {
						desth = clRect.bottom;
						destw = desth*bmp.bmWidth/bmp.bmHeight;
					}
					else {
						destw = bmp.bmWidth;
						desth = clRect.bottom;
					}
					break;
				default:    //clb_topleft
					destw = bmp.bmWidth;
					desth = bmp.bmHeight;
					break;
			}
			desth = clRect.bottom -clRect.top;
			for (;y<maxy;y += desth) {
				if (y<rcPaint->top-desth) continue;
				for (x = 0;x<maxx;x += destw)
					StretchBlt(hdcMem,x,y,destw,desth,hdcBmp,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
			}
			DeleteDC(hdcBmp);
		}
	}

	BitBlt(hdc,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,hdcMem,rcPaint->left,rcPaint->top,SRCCOPY);

	SelectObject(hdcMem,hOldBmp);
	SelectObject(hdcMem,oFont);
	DeleteObject(hBmpOsb);
	DeleteDC(hdcMem);
	paintst.fErase = FALSE;
	//DeleteObject(hFont);
	if ( !mhdc)
		EndPaint(hwnd,&paintst);
}

static int DrawTitleBar(HDC dc,RECT rect,int Frameid)
{
	HDC paintDC = dc;

	HDC hdcMem = CreateCompatibleDC(paintDC);
	HBITMAP hBmpOsb = CreateBitmap(rect.right,rect.bottom,1,GetDeviceCaps(paintDC,BITSPIXEL),NULL);
	HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem,hBmpOsb);

	HFONT hoTTBFont = (HFONT)SelectObject(hdcMem,TitleBarFont);
	SetBkMode(hdcMem,TRANSPARENT);

	HBRUSH hBack = GetSysColorBrush(COLOR_3DFACE);
	HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem,hBack);

	lockfrm();

	int pos = id2pos(Frameid);
	if (pos >= 0 && pos < nFramescount) {
		GetClientRect(Frames[pos].TitleBar.hwnd,&Frames[pos].TitleBar.wndSize);

		//set font charset
		HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (TitleBarLogFont.lfHeight != 0)
			hf = CreateFontIndirectA(&TitleBarLogFont);

		HFONT oFont = (HFONT)SelectObject(hdcMem,hf);

		DrawBackGroundTTB(Frames[pos].TitleBar.hwnd,hdcMem);
		//hFront = CreateSolidPe (SelBkColour);
		//SelectObject(hdcMem,hFront);
		SelBkColour = db_get_dw(NULL,"CLUIFrames","FramesTitleBarFontCol",0);
		if (SelBkColour) SetTextColor(hdcMem,SelBkColour);

		if ( !AlignCOLLIconToLeft) {

			if (Frames[pos].TitleBar.hicon != NULL)	{
				//(TitleBarH>>1)-(GetSystemMetrics(SM_CXSMICON)>>1)
				DrawIconEx(hdcMem,2,((TitleBarH>>1)-(GetSystemMetrics(SM_CYSMICON)>>1)),Frames[pos].TitleBar.hicon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
				TextOut(hdcMem,GetSystemMetrics(SM_CYSMICON)+4,0,Frames[pos].TitleBar.tbname,lstrlen(Frames[pos].TitleBar.tbname));
			}
			else
				TextOut(hdcMem,2,0,Frames[pos].TitleBar.tbname,lstrlen(Frames[pos].TitleBar.tbname));
		}
		else
			TextOut(hdcMem,GetSystemMetrics(SM_CXSMICON)+2,0,Frames[pos].TitleBar.tbname,lstrlen(Frames[pos].TitleBar.tbname));

		if ( !AlignCOLLIconToLeft)
			DrawIconEx(hdcMem,Frames[pos].TitleBar.wndSize.right-GetSystemMetrics(SM_CXSMICON)-2,((TitleBarH>>1)-(GetSystemMetrics(SM_CXSMICON)>>1)),Frames[pos].collapsed?LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN):LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT),GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
		else
			DrawIconEx(hdcMem,0,((TitleBarH>>1)-(GetSystemMetrics(SM_CXSMICON)>>1)),Frames[pos].collapsed?LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN):LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT),GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);

		DeleteObject(hf);
		SelectObject(hdcMem,oFont);
	}
	ulockfrm();

	BitBlt(paintDC,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,hdcMem,rect.left,rect.top,SRCCOPY);

	SelectObject(hdcMem,hoBmp);
	SelectObject(hdcMem,hoBrush);
	SelectObject(hdcMem,hoTTBFont);
	DeleteDC(hdcMem);
	DeleteObject(hBack);
	DeleteObject(hBmpOsb);
	return 0;
}

//for old multiwindow
#define MPCF_CONTEXTFRAMEMENU		3
POINT ptOld;
short	nLeft		 = 0;
short	nTop		 = 0;

LRESULT CALLBACK CLUIFrameTitleBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	int Frameid,Framemod,direction;
	int xpos,ypos;

	Frameid = (GetWindowLongPtr(hwnd,GWLP_USERDATA));
	memset(&rect,0,sizeof(rect));

	switch(msg) {
	case WM_CREATE:
		SendMessage(hwnd,WM_SETFONT,(WPARAM)TitleBarFont,0);
		return FALSE;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);

	case WM_ENABLE:
			if (hwnd != 0) InvalidateRect(hwnd,NULL,FALSE);
		return 0;

	case WM_COMMAND:
		if ( CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),0), Frameid))
			break;

		if (HIWORD(wParam) == 0) { //mouse events for self created menu
			int framepos = id2pos(Frameid);
			if (framepos == -1)
				break;

			switch(LOWORD(wParam)) {
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
				CLUIFrameSetFloat(Frameid,0);
				break;
			}
			CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
		}
		break;

	case WM_RBUTTONDOWN:
		{
			HMENU hmenu;
			POINT pt;
			GetCursorPos(&pt);

			if (ServiceExists(MS_CLIST_MENUBUILDFRAMECONTEXT))
				hmenu = (HMENU)CallService(MS_CLIST_MENUBUILDFRAMECONTEXT,Frameid,0);
			else {
				//legacy menu support
				int framepos = id2pos(Frameid);
				lockfrm();
				if (framepos == -1){ulockfrm();break;}
				hmenu = CreatePopupMenu();

				AppendMenu(hmenu,MF_STRING|MF_DISABLED|MF_GRAYED,15,Frames[framepos].name);
				AppendMenu(hmenu,MF_SEPARATOR,16,_T(""));

				if (Frames[framepos].Locked)
				{AppendMenu(hmenu,MF_STRING|MF_CHECKED,frame_menu_lock,TranslateT("Lock frame"));}
				else{AppendMenu(hmenu,MF_STRING,frame_menu_lock,TranslateT("Lock frame"));}

				if (Frames[framepos].visible)
				{AppendMenu(hmenu,MF_STRING|MF_CHECKED,frame_menu_visible,TranslateT("Visible"));}
				else{AppendMenu(hmenu,MF_STRING,frame_menu_visible,TranslateT("Visible"));}

				if (Frames[framepos].TitleBar.ShowTitleBar)
				{AppendMenu(hmenu,MF_STRING|MF_CHECKED,frame_menu_showtitlebar,TranslateT("Show title bar"));}
				else{AppendMenu(hmenu,MF_STRING,frame_menu_showtitlebar,TranslateT("Show title bar"));}

				if (Frames[framepos].floating)
				{AppendMenu(hmenu,MF_STRING|MF_CHECKED,frame_menu_floating,TranslateT("Floating"));}
				else{AppendMenu(hmenu,MF_STRING,frame_menu_floating,TranslateT("Floating"));}

				ulockfrm();
			}

			TrackPopupMenu(hmenu,TPM_LEFTALIGN,pt.x,pt.y,0,hwnd,0);
			DestroyMenu(hmenu);
		}
		break;

	case WM_LBUTTONDBLCLK:
		Framemod = -1;
		lbypos = -1;oldframeheight = -1;ReleaseCapture();
		CallService(MS_CLIST_FRAMES_UCOLLFRAME,Frameid,0);
		lbypos = -1;oldframeheight = -1;ReleaseCapture();
		break;

	case WM_LBUTTONUP:
		if (GetCapture() != hwnd)
			break;
		curdragbar = -1;lbypos = -1;oldframeheight = -1;ReleaseCapture();
		break;

	case WM_LBUTTONDOWN:
		{
			int framepos = id2pos(Frameid);
			if (framepos == -1)
				break;

			lockfrm();
			if (Frames[framepos].floating)
			{
				POINT pt;
				GetCursorPos(&pt);
				Frames[framepos].TitleBar.oldpos = pt;
			}

			if (( !(wParam & MK_CONTROL)) && Frames[framepos].Locked && (!(Frames[framepos].floating))) {
				if ( db_get_b(NULL,"CLUI","ClientAreaDrag",0)) {
					POINT pt;
					//pt = nm->pt;
					GetCursorPos(&pt);
					return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE|HTCAPTION,MAKELPARAM(pt.x,pt.y));
				}
			}

			if (Frames[framepos].floating) {
				RECT rc;
				GetCursorPos(&ptOld);
				//ClientToScreen(hwnd,&ptOld);
				GetWindowRect( hwnd, &rc );

				nLeft = (short)rc.left;
				nTop = (short)rc.top;
			}
			ulockfrm();
			SetCapture(hwnd);
		}
		break;

	case WM_MOUSEMOVE:
		{
			POINT pt,pt2;
			RECT wndr;
			int pos;

			char TBcapt[255];

			lockfrm();
			pos = id2pos(Frameid);

			if (pos != -1) {
				int oldflags;
				mir_snprintf(TBcapt, SIZEOF(TBcapt), "%s - h:%d, vis:%d, fl:%d, fl:(%d,%d,%d,%d),or: %d",
					Frames[pos].name, Frames[pos].height, Frames[pos].visible, Frames[pos].floating,
					Frames[pos].FloatingPos.x, Frames[pos].FloatingPos.y,
					Frames[pos].FloatingSize.x, Frames[pos].FloatingSize.y,
					Frames[pos].order);

				oldflags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,Frames[pos].id),0);
				if ( !(oldflags & F_SHOWTBTIP))
					oldflags |= F_SHOWTBTIP;
			}

			ulockfrm();

			if (wParam & MK_LBUTTON) {
				RECT rcMiranda;
				RECT rcwnd,rcOverlap;
				POINT newpt,ofspt,curpt,newpos;
				//if (GetCapture() != hwnd){break;}
				//curdragbar = -1;lbypos = -1;oldframeheight = -1;ReleaseCapture();
				lockfrm();
				pos = id2pos(Frameid);
				if (Frames[pos].floating) {

					GetCursorPos(&curpt);
					rcwnd.bottom = curpt.y+5;
					rcwnd.top = curpt.y;
					rcwnd.left = curpt.x;
					rcwnd.right = curpt.x+5;

					GetWindowRect(pcli->hwndContactList, &rcMiranda );
					//GetWindowRect( Frames[pos].ContainerWnd, &rcwnd );
					//IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )
					if (IsWindowVisible(pcli->hwndContactList) &&IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )) {
						int id = Frames[pos].id;
						ulockfrm();

						ofspt.x = 0;ofspt.y = 0;
						ClientToScreen(Frames[pos].TitleBar.hwnd,&ofspt);
						ofspt.x = curpt.x-ofspt.x;ofspt.y = curpt.y-ofspt.y;

						CLUIFrameSetFloat(id,0);
						newpt.x = 0;newpt.y = 0;
						ClientToScreen(Frames[pos].TitleBar.hwnd,&newpt);
						SetCursorPos(newpt.x+ofspt.x,newpt.y+ofspt.y);
						GetCursorPos(&curpt);
						lockfrm();
						Frames[pos].TitleBar.oldpos = curpt;
						ulockfrm();
						return 0;
					}
				}
				else {
					int id = Frames[pos].id;

					GetCursorPos(&curpt);
					rcwnd.bottom = curpt.y+5;
					rcwnd.top = curpt.y;
					rcwnd.left = curpt.x;
					rcwnd.right = curpt.x+5;

					GetWindowRect(pcli->hwndContactList, &rcMiranda );

					if ( !IntersectRect( &rcOverlap, &rcwnd, &rcMiranda )) {
						ulockfrm();
						GetCursorPos(&curpt);
						GetWindowRect( Frames[pos].hWnd, &rcwnd );
						rcwnd.left = rcwnd.right-rcwnd.left;
						rcwnd.top = rcwnd.bottom-rcwnd.top;
						newpos.x = curpt.x;newpos.y = curpt.y;
						if (curpt.x>=(rcMiranda.right-1)){newpos.x = curpt.x+5;}
						if (curpt.x<=(rcMiranda.left+1)){newpos.x = curpt.x-(rcwnd.left)-5;}

						if (curpt.y>=(rcMiranda.bottom-1)){newpos.y = curpt.y+5;}
						if (curpt.y<=(rcMiranda.top+1)){newpos.y = curpt.y-(rcwnd.top)-5;}


						ofspt.x = 0;ofspt.y = 0;
						//ClientToScreen(Frames[pos].TitleBar.hwnd,&ofspt);
						GetWindowRect(Frames[pos].TitleBar.hwnd,&rcwnd);
						ofspt.x = curpt.x-ofspt.x;ofspt.y = curpt.y-ofspt.y;

						Frames[pos].FloatingPos.x = newpos.x;
						Frames[pos].FloatingPos.y = newpos.y;
						CLUIFrameSetFloat(id,0);
						//SetWindowPos(Frames[pos].ContainerWnd,0,newpos.x,newpos.y,0,0,SWP_NOSIZE);

						lockfrm();
						newpt.x = 0;newpt.y = 0;
						ClientToScreen(Frames[pos].TitleBar.hwnd,&newpt);

						GetWindowRect( Frames[pos].hWnd, &rcwnd );
						SetCursorPos(newpt.x+(rcwnd.right-rcwnd.left)/2,newpt.y+(rcwnd.bottom-rcwnd.top)/2);
						GetCursorPos(&curpt);

						Frames[pos].TitleBar.oldpos = curpt;
						ulockfrm();

						return 0;
					}
				}
				ulockfrm();
			}

			if (wParam & MK_LBUTTON) {
				int newh = -1,prevold;

				if (GetCapture() != hwnd){break;}

				lockfrm();
				pos = id2pos(Frameid);

				if (Frames[pos].floating) {
					GetCursorPos(&pt);
					if ((Frames[pos].TitleBar.oldpos.x != pt.x)||(Frames[pos].TitleBar.oldpos.y != pt.y)) {

						pt2 = pt;
						ScreenToClient(hwnd,&pt2);
						GetWindowRect(Frames[pos].ContainerWnd,&wndr);

						int dX,dY;
						POINT ptNew;

						ptNew.x = pt.x;
						ptNew.y = pt.y;
						//ClientToScreen( hwnd, &ptNew );

						dX = ptNew.x - ptOld.x;
						dY = ptNew.y - ptOld.y;

						nLeft += (short)dX;
						nTop	+= (short)dY;

						if ( !(wParam & MK_CONTROL))
							PositionThumb( &Frames[pos], nLeft, nTop );
						else
							SetWindowPos(	Frames[pos].ContainerWnd,
								HWND_TOPMOST,
								nLeft,
								nTop,
								0,
								0,
								SWP_NOSIZE | SWP_NOZORDER );

						ptOld = ptNew;

						pt.x = nLeft;
						pt.y = nTop;
						Frames[pos].TitleBar.oldpos = pt;
					}

					ulockfrm();
					return 0;
				}

				if (Frames[pos].prevvisframe != -1) {
					GetCursorPos(&pt);

					if ((Frames[pos].TitleBar.oldpos.x == pt.x)&&(Frames[pos].TitleBar.oldpos.y == pt.y)) {
						ulockfrm();
						break;
					}

					ypos = rect.top+pt.y;xpos = rect.left+pt.x;
					Framemod = -1;

					if (Frames[pos].align == alBottom)	{
						direction = -1;
						Framemod = pos;
					}
					else {
						direction = 1;
						Framemod = Frames[pos].prevvisframe;
					}
					if (Frames[Framemod].Locked) {ulockfrm();break;}
					if (curdragbar != -1&&curdragbar != pos) {ulockfrm();break;}

					if (lbypos == -1) {
						curdragbar = pos;
						lbypos = ypos;
						oldframeheight = Frames[Framemod].height;
						SetCapture(hwnd);
						{ulockfrm();break;}
					}

					newh = oldframeheight+direction*(ypos-lbypos);
					if (newh > 0)	{
						prevold = Frames[Framemod].height;
						Frames[Framemod].height = newh;
						if ( !CLUIFramesFitInSize()) {
							Frames[Framemod].height = prevold;
							ulockfrm();
							return TRUE;
						}
						Frames[Framemod].height = newh;
						if (newh > 3)
							Frames[Framemod].collapsed = TRUE;
					}
					Frames[pos].TitleBar.oldpos = pt;
				}
				ulockfrm();
				if (newh > 0)
					CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
				break;
			}
			curdragbar = -1;lbypos = -1;oldframeheight = -1;ReleaseCapture();
		}
		break;

	case WM_PRINTCLIENT:
		if (lParam & PRF_CLIENT) {
			GetClientRect(hwnd,&rect);
			DrawTitleBar((HDC)wParam,rect,Frameid);
		}

	case WM_PAINT:
		{
			HDC paintDC;
			PAINTSTRUCT paintStruct;

			//GetClientRect(hwnd,&rect);
			paintDC = BeginPaint(hwnd, &paintStruct);
			rect = paintStruct.rcPaint;
			DrawTitleBar(paintDC,rect,Frameid);
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
	int width,height;
	RECT rect;

	if ( !Frames[framepos].floating){return 0;}
	if (Frames[framepos].ContainerWnd == 0){return 0;}
	GetClientRect(Frames[framepos].ContainerWnd,&rect);

	width = rect.right-rect.left;
	height = rect.bottom-rect.top;

	Frames[framepos].visible?ShowWindow(Frames[framepos].ContainerWnd,SW_SHOW):ShowWindow(Frames[framepos].ContainerWnd,SW_HIDE);

	if (Frames[framepos].TitleBar.ShowTitleBar) {
		ShowWindow(Frames[framepos].TitleBar.hwnd,SW_SHOW);
		Frames[framepos].height = height-DEFAULT_TITLEBAR_HEIGHT;

		SetWindowPos(Frames[framepos].TitleBar.hwnd,HWND_TOP,0,0,width,DEFAULT_TITLEBAR_HEIGHT,SWP_SHOWWINDOW|SWP_DRAWFRAME);
		SetWindowPos(Frames[framepos].hWnd,HWND_TOP,0,DEFAULT_TITLEBAR_HEIGHT,width,height-DEFAULT_TITLEBAR_HEIGHT,SWP_SHOWWINDOW);
	}
	else {
		Frames[framepos].height = height;
		ShowWindow(Frames[framepos].TitleBar.hwnd,SW_HIDE);
		SetWindowPos(Frames[framepos].hWnd,HWND_TOP,0,0,width,height,SWP_SHOWWINDOW);
	}

	if (Frames[framepos].ContainerWnd != 0)
		UpdateWindow(Frames[framepos].ContainerWnd);

	GetWindowRect(Frames[framepos].hWnd,&Frames[framepos].wndSize);
	return 0;
}

static int CLUIFrameOnMainMenuBuild(WPARAM wParam, LPARAM lParam)
{
	CLUIFramesLoadMainMenu();
	return 0;
}

LRESULT CALLBACK CLUIFrameContainerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect = { 0 };
	int Frameid = (GetWindowLongPtr(hwnd,GWLP_USERDATA));

	switch(msg) {
	case WM_CREATE:
		{
			lockfrm();
			int framepos = id2pos(Frameid);
			ulockfrm();
		}
		return 0;

	case WM_GETMINMAXINFO:
		{
			int framepos;
			MINMAXINFO minmax;

			lockfrm();
			framepos = id2pos(Frameid);
			if (framepos<0||framepos>=nFramescount){ulockfrm();break;}
			if ( !Frames[framepos].minmaxenabled){ulockfrm();break;}
			if (Frames[framepos].ContainerWnd == 0){ulockfrm();break;}

			if (Frames[framepos].Locked) {
				RECT rct;

				GetWindowRect(hwnd,&rct);
				((LPMINMAXINFO)lParam)->ptMinTrackSize.x = rct.right-rct.left;
				((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rct.bottom-rct.top;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = rct.right-rct.left;
				((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = rct.bottom-rct.top;
			}

			memset(&minmax,0,sizeof(minmax));
			if (SendMessage(Frames[framepos].hWnd,WM_GETMINMAXINFO,0,(LPARAM)&minmax) == 0) {
				RECT border;
				int tbh = TitleBarH*btoint(Frames[framepos].TitleBar.ShowTitleBar);
				GetBorderSize(hwnd,&border);
				if (minmax.ptMaxTrackSize.x != 0&&minmax.ptMaxTrackSize.y != 0){
					((LPMINMAXINFO)lParam)->ptMinTrackSize.x = minmax.ptMinTrackSize.x;
					((LPMINMAXINFO)lParam)->ptMinTrackSize.y = minmax.ptMinTrackSize.y;
					((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = minmax.ptMaxTrackSize.x+border.left+border.right;
					((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = minmax.ptMaxTrackSize.y+tbh+border.top+border.bottom;
				}
			}
			else {
				ulockfrm();
				return(DefWindowProc(hwnd, msg, wParam, lParam));
			}

			ulockfrm();
		}

	case WM_MOVE:
		{
			lockfrm();
			int framepos = id2pos(Frameid);

			if (framepos<0||framepos>=nFramescount){ulockfrm();break;}
			if (Frames[framepos].ContainerWnd == 0){ulockfrm();return 0;}

			RECT rect;
			GetWindowRect(Frames[framepos].ContainerWnd,&rect);
			Frames[framepos].FloatingPos.x = rect.left;
			Frames[framepos].FloatingPos.y = rect.top;
			Frames[framepos].FloatingSize.x = rect.right-rect.left;
			Frames[framepos].FloatingSize.y = rect.bottom-rect.top;

			CLUIFramesStoreFrameSettings(framepos);
			ulockfrm();
		}
		return 0;

	case WM_SIZE:
		{
			lockfrm();
			int framepos = id2pos(Frameid);

			if (framepos<0||framepos>=nFramescount){ulockfrm();break;}
			if (Frames[framepos].ContainerWnd == 0){ulockfrm();return 0;}
			CLUIFrameResizeFloatingFrame(framepos);

			RECT rect;
			GetWindowRect(Frames[framepos].ContainerWnd,&rect);
			Frames[framepos].FloatingPos.x = rect.left;
			Frames[framepos].FloatingPos.y = rect.top;
			Frames[framepos].FloatingSize.x = rect.right-rect.left;
			Frames[framepos].FloatingSize.y = rect.bottom-rect.top;

			CLUIFramesStoreFrameSettings(framepos);
			ulockfrm();
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

static HWND CreateContainerWindow(HWND parent,int x,int y,int width,int height)
{
	return(CreateWindowA("FramesContainer","aaaa",WS_POPUP|WS_THICKFRAME,x,y,width,height,parent,0,g_hInst,0));
}

INT_PTR CLUIFrameSetFloat(WPARAM wParam, LPARAM lParam)
{
	HWND hwndtmp, hwndtooltiptmp;
	FRAMEWND *frame;
	int pos;

	lockfrm();

	pos = id2pos(wParam);
	if (pos < 0 || pos > nFramescount) {
		ulockfrm();
		return 0;
	}

	frame = &Frames[pos];

	//parent = GetParent(Frames[wParam].hWnd);
	if (frame->floating) {
		SetParent(frame->hWnd,pcli->hwndContactList);
		SetParent(frame->TitleBar.hwnd,pcli->hwndContactList);
		frame->floating = FALSE;
		DestroyWindow(frame->ContainerWnd);
		frame->ContainerWnd = NULL;
	}
	else {
		RECT recttb,rectw,border;
		LONG_PTR temp;
		int neww,newh;
		BOOLEAN locked;

		frame->oldstyles = GetWindowLongPtr(frame->hWnd,GWL_STYLE);
		frame->TitleBar.oldstyles = GetWindowLongPtr(frame->TitleBar.hwnd,GWL_STYLE);
		locked = frame->Locked;
		frame->Locked = FALSE;
		frame->minmaxenabled = FALSE;

		GetWindowRect(frame->hWnd,&rectw);
		GetWindowRect(frame->TitleBar.hwnd,&recttb);
		if ( !frame->TitleBar.ShowTitleBar)
			recttb.top = recttb.bottom = recttb.left = recttb.right = 0;

		frame->ContainerWnd = CreateContainerWindow(pcli->hwndContactList,frame->FloatingPos.x,frame->FloatingPos.y,10,10);

		SetParent(frame->hWnd,frame->ContainerWnd);
		SetParent(frame->TitleBar.hwnd,frame->ContainerWnd);

		GetBorderSize(frame->ContainerWnd,&border);

		SetWindowLongPtr(frame->ContainerWnd, GWLP_USERDATA, frame->id);
		if ((lParam == 1)) {
			if ((frame->FloatingPos.x != 0)&&(frame->FloatingPos.y != 0)) {
				if (frame->FloatingPos.x<20){frame->FloatingPos.x = 40;}
				if (frame->FloatingPos.y<20){frame->FloatingPos.y = 40;}

				SetWindowPos(frame->ContainerWnd,HWND_TOPMOST,frame->FloatingPos.x,frame->FloatingPos.y,frame->FloatingSize.x,frame->FloatingSize.y,SWP_HIDEWINDOW);
			}
			else SetWindowPos(frame->ContainerWnd,HWND_TOPMOST,120,120,140,140,SWP_HIDEWINDOW);
		}
		else {
			neww = rectw.right-rectw.left+border.left+border.right;
			newh = (rectw.bottom-rectw.top)+(recttb.bottom-recttb.top)+border.top+border.bottom;
			if (neww<20){neww = 40;}
			if (newh<20){newh = 40;}
			if (frame->FloatingPos.x<20){frame->FloatingPos.x = 40;}
			if (frame->FloatingPos.y<20){frame->FloatingPos.y = 40;}

			SetWindowPos(frame->ContainerWnd,HWND_TOPMOST,frame->FloatingPos.x,frame->FloatingPos.y,neww,newh,SWP_HIDEWINDOW);
		}

		SetWindowText(frame->ContainerWnd,frame->TitleBar.tbname);

		temp = GetWindowLongPtr(frame->ContainerWnd,GWL_EXSTYLE);
		temp |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST ;
		SetWindowLongPtr(frame->ContainerWnd,GWL_EXSTYLE,temp);

		frame->floating = TRUE;
		frame->Locked = locked;
	}

	CLUIFramesStoreFrameSettings(pos);
	frame->minmaxenabled = TRUE;
	hwndtooltiptmp = frame->TitleBar.hwndTip;

	hwndtmp = frame->ContainerWnd;
	ulockfrm();
	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList, 0);
	SendMessage(hwndtmp,WM_SIZE,0,0);

	SetWindowPos(hwndtooltiptmp, HWND_TOPMOST,0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE  );
	return 0;
}

static int CLUIFrameOnFontChange(WPARAM wParam, LPARAM lParam)
{
	FontID fid = {0};
	fid.cbSize = sizeof(fid);
	memset(&TitleBarLogFont,0,sizeof(TitleBarLogFont));

	strcpy(fid.group,LPGEN("Frames"));
	strcpy(fid.name,LPGEN("TitleBarFont"));
	strcpy(fid.dbSettingsGroup,"CLUIFrames");
	strcpy(fid.prefix,"FramesTitleBarFont");

	CallService(MS_FONT_GET,(WPARAM)&fid,(LPARAM)&TitleBarLogFont);
	CLUIFramesOnClistResize(0,0);
	return 0;
}

static void CLUIRegisterFonts()
{
	FontID fid = {0};
	fid.cbSize = sizeof(fid);
	strcpy(fid.group, LPGEN("Frames"));
	strcpy(fid.name, LPGEN("TitleBarFont"));
	strcpy(fid.dbSettingsGroup, "CLUIFrames");
	strcpy(fid.prefix, "FramesTitleBarFont");
	FontRegister(&fid);

	CLUIFrameOnFontChange(0,0);

	HookEvent(ME_FONT_RELOAD,CLUIFrameOnFontChange);
}

static int CLUIFrameOnModulesLoad(WPARAM wParam, LPARAM lParam)
{
	CLUIFramesLoadMainMenu();
	CLUIFramesCreateMenuForFrame(-1,-1,000010000,Menu_AddContextFrameMenuItem);
	CLUIRegisterFonts();
	return 0;
}

static int CLUIFrameOnModulesUnload(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIVisible, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMITitle, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMITBVisible, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMILock, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIColl, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIFloating, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIAlignRoot, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIAlignTop, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIAlignClient, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIAlignBottom, 0 );
	CallService(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, ( LPARAM )contMIBorder, 0 );
   return 0;
}

int LoadCLUIFramesModule(void)
{
	WNDCLASS wndclass;
	wndclass.style         = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW ;
	wndclass.lpfnWndProc   = CLUIFrameTitleBarProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = CLUIFrameTitleBarClassName;
	RegisterClass(&wndclass);

	//container helper
	WNDCLASS cntclass;
	cntclass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
	cntclass.lpfnWndProc   = CLUIFrameContainerWndProc;
	cntclass.cbClsExtra    = 0;
	cntclass.cbWndExtra    = 0;
	cntclass.hInstance     = g_hInst;
	cntclass.hIcon         = NULL;
	cntclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	cntclass.hbrBackground = NULL;
	cntclass.lpszMenuName  = NULL;
	cntclass.lpszClassName = _T("FramesContainer");
	RegisterClass(&cntclass);
	//end container helper

	GapBetweenFrames = db_get_dw(NULL,"CLUIFrames","GapBetweenFrames",1);

	nFramescount = 0;
	InitializeCriticalSection(&csFrameHook);
	InitFramesMenus();

	HookEvent(ME_SYSTEM_MODULESLOADED,CLUIFrameOnModulesLoad);
	HookEvent(ME_CLIST_PREBUILDFRAMEMENU,CLUIFramesModifyContextMenuForFrame);
	HookEvent(ME_CLIST_PREBUILDMAINMENU,CLUIFrameOnMainMenuBuild);

	CreateServiceFunction(MS_CLIST_FRAMES_ADDFRAME,CLUIFramesAddFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_REMOVEFRAME,CLUIFramesRemoveFrame);

	CreateServiceFunction(MS_CLIST_FRAMES_SETFRAMEOPTIONS,CLUIFramesSetFrameOptions);
	CreateServiceFunction(MS_CLIST_FRAMES_GETFRAMEOPTIONS,CLUIFramesGetFrameOptions);
	CreateServiceFunction(MS_CLIST_FRAMES_UPDATEFRAME,CLUIFramesUpdateFrame);

	CreateServiceFunction(MS_CLIST_FRAMES_SHFRAMETITLEBAR,CLUIFramesShowHideFrameTitleBar);
	CreateServiceFunction(MS_CLIST_FRAMES_SHOWALLFRAMESTB,CLUIFramesShowAllTitleBars);
	CreateServiceFunction(MS_CLIST_FRAMES_HIDEALLFRAMESTB,CLUIFramesHideAllTitleBars);
	CreateServiceFunction(MS_CLIST_FRAMES_SHFRAME,CLUIFramesShowHideFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_SHOWALLFRAMES,CLUIFramesShowAll);

	CreateServiceFunction(MS_CLIST_FRAMES_ULFRAME,CLUIFramesLockUnlockFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_UCOLLFRAME,CLUIFramesCollapseUnCollapseFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_SETUNBORDER,CLUIFramesSetUnSetBorder);

	CreateServiceFunction(CLUIFRAMESSETALIGN,CLUIFramesSetAlign);
	CreateServiceFunction(CLUIFRAMESMOVEUPDOWN,CLUIFramesMoveUpDown);

	CreateServiceFunction(CLUIFRAMESSETALIGNALTOP,CLUIFramesSetAlignalTop);
	CreateServiceFunction(CLUIFRAMESSETALIGNALCLIENT,CLUIFramesSetAlignalClient);
	CreateServiceFunction(CLUIFRAMESSETALIGNALBOTTOM,CLUIFramesSetAlignalBottom);

	CreateServiceFunction("Set_Floating",CLUIFrameSetFloat);
	hWndExplorerToolBar = FindWindowExA(0,0,"Shell_TrayWnd",NULL);
	OnFrameTitleBarBackgroundChange(0,0);
	FramesSysNotStarted = FALSE;
	HookEvent(ME_SYSTEM_PRESHUTDOWN,  CLUIFrameOnModulesUnload);
	return 0;
}

int UnLoadCLUIFramesModule(void)
{
	FramesSysNotStarted = TRUE;

	CLUIFramesOnClistResize((WPARAM)pcli->hwndContactList,0);
	CLUIFramesStoreAllFrames();
	lockfrm();
	for (int i=0; i < nFramescount; i++) {
		DestroyWindow(Frames[i].hWnd);
		Frames[i].hWnd = (HWND)-1;
		DestroyWindow(Frames[i].TitleBar.hwnd);
		Frames[i].TitleBar.hwnd = (HWND)-1;
		DestroyWindow(Frames[i].ContainerWnd);
		Frames[i].ContainerWnd = (HWND)-1;
		DestroyMenu(Frames[i].TitleBar.hmenu);

		mir_free(Frames[i].name);
		mir_free(Frames[i].TitleBar.tbname);
		mir_free(Frames[i].TitleBar.tooltip);
	}

	free(Frames);
	Frames = NULL;
	nFramescount = 0;
	UnregisterClass(CLUIFrameTitleBarClassName,g_hInst);
	DeleteObject(TitleBarFont);
	ulockfrm();
	DeleteCriticalSection(&csFrameHook);
	return 0;
}
