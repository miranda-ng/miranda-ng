#include "stdafx.h"

namespace
{
struct MpValue
{
	enum Type { NIL, BOOL, INT, STR, MAP, ARR } type = NIL;
	bool b = false;
	int64_t i = 0;
	CMStringA s;
	std::map<CMStringA, MpValue> m;
	std::vector<MpValue> a;
};

static void be16(MBinBuffer &b, uint16_t v) { uint8_t x[2] = { uint8_t(v >> 8), uint8_t(v) }; b.append(x, 2); }
static void be32(MBinBuffer &b, uint32_t v) { uint8_t x[4] = { uint8_t(v >> 24), uint8_t(v >> 16), uint8_t(v >> 8), uint8_t(v) }; b.append(x, 4); }
static uint16_t rd16(const uint8_t *p) { return (uint16_t(p[0]) << 8) | p[1]; }
static uint32_t rd32(const uint8_t *p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]; }

static bool recvExact(HNETLIBCONN h, void *p, int cb, int &lastResult, int &lastErr)
{
	uint8_t *dst = (uint8_t*)p;
	lastResult = 0;
	lastErr = 0;
	while (cb > 0) {
		int r = Netlib_Recv(h, (char*)dst, cb, MSG_NODUMP);
		if (r <= 0) {
			lastResult = r;
			lastErr = GetLastError();
			return false;
		}
		dst += r;
		cb -= r;
	}
	return true;
}

static bool lz4DecompressRaw(const uint8_t *src, size_t srcLen, std::vector<uint8_t> &out, size_t maxOut, CMStringW &errorText)
{
	out.clear();
	size_t ip = 0;
	while (ip < srcLen) {
		uint8_t token = src[ip++];
		size_t litLen = token >> 4;
		if (litLen == 15) {
			while (ip < srcLen) {
				uint8_t s = src[ip++];
				litLen += s;
				if (s != 255) break;
			}
		}
		if (ip + litLen > srcLen) {
			errorText = L"LZ4 literal overflow";
			return false;
		}
		if (out.size() + litLen > maxOut) {
			errorText = L"LZ4 output too large";
			return false;
		}
		out.insert(out.end(), src + ip, src + ip + litLen);
		ip += litLen;
		if (ip >= srcLen)
			break; // last literals-only sequence

		if (ip + 2 > srcLen) {
			errorText = L"LZ4 missing match offset";
			return false;
		}
		size_t offset = src[ip] | (size_t(src[ip + 1]) << 8);
		ip += 2;
		if (offset == 0 || offset > out.size()) {
			errorText = L"LZ4 invalid match offset";
			return false;
		}

		size_t matchLen = (token & 0x0F) + 4;
		if ((token & 0x0F) == 15) {
			while (ip < srcLen) {
				uint8_t s = src[ip++];
				matchLen += s;
				if (s != 255) break;
			}
		}
		if (out.size() + matchLen > maxOut) {
			errorText = L"LZ4 output too large";
			return false;
		}

		size_t start = out.size() - offset;
		for (size_t i = 0; i < matchLen; ++i)
			out.push_back(out[start + i]);
	}
	return true;
}

static bool sendExact(HNETLIBCONN h, const void *p, int cb)
{
	const uint8_t *src = (const uint8_t*)p;
	while (cb > 0) {
		int r = Netlib_Send(h, (const char*)src, cb, MSG_NODUMP);
		if (r <= 0)
			return false;
		src += r;
		cb -= r;
	}
	return true;
}

static void mpPutString(MBinBuffer &out, const char *s)
{
	size_t n = strlen(s);
	if (n <= 31) { uint8_t t = uint8_t(0xA0 | n); out.append(&t, 1); }
	else if (n <= 255) { uint8_t t[2] = { 0xD9, uint8_t(n) }; out.append(t, 2); }
	else { uint8_t t = 0xDA; out.append(&t, 1); be16(out, (uint16_t)n); }
	out.append(s, n);
}

static void mpPutString(MBinBuffer &out, const CMStringA &s) { mpPutString(out, s.c_str()); }
static void mpPutInt(MBinBuffer &out, int64_t v)
{
	if (v >= 0) {
		if (v <= 127) {
			uint8_t t = uint8_t(v); out.append(&t, 1); return;
		}
		if (v <= 0xFF) {
			uint8_t t[2] = { 0xCC, uint8_t(v) }; out.append(t, 2); return;
		}
		if (v <= 0xFFFF) {
			uint8_t t = 0xCD; out.append(&t, 1); be16(out, (uint16_t)v); return;
		}
		if (v <= 0xFFFFFFFF) {
			uint8_t t = 0xCE; out.append(&t, 1); be32(out, (uint32_t)v); return;
		}
		uint8_t t = 0xCF; out.append(&t, 1);
		uint8_t b[8] = { uint8_t(v >> 56), uint8_t(v >> 48), uint8_t(v >> 40), uint8_t(v >> 32), uint8_t(v >> 24), uint8_t(v >> 16), uint8_t(v >> 8), uint8_t(v) };
		out.append(b, 8);
		return;
	}

	if (v >= -32) {
		uint8_t t = uint8_t((int8_t)v); out.append(&t, 1); return;
	}
	if (v >= INT8_MIN) {
		uint8_t t[2] = { 0xD0, uint8_t((int8_t)v) }; out.append(t, 2); return;
	}
	if (v >= INT16_MIN) {
		uint8_t t = 0xD1; out.append(&t, 1); be16(out, (uint16_t)(int16_t)v); return;
	}
	if (v >= INT32_MIN) {
		uint8_t t = 0xD2; out.append(&t, 1); be32(out, (uint32_t)(int32_t)v); return;
	}
	uint8_t t = 0xD3; out.append(&t, 1);
	uint8_t b[8] = { uint8_t(v >> 56), uint8_t(v >> 48), uint8_t(v >> 40), uint8_t(v >> 32), uint8_t(v >> 24), uint8_t(v >> 16), uint8_t(v >> 8), uint8_t(v) };
	out.append(b, 8);
}

static void mpMapStart(MBinBuffer &out, uint32_t n)
{
	if (n <= 15) { uint8_t t = uint8_t(0x80 | n); out.append(&t, 1); }
	else { uint8_t t = 0xDE; out.append(&t, 1); be16(out, (uint16_t)n); }
}

static bool mpRead(const uint8_t *&p, const uint8_t *e, MpValue &out);

static bool mpReadStr(const uint8_t *&p, const uint8_t *e, uint32_t n, MpValue &out)
{
	if ((size_t)(e - p) < n) return false;
	out.type = MpValue::STR;
	out.s.Append((const char*)p, (int)n);
	p += n;
	return true;
}

static bool mpReadMap(const uint8_t *&p, const uint8_t *e, uint32_t n, MpValue &out)
{
	out.type = MpValue::MAP;
	for (uint32_t i = 0; i < n; ++i) {
		MpValue k, v;
		if (!mpRead(p, e, k) || !mpRead(p, e, v)) return false;
		CMStringA ks;
		if (k.type == MpValue::STR) ks = k.s;
		else if (k.type == MpValue::INT) ks.Format("%lld", (long long)k.i);
		else ks = "key";
		out.m[ks] = v;
	}
	return true;
}

static bool mpReadArr(const uint8_t *&p, const uint8_t *e, uint32_t n, MpValue &out)
{
	out.type = MpValue::ARR;
	out.a.reserve(n);
	for (uint32_t i = 0; i < n; ++i) {
		MpValue v;
		if (!mpRead(p, e, v)) return false;
		out.a.push_back(v);
	}
	return true;
}

static bool mpRead(const uint8_t *&p, const uint8_t *e, MpValue &out)
{
	if (p >= e) return false;
	uint8_t t = *p++;

	if (t <= 0x7F) { out.type = MpValue::INT; out.i = t; return true; }
	if (t >= 0xE0) { out.type = MpValue::INT; out.i = (int8_t)t; return true; }
	if ((t & 0xE0) == 0xA0) return mpReadStr(p, e, t & 0x1F, out);
	if ((t & 0xF0) == 0x80) return mpReadMap(p, e, t & 0x0F, out);
	if ((t & 0xF0) == 0x90) return mpReadArr(p, e, t & 0x0F, out);

	switch (t) {
	case 0xC0: out.type = MpValue::NIL; return true;
	case 0xC2: out.type = MpValue::BOOL; out.b = false; return true;
	case 0xC3: out.type = MpValue::BOOL; out.b = true; return true;
	case 0xCC: if (e - p < 1) return false; out.type = MpValue::INT; out.i = *p++; return true;
	case 0xCD: if (e - p < 2) return false; out.type = MpValue::INT; out.i = rd16(p); p += 2; return true;
	case 0xCE: if (e - p < 4) return false; out.type = MpValue::INT; out.i = rd32(p); p += 4; return true;
	case 0xD0: if (e - p < 1) return false; out.type = MpValue::INT; out.i = (int8_t)*p++; return true;
	case 0xD1: if (e - p < 2) return false; out.type = MpValue::INT; out.i = (int16_t)rd16(p); p += 2; return true;
	case 0xD2: if (e - p < 4) return false; out.type = MpValue::INT; out.i = (int32_t)rd32(p); p += 4; return true;
	case 0xD3:
		if (e - p < 8) return false;
		out.type = MpValue::INT;
		out.i = (int64_t(p[0]) << 56) | (int64_t(p[1]) << 48) | (int64_t(p[2]) << 40) | (int64_t(p[3]) << 32) |
			(int64_t(p[4]) << 24) | (int64_t(p[5]) << 16) | (int64_t(p[6]) << 8) | int64_t(p[7]);
		p += 8; return true;
	case 0xD9: if (e - p < 1) return false; { uint32_t n = *p++; return mpReadStr(p, e, n, out); }
	case 0xDA: if (e - p < 2) return false; { uint32_t n = rd16(p); p += 2; return mpReadStr(p, e, n, out); }
	case 0xDB: if (e - p < 4) return false; { uint32_t n = rd32(p); p += 4; return mpReadStr(p, e, n, out); }
	case 0xDC: if (e - p < 2) return false; { uint32_t n = rd16(p); p += 2; return mpReadArr(p, e, n, out); }
	case 0xDD: if (e - p < 4) return false; { uint32_t n = rd32(p); p += 4; return mpReadArr(p, e, n, out); }
	case 0xDE: if (e - p < 2) return false; { uint32_t n = rd16(p); p += 2; return mpReadMap(p, e, n, out); }
	case 0xDF: if (e - p < 4) return false; { uint32_t n = rd32(p); p += 4; return mpReadMap(p, e, n, out); }
	default: return false;
	}
}

static const MpValue* mpGet(const MpValue &m, const char *k)
{
	if (m.type != MpValue::MAP) return nullptr;
	auto it = m.m.find(k);
	return it == m.m.end() ? nullptr : &it->second;
}

static void ensureMobileIds(CMaxProto *p, CMStringA &deviceId, CMStringA &mtId)
{
	deviceId = p->getMStringA(DB_KEY_MOBILE_DEVICEID);
	mtId = p->getMStringA(DB_KEY_MTINSTANCE);
	if (!deviceId.IsEmpty() && !mtId.IsEmpty())
		return;

	BYTE b[16];
	Utils_GetRandom(b, sizeof(b));
	char noDash[33];
	for (int i = 0; i < 16; ++i)
		mir_snprintf(noDash + i * 2, 3, "%02x", b[i]);
	noDash[32] = 0;
	deviceId = noDash;

	Utils_GetRandom(b, sizeof(b));
	b[6] = (b[6] & 0x0f) | 0x40;
	b[8] = (b[8] & 0x3f) | 0x80;
	mtId.Format("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
	p->setString(DB_KEY_MOBILE_DEVICEID, deviceId);
	p->setString(DB_KEY_MTINSTANCE, mtId);
}

static bool mobileSend(HNETLIBCONN h, uint8_t ver, uint8_t cmd, uint8_t seq, uint16_t opcode, const MBinBuffer &payload)
{
	MBinBuffer out;
	out.append(&ver, 1);
	be16(out, cmd);
	out.append(&seq, 1);
	be16(out, opcode);
	uint32_t len = (uint32_t)payload.length() & 0x00FFFFFF;
	be32(out, len);
	out.append(payload.data(), payload.length());
	return sendExact(h, out.data(), (int)out.length());
}

static bool mobileRecv(HNETLIBCONN h, uint8_t &seq, uint16_t &opcode, MBinBuffer &payload, CMStringW &errorText)
{
	uint8_t hdr[10];
	int last = 0, err = 0;
	if (!recvExact(h, hdr, 10, last, err)) {
		errorText.Format(L"mobile recv header failed (recv=%d, err=%d)", last, err);
		return false;
	}

	seq = hdr[3];
	opcode = rd16(hdr + 4);
	uint32_t raw = rd32(hdr + 6);
	uint8_t comp = (raw >> 24) & 0xFF;
	uint32_t len = raw & 0x00FFFFFF;
	payload = MBinBuffer();
	if (len == 0)
		return true;
	std::vector<uint8_t> tmp(len);
	if (!recvExact(h, tmp.data(), (int)len, last, err)) {
		errorText.Format(L"mobile recv payload failed (recv=%d, err=%d, len=%u)", last, err, len);
		return false;
	}

	if (comp != 0) {
		std::vector<uint8_t> dec;
		if (!lz4DecompressRaw(tmp.data(), tmp.size(), dec, 5 * 1024 * 1024, errorText))
			return false;
		payload.assign(dec.data(), dec.size());
	}
	else {
		payload.assign(tmp.data(), len);
	}
	return true;
}

static MBinBuffer buildHandshakePayload(const CMStringA &deviceId, const CMStringA &mtId)
{
	MBinBuffer p;
	mpMapStart(p, 4);
	mpPutString(p, "clientSessionId"); mpPutInt(p, 1);
	mpPutString(p, "mt_instanceid"); mpPutString(p, mtId);
	mpPutString(p, "userAgent");
	mpMapStart(p, 10);
	mpPutString(p, "deviceType"); mpPutString(p, "ANDROID");
	mpPutString(p, "appVersion"); mpPutString(p, "25.10.0");
	mpPutString(p, "osVersion"); mpPutString(p, "Android 13");
	mpPutString(p, "timezone"); mpPutString(p, "GMT");
	mpPutString(p, "screen"); mpPutString(p, "130dpi 130dpi 600x874");
	mpPutString(p, "pushDeviceType"); mpPutString(p, "GCM");
	mpPutString(p, "locale"); mpPutString(p, "ru");
	mpPutString(p, "buildNumber"); mpPutInt(p, 6401);
	mpPutString(p, "deviceName"); mpPutString(p, "unknown Generic Android-x86_64");
	mpPutString(p, "deviceLocale"); mpPutString(p, "ru");
	// 4th top-level pair of handshake payload
	// (must be outside userAgent map)
	mpPutString(p, "deviceId"); mpPutString(p, deviceId);
	return p;
}

static bool mobileRequestAuth(CMaxProto *p, uint16_t opcode, const MBinBuffer &requestPayload, MpValue &payloadOut, CMStringW &errorText)
{
	HNETLIBCONN h = Netlib_OpenConnection(p->m_hNetlibUser, "api.oneme.ru", 443, 10000, 0);
	if (h == nullptr) {
		errorText = L"cannot connect mobile API";
		return false;
	}
	if (!Netlib_StartSsl(h, "api.oneme.ru")) {
		Netlib_CloseHandle(h);
		errorText = L"mobile TLS negotiation failed";
		return false;
	}

	uint8_t seq = 1;
	CMStringA dev, mt;
	ensureMobileIds(p, dev, mt);
	MBinBuffer hs = buildHandshakePayload(dev, mt);
	if (!mobileSend(h, 11, 0, seq++, 6, hs)) {
		Netlib_CloseHandle(h);
		errorText = L"mobile handshake send failed";
		return false;
	}

	MBinBuffer respPayload;
	uint8_t rseq; uint16_t rop;
	if (!mobileRecv(h, rseq, rop, respPayload, errorText)) { Netlib_CloseHandle(h); return false; }
	if (rop != 6) { Netlib_CloseHandle(h); errorText = L"mobile handshake response mismatch"; return false; }

	if (!mobileSend(h, 11, 0, seq++, opcode, requestPayload)) {
		Netlib_CloseHandle(h);
		errorText = L"mobile request send failed";
		return false;
	}

	if (!mobileRecv(h, rseq, rop, respPayload, errorText)) { Netlib_CloseHandle(h); return false; }
	Netlib_CloseHandle(h);

	const uint8_t *cur = (const uint8_t*)respPayload.data();
	const uint8_t *end = cur + respPayload.length();
	MpValue top;
	if (!mpRead(cur, end, top) || top.type != MpValue::MAP) {
		errorText = L"mobile msgpack parse failed";
		return false;
	}

	const MpValue *pl = mpGet(top, "payload");
	// Some responses return fields directly in top-level map.
	if (pl != nullptr && pl->type == MpValue::MAP)
		payloadOut = *pl;
	else
		payloadOut = top;
	return true;
}
} // namespace

static CMStringW FormatMobileApiError(const CMStringA &codeUtf8)
{
	if (codeUtf8.IsEmpty())
		return L"";
	if (!mir_strcmp(codeUtf8.c_str(), "error.limit.violate"))
		return TranslateT("Too many SMS/code requests. Please wait before trying again.");
	if (!mir_strcmp(codeUtf8.c_str(), "login.cred"))
		return TranslateT("Server rejected web sync credentials for this token (login.cred).");
	ptrW w(mir_utf8decodeW(codeUtf8.c_str()));
	return CMStringW((const wchar_t *)w);
}

bool CMaxProto::MobileStartAuth(const char *phone, CMStringW &errorText)
{
	errorText.Empty();
	MBinBuffer req;
	mpMapStart(req, 3);
	mpPutString(req, "phone"); mpPutString(req, phone);
	mpPutString(req, "type"); mpPutString(req, "START_AUTH");
	mpPutString(req, "language"); mpPutString(req, "ru");

	MpValue payload;
	if (!mobileRequestAuth(this, 17, req, payload, errorText))
		return false;

	if (const MpValue *err = mpGet(payload, "error")) {
		if (err->type == MpValue::STR)
			errorText = FormatMobileApiError(err->s);
		if (errorText.IsEmpty()) errorText = TranslateT("Auth request was rejected.");
		return false;
	}

	const MpValue *token = mpGet(payload, "token");
	if (token == nullptr || token->type != MpValue::STR || token->s.IsEmpty()) {
		errorText = L"temporary token missing";
		return false;
	}
	setString(DB_KEY_TOKEN_TMP, token->s.c_str());
	return true;
}

bool CMaxProto::MobileCheckCode(const char *code, CMStringA &loginToken, CMStringW &errorText)
{
	loginToken.Empty();
	errorText.Empty();
	CMStringA tmp = getMStringA(DB_KEY_TOKEN_TMP);
	if (tmp.IsEmpty()) {
		errorText = L"temporary token not set";
		return false;
	}

	MBinBuffer req;
	mpMapStart(req, 3);
	mpPutString(req, "token"); mpPutString(req, tmp);
	mpPutString(req, "verifyCode"); mpPutString(req, code);
	mpPutString(req, "authTokenType"); mpPutString(req, "CHECK_CODE");

	MpValue payload;
	if (!mobileRequestAuth(this, 18, req, payload, errorText))
		return false;

	if (const MpValue *err = mpGet(payload, "error")) {
		if (err->type == MpValue::STR)
			errorText = FormatMobileApiError(err->s);
		if (errorText.IsEmpty()) errorText = TranslateT("Wrong code or server rejected verification.");
		return false;
	}

	const MpValue *attrs = mpGet(payload, "tokenAttrs");
	if (attrs == nullptr || attrs->type != MpValue::MAP) {
		errorText = L"tokenAttrs missing";
		return false;
	}

	CMStringA attrNames;
	for (const auto &kv : attrs->m) {
		if (!attrNames.IsEmpty())
			attrNames += ",";
		attrNames += kv.first;
	}
	debugLogA("Max: mobile tokenAttrs keys=%s", attrNames.c_str());

	auto tryAttrToken = [&](const char *name, CMStringA &out) -> bool {
		auto it = attrs->m.find(name);
		if (it == attrs->m.end() || it->second.type != MpValue::MAP)
			return false;
		const MpValue *tok = mpGet(it->second, "token");
		if (tok == nullptr || tok->type != MpValue::STR || tok->s.IsEmpty())
			return false;
		out = tok->s;
		return true;
	};

	// Prefer WEB token for websocket sync, fallback to LOGIN.
	if (tryAttrToken("WEB", loginToken)) {
		debugLogA("Max: selected token type WEB");
		return true;
	}
	if (tryAttrToken("LOGIN", loginToken)) {
		debugLogA("Max: selected token type LOGIN");
		return true;
	}

	// Last resort: take first non-empty token from tokenAttrs.
	for (const auto &kv : attrs->m) {
		if (kv.second.type != MpValue::MAP)
			continue;
		const MpValue *tok = mpGet(kv.second, "token");
		if (tok != nullptr && tok->type == MpValue::STR && !tok->s.IsEmpty()) {
			loginToken = tok->s;
			debugLogA("Max: selected token type fallback=%s", kv.first.c_str());
			return true;
		}
	}

	errorText = L"No usable token in tokenAttrs";
	return false;
}
