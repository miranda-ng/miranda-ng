#if !defined(WASOCKETCONNECTION_H)
#define WASOCKETCONNECTION_H

#include "common.h"
#include "WhatsAPI++/ISocketConnection.h"
#include <iostream>
#include "WhatsAPI++/WAException.h"
#include "WhatsAPI++/WALogin.h"
#include <windows.h>

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
	WASocketConnection(const std::string& dir, int port) throw (WAException);
	void write(int i);
	unsigned char read();
	void flush();
	void write(const std::vector<unsigned char>& b, int length);
	void write(const std::vector<unsigned char>& bytes, int offset, int length);
	int read(std::vector<unsigned char>& b, int off, int length);
	void makeNonBlock();
	int waitForRead();
	void forceShutdown();

	virtual ~WASocketConnection();
	static void initNetwork(HANDLE hNetlibUser) throw (WAException);
	static void quitNetwork();
};

#endif // WASOCKETCONNECTION_H