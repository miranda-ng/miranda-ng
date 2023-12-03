/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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
#ifndef MSGWINDOW_H
#define MSGWINDOW_H

/* child window services */
#define DM_SETPARENT        (WM_USER+0x1B3)
#define DM_ACTIVATE         (WM_USER+0x1B6)
#define DM_GETCONTEXTMENU   (WM_USER+0x1B7)
#define DM_SETFOCUS         (WM_USER+0x1BA)
#define DM_CLISTSETTINGSCHANGED   (WM_USER+0x1BB)

#define TBDF_TEXT 1
#define TBDF_ICON 2

struct TitleBarData
{
	int iFlags;
	wchar_t *pszText;
	HICON hIcon;
	HICON hIconBig;
	HICON hIconNot;
};

#define TCDF_TEXT 1
#define TCDF_ICON 2

struct TabControlData
{
	int iFlags;
	wchar_t *pszText;
	HICON hIcon;
};

#define SBDF_TEXT  1
#define SBDF_ICON  2

struct StatusBarData
{
	int iItem;
	int iFlags;
	wchar_t *pszText;
	HICON hIcon;
};

struct TabCtrlData
{
	int lastClickTime;
	WPARAM clickWParam;
	LPARAM clickLParam;
	POINT mouseLBDownPos;
	HIMAGELIST hDragImageList;
	int bDragging;
	int bDragged;
	int destTab;
	int srcTab;
};

struct ParentWindowData
{
	HWND m_hwnd;
	MCONTACT m_hContact;
	int m_iChildrenCount;
	HWND m_hwndActive;
	HWND m_hwndStatus;
	HWND m_hwndTabs;
	TabFlags flags2;
	RECT childRect;
	POINT mouseLBDownPos;
	int mouseLBDown;
	int nFlash;
	int nFlashMax;
	int bMinimized;
	int bVMaximized;
	int iSplitterX, iSplitterY;

	bool bTopmost;

	void ActivateChild(CMsgDialog *pDlg);
	void ActivateChildByIndex(int index);
	void ActivateNextChild(HWND child);
	void ActivatePrevChild(HWND child);
	void AddChild(CMsgDialog *pDlg);
	void CloseOtherChilden(CMsgDialog *pDlg);
	int  GetChildCount();
	void GetChildWindowRect(RECT *rcChild);
	int  GetTabFromHWND(HWND child);
	CMsgDialog *GetChildFromHWND(HWND hwnd);
	void PopupWindow(CMsgDialog *pDlg, bool bIncoming);
	void RemoveChild(HWND child);
	void MessageSend(const SendQueue::Item &msg);
	void SetContainerWindowStyle();
	void StartFlashing();
	void ToggleInfoBar();
	void ToggleStatusBar();
	void ToggleTitleBar();
	void ToggleToolBar();
	void UpdateStatusBar(const StatusBarData &sbd, HWND);
	void UpdateTabControl(const TabControlData &tbd, HWND);
	void UpdateTitleBar(const TitleBarData &tbd, HWND);

	int windowWasCascaded;
	TabCtrlData *tabCtrlDat;
	BOOL isChat;
	ParentWindowData *prev, *next;
};

HWND GetParentWindow(MCONTACT hContact, bool bChat);

#endif
