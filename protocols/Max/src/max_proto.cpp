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

	CreateProtoService(PS_GETAVATARINFO, &CMaxProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CMaxProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CMaxProto::SvcGetMyAvatar);
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

	int hProcess = (int)(GetTickCount() & 0x7fffffff);
	if (hProcess == 0)
		hProcess = 1;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hProcess, (LPARAM)TranslateT("Gateway is not connected"));
		return hProcess;
	}

	CMStringA chatId(getMStringA(hContact, DB_KEY_MAX_CHATID));
	if (chatId.IsEmpty())
		chatId = getMStringA(hContact, DB_KEY_MAX_UID);

	if (chatId.IsEmpty()) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hProcess, (LPARAM)TranslateT("Missing chat id for this contact"));
		return hProcess;
	}

	CMStringA serverMsgId;
	if (!ApiSendMessage(m_pGateway, chatId.c_str(), msg, &serverMsgId)) {
		CMStringW err = FormatLastError();
		if (err.IsEmpty())
			err = TranslateT("Message send failed");
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hProcess, (LPARAM)err.c_str());
		return hProcess;
	}

	debugLogA("Max: send ok chat=%s id=%s", chatId.c_str(), serverMsgId.IsEmpty() ? "(none)" : serverMsgId.c_str());
	// msgQueue_processack uses lParam as server message id to update outgoing event.
	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)hProcess,
		serverMsgId.IsEmpty() ? 0 : (LPARAM)serverMsgId.c_str());
	return hProcess;
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
