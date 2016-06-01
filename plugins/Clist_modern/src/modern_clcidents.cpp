/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
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

int GetContactIndex(ClcGroup *group, ClcContact *contact)
{
	for (int i = 0; i < group->cl.getCount(); i++)
		if (group->cl[i]->hContact == contact->hContact)
			return i;
	
	return -1;
}

int cliGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex)
{
	int count = 0;
	int subcontactscount = 0;
	bool bMetaExpanding = db_get_b(NULL, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT) != 0;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == NULL)
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

int cliFindItem(HWND hwnd, ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible)
{
	return FindItem(hwnd, dat, dwItem, contact, subgroup, isVisible, false);
}

int FindItem(HWND hwnd, ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible, bool isIgnoreSubcontacts)
{
	int index = 0;
	int nowVisible = 1;

	ClcGroup *group = &dat->list;
	group->scanIndex = 0;
	group = &dat->list;

	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == NULL)
				break;

			nowVisible = 1;
			for (ClcGroup *tgroup = group; tgroup; tgroup = tgroup->parent)
				if (!tgroup->expanded) {
					nowVisible = 0;
					break;
				}

			group->scanIndex++;
			continue;
		}

		if (nowVisible)
			index++;

		ClcContact *cc = group->cl[group->scanIndex];
		if ((IsHContactGroup(dwItem) && cc->type == CLCIT_GROUP && (dwItem & ~HCONTACT_ISGROUP) == cc->groupId) ||
			(IsHContactContact(dwItem) && cc->type == CLCIT_CONTACT && cc->hContact == dwItem) ||
			(IsHContactInfo(dwItem) && cc->type == CLCIT_INFO && cc->hContact == (dwItem & ~HCONTACT_ISINFO))) {
			if (isVisible) {
				if (!nowVisible) *isVisible = 0;
				else {
					int posy = cliGetRowTopY(dat, index + 1);
					if (posy < dat->yScroll)
						*isVisible = 0;
					else {
						RECT clRect;
						GetClientRect(hwnd, &clRect);
						*isVisible = (posy < dat->yScroll + clRect.bottom);
					}
				}
			}
			if (contact) *contact = cc;
			if (subgroup) *subgroup = group;

			return 1;
		}

		if (!isIgnoreSubcontacts && IsHContactContact(dwItem) && cc->type == CLCIT_CONTACT && cc->iSubAllocated > 0) {
			for (int i = 0; i < cc->iSubAllocated; i++) {
				if (cc->subcontacts[i].hContact == dwItem) {
					if (contact) *contact = &cc->subcontacts[i];
					if (subgroup) *subgroup = group;
					return 1;
				}
			}
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
	if (contact)   *contact = NULL;
	if (subgroup)  *subgroup = NULL;
	return 0;
}

int cliGetRowByIndex(ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup)
{
	int index = 0, i;
	ClcGroup *group = &dat->list;

	if (testindex < 0) return (-1);

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == NULL)
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
