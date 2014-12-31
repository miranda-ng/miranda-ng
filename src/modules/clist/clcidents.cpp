/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
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

		ClcContact *cc = group->cl.items[group->scanIndex];
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

int fnFindItem(HWND hwnd, struct ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible)
{
	int index = 0;
	int nowVisible = 1;
	ClcGroup *group = &dat->list;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			ClcGroup *tgroup;
			group = group->parent;
			if (group == NULL)
				break;
			nowVisible = 1;
			for (tgroup = group; tgroup; tgroup = tgroup->parent)
				if (!group->expanded) {
					nowVisible = 0;
					break;
				}
			group->scanIndex++;
			continue;
		}
		if (nowVisible)
			index++;

		ClcContact *cc = group->cl.items[group->scanIndex];
		if ((IsHContactGroup(dwItem) && cc->type == CLCIT_GROUP && (dwItem & ~HCONTACT_ISGROUP) == cc->groupId) || 
			 (IsHContactContact(dwItem) && cc->type == CLCIT_CONTACT && cc->hContact == dwItem) ||
			 (IsHContactInfo(dwItem) && cc->type == CLCIT_INFO && cc->hContact == (dwItem & ~HCONTACT_ISINFO)))
		{
			if (isVisible) {
				if (!nowVisible)
					*isVisible = 0;
				else {
					int posY = cli.pfnGetRowTopY(dat, index+1);
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
				*contact = cc;
			if (subgroup)
				*subgroup = group;
			return 1;
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

int fnGetRowByIndex(struct ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup)
{
	int index = 0;
	ClcGroup *group = &dat->list;

	if (testindex<0)
		return (-1);

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
				break;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl.items[group->scanIndex];
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

HANDLE fnContactToHItem(ClcContact *contact)
{
	switch (contact->type) {
	case CLCIT_CONTACT:
		return (HANDLE)contact->hContact;
	case CLCIT_GROUP:
		return (HANDLE)(contact->groupId | HCONTACT_ISGROUP);
	case CLCIT_INFO:
		return (HANDLE)((UINT_PTR)contact->hContact | HCONTACT_ISINFO);
	}
	return NULL;
}

HANDLE fnContactToItemHandle(ClcContact *contact, DWORD *nmFlags)
{
	switch (contact->type) {
	case CLCIT_CONTACT:
		return (HANDLE)contact->hContact;
	case CLCIT_GROUP:
		if (nmFlags)
			*nmFlags |= CLNF_ISGROUP;
		return (HANDLE)contact->groupId;
	case CLCIT_INFO:
		if (nmFlags)
			*nmFlags |= CLNF_ISINFO;
		return (HANDLE)((UINT_PTR)contact->hContact | HCONTACT_ISINFO);
	}
	return NULL;
}
