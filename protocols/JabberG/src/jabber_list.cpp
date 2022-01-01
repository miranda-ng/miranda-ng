/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

#include "stdafx.h"
#include "jabber_list.h"

/////////////////////////////////////////////////////////////////////////////////////////
// List item constructor & destructor

JABBER_LIST_ITEM::JABBER_LIST_ITEM() :
	arResources(1, PtrKeySortT)
{
}

JABBER_LIST_ITEM::~JABBER_LIST_ITEM()
{
	for (auto &it : arResources)
		delete it;

	delete m_pItemResource;

	if (photoFileName) {
		if (list == LIST_VCARD_TEMP)
			DeleteFile(Utf2T(photoFileName));
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
	if (this != nullptr)
		::InterlockedIncrement(&m_refCount);
}

void JABBER_RESOURCE_STATUS::Release()
{
	if (this != nullptr)
		if (::InterlockedDecrement(&m_refCount) == 0)
			delete this;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::ListInit(void)
{
	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact)) {
			ptrA jid(getUStringA(hContact, "ChatRoomID"));
			if (jid != nullptr)
				ListAdd(LIST_CHATROOM, jid, hContact);
		}
		else {
			ptrA jid(getUStringA(hContact, "jid"));
			if (jid != nullptr)
				ListAdd(LIST_ROSTER, jid, hContact);
		}
	}
}

void CJabberProto::ListWipe(void)
{
	mir_cslock lck(m_csLists);
	for (auto &it : m_lstRoster)
		delete it;

	m_lstRoster.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adding & removing items

JABBER_LIST_ITEM* CJabberProto::ListAdd(JABBER_LIST list, const char *jid, MCONTACT hContact)
{
	bool bUseResource = false;
	mir_cslockfull lck(m_csLists);

	JABBER_LIST_ITEM *item = ListGetItemPtr(list, jid);
	if (item != nullptr) {
		if (hContact)
			item->hContact = hContact;
		return item;
	}

	char *s = mir_strdup(jid);
	char *q = nullptr;

	// strip resource name if any
	if (!((list == LIST_ROSTER) && ListGetItemPtr(LIST_CHATROOM, jid))) { // but only if it is not chat room contact
		if (list != LIST_VCARD_TEMP) {
			char *p;
			if ((p = strchr(s, '@')) != nullptr)
				if ((q = strchr(p, '/')) != nullptr)
					*q = '\0';
		}
	}
	else bUseResource = true;

	if (!bUseResource && list == LIST_ROSTER) {
		// if it is a chat room keep resource and made it resource sensitive
		if (ChatRoomHContactFromJID(s)) {
			if (q != nullptr)
				*q = '/';
			bUseResource = true;
		}
	}

	item = new JABBER_LIST_ITEM();
	item->list = list;
	item->jid = s;
	item->hContact = hContact;
	item->resourceMode = RSMODE_LASTSEEN;
	item->bUseResource = bUseResource;
	m_lstRoster.insert(item);
	lck.unlock();

	MenuUpdateSrmmIcon(item);
	return item;
}

void CJabberProto::ListRemove(JABBER_LIST list, const char *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI != nullptr) {
		m_lstRoster.remove(LI);
		delete LI;
	}
}

void CJabberProto::ListRemoveList(JABBER_LIST list)
{
	mir_cslock lck(m_csLists);
	for (auto &it : m_lstRoster.rev_iter())
		if (it->list == list)
			delete m_lstRoster.removeItem(&it);
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

JABBER_LIST_ITEM* CJabberProto::ListGetItemPtr(JABBER_LIST list, const char *jid)
{
	if (jid == nullptr)
		return nullptr;

	JABBER_LIST_ITEM *tmp = (JABBER_LIST_ITEM*)_alloca(sizeof(JABBER_LIST_ITEM));
	tmp->list = list;
	tmp->jid = (char*)jid;
	tmp->bUseResource = false;

	mir_cslock lck(m_csLists);
	if (list == LIST_ROSTER) {
		tmp->list = LIST_CHATROOM;
		int id = m_lstRoster.getIndex(tmp);
		if (id != -1)
			tmp->bUseResource = true;
		tmp->list = list;
	}

	return m_lstRoster.find(tmp);
}

JABBER_LIST_ITEM* CJabberProto::ListGetItemPtrFromIndex(int index)
{
	mir_cslock lck(m_csLists);
	if (index >= 0 && index < m_lstRoster.getCount())
		return m_lstRoster[index];

	return nullptr;
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

pResourceStatus JABBER_LIST_ITEM::findResource(const char *resourceName) const
{
	if (arResources.getCount() == 0 || resourceName == nullptr || *resourceName == 0)
		return nullptr;

	for (auto &it : arResources)
		if (!mir_strcmp(it->m_szResourceName, resourceName))
			return it;

	return nullptr;
}

pResourceStatus CJabberProto::ListFindResource(JABBER_LIST list, const char *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == nullptr)
		return nullptr;

	const char *p = strchr(jid, '@');
	const char *q = strchr((p == nullptr) ? jid : p, '/');
	return (q == nullptr) ? nullptr : LI->findResource(q + 1);
}

bool CJabberProto::ListAddResource(JABBER_LIST list, const char *jid, int status, const char *statusMessage, int priority, const char *nick)
{
	mir_cslockfull lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == nullptr)
		return false;

	bool bIsNewResource = false;

	const char *p = strchr(jid, '@');
	const char *q = strchr((p == nullptr) ? jid : p, '/');
	if (q) {
		const char *resource = q + 1;
		if (*resource == 0)
			return false;

		JABBER_RESOURCE_STATUS *r = LI->findResource(resource);
		if (r != nullptr) { // Already exists, update status and statusMessage
			r->m_iStatus = status;
			r->m_szStatusMessage = mir_strdup(statusMessage);
			r->m_iPriority = priority;
		}
		else { // Does not exist, add new resource
			bIsNewResource = true;
			r = new JABBER_RESOURCE_STATUS();
			r->m_iStatus = status;
			r->m_affiliation = AFFILIATION_NONE;
			r->m_role = ROLE_NONE;
			r->m_szResourceName = mir_strdup(resource);
			r->m_szNick = mir_strdup(nick);
			if (statusMessage)
				r->m_szStatusMessage = mir_strdup(statusMessage);
			r->m_iPriority = priority;
			LI->arResources.insert(r);
		}
	}
	// No resource, update the main statusMessage
	else {
		JABBER_RESOURCE_STATUS *r = LI->getTemp();
		r->m_iStatus = status;
		r->m_szStatusMessage = mir_strdup(statusMessage);
	}

	lck.unlock();

	MenuUpdateSrmmIcon(LI);
	return bIsNewResource;
}

void CJabberProto::ListRemoveResource(JABBER_LIST list, const char *jid)
{
	mir_cslockfull lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(list, jid);
	if (LI == nullptr)
		return;

	const char *p = strchr(jid, '@');
	const char *q = strchr((p == nullptr) ? jid : p, '/');
	if (q == nullptr)
		return;

	pResourceStatus r(LI->findResource(q + 1));
	if (r == nullptr)
		return;

	// Found last seen resource ID to be removed
	if (LI->m_pLastSeenResource == r)
		LI->m_pLastSeenResource = nullptr;

	// update manually selected resource ID
	if (LI->resourceMode == RSMODE_MANUAL && LI->m_pManualResource == r) {
		LI->resourceMode = RSMODE_LASTSEEN;
		LI->m_pManualResource = nullptr;
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
		return nullptr;

	if (arResources.getCount() == 1)
		return arResources[0];

	if (resourceMode == RSMODE_LASTSEEN && m_pLastSeenResource)
		return m_pLastSeenResource;

	if (resourceMode == RSMODE_MANUAL && m_pManualResource)
		return m_pManualResource;

	int nBestPos = -1, nBestPri = -200;
	for (auto &r : arResources)
		if (r->m_iPriority > nBestPri) {
			nBestPri = r->m_iPriority;
			nBestPos = arResources.indexOf(&r);
		}

	return (nBestPos != -1) ? arResources[nBestPos] : nullptr;
}

JABBER_RESOURCE_STATUS* JABBER_LIST_ITEM::getTemp()
{
	if (m_pItemResource == nullptr)
		m_pItemResource = new JABBER_RESOURCE_STATUS();

	return m_pItemResource;
}

char* CJabberProto::ListGetBestClientResourceNamePtr(const char *jid)
{
	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, jid);
	if (LI == nullptr)
		return nullptr;

	pResourceStatus r(LI->getBestResource());
	if (r != nullptr)
		return r->m_szResourceName;

	return nullptr;
}
