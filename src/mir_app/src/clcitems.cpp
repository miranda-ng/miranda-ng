/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"
#include "clc.h"

static ClcCacheEntry nullpce = {};

// routines for managing adding/removal of items in the list, including sorting

ClcContact* fnAddItemToGroup(ClcGroup *group, int iAboveItem)
{
	ClcContact* newItem = g_clistApi.pfnCreateClcContact();
	newItem->type = CLCIT_DIVIDER;
	newItem->flags = 0;
	newItem->szText[0] = '\0';
	memset(newItem->iExtraImage, 0xFF, sizeof(newItem->iExtraImage));
	group->cl.insert(newItem, iAboveItem);
	return newItem;
}

ClcGroup* fnAddGroup(HWND hwnd, ClcData *dat, const wchar_t *szName, uint32_t flags, int groupId, int calcTotalMembers)
{
	dat->bNeedsResort = true;
	if (!(GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_USEGROUPS))
		return &dat->list;

	ClcGroup *group = &dat->list;
	wchar_t *pNextField = NEWWSTR_ALLOCA(szName);
	do {
		wchar_t *pBackslash = wcschr(pNextField, '\\'), *pThisField = pNextField;
		if (pBackslash == nullptr) {
			pNextField = nullptr;
		}
		else {
			*pBackslash = 0;
			pNextField = pBackslash + 1;
		}
		
		int i, compareResult = 1;
		for (i = 0; i < group->cl.getCount(); i++) {
			ClcContact *cc = group->cl[i];
			if (cc->type == CLCIT_CONTACT)
				break;
			if (cc->type != CLCIT_GROUP)
				continue;
			compareResult = mir_wstrcmp(pThisField, cc->szText);
			if (compareResult == 0) {
				if (pNextField == nullptr && flags != (uint32_t)-1) {
					cc->groupId = (uint16_t)groupId;
					group = cc->group;
					group->expanded = (flags & GROUPF_EXPANDED) != 0;
					group->hideOffline = (flags & GROUPF_HIDEOFFLINE) != 0;
					group->groupId = groupId;
				}
				else group = cc->group;
				break;
			}
			if (pNextField == nullptr && cc->groupId == 0)
				break;
			if (!(dat->exStyle & CLS_EX_SORTGROUPSALPHA) && groupId && cc->groupId > groupId)
				break;
		}

		if (compareResult) { // not found
			if (groupId == 0)
				return nullptr;

			ClcContact *cc = g_clistApi.pfnAddItemToGroup(group, i);
			cc->type = CLCIT_GROUP;
			mir_wstrncpy(cc->szText, pThisField, _countof(cc->szText));
			cc->groupId = (uint16_t)(pNextField ? 0 : groupId);
			cc->group = new ClcGroup(10);
			cc->group->parent = group;
			group = cc->group;

			if (flags == (uint32_t)-1 || pNextField != nullptr) {
				group->expanded = 0;
				group->hideOffline = 0;
			}
			else {
				group->expanded = (flags & GROUPF_EXPANDED) != 0;
				group->hideOffline = (flags & GROUPF_HIDEOFFLINE) != 0;
			}
			group->groupId = pNextField ? 0 : groupId;
			group->totalMembers = 0;
			if (flags != (uint32_t)-1 && pNextField == nullptr && calcTotalMembers) {
				uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
				for (auto &hContact : Contacts()) {
					ClcCacheEntry *cache = Clist_GetCacheEntry(hContact);
					if (!mir_wstrcmp(cache->tszGroup, szName) && (style & CLS_SHOWHIDDEN || !cache->bIsHidden))
						group->totalMembers++;
				}
			}
		}
	} while (pNextField);
	return group;
}

void fnFreeContact(ClcContact* p)
{
	if (p->type == CLCIT_GROUP) {
		FreeGroup(p->group);
		delete p->group; p->group = nullptr;
	}
	
	mir_free(p);
}

void FreeGroup(ClcGroup *group)
{
	if (!group)
		return;

	for (auto &it : group->cl)
		g_clistApi.pfnFreeContact(it);

	group->cl.destroy();
}

static int iInfoItemUniqueHandle = 0;
ClcContact* fnAddInfoItemToGroup(ClcGroup *group, int flags, const wchar_t *pszText)
{
	int i = 0;

	if (flags & CLCIIF_BELOWCONTACTS)
		i = group->cl.getCount();
	else if (flags & CLCIIF_BELOWGROUPS) {
		for (; i < group->cl.getCount(); i++)
			if (group->cl[i]->type == CLCIT_CONTACT)
				break;
	}
	else
		for (; i < group->cl.getCount(); i++)
			if (group->cl[i]->type != CLCIT_INFO)
				break;

	ClcContact *cc = g_clistApi.pfnAddItemToGroup(group, i);
	iInfoItemUniqueHandle = LOWORD(iInfoItemUniqueHandle + 1);
	if (iInfoItemUniqueHandle == 0)
		++iInfoItemUniqueHandle;
	cc->type = CLCIT_INFO;
	cc->flags = (uint8_t)flags;
	cc->hContact = (MCONTACT)++iInfoItemUniqueHandle;
	mir_wstrncpy(cc->szText, pszText, _countof(cc->szText));
	return cc;
}

ClcContact* fnAddContactToGroup(ClcData *dat, ClcGroup *group, MCONTACT hContact)
{
	int i, index = -1;

	dat->bNeedsResort = true;
	for (i = group->cl.getCount() - 1; i >= 0; i--) {
		ClcContact *cc = group->cl[i];
		if (cc->hContact == hContact)
			return cc;

		if (index == -1)
			if (cc->type != CLCIT_INFO || !(cc->flags & CLCIIF_BELOWCONTACTS))
				index = i;
	}

	char *szProto = Proto_GetBaseAccountName(hContact);

	ClcCacheEntry *pce = Clist_GetCacheEntry(hContact);
	replaceStrW(pce->tszGroup, nullptr);

	ClcContact *cc = g_clistApi.pfnAddItemToGroup(group, index + 1);
	cc->type = CLCIT_CONTACT;
	cc->iImage = Clist_GetContactIcon(hContact);
	cc->hContact = hContact;
	cc->pce = pce;
	if (szProto != nullptr && !Clist_IsHiddenMode(dat, db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE)))
		cc->flags |= CONTACTF_ONLINE;
	uint16_t apparentMode = szProto != nullptr ? db_get_w(hContact, szProto, "ApparentMode", 0) : 0;
	if (apparentMode == ID_STATUS_OFFLINE)
		cc->flags |= CONTACTF_INVISTO;
	else if (apparentMode == ID_STATUS_ONLINE)
		cc->flags |= CONTACTF_VISTO;
	else if (apparentMode)
		cc->flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
	if (!Contact::OnList(hContact))
		cc->flags |= CONTACTF_NOTONLIST;
	uint32_t idleMode = szProto != nullptr ? db_get_dw(hContact, szProto, "IdleTS", 0) : 0;
	if (idleMode)
		cc->flags |= CONTACTF_IDLE;
	mir_wstrncpy(cc->szText, Clist_GetContactDisplayName(hContact), _countof(cc->szText));
	return cc;
}

void fnAddContactToTree(HWND hwnd, ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline)
{
	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
	uint16_t status = ID_STATUS_OFFLINE;
	char *szProto = Proto_GetBaseAccountName(hContact);

	dat->bNeedsResort = true;
	if (style & CLS_NOHIDEOFFLINE)
		checkHideOffline = 0;
	if (checkHideOffline)
		if (szProto != nullptr)
			status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);

	int i;
	uint32_t groupFlags;
	ClcGroup *group;
	ptrW tszGroup(Clist_GetGroup(hContact));
	if (tszGroup == nullptr)
		group = &dat->list;
	else {
		group = g_clistApi.pfnAddGroup(hwnd, dat, tszGroup, (uint32_t)-1, 0, 0);
		if (group == nullptr) {
			if (!(style & CLS_HIDEEMPTYGROUPS))
				return;

			if (checkHideOffline && Clist_IsHiddenMode(dat, status)) {
				for (i = 1;; i++) {
					wchar_t *szGroupName = Clist_GroupGetName(i, &groupFlags);
					if (szGroupName == nullptr)
						return;

					if (!mir_wstrcmp(szGroupName, tszGroup))
						break;
				}
				if (groupFlags & GROUPF_HIDEOFFLINE)
					return;
			}
			for (i = 1;; i++) {
				wchar_t *szGroupName = Clist_GroupGetName(i, &groupFlags);
				if (szGroupName == nullptr)
					return;

				if (!mir_wstrcmp(szGroupName, tszGroup))
					break;

				size_t len = mir_wstrlen(szGroupName);
				if (!wcsncmp(szGroupName, tszGroup, len) && tszGroup[len] == '\\')
					g_clistApi.pfnAddGroup(hwnd, dat, szGroupName, groupFlags, i, 1);
			}
			group = g_clistApi.pfnAddGroup(hwnd, dat, tszGroup, groupFlags, i, 1);
		}
	}

	if (checkHideOffline) {
		if (Clist_IsHiddenMode(dat, status) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
			if (updateTotalCount)
				group->totalMembers++;
			return;
		}
	}
	g_clistApi.pfnAddContactToGroup(dat, group, hContact);
	if (updateTotalCount)
		group->totalMembers++;
}

MIR_APP_DLL(ClcGroup*) Clist_RemoveItemFromGroup(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount)
{
	int iContact = group->cl.indexOf(contact);
	if (iContact == -1)
		return group;

	if (contact->type == CLCIT_CONTACT) {
		if (updateTotalCount)
			group->totalMembers--;

		g_clistApi.pfnInvalidateDisplayNameCacheEntry(contact->hContact);
	}

	g_clistApi.pfnFreeContact(group->cl[iContact]);
	group->cl.remove(iContact);

	if ((GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_HIDEEMPTYGROUPS) && group->cl.getCount() == 0 && group->parent != nullptr)
		for (auto &cc : group->parent->cl)
			if (cc->type == CLCIT_GROUP && cc->groupId == group->groupId)
				return Clist_RemoveItemFromGroup(hwnd, group->parent, cc, 0);

	return group;
}

MIR_APP_DLL(void) Clist_DeleteItemFromTree(HWND hwnd, MCONTACT hItem)
{
	ClcData *dat = (ClcData*)GetWindowLongPtr(hwnd, 0);
	dat->bNeedsResort = true;

	// if a contact is found in our contact list, remove it from its group and detach from cache
	ClcGroup *group;
	ClcContact *contact;
	if (Clist_FindItem(hwnd, dat, hItem, &contact, &group)) {
		Clist_RemoveItemFromGroup(hwnd, group, contact, 1);
		contact->pce = &nullpce;
		return;
	}

	// if we don't have this contact, simply try to update the number of contacts in a group
	if (!IsHContactContact(hItem))
		return;
		
	ptrW wszGroup(Clist_GetGroup(hItem));
	if (wszGroup == nullptr)
		return;

	// decrease member counts of all parent groups too
	group = &dat->list;
	int nameOffset = 0;
	for (int i = 0;; i++) {
		if (group->scanIndex == group->cl.getCount())
			break;

		ClcContact *cc = group->cl[i];
		if (cc->type == CLCIT_GROUP) {
			size_t len = mir_wstrlen(cc->szText);
			if (!wcsncmp(cc->szText, wszGroup.get() + nameOffset, len) && (wszGroup[nameOffset + len] == '\\' || wszGroup[nameOffset + len] == '\0')) {
				group->totalMembers--;
				if (wszGroup[nameOffset + len] == '\0')
					break;
			}
		}
	}
}

int fnGetContactHiddenStatus(MCONTACT hContact, char*, ClcData*)
{
	return Contact::IsHidden(hContact);
}

void fnRebuildEntireList(HWND hwnd, ClcData *dat)
{
	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);

	dat->list.expanded = 1;
	dat->list.hideOffline = db_get_b(0, "CLC", "HideOfflineRoot", 0) && (style & CLS_USEGROUPS);
	dat->list.cl.destroy();
	dat->list.totalMembers = 0;
	dat->selection = -1;

	for (int i = 1;; i++) {
		uint32_t groupFlags;
		wchar_t *szGroupName = Clist_GroupGetName(i, &groupFlags);
		if (szGroupName == nullptr)
			break;
		g_clistApi.pfnAddGroup(hwnd, dat, szGroupName, groupFlags, i, 0);
	}

	for (auto &hContact : Contacts()) {
		int nHiddenStatus = g_clistApi.pfnGetContactHiddenStatus(hContact, nullptr, dat);
		if (((style & CLS_SHOWHIDDEN) && nHiddenStatus != -1) || !nHiddenStatus) {
			ClcCacheEntry *pce = Clist_GetCacheEntry(hContact);
			if (pce->szProto == nullptr)
				continue;

			ClcGroup *group;
			ptrW tszGroupName(Clist_GetGroup(hContact));
			if (tszGroupName == nullptr)
				group = &dat->list;
			else {
				group = g_clistApi.pfnAddGroup(hwnd, dat, tszGroupName, (uint32_t)-1, 0, 0);
				if (group == nullptr && style & CLS_SHOWHIDDEN)
					group = &dat->list;
			}

			if (group != nullptr) {
				group->totalMembers++;

				if (dat->bFilterSearch && dat->szQuickSearch[0] != '\0') {
					wchar_t *name = Clist_GetContactDisplayName(hContact);
					wchar_t *lowered_name = CharLowerW(NEWWSTR_ALLOCA(name));
					wchar_t *lowered_search = CharLowerW(NEWWSTR_ALLOCA(dat->szQuickSearch));

					if (wcsstr(lowered_name, lowered_search))
						g_clistApi.pfnAddContactToGroup(dat, group, hContact);
				}
				else if (!(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					char *szProto = Proto_GetBaseAccountName(hContact);
					if (szProto == nullptr) {
						if (!Clist_IsHiddenMode(dat, ID_STATUS_OFFLINE) || g_clistApi.pfnIsVisibleContact(pce, group))
							g_clistApi.pfnAddContactToGroup(dat, group, hContact);
					}
					else if (!Clist_IsHiddenMode(dat, db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE)) || g_clistApi.pfnIsVisibleContact(pce, group))
						g_clistApi.pfnAddContactToGroup(dat, group, hContact);
				}
				else g_clistApi.pfnAddContactToGroup(dat, group, hContact);
			}
		}
	}

	if (style & CLS_HIDEEMPTYGROUPS) {
		ClcGroup *group = &dat->list;
		group->scanIndex = 0;
		for (;;) {
			if (group->scanIndex == group->cl.getCount()) {
				if ((group = group->parent) == nullptr)
					break;
				group->scanIndex++;
				continue;
			}

			ClcContact *cc = group->cl[group->scanIndex];
			if (cc->type == CLCIT_GROUP) {
				if (cc->group->cl.getCount() == 0) {
					group = Clist_RemoveItemFromGroup(hwnd, group, cc, 0);
				}
				else {
					group = cc->group;
					group->scanIndex = 0;
				}
				continue;
			}
			group->scanIndex++;
		}
	}

	g_clistApi.pfnSortCLC(hwnd, dat, 0);
	ExtraIcon_SetAll();
}

int fnGetGroupContentsCount(ClcGroup *group, int visibleOnly)
{
	int count = group->cl.getCount();
	ClcGroup *topgroup = group;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if (group == topgroup)
				break;
			group = group->parent;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP && (!visibleOnly || cc->group->expanded)) {
			group = cc->group;
			group->scanIndex = 0;
			count += group->cl.getCount();
			continue;
		}
		group->scanIndex++;
	}
	return count;
}

static int __cdecl GroupSortProc(const void* p1, const void* p2)
{
	ClcContact **contact1 = (ClcContact**)p1, **contact2 = (ClcContact**)p2;

	return mir_wstrcmpi(contact1[0]->szText, contact2[0]->szText);
}

static int __cdecl ContactSortProc(const void* p1, const void* p2)
{
	ClcContact **contact1 = (ClcContact**)p1, **contact2 = (ClcContact**)p2;

	int result = g_clistApi.pfnCompareContacts(contact1[0], contact2[0]);
	if (result)
		return result;
	//nothing to distinguish them, so make sure they stay in the same order
	return (int)((INT_PTR)contact2[0]->hContact - (INT_PTR)contact1[0]->hContact);
}

static void InsertionSort(ClcContact **pContactArray, int nArray, int(*CompareProc) (const void *, const void *))
{
	int i, j;
	ClcContact* testElement;

	for (i = 1; i < nArray; i++) {
		if (CompareProc(&pContactArray[i - 1], &pContactArray[i]) > 0) {
			testElement = pContactArray[i];
			for (j = i - 2; j >= 0; j--)
				if (CompareProc(&pContactArray[j], &testElement) <= 0)
					break;
			j++;
			memmove(&pContactArray[j + 1], &pContactArray[j], sizeof(void*) * (i - j));
			pContactArray[j] = testElement;
		}
	}
}

static void SortGroup(ClcData *dat, ClcGroup *group, int useInsertionSort)
{
	int i, sortCount;

	for (i = group->cl.getCount() - 1; i >= 0; i--) {
		if (group->cl[i]->type == CLCIT_DIVIDER) {
			mir_free(group->cl[i]);
			group->cl.remove(i);
		}
	}

	for (i = 0; i < group->cl.getCount(); i++)
		if (group->cl[i]->type != CLCIT_INFO)
			break;
	if (i > group->cl.getCount() - 2)
		return;
	if (group->cl[i]->type == CLCIT_GROUP) {
		if (dat->exStyle & CLS_EX_SORTGROUPSALPHA) {
			for (sortCount = 0; i + sortCount < group->cl.getCount(); sortCount++)
				if (group->cl[i + sortCount]->type != CLCIT_GROUP)
					break;
			qsort(group->cl.getArray() + i, sortCount, sizeof(void*), GroupSortProc);
			i = i + sortCount;
		}
		for (; i < group->cl.getCount(); i++)
			if (group->cl[i]->type == CLCIT_CONTACT)
				break;
		if (group->cl.getCount() - i < 2)
			return;
	}
	for (sortCount = 0; i + sortCount < group->cl.getCount(); sortCount++)
		if (group->cl[i + sortCount]->type != CLCIT_CONTACT)
			break;
	if (useInsertionSort)
		InsertionSort(group->cl.getArray() + i, sortCount, ContactSortProc);
	else
		qsort(group->cl.getArray() + i, sortCount, sizeof(void*), ContactSortProc);
	if (dat->exStyle & CLS_EX_DIVIDERONOFF) {
		int prevContactOnline = 0;
		for (i = 0; i < group->cl.getCount(); i++) {
			if (group->cl[i]->type != CLCIT_CONTACT)
				continue;
			if (group->cl[i]->flags & CONTACTF_ONLINE)
				prevContactOnline = 1;
			else {
				if (prevContactOnline) {
					ClcContact *cc = g_clistApi.pfnAddItemToGroup(group, i);
					cc->type = CLCIT_DIVIDER;
					mir_wstrcpy(cc->szText, TranslateT("Offline"));
				}
				break;
			}
		}
	}
}

void fnSortCLC(HWND hwnd, ClcData *dat, int useInsertionSort)
{
	ClcGroup *group = &dat->list;

	if (dat->bNeedsResort) {
		MCONTACT hSelItem;
		ClcContact *selcontact;
		if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) == -1)
			hSelItem = 0;
		else
			hSelItem = Clist_ContactToHItem(selcontact);
		group->scanIndex = 0;
		SortGroup(dat, group, useInsertionSort);
		for (;;) {
			if (group->scanIndex == group->cl.getCount()) {
				if ((group = group->parent) == nullptr)
					break;
				group->scanIndex++;
				continue;
			}

			ClcContact *cc = group->cl[group->scanIndex];
			if (cc->type == CLCIT_GROUP) {
				group = cc->group;
				group->scanIndex = 0;
				SortGroup(dat, group, useInsertionSort);
				continue;
			}
			group->scanIndex++;
		}

		if (hSelItem) {
			ClcGroup *selgroup;
			if (Clist_FindItem(hwnd, dat, hSelItem, &selcontact, &selgroup))
				dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
		}

		g_clistApi.pfnRecalcScrollBar(hwnd, dat);
	}
	dat->bNeedsResort = false;
	g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
}

struct SavedContactState_t
{
	MCONTACT hContact;
	uint16_t iExtraImage[EXTRA_ICON_COUNT];
	int checked;
};

struct SavedGroupState_t
{
	int groupId, expanded;
};

struct SavedInfoState_t
{
	int parentId;
	ClcContact contact;
};

MIR_APP_DLL(void) Clist_SaveStateAndRebuildList(HWND hwnd, ClcData *dat)
{
	Clist_HideInfoTip(dat);
	KillTimer(hwnd, TIMERID_INFOTIP);
	KillTimer(hwnd, TIMERID_RENAME);
	Clist_EndRename(dat, 1);

	dat->bLockScrollbar = true;

	OBJLIST<SavedContactState_t> saveContact(10, NumericKeySortT);
	OBJLIST<SavedGroupState_t> saveGroup(100, NumericKeySortT);
	OBJLIST<SavedInfoState_t> saveInfo(10, NumericKeySortT);

	dat->bNeedsResort = true;
	ClcGroup *group = &dat->list;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP) {
			group = cc->group;
			group->scanIndex = 0;

			SavedGroupState_t *p = new SavedGroupState_t;
			p->groupId = group->groupId;
			p->expanded = group->expanded;
			saveGroup.insert(p);
			continue;
		}
		else if (cc->type == CLCIT_CONTACT) {
			SavedContactState_t *p = new SavedContactState_t;
			p->hContact = cc->hContact;
			memcpy(p->iExtraImage, cc->iExtraImage, sizeof(p->iExtraImage));
			p->checked = cc->flags & CONTACTF_CHECKED;
			saveContact.insert(p);
		}
		else if (cc->type == CLCIT_INFO) {
			SavedInfoState_t *p = new SavedInfoState_t;
			p->parentId = (group->parent == nullptr) ? -1 : group->groupId;
			p->contact = *cc;
			saveInfo.insert(p);
		}
		group->scanIndex++;
	}

	FreeGroup(&dat->list);
	g_clistApi.pfnRebuildEntireList(hwnd, dat);

	group = &dat->list;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP) {
			group = cc->group;
			group->scanIndex = 0;

			SavedGroupState_t tmp, *p;
			tmp.groupId = group->groupId;
			if ((p = saveGroup.find(&tmp)) != nullptr)
				group->expanded = p->expanded;
			continue;
		}
		else if (cc->type == CLCIT_CONTACT) {
			SavedContactState_t tmp, *p;
			tmp.hContact = cc->hContact;
			if ((p = saveContact.find(&tmp)) != nullptr) {
				memcpy(cc->iExtraImage, p->iExtraImage, sizeof(p->iExtraImage));
				if (p->checked)
					cc->flags |= CONTACTF_CHECKED;
			}
		}

		group->scanIndex++;
	}

	for (auto &it : saveInfo) {
		if (it->parentId == -1)
			group = &dat->list;
		else {
			ClcContact *contact;
			if (!Clist_FindItem(hwnd, dat, it->parentId | HCONTACT_ISGROUP, &contact))
				continue;
			group = contact->group;
		}
	
		ClcContact *cc = g_clistApi.pfnAddInfoItemToGroup(group, it->contact.flags, L"");
		*cc = it->contact;
	}

	dat->bLockScrollbar = false;
	Clist_RecalculateGroupCheckboxes(dat);

	g_clistApi.pfnRecalcScrollBar(hwnd, dat);

	NMCLISTCONTROL nm;
	nm.hdr.code = CLN_LISTREBUILT;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
}
