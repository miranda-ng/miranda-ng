#include "Mra.h"
#include "MraOfflineMsg.h"
#include "MraRTFMsg.h"
#include "MraPlaces.h"

DWORD CMraProto::StartConnect()
{
	if (m_bShutdown)
		return ERROR_OPERATION_ABORTED;

	// поток ещё/уже не работал, поставили статус что работает и запускаем
	if (InterlockedCompareExchange((volatile LONG*)&m_dwThreadWorkerRunning, TRUE, FALSE))
		return 0;

	CMStringA szEmail;
	mraGetStringA(NULL, "e-mail", szEmail);

	CMStringA szPass;
	if (szEmail.GetLength() <= 5)
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING, 0, TranslateT("Please, setup e-mail in options"));
	else if (!GetPassDB(szPass))
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING, 0, TranslateT("Please, setup password in options"));
	else {
		InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());
		if (INVALID_HANDLE_VALUE != ForkThreadEx(&CMraProto::MraThreadProc, NULL, 0))
			return 0; /* OK. */
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_ERROR, 0, TranslateT("Thread creation failure"));
	}
	InterlockedExchange((volatile LONG*)&m_dwThreadWorkerRunning, FALSE);
	return ERROR_OPERATION_ABORTED;
}

void CMraProto::MraThreadProc(LPVOID lpParameter)
{
	DWORD dwRetErrorCode = NO_ERROR;

	BOOL bConnected = FALSE;
	CMStringA szHost;
	DWORD dwConnectReTryCount, dwCurConnectReTryCount;

	SleepEx(100, FALSE);// to prevent high CPU load by some status plugins like allwaysonline

	dwConnectReTryCount = getDword("ConnectReTryCountMRIM", MRA_DEFAULT_CONN_RETRY_COUNT_MRIM);

	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof(nloc);
	nloc.flags = NLOCF_V2;
	nloc.timeout = getDword("TimeOutConnectMRIM", MRA_DEFAULT_TIMEOUT_CONN_MRIM);
	if (nloc.timeout < MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout > MRA_TIMEOUT_CONN_MAX) nloc.timeout = MRA_TIMEOUT_CONN_MAX;

	InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());
	if (MraGetNLBData(szHost, &nloc.wPort) == NO_ERROR) {
		nloc.szHost = szHost;
		//nloc.szHost = "217.69.141.245";
		//nloc.wPort = 443;
		//nloc.flags |= NLOCF_SSL;
		dwCurConnectReTryCount = dwConnectReTryCount;
		do {
			InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());
			m_hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
		}
		while (--dwCurConnectReTryCount && m_hConnection == NULL);

		if (m_hConnection)
			bConnected = TRUE;
	}

	if (bConnected == FALSE)
	if (getByte("NLBFailDirectConnect", MRA_DEFAULT_NLB_FAIL_DIRECT_CONNECT)) {
		if (IsHTTPSProxyUsed(m_hNetlibUser))
			nloc.wPort = MRA_SERVER_PORT_HTTPS;
		else {
			nloc.wPort = getWord("ServerPort", MRA_DEFAULT_SERVER_PORT);
			if (nloc.wPort == MRA_SERVER_PORT_STANDART_NLB) nloc.wPort = MRA_SERVER_PORT_STANDART;
		}

		for (DWORD i = 1; (i < MRA_MAX_MRIM_SERVER && m_iStatus != ID_STATUS_OFFLINE); i++) {
			szHost.Format("mrim%lu.mail.ru", i);

			dwCurConnectReTryCount = dwConnectReTryCount;
			do {
				InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());
				m_hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
			}
				while (--dwCurConnectReTryCount && m_hConnection == NULL);

			if (m_hConnection) {
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

	MraMPopSessionQueueFlush(hMPopSessionQueue);
	NETLIB_CLOSEHANDLE(m_hConnection);
	dwCMDNum = 0;

	InterlockedExchange((volatile LONG*)&m_dwThreadWorkerRunning, FALSE);
	SetStatus(ID_STATUS_OFFLINE);
}

DWORD CMraProto::MraGetNLBData(CMStringA &szHost, WORD *pwPort)
{
	DWORD dwRetErrorCode;

	BOOL bContinue = TRUE;
	BYTE btBuff[MAX_PATH];
	DWORD dwConnectReTryCount, dwCurConnectReTryCount;
	LPSTR lpszPort;
	size_t dwBytesReceived, dwRcvBuffSizeUsed = 0;
	NETLIBSELECT nls = { 0 };
	NETLIBOPENCONNECTION nloc = { 0 };

	dwConnectReTryCount = getDword("ConnectReTryCountNLB", MRA_DEFAULT_CONN_RETRY_COUNT_NLB);

	nloc.cbSize = sizeof(nloc);
	nloc.flags = NLOCF_V2;
	if (mraGetStringA(NULL, "Server", szHost))
		nloc.szHost = szHost;
	else
		nloc.szHost = MRA_DEFAULT_SERVER;

	if (IsHTTPSProxyUsed(m_hNetlibUser))
		nloc.wPort = MRA_SERVER_PORT_HTTPS;
	else
		nloc.wPort = getWord("ServerPort", MRA_DEFAULT_SERVER_PORT);

	nloc.timeout = getDword("TimeOutConnectNLB", MRA_DEFAULT_TIMEOUT_CONN_NLB);
	if (nloc.timeout < MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout > MRA_TIMEOUT_CONN_MAX) nloc.timeout = MRA_TIMEOUT_CONN_MAX;

	dwCurConnectReTryCount = dwConnectReTryCount;
	do {
		InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());
		nls.hReadConns[0] = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
	}
		while (--dwCurConnectReTryCount && nls.hReadConns[0] == NULL);

	if (nls.hReadConns[0]) {
		nls.cbSize = sizeof(nls);
		nls.dwTimeout = 1000 * getDword("TimeOutReceiveNLB", MRA_DEFAULT_TIMEOUT_RECV_NLB);
		InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());

		while (m_iStatus != ID_STATUS_OFFLINE && bContinue) {
			switch (CallService(MS_NETLIB_SELECT, 0, (LPARAM)&nls)) {
			case SOCKET_ERROR:
			case 0:// Time out
				bContinue = FALSE;
				break;
			case 1:
				dwBytesReceived = Netlib_Recv(nls.hReadConns[0], (LPSTR)(btBuff + dwRcvBuffSizeUsed), (SIZEOF(btBuff) - dwRcvBuffSizeUsed), 0);
				if (dwBytesReceived && dwBytesReceived != SOCKET_ERROR)
					dwRcvBuffSizeUsed += dwBytesReceived;
				else
					bContinue = FALSE;
				break;
			}
			InterlockedExchange((volatile LONG*)&m_dwThreadWorkerLastPingTime, GetTickCount());
		}
		Netlib_CloseHandle(nls.hReadConns[0]);

		if (dwRcvBuffSizeUsed) {
			lpszPort = (LPSTR)MemoryFindByte(0, btBuff, dwRcvBuffSizeUsed, ':');
			if (lpszPort) {
				(*lpszPort) = 0;
				lpszPort++;

				szHost = (LPSTR)btBuff;
				if (pwPort) (*pwPort) = (WORD)StrToUNum32(lpszPort, (dwRcvBuffSizeUsed - (lpszPort - (LPSTR)btBuff)));
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

	bool bContinue = true;
	DWORD dwDataCurrentBuffSize, dwDataCurrentBuffSizeUsed;
	size_t dwRcvBuffSize = BUFF_SIZE_RCV, dwRcvBuffSizeUsed = 0, dwDataCurrentBuffOffset = 0;
	LPBYTE lpbBufferRcv;
	mrim_packet_header_t *pmaHeader;

	NETLIBSELECT nls = { sizeof(nls) };
	nls.dwTimeout = NETLIB_SELECT_TIMEOUT;
	nls.hReadConns[0] = m_hConnection;

	lpbBufferRcv = (LPBYTE)mir_calloc(dwRcvBuffSize);

	m_dwNextPingSendTickTime = m_dwPingPeriod = MAXDWORD;
	dwCMDNum = 0;
	MraSendCMD(MRIM_CS_HELLO, NULL, 0);
	m_dwThreadWorkerLastPingTime = GetTickCount();
	while (m_iStatus != ID_STATUS_OFFLINE && bContinue) {
		DWORD dwSelectRet = CallService(MS_NETLIB_SELECT, 0, (LPARAM)&nls);
		if (SOCKET_ERROR == dwSelectRet) {
			if (m_iStatus != ID_STATUS_OFFLINE) {
				dwRetErrorCode = GetLastError();
				ShowFormattedErrorMessage(L"Disconnected, socket error", dwRetErrorCode);
			}
			break;
		}
		// Time out or normal
		m_dwThreadWorkerLastPingTime = GetTickCount();
		/* Server ping. */
		if (m_dwNextPingSendTickTime <= m_dwThreadWorkerLastPingTime) {
			m_dwNextPingSendTickTime = (m_dwThreadWorkerLastPingTime + (m_dwPingPeriod * 1000));
			MraSendCMD(MRIM_CS_PING, NULL, 0);
		}
		{ /* Remove old items from send queue. */
			DWORD dwCMDNum, dwFlags, dwAckType;
			MCONTACT hContact;
			LPBYTE lpbData;
			size_t dwDataSize;
			while (!MraSendQueueFindOlderThan(hSendQueueHandle, SEND_QUEUE_TIMEOUT, &dwCMDNum, &dwFlags, &hContact, &dwAckType, &lpbData, &dwDataSize)) {
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
					ProtoBroadcastAck(NULL, dwAckType, ACKRESULT_FAILED, (HANDLE)dwCMDNum, 0);
					mir_free(lpbData);
					break;
				}
				MraSendQueueFree(hSendQueueHandle, dwCMDNum);
			}
		}

		if (dwSelectRet == 0) // Time out
			continue;

		// expand receive buffer dynamically
		if ((dwRcvBuffSize - dwRcvBuffSizeUsed) < BUFF_SIZE_RCV_MIN_FREE) {
			dwRcvBuffSize += BUFF_SIZE_RCV;
			lpbBufferRcv = (LPBYTE)mir_realloc(lpbBufferRcv, dwRcvBuffSize);
		}

		DWORD dwBytesReceived = Netlib_Recv(nls.hReadConns[0], (LPSTR)(lpbBufferRcv + dwRcvBuffSizeUsed), (dwRcvBuffSize - dwRcvBuffSizeUsed), 0);
		if ( !dwBytesReceived || dwBytesReceived == SOCKET_ERROR) { // disconnected
			if (m_iStatus != ID_STATUS_OFFLINE) {
				dwRetErrorCode = GetLastError();
				ShowFormattedErrorMessage(L"Disconnected, socket read error", dwRetErrorCode);
			}
			break;
		}

		dwRcvBuffSizeUsed += dwBytesReceived;
		while (TRUE) {
			dwDataCurrentBuffSize = (dwRcvBuffSize - dwDataCurrentBuffOffset);
			dwDataCurrentBuffSizeUsed = (dwRcvBuffSizeUsed - dwDataCurrentBuffOffset);
			pmaHeader = (mrim_packet_header_t*)(lpbBufferRcv + dwDataCurrentBuffOffset);

			// packet header received
			if (dwDataCurrentBuffSizeUsed < sizeof(mrim_packet_header_t)) { // packet to small, continue receiving
				debugLogW(L"Packet to small, continue receiving\n");
				memmove(lpbBufferRcv, (lpbBufferRcv + dwDataCurrentBuffOffset), dwDataCurrentBuffSizeUsed);
				dwRcvBuffSizeUsed = dwDataCurrentBuffSizeUsed;
				dwDataCurrentBuffOffset = 0;
				break;
			}
			if (pmaHeader->magic != CS_MAGIC) { // bad packet
				debugLogW(L"Bad packet\n");
				dwDataCurrentBuffOffset = 0;
				dwRcvBuffSizeUsed = 0;
				break;
			}
			// packet OK
			if ((dwDataCurrentBuffSizeUsed - sizeof(mrim_packet_header_t)) < pmaHeader->dlen) { // not all packet received, continue receiving
				if (dwDataCurrentBuffOffset) {
					memmove(lpbBufferRcv, (lpbBufferRcv + dwDataCurrentBuffOffset), dwDataCurrentBuffSizeUsed);
					dwRcvBuffSizeUsed = dwDataCurrentBuffSizeUsed;
					dwDataCurrentBuffOffset = 0;
				}
				debugLogW(L"Not all packet received, continue receiving\n");
				break;
			}
			// full packet received, may be more than one
			bContinue = MraCommandDispatcher(pmaHeader);

			// move pointer to next packet in buffer
			if (dwDataCurrentBuffSizeUsed - sizeof(mrim_packet_header_t) > pmaHeader->dlen)
				dwDataCurrentBuffOffset += sizeof(mrim_packet_header_t)+pmaHeader->dlen;
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
	}
	mir_free(lpbBufferRcv);

	return dwRetErrorCode;
}

//Подтверждение установки соединения// UL ## ping_period ## Ожидаемая частота подтверждения соединения (в секундах)
bool CMraProto::CmdHelloAck(BinBuffer &buf)
{
	buf >> m_dwPingPeriod;

	CMStringA szPass;
	if (!GetPassDB(szPass))
		return false;

	char szValueName[MAX_PATH];
	CMStringA szUserAgentFormatted, szEmail;
	CMStringW wszStatusTitle, wszStatusDesc;

	DWORD dwXStatusMir = m_iXStatus, dwXStatus;
	DWORD dwStatus = GetMraStatusFromMiradaStatus(m_iDesiredStatus, dwXStatusMir, &dwXStatus);
	if (IsXStatusValid(dwXStatusMir)) {// xstatuses
		mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatusMir);
		if (!mraGetStringW(NULL, szValueName, wszStatusTitle))
			wszStatusTitle = TranslateTS(lpcszXStatusNameDef[dwXStatusMir]);

		mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatusMir);
		mraGetStringW(NULL, szValueName, wszStatusDesc);
	}
	else wszStatusTitle = pcli->pfnGetStatusModeDescription(m_iDesiredStatus, 0);

	CMStringA szSelfVersionString = MraGetSelfVersionString();
	if (!mraGetStringA(NULL, "MirVerCustom", szUserAgentFormatted))
		szUserAgentFormatted.Format(
		"client=\"magent\" name=\"Miranda NG\" title=\"%s\" version=\"777.%lu.%lu.%lu\" build=\"%lu\" protocol=\"%lu.%lu\"",
		szSelfVersionString, __FILEVERSION_STRING, PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR);

	DWORD dwFutureFlags = (getByte("RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE) ? FEATURE_FLAG_RTF_MESSAGE : 0) | MRA_FEATURE_FLAGS;

	if (!mraGetStringA(NULL, "e-mail", szEmail))
		return false;

	MraLogin2W(szEmail, szPass, dwStatus, CMStringA(lpcszStatusUri[dwXStatus]), wszStatusTitle, wszStatusDesc, dwFutureFlags, szUserAgentFormatted, szSelfVersionString);
	return true;
}

// Successful authorization
bool CMraProto::CmdLoginAck()
{
	m_bLoggedIn = TRUE;
	m_dwNextPingSendTickTime = 0; // force send ping
	MraSendCMD(MRIM_CS_PING, NULL, 0);
	SetStatus(m_iDesiredStatus);
	MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, GAIF_FORCE, NULL, 0);
	return true;
}

// Unsuccessful authorization //LPS ## reason ## причина отказа
bool CMraProto::CmdLoginRejected(BinBuffer &buf)
{
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);

	CMStringA reason; buf >> reason;
	MraPopupShowW(NULL, MRA_POPUP_TYPE_ERROR, 0, TranslateT("Logon error: invalid login/password"), _A2T(reason.c_str()));
	return false;
}

// Message delivery
//LPS ## from ## Адрес отправителя
//LPS ## message ## текстовая версия сообщения
//LPS ## rtf-message ## форматированная версия сообщения
bool CMraProto::CmdMessageAck(BinBuffer &buf)
{
	DWORD dwMsgID, dwFlags;
	CMStringA szEmail, szText, szRTFText, szMultiChatData;
	buf >> dwMsgID >> dwFlags >> szEmail >> szText >> szRTFText;
	if (dwFlags & MESSAGE_FLAG_MULTICHAT)
		buf >> szMultiChatData; // LPS multichat_data

	// подтверждаем получение, только если удалось его обработать
	if (MraRecvCommand_Message((DWORD)_time32(NULL), dwFlags, szEmail, szText, szRTFText, szMultiChatData) == NO_ERROR)
	if ((dwFlags & MESSAGE_FLAG_NORECV) == 0)
		MraMessageRecv(szEmail, dwMsgID);
	return true;
}

bool CMraProto::CmdMessageStatus(ULONG seq, BinBuffer &buf)
{
	DWORD dwAckType, dwTemp = buf.getDword();
	MCONTACT hContact;
	if (!MraSendQueueFind(hSendQueueHandle, seq, NULL, &hContact, &dwAckType, NULL, NULL)) {
		switch (dwTemp) {
		case MESSAGE_DELIVERED:// Message delivered directly to user
			ProtoBroadcastAckAsync(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)seq, 0);
			break;//***deb возможны сбои из-за асинхронности тк там передаётся указатель
		case MESSAGE_REJECTED_NOUSER:// Message rejected - no such user
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)"Message rejected - no such user");
			break;
		case MESSAGE_REJECTED_INTERR:// Internal server error
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)"Internal server error");
			break;
		case MESSAGE_REJECTED_LIMIT_EXCEEDED:// Offline messages limit exceeded
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)"Offline messages limit exceeded");
			break;
		case MESSAGE_REJECTED_TOO_LARGE:// Message is too large
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)"Message is too large");
			break;
		case MESSAGE_REJECTED_DENY_OFFMSG:// User does not accept offline messages
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)"User does not accept offline messages");
			break;
		case MESSAGE_REJECTED_DENY_OFFFLSH:// User does not accept offline flash animation
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)"User does not accept offline flash animation");
			break;
		default:
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)CMStringA().Format("Undefined message delivery error, code: %lu", dwTemp));
			break;
		}
		MraSendQueueFree(hSendQueueHandle, seq);
	}
	// not found in queue
	else if (dwTemp != MESSAGE_DELIVERED)
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateT("MRIM_CS_MESSAGE_STATUS: not found in queue"));
	return true;
}

bool CMraProto::CmdUserInfo(BinBuffer &buf)
{
	CMStringA szString;
	CMStringW szStringW;
	while (!buf.eof()) {
		buf >> szString;
		if (szString == "MESSAGES.TOTAL") {
			buf >> szString;
			dwEmailMessagesTotal = atoi(szString);
		}
		else if (szString == "MESSAGES.UNREAD") {
			buf >> szString;
			m_dwEmailMessagesUnread = atoi(szString);
		}
		else if (szString == "MRIM.NICKNAME") {
			buf >> szStringW;
			mraSetStringW(NULL, "Nick", szStringW);
		}
		else if (szString == "client.endpoint") {
			buf >> szStringW;
			szString = szStringW;
			int lpszDelimiter = szString.Find(':');
			if (lpszDelimiter != -1) {
				CMStringA szAddr(szString, lpszDelimiter);
				setDword("IP", ntohl(inet_addr(szAddr.c_str())));
			}
		}
		else if (szString == "connect.xml") {
			debugLogA(szString);
			buf >> szStringW;
			debugLogW(szStringW);
		}
		else if (szString == "micblog.show_title") {
			debugLogA(szString);
			buf >> szString;
			debugLogW(szStringW);
		}
		else if (szString == "micblog.status.xml") {
			debugLogA(szString);
			buf >> szString;
			debugLogA(szString);
		}
		else if (szString == "micblog.status.id") {
			buf >> szStringW;
			DWORDLONG dwBlogStatusID = _ttoi64(szStringW);
			mraWriteContactSettingBlob(NULL, DBSETTING_BLOGSTATUSID, &dwBlogStatusID, sizeof(DWORDLONG));
		}
		else if (szString == "micblog.status.time") {
			buf >> szStringW;
			setDword(DBSETTING_BLOGSTATUSTIME, _ttoi(szStringW));
		}
		else if (szString == "micblog.status.text") {
			buf >> szStringW;
			mraSetStringW(NULL, DBSETTING_BLOGSTATUS, szStringW);
		}
		else if (szString == "HAS_MYMAIL" || szString == "mrim.status.open_search" || szString == "rb.target.cookie" ||
			szString == "show_web_history_link" || szString == "friends_suggest" || szString == "timestamp" ||
			szString == "trusted_update" || szString == "mrim.wp.dating") {
			debugLogA(szString);
			buf >> szStringW;
			debugLogW(szStringW);
		}
		else _CrtDbgBreak();
	}
	MraUpdateEmailStatus("", "", 0, 0, false);
	return true;
}

//Сообщение доставленное, пока пользователь не был подключен к сети
bool CMraProto::CmdOfflineMessageAck(BinBuffer &buf)
{
	CMStringA szEmail, szText, lpsRTFText, lpsMultiChatData, szString;
	DWORDLONG dwMsgUIDL;
	buf >> dwMsgUIDL >> szString;

	DWORD dwTime, dwFlags;
	if (MraOfflineMessageGet(szString, dwTime, dwFlags, szEmail, szText, lpsRTFText, lpsMultiChatData) == NO_ERROR) {
		DWORD dwTemp = MraRecvCommand_Message(dwTime, dwFlags, szEmail, szText, lpsRTFText, lpsMultiChatData);
		if (dwTemp == NO_ERROR || dwTemp == ERROR_ACCESS_DENIED)
			MraOfflineMessageDel(dwMsgUIDL);
		else
			ShowFormattedErrorMessage(_T("Offline message processing error, message will not deleted from server"), NO_ERROR);
	}
	else ShowFormattedErrorMessage(_T("Offline message processing error, message will not deleted from server"), NO_ERROR);

	return true;
}

// Auth confirmation
bool CMraProto::CmdAuthAck(BinBuffer &buf)
{
	CMStringA szEmail;
	buf >> szEmail;

	BOOL bAdded;
	MCONTACT hContact = MraHContactFromEmail(szEmail, TRUE, TRUE, &bAdded);
	if (bAdded)
		MraUpdateContactInfo(hContact);

	if (IsEMailChatAgent(szEmail) == FALSE) {
		CMStringA szBuff = CreateBlobFromContact(hContact, _T(""));

		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.szModule = m_szModuleName;
		dbei.timestamp = (DWORD)_time32(NULL);
		dbei.eventType = EVENTTYPE_ADDED;
		dbei.cbBlob = szBuff.GetLength();
		dbei.pBlob = (PBYTE)szBuff.GetString();
		db_event_add(0, &dbei);
	}

	DWORD dwTemp;
	GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwTemp, NULL, NULL, NULL, NULL);
	dwTemp &= ~CONTACT_INTFLAG_NOT_AUTHORIZED;
	SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, SCBIF_SERVER_FLAG, 0, 0, 0, dwTemp, 0, 0, 0, 0);
	setDword(hContact, "HooksLocked", TRUE);
	db_unset(hContact, "CList", "NotOnList");
	setDword(hContact, "HooksLocked", FALSE);
	return true;
}

// Web auth key
bool CMraProto::CmdPopSession(BinBuffer &buf)
{
	DWORD dwTemp = buf.getDword();
	if (dwTemp) {
		CMStringA szString; buf >> szString;
		if (NO_ERROR == MraMPopSessionQueueSetNewMPopKey(hMPopSessionQueue, szString)) {
			MraMPopSessionQueueStart(hMPopSessionQueue);
			return true;
		}
	}
	//error
	MraPopupShowFromAgentW(MRA_POPUP_TYPE_WARNING, 0, TranslateT("Server error: can't get MPOP key for web authorize"));
	MraMPopSessionQueueFlush(hMPopSessionQueue);

	return true;
}

bool CMraProto::CmdFileTransfer(BinBuffer &buf)
{
	DWORD dwIDRequest, dwFilesTotalSize, dwTemp;
	CMStringA szFiles, szEmail, szAddresses;
	CMStringW wszFilesW;

	buf >> szEmail >> dwIDRequest >> dwFilesTotalSize >> dwTemp;
	if (dwTemp) {
		buf >> szFiles >> dwTemp;
		if (dwTemp) { // LPS DESCRIPTION
			buf >> dwTemp >> wszFilesW;
			_ASSERTE(dwTemp != 1);
		}
		buf >> szAddresses;
	}

	BOOL bAdded = FALSE;
	MCONTACT hContact = MraHContactFromEmail(szEmail, TRUE, TRUE, &bAdded);
	if (bAdded)
		MraUpdateContactInfo(hContact);

	if (wszFilesW.IsEmpty())
		wszFilesW = szFiles;

	if (!wszFilesW.IsEmpty())
		MraFilesQueueAddReceive(hFilesQueueHandle, 0, hContact, dwIDRequest, wszFilesW, szAddresses);
	return true;
}

bool CMraProto::CmdFileTransferAck(BinBuffer &buf)
{
	CMStringA szEmail, szString;
	DWORD dwAckType, dwTemp;
	buf >> dwAckType >> szEmail >> dwTemp >> szString;

	switch (dwAckType) {
	case FILE_TRANSFER_STATUS_OK:// игнорируем, мы и так уже слушаем порт(ждём), то что кто то согласился ничего не меняет
		//hContact = MraHContactFromEmail(szEmail.lpszData, szEmail.dwSize, TRUE, TRUE, NULL);
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
		MraFilesQueueSendMirror(hFilesQueueHandle, dwTemp, szString);
		break;
	default:// ## unknown error
		TCHAR szBuff[1024];
		mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateT("MRIM_CS_FILE_TRANSFER_ACK: unknown error, code: %lu"), dwAckType);
		ShowFormattedErrorMessage(szBuff, NO_ERROR);
		break;
	}
	return true;
}

// Смена статуса другого пользователя
bool CMraProto::CmdUserStatus(BinBuffer &buf)
{
	DWORD dwStatus, dwXStatus, dwFutureFlags;
	CMStringA szSpecStatusUri, szUserAgentFormatted, szEmail;
	CMStringW szStatusTitle, szStatusDesc;
	buf >> dwStatus >> szSpecStatusUri >> szStatusTitle >> szStatusDesc >> szEmail >> dwFutureFlags >> szUserAgentFormatted;

	BOOL bAdded;
	if (MCONTACT hContact = MraHContactFromEmail(szEmail, TRUE, TRUE, &bAdded)) {
		if (bAdded)
			MraUpdateContactInfo(hContact);

		DWORD dwTemp = GetMirandaStatusFromMraStatus(dwStatus, GetMraXStatusIDFromMraUriStatus(szSpecStatusUri), &dwXStatus);

		MraContactCapabilitiesSet(hContact, dwFutureFlags);
		setByte(hContact, DBSETTING_XSTATUSID, (BYTE)dwXStatus);
		if (dwXStatus) {
			mraSetStringW(hContact, DBSETTING_XSTATUSNAME, szStatusTitle);
			mraSetStringW(hContact, DBSETTING_XSTATUSMSG, szStatusDesc);
		}
		else {
			delSetting(hContact, DBSETTING_XSTATUSNAME);
			delSetting(hContact, DBSETTING_XSTATUSMSG);
		}


		if (dwTemp != ID_STATUS_OFFLINE) { // пишем клиента только если юзер не отключён, иначе не затираем старое
			if (!szUserAgentFormatted.IsEmpty()) {
				if (getByte("MirVerRaw", MRA_DEFAULT_MIRVER_RAW) == FALSE)
					szUserAgentFormatted = MraGetVersionStringFromFormatted(szUserAgentFormatted);
			}
			else szUserAgentFormatted = MIRVER_UNKNOWN;
			mraSetStringA(hContact, "MirVer", szUserAgentFormatted);
		}

		if (dwTemp == MraGetContactStatus(hContact)) {// меняем шило на шило, подозрительно? ;)
			if (dwTemp == ID_STATUS_OFFLINE) { // was/now invisible
				CMStringW szEmail, szBuff;
				mraGetStringW(hContact, "e-mail", szEmail);
				szBuff.Format(L"%s <%s> - %s", GetContactNameW(hContact), szEmail, TranslateT("invisible status changed"));
				MraPopupShowFromContactW(hContact, MRA_POPUP_TYPE_INFORMATION, 0, szBuff);

				MraSetContactStatus(hContact, ID_STATUS_INVISIBLE);
			}
		}
		MraSetContactStatus(hContact, dwTemp);
		SetExtraIcons(hContact);
	}
	return true;
}

bool CMraProto::CmdContactAck(int cmd, int seq, BinBuffer &buf)
{
	DWORD dwAckType; MCONTACT hContact;
	if (!MraSendQueueFind(hSendQueueHandle, seq, NULL, &hContact, &dwAckType, NULL, NULL)) {
		DWORD dwTemp = buf.getDword();
		switch (dwTemp) {
		case CONTACT_OPER_SUCCESS:// ## добавление произведено успешно
			if (cmd == MRIM_CS_ADD_CONTACT_ACK) {
				DWORD dwFlags = SCBIF_ID | SCBIF_SERVER_FLAG, dwGroupID = 0;
				ptrT grpName(db_get_tsa(hContact, "CList", "Group"));
				if (grpName) {
					dwFlags |= SCBIF_GROUP_ID;
					dwGroupID = MraMoveContactToGroup(hContact, -1, grpName);
				}
				SetContactBasicInfoW(hContact, 0, dwFlags, buf.getDword(), dwGroupID, 0, CONTACT_INTFLAG_NOT_AUTHORIZED, 0, 0, 0, 0);
			}
			break;
		case CONTACT_OPER_ERROR:// ## переданные данные были некорректны
			ShowFormattedErrorMessage(_T("Data been sent are invalid"), NO_ERROR);
			break;
		case CONTACT_OPER_INTERR:// ## при обработке запроса произошла внутренняя ошибка
			ShowFormattedErrorMessage(_T("Internal server error"), NO_ERROR);
			break;
		case CONTACT_OPER_NO_SUCH_USER:// ## добавляемого пользователя не существует в системе
			SetContactBasicInfoW(hContact, 0, SCBIF_SERVER_FLAG, 0, 0, 0, -1, 0, 0, 0, 0);
			ShowFormattedErrorMessage(_T("No such user to add"), NO_ERROR);
			break;
		case CONTACT_OPER_INVALID_INFO:// ## некорректное имя пользователя
			ShowFormattedErrorMessage(_T("Invalid user name"), NO_ERROR);
			break;
		case CONTACT_OPER_USER_EXISTS:// ## пользователь уже есть в контакт-листе
			ShowFormattedErrorMessage(_T("User already added"), NO_ERROR);
			break;
		case CONTACT_OPER_GROUP_LIMIT:// ## превышено максимально допустимое количество групп (20)
			ShowFormattedErrorMessage(_T("Group limit is 20"), NO_ERROR);
			break;
		default:// ## unknown error
			TCHAR szBuff[1024];
			mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateT("MRIM_CS_*_CONTACT_ACK: unknown server error, code: %lu"), dwTemp);
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, szBuff);
			break;
		}
		MraSendQueueFree(hSendQueueHandle, seq);
	}
	else MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateT("MRIM_CS_*_CONTACT_ACK: not found in queue"));
	return true;
}

bool CMraProto::CmdAnketaInfo(int seq, BinBuffer &buf)
{
	DWORD dwAckType, dwFlags; MCONTACT hContact;
	if (MraSendQueueFind(hSendQueueHandle, seq, &dwFlags, &hContact, &dwAckType, NULL, NULL)) {
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateT("MRIM_ANKETA_INFO: not found in queue"));
		return true;
	}

	switch (buf.getDword()) {
	case MRIM_ANKETA_INFO_STATUS_NOUSER:// не найдено ни одной подходящей записи
		SetContactBasicInfoW(hContact, 0, SCBIF_SERVER_FLAG, 0, 0, 0, -1, 0, 0, 0, 0);
	case MRIM_ANKETA_INFO_STATUS_DBERR:// ошибка базы данных
	case MRIM_ANKETA_INFO_STATUS_RATELIMERR:// слишком много запросов, поиск временно запрещен
		switch (dwAckType) {
		case ACKTYPE_GETINFO:
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_FAILED, (HANDLE)1, 0);
			break;
		case ACKTYPE_SEARCH:
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)seq, 0);
			break;
		}
		break;

	case MRIM_ANKETA_INFO_STATUS_OK:
		// поиск успешно завершен
		DWORD dwFieldsNum, dwMaxRows, dwServerTime;
		DWORD dwID, dwContactSeverFlags, dwStatus, dwXStatus;
		buf >> dwFieldsNum >> dwMaxRows >> dwServerTime;

		CMStringA *pmralpsFields = new CMStringA[dwFieldsNum];
		CMStringA val;
		CMStringW valW, StatusNameW, StatusMsgW;

		/* Default contact statuses in mail.ru format. */
		dwStatus = STATUS_OFFLINE;
		dwXStatus = MRA_XSTATUS_OFFLINE;

		// read headers name
		for (DWORD i = 0; i < dwFieldsNum; i++) {
			buf >> pmralpsFields[i];
			debugLogA("%s ", pmralpsFields[i]);
		}

		while (!buf.eof()) {
			// write to DB and exit loop
			if (dwAckType == ACKTYPE_GETINFO && hContact) {
				setDword(hContact, "InfoTS", (DWORD)_time32(NULL));
				//MRA_LPS mralpsUsernameValue;
				for (DWORD i = 0; i < dwFieldsNum; i++) {
					CMStringA &fld = pmralpsFields[i];
					if (fld == "Nickname") {
						buf >> valW;
						mraSetStringW(hContact, "Nick", valW);
					}
					else if (fld == "FirstName") {
						buf >> valW;
						mraSetStringW(hContact, "FirstName", valW);
					}
					else if (fld == "LastName") {
						buf >> valW;
						mraSetStringW(hContact, "LastName", valW);
					}
					else if (fld == "Sex") {
						buf >> val;
						switch (atoi(val)) {
						case 1:// мужской
							setByte(hContact, "Gender", 'M');
							break;
						case 2:// женский
							setByte(hContact, "Gender", 'F');
							break;
						default:// а фиг его знает
							delSetting(hContact, "Gender");
							break;
						}
					}
					else if (fld == "Birthday") {
						buf >> val;
						if (val.GetLength() > 9) {// calc "Age"
							SYSTEMTIME stTime = { 0 };
							stTime.wYear = (WORD)StrToUNum32(val.c_str(), 4);
							stTime.wMonth = (WORD)StrToUNum32(val.c_str() + 5, 2);
							stTime.wDay = (WORD)StrToUNum32(val.c_str() + 8, 2);
							setWord(hContact, "BirthYear", stTime.wYear);
							setByte(hContact, "BirthMonth", (BYTE)stTime.wMonth);
							setByte(hContact, "BirthDay", (BYTE)stTime.wDay);
							setWord(hContact, "Age", (WORD)GetYears(&stTime));
						}
						else {
							delSetting(hContact, "BirthYear");
							delSetting(hContact, "BirthMonth");
							delSetting(hContact, "BirthDay");
							delSetting(hContact, "Age");
						}
					}
					else if (fld == "City_id") {
						buf >> val;
						DWORD dwTemp = atoi(val);
						if (dwTemp) {
							for (size_t j = 0; mrapPlaces[j].lpszData; j++) {
								if (mrapPlaces[j].dwCityID == dwTemp) {
									mraSetStringW(hContact, "City", mrapPlaces[j].lpszData);
									break;
								}
							}
						}
						else delSetting(hContact, "City");
					}
					else if (fld == "Location") {
						buf >> valW;
						mraSetStringW(hContact, "About", valW);
					}
					else if (fld == "Country_id") {
						buf >> val;
						DWORD dwTemp = atoi(val);
						if (dwTemp) {
							for (size_t j = 0; mrapPlaces[j].lpszData; j++) {
								if (mrapPlaces[j].dwCountryID == dwTemp) {
									mraSetStringW(hContact, "Country", mrapPlaces[j].lpszData);
									break;
								}
							}
						}
						else delSetting(hContact, "Country");
					}
					else if (fld == "Phone") {
						delSetting(hContact, "Phone");
						delSetting(hContact, "Cellular");
						delSetting(hContact, "Fax");

						buf >> val;
						if (val.GetLength()) {
							int iStart = 0;
							CMStringA szPhone = val.Tokenize(",", iStart);
							if (iStart != -1) {
								mraSetStringA(hContact, "Phone", szPhone);
								szPhone = val.Tokenize(",", iStart);
							}
							if (iStart != -1) {
								mraSetStringA(hContact, "Cellular", szPhone);
								szPhone = val.Tokenize(",", iStart);
							}
							if (iStart != -1)
								mraSetStringA(hContact, "Fax", szPhone);
						}
					}
					else if (fld == "mrim_status") {
						buf >> val;
						if (val.GetLength())
							dwStatus = atoi(val);
					}
					else if (fld == "status_uri") {
						buf >> val;
						if (val.GetLength())
							dwXStatus = GetMraXStatusIDFromMraUriStatus(val);
					}
					else if (fld == "status_title") {
						buf >> StatusNameW;
					}
					else if (fld == "status_desc") {
						buf >> StatusMsgW;
					}
					else {// for DEBUG ONLY
						buf >> val;
						debugLogA("%s = %s\n", fld, val);
					}
				} /* for */
				// для авторизованного нам и так присылают правильный статус
				GetContactBasicInfoW(hContact, &dwID, NULL, NULL, &dwContactSeverFlags, NULL, NULL, NULL, NULL);
				if (dwID == -1 || (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED)) {
					/* Convert mail.ru statuses to miranda. */
					dwStatus = GetMirandaStatusFromMraStatus(dwStatus, dwXStatus, &dwXStatus);
					MraSetContactStatus(hContact, dwStatus);
					setByte(hContact, DBSETTING_XSTATUSID, (BYTE)dwXStatus);
					if (StatusNameW.GetLength())
						mraSetStringW(hContact, DBSETTING_XSTATUSNAME, StatusNameW);
					if (StatusMsgW.GetLength())
						mraSetStringW(hContact, DBSETTING_XSTATUSMSG, StatusMsgW);
				}
			}
			else if (dwAckType == ACKTYPE_SEARCH) {
				TCHAR szNick[MAX_EMAIL_LEN] = { 0 },
					szFirstName[MAX_EMAIL_LEN] = { 0 },
					szLastName[MAX_EMAIL_LEN] = { 0 },
					szEmail[MAX_EMAIL_LEN] = { 0 };
				CMStringA mralpsUsernameValue;
				PROTOSEARCHRESULT psr = { 0 };

				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.nick = szNick;
				psr.firstName = szFirstName;
				psr.lastName = szLastName;
				psr.email = szEmail;
				psr.id = szEmail;

				for (DWORD i = 0; i < dwFieldsNum; i++) {
					CMStringA &fld = pmralpsFields[i];
					if (fld == "Username") {
						buf >> val;
						mralpsUsernameValue = val;
					}
					else if (fld == "Domain") { // имя было уже задано ранее
						buf >> val;
						wcsncpy_s(szEmail, _A2T(mralpsUsernameValue + "@" + val), _TRUNCATE);
					}
					else if (fld == "Nickname") {
						buf >> valW;
						wcsncpy_s(szNick, valW, _TRUNCATE);
					}
					else if (fld == "FirstName") {
						buf >> valW;
						wcsncpy_s(szFirstName, valW, _TRUNCATE);
					}
					else if (fld == "LastName") {
						buf >> valW;
						wcsncpy_s(szLastName, valW, _TRUNCATE);
					}
					else buf >> val;
				}
				ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_DATA, (HANDLE)seq, (LPARAM)&psr);
			}
		}

		delete[] pmralpsFields;

		switch (dwAckType) {
		case ACKTYPE_GETINFO:
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)1, 0);
			break;
		case ACKTYPE_SEARCH:
		default:
			ProtoBroadcastAck(hContact, dwAckType, ACKRESULT_SUCCESS, (HANDLE)seq, 0);
			break;
		}
		break;
	}
	MraSendQueueFree(hSendQueueHandle, seq);
	return true;
}

bool CMraProto::CmdGame(BinBuffer &buf)
{
	MCONTACT hContact;
	CMStringA szEmail, szData;
	DWORD dwGameSessionID, dwGameMsg, dwGameMsgID, dwTemp;
	buf >> szEmail >> dwGameSessionID >> dwGameMsg >> dwGameMsgID >> dwTemp >> szData;

	switch (dwGameMsg) {
	case GAME_CONNECTION_INVITE:
		if (m_iStatus != ID_STATUS_INVISIBLE)
			MraGame(szEmail, dwGameSessionID, GAME_DECLINE, dwGameMsgID, szData);
		break;
	case GAME_CONNECTION_ACCEPT:
		break;
	case GAME_DECLINE:
		break;
	case GAME_INC_VERSION:
		break;
	case GAME_NO_SUCH_GAME:// user invisible
		if ((hContact = MraHContactFromEmail(szEmail, FALSE, TRUE, NULL)))
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
		TCHAR szBuff[1024];
		mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateT("MRIM_CS_GAME: unknown internal game message code: %lu"), dwGameMsg);
		MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, szBuff);
		break;
	}
	return true;
}

bool CMraProto::CmdClist2(BinBuffer &buf)
{
	DWORD dwTemp = buf.getDword();
	if (dwTemp == GET_CONTACTS_OK) { // received contact list
		m_groups.destroy();

		DWORD dwGroupsCount, dwContactFlag, dwGroupID, dwContactSeverFlags, dwStatus, dwXStatus, dwFutureFlags, dwBlogStatusTime;
		ULARGE_INTEGER dwBlogStatusID;
		CMStringA szGroupMask, szContactMask, szEmail, szString;
		CMStringA szCustomPhones, szSpecStatusUri, szUserAgentFormatted;
		CMStringW wszNick, wszString, wszGroupName, wszStatusTitle, wszStatusDesc, wszBlogStatus, wszBlogStatusMusic;
		buf >> dwGroupsCount >> szGroupMask >> szContactMask;

		int iGroupMode = getByte("GroupMode", 100);

		debugLogA("Groups: %s\n", szGroupMask);
		DWORD dwID = 0;
		for (DWORD i = 0; i < dwGroupsCount; i++) { //groups handle
			DWORD dwControlParam = 0, dwGroupFlags = 0;
			for (int j = 0; j < szGroupMask.GetLength(); j++) { //enumerating parameters
				switch (szGroupMask[j]) {
				case 's'://LPS
					buf >> wszString;
					break;
				case 'u'://UL
					buf >> dwTemp;
					break;
				}

				if (j == 0 && szGroupMask[j] == 'u') { // GroupFlags
					dwGroupFlags = dwTemp;
					dwControlParam++;
				}
				else if (j == 1 && szGroupMask[j] == 's') { // GroupName
					wszGroupName = wszString;
					dwControlParam++;
				}
			}

			// add/modify group
			if (dwControlParam > 1) { // все параметры правильно инициализированны!
				if (!(dwGroupFlags & CONTACT_FLAG_REMOVED)) {
					m_groups.insert(new MraGroupItem(dwID, dwGroupFlags, wszGroupName));
					Clist_CreateGroup(0, wszGroupName);
				}

				debugLogW(L"'%s', flags: %lu (", wszGroupName, dwGroupFlags);
				if (dwGroupFlags & CONTACT_FLAG_REMOVED)      debugLogA("CONTACT_FLAG_REMOVED, ");
				if (dwGroupFlags & CONTACT_FLAG_GROUP)        debugLogA("CONTACT_FLAG_GROUP, ");
				if (dwGroupFlags & CONTACT_FLAG_INVISIBLE)    debugLogA("CONTACT_FLAG_INVISIBLE, ");
				if (dwGroupFlags & CONTACT_FLAG_VISIBLE)      debugLogA("CONTACT_FLAG_VISIBLE, ");
				if (dwGroupFlags & CONTACT_FLAG_IGNORE)       debugLogA("CONTACT_FLAG_IGNORE, ");
				if (dwGroupFlags & CONTACT_FLAG_SHADOW)       debugLogA("CONTACT_FLAG_SHADOW, ");
				if (dwGroupFlags & CONTACT_FLAG_AUTHORIZED)   debugLogA("CONTACT_FLAG_AUTHORIZED, ");
				if (dwGroupFlags & CONTACT_FLAG_MULTICHAT)    debugLogA("CONTACT_FLAG_MULTICHAT, ");
				if (dwGroupFlags & CONTACT_FLAG_UNICODE_NAME) debugLogA("CONTACT_FLAG_UNICODE_NAME, ");
				if (dwGroupFlags & CONTACT_FLAG_PHONE)        debugLogA("CONTACT_FLAG_PHONE, ");
				debugLogA(")");
			}
			dwID++;
		}

		debugLogA("Contacts: %s\n", szContactMask);
		dwID = 20;
		while (!buf.eof()) {
			DWORD dwControlParam = 0;
			for (int j = 0; j < szContactMask.GetLength(); j++) { //enumerating parameters
				BYTE fieldType = szContactMask[j];
				if (fieldType == 'u')
					buf >> dwTemp;

				if (j == 0 && fieldType == 'u') { // Flags
					dwContactFlag = dwTemp;
					dwControlParam++;
				}
				else if (j == 1 && fieldType == 'u') { // Group id
					dwGroupID = dwTemp;
					dwControlParam++;
				}
				else if (j == 2 && fieldType == 's') { // Email
					buf >> szEmail;
					dwControlParam++;
				}
				else if (j == 3 && fieldType == 's') { // Nick
					buf >> wszNick;
					dwControlParam++;
				}
				else if (j == 4 && fieldType == 'u') { // Server flags
					dwContactSeverFlags = dwTemp;
					dwControlParam++;
				}
				else if (j == 5 && fieldType == 'u') { // Status
					dwStatus = dwTemp;
					dwControlParam++;
				}
				else if (j == 6 && fieldType == 's') { // Custom Phone number,
					buf >> szCustomPhones;
					dwControlParam++;
				}
				else if (j == 7 && fieldType == 's') { // spec_status_uri
					buf >> szSpecStatusUri;
					dwControlParam++;
				}
				else if (j == 8 && fieldType == 's') { // status_title
					buf >> wszStatusTitle;
					dwControlParam++;
				}
				else if (j == 9 && fieldType == 's') { // status_desc
					buf >> wszStatusDesc;
					dwControlParam++;
				}
				else if (j == 10 && fieldType == 'u') { // com_support (future flags)
					dwFutureFlags = dwTemp;
					dwControlParam++;
				}
				else if (j == 11 && fieldType == 's') { // user_agent (formated string)
					buf >> szUserAgentFormatted;
					dwControlParam++;
				}
				else if (j == 12 && fieldType == 'u') { // BlogStatusID
					dwBlogStatusID.LowPart = dwTemp;
					dwControlParam++;
				}
				else if (j == 13 && fieldType == 'u') { // BlogStatusID
					dwBlogStatusID.HighPart = dwTemp;
					dwControlParam++;
				}
				else if (j == 14 && fieldType == 'u') { // BlogStatusTime
					dwBlogStatusTime = dwTemp;
					dwControlParam++;
				}
				else if (j == 15 && fieldType == 's') { // BlogStatus
					buf >> wszBlogStatus;
					dwControlParam++;
				}
				else if (j == 16 && fieldType == 's') { // BlogStatusMusic
					buf >> wszBlogStatusMusic;
					dwControlParam++;
				}
				else if (j == 17 && fieldType == 's') { // BlogStatusSender // ignory
					buf >> szString;
					dwControlParam++;
				}
				else if (j == 18 && fieldType == 's') { // geo data ?
					buf >> szString;
					dwControlParam++;
				}
				else if (j == 19 && fieldType == 's') { // ?????? ?
					buf >> szString;
					dwControlParam++;
					_ASSERTE(szString.GetLength());
				}
				else {
					if (fieldType == 's') {
						buf >> szString;
						if (szString.GetLength()) {
							debugLogA("%s ", szString);
						}
					}
					else if (fieldType == 'u') {
						char szBuff[50];
						mir_snprintf(szBuff, SIZEOF(szBuff), "%lu, ", dwTemp);//;
						debugLogA("%s ", szBuff);
					}
					else _CrtDbgBreak();
				}
			}

			debugLogA("ID: %lu, Group id: %lu, %s: flags: %lu (", dwID, dwGroupID, szEmail, dwContactFlag);
			if (dwContactFlag & CONTACT_FLAG_REMOVED)      debugLogA("CONTACT_FLAG_REMOVED, ");
			if (dwContactFlag & CONTACT_FLAG_GROUP)        debugLogA("CONTACT_FLAG_GROUP, ");
			if (dwContactFlag & CONTACT_FLAG_INVISIBLE)    debugLogA("CONTACT_FLAG_INVISIBLE, ");
			if (dwContactFlag & CONTACT_FLAG_VISIBLE)      debugLogA("CONTACT_FLAG_VISIBLE, ");
			if (dwContactFlag & CONTACT_FLAG_IGNORE)       debugLogA("CONTACT_FLAG_IGNORE, ");
			if (dwContactFlag & CONTACT_FLAG_SHADOW)       debugLogA("CONTACT_FLAG_SHADOW, ");
			if (dwContactFlag & CONTACT_FLAG_AUTHORIZED)   debugLogA("CONTACT_FLAG_AUTHORIZED, ");
			if (dwContactFlag & CONTACT_FLAG_MULTICHAT)    debugLogA("CONTACT_FLAG_MULTICHAT, ");
			if (dwContactFlag & CONTACT_FLAG_UNICODE_NAME) debugLogA("CONTACT_FLAG_UNICODE_NAME, ");
			if (dwContactFlag & CONTACT_FLAG_PHONE)        debugLogA("CONTACT_FLAG_PHONE, ");
			debugLogA(")");

			debugLogA(": server flags: %lu (", dwContactSeverFlags);
			if (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED) debugLogA("CONTACT_INTFLAG_NOT_AUTHORIZED, ");
			debugLogA(")");

			// add/modify contact
			if (dwGroupID != 103)//***deb filtering phone/sms contats
			if (_strnicmp(szEmail, "phone", 5))
			if (dwControlParam > 5)// все параметры правильно инициализированны!
			if ((dwContactFlag & (CONTACT_FLAG_GROUP | CONTACT_FLAG_REMOVED)) == 0) {
				BOOL bAdded;
				MCONTACT hContact = MraHContactFromEmail(szEmail, TRUE, FALSE, &bAdded);
				if (hContact) {
					// already in list, remove the duplicate
					if (GetContactBasicInfoW(hContact, &dwTemp, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == NO_ERROR && dwTemp != -1) {
						dwTemp = dwTemp;
						_CrtDbgBreak();
					}
					else {
						dwTemp = GetMirandaStatusFromMraStatus(dwStatus, GetMraXStatusIDFromMraUriStatus(szSpecStatusUri), &dwXStatus);

						if (bAdded) { // update user info
							SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID | SCBIF_GROUP_ID | SCBIF_FLAG | SCBIF_SERVER_FLAG | SCBIF_STATUS | SCBIF_NICK | SCBIF_PHONES),
								dwID, dwGroupID, dwContactFlag, dwContactSeverFlags, dwTemp, NULL, &wszNick, &szCustomPhones);
							// request user info from server
							MraUpdateContactInfo(hContact);
						}
						else {
							if (iGroupMode == 100) { // first start
								ptrT tszGroup(db_get_tsa(hContact, "CList", "Group"));
								if (tszGroup)
									dwGroupID = MraMoveContactToGroup(hContact, dwGroupID, tszGroup);
							}

							SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID | SCBIF_GROUP_ID | SCBIF_SERVER_FLAG | SCBIF_STATUS),
								dwID, dwGroupID, dwContactFlag, dwContactSeverFlags, dwTemp, NULL, &wszNick, &szCustomPhones);
							if (wszNick.IsEmpty()) { // set the server-side nick
								wszNick = GetContactNameW(hContact);
								MraModifyContact(hContact, &dwID, &dwContactFlag, &dwGroupID, &szEmail, &wszNick, &szCustomPhones);
							}
						}

						MraContactCapabilitiesSet(hContact, dwFutureFlags);
						setByte(hContact, DBSETTING_XSTATUSID, (BYTE)dwXStatus);
						mraSetStringW(hContact, DBSETTING_XSTATUSNAME, wszStatusTitle);
						mraSetStringW(hContact, DBSETTING_XSTATUSMSG, wszStatusDesc);
						setDword(hContact, DBSETTING_BLOGSTATUSTIME, dwBlogStatusTime);
						mraWriteContactSettingBlob(hContact, DBSETTING_BLOGSTATUSID, &dwBlogStatusID.QuadPart, sizeof(DWORDLONG));
						mraSetStringW(hContact, DBSETTING_BLOGSTATUS, wszBlogStatus);
						mraSetStringW(hContact, DBSETTING_BLOGSTATUSMUSIC, wszBlogStatusMusic);
						if (IsXStatusValid(dwXStatus) || wszBlogStatus.GetLength())
							SetExtraIcons(hContact);

						if (dwTemp != ID_STATUS_OFFLINE) { // пишем клиента только если юзер не отключён, иначе не затираем старое
							if (!szUserAgentFormatted.IsEmpty()) {
								if (getByte("MirVerRaw", MRA_DEFAULT_MIRVER_RAW) == FALSE)
									szUserAgentFormatted = MraGetVersionStringFromFormatted(szUserAgentFormatted);
							}
							else szUserAgentFormatted = MIRVER_UNKNOWN;
							mraSetStringA(hContact, "MirVer", szUserAgentFormatted);
						}

						if (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED)
						if (getByte("AutoAuthRequestOnLogon", MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON))
							CallProtoService(m_szModuleName, MRA_REQ_AUTH, hContact, 0);
					}
				}
			}
			dwID++;
		}// end while (processing contacts)

		// post processing contact list
		{
			CMStringA szEmail, szPhones;
			CMStringW wszAuthMessage, wszNick;

			if (mraGetStringW(NULL, "AuthMessage", wszAuthMessage) == FALSE) // def auth message
				wszAuthMessage = TranslateT(MRA_DEFAULT_AUTH_MESSAGE);

			for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
				if (GetContactBasicInfoW(hContact, &dwID, NULL, NULL, NULL, NULL, &szEmail, NULL, NULL) == NO_ERROR)
				if (dwID == -1) {
					if (IsEMailChatAgent(szEmail)) {// чат: ещё раз запросим авторизацию, пометим как видимый в списке, постоянный
						db_unset(hContact, "CList", "Hidden");
						db_unset(hContact, "CList", "NotOnList");
						SetExtraIcons(hContact);
						MraSetContactStatus(hContact, ID_STATUS_ONLINE);

						CMStringW wszCustomName = GetContactNameW(hContact);
						MraAddContact(hContact, (CONTACT_FLAG_VISIBLE | CONTACT_FLAG_MULTICHAT), -1, szEmail, wszCustomName);
					}
					else {
						if (db_get_b(hContact, "CList", "NotOnList", 0) == 0) { // set extra icons and upload contact
							SetExtraIcons(hContact);
							if (getByte("AutoAddContactsToServer", MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER)) { //add all contacts to server
								GetContactBasicInfoW(hContact, NULL, &dwGroupID, NULL, NULL, NULL, NULL, &wszNick, &szPhones);
								MraAddContact(hContact, (CONTACT_FLAG_VISIBLE | CONTACT_FLAG_UNICODE_NAME), dwGroupID, szEmail, wszNick, &szPhones, &wszAuthMessage);
							}
						}
					}
					MraUpdateContactInfo(hContact);
				}
			}
		}
		setByte("GroupMode", 1);
	}
	else { // контакт лист почемуто не получили
		// всех в offline и id в нестандарт
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID | SCBIF_GROUP_ID | SCBIF_SERVER_FLAG | SCBIF_STATUS),
				-1, -2, 0, 0, ID_STATUS_OFFLINE, 0, 0, 0);
			// request user info from server
			MraUpdateContactInfo(hContact);
		}

		if (dwTemp == GET_CONTACTS_ERROR) // найденный контакт-лист некорректен
			ShowFormattedErrorMessage(L"MRIM_CS_CONTACT_LIST2: bad contact list", NO_ERROR);
		else if (dwTemp == GET_CONTACTS_INTERR) // произошла внутренняя ошибка
			ShowFormattedErrorMessage(L"MRIM_CS_CONTACT_LIST2: internal server error", NO_ERROR);
		else {
			TCHAR szBuff[1024];
			mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateT("MRIM_CS_CONTACT_LIST2: unknown server error, code: %lu"), dwTemp);
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, szBuff);
		}
	}
	return true;
}

bool CMraProto::CmdProxy(BinBuffer &buf)
{
	DWORD dwIDRequest, dwAckType;
	CMStringA szAddresses, szEmail, szString;
	MRA_GUID mguidSessionID;

	buf >> szEmail >> dwIDRequest >> dwAckType >> szString >> szAddresses >> mguidSessionID;
	if (dwAckType == MRIM_PROXY_TYPE_FILES) { // файлы, on file recv
		// set proxy info to file transfer context
		if (!MraMrimProxySetData(MraFilesQueueItemProxyByID(hFilesQueueHandle, dwIDRequest), szEmail, dwIDRequest, dwAckType, szString, szAddresses, &mguidSessionID))
			MraFilesQueueStartMrimProxy(hFilesQueueHandle, dwIDRequest);
		else { // empty/invalid session
			MraProxyAck(PROXY_STATUS_ERROR, szEmail, dwIDRequest, dwAckType, szString, szAddresses, mguidSessionID);
			_CrtDbgBreak();
		}
	}
	return true;
}

bool CMraProto::CmdProxyAck(BinBuffer &buf)
{
	DWORD dwIDRequest, dwTemp, dwAckType;
	HANDLE hMraMrimProxyData;
	CMStringA szAddresses, szEmail, szString;
	MRA_GUID mguidSessionID;
	buf >> dwTemp >> szEmail >> dwIDRequest >> dwAckType >> szString >> szAddresses >> mguidSessionID;

	if (dwAckType == MRIM_PROXY_TYPE_FILES) { // on file send
		if ((hMraMrimProxyData = MraFilesQueueItemProxyByID(hFilesQueueHandle, dwIDRequest))) {
			switch (dwTemp) {
			case PROXY_STATUS_DECLINE:
				MraFilesQueueCancel(hFilesQueueHandle, dwIDRequest, FALSE);
				break;
			case PROXY_STATUS_OK:
				// set proxy info to file transfer context
				if (!MraMrimProxySetData(hMraMrimProxyData, szEmail, dwIDRequest, dwAckType, szString, szAddresses, &mguidSessionID))
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
				_CrtDbgBreak();
				break;
			}
		}
		else _CrtDbgBreak();
	}
	return true;
}

bool CMraProto::CmdNewMail(BinBuffer &buf)
{
	DWORD dwDate, dwUIDL, dwUnreadCount;
	CMStringA szEmail, szString;
	buf >> dwUnreadCount >> szEmail >> szString >> dwDate >> dwUIDL;

	if (dwUnreadCount > dwEmailMessagesTotal)
		dwEmailMessagesTotal += (dwUnreadCount - m_dwEmailMessagesUnread);

	DWORD dwSave = m_dwEmailMessagesUnread;
	m_dwEmailMessagesUnread = dwUnreadCount;// store new value
	if (getByte("IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY) == 0 || dwSave < dwUnreadCount || dwUnreadCount == 0)
		MraUpdateEmailStatus(szEmail, szString, dwDate, dwUIDL, false);
	return true;
}

bool CMraProto::CmdBlogStatus(BinBuffer &buf)
{
	DWORD dwTime, dwFlags;
	CMStringA szEmail, szString;
	CMStringW wszText;
	DWORDLONG dwBlogStatusID;

	buf >> dwFlags >> szEmail >> dwBlogStatusID >> dwTime >> wszText >> szString;

	if (MCONTACT hContact = MraHContactFromEmail(szEmail, FALSE, TRUE, NULL)) {
		if (dwFlags & MRIM_BLOG_STATUS_MUSIC)
			mraSetStringW(hContact, DBSETTING_BLOGSTATUSMUSIC, wszText);
		else {
			setDword(hContact, DBSETTING_BLOGSTATUSTIME, dwTime);
			mraWriteContactSettingBlob(hContact, DBSETTING_BLOGSTATUSID, &dwBlogStatusID, sizeof(DWORDLONG));
			mraSetStringW(hContact, DBSETTING_BLOGSTATUS, wszText);
		}
		SetExtraIcons(hContact);
	}
	return true;
}

bool CMraProto::MraCommandDispatcher(mrim_packet_header_t *pmaHeader)
{
	WCHAR szBuff[4096] = { 0 };
	DWORD dwTemp, dwAckType;
	size_t dwSize;
	MCONTACT hContact = NULL;
	LPBYTE pByte;

	debugLogA("Received packet %x\n", pmaHeader->msg);

	BinBuffer buf((LPBYTE)pmaHeader + sizeof(mrim_packet_header_t), pmaHeader->dlen);

	switch (pmaHeader->msg) {
	case MRIM_CS_HELLO_ACK:	          return CmdHelloAck(buf);
	case MRIM_CS_LOGIN_ACK:           return CmdLoginAck();
	case MRIM_CS_LOGIN_REJ:           return CmdLoginRejected(buf);
	case MRIM_CS_MESSAGE_ACK:         return CmdMessageAck(buf);
	case MRIM_CS_MESSAGE_STATUS:      return CmdMessageStatus(pmaHeader->seq, buf);
	case MRIM_CS_USER_INFO:           return CmdUserInfo(buf);
	case MRIM_CS_OFFLINE_MESSAGE_ACK: return CmdOfflineMessageAck(buf);
	case MRIM_CS_AUTHORIZE_ACK:       return CmdAuthAck(buf);
	case MRIM_CS_MPOP_SESSION:        return CmdPopSession(buf);
	case MRIM_CS_FILE_TRANSFER:       return CmdFileTransfer(buf);
	case MRIM_CS_FILE_TRANSFER_ACK:   return CmdFileTransferAck(buf);
	case MRIM_CS_USER_STATUS:         return CmdUserStatus(buf);
	case MRIM_CS_ADD_CONTACT_ACK:
	case MRIM_CS_MODIFY_CONTACT_ACK:  return CmdContactAck(pmaHeader->msg, pmaHeader->seq, buf);
	case MRIM_CS_ANKETA_INFO:         return CmdAnketaInfo(pmaHeader->seq, buf);
	case MRIM_CS_GAME:                return CmdGame(buf);
	case MRIM_CS_CONTACT_LIST2:       return CmdClist2(buf);
	case MRIM_CS_PROXY:               return CmdProxy(buf);
	case MRIM_CS_PROXY_ACK:           return CmdProxyAck(buf);
	case MRIM_CS_NEW_MAIL:            return CmdNewMail(buf);
	case MRIM_CS_USER_BLOG_STATUS:    return CmdBlogStatus(buf);

	case MRIM_CS_CONNECTION_PARAMS:// Изменение параметров соединения
		buf >> m_dwPingPeriod;
		m_dwNextPingSendTickTime = 0; // force send ping
		MraSendCMD(MRIM_CS_PING, NULL, 0);
		break;

	case MRIM_CS_LOGOUT:// Пользователь отключен из-за параллельного входа с его логином.
		buf >> dwTemp;
		if (dwTemp == LOGOUT_NO_RELOGIN_FLAG)
			ShowFormattedErrorMessage(L"Another user connected with your login", NO_ERROR);
		return false;

	case MRIM_CS_MAILBOX_STATUS:
		buf >> dwTemp;
		if (dwTemp > dwEmailMessagesTotal)
			dwEmailMessagesTotal += (dwTemp - m_dwEmailMessagesUnread);

		dwAckType = m_dwEmailMessagesUnread;// save old value
		m_dwEmailMessagesUnread = dwTemp;// store new value
		if (getByte("IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY) == 0 || dwAckType < dwTemp || dwTemp == 0)
			MraUpdateEmailStatus("", "", 0, 0, false);
		break;

	case MRIM_CS_SMS_ACK:
		buf >> dwTemp;
		if (MraSendQueueFind(hSendQueueHandle, pmaHeader->seq, NULL, &hContact, &dwAckType, &pByte, &dwSize) == NO_ERROR) {
			/* pByte point to phone number ansi string. */
			/* dwAckType = ICQACKTYPE_SMS */
			CMStringA szEmail;
			if (mraGetStringA(NULL, "e-mail", szEmail)) {
				mir_snprintf((LPSTR)szBuff, sizeof(szBuff),
					"<sms_response><source>Mail.ru</source><deliverable>Yes</deliverable><network>Mail.ru, Russia</network><message_id>%s-1-1955988055-%s</message_id><destination>%s</destination><messages_left>0</messages_left></sms_response>\r\n",
					szEmail.c_str(), (LPSTR)pByte, (LPSTR)pByte);
				ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_SENTREQUEST, (HANDLE)pmaHeader->seq, (LPARAM)szBuff);
			}
			mir_free(pByte);
			MraSendQueueFree(hSendQueueHandle, pmaHeader->seq);
		}
		else MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG, 0, TranslateT("MRIM_CS_SMS_ACK: not found in queue"));
		break;

	case MRIM_CS_PROXY_HELLO:
		_CrtDbgBreak();
		break;

	case MRIM_CS_PROXY_HELLO_ACK:
		_CrtDbgBreak();
		break;

	case MRIM_CS_UNKNOWN:
	case MRIM_CS_UNKNOWN2:
	case MRIM_CS_USER_GEO:
	case MRIM_CS_SERVER_SETTINGS:
		break;

	default:
		_CrtDbgBreak();
		break;
	}
	return true;
}

// Сообщение
DWORD CMraProto::MraRecvCommand_Message(DWORD dwTime, DWORD dwFlags, CMStringA &plpsFrom, CMStringA &plpsText, CMStringA &plpsRFTText, CMStringA &plpsMultiChatData)
{
	DWORD dwRetErrorCode = NO_ERROR, dwBackColour;
	CMStringA lpszMessageExt;
	CMStringW wszMessage;

	PROTORECVEVENT pre = { 0 };
	pre.timestamp = dwTime;

	// check flags and datas
	if ((dwFlags & MESSAGE_FLAG_RTF) && plpsRFTText.IsEmpty())
		dwFlags &= ~MESSAGE_FLAG_RTF;

	if ((dwFlags & MESSAGE_FLAG_MULTICHAT) && plpsMultiChatData.IsEmpty())
		dwFlags &= ~MESSAGE_FLAG_MULTICHAT;

	// pre processing - extracting/decoding
	if (dwFlags & MESSAGE_FLAG_AUTHORIZE) { // extract auth message из обычного текста
		unsigned dwAuthDataSize;
		LPBYTE lpbAuthData = (LPBYTE)mir_base64_decode(plpsText, &dwAuthDataSize);
		if (lpbAuthData) {
			BinBuffer buf(lpbAuthData, dwAuthDataSize);

			DWORD dwAuthPartsCount;
			CMStringA lpsAuthFrom;
			buf >> dwAuthPartsCount >> lpsAuthFrom;
			if (dwFlags & MESSAGE_FLAG_v1p16 && (dwFlags & MESSAGE_FLAG_CP1251) == 0) { // unicode text
				CMStringW lpsAuthMessageW;
				buf >> lpsAuthMessageW;
				wszMessage = lpsAuthMessageW;
			}
			else { // преобразуем в юникод текст только если он в АНСИ и если это не Флэш мультик и будильник тоже не нуждается в этом
				CMStringA lpsAuthMessage;
				buf >> lpsAuthMessage;
				wszMessage = ptrW(mir_a2u_cp(lpsAuthMessage, MRA_CODE_PAGE));
			}
			mir_free(lpbAuthData);
		}
	}
	else {
		// unicode text
		if ((dwFlags & (MESSAGE_FLAG_ALARM | MESSAGE_FLAG_FLASH | MESSAGE_FLAG_v1p16)) && (dwFlags & MESSAGE_FLAG_CP1251) == 0) {
			plpsText.AppendChar(0);  // compensate difference between ASCIIZ & WCHARZ
			wszMessage = (WCHAR*)plpsText.GetString();
		}
		else wszMessage = plpsText;

		if (dwFlags & (MESSAGE_FLAG_CONTACT | MESSAGE_FLAG_NOTIFY | MESSAGE_FLAG_SMS | MESSAGE_SMS_DELIVERY_REPORT | MESSAGE_FLAG_ALARM))
			; // do nothing; there's no extra part in a message
		else {
			if ((dwFlags & MESSAGE_FLAG_RTF) && !plpsRFTText.IsEmpty()) { //MESSAGE_FLAG_FLASH there
				size_t dwRFTBuffSize = ((plpsRFTText.GetLength() * 16) + 8192);

				mir_ptr<BYTE> lpbRTFData((LPBYTE)mir_calloc(dwRFTBuffSize));
				if (lpbRTFData) {
					unsigned dwCompressedSize;
					mir_ptr<BYTE> lpbCompressed((LPBYTE)mir_base64_decode(plpsRFTText, &dwCompressedSize));
					DWORD dwRTFDataSize = dwRFTBuffSize;
					if (uncompress(lpbRTFData, &dwRTFDataSize, lpbCompressed, dwCompressedSize) == Z_OK) {
						BinBuffer buf(lpbRTFData, dwRTFDataSize);

						CMStringA lpsRTFString, lpsBackColour, szString;
						DWORD dwRTFPartsCount;

						// количество частей в некоторых случаях больше 2, тогда нужно игнорировать первый текст, тк там сообщения об ущербности
						buf >> dwRTFPartsCount >> lpsRTFString >> dwBackColour;
						if (dwFlags & MESSAGE_FLAG_FLASH) {
							if (dwRTFPartsCount == 4) {
								buf >> szString;
								dwRTFPartsCount--;
							}
							if (dwRTFPartsCount == 3) { // ansi text only
								buf >> szString;
								wszMessage = ptrW(mir_a2u_cp(szString, MRA_CODE_PAGE));
							}
							else _CrtDbgBreak();
						}
						else { // RTF text
							if (dwRTFPartsCount > 2) {
								buf >> szString;
								_CrtDbgBreak();
							}

							lpszMessageExt = lpsRTFString;
						}
					}
					else _CrtDbgBreak();
				}
			}
		}
	}

	debugLogA("Processing message: %08X, from '%s', text '%S'\n", dwFlags, plpsFrom.c_str(), wszMessage.c_str());

	// processing
	if (dwFlags & (MESSAGE_FLAG_SMS | MESSAGE_SMS_DELIVERY_REPORT)) {// SMS //if (IsPhone(plpsFrom->lpszData, plpsFrom->dwSize))
		INTERNET_TIME itTime;
		InternetTimeGetCurrentTime(&itTime);
		CMStringA szTime = InternetTimeGetString(&itTime);
		CMStringA szPhone = CopyNumber(plpsFrom), szEmail;
		mraGetStringA(NULL, "e-mail", szEmail);

		CMStringW wszMessageXMLEncoded = EncodeXML(wszMessage);
		ptrA lpszMessageUTF(mir_utf8encodeW(wszMessageXMLEncoded));

		CMStringA szText;
		if (dwFlags & MESSAGE_SMS_DELIVERY_REPORT) {
			szText.Format("<sms_delivery_receipt><message_id>%s-1-1955988055-%s</message_id><destination>%s</destination><delivered>No</delivered><submition_time>%s</submition_time><error_code>0</error_code><error><id>15</id><params><param>%s</param></params></error></sms_delivery_receipt>",
				szEmail, szPhone, szPhone, szTime, lpszMessageUTF);
			ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_FAILED, 0, (LPARAM)szText.GetString());
		}
		else { // new sms
			szText.Format("<sms_message><source>Mail.ru</source><destination_UIN>%s</destination_UIN><sender>%s</sender><senders_network>Mail.ru</senders_network><text>%s</text><time>%s</time></sms_message>",
				szEmail, szPhone, lpszMessageUTF, szTime);
			ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_SUCCESS, 0, (LPARAM)szText.GetString());
		}
	}
	else {
		BOOL bAdded;
		MCONTACT hContact = MraHContactFromEmail(plpsFrom, TRUE, TRUE, &bAdded);
		if (bAdded)
			MraUpdateContactInfo(hContact);

		// user typing
		if (dwFlags & MESSAGE_FLAG_NOTIFY)
			CallService(MS_PROTO_CONTACTISTYPING, hContact, MAILRU_CONTACTISTYPING_TIMEOUT);
		else { // text/contact/auth // typing OFF
			CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

			if (dwFlags & MESSAGE_FLAG_MULTICHAT) {
				DWORD dwMultiChatEventType;
				CMStringA lpsEMailInMultiChat, szString;
				CMStringW lpsMultichatName;

				BinBuffer buf((PBYTE)plpsMultiChatData.GetString(), plpsMultiChatData.GetLength());
				buf >> dwMultiChatEventType >> lpsMultichatName >> lpsEMailInMultiChat;

				switch (dwMultiChatEventType) {
				case MULTICHAT_MESSAGE:
					MraChatSessionMessageAdd(hContact, lpsEMailInMultiChat, wszMessage, dwTime);// LPS sender
					break;
				case MULTICHAT_ADD_MEMBERS:
					MraChatSessionMembersAdd(hContact, lpsEMailInMultiChat, dwTime);// LPS sender
					buf >> szString;// CLPS members
					MraChatSessionSetIviter(hContact, lpsEMailInMultiChat);
				case MULTICHAT_MEMBERS:
					{
						DWORD dwMultiChatMembersCount;
						BinBuffer buf((PBYTE)lpsEMailInMultiChat.GetString(), lpsEMailInMultiChat.GetLength());
						buf >> dwMultiChatMembersCount;// count
						for (unsigned i = 0; i < dwMultiChatMembersCount && !buf.eof(); i++) {
							buf >> szString;
							MraChatSessionJoinUser(hContact, szString, ((dwMultiChatEventType == MULTICHAT_MEMBERS) ? 0 : dwTime));
						}

						if (dwMultiChatEventType == MULTICHAT_MEMBERS) {
							buf >> szString; // [ LPS owner ]
							MraChatSessionSetOwner(hContact, szString);
						}
					}
					break;
				case MULTICHAT_ATTACHED:
					MraChatSessionJoinUser(hContact, lpsEMailInMultiChat, dwTime);// LPS member
					break;
				case MULTICHAT_DETACHED:
					MraChatSessionLeftUser(hContact, lpsEMailInMultiChat, dwTime);// LPS member
					break;
				case MULTICHAT_INVITE:
					MraChatSessionInvite(hContact, lpsEMailInMultiChat, dwTime);// LPS sender
					MraAddContact(hContact, (CONTACT_FLAG_VISIBLE | CONTACT_FLAG_MULTICHAT | CONTACT_FLAG_UNICODE_NAME), -1, plpsFrom, lpsMultichatName);
					break;
				default:
					_CrtDbgBreak();
					break;
				}
			}
			else if (dwFlags & MESSAGE_FLAG_AUTHORIZE) { // auth request
				BOOL bAutoGrantAuth = FALSE;

				if (IsEMailChatAgent(plpsFrom))
					bAutoGrantAuth = FALSE;
				else {
					// temporary contact
					if (db_get_b(hContact, "CList", "NotOnList", 0)) {
						if (getByte("AutoAuthGrandNewUsers", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS))
							bAutoGrantAuth = TRUE;
					}
					else if (getByte("AutoAuthGrandUsersInCList", MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST))
						bAutoGrantAuth = TRUE;
				}

				CMStringA szBlob = CreateBlobFromContact(hContact, wszMessage);
				if (bAutoGrantAuth) { // auto grant auth
					DBEVENTINFO dbei = { sizeof(dbei) };
					dbei.szModule = m_szModuleName;
					dbei.timestamp = _time32(NULL);
					dbei.flags = DBEF_READ;
					dbei.eventType = EVENTTYPE_AUTHREQUEST;
					dbei.pBlob = (PBYTE)szBlob.c_str();
					dbei.cbBlob = szBlob.GetLength();
					db_event_add(0, &dbei);
					MraAuthorize(plpsFrom);
				}
				else {
					pre.szMessage = (LPSTR)szBlob.GetString();
					pre.lParam = szBlob.GetLength();
					ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
				}
			}
			else {
				db_unset(hContact, "CList", "Hidden");

				if (dwFlags & MESSAGE_FLAG_CONTACT) { // contacts received
					ptrA lpbBuffer(mir_u2a_cp(wszMessage, MRA_CODE_PAGE));
					pre.flags = 0;
					pre.szMessage = (LPSTR)lpbBuffer;
					pre.lParam = strlen(lpbBuffer);

					LPSTR lpbBufferCurPos = lpbBuffer;
					while (TRUE) { // цикл замены ; на 0
						lpbBufferCurPos = (LPSTR)MemoryFindByte((lpbBufferCurPos - (LPSTR)lpbBuffer), lpbBuffer, pre.lParam, ';');
						if (!lpbBufferCurPos)
							break;

						// found
						(*lpbBufferCurPos) = 0;
						lpbBufferCurPos ++;
					}
					ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&pre);
				}
				else if (dwFlags & MESSAGE_FLAG_ALARM) { // alarm
					if (m_heNudgeReceived)
						NotifyEventHooks(m_heNudgeReceived, hContact, NULL);
					else {
						pre.flags = PREF_UNICODE;
						pre.szMessage = (LPSTR)TranslateTS(MRA_ALARM_MESSAGE);
						ProtoChainRecvMsg(hContact, &pre);
					}
				}
				else { // standart message// flash animation
					// пишем в ANSI, всё равно RTF
					if ((dwFlags & MESSAGE_FLAG_RTF) && (dwFlags & MESSAGE_FLAG_FLASH) == 0 && !lpszMessageExt.IsEmpty() && getByte("RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE)) {
						pre.flags = 0;
						pre.szMessage = (LPSTR)lpszMessageExt.GetString();
						ProtoChainRecvMsg(hContact, &pre);
					}
					else {
						// some plugins can change pre.szMessage pointer and we failed to free it
						ptrA lpszMessageUTF(mir_utf8encodeW(wszMessage));
						pre.szMessage = lpszMessageUTF;
						pre.flags = PREF_UTF;
						ProtoChainRecvMsg(hContact, &pre);
					}

					if (dwFlags & MESSAGE_FLAG_SYSTEM)
						MraPopupShowW(hContact, MRA_POPUP_TYPE_INFORMATION, 0, TranslateT("Mail.ru System notify"), (LPWSTR)pre.szMessage);
				}
			}
		}
	}

	return NO_ERROR;
}

DWORD GetMraXStatusIDFromMraUriStatus(const char *szStatusUri)
{
	if (szStatusUri)
		for (size_t i = 0; lpcszStatusUri[i]; i++)
			if (!_stricmp(lpcszStatusUri[i], szStatusUri))
				return i;

	return MRA_XSTATUS_UNKNOWN;
}

DWORD GetMraStatusFromMiradaStatus(DWORD dwMirandaStatus, DWORD dwXStatusMir, DWORD *pdwXStatusMra)
{
	if (IsXStatusValid(dwXStatusMir)) {
		if (pdwXStatusMra)
			*pdwXStatusMra = (dwXStatusMir + MRA_XSTATUS_INDEX_OFFSET - 1);
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
		return (STATUS_ONLINE | STATUS_FLAG_INVISIBLE);
	}

	if (pdwXStatusMra) *pdwXStatusMra = MRA_XSTATUS_OFFLINE;
	return STATUS_OFFLINE;
}

DWORD GetMirandaStatusFromMraStatus(DWORD dwMraStatus, DWORD dwXStatusMra, DWORD *pdwXStatusMir)
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
		case MRA_XSTATUS_MOBILE:  return ID_STATUS_ONTHEPHONE;
		case MRA_XSTATUS_UNKNOWN:
			if (pdwXStatusMir) *pdwXStatusMir = MRA_MIR_XSTATUS_UNKNOWN;
			return ID_STATUS_ONLINE;
		}
		if (pdwXStatusMir) *pdwXStatusMir = dwXStatusMra - MRA_XSTATUS_INDEX_OFFSET + 1;
		return ID_STATUS_ONLINE;
	default:
		if (dwMraStatus & STATUS_FLAG_INVISIBLE)
			return ID_STATUS_INVISIBLE;
	}

	return ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

DWORD BinBuffer::getDword()
{
	if (m_len >= sizeof(DWORD)) {
		DWORD ret = *(DWORD*)m_data;
		m_data += sizeof(DWORD);
		m_len -= sizeof(DWORD);
		return ret;
	}
	return 0;
}

DWORDLONG BinBuffer::getInt64()
{
	if (m_len >= sizeof(DWORDLONG)) {
		DWORDLONG ret = *(DWORDLONG*)m_data;
		m_data += sizeof(DWORDLONG);
		m_len -= sizeof(DWORDLONG);
		return ret;
	}
	return 0;
}

MRA_GUID BinBuffer::getGuid()
{
	MRA_GUID ret;
	if (m_len >= sizeof(MRA_GUID)) {
		ret = *(MRA_GUID*)m_data;
		m_data += sizeof(MRA_GUID);
		m_len -= sizeof(MRA_GUID);
		return ret;
	}
	else memset(&ret, 0, sizeof(ret));
	return ret;
}

void BinBuffer::getStringA(CMStringA& ret)
{
	if (m_len >= sizeof(DWORD)) {
		DWORD dwLen = *(DWORD*)m_data;
		m_data += sizeof(DWORD);
		m_len -= sizeof(DWORD);
		if (m_len >= dwLen) {
			ret = CMStringA((LPSTR)m_data, dwLen);
			m_data += dwLen;
			m_len -= dwLen;
			return;
		}
	}
	ret.Empty();
}

void BinBuffer::getStringW(CMStringW& ret)
{
	if (m_len >= sizeof(DWORD)) {
		DWORD dwLen = *(DWORD*)m_data;
		m_data += sizeof(DWORD);
		m_len -= sizeof(DWORD);
		if (m_len >= dwLen) {
			ret = CMStringW((LPWSTR)m_data, dwLen / 2);
			m_data += dwLen;
			m_len -= dwLen;
			return;
		}
	}
	ret.Empty();
}
