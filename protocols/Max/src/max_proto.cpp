/*
Copyright (c) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.
*/

#include "stdafx.h"

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
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON;

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

	// No stored session: stay offline (SMS/CHECK_CODE uses the mobile API only).
	if (iNewStatus == ID_STATUS_ONLINE && !HasLoginToken()) {
		m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		NotifyUser(TranslateT("Max"), TranslateT("Sign in first: enter the SMS code (or paste a login token) in account settings, then try again."));
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

int CMaxProto::SendMsg(MCONTACT, MEVENT, const char *)
{
	return 0;
}

void CMaxProto::OnShutdown(void)
{
	DisconnectGateway();
}

void CMaxProto::DisconnectGateway()
{
	m_bTerminated = true;
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
		SetDlgItemTextA(hwndDlg, IDC_PHONE, ppro->getMStringA(DB_KEY_PHONE));
		SetDlgItemTextW(hwndDlg, IDC_GROUPNAME, ppro->GetDefaultGroupW());
		SetDlgItemTextW(hwndDlg, IDC_BTN_REQUEST_SMS, TranslateT("Request SMS"));
		SetDlgItemTextW(hwndDlg, IDC_BTN_VERIFY_SMS, TranslateT("Confirm code"));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BTN_REQUEST_SMS:
			if (HIWORD(wParam) == BN_CLICKED) {
				char szPhone[64];
				GetDlgItemTextA(hwndDlg, IDC_PHONE, szPhone, _countof(szPhone));
				if (szPhone[0] == 0) {
					ppro->NotifyUser(TranslateT("Max"), TranslateT("Enter phone number."));
					break;
				}
				ppro->setString(DB_KEY_PHONE, szPhone);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				ppro->ForkThread(&CMaxProto::RequestSmsThread, mir_strdup(szPhone));
			}
			break;

		case IDC_BTN_VERIFY_SMS:
			if (HIWORD(wParam) == BN_CLICKED) {
				char szCode[32];
				GetDlgItemTextA(hwndDlg, IDC_SMSCODE, szCode, _countof(szCode));
				if (szCode[0] == 0) {
					ppro->NotifyUser(TranslateT("Max"), TranslateT("Enter the SMS code."));
					break;
				}
				ppro->ForkThread(&CMaxProto::VerifySmsThread, mir_strdup(szCode));
			}
			break;

		default:
			if (HIWORD(wParam) == EN_CHANGE && (LOWORD(wParam) == IDC_PHONE || LOWORD(wParam) == IDC_SMSCODE || LOWORD(wParam) == IDC_GROUPNAME))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			char buf[512];
			GetDlgItemTextA(hwndDlg, IDC_PHONE, buf, _countof(buf));
			ppro->setString(DB_KEY_PHONE, buf);

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
			return TranslateT("Too many SMS/code requests. Please wait before trying again.");
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

void __cdecl CMaxProto::RequestSmsThread(void *param)
{
	ptrA phone((char *)param);
	if (phone == nullptr || phone[0] == 0) {
		NotifyUser(TranslateT("Max"), TranslateT("Phone number is empty."));
		return;
	}

	CMStringW errText;
	if (!MobileStartAuth(phone, errText)) {
		if (errText.IsEmpty())
			errText = TranslateT("Failed to request SMS.");
		NotifyUser(TranslateT("Max"), errText.c_str());
		return;
	}

	NotifyUser(TranslateT("Max"), TranslateT("SMS request sent. Enter the code from the message and press Confirm code."));
}

void __cdecl CMaxProto::VerifySmsThread(void *param)
{
	ptrA code((char *)param);
	if (code == nullptr || code[0] == 0) {
		NotifyUser(TranslateT("Max"), TranslateT("SMS code is empty."));
		return;
	}

	CMStringA loginToken;
	CMStringW errText;
	if (!MobileCheckCode(code, loginToken, errText)) {
		if (errText.IsEmpty())
			errText = TranslateT("Wrong code or login failed.");
		NotifyUser(TranslateT("Max"), errText.c_str());
		return;
	}
	if (loginToken.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("Login token missing in helper response."));
		return;
	}
	setString(DB_KEY_LOGIN_TOKEN, loginToken.c_str());

	// Force a single reconnect so ConnectionWorker performs exactly one opcode 19 sync
	// with the new token (avoids double sync with VerifySmsThread + worker).
	DisconnectGateway();

	if (!WaitForGatewayReady()) {
		NotifyUser(TranslateT("Max"), TranslateT("Mobile auth passed, but web session is not connected."));
		return;
	}

	if (m_bInitialSyncOk) {
		NotifyUser(TranslateT("Max"), TranslateT("Signed in successfully."));
		return;
	}

	CMStringW err = FormatLastError();
	if (err.IsEmpty())
		NotifyUser(TranslateT("Max"), TranslateT("Code accepted, but sync failed. Try again later."));
	else
		NotifyUser(TranslateT("Max"), err.c_str());
}
