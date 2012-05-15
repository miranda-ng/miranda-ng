#include "Mra.h"
#include "MraMPop.h"




//	MPOP_SESSION
typedef struct
{
	FIFO_MT			ffmtQueueToQuery;
	BOOL			bKeyValid;
	LPSTR			lpszMPOPKey;
	SIZE_T			dwMPOPKeySize;
} MRA_MPOP_SESSION_QUEUE;


typedef struct
{
	FIFO_MT_ITEM	ffmtiFifoItem;
	LPSTR			lpszUrl;
	SIZE_T			dwUrlSize;
} MRA_MPOP_SESSION_QUEUE_ITEM;



void			MraMPopSessionQueueClear			(HANDLE hMPopSessionQueue);




DWORD MraMPopSessionQueueInitialize(HANDLE *phMPopSessionQueue)
{
	DWORD dwRetErrorCode;

	if (phMPopSessionQueue)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue;

		pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)MEMALLOC(sizeof(MRA_MPOP_SESSION_QUEUE));
		if (pmpsqMPopSessionQueue)
		{
			pmpsqMPopSessionQueue->bKeyValid=FALSE;
			pmpsqMPopSessionQueue->lpszMPOPKey=NULL;
			pmpsqMPopSessionQueue->dwMPOPKeySize=0;
			FifoMTInitialize(&pmpsqMPopSessionQueue->ffmtQueueToQuery,0);
			
			(*phMPopSessionQueue)=(HANDLE)pmpsqMPopSessionQueue;
			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_ALREADY_INITIALIZED;
	}
return(dwRetErrorCode);
}


void MraMPopSessionQueueClear(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

		pmpsqMPopSessionQueue->bKeyValid=FALSE;
		MEMFREE(pmpsqMPopSessionQueue->lpszMPOPKey);
		pmpsqMPopSessionQueue->dwMPOPKeySize=0;
		while(FifoMTItemPop(&pmpsqMPopSessionQueue->ffmtQueueToQuery,NULL,(LPVOID*)&pmpsqi)==NO_ERROR) MEMFREE(pmpsqi);
	}
}


void MraMPopSessionQueueFlush(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;

		while(FifoMTGetCount(&pmpsqMPopSessionQueue->ffmtQueueToQuery))
		{
			MraMPopSessionQueueSetNewMPopKey(hMPopSessionQueue,NULL,0);
			MraMPopSessionQueueStart(hMPopSessionQueue);
		}
	}
}


void MraMPopSessionQueueDestroy(HANDLE hMPopSessionQueue)
{
	if (hMPopSessionQueue)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		MraMPopSessionQueueClear(hMPopSessionQueue);
		FifoMTDestroy(&pmpsqMPopSessionQueue->ffmtQueueToQuery);
		MEMFREE(pmpsqMPopSessionQueue);
	}
}


DWORD MraMPopSessionQueueAddUrl(HANDLE hMPopSessionQueue,LPSTR lpszUrl,SIZE_T dwUrlSize)
{
	DWORD dwRetErrorCode;

	if (hMPopSessionQueue && lpszUrl && dwUrlSize)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi=(MRA_MPOP_SESSION_QUEUE_ITEM*)MEMALLOC((sizeof(MRA_MPOP_SESSION_QUEUE_ITEM)+dwUrlSize+sizeof(SIZE_T)));
		
		if (pmpsqi)
		{
			pmpsqi->dwUrlSize=dwUrlSize;
			pmpsqi->lpszUrl=(LPSTR)(pmpsqi+1);
			memmove(pmpsqi->lpszUrl,lpszUrl,dwUrlSize);
			FifoMTItemPush(&pmpsqMPopSessionQueue->ffmtQueueToQuery,&pmpsqi->ffmtiFifoItem,(LPVOID)pmpsqi);
			if (pmpsqMPopSessionQueue->bKeyValid)
			{
				dwRetErrorCode=MraMPopSessionQueueStart(hMPopSessionQueue);
			}else{
				if (masMraSettings.bLoggedIn)
				{
					MraSendCMD(MRIM_CS_GET_MPOP_SESSION,NULL,0);
				}else{
					MraMPopSessionQueueFlush(hMPopSessionQueue);
				}
				dwRetErrorCode=NO_ERROR;
			}
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraMPopSessionQueueAddUrlAndEMail(HANDLE hMPopSessionQueue,LPSTR lpszUrl,SIZE_T dwUrlSize,LPSTR lpszEMail,SIZE_T dwEMailSize)
{
	DWORD dwRetErrorCode;

	if (hMPopSessionQueue && lpszUrl && dwUrlSize && lpszEMail && dwEMailSize)
	{
		char szUrl[BUFF_SIZE_URL],szEMail[MAX_EMAIL_LEN];
		LPSTR lpszUser,lpszDomain;

		dwRetErrorCode=ERROR_INVALID_DATA;
		BuffToLowerCase(szEMail,lpszEMail,dwEMailSize);
		lpszDomain=(LPSTR)MemoryFindByte(0,szEMail,dwEMailSize,'@');
		if (lpszDomain)
		if (lpszUser=(LPSTR)MemoryFindByte((lpszDomain-szEMail),szEMail,dwEMailSize,'.'))
		{
			(*lpszUser)=0;
			lpszUser=szEMail;
			(*lpszDomain)=0;
			lpszDomain++;

			dwUrlSize=mir_snprintf(szUrl,SIZEOF(szUrl),"%s/%s/%s",lpszUrl,lpszDomain,lpszUser);
			dwRetErrorCode=MraMPopSessionQueueAddUrl(hMPopSessionQueue,szUrl,dwUrlSize);
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}

DWORD MraMPopSessionQueueStart(HANDLE hMPopSessionQueue)
{
	DWORD dwRetErrorCode;

	if (hMPopSessionQueue)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;
		MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

		dwRetErrorCode=NO_ERROR;
		if (pmpsqMPopSessionQueue->bKeyValid==TRUE)
		if (FifoMTItemPop(&pmpsqMPopSessionQueue->ffmtQueueToQuery,NULL,(LPVOID*)&pmpsqi)==NO_ERROR)
		{
			char szUrl[BUFF_SIZE_URL],szEMail[MAX_EMAIL_LEN];
			SIZE_T dwUrlSize,dwEMailSize;

			if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				pmpsqMPopSessionQueue->bKeyValid=FALSE;
				BuffToLowerCase(szEMail,szEMail,dwEMailSize);
				dwUrlSize=mir_snprintf(szUrl,SIZEOF(szUrl),MRA_MPOP_AUTH_URL,szEMail,pmpsqMPopSessionQueue->lpszMPOPKey,pmpsqi->lpszUrl);

				CallService(MS_UTILS_OPENURL,TRUE,(LPARAM)szUrl);
				DebugPrint(L"Opening URL: ");DebugPrintCRLFA(szUrl);
				//MraSendCMD(MRIM_CS_GET_MPOP_SESSION,NULL,0);
			}
			MEMFREE(pmpsqi);
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraMPopSessionQueueSetNewMPopKey(HANDLE hMPopSessionQueue,LPSTR lpszKey,SIZE_T dwKeySize)
{
	DWORD dwRetErrorCode;
	
	if (hMPopSessionQueue)
	{
		MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue=(MRA_MPOP_SESSION_QUEUE*)hMPopSessionQueue;

		if (lpszKey==NULL) dwKeySize=0;
		if (pmpsqMPopSessionQueue->dwMPOPKeySize<dwKeySize || dwKeySize==0)
		{
			MEMFREE(pmpsqMPopSessionQueue->lpszMPOPKey);
			pmpsqMPopSessionQueue->lpszMPOPKey=(LPSTR)MEMALLOC(dwKeySize+sizeof(SIZE_T));
		}

		if (pmpsqMPopSessionQueue->lpszMPOPKey)
		{
			pmpsqMPopSessionQueue->bKeyValid=TRUE;
			pmpsqMPopSessionQueue->dwMPOPKeySize=dwKeySize;
			memmove(pmpsqMPopSessionQueue->lpszMPOPKey,lpszKey,dwKeySize);
			(*(pmpsqMPopSessionQueue->lpszMPOPKey+dwKeySize))=0;

			DebugPrint(L"New MPOP session key: ");DebugPrintCRLFA(pmpsqMPopSessionQueue->lpszMPOPKey);
			dwRetErrorCode=NO_ERROR;
		}else{
			pmpsqMPopSessionQueue->bKeyValid=FALSE;
			pmpsqMPopSessionQueue->lpszMPOPKey=NULL;
			pmpsqMPopSessionQueue->dwMPOPKeySize=0;
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}





