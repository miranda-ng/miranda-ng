#include "stdafx.h"

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
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, arg, (LPARAM)&psr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		mir_free(psr.id.w);
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

	JSONNode req(JSON_NODE), resp;
	req << INT64_PARAM("contactId", maxId);
	req << WCHAR_PARAM("firstName", psr->id.w);
	req << CHAR_PARAM("action", "ADD");
	SendAndWait(34, req, resp, 0);
	return hContact;
}
