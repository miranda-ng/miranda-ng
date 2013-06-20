#include "cJSON.h"
#include "queue.h"
#include <time.h>

typedef struct
{
	QUEUEHDR hdr;
	char *pszUser;
	char *pszAuthor;	// In case of groupchat, pszUser is the Groupchat ID and pszAuthor is the user sending it, if not a GC message, pszAuthor is NULL
	char *pszAlias;
	char *pszMessage;
	time_t timestamp;
	char szStatus[16];
	char szFailure[256];
	char szType[20];	// TEXT, SETTOPIC, ADDEDMEMBERS, CREATEDCHATWITH, LEFT, KICKED, ...
	unsigned int uRqId;
} MSGENTRY;

TYP_LIST *MsgQueue_Init(void);
void MsgQueue_Exit(TYP_LIST *hList);

MSGENTRY *MsgQueue_Insert(TYP_LIST *hList, cJSON *pNick);
MSGENTRY *MsgQueue_AddReflect(TYP_LIST *hList, cJSON *pNick, TYP_LIST *hBuddyList);
MSGENTRY *MsgQueue_AddSent(TYP_LIST *hList, char *pszUser, char *pszAlias, char *pszMessage, unsigned int *puMsgId);
MSGENTRY *MsgQueue_AddEvent(TYP_LIST *hList, char *pszUser, char *pszType);
BOOL MsgQueue_Remove(TYP_LIST *hList, unsigned int uMsgNr);
MSGENTRY *MsgQueue_Find(TYP_LIST *hList, unsigned int uMsgNr);
MSGENTRY *MsgQueue_FindByRqId(TYP_LIST *hList, unsigned int uRqId);
