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
#ifndef DIALOGS_INCLUDED
#define DIALOGS_INCLUDED

#include "mucc.h"
#include "m_mucc.h"
#include "ChatWindow.h"


class HelperDialog {
public:
	class HelperContact {
	private:
		char *id;
		char *name;
		HelperContact *next;
	public:
		HelperContact(const char *id, const char *name);
		~HelperContact();
		void			setNext(HelperContact *);
		HelperContact*	getNext();
		const char *	getId();
		const char *	getName();
	};

private:
	static HelperDialog *	list;
	static CRITICAL_SECTION mutex;
	char *			module;
	char *			roomId;
	char *			roomName;
	DWORD			roomFlags;
	HelperContact *	contactList;
	HelperDialog *	prev;
	HelperDialog *	next;
	char *			nick;
	char *			reason;
	HANDLE			hEvent;
	HWND			hWnd;
public:
	HelperDialog();
	HelperDialog(ChatWindow *);
	~HelperDialog();
	void			setModule(const char *);
	void			setRoomId(const char *);
	void			setRoomName(const char *);
	void			setRoomFlags(DWORD);
	void			setNick(const char *);
	void			setReason(const char *);
	void			setPrev(HelperDialog *);
	HelperDialog*	getPrev();
	void			setNext(HelperDialog *);
	HelperDialog*	getNext();
	HWND			getHWND();
	void			setHWND(HWND hWnd);
	void			setContactList(MUCCQUERYRESULT *queryResult);
	HelperContact*	getContactList();
	const char *	getModule();
	const char *	getRoomId();
	const char *	getRoomName();
	DWORD			getRoomFlags();
	const char *	getNick();
	const char *	getReason();
	HANDLE			getEvent();
	static void		init();
	static void		release();
	static void		inviteDlg(ChatWindow *chat, MUCCQUERYRESULT *queryResult);
	static void		acceptDlg(MUCCEVENT *event);
	static void		joinDlg(MUCCEVENT *event);
	static void		errorDlg(MUCCEVENT *event);
	static void		topicDlg(ChatWindow *chat);
};



#endif
