#include <SystemHeaders.h>
#include <MemoryFindByte.h>
#include <MemoryCompare.h>
#include <BuffToLowerCase.h>
#include <ListMT.h>
#include <StrToNum.h>
#include "MraFilesQueue.h"
#include "proto.h"
#include "Mra.h"



#define MRA_FT_HELLO	"MRA_FT_HELLO"
#define MRA_FT_GET_FILE	"MRA_FT_GET_FILE"



typedef struct
{
	LIST_MT			lmtListMT;
	DWORD			dwSendTimeOutInterval;
} MRA_FILES_QUEUE;



struct MRA_FILES_QUEUE_FILE
{
	LPSTR	lpszName;
	SIZE_T	dwNameLen;
	SIZE_T	dwSize;
};

struct MRA_FILES_QUEUE_ADDR
{
	DWORD	dwIP;
	DWORD	dwPort;
};


typedef struct
{
	// internal
	LIST_MT_ITEM			lmtListMTItem;
	DWORD					dwSendTime;
	// external
	DWORD					dwIDRequest;
	DWORD					dwFlags;
	HANDLE					hContact;
	DWORD					dwFilesCount;
	DWORD					dwFilesTotalSize;
	DWORD					dwAddrCount;
	LPSTR					lpszDescription;
	SIZE_T					dwDescriptionSize;
	MRA_FILES_QUEUE_FILE	*pmfqfFiles;
	MRA_FILES_QUEUE_ADDR	*pmfqaAddreses;
	LPSTR					lpszPath;
	SIZE_T					dwPathSize;
	BOOL					bSending;
	HANDLE					hConnection;
	HANDLE					hListen;
	HANDLE					hThread;
	HANDLE					hWaitHandle;

} MRA_FILES_QUEUE_ITEM;



struct MRA_FILES_THREADPROC_PARAMS
{
	HANDLE					hFilesQueueHandle;
	MRA_FILES_QUEUE_ITEM	*pmrafqFilesQueueItem;
};



//#define MEMALLOC(Size)		HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(Size+sizeof(SIZE_T)))
//#define MEMREALLOC(Mem,Size)	HeapReAlloc(GetProcessHeap(),(HEAP_ZERO_MEMORY),(LPVOID)Mem,(Size+sizeof(SIZE_T)))
//#define MEMFREE(Mem)			if (Mem) {HeapFree(GetProcessHeap(),0,(LPVOID)Mem);Mem=NULL;}


DWORD			MraFilesQueueItemFindByID			(HANDLE hFilesQueueHandle,DWORD dwIDRequest,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem);
DWORD			MraFilesQueueItemFindByEMail		(HANDLE hFilesQueueHandle,LPSTR lpszEMail,SIZE_T dwEMailSize,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem);
void			MraFilesQueueItemFree				(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
SIZE_T			MraFilesQueueGetLocalAddressesList	(LPSTR lpszBuff,SIZE_T dwBuffSize,DWORD dwPort);

BOOL			MraFilesQueueHandCheck				(HANDLE hConnection,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem);
void			MraFilesQueueConnectionReceived		(HANDLE hNewConnection,DWORD dwRemoteIP,void *pExtra);

DWORD WINAPI	MraFilesQueueRecvThreadProc			(LPVOID lpParameter);
DWORD WINAPI	MraFilesQueueSendThreadProc			(LPVOID lpParameter);



BOOL CALLBACK MraFilesQueueDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			TranslateDialogDefault(hWndDlg);

			CheckDlgButton(hWndDlg,IDC_FILE_SEND_HIDE_MY_NOOUTCONNECTIONONRECEIVE,DB_Mra_GetByte(NULL,"FileSendNoOutConnOnRcv",MRA_DEF_FS_NO_OUT_CONN_ON_RCV));
			CheckDlgButton(hWndDlg,IDC_FILE_SEND_HIDE_MY_NOOUTCONNECTIONONSEND,DB_Mra_GetByte(NULL,"FileSendNoOutConnOnSend",MRA_DEF_FS_NO_OUT_CONN_ON_SEND));
			CheckDlgButton(hWndDlg,IDC_FILE_SEND_IGNORYADDITIONALPORTS,DB_Mra_GetByte(NULL,"FileSendIgnoryAdditionalPorts",MRA_DEF_FS_IGNORY_ADDITIONAL_PORTS));
			CheckDlgButton(hWndDlg,IDC_FILE_SEND_HIDE_MY_ADDRESSES,DB_Mra_GetByte(NULL,"FileSendHideMyAddresses",MRA_DEF_FS_HIDE_MY_ADDRESSES));
			CheckDlgButton(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS,DB_Mra_GetByte(NULL,"FileSendAddExtraAddresses",MRA_DEF_FS_ADD_EXTRA_ADDRESSES));
			if(!DBGetContactSetting(NULL,PROTOCOL_NAME,"FileSendExtraAddresses",&dbv)) {
				//bit of a security hole here, since it's easy to extract a password from an edit box
				SetDlgItemText(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS,dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			EnableWindow(GetDlgItem(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS),IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS));

			SetDlgItemInt(hWndDlg,IDC_FILE_SEND_BLOCK_SIZE,DB_Mra_GetDword(NULL,"FileSendBlockSize",MRA_DEFAULT_FILE_SEND_BLOCK_SIZE),FALSE);
		}
		return(TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam)==IDC_FILE_SEND_ADD_EXTRA_ADDRESS)
		{
			EnableWindow(GetDlgItem(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS),IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS));
		}

		if ((LOWORD(wParam)==IDC_FILE_SEND_EXTRA_ADDRESS || LOWORD(wParam)==IDC_FILE_SEND_BLOCK_SIZE) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) ) return(0);
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				char szBuff[MAX_PATH];

				DB_Mra_SetByte(NULL,"FileSendNoOutConnOnRcv",IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_HIDE_MY_NOOUTCONNECTIONONRECEIVE));
				DB_Mra_SetByte(NULL,"FileSendNoOutConnOnSend",IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_HIDE_MY_NOOUTCONNECTIONONSEND));
				DB_Mra_SetByte(NULL,"FileSendIgnoryAdditionalPorts",IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_IGNORYADDITIONALPORTS));
				DB_Mra_SetByte(NULL,"FileSendHideMyAddresses",IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_HIDE_MY_ADDRESSES));
				DB_Mra_SetByte(NULL,"FileSendAddExtraAddresses",IsDlgButtonChecked(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS));
				GetDlgItemText(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS,szBuff,sizeof(szBuff));
				DBWriteContactSettingString(NULL,PROTOCOL_NAME,"FileSendExtraAddresses",szBuff);
				DB_Mra_SetDword(NULL,"FileSendBlockSize",(DWORD)GetDlgItemInt(hWndDlg,IDC_FILE_SEND_BLOCK_SIZE,NULL,FALSE));
			}
			return(TRUE);
		}
		break;
	}
return(FALSE);
}



DWORD MraFilesQueueInitialize(DWORD dwSendTimeOutInterval,HANDLE *phFilesQueueHandle)
{
	DWORD dwRetErrorCode;

	if (phFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue;

		pmrafqFilesQueue=(MRA_FILES_QUEUE*)MEMALLOC(sizeof(MRA_FILES_QUEUE));
		if (pmrafqFilesQueue)
		{
			dwRetErrorCode=ListMTInitialize(&pmrafqFilesQueue->lmtListMT,0);
			if (dwRetErrorCode==NO_ERROR)
			{
				pmrafqFilesQueue->dwSendTimeOutInterval=dwSendTimeOutInterval;
				(*phFilesQueueHandle)=(HANDLE)pmrafqFilesQueue;
			}else{
				MEMFREE(pmrafqFilesQueue);
			}
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraFilesQueueDestroy(HANDLE hFilesQueueHandle)
{
	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		while(ListMTItemGetFirst(&pmrafqFilesQueue->lmtListMT,NULL,(LPVOID*)&pmrafqFilesQueueItem)==NO_ERROR)
		{
			ListMTItemDelete(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
			MraFilesQueueItemFree(pmrafqFilesQueueItem);
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);

		ListMTDestroy(&pmrafqFilesQueue->lmtListMT);
		MEMFREE(pmrafqFilesQueue);
	}
}


DWORD MraFilesQueueItemFindByID(HANDLE hFilesQueueHandle,DWORD dwIDRequest,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;
		LIST_MT_ITERATOR lmtiIterator;

		dwRetErrorCode=ERROR_NOT_FOUND;
		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		ListMTIteratorMoveFirst(&pmrafqFilesQueue->lmtListMT,&lmtiIterator);
		do
		{// цикл
			if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&pmrafqFilesQueueItem)==NO_ERROR)
			if (pmrafqFilesQueueItem->dwIDRequest==dwIDRequest)
			{
				if (ppmrafqFilesQueueItem) (*ppmrafqFilesQueueItem)=pmrafqFilesQueueItem;
				dwRetErrorCode=NO_ERROR;
				break;
			}
		}while(ListMTIteratorMoveNext(&lmtiIterator));
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraFilesQueueItemFindByEMail(HANDLE hFilesQueueHandle,LPSTR lpszEMail,SIZE_T dwEMailSize,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		char szEMailLocal[MAX_EMAIL_LEN];
		SIZE_T dwEMailLocalSize;
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;
		LIST_MT_ITERATOR lmtiIterator;

		dwRetErrorCode=ERROR_NOT_FOUND;
		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		ListMTIteratorMoveFirst(&pmrafqFilesQueue->lmtListMT,&lmtiIterator);
		do
		{// цикл
			if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&pmrafqFilesQueueItem)==NO_ERROR)
			if (DB_Mra_GetStaticString(pmrafqFilesQueueItem->hContact,"e-mail",szEMailLocal,sizeof(szEMailLocal),&dwEMailLocalSize))
			if (dwEMailSize==dwEMailLocalSize)
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszEMail,dwEMailSize,szEMailLocal,dwEMailLocalSize)==CSTR_EQUAL)
			{
				if (ppmrafqFilesQueueItem) (*ppmrafqFilesQueueItem)=pmrafqFilesQueueItem;
				dwRetErrorCode=NO_ERROR;
				break;
			}
		}while(ListMTIteratorMoveNext(&lmtiIterator));
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraFilesQueueItemFree(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{

	for(SIZE_T i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
	{
		MEMFREE(pmrafqFilesQueueItem->pmfqfFiles[i].lpszName);
	}
	MEMFREE(pmrafqFilesQueueItem->pmfqfFiles);
	MEMFREE(pmrafqFilesQueueItem->pmfqaAddreses);
	MEMFREE(pmrafqFilesQueueItem->lpszPath);
	//MEMFREE(pmrafqFilesQueueItem->lpszDescription); //placed after pmrafqFilesQueueItem 
	MEMFREE(pmrafqFilesQueueItem);
}





SIZE_T MraFilesQueueGetLocalAddressesList(LPSTR lpszBuff,SIZE_T dwBuffSize,DWORD dwPort)
{
	char szHostName[MAX_PATH]={0};
	LPSTR lpszCurPos=lpszBuff;
	SIZE_T dwAdapter=0;
	hostent *sh;

	if (DB_Mra_GetByte(NULL,"FileSendHideMyAddresses",MRA_DEF_FS_HIDE_MY_ADDRESSES))
	{// не выдаём врагу наш IP адрес!!! :)
		lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-(lpszCurPos-lpszBuff)),"192.168.0.1:26775;");
	}else{// создаём список наших IP адресов
		if (gethostname(szHostName,sizeof(szHostName))==0)
		if ((sh=gethostbyname((LPSTR)&szHostName)))
		{
			while(sh->h_addr_list[dwAdapter])
			{
				lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-(lpszCurPos-lpszBuff)),"%s:%ld;",inet_ntoa(*((struct in_addr*)sh->h_addr_list[dwAdapter])),dwPort);
				dwAdapter++;
			}
		}
	}

	if (DB_Mra_GetByte(NULL,"FileSendAddExtraAddresses",MRA_DEF_FS_ADD_EXTRA_ADDRESSES))// добавляем произвольный адрес
	if (DB_Mra_GetStaticString(NULL,"FileSendExtraAddresses",szHostName,sizeof(szHostName),NULL))
	{
		lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-(lpszCurPos-lpszBuff)),"%s:%ld;",szHostName,dwPort);
	}
return((lpszCurPos-lpszBuff));
}




DWORD MraFilesQueueAddReceive(HANDLE hFilesQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwIDRequest,DWORD dwFilesTotalSize,LPSTR lpszFiles,SIZE_T dwFilesSize,LPSTR lpszDescription,SIZE_T dwDescriptionSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle && dwIDRequest)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		if (!lpszDescription || dwDescriptionSize==0)
		{
			lpszDescription=Translate("No description given");
			dwDescriptionSize=lstrlenA(lpszDescription);
		}

		pmrafqFilesQueueItem=(MRA_FILES_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_FILES_QUEUE_ITEM)+sizeof(LPSTR)+dwDescriptionSize+64);
		if (pmrafqFilesQueueItem)
		{
			char szBuff[MAX_PATH];
			LPSTR lpszCurrentItem,lpszDelimiter,lpszEndItem,lpszBlob;
			SIZE_T dwMemSize,dwAllocatedCount;
			CCSDATA ccs;
			PROTORECVEVENT pre;

			//pmrafqFilesQueueItem->lmtListMTItem;
			pmrafqFilesQueueItem->dwSendTime=GetTickCount();
			pmrafqFilesQueueItem->dwIDRequest=dwIDRequest;
			pmrafqFilesQueueItem->dwFlags=dwFlags;
			pmrafqFilesQueueItem->hContact=hContact;
			pmrafqFilesQueueItem->lpszDescription=(LPSTR)(pmrafqFilesQueueItem+1);
			pmrafqFilesQueueItem->dwDescriptionSize=dwDescriptionSize;
			CopyMemory(pmrafqFilesQueueItem->lpszDescription,lpszDescription,dwDescriptionSize);


			dwAllocatedCount=ALLOCATED_COUNT;
			pmrafqFilesQueueItem->dwFilesCount=0;
			pmrafqFilesQueueItem->dwFilesTotalSize=dwFilesTotalSize;
			pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMALLOC((sizeof(MRA_FILES_QUEUE_FILE)*dwAllocatedCount));
			lpszCurrentItem=lpszFiles;
			while(TRUE)
			{
				lpszDelimiter=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszFiles),lpszFiles,dwFilesSize,';');
				if (lpszDelimiter)
				{
					lpszEndItem=(LPSTR)MemoryFindByte(((lpszDelimiter+1)-lpszFiles),lpszFiles,dwFilesSize,';');
					if (lpszEndItem)
					{
						if (pmrafqFilesQueueItem->dwFilesCount==dwAllocatedCount)
						{
							dwAllocatedCount*=2;
							pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMREALLOC(pmrafqFilesQueueItem->pmfqfFiles,(sizeof(MRA_FILES_QUEUE_FILE)*dwAllocatedCount));
						}

						dwMemSize=(lpszDelimiter-lpszCurrentItem);
						pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].lpszName=(LPSTR)MEMALLOC(dwMemSize);
						CopyMemory(pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].lpszName,lpszCurrentItem,dwMemSize);
						pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].dwNameLen=dwMemSize;
						pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].dwSize=StrToUNum32((lpszDelimiter+1),(lpszEndItem-(lpszDelimiter+1)));
						
						pmrafqFilesQueueItem->dwFilesCount++;
						lpszCurrentItem=(lpszEndItem+1);
					}else{
						break;
					}
				}else{
					break;
				}
			}
			pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMREALLOC(pmrafqFilesQueueItem->pmfqfFiles,(sizeof(MRA_FILES_QUEUE_FILE)*pmrafqFilesQueueItem->dwFilesCount));


			dwAllocatedCount=ALLOCATED_COUNT;
			pmrafqFilesQueueItem->dwAddrCount=0;
			pmrafqFilesQueueItem->pmfqaAddreses=(MRA_FILES_QUEUE_ADDR*)MEMALLOC(sizeof(MRA_FILES_QUEUE_ADDR)*dwAllocatedCount);
			lpszCurrentItem=lpszAddreses;
			while(TRUE)
			{
				lpszEndItem=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses),lpszAddreses,dwAddresesSize,';');
				if (lpszEndItem)
				{
					lpszDelimiter=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses),lpszAddreses,dwAddresesSize,':');
					if (lpszDelimiter)
					{
						if (pmrafqFilesQueueItem->dwAddrCount==dwAllocatedCount)
						{
							dwAllocatedCount*=2;
							pmrafqFilesQueueItem->pmfqaAddreses=(MRA_FILES_QUEUE_ADDR*)MEMREALLOC(pmrafqFilesQueueItem->pmfqaAddreses,(sizeof(MRA_FILES_QUEUE_ADDR)*dwAllocatedCount));
						}

						(*lpszDelimiter)=0;
						pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwIP=inet_addr(lpszCurrentItem);
						pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwPort=StrToUNum32((lpszDelimiter+1),(lpszEndItem-(lpszDelimiter+1)));
						(*lpszDelimiter)=':';

						switch(pmrafqFilesQueueItem->dwAddrCount){
						case 0:
							DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"IP",0));DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"IP",htonl(pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwIP));
							break;
						case 1:
							DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldRealIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"RealIP",0));DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"RealIP",htonl(pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwIP));
							break;
						}
						pmrafqFilesQueueItem->dwAddrCount++;
						lpszCurrentItem=(lpszEndItem+1);
					}else{
						break;
					}
				}else{
					break;
				}
			}
			pmrafqFilesQueueItem->pmfqaAddreses=(MRA_FILES_QUEUE_ADDR*)MEMREALLOC(pmrafqFilesQueueItem->pmfqaAddreses,(sizeof(MRA_FILES_QUEUE_ADDR)*pmrafqFilesQueueItem->dwAddrCount));

			ListMTLock(&pmrafqFilesQueue->lmtListMT);
			ListMTItemAdd(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem,pmrafqFilesQueueItem);
			ListMTUnLock(&pmrafqFilesQueue->lmtListMT);


			// Send chain event
			dwMemSize=(sizeof(DWORD)+dwDescriptionSize+dwAddresesSize+sizeof(SIZE_T));
			for(SIZE_T i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{
				dwMemSize+=(pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen+64);
			}
			lpszBlob=(LPSTR)MEMALLOC(dwMemSize);
			lpszCurrentItem=lpszBlob;
			// user data
            (*((DWORD*)lpszCurrentItem))=dwIDRequest;lpszCurrentItem+=sizeof(DWORD);
			// file name
			StrFormatByteSizeA(pmrafqFilesQueueItem->dwFilesTotalSize,szBuff,sizeof(szBuff));
			lpszCurrentItem+=(mir_snprintf(lpszCurrentItem,(dwMemSize-(lpszCurrentItem-lpszBlob)),"%ld Files (%s)",pmrafqFilesQueueItem->dwFilesCount,szBuff)+1);
			// description
			for(SIZE_T i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{
				StrFormatByteSizeA(pmrafqFilesQueueItem->pmfqfFiles[i].dwSize,szBuff,sizeof(szBuff));
				lpszCurrentItem+=mir_snprintf(lpszCurrentItem,(dwMemSize-(lpszCurrentItem-lpszBlob)),"%s - %s\r\n",pmrafqFilesQueueItem->pmfqfFiles[i].lpszName,szBuff);
			}
			CopyMemory(lpszCurrentItem,lpszAddreses,dwAddresesSize);lpszCurrentItem+=dwAddresesSize;
			lpszCurrentItem+=(mir_snprintf(lpszCurrentItem,(dwMemSize-(lpszCurrentItem-lpszBlob)),"\r\n%s",lpszDescription)+1);

			ccs.szProtoService=PSR_FILE;
			ccs.hContact=hContact;
			ccs.wParam=0;
			ccs.lParam=(LPARAM)&pre;
			pre.flags=0;
			pre.timestamp=time(NULL);
			pre.szMessage=lpszBlob;
			pre.lParam=(lpszCurrentItem-lpszBlob);

			CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
			MEMFREE(lpszBlob);

			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



DWORD MraFilesQueueAddSend(HANDLE hFilesQueueHandle,DWORD dwFlags,HANDLE hContact,LPSTR *plpszFiles,LPSTR lpszDescription,SIZE_T dwDescriptionSize,DWORD *pdwIDRequest)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;


		pmrafqFilesQueueItem=(MRA_FILES_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_FILES_QUEUE_ITEM)+sizeof(LPSTR)+dwDescriptionSize+64);
		if (pmrafqFilesQueueItem)
		{
			SIZE_T i;
			WIN32_FILE_ATTRIBUTE_DATA wfad;

			//pmrafqFilesQueueItem->lmtListMTItem;
			pmrafqFilesQueueItem->dwSendTime=GetTickCount();
			pmrafqFilesQueueItem->dwIDRequest=InterlockedIncrement((LONG volatile*)&masMraSettings.dwCMDNum);// уникальный, рандомный идентификатор
			pmrafqFilesQueueItem->dwFlags=dwFlags;
			pmrafqFilesQueueItem->hContact=hContact;
			pmrafqFilesQueueItem->lpszDescription=(LPSTR)(pmrafqFilesQueueItem+1);
			pmrafqFilesQueueItem->dwDescriptionSize=dwDescriptionSize;
			CopyMemory(pmrafqFilesQueueItem->lpszDescription,lpszDescription,dwDescriptionSize);


			for(pmrafqFilesQueueItem->dwFilesCount=0;plpszFiles[pmrafqFilesQueueItem->dwFilesCount];pmrafqFilesQueueItem->dwFilesCount++);
			pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMALLOC((sizeof(MRA_FILES_QUEUE_FILE)*(pmrafqFilesQueueItem->dwFilesCount+1)));
			pmrafqFilesQueueItem->dwFilesTotalSize=0;

			for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{
				if (GetFileAttributesExA(plpszFiles[i],GetFileExInfoStandard,&wfad))
				{
					pmrafqFilesQueueItem->pmfqfFiles[i].dwSize=wfad.nFileSizeLow;
					pmrafqFilesQueueItem->dwFilesTotalSize+=wfad.nFileSizeLow;
				}else{
					pmrafqFilesQueueItem->pmfqfFiles[i].dwSize=0;
				}
				pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen=lstrlenA(plpszFiles[i]);
				pmrafqFilesQueueItem->pmfqfFiles[i].lpszName=(LPSTR)MEMALLOC(pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen);
				CopyMemory(pmrafqFilesQueueItem->pmfqfFiles[i].lpszName,plpszFiles[i],pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen);
			}
			//pmrafqFilesQueueItem->dwAddrCount=0;
			//pmrafqFilesQueueItem->pmfqaAddreses=NULL;
			pmrafqFilesQueueItem->bSending=TRUE;
			if (pdwIDRequest) (*pdwIDRequest)=pmrafqFilesQueueItem->dwIDRequest;

			ListMTLock(&pmrafqFilesQueue->lmtListMT);
			ListMTItemAdd(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem,pmrafqFilesQueueItem);
			ListMTUnLock(&pmrafqFilesQueue->lmtListMT);

			{
				MRA_FILES_THREADPROC_PARAMS *pmftpp=(MRA_FILES_THREADPROC_PARAMS*)MEMALLOC(sizeof(MRA_FILES_THREADPROC_PARAMS));
				pmftpp->hFilesQueueHandle=hFilesQueueHandle;
				pmftpp->pmrafqFilesQueueItem=pmrafqFilesQueueItem;
				
				pmrafqFilesQueueItem->hThread=CreateThread(0,0,MraFilesQueueSendThreadProc,pmftpp,0,NULL);
			}

			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



DWORD MraFilesQueueAccept(HANDLE hFilesQueueHandle,DWORD dwIDRequest,LPSTR lpszPath,SIZE_T dwPathSize)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		if ((dwRetErrorCode=MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem))==NO_ERROR)
		{
			MRA_FILES_THREADPROC_PARAMS *pmftpp=(MRA_FILES_THREADPROC_PARAMS*)MEMALLOC(sizeof(MRA_FILES_THREADPROC_PARAMS));
			pmrafqFilesQueueItem->lpszPath=(LPSTR)MEMALLOC(dwPathSize);
			pmrafqFilesQueueItem->dwPathSize=dwPathSize;
			CopyMemory(pmrafqFilesQueueItem->lpszPath,lpszPath,dwPathSize);

			pmftpp->hFilesQueueHandle=hFilesQueueHandle;
			pmftpp->pmrafqFilesQueueItem=pmrafqFilesQueueItem;
			
			pmrafqFilesQueueItem->hThread=CreateThread(0,0,MraFilesQueueRecvThreadProc,pmftpp,0,NULL);
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraFilesQueueDecline(HANDLE hFilesQueueHandle,DWORD dwIDRequest,LPSTR lpszDescription,SIZE_T dwDescriptionSize)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		if ((dwRetErrorCode=MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem))==NO_ERROR)
		{
			char szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;
			if (DB_Mra_GetStaticString(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize))
			{
				MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_DECLINE,szEMail,dwEMailSize,dwIDRequest,lpszDescription,dwDescriptionSize);
			}
			ListMTItemDelete(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
			MraFilesQueueItemFree(pmrafqFilesQueueItem);
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraFilesQueueCancel(HANDLE hFilesQueueHandle,DWORD dwIDRequest,BOOL bSendDecline)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		if ((dwRetErrorCode=MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem))==NO_ERROR)
		{//***deb closesocket, send message to thread
			if (bSendDecline)
			{
				char szEMail[MAX_EMAIL_LEN];
				SIZE_T dwEMailSize;
				if (DB_Mra_GetStaticString(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize))
				{
					MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_DECLINE,szEMail,dwEMailSize,dwIDRequest,NULL,0);
				}
			}
			Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
			pmrafqFilesQueueItem->hConnection=NULL;

			Netlib_CloseHandle(pmrafqFilesQueueItem->hListen);
			pmrafqFilesQueueItem->hListen=NULL;

			SetEvent(pmrafqFilesQueueItem->hWaitHandle);

			if (pmrafqFilesQueueItem->hThread==NULL)
			{
				ListMTItemDelete(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
				MraFilesQueueItemFree(pmrafqFilesQueueItem);
			}
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraFilesQueueFree(HANDLE hFilesQueueHandle,DWORD dwIDRequest)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;
		LIST_MT_ITERATOR lmtiIterator;

		dwRetErrorCode=ERROR_NOT_FOUND;
		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		ListMTIteratorMoveFirst(&pmrafqFilesQueue->lmtListMT,&lmtiIterator);
		do
		{// цикл
			if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&pmrafqFilesQueueItem)==NO_ERROR)
			if (pmrafqFilesQueueItem->dwIDRequest==dwIDRequest)
			{
				ListMTItemDelete(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
				MraFilesQueueItemFree(pmrafqFilesQueueItem);
				dwRetErrorCode=NO_ERROR;
				break;
			}
		}while(ListMTIteratorMoveNext(&lmtiIterator));
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraFilesQueueSendMirror(HANDLE hFilesQueueHandle,DWORD dwIDRequest,LPSTR lpszAddreses,SIZE_T dwAddresesSize)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		if ((dwRetErrorCode=MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem))==NO_ERROR)
		{//
			LPSTR lpszCurrentItem,lpszDelimiter,lpszEndItem;
			SIZE_T dwAllocatedCount;

			dwAllocatedCount=ALLOCATED_COUNT;
			pmrafqFilesQueueItem->dwAddrCount=0;
			pmrafqFilesQueueItem->pmfqaAddreses=(MRA_FILES_QUEUE_ADDR*)MEMALLOC(sizeof(MRA_FILES_QUEUE_ADDR)*dwAllocatedCount);
			lpszCurrentItem=lpszAddreses;
			while(TRUE)
			{
				lpszEndItem=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses),lpszAddreses,dwAddresesSize,';');
				if (lpszEndItem)
				{
					lpszDelimiter=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses),lpszAddreses,dwAddresesSize,':');
					if (lpszDelimiter)
					{
						if (pmrafqFilesQueueItem->dwAddrCount==dwAllocatedCount)
						{
							dwAllocatedCount*=2;
							pmrafqFilesQueueItem->pmfqaAddreses=(MRA_FILES_QUEUE_ADDR*)MEMREALLOC(pmrafqFilesQueueItem->pmfqaAddreses,(sizeof(MRA_FILES_QUEUE_ADDR)*dwAllocatedCount));
						}

						(*lpszDelimiter)=0;
						pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwIP=inet_addr(lpszCurrentItem);
						pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwPort=StrToUNum32((lpszDelimiter+1),(lpszEndItem-(lpszDelimiter+1)));
						(*lpszDelimiter)=':';

						switch(pmrafqFilesQueueItem->dwAddrCount){
						case 0:
							DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"IP",0));
							DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"IP",htonl(pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwIP));
							break;
						case 1:
							DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldRealIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"RealIP",0));
							DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"RealIP",htonl(pmrafqFilesQueueItem->pmfqaAddreses[pmrafqFilesQueueItem->dwAddrCount].dwIP));
							break;
						}
						pmrafqFilesQueueItem->dwAddrCount++;
						lpszCurrentItem=(lpszEndItem+1);
					}else{
						break;
					}
				}else{
					break;
				}
			}
			pmrafqFilesQueueItem->pmfqaAddreses=(MRA_FILES_QUEUE_ADDR*)MEMREALLOC(pmrafqFilesQueueItem->pmfqaAddreses,(sizeof(MRA_FILES_QUEUE_ADDR)*pmrafqFilesQueueItem->dwAddrCount));

			pmrafqFilesQueueItem->hConnection=NULL;
			SetEvent(pmrafqFilesQueueItem->hWaitHandle);
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



BOOL MraFilesQueueHandCheck(HANDLE hConnection,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem)
{
	BOOL bRet=FALSE;

	if (hConnection)
	{
		char szEMail[MAX_EMAIL_LEN]={0},szEMailMy[MAX_EMAIL_LEN]={0};
		BYTE btBuff[((MAX_EMAIL_LEN*2)+(sizeof(MRA_FT_HELLO)*2)+8)]={0};
		SIZE_T dwEMailSize,dwEMailMySize,dwBuffSize;

		DB_Mra_GetStaticString(NULL,"e-mail",szEMailMy,sizeof(szEMailMy),&dwEMailMySize);BuffToLowerCase(szEMailMy,szEMailMy,dwEMailMySize);

		if (pmrafqFilesQueueItem->bSending==FALSE)
		{
			dwBuffSize=(mir_snprintf((LPSTR)btBuff,sizeof(btBuff),"%s %s",MRA_FT_HELLO,szEMailMy)+1);
			if (dwBuffSize==Netlib_Send(hConnection,(LPSTR)btBuff,dwBuffSize,0))
			{// my email sended
				//ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_INITIALISING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
				dwBuffSize=Netlib_Recv(hConnection,(LPSTR)btBuff,sizeof(btBuff),0);
				if (ppmrafqFilesQueueItem && (*ppmrafqFilesQueueItem))
				{// have context, connecting or some times in receive
					DB_Mra_GetStaticString((*ppmrafqFilesQueueItem)->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize);BuffToLowerCase(szEMail,szEMail,dwEMailSize);
					if ((dwEMailSize+sizeof(MRA_FT_HELLO)+1)==dwBuffSize)
					{// email received
						mir_snprintf(((LPSTR)btBuff+dwBuffSize),(sizeof(btBuff)-dwBuffSize),"%s %s",MRA_FT_HELLO,szEMail);
						if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(LPSTR)btBuff,dwBuffSize,((LPSTR)btBuff+dwBuffSize),dwBuffSize)==CSTR_EQUAL)
						{// email verifyed
							bRet=TRUE;
						}
					}
				}else{// some times on receive pExtra=NULL, and we looking for context
					if (dwBuffSize!=SOCKET_ERROR && dwBuffSize>sizeof(MRA_FT_HELLO))
					{// readed
						if (MraFilesQueueItemFindByEMail(HANDLE hFilesQueueHandle,((LPSTR)btBuff+sizeof(MRA_FT_HELLO)),(dwBuffSize-(sizeof(MRA_FT_HELLO)+1)),ppmrafqFilesQueueItem)==NO_ERROR)
						{// email verifyed
							bRet=TRUE;
						}
					}
				}
			}
		}else{
			dwBuffSize=Netlib_Recv(hConnection,(LPSTR)btBuff,sizeof(btBuff),0);
			if (ppmrafqFilesQueueItem && (*ppmrafqFilesQueueItem))
			{// have context, connecting or some times in receive
				DB_Mra_GetStaticString((*ppmrafqFilesQueueItem)->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize);BuffToLowerCase(szEMail,szEMail,dwEMailSize);
				if ((dwEMailSize+sizeof(MRA_FT_HELLO)+1)==dwBuffSize)
				{// email received
					ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_INITIALISING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
					mir_snprintf(((LPSTR)btBuff+dwBuffSize),(sizeof(btBuff)-dwBuffSize),"%s %s",MRA_FT_HELLO,szEMail);
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(LPSTR)btBuff,dwBuffSize,((LPSTR)btBuff+dwBuffSize),dwBuffSize)==CSTR_EQUAL)
					{// email verifyed
						dwBuffSize=(mir_snprintf((LPSTR)btBuff,sizeof(btBuff),"%s %s",MRA_FT_HELLO,szEMailMy)+1);
						if (dwBuffSize==Netlib_Send(hConnection,(LPSTR)btBuff,dwBuffSize,0))
						{// my email sended
							bRet=TRUE;
						}
					}
				}
			}else{// some times on receive pExtra=NULL, and we looking for context
				if (dwBuffSize!=SOCKET_ERROR && dwBuffSize>sizeof(MRA_FT_HELLO))
				{// readed
					if (MraFilesQueueItemFindByEMail(HANDLE hFilesQueueHandle,((LPSTR)btBuff+sizeof(MRA_FT_HELLO)),(dwBuffSize-(sizeof(MRA_FT_HELLO)+1)),ppmrafqFilesQueueItem)==NO_ERROR)
					{// email verifyed
						ProtoBroadcastAck(PROTOCOL_NAME,(*ppmrafqFilesQueueItem)->hContact,ACKTYPE_FILE,ACKRESULT_INITIALISING,(HANDLE)(*ppmrafqFilesQueueItem)->dwIDRequest,0);
						dwBuffSize=(mir_snprintf((LPSTR)btBuff,sizeof(btBuff),"%s %s",MRA_FT_HELLO,szEMailMy)+1);
						if (dwBuffSize==Netlib_Send(hConnection,(LPSTR)btBuff,dwBuffSize,0))
						{// my email sended
							bRet=TRUE;
						}
					}
				}
			}
		}
	}
return(bRet);
}


HANDLE MraFilesQueueConnectOut(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{
	if ((pmrafqFilesQueueItem->bSending==FALSE && DB_Mra_GetByte(NULL,"FileSendNoOutConnOnRcv",MRA_DEF_FS_NO_OUT_CONN_ON_RCV)==FALSE) || (pmrafqFilesQueueItem->bSending==TRUE && DB_Mra_GetByte(NULL,"FileSendNoOutConnOnSend",MRA_DEF_FS_NO_OUT_CONN_ON_SEND)==FALSE))
	{
		BOOL bFiltering=FALSE;
		DWORD dwLocalPort=0;
		SIZE_T i;
		NETLIBOPENCONNECTION nloc={0};

		nloc.cbSize=sizeof(nloc);
		nloc.flags=NLOCF_V2;
		if (DB_Mra_GetByte(NULL,"FileSendIgnoryAdditionalPorts",MRA_DEF_FS_IGNORY_ADDITIONAL_PORTS))
		{// фильтруем порты для одного IP, вместо 3 будем коннектится только к одному
			switch(DB_Mra_GetWord(NULL,"ServerPort",MRA_DEFAULT_SERVER_PORT)){
			case MRA_SERVER_PORT_STANDART_NLB: 
				dwLocalPort=MRA_SERVER_PORT_STANDART;
				break;
			case MRA_SERVER_PORT_HTTPS: 
				dwLocalPort=MRA_SERVER_PORT_HTTPS;
				break;
			default:
				dwLocalPort=DB_Mra_GetWord(NULL,"ServerPort",MRA_DEFAULT_SERVER_PORT);
				break;
			}

			for(i=0;i<pmrafqFilesQueueItem->dwAddrCount;i++)
			{// Set up the sockaddr structure
				if (dwLocalPort==pmrafqFilesQueueItem->pmfqaAddreses[i].dwPort)
				{
					bFiltering=TRUE;
					break;
				}
			}
		}

		for(i=0;i<pmrafqFilesQueueItem->dwAddrCount;i++)
		{// Set up the sockaddr structure
			if (dwLocalPort==pmrafqFilesQueueItem->pmfqaAddreses[i].dwPort || bFiltering==FALSE)
			{
				nloc.szHost=inet_ntoa((*((in_addr*)&pmrafqFilesQueueItem->pmfqaAddreses[i].dwIP)));
				nloc.wPort=(WORD)pmrafqFilesQueueItem->pmfqaAddreses[i].dwPort;

				ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
				if ((pmrafqFilesQueueItem->hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nloc)))
				{
					ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
					if (MraFilesQueueHandCheck(pmrafqFilesQueueItem->hConnection,&pmrafqFilesQueueItem))
					{// связь установленная с тем кем нужно
						break;
					}else{// кажется не туда подключились :)
						ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
						Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
						pmrafqFilesQueueItem->hConnection=NULL;
					}
				}
			}
		}
	}
return(pmrafqFilesQueueItem->hConnection);
}


HANDLE MraFilesQueueConnectIn(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{
	char szEMail[MAX_EMAIL_LEN],szAddrList[2048]={0};
	SIZE_T dwEMailSize,dwAddrListSize;
	NETLIBBIND nlbBind={0};

	nlbBind.cbSize=sizeof(nlbBind);
	nlbBind.pfnNewConnectionV2=MraFilesQueueConnectionReceived;
	nlbBind.wPort=0;
	nlbBind.pExtra=(LPVOID)pmrafqFilesQueueItem;

	if (DB_Mra_GetStaticString(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize))
	if ((pmrafqFilesQueueItem->hListen=(HANDLE)CallService(MS_NETLIB_BINDPORT,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nlbBind)))
	{
		dwAddrListSize=MraFilesQueueGetLocalAddressesList(szAddrList,sizeof(szAddrList),nlbBind.wPort);
		if (dwAddrListSize)
		{
			pmrafqFilesQueueItem->hWaitHandle=CreateEvent(NULL,TRUE,FALSE,NULL);
			if (pmrafqFilesQueueItem->bSending==FALSE)
			{// запрашиваем зеркальное соединение
				MraSendCommand_FileTransferAck(FILE_TRANSFER_MIRROR,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,szAddrList,dwAddrListSize);
			}else{// здесь отправляем запрос на передачу(установление соединения)
				LPSTR lpszFiles,lpszFileName,lpszCurPos;
				SIZE_T i,dwFilesSize=((MAX_PATH*2)*pmrafqFilesQueueItem->dwFilesCount);

				lpszFiles=(LPSTR)MEMALLOC(dwFilesSize);
				if (lpszFiles)
				{
					lpszCurPos=lpszFiles;
					for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
					{
						GetFullPathName(pmrafqFilesQueueItem->pmfqfFiles[i].lpszName,MAX_PATH,pmrafqFilesQueueItem->pmfqfFiles[i].lpszName,&lpszFileName);
						lpszCurPos+=mir_snprintf(lpszCurPos,(dwFilesSize-(lpszCurPos-lpszFiles)),"%s;%ld;",lpszFileName,pmrafqFilesQueueItem->pmfqfFiles[i].dwSize);
					}
					MraSendCommand_FileTransfer(szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,pmrafqFilesQueueItem->dwFilesTotalSize,lpszFiles,(lpszCurPos-lpszFiles),pmrafqFilesQueueItem->lpszDescription,pmrafqFilesQueueItem->dwDescriptionSize,szAddrList,dwAddrListSize);
					MEMFREE(lpszFiles);
				}
			}

			WaitForSingleObject(pmrafqFilesQueueItem->hWaitHandle,INFINITE);
			CloseHandle(pmrafqFilesQueueItem->hWaitHandle);
			pmrafqFilesQueueItem->hWaitHandle=NULL;
			Netlib_CloseHandle(pmrafqFilesQueueItem->hListen);
			pmrafqFilesQueueItem->hListen=NULL;
		}
	}
return(pmrafqFilesQueueItem->hConnection);
}

// This function is called from the Netlib when someone is connecting to
// one of our incomming DC ports
void MraFilesQueueConnectionReceived(HANDLE hNewConnection,DWORD dwRemoteIP,void *pExtra)
{
	MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem=(MRA_FILES_QUEUE_ITEM*)pExtra;

	if (MraFilesQueueHandCheck(hNewConnection,&pmrafqFilesQueueItem))
	{// связь установленная с тем кем нужно
		pmrafqFilesQueueItem->hConnection=hNewConnection;
		ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"IP",0));
		DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"IP",dwRemoteIP);
		SetEvent(pmrafqFilesQueueItem->hWaitHandle);
	}else{// кажется кто то не туда подключилися :)
		if (pmrafqFilesQueueItem) ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		Netlib_CloseHandle(hNewConnection);
	}

	if (pExtra==NULL)
	{
		MraPopupShowFromAgent(MRA_POPUP_TYPE_DEBUG,0,"MraFilesQueueConnectionReceived: connection accepted, but pExtra=NULL, this is miranda bug.");
		DebugBreak();
	}
}



DWORD WINAPI MraFilesQueueRecvThreadProc(LPVOID lpParameter)
{
	MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->hFilesQueueHandle;
	MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem=((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->pmrafqFilesQueueItem;
    
	char szFileName[MAX_FILEPATH]={0};
	BYTE btBuff[65536];
	BOOL bFailed=TRUE,bOK,bConnected=FALSE;
	DWORD dwWritten,dwReceived,dwUpdateTimeNext,dwUpdateTimeCur;
	HANDLE hFile;
	SIZE_T i,dwBuffSize;
	PROTOFILETRANSFERSTATUS pfts={0};


	MEMFREE(lpParameter);
	pfts.cbSize=sizeof(pfts);
	pfts.hContact=pmrafqFilesQueueItem->hContact;
	pfts.sending=pmrafqFilesQueueItem->bSending;	//true if sending, false if receiving
	//pfts.files;
	pfts.totalFiles=pmrafqFilesQueueItem->dwFilesCount;
	pfts.currentFileNumber=0;
	pfts.totalBytes=pmrafqFilesQueueItem->dwFilesTotalSize;
	pfts.totalProgress=0;
	pfts.workingDir=pmrafqFilesQueueItem->lpszPath;
	//pfts.currentFile;
	//pfts.currentFileSize;
	//pfts.currentFileProgress;
	//pfts.currentFileTime;  //as seconds since 1970


	CallService(MS_SYSTEM_THREAD_PUSH,0,0);
	//ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);

	if (MraFilesQueueConnectOut(pmrafqFilesQueueItem))
	{
		bConnected=TRUE;
	}else{
		if (MraFilesQueueConnectIn(pmrafqFilesQueueItem)) bConnected=TRUE;
	}

	if (bConnected)
	{// email verifyed
		bFailed=FALSE;
		for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
		{// receiving files
			ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_NEXTFILE,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
			dwBuffSize=(mir_snprintf((LPSTR)btBuff,sizeof(btBuff),"%s %s",MRA_FT_GET_FILE,pmrafqFilesQueueItem->pmfqfFiles[i].lpszName)+1);
			if (dwBuffSize==Netlib_Send(pmrafqFilesQueueItem->hConnection,(LPSTR)btBuff,dwBuffSize,0))
			{// file request sended
				mir_snprintf(szFileName,sizeof(szFileName),"%s%s",pmrafqFilesQueueItem->lpszPath,pmrafqFilesQueueItem->pmfqfFiles[i].lpszName);
				hFile=CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
				if (hFile!=INVALID_HANDLE_VALUE)
				{
					bOK=FALSE;
					dwWritten=0;
					dwUpdateTimeNext=GetTickCount();
					pfts.currentFileNumber=i;
					pfts.currentFile=pmrafqFilesQueueItem->pmfqfFiles[i].lpszName;
					pfts.currentFileSize=pmrafqFilesQueueItem->pmfqfFiles[i].dwSize;
					pfts.currentFileProgress=0;
					//pfts.currentFileTime;  //as seconds since 1970
					ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);

					// pre allocating disk space, best perfomance
					SetFilePointer(hFile,pmrafqFilesQueueItem->pmfqfFiles[i].dwSize,NULL,FILE_BEGIN);
					SetEndOfFile(hFile);
					SetFilePointer(hFile,0,NULL,FILE_BEGIN);

					while(TRUE)
					{
						dwReceived=Netlib_Recv(pmrafqFilesQueueItem->hConnection,(LPSTR)&btBuff,sizeof(btBuff),0);
						if (dwReceived==0 || dwReceived==SOCKET_ERROR)
						{
							dwWritten=0;
							break;
						}else{
							WriteFile(hFile,(LPVOID)&btBuff,dwReceived,&dwReceived,NULL);
							dwWritten+=dwReceived;
							pfts.totalProgress+=dwReceived;
							if (dwWritten>=pmrafqFilesQueueItem->pmfqfFiles[i].dwSize)
							{
								dwUpdateTimeNext=GetTickCount();// force stat update
								bOK=TRUE;
								break;
							}

							// progress updates
							dwUpdateTimeCur=GetTickCount();
							if (dwUpdateTimeNext<=dwUpdateTimeCur)
							{// we update it
								dwUpdateTimeNext=dwUpdateTimeCur+MRA_FILES_QUEUE_PROGRESS_INTERVAL;
								pfts.currentFileProgress=dwWritten;
								ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);
							}
						}
					}// end while
					CloseHandle(hFile);

					if (bOK==FALSE)
					{// file recv failed
						DeleteFile(szFileName);
						bFailed=TRUE;
						break;
					}
				}else{// err on write file
					bFailed=TRUE;
					break;
				}
			}else{//err
				break;
			}
		}// end for

		Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
		pmrafqFilesQueueItem->hConnection=NULL;
	}

	if (bFailed)
	{
		char szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;
		if (DB_Mra_GetStaticString(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize))
		{
			MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_DECLINE,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,NULL,0);
		}
		ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_FAILED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
	}else{
		ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_SUCCESS,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
	}

	ListMTLock(&pmrafqFilesQueue->lmtListMT);
	ListMTItemDelete(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
	CloseHandle(pmrafqFilesQueueItem->hThread);
	MraFilesQueueItemFree(pmrafqFilesQueueItem);
	ListMTUnLock(&pmrafqFilesQueue->lmtListMT);

	CallService(MS_SYSTEM_THREAD_POP,0,0);
	ExitThread(NO_ERROR);
return(0);
}



DWORD WINAPI MraFilesQueueSendThreadProc(LPVOID lpParameter)
{
	MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->hFilesQueueHandle;
	MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem=((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->pmrafqFilesQueueItem;
    
	BYTE btBuff[65536];
	BOOL bFailed=TRUE,bOK,bConnected=FALSE;
	DWORD dwReceived,dwSended,dwSendBlockSize,dwUpdateTimeNext,dwUpdateTimeCur;
	HANDLE hFile;
	SIZE_T i,j,dwBuffSize=0;
	LPSTR lpszFileName;
	PROTOFILETRANSFERSTATUS pfts={0};


	MEMFREE(lpParameter);
	pfts.cbSize=sizeof(pfts);
	pfts.hContact=pmrafqFilesQueueItem->hContact;
	pfts.sending=pmrafqFilesQueueItem->bSending;	//true if sending, false if receiving
	//pfts.files;
	pfts.totalFiles=pmrafqFilesQueueItem->dwFilesCount;
	pfts.currentFileNumber=0;
	pfts.totalBytes=pmrafqFilesQueueItem->dwFilesTotalSize;
	pfts.totalProgress=0;
	pfts.workingDir=pmrafqFilesQueueItem->lpszPath;
	//pfts.currentFile;
	//pfts.currentFileSize;
	//pfts.currentFileProgress;
	//pfts.currentFileTime;  //as seconds since 1970
	dwSendBlockSize=DB_Mra_GetDword(NULL,"FileSendBlockSize",MRA_DEFAULT_FILE_SEND_BLOCK_SIZE);
	if (dwSendBlockSize>sizeof(btBuff)) dwSendBlockSize=sizeof(btBuff);
	if (dwSendBlockSize<512) dwSendBlockSize=MRA_DEFAULT_FILE_SEND_BLOCK_SIZE;


	CallService(MS_SYSTEM_THREAD_PUSH,0,0);
	//ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_SENTREQUEST,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);

	if (MraFilesQueueConnectIn(pmrafqFilesQueueItem))
	{
		bConnected=TRUE;
	}else{
		if (MraFilesQueueConnectOut(pmrafqFilesQueueItem)) bConnected=TRUE;
	}

	if (bConnected)
	{// email verifyed
		bFailed=FALSE;
		for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
		{// seiding files
			ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_NEXTFILE,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);

			dwBuffSize=0;
			while(TRUE)
			{
				dwReceived=Netlib_Recv(pmrafqFilesQueueItem->hConnection,((LPSTR)btBuff+dwBuffSize),(sizeof(btBuff)-dwBuffSize),0);
				if (dwReceived==0 || dwReceived==SOCKET_ERROR)
				{
					bFailed=TRUE;
					break;
				}else{
					dwBuffSize+=dwReceived;
					if (MemoryFindByte((dwBuffSize-dwReceived),btBuff,dwBuffSize,0)) break;
				}
			}// end while (file name passible received)*/

			if (bFailed==FALSE)
			{// ...received
				if (dwBuffSize>(sizeof(MRA_FT_GET_FILE)+1))
				{// file name received
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(LPSTR)btBuff,(sizeof(MRA_FT_GET_FILE)-1),MRA_FT_GET_FILE,(sizeof(MRA_FT_GET_FILE)-1))==CSTR_EQUAL)
					{// MRA_FT_GET_FILE verifyed
						bFailed=TRUE;
						for(j=0;j<pmrafqFilesQueueItem->dwFilesCount;j++)
						{
							if (GetFullPathName(pmrafqFilesQueueItem->pmfqfFiles[j].lpszName,(pmrafqFilesQueueItem->pmfqfFiles[j].dwNameLen+1),pmrafqFilesQueueItem->pmfqfFiles[j].lpszName,&lpszFileName))
							if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(((LPSTR)btBuff)+sizeof(MRA_FT_GET_FILE)),(dwBuffSize-(sizeof(MRA_FT_GET_FILE)+1)),lpszFileName,-1)==CSTR_EQUAL)
							{
								bFailed=FALSE;
								break;
							}
						}

						if (bFailed==FALSE)
						{
							hFile=CreateFile(pmrafqFilesQueueItem->pmfqfFiles[j].lpszName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,(FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN),NULL);
							if (hFile!=INVALID_HANDLE_VALUE)
							{
								bOK=FALSE;
								dwUpdateTimeNext=GetTickCount();
								pfts.currentFileNumber=i;
								pfts.currentFile=pmrafqFilesQueueItem->pmfqfFiles[j].lpszName;
								pfts.currentFileSize=pmrafqFilesQueueItem->pmfqfFiles[j].dwSize;
								pfts.currentFileProgress=0;
								//pfts.currentFileTime;  //as seconds since 1970
								ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);

								while(TRUE)
								{// read and sending
									if (ReadFile(hFile,btBuff,dwSendBlockSize,&dwBuffSize,NULL))
									{
										dwSended=Netlib_Send(pmrafqFilesQueueItem->hConnection,(LPSTR)btBuff,dwBuffSize,0);
										if (dwSended==0 || dwSended==SOCKET_ERROR)
										{
											break;
										}else{
											pfts.currentFileProgress+=dwSended;
											pfts.totalProgress+=dwSended;
											if (pfts.currentFileProgress>=pmrafqFilesQueueItem->pmfqfFiles[j].dwSize)
											{
												dwUpdateTimeNext=GetTickCount();// force stat update
												bOK=TRUE;
												break;
											}

											// progress updates
											dwUpdateTimeCur=GetTickCount();
											if (dwUpdateTimeNext<=dwUpdateTimeCur)
											{// we update it
												dwUpdateTimeNext=dwUpdateTimeCur+MRA_FILES_QUEUE_PROGRESS_INTERVAL;
												//pfts.currentFileProgress=dwSended;
												ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);
											}
										}
									}else{// read failure
										break;
									}
								}// end while
								CloseHandle(hFile);

								if (bOK==FALSE)
								{// file recv failed
									bFailed=TRUE;
									break;
								}
								ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);
							}else{// err on read file
								bFailed=TRUE;
								break;
							}
						}else{
							break;
						}
					}else{// err on receive, trash
						bFailed=TRUE;
						break;
					}
				}else{// bad file name or trash
					bFailed=TRUE;
					break;
				}
			}else{// err on receive
				bFailed=TRUE;
				break;
			}
		}// end for

		Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
		pmrafqFilesQueueItem->hConnection=NULL;
	}

	if (bFailed)
	{
		char szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;
		if (DB_Mra_GetStaticString(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,sizeof(szEMail),&dwEMailSize))
		{
			MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_DECLINE,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,NULL,0);
		}
		ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_FAILED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
	}else{
		ProtoBroadcastAck(PROTOCOL_NAME,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_SUCCESS,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
	}

	ListMTLock(&pmrafqFilesQueue->lmtListMT);
	ListMTItemDelete(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
	CloseHandle(pmrafqFilesQueueItem->hThread);
	MraFilesQueueItemFree(pmrafqFilesQueueItem);
	ListMTUnLock(&pmrafqFilesQueue->lmtListMT);

	CallService(MS_SYSTEM_THREAD_POP,0,0);
	ExitThread(NO_ERROR);
return(0);
}
