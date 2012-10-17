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
#ifndef MANAGERWINDOW_INCLUDED
#define MANAGERWINDOW_INCLUDED
#include "mucc.h"
#include "ChatGroup.h"
#include "ChatRoom.h"
	
class ManagerWindow{

private:
	static CRITICAL_SECTION mutex;
	static bool		released;
	static ManagerWindow *	list;
	HWND			hWnd;
	HWND			hChatRoomsTabWnd;
	HWND			hMyRoomsTabWnd;
	HWND			hMyNicksTabWnd;
	HANDLE			hEvent;
	ManagerWindow *	prev;
	ManagerWindow *	next;
	ChatGroup *		groups;
	ChatRoom *		rooms;
	ChatRoom *		myRooms;
	char *			module;
	char *			moduleName;
	ChatGroup *		currentGroup;
	int				isStarted;
	int				userNickList, userRoomList;
	int				currentTab;
	int				currentRoomsPage;
	int				lastRoomsPage;
public:
	ManagerWindow  (const char *module);
	~ManagerWindow ();
	int				start();
	ChatGroup *		findGroup(const char *id);
	void			queryResultGroups(MUCCQUERYRESULT *queryResult);
	void			queryResultRooms(MUCCQUERYRESULT *queryResult);
	void			queryResultUserRooms(MUCCQUERYRESULT *queryResult);
	void			queryResultUserNick(MUCCQUERYRESULT *queryResult);
	ManagerWindow *	getNext();
	void			setNext(ManagerWindow *);
	ManagerWindow *	getPrev();
	void			setPrev(ManagerWindow *);
	void			setHWND(HWND);
	HWND			getHWND();
	HANDLE			getEvent();
	void			setChatRoomsTabHWND(HWND);
	HWND			getChatRoomsTabHWND();
	void			setMyNicksTabHWND(HWND);
	HWND			getMyNicksTabHWND();
	void			setMyRoomsTabHWND(HWND);
	HWND			getMyRoomsTabHWND();
	void			setModuleName(const char *moduleName);
	void			setCurrentTab(int);
	int				getCurrentTab();
	const char *	getModule();
	const char *	getModuleName();
	void			setCurrentGroup(ChatGroup *);
	ChatGroup *		getCurrentGroup();
	void			setCurrentRoomsPage(int);
	int				getCurrentRoomsPage();
	void			setLastRoomsPage(int);
	int				isLastRoomsPage();
	int				isUserRoomListReceived();
	int				isUserNickListReceived();

	static ManagerWindow* getWindow(const char *module);
	static void		release();
	static void		init();

};
#endif

