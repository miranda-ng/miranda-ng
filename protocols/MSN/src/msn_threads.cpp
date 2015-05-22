/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
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

#include "msn_global.h"
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
				keepFlag = keepFlag && msnNsThread->sendPacket("PNG", "CON 0");
			}
#ifdef OBSOLETE
			p2p_clearDormantSessions();
#endif
			if (hHttpsConnection && (clock() - mHttpsTS) > 60 * CLOCKS_PER_SEC) {
				HANDLE hConn = hHttpsConnection;
				hHttpsConnection = NULL;
				Netlib_CloseHandle(hConn);
			}
			if (mStatusMsgTS && (clock() - mStatusMsgTS) > 60 * CLOCKS_PER_SEC) {
				mStatusMsgTS = 0;
				ForkThread(&CMsnProto::msn_storeProfileThread, NULL);
			}
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

/////////////////////////////////////////////////////////////////////////////////////////
//	MSN server thread - read and process commands from a server

static bool ReallocInfoBuffer(ThreadData *info, size_t mDataSize)
{
	char *mData = (char*)mir_realloc(info->mData, mDataSize+1);
	if (mData == NULL)
		return false;

	info->mData = mData;
	info->mDataSize = mDataSize;
	ZeroMemory(&mData[info->mBytesInData], info->mDataSize-info->mBytesInData+1);
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
			strcpy(info->mServer, MSN_DEFAULT_LOGIN_SERVER);
		else if (info->mIsMainThread)
			strcpy(info->mGatewayIP, info->mServer);

		if (info->gatewayType)
			strcpy(info->mGatewayIP, info->mServer);
		else {
			if (info->mGatewayIP[0] == 0 && db_get_static(NULL, m_szModuleName, "GatewayServer", info->mGatewayIP, sizeof(info->mGatewayIP)))
				strcpy(info->mGatewayIP, MSN_DEFAULT_GATEWAY);
		}
	}
	else {
		if (info->mServer[0] == 0 && db_get_static(NULL, m_szModuleName, "DirectServer", info->mServer, sizeof(info->mServer)))
			strcpy(info->mServer, MSN_DEFAULT_LOGIN_SERVER);
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

	info->s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&tConn);
	if (info->s == NULL) {
		debugLogA("Connection Failed (%d) server='%s:%d'", WSAGetLastError(), tConn.szHost, tConn.wPort);

		switch (info->mType) {
		case SERVER_NOTIFICATION:
			goto LBL_Exit;
			break;

		case SERVER_SWITCHBOARD:
			if (info->mCaller) msnNsThread->sendPacket("XFR", "SB");
			break;
		}
		return;
	}

	if (usingGateway)
		CallService(MS_NETLIB_SETPOLLINGTIMEOUT, WPARAM(info->s), info->mGatewayTimeout);

	debugLogA("Connected with handle=%08X", info->s);

	if (info->mType == SERVER_NOTIFICATION) 
		info->sendPacketPayload("CNT", "CON", "<connect>%s%s%s<ver>2</ver><agent><os>winnt</os><osVer>5.2</osVer><proc>x86</proc><lcid>en-us</lcid></agent></connect>\r\n",
		*info->mState?"<xfr><state>":"", *info->mState?info->mState:"", *info->mState?"</state></xfr>":"");
	else if (info->mType == SERVER_SWITCHBOARD) {
		info->sendPacket(info->mCaller ? "USR" : "ANS", "%s;%s %s", MyOptions.szEmail, MyOptions.szMachineGuid, info->mCookie);
	}
	else if (info->mType == SERVER_FILETRANS && info->mCaller == 0) {
		info->send("VER MSNFTP\r\n", 12);
	}

	if (info->mIsMainThread) {
		msnNsThread = info;
	}

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

#ifdef OBSOLETE
		if (info->mCaller == 1 && info->mType == SERVER_FILETRANS) {
			if (MSN_HandleMSNFTP(info, info->mData))
				break;
		}
		else 
#endif
		{
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

				if (info->mType != SERVER_FILETRANS) {
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
#ifdef OBSOLETE
				else
					if (MSN_HandleMSNFTP(info, msg))
						goto LBL_Exit;
#endif
			}
		}

		if (info->mBytesInData == info->mDataSize) {
			if (!ReallocInfoBuffer(info, info->mDataSize*2)) {
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
			else {
#ifdef OBSOLETE
				p2p_cancelAllSessions();
#endif
				MSN_CloseConnections();
			}

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

	NETLIBSELECTEX nls = { 0 };
	nls.cbSize = sizeof(nls);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];

		switch (T.mType) {
		case SERVER_NOTIFICATION:
		case SERVER_SWITCHBOARD:
			if (T.s != NULL && !T.sessionClosed && !T.termPending) {
				nls.hReadConns[0] = T.s;
				int res = CallService(MS_NETLIB_SELECTEX, 0, (LPARAM)&nls);
				if (res >= 0 || nls.hReadStatus[0] == 0)
					T.sendTerminate();
			}
			break;

		case SERVER_P2P_DIRECT:
			CallService(MS_NETLIB_SHUTDOWN, (WPARAM)T.s, 0);
			break;
		}
	}

	lck.unlock();

	if (hHttpsConnection)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)hHttpsConnection, 0);
}

void CMsnProto::Threads_Uninit(void)
{
	mir_cslock lck(m_csThreads);
	m_arThreads.destroy();
}

ThreadData* CMsnProto::MSN_GetThreadByContact(const char* wlid, TInfoType type)
{
	mir_cslock lck(m_csThreads);

	if (type == SERVER_P2P_DIRECT) {
		for (int i = 0; i < m_arThreads.getCount(); i++) {
			ThreadData &T = m_arThreads[i];
			if (T.mType != SERVER_P2P_DIRECT || !T.mJoinedIdentContactsWLID.getCount() || T.s == NULL)
				continue;

			if (_stricmp(T.mJoinedIdentContactsWLID[0], wlid) == 0)
				return &T;
		}
	}

	char *szEmail = NULL;
	parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mType != type || !T.mJoinedContactsWLID.getCount() || T.mInitialContactWLID || T.s == NULL)
			continue;

		if (_stricmp(T.mJoinedContactsWLID[0], szEmail) == 0 && T.mChatID[0] == 0)
			return &T;
	}

	return NULL;
}

GCThreadData* CMsnProto::MSN_GetThreadByChatId(const TCHAR* chatId)
{
	if (mir_tstrlen(chatId) == 0)
		return NULL;

	mir_cslock lck(m_csThreads);
	for (int i = 0; i < m_arGCThreads.getCount(); i++) {
		GCThreadData *T = m_arGCThreads[i];
		if (mir_tstrcmpi(T->mChatID, chatId) == 0)
			return T;
	}

	return NULL;
}

#ifdef OBSOLETE
ThreadData* CMsnProto::MSN_GetP2PThreadByContact(const char *wlid)
{
	mir_cslock lck(m_csThreads);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mType != SERVER_P2P_DIRECT || !T.mJoinedIdentContactsWLID.getCount())
			continue;

		if (_stricmp(T.mJoinedIdentContactsWLID[0], wlid) == 0)
			return &T;
	}

	char *szEmail = NULL;
	parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);

	ThreadData *result = NULL;
	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mJoinedContactsWLID.getCount() && !T.mInitialContactWLID &&
			_stricmp(T.mJoinedContactsWLID[0], szEmail) == 0) {
			if (T.mType == SERVER_P2P_DIRECT)
				return &T;

			if (T.mType == SERVER_SWITCHBOARD)
				result = &T;
		}
	}

	return result;
}


void CMsnProto::MSN_StartP2PTransferByContact(const char* wlid)
{
	mir_cslock lck(m_csThreads);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mType == SERVER_FILETRANS && T.hWaitEvent != INVALID_HANDLE_VALUE) {
			if ((T.mInitialContactWLID && !_stricmp(T.mInitialContactWLID, wlid)) ||
				(T.mJoinedContactsWLID.getCount() && !_stricmp(T.mJoinedContactsWLID[0], wlid)) ||
				(T.mJoinedIdentContactsWLID.getCount() && !_stricmp(T.mJoinedIdentContactsWLID[0], wlid)))
				ReleaseSemaphore(T.hWaitEvent, 1, NULL);
		}
	}
}

ThreadData* CMsnProto::MSN_GetOtherContactThread(ThreadData* thread)
{
	mir_cslock lck(m_csThreads);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mJoinedContactsWLID.getCount() == 0 || T.s == NULL)
			continue;

		if (&T != thread && _stricmp(T.mJoinedContactsWLID[0], thread->mJoinedContactsWLID[0]) == 0)
			return &T;
	}

	return NULL;
}

ThreadData* CMsnProto::MSN_GetUnconnectedThread(const char* wlid, TInfoType type)
{
	mir_cslock lck(m_csThreads);

	char* szEmail = (char*)wlid;

	if (type == SERVER_SWITCHBOARD && strchr(wlid, ';'))
		parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mType == type && T.mInitialContactWLID && _stricmp(T.mInitialContactWLID, szEmail) == 0)
			return &T;
	}

	return NULL;
}


ThreadData* CMsnProto::MSN_StartSB(const char* wlid, bool& isOffline)
{
	isOffline = false;
	ThreadData *thread = MSN_GetThreadByContact(wlid);
	if (thread == NULL) {
		MCONTACT hContact = MSN_HContactFromEmail(wlid);
		WORD wStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
		if (wStatus != ID_STATUS_OFFLINE) {
			if (MSN_GetUnconnectedThread(wlid) == NULL && MsgQueue_CheckContact(wlid, 5) == NULL)
				msnNsThread->sendPacket("XFR", "SB");
		}
		else isOffline = true;
	}
	return thread;
}


int CMsnProto::MSN_GetActiveThreads(ThreadData** parResult)
{
	int tCount = 0;
	mir_cslock lck(m_csThreads);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mType == SERVER_SWITCHBOARD && T.mJoinedContactsWLID.getCount() != 0 && T.mJoinedContactsWLID.getCount())
			parResult[tCount++] = &T;
	}

	return tCount;
}
#endif

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

#ifdef OBSOLETE
ThreadData* CMsnProto::MSN_GetThreadByPort(WORD wPort)
{
	mir_cslock lck(m_csThreads);

	for (int i = 0; i < m_arThreads.getCount(); i++) {
		ThreadData &T = m_arThreads[i];
		if (T.mIncomingPort == wPort)
			return &T;
	}

	return NULL;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// class ThreadData members

ThreadData::ThreadData()
{
	memset(&mInitialContactWLID, 0, sizeof(ThreadData) - 2 * sizeof(STRLIST));
	mGatewayTimeout = 2;
	resetTimeout();
	hWaitEvent = CreateSemaphore(NULL, 0, MSN_PACKETS_COMBINE, NULL);
	mData = (char*)mir_calloc((mDataSize=8192)+1);
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

#ifdef OBSOLETE
	if (mType == SERVER_SWITCHBOARD) {
		for (int i = 0; i < mJoinedContactsWLID.getCount(); ++i) {
			const char* wlid = mJoinedContactsWLID[i];
			MCONTACT hContact = proto->MSN_HContactFromEmail(wlid);
			int temp_status = proto->getWord(hContact, "Status", ID_STATUS_OFFLINE);
			if (temp_status == ID_STATUS_INVISIBLE && proto->MSN_GetThreadByContact(wlid) == NULL)
				proto->setWord(hContact, "Status", ID_STATUS_OFFLINE);
		}
	}
#endif

	mJoinedContactsWLID.destroy();
	mJoinedIdentContactsWLID.destroy();

	mir_free(mInitialContactWLID); mInitialContactWLID = NULL;

#ifdef OBSOLETE
	const char* wlid = NEWSTR_ALLOCA(mInitialContactWLID);

	if (proto && mType == SERVER_P2P_DIRECT)
		proto->p2p_clearDormantSessions();

	if (wlid != NULL && mType == SERVER_SWITCHBOARD &&
		proto->MSN_GetThreadByContact(wlid) == NULL &&
		proto->MSN_GetUnconnectedThread(wlid) == NULL) {
		proto->MsgQueue_Clear(wlid, true);
	}
#endif

	mir_free(mData);
}

void ThreadData::applyGatewayData(HANDLE hConn, bool isPoll)
{
	char szHttpPostUrl[300];
	getGatewayUrl(szHttpPostUrl, sizeof(szHttpPostUrl), isPoll);

	proto->debugLogA("applying '%s' to %08X [%08X]", szHttpPostUrl, this, GetCurrentThreadId());

	NETLIBHTTPPROXYINFO nlhpi = { 0 };
	nlhpi.cbSize = sizeof(nlhpi);
	nlhpi.flags = NLHPIF_HTTP11;
	nlhpi.szHttpGetUrl = NULL;
	nlhpi.szHttpPostUrl = szHttpPostUrl;
	nlhpi.combinePackets = 5;
	CallService(MS_NETLIB_SETHTTPPROXYINFO, (WPARAM)hConn, (LPARAM)&nlhpi);
}

void ThreadData::getGatewayUrl(char* dest, int destlen, bool isPoll)
{
	static const char openFmtStr[] = "https://%s/gateway/gateway.dll?Action=open&Server=%s&IP=%s";
	static const char pollFmtStr[] = "https://%s/gateway/gateway.dll?Action=poll&SessionID=%s";
	static const char cmdFmtStr[] = "https://%s/gateway/gateway.dll?SessionID=%s";

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
		strcpy(tGateIP, xHost);
	else if (!sscanf(tDelim, "GW-IP=%s", tGateIP))
		return;

	strcpy(mGatewayIP, tGateIP);
	if (gatewayType) strcpy(mServer, tGateIP);
	strcpy(mSessionID, tSessionID);
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
			while (parBytes > mDataSize) mDataSize*=2;
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

