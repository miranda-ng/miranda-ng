/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

TCmdList *tcmdlist_append(TCmdList *list, const char *data, int maxSize, BOOL temporary)
{
	TCmdList *new_list = (TCmdList *)mir_alloc(sizeof(TCmdList));
	TCmdList *attach_to = NULL;

	if (!data) {
		mir_free(new_list);
		return list;
	}
	new_list->temporary = temporary;
	new_list->next = NULL;
	new_list->szCmd = mir_strdup(data);
	for (TCmdList *n = list; n != NULL; n = n->next)
		attach_to = n;

	if (attach_to == NULL) {
		new_list->prev = NULL;
		return new_list;
	}

	new_list->prev = attach_to;
	attach_to->next = new_list;
	if (tcmdlist_len(list) > maxSize)
		list = tcmdlist_remove_first(list);
	return list;
}

TCmdList *tcmdlist_remove_first(TCmdList *list)
{
	TCmdList *n = list;
	if (n->next) n->next->prev = n->prev;
	if (n->prev) n->prev->next = n->next;
	list = n->next;
	mir_free(n->szCmd);
	mir_free(n);
	return list;
}

TCmdList *tcmdlist_remove(TCmdList *list, TCmdList *n)
{
	if (n->next) n->next->prev = n->prev;
	if (n->prev) n->prev->next = n->next;
	if (n == list) list = n->next;
	mir_free(n->szCmd);
	mir_free(n);
	return list;
}

TCmdList *tcmdlist_append2(TCmdList *list, MCONTACT hContact, const char *data)
{
	TCmdList *new_list = (TCmdList *)mir_alloc(sizeof(TCmdList));
	TCmdList *attach_to = NULL;

	if (!data) {
		mir_free(new_list);
		return list;
	}
	new_list->temporary = FALSE;
	new_list->next = NULL;
	new_list->hContact = hContact;
	new_list->szCmd = mir_strdup(data);
	list = tcmdlist_remove2(list, hContact);
	for (TCmdList *n = list; n != NULL; n = n->next)
		attach_to = n;

	if (attach_to == NULL) {
		new_list->prev = NULL;
		return new_list;
	}

	new_list->prev = attach_to;
	attach_to->next = new_list;
	return list;
}

TCmdList *tcmdlist_remove2(TCmdList *list, MCONTACT hContact)
{
	for (TCmdList *n = list; n != NULL; n = n->next) {
		if (n->hContact == hContact) {
			if (n->next) n->next->prev = n->prev;
			if (n->prev) n->prev->next = n->next;
			if (n == list) list = n->next;
			mir_free(n->szCmd);
			mir_free(n);
			return list;
		}
	}
	return list;
}

TCmdList *tcmdlist_get2(TCmdList *list, MCONTACT hContact)
{
	for (TCmdList *n = list; n != NULL; n = n->next)
		if (n->hContact == hContact)
			return n;

	return NULL;
}

int tcmdlist_len(TCmdList *list)
{
	int i = 0;
	for (TCmdList *n = list; n != NULL; n = n->next)
		i++;

	return i;
}

TCmdList *tcmdlist_last(TCmdList *list)
{
	for (TCmdList *n = list; n != NULL; n = n->next)
		if (!n->next)
			return n;

	return NULL;
}

void tcmdlist_free(TCmdList *list)
{
	TCmdList *n = list, *next;

	while (n != NULL) {
		next = n->next;
		mir_free(n->szCmd);
		mir_free(n);
		n = next;
	}
}
