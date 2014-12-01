/*
 * This code implements communication based on Miranda netlib library
 *
 * (c) majvan 2002-2004
 */

#include "..\yamn.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

BOOL SSLLoaded=FALSE;
HANDLE hNetlibUser=NULL;

void __stdcall	SSL_DebugLog(const char *fmt, ...)
{
	char str[4096];
	va_list	vararg;

	va_start( vararg, fmt );
	int tBytes = mir_vsnprintf(str, SIZEOF(str), fmt, vararg);
	if ( tBytes == 0 )
		return;

	if ( tBytes > 0 )
		str[ tBytes ] = 0;
	else
		str[ sizeof(str)-1 ] = 0;

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)str);
	va_end( vararg );
}

HANDLE RegisterNLClient(const char *name)
{
	static NETLIBUSER nlu={0};
	char desc[128];

	mir_snprintf(desc, SIZEOF(desc), Translate("%s connection"), name);

#ifdef DEBUG_COMM
	DebugLog(CommFile,"<Register PROXY support>");
#endif
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING  | NUF_HTTPCONNS;
	nlu.szDescriptiveName=desc;
	nlu.szSettingsModule=(char *)name;
	hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);

#ifdef DEBUG_COMM
	if (NULL==hNetlibUser)
		DebugLog(CommFile,"<error></Register PROXY support>\n");
	else
		DebugLog(CommFile,"</Register PROXY support>\n");
#endif
	return hNetlibUser;
}

//Move connection to SSL
void CNLClient::SSLify() throw(DWORD) {
#ifdef DEBUG_COMM
	SSL_DebugLog("Staring SSL...");
#endif
	int socket = CallService(MS_NETLIB_GETSOCKET, (WPARAM)hConnection, 0);
	if (socket != INVALID_SOCKET)
	{
#ifdef DEBUG_COMM
			SSL_DebugLog("Staring netlib core SSL");
#endif
		if (CallService(MS_NETLIB_STARTSSL, (WPARAM)hConnection, 0)) 
		{
#ifdef DEBUG_COMM
			SSL_DebugLog("Netlib core SSL started");
#endif
			isTLSed = true;
			SSLLoaded = TRUE;
			return;
		}
	}

	//ssl could not be created
	throw NetworkError = (DWORD)ESSL_CREATESSL;
}

//Connects to the server through the sock
//if not success, exception is throwed
void CNLClient::Connect(const char* servername,const int port) throw(DWORD)
{
	NETLIBOPENCONNECTION nloc;

	NetworkError=SystemError=0;
	isTLSed = false;

#ifdef DEBUG_COMM
	DebugLog(CommFile,"<connect>\n");
#endif
	try
	{
		nloc.cbSize=sizeof(NETLIBOPENCONNECTION);
		nloc.szHost=servername;
		nloc.wPort=port;
		nloc.flags=0;
		if (NULL==(hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION,(WPARAM)hNetlibUser,(LPARAM)&nloc)))
		{
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_CONNECT;
		}
#ifdef DEBUG_COMM
		DebugLog(CommFile,"</connect>\n");
#endif
		return;
	}
	catch(...)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<error></connect>\n");
#endif
		throw;
	}
}

//Performs a simple query
// query- command to send
int CNLClient::LocalNetlib_Send(HANDLE hConn,const char *buf,int len,int flags) {
	if (isTLSed) 
	{
#ifdef DEBUG_COMM
		SSL_DebugLog("SSL send: %s", buf);
#endif
	} 
	
	NETLIBBUFFER nlb={(char*)buf,len,flags};
	return CallService(MS_NETLIB_SEND,(WPARAM)hConn,(LPARAM)&nlb);
}

void CNLClient::Send(const char *query) throw(DWORD)
{
	unsigned int Sent;

	if (NULL==query)
		return;
	if (hConnection==NULL)
		return;
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<send>%s",query);
#endif
	try
	{
		if ((SOCKET_ERROR==(Sent=LocalNetlib_Send(hConnection,query,(int)strlen(query),MSG_DUMPASTEXT))) || Sent != (unsigned int)strlen(query))
		{
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_SEND;
		}
#ifdef DEBUG_COMM
		DebugLog(CommFile,"</send>\n");
#endif
	}
	catch(...)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<error></send>\n");
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

int CNLClient::LocalNetlib_Recv(HANDLE hConn,char *buf,int len,int flags) {
	NETLIBBUFFER nlb={buf,len,flags};
	int iReturn = CallService(MS_NETLIB_RECV,(WPARAM)hConn,(LPARAM)&nlb);
	if (isTLSed)
	{
#ifdef DEBUG_COMM
		SSL_DebugLog("SSL recv: %s", buf);
#endif
	}
	
	return iReturn;
}

char* CNLClient::Recv(char *buf,int buflen) throw(DWORD)
{
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<reading>");
#endif
	try
	{
		if (buf==NULL)
			buf=(char *)malloc(sizeof(char)*(buflen+1));
		if (buf==NULL)
			throw NetworkError=(DWORD)ENL_RECVALLOC;

		if (!isTLSed)
		{
			NETLIBSELECT nls;
			memset(&nls, 0, sizeof(NETLIBSELECT));
			nls.cbSize = sizeof(NETLIBSELECT);
			nls.dwTimeout = 60000;
			nls.hReadConns[0] = hConnection;
			switch (CallService(MS_NETLIB_SELECT, 0, (LPARAM) &nls)) 
			{
				case SOCKET_ERROR:
				free(buf);
				SystemError=WSAGetLastError();
				throw NetworkError = (DWORD) ENL_RECV;
				case 0: // time out!
				free(buf);
				throw NetworkError = (DWORD) ENL_TIMEOUT;
			}
 		}

		memset(buf, 0, buflen);
		if (SOCKET_ERROR==(Rcv=LocalNetlib_Recv(hConnection,buf,buflen,MSG_DUMPASTEXT)))
		{
			free(buf);
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_RECV;
		}
		if (!Rcv)
		{
			free(buf);
			SystemError=WSAGetLastError();
			throw NetworkError=(DWORD)ENL_RECV;
		}
#ifdef DEBUG_COMM
		*(buf+Rcv)=0;				//end the buffer to write it to file
		DebugLog(CommFile,"%s",buf);
		DebugLog(CommFile,"</reading>\n");
#endif
		return(buf);
	}
	catch(...)
	{
#ifdef DEBUG_COMM
		DebugLog(CommFile,"<error></reading>\n");
#endif
		throw;
	}
}

//Closes netlib connection
void CNLClient::Disconnect()
{
	Netlib_CloseHandle(hConnection);
	hConnection=(HANDLE)NULL;
}

//Uninitializes netlib library
void UnregisterNLClient()
{
#ifdef DEBUG_COMM
	DebugLog(CommFile,"<Unregister PROXY support>");
#endif
	
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser=(HANDLE)NULL;
#ifdef DEBUG_COMM
	DebugLog(CommFile,"</Unregister PROXY support>\n");
#endif
}
