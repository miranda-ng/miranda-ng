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

#include <commonheaders.h>

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

int FindItem(HWND hwnd, struct ClcData *dat, HANDLE hItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible)
{
	int index = 0;
	int nowVisible = 1;
	ClcGroup *group = &dat->list;

	group->scanIndex = 0;
	for (; ;) {
		if (group->scanIndex == group->cl.count) {
			ClcGroup *tgroup;
			group = group->parent;
			if (group == NULL)
				break;
			nowVisible = 1;
			for (tgroup = group; tgroup; tgroup = tgroup->parent) {
				if ( !(group->expanded)) {
					nowVisible = 0; break;
				}
			}
			group->scanIndex++;
			continue;
		}
		if (nowVisible)
			index++;
		if ((IsHContactGroup(hItem) && group->cl.items[group->scanIndex]->type == CLCIT_GROUP && ((UINT_PTR) hItem & ~HCONTACT_ISGROUP) == group->cl.items[group->scanIndex]->groupId) ||
			 (IsHContactContact(hItem) && group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && group->cl.items[group->scanIndex]->hContact == (MCONTACT)hItem) || 
			 (IsHContactInfo(hItem) && group->cl.items[group->scanIndex]->type == CLCIT_INFO && group->cl.items[group->scanIndex]->hContact == (MCONTACT)((UINT_PTR) hItem & ~HCONTACT_ISINFO)))
		{
			if (isVisible) {
				if ( !nowVisible)
					*isVisible = 0;
				else {
					int posy = RowHeight::getItemTopY(dat,index+1);
					if (posy<dat->yScroll)
						*isVisible=0;
					//if ((index + 1) * dat->rowHeight< dat->yScroll)
					//    *isVisible = 0;
					else {
						RECT clRect;
						GetClientRect(hwnd, &clRect);
						//if (index * dat->rowHeight >= dat->yScroll + clRect.bottom)
						if (posy >= dat->yScroll + clRect.bottom)
							*isVisible = 0;
						else
							*isVisible = 1;
					}
				}
			}
			if (contact)
				*contact = group->cl.items[group->scanIndex];
			if (subgroup)
				*subgroup = group;
			return 1;
		}
		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			nowVisible &= (group->expanded);
			continue;
		}
		group->scanIndex++;
	}

	if (isVisible) *isVisible = FALSE;
	if (contact)   *contact = NULL;
	if (subgroup)  *subgroup = NULL;
	return 0;
}
