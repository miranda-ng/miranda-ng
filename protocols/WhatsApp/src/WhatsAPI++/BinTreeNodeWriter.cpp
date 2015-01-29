/*
 * BinTreeNodeWriter.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "BinTreeNodeWriter.h"
#include "utilities.h"

BinTreeNodeWriter::BinTreeNodeWriter(WAConnection* conn, ISocketConnection* connection, IMutex* mutex)
{
	this->bSecure = false;
	this->mutex = mutex;
	this->conn = conn;
	this->out = new ByteArrayOutputStream(2048);
	this->realOut = connection;
	this->dataBegin = 0;
}

BinTreeNodeWriter::~BinTreeNodeWriter()
{
	delete this->out;
}

void BinTreeNodeWriter::writeDummyHeader()
{
	int num = 3;
	this->dataBegin = (int)out->getPosition();
	int num2 = this->dataBegin + num;
	out->setLength(num2);
	out->setPosition(num2);
}

void BinTreeNodeWriter::processBuffer()
{
	unsigned char num = 0;
	if (bSecure) {
		long num2 = (long)out->getLength() + 4L;
		out->setLength(num2);
		out->setPosition(num2);
		num = 8;
	}
	long num3 = (long)out->getLength() - 3L - (long) this->dataBegin;
	if (num3 >= 1048576L)
		throw WAException("Buffer too large: " + num3, WAException::CORRUPT_STREAM_EX, 0);

	std::vector<unsigned char>& buffer = out->getBuffer();
	this->realOut->dump(buffer.data(), (int)buffer.size());
	if (bSecure) {
		int num4 = (int)num3 - 4;
		this->conn->outputKey.encodeMessage(buffer.data(), this->dataBegin + 3 + num4, this->dataBegin + 3, num4);
	}
	buffer[dataBegin] = (unsigned char)((unsigned long)((unsigned long)num << 4) | (unsigned long)((num3 & 16711680L) >> 16));
	buffer[dataBegin + 1] = (unsigned char)((num3 & 65280L) >> 8);
	buffer[dataBegin + 2] = (unsigned char)(num3 & 255L);
}

void BinTreeNodeWriter::streamStart(std::string domain, std::string resource)
{
	this->mutex->lock();
	try {
		out->setPosition(0);
		out->setLength(0);
		out->write('W');
		out->write('A');
		out->write(1);
		out->write(5);

		std::map<string, string> attributes;
		attributes["to"] = domain;
		attributes["resource"] = resource;
		this->writeDummyHeader();
		this->writeListStart((int)attributes.size() * 2 + 1);
		out->write(1);
		this->writeAttributes(&attributes);
		this->processBuffer();
		this->flushBuffer(true, 0);
	}
	catch (exception& ex) {
		this->mutex->unlock();
		throw ex;
	}
	this->mutex->unlock();
}

void BinTreeNodeWriter::writeListStart(int i)
{
	if (i == 0) {
		out->write(0);
	}
	else if (i < 256) {
		out->write(248);
		writeInt8(i);
	}
	else {
		out->write(249);
		writeInt16(i);
	}
}

void BinTreeNodeWriter::writeInt8(int v)
{
	out->write(v & 0xFF);
}

void BinTreeNodeWriter::writeInt16(int v, ISocketConnection* o)
{
	o->write((v & 0xFF00) >> 8);
	o->write((v & 0xFF) >> 0);
}

void BinTreeNodeWriter::writeInt16(int v)
{
	writeInt16(v, this->out);
}

void BinTreeNodeWriter::writeInt16(int v, ByteArrayOutputStream* o)
{
	o->write((v & 0xFF00) >> 8);
	o->write((v & 0xFF) >> 0);
}

void BinTreeNodeWriter::writeAttributes(std::map<string, string>* attributes)
{
	if (attributes != NULL) {
		std::map<string, string>::iterator ii;
		for (ii = attributes->begin(); ii != attributes->end(); ii++) {
			writeString(ii->first);
			writeString(ii->second);
		}
	}
}

void BinTreeNodeWriter::writeString(const std::string& tag)
{
	int token = WAConnection::tokenLookup(tag);
	if (token != -1)
		writeToken(token);
	else {
		size_t atIndex = tag.find('@');
		if (atIndex == 0 || atIndex == string::npos)
			writeBytes((unsigned char*)tag.data(), (int)tag.length());
		else {
			std::string server = tag.substr(atIndex + 1);
			std::string user = tag.substr(0, atIndex);
			writeJid(&user, server);
		}
	}
}

void BinTreeNodeWriter::writeJid(std::string* user, const std::string& server)
{
	out->write(250);
	if (user != NULL && !user->empty())
		writeString(*user);
	else
		writeToken(0);

	writeString(server);

}

void BinTreeNodeWriter::writeToken(int intValue)
{
	if (intValue & 0x100) {
		out->write(236);
		out->write(intValue & 0xFF);
	}
	else out->write(intValue);
}

void BinTreeNodeWriter::writeBytes(unsigned char* bytes, int length)
{
	if (length >= 256) {
		out->write(253);
		writeInt24(length);
	}
	else {
		out->write(252);
		writeInt8(length);
	}
	out->write(bytes, length);
}

void BinTreeNodeWriter::writeInt24(int v)
{
	out->write((v & 0xFF0000) >> 16);
	out->write((v & 0xFF00) >> 8);
	out->write(v & 0xFF);
}

void BinTreeNodeWriter::writeInternal(const ProtocolTreeNode &node)
{
	writeListStart(
		1 + (node.attributes == NULL ? 0 : (int)node.attributes->size() * 2)
		+ (node.children == NULL ? 0 : 1)
		+ (node.data == NULL ? 0 : 1));
	writeString(node.tag);
	writeAttributes(node.attributes);
	if (node.data != NULL)
		writeBytes((unsigned char*)node.data->data(), (int)node.data->size());

	if (node.children != NULL && !node.children->empty()) {
		writeListStart((int)node.children->size());
		for (size_t a = 0; a < node.children->size(); a++)
			writeInternal(*(*node.children)[a]);
	}
}

void BinTreeNodeWriter::flushBuffer(bool flushNetwork)
{
	this->flushBuffer(flushNetwork, this->dataBegin);
}

void BinTreeNodeWriter::flushBuffer(bool flushNetwork, int startingOffset)
{
	try {
		this->processBuffer();
	}
	catch (WAException& ex) {
		out->setPosition(0);
		out->setLength(0);
		throw ex;
	}

	std::vector<unsigned char> buffer(out->getBuffer().begin(), out->getBuffer().end());
	int num = (int)(out->getLength() - (long)startingOffset);
	if (flushNetwork && ((long)out->getCapacity() - out->getLength() < 3L || out->getLength() > 4096L)) {
		delete this->out;
		this->out = new ByteArrayOutputStream(4096);
	}

	if (flushNetwork) {
		this->realOut->write(buffer, startingOffset, num);
		out->setPosition(0);
		out->setLength(0);
	}
}

void BinTreeNodeWriter::streamEnd()
{
	this->mutex->lock();
	try {
		writeListStart(1);
		out->write(2);
		flushBuffer(true);
	}
	catch (exception& ex) {
		this->mutex->unlock();
		throw ex;
	}
	this->mutex->unlock();
}

void BinTreeNodeWriter::write(const ProtocolTreeNode& node)
{
	write(node, true);
}

void BinTreeNodeWriter::write(const ProtocolTreeNode &node, bool needsFlush)
{
	this->mutex->lock();
	try {
		this->writeDummyHeader();
		#ifdef _DEBUG
		if (bSecure) {
			string tmp = node.toString();
			this->realOut->log(tmp.c_str());
		}
		#endif
		if (node.tag.empty())
			out->write(0);
		else
			writeInternal(node);
		flushBuffer(needsFlush);
	}
	catch (exception& ex) {
		this->mutex->unlock();
		throw WAException(ex.what());
	}
	this->mutex->unlock();
}
