/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov
Copyright (C) 2012-13  Miranda NG Project

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

#include "jabber.h"
#include "jabber_list.h"

void MenuUpdateSrmmIcon(JABBER_LIST_ITEM *item);

/////////////////////////////////////////////////////////////////////////////////////////
// List item freeing

static void JabberListFreeResourceInternal(JABBER_RESOURCE_STATUS *r)
{
	mir_free(r->resourceName);
	mir_free(r->nick);
	mir_free(r->statusMessage);
	mir_free(r->software);
	mir_free(r->version);
	mir_free(r->system);
	mir_free(r->szCapsNode);
	mir_free(r->szCapsVer);
	mir_free(r->szCapsExt);
	mir_free(r->szRealJid);
	
	if (r->pSoftwareInfo)
		delete r->pSoftwareInfo;
}

static void JabberListFreeItemInternal(JABBER_LIST_ITEM *item)
{
	if (item == NULL)
		return;

	for (int i=0; i < item->resourceCount; i++)
		JabberListFreeResourceInternal(&item->pResources[i]);

	JabberListFreeResourceInternal(&item->itemResource);

	if (item->photoFileName) {
		if (item->list == LIST_VCARD_TEMP)
			DeleteFile(item->photoFileName);
		mir_free(item->photoFileName);
	}

	mir_free(item->jid);
	mir_free(item->nick);
	mir_free(item->pResources);
	mir_free(item->group);
	mir_free(item->messageEventIdStr);
	mir_free(item->name);
	mir_free(item->type);
	mir_free(item->service);
	mir_free(item->password);
	if (item->list == LIST_ROSTER && item->ft)
		delete item->ft;
	mir_free(item);
}

void CJabberProto::ListWipe(void)
{
	mir_cslock lck(m_csLists);
	for (int i=0; i < m_lstRoster.getCount(); i++)
		JabberListFreeItemInternal(m_lstRoster[i]);

	m_lstRoster.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adding & removing items

JABBER_LIST_ITEM *CJabberProto::ListAdd(JABBER_LIST list, const TCHAR *jid)
{
	bool bUseResource = false;
	mir_cslockfull lck(m_csLists);

	JABBER_LIST_ITEM *item = ListGetItemPtr(list, jid);
	if (item != NULL)
		return item;

	TCHAR *s = mir_tstrdup(jid);
	TCHAR *q = NULL;
	// strip resource name if any
	//fyr
	if ( !((list== LIST_ROSTER)  && ListGetItemPtr(LIST_CHATROOM, jid))) { // but only if it is not chat room contact
		if (list != LIST_VCARD_TEMP) {
			TCHAR *p;
			if ((p = _tcschr(s, '@')) != NULL)
				if ((q = _tcschr(p, '/')) != NULL)
					*q = '\0';
		}
	}
	else bUseResource = true;

	if ( !bUseResource && list == LIST_ROSTER) {
		//if it is a chat room keep resource and made it resource sensitive
		if (ChatRoomHContactFromJID(s)) {
			if (q != NULL)
				*q='/';
			bUseResource = true;
		}
	}

	item = (JABBER_LIST_ITEM*)mir_calloc(sizeof(JABBER_LIST_ITEM));
	item->list = list;
	item->jid = s;
	item->itemResource.status = ID_STATUS_OFFLINE;
	item->resourceMode = RSMODE_LASTSEEN;
	item->bUseResource = bUseResource;
	m_lstRoster.insert(item);
	lck.unlock();

	MenuUpdateSrmmIcon(item);
	return item;
}

void CJabberProto::ListRemove(JABBER_LIST list, const TCHAR *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI != NULL) {
		m_lstRoster.remove(LI);
		JabberListFreeItemInternal(LI);
	}
}

void CJabberProto::ListRemoveList(JABBER_LIST list)
{
	int i = 0;
	while ((i=ListFindNext(list, i)) >= 0)
		ListRemoveByIndex(i);
}

void CJabberProto::ListRemoveByIndex(int index)
{
	mir_cslock lck(m_csLists);
	if (index >= 0 && index < m_lstRoster.getCount()) {
		JabberListFreeItemInternal(m_lstRoster[index]);
		m_lstRoster.remove(index);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Getting & finding items

JABBER_LIST_ITEM* CJabberProto::ListGetItemPtr(JABBER_LIST list, const TCHAR *jid)
{
	JABBER_LIST_ITEM tmp;
	tmp.list = list;
	tmp.jid  = (TCHAR*)jid;
	tmp.bUseResource = FALSE;

	mir_cslock lck(m_csLists);
	if (list == LIST_ROSTER) {
		tmp.list = LIST_CHATROOM;
		int id = m_lstRoster.getIndex(&tmp);
		if (id != -1)
			tmp.bUseResource = TRUE;
		tmp.list = list;
	}

	return m_lstRoster.find(&tmp);
}

JABBER_LIST_ITEM* CJabberProto::ListGetItemPtrFromIndex(int index)
{
	mir_cslock lck(m_csLists);
	if (index >= 0 && index < m_lstRoster.getCount())
		return m_lstRoster[index];

	return NULL;
}

int CJabberProto::ListFindNext(JABBER_LIST list, int fromOffset)
{
	mir_cslock lck(m_csLists);
	int i = (fromOffset >= 0) ? fromOffset : 0;
	for (; i < m_lstRoster.getCount(); i++)
		if (m_lstRoster[i]->list == list)
			return i;

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Resource related code

JABBER_RESOURCE_STATUS* JABBER_LIST_ITEM::findResource(const TCHAR *resourceName) const
{
	if (pResources == NULL || resourceName == NULL || *resourceName == 0)
		return NULL;

	for (int i=0; i < resourceCount; i++)
		if ( !_tcscmp(pResources[i].resourceName, resourceName))
			return pResources + i;

	return NULL;
}

JABBER_RESOURCE_STATUS* CJabberProto::ListFindResource(JABBER_LIST list, const TCHAR *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == NULL)
		return NULL;

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	return (q == NULL) ? NULL : LI->findResource(q+1);
}

int CJabberProto::ListAddResource(JABBER_LIST list, const TCHAR *jid, int status, const TCHAR *statusMessage, char priority, const TCHAR *nick)
{
	mir_cslockfull lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == NULL)
		return NULL;

	bool bIsNewResource = false;

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	if (q) {
		const TCHAR *resource = q+1;
		if (resource[0]) {
			JABBER_RESOURCE_STATUS *r = LI->findResource(resource);
			if (r != NULL) { // Already exists, update status and statusMessage
				r->status = status;
				replaceStrT(r->statusMessage, statusMessage);
				r->priority = priority;
			}
			else { // Does not exist, add new resource
				LI->pResources = (JABBER_RESOURCE_STATUS *)mir_realloc(LI->pResources, (LI->resourceCount+1)*sizeof(JABBER_RESOURCE_STATUS));
				bIsNewResource = true;
				r = LI->pResources + LI->resourceCount++;
				memset(r, 0, sizeof(JABBER_RESOURCE_STATUS));
				r->status = status;
				r->affiliation = AFFILIATION_NONE;
				r->role = ROLE_NONE;
				r->resourceName = mir_tstrdup(resource);
				r->nick = mir_tstrdup(nick);
				if (statusMessage)
					r->statusMessage = mir_tstrdup(statusMessage);
				r->priority = priority;
		}	}
	}
	// No resource, update the main statusMessage
	else {
		LI->itemResource.status = status;
		replaceStrT(LI->itemResource.statusMessage, statusMessage);
	}

	lck.unlock();

	MenuUpdateSrmmIcon(LI);
	return bIsNewResource;
}

void CJabberProto::ListRemoveResource(JABBER_LIST list, const TCHAR *jid)
{
	mir_cslockfull lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == NULL)
		return;

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	if (q == NULL)
		return;

	JABBER_RESOURCE_STATUS *r = LI->findResource(q+1);
	if (r == NULL)
		return;

	// Found last seen resource ID to be removed
	if (LI->pLastSeenResource == r)
		LI->pLastSeenResource = NULL;

	// update manually selected resource ID
	if (LI->resourceMode == RSMODE_MANUAL) {
		if (LI->pManualResource == r) {
			LI->resourceMode = RSMODE_LASTSEEN;
			LI->pManualResource = NULL;
		}
	}

	// Update MirVer due to possible resource changes
	UpdateMirVer(LI);

	JabberListFreeResourceInternal(r);

	if (LI->resourceCount-- == 1) {
		mir_free(r);
		LI->pResources = NULL;
	}
	else {
		memmove(r, r+1, (LI->resourceCount - (r-LI->pResources))*sizeof(JABBER_RESOURCE_STATUS));
		LI->pResources = (JABBER_RESOURCE_STATUS*)mir_realloc(LI->pResources, LI->resourceCount*sizeof(JABBER_RESOURCE_STATUS));
	}

	lck.unlock();

	MenuUpdateSrmmIcon(LI);
}

JABBER_RESOURCE_STATUS* JABBER_LIST_ITEM::getBestResource() const
{
	if (!resourceCount)
		return NULL;

	if (resourceCount == 1)
		return pResources;

	if (resourceMode == RSMODE_LASTSEEN)
		return pLastSeenResource;

	if (resourceMode == RSMODE_MANUAL)
		return pManualResource;

	int nBestPos = -1, nBestPri = -200;
	for (int j=0; j < resourceCount; j++) {
		if (pResources[j].priority > nBestPri) {
			nBestPri = pResources[j].priority;
			nBestPos = j;
		}
	}
	
	return (nBestPos != -1) ? &pResources[nBestPos] : NULL;
}

TCHAR* CJabberProto::ListGetBestClientResourceNamePtr(const TCHAR *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, jid);
	if (LI == NULL)
		return NULL;

	JABBER_RESOURCE_STATUS *r = LI->getBestResource();
	if (r != NULL)
		return r->resourceName;

	int status = ID_STATUS_OFFLINE;
	TCHAR *res = NULL;
	for (int i=0; i < LI->resourceCount; i++) {
		bool foundBetter = false;
		switch (r[i].status) {
		case ID_STATUS_FREECHAT:
			foundBetter = true;
			break;
		case ID_STATUS_ONLINE:
			if (status != ID_STATUS_FREECHAT)
				foundBetter = true;
			break;
		case ID_STATUS_DND:
			if (status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE)
				foundBetter = true;
			break;
		case ID_STATUS_AWAY:
			if (status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND)
				foundBetter = true;
			break;
		case ID_STATUS_NA:
			if (status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND && status != ID_STATUS_AWAY)
				foundBetter = true;
			break;
		}
		if (foundBetter) {
			res = LI->pResources[i].resourceName;
			status = LI->pResources[i].status;
	}	}

	return res;
}
