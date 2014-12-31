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

#include "commonheaders.h"

/* a simple sorted list implementation */

MIR_CORE_DLL(SortedList*) List_Create(int p_limit, int p_increment)
{
	SortedList* result = (SortedList*)mir_calloc(sizeof(SortedList));
	if (result == NULL)
		return(NULL);

	result->increment = p_increment;
	result->limit = p_limit;
	return result;
}

MIR_CORE_DLL(void) List_Destroy(SortedList* p_list)
{
	if (p_list == NULL)
		return;

	if (p_list->items != NULL) {
		mir_free(p_list->items);
		p_list->items = NULL;
	}

	p_list->realCount = p_list->limit = 0;
}

MIR_CORE_DLL(void*) List_Find(SortedList* p_list, void* p_value)
{
	int idx;
	if (!List_GetIndex(p_list, p_value, &idx))
		return NULL;

	return p_list->items[idx];
}

#ifdef _DEBUG
#pragma optimize("gt", on)
#endif

MIR_CORE_DLL(int) List_GetIndex(SortedList* p_list, void* p_value, int* p_index)
{
	if (p_value == NULL || p_list->sortFunc == NULL) {
		*p_index = -1;
		return 0;
	}

	int low = 0;
	int high = p_list->realCount - 1;

	switch ((INT_PTR)p_list->sortFunc) {
	case HandleKeySort:
	#ifdef _WIN64
		{
			const unsigned __int64 val = *(unsigned __int64 *)p_value;

			while (low <= high) {
				int i = (low + high) / 2;
				unsigned __int64 vali = *(unsigned __int64 *)p_list->items[i];
				if (vali == val) {
					*p_index = i;
					return 1;
				}

				if (vali < val)
					low = i + 1;
				else
					high = i - 1;
			}
		}
		break;
	#endif

	case NumericKeySort:
		{
			const unsigned val = *(unsigned *)p_value;

			while (low <= high) {
				int i = (low + high) / 2;
				unsigned vali = *(unsigned *)p_list->items[i];
				if (vali == val) {
					*p_index = i;
					return 1;
				}

				if (vali < val)
					low = i + 1;
				else
					high = i - 1;
			}
		}
		break;

	case PtrKeySort:
		while (low <= high) {
			int i = (low + high) / 2;
			const void* vali = p_list->items[i];
			if (vali == p_value) {
				*p_index = i;
				return 1;
			}

			if (vali < p_value)
				low = i + 1;
			else
				high = i - 1;
		}
		break;

	default:
		while (low <= high) {
			int i = (low + high) / 2;
			int result = p_list->sortFunc(p_list->items[i], p_value);
			if (result == 0) {
				*p_index = i;
				return 1;
			}

			if (result < 0)
				low = i + 1;
			else
				high = i - 1;
		}
		break;
	}

	*p_index = low;
	return 0;
}

MIR_CORE_DLL(int) List_IndexOf(SortedList* p_list, void* p_value)
{
	if (p_value == NULL)
		return -1;

	for (int i = 0; i < p_list->realCount; i++)
		if (p_list->items[i] == p_value)
			return i;

	return -1;
}

#ifdef _DEBUG
#pragma optimize("", on)
#endif

MIR_CORE_DLL(int) List_Insert(SortedList* p_list, void* p_value, int p_index)
{
	if (p_value == NULL || p_index > p_list->realCount)
		return 0;

	if (p_list->realCount == p_list->limit) {
		p_list->items = (void**)mir_realloc(p_list->items, sizeof(void*)*(p_list->realCount + p_list->increment));
		p_list->limit += p_list->increment;
	}

	if (p_index < p_list->realCount)
		memmove(p_list->items + p_index + 1, p_list->items + p_index, sizeof(void*)*(p_list->realCount - p_index));

	p_list->realCount++;

	p_list->items[p_index] = p_value;
	return 1;
}

MIR_CORE_DLL(int) List_InsertPtr(SortedList* list, void* p)
{
	if (p == NULL)
		return -1;

	int idx;
	if (list->sortFunc == 0)
		idx = list->realCount;
	else
		List_GetIndex(list, p, &idx);
	return List_Insert(list, p, idx);
}

MIR_CORE_DLL(int) List_Remove(SortedList* p_list, int index)
{
	if (index < 0 || index > p_list->realCount)
		return 0;

	p_list->realCount--;
	if (p_list->realCount > index) {
		memmove(p_list->items + index, p_list->items + index + 1, sizeof(void*)*(p_list->realCount - index));
		p_list->items[p_list->realCount] = NULL;
	}

	return 1;
}

MIR_CORE_DLL(int) List_RemovePtr(SortedList* list, void* p)
{
	int idx;
	if (!List_GetIndex(list, p, &idx))
		return -1;

	List_Remove(list, idx);
	return idx;
}

MIR_CORE_DLL(void) List_Copy(SortedList* s, SortedList* d, size_t itemSize)
{
	d->increment = s->increment;
	d->limit = s->limit;
	d->realCount = s->realCount;
	d->items = (void**)mir_alloc(sizeof(void*) * d->realCount);
	memcpy(d->items, s->items, sizeof(void*) * d->realCount);
}

MIR_CORE_DLL(void) List_ObjCopy(SortedList* s, SortedList* d, size_t itemSize)
{
	d->increment = s->increment;
	d->sortFunc = s->sortFunc;

	for (int i = 0; i < s->realCount; i++) {
		void* item = new char[itemSize];
		memcpy(item, s->items[i], itemSize);
		List_Insert(d, item, i);
	}
}
