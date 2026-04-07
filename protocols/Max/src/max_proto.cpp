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
		return PF1_IM | PF1_MODEMSG | PF1_CHAT;

	case PFLAGNUM_2:
		return PF2_ONLINE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_AVATARS | PF4_SERVERMSGID;

	case PFLAG_UNIQUEIDTEXT:
	{
		static wchar_t s_wszUid[64];
		mir_wstrcpy(s_wszUid, TranslateT("Max user ID"));
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
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hProcess, (LPARAM)TranslateT("Gateway is not connected"));
		return hProcess;
	}

	CMStringA chatId(getMStringA(hContact, DB_KEY_MAX_CHATID));
	if (chatId.IsEmpty())
		chatId = getMStringA(hContact, DB_KEY_MAX_UID);

	if (chatId.IsEmpty()) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hProcess, (LPARAM)TranslateT("Missing chat id for this contact"));
		return hProcess;
	}

	CMStringA serverMsgId;
	if (!ApiSendMessage(m_pGateway, chatId.c_str(), msg, &serverMsgId)) {
		CMStringW err = FormatLastError();
		if (!err.IsEmpty())
			NotifyUser(TranslateT("Max"), err.c_str());
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hProcess, (LPARAM)TranslateT("Message send failed"));
		return hProcess;
	}

	debugLogA("Max: send ok chat=%s id=%s", chatId.c_str(), serverMsgId.IsEmpty() ? "(none)" : serverMsgId.c_str());
	ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)hProcess, 0);
	return hProcess;
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

	CMStringA chatId(getMStringA(hContact, DB_KEY_MAX_CHATID));
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

void CMaxProto::OnShutdown(void)
{
	DisconnectGateway();
}

void CMaxProto::DisconnectGateway()
{
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
