#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "memlist.h"

typedef struct
{
	unsigned int uMsgNr;
} QUEUEHDR;

void Queue_Exit(TYP_LIST *hList, void (*fpFree)(void *pEntry));
void* Queue_InsertEntry (TYP_LIST *hList, unsigned int cbSize, unsigned int uMsgNr,
                         void (*fpFree)(void *pEntry));
BOOL Queue_Remove(TYP_LIST *hList, unsigned int uMsgNr, void (*fpFree)(void *pEntry));
void *Queue_Find(TYP_LIST *hList, unsigned int uMsgNr);

#endif
