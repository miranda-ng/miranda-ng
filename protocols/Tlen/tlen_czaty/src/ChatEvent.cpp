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

#include "ChatEvent.h"
#include "Options.h"
#include "Utils.h"
#include <string.h>

ChatEvent::ChatEvent() {

	next = prev = NULL;
	event.cbSize = sizeof(MUCCEVENT);
	event.iType = 0;
	event.pszID = NULL;
	event.pszModule = NULL;
	event.pszName = NULL;
	event.pszNick = NULL;
	event.pszText = NULL;
	event.pszUID = NULL;
}

ChatEvent::ChatEvent(const MUCCEVENT *e) {

	event.cbSize = sizeof(MUCCEVENT);
	event.iType = e->iType;
	event.bIsMe = e->bIsMe;
	event.color = e->color;
	event.dwData = e->dwData;
	event.dwFlags = e->dwFlags;
	event.iFont = e->iFont;
	event.iFontSize = e->iFontSize;
	event.time = e->time;
	event.pszID = NULL;
	event.pszModule = NULL;
	event.pszName = NULL;
	event.pszNick = NULL;
	event.pszText = NULL;
	event.pszUID = NULL;
//	Utils::copyString((char **)&(event.pszID), e->pszID);
//	Utils::copyString((char **)&(event.pszModule), e->pszModule);
//	Utils::copyString((char **)&(event.pszName), e->pszName);
	if (e->iType == MUCC_EVENT_STATUS || e->iType == MUCC_EVENT_MESSAGE) {
		Utils::copyString((char **)&(event.pszNick), e->pszNick);
	}
	if (e->iType == MUCC_EVENT_ERROR || e->iType == MUCC_EVENT_MESSAGE || e->iType == MUCC_EVENT_TOPIC) {
		Utils::copyString((char **)&(event.pszText), e->pszText);
	}
//	Utils::copyString((char **)&(event.pszUID), e->pszUID);
	next = prev = NULL;
}

ChatEvent::~ChatEvent() {

	if (event.pszID != NULL) {
		delete (char *)event.pszID;
	}
	if (event.pszModule != NULL) {
		delete (char *)event.pszModule;
	}
	if (event.pszName != NULL) {
		delete (char *)event.pszName;
	}
	if (event.pszNick != NULL) {
		delete (char *)event.pszNick;
	}
	if (event.pszText != NULL) {
		delete (char *)event.pszText;
	}
	if (event.pszUID != NULL) {
		delete (char *)event.pszUID;
	}
	if (next != NULL) {
		next->setPrev(prev);
	}
	if (prev != NULL) {
		prev->setNext(next);
	}
}

ChatEvent * ChatEvent::getNext() {
	return next;
}

ChatEvent * ChatEvent::getPrev() {
	return prev;
}

void ChatEvent::setNext(ChatEvent *next) {
	this->next = next;
}

void ChatEvent::setPrev(ChatEvent * prev) {
	this->prev = prev;
}

const MUCCEVENT * ChatEvent::getEvent() {
	return &event;
}

ChatEventList::ChatEventList() {
	eventListEnd = &eventListRoot;
	setMaxSize(DEFAULT_MAX_SIZE);
	currentSize = 0;
}

ChatEventList::~ChatEventList() {
	while (eventListRoot.getNext() != NULL) {
		delete eventListRoot.getNext();
	}
}

int ChatEventList::addEvent(const MUCCEVENT *muccevent) {
	int trimmed = 0;
	ChatEvent *event = new ChatEvent(muccevent);
	event->setPrev(eventListEnd);
	eventListEnd->setNext(event);
	eventListEnd=event;
	currentSize++;	
	if (currentSize>hiMaxSize) {
		while (currentSize>loMaxSize && eventListRoot.getNext() != NULL) {
			delete eventListRoot.getNext();
			currentSize--;
			trimmed = 1;
		}
	}
	return trimmed;
}

ChatEvent * ChatEventList::getEvents() {
	return eventListRoot.getNext();
}

void ChatEventList::setMaxSize(int s) {
	loMaxSize = s;
	if (s>200) {
		hiMaxSize = s + s/10;
	} else {
		hiMaxSize = s + 20;
	}
}

void ChatEventList::clear() {
	ChatEvent *event = eventListRoot.getNext();
	eventListRoot.setNext(NULL);
	eventListEnd = &eventListRoot;
	currentSize = 0;
	if (event != NULL) {
		event->setPrev(NULL);
		while (event->getNext() != NULL) {
			delete event->getNext();
		}
		delete event;
	}
}


