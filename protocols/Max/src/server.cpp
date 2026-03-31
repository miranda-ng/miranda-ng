#include "stdafx.h"

void CMaxProto::ShutdownConnection()
{
	mir_cslock lck(m_csNet);
	if (m_hConnection) {
		Netlib_Shutdown(m_hConnection);
		Netlib_CloseHandle(m_hConnection);
		m_hConnection = nullptr;
	}
}

bool CMaxProto::Connect()
{
	mir_cslock lck(m_csNet);
	if (m_hConnection != nullptr)
		return true;

	m_hConnection = Netlib_OpenConnection(m_hNetlibUser, "api.oneme.ru", 443, 20, NLOCF_SSL);
	if (m_hConnection == nullptr)
		Netlib_Logf(m_hNetlibUser, "Max: failed to connect to api.oneme.ru:443");
	return m_hConnection != nullptr;
}

void CMaxProto::WorkerThread(void *)
{
	if (!Connect() || !LoginWithToken()) {
		Netlib_Logf(m_hNetlibUser, "Max: login failed");
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}

	m_iStatus = ID_STATUS_ONLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_ONLINE);

	DWORD dwLastPing = GetTickCount();
	while (!InterlockedExchangeAdd(&m_iTerminated, 0)) {
		NETLIBSELECT nls = {};
		nls.dwTimeout = 1000;
		nls.hReadConns[0] = m_hConnection;
		nls.hReadConns[1] = nullptr;
		Netlib_Select(&nls);

		if (GetTickCount() - dwLastPing > 30000) {
			SendKeepAlive();
			dwLastPing = GetTickCount();
		}

		if (nls.hReadConns[0] == nullptr)
			continue;

		MaxFrame frame;
		if (!ReadFrame(frame))
			break;

		Netlib_Logf(m_hNetlibUser, "Max: RX opcode=%u cmd=%u seq=%u", frame.opcode, frame.cmd, frame.seq);
		DispatchFrame(frame);
	}

	ShutdownConnection();
	if (!Miranda_IsTerminated()) {
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_ONLINE, ID_STATUS_OFFLINE);
	}
}

uint8_t CMaxProto::NextSeq()
{
	return (uint8_t)(InterlockedIncrement(&m_seq) & 0xFF);
}

bool CMaxProto::SendAndWait(uint16_t opcode, const JSONNode &payload, JSONNode &outPayload, uint8_t cmd)
{
	PendingRequest req;
	req.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!req.hEvent)
		return false;

	MaxFrame frame;
	frame.cmd = cmd;
	frame.seq = NextSeq();
	frame.opcode = opcode;
	frame.payload = payload;

	{
		mir_cslock lck(m_csPending);
		m_pending[frame.seq] = &req;
	}

	bool sent = SendFrame(frame);
	if (!sent) {
		mir_cslock lck(m_csPending);
		m_pending.erase(frame.seq);
		CloseHandle(req.hEvent);
		return false;
	}

	DWORD dw = WaitForSingleObject(req.hEvent, 20000);
	CloseHandle(req.hEvent);
	if (dw != WAIT_OBJECT_0)
		return false;

	outPayload = req.payload;
	return req.ok;
}

bool CMaxProto::SendAndWaitBlocking(uint16_t opcode, const JSONNode &payload, JSONNode &outPayload, uint8_t cmd)
{
	MaxFrame frame;
	frame.cmd = cmd;
	frame.seq = NextSeq();
	frame.opcode = opcode;
	frame.payload = payload;
	if (!SendFrame(frame))
		return false;

	DWORD dwStop = GetTickCount() + 20000;
	while (GetTickCount() < dwStop) {
		MaxFrame incoming;
		if (!ReadFrame(incoming))
			return false;

		if (incoming.seq == frame.seq) {
			outPayload = incoming.payload;
			return true;
		}

		DispatchFrame(incoming);
	}
	return false;
}

JSONNode CMaxProto::BuildHandshakePayload()
{
	JSONNode ua(JSON_NODE), root(JSON_NODE);
	ua << CHAR_PARAM("deviceType", "ANDROID");
	ua << CHAR_PARAM("appVersion", "25.10.0");
	ua << CHAR_PARAM("osVersion", "Android 13");
	ua << CHAR_PARAM("timezone", "GMT");
	ua << CHAR_PARAM("screen", "130dpi 130dpi 600x874");
	ua << CHAR_PARAM("pushDeviceType", "GCM");
	ua << CHAR_PARAM("locale", "ru");
	ua << INT_PARAM("buildNumber", 6401);
	ua << CHAR_PARAM("deviceName", "Generic Android");
	ua << CHAR_PARAM("deviceLocale", "ru");
	ua.set_name("userAgent");

	CMStringA deviceId(getMStringA(MAX_SETTINGS_DEVICE_ID));
	if (deviceId.IsEmpty()) {
		char tmp[64];
		mir_snprintf(tmp, "mirmax-%u", (unsigned)time(nullptr));
		deviceId = tmp;
		setString(MAX_SETTINGS_DEVICE_ID, deviceId);
	}

	root << INT_PARAM("clientSessionId", 1);
	root << CHAR_PARAM("mt_instanceid", "miranda-max");
	root << ua;
	root << CHAR_PARAM("deviceId", deviceId);
	return root;
}

JSONNode CMaxProto::BuildSyncPayload()
{
	CMStringA token(getMStringA(MAX_SETTINGS_TOKEN));
	JSONNode root(JSON_NODE);
	root << BOOL_PARAM("interactive", 1);
	root << CHAR_PARAM("token", token);
	root << INT_PARAM("chatsSync", 0);
	root << INT_PARAM("contactsSync", 0);
	root << INT_PARAM("presenceSync", 0);
	root << INT_PARAM("draftsSync", 0);
	root << INT_PARAM("chatsCount", 40);
	return root;
}

bool CMaxProto::LoginWithToken()
{
	CMStringA token(getMStringA(MAX_SETTINGS_TOKEN));
	if (token.IsEmpty()) {
		Netlib_Logf(m_hNetlibUser, "Max: token is empty, set Token in account settings");
		return false;
	}

	JSONNode dummy;
	if (!SendAndWaitBlocking(6, BuildHandshakePayload(), dummy, 0))
		return false;

	return SendAndWaitBlocking(19, BuildSyncPayload(), dummy, 0);
}

bool CMaxProto::SendKeepAlive()
{
	JSONNode req(JSON_NODE), empty(JSON_NODE);
	req << BOOL_PARAM("interactive", 1);
	MaxFrame frame;
	frame.cmd = 0;
	frame.seq = NextSeq();
	frame.opcode = 1;
	frame.payload = req;
	return SendFrame(frame);
}

void CMaxProto::DispatchFrame(const MaxFrame &frame)
{
	bool handled = false;
	{
		mir_cslock lck(m_csPending);
		auto it = m_pending.find(frame.seq);
		if (it != m_pending.end()) {
			PendingRequest *p = it->second;
			p->payload = frame.payload;
			p->ok = true;
			SetEvent(p->hEvent);
			m_pending.erase(it);
			handled = true;
		}
	}
	if (handled)
		return;

	if (frame.opcode == 128)
		DispatchIncomingMessage(frame.payload);
}

void CMaxProto::DispatchIncomingMessage(const JSONNode &payload)
{
	const JSONNode &msg = payload["message"];
	if (!msg || !msg["text"])
		return;

	CMStringW text = msg["text"].as_mstring();
	int64_t senderId = msg["senderId"].as_int();
	if (senderId == 0)
		return;

	MCONTACT hContact = db_find_first(m_szModuleName);
	for (; hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (getDword(hContact, MAX_SETTINGS_ID, 0) == (uint32_t)senderId)
			break;
	}
	if (hContact == 0) {
		hContact = db_add_contact();
		Proto_AddToContact(hContact, m_szModuleName);
		setDword(hContact, MAX_SETTINGS_ID, (uint32_t)senderId);
	}

	ptrA utf8(mir_utf8encodeW(text));
	DB::EventInfo dbei;
	dbei.iTimestamp = (uint32_t)time(nullptr);
	dbei.pBlob = (char *)utf8;
	dbei.cbBlob = (uint32_t)mir_strlen(utf8);
	dbei.eventType = EVENTTYPE_MESSAGE;
	ProtoChainRecvMsg(hContact, dbei);
}
