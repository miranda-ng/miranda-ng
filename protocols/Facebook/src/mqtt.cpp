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

uint8_t *FacebookProto::doZip(size_t cbData, const void *pData, size_t &cbRes)
{
	size_t dataSize = cbData + 100;
	uint8_t *pRes = (uint8_t *)mir_alloc(dataSize);

	z_stream zStreamOut = {};
	deflateInit(&zStreamOut, Z_BEST_COMPRESSION);
	zStreamOut.avail_in = (unsigned)cbData;
	zStreamOut.next_in = (uint8_t *)pData;
	zStreamOut.avail_out = (unsigned)dataSize;
	zStreamOut.next_out = (uint8_t *)pRes;

	switch (deflate(&zStreamOut, Z_FINISH)) {
	case Z_STREAM_END: debugLogA("Deflate: Z_STREAM_END"); break;
	case Z_OK:         debugLogA("Deflate: Z_OK");         break;
	case Z_BUF_ERROR:  debugLogA("Deflate: Z_BUF_ERROR");  break;
	case Z_DATA_ERROR: debugLogA("Deflate: Z_DATA_ERROR"); break;
	case Z_MEM_ERROR:  debugLogA("Deflate: Z_MEM_ERROR");  break;
	}

	deflateEnd(&zStreamOut);
	cbRes = dataSize - zStreamOut.avail_out;
	return pRes;
}

uint8_t *FacebookProto::doUnzip(size_t cbData, const void *pData, size_t &cbRes)
{
	size_t dataSize = cbData * 10;
	uint8_t *pRes = (uint8_t *)mir_alloc(dataSize);

	z_stream zStreamOut = {};
	inflateInit(&zStreamOut);
	zStreamOut.avail_in = (unsigned)cbData;
	zStreamOut.next_in = (uint8_t *)pData;
	zStreamOut.avail_out = (unsigned)dataSize;
	zStreamOut.next_out = (uint8_t *)pRes;

	switch (inflate(&zStreamOut, Z_FINISH)) {
	case Z_STREAM_END: debugLogA("Deflate: Z_STREAM_END"); break;
	case Z_OK:         debugLogA("Deflate: Z_OK");         break;
	case Z_BUF_ERROR:  debugLogA("Deflate: Z_BUF_ERROR");  break;
	case Z_DATA_ERROR: debugLogA("Deflate: Z_DATA_ERROR"); break;
	case Z_MEM_ERROR:  debugLogA("Deflate: Z_MEM_ERROR");  break;
	}

	inflateEnd(&zStreamOut);
	cbRes = dataSize - zStreamOut.avail_out;
	return pRes;
}

/////////////////////////////////////////////////////////////////////////////////////////

FbThrift& FbThrift::operator<<(uint8_t value)
{
	m_buf.append(&value, 1);
	return *this;
}

FbThrift& FbThrift::operator<<(const char *str)
{
	size_t len = mir_strlen(str);
	writeIntV(len);
	m_buf.append((void*)str, len);
	return *this;
}

void FbThrift::writeBool(bool bValue)
{
	uint8_t b = (bValue) ? 0x11 : 0x12;
	m_buf.append(&b, 1);
}

void FbThrift::writeBuf(const void *pData, size_t cbLen)
{
	m_buf.append((void*)pData, cbLen);
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

MqttMessage::MqttMessage() :
	m_leadingByte(0)
{
}

MqttMessage::MqttMessage(FbMqttMessageType type, uint8_t flags)
{
	m_leadingByte = ((type & 0x0F) << 4) | (flags & 0x0F);
}

void MqttMessage::writeStr(const char *str)
{
	size_t len = mir_strlen(str);
	writeInt16((int)len);
	writeBuf(str, len);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MQTT functions

bool FacebookProto::MqttConnect()
{
	NETLIBOPENCONNECTION nloc = {};
	nloc.szHost = "mqtt.facebook.com";
	nloc.wPort = 443;
	nloc.flags = NLOCF_SSL | NLOCF_V2;
	m_mqttConn = Netlib_OpenConnection(m_hNetlibUser, &nloc);
	if (m_mqttConn == nullptr) {
		debugLogA("connection failed, exiting");
		return false;
	}

	return true;
}

bool FacebookProto::MqttParse(const MqttMessage &payload)
{
	auto *pData = (const uint8_t *)payload.data();

	switch (payload.getType()) {
	case FB_MQTT_MESSAGE_TYPE_CONNACK:
		if (pData[1] != 0) { // connection failed;
			ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)m_iStatus, m_iDesiredStatus);
			return false;
		}

		OnLoggedIn();
		MqttPing();
		break;
	}

	return true;
}

bool FacebookProto::MqttRead(MqttMessage &payload)
{
	uint8_t b;
	int res = Netlib_Recv(m_mqttConn, (char *)&b, sizeof(b), MSG_NODUMP);
	if (res != 1)
		return false;

	payload.m_leadingByte = b;

	uint32_t m = 1, remainingBytes = 0;
	do {
		if ((res = Netlib_Recv(m_mqttConn, (char *)&b, sizeof(b), MSG_NODUMP)) != 1)
			return false;

		remainingBytes += (b & 0x7F) * m;
		m *= 128;
	} while ((b & 0x80) != 0);

	debugLogA("Received message of type=%d, flags=%x, body length=%d", payload.getType(), payload.getFlags(), remainingBytes);

	if (remainingBytes != 0) {
		while (remainingBytes > 0) {
			uint8_t buf[1024];
			int size = min(remainingBytes, sizeof(buf));
			if ((res = Netlib_Recv(m_mqttConn, (char *)buf, size)) <= 0)
				return false;

			payload.writeBuf(buf, res);
			remainingBytes -= res;
		}

		// that might be a zipped buffer
		if (payload.size() >= 2) {
			auto *p = (const uint8_t *)payload.data();
			if ((((p[0] << 8) | p[1]) % 31) == 0 && (p[0] & 0x0F) == 8) { // zip header ok
				size_t dataSize;
				void *pData = doUnzip(payload.size(), payload.data(), dataSize);
				if (pData != nullptr) {
					payload.reset(dataSize, pData);
					mir_free(pData);
				}
			}
		}
	}

	return true;
}

void FacebookProto::MqttSend(const MqttMessage &payload)
{
	FbThrift msg;
	msg << payload.m_leadingByte;
	msg.writeIntV(payload.size());
	msg.writeBuf(payload.data(), payload.size());
	Netlib_Send(m_mqttConn, (char*)msg.data(), (unsigned)msg.size());
}

/////////////////////////////////////////////////////////////////////////////////////////
// creates initial MQTT will and sends initialization packet

void FacebookProto::MqttOpen()
{
	uint8_t zeroByte = 0;
	Utils_GetRandom(&m_iMqttId, sizeof(m_iMqttId) / 2);

	FbThrift thrift;
	thrift.writeField(FB_THRIFT_TYPE_STRING);  // Client identifier
	thrift << m_szClientID;

	thrift.writeField(FB_THRIFT_TYPE_STRUCT, 4, 1);

	thrift.writeField(FB_THRIFT_TYPE_I64); // User identifier
	thrift.writeInt64(m_uid);

	thrift.writeField(FB_THRIFT_TYPE_STRING); // User agent
	thrift << FB_API_MQTT_AGENT;

	thrift.writeField(FB_THRIFT_TYPE_I64);
	thrift.writeInt64(23);
	thrift.writeField(FB_THRIFT_TYPE_I64);
	thrift.writeInt64(26);
	thrift.writeField(FB_THRIFT_TYPE_I32);
	thrift.writeInt32(1);

	thrift.writeBool(true);
	thrift.writeBool(m_iStatus != ID_STATUS_INVISIBLE); // visibility

	thrift.writeField(FB_THRIFT_TYPE_STRING); // device id
	thrift << m_szDeviceID;

	thrift.writeBool(true);
	thrift.writeField(FB_THRIFT_TYPE_I32);
	thrift.writeInt32(1);
	thrift.writeField(FB_THRIFT_TYPE_I32);
	thrift.writeInt32(0);
	thrift.writeField(FB_THRIFT_TYPE_I64);
	thrift.writeInt64(m_iMqttId);

	thrift.writeField(FB_THRIFT_TYPE_LIST, 14, 12);
	thrift.writeList(FB_THRIFT_TYPE_I32, 0);
	thrift << zeroByte;

	thrift.writeField(FB_THRIFT_TYPE_STRING);
	thrift << m_szAuthToken << zeroByte;

	size_t dataSize;
	mir_ptr<uint8_t> pData(doZip(thrift.size(), thrift.data(), dataSize));

	uint8_t protocolVersion = 3;
	uint8_t flags = FB_MQTT_CONNECT_FLAG_USER | FB_MQTT_CONNECT_FLAG_PASS | FB_MQTT_CONNECT_FLAG_CLR | FB_MQTT_CONNECT_FLAG_QOS1;
	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_CONNECT);
	payload.writeStr("MQTToT");
	payload << protocolVersion << flags;
	payload.writeInt16(60); // timeout
	payload.writeBuf(pData, dataSize);
	MqttSend(payload);
}

/////////////////////////////////////////////////////////////////////////////////////////
// various MQTT send commands

void FacebookProto::MqttPing()
{
	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_PINGREQ);
	MqttSend(payload);
}

void FacebookProto::MqttPublish(const char *topic, const char *value)
{
	debugLogA("Publish: <%s> -> <%s>", topic, value);

	size_t dataSize;
	mir_ptr<uint8_t> pData(doZip(strlen(value), value, dataSize));

	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_PUBLISH, FB_MQTT_CONNECT_FLAG_QOS1);
	payload.writeStr(topic);
	payload.writeInt16(++m_mid);
	payload.writeBuf(pData, dataSize);
	MqttSend(payload);
}

void FacebookProto::MqttSubscribe(const char *topic, ...)
{
	uint8_t zeroByte = 0;

	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_SUBSCRIBE, FB_MQTT_CONNECT_FLAG_QOS1);
	payload.writeInt16(++m_mid);
	payload.writeStr(topic);
	payload << zeroByte;

	va_list ap;
	va_start(ap, topic);
	while ((topic = va_arg(ap, const char *)) != nullptr) {
		payload.writeStr(topic);
		payload << zeroByte;
	}
	va_end(ap);

	MqttSend(payload);
}

void FacebookProto::MqttUnsubscribe(const char *topic, ...)
{
	uint8_t zeroByte = 0;

	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_UNSUBSCRIBE, FB_MQTT_CONNECT_FLAG_QOS1);
	payload.writeInt16(++m_mid);
	payload.writeStr(topic);
	payload << zeroByte;

	va_list ap;
	va_start(ap, topic);
	while ((topic = va_arg(ap, const char *)) != nullptr) {
		payload.writeStr(topic);
		payload << zeroByte;
	}
	va_end(ap);

	MqttSend(payload);
}
