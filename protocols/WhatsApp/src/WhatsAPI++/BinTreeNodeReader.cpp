/*
 * BinTreeNodeReader.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "BinTreeNodeReader.h"
#include "WAException.h"
#include "ProtocolTreeNode.h"
#include "utilities.h"

extern const char *dictionary[], *extended_dict[];

BinTreeNodeReader::BinTreeNodeReader(WAConnection *conn, ISocketConnection *connection) :
	buf(BUFFER_SIZE)
{
	this->conn = conn;
	this->rawIn = connection;
	this->readSize = 1;
	this->in = NULL;
}

BinTreeNodeReader::~BinTreeNodeReader()
{
	delete this->in;
}

ProtocolTreeNode* BinTreeNodeReader::nextTreeInternal()
{
	int b = this->in->read();
	int size = readListSize(b);
	b = this->in->read();
	if (b == 2)
		return NULL;

	std::string* tag = this->readStringAsString(b);

	if ((size == 0) || (tag == NULL))
		throw WAException("nextTree sees 0 list or null tag", WAException::CORRUPT_STREAM_EX, -1);
	int attribCount = (size - 2 + size % 2) / 2;
	std::map<string, string>* attribs = readAttributes(attribCount);
	if (size % 2 == 1) {
		ProtocolTreeNode* ret = new ProtocolTreeNode(*tag); ret->attributes = attribs;
		delete tag;
		return ret;
	}
	b = this->in->read();
	if (isListTag(b)) {
		ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, NULL, readList(b)); ret->attributes = attribs;
		delete tag;
		return ret;
	}

	ReadData* obj = this->readString(b);
	std::vector<unsigned char>* data;
	if (obj->type == STRING) {
		std::string* s = (std::string*) obj->data;
		data = new std::vector<unsigned char>(s->begin(), s->end());
		delete s;
	}
	else data = (std::vector<unsigned char>*) obj->data;

	ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, data); ret->attributes = attribs;
	delete obj;
	delete tag;
	return ret;
}

bool BinTreeNodeReader::isListTag(int b)
{
	return (b == 248) || (b == 0) || (b == 249);
}

void BinTreeNodeReader::decodeStream(int flags, int offset, int length)
{
	unsigned char *pData = (unsigned char*)&buf[0];

	if ((flags & 8) != 0) {
		if (length < 4)
			throw WAException("invalid length" + length, WAException::CORRUPT_STREAM_EX, 0);

		length -= 4;

		this->conn->inputKey.decodeMessage(pData, offset + length, 0, length);
	}

	if (this->in != NULL)
		delete this->in;
	this->in = new ByteArrayInputStream(&this->buf, offset, length);
}

std::map<string, string>* BinTreeNodeReader::readAttributes(int attribCount)
{
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

std::vector<ProtocolTreeNode*>* BinTreeNodeReader::readList(int token)
{
	int size = readListSize(token);
	std::vector<ProtocolTreeNode*>* list = new std::vector<ProtocolTreeNode*>(size);
	for (int i = 0; i < size; i++)
		(*list)[i] = nextTreeInternal();

	return list;
}

int BinTreeNodeReader::readListSize(int token)
{
	switch (token) {
	case 0:   return 0;
	case 248: return readInt8(this->in);
	case 249: return readInt16(this->in);
	default:
		throw new WAException("invalid list size in readListSize: token " + token, WAException::CORRUPT_STREAM_EX, 0);
	}
	return 0;
}

std::vector<ProtocolTreeNode*>* BinTreeNodeReader::readList()
{
	return readList(this->in->read());
}

ReadData* BinTreeNodeReader::readString()
{
	return readString(this->in->read());
}

ReadData* BinTreeNodeReader::readString(int token)
{
	if (token == -1)
		throw WAException("-1 token in readString", WAException::CORRUPT_STREAM_EX, -1);

	int bSize;
	ReadData *ret = new ReadData();

	if (token > 2 && token <= 236) {
		if (token != 236)
			ret->data = new std::string(dictionary[token]);
		else {
			token = readInt8(this->in);
			ret->data = new std::string(extended_dict[token]);
		}

		ret->type = STRING;
		return ret;
	}

	switch (token) {
	case 0:
		return NULL;

	case 252:
		bSize = readInt8(this->in);
		{
			std::vector<unsigned char>* buf8 = new std::vector<unsigned char>(bSize);
			fillArray(*buf8, bSize, this->in);
			ret->type = ARRAY;
			ret->data = buf8;
		}
		return ret;

	case 253:
		bSize = readInt24(this->in);
		{
			std::vector<unsigned char>* buf24 = new std::vector<unsigned char>(bSize);
			fillArray(*buf24, bSize, this->in);
			ret->type = ARRAY;
			ret->data = buf24;
		}
		return ret;

	case 255:
		bSize = readInt8(this->in);
		{
			int size = bSize & 0x7f;
			int numnibbles = size * 2 - ((bSize & 0x80) ? 1 : 0);

			std::vector<unsigned char> tmp(size);
			fillArray(tmp, size, this->in);
			std::string s;
			for (int i = 0; i < numnibbles; i++) {
				char c = (tmp[i / 2] >> (4 - ((i & 1) << 2))) & 0xF;
				if (c < 10) s += (c + '0');
				else s += (c - 10 + '-');
			}

			ret->type = STRING;
			ret->data = new std::string(s);
		}
		return ret;

	case 250:
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
	throw WAException("readString couldn't match token" + (int)token, WAException::CORRUPT_STREAM_EX, -1);
}

std::string* BinTreeNodeReader::objectAsString(ReadData* o)
{
	if (o->type == STRING)
		return (std::string*) o->data;

	if (o->type == ARRAY) {
		std::vector<unsigned char>* v = (std::vector<unsigned char>*) o->data;
		std::string* ret = new std::string(v->begin(), v->end());
		delete v;
		return ret;
	}

	return NULL;
}

std::string* BinTreeNodeReader::readStringAsString()
{
	ReadData* o = this->readString();
	std::string* ret = this->objectAsString(o);
	delete o;
	return ret;
}

std::string* BinTreeNodeReader::readStringAsString(int token)
{
	ReadData* o = this->readString(token);
	std::string* ret = this->objectAsString(o);
	delete o;
	return ret;
}

void BinTreeNodeReader::fillArray(std::vector<unsigned char>& buff, int len, ByteArrayInputStream* in)
{
	int count = 0;
	while (count < len)
		count += in->read(buff, count, len - count);
}

void BinTreeNodeReader::fillArray(std::vector<unsigned char>& buff, int len, ISocketConnection *in)
{
	int count = 0;
	while (count < len)
		count += in->read(buff, count, len - count);
}

void BinTreeNodeReader::getTopLevelStream()
{
	int stanzaSize = readInt24(this->rawIn);
	int flags = (stanzaSize >> 20);
	stanzaSize &= 0x0FFFFF;	

	if (this->buf.size() < (size_t)stanzaSize) {
		int newsize = max((int)(this->buf.size() * 3 / 2), stanzaSize);
		this->buf.resize(newsize);
	}
	fillArray(this->buf, stanzaSize, this->rawIn);

	this->decodeStream(flags, 0, stanzaSize);
}

int BinTreeNodeReader::readInt8(ByteArrayInputStream* in)
{
	return in->read();
}

int BinTreeNodeReader::readInt16(ByteArrayInputStream* in)
{
	int intTop = in->read();
	int intBot = in->read();
	int value = (intTop << 8) + intBot;
	return value;
}

int BinTreeNodeReader::readInt24(ByteArrayInputStream* in)
{
	int int1 = in->read();
	int int2 = in->read();
	int int3 = in->read();
	int value = (int1 << 16) + (int2 << 8) + int3;

	return value;
}

ProtocolTreeNode* BinTreeNodeReader::nextTree()
{
	this->getTopLevelStream();
	return nextTreeInternal();
}

void BinTreeNodeReader::streamStart()
{
	this->getTopLevelStream();

	int tag = this->in->read();
	int size = readListSize(tag);
	tag = this->in->read();
	if (tag != 1)
		throw WAException("expecting STREAM_START in streamStart", WAException::CORRUPT_STREAM_EX, 0);

	int attribCount = (size - 2 + size % 2) / 2;
	std::map<string, string>* attributes = readAttributes(attribCount);
	delete attributes;
}

int BinTreeNodeReader::readInt8(ISocketConnection *in)
{
	return in->read();
}

int BinTreeNodeReader::readInt16(ISocketConnection *in)
{
	unsigned char data[2];
	in->read(data, 2);
	return (int(data[0]) << 8) + int(data[1]);
}

int BinTreeNodeReader::readInt24(ISocketConnection *in)
{
	unsigned char data[3];
	in->read(data, 3);
	return (int(data[0]) << 16) + (int(data[1]) << 8) + int(data[2]);
}
