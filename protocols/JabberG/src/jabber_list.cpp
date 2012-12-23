/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov

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
	if (r->resourceName) mir_free(r->resourceName);
	if (r->nick) mir_free(r->nick);
	if (r->statusMessage) mir_free(r->statusMessage);
	if (r->software) mir_free(r->software);
	if (r->version) mir_free(r->version);
	if (r->system) mir_free(r->system);
	if (r->szCapsNode) mir_free(r->szCapsNode);
	if (r->szCapsVer) mir_free(r->szCapsVer);
	if (r->szCapsExt) mir_free(r->szCapsExt);
	if (r->szRealJid) mir_free(r->szRealJid);
	if (r->pSoftwareInfo) delete r->pSoftwareInfo;
}

static void JabberListFreeItemInternal(JABBER_LIST_ITEM *item)
{
	if (item == NULL)
		return;

	JABBER_RESOURCE_STATUS* r = item->resource;
	for (int i=0; i < item->resourceCount; i++, r++)
		JabberListFreeResourceInternal(r);

	JabberListFreeResourceInternal(&item->itemResource);
	
	if (item->photoFileName) {
		if (item->list == LIST_VCARD_TEMP)
			DeleteFile(item->photoFileName);
		mir_free(item->photoFileName);
	}

	mir_free(item->jid);
	mir_free(item->nick);
	mir_free(item->resource);
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
	int i;

	EnterCriticalSection(&m_csLists);
	for (i=0; i < m_lstRoster.getCount(); i++)
		JabberListFreeItemInternal(m_lstRoster[i]);

	m_lstRoster.destroy();
	LeaveCriticalSection(&m_csLists);
}

int CJabberProto::ListExist(JABBER_LIST list, const TCHAR *jid)
{
	JABBER_LIST_ITEM tmp;
	tmp.list = list;
	tmp.jid  = (TCHAR*)jid;
	tmp.bUseResource = FALSE;

	EnterCriticalSection(&m_csLists);

	//fyr
	if (list == LIST_ROSTER)
	{
		tmp.list = LIST_CHATROOM;
		int id = m_lstRoster.getIndex(&tmp);
		if (id != -1) 
			tmp.bUseResource = TRUE;
		tmp.list = list;
	}
	
	int idx = m_lstRoster.getIndex(&tmp);

	if (idx == -1) {
		LeaveCriticalSection(&m_csLists);
		return 0;
	}

	LeaveCriticalSection(&m_csLists);
	return idx+1;
}

JABBER_LIST_ITEM *CJabberProto::ListAdd(JABBER_LIST list, const TCHAR *jid)
{
	JABBER_LIST_ITEM* item;
	BOOL bUseResource=FALSE;
	EnterCriticalSection(&m_csLists);
	if ((item = ListGetItemPtr(list, jid)) != NULL) {
		LeaveCriticalSection(&m_csLists);
		return item;
	}

	TCHAR *s = mir_tstrdup(jid);
	TCHAR *q = NULL;
	// strip resource name if any
	//fyr
	if ( !((list== LIST_ROSTER)  && ListExist(LIST_CHATROOM, jid))) { // but only if it is not chat room contact	
		if (list != LIST_VCARD_TEMP) {
			TCHAR *p;
			if ((p = _tcschr(s, '@')) != NULL)
				if ((q = _tcschr(p, '/')) != NULL)
					*q = '\0';
		}
	} else {
		bUseResource=TRUE;
	}
	
	if ( !bUseResource && list== LIST_ROSTER)
	{
		//if it is a chat room keep resource and made it resource sensitive
		if (ChatRoomHContactFromJID(s))
		{
			if (q != NULL)	*q='/';
			bUseResource=TRUE;
		}
	}
	
	item = (JABBER_LIST_ITEM*)mir_calloc(sizeof(JABBER_LIST_ITEM));
	item->list = list;
	item->jid = s;
	item->itemResource.status = ID_STATUS_OFFLINE;
	item->resource = NULL;
	item->resourceMode = RSMODE_LASTSEEN;
	item->lastSeenResource = -1;
	item->manualResource = -1;
	item->bUseResource = bUseResource;

	m_lstRoster.insert(item);
	LeaveCriticalSection(&m_csLists);

	MenuUpdateSrmmIcon(item);
	return item;
}

void CJabberProto::ListRemove(JABBER_LIST list, const TCHAR *jid)
{
	EnterCriticalSection(&m_csLists);
	int i = ListExist(list, jid);
	if (i != 0) {
		JabberListFreeItemInternal(m_lstRoster[ --i ]);
		m_lstRoster.remove(i);
	}
	LeaveCriticalSection(&m_csLists);
}

void CJabberProto::ListRemoveList(JABBER_LIST list)
{
	int i = 0;
	while ((i=ListFindNext(list, i)) >= 0)
		ListRemoveByIndex(i);
}

void CJabberProto::ListRemoveByIndex(int index)
{
	EnterCriticalSection(&m_csLists);
	if (index >= 0 && index < m_lstRoster.getCount()) {
		JabberListFreeItemInternal(m_lstRoster[index]);
		m_lstRoster.remove(index);
	}
	LeaveCriticalSection(&m_csLists);
}

JABBER_RESOURCE_STATUS *CJabberProto::ListFindResource(JABBER_LIST list, const TCHAR *jid)
{
	JABBER_RESOURCE_STATUS *result = NULL;

	EnterCriticalSection(&m_csLists);
	int i = ListExist(list, jid);
	if ( !i) {
		LeaveCriticalSection(&m_csLists);
		return 0;
	}

	JABBER_LIST_ITEM* LI = m_lstRoster[i-1];

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	if (q)
	{
		const TCHAR *resource = q+1;
		if (*resource)
			for (int j=0; j < LI->resourceCount; j++)
				if ( !_tcscmp(LI->resource[j].resourceName, resource))
				{
					result = LI->resource + j;
					break;
				}
	}

	LeaveCriticalSection(&m_csLists);

	return result;
}

int CJabberProto::ListAddResource(JABBER_LIST list, const TCHAR *jid, int status, const TCHAR *statusMessage, char priority, const TCHAR *nick)
{
	EnterCriticalSection(&m_csLists);
	int i = ListExist(list, jid);
	if ( !i) {
		LeaveCriticalSection(&m_csLists);
		return 0;
	}

	JABBER_LIST_ITEM* LI = m_lstRoster[i-1];
	int bIsNewResource = false, j;

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	if (q) {
		const TCHAR *resource = q+1;
		if (resource[0]) {
			JABBER_RESOURCE_STATUS* r = LI->resource;
			for (j=0; j < LI->resourceCount; j++, r++) {
				if ( !_tcscmp(r->resourceName, resource)) {
					// Already exist, update status and statusMessage
					r->status = status;
					replaceStrT(r->statusMessage, statusMessage);
					r->priority = priority;
					break;
			}	}

			if (j >= LI->resourceCount) {
				// Not already exist, add new resource
				LI->resource = (JABBER_RESOURCE_STATUS *) mir_realloc(LI->resource, (LI->resourceCount+1)*sizeof(JABBER_RESOURCE_STATUS));
				bIsNewResource = true;
				r = LI->resource + LI->resourceCount++;
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

	LeaveCriticalSection(&m_csLists);

	MenuUpdateSrmmIcon(LI);
	return bIsNewResource;
}

void CJabberProto::ListRemoveResource(JABBER_LIST list, const TCHAR *jid)
{
	EnterCriticalSection(&m_csLists);
	int i = ListExist(list, jid);
	JABBER_LIST_ITEM* LI = m_lstRoster[i-1];
	if ( !i || LI == NULL) {
		LeaveCriticalSection(&m_csLists);
		return;
	}

	const TCHAR *p = _tcschr(jid, '@');
	const TCHAR *q = _tcschr((p == NULL) ? jid : p, '/');
	if (q) {
		const TCHAR *resource = q+1;
		if (resource[0]) {
			JABBER_RESOURCE_STATUS* r = LI->resource;
			int j;
			for (j=0; j < LI->resourceCount; j++, r++) {
				if ( !_tcsicmp(r->resourceName, resource))
					break;
			}
			if (j < LI->resourceCount) {
				// Found last seen resource ID to be removed
				if (LI->lastSeenResource == j)
					LI->lastSeenResource = -1;
				else if (LI->lastSeenResource > j)
					LI->lastSeenResource--;
				// update manually selected resource ID
				if (LI->resourceMode == RSMODE_MANUAL)
				{
					if (LI->manualResource == j)
					{
						LI->resourceMode = RSMODE_LASTSEEN;
						LI->manualResource = -1;
					} else if (LI->manualResource > j)
						LI->manualResource--;
				}

				// Update MirVer due to possible resource changes
				UpdateMirVer(LI);

				JabberListFreeResourceInternal(r);

				if (LI->resourceCount-- == 1) {
					mir_free(r);
					LI->resource = NULL;
				}
				else {
					memmove(r, r+1, (LI->resourceCount-j)*sizeof(JABBER_RESOURCE_STATUS));
					LI->resource = (JABBER_RESOURCE_STATUS*)mir_realloc(LI->resource, LI->resourceCount*sizeof(JABBER_RESOURCE_STATUS));
	}	}	}	}

	LeaveCriticalSection(&m_csLists);

	MenuUpdateSrmmIcon(LI);
}

TCHAR* CJabberProto::ListGetBestResourceNamePtr(const TCHAR *jid)
{
	EnterCriticalSection(&m_csLists);
	int i = ListExist(LIST_ROSTER, jid);
	if ( !i) {
		LeaveCriticalSection(&m_csLists);
		return NULL;
	}

	TCHAR* res = NULL;

	JABBER_LIST_ITEM* LI = m_lstRoster[i-1];
	if (LI->resourceCount > 1) {
		if (LI->resourceMode == RSMODE_LASTSEEN && LI->lastSeenResource>=0 && LI->lastSeenResource < LI->resourceCount)
			res = LI->resource[ LI->lastSeenResource ].resourceName;
		else if (LI->resourceMode == RSMODE_MANUAL && LI->manualResource>=0 && LI->manualResource < LI->resourceCount)
			res = LI->resource[ LI->manualResource ].resourceName;
		else {
			int nBestPos = -1, nBestPri = -200, j;
			for (j = 0; j < LI->resourceCount; j++) {
				if (LI->resource[ j ].priority > nBestPri) {
					nBestPri = LI->resource[ j ].priority;
					nBestPos = j;
				}
			}
			if (nBestPos != -1)
				res = LI->resource[ nBestPos ].resourceName;
		}
	}

	if ( !res && LI->resource)
		res = LI->resource[0].resourceName;

	LeaveCriticalSection(&m_csLists);
	return res;
}

TCHAR* CJabberProto::ListGetBestClientResourceNamePtr(const TCHAR *jid)
{
	EnterCriticalSection(&m_csLists);
	int i = ListExist(LIST_ROSTER, jid);
	if ( !i) {
		LeaveCriticalSection(&m_csLists);
		return NULL;
	}

	JABBER_LIST_ITEM* LI = m_lstRoster[i-1];
	TCHAR* res = ListGetBestResourceNamePtr(jid);
	if (res == NULL) {
		JABBER_RESOURCE_STATUS* r = LI->resource;
		int status = ID_STATUS_OFFLINE;
		res = NULL;
		for (i=0; i < LI->resourceCount; i++) {
			int s = r[i].status;
			BOOL foundBetter = FALSE;
			switch (s) {
			case ID_STATUS_FREECHAT:
				foundBetter = TRUE;
				break;
			case ID_STATUS_ONLINE:
				if (status != ID_STATUS_FREECHAT)
					foundBetter = TRUE;
				break;
			case ID_STATUS_DND:
				if (status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE)
					foundBetter = TRUE;
				break;
			case ID_STATUS_AWAY:
				if (status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND)
					foundBetter = TRUE;
				break;
			case ID_STATUS_NA:
				if (status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND && status != ID_STATUS_AWAY)
					foundBetter = TRUE;
				break;
			}
			if (foundBetter) {
				res = r[i].resourceName;
				status = s;
	}	}	}

	LeaveCriticalSection(&m_csLists);
	return res;
}

int CJabberProto::ListFindNext(JABBER_LIST list, int fromOffset)
{
	EnterCriticalSection(&m_csLists);
	int i = (fromOffset >= 0) ? fromOffset : 0;
	for (; i<m_lstRoster.getCount(); i++)
		if (m_lstRoster[i]->list == list) {
		  	LeaveCriticalSection(&m_csLists);
			return i;
		}
	LeaveCriticalSection(&m_csLists);
	return -1;
}

JABBER_LIST_ITEM *CJabberProto::ListGetItemPtr(JABBER_LIST list, const TCHAR *jid)
{
	EnterCriticalSection(&m_csLists);
	int i = ListExist(list, jid);
	if ( !i) {
		LeaveCriticalSection(&m_csLists);
		return NULL;
	}
	i--;
	LeaveCriticalSection(&m_csLists);
	return m_lstRoster[i];
}

JABBER_LIST_ITEM *CJabberProto::ListGetItemPtrFromIndex(int index)
{
	EnterCriticalSection(&m_csLists);
	if (index >= 0 && index < m_lstRoster.getCount()) {
		LeaveCriticalSection(&m_csLists);
		return m_lstRoster[index];
	}
	LeaveCriticalSection(&m_csLists);
	return NULL;
}

BOOL CJabberProto::ListLock()
{
	EnterCriticalSection(&m_csLists);
	return TRUE;
}

BOOL CJabberProto::ListUnlock()
{
	LeaveCriticalSection(&m_csLists);
	return TRUE;
}
