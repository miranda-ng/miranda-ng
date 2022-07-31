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

#define IDM_STAYONTOP      (WM_USER+1)

static QSMainDlg *g_pDlg = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// edit control window procedure

static LRESULT CALLBACK sttNewEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto *pDlg = (QSMainDlg *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (pDlg)
		if (INT_PTR res = pDlg->NewEditProc(msg, wParam, lParam))
			return res;

	return mir_callNextSubclass(hwnd, sttNewEditProc, msg, wParam, lParam);
}

INT_PTR QSMainDlg::NewEditProc(UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 27) // Escape
			Close();
		break;

	case WM_KEYUP:
		if (wParam == VK_RETURN) {
			if (m_grid.GetSelectedCount() == 1)
				ShowContactMsgDlg(GetFocusedContact());
			return 0;
		}
		break;

	case WM_KEYDOWN:
		int count = m_grid.GetItemCount();
		int current = m_grid.GetNextItem(-1, LVNI_FOCUSED);
		int next = -1;
		if (count > 0) {
			switch (wParam) {
			case VK_UP:
				if (current > 0)
					next = current - 1;
				break;

			case VK_DOWN:
				if (current < count - 1)
					next = current + 1;
				break;

			case VK_F5:
				onClick_Refresh(0);
				return 0;

			case VK_NEXT:
			case VK_PRIOR:
				int perpage = m_grid.GetCountPerPage();
				if (wParam == VK_NEXT)
					next = min(current + perpage, count);
				else
					next = max(current - perpage, 0);
				break;
			}
		}

		if (next >= 0) {
			m_grid.SetItemState(-1, 0, LVIS_SELECTED);
			m_grid.SetCurSel(next);
			m_grid.EnsureVisible(next, FALSE);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// list header window procedure

static void MakeColumnMenu()
{
	HMENU hMenu = CreatePopupMenu();

	for (auto &it : g_plugin.m_columns) {
		int flag = MF_STRING + (it->bEnabled) ? MF_CHECKED : MF_UNCHECKED;
		AppendMenuW(hMenu, flag, 100 + g_plugin.m_columns.indexOf(&it), TranslateW(it->title));
	}

	POINT pt;
	GetCursorPos(&pt);

	int id = TrackPopupMenu(hMenu, TPM_RETURNCMD + TPM_NONOTIFY, pt.x, pt.y, 0, g_pDlg->GetHwnd(), 0);
	if (id >= 100)
		g_pDlg->ToggleColumn(id - 100);

	DestroyMenu(hMenu);
}

void QSMainDlg::ToggleColumn(int col)
{
	auto &pCol = g_plugin.m_columns[col];

	if (!pCol.bEnabled) { // show column
		pCol.bEnabled = true;

		if (!pCol.bInit) {
			for (auto &it : m_rows)
				it->pValues[col].LoadOneItem(it->hContact, pCol, this);
			pCol.bInit = true;
		}

		// screen
		int lvcol = ColumnToListView(col);
		AddColumn(lvcol, &pCol);
		
		int nCount = m_grid.GetItemCount();
		for (int i = 0; i < nCount; i++) {
			auto *pRow = GetRow(i);

			LV_ITEMW li;
			li.iItem = i;
			li.iSubItem = lvcol;
			li.mask = LVIF_TEXT;
			li.pszText = pRow->pValues[col].text;
			if ((pCol.isClient && (g_plugin.m_flags & QSO_CLIENTICONS) && li.pszText) || pCol.isGender || pCol.isXstatus)
				li.mask |= LVIF_IMAGE;
			m_grid.SetItem(&li);
		}
	}
	else { // hide column
		int cnt = 0;
		for (auto &it : g_plugin.m_columns)
			if (it->bEnabled)
				cnt++;

		// keep at least one visible column (1 + this)
		if (cnt > 2) {
			m_grid.DeleteColumn(col);
			pCol.bEnabled = false;
		}
	}
}

static LRESULT CALLBACK sttNewLVHProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		return 0;

	case WM_RBUTTONDOWN:
		MakeColumnMenu();
		break;
	}

	return mir_callNextSubclass(hwnd, sttNewLVHProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// grid list window procedure

static int OldHSubItem = 0, OldHItem = 0;

static LRESULT CALLBACK sttNewLVProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto *pDlg = (QSMainDlg *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (pDlg)
		if (INT_PTR res = pDlg->NewLVProc(msg, wParam, lParam))
			return res;

	return mir_callNextSubclass(hwnd, sttNewLVProc, msg, wParam, lParam);
}

INT_PTR QSMainDlg::NewLVProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LV_HITTESTINFO pinfo;

	switch (msg) {
	case WM_CHAR:
		switch (wParam) { // ESC
		case 27:
			Close();
			break;

		case 1: // Cltr+A
			m_grid.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		case 3: // Ctrl-C
			CopyMultiLines();
			break;

		case 8: // backspace
			edtFilter.SendMsg(msg, wParam, lParam);
			break;
		}

		if (wParam >= 32 && wParam <= 127) // letters
			edtFilter.SendMsg(msg, wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		pinfo.pt.x = LOWORD(lParam);
		pinfo.pt.y = HIWORD(lParam);
		pinfo.flags = 0;
		if (m_grid.SubItemHitTest(&pinfo) == -1)
			break;

		if ((pinfo.flags & LVHT_ONITEM) && (pinfo.iItem != OldHItem || pinfo.iSubItem != OldHSubItem)) {
			OldHSubItem = pinfo.iSubItem;
			OldHItem = pinfo.iItem;

			if (g_bTipperInstalled) {
				if (TTShowed) {
					TTShowed = false;
					Tipper_Hide();
				}
				m_hover.Stop();

				if (OldHSubItem == 0)
					m_hover.Start(450);
			}

			TOOLINFOW ti = {};
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_SUBCLASS + TTF_IDISHWND;
			ti.hwnd = m_hwnd;
			ti.uId = LPARAM(m_hwnd);

			int num = ListViewToColumn(OldHSubItem);
			auto &pCol = g_plugin.m_columns[num];
			if (pCol.isXstatus || pCol.isGender) {
				auto *pRow = GetRow(OldHItem);
				if (pCol.isGender) {
					switch (pRow->pValues[num].data) {
					case 'M': ti.lpszText = TranslateT("Male"); break;
					case 'F': ti.lpszText = TranslateT("Female"); break;
					default: ti.lpszText = TranslateT("Unknown"); break;
					}
				}
				else {
					wchar_t buf[256];
					mir_wstrncpy(buf, pRow->pValues[num].text, _countof(buf));
					int iStatus = _wtoi(buf);

					CUSTOM_STATUS ics = {};
					ics.cbSize = sizeof(ics);
					ics.status = &iStatus;
					ics.flags = CSSF_DEFAULT_NAME | CSSF_MASK_NAME | CSSF_UNICODE;
					ics.pwszName = buf;
					CallProtoService(pRow->szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&ics);
					ti.lpszText = TranslateW(buf);
				}
			}

			SendMessageW(HintWnd, TTM_SETTOOLINFOW, 0, LPARAM(&ti));
		}
		break;

	case WM_KEYUP:
		switch (wParam) {
		case VK_RETURN:
			if (m_grid.GetSelectedCount() == 1)
				ShowContactMsgDlg(GetFocusedContact());
			break;

		case VK_INSERT:
			CallService(MS_FINDADD_FINDADD, 0, 0);
			break;

		case VK_DELETE:
			lParam = m_grid.GetSelectedCount();
			if (lParam > 1)
				DeleteByList();
			else if (lParam == 1)
				DeleteOneContact(GetFocusedContact());
			break;

		case VK_F5:
			onClick_Refresh(0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == HDN_ITEMSTATEICONCLICK) {
			NMHEADER *pdhr = (NMHEADER *)lParam;
			if ((pdhr->pitem->mask & HDI_FORMAT) && (pdhr->pitem->fmt & HDF_CHECKBOX)) {
				int i = ListViewToColumn(pdhr->iItem);
				auto &pCol = g_plugin.m_columns[i];

				if (pdhr->pitem->fmt & HDF_CHECKED) {
					pCol.bFilter = false;
					pdhr->pitem->fmt &= ~HDF_CHECKED;
				}
				else {
					pCol.bFilter = true;
					pdhr->pitem->fmt |= HDF_CHECKED;
				}

				SendMessage(pdhr->hdr.hwndFrom, HDM_SETITEM, pdhr->iItem, LPARAM(pdhr->pitem));
				FillGrid();
				return TRUE;
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// QSMainDlg class implementation

static int CompareSb(const CStatusBarItem *p1, const CStatusBarItem *p2)
{
	if (p1->bAccDel != p2->bAccDel)
		return (p1->bAccDel) ? 1 : -1;

	if (p1->bAccOff != p2->bAccOff)
		return (p1->bAccOff) ? 1 : -1;

	return mir_strcmp(p1->szProto, p2->szProto);
}

QSMainDlg::QSMainDlg(const wchar_t *pwszPattern) :
	CDlgBase(g_plugin, IDD_MAIN),
	m_rows(50),
	m_patterns(1),
	m_sbdata(10, CompareSb),
	m_grid(this, IDC_LIST),
	m_hover(this, 10),
	cmbProto(this, IDC_CB_PROTOCOLS),
	edtFilter(this, IDC_E_SEARCHTEXT),
	btnRefresh(this, IDC_REFRESH),
	chkColorize(this, IDC_CH_COLORIZE),
	chkShowOffline(this, IDC_CH_SHOWOFFLINE)
{
	SetMinSize(300, 160);

	if (pwszPattern)
		m_wszPatternBuf = mir_wstrdup(pwszPattern);
	else if (g_plugin.m_flags & QSO_SAVEPATTERN)
		m_wszPatternBuf = g_plugin.getWStringA("pattern");

	m_hover.OnEvent = Callback(this, &QSMainDlg::onTimer_Hover);

	m_grid.OnBuildMenu = Callback(this, &QSMainDlg::onBuildMenu_Grid);
	m_grid.OnColumnClick = Callback(this, &QSMainDlg::onColumnClick_Grid);
	m_grid.OnCustomDraw = Callback(this, &QSMainDlg::onCustomDraw_Grid);
	m_grid.OnDoubleClick = Callback(this, &QSMainDlg::onDblClick_Grid);

	btnRefresh.OnClick = Callback(this, &QSMainDlg::onClick_Refresh);

	cmbProto.OnSelChanged = Callback(this, &QSMainDlg::onSelChange_Proto);

	edtFilter.OnChange = Callback(this, &QSMainDlg::onChange_Filter);
	chkColorize.OnChange = Callback(this, &QSMainDlg::onChange_Colorize);
	chkShowOffline.OnChange = Callback(this, &QSMainDlg::onChange_ShowOffline);
}

bool QSMainDlg::OnInitDialog()
{
	g_pDlg = this;
	mnuhandle = 0;

	SetCaption(TranslateT("Quick Search"));

	hwndStatusBar = GetDlgItem(m_hwnd, IDC_STATUSBAR);

	HMENU smenu = GetSystemMenu(m_hwnd, false);
	InsertMenu(smenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	InsertMenuW(smenu, 6, MF_BYPOSITION | MF_STRING, IDM_STAYONTOP, TranslateT("Stay on Top"));

	if (g_plugin.m_flags & QSO_STAYONTOP) {
		CheckMenuItem(smenu, IDM_STAYONTOP, MF_BYCOMMAND | MF_CHECKED);
		SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	chkShowOffline.SetState((g_plugin.m_flags & QSO_SHOWOFFLINE) != 0);

	szFilterProto = nullptr; // display all protocols
	if (g_plugin.m_flags & QSO_SHOWOFFLINE)
		bShowOffline = true;

	chkColorize.SetState((g_plugin.m_flags & QSO_COLORIZE) != 0);

	// Window
	INT_PTR tmp = GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE);
	if (g_plugin.m_flags & QSO_TOOLSTYLE)
		tmp |= WS_EX_TOOLWINDOW;
	else
		tmp &= ~WS_EX_TOOLWINDOW;
	SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, tmp);

	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_plugin.getIcon(IDI_QS));

	// ListView
	m_grid.SetImageList(Clist_GetImageList(), LVSIL_SMALL);

	tmp = LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_HEADERDRAGDROP |
		LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER;
	if (g_plugin.m_flags & QSO_DRAWGRID)
		tmp |= LVS_EX_GRIDLINES;
	m_grid.SetExtendedListViewStyle(tmp);

	// ListView header
	HWND header = m_grid.GetHeader();
	SetWindowLongPtrW(header, GWL_STYLE, GetWindowLongPtrW(header, GWL_STYLE) | HDS_CHECKBOXES);

	mir_subclassWindow(edtFilter.GetHwnd(), &sttNewEditProc);

	SetWindowLongPtrW(m_grid.GetHeader(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_grid.GetHeader(), &sttNewLVHProc);
	
	SetWindowLongPtrW(m_grid.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_grid.GetHwnd(), &sttNewLVProc);

	FillProtoCombo();

	PrepareTable();

	if (m_wszPatternBuf) {
		edtFilter.SetText(m_wszPatternBuf);
		MakePattern(m_wszPatternBuf);
	}
	FillGrid();

	// Show sorting column
	HDITEM hdi = {};
	hdi.mask = HDI_FORMAT;
	SendMessageW(header, HDM_GETITEM, g_plugin.m_sortOrder, LPARAM(&hdi));
	if (g_plugin.m_flags & QSO_SORTASC)
		hdi.fmt |= HDF_SORTUP;
	else
		hdi.fmt |= HDF_SORTDOWN;
	SendMessageW(header, HDM_SETITEM, g_plugin.m_sortOrder, LPARAM(&hdi));

	RECT rc = g_plugin.m_rect;
	::SnapToScreen(rc);
	::MoveWindow(m_hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, false);

	HintWnd = CreateWindowExW(0, TOOLTIPS_CLASS, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hwnd, 0, g_plugin.getInst(), 0);

	TOOLINFOW ti;
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS + TTF_IDISHWND;
	ti.hwnd = m_hwnd;
	ti.uId = UINT_PTR(m_grid.GetHwnd());
	SendMessageW(HintWnd, TTM_ADDTOOLW, 0, LPARAM(&ti));

	hAdd = HookEventMessage(ME_DB_CONTACT_ADDED, m_hwnd, WM_CONTACT_ADDED);
	hDelete = HookEventMessage(ME_DB_CONTACT_DELETED, m_hwnd, WM_CONTACT_DELETED);
	hChange = HookEventMessage(ME_CLIST_CONTACTICONCHANGED, m_hwnd, WM_STATUS_CHANGED);
	return true;
}

void QSMainDlg::OnDestroy()
{
	if (mnuhandle)
		Menu_RemoveItem(mnuhandle);

	UnhookEvent(hAdd);
	UnhookEvent(hDelete);
	UnhookEvent(hChange);

	g_pDlg = nullptr;
	
	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	CopyRect(&g_plugin.m_rect, &rc);

	// save column width/order
	SaveColumnOrder();

	g_plugin.SaveOptWnd();

	m_grid.SetImageList(0, LVSIL_SMALL);

	if (g_plugin.m_flags & QSO_SAVEPATTERN)
		g_plugin.setWString("pattern", ptrW(edtFilter.GetText()));

	m_rows.destroy();
	m_patterns.destroy();
}

int QSMainDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_REFRESH:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;

	case IDC_E_SEARCHTEXT:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_LIST:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_STATUSBAR:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

/////////////////////////////////////////////////////////////////////////////////////////
// event handlers

INT_PTR QSMainDlg::OnContactAdded(UINT, WPARAM hContact, LPARAM)
{
	auto *pRow = new CRowItem(hContact, this);
	m_rows.insert(pRow);
	AddContactToList(hContact, pRow);
	ProcessLine(pRow);
	Sort();
	UpdateSB();
	return 0;
}

INT_PTR QSMainDlg::OnContactDeleted(UINT, WPARAM hContact, LPARAM)
{
	int idx = -1;
	CRowItem *pRow = 0;

	for (auto &it : m_rows)
		if (it->hContact == hContact) {
			pRow = it;
			idx = m_rows.indexOf(&it);
			break;
		}

	if (idx == -1)
		return 0;

	int iItem = FindItem(pRow);
	if (iItem != -1)
		m_grid.DeleteItem(iItem);

	m_rows.remove(idx);
	UpdateSB();
	return 0;
}

INT_PTR QSMainDlg::OnStatusChanged(UINT, WPARAM hContact, LPARAM lParam)
{
	auto *pRow = FindRow(hContact);
	if (pRow == nullptr)
		return 0;

	int oldStatus = pRow->status;
	int newStatus = Contact::GetStatus(hContact);
	pRow->status = newStatus;

	if (oldStatus != ID_STATUS_OFFLINE && newStatus != ID_STATUS_OFFLINE)
		ChangeStatusPicture(pRow, hContact, lParam);
	else if (oldStatus != ID_STATUS_OFFLINE) {
		if (g_plugin.m_flags & QSO_SHOWOFFLINE)
			ChangeStatusPicture(pRow, hContact, lParam);
		else
			ProcessLine(pRow, true);
	}
	else if (newStatus != ID_STATUS_OFFLINE) {
		if (g_plugin.m_flags & QSO_SHOWOFFLINE)
			ChangeStatusPicture(pRow, hContact, lParam);
		else {
			pRow->bActive = false;
			m_grid.DeleteItem(FindItem(pRow));
		}
	}

	if (g_plugin.m_flags & QSO_SORTBYSTATUS)
		Sort();
	
	UpdateSB();
	return 0;
}

INT_PTR QSMainDlg::OnSysCommand(UINT, WPARAM wParam, LPARAM)
{
	if (wParam == IDM_STAYONTOP) {
		int h; HWND w;
		if (g_plugin.m_flags & QSO_STAYONTOP) {
			h = MF_BYCOMMAND | MF_UNCHECKED;
			w = HWND_NOTOPMOST;
		}
		else {
			h = MF_BYCOMMAND | MF_CHECKED;
			w = HWND_TOPMOST;
		}
		CheckMenuItem(GetSystemMenu(m_hwnd, false), IDM_STAYONTOP, h);
		SetWindowPos(m_hwnd, w, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		g_plugin.m_flags ^= QSO_STAYONTOP;
	}
	return 0;
}

INT_PTR QSMainDlg::OnMouseMove(UINT, WPARAM, LPARAM lParam)
{
	if (g_bTipperInstalled) {
		RECT rc;
		GetWindowRect(m_grid.GetHwnd(), &rc);

		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		ClientToScreen(m_hwnd, &pt);
		if (!PtInRect(&rc, pt)) {
			if (TTShowed) {
				TTShowed = false;
				CallService(MS_TIPPER_HIDETIP, 0, 0);
			}
		}

		m_hover.Stop();
	}
	return 0;
}

INT_PTR QSMainDlg::OnKeydown(UINT, WPARAM wParam, LPARAM)
{
	if (wParam == VK_F5)
		PostMessage(m_hwnd, WM_COMMAND, IDC_REFRESH, 0);
	return 0;
}

void QSMainDlg::onBuildMenu_Grid(CContextMenuPos *pos)
{
	int w = m_grid.GetSelectedCount();
	if (w > 1)
		ShowMultiPopup(w);
	else
		ShowContactMenu(GetFocusedContact(), GetLVSubItem(pos->pt.x, pos->pt.y));
}

void QSMainDlg::onSelChange_Proto(CCtrlCombo *)
{
	LPARAM lParam = cmbProto.GetItemData(cmbProto.GetCurSel());
	if (lParam == -1 || lParam == 0)
		szFilterProto = nullptr;
	else
		szFilterProto = ((PROTOACCOUNT *)lParam)->szModuleName;

	AdvancedFilter();
}

void QSMainDlg::onChange_Filter(CCtrlEdit *)
{
	if (!m_bInitialized)
		return;

	MakePattern(ptrW(edtFilter.GetText()));
	FillGrid();
}

void QSMainDlg::onChange_ShowOffline(CCtrlCheck *)
{
	if (chkShowOffline.IsChecked()) {
		g_plugin.m_flags |= QSO_SHOWOFFLINE;
		bShowOffline = true;
	}
	else {
		g_plugin.m_flags &= ~QSO_SHOWOFFLINE;
		bShowOffline = false;
	}
	
	AdvancedFilter();
}

void QSMainDlg::onChange_Colorize(CCtrlCheck *)
{
	if (chkColorize.IsChecked())
		g_plugin.m_flags |= QSO_COLORIZE;
	else
		g_plugin.m_flags &= ~QSO_COLORIZE;
	RedrawWindow(m_grid.GetHwnd(), nullptr, 0, RDW_INVALIDATE);
}

void QSMainDlg::onClick_Refresh(CCtrlButton *)
{
	m_rows.destroy();
	PrepareToFill();
	PrepareTable(true);
	FillGrid();
}

void QSMainDlg::onColumnClick_Grid(CCtrlListView::TEventInfo *ev)
{
	HWND header = m_grid.GetHeader();

	// clear sort mark
	HDITEM hdi = {};
	hdi.mask = HDI_FORMAT;
	SendMessage(header, HDM_GETITEM, g_plugin.m_sortOrder, LPARAM(&hdi));
	hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
	SendMessage(header, HDM_SETITEM, g_plugin.m_sortOrder, LPARAM(&hdi));

	if (g_plugin.m_sortOrder != ev->nmlv->iSubItem) {
		g_plugin.m_flags |= QSO_SORTASC;
		g_plugin.m_sortOrder = ev->nmlv->iSubItem;
	}
	else g_plugin.m_flags ^= QSO_SORTASC;;

	// set new sort mark
	SendMessage(header, HDM_GETITEM, g_plugin.m_sortOrder, LPARAM(&hdi));
	if ((g_plugin.m_flags & QSO_SORTASC) == 0)
		hdi.fmt |= HDF_SORTDOWN;
	else
		hdi.fmt &= ~HDF_SORTUP;
	SendMessage(header, HDM_SETITEM, g_plugin.m_sortOrder, LPARAM(&hdi));

	Sort();
}

void QSMainDlg::onDblClick_Grid(CCtrlListView::TEventInfo*)
{
	ShowContactMsgDlg(GetFocusedContact());
}

/////////////////////////////////////////////////////////////////////////////////////////

void QSMainDlg::onCustomDraw_Grid(CCtrlListView::TEventInfo *ev)
{
	LPNMLVCUSTOMDRAW lplvcd = ev->nmcd;
	HICON h;
	RECT rc;
	auto *pRow = (CRowItem *)lplvcd->nmcd.lItemlParam;

	int result = CDRF_DODEFAULT;
	switch (lplvcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		result = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		pRow->GetCellColor(lplvcd->nmcd.dwItemSpec, lplvcd->clrTextBk, lplvcd->clrText);
		result = CDRF_NOTIFYSUBITEMDRAW;
		break;

	case CDDS_SUBITEM + CDDS_ITEMPREPAINT:
		pRow->GetCellColor(lplvcd->nmcd.dwItemSpec, lplvcd->clrTextBk, lplvcd->clrText);
		{
			int sub = ListViewToColumn(lplvcd->iSubItem);
			auto *pCol = &g_plugin.m_columns[sub];
			if (pCol == nullptr)
				break;

			if (pCol->isGender) {
				m_grid.GetSubItemRect(lplvcd->nmcd.dwItemSpec, lplvcd->iSubItem, LVIR_ICON, &rc);

				switch (pRow->pValues[sub].data) {
				case 'F': h = g_plugin.getIcon(IDI_FEMALE); break;
				case 'M': h = g_plugin.getIcon(IDI_MALE); break;
				default: h = 0;
				}

				if (h)
					DrawIconEx(lplvcd->nmcd.hdc, rc.left + 1, rc.top, h, 16, 16, 0, 0, DI_NORMAL);
				result = CDRF_SKIPDEFAULT;
			}
			else if (pCol->isXstatus) {
				int j = _wtoi(pRow->pValues[sub].text);
				if (j > 0 && ProtoServiceExists(pRow->szProto, PS_GETCUSTOMSTATUSICON)) {
					h = (HICON)CallProtoService(pRow->szProto, PS_GETCUSTOMSTATUSICON, j, LR_SHARED);
					m_grid.GetSubItemRect(lplvcd->nmcd.dwItemSpec, lplvcd->iSubItem, LVIR_ICON, &rc);
					DrawIconEx(lplvcd->nmcd.hdc, rc.left + 1, rc.top, h, 16, 16, 0, 0, DI_NORMAL);
				}
				result = CDRF_SKIPDEFAULT;
			}
			else if ((g_plugin.m_flags & QSO_CLIENTICONS) && pCol->isClient)
				result = CDRF_NOTIFYPOSTPAINT;
		}
		break;

	case CDDS_SUBITEM + CDDS_ITEMPOSTPAINT:
		{
			int sub = ListViewToColumn(lplvcd->iSubItem);
			auto *pCol = &g_plugin.m_columns[sub];
			if (pCol == nullptr)
				break;

			if (pCol->isClient) {
				auto *MirVerW = pRow->pValues[sub].text;
				if (MirVerW && *MirVerW && g_bFingerInstalled) {
					h = Finger_GetClientIcon(MirVerW, FALSE);
					m_grid.GetSubItemRect(lplvcd->nmcd.dwItemSpec, lplvcd->iSubItem, LVIR_ICON, &rc);
					DrawIconEx(lplvcd->nmcd.hdc, rc.left + 1, rc.top, h, 16, 16, 0, 0, DI_NORMAL);
					DestroyIcon(h);
				}
			}
			result = CDRF_SKIPDEFAULT;
		}
		break;
	}

	SetWindowLongPtrW(m_hwnd, DWLP_MSGRESULT, result);
}

void QSMainDlg::onTimer_Hover(CTimer *pTimer)
{
	pTimer->Stop();

	if (GetForegroundWindow() != m_hwnd)
		return;

	auto *pRow = GetRow(OldHItem);
	if (pRow == 0)
		return;

	POINT pt;
	GetCursorPos(&pt);

	RECT rcItem;
	m_grid.GetItemRect(OldHItem, &rcItem, 0);
	ScreenToClient(m_grid.GetHwnd(), &pt);
	if (!PtInRect(&rcItem, pt))
		return;

	CLCINFOTIP info = {};
	info.cbSize = sizeof(info);
	info.hItem = HANDLE(pRow->hContact);
	Tipper_ShowTip(0, &info);
	TTShowed = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CloseSrWindow(bool)
{
	if (!g_pDlg)
		return false;

	g_pDlg->Close();
	return true;
}

int OpenSrWindow(const wchar_t *pwszPattern)
{
	if (g_pDlg) {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(g_pDlg->GetHwnd(), &wp);
		if (wp.showCmd == SW_SHOWMINIMIZED)
			g_pDlg->Show(SW_RESTORE);
		SetForegroundWindow(g_pDlg->GetHwnd());
		return true;
	}

	int count = 0;
	for (auto &it : g_plugin.m_columns)
		if (it->bEnabled)
			count++;

	// no even one visible column
	if (count == 0)
		return true;

	g_plugin.LoadOptWnd();

	auto *pDlg = new QSMainDlg(pwszPattern);
	if (pDlg->PrepareToFill())
		pDlg->Create();
	else
		delete pDlg;

	return true;
}
