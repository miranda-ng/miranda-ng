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
#ifndef CHATEVENT_INCLUDED
#define CHATEVENT_INCLUDED
#include "mucc.h"

class ChatEvent {
private:
	ChatEvent *	prev;
	ChatEvent *	next;
	MUCCEVENT 	event;
public:
	ChatEvent();
	ChatEvent(const MUCCEVENT *);
	~ChatEvent();
	ChatEvent *		getPrev();
	ChatEvent *		getNext();
	void			setPrev(ChatEvent *);
	void			setNext(ChatEvent *);
	const MUCCEVENT *getEvent();
};

class ChatEventList {
private:
	ChatEvent 	eventListRoot;
	ChatEvent *	eventListEnd;
	int			loMaxSize;
	int			hiMaxSize;
	int			currentSize;
public:
	enum {
		DEFAULT_MAX_SIZE = 100
	};
	ChatEventList();
	~ChatEventList();
	ChatEvent * getEvents();
	int			addEvent(const MUCCEVENT *);
	void		clear();
	void		setMaxSize(int s);
};

#endif
