#include "Mra.h"
#include "MraSendQueue.h"




typedef struct
{
	LIST_MT			lmtListMT;
	DWORD			dwSendTimeOutInterval;
} MRA_SEND_QUEUE;


typedef struct
{
	// internal
	LIST_MT_ITEM	lmtListMTItem;
	FILETIME		ftSendTime;
	// external
	DWORD			dwCMDNum;
	DWORD			dwFlags;
	HANDLE			hContact;
	DWORD			dwAckType;
	LPBYTE			lpbData;
	SIZE_T			dwDataSize;
} MRA_SEND_QUEUE_ITEM;



#define FILETIME_SECOND			((DWORDLONG)10000000)




DWORD MraSendQueueInitialize(DWORD dwSendTimeOutInterval,HANDLE *phSendQueueHandle)
{
	DWORD dwRetErrorCode;

	if (phSendQueueHandle)
	{
		MRA_SEND_QUEUE *pmrasqSendQueue;

		pmrasqSendQueue=(MRA_SEND_QUEUE*)MEMALLOC(sizeof(MRA_SEND_QUEUE));
		if (pmrasqSendQueue)
		{
			dwRetErrorCode=ListMTInitialize(&pmrasqSendQueue->lmtListMT,0);
			if (dwRetErrorCode==NO_ERROR)
			{
				pmrasqSendQueue->dwSendTimeOutInterval=dwSendTimeOutInterval;
				(*phSendQueueHandle)=(HANDLE)pmrasqSendQueue;
			}
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraSendQueueDestroy(HANDLE hSendQueueHandle)
{
	if (hSendQueueHandle)
	{
		MRA_SEND_QUEUE *pmrasqSendQueue=(MRA_SEND_QUEUE*)hSendQueueHandle;
		MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;

		ListMTLock(&pmrasqSendQueue->lmtListMT);
		while(ListMTItemGetFirst(&pmrasqSendQueue->lmtListMT,NULL,(LPVOID*)&pmrasqiSendQueueItem)==NO_ERROR)
		{
			ListMTItemDelete(&pmrasqSendQueue->lmtListMT,&pmrasqiSendQueueItem->lmtListMTItem);
			//MEMFREE(pmrasqiSendQueueItem->lpbData);
			MEMFREE(pmrasqiSendQueueItem);
		}
		ListMTUnLock(&pmrasqSendQueue->lmtListMT);

		ListMTDestroy(&pmrasqSendQueue->lmtListMT);
		MEMFREE(pmrasqSendQueue);
	}
}


DWORD MraSendQueueAdd(HANDLE hSendQueueHandle,DWORD dwCMDNum,DWORD dwFlags,HANDLE hContact,DWORD dwAckType,LPBYTE lpbData,SIZE_T dwDataSize)
{
	DWORD dwRetErrorCode;

	if (hSendQueueHandle && dwCMDNum)
	{
		MRA_SEND_QUEUE *pmrasqSendQueue=(MRA_SEND_QUEUE*)hSendQueueHandle;
		MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;

		pmrasqiSendQueueItem=(MRA_SEND_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_SEND_QUEUE_ITEM));
		if (pmrasqiSendQueueItem)
		{
			//pmrasqiSendQueueItem->lmtListMTItem;
			GetSystemTimeAsFileTime(&pmrasqiSendQueueItem->ftSendTime);
			pmrasqiSendQueueItem->dwCMDNum=dwCMDNum;
			pmrasqiSendQueueItem->dwFlags=dwFlags;
			pmrasqiSendQueueItem->hContact=hContact;
			pmrasqiSendQueueItem->dwAckType=dwAckType;
			pmrasqiSendQueueItem->lpbData=lpbData;
			pmrasqiSendQueueItem->dwDataSize=dwDataSize;

			ListMTLock(&pmrasqSendQueue->lmtListMT);
			ListMTItemAdd(&pmrasqSendQueue->lmtListMT,&pmrasqiSendQueueItem->lmtListMTItem,pmrasqiSendQueueItem);
			ListMTUnLock(&pmrasqSendQueue->lmtListMT);
			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraSendQueueFree(HANDLE hSendQueueHandle,DWORD dwCMDNum)
{
	DWORD dwRetErrorCode;

	if (hSendQueueHandle)
	{
		MRA_SEND_QUEUE *pmrasqSendQueue=(MRA_SEND_QUEUE*)hSendQueueHandle;
		MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
		LIST_MT_ITERATOR lmtiIterator;

		dwRetErrorCode=ERROR_NOT_FOUND;
		ListMTLock(&pmrasqSendQueue->lmtListMT);
		ListMTIteratorMoveFirst(&pmrasqSendQueue->lmtListMT,&lmtiIterator);
		do
		{// цикл
			if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&pmrasqiSendQueueItem)==NO_ERROR)
			if (pmrasqiSendQueueItem->dwCMDNum==dwCMDNum)
			{
				ListMTItemDelete(&pmrasqSendQueue->lmtListMT,&pmrasqiSendQueueItem->lmtListMTItem);
				//MEMFREE(pmrasqiSendQueueItem->lpbData);
				MEMFREE(pmrasqiSendQueueItem);
				dwRetErrorCode=NO_ERROR;
				break;
			}
		}while(ListMTIteratorMoveNext(&lmtiIterator));
		ListMTUnLock(&pmrasqSendQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraSendQueueFind(HANDLE hSendQueueHandle,DWORD dwCMDNum,DWORD *pdwFlags,HANDLE *phContact,DWORD *pdwAckType,LPBYTE *plpbData,SIZE_T *pdwDataSize)
{
	DWORD dwRetErrorCode;

	if (hSendQueueHandle)
	{
		MRA_SEND_QUEUE *pmrasqSendQueue=(MRA_SEND_QUEUE*)hSendQueueHandle;
		MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
		LIST_MT_ITERATOR lmtiIterator;

		dwRetErrorCode=ERROR_NOT_FOUND;
		ListMTLock(&pmrasqSendQueue->lmtListMT);
		ListMTIteratorMoveFirst(&pmrasqSendQueue->lmtListMT,&lmtiIterator);
		do
		{// цикл
			if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&pmrasqiSendQueueItem)==NO_ERROR)
			if (pmrasqiSendQueueItem->dwCMDNum==dwCMDNum)
			{
				if (pdwFlags)		(*pdwFlags)=pmrasqiSendQueueItem->dwFlags;
				if (phContact)		(*phContact)=pmrasqiSendQueueItem->hContact;
				if (pdwAckType)		(*pdwAckType)=pmrasqiSendQueueItem->dwAckType;
				if (plpbData)		(*plpbData)=pmrasqiSendQueueItem->lpbData;
				if (pdwDataSize)	(*pdwDataSize)=pmrasqiSendQueueItem->dwDataSize;
				dwRetErrorCode=NO_ERROR;
				break;
			}
		}while(ListMTIteratorMoveNext(&lmtiIterator));
		ListMTUnLock(&pmrasqSendQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraSendQueueFindOlderThan(HANDLE hSendQueueHandle,DWORD dwTime,DWORD *pdwCMDNum,DWORD *pdwFlags,HANDLE *phContact,DWORD *pdwAckType,LPBYTE *plpbData,SIZE_T *pdwDataSize)
{
	DWORD dwRetErrorCode;

	if (hSendQueueHandle)
	{
		FILETIME ftExpireTime;
		MRA_SEND_QUEUE *pmrasqSendQueue=(MRA_SEND_QUEUE*)hSendQueueHandle;
		MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
		LIST_MT_ITERATOR lmtiIterator;

		GetSystemTimeAsFileTime(&ftExpireTime);
		(*((DWORDLONG*)&ftExpireTime))-=((DWORDLONG)dwTime*FILETIME_SECOND);
		dwRetErrorCode=ERROR_NOT_FOUND;
		ListMTLock(&pmrasqSendQueue->lmtListMT);
		ListMTIteratorMoveFirst(&pmrasqSendQueue->lmtListMT,&lmtiIterator);
		do
		{// цикл
			if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&pmrasqiSendQueueItem)==NO_ERROR)
			if ((*((DWORDLONG*)&ftExpireTime))>(*((DWORDLONG*)&pmrasqiSendQueueItem->ftSendTime)))
			{
				if (pdwCMDNum)		(*pdwCMDNum)=pmrasqiSendQueueItem->dwCMDNum;
				if (pdwFlags)		(*pdwFlags)=pmrasqiSendQueueItem->dwFlags;
				if (phContact)		(*phContact)=pmrasqiSendQueueItem->hContact;
				if (pdwAckType)		(*pdwAckType)=pmrasqiSendQueueItem->dwAckType;
				if (plpbData)		(*plpbData)=pmrasqiSendQueueItem->lpbData;
				if (pdwDataSize)	(*pdwDataSize)=pmrasqiSendQueueItem->dwDataSize;
				dwRetErrorCode=NO_ERROR;
				break;
			}
		}while(ListMTIteratorMoveNext(&lmtiIterator));
		ListMTUnLock(&pmrasqSendQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


