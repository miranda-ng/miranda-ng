#if !defined(AFX_LIST_MT__H__INCLUDED_)
#define AFX_LIST_MT__H__INCLUDED_

// структура для работы со списком, элемент списка
typedef struct _LIST_MT_ITEM
{
#ifdef __cplusplus
	_LIST_MT_ITEM	*plmtiPrev;	// *предыдущий элемент в списке
	_LIST_MT_ITEM	*plmtiNext;	// *следующий элемент в списке
#else
	LPVOID			*plmtiPrev;	// *предыдущий элемент в списке
	LPVOID			*plmtiNext;	// *следующий элемент в списке
#endif
	LPVOID			lpData;		// указатель на данные, связанные с элементом списка
	LPVOID			lpListMT;	// указатель на заголовок списка, см структуру ниже
}LIST_MT_ITEM, *PLIST_MT_ITEM, *LPLIST_MT_ITEM;
typedef CONST PLIST_MT_ITEM	PCLIST_MT_ITEM, LPCLIST_MT_ITEM;


// структура для работы со списком, заголовок списка
typedef struct _LIST_MT
{
	size_t				nCount;			// *колличество элементов в списке
	PLIST_MT_ITEM		plmtiFirst;		// *указывает на первый элемент в списке
	PLIST_MT_ITEM		plmtiLast;		// *указывает на последний элемент в списке
	CRITICAL_SECTION	cs;				// *section for exclysive access to List
}LIST_MT, *PLIST_MT, *LPLIST_MT;
typedef CONST PLIST_MT	PCLIST_MT, LPCLIST_MT;


// структура для работы с итератором
typedef struct _LIST_MT_ITERATOR
{
	PLIST_MT_ITEM plmtListMTItem;
}LIST_MT_ITERATOR, *PLIST_MT_ITERATOR, *LPLIST_MT_ITERATOR;
//typedef LIST_MT_ITEM			LIST_MT_ITERATOR, *PLIST_MT_ITERATOR, *LPLIST_MT_ITERATOR;
typedef CONST PLIST_MT_ITERATOR	PCLIST_MT_ITERATOR, LPCLIST_MT_ITERATOR;




__inline DWORD ListMTInitialize(PCLIST_MT pclmtListMT,DWORD dwSpinCount)
{
	DWORD dwRetErrorCode;

#if (_WIN32_WINNT >= 0x0403)
	if (InitializeCriticalSectionAndSpinCount(&pclmtListMT->cs,((dwSpinCount)? (dwSpinCount | 0x80000000):0L)))
#else
	InitializeCriticalSection(&pclmtListMT->cs);
	if (TRUE)
#endif
	{
		InterlockedExchangePointer((volatile PVOID*)&pclmtListMT->nCount,NULL);
		pclmtListMT->plmtiFirst=NULL;
		pclmtListMT->plmtiLast=NULL;
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=GetLastError();
	}
return(dwRetErrorCode);
}


__inline void ListMTDestroy(PCLIST_MT pclmtListMT)
{
	InterlockedExchangePointer((volatile PVOID*)&pclmtListMT->nCount,NULL);
	pclmtListMT->plmtiFirst=NULL;
	pclmtListMT->plmtiLast=NULL;
	DeleteCriticalSection(&pclmtListMT->cs);
}


__inline BOOL ListMTTryLock(PCLIST_MT pclmtListMT)
{
#if (_WIN32_WINNT >= 0x0400)
	return(TryEnterCriticalSection(&pclmtListMT->cs));
#else
	return(FALSE);
#endif
}


__inline void ListMTLock(PCLIST_MT pclmtListMT)
{
	EnterCriticalSection(&pclmtListMT->cs);
}


__inline void ListMTUnLock(PCLIST_MT pclmtListMT)
{
	LeaveCriticalSection(&pclmtListMT->cs);
}


__inline size_t ListMTGetCount(PCLIST_MT pclmtListMT)
{
	return((size_t)InterlockedCompareExchangePointer((LPVOID*)&pclmtListMT->nCount,NULL,NULL));
}


__inline size_t ListMTItemAdd(PCLIST_MT pclmtListMT,PCLIST_MT_ITEM pclmtListMTItem,LPVOID lpData)
{
	size_t dwRet=(size_t)InterlockedIncrementPointer(&pclmtListMT->nCount);//pclmtListMT->nCount++;
	pclmtListMTItem->lpData=lpData;
	pclmtListMTItem->lpListMT=pclmtListMT;

    if (pclmtListMT->plmtiLast) 
	{// add node to end of list        
        pclmtListMTItem->plmtiPrev=pclmtListMT->plmtiLast;
        pclmtListMTItem->plmtiNext=NULL;
        pclmtListMT->plmtiLast->plmtiNext=pclmtListMTItem;
		pclmtListMT->plmtiLast=pclmtListMTItem;
	}else{// add the first node to the linked list
        pclmtListMTItem->plmtiPrev=NULL;
        pclmtListMTItem->plmtiNext=NULL;
		pclmtListMT->plmtiFirst=pclmtListMTItem;
        pclmtListMT->plmtiLast=pclmtListMTItem;
 	}

return(dwRet);
}


__inline DWORD ListMTItemDelete(PCLIST_MT pclmtListMT,PCLIST_MT_ITEM pclmtListMTItem)
{
	DWORD dwRetErrorCode;

	if (pclmtListMTItem->lpListMT==pclmtListMT && pclmtListMT)
	{// Данный элемент принадлежит к этому списку, можно удалить.
		PLIST_MT_ITEM plmtiPrevNode=pclmtListMTItem->plmtiPrev,plmtiNextNode=pclmtListMTItem->plmtiNext;
		
		if (plmtiPrevNode || plmtiNextNode) 
		{
			if (plmtiPrevNode && plmtiNextNode==NULL) 
			{// This is the start node in the list to delete
			// отключился последний подключившийся
				plmtiPrevNode->plmtiNext=NULL;
				pclmtListMT->plmtiLast=plmtiPrevNode;
			}else{
				if (plmtiPrevNode==NULL && plmtiNextNode) 
				{// This is the end node in the list to delete
				// отключился первый подключившийся
					plmtiNextNode->plmtiPrev=NULL;
					pclmtListMT->plmtiFirst=plmtiNextNode;
				}else{// оключился клиент не первый и не последний
					//if (plmtiPrev && plmtiNext) 
					{// Neither start node nor end node in the list
						plmtiPrevNode->plmtiNext=plmtiNextNode;
						plmtiNextNode->plmtiPrev=plmtiPrevNode;
					}
				}
			}
		}else{// This is the only node in the list to delete
			pclmtListMT->plmtiFirst=NULL;
			pclmtListMT->plmtiLast=NULL;
		}

		pclmtListMTItem->lpListMT=NULL;
		InterlockedDecrementPointer(&pclmtListMT->nCount);// pclmtListMT->nCount--;	
		dwRetErrorCode=NO_ERROR;
	}else{// попытались удалить элемент не относящийся к данному списку
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


__inline LPVOID ListMTItemDataGet(PCLIST_MT_ITEM pclmtListMTItem)
{
	return(pclmtListMTItem->lpData);
}


__inline void ListMTItemDataSet(PCLIST_MT_ITEM pclmtListMTItem, LPVOID lpData)
{
	pclmtListMTItem->lpData=lpData;
}


__inline DWORD ListMTItemGetFirst(PCLIST_MT pclmtListMT,PLIST_MT_ITEM *pplmtListMTItem,LPVOID *plpData)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	DWORD dwRetErrorCode;

	if (pclmtListMT->plmtiFirst)
	{
		if (pplmtListMTItem)	(*pplmtListMTItem)=pclmtListMT->plmtiFirst;
		if (plpData)			(*plpData)=pclmtListMT->plmtiFirst->lpData;
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_NO_MORE_ITEMS;
	}
return(dwRetErrorCode);
}


__inline DWORD ListMTItemGetLast(PCLIST_MT pclmtListMT,PLIST_MT_ITEM *pplmtListMTItem,LPVOID *plpData)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	DWORD dwRetErrorCode;

	if (pclmtListMT->plmtiLast)
	{
		if (pplmtListMTItem)	(*pplmtListMTItem)=pclmtListMT->plmtiLast;
		if (plpData)			(*plpData)=pclmtListMT->plmtiLast->lpData;
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_NO_MORE_ITEMS;
	}
return(dwRetErrorCode);
}



__inline void ListMTItemSwap(PCLIST_MT pclmtListMT,PCLIST_MT_ITEM pclmtListMTItem1,PCLIST_MT_ITEM pclmtListMTItem2)
{// поменять два элемента списка местами, даже если они из разных списков

	if (pclmtListMTItem1!=pclmtListMTItem2)
	{// это разные элементы списка
		PLIST_MT_ITEM lpTemp;

		lpTemp=pclmtListMTItem1->plmtiPrev;
		if ((pclmtListMTItem1->plmtiPrev=pclmtListMTItem2->plmtiPrev)==NULL)
		{// pclmtListMTItem2 был первым, обновляем заголвок листа, теперь первый pclmtListMTItem1
			pclmtListMT->plmtiFirst=pclmtListMTItem1;
		}

		if ((pclmtListMTItem2->plmtiPrev=lpTemp)==NULL)
		{// pclmtListMTItem1 был первым, обновляем заголвок листа, теперь первый pclmtListMTItem2
			pclmtListMT->plmtiFirst=pclmtListMTItem2;
		}


		lpTemp=pclmtListMTItem1->plmtiNext;
		if ((pclmtListMTItem1->plmtiNext=pclmtListMTItem2->plmtiNext)==NULL)
		{// pclmtListMTItem2 был последним, обновляем заголвок листа, теперь последний pclmtListMTItem1
			pclmtListMT->plmtiLast=pclmtListMTItem1;
		}

		if ((pclmtListMTItem2->plmtiNext=lpTemp)==NULL)
		{// pclmtListMTItem1 был последним, обновляем заголвок листа, теперь последний pclmtListMTItem2
			pclmtListMT->plmtiLast=pclmtListMTItem2;
		}
	}
}


__inline BOOL ListMTIteratorMoveFirst(PCLIST_MT pclmtListMT,PCLIST_MT_ITERATOR pclmtiIterator)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	return((pclmtiIterator->plmtListMTItem=pclmtListMT->plmtiFirst)? TRUE:FALSE);
}


__inline BOOL ListMTIteratorMoveLast(PCLIST_MT pclmtListMT,PCLIST_MT_ITERATOR pclmtiIterator)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	return((pclmtiIterator->plmtListMTItem=pclmtListMT->plmtiLast)? TRUE:FALSE);
}


__inline BOOL ListMTIteratorMovePrev(PCLIST_MT_ITERATOR pclmtiIterator)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	BOOL bRet=FALSE;

	if (pclmtiIterator->plmtListMTItem)
	{
		if (pclmtiIterator->plmtListMTItem=pclmtiIterator->plmtListMTItem->plmtiPrev) bRet=TRUE;
	}
return(bRet);
}


__inline BOOL ListMTIteratorMoveNext(PCLIST_MT_ITERATOR pclmtiIterator)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	BOOL bRet=FALSE;

	if (pclmtiIterator->plmtListMTItem) 
	{
		if (pclmtiIterator->plmtListMTItem=pclmtiIterator->plmtListMTItem->plmtiNext) bRet=TRUE;
	}
return(bRet);
}


__inline DWORD ListMTIteratorGet(PCLIST_MT_ITERATOR pclmtiIterator,PLIST_MT_ITEM *pplmtListMTItem,LPVOID *plpData)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	DWORD dwRetErrorCode;

	if (pclmtiIterator->plmtListMTItem)
	{
		if (pplmtListMTItem)	(*pplmtListMTItem)=pclmtiIterator->plmtListMTItem;
		if (plpData)			(*plpData)=pclmtiIterator->plmtListMTItem->lpData;
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_NO_MORE_ITEMS;
	}
return(dwRetErrorCode);
}

#endif // !defined(AFX_LIST_MT__H__INCLUDED_)
