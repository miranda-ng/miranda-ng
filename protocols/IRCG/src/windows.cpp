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

CMessageBoxDlg::CMessageBoxDlg(CIrcProto *_pro, DCCINFO *_dci) :
	CIrcBaseDlg(_pro, IDD_MESSAGEBOX),
	pdci(_dci)
{
}

bool CMessageBoxDlg::OnApply()
{
	CDccSession *dcc = new CDccSession(m_proto, pdci);

	CDccSession *olddcc = m_proto->FindDCCSession(pdci->hContact);
	if (olddcc)
		olddcc->Disconnect();
	m_proto->AddDCCSession(pdci->hContact, dcc);

	dcc->Connect();
	return true;
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

bool CWhoisDlg::OnInitDialog()
{
	LOGFONT lf;
	HFONT hFont = (HFONT)m_AwayTime.SendMsg(WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	hFont = CreateFontIndirect(&lf);
	m_AwayTime.SendMsg(WM_SETFONT, (WPARAM)hFont, 0);

	CCoolIrcDlg::OnInitDialog();

	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_WHOIS));
	return true;
}

void CWhoisDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();

	HFONT hFont2 = (HFONT)SendDlgItemMessage(m_hwnd, IDC_AWAYTIME, WM_GETFONT, 0, 0);
	SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_SETFONT, SendDlgItemMessage(m_hwnd, IDOK, WM_GETFONT, 0, 0), 0);
	DeleteObject(hFont2);

	m_proto->m_whoisDlg = nullptr;
}

void CWhoisDlg::OnGo(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_InfoNick.GetText(szTemp, _countof(szTemp));
	m_proto->PostIrcMessage(L"/WHOIS %s %s", szTemp, szTemp);
}

void CWhoisDlg::OnQuery(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_InfoNick.GetText(szTemp, _countof(szTemp));
	m_proto->PostIrcMessage(L"/QUERY %s", szTemp);
}

void CWhoisDlg::OnPing(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_InfoNick.GetText(szTemp, _countof(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(L"/PRIVMSG %s \001PING %u\001", szTemp, time(0));
}

void CWhoisDlg::OnUserInfo(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_InfoNick.GetText(szTemp, _countof(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(L"/PRIVMSG %s \001USERINFO\001", szTemp);
}

void CWhoisDlg::OnTime(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_InfoNick.GetText(szTemp, _countof(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(L"/PRIVMSG %s \001TIME\001", szTemp);
}

void CWhoisDlg::OnVersion(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_InfoNick.GetText(szTemp, _countof(szTemp));
	m_Reply.SetText(TranslateT("Please wait..."));
	m_proto->PostIrcMessage(L"/PRIVMSG %s \001VERSION\001", szTemp);
}

void CWhoisDlg::ShowMessage(const CIrcMessage* pmsg)
{
	if (m_InfoNick.SendMsg(CB_FINDSTRINGEXACT, -1, (LPARAM)pmsg->parameters[1].c_str()) == CB_ERR)
		m_InfoNick.SendMsg(CB_ADDSTRING, 0, (LPARAM)pmsg->parameters[1].c_str());
	int i = m_InfoNick.SendMsg(CB_FINDSTRINGEXACT, -1, (LPARAM)pmsg->parameters[1].c_str());
	m_InfoNick.SendMsg(CB_SETCURSEL, i, 0);
	m_Caption.SetText(pmsg->parameters[1]);
	m_InfoName.SetText(pmsg->parameters[5]);
	m_InfoAddress.SetText(pmsg->parameters[3]);
	m_InfoId.SetText(pmsg->parameters[2]);
	m_InfoChannels.SetText(L"");
	m_InfoServer.SetText(L"");
	m_InfoAway2.SetText(L"");
	m_InfoAuth.SetText(L"");
	m_InfoOther.SetText(L"");
	m_Reply.SetText(L"");
	SetWindowText(m_hwnd, TranslateT("User information"));
	EnableWindow(GetDlgItem(m_hwnd, ID_INFO_QUERY), true);
	ShowWindow(m_hwnd, SW_SHOW);
	if (IsIconic(m_hwnd))
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
	SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(m_hwnd, nullptr, TRUE);
}

void CWhoisDlg::ShowMessageNoUser(const CIrcMessage *pmsg)
{
	m_InfoNick.SetText(pmsg->parameters[2]);
	m_InfoNick.SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
	m_Caption.SetText(pmsg->parameters[2]);
	m_InfoName.SetText(L"");
	m_InfoAddress.SetText(L"");
	m_InfoId.SetText(L"");
	m_InfoChannels.SetText(L"");
	m_InfoServer.SetText(L"");
	m_InfoAway2.SetText(L"");
	m_InfoAuth.SetText(L"");
	m_Reply.SetText(L"");
	EnableWindow(GetDlgItem(m_hwnd, ID_INFO_QUERY), false);
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Change nickname' dialog

CNickDlg::CNickDlg(CIrcProto *_pro) :
	CCoolIrcDlg(_pro, IDD_NICK),
	m_Enick(this, IDC_ENICK)
{
}

bool CNickDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();
	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_RENAME));

	DBVARIANT dbv;
	if (!m_proto->getWString("RecentNicks", &dbv)) {
		for (int i = 0; i < 10; i++)
			if (!GetWord(dbv.pwszVal, i).IsEmpty())
				SendDlgItemMessage(m_hwnd, IDC_ENICK, CB_ADDSTRING, 0, (LPARAM)GetWord(dbv.pwszVal, i).c_str());

		db_free(&dbv);
	}
	return true;
}

void CNickDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();
	m_proto->m_nickDlg = nullptr;
}

bool CNickDlg::OnApply()
{
	wchar_t szTemp[255];
	m_Enick.GetText(szTemp, _countof(szTemp));
	m_proto->PostIrcMessage(L"/NICK %s", szTemp);

	CMStringW S = szTemp;
	DBVARIANT dbv;
	if (!m_proto->getWString("RecentNicks", &dbv)) {
		for (int i = 0; i < 10; i++) {
			CMStringW s = GetWord(dbv.pwszVal, i);
			if (!s.IsEmpty() && s != szTemp)
				S += L" " + s;
		}
		db_free(&dbv);
	}
	m_proto->setWString("RecentNicks", S);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Change nickname' dialog

#define LIST_TIMER 10

CListDlg::CListDlg(CIrcProto *_pro) :
	CIrcBaseDlg(_pro, IDD_LIST),
	m_Join(this, IDC_JOIN),
	m_list(this, IDC_INFO_LISTVIEW),
	m_list2(this, IDC_INFO_LISTVIEW2),
	m_status(this, IDC_TEXT),
	m_filter(this, IDC_FILTER_STRING)
{
	m_list.OnColumnClick = Callback(this, &CListDlg::onColumnClick_List);
	m_list2.OnDoubleClick = m_list.OnDoubleClick = m_Join.OnClick = Callback(this, &CListDlg::onClick_Join);
	m_filter.OnChange = Callback(this, &CListDlg::onChange_Filter);
}

bool CListDlg::OnInitDialog()
{
	RECT screen;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
	LVCOLUMN lvC;
	int COLUMNS_SIZES[4] = { 200, 50, 50, 2000 };
	wchar_t szBuffer[32];

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	for (int index = 0; index < 4; index++) {
		lvC.iSubItem = index;
		lvC.cx = COLUMNS_SIZES[index];

		switch (index) {
			case 0: mir_wstrcpy(szBuffer, TranslateT("Channel")); break;
			case 1: mir_wstrcpy(szBuffer, L"#");               break;
			case 2: mir_wstrcpy(szBuffer, TranslateT("Mode"));    break;
			case 3: mir_wstrcpy(szBuffer, TranslateT("Topic"));   break;
		}
		lvC.pszText = szBuffer;
		m_list.InsertColumn(index, &lvC);
		m_list2.InsertColumn(index, &lvC);
	}

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "channelList_");

	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	m_list2.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_LIST));
	m_status.SetText(TranslateT("Please wait..."));
	return true;
}

INT_PTR CListDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_TIMER) {
		::KillTimer(m_hwnd, m_timer); m_timer = 0;

		// Retrieve the input text
		wchar_t strFilterText[255];
		wchar_t newTitle[255];
		m_filter.GetText(strFilterText, _countof(strFilterText));

		if (strFilterText[0]) {
			int itemCount = 0;
			int j = m_list.GetItemCount();
			if (j <= 0)
				return FALSE;

			// Empty the filtered list
			m_list2.DeleteAllItems();

			LVITEM lvm;
			wchar_t text[255];
			lvm.pszText = text;	// Set buffer for texts
			lvm.cchTextMax = _countof(text);
			lvm.mask = LVIF_TEXT;
			for (int i = 0; i < j; i++) {
				lvm.iSubItem = 0;	// First column
				lvm.iItem = i;
				m_list.GetItem(&lvm);

				// Match the text?
				wchar_t* t = wcsstr(lvm.pszText, strFilterText);
				if (t == nullptr) { // If no, then Check if in the topics
					m_list.GetItem(&lvm);

					// Match the text?
					t = wcsstr(lvm.pszText, strFilterText);
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
			mir_snwprintf(newTitle, TranslateT("%s - Filtered - %d items"), strFilterText, itemCount);
			SetWindowText(m_hwnd, newTitle);
		}
		else {
			ShowWindow(m_list.GetHwnd(), SW_SHOW);
			ShowWindow(m_list2.GetHwnd(), SW_HIDE);
			mir_snwprintf(newTitle, TranslateT("Channels on server"));
			SetWindowText(m_hwnd, newTitle);
		}
	}

	return CProtoDlgBase<CIrcProto>::DlgProc(msg, wParam, lParam);
}

void CListDlg::onChange_Filter(CCtrlEdit*)
{
	m_timer = ::SetTimer(m_hwnd, LIST_TIMER, 200, nullptr);
}

void CListDlg::OnDestroy()
{
	if (m_timer)
		::KillTimer(m_hwnd, m_timer);
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "channelList_");

	mir_cslock lck(m_proto->m_csList);
	m_proto->m_listDlg = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

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

	wchar_t temp1[512];
	wchar_t temp2[512];
	LVITEM lvm;
	lvm.mask = LVIF_TEXT;
	lvm.iItem = lParam1;
	lvm.iSubItem = param->iSubItem;
	lvm.pszText = temp1;
	lvm.cchTextMax = _countof(temp1);
	param->pList->GetItem(&lvm);
	lvm.iItem = lParam2;
	lvm.pszText = temp2;
	param->pList->GetItem(&lvm);
	if (param->iSubItem != 1) {
		if (mir_wstrlen(temp1) != 0 && mir_wstrlen(temp2) != 0)
			return mir_wstrcmpi(temp1, temp2);

		return (*temp1 == 0) ? 1 : -1;
	}

	return (_wtoi(temp1) < _wtoi(temp2)) ? 1 : -1;
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

void CListDlg::onColumnClick_List(CCtrlListView::TEventInfo *ev)
{
	ListViewSortParam param = { &m_list, ev->nmlv->iSubItem };
	m_list.SortItems(ListViewSort, (LPARAM)&param);
	UpdateList();
}

void CListDlg::onClick_Join(CCtrlButton*)
{
	wchar_t szTemp[255];
	m_filter.GetText(szTemp, _countof(szTemp));

	if (szTemp[0])
		m_list2.GetItemText(m_list2.GetSelectionMark(), 0, szTemp, 255);
	else
		m_list.GetItemText(m_list.GetSelectionMark(), 0, szTemp, 255);
	m_proto->PostIrcMessage(L"/JOIN %s", szTemp);
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

CJoinDlg::CJoinDlg(CIrcProto *_pro) :
	CCoolIrcDlg(_pro, IDD_NICK)
{
}

bool CJoinDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	DBVARIANT dbv;
	if (!m_proto->getWString("RecentChannels", &dbv)) {
		for (int i = 0; i < 20; i++) {
			if (!GetWord(dbv.pwszVal, i).IsEmpty()) {
				CMStringW S = GetWord(dbv.pwszVal, i);
				S.Replace(L"%newl", L" ");
				SendDlgItemMessage(m_hwnd, IDC_ENICK, CB_ADDSTRING, 0, (LPARAM)S.c_str());
			}
		}
		db_free(&dbv);
	}
	return true;
}

void CJoinDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();
	m_proto->m_joinDlg = nullptr;
}

bool CJoinDlg::OnApply()
{
	wchar_t szTemp[255];
	GetDlgItemText(m_hwnd, IDC_ENICK, szTemp, _countof(szTemp));
	if (m_proto->IsChannel(szTemp))
		m_proto->PostIrcMessage(L"/JOIN %s", szTemp);
	else
		m_proto->PostIrcMessage(L"/JOIN #%s", szTemp);

	CMStringW S = szTemp;
	S.Replace(L" ", L"%newl");
	CMStringW SL = S;

	DBVARIANT dbv;
	if (!m_proto->getWString("RecentChannels", &dbv)) {
		for (int i = 0; i < 20; i++) {
			CMStringW W = GetWord(dbv.pwszVal, i);
			if (!W.IsEmpty() && W != SL)
				S += L" " + W;
		}
		db_free(&dbv);
	}
	m_proto->setWString("RecentChannels", S);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'Question' dialog

CQuestionDlg::CQuestionDlg(CIrcProto *_pro, CManagerDlg *owner) :
	CCoolIrcDlg(_pro, IDD_QUESTION),
	m_owner(owner)
{
	if (owner != nullptr)
		m_hwndParent = owner->GetHwnd();
}

bool CQuestionDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_IRCQUESTION));
	return true;
}

bool CQuestionDlg::OnApply()
{
	int i = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_EDIT));
	if (i > 0) {
		wchar_t* l = new wchar_t[i + 2];
		GetDlgItemText(m_hwnd, IDC_EDIT, l, i + 1);

		int j = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_HIDDENEDIT));
		wchar_t *m = new wchar_t[j + 2];
		GetDlgItemText(m_hwnd, IDC_HIDDENEDIT, m, j + 1);

		wchar_t *text = wcsstr(m, L"%question");
		wchar_t *p1 = text;
		wchar_t *p2 = nullptr;
		if (p1) {
			p1 += 9;
			if (*p1 == '=' && p1[1] == '\"') {
				p1 += 2;
				for (int k = 0; k < 3; k++) {
					p2 = wcschr(p1, '\"');
					if (p2) {
						p2++;
						if (k == 2 || (*p2 != ',' || (*p2 == ',' && p2[1] != '\"')))
							*p2 = 0;
						else
							p2 += 2;
						p1 = p2;
					}
				}
			}
			else *p1 = 0;
		}

		CMStringW S('\0', j + 2);
		GetDlgItemText(m_hwnd, IDC_HIDDENEDIT, S.GetBuffer(), j + 1);
		S.Replace(text, l);
		m_proto->PostIrcMessageWnd(nullptr, NULL, S);

		delete[]m;
		delete[]l;

		if (m_owner)
			m_owner->ApplyQuestion();
	}
	return true;
}

void CQuestionDlg::OnDestroy()
{
	if (m_owner)
		m_owner->CloseQuestion();
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

	m_add(this, IDC_ADD, g_plugin.getIcon(IDI_ADD), LPGEN("Add ban/invite/exception")),
	m_edit(this, IDC_EDIT, g_plugin.getIcon(IDI_EDIT), LPGEN("Edit selected ban/invite/exception")),
	m_remove(this, IDC_REMOVE, g_plugin.getIcon(IDI_DELETE), LPGEN("Delete selected ban/invite/exception")),
	m_applyModes(this, IDC_APPLYMODES, g_plugin.getIcon(IDI_APPLY), LPGEN("Set these modes for the channel")),
	m_applyTopic(this, IDC_APPLYTOPIC, g_plugin.getIcon(IDI_APPLY), LPGEN("Set this topic for the channel")),

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
				w[1] = 0;
			}
			if (wParam == 2) {
				w[0] = 2;
				w[1] = 0;
			}
			if (wParam == 21) {
				w[0] = 31;
				w[1] = 0;
			}
			SendMessage(m_hwnd, EM_REPLACESEL, false, (LPARAM)w);
			SendMessage(m_hwnd, EM_SCROLLCARET, 0, 0);
			return 0;
		}
		break;
	}

	return mir_callNextSubclass(m_hwnd, MgrEditSubclassProc, msg, wParam, lParam);
}

bool CManagerDlg::OnInitDialog()
{
	CCoolIrcDlg::OnInitDialog();

	POINT pt;
	pt.x = 3;
	pt.y = 3;
	HWND hwndEdit = ChildWindowFromPoint(m_topic.GetHwnd(), pt);
	mir_subclassWindow(hwndEdit, MgrEditSubclassProc);

	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_MANAGER));

	m_list.SendMsg(LB_SETHORIZONTALEXTENT, 750, NULL);
	m_radio1.SetState(true);

	const char* modes = m_proto->sChannelModes;
	if (!strchr(modes, 't')) m_check1.Disable();
	if (!strchr(modes, 'n')) m_check2.Disable();
	if (!strchr(modes, 'i')) m_check3.Disable();
	if (!strchr(modes, 'm')) m_check4.Disable();
	if (!strchr(modes, 'k')) m_check5.Disable();
	if (!strchr(modes, 'l')) m_check6.Disable();
	if (!strchr(modes, 'p')) m_check7.Disable();
	if (!strchr(modes, 's')) m_check8.Disable();
	if (!strchr(modes, 'c')) m_check9.Disable();
	return true;
}

bool CManagerDlg::OnClose()
{
	if (m_applyModes.Enabled() || m_applyTopic.Enabled()) {
		int i = MessageBox(nullptr, TranslateT("You have not applied all changes!\n\nApply before exiting?"), TranslateT("IRC warning"), MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON3);
		if (i == IDCANCEL)
			return false;

		if (i == IDYES) {
			if (m_applyModes.Enabled())
				OnApplyModes(nullptr);
			if (m_applyTopic.Enabled())
				OnApplyTopic(nullptr);
		}
	}

	wchar_t window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));
	CMStringW S = L"";
	wchar_t temp[1000];
	for (int i = 0; i < 5; i++) {
		if (m_topic.SendMsg(CB_GETLBTEXT, i, (LPARAM)temp) != LB_ERR) {
			CMStringW S1 = temp;
			/* FIXME: What the hell does it mean!? GCC won't compile this on UNICODE */
#if !defined(__GNUC__) || !defined(UNICODE)
			S1.Replace(L" ", L"%¤");
#endif
			S += L" " + S1;
		}
	}

	if (!S.IsEmpty() && m_proto->IsConnected()) {
		mir_snwprintf(temp, L"Topic%s", window);
		char* p = mir_u2a(temp);
		m_proto->setWString(p, S);
		mir_free(p);
	}

	return true;
}

void CManagerDlg::OnDestroy()
{
	CCoolIrcDlg::OnDestroy();
	m_proto->m_managerDlg = nullptr;
}

void CManagerDlg::OnAdd(CCtrlButton*)
{
	wchar_t temp[100];
	wchar_t mode[3];
	if (m_radio1.GetState()) {
		mir_wstrcpy(mode, L"+b");
		mir_wstrncpy(temp, TranslateT("Add ban"), 100);
	}
	if (m_radio2.GetState()) {
		mir_wstrcpy(mode, L"+I");
		mir_wstrncpy(temp, TranslateT("Add invite"), 100);
	}
	if (m_radio3.GetState()) {
		mir_wstrcpy(mode, L"+e");
		mir_wstrncpy(temp, TranslateT("Add exception"), 100);
	}

	m_add.Disable();
	m_edit.Disable();
	m_remove.Disable();

	CQuestionDlg* dlg = new CQuestionDlg(m_proto, this);
	dlg->Show();
	HWND addban_hWnd = dlg->GetHwnd();
	SetDlgItemText(addban_hWnd, IDC_CAPTION, temp);
	SetDlgItemText(addban_hWnd, IDC_TEXT, TranslateT("Please enter the hostmask (nick!user@host)"));

	wchar_t temp2[450];
	wchar_t window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));
	mir_snwprintf(temp2, L"/MODE %s %s %s", window, mode, L"%question");
	SetDlgItemText(addban_hWnd, IDC_HIDDENEDIT, temp2);
	dlg->Activate();
}

void CManagerDlg::OnEdit(CCtrlButton*)
{
	if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_NOTOP)) {
		int i = m_list.GetCurSel();
		if (i != LB_ERR) {
			wchar_t* m = m_list.GetItemText(i);
			CMStringW user = GetWord(m, 0);
			mir_free(m);

			wchar_t temp[100];
			wchar_t mode[3];
			if (m_radio1.GetState()) {
				mir_wstrcpy(mode, L"b");
				mir_wstrncpy(temp, TranslateT("Edit ban"), 100);
			}
			if (m_radio2.GetState()) {
				mir_wstrcpy(mode, L"I");
				mir_wstrncpy(temp, TranslateT("Edit invite?"), 100);
			}
			if (m_radio3.GetState()) {
				mir_wstrcpy(mode, L"e");
				mir_wstrncpy(temp, TranslateT("Edit exception?"), 100);
			}

			CQuestionDlg* dlg = new CQuestionDlg(m_proto, this);
			dlg->Show();
			HWND addban_hWnd = dlg->GetHwnd();
			SetDlgItemText(addban_hWnd, IDC_CAPTION, temp);
			SetDlgItemText(addban_hWnd, IDC_TEXT, TranslateT("Please enter the hostmask (nick!user@host)"));
			SetDlgItemText(addban_hWnd, IDC_EDIT, user);

			m_add.Disable();
			m_edit.Disable();
			m_remove.Disable();

			wchar_t temp2[450];
			wchar_t window[256];
			GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));
			mir_snwprintf(temp2, L"/MODE %s -%s %s%s/MODE %s +%s %s", window, mode, user.c_str(), L"%newl", window, mode, L"%question");
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

		wchar_t temp[100], mode[3];
		wchar_t* m = m_list.GetItemText(i, temp, _countof(temp));
		CMStringW user = GetWord(m, 0);

		if (m_radio1.GetState()) {
			mir_wstrcpy(mode, L"-b");
			mir_wstrncpy(temp, TranslateT("Remove ban?"), 100);
		}
		if (m_radio2.GetState()) {
			mir_wstrcpy(mode, L"-I");
			mir_wstrncpy(temp, TranslateT("Remove invite?"), 100);
		}
		if (m_radio3.GetState()) {
			mir_wstrcpy(mode, L"-e");
			mir_wstrncpy(temp, TranslateT("Remove exception?"), 100);
		}

		wchar_t window[256];
		GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));
		if (MessageBox(m_hwnd, user, temp, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
			m_proto->PostIrcMessage(L"/MODE %s %s %s", window, mode, user.c_str());
			ApplyQuestion();
		}
		CloseQuestion();
	}
}

void CManagerDlg::OnListDblClick(CCtrlListBox*)
{
	OnEdit(nullptr);
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
	wchar_t window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));
	auto *wi = m_proto->GetChannelInfo(window);
	if (wi) {
		wchar_t toadd[10]; *toadd = 0;
		wchar_t toremove[10]; *toremove = 0;
		CMStringW appendixadd = L"";
		CMStringW appendixremove = L"";
		if (wi->pszMode && wcschr(wi->pszMode, 't')) {
			if (!m_check1.GetState())
				mir_wstrcat(toremove, L"t");
		}
		else if (m_check1.GetState())
			mir_wstrcat(toadd, L"t");

		if (wi->pszMode && wcschr(wi->pszMode, 'n')) {
			if (!m_check2.GetState())
				mir_wstrcat(toremove, L"n");
		}
		else if (m_check2.GetState())
			mir_wstrcat(toadd, L"n");

		if (wi->pszMode && wcschr(wi->pszMode, 'i')) {
			if (!m_check3.GetState())
				mir_wstrcat(toremove, L"i");
		}
		else if (m_check3.GetState())
			mir_wstrcat(toadd, L"i");

		if (wi->pszMode && wcschr(wi->pszMode, 'm')) {
			if (!m_check4.GetState())
				mir_wstrcat(toremove, L"m");
		}
		else if (m_check4.GetState())
			mir_wstrcat(toadd, L"m");

		if (wi->pszMode && wcschr(wi->pszMode, 'p')) {
			if (!m_check7.GetState())
				mir_wstrcat(toremove, L"p");
		}
		else if (m_check7.GetState())
			mir_wstrcat(toadd, L"p");

		if (wi->pszMode && wcschr(wi->pszMode, 's')) {
			if (!m_check8.GetState())
				mir_wstrcat(toremove, L"s");
		}
		else if (m_check8.GetState())
			mir_wstrcat(toadd, L"s");

		if (wi->pszMode && wcschr(wi->pszMode, 'c')) {
			if (!m_check9.GetState())
				mir_wstrcat(toremove, L"c");
		}
		else if (m_check9.GetState())
			mir_wstrcat(toadd, L"c");

		CMStringW Key = L"";
		CMStringW Limit = L"";
		if (wi->pszMode && wi->pszPassword && wcschr(wi->pszMode, 'k')) {
			if (!m_check5.GetState()) {
				mir_wstrcat(toremove, L"k");
				appendixremove += L" " + CMStringW(wi->pszPassword);
			}
			else if (GetWindowTextLength(m_key.GetHwnd())) {
				wchar_t temp[400];
				m_key.GetText(temp, 14);

				if (Key != temp) {
					mir_wstrcat(toremove, L"k");
					mir_wstrcat(toadd, L"k");
					appendixadd += L" " + CMStringW(temp);
					appendixremove += L" " + CMStringW(wi->pszPassword);
				}
			}
		}
		else if (m_check5.GetState() && GetWindowTextLength(m_key.GetHwnd())) {
			mir_wstrcat(toadd, L"k");
			appendixadd += L" ";

			wchar_t temp[400];
			m_key.GetText(temp, _countof(temp));
			appendixadd += temp;
		}

		if (wcschr(wi->pszMode, 'l')) {
			if (!m_check6.GetState())
				mir_wstrcat(toremove, L"l");
			else if (GetWindowTextLength(GetDlgItem(m_hwnd, IDC_LIMIT))) {
				wchar_t temp[15];
				GetDlgItemText(m_hwnd, IDC_LIMIT, temp, _countof(temp));
				if (wi->pszLimit && mir_wstrcmpi(wi->pszLimit, temp)) {
					mir_wstrcat(toadd, L"l");
					appendixadd += L" " + CMStringW(temp);
				}
			}
		}
		else if (m_check6.GetState() && GetWindowTextLength(m_limit.GetHwnd())) {
			mir_wstrcat(toadd, L"l");
			appendixadd += L" ";

			wchar_t temp[15];
			m_limit.GetText(temp, _countof(temp));
			appendixadd += temp;
		}

		if (mir_wstrlen(toadd) || mir_wstrlen(toremove)) {
			wchar_t temp[500];
			mir_wstrcpy(temp, L"/mode ");
			mir_wstrcat(temp, window);
			mir_wstrcat(temp, L" ");
			if (mir_wstrlen(toremove))
				mir_snwprintf(temp, L"%s-%s", temp, toremove);
			if (mir_wstrlen(toadd))
				mir_snwprintf(temp, L"%s+%s", temp, toadd);
			if (!appendixremove.IsEmpty())
				mir_wstrcat(temp, appendixremove);
			if (!appendixadd.IsEmpty())
				mir_wstrcat(temp, appendixadd);
			m_proto->PostIrcMessage(temp);
		}
	}

	m_applyModes.Disable();
}

void CManagerDlg::OnApplyTopic(CCtrlButton*)
{
	wchar_t temp[470];
	wchar_t window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));
	m_topic.GetText(temp, _countof(temp));
	m_proto->PostIrcMessage(L"/TOPIC %s %s", window, temp);
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
	wchar_t window[256];
	GetDlgItemText(m_hwnd, IDC_CAPTION, window, _countof(window));

	wchar_t mode[3];
	mir_wstrcpy(mode, L"+b");
	if (m_radio2.GetState())
		mir_wstrcpy(mode, L"+I");
	if (m_radio3.GetState())
		mir_wstrcpy(mode, L"+e");
	m_list.ResetContent();
	m_radio1.Disable();
	m_radio2.Disable();
	m_radio3.Disable();
	m_add.Disable();
	m_edit.Disable();
	m_remove.Disable();
	m_proto->PostIrcMessage(L"%s %s %s", L"/MODE", window, mode); //wrong overloaded operator if three args
}

void CManagerDlg::CloseQuestion()
{
	m_add.Enable();
	if (m_list.GetCurSel() != LB_ERR) {
		m_edit.Enable();
		m_remove.Enable();
	}
}

void CManagerDlg::InitManager(int mode, const wchar_t* window)
{
	SetDlgItemText(m_hwnd, IDC_CAPTION, window);

	auto *wi = m_proto->GetChannelInfo(window);
	if (wi) {
		if (m_proto->IsConnected()) {
			wchar_t temp[1000];
			mir_snwprintf(temp, L"Topic%s", window);

			char* p = mir_u2a(temp);

			DBVARIANT dbv;
			if (!m_proto->getWString(p, &dbv)) {
				for (int i = 0; i < 5; i++) {
					CMStringW S = GetWord(dbv.pwszVal, i);
					if (!S.IsEmpty()) {
						/* FIXME: What the hell does it mean!? GCC won't compile this on UNICODE */
#if !defined(__GNUC__) || !defined(UNICODE)
						S.Replace(L"%¤", L" ");
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
		wchar_t* p1 = wi->pszMode;
		if (p1) {
			while (*p1 != 0 && *p1 != ' ') {
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

	if (strchr(m_proto->sChannelModes, 'b')) {
		m_radio1.SetState(true);
		m_proto->PostIrcMessage(L"/MODE %s +b", window);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// 'cool' dialog

CCoolIrcDlg::CCoolIrcDlg(CIrcProto* _pro, int dlgId) :
	CIrcBaseDlg(_pro, dlgId)
{}

bool CCoolIrcDlg::OnInitDialog()
{
	HFONT hFont = (HFONT)SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_GETFONT, 0, 0);

	LOGFONT lf;
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfHeight = (int)(lf.lfHeight*1.2);
	lf.lfWeight = FW_BOLD;
	hFont = CreateFontIndirect(&lf);
	SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_SETFONT, (WPARAM)hFont, 0);

	SendDlgItemMessage(m_hwnd, IDC_LOGO, STM_SETICON, (LPARAM)(HICON)g_plugin.getIcon(IDI_LOGO), 0);
	return true;
}

void CCoolIrcDlg::OnDestroy()
{
	HFONT hFont = (HFONT)SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_GETFONT, 0, 0);
	SendDlgItemMessage(m_hwnd, IDC_CAPTION, WM_SETFONT, SendDlgItemMessage(m_hwnd, IDOK, WM_GETFONT, 0, 0), 0);
	DeleteObject(hFont);

	IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_LOGO, STM_SETICON, 0, 0));
	Window_FreeIcon_IcoLib(m_hwnd);
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
