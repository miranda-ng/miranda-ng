/*
Copyright (c) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.
*/

#include "stdafx.h"
#include "m_history.h"

static INT_PTR CALLBACK MaxAccMgrProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

struct CMaxMsgAckCtx
{
	CMaxProto *pProto = nullptr;
	MCONTACT hContact = 0;
	int result = ACKRESULT_FAILED;
	int hProcess = 0;
	CMStringA msgId;
	CMStringW errText;
};

/////////////////////////////////////////////////////////////////////////////////////////

CMaxProto::CMaxProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CMaxProto>(szModuleName, ptszUserName)
{
	m_hWaitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	HookProtoEvent(ME_OPT_INITIALISE, &CMaxProto::OnOptionsInit);

	NETLIBUSER nlu = {};
	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	CMStringW descr;
	descr.Format(TranslateT("%s connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	m_hProtoIcon = g_plugin.getIconHandle(IDI_MAIN);

	if (getWStringA(DB_KEY_DEFAULT_GROUP) == nullptr)
		setWString(DB_KEY_DEFAULT_GROUP, L"Max");
	Clist_GroupCreate(0, GetDefaultGroupW());
	RegisterChatModule();

	CreateProtoService(PS_GETAVATARINFO, &CMaxProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CMaxProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CMaxProto::SvcGetMyAvatar);
	CreateProtoService(PS_MENU_LOADHISTORY, &CMaxProto::SvcLoadServerHistory);
	CreateProtoService(PS_CAN_EMPTY_HISTORY, &CMaxProto::SvcCanEmptyHistory);
	CreateProtoService(PS_EMPTY_SRV_HISTORY, &CMaxProto::SvcEmptyServerHistory);
}

CMaxProto::~CMaxProto()
{
	DisconnectGateway();
	FreeWsInflater();
	if (m_hWaitEvent)
		CloseHandle(m_hWaitEvent);
}

INT_PTR CMaxProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSG | PF1_CHAT | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_SERVERCLIST;

	case PFLAGNUM_2:
		return PF2_ONLINE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_AVATARS | PF4_SERVERMSGID | PF4_DELETEFORALL;

	case PFLAG_UNIQUEIDTEXT:
	{
		static wchar_t s_wszUid[96];
		mir_wstrcpy(s_wszUid, TranslateT("Phone number (international, e.g. +79001234567)"));
		return (INT_PTR)s_wszUid;
	}
	}
	return 0;
}

int CMaxProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == ID_STATUS_INVISIBLE)
		iNewStatus = ID_STATUS_ONLINE;
	if (iNewStatus != ID_STATUS_OFFLINE && iNewStatus != ID_STATUS_ONLINE)
		iNewStatus = ID_STATUS_ONLINE;

	int iOldStatus = m_iStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		DisconnectGateway();
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		return 0;
	}

	// No stored session: stay offline until the user pastes a web login token.
	if (iNewStatus == ID_STATUS_ONLINE && !HasLoginToken()) {
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		NotifyUser(TranslateT("Max"), TranslateT("Sign in first: paste the login token from the web client (browser) in account settings, then try again."));
		return 0;
	}

	if (m_hConnThread)
		return 0;

	m_bTerminated = false;
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	m_hConnThread = ForkThreadEx(&CMaxProto::ConnectionWorker, nullptr, nullptr);
	if (!m_hConnThread)
		return 1;

	iOldStatus = m_iStatus;
	m_iStatus = iNewStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	return 0;
}

int CMaxProto::SendMsg(MCONTACT hContact, MEVENT, const char *msg)
{
	if (hContact == 0 || msg == nullptr || msg[0] == 0)
		return 0;

	int hProcess = (int)InterlockedIncrement(&m_iSendMsgSeq);
	if (hProcess <= 0)
		hProcess = 1;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		auto *ctx = new CMaxMsgAckCtx;
		ctx->pProto = this;
		ctx->hContact = hContact;
		ctx->result = ACKRESULT_FAILED;
		ctx->hProcess = hProcess;
		ctx->errText = TranslateT("Gateway is not connected");
		ForkThread(&CMaxProto::MessageAckWorker, ctx);
		return hProcess;
	}

	CMStringA chatId(GetOrResolveDialogChatId(hContact));
	if (chatId.IsEmpty()) {
		auto *ctx = new CMaxMsgAckCtx;
		ctx->pProto = this;
		ctx->hContact = hContact;
		ctx->result = ACKRESULT_FAILED;
		ctx->hProcess = hProcess;
		ctx->errText = TranslateT("Missing chat id for this contact");
		ForkThread(&CMaxProto::MessageAckWorker, ctx);
		return hProcess;
	}

	CMStringA serverMsgId;
	if (!ApiSendMessage(m_pGateway, chatId.c_str(), msg, &serverMsgId)) {
		CMStringW err = FormatLastError();
		if (!err.IsEmpty())
			NotifyUser(TranslateT("Max"), err.c_str());
		auto *ctx = new CMaxMsgAckCtx;
		ctx->pProto = this;
		ctx->hContact = hContact;
		ctx->result = ACKRESULT_FAILED;
		ctx->hProcess = hProcess;
		ctx->errText = TranslateT("Message send failed");
		ForkThread(&CMaxProto::MessageAckWorker, ctx);
		return hProcess;
	}

	debugLogA("Max: send ok chat=%s id=%s", chatId.c_str(), serverMsgId.IsEmpty() ? "(none)" : serverMsgId.c_str());
	auto *ctx = new CMaxMsgAckCtx;
	ctx->pProto = this;
	ctx->hContact = hContact;
	ctx->result = ACKRESULT_SUCCESS;
	ctx->hProcess = hProcess;
	ctx->msgId = serverMsgId;
	ForkThread(&CMaxProto::MessageAckWorker, ctx);
	return hProcess;
}

void CMaxProto::OnEventEdited(MCONTACT hContact, MEVENT, const DBEVENTINFO &dbei)
{
	if (hContact == 0 || dbei.szId == nullptr || dbei.pBlob == nullptr)
		return;
	if (!(dbei.flags & DBEF_SENT))
		return;

	CMStringA chatId(GetOrResolveDialogChatId(hContact));
	if (chatId.IsEmpty())
		return;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		debugLogA("Max: edit skipped (gateway not connected) chat=%s msg=%s", chatId.c_str(), dbei.szId);
		return;
	}

	if (!ApiEditMessage(m_pGateway, chatId.c_str(), dbei.szId, dbei.pBlob))
		debugLogA("Max: edit failed chat=%s msg=%s", chatId.c_str(), dbei.szId);
	else
		debugLogA("Max: edit ok chat=%s msg=%s", chatId.c_str(), dbei.szId);
}

void CMaxProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags)
{
	if (hContact == 0 || !(flags & CDF_DEL_HISTORY))
		return;

	DB::EventInfo dbei(hDbEvent, false);
	if (!dbei || mir_strcmp(dbei.szModule, m_szModuleName))
		return;
	if (dbei.eventType != EVENTTYPE_MESSAGE)
		return;
	if (dbei.szId == nullptr || dbei.szId[0] == 0)
		return;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		debugLogA("Max: delete msg skipped (gateway not connected) id=%s", dbei.szId);
		return;
	}

	CMStringA chatId;
	if (isChatRoom(hContact)) {
		SESSION_INFO *si = Chat_Find(hContact, m_szModuleName);
		if (si == nullptr || si->ptszID == nullptr || si->ptszID[0] == 0)
			return;
		ptrA cidUtf(mir_u2a(si->ptszID));
		if (cidUtf == nullptr || cidUtf[0] == 0)
			return;
		chatId = cidUtf.get();
	}
	else
		chatId = GetOrResolveDialogChatId(hContact, false);

	if (chatId.IsEmpty()) {
		debugLogA("Max: delete msg skipped (no chat id) msg=%s", dbei.szId);
		return;
	}

	const bool forEveryone = (flags & CDF_FOR_EVERYONE) != 0;
	const bool forMe = !forEveryone;

	if (!ApiDeleteMessages(m_pGateway, chatId.c_str(), dbei.szId, forMe))
		debugLogA("Max: delete msg API failed chat=%s id=%s forMe=%d", chatId.c_str(), dbei.szId, forMe ? 1 : 0);
	else
		debugLogA("Max: delete msg ok chat=%s id=%s forMe=%d", chatId.c_str(), dbei.szId, forMe ? 1 : 0);
}

void __cdecl CMaxProto::MessageAckWorker(void *param)
{
	std::unique_ptr<CMaxMsgAckCtx> ctx((CMaxMsgAckCtx *)param);
	if (!ctx || ctx->pProto == nullptr)
		return;

	// Give SRMM time to register msgQueue entry after SendMsg returns.
	Sleep(120);
	if (ctx->result == ACKRESULT_SUCCESS)
		ctx->pProto->ProtoBroadcastAck(ctx->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ctx->hProcess,
			ctx->msgId.IsEmpty() ? 0 : (LPARAM)ctx->msgId.c_str());
	else
		ctx->pProto->ProtoBroadcastAck(ctx->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)ctx->hProcess,
			(LPARAM)ctx->errText.c_str());
}

struct CMaxPhoneSearchCtx
{
	CMaxProto *p = nullptr;
	wchar_t *wszPhone = nullptr;
	/// Must match the handle returned from SearchBasic (Find/Add matches ACKDATA::hProcess to it).
	HANDLE hSearch = nullptr;
};

static CMStringA MaxJsonIdStrLocal(const JSONNode &n)
{
	if (n.type() == JSON_NULL)
		return "";
	if (n.type() == JSON_NUMBER) {
		CMStringA s;
		s.Format("%.0f", n.as_float());
		return s;
	}
	return CMStringA(n.as_string().c_str());
}

static bool MaxNormalizePhoneWtoUtf8(const wchar_t *in, CMStringA &outUtf8)
{
	if (in == nullptr || in[0] == 0)
		return false;

	CMStringW digits;
	bool hadLeadingPlus = false;
	for (const wchar_t *p = in; *p; ++p) {
		if ((*p == L'+' || *p == 0xFF0B) && digits.IsEmpty()) { // fullwidth plus
			hadLeadingPlus = true;
			continue;
		}
		if (*p >= L'0' && *p <= L'9')
			digits.AppendChar(*p);
	}

	if (digits.IsEmpty())
		return false;

	CMStringW normalized;
	if (hadLeadingPlus)
		normalized.Format(L"+%s", digits.c_str());
	else if (digits.GetLength() == 11 && digits[0] == L'8') {
		CMStringW tail8 = digits.Mid(1);
		normalized.Format(L"+7%s", tail8.c_str());
	}
	else if (digits.GetLength() == 10)
		normalized.Format(L"+7%s", digits.c_str());
	else
		normalized.Format(L"+%s", digits.c_str());

	ptrA utf(mir_utf8encodeW(normalized));
	if (utf == nullptr || utf[0] == 0)
		return false;
	outUtf8 = utf.get();
	return true;
}

void __cdecl CMaxProto::PhoneSearchWorker(void *param)
{
	auto *ctx = (CMaxPhoneSearchCtx *)param;
	if (ctx == nullptr)
		return;

	CMaxProto *p = ctx->p;
	const HANDLE hSearch = ctx->hSearch;
	ptrW wszPhone(ctx->wszPhone);
	mir_free(ctx);

	if (p == nullptr)
		return;

	CMStringA phoneUtf8;
	if (!MaxNormalizePhoneWtoUtf8(wszPhone, phoneUtf8)) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	if (!p->WaitForGatewayReady() || p->m_pGateway == nullptr) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	JSONNode contact(JSON_NULL);
	if (!p->ApiSearchByPhone(p->m_pGateway, phoneUtf8.c_str(), contact)) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	if (contact.type() != JSON_NODE || contact.size() == 0) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch, 0);
		return;
	}

	CMStringA uid = MaxJsonIdStrLocal(contact["id"]);
	if (uid.IsEmpty()) uid = MaxJsonIdStrLocal(contact["contactId"]);
	if (uid.IsEmpty()) uid = MaxJsonIdStrLocal(contact["userId"]);
	if (uid.IsEmpty()) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	CMStringW fn, ln;
	p->FillNameFromMaxContactJson(contact, fn, ln);

	CMStringW nick;
	if (!fn.IsEmpty() || !ln.IsEmpty()) {
		nick = fn;
		if (!ln.IsEmpty()) {
			if (!nick.IsEmpty()) nick += L' ';
			nick += ln;
		}
	}
	if (nick.IsEmpty())
		nick.Format(L"User %S", uid.c_str());

	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;

	CMStringW uidw;
	uidw.Format(L"%S", uid.c_str());
	psr.id.w = mir_wstrdup(uidw);
	psr.firstName.w = mir_wstrdup(fn.c_str());
	psr.lastName.w = mir_wstrdup(ln.c_str());
	psr.nick.w = mir_wstrdup(nick.c_str());
	psr.email.w = mir_wstrdup(L"");

	p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, hSearch, (LPARAM)&psr);
	p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch, 0);

	mir_free(psr.id.w);
	mir_free(psr.firstName.w);
	mir_free(psr.lastName.w);
	mir_free(psr.nick.w);
	mir_free(psr.email.w);
}

HANDLE CMaxProto::SearchBasic(const wchar_t *id)
{
	if (id == nullptr || id[0] == 0)
		return nullptr;
	if (m_iStatus == ID_STATUS_OFFLINE)
		return nullptr;

	auto *ctx = (CMaxPhoneSearchCtx *)mir_alloc(sizeof(CMaxPhoneSearchCtx));
	if (ctx == nullptr)
		return nullptr;
	ctx->p = this;
	ctx->wszPhone = mir_wstrdup(id);
	if (ctx->wszPhone == nullptr) {
		mir_free(ctx);
		return nullptr;
	}
	ctx->hSearch = (HANDLE)1;
	ForkThread(&CMaxProto::PhoneSearchWorker, ctx);
	return ctx->hSearch;
}

MCONTACT CMaxProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr == nullptr || psr->cbSize < (int)sizeof(PROTOSEARCHRESULT))
		return 0;
	if (!(psr->flags & PSR_UNICODE) || psr->id.w == nullptr || psr->id.w[0] == 0)
		return 0;

	ptrA uidUtf(mir_utf8encodeW(psr->id.w));
	if (uidUtf == nullptr || uidUtf[0] == 0)
		return 0;

	CMStringA chatId;
	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my != nullptr && my[0]) {
		uint64_t a = _strtoui64(my, nullptr, 10);
		uint64_t b = _strtoui64(uidUtf, nullptr, 10);
		if (b != 0) {
			unsigned long long x = (unsigned long long)(a ^ b);
			chatId.Format("%llu", x);
		}
	}

	const wchar_t *pfn = (psr->firstName.w && psr->firstName.w[0]) ? psr->firstName.w : L"";
	const wchar_t *pln = (psr->lastName.w && psr->lastName.w[0]) ? psr->lastName.w : L"";

	MCONTACT hContact = EnsureUserContact(uidUtf, pfn, pln, chatId.IsEmpty() ? nullptr : chatId.c_str());
	if (hContact == 0)
		return 0;

	setByte(hContact, DB_KEY_MAX_PEER_ORIGIN, MAX_PEER_ORIGIN_CONTACTS);

	if (flags & PALF_TEMPORARY)
		Contact::RemoveFromList(hContact);

	if (WaitForGatewayReady() && m_pGateway != nullptr) {
		if (!ApiAddContactOnServer(m_pGateway, uidUtf))
			debugLogA("Max: AddToList server opcode 34 failed or timed out uid=%s", uidUtf.get());
	}

	return hContact;
}

void CMaxProto::OnModulesLoaded()
{
	HookProtoEvent(ME_USERINFO_INITIALISE, &CMaxProto::OnUserInfoInit);
}

bool CMaxProto::ContactNeedsServerDisplayFetch(MCONTACT hContact)
{
	if (hContact == 0 || isChatRoom(hContact))
		return false;
	CMStringW fn = getMStringW(hContact, "FirstName");
	CMStringW ln = getMStringW(hContact, "LastName");
	if (!fn.IsEmpty() && fn.GetLength() >= 5 && !_wcsnicmp(fn.c_str(), L"User ", 5))
		return true;
	return fn.IsEmpty() && ln.IsEmpty();
}

namespace
{
	struct CMaxLiveNotifCtx
	{
		CMaxProto *pProto = nullptr;
		JSONNode *pPayload = nullptr;
	};
}

void CMaxProto::QueueLiveNotifIngest(const JSONNode &payload)
{
	auto *ctx = new CMaxLiveNotifCtx();
	if (ctx == nullptr)
		return;
	ctx->pProto = this;
	ctx->pPayload = new JSONNode(payload);
	if (ctx->pPayload == nullptr) {
		delete ctx;
		return;
	}
	ForkThread(&CMaxProto::LiveNotifIngestWorker, ctx);
}

void __cdecl CMaxProto::LiveNotifIngestWorker(void *param)
{
	auto *ctx = (CMaxLiveNotifCtx *)param;
	if (ctx == nullptr || ctx->pProto == nullptr || ctx->pPayload == nullptr) {
		if (ctx) {
			delete ctx->pPayload;
			delete ctx;
		}
		return;
	}

	CMaxProto *p = ctx->pProto;
	JSONNode *pPl = ctx->pPayload;
	ctx->pPayload = nullptr;
	delete ctx;

	const JSONNode &payload = *pPl;

	p->SyncLiveDialogFromPushPayload(payload);

	const JSONNode &msg = payload["message"];
	if (msg.type() != JSON_NODE) {
		delete pPl;
		return;
	}

	CMStringA sender;
	{
		const JSONNode &sn = msg["sender"];
		if (sn.type() == JSON_NULL)
			sender.Empty();
		else if (sn.type() == JSON_NUMBER) {
			sender.Format("%.0f", sn.as_float());
		}
		else
			sender = sn.as_string().c_str();
	}

	CMStringA chatId;
	{
		const JSONNode &cn = payload["chatId"];
		if (cn.type() == JSON_NUMBER)
			chatId.Format("%.0f", cn.as_float());
		else if (cn.type() == JSON_STRING)
			chatId = cn.as_string().c_str();
	}
	if (chatId.IsEmpty()) {
		delete pPl;
		return;
	}

	if (!p->m_bTerminated && p->WaitForGatewayReady() && p->m_pGateway != nullptr) {
		MCONTACT hPeer = p->ResolveContactForDialogMessage(chatId.c_str(), sender.IsEmpty() ? nullptr : sender.c_str());
		if (hPeer && p->ContactNeedsServerDisplayFetch(hPeer)) {
			ptrA uid(getStringA(hPeer, DB_KEY_MAX_UID));
			if (uid != nullptr && uid[0]) {
				CMStringA one[1] = { uid.get() };
				p->ApiFetchContactsBatch(p->m_pGateway, one, 1, false);
			}
		}
	}

	p->IngestMaxMessageJson(msg, chatId.c_str());
	delete pPl;
}

static uint64_t sttJsonTimeMs(const JSONNode &n)
{
	if (n.type() == JSON_NUMBER)
		return (uint64_t)(n.as_float() + 0.5);
	if (n.type() == JSON_STRING)
		return _strtoui64(n.as_string().c_str(), nullptr, 10);
	return 0;
}

static const JSONNode *sttHistoryMessagesArray(const JSONNode &payload)
{
	if (payload["messages"].type() == JSON_ARRAY)
		return &payload["messages"];
	if (payload["chat"].type() == JSON_NODE && payload["chat"]["messages"].type() == JSON_ARRAY)
		return &payload["chat"]["messages"];
	return nullptr;
}

void __cdecl CMaxProto::LoadHistoryWorker(void *param)
{
	MCONTACT hContact = (MCONTACT)(UINT_PTR)param;
	if (hContact == 0)
		return;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot load server history: gateway is not connected."));
		return;
	}

	CMStringA chatId(GetOrResolveDialogChatId(hContact));
	if (chatId.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot load server history: chat id is missing for this contact."));
		return;
	}

	int totalLoaded = 0;
	int64_t fromMs = (int64_t)time(nullptr) * 1000;
	uint64_t prevOldest = 0;

	const int kBatch = 100;
	const DWORD kInterPageDelayMs = 200;
	const int kMaxRetry = 4;
	int page = 0;
	bool reachedBottom = false;
	bool abortedByError = false;
	const char *stopReason = "unknown";

	while (!m_bTerminated) {
		if (!ApiFetchChatMessages(m_pGateway, chatId.c_str(), fromMs, 0, kBatch, true)) {
			CMStringW err = FormatLastError();
			ptrA err8(mir_utf8encodeW(err));
			bool throttled = false;
			if (!err.IsEmpty()) {
				CMStringW low(err);
				low.MakeLower();
				if (low.Find(L"limit") >= 0 || low.Find(L"too many") >= 0 || low.Find(L"rate") >= 0)
					throttled = true;
			}

			bool okAfterRetry = false;
			for (int retry = 1; retry <= kMaxRetry && !m_bTerminated; ++retry) {
				DWORD waitMs = throttled ? (DWORD)(1000 * (1 << (retry - 1))) : (DWORD)(600 * retry);
				debugLogA("Max: load history retry chat=%s page=%d retry=%d/%d wait=%u err=%s",
					chatId.c_str(), page, retry, kMaxRetry, (unsigned)waitMs,
					(err8 != nullptr && err8[0]) ? err8.get() : "(empty)");
				InterruptibleSleepMs(waitMs);

				if (ApiFetchChatMessages(m_pGateway, chatId.c_str(), fromMs, 0, kBatch, true)) {
					okAfterRetry = true;
					break;
				}

				err = FormatLastError();
				err8 = mir_utf8encodeW(err);
			}

			if (!okAfterRetry) {
				abortedByError = true;
				stopReason = throttled ? "server-throttle" : "server-error";
				debugLogA("Max: load history aborted chat=%s page=%d reason=%s err=%s",
					chatId.c_str(), page, stopReason,
					(err8 != nullptr && err8[0]) ? err8.get() : "(empty)");
				break;
			}
		}

		JSONNode resp = JSONNode::parse(m_szPendingResponse.c_str());
		if (!resp) {
			abortedByError = true;
			stopReason = "bad-json";
			debugLogA("Max: load history aborted chat=%s page=%d reason=bad-json", chatId.c_str(), page);
			break;
		}

		const JSONNode &pl = resp["payload"];
		const JSONNode *msgs = sttHistoryMessagesArray(pl);
		if (msgs == nullptr || msgs->type() != JSON_ARRAY) {
			abortedByError = true;
			stopReason = "bad-payload";
			debugLogA("Max: load history aborted chat=%s page=%d reason=bad-payload", chatId.c_str(), page);
			break;
		}

		if (msgs->size() == 0) {
			reachedBottom = true;
			stopReason = "empty-page";
			break;
		}

		totalLoaded += (int)msgs->size();

		uint64_t oldest = 0;
		for (unsigned i = 0; i < msgs->size(); ++i) {
			uint64_t t = sttJsonTimeMs((*msgs)[i]["time"]);
			if (t != 0 && (oldest == 0 || t < oldest))
				oldest = t;
		}

		if (oldest == 0) {
			abortedByError = true;
			stopReason = "missing-time";
			debugLogA("Max: load history aborted chat=%s page=%d reason=missing-time", chatId.c_str(), page);
			break;
		}

		if (oldest == prevOldest) {
			reachedBottom = true;
			stopReason = "stuck-oldest";
			break;
		}

		prevOldest = oldest;
		if (oldest <= 1) {
			reachedBottom = true;
			stopReason = "reached-zero";
			break;
		}

		if ((int)msgs->size() < kBatch) {
			reachedBottom = true;
			stopReason = "short-page";
			break;
		}

		fromMs = (int64_t)(oldest - 1);
		page++;
		InterruptibleSleepMs(kInterPageDelayMs);
	}

	if (m_bTerminated && !reachedBottom && !abortedByError)
		stopReason = "terminated";

	if (abortedByError && page == 0) {
		if (!mir_strcmp(stopReason, "server-error") || !mir_strcmp(stopReason, "bad-json")
		    || !mir_strcmp(stopReason, "bad-payload"))
			RemoveLocalPeerIfChatOnly(hContact);
	}

	debugLogA("Max: load history done chat=%s fetched=%d reason=%s", chatId.c_str(), totalLoaded, stopReason);
	if (reachedBottom)
		History::FinishLoad(hContact);
}

INT_PTR CMaxProto::SvcLoadServerHistory(WPARAM hContact, LPARAM)
{
	if ((MCONTACT)hContact == 0)
		return 1;

	ForkThread(&CMaxProto::LoadHistoryWorker, (void *)(UINT_PTR)hContact);
	return 0;
}

INT_PTR CMaxProto::SvcCanEmptyHistory(WPARAM hContact, LPARAM)
{
	MCONTACT h = (MCONTACT)hContact;
	if (h == 0)
		return 0;

	if (isChatRoom(h)) {
		SESSION_INFO *si = Chat_Find(h, m_szModuleName);
		return (si != nullptr && si->ptszID != nullptr && si->ptszID[0] != 0) ? 1 : 0;
	}

	CMStringA chatId = GetOrResolveDialogChatId(h, false);
	return chatId.IsEmpty() ? 0 : 1;
}

INT_PTR CMaxProto::SvcEmptyServerHistory(WPARAM hContact, LPARAM lParam)
{
	MCONTACT h = (MCONTACT)hContact;
	if (h == 0 || (lParam & CDF_DEL_HISTORY) == 0)
		return 0;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot remove server chat: not connected."));
		return 0;
	}

	if (isChatRoom(h)) {
		SESSION_INFO *si = Chat_Find(h, m_szModuleName);
		if (si == nullptr || si->ptszID == nullptr || si->ptszID[0] == 0)
			return 0;
		ptrA chatUtf(mir_u2a(si->ptszID));
		if (chatUtf == nullptr)
			return 0;
		if (ApiChatLeave(m_pGateway, chatUtf)) {
			debugLogA("Max: empty history — server leave (opcode 58) chat=%s", chatUtf.get());
			delSetting(h, DB_KEY_MAX_CHATID);
		}
		else
			NotifyUser(TranslateT("Max"), TranslateT("Could not leave the chat on the server."));
		return 0;
	}

	CMStringA chatId = GetOrResolveDialogChatId(h, false);
	if (chatId.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot remove server chat: dialog id is unknown."));
		return 0;
	}

	if (ApiDeleteServerDialog(m_pGateway, chatId.c_str())) {
		debugLogA("Max: empty history — server dialog delete (opcode 52) chat=%s", chatId.c_str());
		delSetting(h, DB_KEY_MAX_CHATID);
	}
	else
		NotifyUser(TranslateT("Max"), TranslateT("Could not delete the dialog on the server."));
	return 0;
}

void CMaxProto::OnShutdown(void)
{
	DisconnectGateway();
}

bool CMaxProto::OnContactDeleted(MCONTACT hContact, uint32_t flags)
{
	if (!(flags & CDF_DEL_CONTACT))
		return true;
	if (hContact == 0)
		return true;

	if (!WaitForGatewayReady() || m_pGateway == nullptr)
		return true;

	if (isChatRoom(hContact)) {
		if (SESSION_INFO *si = Chat_Find(hContact, m_szModuleName))
			if (si->ptszID != nullptr && si->ptszID[0]) {
				ptrA chatUtf(mir_u2a(si->ptszID));
				if (chatUtf && ApiChatLeave(m_pGateway, chatUtf))
					debugLogA("Max: server left group/channel (opcode 58) id=%s", chatUtf.get());
				else if (chatUtf)
					debugLogA("Max: server leave group failed id=%s", chatUtf.get());
			}
		return true;
	}

	CMStringA chatId = GetOrResolveDialogChatId(hContact, false);
	if (chatId.IsEmpty()) {
		debugLogA("Max: OnContactDeleted (server) skipped — no chat id h=%u", (unsigned)hContact);
		return true;
	}

	ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
	// Always try opcode 34 REMOVE when we have uid: MaxPeerOrigin can be CHATONLY while the peer is still
	// in the server address book (official client). REMOVE is cheap; SendJsonAndWait(..., true) tolerates errors.
	if (uid != nullptr && uid[0]) {
		if (ApiRemoveContactFromServer(m_pGateway, uid))
			debugLogA("Max: removed uid=%s from server address book (opcode 34)", uid.get());
		else
			debugLogA("Max: opcode 34 REMOVE finished with error/timeout uid=%s (dialog delete still attempted)", uid.get());
	}

	if (ApiDeleteServerDialog(m_pGateway, chatId.c_str()))
		debugLogA("Max: deleted dialog on server (opcode 52) chatId=%s", chatId.c_str());
	else
		debugLogA("Max: delete dialog on server failed chatId=%s", chatId.c_str());

	return true;
}

void CMaxProto::DisconnectGateway()
{
	const bool hadSession = (m_hConnThread != nullptr || m_hWsRunThread != nullptr || m_pGateway != nullptr);
	if (hadSession) {
		debugLogA("Connection terminated, exiting");
		debugLogA("CMaxProto::OnLoggedOut");
	}

	m_bTerminated = true;
	m_bAvatarWebPrimed = false;
	if (m_pGateway)
		m_pGateway->terminate();

	if (m_hWsRunThread) {
		WaitForSingleObject(m_hWsRunThread, 15000);
		CloseHandle(m_hWsRunThread);
		m_hWsRunThread = nullptr;
	}

	if (m_hConnThread) {
		WaitForSingleObject(m_hConnThread, 15000);
		CloseHandle(m_hConnThread);
		m_hConnThread = nullptr;
	}
	m_pGateway = nullptr;
	m_wsRun.ws = nullptr;
	m_bGatewayConnected = false;
	FreeWsInflater();
	ResetServerContactBookCache();
}

MWindow CMaxProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ACCMGRUI), hwndParent, MaxAccMgrProc, (LPARAM)this);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK MaxAccMgrProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMaxProto *ppro = (CMaxProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ppro = (CMaxProto *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		Window_SetIcon_IcoLib(hwndDlg, ppro->m_hProtoIcon);
		SetDlgItemTextA(hwndDlg, IDC_LOGIN_TOKEN, ppro->getMStringA(DB_KEY_LOGIN_TOKEN));
		SetDlgItemTextW(hwndDlg, IDC_GROUPNAME, ppro->GetDefaultGroupW());
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE && (LOWORD(wParam) == IDC_LOGIN_TOKEN || LOWORD(wParam) == IDC_GROUPNAME))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			char tok[8192];
			GetDlgItemTextA(hwndDlg, IDC_LOGIN_TOKEN, tok, _countof(tok));
			ppro->setString(DB_KEY_LOGIN_TOKEN, tok);

			wchar_t wgrp[128];
			GetDlgItemTextW(hwndDlg, IDC_GROUPNAME, wgrp, _countof(wgrp));
			if (wgrp[0]) {
				ppro->setWString(DB_KEY_DEFAULT_GROUP, wgrp);
				Clist_GroupCreate(0, wgrp);
			}
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMaxProto::SaveMyProfile(const wchar_t *pwszFirstName, const wchar_t *pwszLastName, const wchar_t *pwszBio)
{
	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		NotifyUser(TranslateT("Max"), TranslateT("You must be online to save your profile."));
		return false;
	}

	const wchar_t *fnIn = (pwszFirstName != nullptr) ? pwszFirstName : L"";
	CMStringW fnTrim(fnIn);
	fnTrim.Trim();
	if (fnTrim.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("First name cannot be empty."));
		return false;
	}

	const wchar_t *lnIn = (pwszLastName != nullptr) ? pwszLastName : L"";
	CMStringW lnTrim(lnIn);
	lnTrim.Trim();

	const wchar_t *bioIn = (pwszBio != nullptr) ? pwszBio : L"";

	ptrA fn8(mir_utf8encodeW(fnTrim.c_str()));
	ptrA ln8;
	if (!lnTrim.IsEmpty())
		ln8 = mir_utf8encodeW(lnTrim.c_str());
	ptrA bio8(mir_utf8encodeW(bioIn));

	const char *pLast = (ln8 != nullptr && ln8[0]) ? ln8.get() : nullptr;
	if (!ApiUpdateMyProfile(m_pGateway, fn8, pLast, bio8)) {
		CMStringW err = FormatLastError();
		if (!err.IsEmpty())
			NotifyUser(TranslateT("Max"), err.c_str());
		else
			NotifyUser(TranslateT("Max"), TranslateT("Could not save your profile on the server."));
		return false;
	}

	setWString("FirstName", fnTrim.c_str());
	if (lnTrim.IsEmpty())
		delSetting("LastName");
	else
		setWString("LastName", lnTrim.c_str());

	if (bioIn[0] == 0)
		delSetting("About");
	else
		setWString("About", bioIn);
	return true;
}

CMStringW CMaxProto::FormatLastError()
{
	if (m_szPendingResponse.IsEmpty())
		return L"";
	JSONNode root = JSONNode::parse(m_szPendingResponse.c_str());
	if (!root)
		return L"";
	const JSONNode &pl = root["payload"];
	if (pl["localizedMessage"].type() == JSON_STRING && !pl["localizedMessage"].as_string().empty()) {
		ptrW w(mir_utf8decodeW(pl["localizedMessage"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	if (pl["title"].type() == JSON_STRING && !pl["title"].as_string().empty()) {
		ptrW w(mir_utf8decodeW(pl["title"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	const JSONNode &er = pl["error"];
	if (er.type() == JSON_STRING) {
		CMStringA ecode(er.as_string().c_str());
		if (!mir_strcmp(ecode.c_str(), "error.limit.violate"))
			return TranslateT("Too many requests. Please wait before trying again.");
		if (!mir_strcmp(ecode.c_str(), "login.cred"))
			return TranslateT("Server rejected web sync credentials for this token (login.cred).");

		ptrW w(mir_utf8decodeW(ecode.c_str()));
		return CMStringW((const wchar_t*)w);
	}
	if (er.type() == JSON_NODE && er["message"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(er["message"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	if (pl["message"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(pl["message"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	return L"";
}

void CMaxProto::NotifyUser(const wchar_t *title, const wchar_t *text)
{
	if (Miranda_IsTerminated())
		return;

	// Always mirror user-facing text to netlog (popups may be disabled).
	ptrA title8(mir_utf8encodeW(title ? title : L""));
	ptrA text8(mir_utf8encodeW(text ? text : L""));
	debugLogA("Max: %s — %s",
		(title8 && title8[0]) ? title8.get() : "(no title)",
		(text8 && text8[0]) ? text8.get() : "(no text)");

	if (Popup_Enabled()) {
		POPUPDATAW ppd;
		ppd.lchContact = NULL;
		wcsncpy_s(ppd.lpwzContactName, title, _TRUNCATE);
		wcsncpy_s(ppd.lpwzText, text, _TRUNCATE);
		ppd.lchIcon = g_plugin.getIcon(IDI_MAIN);
		if (PUAddPopupW(&ppd))
			return;
	}
}

int CMaxProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.szTab.w = LPGENW("Account");
	odp.dwInitParam = (LPARAM)this;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_ACCMGRUI);
	odp.pfnDlgProc = MaxAccMgrProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

bool CMaxProto::HasLoginToken()
{
	ptrA t(getStringA(DB_KEY_LOGIN_TOKEN));
	return t != nullptr && t[0] != 0;
}

bool CMaxProto::WaitForGatewayReady()
{
	if (Miranda_IsTerminated())
		return false;

	if (m_hConnThread) {
		if (WaitForSingleObject(m_hConnThread, 0) == WAIT_OBJECT_0) {
			CloseHandle(m_hConnThread);
			m_hConnThread = nullptr;
		}
	}

	if (!m_hConnThread)
		SetStatus(ID_STATUS_ONLINE);

	for (int i = 0; i < 300; i++) {
		if (Miranda_IsTerminated())
			return false;
		if (m_bGatewayConnected && m_pGateway != nullptr)
			return true;
		Sleep(100);
	}

	return m_bGatewayConnected && m_pGateway != nullptr;
}
