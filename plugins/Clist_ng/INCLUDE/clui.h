/*
* astyle --force-indent=tab=4 --brackets=linux --indent-switches
*		  --pad=oper --one-line=keep-blocks  --unpad=paren
*
* Miranda IM: the free IM client for Microsoft* Windows*
*
* Copyright 2000-2010 Miranda ICQ/IM project,
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
* part of clist_nicer plugin for Miranda.
*
* (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
*
* $Id$
*
*/

#ifndef __CLUI_H_
#define __CLUI_H_

#define TM_AUTOALPHA  		1
#define TIMERID_AUTOSIZE 	100
#define MENU_MIRANDAMENU    0xFFFF1234

#define EXICON_COUNT 11

struct TCluiData {
	DWORD 			dwFlags;
	int 			soundsOff;
	BYTE 			bMetaAvail;
	BYTE			bAvatarServiceAvail, tabSRMM_Avail;
	HICON 			hIconVisible, hIconInvisible, hIconChatactive, hIconConnecting;
	int 			notifyActive;
	int 			hIconNotify;
	HMENU 			hMenuNotify;
	int 			iLastEventAdded;
	int 			wNextMenuID;
	MCONTACT		hUpdateContact;
	wchar_t*		szNoEvents;
	BOOL 			forceResize;
	COLORREF 		avatarBorder;
	HBRUSH 			hBrushAvatarBorder;
	HBRUSH 			hBrushCLCBk, hBrushCLCGroupsBk;
	int 			avatarSize;
	BOOL 			bForceRefetchOnPaint;
	BYTE 			dualRowMode;
	BYTE 			avatarPadding;
	BYTE 			isTransparent;
	BYTE 			alpha, autoalpha;
	BYTE 			fadeinout;
	BYTE 			autosize;
	BYTE 			gapBetweenFrames;
	BYTE 			titleBarHeight;
	DWORD 			dwExtraImageMask;
	BYTE 			bRowSpacing;
	HDC 			hdcBg;
	HBITMAP 		hbmBg, hbmBgOld;
	SIZE 			dcSize;
	POINT 			ptW;
	BOOL 			bNoOfflineAvatars;
	BOOL 			bEventAreaEnabled;
	BOOL 			bDblClkAvatars;
	BOOL 			bApplyIndentToBg;
	BOOL 			bEqualSections;
	DWORD 			bFilterEffective;
	BOOL 			bCenterStatusIcons;
	BOOL 			bUsePerProto;
	BOOL 			bOverridePerStatusColors;
	BOOL 			bDontSeparateOffline;
	wchar_t 		groupFilter[2048];
	char 			protoFilter[2048];
	DWORD 			lastMsgFilter;
	char 			current_viewmode[256], old_viewmode[256];
	BYTE 			boldHideOffline;
	DWORD 			statusMaskFilter;
	DWORD 			stickyMaskFilter;
	DWORD 			filterFlags;
	COLORREF 		colorkey;
	char 			szMetaName[256];
	BOOL 			bMetaEnabled;
	BOOL 			bNoTrayTips;
	int 			exIconScale;
	BOOL 			bShowLocalTime;
	BOOL 			bShowLocalTimeSelective;
	BYTE 			bSkinnedButtonMode;
	BYTE 			bFirstRun;
	BYTE 			bUseDCMirroring;
	BYTE 			bUseFloater;
	SIZE 			szOldCTreeSize;
	BYTE 			sortOrder[3];
	BYTE 			bGroupAlign;
    BYTE 			bSkinnedScrollbar;
    DWORD 			langPackCP;
    int  			group_padding;
    DWORD 			t_now;
    BYTE 			exIconOrder[EXICON_COUNT];
    BOOL 			realTimeSaving;
	FILETIME ft;
	SYSTEMTIME st;
};

class CLUI
{
public:
	static void 				Tweak_It					(const COLORREF clr);
	static void 				layoutButtons				(HWND hwnd, RECT *rc);
	static HWND 				preCreateCLC				(HWND parent);
	static int 					createCLC					(HWND parent);
	static void 				loadModule					(void);
	static int 					modulesLoaded				(WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK 	wndProc						(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void 				configureGeometry			(int mode);
	static void 				reloadExtraIcons			();
	static void 				loadExtraIconModule			();
	static void 				removeFromTaskBar			(HWND hWnd);
	static void 				addToTaskBar				(HWND hWnd);
	static DWORD 				getWindowStyle				(BYTE style);
	static void 				applyBorderStyle			();
	static LRESULT CALLBACK 	eventAreaWndProc			(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void 				setFrameButtonStates		(MCONTACT hPassedContact);
	static void 				Show						(HWND hwnd);
	static void 				setButtonStates				(HWND hwnd);
	static void					configureWindowLayout		();
	static void 				hideShowNotifyFrame			();
	static void 				configureEventArea			(HWND hwnd);
	static void					updateLayers				();
	static void 				setLayeredAttributes		(COLORREF clr, BYTE alpha, DWORD flags);
	static inline void			Redraw						();
	static HPEN					hPenFrames;
	static int					fading_active;
	static bool					fInSizing;
	static LRESULT (CALLBACK*	saveContactListWndProc )	(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static WNDPROC 				OldStatusBarProc;
	static RECT					newRect, rcWPC, cluiPos;
	static UPDATELAYEREDWINDOWINFO ulwInfo;
	static BLENDFUNCTION		bf;
	static HIMAGELIST			hExtraImages;
	static TImageItem* 			bgImageItem, *bgImageItem_nonAero, *bgClientItem;
	static TButtonItem*			buttonItems;
#ifdef _USE_D2D
	static ID2D1HwndRenderTarget*	renderTarget;
#endif
};

void CLUI::Redraw()
{
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

int 							MTG_OnmodulesLoad			(WPARAM wParam,LPARAM lParam);
//void 							InitGroupMenus				();

#endif /* __CLUI_H_ */
