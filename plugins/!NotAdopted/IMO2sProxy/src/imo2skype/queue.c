/* Module:  queue.c
   Purpose: Queue management
   Author:  leecher
   Date:    02.09.2009
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "memlist.h"
#include "queue.h"

// Maximum threshold for queues (So that we don't leak memory)
#define THRESHOLD 50

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

void Queue_Exit(TYP_LIST *hList, void (*fpFree)(void *pEntry))
{
	void *pEntry;
	
	while (pEntry=List_Pop(hList))
	{
		if (fpFree) fpFree(pEntry);
		free (pEntry);
	}
	List_Exit(hList);
}

// -----------------------------------------------------------------------------

void* Queue_InsertEntry (TYP_LIST *hList, unsigned int cbSize, unsigned int uMsgNr,
                         void (*fpFree)(void *pEntry))
{
	void *pEntry;

	if (!(pEntry = calloc (1, cbSize))) return NULL;
	if (!List_Push(hList, pEntry))
	{
		free (pEntry);
		return NULL;
	} else ((QUEUEHDR*)pEntry)->uMsgNr = uMsgNr;
#ifdef THRESHOLD
	if (List_Count(hList)>THRESHOLD)
	{
		void *pEntry = List_RemoveElementAt(hList, 0);
		
		if (pEntry) fpFree (pEntry);
		free (pEntry);
	}
#endif
	return pEntry;
}

// -----------------------------------------------------------------------------

BOOL Queue_Remove(TYP_LIST *hList, unsigned int uMsgNr, void (*fpFree)(void *pEntry))
{
	QUEUEHDR *pListEntry;
	unsigned int i;

	for (i=List_Count(hList)-1; (int)i!=-1; i--)
	{
		pListEntry = List_ElementAt (hList, i);
		if (pListEntry->uMsgNr == uMsgNr)
		{
			if (fpFree) fpFree (pListEntry);
			List_RemoveElementAt(hList, i);
			free (pListEntry);
			return TRUE;
		}
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

void *Queue_Find(TYP_LIST *hList, unsigned int uMsgNr)
{
	unsigned int i;
	QUEUEHDR *pEntry;

	for (i=List_Count(hList)-1; (int)i!=-1; i--)
	{
		pEntry = (QUEUEHDR*)List_ElementAt (hList, i);
		if (pEntry->uMsgNr == uMsgNr)
			return pEntry;
	}
	return NULL;
}

// -----------------------------------------------------------------------------
