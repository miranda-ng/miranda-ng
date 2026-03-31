#include "stdafx.h"

namespace
{
	static INT_PTR CALLBACK MaxAccountProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		CMaxProto *ppro = (CMaxProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

		switch (msg) {
		case WM_INITDIALOG:
			ppro = (CMaxProto*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ppro);
			if (ppro != nullptr) {
				SetDlgItemTextA(hwndDlg, IDC_TOKEN, ppro->getMStringA(MAX_SETTINGS_TOKEN));
				SetDlgItemTextA(hwndDlg, IDC_DEVICEID, ppro->getMStringA(MAX_SETTINGS_DEVICE_ID));
			}
			return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_NOTIFY:
			if (((NMHDR*)lParam)->code == PSN_APPLY && ppro != nullptr) {
				char szToken[2048] = {}, szDeviceId[256] = {};
				GetDlgItemTextA(hwndDlg, IDC_TOKEN, szToken, _countof(szToken));
				GetDlgItemTextA(hwndDlg, IDC_DEVICEID, szDeviceId, _countof(szDeviceId));
				ppro->setString(MAX_SETTINGS_TOKEN, szToken);
				ppro->setString(MAX_SETTINGS_DEVICE_ID, szDeviceId);
				return TRUE;
			}
			break;
		}

		return FALSE;
	}

	static bool cmp_reader_cb(cmp_ctx_t *ctx, void *data, size_t limit)
	{
		MBinBuffer *buf = (MBinBuffer*)ctx->buf;
		if (buf->length() < limit)
			return false;

		memcpy(data, buf->data(), limit);
		buf->remove(limit);
		return true;
	}

	static size_t cmp_writer_cb(cmp_ctx_t *ctx, const void *data, size_t count)
	{
		MBinBuffer *buf = (MBinBuffer*)ctx->buf;
		buf->append(data, count);
		return count;
	}
}

CMaxProto::CMaxProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CMaxProto>(szModuleName, ptszUserName)
{
	m_iTerminated = 0;
	m_msgId = 0;
	m_seq = 0;
	m_hNetlibUser = nullptr;
	m_hConnection = nullptr;
	InitNetlib();
}

CMaxProto::~CMaxProto()
{
	InterlockedExchange(&m_iTerminated, 1);
	ShutdownConnection();
	if (m_hNetlibUser != nullptr)
		Netlib_CloseHandle(m_hNetlibUser);
}

MWindow CMaxProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ACCMGRUI), hwndParent, MaxAccountProc, (LPARAM)this);
}

void CMaxProto::InitNetlib()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

INT_PTR CMaxProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_ADDSEARCHRES;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)L"Max ID";
	}
	return 0;
}

int CMaxProto::SetStatus(int iNewStatus)
{
	if (iNewStatus != ID_STATUS_OFFLINE && iNewStatus != ID_STATUS_ONLINE)
		iNewStatus = ID_STATUS_ONLINE;

	if (iNewStatus == m_iStatus)
		return 0;

	int iOldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		InterlockedExchange(&m_iTerminated, 1);
		ShutdownConnection();
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		return 0;
	}

	InterlockedExchange(&m_iTerminated, 0);
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	ForkThread(&CMaxProto::WorkerThread);
	return 0;
}

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

bool CMaxProto::ReadExact(void *buf, int cbSize)
{
	char *p = (char*)buf;
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
	case JSON_NUMBER: {
		CMStringW value = node.as_mstring();
		if (value.Find('.') != -1) {
			return cmp_write_double(ctx, wcstod(value, nullptr));
		}
		return cmp_write_integer(ctx, _wtoi64(value));
	}
	case JSON_STRING: {
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
	case CMP_TYPE_STR32: {
		uint32_t len = obj.as.str_size;
		ptrA tmp((char*)mir_alloc(len + 1));
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
	case CMP_TYPE_ARRAY32: {
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
	case CMP_TYPE_MAP32: {
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
	if (Netlib_Send(m_hConnection, (const char*)header, sizeof(header), MSG_NODUMP) != sizeof(header))
		return false;

	if (payload.length() > 0 && Netlib_Send(m_hConnection, (const char*)payload.data(), payload.length(), MSG_NODUMP) != (int)payload.length())
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
		if (!DecodePayload((const uint8_t*)payload.data(), payloadLen, json)) {
			// Mobile API may return compressed payloads (LZ4). For MVP, keep protocol alive even if payload decode failed.
			Netlib_Logf(m_hNetlibUser, "Max: payload decode failed for opcode=%u seq=%u (compressed=%u), using empty payload", frame.opcode, frame.seq, compressed);
			json = JSONNode(JSON_NODE);
		}
	}

	frame.payload = json;
	return true;
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
	dbei.pBlob = (char*)utf8;
	dbei.cbBlob = (uint32_t)mir_strlen(utf8);
	dbei.eventType = EVENTTYPE_MESSAGE;
	ProtoChainRecvMsg(hContact, dbei);
}

int CMaxProto::SendMsg(MCONTACT hContact, MEVENT, const char *msg)
{
	unsigned int id = InterlockedIncrement(&m_msgId);
	if (m_iStatus != ID_STATUS_ONLINE) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id);
		return id;
	}

	int64_t chatId = getDword(hContact, MAX_SETTINGS_CHAT_ID, 0);
	if (chatId == 0)
		chatId = getDword(hContact, MAX_SETTINGS_ID, 0);

	if (chatId == 0) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id);
		return id;
	}

	JSONNode message(JSON_NODE), req(JSON_NODE), elements(JSON_ARRAY), attaches(JSON_ARRAY);
	message << CHAR_PARAM("text", msg);
	message << INT64_PARAM("cid", (INT64)time(nullptr) * 1000);
	elements.set_name("elements");
	attaches.set_name("attaches");
	message << elements;
	message << attaches;
	message << CHAR_PARAM("link", "");
	message.set_name("message");

	req << INT64_PARAM("chatId", chatId);
	req << message;
	req << BOOL_PARAM("notify", 1);

	JSONNode resp;
	if (!SendAndWait(64, req, resp, 0)) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id);
		return id;
	}

	ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)id);
	return id;
}

HANDLE CMaxProto::SearchBasic(const wchar_t *id)
{
	auto *arg = mir_wstrdup(id);
	ForkThread(&CMaxProto::SearchThread, arg);
	return arg;
}

void CMaxProto::SearchThread(void *arg)
{
	wchar_t *phone = (wchar_t*)arg;
	ptrA utf8(mir_utf8encodeW(phone));

	JSONNode req(JSON_NODE), resp;
	req << CHAR_PARAM("phone", utf8);
	bool ok = SendAndWait(46, req, resp, 0);

	if (ok) {
		PROTOSEARCHRESULT psr = {};
		psr.cbSize = sizeof(psr);
		psr.flags = PSR_UNICODE;
		CMStringW idText;
		if (resp["contact"]["id"])
			idText.Format(L"%lld", (int64_t)resp["contact"]["id"].as_int());
		else
			idText = phone;

		psr.id.w = mir_wstrdup(idText);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, arg, (LPARAM)&psr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		mir_free(psr.id.w);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, arg, 0);

	mir_free(phone);
}

MCONTACT CMaxProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr == nullptr || psr->id.w == nullptr)
		return 0;

	uint32_t maxId = (uint32_t)_wtoi(psr->id.w);
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	if (flags & PALF_TEMPORARY) {
		Contact::Hide(hContact);
		Contact::RemoveFromList(hContact);
	}
	else {
		Contact::Hide(hContact, false);
		Contact::PutOnList(hContact);
	}

	setDword(hContact, MAX_SETTINGS_ID, maxId);
	setWString(hContact, "Nick", psr->id.w);

	JSONNode req(JSON_NODE), resp;
	req << INT64_PARAM("contactId", maxId);
	req << WCHAR_PARAM("firstName", psr->id.w);
	req << CHAR_PARAM("action", "ADD");
	SendAndWait(34, req, resp, 0);
	return hContact;
}
