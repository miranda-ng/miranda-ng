/*
 * Copyright (c) 2003 Rozhuk Ivan <rozhuk.im@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */



#if !defined(AFX_LIST_MT__H__INCLUDED_)
#define AFX_LIST_MT__H__INCLUDED_

#pragma once

#include "Sdk/InterlockedFunctions.h"

#pragma warning(push)
#pragma warning(disable:4312) // warning C4312: 'type cast' : conversion from 'LONG' to 'PVOID' of greater size


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
	LPVOID			lpListMT;	// указатель на заголовок списка, см структуру ниже
	LPVOID			lpData;		// указатель на данные, связанные с элементом списка
}LIST_MT_ITEM, *PLIST_MT_ITEM, *LPLIST_MT_ITEM;
typedef CONST PLIST_MT_ITEM	PCLIST_MT_ITEM, LPCLIST_MT_ITEM;


// структура для работы со списком, заголовок списка
typedef struct _LIST_MT
{
	size_t        nCount;     // *количество элементов в списке
	PLIST_MT_ITEM plmtiFirst; // *указывает на первый элемент в списке
	PLIST_MT_ITEM plmtiLast;  // *указывает на последний элемент в списке
	mir_cs        cs;         // *section for exclysive access to List
}LIST_MT, *PLIST_MT, *LPLIST_MT;
typedef CONST PLIST_MT	PCLIST_MT, LPCLIST_MT;


// структура для работы с итератором
typedef struct _LIST_MT_ITERATOR
{
	PLIST_MT_ITEM plmtListMTItem;
}LIST_MT_ITERATOR, *PLIST_MT_ITERATOR, *LPLIST_MT_ITERATOR;
//typedef LIST_MT_ITEM			LIST_MT_ITERATOR, *PLIST_MT_ITERATOR, *LPLIST_MT_ITERATOR;
typedef CONST PLIST_MT_ITERATOR	PCLIST_MT_ITERATOR, LPCLIST_MT_ITERATOR;




__inline DWORD ListMTInitialize(PCLIST_MT pclmtListMT)
{
	InterlockedExchangePointer((volatile PVOID*)&pclmtListMT->nCount,NULL);
	pclmtListMT->plmtiFirst=NULL;
	pclmtListMT->plmtiLast=NULL;
	return NO_ERROR;
}


__inline void ListMTDestroy(PCLIST_MT pclmtListMT)
{
	InterlockedExchangePointer((volatile PVOID*)&pclmtListMT->nCount,NULL);
	pclmtListMT->plmtiFirst=NULL;
	pclmtListMT->plmtiLast=NULL;
	SecureZeroMemory(&pclmtListMT->cs,sizeof(CRITICAL_SECTION));
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

__inline BOOL ListMTIteratorMoveFirst(PCLIST_MT pclmtListMT,PCLIST_MT_ITERATOR pclmtiIterator)
{// если нужно гарантировать эксклюзивный доступ, то есть ListMTLock и ListMTUnLock
	return((pclmtiIterator->plmtListMTItem=pclmtListMT->plmtiFirst)? TRUE:FALSE);
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

#pragma warning(pop)
#pragma warning(default:4312) // warning C4312: 'type cast' : conversion from 'LONG' to 'PVOID' of greater size

#endif // !defined(AFX_LIST_MT__H__INCLUDED_)
