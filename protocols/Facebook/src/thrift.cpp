/*

Facebook plugin for Miranda NG
Copyright © 2019-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/////////////////////////////////////////////////////////////////////////////////////////
// FbThrift class members

#include "stdafx.h"

static uint8_t encodeType(int type)
{
	switch (type) {
	case FB_THRIFT_TYPE_BOOL:
		return 2;
	case FB_THRIFT_TYPE_BYTE:
		return 3;
	case FB_THRIFT_TYPE_I16:
		return 4;
	case FB_THRIFT_TYPE_I32:
		return 5;
	case FB_THRIFT_TYPE_I64:
		return 6;
	case FB_THRIFT_TYPE_DOUBLE:
		return 7;
	case FB_THRIFT_TYPE_STRING:
		return 8;
	case FB_THRIFT_TYPE_LIST:
		return 9;
	case FB_THRIFT_TYPE_SET:
		return 10;
	case FB_THRIFT_TYPE_MAP:
		return 11;
	case FB_THRIFT_TYPE_STRUCT:
		return 12;
	default:
		return 0;
	}
}

FbThrift& FbThrift::operator<<(uint8_t value)
{
	m_buf.append(&value, 1);
	return *this;
}

FbThrift& FbThrift::operator<<(const char *str)
{
	size_t len = mir_strlen(str);
	writeIntV(len);
	m_buf.append(str, len);
	return *this;
}

void FbThrift::writeBool(bool bValue)
{
	uint8_t b = (bValue) ? 0x11 : 0x12;
	m_buf.append(&b, 1);
}

void FbThrift::writeBuf(const void *pData, size_t cbLen)
{
	m_buf.append(pData, cbLen);
}

void FbThrift::writeField(int iType)
{
	uint8_t type = encodeType(iType) + 0x10;
	m_buf.append(&type, 1);
}

void FbThrift::writeField(int iType, int id, int lastid)
{
	uint8_t type = encodeType(iType);
	uint8_t diff = uint8_t(id - lastid);
	if (diff > 0x0F) {
		m_buf.append(&type, 1);
		writeInt64(id);
	}
	else {
		type += (diff << 4);
		m_buf.append(&type, 1);
	}
}

void FbThrift::writeList(int iType, int size)
{
	uint8_t type = encodeType(iType);
	if (size > 14) {
		writeIntV(size);
		*this << (type | 0xF0);
	}
	else *this << (type | (size << 4));
}

void FbThrift::writeInt16(uint16_t value)
{
	value = htons(value);
	m_buf.append(&value, sizeof(value));
}

void FbThrift::writeInt32(int32_t value)
{
	writeIntV((value << 1) ^ (value >> 31));
}

void FbThrift::writeInt64(int64_t value)
{
	writeIntV((value << 1) ^ (value >> 63));
}

void FbThrift::writeIntV(uint64_t value)
{
	bool bLast;
	do {
		bLast = (value & ~0x7F) == 0;
		uint8_t b = value & 0x7F;
		if (!bLast) {
			b |= 0x80;
			value >>= 7;
		}
		m_buf.append(&b, 1);
	} while (!bLast);
}

/////////////////////////////////////////////////////////////////////////////////////////
// FbThriftReader class members

uint8_t FbThriftReader::decodeType(int type)
{
	switch (type) {
	case 0:
		return FB_THRIFT_TYPE_STOP;
	case 1:
		m_lastBval = m_lastBool = true;
		return FB_THRIFT_TYPE_BOOL;
	case 2:
		m_lastBool = true;
		m_lastBval = false;
		return FB_THRIFT_TYPE_BOOL;
	case 3:
		return FB_THRIFT_TYPE_BYTE;
	case 4:
		return FB_THRIFT_TYPE_I16;
	case 5:
		return FB_THRIFT_TYPE_I32;
	case 6:
		return FB_THRIFT_TYPE_I64;
	case 7:
		return FB_THRIFT_TYPE_DOUBLE;
	case 8:
		return FB_THRIFT_TYPE_STRING;
	case 9:
		return FB_THRIFT_TYPE_LIST;
	case 10:
		return FB_THRIFT_TYPE_SET;
	case 11:
		return FB_THRIFT_TYPE_MAP;
	case 12:
		return FB_THRIFT_TYPE_STRUCT;
	default:
		return 0;
	}
}

bool FbThriftReader::isStop()
{
	byte b;
	if (!readByte(b))
		return true;

	offset--;
	return b == FB_THRIFT_TYPE_STOP;
}

bool FbThriftReader::readBool(bool &bVal)
{
	if (m_lastBool) {
		bVal = m_lastBval;
		m_lastBool = false;
		return true;
	}

	byte b;
	if (!readByte(b)) 
		return false;

	bVal = b == 0x11;
	return true;
}

bool FbThriftReader::readByte(uint8_t &b)
{
	if (offset >= size())
		return false;

	b = *((uint8_t *)data() + offset);
	offset++;
	return true;
}

bool FbThriftReader::readField(uint8_t &type, uint16_t &id)
{
	byte b;
	if (!readByte(b))
		return false;

	type = decodeType(b & 0x0F);
	id = (b >> 4);
	return (id == 0) ? readInt16(id) : true;
}

bool FbThriftReader::readIntV(uint64_t &val)
{
	uint8_t b;
	unsigned i = 0;
	val = 0;

	do {
		if (!readByte(b))
			return false;

		val |= (uint64_t(b & 0x7F) << i);
		i += 7;
	} while ((b & 0x80) != 0);
	
	return true;
}

bool FbThriftReader::readList(uint8_t &type, uint32_t &size)
{
	byte b;
	if (!readByte(b))
		return false;

	type = decodeType(b & 0x0F);
	size = b >> 4;
	if (size == 0x0F) {
		uint64_t tmp;
		if (!readIntV(tmp))
			return false;
		size = (uint32_t)tmp;
	}
	return true;
}

bool FbThriftReader::readStr(char *&val)
{
	uint64_t tmp;
	if (!readIntV(tmp))
		return false;

	uint32_t cbLen = (uint32_t)tmp;
	if (offset + cbLen >= size())
		return false;

	if (cbLen > 0) {
		val = mir_strndup((char *)data() + offset, cbLen);
		offset += cbLen;
	}
	else val = nullptr;
	return true;
}

bool FbThriftReader::readInt16(uint16_t &val)
{
	if (offset + 2 >= size())
		return false;

	val = ntohs(*(u_short *)((char *)data() + offset));
	offset += 2;
	return true;
}

bool FbThriftReader::readInt32(uint32_t &val)
{
	uint64_t tmp;
	if (!readIntV(tmp))
		return false;

	val = (uint32_t )tmp;
	return true;
}

bool FbThriftReader::readInt64(uint64_t &val)
{
	uint64_t tmp;
	if (!readIntV(tmp))
		return false;

	val = (tmp >> 0x01) ^ -(tmp & 0x01);
	return true;
}
