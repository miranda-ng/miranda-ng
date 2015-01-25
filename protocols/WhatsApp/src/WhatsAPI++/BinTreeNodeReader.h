/*
 * BinTreeNodeReader.h
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */
#ifndef BINTREENODEREADER_H_
#define BINTREENODEREADER_H_

#include "ProtocolTreeNode.h"
#include "ISocketConnection.h"
#include "ByteArray.h"
#include "WAConnection.h"
#include <string>
#include <vector>
#include <map>

#define BUFFER_SIZE 512

class WAConnection;

enum ReadType {STRING, ARRAY};

class ReadData {
public:
	ReadData() {};
	virtual ~ReadData() {};

	ReadType type;
	void * data;
};

class BinTreeNodeReader {
private:
	ISocketConnection *rawIn;
	ByteArrayInputStream* in;
	std::vector<unsigned char> buf;
	int readSize;
	WAConnection *conn;

	ProtocolTreeNode* nextTreeInternal();
	bool isListTag(int b);
	void decodeStream(int flags, int offset, int length);
	std::map<string,string>* readAttributes(int attribCount);
	std::vector<ProtocolTreeNode*>* readList(int token);
	int readListSize(int token);
	std::vector<ProtocolTreeNode*>* readList();
	ReadData* readString();
	ReadData* readString(int token);
	static void fillArray(std::vector<unsigned char>& buff, int len, ByteArrayInputStream* in);
	static void fillArray(std::vector<unsigned char>& buff, int len, ISocketConnection* in);
	std::string* objectAsString(ReadData* o);
	std::string* readStringAsString();
	std::string* readStringAsString(int token);
	void getTopLevelStream();
	static int readInt8(ByteArrayInputStream* in);
	static int readInt8(ISocketConnection* in);
	static int readInt16(ByteArrayInputStream* in);
	static int readInt16(ISocketConnection* in);
	static int readInt24(ByteArrayInputStream* in);
	static int readInt24(ISocketConnection* in);


public:
	BinTreeNodeReader(WAConnection* conn, ISocketConnection* connection);
	virtual ~BinTreeNodeReader();
	ProtocolTreeNode* nextTree();
	void streamStart();

};

#endif /* BINTREENODEREADER_H_ */

