#include "Mra.h"
#include "MraSendQueue.h"

struct MRA_SEND_QUEUE : public LIST_MT
{
	DWORD dwSendTimeOutInterval;
};

struct MRA_SEND_QUEUE_ITEM : public LIST_MT_ITEM
{
	// internal
	FILETIME ftSendTime;

	// external
	DWORD    dwCMDNum;
	DWORD    dwFlags;
	MCONTACT hContact;
	DWORD    dwAckType;
	LPBYTE   lpbData;
	size_t   dwDataSize;
};

#define FILETIME_SECOND			((DWORDLONG)10000000)

DWORD MraSendQueueInitialize(DWORD dwSendTimeOutInterval, HANDLE *phSendQueueHandle)
{
	if (!phSendQueueHandle)
		return ERROR_INVALID_HANDLE;

	MRA_SEND_QUEUE *pmrasqSendQueue = new MRA_SEND_QUEUE();

	DWORD dwRetErrorCode = ListMTInitialize(pmrasqSendQueue);
	if (dwRetErrorCode == NO_ERROR) {
		pmrasqSendQueue->dwSendTimeOutInterval = dwSendTimeOutInterval;
		*phSendQueueHandle = (HANDLE)pmrasqSendQueue;
	}
	return dwRetErrorCode;
}

void MraSendQueueDestroy(HANDLE hSendQueueHandle)
{
	if (!hSendQueueHandle)
		return;

	MRA_SEND_QUEUE *pmrasqSendQueue = (MRA_SEND_QUEUE*)hSendQueueHandle;
	MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
	{
		mir_cslock l(pmrasqSendQueue->cs);
		while ( !ListMTItemGetFirst(pmrasqSendQueue, NULL, (LPVOID*)&pmrasqiSendQueueItem)) {
			ListMTItemDelete(pmrasqSendQueue, pmrasqiSendQueueItem);
			mir_free(pmrasqiSendQueueItem);
		}
	}

	ListMTDestroy(pmrasqSendQueue);
	delete pmrasqSendQueue;
}


DWORD MraSendQueueAdd(HANDLE hSendQueueHandle, DWORD dwCMDNum, DWORD dwFlags, MCONTACT hContact, DWORD dwAckType, LPBYTE lpbData, size_t dwDataSize)
{
	if (!hSendQueueHandle || !dwCMDNum)
		return ERROR_INVALID_HANDLE;

	MRA_SEND_QUEUE *pmrasqSendQueue = (MRA_SEND_QUEUE*)hSendQueueHandle;
	MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;

	pmrasqiSendQueueItem = (MRA_SEND_QUEUE_ITEM*)mir_calloc(sizeof(MRA_SEND_QUEUE_ITEM));
	if (!pmrasqiSendQueueItem)
		return GetLastError();

	GetSystemTimeAsFileTime(&pmrasqiSendQueueItem->ftSendTime);
	pmrasqiSendQueueItem->dwCMDNum = dwCMDNum;
	pmrasqiSendQueueItem->dwFlags = dwFlags;
	pmrasqiSendQueueItem->hContact = hContact;
	pmrasqiSendQueueItem->dwAckType = dwAckType;
	pmrasqiSendQueueItem->lpbData = lpbData;
	pmrasqiSendQueueItem->dwDataSize = dwDataSize;

	mir_cslock l(pmrasqSendQueue->cs);
	ListMTItemAdd(pmrasqSendQueue, pmrasqiSendQueueItem, pmrasqiSendQueueItem);
	return 0;
}

DWORD MraSendQueueFree(HANDLE hSendQueueHandle, DWORD dwCMDNum)
{
	if (!hSendQueueHandle)
		return ERROR_INVALID_HANDLE;

	MRA_SEND_QUEUE *pmrasqSendQueue = (MRA_SEND_QUEUE*)hSendQueueHandle;
	MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
	LIST_MT_ITERATOR lmtiIterator;

	mir_cslock l(pmrasqSendQueue->cs);
	ListMTIteratorMoveFirst(pmrasqSendQueue, &lmtiIterator);
	do {
		if ( !ListMTIteratorGet(&lmtiIterator, NULL, (LPVOID*)&pmrasqiSendQueueItem))
		if (pmrasqiSendQueueItem->dwCMDNum == dwCMDNum) {
			ListMTItemDelete(pmrasqSendQueue, pmrasqiSendQueueItem);
			mir_free(pmrasqiSendQueueItem);
			return 0;
		}
	}
		while (ListMTIteratorMoveNext(&lmtiIterator));

	return ERROR_NOT_FOUND;
}

DWORD MraSendQueueFind(HANDLE hSendQueueHandle, DWORD dwCMDNum, DWORD *pdwFlags, MCONTACT *phContact, DWORD *pdwAckType, LPBYTE *plpbData, size_t *pdwDataSize)
{
	if (!hSendQueueHandle)
		return ERROR_INVALID_HANDLE;

	MRA_SEND_QUEUE *pmrasqSendQueue = (MRA_SEND_QUEUE*)hSendQueueHandle;
	MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
	LIST_MT_ITERATOR lmtiIterator;

	mir_cslock l(pmrasqSendQueue->cs);
	ListMTIteratorMoveFirst(pmrasqSendQueue, &lmtiIterator);
	do {
		if ( !ListMTIteratorGet(&lmtiIterator, NULL, (LPVOID*)&pmrasqiSendQueueItem))
		if (pmrasqiSendQueueItem->dwCMDNum == dwCMDNum) {
			if (pdwFlags)    (*pdwFlags) = pmrasqiSendQueueItem->dwFlags;
			if (phContact)   (*phContact) = pmrasqiSendQueueItem->hContact;
			if (pdwAckType)  (*pdwAckType) = pmrasqiSendQueueItem->dwAckType;
			if (plpbData)    (*plpbData) = pmrasqiSendQueueItem->lpbData;
			if (pdwDataSize) (*pdwDataSize) = pmrasqiSendQueueItem->dwDataSize;
			return 0;
		}
	}
		while (ListMTIteratorMoveNext(&lmtiIterator));

	return ERROR_NOT_FOUND;
}

DWORD MraSendQueueFindOlderThan(HANDLE hSendQueueHandle, DWORD dwTime, DWORD *pdwCMDNum, DWORD *pdwFlags, MCONTACT *phContact, DWORD *pdwAckType, LPBYTE *plpbData, size_t *pdwDataSize)
{
	if (!hSendQueueHandle)
		return ERROR_INVALID_HANDLE;

	FILETIME ftExpireTime;
	GetSystemTimeAsFileTime(&ftExpireTime);
	(*((DWORDLONG*)&ftExpireTime))-=((DWORDLONG)dwTime*FILETIME_SECOND);

	MRA_SEND_QUEUE *pmrasqSendQueue = (MRA_SEND_QUEUE*)hSendQueueHandle;
	mir_cslock l(pmrasqSendQueue->cs);

	LIST_MT_ITERATOR lmtiIterator;
	ListMTIteratorMoveFirst(pmrasqSendQueue, &lmtiIterator);
	do {
		MRA_SEND_QUEUE_ITEM *pmrasqiSendQueueItem;
		if ( !ListMTIteratorGet(&lmtiIterator, NULL, (LPVOID*)&pmrasqiSendQueueItem))
		if ((*((DWORDLONG*)&ftExpireTime))>(*((DWORDLONG*)&pmrasqiSendQueueItem->ftSendTime))) {
			if (pdwCMDNum)   *pdwCMDNum = pmrasqiSendQueueItem->dwCMDNum;
			if (pdwFlags)    *pdwFlags = pmrasqiSendQueueItem->dwFlags;
			if (phContact)   *phContact = pmrasqiSendQueueItem->hContact;
			if (pdwAckType)  *pdwAckType = pmrasqiSendQueueItem->dwAckType;
			if (plpbData)    *plpbData = pmrasqiSendQueueItem->lpbData;
			if (pdwDataSize) *pdwDataSize = pmrasqiSendQueueItem->dwDataSize;
			return 0;
		}
	}
		while (ListMTIteratorMoveNext(&lmtiIterator));

	return ERROR_NOT_FOUND;
}
