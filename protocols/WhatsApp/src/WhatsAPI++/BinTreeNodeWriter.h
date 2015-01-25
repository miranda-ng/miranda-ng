/*
 * BinTreeNodeWriter.h
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */


#ifndef BINTREENODEWRITER_H_
#define BINTREENODEWRITER_H_

#include <string>
#include "ProtocolTreeNode.h"
#include "ISocketConnection.h"
#include "ByteArray.h"
#include "IMutex.h"
#include "WAConnection.h"

using namespace std;

#define STREAM_START 1
#define	STREAM_END 2
#define LIST_EMPTY 0
#define	LIST_8 248
#define	LIST_16 249
#define	JID_PAIR 250
#define	BINARY_8 252
#define	BINARY_24 253
#define	TOKEN_8 254

#include <map>

class WAConnection;

class BinTreeNodeWriter {
private:
	WAConnection* conn;
	ISocketConnection *realOut;
	ByteArrayOutputStream *out;
	IMutex* mutex;
	int dataBegin;
	bool bLoggedIn;

	void writeListStart(int i);
	void writeInt8(int v);
	void writeInt16(int v, ISocketConnection* out);
	void writeInt16(int v, ByteArrayOutputStream* out);
	void writeInt16(int v);
	void writeAttributes(std::map<string, string>* attributes);
	void writeString(const std::string& tag);
	void writeJid(std::string* user, const std::string& server);
	void writeToken(int intValue);
	void writeBytes(unsigned char* bytes, int length);
	void writeInt24(int v);
	void writeInternal(const ProtocolTreeNode &node);
	void writeDummyHeader();
	void processBuffer();

public:
	BinTreeNodeWriter(WAConnection* conn, ISocketConnection* connection, IMutex* mutex);
	virtual ~BinTreeNodeWriter();

	void streamStart(std::string domain, std::string resource);
	void flushBuffer(bool flushNetwork);
	void flushBuffer(bool flushNetwork, int startingOffset);
	void streamEnd();
	void write(const ProtocolTreeNode &node);
	void write(const ProtocolTreeNode &node, bool needsFlush);

	void setLoggedIn() { bLoggedIn = true; }
};

#endif /* BINTREENODEWRITER_H_ */
