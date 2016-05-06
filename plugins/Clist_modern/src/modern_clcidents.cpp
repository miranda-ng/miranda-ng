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
	for (int i = 0; i < group->cl.count; i++)
		if (group->cl.items[i]->hContact == contact->hContact)
			return i;
	
	return -1;
}

int cliGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex)
{
	int count = 0;
	int subcontactscount = 0;
	BYTE k = db_get_b(NULL, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT);
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL) break;
			group->scanIndex++;
			continue;
		}
		if (group == subgroup && contactIndex - subcontactscount == group->scanIndex) return count;
		count++;

		ClcContact *c = group->cl.items[group->scanIndex];
		if (c->type == CLCIT_GROUP) {
			if (c->group == subgroup && contactIndex == -1)
				return count - 1;

			if (c->group->expanded) {
				group = c->group;
				group->scanIndex = 0;
				subcontactscount = 0;
				continue;
			}
		}
		if (group == subgroup) {
			if (c->type == CLCIT_CONTACT && c->SubAllocated) {
				int rows = (c->SubAllocated*c->SubExpanded*k);
				if (group->scanIndex + rows >= contactIndex)
					return count + (contactIndex - group->scanIndex) - 1;
			}
		}
		if (c->type == CLCIT_CONTACT) {
			count += (c->SubAllocated * c->SubExpanded * k);
			subcontactscount += (c->SubAllocated * c->SubExpanded * k);
		}
		group->scanIndex++;
	}
	return -1;
}

int cliFindItem(HWND hwnd, ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible)
{
	return FindItem(hwnd, dat, dwItem, contact, subgroup, isVisible, FALSE);
}

int FindItem(HWND hwnd, ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible, BOOL isIgnoreSubcontacts)
{
	int index = 0;
	int nowVisible = 1;

	ClcGroup *group = &dat->list;
	group->scanIndex = 0;
	group = &dat->list;

	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
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

		ClcContact *c = group->cl.items[group->scanIndex];
		if ((IsHContactGroup(dwItem) && c->type == CLCIT_GROUP && (dwItem & ~HCONTACT_ISGROUP) == c->groupId) ||
			(IsHContactContact(dwItem) && c->type == CLCIT_CONTACT && c->hContact == dwItem) ||
			(IsHContactInfo(dwItem) && c->type == CLCIT_INFO && c->hContact == (dwItem & ~HCONTACT_ISINFO))) {
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
			if (contact) *contact = c;
			if (subgroup) *subgroup = group;

			return 1;
		}

		if (!isIgnoreSubcontacts && IsHContactContact(dwItem) && c->type == CLCIT_CONTACT && c->SubAllocated > 0) {
			for (int i = 0; i < c->SubAllocated; i++) {
				if (c->subcontacts[i].hContact == dwItem) {
					if (contact) *contact = &c->subcontacts[i];
					if (subgroup) *subgroup = group;
					return 1;
				}
			}
		}

		if (c->type == CLCIT_GROUP) {
			group = c->group;
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
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL) break;
			group->scanIndex++;
			continue;
		}

		ClcContact *c = group->cl.items[group->scanIndex];
		if (testindex == index) {
			if (contact) *contact = c;
			if (subgroup) *subgroup = group;
			return index;
		}

		if (c->type == CLCIT_CONTACT)
			if (c->SubAllocated)
				if (c->SubExpanded && dat->expandMeta) {
					for (i = 0; i < c->SubAllocated; i++) {
						index++;
						if (testindex == index) {
							if (contact) {
								*contact = &c->subcontacts[i];
								(*contact)->subcontacts = c;
							}

							if (subgroup) *subgroup = group;
							return index;
						}
					}
				}

		index++;
		if (c->type == CLCIT_GROUP && c->group->expanded) {
			group = c->group;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	return -1;
}
