#include "Mra.h"
#include "MraFilesQueue.h"
#include "proto.h"




#define MRA_FT_HELLO	"MRA_FT_HELLO"
#define MRA_FT_GET_FILE	"MRA_FT_GET_FILE"



typedef struct
{
	LIST_MT			lmtListMT;
	DWORD			dwSendTimeOutInterval;
} MRA_FILES_QUEUE;



struct MRA_FILES_QUEUE_FILE
{
	LPWSTR		lpwszName;
	SIZE_T		dwNameLen;
	DWORDLONG	dwSize;
};




typedef struct
{
	// internal
	LIST_MT_ITEM			lmtListMTItem;
	BOOL					bIsWorking;
	DWORD					dwSendTime;
	// external
	DWORD					dwIDRequest;
	DWORD					dwFlags;
	HANDLE					hContact;
	DWORDLONG				dwFilesCount;
	DWORDLONG				dwFilesTotalSize;
	MRA_FILES_QUEUE_FILE	*pmfqfFiles;
	LPWSTR					pwszFilesList;
	LPWSTR					pwszDescription;
	MRA_ADDR_LIST			malAddrList;
	LPWSTR					lpwszPath;
	SIZE_T					dwPathSize;
	BOOL					bSending;
	HANDLE					hConnection;
	HANDLE					hListen;
	HANDLE					hThread;
	HANDLE					hWaitHandle;
	HANDLE					hMraMrimProxyData;

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
//DWORD			MraFilesQueueItemFindByEMail		(HANDLE hFilesQueueHandle,LPSTR lpszEMail,SIZE_T dwEMailSize,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem);
void			MraFilesQueueItemFree				(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
SIZE_T			MraFilesQueueGetLocalAddressesList	(LPSTR lpszBuff,SIZE_T dwBuffSize,DWORD dwPort);

BOOL			MraFilesQueueHandCheck				(HANDLE hConnection,MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem);
void			MraFilesQueueConnectionReceived		(HANDLE hNewConnection,DWORD dwRemoteIP,void *pExtra);

void			MraFilesQueueRecvThreadProc			(LPVOID lpParameter);
void			MraFilesQueueSendThreadProc			(LPVOID lpParameter);



void MraFilesQueueDlgEnableDirectConsControls(HWND hWndDlg,BOOL bEnabled)
{
	WORD wMraFilesControlsList[]={
							IDC_FILE_SEND_NOOUTCONNECTIONONRECEIVE,
							IDC_FILE_SEND_NOOUTCONNECTIONONSEND,
							IDC_FILE_SEND_IGNORYADDITIONALPORTS,
							IDC_FILE_SEND_HIDE_MY_ADDRESSES,
							IDC_FILE_SEND_ADD_EXTRA_ADDRESS,
							IDC_FILE_SEND_EXTRA_ADDRESS
	};
	EnableControlsArray(hWndDlg,(WORD*)&wMraFilesControlsList,SIZEOF(wMraFilesControlsList),bEnabled);
	EnableWindow(GetDlgItem(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS),(bEnabled && IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS)));
}

INT_PTR CALLBACK MraFilesQueueDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			WCHAR szBuff[MAX_PATH];

			TranslateDialogDefault(hWndDlg);

			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_ENABLE_DIRECT_CONN,DB_Mra_GetByte(NULL,"FileSendEnableDirectConn",MRA_DEF_FS_ENABLE_DIRECT_CONN));
			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_NOOUTCONNECTIONONRECEIVE,DB_Mra_GetByte(NULL,"FileSendNoOutConnOnRcv",MRA_DEF_FS_NO_OUT_CONN_ON_RCV));
			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_NOOUTCONNECTIONONSEND,DB_Mra_GetByte(NULL,"FileSendNoOutConnOnSend",MRA_DEF_FS_NO_OUT_CONN_ON_SEND));
			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_IGNORYADDITIONALPORTS,DB_Mra_GetByte(NULL,"FileSendIgnoryAdditionalPorts",MRA_DEF_FS_IGNORY_ADDITIONAL_PORTS));
			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_HIDE_MY_ADDRESSES,DB_Mra_GetByte(NULL,"FileSendHideMyAddresses",MRA_DEF_FS_HIDE_MY_ADDRESSES));
			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS,DB_Mra_GetByte(NULL,"FileSendAddExtraAddresses",MRA_DEF_FS_ADD_EXTRA_ADDRESSES));
			if (DB_Mra_GetStaticStringW(NULL,"FileSendExtraAddresses",szBuff,SIZEOF(szBuff),NULL))
			{
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS,szBuff);
			}

			CHECK_DLG_BUTTON(hWndDlg,IDC_FILE_SEND_ENABLE_MRIMPROXY_CONS,DB_Mra_GetByte(NULL,"FileSendEnableMRIMProxyCons",MRA_DEF_FS_ENABLE_MRIM_PROXY_CONS));
			
			SetDlgItemInt(hWndDlg,IDC_FILE_SEND_BLOCK_SIZE,DB_Mra_GetDword(NULL,"FileSendBlockSize",MRA_DEFAULT_FILE_SEND_BLOCK_SIZE),FALSE);

			MraFilesQueueDlgEnableDirectConsControls(hWndDlg,IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ENABLE_DIRECT_CONN));
		}
		return(TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam)==IDC_FILE_SEND_ENABLE_DIRECT_CONN)
		{
			MraFilesQueueDlgEnableDirectConsControls(hWndDlg,IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ENABLE_DIRECT_CONN));
		}

		if (LOWORD(wParam)==IDC_FILE_SEND_ADD_EXTRA_ADDRESS)
		{
			EnableWindow(GetDlgItem(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS));
		}

		if ((LOWORD(wParam)==IDC_FILE_SEND_EXTRA_ADDRESS || LOWORD(wParam)==IDC_FILE_SEND_BLOCK_SIZE) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) ) return(FALSE);
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				WCHAR szBuff[MAX_PATH];

				DB_Mra_SetByte(NULL,"FileSendEnableDirectConn",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ENABLE_DIRECT_CONN));
				DB_Mra_SetByte(NULL,"FileSendNoOutConnOnRcv",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_NOOUTCONNECTIONONRECEIVE));
				DB_Mra_SetByte(NULL,"FileSendNoOutConnOnSend",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_NOOUTCONNECTIONONSEND));
				DB_Mra_SetByte(NULL,"FileSendIgnoryAdditionalPorts",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_IGNORYADDITIONALPORTS));
				DB_Mra_SetByte(NULL,"FileSendHideMyAddresses",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_HIDE_MY_ADDRESSES));
				DB_Mra_SetByte(NULL,"FileSendAddExtraAddresses",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ADD_EXTRA_ADDRESS));
				GET_DLG_ITEM_TEXT(hWndDlg,IDC_FILE_SEND_EXTRA_ADDRESS,szBuff,SIZEOF(szBuff));
				DB_Mra_SetStringW(NULL,"FileSendExtraAddresses",szBuff);
				DB_Mra_SetDword(NULL,"FileSendBlockSize",(DWORD)GetDlgItemInt(hWndDlg,IDC_FILE_SEND_BLOCK_SIZE,NULL,FALSE));
				DB_Mra_SetByte(NULL,"FileSendEnableMRIMProxyCons",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_FILE_SEND_ENABLE_MRIMPROXY_CONS));
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


HANDLE MraFilesQueueItemProxyByID(HANDLE hFilesQueueHandle,DWORD dwIDRequest)
{
	HANDLE hRet=NULL;
	MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

	if (MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem)==NO_ERROR)
	{
		hRet=pmrafqFilesQueueItem->hMraMrimProxyData;
	}
return(hRet);
}


/*DWORD MraFilesQueueItemFindByEMail(HANDLE hFilesQueueHandle,LPSTR lpszEMail,SIZE_T dwEMailSize,MRA_FILES_QUEUE_ITEM **ppmrafqFilesQueueItem)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		CHAR szEMailLocal[MAX_EMAIL_LEN];
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
			if (DB_Mra_GetStaticStringA(pmrafqFilesQueueItem->hContact,"e-mail",szEMailLocal,SIZEOF(szEMailLocal),&dwEMailLocalSize))
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
}*/


void MraFilesQueueItemFree(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{
 	LIST_MT *plmtListMT=(LIST_MT*)pmrafqFilesQueueItem->lmtListMTItem.lpListMT;

	for(SIZE_T i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
	{
		MEMFREE(pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName);
	}
	MEMFREE(pmrafqFilesQueueItem->pmfqfFiles);
	MEMFREE(pmrafqFilesQueueItem->pwszFilesList);
	MEMFREE(pmrafqFilesQueueItem->pwszDescription);
	MraAddrListFree(&pmrafqFilesQueueItem->malAddrList);
	MraMrimProxyFree(pmrafqFilesQueueItem->hMraMrimProxyData);
	MEMFREE(pmrafqFilesQueueItem->lpwszPath);
	ListMTLock(plmtListMT);
	ListMTItemDelete(plmtListMT,&pmrafqFilesQueueItem->lmtListMTItem);
	ListMTUnLock(plmtListMT);
	MEMFREE(pmrafqFilesQueueItem);
}





SIZE_T MraFilesQueueGetLocalAddressesList(LPSTR lpszBuff,SIZE_T dwBuffSize,DWORD dwPort)
{
	CHAR szHostName[MAX_PATH]={0};
	LPSTR lpszCurPos=lpszBuff;

	if (DB_Mra_GetByte(NULL,"FileSendHideMyAddresses",MRA_DEF_FS_HIDE_MY_ADDRESSES))
	{// не выдаём врагу наш IP адрес!!! :)
		if (DB_Mra_GetByte(NULL,"FileSendAddExtraAddresses",MRA_DEF_FS_ADD_EXTRA_ADDRESSES)==FALSE)
		{// только если не добавляем адрес роутера
			lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-((SIZE_T)lpszCurPos-(SIZE_T)lpszBuff)),MRA_FILES_NULL_ADDRR);
		}
	}else{// создаём список наших IP адресов
		BYTE btAddress[32];
		DWORD dwSelfExternalIP;
		SIZE_T dwAdapter=0;
		hostent *sh;

		dwSelfExternalIP=NTOHL(DB_Mra_GetDword(NULL,"IP",0));
		if (dwSelfExternalIP)
		{
			memmove(&btAddress,&dwSelfExternalIP,sizeof(DWORD));
			lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-((SIZE_T)lpszCurPos-(SIZE_T)lpszBuff)),"%lu.%lu.%lu.%lu:%lu;",btAddress[0],btAddress[1],btAddress[2],btAddress[3],dwPort);
		}

		if (gethostname(szHostName,SIZEOF(szHostName))==0)
		if ((sh=gethostbyname((LPSTR)&szHostName)))
		{
			while(sh->h_addr_list[dwAdapter])
			{
				lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-((SIZE_T)lpszCurPos-(SIZE_T)lpszBuff)),"%s:%lu;",inet_ntoa(*((struct in_addr*)sh->h_addr_list[dwAdapter])),dwPort);
				dwAdapter++;
			}
		}
	}

	if (DB_Mra_GetByte(NULL,"FileSendAddExtraAddresses",MRA_DEF_FS_ADD_EXTRA_ADDRESSES))// добавляем произвольный адрес
	if (DB_Mra_GetStaticStringA(NULL,"FileSendExtraAddresses",szHostName,SIZEOF(szHostName),NULL))
	{
		lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-((SIZE_T)lpszCurPos-(SIZE_T)lpszBuff)),"%s:%lu;",szHostName,dwPort);
	}
return((lpszCurPos-lpszBuff));
}




DWORD MraFilesQueueAddReceive(HANDLE hFilesQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwIDRequest,LPWSTR lpwszFiles,SIZE_T dwFilesSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle && dwIDRequest)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		pmrafqFilesQueueItem=(MRA_FILES_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_FILES_QUEUE_ITEM)+sizeof(LPSTR)+64);
		if (pmrafqFilesQueueItem)
		{
			WCHAR szBuff[MAX_PATH];
			LPWSTR lpwszCurrentItem,lpwszDelimiter,lpwszEndItem;
			SIZE_T dwMemSize,dwAllocatedCount,dwFileNameTotalSize;
			CCSDATA ccs;
			PROTORECVFILET prf;

			//pmrafqFilesQueueItem->lmtListMTItem;
			pmrafqFilesQueueItem->bIsWorking=TRUE;
			pmrafqFilesQueueItem->dwSendTime=GetTickCount();
			pmrafqFilesQueueItem->dwIDRequest=dwIDRequest;
			pmrafqFilesQueueItem->dwFlags=dwFlags;
			pmrafqFilesQueueItem->hContact=hContact;
			if (DB_Mra_GetByte(NULL,"FileSendEnableMRIMProxyCons",MRA_DEF_FS_ENABLE_MRIM_PROXY_CONS)) pmrafqFilesQueueItem->hMraMrimProxyData=MraMrimProxyCreate();


			dwFileNameTotalSize=0;
			dwAllocatedCount=ALLOCATED_COUNT;
			pmrafqFilesQueueItem->dwFilesCount=0;
			pmrafqFilesQueueItem->dwFilesTotalSize=0;
			pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMALLOC((sizeof(MRA_FILES_QUEUE_FILE)*dwAllocatedCount));
			lpwszCurrentItem=lpwszFiles;
			while(TRUE)
			{
				lpwszDelimiter=(LPWSTR)MemoryFind(((SIZE_T)lpwszCurrentItem-(SIZE_T)lpwszFiles),lpwszFiles,(dwFilesSize*sizeof(WCHAR)),";",2);
				if (lpwszDelimiter)
				{
					lpwszEndItem=(LPWSTR)MemoryFind((((SIZE_T)lpwszDelimiter+2)-(SIZE_T)lpwszFiles),lpwszFiles,(dwFilesSize*sizeof(WCHAR)),";",2);
					if (lpwszEndItem)
					{
						if (pmrafqFilesQueueItem->dwFilesCount==dwAllocatedCount)
						{
							dwAllocatedCount*=2;
							pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMREALLOC(pmrafqFilesQueueItem->pmfqfFiles,(sizeof(MRA_FILES_QUEUE_FILE)*dwAllocatedCount));
						}

						dwMemSize=((SIZE_T)lpwszDelimiter-(SIZE_T)lpwszCurrentItem);
						pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].lpwszName=(LPWSTR)MEMALLOC(dwMemSize);
						memmove(pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].lpwszName,lpwszCurrentItem,dwMemSize);
						pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].dwSize=StrToUNum64((LPSTR)((SIZE_T)lpwszDelimiter+1),((SIZE_T)lpwszEndItem-((SIZE_T)lpwszDelimiter+1)));
						pmrafqFilesQueueItem->dwFilesTotalSize+=pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].dwSize;
						pmrafqFilesQueueItem->pmfqfFiles[pmrafqFilesQueueItem->dwFilesCount].dwNameLen=(dwMemSize/sizeof(WCHAR));
						dwFileNameTotalSize+=dwMemSize;
						
						pmrafqFilesQueueItem->dwFilesCount++;
						lpwszCurrentItem=(lpwszEndItem+1);
					}else{
						break;
					}
				}else{
					break;
				}
			}
			pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMREALLOC(pmrafqFilesQueueItem->pmfqfFiles,(sizeof(MRA_FILES_QUEUE_FILE)*(pmrafqFilesQueueItem->dwFilesCount+4)));

			dwMemSize=(((pmrafqFilesQueueItem->dwFilesCount+4)*64)+(dwFileNameTotalSize*sizeof(WCHAR))+(dwAddresesSize*sizeof(WCHAR))+128);
			pmrafqFilesQueueItem->pwszFilesList=(LPWSTR)MEMALLOC(dwMemSize);
			pmrafqFilesQueueItem->pwszDescription=(LPWSTR)MEMALLOC(dwMemSize);


			lpwszDelimiter=pmrafqFilesQueueItem->pwszFilesList;
			lpwszCurrentItem=pmrafqFilesQueueItem->pwszDescription;
			StrFormatByteSizeW(pmrafqFilesQueueItem->dwFilesTotalSize,szBuff,SIZEOF(szBuff));
			lpwszCurrentItem+=mir_sntprintf(lpwszCurrentItem,((dwMemSize-((SIZE_T)lpwszCurrentItem-(SIZE_T)pmrafqFilesQueueItem->pwszDescription))/sizeof(WCHAR)),L"%I64u Files (%s)\r\n",pmrafqFilesQueueItem->dwFilesCount,szBuff);

			// description + filesnames
			for(SIZE_T i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{
				lpwszDelimiter+=mir_sntprintf(lpwszDelimiter,((dwMemSize-((SIZE_T)lpwszDelimiter-(SIZE_T)pmrafqFilesQueueItem->pwszFilesList))/sizeof(WCHAR)),L"%s; ",pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName);
				StrFormatByteSizeW(pmrafqFilesQueueItem->pmfqfFiles[i].dwSize,szBuff,SIZEOF(szBuff));
				lpwszCurrentItem+=mir_sntprintf(lpwszCurrentItem,((dwMemSize-((SIZE_T)lpwszCurrentItem-(SIZE_T)pmrafqFilesQueueItem->pwszDescription))/sizeof(WCHAR)),L"%s - %s\r\n",pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName,szBuff);
			}

			lpwszCurrentItem+=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszAddreses,dwAddresesSize,lpwszCurrentItem,((dwMemSize-((SIZE_T)lpwszCurrentItem-(SIZE_T)pmrafqFilesQueueItem->pwszDescription))/sizeof(WCHAR)));
			(*lpwszCurrentItem)=0;
			//lpwszCurrentItem++;


			MraAddrListGetFromBuff(lpszAddreses,dwAddresesSize,&pmrafqFilesQueueItem->malAddrList);
			MraAddrListStoreToContact(pmrafqFilesQueueItem->hContact,&pmrafqFilesQueueItem->malAddrList);

			ListMTLock(&pmrafqFilesQueue->lmtListMT);
			ListMTItemAdd(&pmrafqFilesQueue->lmtListMT,&pmrafqFilesQueueItem->lmtListMTItem,pmrafqFilesQueueItem);
			ListMTUnLock(&pmrafqFilesQueue->lmtListMT);


			// Send chain event

			ccs.szProtoService=PSR_FILE;
			ccs.hContact=hContact;
			ccs.wParam=0;
			ccs.lParam=(LPARAM)&prf;
			prf.flags=PREF_UNICODE;
			prf.timestamp=_time32(NULL);
			prf.tszDescription=pmrafqFilesQueueItem->pwszDescription;
			prf.fileCount=1;//pmrafqFilesQueueItem->dwFilesCount;
			prf.ptszFiles=&pmrafqFilesQueueItem->pwszFilesList;
			prf.lParam=dwIDRequest;
 
			CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);

			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



DWORD MraFilesQueueAddSend(HANDLE hFilesQueueHandle,DWORD dwFlags,HANDLE hContact,LPWSTR *plpwszFiles,SIZE_T dwFilesCount,DWORD *pdwIDRequest)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;


		pmrafqFilesQueueItem=(MRA_FILES_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_FILES_QUEUE_ITEM)+sizeof(LPSTR)+64);
		if (pmrafqFilesQueueItem)
		{
			SIZE_T i;
			ULARGE_INTEGER uliFileSize;
			WIN32_FILE_ATTRIBUTE_DATA wfad;

			//pmrafqFilesQueueItem->lmtListMTItem;
			pmrafqFilesQueueItem->bIsWorking=TRUE;
			pmrafqFilesQueueItem->dwSendTime=GetTickCount();
			pmrafqFilesQueueItem->dwIDRequest=InterlockedIncrement((LONG volatile*)&masMraSettings.dwCMDNum);// уникальный, рандомный идентификатор
			pmrafqFilesQueueItem->dwFlags=dwFlags;
			pmrafqFilesQueueItem->hContact=hContact;
			if (DB_Mra_GetByte(NULL,"FileSendEnableMRIMProxyCons",MRA_DEF_FS_ENABLE_MRIM_PROXY_CONS)) pmrafqFilesQueueItem->hMraMrimProxyData=MraMrimProxyCreate();
			pmrafqFilesQueueItem->dwFilesCount=dwFilesCount;
			pmrafqFilesQueueItem->pmfqfFiles=(MRA_FILES_QUEUE_FILE*)MEMALLOC((sizeof(MRA_FILES_QUEUE_FILE)*(pmrafqFilesQueueItem->dwFilesCount+1)));
			pmrafqFilesQueueItem->dwFilesTotalSize=0;

			for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{
				if (GetFileAttributesExW(plpwszFiles[i],GetFileExInfoStandard,&wfad))
				{
					uliFileSize.LowPart=wfad.nFileSizeLow;
					uliFileSize.HighPart=wfad.nFileSizeHigh;
					pmrafqFilesQueueItem->pmfqfFiles[i].dwSize=uliFileSize.QuadPart;
					pmrafqFilesQueueItem->dwFilesTotalSize+=uliFileSize.QuadPart;
				}else{
					pmrafqFilesQueueItem->pmfqfFiles[i].dwSize=0;
				}
				pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen=lstrlenW(plpwszFiles[i]);
				pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName=(LPWSTR)MEMALLOC((pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen*sizeof(WCHAR)));
				if (pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName)
				{
					memmove(pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName,plpwszFiles[i],(pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen*sizeof(WCHAR)));
				}
			}
			//pmrafqFilesQueueItem->malAddrList.dwAddrCount=0;
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
				
				pmrafqFilesQueueItem->hThread=(HANDLE)mir_forkthread((pThreadFunc)MraFilesQueueSendThreadProc,pmftpp);
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



DWORD MraFilesQueueAccept(HANDLE hFilesQueueHandle,DWORD dwIDRequest,LPWSTR lpwszPath,SIZE_T dwPathSize)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle && lpwszPath && dwPathSize)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		if ((dwRetErrorCode=MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem))==NO_ERROR)
		{
			MRA_FILES_THREADPROC_PARAMS *pmftpp=(MRA_FILES_THREADPROC_PARAMS*)MEMALLOC(sizeof(MRA_FILES_THREADPROC_PARAMS));
			pmrafqFilesQueueItem->lpwszPath=(LPWSTR)MEMALLOC((dwPathSize*sizeof(WCHAR)));
			pmrafqFilesQueueItem->dwPathSize=dwPathSize;
			memmove(pmrafqFilesQueueItem->lpwszPath,lpwszPath,(dwPathSize*sizeof(WCHAR)));

			if ( (*(WCHAR*)(pmrafqFilesQueueItem->lpwszPath+(pmrafqFilesQueueItem->dwPathSize-1)))!='\\')
			{// add slash at the end if needed
				(*(WCHAR*)(pmrafqFilesQueueItem->lpwszPath+pmrafqFilesQueueItem->dwPathSize))='\\';
				pmrafqFilesQueueItem->dwPathSize++;
				(*(WCHAR*)(pmrafqFilesQueueItem->lpwszPath+pmrafqFilesQueueItem->dwPathSize))=0;
			}

			pmftpp->hFilesQueueHandle=hFilesQueueHandle;
			pmftpp->pmrafqFilesQueueItem=pmrafqFilesQueueItem;
			
			pmrafqFilesQueueItem->hThread=(HANDLE)mir_forkthread((pThreadFunc)MraFilesQueueRecvThreadProc,pmftpp);
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
			InterlockedExchange((volatile LONG*)&pmrafqFilesQueueItem->bIsWorking,FALSE);

			if (bSendDecline)
			{
				CHAR szEMail[MAX_EMAIL_LEN];
				SIZE_T dwEMailSize;

				if (DB_Mra_GetStaticStringA(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
				{
					MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_DECLINE,szEMail,dwEMailSize,dwIDRequest,NULL,0);
				}
			}
			
			MraMrimProxyCloseConnection(pmrafqFilesQueueItem->hMraMrimProxyData);

			Netlib_CloseHandle(pmrafqFilesQueueItem->hListen);
			pmrafqFilesQueueItem->hListen=NULL;

			Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
			pmrafqFilesQueueItem->hConnection=NULL;

			SetEvent(pmrafqFilesQueueItem->hWaitHandle);

			if (pmrafqFilesQueueItem->hThread==NULL)
			{
				MraFilesQueueItemFree(pmrafqFilesQueueItem);
			}
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraFilesQueueStartMrimProxy(HANDLE hFilesQueueHandle,DWORD dwIDRequest)
{
	DWORD dwRetErrorCode;

	if (hFilesQueueHandle && DB_Mra_GetByte(NULL,"FileSendEnableMRIMProxyCons",MRA_DEF_FS_ENABLE_MRIM_PROXY_CONS))
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem;

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		if ((dwRetErrorCode=MraFilesQueueItemFindByID(hFilesQueueHandle,dwIDRequest,&pmrafqFilesQueueItem))==NO_ERROR)
		{//***deb
			if (pmrafqFilesQueueItem->bSending==FALSE)
			{// receiving
				SetEvent(pmrafqFilesQueueItem->hWaitHandle);// cancel wait incomming connection
			}else{// sending

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
			MraAddrListGetFromBuff(lpszAddreses,dwAddresesSize,&pmrafqFilesQueueItem->malAddrList);
			MraAddrListStoreToContact(pmrafqFilesQueueItem->hContact,&pmrafqFilesQueueItem->malAddrList);

			pmrafqFilesQueueItem->hConnection=NULL;
			SetEvent(pmrafqFilesQueueItem->hWaitHandle);
		}
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



BOOL MraFilesQueueHandCheck(HANDLE hConnection,MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{
	BOOL bRet=FALSE;

	if (hConnection && pmrafqFilesQueueItem)
	{
		CHAR szEMail[MAX_EMAIL_LEN]={0},szEMailMy[MAX_EMAIL_LEN]={0};
		BYTE btBuff[((MAX_EMAIL_LEN*2)+(sizeof(MRA_FT_HELLO)*2)+8)]={0};
		SIZE_T dwEMailSize,dwEMailMySize,dwBuffSize;

		DB_Mra_GetStaticStringA(NULL,"e-mail",szEMailMy,SIZEOF(szEMailMy),&dwEMailMySize);BuffToLowerCase(szEMailMy,szEMailMy,dwEMailMySize);
		DB_Mra_GetStaticStringA(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize);BuffToLowerCase(szEMail,szEMail,dwEMailSize);

		if (pmrafqFilesQueueItem->bSending==FALSE)
		{// receiving
			dwBuffSize=(mir_snprintf((LPSTR)btBuff,SIZEOF(btBuff),"%s %s",MRA_FT_HELLO,szEMailMy)+1);
			if (dwBuffSize==Netlib_Send(hConnection,(LPSTR)btBuff,dwBuffSize,0))
			{// my email sended
				ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_INITIALISING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
				dwBuffSize=Netlib_Recv(hConnection,(LPSTR)btBuff,sizeof(btBuff),0);
				if ((dwEMailSize+sizeof(MRA_FT_HELLO)+1)==dwBuffSize)
				{// email received
					mir_snprintf(((LPSTR)btBuff+dwBuffSize),(SIZEOF(btBuff)-dwBuffSize),"%s %s",MRA_FT_HELLO,szEMail);
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(LPSTR)btBuff,dwBuffSize,((LPSTR)btBuff+dwBuffSize),dwBuffSize)==CSTR_EQUAL)
					{// email verifyed
						bRet=TRUE;
					}
				}
			}
		}else{// sending
			dwBuffSize=Netlib_Recv(hConnection,(LPSTR)btBuff,sizeof(btBuff),0);
			if ((dwEMailSize+sizeof(MRA_FT_HELLO)+1)==dwBuffSize)
			{// email received
				ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_INITIALISING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
				mir_snprintf(((LPSTR)btBuff+dwBuffSize),(SIZEOF(btBuff)-dwBuffSize),"%s %s",MRA_FT_HELLO,szEMail);
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(LPSTR)btBuff,dwBuffSize,((LPSTR)btBuff+dwBuffSize),dwBuffSize)==CSTR_EQUAL)
				{// email verifyed
					dwBuffSize=(mir_snprintf((LPSTR)btBuff,SIZEOF(btBuff),"%s %s",MRA_FT_HELLO,szEMailMy)+1);
					if (dwBuffSize==Netlib_Send(hConnection,(LPSTR)btBuff,dwBuffSize,0))
					{// my email sended
						bRet=TRUE;
					}
				}
			}
		}
	}
return(bRet);
}


HANDLE MraFilesQueueConnectOut(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{
	HANDLE hRet;

	if (pmrafqFilesQueueItem)
	{
		if (DB_Mra_GetByte(NULL,"FileSendEnableDirectConn",MRA_DEF_FS_ENABLE_DIRECT_CONN) && InterlockedExchangeAdd((volatile LONG*)&pmrafqFilesQueueItem->bIsWorking,0) && ((pmrafqFilesQueueItem->bSending==FALSE && DB_Mra_GetByte(NULL,"FileSendNoOutConnOnRcv",MRA_DEF_FS_NO_OUT_CONN_ON_RCV)==FALSE) || (pmrafqFilesQueueItem->bSending==TRUE && DB_Mra_GetByte(NULL,"FileSendNoOutConnOnSend",MRA_DEF_FS_NO_OUT_CONN_ON_SEND)==FALSE)))
		{
			BOOL bFiltering=FALSE,bIsHTTPSProxyUsed=IsHTTPSProxyUsed(masMraSettings.hNetlibUser);
			DWORD dwLocalPort,dwConnectReTryCount,dwCurConnectReTryCount;
			SIZE_T i,dwAddrCount;
			NETLIBOPENCONNECTION nloc={0};

			dwLocalPort=0;

			if (DB_Mra_GetByte(NULL,"FileSendIgnoryAdditionalPorts",MRA_DEF_FS_IGNORY_ADDITIONAL_PORTS) || bIsHTTPSProxyUsed)
			{// фильтруем порты для одного IP, вместо 3 будем коннектится только к одному
				if (bIsHTTPSProxyUsed)
				{// через https прокси только 443 порт
					dwLocalPort=MRA_SERVER_PORT_HTTPS;
				}else{
					if ((dwLocalPort=DB_Mra_GetWord(NULL,"ServerPort",MRA_DEFAULT_SERVER_PORT))==MRA_SERVER_PORT_STANDART_NLB) dwLocalPort=MRA_SERVER_PORT_STANDART;
				}

				dwAddrCount=0;
				for(i=0;i<pmrafqFilesQueueItem->malAddrList.dwAddrCount;i++)
				{
					if (dwLocalPort==pmrafqFilesQueueItem->malAddrList.pmaliAddress[i].dwPort)
					{
						bFiltering=TRUE;
						dwAddrCount++;
					}
				}
			}
			if (bFiltering==FALSE) dwAddrCount=pmrafqFilesQueueItem->malAddrList.dwAddrCount;

			if (dwAddrCount)
			{
				pmrafqFilesQueueItem->hConnection=NULL;
				dwConnectReTryCount=DB_Mra_GetDword(NULL,"ConnectReTryCountFileSend",MRA_DEFAULT_CONN_RETRY_COUNT_FILES);
				nloc.cbSize=sizeof(nloc);
				nloc.flags=NLOCF_V2;
				nloc.timeout=DB_Mra_GetDword(NULL,"TimeOutConnectFileSend",((MRA_TIMEOUT_DIRECT_CONN-1)/(dwAddrCount*dwConnectReTryCount)));// -1 сек чтобы был запас
				if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout=MRA_TIMEOUT_CONN_MIN;
				if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout=MRA_TIMEOUT_CONN_МАХ;

				for(i=0;i<pmrafqFilesQueueItem->malAddrList.dwAddrCount;i++)
				{// Set up the sockaddr structure
					if (dwLocalPort==pmrafqFilesQueueItem->malAddrList.pmaliAddress[i].dwPort || bFiltering==FALSE)
					{
						ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);

						nloc.szHost=inet_ntoa((*((in_addr*)&pmrafqFilesQueueItem->malAddrList.pmaliAddress[i].dwAddr)));
						nloc.wPort=(WORD)pmrafqFilesQueueItem->malAddrList.pmaliAddress[i].dwPort;

						dwCurConnectReTryCount=dwConnectReTryCount;
						do{
							pmrafqFilesQueueItem->hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nloc);
						}while(--dwCurConnectReTryCount && pmrafqFilesQueueItem->hConnection==NULL);

						if (pmrafqFilesQueueItem->hConnection)
						{
							ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
							if (MraFilesQueueHandCheck(pmrafqFilesQueueItem->hConnection,pmrafqFilesQueueItem))
							{// связь установленная с тем кем нужно
								DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"IP",0));
								DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"IP",HTONL(pmrafqFilesQueueItem->malAddrList.pmaliAddress[i].dwAddr));
								break;
							}else{// кажется не туда подключились :)
								Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
								pmrafqFilesQueueItem->hConnection=NULL;
							}
						}
					}
				}
			}
		}
		hRet=pmrafqFilesQueueItem->hConnection;
	}else{
		hRet=NULL;
	}
return(hRet);
}


LPWSTR GetFileNameFromFullPathW(LPWSTR lpwszFullPath,SIZE_T dwFullPathSize)
{
	LPWSTR lpwszFileName=lpwszFullPath,lpwszCurPos;

	lpwszCurPos=(lpwszFullPath+dwFullPathSize);
	for(;lpwszCurPos>lpwszFullPath;lpwszCurPos--)
	{
		if ((*lpwszCurPos)=='\\')
		{
			lpwszFileName=(lpwszCurPos+1);
			break;
		}
	}
return(lpwszFileName);
}



HANDLE MraFilesQueueConnectIn(MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem)
{
	HANDLE hRet=NULL;

	if (pmrafqFilesQueueItem)
	if (InterlockedExchangeAdd((volatile LONG*)&pmrafqFilesQueueItem->bIsWorking,0))
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;

		if (DB_Mra_GetStaticStringA(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			CHAR szAddrList[2048]={0};
			SIZE_T dwAddrListSize;

			if (DB_Mra_GetByte(NULL,"FileSendEnableDirectConn",MRA_DEF_FS_ENABLE_DIRECT_CONN))
			{// копируем адреса в соответствии с правилами и начинаем слушать порт
				NETLIBBIND nlbBind={0};

				nlbBind.cbSize=sizeof(nlbBind);
				nlbBind.pfnNewConnectionV2=MraFilesQueueConnectionReceived;
				nlbBind.wPort=0;
				nlbBind.pExtra=(LPVOID)pmrafqFilesQueueItem;

				pmrafqFilesQueueItem->hListen=(HANDLE)CallService(MS_NETLIB_BINDPORT,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nlbBind);
				if (pmrafqFilesQueueItem->hListen)
				{
					ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_LISTENING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
					dwAddrListSize=MraFilesQueueGetLocalAddressesList(szAddrList,sizeof(szAddrList),nlbBind.wPort);
				}else{// не смогли слушать порт, хз почему.
					ShowFormatedErrorMessage(L"Files exchange: cant create listen soscket, will try connect to remonte host. Error",GetLastError());
					
					//dwAddrListSize=0;
					memmove(szAddrList,MRA_FILES_NULL_ADDRR,sizeof(MRA_FILES_NULL_ADDRR));
					dwAddrListSize=(sizeof(MRA_FILES_NULL_ADDRR)-1);
				}
			}else{// подставляем ложный адрес, чтобы точно не подключились и не слушаем порт
				memmove(szAddrList,MRA_FILES_NULL_ADDRR,sizeof(MRA_FILES_NULL_ADDRR));
				dwAddrListSize=(sizeof(MRA_FILES_NULL_ADDRR)-1);
			}

			if (dwAddrListSize)
			{
				pmrafqFilesQueueItem->hWaitHandle=CreateEvent(NULL,TRUE,FALSE,NULL);
				if (pmrafqFilesQueueItem->bSending==FALSE)
				{// запрашиваем зеркальное соединение, тк сами подключится не смогли
					MraSendCommand_FileTransferAck(FILE_TRANSFER_MIRROR,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,(LPBYTE)szAddrList,dwAddrListSize);
				}else{// здесь отправляем запрос на передачу(установление соединения)
					// создаём текстовый список файлов для отправки другой стороне
					LPWSTR lpwszFiles,lpwszCurPos;
					SIZE_T dwFilesSize;

					dwFilesSize=((MAX_PATH*2)*pmrafqFilesQueueItem->dwFilesCount);
					lpwszFiles=(LPWSTR)MEMALLOC((dwFilesSize*sizeof(WCHAR)));
					if (lpwszFiles)
					{
						lpwszCurPos=lpwszFiles;
						for(SIZE_T i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
						{
							lpwszCurPos+=mir_sntprintf(lpwszCurPos,(dwFilesSize-((SIZE_T)lpwszCurPos-(SIZE_T)lpwszFiles)),L"%s;%I64u;",GetFileNameFromFullPathW(pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName,pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen),pmrafqFilesQueueItem->pmfqfFiles[i].dwSize);
						}
						dwFilesSize=(lpwszCurPos-lpwszFiles);// size in WCHARs

						if (pmrafqFilesQueueItem->hMraMrimProxyData)
						{// устанавливаем данные для майловской прокси, если она разрешена
							LPSTR lpszFiles;
							SIZE_T dwFilesSizeA;

							dwFilesSizeA=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszFiles,dwFilesSize,NULL,0,NULL,NULL);
							lpszFiles=(LPSTR)MEMALLOC((dwFilesSizeA+MAX_PATH));
							if (lpszFiles)
							{
								dwFilesSizeA=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszFiles,dwFilesSize,lpszFiles,(dwFilesSizeA+MAX_PATH),NULL,NULL);
								MraMrimProxySetData(pmrafqFilesQueueItem->hMraMrimProxyData,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,MRIM_PROXY_TYPE_FILES,lpszFiles,dwFilesSizeA,NULL,0,NULL);
								MEMFREE(lpszFiles);
							}
							//MraMrimProxySetData(pmrafqFilesQueueItem->hMraMrimProxyData,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,MRIM_PROXY_TYPE_FILES,(LPSTR)lpwszFiles,dwFilesSize,NULL,0,NULL);
						}
						MraSendCommand_FileTransfer(szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,pmrafqFilesQueueItem->dwFilesTotalSize,lpwszFiles,dwFilesSize,szAddrList,dwAddrListSize);
						
						MEMFREE(lpwszFiles);
					}
				}
				WaitForSingleObjectEx(pmrafqFilesQueueItem->hWaitHandle,INFINITE,FALSE);
				CloseHandle(pmrafqFilesQueueItem->hWaitHandle);
				pmrafqFilesQueueItem->hWaitHandle=NULL;
			}
		}
		hRet=pmrafqFilesQueueItem->hConnection;
	}
return(hRet);
}

// This function is called from the Netlib when someone is connecting to
// one of our incomming DC ports
void MraFilesQueueConnectionReceived(HANDLE hNewConnection,DWORD dwRemoteIP,void *pExtra)
{
	if (pExtra)
	{
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem=(MRA_FILES_QUEUE_ITEM*)pExtra;

		ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		if (MraFilesQueueHandCheck(hNewConnection,pmrafqFilesQueueItem))
		{// связь установленная с тем кем нужно
			pmrafqFilesQueueItem->hConnection=hNewConnection;
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
			DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"OldIP",DB_Mra_GetDword(pmrafqFilesQueueItem->hContact,"IP",0));
			DB_Mra_SetDword(pmrafqFilesQueueItem->hContact,"IP",dwRemoteIP);
			SetEvent(pmrafqFilesQueueItem->hWaitHandle);
		}else{// кажется кто то не туда подключилися :)
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_LISTENING,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
			Netlib_CloseHandle(hNewConnection);
		}
	}else{
		Netlib_CloseHandle(hNewConnection);
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,TranslateW(L"MraFilesQueueConnectionReceived: connection accepted, but pExtra=NULL, this is miranda bug."));
		DebugBreak();
	}
}



void MraFilesQueueRecvThreadProc(LPVOID lpParameter)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpParameter)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem=((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->pmrafqFilesQueueItem;
	    
		WCHAR wszFileName[MAX_FILEPATH]={0};
		WCHAR szErrorText[2048];
		BYTE btBuff[BUFF_SIZE_RCV];
		BOOL bContinue,bFailed,bOK,bConnected;
		DWORD dwReceived,dwUpdateTimeNext,dwUpdateTimeCur;
		HANDLE hFile;
		SIZE_T i,dwBuffSizeUsed;
		LARGE_INTEGER liFileSize;
		NETLIBSELECT nls={0};
		PROTOFILETRANSFERSTATUS pfts={0};

		MEMFREE(lpParameter);

		bFailed=TRUE;
		bConnected=FALSE;
		nls.cbSize=sizeof(nls);
		pfts.cbSize=sizeof(pfts);
		pfts.hContact=pmrafqFilesQueueItem->hContact;
		pfts.flags=(PFTS_RECEIVING|PFTS_UNICODE);//		pfts.sending=pmrafqFilesQueueItem->bSending;	//true if sending, false if receiving
		//pfts.files;
		pfts.totalFiles=pmrafqFilesQueueItem->dwFilesCount;
		//pfts.currentFileNumber=0;
		pfts.totalBytes=pmrafqFilesQueueItem->dwFilesTotalSize;
		//pfts.totalProgress=0;
		pfts.wszWorkingDir=pmrafqFilesQueueItem->lpwszPath;
		//pfts.currentFile;
		//pfts.currentFileSize;
		//pfts.currentFileProgress;
		//pfts.currentFileTime;  //as seconds since 1970

		if (MraFilesQueueConnectOut(pmrafqFilesQueueItem))
		{
			bConnected=TRUE;
		}else{
			if (MraFilesQueueConnectIn(pmrafqFilesQueueItem))
			{
				bConnected=TRUE;
			}else{
				if (InterlockedExchangeAdd((volatile LONG*)&pmrafqFilesQueueItem->bIsWorking,0))
				{
					ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKRESULT_CONNECTPROXY,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
					if (MraMrimProxyConnect(pmrafqFilesQueueItem->hMraMrimProxyData,&pmrafqFilesQueueItem->hConnection)==NO_ERROR)
					{// подключились к прокси, проверяем та ли сессия (ещё раз, на этот раз сами)
						if (MraFilesQueueHandCheck(pmrafqFilesQueueItem->hConnection,pmrafqFilesQueueItem))
						{// связь установленная с тем кем нужно// pmrafqFilesQueueItem->bSending
							ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
							bConnected=TRUE;
						}
					}
				}
			}
		}

		if (bConnected)
		{// email verifyed
			bFailed=FALSE;
			for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{// receiving files
				pfts.currentFileNumber=i;
				pfts.wszCurrentFile=wszFileName;
				pfts.currentFileSize=pmrafqFilesQueueItem->pmfqfFiles[i].dwSize;
				pfts.currentFileProgress=0;
				//pfts.currentFileTime;  //as seconds since 1970
				
				if ((pmrafqFilesQueueItem->dwPathSize+pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen)<SIZEOF(wszFileName))
				{
					memmove(wszFileName,pmrafqFilesQueueItem->lpwszPath,(pmrafqFilesQueueItem->dwPathSize*sizeof(WCHAR)));
					memmove((wszFileName+pmrafqFilesQueueItem->dwPathSize),pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName,((pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen+1)*sizeof(WCHAR)));
					wszFileName[pmrafqFilesQueueItem->dwPathSize+pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen]=0;
				}else{
					dwRetErrorCode=ERROR_BAD_PATHNAME;
					ShowFormatedErrorMessage(L"Receive files: error",dwRetErrorCode);
					bFailed=TRUE;
					break;
				}

				//***deb add
				//dwBuffSizeUsed=ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_FILERESUME,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);

				ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_NEXTFILE,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
				
				//dwBuffSizeUsed=(mir_snprintf((LPSTR)btBuff,SIZEOF(btBuff),"%s %S",MRA_FT_GET_FILE,pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName)+1);
				memmove(btBuff,MRA_FT_GET_FILE,sizeof(MRA_FT_GET_FILE));
				btBuff[(sizeof(MRA_FT_GET_FILE)-1)]=' ';
				dwBuffSizeUsed=sizeof(MRA_FT_GET_FILE)+WideCharToMultiByte(MRA_CODE_PAGE,0,pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName,pmrafqFilesQueueItem->pmfqfFiles[i].dwNameLen,(LPSTR)(btBuff+sizeof(MRA_FT_GET_FILE)),(SIZEOF(btBuff)-sizeof(MRA_FT_GET_FILE)),NULL,NULL);
				btBuff[dwBuffSizeUsed]=0;
				dwBuffSizeUsed++;

				if (dwBuffSizeUsed==Netlib_Send(pmrafqFilesQueueItem->hConnection,(LPSTR)btBuff,dwBuffSizeUsed,0))
				{// file request sended
					hFile=CreateFileW(wszFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
					if (hFile!=INVALID_HANDLE_VALUE)
					{// file opened/created, pre allocating disk space, for best perfomance
						bOK=FALSE;

						liFileSize.QuadPart=(LONGLONG)pmrafqFilesQueueItem->pmfqfFiles[i].dwSize;
						if (SetFilePointerEx(hFile,liFileSize,NULL,FILE_BEGIN))
						if (SetEndOfFile(hFile))
						{
							liFileSize.QuadPart=0;
							bOK=SetFilePointerEx(hFile,liFileSize,NULL,FILE_BEGIN);
						}

						if (bOK)
						{// disk space pre allocated
							bOK=FALSE;
							bContinue=TRUE;
							dwUpdateTimeNext=GetTickCount();
							nls.dwTimeout=(1000*DB_Mra_GetDword(NULL,"TimeOutReceiveFileData",MRA_DEF_FS_TIMEOUT_RECV));
							nls.hReadConns[0]=pmrafqFilesQueueItem->hConnection;
							ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);

							while(bContinue)
							{
								switch(CallService(MS_NETLIB_SELECT,0,(LPARAM)&nls)){
								case SOCKET_ERROR:
								case 0:// Time out
									dwRetErrorCode=GetLastError();
									ShowFormatedErrorMessage(L"Receive files: error on receive file data",dwRetErrorCode);
									bContinue=FALSE;
									break;
								case 1:
									dwReceived=Netlib_Recv(pmrafqFilesQueueItem->hConnection,(LPSTR)&btBuff,SIZEOF(btBuff),0);
									if (dwReceived==0 || dwReceived==SOCKET_ERROR)
									{
										dwRetErrorCode=GetLastError();
										ShowFormatedErrorMessage(L"Receive files: error on receive file data",dwRetErrorCode);
										bContinue=FALSE;
									}else{
										if (WriteFile(hFile,(LPVOID)&btBuff,dwReceived,&dwReceived,NULL))
										{
											pfts.currentFileProgress+=dwReceived;
											pfts.totalProgress+=dwReceived;
											
											// progress updates
											dwUpdateTimeCur=GetTickCount();
											if (dwUpdateTimeNext<=dwUpdateTimeCur || pfts.currentFileProgress>=pmrafqFilesQueueItem->pmfqfFiles[i].dwSize)
											{// we update it
												dwUpdateTimeNext=dwUpdateTimeCur+MRA_FILES_QUEUE_PROGRESS_INTERVAL;
												ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);

												if (pfts.currentFileProgress>=pmrafqFilesQueueItem->pmfqfFiles[i].dwSize)
												{// file received
													bOK=TRUE;
													bContinue=FALSE;
												}
											}
										}else{// err on write file
											dwRetErrorCode=GetLastError();
											ShowFormatedErrorMessage(L"Receive files: cant write file data, error",dwRetErrorCode);
											bContinue=FALSE;
										}
									}
									break;
								}
							}// end while
						}else{// err allocating file disk space
							dwRetErrorCode=GetLastError();
							mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Receive files: cant allocate disk space for file, size %lu bytes, error"),pmrafqFilesQueueItem->pmfqfFiles[i].dwSize);
							ShowFormatedErrorMessage(szErrorText,dwRetErrorCode);
						}
						CloseHandle(hFile);

						if (bOK==FALSE)
						{// file recv failed
							DeleteFileW(wszFileName);
							bFailed=TRUE;
							break;
						}
					}else{// err on open file
						dwRetErrorCode=GetLastError();
						mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Receive files: cant open file %s, error"),wszFileName);
						ShowFormatedErrorMessage(szErrorText,dwRetErrorCode);
						bFailed=TRUE;
						break;
					}
				}else{// err on send request for file
					dwRetErrorCode=GetLastError();
					mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Receive files: request for file %s not sended, error"),pmrafqFilesQueueItem->pmfqfFiles[i].lpwszName);
					ShowFormatedErrorMessage(szErrorText,NO_ERROR);
					bFailed=TRUE;
					break;
				}
			}// end for

			Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
			pmrafqFilesQueueItem->hConnection=NULL;
		}

		if (bFailed)
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;

			if (DB_Mra_GetStaticStringA(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_ERROR,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,NULL,0);
			}
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_FAILED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		}else{
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_SUCCESS,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		}

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		MraFilesQueueItemFree(pmrafqFilesQueueItem);
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}
}



void MraFilesQueueSendThreadProc(LPVOID lpParameter)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpParameter)
	{
		MRA_FILES_QUEUE *pmrafqFilesQueue=(MRA_FILES_QUEUE*)((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->hFilesQueueHandle;
		MRA_FILES_QUEUE_ITEM *pmrafqFilesQueueItem=((MRA_FILES_THREADPROC_PARAMS*)lpParameter)->pmrafqFilesQueueItem;
	    
		CHAR szFileName[MAX_FILEPATH]={0};
		WCHAR szErrorText[2048];
		BYTE btBuff[BUFF_SIZE_RCV];
		BOOL bFailed=TRUE,bOK,bConnected=FALSE;
		DWORD dwReceived,dwSendBlockSize,dwUpdateTimeNext,dwUpdateTimeCur;
		HANDLE hFile;
		SIZE_T i,j,dwBuffSizeUsed=0;
		LPWSTR lpwszFileName;
		PROTOFILETRANSFERSTATUS pfts={0};

		MEMFREE(lpParameter);

		pfts.cbSize=sizeof(pfts);
		pfts.hContact=pmrafqFilesQueueItem->hContact;
		pfts.flags=(PFTS_SENDING|PFTS_UNICODE);// pfts.sending=pmrafqFilesQueueItem->bSending;	//true if sending, false if receiving
		//pfts.files;
		pfts.totalFiles=pmrafqFilesQueueItem->dwFilesCount;
		//pfts.currentFileNumber=0;
		pfts.totalBytes=pmrafqFilesQueueItem->dwFilesTotalSize;
		//pfts.totalProgress=0;
		pfts.wszWorkingDir=pmrafqFilesQueueItem->lpwszPath;
		//pfts.currentFile;
		//pfts.currentFileSize;
		//pfts.currentFileProgress;
		//pfts.currentFileTime;  //as seconds since 1970

		dwSendBlockSize=DB_Mra_GetDword(NULL,"FileSendBlockSize",MRA_DEFAULT_FILE_SEND_BLOCK_SIZE);
		if (dwSendBlockSize>SIZEOF(btBuff)) dwSendBlockSize=SIZEOF(btBuff);
		if (dwSendBlockSize<512) dwSendBlockSize=MRA_DEFAULT_FILE_SEND_BLOCK_SIZE;

		if (MraFilesQueueConnectIn(pmrafqFilesQueueItem))
		{
			bConnected=TRUE;
		}else{
			if (MraFilesQueueConnectOut(pmrafqFilesQueueItem))
			{
				bConnected=TRUE;
			}else{
				if (InterlockedExchangeAdd((volatile LONG*)&pmrafqFilesQueueItem->bIsWorking,0))
				{
					ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKRESULT_CONNECTPROXY,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
					if (MraMrimProxyConnect(pmrafqFilesQueueItem->hMraMrimProxyData,&pmrafqFilesQueueItem->hConnection)==NO_ERROR)
					{// подключились к прокси, проверяем та ли сессия (ещё раз, на этот раз сами)
						if (MraFilesQueueHandCheck(pmrafqFilesQueueItem->hConnection,pmrafqFilesQueueItem))
						{// связь установленная с тем кем нужно// pmrafqFilesQueueItem->bSending
							ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_CONNECTED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
							bConnected=TRUE;
						}
					}
				}
			}
		}

		if (bConnected)
		{// email verifyed
			bFailed=FALSE;
			for(i=0;i<pmrafqFilesQueueItem->dwFilesCount;i++)
			{// sending files
				ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_NEXTFILE,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);

				dwBuffSizeUsed=0;
				while(TRUE)
				{
					dwReceived=Netlib_Recv(pmrafqFilesQueueItem->hConnection,((LPSTR)btBuff+dwBuffSizeUsed),(SIZEOF(btBuff)-dwBuffSizeUsed),0);
					if (dwReceived==0 || dwReceived==SOCKET_ERROR)
					{// err on receive file name to send
						dwRetErrorCode=GetLastError();
						ShowFormatedErrorMessage(L"Send files: file send request not received, error",dwRetErrorCode);
						bFailed=TRUE;
						break;
					}else{
						dwBuffSizeUsed+=dwReceived;
						if (MemoryFindByte((dwBuffSizeUsed-dwReceived),btBuff,dwBuffSizeUsed,0)) break;
					}
				}// end while (file name passible received)*/


				if (bFailed==FALSE)
				{// ...received
					if (dwBuffSizeUsed>(sizeof(MRA_FT_GET_FILE)+1))
					{// file name received
						if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(LPSTR)btBuff,(sizeof(MRA_FT_GET_FILE)-1),MRA_FT_GET_FILE,(sizeof(MRA_FT_GET_FILE)-1))==CSTR_EQUAL)
						{// MRA_FT_GET_FILE verifyed
							bFailed=TRUE;
							for(j=0;j<pmrafqFilesQueueItem->dwFilesCount;j++)
							{
								lpwszFileName=GetFileNameFromFullPathW(pmrafqFilesQueueItem->pmfqfFiles[j].lpwszName,pmrafqFilesQueueItem->pmfqfFiles[j].dwNameLen);
								szFileName[WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszFileName,(pmrafqFilesQueueItem->pmfqfFiles[j].dwNameLen-(lpwszFileName-pmrafqFilesQueueItem->pmfqfFiles[j].lpwszName)),szFileName,SIZEOF(szFileName),NULL,NULL)]=0;

								if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,(((LPSTR)btBuff)+sizeof(MRA_FT_GET_FILE)),(dwBuffSizeUsed-(sizeof(MRA_FT_GET_FILE)+1)),szFileName,-1)==CSTR_EQUAL)
								{
									bFailed=FALSE;
									break;
								}
							}

							if (bFailed==FALSE)
							{
								hFile=CreateFileW(pmrafqFilesQueueItem->pmfqfFiles[j].lpwszName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,(FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN),NULL);
								if (hFile!=INVALID_HANDLE_VALUE)
								{
									bOK=FALSE;
									dwUpdateTimeNext=GetTickCount();
									pfts.currentFileNumber=i;
									pfts.wszCurrentFile=pmrafqFilesQueueItem->pmfqfFiles[j].lpwszName;
									pfts.currentFileSize=pmrafqFilesQueueItem->pmfqfFiles[j].dwSize;
									pfts.currentFileProgress=0;
									//pfts.currentFileTime;  //as seconds since 1970

									WideCharToMultiByte(MRA_CODE_PAGE,0,pmrafqFilesQueueItem->pmfqfFiles[j].lpwszName,pmrafqFilesQueueItem->pmfqfFiles[j].dwNameLen,szFileName,SIZEOF(szFileName),NULL,NULL);
									ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);

									while(TRUE)
									{// read and sending
										if (ReadFile(hFile,btBuff,dwSendBlockSize,(DWORD*)&dwBuffSizeUsed,NULL))
										{
											dwReceived=Netlib_Send(pmrafqFilesQueueItem->hConnection,(LPSTR)btBuff,dwBuffSizeUsed,0);
											if (dwBuffSizeUsed==dwReceived)
											{
												pfts.currentFileProgress+=dwBuffSizeUsed;
												pfts.totalProgress+=dwBuffSizeUsed;
												
												// progress updates
												dwUpdateTimeCur=GetTickCount();
												if (dwUpdateTimeNext<=dwUpdateTimeCur || pfts.currentFileProgress>=pmrafqFilesQueueItem->pmfqfFiles[j].dwSize)
												{// we update it
													dwUpdateTimeNext=dwUpdateTimeCur+MRA_FILES_QUEUE_PROGRESS_INTERVAL;

													ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_DATA,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,(LPARAM)&pfts);
													
													if (pfts.currentFileProgress>=pmrafqFilesQueueItem->pmfqfFiles[j].dwSize)
													{// file received
														bOK=TRUE;
														break;
													}
												}
											}else{// err on send file data
												dwRetErrorCode=GetLastError();
												ShowFormatedErrorMessage(L"Send files: error on send file data",dwRetErrorCode);
												break;
											}
										}else{// read failure
											dwRetErrorCode=GetLastError();
											ShowFormatedErrorMessage(L"Send files: cant read file data, error",dwRetErrorCode);
											break;
										}
									}// end while
									CloseHandle(hFile);

									if (bOK==FALSE)
									{// file recv failed
										bFailed=TRUE;
										break;
									}
								}else{// err on open file
									dwRetErrorCode=GetLastError();

									mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Send files: cant open file %s, error"),pmrafqFilesQueueItem->pmfqfFiles[j].lpwszName);
									ShowFormatedErrorMessage(szErrorText,dwRetErrorCode);
									bFailed=TRUE;
									break;
								}
							}else{
								mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Send files: requested file: %S - not found in send files list."),(((LPSTR)btBuff)+sizeof(MRA_FT_GET_FILE)));
								ShowFormatedErrorMessage(szErrorText,NO_ERROR);
								bFailed=TRUE;
								break;
							}
						}else{// err on receive, trash
							ShowFormatedErrorMessage(L"Send files: bad file send request - invalid header",NO_ERROR);
							bFailed=TRUE;
							break;
						}
					}else{// bad file name or trash
						ShowFormatedErrorMessage(L"Send files: bad file send request - to small packet",NO_ERROR);
						bFailed=TRUE;
						break;
					}
				}else{
					break;
				}
			}// end for

			Netlib_CloseHandle(pmrafqFilesQueueItem->hConnection);
			pmrafqFilesQueueItem->hConnection=NULL;
		}

		if (bFailed)
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;

			if (DB_Mra_GetStaticStringA(pmrafqFilesQueueItem->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraSendCommand_FileTransferAck(FILE_TRANSFER_STATUS_ERROR,szEMail,dwEMailSize,pmrafqFilesQueueItem->dwIDRequest,NULL,0);
			}
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_FAILED,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		}else{
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmrafqFilesQueueItem->hContact,ACKTYPE_FILE,ACKRESULT_SUCCESS,(HANDLE)pmrafqFilesQueueItem->dwIDRequest,0);
		}

		ListMTLock(&pmrafqFilesQueue->lmtListMT);
		MraFilesQueueItemFree(pmrafqFilesQueueItem);
		ListMTUnLock(&pmrafqFilesQueue->lmtListMT);
	}
}
