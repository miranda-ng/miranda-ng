#include "Mra.h"
#include "MraOfflineMsg.h"
#include "MraRTFMsg.h"
#include "MraPlaces.h"






void			MraThreadProc					(LPVOID lpParameter);
DWORD			MraGetNLBData					(LPSTR lpszHost,SIZE_T dwHostBuffSize,WORD *pwPort);
DWORD			MraNetworkDispather				();
DWORD			MraCommandDispather				(mrim_packet_header_t *pmaHeader,DWORD *pdwPingPeriod,DWORD *pdwNextPingSendTickTime,BOOL *pbContinue);




//DWORD			GetMraStatusFromMiradaStatus	(DWORD dwMirandaStatus,DWORD dwXStatusMir,DWORD *pdwXStatusMra);
//DWORD			GetMiradaStatusFromMraStatus	(DWORD dwMraStatus,DWORD dwXStatusMra,DWORD *pdwXStatusMir);
//DWORD			GetMraXStatusIDFromMraUriStatus	(LPSTR lpszStatusUri,SIZE_T dwStatusUriSize);

DWORD			GetUL				(LPBYTE *plpData);
DWORDLONG		GetUIDL				(LPBYTE *plpData);
MRA_GUID		GetGUID				(LPBYTE *plpData);
DWORD			GetLPS				(LPBYTE lpbData,DWORD dwDataSize,LPBYTE *plpCurrentData,MRA_LPS *plpsString);






DWORD StartConnect()
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwGlobalPluginRunning,0))
	{
		if (InterlockedCompareExchange((volatile LONG*)&masMraSettings.dwThreadWorkerRunning,TRUE,FALSE)==FALSE)
		{// поток ещё/уже не работал, поставили статус что работает и запускаем
			char szPass[MAX_PATH];
			SIZE_T dwEMailSize=0,dwPasswordSize=0;

			DB_Mra_GetStaticStringA(NULL,"e-mail",NULL,0,&dwEMailSize);

			if (dwEMailSize>5 && GetPassDB(szPass,sizeof(szPass),&dwPasswordSize))
			{// mir_forkthread
				InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
				masMraSettings.hThreadWorker=mir_forkthread(MraThreadProc,NULL);
				if (masMraSettings.hThreadWorker==NULL)
				{
					dwRetErrorCode=GetLastError();
					InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerRunning,FALSE);
					MraSetStatus(ID_STATUS_OFFLINE,0);
				}
			}else{
				MraThreadClean();
				if (!(dwEMailSize>5))
				{
					MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING,0,TranslateW(L"Please, setup e-mail in options"));
				}else{
					MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING,0,TranslateW(L"Please, setup password in options"));
				}
			}
			SecureZeroMemory(szPass,sizeof(szPass));
		}else{
			DebugBreak();
		}
	}else{
		dwRetErrorCode=ERROR_OPERATION_ABORTED;
		//DebugBreak();
	}
return(dwRetErrorCode);
}


void MraThreadProc(LPVOID lpParameter)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (TRUE)
	{
		BOOL bConnected=FALSE;
		char szHost[MAX_PATH];
		DWORD dwConnectReTryCount,dwCurConnectReTryCount;
		NETLIBOPENCONNECTION nloc={0};

		SleepEx(100,FALSE);// to prevent high CPU load by some status plugins like allwaysonline

		dwConnectReTryCount=DB_Mra_GetDword(NULL,"ConnectReTryCountMRIM",MRA_DEFAULT_CONN_RETRY_COUNT_MRIM);
	
		nloc.cbSize=sizeof(nloc);
		nloc.flags=NLOCF_V2;
		nloc.szHost=szHost;
		nloc.timeout=DB_Mra_GetDword(NULL,"TimeOutConnectMRIM",MRA_DEFAULT_TIMEOUT_CONN_MRIM);
		if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout=MRA_TIMEOUT_CONN_MIN;
		if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout=MRA_TIMEOUT_CONN_МАХ;

		InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
		if (MraGetNLBData((LPSTR)nloc.szHost,MAX_PATH,&nloc.wPort)==NO_ERROR)
		{
			dwCurConnectReTryCount=dwConnectReTryCount;
			do{
				InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
				masMraSettings.hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nloc);
			}while(--dwCurConnectReTryCount && masMraSettings.hConnection==NULL);

			if (masMraSettings.hConnection)
			{
				bConnected=TRUE;
			}
		}

		if (bConnected==FALSE)
		if (DB_Mra_GetByte(NULL,"NLBFailDirectConnect",MRA_DEFAULT_NLB_FAIL_DIRECT_CONNECT))
		{
			if (IsHTTPSProxyUsed(masMraSettings.hNetlibUser))
			{// через https прокси только 443 порт
				nloc.wPort=MRA_SERVER_PORT_HTTPS;
			}else{
				nloc.wPort=DB_Mra_GetWord(NULL,"ServerPort",MRA_DEFAULT_SERVER_PORT);
				if (nloc.wPort==MRA_SERVER_PORT_STANDART_NLB) nloc.wPort=MRA_SERVER_PORT_STANDART;
			}

			for(DWORD i=1;(i<MRA_MAX_MRIM_SERVER && MraGetStatus(0,0)!=ID_STATUS_OFFLINE);i++)
			{
				mir_snprintf(szHost,SIZEOF(szHost),"mrim%lu.mail.ru",i);
				
				dwCurConnectReTryCount=dwConnectReTryCount;
				do{
					InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
					masMraSettings.hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nloc);
				}while(--dwCurConnectReTryCount && masMraSettings.hConnection==NULL);

				if (masMraSettings.hConnection)
				{
					bConnected=TRUE;
					break;
				}
			}
		}


		if (bConnected && MraGetStatus(0,0)!=ID_STATUS_OFFLINE)
		{
			/*SOCKET sktSocket;
			struct sockaddr_in saName;
			int iNamelen=sizeof(struct sockaddr_in);

			sktSocket=(SOCKET)CallService(MS_NETLIB_GETSOCKET,(WPARAM)masMraSettings.hConnection,(LPARAM)NULL);
			getsockname(sktSocket,(struct sockaddr*)&saName,&iNamelen);
			memmove(&masMraSettings.dwIP,&saName.sin_addr,4);
			masMraSettings.dwPort=ntohs(saName.sin_port);*/

			dwRetErrorCode=MraNetworkDispather();
		}else{
			if (bConnected==FALSE)
			{
				ShowFormatedErrorMessage(L"Can't connect to MRIM server, error",GetLastError());
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,ACKTYPE_LOGIN,ACKRESULT_FAILED,NULL,LOGINERR_NONETWORK,0);
			}
		}
	}
	MraThreadClean();
}


void MraThreadClean()
{
	MraMPopSessionQueueFlush(masMraSettings.hMPopSessionQueue);
	Netlib_CloseHandle(masMraSettings.hConnection);// called twice, if user set offline, its normal
	masMraSettings.hConnection=NULL;
	masMraSettings.dwCMDNum=0;

	SleepEx(100,FALSE);// to prevent high CPU load by some status plugins like allwaysonline

	masMraSettings.hThreadWorker=NULL;
	InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerRunning,FALSE);
	MraSetStatus(ID_STATUS_OFFLINE,0);
}



DWORD MraGetNLBData(LPSTR lpszHost,SIZE_T dwHostBuffSize,WORD *pwPort)
{
	DWORD dwRetErrorCode;

	BOOL bContinue=TRUE;
	BYTE btBuff[MAX_PATH];
	DWORD dwConnectReTryCount,dwCurConnectReTryCount;
	LPSTR lpszPort;
	SIZE_T dwBytesReceived,dwRcvBuffSizeUsed=0;
	NETLIBSELECT nls={0};
	NETLIBOPENCONNECTION nloc={0};


	dwConnectReTryCount=DB_Mra_GetDword(NULL,"ConnectReTryCountNLB",MRA_DEFAULT_CONN_RETRY_COUNT_NLB);
	
	nloc.cbSize=sizeof(nloc);
	nloc.flags=NLOCF_V2;
	if (DB_Mra_GetStaticStringA(NULL,"Server",(LPSTR)btBuff,SIZEOF(btBuff),NULL))
	{
		nloc.szHost=(LPSTR)btBuff;
	}else{
		nloc.szHost=MRA_DEFAULT_SERVER;
	}

	if (IsHTTPSProxyUsed(masMraSettings.hNetlibUser))
	{// через https прокси только 443 порт
		nloc.wPort=MRA_SERVER_PORT_HTTPS;
	}else{
		nloc.wPort=DB_Mra_GetWord(NULL,"ServerPort",MRA_DEFAULT_SERVER_PORT);
	}
	nloc.timeout=DB_Mra_GetDword(NULL,"TimeOutConnectNLB",MRA_DEFAULT_TIMEOUT_CONN_NLB);
	if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout=MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout=MRA_TIMEOUT_CONN_МАХ;


	dwCurConnectReTryCount=dwConnectReTryCount;
	do{
		InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
		nls.hReadConns[0]=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)masMraSettings.hNetlibUser,(LPARAM)&nloc);
	}while(--dwCurConnectReTryCount && nls.hReadConns[0]==NULL);

	if (nls.hReadConns[0])
	{
		nls.cbSize=sizeof(nls);
		nls.dwTimeout=(1000*DB_Mra_GetDword(NULL,"TimeOutReceiveNLB",MRA_DEFAULT_TIMEOUT_RECV_NLB));
		InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());

		while(MraGetStatus(0,0)!=ID_STATUS_OFFLINE && bContinue)
		{
			switch(CallService(MS_NETLIB_SELECT,0,(LPARAM)&nls)){
			case SOCKET_ERROR:
			case 0:// Time out
				bContinue=FALSE;
				break;
			case 1:
				dwBytesReceived=Netlib_Recv(nls.hReadConns[0],(LPSTR)(btBuff+dwRcvBuffSizeUsed),(SIZEOF(btBuff)-dwRcvBuffSizeUsed),0);
				if (dwBytesReceived && dwBytesReceived!=SOCKET_ERROR)
				{
					dwRcvBuffSizeUsed+=dwBytesReceived;
				}else{
					bContinue=FALSE;
				}
				break;
			}
			InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
		}
		Netlib_CloseHandle(nls.hReadConns[0]);

		if (dwRcvBuffSizeUsed)
		{// received, work with data
			lpszPort=(LPSTR)MemoryFindByte(0,btBuff,dwRcvBuffSizeUsed,':');
			if (lpszPort)
			{
				(*lpszPort)=0;
				lpszPort++;

				lstrcpynA(lpszHost,(LPSTR)btBuff,dwHostBuffSize);
				if (pwPort) (*pwPort)=(WORD)StrToUNum32(lpszPort,(dwRcvBuffSizeUsed-(lpszPort-(LPSTR)btBuff)));
				dwRetErrorCode=NO_ERROR;
			}else{
				dwRetErrorCode=ERROR_INVALID_USER_BUFFER;
				ShowFormatedErrorMessage(L"NLB data corrupted",NO_ERROR);
			}
		}else{
			dwRetErrorCode=GetLastError();
			ShowFormatedErrorMessage(L"Can't get data for NLB, error",dwRetErrorCode);
		}
	}else{
		dwRetErrorCode=GetLastError();
		ShowFormatedErrorMessage(L"Can't connect to NLB server, error",dwRetErrorCode);
	}
return(dwRetErrorCode);
}


DWORD MraNetworkDispather()
{
	DWORD dwRetErrorCode=NO_ERROR;

	BOOL bContinue=TRUE;
	DWORD dwSelectRet,dwBytesReceived,dwDataCurrentBuffSize,dwDataCurrentBuffSizeUsed,dwPingPeriod=MAXDWORD,dwNextPingSendTickTime=MAXDWORD;
	SIZE_T dwRcvBuffSize=BUFF_SIZE_RCV,dwRcvBuffSizeUsed=0,dwDataCurrentBuffOffset=0;
	LPBYTE lpbBufferRcv;
	NETLIBSELECT nls={0};
	mrim_packet_header_t *pmaHeader;

	nls.cbSize=sizeof(nls);
	nls.dwTimeout=NETLIB_SELECT_TIMEOUT;
	nls.hReadConns[0]=masMraSettings.hConnection;

	lpbBufferRcv=(LPBYTE)MEMALLOC(dwRcvBuffSize);

	masMraSettings.dwCMDNum=0;
	MraSendCMD(MRIM_CS_HELLO,NULL,0);
	InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,GetTickCount());
	while(MraGetStatus(0,0)!=ID_STATUS_OFFLINE && bContinue)
	{
		dwSelectRet=CallService(MS_NETLIB_SELECT,0,(LPARAM)&nls);
		switch(dwSelectRet){
		case SOCKET_ERROR:
			if (MraGetStatus(0,0)!=ID_STATUS_OFFLINE)
			{
				dwRetErrorCode=GetLastError();
				ShowFormatedErrorMessage(L"Disconnected, socket error",dwRetErrorCode);
			}
			bContinue=FALSE;
			break;
		case 0:// Time out
		case 1:
			dwBytesReceived=GetTickCount();
			InterlockedExchange((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,dwBytesReceived);
			if (dwNextPingSendTickTime<=dwBytesReceived)
			{// server ping
				dwNextPingSendTickTime=(dwBytesReceived+(dwPingPeriod*1000));
				MraSendCMD(MRIM_CS_PING,NULL,0);
			}
			{
				DWORD dwCMDNum,dwFlags,dwAckType;
				HANDLE hContact;
				LPBYTE lpbData;
				SIZE_T dwDataSize;
				while (MraSendQueueFindOlderThan(masMraSettings.hSendQueueHandle,SEND_QUEUE_TIMEOUT,&dwCMDNum,&dwFlags,&hContact,&dwAckType,&lpbData,&dwDataSize)==NO_ERROR)
				{
					switch(dwAckType){
					case ACKTYPE_ADDED:
					case ACKTYPE_AUTHREQ:
					case ACKTYPE_CONTACTS:
						//nothink to do
						break;
					case ACKTYPE_MESSAGE:
						ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)dwCMDNum,(LPARAM)"Undefined message deliver error, time out",-1);
						break;
					case ACKTYPE_GETINFO:
						ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)1,(LPARAM)NULL,0);
						break;
					case ACKTYPE_SEARCH:
						ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_SUCCESS,(HANDLE)dwCMDNum,(LPARAM)NULL,0);
						break;
					case ICQACKTYPE_SMS:
						MEMFREE(lpbData);
						break;
					}
					MraSendQueueFree(masMraSettings.hSendQueueHandle,dwCMDNum);
				}
			}
			if (dwSelectRet==0) break;// Time out


			if ((dwRcvBuffSize-dwRcvBuffSizeUsed)<BUFF_SIZE_RCV_MIN_FREE)
			{// динамическое увеличение буффера приёма
				dwRcvBuffSize+=BUFF_SIZE_RCV;
				lpbBufferRcv=(LPBYTE)MEMREALLOC(lpbBufferRcv,dwRcvBuffSize);
			}

			dwBytesReceived=Netlib_Recv(nls.hReadConns[0],(LPSTR)(lpbBufferRcv+dwRcvBuffSizeUsed),(dwRcvBuffSize-dwRcvBuffSizeUsed),0);
			if (dwBytesReceived && dwBytesReceived!=SOCKET_ERROR)
			{// connected
				dwRcvBuffSizeUsed+=dwBytesReceived;

				while(TRUE)
				{// разбор принятого
					dwDataCurrentBuffSize=(dwRcvBuffSize-dwDataCurrentBuffOffset);
					dwDataCurrentBuffSizeUsed=(dwRcvBuffSizeUsed-dwDataCurrentBuffOffset);
					pmaHeader=(mrim_packet_header_t*)(lpbBufferRcv+dwDataCurrentBuffOffset);
					if (dwDataCurrentBuffSizeUsed>=sizeof(mrim_packet_header_t))
					{// packet header received
						if (pmaHeader->magic==CS_MAGIC)
						{// packet OK
							if ((dwDataCurrentBuffSizeUsed-sizeof(mrim_packet_header_t))>=pmaHeader->dlen)
							{// full packet received, may be more than one

								MraCommandDispather(pmaHeader,&dwPingPeriod,&dwNextPingSendTickTime,&bContinue);
								
								if ((dwDataCurrentBuffSizeUsed-sizeof(mrim_packet_header_t))>pmaHeader->dlen)
								{// move pointer to next packet in buffer
									dwDataCurrentBuffOffset+=(sizeof(mrim_packet_header_t)+pmaHeader->dlen);
								}else{// move pointer to begin of buffer
									if (dwRcvBuffSize>BUFF_SIZE_RCV)
									{// динамическое уменьшение буффера приёма
										dwRcvBuffSize=BUFF_SIZE_RCV;
										lpbBufferRcv=(LPBYTE)MEMREALLOC(lpbBufferRcv,dwRcvBuffSize);
									}
									dwDataCurrentBuffOffset=0;
									dwRcvBuffSizeUsed=0;
									break;
								}
							}else{// not all packet received, continue receiving
								if (dwDataCurrentBuffOffset)
								{
									memmove(lpbBufferRcv,(lpbBufferRcv+dwDataCurrentBuffOffset),dwDataCurrentBuffSizeUsed);
									dwRcvBuffSizeUsed=dwDataCurrentBuffSizeUsed;
									dwDataCurrentBuffOffset=0;
								}
								DebugPrintCRLFW(L"Not all packet received, continue receiving");
								break;
							}
						}else{// bad packet
							DebugPrintCRLFW(L"Bad packet");
							dwDataCurrentBuffOffset=0;
							dwRcvBuffSizeUsed=0;
							break;
						}
					}else{// packet to small, continue receiving
						DebugPrintCRLFW(L"Packet to small, continue receiving");
						memmove(lpbBufferRcv,(lpbBufferRcv+dwDataCurrentBuffOffset),dwDataCurrentBuffSizeUsed);
						dwRcvBuffSizeUsed=dwDataCurrentBuffSizeUsed;
						dwDataCurrentBuffOffset=0;
						break;
					}
				}
			}else{// disconnected
				if (MraGetStatus(0,0)!=ID_STATUS_OFFLINE)
				{
					dwRetErrorCode=GetLastError();
					ShowFormatedErrorMessage(L"Disconnected, socket read error",dwRetErrorCode);
				}
				bContinue=FALSE;
			}
			break;
		}// end switch
	}// end while
	MEMFREE(lpbBufferRcv);

return(dwRetErrorCode);
}


DWORD MraCommandDispather(mrim_packet_header_t *pmaHeader,DWORD *pdwPingPeriod,DWORD *pdwNextPingSendTickTime,BOOL *pbContinue)
{
	WCHAR szBuff[4096]={0};
	DWORD dwDataSize,dwTemp,dwAckType;
	SIZE_T dwStringSize;
	MRA_LPS lpsString={0},lpsEMail={0};
	HANDLE hContact=NULL;
	LPBYTE lpbData,lpbDataCurrent;

	lpbData=((((LPBYTE)pmaHeader))+sizeof(mrim_packet_header_t));
	lpbDataCurrent=lpbData;
	dwDataSize=pmaHeader->dlen;


	switch(pmaHeader->msg){
	case MRIM_CS_HELLO_ACK://Подтверждение установки соединения// UL ## ping_period ## Ожидаемая частота подтверждения соединения (в секундах)
		//CredUIPromptForCredentials
		//CYPTPROTECTMEMORY_BLOCK_SIZE=RTL_ENCRYPT_MEMORY_SIZE=8
		//CryptProtectMemory(szBuff,sizeof(szBuff),CRYPTPROTECTMEMORY_SAME_PROCESS);
		if(GetPassDB((LPSTR)szBuff,SIZEOF(szBuff),&dwStringSize))
		{//bit of a security hole here, since it's easy to extract a password from an edit box
			CHAR szEMail[MAX_EMAIL_LEN],szSelfVersionString[MAX_PATH],szUserAgentFormated[USER_AGENT_MAX+MAX_PATH],szValueName[MAX_PATH];
			WCHAR wszStatusTitle[STATUS_TITLE_MAX+4],wszStatusDesc[STATUS_DESC_MAX+4];
			DWORD dwStatus,dwXStatus,dwXStatusMir,dwFutureFlags;
			LPWSTR lpwszStatusTitle,lpwszStatusDesc;
			SIZE_T dwEMailSize,dwSelfVersionSize,dwStatusTitleSize,dwStatusDescSize,dwUserAgentFormatedSize;

			dwXStatusMir=MraGetXStatusInternal();
			dwStatus=GetMraStatusFromMiradaStatus(masMraSettings.dwDesiredStatusMode,dwXStatusMir,&dwXStatus);
			if (IsXStatusValid(dwXStatusMir))
			{// xstatuses
				mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldName",dwXStatusMir);
				if (DB_Mra_GetStaticStringW(NULL,szValueName,wszStatusTitle,(STATUS_TITLE_MAX+1),&dwStatusTitleSize))
				{// custom xstatus name
					lpwszStatusTitle=wszStatusTitle;
				}else{// default xstatus name
					lpwszStatusTitle=TranslateW(lpcszXStatusNameDef[dwXStatusMir]);
					dwStatusTitleSize=lstrlenW(lpwszStatusTitle);
				}

				mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldMsg",dwXStatusMir);
				if (DB_Mra_GetStaticStringW(NULL,szValueName,wszStatusDesc,(STATUS_DESC_MAX+1),&dwStatusDescSize))
				{// custom xstatus description
					lpwszStatusDesc=wszStatusDesc;
				}else{// default xstatus description
					lpwszStatusDesc=NULL;
					dwStatusDescSize=0;
				}
			}else{// not xstatuses
				lpwszStatusTitle=GetStatusModeDescriptionW(masMraSettings.dwDesiredStatusMode);
				dwStatusTitleSize=lstrlenW(lpwszStatusTitle);
				lpwszStatusDesc=NULL;
				dwStatusDescSize=0;
			}

			MraGetSelfVersionString(szSelfVersionString,SIZEOF(szSelfVersionString),&dwSelfVersionSize);
			if (DB_Mra_GetStaticStringA(NULL,"MirVerCustom",szUserAgentFormated,SIZEOF(szUserAgentFormated),&dwUserAgentFormatedSize)==FALSE)
			{
				dwUserAgentFormatedSize=mir_snprintf(szUserAgentFormated,SIZEOF(szUserAgentFormated),"client=\"magent\" name=\"Miranda IM\" title=\"%s\" version=\"777.%lu.%lu.%lu\" build=\"%lu\" protocol=\"%lu.%lu\"",szSelfVersionString,(((PLUGIN_VERSION_DWORD)>>24)&0xFF),(((PLUGIN_VERSION_DWORD)>>16)&0xFF),(((PLUGIN_VERSION_DWORD)>>8)&0xFF),((PLUGIN_VERSION_DWORD)&0xFF),PROTO_MAJOR(PROTO_VERSION),PROTO_MINOR(PROTO_VERSION));// "client=\"magent\" version=\"9.3\" build=\"777\""
			}

			dwFutureFlags=((DB_Mra_GetByte(NULL,"RTFReceiveEnable",MRA_DEFAULT_RTF_RECEIVE_ENABLE)? FEATURE_FLAG_RTF_MESSAGE:0)|MRA_FEATURE_FLAGS);

			if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraSendCommand_Login2W(szEMail,dwEMailSize,(LPSTR)szBuff,dwStringSize,dwStatus,lpcszStatusUri[dwXStatus],lstrlenA(lpcszStatusUri[dwXStatus]),lpwszStatusTitle,dwStatusTitleSize,lpwszStatusDesc,dwStatusDescSize,dwFutureFlags,szUserAgentFormated,dwUserAgentFormatedSize,szSelfVersionString,dwSelfVersionSize);
			}else{// no login
				(*pbContinue)=FALSE;
			}
			SecureZeroMemory(szBuff,sizeof(szBuff));
		}else{// no password
			(*pbContinue)=FALSE;
		}
		(*pdwPingPeriod)=GetUL(&lpbDataCurrent);
		break;
	case MRIM_CS_LOGIN_ACK://Успешная авторизация
		masMraSettings.bLoggedIn=TRUE;
		(*pdwNextPingSendTickTime)=0;// force send ping
		MraSendCMD(MRIM_CS_PING,NULL,0);
		MraSetStatus(masMraSettings.dwDesiredStatusMode,masMraSettings.dwDesiredStatusMode);
		//(*((LPBYTE)NULL))=1;// force exception
		//while(TRUE) Sleep(1); // force infinite loop
		MraAvatarsQueueGetAvatarSimple(masMraSettings.hAvatarsQueueHandle,GAIF_FORCE,NULL,0);
		break;
	case MRIM_CS_LOGIN_REJ://Неверная авторизация //LPS ## reason ## причина отказа
		ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,ACKTYPE_LOGIN,ACKRESULT_FAILED,NULL,LOGINERR_WRONGPASSWORD,0);

		GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
		dwStringSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpsString.lpszData,lpsString.dwSize,szBuff,SIZEOF(szBuff));
		szBuff[dwStringSize]=0;
		MraPopupShowW(NULL,MRA_POPUP_TYPE_ERROR,0,TranslateW(L"Logon error: invalid login/password"),szBuff);
		(*pbContinue)=FALSE;
		break;
	case MRIM_CS_MESSAGE_ACK:// Доставка сообщения
		{
			DWORD dwMsgID,dwFlags;
			MRA_LPS lpsText,lpsRTFText,lpsMultiChatData;

			dwMsgID=GetUL(&lpbDataCurrent);
			dwFlags=GetUL(&lpbDataCurrent);
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);//LPS ## from ## Адрес отправителя
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsText);//LPS ## message ## текстовая версия сообщения
			//if (dwFlags&MESSAGE_FLAG_RTF)
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsRTFText);//LPS ## rtf-message ## форматированная версия сообщения
			if (dwFlags&MESSAGE_FLAG_MULTICHAT) GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsMultiChatData); // LPS multichat_data

			if (MraRecvCommand_Message((DWORD)_time32(NULL),dwFlags,&lpsEMail,&lpsText,&lpsRTFText,&lpsMultiChatData)==NO_ERROR)
			{// подтверждаем получение, только если удалось его обработать
				if ((dwFlags&MESSAGE_FLAG_NORECV)==0)
				{// need send delivery status
					MraSendCommand_MessageRecv(lpsEMail.lpszData,lpsEMail.dwSize,dwMsgID);
				}
			}
		}
		break;
	case MRIM_CS_MESSAGE_STATUS:
		if (MraSendQueueFind(masMraSettings.hSendQueueHandle,pmaHeader->seq,NULL,&hContact,&dwAckType,(LPBYTE*)&lpsString.lpszData,&lpsString.dwSize)==NO_ERROR)
		{
			dwTemp=GetUL(&lpbDataCurrent);
			switch(dwTemp){
			case MESSAGE_DELIVERED:// Message delivered directly to user
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_SUCCESS,(HANDLE)pmaHeader->seq,(LPARAM)NULL,0);
				break;//***deb возможны сбои из-за асинхронности тк там передаётся указатель
			case MESSAGE_REJECTED_NOUSER:// Message rejected - no such user
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)"Message rejected - no such user",-1);
				break;
			case MESSAGE_REJECTED_INTERR:// Internal server error
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)"Internal server error",-1);
				break;
			case MESSAGE_REJECTED_LIMIT_EXCEEDED:// Offline messages limit exceeded
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)"Offline messages limit exceeded",-1);
				break;
			case MESSAGE_REJECTED_TOO_LARGE:// Message is too large
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)"Message is too large",-1);
				break;
			case MESSAGE_REJECTED_DENY_OFFMSG:// User does not accept offline messages
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)"User does not accept offline messages",-1);
				break;
			case MESSAGE_REJECTED_DENY_OFFFLSH:// User does not accept offline flash animation
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)"User does not accept offline flash animation",-1);
				break;
			default:
				dwTemp=mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),"Undefined message deliver error, code: %lu",dwTemp);
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)pmaHeader->seq,(LPARAM)szBuff,dwTemp);
				break;
			}
			MraSendQueueFree(masMraSettings.hSendQueueHandle,pmaHeader->seq);
		}else{// not found in queue
			if (GetUL(&lpbDataCurrent)!=MESSAGE_DELIVERED) MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,TranslateW(L"MRIM_CS_MESSAGE_STATUS: not found in queue"));
		}
		break;
	case MRIM_CS_CONNECTION_PARAMS:// Изменение параметров соединения 
		(*pdwPingPeriod)=GetUL(&lpbDataCurrent);
		(*pdwNextPingSendTickTime)=0;// force send ping
		MraSendCMD(MRIM_CS_PING,NULL,0);
		break;
	case MRIM_CS_USER_INFO:
		while (lpbDataCurrent<(lpbData+dwDataSize))
		{
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"MESSAGES.TOTAL",14)==CSTR_EQUAL)
			{
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				masMraSettings.dwEmailMessagesTotal=StrToUNum32(lpsString.lpszData,lpsString.dwSize);
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"MESSAGES.UNREAD",15)==CSTR_EQUAL)
			{
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				masMraSettings.dwEmailMessagesUnRead=StrToUNum32(lpsString.lpszData,lpsString.dwSize);
			}else			
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"MRIM.NICKNAME",13)==CSTR_EQUAL)
			{
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				DB_Mra_SetLPSStringW(NULL,"Nick",&lpsString);
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"client.endpoint",15)==CSTR_EQUAL)
			{
				LPSTR lpszDelimiter;

				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				dwStringSize=WideCharToMultiByte(MRA_CODE_PAGE,0,lpsString.lpwszData,lpsString.dwSize,(LPSTR)szBuff,sizeof(szBuff),NULL,NULL);
				lpszDelimiter=(LPSTR)MemoryFind(0,szBuff,dwStringSize,":",1);
				if (lpszDelimiter)
				{
					(*lpszDelimiter)=0;
					lpszDelimiter=(LPSTR)szBuff;
					DB_Mra_SetDword(NULL,"IP",HTONL(inet_addr(lpszDelimiter)));
				}
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"connect.xml",11)==CSTR_EQUAL)
			{
				DebugPrintA(lpsString.lpszData);
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				DebugPrintCRLFW(lpsString.lpwszData);
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"micblog.show_title",18)==CSTR_EQUAL)
			{
				DebugPrintA(lpsString.lpszData);
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				DebugPrintCRLFW(lpsString.lpwszData);
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"micblog.status.id",17)==CSTR_EQUAL)
			{
				DWORDLONG dwBlogStatusID;

				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				dwBlogStatusID=StrToUNum64(lpsString.lpszData,lpsString.dwSize);
				DB_Mra_WriteContactSettingBlob(NULL,DBSETTING_BLOGSTATUSID,&dwBlogStatusID,sizeof(DWORDLONG));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"micblog.status.time",19)==CSTR_EQUAL)
			{
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"micblog.status.text",19)==CSTR_EQUAL)
			{
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				DB_Mra_SetLPSStringW(NULL,DBSETTING_BLOGSTATUS,&lpsString);
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"HAS_MYMAIL",10)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"mrim.status.open_search",23)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"rb.target.cookie",16)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"show_web_history_link",21)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"friends_suggest",15)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"timestamp",9)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsString.lpszData,lpsString.dwSize,"trusted_update",14)==CSTR_EQUAL)
			{// ???
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				//DB_Mra_SetDword(NULL,DBSETTING_BLOGSTATUSTIME,StrToUNum32(lpsString.lpszData,lpsString.dwSize));
			}else{
			#ifdef _DEBUG
				LPSTR lpszCurrentPos=(LPSTR)szBuff;
				memmove(lpszCurrentPos,lpsString.lpszData,lpsString.dwSize);
				lpszCurrentPos+=lpsString.dwSize;
				(*((WORD*)lpszCurrentPos))=(*((WORD*)": "));
				lpszCurrentPos+=sizeof(WORD);

				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
				memmove(lpszCurrentPos,lpsString.lpszData,lpsString.dwSize);
				lpszCurrentPos+=lpsString.dwSize;
				(*((WORD*)lpszCurrentPos))=(*((WORD*)szCRLF));
				lpszCurrentPos+=sizeof(WORD);(*((WORD*)lpszCurrentPos))=0;

				DebugPrintCRLFA((LPSTR)szBuff);
				//MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,szBuff);
				DebugBreak();
			#endif
			}	
		}
		MraUpdateEmailStatus(NULL,0,NULL,0,0,0);
		break;
	case MRIM_CS_OFFLINE_MESSAGE_ACK://Сообщение доставленное, пока пользователь не был подключен к сети 
		{
			DWORD dwTime,dwFlags;
			MRA_LPS lpsText,lpsRTFText,lpsMultiChatData;
			LPBYTE lpbBuff=NULL;
			DWORDLONG dwMsgUIDL;

			dwMsgUIDL=GetUIDL(&lpbDataCurrent);
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);

			if (MraOfflineMessageGet(&lpsString,&dwTime,&dwFlags,&lpsEMail,&lpsText,&lpsRTFText,&lpsMultiChatData,&lpbBuff)==NO_ERROR)
			{
				dwTemp=MraRecvCommand_Message(dwTime,dwFlags,&lpsEMail,&lpsText,&lpsRTFText,&lpsMultiChatData);
				if (dwTemp==NO_ERROR || dwTemp==ERROR_ACCESS_DENIED)
				{// подтверждаем получение, только если удалось его обработать
					MraSendCommand_OfflineMessageDel(dwMsgUIDL);
				}else{
					ShowFormatedErrorMessage(L"Offline message processing error, message will not deleted from server",NO_ERROR);
				}
			}else{
				ShowFormatedErrorMessage(L"Offline message processing error, message will not deleted from server",NO_ERROR);
			}
			MEMFREE(lpbBuff);
		}
		break;
	case MRIM_CS_AUTHORIZE_ACK://Информация об авторизации 
		{// нас автоизовали, те разрешили нам получать уведомление об изменении статуса, значит юзер у нас в списке
			BOOL bAdded;
			BYTE btBuff[BUFF_SIZE_BLOB];
			DBEVENTINFO dbei={0};

			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);
			hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,TRUE,TRUE,&bAdded);
			if (bAdded) MraUpdateContactInfo(hContact);

			if (IsEMailChatAgent(lpsEMail.lpszData,lpsEMail.dwSize)==FALSE)
			{
				dbei.cbSize=sizeof(dbei);
				dbei.szModule=PROTOCOL_NAMEA;
				dbei.timestamp=(DWORD)_time32(NULL);
				dbei.flags=0;
				dbei.eventType=EVENTTYPE_ADDED;
				//dbei.cbBlob=0;
				CreateBlobFromContact(hContact,NULL,0,(LPBYTE)&btBuff,SIZEOF(btBuff),&dwStringSize);
				dbei.cbBlob=dwStringSize;
				dbei.pBlob=btBuff;

				CallService(MS_DB_EVENT_ADD,(WPARAM)NULL,(LPARAM)&dbei);
			}

			GetContactBasicInfoW(hContact,NULL,NULL,NULL,&dwTemp,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
			dwTemp&=~CONTACT_INTFLAG_NOT_AUTHORIZED;
			SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,SCBIF_SERVER_FLAG,0,0,0,dwTemp,0,NULL,0,NULL,0,NULL,0);
			DB_Mra_SetDword(hContact,"HooksLocked",TRUE);
			DBDeleteContactSetting(hContact,"CList","NotOnList");
			DB_Mra_SetDword(hContact,"HooksLocked",FALSE);
		}
		break;
	case MRIM_CS_MPOP_SESSION://Ключ для web-авторизации //
		if (GetUL(&lpbDataCurrent))
		{
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
			MraMPopSessionQueueSetNewMPopKey(masMraSettings.hMPopSessionQueue,lpsString.lpszData,lpsString.dwSize);
			MraMPopSessionQueueStart(masMraSettings.hMPopSessionQueue);
		}else{//error
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING,0,TranslateW(L"Server error: cant get MPOP key for web authorize"));
			MraMPopSessionQueueFlush(masMraSettings.hMPopSessionQueue);
		}
		break;
/////////////////////////////////////////////////////////////////////////////////////
	case MRIM_CS_FILE_TRANSFER:
		{
			BOOL bAdded;
			DWORD dwIDRequest,dwFilesTotalSize;
			MRA_LPS lpsFiles={0},lpsFilesW={0},lpsAddreses={0};

			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);// LPS TO/FROM ANSI
			dwIDRequest=GetUL(&lpbDataCurrent);// DWORD id_request
			dwFilesTotalSize=GetUL(&lpbDataCurrent);// DWORD FILESIZE
			if (GetUL(&lpbDataCurrent))//LPS:
			{
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsFiles);// LPS Files (FileName;FileSize;FileName;FileSize;) ANSI
				if (GetUL(&lpbDataCurrent))// LPS DESCRIPTION
				{
					dwTemp=GetUL(&lpbDataCurrent);// ???
					DebugBreakIf(dwTemp!=1);
					GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsFilesW);// LPS Files (FileName;FileSize;FileName;FileSize;) UNICODE
				}
				GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsAddreses);// LPS Conn (IP:Port;IP:Port;) ANSI
			}

			hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,TRUE,TRUE,&bAdded);
			if (bAdded) MraUpdateContactInfo(hContact);

			bAdded=FALSE;
			if (lpsFilesW.dwSize==0)
			{
				lpsFilesW.lpwszData=(LPWSTR)MEMALLOC((lpsFiles.dwSize+MAX_PATH)*sizeof(WCHAR));
				if (lpsFilesW.lpwszData)
				{
					lpsFilesW.dwSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpsFiles.lpszData,lpsFiles.dwSize,lpsFilesW.lpwszData,(lpsFiles.dwSize+MAX_PATH));
					bAdded=TRUE;
				}
			}

			if (lpsFilesW.dwSize) MraFilesQueueAddReceive(masMraSettings.hFilesQueueHandle,0,hContact,dwIDRequest,lpsFilesW.lpwszData,lpsFilesW.dwSize,lpsAddreses.lpszData,lpsAddreses.dwSize);
			if (bAdded) MEMFREE(lpsFilesW.lpwszData);
		}
		break;
	case MRIM_CS_FILE_TRANSFER_ACK:
		dwAckType=GetUL(&lpbDataCurrent);// DWORD status 
		GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);// LPS TO/FROM 
		dwTemp=GetUL(&lpbDataCurrent);// DWORD id_request
		GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);// LPS DESCRIPTION

		switch(dwAckType){
		case FILE_TRANSFER_STATUS_OK:// игнорируем, мы и так уже слушаем порт(ждём), то что кто то согласился ничего не меняет
			//hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,TRUE,TRUE,NULL);
			break;
		case FILE_TRANSFER_STATUS_DECLINE:
			MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,dwTemp,FALSE);
			break;
		case FILE_TRANSFER_STATUS_ERROR:
			ShowFormatedErrorMessage(L"File transfer: error",NO_ERROR);
			MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,dwTemp,FALSE);
			break;
		case FILE_TRANSFER_STATUS_INCOMPATIBLE_VERS:
			ShowFormatedErrorMessage(L"File transfer: incompatible versions",NO_ERROR);
			MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,dwTemp,FALSE);
			break;
		case FILE_TRANSFER_MIRROR:
			MraFilesQueueSendMirror(masMraSettings.hFilesQueueHandle,dwTemp,lpsString.lpszData,lpsString.dwSize);
			break;
		default:// ## unknown error
			mir_sntprintf(szBuff,SIZEOF(szBuff),TranslateW(L"MRIM_CS_FILE_TRANSFER_ACK: unknown error, code: %lu"),dwAckType);
			ShowFormatedErrorMessage(szBuff,NO_ERROR);
			break;
		}
		break;
/////////////////////////////////////////////////////////////////////////////////////
	case MRIM_CS_USER_STATUS://Смена статуса другого пользователя
		{
			BOOL bAdded;
			DWORD dwStatus,dwXStatus,dwFutureFlags;
			MRA_LPS lpsSpecStatusUri,lpsStatusTitle,lpsStatusDesc,lpsUserAgentFormated;

			dwStatus=GetUL(&lpbDataCurrent);
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsSpecStatusUri);
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsStatusTitle);
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsStatusDesc);
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);
			dwFutureFlags=GetUL(&lpbDataCurrent);// com_support (>=1.14)
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsUserAgentFormated);


			if ((hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,TRUE,TRUE,&bAdded)))
			{
				if (bAdded) MraUpdateContactInfo(hContact);

				dwTemp=GetMiradaStatusFromMraStatus(dwStatus,GetMraXStatusIDFromMraUriStatus(lpsSpecStatusUri.lpszData,lpsSpecStatusUri.dwSize),&dwXStatus);
				
				MraContactCapabilitiesSet(hContact,dwFutureFlags);
				DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,(BYTE)dwXStatus);
				DB_Mra_SetLPSStringW(hContact,DBSETTING_XSTATUSNAME,&lpsStatusTitle);
				DB_Mra_SetLPSStringW(hContact,DBSETTING_XSTATUSMSG,&lpsStatusDesc);
				
				if (dwTemp!=ID_STATUS_OFFLINE)// пишем клиента только если юзер не отключён, иначе не затираем старое
				{
					if (lpsUserAgentFormated.dwSize)
					{// есть чё писать
						if (DB_Mra_GetByte(NULL,"MirVerRaw",MRA_DEFAULT_MIRVER_RAW)==FALSE)
						{// приводим к человеческому виду, если получится...
							MraGetVersionStringFromFormated(lpsUserAgentFormated.lpszData,lpsUserAgentFormated.dwSize,(LPSTR)szBuff,SIZEOF(szBuff),&dwStringSize);
							lpsUserAgentFormated.lpszData=(LPSTR)szBuff;
							lpsUserAgentFormated.dwSize=dwStringSize;
						}
					}else{// хз чё за клиент
						lpsUserAgentFormated.lpszData=MIRVER_UNKNOWN;
						lpsUserAgentFormated.dwSize=(sizeof(MIRVER_UNKNOWN)-1);
					}
					DB_Mra_SetLPSStringA(hContact,"MirVer",&lpsUserAgentFormated);
				}

				if (dwTemp==MraGetContactStatus(hContact))
				{// меняем шило на шило, подозрительно? ;)
					if (dwTemp==ID_STATUS_OFFLINE)
					{// was/now invisible
						WCHAR szEMail[MAX_EMAIL_LEN];

						DB_Mra_GetStaticStringW(hContact,"e-mail",szEMail,SIZEOF(szEMail),NULL);
						mir_sntprintf(szBuff,SIZEOF(szBuff),L"%s <%s> - %s",GetContactNameW(hContact),szEMail,TranslateW(L"invisible status changed"));
						MraPopupShowFromContactW(hContact,MRA_POPUP_TYPE_INFORMATION,0,szBuff);

						MraSetContactStatus(hContact,ID_STATUS_INVISIBLE);
					}else{// server or miranda bug or status change
						//DebugBreak();
					}
				}
				MraSetContactStatus(hContact,dwTemp);
				SetExtraIcons(hContact);
			}
		}
		break;
	case MRIM_CS_LOGOUT:// Пользователь отключен из-за параллельного входа с его логином.
		if (GetUL(&lpbDataCurrent)==LOGOUT_NO_RELOGIN_FLAG) ShowFormatedErrorMessage(L"Another user connected with your login",NO_ERROR);
		(*pbContinue)=FALSE;
		break;
	case MRIM_CS_ADD_CONTACT_ACK:
	case MRIM_CS_MODIFY_CONTACT_ACK:
		if (MraSendQueueFind(masMraSettings.hSendQueueHandle,pmaHeader->seq,NULL,&hContact,&dwAckType,(LPBYTE*)&lpsString.lpszData,&lpsString.dwSize)==NO_ERROR)
		{
			dwTemp=GetUL(&lpbDataCurrent);
			switch(dwTemp){
			case CONTACT_OPER_SUCCESS:// ## добавление произведено успешно
				if (pmaHeader->msg==MRIM_CS_ADD_CONTACT_ACK) SetContactBasicInfoW(hContact,0,(SCBIF_ID|SCBIF_SERVER_FLAG),GetUL(&lpbDataCurrent),0,0,CONTACT_INTFLAG_NOT_AUTHORIZED,0,NULL,0,NULL,0,NULL,0);
				break;
			case CONTACT_OPER_ERROR:// ## переданные данные были некорректны
				ShowFormatedErrorMessage(L"Sended data is invalid",NO_ERROR);
				break;
			case CONTACT_OPER_INTERR:// ## при обработке запроса произошла внутренняя ошибка
				ShowFormatedErrorMessage(L"Internal server error",NO_ERROR);
				break;
			case CONTACT_OPER_NO_SUCH_USER:// ## добавляемого пользователя не существует в системе
				SetContactBasicInfoW(hContact,0,SCBIF_SERVER_FLAG,0,0,0,-1,0,NULL,0,NULL,0,NULL,0);
				ShowFormatedErrorMessage(L"User does not registred",NO_ERROR);
				break;
			case CONTACT_OPER_INVALID_INFO:// ## некорректное имя пользователя
				ShowFormatedErrorMessage(L"Invalid user name",NO_ERROR);
				break;
			case CONTACT_OPER_USER_EXISTS:// ## пользователь уже есть в контакт-листе
				ShowFormatedErrorMessage(L"User allready added",NO_ERROR);
				break;
			case CONTACT_OPER_GROUP_LIMIT:// ## превышено максимально допустимое количество групп (20)
				ShowFormatedErrorMessage(L"Group limit is 20",NO_ERROR);
				break;
			default:// ## unknown error
				mir_sntprintf(szBuff,SIZEOF(szBuff),TranslateW(L"MRIM_CS_*_CONTACT_ACK: unknown server error, code: %lu"),dwTemp);
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,szBuff);
				break;
			}
			MraSendQueueFree(masMraSettings.hSendQueueHandle,pmaHeader->seq);
		}else{// not found in queue
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,TranslateW(L"MRIM_CS_*_CONTACT_ACK: not found in queue"));
		}
		break;
	case MRIM_CS_ANKETA_INFO:
		if (MraSendQueueFind(masMraSettings.hSendQueueHandle,pmaHeader->seq,NULL,&hContact,&dwAckType,(LPBYTE*)&lpsString.lpszData,&lpsString.dwSize)==NO_ERROR)
		{
			switch(GetUL(&lpbDataCurrent)){
			case MRIM_ANKETA_INFO_STATUS_OK:// поиск успешно завершен
				{
					DWORD dwFeildsNum,dwMaxRows,dwServerTime,dwStatus;
					SIZE_T i;
					MRA_LPS *pmralpsFeilds,*pmralpsValues;

					dwFeildsNum=GetUL(&lpbDataCurrent);
					dwMaxRows=GetUL(&lpbDataCurrent);
					dwServerTime=GetUL(&lpbDataCurrent);

					pmralpsFeilds=(MRA_LPS*)MEMALLOC(((dwFeildsNum*2)+4)*sizeof(MRA_LPS));
					if (pmralpsFeilds)
					{
						pmralpsValues=(pmralpsFeilds+dwFeildsNum);
						// read headers name
						for (i=0;i<dwFeildsNum;i++)
						{
							GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&pmralpsFeilds[i]);
							DebugPrintCRLFA(pmralpsFeilds[i].lpszData);
						}
						
						while (lpbDataCurrent<(lpbData+dwDataSize))
						{
							// read values
							for (i=0;i<dwFeildsNum;i++) GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&pmralpsValues[i]);

							if (dwAckType==ACKTYPE_GETINFO && hContact)
							{// write to DB and exit loop
								DB_Mra_SetDword(hContact,"InfoTS",(DWORD)_time32(NULL));
								//MRA_LPS mralpsUsernameValue;
								for (i=0;i<dwFeildsNum;i++)
								{
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Username",8)==CSTR_EQUAL)
									{
										//mralpsUsernameValue=pmralpsValues[i];
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Domain",6)==CSTR_EQUAL)
									{
										//memmove(szBuff,mralpsUsernameValue.lpszData,mralpsUsernameValue.dwSize);
										//(*((WORD*)&szBuff[mralpsUsernameValue.dwSize]))=(*((WORD*)"@"));
										//memmove(&szBuff[mralpsUsernameValue.dwSize+1],pmralpsValues[i].lpszData,pmralpsValues[i].dwSize);
										//(*((WORD*)&szBuff[pmralpsValues[i].dwSize+mralpsUsernameValue.dwSize+1]))=0;
										//DB_Mra_SetStringA(hContact,"e-mail",pmralpsValues[i].lpszData);
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Flags",5)==CSTR_EQUAL)
									{
										//***
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Nickname",8)==CSTR_EQUAL)
									{
										DB_Mra_SetLPSStringW(hContact,"Nick",&pmralpsValues[i]);
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"FirstName",9)==CSTR_EQUAL)
									{
										DB_Mra_SetLPSStringW(hContact,"FirstName",&pmralpsValues[i]);
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"LastName",8)==CSTR_EQUAL)
									{
										DB_Mra_SetLPSStringW(hContact,"LastName",&pmralpsValues[i]);
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Sex",3)==CSTR_EQUAL)
									{
										switch(StrToUNum32(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize)){
										case 1:// мужской
											DB_Mra_SetByte(hContact,"Gender",'M');
											break;
										case 2:// женский
											DB_Mra_SetByte(hContact,"Gender",'F');
											break;
										default:// а фиг его знает
											DB_Mra_DeleteValue(hContact,"Gender");
											break;
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Birthday",8)==CSTR_EQUAL)
									{
										if (pmralpsValues[i].dwSize>9)
										{// calc "Age"
											SYSTEMTIME stTime={0};

											stTime.wYear=(WORD)StrToUNum32(pmralpsValues[i].lpszData,4);
											stTime.wMonth=(WORD)StrToUNum32((pmralpsValues[i].lpszData+5),2);
											stTime.wDay=(WORD)StrToUNum32((pmralpsValues[i].lpszData+8),2);
											DB_Mra_SetWord(hContact,"BirthYear",stTime.wYear);
											DB_Mra_SetByte(hContact,"BirthMonth",(BYTE)stTime.wMonth);
											DB_Mra_SetByte(hContact,"BirthDay",(BYTE)stTime.wDay);

											DB_Mra_SetWord(hContact,"Age",(WORD)GetYears(&stTime));
										}else{
											DB_Mra_DeleteValue(hContact,"BirthYear");
											DB_Mra_DeleteValue(hContact,"BirthMonth");
											DB_Mra_DeleteValue(hContact,"BirthDay");
											DB_Mra_DeleteValue(hContact,"Age");
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"City_id",7)==CSTR_EQUAL)
									{
										dwTemp=StrToUNum32(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize);
										if (dwTemp)
										{
											for(SIZE_T j=0;mrapPlaces[j].lpszData;j++)
											{
												if (mrapPlaces[j].dwCityID==dwTemp)
												{
													DB_Mra_SetStringW(hContact,"City",mrapPlaces[j].lpszData);
													break;
												}
											}
										}else{
											DB_Mra_DeleteValue(hContact,"City");
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Location",8)==CSTR_EQUAL)
									{//***
										DB_Mra_SetLPSStringW(hContact,"About",&pmralpsValues[i]);
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Zodiac",6)==CSTR_EQUAL)
									{
										//***
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"BMonth",6)==CSTR_EQUAL)
									{// used Birthday
										//if (pmralpsValues[i].dwSize) DB_Mra_SetByte(hContact,"BirthMonth",(BYTE)StrToUNum32(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize));
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"BDay",4)==CSTR_EQUAL)
									{// used Birthday
										//if (pmralpsValues[i].dwSize) DB_Mra_SetByte(hContact,"BirthDay",(BYTE)StrToUNum32(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize));
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Country_id",10)==CSTR_EQUAL)
									{
										dwTemp=StrToUNum32(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize);
										if (dwTemp)
										{
											for(SIZE_T j=0;mrapPlaces[j].lpszData;j++)
											{
												if (mrapPlaces[j].dwCountryID==dwTemp)
												{
													DB_Mra_SetStringW(hContact,"Country",mrapPlaces[j].lpszData);
													break;
												}
											}
										}else{
											DB_Mra_DeleteValue(hContact,"Country");
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Phone",5)==CSTR_EQUAL)
									{
										DB_Mra_DeleteValue(hContact,"Phone");
										DB_Mra_DeleteValue(hContact,"Cellular");
										DB_Mra_DeleteValue(hContact,"Fax");

										if (pmralpsValues[i].dwSize)
										{
											LPSTR lpszCurPos;

											lpsString.lpszData=pmralpsValues[i].lpszData;
											lpszCurPos=(LPSTR)MemoryFindByte(0,pmralpsValues[i].lpszData,pmralpsValues[i].dwSize,',');
											if (lpszCurPos)
											{
												lpsString.dwSize=(lpszCurPos-lpsString.lpszData);
											}else{
												lpsString.dwSize=((pmralpsValues[i].lpszData+pmralpsValues[i].dwSize)-lpsString.lpszData);
											}
											DB_Mra_SetLPSStringA(hContact,"Phone",&lpsString);

											if (lpszCurPos)
											{
												lpsString.lpszData=(++lpszCurPos);
												lpszCurPos=(LPSTR)MemoryFindByte((lpszCurPos-pmralpsValues[i].lpszData),pmralpsValues[i].lpszData,pmralpsValues[i].dwSize,',');
												if (lpszCurPos)
												{
													lpsString.dwSize=(lpszCurPos-lpsString.lpszData);
												}else{
													lpsString.dwSize=((pmralpsValues[i].lpszData+pmralpsValues[i].dwSize)-lpsString.lpszData);
												}
												DB_Mra_SetLPSStringA(hContact,"Cellular",&lpsString);
											}

											if (lpszCurPos)
											{
												lpsString.lpszData=(++lpszCurPos);
												lpszCurPos=(LPSTR)MemoryFindByte((lpszCurPos-pmralpsValues[i].lpszData),pmralpsValues[i].lpszData,pmralpsValues[i].dwSize,',');
												if (lpszCurPos)
												{
													lpsString.dwSize=(lpszCurPos-lpsString.lpszData);
												}else{
													lpsString.dwSize=((pmralpsValues[i].lpszData+pmralpsValues[i].dwSize)-lpsString.lpszData);
												}
												DB_Mra_SetLPSStringA(hContact,"Fax",&lpsString);
											}
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"mrim_status",11)==CSTR_EQUAL)
									{
										if (pmralpsValues[i].dwSize)
										{
											DWORD dwID,dwContactSeverFlags;

											GetContactBasicInfoW(hContact,&dwID,NULL,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
											if (dwID==-1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
											{// для авторизованного нам и так присылают правильный статус
												dwStatus=StrHexToUNum32(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize);
												MraSetContactStatus(hContact,GetMiradaStatusFromMraStatus(dwStatus,MRA_MIR_XSTATUS_NONE,NULL));
												DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,(BYTE)MRA_MIR_XSTATUS_NONE);
											}
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"status_uri",10)==CSTR_EQUAL)
									{
										if (pmralpsValues[i].dwSize)
										{
											DWORD dwID,dwContactSeverFlags,dwXStatus;

											GetContactBasicInfoW(hContact,&dwID,NULL,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
											if (dwID==-1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
											{// для авторизованного нам и так присылают правильный статус
												MraSetContactStatus(hContact,GetMiradaStatusFromMraStatus(dwStatus,GetMraXStatusIDFromMraUriStatus(pmralpsValues[i].lpszData,pmralpsValues[i].dwSize),&dwXStatus));
												DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,(BYTE)dwXStatus);
											}
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"status_title",12)==CSTR_EQUAL)
									{
										if (pmralpsValues[i].dwSize)
										{
											DWORD dwID,dwContactSeverFlags;

											GetContactBasicInfoW(hContact,&dwID,NULL,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
											if (dwID==-1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
											{// для авторизованного нам и так присылают правильный статус
												DB_Mra_SetLPSStringW(hContact,DBSETTING_XSTATUSNAME,&pmralpsValues[i]);
											}
										}
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"status_desc",11)==CSTR_EQUAL)
									{
										if (pmralpsValues[i].dwSize)
										{
											DWORD dwID,dwContactSeverFlags;

											GetContactBasicInfoW(hContact,&dwID,NULL,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
											if (dwID==-1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
											{// для авторизованного нам и так присылают правильный статус
												DB_Mra_SetLPSStringW(hContact,DBSETTING_XSTATUSMSG,&pmralpsValues[i]);
											}
										}
									}else{// for DEBUG ONLY
									#ifdef _DEBUG
										DebugPrintCRLFA(pmralpsFeilds[i].lpszData);
										DebugPrintCRLFA(pmralpsValues[i].lpszData);
										//DebugBreak();
									#endif
									}
								}
							}else
							if (dwAckType==ACKTYPE_SEARCH)
							{
								WCHAR szNick[MAX_EMAIL_LEN]={0},
									szFirstName[MAX_EMAIL_LEN]={0},
									szLastName[MAX_EMAIL_LEN]={0},
									szEMail[MAX_EMAIL_LEN]={0};
								MRA_LPS mralpsUsernameValue={0};
								PROTOSEARCHRESULT psr={0};

								psr.cbSize=sizeof(psr);
								psr.flags=PSR_UNICODE;
								psr.nick=szNick;
								psr.firstName=szFirstName;
								psr.lastName=szLastName;
								psr.email=szEMail;
								psr.id=szEMail;

								for (i=0;i<dwFeildsNum;i++)
								{
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Username",8)==CSTR_EQUAL)
									{
										mralpsUsernameValue=pmralpsValues[i];
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Domain",6)==CSTR_EQUAL)
									{// имя было уже задано ранее
										dwStringSize=MultiByteToWideChar(MRA_CODE_PAGE,0,mralpsUsernameValue.lpszData,mralpsUsernameValue.dwSize,szEMail,SIZEOF(szEMail));

										szEMail[dwStringSize]=(*((WCHAR*)L"@"));
										dwStringSize++;

										dwStringSize+=MultiByteToWideChar(MRA_CODE_PAGE,0,pmralpsValues[i].lpszData,pmralpsValues[i].dwSize,&szEMail[dwStringSize],(SIZEOF(szEMail)-(dwStringSize+1)));
										szEMail[dwStringSize]=0;
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"Nickname",8)==CSTR_EQUAL)
									{
										dwStringSize=min((sizeof(szNick)-sizeof(WCHAR)),pmralpsValues[i].dwSize);
										memmove(szNick,pmralpsValues[i].lpwszData,dwStringSize);
										szNick[dwStringSize]=0;
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"FirstName",9)==CSTR_EQUAL)
									{
										dwStringSize=min((sizeof(szFirstName)-sizeof(WCHAR)),pmralpsValues[i].dwSize);
										memmove(szFirstName,pmralpsValues[i].lpwszData,dwStringSize);
										szFirstName[dwStringSize]=0;
									}else
									if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,pmralpsFeilds[i].lpszData,pmralpsFeilds[i].dwSize,"LastName",8)==CSTR_EQUAL)
									{
										dwStringSize=min((sizeof(szLastName)-sizeof(WCHAR)),pmralpsValues[i].dwSize);
										memmove(szLastName,pmralpsValues[i].lpwszData,dwStringSize);
										szLastName[dwStringSize]=0;
									}
								}// end for
								ProtoBroadcastAck(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_DATA,(HANDLE)pmaHeader->seq,(LPARAM)&psr);
							}
						}// end while	

						MEMFREE(pmralpsFeilds);
					}
				}

				switch(dwAckType){
				case ACKTYPE_GETINFO:
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_SUCCESS,(HANDLE)1,(LPARAM)NULL,0);
					break;
				case ACKTYPE_SEARCH:
				default:
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_SUCCESS,(HANDLE)pmaHeader->seq,(LPARAM)NULL,0);
					break;
				}
				break;
			case MRIM_ANKETA_INFO_STATUS_NOUSER:// не найдено ни одной подходящей записи
				SetContactBasicInfoW(hContact,0,SCBIF_SERVER_FLAG,0,0,0,-1,0,NULL,0,NULL,0,NULL,0);
			case MRIM_ANKETA_INFO_STATUS_DBERR:// ошибка базы данных
			case MRIM_ANKETA_INFO_STATUS_RATELIMERR:// слишком много запросов, поиск временно запрещен
				switch(dwAckType){
				case ACKTYPE_GETINFO:
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_FAILED,(HANDLE)1,(LPARAM)NULL,0);
					break;
				case ACKTYPE_SEARCH:
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,hContact,dwAckType,ACKRESULT_SUCCESS,(HANDLE)pmaHeader->seq,(LPARAM)NULL,0);
					break;
				default:
					DebugBreak();
					break;
				}
				break;
			default:// unknown
				DebugBreak();
				break;
			}
			MraSendQueueFree(masMraSettings.hSendQueueHandle,pmaHeader->seq);
		}else{// not found in queue
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,TranslateW(L"MRIM_ANKETA_INFO: not found in queue"));
		}
		break;
	case MRIM_CS_MAILBOX_STATUS:
		dwTemp=GetUL(&lpbDataCurrent);
		if (dwTemp > masMraSettings.dwEmailMessagesTotal) masMraSettings.dwEmailMessagesTotal+=(dwTemp-masMraSettings.dwEmailMessagesUnRead);

		dwAckType=masMraSettings.dwEmailMessagesUnRead;// save old value
		masMraSettings.dwEmailMessagesUnRead=dwTemp;// store new value
		if (DB_Mra_GetByte(NULL,"IncrementalNewMailNotify",MRA_DEFAULT_INC_NEW_MAIL_NOTIFY)==0 || dwAckType<dwTemp || dwTemp==0)
		{
			MraUpdateEmailStatus(NULL,0,NULL,0,0,0);
		}
		break;
	case MRIM_CS_GAME:
		{
			DWORD dwGameSessionID,dwGameMsg,dwGameMsgID;

			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);//LPS to/from
			dwGameSessionID=GetUL(&lpbDataCurrent);//DWORD session unique per game
			dwGameMsg=GetUL(&lpbDataCurrent);//DWORD msg internal game message
			dwGameMsgID=GetUL(&lpbDataCurrent);//DWORD msg_id id for ack
			dwTemp=GetUL(&lpbDataCurrent);//DWORD time_send time of client
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);//LPS data

		#ifdef _DEBUG
			BYTE btBuff[1024]={0};
			memmove(btBuff,lpsString.lpszData,lpsString.dwSize);
		#endif

			switch(dwGameMsg){
			case GAME_CONNECTION_INVITE:
				if (MraGetStatus(0,0)!=ID_STATUS_INVISIBLE) MraSendCommand_Game(lpsEMail.lpszData,lpsEMail.dwSize,dwGameSessionID,GAME_DECLINE,dwGameMsgID,lpsString.lpszData,lpsString.dwSize);
				break;
			case GAME_CONNECTION_ACCEPT:
				break;
			case GAME_DECLINE:
				break;
			case GAME_INC_VERSION:
				break;
			case GAME_NO_SUCH_GAME:// user invisible
				if ((hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,FALSE,TRUE,NULL)))
				if (MraGetContactStatus(hContact)==ID_STATUS_OFFLINE)
				{
					MraSetContactStatus(hContact,ID_STATUS_INVISIBLE);
				}
				break;
			case GAME_JOIN:
				break;
			case GAME_CLOSE:
				break;
			case GAME_SPEED:
				break;
			case GAME_SYNCHRONIZATION:
				break;
			case GAME_USER_NOT_FOUND:
				break;
			case GAME_ACCEPT_ACK:
				break;
			case GAME_PING:
				break;
			case GAME_RESULT:
				break;
			case GAME_MESSAGES_NUMBER:
				break;
			default:
				mir_sntprintf(szBuff,SIZEOF(szBuff),TranslateW(L"MRIM_CS_GAME: unknown internal game message code: %lu"),dwGameMsg);
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,szBuff);
				break;
			}
		}
		break;
	case MRIM_CS_CONTACT_LIST2:
		dwTemp=GetUL(&lpbDataCurrent);
		if (dwTemp==GET_CONTACTS_OK)
		{// получили контакт лист
			BOOL bAdded;
			char szGroupMask[MAX_PATH],szContactMask[MAX_PATH];
			DWORD dwID,dwGroupFlags,dwContactFlag,dwGroupID,dwContactSeverFlags,dwStatus,dwXStatus,dwFutureFlags,dwBlogStatusTime;
			MRA_LPS mralpsGroupName,mralpsNick,mralpsCustomPhones,lpsSpecStatusUri,lpsStatusTitle,lpsStatusDesc,lpsUserAgentFormated,lpsBlogStatus,lpsBlogStatusMusic;
			SIZE_T i,j,dwGroupsCount,dwGroupMaskSize,dwContactMaskSize,dwControlParam;
			ULARGE_INTEGER dwBlogStatusID;

			dwGroupsCount=GetUL(&lpbDataCurrent);
			
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
			dwGroupMaskSize=lpsString.dwSize;
			memmove(szGroupMask,lpsString.lpszData,dwGroupMaskSize);(*(szGroupMask+dwGroupMaskSize))=0;

			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
			dwContactMaskSize=lpsString.dwSize;
			memmove(szContactMask,lpsString.lpszData,dwContactMaskSize);(*(szContactMask+dwContactMaskSize))=0;

			DebugPrintCRLFW(L"Groups:");
			DebugPrintCRLFA(szGroupMask);
			dwID=0;
			for(i=0;i<dwGroupsCount;i++)//groups handle
			{
				dwControlParam=0;
				for(j=0;j<dwGroupMaskSize;j++)//enumerating parameters
				{
					switch(szGroupMask[j]){
					case 's'://LPS
						GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
						break;
					case 'u'://UL
						dwTemp=GetUL(&lpbDataCurrent);
						break;
					case 'z'://sz
						lpsString.lpszData=(LPSTR)lpbDataCurrent;
						lpsString.dwSize=lstrlenA((LPSTR)lpbDataCurrent);
						lpbDataCurrent+=lpsString.dwSize;
						DebugBreak();
						break;
					}
					
					if (j==0 && szGroupMask[j]=='u')
					{// GroupFlags
						dwGroupFlags=dwTemp;
						dwControlParam++;
					}else
					if (j==1 && szGroupMask[j]=='s')
					{// GroupName
						mralpsGroupName=lpsString;
						dwControlParam++;
					}
				}
				
				// add/modify group
				if (dwControlParam>1)// все параметры правильно инициализированны!
				//if (dwGroupFlags&CONTACT_FLAG_GROUP && (dwGroupFlags&CONTACT_FLAG_REMOVED)==0)
				{
					#ifdef _DEBUG
						memmove(szBuff,mralpsGroupName.lpszData,mralpsGroupName.dwSize);
						szBuff[(mralpsGroupName.dwSize/sizeof(WCHAR))]=0;
						DebugPrintW(szBuff);

						mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),": flags: %lu (",dwGroupFlags);
						DebugPrintA((LPSTR)szBuff);
						if (dwGroupFlags&CONTACT_FLAG_REMOVED)		DebugPrintA("CONTACT_FLAG_REMOVED, ");
						if (dwGroupFlags&CONTACT_FLAG_GROUP)		DebugPrintA("CONTACT_FLAG_GROUP, ");
						if (dwGroupFlags&CONTACT_FLAG_INVISIBLE)	DebugPrintA("CONTACT_FLAG_INVISIBLE, ");
						if (dwGroupFlags&CONTACT_FLAG_VISIBLE)		DebugPrintA("CONTACT_FLAG_VISIBLE, ");
						if (dwGroupFlags&CONTACT_FLAG_IGNORE)		DebugPrintA("CONTACT_FLAG_IGNORE, ");
						if (dwGroupFlags&CONTACT_FLAG_SHADOW)		DebugPrintA("CONTACT_FLAG_SHADOW, ");
						if (dwGroupFlags&CONTACT_FLAG_AUTHORIZED)	DebugPrintA("CONTACT_FLAG_AUTHORIZED, ");
						if (dwGroupFlags&CONTACT_FLAG_MULTICHAT)	DebugPrintA("CONTACT_FLAG_MULTICHAT, ");
						if (dwGroupFlags&CONTACT_FLAG_UNICODE_NAME)	DebugPrintA("CONTACT_FLAG_UNICODE_NAME, ");
						if (dwGroupFlags&CONTACT_FLAG_PHONE)		DebugPrintA("CONTACT_FLAG_PHONE, ");
						DebugPrintCRLFA(")");
					#endif//*/
				}
				dwID++;
			}// end for (processing groups)


			DebugPrintCRLFW(L"Contacts:");
			DebugPrintCRLFA(szContactMask);
			dwID=20;
			while (lpbDataCurrent<(lpbData+dwDataSize))
			{
				dwControlParam=0;
				for(j=0;j<dwContactMaskSize;j++)//enumerating parameters
				{
					switch(szContactMask[j]){
					case 's'://LPS
						GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);
						break;
					case 'u'://UL
						dwTemp=GetUL(&lpbDataCurrent);
						break;
					case 'z'://sz
						lpsString.lpszData=(LPSTR)lpbDataCurrent;
						lpsString.dwSize=lstrlenA((LPSTR)lpbDataCurrent);
						lpbDataCurrent+=lpsString.dwSize;
						DebugBreak();
						break;
					}
					
					if (j==0 && szContactMask[j]=='u')
					{// Flags
						dwContactFlag=dwTemp;
						dwControlParam++;
					}else
					if (j==1 && szContactMask[j]=='u')
					{// Group id
						dwGroupID=dwTemp;
						dwControlParam++;
					}else
					if (j==2 && szContactMask[j]=='s')
					{// Email
						lpsEMail=lpsString;
						dwControlParam++;
					}else
					if (j==3 && szContactMask[j]=='s')
					{// Nick
						mralpsNick=lpsString;
						dwControlParam++;
					}else
					if (j==4 && szContactMask[j]=='u')
					{// Server flags
						dwContactSeverFlags=dwTemp;
						dwControlParam++;
					}else
					if (j==5 && szContactMask[j]=='u')
					{// Status
						dwStatus=dwTemp;
						dwControlParam++;
					}else
					if (j==6 && szContactMask[j]=='s')
					{// Custom Phone number,
						mralpsCustomPhones=lpsString;
						dwControlParam++;
					}else
					if (j==7 && szContactMask[j]=='s')
					{// spec_status_uri
						lpsSpecStatusUri=lpsString;
						dwControlParam++;
					}else
					if (j==8 && szContactMask[j]=='s')
					{// status_title
						lpsStatusTitle=lpsString;
						dwControlParam++;
					}else
					if (j==9 && szContactMask[j]=='s')
					{// status_desc
						lpsStatusDesc=lpsString;
						dwControlParam++;
					}else
					if (j==10 && szContactMask[j]=='u')
					{// com_support (future flags)
						dwFutureFlags=dwTemp;
						dwControlParam++;
					}else
					if (j==11 && szContactMask[j]=='s')
					{// user_agent (formated string)
						lpsUserAgentFormated=lpsString;
						dwControlParam++;
					}else
					if (j==12 && szContactMask[j]=='u')
					{// BlogStatusID
						dwBlogStatusID.LowPart=dwTemp;
						dwControlParam++;
					}else
					if (j==13 && szContactMask[j]=='u')
					{// BlogStatusID
						dwBlogStatusID.HighPart=dwTemp;
						dwControlParam++;
					}else
					if (j==14 && szContactMask[j]=='u')
					{// BlogStatusTime
						dwBlogStatusTime=dwTemp;
						dwControlParam++;
					}else
					if (j==15 && szContactMask[j]=='s')
					{// BlogStatus
						lpsBlogStatus=lpsString;
						dwControlParam++;
					}else
					if (j==16 && szContactMask[j]=='s')
					{// BlogStatusMusic
						lpsBlogStatusMusic=lpsString;
						dwControlParam++;
					}else
					if (j==17 && szContactMask[j]=='s')
					{// BlogStatusSender // ignory
						lpsString=lpsString;
						dwControlParam++;
					}else
					if (j==18 && szContactMask[j]=='s')
					{// geo data ?
						lpsString=lpsString;
						dwControlParam++;
					}else
					if (j==19 && szContactMask[j]=='s')
					{// ?????? ?
						lpsString=lpsString;
						dwControlParam++;
						DebugBreakIf(lpsString.dwSize);
					}else{
						if (szContactMask[j]=='s')
						{
							if (lpsString.dwSize)
							{
								//DebugPrintCRLFA(lpsString.lpszData);
								DebugPrintCRLFW(lpsString.lpwszData);
							}
						}else
						if (szContactMask[j]=='u')
						{
							mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),"%lu, ",dwTemp);//;
							DebugPrintCRLFA((LPSTR)szBuff);
						}else{
							DebugBreak();
						}
					}
				}


				#ifdef _DEBUG

					mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),"ID: %lu, Group id: %lu, ",dwID,dwGroupID);
					DebugPrintA((LPSTR)szBuff);


					memmove(szBuff,lpsEMail.lpszData,lpsEMail.dwSize);
					szBuff[(lpsEMail.dwSize/sizeof(WCHAR))]=0;
					DebugPrintA((LPSTR)szBuff);

					mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),": flags: %lu (",dwContactFlag);
					DebugPrintA((LPSTR)szBuff);
					if (dwContactFlag&CONTACT_FLAG_REMOVED)		DebugPrintA("CONTACT_FLAG_REMOVED, ");
					if (dwContactFlag&CONTACT_FLAG_GROUP)		DebugPrintA("CONTACT_FLAG_GROUP, ");
					if (dwContactFlag&CONTACT_FLAG_INVISIBLE)	DebugPrintA("CONTACT_FLAG_INVISIBLE, ");
					if (dwContactFlag&CONTACT_FLAG_VISIBLE)		DebugPrintA("CONTACT_FLAG_VISIBLE, ");
					if (dwContactFlag&CONTACT_FLAG_IGNORE)		DebugPrintA("CONTACT_FLAG_IGNORE, ");
					if (dwContactFlag&CONTACT_FLAG_SHADOW)		DebugPrintA("CONTACT_FLAG_SHADOW, ");
					if (dwContactFlag&CONTACT_FLAG_AUTHORIZED)	DebugPrintA("CONTACT_FLAG_AUTHORIZED, ");
					if (dwContactFlag&CONTACT_FLAG_MULTICHAT)	DebugPrintA("CONTACT_FLAG_MULTICHAT, ");
					if (dwContactFlag&CONTACT_FLAG_UNICODE_NAME)DebugPrintA("CONTACT_FLAG_UNICODE_NAME, ");
					if (dwContactFlag&CONTACT_FLAG_PHONE)		DebugPrintA("CONTACT_FLAG_PHONE, ");
					DebugPrintA(")");

					mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),": server flags: %lu (",dwContactSeverFlags);
					DebugPrintA((LPSTR)szBuff);
					if (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)		DebugPrintA("CONTACT_INTFLAG_NOT_AUTHORIZED, ");
					DebugPrintCRLFA(")");
				#endif//*/

				
				// add/modify contact
				if (dwGroupID!=103)//***deb filtering phone/sms contats
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpsEMail.lpszData,lpsEMail.dwSize,"phone",5)!=CSTR_EQUAL)
				if (dwControlParam>5)// все параметры правильно инициализированны!
				if ((dwContactFlag&(CONTACT_FLAG_GROUP|CONTACT_FLAG_REMOVED))==0)
				{
					hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,TRUE,FALSE,&bAdded);
					if (hContact)
					{
						if (GetContactBasicInfoW(hContact,&dwTemp,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL)==NO_ERROR && dwTemp!=-1)
						{//deb контакт уже в списке, нахуй дубликата!!!!
							dwTemp=dwTemp;
							//MraSendCommand_ModifyContactW(hContact,dwID,CONTACT_FLAG_REMOVED,dwGroupID,lpsEMail.lpszData,lpsEMail.dwSize,mralpsNick.lpszData,mralpsNick.dwSize,mralpsCustomPhones.lpszData,mralpsCustomPhones.dwSize);
							DebugBreak();
						}else{
							dwTemp=GetMiradaStatusFromMraStatus(dwStatus,GetMraXStatusIDFromMraUriStatus(lpsSpecStatusUri.lpszData,lpsSpecStatusUri.dwSize),&dwXStatus);
							if((dwContactFlag&CONTACT_FLAG_UNICODE_NAME)) mralpsNick.dwSize/=sizeof(WCHAR);

							if (bAdded)
							{// update user info
								SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_FLAG|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_NICK|SCBIF_PHONES),dwID,dwGroupID,dwContactFlag,dwContactSeverFlags,dwTemp,NULL,0,mralpsNick.lpwszData,mralpsNick.dwSize,mralpsCustomPhones.lpszData,mralpsCustomPhones.dwSize);

								// request user info from server
								MraUpdateContactInfo(hContact);
							}else{//****deb - check group ID param
								SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),dwID,dwGroupID,dwContactFlag,dwContactSeverFlags,dwTemp,NULL,0,mralpsNick.lpwszData,mralpsNick.dwSize,mralpsCustomPhones.lpszData,mralpsCustomPhones.dwSize);
								if (mralpsNick.dwSize==0)
								{// прописываем ник в листе на сервере
									lstrcpynW(szBuff,GetContactNameW(hContact),SIZEOF(szBuff));
									mralpsNick.lpwszData=szBuff;
									mralpsNick.dwSize=lstrlenW(mralpsNick.lpwszData);

									MraSendCommand_ModifyContactW(hContact,dwID,dwContactFlag,dwGroupID,lpsEMail.lpszData,lpsEMail.dwSize,mralpsNick.lpwszData,mralpsNick.dwSize,mralpsCustomPhones.lpszData,mralpsCustomPhones.dwSize);
								}
							}

							MraContactCapabilitiesSet(hContact,dwFutureFlags);
							DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,(BYTE)dwXStatus);
							DB_Mra_SetLPSStringW(hContact,DBSETTING_XSTATUSNAME,&lpsStatusTitle);
							DB_Mra_SetLPSStringW(hContact,DBSETTING_XSTATUSMSG,&lpsStatusDesc);
							DB_Mra_SetDword(hContact,DBSETTING_BLOGSTATUSTIME,dwBlogStatusTime);
							DB_Mra_WriteContactSettingBlob(hContact,DBSETTING_BLOGSTATUSID,&dwBlogStatusID.QuadPart,sizeof(DWORDLONG));
							DB_Mra_SetLPSStringW(hContact,DBSETTING_BLOGSTATUS,&lpsBlogStatus);
							DB_Mra_SetLPSStringW(hContact,DBSETTING_BLOGSTATUSMUSIC,&lpsBlogStatusMusic);
							if (IsXStatusValid(dwXStatus)) SetExtraIcons(hContact);
							
							if (dwTemp!=ID_STATUS_OFFLINE)// пишем клиента только если юзер не отключён, иначе не затираем старое
							{
								if (lpsUserAgentFormated.dwSize)
								{// есть чё писать
									if (DB_Mra_GetByte(NULL,"MirVerRaw",MRA_DEFAULT_MIRVER_RAW)==FALSE)
									{
										MraGetVersionStringFromFormated(lpsUserAgentFormated.lpszData,lpsUserAgentFormated.dwSize,(LPSTR)szBuff,SIZEOF(szBuff),&dwStringSize);
										lpsUserAgentFormated.lpszData=(LPSTR)szBuff;
										lpsUserAgentFormated.dwSize=dwStringSize;
									}
								}else{// хз чё за клиент
									lpsUserAgentFormated.lpszData=MIRVER_UNKNOWN;
									lpsUserAgentFormated.dwSize=(sizeof(MIRVER_UNKNOWN)-1);
								}
								DB_Mra_SetLPSStringA(hContact,"MirVer",&lpsUserAgentFormated);
							}

							if (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
							if (DB_Mra_GetByte(NULL,"AutoAuthRequestOnLogon",MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON)) CallProtoService(PROTOCOL_NAMEA,MRA_REQ_AUTH,(WPARAM)hContact,0);
						}
					}
				}
				dwID++;
			}// end while (processing contacts)

			// post processing contact list
			{
				CHAR szEMail[MAX_EMAIL_LEN],szPhones[MAX_EMAIL_LEN];
				WCHAR wszAuthMessage[MAX_PATH],wszNick[MAX_EMAIL_LEN];
				SIZE_T dwEMailSize,dwNickSize,dwPhonesSize,dwAuthMessageSize;

				if (DB_Mra_GetStaticStringW(NULL,"AuthMessage",wszAuthMessage,SIZEOF(wszAuthMessage),&dwAuthMessageSize)==FALSE)
				{// def auth message
					lstrcpynW(wszAuthMessage,TranslateW(MRA_DEFAULT_AUTH_MESSAGE),SIZEOF(wszAuthMessage));
					dwAuthMessageSize=lstrlenW(wszAuthMessage);
				}

				for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
				{
					if (GetContactBasicInfoW(hContact,&dwID,NULL,NULL,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,NULL,0,NULL,NULL,0,NULL)==NO_ERROR)
					if (dwID==-1)
					{
						if (IsEMailChatAgent(szEMail,dwEMailSize))
						{// чат: ещё раз запросим авторизацию, пометим как видимый в списке, постоянный
							DBDeleteContactSetting(hContact,"CList","Hidden");
							DBDeleteContactSetting(hContact,"CList","NotOnList");
							SetExtraIcons(hContact);
							MraSetContactStatus(hContact,ID_STATUS_ONLINE);

							lstrcpynW(szBuff,GetContactNameW(hContact),SIZEOF(szBuff));
							MraSendCommand_AddContactW(hContact,(CONTACT_FLAG_VISIBLE|CONTACT_FLAG_MULTICHAT|CONTACT_FLAG_UNICODE_NAME),-1,szEMail,dwEMailSize,szBuff,lstrlenW(szBuff),NULL,0,NULL,0,0);
						}else{
							if (DBGetContactSettingByte(hContact,"CList","NotOnList",0)==0)
							{// set extra icons and upload contact
								SetExtraIcons(hContact);
								if (DB_Mra_GetByte(NULL,"AutoAddContactsToServer",MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER))
								{//add all contacts to server
									GetContactBasicInfoW(hContact,NULL,&dwGroupID,NULL,NULL,NULL,NULL,0,NULL,wszNick,SIZEOF(wszNick),&dwNickSize,szPhones,SIZEOF(szPhones),&dwPhonesSize);
									MraSendCommand_AddContactW(hContact,(CONTACT_FLAG_VISIBLE|CONTACT_FLAG_UNICODE_NAME),dwGroupID,szEMail,dwEMailSize,wszNick,dwNickSize,szPhones,dwPhonesSize,wszAuthMessage,dwAuthMessageSize,0);
								}
							}
						}
						MraUpdateContactInfo(hContact);
					}
				}
			}
		}else{// контакт лист почемуто не получили
			// всех в offline и id в нестандарт
			for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
			{// функция сама проверяет принадлежность контакта к MRA
				SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),-1,-2,0,0,ID_STATUS_OFFLINE,NULL,0,NULL,0,NULL,0);
				// request user info from server
				MraUpdateContactInfo(hContact);
			}

			if (dwTemp==GET_CONTACTS_ERROR)
			{// найденный контакт-лист некорректен
				ShowFormatedErrorMessage(L"MRIM_CS_CONTACT_LIST2: bad contact list",NO_ERROR);
			}else
			if (dwTemp==GET_CONTACTS_INTERR)
			{// произошла внутренняя ошибка
				ShowFormatedErrorMessage(L"MRIM_CS_CONTACT_LIST2: internal server error",NO_ERROR);
			}else{
				mir_sntprintf(szBuff,SIZEOF(szBuff),TranslateW(L"MRIM_CS_CONTACT_LIST2: unknown server error, code: %lu"),dwTemp);
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,szBuff);
			}
		}
		break;
	case MRIM_CS_SMS_ACK:
		dwTemp=GetUL(&lpbDataCurrent);
		if (MraSendQueueFind(masMraSettings.hSendQueueHandle,pmaHeader->seq,NULL,&hContact,&dwAckType,(LPBYTE*)&lpsString.lpszData,&lpsString.dwSize)==NO_ERROR)
		{
			char szEMail[MAX_EMAIL_LEN];
			LPSTR lpszPhone;
			LPWSTR lpwszMessage;
			SIZE_T dwEMailSize,dwPhoneSize,dwMessageSize;

			if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				dwPhoneSize=(*(DWORD*)lpsString.lpszData);
				dwMessageSize=lpsString.dwSize-(dwPhoneSize+sizeof(DWORD)+2);
				lpszPhone=(lpsString.lpszData+sizeof(DWORD));
				lpwszMessage=(LPWSTR)(lpszPhone+dwPhoneSize+1);

				dwTemp=mir_snprintf((LPSTR)szBuff,SIZEOF(szBuff),"<sms_response><source>Mail.ru</source><deliverable>Yes</deliverable><network>Mail.ru, Russia</network><message_id>%s-1-1955988055-%s</message_id><destination>%s</destination><messages_left>0</messages_left></sms_response>\r\n",szEMail,lpszPhone,lpszPhone);
				ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,dwAckType,ACKRESULT_SENTREQUEST,(HANDLE)pmaHeader->seq,(LPARAM)szBuff,dwTemp);
			}

			MEMFREE(lpsString.lpszData);
			MraSendQueueFree(masMraSettings.hSendQueueHandle,pmaHeader->seq);
		}else{// not found in queue
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,TranslateW(L"MRIM_CS_SMS_ACK: not found in queue"));
		}
		break;
	case MRIM_CS_PROXY:
		{
			DWORD dwIDRequest;
			MRA_LPS lpsAddreses={0};
			MRA_GUID mguidSessionID;
	
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);// LPS to
			dwIDRequest=GetUL(&lpbDataCurrent);// DWORD id_request
			dwAckType=GetUL(&lpbDataCurrent);// DWORD data_type
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);// LPS user_data
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsAddreses);// LPS lps_ip_port
			mguidSessionID=GetGUID(&lpbDataCurrent);// DWORD session_id[4]

			if (dwAckType==MRIM_PROXY_TYPE_FILES)
			{// файлы, on file recv
				if (MraMrimProxySetData(MraFilesQueueItemProxyByID(masMraSettings.hFilesQueueHandle,dwIDRequest),lpsEMail.lpszData,lpsEMail.dwSize,dwIDRequest,dwAckType,lpsString.lpszData,lpsString.dwSize,lpsAddreses.lpszData,lpsAddreses.dwSize,&mguidSessionID)==NO_ERROR)
				{// сессия передачи ещё жива/proxy enabled// set proxy info to file transfer context
					MraFilesQueueStartMrimProxy(masMraSettings.hFilesQueueHandle,dwIDRequest);
				}else{// дохлая сессия/не существующая
					MraSendCommand_ProxyAck(PROXY_STATUS_ERROR,lpsEMail.lpszData,lpsEMail.dwSize,dwIDRequest,dwAckType,lpsString.lpszData,lpsString.dwSize,lpsAddreses.lpszData,lpsAddreses.dwSize,mguidSessionID);
					DebugBreak();
				}
			}
			//DebugBreak();
		}
		break;
	case MRIM_CS_PROXY_ACK:
		{
			DWORD dwIDRequest;
			HANDLE hMraMrimProxyData;
			MRA_LPS lpsAddreses={0};
			MRA_GUID mguidSessionID;
	
			dwTemp=GetUL(&lpbDataCurrent);// DWORD status
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);// LPS to
			dwIDRequest=GetUL(&lpbDataCurrent);// DWORD id_request
			dwAckType=GetUL(&lpbDataCurrent);// DWORD data_type
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);// LPS user_data
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsAddreses);// LPS lps_ip_port
			mguidSessionID=GetGUID(&lpbDataCurrent);// DWORD session_id[4]

			if (dwAckType==MRIM_PROXY_TYPE_FILES)
			{// файлы, on file send
				if ((hMraMrimProxyData=MraFilesQueueItemProxyByID(masMraSettings.hFilesQueueHandle,dwIDRequest)))
				{// сессия передачи ещё жива/proxy enabled
					switch(dwTemp){
					case PROXY_STATUS_DECLINE:
						MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,dwIDRequest,FALSE);
						break;
					case PROXY_STATUS_OK:
						if (MraMrimProxySetData(hMraMrimProxyData,lpsEMail.lpszData,lpsEMail.dwSize,dwIDRequest,dwAckType,lpsString.lpszData,lpsString.dwSize,lpsAddreses.lpszData,lpsAddreses.dwSize,&mguidSessionID)==NO_ERROR)
						{// set proxy info to file transfer context
							MraFilesQueueStartMrimProxy(masMraSettings.hFilesQueueHandle,dwIDRequest);
						}
						break;
					case PROXY_STATUS_ERROR:
						ShowFormatedErrorMessage(L"Proxy File transfer: error",NO_ERROR);
						MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,dwIDRequest,FALSE);
						break;
					case PROXY_STATUS_INCOMPATIBLE_VERS:
						ShowFormatedErrorMessage(L"Proxy File transfer: incompatible versions",NO_ERROR);
						MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,dwIDRequest,FALSE);
						break;
					case PROXY_STATUS_NOHARDWARE:
					case PROXY_STATUS_MIRROR:
					case PROXY_STATUS_CLOSED:
					default:
						DebugBreak();
						break;
					}
				}else{// дохлая сессия/не существующая
					DebugBreak();
				}
			}

			/*if (dwTemp!=2)
			if ((hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,FALSE,TRUE,NULL)))
			if (MraGetContactStatus(hContact)==ID_STATUS_OFFLINE)
			{
				MraSetContactStatus(hContact,ID_STATUS_INVISIBLE);
			}*/

			//DebugBreak();
		}
		break;
	case MRIM_CS_PROXY_HELLO:
		// DWORD[4] Session_id
		DebugBreak();
		break;
	case MRIM_CS_PROXY_HELLO_ACK:
		DebugBreak();
		break;
	case MRIM_CS_NEW_MAIL:
		{
			DWORD dwDate,dwUIDL;

			dwTemp=GetUL(&lpbDataCurrent);// UL unread count
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);// LPS from
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);// LPS subject
			dwDate=GetUL(&lpbDataCurrent);// UL date
			dwUIDL=GetUL(&lpbDataCurrent);// UL uidl

			if (dwTemp > masMraSettings.dwEmailMessagesTotal) masMraSettings.dwEmailMessagesTotal+=(dwTemp-masMraSettings.dwEmailMessagesUnRead);

			dwAckType=masMraSettings.dwEmailMessagesUnRead;// save old value
			masMraSettings.dwEmailMessagesUnRead=dwTemp;// store new value
			if (DB_Mra_GetByte(NULL,"IncrementalNewMailNotify",MRA_DEFAULT_INC_NEW_MAIL_NOTIFY)==0 || dwAckType<dwTemp || dwTemp==0)
			{
				MraUpdateEmailStatus(lpsEMail.lpszData,lpsEMail.dwSize,lpsString.lpszData,lpsString.dwSize,dwDate,dwUIDL);
			}
		}
		break;
	case MRIM_CS_USER_BLOG_STATUS:
		{
			DWORD dwTime,dwFlags;
			MRA_LPS lpsText;
			LPBYTE lpbBuff=NULL;
			DWORDLONG dwBlogStatusID;

			dwFlags=GetUL(&lpbDataCurrent);// UL flags
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsEMail);// LPS user
			dwBlogStatusID=GetUIDL(&lpbDataCurrent);// UINT64 id
			dwTime=GetUL(&lpbDataCurrent);// UL time
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsText);// LPS text (MRIM_BLOG_STATUS_MUSIC: track)
			GetLPS(lpbData,dwDataSize,&lpbDataCurrent,&lpsString);// LPS reply_user_nick

			if ((hContact=MraHContactFromEmail(lpsEMail.lpszData,lpsEMail.dwSize,FALSE,TRUE,NULL)))
			{
				if (dwFlags&MRIM_BLOG_STATUS_MUSIC)
				{
					DB_Mra_SetLPSStringW(hContact,DBSETTING_BLOGSTATUSMUSIC,&lpsText);
				}else{
					DB_Mra_SetDword(hContact,DBSETTING_BLOGSTATUSTIME,dwTime);
					DB_Mra_WriteContactSettingBlob(hContact,DBSETTING_BLOGSTATUSID,&dwBlogStatusID,sizeof(DWORDLONG));
					DB_Mra_SetLPSStringW(hContact,DBSETTING_BLOGSTATUS,&lpsText);
				}
			}
		}
		break;

	case MRIM_CS_UNKNOWN:
		/*{
			BYTE btBuff[8192]={0};
			memmove(btBuff,lpbData,dwDataSize);
			DebugBreak();
		}*/
		break;
	case MRIM_CS_USER_GEO:
		/*{
			BYTE btBuff[8192]={0};
			memmove(btBuff,lpbData,dwDataSize);
			DebugBreak();
		}*/
		break;
	case MRIM_CS_SERVER_SETTINGS:
		/*{
			BYTE btBuff[8192]={0};
			memmove(btBuff,lpbData,dwDataSize);
			DebugBreak();
		}*/
		break;
	default:
#ifdef _DEBUG
		/*{
			HANDLE hFile=CreateFile("C:\\Documents and Settings\\Ivan\\Рабочий стол\\MRIM_CS_CONTACT_LIST2.txt",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

			if (hFile!=INVALID_HANDLE_VALUE)
			{
				WriteFile(hFile,lpbData,dwDataSize,&dwTemp,NULL);
				CloseHandle(hFile);
			}
		}//*/
		{
			BYTE btBuff[8192]={0};
			memmove(btBuff,lpbData,dwDataSize);
			DebugBreak();
		}
#endif
		break;
	}
return(0);
}


DWORD MraRecvCommand_Message(DWORD dwTime,DWORD dwFlags,MRA_LPS *plpsFrom,MRA_LPS *plpsText,MRA_LPS *plpsRFTText,MRA_LPS *plpsMultiChatData)
{// Сообщение
	BOOL bAdded;
	DWORD dwRetErrorCode=NO_ERROR,dwBackColour;
	LPSTR lpszMessageExt=NULL;
	LPWSTR lpwszMessage=NULL;
	SIZE_T dwMessageSize=0,dwMessageExtSize=0;
	CCSDATA ccs={0};
	PROTORECVEVENT pre={0};

	//ccs.wParam=0;
	ccs.lParam=(LPARAM)&pre;
	pre.timestamp=dwTime;

	// check flags and datas
	if (dwFlags&MESSAGE_FLAG_RTF)
	{
		if (plpsRFTText)
		{
			if (plpsRFTText->lpszData==NULL || plpsRFTText->dwSize==0) dwFlags&=~MESSAGE_FLAG_RTF;
		}else{
			dwFlags&=~MESSAGE_FLAG_RTF;
		}
	}

	if (dwFlags&MESSAGE_FLAG_MULTICHAT)
	{
		if (plpsMultiChatData)
		{
			if (plpsMultiChatData->lpszData==NULL || plpsMultiChatData->dwSize==0) dwFlags&=~MESSAGE_FLAG_MULTICHAT;
		}else{
			dwFlags&=~MESSAGE_FLAG_MULTICHAT;
		}
	}


	// pre processing - extracting/decoding
	if (dwFlags&MESSAGE_FLAG_AUTHORIZE)
	{// extract auth message из обычного текста
		SIZE_T dwAuthPartsCount,dwAuthBuffSize=(plpsText->dwSize+32),dwAuthDataSize;
		LPBYTE lpbAuthData,lpbDataCurrent;
		MRA_LPS lpsAuthFrom,lpsAuthMessage;

		lpbAuthData=(LPBYTE)MEMALLOC(dwAuthBuffSize);
		if (lpbAuthData)
		{
			BASE64DecodeFormated(plpsText->lpszData,plpsText->dwSize,lpbAuthData,dwAuthBuffSize,&dwAuthDataSize);

			lpbDataCurrent=lpbAuthData;
			dwAuthPartsCount=GetUL(&lpbDataCurrent);
			if (GetLPS(lpbAuthData,dwAuthDataSize,&lpbDataCurrent,&lpsAuthFrom)==NO_ERROR)
			if (GetLPS(lpbAuthData,dwAuthDataSize,&lpbDataCurrent,&lpsAuthMessage)==NO_ERROR)
			{
				if (dwFlags&MESSAGE_FLAG_v1p16 && (dwFlags&MESSAGE_FLAG_CP1251)==0)
				{// unicode text
					memmove(lpbAuthData,lpsAuthMessage.lpszData,lpsAuthMessage.dwSize);
					lpwszMessage=(LPWSTR)lpbAuthData;
					dwMessageSize=(lpsAuthMessage.dwSize/sizeof(WCHAR));
				}else{// преобразуем в юникод текст только если он в АНСИ и если это не Флэш мультик и будильник тоже не нуждается в этом
					lpwszMessage=(LPWSTR)MEMALLOC(((lpsAuthMessage.dwSize+MAX_PATH)*sizeof(WCHAR)));
					if (lpwszMessage)
					{
						dwMessageSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpsAuthMessage.lpszData,lpsAuthMessage.dwSize,lpwszMessage,(lpsAuthMessage.dwSize+MAX_PATH));
						(*(lpwszMessage+dwMessageSize))=0;
					}else{// не удалось выделить память
						dwRetErrorCode=GetLastError();
					}
				}
			}
			if (lpwszMessage!=(LPWSTR)lpbAuthData) MEMFREE(lpbAuthData);
		}
	}else{
		/*// пупер затычка
		if (dwFlags&MESSAGE_FLAG_v1p16 && dwFlags&MESSAGE_FLAG_CP1251)
		{// и какая же кодировка у текста...мммм...бум гадать!
			DebugPrintW(L"Unknown message encoding: ");
			if (MemoryFindByte(0,plpsText->lpszData,(plpsText->dwSize-1),0))
			{//ооо похоже это юникод!
				dwFlags&=~MESSAGE_FLAG_CP1251;
				DebugPrintCRLFW(plpsText->lpwszData);
			}else{// наверное анси
				dwFlags&=~MESSAGE_FLAG_v1p16;
				DebugPrintCRLFA(plpsText->lpszData);
			}
		}*/

		if (dwFlags&(MESSAGE_FLAG_ALARM|MESSAGE_FLAG_FLASH|MESSAGE_FLAG_v1p16) && (dwFlags&MESSAGE_FLAG_CP1251)==0)
		{// unicode text
			lpwszMessage=plpsText->lpwszData;
			dwMessageSize=(plpsText->dwSize/sizeof(WCHAR));
		}else{// преобразуем в юникод текст только если он в АНСИ и если это не Флэш мультик и будильник тоже не нуждается в этом
			lpwszMessage=(LPWSTR)MEMALLOC(((plpsText->dwSize+MAX_PATH)*sizeof(WCHAR)));
			if (lpwszMessage)
			{
				dwMessageSize=MultiByteToWideChar(MRA_CODE_PAGE,0,plpsText->lpszData,plpsText->dwSize,lpwszMessage,(plpsText->dwSize+MAX_PATH));
				(*(lpwszMessage+dwMessageSize))=0;
			}else{// не удалось выделить память
				dwRetErrorCode=GetLastError();
			}
		}

		if (dwFlags&(MESSAGE_FLAG_CONTACT|MESSAGE_FLAG_NOTIFY|MESSAGE_FLAG_SMS|MESSAGE_SMS_DELIVERY_REPORT|MESSAGE_FLAG_ALARM))
		{
			// ничего не делаем, сообщение не содержит расширенной части или её содержимое игнорируется
		}else{
			if ((dwFlags&MESSAGE_FLAG_RTF) && plpsRFTText) //MESSAGE_FLAG_FLASH there
			if (masMraSettings.lpfnUncompress)// only if uncompress function exist
			if (plpsRFTText->lpszData && plpsRFTText->dwSize)
			{// decode RTF
				SIZE_T dwRTFPartsCount,dwCompressedSize,dwRFTBuffSize=((plpsRFTText->dwSize*16)+8192),dwRTFDataSize;
				LPBYTE lpbRTFData,lpbCompressed,lpbDataCurrent;
				MRA_LPS lpsRTFString,lpsBackColour,lpsString;

				lpbRTFData=(LPBYTE)MEMALLOC(dwRFTBuffSize);
				lpbCompressed=(LPBYTE)MEMALLOC((plpsRFTText->dwSize+32));
				if (lpbRTFData && lpbCompressed)
				{
					BASE64DecodeFormated(plpsRFTText->lpszData,plpsRFTText->dwSize,lpbCompressed,(plpsRFTText->dwSize+32),&dwCompressedSize);
					dwRTFDataSize=dwRFTBuffSize;
					if ((PUNCOMPRESS(masMraSettings.lpfnUncompress))(lpbRTFData,(DWORD*)&dwRTFDataSize,lpbCompressed,dwCompressedSize)==Z_OK)
					{
						lpbDataCurrent=lpbRTFData;
						dwRTFPartsCount=GetUL(&lpbDataCurrent);// колличество частей в некоторых случаях больше 2, тогда нужно игнорировать первый текст, тк там сообщения об ущербности
						if (GetLPS(lpbRTFData,dwRTFDataSize,&lpbDataCurrent,&lpsRTFString)==NO_ERROR)
						if (GetLPS(lpbRTFData,dwRTFDataSize,&lpbDataCurrent,&lpsBackColour)==NO_ERROR)
						{
							dwBackColour=(*(DWORD*)lpsBackColour.lpszData);
							if (dwFlags&MESSAGE_FLAG_FLASH)
							{// Флэш мультик в обычный текст// затирем lpwszMessage=plpsText->lpwszData, тк там затычка с текстом об ущербности
								if (dwRTFPartsCount==3)
								{// только анси текст с мультиком
									DebugBreak();// наверное это уже не должно приходить, ну разве что от совсем старых клиентов, 2008
									GetLPS(lpbRTFData,dwRTFDataSize,&lpbDataCurrent,&lpsString);
									lpwszMessage=(LPWSTR)MEMALLOC(((lpsString.dwSize+MAX_PATH)*sizeof(WCHAR)));
									if (lpwszMessage)
									{
										memmove(lpwszMessage,lpsString.lpszData,lpsString.dwSize);
										dwMessageSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpsString.lpszData,lpsString.dwSize,lpwszMessage,(lpsString.dwSize+MAX_PATH));
										(*(lpwszMessage+dwMessageSize))=0;
									}else{// не удалось выделить память
										dwRetErrorCode=GetLastError();
									}
								}else
								if (dwRTFPartsCount==4)
								{
									GetLPS(lpbRTFData,dwRTFDataSize,&lpbDataCurrent,&lpsString);
									GetLPS(lpbRTFData,dwRTFDataSize,&lpbDataCurrent,&lpsString);
									lpwszMessage=(LPWSTR)MEMALLOC(lpsString.dwSize);
									if (lpwszMessage)
									{
										memmove(lpwszMessage,lpsString.lpszData,lpsString.dwSize);
										dwMessageSize=lpsString.dwSize;
									}else{// не удалось выделить память
										dwRetErrorCode=GetLastError();
									}
								}else{
									DebugBreak();
								}
							}else{// РТФ текст 
								if (dwRTFPartsCount>2)
								{
									GetLPS(lpbRTFData,dwRTFDataSize,&lpbDataCurrent,&lpsString);
									DebugBreak();
								}

								lpszMessageExt=(LPSTR)MEMALLOC(lpsRTFString.dwSize);
								if (lpszMessageExt)
								{
									memmove(lpszMessageExt,lpsRTFString.lpszData,lpsRTFString.dwSize);
									dwMessageExtSize=lpsRTFString.dwSize;
								}else{// не удалось выделить память
									//dwRetErrorCode=GetLastError(); // не смертельно!
									DebugBreak();
								}
							}
						}
					}else{
						MEMFREE(lpszMessageExt);
						dwMessageExtSize=0;
						DebugBreak();
					}
				}
				MEMFREE(lpbCompressed);
				MEMFREE(lpbRTFData);
			}
		}
	}


	// processing
	if (dwRetErrorCode==NO_ERROR)
	if (MraAntiSpamReceivedMessageW(plpsFrom->lpszData,plpsFrom->dwSize,dwFlags,lpwszMessage,dwMessageSize)==MESSAGE_NOT_SPAM)
	{
		if (dwFlags&(MESSAGE_FLAG_SMS|MESSAGE_SMS_DELIVERY_REPORT))
		{// SMS //if (IsPhone(plpsFrom->lpszData,plpsFrom->dwSize))
			char szPhone[MAX_EMAIL_LEN],szEMail[MAX_EMAIL_LEN],szTime[MAX_PATH];
			LPSTR lpszMessageUTF,lpszBuff;
			LPWSTR lpwszMessageXMLEncoded;
			SIZE_T dwBuffLen,dwMessageXMLEncodedSize;
			INTERNET_TIME itTime;

			dwBuffLen=((dwMessageSize+MAX_PATH)*6);
			lpszMessageUTF=(LPSTR)MEMALLOC(dwBuffLen);
			lpwszMessageXMLEncoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
			if (lpszMessageUTF && lpwszMessageXMLEncoded)
			{
				InternetTimeGetCurrentTime(&itTime);
				InternetTimeGetString(&itTime,szTime,SIZEOF(szTime),NULL);
				CopyNumber(szPhone,plpsFrom->lpszData,plpsFrom->dwSize);
				DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),NULL);

				EncodeXML(lpwszMessage,dwMessageSize,lpwszMessageXMLEncoded,dwBuffLen,&dwMessageXMLEncodedSize);
				WideCharToMultiByte(CP_UTF8,0,lpwszMessageXMLEncoded,dwMessageXMLEncodedSize,lpszMessageUTF,dwBuffLen,NULL,NULL);
				lpszBuff=(LPSTR)lpwszMessageXMLEncoded;

				if (dwFlags&MESSAGE_SMS_DELIVERY_REPORT)
				{// отчёт о неудаче доставки смс
					dwBuffLen=mir_snprintf(lpszBuff,(dwBuffLen*sizeof(WCHAR)),"<sms_delivery_receipt><message_id>%s-1-1955988055-%s</message_id><destination>%s</destination><delivered>No</delivered><submition_time>%s</submition_time><error_code>0</error_code><error><id>15</id><params><param>%s</param></params></error></sms_delivery_receipt>",szEMail,szPhone,szPhone,szTime,lpszMessageUTF);
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,ICQACKTYPE_SMS,ACKRESULT_FAILED,(HANDLE)0,(LPARAM)lpszBuff,dwBuffLen);
				}else{// новое смс!!!
					dwBuffLen=mir_snprintf(lpszBuff,(dwBuffLen*sizeof(WCHAR)),"<sms_message><source>Mail.ru</source><destination_UIN>%s</destination_UIN><sender>%s</sender><senders_network>Mail.ru</senders_network><text>%s</text><time>%s</time></sms_message>",szEMail,szPhone,lpszMessageUTF,szTime);
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,ICQACKTYPE_SMS,ACKRESULT_SUCCESS,(HANDLE)0,(LPARAM)lpszBuff,dwBuffLen);
				}
			}else{// не удалось выделить память
				dwRetErrorCode=GetLastError();
			}
			MEMFREE(lpwszMessageXMLEncoded);
			MEMFREE(lpszMessageUTF);
		}else{
			ccs.hContact=MraHContactFromEmail(plpsFrom->lpszData,plpsFrom->dwSize,TRUE,TRUE,&bAdded);
			if (bAdded) MraUpdateContactInfo(ccs.hContact);

			if (dwFlags&MESSAGE_FLAG_NOTIFY)
			{// user typing
				CallService(MS_PROTO_CONTACTISTYPING,(WPARAM)ccs.hContact,MAILRU_CONTACTISTYPING_TIMEOUT);
			}else{// text/contact/auth // typing OFF
				CallService(MS_PROTO_CONTACTISTYPING,(WPARAM)ccs.hContact,PROTOTYPE_CONTACTTYPING_OFF);

				if (dwFlags&MESSAGE_FLAG_MULTICHAT)
				{// MULTICHAT
					LPBYTE lpbMultiChatData,lpbDataCurrent;
					SIZE_T dwMultiChatDataSize;
					DWORD dwMultiChatEventType;
					MRA_LPS lpsEMailInMultiChat,lpsString,lpsMultichatName;

					lpbMultiChatData=(LPBYTE)plpsMultiChatData->lpszData;
					dwMultiChatDataSize=plpsMultiChatData->dwSize;
					lpbDataCurrent=lpbMultiChatData;

					dwMultiChatEventType=GetUL(&lpbDataCurrent);// type
					GetLPS(lpbMultiChatData,dwMultiChatDataSize,&lpbDataCurrent,&lpsMultichatName);// multichat_name
					GetLPS(lpbMultiChatData,dwMultiChatDataSize,&lpbDataCurrent,&lpsEMailInMultiChat);

					switch(dwMultiChatEventType){
					case MULTICHAT_MESSAGE:
						MraChatSessionMessageAdd(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize,lpwszMessage,dwMessageSize,dwTime);// LPS sender
						break;
					case MULTICHAT_ADD_MEMBERS:
						MraChatSessionMembersAdd(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize,dwTime);// LPS sender
						GetLPS(lpbMultiChatData,dwMultiChatDataSize,&lpbDataCurrent,&lpsString);// CLPS members
						MraChatSessionSetIviter(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize);
					case MULTICHAT_MEMBERS:
						{
							LPBYTE lpbMultiChatDataLocal,lpbDataCurrentLocal;
							SIZE_T i,dwMultiChatMembersCount,dwMultiChatDataLocalSize;

							if (dwMultiChatEventType==MULTICHAT_MEMBERS) lpsString=lpsEMailInMultiChat;

							lpbMultiChatDataLocal=(LPBYTE)lpsString.lpszData;
							dwMultiChatDataLocalSize=lpsString.dwSize;
							lpbDataCurrentLocal=lpbMultiChatDataLocal;

							dwMultiChatMembersCount=GetUL(&lpbDataCurrentLocal);// count
							for (i=0;i<dwMultiChatMembersCount;i++)
							{
								if (GetLPS(lpbMultiChatDataLocal,dwMultiChatDataLocalSize,&lpbDataCurrentLocal,&lpsString)==NO_ERROR)
								{// CLPS members
									MraChatSessionJoinUser(ccs.hContact,lpsString.lpszData,lpsString.dwSize,((dwMultiChatEventType==MULTICHAT_MEMBERS)? 0:dwTime));
								}
							}

							if (dwMultiChatEventType==MULTICHAT_MEMBERS)
							{
								GetLPS(lpbMultiChatData,dwMultiChatDataSize,&lpbDataCurrent,&lpsEMailInMultiChat);// [ LPS owner ]
								MraChatSessionSetOwner(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize);
							}
						}
						break;
					case MULTICHAT_ATTACHED:
						MraChatSessionJoinUser(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize,dwTime);// LPS member
						break;
					case MULTICHAT_DETACHED:
						MraChatSessionLeftUser(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize,dwTime);// LPS member
						break;
					case MULTICHAT_INVITE:
						MraChatSessionInvite(ccs.hContact,lpsEMailInMultiChat.lpszData,lpsEMailInMultiChat.dwSize,dwTime);// LPS sender
						MraSendCommand_AddContactW(ccs.hContact,(CONTACT_FLAG_VISIBLE|CONTACT_FLAG_MULTICHAT|CONTACT_FLAG_UNICODE_NAME),-1,plpsFrom->lpszData,plpsFrom->dwSize,lpsMultichatName.lpwszData,(lpsMultichatName.dwSize/sizeof(WCHAR)),NULL,0,NULL,0,0);
						break;
					default:
						DebugBreak();
						break;
					}
				}else
				if (dwFlags&MESSAGE_FLAG_AUTHORIZE)
				{// auth request
					BYTE btBuff[BUFF_SIZE_BLOB];
					BOOL bAutoGrandAuth=FALSE;

					if (IsEMailChatAgent(plpsFrom->lpszData,plpsFrom->dwSize))
					{
						bAutoGrandAuth=FALSE;
					}else{
						if (DBGetContactSettingByte(ccs.hContact,"CList","NotOnList",0))
						{// временный контакт
							if (DB_Mra_GetByte(NULL,"AutoAuthGrandNewUsers",MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS)) bAutoGrandAuth=TRUE;
						}else{// постоянный контакт
							if (DB_Mra_GetByte(NULL,"AutoAuthGrandUsersInCList",MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST)) bAutoGrandAuth=TRUE;
						}
					}

					if (bAdded) DBWriteContactSettingByte(ccs.hContact,"CList","Hidden",1);
					if (bAutoGrandAuth)
					{// auto grand auth
						DBEVENTINFO dbei={0};

						dbei.cbSize=sizeof(dbei);
						dbei.szModule=PROTOCOL_NAMEA;
						dbei.timestamp=_time32(NULL);
						dbei.flags=DBEF_READ;
						dbei.eventType=EVENTTYPE_AUTHREQUEST;
						dbei.pBlob=(PBYTE)btBuff;

						CreateBlobFromContact(ccs.hContact,lpwszMessage,dwMessageSize,btBuff,SIZEOF(btBuff),(SIZE_T*)&dbei.cbBlob);
						CallService(MS_DB_EVENT_ADD,(WPARAM)NULL,(LPARAM)&dbei);
						MraSendCommand_Authorize(plpsFrom->lpszData,plpsFrom->dwSize);
					}else{
						ccs.szProtoService=PSR_AUTH;
						pre.szMessage=(LPSTR)btBuff;
						CreateBlobFromContact(ccs.hContact,lpwszMessage,dwMessageSize,btBuff,SIZEOF(btBuff),(SIZE_T*)&pre.lParam);
						CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
					}
				}else{
					DBDeleteContactSetting(ccs.hContact,"CList","Hidden");

					if (dwFlags&MESSAGE_FLAG_CONTACT)
					{// contacts received
						LPBYTE lpbBuffer,lpbBufferCurPos;

						lpbBuffer=(LPBYTE)MEMALLOC((dwMessageSize+MAX_PATH));
						if (lpbBuffer)
						{
							ccs.szProtoService=PSR_CONTACTS;
							pre.flags=0;
							pre.szMessage=(LPSTR)lpbBuffer;
							pre.lParam=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszMessage,dwMessageSize,(LPSTR)lpbBuffer,(dwMessageSize+MAX_PATH),NULL,NULL);
							
							lpbBufferCurPos=lpbBuffer;
							while(TRUE)
							{// цикл замены ; на 0
								lpbBufferCurPos=(LPBYTE)MemoryFindByte((lpbBufferCurPos-lpbBuffer),lpbBuffer,pre.lParam,';');
								if (lpbBufferCurPos)
								{// founded
									(*lpbBufferCurPos)=0;
									lpbBufferCurPos++;
								}else{
									break;
								}
							}
							CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
							MEMFREE(lpbBuffer);
						}else{// не удалось выделить память
							dwRetErrorCode=GetLastError();
						}
					}else
					if (dwFlags&MESSAGE_FLAG_ALARM)
					{// alarm
						if (masMraSettings.heNudgeReceived)
						{
							NotifyEventHooks(masMraSettings.heNudgeReceived,(WPARAM)ccs.hContact,NULL);
						}else{
							pre.flags=0;
							pre.szMessage=(LPSTR)TranslateTS(MRA_ALARM_MESSAGE);
							//pre.lParam=lstrlenA(pre.szMessage);
							ccs.szProtoService=PSR_MESSAGE;
							CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
						}
					}else{// standart message// flash animation
						if ((dwFlags&MESSAGE_FLAG_RTF) && (dwFlags&MESSAGE_FLAG_FLASH)==0 && lpszMessageExt && dwMessageExtSize && DB_Mra_GetByte(NULL,"RTFReceiveEnable",MRA_DEFAULT_RTF_RECEIVE_ENABLE))
						{// пишем в ANSI, всё равно RTF
							pre.flags=0;
							pre.szMessage=lpszMessageExt;
							//pre.lParam=dwMessageExtSize;
							ccs.szProtoService=PSR_MESSAGE;
							CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
						}else{
							LPSTR lpszMessageUTF;// some plugins can change pre.szMessage pointer and we failed to free it

							lpszMessageUTF=(LPSTR)MEMALLOC(((dwMessageSize+MAX_PATH)*sizeof(WCHAR)));
							if (lpszMessageUTF)
							{
								pre.szMessage=lpszMessageUTF;
								pre.flags=PREF_UTF;
								//pre.lParam=
								WideCharToMultiByte(CP_UTF8,0,lpwszMessage,dwMessageSize,lpszMessageUTF,((dwMessageSize+MAX_PATH)*sizeof(WCHAR)),NULL,NULL);
								ccs.szProtoService=PSR_MESSAGE;
								CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
								MEMFREE(lpszMessageUTF);
							}else{// не удалось выделить память
								dwRetErrorCode=GetLastError();
							}
						}
						if (dwFlags&MESSAGE_FLAG_SYSTEM)
						{
							MraPopupShowW(ccs.hContact,MRA_POPUP_TYPE_INFORMATION,0,TranslateW(L"Mail.ru System notify"),(LPWSTR)pre.szMessage);
						}
					}
				}
			}
		}
	}else{// spam blocked
		if (DB_Mra_GetByte(NULL,"AntiSpamSendSpamReportToSrv",MRA_ANTISPAM_DEFAULT_SEND_SPAM_REPORT_TO_SERVER))
		{// рапортуем о спаме
			//MraSendCommand_MessageAskW(1,(dwFlags|MESSAGE_FLAG_SPAMF_SPAM),plpsFrom->lpszData,plpsFrom->dwSize,plpsText->lpwszData,plpsText->dwSize,plpsRFTText->lpszData,plpsRFTText->dwSize);
		}
		dwRetErrorCode=ERROR_ACCESS_DENIED;
	}

	if (lpwszMessage!=plpsText->lpwszData && lpwszMessage!=(LPWSTR)lpszMessageExt) MEMFREE(lpwszMessage);
	MEMFREE(lpszMessageExt);

return(dwRetErrorCode);
}





DWORD GetMraXStatusIDFromMraUriStatus(LPSTR lpszStatusUri,SIZE_T dwStatusUriSize)
{
	DWORD dwRet=MRA_XSTATUS_UNKNOWN;

	if (lpszStatusUri)
	{
		for(SIZE_T i=0;lpcszStatusUri[i];i++)
		{
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpcszStatusUri[i],-1,lpszStatusUri,dwStatusUriSize)==CSTR_EQUAL)
			{
				dwRet=i;
				break;
			}
		}
	}
return(dwRet);
}


DWORD GetMraStatusFromMiradaStatus(DWORD dwMirandaStatus,DWORD dwXStatusMir,DWORD *pdwXStatusMra)
{
	DWORD dwRet;

	if (IsXStatusValid(dwXStatusMir))
	{
		if (pdwXStatusMra) (*pdwXStatusMra)=(dwXStatusMir+MRA_XSTATUS_INDEX_OFFSET-1);
		dwRet=STATUS_USER_DEFINED;
	}else{
		switch(dwMirandaStatus){
		case ID_STATUS_OFFLINE:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_OFFLINE;
			dwRet=STATUS_OFFLINE;
			break;
		case ID_STATUS_ONLINE:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_ONLINE;
			dwRet=STATUS_ONLINE;
			break;
		case ID_STATUS_AWAY:
		case ID_STATUS_NA:
		case ID_STATUS_ONTHEPHONE:
		case ID_STATUS_OUTTOLUNCH:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_AWAY;
			dwRet=STATUS_AWAY;
			break;
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_DND;
			dwRet=STATUS_USER_DEFINED;
			break;
		case ID_STATUS_FREECHAT:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_CHAT;
			dwRet=STATUS_USER_DEFINED;
			break;
		case ID_STATUS_INVISIBLE:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_INVISIBLE;
			dwRet=(STATUS_ONLINE|STATUS_FLAG_INVISIBLE);
			break;
		default:
			if (pdwXStatusMra) (*pdwXStatusMra)=MRA_XSTATUS_OFFLINE;
			dwRet=STATUS_OFFLINE;
			break;
		}
	}
return(dwRet);
}


DWORD GetMiradaStatusFromMraStatus(DWORD dwMraStatus,DWORD dwXStatusMra,DWORD *pdwXStatusMir)
{
	DWORD dwRet;

	if (pdwXStatusMir) (*pdwXStatusMir)=0;

	switch(dwMraStatus){
	case STATUS_OFFLINE:
		dwRet=ID_STATUS_OFFLINE;
		break;
	case STATUS_ONLINE:
		dwRet=ID_STATUS_ONLINE;
		break;
	case STATUS_AWAY:
		dwRet=ID_STATUS_AWAY;
		break;
	case STATUS_UNDETERMINATED:
		dwRet=ID_STATUS_OFFLINE;
		break;
	case STATUS_USER_DEFINED:
		switch(dwXStatusMra){
		//case MRA_XSTATUS_OFFLINE:	dwRet=ID_STATUS_OFFLINE;	break;
		//case MRA_XSTATUS_ONLINE:	dwRet=ID_STATUS_ONLINE;		break;
		//case MRA_XSTATUS_AWAY:		dwRet=ID_STATUS_AWAY;		break;
		//case MRA_XSTATUS_INVISIBLE:	dwRet=ID_STATUS_INVISIBLE;	break;
		case MRA_XSTATUS_DND:		dwRet=ID_STATUS_DND;		break;
		case MRA_XSTATUS_CHAT:		dwRet=ID_STATUS_FREECHAT;	break;
		case MRA_XSTATUS_UNKNOWN:
			if (pdwXStatusMir) (*pdwXStatusMir)=MRA_MIR_XSTATUS_UNKNOWN;
			dwRet=ID_STATUS_ONLINE;
			break;
		default:
			if (pdwXStatusMir) (*pdwXStatusMir)=(dwXStatusMra-MRA_XSTATUS_INDEX_OFFSET+1);
			dwRet=ID_STATUS_ONLINE;
			break;
		}
		break;
	default:// STATUS_FLAG_INVISIBLE
		if (dwMraStatus&STATUS_FLAG_INVISIBLE)
		{
			dwRet=ID_STATUS_INVISIBLE;
		}else{
			dwRet=ID_STATUS_OFFLINE;
			DebugBreak();
		}
		break;
	}
return(dwRet);
}


DWORD GetUL(LPBYTE *plpData)
{
	DWORD dwRet=(*(DWORD*)(*plpData));
	(*plpData)+=sizeof(DWORD);
return(dwRet);
}


DWORDLONG GetUIDL(LPBYTE *plpData)
{
	DWORDLONG dwRet=(*(DWORDLONG*)(*plpData));
	(*plpData)+=sizeof(DWORDLONG);
return(dwRet);
}


MRA_GUID GetGUID(LPBYTE *plpData)
{
	MRA_GUID guidRet=(*(MRA_GUID*)(*plpData));
	(*plpData)+=sizeof(MRA_GUID);
return(guidRet);
}


DWORD GetLPS(LPBYTE lpbData,DWORD dwDataSize,LPBYTE *plpCurrentData,MRA_LPS *plpsString)
{
	DWORD dwRetErrorCode;
	LPBYTE lpbDataEnd=(lpbData+dwDataSize);

	if (lpbDataEnd>=((*plpCurrentData)+sizeof(DWORD)))
	{// хотябы длинна данных есть
		if (lpbDataEnd>=((*plpCurrentData)+sizeof(DWORD)+(*(DWORD*)(*plpCurrentData))))
		{// все длинна данных в буфере равна или меньше размера буфера
			plpsString->dwSize=(*(DWORD*)(*plpCurrentData));
			plpsString->lpszData=(LPSTR)((*plpCurrentData)+sizeof(DWORD));
			(*plpCurrentData)+=(sizeof(DWORD)+plpsString->dwSize);
			dwRetErrorCode=NO_ERROR;
		}else{
			plpsString->dwSize=0;
			plpsString->lpszData=NULL;
			dwRetErrorCode=ERROR_INVALID_USER_BUFFER;
		}
	}else{
		plpsString->dwSize=0;
		plpsString->lpszData=NULL;
		dwRetErrorCode=ERROR_INVALID_USER_BUFFER;
	}
return(dwRetErrorCode);
}


