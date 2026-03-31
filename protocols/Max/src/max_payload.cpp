#include "stdafx.h"

extern "C" {
	#include "../../Tox/libtox/src/third_party/cmp/cmp.h"
	#include "../../Tox/libtox/src/third_party/cmp/cmp.c"
}

static bool cmp_reader_cb(cmp_ctx_t *ctx, void *data, size_t limit)
{
	MBinBuffer *buf = (MBinBuffer *)ctx->buf;
	if (buf->length() < limit)
		return false;

	memcpy(data, buf->data(), limit);
	buf->remove(limit);
	return true;
}

static size_t cmp_writer_cb(cmp_ctx_t *ctx, const void *data, size_t count)
{
	MBinBuffer *buf = (MBinBuffer *)ctx->buf;
	buf->append(data, count);
	return count;
}

bool CMaxProto::ReadExact(void *buf, int cbSize)
{
	char *p = (char *)buf;
	while (cbSize > 0) {
		int r = Netlib_Recv(m_hConnection, p, cbSize, MSG_NODUMP);
		if (r <= 0)
			return false;
		p += r;
		cbSize -= r;
	}
	return true;
}

bool CMaxProto::EncodeNode(cmp_ctx_t *ctx, const JSONNode &node) const
{
	switch (node.type()) {
	case JSON_NULL:
		return cmp_write_nil(ctx);
	case JSON_BOOL:
		return cmp_write_bool(ctx, node.as_bool());
	case JSON_NUMBER:
		{
			CMStringW value = node.as_mstring();
			if (value.Find('.') != -1) {
				return cmp_write_double(ctx, wcstod(value, nullptr));
			}
			return cmp_write_integer(ctx, _wtoi64(value));
		}
	case JSON_STRING:
		{
			ptrA szValue(mir_utf8encodeW(node.as_mstring()));
			return cmp_write_str(ctx, szValue, (uint32_t)mir_strlen(szValue));
		}
	case JSON_ARRAY:
		if (!cmp_write_array(ctx, (uint32_t)node.size()))
			return false;
		for (auto &it : node)
			if (!EncodeNode(ctx, it))
				return false;
		return true;
	case JSON_NODE:
		if (!cmp_write_map(ctx, (uint32_t)node.size()))
			return false;
		for (auto &it : node) {
			const char *szName = it.name();
			if (!cmp_write_str(ctx, szName, (uint32_t)mir_strlen(szName)))
				return false;
			if (!EncodeNode(ctx, it))
				return false;
		}
		return true;
	}
	return false;
}

bool CMaxProto::EncodePayload(const JSONNode &payload, MBinBuffer &out) const
{
	cmp_ctx_t ctx = {};
	cmp_init(&ctx, &out, nullptr, nullptr, cmp_writer_cb);
	return EncodeNode(&ctx, payload);
}

bool CMaxProto::DecodeNode(cmp_ctx_t *ctx, JSONNode &outNode) const
{
	cmp_object_t obj = {};
	if (!cmp_read_object(ctx, &obj))
		return false;

	switch (obj.type) {
	case CMP_TYPE_NIL:
		outNode = JSONNode(JSON_NULL);
		return true;
	case CMP_TYPE_BOOLEAN:
		outNode = JSONNode("", obj.as.boolean);
		return true;
	case CMP_TYPE_FLOAT:
		outNode = JSONNode("", obj.as.flt);
		return true;
	case CMP_TYPE_DOUBLE:
		outNode = JSONNode("", obj.as.dbl);
		return true;
	case CMP_TYPE_POSITIVE_FIXNUM:
	case CMP_TYPE_UINT8:
	case CMP_TYPE_UINT16:
	case CMP_TYPE_UINT32:
	case CMP_TYPE_UINT64:
		outNode = JSONNode("", (int64_t)obj.as.u64);
		return true;
	case CMP_TYPE_NEGATIVE_FIXNUM:
	case CMP_TYPE_SINT8:
	case CMP_TYPE_SINT16:
	case CMP_TYPE_SINT32:
	case CMP_TYPE_SINT64:
		outNode = JSONNode("", (int64_t)obj.as.s64);
		return true;
	case CMP_TYPE_FIXSTR:
	case CMP_TYPE_STR8:
	case CMP_TYPE_STR16:
	case CMP_TYPE_STR32:
		{
			uint32_t len = obj.as.str_size;
			ptrA tmp((char *)mir_alloc(len + 1));
			if (!cmp_object_to_str(ctx, &obj, tmp, len + 1)) {
				mir_free(tmp);
				return false;
			}
			tmp[len] = 0;
			outNode = JSONNode("", mir_utf8decodeW(tmp));
			return true;
		}
	case CMP_TYPE_FIXARRAY:
	case CMP_TYPE_ARRAY16:
	case CMP_TYPE_ARRAY32:
		{
			JSONNode arr(JSON_ARRAY);
			for (uint32_t i = 0; i < obj.as.array_size; ++i) {
				JSONNode child;
				if (!DecodeNode(ctx, child))
					return false;
				arr.push_back(child);
			}
			outNode = arr;
			return true;
		}
	case CMP_TYPE_FIXMAP:
	case CMP_TYPE_MAP16:
	case CMP_TYPE_MAP32:
		{
			JSONNode dict(JSON_NODE);
			for (uint32_t i = 0; i < obj.as.map_size; ++i) {
				JSONNode key, value;
				if (!DecodeNode(ctx, key) || !DecodeNode(ctx, value))
					return false;
				value.set_name(key.as_string().c_str());
				dict.push_back(value);
			}
			outNode = dict;
			return true;
		}
	default:
		return false;
	}
}

bool CMaxProto::DecodePayload(const uint8_t *data, size_t cbData, JSONNode &outPayload) const
{
	MBinBuffer buf;
	buf.append(data, cbData);
	cmp_ctx_t ctx = {};
	cmp_init(&ctx, &buf, cmp_reader_cb, nullptr, nullptr);
	return DecodeNode(&ctx, outPayload);
}

bool CMaxProto::SendFrame(const MaxFrame &frame)
{
	MBinBuffer payload;
	if (!EncodePayload(frame.payload, payload))
		return false;

	uint8_t header[10] = {};
	header[0] = frame.ver;
	header[1] = 0;
	header[2] = frame.cmd;
	header[3] = frame.seq;
	header[4] = (frame.opcode >> 8) & 0xFF;
	header[5] = frame.opcode & 0xFF;
	uint32_t len = (uint32_t)payload.length() & 0x00FFFFFF;
	header[6] = 0;
	header[7] = (len >> 16) & 0xFF;
	header[8] = (len >> 8) & 0xFF;
	header[9] = len & 0xFF;

	mir_cslock lck(m_csNet);
	Netlib_Logf(m_hNetlibUser, "Max: TX opcode=%u cmd=%u seq=%u payload=%u", frame.opcode, frame.cmd, frame.seq, (unsigned)payload.length());
	if (Netlib_Send(m_hConnection, (const char *)header, sizeof(header), MSG_NODUMP) != sizeof(header))
		return false;

	if (payload.length() > 0 && Netlib_Send(m_hConnection, (const char *)payload.data(), (int)payload.length(), MSG_NODUMP) != (int)payload.length())
		return false;

	return true;
}

bool CMaxProto::ReadFrame(MaxFrame &frame)
{
	uint8_t header[10] = {};
	if (!ReadExact(header, sizeof(header)))
		return false;

	frame.ver = header[0];
	frame.cmd = header[2];
	frame.seq = header[3];
	frame.opcode = (uint16_t(header[4]) << 8) | header[5];
	uint32_t packedLen = (uint32_t(header[6]) << 24) | (uint32_t(header[7]) << 16) | (uint32_t(header[8]) << 8) | header[9];
	uint32_t payloadLen = packedLen & 0x00FFFFFF;
	uint8_t compressed = (packedLen >> 24) & 0xFF;
	Netlib_Logf(m_hNetlibUser, "Max: RX header opcode=%u cmd=%u seq=%u payload=%u compressed=%u", frame.opcode, frame.cmd, frame.seq, payloadLen, compressed);

	MBinBuffer payload;
	if (payloadLen > 0) {
		std::vector<uint8_t> tmp(payloadLen);
		if (!ReadExact(tmp.data(), (int)payloadLen))
			return false;
		payload.append(tmp.data(), payloadLen);
	}

	JSONNode json(JSON_NODE);
	if (payloadLen > 0) {
		if (!DecodePayload((const uint8_t *)payload.data(), payloadLen, json)) {
			// Mobile API may return compressed payloads (LZ4). For MVP, keep protocol alive even if payload decode failed.
			Netlib_Logf(m_hNetlibUser, "Max: payload decode failed for opcode=%u seq=%u (compressed=%u), using empty payload", frame.opcode, frame.seq, compressed);
			json = JSONNode(JSON_NODE);
		}
	}

	frame.payload = json;
	return true;
}
