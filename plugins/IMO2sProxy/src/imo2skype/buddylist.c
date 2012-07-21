/* Module:  buddylist.c
   Purpose: Manages your list of buddies in memory
   Author:  leecher
   Date:    30.08.2009
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memlist.h"
#include "buddylist.h"

static void SetEntry(NICKENTRY *pEntry, cJSON *pNick);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

TYP_LIST *BuddyList_Init(void)
{
	TYP_LIST *hList = List_Init(16);

	return hList;
}

// -----------------------------------------------------------------------------

void BuddyList_Exit(TYP_LIST *hList)
{
	NICKENTRY *pEntry;

	while (pEntry=(NICKENTRY*)List_Pop(hList))
	{
		BuddyList_FreeEntry(pEntry, TRUE);
		free (pEntry);
	}
	List_Exit(hList);
}

// -----------------------------------------------------------------------------

BOOL BuddyList_Insert(TYP_LIST *hList, cJSON *pNick)
{
	NICKENTRY *pEntry;
	cJSON *pElem;

	if (pEntry=BuddyList_Find (hList, cJSON_GetObjectItem(pNick, "buid")->valuestring))
		BuddyList_FreeEntry (pEntry, FALSE);
	else
	{
		if (!(pEntry = calloc (1, sizeof(NICKENTRY)))) return FALSE;
		if ((pElem=cJSON_GetObjectItem(pNick, "group")) && strcmp(pElem->valuestring, "Skype"))
		{
			char szBUID[256];
			NICKENTRY *pGroup;

			// Buddy Belongs to a chatgroup
			sprintf (szBUID, "%s;", pElem->valuestring);
			pGroup = BuddyList_Find (hList, szBUID);

			if (pGroup) 	// Group not yet created? Shouldn't be the case
			{
				if (!pGroup->hGCMembers) pGroup->hGCMembers=List_Init(1);
				hList = pGroup->hGCMembers;
				pEntry->pGroup = pGroup;
			}
		}
		if (!List_Push(hList, pEntry)) return FALSE;
	}
	SetEntry(pEntry, pNick);
	pEntry->iBuddyStatus = 3;
	return TRUE;
}

// -----------------------------------------------------------------------------

BOOL BuddyList_AddTemporaryUser(TYP_LIST *hList, char *pszUser)
{
	NICKENTRY *pEntry;
	
	if (BuddyList_Find (hList, pszUser)) return TRUE;
	if (!(pEntry = calloc (1, sizeof(NICKENTRY)))) return FALSE;
	pEntry->pszUser = strdup(pszUser);
	pEntry->pszAlias = strdup(pszUser);
	strcpy (pEntry->szStatus, "OFFLINE");
	pEntry->iBuddyStatus = 2;
	return List_Push(hList, pEntry);
}

// -----------------------------------------------------------------------------

BOOL BuddyList_Remove(TYP_LIST *hList, NICKENTRY *pEntry)
{
	NICKENTRY *pListEntry;
	int i, nCount;

	if (pEntry->pGroup) hList=pEntry->pGroup->hGCMembers;
	for (i=0, nCount=List_Count(hList); i<nCount; i++)
	{
		pListEntry = List_ElementAt (hList, i);
		if (pListEntry == pEntry) break;
	}
	if (i<nCount)
	{
		BuddyList_FreeEntry (pEntry, TRUE);
		List_RemoveElementAt(hList, i);
		free (pEntry);
		return TRUE;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

NICKENTRY *BuddyList_Find(TYP_LIST *hList, char *pszUser)
{
	int i, nCount;
	NICKENTRY *pEntry;

	for (i=0, nCount=List_Count(hList); i<nCount; i++)
	{
		pEntry = List_ElementAt (hList, i);
		if (strcmp(pEntry->pszUser, pszUser) == 0)
			return pEntry;
		else if (pEntry->hGCMembers)
		{
			if (pEntry = BuddyList_Find(pEntry->hGCMembers, pszUser))
				return pEntry;
		}
	}
	return NULL;
}

// -----------------------------------------------------------------------------

BOOL BuddyList_SetStatus(TYP_LIST *hList, cJSON *pNick)
{
	NICKENTRY *pEntry;

	if ((pEntry = BuddyList_Find(hList,  cJSON_GetObjectItem(pNick, "buid")->valuestring)))
	{
		BuddyList_FreeEntry(pEntry, FALSE);
		SetEntry(pEntry, pNick);
		return TRUE;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

void BuddyList_FreeEntry(NICKENTRY *pEntry, BOOL bFreeGC)
{
	if (pEntry->pszAlias)
	{
		free (pEntry->pszAlias);
		pEntry->pszAlias = NULL;
	}
	if (pEntry->pszUser) 
	{
		free (pEntry->pszUser);
		pEntry->pszUser = NULL;
	}
	if (pEntry->pszStatusText) 
	{
		free(pEntry->pszStatusText);
		pEntry->pszStatusText = NULL;
	}
	if (pEntry->pszDisplay) 
	{
		free(pEntry->pszDisplay);
		pEntry->pszDisplay = NULL;
	}
	if (bFreeGC && pEntry->hGCMembers)
	{
		NICKENTRY *pNick;

		while (pNick=(NICKENTRY*)List_Pop(pEntry->hGCMembers))
		{
			BuddyList_FreeEntry(pNick, bFreeGC);
			free (pNick);
		}
		List_Exit (pEntry->hGCMembers);
		pEntry->hGCMembers = NULL;
	}
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void SetEntry(NICKENTRY *pEntry, cJSON *pNick)
{
	cJSON *obj;

	if ((obj=cJSON_GetObjectItem(pNick, "alias")) && obj->valuestring)
		pEntry->pszAlias =  strdup(obj->valuestring);
	pEntry->pszUser = strdup(cJSON_GetObjectItem(pNick, "buid")->valuestring);
	if ((obj=cJSON_GetObjectItem(pNick, "status")) && obj->valuestring)
		pEntry->pszStatusText = strdup(obj->valuestring);
	if ((obj=cJSON_GetObjectItem(pNick, "primitive")) && obj->valuestring)
		strcpy (pEntry->szStatus, obj->valuestring);
	if ((obj=cJSON_GetObjectItem(pNick, "display")) && obj->valuestring)
		pEntry->pszDisplay = strdup(obj->valuestring);
}
