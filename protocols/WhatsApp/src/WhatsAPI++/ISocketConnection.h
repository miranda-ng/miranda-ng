#ifndef ISOCKETCONNECTION_H_
#define ISOCKETCONNECTION_H_

#include <vector>

class ISocketConnection {

public:
   ISocketConnection() {}
	virtual ~ISocketConnection() {}

	virtual void write(int i) = 0;
	virtual unsigned char read() = 0;
	virtual void flush() = 0;
	virtual void write(const std::vector<unsigned char>& b, int length) = 0;
	virtual void write(const std::vector<unsigned char>& bytes, int offset, int length) = 0;
	virtual int read(unsigned char*, int length) = 0;
	virtual int read(std::vector<unsigned char>& b, int off, int length) = 0;
	virtual void makeNonBlock() = 0;
	virtual int waitForRead() = 0;
	virtual void forceShutdown() = 0;

	virtual void log(const char *prefix, const char *str) = 0;
};

#endif /* ISOCKETCONNECTION_H_ */
