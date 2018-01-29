/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-18 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

#include <windns.h>   // requires Windows Platform SDK

#include "jabber_list.h"
#include "jabber_iq.h"
#include "jabber_secur.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_rc.h"
#include "jabber_proto.h"

#ifndef DNS_TYPE_SRV
#define DNS_TYPE_SRV 0x0021
#endif

// <iq/> identification number for various actions
// for JABBER_REGISTER thread
int iqIdRegGetReg;
int iqIdRegSetReg;

// XML Console
#define JCPF_IN      0x01UL
#define JCPF_OUT     0x02UL
#define JCPF_ERROR   0x04UL

static VOID CALLBACK JabberDummyApcFunc(DWORD_PTR)
{
	return;
}

struct JabberPasswordDlgParam
{
	CJabberProto *pro;

	BOOL   saveOnlinePassword;
	WORD   dlgResult;
	wchar_t  onlinePassword[128];
	HANDLE hEventPasswdDlg;
	wchar_t *ptszJid;
};

static INT_PTR CALLBACK JabberPasswordDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberPasswordDlgParam* param = (JabberPasswordDlgParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		param = (JabberPasswordDlgParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		wchar_t text[512];
		mir_snwprintf(text, TranslateT("Enter password for %s"), param->ptszJid);
		SetDlgItemText(hwndDlg, IDC_JID, text);

		CheckDlgButton(hwndDlg, IDC_SAVEPASSWORD, param->pro->getByte("SaveSessionPassword", 0) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SAVE_PERM:
			EnableWindow(GetDlgItem(hwndDlg, IDC_SAVEPASSWORD), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SAVE_PERM));
			break;
		case IDOK:
			param->saveOnlinePassword = IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD);
			param->pro->setByte("SaveSessionPassword", param->saveOnlinePassword);

			GetDlgItemText(hwndDlg, IDC_PASSWORD, param->onlinePassword, _countof(param->onlinePassword));
			{
				BOOL savePassword = IsDlgButtonChecked(hwndDlg, IDC_SAVE_PERM);
				param->pro->setByte("SavePassword", savePassword);
				if (savePassword) {
					param->pro->setWString("Password", param->onlinePassword);
					param->saveOnlinePassword = TRUE;
				}
			}
			// Fall through
		case IDCANCEL:
			param->dlgResult = LOWORD(wParam);
			SetEvent(param->hEventPasswdDlg);
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static VOID CALLBACK JabberPasswordCreateDialogApcProc(void* param)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PASSWORD), nullptr, JabberPasswordDlgProc, (LPARAM)param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Jabber keep-alive thread

void CJabberProto::OnPingReply(HXML, CJabberIqInfo *pInfo)
{
	if (!pInfo)
		return;
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_FAIL) {
		// disconnect because of timeout
		m_ThreadInfo->send("</stream:stream>");
		m_ThreadInfo->shutdown();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareDNS(const DNS_SRV_DATAA* dns1, const DNS_SRV_DATAA* dns2)
{
	return (int)dns1->wPriority - (int)dns2->wPriority;
}

void ThreadData::xmpp_client_query(void)
{
	if (inet_addr(conn.server) != INADDR_NONE)
		return;

	char temp[256];
	mir_snprintf(temp, "_xmpp-client._tcp.%s", conn.server);

	DNS_RECORDA *results = nullptr;
	DNS_STATUS status = DnsQuery_A(temp, DNS_TYPE_SRV, DNS_QUERY_STANDARD, nullptr, (PDNS_RECORD *)&results, nullptr);
	if (SUCCEEDED(status) && results) {
		LIST<DNS_SRV_DATAA> dnsList(5, CompareDNS);

		for (DNS_RECORDA *rec = results; rec; rec = rec->pNext)
			if (rec->Data.Srv.pNameTarget && rec->wType == DNS_TYPE_SRV)
				dnsList.insert(&rec->Data.Srv);

		for (int i = 0; i < dnsList.getCount(); i++) {
			WORD dnsPort = (conn.port == 0 || conn.port == 5222) ? dnsList[i]->wPort : conn.port;
			char* dnsHost = dnsList[i]->pNameTarget;

			proto->debugLogA("%s%s resolved to %s:%d", "_xmpp-client._tcp.", conn.server, dnsHost, dnsPort);
			s = proto->WsConnect(dnsHost, dnsPort);
			if (s) {
				strncpy_s(conn.manualHost, dnsHost, _TRUNCATE);
				conn.port = dnsPort;
				break;
			}
		}
		DnsRecordListFree(results, DnsFreeRecordList);
	}
	else proto->debugLogA("%s not resolved", temp);
}

void CJabberProto::xmlStreamInitialize(char *szWhich)
{
	debugLogA("Stream will be initialized %s", szWhich);
	if (m_szXmlStreamToBeInitialized)
		free(m_szXmlStreamToBeInitialized);
	m_szXmlStreamToBeInitialized = _strdup(szWhich);
}

void CJabberProto::xmlStreamInitializeNow(ThreadData *info)
{
	debugLogA("Stream is initializing %s",
		m_szXmlStreamToBeInitialized ? m_szXmlStreamToBeInitialized : "after connect");
	if (m_szXmlStreamToBeInitialized) {
		free(m_szXmlStreamToBeInitialized);
		m_szXmlStreamToBeInitialized = nullptr;
	}

	HXML n = xmlCreateNode(L"xml", nullptr, 1) << XATTR(L"version", L"1.0") << XATTR(L"encoding", L"UTF-8");

	HXML stream = n << XCHILDNS(L"stream:stream", L"jabber:client") << XATTR(L"to", _A2T(info->conn.server))
		<< XATTR(L"xmlns:stream", L"http://etherx.jabber.org/streams");

	if (m_tszSelectedLang)
		XmlAddAttr(stream, L"xml:lang", m_tszSelectedLang);

	if (!m_options.Disable3920auth)
		XmlAddAttr(stream, L"version", L"1.0");

	LPTSTR xmlQuery = xmlToString(n, nullptr);
	T2Utf buf(xmlQuery);
	int bufLen = (int)mir_strlen(buf);
	if (bufLen > 2) {
		strdel((char*)buf + bufLen - 2, 1);
		bufLen--;
	}

	info->send(buf, bufLen);
	xmlFree(xmlQuery);
	xmlDestroyNode(n);
}

void CJabberProto::ServerThread(JABBER_CONN_DATA *pParam)
{
	ptrW tszValue;

	ThreadData info(this, pParam);

	debugLogA("Thread started: type=%d", info.bIsReg);
	Thread_SetName("Jabber: ServerThread");

	if (m_options.ManualConnect == TRUE) {
		ptrA szManualHost(getStringA("ManualHost"));
		if (szManualHost != nullptr)
			strncpy_s(info.conn.manualHost, szManualHost, _TRUNCATE);

		info.conn.port = getWord("ManualPort", JABBER_DEFAULT_PORT);
	}
	else info.conn.port = getWord("Port", JABBER_DEFAULT_PORT);

	info.conn.useSSL = m_options.UseSSL;

	if (!info.bIsReg) {
		// Normal server connection, we will fetch all connection parameters
		// e.g. username, password, etc. from the database.
		if (m_ThreadInfo != nullptr) {
			debugLogA("Thread ended, another normal thread is running");
			return;
		}

		m_ThreadInfo = &info;

		if ((tszValue = getWStringA("LoginName")) != nullptr)
			wcsncpy_s(info.conn.username, tszValue, _TRUNCATE);

		if (*rtrimw(info.conn.username) == '\0') {
			DWORD dwSize = _countof(info.conn.username);
			if (GetUserName(info.conn.username, &dwSize))
				setWString("LoginName", info.conn.username);
			else
				info.conn.username[0] = 0;
		}

		if (*rtrimw(info.conn.username) == '\0') {
			debugLogA("Thread ended, login name is not configured");
			JLoginFailed(LOGINERR_BADUSERID);

LBL_FatalError:
			int oldStatus = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
			return;
		}

		ptrA szValue(getStringA("LoginServer"));
		if (szValue != nullptr)
			strncpy_s(info.conn.server, szValue, _TRUNCATE);
		else {
			ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
			debugLogA("Thread ended, login server is not configured");
			goto LBL_FatalError;
		}

		if (m_options.HostNameAsResource) {
			DWORD dwCompNameLen = _countof(info.resource) - 1;
			if (!GetComputerName(info.resource, &dwCompNameLen))
				mir_wstrcpy(info.resource, L"Miranda");
		}
		else {
			if ((tszValue = getWStringA("Resource")) != nullptr)
				wcsncpy_s(info.resource, tszValue, _TRUNCATE);
			else
				mir_wstrcpy(info.resource, L"Miranda");
		}

		wchar_t jidStr[512];
		mir_snwprintf(jidStr, L"%s@%S/%s", info.conn.username, info.conn.server, info.resource);
		wcsncpy_s(info.fullJID, jidStr, _TRUNCATE);

		if (m_options.UseDomainLogin) // in the case of NTLM auth we have no need in password
			info.conn.password[0] = 0;
		else if (!m_options.SavePassword) { // we have to enter a password manually. have we done it before?
			if (m_savedPassword != nullptr)
				wcsncpy_s(info.conn.password, m_savedPassword, _TRUNCATE);
			else {
				mir_snwprintf(jidStr, L"%s@%S", info.conn.username, info.conn.server);

				JabberPasswordDlgParam param;
				param.pro = this;
				param.ptszJid = jidStr;
				param.hEventPasswdDlg = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				CallFunctionAsync(JabberPasswordCreateDialogApcProc, &param);
				WaitForSingleObject(param.hEventPasswdDlg, INFINITE);
				CloseHandle(param.hEventPasswdDlg);

				if (param.dlgResult == IDCANCEL) {
					JLoginFailed(LOGINERR_BADUSERID);
					debugLogA("Thread ended, password request dialog was canceled");
					goto LBL_FatalError;
				}

				m_savedPassword = (param.saveOnlinePassword) ? mir_wstrdup(param.onlinePassword) : nullptr;
				wcsncpy_s(info.conn.password, param.onlinePassword, _TRUNCATE);
			}
		}
		else {
			ptrW tszPassw(getWStringA(0, "Password"));
			if (tszPassw == nullptr) {
				JLoginFailed(LOGINERR_BADUSERID);
				debugLogA("Thread ended, password is not configured");
				goto LBL_FatalError;
			}
			wcsncpy_s(info.conn.password, tszPassw, _TRUNCATE);
		}
	}
	else {
		// Register new user connection, all connection parameters are already filled-in.
		// Multiple thread allowed, although not possible :)
		// thinking again.. multiple thread should not be allowed
		info.reg_done = false;
		SendMessage(info.conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 25, (LPARAM)TranslateT("Connecting..."));
		iqIdRegGetReg = -1;
		iqIdRegSetReg = -1;
	}

	int jabberNetworkBufferSize = 2048;
	if ((info.buffer = (char*)mir_alloc(jabberNetworkBufferSize + 1)) == nullptr) {	// +1 is for '\0' when debug logging this buffer
		debugLogA("Cannot allocate network buffer, thread ended");
		if (info.bIsReg)
			SendMessage(info.conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Error: Not enough memory"));
		else
			ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);

		debugLogA("Thread ended, network buffer cannot be allocated");
		goto LBL_FatalError;
	}

	if (info.conn.manualHost[0] == 0) {
		info.xmpp_client_query();
		if (info.s == nullptr) {
			strncpy_s(info.conn.manualHost, info.conn.server, _TRUNCATE);
			info.s = WsConnect(info.conn.manualHost, info.conn.port);
		}
	}
	else info.s = WsConnect(info.conn.manualHost, info.conn.port);

	debugLogA("Thread type=%d server='%s' port='%d'", info.bIsReg, info.conn.manualHost, info.conn.port);
	if (info.s == nullptr) {
		debugLogA("Connection failed (%d)", WSAGetLastError());
		if (!info.bIsReg) {
			if (m_ThreadInfo == &info)
				ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
		}
		else SendMessage(info.conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Error: Cannot connect to the server"));

		debugLogA("Thread ended, connection failed");
		goto LBL_FatalError;
	}

	// Determine local IP
	if (info.conn.useSSL) {
		debugLogA("Intializing SSL connection");
		if (!Netlib_StartSsl(info.s, nullptr)) {
			debugLogA("SSL intialization failed");
			if (!info.bIsReg)
				ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
			else
				SendMessage(info.conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Error: Cannot connect to the server"));

			info.close();
			debugLogA("Thread ended, SSL connection failed");
			goto LBL_FatalError;
	}	}

	// User may change status to OFFLINE while we are connecting above
	if (m_iDesiredStatus != ID_STATUS_OFFLINE || info.bIsReg) {
		if (!info.bIsReg) {
			size_t len = mir_wstrlen(info.conn.username) + mir_strlen(info.conn.server) + 1;
			m_szJabberJID = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			mir_snwprintf(m_szJabberJID, len + 1, L"%s@%S", info.conn.username, info.conn.server);
			m_bSendKeepAlive = m_options.KeepAlive != 0;
			setWString("jid", m_szJabberJID); // store jid in database

			ListInit();
		}

		xmlStreamInitializeNow(&info);
		const wchar_t *tag = L"stream:stream";

		debugLogA("Entering main recv loop");
		int datalen = 0;

		// cache values
		DWORD dwConnectionKeepAliveInterval = m_options.ConnectionKeepAliveInterval;
		for (;;) {
			if (!info.useZlib || info.zRecvReady) {
				DWORD dwIdle = GetTickCount() - m_lastTicks;
				if (dwIdle >= dwConnectionKeepAliveInterval)
					dwIdle = dwConnectionKeepAliveInterval - 10; // now!

				NETLIBSELECT nls = {};
				nls.dwTimeout = dwConnectionKeepAliveInterval - dwIdle;
				nls.hReadConns[0] = info.s;
				int nSelRes = Netlib_Select(&nls);
				if (nSelRes == -1) // error
					break;
				else if (nSelRes == 0 && m_bSendKeepAlive) {
					if (info.jabberServerCaps & JABBER_CAPS_PING) {
						CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnPingReply, JABBER_IQ_TYPE_GET, nullptr, 0, -1, this);
						pInfo->SetTimeout(m_options.ConnectionKeepAliveTimeout);
						info.send(XmlNodeIq(pInfo) << XATTR(L"from", info.fullJID) << XCHILDNS(L"ping", JABBER_FEAT_PING));
					}
					else info.send(" \t ");
					continue;
			}	}

			int recvResult = info.recv(info.buffer + datalen, jabberNetworkBufferSize - datalen);
			debugLogA("recvResult = %d", recvResult);
			if (recvResult <= 0)
				break;
			datalen += recvResult;

recvRest:
			info.buffer[datalen] = '\0';

			ptrW str(mir_utf8decodeW(info.buffer));

			int bytesParsed = 0;
			XmlNode root(str, &bytesParsed, tag);
			if (root && tag) {
				char *p = strstr(info.buffer, "stream:stream");
				if (p) p = strchr(p, '>');
				if (p)
					bytesParsed = p - info.buffer + 1;
				else {
					root = XmlNode();
					bytesParsed = 0;
				}
			}
			else {
				if (root)
					str[bytesParsed] = 0;
				bytesParsed = (root) ? mir_utf8lenW(str) : 0;
			}

			debugLogA("bytesParsed = %d", bytesParsed);
			if (root) tag = nullptr;

			if (XmlGetName(root) == nullptr) {
				for (int i = 0;; i++) {
					HXML n = XmlGetChild(root, i);
					if (!n)
						break;
					OnProcessProtocol(n, &info);
				}
			}
			else OnProcessProtocol(root, &info);

			if (bytesParsed > 0) {
				if (bytesParsed < datalen)
					memmove(info.buffer, info.buffer + bytesParsed, datalen - bytesParsed);
				datalen -= bytesParsed;
			}
			else if (datalen >= jabberNetworkBufferSize) {
				//jabberNetworkBufferSize += 65536;
				jabberNetworkBufferSize *= 2;
				debugLogA("Increasing network buffer size to %d", jabberNetworkBufferSize);
				if ((info.buffer = (char*)mir_realloc(info.buffer, jabberNetworkBufferSize + 1)) == nullptr) {
					debugLogA("Cannot reallocate more network buffer, go offline now");
					break;
			}	}
			else debugLogA("Unknown state: bytesParsed=%d, datalen=%d, jabberNetworkBufferSize=%d", bytesParsed, datalen, jabberNetworkBufferSize);

			if (m_szXmlStreamToBeInitialized) {
				xmlStreamInitializeNow(&info);
				tag = L"stream:stream";
			}
			if (root && datalen)
				goto recvRest;
		}

		if (!info.bIsReg) {
			m_iqManager.ExpireAll();
			m_bJabberOnline = false;
			info.zlibUninit();
			EnableMenuItems(false);
			RebuildInfoFrame();
			if (m_hwndJabberChangePassword)
				// Since this is a different thread, simulate the click on the cancel button instead
				SendMessage(m_hwndJabberChangePassword, WM_COMMAND, MAKEWORD(IDCANCEL, 0), 0);

			// Quit all chatrooms (will send quit message)
			LISTFOREACH(i, this, LIST_CHATROOM)
				if (JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i))
					GcQuit(item, 0, nullptr);

			ListRemoveList(LIST_CHATROOM);
			ListRemoveList(LIST_BOOKMARK);
			UI_SAFE_NOTIFY_HWND(m_hwndJabberAddBookmark, WM_JABBER_CHECK_ONLINE);
			WindowList_Broadcast(m_hWindowList, WM_JABBER_CHECK_ONLINE, 0, 0);

			// Set status to offline
			int oldStatus = m_iStatus;
			m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

			// Set all contacts to offline
			debugLogA("1");
			for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
				SetContactOfflineStatus(hContact);
			debugLogA("2");

			mir_free(m_szJabberJID);
			m_szJabberJID = nullptr;
			m_tmJabberLoggedInTime = 0;
			ListWipe();

			WindowList_Broadcast(m_hWindowList, WM_JABBER_REFRESH_VCARD, 0, 0);
		}
		else if (info.bIsReg && !info.reg_done)
			SendMessage(info.conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Error: Connection lost"));
	}
	else if (!info.bIsReg) {
		int oldStatus = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}

	debugLogA("Thread ended: type=%d server='%s'", info.bIsReg, info.conn.server);

	info.close();
	debugLogA("Exiting ServerThread");
}

void CJabberProto::PerformRegistration(ThreadData *info)
{
	iqIdRegGetReg = SerialNext();
	info->send(XmlNodeIq(L"get", iqIdRegGetReg, nullptr) << XQUERY(JABBER_FEAT_REGISTER));

	SendMessage(info->conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 50, (LPARAM)TranslateT("Requesting registration instruction..."));
}

void CJabberProto::PerformIqAuth(ThreadData *info)
{
	if (!info->bIsReg) {
		info->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetAuth, JABBER_IQ_TYPE_GET))
			<< XQUERY(L"jabber:iq:auth") << XCHILD(L"username", info->conn.username));
	}
	else PerformRegistration(info);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnProcessStreamOpening(HXML node, ThreadData *info)
{
	if (mir_wstrcmp(XmlGetName(node), L"stream:stream"))
		return;

	if (!info->bIsReg) {
		const wchar_t *sid = XmlGetAttrValue(node, L"id");
		if (sid != nullptr)
			info->szStreamId = mir_u2a(sid);
	}

	// old server - disable SASL then
	if (XmlGetAttrValue(node, L"version") == nullptr)
		info->proto->m_options.Disable3920auth = TRUE;

	if (info->proto->m_options.Disable3920auth)
		info->proto->PerformIqAuth(info);
}

void CJabberProto::PerformAuthentication(ThreadData *info)
{
	TJabberAuth* auth = nullptr;
	char* request = nullptr;

	if (info->auth) {
		delete info->auth;
		info->auth = nullptr;
	}

	if (m_AuthMechs.isSpnegoAvailable) {
		m_AuthMechs.isSpnegoAvailable = false;
		auth = new TNtlmAuth(info, "GSS-SPNEGO");
		if (!auth->isValid()) {
			delete auth;
			auth = nullptr;
	}	}

	if (auth == nullptr && m_AuthMechs.isNtlmAvailable) {
		m_AuthMechs.isNtlmAvailable = false;
		auth = new TNtlmAuth(info, "NTLM");
		if (!auth->isValid()) {
			delete auth;
			auth = nullptr;
	}	}

	if (auth == nullptr && m_AuthMechs.isKerberosAvailable) {
		m_AuthMechs.isKerberosAvailable = false;
		auth = new TNtlmAuth(info, "GSSAPI", m_AuthMechs.m_gssapiHostName);
		if (!auth->isValid()) {
			delete auth;
			auth = nullptr;
		}
		else {
			request = auth->getInitialRequest();
			if (!request) {
				delete auth;
				auth = nullptr;
	}	}	}

	if (auth == nullptr && m_AuthMechs.isScramAvailable) {
		m_AuthMechs.isScramAvailable = false;
		auth = new TScramAuth(info);
	}

	if (auth == nullptr && m_AuthMechs.isMd5Available) {
		m_AuthMechs.isMd5Available = false;
		auth = new TMD5Auth(info);
	}

	if (auth == nullptr && m_AuthMechs.isPlainAvailable) {
		m_AuthMechs.isPlainAvailable = false;
		auth = new TPlainAuth(info, false);
	}

	if (auth == nullptr && m_AuthMechs.isPlainOldAvailable) {
		m_AuthMechs.isPlainOldAvailable = false;
		auth = new TPlainAuth(info, true);
	}

	if (auth == nullptr) {
		if (m_AuthMechs.isAuthAvailable) { // no known mechanisms but iq_auth is available
			m_AuthMechs.isAuthAvailable = false;
			PerformIqAuth(info);
			return;
		}

		wchar_t text[1024];
		mir_snwprintf(text, TranslateT("Authentication failed for %s@%S."), info->conn.username, info->conn.server);
		MsgPopup(0, text, TranslateT("Jabber Authentication"));
		JLoginFailed(LOGINERR_WRONGPASSWORD);
		info->send("</stream:stream>");
		m_ThreadInfo = nullptr;
		return;
	}

	info->auth = auth;

	if (!request) request = auth->getInitialRequest();
	info->send(XmlNode(L"auth", _A2T(request)) << XATTR(L"xmlns", L"urn:ietf:params:xml:ns:xmpp-sasl")
		<< XATTR(L"mechanism", _A2T(auth->getName())));
	mir_free(request);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnProcessFeatures(HXML node, ThreadData *info)
{
	bool isRegisterAvailable = false;
	bool areMechanismsDefined = false;

	for (int i = 0;; i++) {
		HXML n = XmlGetChild(node, i);
		if (!n)
			break;

		if (!mir_wstrcmp(XmlGetName(n), L"starttls")) {
			if (!info->conn.useSSL && m_options.UseTLS) {
				debugLogA("Requesting TLS");
				info->send(XmlNode(XmlGetName(n)) << XATTR(L"xmlns", L"urn:ietf:params:xml:ns:xmpp-tls"));
				return;
		}	}

		if (!mir_wstrcmp(XmlGetName(n), L"compression") && m_options.EnableZlib == TRUE) {
			debugLogA("Server compression available");
			for (int k = 0;; k++) {
				HXML c = XmlGetChild(n, k);
				if (!c)
					break;

				if (!mir_wstrcmp(XmlGetName(c), L"method")) {
					if (!mir_wstrcmp(XmlGetText(c), L"zlib") && info->zlibInit() == TRUE) {
						debugLogA("Requesting Zlib compression");
						info->send(XmlNode(L"compress") << XATTR(L"xmlns", L"http://jabber.org/protocol/compress")
							<< XCHILD(L"method", L"zlib"));
						return;
		}	}	}	}

		if (!mir_wstrcmp(XmlGetName(n), L"mechanisms")) {
			m_AuthMechs.isPlainAvailable = false;
			m_AuthMechs.isPlainOldAvailable = false;
			m_AuthMechs.isMd5Available = false;
			m_AuthMechs.isScramAvailable = false;
			m_AuthMechs.isNtlmAvailable = false;
			m_AuthMechs.isSpnegoAvailable = false;
			m_AuthMechs.isKerberosAvailable = false;
			mir_free(m_AuthMechs.m_gssapiHostName); m_AuthMechs.m_gssapiHostName = nullptr;

			areMechanismsDefined = true;
			//JabberLog("%d mechanisms\n",n->numChild);
			for (int k = 0;; k++) {
				HXML c = XmlGetChild(n, k);
				if (!c)
					break;

				if (!mir_wstrcmp(XmlGetName(c), L"mechanism")) {
					LPCTSTR ptszMechanism = XmlGetText(c);
					if (!mir_wstrcmp(ptszMechanism, L"PLAIN"))        m_AuthMechs.isPlainOldAvailable = m_AuthMechs.isPlainAvailable = true;
					else if (!mir_wstrcmp(ptszMechanism, L"DIGEST-MD5"))   m_AuthMechs.isMd5Available = true;
					else if (!mir_wstrcmp(ptszMechanism, L"SCRAM-SHA-1"))  m_AuthMechs.isScramAvailable = true;
					else if (!mir_wstrcmp(ptszMechanism, L"NTLM"))         m_AuthMechs.isNtlmAvailable = true;
					else if (!mir_wstrcmp(ptszMechanism, L"GSS-SPNEGO"))   m_AuthMechs.isSpnegoAvailable = true;
					else if (!mir_wstrcmp(ptszMechanism, L"GSSAPI"))       m_AuthMechs.isKerberosAvailable = true;
				}
				else if (!mir_wstrcmp(XmlGetName(c), L"hostname")) {
					const wchar_t *mech = XmlGetAttrValue(c, L"mechanism");
					if (mech && mir_wstrcmpi(mech, L"GSSAPI") == 0) {
						m_AuthMechs.m_gssapiHostName = mir_wstrdup(XmlGetText(c));
					}
				}
			}
		}
		else if (!mir_wstrcmp(XmlGetName(n), L"register")) isRegisterAvailable = true;
		else if (!mir_wstrcmp(XmlGetName(n), L"auth")) m_AuthMechs.isAuthAvailable = true;
		else if (!mir_wstrcmp(XmlGetName(n), L"session")) m_AuthMechs.isSessionAvailable = true;
	}

	if (areMechanismsDefined) {
		if (info->bIsReg)
			PerformRegistration(info);
		else
			PerformAuthentication(info);
		return;
	}

	// mechanisms are not defined.
	if (info->auth) { //We are already logged-in
		info->send(
			XmlNodeIq(AddIQ(&CJabberProto::OnIqResultBind, JABBER_IQ_TYPE_SET))
				<< XCHILDNS(L"bind", L"urn:ietf:params:xml:ns:xmpp-bind")
				<< XCHILD(L"resource", info->resource));

		if (m_AuthMechs.isSessionAvailable)
			info->bIsSessionAvailable = TRUE;

		return;
	}

	//mechanisms not available and we are not logged in
	PerformIqAuth(info);
}

void CJabberProto::OnProcessFailure(HXML node, ThreadData *info)
{
	const wchar_t *type;
	//failure xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"
	if ((type = XmlGetAttrValue(node, L"xmlns")) == nullptr) return;
	if (!mir_wstrcmp(type, L"urn:ietf:params:xml:ns:xmpp-sasl")) {
		PerformAuthentication(info);
	}
}

void CJabberProto::OnProcessError(HXML node, ThreadData *info)
{
	//failure xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"
	if (!XmlGetChild(node, 0))
		return;

	bool skipMsg = false;
	CMStringW buff;
	for (int i = 0;; i++) {
		HXML n = XmlGetChild(node, i);
		if (!n)
			break;

		const wchar_t *name = XmlGetName(n);
		const wchar_t *desc = XmlGetText(n);
		if (desc)
			buff.AppendFormat(L"%s: %s\r\n", name, desc);
		else
			buff.AppendFormat(L"%s\r\n", name);

		if (!mir_wstrcmp(name, L"conflict"))
			ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_OTHERLOCATION);
		else if (!mir_wstrcmp(name, L"see-other-host")) {
			skipMsg = true;
		}
	}
	if (!skipMsg)
		MsgPopup(0, buff, TranslateT("Jabber Error"));

	info->send("</stream:stream>");
}

void CJabberProto::OnProcessSuccess(HXML node, ThreadData *info)
{
	const wchar_t *type;
	//	int iqId;
	// RECVED: <success ...
	// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
	if ((type = XmlGetAttrValue(node, L"xmlns")) == nullptr)
		return;

	if (!mir_wstrcmp(type, L"urn:ietf:params:xml:ns:xmpp-sasl")) {
		if (!info->auth->validateLogin(XmlGetText(node))) {
			info->send("</stream:stream>");
			return;
		}

		debugLogA("Success: Logged-in.");
		ptrW tszNick(getWStringA("Nick"));
		if (tszNick == nullptr)
			setWString("Nick", info->conn.username);

		xmlStreamInitialize("after successful sasl");
	}
	else debugLogW(L"Success: unknown action %s.", type);
}

void CJabberProto::OnProcessChallenge(HXML node, ThreadData *info)
{
	if (info->auth == nullptr) {
		debugLogA("No previous auth have been made, exiting...");
		return;
	}

	if (mir_wstrcmp(XmlGetAttrValue(node, L"xmlns"), L"urn:ietf:params:xml:ns:xmpp-sasl"))
		return;

	char* challenge = info->auth->getChallenge(XmlGetText(node));
	info->send(XmlNode(L"response", _A2T(challenge)) << XATTR(L"xmlns", L"urn:ietf:params:xml:ns:xmpp-sasl"));
	mir_free(challenge);
}

void CJabberProto::OnProcessProtocol(HXML node, ThreadData *info)
{
	OnConsoleProcessXml(node, JCPF_IN);

	if (!mir_wstrcmp(XmlGetName(node), L"proceed"))
		OnProcessProceed(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"compressed"))
		OnProcessCompressed(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"stream:features"))
		OnProcessFeatures(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"stream:stream"))
		OnProcessStreamOpening(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"success"))
		OnProcessSuccess(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"failure"))
		OnProcessFailure(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"stream:error"))
		OnProcessError(node, info);
	else if (!mir_wstrcmp(XmlGetName(node), L"challenge"))
		OnProcessChallenge(node, info);
	else if (!info->bIsReg) {
		if (!mir_wstrcmp(XmlGetName(node), L"message"))
			OnProcessMessage(node, info);
		else if (!mir_wstrcmp(XmlGetName(node), L"presence"))
			OnProcessPresence(node, info);
		else if (!mir_wstrcmp(XmlGetName(node), L"iq"))
			OnProcessIq(node);
		else
			debugLogA("Invalid top-level tag (only <message/> <presence/> and <iq/> allowed)");
	}
	else {
		if (!mir_wstrcmp(XmlGetName(node), L"iq"))
			OnProcessRegIq(node, info);
		else
			debugLogA("Invalid top-level tag (only <iq/> allowed)");
	}
}

void CJabberProto::OnProcessProceed(HXML node, ThreadData *info)
{
	const wchar_t *type;
	if ((type = XmlGetAttrValue(node, L"xmlns")) != nullptr && !mir_wstrcmp(type, L"error"))
		return;

	if (!mir_wstrcmp(type, L"urn:ietf:params:xml:ns:xmpp-tls")) {
		debugLogA("Starting TLS...");

		char* gtlk = strstr(info->conn.manualHost, "google.com");
		bool isHosted = gtlk && !gtlk[10] && mir_strcmpi(info->conn.server, "gmail.com") &&
			mir_strcmpi(info->conn.server, "googlemail.com");

		if (!Netlib_StartSsl(info->s, isHosted ? info->conn.manualHost : info->conn.server)) {
			debugLogA("SSL initialization failed");
			info->send("</stream:stream>");
			info->shutdown();
		}
		else
			xmlStreamInitialize("after successful StartTLS");
	}
}

void CJabberProto::OnProcessCompressed(HXML node, ThreadData *info)
{
	debugLogA("Compression confirmed");

	const wchar_t *type = XmlGetAttrValue(node, L"xmlns");
	if (type != nullptr && !mir_wstrcmp(type, L"error"))
		return;
	if (mir_wstrcmp(type, L"http://jabber.org/protocol/compress"))
		return;

	debugLogA("Starting Zlib stream compression...");

	info->useZlib = TRUE;
	info->zRecvData = (char*)mir_alloc(ZLIB_CHUNK_SIZE);

	xmlStreamInitialize("after successful Zlib init");
}

void CJabberProto::OnProcessPubsubEvent(HXML node)
{
	const wchar_t *from = XmlGetAttrValue(node, L"from");
	if (!from)
		return;

	HXML eventNode = XmlGetChildByTag(node, "event", "xmlns", JABBER_FEAT_PUBSUB_EVENT);
	if (!eventNode)
		return;

	m_pepServices.ProcessEvent(from, eventNode);

	MCONTACT hContact = HContactFromJID(from);
	if (!hContact)
		return;

	HXML itemsNode;
	if (m_options.UseOMEMO)
	{
		itemsNode = XmlGetChildByTag(eventNode, L"items", L"node", JABBER_FEAT_OMEMO L".devicelist");
		if (itemsNode)
		{
			OmemoHandleDeviceList(itemsNode);
			return;
		}
		//TODO:handle omemo device list
	}
	if (m_options.EnableUserTune && (itemsNode = XmlGetChildByTag(eventNode, "items", "node", JABBER_FEAT_USER_TUNE))) {
		// node retract?
		if (XmlGetChild(itemsNode, "retract")) {
			SetContactTune(hContact, nullptr, nullptr, nullptr, nullptr, nullptr);
			return;
		}

		HXML tuneNode = XPath(itemsNode, L"item/tune[@xmlns='" JABBER_FEAT_USER_TUNE L"']");
		if (!tuneNode)
			return;

		const wchar_t *szArtist = XPathT(tuneNode, "artist");
		const wchar_t *szLength = XPathT(tuneNode, "length");
		const wchar_t *szSource = XPathT(tuneNode, "source");
		const wchar_t *szTitle = XPathT(tuneNode, "title");
		const wchar_t *szTrack = XPathT(tuneNode, "track");

		wchar_t szLengthInTime[32];
		szLengthInTime[0] = 0;
		if (szLength) {
			int nLength = _wtoi(szLength);
			mir_snwprintf(szLengthInTime, L"%02d:%02d:%02d", nLength / 3600, (nLength / 60) % 60, nLength % 60);
		}

		SetContactTune(hContact, szArtist, szLength ? szLengthInTime : nullptr, szSource, szTitle, szTrack);
	}
}

// returns 0, if error or no events
DWORD JabberGetLastContactMessageTime(MCONTACT hContact)
{
	// TODO: time cache can improve performance
	MEVENT hDbEvent = db_event_last(hContact);
	if (!hDbEvent)
		return 0;

	DWORD dwTime = 0;

	DBEVENTINFO dbei = {};
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob != -1) {
		dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob + 1);
		int nGetTextResult = db_event_get(hDbEvent, &dbei);
		if (!nGetTextResult)
			dwTime = dbei.timestamp;
		mir_free(dbei.pBlob);
	}
	return dwTime;
}

MCONTACT CJabberProto::CreateTemporaryContact(const wchar_t *szJid, JABBER_LIST_ITEM* chatItem)
{
	if (chatItem == nullptr)
		return DBCreateContact(szJid, ptrW(JabberNickFromJID(szJid)), true, true);

	const wchar_t *p = wcschr(szJid, '/');
	if (p != nullptr && p[1] != '\0')
		p++;
	else
		p = szJid;
	MCONTACT hContact = DBCreateContact(szJid, p, true, false);

	pResourceStatus r(chatItem->findResource(p));
	if (r)
		setWord(hContact, "Status", r->m_iStatus);

	return hContact;
}

void CJabberProto::OnProcessMessage(HXML node, ThreadData *info)
{
	HXML xNode, n;

	if (!XmlGetName(node) || mir_wstrcmp(XmlGetName(node), L"message"))
		return;

	LPCTSTR from, type = XmlGetAttrValue(node, L"type");
	if ((from = XmlGetAttrValue(node, L"from")) == nullptr)
		return;

	LPCTSTR idStr = XmlGetAttrValue(node, L"id");
	pResourceStatus pFromResource(ResourceInfoFromJID(from));

	// Message receipts delivery request. Reply here, before a call to HandleMessagePermanent() to make sure message receipts are handled for external plugins too.
	if ((!type || mir_wstrcmpi(type, L"error")) && XmlGetChildByTag(node, "request", "xmlns", JABBER_FEAT_MESSAGE_RECEIPTS)) {
		info->send(
			XmlNode(L"message") << XATTR(L"to", from) << XATTR(L"id", idStr)
				<< XCHILDNS(L"received", JABBER_FEAT_MESSAGE_RECEIPTS) << XATTR(L"id", idStr));

		if (pFromResource)
			pFromResource->m_jcbManualDiscoveredCaps |= JABBER_CAPS_MESSAGE_RECEIPTS;
	}

	if (m_messageManager.HandleMessagePermanent(node, info))
		return;

	//Handle carbons. The message MUST be coming from our bare JID.
	HXML carbon = nullptr;
	bool carbonSent = false; //2 cases: received or sent.
	if (this->IsMyOwnJID(from)) {
		carbon = XmlGetChildByTag(node, "received", "xmlns", JABBER_FEAT_CARBONS);
		if (!carbon) {
			carbon = XmlGetChildByTag(node, "sent", "xmlns", JABBER_FEAT_CARBONS);
			if (carbon)
				carbonSent = true;
		}
		if (carbon) {
			HXML forwarded = NULL;
			HXML message = NULL;
			//Carbons MUST have forwarded/message content
			if (!(forwarded = XmlGetChildByTag(carbon, "forwarded", "xmlns", JABBER_XMLNS_FORWARD))
				|| !(message = XmlGetChild(forwarded, "message")))
				return;

			//Unwrap the carbon in any case
			node = message;
			type = XmlGetAttrValue(node, L"type");

			if (!carbonSent) {
				//Received should just be treated like incoming messages, except maybe not flash the flasher. Simply unwrap.
				from = XmlGetAttrValue(node, L"from");
				if (from == nullptr)
					return;
			}
			else {
				//Sent should set SENT flag and invert from/to.
				from = XmlGetAttrValue(node, L"to");
				if (from == nullptr)
					return;
			}
		}
	}

	MCONTACT hContact = HContactFromJID(from);
	JABBER_LIST_ITEM *chatItem = ListGetItemPtr(LIST_CHATROOM, from);
	if (chatItem) {
		HXML xCaptcha = XmlGetChild(node, "captcha");
		if (xCaptcha)
			if (ProcessCaptcha(xCaptcha, node, info))
				return;
	}

	const wchar_t *szMessage = nullptr;
	HXML bodyNode = XmlGetChildByTag(node, "body", "xml:lang", m_tszSelectedLang);
	if (bodyNode == nullptr)
		bodyNode = XmlGetChild(node, "body");
	if (bodyNode != nullptr)
		szMessage = XmlGetText(bodyNode);

	LPCTSTR ptszSubject = XmlGetText(XmlGetChild(node, "subject"));
	if (ptszSubject && *ptszSubject) {
		size_t cbLen = (szMessage ? mir_wstrlen(szMessage) : 0) + mir_wstrlen(ptszSubject) + 128;
		wchar_t *szTmp = (wchar_t *)alloca(sizeof(wchar_t) * cbLen);
		szTmp[0] = 0;
		if (szMessage)
			mir_wstrcat(szTmp, L"Subject: ");
		mir_wstrcat(szTmp, ptszSubject);
		if (szMessage) {
			mir_wstrcat(szTmp, L"\r\n");
			mir_wstrcat(szTmp, szMessage);
		}
		szMessage = szTmp;
	}

	if (szMessage && (n = XmlGetChildByTag(node, "addresses", "xmlns", JABBER_FEAT_EXT_ADDRESSING))) {
		HXML addressNode = XmlGetChildByTag(n, "address", "type", L"ofrom");
		if (addressNode) {
			const wchar_t *szJid = XmlGetAttrValue(addressNode, L"jid");
			if (szJid) {
				size_t cbLen = mir_wstrlen(szMessage) + 1000;
				wchar_t *p = (wchar_t*)alloca(sizeof(wchar_t) * cbLen);
				mir_snwprintf(p, cbLen, TranslateT("Message redirected from: %s\r\n%s"), from, szMessage);
				szMessage = p;
				from = szJid;
				// rewrite hContact
				hContact = HContactFromJID(from);
			}
		}
	}

	// If message is from a stranger (not in roster), item is nullptr
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, from);
	if (item == nullptr)
		item = ListGetItemPtr(LIST_VCARD_TEMP, from);

	time_t msgTime = 0;
	bool isChatRoomInvitation = false;
	const wchar_t *inviteRoomJid = nullptr;
	const wchar_t *inviteFromJid = nullptr;
	const wchar_t *inviteReason = nullptr;
	const wchar_t *invitePassword = nullptr;
	bool isDelivered = false;

	// check chatstates availability
	if (pFromResource && XmlGetChildByTag(node, "active", "xmlns", JABBER_FEAT_CHATSTATES))
		pFromResource->m_jcbManualDiscoveredCaps |= JABBER_CAPS_CHATSTATES;

	// chatstates composing event
	if (hContact && XmlGetChildByTag(node, "composing", "xmlns", JABBER_FEAT_CHATSTATES))
		CallService(MS_PROTO_CONTACTISTYPING, hContact, 60);

	// chatstates paused event
	if (hContact && XmlGetChildByTag(node, "paused", "xmlns", JABBER_FEAT_CHATSTATES))
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

	// chatstates inactive event
	if (hContact && XmlGetChildByTag(node, "inactive", "xmlns", JABBER_FEAT_CHATSTATES)) {
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
		if (pFromResource)
			pFromResource->m_bMessageSessionActive = false;
	}

	// message receipts delivery notification
	if (n = XmlGetChildByTag(node, "received", "xmlns", JABBER_FEAT_MESSAGE_RECEIPTS)) {
		int nPacketId = JabberGetPacketID(n);
		if (nPacketId == -1)
			nPacketId = JabberGetPacketID(node);
		if (nPacketId != -1)
			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)nPacketId, 0);
	}

	JabberReadXep203delay(node, msgTime);

	// XEP-0224 support (Attention/Nudge)
	if (XmlGetChildByTag(node, "attention", "xmlns", JABBER_FEAT_ATTENTION)) {
		if (!hContact)
			hContact = CreateTemporaryContact(from, chatItem);
		if (hContact)
			NotifyEventHooks(m_hEventNudge, hContact, 0);
	}

	// chatstates gone event
	if (hContact && XmlGetChildByTag(node, "gone", "xmlns", JABBER_FEAT_CHATSTATES) && m_options.LogChatstates) {
		BYTE bEventType = JABBER_DB_EVENT_CHATSTATES_GONE; // gone event
		DBEVENTINFO dbei = {};
		dbei.pBlob = &bEventType;
		dbei.cbBlob = 1;
		dbei.eventType = EVENTTYPE_JABBER_CHATSTATES;
		dbei.flags = DBEF_READ;
		dbei.timestamp = time(nullptr);
		dbei.szModule = m_szModuleName;
		db_event_add(hContact, &dbei);
	}

	if ((n = XmlGetChildByTag(node, "confirm", "xmlns", JABBER_FEAT_HTTP_AUTH)) && m_options.AcceptHttpAuth) {
		const wchar_t *szId = XmlGetAttrValue(n, L"id");
		const wchar_t *szMethod = XmlGetAttrValue(n, L"method");
		const wchar_t *szUrl = XmlGetAttrValue(n, L"url");
		if (!szId || !szMethod || !szUrl)
			return;

		CJabberHttpAuthParams *pParams = (CJabberHttpAuthParams *)mir_alloc(sizeof(CJabberHttpAuthParams));
		if (!pParams)
			return;

		memset(pParams, 0, sizeof(CJabberHttpAuthParams));
		pParams->m_nType = CJabberHttpAuthParams::MSG;
		pParams->m_szFrom = mir_wstrdup(from);
		LPCTSTR ptszThread = XmlGetText(XmlGetChild(node, "thread"));
		if (ptszThread && *ptszThread)
			pParams->m_szThreadId = mir_wstrdup(ptszThread);
		pParams->m_szId = mir_wstrdup(szId);
		pParams->m_szMethod = mir_wstrdup(szMethod);
		pParams->m_szUrl = mir_wstrdup(szUrl);

		AddClistHttpAuthEvent(pParams);
		return;
	}

	// parsing extensions
	for (int i = 0; (xNode = XmlGetChild(node, i)) != nullptr; i++) {
		if (m_options.UseOMEMO)
		{
			if ((xNode = XmlGetNthChild(node, L"encrypted", i + 1)) != nullptr)
			{
				const wchar_t *ptszXmlns = XmlGetAttrValue(xNode, L"xmlns");
				if (ptszXmlns == nullptr)
					continue;

				if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_OMEMO))
				{
					LPCTSTR jid = xmlGetAttrValue(node, L"from");
					if (jid)
					{
						OmemoHandleMessage(xNode, (wchar_t*)jid, msgTime);
						continue;
					}
				}
			}

		}
		if ((xNode = XmlGetNthChild(node, L"x", i + 1)) == nullptr)
			continue;

		const wchar_t *ptszXmlns = XmlGetAttrValue(xNode, L"xmlns");
		if (ptszXmlns == nullptr)
			continue;

		if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_MIRANDA_NOTES)) {
			if (OnIncomingNote(from, XmlGetChild(xNode, "note")))
				return;
		}
		else if (!mir_wstrcmp(ptszXmlns, L"jabber:x:encrypted")) {
			LPCTSTR ptszText = XmlGetText(xNode);
			if (ptszText == nullptr)
				return;

			//XEP-0027 is not strict enough, different clients have different implementations
			//additional validation is required
			wchar_t *prolog = L"-----BEGIN PGP MESSAGE-----";
			wchar_t *prolog_newline = L"\r\n\r\n";
			wchar_t *epilog = L"\r\n-----END PGP MESSAGE-----\r\n";

			size_t len = 0;
			wchar_t *tempstring = nullptr;
			if(!wcsstr(ptszText, prolog))
			{
				len = mir_wstrlen(prolog) + mir_wstrlen(prolog_newline) + mir_wstrlen(ptszText) + mir_wstrlen(epilog) + 3;
				tempstring = (wchar_t*)_alloca(sizeof(wchar_t)*len);
				mir_snwprintf(tempstring, len, L"%s%s%s%s", prolog, prolog_newline, ptszText, epilog);
			}
			else
			{
				len = mir_wstrlen(ptszText) + 3;
				tempstring = (wchar_t*)_alloca(sizeof(wchar_t)*len);
				mir_snwprintf(tempstring, len, L"%s", ptszText);
			}
			
			szMessage = tempstring;
		}
		else if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_DELAY) && msgTime == 0) {
			const wchar_t *ptszTimeStamp = XmlGetAttrValue(xNode, L"stamp");
			if (ptszTimeStamp != nullptr)
				msgTime = JabberIsoToUnixTime(ptszTimeStamp);
		}
		else if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_MESSAGE_EVENTS)) {

			// set events support only if we discovered caps and if events not already set
			JabberCapsBits jcbCaps = GetResourceCapabilities(from);
			if (jcbCaps & JABBER_RESOURCE_CAPS_ERROR)
				jcbCaps = JABBER_RESOURCE_CAPS_NONE;
			// FIXME: disabled due to expired XEP-0022 and problems with bombus delivery checks
			//			if (jcbCaps && pFromResource && (!(jcbCaps & JABBER_CAPS_MESSAGE_EVENTS)))
			//				pFromResource->m_jcbManualDiscoveredCaps |= (JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY);

			if (bodyNode == nullptr) {
				HXML idNode = XmlGetChild(xNode, "id");
				if (XmlGetChild(xNode, "delivered") != nullptr || XmlGetChild(xNode, "offline") != nullptr) {
					int id = -1;
					if (idNode != nullptr && XmlGetText(idNode) != nullptr)
						if (!wcsncmp(XmlGetText(idNode), _T(JABBER_IQID), mir_strlen(JABBER_IQID)))
							id = _wtoi((XmlGetText(idNode)) + mir_strlen(JABBER_IQID));

					if (id != -1)
						ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)id, 0);
				}

				if (hContact && XmlGetChild(xNode, "composing") != nullptr)
					CallService(MS_PROTO_CONTACTISTYPING, hContact, 60);

				// Maybe a cancel to the previous composing
				HXML child = XmlGetChild(xNode, 0);
				if (hContact && (!child || (child && idNode != nullptr)))
					CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
			}
			else {
				// Check whether any event is requested
				if (!isDelivered && (n = XmlGetChild(xNode, "delivered")) != nullptr) {
					isDelivered = true;

					XmlNode m(L"message"); m << XATTR(L"to", from);
					HXML x = m << XCHILDNS(L"x", JABBER_FEAT_MESSAGE_EVENTS);
					x << XCHILD(L"delivered");
					x << XCHILD(L"id", idStr);
					info->send(m);
				}
				if (item != nullptr && XmlGetChild(xNode, "composing") != nullptr) {
					if (item->messageEventIdStr)
						mir_free(item->messageEventIdStr);
					item->messageEventIdStr = (idStr == nullptr) ? nullptr : mir_wstrdup(idStr);
				}
			}
		}
		else if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_OOB2)) {
			LPCTSTR ptszUrl = XmlGetText(XmlGetChild(xNode, "url"));
			if (ptszUrl != nullptr && *ptszUrl) {
				size_t cbLen = (szMessage ? mir_wstrlen(szMessage) : 0) + mir_wstrlen(ptszUrl) + 32;
				wchar_t *szTmp = (wchar_t *)alloca(sizeof(wchar_t)* cbLen);
				mir_wstrcpy(szTmp, ptszUrl);
				if (szMessage) {
					mir_wstrcat(szTmp, L"\r\n");
					mir_wstrcat(szTmp, szMessage);
				}
				szMessage = szTmp;
			}
		}
		else if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_MUC_USER)) {
			HXML inviteNode = XmlGetChild(xNode, L"invite");
			if (inviteNode != nullptr) {
				inviteFromJid = XmlGetAttrValue(inviteNode, L"from");
				inviteReason = XmlGetText(XmlGetChild(inviteNode, L"reason"));
				inviteRoomJid = from;
				if (inviteReason == nullptr)
					inviteReason = szMessage;
				isChatRoomInvitation = true;
				invitePassword = XmlGetText(XmlGetChild(xNode, "password"));
			}
		}
		else if (!mir_wstrcmp(ptszXmlns, JABBER_FEAT_ROSTER_EXCHANGE) &&
			item != nullptr && (item->subscription == SUB_BOTH || item->subscription == SUB_TO)) {
			wchar_t chkJID[JABBER_MAX_JID_LEN] = L"@";
			JabberStripJid(from, chkJID + 1, _countof(chkJID) - 1);
			for (int j = 1;; j++) {
				HXML iNode = XmlGetNthChild(xNode, L"item", j);
				if (iNode == nullptr)
					break;
				
				const wchar_t *action = XmlGetAttrValue(iNode, L"action");
				const wchar_t *jid = XmlGetAttrValue(iNode, L"jid");
				const wchar_t *nick = XmlGetAttrValue(iNode, L"name");
				const wchar_t *group = XmlGetText(XmlGetChild(iNode, L"group"));
				if (action && jid && wcsstr(jid, chkJID)) {
					if (!mir_wstrcmp(action, L"add")) {
						MCONTACT cc = DBCreateContact(jid, nick, false, false);
						if (group)
							db_set_ws(cc, "CList", "Group", group);
					}
					else if (!mir_wstrcmp(action, L"delete")) {
						MCONTACT cc = HContactFromJID(jid);
						if (cc)
							db_delete_contact(cc);
					}
				}
			}
		}
		else if (!isChatRoomInvitation && !mir_wstrcmp(ptszXmlns, JABBER_FEAT_DIRECT_MUC_INVITE)) {
			inviteRoomJid = XmlGetAttrValue(xNode, L"jid");
			inviteFromJid = from;
			if (inviteReason == nullptr)
				inviteReason = XmlGetText(xNode);
			if (!inviteReason)
				inviteReason = szMessage;
			isChatRoomInvitation = true;
		}
	}

	if (isChatRoomInvitation) {
		if (inviteRoomJid != nullptr) {
			if (m_options.IgnoreMUCInvites) {
				// FIXME: temporary disabled due to MUC inconsistence on server side
				/*
				XmlNode m("message"); XmlAddAttr(m, "to", from);
				XmlNode xNode = XmlAddChild(m, "x");
				XmlAddAttr(xNode, "xmlns", JABBER_FEAT_MUC_USER);
				XmlNode declineNode = XmlAddChild(xNode, "decline");
				XmlAddAttr(declineNode, "from", inviteRoomJid);
				XmlNode reasonNode = XmlAddChild(declineNode, "reason", "The user has chosen to not accept chat invites");
				info->send(m);
				*/
			}
			else GroupchatProcessInvite(inviteRoomJid, inviteFromJid, inviteReason, invitePassword);
		}
		return;
	}

	// all service info was already processed
	if (szMessage == nullptr)
		return;

	CMStringW tmp(szMessage);
	tmp += ExtractImage(node);
	tmp.Replace(L"\n", L"\r\n");
	ptrA buf(mir_utf8encodeW(tmp));

	if (item != nullptr) {
		if (pFromResource) {
			pFromResource->m_bMessageSessionActive = TRUE;

			JABBER_RESOURCE_STATUS *pLast = item->m_pLastSeenResource;
			item->m_pLastSeenResource = pFromResource;
			if (item->resourceMode == RSMODE_LASTSEEN && pLast == pFromResource)
				UpdateMirVer(item);
		}
	}

	// Create a temporary contact, if needed
	if (hContact == 0)
		hContact = CreateTemporaryContact(from, chatItem);
	CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

	time_t now = time(nullptr);
	if (!msgTime)
		msgTime = now;

	if (m_options.FixIncorrectTimestamps && (msgTime > now || (msgTime < (time_t)JabberGetLastContactMessageTime(hContact))))
		msgTime = now;

	PROTORECVEVENT recv = { 0 };
	if (carbon) {
		recv.flags |= PREF_CREATEREAD;
		if (carbonSent)
			recv.flags |= PREF_SENT;
	}
	recv.timestamp = (DWORD)msgTime;
	recv.szMessage = buf;
	recv.lParam = (LPARAM)((pFromResource != nullptr && m_options.EnableRemoteControl) ? pFromResource->m_tszResourceName : 0);
	ProtoChainRecvMsg(hContact, &recv);
}

// XEP-0115: Entity Capabilities
void CJabberProto::OnProcessPresenceCapabilites(HXML node, pResourceStatus &r)
{
	if (r == nullptr)
		return;

	// already filled up? ok
	if (r->m_pCaps != nullptr)
		return;

	const wchar_t *from = XmlGetAttrValue(node, L"from");
	if (from == nullptr)
		return;

	HXML n = XmlGetChildByTag(node, "c", "xmlns", JABBER_FEAT_ENTITY_CAPS);
	if (n == nullptr)
		return;

	const wchar_t *szNode = XmlGetAttrValue(n, L"node");
	const wchar_t *szVer = XmlGetAttrValue(n, L"ver");
	const wchar_t *szExt = XmlGetAttrValue(n, L"ext");
	if (szNode == nullptr || szVer == nullptr)
		return;

	const wchar_t *szHash = XmlGetAttrValue(n, L"hash");
	if (szHash == nullptr) { // old version
		ptrA szVerUtf(mir_utf8encodeW(szVer));
		BYTE hashOut[MIR_SHA1_HASH_SIZE];
		mir_sha1_hash((BYTE*)szVerUtf.get(), mir_strlen(szVerUtf), hashOut);
		wchar_t szHashOut[MIR_SHA1_HASH_SIZE * 2 + 1];
		bin2hexW(hashOut, _countof(hashOut), szHashOut);
		r->m_pCaps = m_clientCapsManager.GetPartialCaps(szNode, szHashOut);
		if (r->m_pCaps == nullptr)
			GetCachedCaps(szNode, szHashOut, r);

		if (r->m_pCaps == nullptr) {
			r->m_pCaps = m_clientCapsManager.SetClientCaps(szNode, szHashOut, szVer, JABBER_RESOURCE_CAPS_UNINIT);
			RequestOldCapsInfo(r, from);
		}

		MCONTACT hContact = HContactFromJID(from);
		if (hContact)
			UpdateMirVer(hContact, r);
	}
	else {
		r->m_pCaps = m_clientCapsManager.GetPartialCaps(szNode, szVer);
		if (r->m_pCaps == nullptr)
			GetCachedCaps(szNode, szVer, r);

		if (r->m_pCaps == nullptr) {
			r->m_pCaps = m_clientCapsManager.SetClientCaps(szNode, szVer, L"", JABBER_RESOURCE_CAPS_UNINIT);
			GetResourceCapabilities(from, r);
		}
	}

	r->m_tszCapsExt = mir_wstrdup(szExt);
}

void CJabberProto::UpdateJidDbSettings(const wchar_t *jid)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return;
	
	MCONTACT hContact = HContactFromJID(jid);
	if (hContact == 0)
		return;

	int status = ID_STATUS_OFFLINE;
	if (!item->arResources.getCount()) {
		// set offline only if jid has resources
		if (wcschr(jid, '/') == nullptr)
			status = item->getTemp()->m_iStatus;
		if (item->getTemp()->m_tszStatusMessage)
			db_set_ws(hContact, "CList", "StatusMsg", item->getTemp()->m_tszStatusMessage);
		else
			db_unset(hContact, "CList", "StatusMsg");
	}

	// Determine status to show for the contact based on the remaining resources
	int nSelectedResource = -1;
	int nMaxPriority = -999; // -128...+127 valid range
	for (int i = 0; i < item->arResources.getCount(); i++) {
		pResourceStatus r(item->arResources[i]);
		if (r->m_iPriority > nMaxPriority) {
			nMaxPriority = r->m_iPriority;
			status = r->m_iStatus;
			nSelectedResource = i;
		}
		else if (r->m_iPriority == nMaxPriority) {
			if ((status = JabberCombineStatus(status, r->m_iStatus)) == r->m_iStatus)
				nSelectedResource = i;
		}
	}
	item->getTemp()->m_iStatus = status;
	if (nSelectedResource != -1) {
		pResourceStatus r(item->arResources[nSelectedResource]);
		debugLogW(L"JabberUpdateJidDbSettings: updating jid %s to rc %s", item->jid, r->m_tszResourceName);
		if (r->m_tszStatusMessage)
			db_set_ws(hContact, "CList", "StatusMsg", r->m_tszStatusMessage);
		else
			db_unset(hContact, "CList", "StatusMsg");
		UpdateMirVer(hContact, r);
	}
	else delSetting(hContact, DBSETTING_DISPLAY_UID);

	if (wcschr(jid, '@') != nullptr || m_options.ShowTransport == TRUE)
		if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != status)
			setWord(hContact, "Status", (WORD)status);

	// remove x-status icon
	if (status == ID_STATUS_OFFLINE) {
		delSetting(hContact, DBSETTING_XSTATUSID);
		delSetting(hContact, DBSETTING_XSTATUSNAME);
		delSetting(hContact, DBSETTING_XSTATUSMSG);
	}

	MenuUpdateSrmmIcon(item);
}

void CJabberProto::OnProcessPresence(HXML node, ThreadData *info)
{
	if (!node || !XmlGetName(node) || mir_wstrcmp(XmlGetName(node), L"presence"))
		return;

	LPCTSTR from = XmlGetAttrValue(node, L"from");
	if (from == nullptr)
		return;

	if (m_presenceManager.HandlePresencePermanent(node, info))
		return;

	if (ListGetItemPtr(LIST_CHATROOM, from)) {
		GroupchatProcessPresence(node);
		return;
	}

	MCONTACT hContact;
	bool bSelfPresence = false;
	wchar_t szBareFrom[JABBER_MAX_JID_LEN];
	JabberStripJid(from, szBareFrom, _countof(szBareFrom));
	wchar_t szBareOurJid[JABBER_MAX_JID_LEN];
	JabberStripJid(info->fullJID, szBareOurJid, _countof(szBareOurJid));

	if (!mir_wstrcmpi(szBareFrom, szBareOurJid))
		bSelfPresence = true;

	LPCTSTR type = XmlGetAttrValue(node, L"type");
	if (type == nullptr || !mir_wstrcmp(type, L"available")) {
		ptrW nick(JabberNickFromJID(from));
		if (nick == nullptr)
			return;

		if ((hContact = HContactFromJID(from)) == 0) {
			if (!mir_wstrcmpi(info->fullJID, from) || (!bSelfPresence && !ListGetItemPtr(LIST_ROSTER, from))) {
				debugLogW(L"SKIP Receive presence online from %s (who is not in my roster and not in list - skiping)", from);
				return;
			}
			hContact = DBCreateContact(from, nick, true, true);
		}
		if (!ListGetItemPtr(LIST_ROSTER, from)) {
			debugLogW(L"Receive presence online from %s (who is not in my roster)", from);
			ListAdd(LIST_ROSTER, from, hContact);
		}
		DBCheckIsTransportedContact(from, hContact);
		int status = ID_STATUS_ONLINE;
		if (HXML showNode = XmlGetChild(node, "show")) {
			if (LPCTSTR show = XmlGetText(showNode)) {
				if (!mir_wstrcmp(show, L"away")) status = ID_STATUS_AWAY;
				else if (!mir_wstrcmp(show, L"xa")) status = ID_STATUS_NA;
				else if (!mir_wstrcmp(show, L"dnd")) status = ID_STATUS_DND;
				else if (!mir_wstrcmp(show, L"chat")) status = ID_STATUS_FREECHAT;
			}
		}

		char priority = 0;
		if (LPCTSTR ptszPriority = XmlGetText(XmlGetChild(node, "priority")))
			priority = (char)_wtoi(ptszPriority);

		ListAddResource(LIST_ROSTER, from, status, XmlGetText(XmlGetChild(node, "status")), priority);

		// XEP-0115: Entity Capabilities
		pResourceStatus r(ResourceInfoFromJID(from));
		if (r != nullptr)
			OnProcessPresenceCapabilites(node, r);

		UpdateJidDbSettings(from);

		if (wcschr(from, '@') == nullptr) {
			UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);
		}
		debugLogW(L"%s (%s) online, set contact status to %s", nick, from, pcli->pfnGetStatusModeDescription(status, 0));

		HXML xNode;
		if (m_options.EnableAvatars) {
			bool bHasAvatar = false, bRemovedAvatar = false;

			debugLogA("Avatar enabled");
			for (int i = 1; (xNode = XmlGetNthChild(node, L"x", i)) != nullptr; i++) {
				if (!mir_wstrcmp(XmlGetAttrValue(xNode, L"xmlns"), L"jabber:x:avatar")) {
					LPCTSTR ptszHash = XmlGetText(XmlGetChild(xNode, "hash"));
					if (ptszHash != nullptr) {
						delSetting(hContact, "AvatarXVcard");
						debugLogA("AvatarXVcard deleted");
						setWString(hContact, "AvatarHash", ptszHash);
						bHasAvatar = true;
						ptrW saved(getWStringA(hContact, "AvatarSaved"));
						if (saved == nullptr || mir_wstrcmp(saved, ptszHash)) {
							debugLogA("Avatar was changed");
							ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, nullptr, 0);
						}
					}
					else bRemovedAvatar = true;
				}
			}
			if (!bHasAvatar) { //no jabber:x:avatar. try vcard-temp:x:update
				debugLogA("Not hasXAvatar");
				for (int i = 1; (xNode = XmlGetNthChild(node, L"x", i)) != nullptr; i++) {
					if (!mir_wstrcmp(XmlGetAttrValue(xNode, L"xmlns"), L"vcard-temp:x:update")) {
						if ((xNode = XmlGetChild(xNode, "photo")) != nullptr) {
							LPCTSTR txt = XmlGetText(xNode);
							if (txt != nullptr && txt[0] != 0) {
								setByte(hContact, "AvatarXVcard", 1);
								debugLogA("AvatarXVcard set");
								setWString(hContact, "AvatarHash", txt);
								bHasAvatar = true;
								ptrW saved(getWStringA(hContact, "AvatarSaved"));
								if (saved == nullptr || mir_wstrcmp(saved, txt)) {
									debugLogA("Avatar was changed. Using vcard-temp:x:update");
									ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, nullptr, 0);
								}
							}
							else bRemovedAvatar = true;
			}	}	}	}

			if (!bHasAvatar && bRemovedAvatar) {
				debugLogA("Has no avatar");
				delSetting(hContact, "AvatarHash");

				if (ptrW(getWStringA(hContact, "AvatarSaved")) != nullptr) {
					delSetting(hContact, "AvatarSaved");
					ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr, 0);
		}	}	}
		return;
	}

	if (!mir_wstrcmp(type, L"unavailable")) {
		hContact = HContactFromJID(from);
		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, from);
		if (item != nullptr) {
			ListRemoveResource(LIST_ROSTER, from);

			hContact = HContactFromJID(from);
			if (hContact && db_get_b(hContact, "CList", "NotOnList", 0) == 1) {
				// remove selfcontact, if where is no more another resources
				if (item->arResources.getCount() == 1 && ResourceInfoFromJID(info->fullJID))
					ListRemoveResource(LIST_ROSTER, info->fullJID);
			}


			// set status only if no more available resources
			if (!item->arResources.getCount()) {
				item->getTemp()->m_iStatus = ID_STATUS_OFFLINE;
				item->getTemp()->m_tszStatusMessage = mir_wstrdup(XmlGetText(XmlGetChild(node, "status")));
			}
		}
		else debugLogW(L"SKIP Receive presence offline from %s (who is not in my roster)", from);

		UpdateJidDbSettings(from);

		if (wcschr(from, '@') == nullptr)
			UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);

		DBCheckIsTransportedContact(from, hContact);
		return;
	}

	if (!mir_wstrcmp(type, L"subscribe")) {
		if (hContact = HContactFromJID(from))
			AddDbPresenceEvent(hContact, JABBER_DB_EVENT_PRESENCE_SUBSCRIBE);

		ptrW tszNick(JabberNickFromJID(from));
		HXML xNick = XmlGetChildByTag(node, "nick", "xmlns", JABBER_FEAT_NICK);
		if (xNick != nullptr) {
			LPCTSTR xszNick = XmlGetText(xNick);
			if (xszNick != nullptr && *xszNick) {
				debugLogW(L"Grabbed nick from presence: %s", xszNick);
				tszNick = mir_wstrdup(xszNick);
			}
		}

		// automatically send authorization allowed to agent/transport
		if (wcschr(from, '@') == nullptr || m_options.AutoAcceptAuthorization) {
			ListAdd(LIST_ROSTER, from, hContact);
			info->send(XmlNode(L"presence") << XATTR(L"to", from) << XATTR(L"type", L"subscribed"));

			if (m_options.AutoAdd == TRUE) {
				JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, from);
				if (item == nullptr || (item->subscription != SUB_BOTH && item->subscription != SUB_TO)) {
					debugLogW(L"Try adding contact automatically jid = %s", from);
					if ((hContact = AddToListByJID(from, 0)) != 0) {
						if (item)
							item->hContact = hContact;
						setWString(hContact, "Nick", tszNick);
						db_unset(hContact, "CList", "NotOnList");
					}
				}
			}
			RebuildInfoFrame();
		}
		else {
			debugLogW(L"%s (%s) requests authorization", tszNick, from);
			DBAddAuthRequest(from, tszNick);
		}
		return;
	}

	if (!mir_wstrcmp(type, L"unsubscribe"))
		if (hContact = HContactFromJID(from))
			AddDbPresenceEvent(hContact, JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBE);

	if (!mir_wstrcmp(type, L"unsubscribed"))
		if (hContact = HContactFromJID(from))
			AddDbPresenceEvent(hContact, JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBED);

	if (!mir_wstrcmp(type, L"error"))
		if (hContact = HContactFromJID(from))
			AddDbPresenceEvent(hContact, JABBER_DB_EVENT_PRESENCE_ERROR);

	if (!mir_wstrcmp(type, L"subscribed")) {
		if (hContact = HContactFromJID(from))
			AddDbPresenceEvent(hContact, JABBER_DB_EVENT_PRESENCE_SUBSCRIBED);

		if (JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, from)) {
			if (item->subscription == SUB_FROM) item->subscription = SUB_BOTH;
			else if (item->subscription == SUB_NONE) {
				item->subscription = SUB_TO;
				if (wcschr(from, '@') == nullptr) {
					UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);
				}
			}
			UpdateSubscriptionInfo(hContact, item);
		}
	}
}

BOOL CJabberProto::OnProcessJingle(HXML node)
{
	LPCTSTR type;
	HXML child = XmlGetChildByTag(node, L"jingle", L"xmlns", JABBER_FEAT_JINGLE);

	if (child) {
		if ((type = XmlGetAttrValue(node, L"type")) == nullptr) return FALSE;
		if ((!mir_wstrcmp(type, L"get") || !mir_wstrcmp(type, L"set"))) {
			LPCTSTR szAction = XmlGetAttrValue(child, L"action");
			LPCTSTR idStr = XmlGetAttrValue(node, L"id");
			LPCTSTR from = XmlGetAttrValue(node, L"from");
			if (szAction && !mir_wstrcmp(szAction, L"session-initiate")) {
				// if this is a Jingle 'session-initiate' and noone processed it yet, reply with "unsupported-applications"
				m_ThreadInfo->send(XmlNodeIq(L"result", idStr, from));

				XmlNodeIq iq(L"set", SerialNext(), from);
				HXML jingleNode = iq << XCHILDNS(L"jingle", JABBER_FEAT_JINGLE);

				jingleNode << XATTR(L"action", L"session-terminate");
				LPCTSTR szInitiator = XmlGetAttrValue(child, L"initiator");
				if (szInitiator)
					jingleNode << XATTR(L"initiator", szInitiator);
				LPCTSTR szSid = XmlGetAttrValue(child, L"sid");
				if (szSid)
					jingleNode << XATTR(L"sid", szSid);

				jingleNode << XCHILD(L"reason")
					<< XCHILD(L"unsupported-applications");
				m_ThreadInfo->send(iq);
				return TRUE;
			}
			else {
				// if it's something else than 'session-initiate' and noone processed it yet, reply with "unknown-session"
				XmlNodeIq iq(L"error", idStr, from);
				HXML errNode = iq << XCHILD(L"error");
				errNode << XATTR(L"type", L"cancel");
				errNode << XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas");
				errNode << XCHILDNS(L"unknown-session", L"urn:xmpp:jingle:errors:1");
				m_ThreadInfo->send(iq);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CJabberProto::OnProcessIq(HXML node)
{
	HXML queryNode;
	const wchar_t *type, *xmlns;

	if (!XmlGetName(node) || mir_wstrcmp(XmlGetName(node), L"iq")) return;
	if ((type = XmlGetAttrValue(node, L"type")) == nullptr) return;

	int id = JabberGetPacketID(node);

	queryNode = XmlGetChild(node, "query");
	xmlns = XmlGetAttrValue(queryNode, L"xmlns");

	// new match by id
	if (m_iqManager.HandleIq(id, node))
		return;

	// new iq handler engine
	if (m_iqManager.HandleIqPermanent(node))
		return;

	// Jingle support
	if (OnProcessJingle(node))
		return;

	// RECVED: <iq type='error'> ...
	if (!mir_wstrcmp(type, L"error")) {
		wchar_t tszBuf[20];
		_itow(id, tszBuf, 10);

		debugLogA("XXX on entry");
		// Check for file transfer deny by comparing idStr with ft->iqId
		LISTFOREACH(i, this, LIST_FILE)
		{
			JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
			if (item->ft != nullptr && item->ft->state == FT_CONNECTING && !mir_wstrcmp(tszBuf, item->ft->szId)) {
				debugLogA("Denying file sending request");
				item->ft->state = FT_DENIED;
				if (item->ft->hFileEvent != nullptr)
					SetEvent(item->ft->hFileEvent);	// Simulate the termination of file server connection
			}
		}
	}
	else if ((!mir_wstrcmp(type, L"get") || !mir_wstrcmp(type, L"set"))) {
		XmlNodeIq iq(L"error", id, XmlGetAttrValue(node, L"from"));

		HXML pFirstChild = XmlGetChild(node, 0);
		if (pFirstChild)
			XmlAddChild(iq, pFirstChild);

		iq << XCHILD(L"error") << XATTR(L"type", L"cancel")
				<< XCHILDNS(L"service-unavailable", L"urn:ietf:params:xml:ns:xmpp-stanzas");
		m_ThreadInfo->send(iq);
	}
}

ThreadData *m_regInfo;
void CJabberProto::SetRegConfig(HXML node, void *from)
{
	if (m_regInfo) {
		iqIdRegSetReg = SerialNext();
		
		wchar_t text[MAX_PATH];
		mir_snwprintf(text, L"%s@%S", m_regInfo->conn.username, m_regInfo->conn.server);
		XmlNodeIq iq(L"set", iqIdRegSetReg, (const wchar_t*)from);
		iq << XATTR(L"from", text);
		HXML query = iq << XQUERY(JABBER_FEAT_REGISTER);
		XmlAddChild(query, node);
		m_regInfo->send(iq);
	}
}

void CJabberProto::OnProcessRegIq(HXML node, ThreadData *info)
{
	if (!XmlGetName(node) || mir_wstrcmp(XmlGetName(node), L"iq")) return;
	LPCTSTR type = XmlGetAttrValue(node, L"type");
	if (type == nullptr)
		return;

	int id = JabberGetPacketID(node);

	if (!mir_wstrcmp(type, L"result")) {
		HXML queryNode = XmlGetChild(node, L"query");
		if (queryNode != nullptr) {
			LPCTSTR str = XmlGetAttrValue(queryNode, L"xmlns");
			if (!mir_wstrcmp(str, JABBER_FEAT_REGISTER)) {
				HXML xNode = XmlGetChild(queryNode, L"x");
				if (xNode != nullptr) {
					if (!mir_wstrcmp(XmlGetAttrValue(xNode, L"xmlns"), JABBER_FEAT_DATA_FORMS)) {
						m_regInfo = info;
						FormCreateDialog(xNode, L"Jabber register new user", &CJabberProto::SetRegConfig, mir_wstrdup(XmlGetAttrValue(node, L"from")));
						return;
					}
				}
			}
		}
		// RECVED: result of the request for registration mechanism
		// ACTION: send account registration information
		if (id == iqIdRegGetReg) {
			iqIdRegSetReg = SerialNext();

			XmlNodeIq iq(L"set", iqIdRegSetReg);
			HXML query = iq << XQUERY(JABBER_FEAT_REGISTER);
			query << XCHILD(L"password", info->conn.password);
			query << XCHILD(L"username", info->conn.username);
			info->send(iq);

			SendMessage(info->conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 75, (LPARAM)TranslateT("Sending registration information..."));
		}
		// RECVED: result of the registration process
		// ACTION: account registration successful
		else if (id == iqIdRegSetReg) {
			info->send("</stream:stream>");
			SendMessage(info->conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)TranslateT("Registration successful"));
			info->reg_done = TRUE;
		}
	}

	else if (!mir_wstrcmp(type, L"error")) {
		wchar_t *str = JabberErrorMsg(XmlGetChild(node, "error"));
		SendMessage(info->conn.reg_hwndDlg, WM_JABBER_REGDLG_UPDATE, 100, (LPARAM)str);
		mir_free(str);
		info->reg_done = TRUE;
		info->send("</stream:stream>");
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// Carbons -- this might need to go into its own module

void CJabberProto::EnableCarbons(bool bEnable)
{
	m_ThreadInfo->send(XmlNodeIq(L"set", SerialNext())
		<< XCHILDNS((bEnable) ? L"enable" : L"disable", JABBER_FEAT_CARBONS));
}


/////////////////////////////////////////////////////////////////////////////////////////
// ThreadData constructor & destructor

ThreadData::ThreadData(CJabberProto *_pro, JABBER_CONN_DATA *param)
{
	memset(this, 0, sizeof(*this));

	resolveID = -1;
	proto = _pro;
	iomutex = CreateMutex(nullptr, FALSE, nullptr);

	if (param != nullptr) {
		bIsReg = true;
		memcpy(&conn, param, sizeof(conn));
	}
}

ThreadData::~ThreadData()
{
	if (!bIsReg && proto->m_ThreadInfo == this)
		proto->m_ThreadInfo = nullptr;

	delete auth;
	
	mir_free(zRecvData);
	mir_free(buffer);
	
	CloseHandle(iomutex);
}

void ThreadData::close(void)
{
	if (s) {
		Netlib_CloseHandle(s);
		s = nullptr;
	}
}

void ThreadData::shutdown(void)
{
	if (s)
		Netlib_Shutdown(s);
}

int ThreadData::recvws(char* buf, size_t len, int flags)
{
	if (this == nullptr)
		return 0;

	return proto->WsRecv(s, buf, (int)len, flags);
}

int ThreadData::recv(char* buf, size_t len)
{
	if (useZlib)
		return zlibRecv(buf, (long)len);

	return recvws(buf, len, MSG_DUMPASTEXT);
}

int ThreadData::sendws(char* buf, size_t bufsize, int flags)
{
	return proto->WsSend(s, buf, (int)bufsize, flags);
}

int ThreadData::send(char* buf, int bufsize)
{
	if (this == nullptr)
		return 0;

	if (bufsize == -1)
		bufsize = (int)mir_strlen(buf);

	WaitForSingleObject(iomutex, 6000);

	int result;
	if (useZlib)
		result = zlibSend(buf, bufsize);
	else
		result = sendws(buf, bufsize, MSG_DUMPASTEXT);

	ReleaseMutex(iomutex);
	return result;
}

// Caution: DO NOT use ->send() to send binary (non-string) data
int ThreadData::send(HXML node)
{
	if (this == nullptr)
		return 0;

	while (HXML parent = xmlGetParent(node))
		node = parent;

	if (proto->m_sendManager.HandleSendPermanent(node, this))
		return 0;

	proto->OnConsoleProcessXml(node, JCPF_OUT);

	wchar_t *str = xmlToString(node, nullptr);

	// strip forbidden control characters from outgoing XML stream
	wchar_t *q = str;
	for (wchar_t *p = str; *p; ++p) {

		WCHAR c = *p;
		if (c < 32 && c != '\t' && c != '\n' && c != '\r')
			continue;

		*q++ = *p;
	}
	*q = 0;


	T2Utf utfStr(str);
	int result = send(utfStr, (int)mir_strlen(utfStr));

	xmlFree(str);
	return result;
}
