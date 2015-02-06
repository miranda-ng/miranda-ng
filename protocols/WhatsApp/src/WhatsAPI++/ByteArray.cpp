/*
 * ByteArray.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "ByteArray.h"
#include "WAException.h"
#include "utilities.h"

ByteArrayOutputStream::ByteArrayOutputStream(int size)
{
	buf.reserve(size);
	position = 0;
}

void ByteArrayOutputStream::setLength(size_t length)
{
	buf.resize(length);
}

void ByteArrayOutputStream::setPosition(size_t count)
{
	position = count;
}


std::vector<unsigned char>& ByteArrayOutputStream::getBuffer()
{
	return buf;
}

void ByteArrayOutputStream::write(int i)
{
	if (this->position == this->buf.size())
		buf.push_back((unsigned char)i);
	else
		buf[position] = (unsigned char)i;
	position++;
}

void ByteArrayOutputStream::write(unsigned char* b, size_t len)
{
	if (len == 0)
		return;

	for (size_t i = 0; i < len; i++)
		write(b[i]);
}

void ByteArrayOutputStream::write(const std::string &s)
{
	for (size_t i = 0; i < s.size(); i++)
		write((unsigned char)s[i]);
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
}

ByteArrayInputStream::ByteArrayInputStream(std::vector<unsigned char>* buf, size_t off, size_t length)
{
	this->buf = buf;
	this->pos = off;
	this->count = min(off + length, buf->size());
}

ByteArrayInputStream::ByteArrayInputStream(std::vector<unsigned char>* buf)
{
	this->buf = buf;
	this->pos = 0;
	this->count = buf->size();
}

int ByteArrayInputStream::read()
{
	return (pos < count) ? ((*this->buf)[pos++]) : -1;
}

int ByteArrayInputStream::read(std::vector<unsigned char>& b, size_t  off, size_t len)
{
	if (len > (b.size() - off))
		throw new WAException("Index out of bounds");

	if (len == 0)
		return 0;

	int c = read();
	if (c == -1)
		return -1;

	b[off] = (unsigned char)c;

	size_t i = 1;
	try {
		for (; i < len; i++) {
			c = read();
			if (c == -1)
				break;

			b[off + i] = (unsigned char)c;
		}
	}
	catch (std::exception&) {
	}
	return (int)i;
}

ByteArrayInputStream::~ByteArrayInputStream()
{}
