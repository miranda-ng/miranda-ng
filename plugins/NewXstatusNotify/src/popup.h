/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef POPUP_H
#define POPUP_H

#define POPUP_COLOR_OWN        1
#define POPUP_COLOR_WINDOWS    2
#define POPUP_COLOR_POPUP      3
#define DEFAULT_COLORS         POPUP_COLOR_POPUP

// Actions on popup click
#define PCA_OPENMESSAGEWND     0   // open message window
#define PCA_CLOSEPOPUP         1   // close popup
#define PCA_OPENDETAILS        2   // open contact details window
#define PCA_OPENMENU           3   // open contact menu
#define PCA_OPENHISTORY        4   // open contact history
#define PCA_DONOTHING          5   // do nothing

#define STRING_SHOWPREVIOUSSTATUS LPGENT("(was %s)")

typedef struct tagPLUGINDATA
{
	WORD newStatus;
	WORD oldStatus;
	HWND hWnd;
	HANDLE hAwayMsgProcess;
	HANDLE hAwayMsgHook;
} PLUGINDATA;

static struct {
	TCHAR *Text;
	int Action;
} PopupActions[] = {
	LPGENT("Open message window"), PCA_OPENMESSAGEWND,
	LPGENT("Close popup"), PCA_CLOSEPOPUP,
	LPGENT("Open contact details window"), PCA_OPENDETAILS,
	LPGENT("Open contact menu"), PCA_OPENMENU,
	LPGENT("Open contact history"), PCA_OPENHISTORY,
	LPGENT("Do nothing"), PCA_DONOTHING
};

void ShowChangePopup(MCONTACT hContact, char *szProto, HICON hIcon, WORD newStatus, TCHAR *stzText, PLUGINDATA *pdp = NULL);
LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif