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

#include "irc.h"
#include <win2k.h>

static const CIrcProto* pZero = NULL;

void CIrcProto::ReadSettings(TDbSetting* sets, int count)
{
	BYTE* base = (BYTE*)this;

	DBVARIANT dbv;
	for (int i = 0; i < count; i++) {
		TDbSetting* p = &sets[i];
		BYTE* ptr = base + p->offset;
		switch (p->type) {
		case DBVT_BYTE:
			*(BYTE*)ptr = getByte(p->name, p->defValue);
			break;
		case DBVT_WORD:
			*(WORD*)ptr = getWord(p->name, p->defValue);
			break;
		case DBVT_DWORD:
			*(DWORD*)ptr = getDword(p->name, p->defValue);
			break;
		case DBVT_ASCIIZ:
			if (!getString(p->name, &dbv)) {
				if (p->size != -1) {
					size_t len = min(p->size - 1, strlen(dbv.pszVal));
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
					*(char**)ptr = NULL;
			}
			break;
		case DBVT_TCHAR:
			if (!getTString(p->name, &dbv)) {
				if (p->size != -1) {
					size_t len = min(p->size - 1, _tcslen(dbv.ptszVal));
					memcpy(ptr, dbv.pszVal, len*sizeof(TCHAR));
					*(TCHAR*)&ptr[len*sizeof(TCHAR)] = 0;
				}
				else *(TCHAR**)ptr = mir_tstrdup(dbv.ptszVal);
				db_free(&dbv);
			}
			else {
				if (p->size != -1) {
					if (p->defStr == NULL)
						*ptr = 0;
					else
						mir_tstrncpy((TCHAR*)ptr, p->defStr, (int)p->size);
				}
				else *(TCHAR**)ptr = mir_tstrdup(p->defStr);
			}
			break;
}	}	}

void CIrcProto::WriteSettings( TDbSetting* sets, int count )
{
	BYTE* base = (BYTE*)this;

	for (int i = 0; i < count; i++) {
		TDbSetting* p = &sets[i];
		BYTE* ptr = base + p->offset;
		switch (p->type) {
		case DBVT_BYTE:   setByte(p->name, *(BYTE*)ptr);       break;
		case DBVT_WORD:   setWord(p->name, *(WORD*)ptr);       break;
		case DBVT_DWORD:  setDword(p->name, *(DWORD*)ptr);     break;

		case DBVT_ASCIIZ:
			if (p->size == -1)
				setString(p->name, *(char**)ptr);
			else
				setString(p->name, (char*)ptr);
			break;

		case DBVT_TCHAR:
			if (p->size == -1)
				setTString(p->name, *(TCHAR**)ptr);
			else
				setTString(p->name, (TCHAR*)ptr);
			break;
}	}	}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttServerEnum( const char* szSetting, LPARAM )
{
	DBVARIANT dbv;
	if (db_get_s(NULL, SERVERSMODULE, szSetting, &dbv))
		return 0;

	SERVER_INFO* pData = new SERVER_INFO;
	pData->m_name = mir_strdup(szSetting);

	char* p1 = strchr(dbv.pszVal, ':') + 1;
	pData->m_iSSL = 0;
	if (!_strnicmp(p1, "SSL", 3)) {
		p1 += 3;
		if (*p1 == '1')
			pData->m_iSSL = 1;
		else if (*p1 == '2')
			pData->m_iSSL = 2;
		p1++;
	}
	char* p2 = strchr(p1, ':');
	pData->m_address = (char*)mir_alloc(p2 - p1 + 1);
	mir_strncpy(pData->m_address, p1, p2 - p1 + 1);

	p1 = p2 + 1;
	while (*p2 != 'G' && *p2 != '-')
		p2++;

	char* buf = (char*)alloca(p2 - p1 + 1);
	mir_strncpy(buf, p1, p2 - p1 + 1);
	pData->m_portStart = atoi(buf);

	if (*p2 == 'G')
		pData->m_portEnd = pData->m_portStart;
	else {
		p1 = p2 + 1;
		p2 = strchr(p1, 'G');
		buf = (char*)alloca(p2 - p1 + 1);
		mir_strncpy(buf, p1, p2 - p1 + 1);
		pData->m_portEnd = atoi(buf);
	}

	p1 = strchr(p2, ':') + 1;
	p2 = strchr(p1, '\0');
	pData->m_group = (char*)mir_alloc(p2 - p1 + 1);
	mir_strncpy(pData->m_group, p1, p2 - p1 + 1);

	g_servers.insert(pData);
	db_free(&dbv);
	return 0;
}

void RereadServers()
{
	g_servers.destroy();

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = sttServerEnum;
	dbces.szModule = SERVERSMODULE;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, NULL, (LPARAM)&dbces);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void removeSpaces(TCHAR* p)
{
	while (*p) {
		if (*p == ' ')
			memmove(p, p + 1, sizeof(TCHAR)*mir_tstrlen(p));
		p++;
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
	{ LPGEN("Quick connect"),     "quick",   IDI_QUICK,       0 },
	{ LPGEN("Server window"),     "server",  IDI_SERVER,      0 },
	{ LPGEN("Show channel"),      "show",    IDI_SHOW,        0 },
	{ LPGEN("Question"),          "question",IDI_IRCQUESTION, 0 },
	{ LPGEN("WhoIs"),             "whois",   IDI_WHOIS,       0 },
	{ LPGEN("Incoming DCC Chat"), "dcc",     IDI_DCC,         0 },
	{ LPGEN("Logo (48x48)"),      "logo",    IDI_LOGO,       48 }
};

void InitIcons(void)
{
	Icon_Register(hInst, "Protocols/IRC", iconList, SIZEOF(iconList), "IRC");
}

HICON LoadIconEx(int iconId, bool big)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return Skin_GetIconByHandle(iconList[i].hIcolib, big);

	return NULL;
}

HANDLE GetIconHandle(int iconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}

void ReleaseIconEx(HICON hIcon)
{
	if (hIcon)
		Skin_ReleaseIcon(hIcon);
}

void WindowSetIcon(HWND hWnd, int iconId)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(iconId, true));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx(iconId));
}

void WindowFreeIcon(HWND hWnd)
{
	ReleaseIconEx((HICON)SendMessage(hWnd, WM_SETICON, ICON_BIG, 0));
	ReleaseIconEx((HICON)SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0));
}

/////////////////////////////////////////////////////////////////////////////////////////
// code page handler

struct { UINT cpId; TCHAR *cpName; } static cpTable[] =
{
	{	  874, LPGENT("Thai") },
	{	  932, LPGENT("Japanese") },
	{	  936, LPGENT("Simplified Chinese") },
	{	  949, LPGENT("Korean") },
	{	  950, LPGENT("Traditional Chinese") },
	{	 1250, LPGENT("Central European") },
	{	 1251, LPGENT("Cyrillic (Windows)") },
	{	20866, LPGENT("Cyrillic (KOI8R)") },
	{	 1252, LPGENT("Latin I") },
	{	 1253, LPGENT("Greek") },
	{	 1254, LPGENT("Turkish") },
	{	 1255, LPGENT("Hebrew") },
	{	 1256, LPGENT("Arabic") },
	{	 1257, LPGENT("Baltic") },
	{	 1258, LPGENT("Vietnamese") },
	{	 1361, LPGENT("Korean (Johab)") }
};

static CCtrlCombo* sttCombo;

static BOOL CALLBACK sttLangAddCallback(TCHAR *str)
{
	UINT cp = _ttoi(str);
	CPINFOEX cpinfo;
	if (GetCPInfoEx(cp, 0, &cpinfo)) {
		TCHAR* b = _tcschr(cpinfo.CodePageName, '(');
		if (b) {
			TCHAR* e = _tcsrchr(cpinfo.CodePageName, ')');
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
// 'Add server' dialog

static int sttRequiredFields[] = { IDC_ADD_SERVER, IDC_ADD_ADDRESS, IDC_ADD_PORT, IDC_ADD_PORT2, IDC_ADD_COMBO };

struct CServerDlg : public CProtoDlgBase<CIrcProto>
{
	CConnectPrefsDlg* m_owner;
	int m_action;

	CCtrlButton m_OK;
	CCtrlEdit m_server, m_address, m_port, m_port2;
	CCtrlCombo m_groupCombo;

	CServerDlg(CIrcProto* _pro, CConnectPrefsDlg* _owner, int _action)
		: CProtoDlgBase<CIrcProto>(_pro, IDD_ADDSERVER, _owner->GetHwnd(), false),
		m_owner(_owner),
		m_action(_action),
		m_OK(this, IDOK),
		m_groupCombo(this, IDC_ADD_COMBO),
		m_address(this, IDC_ADD_ADDRESS),
		m_server(this, IDC_ADD_SERVER),
		m_port(this, IDC_ADD_PORT),
		m_port2(this, IDC_ADD_PORT2)
	{
		m_OK.OnClick = Callback(this, &CServerDlg::OnOk);
		m_autoClose = CLOSE_ON_CANCEL;
	}

	virtual void OnInitDialog()
	{
		int i = m_owner->m_serverCombo.GetCount();
		for (int index = 0; index < i; index++) {
			SERVER_INFO* pData = (SERVER_INFO*)m_owner->m_serverCombo.GetItemData(index);
			if (m_groupCombo.FindStringA(pData->m_group, -1, true) == CB_ERR)
				m_groupCombo.AddStringA(pData->m_group);
		}

		if (m_action == 2) {
			int j = m_owner->m_serverCombo.GetCurSel();
			SERVER_INFO* pData = (SERVER_INFO*)m_owner->m_serverCombo.GetItemData(j);
			m_address.SetTextA(pData->m_address);
			m_groupCombo.SetTextA(pData->m_group);
			m_port.SetInt(pData->m_portStart);
			m_port2.SetInt(pData->m_portEnd);

			char *p = strstr(pData->m_name, ": ");
			if (p)
				m_server.SetTextA(p + 2);

			if (pData->m_iSSL == 0)
				CheckDlgButton(m_hwnd, IDC_OFF, BST_CHECKED);
			if (pData->m_iSSL == 1)
				CheckDlgButton(m_hwnd, IDC_AUTO, BST_CHECKED);
			if (pData->m_iSSL == 2)
				CheckDlgButton(m_hwnd, IDC_ON, BST_CHECKED);
		}
		else {
			CheckDlgButton(m_hwnd, IDC_OFF, BST_CHECKED);
			m_port.SetInt(6667);
			m_port2.SetInt(6667);
		}

		int bEnableSsl = TRUE;
		EnableWindow(GetDlgItem(m_hwnd, IDC_ON), bEnableSsl);
		EnableWindow(GetDlgItem(m_hwnd, IDC_OFF), bEnableSsl);
		EnableWindow(GetDlgItem(m_hwnd, IDC_AUTO), bEnableSsl);

		SetFocus(m_groupCombo.GetHwnd());
	}

	virtual void OnClose()
	{
		m_owner->m_serverCombo.Enable();
		m_owner->m_add.Enable();
		m_owner->m_edit.Enable();
		m_owner->m_del.Enable();
	}

	void OnOk(CCtrlButton*)
	{
		for (int k = 0; k < SIZEOF(sttRequiredFields); k++)
			if (!GetWindowTextLength(GetDlgItem(m_hwnd, sttRequiredFields[k]))) {
				MessageBox(m_hwnd, TranslateT("Please complete all fields"), TranslateT("IRC error"), MB_OK | MB_ICONERROR);
				return;
			}

		if (m_action == 2) {
			int i = m_owner->m_serverCombo.GetCurSel();
			m_owner->m_serverCombo.DeleteString(i);
		}

		SERVER_INFO *pData = new SERVER_INFO;
		pData->m_iSSL = 0;
		if (IsDlgButtonChecked(m_hwnd, IDC_ON))
			pData->m_iSSL = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_AUTO))
			pData->m_iSSL = 1;

		pData->m_portStart = m_port.GetInt();
		pData->m_portEnd = m_port2.GetInt();
		pData->m_address = rtrim(m_address.GetTextA());
		pData->m_group = m_groupCombo.GetTextA();
		pData->m_name = m_server.GetTextA();

		char temp[255];
		mir_snprintf(temp, SIZEOF(temp), "%s: %s", pData->m_group, pData->m_name);
		mir_free(pData->m_name);
		pData->m_name = mir_strdup(temp);

		int iItem = m_owner->m_serverCombo.AddStringA(pData->m_name, (LPARAM)pData);
		m_owner->m_serverCombo.SetCurSel(iItem);
		m_owner->OnServerCombo(NULL);

		m_owner->m_serverlistModified = true;
		Close();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// 'Connect preferences' dialog

static TDbSetting ConnectSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_userID), "UserID", DBVT_TCHAR, SIZEOF(pZero->m_userID) },
	{ FIELD_OFFSET(CIrcProto, m_identSystem), "IdentSystem", DBVT_TCHAR, SIZEOF(pZero->m_identSystem) },
	{ FIELD_OFFSET(CIrcProto, m_identPort), "IdentPort", DBVT_TCHAR, SIZEOF(pZero->m_identPort) },

	{ FIELD_OFFSET(CIrcProto, m_serverName ), "ServerName", DBVT_ASCIIZ, SIZEOF(pZero->m_serverName) },
	{ FIELD_OFFSET(CIrcProto, m_portStart ), "PortStart", DBVT_ASCIIZ, SIZEOF(pZero->m_portStart) },
	{ FIELD_OFFSET(CIrcProto, m_portEnd ), "PortEnd", DBVT_ASCIIZ, SIZEOF(pZero->m_portEnd ) },
	{ FIELD_OFFSET(CIrcProto, m_password ), "Password", DBVT_ASCIIZ, SIZEOF(pZero->m_password ) },
	{ FIELD_OFFSET(CIrcProto, m_joinOnInvite ), "JoinOnInvite", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_network ), "Network", DBVT_ASCIIZ, SIZEOF(pZero->m_network ) },
	{ FIELD_OFFSET(CIrcProto, m_iSSL ), "UseSSL", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_onlineNotificationTime) , "OnlineNotificationTime", DBVT_WORD, 0, 30 },
	{ FIELD_OFFSET(CIrcProto, m_onlineNotificationLimit) , "OnlineNotificationLimit", DBVT_WORD, 0, 50 },
	{ FIELD_OFFSET(CIrcProto, m_channelAwayNotification), "ChannelAwayNotification", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_nick), "Nick", DBVT_TCHAR, SIZEOF(pZero->m_nick) },
	{ FIELD_OFFSET(CIrcProto, m_pNick), "PNick", DBVT_TCHAR, SIZEOF(pZero->m_pNick) },
	{ FIELD_OFFSET(CIrcProto, m_alternativeNick), "AlernativeNick", DBVT_TCHAR, SIZEOF(pZero->m_alternativeNick) },
	{ FIELD_OFFSET(CIrcProto, m_name), "Name", DBVT_TCHAR, SIZEOF(pZero->m_name) },
	{ FIELD_OFFSET(CIrcProto, m_disableDefaultServer), "DisableDefaultServer", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_ident), "Ident", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_identTimer), "IdentTimer", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_forceVisible), "ForceVisible", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_disableErrorPopups), "DisableErrorPopups", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_rejoinChannels), "RejoinChannels", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_rejoinIfKicked), "RejoinIfKicked", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_showAddresses), "ShowAddresses", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_oldStyleModes), "OldStyleModes", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_useServer), "UseServer", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_hideServerWindow), "HideServerWindow", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_serverComboSelection), "ServerComboSelection", DBVT_DWORD, 0 },
	{ FIELD_OFFSET(CIrcProto, m_sendKeepAlive), "SendKeepAlive", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_autoOnlineNotification), "AutoOnlineNotification", DBVT_BYTE },
};

CConnectPrefsDlg::CConnectPrefsDlg(CIrcProto* _pro)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_PREFS_CONNECT, NULL, false),
	m_serverCombo(this, IDC_SERVERCOMBO),
	m_server(this, IDC_SERVER),
	m_port(this, IDC_PORT),
	m_port2(this, IDC_PORT2),
	m_pass(this, IDC_PASS),
	m_add(this, IDC_ADDSERVER, LoadIconEx(IDI_ADD), LPGEN("Add a new network")),
	m_edit(this, IDC_EDITSERVER, LoadIconEx(IDI_EDIT), LPGEN("Edit this network")),
	m_del(this, IDC_DELETESERVER, LoadIconEx(IDI_DELETE), LPGEN("Delete this network")),
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
	m_onlineNotif(this, IDC_ONLINENOTIF),
	m_channelAway(this, IDC_CHANNELAWAY),
	m_enableServer(this, IDC_STARTUP),
	m_onlineTimer(this, IDC_ONLINETIMER),
	m_limit(this, IDC_LIMIT),
	m_spin1(this, IDC_SPIN1),
	m_spin2(this, IDC_SPIN2),
	m_ssl(this, IDC_SSL),
	m_serverlistModified(false)
{
	m_serverCombo.OnChange = Callback(this, &CConnectPrefsDlg::OnServerCombo);
	m_add.OnClick = Callback(this, &CConnectPrefsDlg::OnAddServer);
	m_del.OnClick = Callback(this, &CConnectPrefsDlg::OnDeleteServer);
	m_edit.OnClick = Callback(this, &CConnectPrefsDlg::OnEditServer);
	m_enableServer.OnChange = Callback(this, &CConnectPrefsDlg::OnStartup);
	m_ident.OnChange = Callback(this, &CConnectPrefsDlg::OnIdent);
	m_useServer.OnChange = Callback(this, &CConnectPrefsDlg::OnUseServer);
	m_onlineNotif.OnChange = Callback(this, &CConnectPrefsDlg::OnOnlineNotif);
	m_channelAway.OnChange = Callback(this, &CConnectPrefsDlg::OnChannelAway);
}

void CConnectPrefsDlg::OnInitDialog()
{
	m_proto->m_hwndConnect = m_hwnd;

	//	Fill the servers combo box and create SERVER_INFO structures
	for (int i = 0; i < g_servers.getCount(); i++) {
		SERVER_INFO &si = g_servers[i];
		m_serverCombo.AddStringA(si.m_name, LPARAM(&si));
	}

	m_serverCombo.SetCurSel(m_proto->m_serverComboSelection);
	m_server.SetTextA(m_proto->m_serverName);
	m_port.SetTextA(m_proto->m_portStart);
	m_port2.SetTextA(m_proto->m_portEnd);

	if (m_proto->m_iSSL == 0)
		m_ssl.SetText(TranslateT("Off"));
	if (m_proto->m_iSSL == 1)
		m_ssl.SetText(TranslateT("Auto"));
	if (m_proto->m_iSSL == 2)
		m_ssl.SetText(TranslateT("On"));

	if (m_proto->m_serverComboSelection != -1) {
		SERVER_INFO *pData = (SERVER_INFO*)m_serverCombo.GetItemData(m_proto->m_serverComboSelection);
		if ((INT_PTR)pData != CB_ERR) {
			m_server.SetTextA(pData->m_address);
			m_port.SetInt(pData->m_portStart);
			m_port2.SetInt(pData->m_portEnd);
		}
	}

	m_spin1.SendMsg(UDM_SETRANGE, 0, MAKELONG(999, 20));
	m_spin1.SendMsg(UDM_SETPOS, 0, MAKELONG(m_proto->m_onlineNotificationTime, 0));
	m_spin2.SendMsg(UDM_SETRANGE, 0, MAKELONG(200, 0));
	m_spin2.SendMsg(UDM_SETPOS, 0, MAKELONG(m_proto->m_onlineNotificationLimit, 0));
	m_nick.SetText(m_proto->m_nick);
	m_nick2.SetText(m_proto->m_alternativeNick);
	m_userID.SetText(m_proto->m_userID);
	m_name.SetText(m_proto->m_name);
	m_pass.SetTextA(m_proto->m_password);
	m_identSystem.SetText(m_proto->m_identSystem);
	m_identPort.SetText(m_proto->m_identPort);
	m_address.SetState(m_proto->m_showAddresses);
	m_oldStyle.SetState(m_proto->m_oldStyleModes);
	m_channelAway.SetState(m_proto->m_channelAwayNotification);
	m_onlineNotif.SetState(m_proto->m_autoOnlineNotification);
	m_onlineTimer.Enable(m_proto->m_autoOnlineNotification);
	m_channelAway.Enable(m_proto->m_autoOnlineNotification);
	m_spin1.Enable(m_proto->m_autoOnlineNotification);
	m_spin2.Enable(m_proto->m_autoOnlineNotification && m_proto->m_channelAwayNotification);
	m_limit.Enable(m_proto->m_autoOnlineNotification && m_proto->m_channelAwayNotification);
	m_ident.SetState(m_proto->m_ident);
	m_identSystem.Enable(m_proto->m_ident);
	m_identPort.Enable(m_proto->m_ident);
	m_identTimer.Enable(m_proto->m_ident);
	m_identTimer.SetState(m_proto->m_identTimer);
	m_disableError.SetState(m_proto->m_disableErrorPopups);
	m_forceVisible.SetState(m_proto->m_forceVisible);
	m_rejoinChannels.SetState(m_proto->m_rejoinChannels);
	m_rejoinOnKick.SetState(m_proto->m_rejoinIfKicked);
	m_enableServer.SetState(!m_proto->m_disableDefaultServer);
	m_keepAlive.SetState(m_proto->m_sendKeepAlive);
	m_useServer.SetState(m_proto->m_useServer);
	m_showServer.SetState(!m_proto->m_hideServerWindow);
	m_showServer.Enable(m_proto->m_useServer);
	m_autoJoin.SetState(m_proto->m_joinOnInvite);

	m_serverCombo.Enable(!m_proto->m_disableDefaultServer);
	m_add.Enable(!m_proto->m_disableDefaultServer);
	m_edit.Enable(!m_proto->m_disableDefaultServer);
	m_del.Enable(!m_proto->m_disableDefaultServer);
	m_server.Enable(!m_proto->m_disableDefaultServer);
	m_port.Enable(!m_proto->m_disableDefaultServer);
	m_port2.Enable(!m_proto->m_disableDefaultServer);
	m_pass.Enable(!m_proto->m_disableDefaultServer);
}

void CConnectPrefsDlg::OnServerCombo(CCtrlData*)
{
	int i = m_serverCombo.GetCurSel();
	SERVER_INFO* pData = (SERVER_INFO*)m_serverCombo.GetItemData(i);
	if (pData && (INT_PTR)pData != CB_ERR) {
		m_server.SetTextA(pData->m_address);
		m_port.SetInt(pData->m_portStart);
		m_port2.SetInt(pData->m_portEnd);
		m_pass.SetTextA("");

		if (pData->m_iSSL == 0)
			m_ssl.SetText(TranslateT("Off"));
		if (pData->m_iSSL == 1)
			m_ssl.SetText(TranslateT("Auto"));
		if (pData->m_iSSL == 2)
			m_ssl.SetText(TranslateT("On"));

		SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
	}
}

void CConnectPrefsDlg::OnAddServer(CCtrlButton*)
{
	m_serverCombo.Disable();
	m_add.Disable();
	m_edit.Disable();
	m_del.Disable();
	CServerDlg* dlg = new CServerDlg(m_proto, this, 1);
	dlg->Show();
}

void CConnectPrefsDlg::OnDeleteServer(CCtrlButton*)
{
	int i = m_serverCombo.GetCurSel();
	if (i == CB_ERR)
		return;

	m_serverCombo.Disable();
	m_add.Disable();
	m_edit.Disable();
	m_del.Disable();

	SERVER_INFO *pData = (SERVER_INFO*)m_serverCombo.GetItemData(i);
	TCHAR temp[200];
	mir_sntprintf(temp, SIZEOF(temp), TranslateT("Do you want to delete\r\n%s"), (TCHAR*)_A2T(pData->m_name));
	if (MessageBox(m_hwnd, temp, TranslateT("Delete server"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		g_servers.remove(pData);

		m_serverCombo.DeleteString(i);
		if (i >= m_serverCombo.GetCount())
			i--;
		m_serverCombo.SetCurSel(i);
		OnServerCombo(NULL);
		SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
		m_serverlistModified = true;
	}

	m_serverCombo.Enable();
	m_add.Enable();
	m_edit.Enable();
	m_del.Enable();
}

void CConnectPrefsDlg::OnEditServer(CCtrlButton*)
{
	int i = m_serverCombo.GetCurSel();
	if (i == CB_ERR)
		return;

	m_serverCombo.Disable();
	m_add.Disable();
	m_edit.Disable();
	m_del.Disable();
	CServerDlg *dlg = new CServerDlg(m_proto, this, 2);
	dlg->Show();
	SetWindowText(dlg->GetHwnd(), TranslateT("Edit server"));
}

void CConnectPrefsDlg::OnStartup(CCtrlData*)
{
	m_serverCombo.Enable(m_enableServer.GetState());
	m_add.Enable(m_enableServer.GetState());
	m_edit.Enable(m_enableServer.GetState());
	m_del.Enable(m_enableServer.GetState());
	m_server.Enable(m_enableServer.GetState());
	m_port.Enable(m_enableServer.GetState());
	m_port2.Enable(m_enableServer.GetState());
	m_pass.Enable(m_enableServer.GetState());
	m_ssl.Enable(m_enableServer.GetState());
}

void CConnectPrefsDlg::OnIdent(CCtrlData*)
{
	m_identSystem.Enable(m_ident.GetState());
	m_identPort.Enable(m_ident.GetState());
	m_identTimer.Enable(m_ident.GetState());
}

void CConnectPrefsDlg::OnUseServer(CCtrlData*)
{
	EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSERVER), m_useServer.GetState());
}

void CConnectPrefsDlg::OnOnlineNotif(CCtrlData*)
{
	m_channelAway.Enable(m_onlineNotif.GetState());
	m_onlineTimer.Enable(m_onlineNotif.GetState());
	m_spin1.Enable(m_onlineNotif.GetState());
	m_spin2.Enable(m_onlineNotif.GetState());
	m_limit.Enable(m_onlineNotif.GetState() && m_channelAway.GetState());
}

void CConnectPrefsDlg::OnChannelAway(CCtrlData*)
{
	m_spin2.Enable(m_onlineNotif.GetState() && m_channelAway.GetState());
	m_limit.Enable(m_onlineNotif.GetState() && m_channelAway.GetState());
}

void CConnectPrefsDlg::OnApply()
{
	//Save the setting in the CONNECT dialog
	if (m_enableServer.GetState()) {
		m_server.GetTextA(m_proto->m_serverName, SIZEOF(m_proto->m_serverName));
		m_port.GetTextA(m_proto->m_portStart, SIZEOF(m_proto->m_portStart));
		m_port2.GetTextA(m_proto->m_portEnd, SIZEOF(m_proto->m_portEnd));
		m_pass.GetTextA(m_proto->m_password, SIZEOF(m_proto->m_password));
	}
	else m_proto->m_serverName[0] = m_proto->m_portStart[0] = m_proto->m_portEnd[0] = m_proto->m_password[0] = 0;

	m_proto->m_onlineNotificationTime = SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_GETPOS, 0, 0);
	m_proto->m_onlineNotificationLimit = SendDlgItemMessage(m_hwnd, IDC_SPIN2, UDM_GETPOS, 0, 0);
	m_proto->m_channelAwayNotification = m_channelAway.GetState();

	m_nick.GetText(m_proto->m_nick, SIZEOF(m_proto->m_nick));
	removeSpaces(m_proto->m_nick);
	_tcsncpy_s(m_proto->m_pNick, m_proto->m_nick, _TRUNCATE);
	m_nick2.GetText(m_proto->m_alternativeNick, SIZEOF(m_proto->m_alternativeNick));
	removeSpaces(m_proto->m_alternativeNick);
	m_userID.GetText(m_proto->m_userID, SIZEOF(m_proto->m_userID));
	removeSpaces(m_proto->m_userID);
	m_name.GetText(m_proto->m_name, SIZEOF(m_proto->m_name));
	m_identSystem.GetText(m_proto->m_identSystem, SIZEOF(m_proto->m_identSystem));
	m_identPort.GetText(m_proto->m_identPort, SIZEOF(m_proto->m_identPort));
	m_proto->m_disableDefaultServer = !m_enableServer.GetState();
	m_proto->m_ident = m_ident.GetState();
	m_proto->m_identTimer = m_identTimer.GetState();
	m_proto->m_forceVisible = m_forceVisible.GetState();
	m_proto->m_disableErrorPopups = m_disableError.GetState();
	m_proto->m_rejoinChannels = m_rejoinChannels.GetState();
	m_proto->m_rejoinIfKicked = m_rejoinOnKick.GetState();
	m_proto->m_showAddresses = m_address.GetState();
	m_proto->m_oldStyleModes = m_oldStyle.GetState();
	m_proto->m_useServer = m_useServer.GetState();

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS;
	if (!m_proto->m_useServer)
		mi.flags |= CMIF_GRAYED;
	Menu_ModifyItem(m_proto->hMenuServer, &mi);

	m_proto->m_joinOnInvite = m_autoJoin.GetState();
	m_proto->m_hideServerWindow = !m_showServer.GetState();
	m_proto->m_serverComboSelection = m_serverCombo.GetCurSel();
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

	int i = m_serverCombo.GetCurSel();
	SERVER_INFO *pData = (SERVER_INFO*)m_serverCombo.GetItemData(i);
	if (pData && (INT_PTR)pData != CB_ERR) {
		if (m_enableServer.GetState())
			mir_strcpy(m_proto->m_network, pData->m_group);
		else
			mir_strcpy(m_proto->m_network, "");
		m_proto->m_iSSL = pData->m_iSSL;
	}

	if (m_serverlistModified) {
		m_serverlistModified = false;
		CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)SERVERSMODULE);

		int j = m_serverCombo.GetCount();
		if (j != CB_ERR && j != 0) {
			for (int index2 = 0; index2 < j; index2++) {
				SERVER_INFO* pData = (SERVER_INFO*)m_serverCombo.GetItemData(index2);
				if (pData == NULL || (INT_PTR)pData == CB_ERR)
					continue;

				char TextLine[512];
				if (pData->m_iSSL > 0)
					mir_snprintf(TextLine, SIZEOF(TextLine), "SERVER:SSL%u%s:%d-%dGROUP:%s", pData->m_iSSL, pData->m_address, pData->m_portStart, pData->m_portEnd, pData->m_group);
				else
					mir_snprintf(TextLine, SIZEOF(TextLine), "SERVER:%s:%d-%dGROUP:%s", pData->m_address, pData->m_portStart, pData->m_portEnd, pData->m_group);
				db_set_s(NULL, SERVERSMODULE, pData->m_name, TextLine);

				// combobox might contain new items
				if (g_servers.find(pData) == NULL)
					g_servers.insert(pData);
			}
		}
	}

	m_proto->WriteSettings(ConnectSettings, SIZEOF(ConnectSettings));
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'CTCP preferences' dialog

static TDbSetting CtcpSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_userInfo), "UserInfo", DBVT_TCHAR, SIZEOF(pZero->m_userInfo) },
	{ FIELD_OFFSET(CIrcProto, m_DCCPacketSize), "DccPacketSize", DBVT_WORD, 0, 4096 },
	{ FIELD_OFFSET(CIrcProto, m_DCCPassive), "DccPassive", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_DCCMode), "DCCMode", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_manualHost), "ManualHost", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_IPFromServer), "IPFromServer", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_disconnectDCCChats), "DisconnectDCCChats", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_mySpecifiedHost), "SpecHost", DBVT_ASCIIZ, SIZEOF(pZero->m_mySpecifiedHost) },
	{ FIELD_OFFSET(CIrcProto, m_DCCChatAccept), "CtcpChatAccept", DBVT_BYTE, 0, 1 },
	{ FIELD_OFFSET(CIrcProto, m_sendNotice), "SendNotice", DBVT_BYTE, 0, 1 }
};

CCtcpPrefsDlg::CCtcpPrefsDlg(CIrcProto* _pro)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_PREFS_CTCP, NULL, false),
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

void CCtcpPrefsDlg::OnInitDialog()
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

	TCHAR szTemp[10];
	mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%u"), m_proto->m_DCCPacketSize);
	int i = m_combo.SelectString(szTemp);
	if (i == CB_ERR)
		m_combo.SelectString(_T("4096"));

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
				CMString s = (CMString)TranslateT("<Resolved IP: ") + (TCHAR*)_A2T(m_proto->m_myHost) + _T(">");
				m_ip.SetText(s.c_str());
			}
			else m_ip.SetText(TranslateT("<Automatic>"));
		}
		else {
			if (m_proto->m_myLocalHost[0]) {
				CMString s = (CMString)TranslateT("<Local IP: ") + (TCHAR*)_A2T(m_proto->m_myLocalHost) + _T(">");
				m_ip.SetText(s.c_str());
			}
			else m_ip.SetText(TranslateT("<Automatic>"));
		}
	}
}

void CCtcpPrefsDlg::OnClicked(CCtrlData*)
{
	m_ip.Enable(m_enableIP.GetState());
	m_fromServer.Enable(!m_enableIP.GetState());

	if (m_enableIP.GetState())
		m_ip.SetTextA(m_proto->m_mySpecifiedHost);
	else {
		if (m_fromServer.GetState()) {
			if (m_proto->m_myHost[0]) {
				CMString s = (CMString)TranslateT("<Resolved IP: ") + (TCHAR*)_A2T(m_proto->m_myHost) + _T(">");
				m_ip.SetText(s.c_str());
			}
			else m_ip.SetText(TranslateT("<Automatic>"));
		}
		else {
			if (m_proto->m_myLocalHost[0]) {
				CMString s = (CMString)TranslateT("<Local IP: ") + (TCHAR*)_A2T(m_proto->m_myLocalHost) + _T(">");
				m_ip.SetText(s.c_str());
			}
			else m_ip.SetText(TranslateT("<Automatic>"));
		}
	}
}

void CCtcpPrefsDlg::OnApply()
{
	m_userInfo.GetText(m_proto->m_userInfo, SIZEOF(m_proto->m_userInfo));

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

	m_proto->WriteSettings(CtcpSettings, SIZEOF(CtcpSettings));
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Advanced preferences' dialog

static TDbSetting OtherSettings[] =
{
	{ FIELD_OFFSET(CIrcProto, m_quitMessage), "QuitMessage", DBVT_TCHAR, SIZEOF(pZero->m_quitMessage) },
	{ FIELD_OFFSET(CIrcProto, m_alias), "Alias", DBVT_TCHAR, -1 },
	{ FIELD_OFFSET(CIrcProto, m_codepage), "Codepage", DBVT_DWORD, 0, CP_ACP },
	{ FIELD_OFFSET(CIrcProto, m_utfAutodetect), "UtfAutodetect", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_perform), "Perform", DBVT_BYTE },
	{ FIELD_OFFSET(CIrcProto, m_scriptingEnabled), "ScriptingEnabled", DBVT_BYTE }
};

static char* sttPerformEvents[] = {
	LPGEN("Event: Available"),
	LPGEN("Event: Away"),
	LPGEN("Event: N/A"),
	LPGEN("Event: Occupied"),
	LPGEN("Event: DND"),
	LPGEN("Event: Free for chat"),
	LPGEN("Event: On the phone"),
	LPGEN("Event: Out for lunch"),
	LPGEN("Event: Disconnect"),
	LPGEN("ALL NETWORKS")
};

static LRESULT CALLBACK EditSubclassProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 21 || wParam == 11 || wParam == 2) {
			char w[2];
			w[1] = '\0';
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

COtherPrefsDlg::COtherPrefsDlg(CIrcProto* _pro)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_PREFS_OTHER, NULL, false),
	m_url(this, IDC_CUSTOM),
	m_performCombo(this, IDC_PERFORMCOMBO),
	m_codepage(this, IDC_CODEPAGE),
	m_pertormEdit(this, IDC_PERFORMEDIT),
	m_perform(this, IDC_PERFORM),
	m_scripting(this, IDC_SCRIPT),
	m_autodetect(this, IDC_UTF_AUTODETECT),
	m_quitMessage(this, IDC_QUITMESSAGE),
	m_alias(this, IDC_ALIASEDIT),
	m_add(this, IDC_ADD, LoadIconEx(IDI_ADD), LPGEN("Click to set commands that will be performed for this event")),
	m_delete(this, IDC_DELETE, LoadIconEx(IDI_DELETE), LPGEN("Click to delete the commands for this event")),
	m_performlistModified(false)
{
	m_url.OnClick = Callback(this, &COtherPrefsDlg::OnUrl);
	m_performCombo.OnChange = Callback(this, &COtherPrefsDlg::OnPerformCombo);
	m_codepage.OnChange = Callback(this, &COtherPrefsDlg::OnCodePage);
	m_pertormEdit.OnChange = Callback(this, &COtherPrefsDlg::OnPerformEdit);
	m_perform.OnChange = Callback(this, &COtherPrefsDlg::OnPerform);
	m_add.OnClick = Callback(this, &COtherPrefsDlg::OnAdd);
	m_delete.OnClick = Callback(this, &COtherPrefsDlg::OnDelete);
}

void COtherPrefsDlg::OnInitDialog()
{
	mir_subclassWindow(m_alias.GetHwnd(), EditSubclassProc);
	mir_subclassWindow(m_quitMessage.GetHwnd(), EditSubclassProc);
	mir_subclassWindow(m_pertormEdit.GetHwnd(), EditSubclassProc);

	m_alias.SetText(m_proto->m_alias);
	m_quitMessage.SetText(m_proto->m_quitMessage);
	m_perform.SetState(m_proto->m_perform);
	m_scripting.SetState(m_proto->m_scriptingEnabled);
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

	for (int i = 0; i < g_servers.getCount(); i++) {
		SERVER_INFO& si = g_servers[i];
		int idx = m_performCombo.FindStringA(si.m_group, -1, true);
		if (idx == CB_ERR) {
			idx = m_performCombo.AddStringA(si.m_group);
			addPerformComboValue(idx, si.m_group);
		}
	}

	for (int i = 0; i < SIZEOF(sttPerformEvents); i++) {
		int idx = m_performCombo.InsertString(_A2T(Translate(sttPerformEvents[i])), i);
		addPerformComboValue(idx, sttPerformEvents[i]);
	}

	m_performCombo.SetCurSel(0);
	OnPerformCombo(NULL);
	m_autodetect.SetState(m_proto->m_utfAutodetect);
}

void COtherPrefsDlg::OnUrl(CCtrlButton*)
{
	CallService(MS_UTILS_OPENURL, 0, (LPARAM)"http://members.chello.se/matrix/index.html");
}

void COtherPrefsDlg::OnPerformCombo(CCtrlData*)
{
	int i = m_performCombo.GetCurSel();
	PERFORM_INFO* pPerf = (PERFORM_INFO*)m_performCombo.GetItemData(i);
	if (pPerf == 0)
		m_pertormEdit.SetTextA("");
	else
		m_pertormEdit.SetText(pPerf->mText.c_str());
	m_add.Disable();
	if (GetWindowTextLength(m_pertormEdit.GetHwnd()) != 0)
		m_delete.Enable();
	else
		m_delete.Disable();
}

void COtherPrefsDlg::OnCodePage(CCtrlData*)
{
	int curSel = m_codepage.GetCurSel();
	m_autodetect.Enable(m_codepage.GetItemData(curSel) != CP_UTF8);
}

void COtherPrefsDlg::OnPerformEdit(CCtrlData*)
{
	m_add.Enable();

	if (GetWindowTextLength(m_pertormEdit.GetHwnd()) != 0)
		m_delete.Enable();
	else
		m_delete.Disable();
}

void COtherPrefsDlg::OnPerform(CCtrlData*)
{
	m_performCombo.Enable(m_perform.GetState());
	m_pertormEdit.Enable(m_perform.GetState());
	m_add.Enable(m_perform.GetState());
	m_delete.Enable(m_perform.GetState());
}

void COtherPrefsDlg::OnAdd(CCtrlButton*)
{
	TCHAR* temp = m_pertormEdit.GetText();

	if (my_strstri(temp, _T("/away")))
		MessageBox(NULL, TranslateT("The usage of /AWAY in your perform buffer is restricted\n as IRC sends this command automatically."), TranslateT("IRC Error"), MB_OK);
	else {
		int i = m_performCombo.GetCurSel();
		if (i != CB_ERR) {
			PERFORM_INFO* pPerf = (PERFORM_INFO*)m_performCombo.GetItemData(i);
			if (pPerf != NULL)
				pPerf->mText = temp;

			m_add.Disable();
			m_performlistModified = true;
		}
	}
	mir_free(temp);
}

void COtherPrefsDlg::OnDelete(CCtrlButton*)
{
	int i = m_performCombo.GetCurSel();
	if (i != CB_ERR) {
		PERFORM_INFO* pPerf = (PERFORM_INFO*)m_performCombo.GetItemData(i);
		if (pPerf != NULL) {
			pPerf->mText = _T("");
			m_pertormEdit.SetTextA("");
			m_delete.Disable();
			m_add.Disable();
		}

		m_performlistModified = true;
	}
}

void COtherPrefsDlg::OnDestroy()
{
	int i = m_performCombo.GetCount();
	if (i != CB_ERR && i != 0) {
		for (int index = 0; index < i; index++) {
			PERFORM_INFO* pPerf = (PERFORM_INFO*)m_performCombo.GetItemData(index);
			if ((INT_PTR)pPerf != CB_ERR && pPerf != NULL)
				delete pPerf;
		}
	}
}

void COtherPrefsDlg::OnApply()
{
	mir_free(m_proto->m_alias);
	m_proto->m_alias = m_alias.GetText();
	m_quitMessage.GetText(m_proto->m_quitMessage, SIZEOF(m_proto->m_quitMessage));

	int curSel = m_codepage.GetCurSel();
	m_proto->m_codepage = m_codepage.GetItemData(curSel);
	if (m_proto->IsConnected())
		m_proto->setCodepage(m_proto->m_codepage);

	m_proto->m_utfAutodetect = m_autodetect.GetState();
	m_proto->m_perform = m_perform.GetState();
	m_proto->m_scriptingEnabled = m_scripting.GetState();
	if (m_add.Enabled())
		OnAdd(NULL);

	if (m_performlistModified) {
		int count = m_performCombo.GetCount();
		for (int i = 0; i < count; i++) {
			PERFORM_INFO* pPerf = (PERFORM_INFO*)m_performCombo.GetItemData(i);
			if ((INT_PTR)pPerf == CB_ERR)
				continue;

			if (!pPerf->mText.IsEmpty())
				m_proto->setTString(pPerf->mSetting.c_str(), pPerf->mText.c_str());
			else
				db_unset(NULL, m_proto->m_szModuleName, pPerf->mSetting.c_str());
		}
	}
	m_proto->WriteSettings(OtherSettings, SIZEOF(OtherSettings));
}

void COtherPrefsDlg::addPerformComboValue(int idx, const char* szValueName)
{
	CMStringA sSetting = CMStringA("PERFORM:") + szValueName;
	sSetting.MakeUpper();

	PERFORM_INFO* pPref;
	DBVARIANT dbv;
	if (!m_proto->getTString(sSetting.c_str(), &dbv)) {
		pPref = new PERFORM_INFO(sSetting.c_str(), dbv.ptszVal);
		db_free(&dbv);
	}
	else pPref = new PERFORM_INFO(sSetting.c_str(), _T(""));
	m_performCombo.SetItemData(idx, (LPARAM)pPref);
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'add ignore' preferences dialog

CAddIgnoreDlg::CAddIgnoreDlg(CIrcProto* _pro, const TCHAR* mask, CIgnorePrefsDlg* _owner)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_ADDIGNORE, _owner->GetHwnd(), false),
	m_Ok(this, IDOK),
	m_owner(_owner)
{
	if (mask == NULL)
		szOldMask[0] = 0;
	else
		_tcsncpy(szOldMask, mask, SIZEOF(szOldMask));

	m_Ok.OnClick = Callback(this, &CAddIgnoreDlg::OnOk);
}

void CAddIgnoreDlg::OnInitDialog()
{
	if (szOldMask[0] == 0) {
		if (m_proto->IsConnected())
			SetDlgItemText(m_hwnd, IDC_NETWORK, m_proto->m_info.sNetwork.c_str());
		CheckDlgButton(m_hwnd, IDC_Q, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_N, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_I, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_D, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_C, BST_CHECKED);
	}
}

void CAddIgnoreDlg::OnOk(CCtrlButton*)
{
	TCHAR szMask[500];
	TCHAR szNetwork[500];
	CMString flags;
	if (IsDlgButtonChecked(m_hwnd, IDC_Q) == BST_CHECKED) flags += 'q';
	if (IsDlgButtonChecked(m_hwnd, IDC_N) == BST_CHECKED) flags += 'n';
	if (IsDlgButtonChecked(m_hwnd, IDC_I) == BST_CHECKED) flags += 'i';
	if (IsDlgButtonChecked(m_hwnd, IDC_D) == BST_CHECKED) flags += 'd';
	if (IsDlgButtonChecked(m_hwnd, IDC_C) == BST_CHECKED) flags += 'c';
	if (IsDlgButtonChecked(m_hwnd, IDC_M) == BST_CHECKED) flags += 'm';

	GetDlgItemText(m_hwnd, IDC_MASK, szMask, SIZEOF(szMask));
	GetDlgItemText(m_hwnd, IDC_NETWORK, szNetwork, SIZEOF(szNetwork));

	CMString Mask = GetWord(szMask, 0);
	if (Mask.GetLength() != 0) {
		if (!_tcschr(Mask.c_str(), '!') && !_tcschr(Mask.c_str(), '@'))
			Mask += _T("!*@*");

		if (!flags.IsEmpty()) {
			if (*szOldMask)
				m_proto->RemoveIgnore(szOldMask);
			m_proto->AddIgnore(Mask.c_str(), flags.c_str(), szNetwork);
		}
	}
}

void CAddIgnoreDlg::OnClose()
{
	m_owner->FixButtons();
}

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

static int CALLBACK IgnoreListSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CIgnorePrefsDlg* hwndDlg = (CIgnorePrefsDlg*)lParamSort;
	if (!hwndDlg->GetHwnd())
		return 1;

	TCHAR temp1[512];
	TCHAR temp2[512];

	LVITEM lvm;
	lvm.mask = LVIF_TEXT;
	lvm.iSubItem = 0;
	lvm.cchTextMax = SIZEOF(temp1);

	lvm.iItem = lParam1;
	lvm.pszText = temp1;
	hwndDlg->m_list.GetItem(&lvm);

	lvm.iItem = lParam2;
	lvm.pszText = temp2;
	hwndDlg->m_list.GetItem(&lvm);

	if (temp1[0] && temp2[0])
		return mir_tstrcmpi(temp1, temp2);

	return (temp1[0] == 0) ? 1 : -1;
}

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
	TCHAR szTemp[MAX_PATH];
	mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%%miranda_path%%\\Plugins\\%S_ignore.ini"), m_szModuleName);
	TCHAR *szLoadFileName = Utils_ReplaceVarsT(szTemp);
	char* pszIgnoreData = IrcLoadFile(szLoadFileName);
	if (pszIgnoreData != NULL) {
		char *p1 = pszIgnoreData;
		while (*p1 != '\0') {
			while (*p1 == '\r' || *p1 == '\n')
				p1++;
			if (*p1 == '\0')
				break;

			char* p2 = strstr(p1, "\r\n");
			if (!p2)
				p2 = strchr(p1, '\0');

			char* pTemp = p2;
			while (pTemp > p1 && (*pTemp == '\r' || *pTemp == '\n' || *pTemp == '\0' || *pTemp == ' '))
				pTemp--;
			*++pTemp = 0;

			CMStringA mask = GetWord(p1, 0);
			CMStringA flags = GetWord(p1, 1);
			CMStringA network = GetWord(p1, 2);
			if (!mask.IsEmpty())
				m_ignoreItems.insert(new CIrcIgnoreItem(getCodepage(), mask.c_str(), flags.c_str(), network.c_str()));

			p1 = p2;
		}

		RewriteIgnoreSettings();
		delete[] pszIgnoreData;
		::_tremove(szLoadFileName);
	}
	mir_free(szLoadFileName);

	int idx = 0;
	char settingName[40];
	for (;;) {
		mir_snprintf(settingName, SIZEOF(settingName), "IGNORE:%d", idx++);

		DBVARIANT dbv;
		if (getTString(settingName, &dbv))
			break;

		CMString mask = GetWord(dbv.ptszVal, 0);
		CMString flags = GetWord(dbv.ptszVal, 1);
		CMString network = GetWord(dbv.ptszVal, 2);
		m_ignoreItems.insert(new CIrcIgnoreItem(mask.c_str(), flags.c_str(), network.c_str()));
		db_free(&dbv);
	}
}

void CIrcProto::RewriteIgnoreSettings(void)
{
	char settingName[40];

	int i = 0;
	for (;;) {
		mir_snprintf(settingName, SIZEOF(settingName), "IGNORE:%d", i++);
		if (db_unset(NULL, m_szModuleName, settingName))
			break;
	}

	for (i = 0; i < m_ignoreItems.getCount(); i++) {
		mir_snprintf(settingName, SIZEOF(settingName), "IGNORE:%d", i);

		CIrcIgnoreItem& C = m_ignoreItems[i];
		setTString(settingName, (C.mask + _T(" ") + C.flags + _T(" ") + C.network).c_str());
	}
}

CIgnorePrefsDlg::CIgnorePrefsDlg(CIrcProto* _pro)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_PREFS_IGNORE, NULL, false),
	m_list(this, IDC_LIST),
	m_add(this, IDC_ADD, LoadIconEx(IDI_ADD), LPGEN("Add new ignore")),
	m_edit(this, IDC_EDIT, LoadIconEx(IDI_EDIT), LPGEN("Edit this ignore")),
	m_del(this, IDC_DELETE, LoadIconEx(IDI_DELETE), LPGEN("Delete this ignore")),
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

void CIgnorePrefsDlg::OnInitDialog()
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

	static int COLUMNS_SIZES[3] = { 195, 60, 80 };
	LV_COLUMN lvC;
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	for (int index = 0; index < 3; index++) {
		lvC.iSubItem = index;
		lvC.cx = COLUMNS_SIZES[index];

		TCHAR* text = NULL;
		switch (index) {
		case 0: text = TranslateT("Ignore mask"); break;
		case 1: text = TranslateT("Flags"); break;
		case 2: text = TranslateT("Network"); break;
		}
		lvC.pszText = text;
		ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_INFO_LISTVIEW), index, &lvC);
	}

	ListView_SetExtendedListViewStyle(GetDlgItem(m_hwnd, IDC_INFO_LISTVIEW), LVS_EX_FULLROWSELECT);
	RebuildList();
}

INT_PTR CIgnorePrefsDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_LIST:
			switch (((NMHDR*)lParam)->code) {
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

void CIgnorePrefsDlg::OnEnableIgnore(CCtrlData*)
{
	m_ignoreChannel.Enable(m_enable.GetState());
	m_list.Enable(m_enable.GetState());
	m_add.Enable(m_enable.GetState());
}

void CIgnorePrefsDlg::OnIgnoreChat(CCtrlData*)
{
	m_ignoreUnknown.Enable(m_ignoreChat.GetState() == BST_UNCHECKED);
}

void CIgnorePrefsDlg::OnAdd(CCtrlButton*)
{
	CAddIgnoreDlg *dlg = new CAddIgnoreDlg(m_proto, NULL, this);
	dlg->Show();
	SetWindowText(dlg->GetHwnd(), TranslateT("Add ignore"));
	m_add.Disable();
	m_edit.Disable();
	m_del.Disable();
}

void CIgnorePrefsDlg::OnEdit(CCtrlButton*)
{
	if (!m_add.Enabled())
		return;

	TCHAR szMask[512];
	TCHAR szFlags[512];
	TCHAR szNetwork[512];
	int i = m_list.GetSelectionMark();
	m_list.GetItemText(i, 0, szMask, 511);
	m_list.GetItemText(i, 1, szFlags, 511);
	m_list.GetItemText(i, 2, szNetwork, 511);
	CAddIgnoreDlg* dlg = new CAddIgnoreDlg(m_proto, szMask, this);
	dlg->Show();
	HWND hWnd = dlg->GetHwnd();
	SetWindowText(hWnd, TranslateT("Edit ignore"));
	if (szFlags[0]) {
		if (_tcschr(szFlags, 'q'))
			CheckDlgButton(hWnd, IDC_Q, BST_CHECKED);
		if (_tcschr(szFlags, 'n'))
			CheckDlgButton(hWnd, IDC_N, BST_CHECKED);
		if (_tcschr(szFlags, 'i'))
			CheckDlgButton(hWnd, IDC_I, BST_CHECKED);
		if (_tcschr(szFlags, 'd'))
			CheckDlgButton(hWnd, IDC_D, BST_CHECKED);
		if (_tcschr(szFlags, 'c'))
			CheckDlgButton(hWnd, IDC_C, BST_CHECKED);
		if (_tcschr(szFlags, 'm'))
			CheckDlgButton(hWnd, IDC_M, BST_CHECKED);
	}
	SetDlgItemText(hWnd, IDC_MASK, szMask);
	SetDlgItemText(hWnd, IDC_NETWORK, szNetwork);
	m_add.Disable();
	m_edit.Disable();
	m_del.Disable();
}

void CIgnorePrefsDlg::OnDelete(CCtrlButton*)
{
	if (!m_del.Enabled())
		return;

	TCHAR szMask[512];
	int i = m_list.GetSelectionMark();
	m_list.GetItemText(i, 0, szMask, SIZEOF(szMask));
	m_proto->RemoveIgnore(szMask);
}

void CIgnorePrefsDlg::List_OnColumnClick(CCtrlListView::TEventInfo*)
{
	m_list.SortItems(IgnoreListSort, (LPARAM)this);
	UpdateList();
}

void CIgnorePrefsDlg::OnApply()
{
	m_proto->m_DCCFileEnabled = !m_ignoreFile.GetState();
	m_proto->m_DCCChatEnabled = !m_ignoreChat.GetState();
	m_proto->m_ignore = m_enable.GetState();
	m_proto->m_ignoreChannelDefault = m_ignoreChannel.GetState();
	m_proto->m_DCCChatIgnore = m_ignoreUnknown.GetState() ? 2 : 1;
	m_proto->WriteSettings(IgnoreSettings, SIZEOF(IgnoreSettings));
}

void CIgnorePrefsDlg::OnDestroy()
{
	m_proto->m_ignoreDlg = NULL;
	m_proto->m_ignoreItems.destroy();

	int i = m_list.GetItemCount();
	for (int j = 0; j < i; j++) {
		TCHAR szMask[512], szFlags[40], szNetwork[100];
		m_list.GetItemText(j, 0, szMask, SIZEOF(szMask));
		m_list.GetItemText(j, 1, szFlags, SIZEOF(szFlags));
		m_list.GetItemText(j, 2, szNetwork, SIZEOF(szNetwork));
		m_proto->m_ignoreItems.insert(new CIrcIgnoreItem(szMask, szFlags, szNetwork));
	}

	m_proto->RewriteIgnoreSettings();
}

void CIgnorePrefsDlg::FixButtons()
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

void CIgnorePrefsDlg::RebuildList()
{
	m_list.DeleteAllItems();

	for (int i = 0; i < m_proto->m_ignoreItems.getCount(); i++) {
		CIrcIgnoreItem& C = m_proto->m_ignoreItems[i];
		if (C.mask.IsEmpty() || C.flags[0] != '+')
			continue;

		LVITEM lvItem;
		lvItem.iItem = m_list.GetItemCount();
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem = 0;
		lvItem.lParam = lvItem.iItem;
		lvItem.pszText = (TCHAR*)C.mask.c_str();
		lvItem.iItem = m_list.InsertItem(&lvItem);

		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.pszText = (TCHAR*)C.flags.c_str();
		m_list.SetItem(&lvItem);

		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.pszText = (TCHAR*)C.network.c_str();
		m_list.SetItem(&lvItem);
	}

	UpdateList();
	m_list.SortItems(IgnoreListSort, (LPARAM)this);
	UpdateList();

	FixButtons();
}

void CIgnorePrefsDlg::UpdateList()
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

/////////////////////////////////////////////////////////////////////////////////////////

int CIrcProto::OnInitOptionsPages(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PREFS_CONNECT);
	odp.ptszTitle = m_tszUserName;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTab = LPGENT("Account");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.pfnDlgProc = CDlgBase::DynamicDlgProc;
	odp.dwInitParam = (LPARAM)&OptCreateAccount;
	OptCreateAccount.create = CConnectPrefsDlg::Create;
	OptCreateAccount.param = this;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PREFS_CTCP);
	odp.ptszTab = LPGENT("DCC and CTCP");
	odp.dwInitParam = (LPARAM)&OptCreateConn;
	OptCreateConn.create = CCtcpPrefsDlg::Create;
	OptCreateConn.param = this;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PREFS_OTHER);
	odp.ptszTab = LPGENT("Advanced");
	odp.dwInitParam = (LPARAM)&OptCreateOther;
	OptCreateOther.create = COtherPrefsDlg::Create;
	OptCreateOther.param = this;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PREFS_IGNORE);
	odp.ptszTab = LPGENT("Ignore");
	odp.dwInitParam = (LPARAM)&OptCreateIgnore;
	OptCreateIgnore.create = CIgnorePrefsDlg::Create;
	OptCreateIgnore.param = this;
	Options_AddPage(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIrcProto::InitPrefs(void)
{
	ConnectSettings[0].defStr = _T("Miranda");
	ConnectSettings[1].defStr = _T("UNIX");
	ConnectSettings[2].defStr = _T("113");
	ConnectSettings[3].defStr = _T("30");
	ConnectSettings[4].defStr = _T("10");

	CtcpSettings[0].defStr = STR_USERINFO;

	OtherSettings[0].defStr = STR_QUITMESSAGE;

	ReadSettings(ConnectSettings, SIZEOF(ConnectSettings));
	ReadSettings(CtcpSettings, SIZEOF(CtcpSettings));
	ReadSettings(OtherSettings, SIZEOF(OtherSettings));
	ReadSettings(IgnoreSettings, SIZEOF(IgnoreSettings));

	int x = getDword("SizeOfListBottom", -1);
	if (x != -1) {
		db_unset(NULL, m_szModuleName, "SizeOfListBottom");
		setDword("channelList_height", x);
	}
	if ((x = getDword("SizeOfListWidth", -1)) != -1) {
		db_unset(NULL, m_szModuleName, "SizeOfListWidth");
		setDword("channelList_width", x);
	}

	if (m_pNick[0] == 0) {
		if (m_nick[0] != 0) {
			memcpy(m_pNick, m_nick, sizeof(m_pNick));
			setTString("PNick", m_nick);
		}
	}
	else {
		memcpy(m_nick, m_pNick, sizeof(m_nick));
		setTString("Nick", m_nick);
	}

	m_mySpecifiedHostIP[0] = 0;

	if (m_alias == NULL)
		m_alias = mir_tstrdup(_T("/op /mode ## +ooo $1 $2 $3\r\n/dop /mode ## -ooo $1 $2 $3\r\n/voice /mode ## +vvv $1 $2 $3\r\n/dvoice /mode ## -vvv $1 $2 $3\r\n/j /join #$1 $2-\r\n/p /part ## $1-\r\n/w /whois $1\r\n/k /kick ## $1 $2-\r\n/q /query $1\r\n/logon /log on ##\r\n/logoff /log off ##\r\n/save /log buffer $1\r\n/slap /me slaps $1 around a bit with a large trout"));

	m_quickComboSelection = getDword("QuickComboSelection", m_serverComboSelection + 1);
	m_myHost[0] = '\0';

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

///////////////////////////////////////////////////////////////////////////////
// Account manager UI

struct CDlgAccMgrUI : public CProtoDlgBase<CIrcProto>
{
	CCtrlCombo m_serverCombo;
	CCtrlEdit  m_server, m_port, m_port2, m_pass, m_nick, m_nick2, m_name, m_userID, m_ssl;

	CDlgAccMgrUI(CIrcProto* _pro, HWND _owner)
		: CProtoDlgBase<CIrcProto>(_pro, IDD_ACCMGRUI, _owner, false),
		m_serverCombo(this, IDC_SERVERCOMBO),
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
		m_serverCombo.OnChange = Callback(this, &CDlgAccMgrUI::OnChangeCombo);
	}

	virtual void OnInitDialog()
	{
		for (int i = 0; i < g_servers.getCount(); i++) {
			SERVER_INFO& si = g_servers[i];
			m_serverCombo.AddStringA(si.m_name, LPARAM(&si));
		}
		m_serverCombo.SetCurSel(m_proto->m_serverComboSelection);
		m_server.SetTextA(m_proto->m_serverName);
		m_port.SetTextA(m_proto->m_portStart);
		m_port2.SetTextA(m_proto->m_portEnd);
		m_pass.SetTextA(m_proto->m_password);
		switch (m_proto->m_iSSL) {
			case 0:  m_ssl.SetTextA("Off");  break;
			case 1:  m_ssl.SetTextA("Auto"); break;
			case 2:  m_ssl.SetTextA("On");   break;
		}

		m_nick.SetText(m_proto->m_nick);
		m_nick2.SetText(m_proto->m_alternativeNick);
		m_userID.SetText(m_proto->m_userID);
		m_name.SetText(m_proto->m_name);
	}

	virtual void OnApply()
	{
		m_proto->m_serverComboSelection = m_serverCombo.GetCurSel();
		m_server.GetTextA(m_proto->m_serverName, SIZEOF(m_proto->m_serverName));
		m_port.GetTextA(m_proto->m_portStart, SIZEOF(m_proto->m_portStart));
		m_port2.GetTextA(m_proto->m_portEnd, SIZEOF(m_proto->m_portEnd));
		m_pass.GetTextA(m_proto->m_password, SIZEOF(m_proto->m_password));

		m_nick.GetText(m_proto->m_nick, SIZEOF(m_proto->m_nick));
		removeSpaces(m_proto->m_nick);
		_tcsncpy_s(m_proto->m_pNick, m_proto->m_nick, _TRUNCATE);
		m_nick2.GetText(m_proto->m_alternativeNick, SIZEOF(m_proto->m_alternativeNick));
		removeSpaces(m_proto->m_alternativeNick);
		m_userID.GetText(m_proto->m_userID, SIZEOF(m_proto->m_userID));
		removeSpaces(m_proto->m_userID);
		m_name.GetText(m_proto->m_name, SIZEOF(m_proto->m_name));
		m_proto->WriteSettings(ConnectSettings, SIZEOF(ConnectSettings));
	}

	void OnChangeCombo(CCtrlCombo*)
	{
		int i = m_serverCombo.GetCurSel();
		SERVER_INFO* pData = (SERVER_INFO*)m_serverCombo.GetItemData(i);
		if (pData && (INT_PTR)pData != CB_ERR) {
			m_server.SetTextA(pData->m_address);
			m_port.SetInt(pData->m_portStart);
			m_port2.SetInt(pData->m_portEnd);
			m_pass.SetTextA("");
			switch (pData->m_iSSL) {
				case 0:  m_ssl.SetTextA("Off");  break;
				case 1:  m_ssl.SetTextA("Auto"); break;
				case 2:  m_ssl.SetTextA("On");   break;
			}
		}
	}
};

INT_PTR CIrcProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	CDlgAccMgrUI *dlg = new CDlgAccMgrUI(this, (HWND)lParam);
	dlg->Show();
	return (INT_PTR)dlg->GetHwnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Initialize servers list

static void sttImportIni(const TCHAR* szIniFile)
{
	FILE* serverFile = _tfopen(szIniFile, _T("r"));
	if (serverFile == NULL)
		return;

	char buf1[500], buf2[200];
	while (fgets(buf1, sizeof(buf1), serverFile)) {
		char* p = strchr(buf1, '=');
		if (!p)
			continue;

		p++;
		rtrim(p);
		char* p1 = strstr(p, "SERVER:");
		if (!p1)
			continue;

		memcpy(buf2, p, int(p1 - p));
		buf2[int(p1 - p)] = 0;
		db_set_s(NULL, SERVERSMODULE, buf2, p1);
	}
	fclose(serverFile);
	::_tremove(szIniFile);
}

void InitServers()
{
	TCHAR *szTemp = Utils_ReplaceVarsT(_T("%miranda_path%\\Plugins\\IRC_servers.ini"));
	sttImportIni(szTemp);
	mir_free(szTemp);

	RereadServers();

	if (g_servers.getCount() == 0) {
		TCHAR *szIniFile = Utils_ReplaceVarsT(_T("%temp%\\default_servers.ini"));
		FILE *serverFile = _tfopen(szIniFile, _T("a"));
		if (serverFile) {
			char* pszSvrs = (char*)LockResource(LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_SERVERS), _T("TEXT"))));
			if (pszSvrs)
				fwrite(pszSvrs, 1, mir_strlen(pszSvrs) + 1, serverFile);
			fclose(serverFile);

			sttImportIni(szIniFile);
			RereadServers();
		}
		mir_free(szIniFile);
	}
}
