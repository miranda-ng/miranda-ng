#include "cJSON.h"
#include "memlist.h"

typedef struct
{
	char *pszUser;
	char *pszIcon;
} AVATARENTRY;

TYP_LIST *AvatarList_Init(void);
void AvatarList_Exit(TYP_LIST *hList);

BOOL AvatarList_Insert(TYP_LIST *hList, cJSON *pIcon);
BOOL AvatarList_Remove(TYP_LIST *hList, AVATARENTRY *pEntry);
AVATARENTRY *AvatarList_Find(TYP_LIST *hList, char *pszUser);
void AvatarList_FreeEntry(AVATARENTRY *pEntry);
