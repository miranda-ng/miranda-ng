/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

static void removeSpaces(wchar_t* p)
{
	while (*p) {
		if (*p == ' ')
			memmove(p, p + 1, sizeof(wchar_t)*mir_wstrlen(p));
		p++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIrcProto::ReadSettings(TDbSetting *sets, int count)
{
	uint8_t *base = (uint8_t *)this;

	DBVARIANT dbv;
	for (int i = 0; i < count; i++) {
		TDbSetting *p = &sets[i];
		uint8_t *ptr = base + p->offset;
		switch (p->type) {
		case DBVT_BYTE:
			*(uint8_t*)ptr = getByte(p->name, p->defValue);
			break;
		case DBVT_WORD:
			*(uint16_t*)ptr = getWord(p->name, p->defValue);
			break;
		case DBVT_DWORD:
			*(uint32_t*)ptr = getDword(p->name, p->defValue);
			break;
		case DBVT_ASCIIZ:
			if (!getString(p->name, &dbv)) {
				if (p->size != -1) {
					size_t len = min(p->size - 1, mir_strlen(dbv.pszVal));
					memcpy(ptr, dbv.pszVal, len);
					ptr[len] = 0;
				}
				else *(char**)ptr = mir_strdup(dbv.pszVal);
				db_free(&dbv);
			}
			else {
				if (p->size != -1)
					*ptr = 0;
				else
					*(char**)ptr = nullptr;
			}
			break;
		case DBVT_WCHAR:
			if (!getWString(p->name, &dbv)) {
				if (p->size != -1) {
					size_t len = min(p->size - 1, mir_wstrlen(dbv.pwszVal));
					memcpy(ptr, dbv.pszVal, len * sizeof(wchar_t));
					*(wchar_t*)&ptr[len * sizeof(wchar_t)] = 0;
				}
				else *(wchar_t**)ptr = mir_wstrdup(dbv.pwszVal);
				db_free(&dbv);
			}
			else {
				if (p->size != -1) {
					if (p->defStr == nullptr)
						*ptr = 0;
					else
						mir_wstrncpy((wchar_t*)ptr, p->defStr, (int)p->size);
				}
				else *(wchar_t**)ptr = mir_wstrdup(p->defStr);
			}
			break;
		}
	}
}

void CIrcProto::WriteSettings(TDbSetting *sets, int count)
{
	uint8_t *base = (uint8_t*)this;

	for (int i = 0; i < count; i++) {
		TDbSetting *p = &sets[i];
		uint8_t *ptr = base + p->offset;
		switch (p->type) {
		case DBVT_BYTE:   setByte(p->name, *(uint8_t*)ptr);       break;
		case DBVT_WORD:   setWord(p->name, *(uint16_t*)ptr);       break;
		case DBVT_DWORD:  setDword(p->name, *(uint32_t*)ptr);     break;

		case DBVT_ASCIIZ:
			if (p->size == -1)
				setString(p->name, *(char**)ptr);
			else
				setString(p->name, (char*)ptr);
			break;

		case DBVT_WCHAR:
			if (p->size == -1)
				setWString(p->name, *(wchar_t**)ptr);
			else
				setWString(p->name, (wchar_t*)ptr);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// add icons to the skinning module

static IconItem iconList[] =
{
	{ LPGEN("Main"),              "main",    IDI_MAIN,        0 },
	{ LPGEN("Add"),               "add",     IDI_ADD,         0 },
	{ LPGEN("Apply"),             "apply",   IDI_APPLY,       0 },
	{ LPGEN("Rename"),            "rename",  IDI_RENAME,      0 },
	{ LPGEN("Edit"),              "edit",    IDI_EDIT,        0 },
	{ LPGEN("Cancel"),            "delete",  IDI_DELETE,      0 },
	{ LPGEN("Ignore"),            "block",   IDI_BLOCK,       0 },
	{ LPGEN("Channel list"),      "list",    IDI_LIST,        0 },
	{ LPGEN("Channel manager"),   "manager", IDI_MANAGER,     0 },
	{ LPGEN("Server window"),     "server",  IDI_SERVER,      0 },
	{ LPGEN("Show channel"),      "show",    IDI_SHOW,        0 },
	{ LPGEN("Question"),          "question",IDI_IRCQUESTION, 0 },
	{ LPGEN("WhoIs"),             "whois",   IDI_WHOIS,       0 },
	{ LPGEN("Incoming DCC Chat"), "dcc",     IDI_DCC,         0 },
	{ LPGEN("Logo (48x48)"),      "logo",    IDI_LOGO,       48 }
};

void InitIcons(void)
{
	g_plugin.registerIcon("Protocols/IRC", iconList, "IRC");
}

/////////////////////////////////////////////////////////////////////////////////////////
// code page handler

struct { UINT cpId; wchar_t *cpName; } static cpTable[] =
{
	{	  874, LPGENW("Thai") },
	{	  932, LPGENW("Japanese") },
	{	  936, LPGENW("Simplified Chinese") },
	{	  949, LPGENW("Korean") },
	{	  950, LPGENW("Traditional Chinese") },
	{	 1250, LPGENW("Central European") },
	{	 1251, LPGENW("Cyrillic (Windows)") },
	{	20866, LPGENW("Cyrillic (KOI8R)") },
	{	 1252, LPGENW("Latin I") },
	{	 1253, LPGENW("Greek") },
	{	 1254, LPGENW("Turkish") },
	{	 1255, LPGENW("Hebrew") },
	{	 1256, LPGENW("Arabic") },
	{	 1257, LPGENW("Baltic") },
	{	 1258, LPGENW("Vietnamese") },
	{	 1361, LPGENW("Korean (Johab)") }
};

static CCtrlCombo *sttCombo;

static BOOL CALLBACK sttLangAddCallback(wchar_t *str)
{
	UINT cp = _wtoi(str);
	CPINFOEX cpinfo;
	if (GetCPInfoEx(cp, 0, &cpinfo)) {
		wchar_t *b = wcschr(cpinfo.CodePageName, '(');
		if (b) {
			wchar_t *e = wcsrchr(cpinfo.CodePageName, ')');
			if (e) {
				*e = 0;
				sttCombo->AddString(b + 1, cp);
			}
			else sttCombo->AddString(cpinfo.CodePageName, cp);
		}
		else sttCombo->AddString(cpinfo.CodePageName, cp);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Connect preferences' dialog

static TDbSetting ConnectSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_userID), "UserID", DBVT_WCHAR, _countof(CIrcProto::m_userID) },
	{ FIELD_OFFSET(CIrcProto, m_identSystem), "IdentSystem", DBVT_WCHAR, _countof(CIrcProto::m_identSystem) },
	{ FIELD_OFFSET(CIrcProto, m_identPort), "IdentPort", DBVT_WCHAR, _countof(CIrcProto::m_identPort) },

	{ FIELD_OFFSET(CIrcProto, m_serverName), "ServerName", DBVT_ASCIIZ, _countof(CIrcProto::m_serverName) },
	{ FIELD_OFFSET(CIrcProto, m_portStart), "PortStart", DBVT_ASCIIZ, _countof(CIrcProto::m_portStart) },
	{ FIELD_OFFSET(CIrcProto, m_portEnd), "PortEnd", DBVT_ASCIIZ, _countof(CIrcProto::m_portEnd) },
	{ FIELD_OFFSET(CIrcProto, m_password), "Password", DBVT_ASCIIZ, _countof(CIrcProto::m_password) },
	{ FIELD_OFFSET(CIrcProto, m_joinOnInvite), "JoinOnInvite", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_iSSL), "UseSSL", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_onlineNotificationTime) , "OnlineNotificationTime", DBVT_WORD, 0, 30 },
	{ FIELD_OFFSET(CIrcProto, m_onlineNotificationLimit) , "OnlineNotificationLimit", DBVT_WORD, 0, 50 },
	{ FIELD_OFFSET(CIrcProto, m_channelAwayNotification), "ChannelAwayNotification", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_nick), "Nick", DBVT_WCHAR, _countof(CIrcProto::m_nick) },
	{ FIELD_OFFSET(CIrcProto, m_pNick), "PNick", DBVT_WCHAR, _countof(CIrcProto::m_pNick) },
	{ FIELD_OFFSET(CIrcProto, m_alternativeNick), "AlernativeNick", DBVT_WCHAR, _countof(CIrcProto::m_alternativeNick) },
	{ FIELD_OFFSET(CIrcProto, m_name), "Name", DBVT_WCHAR, _countof(CIrcProto::m_name) },
	{ FIELD_OFFSET(CIrcProto, m_ident), "Ident", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_identTimer), "IdentTimer", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_forceVisible), "ForceVisible", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_disableErrorPopups), "DisableErrorPopups", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_rejoinChannels), "RejoinChannels", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_rejoinIfKicked), "RejoinIfKicked", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_showAddresses), "ShowAddresses", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_oldStyleModes), "OldStyleModes", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_useServer), "UseServer", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_bUseSASL), "UseSASL", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_hideServerWindow), "HideServerWindow", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_sendKeepAlive), "SendKeepAlive", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_autoOnlineNotification), "AutoOnlineNotification", DBVT_BYTE },
};

class CConnectPrefsDlg : public CIrcBaseDlg
{
	CCtrlEdit    m_server, m_port, m_port2, m_pass;
	CCtrlEdit    m_nick, m_nick2, m_name, m_userID;
	CCtrlCombo   m_ssl;

	CCtrlCheck   m_ident, m_identTimer;
	CCtrlEdit    m_identSystem, m_identPort;

	CCtrlCheck   m_forceVisible, m_rejoinOnKick, m_rejoinChannels, m_disableError;
	CCtrlCheck   m_address, m_useServer, m_showServer, m_keepAlive, m_autoJoin;
	CCtrlCheck   m_oldStyle, m_onlineNotif, m_channelAway, m_useSasl;

	CCtrlEdit    m_onlineTimer, m_limit;
	CCtrlSpin    m_spin1, m_spin2;

public:
	CConnectPrefsDlg::CConnectPrefsDlg(CIrcProto *_pro) :
		CIrcBaseDlg(_pro, IDD_PREFS_CONNECT),
		m_server(this, IDC_SERVER),
		m_port(this, IDC_PORT),
		m_port2(this, IDC_PORT2),
		m_pass(this, IDC_PASS),
		m_nick(this, IDC_NICK),
		m_nick2(this, IDC_NICK2),
		m_name(this, IDC_NAME),
		m_userID(this, IDC_USERID),
		m_ident(this, IDC_IDENT),
		m_identSystem(this, IDC_IDENTSYSTEM),
		m_identPort(this, IDC_IDENTPORT),
		m_identTimer(this, IDC_IDENT_TIMED),
		m_forceVisible(this, IDC_FORCEVISIBLE),
		m_rejoinOnKick(this, IDC_REJOINONKICK),
		m_rejoinChannels(this, IDC_REJOINCHANNELS),
		m_disableError(this, IDC_DISABLEERROR),
		m_address(this, IDC_ADDRESS),
		m_useServer(this, IDC_USESERVER),
		m_showServer(this, IDC_SHOWSERVER),
		m_keepAlive(this, IDC_KEEPALIVE),
		m_autoJoin(this, IDC_AUTOJOIN),
		m_oldStyle(this, IDC_OLDSTYLE),
		m_useSasl(this, IDC_SASL),
		m_onlineNotif(this, IDC_ONLINENOTIF),
		m_channelAway(this, IDC_CHANNELAWAY),
		m_onlineTimer(this, IDC_ONLINETIMER),
		m_limit(this, IDC_LIMIT),
		m_spin1(this, IDC_SPIN1, 999, 20),
		m_spin2(this, IDC_SPIN2, 200),
		m_ssl(this, IDC_SSL)
	{
		m_ident.OnChange = Callback(this, &CConnectPrefsDlg::OnIdent);
		m_useServer.OnChange = Callback(this, &CConnectPrefsDlg::OnUseServer);
		m_onlineNotif.OnChange = Callback(this, &CConnectPrefsDlg::OnOnlineNotif);
		m_channelAway.OnChange = Callback(this, &CConnectPrefsDlg::OnChannelAway);
	}

	bool OnInitDialog() override
	{
		m_proto->m_hwndConnect = m_hwnd;

		m_server.SetTextA(m_proto->m_serverName);
		m_port.SetTextA(m_proto->m_portStart);
		m_port2.SetTextA(m_proto->m_portEnd);
		m_pass.SetTextA(m_proto->m_password);
		m_useSasl.SetState(m_proto->m_bUseSASL);

		m_ssl.AddString(TranslateT("Off"), 0);
		m_ssl.AddString(TranslateT("Auto"), 1);
		m_ssl.AddString(TranslateT("On"), 2);
		m_ssl.SelectData(m_proto->m_iSSL);

		m_spin1.SetPosition(m_proto->m_onlineNotificationTime);
		m_spin2.SetPosition(m_proto->m_onlineNotificationLimit);

		m_nick.SetText(m_proto->m_nick);
		m_nick2.SetText(m_proto->m_alternativeNick);
		m_userID.SetText(m_proto->m_userID);
		m_name.SetText(m_proto->m_name);
		m_identSystem.SetText(m_proto->m_identSystem);
		m_identPort.SetText(m_proto->m_identPort);
		m_address.SetState(m_proto->m_showAddresses);
		m_oldStyle.SetState(m_proto->m_oldStyleModes);
		m_channelAway.SetState(m_proto->m_channelAwayNotification);
		m_onlineNotif.SetState(m_proto->m_autoOnlineNotification);
		m_ident.SetState(m_proto->m_ident);
		m_identTimer.SetState(m_proto->m_identTimer);
		m_disableError.SetState(m_proto->m_disableErrorPopups);
		m_forceVisible.SetState(m_proto->m_forceVisible);
		m_rejoinChannels.SetState(m_proto->m_rejoinChannels);
		m_rejoinOnKick.SetState(m_proto->m_rejoinIfKicked);
		m_keepAlive.SetState(m_proto->m_sendKeepAlive);
		m_useServer.SetState(m_proto->m_useServer);
		m_showServer.SetState(!m_proto->m_hideServerWindow);
		m_showServer.Enable(m_proto->m_useServer);
		m_autoJoin.SetState(m_proto->m_joinOnInvite);
		return true;
	}

	bool OnApply() override
	{
		// Save the setting in the CONNECT dialog
		m_server.GetTextA(m_proto->m_serverName, _countof(m_proto->m_serverName));
		m_port.GetTextA(m_proto->m_portStart, _countof(m_proto->m_portStart));
		m_port2.GetTextA(m_proto->m_portEnd, _countof(m_proto->m_portEnd));
		m_pass.GetTextA(m_proto->m_password, _countof(m_proto->m_password));
		m_proto->m_iSSL = m_ssl.GetCurData();
		m_proto->m_bUseSASL = m_useSasl.GetState();

		m_proto->m_onlineNotificationTime = SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_GETPOS, 0, 0);
		m_proto->m_onlineNotificationLimit = SendDlgItemMessage(m_hwnd, IDC_SPIN2, UDM_GETPOS, 0, 0);
		m_proto->m_channelAwayNotification = m_channelAway.GetState();

		m_nick.GetText(m_proto->m_nick, _countof(m_proto->m_nick));
		removeSpaces(m_proto->m_nick);
		wcsncpy_s(m_proto->m_pNick, m_proto->m_nick, _TRUNCATE);
		m_nick2.GetText(m_proto->m_alternativeNick, _countof(m_proto->m_alternativeNick));
		removeSpaces(m_proto->m_alternativeNick);
		m_userID.GetText(m_proto->m_userID, _countof(m_proto->m_userID));
		removeSpaces(m_proto->m_userID);
		m_name.GetText(m_proto->m_name, _countof(m_proto->m_name));
		m_identSystem.GetText(m_proto->m_identSystem, _countof(m_proto->m_identSystem));
		m_identPort.GetText(m_proto->m_identPort, _countof(m_proto->m_identPort));
		m_proto->m_ident = m_ident.GetState();
		m_proto->m_identTimer = m_identTimer.GetState();
		m_proto->m_forceVisible = m_forceVisible.GetState();
		m_proto->m_disableErrorPopups = m_disableError.GetState();
		m_proto->m_rejoinChannels = m_rejoinChannels.GetState();
		m_proto->m_rejoinIfKicked = m_rejoinOnKick.GetState();
		m_proto->m_showAddresses = m_address.GetState();
		m_proto->m_oldStyleModes = m_oldStyle.GetState();
		m_proto->m_useServer = m_useServer.GetState();

		Menu_EnableItem(m_proto->hMenuServer, m_proto->m_useServer != 0);

		m_proto->m_joinOnInvite = m_autoJoin.GetState();
		m_proto->m_hideServerWindow = !m_showServer.GetState();
		if (m_proto->m_sendKeepAlive = m_keepAlive.GetState())
			m_proto->SetChatTimer(m_proto->KeepAliveTimer, 60 * 1000, KeepAliveTimerProc);
		else
			m_proto->KillChatTimer(m_proto->KeepAliveTimer);

		m_proto->m_autoOnlineNotification = m_onlineNotif.GetState();
		if (m_proto->m_autoOnlineNotification) {
			if (!m_proto->bTempDisableCheck) {
				m_proto->SetChatTimer(m_proto->OnlineNotifTimer, 500, OnlineNotifTimerProc);
				if (m_proto->m_channelAwayNotification)
					m_proto->SetChatTimer(m_proto->OnlineNotifTimer3, 1500, OnlineNotifTimerProc3);
			}
		}
		else if (!m_proto->bTempForceCheck) {
			m_proto->KillChatTimer(m_proto->OnlineNotifTimer);
			m_proto->KillChatTimer(m_proto->OnlineNotifTimer3);
		}

		m_proto->WriteSettings(ConnectSettings, _countof(ConnectSettings));
		return true;
	}

	void OnIdent(CCtrlData *)
	{
		m_identSystem.Enable(m_ident.GetState());
		m_identPort.Enable(m_ident.GetState());
		m_identTimer.Enable(m_ident.GetState());
	}

	void OnUseServer(CCtrlData *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSERVER), m_useServer.GetState());
	}

	void OnOnlineNotif(CCtrlData *)
	{
		m_channelAway.Enable(m_onlineNotif.GetState());
		m_onlineTimer.Enable(m_onlineNotif.GetState());
		m_spin1.Enable(m_onlineNotif.GetState());
		m_spin2.Enable(m_onlineNotif.GetState());
		m_limit.Enable(m_onlineNotif.GetState() && m_channelAway.GetState());
	}

	void OnChannelAway(CCtrlData *)
	{
		m_spin2.Enable(m_onlineNotif.GetState() && m_channelAway.GetState());
		m_limit.Enable(m_onlineNotif.GetState() && m_channelAway.GetState());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// 'CTCP preferences' dialog

static TDbSetting CtcpSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_userInfo), "UserInfo", DBVT_WCHAR, _countof(CIrcProto::m_userInfo) },
	{ FIELD_OFFSET(CIrcProto, m_DCCPacketSize), "DccPacketSize", DBVT_WORD, 0, 4096 },
	{ FIELD_OFFSET(CIrcProto, m_DCCPassive), "DccPassive", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_DCCMode), "DCCMode", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_manualHost), "ManualHost", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_IPFromServer), "IPFromServer", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_disconnectDCCChats), "DisconnectDCCChats", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_mySpecifiedHost), "SpecHost", DBVT_ASCIIZ, _countof(CIrcProto::m_mySpecifiedHost) },
	{ FIELD_OFFSET(CIrcProto, m_DCCChatAccept), "CtcpChatAccept", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_sendNotice), "SendNotice", DBVT_BYTE, 0, 1 }
};

class CCtcpPrefsDlg : public CIrcBaseDlg
{
	CCtrlCombo m_combo;
	CCtrlCheck m_slow, m_fast, m_disc, m_passive, m_sendNotice, m_enableIP, m_fromServer;
	CCtrlEdit m_ip, m_userInfo;
	CCtrlCheck m_radio1, m_radio2, m_radio3;

public:
	CCtcpPrefsDlg(CIrcProto *_pro) :
		CIrcBaseDlg(_pro, IDD_PREFS_CTCP),
		m_enableIP(this, IDC_ENABLEIP),
		m_fromServer(this, IDC_FROMSERVER),
		m_combo(this, IDC_COMBO),
		m_slow(this, IDC_SLOW),
		m_fast(this, IDC_FAST),
		m_disc(this, IDC_DISC),
		m_passive(this, IDC_PASSIVE),
		m_sendNotice(this, IDC_SENDNOTICE),
		m_ip(this, IDC_IP),
		m_userInfo(this, IDC_USERINFO),
		m_radio1(this, IDC_RADIO1),
		m_radio2(this, IDC_RADIO2),
		m_radio3(this, IDC_RADIO3)
	{
		m_enableIP.OnChange = Callback(this, &CCtcpPrefsDlg::OnClicked);
		m_fromServer.OnChange = Callback(this, &CCtcpPrefsDlg::OnClicked);
	}

	bool OnInitDialog() override
	{
		m_userInfo.SetText(m_proto->m_userInfo);

		m_slow.SetState(m_proto->m_DCCMode == 0);
		m_fast.SetState(m_proto->m_DCCMode == 1);
		m_disc.SetState(m_proto->m_disconnectDCCChats);
		m_passive.SetState(m_proto->m_DCCPassive);
		m_sendNotice.SetState(m_proto->m_sendNotice);

		m_combo.AddStringA("256");
		m_combo.AddStringA("512");
		m_combo.AddStringA("1024");
		m_combo.AddStringA("2048");
		m_combo.AddStringA("4096");
		m_combo.AddStringA("8192");

		wchar_t szTemp[10];
		mir_snwprintf(szTemp, L"%u", m_proto->m_DCCPacketSize);
		int i = m_combo.SelectString(szTemp);
		if (i == CB_ERR)
			m_combo.SelectString(L"4096");

		if (m_proto->m_DCCChatAccept == 1)
			m_radio1.SetState(true);
		if (m_proto->m_DCCChatAccept == 2)
			m_radio2.SetState(true);
		if (m_proto->m_DCCChatAccept == 3)
			m_radio3.SetState(true);

		m_fromServer.SetState(m_proto->m_IPFromServer);
		m_enableIP.SetState(m_proto->m_manualHost);
		m_ip.Enable(m_proto->m_manualHost);
		m_fromServer.Enable(!m_proto->m_manualHost);
		if (m_proto->m_manualHost)
			m_ip.SetTextA(m_proto->m_mySpecifiedHost);
		else {
			if (m_proto->m_IPFromServer) {
				if (m_proto->m_myHost[0]) {
					CMStringW s = (CMStringW)TranslateT("<Resolved IP: ") + (wchar_t *)_A2T(m_proto->m_myHost) + L">";
					m_ip.SetText(s.c_str());
				}
				else m_ip.SetText(TranslateT("<Automatic>"));
			}
			else {
				if (m_proto->m_myLocalHost[0]) {
					CMStringW s = (CMStringW)TranslateT("<Local IP: ") + (wchar_t *)_A2T(m_proto->m_myLocalHost) + L">";
					m_ip.SetText(s.c_str());
				}
				else m_ip.SetText(TranslateT("<Automatic>"));
			}
		}
		return true;
	}

	bool OnApply() override
	{
		m_userInfo.GetText(m_proto->m_userInfo, _countof(m_proto->m_userInfo));

		m_proto->m_DCCPacketSize = m_combo.GetInt();
		m_proto->m_DCCPassive = m_passive.GetState();
		m_proto->m_sendNotice = m_sendNotice.GetState();
		m_proto->m_DCCMode = m_fast.GetState();
		m_proto->m_manualHost = m_enableIP.GetState();
		m_proto->m_IPFromServer = m_fromServer.GetState();
		m_proto->m_disconnectDCCChats = m_disc.GetState();

		if (m_enableIP.GetState()) {
			char szTemp[500];
			m_ip.GetTextA(szTemp, sizeof(szTemp));
			mir_strncpy(m_proto->m_mySpecifiedHost, GetWord(szTemp, 0).c_str(), 499);
			if (mir_strlen(m_proto->m_mySpecifiedHost))
				m_proto->ForkThread(&CIrcProto::ResolveIPThread, new IPRESOLVE(m_proto->m_mySpecifiedHost, IP_MANUAL));
		}
		else m_proto->m_mySpecifiedHost[0] = 0;

		if (m_radio1.GetState())
			m_proto->m_DCCChatAccept = 1;
		if (m_radio2.GetState())
			m_proto->m_DCCChatAccept = 2;
		if (m_radio3.GetState())
			m_proto->m_DCCChatAccept = 3;

		m_proto->WriteSettings(CtcpSettings, _countof(CtcpSettings));
		return true;
	}

	void OnClicked(CCtrlData *)
	{
		m_ip.Enable(m_enableIP.GetState());
		m_fromServer.Enable(!m_enableIP.GetState());

		if (m_enableIP.GetState())
			m_ip.SetTextA(m_proto->m_mySpecifiedHost);
		else {
			if (m_fromServer.GetState()) {
				if (m_proto->m_myHost[0]) {
					CMStringW s = (CMStringW)TranslateT("<Resolved IP: ") + (wchar_t *)_A2T(m_proto->m_myHost) + L">";
					m_ip.SetText(s.c_str());
				}
				else m_ip.SetText(TranslateT("<Automatic>"));
			}
			else {
				if (m_proto->m_myLocalHost[0]) {
					CMStringW s = (CMStringW)TranslateT("<Local IP: ") + (wchar_t *)_A2T(m_proto->m_myLocalHost) + L">";
					m_ip.SetText(s.c_str());
				}
				else m_ip.SetText(TranslateT("<Automatic>"));
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// 'Advanced preferences' dialog

static TDbSetting OtherSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_quitMessage), "QuitMessage", DBVT_WCHAR, _countof(CIrcProto::m_quitMessage) },
	{ FIELD_OFFSET(CIrcProto, m_alias), "Alias", DBVT_WCHAR, (size_t)-1 },
	{ FIELD_OFFSET(CIrcProto, m_codepage), "Codepage", DBVT_DWORD, 0, CP_ACP },
	{ FIELD_OFFSET(CIrcProto, m_utfAutodetect), "UtfAutodetect", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_perform), "Perform", DBVT_BYTE },
};

static char* sttPerformEvents[] = {
	LPGEN("Event: Connect"),
	LPGEN("Event: Available"),
	LPGEN("Event: Away"),
	LPGEN("Event: Not available"),
	LPGEN("Event: Occupied"),
	LPGEN("Event: Do not disturb"),
	LPGEN("Event: Free for chat"),
	LPGEN("Event: Disconnect")
};

static LRESULT CALLBACK EditSubclassProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 21 || wParam == 11 || wParam == 2) {
			char w[2];
			w[1] = 0;
			if (wParam == 11)
				w[0] = 3;
			if (wParam == 2)
				w[0] = 2;
			if (wParam == 21)
				w[0] = 31;
			SendMessage(hwndDlg, EM_REPLACESEL, false, (LPARAM)w);
			SendMessage(hwndDlg, EM_SCROLLCARET, 0, 0);
			return 0;
		}
		break;
	}

	return mir_callNextSubclass(hwndDlg, EditSubclassProc, msg, wParam, lParam);
}

class COtherPrefsDlg : public CIrcBaseDlg
{
	bool m_performlistModified;

	CCtrlButton  m_url;
	CCtrlMButton m_add, m_delete;
	CCtrlCombo   m_performCombo, m_codepage;
	CCtrlEdit    m_pertormEdit, m_quitMessage, m_alias;
	CCtrlCheck   m_perform, m_autodetect;

public:
	COtherPrefsDlg(CIrcProto *_pro) :
		CIrcBaseDlg(_pro, IDD_PREFS_OTHER),
		m_url(this, IDC_CUSTOM),
		m_performCombo(this, IDC_PERFORMCOMBO),
		m_codepage(this, IDC_CODEPAGE),
		m_pertormEdit(this, IDC_PERFORMEDIT),
		m_perform(this, IDC_PERFORM),
		m_autodetect(this, IDC_UTF_AUTODETECT),
		m_quitMessage(this, IDC_QUITMESSAGE),
		m_alias(this, IDC_ALIASEDIT),
		m_add(this, IDC_ADD, g_plugin.getIcon(IDI_ADD), LPGEN("Click to set commands that will be performed for this event")),
		m_delete(this, IDC_DELETE, g_plugin.getIcon(IDI_DELETE), LPGEN("Click to delete the commands for this event")),
		m_performlistModified(false)
	{
		m_performCombo.OnChange = Callback(this, &COtherPrefsDlg::OnPerformCombo);
		m_codepage.OnChange = Callback(this, &COtherPrefsDlg::OnCodePage);
		m_pertormEdit.OnChange = Callback(this, &COtherPrefsDlg::OnPerformEdit);
		m_perform.OnChange = Callback(this, &COtherPrefsDlg::OnPerform);
		m_add.OnClick = Callback(this, &COtherPrefsDlg::OnAdd);
		m_delete.OnClick = Callback(this, &COtherPrefsDlg::OnDelete);
	}

	bool OnInitDialog()
	{
		mir_subclassWindow(m_alias.GetHwnd(), EditSubclassProc);
		mir_subclassWindow(m_quitMessage.GetHwnd(), EditSubclassProc);
		mir_subclassWindow(m_pertormEdit.GetHwnd(), EditSubclassProc);

		m_alias.SetText(m_proto->m_alias);
		m_quitMessage.SetText(m_proto->m_quitMessage);
		m_perform.SetState(m_proto->m_perform);
		m_performCombo.Enable(m_proto->m_perform);
		m_pertormEdit.Enable(m_proto->m_perform);
		m_add.Enable(m_proto->m_perform);
		m_delete.Enable(m_proto->m_perform);

		m_codepage.AddString(TranslateT("Default ANSI codepage"), CP_ACP);

		sttCombo = &m_codepage;
		EnumSystemCodePages(sttLangAddCallback, CP_INSTALLED);

		for (int i = m_codepage.GetCount(); i >= 0; i--) {
			if (m_codepage.GetItemData(i) == m_proto->m_codepage) {
				m_codepage.SetCurSel(i);
				break;
			}
		}

		if (m_proto->m_codepage == CP_UTF8)
			m_autodetect.Disable();

		for (auto &it: sttPerformEvents) {
			CMStringA sSetting = CMStringA("PERFORM:") + it;
			sSetting.MakeUpper();

			PERFORM_INFO *pPref = new PERFORM_INFO(sSetting.c_str(), m_proto->getMStringW(sSetting.c_str()));
			m_performCombo.AddString(_A2T(Translate(it)), (LPARAM)pPref);
		}

		m_performCombo.SetCurSel(0);
		OnPerformCombo(nullptr);
		m_autodetect.SetState(m_proto->m_utfAutodetect);
		return true;
	}

	bool OnApply() override
	{
		mir_free(m_proto->m_alias);
		m_proto->m_alias = m_alias.GetText();
		m_quitMessage.GetText(m_proto->m_quitMessage, _countof(m_proto->m_quitMessage));

		m_proto->m_codepage = m_codepage.GetCurData();
		if (m_proto->IsConnected())
			m_proto->setCodepage(m_proto->m_codepage);

		m_proto->m_utfAutodetect = m_autodetect.GetState();
		m_proto->m_perform = m_perform.GetState();
		if (m_add.Enabled())
			OnAdd(nullptr);

		if (m_performlistModified) {
			int count = m_performCombo.GetCount();
			for (int i = 0; i < count; i++) {
				PERFORM_INFO *pPerf = (PERFORM_INFO *)m_performCombo.GetItemData(i);
				if ((INT_PTR)pPerf == CB_ERR)
					continue;

				if (pPerf->mText.IsEmpty())
					m_proto->delSetting(pPerf->mSetting.c_str());
				else
					m_proto->setWString(pPerf->mSetting.c_str(), pPerf->mText.c_str());
			}
		}
		m_proto->WriteSettings(OtherSettings, _countof(OtherSettings));
		return true;
	}

	void OnDestroy() override
	{
		int i = m_performCombo.GetCount();
		if (i != CB_ERR && i != 0) {
			for (int index = 0; index < i; index++) {
				PERFORM_INFO *pPerf = (PERFORM_INFO *)m_performCombo.GetItemData(index);
				if ((INT_PTR)pPerf != CB_ERR && pPerf != nullptr)
					delete pPerf;
			}
		}
	}

	void OnPerformCombo(CCtrlData *)
	{
		PERFORM_INFO *pPerf = (PERFORM_INFO *)m_performCombo.GetCurData();
		if (pPerf == INVALID_HANDLE_VALUE)
			return;

		if (pPerf == nullptr)
			m_pertormEdit.SetTextA("");
		else
			m_pertormEdit.SetText(pPerf->mText.c_str());
		m_add.Disable();
		if (GetWindowTextLength(m_pertormEdit.GetHwnd()) != 0)
			m_delete.Enable();
		else
			m_delete.Disable();
	}

	void OnCodePage(CCtrlData *)
	{
		m_autodetect.Enable(m_codepage.GetCurData() != CP_UTF8);
	}

	void OnPerformEdit(CCtrlData *)
	{
		m_add.Enable();

		if (GetWindowTextLength(m_pertormEdit.GetHwnd()) != 0)
			m_delete.Enable();
		else
			m_delete.Disable();
	}

	void OnPerform(CCtrlData *)
	{
		m_performCombo.Enable(m_perform.GetState());
		m_pertormEdit.Enable(m_perform.GetState());
		m_add.Enable(m_perform.GetState());
		m_delete.Enable(m_perform.GetState());
	}

	void OnAdd(CCtrlButton *)
	{
		ptrW temp(m_pertormEdit.GetText());

		if (mir_wstrstri(temp, L"/away"))
			MessageBox(nullptr, TranslateT("The usage of /AWAY in your perform buffer is restricted\n as IRC sends this command automatically."), TranslateT("IRC Error"), MB_OK);
		else {
			PERFORM_INFO *pPerf = (PERFORM_INFO *)m_performCombo.GetCurData();
			if (pPerf == INVALID_HANDLE_VALUE)
				return;

			if (pPerf != nullptr)
				pPerf->mText = temp;

			m_add.Disable();
			m_performlistModified = true;
		}
	}

	void OnDelete(CCtrlButton *)
	{
		PERFORM_INFO *pPerf = (PERFORM_INFO *)m_performCombo.GetCurData();
		if (pPerf == INVALID_HANDLE_VALUE)
			return;

		if (pPerf != nullptr) {
			pPerf->mText = L"";
			m_pertormEdit.SetTextA("");
			m_delete.Disable();
			m_add.Disable();
		}

		m_performlistModified = true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// 'add ignore' preferences dialog

struct CAddIgnoreDlg : public CIrcBaseDlg
{
	wchar_t szOldMask[500];

	CAddIgnoreDlg::CAddIgnoreDlg(CIrcProto *_pro, const wchar_t *mask, CDlgBase *_owner) :
		CIrcBaseDlg(_pro, IDD_ADDIGNORE)
	{
		m_hwndParent = _owner->GetHwnd();

		if (mask == nullptr)
			szOldMask[0] = 0;
		else
			wcsncpy(szOldMask, mask, _countof(szOldMask));
	}

	bool OnInitDialog() override
	{
		if (szOldMask[0] == 0) {
			CheckDlgButton(m_hwnd, IDC_Q, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_N, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_I, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_D, BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_C, BST_CHECKED);
		}
		return true;
	}

	bool OnApply() override
	{
		wchar_t szMask[500];
		CMStringW flags;
		if (IsDlgButtonChecked(m_hwnd, IDC_Q) == BST_CHECKED) flags += 'q';
		if (IsDlgButtonChecked(m_hwnd, IDC_N) == BST_CHECKED) flags += 'n';
		if (IsDlgButtonChecked(m_hwnd, IDC_I) == BST_CHECKED) flags += 'i';
		if (IsDlgButtonChecked(m_hwnd, IDC_D) == BST_CHECKED) flags += 'd';
		if (IsDlgButtonChecked(m_hwnd, IDC_C) == BST_CHECKED) flags += 'c';
		if (IsDlgButtonChecked(m_hwnd, IDC_M) == BST_CHECKED) flags += 'm';

		GetDlgItemText(m_hwnd, IDC_MASK, szMask, _countof(szMask));

		CMStringW Mask = GetWord(szMask, 0);
		if (Mask.GetLength() != 0) {
			if (!wcschr(Mask.c_str(), '!') && !wcschr(Mask.c_str(), '@'))
				Mask += L"!*@*";

			if (!flags.IsEmpty()) {
				if (*szOldMask)
					m_proto->RemoveIgnore(szOldMask);
				m_proto->AddIgnore(Mask.c_str(), flags.c_str());
			}
		}
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// 'Ignore' preferences dialog

static TDbSetting IgnoreSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_DCCFileEnabled), "EnableCtcpFile", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_DCCChatEnabled), "EnableCtcpChat", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_DCCChatIgnore), "CtcpChatIgnore", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_ignore), "Ignore", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_ignoreChannelDefault), "IgnoreChannelDefault", DBVT_BYTE },
};

static LRESULT CALLBACK ListviewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYUP:
		if (ListView_GetSelectionMark(GetDlgItem(GetParent(hwnd), IDC_LIST)) != -1) {
			EnableWindow(GetDlgItem(GetParent(hwnd), IDC_EDIT), true);
			EnableWindow(GetDlgItem(GetParent(hwnd), IDC_DELETE), true);
		}
		else {
			EnableWindow(GetDlgItem(GetParent(hwnd), IDC_EDIT), false);
			EnableWindow(GetDlgItem(GetParent(hwnd), IDC_DELETE), false);
		}

		if (wParam == VK_DELETE)
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_DELETE, BN_CLICKED), 0);

		if (wParam == VK_SPACE)
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_EDIT, BN_CLICKED), 0);
		break;
	}

	return mir_callNextSubclass(hwnd, ListviewSubclassProc, msg, wParam, lParam);
}

// Callback for the 'Add ignore' dialog

void CIrcProto::InitIgnore(void)
{
	wchar_t szTemp[MAX_PATH];
	mir_snwprintf(szTemp, L"%%miranda_path%%\\Plugins\\%S_ignore.ini", m_szModuleName);
	wchar_t *szLoadFileName = Utils_ReplaceVarsW(szTemp);
	char* pszIgnoreData = IrcLoadFile(szLoadFileName);
	if (pszIgnoreData != nullptr) {
		char *p1 = pszIgnoreData;
		while (*p1 != 0) {
			while (*p1 == '\r' || *p1 == '\n')
				p1++;
			if (*p1 == 0)
				break;

			char* p2 = strstr(p1, "\r\n");
			if (!p2)
				p2 = strchr(p1, 0);

			char* pTemp = p2;
			while (pTemp > p1 && (*pTemp == '\r' || *pTemp == '\n' || *pTemp == 0 || *pTemp == ' '))
				pTemp--;
			*++pTemp = 0;

			CMStringA mask = GetWord(p1, 0);
			CMStringA flags = GetWord(p1, 1);
			if (!mask.IsEmpty())
				m_ignoreItems.insert(new CIrcIgnoreItem(getCodepage(), mask.c_str(), flags.c_str()));

			p1 = p2;
		}

		RewriteIgnoreSettings();
		delete[] pszIgnoreData;
		::_wremove(szLoadFileName);
	}
	mir_free(szLoadFileName);

	int idx = 0;
	char settingName[40];
	for (;;) {
		mir_snprintf(settingName, "IGNORE:%d", idx++);

		DBVARIANT dbv;
		if (getWString(settingName, &dbv))
			break;

		CMStringW mask = GetWord(dbv.pwszVal, 0);
		CMStringW flags = GetWord(dbv.pwszVal, 1);
		m_ignoreItems.insert(new CIrcIgnoreItem(mask.c_str(), flags.c_str()));
		db_free(&dbv);
	}
}

void CIrcProto::RewriteIgnoreSettings(void)
{
	char settingName[40];

	int i = 0;
	for (;;) {
		mir_snprintf(settingName, "IGNORE:%d", i++);
		if (db_unset(0, m_szModuleName, settingName))
			break;
	}

	for (i = 0; i < m_ignoreItems.getCount(); i++) {
		mir_snprintf(settingName, "IGNORE:%d", i);

		CIrcIgnoreItem &C = m_ignoreItems[i];
		setWString(settingName, (C.mask + L" " + C.flags).c_str());
	}
}

class CIgnorePrefsDlg : public CIrcBaseDlg
{
	void FixButtons()
	{
		m_add.Enable(m_enable.GetState());
		if (m_list.GetSelectionMark() != -1) {
			m_edit.Enable();
			m_del.Enable();
		}
		else {
			m_edit.Disable();
			m_del.Disable();
		}
	}

	static int CALLBACK IgnoreListSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		CIgnorePrefsDlg *hwndDlg = (CIgnorePrefsDlg*)lParamSort;
		if (!hwndDlg->GetHwnd())
			return 1;

		wchar_t temp1[512];
		wchar_t temp2[512];

		LVITEM lvm;
		lvm.mask = LVIF_TEXT;
		lvm.iSubItem = 0;
		lvm.cchTextMax = _countof(temp1);

		lvm.iItem = lParam1;
		lvm.pszText = temp1;
		hwndDlg->m_list.GetItem(&lvm);

		lvm.iItem = lParam2;
		lvm.pszText = temp2;
		hwndDlg->m_list.GetItem(&lvm);

		if (temp1[0] && temp2[0])
			return mir_wstrcmpi(temp1, temp2);

		return (temp1[0] == 0) ? 1 : -1;
	}

	void RebuildList()
	{
		m_list.DeleteAllItems();

		for (auto &C : m_proto->m_ignoreItems) {
			if (C->mask.IsEmpty() || C->flags[0] != '+')
				continue;

			LVITEM lvItem;
			lvItem.iItem = m_list.GetItemCount();

			lvItem.iSubItem = 0;
			lvItem.mask = LVIF_TEXT | LVIF_PARAM;
			lvItem.lParam = lvItem.iItem;
			lvItem.pszText = (wchar_t *)C->mask.c_str();
			lvItem.iItem = m_list.InsertItem(&lvItem);

			lvItem.iSubItem = 1;
			lvItem.mask = LVIF_TEXT;
			lvItem.pszText = (wchar_t *)C->flags.c_str();
			m_list.SetItem(&lvItem);
		}

		UpdateList();
		m_list.SortItems(IgnoreListSort, (LPARAM)this);
		UpdateList();

		FixButtons();
	}

	void UpdateList()
	{
		int j = m_list.GetItemCount();
		if (j > 0) {
			LVITEM lvm;
			lvm.mask = LVIF_PARAM;
			lvm.iSubItem = 0;
			for (int i = 0; i < j; i++) {
				lvm.iItem = i;
				lvm.lParam = i;
				m_list.SetItem(&lvm);
			}
		}
	}

	CCtrlMButton m_add, m_edit, m_del;
	CCtrlCheck m_enable, m_ignoreChat, m_ignoreFile, m_ignoreChannel, m_ignoreUnknown;
	CCtrlListView m_list;

public:
	CIgnorePrefsDlg(CIrcProto *_pro) :
		CIrcBaseDlg(_pro, IDD_PREFS_IGNORE),
		m_list(this, IDC_LIST),
		m_add(this, IDC_ADD, g_plugin.getIcon(IDI_ADD), LPGEN("Add new ignore")),
		m_edit(this, IDC_EDIT, g_plugin.getIcon(IDI_EDIT), LPGEN("Edit this ignore")),
		m_del(this, IDC_DELETE, g_plugin.getIcon(IDI_DELETE), LPGEN("Delete this ignore")),
		m_enable(this, IDC_ENABLEIGNORE),
		m_ignoreChat(this, IDC_IGNORECHAT),
		m_ignoreFile(this, IDC_IGNOREFILE),
		m_ignoreChannel(this, IDC_IGNORECHANNEL),
		m_ignoreUnknown(this, IDC_IGNOREUNKNOWN)
	{
		m_enable.OnChange = Callback(this, &CIgnorePrefsDlg::OnEnableIgnore);
		m_ignoreChat.OnChange = Callback(this, &CIgnorePrefsDlg::OnIgnoreChat);
		m_add.OnClick = Callback(this, &CIgnorePrefsDlg::OnAdd);
		m_list.OnDoubleClick = m_edit.OnClick = Callback(this, &CIgnorePrefsDlg::OnEdit);
		m_del.OnClick = Callback(this, &CIgnorePrefsDlg::OnDelete);
		m_list.OnColumnClick = Callback(this, &CIgnorePrefsDlg::List_OnColumnClick);
	}

	bool OnInitDialog() override
	{
		m_proto->m_ignoreDlg = this;
		mir_subclassWindow(m_list.GetHwnd(), ListviewSubclassProc);

		m_enable.SetState(m_proto->m_ignore);
		m_ignoreFile.SetState(!m_proto->m_DCCFileEnabled);
		m_ignoreChat.SetState(!m_proto->m_DCCChatEnabled);
		m_ignoreChannel.SetState(m_proto->m_ignoreChannelDefault);
		if (m_proto->m_DCCChatIgnore == 2)
			m_ignoreUnknown.SetState(BST_CHECKED);

		m_ignoreUnknown.Enable(m_proto->m_DCCChatEnabled);
		m_list.Enable(m_proto->m_ignore);
		m_ignoreChannel.Enable(m_proto->m_ignore);
		m_add.Enable(m_proto->m_ignore);

		LV_COLUMN lvC;
		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 235;
		lvC.pszText = TranslateT("Ignore mask");
		ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_INFO_LISTVIEW), 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 100;
		lvC.pszText = TranslateT("Flags");
		ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_INFO_LISTVIEW), 1, &lvC);

		ListView_SetExtendedListViewStyle(GetDlgItem(m_hwnd, IDC_INFO_LISTVIEW), LVS_EX_FULLROWSELECT);
		RebuildList();
		return true;
	}

	bool OnApply() override
	{
		m_proto->m_DCCFileEnabled = !m_ignoreFile.GetState();
		m_proto->m_DCCChatEnabled = !m_ignoreChat.GetState();
		m_proto->m_ignore = m_enable.GetState();
		m_proto->m_ignoreChannelDefault = m_ignoreChannel.GetState();
		m_proto->m_DCCChatIgnore = m_ignoreUnknown.GetState() ? 2 : 1;
		m_proto->WriteSettings(IgnoreSettings, _countof(IgnoreSettings));
		return true;
	}

	void OnDestroy() override
	{
		m_proto->m_ignoreDlg = nullptr;
		m_proto->m_ignoreItems.destroy();

		int i = m_list.GetItemCount();
		for (int j = 0; j < i; j++) {
			wchar_t szMask[512], szFlags[40];
			m_list.GetItemText(j, 0, szMask, _countof(szMask));
			m_list.GetItemText(j, 1, szFlags, _countof(szFlags));
			m_proto->m_ignoreItems.insert(new CIrcIgnoreItem(szMask, szFlags));
		}

		m_proto->RewriteIgnoreSettings();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
			case IDC_LIST:
				switch (((NMHDR *)lParam)->code) {
				case NM_CLICK:
				case NM_RCLICK:
					if (m_list.GetSelectionMark() != -1)
						FixButtons();
					break;
				}
			}
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void Update() override
	{
		RebuildList();
	}

	void OnEnableIgnore(CCtrlData *)
	{
		m_ignoreChannel.Enable(m_enable.GetState());
		m_list.Enable(m_enable.GetState());
		m_add.Enable(m_enable.GetState());
	}

	void OnIgnoreChat(CCtrlData *)
	{
		m_ignoreUnknown.Enable(m_ignoreChat.GetState() == BST_UNCHECKED);
	}

	void OnAdd(CCtrlButton *)
	{
		CAddIgnoreDlg *dlg = new CAddIgnoreDlg(m_proto, nullptr, this);
		dlg->Show();
		SetWindowText(dlg->GetHwnd(), TranslateT("Add ignore"));
		m_add.Disable();
		m_edit.Disable();
		m_del.Disable();
	}

	void OnEdit(CCtrlButton *)
	{
		if (!m_add.Enabled())
			return;

		wchar_t szMask[512];
		wchar_t szFlags[512];
		wchar_t szNetwork[512];
		int i = m_list.GetSelectionMark();
		m_list.GetItemText(i, 0, szMask, 511);
		m_list.GetItemText(i, 1, szFlags, 511);
		m_list.GetItemText(i, 2, szNetwork, 511);
		CAddIgnoreDlg *dlg = new CAddIgnoreDlg(m_proto, szMask, this);
		dlg->Show();
		HWND hWnd = dlg->GetHwnd();
		SetWindowText(hWnd, TranslateT("Edit ignore"));
		if (szFlags[0]) {
			if (wcschr(szFlags, 'q'))
				CheckDlgButton(hWnd, IDC_Q, BST_CHECKED);
			if (wcschr(szFlags, 'n'))
				CheckDlgButton(hWnd, IDC_N, BST_CHECKED);
			if (wcschr(szFlags, 'i'))
				CheckDlgButton(hWnd, IDC_I, BST_CHECKED);
			if (wcschr(szFlags, 'd'))
				CheckDlgButton(hWnd, IDC_D, BST_CHECKED);
			if (wcschr(szFlags, 'c'))
				CheckDlgButton(hWnd, IDC_C, BST_CHECKED);
			if (wcschr(szFlags, 'm'))
				CheckDlgButton(hWnd, IDC_M, BST_CHECKED);
		}

		SetDlgItemText(hWnd, IDC_MASK, szMask);
		m_add.Disable();
		m_edit.Disable();
		m_del.Disable();
	}

	void OnDelete(CCtrlButton *)
	{
		if (!m_del.Enabled())
			return;

		wchar_t szMask[512];
		int i = m_list.GetSelectionMark();
		m_list.GetItemText(i, 0, szMask, _countof(szMask));
		m_proto->RemoveIgnore(szMask);
	}

	void List_OnColumnClick(CCtrlListView::TEventInfo *)
	{
		m_list.SortItems(IgnoreListSort, (LPARAM)this);
		UpdateList();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int CIrcProto::OnInitOptionsPages(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.szGroup.w = LPGENW("Network");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CConnectPrefsDlg(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("DCC and CTCP");
	odp.pDialog = new CCtcpPrefsDlg(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new COtherPrefsDlg(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Ignore");
	odp.pDialog = new CIgnorePrefsDlg(this);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Account manager UI

class CDlgAccMgrUI : public CIrcBaseDlg
{
	CCtrlEdit m_server, m_port, m_port2, m_pass, m_nick, m_nick2, m_name, m_userID;
	CCtrlCombo m_ssl;

public:
	CDlgAccMgrUI(CIrcProto* _pro, HWND _owner) :
		CIrcBaseDlg(_pro, IDD_ACCMGRUI),
		m_server(this, IDC_SERVER),
		m_port(this, IDC_PORT),
		m_port2(this, IDC_PORT2),
		m_pass(this, IDC_PASS),
		m_nick(this, IDC_NICK),
		m_nick2(this, IDC_NICK2),
		m_name(this, IDC_NAME),
		m_ssl(this, IDC_SSL),
		m_userID(this, IDC_USERID)
	{
		m_hwndParent = _owner;
	}

	bool OnInitDialog() override
	{
		m_server.SetTextA(m_proto->m_serverName);
		m_port.SetTextA(m_proto->m_portStart);
		m_port2.SetTextA(m_proto->m_portEnd);
		m_pass.SetTextA(m_proto->m_password);

		m_ssl.AddString(TranslateT("Off"), 0);
		m_ssl.AddString(TranslateT("Auto"), 1);
		m_ssl.AddString(TranslateT("On"), 2);
		m_ssl.SelectData(m_proto->m_iSSL);

		m_nick.SetText(m_proto->m_nick);
		m_nick2.SetText(m_proto->m_alternativeNick);
		m_userID.SetText(m_proto->m_userID);
		m_name.SetText(m_proto->m_name);
		return true;
	}

	bool OnApply() override
	{
		m_server.GetTextA(m_proto->m_serverName, _countof(m_proto->m_serverName));
		m_port.GetTextA(m_proto->m_portStart, _countof(m_proto->m_portStart));
		m_port2.GetTextA(m_proto->m_portEnd, _countof(m_proto->m_portEnd));
		m_pass.GetTextA(m_proto->m_password, _countof(m_proto->m_password));

		m_proto->m_iSSL = m_ssl.GetCurData();

		m_nick.GetText(m_proto->m_nick, _countof(m_proto->m_nick));
		removeSpaces(m_proto->m_nick);
		wcsncpy_s(m_proto->m_pNick, m_proto->m_nick, _TRUNCATE);
		m_nick2.GetText(m_proto->m_alternativeNick, _countof(m_proto->m_alternativeNick));
		removeSpaces(m_proto->m_alternativeNick);
		m_userID.GetText(m_proto->m_userID, _countof(m_proto->m_userID));
		removeSpaces(m_proto->m_userID);
		m_name.GetText(m_proto->m_name, _countof(m_proto->m_name));
		m_proto->WriteSettings(ConnectSettings, _countof(ConnectSettings));
		return true;
	}
};

INT_PTR CIrcProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	CDlgAccMgrUI *dlg = new CDlgAccMgrUI(this, (HWND)lParam);
	dlg->Show();
	return (INT_PTR)dlg->GetHwnd();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIrcProto::InitPrefs(void)
{
	ConnectSettings[0].defStr = L"Miranda";
	ConnectSettings[1].defStr = L"UNIX";
	ConnectSettings[2].defStr = L"113";
	ConnectSettings[3].defStr = L"30";
	ConnectSettings[4].defStr = L"10";

	CtcpSettings[0].defStr = STR_USERINFO;

	OtherSettings[0].defStr = STR_QUITMESSAGE;

	ReadSettings(ConnectSettings, _countof(ConnectSettings));
	ReadSettings(CtcpSettings, _countof(CtcpSettings));
	ReadSettings(OtherSettings, _countof(OtherSettings));
	ReadSettings(IgnoreSettings, _countof(IgnoreSettings));

	int x = getDword("SizeOfListBottom", -1);
	if (x != -1) {
		db_unset(0, m_szModuleName, "SizeOfListBottom");
		setDword("channelList_height", x);
	}
	if ((x = getDword("SizeOfListWidth", -1)) != -1) {
		db_unset(0, m_szModuleName, "SizeOfListWidth");
		setDword("channelList_width", x);
	}

	if (m_pNick[0] == 0) {
		if (m_nick[0] != 0) {
			memcpy(m_pNick, m_nick, sizeof(m_pNick));
			setWString("PNick", m_nick);
		}
	}
	else {
		memcpy(m_nick, m_pNick, sizeof(m_nick));
		setWString("Nick", m_nick);
	}

	m_mySpecifiedHostIP[0] = 0;

	if (m_alias == nullptr)
		m_alias = mir_wstrdup(L"/op /mode ## +ooo $1 $2 $3\r\n/dop /mode ## -ooo $1 $2 $3\r\n/voice /mode ## +vvv $1 $2 $3\r\n/dvoice /mode ## -vvv $1 $2 $3\r\n/j /join #$1 $2-\r\n/p /part ## $1-\r\n/w /whois $1\r\n/k /kick ## $1 $2-\r\n/q /query $1\r\n/logon /log on ##\r\n/logoff /log off ##\r\n/save /log buffer $1\r\n/slap /me slaps $1 around a bit with a large trout");

	m_myHost[0] = 0;

	colors[0] = RGB(255, 255, 255);
	colors[1] = RGB(0, 0, 0);
	colors[2] = RGB(0, 0, 127);
	colors[3] = RGB(0, 147, 0);
	colors[4] = RGB(255, 0, 0);
	colors[5] = RGB(127, 0, 0);
	colors[6] = RGB(156, 0, 156);
	colors[7] = RGB(252, 127, 0);
	colors[8] = RGB(255, 255, 0);
	colors[9] = RGB(0, 252, 0);
	colors[10] = RGB(0, 147, 147);
	colors[11] = RGB(0, 255, 255);
	colors[12] = RGB(0, 0, 252);
	colors[13] = RGB(255, 0, 255);
	colors[14] = RGB(127, 127, 127);
	colors[15] = RGB(210, 210, 210);
}
