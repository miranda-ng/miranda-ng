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
#ifndef CHATROOM_INCLUDED
#define CHATROOM_INCLUDED
#include "mucc.h"


class ChatRoom {
private:
	HTREEITEM		hItem;
	ChatRoom *		prev;
	ChatRoom *		next;
	char *			name;
	char *			id;
	int				flags;
public:
	ChatRoom();
	~ChatRoom();
	ChatRoom *		getPrev();
	ChatRoom *		getNext();
	HTREEITEM		getTreeItem();
	const char *	getName();
	const char *	getId();
	int				getFlags();
	void			setPrev(ChatRoom *);
	void			setNext(ChatRoom *);
	void			setName(const char *);
	void			setTreeItem(HTREEITEM );
	void			setId(const char *);
	void			setFlags(int);
};

#endif
