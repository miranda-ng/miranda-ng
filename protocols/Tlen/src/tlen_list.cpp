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


static void TlenListFreeItemInternal(TLEN_LIST_ITEM *item);

void TlenListInit(TlenProtocol *proto)
{
	proto->lists = NULL;
	proto->listsCount = 0;
}

void TlenListUninit(TlenProtocol *proto)
{
	TlenListWipe(proto);
}

void TlenListWipe(TlenProtocol *proto)
{
	int i;

	mir_cslock lck(proto->csLists);
	for (i=0; i<proto->listsCount; i++)
		TlenListFreeItemInternal(&(proto->lists[i]));
	if (proto->lists != NULL) {
		mir_free(proto->lists);
		proto->lists = NULL;
	}
	proto->listsCount=0;
}

void TlenListWipeSpecial(TlenProtocol *proto)
{
	int i;
	mir_cslock lck(proto->csLists);
	for (i=0; i<proto->listsCount; i++) {
		if (proto->lists[i].list != LIST_FILE && proto->lists[i].list != LIST_VOICE) {
			TlenListFreeItemInternal(&(proto->lists[i]));
			proto->listsCount--;
			memmove(proto->lists+i, proto->lists+i+1, sizeof(TLEN_LIST_ITEM)*(proto->listsCount-i));
			i--;
		}
	}
	proto->lists = (TLEN_LIST_ITEM *) mir_realloc(proto->lists, sizeof(TLEN_LIST_ITEM)*proto->listsCount);
}

static void TlenListFreeItemInternal(TLEN_LIST_ITEM *item)
{
	if (item == NULL)
		return;

	if (item->jid) mir_free(item->jid);
	if (item->nick) mir_free(item->nick);
	if (item->statusMessage) mir_free(item->statusMessage);
	if (item->group) mir_free(item->group);
	if (item->messageEventIdStr) mir_free(item->messageEventIdStr);
//	if (item->type) mir_free(item->type);
	//if (item->ft) TlenFileFreeFt(item->ft); // No need to free (it is always free when exit from TlenFileServerThread())
	if (item->roomName) mir_free(item->roomName);
	if (item->version) mir_free(item->version);
	if (item->software) mir_free(item->software);
	if (item->system) mir_free(item->system);
	if (item->avatarHash) mir_free(item->avatarHash);

	if (item->protocolVersion) mir_free(item->protocolVersion);
	if (item->id2) mir_free(item->id2);
}

static char * GetItemId(TLEN_LIST list, const char *jid)
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


int TlenListExist(TlenProtocol *proto, TLEN_LIST list, const char *jid)
{
	int i;
	size_t len;
	char *s, *p;
	s = GetItemId(list, jid);
	len = mir_strlen(s);

	mir_cslock lck(proto->csLists);
	for (i=0; i<proto->listsCount; i++)
		if (proto->lists[i].list == list) {
			p = proto->lists[i].jid;
			if (p && mir_strlen(p) >= len && (p[(int)len] == '\0' || p[(int)len] == '/') && !strncmp(p, s, len)) {
				mir_free(s);
				return i+1;
			}
		}
	mir_free(s);
	return 0;
}

TLEN_LIST_ITEM *TlenListAdd(TlenProtocol *proto, TLEN_LIST list, const char *jid)
{
	char *s;
	TLEN_LIST_ITEM *item;

	mir_cslock lck(proto->csLists);
	if ((item=TlenListGetItemPtr(proto, list, jid)) != NULL) {
		return item;
	}

	s = GetItemId(list, jid);
	proto->lists = (TLEN_LIST_ITEM *) mir_realloc(proto->lists, sizeof(TLEN_LIST_ITEM)*(proto->listsCount+1));
	item = &(proto->lists[proto->listsCount]);
	memset(item, 0, sizeof(TLEN_LIST_ITEM));
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

	return item;
}

void TlenListRemove(TlenProtocol *proto, TLEN_LIST list, const char *jid)
{
	int i;

	mir_cslock lck(proto->csLists);
	i = TlenListExist(proto, list, jid);
	if (!i) {
		return;
	}
	i--;
	TlenListFreeItemInternal(&(proto->lists[i]));
	proto->listsCount--;
	memmove(proto->lists+i, proto->lists+i+1, sizeof(TLEN_LIST_ITEM)*(proto->listsCount-i));
	proto->lists = (TLEN_LIST_ITEM *) mir_realloc(proto->lists, sizeof(TLEN_LIST_ITEM)*proto->listsCount);
}

void TlenListRemoveList(TlenProtocol *proto, TLEN_LIST list)
{
	int i;

	i = 0;
	while ((i=TlenListFindNext(proto, list, i)) >= 0) {
		TlenListRemoveByIndex(proto, i);
	}
}

void TlenListRemoveByIndex(TlenProtocol *proto, int index)
{
	mir_cslock lck(proto->csLists);
	if (index >= 0 && index<proto->listsCount) {
		TlenListFreeItemInternal(&(proto->lists[index]));
		proto->listsCount--;
		memmove(proto->lists+index, proto->lists+index+1, sizeof(TLEN_LIST_ITEM)*(proto->listsCount-index));
		proto->lists = (TLEN_LIST_ITEM *) mir_realloc(proto->lists, sizeof(TLEN_LIST_ITEM)*proto->listsCount);
	}
}

void TlenListAddResource(TlenProtocol *proto, TLEN_LIST list, const char *jid, int status, const char *statusMessage)
{
	int i;

	mir_cslock lck(proto->csLists);
	i = TlenListExist(proto, list, jid);
	if (!i) {
		return;
	}
	i--;

	if (proto->lists[i].statusMessage != NULL)
		mir_free(proto->lists[i].statusMessage);
	if (statusMessage)
		proto->lists[i].statusMessage = mir_strdup(statusMessage);
	else
		proto->lists[i].statusMessage = NULL;
}

void TlenListRemoveResource(TlenProtocol *proto, TLEN_LIST list, const char *jid)
{
	int i;
	mir_cslock lck(proto->csLists);
	i = TlenListExist(proto, list, jid);
	if (!i) {
		return;
	}
	i--;
}

int TlenListFindNext(TlenProtocol *proto, TLEN_LIST list, int fromOffset)
{
	int i;

	mir_cslock lck(proto->csLists);
	i = (fromOffset >= 0) ? fromOffset : 0;
	for (; i<proto->listsCount; i++)
		if (proto->lists[i].list == list) {
			return i;
		}
	return -1;
}

TLEN_LIST_ITEM *TlenListGetItemPtr(TlenProtocol *proto, TLEN_LIST list, const char *jid)
{
	int i;

	mir_cslock lck(proto->csLists);
	i = TlenListExist(proto, list, jid);
	if (!i) {
		return NULL;
	}
	i--;
	return &(proto->lists[i]);
}

TLEN_LIST_ITEM *TlenListFindItemPtrById2(TlenProtocol *proto, TLEN_LIST list, const char *id)
{

	int i;
	size_t len;
	char *p;

	len = mir_strlen(id);

	mir_cslock lck(proto->csLists);
	for (i=0; i<proto->listsCount; i++) {
		if (proto->lists[i].list == list) {
			p = proto->lists[i].id2;
			if (p != NULL) {
				if (!strncmp(p, id, len)) {
					return &(proto->lists[i]);
				}
			}
		}
	}
	return NULL;
}

TLEN_LIST_ITEM *TlenListGetItemPtrFromIndex(TlenProtocol *proto, int index)
{
	mir_cslock lck(proto->csLists);
	if (index >= 0 && index<proto->listsCount) {
		return &(proto->lists[index]);
	}
	return NULL;
}

