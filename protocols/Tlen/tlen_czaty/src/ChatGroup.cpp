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

#include "ChatGroup.h"
#include "Utils.h"

ChatGroup::ChatGroup() {
	prev = next = NULL;
	parent = child =NULL;
	name = NULL;
	id = 0;
}

ChatGroup::~ChatGroup() {
	if (name != NULL) delete name;
	if (id != NULL) delete id;
}
ChatGroup * ChatGroup::getNext() {
	return next;
}

void ChatGroup::setNext(ChatGroup *next) {
	this->next = next;
}

ChatGroup * ChatGroup::getPrev() {
	return prev;
}

void ChatGroup::setPrev(ChatGroup *prev) {
	this->prev = prev;
}

ChatGroup * ChatGroup::getChild() {
	return child;
}

void ChatGroup::addChild(ChatGroup *child) {	
	ChatGroup *group;
	for (group=getChild();group != NULL && group->getNext() != NULL;group=group->getNext()) {
	}
	if (group == NULL) this->child = child;
	else {
		group->setNext(child);
	}
	child->setPrev(group);
	child->setNext(NULL);
}

ChatGroup * ChatGroup::getListNext() {
	return listNext;
}

void ChatGroup::setListNext(ChatGroup *next) {
	this->listNext = next;
}

const char *ChatGroup::getId() {
	return id;
}

void ChatGroup::setId(const char *id) {
	Utils::copyString(&this->id, id);
}

HTREEITEM ChatGroup::getTreeItem() {
	return hItem;
}

void ChatGroup::setTreeItem(HTREEITEM hItem) {
	this->hItem=hItem;
}

const char *ChatGroup::getName() {
	return name;
}

void ChatGroup::setName(const char *name) {
	Utils::copyString(&this->name, name);
}

