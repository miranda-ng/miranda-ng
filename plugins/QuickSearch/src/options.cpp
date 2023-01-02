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

class COptionsDlg : public CDlgBase
{
	OBJLIST<ColumnItem> m_columns;

	void AddColumn(int idx, ColumnItem *pCol)
	{
		LVITEM lvi = {};
		lvi.mask = LVIF_PARAM;
		lvi.iItem = idx;
		lvi.lParam = LPARAM(pCol);
		m_list.InsertItem(&lvi);
	}

	void CheckDirection(int iItem)
	{
		btnUp.Enable(iItem > 0);
		btnDown.Enable(iItem < m_list.GetItemCount()-1);
	}

	void ClearScreen()
	{
		// setting
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_DATATYPE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_DATATYPE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_MODULE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_MODULE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_SETTING), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_SETTING), SW_HIDE);

		// contact info
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_CNFTYPE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_CNFTYPE), SW_HIDE);

		// others
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_OTHER), SW_HIDE);
	}

	void DisplayCurInfo(const ColumnItem *pCol)
	{
		ClearScreen();
		SetupScreen(pCol->setting_type);

		editTitle.SetText(pCol->title);
		cmbVarType.SelectData(pCol->setting_type);

		switch (pCol->setting_type) {
		case QST_SETTING:
			cmbDataType.SelectData(pCol->datatype);
			editModule.SetTextA(pCol->module);
			editSetting.SetTextA(pCol->setting);
			break;

		case QST_SCRIPT:
			SetDlgItemTextW(m_hwnd, IDC_E_SCRIPT, pCol->script);
			break;

		case QST_CONTACTINFO:
			cmbCnfType.SelectData(pCol->cnftype);
			break;

		case QST_OTHER:
			cmbOther.SelectData(pCol->other);
			break;
		}
	}

	void FillTableLine(int item, ColumnItem *pColumn)
	{
		m_list.SetItemText(item, 1, pColumn->title);

		switch (pColumn->setting_type) {
		case QST_SETTING:
			m_list.SetItemText(item, 2, _A2T(pColumn->module));
			m_list.SetItemText(item, 3, _A2T(pColumn->setting));
			break;

		case QST_SCRIPT:
			m_list.SetItemText(item, 2, TranslateT("Script"));
			break;

		case QST_SERVICE:
			m_list.SetItemText(item, 2, TranslateT("Service"));
			m_list.SetItemText(item, 3, _A2T(pColumn->svc.service));
			break;

		case QST_CONTACTINFO:
			m_list.SetItemText(item, 2, TranslateT("Contact info"));
			m_list.SetItemText(item, 3, cnf2str(pColumn->cnftype));
			break;

		case QST_OTHER:
			m_list.SetItemText(item, 2, TranslateT("Other"));
			if (pColumn->other == QSTO_METACONTACT)
				m_list.SetItemText(item, 3, TranslateT("Metacontact"));
			break;
		}
	}

	void InitScreen()
	{
		// setting
		cmbDataType.SetCurSel(0);
		editModule.SetText(L"");
		editSetting.SetText(L"");

		// contact info
		cmbCnfType.SetCurSel(0);

		// others
		cmbOther.SetCurSel(0);
	}

	void ResizeControl(int id, int width)
	{
		HWND hwnd = GetDlgItem(m_hwnd, id);
		RECT rc;
		::GetWindowRect(hwnd, &rc);
		::SetWindowPos(hwnd, 0, 0, 0, width, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	void SetupScreen(int type)
	{
		if (!IsWindowVisible(GetDlgItem(m_hwnd, IDC_E_TITLE)))
			return;

		// setting
		int cmd = (type == QST_SETTING) ? SW_SHOW : SW_HIDE;
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_DATATYPE), cmd);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_DATATYPE), cmd);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_MODULE), cmd);
		editModule.Show(cmd == SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_SETTING), cmd);
		editSetting.Show(cmd == SW_SHOW);

		// contact info
		cmd = (type == QST_CONTACTINFO) ? SW_SHOW : SW_HIDE; 
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_CNFTYPE), cmd);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_CNFTYPE), cmd);

		// script
		cmd = (type == QST_SCRIPT) ? SW_SHOW : SW_HIDE; 
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_SCRIPT), cmd);

		// others
		cmd = (type == QST_OTHER) ? SW_SHOW : SW_HIDE; 
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_OTHER), cmd);
	}

	void UpdateList()
	{
		m_list.DeleteAllItems();

		int cnt = 0;
		for (auto &it : m_columns) {
			AddColumn(cnt, it);
			FillTableLine(cnt, it);
			m_list.SetCheckState(cnt, it->bEnabled);
			cnt++;
		}

		m_list.SetCurSel(0);
	}

	CCtrlEdit editTitle, editModule, editSetting;
	CCtrlCheck chkSortStatus, chkAutoClose, chkUseToolstyle, chkDrawGrid, chkSavePattern, chkClientIcons;
	CCtrlCombo cmbVarType, cmbDataType, cmbOther, cmbCnfType;
	CCtrlListView m_list;
	CCtrlButton btnSave, btnResize;
	CCtrlMButton btnNew, btnUp, btnDown, btnDelete, btnDefault, btnReload;

public: 
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_columns(1),
		m_list(this, IDC_LIST),
		btnSave(this, IDC_SETITEM),
		btnResize(this, IDC_B_RESIZE),
		btnUp(this, IDC_UP, g_plugin.getIcon(IDI_UP), LPGEN("Up")),
		btnNew(this, IDC_NEW, g_plugin.getIcon(IDI_NEW), LPGEN("New")),
		btnDown(this, IDC_DN, g_plugin.getIcon(IDI_DOWN), LPGEN("Down")),
		btnDelete(this, IDC_DELETE, g_plugin.getIcon(IDI_DELETE), LPGEN("Delete")),
		btnReload(this, IDC_RELOAD, g_plugin.getIcon(IDI_RELOAD), LPGEN("Reload")),
		btnDefault(this, IDC_DEFAULT, g_plugin.getIcon(IDI_DEFAULT), LPGEN("Default")),
		editTitle(this, IDC_E_TITLE),
		editModule(this, IDC_E_MODULE),
		editSetting(this, IDC_E_SETTING),
		cmbOther(this, IDC_C_OTHER),
		cmbCnfType(this, IDC_C_CNFTYPE),
		cmbVarType(this, IDC_C_VARTYPE),
		cmbDataType(this, IDC_C_DATATYPE),
		chkDrawGrid(this, IDC_CH_DRAWGRID),
		chkAutoClose(this, IDC_CH_AUTOCLOSE),	
		chkSortStatus(this, IDC_CH_SORTSTATUS),
		chkSavePattern(this, IDC_CH_SAVEPATTERN),
		chkClientIcons(this, IDC_CH_CLIENTICONS),
		chkUseToolstyle(this, IDC_CH_USETOOLSTYLE)
	{
		m_list.OnItemChanged = Callback(this, &COptionsDlg::onItemChanged_List);

		btnUp.OnClick = Callback(this, &COptionsDlg::onClick_Up);
		btnNew.OnClick = Callback(this, &COptionsDlg::onClick_New);
		btnDown.OnClick = Callback(this, &COptionsDlg::onClick_Down);
		btnSave.OnClick = Callback(this, &COptionsDlg::onClick_Save);
		btnDelete.OnClick = Callback(this, &COptionsDlg::onClick_Delete);
		btnReload.OnClick = Callback(this, &COptionsDlg::onClick_Reload);
		btnResize.OnClick = Callback(this, &COptionsDlg::onClick_Resize);
		btnDefault.OnClick = Callback(this, &COptionsDlg::onClick_Default);

		cmbVarType.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged_Var);
	}

	bool OnInitDialog() override
	{
		editTitle.SetSilent(); editModule.SetSilent(); editSetting.SetSilent();
		cmbOther.SetSilent(); cmbCnfType.SetSilent(); cmbVarType.SetSilent(); cmbDataType.SetSilent();

		m_list.SetExtendedListViewStyle(m_list.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		m_list.AddColumn(0, L"#", 20);
		m_list.AddColumn(1, TranslateT("Title"), g_plugin.getWord("col1", 95));
		m_list.AddColumn(2, TranslateT("Module/Info type"), g_plugin.getWord("col2", 105));
		m_list.AddColumn(3, TranslateT("Setting"), g_plugin.getWord("col3", 85));

		cmbVarType.AddString(TranslateT("Database setting"), QST_SETTING);
		cmbVarType.AddString(TranslateT("Script"), QST_SCRIPT);
		cmbVarType.AddString(TranslateT("Contact info"), QST_CONTACTINFO);
		cmbVarType.AddString(TranslateT("Other"), QST_OTHER);
		cmbVarType.SetCurSel(0);

		cmbDataType.AddString(TranslateT("Byte"), QSTS_BYTE);
		cmbDataType.AddString(TranslateT("Word"), QSTS_WORD);
		cmbDataType.AddString(TranslateT("Dword"), QSTS_DWORD);
		cmbDataType.AddString(TranslateT("Signed"), QSTS_SIGNED);
		cmbDataType.AddString(TranslateT("Hexadecimal"), QSTS_HEXNUM);
		cmbDataType.AddString(TranslateT("String"), QSTS_STRING);
		cmbDataType.AddString(TranslateT("Timestamp"), QSTS_TIMESTAMP);
		cmbDataType.SetCurSel(0);

		cmbOther.AddString(TranslateT("Last seen"), QSTO_LASTSEEN);
		cmbOther.AddString(TranslateT("Last event"), QSTO_LASTEVENT);
		cmbOther.AddString(TranslateT("Metacontact"), QSTO_METACONTACT);
		cmbOther.AddString(TranslateT("Event count"), QSTO_EVENTCOUNT);
		cmbOther.AddString(TranslateT("Display name"), QSTO_DISPLAYNAME);
		cmbOther.AddString(TranslateT("Account name"), QSTO_ACCOUNT);
		cmbOther.SetCurSel(0);

		for (int i = CNF_FIRSTNAME; i < CNF_MAX; i++)
			if (auto *pwszText = cnf2str(i))
				cmbCnfType.AddString(pwszText, i);
		cmbCnfType.SetCurSel(0);

		chkDrawGrid.SetState((g_plugin.m_flags & QSO_DRAWGRID) != 0);
		chkAutoClose.SetState((g_plugin.m_flags & QSO_AUTOCLOSE) != 0);
		chkSortStatus.SetState((g_plugin.m_flags & QSO_SORTBYSTATUS) != 0);
		chkClientIcons.SetState((g_plugin.m_flags & QSO_CLIENTICONS) != 0);
		chkSavePattern.SetState((g_plugin.m_flags & QSO_SAVEPATTERN) != 0);
		chkUseToolstyle.SetState((g_plugin.m_flags & QSO_TOOLSTYLE) != 0);

		// make local copy of column descriptions
		for (auto &it : g_plugin.m_columns)
			m_columns.insert(new ColumnItem(*it));

		UpdateList();
		onClick_Resize(0);
		if (m_columns.getCount())
			DisplayCurInfo(&m_columns[0]);
		return true;
	}

	bool OnApply() override
	{
		// checkboxes
		g_plugin.m_flags &= ~QSO_MAINOPTIONS;
		if (chkDrawGrid.IsChecked()) g_plugin.m_flags |= QSO_DRAWGRID;
		if (chkAutoClose.IsChecked()) g_plugin.m_flags |= QSO_AUTOCLOSE;
		if (chkSortStatus.IsChecked()) g_plugin.m_flags |= QSO_SORTBYSTATUS;
		if (chkClientIcons.IsChecked()) g_plugin.m_flags |= QSO_CLIENTICONS;
		if (chkSavePattern.IsChecked()) g_plugin.m_flags |= QSO_SAVEPATTERN;
		if (chkUseToolstyle.IsChecked()) g_plugin.m_flags |= QSO_TOOLSTYLE;

		int tmpbool = CloseSrWindow(false);

		g_plugin.m_columns.destroy();
		int nCount = m_list.GetItemCount();
		for (int i = 0; i < nCount; i++) {
			auto *pCol = (ColumnItem *)m_list.GetItemData(i);
			pCol->bEnabled = m_list.GetCheckState(i) != 0;
			g_plugin.m_columns.insert(new ColumnItem(*pCol));
		}

		g_plugin.SaveOptions();

		if (tmpbool)
			OpenSrWindow(0);
		return true;
	}

	void OnDestroy() override
	{
		m_columns.destroy();

		g_plugin.setWord("col1", m_list.GetColumnWidth(1));
		g_plugin.setWord("col2", m_list.GetColumnWidth(2));
		g_plugin.setWord("col3", m_list.GetColumnWidth(3));
	}

	void onClick_New(CCtrlButton *)
	{
		int idx = m_list.GetSelectionMark()+1;
		auto *pNew = new ColumnItem(TranslateT("New column"));
		m_columns.insert(pNew);

		AddColumn(idx, pNew);
		m_list.EnsureVisible(idx, FALSE);
		m_list.SetCurSel(idx);
		InitScreen();
		CheckDirection(idx);
		btnDelete.Enable();
		NotifyChange();
	}

	void onClick_Delete(CCtrlButton *)
	{
		int idx = m_list.GetSelectionMark();
		auto *pCol = (ColumnItem *)m_list.GetItemData(idx);

		m_list.DeleteItem(idx);
		m_columns.remove(pCol);

		int nCount = m_list.GetItemCount();
		if (nCount == 0) {
			m_list.Disable();
			InitScreen();
		}
		else {
			if (nCount == idx)
				idx--;
			m_list.SetCurSel(idx);
		}
		CheckDirection(idx);
		NotifyChange();
	}

	void onClick_Up(CCtrlButton *)
	{
		int idx = m_list.GetSelectionMark();
		if (idx > 0) {
			CheckDirection(m_list.MoveItem(idx, -1));
			NotifyChange();
		}
	}

	void onClick_Down(CCtrlButton *)
	{
		int idx = m_list.GetSelectionMark();
		if (idx < m_list.GetItemCount() - 1) {
			CheckDirection(m_list.MoveItem(idx, 1));
			NotifyChange();
		}
	}

	void onClick_Reload(CCtrlButton *)
	{
		g_plugin.LoadColumns(m_columns);
		UpdateList();
	}

	void onClick_Default(CCtrlButton *)
	{
		LoadDefaultColumns(m_columns);
		UpdateList();
		NotifyChange();
	}

	void onClick_Save(CCtrlButton *)
	{
		if (m_list.GetItemCount() == 0) {
			AddColumn(0, new ColumnItem(TranslateT("New column")));
			m_list.SetCurSel(0);
			btnDelete.Enable();
		}

		int idx = m_list.GetSelectionMark();
		auto *pCol = (ColumnItem *)m_list.GetItemData(idx);
		pCol->dwFlags = 0;
		if (m_list.GetCheckState(idx))
			pCol->bEnabled = pCol->bFilter = true;
		pCol->setting_type = cmbVarType.GetItemData(cmbVarType.GetCurSel());
		replaceStrW(pCol->title, editTitle.GetText());

		switch (pCol->setting_type) {
		case QST_SETTING:
			pCol->datatype = cmbDataType.GetItemData(cmbDataType.GetCurSel());
			pCol->module = mir_u2a(ptrW(editModule.GetText()));
			pCol->setting = mir_u2a(ptrW(editSetting.GetText()));
			break;

		case QST_CONTACTINFO:
			pCol->cnftype = cmbCnfType.GetItemData(cmbCnfType.GetCurSel());
			break;

		case QST_OTHER:
			pCol->other = cmbCnfType.GetItemData(cmbCnfType.GetCurSel());
			break;
		}

		FillTableLine(idx, pCol);
		NotifyChange();
	}

	void onClick_Resize(CCtrlButton *)
	{
		wchar_t *pcw;
		int dx, rside;

		RECT rc, rc1;
		GetClientRect(m_hwnd, &rc);
		GetWindowRect(btnResize.GetHwnd(), &rc1);

		POINT pt = { rc1.left, 0 };
		ScreenToClient(m_hwnd, &pt);
		if (pt.x < (rc.right - 50)) {
			rside = SW_HIDE;
			dx = rc.right - (rc1.right - rc1.left) - 4;
			pcw = L"<";
		}
		else {
			rside = SW_SHOW;

			GetWindowRect(GetDlgItem(m_hwnd, IDC_S_COLSETTING), &rc);
			pt.x = rc.left;
			pt.y = 0;
			ScreenToClient(m_hwnd, &pt);
			dx = pt.x - (rc1.right - rc1.left) - 4;
			pcw = L">";
		}
		
		btnResize.SetText(pcw);

		// move separator button
		SetWindowPos(btnResize.GetHwnd(), 0, dx + 2, 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

		// resize left side controls
		ResizeControl(IDC_LIST, dx);
		ResizeControl(IDC_CH_GROUP, dx);

		ResizeControl(IDC_CH_USETOOLSTYLE, dx - 8);
		ResizeControl(IDC_CH_DRAWGRID, dx - 8);
		ResizeControl(IDC_CH_SAVEPATTERN, dx - 8);
		ResizeControl(IDC_CH_AUTOCLOSE, dx - 8);
		ResizeControl(IDC_CH_SORTSTATUS, dx - 8);
		ResizeControl(IDC_CH_CLIENTICONS, dx - 8);

		// show/hide setting block (ugly, i know!)
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_COLSETTING), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_LINE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_TITLE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_TITLE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_SCRIPT), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_MODULE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_E_SETTING), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_S_VARTYPE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_VARTYPE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_OTHER), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_CNFTYPE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_C_DATATYPE), rside);
		ShowWindow(GetDlgItem(m_hwnd, IDC_SETITEM), rside);

		ClearScreen();
		if (rside == SW_SHOW)
			SetupScreen(cmbVarType.GetItemData(cmbVarType.GetCurSel()));
	}
	
	void onItemChanged_List(CCtrlListView::TEventInfo *ev)
	{
		auto *nmlv = ev->nmlv;
		// we got new focus
		if ((nmlv->uOldState & LVNI_FOCUSED) < (nmlv->uNewState & LVNI_FOCUSED)) {
			CheckDirection(nmlv->iItem);
			InitScreen();
			DisplayCurInfo((ColumnItem*)nmlv->lParam);
		}
	}

	void onSelChanged_Var(CCtrlCombo *pCombo)
	{
		SetupScreen(pCombo->GetItemData(pCombo->GetCurSel()));
	}
};

int OnOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Contacts");
	odp.szTitle.a = LPGEN("Quick Search");
	odp.position = 900003000;
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
