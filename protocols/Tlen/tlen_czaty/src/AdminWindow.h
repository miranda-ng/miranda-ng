/*

MUCC Group Chat GUI Plugin for Miranda NG
Copyright (C) 2004  Piotr Piastucki

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

class AdminWindow;
#ifndef ADMINWINDOW_INCLUDED
#define ADMINWINDOW_INCLUDED

#include "mucc.h"
#include "ChatWindow.h"

class AdminWindow {
private:
	HWND		hWnd;
	HWND		hKickTabWnd, hBrowserTabWnd;
	int			currentTab;
	int			browserMode;
	ChatWindow *parent;
	char	   *nick;
public:
	enum BROWSER_MODE {
		BROWSE_BANNED = 1,
		BROWSE_OWNERS = 2,
		BROWSE_ADMINS = 3,
		BROWSE_MEMBERS = 4,
		BROWSE_MODERATORS = 5
	};
	AdminWindow (ChatWindow *parent, const char *nick, int mode);
	~AdminWindow();
	void		start();
	void		setHWND(HWND);
	void		setKickTabHWND(HWND);
	void		setBrowserTabHWND(HWND);
	HWND		getKickTabHWND();
	HWND		getBrowserTabHWND();
	int			getCurrentTab();
	int			getBrowserMode();
	void		setCurrentTab(int);
	void		queryUsers(int);
	ChatWindow*	getParent();
	const char *getNick();
	void		queryResultUsers(MUCCQUERYRESULT *queryResult);
};

#endif

