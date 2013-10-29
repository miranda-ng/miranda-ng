// The omnipresent memory list, WIN32 implementation, thread safe
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "memlist.h"

#pragma warning (disable: 4706) // assignment within conditional expression

struct _tagLIST
{
    unsigned int		uiCount;
    unsigned int		uiCapacity;
    void				**apStorage;
	HANDLE				hHeap;
	CRITICAL_SECTION	cs;
};


TYP_LIST *List_Init(unsigned int uiCapacity)
{
	HANDLE hHeap = GetProcessHeap();
    TYP_LIST *pstHandle;

	pstHandle = (TYP_LIST *)HeapAlloc(hHeap, 0, sizeof(TYP_LIST));
    if (!pstHandle) return NULL;
    pstHandle->uiCount = 0;
    pstHandle->uiCapacity = uiCapacity;
    if (uiCapacity == 0)
        pstHandle->apStorage = NULL;
    else
    {
        pstHandle->apStorage = (void **)HeapAlloc(hHeap, 0, sizeof(void *)*uiCapacity);
        if (!pstHandle->apStorage)
		{
			HeapFree (hHeap, 0, pstHandle);
			return NULL;
		}
    }
	pstHandle->hHeap = hHeap;
	InitializeCriticalSection (&pstHandle->cs);
    return pstHandle;
}

void List_Exit(TYP_LIST *pstHandle)
{
    if (pstHandle->apStorage)
        HeapFree (pstHandle->hHeap, 0, pstHandle->apStorage);
	DeleteCriticalSection (&pstHandle->cs);
    HeapFree (pstHandle->hHeap, 0, pstHandle);
}

BOOL List_Push(TYP_LIST *pstHandle, void *pItem)
{
    return List_InsertElementAt(pstHandle, pItem,pstHandle->uiCount);
}

void *List_Pop (TYP_LIST *pstHandle)
{
	if (pstHandle->uiCount)
		return List_RemoveElementAt(pstHandle ,pstHandle->uiCount-1);
	else return NULL;
}

BOOL List_InsertElementAt(TYP_LIST *pstHandle, void *pItem, unsigned int uiPos)
{
    unsigned int    uiStep;
    void            **apNewStorage;

	EnterCriticalSection (&pstHandle->cs);
    if (uiPos > pstHandle->uiCount)
        uiPos = pstHandle->uiCount;

    if (pstHandle->uiCount >= pstHandle->uiCapacity)
    {
        uiStep = pstHandle->uiCount*2;
        if (uiStep < 8) uiStep = 8;

		if (!pstHandle->apStorage)
			apNewStorage = (void **)HeapAlloc(pstHandle->hHeap, 0, sizeof(void *)*uiStep);
		else
			apNewStorage = (void **)HeapReAlloc (pstHandle->hHeap, 0, pstHandle->apStorage, sizeof(void *)*uiStep);
		if (!apNewStorage)
		{
			LeaveCriticalSection (&pstHandle->cs);
			return FALSE;
		}
		pstHandle->apStorage = apNewStorage;
		pstHandle->uiCapacity = uiStep;
	}

	if (uiPos<pstHandle->uiCount)
		MoveMemory (&pstHandle->apStorage[uiPos+1], &pstHandle->apStorage[uiPos], (pstHandle->uiCount-uiPos)*sizeof(void*));
	pstHandle->apStorage[uiPos] = pItem;
    pstHandle->uiCount++;
	LeaveCriticalSection (&pstHandle->cs);
	return TRUE;
}

void *List_RemoveElementAt(TYP_LIST *pstHandle, unsigned int uiPos)
{
    void            *pRet;

	EnterCriticalSection (&pstHandle->cs);
    pRet = pstHandle->apStorage[uiPos];
	if (uiPos<pstHandle->uiCount)
		MoveMemory (&pstHandle->apStorage[uiPos], &pstHandle->apStorage[uiPos+1], (pstHandle->uiCount-uiPos)*sizeof(void*));
    pstHandle->uiCount--;
	LeaveCriticalSection (&pstHandle->cs);
    return pRet;
}

unsigned int List_Count(TYP_LIST *pstHandle)
{
	return pstHandle->uiCount;
}

void *List_ElementAt(TYP_LIST *pstHandle,unsigned int uiPos)
{
	void *pRet = NULL;

	EnterCriticalSection (&pstHandle->cs);
    if (uiPos < pstHandle->uiCount) 
		pRet = pstHandle->apStorage[uiPos];
	LeaveCriticalSection (&pstHandle->cs);
	return pRet;
}

void *List_Top(TYP_LIST *pstHandle)
{
	if (pstHandle->uiCount)
		return List_ElementAt (pstHandle, pstHandle->uiCount-1);
	else return NULL;
}

#ifdef _INC_STDLIB
void List_Sort(TYP_LIST *pstHandle, int (*pFunc)(const void*,const void*))
{
	EnterCriticalSection (&pstHandle->cs);
	qsort(pstHandle->apStorage,pstHandle->uiCount,sizeof(void *),pFunc);
	LeaveCriticalSection (&pstHandle->cs);
}
#endif


void List_FreeElements(TYP_LIST *pstHandle)
{
	void *pEntry;
	HANDLE hHeap = GetProcessHeap();

	while (pEntry = List_Pop(pstHandle))
		HeapFree (hHeap, 0, pEntry);
}

BOOL List_BinarySearch(TYP_LIST *hPList,
	int (*pPFunc)(const void *pstPElement,const void *pstPToFind),
	const void *pstPToFind,int *piPToInsert)
{
	unsigned int
			iStart,
			iEnd,
			iInd;
	int		iRetCompare;

	if (hPList == NULL)
	{
		*piPToInsert = 0;
		return FALSE;
	}

	iStart = 0;
	if (hPList->uiCount == 0)
	{
		*piPToInsert = 0;
		return FALSE;
	}
	EnterCriticalSection (&hPList->cs);
	iEnd = hPList->uiCount-1;

	iRetCompare = (*pPFunc)(hPList->apStorage[0],pstPToFind);
	if (iRetCompare >= 0)
	{
		*piPToInsert = 0;
		LeaveCriticalSection (&hPList->cs);
		return iRetCompare == 0;
	}

	iRetCompare = (*pPFunc)(hPList->apStorage[iEnd],pstPToFind);
	if (iRetCompare < 0)
	{
		*piPToInsert = hPList->uiCount;
		LeaveCriticalSection (&hPList->cs);
		return FALSE;
	}
	else if (iRetCompare == 0)
	{
		*piPToInsert = hPList->uiCount-1;
		LeaveCriticalSection (&hPList->cs);
		return TRUE;
	}

	// Otherwise C4702: unreachable code below
	#pragma warning (suppress: 4127) // conditional expression is constant
	while(1)
	{
		switch (iEnd-iStart)
		{
		case 0:
			*piPToInsert = iStart;
			LeaveCriticalSection (&hPList->cs);
			return FALSE;
		case 1:
			*piPToInsert = iStart+1;
			LeaveCriticalSection (&hPList->cs);
			return FALSE;
		default:
			iInd = iStart + (iEnd-iStart)/2;
			iRetCompare = (*pPFunc)(hPList->apStorage[iInd],pstPToFind);
			if (iRetCompare == 0)
			{
				*piPToInsert = iInd;
				LeaveCriticalSection (&hPList->cs);
				return TRUE;
			}
			if (iRetCompare < 0)
				iStart = iInd;
			else
				iEnd = iInd;
			break;
		}
	}

	LeaveCriticalSection (&hPList->cs);
	return FALSE;
}

BOOL	List_InsertSort(TYP_LIST *hPList,
	int (*pPFunc)(const void *pstPElement,const void *pstPToFind),
	void *pItem)
{
	int iListInd;

	if (!List_BinarySearch(hPList,pPFunc,(void *)pItem,&iListInd))
	{
		return List_InsertElementAt (hPList, pItem, iListInd);
	}
	return FALSE;
}


