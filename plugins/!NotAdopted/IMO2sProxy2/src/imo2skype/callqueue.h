#include "cJSON.h"
#include "queue.h"
#include <time.h>

#define CALL_INCOMING	0
#define CALL_OUTGOING	1

typedef struct
{
	QUEUEHDR hdr;
	char *pszUser;
	char szStatus[16];

	char szSendStream[18];
	char szRecvStream[18];

	char szConv[18];
	char szIP[16];
	int  iRole;

	char szCallFile[256];
	void *hCallWnd;
	int  iDirection;
	time_t timestamp;
} CALLENTRY;

TYP_LIST *CallQueue_Init(void);
void CallQueue_Exit(TYP_LIST *hList);

CALLENTRY *CallQueue_Insert(TYP_LIST *hList, cJSON *pNick, int iDirection);
BOOL CallQueue_Remove(TYP_LIST *hList, unsigned int uCallNr);
CALLENTRY *CallQueue_Find(TYP_LIST *hList, unsigned int uCallNr);
