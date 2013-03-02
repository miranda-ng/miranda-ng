/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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

CFoldersList::CFoldersList(int initialSize)
{
	capacity = 0;
	count = 0;
	list = NULL;
	Enlarge(initialSize);
}

CFoldersList::~CFoldersList()
{
	Clear();
	free(list);
}

void CFoldersList::Clear()
{
	for (int i = 0; i < count; i++)
		delete (list[i]);
	
	count = 0;
}

int CFoldersList::Count()
{
	return count;
}

int CFoldersList::Capacity()
{
	return capacity;
}

void CFoldersList::Save()
{
	for (int i = 0; i < count; i++)
		list[i]->Save();
}

PFolderItem CFoldersList::Get(int index)
{
	index--;
	if ((index < 0) || (index >= count))
		return NULL;

	return list[index];	
}

PFolderItem CFoldersList::Get(const char *section, const TCHAR *name)
{
	for (int i = 0; i < count; i++)
		if (list[i]->IsEqual(section, name))
			return list[i];

	return NULL;
}

PFolderItem CFoldersList::GetTranslated(const char *trSection, const TCHAR *trName)
{
	for (int i = 0; i < count; i++)
		if (list[i]->IsEqualTranslated(trSection, trName))
			return list[i];
	
	return NULL;
}

int CFoldersList::Expand(int index, TCHAR *szResult, int size)
{
	PFolderItem tmp = Get(index);
	if (tmp) {
		tmp->Expand(szResult, size);
		return 0;
	}

	memset(szResult, 0, size);
	return 1;
}

int CFoldersList::Add(CFolderItem *item)
{
	EnsureCapacity();
	int pos = Contains(item);
	if (!pos) {
		list[count++] = item;
		return count;
	}

	delete item;
	return 0;
}

int CFoldersList::Add(FOLDERSDATA* data)
{
	FOLDERSDATA tmp;
	if (data->cbSize < sizeof(FOLDERSDATA)) {
		memset(&tmp, 0, sizeof(FOLDERSDATA));
		memcpy(&tmp, data, data->cbSize);
		data = &tmp;
	}

	if (data->flags & FF_UNICODE)
		return Add( new CFolderItem(data->szSection, data->szName, data->szFormatW, data->szUserNameW));

	return Add( new CFolderItem(data->szSection, data->szName, _A2T(data->szFormat), _A2T(data->szUserName)));
}

void CFoldersList::Remove(CFolderItem *item)
{
}

void CFoldersList::Remove(int uniqueID)
{
}

int CFoldersList::Contains(CFolderItem *item)
{
	return Contains(item->GetSection(), item->GetUserName());
}

int CFoldersList::Contains(const char *section, const TCHAR *name)
{
	for (int i = 0; i < count; i++)
		if (list[i]->IsEqual(section, name))
			return i + 1;

	return 0;
}

void CFoldersList::EnsureCapacity()
{
	if (count >= capacity)
		Enlarge(capacity / 2);
}

void CFoldersList::Enlarge(int increaseAmount)
{
	int newSize = capacity + increaseAmount;
	list = (PFolderItem *) realloc(list, newSize * sizeof(PFolderItem));
	capacity = newSize;
}
