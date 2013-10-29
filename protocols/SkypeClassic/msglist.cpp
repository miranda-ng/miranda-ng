#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "memlist.h"
#include "debug.h"
#include "msglist.h"

#define MSGLIST_TIMEOUT		1800		// Chatmessage references will be kept for 30 minutes

static TYP_LIST *m_hMsgList = NULL;

static int CmpProc(const void *pstPElement,const void *pstPToFind)
{
	return (DWORD)pstPToFind - ((TYP_MSGLENTRY*)pstPElement)->uMsgNum;
}

void MsgList_Init(void)
{
	m_hMsgList = List_Init(128);
}

void MsgList_Exit(void)
{
	if (!m_hMsgList) return;
	List_FreeElements (m_hMsgList);
	List_Exit(m_hMsgList);
	m_hMsgList = NULL;
}

TYP_MSGLENTRY *MsgList_Add(DWORD uMsgNum, HANDLE hEvent)
{
	TYP_MSGLENTRY *pEntry;
	int iListInd;
	BOOL bFound;

	LOG (("MsgList_Add (%d, %08X)", uMsgNum, hEvent));
	if (!m_hMsgList || !hEvent) return FALSE;
	bFound = List_BinarySearch(m_hMsgList,CmpProc,(void *)uMsgNum,&iListInd);
	if (!bFound) pEntry = (TYP_MSGLENTRY*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TYP_MSGLENTRY));
	else pEntry = (TYP_MSGLENTRY*)List_ElementAt (m_hMsgList, iListInd);
	if (!pEntry) return NULL;
	pEntry->uMsgNum = uMsgNum;
	pEntry->hEvent = hEvent;
	pEntry->tEdited = 0;
	time(&pEntry->t);
	if (!bFound) return List_InsertElementAt (m_hMsgList, pEntry, iListInd)?pEntry:NULL;
	return pEntry;
}


TYP_MSGLENTRY *MsgList_FindMessage(DWORD uMsgNum)
{
	TYP_MSGLENTRY *pEntry;
	int iPos;

	LOG (("MsgList_FindEvent (%d)", uMsgNum));
	if (m_hMsgList && List_BinarySearch(m_hMsgList, CmpProc, (void*)uMsgNum, &iPos))
	{
		pEntry = (TYP_MSGLENTRY*)List_ElementAt (m_hMsgList, iPos);
		time(&pEntry->t);	// Touch it, so that it doesn't get thrown away too soon
		LOG (("MsgList_FindEvent(%d): %08X", uMsgNum, pEntry->hEvent));
		return pEntry;
	}
	return NULL;
}

void MsgList_CollectGarbage(void)
{
	unsigned int i;
	TYP_MSGLENTRY *pEntry;
	time_t t;

	if (!m_hMsgList) return;
	time(&t);
	t-=MSGLIST_TIMEOUT;
	for (i=0; i<List_Count(m_hMsgList); i++)
	{
		pEntry = (TYP_MSGLENTRY*)List_ElementAt (m_hMsgList, i);
		if (pEntry->t < t)
		{
			LOG (("MsgList_CollectGarbage throwing out msg %d", pEntry->uMsgNum));
			HeapFree (GetProcessHeap(), 0, List_RemoveElementAt (m_hMsgList, i));
			i--;
		}
	}
}

