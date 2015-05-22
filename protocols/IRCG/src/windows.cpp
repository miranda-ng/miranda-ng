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

/////////////////////////////////////////////////////////////////////////////////////////
// Message Box

CMessageBoxDlg::CMessageBoxDlg(CIrcProto *_pro, DCCINFO *_dci)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_MESSAGEBOX, false),
	pdci(_dci),
	m_Ok(this, IDOK)
{
	m_Ok.OnClick = Callback(this, &CMessageBoxDlg::OnOk);
}

void CMessageBoxDlg::OnOk(CCtrlButton*)
{
	CDccSession *dcc = new CDccSession(m_proto, pdci);

	CDccSession *olddcc = m_proto->FindDCCSession(pdci->hContact);
	if (olddcc)
		olddcc->Disconnect();
	m_proto->AddDCCSession(pdci->hContact, dcc);

	dcc->Connect();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Whois dialog

CWhoisDlg::CWhoisDlg(CIrcProto *_pro)
	: CCoolIrcDlg(_pro, IDD_INFO),
	m_InfoNick(this, IDC_INFO_NICK),
	m_Reply(this, IDC_REPLY),
	m_Caption(this, IDC_CAPTION),
	m_AwayTime(this, IDC_AWAYTIME),
	m_InfoName(this, IDC_INFO_NAME),
	m_InfoId(this, IDC_INFO_ID),
	m_InfoAddress(this, IDC_INFO_ADDRESS),
	m_InfoChannels(this, IDC_INFO_CHANNELS),
	m_InfoAuth(this, IDC_INFO_AUTH),
	m_InfoServer(this, IDC_INFO_SERVER),
	m_InfoAway2(this, IDC_INFO_AWAY2),
	m_InfoOther(this, IDC_INFO_OTHER),
	m_Ping(this, IDC_PING),
	m_Version(this, IDC_VERSION),
	m_Time(this, IDC_TIME),
	m_userInfo(this, IDC_USERINFO),
	m_Refresh(this, ID_INFO_GO),
	m_Query(this, ID_INFO_QUERY)
{
	m_Ping.OnClick = Callback(this, &CWhoisDlg::OnPing);
	m_Version.OnClick = Callback(this, &CWhoisDlg::OnVersion);
	m_Time.OnClick = Callback(this, &CWhoisDlg::OnTime);
	m_userInfo.OnClick = Callback(this, &CWhoisDlg::OnUserInfo);
	m_Refresh.OnClick = Callback(this, &CWhoisDlg::OnGo);
	m_Query.OnClick = Callback(this, &CWhoisDlg::OnQuery);
}

void CWhoisDlg::OnInitDialog()
{
	LOGFONT lf;
	HFONT hFont = (HFONT)m_AwayTime.SendMsg(WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	hFont = CreateFontIndirect(&lf);
	m_AwayTime.SendMsg(WM_SETFONT, (WPARAM)hFont, 0);

	CCoolIrcDlg::OnInitDialog();

	WindowSetIcon(m_hwnd, IDI_WHOIS);
}

void CWhoisDlg::OnClose()
{
	ShowWindow(m_hwnd, SW_HIDE);
	SendMessage(m_hwnd, WM_SETREDRAW, FALSE, 0);
}

void CWhoisDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();

	HFONT hFont2 = (HFONT)SendDlgItemMessage(m_hwnd, IDC_AWAYTIME, WM_GETFONT, 0, 0);
	SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_SETFONT, SendDlgItemMessage(m_hwnd, IDOK, WM_GETFONT, 0, 0), 0);
	DeleteObject(hFont2);

	m_proto->m_whoisDlg = NULL;
}

void CWhoisDlg::OnGo(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
	m_proto->PostIrcMessage(_T("/WHOIS %s %s"), szTemp, szTemp);
}

void CWhoisDlg::OnQuery(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
	m_proto->PostIrcMessage(_T("/QUERY %s"), szTemp);
}

void CWhoisDlg::OnPing(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(_T("/PRIVMSG %s \001PING %u\001"), szTemp, time(0));
}

void CWhoisDlg::OnUserInfo(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(_T("/PRIVMSG %s \001USERINFO\001"), szTemp);
}

void CWhoisDlg::OnTime(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(_T("/PRIVMSG %s \001TIME\001"), szTemp);
}

void CWhoisDlg::OnVersion(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(_T("/PRIVMSG %s \001VERSION\001"), szTemp);
}

void CWhoisDlg::ShowMessage(const CIrcMessage* pmsg)
{
	if (m_InfoNick.SendMsg(CB_FINDSTRINGEXACT, -1, (LPARAM)pmsg->parameters[1].c_str()) == CB_ERR)
		m_InfoNick.SendMsg(CB_ADDSTRING, 0, (LPARAM)pmsg->parameters[1].c_str());
	int i = m_InfoNick.SendMsg(CB_FINDSTRINGEXACT, -1, (LPARAM)pmsg->parameters[1].c_str());
	m_InfoNick.SendMsg(CB_SETCURSEL, i, 0);
	m_Caption.SetText(pmsg->parameters[1].c_str());
	m_InfoName.SetText(pmsg->parameters[5].c_str());
	m_InfoAddress.SetText(pmsg->parameters[3].c_str());
	m_InfoId.SetText(pmsg->parameters[2].c_str());
	m_InfoChannels.SetText(_T(""));
	m_InfoServer.SetText(_T(""));
	m_InfoAway2.SetText(_T(""));
	m_InfoAuth.SetText(_T(""));
	m_InfoOther.SetText(_T(""));
	m_Reply.SetText(_T(""));
	SetWindowText(m_hwnd, TranslateT("User information"));
	EnableWindow(GetDlgItem(m_hwnd, ID_INFO_QUERY), true);
	ShowWindow(m_hwnd, SW_SHOW);
	if (IsIconic(m_hwnd))
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
	SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(m_hwnd, NULL, TRUE);
}

void CWhoisDlg::ShowMessageNoUser(const CIrcMessage *pmsg)
{
	m_InfoNick.SetText(pmsg->parameters[2].c_str());
	m_InfoNick.SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
	m_Caption.SetText(pmsg->parameters[2].c_str());
	m_InfoName.SetText(_T(""));
	m_InfoAddress.SetText(_T(""));
	m_InfoId.SetText(_T(""));
	m_InfoChannels.SetText(_T(""));
	m_InfoServer.SetText(_T(""));
	m_InfoAway2.SetText(_T(""));
	m_InfoAuth.SetText(_T(""));
	m_Reply.SetText(_T(""));
	EnableWindow(GetDlgItem(m_hwnd, ID_INFO_QUERY), false);
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Change nickname' dialog

CNickDlg::CNickDlg(CIrcProto *_pro)
	: CCoolIrcDlg(_pro, IDD_NICK),
	m_Ok(this, IDOK),
	m_Enick(this, IDC_ENICK)
{
	m_Ok.OnClick = Callback(this, &CNickDlg::OnOk);
}

void CNickDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();
	WindowSetIcon(m_hwnd, IDI_RENAME);

	DBVARIANT dbv;
	if (!m_proto->getTString("RecentNicks", &dbv)) {
		for (int i = 0; i < 10; i++)
			if (!GetWord(dbv.ptszVal, i).IsEmpty())
				SendDlgItemMessage(m_hwnd, IDC_ENICK, CB_ADDSTRING, 0, (LPARAM)GetWord(dbv.ptszVal, i).c_str());

		db_free(&dbv);
	}
}

void CNickDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();
	m_proto->m_nickDlg = NULL;
}

void CNickDlg::OnOk(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_Enick.GetText(szTemp, SIZEOF(szTemp));
	m_proto->PostIrcMessage(_T("/NICK %s"), szTemp);

	CMString S = szTemp;
	DBVARIANT dbv;
	if (!m_proto->getTString("RecentNicks", &dbv)) {
		for (int i = 0; i < 10; i++) {
			CMString s = GetWord(dbv.ptszVal, i);
			if (!s.IsEmpty() && s != szTemp)
				S += _T(" ") + s;
		}
		db_free(&dbv);
	}
	m_proto->setTString("RecentNicks", S.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Change nickname' dialog

#define LIST_TIMER 10

CListDlg::CListDlg(CIrcProto *_pro)
	: CProtoDlgBase<CIrcProto>(_pro, IDD_LIST, false),
	m_Join(this, IDC_JOIN),
	m_list(this, IDC_INFO_LISTVIEW),
	m_list2(this, IDC_INFO_LISTVIEW2),
	m_status(this, IDC_TEXT),
	m_filter(this, IDC_FILTER_STRING)
{
	m_list2.OnDoubleClick = m_list.OnDoubleClick = m_Join.OnClick = Callback(this, &CListDlg::OnJoin);
	m_list.OnColumnClick = Callback(this, &CListDlg::List_OnColumnClick);
}

void CListDlg::OnInitDialog()
{
	RECT screen;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
	LVCOLUMN lvC;
	int COLUMNS_SIZES[4] = { 200, 50, 50, 2000 };
	TCHAR szBuffer[32];

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	for (int index = 0; index < 4; index++) {
		lvC.iSubItem = index;
		lvC.cx = COLUMNS_SIZES[index];

		switch (index) {
			case 0: mir_tstrcpy(szBuffer, TranslateT("Channel")); break;
			case 1: mir_tstrcpy(szBuffer, _T("#"));               break;
			case 2: mir_tstrcpy(szBuffer, TranslateT("Mode"));    break;
			case 3: mir_tstrcpy(szBuffer, TranslateT("Topic"));   break;
		}
		lvC.pszText = szBuffer;
		m_list.InsertColumn(index, &lvC);
		m_list2.InsertColumn(index, &lvC);
	}

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "channelList_");

	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	m_list2.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	WindowSetIcon(m_hwnd, IDI_LIST);
	m_status.SetText(TranslateT("Please wait..."));
}

INT_PTR CListDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_TIMER) {
		::KillTimer(m_hwnd, m_timer); m_timer = 0;

		// Retrieve the input text
		TCHAR strFilterText[255];
		TCHAR newTitle[255];
		m_filter.GetText(strFilterText, SIZEOF(strFilterText));

		if (strFilterText[0]) {
			int itemCount = 0;
			int j = m_list.GetItemCount();
			if (j <= 0)
				return FALSE;

			// Empty the filtered list
			m_list2.DeleteAllItems();

			LVITEM lvm;
			TCHAR text[255];
			lvm.pszText = text;	// Set buffer for texts
			lvm.cchTextMax = SIZEOF(text);
			lvm.mask = LVIF_TEXT;
			for (int i = 0; i < j; i++) {
				lvm.iSubItem = 0;	// First column
				lvm.iItem = i;
				m_list.GetItem(&lvm);

				// Match the text?
				TCHAR* t = _tcsstr(lvm.pszText, strFilterText);
				if (t == NULL) { // If no, then Check if in the topics
					m_list.GetItem(&lvm);

					// Match the text?
					t = _tcsstr(lvm.pszText, strFilterText);
				}

				if (t) {
					++itemCount;

					// Column 0
					LVITEM lvItem;
					lvItem.iItem = m_list2.GetItemCount();
					lvItem.mask = LVIF_TEXT | LVIF_PARAM;

					lvItem.iSubItem = 0;
					lvItem.pszText = lvm.pszText;
					lvItem.lParam = lvItem.iItem;
					lvItem.iItem = m_list2.InsertItem(&lvItem);

					// Column 2
					lvm.mask = LVIF_TEXT;
					lvm.iSubItem = 1;
					lvm.iItem = i;
					m_list.GetItem(&lvm);

					lvItem.mask = LVIF_TEXT;
					lvItem.iSubItem = 1;
					lvItem.pszText = lvm.pszText;
					m_list2.SetItem(&lvItem);

					// Column 4
					lvm.mask = LVIF_TEXT;
					lvm.iSubItem = 3;
					lvm.iItem = i;
					m_list.GetItem(&lvm);

					lvItem.mask = LVIF_TEXT;
					lvItem.pszText = lvm.pszText;
					lvItem.iSubItem = 3;
					m_list2.SetItem(&lvItem);
				}
			}

			// Show the list
			SetWindowPos(m_list2.GetHwnd(), HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
			ShowWindow(m_list.GetHwnd(), SW_HIDE);

			// New dialog title
			mir_sntprintf(newTitle, SIZEOF(newTitle), TranslateT("%s - Filtered - %d items"), strFilterText, itemCount);
			SetWindowText(m_hwnd, newTitle);
		}
		else {
			ShowWindow(m_list.GetHwnd(), SW_SHOW);
			ShowWindow(m_list2.GetHwnd(), SW_HIDE);
			mir_sntprintf(newTitle, SIZEOF(newTitle), TranslateT("Channels on server"));
			SetWindowText(m_hwnd, newTitle);
		}
	}

	return CProtoDlgBase<CIrcProto>::DlgProc(msg, wParam, lParam);
}

void CListDlg::OnChange(CCtrlBase *ctrl)
{
	if (ctrl->GetCtrlId() == IDC_FILTER_STRING)
		m_timer = ::SetTimer(m_hwnd, LIST_TIMER, 200, NULL);
}

void CListDlg::OnDestroy()
{
	if (m_timer)
		::KillTimer(m_hwnd, m_timer);
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "channelList_");
	m_proto->m_listDlg = NULL;
}

struct ListViewSortParam
{
	CCtrlListView* pList;
	int iSubItem;
};

static int CALLBACK ListViewSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ListViewSortParam* param = (ListViewSortParam*)lParamSort;
	if (!param->pList->GetHwnd())
		return 0;

	TCHAR temp1[512];
	TCHAR temp2[512];
	LVITEM lvm;
	lvm.mask = LVIF_TEXT;
	lvm.iItem = lParam1;
	lvm.iSubItem = param->iSubItem;
	lvm.pszText = temp1;
	lvm.cchTextMax = SIZEOF(temp1);
	param->pList->GetItem(&lvm);
	lvm.iItem = lParam2;
	lvm.pszText = temp2;
	param->pList->GetItem(&lvm);
	if (param->iSubItem != 1) {
		if (mir_tstrlen(temp1) != 0 && mir_tstrlen(temp2) != 0)
			return mir_tstrcmpi(temp1, temp2);

		return (*temp1 == 0) ? 1 : -1;
	}

	return (_ttoi(temp1) < _ttoi(temp2)) ? 1 : -1;
}

int CListDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_INFO_LISTVIEW:
	case IDC_INFO_LISTVIEW2:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT | RD_ANCHORX_WIDTH;
	case IDC_FILTER_STRING:
	case IDC_FILTER_BTN:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	case IDC_TEXT:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM | RD_ANCHORX_WIDTH;
	}

	return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
}

void CListDlg::List_OnColumnClick(CCtrlListView::TEventInfo *ev)
{
	ListViewSortParam param = { &m_list, ev->nmlv->iSubItem };
	m_list.SortItems(ListViewSort, (LPARAM)&param);
	UpdateList();
}

void CListDlg::OnJoin(CCtrlButton*)
{
	TCHAR szTemp[255];
	m_filter.GetText(szTemp, SIZEOF(szTemp));

	if (szTemp[0])
		m_list2.GetItemText(m_list2.GetSelectionMark(), 0, szTemp, 255);
	else
		m_list.GetItemText(m_list.GetSelectionMark(), 0, szTemp, 255);
	m_proto->PostIrcMessage(_T("/JOIN %s"), szTemp);
}

void CListDlg::UpdateList()
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
// 'Join' dialog

CJoinDlg::CJoinDlg(CIrcProto *_pro)
	: CCoolIrcDlg(_pro, IDD_NICK),
	m_Ok(this, IDOK)
{
	m_Ok.OnClick = Callback(this, &CJoinDlg::OnOk);
}

void CJoinDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	DBVARIANT dbv;
	if (!m_proto->getTString("RecentChannels", &dbv)) {
		for (int i = 0; i < 20; i++) {
			if (!GetWord(dbv.ptszVal, i).IsEmpty()) {
				CMString S = GetWord(dbv.ptszVal, i);
				S.Replace(_T("%newl"), _T(" "));
				SendDlgItemMessage(m_hwnd, IDC_ENICK, CB_ADDSTRING, 0, (LPARAM)S.c_str());
			}
		}
		db_free(&dbv);
	}
}

void CJoinDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();
	m_proto->m_joinDlg = NULL;
}

void CJoinDlg::OnOk(CCtrlButton*)
{
	TCHAR szTemp[255];
	GetDlgItemText(m_hwnd, IDC_ENICK, szTemp, SIZEOF(szTemp));
	if (m_proto->IsChannel(szTemp))
		m_proto->PostIrcMessage(_T("/JOIN %s"), szTemp);
	else
		m_proto->PostIrcMessage(_T("/JOIN #%s"), szTemp);

	CMString S = szTemp;
	S.Replace(_T(" "), _T("%newl"));
	CMString SL = S;

	DBVARIANT dbv;
	if (!m_proto->getTString("RecentChannels", &dbv)) {
		for (int i = 0; i < 20; i++) {
			CMString W = GetWord(dbv.ptszVal, i);
			if (!W.IsEmpty() && W != SL)
				S += _T(" ") + W;
		}
		db_free(&dbv);
	}
	m_proto->setTString("RecentChannels", S.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Quick' dialog

CQuickDlg::CQuickDlg(CIrcProto *_pro)
	: CCoolIrcDlg(_pro, IDD_QUICKCONN),
	m_Ok(this, IDOK),
	m_serverCombo(this, IDC_SERVERCOMBO)
{
	m_Ok.OnClick = Callback(this, &CQuickDlg::OnOk);
	m_serverCombo.OnChange = Callback(this, &CQuickDlg::OnServerCombo);
}

void CQuickDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	if (g_servers.getCount() > 0) {
		for (int i = 0; i < g_servers.getCount(); i++) {
			const SERVER_INFO& si = g_servers[i];
			m_serverCombo.AddStringA(si.m_name, (LPARAM)&si);
		}
	}
	else EnableWindow(GetDlgItem(m_hwnd, IDOK), false);

	m_si = new SERVER_INFO;
	m_si->m_group = mir_strdup("");
	m_si->m_name = mir_strdup(Translate("---- Not listed server ----"));

	DBVARIANT dbv;
	if (!m_proto->getString("ServerName", &dbv)) {
		m_si->m_address = mir_strdup(dbv.pszVal);
		db_free(&dbv);
	}
	else m_si->m_address = mir_strdup(Translate("Type new server address here"));

	if (!m_proto->getString("PortStart", &dbv)) {
		m_si->m_portStart = atoi(dbv.pszVal);
		db_free(&dbv);
	}
	else m_si->m_portStart = 6667;

	if (!m_proto->getString("PortEnd", &dbv)) {
		m_si->m_portEnd = atoi(dbv.pszVal);
		db_free(&dbv);
	}
	else m_si->m_portEnd = 6667;

	m_si->m_iSSL = m_proto->getByte("UseSSL", 0);

	m_serverCombo.AddStringA(m_si->m_name, (LPARAM)m_si);

	if (m_proto->m_quickComboSelection != -1) {
		m_serverCombo.SetCurSel(m_proto->m_quickComboSelection);
		OnServerCombo(NULL);
	}
	else EnableWindow(GetDlgItem(m_hwnd, IDOK), false);
}

void CQuickDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();

	delete m_si;
	m_proto->m_quickDlg = NULL;
}

void CQuickDlg::OnOk(CCtrlButton*)
{
	GetDlgItemTextA(m_hwnd, IDC_SERVER, m_proto->m_serverName, SIZEOF(m_proto->m_serverName));
	GetDlgItemTextA(m_hwnd, IDC_PORT, m_proto->m_portStart, SIZEOF(m_proto->m_portStart));
	GetDlgItemTextA(m_hwnd, IDC_PORT2, m_proto->m_portEnd, SIZEOF(m_proto->m_portEnd));
	GetDlgItemTextA(m_hwnd, IDC_PASS, m_proto->m_password, SIZEOF(m_proto->m_password));

	int i = m_serverCombo.GetCurSel();
	SERVER_INFO* pData = (SERVER_INFO*)m_serverCombo.GetItemData(i);
	if (pData && (INT_PTR)pData != CB_ERR) {
		mir_strcpy(m_proto->m_network, pData->m_group);
		pData->m_iSSL = 0;
		if (IsDlgButtonChecked(m_hwnd, IDC_SSL_ON))
			pData->m_iSSL = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_SSL_AUTO))
			pData->m_iSSL = 1;
		m_proto->m_iSSL = pData->m_iSSL;
	}

	TCHAR windowname[20];
	GetWindowText(m_hwnd, windowname, SIZEOF(windowname));
	if (mir_tstrcmpi(windowname, _T("Miranda IRC")) == 0) {
		m_proto->m_serverComboSelection = m_serverCombo.GetCurSel() - 1;
		m_proto->setDword("ServerComboSelection", m_proto->m_serverComboSelection);
		m_proto->setString("ServerName", m_proto->m_serverName);
		m_proto->setString("PortStart", m_proto->m_portStart);
		m_proto->setString("PortEnd", m_proto->m_portEnd);
		m_proto->setString("Password", m_proto->m_password);
		m_proto->setString("Network", m_proto->m_network);
		m_proto->setByte("UseSSL", m_proto->m_iSSL);
	}
	m_proto->m_quickComboSelection = m_serverCombo.GetCurSel();
	m_proto->setDword("QuickComboSelection", m_proto->m_quickComboSelection);
	m_proto->DisconnectFromServer();
	m_proto->ConnectToServer();
}

void CQuickDlg::OnServerCombo(CCtrlData*)
{
	int i = m_serverCombo.GetCurSel();
	if (i == CB_ERR)
		return;

	SERVER_INFO* pData = (SERVER_INFO*)m_serverCombo.GetItemData(i);
	SetDlgItemTextA(m_hwnd, IDC_SERVER, pData->m_address);
	SetDlgItemTextA(m_hwnd, IDC_PASS, "");
	SetDlgItemInt(m_hwnd, IDC_PORT, pData->m_portStart, FALSE);
	SetDlgItemInt(m_hwnd, IDC_PORT2, pData->m_portEnd, FALSE);

	if (pData->m_iSSL == 0) {
		CheckDlgButton(m_hwnd, IDC_SSL_OFF, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_SSL_AUTO, BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SSL_ON, BST_UNCHECKED);
	}
	if (pData->m_iSSL == 1) {
		CheckDlgButton(m_hwnd, IDC_SSL_AUTO, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_SSL_OFF, BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SSL_ON, BST_UNCHECKED);
	}
	if (pData->m_iSSL == 2) {
		CheckDlgButton(m_hwnd, IDC_SSL_ON, BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_SSL_OFF, BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SSL_AUTO, BST_UNCHECKED);
	}

	if (!mir_strcmp(pData->m_name, Translate("---- Not listed server ----"))) {
		SendDlgItemMessage(m_hwnd, IDC_SERVER, EM_SETREADONLY, false, 0);
		SendDlgItemMessage(m_hwnd, IDC_PORT, EM_SETREADONLY, false, 0);
		SendDlgItemMessage(m_hwnd, IDC_PORT2, EM_SETREADONLY, false, 0);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SSL_OFF), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SSL_AUTO), TRUE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SSL_ON), TRUE);
	}
	else {
		SendDlgItemMessage(m_hwnd, IDC_SERVER, EM_SETREADONLY, true, 0);
		SendDlgItemMessage(m_hwnd, IDC_PORT, EM_SETREADONLY, true, 0);
		SendDlgItemMessage(m_hwnd, IDC_PORT2, EM_SETREADONLY, true, 0);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SSL_OFF), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SSL_AUTO), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SSL_ON), FALSE);
	}

	EnableWindow(GetDlgItem(m_hwnd, IDOK), true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Question' dialog

CQuestionDlg::CQuestionDlg(CIrcProto *_pro, CManagerDlg *owner)
	: CCoolIrcDlg(_pro, IDD_QUESTION),
	m_Ok(this, IDOK),
	m_owner(owner)
{
	if (owner != NULL)
		m_hwndParent = owner->GetHwnd();
	m_Ok.OnClick = Callback(this, &CQuestionDlg::OnOk);
}

void CQuestionDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	WindowSetIcon(m_hwnd, IDI_IRCQUESTION);
}

void CQuestionDlg::OnClose()
{
	if (m_owner)
		m_owner->CloseQuestion();
}

void CQuestionDlg::OnOk(CCtrlButton*)
{
	int i = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_EDIT));
	if (i > 0) {
		TCHAR* l = new TCHAR[i + 2];
		GetDlgItemText(m_hwnd, IDC_EDIT, l, i + 1);

		int j = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_HIDDENEDIT));
		TCHAR *m = new TCHAR[j + 2];
		GetDlgItemText(m_hwnd, IDC_HIDDENEDIT, m, j + 1);

		TCHAR *text = _tcsstr(m, _T("%question"));
		TCHAR *p1 = text;
		TCHAR *p2 = NULL;
		if (p1) {
			p1 += 9;
			if (*p1 == '=' && p1[1] == '\"') {
				p1 += 2;
				for (int k = 0; k < 3; k++) {
					p2 = _tcschr(p1, '\"');
					if (p2) {
						p2++;
						if (k == 2 || (*p2 != ',' || (*p2 == ',' && p2[1] != '\"')))
							*p2 = '\0';
						else
							p2 += 2;
						p1 = p2;
					}
				}
			}
			else *p1 = '\0';
		}

		CMString S(_T('\0'), j + 2);
		GetDlgItemText(m_hwnd, IDC_HIDDENEDIT, S.GetBuffer(), j + 1);
		S.Replace(text, l);
		m_proto->PostIrcMessageWnd(NULL, NULL, S);

		delete[]m;
		delete[]l;

		if (m_owner)
			m_owner->ApplyQuestion();
	}
}

void CQuestionDlg::Activate()
{
	ShowWindow(m_hwnd, SW_SHOW);
	SetActiveWindow(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Channel Manager' dialog

CManagerDlg::CManagerDlg(CIrcProto *_pro)
	: CCoolIrcDlg(_pro, IDD_CHANMANAGER),
	m_list(this, IDC_LIST),

	m_check1(this, IDC_CHECK1),
	m_check2(this, IDC_CHECK2),
	m_check3(this, IDC_CHECK3),
	m_check4(this, IDC_CHECK4),
	m_check5(this, IDC_CHECK5),
	m_check6(this, IDC_CHECK6),
	m_check7(this, IDC_CHECK7),
	m_check8(this, IDC_CHECK8),
	m_check9(this, IDC_CHECK9),

	m_key(this, IDC_KEY),
	m_limit(this, IDC_LIMIT),
	m_topic(this, IDC_TOPIC),

	m_add(this, IDC_ADD, LoadIconEx(IDI_ADD), LPGEN("Add ban/invite/exception")),
	m_edit(this, IDC_EDIT, LoadIconEx(IDI_EDIT), LPGEN("Edit selected ban/invite/exception")),
	m_remove(this, IDC_REMOVE, LoadIconEx(IDI_DELETE), LPGEN("Delete selected ban/invite/exception")),
	m_applyModes(this, IDC_APPLYMODES, LoadIconEx(IDI_APPLY), LPGEN("Set these modes for the channel")),
	m_applyTopic(this, IDC_APPLYTOPIC, LoadIconEx(IDI_APPLY), LPGEN("Set this topic for the channel")),

	m_radio1(this, IDC_RADIO1),
	m_radio2(this, IDC_RADIO2),
	m_radio3(this, IDC_RADIO3)
{
	m_add.OnClick = Callback(this, &CManagerDlg::OnAdd);
	m_edit.OnClick = Callback(this, &CManagerDlg::OnEdit);
	m_remove.OnClick = Callback(this, &CManagerDlg::OnRemove);

	m_applyModes.OnClick = Callback(this, &CManagerDlg::OnApplyModes);
	m_applyTopic.OnClick = Callback(this, &CManagerDlg::OnApplyTopic);

	m_check1.OnChange = Callback(this, &CManagerDlg::OnCheck);
	m_check2.OnChange = Callback(this, &CManagerDlg::OnCheck);
	m_check3.OnChange = Callback(this, &CManagerDlg::OnCheck);
	m_check4.OnChange = Callback(this, &CManagerDlg::OnCheck);
	m_check5.OnChange = Callback(this, &CManagerDlg::OnCheck5);
	m_check6.OnChange = Callback(this, &CManagerDlg::OnCheck6);
	m_check7.OnChange = Callback(this, &CManagerDlg::OnCheck);
	m_check8.OnChange = Callback(this, &CManagerDlg::OnCheck);
	m_check9.OnChange = Callback(this, &CManagerDlg::OnCheck);

	m_key.OnChange = Callback(this, &CManagerDlg::OnChangeModes);
	m_limit.OnChange = Callback(this, &CManagerDlg::OnChangeModes);
	m_topic.OnChange = Callback(this, &CManagerDlg::OnChangeTopic);

	m_radio1.OnChange = Callback(this, &CManagerDlg::OnRadio);
	m_radio2.OnChange = Callback(this, &CManagerDlg::OnRadio);
	m_radio3.OnChange = Callback(this, &CManagerDlg::OnRadio);

	m_list.OnDblClick = Callback(this, &CManagerDlg::OnListDblClick);
	m_list.OnSelChange = Callback(this, &CManagerDlg::OnChangeList);
}

LRESULT CALLBACK MgrEditSubclassProc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 21 || wParam == 11 || wParam == 2) {
			char w[2];
			if (wParam == 11) {
				w[0] = 3;
				w[1] = '\0';
			}
			if (wParam == 2) {
				w[0] = 2;
				w[1] = '\0';
			}
			if (wParam == 21) {
				w[0] = 31;
				w[1] = '\0';
			}
			SendMessage(m_hwnd, EM_REPLACESEL, false, (LPARAM)w);
			SendMessage(m_hwnd, EM_SCROLLCARET, 0, 0);
			return 0;
		}
		break;
	}

	return mir_callNextSubclass(m_hwnd, MgrEditSubclassProc, msg, wParam, lParam);
}

void CManagerDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	POINT pt;
	pt.x = 3;
	pt.y = 3;
	HWND hwndEdit = ChildWindowFromPoint(m_topic.GetHwnd(), pt);
	mir_subclassWindow(hwndEdit, MgrEditSubclassProc);

	WindowSetIcon(m_hwnd, IDI_MANAGER);

	m_list.SendMsg(LB_SETHORIZONTALEXTENT, 750, NULL);
	m_radio1.SetState(true);

	const char* modes = m_proto->sChannelModes.c_str();
	if (!strchr(modes, 't')) m_check1.Disable();
	if (!strchr(modes, 'n')) m_check2.Disable();
	if (!strchr(modes, 'i')) m_check3.Disable();
	if (!strchr(modes, 'm')) m_check4.Disable();
	if (!strchr(modes, 'k')) m_check5.Disable();
	if (!strchr(modes, 'l')) m_check6.Disable();
	if (!strchr(modes, 'p')) m_check7.Disable();
	if (!strchr(modes, 's')) m_check8.Disable();
	if (!strchr(modes, 'c')) m_check9.Disable();
}

void CManagerDlg::OnClose()
{
	if (m_applyModes.Enabled() || m_applyTopic.Enabled()) {
		int i = MessageBox(NULL, TranslateT("You have not applied all changes!\n\nApply before exiting?"), TranslateT("IRC warning"), MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON3);
		if (i == IDCANCEL) {
			m_lresult = TRUE;
			return;
		}

		if (i == IDYES) {
			if (m_applyModes.Enabled())
				OnApplyModes(NULL);
			if (m_applyTopic.Enabled())
				OnApplyTopic(NULL);
		}
	}

	TCHAR window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));
	CMString S = _T("");
	TCHAR temp[1000];
	for (int i = 0; i < 5; i++) {
		if (m_topic.SendMsg(CB_GETLBTEXT, i, (LPARAM)temp) != LB_ERR) {
			CMString S1 = temp;
			/* FIXME: What the hell does it mean!? GCC won't compile this on UNICODE */
#if !defined(__GNUC__) || !defined(UNICODE)
			S1.Replace(_T(" "), _T("%¤"));
#endif
			S += _T(" ") + S1;
		}
	}

	if (!S.IsEmpty() && m_proto->IsConnected()) {
		mir_sntprintf(temp, SIZEOF(temp), _T("Topic%s%s"), window, m_proto->m_info.sNetwork.c_str());
		char* p = mir_t2a(temp);
		m_proto->setTString(p, S.c_str());
		mir_free(p);
	}
	DestroyWindow(m_hwnd);
}

void CManagerDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();
	m_proto->m_managerDlg = NULL;
}

void CManagerDlg::OnAdd(CCtrlButton*)
{
	TCHAR temp[100];
	TCHAR mode[3];
	if (m_radio1.GetState()) {
		mir_tstrcpy(mode, _T("+b"));
		mir_tstrncpy(temp, TranslateT("Add ban"), 100);
	}
	if (m_radio2.GetState()) {
		mir_tstrcpy(mode, _T("+I"));
		mir_tstrncpy(temp, TranslateT("Add invite"), 100);
	}
	if (m_radio3.GetState()) {
		mir_tstrcpy(mode, _T("+e"));
		mir_tstrncpy(temp, TranslateT("Add exception"), 100);
	}

	m_add.Disable();
	m_edit.Disable();
	m_remove.Disable();

	CQuestionDlg* dlg = new CQuestionDlg(m_proto, this);
	dlg->Show();
	HWND addban_hWnd = dlg->GetHwnd();
	SetDlgItemText(addban_hWnd, IDC_CAPTION, temp);
	SetDlgItemText(addban_hWnd, IDC_TEXT, TranslateT("Please enter the hostmask (nick!user@host)"));

	TCHAR temp2[450];
	TCHAR window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));
	mir_sntprintf(temp2, SIZEOF(temp2), _T("/MODE %s %s %s"), window, mode, _T("%question"));
	SetDlgItemText(addban_hWnd, IDC_HIDDENEDIT, temp2);
	dlg->Activate();
}

void CManagerDlg::OnEdit(CCtrlButton*)
{
	if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_NOTOP)) {
		int i = m_list.GetCurSel();
		if (i != LB_ERR) {
			TCHAR* m = m_list.GetItemText(i);
			CMString user = GetWord(m, 0);
			mir_free(m);

			TCHAR temp[100];
			TCHAR mode[3];
			if (m_radio1.GetState()) {
				mir_tstrcpy(mode, _T("b"));
				mir_tstrncpy(temp, TranslateT("Edit ban"), 100);
			}
			if (m_radio2.GetState()) {
				mir_tstrcpy(mode, _T("I"));
				mir_tstrncpy(temp, TranslateT("Edit invite?"), 100);
			}
			if (m_radio3.GetState()) {
				mir_tstrcpy(mode, _T("e"));
				mir_tstrncpy(temp, TranslateT("Edit exception?"), 100);
			}

			CQuestionDlg* dlg = new CQuestionDlg(m_proto, this);
			dlg->Show();
			HWND addban_hWnd = dlg->GetHwnd();
			SetDlgItemText(addban_hWnd, IDC_CAPTION, temp);
			SetDlgItemText(addban_hWnd, IDC_TEXT, TranslateT("Please enter the hostmask (nick!user@host)"));
			SetDlgItemText(addban_hWnd, IDC_EDIT, user.c_str());

			m_add.Disable();
			m_edit.Disable();
			m_remove.Disable();

			TCHAR temp2[450];
			TCHAR window[256];
			GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));
			mir_sntprintf(temp2, SIZEOF(temp2), _T("/MODE %s -%s %s%s/MODE %s +%s %s"), window, mode, user.c_str(), _T("%newl"), window, mode, _T("%question"));
			SetDlgItemText(addban_hWnd, IDC_HIDDENEDIT, temp2);
			dlg->Activate();
		}
	}
}

void CManagerDlg::OnRemove(CCtrlButton*)
{
	int i = m_list.GetCurSel();
	if (i != LB_ERR) {
		m_add.Disable();
		m_edit.Disable();
		m_remove.Disable();

		TCHAR temp[100], mode[3];
		TCHAR* m = m_list.GetItemText(i, temp, SIZEOF(temp));
		CMString user = GetWord(m, 0);

		if (m_radio1.GetState()) {
			mir_tstrcpy(mode, _T("-b"));
			mir_tstrncpy(temp, TranslateT("Remove ban?"), 100);
		}
		if (m_radio2.GetState()) {
			mir_tstrcpy(mode, _T("-I"));
			mir_tstrncpy(temp, TranslateT("Remove invite?"), 100);
		}
		if (m_radio3.GetState()) {
			mir_tstrcpy(mode, _T("-e"));
			mir_tstrncpy(temp, TranslateT("Remove exception?"), 100);
		}

		TCHAR window[256];
		GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));
		if (MessageBox(m_hwnd, user.c_str(), temp, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
			m_proto->PostIrcMessage(_T("/MODE %s %s %s"), window, mode, user.c_str());
			ApplyQuestion();
		}
		CloseQuestion();
	}
}

void CManagerDlg::OnListDblClick(CCtrlListBox*)
{
	OnEdit(NULL);
}

void CManagerDlg::OnChangeList(CCtrlListBox*)
{
	if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_NOTOP)) {
		m_edit.Enable();
		m_remove.Enable();
	}
}

void CManagerDlg::OnChangeModes(CCtrlData*)
{
	m_applyModes.Enable();
}

void CManagerDlg::OnChangeTopic(CCtrlData*)
{
	m_applyTopic.Enable();
}

void CManagerDlg::OnApplyModes(CCtrlButton*)
{
	TCHAR window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));
	CHANNELINFO *wi = (CHANNELINFO *)m_proto->DoEvent(GC_EVENT_GETITEMDATA, window, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
	if (wi) {
		TCHAR toadd[10]; *toadd = '\0';
		TCHAR toremove[10]; *toremove = '\0';
		CMString appendixadd = _T("");
		CMString appendixremove = _T("");
		if (wi->pszMode && _tcschr(wi->pszMode, 't')) {
			if (!m_check1.GetState())
				mir_tstrcat(toremove, _T("t"));
		}
		else if (m_check1.GetState())
			mir_tstrcat(toadd, _T("t"));

		if (wi->pszMode && _tcschr(wi->pszMode, 'n')) {
			if (!m_check2.GetState())
				mir_tstrcat(toremove, _T("n"));
		}
		else if (m_check2.GetState())
			mir_tstrcat(toadd, _T("n"));

		if (wi->pszMode && _tcschr(wi->pszMode, 'i')) {
			if (!m_check3.GetState())
				mir_tstrcat(toremove, _T("i"));
		}
		else if (m_check3.GetState())
			mir_tstrcat(toadd, _T("i"));

		if (wi->pszMode && _tcschr(wi->pszMode, 'm')) {
			if (!m_check4.GetState())
				mir_tstrcat(toremove, _T("m"));
		}
		else if (m_check4.GetState())
			mir_tstrcat(toadd, _T("m"));

		if (wi->pszMode && _tcschr(wi->pszMode, 'p')) {
			if (!m_check7.GetState())
				mir_tstrcat(toremove, _T("p"));
		}
		else if (m_check7.GetState())
			mir_tstrcat(toadd, _T("p"));

		if (wi->pszMode && _tcschr(wi->pszMode, 's')) {
			if (!m_check8.GetState())
				mir_tstrcat(toremove, _T("s"));
		}
		else if (m_check8.GetState())
			mir_tstrcat(toadd, _T("s"));

		if (wi->pszMode && _tcschr(wi->pszMode, 'c')) {
			if (!m_check9.GetState())
				mir_tstrcat(toremove, _T("c"));
		}
		else if (m_check9.GetState())
			mir_tstrcat(toadd, _T("c"));

		CMString Key = _T("");
		CMString Limit = _T("");
		if (wi->pszMode && wi->pszPassword && _tcschr(wi->pszMode, 'k')) {
			if (!m_check5.GetState()) {
				mir_tstrcat(toremove, _T("k"));
				appendixremove += _T(" ") + CMString(wi->pszPassword);
			}
			else if (GetWindowTextLength(m_key.GetHwnd())) {
				TCHAR temp[400];
				m_key.GetText(temp, 14);

				if (Key != temp) {
					mir_tstrcat(toremove, _T("k"));
					mir_tstrcat(toadd, _T("k"));
					appendixadd += _T(" ") + CMString(temp);
					appendixremove += _T(" ") + CMString(wi->pszPassword);
				}
			}
		}
		else if (m_check5.GetState() && GetWindowTextLength(m_key.GetHwnd())) {
			mir_tstrcat(toadd, _T("k"));
			appendixadd += _T(" ");

			TCHAR temp[400];
			m_key.GetText(temp, SIZEOF(temp));
			appendixadd += temp;
		}

		if (_tcschr(wi->pszMode, 'l')) {
			if (!m_check6.GetState())
				mir_tstrcat(toremove, _T("l"));
			else if (GetWindowTextLength(GetDlgItem(m_hwnd, IDC_LIMIT))) {
				TCHAR temp[15];
				GetDlgItemText(m_hwnd, IDC_LIMIT, temp, SIZEOF(temp));
				if (wi->pszLimit && mir_tstrcmpi(wi->pszLimit, temp)) {
					mir_tstrcat(toadd, _T("l"));
					appendixadd += _T(" ") + CMString(temp);
				}
			}
		}
		else if (m_check6.GetState() && GetWindowTextLength(m_limit.GetHwnd())) {
			mir_tstrcat(toadd, _T("l"));
			appendixadd += _T(" ");

			TCHAR temp[15];
			m_limit.GetText(temp, SIZEOF(temp));
			appendixadd += temp;
		}

		if (mir_tstrlen(toadd) || mir_tstrlen(toremove)) {
			TCHAR temp[500];
			mir_tstrcpy(temp, _T("/mode "));
			mir_tstrcat(temp, window);
			mir_tstrcat(temp, _T(" "));
			if (mir_tstrlen(toremove))
				mir_sntprintf(temp, SIZEOF(temp), _T("%s-%s"), temp, toremove);
			if (mir_tstrlen(toadd))
				mir_sntprintf(temp, SIZEOF(temp), _T("%s+%s"), temp, toadd);
			if (!appendixremove.IsEmpty())
				mir_tstrcat(temp, appendixremove.c_str());
			if (!appendixadd.IsEmpty())
				mir_tstrcat(temp, appendixadd.c_str());
			m_proto->PostIrcMessage(temp);
		}
	}

	m_applyModes.Disable();
}

void CManagerDlg::OnApplyTopic(CCtrlButton*)
{
	TCHAR temp[470];
	TCHAR window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));
	m_topic.GetText(temp, SIZEOF(temp));
	m_proto->PostIrcMessage(_T("/TOPIC %s %s"), window, temp);
	int i = m_topic.SendMsg(CB_FINDSTRINGEXACT, -1, (LPARAM)temp);
	if (i != LB_ERR)
		m_topic.SendMsg(CB_DELETESTRING, i, 0);
	m_topic.SendMsg(CB_INSERTSTRING, 0, (LPARAM)temp);
	m_topic.SetText(temp);
	m_applyTopic.Disable();
}

void CManagerDlg::OnCheck(CCtrlData*)
{
	m_applyModes.Enable();
}

void CManagerDlg::OnCheck5(CCtrlData*)
{
	m_key.Enable(m_check5.GetState());
	m_applyModes.Enable();
}

void CManagerDlg::OnCheck6(CCtrlData*)
{
	m_limit.Enable(m_check6.GetState());
	m_applyModes.Enable();
}

void CManagerDlg::OnRadio(CCtrlData*)
{
	ApplyQuestion();
}

void CManagerDlg::ApplyQuestion()
{
	TCHAR window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, SIZEOF(window));

	TCHAR mode[3];
	mir_tstrcpy(mode, _T("+b"));
	if (m_radio2.GetState())
		mir_tstrcpy(mode, _T("+I"));
	if (m_radio3.GetState())
		mir_tstrcpy(mode, _T("+e"));
	m_list.ResetContent();
	m_radio1.Disable();
	m_radio2.Disable();
	m_radio3.Disable();
	m_add.Disable();
	m_edit.Disable();
	m_remove.Disable();
	m_proto->PostIrcMessage(_T("%s %s %s"), _T("/MODE"), window, mode); //wrong overloaded operator if three args
}

void CManagerDlg::CloseQuestion()
{
	m_add.Enable();
	if (m_list.GetCurSel() != LB_ERR) {
		m_edit.Enable();
		m_remove.Enable();
	}
}

void CManagerDlg::InitManager(int mode, const TCHAR* window)
{
	SetDlgItemText(m_hwnd, IDC_CAPTION, window);

	CHANNELINFO *wi = (CHANNELINFO *)m_proto->DoEvent(GC_EVENT_GETITEMDATA, window, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
	if (wi) {
		if (m_proto->IsConnected()) {
			TCHAR temp[1000];
			mir_sntprintf(temp, SIZEOF(temp), _T("Topic%s%s"), window, m_proto->m_info.sNetwork.c_str());

			char* p = mir_t2a(temp);

			DBVARIANT dbv;
			if (!m_proto->getTString(p, &dbv)) {
				for (int i = 0; i < 5; i++) {
					CMString S = GetWord(dbv.ptszVal, i);
					if (!S.IsEmpty()) {
						/* FIXME: What the hell does it mean!? GCC won't compile this on UNICODE */
#if !defined(__GNUC__) || !defined(UNICODE)
						S.Replace(_T("%¤"), _T(" "));
#endif
						m_topic.SendMsg(CB_ADDSTRING, 0, (LPARAM)S.c_str());
					}
				}
				db_free(&dbv);
			}
			mir_free(p);
		}

		if (wi->pszTopic)
			m_topic.SetText(wi->pszTopic);

		if (BST_UNCHECKED == IsDlgButtonChecked(m_proto->m_managerDlg->GetHwnd(), IDC_NOTOP))
			m_add.Enable();

		bool add = false;
		TCHAR* p1 = wi->pszMode;
		if (p1) {
			while (*p1 != '\0' && *p1 != ' ') {
				if (*p1 == '+')
					add = true;
				if (*p1 == '-')
					add = false;
				if (*p1 == 't')
					m_check1.SetState(add);
				if (*p1 == 'n')
					m_check2.SetState(add);
				if (*p1 == 'i')
					m_check3.SetState(add);
				if (*p1 == 'm')
					m_check4.SetState(add);
				if (*p1 == 'p')
					m_check7.SetState(add);
				if (*p1 == 's')
					m_check8.SetState(add);
				if (*p1 == 'c')
					m_check9.SetState(add);
				if (*p1 == 'k' && add) {
					m_check5.SetState(add);
					m_key.Enable(add);
					if (wi->pszPassword)
						m_key.SetText(wi->pszPassword);
				}
				if (*p1 == 'l' && add) {
					m_check6.SetState(add);
					m_limit.Enable(add);
					if (wi->pszLimit)
						m_limit.SetText(wi->pszLimit);
				}
				p1++;
				if (mode == 0) {
					m_limit.Disable();
					m_key.Disable();
					m_check1.Disable();
					m_check2.Disable();
					m_check3.Disable();
					m_check4.Disable();
					m_check5.Disable();
					m_check6.Disable();
					m_check7.Disable();
					m_check8.Disable();
					m_check9.Disable();
					m_add.Disable();
					if (m_check1.GetState())
						m_topic.Disable();
					CheckDlgButton(m_hwnd, IDC_NOTOP, BST_CHECKED);
				}
				ShowWindow(m_hwnd, SW_SHOW);
			}
		}
	}

	if (strchr(m_proto->sChannelModes.c_str(), 'b')) {
		m_radio1.SetState(true);
		m_proto->PostIrcMessage(_T("/MODE %s +b"), window);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'cool' dialog

CCoolIrcDlg::CCoolIrcDlg(CIrcProto* _pro, int dlgId)
	: CProtoDlgBase<CIrcProto>(_pro, dlgId, false)
{}

void CCoolIrcDlg::OnInitDialog()
{
	HFONT hFont = (HFONT)SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_GETFONT, 0, 0);

	LOGFONT lf;
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfHeight = (int)(lf.lfHeight*1.2);
	lf.lfWeight = FW_BOLD;
	hFont = CreateFontIndirect(&lf);
	SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_SETFONT, (WPARAM)hFont, 0);

	SendDlgItemMessage(m_hwnd, IDC_LOGO, STM_SETICON, (LPARAM)(HICON)LoadIconEx(IDI_LOGO), 0);
}

void CCoolIrcDlg::OnDestroy()
{
	HFONT hFont = (HFONT)SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_GETFONT, 0, 0);
	SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_SETFONT, SendDlgItemMessage(m_hwnd, IDOK, WM_GETFONT, 0, 0), 0);
	DeleteObject(hFont);

	ReleaseIconEx((HICON)SendDlgItemMessage(m_hwnd, IDC_LOGO, STM_SETICON, 0, 0));
	WindowFreeIcon(m_hwnd);
}

INT_PTR CCoolIrcDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORSTATIC) {
		switch (GetDlgCtrlID((HWND)lParam)) {
		case IDC_WHITERECT: case IDC_TEXT: case IDC_CAPTION: case IDC_AWAYTIME: case IDC_LOGO:
			SetTextColor((HDC)wParam, RGB(0, 0, 0));
			SetBkColor((HDC)wParam, RGB(255, 255, 255));
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}
