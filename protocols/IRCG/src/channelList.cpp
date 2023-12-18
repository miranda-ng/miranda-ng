/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

struct ListViewSortParam
{
	CCtrlListView *pList;
	int iSubItem, iSortOrder;
};

static int CALLBACK ListViewSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ListViewSortParam *param = (ListViewSortParam *)lParamSort;
	if (!param->pList->GetHwnd())
		return 0;

	LVITEM lvm;
	lvm.mask = LVIF_TEXT;
	lvm.iSubItem = param->iSubItem;

	wchar_t temp1[512];
	lvm.iItem = lParam1;
	lvm.pszText = temp1;
	lvm.cchTextMax = _countof(temp1);
	param->pList->GetItem(&lvm);
	
	wchar_t temp2[512];
	lvm.iItem = lParam2;
	lvm.pszText = temp2;
	param->pList->GetItem(&lvm);
	
	if (param->iSubItem != 1)
		return mir_wstrcmpi(temp1, temp2) * param->iSortOrder;

	int res = (_wtoi(temp1) < _wtoi(temp2)) ? 1 : -1;
	return res * param->iSortOrder;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CListDlg : public CIrcBaseDlg
{
	int iPrevCol = -1, iPrevSort = 1;
	CTimer m_timer;
	CCtrlButton m_Join;
	CCtrlEdit m_filter, m_status;

public:
	CCtrlListView m_list, m_list2;

	CListDlg(CIrcProto *_pro) :
		CIrcBaseDlg(_pro, IDD_LIST),
		m_Join(this, IDC_JOIN),
		m_list(this, IDC_INFO_LISTVIEW),
		m_list2(this, IDC_INFO_LISTVIEW2),
		m_timer(this, 10),
		m_status(this, IDC_TEXT),
		m_filter(this, IDC_FILTER_STRING)
	{
		m_list.OnColumnClick = Callback(this, &CListDlg::onColumnClick_List);
		m_list2.OnDoubleClick = m_list.OnDoubleClick = m_Join.OnClick = Callback(this, &CListDlg::onClick_Join);
		m_timer.OnEvent = Callback(this, &CListDlg::onTimer);
		m_filter.OnChange = Callback(this, &CListDlg::onChange_Filter);
	}

	bool OnInitDialog() override
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

	void OnDestroy() override
	{
		m_timer.Stop();
		Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "channelList_");

		mir_cslock lck(m_proto->m_csList);
		m_proto->m_listDlg = nullptr;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
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

	void onTimer(CTimer *pTimer)
	{
		pTimer->Stop();

		// Retrieve the input text
		wchar_t strFilterText[255];
		wchar_t newTitle[255];
		m_filter.GetText(strFilterText, _countof(strFilterText));

		if (strFilterText[0]) {
			int itemCount = 0;
			int j = m_list.GetItemCount();
			if (j <= 0)
				return;

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
				wchar_t *t = wcsstr(lvm.pszText, strFilterText);
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

	void onClick_Join(CCtrlButton *)
	{
		wchar_t szTemp[255];
		m_filter.GetText(szTemp, _countof(szTemp));

		if (szTemp[0])
			m_list2.GetItemText(m_list2.GetSelectionMark(), 0, szTemp, 255);
		else
			m_list.GetItemText(m_list.GetSelectionMark(), 0, szTemp, 255);
		m_proto->PostIrcMessage(L"/JOIN %s", szTemp);
	}

	void onChange_Filter(CCtrlEdit *)
	{
		m_timer.Start(200);
	}

	void onColumnClick_List(CCtrlListView::TEventInfo *ev)
	{
		ListViewSortParam param = { &m_list, ev->nmlv->iSubItem };
		if (param.iSubItem != iPrevCol) {
			param.iSortOrder = iPrevSort = 1;
			iPrevCol = param.iSubItem;
		}
		else param.iSortOrder = iPrevSort = iPrevSort * -1;
		
		m_list.SortItems(ListViewSort, (LPARAM)&param);
		UpdateList();
	}

	void AddChannel(const CIrcMessage *pmsg)
	{
		LVITEM lvItem;
		lvItem.iItem = m_list.GetItemCount();
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem = 0;
		lvItem.pszText = pmsg->parameters[1].GetBuffer();
		lvItem.lParam = lvItem.iItem;
		lvItem.iItem = m_list.InsertItem(&lvItem);

		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.pszText = pmsg->parameters[pmsg->parameters.getCount() - 2].GetBuffer();
		m_list.SetItem(&lvItem);

		auto &wszTopic = pmsg->parameters[pmsg->parameters.getCount() - 1];
		int iStart = wszTopic.Find(L"[+");
		int iEnd = wszTopic.Find(']') + 1;
		if (iStart == 0 && iEnd > 3) {
			CMStringW wszMode(wszTopic.Mid(0, iEnd));
			lvItem.iSubItem = 2;
			lvItem.pszText = wszMode.GetBuffer();
			m_list.SetItem(&lvItem);

			wszTopic.Delete(0, iEnd);
		}

		lvItem.iSubItem = 3;
		lvItem.pszText = DoColorCodes(wszTopic, TRUE, FALSE);
		m_list.SetItem(&lvItem);

		int percent = 100;
		if (m_proto->m_noOfChannels > 0)
			percent = (int)(m_proto->m_channelNumber * 100) / m_proto->m_noOfChannels;

		wchar_t text[100];
		if (percent < 100)
			mir_snwprintf(text, TranslateT("Downloading list (%u%%) - %u channels"), percent, m_proto->m_channelNumber);
		else
			mir_snwprintf(text, TranslateT("Downloading list - %u channels"), m_proto->m_channelNumber);
		m_status.SetText(text);
	}

	void UpdateList(void)
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
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

static INT_PTR __stdcall sttShowDlgList(void *param)
{
	CIrcProto *ppro = (CIrcProto *)param;

	mir_cslock lck(ppro->m_csList);
	if (ppro->m_listDlg == nullptr) {
		ppro->m_listDlg = new CListDlg(ppro);
		ppro->m_listDlg->Show();
	}
	return 0;
}

bool CIrcProto::OnIrc_LISTSTART(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		CallFunctionSync(sttShowDlgList, this);
		m_channelNumber = 0;
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_LIST(const CIrcMessage *pmsg)
{
	if (!pmsg->m_bIncoming || pmsg->parameters.getCount() <= 2)
		return true;

	mir_cslockfull lck(m_csList);
	if (auto *pDlg = (CListDlg *)m_listDlg) {
		m_channelNumber++;
		lck.unlock();

		pDlg->AddChannel(pmsg);
	}
	return true;
}

bool CIrcProto::OnIrc_LISTEND(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		mir_cslock lck(m_csList);
		if (auto *pDlg = (CListDlg *)m_listDlg) {
			EnableWindow(GetDlgItem(m_listDlg->GetHwnd(), IDC_JOIN), true);
			pDlg->m_list.SetSelectionMark(0);
			pDlg->m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
			pDlg->m_list.SetColumnWidth(2, LVSCW_AUTOSIZE);
			pDlg->m_list.SetColumnWidth(3, LVSCW_AUTOSIZE);
			pDlg->UpdateList();

			wchar_t text[100];
			mir_snwprintf(text, TranslateT("Done: %u channels"), m_channelNumber);
			int percent = 100;
			if (m_noOfChannels > 0)
				percent = (int)(m_channelNumber * 100) / m_noOfChannels;
			if (percent < 70) {
				mir_wstrcat(text, L" ");
				mir_wstrcat(text, TranslateT("(probably truncated by server)"));
			}
			SetDlgItemText(m_listDlg->GetHwnd(), IDC_TEXT, text);
		}
	}
	ShowMessage(pmsg);
	return true;
}

void CIrcProto::ResetChannelList()
{
	{	mir_cslock lck(m_csList);
		if (m_listDlg == nullptr) {
			m_listDlg = new CListDlg(this);
			m_listDlg->Show();
		}
	}
	SetActiveWindow(m_listDlg->GetHwnd());

	ListView_DeleteAllItems(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW));
}
