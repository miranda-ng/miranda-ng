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

#include "ChatRoom.h"
#include "Utils.h"


ChatRoom::ChatRoom() {
	next = prev = NULL;
	name = NULL;
	id = NULL;
}

ChatRoom::~ChatRoom() {
	if (name!=NULL) delete name;
	if (id!=NULL) delete id;
}

void ChatRoom::setId(const char *id) {
	Utils::copyString(&this->id, id);
}

void ChatRoom::setName(const char *name) {
	Utils::copyString(&this->name, name);
}

const char* ChatRoom::getId() {
	return id;
}

const char* ChatRoom::getName() {
	return name;
}

ChatRoom *ChatRoom::getNext() {
	return next;
}

void ChatRoom::setNext(ChatRoom *next) {
	this->next = next;
}

void ChatRoom::setFlags(int flags) {
	this->flags = flags;
}

int ChatRoom::getFlags() {
	return flags;
}

