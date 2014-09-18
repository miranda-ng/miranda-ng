/*
 * ByteArray.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "ByteArray.h"
#include "WAException.h"
#include <iostream>
#include <algorithm>
#include "utilities.h"

ByteArrayOutputStream::ByteArrayOutputStream(int size) {
	this->buf = new std::vector<unsigned char>();
	this->buf->reserve(size);
	this->position = 0;
}

void ByteArrayOutputStream::setLength(size_t length) {
	this->buf->resize(length);
}

size_t ByteArrayOutputStream::getLength() {
	return this->buf->size();
}

size_t ByteArrayOutputStream::getCapacity() {
	return this->buf->capacity();
}

size_t ByteArrayOutputStream::getPosition() {
	return this->position;
}

void ByteArrayOutputStream::setPosition(size_t count) {
	this->position = count;
}


std::vector<unsigned char>* ByteArrayOutputStream::toByteArray() {
	std::vector<unsigned char>* array = new std::vector<unsigned char>(this->buf->size());
	for (size_t i = 0; i < this->buf->size(); i++)
		(*array)[i] = (*this->buf)[i];
	return array;
}

std::vector<unsigned char>* ByteArrayOutputStream::getBuffer() {
	return this->buf;
}

void ByteArrayOutputStream::write(int i) {
	if (this->position == this->buf->size())
		this->buf->push_back((unsigned char) i);
	else
		(*this->buf)[this->position] = (unsigned char) i;
	this->position = this->position + 1;
}

void ByteArrayOutputStream::write(unsigned char* b, size_t len) {
	if (len == 0)
		return;

	for (size_t i = 0; i < len; i++)
		write(b[i]);
}

void ByteArrayOutputStream::write(const std::string& s) {
	for (size_t i = 0; i < s.size(); i++)
		write((unsigned char) s[i]);
}


ByteArrayOutputStream::~ByteArrayOutputStream() {
	delete this->buf;
}


ByteArrayInputStream::ByteArrayInputStream(std::vector<unsigned char>* buf,  size_t off, size_t length ) {
	this->buf = buf;
	this->pos = off;
	this->count = min(off + length, buf->size());
}

ByteArrayInputStream::ByteArrayInputStream(std::vector<unsigned char>* buf)  {
	this->buf = buf;
	this->pos = 0;
	this->count = buf->size();
}

int ByteArrayInputStream::read() {
	return (pos < count) ? ((*this->buf)[pos++]) : -1;
}

int ByteArrayInputStream::read(std::vector<unsigned char>& b, size_t  off, size_t len) {
	if (len > (b.size() - off)) {
		throw new WAException("Index out of bounds");
	} else if (len == 0) {
		return 0;
	}

	int c = read();
	if (c == -1) {
		return -1;
	}
	b[off] = (unsigned char) c;

	size_t i = 1;
	try {
		for (; i < len ; i++) {
			c = read();
			if (c == -1) {
				break;
			}
			b[off + i] = (unsigned char) c;
		}
	} catch (std::exception& ) {
	}
	return (int)i;
}

ByteArrayInputStream::~ByteArrayInputStream() {
}

void ByteArrayInputStream::print() {
	std::cout << "[";
	for (size_t i = 0; i < this->count; i++) {
		std::cout << (*this->buf)[i] << " ";
	}
	std::cout << std::endl;
	for (size_t i = 0; i < this->count; i++) {
		std::cout << (int) ((signed char) (*this->buf)[i]) << " ";
	}
	std::cout << "]" << std::endl;
}

void ByteArrayOutputStream::print() {
	_LOGDATA("[");

	std::string chars(this->buf->begin(), this->buf->end());
	_LOGDATA("%s ", chars.c_str());

	std::string numbers = "";
	for (size_t i = 0; i < this->buf->size(); i++) {
		numbers +=  Utilities::intToStr((int) ((signed char) (*this->buf)[i])) + " ";
	}
	_LOGDATA("%s", numbers.c_str());
	_LOGDATA("]");
}
