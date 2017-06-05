/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2017 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
//	Keep-alive thread for the main connection

void __cdecl CMsnProto::msn_keepAliveThread(void*)
{
	bool keepFlag = true;

	hKeepAliveThreadEvt = CreateEvent(NULL, FALSE, FALSE, NULL);

	msnPingTimeout = 45;

	while (keepFlag) {
		switch (WaitForSingleObject(hKeepAliveThreadEvt, msnPingTimeout * 1000)) {
		case WAIT_TIMEOUT:
			keepFlag = msnNsThread != NULL;
			if (usingGateway)
				msnPingTimeout = 45;
			else {
				msnPingTimeout = 20;
				
				if (msnNsThread) {
					if (lastMsgId)
						keepFlag = msnNsThread->sendPacketPayload("PNG", "CON", "\bLast-Msg-Id: %I64u\r\n\r\n", lastMsgId);
					else if (msnRegistration)
						keepFlag = msnNsThread->sendPacketPayload("PNG", "CON", "\b\r\n");
					else
						keepFlag = msnNsThread->sendPacket("PNG", "CON 0");
				}
			}

			if (hHttpsConnection && (clock() - mHttpsTS) > 60 * CLOCKS_PER_SEC) {
				HNETLIBCONN hConn = hHttpsConnection;
				hHttpsConnection = NULL;
				Netlib_Shutdown(hConn);
			}

			if (mStatusMsgTS && (clock() - mStatusMsgTS) > 60 * CLOCKS_PER_SEC) {
				mStatusMsgTS = 0;
				ForkThread(&CMsnProto::msn_storeProfileThread, NULL);
			}
			if (keepFlag && MyOptions.netId != NETID_SKYPE && MSN_RefreshOAuthTokens(true))
				ForkThread(&CMsnProto::msn_refreshOAuthThread, msnNsThread);
			break;

		case WAIT_OBJECT_0:
			keepFlag = msnPingTimeout > 0;
			break;

		default:
			keepFlag = false;
			break;
		}
	}

	CloseHandle(hKeepAliveThreadEvt); hKeepAliveThreadEvt = NULL;
	debugLogA("Closing keep-alive thread");
}

void __cdecl CMsnProto::msn_loginThread(void*)
{
	MSN_RefreshContactList();
	MSN_FetchRecentMessages();
}

void __cdecl CMsnProto::msn_refreshOAuthThread(void *param)
{
	if (MSN_RefreshOAuthTokens(false) > 0) {
		bIgnoreATH = true;
		MSN_SendATH((ThreadData*)param);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
//	MSN server thread - read and process commands from a server

static bool ReallocInfoBuffer(ThreadData *info, size_t mDataSize)
{
	char *mData = (char*)mir_realloc(info->mData, mDataSize + 1);
	if (mData == NULL)
		return false;

	info->mData = mData;
	info->mDataSize = mDataSize;
	ZeroMemory(&mData[info->mBytesInData], info->mDataSize - info->mBytesInData + 1);
	return true;
}

void __cdecl CMsnProto::MSNServerThread(void* arg)
{
	ThreadData* info = (ThreadData*)arg;
	if (info->mIsMainThread)
		isConnectSuccess = false;

	int tPortNumber = -1;
	{
		char* tPortDelim = strrchr(info->mServer, ':');
		if (tPortDelim != NULL) {
			*tPortDelim = '\0';
			if ((tPortNumber = atoi(tPortDelim + 1)) == 0)
				tPortNumber = -1;
			else if (usingGateway && !(tPortNumber == 80 || tPortNumber == 443))
				usingGateway = false;
		}
	}

	if (usingGateway) {
		if (info->mServer[0] == 0)
			mir_strcpy(info->mServer, MSN_DEFAULT_LOGIN_SERVER);
		else if (info->mIsMainThread)
			mir_strcpy(info->mGatewayIP, info->mServer);

		if (info->gatewayType)
			mir_strcpy(info->mGatewayIP, info->mServer);
		else {
			if (info->mGatewayIP[0] == 0 && db_get_static(NULL, m_szModuleName, "GatewayServer", info->mGatewayIP, sizeof(info->mGatewayIP)))
				mir_strcpy(info->mGatewayIP, MSN_DEFAULT_GATEWAY);
		}
	}
	else {
		if (info->mServer[0] == 0 && db_get_static(NULL, m_szModuleName, "DirectServer", info->mServer, sizeof(info->mServer)))
			mir_strcpy(info->mServer, MSN_DEFAULT_LOGIN_SERVER);
	}

	NETLIBOPENCONNECTION tConn = { 0 };
	tConn.cbSize = sizeof(tConn);
	tConn.flags = NLOCF_V2;
	tConn.timeout = 5;

	if (usingGateway) {
		tConn.flags |= NLOCF_HTTPGATEWAY;
		tConn.szHost = info->mGatewayIP;
		tConn.wPort = MSN_DEFAULT_GATEWAY_PORT;
	}
	else {
		tConn.flags = NLOCF_SSL;
		tConn.szHost = info->mServer;
		tConn.wPort = MSN_DEFAULT_PORT;
	}
	if (tPortNumber != -1)
		tConn.wPort = (WORD)tPortNumber;

	debugLogA("Thread started: server='%s:%d', type=%d", tConn.szHost, tConn.wPort, info->mType);

	info->s = Netlib_OpenConnection(m_hNetlibUser, &tConn);
	if (info->s == NULL) {
		debugLogA("Connection Failed (%d) server='%s:%d'", WSAGetLastError(), tConn.szHost, tConn.wPort);

		switch (info->mType) {
		case SERVER_NOTIFICATION:
			goto LBL_Exit;
			break;
		}
		return;
	}

	if (usingGateway)
		Netlib_SetPollingTimeout(info->s, info->mGatewayTimeout);

	debugLogA("Connected with handle=%08X", info->s);

	if (info->mType == SERVER_NOTIFICATION)
		info->sendPacketPayload("CNT", "CON", "<connect>%s%s%s<ver>2</ver><agent><os>winnt</os><osVer>5.2</osVer><proc>x86</proc><lcid>en-us</lcid></agent></connect>\r\n",
			*info->mState ? "<xfr><state>" : "", *info->mState ? info->mState : "", *info->mState ? "</state></xfr>" : "");

	if (info->mIsMainThread)
		msnNsThread = info;

	debugLogA("Entering main recv loop");
	info->mBytesInData = 0;
	for (;;) {
		int recvResult = info->recv(info->mData + info->mBytesInData, info->mDataSize - info->mBytesInData);
		if (recvResult == SOCKET_ERROR) {
			debugLogA("Connection %08p [%08X] was abortively closed", info->s, GetCurrentThreadId());
			break;
		}

		if (!recvResult) {
			debugLogA("Connection %08p [%08X] was gracefully closed", info->s, GetCurrentThreadId());
			break;
		}

		info->mBytesInData += recvResult;

		for (;;) {
			char* peol = strchr(info->mData, '\r');
			if (peol == NULL)
				break;

			int msgLen = (int)(peol - info->mData);
			if (info->mBytesInData < msgLen + 2)
				break;  //wait for full line end

			char msg[1024];
			strncpy_s(msg, info->mData, msgLen);

			if (*++peol != '\n')
				debugLogA("Dodgy line ending to command: ignoring");
			else
				peol++;

			info->mBytesInData -= peol - info->mData;
			memmove(info->mData, peol, info->mBytesInData);
			debugLogA("RECV: %s", msg);

			if (!isalnum(msg[0]) || !isalnum(msg[1]) || !isalnum(msg[2]) || (msg[3] && msg[3] != ' ')) {
				debugLogA("Invalid command name");
				continue;
			}

			int handlerResult;
			if (isdigit(msg[0]) && isdigit(msg[1]) && isdigit(msg[2]))   //all error messages
				handlerResult = MSN_HandleErrors(info, msg);
			else
				handlerResult = MSN_HandleCommands(info, msg);

			if (handlerResult) {
				if (info->sessionClosed) goto LBL_Exit;
				info->sendTerminate();
			}
		}

		if (info->mBytesInData == info->mDataSize) {
			if (!ReallocInfoBuffer(info, info->mDataSize * 2)) {
				debugLogA("sizeof(data) is too small: the longest line won't fit");
				break;
			}
		}
	}

LBL_Exit:
	if (info->mIsMainThread) {
		/*
		if (!isConnectSuccess && !usingGateway && m_iDesiredStatus != ID_STATUS_OFFLINE) {
			msnNsThread = NULL;
			usingGateway = true;

			ThreadData* newThread = new ThreadData;
			newThread->mType = SERVER_NOTIFICATION;
			newThread->mIsMainThread = true;

			newThread->startThread(&CMsnProto::MSNServerThread, this);
		}
		else*/ {
			if (hKeepAliveThreadEvt) {
				msnPingTimeout *= -1;
				SetEvent(hKeepAliveThreadEvt);
			}

			if (info->s == NULL)
				ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
			else
				MSN_CloseConnections();

			if (hHttpsConnection) {
				Netlib_CloseHandle(hHttpsConnection);
				hHttpsConnection = NULL;
			}

			MSN_GoOffline();
			msnNsThread = NULL;
		}
	}

	debugLogA("Thread [%08X] ending now", GetCurrentThreadId());
}

void CMsnProto::MSN_CloseConnections(void)
{
	mir_cslockfull lck(m_csThreads);

	NETLIBSELECTEX nls = {};

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];

		switch (T.mType) {
		case SERVER_NOTIFICATION:
			if (T.s != NULL && !T.sessionClosed && !T.termPending) {
				nls.hReadConns[0] = T.s;
				int res = Netlib_SelectEx(&nls);
				if (res >= 0 || nls.hReadStatus[0] == 0)
					T.sendTerminate();
			}
			break;
		}
	}

	lck.unlock();

	if (hHttpsConnection)
		Netlib_Shutdown(hHttpsConnection);
}

void CMsnProto::Threads_Uninit(void)
{
	mir_cslock lck(m_csThreads);
	m_arThreads.destroy();
}

GCThreadData* CMsnProto::MSN_GetThreadByChatId(const wchar_t* chatId)
{
	if (mir_wstrlen(chatId) == 0)
		return NULL;

	mir_cslock lck(m_csThreads);
	for (int i = 0; i < m_arGCThreads.getCount(); i++) {
		GCThreadData *T = m_arGCThreads[i];
		if (mir_wstrcmpi(T->mChatID, chatId) == 0)
			return T;
	}

	return NULL;
}

ThreadData* CMsnProto::MSN_GetThreadByConnection(HANDLE s)
{
	mir_cslock lck(m_csThreads);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.s == s)
			return &T;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// class ThreadData members

ThreadData::ThreadData()
{
	memset(&mInitialContactWLID, 0, sizeof(ThreadData) - 2 * sizeof(STRLIST));
	mGatewayTimeout = 2;
	resetTimeout();
	hWaitEvent = CreateSemaphore(NULL, 0, MSN_PACKETS_COMBINE, NULL);
	mData = (char*)mir_calloc((mDataSize = 8192) + 1);
}

ThreadData::~ThreadData()
{
	if (s != NULL) {
		proto->debugLogA("Closing connection handle %08X", s);
		Netlib_CloseHandle(s);
	}

	if (mIncomingBoundPort != NULL) {
		Netlib_CloseHandle(mIncomingBoundPort);
	}

	if (mMsnFtp != NULL) {
		delete mMsnFtp;
		mMsnFtp = NULL;
	}

	if (hWaitEvent != INVALID_HANDLE_VALUE)
		CloseHandle(hWaitEvent);

	if (mTimerId != 0)
		KillTimer(NULL, mTimerId);

	mJoinedContactsWLID.destroy();
	mJoinedIdentContactsWLID.destroy();

	mir_free(mInitialContactWLID); mInitialContactWLID = NULL;
	mir_free(mData);
}

void ThreadData::applyGatewayData(HNETLIBCONN hConn, bool isPoll)
{
	char szHttpPostUrl[300];
	getGatewayUrl(szHttpPostUrl, sizeof(szHttpPostUrl), isPoll);

	proto->debugLogA("applying '%s' to %08X [%08X]", szHttpPostUrl, this, GetCurrentThreadId());

	NETLIBHTTPPROXYINFO nlhpi = {};
	nlhpi.flags = NLHPIF_HTTP11;
	nlhpi.szHttpGetUrl = NULL;
	nlhpi.szHttpPostUrl = szHttpPostUrl;
	nlhpi.combinePackets = 5;
	Netlib_SetHttpProxyInfo(hConn, &nlhpi);
}

void ThreadData::getGatewayUrl(char* dest, int destlen, bool isPoll)
{
	static const char openFmtStr[] = "http://%s/gateway/gateway.dll?Action=open&Server=%s&IP=%s";
	static const char pollFmtStr[] = "http://%s/gateway/gateway.dll?Action=poll&SessionID=%s";
	static const char cmdFmtStr[] = "http://%s/gateway/gateway.dll?SessionID=%s";

	if (mSessionID[0] == 0) {
		const char* svr = mType == SERVER_NOTIFICATION ? "NS" : "SB";
		mir_snprintf(dest, destlen, openFmtStr, mGatewayIP, svr, mServer);
	}
	else mir_snprintf(dest, destlen, isPoll ? pollFmtStr : cmdFmtStr, mGatewayIP, mSessionID);
}

void ThreadData::processSessionData(const char* xMsgr, const char* xHost)
{
	char tSessionID[40], tGateIP[80];

	char* tDelim = (char*)strchr(xMsgr, ';');
	if (tDelim == NULL)
		return;

	*tDelim = 0; tDelim += 2;

	if (!sscanf(xMsgr, "SessionID=%s", tSessionID))
		return;

	char* tDelim2 = strchr(tDelim, ';');
	if (tDelim2 != NULL)
		*tDelim2 = '\0';
	if (xHost)
		mir_strcpy(tGateIP, xHost);
	else if (!sscanf(tDelim, "GW-IP=%s", tGateIP))
		return;

	mir_strcpy(mGatewayIP, tGateIP);
	if (gatewayType) mir_strcpy(mServer, tGateIP);
	mir_strcpy(mSessionID, tSessionID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// thread start code
/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CMsnProto::ThreadStub(void* arg)
{
	ThreadData* info = (ThreadData*)arg;

	debugLogA("Starting thread %08X (%08X)", GetCurrentThreadId(), info->mFunc);

	(this->*(info->mFunc))(info);

	debugLogA("Leaving thread %08X (%08X)", GetCurrentThreadId(), info->mFunc);
	{
		mir_cslock lck(m_csThreads);
		m_arThreads.LIST<ThreadData>::remove(info);
	}
	delete info;
}

void ThreadData::startThread(MsnThreadFunc parFunc, CMsnProto *prt)
{
	mFunc = parFunc;
	proto = prt;
	{
		mir_cslock lck(prt->m_csThreads);
		proto->m_arThreads.insert(this);
	}
	proto->ForkThread(&CMsnProto::ThreadStub, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
// HReadBuffer members

HReadBuffer::HReadBuffer(ThreadData *T, int iStart)
{
	owner = T;
	buffer = (BYTE*)T->mData;
	totalDataSize = T->mBytesInData;
	startOffset = iStart;
}

HReadBuffer::~HReadBuffer()
{
	if (totalDataSize > startOffset) {
		memmove(buffer, buffer + startOffset, (totalDataSize -= startOffset));
		owner->mBytesInData = (int)totalDataSize;
	}
	else owner->mBytesInData = 0;

	buffer[owner->mBytesInData] = 0;
}

BYTE* HReadBuffer::surelyRead(size_t parBytes)
{
	if ((startOffset + parBytes) > owner->mDataSize) {
		if (totalDataSize > startOffset)
			memmove(buffer, buffer + startOffset, (totalDataSize -= startOffset));
		else
			totalDataSize = 0;

		startOffset = 0;

		if (parBytes > owner->mDataSize) {
			size_t mDataSize = owner->mDataSize;
			while (parBytes > mDataSize) mDataSize *= 2;
			if (!ReallocInfoBuffer(owner, mDataSize)) {
				owner->proto->debugLogA("HReadBuffer::surelyRead: not enough memory, %d %d %d", parBytes, owner->mDataSize, startOffset);
				return NULL;
			}
			buffer = (BYTE*)owner->mData;
		}
	}

	while ((startOffset + parBytes) > totalDataSize) {
		int recvResult = owner->recv((char*)buffer + totalDataSize, owner->mDataSize - totalDataSize);

		if (recvResult <= 0)
			return NULL;

		totalDataSize += recvResult;
	}

	BYTE *result = buffer + startOffset; startOffset += parBytes;
	buffer[totalDataSize] = 0;
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// class GCThreadData members

GCThreadData::GCThreadData() :
	mJoinedContacts(10, PtrKeySortT)
{
	memset(&mCreator, 0, sizeof(GCThreadData) - sizeof(mJoinedContacts));
}

GCThreadData::~GCThreadData()
{
}
