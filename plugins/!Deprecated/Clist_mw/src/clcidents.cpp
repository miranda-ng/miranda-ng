/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "commonheaders.h"
#include "clc.h"

/* the CLC uses 3 different ways to identify elements in its list, this file
contains routines to convert between them.

1) struct ClcContact/ClcGroup pair. Only ever used within the duration
of a single operation, but used at some point in nearly everything
2) index integer. The 0-based number of the item from the top. Only visible
items are counted (ie not closed groups). Used for saving selection and drag
highlight
3) hItem handle. Either the hContact or (hGroup|HCONTACT_ISGROUP). Used
exclusively externally

1->2: GetRowsPriorTo()
1->3: ContactToHItem()
3->1: FindItem()
2->1: GetRowByIndex()
*/

int GetRowsPriorTo(ClcGroup *group,ClcGroup *subgroup,int contactIndex)
{
	int count = 0;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
				break;
			group->scanIndex++;
			continue;
		}

		if (group == subgroup && contactIndex == group->scanIndex)
			return count;

		count++;
		if ((group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) && (group->cl.items[group->scanIndex]->flags & CONTACTF_STATUSMSG))
			count++;

		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			if (group->cl.items[group->scanIndex]->group == subgroup && contactIndex == -1)
				return count-1;
			if (group->cl.items[group->scanIndex]->group->expanded) {
				group = group->cl.items[group->scanIndex]->group;
				group->scanIndex = 0;
				continue;
			}
		}

		if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT)
			count += group->cl.items[group->scanIndex]->SubAllocated*group->cl.items[group->scanIndex]->SubExpanded;

		group->scanIndex++;
	}
	return -1;
}

ClcCacheEntry *GetCLCFullCacheEntry(struct ClcData *dat,MCONTACT hContact)
{
	if (hContact == 0)
		return NULL;

	ClcCacheEntry dnce;
	dnce.hContact = hContact;
	ClcCacheEntry *pdnce = (ClcCacheEntry*)List_Find(&dat->lCLCContactsCache,&dnce);
	if (pdnce == NULL) {
		pdnce = (ClcCacheEntry*)mir_calloc(sizeof(ClcCacheEntry));
		pdnce->hContact = hContact;

		int idx;
		List_GetIndex(&dat->lCLCContactsCache,pdnce,&idx);
		List_Insert(&dat->lCLCContactsCache,pdnce,idx);

		ClcCacheEntry *pdnce2 = (ClcCacheEntry*)List_Find(&dat->lCLCContactsCache,&dnce);//for check
		if (pdnce2->hContact != pdnce->hContact)
			return NULL;

		if (pdnce2 != pdnce)
			return NULL;
	}

	return (pdnce);
}

void ClearClcContactCache(struct ClcData *dat,MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry;

	if (hContact == INVALID_CONTACT_ID) {
		int i,tick;
		tick = GetTickCount();

		for (i = 0;i<(dat->lCLCContactsCache.realCount);i++) {
			ClcCacheEntry *pdnce = (ClcCacheEntry *)dat->lCLCContactsCache.items[i];
			pdnce->ClcContact = NULL;
		}
		tick = GetTickCount()-tick;
		{
			char buf[256];
			mir_snprintf(buf, SIZEOF(buf), "Clear full cache %d ms\r\n", tick);
			OutputDebugStringA(buf);
		}
	}
	if ( !IsHContactGroup(hContact) && !IsHContactInfo(hContact)) {
		cacheEntry = GetCLCFullCacheEntry(dat,hContact);
		if (cacheEntry != NULL)
			cacheEntry->ClcContact = NULL;
	}
}

void SetClcContactCacheItem(struct ClcData *dat, MCONTACT hContact, void *contact)
{
	ClcCacheEntry *cacheEntry;
	if ( !IsHContactGroup(hContact) && !IsHContactInfo(hContact)) {
		cacheEntry = GetCLCFullCacheEntry(dat,hContact);
		if (cacheEntry != NULL)
			cacheEntry->ClcContact = contact;
	}
}

int FindItem(HWND hwnd, struct ClcData *dat, DWORD dwItem, struct ClcContact **contact, ClcGroup **subgroup, int *isVisible)
{
	int index = 0, i;
	int nowVisible = 1;
	ClcGroup *group = &dat->list;

	group->scanIndex = 0;

	if (isVisible == NULL && dwItem != NULL && subgroup == NULL && !IsHContactGroup(dwItem) && !IsHContactInfo(dwItem)) {
		//try use cache
		ClcCacheEntry *cacheEntry = GetCLCFullCacheEntry(dat, dwItem);
		if (cacheEntry != NULL) {
			if (cacheEntry->ClcContact == NULL) {
				int *isv = {0};
				void *z = {0};
				int ret;
				ret = FindItem(hwnd,dat,dwItem,(struct ClcContact ** )&z,(struct  ClcGroup** )&isv,NULL);
				if (ret == 0) {return 0;}
				cacheEntry->ClcContact = (void *)z;
			}

			if (cacheEntry->ClcContact != NULL) {
				if (contact != NULL)
					*contact = (struct ClcContact *)cacheEntry->ClcContact;

				return 1;
			}
		}
	}

	group = &dat->list;

	for (;;) {
		if (group->scanIndex == group->cl.count) {
			ClcGroup *tgroup;
			group = group->parent;
			if (group == NULL)
				break;

			nowVisible = 1;
			for (tgroup = group;tgroup;tgroup = tgroup->parent)
				if ( !group->expanded) {
					nowVisible = 0;
					break;
				}

			group->scanIndex++;
			continue;
		}
		if (nowVisible) index++;
		if ((IsHContactGroup(dwItem) && group->cl.items[group->scanIndex]->type == CLCIT_GROUP && (dwItem & ~HCONTACT_ISGROUP) == group->cl.items[group->scanIndex]->groupId) ||
			 (IsHContactContact(dwItem) && group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && group->cl.items[group->scanIndex]->hContact == dwItem) ||
			 (IsHContactInfo(dwItem) && group->cl.items[group->scanIndex]->type == CLCIT_INFO && group->cl.items[group->scanIndex]->hContact == (dwItem & ~HCONTACT_ISINFO)))
		{
			if (isVisible) {
				if ( !nowVisible) *isVisible = 0;
				else {
					if ((index+1)*dat->rowHeight<dat->yScroll) *isVisible = 0;
					else {
						RECT clRect;
						GetClientRect(hwnd,&clRect);
						if (index*dat->rowHeight>=dat->yScroll+clRect.bottom) *isVisible = 0;
						else *isVisible = 1;
			}	}	}

			if (contact) *contact = group->cl.items[group->scanIndex];
			if (subgroup) *subgroup = group;
			return 1;
		}

		if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT &&
			group->cl.items[group->scanIndex]->SubAllocated>0)
			for (i = 1; i<=group->cl.items[group->scanIndex]->SubAllocated; i++)
				if (IsHContactContact(dwItem)  && group->cl.items[group->scanIndex]->subcontacts[i-1].hContact == dwItem) {
					if (contact) *contact = &group->cl.items[group->scanIndex]->subcontacts[i-1];
					if (subgroup) *subgroup = group;
					return 1;
				}

		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			ClcGroup* save = group;
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			nowVisible &= group->expanded;
			continue;
		}
		group->scanIndex++;
	}

	if (isVisible) *isVisible = FALSE;
	if (contact)   *contact = NULL;
	if (subgroup)  *subgroup = NULL;
	return 0;
}
#define CacheArrSize 255
ClcGroup *CacheIndex[CacheArrSize] = {NULL};
boolean CacheIndexClear = TRUE;
void ClearRowByIndexCache()
{
	if ( !CacheIndexClear) {
		memset(CacheIndex,0,sizeof(CacheIndex));
		CacheIndexClear = TRUE;
	}
}

int GetRowByIndex(struct ClcData *dat,int testindex,struct ClcContact **contact,ClcGroup **subgroup)
{
	int index = 0,i;
	ClcGroup *group = &dat->list;

	if (testindex < 0) return -1;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL) break;
			group->scanIndex++;
			continue;
		}
		if ((index>0) && (index<CacheArrSize))
		{
			CacheIndex[index] = group;
			CacheIndexClear = FALSE;
		}

		if (testindex == index) {
			if (contact)  *contact = group->cl.items[group->scanIndex];
			if (subgroup) *subgroup = group;
			return index;
		}

		if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT)
			if (group->cl.items[group->scanIndex]->SubAllocated)
				if (group->cl.items[group->scanIndex]->SubExpanded) {
					for (i = 0;i<group->cl.items[group->scanIndex]->SubAllocated;i++) {
						if ((index>0) && (index<CacheArrSize)) {
							CacheIndex[index] = group;
							CacheIndexClear = FALSE;
						}
						index++;
						if (testindex == index) {
							if (contact) *contact = &group->cl.items[group->scanIndex]->subcontacts[i];
							if (subgroup) *subgroup = group;
							return index;
						}
					}
				}

		index++;
		if ((group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) && (group->cl.items[group->scanIndex]->flags & CONTACTF_STATUSMSG))
			index++;

		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && group->cl.items[group->scanIndex]->group->expanded) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	return -1;
}
