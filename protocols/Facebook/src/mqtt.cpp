/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

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
	writeInt32((int)len);
	m_buf.append((void*)str, len);
	return *this;
}

void FbThrift::writeBool(bool bValue)
{
	uint8_t b = (bValue) ? 1 : 2;
	m_buf.append(&b, 1);
}

void FbThrift::writeBuf(const void *pData, size_t cbLen)
{
	m_buf.append((void*)pData, cbLen);
}

void FbThrift::writeField(int iType, int id)
{
	uint8_t type = encodeType(iType);
	m_buf.append(&type, 1);
	writeInt64(id);
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

void FbThrift::writeInt32(__int32 value)
{
	writeIntV((value << 1) ^ (value >> 31));
}

void FbThrift::writeInt64(__int64 value)
{
	writeIntV((value << 1) ^ (value >> 63));
}

void FbThrift::writeIntV(__int64 value)
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
// MQTT functions

void FacebookProto::MqttOpen()
{
	FbThrift thrift;
	thrift.writeField(FB_THRIFT_TYPE_STRING, 1);  // Client identifier
	thrift << m_szClientID;

	thrift.writeField(FB_THRIFT_TYPE_STRUCT, 4, 1);

	thrift.writeField(FB_THRIFT_TYPE_I64, 1); // User identifier
	thrift.writeInt64(m_uid);

	thrift.writeField(FB_THRIFT_TYPE_STRING, 2); // User agent
	thrift << NETLIB_USER_AGENT;

	thrift.writeField(FB_THRIFT_TYPE_I64, 3);
	thrift.writeInt64(23);
	thrift.writeField(FB_THRIFT_TYPE_I64, 4);
	thrift.writeInt64(26);
	thrift.writeField(FB_THRIFT_TYPE_I32, 5);
	thrift.writeInt32(1);
	thrift.writeField(FB_THRIFT_TYPE_BOOL, 6);
	thrift.writeBool(true);
	thrift.writeField(FB_THRIFT_TYPE_BOOL, 7); // visibility
	thrift.writeBool(m_iStatus != ID_STATUS_INVISIBLE);

	thrift.writeField(FB_THRIFT_TYPE_STRING, 8); // device id
	thrift << m_szDeviceID;

	thrift.writeField(FB_THRIFT_TYPE_BOOL, 9);
	thrift.writeBool(true);
	thrift.writeField(FB_THRIFT_TYPE_I32, 10);
	thrift.writeInt32(1);
	thrift.writeField(FB_THRIFT_TYPE_I32, 11);
	thrift.writeInt32(0);
	thrift.writeField(FB_THRIFT_TYPE_I64, 12);
	thrift.writeInt64(m_iMqttId);

	thrift.writeField(FB_THRIFT_TYPE_LIST, 14, 12);
	thrift.writeField(FB_THRIFT_TYPE_I32, 0);
	thrift << (BYTE)0;

	thrift.writeField(FB_THRIFT_TYPE_LIST, 15);
	thrift << m_szAuthToken << (BYTE)0;
}
