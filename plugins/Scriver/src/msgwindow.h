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

/* container services */
#define CM_ADDCHILD          (WM_USER+0x180)
#define CM_REMOVECHILD		 (WM_USER+0x181)
#define CM_ACTIVATECHILD	 (WM_USER+0x182)
#define CM_ACTIVATEPREV		 (WM_USER+0x183)
#define CM_ACTIVATENEXT		 (WM_USER+0x184)
#define CM_ACTIVATEBYINDEX	 (WM_USER+0x185)

#define CM_GETCHILDCOUNT	 (WM_USER+0x188)
#define CM_GETACTIVECHILD	 (WM_USER+0x189)
#define CM_GETFLAGS			 (WM_USER+0x18A)

#define CM_UPDATETITLEBAR    (WM_USER+0x190)
#define CM_UPDATESTATUSBAR   (WM_USER+0x191)
#define CM_UPDATETABCONTROL  (WM_USER+0x192)
#define CM_STARTFLASHING	 (WM_USER+0x1A0)
#define CM_POPUPWINDOW		 (WM_USER+0x1A1)

#define CM_GETTOOLBARSTATUS  (WM_USER+0x1A2)

/* child window services */
#define DM_UPDATETITLEBAR    (WM_USER+0x200)
#define DM_UPDATESTATUSBAR   (WM_USER+0x201)
#define DM_UPDATETABCONTROL  (WM_USER+0x202)
#define DM_SETPARENT	 	 (WM_USER+0x203)
#define DM_ACTIVATE			 (WM_USER+0x206)
#define DM_GETCONTEXTMENU    (WM_USER+0x207)
#define DM_SETFOCUS			 (WM_USER+0x20A)
#define DM_CLISTSETTINGSCHANGED   (WM_USER+0x20B)

#define SBDF_TEXT  1
#define SBDF_ICON  2

struct StatusBarData
{
	int iItem;
	int iFlags;
	TCHAR *pszText;
	HICON hIcon;
};

#define TBDF_TEXT 1
#define TBDF_ICON 2

struct TitleBarData
{
	int iFlags;
	TCHAR *pszText;
	HICON hIcon;
	HICON hIconBig;
	HICON hIconNot;
};

#define TCDF_TEXT 1
#define TCDF_ICON 2

struct TabControlData
{
	int iFlags;
	TCHAR *pszText;
	HICON hIcon;
};

TCHAR* GetWindowTitle(MCONTACT hContact, const char *szProto);
TCHAR* GetTabName(MCONTACT hContact);
HWND   GetParentWindow(MCONTACT hContact, BOOL bChat);
void   NotifyLocalWinEvent(MCONTACT hContact, HWND hwnd, unsigned int type);

#endif
