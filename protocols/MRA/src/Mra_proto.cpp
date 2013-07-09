#include "Mra.h"
#include "MraOfflineMsg.h"
#include "MraRTFMsg.h"
#include "MraPlaces.h"

DWORD     GetUL(LPBYTE *plpData);
DWORDLONG GetUIDL(LPBYTE *plpData);
MRA_GUID  GetGUID(LPBYTE *plpData);
DWORD     GetLPS(LPBYTE lpbData, DWORD dwDataSize, LPBYTE *plpCurrentData, MRA_LPS *plpsString);

DWORD CMraProto::StartConnect()
{
	if ( !InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwGlobalPluginRunning, 0))
		return ERROR_OPERATION_ABORTED;

	// поток ещё/уже не работал, поставили статус что работает и запускаем
	if (InterlockedCompareExchange((volatile LONG*)&dwThreadWorkerRunning, TRUE, FALSE) == FALSE) {
		char szPass[MAX_PATH];
		size_t dwEMailSize = 0, dwPasswordSize = 0;

		mraGetStaticStringA(NULL, "e-mail", NULL, 0, &dwEMailSize);

		if (dwEMailSize > 5 && GetPassDB(szPass, sizeof(szPass), &dwPasswordSize)) {
			InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
			hThreadWorker = ForkThreadEx(&CMraProto::MraThreadProc, NULL, 0);
			if (hThreadWorker == NULL) {
				DWORD dwRetErrorCode = GetLastError();
				InterlockedExchange((volatile LONG*)&dwThreadWorkerRunning, FALSE);
				SetStatus(ID_STATUS_OFFLINE);
				return dwRetErrorCode;
			}
		}
		else {
			MraThreadClean();
			if (dwEMailSize <= 5)
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING, 0, TranslateW(L"Please, setup e-mail in options"));
			else
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING, 0, TranslateW(L"Please, setup password in options"));
		}
		SecureZeroMemory(szPass, sizeof(szPass));
	}

	return 0;
}

void CMraProto::MraThreadProc(LPVOID lpParameter)
{
	DWORD dwRetErrorCode = NO_ERROR;

	BOOL bConnected = FALSE;
	char szHost[MAX_PATH];
	DWORD dwConnectReTryCount, dwCurConnectReTryCount;
	NETLIBOPENCONNECTION nloc = {0};

	SleepEx(100, FALSE);// to prevent high CPU load by some status plugins like allwaysonline

	dwConnectReTryCount = getDword(NULL, "ConnectReTryCountMRIM", MRA_DEFAULT_CONN_RETRY_COUNT_MRIM);

	nloc.cbSize = sizeof(nloc);
	nloc.flags = NLOCF_V2;
	nloc.szHost = szHost;
	nloc.timeout = getDword(NULL, "TimeOutConnectMRIM", MRA_DEFAULT_TIMEOUT_CONN_MRIM);
	if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout = MRA_TIMEOUT_CONN_МАХ;

	InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
	if (MraGetNLBData((LPSTR)nloc.szHost, MAX_PATH, &nloc.wPort) == NO_ERROR) {
		dwCurConnectReTryCount = dwConnectReTryCount;
		do {
			InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
			hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&nloc);
		}
			while (--dwCurConnectReTryCount && hConnection == NULL);

		if (hConnection)
			bConnected = TRUE;
	}

	if (bConnected == FALSE)
	if (getByte(NULL, "NLBFailDirectConnect", MRA_DEFAULT_NLB_FAIL_DIRECT_CONNECT)) {
		if (IsHTTPSProxyUsed(hNetlibUser))
			nloc.wPort = MRA_SERVER_PORT_HTTPS;
		else {
			nloc.wPort = getWord(NULL, "ServerPort", MRA_DEFAULT_SERVER_PORT);
			if (nloc.wPort == MRA_SERVER_PORT_STANDART_NLB) nloc.wPort = MRA_SERVER_PORT_STANDART;
		}

		for (DWORD i = 1;(i<MRA_MAX_MRIM_SERVER && m_iStatus != ID_STATUS_OFFLINE); i++) {
			mir_snprintf(szHost, SIZEOF(szHost), "mrim%lu.mail.ru", i);

			dwCurConnectReTryCount = dwConnectReTryCount;
			do {
				InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
				hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&nloc);
			}
				while (--dwCurConnectReTryCount && hConnection == NULL);

			if (hConnection) {
				bConnected = TRUE;
				break;
			}
		}
	}

	if (bConnected && m_iStatus != ID_STATUS_OFFLINE)
		MraNetworkDispatcher();
	else {
		if (bConnected == FALSE) {
			ShowFormattedErrorMessage(L"Can't connect to MRIM server, error", GetLastError());
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
		}
	}

	MraThreadClean();
}

void CMraProto::MraThreadClean()
{
	MraMPopSessionQueueFlush(hMPopSessionQueue);
	Netlib_CloseHandle(hConnection);// called twice, if user set offline, its normal
	hConnection = NULL;
	dwCMDNum = 0;

	SleepEx(100, FALSE);// to prevent high CPU load by some status plugins like allwaysonline

	if (hThreadWorker) {
		CloseHandle(hThreadWorker);
		hThreadWorker = NULL;
	}
	InterlockedExchange((volatile LONG*)&dwThreadWorkerRunning, FALSE);
	SetStatus(ID_STATUS_OFFLINE);
}

DWORD CMraProto::MraGetNLBData(LPSTR lpszHost, size_t dwHostBuffSize, WORD *pwPort)
{
	DWORD dwRetErrorCode;

	BOOL bContinue = TRUE;
	BYTE btBuff[MAX_PATH];
	DWORD dwConnectReTryCount, dwCurConnectReTryCount;
	LPSTR lpszPort;
	size_t dwBytesReceived, dwRcvBuffSizeUsed = 0;
	NETLIBSELECT nls = {0};
	NETLIBOPENCONNECTION nloc = {0};

	dwConnectReTryCount = getDword(NULL, "ConnectReTryCountNLB", MRA_DEFAULT_CONN_RETRY_COUNT_NLB);

	nloc.cbSize = sizeof(nloc);
	nloc.flags = NLOCF_V2;
	if (mraGetStaticStringA(NULL, "Server", (LPSTR)btBuff, SIZEOF(btBuff), NULL))
		nloc.szHost = (LPSTR)btBuff;
	else
		nloc.szHost = MRA_DEFAULT_SERVER;

	if ( IsHTTPSProxyUsed(hNetlibUser))
		nloc.wPort = MRA_SERVER_PORT_HTTPS;
	else
		nloc.wPort = getWord(NULL, "ServerPort", MRA_DEFAULT_SERVER_PORT);

	nloc.timeout = getDword(NULL, "TimeOutConnectNLB", MRA_DEFAULT_TIMEOUT_CONN_NLB);
	if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout = MRA_TIMEOUT_CONN_МАХ;

	dwCurConnectReTryCount = dwConnectReTryCount;
	do {
		InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
		nls.hReadConns[0] = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&nloc);
	}
		while (--dwCurConnectReTryCount && nls.hReadConns[0] == NULL);

	if (nls.hReadConns[0]) {
		nls.cbSize = sizeof(nls);
		nls.dwTimeout = 1000 * getDword(NULL, "TimeOutReceiveNLB", MRA_DEFAULT_TIMEOUT_RECV_NLB);
		InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());

		while (m_iStatus != ID_STATUS_OFFLINE && bContinue) {
			switch (CallService(MS_NETLIB_SELECT, 0, (LPARAM)&nls)) {
			case SOCKET_ERROR:
			case 0:// Time out
				bContinue = FALSE;
				break;
			case 1:
				dwBytesReceived = Netlib_Recv(nls.hReadConns[0], (LPSTR)(btBuff+dwRcvBuffSizeUsed), (SIZEOF(btBuff)-dwRcvBuffSizeUsed), 0);
				if (dwBytesReceived && dwBytesReceived != SOCKET_ERROR)
					dwRcvBuffSizeUsed += dwBytesReceived;
				else
					bContinue = FALSE;
				break;
			}
			InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
		}
		Netlib_CloseHandle(nls.hReadConns[0]);

		if (dwRcvBuffSizeUsed) {
			lpszPort = (LPSTR)MemoryFindByte(0, btBuff, dwRcvBuffSizeUsed, ':');
			if (lpszPort) {
				(*lpszPort) = 0;
				lpszPort++;

				lstrcpynA(lpszHost, (LPSTR)btBuff, dwHostBuffSize);
				if (pwPort) (*pwPort) = (WORD)StrToUNum32(lpszPort, (dwRcvBuffSizeUsed-(lpszPort-(LPSTR)btBuff)));
				dwRetErrorCode = NO_ERROR;
			}
			else {
				dwRetErrorCode = ERROR_INVALID_USER_BUFFER;
				ShowFormattedErrorMessage(L"NLB data corrupted", NO_ERROR);
			}
		}
		else {
			dwRetErrorCode = GetLastError();
			ShowFormattedErrorMessage(L"Can't get data for NLB, error", dwRetErrorCode);
		}
	}
	else {
		dwRetErrorCode = GetLastError();
		ShowFormattedErrorMessage(L"Can't connect to NLB server, error", dwRetErrorCode);
	}

	return dwRetErrorCode;
}

DWORD CMraProto::MraNetworkDispatcher()
{
	DWORD dwRetErrorCode = NO_ERROR;

	BOOL bContinue = TRUE;
	DWORD dwSelectRet, dwBytesReceived, dwDataCurrentBuffSize, dwDataCurrentBuffSizeUsed, dwPingPeriod = MAXDWORD, dwNextPingSendTickTime = MAXDWORD;
	size_t dwRcvBuffSize = BUFF_SIZE_RCV, dwRcvBuffSizeUsed = 0, dwDataCurrentBuffOffset = 0;
	LPBYTE lpbBufferRcv;
	mrim_packet_header_t *pmaHeader;

	NETLIBSELECT nls = {0};
	nls.cbSize = sizeof(nls);
	nls.dwTimeout = NETLIB_SELECT_TIMEOUT;
	nls.hReadConns[0] = hConnection;

	lpbBufferRcv = (LPBYTE)mir_calloc(dwRcvBuffSize);

	dwCMDNum = 0;
	MraSendCMD(MRIM_CS_HELLO, NULL, 0);
	InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, GetTickCount());
	while (m_iStatus != ID_STATUS_OFFLINE && bContinue)
	{
		dwSelectRet = CallService(MS_NETLIB_SELECT, 0, (LPARAM)&nls);
		switch (dwSelectRet) {
		case SOCKET_ERROR:
			if (m_iStatus != ID_STATUS_OFFLINE) {
				dwRetErrorCode = GetLastError();
				ShowFormattedErrorMessage(L"Disconnected, socket error", dwRetErrorCode);
			}
			bContinue = FALSE;
			break;

		case 0:// Time out
		case 1:
			dwBytesReceived = GetTickCount();
			InterlockedExchange((volatile LONG*)&dwThreadWorkerLastPingTime, dwBytesReceived);
			// server ping
			if (dwNextPingSendTickTime <= dwBytesReceived) {
				dwNextPingSendTickTime = (dwBytesReceived+(dwPingPeriod*1000));
				MraSendCMD(MRIM_CS_PING, NULL, 0);
			}
			{
				DWORD dwCMDNum, dwFlags, dwAckType;
				HANDLE hContact;
				LPBYTE lpbData;
				size_t dwDataSize;
				while ( !MraSendQueueFindOlderThan(hSendQueueHandle, SEND_QUEUE_TIMEOUT, &dwCMDNum, &dwFlags, &hContact, &dwAckType, &lpbData, &dwDataSize)) {
					switch (dwAckType) {
					case ACKTYPE_ADDED:
					case ACKTYPE_AUTHREQ:
					case ACKTYPE_CONTACTS:
						//nothing to do
						break;
					case ACKTYPE_MESSAGE:
						ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)dwCMDNum, (LPARAM)"Undefined message deliver error, time out");
						break;
					case ACKTYPE_GETINFO:
						ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)1, 0);
						break;
					case ACKTYPE_SEARCH:
						ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)dwCMDNum, 0);
						break;
					case ICQACKTYPE_SMS:
						mir_free(lpbData);
						break;
					}
					MraSendQueueFree(hSendQueueHandle, dwCMDNum);
				}
			}

			if (dwSelectRet == 0) // Time out
				break;

			// expand receive buffer dynamically
			if ((dwRcvBuffSize - dwRcvBuffSizeUsed) < BUFF_SIZE_RCV_MIN_FREE) {
				dwRcvBuffSize += BUFF_SIZE_RCV;
				lpbBufferRcv = (LPBYTE)mir_realloc(lpbBufferRcv, dwRcvBuffSize);
			}

			dwBytesReceived = Netlib_Recv(nls.hReadConns[0], (LPSTR)(lpbBufferRcv+dwRcvBuffSizeUsed), (dwRcvBuffSize-dwRcvBuffSizeUsed), 0);
			if (dwBytesReceived && dwBytesReceived != SOCKET_ERROR) {
				dwRcvBuffSizeUsed += dwBytesReceived;

				while (TRUE) {
					dwDataCurrentBuffSize = (dwRcvBuffSize-dwDataCurrentBuffOffset);
					dwDataCurrentBuffSizeUsed = (dwRcvBuffSizeUsed-dwDataCurrentBuffOffset);
					pmaHeader = (mrim_packet_header_t*)(lpbBufferRcv+dwDataCurrentBuffOffset);

					// packet header received
					if (dwDataCurrentBuffSizeUsed >= sizeof(mrim_packet_header_t)) {
						// packet OK
						if (pmaHeader->magic == CS_MAGIC) {
							// full packet received, may be more than one
							if ((dwDataCurrentBuffSizeUsed-sizeof(mrim_packet_header_t)) >= pmaHeader->dlen) {

								MraCommandDispatcher(pmaHeader, &dwPingPeriod, &dwNextPingSendTickTime, &bContinue);

								// move pointer to next packet in buffer
								if (dwDataCurrentBuffSizeUsed - sizeof(mrim_packet_header_t) > pmaHeader->dlen)
									dwDataCurrentBuffOffset += sizeof(mrim_packet_header_t) + pmaHeader->dlen;
								// move pointer to begin of buffer
								else {
									// динамическое уменьшение буффера приёма
									if (dwRcvBuffSize > BUFF_SIZE_RCV) {
										dwRcvBuffSize = BUFF_SIZE_RCV;
										lpbBufferRcv = (LPBYTE)mir_realloc(lpbBufferRcv, dwRcvBuffSize);
									}
									dwDataCurrentBuffOffset = 0;
									dwRcvBuffSizeUsed = 0;
									break;
								}
							}
							// not all packet received, continue receiving
							else {
								if (dwDataCurrentBuffOffset) {
									memmove(lpbBufferRcv, (lpbBufferRcv+dwDataCurrentBuffOffset), dwDataCurrentBuffSizeUsed);
									dwRcvBuffSizeUsed = dwDataCurrentBuffSizeUsed;
									dwDataCurrentBuffOffset = 0;
								}
								DebugPrintCRLFW(L"Not all packet received, continue receiving");
								break;
							}
						}
						// bad packet
						else {
							DebugPrintCRLFW(L"Bad packet");
							dwDataCurrentBuffOffset = 0;
							dwRcvBuffSizeUsed = 0;
							break;
						}
					}
					// packet to small, continue receiving
					else {
						DebugPrintCRLFW(L"Packet to small, continue receiving");
						memmove(lpbBufferRcv, (lpbBufferRcv+dwDataCurrentBuffOffset), dwDataCurrentBuffSizeUsed);
						dwRcvBuffSizeUsed = dwDataCurrentBuffSizeUsed;
						dwDataCurrentBuffOffset = 0;
						break;
					}
				}
			}
			// disconnected
			else {
				if (m_iStatus != ID_STATUS_OFFLINE) {
					dwRetErrorCode = GetLastError();
					ShowFormattedErrorMessage(L"Disconnected, socket read error", dwRetErrorCode);
				}
				bContinue = FALSE;
			}
			break;
		}// end switch
	}// end while
	mir_free(lpbBufferRcv);

	return dwRetErrorCode;
}

DWORD CMraProto::MraCommandDispatcher(mrim_packet_header_t *pmaHeader, DWORD *pdwPingPeriod, DWORD *pdwNextPingSendTickTime, BOOL *pbContinue)
{
	WCHAR szBuff[4096] = {0};
	DWORD dwDataSize, dwTemp, dwAckType;
	size_t dwStringSize;
	MRA_LPS lpsString = {0}, lpsEMail = {0};
	HANDLE hContact = NULL;
	LPBYTE lpbData, lpbDataCurrent;

	lpbData = ((((LPBYTE)pmaHeader))+sizeof(mrim_packet_header_t));
	lpbDataCurrent = lpbData;
	dwDataSize = pmaHeader->dlen;

	Netlib_Logf(hNetlibUser, "Received packet %x\n", pmaHeader->msg);

	switch (pmaHeader->msg) {
	case MRIM_CS_HELLO_ACK://Подтверждение установки соединения// UL ## ping_period ## Ожидаемая частота подтверждения соединения (в секундах)
		//bit of a security hole here, since it's easy to extract a password from an edit box
		if (GetPassDB((LPSTR)szBuff, SIZEOF(szBuff), &dwStringSize)) {
			CHAR szEMail[MAX_EMAIL_LEN], szSelfVersionString[MAX_PATH], szUserAgentFormated[USER_AGENT_MAX+MAX_PATH], szValueName[MAX_PATH];
			WCHAR wszStatusTitle[STATUS_TITLE_MAX+4], wszStatusDesc[STATUS_DESC_MAX+4];
			DWORD dwStatus, dwXStatus, dwXStatusMir, dwFutureFlags;
			LPWSTR lpwszStatusTitle, lpwszStatusDesc;
			size_t dwEMailSize, dwSelfVersionSize, dwStatusTitleSize, dwStatusDescSize, dwUserAgentFormattedSize;

			dwXStatusMir = m_iXStatus;
			dwStatus = GetMraStatusFromMiradaStatus(m_iDesiredStatus, dwXStatusMir, &dwXStatus);
			if ( IsXStatusValid(dwXStatusMir)) {// xstatuses
				mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatusMir);
				if (mraGetStaticStringW(NULL, szValueName, wszStatusTitle, (STATUS_TITLE_MAX+1), &dwStatusTitleSize))
					lpwszStatusTitle = wszStatusTitle;
				else { // default xstatus name
					lpwszStatusTitle = TranslateW(lpcszXStatusNameDef[dwXStatusMir]);
					dwStatusTitleSize = lstrlenW(lpwszStatusTitle);
				}

				mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatusMir);
				if (mraGetStaticStringW(NULL, szValueName, wszStatusDesc, (STATUS_DESC_MAX+1), &dwStatusDescSize))
					lpwszStatusDesc = wszStatusDesc;
				else { // default xstatus description
					lpwszStatusDesc = NULL;
					dwStatusDescSize = 0;
				}
			}
			else { // not xstatuses
				lpwszStatusTitle = GetStatusModeDescriptionW(m_iDesiredStatus);
				dwStatusTitleSize = lstrlenW(lpwszStatusTitle);
				lpwszStatusDesc = NULL;
				dwStatusDescSize = 0;
			}

			MraGetSelfVersionString(szSelfVersionString, SIZEOF(szSelfVersionString), &dwSelfVersionSize);
			if ( mraGetStaticStringA(NULL, "MirVerCustom", szUserAgentFormated, SIZEOF(szUserAgentFormated), &dwUserAgentFormattedSize) == FALSE) {
				dwUserAgentFormattedSize = mir_snprintf(szUserAgentFormated, SIZEOF(szUserAgentFormated),
					"client=\"magent\" name=\"Miranda NG\" title=\"%s\" version=\"777.%lu.%lu.%lu\" build=\"%lu\" protocol=\"%lu.%lu\"",
					szSelfVersionString, __FILEVERSION_STRING, PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR);
			}

			dwFutureFlags = ((getByte(NULL, "RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE)? FEATURE_FLAG_RTF_MESSAGE:0)|MRA_FEATURE_FLAGS);

			if (mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
				MraLogin2W(szEMail, dwEMailSize, (LPSTR)szBuff, dwStringSize, dwStatus, lpcszStatusUri[dwXStatus], lstrlenA(lpcszStatusUri[dwXStatus]), lpwszStatusTitle, dwStatusTitleSize, lpwszStatusDesc, dwStatusDescSize, dwFutureFlags, szUserAgentFormated, dwUserAgentFormattedSize, szSelfVersionString, dwSelfVersionSize);
			else
				*pbContinue = FALSE;

			SecureZeroMemory(szBuff, sizeof(szBuff));
		}
		else *pbContinue = FALSE;

		*pdwPingPeriod = GetUL(&lpbDataCurrent);
		break;

	case MRIM_CS_LOGIN_ACK: // Successful authorization
		m_bLoggedIn = TRUE;
		*pdwNextPingSendTickTime = 0; // force send ping
		MraSendCMD(MRIM_CS_PING, NULL, 0);
		SetStatus(m_iDesiredStatus);
		MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, GAIF_FORCE, NULL, 0);
		break;

	case MRIM_CS_LOGIN_REJ: // Unsuccessful authorization //LPS ## reason ## причина отказа
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);

		GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
		dwStringSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpsString.lpszData, lpsString.dwSize, szBuff, SIZEOF(szBuff));
		szBuff[dwStringSize] = 0;
		MraPopupShowW(NULL, MRA_POPUP_TYPE_ERROR, 0, TranslateW(L"Logon error: invalid login/password"), szBuff);
		*pbContinue = FALSE;
		break;

	case MRIM_CS_MESSAGE_ACK:// Message delivery
		{
			DWORD dwMsgID, dwFlags;
			MRA_LPS lpsText, lpsRTFText, lpsMultiChatData;

			dwMsgID = GetUL(&lpbDataCurrent);
			dwFlags = GetUL(&lpbDataCurrent);
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);//LPS ## from ## Адрес отправителя
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsText);//LPS ## message ## текстовая версия сообщения
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsRTFText);//LPS ## rtf-message ## форматированная версия сообщения
			if (dwFlags&MESSAGE_FLAG_MULTICHAT) GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsMultiChatData); // LPS multichat_data

			// подтверждаем получение, только если удалось его обработать
			if (MraRecvCommand_Message((DWORD)_time32(NULL), dwFlags, &lpsEMail, &lpsText, &lpsRTFText, &lpsMultiChatData) == NO_ERROR)
				if ((dwFlags&MESSAGE_FLAG_NORECV) == 0)
					MraMessageRecv(lpsEMail.lpszData, lpsEMail.dwSize, dwMsgID);
		}
		break;

	case MRIM_CS_MESSAGE_STATUS:
		if ( !MraSendQueueFind(hSendQueueHandle, pmaHeader->seq, NULL, &hContact, &dwAckType, (LPBYTE*)&lpsString.lpszData, &lpsString.dwSize)) {
			dwTemp = GetUL(&lpbDataCurrent);
			switch (dwTemp) {
			case MESSAGE_DELIVERED:// Message delivered directly to user
				ProtoBroadcastAckAsync(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)pmaHeader->seq, 0);
				break;//***deb возможны сбои из-за асинхронности тк там передаётся указатель
			case MESSAGE_REJECTED_NOUSER:// Message rejected - no such user
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)"Message rejected - no such user");
				break;
			case MESSAGE_REJECTED_INTERR:// Internal server error
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)"Internal server error");
				break;
			case MESSAGE_REJECTED_LIMIT_EXCEEDED:// Offline messages limit exceeded
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)"Offline messages limit exceeded");
				break;
			case MESSAGE_REJECTED_TOO_LARGE:// Message is too large
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)"Message is too large");
				break;
			case MESSAGE_REJECTED_DENY_OFFMSG:// User does not accept offline messages
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)"User does not accept offline messages");
				break;
			case MESSAGE_REJECTED_DENY_OFFFLSH:// User does not accept offline flash animation
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)"User does not accept offline flash animation");
				break;
			default:
				dwTemp = mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), "Undefined message delivery error, code: %lu", dwTemp);
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)pmaHeader->seq, (LPARAM)szBuff);
				break;
			}
			MraSendQueueFree(hSendQueueHandle, pmaHeader->seq);
		}
		// not found in queue
		else if ( GetUL(&lpbDataCurrent) != MESSAGE_DELIVERED)
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateW(L"MRIM_CS_MESSAGE_STATUS: not found in queue"));
		break;

	case MRIM_CS_CONNECTION_PARAMS:// Изменение параметров соединения
		*pdwPingPeriod = GetUL(&lpbDataCurrent);
		*pdwNextPingSendTickTime = 0; // force send ping
		MraSendCMD(MRIM_CS_PING, NULL, 0);
		break;

	case MRIM_CS_USER_INFO:
		while (lpbDataCurrent < lpbData + dwDataSize) {
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
			if ( !_strnicmp(lpsString.lpszData, "MESSAGES.TOTAL", 14)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				dwEmailMessagesTotal = StrToUNum32(lpsString.lpszData, lpsString.dwSize);
			}
			else if ( !_strnicmp(lpsString.lpszData, "MESSAGES.UNREAD", 15)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				dwEmailMessagesUnread = StrToUNum32(lpsString.lpszData, lpsString.dwSize);
			}
			else if ( !_strnicmp(lpsString.lpszData, "MRIM.NICKNAME", 13)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				mraSetLPSStringW(NULL, "Nick", &lpsString);
			}
			else if ( !_strnicmp(lpsString.lpszData, "client.endpoint", 15)) {
				LPSTR lpszDelimiter;

				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				dwStringSize = WideCharToMultiByte(MRA_CODE_PAGE, 0, lpsString.lpwszData, lpsString.dwSize, (LPSTR)szBuff, sizeof(szBuff), NULL, NULL);
				lpszDelimiter = (LPSTR)MemoryFind(0, szBuff, dwStringSize, ":", 1);
				if (lpszDelimiter) {
					(*lpszDelimiter) = 0;
					lpszDelimiter = (LPSTR)szBuff;
					setDword(NULL, "IP", HTONL(inet_addr(lpszDelimiter)));
				}
			}
			else if ( !_strnicmp(lpsString.lpszData, "connect.xml", 11)) {
				DebugPrintA(lpsString.lpszData);
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				DebugPrintCRLFW(lpsString.lpwszData);
			}
			else if ( !_strnicmp(lpsString.lpszData, "micblog.show_title", 18)) {
				DebugPrintA(lpsString.lpszData);
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				DebugPrintCRLFW(lpsString.lpwszData);
			}
			else if ( !_strnicmp(lpsString.lpszData, "micblog.status.id", 17)) {
				DWORDLONG dwBlogStatusID;

				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				dwBlogStatusID = StrToUNum64(lpsString.lpszData, lpsString.dwSize);
				mraWriteContactSettingBlob(NULL, DBSETTING_BLOGSTATUSID, &dwBlogStatusID, sizeof(DWORDLONG));
			}
			else if ( !_strnicmp(lpsString.lpszData, "micblog.status.time", 19)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "micblog.status.text", 19)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				mraSetLPSStringW(NULL, DBSETTING_BLOGSTATUS, &lpsString);
			}
			else if ( !_strnicmp(lpsString.lpszData, "HAS_MYMAIL", 10)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "mrim.status.open_search", 23)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "rb.target.cookie", 16)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "show_web_history_link", 21)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "friends_suggest", 15)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "timestamp", 9)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else if ( !_strnicmp(lpsString.lpszData, "trusted_update", 14)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
				//setDword(NULL, DBSETTING_BLOGSTATUSTIME, StrToUNum32(lpsString.lpszData, lpsString.dwSize));
			}
			else {
				#ifdef _DEBUG
					LPSTR lpszCurrentPos = (LPSTR)szBuff;
					memmove(lpszCurrentPos, lpsString.lpszData, lpsString.dwSize);
					lpszCurrentPos += lpsString.dwSize;
					*((WORD*)lpszCurrentPos) = *((WORD*)": ");
					lpszCurrentPos += sizeof(WORD);

					GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
					memmove(lpszCurrentPos, lpsString.lpszData, lpsString.dwSize);
					lpszCurrentPos += lpsString.dwSize;
					*((WORD*)lpszCurrentPos) = *((WORD*)szCRLF);
					lpszCurrentPos += sizeof(WORD);(*((WORD*)lpszCurrentPos)) = 0;

					DebugPrintCRLFA((LPSTR)szBuff);
					DebugBreak();
				#endif
			}
		}
		MraUpdateEmailStatus(NULL, 0, NULL, 0, 0, 0);
		break;

	case MRIM_CS_OFFLINE_MESSAGE_ACK://Сообщение доставленное, пока пользователь не был подключен к сети
		{
			DWORD dwTime, dwFlags;
			MRA_LPS lpsText, lpsRTFText, lpsMultiChatData;
			LPBYTE lpbBuff = NULL;
			DWORDLONG dwMsgUIDL = GetUIDL(&lpbDataCurrent);
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);

			if (MraOfflineMessageGet(&lpsString, &dwTime, &dwFlags, &lpsEMail, &lpsText, &lpsRTFText, &lpsMultiChatData, &lpbBuff) == NO_ERROR) {
				dwTemp = MraRecvCommand_Message(dwTime, dwFlags, &lpsEMail, &lpsText, &lpsRTFText, &lpsMultiChatData);
				if (dwTemp == NO_ERROR || dwTemp == ERROR_ACCESS_DENIED)
					MraOfflineMessageDel(dwMsgUIDL);
				else
					ShowFormattedErrorMessage(L"Offline message processing error, message will not deleted from server", NO_ERROR);
			}
			else ShowFormattedErrorMessage(L"Offline message processing error, message will not deleted from server", NO_ERROR);

			mir_free(lpbBuff);
		}
		break;

	case MRIM_CS_AUTHORIZE_ACK: // Auth confirmation
		{
			BOOL bAdded;
			BYTE btBuff[BUFF_SIZE_BLOB];

			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);
			hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, TRUE, TRUE, &bAdded);
			if (bAdded) MraUpdateContactInfo(hContact);

			if (IsEMailChatAgent(lpsEMail.lpszData, lpsEMail.dwSize) == FALSE) {
				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.szModule = m_szModuleName;
				dbei.timestamp = (DWORD)_time32(NULL);
				dbei.flags = 0;
				dbei.eventType = EVENTTYPE_ADDED;
				CreateBlobFromContact(hContact, NULL, 0, (LPBYTE)&btBuff, SIZEOF(btBuff), &dwStringSize);
				dbei.cbBlob = dwStringSize;
				dbei.pBlob = btBuff;
				db_event_add(0, &dbei);
			}

			GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwTemp, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);
			dwTemp &= ~CONTACT_INTFLAG_NOT_AUTHORIZED;
			SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, SCBIF_SERVER_FLAG, 0, 0, 0, dwTemp, 0, NULL, 0, NULL, 0, NULL, 0);
			setDword(hContact, "HooksLocked", TRUE);
			db_unset(hContact, "CList", "NotOnList");
			setDword(hContact, "HooksLocked", FALSE);
		}
		break;

	case MRIM_CS_MPOP_SESSION: // Web auth key
		if ( GetUL(&lpbDataCurrent)) {
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
			MraMPopSessionQueueSetNewMPopKey(hMPopSessionQueue, lpsString.lpszData, lpsString.dwSize);
			MraMPopSessionQueueStart(hMPopSessionQueue);
		}
		else { //error
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING, 0, TranslateW(L"Server error: cant get MPOP key for web authorize"));
			MraMPopSessionQueueFlush(hMPopSessionQueue);
		}
		break;

	/////////////////////////////////////////////////////////////////////////////////////
	case MRIM_CS_FILE_TRANSFER:
		{
			BOOL bAdded;
			DWORD dwIDRequest, dwFilesTotalSize;
			MRA_LPS lpsFiles = {0}, lpsFilesW = {0}, lpsAddreses = {0};

			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);// LPS TO/FROM ANSI
			dwIDRequest = GetUL(&lpbDataCurrent);// DWORD id_request
			dwFilesTotalSize = GetUL(&lpbDataCurrent);// DWORD FILESIZE
			if ( GetUL(&lpbDataCurrent)) {
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsFiles);// LPS Files (FileName;FileSize;FileName;FileSize;) ANSI
				if ( GetUL(&lpbDataCurrent)) { // LPS DESCRIPTION
					dwTemp = GetUL(&lpbDataCurrent);
					DebugBreakIf(dwTemp != 1);
					GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsFilesW);// LPS Files (FileName;FileSize;FileName;FileSize;) UNICODE
				}
				GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsAddreses);// LPS Conn (IP:Port;IP:Port;) ANSI
			}

			hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, TRUE, TRUE, &bAdded);
			if (bAdded) MraUpdateContactInfo(hContact);

			bAdded = FALSE;
			if (lpsFilesW.dwSize == 0) {
				lpsFilesW.lpwszData = (LPWSTR)mir_calloc((lpsFiles.dwSize+MAX_PATH)*sizeof(WCHAR));
				if (lpsFilesW.lpwszData) {
					lpsFilesW.dwSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpsFiles.lpszData, lpsFiles.dwSize, lpsFilesW.lpwszData, (lpsFiles.dwSize+MAX_PATH));
					bAdded = TRUE;
				}
			}

			if (lpsFilesW.dwSize)
				MraFilesQueueAddReceive(hFilesQueueHandle, 0, hContact, dwIDRequest, lpsFilesW.lpwszData, lpsFilesW.dwSize, lpsAddreses.lpszData, lpsAddreses.dwSize);
			if (bAdded)
				mir_free(lpsFilesW.lpwszData);
		}
		break;

	case MRIM_CS_FILE_TRANSFER_ACK:
		dwAckType = GetUL(&lpbDataCurrent);// DWORD status
		GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);// LPS TO/FROM
		dwTemp = GetUL(&lpbDataCurrent);// DWORD id_request
		GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);// LPS DESCRIPTION

		switch (dwAckType) {
		case FILE_TRANSFER_STATUS_OK:// игнорируем, мы и так уже слушаем порт(ждём), то что кто то согласился ничего не меняет
			//hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, TRUE, TRUE, NULL);
			break;
		case FILE_TRANSFER_STATUS_DECLINE:
			MraFilesQueueCancel(hFilesQueueHandle, dwTemp, FALSE);
			break;
		case FILE_TRANSFER_STATUS_ERROR:
			ShowFormattedErrorMessage(L"File transfer: error", NO_ERROR);
			MraFilesQueueCancel(hFilesQueueHandle, dwTemp, FALSE);
			break;
		case FILE_TRANSFER_STATUS_INCOMPATIBLE_VERS:
			ShowFormattedErrorMessage(L"File transfer: incompatible versions", NO_ERROR);
			MraFilesQueueCancel(hFilesQueueHandle, dwTemp, FALSE);
			break;
		case FILE_TRANSFER_MIRROR:
			MraFilesQueueSendMirror(hFilesQueueHandle, dwTemp, lpsString.lpszData, lpsString.dwSize);
			break;
		default:// ## unknown error
			mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateW(L"MRIM_CS_FILE_TRANSFER_ACK: unknown error, code: %lu"), dwAckType);
			ShowFormattedErrorMessage(szBuff, NO_ERROR);
			break;
		}
		break;

	/////////////////////////////////////////////////////////////////////////////////////
	case MRIM_CS_USER_STATUS://Смена статуса другого пользователя
		{
			BOOL bAdded;
			DWORD dwStatus, dwXStatus, dwFutureFlags;
			MRA_LPS lpsSpecStatusUri, lpsStatusTitle, lpsStatusDesc, lpsUserAgentFormated;

			dwStatus = GetUL(&lpbDataCurrent);
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsSpecStatusUri);
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsStatusTitle);
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsStatusDesc);
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);
			dwFutureFlags = GetUL(&lpbDataCurrent);// com_support ( >= 1.14)
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsUserAgentFormated);

			if ((hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, TRUE, TRUE, &bAdded))) {
				if (bAdded)
					MraUpdateContactInfo(hContact);

				dwTemp = GetMiradaStatusFromMraStatus(dwStatus, GetMraXStatusIDFromMraUriStatus(lpsSpecStatusUri.lpszData, lpsSpecStatusUri.dwSize), &dwXStatus);

				MraContactCapabilitiesSet(hContact, dwFutureFlags);
				setByte(hContact, DBSETTING_XSTATUSID, (BYTE)dwXStatus);
				mraSetLPSStringW(hContact, DBSETTING_XSTATUSNAME, &lpsStatusTitle);
				mraSetLPSStringW(hContact, DBSETTING_XSTATUSMSG, &lpsStatusDesc);

				if (dwTemp != ID_STATUS_OFFLINE) { // пишем клиента только если юзер не отключён, иначе не затираем старое
					if (lpsUserAgentFormated.dwSize) {
						if (getByte(NULL, "MirVerRaw", MRA_DEFAULT_MIRVER_RAW) == FALSE) {
							MraGetVersionStringFromFormatted(lpsUserAgentFormated.lpszData, lpsUserAgentFormated.dwSize, (LPSTR)szBuff, SIZEOF(szBuff), &dwStringSize);
							lpsUserAgentFormated.lpszData = (LPSTR)szBuff;
							lpsUserAgentFormated.dwSize = dwStringSize;
						}
					}
					else { // хз чё за клиент
						lpsUserAgentFormated.lpszData = MIRVER_UNKNOWN;
						lpsUserAgentFormated.dwSize = (sizeof(MIRVER_UNKNOWN)-1);
					}
					mraSetLPSStringA(hContact, "MirVer", &lpsUserAgentFormated);
				}

				if (dwTemp == MraGetContactStatus(hContact)) {// меняем шило на шило, подозрительно? ;)
					if (dwTemp == ID_STATUS_OFFLINE) { // was/now invisible
						WCHAR szEMail[MAX_EMAIL_LEN];

						mraGetStaticStringW(hContact, "e-mail", szEMail, SIZEOF(szEMail), NULL);
						mir_sntprintf(szBuff, SIZEOF(szBuff), L"%s <%s> - %s", GetContactNameW(hContact), szEMail, TranslateW(L"invisible status changed"));
						MraPopupShowFromContactW(hContact, MRA_POPUP_TYPE_INFORMATION, 0, szBuff);

						MraSetContactStatus(hContact, ID_STATUS_INVISIBLE);
					}
				}
				MraSetContactStatus(hContact, dwTemp);
				SetExtraIcons(hContact);
			}
		}
		break;

	case MRIM_CS_LOGOUT:// Пользователь отключен из-за параллельного входа с его логином.
		if ( GetUL(&lpbDataCurrent) == LOGOUT_NO_RELOGIN_FLAG)
			ShowFormattedErrorMessage(L"Another user connected with your login", NO_ERROR);
		*pbContinue = FALSE;
		break;

	case MRIM_CS_ADD_CONTACT_ACK:
	case MRIM_CS_MODIFY_CONTACT_ACK:
		if ( !MraSendQueueFind(hSendQueueHandle, pmaHeader->seq, NULL, &hContact, &dwAckType, (LPBYTE*)&lpsString.lpszData, &lpsString.dwSize)) {
			dwTemp = GetUL(&lpbDataCurrent);
			switch (dwTemp) {
			case CONTACT_OPER_SUCCESS:// ## добавление произведено успешно
				if (pmaHeader->msg == MRIM_CS_ADD_CONTACT_ACK) SetContactBasicInfoW(hContact, 0, (SCBIF_ID|SCBIF_SERVER_FLAG), GetUL(&lpbDataCurrent), 0, 0, CONTACT_INTFLAG_NOT_AUTHORIZED, 0, NULL, 0, NULL, 0, NULL, 0);
				break;
			case CONTACT_OPER_ERROR:// ## переданные данные были некорректны
				ShowFormattedErrorMessage(L"Data been sent are invalid", NO_ERROR);
				break;
			case CONTACT_OPER_INTERR:// ## при обработке запроса произошла внутренняя ошибка
				ShowFormattedErrorMessage(L"Internal server error", NO_ERROR);
				break;
			case CONTACT_OPER_NO_SUCH_USER:// ## добавляемого пользователя не существует в системе
				SetContactBasicInfoW(hContact, 0, SCBIF_SERVER_FLAG, 0, 0, 0, -1, 0, NULL, 0, NULL, 0, NULL, 0);
				ShowFormattedErrorMessage(L"User does not registred", NO_ERROR);
				break;
			case CONTACT_OPER_INVALID_INFO:// ## некорректное имя пользователя
				ShowFormattedErrorMessage(L"Invalid user name", NO_ERROR);
				break;
			case CONTACT_OPER_USER_EXISTS:// ## пользователь уже есть в контакт-листе
				ShowFormattedErrorMessage(L"User allready added", NO_ERROR);
				break;
			case CONTACT_OPER_GROUP_LIMIT:// ## превышено максимально допустимое количество групп (20)
				ShowFormattedErrorMessage(L"Group limit is 20", NO_ERROR);
				break;
			default:// ## unknown error
				mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateW(L"MRIM_CS_*_CONTACT_ACK: unknown server error, code: %lu"), dwTemp);
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, szBuff);
				break;
			}
			MraSendQueueFree(hSendQueueHandle, pmaHeader->seq);
		}
		else MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateW(L"MRIM_CS_*_CONTACT_ACK: not found in queue"));
		break;

	case MRIM_CS_ANKETA_INFO:
		if ( !MraSendQueueFind(hSendQueueHandle, pmaHeader->seq, NULL, &hContact, &dwAckType, (LPBYTE*)&lpsString.lpszData, &lpsString.dwSize)) {
			switch (GetUL(&lpbDataCurrent)) {
			case MRIM_ANKETA_INFO_STATUS_OK:// поиск успешно завершен
				{
					DWORD dwFeildsNum, dwMaxRows, dwServerTime, dwStatus;
					size_t i;
					MRA_LPS *pmralpsFeilds, *pmralpsValues;

					dwFeildsNum = GetUL(&lpbDataCurrent);
					dwMaxRows = GetUL(&lpbDataCurrent);
					dwServerTime = GetUL(&lpbDataCurrent);

					pmralpsFeilds = (MRA_LPS*)mir_calloc(((dwFeildsNum*2)+4)*sizeof(MRA_LPS));
					if (pmralpsFeilds) {
						pmralpsValues = (pmralpsFeilds+dwFeildsNum);
						// read headers name
						for (i = 0; i < dwFeildsNum; i++) {
							GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &pmralpsFeilds[i]);
							DebugPrintCRLFA(pmralpsFeilds[i].lpszData);
						}

						while (lpbDataCurrent < lpbData+dwDataSize) {
							// read values
							for (i = 0;i<dwFeildsNum;i++)
								GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &pmralpsValues[i]);

							// write to DB and exit loop
							if (dwAckType == ACKTYPE_GETINFO && hContact) {
								setDword(hContact, "InfoTS", (DWORD)_time32(NULL));
								//MRA_LPS mralpsUsernameValue;
								for (i = 0; i < dwFeildsNum; i++) {
									if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Username", 8))
										; //mralpsUsernameValue = pmralpsValues[i];
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Domain", 6))
										;
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Flags", 5))
										;
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Nickname", 8))
										mraSetLPSStringW(hContact, "Nick", &pmralpsValues[i]);
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "FirstName", 9))
										mraSetLPSStringW(hContact, "FirstName", &pmralpsValues[i]);
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "LastName", 8))
										mraSetLPSStringW(hContact, "LastName", &pmralpsValues[i]);
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Sex", 3)) {
										switch (StrToUNum32(pmralpsValues[i].lpszData, pmralpsValues[i].dwSize)) {
										case 1:// мужской
											setByte(hContact, "Gender", 'M');
											break;
										case 2:// женский
											setByte(hContact, "Gender", 'F');
											break;
										default:// а фиг его знает
											mraDelValue(hContact, "Gender");
											break;
										}
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Birthday", 8)) {
										if (pmralpsValues[i].dwSize > 9) {// calc "Age"
											SYSTEMTIME stTime = {0};

											stTime.wYear = (WORD)StrToUNum32(pmralpsValues[i].lpszData, 4);
											stTime.wMonth = (WORD)StrToUNum32((pmralpsValues[i].lpszData+5), 2);
											stTime.wDay = (WORD)StrToUNum32((pmralpsValues[i].lpszData+8), 2);
											setWord(hContact, "BirthYear", stTime.wYear);
											setByte(hContact, "BirthMonth", (BYTE)stTime.wMonth);
											setByte(hContact, "BirthDay", (BYTE)stTime.wDay);

											setWord(hContact, "Age", (WORD)GetYears(&stTime));
										}
										else {
											mraDelValue(hContact, "BirthYear");
											mraDelValue(hContact, "BirthMonth");
											mraDelValue(hContact, "BirthDay");
											mraDelValue(hContact, "Age");
										}
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "City_id", 7)) {
										dwTemp = StrToUNum32(pmralpsValues[i].lpszData, pmralpsValues[i].dwSize);
										if (dwTemp) {
											for (size_t j = 0;mrapPlaces[j].lpszData;j++) {
												if (mrapPlaces[j].dwCityID == dwTemp) {
													mraSetStringW(hContact, "City", mrapPlaces[j].lpszData);
													break;
												}
											}
										}
										else mraDelValue(hContact, "City");
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Location", 8))
										mraSetLPSStringW(hContact, "About", &pmralpsValues[i]);
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Zodiac", 6))
										;
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "BMonth", 6))
										;
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "BDay", 4))
										;
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Country_id", 10)) {
										dwTemp = StrToUNum32(pmralpsValues[i].lpszData, pmralpsValues[i].dwSize);
										if (dwTemp) {
											for (size_t j = 0; mrapPlaces[j].lpszData; j++) {
												if (mrapPlaces[j].dwCountryID == dwTemp) {
													mraSetStringW(hContact, "Country", mrapPlaces[j].lpszData);
													break;
												}
											}
										}
										else mraDelValue(hContact, "Country");
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Phone", 5)) {
										mraDelValue(hContact, "Phone");
										mraDelValue(hContact, "Cellular");
										mraDelValue(hContact, "Fax");

										if (pmralpsValues[i].dwSize) {
											lpsString.lpszData = pmralpsValues[i].lpszData;
											LPSTR lpszCurPos = (LPSTR)MemoryFindByte(0, pmralpsValues[i].lpszData, pmralpsValues[i].dwSize, ',');
											if (lpszCurPos)
												lpsString.dwSize = (lpszCurPos-lpsString.lpszData);
											else
												lpsString.dwSize = ((pmralpsValues[i].lpszData+pmralpsValues[i].dwSize)-lpsString.lpszData);

											mraSetLPSStringA(hContact, "Phone", &lpsString);

											if (lpszCurPos) {
												lpsString.lpszData = (++lpszCurPos);
												lpszCurPos = (LPSTR)MemoryFindByte((lpszCurPos-pmralpsValues[i].lpszData), pmralpsValues[i].lpszData, pmralpsValues[i].dwSize, ',');
												if (lpszCurPos)
													lpsString.dwSize = (lpszCurPos-lpsString.lpszData);
												else
													lpsString.dwSize = ((pmralpsValues[i].lpszData+pmralpsValues[i].dwSize)-lpsString.lpszData);

												mraSetLPSStringA(hContact, "Cellular", &lpsString);
											}

											if (lpszCurPos) {
												lpsString.lpszData = (++lpszCurPos);
												lpszCurPos = (LPSTR)MemoryFindByte((lpszCurPos-pmralpsValues[i].lpszData), pmralpsValues[i].lpszData, pmralpsValues[i].dwSize, ',');
												if (lpszCurPos)
													lpsString.dwSize = (lpszCurPos-lpsString.lpszData);
												else
													lpsString.dwSize = ((pmralpsValues[i].lpszData+pmralpsValues[i].dwSize)-lpsString.lpszData);

												mraSetLPSStringA(hContact, "Fax", &lpsString);
											}
										}
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "mrim_status", 11)) {
										if (pmralpsValues[i].dwSize) {
											DWORD dwID, dwContactSeverFlags;

											GetContactBasicInfoW(hContact, &dwID, NULL, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);
											// для авторизованного нам и так присылают правильный статус
											if (dwID == -1 || (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED)) {
												dwStatus = StrHexToUNum32(pmralpsValues[i].lpszData, pmralpsValues[i].dwSize);
												MraSetContactStatus(hContact, GetMiradaStatusFromMraStatus(dwStatus, MRA_MIR_XSTATUS_NONE, NULL));
												setByte(hContact, DBSETTING_XSTATUSID, (BYTE)MRA_MIR_XSTATUS_NONE);
											}
										}
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "status_uri", 10)) {
										if (pmralpsValues[i].dwSize) {
											DWORD dwID, dwContactSeverFlags, dwXStatus;

											GetContactBasicInfoW(hContact, &dwID, NULL, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);
											if (dwID == -1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED) {
												MraSetContactStatus(hContact, GetMiradaStatusFromMraStatus(dwStatus, GetMraXStatusIDFromMraUriStatus(pmralpsValues[i].lpszData, pmralpsValues[i].dwSize), &dwXStatus));
												setByte(hContact, DBSETTING_XSTATUSID, (BYTE)dwXStatus);
											}
										}
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "status_title", 12)) {
										if (pmralpsValues[i].dwSize) {
											DWORD dwID, dwContactSeverFlags;

											GetContactBasicInfoW(hContact, &dwID, NULL, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);
											if (dwID == -1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
												mraSetLPSStringW(hContact, DBSETTING_XSTATUSNAME, &pmralpsValues[i]);
										}
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "status_desc", 11)) {
										if (pmralpsValues[i].dwSize) {
											DWORD dwID, dwContactSeverFlags;

											GetContactBasicInfoW(hContact, &dwID, NULL, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);
											if (dwID == -1 || dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)
												mraSetLPSStringW(hContact, DBSETTING_XSTATUSMSG, &pmralpsValues[i]);
										}
									}
									else {// for DEBUG ONLY
										#ifdef _DEBUG
											DebugPrintCRLFA(pmralpsFeilds[i].lpszData);
											DebugPrintCRLFA(pmralpsValues[i].lpszData);
											//DebugBreak();
										#endif
									}
								}
							}
							else if (dwAckType == ACKTYPE_SEARCH) {
								WCHAR szNick[MAX_EMAIL_LEN] = {0},
									szFirstName[MAX_EMAIL_LEN] = {0},
									szLastName[MAX_EMAIL_LEN] = {0},
									szEMail[MAX_EMAIL_LEN] = {0};
								MRA_LPS mralpsUsernameValue = {0};
								PROTOSEARCHRESULT psr = {0};

								psr.cbSize = sizeof(psr);
								psr.flags = PSR_UNICODE;
								psr.nick = szNick;
								psr.firstName = szFirstName;
								psr.lastName = szLastName;
								psr.email = szEMail;
								psr.id = szEMail;

								for (i = 0;i<dwFeildsNum;i++) {
									if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Username", 8))
										mralpsUsernameValue = pmralpsValues[i];
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Domain", 6)) { // имя было уже задано ранее
										dwStringSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, mralpsUsernameValue.lpszData, mralpsUsernameValue.dwSize, szEMail, SIZEOF(szEMail));

										szEMail[dwStringSize] = (*((WCHAR*)L"@"));
										dwStringSize++;

										dwStringSize += MultiByteToWideChar(MRA_CODE_PAGE, 0, pmralpsValues[i].lpszData, pmralpsValues[i].dwSize, &szEMail[dwStringSize], (SIZEOF(szEMail)-(dwStringSize+1)));
										szEMail[dwStringSize] = 0;
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "Nickname", 8)) {
										dwStringSize = min((sizeof(szNick)-sizeof(WCHAR)), pmralpsValues[i].dwSize);
										memmove(szNick, pmralpsValues[i].lpwszData, dwStringSize);
										szNick[dwStringSize] = 0;
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "FirstName", 9)) {
										dwStringSize = min((sizeof(szFirstName)-sizeof(WCHAR)), pmralpsValues[i].dwSize);
										memmove(szFirstName, pmralpsValues[i].lpwszData, dwStringSize);
										szFirstName[dwStringSize] = 0;
									}
									else if ( !_strnicmp(pmralpsFeilds[i].lpszData, "LastName", 8)) {
										dwStringSize = min((sizeof(szLastName)-sizeof(WCHAR)), pmralpsValues[i].dwSize);
										memmove(szLastName, pmralpsValues[i].lpwszData, dwStringSize);
										szLastName[dwStringSize] = 0;
									}
								}// end for
								ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_DATA, (HANDLE)pmaHeader->seq, (LPARAM)&psr);
							}
						}// end while

						mir_free(pmralpsFeilds);
					}
				}

				switch (dwAckType) {
				case ACKTYPE_GETINFO:
					ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)1, 0);
					break;
				case ACKTYPE_SEARCH:
				default:
					ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)pmaHeader->seq, 0);
					break;
				}
				break;

			case MRIM_ANKETA_INFO_STATUS_NOUSER:// не найдено ни одной подходящей записи
				SetContactBasicInfoW(hContact, 0, SCBIF_SERVER_FLAG, 0, 0, 0, -1, 0, NULL, 0, NULL, 0, NULL, 0);
			case MRIM_ANKETA_INFO_STATUS_DBERR:// ошибка базы данных
			case MRIM_ANKETA_INFO_STATUS_RATELIMERR:// слишком много запросов, поиск временно запрещен
				switch (dwAckType) {
				case ACKTYPE_GETINFO:
					ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)1, 0);
					break;
				case ACKTYPE_SEARCH:
					ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)pmaHeader->seq, 0);
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
			MraSendQueueFree(hSendQueueHandle, pmaHeader->seq);
		}
		else MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateW(L"MRIM_ANKETA_INFO: not found in queue"));
		break;

	case MRIM_CS_MAILBOX_STATUS:
		dwTemp = GetUL(&lpbDataCurrent);
		if (dwTemp > dwEmailMessagesTotal)
			dwEmailMessagesTotal += (dwTemp - dwEmailMessagesUnread);

		dwAckType = dwEmailMessagesUnread;// save old value
		dwEmailMessagesUnread = dwTemp;// store new value
		if (getByte(NULL, "IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY) == 0 || dwAckType<dwTemp || dwTemp == 0)
			MraUpdateEmailStatus(NULL, 0, NULL, 0, 0, 0);
		break;

	case MRIM_CS_GAME:
		{
			DWORD dwGameSessionID, dwGameMsg, dwGameMsgID;

			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);//LPS to/from
			dwGameSessionID = GetUL(&lpbDataCurrent);//DWORD session unique per game
			dwGameMsg = GetUL(&lpbDataCurrent);//DWORD msg internal game message
			dwGameMsgID = GetUL(&lpbDataCurrent);//DWORD msg_id id for ack
			dwTemp = GetUL(&lpbDataCurrent);//DWORD time_send time of client
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);//LPS data

			#ifdef _DEBUG
				BYTE btBuff[1024] = {0};
				memmove(btBuff, lpsString.lpszData, lpsString.dwSize);
			#endif

			switch (dwGameMsg) {
			case GAME_CONNECTION_INVITE:
				if (m_iStatus != ID_STATUS_INVISIBLE)
					MraGame(lpsEMail.lpszData, lpsEMail.dwSize, dwGameSessionID, GAME_DECLINE, dwGameMsgID, lpsString.lpszData, lpsString.dwSize);
				break;
			case GAME_CONNECTION_ACCEPT:
				break;
			case GAME_DECLINE:
				break;
			case GAME_INC_VERSION:
				break;
			case GAME_NO_SUCH_GAME:// user invisible
				if ((hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, FALSE, TRUE, NULL)))
				if (MraGetContactStatus(hContact) == ID_STATUS_OFFLINE)
					MraSetContactStatus(hContact, ID_STATUS_INVISIBLE);
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
				mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateW(L"MRIM_CS_GAME: unknown internal game message code: %lu"), dwGameMsg);
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, szBuff);
				break;
			}
		}
		break;
	case MRIM_CS_CONTACT_LIST2:
		dwTemp = GetUL(&lpbDataCurrent);
		if (dwTemp == GET_CONTACTS_OK) { // received contact list
			BOOL bAdded;
			char szGroupMask[MAX_PATH], szContactMask[MAX_PATH];
			DWORD dwID, dwGroupFlags, dwContactFlag, dwGroupID, dwContactSeverFlags, dwStatus, dwXStatus, dwFutureFlags, dwBlogStatusTime;
			MRA_LPS mralpsGroupName, mralpsNick, mralpsCustomPhones, lpsSpecStatusUri, lpsStatusTitle, lpsStatusDesc, lpsUserAgentFormated, lpsBlogStatus, lpsBlogStatusMusic;
			size_t i, j, dwGroupsCount, dwGroupMaskSize, dwContactMaskSize, dwControlParam;
			ULARGE_INTEGER dwBlogStatusID;

			dwGroupsCount = GetUL(&lpbDataCurrent);

			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
			dwGroupMaskSize = lpsString.dwSize;
			memmove(szGroupMask, lpsString.lpszData, dwGroupMaskSize);(*(szGroupMask+dwGroupMaskSize)) = 0;

			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
			dwContactMaskSize = lpsString.dwSize;
			memmove(szContactMask, lpsString.lpszData, dwContactMaskSize);(*(szContactMask+dwContactMaskSize)) = 0;

			DebugPrintCRLFW(L"Groups:");
			DebugPrintCRLFA(szGroupMask);
			dwID = 0;
			for (i = 0; i < dwGroupsCount; i++) { //groups handle
				dwControlParam = 0;
				for (j = 0; j < dwGroupMaskSize; j++) { //enumerating parameters
					switch (szGroupMask[j]) {
					case 's'://LPS
						GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
						break;
					case 'u'://UL
						dwTemp = GetUL(&lpbDataCurrent);
						break;
					case 'z'://sz
						lpsString.lpszData = (LPSTR)lpbDataCurrent;
						lpsString.dwSize = lstrlenA((LPSTR)lpbDataCurrent);
						lpbDataCurrent += lpsString.dwSize;
						DebugBreak();
						break;
					}

					if (j == 0 && szGroupMask[j] == 'u') {// GroupFlags
						dwGroupFlags = dwTemp;
						dwControlParam++;
					}
					else if (j == 1 && szGroupMask[j] == 's') {// GroupName
						mralpsGroupName = lpsString;
						dwControlParam++;
					}
				}

				// add/modify group
				if (dwControlParam > 1) { // все параметры правильно инициализированны!
					#ifdef _DEBUG
						memmove(szBuff, mralpsGroupName.lpszData, mralpsGroupName.dwSize);
						szBuff[(mralpsGroupName.dwSize/sizeof(WCHAR))] = 0;
						DebugPrintW(szBuff);

						mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), ": flags: %lu (", dwGroupFlags);
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
			dwID = 20;
			while (lpbDataCurrent < lpbData + dwDataSize) {
				dwControlParam = 0;
				for (j = 0; j < dwContactMaskSize; j++) { //enumerating parameters
					switch (szContactMask[j]) {
					case 's'://LPS
						GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);
						break;
					case 'u'://UL
						dwTemp = GetUL(&lpbDataCurrent);
						break;
					case 'z'://sz
						lpsString.lpszData = (LPSTR)lpbDataCurrent;
						lpsString.dwSize = lstrlenA((LPSTR)lpbDataCurrent);
						lpbDataCurrent += lpsString.dwSize;
						DebugBreak();
						break;
					}

					if (j == 0 && szContactMask[j] == 'u') { // Flags
						dwContactFlag = dwTemp;
						dwControlParam++;
					}
					else if (j == 1 && szContactMask[j] == 'u') { // Group id
						dwGroupID = dwTemp;
						dwControlParam++;
					}
					else if (j == 2 && szContactMask[j] == 's') { // Email
						lpsEMail = lpsString;
						dwControlParam++;
					}
					else if (j == 3 && szContactMask[j] == 's') { // Nick
						mralpsNick = lpsString;
						dwControlParam++;
					}
					else if (j == 4 && szContactMask[j] == 'u') { // Server flags
						dwContactSeverFlags = dwTemp;
						dwControlParam++;
					}
					else if (j == 5 && szContactMask[j] == 'u') { // Status
						dwStatus = dwTemp;
						dwControlParam++;
					}
					else if (j == 6 && szContactMask[j] == 's') { // Custom Phone number,
						mralpsCustomPhones = lpsString;
						dwControlParam++;
					}
					else if (j == 7 && szContactMask[j] == 's') { // spec_status_uri
						lpsSpecStatusUri = lpsString;
						dwControlParam++;
					}
					else if (j == 8 && szContactMask[j] == 's') { // status_title
						lpsStatusTitle = lpsString;
						dwControlParam++;
					}
					else if (j == 9 && szContactMask[j] == 's') { // status_desc
						lpsStatusDesc = lpsString;
						dwControlParam++;
					}
					else if (j == 10 && szContactMask[j] == 'u') { // com_support (future flags)
						dwFutureFlags = dwTemp;
						dwControlParam++;
					}
					else if (j == 11 && szContactMask[j] == 's') { // user_agent (formated string)
						lpsUserAgentFormated = lpsString;
						dwControlParam++;
					}
					else if (j == 12 && szContactMask[j] == 'u') { // BlogStatusID
						dwBlogStatusID.LowPart = dwTemp;
						dwControlParam++;
					}
					else if (j == 13 && szContactMask[j] == 'u') { // BlogStatusID
						dwBlogStatusID.HighPart = dwTemp;
						dwControlParam++;
					}
					else if (j == 14 && szContactMask[j] == 'u') { // BlogStatusTime
						dwBlogStatusTime = dwTemp;
						dwControlParam++;
					}
					else if (j == 15 && szContactMask[j] == 's') { // BlogStatus
						lpsBlogStatus = lpsString;
						dwControlParam++;
					}
					else if (j == 16 && szContactMask[j] == 's') { // BlogStatusMusic
						lpsBlogStatusMusic = lpsString;
						dwControlParam++;
					}
					else if (j == 17 && szContactMask[j] == 's') { // BlogStatusSender // ignory
						lpsString = lpsString;
						dwControlParam++;
					}
					else if (j == 18 && szContactMask[j] == 's') { // geo data ?
						lpsString = lpsString;
						dwControlParam++;
					}
					else if (j == 19 && szContactMask[j] == 's') { // ?????? ?
						lpsString = lpsString;
						dwControlParam++;
						DebugBreakIf(lpsString.dwSize);
					}
					else {
						if (szContactMask[j] == 's') {
							if (lpsString.dwSize) {
								DebugPrintCRLFW(lpsString.lpwszData);
							}
						}
						else if (szContactMask[j] == 'u') {
							mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), "%lu, ", dwTemp);//;
							DebugPrintCRLFA((LPSTR)szBuff);
						}
						else DebugBreak();
					}
				}

				#ifdef _DEBUG
					mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), "ID: %lu, Group id: %lu, ", dwID, dwGroupID);
					DebugPrintA((LPSTR)szBuff);

					memmove(szBuff, lpsEMail.lpszData, lpsEMail.dwSize);
					szBuff[(lpsEMail.dwSize/sizeof(WCHAR))] = 0;
					DebugPrintA((LPSTR)szBuff);

					mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), ": flags: %lu (", dwContactFlag);
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

					mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), ": server flags: %lu (", dwContactSeverFlags);
					DebugPrintA((LPSTR)szBuff);
					if (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)		DebugPrintA("CONTACT_INTFLAG_NOT_AUTHORIZED, ");
					DebugPrintCRLFA(")");
				#endif

				// add/modify contact
				if (dwGroupID != 103)//***deb filtering phone/sms contats
				if ( _strnicmp(lpsEMail.lpszData, "phone", 5))
				if (dwControlParam>5)// все параметры правильно инициализированны!
				if ((dwContactFlag&(CONTACT_FLAG_GROUP|CONTACT_FLAG_REMOVED)) == 0) {
					hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, TRUE, FALSE, &bAdded);
					if (hContact) {
						// already in list, remove the duplicate
						if (GetContactBasicInfoW(hContact, &dwTemp, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL) == NO_ERROR && dwTemp != -1) {
							dwTemp = dwTemp;
							DebugBreak();
						}
						else {
							dwTemp = GetMiradaStatusFromMraStatus(dwStatus, GetMraXStatusIDFromMraUriStatus(lpsSpecStatusUri.lpszData, lpsSpecStatusUri.dwSize), &dwXStatus);
							if ((dwContactFlag&CONTACT_FLAG_UNICODE_NAME))
								mralpsNick.dwSize /= sizeof(WCHAR);

							if (bAdded) { // update user info
								SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_FLAG|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_NICK|SCBIF_PHONES), dwID, dwGroupID, dwContactFlag, dwContactSeverFlags, dwTemp, NULL, 0, mralpsNick.lpwszData, mralpsNick.dwSize, mralpsCustomPhones.lpszData, mralpsCustomPhones.dwSize);
								// request user info from server
								MraUpdateContactInfo(hContact);
							}
							else { //****deb - check group ID param
								SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS), dwID, dwGroupID, dwContactFlag, dwContactSeverFlags, dwTemp, NULL, 0, mralpsNick.lpwszData, mralpsNick.dwSize, mralpsCustomPhones.lpszData, mralpsCustomPhones.dwSize);
								if (mralpsNick.dwSize == 0) { // set the server-side nick
									lstrcpynW(szBuff, GetContactNameW(hContact), SIZEOF(szBuff));
									mralpsNick.lpwszData = szBuff;
									mralpsNick.dwSize = lstrlenW(mralpsNick.lpwszData);

									MraModifyContactW(hContact, dwID, dwContactFlag, dwGroupID, lpsEMail.lpszData, lpsEMail.dwSize, mralpsNick.lpwszData, mralpsNick.dwSize, mralpsCustomPhones.lpszData, mralpsCustomPhones.dwSize);
								}
							}

							MraContactCapabilitiesSet(hContact, dwFutureFlags);
							setByte(hContact, DBSETTING_XSTATUSID, (BYTE)dwXStatus);
							mraSetLPSStringW(hContact, DBSETTING_XSTATUSNAME, &lpsStatusTitle);
							mraSetLPSStringW(hContact, DBSETTING_XSTATUSMSG, &lpsStatusDesc);
							setDword(hContact, DBSETTING_BLOGSTATUSTIME, dwBlogStatusTime);
							mraWriteContactSettingBlob(hContact, DBSETTING_BLOGSTATUSID, &dwBlogStatusID.QuadPart, sizeof(DWORDLONG));
							mraSetLPSStringW(hContact, DBSETTING_BLOGSTATUS, &lpsBlogStatus);
							mraSetLPSStringW(hContact, DBSETTING_BLOGSTATUSMUSIC, &lpsBlogStatusMusic);
							if ( IsXStatusValid(dwXStatus))
								SetExtraIcons(hContact);

							if (dwTemp != ID_STATUS_OFFLINE) { // пишем клиента только если юзер не отключён, иначе не затираем старое
								if (lpsUserAgentFormated.dwSize) {
									if (getByte(NULL, "MirVerRaw", MRA_DEFAULT_MIRVER_RAW) == FALSE) {
										MraGetVersionStringFromFormatted(lpsUserAgentFormated.lpszData, lpsUserAgentFormated.dwSize, (LPSTR)szBuff, SIZEOF(szBuff), &dwStringSize);
										lpsUserAgentFormated.lpszData = (LPSTR)szBuff;
										lpsUserAgentFormated.dwSize = dwStringSize;
									}
								}
								else {
									lpsUserAgentFormated.lpszData = MIRVER_UNKNOWN;
									lpsUserAgentFormated.dwSize = (sizeof(MIRVER_UNKNOWN)-1);
								}
								mraSetLPSStringA(hContact, "MirVer", &lpsUserAgentFormated);
							}

							if (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED)
							if (getByte(NULL, "AutoAuthRequestOnLogon", MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON))
								CallProtoService(m_szModuleName, MRA_REQ_AUTH, (WPARAM)hContact, 0);
						}
					}
				}
				dwID++;
			}// end while (processing contacts)

			// post processing contact list
			{
				CHAR szEMail[MAX_EMAIL_LEN], szPhones[MAX_EMAIL_LEN];
				WCHAR wszAuthMessage[MAX_PATH], wszNick[MAX_EMAIL_LEN];
				size_t dwEMailSize, dwNickSize, dwPhonesSize, dwAuthMessageSize;

				if (mraGetStaticStringW(NULL, "AuthMessage", wszAuthMessage, SIZEOF(wszAuthMessage), &dwAuthMessageSize) == FALSE) { // def auth message
					lstrcpynW(wszAuthMessage, TranslateW(MRA_DEFAULT_AUTH_MESSAGE), SIZEOF(wszAuthMessage));
					dwAuthMessageSize = lstrlenW(wszAuthMessage);
				}

				for (hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					if (GetContactBasicInfoW(hContact, &dwID, NULL, NULL, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, NULL, 0, NULL, NULL, 0, NULL) == NO_ERROR)
					if (dwID == -1) {
						if (IsEMailChatAgent(szEMail, dwEMailSize)) {// чат: ещё раз запросим авторизацию, пометим как видимый в списке, постоянный
							db_unset(hContact, "CList", "Hidden");
							db_unset(hContact, "CList", "NotOnList");
							SetExtraIcons(hContact);
							MraSetContactStatus(hContact, ID_STATUS_ONLINE);

							lstrcpynW(szBuff, GetContactNameW(hContact), SIZEOF(szBuff));
							MraAddContactW(hContact, (CONTACT_FLAG_VISIBLE|CONTACT_FLAG_MULTICHAT|CONTACT_FLAG_UNICODE_NAME), -1, szEMail, dwEMailSize, szBuff, lstrlenW(szBuff), NULL, 0, NULL, 0, 0);
						}
						else {
							if (db_get_b(hContact, "CList", "NotOnList", 0) == 0) { // set extra icons and upload contact
								SetExtraIcons(hContact);
								if (getByte(NULL, "AutoAddContactsToServer", MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER)) { //add all contacts to server
									GetContactBasicInfoW(hContact, NULL, &dwGroupID, NULL, NULL, NULL, NULL, 0, NULL, wszNick, SIZEOF(wszNick), &dwNickSize, szPhones, SIZEOF(szPhones), &dwPhonesSize);
									MraAddContactW(hContact, (CONTACT_FLAG_VISIBLE|CONTACT_FLAG_UNICODE_NAME), dwGroupID, szEMail, dwEMailSize, wszNick, dwNickSize, szPhones, dwPhonesSize, wszAuthMessage, dwAuthMessageSize, 0);
								}
							}
						}
						MraUpdateContactInfo(hContact);
					}
				}
			}
		}
		else { // контакт лист почемуто не получили
			// всех в offline и id в нестандарт
			for (HANDLE hContact = db_find_first();hContact != NULL;hContact = db_find_next(hContact)) {
				SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS), -1, -2, 0, 0, ID_STATUS_OFFLINE, NULL, 0, NULL, 0, NULL, 0);
				// request user info from server
				MraUpdateContactInfo(hContact);
			}

			if (dwTemp == GET_CONTACTS_ERROR) // найденный контакт-лист некорректен
				ShowFormattedErrorMessage(L"MRIM_CS_CONTACT_LIST2: bad contact list", NO_ERROR);
			else if (dwTemp == GET_CONTACTS_INTERR) // произошла внутренняя ошибка
				ShowFormattedErrorMessage(L"MRIM_CS_CONTACT_LIST2: internal server error", NO_ERROR);
			else {
				mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateW(L"MRIM_CS_CONTACT_LIST2: unknown server error, code: %lu"), dwTemp);
				MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, szBuff);
			}
		}
		break;

	case MRIM_CS_SMS_ACK:
		dwTemp = GetUL(&lpbDataCurrent);
		if ( MraSendQueueFind(hSendQueueHandle, pmaHeader->seq, NULL, &hContact, &dwAckType, (LPBYTE*)&lpsString.lpszData, &lpsString.dwSize) == NO_ERROR) {
			char szEMail[MAX_EMAIL_LEN];
			LPSTR lpszPhone;
			LPWSTR lpwszMessage;
			size_t dwEMailSize, dwPhoneSize, dwMessageSize;

			if (mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
				dwPhoneSize = (*(DWORD*)lpsString.lpszData);
				dwMessageSize = lpsString.dwSize-(dwPhoneSize+sizeof(DWORD)+2);
				lpszPhone = (lpsString.lpszData+sizeof(DWORD));
				lpwszMessage = (LPWSTR)(lpszPhone+dwPhoneSize+1);

				dwTemp = mir_snprintf((LPSTR)szBuff, SIZEOF(szBuff), "<sms_response><source>Mail.ru</source><deliverable>Yes</deliverable><network>Mail.ru, Russia</network><message_id>%s-1-1955988055-%s</message_id><destination>%s</destination><messages_left>0</messages_left></sms_response>\r\n", szEMail, lpszPhone, lpszPhone);
				ProtoBroadcastAck(NULL, dwAckType, ACKRESULT_SENTREQUEST, (HANDLE)pmaHeader->seq, (LPARAM)szBuff);
			}

			mir_free(lpsString.lpszData);
			MraSendQueueFree(hSendQueueHandle, pmaHeader->seq);
		}
		else MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateW(L"MRIM_CS_SMS_ACK: not found in queue"));
		break;

	case MRIM_CS_PROXY:
		{
			DWORD dwIDRequest;
			MRA_LPS lpsAddreses = {0};
			MRA_GUID mguidSessionID;

			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);// LPS to
			dwIDRequest = GetUL(&lpbDataCurrent);// DWORD id_request
			dwAckType = GetUL(&lpbDataCurrent);// DWORD data_type
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);// LPS user_data
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsAddreses);// LPS lps_ip_port
			mguidSessionID = GetGUID(&lpbDataCurrent);// DWORD session_id[4]

			if (dwAckType == MRIM_PROXY_TYPE_FILES) { // файлы, on file recv
				// set proxy info to file transfer context
				if ( !MraMrimProxySetData(MraFilesQueueItemProxyByID(hFilesQueueHandle, dwIDRequest), lpsEMail.lpszData, lpsEMail.dwSize, dwIDRequest, dwAckType, lpsString.lpszData, lpsString.dwSize, lpsAddreses.lpszData, lpsAddreses.dwSize, &mguidSessionID))
					MraFilesQueueStartMrimProxy(hFilesQueueHandle, dwIDRequest);
				else { // empty/invalid session
					MraProxyAck(PROXY_STATUS_ERROR, lpsEMail.lpszData, lpsEMail.dwSize, dwIDRequest, dwAckType, lpsString.lpszData, lpsString.dwSize, lpsAddreses.lpszData, lpsAddreses.dwSize, mguidSessionID);
					DebugBreak();
				}
			}
		}
		break;

	case MRIM_CS_PROXY_ACK:
		{
			DWORD dwIDRequest;
			HANDLE hMraMrimProxyData;
			MRA_LPS lpsAddreses = {0};
			MRA_GUID mguidSessionID;

			dwTemp = GetUL(&lpbDataCurrent);// DWORD status
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);// LPS to
			dwIDRequest = GetUL(&lpbDataCurrent);// DWORD id_request
			dwAckType = GetUL(&lpbDataCurrent);// DWORD data_type
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);// LPS user_data
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsAddreses);// LPS lps_ip_port
			mguidSessionID = GetGUID(&lpbDataCurrent);// DWORD session_id[4]

			if (dwAckType == MRIM_PROXY_TYPE_FILES) { // on file send
				if ((hMraMrimProxyData = MraFilesQueueItemProxyByID(hFilesQueueHandle, dwIDRequest))) {
					switch (dwTemp) {
					case PROXY_STATUS_DECLINE:
						MraFilesQueueCancel(hFilesQueueHandle, dwIDRequest, FALSE);
						break;
					case PROXY_STATUS_OK:
						// set proxy info to file transfer context
						if ( !MraMrimProxySetData(hMraMrimProxyData, lpsEMail.lpszData, lpsEMail.dwSize, dwIDRequest, dwAckType, lpsString.lpszData, lpsString.dwSize, lpsAddreses.lpszData, lpsAddreses.dwSize, &mguidSessionID))
							MraFilesQueueStartMrimProxy(hFilesQueueHandle, dwIDRequest);
						break;
					case PROXY_STATUS_ERROR:
						ShowFormattedErrorMessage(L"Proxy File transfer: error", NO_ERROR);
						MraFilesQueueCancel(hFilesQueueHandle, dwIDRequest, FALSE);
						break;
					case PROXY_STATUS_INCOMPATIBLE_VERS:
						ShowFormattedErrorMessage(L"Proxy File transfer: incompatible versions", NO_ERROR);
						MraFilesQueueCancel(hFilesQueueHandle, dwIDRequest, FALSE);
						break;
					case PROXY_STATUS_NOHARDWARE:
					case PROXY_STATUS_MIRROR:
					case PROXY_STATUS_CLOSED:
					default:
						DebugBreak();
						break;
					}
				}
				else DebugBreak();
			}
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
			DWORD dwDate, dwUIDL;

			dwTemp = GetUL(&lpbDataCurrent);// UL unread count
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);// LPS from
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);// LPS subject
			dwDate = GetUL(&lpbDataCurrent);// UL date
			dwUIDL = GetUL(&lpbDataCurrent);// UL uidl

			if (dwTemp > dwEmailMessagesTotal)
				dwEmailMessagesTotal += (dwTemp-dwEmailMessagesUnread);

			dwAckType = dwEmailMessagesUnread;// save old value
			dwEmailMessagesUnread = dwTemp;// store new value
			if (getByte(NULL, "IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY) == 0 || dwAckType<dwTemp || dwTemp == 0)
				MraUpdateEmailStatus(lpsEMail.lpszData, lpsEMail.dwSize, lpsString.lpszData, lpsString.dwSize, dwDate, dwUIDL);
		}
		break;

	case MRIM_CS_USER_BLOG_STATUS:
		{
			DWORD dwTime, dwFlags;
			MRA_LPS lpsText;
			LPBYTE lpbBuff = NULL;
			DWORDLONG dwBlogStatusID;

			dwFlags = GetUL(&lpbDataCurrent);// UL flags
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsEMail);// LPS user
			dwBlogStatusID = GetUIDL(&lpbDataCurrent);// UINT64 id
			dwTime = GetUL(&lpbDataCurrent);// UL time
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsText);// LPS text (MRIM_BLOG_STATUS_MUSIC: track)
			GetLPS(lpbData, dwDataSize, &lpbDataCurrent, &lpsString);// LPS reply_user_nick

			if ((hContact = MraHContactFromEmail(lpsEMail.lpszData, lpsEMail.dwSize, FALSE, TRUE, NULL))) {
				if (dwFlags & MRIM_BLOG_STATUS_MUSIC)
					mraSetLPSStringW(hContact, DBSETTING_BLOGSTATUSMUSIC, &lpsText);
				else {
					setDword(hContact, DBSETTING_BLOGSTATUSTIME, dwTime);
					mraWriteContactSettingBlob(hContact, DBSETTING_BLOGSTATUSID, &dwBlogStatusID, sizeof(DWORDLONG));
					mraSetLPSStringW(hContact, DBSETTING_BLOGSTATUS, &lpsText);
				}
			}
		}
		break;

	case MRIM_CS_UNKNOWN:
	case MRIM_CS_USER_GEO:
	case MRIM_CS_SERVER_SETTINGS:
		break;

	default:
		#ifdef _DEBUG
			BYTE btBuff[8192] = {0};
			memmove(btBuff, lpbData, dwDataSize);
			DebugBreak();
		#endif
		break;
	}
	return 0;
}

DWORD CMraProto::MraRecvCommand_Message(DWORD dwTime, DWORD dwFlags, MRA_LPS *plpsFrom, MRA_LPS *plpsText, MRA_LPS *plpsRFTText, MRA_LPS *plpsMultiChatData)
{// Сообщение
	BOOL bAdded;
	DWORD dwRetErrorCode = NO_ERROR, dwBackColour;
	LPSTR lpszMessageExt = NULL;
	LPWSTR lpwszMessage = NULL;
	size_t dwMessageSize = 0, dwMessageExtSize = 0;

	PROTORECVEVENT pre = {0};
	pre.timestamp = dwTime;

	// check flags and datas
	if (dwFlags & MESSAGE_FLAG_RTF) {
		if (plpsRFTText) {
			if (plpsRFTText->lpszData == NULL || plpsRFTText->dwSize == 0)
				dwFlags &= ~MESSAGE_FLAG_RTF;
		}
		else dwFlags &= ~MESSAGE_FLAG_RTF;
	}

	if (dwFlags & MESSAGE_FLAG_MULTICHAT) {
		if (plpsMultiChatData) {
			if (plpsMultiChatData->lpszData == NULL || plpsMultiChatData->dwSize == 0)
				dwFlags &= ~MESSAGE_FLAG_MULTICHAT;
		}
		else dwFlags &= ~MESSAGE_FLAG_MULTICHAT;
	}

	// pre processing - extracting/decoding
	if (dwFlags & MESSAGE_FLAG_AUTHORIZE) { // extract auth message из обычного текста
		size_t dwAuthPartsCount, dwAuthBuffSize = (plpsText->dwSize+32), dwAuthDataSize;
		LPBYTE lpbAuthData, lpbDataCurrent;
		MRA_LPS lpsAuthFrom, lpsAuthMessage;

		lpbAuthData = (LPBYTE)mir_calloc(dwAuthBuffSize);
		if (lpbAuthData) {
			BASE64DecodeFormated(plpsText->lpszData, plpsText->dwSize, lpbAuthData, dwAuthBuffSize, &dwAuthDataSize);

			lpbDataCurrent = lpbAuthData;
			dwAuthPartsCount = GetUL(&lpbDataCurrent);
			if ( !GetLPS(lpbAuthData, dwAuthDataSize, &lpbDataCurrent, &lpsAuthFrom))
			if ( !GetLPS(lpbAuthData, dwAuthDataSize, &lpbDataCurrent, &lpsAuthMessage)) {
				if (dwFlags&MESSAGE_FLAG_v1p16 && (dwFlags&MESSAGE_FLAG_CP1251) == 0) { // unicode text
					memmove(lpbAuthData, lpsAuthMessage.lpszData, lpsAuthMessage.dwSize);
					lpwszMessage = (LPWSTR)lpbAuthData;
					dwMessageSize = (lpsAuthMessage.dwSize/sizeof(WCHAR));
				}
				else { // преобразуем в юникод текст только если он в АНСИ и если это не Флэш мультик и будильник тоже не нуждается в этом
					lpwszMessage = (LPWSTR)mir_calloc(((lpsAuthMessage.dwSize+MAX_PATH)*sizeof(WCHAR)));
					if (lpwszMessage) {
						dwMessageSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpsAuthMessage.lpszData, lpsAuthMessage.dwSize, lpwszMessage, (lpsAuthMessage.dwSize+MAX_PATH));
						(*(lpwszMessage+dwMessageSize)) = 0;
					}
					else dwRetErrorCode = GetLastError();
				}
			}
			if (lpwszMessage != (LPWSTR)lpbAuthData)
				mir_free(lpbAuthData);
		}
	}
	else {
		// unicode text
		if ((dwFlags & (MESSAGE_FLAG_ALARM|MESSAGE_FLAG_FLASH|MESSAGE_FLAG_v1p16)) && (dwFlags & MESSAGE_FLAG_CP1251) == 0) {
			lpwszMessage = plpsText->lpwszData;
			dwMessageSize = (plpsText->dwSize/sizeof(WCHAR));
		}
		else {
			lpwszMessage = (LPWSTR)mir_calloc(((plpsText->dwSize+MAX_PATH)*sizeof(WCHAR)));
			if (lpwszMessage) {
				dwMessageSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, plpsText->lpszData, plpsText->dwSize, lpwszMessage, (plpsText->dwSize+MAX_PATH));
				(*(lpwszMessage+dwMessageSize)) = 0;
			}
			else dwRetErrorCode = GetLastError();
		}

		if (dwFlags & (MESSAGE_FLAG_CONTACT|MESSAGE_FLAG_NOTIFY|MESSAGE_FLAG_SMS|MESSAGE_SMS_DELIVERY_REPORT|MESSAGE_FLAG_ALARM))
			; // do nothing; there's no extra part in a message
		else {
			if ((dwFlags & MESSAGE_FLAG_RTF) && plpsRFTText) //MESSAGE_FLAG_FLASH there
			if (plpsRFTText->lpszData && plpsRFTText->dwSize) { // decode RTF
				size_t dwRTFPartsCount, dwCompressedSize, dwRFTBuffSize = ((plpsRFTText->dwSize*16)+8192), dwRTFDataSize;
				LPBYTE lpbRTFData, lpbCompressed, lpbDataCurrent;
				MRA_LPS lpsRTFString, lpsBackColour, lpsString;

				lpbRTFData = (LPBYTE)mir_calloc(dwRFTBuffSize);
				lpbCompressed = (LPBYTE)mir_calloc((plpsRFTText->dwSize+32));
				if (lpbRTFData && lpbCompressed) {
					BASE64DecodeFormated(plpsRFTText->lpszData, plpsRFTText->dwSize, lpbCompressed, (plpsRFTText->dwSize+32), &dwCompressedSize);
					dwRTFDataSize = dwRFTBuffSize;
					if ( uncompress(lpbRTFData, (DWORD*)&dwRTFDataSize, lpbCompressed, dwCompressedSize) == Z_OK) {
						lpbDataCurrent = lpbRTFData;
						dwRTFPartsCount = GetUL(&lpbDataCurrent);// колличество частей в некоторых случаях больше 2, тогда нужно игнорировать первый текст, тк там сообщения об ущербности
						if (GetLPS(lpbRTFData, dwRTFDataSize, &lpbDataCurrent, &lpsRTFString) == NO_ERROR)
						if (GetLPS(lpbRTFData, dwRTFDataSize, &lpbDataCurrent, &lpsBackColour) == NO_ERROR) {
							dwBackColour = (*(DWORD*)lpsBackColour.lpszData);
							if (dwFlags & MESSAGE_FLAG_FLASH) {
								if (dwRTFPartsCount == 3) { // ansi text only
									GetLPS(lpbRTFData, dwRTFDataSize, &lpbDataCurrent, &lpsString);
									lpwszMessage = (LPWSTR)mir_calloc(((lpsString.dwSize+MAX_PATH)*sizeof(WCHAR)));
									if (lpwszMessage) {
										memmove(lpwszMessage, lpsString.lpszData, lpsString.dwSize);
										dwMessageSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpsString.lpszData, lpsString.dwSize, lpwszMessage, (lpsString.dwSize+MAX_PATH));
										(*(lpwszMessage+dwMessageSize)) = 0;
									}
									else dwRetErrorCode = GetLastError();
								}
								else if (dwRTFPartsCount == 4) {
									GetLPS(lpbRTFData, dwRTFDataSize, &lpbDataCurrent, &lpsString);
									GetLPS(lpbRTFData, dwRTFDataSize, &lpbDataCurrent, &lpsString);
									lpwszMessage = (LPWSTR)mir_calloc(lpsString.dwSize);
									if (lpwszMessage) {
										memmove(lpwszMessage, lpsString.lpszData, lpsString.dwSize);
										dwMessageSize = lpsString.dwSize;
									}
									else dwRetErrorCode = GetLastError();
								}
								else DebugBreak();
							}
							else { // RTF text
								if (dwRTFPartsCount > 2) {
									GetLPS(lpbRTFData, dwRTFDataSize, &lpbDataCurrent, &lpsString);
									DebugBreak();
								}

								lpszMessageExt = (LPSTR)mir_calloc(lpsRTFString.dwSize);
								if (lpszMessageExt) {
									memmove(lpszMessageExt, lpsRTFString.lpszData, lpsRTFString.dwSize);
									dwMessageExtSize = lpsRTFString.dwSize;
								}
								else DebugBreak();
							}
						}
					}else {
						mir_free(lpszMessageExt);
						dwMessageExtSize = 0;
						DebugBreak();
					}
				}
				mir_free(lpbCompressed);
				mir_free(lpbRTFData);
			}
		}
	}

	Netlib_Logf(hNetlibUser, "Processing message: %08X, from '%s', text '%S'\n", dwFlags, plpsFrom->lpszData, lpwszMessage);

	// processing
	if (dwRetErrorCode == NO_ERROR)
	if (MraAntiSpamReceivedMessageW(plpsFrom->lpszData, plpsFrom->dwSize, dwFlags, lpwszMessage, dwMessageSize) == MESSAGE_NOT_SPAM) {
		if (dwFlags & (MESSAGE_FLAG_SMS | MESSAGE_SMS_DELIVERY_REPORT)) {// SMS //if (IsPhone(plpsFrom->lpszData, plpsFrom->dwSize))
			char szPhone[MAX_EMAIL_LEN], szEMail[MAX_EMAIL_LEN], szTime[MAX_PATH];
			LPSTR lpszMessageUTF, lpszBuff;
			LPWSTR lpwszMessageXMLEncoded;
			size_t dwBuffLen, dwMessageXMLEncodedSize;
			INTERNET_TIME itTime;

			dwBuffLen = ((dwMessageSize+MAX_PATH)*6);
			lpszMessageUTF = (LPSTR)mir_calloc(dwBuffLen);
			lpwszMessageXMLEncoded = (LPWSTR)mir_calloc((dwBuffLen*sizeof(WCHAR)));
			if (lpszMessageUTF && lpwszMessageXMLEncoded) {
				InternetTimeGetCurrentTime(&itTime);
				InternetTimeGetString(&itTime, szTime, SIZEOF(szTime), NULL);
				CopyNumber(szPhone, plpsFrom->lpszData, plpsFrom->dwSize);
				mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), NULL);

				EncodeXML(lpwszMessage, dwMessageSize, lpwszMessageXMLEncoded, dwBuffLen, &dwMessageXMLEncodedSize);
				WideCharToMultiByte(CP_UTF8, 0, lpwszMessageXMLEncoded, dwMessageXMLEncodedSize, lpszMessageUTF, dwBuffLen, NULL, NULL);
				lpszBuff = (LPSTR)lpwszMessageXMLEncoded;

				if (dwFlags & MESSAGE_SMS_DELIVERY_REPORT) {
					dwBuffLen = mir_snprintf(lpszBuff, (dwBuffLen*sizeof(WCHAR)), "<sms_delivery_receipt><message_id>%s-1-1955988055-%s</message_id><destination>%s</destination><delivered>No</delivered><submition_time>%s</submition_time><error_code>0</error_code><error><id>15</id><params><param>%s</param></params></error></sms_delivery_receipt>", szEMail, szPhone, szPhone, szTime, lpszMessageUTF);
					ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)lpszBuff);
				}
				else { // new sms
					dwBuffLen = mir_snprintf(lpszBuff, (dwBuffLen*sizeof(WCHAR)), "<sms_message><source>Mail.ru</source><destination_UIN>%s</destination_UIN><sender>%s</sender><senders_network>Mail.ru</senders_network><text>%s</text><time>%s</time></sms_message>", szEMail, szPhone, lpszMessageUTF, szTime);
					ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_SUCCESS, (HANDLE)0, (LPARAM)lpszBuff);
				}
			}
			else dwRetErrorCode = GetLastError();

			mir_free(lpwszMessageXMLEncoded);
			mir_free(lpszMessageUTF);
		}
		else {
			HANDLE hContact = MraHContactFromEmail(plpsFrom->lpszData, plpsFrom->dwSize, TRUE, TRUE, &bAdded);
			if (bAdded)
				MraUpdateContactInfo(hContact);

			// user typing
			if (dwFlags & MESSAGE_FLAG_NOTIFY)
				CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, MAILRU_CONTACTISTYPING_TIMEOUT);
			else { // text/contact/auth // typing OFF
				CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, PROTOTYPE_CONTACTTYPING_OFF);

				if (dwFlags & MESSAGE_FLAG_MULTICHAT) {
					LPBYTE lpbMultiChatData, lpbDataCurrent;
					size_t dwMultiChatDataSize;
					DWORD dwMultiChatEventType;
					MRA_LPS lpsEMailInMultiChat, lpsString, lpsMultichatName;

					lpbMultiChatData = (LPBYTE)plpsMultiChatData->lpszData;
					dwMultiChatDataSize = plpsMultiChatData->dwSize;
					lpbDataCurrent = lpbMultiChatData;

					dwMultiChatEventType = GetUL(&lpbDataCurrent);// type
					GetLPS(lpbMultiChatData, dwMultiChatDataSize, &lpbDataCurrent, &lpsMultichatName);// multichat_name
					GetLPS(lpbMultiChatData, dwMultiChatDataSize, &lpbDataCurrent, &lpsEMailInMultiChat);

					switch (dwMultiChatEventType) {
					case MULTICHAT_MESSAGE:
						MraChatSessionMessageAdd(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize, lpwszMessage, dwMessageSize, dwTime);// LPS sender
						break;
					case MULTICHAT_ADD_MEMBERS:
						MraChatSessionMembersAdd(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize, dwTime);// LPS sender
						GetLPS(lpbMultiChatData, dwMultiChatDataSize, &lpbDataCurrent, &lpsString);// CLPS members
						MraChatSessionSetIviter(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize);
					case MULTICHAT_MEMBERS:
						{
							LPBYTE lpbMultiChatDataLocal, lpbDataCurrentLocal;
							size_t i, dwMultiChatMembersCount, dwMultiChatDataLocalSize;

							if (dwMultiChatEventType == MULTICHAT_MEMBERS) lpsString = lpsEMailInMultiChat;

							lpbMultiChatDataLocal = (LPBYTE)lpsString.lpszData;
							dwMultiChatDataLocalSize = lpsString.dwSize;
							lpbDataCurrentLocal = lpbMultiChatDataLocal;

							dwMultiChatMembersCount = GetUL(&lpbDataCurrentLocal);// count
							for (i = 0; i < dwMultiChatMembersCount; i++)
								if (GetLPS(lpbMultiChatDataLocal, dwMultiChatDataLocalSize, &lpbDataCurrentLocal, &lpsString) == NO_ERROR)
									MraChatSessionJoinUser(hContact, lpsString.lpszData, lpsString.dwSize, ((dwMultiChatEventType == MULTICHAT_MEMBERS)? 0:dwTime));

							if (dwMultiChatEventType == MULTICHAT_MEMBERS) {
								GetLPS(lpbMultiChatData, dwMultiChatDataSize, &lpbDataCurrent, &lpsEMailInMultiChat);// [ LPS owner ]
								MraChatSessionSetOwner(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize);
							}
						}
						break;
					case MULTICHAT_ATTACHED:
						MraChatSessionJoinUser(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize, dwTime);// LPS member
						break;
					case MULTICHAT_DETACHED:
						MraChatSessionLeftUser(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize, dwTime);// LPS member
						break;
					case MULTICHAT_INVITE:
						MraChatSessionInvite(hContact, lpsEMailInMultiChat.lpszData, lpsEMailInMultiChat.dwSize, dwTime);// LPS sender
						MraAddContactW(hContact, (CONTACT_FLAG_VISIBLE|CONTACT_FLAG_MULTICHAT|CONTACT_FLAG_UNICODE_NAME), -1, plpsFrom->lpszData, plpsFrom->dwSize, lpsMultichatName.lpwszData, (lpsMultichatName.dwSize/sizeof(WCHAR)), NULL, 0, NULL, 0, 0);
						break;
					default:
						DebugBreak();
						break;
					}
				}
				else if (dwFlags & MESSAGE_FLAG_AUTHORIZE) { // auth request
					BYTE btBuff[BUFF_SIZE_BLOB];
					BOOL bAutoGrantAuth = FALSE;

					if ( IsEMailChatAgent(plpsFrom->lpszData, plpsFrom->dwSize))
						bAutoGrantAuth = FALSE;
					else {
						// temporary contact
						if (db_get_b(hContact, "CList", "NotOnList", 0)) {
							if (getByte(NULL, "AutoAuthGrandNewUsers", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS))
								bAutoGrantAuth = TRUE;
						}
						else if (getByte(NULL, "AutoAuthGrandUsersInCList", MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST))
							bAutoGrantAuth = TRUE;
					}

					if (bAdded)
						db_set_b(hContact, "CList", "Hidden", 1);

					if (bAutoGrantAuth) { // auto grant auth
						DBEVENTINFO dbei = { sizeof(dbei) };
						dbei.szModule = m_szModuleName;
						dbei.timestamp = _time32(NULL);
						dbei.flags = DBEF_READ;
						dbei.eventType = EVENTTYPE_AUTHREQUEST;
						dbei.pBlob = (PBYTE)btBuff;
						CreateBlobFromContact(hContact, lpwszMessage, dwMessageSize, btBuff, SIZEOF(btBuff), (size_t*)&dbei.cbBlob);
						db_event_add(0, &dbei);
						MraAuthorize(plpsFrom->lpszData, plpsFrom->dwSize);
					}
					else {
						pre.szMessage = (LPSTR)btBuff;
						CreateBlobFromContact(hContact, lpwszMessage, dwMessageSize, btBuff, SIZEOF(btBuff), (size_t*)&pre.lParam);
						ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
					}
				}
				else {
					db_unset(hContact, "CList", "Hidden");

					if (dwFlags & MESSAGE_FLAG_CONTACT) { // contacts received
						LPBYTE lpbBuffer, lpbBufferCurPos;

						lpbBuffer = (LPBYTE)mir_calloc((dwMessageSize+MAX_PATH));
						if (lpbBuffer) {
							pre.flags = 0;
							pre.szMessage = (LPSTR)lpbBuffer;
							pre.lParam = WideCharToMultiByte(MRA_CODE_PAGE, 0, lpwszMessage, dwMessageSize, (LPSTR)lpbBuffer, (dwMessageSize+MAX_PATH), NULL, NULL);

							lpbBufferCurPos = lpbBuffer;
							while (TRUE) { // цикл замены ; на 0
								lpbBufferCurPos = (LPBYTE)MemoryFindByte((lpbBufferCurPos-lpbBuffer), lpbBuffer, pre.lParam, ';');
								if (!lpbBufferCurPos)
									break;

								// found
								(*lpbBufferCurPos) = 0;
								lpbBufferCurPos++;
							}
							ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&pre);
							mir_free(lpbBuffer);
						}
						else dwRetErrorCode = GetLastError();
					}
					else if (dwFlags & MESSAGE_FLAG_ALARM) { // alarm
						if (heNudgeReceived)
							NotifyEventHooks(heNudgeReceived, (WPARAM)hContact, NULL);
						else {
							pre.flags = 0;
							pre.szMessage = (LPSTR)TranslateTS(MRA_ALARM_MESSAGE);
							ProtoChainRecvMsg(hContact, &pre);
						}
					}
					else { // standart message// flash animation
						// пишем в ANSI, всё равно RTF
						if ((dwFlags & MESSAGE_FLAG_RTF) && (dwFlags & MESSAGE_FLAG_FLASH) == 0 && lpszMessageExt && dwMessageExtSize && getByte(NULL, "RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE)) {
							pre.flags = 0;
							pre.szMessage = lpszMessageExt;
							ProtoChainRecvMsg(hContact, &pre);
						}
						else {
							// some plugins can change pre.szMessage pointer and we failed to free it
							LPSTR lpszMessageUTF = (LPSTR)mir_calloc(((dwMessageSize+MAX_PATH)*sizeof(WCHAR)));
							if (lpszMessageUTF) {
								pre.szMessage = lpszMessageUTF;
								pre.flags = PREF_UTF;
								WideCharToMultiByte(CP_UTF8, 0, lpwszMessage, dwMessageSize, lpszMessageUTF, ((dwMessageSize+MAX_PATH)*sizeof(WCHAR)), NULL, NULL);
								ProtoChainRecvMsg(hContact, &pre);
								mir_free(lpszMessageUTF);
							}
							else dwRetErrorCode = GetLastError();
						}

						if (dwFlags & MESSAGE_FLAG_SYSTEM)
							MraPopupShowW(hContact, MRA_POPUP_TYPE_INFORMATION, 0, TranslateW(L"Mail.ru System notify"), (LPWSTR)pre.szMessage);
					}
				}
			}
		}
	}
	else dwRetErrorCode = ERROR_ACCESS_DENIED;

	if (lpwszMessage != plpsText->lpwszData && lpwszMessage != (LPWSTR)lpszMessageExt)
		mir_free(lpwszMessage);
	mir_free(lpszMessageExt);

	return dwRetErrorCode;
}

DWORD GetMraXStatusIDFromMraUriStatus(LPSTR lpszStatusUri, size_t dwStatusUriSize)
{
	if (lpszStatusUri)
		for (size_t i = 0; lpcszStatusUri[i]; i++)
			if ( !_strnicmp(lpcszStatusUri[i], lpszStatusUri, dwStatusUriSize))
				return i;

	return MRA_XSTATUS_UNKNOWN;
}

DWORD GetMraStatusFromMiradaStatus(DWORD dwMirandaStatus, DWORD dwXStatusMir, DWORD *pdwXStatusMra)
{
	if ( IsXStatusValid(dwXStatusMir)) {
		if (pdwXStatusMra)
			*pdwXStatusMra = (dwXStatusMir+MRA_XSTATUS_INDEX_OFFSET-1);
		return STATUS_USER_DEFINED;
	}

	switch (dwMirandaStatus) {
	case ID_STATUS_OFFLINE:
		if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_OFFLINE;
		return STATUS_OFFLINE;

	case ID_STATUS_ONLINE:
		if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_ONLINE;
		return STATUS_ONLINE;

	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_AWAY;
		return STATUS_AWAY;

	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_DND;
		return STATUS_USER_DEFINED;

	case ID_STATUS_FREECHAT:
		if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_CHAT;
		return STATUS_USER_DEFINED;

	case ID_STATUS_INVISIBLE:
		if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_INVISIBLE;
		return (STATUS_ONLINE|STATUS_FLAG_INVISIBLE);
	}

	if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_OFFLINE;
	return STATUS_OFFLINE;
}


DWORD GetMiradaStatusFromMraStatus(DWORD dwMraStatus, DWORD dwXStatusMra, DWORD *pdwXStatusMir)
{
	if (pdwXStatusMir) *pdwXStatusMir = 0;

	switch (dwMraStatus) {
	case STATUS_OFFLINE:        return ID_STATUS_OFFLINE;
	case STATUS_ONLINE:         return ID_STATUS_ONLINE;
	case STATUS_AWAY:           return ID_STATUS_AWAY;
	case STATUS_UNDETERMINATED: return ID_STATUS_OFFLINE;
	case STATUS_USER_DEFINED:
		switch (dwXStatusMra) {
		case MRA_XSTATUS_DND:     return ID_STATUS_DND;
		case MRA_XSTATUS_CHAT:    return ID_STATUS_FREECHAT;
		case MRA_XSTATUS_UNKNOWN:
			if (pdwXStatusMir) *pdwXStatusMir = MRA_MIR_XSTATUS_UNKNOWN;
			return ID_STATUS_ONLINE;
		}
		if (pdwXStatusMir) *pdwXStatusMir = dwXStatusMra-MRA_XSTATUS_INDEX_OFFSET+1;
		return ID_STATUS_ONLINE;
	}

	if (dwMraStatus & STATUS_FLAG_INVISIBLE)
		return ID_STATUS_INVISIBLE;

	return ID_STATUS_OFFLINE;
}

DWORD GetUL(LPBYTE *plpData)
{
	DWORD dwRet = *(DWORD*)(*plpData);
	plpData[0] += sizeof(DWORD);
	return dwRet;
}

DWORDLONG GetUIDL(LPBYTE *plpData)
{
	DWORDLONG dwRet = *(DWORDLONG*)(*plpData);
	plpData[0] += sizeof(DWORDLONG);
	return dwRet;
}

MRA_GUID GetGUID(LPBYTE *plpData)
{
	MRA_GUID guidRet = *(MRA_GUID*)(*plpData);
	plpData[0] += sizeof(MRA_GUID);
	return guidRet;
}

DWORD GetLPS(LPBYTE lpbData, DWORD dwDataSize, LPBYTE *plpCurrentData, MRA_LPS *plpsString)
{
	LPBYTE lpbDataEnd = (lpbData+dwDataSize);

	// хотябы длинна данных есть
	if (lpbDataEnd >= ((*plpCurrentData)+sizeof(DWORD))) {
		// все длинна данных в буфере равна или меньше размера буфера
		if (lpbDataEnd >= ((*plpCurrentData)+sizeof(DWORD)+(*(DWORD*)(*plpCurrentData)))) {
			plpsString->dwSize = (*(DWORD*)(*plpCurrentData));
			plpsString->lpszData = (LPSTR)((*plpCurrentData)+sizeof(DWORD));
			(*plpCurrentData) += (sizeof(DWORD)+plpsString->dwSize);
			return NO_ERROR;
		}
	}

	plpsString->dwSize = 0;
	plpsString->lpszData = NULL;
	return ERROR_INVALID_USER_BUFFER;
}
