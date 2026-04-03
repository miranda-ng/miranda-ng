/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"

// WebSocket endpoint and Origin for Max web client.
static const char *szWsUrl = "wss://ws-api.oneme.ru/websocket";
static const char *szOrigin = "https://web.max.ru";
static const char *szWsUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:142.0) Gecko/20100101 Firefox/142.0";

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
		// stream got desynced, recreate inflater for next message
		InitWsInflater();
		return false;
	}
	return !out.IsEmpty();
}

static bool InflatePerMessageDeflate(CMaxProto *pPro, const uint8_t *pData, size_t cbData, CMStringA &out)
{
	// 1) RFC7692 stream with possible context takeover (persistent inflater)
	if (pPro->InflateWsFrame(pData, cbData, out))
		return true;

	out.Empty();
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

bool CMaxProto::SendJsonAndWait(WebSocket<CMaxProto> *ws, uint16_t opcode, JSONNode &payload, uint8_t cmd)
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

	if (WaitForSingleObject(m_hWaitEvent, 10000) != WAIT_OBJECT_0)
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	if (!resp)
		return false;

	const JSONNode &pl = resp["payload"];
	if (pl["error"].type() != JSON_NULL)
		return false;
	return true;
}

bool CMaxProto::SendHandshake(WebSocket<CMaxProto> *ws)
{
	EnsureDeviceId();
	ptrW wszDev(getWStringA(DB_KEY_DEVICEID));
	if (wszDev == nullptr)
		return false;

	ptrA devUtf(mir_utf8encodeW(wszDev));

	JSONNode ua(JSON_NODE);
	ua << CHAR_PARAM("deviceType", "WEB") << CHAR_PARAM("locale", "ru") << CHAR_PARAM("deviceLocale", "ru") << CHAR_PARAM("osVersion", "Linux")
		<< CHAR_PARAM("deviceName", "Chrome")
		<< CHAR_PARAM("headerUserAgent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36")
		<< CHAR_PARAM("appVersion", "25.10.13") << CHAR_PARAM("screen", "1080x1920 1.0x") << CHAR_PARAM("timezone", "Europe/Moscow");

	JSONNode payload(JSON_NODE);
	payload << CHAR_PARAM("deviceId", devUtf) << JSON_PARAM("userAgent", ua);

	return SendJsonAndWait(ws, 6, payload, 0);
}

bool CMaxProto::ApiStartAuth(WebSocket<CMaxProto> *ws, const char *phone)
{
	JSONNode payload(JSON_NODE);
	payload << CHAR_PARAM("phone", phone) << CHAR_PARAM("type", "START_AUTH") << CHAR_PARAM("language", "ru");
	if (!SendJsonAndWait(ws, 17, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	const JSONNode &pl = resp["payload"];
	if (pl["token"].type() == JSON_NULL)
		return false;
	ptrA tok(mir_strdup(pl["token"].as_string().c_str()));
	setString(DB_KEY_TOKEN_TMP, tok);
	return true;
}

bool CMaxProto::ApiVerifyCode(WebSocket<CMaxProto> *ws, const char *code)
{
	ptrA tmp(getStringA(DB_KEY_TOKEN_TMP));
	if (tmp == nullptr)
		return false;

	JSONNode payload(JSON_NODE);
	payload << CHAR_PARAM("token", tmp) << CHAR_PARAM("verifyCode", code) << CHAR_PARAM("authTokenType", "CHECK_CODE");

	if (!SendJsonAndWait(ws, 18, payload, 0))
		return false;

	JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
	const JSONNode &pl = resp["payload"];
	const JSONNode &attrs = pl["tokenAttrs"];
	if (attrs.type() == JSON_NULL)
		return false;

	const JSONNode &login = attrs["LOGIN"];
	if (login.type() == JSON_NULL || login["token"].type() == JSON_NULL)
		return false;

	ptrA tok(mir_strdup(login["token"].as_string().c_str()));
	setString(DB_KEY_LOGIN_TOKEN, tok);
	return true;
}

bool CMaxProto::ApiSync(WebSocket<CMaxProto> *ws)
{
	ptrA tok(getStringA(DB_KEY_LOGIN_TOKEN));
	if (tok == nullptr || tok[0] == 0)
		return false;

	JSONNode payload(JSON_NODE);
	payload << BOOL_PARAM("interactive", true) << CHAR_PARAM("token", tok) << INT_PARAM("chatsSync", 0) << INT_PARAM("contactsSync", 0)
		<< INT_PARAM("presenceSync", 0) << INT_PARAM("draftsSync", 0) << INT_PARAM("chatsCount", 40);

	return SendJsonAndWait(ws, 19, payload, 0);
}

void CMaxProto::OnGatewayPush(const JSONNode &payload, int opcode)
{
	debugLogA("Max: push opcode=%d", opcode);
	if (opcode == 128)
		(void)payload;
}

void CMaxProto::WsRunThread(void *)
{
	if (m_wsRun.ws != nullptr)
		m_wsRun.ws->run();
}

void CMaxProto::ConnectionWorker(void *)
{
	WebSocket<CMaxProto> ws(this);
	MHttpHeaders hdrs;
	hdrs.AddHeader("Origin", szOrigin);
	hdrs.AddHeader("User-Agent", szWsUserAgent);

	NLHR_PTR pReply(ws.connect(m_hNetlibUser, szWsUrl, &hdrs));
	if (!pReply || pReply->resultCode != 101) {
		debugLogA("Max: WebSocket failed (code=%d)", pReply ? pReply->resultCode : -1);
		return;
	}

	m_pGateway = &ws;
	m_wsRun.ws = &ws;
	m_bGatewayConnected = false;
	m_bInitialSyncOk = false;
	InitWsInflater();

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
	if (token != nullptr && token[0])
		m_bInitialSyncOk = ApiSync(&ws);
	else
		m_bInitialSyncOk = true;

	// Ready for WaitForGatewayReady() only after handshake + optional sync finished.
	m_bGatewayConnected = true;

	WaitForSingleObject(m_hWsRunThread, INFINITE);
	CloseHandle(m_hWsRunThread);
	m_hWsRunThread = nullptr;

	m_bTerminated = true;

	m_pGateway = nullptr;
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

	if (json["seq"].type() != JSON_NULL) {
		uint64_t seq = (uint64_t)json["seq"].as_int();
		if (seq == p->m_waitSeq) {
			json_string s = json.write();
			p->m_szPendingResponse = s.c_str();
			SetEvent(p->m_hWaitEvent);
			return;
		}
	}

	int opcode = json["opcode"].as_int();
	p->OnGatewayPush(json["payload"], opcode);
}
