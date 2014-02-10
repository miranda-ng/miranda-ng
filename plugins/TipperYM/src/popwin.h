/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#ifndef _POPWIN_INC
#define _POPWIN_INC

#define POP_WIN_CLASS					_T(MODULE) _T("MimTTClass")

#define PUM_GETHEIGHT					(WM_USER + 0x020)
#define PUM_CALCPOS						(WM_USER + 0x021)
#define PUM_SETSTATUSTEXT				(WM_USER + 0x022)
#define PUM_UPDATERGN					(WM_USER + 0x023)
#define PUM_SETAVATAR					(WM_USER + 0x024)
#define PUM_REFRESH_VALUES				(WM_USER + 0x025)
#define PUM_SHOWXSTATUS					(WM_USER + 0x026)
#define PUM_EXPANDTRAYTIP				(WM_USER + 0x027)
#define PUM_REFRESHTRAYTIP				(WM_USER + 0x028)
#define PUM_FADEOUTWINDOW				(WM_USER + 0x029)

// extra icons
#define EXICONS_COUNT		6
#define CTRY_ERROR			42	//Flags - some strange value ???
#define GEN_FEMALE			70	
#define GEN_MALE			77

// copy menu
#define	COPYMENU_ALLITEMS_LABELS	1000	
#define	COPYMENU_ALLITEMS			1001
#define	COPYMENU_AVATAR				1002

// tray tooltip items
#define TRAYTIP_ITEMS_COUNT			11

#define TRAYTIP_NUMCONTACTS			1
#define TRAYTIP_LOCKSTATUS			2
#define TRAYTIP_LOGON				4
#define TRAYTIP_UNREAD_EMAILS		8
#define TRAYTIP_STATUS				16
#define TRAYTIP_STATUS_MSG			32
#define TRAYTIP_EXTRA_STATUS		64
#define TRAYTIP_LISTENINGTO			128
#define TRAYTIP_FAVCONTACTS			256
#define TRAYTIP_MIRANDA_UPTIME		512
#define TRAYTIP_CLIST_EVENT			1024

// favorite contacts options
#define FAVCONT_HIDE_OFFLINE		1
#define FAVCONT_APPEND_PROTO		2

// other
#define TITLE_TEXT_LEN		512
#define MAX_VALUE_LEN		64

#define ANIM_ELAPSE			10
#define ANIM_STEPS			255
#define CHECKMOUSE_ELAPSE	250

#define ID_TIMER_ANIMATE	0x0100
#define ID_TIMER_CHECKMOUSE	0x0101
#define ID_TIMER_TRAYTIP	0x0102


typedef struct {
	HICON hIcon;
	bool bDestroy;
} ExtraIcons;

typedef struct {
	int cbSize;
	int isTreeFocused;   //so the plugin can provide an option
	int isGroup;		//0 if it's a contact, 1 if it's a group
	HANDLE hItem;		//handle to group or contact
	POINT ptCursor;
	RECT rcItem;
	TCHAR *swzText;		// for tips with specific text
	char *szProto;		// for proto tips
} CLCINFOTIPEX;

typedef struct {
	TCHAR *swzLabel, *swzValue;
	HICON hIcon;
	bool bValueNewline;
	bool bLineAbove;
	bool bIsTitle;
	int iLabelHeight, iValueHeight, iTotalHeight;
	SMILEYPARSEINFO spi;
} RowData;

typedef struct {
	HPEN hpenBorder, hpenDivider;
	int iTitleHeight, iAvatarHeight, iIconsHeight, iTextHeight, iLabelWidth;
	int iRealAvatarWidth, iRealAvatarHeight;
	MCONTACT hContact;
	int iIconIndex;
	CLCINFOTIPEX clcit;
	TCHAR swzTitle[TITLE_TEXT_LEN];
	SMILEYPARSEINFO spiTitle;
	RowData *rows;
	int iRowCount;
	int iAnimStep;
	int iCurrentTrans;
	bool bIsTextTip, bIsTrayTip;
	int iIndent, iSidebarWidth;
	POINT ptCursorStartPos; // work around bugs with hiding tips (timer check mouse position)
	ExtraIcons extraIcons[EXICONS_COUNT]; 
	bool bIsIconVisible[EXICONS_COUNT];
	int iTrans;
	int iHotkeyId;
	bool bIsPainted;
	bool bNeedRefresh;
	bool bAllowReposition;
	RECT rcWindow;   
	HRGN hrgnAeroGlass; 
} PopupWindowData;


LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern int IsTrayProto(const TCHAR *swzProto, BOOL bExtendedTip);


#endif
