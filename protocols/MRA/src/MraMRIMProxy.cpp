#include "stdafx.h"
#include "MraMRIMProxy.h"
#include "proto.h"

struct MRA_MRIMPROXY_DATA
{
	LPSTR         szEmail;        // LPS to
	DWORD         dwIDRequest;    // DWORD id_request
	DWORD         dwDataType;     // DWORD data_type
	LPSTR         lpszUserData;   // LPS user_data
	size_t        dwUserDataSize;
	MRA_ADDR_LIST malAddrList;    // LPS lps_ip_port
	MRA_GUID      mguidSessionID; // DWORD session_id[4]
	HANDLE        hConnection;
	HANDLE        hWaitHandle;    // internal
};

HANDLE MraMrimProxyCreate()
{
	MRA_MRIMPROXY_DATA *pmmpd = (MRA_MRIMPROXY_DATA*)mir_calloc(sizeof(MRA_MRIMPROXY_DATA));
	return (HANDLE)pmmpd;
}

DWORD MraMrimProxySetData(HANDLE hMraMrimProxyData, const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszUserData, const CMStringA &szAddresses, MRA_GUID *pmguidSessionID)
{
	if (!hMraMrimProxyData)
		return ERROR_INVALID_HANDLE;

	MRA_MRIMPROXY_DATA *pmmpd = (MRA_MRIMPROXY_DATA*)hMraMrimProxyData;

	if (!szEmail.IsEmpty())
		replaceStr(pmmpd->szEmail, szEmail);

	if (dwIDRequest) pmmpd->dwIDRequest = dwIDRequest;
	if (dwDataType) pmmpd->dwDataType = dwDataType;

	if (!lpszUserData.IsEmpty())
		replaceStr(pmmpd->lpszUserData, lpszUserData);

	if (!szAddresses.IsEmpty())
		MraAddrListGetFromBuff(szAddresses, &pmmpd->malAddrList);
	if (pmguidSessionID)
		memcpy(&pmmpd->mguidSessionID, pmguidSessionID, sizeof(MRA_GUID));

	SetEvent(pmmpd->hWaitHandle);
	return 0;
}

void MraMrimProxyFree(HANDLE hMraMrimProxyData)
{
	if (hMraMrimProxyData) {
		MRA_MRIMPROXY_DATA *pmmpd = (MRA_MRIMPROXY_DATA*)hMraMrimProxyData;

		CloseHandle(pmmpd->hWaitHandle);
		Netlib_CloseHandle(pmmpd->hConnection);
		mir_free(pmmpd->szEmail);
		mir_free(pmmpd->lpszUserData);
		MraAddrListFree(&pmmpd->malAddrList);
		mir_free(hMraMrimProxyData);
	}
}

void MraMrimProxyCloseConnection(HANDLE hMraMrimProxyData)
{
	if (hMraMrimProxyData) {
		MRA_MRIMPROXY_DATA *pmmpd = (MRA_MRIMPROXY_DATA*)hMraMrimProxyData;
		SetEvent(pmmpd->hWaitHandle);
		NETLIB_CLOSEHANDLE(pmmpd->hConnection);
	}
}

DWORD CMraProto::MraMrimProxyConnect(HANDLE hMraMrimProxyData, HANDLE *phConnection)
{
	DWORD dwRetErrorCode;

	if (hMraMrimProxyData && phConnection) {
		BOOL bIsHTTPSProxyUsed, bContinue;
		BYTE lpbBufferRcv[BUFF_SIZE_RCV_MIN_FREE];
		DWORD dwBytesReceived, dwConnectReTryCount, dwCurConnectReTryCount;
		size_t dwRcvBuffSize = BUFF_SIZE_RCV_MIN_FREE, dwRcvBuffSizeUsed;
		NETLIBSELECT nls = {0};
		MRA_MRIMPROXY_DATA *pmmpd = (MRA_MRIMPROXY_DATA*)hMraMrimProxyData;
		NETLIBOPENCONNECTION nloc = {0};

		// адреса есть, значит инициаторы не мы
		if (pmmpd->malAddrList.dwAddrCount) {
			CMStringA szAddresses = MraAddrListGetToBuff(&pmmpd->malAddrList);
			MraProxyAck(PROXY_STATUS_OK, pmmpd->szEmail, pmmpd->dwIDRequest, pmmpd->dwDataType, pmmpd->lpszUserData, szAddresses, pmmpd->mguidSessionID);
		}
		// мы инициаторы
		else {
			pmmpd->hWaitHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (pmmpd->szEmail)
			if (MraProxy(pmmpd->szEmail, pmmpd->dwIDRequest, pmmpd->dwDataType, pmmpd->lpszUserData, "", pmmpd->mguidSessionID))
				WaitForSingleObjectEx(pmmpd->hWaitHandle, INFINITE, FALSE);

			CloseHandle(pmmpd->hWaitHandle);
			pmmpd->hWaitHandle = NULL;
		}

		dwRetErrorCode = ERROR_NO_NETWORK;
		if (pmmpd->malAddrList.dwAddrCount) {
			pmmpd->hConnection = NULL;
			bIsHTTPSProxyUsed = IsHTTPSProxyUsed(m_hNetlibUser);
			dwConnectReTryCount = getDword("ConnectReTryCountMRIMProxy", MRA_DEFAULT_CONN_RETRY_COUNT_MRIMPROXY);
			nloc.cbSize = sizeof(nloc);
			nloc.flags = NLOCF_V2;
			nloc.timeout = ((MRA_TIMEOUT_DIRECT_CONN-1)/(pmmpd->malAddrList.dwAddrCount*dwConnectReTryCount));// -1 сек чтобы был запас
			if (nloc.timeout < MRA_TIMEOUT_CONN_MIN) nloc.timeout = MRA_TIMEOUT_CONN_MIN;
			if (nloc.timeout > MRA_TIMEOUT_CONN_MAX) nloc.timeout = MRA_TIMEOUT_CONN_MAX;

			// Set up the sockaddr structure
			for (size_t i = 0; i < pmmpd->malAddrList.dwAddrCount && dwRetErrorCode != NO_ERROR; i++) {
				// через https прокси только 443 порт
				if ((pmmpd->malAddrList.pMailAddress[i].dwPort == MRA_SERVER_PORT_HTTPS && bIsHTTPSProxyUsed) || bIsHTTPSProxyUsed == FALSE) {
					if (pmmpd->dwDataType == MRIM_PROXY_TYPE_FILES)
						ProtoBroadcastAck(MraHContactFromEmail(pmmpd->szEmail, FALSE, TRUE, NULL), ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)pmmpd->dwIDRequest, 0);

					nloc.szHost = inet_ntoa((*((in_addr*)&pmmpd->malAddrList.pMailAddress[i].dwAddr)));
					nloc.wPort = (WORD)pmmpd->malAddrList.pMailAddress[i].dwPort;

					dwCurConnectReTryCount = dwConnectReTryCount;
					do {
						pmmpd->hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
					}
						while (--dwCurConnectReTryCount && pmmpd->hConnection == NULL);

					if (pmmpd->hConnection) {
						nls.cbSize = sizeof(nls);
						nls.dwTimeout = (MRA_TIMEOUT_DIRECT_CONN*1000*2);
						nls.hReadConns[0] = pmmpd->hConnection;
						bContinue = TRUE;
						dwRcvBuffSizeUsed = 0;

						if (pmmpd->dwDataType == MRIM_PROXY_TYPE_FILES)
							ProtoBroadcastAck(MraHContactFromEmail(pmmpd->szEmail, FALSE, TRUE, NULL), ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)pmmpd->dwIDRequest, 0);
						MraSendPacket(nls.hReadConns[0], 0, MRIM_CS_PROXY_HELLO, &pmmpd->mguidSessionID, sizeof(MRA_GUID));

						while (bContinue) {
							switch (CallService(MS_NETLIB_SELECT, 0, (LPARAM)&nls)) {
							case SOCKET_ERROR:
							case 0:// Time out
								dwRetErrorCode = GetLastError();
								ShowFormattedErrorMessage(L"Disconnected, socket error", dwRetErrorCode);
								bContinue = FALSE;
								break;

							case 1:
								if (dwRcvBuffSizeUsed == BUFF_SIZE_RCV_MIN_FREE) { // bad packet
									bContinue = FALSE;
									_CrtDbgBreak();
								}
								else {
									dwBytesReceived = Netlib_Recv(nls.hReadConns[0], (LPSTR)(lpbBufferRcv+dwRcvBuffSizeUsed), (dwRcvBuffSize-dwRcvBuffSizeUsed), 0);
									if (dwBytesReceived && dwBytesReceived != SOCKET_ERROR) { // connected
										dwRcvBuffSizeUsed += dwBytesReceived;
										if (dwRcvBuffSizeUsed >= sizeof(mrim_packet_header_t)) { // packet header received
											if (((mrim_packet_header_t*)lpbBufferRcv)->magic == CS_MAGIC) { // packet OK
												if ((dwRcvBuffSizeUsed-sizeof(mrim_packet_header_t)) >= ((mrim_packet_header_t*)lpbBufferRcv)->dlen) { // full packet received, may be more than one
													if (((mrim_packet_header_t*)lpbBufferRcv)->msg == MRIM_CS_PROXY_HELLO_ACK) // connect OK!
														dwRetErrorCode = NO_ERROR;
													else // bad/wrong
														_CrtDbgBreak();

													bContinue = FALSE;
												}
												else // not all packet received, continue receiving
													debugLogA("Not all packet received, continue receiving\n");
											}
											else  { // bad packet
												debugLogA("Bad packet\n");
												_ASSERT(false);
												bContinue = FALSE;
											}
										}
										else // packet too small, continue receiving
											debugLogA("Packet to small, continue receiving\n");
									}
									else { // disconnected
										dwRetErrorCode = GetLastError();
										ShowFormattedErrorMessage(L"Disconnected, socket read error", dwRetErrorCode);
										bContinue = FALSE;
									}
								}
								break;
							}// end switch
						}// end while
					}
					else dwRetErrorCode = GetLastError();
				}// filtered
			}// end for

			if (dwRetErrorCode != NO_ERROR) // кажется не туда подключились :)
				NETLIB_CLOSEHANDLE(pmmpd->hConnection);
		}
		*phConnection = pmmpd->hConnection;
	}
	else dwRetErrorCode = ERROR_INVALID_HANDLE;
	return dwRetErrorCode;
}
