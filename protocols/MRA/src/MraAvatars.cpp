#include "Mra.h"
#include "MraAvatars.h"

#define PA_FORMAT_MAX		7
const LPTSTR lpcszExtensions[9] =
{
	_T(".dat"), // PA_FORMAT_UNKNOWN
	_T(".png"), // PA_FORMAT_PNG
	_T(".jpg"), // PA_FORMAT_JPEG
	_T(".ico"), // PA_FORMAT_ICON
	_T(".bmp"), // PA_FORMAT_BMP
	_T(".gif"), // PA_FORMAT_GIF
	_T(".swf"), // PA_FORMAT_SWF
	_T(".xml"), // PA_FORMAT_XML
	NULL
};

const LPSTR lpcszContentType[9] =
{
	"", 					// PA_FORMAT_UNKNOWN
	"image/png", 		// PA_FORMAT_PNG
	"image/jpeg", 		// PA_FORMAT_JPEG
	"image/icon", 		// PA_FORMAT_ICON
	"image/x-xbitmap", 	// PA_FORMAT_BMP
	"image/gif", 		// PA_FORMAT_GIF
	"", 					// PA_FORMAT_SWF
	"", 					// PA_FORMAT_XML
	NULL
};

struct MRA_AVATARS_QUEUE : public FIFO_MT
{
	BOOL   bIsRunning;
	HANDLE hNetlibUser;
	HANDLE hThreadEvent;
	int    iThreadsCount;
	HANDLE hThread[MAXIMUM_WAIT_OBJECTS];
	LONG   lThreadsRunningCount;
	HANDLE hAvatarsPath;
};

struct MRA_AVATARS_QUEUE_ITEM : public FIFO_MT_ITEM
{
	DWORD  dwAvatarsQueueID;
	DWORD  dwFlags;
	HANDLE hContact;
};

#define FILETIME_SECOND				((DWORDLONG)10000000)
#define FILETIME_MINUTE				((DWORDLONG)FILETIME_SECOND*60)


char szAvtSectName[MAX_PATH];
#define MRA_AVT_SECT_NAME		szAvtSectName

#define NETLIB_CLOSEHANDLE(hConnection) {Netlib_CloseHandle(hConnection);hConnection = NULL;}

HANDLE MraAvatarsHttpConnect(HANDLE hNetlibUser, LPSTR lpszHost, DWORD dwPort);

#define MAHTRO_AVT          0
#define MAHTRO_AVTMRIM      1
#define MAHTRO_AVTSMALL     2
#define MAHTRO_AVTSMALLMRIM 3

DWORD MraAvatarsHttpTransaction	(HANDLE hConnection, DWORD dwRequestType, LPSTR lpszUser, LPSTR lpszDomain, LPSTR lpszHost, DWORD dwReqObj, BOOL bUseKeepAliveConn, DWORD *pdwResultCode, BOOL *pbKeepAlive, DWORD *pdwFormat, size_t *pdwAvatarSize, INTERNET_TIME *pitLastModifiedTime);
DWORD MraAvatarsGetFileFormat		(LPTSTR lpszPath, size_t dwPathSize);

DWORD CMraProto::MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle)
{
	mir_snprintf(szAvtSectName, SIZEOF(szAvtSectName), "%s Avatars", m_szModuleName);

	if (phAvatarsQueueHandle == NULL)
		return ERROR_INVALID_HANDLE;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)mir_calloc(sizeof(MRA_AVATARS_QUEUE));
	if (pmraaqAvatarsQueue == NULL)
		return GetLastError();

	DWORD dwRetErrorCode = FifoMTInitialize(pmraaqAvatarsQueue, 0);
	if (dwRetErrorCode == NO_ERROR) {
		CHAR szBuffer[MAX_PATH];
		mir_snprintf(szBuffer, SIZEOF(szBuffer), "%s %s", m_szModuleName, Translate("Avatars' plugin connections"));

		NETLIBUSER nlu = {0};
		nlu.cbSize = sizeof(nlu);
		nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
		nlu.szSettingsModule = MRA_AVT_SECT_NAME;
		nlu.szDescriptiveName = szBuffer;
		pmraaqAvatarsQueue->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
		if (pmraaqAvatarsQueue->hNetlibUser) {
			TCHAR tszPath[ MAX_PATH ];
			mir_sntprintf( tszPath, SIZEOF(tszPath), _T("%%miranda_avatarcache%%\\%s"), m_tszUserName);
			pmraaqAvatarsQueue->hAvatarsPath = FoldersRegisterCustomPathT(LPGEN("Avatars"), m_szModuleName, tszPath, m_tszUserName);

			InterlockedExchange((volatile LONG*)&pmraaqAvatarsQueue->bIsRunning, TRUE);
			pmraaqAvatarsQueue->hThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			pmraaqAvatarsQueue->iThreadsCount = db_get_dw(NULL, MRA_AVT_SECT_NAME, "WorkThreadsCount", MRA_AVT_DEFAULT_WRK_THREAD_COUNTS);
			if (pmraaqAvatarsQueue->iThreadsCount == 0)
				pmraaqAvatarsQueue->iThreadsCount = 1;
			if (pmraaqAvatarsQueue->iThreadsCount > MAXIMUM_WAIT_OBJECTS)
				pmraaqAvatarsQueue->iThreadsCount = MAXIMUM_WAIT_OBJECTS;
			for (int i=0; i < pmraaqAvatarsQueue->iThreadsCount; i++)
				pmraaqAvatarsQueue->hThread[i] = ForkThreadEx(&CMraProto::MraAvatarsThreadProc, pmraaqAvatarsQueue, 0);

			*phAvatarsQueueHandle = (HANDLE)pmraaqAvatarsQueue;
		}
	}

	return dwRetErrorCode;
}

void CMraProto::MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle)
{
	if ( !hAvatarsQueueHandle)
		return;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;

	PROTO_AVATAR_INFORMATIONT pai = {0};
	pai.cbSize = sizeof(pai);
	pai.format = PA_FORMAT_UNKNOWN;

	while (FifoMTItemPop(pmraaqAvatarsQueue, NULL, (LPVOID*)&pmraaqiAvatarsQueueItem) == NO_ERROR) {
		pai.hContact = pmraaqiAvatarsQueueItem->hContact;
		ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, 0);
		mir_free(pmraaqiAvatarsQueueItem);
	}
}

void CMraProto::MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle)
{
	if ( !hAvatarsQueueHandle)
		return;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;

	InterlockedExchange((volatile LONG*)&pmraaqAvatarsQueue->bIsRunning, FALSE);
	SetEvent(pmraaqAvatarsQueue->hThreadEvent);

	WaitForMultipleObjects(pmraaqAvatarsQueue->iThreadsCount, (HANDLE*)&pmraaqAvatarsQueue->hThread[0], TRUE, (WAIT_FOR_THREAD_TIMEOUT*1000));

	if (InterlockedExchangeAdd((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount, 0))
		while (InterlockedExchangeAdd((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount, 0))
			SleepEx(100, TRUE);

	for (int i=0; i < pmraaqAvatarsQueue->iThreadsCount; i++)
		CloseHandle(pmraaqAvatarsQueue->hThread[i]);
	CloseHandle(pmraaqAvatarsQueue->hThreadEvent);

	MraAvatarsQueueClear(hAvatarsQueueHandle);

	FifoMTDestroy(pmraaqAvatarsQueue);
	Netlib_CloseHandle(pmraaqAvatarsQueue->hNetlibUser);
	mir_free(pmraaqAvatarsQueue);
}

DWORD CMraProto::MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD *pdwAvatarsQueueID)
{
	if ( !hAvatarsQueueHandle)
		return ERROR_INVALID_HANDLE;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem = (MRA_AVATARS_QUEUE_ITEM*)mir_calloc(sizeof(MRA_AVATARS_QUEUE_ITEM));
	if ( !pmraaqiAvatarsQueueItem)
		return GetLastError();

	pmraaqiAvatarsQueueItem->dwAvatarsQueueID = GetTickCount();
	pmraaqiAvatarsQueueItem->dwFlags = dwFlags;
	pmraaqiAvatarsQueueItem->hContact = hContact;

	FifoMTItemPush(pmraaqAvatarsQueue, pmraaqiAvatarsQueueItem, (LPVOID)pmraaqiAvatarsQueueItem);
	if (pdwAvatarsQueueID)
		*pdwAvatarsQueueID = pmraaqiAvatarsQueueItem->dwAvatarsQueueID;
	SetEvent(pmraaqAvatarsQueue->hThreadEvent);
	return NO_ERROR;
}

void CMraProto::MraAvatarsThreadProc(LPVOID lpParameter)
{
	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)lpParameter;
	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;

	char szEMail[MAX_EMAIL_LEN], szServer[MAX_PATH];
	WCHAR wszFileName[MAX_FILEPATH], szErrorText[2048];
	BOOL bContinue, bKeepAlive, bUseKeepAliveConn, bFailed, bDownloadNew, bDefaultAvt;
	BYTE btBuff[BUFF_SIZE_RCV];
	DWORD dwResultCode, dwAvatarFormat, dwReceived, dwServerPort, dwErrorCode;
	LPSTR lpszUser, lpszDomain;
	size_t dwEMailSize, dwAvatarSizeServer, dwFileNameSize;
	FILETIME ftLastModifiedTimeServer, ftLastModifiedTimeLocal;
	SYSTEMTIME stAvatarLastModifiedTimeLocal;
	HANDLE hConnection = NULL;
	NETLIBSELECT nls = {0};
	INTERNET_TIME itAvatarLastModifiedTimeServer;
	PROTO_AVATAR_INFORMATIONT pai;

	nls.cbSize = sizeof(nls);
	pai.cbSize = sizeof(pai);

	InterlockedIncrement((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount);

	while ( InterlockedExchangeAdd((volatile LONG*)&pmraaqAvatarsQueue->bIsRunning, 0)) {
		if ( FifoMTItemPop(pmraaqAvatarsQueue, NULL, (LPVOID*)&pmraaqiAvatarsQueueItem) == NO_ERROR) {
			bFailed = TRUE;
			bDownloadNew = FALSE;
			bDefaultAvt = FALSE;

			if (DB_GetStaticStringA(NULL, MRA_AVT_SECT_NAME, "Server", szServer, SIZEOF(szServer), NULL) == FALSE) memmove(szServer, MRA_AVT_DEFAULT_SERVER, sizeof(MRA_AVT_DEFAULT_SERVER));
			dwServerPort = db_get_dw(NULL, MRA_AVT_SECT_NAME, "ServerPort", MRA_AVT_DEFAULT_SERVER_PORT);
			bUseKeepAliveConn = db_get_b(NULL, MRA_AVT_SECT_NAME, "UseKeepAliveConn", MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN);

			if ( mraGetStaticStringA(pmraaqiAvatarsQueueItem->hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
				BuffToLowerCase(szEMail, szEMail, dwEMailSize);
				lpszDomain = (LPSTR)MemoryFindByte(0, szEMail, dwEMailSize, '@');
				lpszUser = (LPSTR)MemoryFindByte((lpszDomain-szEMail), szEMail, dwEMailSize, '.');
				if (lpszDomain && lpszUser) {
					*lpszUser = 0;
					lpszUser = szEMail;
					*lpszDomain = 0;
					lpszDomain++;

					ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_CONNECTING, NULL, 0);
					if (hConnection == NULL)
						hConnection = MraAvatarsHttpConnect(pmraaqAvatarsQueue->hNetlibUser, szServer, dwServerPort);
					if (hConnection) {
						ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_CONNECTED, NULL, 0);
						ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_SENTREQUEST, NULL, 0);
						if ( !MraAvatarsHttpTransaction(hConnection, REQUEST_HEAD, lpszUser, lpszDomain, szServer, MAHTRO_AVTMRIM, bUseKeepAliveConn, &dwResultCode, &bKeepAlive, &dwAvatarFormat, &dwAvatarSizeServer, &itAvatarLastModifiedTimeServer)) {
							switch (dwResultCode) {
							case 200:
								if (MraAvatarsGetContactTime(pmraaqiAvatarsQueueItem->hContact, "AvatarLastModifiedTime", &stAvatarLastModifiedTimeLocal)) {
									SystemTimeToFileTime(&itAvatarLastModifiedTimeServer.stTime, &ftLastModifiedTimeServer);
									SystemTimeToFileTime(&stAvatarLastModifiedTimeLocal, &ftLastModifiedTimeLocal);

									if ((*((DWORDLONG*)&ftLastModifiedTimeServer)) != (*((DWORDLONG*)&ftLastModifiedTimeLocal)))
									{// need check for update
										bDownloadNew = TRUE;
										//ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, 0, 0);
									}
									else {// avatar is valid
										if (MraAvatarsGetFileName(pmraaqAvatarsQueue, pmraaqiAvatarsQueueItem->hContact, dwAvatarFormat, wszFileName, SIZEOF(wszFileName), NULL) == NO_ERROR) {
											if ( IsFileExist( wszFileName ))
												bFailed = FALSE;
											else
												bDownloadNew = TRUE;
										}
									}
								}
								else // need update
									bDownloadNew = TRUE;

								break;
							case 404:// return def avatar
								if (MraAvatarsGetFileName((HANDLE)pmraaqAvatarsQueue, NULL, PA_FORMAT_DEFAULT, wszFileName, SIZEOF(wszFileName), &dwFileNameSize) == NO_ERROR) {
									if ( IsFileExist( wszFileName )) {
										dwAvatarFormat = MraAvatarsGetFileFormat(wszFileName, dwFileNameSize);
										bFailed = FALSE;
									}
									else//loading default avatar
										bDownloadNew = TRUE;

									bDefaultAvt = TRUE;
								}
								break;

							default:
								mir_sntprintf(szErrorText, SIZEOF(szErrorText), TranslateW(L"Avatars: server return HTTP code: %lu"), dwResultCode);
								ShowFormattedErrorMessage(szErrorText, NO_ERROR);
								break;
							}
						}
						if (bUseKeepAliveConn == FALSE || bKeepAlive == FALSE) NETLIB_CLOSEHANDLE(hConnection);
					}

					if (bDownloadNew) {
						if (hConnection == NULL)
							hConnection = MraAvatarsHttpConnect(pmraaqAvatarsQueue->hNetlibUser, szServer, dwServerPort);

						if (hConnection) {
							ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_DATA, NULL, 0);
							if (MraAvatarsHttpTransaction(hConnection, REQUEST_GET, lpszUser, lpszDomain, szServer, MAHTRO_AVT, bUseKeepAliveConn, &dwResultCode, &bKeepAlive, &dwAvatarFormat, &dwAvatarSizeServer, &itAvatarLastModifiedTimeServer) == NO_ERROR && dwResultCode == 200) {
								if (bDefaultAvt)
									dwAvatarFormat = PA_FORMAT_DEFAULT;

								if ( !MraAvatarsGetFileName(pmraaqAvatarsQueue, pmraaqiAvatarsQueueItem->hContact, dwAvatarFormat, wszFileName, SIZEOF(wszFileName), NULL)) {
									HANDLE hFile = CreateFile(wszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
									if (hFile != INVALID_HANDLE_VALUE) {
										DWORD dwWritten = 0;
										bContinue = TRUE;
										nls.dwTimeout = (1000*db_get_dw(NULL, MRA_AVT_SECT_NAME, "TimeOutReceive", MRA_AVT_DEFAULT_TIMEOUT_RECV));
										nls.hReadConns[0] = hConnection;

										while ( bContinue ) {
											switch (CallService(MS_NETLIB_SELECT, 0, (LPARAM)&nls)) {
											case SOCKET_ERROR:
											case 0:// Time out
												dwErrorCode = GetLastError();
												ShowFormattedErrorMessage(L"Avatars: error on receive file data", dwErrorCode);
												bContinue = FALSE;
												break;
											case 1:
												dwReceived = Netlib_Recv(hConnection, (LPSTR)&btBuff, SIZEOF(btBuff), 0);
												if (dwReceived == 0 || dwReceived == SOCKET_ERROR) {
													dwErrorCode = GetLastError();
													ShowFormattedErrorMessage(L"Avatars: error on receive file data", dwErrorCode);
													bContinue = FALSE;
												}
												else {
													if ( WriteFile( hFile, (LPVOID)&btBuff, dwReceived, &dwReceived, NULL)) {
														dwWritten += dwReceived;
														if (dwWritten >= dwAvatarSizeServer)
															bContinue = FALSE;
													}
													else {
														dwErrorCode = GetLastError();
														ShowFormattedErrorMessage(L"Avatars: cant write file data, error", dwErrorCode);
														bContinue = FALSE;
													}
												}
												break;
											}
										}
										CloseHandle(hFile);
										bFailed = FALSE;
									}
									else {
										dwErrorCode = GetLastError();
										mir_sntprintf(szErrorText, SIZEOF(szErrorText), TranslateW(L"Avatars: cant open file %s, error"), wszFileName);
										ShowFormattedErrorMessage(szErrorText, dwErrorCode);
									}
								}
							}
							else DebugBreak();

							if (bUseKeepAliveConn == FALSE || bKeepAlive == FALSE) NETLIB_CLOSEHANDLE(hConnection);
						}
					}
				}
			}

			if (bFailed) {
				DeleteFile(wszFileName);
				pai.hContact = pmraaqiAvatarsQueueItem->hContact;
				pai.format = PA_FORMAT_UNKNOWN;
				pai.filename[0] = 0;
				ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, 0);
			}
			else {
				pai.hContact = pmraaqiAvatarsQueueItem->hContact;
				pai.format = dwAvatarFormat;
				if ( db_get_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", MRA_AVT_DEFAULT_RET_ABC_PATH))
					lstrcpyn(pai.filename, wszFileName, SIZEOF(pai.filename));
				else
					PathToRelativeT(wszFileName, pai.filename);

				if (bDefaultAvt) dwAvatarFormat = PA_FORMAT_DEFAULT;
				SetContactAvatarFormat(pmraaqiAvatarsQueueItem->hContact, dwAvatarFormat);
				MraAvatarsSetContactTime(pmraaqiAvatarsQueueItem->hContact, "AvatarLastModifiedTime", &itAvatarLastModifiedTimeServer.stTime);
				// write owner avatar file name to DB
				if ( pmraaqiAvatarsQueueItem->hContact == NULL) // proto avatar
					CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);

				ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
			}
			mir_free(pmraaqiAvatarsQueueItem);
		}
		else { // waiting until service stop or new task
			NETLIB_CLOSEHANDLE(hConnection);
			WaitForSingleObjectEx(pmraaqAvatarsQueue->hThreadEvent, MRA_AVT_DEFAULT_QE_CHK_INTERVAL, FALSE);
		}
	}

	InterlockedDecrement((volatile LONG*)&pmraaqAvatarsQueue->lThreadsRunningCount);
}

HANDLE MraAvatarsHttpConnect(HANDLE hNetlibUser, LPSTR lpszHost, DWORD dwPort)
{
	NETLIBOPENCONNECTION nloc = {0};
	nloc.cbSize = sizeof(nloc);
	nloc.flags = (NLOCF_HTTP|NLOCF_V2);
	nloc.szHost = lpszHost;
	nloc.wPort = ( IsHTTPSProxyUsed(hNetlibUser)) ? MRA_SERVER_PORT_HTTPS : dwPort;
	nloc.timeout = db_get_dw(NULL, MRA_AVT_SECT_NAME, "TimeOutConnect", MRA_AVT_DEFAULT_TIMEOUT_CONN);
	if (nloc.timeout<MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout>MRA_TIMEOUT_CONN_МАХ) nloc.timeout = MRA_TIMEOUT_CONN_МАХ;

	DWORD dwConnectReTryCount = db_get_dw(NULL, MRA_AVT_SECT_NAME, "ConnectReTryCount", MRA_AVT_DEFAULT_CONN_RETRY_COUNT);
	DWORD dwCurConnectReTryCount = dwConnectReTryCount;
	HANDLE hConnection;
	do {
		hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&nloc);
	}
	while (--dwCurConnectReTryCount && hConnection == NULL);

	return hConnection;
}

DWORD MraAvatarsHttpTransaction(HANDLE hConnection, DWORD dwRequestType, LPSTR lpszUser, LPSTR lpszDomain, LPSTR lpszHost, DWORD dwReqObj, BOOL bUseKeepAliveConn, DWORD *pdwResultCode, BOOL *pbKeepAlive, DWORD *pdwFormat, size_t *pdwAvatarSize, INTERNET_TIME *pitLastModifiedTime)
{
	if (pdwResultCode)      *pdwResultCode = 0;
	if (pbKeepAlive)        *pbKeepAlive = FALSE;
	if (pdwFormat)          *pdwFormat = PA_FORMAT_UNKNOWN;
	if (pdwAvatarSize)      *pdwAvatarSize = 0;
	if (pitLastModifiedTime) bzero(pitLastModifiedTime, sizeof(INTERNET_TIME));

	if (!hConnection)
		return ERROR_INVALID_HANDLE;

	LPSTR lpszReqObj;

	switch (dwReqObj) {
		case MAHTRO_AVT:          lpszReqObj = "_avatar"; break;
		case MAHTRO_AVTMRIM:      lpszReqObj = "_mrimavatar"; break;
		case MAHTRO_AVTSMALL:     lpszReqObj = "_avatarsmall"; break;
		case MAHTRO_AVTSMALLMRIM: lpszReqObj = "_mrimavatarsmall"; break;
		default:                  lpszReqObj = ""; break;
	}

	char szBuff[4096], szSelfVersionString[MAX_PATH];
	DWORD dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "http://%s/%s/%s/%s", lpszHost, lpszDomain, lpszUser, lpszReqObj);
	MraGetSelfVersionString(szSelfVersionString, SIZEOF(szSelfVersionString), NULL);

	NETLIBHTTPHEADER nlbhHeaders[8] = {0};
	nlbhHeaders[0].szName = "User-Agent";		 nlbhHeaders[0].szValue = szSelfVersionString;
	nlbhHeaders[1].szName = "Accept-Encoding"; nlbhHeaders[1].szValue = "deflate";
	nlbhHeaders[2].szName = "Pragma";			 nlbhHeaders[2].szValue = "no-cache";
	nlbhHeaders[3].szName = "Connection";		 nlbhHeaders[3].szValue = (bUseKeepAliveConn)? "keep-alive":"close";

	NETLIBHTTPREQUEST *pnlhr, nlhr = {0};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = dwRequestType;
	nlhr.flags = (NLHRF_GENERATEHOST|NLHRF_SMARTREMOVEHOST|NLHRF_SMARTAUTHHEADER);
	nlhr.szUrl = szBuff;
	nlhr.headers = (NETLIBHTTPHEADER*)&nlbhHeaders;
	nlhr.headersCount = 4;

	DWORD dwSent = CallService(MS_NETLIB_SENDHTTPREQUEST, (WPARAM)hConnection, (LPARAM)&nlhr);
	if (dwSent == SOCKET_ERROR || !dwSent)
		return GetLastError();

	pnlhr = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_RECVHTTPHEADERS, (WPARAM)hConnection, 0);
	if (!pnlhr)
		return GetLastError();

	for (int i = 0; i < pnlhr->headersCount; i++) {
		if ( !_strnicmp(pnlhr->headers[i].szName, "Connection", 10)) {
			if (pbKeepAlive)
				*pbKeepAlive = !_strnicmp(pnlhr->headers[i].szValue, "keep-alive", 10);
		}
		else if ( !_strnicmp(pnlhr->headers[i].szName, "Content-Type", 12)) {
			if (pdwFormat) {
				for (DWORD j = 0; j < PA_FORMAT_MAX; j++) {
					if ( !_stricmp(pnlhr->headers[i].szValue, lpcszContentType[j])) {
						*pdwFormat = j;
						break;
					}
				}
			}
		}
		else if ( !_strnicmp(pnlhr->headers[i].szName, "Content-Length", 14)) {
			if (pdwAvatarSize)
				*pdwAvatarSize = StrToUNum(pnlhr->headers[i].szValue, lstrlenA(pnlhr->headers[i].szValue));
		}
		else if ( !_strnicmp(pnlhr->headers[i].szName, "Last-Modified", 13)) {
			if (pitLastModifiedTime)
				InternetTimeGetTime(pnlhr->headers[i].szValue, lstrlenA(pnlhr->headers[i].szValue), pitLastModifiedTime);
		}
	}

	if (pdwResultCode) (*pdwResultCode) = pnlhr->resultCode;
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	return 0;
}

BOOL CMraProto::MraAvatarsGetContactTime(HANDLE hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime)
{
	if (lpszValueName && pstTime) {
		char szBuff[MAX_PATH];
		size_t dwBuffSize;
		INTERNET_TIME itAvatarLastModifiedTimeLocal;

		if (mraGetStaticStringA(hContact, lpszValueName, (LPSTR)szBuff, SIZEOF(szBuff), &dwBuffSize))
		if (InternetTimeGetTime(szBuff, dwBuffSize, &itAvatarLastModifiedTimeLocal) == NO_ERROR) {
			memmove(pstTime, &itAvatarLastModifiedTimeLocal.stTime, sizeof(SYSTEMTIME));
			return TRUE;
		}
	}
	return FALSE;
}


void CMraProto::MraAvatarsSetContactTime(HANDLE hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime)
{
	if (!lpszValueName)
		return;

	char szBuff[MAX_PATH];
	size_t dwBuffUsedSize;
	INTERNET_TIME itTime;

	if (pstTime) {
		itTime.lTimeZone = 0;
		memmove(&itTime.stTime, pstTime, sizeof(SYSTEMTIME));
	}
	else InternetTimeGetCurrentTime(&itTime);

	if (itTime.stTime.wYear) {
		InternetTimeGetString(&itTime, (LPSTR)szBuff, SIZEOF(szBuff), &dwBuffUsedSize);
		mraSetStringExA(hContact, lpszValueName, (LPSTR)szBuff, dwBuffUsedSize);
	}
	else delSetting(hContact, lpszValueName);
}

DWORD MraAvatarsGetFileFormat(LPTSTR lpszPath, size_t dwPathSize)
{
	TCHAR dwExt[ 5 ];
	BuffToLowerCase(&dwExt, lpszPath+(dwPathSize-4), 4);

	for ( DWORD i = 0; i < PA_FORMAT_MAX; i++ )
		if ( !_tcscmp( dwExt, lpcszExtensions[i]))
			return i;

	return -1;
}


DWORD CMraProto::MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle, HANDLE hContact, DWORD dwFormat, LPTSTR lpszPath, size_t dwPathSize, size_t *pdwPathSizeRet)
{
	if (hAvatarsQueueHandle == NULL)
		return ERROR_INVALID_HANDLE;

	if (IsContactChatAgent(hContact))
		return ERROR_NOT_SUPPORTED;

	LPTSTR lpszCurPath = lpszPath;
	size_t dwEMailSize;
	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;

	if (pmraaqAvatarsQueue->hAvatarsPath == NULL)
		// default path
		dwEMailSize = mir_sntprintf(lpszPath, dwPathSize, _T("%s\\%s\\"), (TCHAR*)VARST( _T("%miranda_avatarcache%")), m_tszUserName);
	else {
		FoldersGetCustomPathT(pmraaqAvatarsQueue->hAvatarsPath, lpszCurPath, int(dwPathSize), _T(""));
		dwEMailSize = lstrlen( lpszCurPath );
		_tcscpy( lpszCurPath + dwEMailSize, _T("\\"));
		dwEMailSize++;
	}

	dwPathSize -= dwEMailSize;
	lpszCurPath += dwEMailSize;

	if (dwPathSize) {
		// some path in buff and free space for file name is avaible
		CreateDirectoryTreeT(lpszPath);

		if (dwFormat != PA_FORMAT_DEFAULT) {
			if ( mraGetStaticStringW(hContact, "e-mail", lpszCurPath, dwPathSize-5, &dwEMailSize)) {
				BuffToLowerCase(lpszCurPath, lpszCurPath, dwEMailSize);
				lpszCurPath += dwEMailSize;
				_tcscpy( lpszCurPath, lpcszExtensions[dwFormat] );
				lpszCurPath += 4;
				*lpszCurPath = 0;

				if ( pdwPathSizeRet )
					*pdwPathSizeRet = lpszCurPath - lpszPath;
				return NO_ERROR;
			}
		}
		else {
			if ( DB_GetStaticStringW(NULL, MRA_AVT_SECT_NAME, "DefaultAvatarFileName", lpszCurPath, dwPathSize-5, &dwEMailSize ) == FALSE) {
				_tcscpy(lpszCurPath, MRA_AVT_DEFAULT_AVT_FILENAME);
				lpszCurPath += SIZEOF( MRA_AVT_DEFAULT_AVT_FILENAME )-1;

				if (pdwPathSizeRet)
					*pdwPathSizeRet = lpszCurPath - lpszPath;
				return NO_ERROR;
			}
		}
	}

	return ERROR_INSUFFICIENT_BUFFER;
}

DWORD CMraProto::MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD *pdwAvatarsQueueID, DWORD *pdwFormat, LPTSTR lpszPath)
{
	DWORD dwRetCode = GAIR_NOAVATAR;

	if (hAvatarsQueueHandle)
	if (db_get_b(NULL, MRA_AVT_SECT_NAME, "Enable", MRA_AVT_DEFAULT_ENABLE))
	if (IsContactChatAgent(hContact) == FALSE) {
		// not @chat.agent conference
		BOOL bQueueAdd = TRUE;// check for updates
		MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
		SYSTEMTIME stAvatarLastCheckTime;

		if ((dwFlags & GAIF_FORCE) == 0)// если флаг принудит. обновления, то даже не проверяем времени последнего обновления
		if (MraAvatarsGetContactTime(hContact, "AvatarLastCheckTime", &stAvatarLastCheckTime)) {
			TCHAR wszFileName[MAX_FILEPATH];
			size_t dwPathSize;
			FILETIME ftCurrentTime, ftExpireTime;

			GetSystemTimeAsFileTime(&ftCurrentTime);
			SystemTimeToFileTime(&stAvatarLastCheckTime, &ftExpireTime);
			(*((DWORDLONG*)&ftExpireTime)) += (FILETIME_MINUTE*(DWORDLONG)db_get_dw(NULL, MRA_AVT_SECT_NAME, "CheckInterval", MRA_AVT_DEFAULT_CHK_INTERVAL));

			if ((*((DWORDLONG*)&ftExpireTime))>(*((DWORDLONG*)&ftCurrentTime)))
			if ( MraAvatarsGetFileName(hAvatarsQueueHandle, hContact, GetContactAvatarFormat(hContact, PA_FORMAT_DEFAULT), wszFileName, SIZEOF(wszFileName), &dwPathSize) == NO_ERROR)
			if ( IsFileExist( wszFileName )) {
				// файл с аватаром существует и не устарел/не было комманды обновлять(просто запрос имени)
				if (lpszPath) {
					if (db_get_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", MRA_AVT_DEFAULT_RET_ABC_PATH))
						lstrcpyn(lpszPath, wszFileName, MAX_PATH);
					else
						PathToRelativeT(wszFileName, lpszPath);
				}
				if (pdwFormat)
					*pdwFormat = GetContactAvatarFormat(hContact, PA_FORMAT_DEFAULT);
				dwRetCode = GAIR_SUCCESS;
				bQueueAdd = FALSE;
			}
		}

		if (bQueueAdd || (dwFlags & GAIF_FORCE))
		if ( !MraAvatarsQueueAdd(hAvatarsQueueHandle, dwFlags, hContact, pdwAvatarsQueueID)) {
			MraAvatarsSetContactTime(hContact, "AvatarLastCheckTime", NULL);
			dwRetCode = GAIR_WAITFOR;
		}
	}
	return dwRetCode;
}

DWORD CMraProto::MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwSourceID)
{
	DWORD dwRetCode = GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATIONT pai = {0};
	pai.cbSize = sizeof(pai);
	pai.hContact = hContact;
	if ((dwRetCode = MraAvatarsQueueGetAvatar(hAvatarsQueueHandle, dwFlags, hContact, NULL, (DWORD*)&pai.format, pai.filename )) == GAIR_SUCCESS ) {
		// write owner avatar file name to DB
		if (hContact == NULL)
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);

		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
	}
	return dwRetCode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Avatars options

WORD wMraAvatarsControlsList[] = {
	IDC_SERVER,
	IDC_SERVERPORT,
	IDC_BUTTON_DEFAULT,
	IDC_USE_KEEPALIVE_CONN,
	IDC_UPD_CHECK_INTERVAL,
	IDC_RETURN_ABC_PATH,
	IDC_DELETE_AVT_ON_CONTACT_DELETE
};

INT_PTR CALLBACK MraAvatarsQueueDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;
		{
			WCHAR szServer[MAX_PATH];

			CHECK_DLG_BUTTON(hWndDlg, IDC_ENABLE, db_get_b(NULL, MRA_AVT_SECT_NAME, "Enable", MRA_AVT_DEFAULT_ENABLE));

			if (DB_GetStaticStringW(NULL, MRA_AVT_SECT_NAME, "Server", szServer, MAX_PATH, NULL))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_SERVER, szServer);
			else
				SET_DLG_ITEM_TEXTA(hWndDlg, IDC_SERVER, MRA_AVT_DEFAULT_SERVER);

			SetDlgItemInt(hWndDlg, IDC_SERVERPORT, db_get_dw(NULL, MRA_AVT_SECT_NAME, "ServerPort", MRA_AVT_DEFAULT_SERVER_PORT), FALSE);
			CHECK_DLG_BUTTON(hWndDlg, IDC_USE_KEEPALIVE_CONN, db_get_b(NULL, MRA_AVT_SECT_NAME, "UseKeepAliveConn", MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN));
			SetDlgItemInt(hWndDlg, IDC_UPD_CHECK_INTERVAL, db_get_dw(NULL, MRA_AVT_SECT_NAME, "CheckInterval", MRA_AVT_DEFAULT_CHK_INTERVAL), FALSE);
			CHECK_DLG_BUTTON(hWndDlg, IDC_RETURN_ABC_PATH, db_get_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", MRA_AVT_DEFAULT_RET_ABC_PATH));
			CHECK_DLG_BUTTON(hWndDlg, IDC_DELETE_AVT_ON_CONTACT_DELETE, db_get_b(NULL, MRA_AVT_SECT_NAME, "DeleteAvtOnContactDelete", MRA_DELETE_AVT_ON_CONTACT_DELETE));

			EnableControlsArray(hWndDlg, (WORD*)&wMraAvatarsControlsList, SIZEOF(wMraAvatarsControlsList), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_ENABLE));
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ENABLE)
			EnableControlsArray(hWndDlg, (WORD*)&wMraAvatarsControlsList, SIZEOF(wMraAvatarsControlsList), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_ENABLE));

		if (LOWORD(wParam) == IDC_BUTTON_DEFAULT) {
			SET_DLG_ITEM_TEXTA(hWndDlg, IDC_SERVER, MRA_AVT_DEFAULT_SERVER);
			SetDlgItemInt(hWndDlg, IDC_SERVERPORT, MRA_AVT_DEFAULT_SERVER_PORT, FALSE);
		}

		if ((LOWORD(wParam) == IDC_SERVER || LOWORD(wParam) == IDC_SERVERPORT || LOWORD(wParam) == IDC_UPD_CHECK_INTERVAL) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return FALSE;
		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			{
				WCHAR szServer[MAX_PATH];

				db_set_b(NULL, MRA_AVT_SECT_NAME, "Enable", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_ENABLE));
				db_set_b(NULL, MRA_AVT_SECT_NAME, "DeleteAvtOnContactDelete", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_DELETE_AVT_ON_CONTACT_DELETE));
				db_set_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_RETURN_ABC_PATH));
				db_set_dw(NULL, MRA_AVT_SECT_NAME, "CheckInterval", GetDlgItemInt(hWndDlg, IDC_UPD_CHECK_INTERVAL, NULL, FALSE));
				db_set_b(NULL, MRA_AVT_SECT_NAME, "UseKeepAliveConn", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_USE_KEEPALIVE_CONN));
				db_set_dw(NULL, MRA_AVT_SECT_NAME, "ServerPort", GetDlgItemInt(hWndDlg, IDC_SERVERPORT, NULL, FALSE));

				GET_DLG_ITEM_TEXT(hWndDlg, IDC_SERVER, szServer, SIZEOF(szServer));
				db_set_ws(NULL, MRA_AVT_SECT_NAME, "Server", szServer);
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}


DWORD CMraProto::MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle, HANDLE hContact)
{
	if (hAvatarsQueueHandle == NULL)
		return ERROR_INVALID_HANDLE;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;

	DWORD dwAvatarFormat = GetContactAvatarFormat(hContact, PA_FORMAT_UNKNOWN);
	if ( db_get_b(NULL, MRA_AVT_SECT_NAME, "DeleteAvtOnContactDelete", MRA_DELETE_AVT_ON_CONTACT_DELETE) && dwAvatarFormat != PA_FORMAT_DEFAULT) {
		TCHAR szFileName[MAX_FILEPATH];
		if ( !MraAvatarsGetFileName(hAvatarsQueueHandle, hContact, dwAvatarFormat, szFileName, SIZEOF(szFileName), NULL))
			return DeleteFile(szFileName);
	}
	return NO_ERROR;
}
