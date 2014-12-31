/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

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
// List item constructor & destructor

JABBER_LIST_ITEM::JABBER_LIST_ITEM() :
	arResources(1, PtrKeySortT)
{
}

JABBER_LIST_ITEM::~JABBER_LIST_ITEM()
{
	for (int i=0; i < arResources.getCount(); i++)
		delete arResources[i];

	if (m_pItemResource)
		delete m_pItemResource;

	if (photoFileName) {
		if (list == LIST_VCARD_TEMP)
			DeleteFile(photoFileName);
		mir_free(photoFileName);
	}

	mir_free(jid);
	mir_free(nick);
	mir_free(group);
	mir_free(messageEventIdStr);
	mir_free(name);
	mir_free(type);
	mir_free(service);
	mir_free(password);
	if (list == LIST_ROSTER && ft)
		delete ft;
}

/////////////////////////////////////////////////////////////////////////////////////////

JABBER_RESOURCE_STATUS::JABBER_RESOURCE_STATUS() :
	m_refCount(1),
	m_iStatus(ID_STATUS_OFFLINE)
{
}

JABBER_RESOURCE_STATUS::~JABBER_RESOURCE_STATUS()
{
}

void JABBER_RESOURCE_STATUS::AddRef()
{
	if (this != NULL)
		::InterlockedIncrement(&m_refCount);
}

void JABBER_RESOURCE_STATUS::Release()
{
	if (this != NULL)
		if (::InterlockedDecrement(&m_refCount) == 0)
			delete this;	
}

void CJabberProto::ListWipe(void)
{
	mir_cslock lck(m_csLists);
	for (int i=0; i < m_lstRoster.getCount(); i++)
		delete m_lstRoster[i];

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
	if (!((list== LIST_ROSTER)  && ListGetItemPtr(LIST_CHATROOM, jid))) { // but only if it is not chat room contact
		if (list != LIST_VCARD_TEMP) {
			TCHAR *p;
			if ((p = _tcschr(s, '@')) != NULL)
				if ((q = _tcschr(p, '/')) != NULL)
					*q = '\0';
		}
	}
	else bUseResource = true;

	if (!bUseResource && list == LIST_ROSTER) {
		//if it is a chat room keep resource and made it resource sensitive
		if (ChatRoomHContactFromJID(s)) {
			if (q != NULL)
				*q='/';
			bUseResource = true;
		}
	}

	item = new JABBER_LIST_ITEM();
	item->list = list;
	item->jid = s;
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
		delete LI;
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
		delete m_lstRoster[index];
		m_lstRoster.remove(index);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Getting & finding items

JABBER_LIST_ITEM* CJabberProto::ListGetItemPtr(JABBER_LIST list, const TCHAR *jid)
{
	JABBER_LIST_ITEM *tmp = (JABBER_LIST_ITEM*)_alloca( sizeof(JABBER_LIST_ITEM));
	tmp->list = list;
	tmp->jid  = (TCHAR*)jid;
	tmp->bUseResource = FALSE;

	mir_cslock lck(m_csLists);
	if (list == LIST_ROSTER) {
		tmp->list = LIST_CHATROOM;
		int id = m_lstRoster.getIndex(tmp);
		if (id != -1)
			tmp->bUseResource = TRUE;
		tmp->list = list;
	}

	return m_lstRoster.find(tmp);
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

pResourceStatus JABBER_LIST_ITEM::findResource(const TCHAR *resourceName) const
{
	if (arResources.getCount() == 0 || resourceName == NULL || *resourceName == 0)
		return NULL;

	for (int i=0; i < arResources.getCount(); i++) {
		JABBER_RESOURCE_STATUS *r = arResources[i];
		if (!_tcscmp(r->m_tszResourceName, resourceName))
			return r;
	}

	return NULL;
}

pResourceStatus CJabberProto::ListFindResource(JABBER_LIST list, const TCHAR *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == NULL)
		return NULL;

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	return (q == NULL) ? NULL : LI->findResource(q+1);
}

bool CJabberProto::ListAddResource(JABBER_LIST list, const TCHAR *jid, int status, const TCHAR *statusMessage, char priority, const TCHAR *nick)
{
	mir_cslockfull lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == NULL)
		return false;

	bool bIsNewResource = false;

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	if (q) {
		const TCHAR *resource = q+1;
		if (*resource == 0)
			return 0;

		JABBER_RESOURCE_STATUS *r = LI->findResource(resource);
		if (r != NULL) { // Already exists, update status and statusMessage
			r->m_iStatus = status;
			r->m_tszStatusMessage = mir_tstrdup(statusMessage);
			r->m_iPriority = priority;
		}
		else { // Does not exist, add new resource
			bIsNewResource = true;
			r = new JABBER_RESOURCE_STATUS();
			r->m_iStatus = status;
			r->m_affiliation = AFFILIATION_NONE;
			r->m_role = ROLE_NONE;
			r->m_tszResourceName = mir_tstrdup(resource);
			r->m_tszNick = mir_tstrdup(nick);
			if (statusMessage)
				r->m_tszStatusMessage = mir_tstrdup(statusMessage);
			r->m_iPriority = priority;
			LI->arResources.insert(r);
		}
	}
	// No resource, update the main statusMessage
	else {
		JABBER_RESOURCE_STATUS *r = LI->getTemp();
		r->m_iStatus = status;
		r->m_tszStatusMessage = mir_tstrdup(statusMessage);
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

	pResourceStatus r( LI->findResource(q+1));
	if (r == NULL)
		return;

	// Found last seen resource ID to be removed
	if (LI->m_pLastSeenResource == r)
		LI->m_pLastSeenResource = NULL;

	// update manually selected resource ID
	if (LI->resourceMode == RSMODE_MANUAL && LI->m_pManualResource == r) {
		LI->resourceMode = RSMODE_LASTSEEN;
		LI->m_pManualResource = NULL;
	}

	// Update MirVer due to possible resource changes
	UpdateMirVer(LI);

	LI->arResources.remove(r);
	r->Release();
	lck.unlock();

	MenuUpdateSrmmIcon(LI);
}

pResourceStatus JABBER_LIST_ITEM::getBestResource() const
{
	if (!arResources.getCount())
		return NULL;

	if (arResources.getCount() == 1)
		return arResources[0];

	if (resourceMode == RSMODE_LASTSEEN)
		return m_pLastSeenResource;

	if (resourceMode == RSMODE_MANUAL)
		return m_pManualResource;

	int nBestPos = -1, nBestPri = -200;
	for (int i=0; i < arResources.getCount(); i++) {
		JABBER_RESOURCE_STATUS *r = arResources[i];
		if (r->m_iPriority > nBestPri) {
			nBestPri = r->m_iPriority;
			nBestPos = i;
		}
	}
	
	return (nBestPos != -1) ? arResources[nBestPos] : NULL;
}

JABBER_RESOURCE_STATUS* JABBER_LIST_ITEM::getTemp()
{
	if (m_pItemResource == NULL)
		m_pItemResource = new JABBER_RESOURCE_STATUS();

	return m_pItemResource;
}

TCHAR* CJabberProto::ListGetBestClientResourceNamePtr(const TCHAR *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, jid);
	if (LI == NULL)
		return NULL;

	pResourceStatus r( LI->getBestResource());
	if (r != NULL)
		return r->m_tszResourceName;

	int status = ID_STATUS_OFFLINE;
	TCHAR *res = NULL;
	for (int i=0; i < LI->arResources.getCount(); i++) {
		r = LI->arResources[i];
		bool foundBetter = false;
		switch (r->m_iStatus) {
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
			res = r->m_tszResourceName;
			status = r->m_iStatus;
		}
	}

	return res;
}
