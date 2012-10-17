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

class ChatContainer;

#ifndef CHATCONTAINER_INCLUDED
#define CHATCONTAINER_INCLUDED
#include "mucc.h"
#include "ChatEvent.h"
#include "ChatUser.h"
#include "AdminWindow.h"

class ChatContainerChild {
public:
	ChatWindow *window;
	ChatContainerChild *next;
	ChatContainerChild *prev;
};

class ChatContainer{

private:
	static ChatContainer *	list;
	static bool		released;
	static CRITICAL_SECTION mutex;
	HWND			hWnd;
	HANDLE			hEvent;
	int				childCount;
	int				width, height;
	int				nFlash, nFlashMax;
	ChatContainer  *next, *prev;
	ChatWindow * active;
	ChatContainer();
protected:
	friend INT_PTR CALLBACK ContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	void			addChild(ChatWindow *);
	void			activateChild(ChatWindow *);
	void			changeChildData(ChatWindow *);
	void			removeChild(ChatWindow *);
	void			setUnread(ChatWindow *child, int unread);
	void			getChildWindowRect(RECT *rcChild);
	ChatWindow *	getChildFromTab(int tabId);
	int				getChildTab(ChatWindow *);
	int				getChildCount();
	void			setChildCount(int);
	ChatWindow *	getActive();
	ChatContainerChild *getChildren();
	ChatContainer *	getNext();
	void			setNext(ChatContainer *);
	ChatContainer * getPrev();
	void			setPrev(ChatContainer *);
	int				lastClickTime;
	int				lastClickTab;
	HANDLE			getEvent();
	void			setHWND(HWND);
	int				getFlash();
	int				getFlashMax();
	int				getFlashTimeout();
	void			setFlash(int);
public:
	enum FLAGS {
		FLAG_USE_TABS	= 0x00000001,
	};

	~ChatContainer();
	static void		init();
	static void		release();
	static ChatContainer * getWindow();
	static int		getDefaultOptions();
	void			show(bool bShow);
	HWND			getHWND();

	HWND			remoteCreateChild(DLGPROC proc, ChatWindow *);
	void			remoteAddChild(ChatWindow *);
	void			remoteChangeChildData(ChatWindow *);
	void			remoteRemoveChild(ChatWindow *);
	void			remoteSetUnread(ChatWindow *ptr, int unread);
	void			remoteFlashWindow();
};


#endif
