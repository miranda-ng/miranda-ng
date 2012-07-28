#include <stdlib.h>
#include <string.h>
#include "fifo.h"

struct _tagFIFO
{
	unsigned int    uiCount;
	unsigned int    uiCapacity;
	unsigned int	uiActInd;
	char			*acStorage;
};


TYP_FIFO *Fifo_Init(unsigned int uiCapacity)
{
	TYP_FIFO *pstHandle;

	pstHandle = (TYP_FIFO *)malloc(sizeof(TYP_FIFO));
	if (!pstHandle) return NULL;
	pstHandle->uiCount = pstHandle->uiActInd = 0;
	pstHandle->uiCapacity = uiCapacity;
	if (uiCapacity == 0)
		pstHandle->acStorage = NULL;
	else
	{
		pstHandle->acStorage = (char *)malloc(uiCapacity);
		if (!pstHandle->acStorage)
		{
			free (pstHandle);
			return NULL;
		}
	}
	return pstHandle;
}

void Fifo_Exit(TYP_FIFO *pstHandle)
{
	if (pstHandle->acStorage)
		free (pstHandle->acStorage);
	free (pstHandle);
}

char *Fifo_AllocBuffer(TYP_FIFO *pstHandle, unsigned int uiPCount)
{
	unsigned int uiCount = pstHandle->uiCount;
	
	if (!Fifo_Add (pstHandle, NULL, uiPCount)) return NULL;
	return &pstHandle->acStorage[pstHandle->uiActInd+uiCount];
	
}

BOOL Fifo_Add(TYP_FIFO *pstHandle, char *acPBytes, unsigned int uiPCount)
{
	unsigned int uiFree;

	if (uiPCount == 0) return TRUE;
	if (pstHandle->uiCapacity == 0)
	{
		if (!(pstHandle->acStorage = (char *)calloc(1, uiPCount)))
			return FALSE;
		if (acPBytes)
			memcpy(pstHandle->acStorage, acPBytes, uiPCount);
		else
			memset(pstHandle->acStorage, 0, uiPCount);
		pstHandle->uiCapacity = pstHandle->uiCount = uiPCount;
		pstHandle->uiActInd = 0;
	}
	else
	{
		uiFree = pstHandle->uiCapacity-(pstHandle->uiActInd+pstHandle->uiCount);
		if (uiFree < uiPCount)
		{
			if (pstHandle->uiActInd>=uiPCount && pstHandle->uiActInd*4>pstHandle->uiCount)
			{
				memmove(pstHandle->acStorage, pstHandle->acStorage+pstHandle->uiActInd, pstHandle->uiCount);
				pstHandle->uiActInd = 0;
			}
			else
			{
				char *acBuf;
				unsigned int uiNewLen;

				if (pstHandle->uiCapacity*2 <
					pstHandle->uiCount+pstHandle->uiActInd+uiPCount)
					uiNewLen = pstHandle->uiCount+pstHandle->uiActInd+uiPCount;
				else
					uiNewLen = pstHandle->uiCapacity*2;
				acBuf = realloc(pstHandle->acStorage, uiNewLen);
				if (acBuf == NULL) return FALSE;
				pstHandle->acStorage = acBuf;
				memset (acBuf+pstHandle->uiCapacity, 0, uiNewLen-pstHandle->uiCapacity);
				pstHandle->uiCapacity = uiNewLen;
			}
		}
		if (acPBytes)
			memcpy (&pstHandle->acStorage[pstHandle->uiActInd+pstHandle->uiCount], acPBytes, uiPCount);
		else
			memset (&pstHandle->acStorage[pstHandle->uiActInd+pstHandle->uiCount], 0, uiPCount);

		pstHandle->uiCount += uiPCount;
	}

	return TRUE;
}

BOOL Fifo_AddString(TYP_FIFO *pstHandle, char *pszString)
{
	BOOL bRet;

	while (pstHandle->uiCount && pstHandle->acStorage[pstHandle->uiCount+pstHandle->uiActInd-1]==0)
		pstHandle->uiCount--;
	bRet = Fifo_Add (pstHandle, pszString, strlen(pszString)+1);
	return bRet;
}

char *Fifo_Get (TYP_FIFO *pstHandle, unsigned int *uiPCount)
{
	unsigned int uiCount;
	char *pRet;

	if (!uiPCount) uiCount = pstHandle->uiCount;
	else
	{
		if (pstHandle->uiCount < *uiPCount)
			*uiPCount = pstHandle->uiCount;
		uiCount = *uiPCount;
	}
	if (!uiCount) return NULL;

	pRet = &pstHandle->acStorage[pstHandle->uiActInd];
	pstHandle->uiActInd += uiCount;
	pstHandle->uiCount -= uiCount;
	return pRet;
}

void Fifo_Reset (TYP_FIFO *pstHandle)
{
	pstHandle->uiCount = pstHandle->uiActInd = 0;
}

unsigned int Fifo_Count (TYP_FIFO *pstHandle)
{
	return pstHandle->uiCount;
}
