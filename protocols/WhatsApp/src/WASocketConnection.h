#if !defined(WASOCKETCONNECTION_H)
#define WASOCKETCONNECTION_H

#include "WhatsAPI++/ISocketConnection.h"
#include "WhatsAPI++/WAException.h"

class WASocketConnection : public ISocketConnection
{
public:
	static HANDLE hNetlibUser;

private:
	int readSize;
	int maxBufRead;
	bool connected;

	HANDLE hConn;

public:
	WASocketConnection(const std::string &dir, int port) throw (WAException);
	virtual ~WASocketConnection();

	virtual void write(int i);
	virtual unsigned char read();
	virtual int read(std::vector<unsigned char>& b, int off, int length);
	virtual int read(unsigned char*, int length);
	virtual void flush();
	virtual void write(const std::vector<unsigned char>& b, int length);
	// virtual void write(const std::vector<unsigned char>& bytes, int offset, int length);
	virtual void makeNonBlock();
	virtual int waitForRead();
	virtual void forceShutdown();
	
	virtual void log(const char *prefix, const char *str);

	static void initNetwork(HANDLE hNetlibUser) throw (WAException);
	static void quitNetwork();
};

#endif // WASOCKETCONNECTION_H