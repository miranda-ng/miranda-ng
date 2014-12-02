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

#define CLIST_GROUPS "CListGroups"

HANDLE hsYGMove = NULL;
HANDLE hsYGReset = NULL;

int InitServices()
{
	hsYGMove = CreateServiceFunction(MS_YAHOOGROUPS_MOVE, YahooGroupsMoveService);
	hsYGReset = CreateServiceFunction(MS_YAHOOGROUPS_RESET, YahooGroupsResetService);
	
	ReadAvailableGroups();

	return 0;
}

int DestroyServices()
{
	DestroyServiceFunction(hsYGMove);
	DestroyServiceFunction(hsYGReset);

	return 0;
}

void ReadAvailableGroups()
{
	int ok = 1;
	int index = 0;
	
	char group[1024] = {0};
	char tmp[128] = {0};
	
	while (ok)
	{
		mir_snprintf(tmp, SIZEOF(tmp), "%d", index);
		GetStringFromDatabase(NULL, CLIST_GROUPS, tmp, NULL, group, sizeof(group));
		if (strlen(group) > 0)
		{
			availableGroups.Add(_strdup(group + 1));
			index += 1;
		}
		else{
			ok = 0;
		}
	}
}

int GetNextGroupIndex()
{
	int index = 0;
	int found = 0;
	
	char buffer[1024] = {0};
	char tmp[128] = {0};
	
	while (!found)
	{
		mir_snprintf(tmp, SIZEOF(tmp), "%d", index++);
		
		if (GetStringFromDatabase(NULL, CLIST_GROUPS, tmp, NULL, buffer, sizeof(buffer)))
		{
			found = 1;
		}
	}
	
	return index - 1;
}

void AddNewGroup(char *newGroup)
{
	int index = GetNextGroupIndex();
	
	char tmp[128];
	char group[1024];

	*group = 1;
	strncpy_s((group + 1), (SIZEOF(group) - 1), newGroup, _TRUNCATE);
	
	mir_snprintf(tmp, SIZEOF(tmp), "%d", index);
	const int MAX_SIZE = 1024;
	wchar_t wide[MAX_SIZE] = {0};
	*wide = 1;
	MultiByteToWideChar(currentCodePage, 0, group + 1, -1, wide + 1, MAX_SIZE - 1);
	db_set_ws(NULL, CLIST_GROUPS, tmp, wide);
	
	availableGroups.Add(_strdup(group + 1));
}

void AddContactToGroup(MCONTACT hContact, char *group)
{
	const int MAX_SIZE = 1024;
	wchar_t wide[MAX_SIZE] = {0};
	MultiByteToWideChar(currentCodePage, 0, group, -1, wide, MAX_SIZE);
	db_set_ws(hContact, "CList", "Group", wide);
}

void CreateGroup(char *group)
{
	char *p = group;
	char *sub = group;
	char buffer[1024] = {0};
	
	while ((p = strchr(sub, '\\')))
	{
		*p = 0;
		if (strlen(buffer) > 0)
		{
			strncat(buffer, "\\", sizeof(buffer));
			strncat(buffer, sub, sizeof(buffer));
		}
		else{
			strncpy_s(buffer, sub, _TRUNCATE);
		}
		
		if (!availableGroups.Contains(buffer))
		{
			AddNewGroup(buffer);
		}
		
		*p++ = '\\';
		sub = p;
	}
	
	if (sub)
	{
		if (strlen(buffer) > 0)
		{
			strncat(buffer, "\\", sizeof(buffer));
			strncat(buffer, sub, sizeof(buffer));
		}
		else{
			strncpy_s(buffer, sub, _TRUNCATE);
		}
		
		if (!availableGroups.Contains(buffer))
		{
			AddNewGroup(buffer);
		}
	}
}

void ProcessContacts(void (*callback)(MCONTACT, char *), char *protocol)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		callback(hContact, protocol);
}

void YahooMoveCallback(MCONTACT hContact, char *unused)
{
	char protocol[128] = {0};
	GetContactProtocol(hContact, protocol, sizeof(protocol));
	
	if (strlen(protocol) > 0)
	{
		char ygroup[128] = {0};
		
		if (!GetStringFromDatabase(hContact, protocol, "YGroup", NULL, ygroup, sizeof(ygroup)))
		{
			CreateGroup(ygroup);
			AddContactToGroup(hContact, ygroup);
		}
	}
}

void ResetGroupCallback(MCONTACT hContact, char *protocol)
{
	char p[128] = {0};
	
	GetContactProtocol(hContact, p, sizeof(protocol));
	
	if ((!protocol) || (_stricmp(protocol, p) == 0))
	{
		db_unset(hContact, "CList", "Group");
	}
}

INT_PTR YahooGroupsMoveService(WPARAM wParam, LPARAM lParam)
{
	ProcessContacts(YahooMoveCallback, NULL);
	return 0;
}

INT_PTR YahooGroupsResetService(WPARAM wParam, LPARAM lParam)
{
	ProcessContacts(ResetGroupCallback, NULL);
	return 0;
}
