#include <time.h>

typedef struct {
	DWORD uMsgNum;
	HANDLE hEvent;
	HANDLE hMetaEvent;
	time_t t;
	time_t tEdited;
	void *pfts;
} TYP_MSGLENTRY;

void MsgList_Init(void);
void MsgList_Exit(void);
TYP_MSGLENTRY *MsgList_Add(DWORD uMsgNum, HANDLE hEvent);
TYP_MSGLENTRY *MsgList_FindMessage(DWORD uMsgNum);
void MsgList_CollectGarbage(void);
