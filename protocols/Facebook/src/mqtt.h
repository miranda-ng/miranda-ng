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

#pragma once

#define FB_THRIFT_TYPE_STOP   0
#define FB_THRIFT_TYPE_VOID   1
#define FB_THRIFT_TYPE_BOOL   2
#define FB_THRIFT_TYPE_BYTE   3
#define FB_THRIFT_TYPE_DOUBLE 4
#define FB_THRIFT_TYPE_I16    6
#define FB_THRIFT_TYPE_I32    8
#define FB_THRIFT_TYPE_I64    10
#define FB_THRIFT_TYPE_STRING 11
#define FB_THRIFT_TYPE_STRUCT 12
#define FB_THRIFT_TYPE_MAP    13
#define FB_THRIFT_TYPE_SET    14
#define FB_THRIFT_TYPE_LIST   15

class FbThrift
{
	MBinBuffer m_buf;

public:
	FbThrift& operator<<(uint8_t);
	FbThrift& operator<<(const char *);

	void writeBool(bool value);
	void writeBuf(const void *pData, size_t cbLen);
	void writeInt32(__int32 value);
	void writeInt64(__int64 value);
	void writeIntV(__int64 value);
	void writeField(int type, int id);
	void writeField(int type, int id, int lastid);
};

#define FB_MQTT_CONNECT_FLAG_CLR  0x0002
#define FB_MQTT_CONNECT_FLAG_WILL 0x0004
#define FB_MQTT_CONNECT_FLAG_QOS0 0x0000
#define FB_MQTT_CONNECT_FLAG_QOS1 0x0008
#define FB_MQTT_CONNECT_FLAG_QOS2 0x0010
#define FB_MQTT_CONNECT_FLAG_RET  0x0020
#define FB_MQTT_CONNECT_FLAG_PASS 0x0040
#define FB_MQTT_CONNECT_FLAG_USER 0x0080
