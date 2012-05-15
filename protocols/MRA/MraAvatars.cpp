#include "Mra.h"
#include "MraAvatars.h"




#define PA_FORMAT_MAX		7
const LPSTR lpcszExtensions[9]=
{
	".dat",// PA_FORMAT_UNKNOWN
	".png",// PA_FORMAT_PNG
	".jpg",// PA_FORMAT_JPEG
	".ico",// PA_FORMAT_ICON
	".bmp",// PA_FORMAT_BMP
	".gif",// PA_FORMAT_GIF
	".swf",// PA_FORMAT_SWF
	".xml",// PA_FORMAT_XML
	NULL
};


const LPSTR lpcszContentType[9]=
{
	"",					// PA_FORMAT_UNKNOWN
	"image/png",		// PA_FORMAT_PNG
	"image/jpeg",		// PA_FORMAT_JPEG
	"image/icon",		// PA_FORMAT_ICON
	"image/x-xbitmap",	// PA_FORMAT_BMP
	"image/gif",		// PA_FORMAT_GIF
	"",					// PA_FORMAT_SWF
	"",					// PA_FORMAT_XML
	NULL
};



typedef struct
{
	FIFO_MT					ffmtQueueToQuery;
	BOOL					bIsRunning;
	HANDLE					hNetlibUser;
	HANDLE					hThreadEvent;
	DWORD					dwThreadsCount;
	HANDLE					hThread[MAXIMUM_WAIT_OBJECTS];
	LONG					lThreadsRunningCount;
	HANDLE					hAvatarsPath;
} MRA_AVATARS_QUEUE;


typedef struct
{
	// internal
	FIFO_MT_ITEM	ffmtiFifoItem;
	// external
	DWORD			dwAvatarsQueueID;
	DWORD			dwFlags;
	HANDLE			hContact;
} MRA_AVATARS_QUEUE_ITEM;



#define FILETIME_SECOND				((DWORDLONG)10000000)
#define FILETIME_MINUTE				((DWORDLONG)FILETIME_SECOND*60)


char szAvtSectName[MAX_PATH];
#define MRA_AVT_SECT_NAME		szAvtSectName





//#define MEMALLOC(Size)		HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(Size+sizeof(SIZE_T)))
//#define MEMREALLOC(Mem,Size)	HeapReAlloc(GetProcessHeap(),(HEAP_ZERO_MEMORY),(LPVOID)Mem,(Size+sizeof(SIZE_T)))
//#define MEMFREE(Mem)			if (Mem) {HeapFree(GetProcessHeap(),0,(LPVOID)Mem);Mem=NULL;}

#define NETLIB_CLOSEHANDLE(hConnection) {Netlib_CloseHandle(hConnection);hConnection=NULL;}

void			MraAvatarsQueueClear		(HANDLE hAvatarsQueueHandle);
DWORD			MraAvatarsQueueAdd			(HANDLE hAvatarsQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD *pdwAvatarsQueueID);
void			MraAvatarsThreadProc		(LPVOID lpParameter);
HANDLE			MraAvatarsHttpConnect		(HANDLE hNetlibUser,LPSTR lpszHost,DWORD dwPort);



#define MAHTRO_AVT			0
#define MAHTRO_AVTMRIM		1
#define MAHTRO_AVTSMALL		2
#define MAHTRO_AVTSMALLMRIM	3
DWORD			MraAvatarsHttpTransaction	(HANDLE hConnection,DWORD dwRequestType,LPSTR lpszUser,LPSTR lpszDomain,LPSTR lpszHost,DWORD dwReqObj,BOOL bUseKeepAliveConn,DWORD *pdwResultCode,BOOL *pbKeepAlive,DWORD *pdwFormat,SIZE_T *pdwAvatarSize,INTERNET_TIME *pitLastModifiedTime);
BOOL			MraAvatarsGetContactTime	(HANDLE hContact,LPSTR lpszValueName,SYSTEMTIME *pstTime);
void			MraAvatarsSetContactTime	(HANDLE hContact,LPSTR lpszValueName,SYSTEMTIME *pstTime);
DWORD			MraAvatarsGetFileFormat		(LPSTR lpszPath,SIZE_T dwPathSize);





DWORD MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle)
{
	DWORD dwRetErrorCode;

	mir_snprintf(szAvtSectName,SIZEOF(szAvtSectName),"%s Avatars",PROTOCOL_NAMEA);

	if (phAvatarsQueueHandle)
	{
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue;

		pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)MEMALLOC(sizeof(MRA_AVATARS_QUEUE));
		if (pmraaqAvatarsQueue)
		{
			dwRetErrorCode=FifoMTInitialize(&pmraaqAvatarsQueue->ffmtQueueToQuery,0);
			if (dwRetErrorCode==NO_ERROR)
			{
				CHAR szBuffer[MAX_PATH];
				NETLIBUSER nlu={0};

				mir_snprintf(szBuffer,SIZEOF(szBuffer),"%s %s %s",PROTOCOL_NAMEA,Translate("Avatars"),Translate("plugin connections"));
				nlu.cbSize=sizeof(nlu);
				nlu.flags=(NUF_OUTGOING|NUF_HTTPCONNS);
				nlu.szSettingsModule=MRA_AVT_SECT_NAME;
				nlu.szDescriptiveName=szBuffer;
				pmraaqAvatarsQueue->hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
				if (pmraaqAvatarsQueue->hNetlibUser)
				{
					LPSTR lpszPathToAvatarsCache;

					lpszPathToAvatarsCache=Utils_ReplaceVars("%miranda_avatarcache%");
					pmraaqAvatarsQueue->hAvatarsPath=FoldersRegisterCustomPath(MRA_AVT_SECT_NAME,"AvatarsPath",lpszPathToAvatarsCache);
					mir_free(lpszPathToAvatarsCache); 

					InterlockedExchange((volatile LONG*)&pmraaqAvatarsQueue->bIsRunning,TRUE);
					pmraaqAvatarsQueue->hThreadEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

					pmraaqAvatarsQueue->dwThreadsCount=DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"WorkThreadsCount",MRA_AVT_DEFAULT_WRK_THREAD_COUNTS);
					if (pmraaqAvatarsQueue->dwThreadsCount==0) pmraaqAvatarsQueue->dwThreadsCount=1;
					if (pmraaqAvatarsQueue->dwThreadsCount>MAXIMUM_WAIT_OBJECTS) pmraaqAvatarsQueue->dwThreadsCount=MAXIMUM_WAIT_OBJECTS;
					for (DWORD i=0;i<pmraaqAvatarsQueue->dwThreadsCount;i++)
					{
						pmraaqAvatarsQueue->hThread[i]=(HANDLE)mir_forkthread((pThreadFunc)MraAvatarsThreadProc,pmraaqAvatarsQueue);
					}

					(*phAvatarsQueueHandle)=(HANDLE)pmraaqAvatarsQueue;
				}
			}
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle)
{
	if (hAvatarsQueueHandle)
	{
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
		MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;
		PROTO_AVATAR_INFORMATION pai={0};

		pai.cbSize=sizeof(pai);
		//pai.hContact=pmraaqiAvatarsQueueItem->hContact;
		pai.format=PA_FORMAT_UNKNOWN;
		//pai.filename[0]=0;

		while(FifoMTItemPop(&pmraaqAvatarsQueue->ffmtQueueToQuery,NULL,(LPVOID*)&pmraaqiAvatarsQueueItem)==NO_ERROR)
		{
			pai.hContact=pmraaqiAvatarsQueueItem->hContact;
			ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_FAILED,(HANDLE)&pai,0);
			MEMFREE(pmraaqiAvatarsQueueItem);
		}
	}
}


void MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle)
{
	if (hAvatarsQueueHandle)
	{
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;

		InterlockedExchange((volatile LONG*)&pmraaqAvatarsQueue->bIsRunning,FALSE);
		SetEvent(pmraaqAvatarsQueue->hThreadEvent);

		WaitForMultipleObjects(pmraaqAvatarsQueue->dwThreadsCount,(HANDLE*)&pmraaqAvatarsQueue->hThread[0],TRUE,(WAIT_FOR_THREAD_TIMEOUT*1000));

		if (InterlockedExchangeAdd((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount,0))
		{// Иногда они не завершаются, хотя уведомление говорит об обратном %)
			while(InterlockedExchangeAdd((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount,0))
			{// ждём пока точно все завершатся
				SleepEx(100,TRUE);
			}
		}

		CloseHandle(pmraaqAvatarsQueue->hThreadEvent);

		MraAvatarsQueueClear(hAvatarsQueueHandle);

		FifoMTDestroy(&pmraaqAvatarsQueue->ffmtQueueToQuery);
		Netlib_CloseHandle(pmraaqAvatarsQueue->hNetlibUser);
		MEMFREE(pmraaqAvatarsQueue);
	}
}


DWORD MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD *pdwAvatarsQueueID)
{
	DWORD dwRetErrorCode;

	if (hAvatarsQueueHandle)
	{
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
		MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;

		pmraaqiAvatarsQueueItem=(MRA_AVATARS_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_AVATARS_QUEUE_ITEM));
		if (pmraaqiAvatarsQueueItem)
		{
			//pmraaqiAvatarsQueueItem->ffmtiFifoItem;
			pmraaqiAvatarsQueueItem->dwAvatarsQueueID=GetTickCount();
			pmraaqiAvatarsQueueItem->dwFlags=dwFlags;
			pmraaqiAvatarsQueueItem->hContact=hContact;

			FifoMTItemPush(&pmraaqAvatarsQueue->ffmtQueueToQuery,&pmraaqiAvatarsQueueItem->ffmtiFifoItem,(LPVOID)pmraaqiAvatarsQueueItem);
			if (pdwAvatarsQueueID) (*pdwAvatarsQueueID)=pmraaqiAvatarsQueueItem->dwAvatarsQueueID;
			SetEvent(pmraaqAvatarsQueue->hThreadEvent);

			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
			ShowFormatedErrorMessage(L"Avatars: can't add request to queue, error",dwRetErrorCode);
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraAvatarsThreadProc(LPVOID lpParameter)
{
	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)lpParameter;
	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;
	
	char szEMail[MAX_EMAIL_LEN],szFileName[MAX_FILEPATH],szServer[MAX_PATH];
	WCHAR szErrorText[2048];
	BOOL bContinue,bKeepAlive,bUseKeepAliveConn,bFailed,bDownloadNew,bDefaultAvt;
	BYTE btBuff[BUFF_SIZE_RCV];
	DWORD dwResultCode,dwAvatarFormat,dwReceived,dwWritten,dwServerPort,dwErrorCode;
	LPSTR lpszUser,lpszDomain;
	SIZE_T dwEMailSize,dwAvatarSizeServer,dwFileNameSize;
	FILETIME ftLastModifiedTimeServer,ftLastModifiedTimeLocal;
	SYSTEMTIME stAvatarLastModifiedTimeLocal;
	HANDLE hConnection=NULL,hFile;
	NETLIBSELECT nls={0};
	INTERNET_TIME itAvatarLastModifiedTimeServer;
	PROTO_AVATAR_INFORMATION pai;

	nls.cbSize=sizeof(nls);
	pai.cbSize=sizeof(pai);

	InterlockedIncrement((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount);

	while(InterlockedExchangeAdd((volatile LONG*)&pmraaqAvatarsQueue->bIsRunning,0))
	{
		if (FifoMTItemPop(&pmraaqAvatarsQueue->ffmtQueueToQuery,NULL,(LPVOID*)&pmraaqiAvatarsQueueItem)==NO_ERROR)
		{
			bFailed=TRUE;
			bDownloadNew=FALSE;
			bDefaultAvt=FALSE;

			if (DB_GetStaticStringA(NULL,MRA_AVT_SECT_NAME,"Server",szServer,SIZEOF(szServer),NULL)==FALSE) memmove(szServer,MRA_AVT_DEFAULT_SERVER,sizeof(MRA_AVT_DEFAULT_SERVER));
			dwServerPort=DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"ServerPort",MRA_AVT_DEFAULT_SERVER_PORT);
			bUseKeepAliveConn=DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"UseKeepAliveConn",MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN);


			if (DB_Mra_GetStaticStringA(pmraaqiAvatarsQueueItem->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				BuffToLowerCase(szEMail,szEMail,dwEMailSize);
				if (lpszDomain=(LPSTR)MemoryFindByte(0,szEMail,dwEMailSize,'@'))
				if (lpszUser=(LPSTR)MemoryFindByte((lpszDomain-szEMail),szEMail,dwEMailSize,'.'))
				{
					(*lpszUser)=0;
					lpszUser=szEMail;
					(*lpszDomain)=0;
					lpszDomain++;


					ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_CONNECTING,(HANDLE)pmraaqiAvatarsQueueItem->dwAvatarsQueueID,0);
					if (hConnection==NULL) hConnection=MraAvatarsHttpConnect(pmraaqAvatarsQueue->hNetlibUser,szServer,dwServerPort);
					if (hConnection)
					{
						ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_CONNECTED,(HANDLE)pmraaqiAvatarsQueueItem->dwAvatarsQueueID,0);
						ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_SENTREQUEST,(HANDLE)pmraaqiAvatarsQueueItem->dwAvatarsQueueID,0);
						if (MraAvatarsHttpTransaction(hConnection,REQUEST_HEAD,lpszUser,lpszDomain,szServer,MAHTRO_AVTMRIM,bUseKeepAliveConn,&dwResultCode,&bKeepAlive,&dwAvatarFormat,&dwAvatarSizeServer,&itAvatarLastModifiedTimeServer)==NO_ERROR)
						{
							switch(dwResultCode){
							case 200:
								if (MraAvatarsGetContactTime(pmraaqiAvatarsQueueItem->hContact,"AvatarLastModifiedTime",&stAvatarLastModifiedTimeLocal))
								{
									SystemTimeToFileTime(&itAvatarLastModifiedTimeServer.stTime,&ftLastModifiedTimeServer);
									SystemTimeToFileTime(&stAvatarLastModifiedTimeLocal,&ftLastModifiedTimeLocal);

									if ((*((DWORDLONG*)&ftLastModifiedTimeServer))!=(*((DWORDLONG*)&ftLastModifiedTimeLocal)))
									{// need check for update
										bDownloadNew=TRUE;
										//ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_STATUS,0,0);
									}else{// avatar is valid
										if (MraAvatarsGetFileName((HANDLE)pmraaqAvatarsQueue,pmraaqiAvatarsQueueItem->hContact,dwAvatarFormat,(LPSTR)szFileName,SIZEOF(szFileName),NULL)==NO_ERROR)
										{
											if(IsFileExistA(szFileName))
											{
												bFailed=FALSE;
											}else{
												bDownloadNew=TRUE;
											}
										}
									}
								}else{// need update
									bDownloadNew=TRUE;
								}
								break;
							case 404:// return def avatar
								if (MraAvatarsGetFileName((HANDLE)pmraaqAvatarsQueue,NULL,PA_FORMAT_DEFAULT,(LPSTR)szFileName,SIZEOF(szFileName),&dwFileNameSize)==NO_ERROR)
								{
									if(IsFileExistA(szFileName))
									{
										dwAvatarFormat=MraAvatarsGetFileFormat(szFileName,dwFileNameSize);
                                        bFailed=FALSE;
									}else{//loading default avatar
										bDownloadNew=TRUE;
									}
									bDefaultAvt=TRUE;
								}
								break;
							default:
								mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Avatars: server return HTTP code: %lu"),dwResultCode);
								ShowFormatedErrorMessage(szErrorText,NO_ERROR);
								break;
							}
						}
						if (bUseKeepAliveConn==FALSE || bKeepAlive==FALSE) NETLIB_CLOSEHANDLE(hConnection);
					}


					if (bDownloadNew)
					{
						if (hConnection==NULL) hConnection=MraAvatarsHttpConnect(pmraaqAvatarsQueue->hNetlibUser,szServer,dwServerPort);
						if (hConnection)
						{
							ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_DATA,(HANDLE)pmraaqiAvatarsQueueItem->dwAvatarsQueueID,0);
							if (MraAvatarsHttpTransaction(hConnection,REQUEST_GET,lpszUser,lpszDomain,szServer,MAHTRO_AVT,bUseKeepAliveConn,&dwResultCode,&bKeepAlive,&dwAvatarFormat,&dwAvatarSizeServer,&itAvatarLastModifiedTimeServer)==NO_ERROR && dwResultCode==200)
							{
								if (bDefaultAvt) dwAvatarFormat=PA_FORMAT_DEFAULT;
								if (MraAvatarsGetFileName((HANDLE)pmraaqAvatarsQueue,pmraaqiAvatarsQueueItem->hContact,dwAvatarFormat,(LPSTR)szFileName,SIZEOF(szFileName),NULL)==NO_ERROR)
								{
									hFile=CreateFileA(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
									if (hFile!=INVALID_HANDLE_VALUE)
									{
										dwWritten=0;
										bContinue=TRUE;
										nls.dwTimeout=(1000*DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"TimeOutReceive",MRA_AVT_DEFAULT_TIMEOUT_RECV));
										nls.hReadConns[0]=hConnection;

										while(bContinue)
										{
											switch(CallService(MS_NETLIB_SELECT,0,(LPARAM)&nls)){
											case SOCKET_ERROR:
											case 0:// Time out
												dwErrorCode=GetLastError();
												ShowFormatedErrorMessage(L"Avatars: error on receive file data",dwErrorCode);
												bContinue=FALSE;
												break;
											case 1:
												dwReceived=Netlib_Recv(hConnection,(LPSTR)&btBuff,SIZEOF(btBuff),0);
												if (dwReceived==0 || dwReceived==SOCKET_ERROR)
												{
													dwErrorCode=GetLastError();
													ShowFormatedErrorMessage(L"Avatars: error on receive file data",dwErrorCode);
													bContinue=FALSE;
												}else{
													if (WriteFile(hFile,(LPVOID)&btBuff,dwReceived,&dwReceived,NULL))
													{
														dwWritten+=dwReceived;
														if (dwWritten>=dwAvatarSizeServer) bContinue=FALSE;
													}else{
														dwErrorCode=GetLastError();
														ShowFormatedErrorMessage(L"Avatars: cant write file data, error",dwErrorCode);
														bContinue=FALSE;
													}
												}
												break;
											}

										}
										CloseHandle(hFile);
										bFailed=FALSE;
									}else{
										dwErrorCode=GetLastError();
										mir_sntprintf(szErrorText,SIZEOF(szErrorText),TranslateW(L"Avatars: cant open file %S, error"),szFileName);
										ShowFormatedErrorMessage(szErrorText,dwErrorCode);
									}
								}
							}else{
								DebugBreak();
							}
							if (bUseKeepAliveConn==FALSE || bKeepAlive==FALSE) NETLIB_CLOSEHANDLE(hConnection);
						}
					}
				}
			}

			if (bFailed)
			{
				DeleteFileA(szFileName);
				pai.hContact=pmraaqiAvatarsQueueItem->hContact;
				pai.format=PA_FORMAT_UNKNOWN;
				pai.filename[0]=0;
				//MraAvatarsSetContactTime(pmraaqiAvatarsQueueItem->hContact,"AvatarLastModifiedTime",NULL);
				// write owner avatar file name to DB
				//if (pmraaqiAvatarsQueueItem->hContact==NULL) DB_Mra_DeleteValue(NULL,"AvatarFile");
				
				ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_FAILED,(HANDLE)&pai,0);
			}else{
				pai.hContact=pmraaqiAvatarsQueueItem->hContact;
				pai.format=dwAvatarFormat;
				if (DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"ReturnAbsolutePath",MRA_AVT_DEFAULT_RET_ABC_PATH))
				{
                    lstrcpynA(pai.filename,szFileName,SIZEOF(pai.filename));
				}else{
					CallService(MS_UTILS_PATHTORELATIVE,(WPARAM)szFileName,(LPARAM)pai.filename);
				}
				if (bDefaultAvt) dwAvatarFormat=PA_FORMAT_DEFAULT;
				SetContactAvatarFormat(pmraaqiAvatarsQueueItem->hContact,dwAvatarFormat);
				MraAvatarsSetContactTime(pmraaqiAvatarsQueueItem->hContact,"AvatarLastModifiedTime",&itAvatarLastModifiedTimeServer.stTime);
				// write owner avatar file name to DB
				if (pmraaqiAvatarsQueueItem->hContact==NULL)
				{// proto avatar
					//DB_Mra_SetStringA(NULL,"AvatarFile",pai.filename);
					CallService(MS_AV_REPORTMYAVATARCHANGED,(WPARAM)PROTOCOL_NAMEA,0);
				}
				
				ProtoBroadcastAck(PROTOCOL_NAMEA,pmraaqiAvatarsQueueItem->hContact,ACKTYPE_AVATAR,ACKRESULT_SUCCESS,(HANDLE)&pai,0);
			}
			MEMFREE(pmraaqiAvatarsQueueItem);
		}else{// waiting until service stop or new task
			NETLIB_CLOSEHANDLE(hConnection);
			WaitForSingleObjectEx(pmraaqAvatarsQueue->hThreadEvent,MRA_AVT_DEFAULT_QE_CHK_INTERVAL,FALSE);
		}
	}

	InterlockedDecrement((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount);
}


HANDLE MraAvatarsHttpConnect(HANDLE hNetlibUser,LPSTR lpszHost,DWORD dwPort)
{
	HANDLE hConnection;
	DWORD dwConnectReTryCount,dwCurConnectReTryCount;
	NETLIBOPENCONNECTION nloc={0};

	dwConnectReTryCount=DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"ConnectReTryCount",MRA_AVT_DEFAULT_CONN_RETRY_COUNT);

	nloc.cbSize=sizeof(nloc);
	nloc.flags=(NLOCF_HTTP|NLOCF_V2);
	nloc.szHost=lpszHost;
	if (IsHTTPSProxyUsed(hNetlibUser))
	{// через https прокси только 443 порт
		nloc.wPort=MRA_SERVER_PORT_HTTPS;
	}else{
		nloc.wPort=(WORD)dwPort;
	}
	nloc.timeout=DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"TimeOutConnect",MRA_AVT_DEFAULT_TIMEOUT_CONN);
	if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout=MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout=MRA_TIMEOUT_CONN_МАХ;


	dwCurConnectReTryCount=dwConnectReTryCount;
	do{
		hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)hNetlibUser,(LPARAM)&nloc);
	}while(--dwCurConnectReTryCount && hConnection==NULL);

	if (hConnection==NULL)
	{
		ShowFormatedErrorMessage(L"Avatars: can't connect to server, error",GetLastError());
	}
return(hConnection);
}


DWORD MraAvatarsHttpTransaction(HANDLE hConnection,DWORD dwRequestType,LPSTR lpszUser,LPSTR lpszDomain,LPSTR lpszHost,DWORD dwReqObj,BOOL bUseKeepAliveConn,DWORD *pdwResultCode,BOOL *pbKeepAlive,DWORD *pdwFormat,SIZE_T *pdwAvatarSize,INTERNET_TIME *pitLastModifiedTime)
{
	DWORD dwRetErrorCode;

	if (pdwResultCode)		(*pdwResultCode)=0;
	if (pbKeepAlive)		(*pbKeepAlive)=FALSE;
	if (pdwFormat)			(*pdwFormat)=PA_FORMAT_UNKNOWN;
	if (pdwAvatarSize)		(*pdwAvatarSize)=0;
	if (pitLastModifiedTime) memset(pitLastModifiedTime,0, sizeof(INTERNET_TIME));

	if (hConnection)
	{
		char szBuff[4096],szSelfVersionString[MAX_PATH];
		DWORD dwSended,dwBuffSize;
		LPSTR lpszReqObj;
		NETLIBHTTPHEADER nlbhHeaders[8]={0};
		NETLIBHTTPREQUEST *pnlhr,nlhr={0};

		switch(dwReqObj){
		case MAHTRO_AVT:			lpszReqObj="_avatar";			break;
		case MAHTRO_AVTMRIM:		lpszReqObj="_mrimavatar";		break;
		case MAHTRO_AVTSMALL:		lpszReqObj="_avatarsmall";		break;
		case MAHTRO_AVTSMALLMRIM:	lpszReqObj="_mrimavatarsmall";	break;
		default:					lpszReqObj="";					break;
		}
		dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"http://%s/%s/%s/%s",lpszHost,lpszDomain,lpszUser,lpszReqObj);

		MraGetSelfVersionString(szSelfVersionString,SIZEOF(szSelfVersionString),NULL);

		nlbhHeaders[0].szName="User-Agent";		nlbhHeaders[0].szValue=szSelfVersionString;
		nlbhHeaders[1].szName="Accept-Encoding";nlbhHeaders[1].szValue="deflate";
		nlbhHeaders[2].szName="Pragma";			nlbhHeaders[2].szValue="no-cache";
		nlbhHeaders[3].szName="Connection";		nlbhHeaders[3].szValue=(bUseKeepAliveConn)? "keep-alive":"close";
		//nlbhHeaders[4].szName="If-Modified-Since";nlbhHeaders[4].szValue="Thu, 03 Aug 2006 19:54:33 GMT";

		nlhr.cbSize=sizeof(nlhr);
		nlhr.requestType=dwRequestType;
		nlhr.flags=(NLHRF_GENERATEHOST|NLHRF_SMARTREMOVEHOST|NLHRF_SMARTAUTHHEADER);
		nlhr.szUrl=szBuff;
		nlhr.headers=(NETLIBHTTPHEADER*)&nlbhHeaders;
		nlhr.headersCount=4;
		//nlhr.pData=NULL;
		//nlhr.dataLength=0;
		//nlhr.resultCode=0;
		//nlhr.szResultDescr=NULL;
		//nlhr.nlc=NULL;
		
		dwSended=CallService(MS_NETLIB_SENDHTTPREQUEST,(WPARAM)hConnection,(LPARAM)&nlhr);
		if (dwSended!=SOCKET_ERROR && dwSended)
		{
			pnlhr=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_RECVHTTPHEADERS,(WPARAM)hConnection,(LPARAM)0);
			if (pnlhr)
			{
				for(SIZE_T i=0;i<(SIZE_T)pnlhr->headersCount;i++)
				{
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pnlhr->headers[i].szName,-1,"Connection",10)==CSTR_EQUAL)
					{
						if (pbKeepAlive) (*pbKeepAlive)=(CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pnlhr->headers[i].szValue,-1,"keep-alive",10)==CSTR_EQUAL);
					}else
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pnlhr->headers[i].szName,-1,"Content-Type",12)==CSTR_EQUAL)
					{
						if (pdwFormat)
						{
							for (SIZE_T j=0;j<PA_FORMAT_MAX;j++)
							{
								if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pnlhr->headers[i].szValue,-1,lpcszContentType[j],-1)==CSTR_EQUAL)
								{
									(*pdwFormat)=j;
									break;
								}
							}
						}
					}else
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pnlhr->headers[i].szName,-1,"Content-Length",14)==CSTR_EQUAL)
					{
						if (pdwAvatarSize) (*pdwAvatarSize)=StrToUNum(pnlhr->headers[i].szValue,lstrlenA(pnlhr->headers[i].szValue));
					}else
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pnlhr->headers[i].szName,-1,"Last-Modified",13)==CSTR_EQUAL)
					{
						if (pitLastModifiedTime) InternetTimeGetTime(pnlhr->headers[i].szValue,lstrlenA(pnlhr->headers[i].szValue),pitLastModifiedTime);
					}
				}// end for

				if (pdwResultCode) (*pdwResultCode)=pnlhr->resultCode;
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,(WPARAM)0,(LPARAM)pnlhr);
				dwRetErrorCode=NO_ERROR;
			}else{// err on receive
				dwRetErrorCode=GetLastError();
				ShowFormatedErrorMessage(L"Avatars: error on receive HTTP answer",dwRetErrorCode);
			}
		}else{// err on send http request
			dwRetErrorCode=GetLastError();
			ShowFormatedErrorMessage(L"Avatars: error on send HTTP request",dwRetErrorCode);
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}

return(dwRetErrorCode);
}


BOOL MraAvatarsGetContactTime(HANDLE hContact,LPSTR lpszValueName,SYSTEMTIME *pstTime)
{
	BOOL bRet=FALSE;

	if (lpszValueName && pstTime)
	{
		char szBuff[MAX_PATH];
		SIZE_T dwBuffSize;
		INTERNET_TIME itAvatarLastModifiedTimeLocal;

		if (DB_Mra_GetStaticStringA(hContact,lpszValueName,(LPSTR)szBuff,SIZEOF(szBuff),&dwBuffSize))
		if (InternetTimeGetTime(szBuff,dwBuffSize,&itAvatarLastModifiedTimeLocal)==NO_ERROR)
		{
			memmove(pstTime,&itAvatarLastModifiedTimeLocal.stTime,sizeof(SYSTEMTIME));
			bRet=TRUE;
		}
	}
return(bRet);
}


void MraAvatarsSetContactTime(HANDLE hContact,LPSTR lpszValueName,SYSTEMTIME *pstTime)
{
	if (lpszValueName)
	{
		char szBuff[MAX_PATH];
		SIZE_T dwBuffUsedSize;
		INTERNET_TIME itTime;

		if (pstTime)
		{
			itTime.lTimeZone=0;
			memmove(&itTime.stTime,pstTime,sizeof(SYSTEMTIME));
		}else{
			InternetTimeGetCurrentTime(&itTime);
		}

		if (itTime.stTime.wYear)
		{
			InternetTimeGetString(&itTime,(LPSTR)szBuff,SIZEOF(szBuff),&dwBuffUsedSize);
			DB_Mra_SetStringExA(hContact,lpszValueName,(LPSTR)szBuff,dwBuffUsedSize);
		}else{
			DB_Mra_DeleteValue(hContact,lpszValueName);
		}
	}
}


DWORD MraAvatarsGetFileFormat(LPSTR lpszPath,SIZE_T dwPathSize)
{
	DWORD dwRet=-1,dwExt;

	BuffToLowerCase(&dwExt,(lpszPath+(dwPathSize-sizeof(DWORD))),sizeof(DWORD));
	for(DWORD i=0;i<PA_FORMAT_MAX;i++)
	{
		if (dwExt==(*((DWORD*)lpcszExtensions[i])))
		{
			dwRet=i;
			break;
		}
	}
return(dwRet);
}


DWORD MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle,HANDLE hContact,DWORD dwFormat,LPSTR lpszPath,SIZE_T dwPathSize,SIZE_T *pdwPathSizeRet)
{
	DWORD dwRetErrorCode;

	if (hAvatarsQueueHandle)
	{
		if (IsContactChatAgent(hContact)==FALSE)
		{
			LPSTR lpszCurPath=lpszPath;
			SIZE_T dwEMailSize;
			MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;

			dwRetErrorCode=ERROR_INSUFFICIENT_BUFFER;
			if (pmraaqAvatarsQueue->hAvatarsPath==NULL || FoldersGetCustomPath(pmraaqAvatarsQueue->hAvatarsPath,lpszCurPath,dwPathSize,""))
			{// default path
				LPSTR lpszPathToAvatarsCache;
				SIZE_T dwPathToAvatarsCacheSize;

				lpszPathToAvatarsCache=Utils_ReplaceVars("%miranda_avatarcache%");
				dwPathToAvatarsCacheSize=lstrlenA(lpszPathToAvatarsCache);
				if (dwPathSize>(dwPathToAvatarsCacheSize+8))
				{
					memmove(lpszCurPath,lpszPathToAvatarsCache,dwPathToAvatarsCacheSize);
					dwPathSize-=(dwPathToAvatarsCacheSize+1);
					lpszCurPath+=dwPathToAvatarsCacheSize;
					(*((BYTE*)lpszCurPath++))='\\';
					(*((BYTE*)lpszCurPath))=0;// теперь точно строка закончится нулём
				}else{
					dwPathSize=0;
					if (pdwPathSizeRet) (*pdwPathSizeRet)=(dwPathToAvatarsCacheSize+MAX_PATH+32);
				}
				mir_free(lpszPathToAvatarsCache); 
			}else{
				dwEMailSize=lstrlenA(lpszCurPath);
				dwPathSize-=dwEMailSize;
				lpszCurPath+=dwEMailSize;
			}

			if (dwPathSize)
			{// some path in buff and free space for file name is avaible
				CreateDirectoryA(lpszPath,NULL);

				if (dwFormat!=PA_FORMAT_DEFAULT)
				{
					if (DB_Mra_GetStaticStringA(hContact,"e-mail",lpszCurPath,(dwPathSize-5),&dwEMailSize))
					{
						BuffToLowerCase(lpszCurPath,lpszCurPath,dwEMailSize);
						lpszCurPath+=dwEMailSize;
						(*((DWORD*)lpszCurPath))=(*((DWORD*)lpcszExtensions[dwFormat]));
						lpszCurPath+=sizeof(DWORD);
						(*((BYTE*)lpszCurPath))=0;

						if (pdwPathSizeRet) (*pdwPathSizeRet)=(lpszCurPath-lpszPath);
						dwRetErrorCode=NO_ERROR;
					}
				}else{
					if (DB_GetStaticStringA(NULL,MRA_AVT_SECT_NAME,"DefaultAvatarFileName",lpszCurPath,(dwPathSize-5),&dwEMailSize)==FALSE)
					{
						memmove(lpszCurPath,MRA_AVT_DEFAULT_AVT_FILENAME,sizeof(MRA_AVT_DEFAULT_AVT_FILENAME));
						lpszCurPath+=(sizeof(MRA_AVT_DEFAULT_AVT_FILENAME)-1);
						(*((BYTE*)lpszCurPath))=0;

						if (pdwPathSizeRet) (*pdwPathSizeRet)=(lpszCurPath-lpszPath);
						dwRetErrorCode=NO_ERROR;
					}
				}
			}
		}else{//146119@chat.agent - conferences has no avatars
			dwRetErrorCode=ERROR_NOT_SUPPORTED;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



DWORD MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD *pdwAvatarsQueueID,DWORD *pdwFormat,LPSTR lpszPath)
{
	DWORD dwRetCode=GAIR_NOAVATAR;

	if (hAvatarsQueueHandle)
	if (DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"Enable",MRA_AVT_DEFAULT_ENABLE))
	if (IsContactChatAgent(hContact)==FALSE)
	{// not @chat.agent conference
		BOOL bQueueAdd=TRUE;// check for updates
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
		SYSTEMTIME stAvatarLastCheckTime;

		if ((dwFlags&GAIF_FORCE)==0)// если флаг принудит. обновления, то даже не проверяем времени последнего обновления
		if (MraAvatarsGetContactTime(hContact,"AvatarLastCheckTime",&stAvatarLastCheckTime))
		{
			char szFileName[MAX_FILEPATH];
			SIZE_T dwPathSize;
			FILETIME ftCurrentTime,ftExpireTime;

			GetSystemTimeAsFileTime(&ftCurrentTime);
			SystemTimeToFileTime(&stAvatarLastCheckTime,&ftExpireTime);
			(*((DWORDLONG*)&ftExpireTime))+=(FILETIME_MINUTE*(DWORDLONG)DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"CheckInterval",MRA_AVT_DEFAULT_CHK_INTERVAL));
			
			if ((*((DWORDLONG*)&ftExpireTime))>(*((DWORDLONG*)&ftCurrentTime)))
			if (MraAvatarsGetFileName(hAvatarsQueueHandle,hContact,GetContactAvatarFormat(hContact,PA_FORMAT_DEFAULT),(LPSTR)szFileName,SIZEOF(szFileName),&dwPathSize)==NO_ERROR)
			if (IsFileExistA(szFileName))
			{// файл с аватаром существует и не устарел/не было комманды обновлять(просто запрос имени)
				if (lpszPath)
				if (DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"ReturnAbsolutePath",MRA_AVT_DEFAULT_RET_ABC_PATH))
				{
					lstrcpynA(lpszPath,szFileName,MAX_PATH);
				}else{
					CallService(MS_UTILS_PATHTORELATIVE,(WPARAM)szFileName,(LPARAM)lpszPath);
				}
				if (pdwFormat) (*pdwFormat)=GetContactAvatarFormat(hContact,PA_FORMAT_DEFAULT);
				dwRetCode=GAIR_SUCCESS;
				bQueueAdd=FALSE;
			}
		}

		if (bQueueAdd || (dwFlags&GAIF_FORCE))
		if (MraAvatarsQueueAdd(hAvatarsQueueHandle,dwFlags,hContact,pdwAvatarsQueueID)==NO_ERROR)
		{
			MraAvatarsSetContactTime(hContact,"AvatarLastCheckTime",NULL);
			dwRetCode=GAIR_WAITFOR;
		}
	}
return(dwRetCode);
}


DWORD MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwSourceID)
{//***deb dwSourceID - for filtering cals from different places
	DWORD dwRetCode=GAIR_NOAVATAR;
	PROTO_AVATAR_INFORMATION pai={0};
	
	pai.cbSize=sizeof(pai);
	pai.hContact=hContact;
	if ((dwRetCode=MraAvatarsQueueGetAvatar(hAvatarsQueueHandle,dwFlags,hContact,NULL,(DWORD*)&pai.format,pai.filename))==GAIR_SUCCESS)
	{
		// write owner avatar file name to DB
		if (hContact==NULL)
		{
			//DB_Mra_SetStringA(NULL,"AvatarFile",pai.filename);
			CallService(MS_AV_REPORTMYAVATARCHANGED,(WPARAM)PROTOCOL_NAMEA,0);
		}
		ProtoBroadcastAck(PROTOCOL_NAMEA,hContact,ACKTYPE_AVATAR,ACKRESULT_SUCCESS,(HANDLE)&pai,0);
	}
return(dwRetCode);
}


WORD wMraAvatarsControlsList[]={
						IDC_SERVER,
						IDC_SERVERPORT,
						IDC_BUTTON_DEFAULT,
						IDC_USE_KEEPALIVE_CONN,
						IDC_UPD_CHECK_INTERVAL,
						IDC_RETURN_ABC_PATH,
						IDC_DELETE_AVT_ON_CONTACT_DELETE
};

INT_PTR CALLBACK MraAvatarsQueueDlgProcOpts(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			WCHAR szServer[MAX_PATH];

			TranslateDialogDefault(hWndDlg);

			CHECK_DLG_BUTTON(hWndDlg,IDC_ENABLE,DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"Enable",MRA_AVT_DEFAULT_ENABLE));

			if (DB_GetStaticStringW(NULL,MRA_AVT_SECT_NAME,"Server",szServer,MAX_PATH,NULL))
			{
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_SERVER,szServer);
			}else{
				SET_DLG_ITEM_TEXTA(hWndDlg,IDC_SERVER,MRA_AVT_DEFAULT_SERVER);
			}

			SetDlgItemInt(hWndDlg,IDC_SERVERPORT,DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"ServerPort",MRA_AVT_DEFAULT_SERVER_PORT),FALSE);
			CHECK_DLG_BUTTON(hWndDlg,IDC_USE_KEEPALIVE_CONN,DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"UseKeepAliveConn",MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN));
			SetDlgItemInt(hWndDlg,IDC_UPD_CHECK_INTERVAL,DBGetContactSettingDword(NULL,MRA_AVT_SECT_NAME,"CheckInterval",MRA_AVT_DEFAULT_CHK_INTERVAL),FALSE);
			CHECK_DLG_BUTTON(hWndDlg,IDC_RETURN_ABC_PATH,DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"ReturnAbsolutePath",MRA_AVT_DEFAULT_RET_ABC_PATH));
			CHECK_DLG_BUTTON(hWndDlg,IDC_DELETE_AVT_ON_CONTACT_DELETE,DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"DeleteAvtOnContactDelete",MRA_DELETE_AVT_ON_CONTACT_DELETE));
			
			EnableControlsArray(hWndDlg,(WORD*)&wMraAvatarsControlsList,SIZEOF(wMraAvatarsControlsList),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ENABLE));
		}
		return(TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam)==IDC_ENABLE)
		{
			EnableControlsArray(hWndDlg,(WORD*)&wMraAvatarsControlsList,SIZEOF(wMraAvatarsControlsList),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ENABLE));
		}

		if (LOWORD(wParam)==IDC_BUTTON_DEFAULT)
		{
			SET_DLG_ITEM_TEXTA(hWndDlg,IDC_SERVER,MRA_AVT_DEFAULT_SERVER);
			SetDlgItemInt(hWndDlg,IDC_SERVERPORT,MRA_AVT_DEFAULT_SERVER_PORT,FALSE);
		}

		if ((LOWORD(wParam)==IDC_SERVER || LOWORD(wParam)==IDC_SERVERPORT || LOWORD(wParam)==IDC_UPD_CHECK_INTERVAL) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) ) return(FALSE);
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				WCHAR szServer[MAX_PATH];

				DBWriteContactSettingByte(NULL,MRA_AVT_SECT_NAME,"Enable",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ENABLE));
				DBWriteContactSettingByte(NULL,MRA_AVT_SECT_NAME,"DeleteAvtOnContactDelete",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_DELETE_AVT_ON_CONTACT_DELETE));
				DBWriteContactSettingByte(NULL,MRA_AVT_SECT_NAME,"ReturnAbsolutePath",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_RETURN_ABC_PATH));
				DBWriteContactSettingDword(NULL,MRA_AVT_SECT_NAME,"CheckInterval",GetDlgItemInt(hWndDlg,IDC_UPD_CHECK_INTERVAL,NULL,FALSE));
				DBWriteContactSettingByte(NULL,MRA_AVT_SECT_NAME,"UseKeepAliveConn",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_USE_KEEPALIVE_CONN));
				DBWriteContactSettingDword(NULL,MRA_AVT_SECT_NAME,"ServerPort",GetDlgItemInt(hWndDlg,IDC_SERVERPORT,NULL,FALSE));
				GET_DLG_ITEM_TEXT(hWndDlg,IDC_SERVER,szServer,SIZEOF(szServer));DB_SetStringW(NULL,MRA_AVT_SECT_NAME,"Server",szServer);
			}
			return(TRUE);
		}
		break;
	}
return(FALSE);
}


DWORD MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle,HANDLE hContact)
{
	DWORD dwRetErrorCode,dwAvatarFormat;
	char szFileName[MAX_FILEPATH];

	if (hAvatarsQueueHandle)
	{
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue=(MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;

		dwAvatarFormat=GetContactAvatarFormat(hContact,PA_FORMAT_UNKNOWN);
		if (DBGetContactSettingByte(NULL,MRA_AVT_SECT_NAME,"DeleteAvtOnContactDelete",MRA_DELETE_AVT_ON_CONTACT_DELETE) && dwAvatarFormat!=PA_FORMAT_DEFAULT)
		{
			if ((dwRetErrorCode=MraAvatarsGetFileName(hAvatarsQueueHandle,hContact,dwAvatarFormat,(LPSTR)szFileName,SIZEOF(szFileName),NULL))==NO_ERROR)
			if (DeleteFileA(szFileName))
			{
				dwRetErrorCode=NO_ERROR;
			}else{
				dwRetErrorCode=GetLastError();
			}
		}else{
			dwRetErrorCode=NO_ERROR;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


