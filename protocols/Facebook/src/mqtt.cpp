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

#include "stdafx.h"

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
	deflate(&zStreamOut, Z_FINISH);
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
	int rc = inflate(&zStreamOut, Z_FINISH);
	inflateEnd(&zStreamOut);

	switch (rc) {
	case Z_OK:
	case Z_STREAM_END:
		cbRes = dataSize - zStreamOut.avail_out;
		return pRes;
	}

	mir_free(pRes);
	cbRes = 0;
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MqttMessage class members

MqttMessage::MqttMessage() :
	m_leadingByte(0)
{
}

MqttMessage::MqttMessage(FbMqttMessageType type, uint8_t flags)
{
	m_leadingByte = ((type & 0x0F) << 4) | (flags & 0x0F);
}

char* MqttMessage::readStr(const uint8_t *&pData) const
{
	u_short len = ntohs(*(u_short *)pData); pData += sizeof(u_short);
	if (len == 0)
		return nullptr;

	char *res = (char*)mir_alloc(len + 1);
	memcpy(res, pData, len);
	res[len] = 0;
	pData += len;
	return res;
}

void MqttMessage::writeStr(const char *str)
{
	size_t len = mir_strlen(str);
	writeInt16((uint16_t)len);
	writeBuf(str, len);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MQTT functions

bool FacebookProto::MqttParse(const MqttMessage &payload)
{
	auto *pData = (const uint8_t *)payload.data(), *pBeg = pData;
	int flags = payload.getFlags();
	uint16_t mid;

	switch (payload.getType()) {
	case FB_MQTT_MESSAGE_TYPE_CONNACK:
		if (pData[1] != 0) { // connection failed;
			int iErrorCode = ntohs(*(u_short *)pData);
			debugLogA("Login failed with error %d", iErrorCode);

			if (iErrorCode == 4) { // invalid login/password
				delSetting(DBKEY_TOKEN);
				m_szAuthToken.Empty();
				ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, 0, LOGINERR_WRONGPASSWORD);
			}
			else ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, 0, LOGINERR_WRONGPROTOCOL);
			return false;
		}

		OnLoggedIn();
		break;

	case FB_MQTT_MESSAGE_TYPE_PUBREL:
		mid = ntohs(*(u_short *)pData);
		pData += 2;
		{
			MqttMessage reply(FB_MQTT_MESSAGE_TYPE_PUBCOMP);
			reply.writeInt16(mid);
			MqttSend(reply);
		}
		break;

	case FB_MQTT_MESSAGE_TYPE_PUBLISH:
		char *str = payload.readStr(pData);

		if ((flags & FB_MQTT_MESSAGE_FLAG_QOS1) || (flags & FB_MQTT_MESSAGE_FLAG_QOS2)) {
			mid = ntohs(*(u_short *)pData);
			pData += 2;

			MqttMessage reply((flags & FB_MQTT_MESSAGE_FLAG_QOS1) ? FB_MQTT_MESSAGE_TYPE_PUBACK : FB_MQTT_MESSAGE_TYPE_PUBREC);
			reply.writeInt16(mid);
			MqttSend(reply);
		}

		OnPublish(str, pData, payload.size() - (pData - pBeg));
		mir_free(str);
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

void FacebookProto::MqttLogin()
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
	thrift.writeBool(!m_bLoginInvisible); // visibility

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
	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_PINGREQ, FB_MQTT_MESSAGE_FLAG_QOS1);
	MqttSend(payload);
}

void FacebookProto::MqttPublish(const char *topic, const JSONNode &value)
{
	auto str = value.write();
	debugLogA("Publish: <%s> -> <%s>", topic, str.c_str());

	size_t dataSize;
	mir_ptr<uint8_t> pData(doZip(str.length(), str.c_str(), dataSize));

	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_PUBLISH, FB_MQTT_MESSAGE_FLAG_QOS1);
	payload.writeStr(topic);
	payload.writeInt16(++m_mid);
	payload.writeBuf(pData, dataSize);
	MqttSend(payload);
}

void FacebookProto::MqttSubscribe(const char *topic, ...)
{
	uint8_t zeroByte = 0;

	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_SUBSCRIBE, FB_MQTT_MESSAGE_FLAG_QOS1);
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
	MqttMessage payload(FB_MQTT_MESSAGE_TYPE_UNSUBSCRIBE, FB_MQTT_MESSAGE_FLAG_QOS1);
	payload.writeInt16(++m_mid);
	payload.writeStr(topic);

	va_list ap;
	va_start(ap, topic);
	while ((topic = va_arg(ap, const char *)) != nullptr)
		payload.writeStr(topic);
	va_end(ap);

	MqttSend(payload);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MQTT queue

void FacebookProto::MqttQueueConnect()
{
	JSONNode query;
	query << INT_PARAM("delta_batch_size", 125) << INT_PARAM("max_deltas_able_to_process", 1000) << INT_PARAM("sync_api_version", 3) << CHAR_PARAM("encoding", "JSON");
	if (m_szSyncToken.IsEmpty()) {
		JSONNode hashes; hashes.set_name("graphql_query_hashes"); hashes << CHAR_PARAM("xma_query_id", __STRINGIFY(FB_API_QUERY_XMA));

		JSONNode xma; xma.set_name(__STRINGIFY(FB_API_QUERY_XMA)); xma << CHAR_PARAM("xma_id", "<ID>");
		JSONNode hql; hql.set_name("graphql_query_params"); hql << xma;

		JSONNode params; params.set_name("queue_params");
		params << CHAR_PARAM("buzz_on_deltas_enabled", "false") << hashes << hql;

		query << INT64_PARAM("initial_titan_sequence_id", m_sid) << CHAR_PARAM("device_id", m_szDeviceID) << INT64_PARAM("entity_fbid", m_uid) << params;
		MqttPublish("/messenger_sync_create_queue", query);
	}
	else {
		query << INT64_PARAM("last_seq_id", m_sid) << CHAR_PARAM("sync_token", m_szSyncToken);
		MqttPublish("/messenger_sync_get_diffs", query);
	}
}
