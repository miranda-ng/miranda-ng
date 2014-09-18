/*
 * BinTreeNodeReader.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include <algorithm>
#include "BinTreeNodeReader.h"
#include "WAException.h"
#include "ProtocolTreeNode.h"
#include "utilities.h"

BinTreeNodeReader::BinTreeNodeReader(WAConnection* conn, ISocketConnection* connection, const char** dictionary, const int dictionarysize) {
	this->conn = conn;
	this->rawIn = connection;
	this->tokenMap = dictionary;
	this->tokenmapsize = dictionarysize;
	this->readSize = 1;
	this->in = NULL;
	this->buf = new std::vector<unsigned char>(BUFFER_SIZE);
}

BinTreeNodeReader::~BinTreeNodeReader() {
	if (this->buf != NULL)
		delete this->buf;
	if (this->in != NULL)
		delete this->in;
}

ProtocolTreeNode* BinTreeNodeReader::nextTreeInternal() {
	int b = this->in->read();
	int size = readListSize(b);
	b = this->in->read();
	if (b == 2)
		return NULL;

	std::string* tag = this->readStringAsString(b);

	if ((size == 0) || (tag == NULL))
		throw WAException("nextTree sees 0 list or null tag", WAException::CORRUPT_STREAM_EX, -1);
	int attribCount = (size - 2 + size % 2) / 2;
	std::map<string,string>* attribs = readAttributes(attribCount);
	if (size % 2 == 1) {
		ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, attribs);
		delete tag;
		return ret;
	}
	b = this->in->read();
	if (isListTag(b)) {
		ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, attribs, NULL, readList(b));
		delete tag;
		return ret;
	}

	ReadData* obj = this->readString(b);
	std::vector<unsigned char>* data;
	if (obj->type == STRING) {
		std::string* s = (std::string*) obj->data;
		data = new std::vector<unsigned char>(s->begin(), s->end());
		delete s;
	} else {
		data = (std::vector<unsigned char>*) obj->data;
	}

	ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, attribs, data);
	delete obj;
	delete tag;
	return ret;
}

bool BinTreeNodeReader::isListTag(int b) {
	return (b == 248) || (b == 0) || (b == 249);
}

void BinTreeNodeReader::decodeStream(int flags, int offset, int length) {
	if ((flags & 8) != 0) {
		if (length < 4) {
			throw WAException("invalid length"  + length, WAException::CORRUPT_STREAM_EX, 0);
		}
		offset += 4;
		length -= 4;
		this->conn->inputKey->decodeMessage(&(*this->buf)[0], offset - 4, offset, length);
	}
	if (this->in != NULL)
		delete this->in;
	this->in = new ByteArrayInputStream(this->buf, offset, length);
}

std::map<string, string>* BinTreeNodeReader::readAttributes(int attribCount) {
	std::map<string, string>* attribs = new std::map<string, string>();
	for (int i = 0; i < attribCount; i++) {
		std::string* key = readStringAsString();
		std::string* value = readStringAsString();
		(*attribs)[*key] = *value;
		delete key;
		delete value;
	}
	return attribs;
}

std::vector<ProtocolTreeNode*>* BinTreeNodeReader::readList(int token) {
	int size = readListSize(token);
	std::vector<ProtocolTreeNode*>* list = new std::vector<ProtocolTreeNode*>(size);
	for (int i = 0; i < size; i++) {
		(*list)[i] = nextTreeInternal();
	}

	return list;
}

int BinTreeNodeReader::readListSize(int token) {
	int size;
	if (token == 0) {
		size = 0;
	}
	else {
		size = 0;
		if (token == 248) {
			size = readInt8(this->in);
		}
		else
		{
			size = 0;
			if (token == 249)
				size = readInt16(this->in);
			else
				throw new WAException("invalid list size in readListSize: token " + token, WAException::CORRUPT_STREAM_EX, 0);
		}
	}

	return size;
}

std::vector<ProtocolTreeNode*>* BinTreeNodeReader::readList() {
	return readList(this->in->read());
}

ReadData* BinTreeNodeReader::readString() {
	return readString(this->in->read());
}

ReadData* BinTreeNodeReader::readString(int token) {
	if (token == -1) {
		throw WAException("-1 token in readString", WAException::CORRUPT_STREAM_EX, -1);
	}

	ReadData* ret = new ReadData();

	if ((token > 4) && (token < 245)) {
		ret->type = STRING;
		ret->data = new std::string(getToken(token));
		return ret;
	}

	switch(token) {
	case 0:
		return NULL;
	case 252: {
		int size8 = readInt8(this->in);
		std::vector<unsigned char>* buf8 = new std::vector<unsigned char>(size8);
		fillArray(*buf8, size8, this->in);
		// std::string* ret = new std::string(buf8->begin(), buf8->end());
		// delete buf8;
		ret->type = ARRAY;
		ret->data = buf8;
		return ret;
	}
	case 253: {
		int size24 = readInt24(this->in);
		std::vector<unsigned char>* buf24 = new std::vector<unsigned char>(size24);
		fillArray(*buf24, size24, this->in);
		// std::string* ret = new std::string(buf24->begin(), buf24->end());
		// delete buf24;
		ret->type = ARRAY;
		ret->data = buf24;

		return ret;
	}
	case 254: {
		token = (unsigned char) this->in->read();
		ret->type = STRING;
		ret->data = new std::string(getToken(245 + token));
		return ret;
	}
	case 250: {
		std::string* user = readStringAsString();
		std::string* server = readStringAsString();
		if ((user != NULL) && (server != NULL)) {
			std::string* result = new std::string(*user + "@" + *server);
			delete user;
			delete server;
			ret->type = STRING;
			ret->data = result;
			return ret;
		}
		if (server != NULL) {
			ret->type = STRING;
			ret->data = server;
			return ret;
		}
		throw WAException("readString couldn't reconstruct jid", WAException::CORRUPT_STREAM_EX, -1);
	}
	}
	throw WAException("readString couldn't match token" + (int) token, WAException::CORRUPT_STREAM_EX, -1);
}

std::string* BinTreeNodeReader::objectAsString(ReadData* o) {
	if (o->type == STRING) {
		return (std::string*) o->data;
	}

	if (o->type == ARRAY) {
		std::vector<unsigned char>* v = (std::vector<unsigned char>*) o->data;
		std::string* ret = new std::string(v->begin(), v->end());
		delete v;
		return ret;
	}

	return NULL;
}

std::string* BinTreeNodeReader::readStringAsString() {
	ReadData* o = this->readString();
	std::string* ret = this->objectAsString(o);
	delete o;
	return ret;
}

std::string* BinTreeNodeReader::readStringAsString(int token) {
	ReadData* o = this->readString(token);
	std::string* ret = this->objectAsString(o);
	delete o;
	return ret;
}


void BinTreeNodeReader::fillArray(std::vector<unsigned char>& buff, int len, ByteArrayInputStream* in) {
	int count = 0;
	while (count < len) {
		count += in->read(buff, count, len - count);
	}
}

void BinTreeNodeReader::fillArray(std::vector<unsigned char>& buff, int len, ISocketConnection* in) {
	int count = 0;
	while (count < len) {
		count += in->read(buff, count, len - count);
	}
}


std::string BinTreeNodeReader::getToken(int token) {
	std::string ret;

	if ((token >= 0) && (token < this->tokenmapsize))
		ret = std::string(this->tokenMap[token]);
	if (ret.empty()) {
		throw WAException("invalid token/length in getToken", WAException::CORRUPT_STREAM_EX, 0);
	}
	return ret;
}


void BinTreeNodeReader::getTopLevelStream() {
	int stanzaSize;
	int flags;
	int byte = readInt8(this->rawIn);
	flags = byte >> 4;
	int size0 = byte & 15;
	int size1 = readInt8(this->rawIn);
	int size2 = readInt8(this->rawIn);

	stanzaSize = (size0 << 16) + (size1 << 8) + size2;

	if (this->buf->size() < (size_t) stanzaSize) {
		int newsize = max((int) (this->buf->size() * 3 / 2), stanzaSize);
		delete this->buf;
		this->buf = new std::vector<unsigned char>(newsize);
	}
	fillArray(*this->buf, stanzaSize, this->rawIn);

	this->decodeStream(flags, 0, stanzaSize);
}

int BinTreeNodeReader::readInt8(ByteArrayInputStream* in) {
	return in->read();
}

int BinTreeNodeReader::readInt16(ByteArrayInputStream* in) {
	int intTop = in->read();
	int intBot = in->read();
	int value = (intTop << 8) + intBot;
	return value;
}

int BinTreeNodeReader::readInt24(ByteArrayInputStream* in) {
	int int1 = in->read();
	int int2 = in->read();
	int int3 = in->read();
	int value = (int1 << 16) + (int2 << 8) + int3;

	return value;
}

ProtocolTreeNode* BinTreeNodeReader::nextTree() {
	this->getTopLevelStream();
	return nextTreeInternal();
}

void BinTreeNodeReader::streamStart() {
	this->getTopLevelStream();

	int tag = this->in->read();
	int size = readListSize(tag);
	tag = this->in->read();
	if (tag != 1) {
		throw WAException("expecting STREAM_START in streamStart", WAException::CORRUPT_STREAM_EX, 0);
	}
	int attribCount = (size - 2 + size % 2) / 2;

	std::map<string,string>* attributes = readAttributes(attribCount);
	delete attributes;
}

int BinTreeNodeReader::readInt8(ISocketConnection* in) {
	return in->read();
}

int BinTreeNodeReader::readInt16(ISocketConnection* in) {
	int intTop = in->read();
	int intBot = in->read();
	int value = (intTop << 8) + intBot;
	return value;
}

int BinTreeNodeReader::readInt24(ISocketConnection* in) {
	int int1 = in->read();
	int int2 = in->read();
	int int3 = in->read();
	int value = (int1 << 16) + (int2 << 8) + int3;

	return value;
}
