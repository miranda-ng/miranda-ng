#include "stdafx.h"

static bool MaxJsonNodeToInt64(const JSONNode &n, int64_t &out)
{
	if (!n)
		return false;
	switch (n.type()) {
	case JSON_NUMBER:
		out = (int64_t)n.as_int();
		return true;
	case JSON_STRING:
		out = _wtoi64(n.as_mstring());
		return true;
	default:
		return false;
	}
}

static bool MaxTryExtractChatIdFromRpcPayload(const JSONNode &payload, int64_t &out)
{
	if (MaxJsonNodeToInt64(payload["chatId"], out) && out != 0)
		return true;
	if (payload["chat"] && MaxJsonNodeToInt64(payload["chat"]["id"], out) && out != 0)
		return true;
	if (payload["contact"] && MaxJsonNodeToInt64(payload["contact"]["chatId"], out) && out != 0)
		return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

int64_t CMaxProto::GetContactChatId(MCONTACT hContact)
{
	CMStringA s(getMStringA(hContact, MAX_SETTINGS_CHAT_ID));
	if (!s.IsEmpty())
		return _atoi64(s);

	uint32_t legacy = getDword(hContact, MAX_SETTINGS_CHAT_ID, 0);
	if (legacy != 0)
		return (int64_t)(int32_t)legacy;

	return 0;
}

void CMaxProto::SetContactChatId(MCONTACT hContact, int64_t chatId)
{
	if (hContact == 0 || chatId == 0)
		return;

	CMStringA s;
	s.Format("%lld", (long long)chatId);
	setString(hContact, MAX_SETTINGS_CHAT_ID, s);
}

/////////////////////////////////////////////////////////////////////////////////////////

CMaxProto::CMaxProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CMaxProto>(szModuleName, ptszUserName),
	m_szToken(this, MAX_SETTINGS_TOKEN),
	m_szDeviceId(this, MAX_SETTINGS_DEVICE_ID)
{
	// init netlib
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

CMaxProto::~CMaxProto()
{
	InterlockedExchange(&m_iTerminated, 1);
	StopWorker(true);
	ShutdownConnection();
	if (m_hNetlibUser != nullptr)
		Netlib_CloseHandle(m_hNetlibUser);
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
		StopWorker(false);
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		return 0;
	}

	InterlockedExchange(&m_iTerminated, 0);
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	if (m_hWorkerThread != nullptr) {
		if (WaitForSingleObject(m_hWorkerThread, 0) == WAIT_OBJECT_0) {
			CloseHandle(m_hWorkerThread);
			m_hWorkerThread = nullptr;
		}
	}
	if (m_hWorkerThread == nullptr)
		m_hWorkerThread = ForkThreadEx(&CMaxProto::WorkerThread, nullptr, nullptr);
	return 0;
}

int CMaxProto::SendMsg(MCONTACT hContact, MEVENT, const char *msg)
{
	unsigned int id = InterlockedIncrement(&m_msgId);
	if (m_iStatus != ID_STATUS_ONLINE) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id);
		return id;
	}

	int64_t chatId = GetContactChatId(hContact);
	if (chatId == 0) {
		uint32_t uid = getDword(hContact, MAX_SETTINGS_ID, 0);
		if (uid != 0) {
			chatId = (int64_t)(int32_t)uid;
			Netlib_Logf(m_hNetlibUser, "Max: send: no ChatID for contact, using MaxID %lld as fallback", (long long)chatId);
		}
	}

	if (chatId == 0) {
		Netlib_Logf(m_hNetlibUser, "Max: send: neither ChatID nor MaxID for contact");
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

/////////////////////////////////////////////////////////////////////////////////////////

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

		int64_t chatId = 0;
		if (MaxTryExtractChatIdFromRpcPayload(resp, chatId)) {
			CMStringW wcid;
			wcid.Format(L"%lld", (long long)chatId);
			psr.email.w = mir_wstrdup(wcid);
		}

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, arg, (LPARAM)&psr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		mir_free(psr.id.w);
		if (psr.email.w)
			mir_free(psr.email.w);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, arg, 0);

	mir_free(phone);
}

////////////////////////////////////////////////////////////////////////////////////////

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

	if (psr->email.w && psr->email.w[0]) {
		int64_t fromSearch = _wtoi64(psr->email.w);
		if (fromSearch != 0)
			SetContactChatId(hContact, fromSearch);
	}

	JSONNode req(JSON_NODE), resp;
	req << INT64_PARAM("contactId", maxId);
	req << WCHAR_PARAM("firstName", psr->id.w);
	req << CHAR_PARAM("action", "ADD");
	if (SendAndWait(34, req, resp, 0)) {
		int64_t chatId = 0;
		if (MaxTryExtractChatIdFromRpcPayload(resp, chatId))
			SetContactChatId(hContact, chatId);
	}
	return hContact;
}
