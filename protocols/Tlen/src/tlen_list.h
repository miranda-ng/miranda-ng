/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

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

#include "tlen.h"

#ifndef _TLEN_LIST_H_
#define _TLEN_LIST_H_

typedef struct TLEN_LIST_ITEM_STRUCT {
	TLEN_LIST list;
	char *jid;
	char *id2;

	// LIST_ROSTER
	// jid = jid of the contact
	char *nick;
	int status;	// Main status, currently useful for transport where no resource information is kept.
				// On normal contact, this is the same status as shown on contact list.
	TLEN_SUBSCRIPTION subscription;
	char *statusMessage;	// Status message when the update is to JID with no resource specified (e.g. transport user)
	char *software;
	char *version;
	char *system;
	char *group;
	char *protocolVersion;
	int	 avatarFormat;
	char *avatarHash;
	BOOL newAvatarDownloading;
	BOOL versionRequested;
	BOOL infoRequested;
	int idMsgAckPending;
	char *messageEventIdStr;
	BOOL wantComposingEvent;
	BOOL isTyping;

	// LIST_ROOM
	// jid = room JID
	// char *name; // room name
	//char *type;	// room type

	// LIST_CHATROOM
	// jid = room JID
	// char *nick;	// my nick in this chat room (SPECIAL: in UTF8)
	// TLEN_RESOURCE_STATUS *resource;	// participant nicks in this room
	char *roomName;

	// LIST_FILE
	struct TLEN_FILE_TRANSFER_STRUCT *ft;
} TLEN_LIST_ITEM;


void TlenListInit(TlenProtocol *proto);
void TlenListUninit(TlenProtocol *proto);
void TlenListWipe(TlenProtocol *proto);
void TlenListWipeSpecial(TlenProtocol *proto);
int TlenListExist(TlenProtocol *proto, TLEN_LIST list, const char *jid);
TLEN_LIST_ITEM *TlenListAdd(TlenProtocol *proto, TLEN_LIST list, const char *jid);
void TlenListRemove(TlenProtocol *proto, TLEN_LIST list, const char *jid);
void TlenListRemoveList(TlenProtocol *proto, TLEN_LIST list);
void TlenListRemoveByIndex(TlenProtocol *proto, int index);
int TlenListFindNext(TlenProtocol *proto, TLEN_LIST list, int fromOffset);
TLEN_LIST_ITEM *TlenListGetItemPtr(TlenProtocol *proto, TLEN_LIST list, const char *jid);
TLEN_LIST_ITEM *TlenListGetItemPtrFromIndex(TlenProtocol *proto, int index);
TLEN_LIST_ITEM *TlenListFindItemPtrById2(TlenProtocol *proto, TLEN_LIST list, const char *id);

void TlenListAddResource(TlenProtocol *proto, TLEN_LIST list, const char *jid, int status, const char *statusMessage);
void TlenListRemoveResource(TlenProtocol *proto, TLEN_LIST list, const char *jid);

#endif

