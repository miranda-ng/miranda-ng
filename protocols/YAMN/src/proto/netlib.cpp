/*
 * This code implements communication based on Miranda netlib library
 *
 * (c) majvan 2002-2004
 */

#include "../stdafx.h"

 //--------------------------------------------------------------------------------------------------
 //--------------------------------------------------------------------------------------------------

BOOL SSLLoaded = FALSE;
HNETLIBUSER hNetlibUser = nullptr;

void __stdcall	SSL_DebugLog(const char *fmt, ...)
{
	char str[4096];
	va_list	vararg;

	va_start(vararg, fmt);
	int tBytes = mir_vsnprintf(str, _countof(str), fmt, vararg);
	if (tBytes == 0)
		return;

	if (tBytes > 0)
		str[tBytes] = 0;
	else
		str[sizeof(str) - 1] = 0;

	Netlib_Log(hNetlibUser, str);
	va_end(vararg);
}

HANDLE RegisterNLClient(const char *name)
{
	char desc[128];

	mir_snprintf(desc, Translate("%s connection"), name);

	#ifdef DEBUG_COMM
	DebugLog(CommFile, "<Register PROXY support>");
	#endif

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szDescriptiveName.a = desc;
	nlu.szSettingsModule = (char *)name;
	hNetlibUser = Netlib_RegisterUser(&nlu);

	#ifdef DEBUG_COMM
	if (NULL == hNetlibUser)
		DebugLog(CommFile, "<error></Register PROXY support>\n");
	else
		DebugLog(CommFile, "</Register PROXY support>\n");
	#endif
	return hNetlibUser;
}

//Move connection to SSL
void CNLClient::SSLify() throw(DWORD)
{
	#ifdef DEBUG_COMM
	SSL_DebugLog("Staring SSL...");
	#endif
	int socket = Netlib_GetSocket(hConnection);
	if (socket != INVALID_SOCKET) {
		#ifdef DEBUG_COMM
		SSL_DebugLog("Staring netlib core SSL");
		#endif
		if (Netlib_StartSsl(hConnection, nullptr)) {
			#ifdef DEBUG_COMM
			SSL_DebugLog("Netlib core SSL started");
			#endif
			isTLSed = true;
			SSLLoaded = TRUE;
			return;
		}
	}

	//ssl could not be created
	throw NetworkError = (uint32_t)ESSL_CREATESSL;
}

//Connects to the server through the sock
//if not success, exception is throwed
void CNLClient::Connect(const char* servername, const int port) throw(DWORD)
{
	NetworkError = SystemError = 0;
	isTLSed = false;

	#ifdef DEBUG_COMM
	DebugLog(CommFile, "<connect>\n");
	#endif
	try {
		NETLIBOPENCONNECTION nloc = {};
		nloc.szHost = servername;
		nloc.wPort = port;
		if (nullptr == (hConnection = Netlib_OpenConnection(hNetlibUser, &nloc))) {
			SystemError = WSAGetLastError();
			throw NetworkError = (uint32_t)ENL_CONNECT;
		}
		#ifdef DEBUG_COMM
		DebugLog(CommFile, "</connect>\n");
		#endif
		return;
	}
	catch (...) {
		#ifdef DEBUG_COMM
		DebugLog(CommFile, "<error></connect>\n");
		#endif
		throw;
	}
}

//Performs a simple query
// query- command to send
int CNLClient::LocalNetlib_Send(HNETLIBCONN hConn, const char *buf, int len, int flags)
{
	return Netlib_Send(hConn, buf, len, flags);
}

void CNLClient::Send(const char *query) throw(DWORD)
{
	unsigned int Sent;

	if (nullptr == query)
		return;
	if (hConnection == nullptr)
		return;
	#ifdef DEBUG_COMM
	DebugLog(CommFile, "<send>%s", query);
	#endif
	try {
		if ((SOCKET_ERROR == (Sent = LocalNetlib_Send(hConnection, query, (int)mir_strlen(query), MSG_DUMPASTEXT))) || Sent != (unsigned int)mir_strlen(query)) {
			SystemError = WSAGetLastError();
			throw NetworkError = (uint32_t)ENL_SEND;
		}
		#ifdef DEBUG_COMM
		DebugLog(CommFile, "</send>\n");
		#endif
	}
	catch (...) {
		#ifdef DEBUG_COMM
		DebugLog(CommFile, "<error></send>\n");
		#endif
		throw;
	}
}

//Reads data from socket
// buf- buffer where to store max. buflen of received characters
//      if buf is NULL, creates buffer of buflen size
//      buf is NULL by default
//You need free() returned buffer, which can be allocated in this function
//if not success, exception is throwed

int CNLClient::LocalNetlib_Recv(HNETLIBCONN hConn, char *buf, int len, int flags)
{
	int iReturn = Netlib_Recv(hConn, buf, len, flags);
	if (isTLSed) {
		#ifdef DEBUG_COMM
		SSL_DebugLog("SSL recv: %s", buf);
		#endif
	}

	return iReturn;
}

char* CNLClient::Recv(char *buf, int buflen) throw(DWORD)
{
	#ifdef DEBUG_COMM
	DebugLog(CommFile, "<reading>");
	#endif
	try {
		if (buf == nullptr)
			buf = (char *)malloc(sizeof(char)*(buflen + 1));
		if (buf == nullptr)
			throw NetworkError = (uint32_t)ENL_RECVALLOC;

		if (!isTLSed) {
			NETLIBSELECT nls = {};
			nls.dwTimeout = 60000;
			nls.hReadConns[0] = hConnection;
			switch (Netlib_Select(&nls)) {
			case SOCKET_ERROR:
				free(buf);
				SystemError = WSAGetLastError();
				throw NetworkError = (uint32_t)ENL_RECV;
			case 0: // time out!
				free(buf);
				throw NetworkError = (uint32_t)ENL_TIMEOUT;
			}
		}

		memset(buf, 0, buflen);
		if (SOCKET_ERROR == (Rcv = LocalNetlib_Recv(hConnection, buf, buflen, MSG_DUMPASTEXT))) {
			free(buf);
			SystemError = WSAGetLastError();
			throw NetworkError = (uint32_t)ENL_RECV;
		}
		if (!Rcv) {
			free(buf);
			SystemError = WSAGetLastError();
			throw NetworkError = (uint32_t)ENL_RECV;
		}
		#ifdef DEBUG_COMM
		*(buf + Rcv) = 0;				//end the buffer to write it to file
		DebugLog(CommFile, "%s", buf);
		DebugLog(CommFile, "</reading>\n");
		#endif
		return(buf);
	}
	catch (...) {
		#ifdef DEBUG_COMM
		DebugLog(CommFile, "<error></reading>\n");
		#endif
		throw;
	}
}

//Closes netlib connection
void CNLClient::Disconnect()
{
	Netlib_CloseHandle(hConnection);
	hConnection = nullptr;
}

//Uninitializes netlib library
void UnregisterNLClient()
{
	#ifdef DEBUG_COMM
	DebugLog(CommFile, "<Unregister PROXY support>");
	#endif

	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = nullptr;
	#ifdef DEBUG_COMM
	DebugLog(CommFile, "</Unregister PROXY support>\n");
	#endif
}
