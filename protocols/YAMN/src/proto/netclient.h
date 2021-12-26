#ifndef __CLIENT_H
#define __CLIENT_H

class CNetClient
{
public:
	CNetClient(): Stopped(FALSE) {}
	virtual ~CNetClient() {}

	virtual void Connect(const char* servername, const int port)=0;
	virtual void Send(const char *query)=0;
	virtual char* Recv(char *buf= nullptr, int buflen=65536)=0;
	virtual void Disconnect()=0;
	virtual BOOL Connected()=0;
	virtual void SSLify()=0;

	BOOL Stopped;
	int Rcv;
	uint32_t NetworkError;
	uint32_t SystemError;
	BOOL ifTLSed;
};

#endif
