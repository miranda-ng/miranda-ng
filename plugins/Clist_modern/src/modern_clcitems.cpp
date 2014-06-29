/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_clist.h"
#include "m_metacontacts.h"
#include "hdr/modern_commonprototypes.h"

void AddSubcontacts(ClcData *dat, ClcContact *cont, BOOL showOfflineHereGroup)
{
	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(cont->hContact);
	cont->SubExpanded = (db_get_b(cont->hContact, "CList", "Expanded", 0) && (db_get_b(NULL, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT)));
	int subcount = db_mc_getSubCount(cont->hContact);
	if (subcount <= 0) {
		cont->isSubcontact = 0;
		cont->subcontacts = NULL;
		cont->SubAllocated = 0;
		return;
	}

	cont->isSubcontact = 0;
	mir_free(cont->subcontacts);
	cont->subcontacts = (ClcContact *)mir_calloc(sizeof(ClcContact)*subcount);
	cont->SubAllocated = subcount;
	int i = 0;
	int bHideOffline = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	int bMetaHideOfflineSub = db_get_b(NULL, "CLC", "MetaHideOfflineSub", SETTING_METAHIDEOFFLINESUB_DEFAULT);
	for (int j = 0; j < subcount; j++) {
		MCONTACT hsub = db_mc_getSub(cont->hContact, j);
		cacheEntry = pcli->pfnGetCacheEntry(hsub);
		WORD wStatus = pdnce___GetStatus(cacheEntry);

		if (showOfflineHereGroup || (!(bMetaHideOfflineSub && bHideOffline) || wStatus != ID_STATUS_OFFLINE)) {
			ClcContact& p = cont->subcontacts[i];
			p.hContact = cacheEntry->hContact;

			p.avatar_pos = AVATAR_POS_DONT_HAVE;
			Cache_GetAvatar(dat, &p);

			p.iImage = corecli.pfnGetContactIcon(cacheEntry->hContact);
			memset(p.iExtraImage, 0xFF, sizeof(p.iExtraImage));
			p.proto = cacheEntry->m_cache_cszProto;
			p.type = CLCIT_CONTACT;
			p.flags = 0;//CONTACTF_ONLINE;
			p.isSubcontact = i + 1;
			p.lastPaintCounter = 0;
			p.subcontacts = cont;
			p.image_is_special = FALSE;
			//p.status = cacheEntry->status;
			Cache_GetTimezone(dat, (&p)->hContact);
			Cache_GetText(dat, &p, 1);

			char *szProto = cacheEntry->m_cache_cszProto;
			if (szProto != NULL && !pcli->pfnIsHiddenMode(dat, wStatus))
				p.flags |= CONTACTF_ONLINE;
			int apparentMode = szProto != NULL ? cacheEntry->ApparentMode : 0;
			if (apparentMode == ID_STATUS_OFFLINE)	p.flags |= CONTACTF_INVISTO;
			else if (apparentMode == ID_STATUS_ONLINE) p.flags |= CONTACTF_VISTO;
			else if (apparentMode) p.flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
			if (cacheEntry->NotOnList) p.flags |= CONTACTF_NOTONLIST;
			int idleMode = szProto != NULL ? cacheEntry->IdleTS : 0;
			if (idleMode) p.flags |= CONTACTF_IDLE;
			i++;
		}
	}

	cont->SubAllocated = i;
	if (!i && cont->subcontacts != NULL)
		mir_free_and_nil(cont->subcontacts);
}

int cli_AddItemToGroup(ClcGroup *group, int iAboveItem)
{
	if (group == NULL)
		return 0;

	iAboveItem = corecli.pfnAddItemToGroup(group, iAboveItem);
	ClearRowByIndexCache();
	return iAboveItem;
}

ClcGroup* cli_AddGroup(HWND hwnd, ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers)
{
	ClearRowByIndexCache();
	if (!dat->force_in_dialog && !(GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN))
		if (!lstrcmp(_T("-@-HIDDEN-GROUP-@-"), szName)) { //group is hidden
			ClearRowByIndexCache();
			return NULL;
		}

	ClcGroup *result = corecli.pfnAddGroup(hwnd, dat, szName, flags, groupId, calcTotalMembers);
	ClearRowByIndexCache();
	return result;
}

void cli_FreeContact(ClcContact *p)
{
	if (p->SubAllocated) {
		if (p->subcontacts && !p->isSubcontact) {
			for (int i = 0; i < p->SubAllocated; i++) {
				p->subcontacts[i].ssText.DestroySmileyList();
				if (p->subcontacts[i].avatar_pos == AVATAR_POS_ANIMATED)
					AniAva_RemoveAvatar(p->subcontacts[i].hContact);
				p->subcontacts[i].avatar_pos = AVATAR_POS_DONT_HAVE;
			}
			mir_free_and_nil(p->subcontacts);
		}
	}

	p->ssText.DestroySmileyList();
	if (p->avatar_pos == AVATAR_POS_ANIMATED)
		AniAva_RemoveAvatar(p->hContact);
	p->avatar_pos = AVATAR_POS_DONT_HAVE;
	corecli.pfnFreeContact(p);
}

void cli_FreeGroup(ClcGroup* group)
{
	corecli.pfnFreeGroup(group);
	ClearRowByIndexCache();
}

int cli_AddInfoItemToGroup(ClcGroup *group, int flags, const TCHAR *pszText)
{
	int i = corecli.pfnAddInfoItemToGroup(group, flags, pszText);
	ClearRowByIndexCache();
	return i;
}

static void _LoadDataToContact(ClcContact *cont, ClcGroup *group, ClcData *dat, MCONTACT hContact)
{
	if (!cont)
		return;
	
	cont->type = CLCIT_CONTACT;
	cont->SubAllocated = 0;
	cont->isSubcontact = 0;
	cont->subcontacts = NULL;
	cont->szText[0] = 0;
	cont->lastPaintCounter = 0;
	cont->image_is_special = FALSE;
	cont->hContact = hContact;

	pcli->pfnInvalidateDisplayNameCacheEntry(hContact);

	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(hContact);
	char *szProto = cacheEntry->m_cache_cszProto;
	cont->proto = szProto;

	if (szProto != NULL && !pcli->pfnIsHiddenMode(dat, pdnce___GetStatus(cacheEntry)))
		cont->flags |= CONTACTF_ONLINE;

	WORD apparentMode = szProto != NULL ? cacheEntry->ApparentMode : 0;

	if (apparentMode)
		switch (apparentMode) {
		case ID_STATUS_OFFLINE:
			cont->flags |= CONTACTF_INVISTO;
			break;
		case ID_STATUS_ONLINE:
			cont->flags |= CONTACTF_VISTO;
			break;
		default:
			cont->flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
		}

	if (cacheEntry->NotOnList)
		cont->flags |= CONTACTF_NOTONLIST;

	DWORD idleMode = szProto != NULL ? cacheEntry->IdleTS : 0;
	if (idleMode)
		cont->flags |= CONTACTF_IDLE;

	// Add subcontacts
	if (szProto)
		if (dat->IsMetaContactsEnabled && mir_strcmp(cont->proto, META_PROTO) == 0)
			AddSubcontacts(dat, cont, CLCItems_IsShowOfflineGroup(group));

	cont->lastPaintCounter = 0;
	cont->avatar_pos = AVATAR_POS_DONT_HAVE;
	Cache_GetAvatar(dat, cont);
	Cache_GetText(dat, cont, 1);
	Cache_GetTimezone(dat, cont->hContact);
	cont->iImage = corecli.pfnGetContactIcon(hContact);
	cont->bContactRate = db_get_b(hContact, "CList", "Rate", 0);
}

static ClcContact* AddContactToGroup(ClcData *dat, ClcGroup *group, ClcCacheEntry *cacheEntry)
{
	MCONTACT hContact;
	int i;
	if (cacheEntry == NULL) return NULL;
	if (group == NULL) return NULL;
	if (dat == NULL) return NULL;
	hContact = cacheEntry->hContact;
	dat->needsResort = 1;
	for (i = group->cl.count - 1; i >= 0; i--)
		if (group->cl.items[i]->type != CLCIT_INFO || !(group->cl.items[i]->flags&CLCIIF_BELOWCONTACTS)) break;
	i = cli_AddItemToGroup(group, i + 1);

	_LoadDataToContact(group->cl.items[i], group, dat, hContact);
	cacheEntry = pcli->pfnGetCacheEntry(hContact);
	ClearRowByIndexCache();
	return group->cl.items[i];
}

void* AddTempGroup(HWND hwnd, ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers)
{
	int i = 0;
	int f = 0;
	DWORD groupFlags;

	if (wildcmp(_T2A(szName), "-@-HIDDEN-GROUP-@-"))
		return NULL;

	for (i = 1;; i++) {
		TCHAR *szGroupName = pcli->pfnGetGroupName(i, &groupFlags);
		if (szGroupName == NULL) break;
		if (!mir_tstrcmpi(szGroupName, szName)) f = 1;
	}
	if (!f) {
		char buf[20];
		TCHAR b2[255];
		void * res = NULL;
		mir_snprintf(buf, SIZEOF(buf), "%d", (i - 1));
		mir_sntprintf(b2, SIZEOF(b2), _T("#%s"), szName);
		b2[0] = 1 | GROUPF_EXPANDED;
		db_set_ws(NULL, "CListGroups", buf, b2);
		pcli->pfnGetGroupName(i, &groupFlags);
		res = cli_AddGroup(hwnd, dat, szName, groupFlags, i, 0);
		return res;
	}
	return NULL;
}

void cli_AddContactToTree(HWND hwnd, ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline)
{
	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(hContact);
	if (dat->IsMetaContactsEnabled && cacheEntry && cacheEntry->m_bIsSub)
		return;		//contact should not be added

	if (!dat->IsMetaContactsEnabled && cacheEntry && !mir_strcmp(cacheEntry->m_cache_cszProto, META_PROTO))
		return;

	corecli.pfnAddContactToTree(hwnd, dat, hContact, updateTotalCount, checkHideOffline);

	ClcGroup *group;
	ClcContact *cont;
	if (FindItem(hwnd, dat, hContact, &cont, &group, NULL, FALSE))
		_LoadDataToContact(cont, group, dat, hContact);
	return;
}

void cli_DeleteItemFromTree(HWND hwnd, MCONTACT hItem)
{
	ClcData *dat = (ClcData *)GetWindowLongPtr(hwnd, 0);
	ClearRowByIndexCache();
	corecli.pfnDeleteItemFromTree(hwnd, hItem);

	//check here contacts are not resorting
	if (hwnd == pcli->hwndContactTree)
		pcli->pfnFreeCacheItem(pcli->pfnGetCacheEntry(hItem));
	dat->needsResort = 1;
	ClearRowByIndexCache();
}


__inline BOOL CLCItems_IsShowOfflineGroup(ClcGroup* group)
{
	DWORD groupFlags = 0;
	if (!group) return FALSE;
	if (group->hideOffline) return FALSE;
	pcli->pfnGetGroupName(group->groupId, &groupFlags);
	return (groupFlags&GROUPF_SHOWOFFLINE) != 0;
}

MCONTACT SaveSelection(ClcData *dat)
{
	ClcContact *selcontact = NULL;
	if (pcli->pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) == -1)
		return NULL;

	return (MCONTACT)pcli->pfnContactToHItem(selcontact);
}

int RestoreSelection(ClcData *dat, MCONTACT hSelected)
{
	ClcContact *selcontact = NULL;
	ClcGroup *selgroup = NULL;

	if (!hSelected || !pcli->pfnFindItem(dat->hWnd, dat, hSelected, &selcontact, &selgroup, NULL)) {
		dat->selection = -1;
		return dat->selection;
	}

	if (!selcontact->isSubcontact)
		dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, selgroup, List_IndexOf((SortedList*)&selgroup->cl, selcontact));
	else {
		dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, selgroup, List_IndexOf((SortedList*)&selgroup->cl, selcontact->subcontacts));

		if (dat->selection != -1)
			dat->selection += selcontact->isSubcontact;
	}
	return dat->selection;

}

void cliRebuildEntireList(HWND hwnd, ClcData *dat)
{
	DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
	ClcContact *cont;
	ClcGroup *group;
	static int rebuildCounter = 0;

	BOOL PlaceOfflineToRoot = db_get_b(NULL, "CList", "PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT);
	KillTimer(hwnd, TIMERID_REBUILDAFTER);

	ClearRowByIndexCache();
	ImageArray_Clear(&dat->avatar_cache);
	RowHeights_Clear(dat);
	RowHeights_GetMaxRowHeight(dat, hwnd);
	TRACEVAR("Rebuild Entire List %d times\n", ++rebuildCounter);

	dat->list.expanded = 1;
	dat->list.hideOffline = db_get_b(NULL, "CLC", "HideOfflineRoot", SETTING_HIDEOFFLINEATROOT_DEFAULT) && style&CLS_USEGROUPS;
	dat->list.cl.count = dat->list.cl.limit = 0;
	dat->list.cl.increment = 50;
	dat->needsResort = 1;

	MCONTACT hSelected = SaveSelection(dat);
	dat->selection = -1;
	dat->HiLightMode = db_get_b(NULL, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT);
	{
		for (int i = 1;; i++) {
			DWORD groupFlags;
			TCHAR *szGroupName = pcli->pfnGetGroupName(i, &groupFlags); //UNICODE
			if (szGroupName == NULL)
				break;
			cli_AddGroup(hwnd, dat, szGroupName, groupFlags, i, 0);
		}
	}

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ClcCacheEntry *cacheEntry = NULL;
		cont = NULL;
		cacheEntry = pcli->pfnGetCacheEntry(hContact);

		int nHiddenStatus = CLVM_GetContactHiddenStatus(hContact, NULL, dat);
		if ((style & CLS_SHOWHIDDEN && nHiddenStatus != -1) || !nHiddenStatus) {
			if (lstrlen(cacheEntry->tszGroup) == 0)
				group = &dat->list;
			else
				group = cli_AddGroup(hwnd, dat, cacheEntry->tszGroup, (DWORD)-1, 0, 0);

			if (group != NULL) {
				WORD wStatus = pdnce___GetStatus(cacheEntry);
				if (wStatus == ID_STATUS_OFFLINE)
					if (PlaceOfflineToRoot)
						group = &dat->list;
				group->totalMembers++;

				if (!(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					if (cacheEntry->m_cache_cszProto == NULL) {
						if (!pcli->pfnIsHiddenMode(dat, ID_STATUS_OFFLINE) || cacheEntry->m_cache_nNoHiddenOffline || CLCItems_IsShowOfflineGroup(group))
							cont = AddContactToGroup(dat, group, cacheEntry);
					}
					else if (!pcli->pfnIsHiddenMode(dat, wStatus) || cacheEntry->m_cache_nNoHiddenOffline || CLCItems_IsShowOfflineGroup(group))
						cont = AddContactToGroup(dat, group, cacheEntry);
				}
				else cont = AddContactToGroup(dat, group, cacheEntry);
			}
		}
		if (cont) {
			cont->SubAllocated = 0;
			if (cont->proto && dat->IsMetaContactsEnabled  && strcmp(cont->proto, META_PROTO) == 0)
				AddSubcontacts(dat, cont, CLCItems_IsShowOfflineGroup(group));
		}
	}

	if (style & CLS_HIDEEMPTYGROUPS) {
		group = &dat->list;
		group->scanIndex = 0;
		for (;;) {
			if (group->scanIndex == group->cl.count) {
				group = group->parent;
				if (group == NULL) break;
			}
			else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
				if (group->cl.items[group->scanIndex]->group->cl.count == 0) {
					group = pcli->pfnRemoveItemFromGroup(hwnd, group, group->cl.items[group->scanIndex], 0);
				}
				else {
					group = group->cl.items[group->scanIndex]->group;
					group->scanIndex = 0;
				}
				continue;
			}
			group->scanIndex++;
		}
	}

	pcli->pfnSortCLC(hwnd, dat, 0);

	RestoreSelection(dat, hSelected);
}

void cli_SortCLC(HWND hwnd, ClcData *dat, int useInsertionSort)
{
	MCONTACT hSelected = SaveSelection(dat);
	corecli.pfnSortCLC(hwnd, dat, useInsertionSort);
	RestoreSelection(dat, hSelected);
}

int GetNewSelection(ClcGroup *group, int selection, int direction)
{
	int lastcount = 0, count = 0;//group->cl.count;
	if (selection < 0) {
		return 0;
	}
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL) break;
			group->scanIndex++;
			continue;
		}
		if (count >= selection) return count;
		lastcount = count;
		count++;
		if (!direction) {
			if (count > selection) return lastcount;
		}
		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && (group->cl.items[group->scanIndex]->group->expanded)) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	return lastcount;
}

struct SavedContactState_t {
	MCONTACT hContact;
	WORD iExtraImage[EXTRA_ICON_COUNT];
	int checked;
};

struct SavedGroupState_t {
	int groupId, expanded;
};

struct SavedInfoState_t {
	int parentId;
	ClcContact contact;
};

BOOL LOCK_RECALC_SCROLLBAR = FALSE;
void cli_SaveStateAndRebuildList(HWND hwnd, ClcData *dat)
{
	LOCK_RECALC_SCROLLBAR = TRUE;

	NMCLISTCONTROL nm;
	int i, j;
	OBJLIST<SavedGroupState_t> savedGroup(4);
	OBJLIST<SavedContactState_t> savedContact(4);
	OBJLIST<SavedInfoState_t> savedInfo(4);

	ClcGroup *group;
	ClcContact *contact;

	pcli->pfnHideInfoTip(hwnd, dat);
	KillTimer(hwnd, TIMERID_INFOTIP);
	KillTimer(hwnd, TIMERID_RENAME);
	pcli->pfnEndRename(hwnd, dat, 1);

	dat->needsResort = 1;
	group = &dat->list;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
				break;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;

			SavedGroupState_t* p = new SavedGroupState_t;
			p->groupId = group->groupId;
			p->expanded = group->expanded;
			savedGroup.insert(p);
			continue;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) {
			SavedContactState_t* p = new SavedContactState_t;
			p->hContact = group->cl.items[group->scanIndex]->hContact;
			memcpy(p->iExtraImage, group->cl.items[group->scanIndex]->iExtraImage, sizeof(p->iExtraImage));
			p->checked = group->cl.items[group->scanIndex]->flags & CONTACTF_CHECKED;
			savedContact.insert(p);
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_INFO) {
			SavedInfoState_t *p = new SavedInfoState_t;
			memset(p, 0, sizeof(SavedInfoState_t));
			if (group->parent == NULL)
				p->parentId = -1;
			else
				p->parentId = group->groupId;
			p->contact = *group->cl.items[group->scanIndex];
			savedInfo.insert(p);
		}
		group->scanIndex++;
	}

	pcli->pfnFreeGroup(&dat->list);
	pcli->pfnRebuildEntireList(hwnd, dat);

	group = &dat->list;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
				break;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			for (i = 0; i < savedGroup.getCount(); i++)
				if (savedGroup[i].groupId == group->groupId) {
				group->expanded = savedGroup[i].expanded;
				break;
				}
			continue;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) {
			for (i = 0; i < savedContact.getCount(); i++)
				if (savedContact[i].hContact == group->cl.items[group->scanIndex]->hContact) {
				memcpy(group->cl.items[group->scanIndex]->iExtraImage, savedContact[i].iExtraImage, sizeof(contact->iExtraImage));
				if (savedContact[i].checked)
					group->cl.items[group->scanIndex]->flags |= CONTACTF_CHECKED;
				break;
				}
		}
		group->scanIndex++;
	}

	for (i = 0; i < savedInfo.getCount(); i++) {
		if (savedInfo[i].parentId == -1)
			group = &dat->list;
		else {
			if (!pcli->pfnFindItem(hwnd, dat, savedInfo[i].parentId | HCONTACT_ISGROUP, &contact, NULL, NULL))
				continue;
			group = contact->group;
		}
		j = pcli->pfnAddInfoItemToGroup(group, savedInfo[i].contact.flags, _T(""));
		*group->cl.items[j] = savedInfo[i].contact;
	}

	LOCK_RECALC_SCROLLBAR = FALSE;
	pcli->pfnRecalculateGroupCheckboxes(hwnd, dat);

	pcli->pfnRecalcScrollBar(hwnd, dat);
	nm.hdr.code = CLN_LISTREBUILT;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
}


WORD pdnce___GetStatus(ClcCacheEntry *pdnce)
{
	return (!pdnce) ? ID_STATUS_OFFLINE : pdnce->m_cache_nStatus;
}

ClcContact* cliCreateClcContact()
{
	ClcContact* contact = (ClcContact*)mir_calloc(sizeof(ClcContact));
	memset(contact->iExtraImage, 0xFF, sizeof(contact->iExtraImage));
	return contact;
}

ClcCacheEntry* cliCreateCacheItem(MCONTACT hContact)
{
	ClcCacheEntry *p = (ClcCacheEntry *)mir_calloc(sizeof(ClcCacheEntry));
	if (p == NULL)
		return NULL;

	p->hContact = hContact;
	InvalidateDNCEbyPointer(hContact, p, 0);
	p->szSecondLineText = NULL;
	p->szThirdLineText = NULL;
	p->ssSecondLine.plText = NULL;
	p->ssThirdLine.plText = NULL;
	return p;
}

void cliInvalidateDisplayNameCacheEntry(MCONTACT hContact)
{
	if (hContact == INVALID_CONTACT_ID)
		corecli.pfnInvalidateDisplayNameCacheEntry(INVALID_CONTACT_ID);
	else {
		ClcCacheEntry *p = pcli->pfnGetCacheEntry(hContact);
		if (p)
			InvalidateDNCEbyPointer(hContact, p, 0);
	}
}

char* cli_GetGroupCountsText(ClcData *dat, ClcContact *contact)
{
	return corecli.pfnGetGroupCountsText(dat, contact);
}

int cliGetGroupContentsCount(ClcGroup *group, int visibleOnly)
{
	int count = group->cl.count;
	ClcGroup *topgroup = group;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			if (group == topgroup)
				break;
			group = group->parent;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && (!(visibleOnly & 0x01) || group->cl.items[group->scanIndex]->group->expanded)) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			count += group->cl.count;
			continue;
		}
		else if ((group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) &&
			(group->cl.items[group->scanIndex]->subcontacts != NULL) &&
			((group->cl.items[group->scanIndex]->SubExpanded || (!visibleOnly)))) {
			count += group->cl.items[group->scanIndex]->SubAllocated;
		}
		group->scanIndex++;
	}
	return count;
}

/*
* checks the currently active view mode filter and returns true, if the contact should be hidden
* if no view mode is active, it returns the CList/Hidden setting
* also cares about sub contacts (if meta is active)
*/

int __fastcall CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szProto, ClcData *dat)
{
	int dbHidden = db_get_b(hContact, "CList", "Hidden", 0);		// default hidden state, always respect it.
	int filterResult = 1;
	int searchResult = 0;
	DBVARIANT dbv = { 0 };
	char szTemp[64];
	TCHAR szGroupMask[256];
	DWORD dwLocalMask;
	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(hContact);
	BOOL fEmbedded = dat->force_in_dialog;
	// always hide subcontacts (but show them on embedded contact lists)

	if (dat != NULL && dat->IsMetaContactsEnabled && db_mc_isSub(hContact))
		return -1; //subcontact
	if (pdnce && pdnce->isUnknown && !fEmbedded)
		return 1; //'Unknown Contact'
	if (dat->filterSearch && dat->szQuickSearch && pdnce->tszName) {
		// search filtering
		TCHAR *lowered_name = CharLowerW(NEWTSTR_ALLOCA(pdnce->tszName));
		TCHAR *lowered_search = CharLowerW(NEWTSTR_ALLOCA(dat->szQuickSearch));
		searchResult = _tcsstr(lowered_name, lowered_search) ? 0 : 1;
	}
	if (pdnce && g_CluiData.bFilterEffective && !fEmbedded) {
		if (szProto == NULL)
			szProto = GetContactProto(hContact);
		// check stickies first (priority), only if we really have stickies defined (CLVM_STICKY_CONTACTS is set).
		if (g_CluiData.bFilterEffective & CLVM_STICKY_CONTACTS) {
			if ((dwLocalMask = db_get_dw(hContact, CLVM_MODULE, g_CluiData.current_viewmode, 0)) != 0) {
				if (g_CluiData.bFilterEffective & CLVM_FILTER_STICKYSTATUS) {
					WORD wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
					return !((1 << (wStatus - ID_STATUS_OFFLINE)) & HIWORD(dwLocalMask)) | searchResult;
				}
				return 0 | searchResult;
			}
		}
		// check the proto, use it as a base filter result for all further checks
		if (g_CluiData.bFilterEffective & CLVM_FILTER_PROTOS) {
			mir_snprintf(szTemp, SIZEOF(szTemp), "%s|", szProto);
			filterResult = strstr(g_CluiData.protoFilter, szTemp) ? 1 : 0;
		}
		if (g_CluiData.bFilterEffective & CLVM_FILTER_GROUPS) {
			if (!db_get_ts(hContact, "CList", "Group", &dbv)) {
				mir_sntprintf(szGroupMask, SIZEOF(szGroupMask), _T("%s|"), &dbv.ptszVal[0]);
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? (filterResult | (_tcsstr(g_CluiData.groupFilter, szGroupMask) ? 1 : 0)) : (filterResult & (_tcsstr(g_CluiData.groupFilter, szGroupMask) ? 1 : 0));
				mir_free(dbv.ptszVal);
			}
			else if (g_CluiData.filterFlags & CLVM_INCLUDED_UNGROUPED)
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 1;
			else
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 0;
		}
		if (g_CluiData.bFilterEffective & CLVM_FILTER_STATUS) {
			WORD wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			filterResult = (g_CluiData.filterFlags & CLVM_GROUPSTATUS_OP) ? ((filterResult | ((1 << (wStatus - ID_STATUS_OFFLINE)) & g_CluiData.statusMaskFilter ? 1 : 0))) : (filterResult & ((1 << (wStatus - ID_STATUS_OFFLINE)) & g_CluiData.statusMaskFilter ? 1 : 0));
		}
		if (g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG) {
			if (pdnce && pdnce->dwLastMsgTime != -1) {
				DWORD now = g_CluiData.t_now;
				now -= g_CluiData.lastMsgFilter;
				if (g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG_OLDERTHAN)
					filterResult = filterResult & (pdnce->dwLastMsgTime < now);
				else if (g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG_NEWERTHAN)
					filterResult = filterResult & (pdnce->dwLastMsgTime > now);
			}
		}
		return (dbHidden | !filterResult | searchResult);
	}

	return dbHidden | searchResult;
}
