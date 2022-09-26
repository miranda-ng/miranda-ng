/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it &/|
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY | FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// patterns

bool QSMainDlg::CheckPattern(CRowItem *pRow)
{
	if (m_patterns.getCount() == 0)
		return true;
	
	for (auto &p : m_patterns)
		p->res = 0;

	int i = 0;
	for (auto &it : g_plugin.m_columns) {
		if (it->bEnabled && it->bFilter && pRow->pValues[i].text != nullptr) {
			CMStringW buf(pRow->pValues[i].text);
			buf.MakeLower();

			for (auto &p : m_patterns)
				if (!p->res && buf.Find(p->str) != -1)
					p->res = true;
		}
		i++;
	}

	bool result = true;
	for (auto &p : m_patterns)
		result = result && p->res;
	return result;
}

void QSMainDlg::MakePattern(const wchar_t *pwszPattern)
{
	m_patterns.destroy();
	if (mir_wstrlen(pwszPattern) == 0)
		return;

	// m_wszPatternBuf works as a storage for patterns, we store pointers to it in m_patterns
	m_wszPatternBuf = mir_wstrdup(pwszPattern);
	CharLowerW(m_wszPatternBuf);
	
	for (wchar_t *p = m_wszPatternBuf; *p; ) {
		auto *pWord = wcspbrk(p, L" \"");
		if (pWord == nullptr) {
			m_patterns.insert(new Pattern(p));
			return;
		}

		bool isSpace = pWord[0] == ' ';

		// there's some valuable info between p and pWord
		if (pWord != p) {
			*pWord = 0;
			m_patterns.insert(new Pattern(p));
		}

		if (isSpace) {
			p = ltrimpw(pWord + 1); // skip all spaces
		}
		else {
			auto *pEnd = wcschr(++pWord, '\"');
			
			// treat the rest of line as one pattern
			if (pEnd == nullptr) {
				m_patterns.insert(new Pattern(pWord));
				return;
			}

			*pEnd = 0;
			m_patterns.insert(new Pattern(pWord));
			p = ltrimpw(pEnd + 1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void QSMainDlg::AddColumn(int idx, ColumnItem *pCol)
{
	LV_COLUMN lvcol = {};
	lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
	lvcol.pszText = TranslateW(pCol->title);
	lvcol.cx = pCol->width;
	m_grid.InsertColumn(idx, &lvcol);

	HDITEM hdi;
	hdi.mask = HDI_FORMAT;
	if (pCol->bFilter)
		hdi.fmt = HDF_LEFT | HDF_STRING | HDF_CHECKBOX | HDF_CHECKED;
	else
		hdi.fmt = HDF_LEFT | HDF_STRING | HDF_CHECKBOX;
	SendMessage(m_grid.GetHeader(), HDM_SETITEM, idx, LPARAM(&hdi));
}

void QSMainDlg::AddContactToList(MCONTACT hContact, CRowItem *pRow)
{
	LV_ITEMW li = {};
	li.mask = LVIF_IMAGE | LVIF_PARAM;
	li.iItem = 100000;
	li.iImage = Clist_GetContactIcon(hContact);
	li.lParam = LPARAM(pRow);

	li.iItem = m_grid.InsertItem(&li);
	li.iImage = 0;
	li.iSubItem = 0;

	for (int i = 0; i < g_plugin.m_columns.getCount(); i++) {
		auto &col = g_plugin.m_columns[i];
		if (!col.bEnabled)
			continue;

		// Client icons preprocess
		li.pszText = pRow->pValues[i].text;
		li.mask = LVIF_TEXT;
		if ((col.isClient && (g_plugin.m_flags & QSO_CLIENTICONS) && li.pszText != 0) || col.isXstatus || col.isGender)
			li.mask |= LVIF_IMAGE;
		m_grid.SetItem(&li);
		li.iSubItem++;
	}
}

void QSMainDlg::AdvancedFilter()
{
	m_grid.SetDraw(false);

	for (auto &it : m_rows) {
		bool bShow = (szFilterProto == nullptr) || !mir_strcmp(szFilterProto, it->szProto);
		if (bShow && !bShowOffline && it->status == ID_STATUS_OFFLINE)
			bShow = false;

		if (it->bPattern) {
			if (bShow) {
				if (!it->bActive)
					ProcessLine(it, false);
			}
			else {
				it->bActive = false;
				m_grid.DeleteItem(FindItem(it));
			}
		}
	}

	m_grid.SetDraw(true);
	InvalidateRect(m_grid.GetHwnd(), 0, false);

	Sort();
	UpdateSB();
}

void QSMainDlg::ChangeStatusPicture(CRowItem *pRow, MCONTACT, LPARAM lParam)
{
	int idx = FindItem(pRow);
	if (idx == -1)
		return;

	LV_ITEMW li = {};
	li.iItem = idx;
	li.mask = LVIF_IMAGE;
	li.iImage = lParam; //CallService(MS_CLIST_GETCONTACTICON,hContact,0);
	m_grid.SetItem(&li);
}

void QSMainDlg::CopyMultiLines()
{
	CMStringW buf;

	int i = 0;
	for (auto &it : g_plugin.m_columns) {
		if (it->bEnabled) {
			it->width = m_grid.GetColumnWidth(i);
			if (it->width >= 10)
				buf.AppendFormat(L"%s\t", it->title);
		}
		i++;
	}
	buf.Append(L"\r\n");

	int nRows = m_grid.GetItemCount();
	int nSelected = m_grid.GetSelectedCount();

	for (int j = 0; j < nRows; j++) {
		if (nSelected > 1 && !m_grid.GetItemState(j, LVIS_SELECTED))
			continue;

		auto *pRow = GetRow(j);
		
		i = 0;
		for (auto &it : g_plugin.m_columns) {
			if (it->bEnabled && it->width >= 10)
				buf.AppendFormat(L"%s\t", pRow->pValues[i].getText());
			i++;
		}
		buf.Append(L"\r\n");
	}

	Utils_ClipboardCopy(buf);
}

void QSMainDlg::DeleteByList()
{
	if (IDOK != MessageBoxW(0, TranslateT("Do you really want to delete selected contacts?"), TranslateT("Warning"), MB_OKCANCEL + MB_ICONWARNING))
		return;

	m_grid.SetDraw(false);

	for (int i = m_grid.GetItemCount() - 1; i >= 0; i--)
		if (m_grid.GetItemState(i, LVIS_SELECTED))
			db_delete_contact(GetRow(i)->hContact);

	m_grid.SetDraw(true);
}

void QSMainDlg::DeleteOneContact(MCONTACT hContact)
{
	if (ServiceExists(MS_CLIST_DELETECONTACT))
		CallService(MS_CLIST_DELETECONTACT, hContact, 0);
	else
		db_delete_contact(hContact);
}

wchar_t* QSMainDlg::DoMeta(MCONTACT hContact)
{
	for (auto &it : m_rows) {
		if (it->hContact != hContact)
			continue;

		if (it->bIsMeta) {
			if (it->wparam == 0)
				it->wparam = ++hLastMeta;
		}
		else if (it->bIsSub)
			it->lparam = FindMeta(db_mc_getMeta(hContact), it->wparam);

		if (it->wparam > 0) {
			CMStringW tmp(FORMAT, L"[%d]", int(it->wparam));
			if (it->lparam > 0)
				tmp.AppendFormat(L" %d", int(it->lparam));
			return tmp.Detach();
		}
		break;
	}

	return nullptr;
}

void QSMainDlg::DrawSB()
{
	CStatusBarItem global(0, 0);
	for (auto &it : m_sbdata) {
		global.found += it->found;
		global.liston += it->liston;
		global.online += it->online;
		global.total += it->total;
	}

	CMStringW buf(FORMAT, TranslateT("%i users found (%i) Online: %i"), global.found, m_rows.getCount(), global.online);
	
	RECT rc;
	HDC hdc = GetDC(hwndStatusBar);
	DrawTextW(hdc, buf, buf.GetLength(), &rc, DT_CALCRECT);
	ReleaseDC(hwndStatusBar, hdc);

	int all = rc.right - rc.left, i = 1;

	mir_ptr<int> parts((int*)mir_alloc(sizeof(int) * (m_sbdata.getCount()+2)));
	parts[0] = all;
	for (auto &it : m_sbdata) {
		UNREFERENCED_PARAMETER(it);
		all += 55;
		parts[i++] = all;
	}
	parts[i] = -1;
	SendMessageW(hwndStatusBar, SB_SETPARTS, m_sbdata.getCount() + 2, LPARAM(parts.get()));
	SendMessageW(hwndStatusBar, SB_SETTEXTW, 0, LPARAM(buf.c_str()));

	i = 1;
	for (auto &it : m_sbdata) {
		HICON hIcon;
		wchar_t c, *pc;
		if (it->bAccDel) {
			c = '!';
			pc = TranslateT("deleted");
			hIcon = Skin_LoadProtoIcon(it->szProto, ID_STATUS_OFFLINE);
		}
		else if (it->bAccOff) {
			c = '?';
			pc = TranslateT("disabled");
			hIcon = Skin_LoadProtoIcon(it->szProto, ID_STATUS_OFFLINE);
		}
		else {
			c = ' ';
			pc = TranslateT("active");
			hIcon = Skin_LoadProtoIcon(it->szProto, ID_STATUS_ONLINE);
		}

		SendMessageW(hwndStatusBar, SB_SETICON, i, (LPARAM)hIcon);

		buf.Format(L"%c %d", c, it->found);
		SendMessageW(hwndStatusBar, SB_SETTEXTW, i, LPARAM(buf.c_str()));

		auto *pa = Proto_GetAccount(it->szProto);
		buf.Format(L"%s (%s): %d (%d); %s %d (%d))", pa->tszAccountName, pc, it->found, it->total, TranslateT("Online"), it->liston, it->online);
		SendMessageW(hwndStatusBar, SB_SETTIPTEXTW, i, LPARAM(buf.c_str()));
		i++;
	}
}

void QSMainDlg::FillGrid()
{
	m_grid.SetDraw(false);

	for (auto &it: m_rows)
		ProcessLine(it);

	m_grid.SetDraw(true);
	InvalidateRect(m_grid.GetHwnd(), 0, FALSE);

	Sort();
	UpdateSB();
	AdvancedFilter();

	m_grid.SetCurSel(0);
}

void QSMainDlg::FillProtoCombo()
{
	cmbProto.ResetContent();
	cmbProto.AddString(TranslateT("All"));

	for (auto &it : Accounts())
		cmbProto.AddString(it->tszAccountName, (LPARAM)it);

	cmbProto.SetCurSel(0);
}

int QSMainDlg::FindItem(CRowItem *pRow)
{
	if (pRow == nullptr)
		return -1;

	LV_FINDINFO fi = {};
	fi.flags = LVFI_PARAM;
	fi.lParam = LPARAM(pRow);
	return m_grid.FindItem(-1, &fi);
}

int QSMainDlg::FindMeta(MCONTACT hMeta, WPARAM &metaNum)
{
	for (auto &it : m_rows) {
		if (it->hContact != hMeta)
			continue;

		// new meta
		if (it->wparam == 0) {
			it->wparam = ++hLastMeta;
			it->lparam = 0;
		}
		metaNum = it->wparam;
		it->lparam++;
		return it->lparam;
	}

	return 0;
}

CRowItem* QSMainDlg::FindRow(MCONTACT hContact)
{
	for (auto &it : m_rows)
		if (it->hContact == hContact)
			return it;

	return nullptr;
}

MCONTACT QSMainDlg::GetFocusedContact()
{
	int idx = m_grid.GetSelectionMark();
	if (idx == -1)
		return -1;
	
	INT_PTR data = m_grid.GetItemData(idx);
	return (data == -1) ? -1 : ((CRowItem *)data)->hContact;
}

int QSMainDlg::GetLVSubItem(int x, int y)
{
	LV_HITTESTINFO info = {};
	info.pt.x = x;
	info.pt.y = y;
	ScreenToClient(m_grid.GetHwnd(), &info.pt);
	if (m_grid.SubItemHitTest(&info) == -1)
		return -1;

	return (info.flags & LVHT_ONITEM) ? info.iSubItem : -1;
}

void QSMainDlg::PrepareTable(bool bReset)
{
	m_grid.DeleteAllItems();

	HDITEM hdi = {};
	hdi.mask = HDI_FORMAT;

	int old = tableColumns;
	tableColumns = 0;

	LV_COLUMN lvc = {};
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	for (auto &it : g_plugin.m_columns) {
		if (it->bEnabled)
			AddColumn(tableColumns++, it);
			
		it->SetSpecialColumns();
	}

	if (bReset)
		for (int i = old + tableColumns - 1; i >= tableColumns; i--)
			m_grid.DeleteColumn(i);
}

bool QSMainDlg::PrepareToFill()
{
	if (g_plugin.m_columns.getCount() == 0)
		return false;

	for (auto &it : g_plugin.m_columns)
		if (it->bEnabled)
			it->bInit = true;

	hLastMeta = 0;

	m_rows.destroy();
	for (auto &hContact : Contacts())
		m_rows.insert(new CRowItem(hContact, this));

	return m_rows.getCount() != 0;
}

void QSMainDlg::ProcessLine(CRowItem *pRow, bool test)
{
	if (pRow->bDeleted)
		return;

	if (test)
		pRow->bPattern = CheckPattern(pRow);

	if (pRow->bPattern) {
		if (!pRow->bActive) {
			if ((g_plugin.m_flags & QSO_SHOWOFFLINE) || pRow->status != ID_STATUS_OFFLINE) {
				// check for proto in combo
				if (!szFilterProto || !mir_strcmp(szFilterProto, pRow->szProto)) {
					pRow->bActive = true;
					AddContactToList(pRow->hContact, pRow);
				}
			}
		}
	}
	else if (pRow->bActive) {
		pRow->bActive = false;
		m_grid.DeleteItem(FindItem(pRow));
	}
}

void QSMainDlg::SaveColumnOrder()
{
	int idx = 0, col = 0;
	for (auto &it : g_plugin.m_columns) {
		if (it->bEnabled) {
			it->width = m_grid.GetColumnWidth(col++);
			g_plugin.SaveColumn(idx, *it);
		}
		idx++;
	}
}

void QSMainDlg::ShowContactMsgDlg(MCONTACT hContact)
{
	if (hContact) {
		Clist_ContactDoubleClicked(hContact);
		if (g_plugin.m_flags & QSO_AUTOCLOSE)
			Close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact menu

static INT_PTR ColChangeFunc(void *pThis, WPARAM hContact, LPARAM, LPARAM param)
{
	((QSMainDlg *)pThis)->ChangeCellValue(hContact, (int)param);
	return 0;
}

void QSMainDlg::ChangeCellValue(MCONTACT hContact, int col)
{
	auto &pCol = g_plugin.m_columns[col];

	auto *pRow = FindRow(hContact);
	if (pRow == nullptr)
		return;

	const char *szModule = pCol.module;
	if (szModule == nullptr)
		szModule = pRow->szProto;

	auto &pVal = pRow->pValues[col];
	CMStringW wszTitle(FORMAT, TranslateT("Editing of column %s"), pCol.title);

	ENTER_STRING es = {};
	es.szModuleName = MODULENAME;
	es.caption = TranslateT("Enter new cell value");
	es.ptszInitVal = pVal.text;
	if (!EnterString(&es))
		return;

	replaceStrW(pVal.text, es.ptszResult);
	if (pCol.datatype != QSTS_STRING)
		pVal.data = _wtoi(pVal.text);

	switch (pCol.datatype) {
	case QSTS_BYTE:
		db_set_b(hContact, szModule, pCol.setting, pVal.data);
		break;
	case QSTS_WORD:
		db_set_w(hContact, szModule, pCol.setting, pVal.data);
		break;
	case QSTS_DWORD:
	case QSTS_SIGNED:
	case QSTS_HEXNUM:
		db_set_dw(hContact, szModule, pCol.setting, pVal.data);
		break;

	case QSTS_STRING:
		db_set_ws(hContact, szModule, pCol.setting, pVal.text);
		break;
	}

	UpdateLVCell(FindItem(pRow), col, pVal.text);
}

void QSMainDlg::ShowContactMenu(MCONTACT hContact, int col)
{
	if (hContact == 0)
		return;

	HANDLE srvhandle = 0;

	bool bDoit = false;
	if (col >= 0) {
		if ((col = ListViewToColumn(col)) == -1)
			return;

		auto &pCol = g_plugin.m_columns[col];
		if (pCol.setting_type == QST_SETTING && pCol.datatype != QSTS_TIMESTAMP) {
			bDoit = true;

			srvhandle = CreateServiceFunctionObjParam("QS/Dummy", &ColChangeFunc, this, col);

			if (mnuhandle == nullptr) {
				CMenuItem mi(&g_plugin);
				SET_UID(mi, 0xD384A798, 0x5D4C, 0x48B4, 0xB3, 0xE2, 0x30, 0x04, 0x6E, 0xD6, 0xF4, 0x81);
				mi.name.a = LPGEN("Change setting through QS");
				mi.pszService = "QS/Dummy";
				mnuhandle = Menu_AddContactMenuItem(&mi);
			}
			else Menu_ModifyItem(mnuhandle, 0, INVALID_HANDLE_VALUE, 0);
		}
	}

	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = Menu_BuildContactMenu(hContact);
	if (hMenu) {
		int iCmd = ::TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_grid.GetHwnd(), 0);
		if (iCmd) {
			if (Clist_MenuProcessCommand(iCmd, MPCF_CONTACTMENU, hContact)) {
				if (g_plugin.m_flags & QSO_AUTOCLOSE)
					CloseSrWindow();
			}
		}

		::DestroyMenu(hMenu);
	}

	if (srvhandle)
		DestroyServiceFunction(srvhandle);
	if (bDoit)
		Menu_ShowItem(mnuhandle, false);
}

/////////////////////////////////////////////////////////////////////////////////////////
// muptiple selection popup menu

static HMENU MakeContainerMenu()
{
	HMENU hMenu = CreatePopupMenu();

	for (int i = 0;; i++) {
		char setting[10];
		_itoa_s(i, setting, 10);
		ptrW wszName(db_get_wsa(0, "TAB_ContainersW", setting));
		if (wszName != nullptr)
			AppendMenuW(hMenu, MF_STRING, 300 + i, wszName);
		else
			break;
	}
	
	return hMenu;
}

void QSMainDlg::ShowMultiPopup(int cnt)
{
	HMENU hMenu = CreatePopupMenu();
	AppendMenuW(hMenu, MF_DISABLED + MF_STRING, 0, CMStringW(FORMAT, TranslateT("Selected %d contacts"), cnt));
	AppendMenuW(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenuW(hMenu, MF_STRING, 101, TranslateT("&Delete"));
	AppendMenuW(hMenu, MF_STRING, 102, TranslateT("&Copy"));
	AppendMenuW(hMenu, MF_STRING, 103, TranslateT("C&onvert to Meta"));

	HMENU cntmenu = MakeContainerMenu();
	AppendMenuW(hMenu, MF_POPUP, UINT_PTR(cntmenu), TranslateT("Attach to &Tab container"));

	if (HMENU grpmenu = Clist_GroupBuildMenu(400))
		AppendMenuW(hMenu, MF_POPUP, UINT_PTR(grpmenu), TranslateT("&Move to Group"));

	POINT pt;
	GetCursorPos(&pt);

	int iRes = TrackPopupMenu(hMenu, TPM_RETURNCMD+TPM_NONOTIFY, pt.x, pt.y, 0, m_hwnd, 0);
	switch (iRes) {
	case 101:
		DeleteByList();
		break;

	case 102:
		CopyMultiLines();
		break;

	case 103:
		ConvertToMeta();
		break;
	}
	
	if (iRes >= 300 && iRes <= 399) {
		wchar_t buf[100];
		if (iRes == 300) // default container, just delete setting
			buf[0] = 0;
		else
			GetMenuStringW(cntmenu, iRes, buf, _countof(buf), MF_BYCOMMAND);

		MoveToContainer(buf);
	}
	else if (iRes >= 400 && iRes <= 499)
		MoveToGroup(Clist_GroupGetName(iRes - 400));
}

void QSMainDlg::ConvertToMeta()
{
	MCONTACT hMeta = 0;
	
	int nCount = m_grid.GetItemCount();
	for (int i = 0; i < nCount; i++) {
		if (!m_grid.GetItemState(i, LVIS_SELECTED))
			continue;

		auto *pRow = GetRow(i);
		if (MCONTACT tmp = db_mc_getMeta(pRow->hContact)) {
			if (hMeta == 0)
				hMeta = tmp;
			else if (hMeta != tmp) {
				MessageBoxW(m_hwnd, TranslateT("Some of selected contacts in different metacontacts already"), L"Quick Search", MB_ICONERROR);
				return;
			}
		}
	}

	if (hMeta != 0)
		if (IDYES != MessageBoxW(0, TranslateT("One or more contacts already belong to the same metacontact. Try to convert anyway?"), L"Quick Search", MB_YESNO + MB_ICONWARNING))
			return;

	for (int i = 0; i < nCount; i++) {
		if (!m_grid.GetItemState(i, LVIS_SELECTED))
			continue;

		auto *pRow = GetRow(i);
		if (hMeta)
			db_mc_addToMeta(pRow->hContact, hMeta);
		else
			db_mc_convertToMeta(pRow->hContact);
	}
}

void QSMainDlg::MoveToContainer(const wchar_t *pwszName)
{
	int grcol = -1;
	for (auto &it : g_plugin.m_columns) {
		if (it->isContainer) {
			if (it->bEnabled)
				grcol = g_plugin.m_columns.indexOf(&it);
			else
				it->bInit = false;
		}
	}

	int nCount = m_grid.GetItemCount();
	for (int i = 0; i < nCount; i++) {
		if (!m_grid.GetItemState(i, LVIS_SELECTED))
			continue;

		auto *pRow = GetRow(i);
		if (*pwszName == 0)
			db_unset(pRow->hContact, "Tab_SRMsg", "containerW");
		else
			db_set_ws(pRow->hContact, "Tab_SRMsg", "containerW", pwszName);

		if (grcol != -1) {
			auto &pVal = pRow->pValues[grcol];
			replaceStrW(pVal.text, (*pwszName) ? pwszName : nullptr);
			UpdateLVCell(i, grcol, pwszName);
		}
	}
}

void QSMainDlg::MoveToGroup(const wchar_t *pwszName)
{
	int grcol = -1;
	for (auto &it : g_plugin.m_columns) {
		if (it->isGroup) {
			if (it->bEnabled)
				grcol = g_plugin.m_columns.indexOf(&it);
			else
				it->bInit = false;
		}
	}

	int nCount = m_grid.GetItemCount();
	for (int i = 0; i < nCount; i++) {
		if (!m_grid.GetItemState(i, LVIS_SELECTED))
			continue;

		auto *pRow = GetRow(i);
		Clist_SetGroup(pRow->hContact, pwszName);

		if (grcol != -1) {
			auto &pVal = pRow->pValues[grcol];
			replaceStrW(pVal.text, pwszName);
			UpdateLVCell(i, grcol, pwszName);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// grid sorting

static int CALLBACK CompareItem(LPARAM l1, LPARAM l2, LPARAM type)
{
	bool typ1, typ2;
	UINT_PTR i1, i2;
	int result = 0;
	CRowItem *r1 = (CRowItem *)l1, *r2 = (CRowItem *)l2;

	if (type == StatusSort) {
		i1 = r1->status, i2 = r2->status;
		if (i1 == ID_STATUS_OFFLINE) i1 += 64;
		if (i2 == ID_STATUS_OFFLINE) i2 += 64;
		typ1 = typ2 = false;
	}
	else {
		auto &res1 = r1->pValues[type], &res2 = r2->pValues[type];
		i1 = res1.data, i2 = res2.data;
		typ1 = i1 == -1; typ2 = i1 == -1;

		if (typ1 && typ2) { // two strings
			if (res1.text == 0 && res2.text == 0)
				result = 0;
			else if (res2.text == 0)
				result = 1;
			else if (res1.text == 0)
				result = -1;
			else
				result = lstrcmpiW(res1.text, res2.text);
		}
		else if (typ1 || typ2) // string & num
			result = (typ1) ? 1 : -1;
	}

	if (!typ1 && !typ2) { // not strings
		if (i1 > i2)
			result = 1;
		else if (i1 < i2)
			result = -1;
		else
			result = 0;
	}

	if (g_plugin.m_flags & QSO_SORTASC)
		result = -result;
	return result;
}

void QSMainDlg::Sort()
{
	if (g_plugin.m_sortOrder >= tableColumns)
		g_plugin.m_sortOrder = StatusSort;
	m_grid.SortItems(&CompareItem, ListViewToColumn(g_plugin.m_sortOrder));

	if (g_plugin.m_sortOrder != StatusSort && (g_plugin.m_flags & QSO_SORTBYSTATUS))
		m_grid.SortItems(&CompareItem, StatusSort);
}

void QSMainDlg::UpdateLVCell(int item, int column, const wchar_t *pwszText)
{
	auto &pCol = g_plugin.m_columns[column];
	auto *pRow = GetRow(item);

	if (pwszText == nullptr) {
		auto &pVal = pRow->pValues[column];
		replaceStrW(pVal.text, 0);
		pVal.LoadOneItem(pRow->hContact, pCol, this);
		pwszText = pVal.text;
	}

	m_grid.SetItemText(item, ColumnToListView(column), pwszText);
	
	if (pCol.bFilter)
		ProcessLine(pRow, true);
	if (g_plugin.m_sortOrder == column)
		Sort();
}

void QSMainDlg::UpdateSB()
{
	m_sbdata.destroy();

	for (auto &it : m_rows) {
		if (it->szProto == nullptr)
			continue;

		CStatusBarItem tmp(it->szProto, it->flags);
		auto *pItem = m_sbdata.find(&tmp);
		if (pItem == nullptr)
			m_sbdata.insert(pItem = new CStatusBarItem(it->szProto, it->flags));

		pItem->total++;

		if (it->bActive)
			pItem->found++;

		if (it->status != ID_STATUS_OFFLINE) {
			pItem->online++;
			if (it->bActive)
				pItem->liston++;
		}
	}

	DrawSB();
}
