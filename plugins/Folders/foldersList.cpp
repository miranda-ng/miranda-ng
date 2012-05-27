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

#include "foldersList.h"

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
	int i;
	for (i = 0; i < count; i++)
		{
			delete (list[i]);
		}
	
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
	int i;
	for (i = 0; i < count; i++)
		{
			list[i]->Save();
		}
}

PFolderItem CFoldersList::Get(int index)
{
	index--;
	if ((index < 0) || (index >= count))
		{
			return NULL;
		}
	return list[index];	
}

PFolderItem CFoldersList::Get(const char *section, const char *name)
{
	int i;
	for (i = 0; i < count; i++)
		{
			if (list[i]->IsEqual(section, name))
				{
					return list[i];
				}
		}
	return NULL;
}

PFolderItem CFoldersList::Get(const WCHAR *section, const WCHAR *name)
{
	const int MAX_SIZE = 2048;
	char aSection[MAX_SIZE];
	char aName[MAX_SIZE];
	UINT cp = static_cast<UINT>(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0));
	
	WideCharToMultiByte(cp, 0, section, -1, aSection, MAX_SIZE, NULL, NULL);
	WideCharToMultiByte(cp, 0, name, -1, aName, MAX_SIZE, NULL, NULL);
	
	return Get(aSection, aName);
}

PFolderItem CFoldersList::GetTranslated(const char *trSection, const char *trName)
{
	int i;
	for (i = 0; i < count; i++)
	{
		if (list[i]->IsEqualTranslated(trSection, trName))
		{
			return list[i];
		}
	}
	
	return NULL;
}

PFolderItem CFoldersList::GetTranslated(WCHAR *trSection, const WCHAR *trName)
{
	const int MAX_SIZE = 2048;
	char aSection[MAX_SIZE];
	char aName[MAX_SIZE];
	UINT cp = static_cast<UINT>(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0));
	
	WideCharToMultiByte(cp, 0, trSection, -1, aSection, MAX_SIZE, NULL, NULL);
	WideCharToMultiByte(cp, 0, trName, -1, aName, MAX_SIZE, NULL, NULL);
	
	return GetTranslated(aSection, aName);
}


int CFoldersList::Expand(int index, char *szResult, int size)
{
	PFolderItem tmp = Get(index);
	int res = 1;
	if (tmp)
		{
			tmp->Expand(szResult, size);
			res = 0;
		}
		else{
			memset(szResult, 0, size);
		}
	return res;
}

int CFoldersList::Add(CFolderItem *item)
{
	EnsureCapacity();
	int pos = Contains(item);
	if (!pos)
		{
			list[count++] = item;
			return count;
		}
		else{
			delete item;
			return pos;
		}
	return 0;
}

int CFoldersList::Add(FOLDERSDATA data)
{
	CFolderItem *item;
	if (data.flags & FF_UNICODE)
		{
			item = new CFolderItem(data.szSection, data.szName, (char *) data.szFormatW, data.flags);
		}
		else{
			item = new CFolderItem(data.szSection, data.szName, data.szFormat, data.flags);
		}
	return Add(item);
}

void CFoldersList::Remove(CFolderItem *item)
{
//	Remove(item->GetUniqueID());
}

void CFoldersList::Remove(int uniqueID)
{
/*	int i, j;
	CFolderItem *tmp;
	for (i = 0; i < count; i++)
		{
			if (list[i]->GetUniqueID() == uniqueID)
				{
					tmp = list[i];
					for (j = i; j < count - 1; j++)
						{
							list[j] = list[j + 1];
						}
					count--;
					delete tmp;
					return;
				}
		}
*/
}

int CFoldersList::Contains(CFolderItem *item)
{
	return Contains(item->GetSection(), item->GetName());
}

int CFoldersList::Contains(const char *section, const char *name)
{
	int i;
	for (i = 0; i < count; i++)
		{
			if (list[i]->IsEqual(section, name))
				{
					return i + 1;
				}
		}
	return 0;
}

void CFoldersList::EnsureCapacity()
{
	if (count >= capacity)
		{
			Enlarge(capacity / 2);
		}
}

void CFoldersList::Enlarge(int increaseAmount)
{
	int newSize = capacity + increaseAmount;
	list = (PFolderItem *) realloc(list, newSize * sizeof(PFolderItem));
	capacity = newSize;
}