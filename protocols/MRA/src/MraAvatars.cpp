#include "stdafx.h"
#include "MraAvatars.h"

#define PA_FORMAT_MAX		7

const LPSTR lpcszContentType[9] =
{
	"", 			// PA_FORMAT_UNKNOWN
	"image/png", 		// PA_FORMAT_PNG
	"image/jpeg", 		// PA_FORMAT_JPEG
	"image/icon", 		// PA_FORMAT_ICON
	"image/x-xbitmap", 	// PA_FORMAT_BMP
	"image/gif", 		// PA_FORMAT_GIF
	"", 			// PA_FORMAT_SWF
	"", 			// PA_FORMAT_XML
	NULL
};

struct MRA_AVATARS_QUEUE : public FIFO_MT
{
	HANDLE hNetlibUser;
	HANDLE hThreadEvents[MAXIMUM_WAIT_OBJECTS];
	int    iThreadsCount, iThreadsRunning;
};

struct MRA_AVATARS_QUEUE_ITEM : public FIFO_MT_ITEM
{
	DWORD  dwAvatarsQueueID;
	DWORD  dwFlags;
	MCONTACT hContact;
};

#define FILETIME_SECOND		((DWORDLONG)10000000)
#define FILETIME_MINUTE		((DWORDLONG)FILETIME_SECOND * 60)


char szAvtSectName[MAX_PATH];
#define MRA_AVT_SECT_NAME	szAvtSectName

HANDLE MraAvatarsHttpConnect(HANDLE hNetlibUser, LPCSTR lpszHost, DWORD dwPort);

#define MAHTRO_AVT		0
#define MAHTRO_AVTMRIM		1
#define MAHTRO_AVTSMALL		2
#define MAHTRO_AVTSMALLMRIM	3

DWORD MraAvatarsHttpTransaction(HANDLE hConnection, DWORD dwRequestType, LPCSTR lpszUser, LPCSTR lpszDomain, LPCSTR lpszHost, DWORD dwReqObj, BOOL bUseKeepAliveConn, DWORD *pdwResultCode, BOOL *pbKeepAlive, DWORD *pdwFormat, size_t *pdwAvatarSize, INTERNET_TIME *pitLastModifiedTime);

DWORD CMraProto::MraAvatarsQueueInitialize(HANDLE *phAvatarsQueueHandle)
{
	mir_snprintf(szAvtSectName, SIZEOF(szAvtSectName), "%s Avatars", m_szModuleName);

	if (phAvatarsQueueHandle == NULL)
		return ERROR_INVALID_HANDLE;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = new MRA_AVATARS_QUEUE();

	TCHAR szBuffer[MAX_PATH];
	mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s %s"), m_tszUserName, TranslateT("Avatars' plugin connections"));

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MRA_AVT_SECT_NAME;
	nlu.ptszDescriptiveName = szBuffer;
	pmraaqAvatarsQueue->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	if (pmraaqAvatarsQueue->hNetlibUser) {
		pmraaqAvatarsQueue->iThreadsCount = db_get_dw(NULL, MRA_AVT_SECT_NAME, "WorkThreadsCount", MRA_AVT_DEFAULT_WRK_THREAD_COUNTS);
		if (pmraaqAvatarsQueue->iThreadsCount == 0)
			pmraaqAvatarsQueue->iThreadsCount = 1;
		if (pmraaqAvatarsQueue->iThreadsCount > MAXIMUM_WAIT_OBJECTS)
			pmraaqAvatarsQueue->iThreadsCount = MAXIMUM_WAIT_OBJECTS;

		pmraaqAvatarsQueue->iThreadsRunning = 0;
		for (int i = 0; i < pmraaqAvatarsQueue->iThreadsCount; i++)
			ForkThread(&CMraProto::MraAvatarsThreadProc, pmraaqAvatarsQueue);

		*phAvatarsQueueHandle = (HANDLE)pmraaqAvatarsQueue;
	}
	return NO_ERROR;
}

void CMraProto::MraAvatarsQueueClear(HANDLE hAvatarsQueueHandle)
{
	if (!hAvatarsQueueHandle)
		return;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;

	PROTO_AVATAR_INFORMATIONT pai = { 0 };
	pai.cbSize = sizeof(pai);
	pai.format = PA_FORMAT_UNKNOWN;

	while (FifoMTItemPop(pmraaqAvatarsQueue, NULL, (LPVOID*)&pmraaqiAvatarsQueueItem) == NO_ERROR) {
		pai.hContact = pmraaqiAvatarsQueueItem->hContact;
		ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, 0);
		mir_free(pmraaqiAvatarsQueueItem);
	}
}

void CMraProto::MraAvatarsQueueSuspend(HANDLE hAvatarsQueueHandle)
{
	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
	MraAvatarsQueueClear(hAvatarsQueueHandle);
	for (int i = 0; i < pmraaqAvatarsQueue->iThreadsCount; i++)
		SetEvent(pmraaqAvatarsQueue->hThreadEvents[i]);
}

void CMraProto::MraAvatarsQueueDestroy(HANDLE hAvatarsQueueHandle)
{
	if (!hAvatarsQueueHandle)
		return;

	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
	Netlib_CloseHandle(pmraaqAvatarsQueue->hNetlibUser);
	delete pmraaqAvatarsQueue;
}

DWORD CMraProto::MraAvatarsQueueAdd(HANDLE hAvatarsQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD *pdwAvatarsQueueID)
{
	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)hAvatarsQueueHandle;
	if (pmraaqAvatarsQueue == NULL || g_bShutdown)
		return ERROR_INVALID_HANDLE;

	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem = (MRA_AVATARS_QUEUE_ITEM*)mir_calloc(sizeof(MRA_AVATARS_QUEUE_ITEM));
	if (!pmraaqiAvatarsQueueItem)
		return GetLastError();

	pmraaqiAvatarsQueueItem->dwAvatarsQueueID = GetTickCount();
	pmraaqiAvatarsQueueItem->dwFlags = dwFlags;
	pmraaqiAvatarsQueueItem->hContact = hContact;

	FifoMTItemPush(pmraaqAvatarsQueue, pmraaqiAvatarsQueueItem, (LPVOID)pmraaqiAvatarsQueueItem);
	if (pdwAvatarsQueueID)
		*pdwAvatarsQueueID = pmraaqiAvatarsQueueItem->dwAvatarsQueueID;

	mir_cslock(pmraaqAvatarsQueue->cs);
	int threadno = (pmraaqAvatarsQueue->iThreadsRunning + 1) % pmraaqAvatarsQueue->iThreadsCount;
	SetEvent(pmraaqAvatarsQueue->hThreadEvents[threadno]);
	return NO_ERROR;
}

void CMraProto::MraAvatarsThreadProc(LPVOID lpParameter)
{
	MRA_AVATARS_QUEUE *pmraaqAvatarsQueue = (MRA_AVATARS_QUEUE*)lpParameter;
	MRA_AVATARS_QUEUE_ITEM *pmraaqiAvatarsQueueItem;

	CMStringA szEmail, szServer;
	CMStringW wszFileName;
	BOOL bContinue, bKeepAlive, bUseKeepAliveConn, bFailed, bDownloadNew;
	BYTE btBuff[BUFF_SIZE_RCV];
	DWORD dwResultCode, dwAvatarFormat = PA_FORMAT_DEFAULT, dwReceived, dwServerPort, dwErrorCode;
	size_t dwAvatarSizeServer;
	FILETIME ftLastModifiedTimeServer, ftLastModifiedTimeLocal;
	SYSTEMTIME stAvatarLastModifiedTimeLocal;
	HANDLE hConnection = NULL;
	NETLIBSELECT nls = { 0 };
	INTERNET_TIME itAvatarLastModifiedTimeServer;
	PROTO_AVATAR_INFORMATIONT pai;
	WCHAR szErrorText[2048];

	nls.cbSize = sizeof(nls);
	pai.cbSize = sizeof(pai);

	HANDLE hThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	{
		mir_cslock lck(pmraaqAvatarsQueue->cs);
		pmraaqAvatarsQueue->hThreadEvents[pmraaqAvatarsQueue->iThreadsRunning++] = hThreadEvent;
	}

	while (!g_bShutdown) {
		if (FifoMTItemPop(pmraaqAvatarsQueue, NULL, (LPVOID*)&pmraaqiAvatarsQueueItem) != NO_ERROR) { // waiting until service stop or new task
			NETLIB_CLOSEHANDLE(hConnection);
			WaitForSingleObjectEx(hThreadEvent, MRA_AVT_DEFAULT_QE_CHK_INTERVAL, FALSE);
			continue;
		}
		
		// Try download.
		bFailed = TRUE;
		bDownloadNew = FALSE;

		if (!DB_GetStringA(NULL, MRA_AVT_SECT_NAME, "Server", szServer))
			szServer = MRA_AVT_DEFAULT_SERVER;
		dwServerPort = db_get_dw(NULL, MRA_AVT_SECT_NAME, "ServerPort", MRA_AVT_DEFAULT_SERVER_PORT);
		bUseKeepAliveConn = db_get_b(NULL, MRA_AVT_SECT_NAME, "UseKeepAliveConn", MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN);

		if (mraGetStringA(pmraaqiAvatarsQueueItem->hContact, "e-mail", szEmail)) {
			szEmail.MakeLower();

			int iStart = 0;
			CMStringA szUser = szEmail.Tokenize("@", iStart);
			CMStringA szDomain = szEmail.Tokenize("@", iStart);
			if (!szUser.IsEmpty() && !szDomain.IsEmpty()) {
				ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_CONNECTING, NULL, 0);
				if (hConnection == NULL)
					hConnection = MraAvatarsHttpConnect(pmraaqAvatarsQueue->hNetlibUser, szServer, dwServerPort);
				if (hConnection) {
					ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_CONNECTED, NULL, 0);
					ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_SENTREQUEST, NULL, 0);
					if (!MraAvatarsHttpTransaction(hConnection, REQUEST_HEAD, szUser, szDomain, szServer, MAHTRO_AVTMRIM, bUseKeepAliveConn, &dwResultCode, &bKeepAlive, &dwAvatarFormat, &dwAvatarSizeServer, &itAvatarLastModifiedTimeServer)) {
						switch (dwResultCode) {
						case 200:
							if (MraAvatarsGetContactTime(pmraaqiAvatarsQueueItem->hContact, "AvatarLastModifiedTime", &stAvatarLastModifiedTimeLocal)) {
								SystemTimeToFileTime(&itAvatarLastModifiedTimeServer.stTime, &ftLastModifiedTimeServer);
								SystemTimeToFileTime(&stAvatarLastModifiedTimeLocal, &ftLastModifiedTimeLocal);

								if ((*((DWORDLONG*)&ftLastModifiedTimeServer)) != (*((DWORDLONG*)&ftLastModifiedTimeLocal))) {// need check for update
									bDownloadNew = TRUE;
									//ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, 0, 0);
								}
								else {// avatar is valid
									if (MraAvatarsGetFileName(pmraaqAvatarsQueue, pmraaqiAvatarsQueueItem->hContact, dwAvatarFormat, wszFileName) == NO_ERROR) {
										if (IsFileExist(wszFileName))
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
							if (MraAvatarsGetFileName((HANDLE)pmraaqAvatarsQueue, NULL, PA_FORMAT_DEFAULT, wszFileName) == NO_ERROR) {
								if (IsFileExist(wszFileName)) {
									dwAvatarFormat = ProtoGetAvatarFormat(wszFileName);
									bFailed = FALSE;
								}
								else//loading default avatar
									bDownloadNew = TRUE;
							}
							break;

						default:
							mir_sntprintf(szErrorText, SIZEOF(szErrorText), TranslateT("Avatars: server return HTTP code: %lu"), dwResultCode);
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
						if (MraAvatarsHttpTransaction(hConnection, REQUEST_GET, szUser, szDomain, szServer, MAHTRO_AVT, bUseKeepAliveConn, &dwResultCode, &bKeepAlive, &dwAvatarFormat, &dwAvatarSizeServer, &itAvatarLastModifiedTimeServer) == NO_ERROR && dwResultCode == 200) {
							if (!MraAvatarsGetFileName(pmraaqAvatarsQueue, pmraaqiAvatarsQueueItem->hContact, dwAvatarFormat, wszFileName)) {
								HANDLE hFile = CreateFile(wszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile != INVALID_HANDLE_VALUE) {
									DWORD dwWritten = 0;
									bContinue = TRUE;
									nls.dwTimeout = (1000 * db_get_dw(NULL, MRA_AVT_SECT_NAME, "TimeOutReceive", MRA_AVT_DEFAULT_TIMEOUT_RECV));
									nls.hReadConns[0] = hConnection;

									while (bContinue) {
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
												if (WriteFile(hFile, (LPVOID)&btBuff, dwReceived, &dwReceived, NULL)) {
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
									mir_sntprintf(szErrorText, SIZEOF(szErrorText), TranslateT("Avatars: can't open file %s, error"), wszFileName);
									ShowFormattedErrorMessage(szErrorText, dwErrorCode);
								}
							}
						}
						else _CrtDbgBreak();

						if (bUseKeepAliveConn == FALSE || bKeepAlive == FALSE)
							NETLIB_CLOSEHANDLE(hConnection);
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
			if (db_get_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", MRA_AVT_DEFAULT_RET_ABC_PATH))
				mir_tstrncpy(pai.filename, wszFileName, SIZEOF(pai.filename));
			else
				PathToRelativeT(wszFileName, pai.filename);

			SetContactAvatarFormat(pmraaqiAvatarsQueueItem->hContact, dwAvatarFormat);
			MraAvatarsSetContactTime(pmraaqiAvatarsQueueItem->hContact, "AvatarLastModifiedTime", &itAvatarLastModifiedTimeServer.stTime);
			// write owner avatar file name to DB
			if (pmraaqiAvatarsQueueItem->hContact == NULL) // proto avatar
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);

			ProtoBroadcastAck(pmraaqiAvatarsQueueItem->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
		}
		mir_free(pmraaqiAvatarsQueueItem);
	}
	CloseHandle(hThreadEvent);
}

HANDLE MraAvatarsHttpConnect(HANDLE hNetlibUser, LPCSTR lpszHost, DWORD dwPort)
{
	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof(nloc);
	nloc.flags = (NLOCF_HTTP | NLOCF_V2);
	nloc.szHost = lpszHost;
	nloc.wPort = (IsHTTPSProxyUsed(hNetlibUser)) ? MRA_SERVER_PORT_HTTPS : dwPort;
	nloc.timeout = db_get_dw(NULL, MRA_AVT_SECT_NAME, "TimeOutConnect", MRA_AVT_DEFAULT_TIMEOUT_CONN);
	if (nloc.timeout < MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
	if (nloc.timeout > MRA_TIMEOUT_CONN_MAX) nloc.timeout = MRA_TIMEOUT_CONN_MAX;

	DWORD dwConnectReTryCount = db_get_dw(NULL, MRA_AVT_SECT_NAME, "ConnectReTryCount", MRA_AVT_DEFAULT_CONN_RETRY_COUNT);
	DWORD dwCurConnectReTryCount = dwConnectReTryCount;
	HANDLE hConnection;
	do {
		hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&nloc);
	}
		while (--dwCurConnectReTryCount && hConnection == NULL);

	return hConnection;
}

DWORD MraAvatarsHttpTransaction(HANDLE hConnection, DWORD dwRequestType, LPCSTR lpszUser, LPCSTR lpszDomain, LPCSTR lpszHost, DWORD dwReqObj, BOOL bUseKeepAliveConn, DWORD *pdwResultCode, BOOL *pbKeepAlive, DWORD *pdwFormat, size_t *pdwAvatarSize, INTERNET_TIME *pitLastModifiedTime)
{
	if (pdwResultCode)      *pdwResultCode = 0;
	if (pbKeepAlive)        *pbKeepAlive = FALSE;
	if (pdwFormat)          *pdwFormat = PA_FORMAT_UNKNOWN;
	if (pdwAvatarSize)      *pdwAvatarSize = 0;
	if (pitLastModifiedTime) memset(pitLastModifiedTime, 0, sizeof(INTERNET_TIME));

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

	char szBuff[4096];
	mir_snprintf(szBuff, SIZEOF(szBuff), "http://%s/%s/%s/%s", lpszHost, lpszDomain, lpszUser, lpszReqObj);
	CMStringA szSelfVersionString = MraGetSelfVersionString();

	NETLIBHTTPHEADER nlbhHeaders[8] = { 0 };
	nlbhHeaders[0].szName = "User-Agent";		nlbhHeaders[0].szValue = (LPSTR)szSelfVersionString.c_str();
	nlbhHeaders[1].szName = "Accept-Encoding";	nlbhHeaders[1].szValue = "deflate";
	nlbhHeaders[2].szName = "Pragma";		nlbhHeaders[2].szValue = "no-cache";
	nlbhHeaders[3].szName = "Connection";		nlbhHeaders[3].szValue = (bUseKeepAliveConn) ? "keep-alive" : "close";

	NETLIBHTTPREQUEST nlhr = { 0 };
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = dwRequestType;
	nlhr.flags = (NLHRF_GENERATEHOST | NLHRF_SMARTREMOVEHOST | NLHRF_SMARTAUTHHEADER);
	nlhr.szUrl = szBuff;
	nlhr.headers = (NETLIBHTTPHEADER*)&nlbhHeaders;
	nlhr.headersCount = 4;

	DWORD dwSent = CallService(MS_NETLIB_SENDHTTPREQUEST, (WPARAM)hConnection, (LPARAM)&nlhr);
	if (dwSent == SOCKET_ERROR || !dwSent)
		return GetLastError();

	NETLIBHTTPREQUEST *pnlhr = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_RECVHTTPHEADERS, (WPARAM)hConnection, 0);
	if (!pnlhr)
		return GetLastError();

	for (int i = 0; i < pnlhr->headersCount; i++) {
		if (!_strnicmp(pnlhr->headers[i].szName, "Connection", 10)) {
			if (pbKeepAlive)
				*pbKeepAlive = !_strnicmp(pnlhr->headers[i].szValue, "keep-alive", 10);
		}
		else if (!_strnicmp(pnlhr->headers[i].szName, "Content-Type", 12)) {
			if (pdwFormat) {
				for (DWORD j = 0; j < PA_FORMAT_MAX; j++) {
					if (!_stricmp(pnlhr->headers[i].szValue, lpcszContentType[j])) {
						*pdwFormat = j;
						break;
					}
				}
			}
		}
		else if (!_strnicmp(pnlhr->headers[i].szName, "Content-Length", 14)) {
			if (pdwAvatarSize)
				*pdwAvatarSize = atol(pnlhr->headers[i].szValue);
		}
		else if (!_strnicmp(pnlhr->headers[i].szName, "Last-Modified", 13)) {
			if (pitLastModifiedTime)
				InternetTimeGetTime(pnlhr->headers[i].szValue, *pitLastModifiedTime);
		}
	}

	if (pdwResultCode)
		*pdwResultCode = pnlhr->resultCode;
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	return 0;
}

bool CMraProto::MraAvatarsGetContactTime(MCONTACT hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime)
{
	if (lpszValueName && pstTime) {
		INTERNET_TIME itAvatarLastModifiedTimeLocal;
		CMStringA szBuff;
		if (mraGetStringA(hContact, lpszValueName, szBuff))
		if (InternetTimeGetTime(szBuff, itAvatarLastModifiedTimeLocal) == NO_ERROR) {
			memcpy(pstTime, &itAvatarLastModifiedTimeLocal.stTime, sizeof(SYSTEMTIME));
			return true;
		}
	}
	return false;
}

void CMraProto::MraAvatarsSetContactTime(MCONTACT hContact, LPSTR lpszValueName, SYSTEMTIME *pstTime)
{
	if (!lpszValueName)
		return;

	INTERNET_TIME itTime;
	if (pstTime) {
		itTime.lTimeZone = 0;
		memcpy(&itTime.stTime, pstTime, sizeof(SYSTEMTIME));
	}
	else InternetTimeGetCurrentTime(&itTime);

	if (itTime.stTime.wYear)
		mraSetStringExA(hContact, lpszValueName, InternetTimeGetString(&itTime));
	else
		delSetting(hContact, lpszValueName);
}

DWORD CMraProto::MraAvatarsGetFileName(HANDLE hAvatarsQueueHandle, MCONTACT hContact, DWORD dwFormat, CMStringW &res)
{
	res.Empty();
	if (hAvatarsQueueHandle == NULL)
		return ERROR_INVALID_HANDLE;

	if (IsContactChatAgent(hContact))
		return ERROR_NOT_SUPPORTED;

	TCHAR tszBase[MAX_PATH];
	mir_sntprintf(tszBase, SIZEOF(tszBase), _T("%s\\%s\\"), VARST(_T("%miranda_avatarcache%")), m_tszUserName);
	res = tszBase;

	// some path in buff and free space for file name is avaible
	CreateDirectoryTreeT(res);

	if (dwFormat != PA_FORMAT_DEFAULT) {
		CMStringW szEmail;
		if (mraGetStringW(hContact, "e-mail", szEmail)) {
			szEmail.MakeLower();
			res += szEmail + ProtoGetAvatarExtension(dwFormat);
			return NO_ERROR;
		}
	}
	else {
		CMStringW szDefName;
		if (!DB_GetStringW(NULL, MRA_AVT_SECT_NAME, "DefaultAvatarFileName", szDefName)) {
			res += MRA_AVT_DEFAULT_AVT_FILENAME;
			return NO_ERROR;
		}
	}

	return ERROR_INSUFFICIENT_BUFFER;
}

DWORD CMraProto::MraAvatarsQueueGetAvatar(HANDLE hAvatarsQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD *pdwAvatarsQueueID, DWORD *pdwFormat, LPTSTR lpszPath)
{
	DWORD dwRetCode = GAIR_NOAVATAR;

	if ( !hAvatarsQueueHandle)
		return GAIR_NOAVATAR;
	if ( !db_get_b(NULL, MRA_AVT_SECT_NAME, "Enable", MRA_AVT_DEFAULT_ENABLE))
		return GAIR_NOAVATAR;
	if (IsContactChatAgent(hContact)) // @chat.agent conference
		return GAIR_NOAVATAR;

	BOOL bQueueAdd = TRUE;// check for updates
	SYSTEMTIME stAvatarLastCheckTime;

	if ((dwFlags & GAIF_FORCE) == 0)// если флаг принудит. обновления, то даже не проверяем времени последнего обновления
	if (MraAvatarsGetContactTime(hContact, "AvatarLastCheckTime", &stAvatarLastCheckTime)) {
		CMStringW wszFileName;
		FILETIME ftCurrentTime, ftExpireTime;

		GetSystemTimeAsFileTime(&ftCurrentTime);
		SystemTimeToFileTime(&stAvatarLastCheckTime, &ftExpireTime);
		(*((DWORDLONG*)&ftExpireTime)) += (FILETIME_MINUTE*(DWORDLONG)db_get_dw(NULL, MRA_AVT_SECT_NAME, "CheckInterval", MRA_AVT_DEFAULT_CHK_INTERVAL));

		if ((*((DWORDLONG*)&ftExpireTime)) > (*((DWORDLONG*)&ftCurrentTime)))
		if (MraAvatarsGetFileName(hAvatarsQueueHandle, hContact, GetContactAvatarFormat(hContact, PA_FORMAT_DEFAULT), wszFileName) == NO_ERROR)
		if (IsFileExist(wszFileName)) {
			// файл с аватаром существует и не устарел/не было комманды обновлять(просто запрос имени)
			if (lpszPath) {
				if (db_get_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", MRA_AVT_DEFAULT_RET_ABC_PATH))
					mir_tstrncpy(lpszPath, wszFileName, MAX_PATH);
				else
					PathToRelativeT(wszFileName, lpszPath);
			}
			if (pdwFormat)
				*pdwFormat = ProtoGetAvatarFormat(lpszPath);
			dwRetCode = GAIR_SUCCESS;
			bQueueAdd = FALSE;
		}
	}

	if (bQueueAdd || (dwFlags & GAIF_FORCE))
	if (!MraAvatarsQueueAdd(hAvatarsQueueHandle, dwFlags, hContact, pdwAvatarsQueueID)) {
		MraAvatarsSetContactTime(hContact, "AvatarLastCheckTime", NULL);
		dwRetCode = GAIR_WAITFOR;
	}
	return dwRetCode;
}

DWORD CMraProto::MraAvatarsQueueGetAvatarSimple(HANDLE hAvatarsQueueHandle, DWORD dwFlags, MCONTACT hContact)
{
	if ( !hAvatarsQueueHandle)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATIONT pai = { 0 };
	pai.cbSize = sizeof(pai);
	pai.hContact = hContact;
	DWORD dwRetCode = MraAvatarsQueueGetAvatar(hAvatarsQueueHandle, dwFlags, hContact, NULL, (DWORD*)&pai.format, pai.filename);
	if (dwRetCode != GAIR_SUCCESS)
		return dwRetCode;
	
	// write owner avatar file name to DB
	if (hContact == NULL)
		CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
	return GAIR_SUCCESS;
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
			CheckDlgButton(hWndDlg, IDC_ENABLE, db_get_b(NULL, MRA_AVT_SECT_NAME, "Enable", MRA_AVT_DEFAULT_ENABLE) ? BST_CHECKED : BST_UNCHECKED);

			CMStringW szServer;
			if (DB_GetStringW(NULL, MRA_AVT_SECT_NAME, "Server", szServer))
				SetDlgItemText(hWndDlg, IDC_SERVER, szServer.c_str());
			else
				SetDlgItemTextA(hWndDlg, IDC_SERVER, MRA_AVT_DEFAULT_SERVER);

			SetDlgItemInt(hWndDlg, IDC_SERVERPORT, db_get_dw(NULL, MRA_AVT_SECT_NAME, "ServerPort", MRA_AVT_DEFAULT_SERVER_PORT), FALSE);
			CheckDlgButton(hWndDlg, IDC_USE_KEEPALIVE_CONN, db_get_b(NULL, MRA_AVT_SECT_NAME, "UseKeepAliveConn", MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN) ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemInt(hWndDlg, IDC_UPD_CHECK_INTERVAL, db_get_dw(NULL, MRA_AVT_SECT_NAME, "CheckInterval", MRA_AVT_DEFAULT_CHK_INTERVAL), FALSE);
			CheckDlgButton(hWndDlg, IDC_RETURN_ABC_PATH, db_get_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", MRA_AVT_DEFAULT_RET_ABC_PATH) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWndDlg, IDC_DELETE_AVT_ON_CONTACT_DELETE, db_get_b(NULL, MRA_AVT_SECT_NAME, "DeleteAvtOnContactDelete", MRA_DELETE_AVT_ON_CONTACT_DELETE) ? BST_CHECKED : BST_UNCHECKED);

			EnableControlsArray(hWndDlg, (WORD*)&wMraAvatarsControlsList, SIZEOF(wMraAvatarsControlsList), IsDlgButtonChecked(hWndDlg, IDC_ENABLE));
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ENABLE)
			EnableControlsArray(hWndDlg, (WORD*)&wMraAvatarsControlsList, SIZEOF(wMraAvatarsControlsList), IsDlgButtonChecked(hWndDlg, IDC_ENABLE));

		if (LOWORD(wParam) == IDC_BUTTON_DEFAULT) {
			SetDlgItemTextA(hWndDlg, IDC_SERVER, MRA_AVT_DEFAULT_SERVER);
			SetDlgItemInt(hWndDlg, IDC_SERVERPORT, MRA_AVT_DEFAULT_SERVER_PORT, FALSE);
		}

		if ((LOWORD(wParam) == IDC_SERVER || LOWORD(wParam) == IDC_SERVERPORT || LOWORD(wParam) == IDC_UPD_CHECK_INTERVAL) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return FALSE;
		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			db_set_b(NULL, MRA_AVT_SECT_NAME, "Enable", IsDlgButtonChecked(hWndDlg, IDC_ENABLE));
			db_set_b(NULL, MRA_AVT_SECT_NAME, "DeleteAvtOnContactDelete", IsDlgButtonChecked(hWndDlg, IDC_DELETE_AVT_ON_CONTACT_DELETE));
			db_set_b(NULL, MRA_AVT_SECT_NAME, "ReturnAbsolutePath", IsDlgButtonChecked(hWndDlg, IDC_RETURN_ABC_PATH));
			db_set_dw(NULL, MRA_AVT_SECT_NAME, "CheckInterval", GetDlgItemInt(hWndDlg, IDC_UPD_CHECK_INTERVAL, NULL, FALSE));
			db_set_b(NULL, MRA_AVT_SECT_NAME, "UseKeepAliveConn", IsDlgButtonChecked(hWndDlg, IDC_USE_KEEPALIVE_CONN));
			db_set_dw(NULL, MRA_AVT_SECT_NAME, "ServerPort", GetDlgItemInt(hWndDlg, IDC_SERVERPORT, NULL, FALSE));

			TCHAR szServer[MAX_PATH];
			GetDlgItemText(hWndDlg, IDC_SERVER, szServer, SIZEOF(szServer));
			db_set_ts(NULL, MRA_AVT_SECT_NAME, "Server", szServer);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


DWORD CMraProto::MraAvatarsDeleteContactAvatarFile(HANDLE hAvatarsQueueHandle, MCONTACT hContact)
{
	if (hAvatarsQueueHandle == NULL)
		return ERROR_INVALID_HANDLE;

	DWORD dwAvatarFormat = GetContactAvatarFormat(hContact, PA_FORMAT_UNKNOWN);
	if (db_get_b(NULL, MRA_AVT_SECT_NAME, "DeleteAvtOnContactDelete", MRA_DELETE_AVT_ON_CONTACT_DELETE) && dwAvatarFormat != PA_FORMAT_DEFAULT) {
		CMStringW szFileName;
		if (!MraAvatarsGetFileName(hAvatarsQueueHandle, hContact, dwAvatarFormat, szFileName))
			return DeleteFile(szFileName);
	}
	return NO_ERROR;
}
