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
#include "tlen_list.h"


static void JabberListFreeItemInternal(JABBER_LIST_ITEM *item);

void JabberListInit(TlenProtocol *proto)
{
	proto->lists = NULL;
	proto->listsCount = 0;
	InitializeCriticalSection(&proto->csLists);
}

void JabberListUninit(TlenProtocol *proto)
{
	JabberListWipe(proto);
	DeleteCriticalSection(&proto->csLists);
}

void JabberListWipe(TlenProtocol *proto)
{
	int i;

	EnterCriticalSection(&proto->csLists);
	for (i=0; i<proto->listsCount; i++)
		JabberListFreeItemInternal(&(proto->lists[i]));
	if (proto->lists != NULL) {
		mir_free(proto->lists);
		proto->lists = NULL;
	}
	proto->listsCount=0;
	LeaveCriticalSection(&proto->csLists);
}

void JabberListWipeSpecial(TlenProtocol *proto)
{
	int i;
	EnterCriticalSection(&proto->csLists);
	for (i=0; i<proto->listsCount; i++) {
		if (proto->lists[i].list != LIST_FILE && proto->lists[i].list != LIST_VOICE) {
			JabberListFreeItemInternal(&(proto->lists[i]));
			proto->listsCount--;
			memmove(proto->lists+i, proto->lists+i+1, sizeof(JABBER_LIST_ITEM)*(proto->listsCount-i));
			i--;
		}
	}
	proto->lists = (JABBER_LIST_ITEM *) mir_realloc(proto->lists, sizeof(JABBER_LIST_ITEM)*proto->listsCount);
	LeaveCriticalSection(&proto->csLists);
}

static void JabberListFreeItemInternal(JABBER_LIST_ITEM *item)
{
	if (item == NULL)
		return;

	if (item->jid) mir_free(item->jid);
	if (item->nick) mir_free(item->nick);
	if (item->statusMessage) mir_free(item->statusMessage);
	if (item->group) mir_free(item->group);
	if (item->messageEventIdStr) mir_free(item->messageEventIdStr);
//	if (item->type) mir_free(item->type);
	//if (item->ft) JabberFileFreeFt(item->ft); // No need to free (it is always free when exit from JabberFileServerThread())
	if (item->roomName) mir_free(item->roomName);
	if (item->version) mir_free(item->version);
	if (item->software) mir_free(item->software);
	if (item->system) mir_free(item->system);
	if (item->avatarHash) mir_free(item->avatarHash);

	if (item->protocolVersion) mir_free(item->protocolVersion);
	if (item->id2) mir_free(item->id2);
}

static char * GetItemId(JABBER_LIST list, const char *jid)
{
	char *s, *p, *q;
	s = mir_strdup(jid);
	if (list != LIST_PICTURE) {
		_strlwr(s);
		// strip resouce name if any
		if ((p=strchr(s, '@')) != NULL) {
			if ((q=strchr(p, '/')) != NULL)
				*q = '\0';
		}
	}
	return s;
}


int JabberListExist(TlenProtocol *proto, JABBER_LIST list, const char *jid)
{
	int i;
	size_t len;
	char *s, *p;
	s = GetItemId(list, jid);
	len = strlen(s);

	EnterCriticalSection(&proto->csLists);
	for (i=0; i<proto->listsCount; i++)
		if (proto->lists[i].list == list) {
			p = proto->lists[i].jid;
			if (p && strlen(p) >= len && (p[(int)len] == '\0' || p[(int)len] == '/') && !strncmp(p, s, len)) {
				LeaveCriticalSection(&proto->csLists);
				mir_free(s);
				return i+1;
			}
		}
	LeaveCriticalSection(&proto->csLists);
	mir_free(s);
	return 0;
}

JABBER_LIST_ITEM *JabberListAdd(TlenProtocol *proto, JABBER_LIST list, const char *jid)
{
	char *s;
	JABBER_LIST_ITEM *item;

	EnterCriticalSection(&proto->csLists);
	if ((item=JabberListGetItemPtr(proto, list, jid)) != NULL) {
		LeaveCriticalSection(&proto->csLists);
		return item;
	}

	s = GetItemId(list, jid);
	proto->lists = (JABBER_LIST_ITEM *) mir_realloc(proto->lists, sizeof(JABBER_LIST_ITEM)*(proto->listsCount+1));
	item = &(proto->lists[proto->listsCount]);
	memset(item, 0, sizeof(JABBER_LIST_ITEM));
	item->list = list;
	item->jid = s;
	item->nick = NULL;
	item->status = ID_STATUS_OFFLINE;
	item->statusMessage = NULL;
	item->group = NULL;
	item->messageEventIdStr = NULL;
	item->wantComposingEvent = FALSE;
	item->isTyping = FALSE;
//	item->type = NULL;
	item->ft = NULL;
	item->roomName = NULL;
	item->version = NULL;
	item->software = NULL;
	item->system = NULL;
	item->avatarHash = NULL;
	item->avatarFormat = PA_FORMAT_UNKNOWN;
	item->newAvatarDownloading = FALSE;
	item->versionRequested = FALSE;
	item->infoRequested = FALSE;
	proto->listsCount++;
	LeaveCriticalSection(&proto->csLists);

	return item;
}

void JabberListRemove(TlenProtocol *proto, JABBER_LIST list, const char *jid)
{
	int i;

	EnterCriticalSection(&proto->csLists);
	i = JabberListExist(proto, list, jid);
	if (!i) {
		LeaveCriticalSection(&proto->csLists);
		return;
	}
	i--;
	JabberListFreeItemInternal(&(proto->lists[i]));
	proto->listsCount--;
	memmove(proto->lists+i, proto->lists+i+1, sizeof(JABBER_LIST_ITEM)*(proto->listsCount-i));
	proto->lists = (JABBER_LIST_ITEM *) mir_realloc(proto->lists, sizeof(JABBER_LIST_ITEM)*proto->listsCount);
	LeaveCriticalSection(&proto->csLists);
}

void JabberListRemoveList(TlenProtocol *proto, JABBER_LIST list)
{
	int i;

	i = 0;
	while ((i=JabberListFindNext(proto, list, i)) >= 0) {
		JabberListRemoveByIndex(proto, i);
	}
}

void JabberListRemoveByIndex(TlenProtocol *proto, int index)
{
	EnterCriticalSection(&proto->csLists);
	if (index >= 0 && index<proto->listsCount) {
		JabberListFreeItemInternal(&(proto->lists[index]));
		proto->listsCount--;
		memmove(proto->lists+index, proto->lists+index+1, sizeof(JABBER_LIST_ITEM)*(proto->listsCount-index));
		proto->lists = (JABBER_LIST_ITEM *) mir_realloc(proto->lists, sizeof(JABBER_LIST_ITEM)*proto->listsCount);
	}
	LeaveCriticalSection(&proto->csLists);
}

void JabberListAddResource(TlenProtocol *proto, JABBER_LIST list, const char *jid, int status, const char *statusMessage)
{
	int i;

	EnterCriticalSection(&proto->csLists);
	i = JabberListExist(proto, list, jid);
	if (!i) {
		LeaveCriticalSection(&proto->csLists);
		return;
	}
	i--;

	if (proto->lists[i].statusMessage != NULL)
		mir_free(proto->lists[i].statusMessage);
	if (statusMessage)
		proto->lists[i].statusMessage = mir_strdup(statusMessage);
	else
		proto->lists[i].statusMessage = NULL;
	LeaveCriticalSection(&proto->csLists);
}

void JabberListRemoveResource(TlenProtocol *proto, JABBER_LIST list, const char *jid)
{
	int i;
	EnterCriticalSection(&proto->csLists);
	i = JabberListExist(proto, list, jid);
	if (!i) {
		LeaveCriticalSection(&proto->csLists);
		return;
	}
	i--;
	LeaveCriticalSection(&proto->csLists);
}

int JabberListFindNext(TlenProtocol *proto, JABBER_LIST list, int fromOffset)
{
	int i;

	EnterCriticalSection(&proto->csLists);
	i = (fromOffset >= 0) ? fromOffset : 0;
	for (; i<proto->listsCount; i++)
		if (proto->lists[i].list == list) {
			LeaveCriticalSection(&proto->csLists);
			return i;
		}
	LeaveCriticalSection(&proto->csLists);
	return -1;
}

JABBER_LIST_ITEM *JabberListGetItemPtr(TlenProtocol *proto, JABBER_LIST list, const char *jid)
{
	int i;

	EnterCriticalSection(&proto->csLists);
	i = JabberListExist(proto, list, jid);
	if (!i) {
		LeaveCriticalSection(&proto->csLists);
		return NULL;
	}
	i--;
	LeaveCriticalSection(&proto->csLists);
	return &(proto->lists[i]);
}

JABBER_LIST_ITEM *JabberListFindItemPtrById2(TlenProtocol *proto, JABBER_LIST list, const char *id)
{

	int i;
	size_t len;
	char *p;

	len = strlen(id);

	EnterCriticalSection(&proto->csLists);
	for (i=0; i<proto->listsCount; i++) {
		if (proto->lists[i].list == list) {
			p = proto->lists[i].id2;
			if (p != NULL) {
				if (!strncmp(p, id, len)) {
					LeaveCriticalSection(&proto->csLists);
					return &(proto->lists[i]);
				}
			}
		}
	}
	LeaveCriticalSection(&proto->csLists);
	return NULL;
}

JABBER_LIST_ITEM *JabberListGetItemPtrFromIndex(TlenProtocol *proto, int index)
{
	EnterCriticalSection(&proto->csLists);
	if (index >= 0 && index<proto->listsCount) {
		LeaveCriticalSection(&proto->csLists);
		return &(proto->lists[index]);
	}
	LeaveCriticalSection(&proto->csLists);
	return NULL;
}

