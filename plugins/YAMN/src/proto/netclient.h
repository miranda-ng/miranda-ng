#ifndef __CLIENT_H
#define __CLIENT_H

class CNetClient
{
public:
	CNetClient(): Stopped(FALSE) {}
	virtual void Connect(const char* servername,const int port)=0;
	virtual void Send(const char *query)=0;
	virtual char* Recv(char *buf=NULL,int buflen=65536)=0;
	virtual void Disconnect()=0;
	virtual BOOL Connected()=0;
	virtual void SSLify()=0;

	BOOL Stopped;
	int Rcv;
	DWORD NetworkError;
	DWORD SystemError;
	BOOL ifTLSed;
};

#endif
