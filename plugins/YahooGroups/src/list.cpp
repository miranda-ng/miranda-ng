/*
YahooGroups plugin for Miranda IM

Copyright © 2007 Cristian Libotean

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

CGroupsList &availableGroups = CGroupsList();

void CGroupsList::Enlarge(int amount)
{
	size += amount;
	groups = (char **) realloc(groups, size * sizeof(char *));
}

void CGroupsList::EnsureCapacity()
{
	if (count >= size - 1)
	{
		Enlarge(size / 2);
	}
}

CGroupsList::CGroupsList()
{
	groups = NULL;
	size = 0;
	count = 0;
	Enlarge(10);
}

CGroupsList::~CGroupsList()
{
	Clear();
	
	free(groups);
	groups = NULL;
}

void CGroupsList::Clear()
{
	if (groups)
	{
		for (int i = 0; i < count; i++)
		{
			delete groups[i];
		}
		
		count = 0;
	}
}

void CGroupsList::Add(char *group)
{
	EnsureCapacity();
	groups[count++] = group;
}

char *CGroupsList::operator [](int index)
{
	if ((index < 0) || (index >= count))
	{
		return NULL;
	}
	
	return groups[index];
}

int CGroupsList::Count()
{
	return count;
}

int CGroupsList::Contains(const char *group)
{
	return (Index(group) >= 0);
}

int CGroupsList::Index(const char *group)
{
	int i;
	for (i = 0; i < count; i++)
	{
		if (_stricmp(groups[i], group) == 0)
		{
			return i;
		}
	}
	
	return -1;
}