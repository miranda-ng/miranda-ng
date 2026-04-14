/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"
#include <ctype.h>
#include <stdlib.h>
#include <m_timezones.h>

static bool MaxAsciiContainsInsensitive(const char *szHaystack, const char *szNeedle)
{
	if (!szHaystack || !szNeedle || !*szNeedle)
		return false;
	CMStringA h(szHaystack), n(szNeedle);
	h.MakeLower();
	n.MakeLower();
	return strstr(h, n) != nullptr;
}

// WebSocket endpoint and Origin for Max web client.
static const char *szWsUrl = "wss://ws-api.oneme.ru/websocket";
static const char *szOrigin = MAX_HTTP_ORIGIN_HEADER;
static const char *szWsUserAgent = MAX_HTTP_USER_AGENT;

void CMaxProto::ApplyWsExtensionsFromHttp(MHttpResponse *pReply)
{
	m_bWsPmDeflateIndependent = false;
	if (!pReply)
		return;
	for (auto &it : *pReply) {
		if (!it->szName || mir_strcmpi(it->szName, "Sec-WebSocket-Extensions"))
			continue;
		if (it->szValue && MaxAsciiContainsInsensitive(it->szValue, "server_no_context_takeover"))
			m_bWsPmDeflateIndependent = true;
	}
}

void CMaxProto::InitWsInflater()
{
	FreeWsInflater();
	memset(&m_wsInflate, 0, sizeof(m_wsInflate));
	if (inflateInit2(&m_wsInflate, -MAX_WBITS) == Z_OK)
		m_wsInflateInited = true;
}

void CMaxProto::FreeWsInflater()
{
	if (m_wsInflateInited) {
		inflateEnd(&m_wsInflate);
		m_wsInflateInited = false;
	}
	memset(&m_wsInflate, 0, sizeof(m_wsInflate));
}

// Long Sleep() would ignore m_bTerminated for tens of seconds; short slices make offline/shutdown responsive.
void CMaxProto::InterruptibleSleepMs(DWORD msTotal, DWORD sliceMs)
{
	DWORD done = 0;
	while (done < msTotal) {
		if (m_bTerminated)
			return;
		DWORD step = msTotal - done;
		if (step > sliceMs)
			step = sliceMs;
		Sleep(step);
		done += step;
	}
}

bool CMaxProto::InflateWsFrame(const uint8_t *pData, size_t cbData, CMStringA &out)
{
	out.Empty();
	if (!m_wsInflateInited)
		InitWsInflater();
	if (!m_wsInflateInited)
		return false;

	MBinBuffer src;
	src.append(pData, cbData);
	static const uint8_t tail[] = { 0x00, 0x00, 0xFF, 0xFF };
	src.append(tail, sizeof(tail));

	m_wsInflate.next_in = (Bytef *)src.data();
	m_wsInflate.avail_in = (uInt)src.length();

	char tmp[4096];
	int zret = Z_OK;
	while (zret == Z_OK) {
		m_wsInflate.next_out = (Bytef *)tmp;
		m_wsInflate.avail_out = sizeof(tmp);
		zret = inflate(&m_wsInflate, Z_SYNC_FLUSH);
		size_t produced = sizeof(tmp) - m_wsInflate.avail_out;
		if (produced)
			out.Append(tmp, (int)produced);
	}

	if (zret != Z_STREAM_END && zret != Z_BUF_ERROR) {
		// With sliding-window takeover, resetting here desyncs from the server; only reset for standalone frames.
		if (IsWsPmDeflateIndependent())
			InitWsInflater();
		return false;
	}
	return !out.IsEmpty();
}

// Single-message DEFLATE (permessage-deflate). RFC7692 suggests appending 00 00 FF FF before raw
// inflate; some stacks already end the bit stream and the trailer breaks inflate (Z_DATA_ERROR).
static bool sttInflateOneWsPayload(const uint8_t *pData, size_t cbData, CMStringA &out, int windowBits, bool appendRfc7692Tail)
{
	out.Empty();
	if (pData == nullptr || cbData == 0)
		return false;

	z_stream zs = {};
	if (inflateInit2(&zs, windowBits) != Z_OK)
		return false;

	MBinBuffer src;
	src.append(pData, cbData);
	if (appendRfc7692Tail) {
		static const uint8_t tail[] = { 0x00, 0x00, 0xFF, 0xFF };
		src.append(tail, sizeof(tail));
	}

	zs.next_in = (Bytef *)src.data();
	zs.avail_in = (uInt)src.length();

	char tmp[16384];
	int zret = Z_OK;
	unsigned guard = 0;
	while (zret == Z_OK) {
		if (++guard > 4096) {
			inflateEnd(&zs);
			return false;
		}
		zs.next_out = (Bytef *)tmp;
		zs.avail_out = sizeof(tmp);
		zret = inflate(&zs, Z_NO_FLUSH);
		size_t produced = sizeof(tmp) - zs.avail_out;
		if (produced)
			out.Append(tmp, (int)produced);
	}

	inflateEnd(&zs);
	if (zret != Z_STREAM_END)
		return false;
	return !out.IsEmpty();
}

static bool InflatePerMessageDeflate(CMaxProto *pPro, const uint8_t *pData, size_t cbData, CMStringA &out)
{
	// server_no_context_takeover: each RSV1 payload is a standalone raw deflate block.
	if (pPro->IsWsPmDeflateIndependent()) {
		if (sttInflateOneWsPayload(pData, cbData, out, -MAX_WBITS, true))
			return true;
		if (sttInflateOneWsPayload(pData, cbData, out, -MAX_WBITS, false))
			return true;
		if (sttInflateOneWsPayload(pData, cbData, out, MAX_WBITS, false))
			return true;
		if (sttInflateOneWsPayload(pData, cbData, out, MAX_WBITS | 32, false))
			return true;
		return false;
	}

	// Default (sliding-window takeover): the shared inflater must consume every compressed server frame in order.
	// Single-shot inflate on a continuation chunk fails with Z_DATA_ERROR / "invalid distance too far back".
	if (pPro->InflateWsFrame(pData, cbData, out))
		return true;

	// Fallback for odd first-frame edge cases.
	if (sttInflateOneWsPayload(pData, cbData, out, -MAX_WBITS, true))
		return true;
	if (sttInflateOneWsPayload(pData, cbData, out, -MAX_WBITS, false))
		return true;
	if (sttInflateOneWsPayload(pData, cbData, out, MAX_WBITS, false))
		return true;
	if (sttInflateOneWsPayload(pData, cbData, out, MAX_WBITS | 32, false))
		return true;

	out.Empty();
	// Do not InitWsInflater() here: a failed chunk may still be valid continuation; resetting desyncs the window.
	return false;
}

static bool ParsePossiblyWrappedJson(const char *buf, int len, JSONNode &root)
{
	root = JSONNode::parse(CMStringA(buf, len));
	if (root)
		return true;

	// Some responses may include noise around JSON or multiple chunks.
	for (int start = 0; start < len; ++start) {
		if (buf[start] != '{')
			continue;

		bool inString = false, esc = false;
		int depth = 0;
		for (int i = start; i < len; ++i) {
			char c = buf[i];
			if (inString) {
				if (esc) esc = false;
				else if (c == '\\') esc = true;
				else if (c == '"') inString = false;
				continue;
			}

			if (c == '"') {
				inString = true;
				continue;
			}
			if (c == '{') {
				++depth;
				continue;
			}
			if (c == '}') {
				--depth;
				if (depth == 0) {
					int cb = i - start + 1;
					root = JSONNode::parse(CMStringA(buf + start, cb));
					if (root)
						return true;
					break;
				}
			}
		}
	}
	return false;
}

static bool ParseServerJson(CMaxProto *pPro, const uint8_t *pData, size_t cbData, JSONNode &root)
{
	if (ParsePossiblyWrappedJson((const char*)pData, (int)cbData, root))
		return true;

	CMStringA inflated;
	if (!InflatePerMessageDeflate(pPro, pData, cbData, inflated))
		return false;

	return ParsePossiblyWrappedJson(inflated.c_str(), inflated.GetLength(), root);
}

// String success markers (API may send "error":"false" — strtol would mis-read as failure).
static bool MaxJsonStringMeansSuccess(const json_string &cs)
{
	size_t a = 0, b = cs.length();
	while (a < b && isspace((unsigned char)cs[a])) a++;
	while (b > a && isspace((unsigned char)cs[b - 1])) b--;
	if (a >= b)
		return true;

	json_string t = cs.substr(a, b - a);

	if (!_stricmp(t.c_str(), "false")) return true;
	if (!_stricmp(t.c_str(), "ok")) return true;
	if (!_stricmp(t.c_str(), "success")) return true;
	if (!_stricmp(t.c_str(), "none")) return true;
	if (t == "0") return true;

	if (!_stricmp(t.c_str(), "true")) return false;

	char *end = nullptr;
	long v = strtol(t.c_str(), &end, 10);
	if (end == t.c_str() + t.length())
		return v == 0;

	return false;
}

// Opcode 19 success envelopes include profile and/or chat/contact arrays (possibly empty).
// Opcode 32 often returns a singular "contact" (not "contacts[]") — must count as body so
// MaxPayloadSaysError does not treat e.g. payload.message as a hard failure.
static bool MaxWsSyncPayloadHasBody(const JSONNode &pl)
{
	if (pl["profile"].type() != JSON_NULL)
		return true;
	if (pl["messages"].type() == JSON_ARRAY)
		return true;
	if (pl["chat"].type() == JSON_NODE && pl["chat"]["messages"].type() == JSON_ARRAY)
		return true;
	if (pl["chats"].type() == JSON_ARRAY)
		return true;
	if (pl["contacts"].type() == JSON_ARRAY)
		return true;
	if (pl["contact"].type() == JSON_NODE)
		return true;
	if (pl["contacts"].type() == JSON_NODE && pl["contacts"].size() != 0)
		return true;
	return false;
}

// Server LOGIN/sync failure uses title + localizedMessage (+ message); not the chats/profile shape.
static bool MaxPayloadHasServerErrorUi(const JSONNode &pl)
{
	const JSONNode &t = pl["title"];
	if (t.type() == JSON_STRING && !t.as_string().empty())
		return true;
	const JSONNode &lm = pl["localizedMessage"];
	if (lm.type() == JSON_STRING && !lm.as_string().empty())
		return true;
	return false;
}

// API: "error" may be false / "false" / {} / 0 / { "code": 0 } / { "success": true } on success.
static bool MaxPayloadSaysError(const JSONNode &pl)
{
	if (!MaxWsSyncPayloadHasBody(pl) && MaxPayloadHasServerErrorUi(pl))
		return true;
	// Plain { "error", "message" } failures without title/localizedMessage.
	if (!MaxWsSyncPayloadHasBody(pl)) {
		const JSONNode &m = pl["message"];
		if (m.type() == JSON_STRING && !m.as_string().empty()) {
			const JSONNode &erp = pl["error"];
			if (erp.type() == JSON_BOOL && !erp.as_bool())
				return false;
			if (erp.type() == JSON_STRING && MaxJsonStringMeansSuccess(erp.as_string()))
				return false;
			return true;
		}
	}

	const JSONNode &er = pl["error"];
	if (er.type() == JSON_NULL)
		return false;
	if (er.type() == JSON_BOOL)
		return er.as_bool();
	if (er.type() == JSON_NUMBER)
		return er.as_float() != 0.0;
	if (er.type() == JSON_STRING)
		return !MaxJsonStringMeansSuccess(er.as_string());
	if (er.type() == JSON_NODE) {
		if (er.size() == 0)
			return false;
		const JSONNode &sok = er["success"];
		if (sok.type() == JSON_BOOL)
			return !sok.as_bool();
		const JSONNode &st = er["status"];
		if (st.type() == JSON_STRING && MaxJsonStringMeansSuccess(st.as_string()))
			return false;

		const JSONNode &code = er["code"];
		if (code.type() == JSON_NUMBER)
			return code.as_float() != 0.0;
		if (code.type() == JSON_BOOL)
			return code.as_bool();
		if (code.type() == JSON_STRING) {
			if (MaxJsonStringMeansSuccess(code.as_string()))
				return false;
			return true;
		}

		const JSONNode &msg = er["message"];
		if (msg.type() == JSON_STRING && !msg.as_string().empty())
			return true;
		const JSONNode &msg2 = er["msg"];
		if (msg2.type() == JSON_STRING && !msg2.as_string().empty())
			return true;
		return false;
	}
	// Some replies use []; non-empty arrays are not reliably errors — sync payload also uses arrays.
	if (er.type() == JSON_ARRAY)
		return false;
	return false;
}

// Server may use large seq values; as_int() truncates — read number or string.
static uint64_t JsonReplySeq(const JSONNode &json)
{
	const JSONNode &n = json["seq"];
	if (n.type() == JSON_NULL)
		return 0;
	if (n.type() == JSON_NUMBER)
		return (uint64_t)(n.as_float() + 0.5);
	return (uint64_t)_strtoui64(n.as_string().c_str(), nullptr, 10);
}

static int MaxJsonOpcodeInt(const JSONNode &json)
{
	const JSONNode &o = json["opcode"];
	if (o.type() == JSON_NULL)
		return -1;
	if (o.type() == JSON_NUMBER)
		return (int)(o.as_float() + 0.5);
	if (o.type() == JSON_STRING)
		return atoi(o.as_string().c_str());
	return -1;
}

// cmd=0: server push (or client request). cmd!=0: RPC result (e.g. 1) — must not conflate with push when seq collides.
static int JsonCmdInt(const JSONNode &json)
{
	const JSONNode &n = json["cmd"];
	if (n.type() == JSON_NULL)
		return -1;
	if (n.type() == JSON_NUMBER)
		return (int)(n.as_float() + 0.5);
	if (n.type() == JSON_STRING)
		return atoi(n.as_string().c_str());
	return -1;
}

// Opcode-32 fetch: reply may omit client seq or nest contacts — still complete SendJsonAndWait.
// Do not treat empty contacts:[] as a usable reply (would wake wait and merge nothing).
static bool MaxPayloadLooksLikeOpcode32ContactsReply(const JSONNode &pl)
{
	auto hasUsefulContact = [](const JSONNode &n) -> bool {
		if (n.type() != JSON_NODE || n.size() == 0)
			return false;
		return n["names"].type() != JSON_NULL || n["id"].type() != JSON_NULL || n["userId"].type() != JSON_NULL;
	};

	if (pl["contact"].type() == JSON_NODE && hasUsefulContact(pl["contact"]))
		return true;
	if (pl["contacts"].type() == JSON_ARRAY && pl["contacts"].size() > 0)
		return true;
	if (pl["contacts"].type() == JSON_NODE && pl["contacts"].size() != 0)
		return true;

	static const char *inners[] = { "result", "data", "response", "payload", nullptr };
	for (int i = 0; inners[i]; i++) {
		const JSONNode &n = pl[inners[i]];
		if (n.type() != JSON_NODE)
			continue;
		if (n["contact"].type() == JSON_NODE && hasUsefulContact(n["contact"]))
			return true;
		if (n["contacts"].type() == JSON_ARRAY && n["contacts"].size() > 0)
			return true;
		if (n["contacts"].type() == JSON_NODE && n["contacts"].size() != 0)
			return true;
	}
	return false;
}

static bool MaxPayloadLooksLikeOpcode48ChatsReply(const JSONNode &pl)
{
	if (pl["chats"].type() == JSON_ARRAY && pl["chats"].size() > 0)
		return true;
	static const char *inners[] = { "result", "data", "response", "payload", nullptr };
	for (int i = 0; inners[i]; i++) {
		const JSONNode &n = pl[inners[i]];
		if (n.type() == JSON_NODE && n["chats"].type() == JSON_ARRAY && n["chats"].size() > 0)
			return true;
	}
	return false;
}

static CMStringA HexPreview(const uint8_t *pData, size_t cbData, size_t limit = 48)
{
	const size_t n = min(cbData, limit);
	CMStringA out;
	for (size_t i = 0; i < n; ++i) {
		char b[4];
		mir_snprintf(b, "%02X", pData[i]);
		out += b;
		if (i + 1 < n)
			out += ' ';
	}
	return out;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMaxProto::EnsureDeviceId()
{
	ptrW w(getWStringA(DB_KEY_DEVICEID));
	if (w != nullptr && w[0])
		return;

	BYTE b[16];
	Utils_GetRandom(b, sizeof(b));
	b[6] = (b[6] & 0x0f) | 0x40;
	b[8] = (b[8] & 0x3f) | 0x80;
	wchar_t buf[40];
	mir_snwprintf(buf,
		L"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
	setWString(DB_KEY_DEVICEID, buf);
}

bool CMaxProto::SendJsonAndWait(WebSocket<CMaxProto> *ws, uint16_t opcode, JSONNode &payload, uint8_t cmd, bool acceptPayloadError)
{
	// Keep request/response matching strictly serialized: shared wait state
	// (m_waitSeq + m_szPendingResponse + m_hWaitEvent) is single-slot.
	mir_cslock lckWait(m_csWait);

	uint64_t seq;
	m_seq++;
	seq = m_seq;
	m_waitSeq = seq;
	m_szPendingResponse.Empty();
	ResetEvent(m_hWaitEvent);

	JSONNode root(JSON_NODE);
	root << INT_PARAM("ver", 11) << INT_PARAM("cmd", cmd) << INT64_PARAM("seq", (int64_t)seq) << INT_PARAM("opcode", opcode)
		<< JSON_PARAM("payload", payload);

	json_string s = root.write();
	{
		mir_cslock lckSend(m_csSend);
		ws->sendText(s.c_str());
	}

	if (WaitForSingleObject(m_hWaitEvent, 10000) != WAIT_OBJECT_0) {
		debugLogA("Max: request timeout (opcode %u, waited seq %llu)", (unsigned)opcode, (unsigned long long)m_waitSeq);
		m_waitSeq = 0;
		InitWsInflater();
		return false;
	}

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp) {
		debugLogA("Max: empty/invalid JSON after response (opcode %u)", (unsigned)opcode);
		m_waitSeq = 0;
		return false;
	}

	const JSONNode &pl = resp["payload"];
	bool payloadErr = MaxPayloadSaysError(pl);
	// Server sometimes sets error to a non-printable or odd string while still returning sync data.
	if (payloadErr && opcode == 19 && MaxWsSyncPayloadHasBody(pl)) {
		debugLogA("Max: opcode 19 treating as OK (sync body present); errType=%u strLen=%u",
			(unsigned)pl["error"].type(),
			pl["error"].type() == JSON_STRING ? (unsigned)pl["error"].as_string().length() : 0u);
		payloadErr = false;
	}
	// Ignore odd string "error" on opcode 19 only when there is no user-visible error text (see title/localizedMessage/message).
	if (payloadErr && opcode == 19 && pl["error"].type() == JSON_STRING && !MaxPayloadHasServerErrorUi(pl)) {
		const JSONNode &m = pl["message"];
		if (m.type() != JSON_STRING || m.as_string().empty()) {
			debugLogA("Max: opcode 19 ignoring string error field (len=%u)",
				(unsigned)pl["error"].as_string().length());
			payloadErr = false;
		}
	}
	if (payloadErr && !acceptPayloadError) {
		json_string err = pl["error"].write();
		debugLogA("Max: payload error (opcode %u) errType=%u: %s", (unsigned)opcode, (unsigned)pl["error"].type(),
			err.empty() ? "(empty)" : err.c_str());
		if (pl["title"].type() == JSON_STRING && !pl["title"].as_string().empty())
			debugLogA("Max:   title=%s", pl["title"].as_string().c_str());
		if (pl["localizedMessage"].type() == JSON_STRING && !pl["localizedMessage"].as_string().empty())
			debugLogA("Max:   localizedMessage=%s", pl["localizedMessage"].as_string().c_str());
		if (pl["message"].type() == JSON_STRING && !pl["message"].as_string().empty())
			debugLogA("Max:   message=%s", pl["message"].as_string().c_str());
		m_waitSeq = 0;
		return false;
	}
	m_waitSeq = 0;
	return true;
}

bool CMaxProto::SendHandshake(WebSocket<CMaxProto> *ws)
{
	EnsureDeviceId();
	ptrW wszDev(getWStringA(DB_KEY_DEVICEID));
	if (wszDev == nullptr)
		return false;

	ptrA devUtf(mir_utf8encodeW(wszDev));

	const char *szLocaleRaw = Langpack_GetDefaultLocaleName();
	const char *szLocale = "en";
	// Use current Miranda langpack, but fallback to English unless it's Russian.
	// (Server appears to support only a limited locale set; rare locales may return empty localizedMessage.)
	if (szLocaleRaw != nullptr && (szLocaleRaw[0] == 'r' || szLocaleRaw[0] == 'R') && (szLocaleRaw[1] == 'u' || szLocaleRaw[1] == 'U'))
		szLocale = "ru";
	m_wsLocale = szLocale;

	LPCTSTR tszTz = TimeZone_GetName(LOCAL_TIME_HANDLE);
	ptrA tzUtf((tszTz != nullptr && tszTz[0] != 0) ? mir_u2a(tszTz) : nullptr);
	const char *szTz = (tzUtf != nullptr && tzUtf[0] != 0) ? tzUtf.get() : "UTC";

	JSONNode ua(JSON_NODE);
	ua << CHAR_PARAM("deviceType", "WEB") << CHAR_PARAM("locale", szLocale) << CHAR_PARAM("deviceLocale", szLocale) << CHAR_PARAM("osVersion", "Linux")
		<< CHAR_PARAM("deviceName", "Chrome")
		<< CHAR_PARAM("headerUserAgent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36")
		<< CHAR_PARAM("appVersion", "25.10.13") << CHAR_PARAM("screen", "1080x1920 1.0x") << CHAR_PARAM("timezone", szTz);

	JSONNode payload(JSON_NODE);
	payload << CHAR_PARAM("deviceId", devUtf) << JSON_PARAM("userAgent", ua);

	return SendJsonAndWait(ws, 6, payload, 0);
}

bool CMaxProto::ApiSync(WebSocket<CMaxProto> *ws)
{
	ptrA tok(getStringA(DB_KEY_LOGIN_TOKEN));
	if (tok == nullptr || tok[0] == 0)
		return false;

	JSONNode payload(JSON_NODE);
	// presenceSync=0 matches web client behaviour.
	// chatsSync=1: server includes dialogs with lastMessage (missed while offline); was 0 and only live opcode-128 worked.
	payload << BOOL_PARAM("interactive", true) << CHAR_PARAM("token", tok) << INT_PARAM("chatsSync", 1) << INT_PARAM("contactsSync", 1)
		<< INT_PARAM("presenceSync", 0) << INT_PARAM("draftsSync", 0) << INT_PARAM("chatsCount", 40);

	return SendJsonAndWait(ws, 19, payload, 0);
}

bool CMaxProto::ApiFetchChatMessages(WebSocket<CMaxProto> *ws, const char *szChatId, int64_t fromMs, int forward, int backward, bool bMarkRead)
{
	if (!ws || szChatId == nullptr || szChatId[0] == 0)
		return false;
	if (forward <= 0 && backward <= 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	if (fromMs <= 0)
		fromMs = (int64_t)time(nullptr) * 1000;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid) << INT64_PARAM("from", fromMs) << INT_PARAM("forward", forward) << INT_PARAM("backward", backward) << BOOL_PARAM("getMessages", true);

	if (!SendJsonAndWait(ws, 49, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return false;

	IngestChatHistoryPayload(resp["payload"], szChatId, bMarkRead);
	return true;
}

bool CMaxProto::ApiSendMessage(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szText, CMStringA *pOutMsgId)
{
	if (pOutMsgId != nullptr)
		pOutMsgId->Empty();
	if (!ws || szChatId == nullptr || szChatId[0] == 0 || szText == nullptr || szText[0] == 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	FILETIME ft = {};
	GetSystemTimeAsFileTime(&ft);
	ULARGE_INTEGER ui = {};
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;
	// Windows epoch (100ns) -> Unix epoch milliseconds.
	uint64_t nowMs = (ui.QuadPart - 116444736000000000ULL) / 10000ULL;
	uint64_t cidMs = 0;
	{
		mir_cslock lck(m_csCid);
		cidMs = nowMs;
		if (cidMs <= m_lastClientCidMs)
			cidMs = m_lastClientCidMs + 1;
		m_lastClientCidMs = cidMs;
	}
	JSONNode msg(JSON_NODE);
	msg << CHAR_PARAM("text", szText) << INT64_PARAM("cid", (int64_t)cidMs);
	JSONNode elems(JSON_ARRAY), attaches(JSON_ARRAY);
	msg << JSON_PARAM("elements", elems) << JSON_PARAM("attaches", attaches);

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid) << JSON_PARAM("message", msg) << BOOL_PARAM("notify", true);

	if (!SendJsonAndWait(ws, 64, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return true;

	const JSONNode &pl = resp["payload"];
	const JSONNode &m = pl["message"];
	const JSONNode *idNode = nullptr;
	if (m.type() == JSON_NODE && m["id"].type() != JSON_NULL)
		idNode = &m["id"];
	else if (pl["messageId"].type() != JSON_NULL)
		idNode = &pl["messageId"];
	else if (pl["id"].type() != JSON_NULL)
		idNode = &pl["id"];

	if (idNode != nullptr && pOutMsgId != nullptr) {
		if (idNode->type() == JSON_NUMBER)
			pOutMsgId->Format("%.0f", idNode->as_float());
		else
			*pOutMsgId = idNode->as_string().c_str();
	}
	return true;
}

bool CMaxProto::ApiSendTyping(WebSocket<CMaxProto> *ws, const char *szChatId, bool bTyping)
{
	if (!ws || szChatId == nullptr || szChatId[0] == 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid) << BOOL_PARAM("typing", bTyping);

	// Opcode 65: MSG_TYPING (fire-and-forget to keep UI responsive).
	uint64_t seq = 0;
	{
		mir_cslock lckWait(m_csWait);
		m_seq++;
		seq = m_seq;
	}

	JSONNode root(JSON_NODE);
	root << INT_PARAM("ver", 11) << INT_PARAM("cmd", 0) << INT64_PARAM("seq", (int64_t)seq) << INT_PARAM("opcode", 65)
		<< JSON_PARAM("payload", payload);
	json_string s = root.write();
	{
		mir_cslock lckSend(m_csSend);
		ws->sendText(s.c_str());
	}
	return true;
}

bool CMaxProto::ApiEditMessage(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szMsgId, const char *szText)
{
	if (!ws || szChatId == nullptr || szChatId[0] == 0 || szMsgId == nullptr || szMsgId[0] == 0 || szText == nullptr)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	JSONNode payload(JSON_NODE);
	JSONNode elems(JSON_ARRAY), attaches(JSON_ARRAY);
	payload << INT64_PARAM("chatId", cid) << CHAR_PARAM("messageId", szMsgId) << CHAR_PARAM("text", szText)
		<< JSON_PARAM("elements", elems) << JSON_PARAM("attachments", attaches);

	return SendJsonAndWait(ws, 67, payload, 0);
}

bool CMaxProto::ApiGetFileDownloadUrl(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szMsgId, int64_t fileId, CMStringA &outUrl)
{
	outUrl.Empty();
	if (!ws || szChatId == nullptr || szChatId[0] == 0 || szMsgId == nullptr || szMsgId[0] == 0 || fileId <= 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid) << CHAR_PARAM("messageId", szMsgId) << INT64_PARAM("fileId", fileId);
	if (!SendJsonAndWait(ws, 88, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return false;

	const JSONNode &pl = resp["payload"];
	if (pl["url"].type() == JSON_STRING && !pl["url"].as_string().empty()) {
		outUrl = pl["url"].as_string().c_str();
		return true;
	}
	return false;
}

bool CMaxProto::ApiDeleteMessages(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szMsgId, bool bForMe)
{
	if (!ws || szChatId == nullptr || szChatId[0] == 0 || szMsgId == nullptr || szMsgId[0] == 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	JSONNode messageIds(JSON_ARRAY);
	messageIds << CHAR_PARAM("", szMsgId);

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid) << JSON_PARAM("messageIds", messageIds) << BOOL_PARAM("forMe", bForMe);

	// PyMax / vkmax: Opcode delete message == 66
	return SendJsonAndWait(ws, 66, payload, 0);
}

bool CMaxProto::ApiSearchByPhone(WebSocket<CMaxProto> *ws, const char *szPhoneUtf8, JSONNode &outContact)
{
	outContact = JSONNode(JSON_NULL);
	if (!ws || szPhoneUtf8 == nullptr || szPhoneUtf8[0] == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << CHAR_PARAM("phone", szPhoneUtf8);

	if (!SendJsonAndWait(ws, 46, payload, 0, true))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return false;

	const JSONNode &pl = resp["payload"];
	if (pl.type() != JSON_NODE)
		return false;

	const JSONNode &ct = pl["contact"];
	if (ct.type() == JSON_NODE && ct.size() != 0) {
		outContact = ct;
		return true;
	}

	// not.found and similar — still a completed reply
	if (MaxPayloadSaysError(pl))
		return true;

	return false;
}

bool CMaxProto::ApiAddContactOnServer(WebSocket<CMaxProto> *ws, const char *szUidDecimal)
{
	if (!ws || szUidDecimal == nullptr || szUidDecimal[0] == 0)
		return false;

	int64_t contactId = _strtoi64(szUidDecimal, nullptr, 10);
	if (contactId == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("contactId", contactId) << CHAR_PARAM("action", "ADD");

	return SendJsonAndWait(ws, 34, payload, 0, true);
}

bool CMaxProto::ApiRemoveContactFromServer(WebSocket<CMaxProto> *ws, const char *szUidDecimal)
{
	if (!ws || szUidDecimal == nullptr || szUidDecimal[0] == 0)
		return false;

	int64_t contactId = _strtoi64(szUidDecimal, nullptr, 10);
	if (contactId == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("contactId", contactId) << CHAR_PARAM("action", "REMOVE");

	return SendJsonAndWait(ws, 34, payload, 0, true);
}

bool CMaxProto::ApiDeleteServerDialog(WebSocket<CMaxProto> *ws, const char *szChatId)
{
	if (!ws || szChatId == nullptr || szChatId[0] == 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid);

	// MaxApiTeam/PyMax: Opcode.CHAT_DELETE == 52 (75 is CHAT_SUBSCRIBE — does not remove dialog from list).
	return SendJsonAndWait(ws, 52, payload, 0, true);
}

bool CMaxProto::ApiChatLeave(WebSocket<CMaxProto> *ws, const char *szChatId)
{
	if (!ws || szChatId == nullptr || szChatId[0] == 0)
		return false;

	int64_t cid = _strtoi64(szChatId, nullptr, 10);
	if (cid == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << INT64_PARAM("chatId", cid);

	// PyMax: Opcode.CHAT_LEAVE == 58 (LeaveChatPayload — chatId only).
	return SendJsonAndWait(ws, 58, payload, 0, true);
}

bool CMaxProto::ApiUpdateMyProfile(WebSocket<CMaxProto> *ws, const char *szFirstNameUtf8, const char *szLastNameUtf8, const char *szDescriptionUtf8)
{
	if (!ws || szFirstNameUtf8 == nullptr || szFirstNameUtf8[0] == 0)
		return false;
	if (szDescriptionUtf8 == nullptr)
		return false;

	JSONNode payload(JSON_NODE);
	payload << CHAR_PARAM("firstName", szFirstNameUtf8) << CHAR_PARAM("avatarType", "USER_AVATAR");
	if (szLastNameUtf8 != nullptr && szLastNameUtf8[0] != 0)
		payload << CHAR_PARAM("lastName", szLastNameUtf8);
	payload << CHAR_PARAM("description", szDescriptionUtf8);

	// PyMax: Opcode.PROFILE == 16 — treat payload errors as failure (show localizedMessage, do not update DB).
	return SendJsonAndWait(ws, 16, payload, 0, false);
}

bool CMaxProto::ApiPing(WebSocket<CMaxProto> *ws)
{
	JSONNode payload(JSON_NODE);
	// Keep session in active/foreground mode to receive ephemeral pushes (typing etc.).
	payload << BOOL_PARAM("interactive", true);
	return SendJsonAndWait(ws, 1, payload, 0);
}

bool CMaxProto::ApiWebSessionBootstrap(WebSocket<CMaxProto> *ws)
{
	// Session: opcode 6 (handshake) then opcode 19 — no extra pre-sync ping here.
	(void)ws;
	return true;
}

bool CMaxProto::ApiSendTelemetryColdStart(WebSocket<CMaxProto> *ws)
{
	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my == nullptr || my[0] == 0)
		return false;

	uint64_t userId = _strtoui64(my, nullptr, 10);
	int64_t t = (int64_t)(time(nullptr) * 1000);
	int64_t sessionId = (int64_t)GetTickCount64() + (int64_t)(time(nullptr) * 1000);

	JSONNode params(JSON_NODE);
	params << INT64_PARAM("actionId", 1) << INT_PARAM("screenTo", 150) << INT_PARAM("screenFrom", 1)
		<< INT_PARAM("sourceId", 1) << INT64_PARAM("sessionId", sessionId);

	JSONNode ev(JSON_NODE);
	ev << CHAR_PARAM("event", "COLD_START") << CHAR_PARAM("type", "NAV") << INT64_PARAM("time", t)
		<< INT64_PARAM("userId", (int64_t)userId) << JSON_PARAM("params", params);

	JSONNode events(JSON_ARRAY);
	events.push_back(ev);

	JSONNode payload(JSON_NODE);
	payload << JSON_PARAM("events", events);

	if (!SendJsonAndWait(ws, 5, payload, 0))
		return false;
	return true;
}

void __cdecl CMaxProto::PingWorker(void *)
{
	// Keepalive ~30s cadence; short delay avoids racing the post-login burst.
	InterruptibleSleepMs(2500);
	if (!m_bTerminated && m_pGateway) {
		if (!ApiPing(m_pGateway))
			debugLogA("Max: first ping failed");
	}
	while (!m_bTerminated && m_pGateway) {
		InterruptibleSleepMs(30000);
		if (m_bTerminated || !m_pGateway)
			break;
		if (!ApiPing(m_pGateway))
			debugLogA("Max: ping failed");
	}
}

void CMaxProto::OnGatewayPush(const JSONNode &payload, int opcode)
{
	debugLogA("Max: push opcode=%d", opcode);
	// Some server builds send typing in different push opcodes; try generic parse first.
	TryIngestTypingPayload(payload);
	if (opcode == 135)
		OnMaxPushChatRemoved(payload);
	if (opcode == 128)
		TryIngestNotifMessagePayload(payload);
	TryMergeContactsFromPayload(payload);
	TryApplySyncPayloadFromPush(payload);
}

void __cdecl CMaxProto::WsRunThread(void *)
{
	if (m_wsRun.ws != nullptr)
		m_wsRun.ws->run();
}

void __cdecl CMaxProto::ConnectionWorker(void *)
{
	WebSocket<CMaxProto> ws(this);
	MHttpHeaders hdrs;
	hdrs.AddHeader("Origin", szOrigin);
	hdrs.AddHeader("User-Agent", szWsUserAgent);
	// Request independent permessage-deflate blocks; this avoids zlib context drift on tiny pushes.
	hdrs.AddHeader("Sec-WebSocket-Extensions", "permessage-deflate; client_no_context_takeover; server_no_context_takeover; client_max_window_bits");

	NLHR_PTR pReply(ws.connect(m_hNetlibUser, szWsUrl, &hdrs));
	if (!pReply || pReply->resultCode != 101) {
		debugLogA("Max: WebSocket failed (code=%d)", pReply ? pReply->resultCode : -1);
		return;
	}

	ApplyWsExtensionsFromHttp(pReply);
	debugLogA("Max: WS deflate server_no_context_takeover=%d", IsWsPmDeflateIndependent() ? 1 : 0);

	m_pGateway = &ws;
	m_wsRun.ws = &ws;
	m_bGatewayConnected = false;
	m_bInitialSyncOk = false;
	InitWsInflater();
	ResetServerContactBookCache();

	m_hWsRunThread = ForkThreadEx(&CMaxProto::WsRunThread, this, nullptr);
	if (!m_hWsRunThread) {
		m_pGateway = nullptr;
		m_wsRun.ws = nullptr;
		m_bGatewayConnected = false;
		return;
	}

	Sleep(50);

	if (!SendHandshake(&ws)) {
		debugLogA("Max: handshake failed");
		ws.terminate();
		WaitForSingleObject(m_hWsRunThread, 15000);
		CloseHandle(m_hWsRunThread);
		m_hWsRunThread = nullptr;
		m_pGateway = nullptr;
		m_wsRun.ws = nullptr;
		m_bGatewayConnected = false;
		return;
	}

	ptrA token(getStringA(DB_KEY_LOGIN_TOKEN));
	if (token != nullptr && token[0]) {
		ApiWebSessionBootstrap(&ws);
		m_bInitialSyncOk = ApiSync(&ws);
		if (m_bInitialSyncOk) {
			debugLogA("Max: ApiSync OK, applying roster");
			// Let the WS reader thread drain follow-up frames (server may push sync after LOGIN ack).
			Sleep(400);
			JSONNode syncRoot = JSONNode::parse(m_szPendingResponse.c_str());
			if (syncRoot)
				ApplySyncPayload(syncRoot["payload"], &ws);
		}
		else
			debugLogA("Max: ApiSync failed — no roster (see timeout/error lines above)");
	}
	else
		m_bInitialSyncOk = true;

	// Ready for WaitForGatewayReady() only after handshake + optional sync finished.
	m_bGatewayConnected = true;

	if (token != nullptr && token[0] && m_bInitialSyncOk) {
		ApiSendTelemetryColdStart(&ws);
		m_hPingThread = ForkThreadEx(&CMaxProto::PingWorker, this, nullptr);
	}

	WaitForSingleObject(m_hWsRunThread, INFINITE);
	CloseHandle(m_hWsRunThread);
	m_hWsRunThread = nullptr;

	m_pGateway = nullptr;
	if (m_hPingThread) {
		// Ping uses interruptible sleep; a few seconds is enough unless the process is wedged.
		WaitForSingleObject(m_hPingThread, 5000);
		CloseHandle(m_hPingThread);
		m_hPingThread = nullptr;
	}

	m_bTerminated = true;

	m_wsRun.ws = nullptr;
	m_bGatewayConnected = false;
}

void WebSocket<CMaxProto>::process(const uint8_t *buf, size_t cbLen)
{
	JSONNode json;
	if (!ParseServerJson(p, buf, cbLen, json)) {
		CMStringA hex = HexPreview(buf, cbLen);
		p->debugLogA("Max: cannot parse websocket frame (len=%u, hex=%s)", (unsigned)cbLen, hex.c_str());
		return;
	}

	// Recv uses MSG_NODUMP — mirror a short preview into the same log channel as other Max: lines.
	{
		json_string sj = json.write();
		const char *pch = sj.c_str();
		unsigned n = (unsigned)sj.length();
		if (n > 1800) {
			CMStringA head(pch, 1800);
			p->debugLogA("Max: ws in (%u bytes, truncated) %s", n, head.c_str());
		}
		else if (n)
			p->debugLogA("Max: ws in (%u bytes) %s", n, pch);
	}

	uint64_t seq = JsonReplySeq(json);
	if (seq != 0 && seq == p->m_waitSeq) {
		const int cmd = JsonCmdInt(json);
		// Pushes reuse cmd=0; matching seq alone would skip OnGatewayPush and break contact/chat sync.
		if (cmd != 0) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	int op = MaxJsonOpcodeInt(json);

	// Opcode 16 (PROFILE): server often echoes cmd=0 like a push; seq still matches the client request.
	if (p->m_waitSeq != 0 && seq == p->m_waitSeq && op == 16) {
		const JSONNode &pl = json["payload"];
		if (pl.type() == JSON_NODE) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Contact fetch: seq may not match client value; payload still carries contacts/contact.
	if (p->m_waitSeq != 0 && op == 32) {
		const JSONNode &pl = json["payload"];
		if (MaxPayloadLooksLikeOpcode32ContactsReply(pl)) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Chat details by id (opcode 48); used to fill dialog title when opcode 32 returns no names.
	if (p->m_waitSeq != 0 && op == 48) {
		const JSONNode &pl = json["payload"];
		if (MaxPayloadLooksLikeOpcode48ChatsReply(pl)) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Opcode 49: chat history (messages[]), same seq as request.
	if (p->m_waitSeq != 0 && op == 49) {
		const JSONNode &pl = json["payload"];
		if (pl.type() == JSON_NODE) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Opcode 46: phone lookup — server may omit echo of client seq.
	if (p->m_waitSeq != 0 && op == 46) {
		const JSONNode &pl = json["payload"];
		if (pl.type() == JSON_NODE) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Opcode 34: add contact — same pattern as 46.
	if (p->m_waitSeq != 0 && op == 34) {
		const JSONNode &pl = json["payload"];
		if (pl.type() == JSON_NODE) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Opcode 88: file download url by (chatId,messageId,fileId).
	if (p->m_waitSeq != 0 && op == 88) {
		const JSONNode &pl = json["payload"];
		if (pl.type() == JSON_NODE) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Opcode 66: delete message — reply often keeps cmd=0 like pushes.
	if (p->m_waitSeq != 0 && seq == p->m_waitSeq && op == 66) {
		const JSONNode &pl = json["payload"];
		if (pl.type() == JSON_NODE) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	// Rare: reply echoes server-side seq — accept sync payload when it clearly carries roster data.
	if (p->m_waitSeq != 0 && op == 19) {
		const JSONNode &pl = json["payload"];
		if (pl["chats"].type() == JSON_ARRAY || pl["contacts"].type() == JSON_ARRAY || pl["profile"].type() != JSON_NULL
		    || pl["contact"].type() == JSON_NODE
		    || (pl["contacts"].type() == JSON_NODE && pl["contacts"].size() != 0)) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	p->OnGatewayPush(json["payload"], op);
}
