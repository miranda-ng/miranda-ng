/*

MUCC Group Chat GUI Plugin for Miranda IM
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

class ChatWindow;

#ifndef CHATWINDOW_INCLUDED
#define CHATWINDOW_INCLUDED
#include "mucc.h"
#include "ChatEvent.h"
#include "ChatUser.h"
#include "AdminWindow.h"
#include "ChatContainer.h"

#define DM_CHAT_EVENT   (WM_USER+1)
#define DM_CHAT_QUERY   (WM_USER+2)
#define WM_TLEN_SMILEY  (WM_USER+200)


class StreamData {
public:
	ChatWindow *chat;
	MUCCEVENT  *event;
};

class ChatWindow{
private:
	static ChatWindow *	list;
	static CRITICAL_SECTION mutex;
	static bool		released;
	static HFONT	hListGroupFont, hListFont;
	static COLORREF colorListBg, colorListText, colorListGroupText;
	static COLORREF colorInputBg, colorLogBg;
	ChatContainer *	container;
	HANDLE			hEvent;
	HWND			hWnd;
	HWND			hWndLog;
	HFONT			hEditFont;
	HTREEITEM		hUserGroups[5];
	char *			module;
	char *			roomId;
	char *			roomName;
	char *			topic;
	int				options;
	int				roomFlags;
	int				font, fontSize;
	int				bBold, bItalic, bUnderline;
	int				wasFirstMessage;
	bool			isEmpty;
	COLORREF		fontColor;
	int				isStarted;
	ChatWindow *	prev;
	ChatWindow *	next;
	int				adminDialogMode;
	AdminWindow *	adminWindow;
	ChatUser *		users;
	ChatUser *		userMe;
	void			addUser(ChatUser *);
	void			removeUser(ChatUser *);
	ChatUser *		findUser(const char *);
	int				getUserGroup(ChatUser *);
	ChatUser *		getSelectedUser();
	ChatEventList 	eventList;
	void			createRTFHeader();
	int				appendMessage(const MUCCEVENT *event);
	int				logMessage(const MUCCEVENT *event);
public:
	enum LOGFLAGS {
		FLAG_SHOW_NICKNAMES	= 0x00000001,
		FLAG_MSGINNEWLINE	= 0x00000002,
		FLAG_OPT_SENDONENTER= 0x00000004,
		FLAG_OPT_ENABLEIEVIEW=0x00000008,

		FLAG_SHOW_DATE		= 0x00000010,
		FLAG_SHOW_TIMESTAMP	= 0x00000020,
		FLAG_SHOW_SECONDS	= 0x00000040,
		FLAG_LONG_DATE		= 0x00000080,

		FLAG_FORMAT_FONT	= 0x00000100,
		FLAG_FORMAT_SIZE	= 0x00000200,
		FLAG_FORMAT_COLOR	= 0x00000400,
		FLAG_FORMAT_STYLE	= 0x00000800,
		FLAG_FORMAT_ALL		= 0x00000F00,

		FLAG_LOG_MESSAGES	= 0x00001000,
		FLAG_LOG_JOINED		= 0x00002000,
		FLAG_LOG_LEFT		= 0x00004000,
		FLAG_LOG_TOPIC		= 0x00008000,
		FLAG_LOG_ALL		= 0x0000F000,

		FLAG_FLASH_MESSAGES	= 0x00010000,
		FLAG_FLASH_JOINED	= 0x00020000,
		FLAG_FLASH_LEFT		= 0x00040000,
		FLAG_FLASH_TOPIC	= 0x00080000,
		FLAG_FLASH_ALL		= 0x000F0000,


	};

	enum ADMINMODES {
		ADMIN_MODE_KICK		= 1,
		ADMIN_MODE_ROLE		= 2,
		ADMIN_MODE_BROWSER	= 3
	};
	int				hSplitterPos;
	int				vSplitterPos;
	int				hSplitterMinBottom;
	int				hSplitterMinTop;
	int				vSplitterMinLeft;
	int				vSplitterMinRight;
	ChatWindow (MUCCWINDOW *);
	~ChatWindow ();
	ChatWindow *	getNext();
	void			setNext(ChatWindow *);
	ChatWindow *	getPrev();
	ChatContainer*	getContainer();
	void			setPrev(ChatWindow *);
	void			setHWND(HWND);
	HWND			getHWND();
	void			setHWNDLog(HWND);
	HWND			getHWNDLog();
	HANDLE			getEvent();
	const char *	getModule();
	void			setModule(const char *);
	const char *	getRoomId();
	void			setRoomId(const char *);
	const char *	getRoomName();
	void			setRoomName(const char *);
	void			setOptions(int);
	int				getOptions();
	void			setRoomFlags(int);
	int				getRoomFlags();
	int				getFontSizeNum();
	int				getFontSize(int);
	int				getFontNameNum();
	const char *	getFontName(int);
	void			setFont(int font, int size, int bBold, int bItalic, int bUnderline, COLORREF color);
	int				getFont();
	int				getFontSize();
	int				getFontStyle();
	COLORREF		getFontColor();
	int				start();
	int				startPriv();
	void			startAdminDialog(int mode);
	int				kickAndBan(int time);
	int				kickAndBan(const char *id, int time, const char *);
	int				unban(const char *id);
	int				setRights(int flags);
	int				setRights(const char *id, int flags);
	void			queryResultContacts(MUCCQUERYRESULT *queryResult);
	void			queryResultUsers(MUCCQUERYRESULT *queryResult);
	void			setTreeItem(int, HTREEITEM);
	HTREEITEM		getTreeItem(int);
	ChatUser *		findUserByNick(const char *);
	ChatUser *		getMe();
	void			refreshSettings();
	void			setAdminWindow(AdminWindow *);
	AdminWindow *	getAdminWindow();
	void			rebuildLog();
	void			clearLog();
	int				logEvent(const MUCCEVENT *event);
	int				changePresence(const MUCCEVENT *event);
	int				changeTopic(const MUCCEVENT *event);
	int				changeRoomInfo(const MUCCEVENT *event);
	int				leave();

	static void		refreshSettings(int force);
	static HFONT	getListGroupFont();
	static HFONT	getListFont();
	static COLORREF getListTextColor();
	static COLORREF getListGroupTextColor();
	static int		getDefaultOptions();

	static void		init();
	static void		release();
	static ChatWindow * getWindow(const char *module, const char *roomId);

};
#endif
