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

#pragma once

#define FACEBOOK_ORCA_AGENT FB_API_MQTT_AGENT

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

enum FbMqttMessageType
{
	FB_MQTT_MESSAGE_TYPE_CONNECT = 1,
	FB_MQTT_MESSAGE_TYPE_CONNACK = 2,
	FB_MQTT_MESSAGE_TYPE_PUBLISH = 3,
	FB_MQTT_MESSAGE_TYPE_PUBACK = 4,
	FB_MQTT_MESSAGE_TYPE_PUBREC = 5,
	FB_MQTT_MESSAGE_TYPE_PUBREL = 6,
	FB_MQTT_MESSAGE_TYPE_PUBCOMP = 7,
	FB_MQTT_MESSAGE_TYPE_SUBSCRIBE = 8,
	FB_MQTT_MESSAGE_TYPE_SUBACK = 9,
	FB_MQTT_MESSAGE_TYPE_UNSUBSCRIBE = 10,
	FB_MQTT_MESSAGE_TYPE_UNSUBACK = 11,
	FB_MQTT_MESSAGE_TYPE_PINGREQ = 12,
	FB_MQTT_MESSAGE_TYPE_PINGRESP = 13,
	FB_MQTT_MESSAGE_TYPE_DISCONNECT = 14
};

class FbThrift
{
	MBinBuffer m_buf;

public:
	__forceinline void* data() const { return m_buf.data(); }
	__forceinline size_t size() const { return m_buf.length(); }

	__forceinline void reset(size_t cbLen, void *pData)
	{	m_buf.assign(pData, cbLen);
	}

	FbThrift& operator<<(uint8_t);
	FbThrift& operator<<(const char *);

	void writeBool(bool value);
	void writeBuf(const void *pData, size_t cbLen);
	void writeInt16(uint16_t value);
	void writeInt32(int32_t value);
	void writeInt64(int64_t value);
	void writeIntV(uint64_t value);
	void writeField(int type);
	void writeField(int type, int id, int lastid);
	void writeList(int iType, int size);
};

class FbThriftReader : public FbThrift
{
	bool m_lastBool = false, m_lastBval = false;
	size_t offset = 0;

	uint8_t decodeType(int type);

public:
	__forceinline CMStringA rest() const { return CMStringA((char*)data() + offset, int(size() - offset)); }

	bool isStop();
	bool readBool(bool &bVal);
	bool readByte(uint8_t &val);
	bool readField(uint8_t &type, uint16_t &id);
	bool readInt16(uint16_t &val);
	bool readInt32(uint32_t &val);
	bool readInt64(uint64_t &val);
	bool readIntV(uint64_t &val);
	bool readList(uint8_t &type, uint32_t &size);
	bool readStr(char *&val); // val must be freed via mir_free()
};

class MqttMessage : public FbThrift
{
	friend class FacebookProto;

	uint8_t m_leadingByte;

public:
	MqttMessage();
	MqttMessage(FbMqttMessageType type, uint8_t flags = 0);

	__forceinline int getType() const
	{
		return m_leadingByte >> 4;
	}

	__forceinline int getFlags() const
	{	return m_leadingByte & 0x0F;
	}

	char* readStr(const uint8_t *&pData) const;
	void writeStr(const char *str);
};

#define FB_MQTT_CONNECT_FLAG_CLR  0x0002
#define FB_MQTT_CONNECT_FLAG_WILL 0x0004
#define FB_MQTT_CONNECT_FLAG_QOS0 0x0000
#define FB_MQTT_CONNECT_FLAG_QOS1 0x0008
#define FB_MQTT_CONNECT_FLAG_QOS2 0x0010
#define FB_MQTT_CONNECT_FLAG_RET  0x0020
#define FB_MQTT_CONNECT_FLAG_PASS 0x0040
#define FB_MQTT_CONNECT_FLAG_USER 0x0080

#define FB_MQTT_MESSAGE_FLAG_QOS0 0x0000
#define FB_MQTT_MESSAGE_FLAG_QOS1 0x0002
#define FB_MQTT_MESSAGE_FLAG_QOS2 0x0004
