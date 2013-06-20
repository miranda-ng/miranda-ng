/* Module:  callqueue.c
   Purpose: Queue for incoming calls
   Author:  leecher
   Date:    02.09.2009

   Fixme:   Sort on insert, do a binary search instead of iterating list.
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "callqueue.h"

static volatile unsigned int m_uMsgNr=0;
static void FreeEntry(void *pPEntry);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

TYP_LIST *CallQueue_Init(void)
{
	TYP_LIST *hList = List_Init(16);

	return hList;
}

// -----------------------------------------------------------------------------

void CallQueue_Exit(TYP_LIST *hList)
{
	Queue_Exit (hList, FreeEntry);
}

// -----------------------------------------------------------------------------

CALLENTRY *CallQueue_Insert(TYP_LIST *hList, cJSON *pNick, int iDirection)
{
	CALLENTRY *pEntry;
	cJSON *pStream, *pVal, *pPipe;

	if (!(pEntry = Queue_InsertEntry(hList, sizeof(CALLENTRY), ++m_uMsgNr,
		FreeEntry))) return NULL;
	pEntry->pszUser = strdup(cJSON_GetObjectItem(pNick, "buid")->valuestring);
	time (&pEntry->timestamp);
	strcpy (pEntry->szStatus, "RINGING");

	if (pStream = cJSON_GetObjectItem(pNick, "send_stream"))
	{
		strcpy (pEntry->szSendStream, pStream->valuestring);
		if (pStream = cJSON_GetObjectItem(pNick, "recv_stream"))
			strcpy (pEntry->szRecvStream, pStream->valuestring);
	}
	else
	{
		// Copy pipe to Call object
		if (pPipe = cJSON_GetObjectItem(pNick, "pipe"))
		{
			if (pVal = cJSON_GetObjectItem(pPipe, "ip"))
				strncpy (pEntry->szIP, pVal->valuestring, sizeof(pEntry->szIP));
			if (pVal = cJSON_GetObjectItem(pPipe, "conv"))
				strncpy (pEntry->szConv, pVal->valuestring, sizeof(pEntry->szConv));
			if (pVal = cJSON_GetObjectItem(pPipe, "role"))
				pEntry->iRole = pVal->valueint;
		}
	}
	pEntry->iDirection = iDirection;
	return pEntry;
}

// -----------------------------------------------------------------------------

BOOL CallQueue_Remove(TYP_LIST *hList, unsigned int uMsgNr)
{
	return Queue_Remove (hList, uMsgNr, FreeEntry);
}

// -----------------------------------------------------------------------------

CALLENTRY *CallQueue_Find(TYP_LIST *hList, unsigned int uMsgNr)
{
	return (CALLENTRY*)Queue_Find(hList, uMsgNr);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void FreeEntry(void *pPEntry)
{
	CALLENTRY *pEntry = (CALLENTRY*)pPEntry;

	free (pEntry->pszUser);
}

