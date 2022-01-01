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

int cliGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex)
{
	int count = 0;
	int subcontactscount = 0;
	bool bMetaExpanding = db_get_b(0, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT) != 0;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}
		if (group == subgroup && contactIndex - subcontactscount == group->scanIndex) return count;
		count++;

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP) {
			if (cc->group == subgroup && contactIndex == -1)
				return count - 1;

			if (cc->group->expanded) {
				group = cc->group;
				group->scanIndex = 0;
				subcontactscount = 0;
				continue;
			}
		}
		int iRows = (bMetaExpanding && cc->bSubExpanded) ? cc->iSubAllocated : 0;
		if (group == subgroup) {
			if (cc->type == CLCIT_CONTACT && cc->iSubAllocated) {
				if (group->scanIndex + iRows >= contactIndex)
					return count + (contactIndex - group->scanIndex) - 1;
			}
		}
		if (cc->type == CLCIT_CONTACT) {
			count += iRows;
			subcontactscount += iRows;
		}
		group->scanIndex++;
	}
	return -1;
}

ClcContact* cliFindItem(uint32_t dwItem, ClcContact *cc)
{
	if (corecli.pfnFindItem(dwItem, cc))
		return cc;

	if (IsHContactContact(dwItem) && cc->type == CLCIT_CONTACT && cc->iSubAllocated > 0)
		for (int i = 0; i < cc->iSubAllocated; i++)
			if (cc->subcontacts[i].hContact == dwItem)
				return &cc->subcontacts[i];

	return nullptr;
}

int cliGetRowByIndex(ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup)
{
	int index = 0, i;
	ClcGroup *group = &dat->list;

	if (testindex < 0) return (-1);

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
			if (contact) *contact = cc;
			if (subgroup) *subgroup = group;
			return index;
		}

		if (cc->type == CLCIT_CONTACT)
			if (cc->iSubAllocated)
				if (cc->bSubExpanded && dat->bMetaExpanding) {
					for (i = 0; i < cc->iSubAllocated; i++) {
						index++;
						if (testindex == index) {
							if (contact) {
								*contact = &cc->subcontacts[i];
								(*contact)->subcontacts = cc;
							}

							if (subgroup) *subgroup = group;
							return index;
						}
					}
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
