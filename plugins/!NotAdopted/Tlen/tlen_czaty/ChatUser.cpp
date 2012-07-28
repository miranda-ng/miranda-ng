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

#include "ChatUser.h"
#include "Utils.h"

ChatUser::ChatUser() {
	id = NULL;
	nick = NULL;
	hTreeItem = NULL;
	me = 0;
}

ChatUser::~ChatUser() {
	if (id!=NULL) delete id;
	if (nick!=NULL) delete nick;
}

void ChatUser::setNick(const char *nick) {
	Utils::copyString(&this->nick, nick);
}

void ChatUser::setId(const char *id) {
	Utils::copyString(&this->id, id);
}

const char *ChatUser::getNick() {
	return nick;
}

const char *ChatUser::getId() {
	return id;
}

void ChatUser::setNext(ChatUser *next) {
	this->next = next;
}

ChatUser * ChatUser::getNext() {
	return next;
}

void ChatUser::setFlags(int flags) {
	this->flags = flags;
}

int ChatUser::getFlags() {
	return flags;
}

void ChatUser::setMe(int me) {
	this->me = me;
}

int ChatUser::isMe() {
	return me;
}

void ChatUser::setHTreeItem(HTREEITEM hti) {
	hTreeItem = hti;
}

HTREEITEM ChatUser::getHTreeItem() {
	return hTreeItem;
}
