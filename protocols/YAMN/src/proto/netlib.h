#ifndef __NETLIB_H
#define __NETLIB_H

class CNLClient: public CNetClient
{
public:
	CNLClient() {}
	void Connect(const char* servername, const int port);
	void Send(const char *query);
	char* Recv(char *buf= nullptr, int buflen = 65536);
	void Disconnect();
	void SSLify();
	
	inline BOOL Connected() {return hConnection != nullptr;}

protected:
	HNETLIBCONN hConnection = nullptr;
	BOOL isTLSed = false;
	int LocalNetlib_Send(HNETLIBCONN hConn, const char *buf, int len, int flags);
	int LocalNetlib_Recv(HNETLIBCONN hConn, char *buf, int len, int flags);
};

void SSL_DebugLog(const char *fmt, ...);

enum
{
	ENL_WINSOCKINIT=1,	//error initializing socket	//only wsock
	ENL_GETHOSTBYNAME,	//DNS error			//only wsock
	ENL_CREATESOCKET,	//error creating socket		//only wsock
	ENL_CONNECT,		//cannot connect to server
	ENL_SEND,			//cannot send data
	ENL_RECV,			//cannot receive data
	ENL_RECVALLOC,		//cannot allocate memory for received data
	ENL_TIMEOUT,		//timed out during recv
};

enum
{
	ESSL_NOTLOADED=1,	//OpenSSL is not loaded
	ESSL_WINSOCKINIT,	//WinSock 2.0 init failed
	ESSL_GETHOSTBYNAME,	//DNS error
	ESSL_CREATESOCKET,	//error creating socket
	ESSL_SOCKETCONNECT,	//error connecting with socket
	ESSL_CREATESSL,		//error creating SSL session structure
	ESSL_SETSOCKET,		//error connect socket with SSL session for bidirect I/O space
	ESSL_CONNECT,		//cannot connect to server
	ESSL_SEND,		//cannot send data
	ESSL_RECV,		//cannot receive data
	ESSL_RECVALLOC,		//cannot allocate memory for received data
};
#endif
