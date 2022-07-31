/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

void AddSubcontacts(ClcData *dat, ClcContact *cont, BOOL showOfflineHereGroup)
{
	cont->bSubExpanded = g_plugin.getByte(cont->hContact, "Expanded") && db_get_b(0, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT);
	int subcount = db_mc_getSubCount(cont->hContact);
	if (subcount <= 0) {
		cont->iSubNumber = 0;
		cont->subcontacts = nullptr;
		cont->iSubAllocated = 0;
		return;
	}

	cont->iSubNumber = 0;
	mir_free(cont->subcontacts);
	cont->subcontacts = (ClcContact *)mir_calloc(sizeof(ClcContact) * subcount);
	cont->iSubAllocated = subcount;
	int i = 0;
	int bHideOffline = Clist::HideOffline;
	for (int j = 0; j < subcount; j++) {
		MCONTACT hsub = db_mc_getSub(cont->hContact, j);
		if (!hsub)
			continue;

		ClcCacheEntry *pdnce = Clist_GetCacheEntry(hsub);
		if (pdnce->szProto == nullptr)
			continue;

		uint16_t wStatus = pdnce->getStatus();
		if (!showOfflineHereGroup && bHideOffline && !pdnce->m_bNoHiddenOffline && wStatus == ID_STATUS_OFFLINE)
			continue;

		ClcContact &p = cont->subcontacts[i];
		p.hContact = pdnce->hContact;
		p.pce = pdnce;

		p.avatar_pos = AVATAR_POS_DONT_HAVE;
		Cache_GetAvatar(dat, &p);

		p.iImage = Clist_GetContactIcon(pdnce->hContact);
		memset(p.iExtraImage, 0xFF, sizeof(p.iExtraImage));
		p.type = CLCIT_CONTACT;
		p.flags = 0;
		p.iSubNumber = i + 1;
		p.lastPaintCounter = 0;
		p.subcontacts = cont;
		p.bImageIsSpecial = false;
		Cache_GetTimezone(dat, (&p)->hContact);
		Cache_GetText(dat, &p);

		char *szProto = pdnce->szProto;
		if (szProto != nullptr && !Clist_IsHiddenMode(dat, wStatus))
			p.flags |= CONTACTF_ONLINE;
		int apparentMode = szProto != nullptr ? pdnce->ApparentMode : 0;
		if (apparentMode == ID_STATUS_OFFLINE)	p.flags |= CONTACTF_INVISTO;
		else if (apparentMode == ID_STATUS_ONLINE) p.flags |= CONTACTF_VISTO;
		else if (apparentMode) p.flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
		if (pdnce->NotOnList) p.flags |= CONTACTF_NOTONLIST;
		int idleMode = szProto != nullptr ? pdnce->IdleTS : 0;
		if (idleMode) p.flags |= CONTACTF_IDLE;
		i++;
	}

	cont->iSubAllocated = i;
	if (!i && cont->subcontacts != nullptr)
		mir_free_and_nil(cont->subcontacts);
}

void cli_FreeContact(ClcContact *p)
{
	if (p->iSubAllocated) {
		if (p->subcontacts && !p->iSubNumber) {
			for (int i = 0; i < p->iSubAllocated; i++) {
				p->subcontacts[i].ssText.DestroySmileyList();
				if (p->subcontacts[i].avatar_pos == AVATAR_POS_ANIMATED)
					AniAva_RemoveAvatar(p->subcontacts[i].hContact);
				p->subcontacts[i].avatar_pos = AVATAR_POS_DONT_HAVE;
			}
			mir_free_and_nil(p->subcontacts);
		}
		p->iSubAllocated = 0;
	}

	p->ssText.DestroySmileyList();
	if (p->avatar_pos == AVATAR_POS_ANIMATED)
		AniAva_RemoveAvatar(p->hContact);
	p->avatar_pos = AVATAR_POS_DONT_HAVE;
	corecli.pfnFreeContact(p);
}

static void _LoadDataToContact(ClcContact *cont, ClcCacheEntry *pdnce, ClcGroup *group, ClcData *dat, MCONTACT hContact)
{
	if (!cont)
		return;

	char *szProto = pdnce->szProto;

	cont->type = CLCIT_CONTACT;
	cont->pce = pdnce;
	cont->iSubAllocated = 0;
	cont->iSubNumber = 0;
	cont->subcontacts = nullptr;
	cont->szText[0] = 0;
	cont->lastPaintCounter = 0;
	cont->bImageIsSpecial = false;
	cont->hContact = hContact;

	if (szProto != nullptr && !Clist_IsHiddenMode(dat, pdnce->m_iStatus))
		cont->flags |= CONTACTF_ONLINE;

	uint16_t apparentMode = szProto != nullptr ? pdnce->ApparentMode : 0;
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

	if (pdnce->NotOnList)
		cont->flags |= CONTACTF_NOTONLIST;

	uint32_t idleMode = szProto != nullptr ? pdnce->IdleTS : 0;
	if (idleMode)
		cont->flags |= CONTACTF_IDLE;

	// Add subcontacts
	if (dat->IsMetaContactsEnabled && !mir_strcmp(szProto, META_PROTO))
		AddSubcontacts(dat, cont, CLCItems_IsShowOfflineGroup(group));

	cont->lastPaintCounter = 0;
	cont->avatar_pos = AVATAR_POS_DONT_HAVE;
	Cache_GetAvatar(dat, cont);
	Cache_GetText(dat, cont);
	Cache_GetTimezone(dat, cont->hContact);
	cont->iImage = Clist_GetContactIcon(hContact);
	cont->bContactRate = g_plugin.getByte(hContact, "Rate");
}

ClcContact* cli_AddContactToGroup(ClcData *dat, ClcGroup *group, MCONTACT hContact)
{
	ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
	if (pdnce->m_iStatus == ID_STATUS_OFFLINE && dat->bPlaceOfflineToRoot)
		group = &dat->list;

	ClcContact *cc = corecli.pfnAddContactToGroup(dat, group, hContact);
	_LoadDataToContact(cc, pdnce, group, dat, hContact);
	return cc;
}

void cli_AddContactToTree(HWND hwnd, ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline)
{
	ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
	if (dat->IsMetaContactsEnabled && pdnce->m_bIsSub)
		return;		//contact should not be added

	if (!dat->IsMetaContactsEnabled && !mir_strcmp(pdnce->szProto, META_PROTO))
		return;

	corecli.pfnAddContactToTree(hwnd, dat, hContact, updateTotalCount, checkHideOffline);

	ClcGroup *group;
	ClcContact *cont;
	if (Clist_FindItem(hwnd, dat, hContact, &cont, &group))
		_LoadDataToContact(cont, pdnce, group, dat, hContact);
}

bool CLCItems_IsShowOfflineGroup(ClcGroup *group)
{
	if (!group) return false;
	if (group->hideOffline) return false;

	uint32_t groupFlags = 0;
	Clist_GroupGetName(group->groupId, &groupFlags);
	return (groupFlags & GROUPF_SHOWOFFLINE) != 0;
}

MCONTACT SaveSelection(ClcData *dat)
{
	ClcContact *selcontact = nullptr;
	if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) == -1)
		return 0;

	return Clist_ContactToHItem(selcontact);
}

int RestoreSelection(ClcData *dat, MCONTACT hSelected)
{
	ClcGroup *selgroup = nullptr;
	ClcContact *selcontact = nullptr;
	if (!hSelected || !Clist_FindItem(dat->hWnd, dat, hSelected, &selcontact, &selgroup)) {
		dat->selection = -1;
		return dat->selection;
	}

	if (!selcontact->iSubNumber)
		dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
	else {
		dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact->subcontacts));
		if (dat->selection != -1)
			dat->selection += selcontact->iSubNumber;
	}
	return dat->selection;
}

int cliIsVisibleContact(ClcCacheEntry *pce, ClcGroup *group)
{
	if (pce->m_bNoHiddenOffline)
		return true;

	if (CLCItems_IsShowOfflineGroup(group))
		return true;
	
	return false;
}

void cliRebuildEntireList(HWND hwnd, ClcData *dat)
{
	KillTimer(hwnd, TIMERID_REBUILDAFTER);
	g_clistApi.bAutoRebuild = false;

	ImageArray_Clear(&dat->avatar_cache);
	RowHeights_Clear(dat);
	RowHeights_GetMaxRowHeight(dat, hwnd);

	MCONTACT hSelected = SaveSelection(dat);
	dat->selection = -1;
	dat->bNeedsResort = true;
	dat->HiLightMode = db_get_b(0, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT);
	dat->bPlaceOfflineToRoot = g_plugin.getByte("PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT) != 0;

	corecli.pfnRebuildEntireList(hwnd, dat);

	RestoreSelection(dat, hSelected);
}

void cli_SortCLC(HWND hwnd, ClcData *dat, int useInsertionSort)
{
	MCONTACT hSelected = SaveSelection(dat);
	corecli.pfnSortCLC(hwnd, dat, useInsertionSort);
	RestoreSelection(dat, hSelected);
}

/////////////////////////////////////////////////////////////////////////////////////////

int GetNewSelection(ClcGroup *group, int selection, int direction)
{
	if (selection < 0)
		return 0;

	int lastcount = 0, count = 0;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}

		if (count >= selection)
			return count;

		lastcount = count;
		count++;
		if (!direction && count > selection)
			return lastcount;

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP && (cc->group->expanded)) {
			group = cc->group;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	return lastcount;
}

/////////////////////////////////////////////////////////////////////////////////////////

ClcContact* cliCreateClcContact()
{
	ClcContact *contact = (ClcContact *)mir_calloc(sizeof(ClcContact));
	memset(contact->iExtraImage, 0xFF, sizeof(contact->iExtraImage));
	return contact;
}

ClcCacheEntry* cliCreateCacheItem(MCONTACT hContact)
{
	if (hContact == 0)
		return nullptr;

	ClcCacheEntry *pdnce = (ClcCacheEntry *)mir_calloc(sizeof(ClcCacheEntry));
	if (pdnce == nullptr)
		return nullptr;

	pdnce->hContact = hContact;
	pdnce->szProto = Proto_GetBaseAccountName(hContact);
	pdnce->bIsHidden = Contact::IsHidden(hContact);
	pdnce->m_bIsSub = db_mc_isSub(hContact) != 0;
	pdnce->m_bNoHiddenOffline = g_plugin.getByte(hContact, "noOffline");
	pdnce->IdleTS = db_get_dw(hContact, pdnce->szProto, "IdleTS", 0);
	pdnce->ApparentMode = db_get_w(hContact, pdnce->szProto, "ApparentMode", 0);
	pdnce->NotOnList = !Contact::OnList(hContact);
	pdnce->IsExpanded = g_plugin.getByte(hContact, "Expanded");
	pdnce->dwLastOnlineTime = db_get_dw(hContact, pdnce->szProto, "LastSeen", 0);
	pdnce->dwLastMsgTime = -1;
	return pdnce;
}

/////////////////////////////////////////////////////////////////////////////////////////

void cliInvalidateDisplayNameCacheEntry(MCONTACT hContact)
{
	if (hContact != INVALID_CONTACT_ID) {
		ClcCacheEntry *p = Clist_GetCacheEntry(hContact);
		if (p)
			p->m_iStatus = 0;
	}

	corecli.pfnInvalidateDisplayNameCacheEntry(hContact);
}

void cli_SetContactCheckboxes(ClcContact *cc, int checked)
{
	corecli.pfnSetContactCheckboxes(cc, checked);

	for (int i = 0; i < cc->iSubAllocated; i++)
		corecli.pfnSetContactCheckboxes(&cc->subcontacts[i], checked);
}

int cliGetGroupContentsCount(ClcGroup *group, int visibleOnly)
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
		if (cc->type == CLCIT_GROUP && (!(visibleOnly & 0x01) || cc->group->expanded)) {
			group = cc->group;
			group->scanIndex = 0;
			count += group->cl.getCount();
			continue;
		}
		if (cc->type == CLCIT_CONTACT && cc->subcontacts != nullptr && (cc->bSubExpanded || !visibleOnly))
			count += cc->iSubAllocated;

		group->scanIndex++;
	}
	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks the currently active view mode filter and returns true, if the contact should be hidden
// if no view mode is active, it returns the CList/Hidden setting
// also cares about sub contacts (if meta is active)

int CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szProto, ClcData *dat)
{
	int dbHidden = Contact::IsHidden(hContact); // default hidden state, always respect it.
	int filterResult = 1;
	int searchResult = 0;
	ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);

	// always hide subcontacts (but show them on embedded contact lists)
	if (dat != nullptr && dat->IsMetaContactsEnabled && db_mc_isSub(hContact))
		return -1; //subcontact

	if (pdnce && pdnce->m_bIsUnknown && dat != nullptr && !dat->bForceInDialog)
		return 1; //'Unknown contact'

	if (dat != nullptr && dat->bFilterSearch && pdnce && pdnce->tszName) {
		// search filtering
		wchar_t *lowered_name = CharLowerW(NEWWSTR_ALLOCA(pdnce->tszName));
		wchar_t *lowered_search = CharLowerW(NEWWSTR_ALLOCA(dat->szQuickSearch));
		searchResult = wcsstr(lowered_name, lowered_search) ? 0 : 1;
	}

	if (pdnce && g_CluiData.bFilterEffective && dat != nullptr) {
		if (szProto == nullptr)
			szProto = Proto_GetBaseAccountName(hContact);
		// check stickies first (priority), only if we really have stickies defined (CLVM_STICKY_CONTACTS is set).
		if (g_CluiData.bFilterEffective & CLVM_STICKY_CONTACTS) {
			if (uint32_t dwLocalMask = db_get_dw(hContact, CLVM_MODULE, g_CluiData.current_viewmode, 0)) {
				if (g_CluiData.bFilterEffective & CLVM_FILTER_STICKYSTATUS) {
					uint16_t wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
					return !((1 << (wStatus - ID_STATUS_OFFLINE)) & HIWORD(dwLocalMask)) | searchResult;
				}
				return 0 | searchResult;
			}
		}

		// check the proto, use it as a base filter result for all further checks
		if (g_CluiData.bFilterEffective & CLVM_FILTER_PROTOS) {
			char szTemp[64];
			mir_snprintf(szTemp, "%s|", szProto);
			if (db_mc_isMeta(hContact)) {
				for (int i = db_mc_getSubCount(hContact) - 1; i >= 0; i--) {
					mir_snprintf(szTemp, "%s|", Proto_GetBaseAccountName(db_mc_getSub(hContact, i)));
					if (strstr(g_CluiData.protoFilter, szTemp) != 0) {
						filterResult = 1;
						break;
					}
				}
			}
			else filterResult = strstr(g_CluiData.protoFilter, szTemp) != 0;
		}

		if (g_CluiData.bFilterEffective & CLVM_FILTER_GROUPS) {
			ptrW tszGroup(Clist_GetGroup(hContact));
			if (tszGroup != nullptr) {
				wchar_t szGroupMask[256];
				mir_snwprintf(szGroupMask, L"%s|", tszGroup.get());
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? (filterResult | (wcsstr(g_CluiData.groupFilter, szGroupMask) ? 1 : 0)) : (filterResult & (wcsstr(g_CluiData.groupFilter, szGroupMask) ? 1 : 0));
			}
			else if (g_CluiData.filterFlags & CLVM_INCLUDED_UNGROUPED)
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 1;
			else
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 0;
		}

		if (g_CluiData.bFilterEffective & CLVM_FILTER_STATUS) {
			uint16_t wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			filterResult = (g_CluiData.filterFlags & CLVM_GROUPSTATUS_OP) ? ((filterResult | ((1 << (wStatus - ID_STATUS_OFFLINE)) & g_CluiData.statusMaskFilter ? 1 : 0))) : (filterResult & ((1 << (wStatus - ID_STATUS_OFFLINE)) & g_CluiData.statusMaskFilter ? 1 : 0));
		}

		if (g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG) {
			if (pdnce->dwLastMsgTime != -1) {
				uint32_t now = g_CluiData.t_now;
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
