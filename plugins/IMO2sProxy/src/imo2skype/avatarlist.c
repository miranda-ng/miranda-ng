/* Module:  avatarlist.c
   Purpose: Manages the avatars of your buddies
   Author:  leecher
   Date:    18.08.2011
*/

#include <stdlib.h>
#include <string.h>
#include "memlist.h"
#include "avatarlist.h"

static void SetEntry(AVATARENTRY *pEntry, cJSON *pNick);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

TYP_LIST *AvatarList_Init(void)
{
	TYP_LIST *hList = List_Init(16);

	return hList;
}

// -----------------------------------------------------------------------------

void AvatarList_Exit(TYP_LIST *hList)
{
	AVATARENTRY *pEntry;

	while (pEntry=(AVATARENTRY*)List_Pop(hList))
	{
		AvatarList_FreeEntry(pEntry);
		free (pEntry);
	}
	List_Exit(hList);
}

// -----------------------------------------------------------------------------

BOOL AvatarList_Insert(TYP_LIST *hList, cJSON *pIcon)
{
	AVATARENTRY *pEntry;

	if (pEntry=AvatarList_Find (hList, cJSON_GetObjectItem(pIcon, "buid")->valuestring))
		AvatarList_FreeEntry (pEntry);
	else
	{
		if (!(pEntry = calloc (1, sizeof(AVATARENTRY)))) return FALSE;
		if (!List_Push(hList, pEntry)) return FALSE;
	}
	SetEntry(pEntry, pIcon);
	return TRUE;
}

// -----------------------------------------------------------------------------

BOOL AvatarList_Remove(TYP_LIST *hList, AVATARENTRY *pEntry)
{
	AVATARENTRY *pListEntry;
	int i, nCount;

	for (i=0, nCount=List_Count(hList); i<nCount; i++)
	{
		pListEntry = List_ElementAt (hList, i);
		if (pListEntry == pEntry) break;
	}
	if (i<nCount)
	{
		AvatarList_FreeEntry (pEntry);
		List_RemoveElementAt(hList, i);
		free (pEntry);
		return TRUE;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

AVATARENTRY *AvatarList_Find(TYP_LIST *hList, char *pszUser)
{
	int i, nCount;
	AVATARENTRY *pEntry;

	for (i=0, nCount=List_Count(hList); i<nCount; i++)
	{
		pEntry = List_ElementAt (hList, i);
		if (strcmp(pEntry->pszUser, pszUser) == 0)
			return pEntry;
	}
	return NULL;
}

// -----------------------------------------------------------------------------

void AvatarList_FreeEntry(AVATARENTRY *pEntry)
{
	if (pEntry->pszUser) free (pEntry->pszUser);
	if (pEntry->pszIcon) free (pEntry->pszIcon);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void SetEntry(AVATARENTRY *pEntry, cJSON *pNick)
{
	pEntry->pszUser = strdup(cJSON_GetObjectItem(pNick, "buid")->valuestring);
	pEntry->pszIcon = strdup(cJSON_GetObjectItem(pNick, "icon")->valuestring);
}
