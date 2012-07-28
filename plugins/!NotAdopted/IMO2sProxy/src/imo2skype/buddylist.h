#include "cJSON.h"
#include "memlist.h"

struct tag_nickentry;
typedef struct tag_nickentry NICKENTRY;

struct tag_nickentry
{
	char *pszUser;
	char *pszStatusText;
	char szStatus[16];
	char *pszAlias;
	char *pszDisplay;		// Display name, on Groupchat it is topic
	int  iBuddyStatus;
	TYP_LIST *hGCMembers;	// List of groupchat members, if this is a groupchat
	NICKENTRY *pGroup;		// Uplink to Chatgroup, if Groupchat-Member
};

TYP_LIST *BuddyList_Init(void);
void BuddyList_Exit(TYP_LIST *hList);

BOOL BuddyList_Insert(TYP_LIST *hList, cJSON *pNick);
BOOL BuddyList_AddTemporaryUser(TYP_LIST *hList, char *pszUser);
BOOL BuddyList_Remove(TYP_LIST *hList, NICKENTRY *pEntry);
NICKENTRY *BuddyList_Find(TYP_LIST *hList, char *pszUser);
BOOL BuddyList_SetStatus(TYP_LIST *hList, cJSON *pNick);
void BuddyList_FreeEntry(NICKENTRY *pEntry, BOOL bFreeGC);
#define BuddyList_IsGroupchat(pEntry) (pEntry->hGCMembers!=NULL)
