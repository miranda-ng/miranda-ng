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

#ifndef _JABBER_LIST_H_
#define _JABBER_LIST_H_

typedef struct JABBER_LIST_ITEM_STRUCT {
	JABBER_LIST list;
	char *jid;
	char *id2;

	// LIST_ROSTER
	// jid = jid of the contact
	char *nick;
	int status;	// Main status, currently useful for transport where no resource information is kept.
				// On normal contact, this is the same status as shown on contact list.
	JABBER_SUBSCRIPTION subscription;
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
	// JABBER_RESOURCE_STATUS *resource;	// participant nicks in this room
	char *roomName;

	// LIST_FILE
	struct TLEN_FILE_TRANSFER_STRUCT *ft;
} JABBER_LIST_ITEM;


void JabberListInit(TlenProtocol *proto);
void JabberListUninit(TlenProtocol *proto);
void JabberListWipe(TlenProtocol *proto);
void JabberListWipeSpecial(TlenProtocol *proto);
int JabberListExist(TlenProtocol *proto, JABBER_LIST list, const char *jid);
JABBER_LIST_ITEM *JabberListAdd(TlenProtocol *proto, JABBER_LIST list, const char *jid);
void JabberListRemove(TlenProtocol *proto, JABBER_LIST list, const char *jid);
void JabberListRemoveList(TlenProtocol *proto, JABBER_LIST list);
void JabberListRemoveByIndex(TlenProtocol *proto, int index);
int JabberListFindNext(TlenProtocol *proto, JABBER_LIST list, int fromOffset);
JABBER_LIST_ITEM *JabberListGetItemPtr(TlenProtocol *proto, JABBER_LIST list, const char *jid);
JABBER_LIST_ITEM *JabberListGetItemPtrFromIndex(TlenProtocol *proto, int index);
JABBER_LIST_ITEM *JabberListFindItemPtrById2(TlenProtocol *proto, JABBER_LIST list, const char *id);

void JabberListAddResource(TlenProtocol *proto, JABBER_LIST list, const char *jid, int status, const char *statusMessage);
void JabberListRemoveResource(TlenProtocol *proto, JABBER_LIST list, const char *jid);

#endif

