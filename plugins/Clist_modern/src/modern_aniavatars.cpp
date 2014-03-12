/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

// File contains implementation of animated avatars in contact list

***************************************************************************/

#include "hdr/modern_commonheaders.h"

#define IMMEDIATE_DRAW (!s_bSeparateWindow)

void GDIPlus_ExtractAnimatedGIF (TCHAR * szName, int width, int height, HBITMAP  * pBmp, int ** pframesDelay, int * pframesCount, SIZE * sizeAvatar);
BOOL GDIPlus_IsAnimatedGif (TCHAR * szName);

/* Next is module */
#define ANIAVAWINDOWCLASS _T("MirandaModernAniAvatar")
#define aacheck if (!s_bModuleStarted) return

#define AAO_HAS_BORDER	  0x01
#define AAO_ROUND_CORNERS 0x02
#define AAO_HAS_OVERLAY	  0x04
#define AAO_OPAQUE		  0x08

//messages
enum {
	AAM_FIRST = WM_USER,
	AAM_SETAVATAR ,					//sync		WPARAM: TCHAR * filename, LPARAM: SIZE * size, RESULT: actual size
	AAM_SETPOSITION,				//async		LPARAM: pointer to set pos info - the handler will empty it, RESULT: 0
	AAM_REDRAW,						//async
	AAM_STOP,						//async		stops animation, timer, hide window - prepeare for deleting
	AAM_PAUSE,						//sync		keep timer and window, but do not process painting -need before graphics change
	AAM_RESUME,						//async		remobe previous flag. repaints if required
	AAM_REMOVEAVATAR,				//sync		WPARAM: if y more then wParam, LPARAM: shift up to lParam( remove if values is same)
	AAM_SETPARENT,					//async	    WPARAM: handle of new parent window
	AAM_SELFDESTROY,				//sync
	AAM_RENDER,						//sync
	AAM_LAST,
};

struct ANIAVA_OBJECT
{
	MCONTACT hContact;
	HWND   hWindow;
	BOOL   bInvalidPos;
	BOOL   bToBeDeleted;
	DWORD  dwAvatarUniqId;
	SIZE   ObjectSize;
};

struct ANIAVA_INFO
{
	DWORD  dwAvatarUniqId;
	TCHAR *tcsFilename;
	int    nRefCount;
	int    nStripTop;
	int    nFrameCount;
	int   *pFrameDelays;
	SIZE   FrameSize;
};

struct ANIAVA_WINDOWINFO
{
	HWND  hWindow;
	RECT  rcPos;
	SIZE  sizeAvatar;
	BOOL  StopTimer;
	int   TimerId;
	int   nFramesCount;
	int  *delaysInterval;
	int   currentFrame;

	POINT ptFromPoint;

	BOOL  bPlaying;
	int   overlayIconIdx;
	BYTE  bAlpha;
	BOOL  bOrderTop;

	BOOL  bPaused;			// was request do not draw
	BOOL  bPended;			// till do not draw - was painting - need to be repaint
};

struct ANIAVA_POSINFO
{
	RECT rcPos;
	int  idxOverlay;
	BYTE bAlpha;
};

struct ANIAVA_SYNCCALLITEM
{
	WPARAM  wParam;
	LPARAM  lParam;
	INT_PTR nResult;
	HANDLE  hDoneEvent;
	PSYNCCALLBACKPROC pfnProc;
};

struct ANIAVATARIMAGEINFO
{
	POINT ptImagePos;
	int	nFramesCount;
	int  *pFrameDelays;
	SIZE	szSize;
};

//protection
static BOOL s_bModuleStarted;
static CRITICAL_SECTION s_CS;

//options
static BYTE s_bFlags;				// 0x1 has border, 0x2 has round corners, 0x4 has overlay, 0x8 background color
static BYTE s_cornerRadius;
static COLORREF s_borderColor;
static COLORREF s_bkgColor;
static HIMAGELIST s_overlayIconImageList;

//animations
static HDC     s_hAniAvaDC;
static HBITMAP s_hAniAvaBitmap;
static HBITMAP s_hAniAvaOldBitmap;
static int     s_width, s_height;
static UINT    s_AnimationThreadID;
static HANDLE  s_AnimationThreadHandle;
static HANDLE  s_hExitEvent;

static int  _AniAva_SortAvatarInfo(const ANIAVA_INFO *first, const ANIAVA_INFO *last);
static LIST<ANIAVA_INFO> s_AniAvatarList(1, _AniAva_SortAvatarInfo);

//Objects
static LIST<ANIAVA_OBJECT> s_Objects(2);
static BOOL s_bSeparateWindow;

//module static declarations
static void _AniAva_DestroyAvatarWindow(HWND hwnd);
static void _AniAva_Clear_ANIAVA_WINDOWINFO(ANIAVA_WINDOWINFO *pavwi);
static void _AniAva_RenderAvatar(ANIAVA_WINDOWINFO *dat, HDC hdcParent = NULL, RECT *rcInParent = NULL);
static void _AniAva_PausePainting();
static void _AniAva_ResumePainting();
static void _AniAva_LoadOptions();
static void _AniAva_ReduceAvatarImages(int startY, int dY, BOOL bDestroyWindow);
static void _AniAva_RemoveAniAvaDC();
static void _AniAva_RealRemoveAvatar(DWORD UniqueID);
static int  _AniAva_LoadAvatarFromImage(TCHAR * szFileName, int width, int height, ANIAVATARIMAGEINFO *pRetAII);
static BOOL _AniAva_GetAvatarImageInfo(DWORD dwAvatarUniqId, ANIAVATARIMAGEINFO *avii);
static HWND _AniAva_CreateAvatarWindowSync(TCHAR *szFileName);

static LRESULT CALLBACK _AniAva_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

///	IMPLEMENTATION

int _AniAva_OnModulesUnload(WPARAM wParam, LPARAM lParam)
{
	SetEvent(s_hExitEvent);
	return 0;
}

static void _AniAva_AnimationTreadProc(void*)
{
	//wait forever till hExitEvent signalled
	HANDLE hThread = 0;
	s_AnimationThreadID = GetCurrentThreadId();
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
	s_AnimationThreadHandle = hThread;
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	for (;;) {
		DWORD rc = MsgWaitForMultipleObjectsEx(1, &s_hExitEvent, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
		ResetEvent(s_hExitEvent);
		if (rc == WAIT_OBJECT_0 + 1) {
			MSG msg;
			while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if ( IsDialogMessage(msg.hwnd, &msg)) continue;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (rc == WAIT_OBJECT_0)
			break;
	}
	CloseHandle(s_AnimationThreadHandle);
	s_AnimationThreadHandle = NULL;
}

// Init AniAva module
int AniAva_InitModule()
{
	if (!(db_get_b(NULL, "CList", "AvatarsAnimated", ServiceExists(MS_AV_GETAVATARBITMAP)) &&
		    db_get_b(NULL,"CList","AvatarsShow",SETTINGS_SHOWAVATARS_DEFAULT)))
		 return 0;

	WNDCLASSEX wc = { sizeof(wc) };
	wc.lpszClassName = ANIAVAWINDOWCLASS;
	wc.lpfnWndProc = _AniAva_WndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(ANIAVA_WINDOWINFO*);
	wc.style = CS_GLOBALCLASS;
	RegisterClassEx(&wc);

	InitializeCriticalSection(&s_CS);
	s_bModuleStarted = TRUE;
	s_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	mir_forkthread(_AniAva_AnimationTreadProc, NULL);
	HookEvent(ME_SYSTEM_PRESHUTDOWN,  _AniAva_OnModulesUnload);

	_AniAva_LoadOptions();
	return 1;
}

// Unload AniAva module
int AniAva_UnloadModule()
{
	aacheck 0;
	{
		mir_cslock lck(s_CS);
		int i;
		s_bModuleStarted = FALSE;
		for (i=0; i < s_Objects.getCount(); i++) {
			_AniAva_DestroyAvatarWindow(s_Objects[i]->hWindow);
			mir_free(s_Objects[i]);
		}

		for (i=0; i < s_AniAvatarList.getCount(); i++) {
			ANIAVA_INFO *aai = s_AniAvatarList[i];
			mir_free(aai->tcsFilename);
			free(aai->pFrameDelays);
			mir_free(aai);
		}

		_AniAva_RemoveAniAvaDC();
		SetEvent(s_hExitEvent);
		CloseHandle(s_hExitEvent);
	}
	DeleteCriticalSection(&s_CS);
	return 1;
}
// Update options
int AniAva_UpdateOptions()
{
	BOOL bReloadAvatars = FALSE;
	BOOL bBeEnabled = (db_get_b(NULL, "CList", "AvatarsAnimated", ServiceExists(MS_AV_GETAVATARBITMAP)) &&
		db_get_b(NULL,"CList","AvatarsShow",SETTINGS_SHOWAVATARS_DEFAULT));
	if (bBeEnabled && !s_bModuleStarted) {
		AniAva_InitModule();
		bReloadAvatars = TRUE;
	}
	else if (!bBeEnabled && s_bModuleStarted) {
		AniAva_UnloadModule();
		bReloadAvatars = TRUE;
	}

	BOOL oldSeparate = s_bSeparateWindow;
	_AniAva_LoadOptions();
	if (oldSeparate != s_bSeparateWindow) {
		AniAva_InvalidateAvatarPositions(NULL);
		AniAva_RemoveInvalidatedAvatars();
	}

	if (bReloadAvatars)
		PostMessage(pcli->hwndContactTree, INTM_AVATARCHANGED, 0, 0);
	else
		AniAva_RedrawAllAvatars(TRUE);
	return 0;
}

// adds avatars to be displayed
int AniAva_AddAvatar(MCONTACT hContact, TCHAR * szFilename, int width, int heigth)
{
	aacheck 0;
	if (!GDIPlus_IsAnimatedGif (szFilename))
		return 0;

	mir_cslock lck(s_CS);
	//first try to find window for contact avatar
	HWND hwnd = NULL;
	int i;
	ANIAVA_OBJECT * pavi;
	ANIAVATARIMAGEINFO avii = {0};
	SIZE szAva = { width, heigth };
	for (i=0; i < s_Objects.getCount(); i++) {
		pavi = s_Objects[i];
		if (pavi->hContact == hContact) {
			if (pavi->ObjectSize.cx == width && pavi->ObjectSize.cy == heigth) {
				hwnd = pavi->hWindow;
				break;
			}
			else {
				_AniAva_DestroyAvatarWindow(pavi->hWindow);
				pavi->hWindow = NULL;
				_AniAva_RealRemoveAvatar(pavi->dwAvatarUniqId);
				pavi->dwAvatarUniqId = 0;
				break;
			}
		}
	}
	if (i == s_Objects.getCount()) {
		pavi = (ANIAVA_OBJECT*) mir_calloc( sizeof(ANIAVA_OBJECT));
		pavi->hContact = hContact;
		s_Objects.insert(pavi);
	}
	//change avatar
	pavi->bToBeDeleted = FALSE;
	pavi->bInvalidPos	 = 0;
	// now CreateAvatar
	if (pavi->dwAvatarUniqId)
		_AniAva_GetAvatarImageInfo(pavi->dwAvatarUniqId,&avii);
	else
		pavi->dwAvatarUniqId = _AniAva_LoadAvatarFromImage(szFilename, width, heigth, &avii);
	if (hwnd)
		SendMessage(hwnd, AAM_SETAVATAR, (WPARAM)&avii, (LPARAM) 0);
	pavi->ObjectSize = avii.szSize;
	return MAKELONG(avii.szSize.cx, avii.szSize.cy);
}

// call windows to set they parent in order to ensure valid zorder
void AniAva_UpdateParent()
{
	aacheck;
	mir_cslock lck(s_CS);
	HWND parent = GetAncestor(pcli->hwndContactList, GA_PARENT);
	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT * pai = (ANIAVA_OBJECT *)s_Objects[i];
		SendMessage(pai->hWindow, AAM_SETPARENT, (WPARAM)parent,0);
	}
}

ANIAVA_OBJECT * FindAvatarByContact(MCONTACT hContact )
{
	for ( int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = ((ANIAVA_OBJECT *)s_Objects[i]);
		if (pai->hContact == hContact)
			return pai;
	}
	return NULL;
}

int AniAva_RenderAvatar(MCONTACT hContact, HDC hdcMem, RECT *rc )
{
	aacheck 0;
	mir_cslock lck(s_CS);
	ANIAVA_OBJECT *pai = FindAvatarByContact( hContact );
	if (pai)
		SendMessage(pai->hWindow, AAM_RENDER, (WPARAM)hdcMem, (LPARAM) rc);
	return 0;
}

// update avatars pos
int AniAva_SetAvatarPos(MCONTACT hContact, RECT *rc, int overlayIdx, BYTE bAlpha)
{
	aacheck 0;
	mir_cslock lck(s_CS);
	if (s_CS.LockCount > 0)
		return 0;

	ANIAVA_OBJECT *pai = FindAvatarByContact( hContact );
	if (pai) {
		ANIAVA_POSINFO * api = (ANIAVA_POSINFO *)malloc(sizeof(ANIAVA_POSINFO));
		if (!pai->hWindow) {
			HWND hwnd;
			HWND parent;
			ANIAVATARIMAGEINFO avii = {0};
			//not found -> create window
			char szName[150] = "AniAvaWnd_";
			TCHAR * tszName;
			_itoa((int)hContact,szName+10, 16);

			tszName = mir_a2t( szName );
			hwnd = _AniAva_CreateAvatarWindowSync(tszName);
			mir_free( tszName );
			parent = GetAncestor(pcli->hwndContactList, GA_PARENT);
			pai->hWindow = hwnd;
			SendMessage(hwnd,AAM_SETPARENT,(WPARAM)parent,0);
			if (_AniAva_GetAvatarImageInfo(pai->dwAvatarUniqId,&avii))
				SendMessage(pai->hWindow, AAM_SETAVATAR, (WPARAM)&avii, (LPARAM) 0);
		}
		api->bAlpha = bAlpha;
		api->idxOverlay = overlayIdx;
		api->rcPos = *rc;
		SendNotifyMessage(pai->hWindow, AAM_SETPOSITION, 0, (LPARAM) api);
		// the AAM_SETPOSITION is responsible to destroy memory under api
		pai->bInvalidPos = FALSE;
		pai->bToBeDeleted = FALSE;
	}
	return 1;
}
// remove avatar
int AniAva_RemoveAvatar(MCONTACT hContact)
{
	aacheck 0;
	mir_cslock lck(s_CS);

	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = s_Objects[i];
		if (pai->hContact == hContact) {
			pai->bToBeDeleted = TRUE;
			break;
		}
	}
	return 1;
}
// reset positions of avatars to be drawn (still be painted at same place)
int AniAva_InvalidateAvatarPositions(MCONTACT hContact)
{
	aacheck 0;
	mir_cslock lck(s_CS);
	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = s_Objects[i];
		if (pai->hContact == hContact || !hContact) {
			pai->bInvalidPos++;
			if (hContact)
				break;
		}
	}
	return 1;
}
// all avatars without validated position will be stop painted and probably removed
int AniAva_RemoveInvalidatedAvatars()
{
	aacheck 0;
	mir_cslock lck(s_CS);

	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = (ANIAVA_OBJECT *)s_Objects[i];
		if (pai->hWindow && (pai->bInvalidPos)) {
			SendMessage(pai->hWindow,AAM_STOP, 0, 0);
			if (pai->bInvalidPos) {
				pai->bInvalidPos = 0;
				_AniAva_DestroyAvatarWindow(pai->hWindow);
				pai->hWindow = NULL;
			}
		}
		if (pai->bToBeDeleted) {
			if (pai->hWindow) {
				_AniAva_DestroyAvatarWindow(pai->hWindow);
				pai->hWindow = NULL;
			}
			_AniAva_RealRemoveAvatar(pai->dwAvatarUniqId);
			mir_free(pai);
			s_Objects.remove(i);
			i--;
		}
	}
	return 1;
}

// repaint all avatars at positions (eg on main window movement)
int AniAva_RedrawAllAvatars(BOOL updateZOrder)
{
	aacheck 0;
	mir_cslock lck(s_CS);
	updateZOrder = 1;
	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = (ANIAVA_OBJECT *)s_Objects[i];
		if (updateZOrder)
			SendMessage(pai->hWindow,AAM_REDRAW,(WPARAM)updateZOrder,0);
		else
			SendNotifyMessage(pai->hWindow,AAM_REDRAW,(WPARAM)updateZOrder,0);
	}
	return 1;
}

//Static procedures
static void CALLBACK _AniAva_SyncCallerUserAPCProc(DWORD_PTR dwParam)
{
	ANIAVA_SYNCCALLITEM* item = (ANIAVA_SYNCCALLITEM*) dwParam;
	item->nResult = item->pfnProc(item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}
static INT_PTR _AniAva_CreateAvatarWindowSync_Worker(WPARAM tszName, LPARAM lParam)
{
	HWND hwnd = CreateWindowEx( WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOPARENTNOTIFY,ANIAVAWINDOWCLASS,(TCHAR*)tszName,WS_POPUP,
		0, 0, 1,1,pcli->hwndContactList, NULL, pcli->hInst, NULL );
	return (INT_PTR)hwnd;
}

static HWND _AniAva_CreateAvatarWindowSync(TCHAR *szFileName)
{
	if (s_AnimationThreadHandle == 0 || s_AnimationThreadID == 0)
		return NULL;

	ANIAVA_SYNCCALLITEM item = {0};
	item.wParam = (WPARAM)szFileName;
	item.pfnProc = _AniAva_CreateAvatarWindowSync_Worker;
	item.hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (GetCurrentThreadId() != s_AnimationThreadID)
		QueueUserAPC(_AniAva_SyncCallerUserAPCProc, s_AnimationThreadHandle, (DWORD_PTR) &item);
	else
		_AniAva_SyncCallerUserAPCProc((DWORD_PTR) &item);
	WaitForSingleObject(item.hDoneEvent, INFINITE);
	CloseHandle(item.hDoneEvent);
	return (HWND)item.nResult;
}

static void _AniAva_RealRemoveAvatar(DWORD UniqueID)
{
	for (int j = 0; j < s_AniAvatarList.getCount(); j++) {
		ANIAVA_INFO * aai = (ANIAVA_INFO *) s_AniAvatarList[j];
		if (aai->dwAvatarUniqId == UniqueID) {
			aai->nRefCount--;
			if (aai->nRefCount == 0) {
				_AniAva_PausePainting();

				mir_free(aai->tcsFilename);
				free(aai->pFrameDelays);
				_AniAva_ReduceAvatarImages(aai->nStripTop,aai->FrameSize.cx*aai->nFrameCount, FALSE);
				for (int k = 0; k < s_AniAvatarList.getCount(); k++)
					if (k != j)	{
						ANIAVA_INFO * taai = (ANIAVA_INFO *) s_AniAvatarList[k];
						if (taai->nStripTop>aai->nStripTop)
							taai->nStripTop -= aai->FrameSize.cx*aai->nFrameCount;
					}
				if (s_AniAvatarList.getCount() > 0) {
					int newWidth = s_width-aai->FrameSize.cx*aai->nFrameCount;
					int newHeight = 0;
					int i;
					for (i=0; i < s_AniAvatarList.getCount(); i++)
						if (i != j)
						{
							newHeight = max(newHeight,((ANIAVA_INFO *) s_AniAvatarList[i])->FrameSize.cy);
						}

					HDC hNewDC = CreateCompatibleDC(NULL);
					HBITMAP hNewBmp = ske_CreateDIB32(newWidth,newHeight);
					HBITMAP hNewOldBmp = (HBITMAP)SelectObject(hNewDC,hNewBmp);
					// copy from old and from new strip
					if (aai->nStripTop>0)
						BitBlt(hNewDC, 0, 0, aai->nStripTop,newHeight,s_hAniAvaDC, 0, 0, SRCCOPY);
					if (aai->nStripTop+aai->FrameSize.cx*aai->nFrameCount < s_width)
						BitBlt(hNewDC,aai->nStripTop, 0, s_width-(aai->nStripTop+aai->FrameSize.cx*aai->nFrameCount),newHeight,s_hAniAvaDC,aai->nStripTop+aai->FrameSize.cx*aai->nFrameCount, 0, SRCCOPY);

					_AniAva_RemoveAniAvaDC();
					s_width = newWidth;
					s_height = newHeight;
					s_hAniAvaDC = hNewDC;
					s_hAniAvaBitmap = hNewBmp;
					s_hAniAvaOldBitmap = hNewOldBmp;
				}
				else _AniAva_RemoveAniAvaDC();

				s_AniAvatarList.remove(j);
				mir_free(aai);
				_AniAva_ResumePainting();
				break;
			}
		}
	}
}

static void _AniAva_RemoveAniAvaDC()
{
	if (s_hAniAvaDC) {
		SelectObject(s_hAniAvaDC, s_hAniAvaOldBitmap);
		DeleteObject(s_hAniAvaBitmap);
		DeleteDC(s_hAniAvaDC);
		s_hAniAvaDC = NULL;
		s_height = 0;
		s_width = 0;
		s_hAniAvaBitmap = NULL;
	}
}

static void _AniAva_DestroyAvatarWindow( HWND hwnd)
{
	SendMessage(hwnd,AAM_SELFDESTROY, 0, 0);
}

static int	_AniAva_LoadAvatarFromImage(TCHAR * szFileName, int width, int height, ANIAVATARIMAGEINFO *pRetAII)
{
	ANIAVA_INFO * paai = NULL;
	ANIAVA_INFO aai = {0};
	aai.tcsFilename = szFileName;
	aai.FrameSize.cx = width;
	aai.FrameSize.cy = height;

	int idx = s_AniAvatarList.getIndex(&aai);
	if (idx == -1) { //item not present in list
		paai = (ANIAVA_INFO *)mir_calloc(sizeof(ANIAVA_INFO));
		paai->tcsFilename = mir_tstrdup(szFileName);
		paai->dwAvatarUniqId = rand();

		//get image strip
		HBITMAP hBitmap = NULL;
		GDIPlus_ExtractAnimatedGIF (szFileName, width, height, &hBitmap, &(paai->pFrameDelays), &(paai->nFrameCount), &(paai->FrameSize));

		//copy image to temp DC
		HDC     hTempDC = CreateCompatibleDC(NULL);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hTempDC,hBitmap);

		int     newWidth = s_width+paai->FrameSize.cx*paai->nFrameCount;
		int     newHeight = max(paai->FrameSize.cy,s_height);

		HDC     hNewDC = CreateCompatibleDC(NULL);
		HBITMAP hNewBmp = ske_CreateDIB32(newWidth,newHeight);
		HBITMAP hNewOldBmp = (HBITMAP)SelectObject(hNewDC,hNewBmp);

		_AniAva_PausePainting();
		GdiFlush();
		// copy from old and from new strip
		BitBlt(hNewDC, 0, 0, s_width,s_height,s_hAniAvaDC, 0, 0, SRCCOPY);
		BitBlt(hNewDC,s_width, 0, paai->FrameSize.cx*paai->nFrameCount,paai->FrameSize.cy,hTempDC, 0, 0, SRCCOPY);

		paai->nStripTop = s_width;
		s_AniAvatarList.insert(paai);

		GdiFlush();
		//remove temp DC
		SelectObject(hTempDC,hOldBitmap);
		DeleteObject(hNewBmp);
		DeleteDC(hTempDC);
		DeleteObject(hBitmap);

		//delete old
		_AniAva_RemoveAniAvaDC();

		//setNewDC;
		s_width = newWidth;
		s_height = newHeight;
		s_hAniAvaDC = hNewDC;
		s_hAniAvaBitmap = hNewBmp;
		s_hAniAvaOldBitmap = hNewOldBmp;
		GdiFlush();
		_AniAva_ResumePainting();
	}
	else paai = (ANIAVA_INFO*)s_AniAvatarList[idx];

	if (paai == NULL)
		return 0;

	paai->nRefCount++;
	pRetAII->nFramesCount = paai->nFrameCount;
	pRetAII->pFrameDelays = paai->pFrameDelays;
	pRetAII->ptImagePos.x = paai->nStripTop;
	pRetAII->ptImagePos.y = 0;
	pRetAII->szSize = paai->FrameSize;
	return paai->dwAvatarUniqId;
}

static BOOL _AniAva_GetAvatarImageInfo(DWORD dwAvatarUniqId, ANIAVATARIMAGEINFO * avii)
{
	BOOL res = FALSE;
	for (int j = 0; j < s_AniAvatarList.getCount(); j++) {
		ANIAVA_INFO * aai = (ANIAVA_INFO *) s_AniAvatarList[j];
		if (aai->dwAvatarUniqId == dwAvatarUniqId) {
			avii->nFramesCount = aai->nFrameCount;
			avii->pFrameDelays = aai->pFrameDelays;
			avii->ptImagePos.x = aai->nStripTop;
			avii->ptImagePos.y = 0;
			avii->szSize = aai->FrameSize;
			res = TRUE;
			break;
		}
	}
	return res;
}

static void _AniAva_Clear_ANIAVA_WINDOWINFO(ANIAVA_WINDOWINFO *pavwi)
{
	pavwi->delaysInterval = NULL;
	pavwi->nFramesCount = 0;
	KillTimer(pavwi->hWindow,2);
	pavwi->bPlaying  = FALSE;
	pavwi->TimerId = 0;
}

static void _AniAva_RenderAvatar(ANIAVA_WINDOWINFO * dat, HDC hdcParent /*= NULL*/, RECT *rcInParent /*= NULL */ )
{
	if (dat->bPaused>0)	{	dat->bPended = TRUE;	return; 	}
	else dat->bPended = FALSE;

	if ( IMMEDIATE_DRAW && hdcParent == NULL ) return;
	GdiFlush();

	if (dat->bPlaying && IsWindowVisible(dat->hWindow)) {
		POINT ptWnd = {0};
		SIZE szWnd = {dat->rcPos.right-dat->rcPos.left,dat->rcPos.bottom-dat->rcPos.top};
		BLENDFUNCTION bf = {AC_SRC_OVER, 0, g_CluiData.bCurrentAlpha*dat->bAlpha/256, AC_SRC_ALPHA };
		POINT pt_from = {0, 0};
		HDC hDC_animation = GetDC(NULL);
		HDC copyFromDC;
		RECT clistRect;
		HDC tempDC = NULL;
		HBITMAP hBmp;
		HBITMAP hOldBmp;

		/*
		int x = bf.SourceConstantAlpha;
		x = (49152/(383-x))-129;
		x = min(x,255);	x = max(x,0);
		bf.SourceConstantAlpha = x;
		*/
		if ( s_bFlags == 0 ) { //simple and fastest method - no borders, round corners and etc. just copy
			pt_from.x = dat->ptFromPoint.x+dat->currentFrame*dat->sizeAvatar.cx;
			pt_from.y = dat->ptFromPoint.y;
			copyFromDC = s_hAniAvaDC;
		}
		else {
			// ... need to create additional hDC_animation
			HRGN hRgn = NULL;
			int cornerRadius = s_cornerRadius;
			tempDC = CreateCompatibleDC( NULL );
			hBmp = ske_CreateDIB32( szWnd.cx, szWnd.cy );
			hOldBmp = (HBITMAP)SelectObject(tempDC,hBmp);
			if (s_bFlags & AAO_ROUND_CORNERS)
				if (!cornerRadius)  //auto radius
					cornerRadius = min(szWnd.cx, szWnd.cy )/5;

			if (s_bFlags & AAO_HAS_BORDER) {
				// if has borders - create region (round corners) and fill it, remember internal as clipping
				HBRUSH hBrush = CreateSolidBrush( s_borderColor );
				HBRUSH hOldBrush = (HBRUSH)SelectObject( tempDC, hBrush );
				HRGN rgnOutside = CreateRoundRectRgn( 0, 0, szWnd.cx+1, szWnd.cy+1, cornerRadius << 1, cornerRadius << 1);
				hRgn = CreateRoundRectRgn( 1, 1, szWnd.cx, szWnd.cy, cornerRadius << 1, cornerRadius << 1);
				CombineRgn( rgnOutside,rgnOutside,hRgn,RGN_DIFF);
				FillRgn( tempDC, rgnOutside, hBrush);
				ske_SetRgnOpaque( tempDC, rgnOutside, TRUE);
				SelectObject(tempDC, hOldBrush);
				DeleteObject(hBrush);
				DeleteObject(rgnOutside);
			}
			else if ( cornerRadius > 0 )
				// else create clipping area (round corners)
				hRgn = CreateRoundRectRgn(0, 0, szWnd.cx+1, szWnd.cy+1, cornerRadius << 1, cornerRadius << 1);
			else
				hRgn = CreateRectRgn(0, 0, szWnd.cx+1, szWnd.cy+1);

			// select clip area
			if ( hRgn )
				ExtSelectClipRgn(tempDC, hRgn, RGN_AND);

			if (s_bFlags & AAO_OPAQUE) {
				// if back color - fill clipping area
				HBRUSH hBrush = CreateSolidBrush( s_bkgColor );
				HBRUSH hOldBrush = (HBRUSH)SelectObject( tempDC, hBrush );
				FillRgn( tempDC, hRgn, hBrush );
				ske_SetRgnOpaque( tempDC, hRgn, TRUE );
			}
			// draw avatar
			if (!(s_bFlags & AAO_OPAQUE))
				BitBlt(tempDC, 0, 0, szWnd.cx, szWnd.cy , s_hAniAvaDC , dat->ptFromPoint.x+dat->sizeAvatar.cx*dat->currentFrame, dat->ptFromPoint.y, SRCCOPY);
			else {
				BLENDFUNCTION abf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
				ske_AlphaBlend(tempDC, 0, 0, szWnd.cx, szWnd.cy , s_hAniAvaDC, dat->ptFromPoint.x+dat->sizeAvatar.cx*dat->currentFrame, dat->ptFromPoint.y, szWnd.cx, szWnd.cy, abf);
			}
			// reset clip area
			if (hRgn) {
				DeleteObject(hRgn);
				hRgn = CreateRectRgn(0, 0, szWnd.cx, szWnd.cy);
				SelectClipRgn(tempDC, hRgn);
				DeleteObject(hRgn);
			}

			if ((s_bFlags & AAO_HAS_OVERLAY ) && dat->overlayIconIdx != -1 && s_overlayIconImageList) {
				// if overlay - draw overlay icon
				// position - on avatar
				int x = szWnd.cx - ICON_WIDTH;
				int y = szWnd.cy - ICON_HEIGHT;
				ske_ImageList_DrawEx(s_overlayIconImageList,
					dat->overlayIconIdx & 0xFFFF,
					tempDC, x, y, ICON_WIDTH, ICON_HEIGHT,
					CLR_NONE, CLR_NONE, ILD_NORMAL);
			}
			copyFromDC = tempDC;
		}
		// intersect visible area
		// update layered window
		GetWindowRect(pcli->hwndContactTree, &clistRect);
		if (dat->rcPos.top < 0) {
			pt_from.y += -dat->rcPos.top;
			szWnd.cy += dat->rcPos.top;
		}
		if (dat->rcPos.bottom>clistRect.bottom-clistRect.top)
			szWnd.cy -= (dat->rcPos.bottom-(clistRect.bottom-clistRect.top));

		ptWnd.x = dat->rcPos.left+clistRect.left;
		ptWnd.y = (dat->rcPos.top>0 ? dat->rcPos.top :0)+clistRect.top;
		if (szWnd.cy > 0) {
			if (hdcParent && rcInParent && IMMEDIATE_DRAW) {
				if ( s_bFlags & AAO_OPAQUE )
					BitBlt( hdcParent, rcInParent->left, rcInParent->top, szWnd.cx, szWnd.cy, copyFromDC, pt_from.x, pt_from.y, SRCCOPY);
				else {
					BLENDFUNCTION abf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					ske_AlphaBlend( hdcParent, rcInParent->left, rcInParent->top, szWnd.cx, szWnd.cy, copyFromDC, pt_from.x, pt_from.y, szWnd.cx, szWnd.cy, abf);
				}
			}
			else if (!UpdateLayeredWindow(dat->hWindow, hDC_animation, &ptWnd, &szWnd, copyFromDC, &pt_from, RGB(0, 0, 0), &bf, ULW_ALPHA )) {
				LONG_PTR exStyle;
				exStyle = GetWindowLongPtr(dat->hWindow,GWL_EXSTYLE);
				exStyle |= WS_EX_LAYERED;
				SetWindowLongPtr(dat->hWindow,GWL_EXSTYLE,exStyle);
				if (!IMMEDIATE_DRAW )
					SetWindowPos( pcli->hwndContactTree, dat->hWindow, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING );
				UpdateLayeredWindow(dat->hWindow, hDC_animation, &ptWnd, &szWnd, copyFromDC, &pt_from, RGB(0, 0, 0), &bf, ULW_ALPHA );
			}

			g_CluiData.fAeroGlass = false;
			CLUI_UpdateAeroGlass();
		}
		else dat->bPlaying = FALSE;

		ReleaseDC(NULL,hDC_animation);
		if (tempDC) {
			SelectObject(tempDC, hOldBmp);
			DeleteObject(hBmp);
			DeleteDC(tempDC);
		}
	}
	if (!dat->bPlaying) {
		ShowWindow(dat->hWindow, SW_HIDE);
		KillTimer(dat->hWindow,2);  //stop animation till set pos will be called
	}
	GdiFlush();
}

static void _AniAva_PausePainting()
{
	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = s_Objects[i];
		SendMessage(pai->hWindow, AAM_PAUSE, 0, 0);
	}
}
static void _AniAva_ResumePainting()
{
	for (int i=0; i < s_Objects.getCount(); i++) {
		ANIAVA_OBJECT *pai = s_Objects[i];
		SendNotifyMessage(pai->hWindow, AAM_RESUME, 0, 0);
	}
}

static void _AniAva_ReduceAvatarImages(int startY, int dY, BOOL bDestroyWindow)
{
	for (int i=s_Objects.getCount()-1; i >= 0; i--) {
		ANIAVA_OBJECT *pai = s_Objects[i];
		int res = SendMessage(pai->hWindow,AAM_REMOVEAVATAR,(WPARAM)startY,(LPARAM)dY);
		if (res == 0xDEAD && bDestroyWindow) {
			_AniAva_DestroyAvatarWindow(pai->hWindow);
			mir_free(pai);
			s_Objects.remove(i);
		}
	}
}

static void _AniAva_LoadOptions()
{
	aacheck;
	mir_cslock lck(s_CS);

	s_bFlags = ( db_get_b(NULL,"CList","AvatarsDrawBorders",SETTINGS_AVATARDRAWBORDER_DEFAULT)?	AAO_HAS_BORDER		:0) |
		( db_get_b(NULL,"CList","AvatarsRoundCorners",SETTINGS_AVATARROUNDCORNERS_DEFAULT)?	AAO_ROUND_CORNERS	:0) |
		( db_get_b(NULL,"CList","AvatarsDrawOverlay",SETTINGS_AVATARDRAWOVERLAY_DEFAULT)?	AAO_HAS_OVERLAY		:0) |
		((0) ? AAO_OPAQUE :0);

	if (s_bFlags & AAO_HAS_BORDER)
		s_borderColor = (COLORREF)db_get_dw(NULL,"CList","AvatarsBorderColor",SETTINGS_AVATARBORDERCOLOR_DEFAULT);
	if (s_bFlags & AAO_ROUND_CORNERS)
		s_cornerRadius = db_get_b(NULL,"CList","AvatarsUseCustomCornerSize",SETTINGS_AVATARUSECUTOMCORNERSIZE_DEFAULT)? db_get_w(NULL,"CList","AvatarsCustomCornerSize",SETTINGS_AVATARCORNERSIZE_DEFAULT) : 0;
	if (s_bFlags & AAO_HAS_OVERLAY) {
		//check image list
		BYTE type = db_get_b(NULL,"CList","AvatarsOverlayType",SETTINGS_AVATAROVERLAYTYPE_DEFAULT);
		switch(type) {
		case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
			s_overlayIconImageList = hAvatarOverlays;
			break;
		case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
		case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
			s_overlayIconImageList = g_himlCListClc;
			break;
		default:
			s_overlayIconImageList = NULL;
		}
	}
	if (s_bFlags & AAO_OPAQUE)
		s_bkgColor = 0;
	s_bSeparateWindow = db_get_b(NULL,"CList","AvatarsInSeparateWnd",SETTINGS_AVATARINSEPARATE_DEFAULT);
}

static int	_AniAva_SortAvatarInfo(const ANIAVA_INFO *aai1, const ANIAVA_INFO *aai2)
{
	int res = 0;
	if (aai1 && aai1->tcsFilename && aai2 && aai2->tcsFilename)
		res = _tcsicmp(aai2->tcsFilename, aai1->tcsFilename);
	else {
		int a1 = (aai1 != NULL && aai1->tcsFilename != NULL);
		int a2 = (aai2 != NULL && aai2->tcsFilename != NULL);
		res = a1-a2;
	}
	if (res)
		return res;

	return !(aai1->FrameSize.cx == aai2->FrameSize.cx && aai1->FrameSize.cy == aai2->FrameSize.cy);
}

void _AniAva_InvalidateParent(ANIAVA_WINDOWINFO * dat)
{
	if (!IMMEDIATE_DRAW ) return;
	HWND hwndParent = pcli->hwndContactTree;
	RECT rcPos = dat->rcPos;
	pcli->pfnInvalidateRect( hwndParent, &rcPos, FALSE );
}

static LRESULT CALLBACK _AniAva_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ANIAVA_WINDOWINFO *dat = NULL;
	if (msg == WM_TIMER || msg == WM_DESTROY  || 	(msg>AAM_FIRST && msg < AAM_LAST))
		dat = (ANIAVA_WINDOWINFO *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case AAM_REMOVEAVATAR:
		if (dat->ptFromPoint.x == (int)wParam) return 0xDEAD;	 //need to destroy window
		else if (dat->ptFromPoint.x>(int)wParam) dat->ptFromPoint.x -= (int)lParam;
		return 0;

	case AAM_PAUSE:
		dat->bPaused++;
		return 0;

	case AAM_RESUME:
		dat->bPaused--;
		if (dat->bPaused) return 0;
		if (dat->bPended)
			if (!IMMEDIATE_DRAW )
				_AniAva_RenderAvatar(dat);
		dat->bPended = FALSE;
		return 0;

	case AAM_STOP:
		if (dat->bPlaying) {
			dat->bPlaying = FALSE;
			KillTimer(hwnd,2);
			ShowWindow(hwnd, SW_HIDE);
		}
		return 0;

	case AAM_SETAVATAR:
		{
			ANIAVATARIMAGEINFO *paaii = (ANIAVATARIMAGEINFO*)wParam;
			_AniAva_Clear_ANIAVA_WINDOWINFO(dat);
			dat->nFramesCount = paaii->nFramesCount;
			dat->delaysInterval = paaii->pFrameDelays;
			dat->sizeAvatar = paaii->szSize;
			dat->ptFromPoint = paaii->ptImagePos;
			dat->currentFrame = 0;
			dat->bPlaying = FALSE;
			return MAKELONG(dat->sizeAvatar.cx,dat->sizeAvatar.cy);
		}

	case AAM_SETPOSITION:
		{
			ANIAVA_POSINFO * papi = (ANIAVA_POSINFO *)lParam;
			if (!dat->delaysInterval) return 0;
			if (!papi) return 0;
			dat->rcPos = papi->rcPos;
			dat->overlayIconIdx = papi->idxOverlay;
			dat->bAlpha = papi->bAlpha;
			free(papi);
			if (!dat->bPlaying) {
				dat->bPlaying = TRUE;
				ShowWindow(hwnd,SW_SHOWNA);
				dat->currentFrame = 0;
				KillTimer(hwnd,2);
				SetTimer(hwnd,2,dat->delaysInterval[0],NULL);
			}
			if (!IMMEDIATE_DRAW )
				_AniAva_RenderAvatar(dat);
		}
		return 0;

	case AAM_SETPARENT:
		if ( IMMEDIATE_DRAW ) return 0;
		dat->bOrderTop = ((HWND)wParam != GetDesktopWindow());
		SetParent(hwnd,(HWND)wParam);
		if (dat->bOrderTop)
			SetWindowPos(hwnd,HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_ASYNCWINDOWPOS);
		else {
			LONG exStyle = GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE);
			SetWindowPos(pcli->hwndContactList,hwnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE/*|SWP_ASYNCWINDOWPOS*/);
			if (!(exStyle & WS_EX_TOPMOST))
				SetWindowPos(pcli->hwndContactList,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE/*|SWP_ASYNCWINDOWPOS*/);
		}
		return 0;

	case AAM_REDRAW:
		if ( IMMEDIATE_DRAW )
			return 0;

		if (wParam) {
			if (dat->bOrderTop)
				SetWindowPos(hwnd,HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_ASYNCWINDOWPOS);
			else {
				LONG exStyle = GetWindowLongPtr(pcli->hwndContactList,GWL_EXSTYLE);
				SetWindowPos(pcli->hwndContactList,hwnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE/*|SWP_ASYNCWINDOWPOS*/);
				if (!(exStyle&WS_EX_TOPMOST))
					SetWindowPos(pcli->hwndContactList,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE/*|SWP_ASYNCWINDOWPOS*/);
			}
		}

		_AniAva_RenderAvatar(dat);
		return 0;

	case AAM_RENDER:
		_AniAva_RenderAvatar(dat, (HDC)wParam, (RECT*)lParam);
		return 0;

	case AAM_SELFDESTROY:
		return DestroyWindow(hwnd);

	case WM_CREATE:
		{
			ANIAVA_WINDOWINFO *dat = (ANIAVA_WINDOWINFO *) mir_calloc(sizeof (ANIAVA_WINDOWINFO));
			SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)dat);
			dat->hWindow = hwnd;
			//change layered mode
			LONG_PTR exStyle = GetWindowLongPtr(dat->hWindow, GWL_EXSTYLE);
			exStyle |= WS_EX_LAYERED;
			SetWindowLongPtr(dat->hWindow, GWL_EXSTYLE, exStyle);
			exStyle = GetWindowLongPtr(dat->hWindow, GWL_STYLE);
			exStyle &= ~WS_POPUP;
			exStyle |= WS_CHILD;
			SetWindowLongPtr(dat->hWindow, GWL_STYLE, exStyle);
		}
		break;

	case WM_TIMER:
		if (!IsWindowVisible(hwnd)) {
			DestroyWindow(hwnd);
			return 0;
		}
		dat->currentFrame++;
		if (dat->currentFrame >= dat->nFramesCount)
			dat->currentFrame = 0;

		if (!IMMEDIATE_DRAW)
			_AniAva_RenderAvatar(dat);
		else
			_AniAva_InvalidateParent(dat);

		KillTimer(hwnd, 2);
		SetTimer(hwnd, 2, dat->delaysInterval[dat->currentFrame]+1, NULL);
		return 0;

	case WM_DESTROY:
		_AniAva_Clear_ANIAVA_WINDOWINFO(dat);
		mir_free(dat);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

#undef aacheck
#undef aalock
#undef aaunlock
