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

/* the CLC uses 3 different ways to identify elements in its list, this file
contains routines to convert between them.

1) ClcContact/ClcGroup pair. Only ever used within the duration
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

int fnGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex)
{
	int count = 0;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}
		if (group == subgroup && contactIndex == group->scanIndex)
			return count;
		count++;

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP) {
			if (cc->group == subgroup && contactIndex == -1)
				return count - 1;
			if (cc->group->expanded) {
				group = cc->group;
				group->scanIndex = 0;
				continue;
			}
		}
		group->scanIndex++;
	}
	return -1;
}

ClcContact* fnFindItem(uint32_t dwItem, ClcContact *cc)
{
	if (IsHContactGroup(dwItem) && cc->type == CLCIT_GROUP && (dwItem & ~HCONTACT_ISGROUP) == cc->groupId)
		return cc;

	if (IsHContactContact(dwItem) && cc->type == CLCIT_CONTACT && cc->hContact == dwItem)
		return cc;
	
	if (IsHContactInfo(dwItem) && cc->type == CLCIT_INFO && cc->hContact == (dwItem & ~HCONTACT_ISINFO))
		return cc;

	return nullptr;
}

MIR_APP_DLL(bool) Clist_FindItem(HWND hwnd, ClcData *dat, uint32_t dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible)
{
	int index = 0;
	int nowVisible = 1;
	ClcGroup *group = &dat->list;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;

			nowVisible = 1;
			for (ClcGroup *tgroup = group; tgroup; tgroup = tgroup->parent)
				if (!group->expanded) {
					nowVisible = 0;
					break;
				}
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (nowVisible && cc->type != CLCIT_GROUP)
			index++;

		ClcContact *res = g_clistApi.pfnFindItem(dwItem, cc);
		if (res != nullptr) {
			if (isVisible) {
				if (!nowVisible)
					*isVisible = 0;
				else {
					int posY = g_clistApi.pfnGetRowTopY(dat, index+1);
					if (posY < dat->yScroll)
						*isVisible = 0;
					else {
						RECT clRect;
						GetClientRect(hwnd, &clRect);
						if (posY >= dat->yScroll + clRect.bottom)
							*isVisible = 0;
						else
							*isVisible = 1;
					}
				}
			}
			if (contact)
				*contact = res;
			if (subgroup)
				*subgroup = group;
			return true;
		}
		
		if (cc->type == CLCIT_GROUP) {
			group = cc->group;
			group->scanIndex = 0;
			nowVisible &= group->expanded;
			continue;
		}
		group->scanIndex++;
	}

	if (isVisible) *isVisible = FALSE;
	if (contact)   *contact = nullptr;
	if (subgroup)  *subgroup = nullptr;
	return false;
}

int fnGetRowByIndex(ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup)
{
	int index = 0;
	ClcGroup *group = &dat->list;

	if (testindex<0)
		return (-1);

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (testindex == index) {
			if (contact)
				*contact = cc;
			if (subgroup)
				*subgroup = group;
			return index;
		}
		index++;
		if (cc->type == CLCIT_GROUP && cc->group->expanded) {
			group = cc->group;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	return -1;
}

MIR_APP_DLL(uint32_t) Clist_ContactToHItem(ClcContact *cc)
{
	switch (cc->type) {
	case CLCIT_CONTACT:
		return cc->hContact;
	case CLCIT_GROUP:
		return cc->groupId | HCONTACT_ISGROUP;
	case CLCIT_INFO:
		return cc->hContact | HCONTACT_ISINFO;
	}
	return 0;
}

MIR_APP_DLL(HANDLE) Clist_ContactToItemHandle(ClcContact *cc, uint32_t *nmFlags)
{
	switch (cc->type) {
	case CLCIT_CONTACT:
		return (HANDLE)cc->hContact;
	case CLCIT_GROUP:
		if (nmFlags)
			*nmFlags |= CLNF_ISGROUP;
		return (HANDLE)cc->groupId;
	case CLCIT_INFO:
		if (nmFlags)
			*nmFlags |= CLNF_ISINFO;
		return (HANDLE)((UINT_PTR)cc->hContact | HCONTACT_ISINFO);
	}
	return nullptr;
}

MIR_APP_DLL(int) Clist_GetRealStatus(ClcContact *cc, int iDefaultValue)
{
	if (cc->pce) {
		PROTOACCOUNT *pa = Proto_GetAccount(cc->pce->szProto);
		if (pa)
			return pa->iRealStatus;
	}

	return iDefaultValue;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GetGeneralizedStatus(char **szProto)
{
	int status = ID_STATUS_OFFLINE;
	int statusOnlineness = 0;

	for (auto &pa : g_arAccounts) {
		if (!pa->IsVisible())
			continue;

		if (pa->iRealStatus == ID_STATUS_INVISIBLE)
			return ID_STATUS_INVISIBLE;

		int iStatusWeight;
		switch (pa->iRealStatus) {
		case ID_STATUS_FREECHAT:    iStatusWeight = 110; break;
		case ID_STATUS_ONLINE:      iStatusWeight = 100; break;
		case ID_STATUS_OCCUPIED:    iStatusWeight = 60;  break;
		case ID_STATUS_DND:         iStatusWeight = 40;  break;
		case ID_STATUS_AWAY:        iStatusWeight = 30;  break;
		case ID_STATUS_NA:          iStatusWeight = 10;  break;
		case ID_STATUS_INVISIBLE:   iStatusWeight = 5;   break;
		default:                    
			iStatusWeight = IsStatusConnecting(pa->iRealStatus) ? 120 : 0;
			break;
		}

		if (iStatusWeight > statusOnlineness) {
			if (szProto != nullptr)
				*szProto = pa->szModuleName;
			status = pa->iRealStatus;
			statusOnlineness = iStatusWeight;
		}
	}
	return status;
}
